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

//7.2.2 Stopping those pesky leaks: managing memory in lock-free data structures
//Listing 7.4 Reclaiming nodes when no threads are in pop()
template<typename T>
class lock_free_reclaim_stack {
private:
    struct node {
        std::shared_ptr<T> data;
        node* next = nullptr;
        explicit node(T const& data_) : data(std::make_shared<T>(data_)) {}
    };
    std::atomic<node*> head;
    std::atomic<node*> to_be_deleted;
    std::atomic<unsigned> threads_in_pop;//Atomic variable
    static void delete_nodes(node* nodes) {
        TICK();
        while (nodes) {
            node* next = nodes->next;
            delete nodes;
            nodes = next;
        }
    }
    void chain_pending_nodes(node* nodes) {
        TICK();
        node* last = nodes;
        while (node* const next = last->next) {//Follow the next pointer chain to the end
            last = next;
        }
        chain_pending_nodes(nodes, last);
    }
    void chain_pending_nodes(node* first, node* last) {
        TICK();
        last->next = to_be_deleted;
        //Loop to guarantee that last->next is correct
        while (!to_be_deleted.compare_exchange_weak(last->next, first)) {
            WARN("to_be_deleted loop");
        }
    }
    void clain_pending_node(node* n) {
        TICK();
        chain_pending_nodes(n, n);
    }
    void try_reclaim(node* old_head) {
        TICK();
        if (threads_in_pop == 1) {
            node* nodes_to_delete = to_be_deleted.exchange(nullptr);
            if (!--threads_in_pop) {
                delete_nodes(nodes_to_delete);
            } else if (nodes_to_delete) {
                chain_pending_nodes(nodes_to_delete);
            }
            delete old_head;
        }
    }

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
        ++threads_in_pop;//Increase counter before doing anything else
        node* old_head = head.load();
        while (old_head && !head.compare_exchange_weak(old_head, old_head->next)) {
            WARN("pop loop");
        }
        std::shared_ptr<T> res(std::make_shared<T>());
        if (old_head) {
            res.swap(old_head->data);//Reclaim deleted nodes if you can
        }
        try_reclaim(old_head);//Extract data from node rather than copying pointer
        return res;
    }
};
void lock_free_reclaim_stack_test();

//7.2.3 Detecting nodes that can`t be reclaimed using hazard pointers
//Listing 7.6 An implementation of pop() using hazard pointers
//Listing 7.7 A simple implementation of get_hazard_pointer_for_current_thread()
//Listing 7.8 A simple implementation of the reclaim functions

//7.2.4 Detecting nodes in use with reference counting
//Listing 7.9 A lock-free stack using a lock-free std::shared_ptr<> implementation
template<typename T>
class lock_free_shared_stack {
private:
    struct node {
        std::shared_ptr<T> data;
        std::shared_ptr<node> next;
        explicit node(T const& data_) : data(std::make_shared<T>(data_)), next(nullptr) {}
    };
    std::shared_ptr<node> head;

public:
    void push(T const& data) {
        TICK();
        std::shared_ptr<node> const new_node = std::make_shared<node>(data);
        new_node->next = std::atomic_load(&head);
        while (!std::atomic_compare_exchange_weak(&head, &new_node->next, new_node)) {
            WARN("push loop");
        }
    }
    std::shared_ptr<T> pop() {
        TICK();
        std::shared_ptr<node> old_head = std::atomic_load(&head);
        while (old_head && !std::atomic_compare_exchange_weak(&head, &old_head, old_head->next)) {
            WARN("pop loop");
        }
        return old_head ? old_head->data : std::make_shared<T>();
    }
};
void lock_free_shared_stack_test();

//Listing 7.10 Pushing a node on a lock-free stack using split reference counts
//Listing 7.11 Popping a node form a lock-free stack using split reference counts
template<typename T>
class lock_free_split_ref_cnt_stack {
private:
    struct node;
    struct counted_node_ptr {
        int external_count;
        node* ptr;
    };
    struct node {
        std::shared_ptr<T> data;
        std::atomic<int> internal_count;
        counted_node_ptr* next;
        explicit node(T const& data_) : data(std::make_shared<T>(data_)), internal_count(0), next(nullptr) {}
    };
    std::atomic<counted_node_ptr*> head;
    void increase_head_count(counted_node_ptr* old_counter) {
        TICK();
        counted_node_ptr* new_counter;
        do {
            WARN("do loop");
            if (nullptr == old_counter) {
                return;
            }
            new_counter = old_counter;
            ++(new_counter->external_count);
        } while (!head.compare_exchange_strong(old_counter, new_counter));
        old_counter->external_count = new_counter->external_count;
    }

public:
    ~lock_free_split_ref_cnt_stack() {
        TICK();
        while (pop()) {
            WARN("dtr loop");
        }
    }
    void push(T const& data) {
        TICK();
        counted_node_ptr* new_node = new counted_node_ptr();
        new_node->ptr = new node(data);
        new_node->external_count = 1;
        new_node->ptr->next = head.load();
        while (!head.compare_exchange_weak(new_node->ptr->next, new_node)) {
            WARN("push loop");
        }
    }
#if 0
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr* old_head = head.load();
        while (old_head && !head.compare_exchange_weak(old_head, old_head->ptr->next)) {
            WARN("pop loop");
        }
        return old_head ? old_head->ptr->data : nullptr;
    }
#else
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr* old_head = head.load();
        for (;;) {
            increase_head_count(old_head);
            node* const ptr = old_head->ptr;
            if (!ptr) {
                return std::shared_ptr<T>();
            }
            if (!head.compare_exchange_strong(old_head, ptr->next)) {
                std::shared_ptr<T> res;
                res.swap(ptr->data);

                int const count_increase = old_head->external_count - 2;
                if (ptr->internal_count.fetch_add(count_increase) == -count_increase) {
                    delete ptr;
                }
                return res;
            } else if (ptr->internal_count.fetch_sub(1) == 1) {
                delete ptr;
            }
        }
    }
#endif
};
void lock_free_split_ref_cnt_stack_test();

















}//namespace lock_free_conc_data

#endif  //LOCK_FREE_CONCURRENT_DATA_STRUCTURES_H

