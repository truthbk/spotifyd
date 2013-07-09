#ifndef _TICKING_STORE_H
#define _TICKING_STORE_H

#include <vector>
#include <ctime>

#include "lockable.h"
#include "runnable.h"

#define STORAGE_TICK_MS 200

template <typename T>
class TickingStore
    : private Lockable
    , public  Runnable 
{
    public:
        typedef bool (* ready)(T t);
        typedef bool (* action)(T t);
    private:
        std::vector<T> store;
        ready          m_ready_fn;
        action         m_action_fn;
        timespec       m_to;
    protected:
        void run() {
            while(!m_done) {
                lock();
                while(store.empty()) {
                    cond_timedwait(&m_to);
                }
                typename std::vector<T>::iterator it;
                for (it=store.begin() ; it<store.end() ; ) {
                    if(m_ready_fn(*it) && m_action_fn(*it)){
                        it = store.erase(it);
                    } else {
                        it++;
                    }
                }
                unlock();
            }
        }
    public:
        TickingStore(size_t ms_to, ready f_r, action f_a) 
            : Lockable() 
            , Runnable()
            , m_ready_fn(f_r)
            , m_action_fn(f_a)
        {
            m_to.tv_sec = ms_to/1000;
            m_to.tv_nsec = (ms_to % 1000)*1000*1000;
        }
        void push_back(T& t) {
            store.push_back(t);
        }
};

#endif
