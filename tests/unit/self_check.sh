#!/usr/bin/env bash
set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/../../scripts/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

BINARY="${1:-$BIN_DIR/bulk}"

SELF_CHECK_SERVER_PID=""
SELF_CHECK_WORKDIR=""
SELF_CHECK_OUTPUT=""
SELF_CHECK_ERROR=""

cleanup() {
    if [[ -n "$SELF_CHECK_SERVER_PID" ]] && kill -0 "$SELF_CHECK_SERVER_PID" 2>/dev/null; then
        kill "$SELF_CHECK_SERVER_PID" 2>/dev/null || true
        wait "$SELF_CHECK_SERVER_PID" 2>/dev/null || true
    fi
    SELF_CHECK_SERVER_PID=""

    if [[ -n "$SELF_CHECK_WORKDIR" && -d "$SELF_CHECK_WORKDIR" ]]; then
        rm -rf "$SELF_CHECK_WORKDIR"
    fi
    SELF_CHECK_WORKDIR=""
}

trap cleanup EXIT

fail() {
    local message="$1"

    log_error "$message"
    if [[ -n "$SELF_CHECK_ERROR" && -s "$SELF_CHECK_ERROR" ]]; then
        log_error "Server stderr:"
        cat "$SELF_CHECK_ERROR" >&2
    fi
    exit 1
}

check_output() {
    local name="$1"
    local expected="$2"
    local actual="$3"

    if [[ "$actual" != "$expected" ]]; then
        log_error "Test failed: $name"
        log_error "Expected: [$expected]"
        log_error "Actual:   [$actual]"
        exit 1
    fi

    log_ok "Passed: $name"
}

check_output_prefix() {
    local name="$1"
    local expected_prefix="$2"
    local actual="$3"

    if [[ "$actual" != "$expected_prefix"* ]]; then
        log_error "Test failed: $name"
        log_error "Expected prefix: [$expected_prefix]"
        log_error "Actual:          [$actual]"
        exit 1
    fi

    log_ok "Passed: $name"
}

wait_for_server() {
    local port="$1"

    for _ in {1..100}; do
        if nc -z localhost "$port" 2>/dev/null; then
            return
        fi
        if ! kill -0 "$SELF_CHECK_SERVER_PID" 2>/dev/null; then
            fail "Server stopped before opening port $port"
        fi
        sleep 0.05
    done

    fail "Server did not open port $port"
}

wait_for_output_lines() {
    local expected_count="$1"
    local actual_count

    for _ in {1..100}; do
        actual_count="$(wc -l < "$SELF_CHECK_OUTPUT")"
        if (( actual_count >= expected_count )); then
            return
        fi
        if ! kill -0 "$SELF_CHECK_SERVER_PID" 2>/dev/null; then
            fail "Server stopped while waiting for output"
        fi
        sleep 0.05
    done

    fail "Expected $expected_count output lines, got $actual_count"
}

start_server() {
    local port="$1"
    local block_size="$2"

    cleanup
    SELF_CHECK_WORKDIR="$(mktemp -d)"
    SELF_CHECK_OUTPUT="$SELF_CHECK_WORKDIR/server.out"
    SELF_CHECK_ERROR="$SELF_CHECK_WORKDIR/server.err"

    (
        cd "$SELF_CHECK_WORKDIR"
        exec stdbuf -oL -eL "$BINARY" "$port" "$block_size" \
            >"$SELF_CHECK_OUTPUT" 2>"$SELF_CHECK_ERROR"
    ) &
    SELF_CHECK_SERVER_PID=$!

    wait_for_server "$port"
}

send_commands() {
    local port="$1"
    local input="$2"

    printf '%s' "$input" | nc -N -w 2 localhost "$port"
}

check_log_files() {
    local name="$1"
    local expected="$2"
    local expected_sorted
    local actual_sorted

    expected_sorted="$(printf '%s\n' "$expected" | sort)"
    actual_sorted="$(find "$SELF_CHECK_WORKDIR" -maxdepth 1 -type f -name 'bulk*.log' \
        -exec cat {} + | sort)"

    check_output "$name" "$expected_sorted" "$actual_sorted"
}

run_error_case() {
    local name="$1"
    local expected="$2"
    local output
    shift 2

    if output="$("$BINARY" "$@" 2>&1)"; then
        fail "Test failed: $name; expected non-zero exit code"
    fi

    check_output "$name" "$expected" "$output"
}

#if (1)  # Part 1. Проверка аргументов командной строки

# Test 1.1. Запуск без порта и размера блока завершается с ошибкой.
run_cli_error_test() {
    run_error_case \
        "missing port and block size arguments" \
        "Expected two arguments: port and block size"
}

# Test 1.2. Запуск на занятом порту завершается с понятной ошибкой.
run_port_conflict_test() {
    local port=19000
    local output

    start_server "$port" 3
    if output="$("$BINARY" "$port" 3 2>&1)"; then
        fail "Test failed: port is already in use; expected non-zero exit code"
    fi

    check_output_prefix \
        "port is already in use" \
        "bind: Address already in use" \
        "$output"
}

#endif  # Part 1. Проверка аргументов командной строки

#if (1)  # Part 2. Обработка одного TCP-подключения

# Test 2.1. Статический блок дополняется командами перед закрытием клиента.
run_static_blocks_test() {
    local port=19001
    local expected=$'bulk: cmd1, cmd2, cmd3\nbulk: cmd4, cmd5'
    local actual

    start_server "$port" 3
    send_commands "$port" $'cmd1\ncmd2\ncmd3\ncmd4\ncmd5\n'
    wait_for_output_lines 2

    actual="$(cat "$SELF_CHECK_OUTPUT")"
    check_output "static blocks and disconnect flush" "$expected" "$actual"
    check_log_files "static block log files" "$expected"
}

# Test 2.2. Незакрытый динамический блок отбрасывается при отключении клиента.
run_dynamic_blocks_test() {
    local port=19002
    local expected=$'bulk: cmd1, cmd2\nbulk: cmd3, cmd4'
    local actual

    start_server "$port" 3
    send_commands "$port" $'cmd1\ncmd2\n{\ncmd3\ncmd4\n}\n{\ncmd5\ncmd6\n'
    wait_for_output_lines 2

    actual="$(cat "$SELF_CHECK_OUTPUT")"
    check_output "dynamic and unfinished blocks" "$expected" "$actual"
    check_log_files "dynamic block log files" "$expected"
}

#endif  # Part 2. Обработка одного TCP-подключения

#if (1)  # Part 3. Совместная обработка TCP-подключений

# Test 3.1. Статические команды двух одновременных клиентов образуют общие блоки.
run_multiple_clients_test() {
    local port=19003
    local first_client_pid
    local second_client_pid
    local commands
    local block_sizes
    local output

    start_server "$port" 3

    { sleep 0.2; printf 'a1\na2\n'; sleep 0.5; } | nc -N -w 2 localhost "$port" &
    first_client_pid=$!
    { sleep 0.2; printf 'b1\nb2\n'; sleep 0.5; } | nc -N -w 2 localhost "$port" &
    second_client_pid=$!

    wait "$first_client_pid"
    wait "$second_client_pid"
    wait_for_output_lines 2

    output="$(cat "$SELF_CHECK_OUTPUT")"
    commands="$(printf '%s\n' "$output" | sed 's/^bulk: //; s/, /\n/g' | sort)"
    block_sizes="$(printf '%s\n' "$output" | awk -F', ' '{print NF}' | sort -n)"

    check_output "commands from multiple clients" $'a1\na2\nb1\nb2' "$commands"
    check_output "shared static block sizes" $'1\n3' "$block_sizes"
    check_log_files "multiple client log files" "$output"
}

#endif  # Part 3. Совместная обработка TCP-подключений

main() {
    if [[ ! -x "$BINARY" ]]; then
        fail "Binary not found or not executable: $BINARY"
    fi

    run_cli_error_test
    run_port_conflict_test
    run_static_blocks_test
    run_dynamic_blocks_test
    run_multiple_clients_test

    log_ok "Self-check passed"
}

main "$@"
