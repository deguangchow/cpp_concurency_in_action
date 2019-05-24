///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter8: Designing concurrent code
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/20
#include "stdafx.h"
#include "designing_concurrent_code.h"

namespace design_conc_code {

//8.1.1 Dividing data between threads before processing begins
//8.1.2 Dividing date recursively
//Listing 8.1 Parallel Quicksort using a stack of pending chunks to sort
void test_parallel_quick_sort() {
    TICK();

    list<unsigned> lstResult = design_conc_code::parallel_quick_sort<unsigned>(LST_NUMBERS);
    for (auto const &pos : lstResult) {
        cout << pos << ", ";
    }
    cout << endl;
}

//8.1.3 Dividing work by task type

//8.2 Factors affecting the performance of concurrent code
//8.2.1 How many processors?
//8.2.2 Data contention and cache ping-pang
void do_something() {
    //TICK();
}
atomic<unsigned long> g_ulCounter_a(0);
void processing_loop() {
    TICK();
    while (g_ulCounter_a.fetch_add(1, memory_order::memory_order_relaxed) < 1000) {
        INFO("counter=%d", g_ulCounter_a.load(memory_order::memory_order_relaxed));
        do_something();
        yield();
    }
}
void test_processing_loop() {
    TICK();
    vector<thread> vctThreads(HARDWARE_CONCURRENCY);
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreads[i] = thread(&processing_loop);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreads[i].join();
    }
}

mutex g_mutex;
my_data g_myData;
bool done_processing(my_data const& data) {
    TICK();
    return true;
}
void processing_loop_with_mutex() {
    TICK();
    while (true) {
        WARN("loop...");
        lock_guard<mutex> lock(g_mutex);
        if (done_processing(g_myData)) {
            break;
        }
    }
}
void test_processing_loop_with_mutex() {
    TICK();
    vector<thread> vctThreads(HARDWARE_CONCURRENCY);
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreads[i] = thread(&processing_loop_with_mutex);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vctThreads[i].join();
    }
}

//8.3 Designing data structures for multithreaded performance
//8.3.1 Dividing array elements for complex operations
//8.3.2 Data access patterns in other data structures
protected_data p_data;
bool done_processing(protected_data const& data) {
    TICK();
    return true;
}
void processing_loop_protected_data() {
    TICK();
    while (true) {
        WARN("loop...");
        lock_guard<mutex> lock(p_data.m);
        if (done_processing(p_data)) {
            break;
        }
    }
}
void test_processing_loop_protect() {
    TICK();
    vector<thread> vctThreads(HARDWARE_CONCURRENCY);
#if 0
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vct_pro[i] = thread(&processing_loop_protect);
    }
    for (unsigned i = 0; i < HARDWARE_CONCURRENCY; ++i) {
        vct_pro[i].join();
    }
#else
    for_each(vctThreads.begin(), vctThreads.end(), [](thread &t) {
        t = thread(&processing_loop_protected_data);
    });
    for_each(vctThreads.begin(), vctThreads.end(), mem_fn(&thread::join));
#endif
}

//Listing 8.3 A parallel version of accumulate using packaged_task
void test_parallel_accumulate() {
    TICK();
    unsigned uResult = parallel_accumulate(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), 0);
    INFO("parallel_accumulate()=%d", uResult);
}

//Listing 8.4 An exception-safe parallel version of accumulate
void test_parallel_accumulate_join() {
    TICK();
    unsigned uResult = parallel_accumulate_join(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), 0);
    INFO("parallel_accumulate_join()=%d", uResult);
}

//Listing 8.5 An exception-safe parallel version of accumulate using async
void test_parallel_accumulate_async() {
    TICK();
    unsigned uResult = parallel_accumulate_async(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), 0);
    INFO("parallel_accumulate_async()=%d", uResult);
}

//8.4.4 Improving responsiveness with concurrency
//Listing 8.6 Separating GUI thread from task thread
thread task_thread;
atomic<bool> task_canceled(false);
design_conc_code::event_data get_event() {
    TICK();
    return event_data();
}
void display_results() {
    TICK();
}
void process(event_data const& event) {
    TICK();
    switch (event.type) {
    case event_data::start_task:
        task_canceled = false;
        task_thread = thread(task);
        break;
    case event_data::stop_task:
        task_canceled = true;
        task_thread.join();
        break;
    case event_data::task_complete:
        task_thread.join();
        display_results();
        break;
    default:
        break;
    }
}
void gui_thread() {
    TICK();
    while (true) {
        event_data event = get_event();
        if (event.type == event_data::quit) {
            break;
        }
        process(event);
    }
}
bool task_complete() {
    TICK();
    return true;
}
void do_next_operation() {
    TICK();
}
void perform_cleanup() {
    TICK();
}
void post_gui_event(event_data::event_type const& type) {
    TICK();
}
void task() {
    TICK();
    while (!task_complete() && !task_canceled) {
        do_next_operation();
    }
    if (task_canceled) {
        perform_cleanup();
    } else {
        post_gui_event(event_data::task_complete);
    }
}

//Listing 8.7 A parallel version of for_each
void test_parallel_for_each() {
    TICK();
    parallel_for_each(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), [](unsigned const& val) {
        DEBUG("%d", val);
        yield();
    });
}

//Listing 8.8 A parallel version of for_each using async
void test_parallel_for_each_async() {
    TICK();
    parallel_for_each_async(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), [](unsigned const& val) {
        DEBUG("%d", val);
        yield();
    });
}

//8.5.2 A parallel implementation of find
//Listing 8.9 An implementation of a parallel find algorithm
void test_parallel_find() {
    TICK();
    unsigned uMatch = 8;
#if 1
    uMatch = 88;
#endif
    auto const& posFind = parallel_find(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), uMatch);
    if (posFind != VCT_NUMBERS.end()) {
        INFO("find");
    } else {
        ERR("not find");
    }
}

//Listing 8.10 An implementation of a parallel find algorithm using async
void test_parallel_find_async() {
    TICK();
    unsigned uMatch = 8;
#if 0
    uMatch = 88;
#endif
    atomic<bool> bDone_a(false);
    auto const& posFind = parallel_find_async(VCT_NUMBERS.begin(), VCT_NUMBERS.end(), uMatch, bDone_a);
    if (posFind != VCT_NUMBERS.end()) {
        INFO("find");
    } else {
        ERR("not find");
    }
}

//8.5.3 A parallel implementation of partial_sum
//Listing 8.11 Calculating partial sums in parallel by dividing the problem
void test_parallel_partial_sum() {
    TICK();
    vector<unsigned> vctInput = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };
    parallel_partial_sum(vctInput.begin(), vctInput.end());
    for (unsigned long i = 0; i < vctInput.size(); ++i) {
        INFO("no.%3d : %5d", i + 1, vctInput[i]);
    }
}

//Listing 8.12 A simple barrier class
//Listing 8.13 A parallel implementation of partial_sum by pairwise updates
void test_parallel_partial_sum_pairwise() {
    TICK();
    vector<unsigned> vctInput = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };
    parallel_partial_sum_pairwise(vctInput.begin(), vctInput.end());
    for (unsigned long i = 0; i < vctInput.size(); ++i) {
        INFO("no.%3d : %5d", i + 1, vctInput[i]);
    }
}

}//namespace design_conc_code

