///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter6: Designing lock-based concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/11
#pragma once
#ifndef LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H
#define LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H

namespace lock_based_conc_data {

//6.1 What does it mean to design for concurrency?
//6.11 Guidelines for designing data structures for concurrency

//6.2 Lock-based concurrent data structures
//6.2.1 A thread-safe stack using locks
//Listing 6.1 A class definition for a thread-safe stack
struct empty_stack : std::exception {
    const char* what() const throw() {
        return "empty_stack";
    }
};
template<typename T>
class thread_safe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    thread_safe_stack() {}
    thread_safe_stack(const thread_safe_stack &other) : m(std::mutex()) {
        TICK();
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    thread_safe_stack& operator=(const thread_safe_stack &) = delete;
    void push(T new_value) {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop() {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) {
            throw empty_stack();
        }
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
        data.pop();
        return res;
    }
    void pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) {
            throw empty_stack();
        }
        value = std::move(data.top());
        data.pop();
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
void lock_thread_safe_stack_write();
void lock_thread_safe_stack_read();
void lock_thread_safe_stack_test();

//6.2.2 A thread-safe queue using locks and condition variables
//Listing 6.2 The full class definition for a thread-safe queue using condition variables
template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() {}
    void push(T new_value) {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }
    void wait_and_pop(T &value) {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop() {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool try_pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return false;
        }
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop() {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        return data_queue.empty();
    }
};
void threadsafe_queue_write();
void threadsafe_queue_read();
void treadsafe_queue_test();

//Listing 6.3 A thread-safe queue holding std::shared_ptr<> instances
template<typename T>
class threadsafe_queue_shared_ptr {
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue_shared_ptr() {}
    void wait_and_pop(T &value) {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        value = std::move(*data_queue.front());
        data_queue.pop();
    }
    bool try_pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return false;
        }
        value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> wait_and_pop() {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res = std::move(data_queue.front());
        data_queue.pop();
        return res;
    }
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res = std::move(data_queue.front());
        data_queue.pop();
        return res;
    }
    void push(T const new_value) {
        TICK();
        std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lock(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        return data_queue.empty();
    }
};
void threadsafe_queue_shared_ptr_write();
void threadsafe_queue_shared_ptr_read();
void threadsafe_queue_shared_ptr_test();

//6.2.3 A thread-safe queue using fine-grained locks and condition variables
//Listing 6.4 A simple single-threaded queue implementation
template<typename T>
class queue {
private:
    struct node {
        T data;
        std::unique_ptr<node> next;
        explicit node(T data_) : data(std::move(data_)) {}
    };
    std::unique_ptr<node> head;
    node *tail;

public:
    queue() : tail(nullptr) {}
    queue(queue const &other) = delete;
    queue& operator=(queue const &other) = delete;
    std::shared_ptr<T> try_pop() {
        TICK();
        if (!head) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(head->data)));
        std::unique_ptr<T> const old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }
    void push(T new_value) {
        std::unique_ptr<node> p(new node(std::move(new_value)));
        node* const new_tail = p->get();
        if (tail) {
            tail->next = std::move(p);
        } else {
            head = std::move(p);
        }
        tail = new_tail;
    }
};

//Listing 6.5 A simple queue with a dummy node
template<typename T>
class dummy_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::unique_ptr<node> head;
    node *tail;

public:
    dummy_queue() : head(new node), tail(head.get()) {}
    dummy_queue(dummy_queue const &other) = delete;
    dummy_queue& operator=(dummy_queue const &other) = delete;
    std::shared_ptr<T> try_pop() {
        TICK();
        if (head->get() == tail) {
            return std::make_shared<T>();
        }
        std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }
    void push(T new_value) {
        TICK();
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        tail->data = new_data;
        node const *new_tail = p->get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};

}//namespace lock_based_conc_data

#endif  //LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H

