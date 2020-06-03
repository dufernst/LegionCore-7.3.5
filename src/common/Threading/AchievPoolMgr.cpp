#include "AchievPoolMgr.hpp"

namespace Trinity {

AchievPoolMgr::AchievPoolMgr()
    : requestCount_(0)
{ }

void AchievPoolMgr::start(std::size_t numThreads)
{
    threads_.reserve(numThreads);
    for (std::size_t i = 0; i < numThreads; ++i)
        threads_.emplace_back(&AchievPoolMgr::threadFunc, this);
}

void AchievPoolMgr::stop()
{
    if (!queue_.frozen()) {
        queue_.freeze();
        for (auto &t : threads_)
            t.join();
    }
}

void AchievPoolMgr::wait()
{
    GuardType guard(lock_);
    waitCond_.wait(guard, [this] { return requestCount_ == 0; });
}

void AchievPoolMgr::threadFunc()
{
    cds::threading::Manager::attachThread();
    FunctorType f;
    while (queue_.pop(f)) {
        f();
        GuardType g(lock_);
        if (--requestCount_ == 0)
            waitCond_.notify_all();
    }
    cds::threading::Manager::detachThread();
}

} // namespace Trinity
