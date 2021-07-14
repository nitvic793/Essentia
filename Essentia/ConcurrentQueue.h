#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

namespace es
{
    template<typename T>
    class ConcurrentQueue
    {
    public:
        void    Push(T&& val);
        void    Push(const T& val);
        void    Push(T& val);

        T       Pop();
        void    Pop(T& val);

        bool    IsEmpty() const;
    private:
        using UniqueLock = std::unique_lock<std::mutex>;

        std::mutex              mMutex;
        std::condition_variable mConditionVar;
        std::queue<T>           mQueue;
    };

    template<typename T>
    inline void ConcurrentQueue<T>::Push(T&& val)
    {
        {
            UniqueLock lock(mMutex);
            mQueue.push(val);
            lock.unlock();
        }

        mConditionVar.notify_one();
    }

    template<typename T>
    inline void ConcurrentQueue<T>::Push(const T& val)
    {
        {
            UniqueLock lock(mMutex);
            mQueue.push(val);
            lock.unlock();
        }

        mConditionVar.notify_one();
    }

    template<typename T>
    inline void ConcurrentQueue<T>::Push(T& val)
    {
        {
            UniqueLock lock(mMutex);
            mQueue.push(val);
            lock.unlock();
        }

        mConditionVar.notify_one();
    }

    template<typename T>
    inline T ConcurrentQueue<T>::Pop()
    {
        UniqueLock lock(mMutex);
        while (IsEmpty())
        {
            mConditionVar.wait(lock);
        }
        auto val = mQueue.front();
        mQueue.pop();
        return val;
    }

    template<typename T>
    inline void ConcurrentQueue<T>::Pop(T& val)
    {
        UniqueLock lock(mMutex);
        while (IsEmpty())
        {
            mConditionVar.wait(lock);
        }
        val = mQueue.front();
        mQueue.pop();
        return val;
    }

    template<typename T>
    inline bool ConcurrentQueue<T>::IsEmpty() const
    {
        return mQueue.empty();
    }
}