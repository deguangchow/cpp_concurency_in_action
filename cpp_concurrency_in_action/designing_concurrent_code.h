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
        list<T> data;
        promise<list<T>> promise;
    };
    lock_based_conc_data::thread_safe_stack<chunk_to_sort>  m_stChunks;
    vector<thread>                                          m_vctThreads;
    unsigned const                                          m_uMaxThreadCount;
    atomic<bool>                                            m_bDone_a;
    sorter() : m_uMaxThreadCount(HARDWARE_CONCURRENCY - 1), m_bDone_a(false) {
        TICK();
        INFO("max thread count is %d.", m_uMaxThreadCount);
    }
    ~sorter() {
        TICK();
        m_bDone_a = true;

        for (unsigned i = 0; i < m_vctThreads.size(); ++i) {
            m_vctThreads[i].join();
        }
    }
    void try_sort_chunk() {
        TICK();
        shared_ptr<chunk_to_sort> chunk = m_stChunks.pop();
        if (chunk) {
            sort_chunk(chunk);
        }
    }
    list<T> do_sort(list<T>& chunk_data) {
        TICK();
        if (chunk_data.empty()) {
            return chunk_data;
        }
        list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const& tPartitionValue = *result.begin();

        auto posDivide = partition(chunk_data.begin(), chunk_data.end(),
            [&](T const& val) {
            return val < tPartitionValue;
        });

        chunk_to_sort chunkNewLower;
        chunkNewLower.data.splice(chunkNewLower.data.end(), chunk_data, chunk_data.begin(), posDivide);
        future<list<T>> lstNewLower_f = chunkNewLower.promise.get_future();
        m_stChunks.push(move(chunkNewLower));
        if (m_vctThreads.size() < m_uMaxThreadCount) {
            m_vctThreads.push_back(thread(&sorter<T>::sort_thread, this));
        }
        list<T> lstNewHigher(do_sort(chunk_data));
        result.splice(result.end(), lstNewHigher);
        while (lstNewLower_f.wait_for(seconds(0)) == future_status::timeout) {
            WARN("do_sort() loop...");
            try_sort_chunk();
        }
        result.splice(result.begin(), lstNewLower_f.get());
        return result;
    }
    void sort_chunk(shared_ptr<chunk_to_sort> const& chunk) {
        TICK();
        chunk->promise.set_value(do_sort(chunk->data));
    }
    void sort_thread() {
        TICK();
        while (!m_bDone_a) {
            DEBUG("sort_thread() loop...");
            try_sort_chunk();
            yield();
        }
    }
};
template<typename T>
list<T> parallel_quick_sort(list<T> input) {
    TICK();
    if (input.empty()) {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}
void test_parallel_quick_sort();

//8.1.3 Dividing work by task type

//8.2 Factors affecting the performance of concurrent code
//8.2.1 How many processors?
//8.2.2 Data contention and cache ping-pang
void test_processing_loop();

void test_processing_loop_with_mutex();

//8.2.3 False sharing

//8.2.4 How close is your data?
//8.2.5 Oversubscription and excessive task switching
typedef unsigned my_data;

//8.3 Designing data structures for multithreaded performance
//8.3.1 Dividing array elements for complex operations
//8.3.2 Data access patterns in other data structures
#if 1
struct protected_data {
    mutex m;
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
void test_processing_loop_protect();

//8.4 Additional considerations when designing for concurrency
//8.4.1 Exception safety in parallel algorithms
//Listing 8.2 A naive parallel version of accumulate(from listing 2.8)

//Listing 8.3 A parallel version of accumulate using packaged_task
template<typename Iterator, typename T>
struct accumulate_block {
    T operator()(Iterator first, Iterator last) {
        TICK();
        return accumulate(first, last, T());
    }
};
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const LENGTH = distance(first, last);
    if (!LENGTH) {
        return init;
    }

    unsigned long const&    MIN_PER_THREAD = 25;
    unsigned long const&    MAX_THREADS = (LENGTH + MIN_PER_THREAD - 1) / MIN_PER_THREAD;
    unsigned long const&    NUM_THREADS = min(HARDWARE_CONCURRENCY, MAX_THREADS);
    unsigned long const&    BLOCK_SIZE = LENGTH / NUM_THREADS;
    unsigned long const&    THREAD_SIZE = NUM_THREADS - 1;

    vector<future<T>>       vctFutures(THREAD_SIZE);
    vector<thread>          vctThreads(THREAD_SIZE);

    Iterator                posBlockStart = first;
    T                       tLastResult;
    try {
        for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
            Iterator posBlockEnd = posBlockStart;
            advance(posBlockEnd, BLOCK_SIZE);

            //be careful about the ()!!!
            packaged_task<T(Iterator, Iterator)> packagedTask((accumulate_block<Iterator, T>()));
            vctFutures[i] = packagedTask.get_future();
            vctThreads[i] = thread(move(packagedTask), posBlockStart, posBlockEnd);
            posBlockStart = posBlockEnd;
        }
        tLastResult = accumulate_block<Iterator, T>()(posBlockStart, last);

        for_each(vctThreads.begin(), vctThreads.end(), mem_fn(&thread::join));
    } catch (...) {
        ERR("...");
        for (unsigned i = 0; i < THREAD_SIZE; ++i) {
            if (vctThreads[i].joinable()) {
                vctThreads[i].join();
            }
        }
    }

    T result = init;
    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        result += vctFutures[i].get();
    }
    result += tLastResult;
    return result;
}
void test_parallel_accumulate();

//Listing 8.4 An exception-safe parallel version of accumulate
class join_threads {
    vector<thread>& threads;
public:
    explicit join_threads(vector<thread>& threads_) : threads(threads_) {}
    ~join_threads() {
        TICK();
        for (unsigned i = 0; i < threads.size(); ++i) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
};
template<typename Iterator, typename T>
T parallel_accumulate_join(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const LENGTH = distance(first, last);
    if (!LENGTH) {
        return init;
    }

    unsigned long const&    MIN_PER_THREAD = 25;
    unsigned long const&    MAX_THREADS = (LENGTH + MIN_PER_THREAD - 1) / MIN_PER_THREAD;
    unsigned long const&    NUM_THREADS = min(HARDWARE_CONCURRENCY, MAX_THREADS);
    unsigned long const&    BLOCK_SIZE = LENGTH / NUM_THREADS;
    unsigned long const&    THREAD_SIZE = NUM_THREADS - 1;

    vector<future<T>>       vctFutures(THREAD_SIZE);
    vector<thread>          vctThreads(THREAD_SIZE);
    join_threads            threadsJoiner(vctThreads);

    Iterator posBlockStart = first;

    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        Iterator block_end = posBlockStart;
        advance(block_end, BLOCK_SIZE);
        //be careful about the ()!!!
        packaged_task<T(Iterator, Iterator)> task((accumulate_block<Iterator, T>()));
        vctFutures[i] = task.get_future();
        vctThreads[i] = thread(move(task), posBlockStart, block_end);
        posBlockStart = block_end;
    }
    T tLastResult = accumulate_block<Iterator, T>()(posBlockStart, last);

    for_each(vctThreads.begin(), vctThreads.end(), mem_fn(&thread::join));

    T result = init;
    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        result += vctFutures[i].get();
    }
    result += tLastResult;
    return result;
}
void test_parallel_accumulate_join();

//Listing 8.5 An exception-safe parallel version of accumulate using async
template<typename Iterator, typename T>
T parallel_accumulate_async(Iterator first, Iterator last, T init) {
    TICK();
    unsigned long const LENGTH = distance(first, last);
    unsigned long const MAX_CHUNK_SIZE = 25;
    if (LENGTH <= MAX_CHUNK_SIZE) {
        return accumulate(first, last, init);
    } else {
        Iterator posMid = first;
        advance(posMid, LENGTH / 2);
        future<T> tHalfResult_f = async(&parallel_accumulate_async<Iterator, T>, first, posMid, init);
        T tSecondHalfResult = parallel_accumulate_async(posMid, last, T());

        auto const& tHalfResult = tHalfResult_f.get();
        INFO("{%d, %d}", tHalfResult, tSecondHalfResult);
        return tHalfResult + tSecondHalfResult;
    }
}
void test_parallel_accumulate_async();

//8.4.2 Scalability and Amdahl`s law

//8.4.3 Hiding latenct with multiple threads

//8.4.4 Improving responsiveness with concurrency
//Listing 8.6 Separating GUI thread from task thread
struct event_data {
    enum event_type {
        start_task,
        stop_task,
        task_complete,
        quit
    };
    event_type type;
    event_data() : type(start_task) {}
};
event_data get_event();
void process(event_data const& event);
void gui_thread();
bool task_complete();
void do_next_operation();
void perform_cleanup();
void post_gui_event(event_data::event_type const& type);
void task();

//Listing 8.7 A parallel version of for_each
template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func&& f) {
    TICK();
    unsigned long const LENGTH = distance(first, last);
    if (!LENGTH) {
        return;
    }

    unsigned long const&    MIN_PER_THREAD = 25;
    unsigned long const&    MAX_THREADS = (LENGTH + MIN_PER_THREAD - 1) / MIN_PER_THREAD;
    unsigned long const&    NUM_THREADS = min(HARDWARE_CONCURRENCY, MAX_THREADS);
    unsigned long const&    BLOCK_SIZE = LENGTH / NUM_THREADS;
    unsigned long const&    THREAD_SIZE = NUM_THREADS - 1;

    vector<future<void>>    vctFutures(THREAD_SIZE);
    vector<thread>          vctThreads(THREAD_SIZE);
    join_threads            threadsJoiner(vctThreads);

    Iterator                posBlockStart = first;
    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        Iterator posBlockEnd = posBlockStart;
        advance(posBlockEnd, BLOCK_SIZE);
        packaged_task<void(void)> packagedTask([=]() {
            for_each(posBlockStart, posBlockEnd, f);
        });
        vctFutures[i] = packagedTask.get_future();
        vctThreads[i] = thread(move(packagedTask));
        posBlockStart = posBlockEnd;
    }

    for_each(posBlockStart, last, f);

    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        vctFutures[i].get();
    }
}
void test_parallel_for_each();

//Listing 8.8 A parallel version of for_each using async
template<typename Iterator, typename Func>
void parallel_for_each_async(Iterator first, Iterator last, Func f) {
    TICK();
    unsigned long const& LENGTH = distance(first, last);
    if (!LENGTH) {
        return;
    }
    unsigned long const& MIN_PER_THREAD = 25;
    if (LENGTH < (2 * MIN_PER_THREAD)) {
        for_each(first, last, f);
    } else {
        Iterator const& posMid      = first + LENGTH / 2;
        future<void> firstHalf_f    = async(&parallel_for_each_async<Iterator, Func>, first, posMid, f);
        parallel_for_each_async(posMid, last, f);
        firstHalf_f.get();
    }
}
void test_parallel_for_each_async();

//8.5.2 A parallel implementation of find
//Listing 8.9 An implementation of a parallel find algorithm
template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match) {
    TICK();
    struct find_element {
        void operator()(Iterator begin, Iterator end, MatchType match,
            promise<Iterator>* result, atomic<bool>* done_flag) {
            TICK();
            try {
                for (; (begin != end) && !done_flag->load(); ++begin) {
                    if (*begin == match) {
                        result->set_value(begin);
                        done_flag->store(true);
                        DEBUG("bingo");
                        return;
                    }
                }
            } catch (...) {
                try {
                    result->set_exception(current_exception());
                    done_flag->store(true);
                } catch (...) {
                }
            }
        }
    };

    unsigned long const LENGTH = distance(first, last);
    if (!LENGTH) {
        return last;
    }

    unsigned long const&    MIN_PER_THREAD = 25;
    unsigned long const&    MAX_THREADS = (LENGTH + MIN_PER_THREAD - 1) / MIN_PER_THREAD;
    unsigned long const&    NUM_THREADS = min(HARDWARE_CONCURRENCY, MAX_THREADS);
    unsigned long const&    BLOCK_SIZE = LENGTH / NUM_THREADS;
    unsigned long const&    THREAD_SIZE = NUM_THREADS - 1;

    promise<Iterator>       posResult_p;
    atomic<bool>            bDone_a(false);
    vector<thread>          vctThreads(THREAD_SIZE);
    {
        join_threads        threadsJoiner(vctThreads);
        Iterator            posBlockStart = first;
        for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
            Iterator block_end = posBlockStart;
            advance(block_end, BLOCK_SIZE);
            vctThreads[i] = thread(find_element(), posBlockStart, block_end, match, &posResult_p, &bDone_a);
            posBlockStart = block_end;
        }
        find_element()(posBlockStart, last, match, &posResult_p, &bDone_a);
    }
    if (!bDone_a.load()) {
        return last;
    }
    return posResult_p.get_future().get();
}
void test_parallel_find();

//Listing 8.10 An implementation of a parallel find algorithm using async
template<typename Iterator, typename MatchType>
Iterator parallel_find_async(Iterator first, Iterator last, MatchType match, atomic<bool>& done) {
    if (done.load()) {
        return first;
    }
    TICK();
    try {
        unsigned long const&    LENGTH = distance(first, last);
        unsigned long const&    MIN_PER_THREAD = 25;
        if (LENGTH < (2 * MIN_PER_THREAD)) {
            for (; (first != last) && !done.load(); ++first) {
                if (*first == match) {
                    DEBUG("bingo");
                    done = true;
                    return first;
                }
            }
            return last;
        } else {
            Iterator const&     posMid = first + (LENGTH / 2);
            future<Iterator>    posAsyncResult_f = async(&parallel_find_async<Iterator, MatchType>,
                posMid, last, match, ref(done));
            Iterator const&     posDirectResult = parallel_find_async(first, posMid, match, done);
            return (posDirectResult == posMid) ? posAsyncResult_f.get() : posDirectResult;
        }
    } catch (...) {
        ERR("...");
        done = true;
        throw;
    }
}
void test_parallel_find_async();

//8.5.3 A parallel implementation of partial_sum
//Listing 8.11 Calculating partial sums in parallel by dividing the problem
template<typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last) {
    TICK();
    typedef typename Iterator::value_type value_type;
    struct process_chunk {
        void operator()(Iterator begin, Iterator last,
            future<value_type>* previous_end_value, promise<value_type>* end_value) {
            TICK();
            try {
                Iterator end = last;
                ++end;
                partial_sum(begin, end, begin);
                if (previous_end_value) {
                    value_type const& addend = previous_end_value->get();
                    *last += addend;
                    if (end_value) {
                        end_value->set_value(*last);
                    }
                    for_each(begin, last, [addend](value_type& item) {item += addend; });
                } else {
                    end_value->set_value(*last);
                }
            } catch (...) {
                if (end_value) {
                    end_value->set_exception(current_exception());
                } else {
                    throw;
                }
            }
        }
    };

    unsigned long const&        LENGTH = distance(first, last);
    if (!LENGTH) {
        return;
    }

    unsigned long const&        MIN_PER_THREAD  = 25;
    unsigned long const&        MAX_THREADS     = (LENGTH + MIN_PER_THREAD - 1) / MIN_PER_THREAD;
    unsigned long const&        NUM_THREADS     = min(HARDWARE_CONCURRENCY, MAX_THREADS);
    unsigned long const&        BLOCK_SIZE      = LENGTH / NUM_THREADS;
    unsigned long const&        THREAD_SIZE     = NUM_THREADS - 1;

    typedef typename Iterator::value_type value_type;
    vector<thread>              vctThreads(THREAD_SIZE);
    vector<promise<value_type>> vctPromiseValues(THREAD_SIZE);
    vector<future<value_type>>  vctFuturesValues;
    vctFuturesValues.reserve(THREAD_SIZE);

    join_threads                threadsJoiner(vctThreads);
    Iterator                    posBlockStart = first;
    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        Iterator block_last = posBlockStart;
        advance(block_last, BLOCK_SIZE);
        vctThreads[i] = thread(process_chunk(), posBlockStart, block_last,
            (i != 0) ? &vctFuturesValues[i - 1] : 0, &vctPromiseValues[i]);
        posBlockStart = block_last;
        ++posBlockStart;
        vctFuturesValues.push_back(vctPromiseValues[i].get_future());
    }
    Iterator posFinal = posBlockStart;
    advance(posFinal, distance(posBlockStart, last) - 1);
    process_chunk()(posBlockStart, posFinal,
        (NUM_THREADS > 1) ? &vctFuturesValues.back() : 0, 0);
}
void test_parallel_partial_sum();

#if 0
//Listing 8.12 A simple barrier class
class barrier {
    unsigned const count;
    atomic<unsigned> spaces;
    atomic<unsigned> generation;
public:
    explicit barrier(unsigned count_) : count(count_), spaces(count), generation(0) {}
    void wait() {
        TICK();
        unsigned const my_generation = generation;
        if (!--spaces) {
            spaces = count;
            ++generation;
        } else {
            while (generation == my_generation) {
                this_thread::yield();
            }
        }
    }
};
#else
//Listing 8.13 A parallel implementation of partial_sum by pairwise updates
struct barrier {
    atomic<unsigned> uCount_a;
    atomic<unsigned> uSpaces_a;
    atomic<unsigned> uGeneration_a;
    explicit barrier(unsigned count_) : uCount_a(count_), uSpaces_a(count_), uGeneration_a(0) {}
    void wait() {
        TICK();
        unsigned const uGeneration = uGeneration_a.load();
        if (!--uSpaces_a) {
            uSpaces_a = uCount_a.load();
            ++uGeneration_a;
        } else {
            while (uGeneration_a.load() == uGeneration) {
                yield();
            }
        }
    }
    void done_waiting() {
        TICK();
        --uCount_a;
        if (!--uSpaces_a) {
            uSpaces_a = uCount_a.load();
            ++uGeneration_a;
        }
    }
};
#endif
template<typename Iterator>
void parallel_partial_sum_pairwise(Iterator first, Iterator last) {
    TICK();
    typedef typename Iterator::value_type value_type;
    struct process_element {
        void operator()(Iterator first, Iterator last, vector<value_type>& buffer, unsigned i, barrier& b) {
            TICK();
            value_type& ith_element = *(first + i);
            bool update_source = false;

            for (unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2) {
                value_type const& source = (step % 2) ? buffer[i] : ith_element;
                value_type &dest = (step % 2) ? ith_element : buffer[i];
                value_type const& addend = (step % 2) ? buffer[i - stride] : *(first + i - stride);

                dest = source + addend;
                update_source = !(step % 2);
                b.wait();
            }
            if (update_source) {
                ith_element = buffer[i];
            }
            b.done_waiting();
        }
    };
    unsigned long const&    LENGTH = distance(first, last);
    if (LENGTH <= 1) {
        return;
    }
    unsigned long const&    THREAD_SIZE = LENGTH - 1;
    vector<value_type>      vctBuffer(LENGTH);
    barrier                 bar(LENGTH);

    vector<thread>          vctThreads(THREAD_SIZE);
    join_threads            threadsJoiner(vctThreads);

    Iterator                posBlockStart = first;
    for (unsigned long i = 0; i < THREAD_SIZE; ++i) {
        vctThreads[i] = thread(process_element(), first, last, ref(vctBuffer), i, ref(bar));
    }
    process_element()(first, last, vctBuffer, THREAD_SIZE, bar);
}
void test_parallel_partial_sum_pairwise();

}//namespace design_conc_code

#endif  //DESIGNING_CONCURRENT_CODE_H

