/**
 * @file bulk_processor_test.cpp
 * @brief Unit-тесты пакетирования команд.
 */

#include "bulk_processor.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

class BulkProcessorTest : public ::testing::Test {
protected:
    void save_block(const CommandBlock& block) { blocks.push_back(block); }

    std::vector<CommandBlock> blocks;
};

#if (1)  // 1. Статические блоки

// 1.1 Статический блок завершается при достижении заданного размера.
TEST_F(BulkProcessorTest, StaticBlock_WhenSizeReached_EmitsBlock) {
    BulkProcessor processor(3, [this](const CommandBlock& block) { save_block(block); });
    processor.process_command("cmd1", 100);
    processor.process_command("cmd2", 101);
    processor.process_command("cmd3", 102);
    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2", "cmd3" }), blocks[0].commands);
    EXPECT_EQ(100, blocks[0].timestamp);
}

// 1.2 Конец ввода принудительно завершает неполный статический блок.
TEST_F(BulkProcessorTest, StaticBlock_WhenFinish_EmitsIncompleteBlock) {
    BulkProcessor processor(3, [this](const CommandBlock& block) { save_block(block); });
    processor.process_command("cmd1", 100);
    processor.process_command("cmd2", 101);
    processor.finish();
    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2" }), blocks[0].commands);
    EXPECT_EQ(100, blocks[0].timestamp);
}

// 1.3 Начало динамического блока завершает накопленный статический блок.
TEST_F(BulkProcessorTest, StaticBlock_WhenDynamicBlockStarts_EmitsIncompleteBlock) {
    BulkProcessor processor(3, [this](const CommandBlock& block) { save_block(block); });
    processor.process_command("cmd1", 100);
    processor.process_command("cmd2", 101);
    processor.process_command("{", 102);
    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2" }), blocks[0].commands);
    EXPECT_EQ(100, blocks[0].timestamp);
}

#endif

#if (1)  // 2. Динамические блоки

// 2.1 Динамический блок завершается только закрывающей скобкой.
TEST_F(BulkProcessorTest, DynamicBlock_WhenClosed_EmitsBlock) {
    BulkProcessor processor(3, [this](const CommandBlock& block) { save_block(block); });
    processor.process_command("{", 100);
    processor.process_command("cmd1", 101);
    processor.process_command("cmd2", 102);
    processor.process_command("cmd3", 103);
    processor.process_command("cmd4", 104);
    processor.process_command("}", 105);
    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2", "cmd3", "cmd4" }), blocks[0].commands);
    EXPECT_EQ(101, blocks[0].timestamp);
}

// 2.2 Вложенные служебные команды не попадают в блок как команды.
TEST_F(BulkProcessorTest, DynamicBlock_WhenNested_EmitsOuterBlockOnly) {
    BulkProcessor processor(3, [this](const CommandBlock& block) { save_block(block); });
    processor.process_command("{", 100);
    processor.process_command("cmd1", 101);
    processor.process_command("{", 102);
    processor.process_command("cmd2", 103);
    processor.process_command("}", 104);
    processor.process_command("cmd3", 105);
    processor.process_command("}", 106);
    ASSERT_EQ(1U, blocks.size());
    EXPECT_EQ((std::vector<std::string>{ "cmd1", "cmd2", "cmd3" }), blocks[0].commands);
    EXPECT_EQ(101, blocks[0].timestamp);
}

// 2.3 Конец ввода внутри динамического блока игнорирует весь блок.
TEST_F(BulkProcessorTest, DynamicBlock_WhenFinishBeforeClose_DropsBlock) {
    BulkProcessor processor(3, [this](const CommandBlock& block) { save_block(block); });
    processor.process_command("{", 100);
    processor.process_command("cmd1", 101);
    processor.process_command("cmd2", 102);
    processor.finish();
    EXPECT_TRUE(blocks.empty());
}

#endif
