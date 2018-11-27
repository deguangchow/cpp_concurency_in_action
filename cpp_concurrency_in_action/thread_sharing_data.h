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

//3.2.4 Deadlock: the problem and a solution
//Listing 3.6 Using std::lock() and std::lock_guard in a swap operation
template<typename T>
class some_big_object {
    T data;

public:
    explicit some_big_object(T val) : data(val) {}
    void set_data(T const &val) {
        data = val;
    }
    T get_data() {
        return data;
    }
};

template<typename T>
void swap(some_big_object<T> &lhs, some_big_object<T> &rhs) {
    TICK();
    T tmp = lhs.get_data();
    lhs.set_data(rhs.get_data());
    rhs.set_data(tmp);
}

template<typename T>
class X {
private:
    some_big_object<T> some_detail;
    std::mutex m;
public:
    explicit X(some_big_object<T> const &sd) :some_detail(sd) {}

    friend void swap(X<T> &lhs, X<T> &rhs) {
        TICK();
        if (&lhs == &rhs) {
            return;
        }
        std::lock(lhs.m, rhs.m);
        std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
        std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
        swap(lhs.some_detail, rhs.some_detail);
    }
};

void std_lock_test();

//3.2.5 Further guidelines for avoiding deadlock
//Listing 3.8 A simple hierarchical mutex
class hierarchical_mutex {
    std::mutex internal_mutex;
    unsigned long const hierachy_value;
    unsigned long previous_hierarchy_value;
    static thread_local unsigned long this_thread_hierarchy_value;

    void check_for_hierarchy_violation() {
        TICK();
        if (this_thread_hierarchy_value <= hierachy_value) {
            INFO("mutex hierarchy violated");
            throw std::logic_error("mutex hierarchy violated");
        }
    }

    void update_hierarchy_value() {
        TICK();
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierachy_value;
    }

public:
    explicit hierarchical_mutex(unsigned long value) : hierachy_value(value), previous_hierarchy_value(0) {}
    void lock() {
        TICK();
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }
    void unlock() {
        TICK();
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }
    bool try_lock() {
        TICK();
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock()) {
            return false;
        }
        update_hierarchy_value();
        return true;
    }
};

//Listing 3.7 Using a lock hierarchy to prevent deadlock
int do_low_level_stuff();
int low_level_func();
void do_high_level_stuff(int some_param);
void high_level_func();
void thread_a();
void do_other_stuff();
void other_stuff();
void thread_b();

void hierarchical_mutex_test();

//3.2.6 Flexible locking with std::unique_lock
//Listing 3.9 Using std::lock() and std::unique_lock in a swap oopration
template<typename T>
class X_EX {
private:
    some_big_object<T> some_detail;
    std::mutex m;
    
public:
    explicit X_EX(some_big_object<T> const &sd) : some_detail(sd) {}
    friend void swap(X_EX<T> &lhs, X_EX<T> &rhs) {
        TICK();
        if (&lhs == &rhs) {
            return;
        }

        std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
        std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock); //std::defer_lock leaves mutexes unlocked
        std::lock(lock_a, lock_b);  //Mutexes are locked here
        swap(lhs.some_detail, rhs.some_detail);
    }
};

void std_lock_ex_test();

//3.2.7 Transferring mutex ownership between scopes
void prepare_data();
std::unique_lock<std::mutex> get_lock();
void do_something();
void process_data();

//3.2.8 Locking at an appropriate granularity
class some_class {

};
some_class get_next_data_chunk();
typedef unsigned result_type;
result_type process(some_class data);
void write_result(some_class const& data, result_type &result);
void get_and_process_data();

//Listing 3.10 Locking one mutex at a time in a comparison operator
template<typename T>
class Y {
private:
    T some_detail;
    mutable std::mutex m;

    T get_detail() const {
        TICK();
        std::lock_guard<std::mutex> lock_a(m);
        return some_detail;
    }
public:
    Y(T sd) :some_detail(sd) {}
    friend bool operator==(Y<T> const &lhs, Y<T> const &rhs) {
        TICK();
        if (&lhs == &rhs) {
            return true;
        }
        T const &lhs_value = lhs.get_detail();
        T const &rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }
};

void compare_operator_test();

//3.3 Alternative facilities for protecting shared data
//3.3.1 Protecting shared data during initialization
class some_resource {
public:
    void do_something() {
        TICK();
    }
};
void RAII_test();


}//namespace thread_sharing_data

#endif  //THREAD_SHARING_DATA_H
