#ifndef DATABASE_H
#define DATABASE_H

/**
 * @file database.h
 * @brief Хранилище таблиц A и B в базе данных SQLite.
 */

#include <string>
#include <vector>

struct sqlite3;

/**
 * @brief Таблица данных, доступная через протокол join_server.
 * @ingroup database_group
 */
enum class Table {
    A,  ///< Таблица A.
    B   ///< Таблица B.
};

/**
 * @brief Результат добавления записи в таблицу.
 * @ingroup database_group
 */
enum class InsertResult {
    Inserted,  ///< Запись добавлена.
    Duplicate  ///< Идентификатор уже существует в выбранной таблице.
};

/**
 * @brief Строка результата объединения таблиц A и B.
 * @ingroup database_group
 */
struct JoinedRow {
    int id;              ///< Общий идентификатор строки.
    std::string a_name;  ///< Имя из таблицы A либо пустая строка.
    std::string b_name;  ///< Имя из таблицы B либо пустая строка.
};

/**
 * @brief Управляет in-memory базой SQLite с таблицами A и B.
 * @ingroup database_group
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
     * @param table Таблица A или B.
     * @param id Первичный ключ записи.
     * @param name Имя записи.
     * @return Inserted либо Duplicate, если идентификатор уже существует.
     */
    InsertResult insert(Table table, int id, const std::string& name);

    /**
     * @brief Удаляет все записи из выбранной таблицы.
     * @param table Таблица A или B.
     */
    void truncate(Table table);

    /**
     * @brief Формирует пересечение таблиц A и B.
     * @return Строки с идентификаторами, присутствующими в обеих таблицах.
     */
    std::vector<JoinedRow> intersection();

    /**
     * @brief Формирует симметрическую разность таблиц A и B.
     * @return Строки с идентификаторами, присутствующими только в одной таблице.
     */
    std::vector<JoinedRow> symmetric_difference();

private:
    /**
     * @brief Выполняет SQL-запрос, который не возвращает строки.
     * @param query Текст SQL-запроса.
     */
    void execute(const std::string& query);

    /**
     * @brief Выполняет SQL-запрос объединения таблиц.
     * @param query Текст SQL-запроса.
     * @return Строки результата запроса.
     */
    std::vector<JoinedRow> select_joined_rows(const std::string& query);

    /// Соединение с SQLite.
    sqlite3* m_Database = nullptr;
};

#endif  // DATABASE_H
