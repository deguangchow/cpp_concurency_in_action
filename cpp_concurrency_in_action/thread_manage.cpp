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


void background_task::operator()() const {
    TICK();
    do_something();
    do_something_else();
}


void test_launching_thread() {
    TICK();

    thread my_thread1(do_some_work);
    my_thread1.join();

    background_task f;
    thread my_thread2(f);
    my_thread2.join();

    thread my_thread3{ background_task() };
    my_thread3.join();

    thread my_thread4([] {
        do_something();
        do_something_else();
    });
    my_thread4.join();
}

void do_something(int &i) {
    TICK();
}

void func::operator()() {
    TICK();
    for (unsigned j = 0; j < MILLION; ++j) {
        do_something(i);//Potential access to dangling reference.
    }
}

void test_oops() {
    TICK();

    int nSomeLocalState = 0;
    FUNC my_func(nSomeLocalState);
    thread my_thread(my_func);
    my_thread.detach(); //Do not wait for the my_thread to finish
}                       //The my_thread might still be running!!!

void do_something_in_current_thread() {
    TICK();
    throw exception();//throw a exception.
}

void test_oops_exception() {
    TICK();

    int nSomeLocalState = 0;
    FUNC my_func(nSomeLocalState);
    thread t(my_func);
    try {
        do_something_in_current_thread();
    } catch (...) {//catch a exception.
        t.join();
        throw;
    }
    t.join();
}

void test_thread_guard() {
    TICK();

    int nSomeLocalState = 0;
    FUNC my_func(nSomeLocalState);
    thread t(my_func);
    thread_guard threadGuard(t);

    do_something_in_current_thread();
}

void do_background_work() {
    TICK();
}

void run_thread_background() {
    TICK();

    thread t(do_background_work);
    t.detach();
    assert(!t.joinable());
}

void open_document_and_display(string const &filename) {
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

//tips: don`t try to return a refrence, or you will get noting but an error:
//(error) Reference to temporary returned.
string const get_filename_from_user() {
    TICK();
    return string();
}

void process_user_input(user_command const &cmd) {
    TICK();
}

void edit_document(string const &filename) {
    TICK();

    open_document_and_display(filename);
    while (!done_editing()) {
        user_command cmd = get_user_input();
        if (cmd.type == open_new_document) {
            string const &new_name = get_filename_from_user();
            thread t(edit_document, new_name);
            t.detach();
        } else {
            process_user_input(cmd);
        }
        yield();
    }
}

void test_edit_document() {
    TICK();

    string const &filename = "a.doc";
    edit_document(filename);
}

void f_passing_argument_test(int i, string const& s) {
    TICK();
}

void test_oops(int some_param) {
    TICK();

    char buffer[BUFFER_1024];
    sprintf_s(buffer, BUFFER_1024, "%i", some_param);
    thread t(f_passing_argument_test, 3, buffer);
    t.detach();
}

void test_not_oops(int some_param) {
    TICK();

    char buffer[BUFFER_1024];
    sprintf_s(buffer, BUFFER_1024, "%i", some_param);
    thread t(f_passing_argument_test, 3, string(buffer));
    t.detach();
}

void update_date_for_widget(widget_id w, widget_data &data) {
    TICK();
    data = 2;
}

void process_widget_data(widget_data const &data) {
    TICK();
    cout << data << endl;
}

void display_status() {
    TICK();
}

void test_oops_again(widget_id w) {
    TICK();

    widget_data data = 1;
    thread t(update_date_for_widget, w, ref(data));//Not euqal with the book: the 2nd param must be a ref.
    display_status();
    t.join();
    process_widget_data(data);
}

void test_x() {
    TICK();

    X my_x;
    thread t(&X::do_lengthy_work, &my_x);
    t.join();
}

void test_x1() {
    TICK();

    X1 my_x1;
    int num(0);
    thread t(&X1::do_lengthy_work, &my_x1, num);
    t.join();
}

void process_big_object(unique_ptr<big_object>) {
    TICK();
}

void test_move() {
    TICK();

    unique_ptr<big_object> ptrBigObject(new big_object());
    ptrBigObject->prepare_data(42);
    thread t(&process_big_object, move(ptrBigObject));
    t.detach();
}

void some_function() {
    TICK();
}

void some_other_function() {
    TICK();
}

void test_thread_move() {
    TICK();

    thread t1(some_function);
    thread t2 = move(t1);
    t1 = thread(some_other_function);
    thread t3;
    t3 = move(t2);
    t1 = move(t3); //This assignment will terminate program!
                        //In this case t1 already had an associated thread.
}


void some_other_function_1(int num) {
    TICK();
}

thread get_thread() {
    TICK();
    thread t(some_other_function_1, 42);
    return t;
}

void test_get_thread() {
    TICK();
    thread t = get_thread();
    t.join();
}

void input_thread(thread t) {
    TICK();
    t.join();
}

void test_get_thread1() {
    TICK();
    input_thread(thread(some_function));
    thread t(some_function);
    input_thread(move(t));
}

void test_scopt_thread() {
    TICK();

    int nSomeLocalState = 42;
    FUNC f(nSomeLocalState);
    thread t_(f);
    scoped_thread t(move(t_)); //Not equal to the book: scoped_thread`s constructor must be a right reference.
                                    //thread::thread(const thread &) already be deleted.
    do_something_in_current_thread();
}

void do_work(unsigned id) {
    TICK();
}

void test_spawn_threads() {
    TICK();
    vector<thread> vctThreads;
    for (unsigned i = 0; i < 20; ++i) {
        vctThreads.push_back(thread(do_work, i));
    }
    for_each(vctThreads.begin(), vctThreads.end(), mem_fn(&thread::join));//Call join() on each thread in turn.
}

void test_parallel_accumulate() {
    TICK();
    unsigned uResult = parallel_accumulate(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), 0);
    INFO("uResult=%d", uResult);
}

void do_master_thread_work() {
    TICK();
}

void do_common_work() {
    TICK();
}

void some_core_part_of_algorithm(thread::id master_thread) {
    TICK();

    if (get_id() == master_thread) {
        do_master_thread_work();
    }
    do_common_work();
}

void test_identifying_threads() {
    TICK();
    thread::id master_thread = get_id();
    thread t(some_core_part_of_algorithm, master_thread);
    t.join();
}

}//namespace thread_manage


