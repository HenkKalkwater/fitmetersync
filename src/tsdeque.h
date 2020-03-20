#ifndef TSDEQUE_H
#define TSDEQUE_H

#include <3ds.h>
#include <deque>

template<class T>
class TSDeque {
public:
    TSDeque() {
        LightLock_Init(&lock);
        LightEvent_Init(&event, RESET_ONESHOT);
    }
    
    bool empty() {
        LightLock_Lock(&lock);
        bool res = deque.empty();
        LightLock_Unlock(&lock);
        return res;
    }
    
    T pop_front() {
        LightLock_Lock(&lock);
        while(deque.empty()) {
            LightLock_Unlock(&lock);
            LightEvent_Wait(&event);
            LightLock_Lock(&lock);
        }
        T t = deque.front();
        deque.pop_front();
        LightLock_Unlock(&lock);
        return t;
    }
    
    void push_back(const T &t) {
        LightLock_Lock(&lock);
        deque.push_back(t);
        LightLock_Unlock(&lock);
        LightEvent_Signal(&event);
    }
private:
    std::deque<T> deque;
    LightLock lock;
    LightEvent event;
};  

#endif // TSDEQUE_H
