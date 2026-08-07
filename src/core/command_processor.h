#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

/**
 * @file command_processor.h
 * @brief Обработчик текстовых команд join_server.
 */

#include <string>

class Database;

/**
 * @brief Преобразует команды протокола в операции над общей базой данных.
 * @ingroup command_group
 */
class CommandProcessor {
public:
    /// Создаёт обработчик команд для указанной базы данных.
    explicit CommandProcessor(Database& database);

    /// Выполняет одну команду без завершающего символа перевода строки.
    std::string process(const std::string& command);

private:
    /// Общая база данных сервера.
    Database& m_Database;
};

#endif  // COMMAND_PROCESSOR_H
