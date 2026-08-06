#!/usr/bin/env bash
set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

# Приёмочный тест воспроизводит оба примера из TASK.md через настоящие
# TCP-подключения и дополнительно проверяет созданные bulk-файлы.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/../../scripts/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

BINARY="${1:-$BIN_DIR/bulk_server}"
SERVER_PID=""
WORKDIR=""
OUTPUT=""
ERROR_OUTPUT=""
PORT=""

# Сервер запускается в фоне, поэтому при любом завершении теста необходимо
# остановить процесс и удалить временный каталог с результатами сценария.
cleanup() {
    if [[ -n "$SERVER_PID" ]] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi

    if [[ -n "$WORKDIR" && -d "$WORKDIR" ]]; then
        rm -rf "$WORKDIR"
    fi

    SERVER_PID=""
    WORKDIR=""
}

trap cleanup EXIT

fail() {
    log_error "$1"
    if [[ -n "$ERROR_OUTPUT" && -s "$ERROR_OUTPUT" ]]; then
        cat "$ERROR_OUTPUT" >&2
    fi
    exit 1
}

wait_for_server() {
    # Подключение через nc подтверждает, что async_accept уже готов принимать
    # тестового клиента. Проверка PID позволяет сразу заметить падение сервера.
    for _ in {1..100}; do
        if nc -z localhost "$PORT" 2>/dev/null; then
            return
        fi
        if ! kill -0 "$SERVER_PID" 2>/dev/null; then
            fail "Server stopped before opening port $PORT"
        fi
        sleep 0.05
    done

    fail "Server did not open port $PORT"
}

wait_for_output() {
    local expected_count="$1"
    local line_count

    # Запись блоков выполняется асинхронно. Ожидание с ограничением по времени
    # исключает гонку между завершением nc и записью последнего блока.
    for _ in {1..100}; do
        line_count="$(wc -l < "$OUTPUT")"
        if (( line_count >= expected_count )); then
            return
        fi
        sleep 0.05
    done

    fail "Expected $expected_count output lines, got $line_count"
}

check_equal() {
    if [[ "$2" != "$3" ]]; then
        log_error "Test failed: $1"
        log_error "Expected: [$2]"
        log_error "Actual:   [$3]"
        exit 1
    fi
}

start_server() {
    PORT="$1"
    local block_size="$2"

    WORKDIR="$(mktemp -d)"
    OUTPUT="$WORKDIR/server.out"
    ERROR_OUTPUT="$WORKDIR/server.err"

    (
        cd "$WORKDIR"
        # Построчная буферизация делает строки bulk сразу доступными тесту.
        exec stdbuf -oL -eL "$BINARY" "$PORT" "$block_size" >"$OUTPUT" 2>"$ERROR_OUTPUT"
    ) &
    SERVER_PID=$!

    wait_for_server
}

#if (1)  # Part 1. Проверка примеров из задания

# Test 1.1. Один клиент формирует четыре статических блока.
run_single_client_test() {
    local expected=$'bulk: 0, 1, 2\nbulk: 3, 4, 5\nbulk: 6, 7, 8\nbulk: 9'
    local actual
    local logs
    local log_count

    start_server 19004 3
    # nc -N закрывает соединение после EOF, а -w ограничивает ожидание клиента.
    seq 0 9 | nc -N -w 2 localhost 19004
    wait_for_output 4

    actual="$(cat "$OUTPUT")"
    logs="$(find "$WORKDIR" -maxdepth 1 -type f -name 'bulk*.log' -exec cat {} + | sort)"
    log_count="$(find "$WORKDIR" -maxdepth 1 -type f -name 'bulk*.log' | wc -l)"

    check_equal "TASK.md console output" "$expected" "$actual"
    check_equal "TASK.md log contents" "$(printf '%s\n' "$expected" | sort)" "$logs"
    check_equal "TASK.md log file count" "4" "$log_count"

    log_ok "Passed: TASK.md single client example"
    cleanup
}

# Test 1.2. Два одновременных клиента формируют общие статические блоки.
run_two_clients_test() {
    local first_client_pid
    local second_client_pid
    local commands
    local block_sizes
    local output
    local logs
    local log_count

    start_server 19005 3

    # Начальная пауза даёт серверу принять оба подключения до отправки команд.
    # Конечная пауза удерживает оба контекста активными до обработки данных.
    { sleep 0.2; seq 0 9; sleep 0.2; } | nc -N -w 2 localhost 19005 &
    first_client_pid=$!
    { sleep 0.2; seq 10 19; sleep 0.2; } | nc -N -w 2 localhost 19005 &
    second_client_pid=$!

    wait "$first_client_pid"
    wait "$second_client_pid"
    wait_for_output 7

    output="$(cat "$OUTPUT")"
    # Порядок смешивания клиентов не определён заданием. Поэтому отдельно
    # проверяются полный набор команд и размеры сформированных блоков.
    commands="$(printf '%s\n' "$output" | sed 's/^bulk: //; s/, /\n/g' | sort -n)"
    block_sizes="$(printf '%s\n' "$output" | awk -F', ' '{print NF}' | sort -n)"
    logs="$(find "$WORKDIR" -maxdepth 1 -type f -name 'bulk*.log' -exec cat {} + | sort)"
    log_count="$(find "$WORKDIR" -maxdepth 1 -type f -name 'bulk*.log' | wc -l)"

    check_equal "TASK.md two client commands" "$(seq 0 19)" "$commands"
    check_equal "TASK.md two client block sizes" $'2\n3\n3\n3\n3\n3\n3' "$block_sizes"
    check_equal "TASK.md two client log contents" "$(printf '%s\n' "$output" | sort)" "$logs"
    check_equal "TASK.md two client log file count" "7" "$log_count"

    log_ok "Passed: TASK.md two clients example"
    cleanup
}

#endif  # Part 1. Проверка примеров из задания

main() {
    if [[ ! -x "$BINARY" ]]; then
        fail "Binary not found or not executable: $BINARY"
    fi

    run_single_client_test
    run_two_clients_test

    log_ok "TASK.md check passed"
}

main "$@"
