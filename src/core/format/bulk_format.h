#ifndef BULK_FORMAT_H
#define BULK_FORMAT_H

/**
 * @file bulk_format.h
 * @brief Форматирование готовых блоков команд.
 */

#include <string>

#include "command_block.h"

/**
 * @brief Форматирует блок команд для вывода.
 * @ingroup format_group
 *
 * Возвращает строку вида `bulk: cmd1, cmd2, cmd3`. Если блок не содержит
 * команд, возвращается строка с одним префиксом `bulk: `.
 *
 * @param block Готовый блок команд.
 * @return Строковое представление блока в формате bulk.
 */
std::string format_bulk(const CommandBlock& block);

#endif  // BULK_FORMAT_H
