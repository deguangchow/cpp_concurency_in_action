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
#endif

    thread_sharing_data::hierarchical_mutex_test();

    return 0;
}

