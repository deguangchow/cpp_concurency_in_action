///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter3: Sharing data between threads.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/26
#include "stdafx.h"
#include "thread_sharing_data.h"

namespace thread_sharing_data {

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value) {
    TICK();
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find) {
    TICK();
    std::lock_guard<std::mutex> gurad(some_mutex);
    return std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}

some_data *unprotected;
void malicious_function(some_data &protected_data) {
    TICK();
    unprotected = &protected_data;
}
data_wrapper x;
void foo() {
    TICK();
    x.process_data(malicious_function); //Pass in a malicious function
    unprotected->do_something();        //Unprotected access to protected data
}

void do_something(int val) {
    TICK();
}

void stack_test() {
    TICK();
    stack<int> s;
    s.push(1);
    s.push(2);
    s.push(3);
    while (!s.empty()) {
        int const value = s.top();
        s.pop();
        do_something(value);
    }
}

thread_safe_stack<int> s;
void thread_safe_stack_test() {
    TICK();

    unsigned const push_thread_num = THREAD_NUM_128 - 1;
    unsigned const pop_thread_num = THREAD_NUM_128;

    std::vector<std::thread> push_threads(push_thread_num);
    std::vector<std::thread> pop_threads(pop_thread_num);

    for (unsigned i = 0; i < push_thread_num; ++i) {
        push_threads[i] = std::thread(&thread_safe_stack<int>::push, &s, i);
    }
    for (unsigned i = 0; i < pop_thread_num; ++i) {
        pop_threads[i] = std::thread(&thread_safe_stack<int>::pop, &s);
    }

    for (unsigned i = 0; i < push_thread_num; ++i) {
        push_threads[i].join();
    }
    for (unsigned i = 0; i < pop_thread_num; ++i) {
        pop_threads[i].join();  //When the num of threads to pop data from the stack is more than to push,
                                //there must be an "empty_stack" assert!
    }
}

void std_lock_test() {
    TICK();
    X<int> x1(some_big_object<int>(1));
    X<int> x2(some_big_object<int>(2));
    swap(x1, x2);
}

//3.2.5 Further guidelines for avoiding deadlock
//Listing 3.7 Using a lock hierarchy to prevent deadlock
thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);
hierarchical_mutex high_level_mutex(TEN_THOUSAND);
hierarchical_mutex low_level_mutex(THOUSAND * 5);
hierarchical_mutex other_mutex(HUNDRED);

int do_low_level_stuff() {
    TICK();
    return 0;
}

int low_level_func() {
    TICK();
    std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
    return do_low_level_stuff();
}

void do_high_level_stuff(int some_param) {
    TICK();
}

void high_level_func() {
    TICK();
    std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
    return do_high_level_stuff(42);
}

void thread_a() {
    TICK();
    high_level_func();
}

void do_other_stuff() {
    TICK();
}

void other_stuff() {
    TICK();
    high_level_func();//it`s thus violating the hierarchy: high_level_func() tries to acquire the high_level_mutex
    do_other_stuff();
}

void thread_b() {
    TICK();
    std::lock_guard<hierarchical_mutex> lk(other_mutex);
    other_stuff();
}

void hierarchical_mutex_test() {
    TICK();
    std::thread t1(thread_a);
    std::thread t2(thread_b);
    t1.join();
    t2.join();
}



}//namespace thread_sharing_data


