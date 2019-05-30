///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief The code of the book.
///    <<C++ Concurrency In Action>>
///    author: Anthony Williams
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#include "stdafx.h"
#include "thread_test.h"
#include "thread_manage.h"
#include "thread_sharing_data.h"
#include "synchronizing_concurrent_operations.h"
#include "atomic_memery_model_operations.h"
#include "lock_based_concurrent_data_structures.h"
#include "lock_free_concurrent_data_structures.h"
#include "designing_concurrent_code.h"
#include "advanced_thread_management.h"
#include "test_debug_mulitithreaded_app.h"
#include "messaging.h"

int main() {
    TICK();

#if 0//chapter1
    thread_test::hello_test();
    thread_test::hello_concurrency();
#endif

#if 0//chapter2
    thread_manage::launching_thread_test();
    thread_manage::oops();
    thread_manage::f_oops_exception();
    thread_manage::f_thread_guard();
    thread_manage::edit_document_test();
    thread_manage::oops(42);
    thread_manage::not_oops(42);
    thread_manage::oops_again(42);
    thread_manage::x_test();
    thread_manage::x1_test();
    thread_manage::move_test();
    thread_manage::thread_move_test();
    thread_manage::g_test();
    thread_manage::g_1();
    thread_manage::scopt_thread_test();
    thread_manage::f_spawn_threads();
    thread_manage::parallel_accumulate_test();
    thread_manage::identifying_threads_test();
#endif

#if 0//chapter3
    thread_sharing_data::test_foo();
    thread_sharing_data::test_stack();
    thread_sharing_data::test_thread_safe_stack();
    thread_sharing_data::test_std_lock();
    thread_sharing_data::test_hierarchical_mutex();
    thread_sharing_data::test_std_lock_ex();
    thread_sharing_data::test_process_data();
    thread_sharing_data::test_get_and_process_data();
    thread_sharing_data::test_compare_operator();
    thread_sharing_data::test_RAII();
    thread_sharing_data::test_RAII_lock();
    thread_sharing_data::test_call_once();
    thread_sharing_data::test_connection_call_once();
    thread_sharing_data::test_connection_concurrency_call_once();
#endif

#if 0//chapter4
    sync_conc_opera::test_wait_for_flag();
    sync_conc_opera::test_wait_for_condition_variable();
    sync_conc_opera::test_threadsafe_queue();
    sync_conc_opera::test_future_async();
    sync_conc_opera::test_future_async_struct();
    sync_conc_opera::test_packaged_task();
    sync_conc_opera::test_process_connections();
    sync_conc_opera::test_future_exception();
    sync_conc_opera::test_promise_exception();
    sync_conc_opera::test_durations();
    sync_conc_opera::test_time_points();
    sync_conc_opera::test_condition_variable_timeout();
    sync_conc_opera::test_sequential_quick_sort();
    sync_conc_opera::test_parallel_quick_sort();
    sync_conc_opera::test_spawn_task();
#endif

#if 0//chapter5
    atomic_type::test_atomic_flag();
    atomic_type::test_spinlock_mutex();
    atomic_type::test_atomic_bool();
    atomic_type::test_compare_exchange_weak();
    atomic_type::test_compare_exchange_weak_memory_order();
    atomic_type::test_atomic_pointer();
    atomic_type::test_atomic_load_store();
    atomic_type::test_atomic_sync_from_thread();
    atomic_type::test_call_unordered();
    atomic_type::test_sequential_consistency();
    atomic_type::test_relaxed();
    atomic_type::test_relaxed_multi_thread();
    atomic_type::test_acquire_release();
    atomic_type::test_acquire_release_relaxed();
    atomic_type::test_transitive_sync_acquire_release();
    atomic_type::test_consume();
    atomic_type::test_consume_queue();
    atomic_type::test_fences();
    atomic_type::test_nonatomic();
#endif

#if 0//chapter6
    lock_based_conc_data::test_lock_based_thread_safe_stack();
    lock_based_conc_data::test_threadsafe_queue();
    lock_based_conc_data::test_threadsafe_queue_shared_ptr();
    lock_based_conc_data::test_queue();
    lock_based_conc_data::test_dummy_queue();
    lock_based_conc_data::test_threadsafe_queue_fine_grained();
    lock_based_conc_data::test_threadsafe_waiting_queue();
    lock_based_conc_data::test_threadsafe_lookup_table();
    lock_based_conc_data::test_threadsafe_list();
#endif

#if 0//chapter7
    lock_free_conc_data::test_spinlock_mutex();
    lock_free_conc_data::test_lock_free_stack();
    lock_free_conc_data::test_lock_free_shared_ptr_stack();
    lock_free_conc_data::test_lock_free_reclaim_stack();
    lock_free_conc_data::test_lock_free_shared_stack();
    lock_free_conc_data::test_lock_free_split_ref_cnt_stack();
    lock_free_conc_data::test_lock_free_memory_split_ref_cnt_stack();
    lock_free_conc_data::test_lock_free_queue();
#endif

#if 0//chapter8
    design_conc_code::test_parallel_quick_sort();
    design_conc_code::test_processing_loop();
    design_conc_code::test_processing_loop_with_mutex();
    design_conc_code::test_processing_loop_protect();
    design_conc_code::test_parallel_accumulate();
    design_conc_code::test_parallel_accumulate_join();
    design_conc_code::test_parallel_accumulate_async();
    design_conc_code::test_parallel_for_each();
    design_conc_code::test_parallel_for_each_async();
    design_conc_code::test_parallel_find();
    design_conc_code::test_parallel_find_async();
    design_conc_code::test_parallel_partial_sum();
    design_conc_code::test_parallel_partial_sum_pairwise();
#endif

#if 0//chapter9
    adv_thread_mg::test_simple_thread_pool();

    adv_thread_mg::test_thread_pool<adv_thread_mg::thread_pool>();
    adv_thread_mg::test_parallel_accumulate<adv_thread_mg::thread_pool>();
    adv_thread_mg::test_parallel_quick_sort<adv_thread_mg::thread_pool>();

    adv_thread_mg::test_thread_pool<adv_thread_mg::thread_pool_local>();
    adv_thread_mg::test_parallel_accumulate<adv_thread_mg::thread_pool_local>();
    adv_thread_mg::test_parallel_quick_sort<adv_thread_mg::thread_pool_local>();

    adv_thread_mg::test_thread_pool<adv_thread_mg::thread_pool_steal>();
    adv_thread_mg::test_parallel_accumulate<adv_thread_mg::thread_pool_steal>();
    adv_thread_mg::test_parallel_quick_sort<adv_thread_mg::thread_pool_steal>();

    adv_thread_mg::test_interruptible_thread();
    adv_thread_mg::test_monitor_filesystem();
#endif

#if 0//chapter10
    test_debug_mulitithread::test_concurrent_push_and_pop_on_empty_queue();
#endif

#if 0//chapterC
    messaging::test_functional_pointer_1();
    messaging::test_functional_pointer_2();
    messaging::test_atm_messaging();
#endif

    return 0;
}

