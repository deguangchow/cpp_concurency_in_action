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
        try {
            for (;;) {
                auto msg = q->wait_and_pop();
                if (dispatch(msg)) {
                    break;
                }
            }
        } catch (messaging::close_queue const& e) {
            INFO("catch close_queue, throw it.");
            throw e;
        } catch (...) {
            ERR("catch ..., throw close_queue.");
            throw close_queue();
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
        prev_->chained = true;
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

//Listing C.6 ATM message
struct withdraw {
    std::string account;
    unsigned amount;
    mutable messaging::sender atm_queue;
    withdraw(std::string const& account_, unsigned amount_, messaging::sender atm_queue_) :
        account(account_), amount(amount_), atm_queue(atm_queue_) {}
};
struct withdraw_ok {};
struct withdraw_denied {};
struct cancel_withdrawal {
    std::string account;
    unsigned amount;
    cancel_withdrawal(std::string const& account_, unsigned amount_) :account(account_), amount(amount_) {}
};
struct withdraw_processed {
    std::string account;
    unsigned amount;
    withdraw_processed(std::string const& account_, unsigned amount_) :account(account_), amount(amount_) {}
};
struct card_inserted {
    std::string account;
    explicit card_inserted(std::string const& account_) :account(account_) {}
};
struct digit_pressed {
    char digit;
    explicit digit_pressed(char digit_) :digit(digit_) {}
};
struct clear_last_pressed {};
struct eject_card {};
struct withdraw_passed {
    unsigned amount;
    explicit withdraw_passed(unsigned amount_) :amount(amount_) {}
};
struct cancel_pressed {};
struct issue_money {
    unsigned amount;
    explicit issue_money(unsigned amount_) :amount(amount_) {}
};
struct verify_pin {
    std::string acount;
    std::string pin;
    mutable messaging::sender atm_queue;
    verify_pin(std::string const& acount_, std::string const& pin_, messaging::sender atm_queue_) :
        acount(acount_), pin(pin_), atm_queue(atm_queue_) {}
};
struct pin_verified {};
struct pin_incorrect {};
struct display_enter_pin {};
struct display_enter_card {};
struct display_insufficient_funds {};
struct display_withdrawal_cancelled {};
struct display_pin_incorrect_message {};
struct display_withdrawal_options {};
struct get_balance {
    std::string account;
    mutable messaging::sender atm_queue;
    get_balance(std::string const& account_, messaging::sender atm_queue_) :account(account_), atm_queue(atm_queue_) {}
};
struct balance {
    unsigned amount;
    explicit balance(unsigned amount_) :amount(amount_) {}
};
struct display_balance {
    unsigned amount;
    explicit display_balance(unsigned amount_) :amount(amount_) {}
};
struct balance_pressed {};

//Listing C.7 The ATM state machine
class atm {
    messaging::receiver incoming;
    messaging::sender bank;
    messaging::sender interface_hardware;
    void(atm::*state) ();
    std::string account;
    unsigned withdrawal_amount;
    std::string pin;
    void process_withdrawal();
    void process_balance();
    void wait_for_action();
    void verifying_pin();
    void getting_pin();
    void waiting_for_card();
    void done_processing();
    atm(atm const&) = delete;
    atm& operator=(atm const&) = delete;

public:
    atm(messaging::sender bank_, messaging::sender interface_hardware_);
    void done();
    void run();
    messaging::sender get_sender();
};

//Listing C.8 The bank state machine
class bank_machine {
    messaging::receiver incoming;
    unsigned _balance;
public:
    bank_machine();
    void done();
    void run();
    messaging::sender get_sender();
};

//Listing C.9 The user-interface state machine
class interface_machine {
    messaging::receiver incoming;
public:
    void done();
    void run();
    messaging::sender get_sender();
};

//Listing C.10 The driving code
void atm_messaging_test();

}//namespace messaging
#endif  //MESSAGING_H

