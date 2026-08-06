#include "bulk_processor.h"

#include <utility>

BulkProcessor::BulkProcessor(std::size_t block_size, CommandBlockHandler command_block_handler)
    : m_block_size(block_size),
      m_handler(std::move(command_block_handler)),
      m_block{ {}, 0 },
      m_dynamic_depth(0) {}

void BulkProcessor::process_command(const std::string& command, std::time_t received_at) {
    if (command == "{") {
        if (!is_dynamic_block_active()) {
            flush_current_block();
        }
        ++m_dynamic_depth;
        return;
    }

    if (command == "}") {
        if (is_dynamic_block_active()) {
            --m_dynamic_depth;
            if (!is_dynamic_block_active()) {
                flush_current_block();
            }
        }
        return;
    }

    append_command_to_current_block(command, received_at);

    if (!is_dynamic_block_active() && m_block.commands.size() == m_block_size) {
        flush_current_block();
    }
}

void BulkProcessor::finish() {
    if (is_dynamic_block_active()) {
        reset_current_block();
        m_dynamic_depth = 0;
        return;
    }
    flush_current_block();
}

bool BulkProcessor::is_dynamic_block_active() const { return m_dynamic_depth > 0; }

void BulkProcessor::append_command_to_current_block(const std::string& command,
                                                    std::time_t received_at) {
    if (m_block.commands.empty()) {
        m_block.timestamp = received_at;
    }
    m_block.commands.push_back(command);
}

void BulkProcessor::flush_current_block() {
    if (m_block.commands.empty()) {
        return;
    }
    m_handler(m_block);
    reset_current_block();
}

void BulkProcessor::reset_current_block() {
    m_block.commands.clear();
    m_block.timestamp = 0;
}
