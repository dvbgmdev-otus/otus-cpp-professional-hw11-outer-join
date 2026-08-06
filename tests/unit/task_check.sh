#!/usr/bin/env bash
set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

# Приёмочный тест проверяет протокол join_server через настоящие
# TCP-подключения и воспроизводит данные из TASK.md.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/../../scripts/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

BINARY="${1:-$BIN_DIR/join_server}"
SERVER_PID=""
WORKDIR=""
ERROR_OUTPUT=""
PORT=19004

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

#if (1)  # Part 1. Сценарий TASK.md

# Test 1.1. Сервер воспроизводит ввод и ожидаемый вывод из TASK.md.
run_task_scenario() {
    local commands=$'INSERT A 0 lean\nINSERT A 0 understand\nINSERT A 1 sweater\nINSERT A 2 frank\nINSERT A 3 violation\nINSERT A 4 quality\nINSERT A 5 precision\nINSERT B 3 proposal\nINSERT B 4 example\nINSERT B 5 lake\nINSERT B 6 flour\nINSERT B 7 wonder\nINSERT B 8 selection\nINTERSECTION\nSYMMETRIC_DIFFERENCE\nTRUNCATE A\n'
    local expected=$'OK\nERR duplicate 0\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\nOK\n3,violation,proposal\n4,quality,example\n5,precision,lake\nOK\n0,lean,\n1,sweater,\n2,frank,\n6,,flour\n7,,wonder\n8,,selection\nOK\nOK'

    check_equal "TASK.md scenario" "$expected" "$(send_commands "$commands")"
    log_ok "Passed: TASK.md scenario"
}

#endif  # Part 1. Сценарий TASK.md

main() {
    if [[ ! -x "$BINARY" ]]; then
        fail "Binary not found or not executable: $BINARY"
    fi

    start_server
    run_task_scenario

    log_ok "TASK.md check passed"
}

main "$@"
