#include "command_reader.h"

#include <ctime>
#include <utility>

namespace {

std::time_t current_time() { return std::time(nullptr); }

}  // namespace

CommandReader::CommandReader(std::istream& input,
                             CommandHandler command_handler,
                             FinishHandler finish_handler,
                             Clock clock)
    : m_input(input),
      m_command_handler(std::move(command_handler)),
      m_finish_handler(std::move(finish_handler)),
      m_clock(std::move(clock)) {
    if (!m_clock) {
        m_clock = current_time;
    }
}

void CommandReader::read_all() {
    std::string command;
    while (std::getline(m_input, command)) {
        m_command_handler(command, m_clock());
    }
    m_finish_handler();
}
