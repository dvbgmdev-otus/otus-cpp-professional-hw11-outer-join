#!/usr/bin/env bash
set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/../../scripts/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

BINARY="${1:-$BIN_DIR/join_server}"
SERVER_PID=""
WORKDIR=""
ERROR_OUTPUT=""
PORT=19005

cleanup() {
    if [[ -n "$SERVER_PID" ]] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi

    if [[ -n "$WORKDIR" && -d "$WORKDIR" ]]; then
        rm -rf "$WORKDIR"
    fi
}

trap cleanup EXIT

fail() {
    log_error "$1"
    if [[ -n "$ERROR_OUTPUT" && -s "$ERROR_OUTPUT" ]]; then
        cat "$ERROR_OUTPUT" >&2
    fi
    exit 1
}

check_equal() {
    if [[ "$2" != "$3" ]]; then
        log_error "Test failed: $1"
        log_error "Expected: [$2]"
        log_error "Actual:   [$3]"
        exit 1
    fi

    log_ok "Passed: $1"
}

wait_for_server() {
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

start_server() {
    WORKDIR="$(mktemp -d)"
    ERROR_OUTPUT="$WORKDIR/server.err"

    "$BINARY" "$PORT" >"$WORKDIR/server.out" 2>"$ERROR_OUTPUT" &
    SERVER_PID=$!
    wait_for_server
}

send_commands() {
    printf '%s' "$1" | nc -N -w 2 localhost "$PORT"
}

# Part 1. Границы TCP-чтения

# Test 1.1. Команда, разделённая между двумя записями, обрабатывается целиком.
check_fragmented_command() {
    local actual

    actual="$({
        printf 'INSERT A 1 frag'
        sleep 0.1
        printf 'mented\n'
    } | nc -N -w 2 localhost "$PORT")"

    check_equal "fragmented command" "OK" "$actual"
}

# Test 1.2. Несколько команд из одной записи обрабатываются по порядку.
check_command_batch() {
    local commands=$'INSERT B 1 pair\nINTERSECTION\n'
    local expected=$'OK\n1,fragmented,pair\nOK'

    check_equal "command batch" "$expected" "$(send_commands "$commands")"
}

# Part 2. Общее состояние сервера

# Test 2.1. Данные из одного TCP-подключения доступны в другом.
check_shared_database() {
    check_equal "insert from first connection" "OK" \
        "$(send_commands $'INSERT A 2 shared\n')"
    check_equal "select from second connection" $'2,shared,\nOK' \
        "$(send_commands $'SYMMETRIC_DIFFERENCE\n')"
}

# Part 3. Ошибки и очистка

# Test 3.1. Ошибки команд не мешают обработке следующих команд.
check_recovery_after_errors() {
    local commands=$'UNKNOWN\nINSERT C 2 value\nINSERT B 2 value\nINTERSECTION\n'
    local expected=$'ERR unknown command\nERR unknown table\nOK\n1,fragmented,pair\n2,shared,value\nOK'

    check_equal "recovery after errors" "$expected" "$(send_commands "$commands")"
}

# Test 3.2. Очистка таблицы A не изменяет таблицу B.
check_independent_truncate() {
    local commands=$'TRUNCATE A\nSYMMETRIC_DIFFERENCE\n'
    local expected=$'OK\n1,,pair\n2,,value\nOK'

    check_equal "independent TRUNCATE" "$expected" "$(send_commands "$commands")"
}

# Part 4. Ошибки запуска

# Test 4.1. Запуск без порта завершается с сообщением об ошибке.
check_missing_port() {
    local actual

    if actual="$("$BINARY" 2>&1)"; then
        fail "Server started without port argument"
    fi

    check_equal "missing port" "Expected one argument: port" "$actual"
}

# Test 4.2. Запуск на занятом порту завершается с сообщением об ошибке.
check_port_conflict() {
    local actual

    if actual="$("$BINARY" "$PORT" 2>&1)"; then
        fail "Second server started on occupied port $PORT"
    fi
    if [[ -z "$actual" ]]; then
        fail "Port conflict did not produce an error message"
    fi

    log_ok "Passed: occupied port"
}

main() {
    if [[ ! -x "$BINARY" ]]; then
        fail "Binary not found or not executable: $BINARY"
    fi

    check_missing_port
    start_server
    check_port_conflict
    check_fragmented_command
    check_command_batch
    check_shared_database
    check_recovery_after_errors
    check_independent_truncate

    log_ok "Self-check passed"
}

main "$@"
