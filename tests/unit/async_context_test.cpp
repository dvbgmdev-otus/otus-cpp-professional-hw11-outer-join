/**
 * @file async_context_test.cpp
 * @brief Unit-тесты контекста async-подключения.
 */

#include "async_context.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

class AsyncContextTest : public ::testing::Test {
protected:
    void save_block(const CommandBlock& block) { blocks.push_back(block); }

    std::vector<CommandBlock> blocks;
};

#if (1)  // 1. Разбор входного буфера

// 1.1 Полная строка из receive передаётся в processor как команда.
TEST_F(AsyncContextTest, Receive_WhenSingleLinePassed_EmitsCommandBlock) {
    AsyncContext context(1, [this](const CommandBlock& block) { save_block(block); });
    context.receive("cmd1\n", 5);

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), blocks[0].commands);
}

// 1.2 Несколько строк из одного буфера передаются в processor по порядку.
TEST_F(AsyncContextTest, Receive_WhenSeveralLinesPassed_EmitsStaticBlock) {
    AsyncContext context(3, [this](const CommandBlock& block) { save_block(block); });
    context.receive("cmd1\ncmd2\ncmd3\n", 15);

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2", "cmd3" }), blocks[0].commands);
}

// 1.3 Строка, переданная частями, склеивается до передачи в processor.
TEST_F(AsyncContextTest, Receive_WhenLineSplitBetweenCalls_EmitsMergedCommand) {
    AsyncContext context(1, [this](const CommandBlock& block) { save_block(block); });
    context.receive("cm", 2);
    context.receive("d1\n", 3);

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), blocks[0].commands);
}

// 1.4 disconnect завершает последнюю строку без перевода строки.
TEST_F(AsyncContextTest, Disconnect_WhenPendingLineExists_EmitsPendingCommand) {
    AsyncContext context(3, [this](const CommandBlock& block) { save_block(block); });
    context.receive("cmd1", 4);
    context.disconnect();

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), blocks[0].commands);
}

// 1.5 Пустой буфер и nullptr не передают команды в processor.
TEST_F(AsyncContextTest, Receive_WhenInputIsEmpty_DoesNotEmitBlock) {
    AsyncContext context(1, [this](const CommandBlock& block) { save_block(block); });
    context.receive(nullptr, 3);
    context.receive("cmd1\n", 0);
    context.disconnect();

    EXPECT_TRUE(blocks.empty());
}

#endif

#if (1)  // 2. Независимость контекстов

// 2.1 Разные контексты не смешивают незавершённые строки.
TEST_F(AsyncContextTest, Receive_WhenSeveralContextsUsed_KeepsPendingLinesSeparate) {
    std::vector<CommandBlock> first_blocks;
    std::vector<CommandBlock> second_blocks;

    AsyncContext first_context(
        1, [&first_blocks](const CommandBlock& block) { first_blocks.push_back(block); });
    AsyncContext second_context(
        1, [&second_blocks](const CommandBlock& block) { second_blocks.push_back(block); });

    first_context.receive("cm", 2);
    second_context.receive("ab", 2);
    first_context.receive("d1\n", 3);
    second_context.receive("c\n", 2);

    ASSERT_EQ(1U, first_blocks.size());
    ASSERT_EQ(1U, second_blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), first_blocks[0].commands);
    EXPECT_EQ((std::vector<std::string>{ "abc" }), second_blocks[0].commands);
}

#endif

#if (1)  // Part 3. Граничные состояния контекста

// Test 3.1. Повторный disconnect не публикует незавершённый блок повторно.
TEST_F(AsyncContextTest, Disconnect_WhenCalledTwice_EmitsPendingBlockOnce) {
    AsyncContext context(3, [this](const CommandBlock& block) { save_block(block); });
    context.receive("cmd1", 4);

    context.disconnect();
    context.disconnect();

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), blocks[0].commands);
}

// Test 3.2. Вложенный динамический блок завершается только внешней закрывающей скобкой.
TEST_F(AsyncContextTest, Receive_WhenDynamicBlockNested_EmitsBlockAfterOuterBrace) {
    AsyncContext context(3, [this](const CommandBlock& block) { save_block(block); });
    const std::string input = "{\ncmd1\n{\ncmd2\n}\ncmd3\n}\n";

    context.receive(input.data(), input.size());

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2", "cmd3" }), blocks[0].commands);
}

// Test 3.3. Закрывающая скобка вне динамического блока не считается командой.
TEST_F(AsyncContextTest, Receive_WhenClosingBraceWithoutDynamicBlock_DoesNotEmitCommand) {
    AsyncContext context(1, [this](const CommandBlock& block) { save_block(block); });
    const std::string input = "}\ncmd1\n";

    context.receive(input.data(), input.size());

    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1" }), blocks[0].commands);
}

#endif  // Part 3. Граничные состояния контекста
