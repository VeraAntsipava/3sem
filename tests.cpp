// Unit-тесты на Google Test.
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "sharedarray.h"

// Тест 1: новый массив заполнен нулями, свободный элемент помечается
TEST(SharedArrayTest, MarksFreeCell) {
    SharedArray a(5);
    for (int v : a.snapshot()) EXPECT_EQ(v, 0);
    EXPECT_TRUE(a.try_mark(2, 7));
    EXPECT_EQ(a.snapshot()[2], 7);
    EXPECT_EQ(a.count_marked(7), 1);
}

// Тест 2: занятый элемент пометить нельзя, значение не меняется
TEST(SharedArrayTest, CannotMarkBusyCell) {
    SharedArray a(5);
    ASSERT_TRUE(a.try_mark(1, 3));
    EXPECT_FALSE(a.try_mark(1, 4));
    EXPECT_EQ(a.snapshot()[1], 3);
}

// Тест 3: clear_marked обнуляет только элементы своего потока
TEST(SharedArrayTest, ClearsOnlyOwnMarks) {
    SharedArray a(6);
    a.try_mark(0, 1); a.try_mark(1, 2); a.try_mark(2, 1);
    EXPECT_EQ(a.clear_marked(1), 2);
    auto s = a.snapshot();
    EXPECT_EQ(s[0], 0);
    EXPECT_EQ(s[1], 2);
    EXPECT_EQ(s[2], 0);
}

// Тест 4: потокобезопасность - несколько потоков пытаются занять все ячейки,
// каждая ячейка должна быть занята ровно одним потоком
TEST(SharedArrayTest, ThreadSafeMarking) {
    const size_t N = 100;
    SharedArray a(N);
    std::vector<int> success(5, 0);
    std::vector<std::thread> ts;
    for (int id = 1; id <= 4; ++id)
        ts.emplace_back([&, id] {
            for (size_t i = 0; i < N; ++i)
                if (a.try_mark(i, id)) ++success[id];
        });
    for (auto& t : ts) t.join();
    EXPECT_EQ(success[1] + success[2] + success[3] + success[4], (int)N);
    for (int v : a.snapshot()) EXPECT_NE(v, 0);
}