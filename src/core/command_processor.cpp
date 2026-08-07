#include "command_processor.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "database.h"

namespace {

const std::string OK_RESPONSE = "OK\n";

std::vector<std::string> split(const std::string& command) {
    std::vector<std::string> tokens;
    std::size_t begin = 0;

    std::size_t separator = command.find(' ', begin);
    while (separator != std::string::npos) {
        tokens.push_back(command.substr(begin, separator - begin));
        begin = separator + 1;
        separator = command.find(' ', begin);
    }
    tokens.push_back(command.substr(begin));
    return tokens;
}  // LCOV_EXCL_LINE

bool parse_table(const std::string& value, database::Table& table) {
    if (value == "A") {
        table = database::Table::A;
        return true;
    }
    if (value == "B") {
        table = database::Table::B;
        return true;
    }
    return false;
}

bool parse_id(const std::string& value, int& id) {
    if (value.empty()) {
        return false;
    }

    try {
        std::size_t parsed = 0;
        const long long result = std::stoll(value, &parsed, 10);
        if (parsed != value.size() || result < std::numeric_limits<int>::min() ||
            result > std::numeric_limits<int>::max()) {
            return false;
        }
        id = static_cast<int>(result);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

std::string error_response(const std::string& message) { return "ERR " + message + '\n'; }

std::string rows_response(const std::vector<database::JoinedRow>& rows) {
    std::string response;
    for (const database::JoinedRow& row : rows) {
        response += std::to_string(row.id) + ',' + row.a_name + ',' + row.b_name + '\n';
    }
    return response + OK_RESPONSE;
}

}  // namespace

CommandProcessor::CommandProcessor(database::Database& database) : m_Database(database) {}

std::string CommandProcessor::process(const std::string& command) {
    const std::vector<std::string> tokens = split(command);

    if (!tokens.empty() && tokens[0] == "INSERT") {
        if (tokens.size() != 4 || tokens[3].empty()) {
            return error_response("invalid arguments");
        }

        database::Table table = database::Table::A;
        if (!parse_table(tokens[1], table)) {
            return error_response("unknown table");
        }

        int id = 0;
        if (!parse_id(tokens[2], id)) {
            return error_response("invalid id");
        }

        try {
            if (m_Database.insert(table, id, tokens[3]) ==
                database::InsertResult::Duplicate) {
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

        database::Table table = database::Table::A;
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
