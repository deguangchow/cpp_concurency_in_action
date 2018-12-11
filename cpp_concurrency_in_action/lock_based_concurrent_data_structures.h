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
    thread_safe_stack(const thread_safe_stack &other) {
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

}//namespace lock_based_conc_data

#endif  //LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H

