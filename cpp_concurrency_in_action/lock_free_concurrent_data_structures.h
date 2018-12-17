///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter7: Designing lock-free concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/16
#pragma once
#ifndef LOCK_FREE_CONCURRENT_DATA_STRUCTURES_H
#define LOCK_FREE_CONCURRENT_DATA_STRUCTURES_H

namespace lock_free_conc_data {

//7.1 Definitions and consequences
//7.1.1 Types of nonblocking data structures
//Listing 7.1 Implementation of a spin-lock mutex using std::atmoic_flag
class spinlock_mutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    spinlock_mutex() {}
    void lock() {
        TICK();
        //atomically set the flag to true and return previous value
        while (flag.test_and_set(std::memory_order_acquire)) {
            WARN("lock loop");
        }
    }
    void unlock() {
        TICK();
        flag.clear(std::memory_order_release);
    }
};
void spinlock_mutex_plus();
void spinlock_mutex_test();

//7.1.2 Lock-free data structures
//7.1.3 Wait-free data sturctures
//7.1.4 The pros and cons of lock-free data structures

//7.2 Example of lock-free data structures
//7.2.1 Writing a thread-safe stack without locks
//Listing 7.2 Implementing push() without locks
template<typename T>
class lock_free_stack {
private:
    struct node {
        T data;
        node* next;
        explicit node(T const& data_) : data(data_), next(nullptr) {}
    };
    std::atomic<node*> head;

public:
    void push(T const& data) {
        TICK();
        node* const new_node = new node(data);
        new_node->next = head.load();
        //compare and change value store in head with arg1, arg2
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
            WARN("push loop");
        }
        INFO("push(%d)", data);
    }
    void pop(T& result) {
        TICK();
        node* old_head = head.load();
        while (!head.compare_exchange_weak(old_head, old_head->next)) {
            WARN("pop loop");
        }
        result = old_head->data;
        INFO("pop()=%d", result);
    }
};
void lock_free_stack_test();

//Listing 7.3 A lock-free stack that leaks nodes
template<typename T>
class lock_free_shared_ptr_stack {
private:
    struct node {
        std::shared_ptr<T> data;//Data is now held by pointer
        node* next = nullptr;
        //Create std::shared_ptr for newly allocated T
        explicit node(T const& data_) : data(std::make_shared<T>(data_)) {}
    };
    std::atomic<node*> head;

public:
    void push(T const& data) {
        TICK();
        node* const new_node = new node(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
            WARN("push loop");
        }
    }
    std::shared_ptr<T> pop() {
        TICK();
        node* old_head = head.load();
        //Check old_head is not a null pointer before you dereference it
        while (old_head && !head.compare_exchange_weak(old_head, old_head->next)) {
            WARN("push loop");
        }
        return old_head ? old_head->data : std::make_shared<T>();
    }
};
void lock_free_shared_ptr_stack_test();

}//namespace lock_free_conc_data

#endif  //LOCK_FREE_CONCURRENT_DATA_STRUCTURES_H

