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
class threadsafe_queue {
    mutex               m_mutex;
    condition_variable  m_conditonVariable;
    queue<shared_ptr<message_base>> m_queueMessage;
public:
    template<typename T>
    void push(T const& msg);
    shared_ptr<message_base> wait_and_pop();
};

//Listing C.2 The sender class
class sender {
    threadsafe_queue*   m_pQueueMessage;
public:
    sender();
    explicit sender(threadsafe_queue* q_);

    template<typename Message>
    void send(Message const& msg);
};

//Listing C.3 The receiver class
class dispatcher;
class receiver {
    threadsafe_queue    m_queueMessage;
public:
    operator sender();
    dispatcher wait();
};

//Listing C.4 The dispatcher class
class close_queue {};
class dispatcher {
    threadsafe_queue*   m_pQueueMessage;
    bool                m_bChained;
    dispatcher(dispatcher const&) = delete;
    dispatcher& operator=(dispatcher const&) = delete;

    template<typename Dispatcher, typename Msg, typename Func>
    friend class TemplateDispatcher;

    void wait_and_dispatch();
    bool dispatch(shared_ptr<message_base> const& msg);
public:
    dispatcher(dispatcher&& other);
    explicit dispatcher(threadsafe_queue* q_);
    ~dispatcher() noexcept(false);

    template<typename Message, typename Func>
    TemplateDispatcher<dispatcher, Message, Func> handle(Func&& f);
};

//Listing C.5 The TemplateDispatcher class template
template<typename PreviousDispatcher, typename Msg, typename Func>
class TemplateDispatcher {
    threadsafe_queue*   m_pQueueMessage;
    PreviousDispatcher* m_pPreviousDispatcher;
    Func                m_function;
    bool                m_bChained;

    TemplateDispatcher(TemplateDispatcher const&) = delete;
    TemplateDispatcher& operator=(TemplateDispatcher const&) = delete;
    template<typename Dispatcher, typename OtherMsg, typename OtherFunc>
    friend class TemplateDispatcher;

    void wait_and_dispatch() {
        TICK();
        try {
            for (;;) {
                auto msg = m_pQueueMessage->wait_and_pop();
                if (dispatch(msg)) {
                    break;
                }
                yield();
            }
        } catch (close_queue const& e) {
            INFO("catch close_queue, throw it.");
            throw e;
        } catch (...) {
            ERR("catch ..., throw close_queue.");
            throw close_queue();
        }
    }
    bool dispatch(shared_ptr<message_base> const& msg) {
        TICK();
        if (wrapped_message<Msg>* wrapper = dynamic_cast<wrapped_message<Msg>*>(msg.get())) {
            m_function(wrapper->content);
            return true;
        } else {
            return m_pPreviousDispatcher->dispatch(msg);
        }
    }

public:
    TemplateDispatcher(TemplateDispatcher&& other) :
        m_pQueueMessage(other.m_pQueueMessage), m_pPreviousDispatcher(other.m_pPreviousDispatcher),
        m_function(move(other.m_function)), m_bChained(other.m_bChained) {
        other.m_bChained = true;
    }
    TemplateDispatcher(threadsafe_queue* q_, PreviousDispatcher* prev_, Func&& f_) :
        m_pQueueMessage(q_), m_pPreviousDispatcher(prev_), m_function(forward<Func>(f_)), m_bChained(false) {
        prev_->m_bChained = true;
    }
    template<typename OtherMsg, typename OtherFunc>
    TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc> handle(OtherFunc&& of) {
        TICK();
        return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>
            (m_pQueueMessage, this, forward<OtherFunc>(of));
    }
    ~TemplateDispatcher() noexcept(false) {
        if (!m_bChained) {
            wait_and_dispatch();
        }
    }
};

//Listing C.6 ATM message
struct withdraw {
    string account;
    unsigned amount;
    mutable sender atm_queue;
    withdraw(string const& account_, unsigned amount_, sender atm_queue_) :
        account(account_), amount(amount_), atm_queue(atm_queue_) {}
};
struct withdraw_amount {};
struct withdraw_ok {};
struct withdraw_denied {};
struct cancel_withdrawal {
    string account;
    unsigned amount;
    cancel_withdrawal(string const& account_, unsigned amount_) :account(account_), amount(amount_) {}
};
struct withdraw_processed {
    string account;
    unsigned amount;
    withdraw_processed(string const& account_, unsigned amount_) :account(account_), amount(amount_) {}
};
struct withdraw_amount_processed {
    string account;
    explicit withdraw_amount_processed(string const& account_) :account(account_) {}
};
struct card_inserted {
    string account;
    explicit card_inserted(string const& account_) :account(account_) {}
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
struct ok_pressed {};
struct issue_money {
    unsigned amount;
    explicit issue_money(unsigned amount_) :amount(amount_) {}
};
struct verify_pin {
    string acount;
    string pin;
    mutable sender atm_queue;
    verify_pin(string const& acount_, string const& pin_, sender atm_queue_) :
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
struct display_withdrawal_amount {};
struct get_balance {
    string account;
    mutable sender atm_queue;
    get_balance(string const& account_, sender atm_queue_) :account(account_), atm_queue(atm_queue_) {}
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
class atm_state_machine {
    receiver    m_receiverInComing;
    sender      m_senderBank;
    sender      m_senderUI;

    //functional pointer member variable, it means that m_fpState point to a function: void f();
    void(atm_state_machine::* m_fpState) ();

    string      m_sAccount;
    unsigned    m_uWithdrawalAmount;
    string      m_sPIN;

    void getting_amount();
    void process_withdrawal();
    void process_balance();
    void wait_for_action();
    void verifying_pin();
    void getting_pin();
    void waiting_for_card();
    void done_processing();
    void continue_processing();
    atm_state_machine(atm_state_machine const&) = delete;
    atm_state_machine& operator=(atm_state_machine const&) = delete;

public:
    atm_state_machine(sender bank_, sender ui_);
    void done();
    void run();
    sender get_sender();

#if 1
    int(atm_state_machine::* m_fpTest)(int const&, int const&) const;
    int sum(int const& a, int const& b) const {
        TICK();
        return a + b;
    }
    void test_functional_pointer() {
        TICK();
        m_fpTest = &atm_state_machine::sum;
        auto const& a = 1;
        auto const& b = 2;
        auto const& c = (this->*m_fpTest)(a, b);
        DEBUG("sum(%d, %d) = %d", a, b, c);
    }
#endif
};

//Listing C.8 The bank state machine
class bank_state_machine {
    receiver    m_receiverIncoming;
    unsigned    m_uBalance;
    void(bank_state_machine::* m_fpState)();

    void wait_for_action();
public:
    bank_state_machine();
    void done();
    void run();
    sender get_sender();
};

//Listing C.9 The user-interface state machine
class ui_state_machine {
    receiver    m_receiverIncoming;
    void(ui_state_machine::* m_fpState)();

    void wait_for_show();
public:
    ui_state_machine();
    void done();
    void run();
    sender get_sender();
};

//Listing C.10 The driving code
void test_atm_messaging();

void test_functional_pointer_1();
void test_functional_pointer_2();

}//namespace messaging
#endif  //MESSAGING_H

