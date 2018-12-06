///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter5: The C++ memory model and operations on atomic types
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/04
#pragma once
#ifndef ATOMIC_MEMORY_MODEL_OPERATIONS_H
#define ATOMIC_MEMORY_MODEL_OPERATIONS_H

namespace atomic_type {

//5.1 Memory model basics
//5.1.1 Objects and memory locations
//5.1.2 Objects, memory locations, and concurrency
//5.1.3 Modification orders

//5.2 Atomic operations and types in C++
//5.2.1 The standard atomic types
//5.2.2 Operations on std::atomic_flag
void atomic_flag_test();

//Listing 5.1 Implementation of a spinlock mutex using std::atomic_flag
class spinlock_mutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    spinlock_mutex() {}
    void lock() {
        TICK();
        //Atomically set flag to true and return previous value
        while (flag.test_and_set(std::memory_order_acquire)) {
        }
    }
    void unlock() {
        TICK();
        flag.clear(std::memory_order_release);//Atomically clear flag to false
    }
};
void spinlock_mutex_test();

//5.2.3 Operations on std::atomic<bool>
void atomic_bool_test();

void get_atomic_bool();
void compare_exchange_weak_test();
void compare_exchange_weak_memory_order_test();

//5.2.4 Operations on std::atomic<T*>:pointer arithmetic
class Foo {};
void atomic_pointer_test();

//5.2.5 Operations on standard atomic integral types
//5.2.6 The std::atomic<> primary class template
//5.2.7 Free functions for atomic operations
typedef unsigned my_data;
void process_data(std::shared_ptr<my_data>  const &data);
void process_global_data();
void update_global_data();
void atomic_load_store_test();

//5.3 Synchronizing operations and enforcing ordering
//Listing 5.2 Reading and writing variables from different threads
void reader_thread();
void writer_thread();
void atomic_sync_from_thread_test();

//5.3.1 The synchronizes-with relationship
//5.3.2 The happens-before relationship
//Listing 5.3 Order of evaluation of arguments to a function call is unspecified
void foo(int a, int b);
int get_num();
void call_unordered_test();

//5.3.3 Memory ordering for atomic operations
//Listing 5.4 Sequential consistency implies a total ordering
void write_x();
void write_y();
void read_x_then_y();
void read_y_then_x();
void sequential_consistency_test();

//Listing 5.5 Relaxed operations have very few ordering requirements
void write_x_then_y_relaxed();
void write_y_then_x_relaxed();
void relaxed_test();

//Listing 5.6 Relaxed operations on multiple threads
struct read_values {
    int x, y, z;
};
void increment(std::atomic<int>* var_to_inc, read_values* values);
void read_vals(read_values* values);
void print(read_values *v);
void relaxed_multi_thread_test();

}//namespace atomic_type

#endif  //ATOMIC_MEMORY_MODEL_OPERATIONS_H
