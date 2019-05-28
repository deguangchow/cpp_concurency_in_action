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
//5.2.2 Operations on atomic_flag
void test_atomic_flag() {
    TICK();
    atomic_flag flag_a = ATOMIC_FLAG_INIT;
    flag_a.clear(memory_order::memory_order_release);

    bool bRetX = flag_a.test_and_set();
    INFO("bRetX=%s", bRetX ? "true" : "false");

    bool bRetY = flag_a.test_and_set();
    INFO("bRetY=%s", bRetY ? "true" : "false");
}

//Listing 5.1 Implementation of a spinlock mutex using atomic_flag
spinlock_mutex          g_spinlokcMutex;
void task1() {
    TICK();
    while (true) {
        unique_lock<spinlock_mutex> lock(g_spinlokcMutex);
        DEBUG("task1----------------------------------------------------");
        yield();
    }
}
void task2() {
    TICK();
    while (true) {
        unique_lock<spinlock_mutex> lock(g_spinlokcMutex);
        INFO("task2++++++++++++++++++++++++++++++++++++++++++++++++++++");
        yield();
    }
}
void test_spinlock_mutex() {
    TICK();
    thread t1(task1);
    thread t2(task2);
    t1.join();
    t2.join();
}

//5.2.3 Operations on atomic<bool>
void test_atomic_bool() {
    TICK();
    atomic<bool>    bX_a = true;
    bool            bX = bX_a.load(memory_order::memory_order_acquire);
    INFO("x=%s", bX ? "true" : "false");

    bX_a.store(false);
    bX = bX_a.load(memory_order::memory_order_acquire);
    INFO("x=%s", bX ? "true" : "false");

    bX_a.store(true);
    bX = bX_a.load(memory_order::memory_order_acquire);
    INFO("x=%s", bX ? "true" : "false");

    auto const& bY = bX_a.exchange(false, memory_order::memory_order_acq_rel);
    bX = bX_a.load(memory_order::memory_order_acquire);
    INFO("x=%s, y=%s", bX ? "true" : "false", bY ? "true" : "false");
}

bool                        g_bExpected = false;
atomic<bool>                g_bX_a = false;
void get_atomic_bool() {
    TICK();
    while (bool bX = !g_bX_a.load(memory_order::memory_order_acquire)) {
        WARN("get_atomic_bool() loop...");
        INFO("get_atomic_bool() x=%s", !bX ? "true" : "false");
#if 0
        const unsigned &THREAD_SLEEP_TIME_MS = ONE;
        sleep_for(chrono::milliseconds(THREAD_SLEEP_TIME_MS));
#endif
        yield();
    }
}
void compare_exchange_weak_bool() {
    TICK();
    while (!g_bX_a.compare_exchange_weak(g_bExpected, true) && !g_bExpected) {
        WARN("compare_exchange_weak_bool() loop...");
        yield();
    }
    bool bX = g_bX_a.load(memory_order::memory_order_acquire);
    INFO("compare_exchange_weak_bool() x=%s", bX ? "true" : "false");
}
void test_compare_exchange_weak() {
    TICK();
    thread t1(get_atomic_bool);
    thread t2(compare_exchange_weak_bool);
    t1.join();
    t2.join();
}
void test_compare_exchange_weak_memory_order() {
    TICK();

    atomic<bool>    bX_a = false;
    bool            bExpected = false;
    bool            bX = true;
#if 0//running time error: invalid memory_order(_Order2)
    bX_a.compare_exchange_weak(bExpected, true, memory_order::memory_order_acq_rel, memory_order::memory_order_release);
#else
    bX_a.compare_exchange_weak(bExpected, true, memory_order::memory_order_acq_rel, memory_order::memory_order_relaxed);
#endif
    bX = bX_a.load(memory_order::memory_order_acquire);
    INFO("x=%s", bX ? "true" : "false");

    bX_a.compare_exchange_weak(bExpected, true, memory_order::memory_order_acq_rel);
    bX = bX_a.load(memory_order::memory_order_acquire);
    INFO("x=%s", bX ? "true" : "false");
}

//5.2.4 Operations on atomic<T*>:pointer arithmetic
void test_atomic_pointer() {
    TICK();
    Foo arrayFoos[5];
    atomic<Foo*> pFoo(arrayFoos);
#if 1
    Foo *pFooTmp = pFoo.fetch_add(2);    //Add 2 to p and return old value
#else
    //x=fetch_add(n) <=equal=> (1)x=p; (2) p+=n;
    Foo *pFooTmp = pFoo;
    pFoo += 2;
#endif
    assert(pFooTmp == arrayFoos);
    assert(pFoo.load() == &arrayFoos[2]);

    pFooTmp = (pFoo -= 1);   //Subtract 1 from p and return new value
    assert(pFooTmp == &arrayFoos[1]);
    assert(pFoo.load() == &arrayFoos[1]);

    pFooTmp = pFoo.fetch_add(-1, memory_order::memory_order_release);
    assert(pFooTmp == &arrayFoos[1]);
    assert(pFoo.load() == &arrayFoos[0]);

#if 1
    pFooTmp = pFoo.fetch_sub(-4, memory_order::memory_order_release); //Subtract -4 from p and return new value
#else
    //x=fetch_sub(n) <=equal=> (1)x=p; (2) p-=n;
    pFooTmp = pFoo;
    pFoo -= (-4);
#endif
    assert(pFooTmp == &arrayFoos[0]);
    assert(pFoo.load() == &arrayFoos[4]);

    pFooTmp = pFoo.fetch_sub(1, memory_order::memory_order_release);
    assert(pFooTmp == &arrayFoos[4]);
    assert(pFoo.load() == &arrayFoos[3]);
}

//5.2.5 Operations on standard atomic integral types
//5.2.6 The atomic<> primary class template
//5.2.7 Free functions for atomic operations
typedef unsigned my_data;
shared_ptr<my_data> g_ptrMyData(new my_data(-1));
void process_data(shared_ptr<my_data> const &data) {
    TICK();
    INFO("data=%d", *data);
}
void process_global_data() {
    TICK();
    shared_ptr<my_data> ptrLocal = atomic_load(&g_ptrMyData);
    process_data(ptrLocal);
}
void update_global_data() {
    TICK();
    INFO("data=%d", *g_ptrMyData);
    shared_ptr<my_data> ptrLocal(new my_data(42));
    atomic_store(&g_ptrMyData, ptrLocal);
    DEBUG("local=%d", *ptrLocal);
    INFO("data=%d", *g_ptrMyData);
}
void test_atomic_load_store() {
    TICK();
#if 0
    update_global_data();
    process_global_data();
#else
    async(update_global_data);
    async(process_global_data);

    thread t1(update_global_data);
    thread t2(process_global_data);
    t1.join();
    t2.join();
#endif
}

//5.3 Synchronizing operations and enforcing ordering
//Listing 5.2 Reading and writing variables from different threads
vector<int>         g_vctData;
atomic<bool>        g_bDataReady_a(false);
void reader_thread() {
    TICK();
    while (!g_bDataReady_a.load()) {
        common_fun::sleep(ONE);
    }
    INFO("The answer=%d", g_vctData[0]);
}
void writer_thread() {
    TICK();
    g_vctData.push_back(42);
    g_bDataReady_a = true;
}
void test_atomic_sync_from_thread() {
    TICK();
    thread threadRead(reader_thread);
    thread threadWriter(writer_thread);
    threadRead.join();
    threadWriter.join();
}

//5.3.1 The synchronizes-with relationship
//5.3.2 The happens-before relationship
//Listing 5.3 Order of evaluation of arguments to a function call is unspecified
void foo(int a, int b) {
    TICK();
    INFO("foo(%d, %d)", a, b);
}
int get_num() {
    TICK();
    static int i = 0;
    return ++i;
}
void test_call_unordered() {
    TICK();
    while (true) {
        foo(get_num(), get_num());  //Calls to get_num() are unordered
        common_fun::sleep(THOUSAND);
    }
}

//5.3.3 Memory ordering for atomic operations
//Listing 5.4 Sequential consistency implies a total ordering
atomic<bool>    g_bXX_a, g_bYY_a;
atomic<int>     g_nZZ_a;
void write_x() {
    TICK();
    g_bXX_a.store(true, memory_order::memory_order_seq_cst);
}
void write_y() {
    TICK();
    g_bYY_a.store(true, memory_order::memory_order_seq_cst);
}
void read_x_then_y() {
    TICK();
    while (!g_bXX_a.load(memory_order::memory_order_seq_cst)) {
        WARN("read_x_then_y() loop...");
        yield();
    }
    if (g_bYY_a.load(memory_order::memory_order_seq_cst)) {
        ++g_nZZ_a;
    }
}
void read_y_then_x() {
    TICK();
    while (!g_bYY_a.load(memory_order::memory_order_seq_cst)) {
        WARN("read_y_then_x() loop...");
        yield();
    }
    if (g_bXX_a.load(memory_order::memory_order_seq_cst)) {
        ++g_nZZ_a;
    }
}
void test_sequential_consistency() {
    TICK();
    g_bXX_a = false;
    g_bYY_a = false;
    g_nZZ_a = 0;

    thread a(write_x);
    thread b(write_y);
    thread c(read_x_then_y);
    thread d(read_y_then_x);

    a.join();
    b.join();
    c.join();
    d.join();

    assert(g_nZZ_a.load() != 0);
}


//Listing 5.5 Relaxed operations have very few ordering requirements
void write_x_then_y_relaxed() {
    TICK();
    common_fun::sleep(ONE);
    g_bXX_a.store(true, memory_order::memory_order_relaxed);
    g_bYY_a.store(true, memory_order::memory_order_relaxed);
}
void write_y_then_x_relaxed() {
    TICK();
    while (!g_bYY_a.load(memory_order::memory_order_relaxed)) {
        WARN("write_y_then_x_relaxed() loop...");
    }
    if (g_bXX_a.load(memory_order::memory_order_relaxed)) {
        ++g_nZZ_a;
    }
}
void test_relaxed() {
    TICK();
    g_bXX_a = false;
    g_bYY_a = false;
    g_nZZ_a = 0;
    thread a(write_x_then_y_relaxed);
    thread b(write_y_then_x_relaxed);
    a.join();
    b.join();

    INFO("z=%d", g_nZZ_a.load());
    assert(g_nZZ_a.load() != 0);
}

//Listing 5.6 Relaxed operations on multiple threads
atomic<int>     g_nX_a(0), g_nY_a(0), g_nZ_a(0);
atomic<bool>    g_bGo_a(false);
unsigned const  kLoopCount = 10;
struct read_values {
    int x, y, z;
};
read_values g_values1[kLoopCount];
read_values g_values2[kLoopCount];
read_values g_values3[kLoopCount];
read_values g_values4[kLoopCount];
read_values g_values5[kLoopCount];
void increment(atomic<int>* pnVarToInc_a, read_values* pReadValues) {
    TICK();
    while (!g_bGo_a) {   //Spin, waiting for the signal
        WARN("increment() loop...");
        yield();
    }
    for (unsigned i = 0; i < kLoopCount; ++i) {
        pReadValues[i].x = g_nX_a.load(memory_order::memory_order_relaxed);
        pReadValues[i].y = g_nY_a.load(memory_order::memory_order_relaxed);
        pReadValues[i].z = g_nZ_a.load(memory_order::memory_order_relaxed);
        pnVarToInc_a->store(i + 1, memory_order::memory_order_relaxed);
        DEBUG("%d", pnVarToInc_a->load(memory_order::memory_order_relaxed));
        yield();
    }
}
void load_vals(read_values* pReadValues) {
    TICK();
    while (!g_bGo_a) {   //Spin, waiting for the signal
        WARN("read_vals() loop...");
        yield();
    }
    for (unsigned i = 0; i < kLoopCount; ++i) {
        pReadValues[i].x = g_nX_a.load(memory_order::memory_order_relaxed);
        pReadValues[i].y = g_nY_a.load(memory_order::memory_order_relaxed);
        pReadValues[i].z = g_nZ_a.load(memory_order::memory_order_relaxed);
        yield();
    }
}
void print(read_values *v) {
    TICK();
    for (unsigned i = 0; i < kLoopCount; ++i) {
        INFO("i=%d, x=%d, y=%d, z=%d", i, v[i].x, v[i].y, v[i].z);
    }
}
void test_relaxed_multi_thread() {
    TICK();
    thread t1(increment, &g_nX_a, g_values1);
    thread t2(increment, &g_nY_a, g_values2);
    thread t3(increment, &g_nZ_a, g_values3);
    thread t4(load_vals, g_values4);
    thread t5(load_vals, g_values5);

    g_bGo_a = true;

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();

    print(g_values1);
    print(g_values2);
    print(g_values3);
    print(g_values4);
    print(g_values5);
}


//Listing 5.7 Acquire-release doesn`t imply a total ordering
void write_x_release() {
    TICK();
    common_fun::sleep(TEN);
    g_bXX_a.store(true, memory_order::memory_order_release);
}
void write_y_release() {
    TICK();
    common_fun::sleep(TEN);
    g_bYY_a.store(true, memory_order::memory_order_release);
}
void read_x_then_y_acquire() {
    TICK();
    while (!g_bXX_a.load(memory_order::memory_order_acquire)) {
        WARN("read_x_then_y_acquire() loop...");
        yield();
    }
    if (g_bYY_a.load(memory_order::memory_order_acquire)) {
        ++g_nZZ_a;
        INFO("read_x_then_y_acquire() z=%d", g_nZZ_a);
    }
}
void read_y_then_x_acquire() {
    TICK();
    while (!g_bYY_a.load(memory_order::memory_order_acquire)) {
        WARN("read_y_then_x_acquire() loop...");
        yield();
    }
    if (g_bXX_a.load(memory_order::memory_order_acquire)) {
        ++g_nZZ_a;
        INFO("read_y_then_x_acquire() z=%d", g_nZZ_a);
    }
}
void test_acquire_release() {
    TICK();
    g_bXX_a = false;
    g_bYY_a = false;
    g_nZZ_a = 0;

    thread a(write_x_release);
    thread b(write_y_release);

    thread c(read_x_then_y_acquire);
    thread d(read_y_then_x_acquire);

    a.join();
    b.join();
    c.join();
    d.join();

    assert(g_nZZ_a.load() != 0);
}

//Listing 5.8 Acquire-release operations can impose ordering on relaxed operations
void write_x_then_y_relaxed_release() {
    TICK();
    common_fun::sleep(TEN);
    g_bXX_a.store(true, memory_order::memory_order_relaxed);
    g_bYY_a.store(true, memory_order::memory_order_release);
}
void read_y_then_x_acquire_relaxed() {
    TICK();
    while (!g_bYY_a.load(memory_order::memory_order_acquire)) {
        WARN("read_y_then_x_acquire_relaxed() loop...");
        yield();
    }
    if (g_bXX_a.load(memory_order::memory_order_relaxed)) {
        ++g_nZZ_a;
        INFO("z=%d", g_nZZ_a);
    }
}
void test_acquire_release_relaxed()  {
    TICK();
    g_bXX_a = false;
    g_bYY_a = false;
    g_nZZ_a = 0;

    thread a(write_x_then_y_relaxed_release);
    thread b(read_y_then_x_acquire_relaxed);

    a.join();
    b.join();

    assert(g_nZZ_a.load() != 0);
}

//Listing 5.9 Transitive synchronization using acquire and release ordering
atomic<int>         g_nData_a[5];
atomic<bool>        g_bSync1(false), g_bSync2(false);
void thread_1() {
    TICK();
    common_fun::sleep(ONE);
    g_nData_a[0].store(42, memory_order::memory_order_relaxed);
    g_nData_a[1].store(97, memory_order::memory_order_relaxed);
    g_nData_a[2].store(17, memory_order::memory_order_relaxed);
    g_nData_a[3].store(-141, memory_order::memory_order_relaxed);
    g_nData_a[4].store(2003, memory_order::memory_order_relaxed);
    g_bSync1.store(true, memory_order::memory_order_release);   //Set g_bSync1
}
void thread_2() {
    TICK();
    while (!g_bSync1.load(memory_order::memory_order_acquire)) {//Loop until g_bSync1 is set
        WARN("thread_2() loop...");
        yield();
    }
    common_fun::sleep(ONE);
    g_bSync2.store(true, memory_order::memory_order_release);   //Set g_bSync2
}
void thread_3() {
    TICK();
    while (!g_bSync2.load(memory_order::memory_order_acquire)) {//Loop until g_bSync2 is set
        WARN("thread_3() loop...");
        yield();
    }

    INFO("atomic_data={%d, %d, %d, %d, %d}",
        g_nData_a[0].load(memory_order::memory_order_relaxed),
        g_nData_a[1].load(memory_order::memory_order_relaxed),
        g_nData_a[2].load(memory_order::memory_order_relaxed),
        g_nData_a[3].load(memory_order::memory_order_relaxed),
        g_nData_a[4].load(memory_order::memory_order_relaxed));

    assert(g_nData_a[0].load(memory_order::memory_order_relaxed) == 42);
    assert(g_nData_a[1].load(memory_order::memory_order_relaxed) == 97);
    assert(g_nData_a[2].load(memory_order::memory_order_relaxed) == 17);
    assert(g_nData_a[3].load(memory_order::memory_order_relaxed) == -141);
    assert(g_nData_a[4].load(memory_order::memory_order_relaxed) == 2003);
}
void test_transitive_sync_acquire_release() {
    TICK();
    thread t1(thread_1);
    thread t2(thread_2);
    thread t3(thread_3);
    t1.join();
    t2.join();
    t3.join();
}

//Listing 5.10 Using memory_order_consume to synchronize data
struct X1 {
    int     i;
    string  s;
};
atomic<X1*> g_pX1_a;
atomic<int> g_nA_a;
void create_x() {
    TICK();
    common_fun::sleep(ONE);
    X1 *x = new X1;
    x->i = 42;
    x->s = "hello";
    g_nA_a.store(99, memory_order::memory_order_relaxed);
    g_pX1_a.store(x, memory_order::memory_order_relaxed);
}
void use_x() {
    TICK();
    X1* x;
    while (!(x = g_pX1_a.load(memory_order::memory_order_consume))) {
        WARN("use_x() loop...");
        yield();
    }
    INFO("x={%d, %s}, g_nA_a=%d", x->i, x->s.c_str(), g_nA_a.load(memory_order::memory_order_relaxed));
    assert(x->i == 42);
    assert(x->s == "hello");
    assert(g_nA_a.load(memory_order::memory_order_relaxed) == 99);
}
void test_consume() {
    TICK();
    thread t1(create_x);
    thread t2(use_x);
    t1.join();
    t2.join();
}

//5.3.4 Release sequences and synchronizes-with
//Listing 5.11 Reading values from a queue with atomic operations
vector<int> g_vctQueueData;
atomic<int> g_bCount_a;
unsigned    g_uTotalCount;
void populate_queue() {
    TICK();
    unsigned const uNumberOfItem = 20;
    g_vctQueueData.clear();
    for (unsigned i = 0; i < uNumberOfItem; ++i) {
        g_vctQueueData.push_back(i);
    }
    g_bCount_a.store(uNumberOfItem, memory_order::memory_order_release);//The initial store

    g_uTotalCount = uNumberOfItem;
}
void wait_for_more_items() {
    TICK();
    unsigned const uNumberOfItem = 10;
    for (unsigned i = 20; i < 20 + uNumberOfItem; ++i) {
        g_vctQueueData.push_back(i);
    }

    g_uTotalCount += uNumberOfItem;
    g_bCount_a.store(g_uTotalCount, memory_order::memory_order_release);//The initial store
}
void process(int const &val) {
    TICK();
    INFO("val=%d", val);
    common_fun::sleep(TEN * HUNDRED);
}
void consume_queue_item() {
    TICK();
    while (true) {
        WARN("consume_queue_item() loop...");
        int item_index;
        if ((item_index = g_bCount_a.fetch_sub(1, memory_order::memory_order_acquire)) <= 0) {// An RMW operation
            wait_for_more_items();//Wait for more items
            yield();
            continue;
        }
        process(g_vctQueueData[item_index - 1]);//Reading queue_datas is safe
        yield();
    }
}
void test_consume_queue() {
    TICK();
    thread a(populate_queue);
    thread b(consume_queue_item);
    thread c(consume_queue_item);
    a.join();
    b.join();
    c.join();
}

//5.3.5 Fences
//Listing 5.12 Relaxed operations can be ordered with fences
void write_x_then_y_fence() {
    TICK();
    common_fun::sleep(ONE);
#if 1
    g_bXX_a.store(true, memory_order::memory_order_relaxed);
    atomic_thread_fence(memory_order::memory_order_release);
    g_bYY_a.store(true, memory_order::memory_order_relaxed);
#else
    atomic_thread_fence(memory_order::memory_order_release);
    g_bXX_a.store(true, memory_order::memory_order_relaxed);
    g_bYY_a.store(true, memory_order::memory_order_relaxed);

#endif
}
void read_y_then_x_fence() {
    TICK();
    while (!g_bYY_a.load(memory_order::memory_order_relaxed)) {
        WARN("read_y_then_x_fence() loop...");
        yield();
    }
    atomic_thread_fence(memory_order::memory_order_acquire);
    if (g_bXX_a.load(memory_order::memory_order_relaxed)) {
        ++g_nZZ_a;
        INFO("z=%d", g_nZZ_a);
    }
}
void test_fences() {
    TICK();
    g_bXX_a = false;
    g_bYY_a = false;
    g_nZZ_a = 0;

    thread a(write_x_then_y_fence);
    thread b(read_y_then_x_fence);

    a.join();
    b.join();

    assert(g_nZZ_a.load() != 0);
}


//5.3.6 Ordering nonatomic operations with atomics
//Listing 5.13 Enforcing ordering on nonatomic operations
bool g_bX = false;//x is now a plain nonatomic variable
#if 0
atomic<bool> y;
atomic<int> z;
#endif
void write_x_then_y_nonatomic() {
    TICK();
    common_fun::sleep(ONE);
    g_bX = true;//#1 Store to x before the fence
    atomic_thread_fence(memory_order::memory_order_release);
    g_bYY_a.store(true, memory_order::memory_order_relaxed);//#2 Store to y after the fence
}
void read_y_then_x_nonatomic() {
    TICK();
    while (!g_bYY_a.load(memory_order::memory_order_relaxed)) {//#3 Wait until you see the write from #2
        WARN("read_y_then_x_nonatomic() loop...");
    }
    atomic_thread_fence(memory_order::memory_order_acquire);
    if (g_bX) {
        ++g_nZZ_a;//This will read the value written by #1
        INFO("z=%d", g_nZZ_a);
    }
}
void test_nonatomic() {
    TICK();
    g_bX    = false;
    g_bYY_a = false;
    g_nZZ_a = 0;

    thread a(write_x_then_y_nonatomic);
    thread b(read_y_then_x_nonatomic);

    a.join();
    b.join();

    assert(g_nZZ_a.load() != 0);
}

}//namespace atomic_type

