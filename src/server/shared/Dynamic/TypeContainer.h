/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_TYPECONTAINER_H
#define TRINITY_TYPECONTAINER_H

/*
 * Here, you'll find a series of containers that allow you to hold multiple
 * types of object at the same time.
 */

#include "Define.h"
#include "Dynamic/TypeList.h"

#include <type_traits>
#include <vector>

namespace Trinity {

namespace Detail {

/*
 * @class ContainerMapList is a mulit-type container for map elements
 * By itself its meaningless but collaborate along with TypeContainers,
 * it become the most powerfully container in the whole system.
 */
template <typename T>
struct ContainerMapList
{
    std::vector<T*> elements;
};

template <>
struct ContainerMapList<TypeNull> { };

template <typename Head, typename Tail>
struct ContainerMapList<TypeList<Head, Tail>>
{
    ContainerMapList<Head> head;
    ContainerMapList<Tail> tail;
};

template <typename SpecificType, typename Head, typename Tail>
typename std::enable_if<
    std::is_same<Head, SpecificType>::value,
    ContainerMapList<SpecificType> const &
>::type
mapForType(ContainerMapList<TypeList<Head, Tail>> const &m)
{
    return m.head;
}

template <typename SpecificType, typename Head, typename Tail>
typename std::enable_if<
    !std::is_same<Head, SpecificType>::value,
    ContainerMapList<SpecificType> const &
>::type
mapForType(ContainerMapList<TypeList<Head, Tail>> const &m)
{
    return Trinity::Detail::mapForType<SpecificType>(m.tail);
}

template <typename SpecificType, typename Head, typename Tail>
typename std::enable_if<
    std::is_same<Head, SpecificType>::value,
    ContainerMapList<SpecificType> &
>::type
mapForType(ContainerMapList<TypeList<Head, Tail>> &m)
{
    return m.head;
}

template <typename SpecificType, typename Head, typename Tail>
typename std::enable_if<
    !std::is_same<Head, SpecificType>::value,
    ContainerMapList<SpecificType> &
>::type
mapForType(ContainerMapList<TypeList<Head, Tail>> &m)
{
    return Trinity::Detail::mapForType<SpecificType>(m.tail);
}

} // namespace Detail

/*
 * @class TypeMapContainer contains a fixed number of types and is
 * determined at compile time.  This is probably the most complicated
 * class and do its simplest thing, that is, holds objects
 * of different types.
 */

template <typename ObjectTypes>
class TypeMapContainer final
{
public:
    typedef Detail::ContainerMapList<ObjectTypes> ObjectMap;

public:
    template <typename SpecificType>
    std::size_t count() const
    {
        auto const &m = Detail::mapForType<SpecificType>(m_objectMap);
        return m.elements.size();
    }

    template <typename SpecificType>
    void insert(SpecificType *obj)
    {
        auto &m = Detail::mapForType<SpecificType>(m_objectMap);
        obj->AddToGrid(m.elements);
    }

    ObjectMap & objectMap() { return m_objectMap; }

    ObjectMap const & objectMap() const { return m_objectMap; }

private:
    ObjectMap m_objectMap;
};

} // namespace Trinity

#endif
