///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapterC: appendix C A message-passing framework and complete ATM example
///
///    \author   deguangchow
///    \version  1.0
///    \2019/03/26
#include "stdafx.h"
#include "messaging.h"

namespace messaging {


template<typename T>
void queue::push(T const& msg) {
    TICK();
    std::lock_guard<std::mutex> lk(m);
    q.push(std::make_shared<wrapped_message<T>>(msg));
    c.notify_all();
}
std::shared_ptr<message_base> queue::wait_and_pop() {
    TICK();
    std::unique_lock<std::mutex> lk(m);
    c.wait(lk, [&] {return !q.empty(); });
    auto res = q.front();
    q.pop();
    return res;
}

receiver::operator sender() {
    TICK();
    return sender(&q);
}
messaging::dispatcher receiver::wait() {
    TICK();
    return dispatcher(&q);
}


sender::sender(queue* q_) : q(q_) {
}
sender::sender() : q(nullptr) {
}
template<typename Message>
void sender::send(Message const& msg) {
    TICK();
    if (q) {
        q->push(msg);
    }
}

dispatcher::dispatcher(queue* q_) :q(q_), chained(false) {
}
dispatcher::dispatcher(dispatcher&& other) : q(other.q), chained(other.chained) {
    other.chained = true;
}
void dispatcher::wait_and_dispatch() {
    TICK();
    for (;;) {
        auto msg = q->wait_and_pop();
        dispatch(msg);
    }
}
bool dispatcher::dispatch(std::shared_ptr<message_base> const& msg) {
    TICK();
    if (dynamic_cast<wrapped_message<close_queue>*>(msg.get())) {
        throw close_queue();
    }
    return false;
}
template<typename Message, typename Func>
TemplateDispatcher<dispatcher, Message, Func>
dispatcher::handle(Func&& f) {
    TICK();
    return TemplateDispatcher<dispatcher, Message, Func>(q, this, std::forward<Func>(f));
}
dispatcher::~dispatcher() noexcept(false) {
    if (!chained) {
        wait_and_dispatch();
    }
}

}//namespace messaging

