/**
 * @file command_reader_test.cpp
 * @brief Unit-тесты построчного чтения команд.
 */

#include "command_reader.h"

#include <gtest/gtest.h>

#include <ctime>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class CommandReaderTest : public ::testing::Test {
protected:
    using ReceivedCommand = std::pair<std::string, std::time_t>;

    std::time_t next_time() { return 100 + static_cast<std::time_t>(commands.size()); }

    void save_command(const std::string& command, std::time_t received_at) {
        commands.push_back(ReceivedCommand{ command, received_at });
        events.push_back("command");
    }

    void save_finish() {
        ++finish_count;
        events.push_back("finish");
    }

    std::vector<ReceivedCommand> commands;
    std::vector<std::string> events;
    std::size_t finish_count = 0;
};

#if (1)  // 1. Построчное чтение команд

// 1.1 Все строки входного потока передаются как команды.
TEST_F(CommandReaderTest, ReadAll_WhenInputHasLines_EmitsAllCommands) {
    std::istringstream input("cmd1\ncmd2\ncmd3\n");
    CommandReader reader(
        input,
        [this](const std::string& command, std::time_t received_at) {
            save_command(command, received_at);
        },
        [this]() { save_finish(); },
        [this]() { return next_time(); });
    reader.read_all();
    EXPECT_EQ((std::vector<ReceivedCommand>{
                  { "cmd1", 100 },
                  { "cmd2", 101 },
                  { "cmd3", 102 },
              }),
              commands);
}

// 1.2 Пустая строка входного потока передается как обычная команда.
TEST_F(CommandReaderTest, ReadAll_WhenInputHasEmptyLine_EmitsEmptyCommand) {
    std::istringstream input("cmd1\n\ncmd2\n");
    CommandReader reader(
        input,
        [this](const std::string& command, std::time_t received_at) {
            save_command(command, received_at);
        },
        [this]() { save_finish(); },
        [this]() { return next_time(); });
    reader.read_all();
    EXPECT_EQ((std::vector<ReceivedCommand>{
                  { "cmd1", 100 },
                  { "", 101 },
                  { "cmd2", 102 },
              }),
              commands);
}

// 1.3 После конца входного потока вызывается обработчик завершения.
TEST_F(CommandReaderTest, ReadAll_WhenInputEnds_EmitsFinishAfterCommands) {
    std::istringstream input("cmd1\n");
    CommandReader reader(
        input,
        [this](const std::string& command, std::time_t received_at) {
            save_command(command, received_at);
        },
        [this]() { save_finish(); },
        [this]() { return next_time(); });
    reader.read_all();
    EXPECT_EQ(1U, finish_count);
    EXPECT_EQ((std::vector<std::string>{ "command", "finish" }), events);
}

// 1.4 Для пустого входного потока вызывается только обработчик завершения.
TEST_F(CommandReaderTest, ReadAll_WhenInputIsEmpty_EmitsOnlyFinish) {
    std::istringstream input;
    CommandReader reader(
        input,
        [this](const std::string& command, std::time_t received_at) {
            save_command(command, received_at);
        },
        [this]() { save_finish(); },
        [this]() { return next_time(); });
    reader.read_all();
    EXPECT_TRUE(commands.empty());
    EXPECT_EQ(1U, finish_count);
    EXPECT_EQ((std::vector<std::string>{ "finish" }), events);
}

#endif

#if (1)  // 2. Время получения команд

// 2.1 Конструктор по умолчанию использует текущее системное время.
TEST_F(CommandReaderTest, ReadAll_WhenDefaultClockUsed_EmitsCurrentTimestamp) {
    std::istringstream input("cmd1\n");
    const std::time_t before = std::time(nullptr);
    CommandReader reader(
        input,
        [this](const std::string& command, std::time_t received_at) {
            save_command(command, received_at);
        },
        [this]() { save_finish(); });
    reader.read_all();
    const std::time_t after = std::time(nullptr);
    ASSERT_EQ(1U, commands.size());
    EXPECT_EQ("cmd1", commands[0].first);
    EXPECT_LE(before, commands[0].second);
    EXPECT_LE(commands[0].second, after);
}

#endif
