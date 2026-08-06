#ifndef ASYNC_COMMAND_PROCESSOR_H
#define ASYNC_COMMAND_PROCESSOR_H

/**
 * @file async_command_processor.h
 * @brief Общая обработка статических команд подключений async.
 */

#include <cstddef>
#include <ctime>
#include <functional>
#include <mutex>
#include <string>

#include "bulk_processor.h"
#include "command_block.h"

/**
 * @brief Координирует общие статические блоки нескольких подключений.
 * @ingroup async_internal_group
 */
class AsyncCommandProcessor {
public:
    /// Обработчик готового блока команд.
    using CommandBlockHandler = std::function<void(const CommandBlock&)>;

    /**
     * @brief Создаёт processor общих статических команд.
     *
     * @param block_size Размер статического блока команд.
     * @param command_block_handler Обработчик готовых блоков команд.
     */
    AsyncCommandProcessor(std::size_t block_size, CommandBlockHandler command_block_handler);

    /// Регистрирует новое подключение.
    void attach_context();

    /**
     * @brief Обрабатывает статическую команду в общем блоке.
     *
     * @param command Текст команды.
     * @param received_at Время получения команды.
     */
    void process_static_command(const std::string& command, std::time_t received_at);

    /// Завершает текущий неполный статический блок.
    void flush_static_block();

    /**
     * @brief Публикует готовый динамический блок отдельного подключения.
     *
     * @param block Готовый динамический блок команд.
     */
    void publish_dynamic_block(const CommandBlock& block);

    /// Удаляет подключение и завершает статический блок после последнего клиента.
    void detach_context();

    /**
     * @brief Возвращает размер статического блока.
     *
     * @return Настроенный размер статического блока команд.
     */
    std::size_t block_size() const;

private:
    /// Синхронизирует команды и жизненный цикл подключений.
    mutable std::mutex m_mutex;

    /// Обработчик всех готовых блоков.
    CommandBlockHandler m_handler;

    /// Общий processor статических команд.
    BulkProcessor m_static_processor;

    /// Количество активных подключений.
    std::size_t m_context_count = 0;

    /// Размер статического блока.
    std::size_t m_block_size;
};

#endif  // ASYNC_COMMAND_PROCESSOR_H
