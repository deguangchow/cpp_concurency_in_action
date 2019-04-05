///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter1: unit test.
///
///    \author   deguangchow
///    \version  1.0
///    \2019/04/05
#include "stdafx.h"
#include <gtest/gtest.h>
#include "thread_test.h"


TEST(THREAD_TEST, 001) {
    TICK();
    EXPECT_NO_THROW(thread_test::hello_test());
    EXPECT_NO_THROW(thread_test::hello());
    EXPECT_NO_THROW(thread_test::hello_concurrency());
}

TEST(thread_test, 002) {
    TICK();
    EXPECT_NO_THROW({
        std::thread t([] {std::cout << "hello worold" << std::endl; });
    t.join();
    });
}

TEST(thread_test, 002_1) {
    TICK();
    EXPECT_NO_THROW({
        std::thread t([] {std::cout << "hello worold" << std::endl; });
    t.detach();
    });
}

TEST(thread_test, 003) {
    TICK();
    EXPECT_NO_THROW({
        std::async(std::launch::async, [] {std::cout << "hello worold" << std::endl; });
    });
}

TEST(thread_test, 003_1) {
    TICK();
    EXPECT_NO_THROW({
        std::async(std::launch::deferred, [] {std::cout << "hello worold" << std::endl; });
    });
}

