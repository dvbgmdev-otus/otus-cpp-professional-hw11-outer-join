/**
 * @file database_test.cpp
 * @brief Unit-тесты хранилища таблиц A и B в SQLite.
 */

#include "database.h"

#include <gtest/gtest.h>

class DatabaseTest : public ::testing::Test {
protected:
    database::Database m_Database;
};

#if (1)  // Part 1. Добавление записей

// Test 1.1. Запись добавляется в пустую таблицу.
TEST_F(DatabaseTest, Insert_WhenIdIsNew_ReturnsInserted) {
    EXPECT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 1, "sweater"));
}

// Test 1.2. Повторный идентификатор в одной таблице отклоняется.
TEST_F(DatabaseTest, Insert_WhenIdAlreadyExistsInTable_ReturnsDuplicate) {
    ASSERT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 1, "sweater"));

    EXPECT_EQ(database::InsertResult::Duplicate,
              m_Database.insert(database::Table::A, 1, "understand"));
}

// Test 1.3. Одинаковый идентификатор разрешён в разных таблицах.
TEST_F(DatabaseTest, Insert_WhenIdExistsInOtherTable_ReturnsInserted) {
    ASSERT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 3, "violation"));

    EXPECT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::B, 3, "proposal"));
}

#endif  // Part 1. Добавление записей

#if (1)  // Part 2. Очистка таблиц

// Test 2.1. После очистки идентификатор можно добавить повторно.
TEST_F(DatabaseTest, Truncate_WhenTableHasRows_AllowsIdsToBeInsertedAgain) {
    ASSERT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 1, "sweater"));

    m_Database.truncate(database::Table::A);

    EXPECT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 1, "understand"));
}

// Test 2.2. Очистка одной таблицы не затрагивает вторую.
TEST_F(DatabaseTest, Truncate_WhenOtherTableHasRows_KeepsOtherTableUnchanged) {
    ASSERT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 3, "violation"));
    ASSERT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::B, 3, "proposal"));

    m_Database.truncate(database::Table::A);

    EXPECT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::A, 3, "quality"));
    EXPECT_EQ(database::InsertResult::Duplicate,
              m_Database.insert(database::Table::B, 3, "example"));
}

// Test 2.3. После очистки таблицы B идентификатор можно добавить повторно.
TEST_F(DatabaseTest, Truncate_WhenTableBHasRows_AllowsIdsToBeInsertedAgain) {
    ASSERT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::B, 6, "flour"));

    m_Database.truncate(database::Table::B);

    EXPECT_EQ(database::InsertResult::Inserted,
              m_Database.insert(database::Table::B, 6, "wonder"));
}

#endif  // Part 2. Очистка таблиц
