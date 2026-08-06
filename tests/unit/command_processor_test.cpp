/**
 * @file command_processor_test.cpp
 * @brief Unit-тесты текстовых команд изменения базы данных.
 */

#include "command_processor.h"

#include <gtest/gtest.h>

#include "database.h"

class CommandProcessorTest : public ::testing::Test {
protected:
    CommandProcessorTest() : processor(test_database) {}

    database::Database test_database;
    CommandProcessor processor;
};

#if (1)  // Part 1. Команда INSERT

// Test 1.1. Запись добавляется в таблицу A.
TEST_F(CommandProcessorTest, Insert_WhenTableAAndIdAreValid_ReturnsOk) {
    EXPECT_EQ("OK\n", processor.process("INSERT A 1 sweater"));
}

// Test 1.2. Запись добавляется в таблицу B.
TEST_F(CommandProcessorTest, Insert_WhenTableBAndIdAreValid_ReturnsOk) {
    EXPECT_EQ("OK\n", processor.process("INSERT B 6 flour"));
}

// Test 1.3. Повторный идентификатор возвращает предусмотренную протоколом ошибку.
TEST_F(CommandProcessorTest, Insert_WhenIdAlreadyExists_ReturnsDuplicateError) {
    ASSERT_EQ("OK\n", processor.process("INSERT A 1 sweater"));

    EXPECT_EQ("ERR duplicate 1\n", processor.process("INSERT A 1 understand"));
}

#endif  // Part 1. Команда INSERT

#if (1)  // Part 2. Команда TRUNCATE

// Test 2.1. Очистка таблицы A позволяет повторно добавить идентификатор.
TEST_F(CommandProcessorTest, Truncate_WhenTableAHasRows_ClearsTable) {
    ASSERT_EQ("OK\n", processor.process("INSERT A 1 sweater"));

    EXPECT_EQ("OK\n", processor.process("TRUNCATE A"));
    EXPECT_EQ("OK\n", processor.process("INSERT A 1 understand"));
}

// Test 2.2. Очистка таблицы B позволяет повторно добавить идентификатор.
TEST_F(CommandProcessorTest, Truncate_WhenTableBHasRows_ClearsTable) {
    ASSERT_EQ("OK\n", processor.process("INSERT B 6 flour"));

    EXPECT_EQ("OK\n", processor.process("TRUNCATE B"));
    EXPECT_EQ("OK\n", processor.process("INSERT B 6 wonder"));
}

#endif  // Part 2. Команда TRUNCATE

#if (1)  // Part 3. Команды выборки

// Test 3.1. Пересечение пустых таблиц возвращает только признак успеха.
TEST_F(CommandProcessorTest, Intersection_WhenTablesAreEmpty_ReturnsOk) {
    EXPECT_EQ("OK\n", processor.process("INTERSECTION"));
}

// Test 3.2. Пересечение возвращает общие строки в формате протокола.
TEST_F(CommandProcessorTest, Intersection_WhenTablesOverlap_ReturnsRowsAndOk) {
    ASSERT_EQ("OK\n", processor.process("INSERT A 4 quality"));
    ASSERT_EQ("OK\n", processor.process("INSERT A 2 frank"));
    ASSERT_EQ("OK\n", processor.process("INSERT A 3 violation"));
    ASSERT_EQ("OK\n", processor.process("INSERT B 6 flour"));
    ASSERT_EQ("OK\n", processor.process("INSERT B 4 example"));
    ASSERT_EQ("OK\n", processor.process("INSERT B 3 proposal"));

    EXPECT_EQ("3,violation,proposal\n4,quality,example\nOK\n",
              processor.process("INTERSECTION"));
}

// Test 3.3. Симметрическая разность пустых таблиц возвращает только признак успеха.
TEST_F(CommandProcessorTest, SymmetricDifference_WhenTablesAreEmpty_ReturnsOk) {
    EXPECT_EQ("OK\n", processor.process("SYMMETRIC_DIFFERENCE"));
}

// Test 3.4. Симметрическая разность возвращает уникальные строки с пустыми полями.
TEST_F(CommandProcessorTest, SymmetricDifference_WhenTablesOverlap_ReturnsRowsAndOk) {
    ASSERT_EQ("OK\n", processor.process("INSERT B 6 flour"));
    ASSERT_EQ("OK\n", processor.process("INSERT A 3 violation"));
    ASSERT_EQ("OK\n", processor.process("INSERT A 0 lean"));
    ASSERT_EQ("OK\n", processor.process("INSERT B 3 proposal"));

    EXPECT_EQ("0,lean,\n6,,flour\nOK\n",
              processor.process("SYMMETRIC_DIFFERENCE"));
}

#endif  // Part 3. Команды выборки
