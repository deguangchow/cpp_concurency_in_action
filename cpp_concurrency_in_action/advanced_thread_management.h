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
int sum(int a, int b);
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
    template<typename F, typename...Args>
    struct impl_type : impl_base {
        F f;
        explicit impl_type(F&& f_) : f(move(f_)) {}
        void call(Args&&...args) {
            //TICK();
            f(args...);
        }
    };

public:
    template<typename F>
    explicit function_wrapper(F&& f) : impl(new impl_type<F>(move(f))) {}
    template<typename...Args>
    void operator()(Args&&...args) { impl->call(args...); }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) : impl(move(other.impl)) {}
    function_wrapper& operator=(function_wrapper&& other) {
        //TICK();
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
                DEBUG("run");
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
    void run_pending() {
        TICK();
        function_wrapper task;
        if (m_queueTasks.try_pop(task)) {
            DEBUG("run_pending");
            task();
        } else {
            yield();
        }
    }
};

template<typename ThreadPool>
void test_thread_pool() {
    TICK();
    ThreadPool threadPool;
    unsigned const& THREAD_NUMS = 5;
    vector<thread> vctThreads(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vctThreads[i] = thread(&ThreadPool::submit<TASK_TYPE>, &threadPool, task1);
        async(&ThreadPool::submit<TASK_TYPE>, &threadPool, task2);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vctThreads[i].join();
    }

    async(&ThreadPool::submit<TASK_TYPE>, &threadPool, [] {
        TICK();
        INFO("lambda.");
    });

    auto const& lambdaSum = [](int a, int b) {
        TICK();
        INFO("lambda_sum(%d, %d)", a, b);
        return a + b;
    };

    int a = 1, b = 2;
    auto fnRes1 = threadPool.submit(bind(sum, a, b));
    INFO("sum(%d, %d)=%d", a, b, fnRes1.get());

    a = 3, b = 4;
    auto fnRes2 = threadPool.submit(bind(lambdaSum, a, b));
    INFO("sum(%d, %d)=%d", a, b, fnRes2.get());

    a = 5, b = 6;
    async(&ThreadPool::submit<TASK_TYPE>, &threadPool, bind(sum, a, b));

    a = 7, b = 8;
    async(&ThreadPool::submit<TASK_TYPE>, &threadPool, bind(lambdaSum, a, b));
}

//Listing 9.3 parallel_accumulate using a thread pool with waitable tasks
template<typename Iterator, typename T, typename ThreadPool>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const DATA_LENGTH = distance(first, last);
    if (!DATA_LENGTH) {
        return init;
    }
    unsigned long const BLOCK_SIZE = 25;
    unsigned long const BLOCK_NUMS = (DATA_LENGTH + BLOCK_SIZE - 1) / BLOCK_SIZE;

    vector<future<T>>   vctFutures(BLOCK_NUMS - 1);
    ThreadPool          threadPool;
    Iterator            posBlockStart = first;
    for (unsigned long i = 0; i < (BLOCK_NUMS - 1); ++i) {
        Iterator posBlockEnd = posBlockStart;
        advance(posBlockEnd, BLOCK_SIZE);

        //submit tasks to the thread pool, and return the result of the task to the future variables.
        vctFutures[i] = threadPool.submit(
            bind(design_conc_code::accumulate_block<Iterator, T>(), posBlockStart, posBlockEnd));

        posBlockStart = posBlockEnd;
    }
    T last_result = design_conc_code::accumulate_block<Iterator, T>()(posBlockStart, last);
    T result = init;
    for (unsigned long i = 0; i < (BLOCK_NUMS - 1); ++i) {
        result += vctFutures[i].get();
    }
    result += last_result;
    return result;
}

template<typename ThreadPool>
void test_parallel_accumulate() {
    TICK();
    unsigned nRes = parallel_accumulate<vector<unsigned>::const_iterator, unsigned, ThreadPool>
        (VCT_NUMBERS.begin(), VCT_NUMBERS.end(), 0);

    INFO("test_parallel_accumulate()=%d", nRes);
}

//Listing 9.5 A thread pool-based implementation of Quicksort
template<typename T, typename ThreadPool>
struct sorter {
    ThreadPool          threadPool;
    list<T> do_sort(list<T>& chunk_data) {
        //TICK();
        if (chunk_data.empty()) {
            return chunk_data;
        }
        list<T>         result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const&        tPartitionValue = *result.begin();
        auto            posDivide = partition(chunk_data.begin(), chunk_data.end(),
            [&](T const& val) {
            return val < tPartitionValue;
        });

        list<T>         lstNewLowerChunk;
        lstNewLowerChunk.splice(lstNewLowerChunk.end(), chunk_data, chunk_data.begin(), posDivide);

        future<list<T>> lstLower_f =
            threadPool.submit(bind(&sorter::do_sort, this, move(lstNewLowerChunk)));

        list<T>         lstNewHigherChunk(do_sort(chunk_data));
        result.splice(result.end(), lstNewHigherChunk);

        while (lstLower_f.wait_for(seconds(0)) == future_status::timeout) {
            WARN("sorter<T>::do_sort() loop...");
            threadPool.run_pending();
        }
        result.splice(result.begin(), lstLower_f.get());
        return result;
    }
};

template<typename T, typename ThreadPool>
list<T> parallel_quick_sort(list<T> input) {
    TICK();
    if (input.empty()) {
        return input;
    }
    sorter<T, ThreadPool> s;
    return s.do_sort(input);
}

template<typename ThreadPool>
void test_parallel_quick_sort() {
    TICK();

    list<unsigned> lstResult = adv_thread_mg::parallel_quick_sort<unsigned, ThreadPool>(LST_NUMBERS);
    for (auto const &pos : lstResult) {
        cout << pos << ", ";
    }
    cout << endl;
}

//Listing 9.6 A thread pool with thread-local work queue
typedef std::queue<function_wrapper> LOCAL_QUEUE_TYPE;
class thread_pool_local {
    atomic_bool                                                 m_bDone_a;
    vector<thread>                                              m_vctThreads;
    lock_based_conc_data::threadsafe_queue<function_wrapper>    m_queuePoolTasks;
    static thread_local unique_ptr<LOCAL_QUEUE_TYPE>            m_pQueuelocalTasks_tl;
    design_conc_code::join_threads                              m_threadJoiner;

    void run() {
        TICK();
        m_pQueuelocalTasks_tl.reset(new LOCAL_QUEUE_TYPE());
        while (!m_bDone_a) {
            run_pending();

            yield();
        }
    }

public:
    thread_pool_local() : m_bDone_a(false), m_threadJoiner(m_vctThreads) {
        TICK();
        auto const& THREAD_NUMS = thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < THREAD_NUMS; ++i) {
                m_vctThreads.push_back(thread(&thread_pool_local::run, this));
            }
        } catch (...) {
            m_bDone_a = true;
            throw;
        }
    }
    ~thread_pool_local() {
        TICK();
        m_bDone_a = true;
    }
    template<typename F, typename...Args>
    future<typename result_of<F(Args...)>::type> submit(F&& f, Args&&...args) {
        TICK();
        typedef typename result_of<F(Args...)>::type result_type;

        packaged_task<result_type(Args...)> task(move(f));
        future<result_type> res(task.get_future());
        if (m_pQueuelocalTasks_tl) {
            //if the class member variable 'm_pQueuelocalTasks_tl' has been initialized to 'nullptr', never run to here.
            m_pQueuelocalTasks_tl->push(function_wrapper(move(task)));
        } else {
            //if the class member variable 'm_pQueuelocalTasks_tl' has been initialized to object, never run to here.
            m_queuePoolTasks.push(function_wrapper(move(task)));
        }
        return res;
    }
    void run_pending() {
        TICK();
        function_wrapper task;
        if (m_pQueuelocalTasks_tl && !m_pQueuelocalTasks_tl->empty()) {
            task = move(m_pQueuelocalTasks_tl->front());
            m_pQueuelocalTasks_tl->pop();
            DEBUG("local task");
            task();
        } else if (m_queuePoolTasks.try_pop(task)) {
            DEBUG("pool task");
            task();
        } else {
            WARN("run_pending, yield...");
            yield();
        }
    }
};


//Listing 9.7 Lock-based queue for work stealing
class work_stealing_queue {
private:
    typedef function_wrapper DATA_TYPE;
    deque<DATA_TYPE>                    m_dequeData;
    mutable mutex                       m_mutex;

public:
    work_stealing_queue() {}
    work_stealing_queue& operator=(const work_stealing_queue& other) = delete;
    void push(DATA_TYPE data) {
        TICK();
        lock_guard<mutex> lock(m_mutex);
        m_dequeData.push_front(move(data));
    }
    bool empty() const {
        TICK();
        lock_guard<mutex> lock(m_mutex);
        return m_dequeData.empty();
    }
    bool try_pop(DATA_TYPE& res) {
        TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_dequeData.empty()) {
            return false;
        }

        DEBUG("local task");
        res = move(m_dequeData.front());
        m_dequeData.pop_front();
        return true;
    }
    bool try_steal(DATA_TYPE& res) {
        lock_guard<mutex> lock(m_mutex);
        if (m_dequeData.empty()) {
            return false;
        }

        WARN("steal task");
        res = move(m_dequeData.back());
        m_dequeData.pop_back();
        return true;
    }
};

//Listing 9.8 A thread pool that uses work stealing
class thread_pool_steal {
    typedef function_wrapper TASK_TYPE;

    atomic<bool>                                        m_bDone_a;
    lock_based_conc_data::threadsafe_queue<TASK_TYPE>   m_queuePoolTasks;
    vector<unique_ptr<work_stealing_queue>>             m_vctStealingQueues;
    vector<thread>                                      m_vctThreads;
    design_conc_code::join_threads                      m_threadJoiner;

    static thread_local work_stealing_queue*            m_pQueueLocalTasks_tl;
    static thread_local unsigned                        m_uIndex_tl;

    void run(unsigned my_index_) {
        TICK();

        //common_fun::sleep(10);

        m_uIndex_tl             = my_index_;
        m_pQueueLocalTasks_tl   = m_vctStealingQueues[m_uIndex_tl].get();
        while (!m_bDone_a) {
            run_pending();
        }
    }
    bool pop_task_from_local_queue(TASK_TYPE& task) {
        TICK();
        return m_pQueueLocalTasks_tl && m_pQueueLocalTasks_tl->try_pop(task);
    }
    bool pop_task_from_pool_queue(TASK_TYPE& task) {
        TICK();
        return m_queuePoolTasks.try_pop(task);
    }
    bool pop_task_from_other_thread_queue(TASK_TYPE& task) {
        TICK();
        for (unsigned i = 0; i < m_vctStealingQueues.size(); ++i) {
            unsigned const index = (m_uIndex_tl + i + 1) % m_vctStealingQueues.size();
            if (m_vctStealingQueues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

public:
    thread_pool_steal() : m_bDone_a(false), m_threadJoiner(m_vctThreads) {
        TICK();
        unsigned const thread_count = thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                m_vctStealingQueues.push_back(unique_ptr<work_stealing_queue>(new work_stealing_queue));
                m_vctThreads.push_back(thread(&thread_pool_steal::run, this, i));
            }
        } catch (...) {
            m_bDone_a = true;
            throw;
        }
    }
    ~thread_pool_steal() {
        m_bDone_a = true;
    }
    template<typename FunctionType>
    future<typename result_of<FunctionType()>::type> submit(FunctionType f) {
        TICK();
        typedef typename result_of<FunctionType()>::type result_type;

        packaged_task<result_type()> task(f);
        future<result_type> res(task.get_future());
        if (m_pQueueLocalTasks_tl) {
            m_pQueueLocalTasks_tl->push(function_wrapper(move(task)));
        } else {
            m_queuePoolTasks.push(function_wrapper(move(task)));
        }
        return res;
    }
    void run_pending() {
        TICK();
        TASK_TYPE task;
        if (pop_task_from_local_queue(task) ||
            pop_task_from_pool_queue(task) ||
            pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            //WARN("run_pending, yield...");
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


