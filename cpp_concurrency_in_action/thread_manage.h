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
class background_task {
public:
    void operator()() const;
};
void test_launching_thread();


//Listing 2.1: A function that returns while a thread still has access to local variables.
typedef struct func {
    int &i;
    func(int &i_) :i(i_) {}
    void operator()();
}FUNC;
typedef shared_ptr<FUNC> FUNC_PTR;
void test_oops();

//2.1.2 Waiting for a thread to complete

//2.1.3 Waiting in exceptional circumstances
//Listing 2.2 Waiting for a thread to finish
void test_oops_exception();

//Listing 2.3 Using RAII to wait for a thread to complete
class thread_guard {
    thread          &m_thread;
public:
    explicit thread_guard(thread &t_) : m_thread(t_) { }
    ~thread_guard() {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};
void test_thread_guard();


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
    explicit user_command(DOC_OPREA_TYPE const &type_) :type(type_) {}
    ~user_command() {}
};
void test_edit_document();

//2.2 Passing arguments to a thread function
void test_oops(int some_param);
void test_not_oops(int some_param);

typedef unsigned widget_id;
typedef unsigned widget_data;
void test_oops_again(widget_id w);

class X {
public:
    void do_lengthy_work() { TICK(); }
};
void test_x();

class X1 {
public:
    void do_lengthy_work(int num) { TICK(); }
};
void test_x1();

struct big_object {
    unsigned data;
    void prepare_data(unsigned const &data_) {
        TICK();
        data = data_;
    }
};
void test_move();

//2.3 Transferring ownship of a thread
void test_thread_move();

//Listing 2.5 Returning a thread form a function
void test_get_thread();
void test_get_thread1();

//Listing 2.6 scoped_thread and example usage
class scoped_thread {
    thread          m_thread;
public:
    explicit scoped_thread(thread t_) :m_thread(move(t_)) {
        TICK();
        if (!m_thread.joinable()) {
            throw logic_error("No thread");
        }
    }
    ~scoped_thread() {
        TICK();
        m_thread.join();
    }
    //scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};
void test_scopt_thread();

//Listing 2.7 Spawn some threads and wait for them to finish
void test_spawn_threads();

//2.4 Choosing the number of threads at runtime
//Listing 2.8 A naive parallel version of accumulate
template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T &result) {
        TICK();
        result = accumulate(first, last, result);
    }
};
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    TICK();

    unsigned long const LENGTH = distance(first, last);
    if (!LENGTH) {
        return init;
    }

    unsigned long const MIN_PER_THREAD = 25;
    unsigned long const MAX_THREADS = (LENGTH + MIN_PER_THREAD - 1) / MIN_PER_THREAD;
    unsigned long const NUM_THREADS = min(HARDWARE_CONCURRENCY != 0 ? HARDWARE_CONCURRENCY : 2, MAX_THREADS);
    unsigned long const BLOCK_SIZE = LENGTH / NUM_THREADS;
    vector<T>           vctResults(NUM_THREADS);
    vector<thread>      vctThreads(NUM_THREADS - 1);

    Iterator            posBlockStart = first;
    for (unsigned i = 0; i < NUM_THREADS - 1; ++i) {
        Iterator posBlockEnd = posBlockStart;
        advance(posBlockEnd, BLOCK_SIZE);
        vctThreads[i] = thread(accumulate_block<Iterator, T>(), posBlockStart, posBlockEnd, ref(vctResults[i]));
        posBlockStart = posBlockEnd;
    }
    accumulate_block<Iterator, T>()(posBlockStart, last, vctResults[NUM_THREADS - 1]);
    for_each(vctThreads.begin(), vctThreads.end(), mem_fn(&thread::join));
    return accumulate(vctResults.begin(), vctResults.end(), init);
}

void test_parallel_accumulate();

//2.5 Identifying threads
void test_identifying_threads();


}//namespace thread_manage

#endif  //THREAD_MANAGE_H
