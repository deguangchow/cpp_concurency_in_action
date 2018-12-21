///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter8: Designing concurrent code
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/20
#pragma once
#ifndef DESIGNING_CONCURRENT_CODE_H
#define DESIGNING_CONCURRENT_CODE_H

#include "lock_based_concurrent_data_structures.h"
#include "lock_free_concurrent_data_structures.h"

namespace design_conc_code {

//8.1.1 Dividing data between threads before processing begins
//8.1.2 Dividing date recursively
//Listing 8.1 Parallel Quicksort using a stack of pending chunks to sort
template<typename T>
struct sorter {
    struct chunk_to_sort {
        std::list<T> data;
        std::promise<std::list<T>> promise;
    };
    lock_based_conc_data::thread_safe_stack<chunk_to_sort> chunks;
    std::vector<std::thread> threads;
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;
    sorter() : max_thread_count(std::thread::hardware_concurrency() - 1), end_of_data(false) {
        TICK();
        INFO("max_thread_count=%d", max_thread_count);
    }
    ~sorter() {
        TICK();
        end_of_data = true;

        for (unsigned i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
    }
    void try_sort_chunk() {
        TICK();
        std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
        if (chunk) {
            sort_chunk(chunk);
        }
    }
    std::list<T> do_sort(std::list<T>& chunk_data) {
        TICK();
        if (chunk_data.empty()) {
            return chunk_data;
        }
        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const& partition_val = *result.begin();

        typename std::list<T>::iterator divide_point = std::partition(chunk_data.begin(), chunk_data.end(),
            [&](T const& val) {
            INFO("compare %d, %d", val, partition_val);
            return val < partition_val;
        });

        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk_data, chunk_data.begin(), divide_point);
        std::future<std::list<T>> new_lower = new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk));
        if (threads.size() < max_thread_count) {
            threads.push_back(std::thread(&sorter<T>::sort_thread, this));
        }
        std::list<T> new_higher(do_sort(chunk_data));
        result.splice(result.end(), new_higher);
        while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            WARN("do_sort() loop");
            try_sort_chunk();
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }
    void sort_chunk(std::shared_ptr<chunk_to_sort> const& chunk) {
        TICK();
        chunk->promise.set_value(do_sort(chunk->data));
    }
    void sort_thread() {
        TICK();
        while (!end_of_data) {
            WARN("sort_thread() loop");
            try_sort_chunk();
            std::this_thread::yield();
        }
    }
};
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
    TICK();
    if (input.empty()) {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}
void parallel_quick_sort_test();

//8.1.3 Dividing work by task type

//8.2 Factors affecting the performance of concurrent code
//8.2.1 How many processors?
//8.2.2 Data contention and cache ping-pang
void processing_loop();
void processing_loop_test();

void processing_loop_with_mutex();
void processing_loop_with_mutex_test();

//8.2.3 False sharing

//8.2.4 How close is your data?
//8.2.5 Oversubscription and excessive task switching
typedef unsigned my_data;

//8.3 Designing data structures for multithreaded performance
//8.3.1 Dividing array elements for complex operations
//8.3.2 Data access patterns in other data structures
#if 1
struct protected_data {
    std::mutex m;
    char padding[CACHE_LINE];//65536 bytes is orders of magnitude larger than a cache line
    my_data data_to_protect;
};
#else
typedef unsigned data_item1;
typedef unsigned data_item2;
struct my_data {
    data_item1 d1;
    data_item2 d2;
    char padding[CACHE_LINE];
};
my_data some_array[256];
#endif
void processing_loop_protect();
void processing_loop_protect_test();

//8.4 Additional considerations when designing for concurrency
//8.4.1 Exception safety in parallel algorithms
//Listing 8.2 A naive parallel version of std::accumulate(from listing 2.8)

//Listing 8.3 A parallel version of std::accumulate using std::packaged_task
template<typename Iterator, typename T>
struct accumulate_block {
    T operator()(Iterator first, Iterator last) {
        TICK();
        return std::accumulate(first, last, T());
    }
};
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const length = std::distance(first, last);
    if (!length) {
        return init;
    }
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::vector<std::future<T>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    T last_result;
    try {
        for (unsigned long i = 0; i < (num_threads - 1); ++i) {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            //be careful about the ()!!!
            std::packaged_task<T(Iterator, Iterator)> task((accumulate_block<Iterator, T>()));
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task), block_start, block_end);
            block_start = block_end;
        }
        last_result = accumulate_block<Iterator, T>()(block_start, last);

        std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    }
    catch (...) {
        for (unsigned i = 0; i < (num_threads - 1); ++i) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
    }

    T result = init;
    for (unsigned long i = 0; i < (num_threads - 1); ++i) {
        result += futures[i].get();
    }
    result += last_result;
    return result;
}
void parallel_accumulate_test();

}//namespace design_conc_code

#endif  //DESIGNING_CONCURRENT_CODE_H

