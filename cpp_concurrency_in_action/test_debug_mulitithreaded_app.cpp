///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter10: Testing and debugging multithreaded applications
///
///    \author   deguangchow
///    \version  1.0
///    \2019/03/25
#include "stdafx.h"
#include "test_debug_mulitithreaded_app.h"
#include "lock_based_concurrent_data_structures.h"

namespace test_debug_mulitithread {


void test_concurrent_push_and_pop_on_empty_queue() {
    TICK();
    lock_based_conc_data::threadsafe_queue<int> q;

    std::promise<void> go, push_ready, pop_ready;
    std::shared_future<void> ready(go.get_future());

    std::future<void> push_done;
    std::future<std::shared_ptr<int>> pop_done;

    try {
        push_done = std::async(std::launch::async, [&q, ready, &push_ready]() {
            push_ready.set_value();
            ready.wait();
            q.push(42);
        });
        pop_done = std::async(std::launch::async, [&q, ready, &pop_ready]() {
            pop_ready.set_value();
            ready.wait();
            return q.try_pop();
        });
        push_ready.get_future().wait();
        pop_ready.get_future().wait();
        go.set_value();

        push_done.get();
        assert(*(pop_done.get()) == 42);
        assert(q.empty());
    } catch (...) {
        go.set_value();
        throw;
    }
}

}//namespace test_debug_mulitithread

