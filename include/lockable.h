#ifndef _LOCKABLE_H
#define _LOCKABLE_H

#include <cstdlib>

//are extern "C" braces needed?
#include <pthread.h>

//move elsewhere.
class Lockable {
public:
    Lockable() {
            //Init...
            pthread_mutex_init(&m_mutex, NULL);
            pthread_cond_init(&m_cond, NULL);
    }
    ~Lockable() {
            pthread_cond_destroy(&m_cond);
            pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
            pthread_mutex_unlock(&m_mutex);
    }

    void cond_signal() {
            pthread_cond_signal(&m_cond);
    }

    void cond_wait() {
            pthread_cond_wait(&m_cond, &m_mutex);
    }

    void cond_timedwait(struct timespec * ts) {
            pthread_cond_timedwait(&m_cond, &m_mutex, ts);
    }

    void unlock() {
            pthread_mutex_unlock(&m_mutex);
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

#endif
