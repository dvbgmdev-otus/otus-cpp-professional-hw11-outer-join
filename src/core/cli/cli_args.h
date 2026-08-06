#ifndef CLI_ARGS_H
#define CLI_ARGS_H

/**
 * @file cli_args.h
 * @brief Разбор аргументов командной строки.
 */

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Результат разбора аргументов командной строки.
 * @ingroup cli_group
 */
struct CliArgsParseResult {
    /// Признак успешного разбора аргументов.
    bool success = false;

    /// TCP-порт для входящих соединений.
    std::uint16_t port = 0;

    /// Размер статического блока команд.
    std::size_t block_size = 0;

    /// Текст ошибки разбора. Заполнен, если success равен false.
    std::string error;
};

/**
 * @brief Разбирает аргументы командной строки приложения.
 * @ingroup cli_group
 *
 * Ожидает TCP-порт в диапазоне 1..65535 и положительное целое число,
 * задающее размер статического блока команд.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return Результат разбора аргументов.
 */
CliArgsParseResult parse_cli_args(int argc, const char* const argv[]);

#endif  // CLI_ARGS_H
