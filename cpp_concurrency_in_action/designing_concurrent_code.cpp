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
void parallel_quick_sort_test() {
    TICK();
    std::list<unsigned> lst_input{
#if 0
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
#else
        2, 1
#endif
    };
    std::list<unsigned> lst_result = parallel_quick_sort<unsigned>(lst_input);
    for (auto const &pos : lst_result) {
        std::cout << pos << ", ";
    }
    std::cout << std::endl;
}

//8.1.3 Dividing work by task type

//8.2 Factors affecting the performance of concurrent code
//8.2.1 How many processors?
//8.2.2 Data contention and cache ping-pang
void do_something() {
    //TICK();
}
std::atomic<unsigned long> counter(0);
void processing_loop() {
    TICK();
    while (counter.fetch_add(1, std::memory_order_relaxed) < 10000000) {
        INFO("%d", counter.load(std::memory_order_relaxed));
        do_something();
    }
}
void processing_loop_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    std::vector<std::thread> vct_pro(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i] = std::thread(&processing_loop);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i].join();
    }
}

std::mutex m;
typedef unsigned my_data;
my_data data;
bool done_processing(my_data const& data) {
    TICK();
    return true;
}
void processing_loop_with_mutex() {
    TICK();
    while (true) {
        WARN("processing_loop_with_mutex() loop");
        std::lock_guard<std::mutex> lock(m);
        if (done_processing(data)) {
            break;
        }
    }
}
void processing_loop_with_mutex_test() {
    TICK();
    unsigned const& THREAD_NUMS = 5;
    std::vector<std::thread> vct_pro(THREAD_NUMS);
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i] = std::thread(&processing_loop_with_mutex);
    }
    for (unsigned i = 0; i < THREAD_NUMS; ++i) {
        vct_pro[i].join();
    }
}

}//namespace design_conc_code

