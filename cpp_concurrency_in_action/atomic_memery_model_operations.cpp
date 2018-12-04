///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter5: The C++ memory model and operations on atomic types
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/04
#include "stdafx.h"
#include "atomic_memery_model_operations.h"

namespace atomic_type {

//5.2 Atomic operations and types in C++
//5.2.1 The standard atomic types
//5.2.2 Operations on std::atomic_flag
void atomic_flag_test() {
    TICK();
    std::atomic_flag f = ATOMIC_FLAG_INIT;
    f.clear(std::memory_order_release);
    bool x = f.test_and_set();
    INFO("x=%s", x ? "true" : "false");
}

//Listing 5.1 Implementation of a spinlock mutex using std::atomic_flag
spinlock_mutex s_mutex;
void task1() {
    TICK();
    while (true) {
        std::unique_lock<spinlock_mutex> lk(s_mutex);
        std::cout << "task1" << std::endl << "----------------------------------------------------" << std::endl;
    }
}
void task2() {
    TICK();
    while (true) {
        std::unique_lock<spinlock_mutex> lk(s_mutex);
        std::cout << "task2" << std::endl << "++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    }
}
void spinlock_mutex_test() {
    TICK();
    std::thread t1(task1);
    std::thread t2(task2);
    t1.join();
    t2.join();
}

//5.2.3 Operations on std::atomic<bool>
void atomic_bool_test() {
    TICK();
    std::atomic<bool> b;
    bool x = b.load(std::memory_order_acquire);
    INFO("x=%s", x ? "true" : "false");

    b.store(false);
    x = b.load(std::memory_order_acquire);
    INFO("x=%s", x ? "true" : "false");

    b.store(true);
    x = b.load(std::memory_order_acquire);
    INFO("x=%s", x ? "true" : "false");


    x = b.exchange(false, std::memory_order_acq_rel);
    INFO("x=%s", x ? "true" : "false");
}

}//atomic_type