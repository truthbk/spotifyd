#ifndef _RUNNABLE_H
#define _RUNNABLE_H

#include <thread>
#include <atomic>
#include <cstdlib>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

class Runnable {
    //how do we handle copies?
    Runnable(Runnable const&) = delete;
    Runnable& operator =(Runnable const&) = delete;

    public:
        Runnable()
            : m_done(true)
            , m_running(false)
            , m_t()
        {
        }
        virtual ~Runnable()
        {
            stop();
        }

        void start() 
        {
            if(m_running)
            {
                return;
            }
            m_done = false;
            m_t = std::thread(&Runnable::run, this);
            m_running = true;
        }

        void stop() 
        {
            if(m_running)
            {
                return;
            }
            m_done = true;
            m_t.join();
            m_running = false;
        }

    protected:
        virtual void run() = 0;
        std::atomic<bool> m_done;

    private:
        std::atomic<bool> m_running;
        std::thread m_t;
};

#endif
