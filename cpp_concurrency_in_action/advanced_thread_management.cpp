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
    m_bFlag_a.store(true);
}
bool interrupt_flag::is_set() const {
    return m_bFlag_a.load();
}

thread_local interrupt_flag g_InterruptFlag_tl;

template<typename FunctionType>
interruptible_thread::interruptible_thread(FunctionType f) {
    promise<interrupt_flag*> p;
    m_threadInternal = thread([f, &p] {
        p.set_value(&g_InterruptFlag_tl);
        f();
    });
    m_pFlag = p.get_future().get();
}
void interruptible_thread::join() {
    m_threadInternal.join();
}
void interruptible_thread::detach() {
    m_threadInternal.detach();
}
bool interruptible_thread::joinable() const {
    return m_threadInternal.joinable();
}
void interruptible_thread::interrupt() {
    TICK();
    if (m_pFlag) {
        m_pFlag->set();
    }
}

//9.2.2 Detecting that a thread has been interrupted
void interruption_point() {
    TICK();
    if (g_InterruptFlag_tl.is_set()) {
        DEBUG("is_set");
        throw current_exception();//throw thread_interrupted();
    }
}
void test_interruptible_thread() {
    TICK();
    bool done = false;
    while (!done) {
        interruption_point();
        //process_next_item();

        INFO("process_next_item");
        common_fun::sleep(1000);
        yield();
    }
}

//9.2.3 Interrupting a condition variable wait
//Listing 9.10 A broken version of interruptible_wait for condition_variable
//Listing 9.11 Using a timeout in interruptible_wait for condition_variable
thread_local interrupt_flag_cv g_interruptFlagCv_tl;

interrupt_flag_cv::interrupt_flag_cv() : m_pcvThread(0) {
}
void interrupt_flag_cv::set() {
    TICK();
    m_bFlag_a.store(true, memory_order::memory_order_relaxed);
    lock_guard<mutex> lk(m_mutexSetClear);
    if (m_pcvThread) {
        m_pcvThread->notify_all();
    }
}
bool interrupt_flag_cv::is_set() const {
    TICK();
    return m_bFlag_a.load(memory_order::memory_order_relaxed);
}
void interrupt_flag_cv::set_condition_variable(condition_variable& cv) {
    TICK();
    lock_guard<mutex> lk(m_mutexSetClear);
    m_pcvThread = &cv;
}
void interrupt_flag_cv::clear_condition_variable() {
    TICK();
    lock_guard<mutex> lk(m_mutexSetClear);
    m_pcvThread = 0;
}
interrupt_flag_cv::clear_cv_on_destruct::~clear_cv_on_destruct() {
    TICK();
    g_interruptFlagCv_tl.clear_condition_variable();
}
void interruption_point_cv() {
    TICK();
    if (g_interruptFlagCv_tl.is_set()) {
        WARN("is_set, throw thread_interrupted");
        throw current_exception();//throw thread_interrupted();
    }
}
void interruptible_wait(condition_variable& cv, unique_lock<mutex>& lk) {
    TICK();
    interruption_point_cv();
    g_interruptFlagCv_tl.set_condition_variable(cv);
    interrupt_flag_cv::clear_cv_on_destruct guard;
    interruption_point_cv();
    cv.wait_for(lk, milliseconds(ONE));
    interruption_point_cv();
}
template<typename Predicate>
void interruptible_wait(condition_variable& cv, unique_lock<mutex>& lk, Predicate pred) {
    TICK();
    interruption_point_cv();
    g_interruptFlagCv_tl.set_condition_variable(cv);
    interrupt_flag_cv::clear_cv_on_destruct guard;
    while (!g_interruptFlagCv_tl.is_set() && !pred()) {
        cv.wait_for(lk, milliseconds(ONE));
    }
    interruption_point_cv();
}

//9.2.4 Interrupting a wait on condition_variable_any
//Listing 9.12 interruptible_wait for condition_variable_any
thread_local interrupt_flag_cva g_interruptFlagCva_tl;

interrupt_flag_cva::interrupt_flag_cva() : m_pcvThread(0), m_pcvaThread(0) {
}
void interrupt_flag_cva::set() {
    TICK();
    m_bFlag_a.store(true, memory_order::memory_order_relaxed);
    lock_guard<mutex> lk(m_mutexSetClear);
    if (m_pcvThread) {
        m_pcvThread->notify_all();
    } else if (m_pcvaThread) {
        m_pcvaThread->notify_all();
    }
}
bool interrupt_flag_cva::is_set() const {
    TICK();
    return m_bFlag_a.load(memory_order::memory_order_relaxed);
}
template<typename Lockable>
void interrupt_flag_cva::wait(condition_variable_any& cv, Lockable& lk) {
    struct custom_lock {
        interrupt_flag_cva* self;
        Lockable& lk;
        custom_lock(interrupt_flag_cva* self_, condition_variable_any& cond, Lockable& lk_) :
            self(self_), lk(lk_) {
            TICK();
            self->m_mutexSetClear.lock();
            self->m_pcvaThread = &cond;
        }
        void unlock() {
            TICK();
            lk.unlock();
            self->m_mutexSetClear.unlock();
        }
        void lock() {
            TICK();
            lock(self->m_mutexSetClear, lk);
        }
        ~custom_lock() {
            TICK();
            self->m_pcvaThread = 0;
            self->m_mutexSetClear.unlock();
        }
    };
    custom_lock cl(this, cv, lk);
    interruption_point_cva();
    cv.wait(cl);
    interruption_point_cva();
}
void interrupt_flag_cva::set_condition_variable(condition_variable& cv) {
    TICK();
    lock_guard<mutex> lk(m_mutexSetClear);
    m_pcvThread = &cv;
}
void interrupt_flag_cva::clear_condition_variable() {
    TICK();
    lock_guard<mutex> lk(m_mutexSetClear);
    m_pcvThread = 0;
    m_pcvaThread = 0;
}
void interruption_point_cva() {
    TICK();
    if (g_interruptFlagCva_tl.is_set()) {
        throw current_exception();//throw thread_interrupted();
    }
}
template<typename Lockable>
void interruptible_wait(condition_variable_any& cv, Lockable& lk) {
    TICK();
    g_interruptFlagCva_tl.wait(cv, lk);
}

//9.2.5 Interrupting other blocking calls
template<typename T>
void interruptible_wait(future<T>& uf) {
    TICK();
    while (!g_interruptFlagCva_tl.is_set()) {
        if (uf.wait_for(lk, milliseconds(1)) == future_status::ready) {
            break;
        }
    }
    interruption_point_cva();
}

//9.2.7 Interrupting background tasks on application exit
//Listing 9.13 Monitoring the filesystem in the background
mutex g_mutexConfig;
vector<interruptible_thread> g_vctBackgroundThreads;
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
    g_vctBackgroundThreads.push_back(interruptible_thread(bind(background_thread, disk_1)));
    g_vctBackgroundThreads.push_back(interruptible_thread(bind(background_thread, disk_2)));
}
void process_gui_until_exit() {
    TICK();

    async([] {
        while (1) {
            INFO("UpdateUI...");
            common_fun::sleep(1000);
            yield();
        }
    });
}
void test_monitor_filesystem() {
    TICK();
    start_background_processing();
    process_gui_until_exit();
    unique_lock<mutex> lk(g_mutexConfig);
    for (unsigned i = 0; i < g_vctBackgroundThreads.size(); ++i) {
        g_vctBackgroundThreads[i].interrupt();
    }
    for (unsigned i = 0; i < g_vctBackgroundThreads.size(); ++i) {
        g_vctBackgroundThreads[i].join();
    }
}

}//namespace adv_thread_mg

