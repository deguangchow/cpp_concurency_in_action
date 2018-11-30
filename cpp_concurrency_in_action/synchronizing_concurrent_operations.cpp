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

}//namespace synchronizing_concurrent_operations


