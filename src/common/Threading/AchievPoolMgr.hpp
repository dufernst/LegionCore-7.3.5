#ifndef TRINITY_SHARED_ACHIEV_POOL_MGR_HPP
#define TRINITY_SHARED_ACHIEV_POOL_MGR_HPP

#include "SynchronizedQueue.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include <cstddef>

#include <cds/init.h>
#include <cds/gc/hp.h>

namespace Trinity {

class AchievPoolMgr final
{
    typedef std::mutex LockType;

    typedef std::unique_lock<LockType> GuardType;

    typedef std::function<void()> FunctorType;

    typedef Trinity::SynchronizedQueue<FunctorType> QueueType;

private:
    AchievPoolMgr();

public:
    static AchievPoolMgr * instance()
    {
        static AchievPoolMgr mgr;
        return &mgr;
    }

    void start(std::size_t numThreads);

    void stop();

    template <typename RequestType>
    void schedule(RequestType request)
    {
        GuardType g(lock_);
        if (queue_.push(std::move(request)))
            ++requestCount_;
    }

    void wait();

private:
    void threadFunc();

    QueueType queue_;

    std::vector<std::thread> threads_;

    int requestCount_;

    LockType lock_;

    std::condition_variable waitCond_;
};

} // namespace Trinity

#define sAchievPoolMgr Trinity::AchievPoolMgr::instance()

#endif // TRINITY_SHARED_THREAD_POOL_MGR_HPP
