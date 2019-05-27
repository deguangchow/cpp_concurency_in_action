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
//Listing 7.1 Implementation of a spin-lock mutex using atmoic_flag
class spinlock_mutex {
    atomic_flag flag_a = ATOMIC_FLAG_INIT;
public:
    spinlock_mutex() {}
    void lock() {
        TICK();
        //atomically set the flag to true and return previous value
        while (flag_a.test_and_set(memory_order::memory_order_acquire)) {
            WARN("lock loop...");
            yield();
        }
    }
    void unlock() {
        TICK();
        flag_a.clear(memory_order::memory_order_release);
    }
};
void test_spinlock_mutex();

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
    atomic<node*>   m_pHead_a = nullptr;

public:
    void push(T const& data) {
        TICK();
        node* const pNewNode = new node(data);
        pNewNode->next = m_pHead_a.load();

        //compare and change value store in head with arg1, arg2
        //bool ret = addr.compare_exchange_weak(old_value, new_value)
        //if ret == true, *addr = new_value;
        //if ret == false, old_vale = *addr;
        while (!m_pHead_a.compare_exchange_weak(pNewNode->next, pNewNode)) {
            WARN("push() loop...");
            yield();
        }
        DEBUG("push(%d)", data);
    }
    void pop(T& result) {
        TICK();
        node* pOldHead = m_pHead_a.load();
        while (pOldHead && !m_pHead_a.compare_exchange_weak(pOldHead, pOldHead->next)) {
            WARN("pop() loop...");
            yield();
        }
        result = pOldHead ? pOldHead->data : 0;
        INFO("pop()=%d", result);
    }
};
void test_lock_free_stack();

//Listing 7.3 A lock-free stack that leaks nodes
template<typename T>
class lock_free_shared_ptr_stack {
private:
    struct node {
        shared_ptr<T> data;//Data is now held by pointer
        node* next = nullptr;
        //Create shared_ptr for newly allocated T
        explicit node(T const& data_) : data(make_shared<T>(data_)) {}
    };
    atomic<node*> m_pHead_a = nullptr;

public:
    void push(T const& data) {
        TICK();
        node* const pNewNode = new node(data);
        pNewNode->next = m_pHead_a.load();
        while (!m_pHead_a.compare_exchange_weak(pNewNode->next, pNewNode)) {
            WARN("push() loop...");
            yield();
        }
    }
    shared_ptr<T> pop() {
        TICK();
        node* pOldHead = m_pHead_a.load();
        //Check old_head is not a null pointer before you dereference it
        while (pOldHead && !m_pHead_a.compare_exchange_weak(pOldHead, pOldHead->next)) {
            WARN("pop() loop...");
        }
        return pOldHead ? pOldHead->data : make_shared<T>(0);
    }
};
void test_lock_free_shared_ptr_stack();

//7.2.2 Stopping those pesky leaks: managing memory in lock-free data structures
//Listing 7.4 Reclaiming nodes when no threads are in pop()
template<typename T>
class lock_free_reclaim_stack {
private:
    struct node {
        shared_ptr<T> data;
        node* next = nullptr;
        explicit node(T const& data_) : data(make_shared<T>(data_)) {}
    };
    atomic<node*>       m_pHead_a = nullptr;
    atomic<node*>       m_pToBeDeleted_a = nullptr;
    atomic<unsigned>    m_uThreadsInPop_a = 0;//Atomic variable
    static void delete_nodes(node* nodes) {
        TICK();
        while (nodes) {
            node* next = nodes->next;

            WARN("delete node(%d)", *nodes->data.get());
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
        last->next = m_pToBeDeleted_a;
        //Loop to guarantee that last->next is correct
        while (!m_pToBeDeleted_a.compare_exchange_weak(last->next, first)) {
            WARN("chain_pending_nodes() loop...");
            yield();
        }
    }
    void clain_pending_node(node* n) {
        TICK();
        chain_pending_nodes(n, n);
    }
    void try_reclaim(node* old_head) {
        TICK();
        if (m_uThreadsInPop_a == 1) {
            node* pToBeDeleted = m_pToBeDeleted_a.exchange(nullptr);
            if (!--m_uThreadsInPop_a) {
                delete_nodes(pToBeDeleted);
            } else if (pToBeDeleted) {
                chain_pending_nodes(pToBeDeleted);
            }

            WARN("try reclaim(%d)", *old_head->data.get());
            delete old_head;
        }
    }

public:
    void push(T const& data) {
        TICK();
        node* const pNewNode = new node(data);
        pNewNode->next = m_pHead_a.load();
        while (!m_pHead_a.compare_exchange_weak(pNewNode->next, pNewNode)) {
            WARN("push() loop...");
            yield();
        }
    }
    shared_ptr<T> pop() {
        TICK();
        ++m_uThreadsInPop_a;//Increase counter before doing anything else
        node* pOldNode = m_pHead_a.load();
        while (pOldNode && !m_pHead_a.compare_exchange_weak(pOldNode, pOldNode->next)) {
            WARN("pop() loop...");
            yield();
        }
        shared_ptr<T> result(make_shared<T>());
        if (pOldNode) {
            result.swap(pOldNode->data);//Reclaim deleted nodes if you can
        }
        try_reclaim(pOldNode);//Extract data from node rather than copying pointer
        return result;
    }
};
void test_lock_free_reclaim_stack();

//7.2.3 Detecting nodes that can`t be reclaimed using hazard pointers
//Listing 7.6 An implementation of pop() using hazard pointers
//Listing 7.7 A simple implementation of get_hazard_pointer_for_current_thread()
//Listing 7.8 A simple implementation of the reclaim functions

//7.2.4 Detecting nodes in use with reference counting
//Listing 7.9 A lock-free stack using a lock-free shared_ptr<> implementation
template<typename T>
class lock_free_shared_stack {
private:
    struct node {
        shared_ptr<T> data;
        shared_ptr<node> next;
        explicit node(T const& data_) : data(make_shared<T>(data_)), next(nullptr) {}
    };
    shared_ptr<node>    m_ptrHead = nullptr;

public:
    void push(T const& data) {
        TICK();
        shared_ptr<node> const ptrNewNode = make_shared<node>(data);
        ptrNewNode->next = atomic_load(&m_ptrHead);
        while (!atomic_compare_exchange_weak(&m_ptrHead, &ptrNewNode->next, ptrNewNode)) {
            WARN("push() loop...");
            yield();
        }
    }
    shared_ptr<T> pop() {
        TICK();
        shared_ptr<node> ptrOldHead = atomic_load(&m_ptrHead);
        while (ptrOldHead && !atomic_compare_exchange_weak(&m_ptrHead, &ptrOldHead, ptrOldHead->next)) {
            WARN("pop() loop...");
            yield();
        }
        return ptrOldHead ? ptrOldHead->data : make_shared<T>();
    }
};
void test_lock_free_shared_stack();

//Listing 7.10 Pushing a node on a lock-free stack using split reference counts
//Listing 7.11 Popping a node form a lock-free stack using split reference counts
template<typename T>
class lock_free_split_ref_cnt_stack {
private:
    struct node;
    struct counted_node_ptr {
        int                 external_count;
        node*               ptr;
        counted_node_ptr() : external_count(0), ptr(nullptr) {}
    };
    struct node {
        shared_ptr<T>       data;
        atomic<int>         internal_count;
        counted_node_ptr*   next;
        explicit node(T const& data_) : data(make_shared<T>(data_)), internal_count(0), next(nullptr) {}
    };
#if 1
    atomic<counted_node_ptr*>   m_pHead_a = nullptr;
#else//undefining '_ENABLE_ATOMIC_ALIGNMENT_FIX', the '<Type>' can`t be compiled correctly.
    atomic<counted_node_ptr>    m_pHead_a;
    void increase_head_count(counted_node_ptr& old_counter) {
        TICK();
        counted_node_ptr new_counter;
        do {
            WARN("increase_head_count() loop");
            new_counter = old_counter;
            ++(new_counter.external_count);
        } while (!m_pHead_a.compare_exchange_strong(old_counter, new_counter));
        (old_counter).external_count = new_counter.external_count;
    }
#endif

public:
    ~lock_free_split_ref_cnt_stack() {
        TICK();
        while (auto const ptr = pop()) {
            WARN("~lock_free_split_ref_cnt_stack() loop...");
            INFO("pop()=%d", *ptr.get());
            yield();
        }
    }
    void push(T const& data) {
        TICK();
        counted_node_ptr* new_node = new counted_node_ptr();
        new_node->ptr = new node(data);
        new_node->external_count = 1;
        new_node->ptr->next = m_pHead_a.load();
        while (!m_pHead_a.compare_exchange_weak(new_node->ptr->next, new_node)) {
            WARN("push() loop...");
            yield();
        }
    }
#if 1
    shared_ptr<T> pop() {
        TICK();
        counted_node_ptr* pOldHead = m_pHead_a.load();
        while (pOldHead && !m_pHead_a.compare_exchange_weak(pOldHead, pOldHead->ptr->next)) {
            WARN("pop() loop...");
            yield();
        }
        return pOldHead ? pOldHead->ptr->data : nullptr;
    }
#else
    shared_ptr<T> pop() {
        TICK();
        counted_node_ptr pOldHead = m_pHead_a.load();
        for (;;) {
            WARN("pop() loop");
            increase_head_count(pOldHead);
            node* const ptr = pOldHead.ptr;
            if (!ptr) {
                WARN("ptr(nullptr)");
                return shared_ptr<T>();
            }
            if (!m_pHead_a.compare_exchange_strong(pOldHead, ptr->next)) {
                shared_ptr<T> res;
                res.swap(ptr->data);

                int const count_increase = pOldHead.external_count - 2;
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
void test_lock_free_split_ref_cnt_stack();

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
        shared_ptr<T> data;
        atomic<int> intrenal_count;
        counted_node_ptr* next;
        explicit node(T const& data_) : data(make_shared<T>(data_)), intrenal_count(0), next(nullptr) {}
    };
#if 1
    atomic<counted_node_ptr*>   m_pHead_a = nullptr;
#else
    atomic<counted_node_ptr>    m_pHead_a;
    void increase_head_count(counted_node_ptr& old_counter) {
        TICK();
        counted_node_ptr new_counter;
        do {
            WARN("increase_head_count() loop");
            new_counter = old_counter;
            ++new_counter.external_count;
        } while (!m_pHead_a.compare_exchange_strong(old_counter, new_counter,
            memory_order_acquire, memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }
#endif


public:
    ~lock_free_memory_split_ref_cnt_stack() {
        TICK();
        while (pop()) {
            WARN("~lock_free_memory_split_ref_cnt_stack() loop");
            yield();
        }
    }
#if 1
    void push(T const& data) {
        TICK();
        counted_node_ptr* pNewNode = new counted_node_ptr();
        pNewNode->ptr = new node(data);
        pNewNode->external_count = 1;
        pNewNode->ptr->next = m_pHead_a.load(memory_order::memory_order_relaxed);
        while (!m_pHead_a.compare_exchange_weak(pNewNode->ptr->next, pNewNode,
            memory_order::memory_order_release, memory_order::memory_order_relaxed)) {
            WARN("push() loop...");
            yield();
        }
    }
    shared_ptr<T> pop() {
        TICK();
        counted_node_ptr* pOldHead = m_pHead_a.load();
        while (pOldHead && !m_pHead_a.compare_exchange_weak(pOldHead, pOldHead->ptr->next)) {
            WARN("pop() loop...");
            yield();
        }
        return pOldHead ? pOldHead->ptr->data : nullptr;
    }
#else
    void push(T const& data) {
        TICK();
        counted_node_ptr pNewNode;
        pNewNode.ptr = new node(data);
        pNewNode.external_count = 1;
        pNewNode.ptr->next = m_pHead_a.load(memory_order_relaxed);
        while (!m_pHead_a.compare_exchange_weak(pNewNode.ptr->next, &pNewNode,
            memory_order_release, memory_order_relaxed)) {
            WARN("push() loop");
        }
    }
    shared_ptr<T> pop() {
        TICK();
        counted_node_ptr pOldHead = m_pHead_a.load(memory_order_acq_rel);
        for (;;) {
            WARN("pop() loop");
            increase_head_count(pOldHead);
            node* const ptr = pOldHead.ptr;
            if (!ptr) {
                WARN("ptr(nullptr)");
                return shared_ptr<T>();
            }
            if (m_pHead_a.compare_exchange_strong(pOldHead, ptr->next, memory_order_relaxed)) {
                shared_ptr<T> res;
                res.swap(ptr->data);
                int const count_increase = pOldHead.external_count - 2;
                if (ptr->intrenal_count.fetch_add(count_increase, memory_order_release) == -count_increase) {
                    delete ptr;
                }
                return res;
            } else if (ptr->intrenal_count.fetch_add(-1, memory_order_relaxed) == 1) {
                ptr->intrenal_count.load(memory_order_acquire);
                delete ptr;
            }
        }
    }
#endif
};
void test_lock_free_memory_split_ref_cnt_stack();

//7.2.6 Writing a thread-safe queue without lock
//Listing 7.13 A single-producer, single-consumer lock-free queue
template<typename T>
class lock_free_queue {
private:
    struct node {
        shared_ptr<T>   data;
        node*           next;
        node() : data(nullptr), next(nullptr) {}
    };
    atomic<node*>       m_pHead_a = nullptr;
    atomic<node*>       m_pTail_a = nullptr;
    node* pop_head() {
        TICK();
        node* const pOldHead = m_pHead_a.load();
        if (pOldHead == m_pTail_a.load()) {
            return nullptr;
        }
        m_pHead_a.store(pOldHead->next);
        return pOldHead;
    }

public:
    lock_free_queue() : m_pHead_a(new node), m_pTail_a(m_pHead_a.load()) {}
    lock_free_queue(const lock_free_queue& other) = delete;
    lock_free_queue& operator=(const lock_free_queue& other) = delete;
    ~lock_free_queue() {
        TICK();
        while (node* pOldHead = m_pHead_a.load()) {
            m_pHead_a.store(pOldHead->next);
            WARN("~lock_free_queue() loop...");
            delete pOldHead;
            pOldHead = nullptr;
            yield();
        }
    }
    shared_ptr<T> pop() {
        TICK();
        node* pOldHead = pop_head();
        if (!pOldHead) {
            return make_shared<T>();
        }
        shared_ptr<T> const ptrResult(pOldHead->data);
        delete pOldHead;
        pOldHead = nullptr;
        return ptrResult;
    }
    void push(T new_value) {
        TICK();
        shared_ptr<T> ptrNewData(make_shared<T>(new_value));
        node* pNewNode = new node();
        node* const pOldTail = m_pTail_a.load();
        pOldTail->data.swap(ptrNewData);
        pOldTail->next = pNewNode;
        m_pTail_a.store(pNewNode);
    }
};
void test_lock_free_queue();

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
    atomic<counted_node_ptr> head;
    atomic<counted_node_ptr> tail;
    struct node_counter {
        unsigned internal_count : 30;
        unsigned external_counters : 2;
    };
    struct node {
        atomic<T*> data;
        atomic<node_counter> count;
        counted_node_ptr next;
        node() {
            TICK();
            node_counter new_count;
            new_count.internal_count = 0;
            new_count.external_counters = 2;
            count.store(new_count);
            next = nullptr;
        }
        void release_ref() {
            TICK();
        }
    };
    static void increase_external_count(atomic<counted_node_ptr>& counter, counted_node_ptr& old_counter) {
        TICK();
        counted_node_ptr new_counter;
        do {
        } while (!counter.compare_exchange_strong(old_counter, new_counter,
            memory_order_acquire, memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }

public:
    void push(T new_value) {
        TICK();
        unique_ptr<T> new_data(new T(new_value));
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
    shared_ptr<T> pop() {
        TICK();
        counted_node_ptr old_head = head.load(memory_order_relaxed);
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

