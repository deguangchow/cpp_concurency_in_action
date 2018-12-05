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


}//namespace atomic_type

#endif  //ATOMIC_MEMORY_MODEL_OPERATIONS_H
