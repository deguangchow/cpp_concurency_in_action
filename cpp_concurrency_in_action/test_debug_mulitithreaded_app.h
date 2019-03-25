///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter10: Testing and debugging multithreaded applications
///
///    \author   deguangchow
///    \version  1.0
///    \2019/03/25
#pragma once
#ifndef TEST_DEBUG_MULTITHREAED_APP_H
#define TEST_DEBUG_MULTITHREAED_APP_H

namespace test_debug_mulitithread {

//10.2.5 Structuring multithreaded test code
//Listing 10.1 An example test for concurrent push() and pop() calls on a queue
void test_concurrent_push_and_pop_on_empty_queue();


}//namespace test_debug_mulitithread

#endif  //TEST_DEBUG_MULTITHREAED_APP_H

