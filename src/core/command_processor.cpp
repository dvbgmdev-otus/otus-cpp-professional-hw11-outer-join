#include "command_processor.h"

#include <charconv>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "database.h"

namespace {

const std::string OK_RESPONSE = "OK\n";

std::vector<std::string_view> split(
    std::string_view command, std::size_t max_tokens = std::numeric_limits<std::size_t>::max()) {
    std::vector<std::string_view> tokens;
    std::size_t begin = 0;

    std::size_t separator = command.find(' ', begin);
    while (separator != std::string::npos && tokens.size() + 1 < max_tokens) {
        tokens.push_back(command.substr(begin, separator - begin));
        begin = separator + 1;
        separator = command.find(' ', begin);
    }
    tokens.push_back(command.substr(begin));
    return tokens;
}  // LCOV_EXCL_LINE

bool parse_table(std::string_view value, Table& table) {
    if (value == "A") {
        table = Table::A;
        return true;
    }
    if (value == "B") {
        table = Table::B;
        return true;
    }
    return false;
}

bool parse_id(std::string_view value, int& id) {
    if (value.empty()) {
        return false;
    }

    const char* const end = value.data() + value.size();
    const std::from_chars_result result = std::from_chars(value.data(), end, id);
    if (result.ec != std::errc{} || result.ptr != end) {
        return false;
    }
    return true;
}

std::string error_response(const std::string& message) { return "ERR " + message + '\n'; }

std::string rows_response(const std::vector<JoinedRow>& rows) {
    std::string response;
    for (const JoinedRow& row : rows) {
        response += std::to_string(row.id) + ',' + row.a_name + ',' + row.b_name + '\n';
    }
    return response + OK_RESPONSE;
}

}  // namespace

CommandProcessor::CommandProcessor(Database& database) : m_Database(database) {}

std::string CommandProcessor::process(const std::string& command) {
    std::vector<std::string_view> tokens = split(command);

    if (!tokens.empty() && tokens[0] == "INSERT") {
        tokens = split(command, 4);
        if (tokens.size() != 4 || tokens[1].empty() || tokens[3].empty()) {
            return error_response("invalid arguments");
        }

        Table table = Table::A;
        if (!parse_table(tokens[1], table)) {
            return error_response("unknown table");
        }

        int id = 0;
        if (!parse_id(tokens[2], id)) {
            return error_response("invalid id");
        }

        try {
            if (m_Database.insert(table, id, std::string(tokens[3])) == InsertResult::Duplicate) {
                return error_response("duplicate " + std::to_string(id));
            }
        } catch (const std::exception& error) {  // LCOV_EXCL_START
            return error_response(error.what());
        }
        // LCOV_EXCL_STOP

        return OK_RESPONSE;
    }

    if (!tokens.empty() && tokens[0] == "TRUNCATE") {
        if (tokens.size() != 2) {
            return error_response("invalid arguments");
        }

        Table table = Table::A;
        if (!parse_table(tokens[1], table)) {
            return error_response("unknown table");
        }

        try {
            m_Database.truncate(table);
        } catch (const std::exception& error) {  // LCOV_EXCL_START
            return error_response(error.what());
        }
        // LCOV_EXCL_STOP
        return OK_RESPONSE;
    }

    if (!tokens.empty() && tokens[0] == "INTERSECTION") {
        if (tokens.size() != 1) {
            return error_response("invalid arguments");
        }

        try {
            return rows_response(m_Database.intersection());
        } catch (const std::exception& error) {  // LCOV_EXCL_START
            return error_response(error.what());
        }
        // LCOV_EXCL_STOP
    }

    if (!tokens.empty() && tokens[0] == "SYMMETRIC_DIFFERENCE") {
        if (tokens.size() != 1) {
            return error_response("invalid arguments");
        }

        try {
            return rows_response(m_Database.symmetric_difference());
        } catch (const std::exception& error) {  // LCOV_EXCL_START
            return error_response(error.what());
        }
        // LCOV_EXCL_STOP
    }

    return error_response("unknown command");
}
