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

    void worker_thread() {
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
                threads.push_back(std::thread(&simple_thread_pool::worker_thread, this));
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

//9.1.2 Waiting for tasks submitted to a thread pool
//Listing 9.2 A thread pool with waitable tasks
class function_wrapper {
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };
    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type : impl_base {
        F f;
        explicit impl_type(F&& f_) : f(std::move(f_)) {}
        void call() {
            TICK();
            f();
        }
    };

public:
    template<typename F>
    explicit function_wrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}
    void operator()() { impl->call(); }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) : impl(std::move(other.impl)) {}
    function_wrapper& operator=(function_wrapper&& other) {
        TICK();
        impl = std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};
class thread_pool {
    std::atomic_bool done;
    lock_based_conc_data::threadsafe_queue<function_wrapper> work_queue;
    std::vector<std::thread> threads;
    design_conc_code::join_threads joiner;
    void worker_thread() {
        while (!done) {
            function_wrapper task;
            if (work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
                common_fun::sleep(ONE);
            }
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
        TICK();
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.push_back(std::thread(&thread_pool::worker_thread, this));
            }
        } catch (...) {
            done = true;
            throw;
        }
    }
    ~thread_pool() {
        TICK();
        done = true;
    }
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        TICK();
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push(function_wrapper(std::move(task)));
        return res;
    }
};
void thread_pool_test();

}//namespace adv_thread_mg

#endif  //ADVANCED_THREAD_MANAGEMENT_H


