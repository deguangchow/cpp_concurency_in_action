///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    define some common functions.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/06
#include "stdafx.h"
#include "common_fun.h"

namespace common_fun {

#if 0
void sleep(unsigned sleep_ms) {
    INFO("thread(%d) sleep:(%d)ms", std::this_thread::get_id(), sleep_ms);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
}
#endif

}//namespace common_fun


