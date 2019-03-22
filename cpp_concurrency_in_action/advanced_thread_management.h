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
void task1();
void task2();
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
        TICK();
        while (!done) {
            WARN("worker_thread loop");
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
    template<typename F, typename...Args>
    std::future<typename std::result_of<F(Args...)>::type> submit(F&& f, Args&&...args) {
        TICK();
        typedef typename std::result_of<F(Args...)>::type result_type;

        std::packaged_task<result_type(Args...)> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push(function_wrapper(std::move(task)));
        return res;
    }
    //9.1.3 Tasks that wait for other tasks
    //Listing 9.4 An implementation of run_pending_task()
    void running_pending_task() {
        TICK();
        function_wrapper task;
        if (work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};
void thread_pool_test();

//Listing 9.3 parallel_accumulate using a thread pool with waitable tasks
int task3(int a, int b);
template<typename Iterator, typename T>
T task4(Iterator first, Iterator last) {
    TICK();
    return std::accumulate(first, last, T());
}
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const length = std::distance(first, last);
    if (!length) {
        return init;
    }
    unsigned long const block_size = 25;
    unsigned long const num_blocks = (length + block_size - 1) / block_size;

    std::vector<std::future<T>> futures(num_blocks - 1);
    thread_pool pool;
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_blocks - 1); ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
#if 0//exist compile error
        typedef std::function<int(int, int)> Func;
        futures[i] = pool.submit<Func>(design_conc_code::accumulate_block<Iterator, T>());
#else//only 'void()' can be compiled correctly
        typedef std::function<void()> Func;
        pool.submit<Func>(&task2);
#endif
        block_start = block_end;
    }
    T last_result = design_conc_code::accumulate_block<Iterator, T>()(block_start, last);
    T result = init;
    for (unsigned long i = 0; i < (num_blocks - 1); ++i) {
        result += futures[i].get();
    }
    result += last_result;
    return result;
}
void parallel_accumulate_test();

//Listing 9.5 A thread pool-based implementation of Quicksort
template<typename T>
struct sorter {
    thread_pool pool;
    std::list<T> do_sort(std::list<T>& chunk_data) {
        if (chunk_data.empty()) {
            return chunk_data;
        }
        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const& partition_val = *result.begin();
        typename std::list<T>::iterator divide_point = std::partition(chunk_data.begin(), chunk_data.end(),
            [&](T const& val) {return val < partition_val; });

        std::list<T> new_lower_chunk;
        new_lower_chunk.splice(new_lower_chunk.end(), chunk_data, chunk_data.begin(), divide_point);
        std::future<std::list<T>> new_lower =
            pool.submit(std::bind(&sorter::do_sort, this, std::move(new_lower_chunk)));

        std::list<T> new_higher(do_sort(chunk_data));
        result.splice(result.end(), new_higher);

        while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::timeout) {
            pool.running_pending_task();
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }
};
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
    TICK();
    if (input.empty()) {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}
void parallel_quick_sort_test();

//Listing 9.6 A thread pool with thread-local work queue
class thread_pool_local {
    lock_based_conc_data::threadsafe_queue<function_wrapper> pool_work_queue;

    std::atomic_bool done;
    typedef std::queue<function_wrapper> local_queue_type;
    static thread_local std::unique_ptr<local_queue_type> local_work_queue;

    void worker_thread() {
        TICK();
        local_work_queue.reset(new local_queue_type);
        while (!done) {
            run_pending_task();
        }
    }

public:
    thread_pool_local() : done(false) {
        TICK();
    }
    ~thread_pool_local() {
        TICK();
        done = true;
    }
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        TICK();
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }
    void run_pending_task() {
        TICK();
        function_wrapper task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = std::move(local_work_queue->front());
            local_work_queue->pop();
            task();
        } else if (pool_work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};

//Listing 9.7 Lock-based queue for work stealing
class work_stealing_queue {
private:
    typedef function_wrapper data_type;
    std::deque<data_type> the_queue;
    mutable std::mutex the_mutex;

public:
    work_stealing_queue() {}
    work_stealing_queue& operator=(const work_stealing_queue& other) = delete;
    void push(data_type data) {
        TICK();
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push_front(std::move(data));
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }
    bool try_pop(data_type& res) {
        TICK();
        std::lock_guard<std::mutex> lock(the_mutex);
        if (the_queue.empty()) {
            return false;
        }
        res = std::move(the_queue.front());
        the_queue.pop_front();
        return true;
    }
    bool try_steal(data_type& res) {
        std::lock_guard<std::mutex> lock(the_mutex);
        if (the_queue.empty()) {
            return false;
        }
        res = std::move(the_queue.back());
        the_queue.pop_back();
        return true;
    }
};

//Listing 9.8 A thread pool that uses work stealing
class thread_pool_steal {
    typedef function_wrapper task_type;

    std::atomic<bool> done;
    lock_based_conc_data::threadsafe_queue<task_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue>> queues;
    std::vector<std::thread> threads;
    design_conc_code::join_threads joiner;

    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned my_index;

    void worker_thread(unsigned my_index_) {
        TICK();
        my_index = my_index_;
        local_work_queue = queues[my_index].get();
        while (!done) {
            run_pending_task();
        }
    }
    bool pop_task_from_local_queue(task_type& task) {
        TICK();
        return local_work_queue && local_work_queue->try_pop(task);
    }
    bool pop_task_from_pool_queue(task_type& task) {
        TICK();
        return pool_work_queue.try_pop(task);
    }
    bool pop_task_from_other_thread_queue(task_type& task) {
        TICK();
        for (unsigned i = 0; i < queues.size(); ++i) {
            unsigned const index = (my_index + i + 1) % queues.size();
            if (queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

public:
    thread_pool_steal() : done(false), joiner(threads) {
        TICK();
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                queues.push_back(std::unique_ptr<work_stealing_queue>(new work_stealing_queue));
                threads.push_back(std::thread(&thread_pool_steal::worker_thread, this, i));
            }
        } catch (...) {
            done = true;
            throw;
        }
    }
    ~thread_pool_steal() {
        done = true;
    }
    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        TICK();
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }
    void run_pending_task() {
        TICK();
        task_type task;
        if (pop_task_from_local_queue(task) ||
            pop_task_from_pool_queue(task) ||
            pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};

//9.2 Interrupting threads
//9.2.1 Launching and interrupting another thread
//Listing 9.9 Basic implementation of interruptible_thread
class interrupt_flag {
    std::atomic<bool> flag = false;
public:
    void set();
    bool is_set() const;
};
class interruptible_thread {
    std::thread internal_thread;
    interrupt_flag* flag;
public:
    template<typename FunctionType>
    explicit interruptible_thread(FunctionType f);
    void join();
    void detach();
    bool joinable() const;
    void interrupt();
};

//9.2.2 Detecting that a thread has been interrupted
void interruption_point();
void interruptible_thread_test();


}//namespace adv_thread_mg

#endif  //ADVANCED_THREAD_MANAGEMENT_H


