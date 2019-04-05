///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    cpp_concurrency_in_action unit test, with gtest.
///
///    \author   deguangchow
///    \version  1.0
///    \2019/04/05

#include "stdafx.h"
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = ::RUN_ALL_TESTS();
    return ret;
}

