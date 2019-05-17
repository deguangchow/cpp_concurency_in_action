///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    stdafx.h
///
///    \author   deguangchow
///    \version  1.0
///    \2018/11/23
#pragma once
#ifndef STDAFX_H
#define STDAFX_H

//for the usage of 'rand_s'
#define _CRT_RAND_S

//+ system`s head file.
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <list>
#include <mutex>
#include <deque>
#include <stack>
#include <map>
#include <queue>
#include <memory>
#include <condition_variable>
#include <future>
#include <utility>
#include <set>

//+ user`s head file.
#include "targetver.h"
#include "common_def.h"
#include "console_log.h"
#include "common_fun.h"


//using std::
using std::thread;
using std::mutex;
using std::condition_variable;
using std::condition_variable_any;
using std::lock_guard;
using std::unique_lock;
using std::future;
using std::promise;
using std::packaged_task;
using std::this_thread::get_id;
using std::this_thread::sleep_for;
using std::this_thread::sleep_until;
using std::this_thread::yield;

using std::chrono::seconds;
using std::chrono::milliseconds;
using std::chrono::microseconds;

using std::move;
using std::forward;

using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;

using std::string;
using std::map;
using std::set;
using std::queue;
using std::vector;
using std::list;
using std::array;

#endif  //STDAFX_H

