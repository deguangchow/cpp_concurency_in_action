// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <gtest/gtest.h>
#include "thread_test.h"


TEST(THREAD_TEST, 001) {
    EXPECT_NO_THROW(thread_test::hello_test());
    EXPECT_NO_THROW(thread_test::hello());
    EXPECT_NO_THROW(thread_test::hello_concurrency());
}

TEST(thread_test, 002) {
    EXPECT_NO_THROW({
        std::thread t([] {std::cout << "hello worold" << std::endl; });
    t.join();
    });
}

TEST(thread_test, 003) {
    EXPECT_NO_THROW({
        std::async(std::launch::async, [] {std::cout << "hello worold" << std::endl; });
    });
}
