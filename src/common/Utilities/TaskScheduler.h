/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#include "Random.h"
#include "Common.h"
#include <chrono>
#include <vector>
#include <queue>
#include <memory>
#include <set>

class TaskContext;

/// The TaskScheduler class provides the ability to schedule std::function's in the near future.
/// Use TaskScheduler::Update to update the scheduler.
/// Popular methods are:
/// * Schedule (Schedules a std::function which will be executed in the near future).
/// * Schedules an asynchronous function which will be executed at the next update tick.
/// * Cancel, Delay & Reschedule (Methods to manipulate already scheduled tasks).
/// Tasks are organized in groups (uint), multiple tasks can have the same group id,
/// you can provide a group or not, but keep in mind that you can only manipulate specific tasks through its group id!
/// Tasks callbacks use the function signature void(TaskContext) where TaskContext provides
/// access to the function schedule plan which makes it possible to repeat the task
/// with the same duration or a new one.
/// It also provides access to the repeat counter which is useful for task that repeat itself often
/// but behave different every time (spoken event dialogs for example).
class TaskScheduler
{
    friend class TaskContext;

    typedef std::chrono::steady_clock clock_t;
    typedef clock_t::time_point timepoint_t;
    typedef clock_t::duration duration_t;
    typedef std::function<void(TaskContext)> task_handler_t;
    typedef std::function<bool()> predicate_t;
    typedef std::function<void()> success_t;

    class Task
    {
        friend class TaskContext;
        friend class TaskScheduler;

        timepoint_t _end;
        duration_t _duration;
        Optional<uint32> _group;
        uint32 _repeated;
        task_handler_t _task;

    public:
        Task(timepoint_t const& end, duration_t const& duration, Optional<uint32>  group, uint32 repeated, task_handler_t  task);
        Task(timepoint_t const& end, duration_t const& duration, task_handler_t  task);

        Task(Task const&) = delete;
        Task(Task&&) = delete;
        Task& operator= (Task const&) = default;
        Task& operator= (Task&& right) = delete;
        bool operator<(Task const& other) const;
        bool operator>(Task const& other) const;
        bool operator==(Task const& other);
        bool IsInGroup(uint32 group) const;
    };

    typedef std::shared_ptr<Task> TaskContainer;

    struct Compare
    {
		bool operator()(TaskContainer const& left, TaskContainer const& right) const;
    };

    class TaskQueue
    {
        std::multiset<TaskContainer, Compare> container;

    public:
        void Push(TaskContainer&& task);
        TaskContainer Pop();
        TaskContainer const& First() const;

        void Clear();
        void RemoveIf(std::function<bool(TaskContainer const&)> const& filter);
        void ModifyIf(std::function<bool(TaskContainer const&)> const& filter);
        bool IsEmpty() const;
    };

    std::shared_ptr<TaskScheduler> self_reference;
    timepoint_t _now;
    TaskQueue _task_holder;

    typedef std::queue<std::function<void()>> AsyncHolder;

    AsyncHolder _asyncHolder;
    predicate_t _predicate;

    static bool EmptyValidator()
    {
        return true;
    }

    static void EmptyCallback()
    {
    }

public:
    TaskScheduler();

    template<typename P>
    explicit TaskScheduler(P&& predicate) : self_reference(this, [](TaskScheduler const*) { }), _now(clock_t::now()), _predicate(std::forward<P>(predicate)) { }

    TaskScheduler(TaskScheduler const&) = delete;
    TaskScheduler(TaskScheduler&&) = delete;
    TaskScheduler& operator= (TaskScheduler const&) = delete;
    TaskScheduler& operator= (TaskScheduler&&) = delete;

    template<typename P>
    TaskScheduler& SetValidator(P&& predicate)
    {
        _predicate = std::forward<P>(predicate);
        return *this;
    }

    TaskScheduler& ClearValidator();

    TaskScheduler& Update(success_t const& callback = EmptyCallback);
    TaskScheduler& Update(size_t milliseconds, success_t const& callback = EmptyCallback);

    template<class _Rep, class _Period>
    TaskScheduler& Update(std::chrono::duration<_Rep, _Period> const& difftime, success_t const& callback = EmptyCallback)
    {
        _now += difftime;
        Dispatch(callback);
        return *this;
    }

    TaskScheduler& Async(std::function<void()> const& callable);

    template<class _Rep, class _Period>
    TaskScheduler& Schedule(std::chrono::duration<_Rep, _Period> const& time, task_handler_t const& task)
    {
        return ScheduleAt(_now, time, task);
    }

    template<class _Rep, class _Period>
    TaskScheduler& Schedule(std::chrono::duration<_Rep, _Period> const& time, uint32 const group, task_handler_t const& task)
    {
        return ScheduleAt(_now, time, group, task);
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskScheduler& Schedule(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max, task_handler_t const& task)
    {
        return Schedule(RandomDurationBetween(min, max), task);
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskScheduler& Schedule(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max, uint32 const group, task_handler_t const& task)
    {
        return Schedule(RandomDurationBetween(min, max), group, task);
    }

    template<class _Rep, class _Period>
    void Schedule(std::initializer_list<std::chrono::duration<_Rep, _Period>> const& times, task_handler_t const& task)
    {
        for (auto time : times)
            ScheduleAt(_now, time, task);
    }

    template<class _Rep, class _Period>
    void Schedule(std::initializer_list<std::chrono::duration<_Rep, _Period>> const& times, uint32 const group, task_handler_t const& task)
    {
        for (auto time : times)
            ScheduleAt(_now, time, group, task);
    }

    TaskScheduler& CancelAll();
    TaskScheduler& CancelGroup(uint32 group);
    TaskScheduler& CancelGroupsOf(std::vector<uint32> const& groups);

    /// Delays all tasks with the given duration.
    template<class _Rep, class _Period>
    TaskScheduler& DelayAll(std::chrono::duration<_Rep, _Period> const& duration)
    {
        _task_holder.ModifyIf([&duration](TaskContainer const& task) -> bool
        {
            task->_end += duration;
            return true;
        });
        return *this;
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskScheduler& DelayAll(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return DelayAll(RandomDurationBetween(min, max));
    }

    template<class _Rep, class _Period>
    TaskScheduler& DelayGroup(uint32 const group, std::chrono::duration<_Rep, _Period> const& duration)
    {
        _task_holder.ModifyIf([&duration, group](TaskContainer const& task) -> bool
        {
            if (task->IsInGroup(group))
            {
                task->_end += duration;
                return true;
            }
            else
                return false;
        });
        return *this;
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskScheduler& DelayGroup(uint32 const group, std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return DelayGroup(group, RandomDurationBetween(min, max));
    }

    template<class _Rep, class _Period>
    TaskScheduler& RescheduleAll(std::chrono::duration<_Rep, _Period> const& duration)
    {
        auto const end = _now + duration;
        _task_holder.ModifyIf([end](TaskContainer const& task) -> bool
        {
            task->_end = end;
            return true;
        });
        return *this;
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskScheduler& RescheduleAll(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return RescheduleAll(RandomDurationBetween(min, max));
    }

    template<class _Rep, class _Period>
    TaskScheduler& RescheduleGroup(uint32 const group, std::chrono::duration<_Rep, _Period> const& duration)
    {
        auto const end = _now + duration;
       _task_holder.ModifyIf([end, group](TaskContainer const& task) -> bool
        {
            if (task->IsInGroup(group))
            {
                task->_end = end;
                return true;
            }
            else
                return false;
        });
        return *this;
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskScheduler& RescheduleGroup(uint32 const group, std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return RescheduleGroup(group, RandomDurationBetween(min, max));
    }

private:
    TaskScheduler& InsertTask(TaskContainer task);

    template<class _Rep, class _Period>
    TaskScheduler& ScheduleAt(timepoint_t const& end, std::chrono::duration<_Rep, _Period> const& time, task_handler_t const& task)
    {
        return InsertTask(std::make_shared<Task>(end + time, time, task));
    }

    template<class _Rep, class _Period>
    TaskScheduler& ScheduleAt(timepoint_t const& end, std::chrono::duration<_Rep, _Period> const& time, uint32 const group, task_handler_t const& task)
    {
        static uint32 const DEFAULT_REPEATED = 0;
        return InsertTask(std::make_shared<Task>(end + time, time, group, DEFAULT_REPEATED, task));
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    static std::chrono::milliseconds
    RandomDurationBetween(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        auto const milli_min = std::chrono::duration_cast<std::chrono::milliseconds>(min);
        auto const milli_max = std::chrono::duration_cast<std::chrono::milliseconds>(max);
        return std::chrono::milliseconds(urand(uint32(milli_min.count()), uint32(milli_max.count())));
    }

    void Dispatch(success_t const& callback);
};

class TaskContext
{
    friend class TaskScheduler;

    TaskScheduler::TaskContainer _task;
    std::weak_ptr<TaskScheduler> _owner;
    std::shared_ptr<bool> _consumed;

    TaskContext& Dispatch(std::function<TaskScheduler&(TaskScheduler&)> const& apply);
public:
    TaskContext();

    explicit TaskContext(TaskScheduler::TaskContainer&& task, std::weak_ptr<TaskScheduler>&& owner);
    TaskContext(TaskContext const& right);
    TaskContext(TaskContext&& right) noexcept;

    TaskContext& operator=(TaskContext const& right);
    TaskContext& operator=(TaskContext&& right) noexcept;

    bool IsExpired() const;

    bool IsInGroup(uint32 group) const;
    TaskContext& SetGroup(uint32 group);
    TaskContext& ClearGroup();

    uint32 GetRepeatCounter() const;

    template<class _Rep, class _Period>
    TaskContext& Repeat(std::chrono::duration<_Rep, _Period> const& duration)
    {
        AssertOnConsumed();

        // Set new duration, in-context timing and increment repeat counter
        _task->_duration = duration;
        _task->_end += duration;
        _task->_repeated += 1;
        *_consumed = true;
        return Dispatch(std::bind(&TaskScheduler::InsertTask, std::placeholders::_1, _task));
    }

    TaskContext& Repeat();

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& Repeat(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return Repeat(TaskScheduler::RandomDurationBetween(min, max));
    }

    TaskContext& Async(std::function<void()> const& callable);

    template<class _Rep, class _Period>
    TaskContext& Schedule(std::chrono::duration<_Rep, _Period> const& time, TaskScheduler::task_handler_t const& task)
    {
        auto const end = _task->_end;
        return Dispatch([end, time, task](TaskScheduler& scheduler) -> TaskScheduler&
        {
            return scheduler.ScheduleAt<_Rep, _Period>(end, time, task);
        });
    }

    template<class _Rep, class _Period>
    TaskContext& Schedule(std::chrono::duration<_Rep, _Period> const& time, uint32 const group, TaskScheduler::task_handler_t const& task)
    {
        auto const end = _task->_end;
        return Dispatch([end, time, group, task](TaskScheduler& scheduler) -> TaskScheduler&
        {
            return scheduler.ScheduleAt<_Rep, _Period>(end, time, group, task);
        });
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& Schedule(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max, TaskScheduler::task_handler_t const& task)
    {
        return Schedule(TaskScheduler::RandomDurationBetween(min, max), task);
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& Schedule(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max, uint32 const group, TaskScheduler::task_handler_t const& task)
    {
        return Schedule(TaskScheduler::RandomDurationBetween(min, max), group, task);
    }

    TaskContext& CancelAll();
    TaskContext& CancelGroup(uint32 group);
    TaskContext& CancelGroupsOf(std::vector<uint32> const& groups);

    template<class _Rep, class _Period>
    TaskContext& DelayAll(std::chrono::duration<_Rep, _Period> const& duration)
    {
        return Dispatch(std::bind(&TaskScheduler::DelayAll<_Rep, _Period>, std::placeholders::_1, duration));
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& DelayAll(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return DelayAll(TaskScheduler::RandomDurationBetween(min, max));
    }

    template<class _Rep, class _Period>
    TaskContext& DelayGroup(uint32 const group, std::chrono::duration<_Rep, _Period> const& duration)
    {
        return Dispatch(std::bind(&TaskScheduler::DelayGroup<_Rep, _Period>, std::placeholders::_1, group, duration));
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& DelayGroup(uint32 const group, std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return DelayGroup(group, TaskScheduler::RandomDurationBetween(min, max));
    }

    template<class _Rep, class _Period>
    TaskContext& RescheduleAll(std::chrono::duration<_Rep, _Period> const& duration)
    {
        return Dispatch(std::bind(&TaskScheduler::RescheduleAll, std::placeholders::_1, duration));
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& RescheduleAll(std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return RescheduleAll(TaskScheduler::RandomDurationBetween(min, max));
    }

    template<class _Rep, class _Period>
    TaskContext& RescheduleGroup(uint32 const group, std::chrono::duration<_Rep, _Period> const& duration)
    {
        return Dispatch(std::bind(&TaskScheduler::RescheduleGroup<_Rep, _Period>, std::placeholders::_1, group, duration));
    }

    template<class _RepLeft, class _PeriodLeft, class _RepRight, class _PeriodRight>
    TaskContext& RescheduleGroup(uint32 const group, std::chrono::duration<_RepLeft, _PeriodLeft> const& min, std::chrono::duration<_RepRight, _PeriodRight> const& max)
    {
        return RescheduleGroup(group, TaskScheduler::RandomDurationBetween(min, max));
    }

private:
    void AssertOnConsumed() const;
    void Invoke();
};

#endif /// _TASK_SCHEDULER_H_
