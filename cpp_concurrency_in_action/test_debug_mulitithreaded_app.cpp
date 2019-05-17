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
    lock_based_conc_data::threadsafe_queue<int> threadsafe_queue_;

    std::promise<void>                          promise_go, promise_push, promise_pop;
    std::shared_future<void>                    future_go(promise_go.get_future());

    std::future<void>                           future_push_done;
    std::future<std::shared_ptr<int>>           future_pop_done;

    try {
        future_push_done = std::async(std::launch::async, [&threadsafe_queue_, future_go, &promise_push]() {
            promise_push.set_value();
            future_go.wait();
            DEBUG("async push");
            threadsafe_queue_.push(42);
        });
        future_pop_done = std::async(std::launch::async, [&threadsafe_queue_, future_go, &promise_pop]() {
            promise_pop.set_value();
            future_go.wait();
            //sleep_for(milliseconds(1));
            DEBUG("async pop");
            return threadsafe_queue_.try_pop();
        });
        promise_push.get_future().wait();
        promise_pop.get_future().wait();
        promise_go.set_value();

        future_push_done.get();
        assert(*(future_pop_done.get()) == 42);
        assert(threadsafe_queue_.empty());
    } catch (...) {
        promise_go.set_value();
        throw;
    }
}

}//namespace test_debug_mulitithread

