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

}//namespace design_conc_code

#endif  //DESIGNING_CONCURRENT_CODE_H

