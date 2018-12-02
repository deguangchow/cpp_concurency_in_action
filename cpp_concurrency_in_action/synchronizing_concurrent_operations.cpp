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

}//namespace sync_conc_opera


