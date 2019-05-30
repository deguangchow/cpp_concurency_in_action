///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter4: Synchronizing concurrent operations
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/29
#pragma once
#ifndef SYNCHRONIZING_CONCURRENT_OPERATIONS_H
#define SYNCHRONIZING_CONCURRENT_OPERATIONS_H

namespace sync_conc_opera {

//4.1 Waiting for an event or other condition
void test_wait_for_flag();

//4.1.1 Waiting for a condition with condition variables
//Listing 4.1 Waiting for data to process with a condition_variable
void test_wait_for_condition_variable();

//4.1.2 Building a thread-safe queue with condition variables
//Listing 4.2 queue interface
template<class T, class Container = std::deque<T>>
class queue {
private:
    Container data_queue;

public:
    explicit queue(Container const&) {}
    explicit queue(Container && = Container()) {}
    template<class Alloc> explicit queue(Alloc const&) {}
    template<class Alloc> queue(Container const&, Alloc const &) {}
    template<class Alloc> queue(Container&&, Alloc const&) {}
    template<class Alloc> queue(queue &&, Alloc const&) {}
    void swap(queue &q) {
        TICK();
        data_queue.swap(q);
    }
    bool empty() const {
        TICK();
        return data_queue.empty();
    }
    size_t size() const {
        TICK();
        return data_queue.size();
    }
    T& front() {
        TICK();
        return data_queue.front();
    }
    T const& front() const {
        TICK();
        return data_queue.front();
    }
    T& back() {
        TICK();
        return data_queue.back();
    }
    T const& back() {
        TICK();
        return data_queue.back();
    }
    void push(T const &x) {
        TICK();
        data_queue.push_back(x);
    }
    void push(T &&x) {
        TICK();
        data_queue.push_back(x);
    }
    void pop() {
        TICK();
        data_queue.pop_front();
    }
    template<class...Args>
    void emplace(Args&&...args) {
        TICK();
        data_queue.emplace(args);
    }
};

//Listing 4.3 The interface of your threadsafe_queue
//Listing 4.5 Full class definition for a thread-safe queue using condition variables
template<typename T>
class threadsafe_queue {
private:
    mutable mutex           m_mtxData; //The mutex must be mutable
    std::queue<T>           m_queueData;
    condition_variable      m_cvData;

public:
    threadsafe_queue() : m_mtxData(), m_queueData(), m_cvData() {}
    threadsafe_queue(threadsafe_queue const &other) : m_mtxData(), m_queueData(), m_cvData() {
        lock_guard<mutex> lk(other.m_mtxData);
        m_queueData = other.m_queueData;
    }
    threadsafe_queue& operator=(threadsafe_queue const&) = delete;  //Disallow assignment for simplicity

    void push(T new_value) {
        TICK();
        lock_guard <mutex> lock(m_mtxData);
        m_queueData.push(new_value);
        m_cvData.notify_one();
    }
    bool try_pop(T &value) {
        TICK();
        lock_guard<mutex> lock(m_mtxData);
        if (m_queueData.empty()) {
            return false;
        }
        value = m_queueData.front();
        m_queueData.pop();
        return true;
    }
    shared_ptr<T> try_pop() {
        TICK();
        lock_guard<mutex> lock(m_mtxData);
        if (m_queueData.empty()) {
            return make_shared<T>();
        }
        shared_ptr<T> ptrRes(make_shared<T>(m_queueData.front()));
        m_queueData.pop();
        return ptrRes;
    }
    void wait_and_pop(T &value) {
        TICK();
        unique_lock<mutex> ulock(m_mtxData);
        m_cvData.wait(ulock, [this] {return !m_queueData.empty(); });
        value = m_queueData.front();
        m_queueData.pop();
    }
    shared_ptr<T> wait_and_pop() {
        TICK();
        unique_lock<mutex> ulock(m_mtxData);
        m_cvData.wait(ulock, [this] {return !m_queueData.empty(); });
        shared_ptr<T> ptrRes(make_shared<T>(m_queueData.front()));
        m_cvData.pop();
        return ptrRes;
    }
    bool empty() const {
        lock_guard<mutex> lock(m_mtxData);
        return m_queueData.empty();
    }
};

void test_threadsafe_queue();

//4.2 Waiting for one-off events with futures
//4.2.1 Returning values from background tasks.
//Listing 4.6 Using future to get the return value of an asynchronous task
void test_future_async();

//Listing 4.7 Passing arguments to a function with async
struct X {
    int foo(int i, string const &s) {
        TICK();
        return TEN * i;
    }
    string bar(string const &s) {
        TICK();
        return string("bar: ") + s;
    }
};
struct Y {
    double operator()(double d) {
        TICK();
        return TEN * d;
    }
};
class move_only {
public:
    move_only() {}
    move_only(move_only &&) {}
    move_only(move_only const&) = delete;
    move_only& operator=(move_only &&) {}
    move_only& operator=(move_only const&) = delete;
    void operator()() {
        TICK();
    }
};
void test_future_async_struct();

//4.2.2 Associating a task with a future
//Listing 4.8 Partial class definaition for a specialization of packaged_task<>
#if 0//error: can`t be compiled correctly
template<>
class packaged_task<string(vector<char>*, int)> {
public:
    template<typename Callable>
    explicit packaged_task(Callable &&f) {}
    future<string> get_future() {
        TICK();
        return future<string>("");
    }
    void operator()(vector<char>*, int) {
        TICK();
    }
};
#endif

//Listing 4.9 Running code on a GUI thread using packaged_task
void test_packaged_task();

//4.2.3 Making ()promises
//Listing 4.10 Handling multiple connections from a single thread using promise
class payload_type {};
struct data_packet {
    unsigned        id;
    payload_type    payload;
};
struct outgoing_packet {
    unsigned        id;
    payload_type    payload;
    promise<bool>   promise;
};
class Connection {
public:
    Connection() {}
    ~Connection() {}
    bool has_incoming_data() {
        TICK();
        return true;
    }
    data_packet incoming() {
        TICK();
        return data_packet();
    }
    promise<payload_type> get_promise(unsigned const &id) {
        TICK();
        return promise<payload_type>();
    }
    bool has_outgoing_data() {
        TICK();
        return true;
    }
    outgoing_packet top_of_outgoing_queue() {
        TICK();
        return outgoing_packet();
    }
    void send(payload_type const &payload) {
        TICK();
    }
};
typedef shared_ptr<Connection>      Connection_ptr;
typedef set<Connection_ptr>         SetConnection;
typedef SetConnection::iterator     Connection_iterator;
void test_process_connections();

//4.2.4 Saving an exception for the future
void test_future_exception();

void test_promise_exception();

//4.3 Waiting with a time limit
//4.3.1 Clocks
//4.3.2 Durations
void test_durations();
//4.3.3 Time points
void test_time_points();
//Listing 4.11 Waiting for a condition variable with a timeout
void test_condition_variable_timeout();

//4.4 Using synchronization of operations to simplify code
//4.4.1 Funcional programming with futures
//Listing 4.12 A sequential implementation of Quicksort
template<typename T>
list<T> sequential_quick_sort(list<T> lstInput) {
    TICK();
    if (lstInput.empty()) {
        return lstInput;
    }
    list<T> lstResult;
    lstResult.splice(lstResult.begin(), lstInput, lstInput.begin());
    T const &tCompare = *lstResult.begin();

    auto posDivide = partition(lstInput.begin(), lstInput.end(),
        [&tCompare](T const &t) {return t < tCompare; });

    list<T> lstLowerPart;
    lstLowerPart.splice(lstLowerPart.end(), lstInput, lstInput.begin(), posDivide);

    auto lstNewLower(sequential_quick_sort(move(lstLowerPart)));
    auto lstNewHigher(sequential_quick_sort(move(lstInput)));
    lstResult.splice(lstResult.end(), lstNewHigher);
    lstResult.splice(lstResult.begin(), lstNewLower);
    return lstResult;
}

void test_sequential_quick_sort();

//Listing 4.13 Parallel Quicksort using futures
template<typename T>
list<T> parallel_quick_sort(list<T> lstInput) {
    TICK();
    if (lstInput.empty()) {
        return lstInput;
    }

    list<T> lstResult;
    lstResult.splice(lstResult.begin(), lstInput, lstInput.begin());
    T const &tCompare = *lstResult.begin();
    auto posDivide = partition(lstInput.begin(), lstInput.end(),
        [&tCompare](T const &t) {return t < tCompare; });

    list<T> lstLowerPart;
    lstLowerPart.splice(lstLowerPart.end(), lstInput, lstInput.begin(), posDivide);

    future<list<T>> lstNewLower_f(async(&parallel_quick_sort<T>, move(lstLowerPart)));
    auto            lstNewHigher(move(parallel_quick_sort(lstInput)));

    lstResult.splice(lstResult.end(), lstNewHigher);
    lstResult.splice(lstResult.begin(), lstNewLower_f.get());
    return lstResult;
}

void test_parallel_quick_sort();

//Listing 4.14 A sample implementation of spawn_task
#if 0//original sample
template<typename F, typename A>
future<typename result_of<F(A&&)>::type> spawn_task(F &&f, A &&a) {
    TICK();
    typedef result_of<F(A&&)>::type result_type;
    packaged_task<result_type(A&&)> ptask(move(f));
    future<result_type> result_f(ptask.get_future());
    thread t(move(ptask), move(a));
    t.detach();
    return result_f;
}
#elseif 0//extended sample
template<typename F, typename...Args>
auto spawn_task(F &&f, Args &&...args)->future<typename result_of<F(Args...)>::type> {
    TICK();
    using result_type = typename result_of<F(Args...)>::type;
    packaged_task<result_type(Args...)> ptask(move(f));
    future<result_type> result_f(ptask.get_future());
    thread t(move(ptask), forward<Args>(args)...);
    t.detach();
    return result_f;
}
#else//prefer this type to spawn task!
template<typename F, typename...Args, typename result_type = result_of<F(Args...)>::type>
future<result_type> spawn_task(F &&f, Args &&...args) {
    TICK();
    packaged_task<result_type(Args...)> ptask(move(f));
    future<result_type> result_f(ptask.get_future());
    thread t(move(ptask), forward<Args>(args)...);
    t.detach();
    return result_f;
}
#endif
void test_spawn_task();

//4.4.2 Sybchronizing operations with message passing
//Listing 4.15 A simple implementation of an ATM logic class
#if 0
namespace messaging {
class close_queue {
};

struct card_inserted {
    string account;
};
template<typename T>
class handle {
public:
    typedef function<void(T const &)> Func;
    void operator()(Func) {
        TICK();
    }
};
class receiver {
public:
    template<typename T>
    handle<T> wait() {
        TICK();
    }
};
class sender {
public:
    void send(int const &) {
        TICK();
    }
};
class digit_pressed {
public:
    unsigned digit;
};
class atm {
    messaging::receiver incoming;
    messaging::sender bank;
    messaging::sender interface_hardware;
    void(atm::*state)();

    string account;
    string pin;
    int display_enter_card() {
        TICK();
        return 0;
    }
    void waiting_for_card() {
        TICK();
        interface_hardware.send(display_enter_card());
        incoming.wait().handle<card_inserted>([&](card_inserted const &msg) {
            account = msg.account;
            pin = "";
            interface_hardware.send(display_enter_card());
            state = &atm::getting_pin;
        });
    }
    void getting_pin() {
        TICK();
        incoming.wait().handle<digit_pressed>([&](digit_pressed const& msg) {
            unsigned const pin_length = 4;
            pin += msg.digit;
            if (pin.length() == pin_length) {
                bank.send(verify_pin(account, pin, incoming));
            }
        });
    }
    int verify_pin(string const&, string const&, messaging::receiver const &) {
        TICK();
        return 0;
    }

public:
    void run() {
        TICK();
        state = &atm::waiting_for_card;
        try {
            for (;;) {
                (this->*state)();
            }
        }
        catch (messaging::close_queue const&) {
        }
    }
};
}//namespace messaging
#endif

}//namespace sync_conc_opera

#endif  //SYNCHRONIZING_CONCURRENT_OPERATIONS_H

