/**
 * @file database_test.cpp
 * @brief Unit-тесты хранилища таблиц A и B в SQLite.
 */

#include "database.h"

#include <gtest/gtest.h>

class DatabaseTest : public ::testing::Test {
protected:
    database::Database test_database;
};

#if (1)  // Part 1. Добавление записей

// Test 1.1. Запись добавляется в пустую таблицу.
TEST_F(DatabaseTest, Insert_WhenIdIsNew_ReturnsInserted) {
    EXPECT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 1, "sweater"));
}

// Test 1.2. Повторный идентификатор в одной таблице отклоняется.
TEST_F(DatabaseTest, Insert_WhenIdAlreadyExistsInTable_ReturnsDuplicate) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 1, "sweater"));

    EXPECT_EQ(database::InsertResult::Duplicate,
              test_database.insert(database::Table::A, 1, "understand"));
}

// Test 1.3. Одинаковый идентификатор разрешён в разных таблицах.
TEST_F(DatabaseTest, Insert_WhenIdExistsInOtherTable_ReturnsInserted) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 3, "violation"));

    EXPECT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 3, "proposal"));
}

#endif  // Part 1. Добавление записей

#if (1)  // Part 2. Очистка таблиц

// Test 2.1. После очистки идентификатор можно добавить повторно.
TEST_F(DatabaseTest, Truncate_WhenTableHasRows_AllowsIdsToBeInsertedAgain) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 1, "sweater"));

    test_database.truncate(database::Table::A);

    EXPECT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 1, "understand"));
}

// Test 2.2. Очистка одной таблицы не затрагивает вторую.
TEST_F(DatabaseTest, Truncate_WhenOtherTableHasRows_KeepsOtherTableUnchanged) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 3, "violation"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 3, "proposal"));

    test_database.truncate(database::Table::A);

    EXPECT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 3, "quality"));
    EXPECT_EQ(database::InsertResult::Duplicate,
              test_database.insert(database::Table::B, 3, "example"));
}

// Test 2.3. После очистки таблицы B идентификатор можно добавить повторно.
TEST_F(DatabaseTest, Truncate_WhenTableBHasRows_AllowsIdsToBeInsertedAgain) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 6, "flour"));

    test_database.truncate(database::Table::B);

    EXPECT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 6, "wonder"));
}

#endif  // Part 2. Очистка таблиц

#if (1)  // Part 3. Пересечение таблиц

// Test 3.1. Пересечение пустых таблиц не содержит строк.
TEST_F(DatabaseTest, Intersection_WhenTablesAreEmpty_ReturnsEmptyResult) {
    EXPECT_TRUE(test_database.intersection().empty());
}

// Test 3.2. Пересечение содержит только общие идентификаторы и оба имени.
TEST_F(DatabaseTest, Intersection_WhenTablesOverlap_ReturnsCommonRows) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 2, "frank"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 3, "violation"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 4, "quality"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 3, "proposal"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 4, "example"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 6, "flour"));

    const std::vector<database::JoinedRow> rows = test_database.intersection();

    ASSERT_EQ(2U, rows.size());
    EXPECT_EQ(3, rows[0].id);
    EXPECT_EQ("violation", rows[0].a_name);
    EXPECT_EQ("proposal", rows[0].b_name);
    EXPECT_EQ(4, rows[1].id);
    EXPECT_EQ("quality", rows[1].a_name);
    EXPECT_EQ("example", rows[1].b_name);
}

// Test 3.3. Строки пересечения упорядочены по возрастанию идентификатора.
TEST_F(DatabaseTest, Intersection_WhenRowsInsertedOutOfOrder_ReturnsRowsOrderedById) {
    for (const int id : { 5, 3, 4 }) {
        ASSERT_EQ(database::InsertResult::Inserted,
                  test_database.insert(database::Table::A, id, "A"));
        ASSERT_EQ(database::InsertResult::Inserted,
                  test_database.insert(database::Table::B, id, "B"));
    }

    const std::vector<database::JoinedRow> rows = test_database.intersection();

    ASSERT_EQ(3U, rows.size());
    EXPECT_EQ(3, rows[0].id);
    EXPECT_EQ(4, rows[1].id);
    EXPECT_EQ(5, rows[2].id);
}

#endif  // Part 3. Пересечение таблиц

#if (1)  // Part 4. Симметрическая разность таблиц

// Test 4.1. Симметрическая разность пустых таблиц не содержит строк.
TEST_F(DatabaseTest, SymmetricDifference_WhenTablesAreEmpty_ReturnsEmptyResult) {
    EXPECT_TRUE(test_database.symmetric_difference().empty());
}

// Test 4.2. Результат содержит только уникальные для каждой таблицы строки.
TEST_F(DatabaseTest, SymmetricDifference_WhenTablesOverlap_ReturnsExclusiveRows) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 0, "lean"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 3, "violation"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 3, "proposal"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 6, "flour"));

    const std::vector<database::JoinedRow> rows = test_database.symmetric_difference();

    ASSERT_EQ(2U, rows.size());
    EXPECT_EQ(0, rows[0].id);
    EXPECT_EQ("lean", rows[0].a_name);
    EXPECT_TRUE(rows[0].b_name.empty());
    EXPECT_EQ(6, rows[1].id);
    EXPECT_TRUE(rows[1].a_name.empty());
    EXPECT_EQ("flour", rows[1].b_name);
}

// Test 4.3. Строки симметрической разности упорядочены по идентификатору.
TEST_F(DatabaseTest, SymmetricDifference_WhenRowsInsertedOutOfOrder_ReturnsRowsOrderedById) {
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 8, "selection"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 2, "frank"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::B, 7, "wonder"));
    ASSERT_EQ(database::InsertResult::Inserted,
              test_database.insert(database::Table::A, 1, "sweater"));

    const std::vector<database::JoinedRow> rows = test_database.symmetric_difference();

    ASSERT_EQ(4U, rows.size());
    EXPECT_EQ(1, rows[0].id);
    EXPECT_EQ(2, rows[1].id);
    EXPECT_EQ(7, rows[2].id);
    EXPECT_EQ(8, rows[3].id);
}

#endif  // Part 4. Симметрическая разность таблиц
