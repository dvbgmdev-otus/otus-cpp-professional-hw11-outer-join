#ifndef BULK_PROCESSOR_H
#define BULK_PROCESSOR_H

/**
 * @file bulk_processor.h
 * @brief Формирование статических и динамических блоков команд.
 */

#include <cstddef>
#include <ctime>
#include <functional>
#include <string>

#include "command_block.h"

/**
 * @brief Формирует блоки команд по правилам задания.
 * @ingroup processor_group
 *
 * BulkProcessor принимает команды по одной, накапливает их в текущем блоке и
 * передаёт готовые блоки во внешний обработчик. В статическом режиме блок
 * завершается после накопления block_size команд. Строка `{` начинает
 * динамический блок и досрочно завершает текущий статический блок. Строка `}`
 * закрывает динамический блок. Если вход заканчивается внутри незакрытого
 * динамического блока, накопленные команды этого блока отбрасываются.
 */
class BulkProcessor {
public:
    /// Обработчик готового блока команд.
    using CommandBlockHandler = std::function<void(const CommandBlock&)>;

    /**
     * @brief Создаёт processor для блоков заданного размера.
     *
     * @param block_size Размер статического блока команд.
     * @param command_block_handler Обработчик готовых блоков команд.
     */
    BulkProcessor(std::size_t block_size, CommandBlockHandler command_block_handler);

    /**
     * @brief Обрабатывает очередную строку входного потока.
     *
     * @param command Прочитанная команда или служебная строка `{` / `}`.
     * @param received_at Время получения команды.
     */
    void process_command(const std::string& command, std::time_t received_at);

    /**
     * @brief Завершает обработку входного потока.
     *
     * В статическом режиме выводит неполный накопленный блок. Если активен
     * незакрытый динамический блок, отбрасывает накопленные команды.
     */
    void finish();

private:
    /**
     * @brief Проверяет, находится ли processor внутри динамического блока.
     *
     * @return true, если глубина динамических блоков больше нуля.
     */
    bool is_dynamic_block_active() const;

    /**
     * @brief Добавляет обычную команду в текущий блок.
     *
     * @param command Обычная команда без служебного значения.
     * @param received_at Время получения команды.
     */
    void append_command_to_current_block(const std::string& command, std::time_t received_at);

    /// Передаёт текущий непустой блок обработчику и сбрасывает накопление.
    void flush_current_block();

    /// Очищает текущий блок и сбрасывает timestamp.
    void reset_current_block();

    /// Размер статического блока команд.
    std::size_t m_block_size;

    /// Обработчик готовых блоков.
    CommandBlockHandler m_handler;

    /// Текущий накапливаемый блок команд.
    CommandBlock m_block;

    /// Текущая глубина вложенности динамических блоков.
    std::size_t m_dynamic_depth;
};

#endif  // BULK_PROCESSOR_H
