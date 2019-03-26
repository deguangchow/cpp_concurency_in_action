///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapterC: appendix C A message-passing framework and complete ATM example
///
///    \author   deguangchow
///    \version  1.0
///    \2019/03/26
#pragma once
#ifndef MESSAGING_H
#define MESSAGING_H
namespace messaging {

//Listing C.1 A simple message queue
struct message_base {
    virtual ~message_base() {}
};
template<typename Msg>
struct wrapped_message : message_base {
    Msg content;
    explicit wrapped_message(Msg const& content_) :content(content_) {}
};
class queue {
    std::mutex m;
    std::condition_variable c;
    std::queue<std::shared_ptr<message_base>> q;
public:
    template<typename T>
    void push(T const& msg);
    std::shared_ptr<message_base> wait_and_pop();
};

//Listing C.2 The sender class
class sender {
    queue* q;
public:
    sender();
    explicit sender(queue* q_);
    template<typename Message>
    void send(Message const& msg);
};

//Listing C.3 The receiver class
class dispatcher;
class receiver {
    queue q;
public:
    operator sender();
    dispatcher wait();
};

//Listing C.4 The dispatcher class
class close_queue {};
class dispatcher {
    queue* q;
    bool chained;
    dispatcher(dispatcher const&) = delete;
    dispatcher& operator=(dispatcher const&) = delete;
    template<typename Dispatcher, typename Msg, typename Func>
    friend class TemplateDispatcher;
    void wait_and_dispatch();
    bool dispatch(std::shared_ptr<message_base> const& msg);
public:
    dispatcher(dispatcher&& other);
    explicit dispatcher(queue* q_);
    template<typename Message, typename Func>
    TemplateDispatcher<dispatcher, Message, Func> handle(Func&& f);
    ~dispatcher() noexcept(false);
};

//Listing C.5 The TemplateDispatcher class template
template<typename PreviousDispatcher, typename Msg, typename Func>
class TemplateDispatcher {
    queue* q;
    PreviousDispatcher* prev;
    Func f;
    bool chained;

    TemplateDispatcher(TemplateDispatcher const&) = delete;
    TemplateDispatcher& operator=(TemplateDispatcher const&) = delete;
    template<typename Dispatcher, typename OtherMsg, typename OtherFunc>
    friend class TemplateDispatcher;

    void wait_and_dispatch() {
        TICK();
        for (;;) {
            auto msg = q->wait_and_pop();
            if (dispatch(msg)) {
                break;
            }
        }
    }
    bool dispatch(std::shared_ptr<message_base> const& msg) {
        TICK();
        if (wrapped_message<Msg>* wrapper = dynamic_cast<wrapped_message<Msg>*>(msg.get())) {
            f(wrapper->content);
            return true;
        } else {
            return prev->dispatch(msg);
        }
    }

public:
    TemplateDispatcher(TemplateDispatcher&& other) :
        q(other.q), prev(other.prev), f(std::move(other.f)), chained(other.chained) {
        other.chained = true;
    }
    TemplateDispatcher(queue* q_, PreviousDispatcher* prev_, Func&& f_) :
        q(q_), prev(prev_), f(std::forward<Func>(f_)), chained(false) {
        prev_->chained = false;
    }
    template<typename OtherMsg, typename OtherFunc>
    TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc> handle(OtherFunc&& of) {
        TICK();
        return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(q, this, std::forward<OtherFunc>(of));
    }
    ~TemplateDispatcher() noexcept(false) {
        if (!chained) {
            wait_and_dispatch();
        }
    }
};

}//namespace messaging
#endif  //MESSAGING_H

