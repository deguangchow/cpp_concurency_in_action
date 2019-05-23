///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter9: Advanced thread management
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/22
#include "stdafx.h"
#include "advanced_thread_management.h"

namespace adv_thread_mg {

//9.1 Thread pools
//9.1.1 The simplest posible thread pool
//Listing 9.1 Simple thread pool
void task1() {
    TICK();
    INFO("task1");
}
void task2() {
    TICK();
    INFO("task2");
}
int sum(int a, int b) {
    TICK();
    INFO("sum(%d, %d)", a, b);
    return a + b;
}
void test_simple_thread_pool() {
    TICK();
    simple_thread_pool simpleThreadPool;
    for (int i = 1; i <= 2; ++i) {
        simpleThreadPool.submit(task1);
    }
    for (int i = 1; i <= 2; ++i) {
        simpleThreadPool.submit(task2);
    }

    async(&simple_thread_pool::submit<TASK_TYPE>, &simpleThreadPool, task1);
    async(&simple_thread_pool::submit<TASK_TYPE>, &simpleThreadPool, task2);

    simpleThreadPool.submit([] {
        TICK();
        INFO("task5");
    });
    simpleThreadPool.submit([] {
        TICK();
        INFO("task6");
    });

    int a = 1, b = 2;
    auto const& lambdaSum = [](int a, int b) {
        TICK();
        INFO("lambda_sum(%d, %d)", a, b);
        return a + b;
    };

    simpleThreadPool.submit<TASK_TYPE>(bind(sum, a, b));
    simpleThreadPool.submit<TASK_TYPE>(bind(lambdaSum, a, b));

    async(&simple_thread_pool::submit<TASK_TYPE>, &simpleThreadPool, bind(sum, a, b));
    async(&simple_thread_pool::submit<TASK_TYPE>, &simpleThreadPool, bind(lambdaSum, a, b));
}


thread_local unique_ptr<LOCAL_QUEUE_TYPE>    thread_pool_local::m_pQueuelocalTasks_tl = nullptr;


thread_local work_stealing_queue*            thread_pool_steal::m_pQueueLocalTasks_tl;
thread_local unsigned                        thread_pool_steal::m_uIndex_tl;


//9.2 Interrupting threads
//9.2.1 Launching and interrupting another thread
//Listing 9.9 Basic implementation of interruptible_thread
void interrupt_flag::set() {
    flag.store(true);
}
bool interrupt_flag::is_set() const {
    return flag.load();
}

thread_local interrupt_flag this_thread_interrupt_flag;

template<typename FunctionType>
interruptible_thread::interruptible_thread(FunctionType f) {
    promise<interrupt_flag*> p;
    internal_thread = thread([f, &p] {
        p.set_value(&this_thread_interrupt_flag);
        f();
    });
    flag = p.get_future().get();
}
void interruptible_thread::join() {
    internal_thread.join();
}
void interruptible_thread::detach() {
    internal_thread.detach();
}
bool interruptible_thread::joinable() const {
    return internal_thread.joinable();
}
void interruptible_thread::interrupt() {
    if (flag) {
        flag->set();
    }
}

//9.2.2 Detecting that a thread has been interrupted
void interruption_point() {
    if (this_thread_interrupt_flag.is_set()) {
        throw current_exception();//throw thread_interrupted();
    }
}
void interruptible_thread_test() {
    bool done = false;
    while (!done) {
        interruption_point();
        //process_next_item();
    }
}

//9.2.3 Interrupting a condition variable wait
//Listing 9.10 A broken version of interruptible_wait for condition_variable
//Listing 9.11 Using a timeout in interruptible_wait for condition_variable
thread_local interrupt_flag_cv this_thread_interrupt_flag_cv;

interrupt_flag_cv::interrupt_flag_cv() : thread_cond(0) {
}
void interrupt_flag_cv::set() {
    TICK();
    flag.store(true, memory_order::memory_order_relaxed);
    lock_guard<mutex> lk(set_clear_mutex);
    if (thread_cond) {
        thread_cond->notify_all();
    }
}
bool interrupt_flag_cv::is_set() const {
    TICK();
    return flag.load(memory_order::memory_order_relaxed);
}
void interrupt_flag_cv::set_condition_variable(condition_variable& cv) {
    TICK();
    lock_guard<mutex> lk(set_clear_mutex);
    thread_cond = &cv;
}
void interrupt_flag_cv::clear_condition_variable() {
    TICK();
    lock_guard<mutex> lk(set_clear_mutex);
    thread_cond = 0;
}
interrupt_flag_cv::clear_cv_on_destruct::~clear_cv_on_destruct() {
    TICK();
    this_thread_interrupt_flag_cv.clear_condition_variable();
}
void interruption_point_cv() {
    TICK();
    if (this_thread_interrupt_flag_cv.is_set()) {
        throw current_exception();//throw thread_interrupted();
    }
}
void interruptible_wait(condition_variable& cv, unique_lock<mutex>& lk) {
    TICK();
    interruption_point_cv();
    this_thread_interrupt_flag_cv.set_condition_variable(cv);
    interrupt_flag_cv::clear_cv_on_destruct guard;
    interruption_point_cv();
    cv.wait_for(lk, milliseconds(ONE));
    interruption_point_cv();
}
template<typename Predicate>
void interruptible_wait(condition_variable& cv, unique_lock<mutex>& lk, Predicate pred) {
    TICK();
    interruption_point_cv();
    this_thread_interrupt_flag_cv.set_condition_variable(cv);
    interrupt_flag_cv::clear_cv_on_destruct guard;
    while (!this_thread_interrupt_flag_cv.is_set() && !pred()) {
        cv.wait_for(lk, milliseconds(ONE));
    }
    interruption_point_cv();
}

//9.2.4 Interrupting a wait on condition_variable_any
//Listing 9.12 interruptible_wait for condition_variable_any
thread_local interrupt_flag_cva this_thread_interrupt_flag_cva;

interrupt_flag_cva::interrupt_flag_cva() : thread_cond(0), thread_cond_any(0) {
}
void interrupt_flag_cva::set() {
    TICK();
    flag.store(true, memory_order::memory_order_relaxed);
    lock_guard<mutex> lk(set_clear_mutex);
    if (thread_cond) {
        thread_cond->notify_all();
    } else if (thread_cond_any) {
        thread_cond_any->notify_all();
    }
}
bool interrupt_flag_cva::is_set() const {
    TICK();
    return flag.load(memory_order::memory_order_relaxed);
}
template<typename Lockable>
void interrupt_flag_cva::wait(condition_variable_any& cv, Lockable& lk) {
    struct custom_lock {
        interrupt_flag_cva* self;
        Lockable& lk;
        custom_lock(interrupt_flag_cva* self_, condition_variable_any& cond, Lockable& lk_) :
            self(self_), lk(lk_) {
            TICK();
            self->set_clear_mutex.lock();
            self->thread_cond_any = &cond;
        }
        void unlock() {
            TICK();
            lk.unlock();
            self->set_clear_mutex.unlock();
        }
        void lock() {
            TICK();
            lock(self->set_clear_mutex, lk);
        }
        ~custom_lock() {
            TICK();
            self->thread_cond_any = 0;
            self->set_clear_mutex.unlock();
        }
    };
    custom_lock cl(this, cv, lk);
    interruption_point_cva();
    cv.wait(cl);
    interruption_point_cva();
}
void interrupt_flag_cva::set_condition_variable(condition_variable& cv) {
    TICK();
    lock_guard<mutex> lk(set_clear_mutex);
    thread_cond = &cv;
}
void interrupt_flag_cva::clear_condition_variable() {
    TICK();
    lock_guard<mutex> lk(set_clear_mutex);
    thread_cond = 0;
    thread_cond_any = 0;
}
void interruption_point_cva() {
    TICK();
    if (this_thread_interrupt_flag_cva.is_set()) {
        throw current_exception();//throw thread_interrupted();
    }
}
template<typename Lockable>
void interruptible_wait(condition_variable_any& cv, Lockable& lk) {
    TICK();
    this_thread_interrupt_flag_cva.wait(cv, lk);
}

//9.2.5 Interrupting other blocking calls
template<typename T>
void interruptible_wait(future<T>& uf) {
    TICK();
    while (!this_thread_interrupt_flag_cva.is_set()) {
        if (uf.wait_for(lk, milliseconds(1)) == future_status::ready) {
            break;
        }
    }
    interruption_point_cva();
}

//9.2.7 Interrupting background tasks on application exit
//Listing 9.13 Monitoring the filesystem in the background
mutex config_mutex;
vector<interruptible_thread> background_threads;
class fs_change {
public:
    bool has_changes() const {
        return true;
    }
};
fs_change get_fs_changes(int disk_id) {
    TICK();
    return fs_change();
}
void update_index(fs_change const& fsc) {
    TICK();
}
void background_thread(int disk_id) {
    TICK();
    while (true) {
        interruption_point_cva();
        fs_change fsc = get_fs_changes(disk_id);
        if (fsc.has_changes()) {
            update_index(fsc);
        }
    }
}
void start_background_processing() {
    TICK();
    int disk_1 = 1, disk_2 = 2;
    typedef function<void(int)> F;
    //background_threads.push_back(interruptible_thread::<F>(background_thread, disk_1));
    //background_threads.push_back(interruptible_thread::<F>(background_thread, disk_2));
}
void process_gui_until_exit() {
    TICK();
}
void monitor_filesystem() {
    TICK();
    start_background_processing();
    process_gui_until_exit();
    unique_lock<mutex> lk(config_mutex);
    for (unsigned i = 0; i < background_threads.size(); ++i) {
        background_threads[i].interrupt();
    }
    for (unsigned i = 0; i < background_threads.size(); ++i) {
        background_threads[i].join();
    }
}

}//namespace adv_thread_mg

