///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter7: Designing lock-free concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/16
#include "stdafx.h"
#include "lock_free_concurrent_data_structures.h"

namespace lock_free_conc_data {

spinlock_mutex slm;
unsigned count = 0;
void spinlock_mutex_plus() {
    TICK();
    std::lock_guard<spinlock_mutex> lock(slm);
    ++count;
    INFO("count=%d", count);
}
void spinlock_mutex_test() {
    TICK();
    std::thread t1(spinlock_mutex_plus);
    std::thread t2(spinlock_mutex_plus);
    std::thread t3(spinlock_mutex_plus);
    std::thread t4(spinlock_mutex_plus);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

}//namespace lock_free_conc_data


