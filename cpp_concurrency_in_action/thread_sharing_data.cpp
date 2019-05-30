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

list<int>  g_lstSome;
mutex      g_mutexSome;

void add_to_list(int new_value) {
    TICK();
    lock_guard<mutex> lock(g_mutexSome);
    g_lstSome.push_back(new_value);
}

bool list_contains(int value_to_find) {
    TICK();
    lock_guard<mutex> lock(g_mutexSome);
    return find(g_lstSome.begin(), g_lstSome.end(), value_to_find) != g_lstSome.end();
}

some_data       *g_pUnprotectedData;
void malicious_function(some_data &protected_data) {
    TICK();
    g_pUnprotectedData = &protected_data;
}
data_wrapper    g_datawrapperX;
void test_foo() {
    TICK();
    g_datawrapperX.process_data(malicious_function); //Pass in a malicious function
    g_pUnprotectedData->do_something();        //Unprotected access to protected data
}

void do_something(int val) {
    TICK();
}

void test_stack() {
    TICK();
    stack<int> stkData;
    stkData.push(1);
    stkData.push(2);
    stkData.push(3);
    while (!stkData.empty()) {
        int const value = stkData.top();
        stkData.pop();
        do_something(value);
    }
}

thread_safe_stack<int>          g_threadSafeStack;
void test_thread_safe_stack() {
    TICK();

    unsigned const PUSH_THREAD_NUM  = THREAD_NUM_128 - 1;
    unsigned const POP_THREAD_NUM   = THREAD_NUM_128;

    vector<thread> vctPushThreads(PUSH_THREAD_NUM);
    vector<thread> vctPopThreads(POP_THREAD_NUM);
    try {
        for (unsigned i = 0; i < PUSH_THREAD_NUM; ++i) {
            vctPushThreads[i] = thread(&thread_safe_stack<int>::push, &g_threadSafeStack, i);
        }
        for (unsigned i = 0; i < POP_THREAD_NUM; ++i) {
            vctPopThreads[i] = thread(&thread_safe_stack<int>::pop, &g_threadSafeStack);
        }
        for (unsigned i = 0; i < PUSH_THREAD_NUM; ++i) {
            vctPushThreads[i].join();
        }
        for (unsigned i = 0; i < POP_THREAD_NUM; ++i) {
            vctPopThreads[i].join();  //When the num of threads to pop data from the stack is more than to push,
                                      //there must be an "empty_stack" assert!
        }
    } catch (empty_stack const& e) {// can not catch anything!!!
        ERR("catch empty_stack: %s", e.what());
    } catch (...) {
        ERR("catch ...");
    }
}

void test_std_lock() {
    TICK();
    X<int> x1(some_big_object<int>(1));
    X<int> x2(some_big_object<int>(2));
    swap(x1, x2);
}

//3.2.5 Further guidelines for avoiding deadlock
//Listing 3.7 Using a lock hierarchy to prevent deadlock
thread_local unsigned long hierarchical_mutex::m_uThisThreadHierarchy_tl(ULONG_MAX);
hierarchical_mutex g_hmtxHighLevel(TEN_THOUSAND);
hierarchical_mutex g_hmtxLowHevel(THOUSAND * 5);
hierarchical_mutex g_hmtxOther(HUNDRED);

int do_low_level_stuff() {
    TICK();
    return 0;
}

int low_level_func() {
    TICK();
    lock_guard<hierarchical_mutex> lock(g_hmtxLowHevel);
    return do_low_level_stuff();
}

void do_high_level_stuff(int some_param) {
    TICK();
}

void high_level_func() {
    TICK();
    lock_guard<hierarchical_mutex> lock(g_hmtxHighLevel);
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
    lock_guard<hierarchical_mutex> lock(g_hmtxOther);
    other_stuff();
}

void test_hierarchical_mutex() {
    TICK();
    thread t1(thread_a);
    thread t2(thread_b);
    t1.join();
    t2.join();
}

void test_std_lock_ex() {
    TICK();
    X_EX<int> x1(some_big_object<int>(1));
    X_EX<int> x2(some_big_object<int>(2));
    swap(x1, x2);
}

void prepare_data() {
    TICK();
}

unique_lock<mutex> get_lock() {
    TICK();
    extern mutex g_mutexSome;
    unique_lock<mutex> ulock(g_mutexSome);
    prepare_data();
    return ulock;
}


void do_something() {
    TICK();
}

void test_process_data() {
    TICK();
    unique_lock<mutex> ulock(get_lock());
    do_something();
}

//3.2.8 Locking at an appropriate granularity
mutex           g_mtxTheOne;
thread_sharing_data::some_class get_next_data_chunk() {
    TICK();
    return some_class();
}

thread_sharing_data::result_type process(some_class data) {
    TICK();
    return 0;
}

void write_result(some_class const& data, result_type &result) {
    TICK();
}

void test_get_and_process_data() {
    TICK();

    unique_lock<mutex> ulock(g_mtxTheOne);
    some_class data_to_process = get_next_data_chunk();

    INFO("Don`t need mutex locked across call to process()");
    ulock.unlock();
    result_type result = process(data_to_process);

    INFO("Relock mutex to write result");
    ulock.lock();
    write_result(data_to_process, result);
}

void test_compare_operator() {
    TICK();
    int i1 = 1;
    int i2 = 2;
    Y<int> y1(i1);
    Y<int> y2(i2);
    INFO("%d %s %d", i1, y1 == y2 ? "==" : "!=", i2);
}

//3.3 Alternative facilities for protecting shared data
//3.3.1 Protecting shared data during initialization
shared_ptr<some_resource>   g_ptrResource = nullptr;
void test_RAII() {
    TICK();
    if (!g_ptrResource) {
        g_ptrResource.reset(new some_resource);
    }
    g_ptrResource->do_something();
}

mutex                       g_mtxResource;
void test_RAII_lock() {
    TICK();
    unique_lock<mutex> ulock(g_mtxResource);    //All threads are serialized here
    if (!g_ptrResource) {
        g_ptrResource.reset(new some_resource);  //Only the initialization needs protection
    }
    ulock.unlock();
    g_ptrResource->do_something();
}

void test_RAII_lock_double_check() {
    TICK();
    if (!g_ptrResource) {
        lock_guard<mutex> lock(g_mtxResource);
        if (!g_ptrResource) {
            g_ptrResource.reset(new some_resource);
        }
    }
    g_ptrResource->do_something();
}

once_flag                   g_onecResource;
void init_resource() {
    TICK();
    INFO("Initialization is called exactly once");
    g_ptrResource.reset(new some_resource);
}
void test_once_flag() {
    TICK();
    call_once(g_onecResource, init_resource);   //Initialization is called exactly once
}

void test_call_once() {
    TICK();
    unsigned const &uNumThread = THREAD_NUM_8;
    vector<thread> vctThreads(uNumThread);
    for (unsigned i = 0; i < uNumThread; ++i) {
        vctThreads[i] = thread(test_once_flag);
    }
    for (unsigned i = 0; i < uNumThread; ++i) {
        vctThreads[i].join();
    }
}

void test_connection_call_once() {
    TICK();
    connection_info conn_info;
    Connection conn(conn_info);
    data_packet data1;
    data_packet data2;
    conn.send_data(data1);
    conn.send_data(data2);
    data_packet const &data3 = conn.receive_data();
    data_packet const &data4 = conn.receive_data();
}

void test_connection_concurrency_call_once() {
    TICK();
    connection_info conn_info;
    Connection conn(conn_info);
    data_packet data1;
    data_packet data2;
    thread t1(&Connection::send_data, &conn, data1);
    thread t2(&Connection::send_data, &conn, data2);
#if 0
    thread t3(&Connection::receive_data, &conn);
    thread t4(&Connection::receive_data, &conn);
#else
    future<data_packet> datePacket3_f = async(&Connection::receive_data, &conn);
    future<data_packet> datePacket4_f = async(&Connection::receive_data, &conn);
#endif

    t1.join();
    t2.join();
#if 0
    t3.join();
    t4.join();
#else
    auto const& dataPacket3 = datePacket3_f.get();
    auto const& dataPacket4 = datePacket4_f.get();
#endif
}

my_class& get_my_class_instance() {
    static my_class instance;   //Initialization guaranteed to be thread-safe
    return instance;
}

}//namespace thread_sharing_data


