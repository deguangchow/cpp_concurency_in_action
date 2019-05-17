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
void threadsafe_queue::push(T const& msg) {
    TICK();
    lock_guard<mutex> lk(m_mutex);
    m_queueMessage.push(make_shared<wrapped_message<T>>(msg));
    m_conditonVariable.notify_all();
}
shared_ptr<message_base> threadsafe_queue::wait_and_pop() {
    TICK();
    unique_lock<mutex> lk(m_mutex);
    m_conditonVariable.wait(lk, [&] {return !m_queueMessage.empty(); });
    auto res = m_queueMessage.front();
    m_queueMessage.pop();
    return res;
}

receiver::operator sender() {
    TICK();
    return sender(&m_queueMessage);
}
dispatcher receiver::wait() {
    TICK();
    return dispatcher(&m_queueMessage);
}


sender::sender(threadsafe_queue* q_) : m_pQueueMessage(q_) {
}
sender::sender() : m_pQueueMessage(nullptr) {
}
template<typename Message>
void sender::send(Message const& msg) {
    TICK();
    if (m_pQueueMessage) {
        m_pQueueMessage->push(msg);
    }
}

dispatcher::dispatcher(threadsafe_queue* q_) :m_pQueueMessage(q_), m_bChained(false) {
}
dispatcher::dispatcher(dispatcher&& other) : m_pQueueMessage(other.m_pQueueMessage), m_bChained(other.m_bChained) {
    other.m_bChained = true;
}
void dispatcher::wait_and_dispatch() {
    TICK();
    try {
        for (;;) {
            auto msg = m_pQueueMessage->wait_and_pop();
            dispatch(msg);
            yield();
        }
    } catch (close_queue const&) {
        INFO("catch close_queue");
    } catch (...) {
        INFO("catch ...");
    }
}
bool dispatcher::dispatch(shared_ptr<message_base> const& msg) {
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
    return TemplateDispatcher<dispatcher, Message, Func>(m_pQueueMessage, this, forward<Func>(f));
}
dispatcher::~dispatcher() noexcept(false) {
    if (!m_bChained) {
        wait_and_dispatch();
    }
}

//Listing C.7 The ATM state machine
void atm_state_machine::getting_amount() {
    TICK();
    m_receiverInComing.wait().handle<digit_pressed>([&](digit_pressed const& msg) {
        m_uWithdrawalAmount *= 10;
        m_uWithdrawalAmount += (msg.digit - '0');
    }).handle<clear_last_pressed>([&](clear_last_pressed const& msg) {
        m_uWithdrawalAmount /= 10;
    }).handle<ok_pressed>([&](ok_pressed const& msg) {
        m_senderBank.send(withdraw(m_sAccount, m_uWithdrawalAmount, m_receiverInComing));
        m_fpState = &atm_state_machine::process_withdrawal;
    }).handle<cancel_pressed>([&](cancel_pressed const&) {
        m_fpState = &atm_state_machine::done_processing;
    });
}
void atm_state_machine::process_withdrawal() {
    TICK();
    m_receiverInComing.wait().handle<withdraw_ok>([&](withdraw_ok const& msg) {
        m_senderUI.send(issue_money(m_uWithdrawalAmount));
        m_fpState = &atm_state_machine::continue_processing;
    }).handle<withdraw_denied>([&](withdraw_denied const& msg) {
        m_senderUI.send(display_insufficient_funds());
        m_fpState = &atm_state_machine::continue_processing;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        m_senderBank.send(cancel_withdrawal(m_sAccount, m_uWithdrawalAmount));
        m_senderUI.send(display_withdrawal_cancelled());
        m_fpState = &atm_state_machine::done_processing;
    });
}
void atm_state_machine::process_balance() {
    TICK();
    m_receiverInComing.wait().handle<balance>([&](balance const& msg) {
        m_senderUI.send(display_balance(msg.amount));
        m_fpState = &atm_state_machine::wait_for_action;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        m_fpState = &atm_state_machine::done_processing;
    });
}
void atm_state_machine::wait_for_action() {
    TICK();
    m_senderUI.send(display_withdrawal_options());
    m_receiverInComing.wait().handle<withdraw_amount_processed>([&](withdraw_amount_processed const& msg) {
        m_senderUI.send(display_withdrawal_amount());
        m_fpState = &atm_state_machine::getting_amount;
    }).handle<withdraw_processed>([&](withdraw_processed const& msg) {
        m_uWithdrawalAmount = msg.amount;
        m_senderBank.send(withdraw(m_sAccount, msg.amount, m_receiverInComing));
        m_fpState = &atm_state_machine::process_withdrawal;
    }).handle<balance_pressed>([&](balance_pressed const& msg) {
        m_senderBank.send(get_balance(m_sAccount, m_receiverInComing));
        m_fpState = &atm_state_machine::process_balance;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        m_fpState = &atm_state_machine::done_processing;
    });
}
void atm_state_machine::verifying_pin() {
    TICK();
    m_receiverInComing.wait().handle<pin_verified>([&](pin_verified const& msg) {
        m_fpState = &atm_state_machine::wait_for_action;
    }).handle<pin_incorrect>([&](pin_incorrect const& msg) {
        m_senderUI.send(display_pin_incorrect_message());
        m_fpState = &atm_state_machine::done_processing;
    }).handle<cancel_pressed>([&](cancel_pressed const& msg) {
        m_fpState = &atm_state_machine::done_processing;
    });
}
void atm_state_machine::getting_pin() {
    TICK();
    m_receiverInComing.wait().handle<digit_pressed>([&](digit_pressed const& msg) {
        m_sPIN += msg.digit;
    }).handle<clear_last_pressed>([&](clear_last_pressed const& msg) {
        if (!m_sPIN.empty()) {
            m_sPIN.pop_back();
        }
    }).handle<ok_pressed>([&](ok_pressed const& msg) {
        m_senderBank.send(verify_pin(m_sAccount, m_sPIN, m_receiverInComing));
        m_fpState = &atm_state_machine::verifying_pin;
    }).handle<cancel_pressed>([&](cancel_pressed const&) {
        m_fpState = &atm_state_machine::done_processing;
    });
}
void atm_state_machine::waiting_for_card() {
    TICK();
    m_senderUI.send(display_enter_card());
    m_receiverInComing.wait().handle<card_inserted>([&](card_inserted const& msg) {
        m_sAccount = msg.account;
        m_sPIN = "";
        m_senderUI.send(display_enter_pin());
        m_fpState = &atm_state_machine::getting_pin;
    });
}
void atm_state_machine::done_processing() {
    TICK();
    m_senderUI.send(eject_card());
    m_fpState = &atm_state_machine::waiting_for_card;
}
void atm_state_machine::continue_processing() {
    TICK();
    m_fpState = &atm_state_machine::wait_for_action;
}
atm_state_machine::atm_state_machine(sender bank_, sender ui_) :
    m_senderBank(bank_), m_senderUI(ui_) {
    m_fpState = nullptr;
    m_fpTest = nullptr;
    m_uWithdrawalAmount = 0;
}
void atm_state_machine::done() {
    TICK();
    get_sender().send(close_queue());
}
void atm_state_machine::run() {
    TICK();
    m_fpState = &atm_state_machine::waiting_for_card;
    try {
        for (;;) {
            (this->*m_fpState)();
            yield();
        }
    } catch (close_queue const&) {
        INFO("catch close_queue");
    } catch (...) {
        ERR("catch...");
    }
}
sender atm_state_machine::get_sender() {
    TICK();
    return m_receiverInComing;
}

//Listing C.8 The bank state machine
bank_state_machine::bank_state_machine() :m_uBalance(199) {
    m_fpState = nullptr;
}
void bank_state_machine::wait_for_action() {
    m_receiverIncoming.wait().handle<verify_pin>([&](verify_pin const& msg) {
        if (msg.pin == "1937") {
            msg.atm_queue.send(pin_verified());
        } else {
            msg.atm_queue.send(pin_incorrect());
        }
    }).handle<withdraw>([&](withdraw const& msg) {
        if (m_uBalance >= msg.amount) {
            msg.atm_queue.send(withdraw_ok());
            m_uBalance -= msg.amount;
        } else {
            msg.atm_queue.send(withdraw_denied());
        }
    }).handle<get_balance>([&](get_balance const& msg) {
        msg.atm_queue.send(balance(m_uBalance));
    }).handle<withdraw_processed>([&](withdraw_processed const& msg) {
    }).handle<cancel_withdrawal>([&](cancel_withdrawal const& msg) {
    });
}
void bank_state_machine::done() {
    TICK();
    get_sender().send(close_queue());
}
void bank_state_machine::run() {
    TICK();
    m_fpState = &bank_state_machine::wait_for_action;
    try {
        for (;;) {
            (this->*m_fpState)();
            yield();
        }
    } catch (close_queue const&) {
        INFO("catch close_queue");
    } catch (...) {
        ERR("catch ...");
    }
}
sender bank_state_machine::get_sender() {
    TICK();
    return m_receiverIncoming;
}

//Listing C.9 The user-interface state machine
ui_state_machine::ui_state_machine() {
    m_fpState = nullptr;
}
void ui_state_machine::done() {
    TICK();
    get_sender().send(close_queue());
}
void ui_state_machine::wait_for_show() {
    m_receiverIncoming.wait().handle<issue_money>([&](issue_money const& msg) {
        INFO("Issuing %d", msg.amount);
    }).handle<display_insufficient_funds>([&](display_insufficient_funds const& msg) {
        INFO("Insufficient funds");
    }).handle<display_enter_pin>([&](display_enter_pin const& msg) {
        INFO("Please enter your PIN(0-9), OK(#), BackSpace(B)");
    }).handle<display_enter_card>([&](display_enter_card const& msg) {
        INFO("Please enter your card(I)");
    }).handle<display_balance>([&](display_balance const& msg) {
        INFO("The balance of your account is %d", msg.amount);
    }).handle<display_withdrawal_options>([&](display_withdrawal_options const& msg) {
        INFO("Withdraw ? (W)");
        INFO("Withdraw 50? (w)");
        INFO("Display Balance? (b)");
        INFO("Cancel? (c)");
    }).handle<display_withdrawal_amount>([&](display_withdrawal_amount const& msg) {
        INFO("Please enter your withdrawal amount(0-9), OK(#), BackSpace(B)");
    }).handle<display_withdrawal_cancelled>([&](display_withdrawal_cancelled const& msg) {
        INFO("Withdraw cancelled");
    }).handle<display_pin_incorrect_message>([&](display_pin_incorrect_message const& msg) {
        INFO("PIN incorrect");
    }).handle<eject_card>([&](eject_card const& msg) {
        INFO("Ejecting card");
    });
}
void ui_state_machine::run() {
    TICK();
    m_fpState = &ui_state_machine::wait_for_show;
    try {
        for (;;) {
            (this->*m_fpState)();
            yield();
        }
    } catch (close_queue&) {
        INFO("catch close_queue");
    } catch (...) {
        ERR("catch ...");
    }
}
sender ui_state_machine::get_sender() {
    TICK();
    return m_receiverIncoming;
}

//Listing C.10 The driving code
void test_atm_messaging() {
    TICK();

    bank_state_machine bank;
    ui_state_machine ui;
    atm_state_machine atm(bank.get_sender(), ui.get_sender());

    thread bank_thread(&bank_state_machine::run, &bank);
    thread ui_thread(&ui_state_machine::run, &ui);
    thread atm_thread(&atm_state_machine::run, &atm);

    sender incoming_sender(atm.get_sender());
    bool quit_pressed = false;

    while (!quit_pressed) {
        char c = getchar();
        switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            incoming_sender.send(digit_pressed(c));
            break;
        case 'b':
            incoming_sender.send(balance_pressed());
            break;
        case 'B':
            incoming_sender.send(clear_last_pressed());
            break;
        case 'w':
            incoming_sender.send(withdraw_processed("acc1234", 50));
            break;
        case 'W':
            incoming_sender.send(withdraw_amount_processed("acc1234"));
            break;
        case 'c':
            incoming_sender.send(cancel_pressed());
            break;
        case '#':
            incoming_sender.send(ok_pressed());
            break;
        case 'q':
            quit_pressed = true;
            break;
        case 'i':
            incoming_sender.send(card_inserted("acc1234"));
            break;
        }
        yield();
    }

    bank.done();
    atm.done();
    ui.done();

    atm_thread.join();
    bank_thread.join();
    ui_thread.join();
}

void test_functional_pointer_1() {
    TICK();
    sender bank;
    sender ui;
    atm_state_machine atm(bank, ui);
    atm.test_functional_pointer();
}

void hello() {
    TICK();
    DEBUG("hello world");
}
void test_functional_pointer_2() {
    TICK();
    void(*fp)();
    fp = &messaging::hello;
    (*fp)();
}

}//namespace messaging

