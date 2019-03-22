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

}//namespace adv_thread_mg

