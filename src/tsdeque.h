#ifndef TSDEQUE_H
#define TSDEQUE_H

#include "platform/condvar.h"
#include "platform/lock.h"
#include "platform/mutex.h"
#include <deque>

namespace fms {

template<class T>
class TSDeque {
public:
    TSDeque() {}
    
    bool empty() {
        platform::Lock lock(m_mutex) ;
        bool res = deque.empty();
        return res;
    }
    
    T pop_front() {
        platform::Lock lock(m_mutex) ;
        while(deque.empty()) {
            m_condition.wait(lock);
        }
        T t = deque.front();
        deque.pop_front();;
        return t;
    }
    
    void push_back(const T &t) {
        platform::Lock lock(m_mutex) ;
        deque.push_back(t);
        m_condition.wakeup();
    }
private:
    std::deque<T> deque;
    platform::Mutex m_mutex;
    platform::CondVar m_condition;
};  

} // NS fms

#endif // TSDEQUE_H
