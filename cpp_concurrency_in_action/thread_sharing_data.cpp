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

}//namespace thread_sharing_data


