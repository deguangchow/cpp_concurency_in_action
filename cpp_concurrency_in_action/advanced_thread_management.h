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

using TASK_TYPE = function<void()>;


//9.1 Thread pools
//9.1.1 The simplest posible thread pool
//Listing 9.1 Simple thread pool
void task1();
void task2();
class simple_thread_pool {
    atomic_bool                                                         m_abDone;
    lock_based_conc_data::threadsafe_queue_shared_ptr<TASK_TYPE>        m_queueTasks;
    vector<thread>                                                      m_vctThreads;
    design_conc_code::join_threads                                      m_threadJoiner;

    void run() {
        while (!m_abDone) {
            TASK_TYPE task;
            if (m_queueTasks.try_pop(task)) {
                task();
            }
            yield();
        }
    }

public:
    simple_thread_pool() : m_abDone(false), m_threadJoiner(m_vctThreads) {
        TICK();
        auto const& THREAD_NUMS = thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < THREAD_NUMS; ++i) {
                m_vctThreads.push_back(thread(&simple_thread_pool::run, this));
            }
        } catch (...) {
            ERR("catch...");
            m_abDone = true;
            throw;
        }
    }
    ~simple_thread_pool() {
        TICK();
        m_abDone = true;
    }

    template<typename FunctionType>
    void submit(FunctionType f) {
        TICK();
        m_queueTasks.push(TASK_TYPE(f));
    }
};
void test_simple_thread_pool();

//9.1.2 Waiting for tasks submitted to a thread pool
//Listing 9.2 A thread pool with waitable tasks
class function_wrapper {
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };
    unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type : impl_base {
        F f;
        explicit impl_type(F&& f_) : f(move(f_)) {}
        void call() {
            TICK();
            f();
        }
    };

public:
    template<typename F>
    explicit function_wrapper(F&& f) : impl(new impl_type<F>(move(f))) {}
    void operator()() { impl->call(); }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) : impl(move(other.impl)) {}
    function_wrapper& operator=(function_wrapper&& other) {
        TICK();
        impl = move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    //function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};
class thread_pool {
    atomic_bool                                                 m_abDone;
    lock_based_conc_data::threadsafe_queue<function_wrapper>    m_queueTasks;
    vector<thread>                                              m_vctThreads;
    design_conc_code::join_threads                              m_threadJoiner;
    void run() {
        TICK();
        while (!m_abDone) {
            function_wrapper task;
            if (m_queueTasks.try_pop(task)) {
                task();
            } else {
                yield();
            }
        }
    }

public:
    thread_pool() : m_abDone(false), m_threadJoiner(m_vctThreads) {
        TICK();
        auto const& THREAD_NUMS = thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < THREAD_NUMS; ++i) {
                m_vctThreads.push_back(thread(&thread_pool::run, this));
            }
        } catch (...) {
            m_abDone = true;
            throw;
        }
    }
    ~thread_pool() {
        TICK();
        m_abDone = true;
    }
    template<typename F, typename...Args>
    future<typename result_of<F(Args...)>::type> submit(F&& f, Args&&...args) {
        TICK();
        typedef typename result_of<F(Args...)>::type result_type;

        packaged_task<result_type(Args...)> task(move(f));
        future<result_type> res(task.get_future());
        m_queueTasks.push(function_wrapper(move(task)));
        return res;
    }
    //9.1.3 Tasks that wait for other tasks
    //Listing 9.4 An implementation of run_pending_task()
    void running_pending_task() {
        TICK();
        function_wrapper task;
        if (m_queueTasks.try_pop(task)) {
            task();
        } else {
            yield();
        }
    }
};
void test_thread_pool();

//Listing 9.3 parallel_accumulate using a thread pool with waitable tasks
int task3(int a, int b);
template<typename Iterator, typename T>
T task4(Iterator first, Iterator last) {
    TICK();
    return accumulate(first, last, T());
}
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const length = distance(first, last);
    if (!length) {
        return init;
    }
    unsigned long const block_size = 25;
    unsigned long const num_blocks = (length + block_size - 1) / block_size;

    vector<future<T>> futures(num_blocks - 1);
    thread_pool pool;
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_blocks - 1); ++i) {
        Iterator block_end = block_start;
        advance(block_end, block_size);
#if 0//exist compile error
        typedef function<int(int, int)> Func;
        futures[i] = pool.submit<Func>(design_conc_code::accumulate_block<Iterator, T>());
#else//only 'void()' can be compiled correctly
        typedef function<void()> Func;
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
    list<T> do_sort(list<T>& chunk_data) {
        if (chunk_data.empty()) {
            return chunk_data;
        }
        list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const& partition_val = *result.begin();
        typename list<T>::iterator divide_point = partition(chunk_data.begin(), chunk_data.end(),
            [&](T const& val) {return val < partition_val; });

        list<T> new_lower_chunk;
        new_lower_chunk.splice(new_lower_chunk.end(), chunk_data, chunk_data.begin(), divide_point);
        future<list<T>> new_lower =
            pool.submit(bind(&sorter::do_sort, this, move(new_lower_chunk)));

        list<T> new_higher(do_sort(chunk_data));
        result.splice(result.end(), new_higher);

        while (new_lower.wait_for(seconds(0)) != future_status::timeout) {
            pool.running_pending_task();
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }
};
template<typename T>
list<T> parallel_quick_sort(list<T> input) {
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

    atomic_bool done;
    typedef queue<function_wrapper> local_queue_type;
    static thread_local unique_ptr<local_queue_type> local_work_queue;

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
    future<typename result_of<FunctionType()>::type> submit(FunctionType f) {
        TICK();
        typedef typename result_of<FunctionType()>::type result_type;

        packaged_task<result_type()> task(f);
        future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(move(task));
        } else {
            pool_work_queue.push(move(task));
        }
        return res;
    }
    void run_pending_task() {
        TICK();
        function_wrapper task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = move(local_work_queue->front());
            local_work_queue->pop();
            task();
        } else if (pool_work_queue.try_pop(task)) {
            task();
        } else {
            yield();
        }
    }
};

//Listing 9.7 Lock-based queue for work stealing
class work_stealing_queue {
private:
    typedef function_wrapper data_type;
    deque<data_type> the_queue;
    mutable mutex the_mutex;

public:
    work_stealing_queue() {}
    work_stealing_queue& operator=(const work_stealing_queue& other) = delete;
    void push(data_type data) {
        TICK();
        lock_guard<mutex> lock(the_mutex);
        the_queue.push_front(move(data));
    }
    bool empty() const {
        TICK();
        lock_guard<mutex> lock(the_mutex);
        return the_queue.empty();
    }
    bool try_pop(data_type& res) {
        TICK();
        lock_guard<mutex> lock(the_mutex);
        if (the_queue.empty()) {
            return false;
        }
        res = move(the_queue.front());
        the_queue.pop_front();
        return true;
    }
    bool try_steal(data_type& res) {
        lock_guard<mutex> lock(the_mutex);
        if (the_queue.empty()) {
            return false;
        }
        res = move(the_queue.back());
        the_queue.pop_back();
        return true;
    }
};

//Listing 9.8 A thread pool that uses work stealing
class thread_pool_steal {
    typedef function_wrapper task_type;

    atomic<bool> done;
    lock_based_conc_data::threadsafe_queue<task_type> pool_work_queue;
    vector<unique_ptr<work_stealing_queue>> queues;
    vector<thread> threads;
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
        unsigned const thread_count = thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                queues.push_back(unique_ptr<work_stealing_queue>(new work_stealing_queue));
                threads.push_back(thread(&thread_pool_steal::worker_thread, this, i));
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
    future<typename result_of<FunctionType()>::type> submit(FunctionType f) {
        TICK();
        typedef typename result_of<FunctionType()>::type result_type;

        packaged_task<result_type()> task(f);
        future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(move(task));
        } else {
            pool_work_queue.push(move(task));
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
            yield();
        }
    }
};

//9.2 Interrupting threads
//9.2.1 Launching and interrupting another thread
//Listing 9.9 Basic implementation of interruptible_thread
class interrupt_flag {
    atomic<bool> flag = false;
public:
    void set();
    bool is_set() const;
};
class interruptible_thread {
    thread internal_thread;
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

//9.2.3 Interrupting a condition variable wait
//Listing 9.10 A broken version of interruptible_wait for condition_variable
//Listing 9.11 Using a timeout in interruptible_wait for condition_variable
class interrupt_flag_cv {
    atomic<bool> flag;
    condition_variable* thread_cond;
    mutex set_clear_mutex;
public:
    interrupt_flag_cv();
    void set();
    bool is_set() const;
    void set_condition_variable(condition_variable& cv);
    void clear_condition_variable();
    struct clear_cv_on_destruct {
        ~clear_cv_on_destruct();
    };
};
void interruption_point_cv();
void interruptible_wait(condition_variable& cv, unique_lock<mutex>& lk);
template<typename Predicate>
void interruptible_wait(condition_variable& cv, unique_lock<mutex>& lk, Predicate pred);

//9.2.4 Interrupting a wait on condition_variable_any
//Listing 9.12 interruptible_wait for condition_variable_any
class interrupt_flag_cva {
    atomic<bool> flag;
    condition_variable* thread_cond;
    condition_variable_any* thread_cond_any;
    mutex set_clear_mutex;
public:
    interrupt_flag_cva();
    void set();
    bool is_set() const;
    template<typename Lockable>
    void wait(condition_variable_any& cv, Lockable& lk);
    void set_condition_variable(condition_variable& cv);
    void clear_condition_variable();
};
void interruption_point_cva();
template<typename Lockable>
void interruptible_wait(condition_variable_any& cv, Lockable& lk);

//9.2.5 Interrupting other blocking calls
template<typename T>
void interruptible_wait(future<T>& uf);

//9.2.6 Handling interruptions

//9.2.7 Interrupting background tasks on application exit
//Listing 9.13 Monitoring the filesystem in the background
void background_thread(int disk_id);
void start_background_processing();
void monitor_filesystem();

}//namespace adv_thread_mg

#endif  //ADVANCED_THREAD_MANAGEMENT_H


