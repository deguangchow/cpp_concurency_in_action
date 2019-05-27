///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter7: Designing lock-free concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/16
#include "stdafx.h"
#include "lock_free_concurrent_data_structures.h"

namespace lock_free_conc_data {

spinlock_mutex  g_spinLockMutex;
unsigned        g_uCount = 0;
void spinlock_mutex_plus() {
    TICK();
    lock_guard<spinlock_mutex> lock(g_spinLockMutex);
    ++g_uCount;
    INFO("count=%d", g_uCount);

    common_fun::sleep(1);
}
void test_spinlock_mutex() {
    TICK();
#if 0
    thread t1(spinlock_mutex_plus);
    thread t2(spinlock_mutex_plus);
    thread t3(spinlock_mutex_plus);
    thread t4(spinlock_mutex_plus);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
#else
    vector<thread> vctThreads(HARDWARE_CONCURRENCY);
    for_each(vctThreads.begin(), vctThreads.end(), [](thread& t) {
        t = move(thread(&spinlock_mutex_plus));
    });
    for_each(vctThreads.begin(), vctThreads.end(), mem_fn(&thread::join));
#endif
}

//7.2 Example of lock-free data structures
//7.2.1 Writing a thread-safe stack without locks
//Listing 7.2 Implementing push() without locks
void test_lock_free_stack() {
    TICK();
    lock_free_stack<unsigned>   lockFreeStack;
    vector<thread>              vctThreadPush(HARDWARE_CONCURRENCY);
    vector<thread>              vctThreadPop(HARDWARE_CONCURRENCY);

#if 1
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i] = thread(&lock_free_stack<unsigned>::push, &lockFreeStack, i + 1);
        unsigned result = -1;
        vctThreadPop[i] = thread(&lock_free_stack<unsigned>::pop, &lockFreeStack, ref(result));
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        vctThreadPop[i].join();
    }
#else
    unsigned uValue = 0;
    for_each(vctThreadPush.begin(), vctThreadPush.end(), [&lockFreeStack, &uValue](thread& t) {
        t = move(thread(&lock_free_stack<unsigned>::push, &lockFreeStack, ++uValue));
    });
    unsigned uResult = -1;
    for_each(vctThreadPop.begin(), vctThreadPop.end(), [&lockFreeStack, &uResult](thread& t) {
        t = move(thread(&lock_free_stack<unsigned>::pop, ref(uResult)));
    });
    for_each(vctThreadPush.begin(), vctThreadPush.end(), mem_fn(&thread::join));
    for_each(vctThreadPop.begin(), vctThreadPop.end(), mem_fn(&thread::join));
#endif
}

//Listing 7.3 A lock-free stack that leaks nodes
void test_lock_free_shared_ptr_stack() {
    TICK();
    lock_free_shared_ptr_stack<unsigned>    lockFreeSharedPtrStack;

    vector<thread>                          vctThreadPush(HARDWARE_CONCURRENCY);
    vector<future<shared_ptr<unsigned>>>    vctFutureResult(HARDWARE_CONCURRENCY);

    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        DEBUG("push(%d)", i + 1);
        vctThreadPush[i] = thread(&lock_free_shared_ptr_stack<unsigned>::push, &lockFreeSharedPtrStack, i + 1);
        vctFutureResult[i] = async(&lock_free_shared_ptr_stack<unsigned>::pop, &lockFreeSharedPtrStack);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        INFO("pop()=%d", *vctFutureResult[i].get());
    }
}

//7.2.2 Stopping those pesky leaks: managing memory in lock-free data structures
//Listing 7.4 Reclaiming nodes when no threads are in pop()
void test_lock_free_reclaim_stack() {
    TICK();
    lock_free_reclaim_stack<unsigned>       lockFreeReclaimStack;

    vector<thread>                          vctThreadPush(HARDWARE_CONCURRENCY);
    vector<future<shared_ptr<unsigned>>>    vctFutureResult(HARDWARE_CONCURRENCY);

    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        DEBUG("push(%d)", i + 1);
        vctThreadPush[i] = thread(&lock_free_reclaim_stack<unsigned>::push, &lockFreeReclaimStack, i + 1);
        vctFutureResult[i] = async(&lock_free_reclaim_stack<unsigned>::pop, &lockFreeReclaimStack);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        INFO("pop()=%d", *vctFutureResult[i].get());
    }
}

//7.2.4 Detecting nodes in use with reference counting
//Listing 7.8 A lock-free stack using a lock-free shared_ptr<> implementation
void test_lock_free_shared_stack() {
    TICK();
    lock_free_shared_stack<unsigned>        lockFreeSharedStack;

    vector<thread>                          vctThreadPush(HARDWARE_CONCURRENCY);
    vector<future<shared_ptr<unsigned>>>    vctFutureResult(HARDWARE_CONCURRENCY);

    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        DEBUG("push(%d)", i + 1);
        vctThreadPush[i] = thread(&lock_free_shared_stack<unsigned>::push, &lockFreeSharedStack, i + 1);
        vctFutureResult[i] = async(&lock_free_shared_stack<unsigned>::pop, &lockFreeSharedStack);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        INFO("pop()=%d", *vctFutureResult[i].get());
    }
}

//Listing 7.10 Pushing a node on a lock-free stack using split reference counts
void test_lock_free_split_ref_cnt_stack() {
    TICK();
    lock_free_split_ref_cnt_stack<unsigned> lockFreeSplitRefCntStack;

    vector<thread>                          vctThreadPush(HARDWARE_CONCURRENCY);
    vector<future<shared_ptr<unsigned>>>    vctFutureResult(HARDWARE_CONCURRENCY);

    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        DEBUG("push(%d)", i + 1);
        vctThreadPush[i] = thread(&lock_free_split_ref_cnt_stack<unsigned>::push, &lockFreeSplitRefCntStack, i + 1);
        vctFutureResult[i] = async(&lock_free_split_ref_cnt_stack<unsigned>::pop, &lockFreeSplitRefCntStack);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        shared_ptr<unsigned> ptr = vctFutureResult[i].get();
        INFO("pop()=%d", ptr ? *ptr : 0);
    }
}

//7.2.5 Appling the memory model to the lock-free stack
//Listing 7.12 A lock-free stack with reference counting and relaxed atomic operations
void test_lock_free_memory_split_ref_cnt_stack() {
    TICK();
    lock_free_memory_split_ref_cnt_stack<unsigned>  lockFreeMemorySplitRefCntStack;

    vector<thread>                                  vctThreadPush(HARDWARE_CONCURRENCY);
    vector<future<shared_ptr<unsigned>>>            vctFutureResult(HARDWARE_CONCURRENCY);

    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        DEBUG("push(%d)", i + 1);
        vctThreadPush[i] = thread(&lock_free_memory_split_ref_cnt_stack<unsigned>::push,
            &lockFreeMemorySplitRefCntStack, i + 1);
        vctFutureResult[i] = async(&lock_free_memory_split_ref_cnt_stack<unsigned>::pop,
            &lockFreeMemorySplitRefCntStack);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        shared_ptr<unsigned> ptr = vctFutureResult[i].get();
        INFO("pop()=%d", ptr ? *ptr : 0);
    }
}

//7.2.6 Writing a thread-safe queue without lock
//Listing 7.13 A single-producer, single-consumer lock-free queue
void test_lock_free_queue() {
    TICK();
    lock_free_queue<unsigned>               lockFreeQueue;

    vector<thread>                          vctThreadPush(HARDWARE_CONCURRENCY);
    vector<future<shared_ptr<unsigned>>>    vctFutureResult(HARDWARE_CONCURRENCY);

    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        DEBUG("push(%d)", i + 1);
        vctThreadPush[i] = thread(&lock_free_queue<unsigned>::push, &lockFreeQueue, i + 1);
        vctFutureResult[i] = async(&lock_free_queue<unsigned>::pop, &lockFreeQueue);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreadPush[i].join();
        shared_ptr<unsigned> ptr = vctFutureResult[i].get();
        INFO("pop()=%d", ptr ? *ptr : 0);
    }
}

}//namespace lock_free_conc_data


