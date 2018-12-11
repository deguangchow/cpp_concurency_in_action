///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter6: Designing lock-based concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/11
#include "stdafx.h"
#include "lock_based_concurrent_data_structures.h"

namespace lock_based_conc_data {

//6.2 Lock-based concurrent data structures
//6.2.1 A thread-safe stack using locks
//Listing 6.1 A class definition for a thread-safe stack
thread_safe_stack<int> st1;
void lock_thread_safe_stack_write() {
    TICK();
    st1.push(1);
    st1.push(2);
    st1.push(3);
}
void lock_thread_safe_stack_read() {
    TICK();
    std::shared_ptr<int> ptr1 = st1.pop();
    std::shared_ptr<int> ptr2 = st1.pop();
    std::shared_ptr<int> ptr3 = st1.pop();
    INFO("ptr1=%d", *ptr1);
    INFO("ptr2=%d", *ptr2);
    INFO("ptr3=%d", *ptr3);
}
void lock_thread_safe_stack_test() {
    TICK();
    try {
        std::thread t1(lock_thread_safe_stack_write);
        std::thread t2(lock_thread_safe_stack_write);
        std::thread t3(lock_thread_safe_stack_write);
        std::thread t4(lock_thread_safe_stack_read);
        std::thread t5(lock_thread_safe_stack_read);
        std::thread t6(lock_thread_safe_stack_read);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
    } catch (empty_stack const &e) {
        INFO("empty_stack err:%s", e.what());
    } catch (...) {
        INFO("exception");
    }
}



}//namespace lock_based_conc_data

