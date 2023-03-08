/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#include "DynamicTree.h"
#include "BoundingIntervalHierarchyWrapper.h"
#include "GameObjectModel.h"
#include "Log.h"
#include "MapTree.h"
#include "ModelInstance.h"
#include "RegularGrid.h"
#include "Timer.h"
#include <G3D/AABox.h>
#include <G3D/Ray.h>
#include <G3D/Vector3.h>

using VMAP::ModelInstance;

namespace {

int CHECK_TREE_PERIOD = 200;

} // namespace

template<> struct HashTrait< GameObjectModel>{
    static size_t hashCode(const GameObjectModel& g) { return (size_t)(void*)&g; }
};

template<> struct PositionTrait< GameObjectModel> {
    static void getPosition(const GameObjectModel& g, G3D::Vector3& p) { p = g.getPosition(); }
};

template<> struct BoundsTrait< GameObjectModel> {
    static void getBounds(const GameObjectModel& g, G3D::AABox& out) { out = g.getBounds();}
    static void getBounds2(const GameObjectModel* g, G3D::AABox& out) { out = g->getBounds();}
};

/*
static bool operator == (const GameObjectModel& mdl, const GameObjectModel& mdl2){
    return &mdl == &mdl2;
}
*/

typedef RegularGrid2D<GameObjectModel, BIHWrap<GameObjectModel> > ParentTree;

struct DynTreeImpl : public ParentTree/*, public Intersectable*/
{
    typedef GameObjectModel Model;
    typedef ParentTree base;

    DynTreeImpl() :
        rebalance_timer(CHECK_TREE_PERIOD),
        unbalanced_times(0)
    {
    }

    void insert(const Model& mdl)
    {
        base::insert(mdl);
        ++unbalanced_times;
    }

    void remove(const Model& mdl)
    {
        base::remove(mdl);
        ++unbalanced_times;
    }

    void balance()
    {
        base::balance();
        unbalanced_times = 0;
    }

    void update(uint32 difftime)
    {
        if (empty())
            return;

        rebalance_timer.Update(difftime);
        if (rebalance_timer.Passed())
        {
            rebalance_timer.Reset(CHECK_TREE_PERIOD);
            if (unbalanced_times > 0)
                balance();
        }
    }

    TimeTrackerSmall rebalance_timer;
    int unbalanced_times;
};

DynamicMapTree::DynamicMapTree() : impl(new DynTreeImpl()) { }

DynamicMapTree::~DynamicMapTree()
{
    delete impl;
}

void DynamicMapTree::insert(const GameObjectModel& mdl)
{
    RecursiveGuard guard(dynamic_lock);
    impl->insert(mdl);
}

void DynamicMapTree::remove(const GameObjectModel& mdl)
{
    RecursiveGuard guard(dynamic_lock);
    impl->remove(mdl);
}

bool DynamicMapTree::contains(const GameObjectModel& mdl) const
{
    return impl->contains(mdl);
}

void DynamicMapTree::balance()
{
    RecursiveGuard guard(dynamic_lock);
    impl->balance();
}

void DynamicMapTree::update(uint32 t_diff)
{
    impl->update(t_diff);
}

struct DynamicTreeIntersectionCallback
{
    DynamicTreeIntersectionCallback(std::set<uint32> const& phases, bool otherUsePlayerPhasingRules) : _didHit(false), _phases(phases), _otherUsePlayerPhasingRules(otherUsePlayerPhasingRules), _go(nullptr) { }

    bool operator()(G3D::Ray const& r, GameObjectModel const& obj, float& distance)
    {
        _didHit = obj.intersectRay(r, distance, true, _phases, _otherUsePlayerPhasingRules);
        if (_didHit)
        {
            if (obj.owner->IsDoor()) // Collision for door
                distance = distance > 1.0f ? distance - 1.0f : 0.0f;
            _go = const_cast<GameObject*>(obj.owner->GetOwner());
        }
        return _didHit;
    }

    bool didHit() const { return _didHit; }
    GameObject* _go;

private:
    bool _didHit;
    std::set<uint32> _phases;
    bool _otherUsePlayerPhasingRules;
};

struct DynamicTreeisInLineOfSightCallback
{
    DynamicTreeisInLineOfSightCallback(std::set<uint32> const& phases, bool otherUsePlayerPhasingRules) : _didHit(false), _phases(phases), _otherUsePlayerPhasingRules(otherUsePlayerPhasingRules), _go(nullptr) { }

    bool operator()(G3D::Ray const& r, GameObjectModel const& obj, float& distance)
    {
        _didHit = obj.intersectLine(r, distance, true, _phases, _otherUsePlayerPhasingRules);
        if (_didHit)
            _go = const_cast<GameObject*>(obj.owner->GetOwner());
        return _didHit;
    }

    bool didHit() const { return _didHit; }
    GameObject* _go;

private:
    bool _didHit;
    std::set<uint32> _phases;
    bool _otherUsePlayerPhasingRules;
};

struct DynamicTreeAreaInfoCallback
{
    DynamicTreeAreaInfoCallback(std::set<uint32> const& phases, bool otherUsePlayerPhasingRules) : _phases(phases), _otherUsePlayerPhasingRules(otherUsePlayerPhasingRules) {}

    void operator()(G3D::Vector3 const& p, GameObjectModel const& obj)
    {
        obj.intersectPoint(p, _areaInfo, _phases, _otherUsePlayerPhasingRules);
    }

    VMAP::AreaInfo const& GetAreaInfo() const { return _areaInfo; }

private:
    std::set<uint32> _phases;
    VMAP::AreaInfo _areaInfo;
    bool _otherUsePlayerPhasingRules;
};

bool DynamicMapTree::getIntersectionTime(std::set<uint32> const& phases, bool otherUsePlayerPhasingRules, G3D::Ray const& ray, G3D::Vector3 const& endPos, float& maxDist, DynamicTreeCallback* dCallback) const
{
    float distance = maxDist;
    DynamicTreeIntersectionCallback callback(phases, otherUsePlayerPhasingRules);
    RecursiveGuard guard(dynamic_lock);
    impl->intersectRay(ray, callback, distance, endPos);
    if (callback.didHit())
    {
        if (dCallback)
            dCallback->go = callback._go;
        maxDist = distance;
    }
    return callback.didHit();
}

bool DynamicMapTree::getObjectHitPos(std::set<uint32> const& phases, bool otherUsePlayerPhasingRules, G3D::Vector3 const& startPos, G3D::Vector3 const& endPos, G3D::Vector3& resultHitPos, float modifyDist, DynamicTreeCallback* dCallback) const
{
    bool result = false;
    float maxDist = (endPos - startPos).magnitude();
    // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
    if (maxDist >= std::numeric_limits<float>::max())
        return false;
    // ASSERT(maxDist < std::numeric_limits<float>::max());
    // prevent NaN values which can cause BIH intersection to enter infinite loop
    if (maxDist < 1e-10f)
    {
        resultHitPos = endPos;
        return false;
    }
    G3D::Vector3 dir = (endPos - startPos)/maxDist;              // direction with length of 1
    G3D::Ray ray(startPos, dir);
    float dist = maxDist;
    if (getIntersectionTime(phases, otherUsePlayerPhasingRules, ray, endPos, dist, dCallback))
    {
        resultHitPos = startPos + dir * dist;
        if (modifyDist < 0)
        {
            if ((resultHitPos - startPos).magnitude() > -modifyDist)
                resultHitPos += dir * modifyDist;
            else
                resultHitPos = startPos;
        }
        else
            resultHitPos += dir * modifyDist;

        result = true;
    }
    else
    {
        resultHitPos = endPos;
        result = false;
    }
    return result;
}

bool DynamicMapTree::isInLineOfSight(G3D::Vector3 const& startPos, G3D::Vector3 const& endPos, std::set<uint32> const& phases, bool otherUsePlayerPhasingRules, DynamicTreeCallback* dCallback) const
{
    float maxDist = (endPos - startPos).magnitude();

    if (!G3D::fuzzyGt(maxDist, 0))
        return true;

    G3D::Ray r(startPos, (endPos - startPos) / maxDist);
    DynamicTreeisInLineOfSightCallback callback(phases, otherUsePlayerPhasingRules);
    RecursiveGuard guard(dynamic_lock);
    impl->intersectRay(r, callback, maxDist, endPos);

    if (callback.didHit())
        if (dCallback)
            dCallback->go = callback._go;

    return !callback.didHit();
}

float DynamicMapTree::getHeight(float x, float y, float z, float maxSearchDist, std::set<uint32> const& phases, bool otherUsePlayerPhasingRules, DynamicTreeCallback* dCallback) const
{
    G3D::Vector3 v(x, y, z + 0.5f);
    G3D::Ray r(v, G3D::Vector3(0, 0, -1));
    DynamicTreeIntersectionCallback callback(phases, otherUsePlayerPhasingRules);
    RecursiveGuard guard(dynamic_lock);
    impl->intersectZAllignedRay(r, callback, maxSearchDist);

    if (callback.didHit())
    {
        if (dCallback)
            dCallback->go = callback._go;
        return v.z - maxSearchDist;
    }
    else
        return -G3D::finf();
}

bool DynamicMapTree::getAreaInfo(float x, float y, float& z, std::set<uint32> const& phases, bool otherUsePlayerPhasingRules, uint32& flags, int32& adtId, int32& rootId, int32& groupId) const
{
    G3D::Vector3 v(x, y, z + 0.5f);
    DynamicTreeAreaInfoCallback intersectionCallBack(phases, otherUsePlayerPhasingRules);
    RecursiveGuard guard(dynamic_lock);
    impl->intersectPoint(v, intersectionCallBack);
    if (intersectionCallBack.GetAreaInfo().result)
    {
        flags = intersectionCallBack.GetAreaInfo().flags;
        adtId = intersectionCallBack.GetAreaInfo().adtId;
        rootId = intersectionCallBack.GetAreaInfo().rootId;
        groupId = intersectionCallBack.GetAreaInfo().groupId;
        z = intersectionCallBack.GetAreaInfo().ground_Z;
        return true;
    }
    return false;
}
