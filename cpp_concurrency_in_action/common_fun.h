///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    define some common functions.
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/06
#pragma once
#ifndef COMMON_FUN_H
#define COMMON_FUN_H

namespace common_fun {

inline void sleep(unsigned sleep_ms) {
    DEBUG("sleep:(%d)ms", sleep_ms);
    sleep_for(milliseconds(sleep_ms));
}

}//namespace common_fun
#endif  //COMMON_FUN_H
