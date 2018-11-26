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
}                       //The my_thread might still be running

void do_something_in_current_thread() {
    TICK();
    throw std::exception();//throw a exception.
}

void f_oops_exception() {
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

void f_thread_guard() {
    TICK();

    int some_local_state = 0;
    FUNC my_func(some_local_state);
    std::thread t(my_func);
    thread_gurad g(t);

    do_something_in_current_thread();
}

void do_background_work() {
    TICK();
}

void run_thread_background() {
    TICK();

    std::thread t(do_background_work);
    t.detach();
    assert(!t.joinable());
}

void open_document_and_display(std::string const &filename) {
    TICK();
}

bool done_editing() {
    TICK();
    unsigned n = 0;
    errno_t err = rand_s(&n);
    return 0 == n%2;
}

user_command const get_user_input() {
    TICK();
    return user_command();
}

std::string const get_filename_from_user() {
    TICK();
    return std::string();
}

void process_user_input(user_command const &cmd) {
    TICK();
}

void edit_document(std::string const &filename) {
    TICK();

    open_document_and_display(filename);
    while (!done_editing()) {
        user_command cmd = get_user_input();
        if (cmd.type == open_new_document) {
            std::string const &new_name = get_filename_from_user();
            std::thread t(edit_document, new_name);
            t.detach();
        } else {
            process_user_input(cmd);
        }
    }
}

void edit_document_test() {
    TICK();

    std::string const &filename = "a.doc";
    edit_document(filename);
}

void f_passing_argument_test(int i, std::string const& s) {
    TICK();
}

void oops(int some_param) {
    TICK();

    char buffer[BUFFER_1024];
    sprintf_s(buffer, BUFFER_1024, "%i", some_param);
    std::thread t(f_passing_argument_test, 3, buffer);
    t.detach();
}

void not_oops(int some_param) {
    TICK();

    char buffer[BUFFER_1024];
    sprintf_s(buffer, BUFFER_1024, "%i", some_param);
    std::thread t(f_passing_argument_test, 3, std::string(buffer));
    t.detach();
}

void update_date_for_widget(widget_id w, widget_data &data) {
    TICK();
    data = 2;
}

void process_widget_data(widget_data const &data) {
    TICK();
    std::cout << data << std::endl;
}

void display_status() {
    TICK();
}

void oops_again(widget_id w) {
    TICK();

    widget_data data = 1;
    std::thread t(update_date_for_widget, w, std::ref(data));//Not euqal with the book: the 2nd param must be a ref.
    display_status();
    t.join();
    process_widget_data(data);
}

void x_test() {
    TICK();

    X my_x;
    std::thread t(&X::do_lengthy_work, &my_x);
    t.join();
}

void x1_test() {
    TICK();

    X1 my_x1;
    int num(0);
    std::thread t(&X1::do_lengthy_work, &my_x1, num);
    t.join();
}

void process_big_object(std::unique_ptr<big_object>) {
    TICK();
}

void move_test() {
    TICK();

    std::unique_ptr<big_object> p(new big_object());
    p->prepare_data(42);
    std::thread t(process_big_object, std::move(p));
    t.detach();
}

void some_function() {
    TICK();
}

void some_other_function() {
    TICK();
}

void thread_move_test() {
    TICK();

    std::thread t1(some_function);
    std::thread t2 = std::move(t1);
    t1 = std::thread(some_other_function);
    std::thread t3;
    t3 = std::move(t2);
    t1 = std::move(t3); //This assignment will terminate program!
                        //In this case t1 already had an associated thread.
}


std::thread f() {
    TICK();
    return std::thread(some_function);
}

void some_other_function_1(int num) {
    TICK();
}

std::thread g() {
    TICK();
    std::thread t(some_other_function_1, 42);
    return t;
}

void g_test() {
    std::thread t = g();
    t.join();
}

void f(std::thread t) {
    TICK();
    t.join();
}

void g_1() {
    TICK();
    f(std::thread(some_function));
    std::thread t(some_function);
    f(std::move(t));
}

void scopt_thread_test() {
    TICK();

    int some_local_state = 42;
    FUNC f(some_local_state);
    std::thread t_(f);
    scoped_thread t(std::move(t_)); //Not equal to the book: scoped_thread`s constructor must be a right reference.
                                    //std::thread::thread(const std::thread &) already be deleted.
    do_something_in_current_thread();
}

void do_work(unsigned id) {
    TICK();
}

void f_spawn_threads() {
    TICK();
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < 20; ++i) {
        threads.push_back(std::thread(do_work, i));
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));//Call join() on each thread in turn.
}

}//namespace thread_manage


