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

const char* insert_query(Table table) {
    switch (table) {
        case Table::A:
            return "INSERT INTO A(id, name) VALUES(?, ?)";
        case Table::B:
            return "INSERT INTO B(id, name) VALUES(?, ?)";
    }

    throw std::invalid_argument("Unknown table");
}

const char* truncate_query(Table table) {
    switch (table) {
        case Table::A:
            return "DELETE FROM A";
        case Table::B:
            return "DELETE FROM B";
    }

    throw std::invalid_argument("Unknown table");
}

// LCOV_EXCL_START
std::runtime_error sqlite_error(sqlite3* database, const std::string& operation) {
    return std::runtime_error(operation + ": " + sqlite3_errmsg(database));
}
// LCOV_EXCL_STOP

std::string column_text(sqlite3_stmt* statement, int column) {
    const unsigned char* value = sqlite3_column_text(statement, column);
    return value == nullptr ? std::string{} : reinterpret_cast<const char*>(value);
}

}  // namespace

Database::Database() {
    const int open_result = sqlite3_open(":memory:", &m_Database);
    if (open_result != SQLITE_OK) {  // LCOV_EXCL_START
        const std::string message = m_Database == nullptr
                                        ? "Unable to open SQLite database"
                                        : sqlite3_errmsg(m_Database);
        sqlite3_close(m_Database);
        m_Database = nullptr;
        throw std::runtime_error(message);
    }
    // LCOV_EXCL_STOP

    try {
        execute("CREATE TABLE A(id INTEGER PRIMARY KEY, name TEXT NOT NULL)");
        execute("CREATE TABLE B(id INTEGER PRIMARY KEY, name TEXT NOT NULL)");
    } catch (...) {  // LCOV_EXCL_START
        sqlite3_close(m_Database);
        m_Database = nullptr;
        throw;
    }
    // LCOV_EXCL_STOP
}

Database::~Database() { sqlite3_close(m_Database); }

InsertResult Database::insert(Table table, int id, const std::string& name) {
    sqlite3_stmt* raw_statement = nullptr;
    if (sqlite3_prepare_v2(m_Database, insert_query(table), -1, &raw_statement, nullptr) !=
        SQLITE_OK) {
        throw sqlite_error(m_Database, "Unable to prepare INSERT");  // LCOV_EXCL_LINE
    }
    Statement statement(raw_statement);

    if (sqlite3_bind_int(statement.get(), 1, id) != SQLITE_OK ||
        sqlite3_bind_text(statement.get(), 2, name.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK) {
        throw sqlite_error(m_Database, "Unable to bind INSERT parameters");  // LCOV_EXCL_LINE
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

    throw sqlite_error(m_Database, "Unable to execute INSERT");  // LCOV_EXCL_LINE
}

void Database::truncate(Table table) { execute(truncate_query(table)); }

std::vector<JoinedRow> Database::intersection() {
    return select_joined_rows("SELECT A.id, A.name, B.name "
                              "FROM A INNER JOIN B ON A.id = B.id "
                              "ORDER BY A.id");
}

std::vector<JoinedRow> Database::symmetric_difference() {
    return select_joined_rows("SELECT A.id AS id, A.name AS a_name, NULL AS b_name "
                              "FROM A LEFT JOIN B ON A.id = B.id "
                              "WHERE B.id IS NULL "
                              "UNION ALL "
                              "SELECT B.id AS id, NULL AS a_name, B.name AS b_name "
                              "FROM B LEFT JOIN A ON B.id = A.id "
                              "WHERE A.id IS NULL "
                              "ORDER BY id");
}

void Database::execute(const std::string& query) {
    char* raw_error = nullptr;
    const int result = sqlite3_exec(m_Database, query.c_str(), nullptr, nullptr, &raw_error);
    if (result == SQLITE_OK) {
        return;
    }

    // LCOV_EXCL_START
    const std::string message = raw_error == nullptr ? sqlite3_errmsg(m_Database) : raw_error;
    sqlite3_free(raw_error);
    throw std::runtime_error("Unable to execute SQL: " + message);
}
// LCOV_EXCL_STOP

std::vector<JoinedRow> Database::select_joined_rows(const std::string& query) {
    sqlite3_stmt* raw_statement = nullptr;
    if (sqlite3_prepare_v2(m_Database, query.c_str(), -1, &raw_statement, nullptr) !=
        SQLITE_OK) {
        throw sqlite_error(m_Database, "Unable to prepare SELECT");  // LCOV_EXCL_LINE
    }
    Statement statement(raw_statement);

    std::vector<JoinedRow> rows;
    int step_result = sqlite3_step(statement.get());
    while (step_result == SQLITE_ROW) {
        rows.push_back(JoinedRow{ sqlite3_column_int(statement.get(), 0),
                                  column_text(statement.get(), 1),
                                  column_text(statement.get(), 2) });
        step_result = sqlite3_step(statement.get());
    }

    if (step_result != SQLITE_DONE) {
        throw sqlite_error(m_Database, "Unable to execute SELECT");  // LCOV_EXCL_LINE
    }

    return rows;
}
