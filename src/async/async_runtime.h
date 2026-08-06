#ifndef ASYNC_RUNTIME_H
#define ASYNC_RUNTIME_H

/**
 * @file async_runtime.h
 * @brief Runtime вывода готовых блоков async.
 */

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <iosfwd>
#include <mutex>
#include <thread>

#include "blocking_queue.h"
#include "command_block.h"

/**
 * @brief Выполняет вывод готовых блоков команд.
 *
 * Владеет очередями и worker-потоками для консольного и файлового вывода.
 * @ingroup async_internal_group
 */
class AsyncRuntime {
public:
    /**
     * @brief Возвращает общий runtime async-библиотеки.
     *
     * @return Ссылка на singleton runtime.
     */
    static AsyncRuntime& instance();

    /**
     * @brief Передаёт готовый блок в подсистему вывода.
     *
     * @param block Готовый блок команд.
     */
    void publish(const CommandBlock& block);

    /**
     * @brief Дожидается обработки всех опубликованных блоков.
     */
    void wait();

private:
    /**
     * @brief Создаёт runtime и запускает worker-потоки.
     */
    AsyncRuntime();

    /**
     * @brief Останавливает очереди и дожидается завершения worker-потоков.
     */
    ~AsyncRuntime();

    AsyncRuntime(const AsyncRuntime&) = delete;
    AsyncRuntime& operator=(const AsyncRuntime&) = delete;

    /**
     * @brief Основной цикл worker-потока консольного вывода.
     */
    void run_console_worker();

    /**
     * @brief Основной цикл worker-потока файлового вывода.
     */
    void run_file_worker();

    /**
     * @brief Записывает блок команд в консольный поток.
     *
     * @param block Готовый блок команд.
     */
    void write_console(const CommandBlock& block);

    /**
     * @brief Записывает блок команд в log-файл.
     *
     * @param block Готовый блок команд.
     */
    void write_file(const CommandBlock& block);

    /**
     * @brief Увеличивает счётчик ожидающих задач вывода.
     *
     * @param count Количество добавляемых задач.
     */
    void add_pending_tasks(std::size_t count);

    /**
     * @brief Отмечает завершение одной задачи вывода.
     */
    void complete_pending_task();

    /// Очередь блоков для консольного вывода.
    BlockingQueue<CommandBlock> m_console_queue;

    /// Очередь блоков для файлового вывода.
    BlockingQueue<CommandBlock> m_file_queue;

    /// Worker консольного вывода.
    std::thread m_log_thread;

    /// Первый worker файлового вывода.
    std::thread m_file_thread1;

    /// Второй worker файлового вывода.
    std::thread m_file_thread2;

    /// Mutex для доступа к потоку консольного вывода.
    mutable std::mutex m_output_mutex;

    /// Поток для консольного вывода.
    std::ostream* m_output;

    /// Mutex для ожидания завершения задач вывода.
    std::mutex m_pending_mutex;

    /// Условная переменная ожидания завершения задач вывода.
    std::condition_variable m_pending_condition;

    /// Количество опубликованных, но ещё не обработанных задач вывода.
    std::size_t m_pending_tasks = 0;

    /// Счётчик для уникальных имён log-файлов.
    std::atomic<std::size_t> m_file_index{ 0 };
};

#endif  // ASYNC_RUNTIME_H
