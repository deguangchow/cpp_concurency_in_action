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

mutex m;
my_data data;
bool done_processing(my_data const& data) {
    TICK();
    return true;
}
void processing_loop_with_mutex() {
    TICK();
    while (true) {
        WARN("processing_loop_with_mutex() loop");
        lock_guard<mutex> lock(m);
        if (done_processing(data)) {
            break;
        }
    }
}
void processing_loop_with_mutex_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    vector<thread> vct_pro(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i] = thread(&processing_loop_with_mutex);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i].join();
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
void processing_loop_protect() {
    TICK();
    while (true) {
        WARN("processing_loop_protect_test() loop");
        lock_guard<mutex> lock(p_data.m);
        if (done_processing(p_data)) {
            break;
        }
    }
}
void processing_loop_protect_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    vector<thread> vct_pro(THREAD_NUMS);
#if 0
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i] = thread(&processing_loop_protect);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i].join();
    }
#else
    for_each(vct_pro.begin(), vct_pro.end(), [](thread &t) {t = thread(&processing_loop_protect); });
    for_each(vct_pro.begin(), vct_pro.end(), mem_fn(&thread::join));
#endif
}

//Listing 8.3 A parallel version of accumulate using packaged_task
void parallel_accumulate_test() {
    TICK();
    vector<unsigned> vct {
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
    unsigned ret = parallel_accumulate(vct.begin(), vct.end(), 0);
    INFO("ret=%d\r\n", ret);
}

//Listing 8.4 An exception-safe parallel version of accumulate
void parallel_accumulate_join_test() {
    TICK();
    vector<unsigned> vct {
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
    unsigned ret = parallel_accumulate_join(vct.begin(), vct.end(), 0);
    INFO("ret=%d\r\n", ret);
}

//Listing 8.5 An exception-safe parallel version of accumulate using async
void parallel_accumulate_async_test() {
    TICK();
    vector<unsigned> vct {
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
    unsigned ret = parallel_accumulate_async(vct.begin(), vct.end(), 0);
    INFO("ret=%d\r\n", ret);
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
void parallel_for_each_test() {
    TICK();
    vector<unsigned> vct {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
    };
    parallel_for_each(vct.begin(), vct.end(), [](unsigned const& val) {INFO("%d", val); });
}

//Listing 8.8 A parallel version of for_each using async
void parallel_for_each_async_test() {
    TICK();

    vector<unsigned> vct {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
    };
    parallel_for_each_async(vct.begin(), vct.end(), [](unsigned const& val) {INFO("%d", val); });
}

//8.5.2 A parallel implementation of find
//Listing 8.9 An implementation of a parallel find algorithm
void parallel_find_test() {
    TICK();
    vector<unsigned> vct {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
    };
    unsigned match = 8;
#if 0
    match = 88;
#endif
    vector<unsigned>::iterator pos = parallel_find(vct.begin(), vct.end(), match);
    if (pos != vct.end()) {
        INFO("find");
    } else {
        INFO("not find");
    }
}

//Listing 8.10 An implementation of a parallel find algorithm using async
void parallel_find_async_test() {
    TICK();
    vector<unsigned> vct {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
    };
    unsigned match = 8;
#if 0
    match = 88;
#endif
    atomic<bool> done(false);
    vector<unsigned>::iterator pos = parallel_find_async(vct.begin(), vct.end(), match, done);
    if (pos != vct.end()) {
        INFO("find");
    } else {
        INFO("not find");
    }
}

//8.5.3 A parallel implementation of partial_sum
//Listing 8.11 Calculating partial sums in parallel by dividing the problem
void parallel_partial_sum_test() {
    TICK();
    vector<unsigned> vct = {
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
    parallel_partial_sum(vct.begin(), vct.end());
    for (unsigned long i = 0; i < vct.size(); ++i) {
        INFO("%d=%d", i, vct[i]);
    }
}

//Listing 8.12 A simple barrier class
//Listing 8.13 A parallel implementation of partial_sum by pairwise updates
void parallel_partial_sum_pairwise_test() {
    TICK();
    vector<unsigned> vct = {
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
    parallel_partial_sum_pairwise(vct.begin(), vct.end());
    for (unsigned long i = 0; i < vct.size(); ++i) {
        INFO("%d=%d", i, vct[i]);
    }
}

}//namespace design_conc_code

