/**
 * @file bulk_writer_test.cpp
 * @brief Unit-тесты вывода блоков команд.
 */

#include "bulk_writer.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

class BulkWriterTest : public ::testing::Test {
protected:
    void SetUp() override { remove_log_file(); }

    void TearDown() override { remove_log_file(); }

    std::string read_log_file() const {
        std::ifstream input(log_file_name);
        std::ostringstream content;
        content << input.rdbuf();
        return content.str();
    }

    std::string read_postfixed_log_file() const {
        std::ifstream input(postfixed_log_file_name);
        std::ostringstream content;
        content << input.rdbuf();
        return content.str();
    }

    void remove_log_file() const {
        std::remove(log_file_name.c_str());
        std::remove(postfixed_log_file_name.c_str());
    }

    const std::string log_file_name = "bulk123.log";
    const std::string postfixed_log_file_name = "bulk123_7.log";
};

#if (1)  // 1. Вывод в консоль

// 1.1 Консольный writer выводит отформатированный блок и перевод строки.
TEST_F(BulkWriterTest, ConsoleWriter_WhenBlockWritten_EmitsBulkLine) {
    std::ostringstream output;
    ConsoleBulkWriter writer(output);
    const CommandBlock block{ { "cmd1", "cmd2" }, 123 };
    writer.write(block);
    EXPECT_EQ("bulk: cmd1, cmd2\n", output.str());
}

#endif

#if (1)  // 2. Вывод в файл

// 2.1 Файловый writer создает файл с timestamp блока в имени.
TEST_F(BulkWriterTest, FileWriter_WhenBlockWritten_CreatesTimestampedFile) {
    FileBulkWriter writer;
    const CommandBlock block{ { "cmd1", "cmd2" }, 123 };
    writer.write(block);
    std::ifstream input(log_file_name);
    EXPECT_TRUE(input.is_open());
}

// 2.2 Файловый writer записывает отформатированный блок и перевод строки.
TEST_F(BulkWriterTest, FileWriter_WhenBlockWritten_WritesBulkLine) {
    FileBulkWriter writer;
    const CommandBlock block{ { "cmd1", "cmd2" }, 123 };
    writer.write(block);
    EXPECT_EQ("bulk: cmd1, cmd2\n", read_log_file());
}

// 2.3 Файловый writer добавляет постфикс к имени файла.
TEST_F(BulkWriterTest, FileWriter_WhenPostfixPassed_CreatesPostfixedFile) {
    FileBulkWriter writer;
    const CommandBlock block{ { "cmd1", "cmd2" }, 123 };
    writer.write(block, 7);
    std::ifstream input(postfixed_log_file_name);
    EXPECT_TRUE(input.is_open());
    EXPECT_EQ("bulk: cmd1, cmd2\n", read_postfixed_log_file());
}

#endif
