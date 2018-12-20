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
        counted_node_ptr() : external_count(0), ptr(nullptr) {}
    };
    struct node {
        std::shared_ptr<T> data;
        std::atomic<int> internal_count;
        counted_node_ptr* next;
        explicit node(T const& data_) : data(std::make_shared<T>(data_)), internal_count(0), next(nullptr) {}
    };
#if 1
    std::atomic<counted_node_ptr*> head;
#else//undefining '_ENABLE_ATOMIC_ALIGNMENT_FIX', the 'std::<Type>' can`t be compiled correctly.
    std::atomic<counted_node_ptr> head;
    void increase_head_count(counted_node_ptr& old_counter) {
        TICK();
        counted_node_ptr new_counter;
        do {
            WARN("increase_head_count() loop");
            new_counter = old_counter;
            ++(new_counter.external_count);
        } while (!head.compare_exchange_strong(old_counter, new_counter));
        (old_counter).external_count = new_counter.external_count;
    }
#endif

public:
    ~lock_free_split_ref_cnt_stack() {
        TICK();
        while (pop()) {
            WARN("~lock_free_split_ref_cnt_stack() loop");
        }
    }
    void push(T const& data) {
        TICK();
        counted_node_ptr* new_node = new counted_node_ptr();
        new_node->ptr = new node(data);
        new_node->external_count = 1;
        new_node->ptr->next = head.load();
        while (!head.compare_exchange_weak(new_node->ptr->next, new_node)) {
            WARN("push() loop");
        }
    }
#if 1
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr* old_head = head.load();
        while (old_head && !head.compare_exchange_weak(old_head, old_head->ptr->next)) {
            WARN("pop() loop");
        }
        return old_head ? old_head->ptr->data : nullptr;
    }
#else
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr old_head = head.load();
        for (;;) {
            WARN("pop() loop");
            increase_head_count(old_head);
            node* const ptr = old_head.ptr;
            if (!ptr) {
                WARN("ptr(nullptr)");
                return std::shared_ptr<T>();
            }
            if (!head.compare_exchange_strong(old_head, ptr->next)) {
                std::shared_ptr<T> res;
                res.swap(ptr->data);

                int const count_increase = old_head.external_count - 2;
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

//7.2.5 Appling the memory model to the lock-free stack
//Listing 7.12 A lock-free stack with reference counting and relaxed atomic operations
template<typename T>
class lock_free_memory_split_ref_cnt_stack {
private:
    struct node;
    struct counted_node_ptr {
        int external_count;
        node* ptr;
    };
    struct node {
        std::shared_ptr<T> data;
        std::atomic<int> intrenal_count;
        counted_node_ptr* next;
        explicit node(T const& data_) : data(std::make_shared<T>(data_)), intrenal_count(0), next(nullptr) {}
    };
#if 1
    std::atomic<counted_node_ptr*> head;
#else
    std::atomic<counted_node_ptr> head;
    void increase_head_count(counted_node_ptr& old_counter) {
        TICK();
        counted_node_ptr new_counter;
        do {
            WARN("increase_head_count() loop");
            new_counter = old_counter;
            ++new_counter.external_count;
        } while (!head.compare_exchange_strong(old_counter, new_counter,
            std::memory_order_acquire, memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }
#endif


public:
    ~lock_free_memory_split_ref_cnt_stack() {
        TICK();
        while (pop()) {
            WARN("~lock_free_memory_split_ref_cnt_stack() loop");
        }
    }
#if 1
    void push(T const& data) {
        TICK();
        counted_node_ptr* new_node = new counted_node_ptr();
        new_node->ptr = new node(data);
        new_node->external_count = 1;
        new_node->ptr->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node->ptr->next, new_node,
            std::memory_order_release, std::memory_order_relaxed)) {
            WARN("push() loop");
        }
    }
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr* old_head = head.load();
        while (old_head && !head.compare_exchange_weak(old_head, old_head->ptr->next)) {
            WARN("pop() loop");
        }
        return old_head ? old_head->ptr->data : nullptr;
    }
#else
    void push(T const& data) {
        TICK();
        counted_node_ptr new_node;
        new_node.ptr = new node(data);
        new_node.external_count = 1;
        new_node.ptr->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(new_node.ptr->next, &new_node,
            std::memory_order_release, std::memory_order_relaxed)) {
            WARN("push() loop");
        }
    }
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr old_head = head.load(std::memory_order_acq_rel);
        for (;;) {
            WARN("pop() loop");
            increase_head_count(old_head);
            node* const ptr = old_head.ptr;
            if (!ptr) {
                WARN("ptr(nullptr)");
                return std::shared_ptr<T>();
            }
            if (head.compare_exchange_strong(old_head, ptr->next, std::memory_order_relaxed)) {
                std::shared_ptr<T> res;
                res.swap(ptr->data);
                int const count_increase = old_head.external_count - 2;
                if (ptr->intrenal_count.fetch_add(count_increase, std::memory_order_release) == -count_increase) {
                    delete ptr;
                }
                return res;
            } else if (ptr->intrenal_count.fetch_add(-1, std::memory_order_relaxed) == 1) {
                ptr->intrenal_count.load(std::memory_order_acquire);
                delete ptr;
            }
        }
    }
#endif
};
void lock_free_memory_split_ref_cnt_stack_test();

//7.2.6 Writing a thread-safe queue without lock
//Listing 7.13 A single-producer, single-consumer lock-free queue
template<typename T>
class lock_free_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        node* next;
        node() : data(nullptr), next(nullptr) {}
    };
    std::atomic<node*> head;
    std::atomic<node*> tail;
    node* pop_head() {
        TICK();
        node* const old_head = head.load();
        if (old_head == tail.load()) {
            return nullptr;
        }
        head.store(old_head->next);
        return old_head;
    }

public:
    lock_free_queue() : head(new node), tail(head.load()) {}
    lock_free_queue(const lock_free_queue& other) = delete;
    lock_free_queue& operator=(const lock_free_queue& other) = delete;
    ~lock_free_queue() {
        TICK();
        while (node* const old_haed = head.load()) {
            head.store(old_haed->next);
            WARN("~lock_free_queue() loop");
            delete old_haed;
        }
    }
    std::shared_ptr<T> pop() {
        TICK();
        node* old_head = pop_head();
        if (!old_head) {
            return std::make_shared<T>();
        }
        std::shared_ptr<T> const res(old_head->data);
        delete old_head;
        old_head = nullptr;
        return res;
    }
    void push(T new_value) {
        TICK();
        std::shared_ptr<T> new_data(std::make_shared<T>(new_value));
        node* p = new node();
        node* const old_tail = tail.load();
        old_tail->data.swap(new_data);
        old_tail->next = p;
        tail.store(p);
    }
};
void lock_free_queue_test();

//Listing 7.14 A (broken) first attempt at revising push()

//Listing 7.15 Implementating push() for a lock-free queue with a reference-counted tail
template<typename T>
class lock_free_ref_cnt_queue {
private:
    struct node;
    struct counted_node_ptr {
        int external_count;
        node* ptr;
    };
    std::atomic<counted_node_ptr> head;
    std::atomic<counted_node_ptr> tail;
    struct node_counter {
        unsigned internal_count : 30;
        unsigned external_counters : 2;
    };
    struct node {
        std::atomic<T*> data;
        std::atomic<node_counter> count;
        counted_node_ptr next;
        node() {
            TICK();
            node_counter new_count;
            new_count.internal_count = 0;
            new_count.external_counters = 2;
            count.store(new_count);
        }
        void release_ref() {
            TICK();
        }
    };
    static void increase_external_count(std::atomic<counted_node_ptr>& counter, counted_node_ptr& old_counter) {
        TICK();
        counted_node_ptr new_counter;
        do {
        } while (!counter.compare_exchange_strong(old_counter, new_counter,
            std::memory_order_acquire, std::memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }

public:
    void push(T new_value) {
        TICK();
        std::unique_ptr<T> new_data(new T(new_value));
        counted_node_ptr new_next;
        new_next.ptr = new node;
        new_next.external_count = 1;
        counted_node_ptr old_tail = tail.load();
        for (;;) {
            WARN("push loop");
            increase_external_count(head, old_tail);

            T* old_data = nullptr;
            if (old_tail.ptr->data.compare_exchange_strong(old_data, new_data.get())) {
                old_tail.ptr->next = new_next;
                old_tail = tail.exchange(new_next);
                free_external_counter(old_tail);
                new_data.release();
                break;
            }
            old_tail.ptr->release_ref();
        }
    }
    std::shared_ptr<T> pop() {
        TICK();
        counted_node_ptr old_head = head.load(std::memory_order_relaxed);
        for (;;) {
            WARN("pop loop");
            increase_external_count(head, old_head);
            node* const ptr = old_head.ptr;
            if (ptr == tail.load().ptr) {
                ptr->release_ref();


            }
        }
    }
};

}//namespace lock_free_conc_data

#endif  //LOCK_FREE_CONCURRENT_DATA_STRUCTURES_H

