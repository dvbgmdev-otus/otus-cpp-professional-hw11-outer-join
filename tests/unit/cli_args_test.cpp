/**
 * @file cli_args_test.cpp
 * @brief Unit-тесты разбора аргументов командной строки.
 */

#include "cli_args.h"

#include <gtest/gtest.h>

#if (1)  // Part 1. Успешный разбор аргументов

// Test 1.1. Порт извлекается из корректных аргументов.
TEST(CliArgsTest, Parse_WhenPortIsValid_ReturnsParsedValue) {
    const char* argv[] = { "join_server", "9000" };

    const CliArgsParseResult result = parse_cli_args(2, argv);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(9000U, result.port);
    EXPECT_TRUE(result.error.empty());
}

// Test 1.2. Граничный TCP-порт 65535 допустим.
TEST(CliArgsTest, Parse_WhenMaximumPortIsPassed_ReturnsParsedPort) {
    const char* argv[] = { "join_server", "65535" };

    const CliArgsParseResult result = parse_cli_args(2, argv);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(65535U, result.port);
}

#endif  // Part 1. Успешный разбор аргументов

#if (1)  // Part 2. Ошибки количества аргументов

// Test 2.1. Отсутствие порта возвращает ошибку.
TEST(CliArgsTest, Parse_WhenArgumentsAreMissing_ReturnsError) {
    const char* argv[] = { "join_server" };

    const CliArgsParseResult result = parse_cli_args(1, argv);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

// Test 2.2. Лишний аргумент возвращает ошибку.
TEST(CliArgsTest, Parse_WhenExtraArgumentIsPassed_ReturnsError) {
    const char* argv[] = { "join_server", "9000", "extra" };

    const CliArgsParseResult result = parse_cli_args(3, argv);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

#endif  // Part 2. Ошибки количества аргументов

#if (1)  // Part 3. Ошибки значения порта

// Test 3.1. Нулевой порт возвращает ошибку.
TEST(CliArgsTest, Parse_WhenPortIsZero_ReturnsError) {
    const char* argv[] = { "join_server", "0" };

    const CliArgsParseResult result = parse_cli_args(2, argv);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(0U, result.port);
}

// Test 3.2. Порт выше 65535 возвращает ошибку.
TEST(CliArgsTest, Parse_WhenPortExceedsMaximum_ReturnsError) {
    const char* argv[] = { "join_server", "65536" };

    const CliArgsParseResult result = parse_cli_args(2, argv);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(0U, result.port);
}

// Test 3.3. Отрицательный порт возвращает ошибку.
TEST(CliArgsTest, Parse_WhenPortIsNegative_ReturnsError) {
    const char* argv[] = { "join_server", "-1" };

    const CliArgsParseResult result = parse_cli_args(2, argv);

    EXPECT_FALSE(result.success);
}

// Test 3.4. Нечисловой порт возвращает ошибку.
TEST(CliArgsTest, Parse_WhenPortIsNotNumeric_ReturnsError) {
    const char* argv[] = { "join_server", "port" };

    const CliArgsParseResult result = parse_cli_args(2, argv);

    EXPECT_FALSE(result.success);
}

#endif  // Part 3. Ошибки значения порта
