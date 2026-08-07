#include "cli_args.h"

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace {

CliArgsParseResult make_error(const std::string& error) {
    return CliArgsParseResult{ false, 0, error };
}

CliArgsParseResult make_success(std::uint16_t port) {
    return CliArgsParseResult{ true, port, {} };
}

bool is_decimal_number(const char* value) {
    const char* current = value;
    while (*current != '\0') {
        if (!std::isdigit(static_cast<unsigned char>(*current))) {
            return false;
        }
        ++current;
    }
    return current != value;
}

}  // namespace

CliArgsParseResult parse_cli_args(int argc, const char* const argv[]) {
    if (argc != 2) {
        return make_error("Expected one argument: port");
    }

    const char* port_value = argv[1];
    if (!is_decimal_number(port_value)) {
        return make_error("Port must be an integer from 1 to 65535");
    }

    errno = 0;
    const unsigned long long parsed_port = std::strtoull(port_value, nullptr, 10);
    if (errno == ERANGE || parsed_port == 0 ||
        parsed_port > std::numeric_limits<std::uint16_t>::max()) {
        return make_error("Port must be an integer from 1 to 65535");
    }

    return make_success(static_cast<std::uint16_t>(parsed_port));
}
