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

bool expected = false;
std::atomic<bool> b;
void get_atomic_bool() {
    TICK();
    while (bool x = !b.load(std::memory_order_acquire)) {
        INFO("get_atomic_bool x=%s", !x ? "true" : "false");
#if 0
        const unsigned &THREAD_SLEEP_TIME_MS = ONE;
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));
#endif
    }
}
void compare_exchange_weak_bool() {
    TICK();
    while (!b.compare_exchange_weak(expected, true) && !expected) {
        INFO("compare_exchange_weak_bool while");
    }
    bool x = b.load(std::memory_order_acquire);
    INFO("compare_exchange_weak_bool x=%s", x ? "true" : "false");
}
void compare_exchange_weak_test() {
    TICK();
    std::thread t1(get_atomic_bool);
    std::thread t2(compare_exchange_weak_bool);
    t1.join();
    t2.join();
}
void compare_exchange_weak_memory_order_test() {
    TICK();

    std::atomic<bool> b;
    bool expected;
    bool x;
#if 0//running time error: invalid memory_order(_Order2)
    b.compare_exchange_weak(expected, true, std::memory_order_acq_rel, std::memory_order_release);
#else
    b.compare_exchange_weak(expected, true, std::memory_order_acq_rel, std::memory_order_relaxed);
#endif
    x = b.load(std::memory_order_acquire);
    INFO("compare_exchange_weak_bool() x=%s", x ? "true" : "false");

    b.compare_exchange_weak(expected, true, std::memory_order_acq_rel);
    x = b.load(std::memory_order_acquire);
    INFO("compare_exchange_weak_bool x=%s", x ? "true" : "false");
}

//5.2.4 Operations on std::atomic<T*>:pointer arithmetic
void atomic_pointer_test() {
    TICK();
    Foo some_array[5];
    std::atomic<Foo*> p(some_array);
#if 1
    Foo *x = p.fetch_add(2);    //Add 2 to p and return old value
#else
    //x=fetch_add(n) <=equal=> (1)x=p; (2) p+=n;
    Foo *x = p;
    p += 2;
#endif
    assert(x == some_array);
    assert(p.load() == &some_array[2]);

    x = (p -= 1);   //Subtract 1 from p and return new value
    assert(x == &some_array[1]);
    assert(p.load() == &some_array[1]);

    x = p.fetch_add(-1, std::memory_order_release);
    assert(x == &some_array[1]);
    assert(p.load() == &some_array[0]);

#if 1
    x = p.fetch_sub(-4, std::memory_order_release); //Subtract -4 from p and return new value
#else
    //x=fetch_sub(n) <=equal=> (1)x=p; (2) p-=n;
    x = p;
    p -= (-4);
#endif
    assert(x == &some_array[0]);
    assert(p.load() == &some_array[4]);

    x = p.fetch_sub(1, std::memory_order_release);
    assert(x == &some_array[4]);
    assert(p.load() == &some_array[3]);
}

//5.2.5 Operations on standard atomic integral types
//5.2.6 The std::atomic<> primary class template
//5.2.7 Free functions for atomic operations
std::shared_ptr<my_data> p(new my_data(-1));
void process_data(std::shared_ptr<my_data> const &data) {
    TICK();
    INFO("data=%d", *data);
}
void process_global_data() {
    TICK();
    std::shared_ptr<my_data> local = std::atomic_load(&p);
    process_data(local);
}
void update_global_data() {
    TICK();
    INFO("p=%d", *p);
    std::shared_ptr<my_data> local(new my_data(42));
    std::atomic_store(&p, local);
    INFO("local=%d", *local);
    INFO("p=%d", *p);
}
void atomic_load_store_test() {
    TICK();
#if 0
    update_global_data();
    process_global_data();
#else
    std::async(update_global_data);
    std::async(process_global_data);

    std::thread t1(update_global_data);
    std::thread t2(process_global_data);
    t1.join();
    t2.join();
#endif
}

//5.3 Synchronizing operations and enforcing ordering
//Listing 5.2 Reading and writing variables from different threads
std::vector<int> data;
std::atomic<bool> data_ready(false);
void reader_thread() {
    TICK();
    while (!data_ready.load()) {
        common_fun::sleep(ONE);
    }
    INFO("The answer=%d", data[0]);
}
void writer_thread() {
    TICK();
    data.push_back(42);
    data_ready = true;
}
void atomic_sync_from_thread_test() {
    TICK();
    std::thread th_read(reader_thread);
    std::thread th_writer(writer_thread);
    th_read.join();
    th_writer.join();
}

//5.3.1 The synchronizes-with relationship
//5.3.2 The happens-before relationship
//Listing 5.3 Order of evaluation of arguments to a function call is unspecified
void foo(int a, int b) {
    TICK();
    INFO("%d, %d", a, b);
}
int get_num() {
    TICK();
    static int i = 0;
    return ++i;
}
void call_unordered_test() {
    TICK();
    while (true) {
        foo(get_num(), get_num());  //Calls to get_num() are unordered
        common_fun::sleep(5 * HUNDRED);
    }
}

//5.3.3 Memory ordering for atomic operations
//Listing 5.4 Sequential consistency implies a total ordering
std::atomic<bool> x, y;
std::atomic<int> z;
void write_x() {
    TICK();
    x.store(true, std::memory_order_seq_cst);
}
void write_y() {
    TICK();
    y.store(true, std::memory_order_seq_cst);
}
void read_x_then_y() {
    TICK();
    while (!x.load(std::memory_order_seq_cst)) {
    }
    if (y.load(std::memory_order_seq_cst)) {
        ++z;
    }
}
void read_y_then_x() {
    TICK();
    while (!y.load(std::memory_order_seq_cst)) {
    }
    if (x.load(std::memory_order_seq_cst)) {
        ++z;
    }
}
void sequential_consistency_test() {
    TICK();
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x);
    std::thread b(write_y);
    std::thread c(read_x_then_y);
    std::thread d(read_y_then_x);
    a.join();
    b.join();
    c.join();
    d.join();
    assert(z.load() != 0);
}

}//namespace atomic_type
