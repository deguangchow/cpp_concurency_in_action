///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter2: thread manage.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#include "stdafx.h"
#include "thread_manage.h"

namespace thread_manage {

void do_some_work() {
    TICK();
}

void do_something() {
    TICK();
}

void do_something_else() {
    TICK();
}

void launching_thread_test() {
    TICK();

    std::thread my_thread1(do_some_work);
    my_thread1.join();

    background_task f;
    std::thread my_thread2(f);
    my_thread2.join();

    std::thread my_thread3{ background_task() };
    my_thread3.join();

    std::thread my_thread4([] {
        do_something();
        do_something_else();
    });
    my_thread4.join();
}

void do_something(int &i) {
    TICK();
}

void oops() {
    TICK();

    int some_local_state = 0;
    FUNC my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach(); //Do not wait for the my_thread to finish
}
void do_something_in_current_thread() {
    TICK();
    throw std::exception();//throw a exception.
}

void f() {
    TICK();

    int some_local_state = 0;
    FUNC my_func(some_local_state);
    std::thread t(my_func);
    try {
        do_something_in_current_thread();
    } catch (...) {//catch a exception.
        t.join();
        throw;
    }
    t.join();
}

//The my_thread might still be running

}//namespace thread_manage


