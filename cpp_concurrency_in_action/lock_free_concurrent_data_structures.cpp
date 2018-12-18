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
    unsigned const &THREAD_NUMS = 5;
    std::vector<std::thread> vct_push(THREAD_NUMS);
    std::vector<std::thread> vct_pop(THREAD_NUMS);

    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_push[i] = std::thread(&lock_free_stack<unsigned>::push, &lfs, i);
        unsigned result = -1;
        vct_pop[i] = std::thread(&lock_free_stack<unsigned>::pop, &lfs, std::ref(result));
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_push[i].join();
        vct_pop[i].join();
    }
}

//Listing 7.3 A lock-free stack that leaks nodes
lock_free_shared_ptr_stack<unsigned> lfsps;
void lock_free_shared_ptr_stack_test() {
    TICK();
    unsigned const &THREAD_NUMS = 5;
    std::vector<std::thread> vct_push(THREAD_NUMS);
    std::vector<std::future<std::shared_ptr<unsigned>>> vct_pop_res(THREAD_NUMS);

    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        INFO("push(%d)", i + 1);
        vct_push[i] = std::thread(&lock_free_shared_ptr_stack<unsigned>::push, &lfsps, i + 1);
        vct_pop_res[i] = std::async(&lock_free_shared_ptr_stack<unsigned>::pop, &lfsps);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_push[i].join();
        INFO("pop()=%d", *vct_pop_res[i].get());
    }
}

//7.2.2 Stopping those pesky leaks: managing memory in lock-free data structures
//Listing 7.4 Reclaiming nodes when no threads are in pop()
lock_free_reclaim_stack<unsigned> lfrs;
void lock_free_reclaim_stack_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    std::vector<std::thread> vct_push(THREAD_NUMS);
    std::vector<std::future<std::shared_ptr<unsigned>>> vct_pop_res(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        INFO("push(%d)", i + 1);
        vct_push[i] = std::thread(&lock_free_reclaim_stack<unsigned>::push, &lfrs, i + 1);
        vct_pop_res[i] = std::async(&lock_free_reclaim_stack<unsigned>::pop, &lfrs);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_push[i].join();
        INFO("pop()=%d", *vct_pop_res[i].get());
    }
}

//7.2.4 Detecting nodes in use with reference counting
//Listing 7.8 A lock-free stack using a lock-free std::shared_ptr<> implementation
lock_free_shared_stack<unsigned> lfss;
void lock_free_shared_stack_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    std::vector<std::thread> vct_push(THREAD_NUMS);
    std::vector<std::future<std::shared_ptr<unsigned>>> vct_pop_res(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        INFO("push(%d)", i + 1);
        vct_push[i] = std::thread(&lock_free_shared_stack<unsigned>::push, &lfss, i + 1);
        vct_pop_res[i] = std::async(&lock_free_shared_stack<unsigned>::pop, &lfss);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_push[i].join();
        INFO("pop()=%d", *vct_pop_res[i].get());
    }
}

//Listing 7.10 Pushing a node on a lock-free stack using split reference counts
lock_free_split_ref_cnt_stack<unsigned> lfsrcs;
void lock_free_split_ref_cnt_stack_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    std::vector<std::thread> vct_push(THREAD_NUMS);
    std::vector<std::future<std::shared_ptr<unsigned>>> vct_pop_res(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        INFO("push(%d)", i + 1);
        vct_push[i] = std::thread(&lock_free_split_ref_cnt_stack<unsigned>::push, &lfsrcs, i + 1);
        vct_pop_res[i] = std::async(&lock_free_split_ref_cnt_stack<unsigned>::pop, &lfsrcs);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_push[i].join();
        std::shared_ptr<unsigned> ptr = vct_pop_res[i].get();
        INFO("pop()=%d", ptr ? *ptr : -1);
    }
}

}//namespace lock_free_conc_data


