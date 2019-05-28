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
//5.2.2 Operations on atomic_flag
void test_atomic_flag();

//Listing 5.1 Implementation of a spinlock mutex using atomic_flag
class spinlock_mutex {
    atomic_flag m_flag_a = ATOMIC_FLAG_INIT;
public:
    spinlock_mutex() {}
    void lock() {
        TICK();
        //Atomically set flag to true and return previous value
        while (m_flag_a.test_and_set(memory_order::memory_order_acquire)) {
        }
    }
    void unlock() {
        TICK();
        m_flag_a.clear(memory_order::memory_order_release);//Atomically clear flag to false
    }
};
void test_spinlock_mutex();

//5.2.3 Operations on atomic<bool>
void test_atomic_bool();

void test_compare_exchange_weak();

void test_compare_exchange_weak_memory_order();

//5.2.4 Operations on atomic<T*>:pointer arithmetic
class Foo {};
void test_atomic_pointer();

//5.2.5 Operations on standard atomic integral types
//5.2.6 The atomic<> primary class template
//5.2.7 Free functions for atomic operations
void test_atomic_load_store();

//5.3 Synchronizing operations and enforcing ordering
//Listing 5.2 Reading and writing variables from different threads
void test_atomic_sync_from_thread();

//5.3.1 The synchronizes-with relationship
//5.3.2 The happens-before relationship
//Listing 5.3 Order of evaluation of arguments to a function call is unspecified
void test_call_unordered();

//5.3.3 Memory ordering for atomic operations
//Listing 5.4 Sequential consistency implies a total ordering
void test_sequential_consistency();

//Listing 5.5 Relaxed operations have very few ordering requirements
void test_relaxed();

//Listing 5.6 Relaxed operations on multiple threads
void test_relaxed_multi_thread();

//Listing 5.7 Acquire-release doesn`t imply a total ordering
void test_acquire_release();

//Listing 5.8 Acquire-release operations can impose ordering on relaxed operations
void test_acquire_release_relaxed();

//Listing 5.9 Transitive synchronization using acquire and release ordering
void test_transitive_sync_acquire_release();

//Listing 5.10 Using memory_order_consume to synchronize data
void test_consume();

//5.3.4 Release sequences and synchronizes-with
//Listing 5.11 Reading values from a queue with atomic operations
void test_consume_queue();

//5.3.5 Fences
//Listing 5.12 Relaxed operations can be ordered with fences
void test_fences();

//5.3.6 Ordering nonatomic operations with atomics
//Listing 5.13 Enforcing ordering on nonatomic operations
void test_nonatomic();


}//namespace atomic_type

#endif  //ATOMIC_MEMORY_MODEL_OPERATIONS_H
