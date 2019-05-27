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
struct empty_stack : exception {
    const char* what() const throw() {
        return "empty_stack";
    }
};
template<typename T>
class thread_safe_stack {
private:
    stack<T>            m_stackData;
    mutable mutex       m_mutex;

public:
    thread_safe_stack() {}
    thread_safe_stack(const thread_safe_stack &other) : m_mutex(mutex()) {
        //TICK();
        lock_guard<mutex> lock(other.m_mutex);
        m_stackData = other.m_stackData;
    }
    thread_safe_stack& operator=(const thread_safe_stack &) = delete;
    void push(T new_value) {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        m_stackData.push(move(new_value));
    }
    shared_ptr<T> pop() {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_stackData.empty()) {
#if 0
            throw empty_stack();
#else
            return nullptr;
#endif
        }
        shared_ptr<T> const ptrResult(make_shared<T>(move(m_stackData.top())));
        m_stackData.pop();
        return ptrResult;
    }
    void pop(T &value) {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_stackData.empty()) {
            throw empty_stack();
        }
        value = move(m_stackData.top());
        m_stackData.pop();
    }
    bool empty() const {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        return m_stackData.empty();
    }
};
void test_lock_based_thread_safe_stack();

//6.2.2 A thread-safe queue using locks and condition variables
//Listing 6.2 The full class definition for a thread-safe queue using condition variables
template<typename T>
class threadsafe_queue {
private:
    mutable mutex       m_mutex;
    std::queue<T>       m_queueData;
    condition_variable  m_cvData;

public:
    threadsafe_queue() {}
    void push(T new_value) {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        m_queueData.push(move(new_value));
        m_cvData.notify_one();
    }
    void wait_and_pop(T &value) {
        //TICK();
        unique_lock<mutex> lock(m_mutex);
#if 0
        m_cvData.wait(lock, [this] {return !m_queueData.empty(); });
#else
        if (!m_cvData.wait_for(lock, milliseconds(10), [this] {return !m_queueData.empty(); })) {
            WARN("void wait_and_pop(T&), wait_for=false");
            return;
        }
#endif
        value = move(m_queueData.front());
        m_queueData.pop();
    }
    shared_ptr<T> wait_and_pop() {
        //TICK();
        unique_lock<mutex> lock(m_mutex);
#if 0
        m_cvData.wait(lock, [this] {return !m_queueData.empty(); });
#else
        if (!m_cvData.wait_for(lock, milliseconds(10), [this] {return !m_queueData.empty(); })) {
            WARN("shared_ptr<T> wait_and_pop(), wait_for=false");
            return nullptr;
        }
#endif
        shared_ptr<T> ptrRes(make_shared<T>(move(m_queueData.front())));
        m_queueData.pop();
        return ptrRes;
    }
    bool try_pop(T &value) {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_queueData.empty()) {
            return false;
        }
        //INFO("pool task");
        value = move(m_queueData.front());
        m_queueData.pop();
        return true;
    }
    shared_ptr<T> try_pop() {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_queueData.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> ptrRes(make_shared<T>(move(m_queueData.front())));
        m_queueData.pop();
        return ptrRes;
    }
    bool empty() const {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        return m_queueData.empty();
    }
};

void test_threadsafe_queue();

//Listing 6.3 A thread-safe queue holding shared_ptr<> instances
template<typename T>
class threadsafe_queue_shared_ptr {
private:
    mutable mutex               m_mutex;
    std::queue<shared_ptr<T>>   m_queueData;
    condition_variable          m_cvData;

public:
    threadsafe_queue_shared_ptr() {}
    void wait_and_pop(T &value) {
        //TICK();
        unique_lock<mutex> lock(m_mutex);
#if 0
        m_cvData.wait(lock, [this] { return !m_queueData.empty(); });
#else
        if (!m_cvData.wait_for(lock, milliseconds(10), [this] {return !m_queueData.empty(); })) {
            WARN("void wait_and_pop(T&), wait_for()=false");
            return;
        }
#endif
        value = move(*m_queueData.front());
        m_queueData.pop();
    }
    bool try_pop(T &value) {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_queueData.empty()) {
            return false;
        }
        value = move(*m_queueData.front());
        m_queueData.pop();
        return true;
    }
    shared_ptr<T> wait_and_pop() {
        //TICK();
        unique_lock<mutex> lock(m_mutex);
#if 0
        m_cvData.wait(lock, [this] {return !m_queueData.empty(); });
#else
        if (!m_cvData.wait_for(lock, milliseconds(10), [this] {return !m_queueData.empty(); })) {
            WARN("shared_ptr<T> wait_and_pop(), wait_for()=false");
            return nullptr;
        }
#endif
        shared_ptr<T> ptrRes = move(m_queueData.front());
        m_queueData.pop();
        return ptrRes;
    }
    shared_ptr<T> try_pop() {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        if (m_queueData.empty()) {
            return shared_ptr<T>();
        }
        shared_ptr<T> ptrRes = move(m_queueData.front());
        m_queueData.pop();
        return ptrRes;
    }
    void push(T const new_value) {
        //TICK();
        shared_ptr<T> ptrNewData(make_shared<T>(move(new_value)));
        lock_guard<mutex> lock(m_mutex);
        m_queueData.push(ptrNewData);
        m_cvData.notify_one();
    }
    bool empty() const {
        //TICK();
        lock_guard<mutex> lock(m_mutex);
        return m_queueData.empty();
    }
};
void test_threadsafe_queue_shared_ptr();

//6.2.3 A thread-safe queue using fine-grained locks and condition variables
//Listing 6.4 A simple single-threaded queue implementation
template<typename T>
class queue {
private:
    struct node {
        T                   data;
        unique_ptr<node>    next;
        explicit node(T data_) : data(move(data_)) {}
    };
    unique_ptr<node>        m_ptrHead;
    node*                   m_pTail;

public:
    queue() : m_ptrHead(nullptr), m_pTail(nullptr) {}
    queue(queue const &other) = delete;
    queue& operator=(queue const &other) = delete;
    shared_ptr<T> try_pop() {
        TICK();
        if (!m_ptrHead) {
            return make_shared<T>();
        }
        shared_ptr<T> const ptrRes(make_shared<T>(move(m_ptrHead->data)));
        unique_ptr<node> const ptrOldHead = move(m_ptrHead);
        m_ptrHead = move(ptrOldHead->next);
        return ptrRes;
    }
    void push(T new_value) {
        unique_ptr<node> ptrNewNode(new node(move(new_value)));
        node* const pNewTail = ptrNewNode.get();
        if (m_pTail) {
            m_pTail->next = move(ptrNewNode);
        } else {
            m_ptrHead = move(ptrNewNode);
        }
        m_pTail = pNewTail;
    }
};
void test_queue();

//Listing 6.5 A simple queue with a dummy node
template<typename T>
class dummy_queue {
private:
    struct node {
        shared_ptr<T>       data;
        unique_ptr<node>    next;
    };
    unique_ptr<node>        m_ptrHead;
    node                    *m_pTail;

public:
    dummy_queue() : m_ptrHead(new node), m_pTail(m_ptrHead.get()) {}
    dummy_queue(dummy_queue const &other) = delete;
    dummy_queue& operator=(dummy_queue const &other) = delete;
    shared_ptr<T> try_pop() {
        TICK();
        if (m_ptrHead.get() == m_pTail) {
            return make_shared<T>();
        }
        shared_ptr<T> const ptrRes(m_ptrHead->data);
        unique_ptr<node> ptrOldHead = move(m_ptrHead);
        m_ptrHead = move(ptrOldHead->next);
        return ptrRes;
    }
    void push(T new_value) {
        TICK();
        shared_ptr<T> ptrNewData(make_shared<T>(move(new_value)));
        unique_ptr<node> ptrNewNode(new node);
        m_pTail->data = ptrNewData;
        node* const pNewTail = ptrNewNode.get();
        m_pTail->next = move(ptrNewNode);
        m_pTail = pNewTail;
    }
};
void test_dummy_queue();

//Listing 6.6 A thread-safe queue with fine-grained locking
template<typename T>
class threadsafe_queue_fine_grained {
private:
    struct node {
        shared_ptr<T>       data;
        unique_ptr<node>    next;
    };
    mutex                   m_mutexHead;
    unique_ptr<node>        m_ptrHead;
    mutex                   m_mutexTail;
    node                    *m_pTail;

    node* get_tail() {
        TICK();
        lock_guard<mutex>   lockTail(m_mutexTail);
        return m_pTail;
    }
#if 0//thead-safe
    unique_ptr<node> pop_head() {
        TICK();
        lock_guard<mutex>   lockHead(m_mutexHead);
        if (m_ptrHead.get() == m_pTail) {
            return nullptr;
        }
        unique_ptr<node>    ptrOldHead = move(m_ptrHead);
        m_ptrHead = move(ptrOldHead->next);
        return ptrOldHead;
    }
#else//thread-unsafe
    unique_ptr<node> pop_head() {//This is a broken implementation
        TICK();
        node const          *pOldTail = get_tail();//Get old tail value outside lock on head_mutex
        lock_guard<mutex>   lockHead(m_mutexHead);
        if (m_ptrHead.get() == pOldTail) {
            return nullptr;
        }
        unique_ptr<node>    ptrOldHead = move(m_ptrHead);
        m_ptrHead = move(ptrOldHead->next);
        return ptrOldHead;
    }
#endif

public:
    threadsafe_queue_fine_grained() : m_ptrHead(new node), m_pTail(m_ptrHead.get()) {}
    threadsafe_queue_fine_grained(threadsafe_queue_fine_grained const &other) = delete;
    threadsafe_queue_fine_grained& operator=(threadsafe_queue_fine_grained const &other) = delete;
    shared_ptr<T> try_pop() {
        TICK();
        unique_ptr<node>    ptrOldHead = pop_head();
        return ptrOldHead ? ptrOldHead->data : make_shared<T>();
    }
    void push(T new_value) {
        TICK();
        shared_ptr<T>       ptrNewData(make_shared<T>(move(new_value)));
        unique_ptr<node>    ptrNewNode(new node);
        node* const         ptrNewTail = ptrNewNode.get();//Pay more attention to the position of the key-word 'const'.

        lock_guard<mutex>   lockTail(m_mutexTail);
        m_pTail->data = ptrNewData;
        m_pTail->next = move(ptrNewNode);
        m_pTail = ptrNewTail;
    }
};

void test_threadsafe_queue_fine_grained();

//Listing 6.7 A thread-safe queue with locking and waiting: internals and interface
template<typename T>
class threadsafe_waiting_queue {
private:
    struct node {
        shared_ptr<T>       data;
        unique_ptr<node>    next;
    };
    mutex                   m_mutexHead;
    unique_ptr<node>        m_ptrHead;
    mutex                   m_mutexTail;
    node                    *pTail;
    condition_variable      m_cvData;

    node* get_tail() {
        TICK();
        lock_guard<mutex>   lockTail(m_mutexTail);
        return pTail;
    }
    unique_ptr<node> pop_head() {
        TICK();
        unique_ptr<node>    ptrOldHead = move(m_ptrHead);
        m_ptrHead = move(ptrOldHead->next);
        return ptrOldHead;
    }
    unique_lock<mutex> wait_for_data() {
        TICK();
        unique_lock<mutex>  lockHead(m_mutexHead);
        m_cvData.wait(lockHead, [&] {return m_ptrHead.get() != get_tail(); });
        return move(lockHead);
    }
    unique_ptr<node> wait_pop_head() {
        TICK();
        unique_lock<mutex>  lockHead(wait_for_data());
        return pop_head();
    }
    unique_ptr<node> wait_pop_head(T &value) {
        TICK();
        unique_lock<mutex>  lockHead(wait_for_data());
        value = move(*m_ptrHead->data);
        return pop_head();
    }
    unique_ptr<node> try_pop_head() {
        TICK();
        lock_guard<mutex>   lockHead(m_mutexHead);
        if (m_ptrHead.get() == pTail) {
            return unique_ptr<node>();
        }
        return pop_head();
    }
    unique_ptr<node> try_pop_head(T& value) {
        TICK();
        lock_guard<mutex>   lockHead(m_mutexHead);
        if (m_ptrHead.get() == pTail) {
            return unique_ptr<node>();
        }
        value = move(*m_ptrHead->data);
        return pop_head();
    }

public:
    threadsafe_waiting_queue() :m_ptrHead(new node), pTail(m_ptrHead.get()) {}
    threadsafe_waiting_queue(const threadsafe_waiting_queue &other) = delete;
    threadsafe_waiting_queue& operator=(const threadsafe_waiting_queue &other) = delete;

    shared_ptr<T> wait_and_pop() {
        TICK();
        unique_ptr<node> const ptrOldHead = wait_pop_head();
        return ptrOldHead->data;
    }
    void wait_and_pop(T& value) {
        TICK();
        unique_ptr<node> const ptrOldHead = wait_pop_head(value);
    }
    shared_ptr<T> try_pop() {
        TICK();
#if 0
        unique_lock<mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&] {return head.get() != tail; });

        unique_ptr<node> old_head = move(head);
        if (!old_head) {
            return make_shared<T>();
        }
        head = move(old_head->next);
        return old_head->data;
#else
        unique_ptr<node> ptrOldHead = try_pop_head();
        return ptrOldHead ? ptrOldHead->data : make_shared<T>();
#endif
    }
    bool try_pop(T& value) {
        TICK();
        unique_ptr<node> const ptrOldHead = try_pop_head(value);
        return ptrOldHead ? true : false;
    }
    void push(T new_value) {
        TICK();
        shared_ptr<T>           ptrNewData(make_shared<T>(move(new_value)));
        unique_ptr<node>        ptrNewNode(new node);
        {
            lock_guard<mutex>   lockTail(m_mutexTail);
            pTail->data = ptrNewData;
            node* const new_tail = ptrNewNode.get();
            pTail->next = move(ptrNewNode);
            pTail = new_tail;
        }
        m_cvData.notify_one();
    }
    bool empty() {
        TICK();
        lock_guard<mutex>       lockHead(m_mutexHead);
        return m_ptrHead.get() == pTail;
    }
    void loop() {
        TICK();
        while (true) {
            WARN("loop...");
            if (empty()) {
                common_fun::sleep(THOUSAND);
                continue;
            }
            m_cvData.notify_one();
            yield();
        }
    }
};

void test_threadsafe_waiting_queue();

//6.3 Designing more complex lock-based data structures
//6.3.1 Writing a thread-safe lookup table using locks
//Listing 6.11 A thread-safe lookup table
#define USE_BOOST_SHARED_LOCK 0
template<typename Key, typename Value, typename Hash = hash<Key>>
class threadsafe_lookup_table {
private:
    class bucket_type {
    private:
        typedef pair<Key, Value>                    BUCKET_VALUE;
        typedef list<BUCKET_VALUE>                  LST_BUCKET_DATA;
#if 1
        typedef typename LST_BUCKET_DATA::iterator  BUCKET_ITERATOR;
#else
        typedef typename LST_BUCKET_DATA::const_iterator BUCKET_ITERATOR;
#endif
        //data: 'mutable' should be added or the return type of 'find' is LST_BUCKET_DATA::const_iterator.
        mutable LST_BUCKET_DATA     m_bucketData;
#if USE_BOOST_SHARED_LOCK
        mutable boost::shared_mutex m_mutex;
#else
        mutable mutex               m_mutex;
#endif
        BUCKET_ITERATOR find(Key const& key) const {
            TICK();
            return find_if(m_bucketData.begin(), m_bucketData.end(),
                [&](BUCKET_VALUE const& item) {return item.first == key; });
        }

    public:
        Value get(Key const& key, Value const& default_value)const {
            TICK();
#if USE_BOOST_SHARED_LOCK
            shared_lock<boost::shared_mutex> lock(m_mutex);
#else
            unique_lock<mutex> lock(m_mutex);
#endif
            BUCKET_ITERATOR const posFind = find(key);
            return (posFind == m_bucketData.end()) ? default_value : posFind->second;
        }
        bool insert(Key const& key, Value const& value) {
            TICK();
#if USE_BOOST_SHARED_LOCK
            unique_lock<boost::shared_mutex> lock(m_mutex);
#else
            unique_lock<mutex> lock(m_mutex);
#endif
            BUCKET_ITERATOR const posFind = find(key);
            if (posFind == m_bucketData.end()) {
                m_bucketData.push_back(BUCKET_VALUE(key, value));
                return true;
            } else {
                posFind->second = value;
                return false;
            }
        }
        bool remove(Key const& key) {
            TICK();
#if USE_BOOST_SHARED_LOCK
            unique_lock<boost::shared_mutex> lock(m_mutex);
#else
            unique_lock<mutex> lock(m_mutex);
#endif
            BUCKET_ITERATOR const posFind = find(key);
            if (posFind != m_bucketData.end()) {
                m_bucketData.erase(posFind);
                return true;
            } else {
                return false;
            }
        }
        template<typename Key, typename Value, typename Hash>
        friend class threadsafe_lookup_table;
    };
    vector<unique_ptr<bucket_type>> m_vctBuckets;
    Hash                            m_hasher;

    bucket_type& get_bucket(Key const& key) const {
        TICK();
        size_t const uBucketIndex = m_hasher(key) % m_vctBuckets.size();
        return *m_vctBuckets[uBucketIndex];
    }

public:
    typedef Key     key_type;
    typedef Value   mapped_type;
    typedef Hash    hash_type;

    explicit threadsafe_lookup_table(unsigned num_buckets = 19, Hash const& hasher_ = Hash()) :
        m_vctBuckets(num_buckets), m_hasher(hasher_) {
        //TICK();
        for (unsigned i = 0; i < num_buckets; ++i) {
            m_vctBuckets[i].reset(new bucket_type);
        }
    }

    threadsafe_lookup_table(threadsafe_lookup_table const& other) = delete;
    threadsafe_lookup_table& operator=(threadsafe_lookup_table const& other) = delete;

    Value get(Key const& key, Value const& default_value = Value()) const {
        TICK();
        return get_bucket(key).get(key, default_value);
    }
    bool insert(Key const& key, Value const& value) {
        TICK();
        return get_bucket(key).insert(key, value);
    }
    bool remove(Key const& key) {
        TICK();
        return get_bucket(key).remove(key);
    }
    map<Key, Value> get_map() const {
        TICK();
#if USE_BOOST_SHARED_LOCK
        vector<unique_lock<boost::shared_mutex>>    vctLocks;
#else
        vector<unique_lock<mutex>>                  vctLocks;
#endif
        for (unsigned i = 0; i < m_vctBuckets.size(); ++i) {
#if USE_BOOST_SHARED_LOCK
            vctLocks.push_back(unique_lock<boost::shared_mutex>(m_vctBuckets[i].m_mutex));
#else
            vctLocks.push_back(unique_lock<mutex>(m_vctBuckets[i]->m_mutex));
#endif
        }
        map<Key, Value> mapRes;
        for (unsigned i = 0; i < m_vctBuckets.size(); ++i) {
            for (bucket_type::BUCKET_ITERATOR pos = m_vctBuckets[i]->m_bucketData.begin();
                pos != m_vctBuckets[i]->m_bucketData.end(); ++pos) {
                mapRes.insert(*pos);
            }
        }
        return mapRes;
    }
};

void test_threadsafe_lookup_table();

//6.3.2 Writing a thread-safe list using locks
//Listing 6.13 A thread-safe list with iteration support
template<typename T>
class threadsafe_list {
    struct node {
        mutex               m;
        shared_ptr<T>       data;
        unique_ptr<node>    next;

        node() : data(nullptr), next() {}
        explicit node(T const& value) : data(make_shared<T>(value)), next(nullptr) {}
    };
    node                    m_head;

public:
    threadsafe_list() {}
    ~threadsafe_list() {
        //TICK();
        remove_if([](T const &) {return true; });
    }
    threadsafe_list(threadsafe_list const& other) = delete;
    threadsafe_list& operator=(threadsafe_list const& other) = delete;

    void push_front(T const& value) {
        TICK();
        unique_ptr<node> ptrNewNode(new node(value));
        lock_guard<mutex> lock(m_head.m);
        ptrNewNode->next = move(m_head.next);
        m_head.next = move(ptrNewNode);
    }
    template<typename Function>
    void for_each(Function f) {
        TICK();
        node* pCurrent = &m_head;
        unique_lock<mutex> lockHead(m_head.m);
        while (node* const pNext = pCurrent->next.get()) {
            unique_lock<mutex> lockNext(pNext->m);
            lockHead.unlock();
            f(*pNext->data);
            pCurrent = pNext;
            lockHead = move(lockNext);
        }
    }
    template<typename Predicate>
    shared_ptr<T> find_first_if(Predicate p) {
        TICK();
        node* pCurrent = &m_head;
        unique_lock<mutex> lockHead(m_head.m);
        while (node* const pNext = pCurrent->next.get()) {
            unique_lock<mutex> lockNext(pNext->m);
            lockHead.unlock();
            if (p(*pNext->data)) {
                return pNext->data;
            }
            pCurrent = pNext;
            lockHead = move(lockNext);
        }
        return make_shared<T>();
    }
    template<typename Predicate>
    void remove_if(Predicate p) {
        //TICK();
        node* pCurrent = &m_head;
        unique_lock<mutex> lockHead(m_head.m);
        while (node* const pNext = pCurrent->next.get()) {
            unique_lock<mutex> lockNext(pNext->m);
            if (p(*(pNext->data))) {
                unique_ptr<node> ptrOldNext = move(pCurrent->next);
                pCurrent->next = move(pNext->next);
                lockNext.unlock();
            } else {
                lockHead.unlock();
                pCurrent = pNext;
                lockHead = move(lockNext);
            }
        }
    }
};

void test_threadsafe_list();

}//namespace lock_based_conc_data

#endif  //LOCK_BASED_CONCURRENT_DATA_STRUCTURES_H

