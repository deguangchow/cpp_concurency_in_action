///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter2: thread manage
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#pragma once
#ifndef THREAD_MANAGE_H
#define THREAD_MANAGE_H

namespace thread_manage {

//2.1.1 Launching a thread
void do_some_work();
void do_something();
void do_something_else();
class background_task {
public:
    void operator()() const {
        do_something();
        do_something_else();
    }
};
void launching_thread_test();


//Listing 2.1: A function that returns while a thread still has access to local variables.
void do_something(int &i);
typedef struct func {
    int &i;
    func(int &i_) :i(i_) {}
    void operator()() {
        for (unsigned j = 0; j < MILLION; ++j) {
            do_something(i);//Potential access to dangling reference.
        }
    }
}FUNC;
typedef std::shared_ptr<FUNC> ptrFUNC;
void oops();

//2.1.2 Waiting for a thread to complete

//2.1.3 Waiting in exceptional circumstances
//Listing 2.2 Waiting for a thread to finish
void do_something_in_current_thread();
void f();

}//namespace thread_manage

#endif  //THREAD_MANAGE_H
