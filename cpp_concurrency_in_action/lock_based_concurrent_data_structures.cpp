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

//6.2.2 A thread-safe queue using locks and condition variables
//Listing 6.2 The full class definition for a thread-safe queue using condition variables
threadsafe_queue<unsigned> tsq;
void threadsafe_queue_write() {
    TICK();
    tsq.push(1);
    tsq.push(2);
    tsq.push(3);
#if 0
    tsq.push(4);
#endif
}
void threadsafe_queue_read() {
    TICK();
    std::shared_ptr<unsigned> ptr1 = tsq.wait_and_pop();
    if (nullptr == ptr1) {
        INFO("wait_and_pop()=nullptr");
    } else {
        INFO("wait_and_pop()=%d", *ptr1);
    }
    std::shared_ptr<unsigned> ptr2 = tsq.try_pop();
    if (nullptr == ptr2) {
        INFO("try_pop()=nullptr");
    } else {
        INFO("try_pop()=%d", *ptr2);
    }
    unsigned u3 = -1;
    tsq.wait_and_pop(u3);
    INFO("wait_and_pop(%d)", u3);
    unsigned u4 = -1;
    tsq.try_pop(u4);
    INFO("try_pop(%d)", u4);
}
void treadsafe_queue_test() {
    TICK();
    std::thread t1(threadsafe_queue_write);
    std::thread t2(threadsafe_queue_write);
    std::thread t3(threadsafe_queue_write);
    std::thread t4(threadsafe_queue_read);
    std::thread t5(threadsafe_queue_read);
    std::thread t6(threadsafe_queue_read);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
}

//Listing 6.3 A thread-safe queue holding std::shared_ptr<> instances
threadsafe_queue_shared_ptr<unsigned> tsqsp;
void threadsafe_queue_shared_ptr_write() {
    TICK();
    tsqsp.push(1);
    tsqsp.push(2);
    tsqsp.push(3);
    tsqsp.push(4);
}
void threadsafe_queue_shared_ptr_read() {
    TICK();
    unsigned u1 = -1;
    tsqsp.wait_and_pop(u1);
    INFO("wait_and_pop(%d)", u1);
    unsigned u2 = -1;
    tsqsp.try_pop(u2);
    INFO("try_pop(%d)", u2);
    std::shared_ptr<unsigned> ptr3 = tsqsp.wait_and_pop();
    if (nullptr == ptr3) {
        INFO("wait_and_pop()=nullptr");
    } else {
        INFO("wait_and_pop()=%d", *ptr3);
    }
    std::shared_ptr<unsigned> ptr4 = tsqsp.try_pop();
    if (nullptr == ptr4) {
        INFO("try_pop()=nullptr");
    } else {
        INFO("try_pop()=%d", *ptr4);
    }
}

void threadsafe_queue_shared_ptr_test() {
    TICK();
    std::thread t1(threadsafe_queue_shared_ptr_write);
    std::thread t2(threadsafe_queue_shared_ptr_write);
    std::thread t3(threadsafe_queue_shared_ptr_write);
    std::thread t4(threadsafe_queue_shared_ptr_read);
    std::thread t5(threadsafe_queue_shared_ptr_read);
    std::thread t6(threadsafe_queue_shared_ptr_read);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
}

//Listing 6.5 A simple queue with a dummy node
dummy_queue<unsigned> dq;
void dummy_queue_write() {
    TICK();
    dq.push(1);
    dq.push(2);
    dq.push(3);
    dq.push(4);
}
void dummy_queue_read() {
    TICK();
    std::shared_ptr<unsigned> ptr1 = dq.try_pop();
    std::shared_ptr<unsigned> ptr2 = dq.try_pop();
    std::shared_ptr<unsigned> ptr3 = dq.try_pop();
    std::shared_ptr<unsigned> ptr4 = dq.try_pop();
    INFO("try_pop()=%d", *ptr1);
    INFO("try_pop()=%d", *ptr2);
    INFO("try_pop()=%d", *ptr3);
    INFO("try_pop()=%d", *ptr4);
}
void dummy_queue_test() {
    TICK();
    std::thread t1(dummy_queue_write);
    std::thread t2(dummy_queue_write);
    std::thread t3(dummy_queue_write);
    std::thread t4(dummy_queue_read);
    std::thread t5(dummy_queue_read);
    std::thread t6(dummy_queue_read);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
}

//Listing 6.6 A thread-safe queue with fine-grained locking
threadsafe_queue_fine_grained<unsigned> tsqfg;
void threadsafe_queue_fine_grained_write() {
    TICK();
    tsqfg.push(1);
    tsqfg.push(2);
    tsqfg.push(3);
    tsqfg.push(4);
}
void threadsafe_queue_fine_grained_read() {
    TICK();
    std::shared_ptr<unsigned> ptr1 = tsqfg.try_pop();
    std::shared_ptr<unsigned> ptr2 = tsqfg.try_pop();
    std::shared_ptr<unsigned> ptr3 = tsqfg.try_pop();
    std::shared_ptr<unsigned> ptr4 = tsqfg.try_pop();
    INFO("try_pop()=%d", ptr1 ? *ptr1 : -1);//-1 won`t be printed as the function 'try_pop' would return 0 if empty
    INFO("try_pop()=%d", ptr2 ? *ptr2 : -1);
    INFO("try_pop()=%d", ptr3 ? *ptr3 : -1);
    INFO("try_pop()=%d", ptr4 ? *ptr4 : -1);
}
void threadsafe_queue_fine_grained_test() {
    TICK();
    std::thread t1(threadsafe_queue_fine_grained_write);
    std::thread t2(threadsafe_queue_fine_grained_write);
    std::thread t3(threadsafe_queue_fine_grained_write);
    std::thread t4(threadsafe_queue_fine_grained_read);
    std::thread t5(threadsafe_queue_fine_grained_read);
    std::thread t6(threadsafe_queue_fine_grained_read);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
}

}//namespace lock_based_conc_data

