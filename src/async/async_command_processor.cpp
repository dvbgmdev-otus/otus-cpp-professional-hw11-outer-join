#include "async_command_processor.h"

#include <utility>

AsyncCommandProcessor::AsyncCommandProcessor(std::size_t block_size,
                                             CommandBlockHandler command_block_handler)
    : m_handler(std::move(command_block_handler)),
      m_static_processor(block_size, m_handler),
      m_block_size(block_size) {}

void AsyncCommandProcessor::attach_context() {
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_context_count;
}

void AsyncCommandProcessor::process_static_command(const std::string& command,
                                                   std::time_t received_at) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_static_processor.process_command(command, received_at);
}

void AsyncCommandProcessor::flush_static_block() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_static_processor.finish();
}

void AsyncCommandProcessor::publish_dynamic_block(const CommandBlock& block) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_handler(block);
}

void AsyncCommandProcessor::detach_context() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_context_count == 0) {
        return;
    }

    --m_context_count;
    if (m_context_count == 0) {
        m_static_processor.finish();
    }
}

std::size_t AsyncCommandProcessor::block_size() const { return m_block_size; }
