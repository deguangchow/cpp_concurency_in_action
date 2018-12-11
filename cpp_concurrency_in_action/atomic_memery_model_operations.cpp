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


//Listing 5.5 Relaxed operations have very few ordering requirements
void write_x_then_y_relaxed() {
    TICK();
    common_fun::sleep(ONE);
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}
void write_y_then_x_relaxed() {
    TICK();
    while (!y.load(std::memory_order_relaxed)) {
        WARN("Loop");
    }
    if (x.load(std::memory_order_relaxed)) {
        ++z;
    }
}
void relaxed_test() {
    TICK();
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y_relaxed);
    std::thread b(write_y_then_x_relaxed);
    a.join();
    b.join();

    INFO("z=%d", z.load());
    assert(z.load() != 0);
}

//Listing 5.6 Relaxed operations on multiple threads
std::atomic<int> X(0), Y(0), Z(0);
std::atomic<bool> go(false);
unsigned const kLoopCount = 10;
read_values values1[kLoopCount];
read_values values2[kLoopCount];
read_values values3[kLoopCount];
read_values values4[kLoopCount];
read_values values5[kLoopCount];
void increment(std::atomic<int>* var_to_inc, read_values* values) {
    TICK();
    while (!go) {   //Spin, waiting for the signal
        WARN("Loop1");
        std::this_thread::yield();
    }
    for (unsigned i = 0; i < kLoopCount; ++i) {
        values[i].x = X.load(std::memory_order_relaxed);
        values[i].y = Y.load(std::memory_order_relaxed);
        values[i].z = Z.load(std::memory_order_relaxed);
        var_to_inc->store(i + 1, std::memory_order_relaxed);
        std::this_thread::yield();
    }
}
void read_vals(read_values* values) {
    TICK();
    while (!go) {   //Spin, waiting for the signal
        WARN("Loop2");
        std::this_thread::yield();
    }
    for (unsigned i = 0; i < kLoopCount; ++i) {
        values[i].x = X.load(std::memory_order_relaxed);
        values[i].y = Y.load(std::memory_order_relaxed);
        values[i].z = Z.load(std::memory_order_relaxed);
        std::this_thread::yield();
    }
}
void print(read_values *v) {
    TICK();
    for (unsigned i = 0; i < kLoopCount; ++i) {
        INFO("i=%d, x=%d, y=%d, z=%d", i, v[i].x, v[i].y, v[i].z);
    }
}
void relaxed_multi_thread_test() {
    TICK();
    std::thread t1(increment, &X, values1);
    std::thread t2(increment, &Y, values2);
    std::thread t3(increment, &Z, values3);
    std::thread t4(read_vals, values4);
    std::thread t5(read_vals, values5);

    go = true;

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();

    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values5);
}


//Listing 5.7 Acquire-release doesn`t imply a total ordering
void write_x_release() {
    TICK();
    common_fun::sleep(TEN);
    x.store(true, std::memory_order_release);
}
void write_y_release() {
    TICK();
    common_fun::sleep(TEN);
    y.store(true, std::memory_order_release);
}
void read_x_then_y_acquire() {
    TICK();
    while (!x.load(std::memory_order_acquire)) {
        WARN("Loop1");
    }
    if (y.load(std::memory_order_acquire)) {
        ++z;
        INFO("1 z=%d", z);
    }
}
void read_y_then_x_acquire() {
    TICK();
    while (!y.load(std::memory_order_acquire)) {
        WARN("Loop2");
    }
    if (x.load(std::memory_order_acquire)) {
        ++z;
        INFO("2 z=%d", z);
    }
}
void acquire_release_test() {
    TICK();
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_release);
    std::thread b(write_y_release);
    std::thread c(read_x_then_y_acquire);
    std::thread d(read_y_then_x_acquire);
    a.join();
    b.join();
    c.join();
    d.join();
    assert(z.load() != 0);
}

//Listing 5.8 Acquire-release operations can impose ordering on relaxed operations
void write_x_then_y_relaxed_release() {
    TICK();
    common_fun::sleep(ONE);
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_release);
}
void read_y_then_x_acquire_relaxed() {
    TICK();
    while (!y.load(std::memory_order_acquire)) {
        WARN("Loop");
    }
    if (x.load(std::memory_order_relaxed)) {
        ++z;
        INFO("z=%d", z);
    }
}
void acquire_release_relaxed_test()  {
    TICK();
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y_relaxed_release);
    std::thread b(read_y_then_x_acquire_relaxed);
    a.join();
    b.join();
    assert(z.load() != 0);
}

//Listing 5.9 Transitive synchronization using acquire and release ordering
std::atomic<int> atomic_data[5];
std::atomic<bool> sync1(false), sync2(false);
void thread_1() {
    TICK();
    common_fun::sleep(ONE);
    atomic_data[0].store(42, std::memory_order_relaxed);
    atomic_data[1].store(97, std::memory_order_relaxed);
    atomic_data[2].store(17, std::memory_order_relaxed);
    atomic_data[3].store(-141, std::memory_order_relaxed);
    atomic_data[4].store(2003, std::memory_order_relaxed);
    sync1.store(true, std::memory_order_release);   //Set sync1
}
void thread_2() {
    TICK();
    while (!sync1.load(std::memory_order_acquire)) {//Loop until sync1 is set
        WARN("thread_2 Loop");
    }
    common_fun::sleep(ONE);
    sync2.store(true, std::memory_order_release);   //Set sync2
}
void thread_3() {
    TICK();
    while (!sync2.load(std::memory_order_acquire)) {//Loop until sync2 is set
        WARN("thread_3 Loop");
    }

    INFO("atomic_data={%d, %d, %d, %d, %d}",
        atomic_data[0].load(std::memory_order_relaxed),
        atomic_data[1].load(std::memory_order_relaxed),
        atomic_data[2].load(std::memory_order_relaxed),
        atomic_data[3].load(std::memory_order_relaxed),
        atomic_data[4].load(std::memory_order_relaxed));

    assert(atomic_data[0].load(std::memory_order_relaxed) == 42);
    assert(atomic_data[1].load(std::memory_order_relaxed) == 97);
    assert(atomic_data[2].load(std::memory_order_relaxed) == 17);
    assert(atomic_data[3].load(std::memory_order_relaxed) == -141);
    assert(atomic_data[4].load(std::memory_order_relaxed) == 2003);
}
void transitive_sync_acquire_release() {
    TICK();
    std::thread t1(thread_1);
    std::thread t2(thread_2);
    std::thread t3(thread_3);
    t1.join();
    t2.join();
    t3.join();
}

//Listing 5.10 Using std::memory_order_consume to synchronize data
std::atomic<X1*> pX1;
std::atomic<int> a;
void create_x() {
    TICK();
    common_fun::sleep(ONE);
    X1 *x = new X1;
    x->i = 42;
    x->s = "hello";
    a.store(99, std::memory_order_relaxed);
    pX1.store(x, std::memory_order_relaxed);
}
void use_x() {
    TICK();
    X1* x;
    while (!(x = pX1.load(std::memory_order_consume))) {
        WARN("Loop");
    }
    INFO("x={%d, %s}, a=%d", x->i, x->s.c_str(), a.load(std::memory_order_relaxed));
    assert(x->i == 42);
    assert(x->s == "hello");
    assert(a.load(std::memory_order_relaxed) == 99);
}
void consume_test() {
    std::thread t1(create_x);
    std::thread t2(use_x);
    t1.join();
    t2.join();
}

//5.3.4 Release sequences and synchronizes-with
//Listing 5.11 Reading values from a queue with atomic operations
std::vector<int> queue_data;
std::atomic<int> count;
void populate_queue() {
    TICK();
    unsigned const number_of_item = 20;
    queue_data.clear();
    for (unsigned i = 0; i < number_of_item; ++i) {
        queue_data.push_back(i);
    }
    count.store(number_of_item, std::memory_order_release);//The initial store
}
void wait_for_more_items() {
    TICK();
}
void process(int const &val) {
    TICK();
    INFO("val=%d", val);
    common_fun::sleep(TEN * HUNDRED);
}
void consume_queue_item() {
    TICK();
    while (true) {
        int item_index;
        if ((item_index = count.fetch_sub(1, std::memory_order_acquire)) <= 0) {// An RMW operation
            wait_for_more_items();//Wait for more items
            continue;
        }
        process(queue_data[item_index - 1]);//Reading queue_datas is safe
    }
}
void consume_queue_test() {
    TICK();
    std::thread a(populate_queue);
    std::thread b(consume_queue_item);
    std::thread c(consume_queue_item);
    a.join();
    b.join();
    c.join();
}

//5.3.5 Fences
//Listing 5.12 Relaxed operations can be ordered with fences
void write_x_then_y_fence() {
    TICK();
    common_fun::sleep(ONE);
#if 0
    x.store(true, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed);
#else
    std::atomic_thread_fence(std::memory_order_release);
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);

#endif
}
void read_y_then_x_fence() {
    TICK();
    while (!y.load(std::memory_order_relaxed)) {
        WARN("loop");
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    if (x.load(std::memory_order_relaxed)) {
        ++z;
        INFO("z=%d", z);
    }
}
void fences_test() {
    TICK();
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y_fence);
    std::thread b(read_y_then_x_fence);
    a.join();
    b.join();
    assert(z.load() != 0);
}


//5.3.6 Ordering nonatomic operations with atomics
//Listing 5.13 Enforcing ordering on nonatomic operations
bool x_nonatomic = false;//x is now a plain nonatomic variable
#if 0
std::atomic<bool> y;
std::atomic<int> z;
#endif
void write_x_then_y_nonatomic() {
    TICK();
    common_fun::sleep(ONE);
    x_nonatomic = true;//#1 Store to x before the fence
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed);//#2 Store to y after the fence
}
void read_y_then_x_nonatomic() {
    TICK();
    while (!y.load(std::memory_order_relaxed)) {//#3 Wait until you see the write from #2
        WARN("Loop");
    }
    std::atomic_thread_fence(std::memory_order_acquire);
    if (x_nonatomic) {
        ++z;//This will read the value written by #1
        INFO("z=%d", z);
    }
}
void nonatomic_test() {
    TICK();
    x_nonatomic = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y_nonatomic);
    std::thread b(read_y_then_x_nonatomic);
    a.join();
    b.join();
    assert(z.load() != 0);
}

}//namespace atomic_type

