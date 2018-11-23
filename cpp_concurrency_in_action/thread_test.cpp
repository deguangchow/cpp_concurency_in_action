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

void hello_test() {
    TICK();
    std::cout << "Hello World" << std::endl;
}

void hello() {
    TICK();
    std::cout << "Hello Concurrent world" << std::endl;
}

void hello_concurrency() {
    TICK();
    std::thread t(hello);
    t.join();
}

}//namespace thread_test


