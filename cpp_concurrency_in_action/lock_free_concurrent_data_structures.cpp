///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter7: Designing lock-free concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/16
#include "stdafx.h"
#include "lock_free_concurrent_data_structures.h"

namespace lock_free_conc_data {

spinlock_mutex slm;
unsigned count = 0;
void spinlock_mutex_plus() {
    TICK();
    std::lock_guard<spinlock_mutex> lock(slm);
    ++count;
    INFO("count=%d", count);
}
void spinlock_mutex_test() {
    TICK();
    std::thread t1(spinlock_mutex_plus);
    std::thread t2(spinlock_mutex_plus);
    std::thread t3(spinlock_mutex_plus);
    std::thread t4(spinlock_mutex_plus);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

//7.2 Example of lock-free data structures
//7.2.1 Writing a thread-safe stack without locks
//Listing 7.2 Implementing push() without locks
lock_free_stack<unsigned> lfs;
void lock_free_stack_test() {
    TICK();
    unsigned const THREAD_NUS = 5;
    std::vector<std::thread> vct_push(THREAD_NUS);
    std::vector<std::thread> vct_pop(THREAD_NUS);

    for (unsigned i = 0; i < THREAD_NUS; ++i) {
        vct_push[i] = std::thread(&lock_free_stack<unsigned>::push, &lfs, i);
    }
    for (unsigned i = 0; i < THREAD_NUS; ++i) {
        unsigned result = -1;
        vct_pop[i] = std::thread(&lock_free_stack<unsigned>::pop, &lfs, std::ref(result));
    }
    for (unsigned i = 0; i < THREAD_NUS; ++i) {
        vct_push[i].join();
    }
    for (unsigned i = 0; i < THREAD_NUS; ++i) {
        vct_pop[i].join();
    }
}

}//namespace lock_free_conc_data


