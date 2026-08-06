#include "database.h"

#include <memory>
#include <stdexcept>
#include <string>

#include <sqlite3.h>

namespace {

struct StatementDeleter {
    void operator()(sqlite3_stmt* statement) const { sqlite3_finalize(statement); }
};

using Statement = std::unique_ptr<sqlite3_stmt, StatementDeleter>;

const char* insert_query(database::Table table) {
    switch (table) {
        case database::Table::A:
            return "INSERT INTO A(id, name) VALUES(?, ?)";
        case database::Table::B:
            return "INSERT INTO B(id, name) VALUES(?, ?)";
    }

    throw std::invalid_argument("Unknown table");
}

const char* truncate_query(database::Table table) {
    switch (table) {
        case database::Table::A:
            return "DELETE FROM A";
        case database::Table::B:
            return "DELETE FROM B";
    }

    throw std::invalid_argument("Unknown table");
}

std::runtime_error sqlite_error(sqlite3* database, const std::string& operation) {
    return std::runtime_error(operation + ": " + sqlite3_errmsg(database));
}

}  // namespace

namespace database {

Database::Database() {
    const int open_result = sqlite3_open(":memory:", &m_Database);
    if (open_result != SQLITE_OK) {
        const std::string message = m_Database == nullptr
                                        ? "Unable to open SQLite database"
                                        : sqlite3_errmsg(m_Database);
        sqlite3_close(m_Database);
        m_Database = nullptr;
        throw std::runtime_error(message);
    }

    try {
        execute("CREATE TABLE A(id INTEGER PRIMARY KEY, name TEXT NOT NULL)");
        execute("CREATE TABLE B(id INTEGER PRIMARY KEY, name TEXT NOT NULL)");
    } catch (...) {
        sqlite3_close(m_Database);
        m_Database = nullptr;
        throw;
    }
}

Database::~Database() { sqlite3_close(m_Database); }

InsertResult Database::insert(Table table, int id, const std::string& name) {
    sqlite3_stmt* raw_statement = nullptr;
    if (sqlite3_prepare_v2(m_Database, insert_query(table), -1, &raw_statement, nullptr) !=
        SQLITE_OK) {
        throw sqlite_error(m_Database, "Unable to prepare INSERT");
    }
    Statement statement(raw_statement);

    if (sqlite3_bind_int(statement.get(), 1, id) != SQLITE_OK ||
        sqlite3_bind_text(statement.get(), 2, name.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK) {
        throw sqlite_error(m_Database, "Unable to bind INSERT parameters");
    }

    const int step_result = sqlite3_step(statement.get());
    if (step_result == SQLITE_DONE) {
        return InsertResult::Inserted;
    }

    const int extended_error = sqlite3_extended_errcode(m_Database);
    if (extended_error == SQLITE_CONSTRAINT_PRIMARYKEY ||
        extended_error == SQLITE_CONSTRAINT_UNIQUE) {
        return InsertResult::Duplicate;
    }

    throw sqlite_error(m_Database, "Unable to execute INSERT");
}

void Database::truncate(Table table) { execute(truncate_query(table)); }

void Database::execute(const std::string& query) {
    char* raw_error = nullptr;
    const int result = sqlite3_exec(m_Database, query.c_str(), nullptr, nullptr, &raw_error);
    if (result == SQLITE_OK) {
        return;
    }

    const std::string message = raw_error == nullptr ? sqlite3_errmsg(m_Database) : raw_error;
    sqlite3_free(raw_error);
    throw std::runtime_error("Unable to execute SQL: " + message);
}

}  // namespace database
