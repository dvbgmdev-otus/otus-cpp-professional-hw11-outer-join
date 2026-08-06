#ifndef COMMAND_BLOCK_H
#define COMMAND_BLOCK_H

/**
 * @file command_block.h
 * @brief Модель готового блока команд.
 */

#include <ctime>
#include <string>
#include <vector>

/**
 * @brief Готовый блок команд.
 * @ingroup model_group
 *
 * Хранит команды одного сформированного блока и время получения первой команды
 * этого блока. Timestamp используется при формировании имени log-файла.
 */
struct CommandBlock {
    /// Команды, входящие в блок, в порядке получения из входного потока.
    std::vector<std::string> commands;

    /// Время получения первой команды блока.
    std::time_t timestamp{ 0 };
};

#endif  // COMMAND_BLOCK_H
