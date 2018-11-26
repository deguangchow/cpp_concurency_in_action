///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter3: Sharing data between threads.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/26
#pragma once
#ifndef THREAD_SHARING_DATA_H
#define THREAD_SHARING_DATA_H

namespace thread_sharing_data {

//3.1 Problems with sharing data between threads
//3.1.1 Race conditions
//3.1.2 Avoiding problematic race conditions

//3.2 Protecting shared data with mutexes
//3.2.1 Using mutexes in C++
//Listing 3.1 Protecting a list with a mutex
void add_to_list(int new_value);
bool list_contains(int value_to_find);

//3.2.2 Structing code for protecting shared data
//Listing 3.2 Accidentally passing out a reference to protected data
class some_data {
    int a;
    std::string b;
public:
    void do_something() {}
};
class data_wrapper {
    some_data data;
    std::mutex m;
public:
    template<typename Function>
    void process_data(Function func) {
        TICK();
        std::lock_guard<std::mutex> l(m);
        func(data); //Pass "protected" data to user-supplied function!!!
    }
};
void foo();

//3.2.3 Spotting race conditions inherent in interfaces
//Listing 3.3 The interface to the std::stack container adapter
template<typename T, typename Container = std::deque<T>>
class stack {
    Container container;

public:
    explicit stack(const Container&) {}
    explicit stack(Container && = Container()) {}
    template<class Alloc> explicit stack(const Alloc&) {}
    template<class Alloc> stack(const Container&, const Alloc&) {}
    template<class Alloc> stack(Container&&, const Alloc&) {}
    template<class Alloc> stack(stack&&, const Alloc) {}

    bool empty() const {
        TICK();
        return container.empty();
    }
    size_t size() const {
        TICK();
        return container.size();
    }
    T& top() {
        TICK();
        INFO("val=%d\r\n", container.back());
        return container.back();
    }
    T const& top() const {
        TICK();
        INFO("val=%d\r\n", container.back());
        return container.back();
    }
    void push(T const &val) {
        TICK();
        INFO("val=%d\r\n", val);
        container.push_back(val);
    }
    void push(T &&val) {
        TICK();
        INFO("val=%d\r\n", val);
        container.push_back(val);
    }
    void pop() {
        TICK();
        container.pop_back();
    }
    void swap(stack &&s) {
        TICK();
        container.swap(s);
    }
};

void stack_test();

//Listing 3.4 An outline class defination for a thread-safe stack

//Listing 3.5 A fleshed-out class definition for a thread-safe stack
struct empty_stack : std::exception {
    const char* what() const throw() {
        TICK();
        return "empty_stack";
    }
};

template<typename T>
class thread_safe_stack {
    std::stack<T> data;
    mutable std::mutex m;

public:
    thread_safe_stack() {}
    thread_safe_stack(const thread_safe_stack &other) : data(), m() {
        TICK();
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;  //Copy performed in constructor body
    }
    thread_safe_stack& operator=(const thread_safe_stack &) = delete; //Assignment operator is deleted

    void push(T new_value) {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
    }
    std::shared_ptr<T> pop() {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) { //Check for empty before trying to pop value
            throw empty_stack();
        }
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));  //Allcate return value before modifying stack
        data.pop();
        return res;
    }
#if 0//If there are some override functions, the thread object init with it, something going wrong when compiling.
    void pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) {
            throw empty_stack();
        }
        value = data.top();
        data.pop();
    }
#endif
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

void thread_safe_stack_test();

}//namespace thread_sharing_data

#endif  //THREAD_SHARING_DATA_H
