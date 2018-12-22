///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter9: Advanced thread management
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/22
#include "stdafx.h"
#include "advanced_thread_management.h"

namespace adv_thread_mg {

//9.1 Thread pools
//9.1.1 The simplest posible thread pool
//Listing 9.1 Simple thread pool
void task1() {
    TICK();
    INFO("task1");
}
void task2() {
    TICK();
    INFO("task2");
}
void simple_thread_pool_test() {
    TICK();
    simple_thread_pool stp;
    stp.submit(task1);
    stp.submit(task1);
    stp.submit(task1);
    stp.submit(task1);
    stp.submit(task2);
    stp.submit(task2);
    stp.submit(task2);
    stp.submit(task2);
    typedef std::function<void()> Func;
    std::async(&simple_thread_pool::submit<Func>, &stp, task1);
    std::async(&simple_thread_pool::submit<Func>, &stp, task2);
    stp.submit([] {TICK(); INFO("task5"); });
    stp.submit([] {TICK(); INFO("task6"); });
}

}//namespace adv_thread_mg

