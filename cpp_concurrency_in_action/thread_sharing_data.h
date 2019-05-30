///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter3: Sharing data between threads.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/26
#pragma once
#ifndef THREAD_SHARING_DATA_H
#define THREAD_SHARING_DATA_H

namespace thread_sharing_data {

//3.1 Problems with sharing data between threads
//3.1.1 Race conditions
//3.1.2 Avoiding problematic race conditions

//3.2 Protecting shared data with mutexes
//3.2.1 Using mutexes in C++
//Listing 3.1 Protecting a list with a mutex

//3.2.2 Structing code for protecting shared data
//Listing 3.2 Accidentally passing out a reference to protected data
class some_data {
    int a;
    string b;
public:
    void do_something() {}
};
class data_wrapper {
    some_data   m_data;
    mutex       m_mutex;
public:
    template<typename Function>
    void process_data(Function func) {
        TICK();
        lock_guard<mutex> lock(m_mutex);
        func(m_data); //Pass "protected" data to user-supplied function!!!
    }
};
void test_foo();

//3.2.3 Spotting race conditions inherent in interfaces
//Listing 3.3 The interface to the stack container adapter
template<typename T, typename Container = deque<T>>
class stack {
    Container container;

public:
    explicit stack(const Container&) {}
    explicit stack(Container && = Container()) {}
    template<class Alloc> explicit stack(const Alloc&) {}
    template<class Alloc> stack(const Container&, const Alloc&) {}
    template<class Alloc> stack(Container&&, const Alloc&) {}
    template<class Alloc> stack(stack&&, const Alloc) {}

    bool empty() const {
        TICK();
        return container.empty();
    }
    size_t size() const {
        TICK();
        return container.size();
    }
    T& top() {
        TICK();
        INFO("val=%d\r\n", container.back());
        return container.back();
    }
    T const& top() const {
        TICK();
        INFO("val=%d\r\n", container.back());
        return container.back();
    }
    void push(T const &val) {
        TICK();
        INFO("val=%d\r\n", val);
        container.push_back(val);
    }
    void push(T &&val) {
        TICK();
        INFO("val=%d\r\n", val);
        container.push_back(val);
    }
    void pop() {
        TICK();
        container.pop_back();
    }
    void swap(stack &&s) {
        TICK();
        container.swap(s);
    }
};

void test_stack();

//Listing 3.4 An outline class defination for a thread-safe stack

//Listing 3.5 A fleshed-out class definition for a thread-safe stack
struct empty_stack : exception {
    const char* what() const throw() {
        TICK();
        return "empty_stack";
    }
};

template<typename T>
class thread_safe_stack {
    stack<T>        g_stkData;
    mutable mutex   g_mtxData;

public:
    thread_safe_stack() {}
    thread_safe_stack(const thread_safe_stack &other) : g_stkData(), g_mtxData() {
        TICK();
        lock_guard<mutex> lock(other.g_mtxData);
        g_stkData = other.g_stkData;  //Copy performed in constructor body
    }
    thread_safe_stack& operator=(const thread_safe_stack &) = delete; //Assignment operator is deleted

    void push(T new_value) {
        TICK();
        lock_guard<mutex> lock(g_mtxData);
        g_stkData.push(new_value);
    }
    shared_ptr<T> pop() {
        TICK();
        lock_guard<mutex> lock(g_mtxData);
        if (g_stkData.empty()) { //Check for empty before trying to pop value
            throw empty_stack();
        }
        shared_ptr<T> const ptrRes(make_shared<T>(g_stkData.top()));  //Allcate return value before modifying stack
        g_stkData.pop();
        return ptrRes;
    }
#if 0//If there are some override functions, the thread object init with it, something going wrong when compiling.
    void pop(T &value) {
        TICK();
        lock_guard<mutex> lock(g_mtxData);
        if (g_stkData.empty()) {
            throw empty_stack();
        }
        value = g_stkData.top();
        g_stkData.pop();
    }
#endif
    bool empty() const {
        TICK();
        lock_guard<mutex> lock(g_mtxData);
        return g_stkData.empty();
    }
};

void test_thread_safe_stack();

//3.2.4 Deadlock: the problem and a solution
//Listing 3.6 Using lock() and lock_guard in a swap operation
template<typename T>
class some_big_object {
    T   m_data;

public:
    explicit some_big_object(T val) : m_data(val) {}
    void set_data(T const &val) {
        m_data = val;
    }
    T get_data() {
        return m_data;
    }
};

template<typename T>
void swap(some_big_object<T> &lhs, some_big_object<T> &rhs) {
    TICK();
    T tmp = lhs.get_data();
    lhs.set_data(rhs.get_data());
    rhs.set_data(tmp);
}

template<typename T>
class X {
private:
    some_big_object<T>  m_tSomeDetail;
    mutex               m_mtxData;
public:
    explicit X(some_big_object<T> const &sd) :m_tSomeDetail(sd) {}

    friend void swap(X<T> &lhs, X<T> &rhs) {
        TICK();
        if (&lhs == &rhs) {
            return;
        }
        std::lock(lhs.m_mtxData, rhs.m_mtxData);
        lock_guard<mutex> lock_a(lhs.m_mtxData, adopt_lock);
        lock_guard<mutex> lock_b(rhs.m_mtxData, adopt_lock);
        swap(lhs.m_tSomeDetail, rhs.m_tSomeDetail);
    }
};

void test_std_lock();

//3.2.5 Further guidelines for avoiding deadlock
//Listing 3.7 Using a lock hierarchy to prevent deadlock
//Listing 3.8 A simple hierarchical mutex
class hierarchical_mutex {
    mutex                               m_mtxInternal;
    unsigned long const                 m_uHierachy;
    unsigned long                       m_uRreviousHierarchy;
    static thread_local unsigned long   m_uThisThreadHierarchy_tl;//something wrong with 'thread_local' in vs2015.

    void check_for_hierarchy_violation() {
        TICK();
        DEBUG("%d, %d, %d", m_uHierachy, m_uRreviousHierarchy, m_uThisThreadHierarchy_tl);
        if (m_uThisThreadHierarchy_tl <= m_uHierachy) {
            ERR("mutex hierarchy violated");
            throw logic_error("mutex hierarchy violated");
        }
    }

    void update_hierarchy_value() {
        TICK();
        DEBUG("%d, %d, %d", m_uHierachy, m_uRreviousHierarchy, m_uThisThreadHierarchy_tl);
        m_uRreviousHierarchy = m_uThisThreadHierarchy_tl;
        m_uThisThreadHierarchy_tl = m_uHierachy;
        INFO("%d, %d, %d", m_uHierachy, m_uRreviousHierarchy, m_uThisThreadHierarchy_tl);
    }

public:
    explicit hierarchical_mutex(unsigned long value) : m_uHierachy(value), m_uRreviousHierarchy(0) {}
    void lock() {
        TICK();
        check_for_hierarchy_violation();
        m_mtxInternal.lock();
        update_hierarchy_value();
    }
    void unlock() {
        TICK();
        DEBUG("%d, %d, %d", m_uHierachy, m_uRreviousHierarchy, m_uThisThreadHierarchy_tl);
        m_uThisThreadHierarchy_tl = m_uRreviousHierarchy;
        m_mtxInternal.unlock();
        INFO("%d, %d, %d", m_uHierachy, m_uRreviousHierarchy, m_uThisThreadHierarchy_tl);
    }
    bool try_lock() {
        TICK();
        check_for_hierarchy_violation();
        if (!m_mtxInternal.try_lock()) {
            return false;
        }
        update_hierarchy_value();
        return true;
    }
};

void test_hierarchical_mutex();

//3.2.6 Flexible locking with unique_lock
//Listing 3.9 Using lock() and unique_lock in a swap oopration
template<typename T>
class X_EX {
private:
    some_big_object<T>  m_tSomeDetail;
    mutex               m_mtxData;
public:
    explicit X_EX(some_big_object<T> const &sd) : m_tSomeDetail(sd) {}
    friend void swap(X_EX<T> &lhs, X_EX<T> &rhs) {
        TICK();
        if (&lhs == &rhs) {
            return;
        }

        unique_lock<mutex> ulock_l(lhs.m_mtxData, defer_lock);
        unique_lock<mutex> ulock_r(rhs.m_mtxData, defer_lock); //defer_lock leaves mutexes unlocked
        std::lock(ulock_l, ulock_r);  //Mutexes are locked here
        swap(lhs.m_tSomeDetail, rhs.m_tSomeDetail);
    }
};

void test_std_lock_ex();

//3.2.7 Transferring mutex ownership between scopes
void test_process_data();

//3.2.8 Locking at an appropriate granularity
class some_class {};
typedef unsigned result_type;
void test_get_and_process_data();

//Listing 3.10 Locking one mutex at a time in a comparison operator
template<typename T>
class Y {
private:
    T               g_tSomeDetail;
    mutable mutex   g_mtxData;

    T get_detail() const {
        TICK();
        lock_guard<mutex> lock(g_mtxData);
        return g_tSomeDetail;
    }
public:
    explicit Y(T sd) :g_tSomeDetail(sd) {}
    friend bool operator==(Y<T> const &lhs, Y<T> const &rhs) {
        TICK();
        if (&lhs == &rhs) {
            return true;
        }
        T const &lhs_value = lhs.get_detail();
        T const &rhs_value = rhs.get_detail();
        return lhs_value == rhs_value;
    }
};

void test_compare_operator();

//3.3 Alternative facilities for protecting shared data
//3.3.1 Protecting shared data during initialization
class some_resource {
public:
    void do_something() {
        TICK();
    }
};
void test_RAII();
void test_RAII_lock();
void test_RAII_lock_double_check();

void test_call_once();

//Listing 3.12 Threads-safe lazy initialization of a class member using call_once
class data_packet {};
class connection_info {
public:
    connection_info() {}
};
class connection_handle {
public:
    void send_data(data_packet const &data) {
        TICK();
    }
    data_packet receive_data() {
        TICK();
        return data_packet();
    }
};
class connection_manager {
public:
    connection_handle open(connection_info const &conn_info) {
        TICK();
        return connection_handle();
    }
};

class Connection {
private:
    connection_manager  m_connectionMgr;
    connection_info     m_connectionDetails;
    connection_handle   m_connectionHandle;
    once_flag           m_onceConnectionInitFlag;

    void open_connection() {
        TICK();
        INFO("Initialization is called exactly once");
        m_connectionHandle = m_connectionMgr.open(m_connectionDetails);
    }
public:
    explicit Connection(connection_info const &connection_details_) : m_connectionDetails(connection_details_) {}
    void send_data(data_packet const &data) {
        TICK();
        call_once(m_onceConnectionInitFlag, &Connection::open_connection, this);
        m_connectionHandle.send_data(data);
    }
    data_packet receive_data() {
        TICK();
        call_once(m_onceConnectionInitFlag, &Connection::open_connection, this);
        return m_connectionHandle.receive_data();
    }
};

void test_connection_call_once();
void test_connection_concurrency_call_once();

class my_class {};
my_class& get_my_class_instance();

//3.2.2 Protecting rarely updated data structures
//Listing 3.13 Protecting a data structure with a boost::share_mutex
//todo(deguangchow): there are no boost!!!

//3.3.3 Recursive locking

}//namespace thread_sharing_data

#endif  //THREAD_SHARING_DATA_H
