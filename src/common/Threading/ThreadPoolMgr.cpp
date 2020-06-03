#include "ThreadPoolMgr.hpp"

namespace Trinity {

ThreadPoolMgr::ThreadPoolMgr()
    : requestCount_(0)
{ }

void ThreadPoolMgr::start(std::size_t numThreads)
{
    threads_.reserve(numThreads);
    for (std::size_t i = 0; i < numThreads; ++i)
        threads_.emplace_back(&ThreadPoolMgr::threadFunc, this);
}

void ThreadPoolMgr::stop()
{
    if (!queue_.frozen()) {
        queue_.freeze();
        for (auto &t : threads_)
            t.join();
    }
}

void ThreadPoolMgr::wait()
{
    GuardType guard(lock_);
    waitCond_.wait(guard, [this] { return requestCount_ == 0; });
}

void ThreadPoolMgr::threadFunc()
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
