/**
 * @file async_command_processor_test.cpp
 * @brief Unit-тесты общего обработчика async-подключений.
 */

#include "async_command_processor.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#if (1)  // Part 1. Управление контекстами

// Test 1.1. Отключение без активных контекстов не изменяет состояние processor-а.
TEST(AsyncCommandProcessorTest, DetachContext_WhenNoContextsAttached_KeepsProcessorUsable) {
    std::vector<CommandBlock> blocks;
    AsyncCommandProcessor processor(
        3, [&blocks](const CommandBlock& block) { blocks.push_back(block); });

    processor.detach_context();
    EXPECT_TRUE(blocks.empty());

    processor.attach_context();
    processor.process_static_command("cmd1", 100);
    processor.detach_context();

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), blocks[0].commands);
}

#endif  // Part 1. Управление контекстами
