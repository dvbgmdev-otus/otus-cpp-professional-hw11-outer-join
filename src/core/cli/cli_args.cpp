#include "cli_args.h"

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace {

CliArgsParseResult make_error(const std::string& error) {
    return CliArgsParseResult{ false, 0, 0, error };
}

CliArgsParseResult make_success(std::uint16_t port, std::size_t block_size) {
    return CliArgsParseResult{ true, port, block_size, {} };
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
    if (argc != 3) {
        return make_error("Expected two arguments: port and block size");
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

    const char* block_size_value = argv[2];
    if (!is_decimal_number(block_size_value)) {
        return make_error("Block size must be a positive integer");
    }

    errno = 0;
    const unsigned long long parsed_block_size = std::strtoull(block_size_value, nullptr, 10);
    if (errno == ERANGE || parsed_block_size == 0 ||
        parsed_block_size > std::numeric_limits<std::size_t>::max()) {
        return make_error("Block size must be a positive integer");
    }

    return make_success(static_cast<std::uint16_t>(parsed_port),
                        static_cast<std::size_t>(parsed_block_size));
}
