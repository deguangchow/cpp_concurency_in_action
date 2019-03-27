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

//Listing C.7 The ATM state machine
void atm::process_withdrawal() {
    TICK();
    incoming.wait().handle<withdraw_ok>([&](withdraw_ok const& msg) {
        interface_hardware.send(issue_money(withdrawal_amount));
        state = &atm::done_processing;
    }).handle<withdraw_denied>([&](withdraw_denied const& msg) {
        interface_hardware.send(display_insufficient_funds());
        state = &atm::done_processing;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        bank.send(cancel_withdrawal(account, withdrawal_amount));
        interface_hardware.send(display_withdrawal_cancelled());
        state = &atm::done_processing;
    });
}
void atm::process_balance() {
    TICK();
    incoming.wait().handle<balance>([&](balance const& msg) {
        interface_hardware.send(display_balance(msg.amount));
        state = &atm::wait_for_action;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        state = &atm::done_processing;
    });
}
void atm::wait_for_action() {
    TICK();
    interface_hardware.send(display_withdrawal_options());
    incoming.wait().handle<withdraw_processed>([&](withdraw_processed const& msg) {
        withdrawal_amount = msg.amount;
        bank.send(withdraw(account, msg.amount, incoming));
        state = &atm::process_withdrawal;
    }).handle<balance_pressed>([&](balance_pressed const& msg) {
        bank.send(get_balance(account, incoming));
        state = &atm::process_balance;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        state = &atm::done_processing;
    });
}
void atm::verifying_pin() {
    TICK();
    incoming.wait().handle<pin_verified>([&](pin_verified const& msg) {
        state = &atm::wait_for_action;
    }).handle<pin_incorrect>([&](pin_incorrect const& msg) {
        interface_hardware.send(display_pin_incorrect_message());
        state = &atm::done_processing;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        state = &atm::done_processing;
    });
}
void atm::getting_pin() {
    TICK();
    incoming.wait().handle<digit_passed>([&](digit_passed const& msg) {
        unsigned const pin_length = 4;
        pin + msg.digit;
        if (pin.length() == pin.length()) {
            bank.send(verify_pin(account, pin, incoming));
            state = &atm::verifying_pin;
        }
    }).handle<clear_last_pressed>([&](clear_last_pressed const& msg) {
        if (!pin.empty()) {
            pin.pop_back();
        }
    }).handle<cancel_pressed>([&](cancel_pressed const&) {
        state = &atm::done_processing;
    });
}
void atm::waiting_for_card() {
    TICK();
    interface_hardware.send(display_enter_card());
    incoming.wait().handle<card_inserted>([&](card_inserted const& msg) {
        account = msg.account;
        pin = "";
        interface_hardware.send(display_enter_pin());
        state = &atm::getting_pin;
    });
}
void atm::done_processing() {
    TICK();
    interface_hardware.send(eject_card());
    state = &atm::waiting_for_card;
}
atm::atm(messaging::sender bank_, messaging::sender interface_hardware_) :
    bank(bank_), interface_hardware(interface_hardware_) {
    state = nullptr;
    withdrawal_amount = 0;
}
void atm::done() {
    TICK();
    get_sender().send(messaging::close_queue());
}
void atm::run() {
    TICK();
    state = &atm::waiting_for_card;
    try {
        for (;;) {
            (this->*state)();
        }
    }
    catch (...) {
    }
}
messaging::sender atm::get_sender() {
    TICK();
    return incoming;
}

}//namespace messaging

