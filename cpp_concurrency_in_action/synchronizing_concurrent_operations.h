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
void wait_for_flag();

//4.1.1 Waiting for a condition with condition variables
//Listing 4.1 Waiting for data to process with a condition_variable
class data_chunk {};
bool more_data_to_prepare();
data_chunk prepare_data();
void data_preparation_thread();
void process_data(data_chunk data);
bool is_last_chunk(data_chunk data);
void data_processing_thread();
void wait_for_condition_variable();

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
    mutable mutex mut; //The mutex must be mutable
    std::queue<T> data_queue;
    condition_variable data_cond;

public:
    threadsafe_queue() : mut(), data_queue(), data_cond() {}
    threadsafe_queue(threadsafe_queue const &other) : mut(), data_queue(), data_cond() {
        lock_guard<mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    threadsafe_queue& operator=(threadsafe_queue const&) = delete;  //Disallow assignment for simplicity

    void push(T new_value) {
        TICK();
        lock_guard <mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }
    bool try_pop(T &value) {
        TICK();
        lock_guard<mutex> lk(mut);
        if (data_queue.empty()) {
            return false;
        }
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    shared_ptr<T> try_pop() {
        TICK();
        lock_guard<mutex> lk(mut);
        if (data_queue.empty()) {
            return make_shared<T>();
        }
        shared_ptr<T> res(make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }
    void wait_and_pop(T &value) {
        TICK();
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }
    shared_ptr<T> wait_and_pop() {
        TICK();
        unique_lock<mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        shared_ptr<T> res(make_shared<T>(data_queue.front()));
        data_cond.pop();
        return res;
    }
    bool empty() const {
        lock_guard<mutex> lc(mut);
        return data_queue.empty();
    }
};

void data_preparation_safe_thread();
void data_processing_safe_thread();
void threadsafe_queue_test();

//4.2 Waiting for one-off events with futures
//4.2.1 Returning values from background tasks.
//Listing 4.6 Using future to get the return value of an asynchronous task
int find_the_answer_to_ltuae();
void do_other_stuff();
void future_async_test();

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
void future_async_struct_test();

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

bool gui_shutdown_message_received();
void get_and_process_gui_message();
void gui_thread();
template<typename Func>
future<void> post_task_for_gui_thread(Func f);
void packaged_task_test();

//4.2.3 Making ()promises
//Listing 4.10 Handling multiple connections from a single thread using promise
class payload_type {};
struct data_packet {
    unsigned id;
    payload_type payload;
};
struct outgoing_packet {
    unsigned id;
    payload_type payload;
    promise<bool> promise;
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
typedef shared_ptr<Connection> Connection_ptr;
typedef set<Connection_ptr> SetConnection;
typedef SetConnection::iterator Connection_iterator;
bool done(SetConnection const &connections);
void process_connections(SetConnection &connections);
void process_connections_test();

//4.2.4 Saving an exception for the future
double square_root(double x);
void future_exception_test();

double calcuate_value();
void promise_exception();
void promise_exception_test();

//4.3 Waiting with a time limit
//4.3.1 Clocks
//4.3.2 Durations
int some_task();
void do_something_with(int const &val);
void durations_test();
//4.3.3 Time points
void do_something();
void time_points_test();
//Listing 4.11 Waiting for a condition variable with a timeout
bool wait_until_loop();
void condition_variable_timeout_test();

//4.4 Using synchronization of operations to simplify code
//4.4.1 Funcional programming with futures
//Listing 4.12 A sequential implementation of Quicksort
template<typename T>
list<T> sequential_quick_sort(list<T> input) {
    TICK();
    if (input.empty()) {
        return input;
    }
    list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const &pivot = *result.begin();

    auto divide_point = partition(input.begin(), input.end(),
        [&pivot](T const &t) {return t < pivot; });

    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    auto new_lower(sequential_quick_sort(move(lower_part)));
    auto new_higher(sequential_quick_sort(move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

void sequential_quick_sort_test();

//Listing 4.13 Parallel Quicksort using futures
template<typename T>
list<T> parallel_quick_sort(list<T> input) {
    TICK();
    if (input.empty()) {
        return input;
    }

    list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const &pivot = *result.begin();
    auto divide_point = partition(input.begin(), input.end(),
        [&pivot](T const &t) {return t < pivot; });

    list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    future<list<T>> new_lower(async(&parallel_quick_sort<T>, move(lower_part)));
    auto new_higher(move(parallel_quick_sort(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

void parallel_quick_sort_test();

//Listing 4.14 A sample implementation of spawn_task
#if 0//original sample
template<typename F, typename A>
future<typename result_of<F(A&&)>::type> spawn_task(F &&f, A &&a) {
    TICK();
    typedef result_of<F(A&&)>::type result_type;
    packaged_task<result_type(A&&)> task(move(f));
    future<result_type> res(task.get_future());
    thread t(move(task), move(a));
    t.detach();
    return res;
}
#else//extended sample
template<typename F, typename...Args>
auto spawn_task(F &&f, Args &&...args)->future<typename result_of<F(Args...)>::type> {
    TICK();
    using result_type = typename result_of<F(Args...)>::type;
    packaged_task<result_type(Args...)> task(move(f));
    future<result_type> res(task.get_future());
    thread t(move(task), forward<Args>(args)...);
    t.detach();
    return res;
}
#endif
double do_multiplus_work(double const &val);
int do_mod(int const &val);
int do_1();
void spawn_task_test();

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

