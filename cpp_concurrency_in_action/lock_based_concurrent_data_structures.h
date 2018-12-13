///    Copyright (C) 2018 DG.C, DGCHOW, deguangchow
///        deguangchow@qq.com
///
///    \brief    chapter6: Designing lock-based concurrent data structures
///
///    \author   deguangchow
///    \version  1.0
///    \2018/12/11
#pragma once
#ifndef LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H
#define LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H

namespace lock_based_conc_data {

//6.1 What does it mean to design for concurrency?
//6.11 Guidelines for designing data structures for concurrency

//6.2 Lock-based concurrent data structures
//6.2.1 A thread-safe stack using locks
//Listing 6.1 A class definition for a thread-safe stack
struct empty_stack : std::exception {
    const char* what() const throw() {
        return "empty_stack";
    }
};
template<typename T>
class thread_safe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    thread_safe_stack() {}
    thread_safe_stack(const thread_safe_stack &other) : m(std::mutex()) {
        TICK();
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    thread_safe_stack& operator=(const thread_safe_stack &) = delete;
    void push(T new_value) {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop() {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) {
            throw empty_stack();
        }
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
        data.pop();
        return res;
    }
    void pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) {
            throw empty_stack();
        }
        value = std::move(data.top());
        data.pop();
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
void lock_thread_safe_stack_write();
void lock_thread_safe_stack_read();
void lock_thread_safe_stack_test();

//6.2.2 A thread-safe queue using locks and condition variables
//Listing 6.2 The full class definition for a thread-safe queue using condition variables
template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() {}
    void push(T new_value) {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }
    void wait_and_pop(T &value) {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop() {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool try_pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return false;
        }
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop() {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        return data_queue.empty();
    }
};
void threadsafe_queue_write();
void threadsafe_queue_read();
void treadsafe_queue_test();

//Listing 6.3 A thread-safe queue holding std::shared_ptr<> instances
template<typename T>
class threadsafe_queue_shared_ptr {
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue_shared_ptr() {}
    void wait_and_pop(T &value) {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        value = std::move(*data_queue.front());
        data_queue.pop();
    }
    bool try_pop(T &value) {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return false;
        }
        value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> wait_and_pop() {
        TICK();
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this] {return !data_queue.empty(); });
        std::shared_ptr<T> res = std::move(data_queue.front());
        data_queue.pop();
        return res;
    }
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res = std::move(data_queue.front());
        data_queue.pop();
        return res;
    }
    void push(T const new_value) {
        TICK();
        std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lock(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
    bool empty() const {
        TICK();
        std::lock_guard<std::mutex> lock(mut);
        return data_queue.empty();
    }
};
void threadsafe_queue_shared_ptr_write();
void threadsafe_queue_shared_ptr_read();
void threadsafe_queue_shared_ptr_test();

//6.2.3 A thread-safe queue using fine-grained locks and condition variables
//Listing 6.4 A simple single-threaded queue implementation
template<typename T>
class queue {
private:
    struct node {
        T data;
        std::unique_ptr<node> next;
        explicit node(T data_) : data(std::move(data_)) {}
    };
    std::unique_ptr<node> head;
    node *tail;

public:
    queue() : tail(nullptr) {}
    queue(queue const &other) = delete;
    queue& operator=(queue const &other) = delete;
    std::shared_ptr<T> try_pop() {
        TICK();
        if (!head) {
            return std::make_shared<T>();
        }
        std::shared_ptr<T> const res(std::make_shared<T>(std::move(head->data)));
        std::unique_ptr<node> const old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }
    void push(T new_value) {
        std::unique_ptr<node> p(new node(std::move(new_value)));
        node* const new_tail = p.get();
        if (tail) {
            tail->next = std::move(p);
        } else {
            head = std::move(p);
        }
        tail = new_tail;
    }
};
void queue_test();

//Listing 6.5 A simple queue with a dummy node
template<typename T>
class dummy_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::unique_ptr<node> head;
    node *tail;

public:
    dummy_queue() : head(new node), tail(head.get()) {}
    dummy_queue(dummy_queue const &other) = delete;
    dummy_queue& operator=(dummy_queue const &other) = delete;
    std::shared_ptr<T> try_pop() {
        TICK();
        if (head.get() == tail) {
            return std::make_shared<T>();
        }
        std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }
    void push(T new_value) {
        TICK();
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};
void dummy_queue_write();
void dummy_queue_read();
void dummy_queue_test();

//Listing 6.6 A thread-safe queue with fine-grained locking
template<typename T>
class threadsafe_queue_fine_grained {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node *tail;

    node* get_tail() {
        TICK();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
#if 1//thead-safe
    std::unique_ptr<node> pop_head() {
        TICK();
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == tail) {
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
#else//thread-unsafe
    std::unique_ptr<node> pop_head() {//This is a broken implementation
        TICK();
        node const *old_tail = get_tail();//Get old tail value outside lock on head_mutex
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == old_tail) {
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
#endif

public:
    threadsafe_queue_fine_grained() : head(new node), tail(head.get()) {}
    threadsafe_queue_fine_grained(threadsafe_queue_fine_grained const &other) = delete;
    threadsafe_queue_fine_grained& operator=(threadsafe_queue_fine_grained const &other) = delete;
    std::shared_ptr<T> try_pop() {
        TICK();
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head->data : std::make_shared<T>();
    }
    void push(T new_value) {
        TICK();
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        node* const new_tail = p.get();//Pay more attention to the position of the key-word 'const'.

        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(p);
        tail = new_tail;
    }
};

void threadsafe_queue_fine_grained_write();
void threadsafe_queue_fine_grained_read();
void threadsafe_queue_fine_grained_test();

//Listing 6.7 A thread-safe queue with locking and waiting: internals and interface
template<typename T>
class threadsafe_waiting_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node *tail;
    std::condition_variable data_cond;

    node* get_tail() {
        TICK();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
    std::unique_ptr<node> pop_head() {
        TICK();
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
    std::unique_lock<std::mutex> wait_for_data() {
        TICK();
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&] {return head.get() != get_tail(); });
        return std::move(head_lock);
    }
    std::unique_ptr<node> wait_pop_head() {
        TICK();
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }
    std::unique_ptr<node> wait_pop_head(T &value) {
        TICK();
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value = std::move(*head->data);
        return pop_head();
    }
    std::unique_ptr<node> try_pop_head() {
        TICK();
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == tail) {
            return std::unique_ptr<node>();
        }
        return pop_head();
    }
    std::unique_ptr<node> try_pop_head(T& value) {
        TICK();
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == tail) {
            return std::unique_ptr<node>();
        }
        value = std::move(*head->data);
        return pop_head();
    }

public:
    threadsafe_waiting_queue() :head(new node), tail(head.get()) {}
    threadsafe_waiting_queue(const threadsafe_waiting_queue &other) = delete;
    threadsafe_waiting_queue& operator=(const threadsafe_waiting_queue &other) = delete;

    std::shared_ptr<T> wait_and_pop() {
        TICK();
        std::unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }
    void wait_and_pop(T& value) {
        TICK();
        std::unique_ptr<node> const old_head = wait_pop_head(value);
    }
    std::shared_ptr<T> try_pop() {
        TICK();
#if 0
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&] {return head.get() != tail; });

        std::unique_ptr<node> old_head = std::move(head);
        if (!old_head) {
            return std::make_shared<T>();
        }
        head = std::move(old_head->next);
        return old_head->data;
#else
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::make_shared<T>();
#endif
    }
    bool try_pop(T& value) {
        TICK();
        std::unique_ptr<node> const old_head = try_pop_head(value);
        return old_head ? true : false;
    }
    void push(T new_value) {
        TICK();
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            tail->data = new_data;
            node* const new_tail = p.get();
            tail->next = std::move(p);
            tail = new_tail;
        }
        data_cond.notify_one();
    }
    bool empty() {
        TICK();
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return head.get() == tail;
    }
    void loop() {
        TICK();
        while (true) {
            if (empty()) {
                common_fun::sleep(THOUSAND);
                continue;
            }
            data_cond.notify_one();
        }
    }
};
void threadsafe_waiting_queue_write();
void threadsafe_waiting_queue_read();
void threadsafe_waiting_queue_loop();
void threadsafe_waiting_queue_test();

//6.3 Designing more complex lock-based data structures
//6.3.1 Writing a thread-safe lookup table using locks
//Listing 6.11 A thread-safe lookup table
#define USE_BOOST_SHARED_LOCK 0
template<typename Key, template Value, typename Hash = std::hash<Key>>
class threadsafe_lookup_table {
private:
    class bucket_type {
    private:
        typedef std::pair<Key, Value> bucket_value;
        typedef std::list<bucket_value> bucket_data;
        typedef typename bucket_data::iterator bucket_iterator;
        bucket_data data;
#if USE_BOOST_SHARED_LOCK
        mutable boost::shared_mutex mutex;
#else
        mutable std::mutex mutex;
#endif
        bucket_iterator find_entry_for(Key const& key) const {
            TICK();
            return std::find_if(data.begin(), data.end(), [&](bucket_value const& item) {return item.first == key; });
        }

    public:
        Value value_for(Key const& key, Value const& default_value)const {
            TICK();
#if USE_BOOST_SHARED_LOCK
            std::shared_lock<boost::shared_mutex> lock(mutex);
#else
            std::unique_lock<std::mutex> lock(mutex);
#endif
            bucket_iterator const found_entry = find_entry_for(key);
            return (found_entry == data.end()) ? default_value : found_entry->second;
        }
        void add_or_update_mapping(Key const& key, Value const& value) {
            TICK();
#if USE_BOOST_SHARED_LOCK
            std::unique_lock<boost::shared_mutex> lock(mutex);
#else
            std::unique_lock<std::mutex> lock(mutex);
#endif
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry == data.end()) {
                data.push_back(bucket_value(key, value));
            } else {
                found_entry->second = value;
            }
        }
        void remove_mapping(Key const& key) {
            TICK();
#if USE_BOOST_SHARED_LOCK
            std::unique_lock<boost::shared_mutex> lock(mutex);
#else
            std::unique_lock<std::mutex> lock(mutex);
#endif
            bucket_iterator const& found_entry = find_entry_for(key);
            if (found_entry != data.end()) {
                data.erase(found_entry);
            }
        }
    };
    std::vector<std::unique_ptr<bucket_type>> buckets;
    Hash hasher;

    bucket_type& get_bucket(Key const& key) const {
        TICK();
        std::size_t const bucket_index = hasher(key) % buckets.size();
        return *buckets[bucket_index];
    }

public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_type;

    explicit threadsafe_lookup_table(unsigned num_buckets = 19, Hash const& hasher_ = Has()) :
        buckets(num_buckets), hasher(hasher_) {
        TICK();
        for (unsigned i = 0; i < num_buckets; ++i) {
            buckets[i].reset(new bucket_type);
        }
    }

    threadsafe_lookup_table(threadsafe_lookup_table const& other) = delete;
    threadsafe_lookup_table& operator=(threadsafe_lookup_table const& other) = delete;

    Value value_for(Key const& key, Value const& default_value = Value()) const {
        TICK();
        return get_bucket(key).value_for(key, default_value);
    }
    void add_or_update_mapping(Key const& key, Value const& value) {
        TICK();
    }
};


}//namespace lock_based_conc_data

#endif  //LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H

