////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SCRIPT_UTILS_H
#define SCRIPT_UTILS_H

#include "GridNotifiers.h"
#include "ObjectVisitors.hpp" 
#include "CellImpl.h"

namespace MS
{
    namespace ScriptUtils
    {
        inline GameObject* SelectNearestGameObjectWithEntry(Unit* me, uint32 entry, float range = 0.0f)
        {
            std::list<GameObject*> targetList;

            Trinity::NearestGameObjectEntryInObjectRangeCheck check(*me, entry, range);
            Trinity::GameObjectListSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(me, targetList, check);
            me->VisitNearbyObject(range, searcher);

            for (auto go : targetList)
                return go;

            return nullptr;
        }

        inline Creature* SelectNearestCreatureWithEntry(Unit* me, uint32 entry, float radius = 0.0f)
        {
            std::list<Unit*> targetList;
            Trinity::AnyUnitInObjectRangeCheck check(me, radius);
            Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(me, targetList, check);
            me->VisitNearbyObject(radius, searcher);

            for (auto unit : targetList)
                if (unit->IsUnit() && unit->GetEntry() == entry)
                    return dynamic_cast<Creature*>(unit);

            return nullptr;
        }

        inline Unit* SelectRandomCreatureWithEntry(Unit* me, uint32 entry, float radius = 0.0f)
        {
            std::list<Unit*> targetList;
            Trinity::AnyUnitInObjectRangeCheck check(me, radius);
            Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(me, targetList, check);
            me->VisitNearbyObject(radius, searcher);

            std::list<Unit*> results;
            for (auto unit : targetList)
                if (unit->GetEntry() == entry)
                    results.emplace_back(unit);

            if (results.empty())
                return nullptr;

            return Trinity::Containers::SelectRandomContainerElement(results);
        }

        inline std::list<Unit*> SelectNearestUnits(Unit* me, uint32 withEntry, float radius = 0.0f, bool friendly = false)
        {
            std::list<Unit*> targetList;

            if (friendly)
            {
                Trinity::AnyFriendlyUnitInObjectRangeCheck check(me, me, radius);
                Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, targetList, check);
                VisitNearbyObject(me, radius, searcher);
            }
            else
            {
                Trinity::AnyUnitInObjectRangeCheck check(me, radius);
                Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(me, targetList, check);
                VisitNearbyObject(me, radius, searcher);
            }

            targetList.remove_if([withEntry](Unit* unit)
            {
                return withEntry && unit->GetEntry() != withEntry;
            });

            return targetList;
        }

        inline Unit* SelectRandomEnemy(Unit* me, float range = 0.0f, bool checkLoS = true)
        {
            std::list<Unit*> targetList;
            auto radius = range;

            Trinity::AnyFriendlyUnitInObjectRangeCheck check(me, me, radius);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, targetList, check);
            me->VisitNearbyObject(radius, searcher);

            targetList.remove_if([me, range, checkLoS](Unit* unit)
            {
                return !(unit && (me->IsWithinLOSInMap(unit) || !checkLoS) && me->IsWithinDistInMap(unit, range) && unit->isAlive() && unit->GetGUID() != me->GetGUID());
            });

            if (targetList.empty())
                return nullptr;

            return Trinity::Containers::SelectRandomContainerElement(targetList);
        }

        inline Unit* SelectNearestFriendExcluededMe(Unit* me, float range = 0.0f, bool checkLoS = true)
        {
            std::list<Unit*> targetList;
            auto radius = range;

            Trinity::AnyFriendlyUnitInObjectRangeCheck check(me, me, radius);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, targetList, check);
            me->VisitNearbyObject(radius, searcher);

            for (auto unit : targetList)
                if (unit && (me->IsWithinLOSInMap(unit) || !checkLoS) && me->IsWithinDistInMap(unit, range) && unit->isAlive() && unit->GetGUID() != me->GetGUID())
                    return unit;

            return nullptr;
        }

        inline Player* SelectNearestPlayer(Unit* me, float range = 0.0f, bool checkLoS = true)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return nullptr;

            auto const& playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return nullptr;

            std::list<Player*> temp;
            for (auto& itr : playerList)
                if (!itr.getSource()->isGameMaster() && (me->IsWithinLOSInMap(itr.getSource()) || !checkLoS) && me->GetExactDist2d(itr.getSource()) < range && itr.getSource()->isAlive())
                    temp.push_back(itr.getSource());

            if (!temp.empty())
                return temp.front();

            return nullptr;
        }

        inline void ApplyOnEveryPlayer(Unit* me, std::function<void(Unit*, Player*)> const& function)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return;

            auto const& playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return;

            std::list<Player*> temp;
            for (auto& itr : playerList)
                function(me, itr.getSource());
        }

        inline Player* SelectRandomPlayerExcludedTank(Unit* me, float range = 0.0f, bool checkLoS = true)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return nullptr;

            auto const &playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return nullptr;

            std::list<Player*> targetList;
            for (auto& itr : playerList)
                if ((me->IsWithinLOSInMap(itr.getSource()) || !checkLoS) && me->getVictim() != itr.getSource() && me->IsWithinDistInMap(itr.getSource(), range) && itr.getSource()->isAlive())
                    targetList.push_back(itr.getSource());

            if (targetList.empty())
                return nullptr;

            return Trinity::Containers::SelectRandomContainerElement(targetList);
        }

        inline Player* SelectFarEnoughPlayerIncludedTank(Unit* me, float range = 0.0f, bool checkLoS = true)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return nullptr;

            auto const &playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return nullptr;

            std::list<Player*> temp;
            for (auto& itr : playerList)
                if (!itr.getSource()->isGameMaster() && (me->IsWithinLOSInMap(itr.getSource()) || !checkLoS) && !me->IsWithinDistInMap(itr.getSource(), range) && itr.getSource()->isAlive())
                    return itr.getSource();

            return nullptr;
        }

        inline Player* SelectRandomPlayerIncludedTank(Unit* me, float range = 0.0f, bool checkLoS = false)
        {
            auto map = me->GetMap();
            if (!map->IsDungeon())
                return nullptr;

            auto const &playerList = map->GetPlayers();
            if (playerList.isEmpty())
                return nullptr;

            std::list<Player*> targetList;
            for (auto& itr : playerList)
                if (!itr.getSource()->isGameMaster() && (me->IsWithinLOSInMap(itr.getSource()) || !checkLoS) && (me->IsWithinDistInMap(itr.getSource(), range) || !range) && itr.getSource()->isAlive())
                    targetList.push_back(itr.getSource());

            if (targetList.empty())
                return nullptr;

            return Trinity::Containers::SelectRandomContainerElement(targetList);
        }

        inline WorldObject* AnyDeadUnitSpellTargetInRangeCheck(Unit* caster, SpellInfo const* spellInfo, float range)
        {
            WorldObject* result = nullptr;
            Trinity::AnyDeadUnitSpellTargetInRangeCheck check(caster, range, spellInfo, TARGET_CHECK_ENEMY);
            Trinity::WorldObjectSearcher<Trinity::AnyDeadUnitSpellTargetInRangeCheck> searcher(caster, result, check);
            return result;
        }
    }
}

#endif /* !SCRIPT_UTILS */
