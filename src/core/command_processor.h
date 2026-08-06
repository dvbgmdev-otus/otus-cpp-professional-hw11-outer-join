#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

/**
 * @file command_processor.h
 * @brief Обработчик текстовых команд join_server.
 */

#include <string>

namespace database {
class Database;
}

/**
 * @brief Преобразует команды протокола в операции над общей базой данных.
 */
class CommandProcessor {
public:
    /// Создаёт обработчик команд для указанной базы данных.
    explicit CommandProcessor(database::Database& database);

    /// Выполняет одну команду без завершающего символа перевода строки.
    std::string process(const std::string& command);

private:
    /// Общая база данных сервера.
    database::Database& m_Database;
};

#endif  // COMMAND_PROCESSOR_H
