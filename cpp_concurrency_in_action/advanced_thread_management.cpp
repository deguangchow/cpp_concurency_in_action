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
int task3(int a, int b) {
    TICK();
    return 0;
}
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

//Listing 9.5 A thread pool-based implementation of Quicksort
void parallel_quick_sort_test() {
    TICK();
    std::list<unsigned> lst_input = {
#if 1
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
#else
        2, 1
#endif
    };
    std::list<unsigned> lst_result = parallel_quick_sort<unsigned>(lst_input);
    for (auto const &pos : lst_result) {
        std::cout << pos << ", ";
    }
    std::cout << std::endl;
}

//9.2 Interrupting threads
//9.2.1 Launching and interrupting another thread
//Listing 9.9 Basic implementation of interruptible_thread
void interrupt_flag::set() {
    flag.store(true);
}
bool interrupt_flag::is_set() const {
    return flag.load();
}

thread_local interrupt_flag this_thread_interrupt_flag;

template<typename FunctionType>
interruptible_thread::interruptible_thread(FunctionType f) {
    std::promise<interrupt_flag*> p;
    internal_thread = std::thread([f, &p] {
        p.set_value(&this_thread_interrupt_flag);
        f();
    });
    flag = p.get_future().get();
}
void interruptible_thread::join() {
    internal_thread.join();
}
void interruptible_thread::detach() {
    internal_thread.detach();
}
bool interruptible_thread::joinable() const {
    return internal_thread.joinable();
}
void interruptible_thread::interrupt() {
    if (flag) {
        flag->set();
    }
}

//9.2.2 Detecting that a thread has been interrupted
void interruption_point() {
    if (this_thread_interrupt_flag.is_set()) {
        throw std::current_exception();//throw thread_interrupted();
    }
}
void interruptible_thread_test() {
    bool done = false;
    while (!done) {
        interruption_point();
        //process_next_item();
    }
}

//9.2.3 Interrupting a condition variable wait
//Listing 9.10 A broken version of interruptible_wait for std::condition_variable
//Listing 9.11 Using a timeout in interruptible_wait for std::condition_variable
thread_local interrupt_flag_cv this_thread_interrupt_flag_cv;

interrupt_flag_cv::interrupt_flag_cv() : thread_cond(0) {
}
void interrupt_flag_cv::set() {
    TICK();
    flag.store(true, std::memory_order_relaxed);
    std::lock_guard<std::mutex> lk(set_clear_mutex);
    if (thread_cond) {
        thread_cond->notify_all();
    }
}
bool interrupt_flag_cv::is_set() const {
    TICK();
    return flag.load(std::memory_order_relaxed);
}
void interrupt_flag_cv::set_condition_variable(std::condition_variable& cv) {
    TICK();
    std::lock_guard<std::mutex> lk(set_clear_mutex);
    thread_cond = &cv;
}
void interrupt_flag_cv::clear_condition_variable() {
    TICK();
    std::lock_guard<std::mutex> lk(set_clear_mutex);
    thread_cond = 0;
}
interrupt_flag_cv::clear_cv_on_destruct::~clear_cv_on_destruct() {
    TICK();
    this_thread_interrupt_flag_cv.clear_condition_variable();
}
void interruption_point_cv() {
    TICK();
    if (this_thread_interrupt_flag_cv.is_set()) {
        throw std::current_exception();//throw thread_interrupted();
    }
}
void interruptible_wait(std::condition_variable& cv, std::unique_lock<std::mutex>& lk) {
    TICK();
    interruption_point_cv();
    this_thread_interrupt_flag_cv.set_condition_variable(cv);
    interrupt_flag_cv::clear_cv_on_destruct guard;
    interruption_point_cv();
    cv.wait_for(lk, std::chrono::milliseconds(ONE));
    interruption_point_cv();
}
template<typename Predicate>
void interruptible_wait(std::condition_variable& cv, std::unique_lock<std::mutex>& lk, Predicate pred) {
    TICK();
    interruption_point_cv();
    this_thread_interrupt_flag_cv.set_condition_variable(cv);
    interrupt_flag_cv::clear_cv_on_destruct guard;
    while (!this_thread_interrupt_flag_cv.is_set() && !pred()) {
        cv.wait_for(lk, std::chrono::milliseconds(ONE));
    }
    interruption_point_cv();
}

//9.2.4 Interrupting a wait on std::condition_variable_any
//Listing 9.12 interruptible_wait for std::condition_variable_any
thread_local interrupt_flag_cva this_thread_interrupt_flag_cva;

interrupt_flag_cva::interrupt_flag_cva() : thread_cond(0), thread_cond_any(0) {
}
void interrupt_flag_cva::set() {
    TICK();
    flag.store(true, std::memory_order_relaxed);
    std::lock_guard<std::mutex> lk(set_clear_mutex);
    if (thread_cond) {
        thread_cond->notify_all();
    } else if (thread_cond_any) {
        thread_cond_any->notify_all();
    }
}
bool interrupt_flag_cva::is_set() const {
    TICK();
    return flag.load(std::memory_order_relaxed);
}
template<typename Lockable>
void interrupt_flag_cva::wait(std::condition_variable_any& cv, Lockable& lk) {
    struct custom_lock {
        interrupt_flag_cva* self;
        Lockable& lk;
        custom_lock(interrupt_flag_cva* self_, std::condition_variable_any& cond, Lockable& lk_) :
            self(self_), lk(lk_) {
            TICK();
            self->set_clear_mutex.lock();
            self->thread_cond_any = &cond;
        }
        void unlock() {
            TICK();
            lk.unlock();
            self->set_clear_mutex.unlock();
        }
        void lock() {
            TICK();
            std::lock(self->set_clear_mutex, lk);
        }
        ~custom_lock() {
            TICK();
            self->thread_cond_any = 0;
            self->set_clear_mutex.unlock();
        }
    };
    custom_lock cl(this, cv, lk);
    interruption_point_cva();
    cv.wait(cl);
    interruption_point_cva();
}
void interrupt_flag_cva::set_condition_variable(std::condition_variable& cv) {
    TICK();
    std::lock_guard<std::mutex> lk(set_clear_mutex);
    thread_cond = &cv;
}
void interrupt_flag_cva::clear_condition_variable() {
    TICK();
    std::lock_guard<std::mutex> lk(set_clear_mutex);
    thread_cond = 0;
    thread_cond_any = 0;
}
void interruption_point_cva() {
    TICK();
    if (this_thread_interrupt_flag_cva.is_set()) {
        throw std::current_exception();//throw thread_interrupted();
    }
}
template<typename Lockable>
void interruptible_wait(std::condition_variable_any& cv, Lockable& lk) {
    TICK();
    this_thread_interrupt_flag_cva.wait(cv, lk);
}

//9.2.5 Interrupting other blocking calls
template<typename T>
void interruptible_wait(std::future<T>& uf) {
    TICK();
    while (!this_thread_interrupt_flag_cva.is_set()) {
        if (uf.wait_for(lk, std::chrono::milliseconds(1)) == std::future_status::ready) {
            break;
        }
    }
    interruption_point_cva();
}

}//namespace adv_thread_mg

