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

//9.1.2 Waiting for tasks submitted to a thread pool
//Listing 9.2 A thread pool with waitable tasks
void thread_pool_test() {
    TICK();
    thread_pool tp;
    unsigned const& THREAD_NUMS = 5;
    std::vector<std::thread> vct_submit(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        typedef std::function<void()> Func;
        vct_submit[i] = std::thread(&thread_pool::submit<Func>, &tp, task1);
        std::async(&thread_pool::submit<Func>, &tp, task2);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_submit[i].join();
    }
}

//Listing 9.3 parallel_accumulate using a thread pool with waitable tasks
void parallel_accumulate_test() {
    TICK();
    std::vector<unsigned> vct = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };
    unsigned ret = parallel_accumulate(vct.begin(), vct.end(), 0);
    INFO("ret=%d\r\n", ret);
}

}//namespace adv_thread_mg

