/*
 * Copyright (C) 2011-2012 /dev/rsa for MangosR2 <http://github.com/MangosR2>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

 /* based on LockedQueue class from MaNGOS */
 /* written for use instead not locked std::list && std::vector */

#ifndef LOCKEDVECTOR_H
#define LOCKEDVECTOR_H

#include "Common.h"
#include <vector>
#include <list>
#include <assert.h>

namespace Trinity
{
    template <class T, class Allocator = std::allocator<T> >
    class LockedVector
    {
    public:

        typedef std::mutex LockType;
        typedef std::lock_guard<LockType> LockGuard;
        typedef LockGuard ReadGuard;
        typedef LockGuard WriteGuard;

        typedef typename std::vector<T, Allocator>::iterator               iterator;
        typedef typename std::vector<T, Allocator>::const_iterator         const_iterator;
        typedef typename std::vector<T, Allocator>::reverse_iterator       reverse_iterator;
        typedef typename std::vector<T, Allocator>::const_reverse_iterator const_reverse_iterator;
        typedef typename std::vector<T, Allocator>::allocator_type         allocator_type;
        typedef typename std::vector<T, Allocator>::value_type             value_type;
        typedef typename std::vector<T, Allocator>::size_type              size_type;
        typedef typename std::vector<T, Allocator>::difference_type        difference_type;

        //Constructors
        explicit LockedVector(const Allocator& alloc = Allocator()) : m_storage(alloc)
        {}

        explicit LockedVector(size_type n, const T& value = T(), const Allocator& alloc = Allocator())
            : m_storage(n, value, alloc)
        {}

        virtual ~LockedVector(void)
        {
            WriteGuard Guard(GetLock());
        }

        void reserve(size_type idx)
        {
            WriteGuard Guard(GetLock());
            m_storage.reserve(idx);
        }

        // Methods
        void push_back(const T& item)
        {
            WriteGuard Guard(GetLock());
            m_storage.push_back(item);
        }

        void insert(iterator pos, size_type n, const T& u)
        {
            WriteGuard Guard(GetLock());
            m_storage.insert(pos, n, u);
        }

        template <class InputIterator>
        void insert(iterator pos, InputIterator begin, InputIterator end)
        {
            WriteGuard Guard(GetLock());
            m_storage.insert(pos, begin, end);
        }

        void pop_back()
        {
            WriteGuard Guard(GetLock());
            m_storage.pop_back();
        }

        void erase(size_type pos)
        {
            WriteGuard Guard(GetLock());
            m_storage.erase(m_storage.begin() + pos);
        }

        iterator erase(iterator itr)
        {
            WriteGuard Guard(GetLock());
            return m_storage.erase(itr);
        }

        void remove(const T& item)
        {
            erase(item);
        }

        void erase(const T& item)
        {
            WriteGuard Guard(GetLock());
            for (size_type i = 0; i < m_storage.size();)
            {
                if (item == m_storage[i])
                    m_storage.erase(m_storage.begin() + i);
                else
                    ++i;
            }
        }

        T* find(const T& item)
        {
            ReadGuard Guard(GetLock());
            for (size_type i = 0; i < m_storage.size(); ++i)
            {
                if (item == m_storage[i])
                    return &m_storage[i];
            }
            return NULL;
        }

        const T* find(const T& item) const
        {
            ReadGuard Guard(GetLock());
            for (size_type i = 0; i < m_storage.size(); ++i)
            {
                if (item == m_storage[i])
                    return &m_storage[i];
            }
            return NULL;
        }

        void clear()
        {
            WriteGuard Guard(GetLock());
            m_storage.clear();
        }

        T& operator[](size_type idx)
        {
            ReadGuard Guard(GetLock());
            return m_storage[idx];
        }

        const T& operator[](size_type idx) const
        {
            ReadGuard Guard(GetLock());
            return m_storage[idx];
        }

        T& at(size_type idx)
        {
            ReadGuard Guard(GetLock());
            return m_storage.at(idx);
        }

        T& front()
        {
            ReadGuard Guard(GetLock());
            return m_storage.front();
        }

        T& back()
        {
            ReadGuard Guard(GetLock());
            return m_storage.back();
        }

        const T& front() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.front();
        }

        const T& back() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.back();
        }

        iterator begin()
        {
            ReadGuard Guard(GetLock());
            return m_storage.begin();
        }

        iterator end()
        {
            ReadGuard Guard(GetLock());
            return m_storage.end();
        }

        const_iterator begin() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.begin();
        }

        const_iterator end() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.end();
        }

        reverse_iterator rbegin()
        {
            ReadGuard Guard(GetLock());
            return m_storage.rbegin();
        }

        reverse_iterator rend()
        {
            ReadGuard Guard(GetLock());
            return m_storage.rend();
        }

        const_reverse_iterator rbegin() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.rbegin();
        }

        const_reverse_iterator rend() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.rend();
        }

        bool empty() const
        {
            ReadGuard Guard(GetLock());
            return    m_storage.empty();
        }

        size_type size() const
        {
            ReadGuard Guard(GetLock());
            return    m_storage.size();
        }

        LockedVector& operator=(const std::vector<T> &v)
        {
            clear();
            WriteGuard Guard(GetLock());
            for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i)
            {
                this->push_back(*i);
            }
            return *this;
        }

        LockedVector(const std::vector<T> &v)
        {
            WriteGuard Guard(GetLock());
            for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i)
            {
                this->push_back(*i);
            }
        }

        LockedVector& operator=(const std::list<T> &v)
        {
            clear();
            WriteGuard Guard(GetLock());
            for (typename std::list<T>::const_iterator i = v.begin(); i != v.end(); ++i)
            {
                this->push_back(*i);
            }
            return *this;
        }

        LockedVector(const std::list<T> &v)
        {
            WriteGuard Guard(GetLock());
            for (typename std::list<T>::const_iterator i = v.begin(); i != v.end(); ++i)
            {
                this->push_back(*i);
            }
        }

        LockedVector& operator=(const LockedVector<T> &v)
        {
            WriteGuard Guard(GetLock());
            ReadGuard GuardX(v.GetLock());
            m_storage = v.m_storage;
            return *this;
        }

        LockedVector(const LockedVector<T> &v)
        {
            WriteGuard Guard(GetLock());
            ReadGuard GuardX(v.GetLock());
            m_storage = v.m_storage;
        }

        void swap(LockedVector<T, Allocator>& x) noexcept
        {
            WriteGuard Guard(GetLock());
            WriteGuard GuardX(x.GetLock());
            m_storage.swap(x.m_storage);
        }

        void resize(size_type num, T def = T())
        {
            WriteGuard Guard(GetLock());
            m_storage.resize(num, def);
        }

        //Allocator
        allocator_type get_allocator() const
        {
            ReadGuard Guard(GetLock());
            return m_storage.get_allocator();
        }

        // Sort template
        template <typename C>
        void sort(C& compare)
        {
            iterator _begin = begin();
            iterator _end = end();
            WriteGuard Guard(GetLock());
            std::stable_sort(_begin, _end, compare);
        }

        LockType& GetLock() { return i_lock; }
        LockType& GetLock() const { return i_lock; }

        // may be used _ONLY_ with external locking!
        std::vector<T>&  getSource()
        {
            return m_storage;
        }

    protected:
        mutable LockType i_lock;
        std::vector<T, Allocator> m_storage;
    };
}
#endif
