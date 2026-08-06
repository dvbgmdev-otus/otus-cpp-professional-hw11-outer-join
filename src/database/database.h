#ifndef DATABASE_H
#define DATABASE_H

/**
 * @file database.h
 * @brief Хранилище таблиц A и B в базе данных SQLite.
 */

#include <string>
#include <vector>

struct sqlite3;

namespace database {

/// Таблица данных, доступная через протокол join_server.
enum class Table { A, B };

/// Результат добавления записи в таблицу.
enum class InsertResult { Inserted, Duplicate };

/// Строка результата объединения таблиц A и B.
struct JoinedRow {
    int id;
    std::string a_name;
    std::string b_name;
};

/**
 * @brief Управляет in-memory базой SQLite с таблицами A и B.
 */
class Database {
public:
    /// Открывает базу в памяти и создаёт пустые таблицы A и B.
    Database();

    /// Закрывает соединение с базой данных.
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /**
     * @brief Добавляет запись в выбранную таблицу.
     * @return Inserted либо Duplicate, если идентификатор уже существует.
     */
    InsertResult insert(Table table, int id, const std::string& name);

    /// Удаляет все записи из выбранной таблицы.
    void truncate(Table table);

    /// Возвращает строки с идентификаторами, присутствующими в обеих таблицах.
    std::vector<JoinedRow> intersection();

    /// Возвращает строки с идентификаторами, присутствующими только в одной таблице.
    std::vector<JoinedRow> symmetric_difference();

private:
    /// Выполняет SQL-запрос, который не возвращает строки.
    void execute(const std::string& query);

    /// Выполняет запрос объединения и возвращает его строки.
    std::vector<JoinedRow> select_joined_rows(const std::string& query);

    /// Соединение с SQLite.
    sqlite3* m_Database = nullptr;
};

}  // namespace database

#endif  // DATABASE_H
