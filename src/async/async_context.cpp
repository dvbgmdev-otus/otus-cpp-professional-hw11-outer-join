#include "async_context.h"

#include <ctime>
#include <utility>

namespace {

std::time_t current_time() { return std::time(nullptr); }

}  // namespace

AsyncContext::AsyncContext(std::size_t block_size,
                           CommandBlockHandler command_block_handler,
                           Clock clock)
    : AsyncContext(std::make_shared<AsyncCommandProcessor>(
                       block_size, std::move(command_block_handler)),
                   std::move(clock)) {}

AsyncContext::AsyncContext(std::shared_ptr<AsyncCommandProcessor> command_processor, Clock clock)
    : m_command_processor(std::move(command_processor)),
      m_dynamic_processor(
          m_command_processor->block_size(),
          [processor = m_command_processor](const CommandBlock& block) {
              processor->publish_dynamic_block(block);
          }),
      m_clock(std::move(clock)),
      m_pending_line() {
    m_command_processor->attach_context();
}

void AsyncContext::receive(const char* data, std::size_t size) {
    if (m_disconnected || data == nullptr || size == 0) {
        return;
    }

    for (std::size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            process_line(m_pending_line);
            m_pending_line.clear();
            continue;
        }

        m_pending_line.push_back(data[i]);
    }
}

void AsyncContext::disconnect() {
    if (m_disconnected) {
        return;
    }

    if (!m_pending_line.empty()) {
        process_line(m_pending_line);
        m_pending_line.clear();
    }

    if (m_dynamic_depth > 0) {
        m_dynamic_processor.finish();
        m_dynamic_depth = 0;
    }

    m_command_processor->detach_context();
    m_disconnected = true;
}

void AsyncContext::process_line(const std::string& line) {
    if (!m_clock) {
        m_clock = current_time;
    }

    const std::time_t received_at = m_clock();

    if (m_dynamic_depth > 0) {
        m_dynamic_processor.process_command(line, received_at);
        if (line == "{") {
            ++m_dynamic_depth;
        } else if (line == "}") {
            --m_dynamic_depth;
        }
        return;
    }

    if (line == "{") {
        m_command_processor->flush_static_block();
        m_dynamic_depth = 1;
        m_dynamic_processor.process_command(line, received_at);
        return;
    }

    if (line == "}") {
        return;
    }

    m_command_processor->process_static_command(line, received_at);
}
