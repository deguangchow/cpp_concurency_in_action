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
    thread_sharing_data::foo();
    thread_sharing_data::stack_test();
    thread_sharing_data::thread_safe_stack_test();
    thread_sharing_data::std_lock_test();
    thread_sharing_data::hierarchical_mutex_test();
    thread_sharing_data::std_lock_ex_test();
    thread_sharing_data::process_data();
    thread_sharing_data::get_and_process_data();
    thread_sharing_data::compare_operator_test();
    thread_sharing_data::RAII_test();
    thread_sharing_data::call_once_test();
    thread_sharing_data::Connection_call_once_test();
    thread_sharing_data::Connection_concurrency_call_once_test();
#endif

#if 0//chapter4
    sync_conc_opera::wait_for_flag();
    sync_conc_opera::wait_for_condition_variable();
    sync_conc_opera::threadsafe_queue_test();
    sync_conc_opera::future_async_test();
    sync_conc_opera::future_async_struct_test();
    sync_conc_opera::packaged_task_test();
    sync_conc_opera::process_connections_test();
    sync_conc_opera::future_exception_test();
    sync_conc_opera::promise_exception_test();
    sync_conc_opera::durations_test();
    sync_conc_opera::time_points_test();
    sync_conc_opera::condition_variable_timeout_test();
    sync_conc_opera::sequential_quick_sort_test();
    sync_conc_opera::parallel_quick_sort_test();
    sync_conc_opera::spawn_task_test();
#endif

#if 0//chapter5

#endif

    atomic_type::atomic_flag_test();
    return 0;
}

