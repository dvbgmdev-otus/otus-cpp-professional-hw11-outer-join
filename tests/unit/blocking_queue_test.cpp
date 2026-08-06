/**
 * @file blocking_queue_test.cpp
 * @brief Unit-тесты блокирующей очереди.
 */

#include "blocking_queue.h"

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <string>

class BlockingQueueTest : public ::testing::Test {
protected:
    BlockingQueue<int> queue;
};

#if (1)  // 1. FIFO-поведение

// 1.1 Очередь возвращает элементы в порядке добавления.
TEST_F(BlockingQueueTest, Pop_WhenSeveralItemsPushed_ReturnsItemsInFifoOrder) {
    ASSERT_TRUE(queue.push(1));
    ASSERT_TRUE(queue.push(2));
    ASSERT_TRUE(queue.push(3));

    int value = 0;
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(1, value);
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(2, value);
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(3, value);
}

// 1.2 Очередь поддерживает move-only элементы.
TEST_F(BlockingQueueTest, Pop_WhenMoveOnlyItemPushed_ReturnsMovedItem) {
    BlockingQueue<std::unique_ptr<int>> move_only_queue;

    ASSERT_TRUE(move_only_queue.push(std::unique_ptr<int>(new int(42))));

    std::unique_ptr<int> value;
    ASSERT_TRUE(move_only_queue.pop(value));
    ASSERT_NE(nullptr, value);
    EXPECT_EQ(42, *value);
}

#endif

#if (1)  // 2. Блокировка и пробуждение

// 2.1 pop ожидает появления элемента и возвращает его после push.
TEST_F(BlockingQueueTest, Pop_WhenQueueIsEmpty_WaitsUntilItemPushed) {
    auto result = std::async(std::launch::async, [this]() {
        int value = 0;
        const bool popped = queue.pop(value);
        return std::make_pair(popped, value);
    });

    EXPECT_EQ(std::future_status::timeout, result.wait_for(std::chrono::milliseconds(50)));

    ASSERT_TRUE(queue.push(7));

    ASSERT_EQ(std::future_status::ready, result.wait_for(std::chrono::seconds(1)));
    const auto popped_value = result.get();
    EXPECT_TRUE(popped_value.first);
    EXPECT_EQ(7, popped_value.second);
}

#endif

#if (1)  // 3. Закрытие очереди

// 3.1 close будит ожидающий pop и тот возвращает false для пустой очереди.
TEST_F(BlockingQueueTest, Pop_WhenQueueClosedAndEmpty_ReturnsFalse) {
    auto result = std::async(std::launch::async, [this]() {
        int value = 0;
        return queue.pop(value);
    });

    EXPECT_EQ(std::future_status::timeout, result.wait_for(std::chrono::milliseconds(50)));

    queue.close();

    ASSERT_EQ(std::future_status::ready, result.wait_for(std::chrono::seconds(1)));
    EXPECT_FALSE(result.get());
}

// 3.2 После close очередь отдаёт уже добавленные элементы, а затем возвращает false.
TEST_F(BlockingQueueTest, Pop_WhenQueueClosedWithItems_DrainsItemsBeforeFalse) {
    ASSERT_TRUE(queue.push(1));
    ASSERT_TRUE(queue.push(2));
    queue.close();

    int value = 0;
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(1, value);
    ASSERT_TRUE(queue.pop(value));
    EXPECT_EQ(2, value);
    EXPECT_FALSE(queue.pop(value));
}

// 3.3 После close новые элементы не принимаются.
TEST_F(BlockingQueueTest, Push_WhenQueueClosed_ReturnsFalse) {
    queue.close();

    EXPECT_FALSE(queue.push(1));
    EXPECT_TRUE(queue.is_closed());
}

#endif
