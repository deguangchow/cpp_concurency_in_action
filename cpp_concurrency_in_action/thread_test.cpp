///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter1: Hello, Concurrency World.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#include "stdafx.h"
#include "thread_test.h"

namespace thread_test {

void test_hello() {
    TICK();
    DEBUG("Hello World");
}

void hello() {
    TICK();
    DEBUG("Hello Concurrent world");
}

void test_hello_concurrency() {
    TICK();
    thread t(hello);
    t.join();
}

}//namespace thread_test


