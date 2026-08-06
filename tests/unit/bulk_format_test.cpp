/**
 * @file bulk_format_test.cpp
 * @brief Unit-тесты форматирования блока команд.
 */

#include "bulk_format.h"

#include <gtest/gtest.h>

#if (1)  // 1. Форматирование блоков команд

// 1.1 Блок из одной команды форматируется без разделителей.
TEST(BulkFormatTest, SingleCommand_ReturnsBulkLine) {
    const CommandBlock block{{"cmd1"}, 123};
    EXPECT_EQ("bulk: cmd1", format_bulk(block));
}

// 1.2 Блок из нескольких команд форматируется с разделителями-запятыми.
TEST(BulkFormatTest, MultipleCommands_ReturnsCommaSeparatedBulkLine) {
    const CommandBlock block{{"cmd1", "cmd2", "cmd3"}, 123};
    EXPECT_EQ("bulk: cmd1, cmd2, cmd3", format_bulk(block));
}

// 1.3 Пустой блок сохраняет только префикс bulk.
TEST(BulkFormatTest, EmptyCommands_ReturnsBulkPrefixOnly) {
    const CommandBlock block{{}, 123};
    EXPECT_EQ("bulk: ", format_bulk(block));
}

#endif  // Форматирование блоков команд
