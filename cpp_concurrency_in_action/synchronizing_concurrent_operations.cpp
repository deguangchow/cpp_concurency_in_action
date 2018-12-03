///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter4: Synchronizing concurrent operations
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/29

#include "stdafx.h"
#include "synchronizing_concurrent_operations.h"

namespace sync_conc_opera {

//4.1 Waiting for an event or other condition
bool flag = false;
std::mutex m;
void wait_for_flag() {
    TICK();
    std::unique_lock<std::mutex> lk(m);
    while (!flag) {
        DEBUG("Unlock the mutex");
        lk.unlock();
#if 0//_DEBUG
        unsigned const &SLEEP_TIME_MS = THOUSAND;   //1000 ms
        INFO("Sleep for %d(ms)", SLEEP_TIME_MS);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
#endif//_DEBUG
        DEBUG("Relock the mutex");
        lk.lock();
    }
}

//4.1.1 Waiting for a condition with condition variables
//Listing 4.1 Waiting for data to process with a std::condition_variable
std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;
bool more_data_to_prepare() {
    TICK();
    return true;
}
data_chunk prepare_data() {
    TICK();
    INFO("prepare_data thread_id=%d", std::this_thread::get_id());
#if 0//_DEBUG
    unsigned const &SLEEP_TIME_MS = 5 * HUNDRED;   //500 ms
    INFO("Sleep for %d(ms)", SLEEP_TIME_MS);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
#endif//_DEBUG
    return data_chunk();
}
void data_preparation_thread() {
    TICK();
    while (more_data_to_prepare()) {
        data_chunk const data = prepare_data();
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
}
void process_data(data_chunk data) {
    TICK();
    INFO("process_data thread_id=%d", std::this_thread::get_id());
}
bool is_last_chunk(data_chunk data) {
    TICK();
    return false;
}
void data_processing_thread() {
    TICK();
    while (true) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [] {return !data_queue.empty(); });
        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();
        process_data(data);
        if (is_last_chunk(data)) {
            break;
        }
    }
}
void wait_for_condition_variable() {
    TICK();
    std::thread t1(data_preparation_thread);
    std::thread t2(data_processing_thread);
    t1.join();
    t2.join();
}

//Listing 4.3 The interface of your threadsafe_queue
//Listing 4.5 Full class definition for a thread-safe queue using condition variables
threadsafe_queue<data_chunk> data_threadsafe_queue;
void data_preparation_safe_thread() {
    TICK();
    while (more_data_to_prepare()) {
        data_chunk const &data = prepare_data();
        data_threadsafe_queue.push(data);
    }
}
void data_processing_safe_thread() {
    TICK();
    while (true) {
        data_chunk data;
        data_threadsafe_queue.wait_and_pop(data);
        process_data(data);
        if (is_last_chunk(data)) {
            break;
        }
    }
}
void threadsafe_queue_test() {
    TICK();
    std::thread t1(data_preparation_safe_thread);
    std::thread t2(data_preparation_safe_thread);
    std::thread t3(data_processing_safe_thread);
    std::thread t4(data_processing_safe_thread);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

//4.2 Waiting for one-off events with futures
//4.2.1 Returning values from background tasks.
//Listing 4.6 Using std::future to get the return value of an asynchronous task
int find_the_answer_to_ltuae() {
    TICK();
    return 42;
}
void do_other_stuff() {
    TICK();
}
void future_async_test() {
    TICK();
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
    do_other_stuff();
    INFO("The answer is %d", the_answer.get());
}

//Listing 4.7 Passing arguments to a function with std::async
void future_async_struct_test() {
    TICK();
    X x;
    std::future<int> f1 = std::async(&X::foo, &x, 42, "hello"); //Calls p->foo(42, "hello") where p is &x
    std::future<std::string> f2 = std::async(&X::bar, x, "goodbye");    //Calls tmpx.bar() where tmpx is a copy of x
    INFO("f1=%d", f1.get());
    INFO("f2=%s", f2.get().c_str());

    Y y;
    std::future<double> f3 = std::async(Y(), 3.141);    //Calls tmpy(3.141) where tmpy is move-constructed from Y()
    std::future<double> f4 = std::async(std::ref(y), 2.718);    //Calls y(2.718)
    INFO("f3=%f", f3.get());
    INFO("f4=%f", f4.get());

#if 0//strange usage: can`t be compiled correctly
    X baz(X&);
    std::async(baz, std::ref(x));   //Calls baz(x)
#endif

    std::future<void> f5 = std::async(move_only());
    std::future<double> f6 = std::async(std::launch::async, Y(), 1.2);  //Run in new thread
    INFO("f6=%f", f6.get());
#if 0//strange usage: can`t be compiled correctly
    std::future<X> f7 = std::async(std::launch::deferred, baz, std::ref(x));    //Run in wait() or get()
    std::future<X> f8 = std::async(std::launch::deferred | std::launch::async, baz, std::ref(x));
    std::future<X> f9 = std::async(baz, std::ref(x));   //Implementation chooses
    f7.wait();  //Invoke deferred function
#endif
}

//Listing 4.9 Running code on a GUI thread using std::packaged_task
std::deque<std::packaged_task<void()>> tasks;
std::mutex mutex_tasks;
std::condition_variable cond_tasks;
bool gui_shutdown_message_received() {
    TICK();
    std::unique_lock<std::mutex> lk(mutex_tasks);
    cond_tasks.wait(lk, [] {return !tasks.empty(); });
    return false;
}
void get_and_process_gui_message() {
    TICK();
}
void gui_thread() {
    TICK();
    while (!gui_shutdown_message_received()) {
        get_and_process_gui_message();
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(mutex_tasks);
            if (tasks.empty()) {
                continue;
            }
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        task();
    }
}
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f) {
    TICK();
    std::packaged_task<void()> task(f);
    std::future<void> res = task.get_future();
    std::lock_guard<std::mutex> lk(mutex_tasks);
    tasks.push_back(std::move(task));
    cond_tasks.notify_one();
    return res;
}
void do_some_task() {
    TICK();
}
int do_other_task(int const &value) {
    TICK();
    return TEN * value;
}
void packaged_task_test() {
    TICK();
    std::thread gui_bg_thread(gui_thread);
    gui_bg_thread.detach();

    while (true) {
        std::thread task_thread(post_task_for_gui_thread<void()>, do_some_task);
        task_thread.detach();

#if _DEBUG
        unsigned const &THREAD_SLEEP_TIME_MS = 5 * HUNDRED;
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));
#endif
    }
}

//4.2.3 Making (std::)promises
//Listing 4.10 Handling multiple connections from a single thread using promise
bool done(SetConnection const &connections) {
    TICK();
    return false;
}
void process_connections(SetConnection &connections) {
    TICK();
    while (!done(connections)) {
        for (Connection_iterator &pos = connections.cbegin(); pos != connections.cend(); ++pos) {
            Connection_ptr const& conn = *pos;
            if (conn->has_incoming_data()) {
                data_packet data = conn->incoming();
                std::promise<payload_type>& p = conn->get_promise(data.id);
                p.set_value(data.payload);
            }
            if (conn->has_outgoing_data()) {
                outgoing_packet data = conn->top_of_outgoing_queue();
                conn->send(data.payload);
                data.promise.set_value(true);
            }
        }
    }
}
void process_connections_test() {
    TICK();
    SetConnection connections;
    connections.insert(std::make_shared<Connection>());
    connections.insert(std::make_shared<Connection>());
    connections.insert(std::make_shared<Connection>());
    process_connections(connections);
}

double square_root(double x) {
    TICK();
    if (x < 0) {
        throw std::out_of_range("x<0");
    }
    return sqrt(x);
}

void future_exception_test() {
    TICK();
    try {
        double d1 = 256;
        std::future<double> f1 = std::async(square_root, d1);
        INFO("async square_root(%f)=%f", d1, f1.get());

        double d2 = -1;
        std::future<double> f2 = std::async(square_root, d2);
        INFO("async square_root(%f)=%f", d2, f2.get());
    } catch (std::exception e) {
        INFO("catch exception: %s", e.what());
    }
}

double calcuate_value() {
    TICK();
    throw std::out_of_range("calcuate_value exception.");
    return 3.141;
}
std::promise<double> some_promise;
void promise_exception() {
    TICK();
    try {
        some_promise.set_value(calcuate_value());
    }
    catch (std::exception e) {
        INFO("catch exception: %s", e.what());
        some_promise.set_exception(std::current_exception());
    }
}
void promise_exception_test() {
    TICK();
    promise_exception();

    std::future<double> f = some_promise.get_future();
    assert(f.valid());
    std::shared_future<double> sf(std::move(f));
#if 0
    assert(!f.valid());
#endif
    assert(sf.valid());
}

//4.3 Waiting with a time limit
//4.3.1 Clocks
//3.3.2 Durations
int some_task() {
    TICK();
    return 42;
}
void do_something_with(int const &val) {
    TICK();
}
void durations_test() {
    TICK();
    std::chrono::milliseconds ms(54802);
    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
    INFO("ms=%ld, s=ld", ms.count(), s.count());    //error: The print of s is 0

    std::chrono::seconds s1(54802);
    INFO("s1=%ld", s1.count()); //The print of s1 is 54802

    std::future<int> f = std::async(some_task);
    if (f.wait_for(std::chrono::milliseconds(35)) == std::future_status::ready) {
        int const &res = f.get();
        INFO("f=%d", res);
        do_something_with(res);
    }
}
//4.3.3 Time points
void do_something() {
    TICK();
    unsigned const &THREAD_SLEEP_TIME_MS = 5 * HUNDRED;
    std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MS));
}

void time_points_test() {
    TICK();
    auto start = std::chrono::high_resolution_clock::now();
    do_something();
    auto stop = std::chrono::high_resolution_clock::now();
    auto tick = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    INFO("do_something() took %ld milliseconds", tick);
}

//Listing 4.11 Waiting for a condition variable with a timeout
std::condition_variable cv;
bool done_wait = false;
std::mutex mutex_wait;
unsigned const &CV_WAIT_TIME_MS = 5 * HUNDRED;
bool wait_until_loop() {
    TICK();
    while (!done_wait) {
        auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(CV_WAIT_TIME_MS);
        std::unique_lock<std::mutex> lk(mutex_wait);
        if (cv.wait_until(lk, timeout) == std::cv_status::timeout) {
            break;
        }
    }
    return done_wait;
}
bool wait_for_loop() {
    TICK();
    while (!done_wait) {
        std::unique_lock<std::mutex> lk(mutex_wait);
        if (cv.wait_for(lk, std::chrono::milliseconds(CV_WAIT_TIME_MS)) == std::cv_status::timeout) {
            break;
        }
    }
    return done_wait;
}
void condition_variable_timeout_test() {
    TICK();
    done_wait = false;
    wait_until_loop();

    done_wait = false;
    wait_for_loop();

    cv.notify_one();
    cv.notify_one();
}

//4.4 Using synchronization of operations to simplify code
//4.4.1 Funcional programming with futures
//Listing 4.12 A sequential implementation of Quicksort
const static std::list<int> input = {
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8,
    4, 2, 6, 3, 2, 5, 10, 0, 2, 7, 9, 3, 1, 10, 8
};
void sequential_quick_sort_test() {
    TICK();
    TV_LIST_INT(input);

    std::list<int> result = sequential_quick_sort(input);
    TV_LIST_INT(result);
}

//Listing 4.13 Parallel Quicksort using futures
void parallel_quick_sort_test() {
    TICK();
    TV_LIST_INT(input);

    std::list<int> result = parallel_quick_sort(input);
    TV_LIST_INT(result);
}

}//namespace sync_conc_opera


