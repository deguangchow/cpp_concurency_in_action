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
bool    g_bFlag = false;
mutex   g_mutex;
void test_wait_for_flag() {
    TICK();
    unique_lock<mutex> ulock(g_mutex);
    while (!g_bFlag) {
        INFO("Unlock the mutex");
        ulock.unlock();
#if 1//_DEBUG
        common_fun::sleep(THOUSAND);   //1000 ms
#endif//_DEBUG
        INFO("Relock the mutex");
        ulock.lock();
    }
}

//4.1.1 Waiting for a condition with condition variables
//Listing 4.1 Waiting for data to process with a condition_variable
class data_chunk {};
mutex                   g_mutexData;
std::queue<data_chunk>  g_queueData;
condition_variable      g_cvData;
bool more_data_to_prepare() {
    TICK();
    return true;
}
data_chunk prepare_data() {
    TICK();
    INFO("prepare_data thread_id=%d", get_id());
#if 1//_DEBUG
    common_fun::sleep(5 * HUNDRED);   //500 ms
#endif//_DEBUG
    return data_chunk();
}
void data_preparation_thread() {
    TICK();
    while (more_data_to_prepare()) {
        data_chunk const data = prepare_data();
        lock_guard<mutex> lock(g_mutexData);
        g_queueData.push(data);
        g_cvData.notify_one();
        yield();
    }
}
void process_data(data_chunk data) {
    TICK();
    INFO("process_data thread_id=%d", get_id());
#if 1//_DEBUG
    common_fun::sleep(THOUSAND);   //1000 ms
#endif//_DEBUG
}
bool is_last_chunk(data_chunk data) {
    TICK();
    return false;
}
void data_processing_thread() {
    TICK();
    while (true) {
        unique_lock<mutex> ulock(g_mutexData);
        g_cvData.wait(ulock, [] {return !g_queueData.empty(); });
        data_chunk data = g_queueData.front();
        g_queueData.pop();
        ulock.unlock();
        process_data(data);
        if (is_last_chunk(data)) {
            break;
        }
        yield();
    }
}
void test_wait_for_condition_variable() {
    TICK();
    thread t1(data_preparation_thread);
    thread t2(data_processing_thread);
    t1.join();
    t2.join();
}

//Listing 4.3 The interface of your threadsafe_queue
//Listing 4.5 Full class definition for a thread-safe queue using condition variables
threadsafe_queue<data_chunk>        g_threadSafeQueueData;
void data_preparation_safe_thread() {
    TICK();
    while (more_data_to_prepare()) {
        data_chunk const &data = prepare_data();
        g_threadSafeQueueData.push(data);
        yield();
    }
}
void data_processing_safe_thread() {
    TICK();
    while (true) {
        data_chunk data;
        g_threadSafeQueueData.wait_and_pop(data);
        process_data(data);
        if (is_last_chunk(data)) {
            break;
        }
        yield();
    }
}
void test_threadsafe_queue() {
    TICK();
    thread t1(data_preparation_safe_thread);
    thread t2(data_preparation_safe_thread);

    thread t3(data_processing_safe_thread);
    thread t4(data_processing_safe_thread);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

//4.2 Waiting for one-off events with futures
//4.2.1 Returning values from background tasks.
//Listing 4.6 Using future to get the return value of an asynchronous task
int find_the_answer_to_ltuae() {
    TICK();
    common_fun::sleep(THOUSAND);//1000ms
    return 42;
}
void do_other_stuff() {
    TICK();
}
void test_future_async() {
    TICK();
    future<int> nAnswer_f = async(find_the_answer_to_ltuae);
    do_other_stuff();
    INFO("The answer is %d", nAnswer_f.get());
}

//Listing 4.7 Passing arguments to a function with async
void test_future_async_struct() {
    TICK();
    X x;
    future<int>     nResult1_f = async(&X::foo, &x, 42, "hello");   //Calls p->foo(42, "hello") where p is &x
    future<string>  sResult2_f = async(&X::bar, x, "goodbye");      //Calls tmpx.bar() where tmpx is a copy of x
    INFO("nResult1_f=%d", nResult1_f.get());
    INFO("sResult2_f=%s", sResult2_f.get().c_str());

    Y y;
    future<double> dResult3_f = async(Y(), 3.141);      //Calls tmpy(3.141) where tmpy is move-constructed from Y()
    future<double> dResult4_f = async(ref(y), 2.718);   //Calls y(2.718)
    INFO("dResult3_f=%f", dResult3_f.get());
    INFO("dResult4_f=%f", dResult4_f.get());

#if 0//strange usage: can`t be compiled correctly
    X z(X&);
    async(z, ref(x));   //Calls z(x)
#endif

    future<void>    vResult5_f = async(move_only());

    future<double>  dResult6_f = async(launch::async, Y(), 1.2);  //Run in new thread
    INFO("dResult6_f=%f", dResult6_f.get());

#if 0//strange usage: can`t be compiled correctly
    future<X> xResult7_f = async(launch::deferred, z, ref(x));    //Run in wait() or get()
    future<X> xResult8_f = async(launch::deferred | launch::async, z, ref(x));
    future<X> xResult9_f = async(z, ref(x));   //Implementation chooses
    xResult7_f.wait();  //Invoke deferred function
#endif
}

//Listing 4.9 Running code on a GUI thread using packaged_task
deque<packaged_task<void()>>        g_dequePackagedTasks;
mutex                               g_mutexTasks;
condition_variable                  g_cvTasks;
bool gui_shutdown_message_received() {
    TICK();
    unique_lock<mutex> ulock(g_mutexTasks);
    g_cvTasks.wait(ulock, [] {return !g_dequePackagedTasks.empty(); });
    return false;
}
void get_and_process_gui_message() {
    TICK();
}
void gui_thread() {
    TICK();
    while (!gui_shutdown_message_received()) {
        get_and_process_gui_message();
        packaged_task<void()> ptask;
        {
            lock_guard<mutex> lock(g_mutexTasks);
            if (g_dequePackagedTasks.empty()) {
                continue;
            }
            ptask = move(g_dequePackagedTasks.front());
            g_dequePackagedTasks.pop_front();
        }
        ptask();
        yield();
    }
}
template<typename Func>
future<void> post_task_for_gui_thread(Func f) {
    TICK();
    packaged_task<void()> ptask(f);
    future<void> vResult_f = ptask.get_future();
    lock_guard<mutex> lock(g_mutexTasks);
    g_dequePackagedTasks.push_back(move(ptask));
    g_cvTasks.notify_one();
    return vResult_f;
}
void do_some_task() {
    TICK();
}
int do_other_task(int const &value) {
    TICK();
    return TEN * value;
}
void test_packaged_task() {
    TICK();
    thread threadBackgroundGui(gui_thread);
    threadBackgroundGui.detach();

    while (true) {
        thread threadTask(post_task_for_gui_thread<void()>, do_some_task);
        threadTask.detach();

        common_fun::sleep(5 * HUNDRED);//500ms
    }
}

//4.2.3 Making ()promises
//Listing 4.10 Handling multiple connections from a single thread using promise
bool done(SetConnection const &connections) {
    TICK();
    common_fun::sleep(THOUSAND);
    return false;
}
void process_connections(SetConnection &connections) {
    TICK();
    while (!done(connections)) {
        for (Connection_iterator &pos = connections.cbegin(); pos != connections.cend(); ++pos) {
            Connection_ptr const& conn = *pos;
            if (conn->has_incoming_data()) {
                data_packet data = conn->incoming();
                promise<payload_type>& payload_p = conn->get_promise(data.id);
                payload_p.set_value(data.payload);
            }
            if (conn->has_outgoing_data()) {
                outgoing_packet data = conn->top_of_outgoing_queue();
                conn->send(data.payload);
                data.promise.set_value(true);
            }
        }
        yield();
    }
}
void test_process_connections() {
    TICK();
    SetConnection connections;
    connections.insert(make_shared<Connection>());
    connections.insert(make_shared<Connection>());
    connections.insert(make_shared<Connection>());
    process_connections(connections);
}

double square_root(double x) {
    TICK();
    if (x < 0) {
        throw out_of_range("x<0");
    }
    return sqrt(x);
}

void test_future_exception() {
    TICK();
    try {
        double d1 = 256;
        future<double> dSqrt1_f = async(square_root, d1);
        INFO("async square_root(%f)=%f", d1, dSqrt1_f.get());

        double d2 = -1;
        future<double> dSqrt2_f = async(square_root, d2);
        INFO("async square_root(%f)=%f", d2, dSqrt2_f.get());
    } catch (exception const& e) {
        ERR("catch exception: %s", e.what());
    }
}

promise<double>             g_dSome_p;
double calcuate_value() {
    TICK();
    throw out_of_range("calcuate_value exception.");
    return 3.141;
}
void promise_exception() {
    TICK();
    try {
        g_dSome_p.set_value(calcuate_value());
    }
    catch (exception const& e) {
        ERR("catch exception: %s", e.what());
        g_dSome_p.set_exception(current_exception());
    }
}
void test_promise_exception() {
    TICK();
    promise_exception();

    future<double> dResult_f = g_dSome_p.get_future();
    assert(dResult_f.valid());
    shared_future<double> dResult_sf(move(dResult_f));

    assert(!dResult_f.valid());
    assert(dResult_sf.valid());
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
void test_durations() {
    TICK();
    milliseconds ms(54802);
    seconds s = duration_cast<seconds>(ms);
    INFO("ms=%ld, s=%d", ms.count(), s.count());    //error: The print of s is 0

    seconds s1(54802);
    INFO("s1=%ld", s1.count()); //The print of s1 is 54802

    future<int> nResult_f = async(some_task);
    if (nResult_f.wait_for(milliseconds(35)) == future_status::ready) {
        int const &nResult = nResult_f.get();
        INFO("nResult_f=%d", nResult);
        do_something_with(nResult);
    }
}
//4.3.3 Time points
void do_something() {
    TICK();
    common_fun::sleep(5 * HUNDRED);
}

void test_time_points() {
    TICK();
    auto timepointStart = high_resolution_clock::now();
    do_something();
    auto timepoingStop  = high_resolution_clock::now();
    auto llTick         = duration_cast<milliseconds>(timepoingStop - timepointStart).count();
    INFO("do_something() took %ld milliseconds", llTick);
}

//Listing 4.11 Waiting for a condition variable with a timeout
condition_variable  g_cvWait;
bool                g_bDoneWait = false;
mutex               g_mutexWait;
unsigned const &    CV_WAIT_TIME_MS = 5 * HUNDRED;
bool wait_until_loop() {
    TICK();
    while (!g_bDoneWait) {
        WARN("wait_until_loop() ...");
        auto const timeout = steady_clock::now() + milliseconds(CV_WAIT_TIME_MS);
        unique_lock<mutex> ulock(g_mutexWait);
        if (g_cvWait.wait_until(ulock, timeout) == cv_status::timeout) {
            break;
        }
        yield();
    }
    return g_bDoneWait;
}
bool wait_for_loop() {
    TICK();
    while (!g_bDoneWait) {
        WARN("wait_for_loop() ...");
        unique_lock<mutex> ulock(g_mutexWait);
        if (g_cvWait.wait_for(ulock, milliseconds(CV_WAIT_TIME_MS)) == cv_status::timeout) {
            break;
        }
        yield();
    }
    return g_bDoneWait;
}
void test_condition_variable_timeout() {
    TICK();
    g_bDoneWait = false;
    wait_until_loop();

    g_bDoneWait = false;
    wait_for_loop();

    g_cvWait.notify_one();
    g_cvWait.notify_one();
}

//4.4 Using synchronization of operations to simplify code
//4.4.1 Funcional programming with futures
//Listing 4.12 A sequential implementation of Quicksort
const list<int> g_lstInput = {
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
void test_sequential_quick_sort() {
    TICK();
    TV_LIST_INT(g_lstInput);

    list<int> lstResult = sequential_quick_sort(g_lstInput);
    TV_LIST_INT(lstResult);
}

//Listing 4.13 Parallel Quicksort using futures
void test_parallel_quick_sort() {
    TICK();
    TV_LIST_INT(g_lstInput);

    list<int> result = parallel_quick_sort(g_lstInput);
    TV_LIST_INT(result);
}

//Listing 4.14 A sample implementation of spawn_task
double do_plus_of_3(double const& d1, double const& d2, double const& d3) {
    TICK();
    return d1 + d2 + d3;
}
double do_multiplus_work(double const &val) {
    TICK();
    return val * PI;
}
int do_mod(int const &val) {
    TICK();
    return val % 2;
}
int do_1() {
    TICK();
    return 1;
}
void test_spawn_task() {
    TICK();
#if 1
    typedef function<double(double const &)> F1;
    future<double> dResult1_f = spawn_task<F1, double>(&do_multiplus_work, 1.5);
    INFO("spawn_task<do_multiplus_work> dResult1_f=%f", dResult1_f.get());

    typedef function<int(int)> F2;
    future<int> nResult2_f = spawn_task<F2, int>(&do_mod, 19);
    INFO("spawn_task<do_mod> nResult2_f=%d", nResult2_f.get());

    typedef function<int()> F3;
    future<int> nResult3_f = spawn_task<F3>(&do_1);
    INFO("spawn_task<do_1> nResult3_f=%d", nResult3_f.get());
#else
    double const &d1 = 1, &d2 = 2.1, &d3 = -3.5;
    future<double> dResult0_f = spawn_task(&do_plus_of_3, d1, d2, d3);
    INFO("spawn_task<do_plus_of_3> result0_f=%f", dResult0_f.get());

    double const &dR = 1.5;
    future<double> dResult1_f = spawn_task(&do_multiplus_work, dR);
    INFO("spawn_task<do_multiplus_work> dResult1_f=%f", dResult1_f.get());

    int const &nA = 19;
    future<int> nResult2_f = spawn_task(&do_mod, nA);
    INFO("spawn_task<do_mod> nResult2_f=%d", nResult2_f.get());

    future<int> nResult3_f = spawn_task(&do_1);
    INFO("spawn_task<do_1> nResult3_f=%d", nResult3_f.get());
#endif

    vector<future<unsigned>> vctFutureResult;
    for (unsigned i = 0; i < 8; ++i) {
        vctFutureResult.emplace_back(spawn_task([i] {
            TICK();
            return i*i;
        }));
    }
    for (auto &&pos : vctFutureResult) {
        INFO("%d", pos.get());
    }
}

}//namespace sync_conc_opera


