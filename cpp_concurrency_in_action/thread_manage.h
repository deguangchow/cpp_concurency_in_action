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
void f_oops_exception();

//Listing 2.3 Using RAII to wait for a thread to complete
class thread_gurad {
    std::thread &t;
public:
    explicit thread_gurad(std::thread &t_) : t(t_) { }
    ~thread_gurad() {
        if (t.joinable()) {
            t.join();
        }
    }
    thread_gurad(thread_gurad const&) = delete;
    thread_gurad& operator=(thread_gurad const&) = delete;
};
void f_thread_guard();


//2.1.4 Running threads in the background
void do_background_work();
void run_thread_background();

//Listing 2.4 Detaching a thread to handle other documents
enum DOC_OPREA_TYPE {
    open_new_document = 0
};
class user_command {
public:
    DOC_OPREA_TYPE type;
public:
    user_command() : type(open_new_document) {}
    user_command(DOC_OPREA_TYPE const &type_) :type(type_) {}
    ~user_command() {}
};
void open_document_and_display(std::string const &filename);
bool done_editing();
user_command const get_user_input();
std::string const get_filename_from_user(); //tips: don`t try to return a refrence, or you will get noting but an error:
                                            //(error) Reference to temporary returned.
void process_user_input(user_command const &cmd);
void edit_document(std::string const &filename);
void edit_document_test();

//2.2 Passing arguments to a thread function
void f_passing_argument_test(int i, std::string const& s);
void oops(int some_param);
void not_oops(int some_param);

typedef unsigned widget_id;
typedef unsigned widget_data;
void update_date_for_widget(widget_id w, widget_data &data);
void process_widget_data(widget_data const &data);
void display_status();
void oops_again(widget_id w);

class X {
public:
    void do_lengthy_work() { TICK(); }
};
void x_test();

class X1 {
public:
    void do_lengthy_work(int num) { TICK(); }
};
void x1_test();

struct big_object {
    unsigned data;
    void prepare_data(unsigned const &data_) {
        TICK();
        data = data_;
    }
};
void process_big_object(std::unique_ptr<big_object>);
void move_test();


}//namespace thread_manage

#endif  //THREAD_MANAGE_H
