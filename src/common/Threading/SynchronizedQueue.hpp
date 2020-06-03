#ifndef TRINITY_SHARED_SYNCHRONIZED_QUEUE_HPP
#define TRINITY_SHARED_SYNCHRONIZED_QUEUE_HPP

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace Trinity {

template <typename T>
class SynchronizedQueue
{
    typedef std::deque<T> QueueType;

public:
    SynchronizedQueue()
        : frozen_(false)
    { }

    bool frozen() const
    {
        return frozen_.load(std::memory_order_acquire);
    }

    void freeze()
    {
        frozen_.store(true, std::memory_order_release);
        waitCond_.notify_all();
    }

    bool push(T data)
    {
        if (frozen())
            return false;

        {
            std::lock_guard<std::mutex> guard(mutex_);
            queue_.push_back(std::move(data));
        }

        waitCond_.notify_one();
        return true;
    }

    bool pop(T &data)
    {
        if (frozen())
            return false;

        std::unique_lock<std::mutex> guard(mutex_);
        waitCond_.wait(guard, [this] { return frozen() || !queue_.empty(); });

        if (frozen())
            return false;

        data = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    bool try_pop(T &data)
    {
        std::lock_guard<std::mutex> guard(mutex_);

        if (queue_.empty())
            return false;

        data = queue_.front();
        queue_.pop_front();

        return true;
    }

private:
    QueueType queue_;
    std::atomic<bool> frozen_;
    std::mutex mutex_;
    std::condition_variable waitCond_;
};

} // namespace Trinity

#endif // TRINITY_SHARED_SYNCHRONIZED_QUEUE_HPP
