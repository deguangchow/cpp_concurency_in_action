///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter9: Advanced thread management
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/22
#pragma once
#ifndef ADVANCED_THREAD_MANAGEMENT_H
#define ADVANCED_THREAD_MANAGEMENT_H
#include "lock_based_concurrent_data_structures.h"
#include "designing_concurrent_code.h"

namespace adv_thread_mg {

//9.1 Thread pools
//9.1.1 The simplest posible thread pool
//Listing 9.1 Simple thread pool
class simple_thread_pool {
    std::atomic_bool done;
    lock_based_conc_data::threadsafe_queue_shared_ptr<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    design_conc_code::join_threads joiner;

    void work_thread() {
        while (!done) {
            std::function<void()> task;
            if (work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
                common_fun::sleep(ONE);
            }
        }
    }

public:
    simple_thread_pool() : done(false), joiner(threads) {
        TICK();
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.push_back(std::thread(&simple_thread_pool::work_thread, this));
            }
        } catch (...) {
            done = true;
            throw;
        }
    }
    ~simple_thread_pool() {
        TICK();
        done = true;
    }
    template<typename FunctionType>
    void submit(FunctionType f) {
        TICK();
        work_queue.push(std::function<void()>(f));
    }
};
void simple_thread_pool_test();

}//namespace adv_thread_mg

#endif  //ADVANCED_THREAD_MANAGEMENT_H


