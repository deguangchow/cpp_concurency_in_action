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

}//namespace thread_sharing_data


