#include "async_runtime.h"

#include <iostream>

#include "bulk_writer.h"

AsyncRuntime& AsyncRuntime::instance() {
    static AsyncRuntime runtime;
    return runtime;
}

AsyncRuntime::AsyncRuntime() : m_output(&std::cout) {
    m_log_thread = std::thread(&AsyncRuntime::run_console_worker, this);
    m_file_thread1 = std::thread(&AsyncRuntime::run_file_worker, this);
    m_file_thread2 = std::thread(&AsyncRuntime::run_file_worker, this);
}

AsyncRuntime::~AsyncRuntime() {
    m_console_queue.close();
    m_file_queue.close();

    wait();

    if (m_log_thread.joinable()) {
        m_log_thread.join();
    }
    if (m_file_thread1.joinable()) {
        m_file_thread1.join();
    }
    if (m_file_thread2.joinable()) {
        m_file_thread2.join();
    }
}

void AsyncRuntime::publish(const CommandBlock& block) {
    add_pending_tasks(2);

    m_console_queue.push(block);
    m_file_queue.push(block);
}

void AsyncRuntime::wait() {
    std::unique_lock<std::mutex> lock(m_pending_mutex);
    m_pending_condition.wait(lock, [this]() { return m_pending_tasks == 0; });
}

void AsyncRuntime::run_console_worker() {
    CommandBlock block;
    while (m_console_queue.pop(block)) {
        write_console(block);
        complete_pending_task();
    }
}

void AsyncRuntime::run_file_worker() {
    CommandBlock block;
    while (m_file_queue.pop(block)) {
        write_file(block);
        complete_pending_task();
    }
}

void AsyncRuntime::write_console(const CommandBlock& block) {
    std::lock_guard<std::mutex> lock(m_output_mutex);
    ConsoleBulkWriter writer(*m_output);
    writer.write(block);
}

void AsyncRuntime::write_file(const CommandBlock& block) {
    const std::size_t file_index = ++m_file_index;
    FileBulkWriter writer;
    writer.write(block, file_index);
}

void AsyncRuntime::add_pending_tasks(std::size_t count) {
    std::lock_guard<std::mutex> lock(m_pending_mutex);
    m_pending_tasks += count;
}

void AsyncRuntime::complete_pending_task() {
    std::unique_lock<std::mutex> lock(m_pending_mutex);
    --m_pending_tasks;
    lock.unlock();

    m_pending_condition.notify_all();
}
