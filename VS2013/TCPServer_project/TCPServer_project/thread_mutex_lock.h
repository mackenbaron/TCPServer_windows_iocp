#ifndef THREAD_MUTEX_H_
#define THREAD_MUTEX_H_


#if defined(WIN32) || defined(WIN64) || defined(_WIN32_WCE)
#include <Windows.h>

class thread_mutex
{
    friend class thread_mutex_lock;
public:
    inline thread_mutex()
    {
        InitializeCriticalSection(&m_mutex);
    }

    inline ~thread_mutex()
    {
        DeleteCriticalSection(&m_mutex);
    }

private:
    CRITICAL_SECTION m_mutex;
};


class thread_mutex_lock
{
public:
    inline thread_mutex_lock(thread_mutex& mutex) : m_mutex(&(mutex.m_mutex))
    {
        EnterCriticalSection(m_mutex);
    }

    inline ~thread_mutex_lock()
    {
        LeaveCriticalSection(m_mutex);
    }

private:
    CRITICAL_SECTION* m_mutex;
};

#else

#include <stdio.h>
#include <pthread.h>


class thread_mutex
{
    friend class thread_mutex_lock;
public:
    inline thread_mutex() :m_init(false)
    {
        if(pthread_mutex_init(&m_mutex, NULL) != 0)  perror("pthread_mutex_init() fail");
        else  m_init = true;
    }

    inline ~thread_mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

private:
    pthread_mutex_t m_mutex;
    bool m_init;
};


class thread_mutex_lock
{
public:
    inline thread_mutex_lock(thread_mutex& mutex) : m_mutex(&(mutex.m_mutex)), m_locked(false)
    {
        if(!(mutex.m_init))  return;

        if(pthread_mutex_lock(m_mutex) != 0)  perror("pthread_mutex_lock() fail");
        else  m_locked = true;
    }

    inline ~thread_mutex_lock()
    {
        pthread_mutex_unlock(m_mutex);
    }

    inline bool IsLocked()
    {
        return m_locked;
    }

private:
    pthread_mutex_t* m_mutex;
    bool m_locked;
};

#endif

#endif
