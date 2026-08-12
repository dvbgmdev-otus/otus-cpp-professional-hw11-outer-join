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

    Database test_database;
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

// Test 1.4. Имя с пробелами целиком сохраняется в таблице.
TEST_F(CommandProcessorTest, Insert_WhenNameContainsSpaces_StoresEntireName) {
    ASSERT_EQ("OK\n", processor.process("INSERT A 1 John Doe"));
    ASSERT_EQ("OK\n", processor.process("INSERT B 1 Jane Doe"));
    EXPECT_EQ("1,John Doe,Jane Doe\nOK\n", processor.process("INTERSECTION"));
}

#endif
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

#endif

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
    EXPECT_EQ("3,violation,proposal\n4,quality,example\nOK\n", processor.process("INTERSECTION"));
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
    EXPECT_EQ("0,lean,\n6,,flour\nOK\n", processor.process("SYMMETRIC_DIFFERENCE"));
}

#endif

#if (1)  // Part 4. Неизвестные команды и таблицы

// Test 4.1. Пустая строка возвращает ошибку неизвестной команды.
TEST_F(CommandProcessorTest, Process_WhenCommandIsEmpty_ReturnsUnknownCommandError) {
    EXPECT_EQ("ERR unknown command\n", processor.process(""));
}

// Test 4.2. Неизвестная команда возвращает ошибку.
TEST_F(CommandProcessorTest, Process_WhenCommandIsUnknown_ReturnsUnknownCommandError) {
    EXPECT_EQ("ERR unknown command\n", processor.process("SELECT"));
}

// Test 4.3. INSERT с неизвестной таблицей возвращает ошибку.
TEST_F(CommandProcessorTest, Insert_WhenTableIsUnknown_ReturnsUnknownTableError) {
    EXPECT_EQ("ERR unknown table\n", processor.process("INSERT C 1 name"));
}

// Test 4.4. TRUNCATE с неизвестной таблицей возвращает ошибку.
TEST_F(CommandProcessorTest, Truncate_WhenTableIsUnknown_ReturnsUnknownTableError) {
    EXPECT_EQ("ERR unknown table\n", processor.process("TRUNCATE C"));
}

#endif

#if (1)  // Part 5. Неверное количество аргументов и разделители

// Test 5.1. INSERT без имени возвращает ошибку аргументов.
TEST_F(CommandProcessorTest, Insert_WhenNameIsMissing_ReturnsInvalidArgumentsError) {
    EXPECT_EQ("ERR invalid arguments\n", processor.process("INSERT A 1"));
}

// Test 5.2. TRUNCATE с неверным количеством аргументов возвращает ошибку.
TEST_F(CommandProcessorTest, Truncate_WhenArgumentCountIsInvalid_ReturnsError) {
    EXPECT_EQ("ERR invalid arguments\n", processor.process("TRUNCATE"));
    EXPECT_EQ("ERR invalid arguments\n", processor.process("TRUNCATE A extra"));
}

// Test 5.3. Команды выборки не принимают аргументы.
TEST_F(CommandProcessorTest, SelectCommands_WhenArgumentIsPassed_ReturnInvalidArgumentsError) {
    EXPECT_EQ("ERR invalid arguments\n", processor.process("INTERSECTION extra"));
    EXPECT_EQ("ERR invalid arguments\n", processor.process("SYMMETRIC_DIFFERENCE extra"));
}

// Test 5.4. Повторные и начальные пробелы отклоняются.
TEST_F(CommandProcessorTest, Process_WhenSpacesAreNotStrict_ReturnsError) {
    EXPECT_EQ("ERR invalid arguments\n", processor.process("INSERT  A 1 sweater"));
    EXPECT_EQ("ERR unknown command\n", processor.process(" INSERT A 1 sweater"));
}

#endif

#if (1)  // Part 6. Неверный идентификатор

// Test 6.1. Нечисловой идентификатор возвращает ошибку.
TEST_F(CommandProcessorTest, Insert_WhenIdIsNotNumeric_ReturnsInvalidIdError) {
    EXPECT_EQ("ERR invalid id\n", processor.process("INSERT A id sweater"));
}

// Test 6.2. Идентификатор с лишними символами возвращает ошибку.
TEST_F(CommandProcessorTest, Insert_WhenIdHasTrailingCharacters_ReturnsInvalidIdError) {
    EXPECT_EQ("ERR invalid id\n", processor.process("INSERT A 1x sweater"));
}

// Test 6.3. Идентификатор за пределами int возвращает ошибку.
TEST_F(CommandProcessorTest, Insert_WhenIdExceedsIntRange_ReturnsInvalidIdError) {
    EXPECT_EQ("ERR invalid id\n", processor.process("INSERT A 2147483648 sweater"));
}

// Test 6.4. Слишком длинное числовое значение возвращает ошибку.
TEST_F(CommandProcessorTest, Insert_WhenIdExceedsParserRange_ReturnsInvalidIdError) {
    EXPECT_EQ("ERR invalid id\n", processor.process("INSERT A 999999999999999999999999 sweater"));
}

// Test 6.5. Пустой идентификатор возвращает ошибку.
TEST_F(CommandProcessorTest, Insert_WhenIdIsEmpty_ReturnsInvalidIdError) {
    EXPECT_EQ("ERR invalid id\n", processor.process("INSERT A  sweater"));
}

// Test 6.6. Ошибочная команда не изменяет содержимое таблицы.
TEST_F(CommandProcessorTest, Insert_WhenCommandIsInvalid_KeepsDatabaseUnchanged) {
    ASSERT_EQ("ERR invalid id\n", processor.process("INSERT A 1x sweater"));
    EXPECT_EQ("OK\n", processor.process("INSERT A 1 sweater"));
}

#endif
