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
/* written for use instead not locked std::map */

#ifndef LOCKEDMAP_H
#define LOCKEDMAP_H

#include "Common.h"
#include <map>
#include <assert.h>

namespace Trinity
{
    template <class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key,T> > >
    class LockedMap
    {
        public:

        typedef std::mutex LockType;
        typedef std::lock_guard<LockType> LockGuard;
        typedef LockGuard ReadGuard;
        typedef LockGuard WriteGuard;

        typedef typename std::map<Key, T, Compare, Allocator>::iterator               iterator;
        typedef typename std::map<Key, T, Compare, Allocator>::const_iterator         const_iterator;
        typedef typename std::map<Key, T, Compare, Allocator>::reverse_iterator       reverse_iterator;
        typedef typename std::map<Key, T, Compare, Allocator>::const_reverse_iterator const_reverse_iterator;
        typedef typename std::map<Key, T, Compare, Allocator>::allocator_type         allocator_type;
        typedef typename std::map<Key, T, Compare, Allocator>::value_type             value_type;
        typedef typename std::map<Key, T, Compare, Allocator>::size_type              size_type;
        typedef typename std::map<Key, T, Compare, Allocator>::key_compare            key_compare;
        typedef typename std::map<Key, T, Compare, Allocator>::value_compare          value_compare;

        // Constructors
        explicit LockedMap(const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : m_storage(comp, alloc)
        {}

        template <class InputIterator> LockedMap(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : m_storage(first, last, comp, alloc)
        {}

        LockedMap(const LockedMap<Key, T, Compare, Allocator> & x) : m_storage(x.m_storage)
        {}

        // Destructor
        virtual ~LockedMap(void)
        {
            WriteGuard Guard(GetLock());
        }

        // Copy
        LockedMap<Key, Compare, Allocator>& operator= (const LockedMap<Key, Compare, Allocator>& x)
        {
            WriteGuard Guard(GetLock());
            ReadGuard GuardX(x.GetLock());
            m_storage = x.m_storage;
            return *this;
        }

        // Iterators
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

        // Capacity
        size_type size(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.size();
        }

        size_type max_size(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.max_size();
        }

        bool empty(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.empty();
        }

        // Access
        T& operator[](const Key& x)
        {
            if (find(x) == end())
            {
                WriteGuard Guard(GetLock());
                return m_storage[x];
            }
            else
            {
                ReadGuard Guard(GetLock());
                return m_storage[x];
            }
        }

        const T& operator[](const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage[x];
        }

        // Modifiers
        std::pair<iterator, bool> insert(const value_type& x)
        {
            WriteGuard Guard(GetLock());
            return m_storage.insert(x);
        }

        iterator insert(iterator position, const value_type& x)
        {
            WriteGuard Guard(GetLock());
            return m_storage.insert(position, x);
        }

        template <class InputIterator> void insert(InputIterator first, InputIterator last)
        {
            WriteGuard Guard(GetLock());
            m_storage.insert(first, last);
        }

        void erase(iterator pos)
        {
            WriteGuard Guard(GetLock());
            m_storage.erase(pos);
        }

        size_type erase(const Key& x)
        {
            WriteGuard Guard(GetLock());
            return m_storage.erase(x);
        }

        void erase(iterator begin, iterator end)
        {
            WriteGuard Guard(GetLock());
            m_storage.erase(begin, end);
        }

        void swap(LockedMap<Key, T, Compare, Allocator>& x) noexcept
        {
            WriteGuard Guard(GetLock());
            WriteGuard GuardX(x.GetLock());
            m_storage.swap(x.storage);
        }

        void clear(void)
        {
            WriteGuard Guard(GetLock());
            m_storage.clear();
        }

        // Observers
        key_compare key_comp(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.key_comp();
        }

        value_compare value_comp(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.value_comp();
        }

        // Operations
        const_iterator find(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.find(x);
        }

        iterator find(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.find(x);
        }

        size_type count(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.count(x);
        }

        const_iterator lower_bound(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.lower_bound(x);
        }

        iterator lower_bound(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.lower_bound(x);
        }

        const_iterator upper_bound(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.upper_bound(x);
        }

        iterator upper_bound(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.upper_bound(x);
        }

        std::pair<const_iterator, const_iterator> equal_range(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.equal_range(x);
        }

        std::pair<iterator, iterator> equal_range(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.equal_range(x);
        }

        // Allocator
        allocator_type get_allocator(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.get_allocator();
        }

        LockType&       GetLock() { return i_lock; }
        LockType&       GetLock() const { return i_lock; }

        // may be used _ONLY_ with external locking!
        std::map<Key, T>&  getSource()
        {
            return m_storage;
        }

    protected:
        mutable LockType           i_lock;
        std::map<Key, T, Compare, Allocator>     m_storage;
    };

    template <class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key,T> > >
    class LockedMultiMap
    {
        public:

        typedef std::mutex LockType;
        typedef std::lock_guard<LockType> LockGuard;
        typedef LockGuard ReadGuard;
        typedef LockGuard WriteGuard;

        typedef typename std::multimap<Key, T, Compare, Allocator>::iterator               iterator;
        typedef typename std::multimap<Key, T, Compare, Allocator>::const_iterator         const_iterator;
        typedef typename std::multimap<Key, T, Compare, Allocator>::reverse_iterator       reverse_iterator;
        typedef typename std::multimap<Key, T, Compare, Allocator>::const_reverse_iterator const_reverse_iterator;
        typedef typename std::multimap<Key, T, Compare, Allocator>::allocator_type         allocator_type;
        typedef typename std::multimap<Key, T, Compare, Allocator>::value_type             value_type;
        typedef typename std::multimap<Key, T, Compare, Allocator>::size_type              size_type;
        typedef typename std::multimap<Key, T, Compare, Allocator>::key_compare            key_compare;
        typedef typename std::multimap<Key, T, Compare, Allocator>::value_compare          value_compare;

        // Constructors
        explicit LockedMultiMap(const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : m_storage(comp, alloc)
        {}

        template <class InputIterator> LockedMultiMap(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : m_storage(first, last, comp, alloc)
        {}

        LockedMultiMap(const LockedMap<Key, T, Compare, Allocator> & x) : m_storage(x.m_storage)
        {}

        // Destructor
        virtual ~LockedMultiMap(void)
        {
            WriteGuard Guard(GetLock());
        }

        // Copy
        LockedMultiMap<Key, Compare, Allocator>& operator= (const LockedMultiMap<Key, Compare, Allocator>& x)
        {
            WriteGuard Guard(GetLock());
            ReadGuard GuardX(x.GetLock());
            m_storage = x.m_storage;
            return *this;
        }

        // Iterators
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

        // Capacity
        size_type size(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.size();
        }

        size_type max_size(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.max_size();
        }

        bool empty(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.empty();
        }

        // Modifiers
        std::pair<iterator, bool> insert(const value_type& x)
        {
            WriteGuard Guard(GetLock());
            return m_storage.insert(x);
        }

        iterator insert(iterator position, const value_type& x)
        {
            WriteGuard Guard(GetLock());
            return m_storage.insert(position, x);
        }

        template <class InputIterator> void insert(InputIterator first, InputIterator last)
        {
            WriteGuard Guard(GetLock());
            m_storage.insert(first, last);
        }

        void erase(iterator pos)
        {
            WriteGuard Guard(GetLock());
            m_storage.erase(pos);
        }

        size_type erase(const Key& x)
        {
            WriteGuard Guard(GetLock());
            return m_storage.erase(x);
        }

        void erase(iterator begin, iterator end)
        {
            WriteGuard Guard(GetLock());
            m_storage.erase(begin, end);
        }

        void swap(LockedMap<Key, T, Compare, Allocator>& x)
        {
            WriteGuard Guard(GetLock());
            WriteGuard GuardX(x.GetLock());
            m_storage.swap(x.storage);
        }

        void clear(void)
        {
            WriteGuard Guard(GetLock());
            m_storage.clear();
        }

        // Observers
        key_compare key_comp(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.key_comp();
        }

        value_compare value_comp(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.value_comp();
        }

        // Operations
        const_iterator find(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.find(x);
        }

        iterator find(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.find(x);
        }

        size_type count(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.count(x);
        }

        const_iterator lower_bound(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.lower_bound(x);
        }

        iterator lower_bound(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.lower_bound(x);
        }

        const_iterator upper_bound(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.upper_bound(x);
        }

        iterator upper_bound(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.upper_bound(x);
        }

        std::pair<const_iterator, const_iterator> equal_range(const Key& x) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.equal_range(x);
        }

        std::pair<iterator, iterator> equal_range(const Key& x)
        {
            ReadGuard Guard(GetLock());
            return m_storage.equal_range(x);
        }

        // Allocator
        allocator_type get_allocator(void) const
        {
            ReadGuard Guard(GetLock());
            return m_storage.get_allocator();
        }

        LockType&       GetLock() { return i_lock; }
        LockType&       GetLock() const { return i_lock; }

        // may be used _ONLY_ with external locking!
        std::multimap<Key, T>&  getSource()
        {
            return m_storage;
        }

    protected:
        mutable LockType           i_lock;
        std::multimap<Key, T, Compare, Allocator>     m_storage;
    };
}
#endif
