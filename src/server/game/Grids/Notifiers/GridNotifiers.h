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

#ifndef TRINITY_GRIDNOTIFIERS_H
#define TRINITY_GRIDNOTIFIERS_H

#include "AreaTrigger.h"
#include "Conversation.h"
#include "Corpse.h"
#include "CreatureAI.h"
#include "DynamicObject.h"
#include "EventObject.h"
#include "GameObject.h"
#include "Player.h"
#include "SocialMgr.h"
#include "Spell.h"
#include "UpdateData.h"

namespace Trinity
{
    struct VisibleNotifier
    {
        Player &i_player;
        UpdateData i_data;
        std::set<Unit*> i_visibleNow;
        GuidSet vis_guids;

        VisibleNotifier(Player& player);

        void AddMaxVisible();
        void SendToSelf();

        template <typename AnyMapType>
        void Visit(AnyMapType &m);
        void Visit(EventObjectMapType &m);
    };

    struct VisibleChangesNotifier
    {
        WorldObject &i_object;

        explicit VisibleChangesNotifier(WorldObject& object);

        void Visit(GameObjectMapType &);
        void Visit(PlayerMapType &);
        void Visit(CreatureMapType &);
        void Visit(DynamicObjectMapType &);
        void Visit(AreaTriggerMapType &);
        void Visit(ConversationMapType &);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    class AIRelocationNotifier
    {
    public:
        explicit AIRelocationNotifier(Unit& unit);

        void Visit(CreatureMapType &);
        void Visit(EventObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) { }

    private:
        Unit *unit_;
        std::vector<std::pair<Creature *, bool>> movedInLos_;
        std::recursive_mutex i_movedInLosLock;
    };

    struct MessageDistDeliverer
    {
        WorldObject* i_source;
        WorldPacket const* i_message;
        uint32 i_phaseMask;
        float i_distSq;
        uint32 team;
        Player const* skipped_receiver;
        GuidUnorderedSet m_IgnoredGUIDs;
        MessageDistDeliverer(WorldObject* src, WorldPacket const* msg, float dist, bool own_team_only = false, Player const* skipped = nullptr, GuidUnorderedSet ignoredSet = GuidUnorderedSet());

        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(DynamicObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}

        void SendPacket(Player* player);
    };

    struct UnfriendlyMessageDistDeliverer
    {
        Unit const *i_source;
        WorldPacket* i_message;
        uint32 i_phaseMask;
        float i_distSq;

        UnfriendlyMessageDistDeliverer(Unit const* src, WorldPacket* msg, float dist);

        void Visit(PlayerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}

        void SendPacket(Player* player);
    };

    // SEARCHERS & LIST SEARCHERS & WORKERS

    // WorldObject searchers & workers

    template<class Check>
    struct WorldObjectSearcher
    {
        uint32 i_mapTypeMask;
        uint32 i_phaseMask;
        WorldObject* &i_object;
        Check &i_check;

        WorldObjectSearcher(WorldObject const* searcher, WorldObject* & result, Check& check, uint32 mapTypeMask = GRID_MAP_TYPE_MASK_ALL)
            : i_mapTypeMask(mapTypeMask), i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(GameObjectMapType &m);
        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(CorpseMapType &m);
        void Visit(DynamicObjectMapType &m);
        void Visit(AreaTriggerMapType &m);
        void Visit(ConversationMapType &m);
        void Visit(EventObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Check>
    struct WorldObjectLastSearcher
    {
        uint32 i_mapTypeMask;
        uint32 i_phaseMask;
        WorldObject* &i_object;
        Check &i_check;

        WorldObjectLastSearcher(WorldObject const* searcher, WorldObject* & result, Check& check, uint32 mapTypeMask = GRID_MAP_TYPE_MASK_ALL)
            : i_mapTypeMask(mapTypeMask), i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(GameObjectMapType &m);
        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(CorpseMapType &m);
        void Visit(DynamicObjectMapType &m);
        void Visit(AreaTriggerMapType &m);
        void Visit(ConversationMapType &m);
        void Visit(EventObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Check>
    struct WorldObjectListSearcher
    {
        uint32 i_mapTypeMask;
        uint32 i_phaseMask;
        std::list<WorldObject*> &i_objects;
        Check& i_check;

        WorldObjectListSearcher(WorldObject const* searcher, std::list<WorldObject*> &objects, Check & check, uint32 mapTypeMask = GRID_MAP_TYPE_MASK_ALL)
            : i_mapTypeMask(mapTypeMask), i_phaseMask(searcher->GetPhaseMask()), i_objects(objects), i_check(check) {}

        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);
        void Visit(CorpseMapType &m);
        void Visit(GameObjectMapType &m);
        void Visit(DynamicObjectMapType &m);
        void Visit(AreaTriggerMapType &m);
        void Visit(ConversationMapType &m);
        void Visit(EventObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Do>
    struct WorldObjectWorker
    {
        uint32 i_mapTypeMask;
        uint32 i_phaseMask;
        Do const& i_do;

        WorldObjectWorker(WorldObject const* searcher, Do const& _do, uint32 mapTypeMask = GRID_MAP_TYPE_MASK_ALL)
            : i_mapTypeMask(mapTypeMask), i_phaseMask(searcher->GetPhaseMask()), i_do(_do) {}

        void Visit(GameObjectMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_GAMEOBJECT))
                return;

            for (auto &obj : m)
                if (obj->InSamePhase(i_phaseMask))
                    i_do(obj);
        }

        void Visit(PlayerMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_PLAYER))
                return;

            for (auto &player : m)
                if (player->InSamePhase(i_phaseMask))
                    i_do(player);
        }

        void Visit(CreatureMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CREATURE))
                return;

            for (auto &creature : m)
                if (creature->InSamePhase(i_phaseMask))
                    i_do(creature);
        }

        void Visit(CorpseMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CORPSE))
                return;

            for (auto &corpse : m)
                if (corpse->InSamePhase(i_phaseMask))
                    i_do(corpse);
        }

        void Visit(DynamicObjectMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_DYNAMICOBJECT))
                return;

            for (auto &obj : m)
                if (obj->InSamePhase(i_phaseMask))
                    i_do(obj);
        }

        void Visit(AreaTriggerMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_AREATRIGGER))
                return;

            for (auto &trigger : m)
                if (trigger->InSamePhase(i_phaseMask))
                    i_do(trigger);
        }

        void Visit(ConversationMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_CONVERSATION))
                return;

            for (auto &trigger : m)
                if (trigger->InSamePhase(i_phaseMask))
                    i_do(trigger);
        }

        void Visit(EventObjectMapType &m)
        {
            if (!(i_mapTypeMask & GRID_MAP_TYPE_MASK_EVENTOBJECT))
                return;

            for (auto &trigger : m)
                if (trigger->InSamePhase(i_phaseMask))
                    i_do(trigger);
        }

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // Gameobject searchers

    template<class Check>
    struct GameObjectSearcher
    {
        uint32 i_phaseMask;
        GameObject* &i_object;
        Check &i_check;

        GameObjectSearcher(WorldObject const* searcher, GameObject* & result, Check& check)
            : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(GameObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // Last accepted by Check GO if any (Check can change requirements at each call)
    template<class Check>
    struct GameObjectLastSearcher
    {
        uint32 i_phaseMask;
        GameObject* &i_object;
        Check& i_check;

        GameObjectLastSearcher(WorldObject const* searcher, GameObject* & result, Check& check)
            : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(GameObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Check>
    struct GameObjectListSearcher
    {
        uint32 i_phaseMask;
        std::list<GameObject*> &i_objects;
        Check& i_check;

        GameObjectListSearcher(WorldObject const* searcher, std::list<GameObject*> &objects, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_objects(objects), i_check(check) {}

        void Visit(GameObjectMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Functor>
    struct GameObjectWorker
    {
        GameObjectWorker(WorldObject const* searcher, Functor& func)
            : _func(func), _phaseMask(searcher->GetPhaseMask()) {}

        void Visit(GameObjectMapType& m)
        {
            for (auto &obj : m)
                if (obj->InSamePhase(_phaseMask))
                    _func(obj);
        }

        template <typename NotInterested>
        void Visit(NotInterested &) {}

    private:
        Functor& _func;
        uint32 _phaseMask;
    };

    // Unit searchers

    // First accepted by Check Unit if any
    template<class Check>
    struct UnitSearcher
    {
        uint32 i_phaseMask;
        Unit* &i_object;
        Check & i_check;

        UnitSearcher(WorldObject const* searcher, Unit* & result, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(CreatureMapType &m);
        void Visit(PlayerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // Last accepted by Check Unit if any (Check can change requirements at each call)
    template<class Check>
    struct UnitLastSearcher
    {
        uint32 i_phaseMask;
        Unit* &i_object;
        Check & i_check;

        UnitLastSearcher(WorldObject const* searcher, Unit* & result, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(CreatureMapType &m);
        void Visit(PlayerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // All accepted by Check units if any
    template<class Check>
    struct UnitListSearcher
    {
        uint32 i_phaseMask;
        std::list<Unit*> &i_objects;
        Check& i_check;

        UnitListSearcher(WorldObject const* searcher, std::list<Unit*> &objects, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_objects(objects), i_check(check) {}

        void Visit(PlayerMapType &m);
        void Visit(CreatureMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // All accepted by Check units if any
    template<class Check>
    struct AreaTriggerListSearcher
    {
        uint32 i_phaseMask;
        std::list<AreaTrigger*> &i_objects;
        Check& i_check;

        AreaTriggerListSearcher(WorldObject const* searcher, std::list<AreaTrigger*> &objects, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_objects(objects), i_check(check) {}

        void Visit(AreaTriggerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // Creature searchers

    template<class Check>
    struct CreatureSearcher
    {
        uint32 i_phaseMask;
        Creature* &i_object;
        Check & i_check;

        CreatureSearcher(WorldObject const* searcher, Creature* & result, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(CreatureMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // Last accepted by Check Creature if any (Check can change requirements at each call)
    template<class Check>
    struct CreatureLastSearcher
    {
        uint32 i_phaseMask;
        Creature* &i_object;
        Check & i_check;

        CreatureLastSearcher(WorldObject const* searcher, Creature* & result, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(CreatureMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Check>
    struct CreatureListSearcher
    {
        uint32 i_phaseMask;
        std::list<Creature*> &i_objects;
        Check& i_check;

        CreatureListSearcher(WorldObject const* searcher, std::list<Creature*> &objects, Check & check)
            : i_phaseMask(searcher->GetPhaseMask()), i_objects(objects), i_check(check) {}

        void Visit(CreatureMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Do>
    struct CreatureWorker
    {
        uint32 i_phaseMask;
        Do& i_do;

        CreatureWorker(WorldObject const* searcher, Do& _do)
            : i_phaseMask(searcher->GetPhaseMask()), i_do(_do) {}

        void Visit(CreatureMapType &m)
        {
            for (auto &creature : m)
                if (creature && creature->IsInWorld())
                    if (creature->InSamePhase(i_phaseMask))
                        i_do(creature);
        }

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // Player searchers

    template<class Check>
    struct PlayerSearcher
    {
        uint32 i_phaseMask;
        Player* &i_object;
        Check & i_check;

        PlayerSearcher(WorldObject const* searcher, Player* & result, Check & check) : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) {}

        void Visit(PlayerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Check>
    struct PlayerListSearcher
    {
        uint32 i_phaseMask;
        std::list<Player*> &i_objects;
        Check& i_check;

        PlayerListSearcher(WorldObject const* searcher, std::list<Player*> &objects, Check & check) : i_phaseMask(searcher->GetPhaseMask()), i_objects(objects), i_check(check) {}

        void Visit(PlayerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Check>
    struct PlayerLastSearcher
    {
        uint32 i_phaseMask;
        Player* &i_object;
        Check& i_check;

        PlayerLastSearcher(WorldObject const* searcher, Player*& result, Check& check) : i_phaseMask(searcher->GetPhaseMask()), i_object(result), i_check(check) { }

        void Visit(PlayerMapType &m);

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Do>
    struct PlayerWorker
    {
        uint32 i_phaseMask;
        Do& i_do;

        PlayerWorker(WorldObject const* searcher, Do& _do)
            : i_phaseMask(searcher->GetPhaseMask()), i_do(_do) {}

        void Visit(PlayerMapType &m)
        {
            for (auto &player : m)
                if (player->InSamePhase(i_phaseMask))
                    i_do(player);
        }

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    template<class Do>
    struct PlayerDistWorker
    {
        WorldObject const* i_searcher;
        float i_dist;
        Do& i_do;

        PlayerDistWorker(WorldObject const* searcher, float _dist, Do& _do)
            : i_searcher(searcher), i_dist(_dist), i_do(_do) {}

        void Visit(PlayerMapType &m)
        {
            for (auto &player : m)
                if (player->InSamePhase(i_searcher) && player->IsWithinDist(i_searcher, i_dist))
                    i_do(player);
        }

        template <typename NotInterested>
        void Visit(NotInterested &) {}
    };

    // CHECKS && DO classes

    // WorldObject check classes

    class AnyDeadUnitObjectInRangeCheck
    {
    public:
        AnyDeadUnitObjectInRangeCheck(Unit* searchObj, float range);
        bool operator()(Player* u);
        bool operator()(Corpse* u);
        bool operator()(Creature* u);
        template<class NOT_INTERESTED> bool operator()(NOT_INTERESTED*) { return false; }
    protected:
        Unit const* const i_searchObj;
        float i_range;
    };

    class AnyDeadUnitSpellTargetInRangeCheck : public AnyDeadUnitObjectInRangeCheck
    {
    public:
        AnyDeadUnitSpellTargetInRangeCheck(Unit* searchObj, float range, SpellInfo const* spellInfo, SpellTargetCheckTypes check);
        bool operator()(Player* u);
        bool operator()(Corpse* u);
        bool operator()(Creature* u);
        template<class NOT_INTERESTED> bool operator()(NOT_INTERESTED*) { return false; }
    protected:
        SpellInfo const* i_spellInfo;
        WorldObjectSpellTargetCheck i_check;
    };

    // WorldObject do classes

    class RespawnDo
    {
    public:
        RespawnDo() {}
        void operator()(Creature* u) const { u->Respawn(false); }
        void operator()(GameObject* u) const { u->Respawn(); }
        void operator()(WorldObject*) const {}
        void operator()(Corpse*) const {}
    };

    // GameObject checks

    class GameObjectFocusCheck
    {
    public:
        GameObjectFocusCheck(Unit const* unit, uint32 focusId);
        bool operator()(GameObject* go) const;
    private:
        Unit const* i_unit;
        uint32 i_focusId;
    };

    // Find the nearest Fishing hole and return true only if source object is in range of hole
    class NearestGameObjectFishingHole
    {
    public:
        NearestGameObjectFishingHole(WorldObject const& obj, float range);
        bool operator()(GameObject* go);
        float GetLastRange() const { return i_range; }
    private:
        WorldObject const& i_obj;
        float  i_range;

        // prevent clone
        NearestGameObjectFishingHole(NearestGameObjectFishingHole const&) = delete;
    };

    class NearestGameObjectCheck
    {
    public:
        NearestGameObjectCheck(WorldObject const& obj);
        bool operator()(GameObject* go);
        float GetLastRange() const { return i_range; }
    private:
        WorldObject const& i_obj;
        float i_range;

        // prevent clone this object
        NearestGameObjectCheck(NearestGameObjectCheck const&) = delete;
    };

    // Success at unit in range, range update for next check (this can be use with GameobjectLastSearcher to find nearest GO)
    class NearestGameObjectEntryInObjectRangeCheck
    {
    public:
        NearestGameObjectEntryInObjectRangeCheck(WorldObject const& obj, uint32 entry, float range);
        bool operator()(GameObject* go);
        float GetLastRange() const { return i_range; }
    private:
        WorldObject const& i_obj;
        uint32 i_entry;
        float  i_range;

        // prevent clone this object
        NearestGameObjectEntryInObjectRangeCheck(NearestGameObjectEntryInObjectRangeCheck const&) = delete;
    };

    // Success at unit in range, range update for next check (this can be use with GameobjectLastSearcher to find nearest GO with a certain type)
    class NearestGameObjectTypeInObjectRangeCheck
    {
    public:
        NearestGameObjectTypeInObjectRangeCheck(WorldObject const& obj, GameobjectTypes type, float range);
        bool operator()(GameObject* go);
        float GetLastRange() const { return i_range; }
    private:
        WorldObject const& i_obj;
        GameobjectTypes i_type;
        float  i_range;

        // prevent clone this object
        NearestGameObjectTypeInObjectRangeCheck(NearestGameObjectTypeInObjectRangeCheck const&) = delete;

    };

    class GameObjectWithDbGUIDCheck
    {
    public:
        GameObjectWithDbGUIDCheck(ObjectGuid::LowType db_guid);
        bool operator()(GameObject const* go) const;
    private:
        ObjectGuid::LowType i_db_guid;
    };

    // Unit checks

    class MostHPMissingInRange
    {
    public:
        MostHPMissingInRange(Unit const* obj, float range, uint32 hp);
        bool operator()(Unit* u);
    private:
        Unit const* i_obj;
        float i_range;
        uint32 i_hp;
    };

    class FriendlyCCedInRange
    {
    public:
        FriendlyCCedInRange(Unit const* obj, float range);
        bool operator()(Unit* u);
    private:
        Unit const* i_obj;
        float i_range;
    };

    class FriendlyMissingBuffInRange
    {
    public:
        FriendlyMissingBuffInRange(Unit const* obj, float range, uint32 spellid);
        bool operator()(Unit* u);
    private:
        Unit const* i_obj;
        float i_range;
        uint32 i_spell;
    };

    class AnyUnfriendlyUnitInObjectRangeCheck
    {
    public:
        AnyUnfriendlyUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        Unit const* i_funit;
        float i_range;
    };

    class AnyUnfriendlyNoTotemUnitInObjectRangeCheck
    {
    public:
        AnyUnfriendlyNoTotemUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        Unit const* i_funit;
        float i_range;
    };

    class AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck
    {
    public:
        AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck(Unit const* funit, float range, bool checkin);
        bool operator()(const Unit* u);
    private:
        Unit const* i_funit;
        float i_range;
        bool i_checkin;
    };

    class CreatureWithDbGUIDCheck
    {
    public:
        CreatureWithDbGUIDCheck(ObjectGuid::LowType const& lowguid);
        bool operator()(Creature* u);
    private:
        ObjectGuid::LowType i_lowguid;
    };

    class AnyFriendlyUnitInObjectRangeCheck
    {
    public:
        AnyFriendlyUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        Unit const* i_funit;
        float i_range;
    };

    class AnyUnitHavingBuffInObjectRangeCheck
    {
    public:
        AnyUnitHavingBuffInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range, uint32 spellid, bool isfriendly);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        Unit const* i_funit;
        float i_range;
        uint32 i_spellid;
        bool i_friendly;
    };

    class AnyGroupedUnitInObjectRangeCheck
    {
    public:
        AnyGroupedUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range, bool raid);
        bool operator()(Unit* u);
    private:
        WorldObject const* _source;
        Unit const* _refUnit;
        float _range;
        bool _raid;
    };

    class AnyUnitInObjectRangeCheck
    {
    public:
        AnyUnitInObjectRangeCheck(WorldObject const* obj, float range, bool aliveOnly = true);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        float i_range;
        bool i_aliveOnly;
    };

    class AreaTriggerWithEntryInObjectRangeCheck
    {
    public:
        AreaTriggerWithEntryInObjectRangeCheck(WorldObject const* obj, uint32 entry, ObjectGuid casterGuid, float range);
        bool operator()(AreaTrigger* at);
    private:
        WorldObject const* i_obj;
        float i_range;
        uint32 i_entry;
        ObjectGuid i_casterGuid;
    };

    // Success at unit in range, range update for next check (this can be use with UnitLastSearcher to find nearest unit)
    class NearestAttackableUnitInObjectRangeCheck
    {
    public:
        NearestAttackableUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        Unit const* i_funit;
        float i_range;

        // prevent clone this object
        NearestAttackableUnitInObjectRangeCheck(NearestAttackableUnitInObjectRangeCheck const&) = delete;
    };

    // Success at unit in range, range update for next check (this can be use with UnitLastSearcher to find nearest unit)
    class NearestAttackableNoCCUnitInObjectRangeCheck
    {
    public:
        NearestAttackableNoCCUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range);
        bool operator()(Unit* u);
    private:
        WorldObject const* i_obj;
        Unit const* i_funit;
        float i_range;

        // prevent clone this object
        NearestAttackableNoCCUnitInObjectRangeCheck(NearestAttackableNoCCUnitInObjectRangeCheck const&) = delete;
    };

    class AnyAoETargetUnitInObjectRangeCheck
    {
    public:
        AnyAoETargetUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range);
        bool operator()(Unit* u);
    private:
        bool i_targetForPlayer;
        WorldObject const* i_obj;
        Unit const* i_funit;
        SpellInfo const* _spellInfo;
        float i_range;
    };

    // do attack at call of help to friendly crearture
    class CallOfHelpCreatureInRangeDo
    {
    public:
        CallOfHelpCreatureInRangeDo(Unit* funit, Unit* enemy, float range);
        void operator()(Creature* u);
    private:
        Unit* const i_funit;
        Unit* const i_enemy;
        float i_range;
    };

    struct AnyDeadUnitCheck
    {
        bool operator()(Unit* u);
    };

    // Creature checks

    class NearestHostileUnitCheck
    {
    public:
        explicit NearestHostileUnitCheck(Unit const* creature, float dist = 0);
        bool operator()(Unit* u);
    private:
        Unit const* me;
        float m_range;
        NearestHostileUnitCheck(NearestHostileUnitCheck const&) = delete;
    };

    class NearestHostileNoCCUnitCheck
    {
    public:
        explicit NearestHostileNoCCUnitCheck(Creature const* creature, float dist = 0);
        bool operator()(Unit* u);
    private:
        Creature const *me;
        float m_range;
        NearestHostileNoCCUnitCheck(NearestHostileNoCCUnitCheck const&) = delete;
    };

    class NearestHostileUnitInAttackDistanceCheck
    {
    public:
        explicit NearestHostileUnitInAttackDistanceCheck(Creature const* creature, float dist = 0);
        bool operator()(Unit* u);
        float GetLastRange() const { return m_range; }
    private:
        Creature const* me;
        float m_range;
        bool m_force;
        NearestHostileUnitInAttackDistanceCheck(NearestHostileUnitInAttackDistanceCheck const&) = delete;
    };

    class NearestHostileUnitInAggroRangeCheck
    {
    public:
        explicit NearestHostileUnitInAggroRangeCheck(Creature const* creature, bool useLOS = false);
        bool operator()(Unit* u);
    private:
        Creature const* _me;
        bool _useLOS;
        NearestHostileUnitInAggroRangeCheck(NearestHostileUnitInAggroRangeCheck const&) = delete;
    };

    class AnyAssistCreatureInRangeCheck
    {
    public:
        AnyAssistCreatureInRangeCheck(Unit* funit, Unit* enemy, float range);
        bool operator()(Creature* u);
    private:
        Unit* const i_funit;
        Unit* const i_enemy;
        float i_range;
    };

    class NearestAssistCreatureInCreatureRangeCheck
    {
    public:
        NearestAssistCreatureInCreatureRangeCheck(Creature* obj, Unit* enemy, float range);
        bool operator()(Creature* u);
        float GetLastRange() const { return i_range; }
    private:
        Creature* const i_obj;
        Unit* const i_enemy;
        float  i_range;

        // prevent clone this object
        NearestAssistCreatureInCreatureRangeCheck(NearestAssistCreatureInCreatureRangeCheck const&) = delete;
    };

    // Success at unit in range, range update for next check (this can be use with CreatureLastSearcher to find nearest creature)
    class NearestCreatureEntryWithLiveStateInObjectRangeCheck
    {
    public:
        NearestCreatureEntryWithLiveStateInObjectRangeCheck(WorldObject const& obj, uint32 entry, bool alive, float range);
        bool operator()(Creature* u);
        float GetLastRange() const { return i_range; }
    private:
        WorldObject const& i_obj;
        uint32 i_entry;
        bool   i_alive;
        float  i_range;

        // prevent clone this object
        NearestCreatureEntryWithLiveStateInObjectRangeCheck(NearestCreatureEntryWithLiveStateInObjectRangeCheck const&) = delete;
    };

    class AnyPlayerInObjectRangeCheck
    {
    public:
        AnyPlayerInObjectRangeCheck(WorldObject const* obj, float range, bool reqAlive = true);
        bool operator()(Player* u);
    private:
        WorldObject const* _obj;
        float _range;
        bool _reqAlive;
    };

    class NearestPlayerInObjectRangeCheck
    {
    public:
        NearestPlayerInObjectRangeCheck(WorldObject const* obj, float range);
        bool operator()(Player* u);
    private:
        WorldObject const* i_obj;
        float i_range;

        NearestPlayerInObjectRangeCheck(NearestPlayerInObjectRangeCheck const&) = delete;
    };

    class NearestPlayerNotGMInObjectRangeCheck
    {
    public:
        NearestPlayerNotGMInObjectRangeCheck(WorldObject const* obj, float range);
        bool operator()(Player* u);
    private:
        WorldObject const* i_obj;
        float i_range;

        NearestPlayerNotGMInObjectRangeCheck(NearestPlayerNotGMInObjectRangeCheck const&) = delete;
    };

    class AllFriendlyCreaturesInGrid
    {
    public:
        AllFriendlyCreaturesInGrid(Unit const* obj);
        bool operator()(Unit* u);
    private:
        Unit const* unit;
    };

    class AllGameObjectsWithEntryInRange
    {
    public:
        AllGameObjectsWithEntryInRange(const WorldObject* object, uint32 entry, float maxRange);
        bool operator()(GameObject* go);
    private:
        const WorldObject* m_pObject;
        uint32 m_uiEntry;
        float m_fRange;
    };

    class AllCreaturesOfEntryInRange
    {
    public:
        AllCreaturesOfEntryInRange(const WorldObject* object, uint32 entry, float maxRange);
        bool operator()(Unit* unit);
    private:
        const WorldObject* m_pObject;
        uint32 m_uiEntry;
        float m_fRange;
    };

    class AllCreaturesInRange
    {
    public:
        AllCreaturesInRange(const WorldObject* object, float maxRange);
        bool operator()(Unit* unit);
    private:
        const WorldObject* m_pObject;
        float m_fRange;
    };

    class AllAreaTriggeresOfEntryInRange
    {
    public:
        AllAreaTriggeresOfEntryInRange(const WorldObject* object, uint32 entry, float maxRange);
        bool operator()(AreaTrigger* at);
    private:
        const WorldObject* m_pObject;
        uint32 m_uiEntry;
        float m_fRange;
    };

    class AllAliveCreaturesOfEntryInRange
    {
    public:
        AllAliveCreaturesOfEntryInRange(const WorldObject* object, uint32 entry, float maxRange);
        bool operator()(Unit* unit);
    private:
        const WorldObject* m_pObject;
        uint32 m_uiEntry;
        float m_fRange;
    };

    class SearchCorpseCreatureCheck
    {
    public:
        SearchCorpseCreatureCheck(const WorldObject* object, float range);
        bool operator()(Creature* u);
    private:
        Player* m_owner;
        const WorldObject* m_pObject;
        float i_range;
    };

    class AllDeadCreaturesInRange
    {
    public:
        AllDeadCreaturesInRange(const WorldObject* object, float maxRange, ObjectGuid excludeGUID);
        bool operator()(Unit const* unit) const;
    private:
        const WorldObject* m_pObject;
        float m_fRange;
        ObjectGuid m_excludeGUID;
    };

    class PlayerAtMinimumRangeAway
    {
    public:
        PlayerAtMinimumRangeAway(Unit const* unit, float fMinRange);
        bool operator()(Player* player);
    private:
        Unit const* unit;
        float fRange;
    };

    class GameObjectInRangeCheck
    {
    public:
        GameObjectInRangeCheck(float _x, float _y, float _z, float _range, uint32 _entry = 0);
        bool operator()(GameObject* go);
    private:
        float x, y, z, range;
        uint32 entry;
    };

    class AllWorldObjectsInRange
    {
    public:
        AllWorldObjectsInRange(const WorldObject* object, float maxRange);
        bool operator()(WorldObject* go);
    private:
        const WorldObject* m_pObject;
        float m_fRange;
    };

    class ObjectTypeIdCheck
    {
    public:
        ObjectTypeIdCheck(TypeID typeId, bool equals);
        bool operator()(WorldObject* object);
    private:
        TypeID _typeId;
        bool _equals;
    };

    class ObjectGUIDCheck
    {
    public:
        ObjectGUIDCheck(ObjectGuid GUID);
        bool operator()(WorldObject* object);
    private:
        ObjectGuid _GUID;
    };

    class UnitAuraCheck
    {
    public:
        UnitAuraCheck(bool present, uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        bool _present;
        uint32 _spellId;
        ObjectGuid _casterGUID;
    };

    class UnitAuraTypeCheck
    {
    public:
        UnitAuraTypeCheck(bool present, AuraType type);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        bool _present;
        AuraType _type;
    };

    class MailBoxMasterCheck
    {
    public:
        MailBoxMasterCheck(Player* plr);
        bool operator()(GameObject* u);
    private:
        Player* _plr;
    };

    // Player checks and do

    // Prepare using Builder localized packets with caching and send to player
    template<class Builder>
    class LocalizedPacketDo
    {
    public:
        explicit LocalizedPacketDo(Builder& builder) : _builder(builder) { }

        ~LocalizedPacketDo()
        {
            for (size_t i = 0; i < _dataCache.size(); ++i)
                delete _dataCache[i];
        }

        void operator()(Player* p);

    private:
        Builder& _builder;
        std::vector<WorldPackets::Packet*> _dataCache;         // 0 = default, i => i-1 locale index
    };

    template<class Builder>
    class LocalizedPacketListDo
    {
    public:
        typedef std::vector<WorldPackets::Packet*> WorldPacketList;
        explicit LocalizedPacketListDo(Builder& builder) : _builder(builder) { }

        ~LocalizedPacketListDo()
        {
            for (size_t i = 0; i < _dataCache.size(); ++i)
                for (size_t j = 0; j < _dataCache[i].size(); ++j)
                    delete _dataCache[i][j];
        }
        void operator()(Player* p);

    private:
        Builder& _builder;
        std::vector<WorldPacketList> _dataCache;
        // 0 = default, i => i-1 locale index
    };

    class SummonTimerOrderPred
    {
    public:
        SummonTimerOrderPred(bool ascending = true);
        bool operator()(const Unit* a, const Unit* b) const;
    private:
        const bool m_ascending;
    };

    class UnitHealthState
    {
    public:
        UnitHealthState(bool sortlow);
        bool operator()(Unit* unitA, Unit* unitB) const;
        bool operator()(WorldObject* objectA, WorldObject* objectB) const;
    private:
        bool _sortlow;
    };

    class UnitDistanceCheck
    {
    public:
        UnitDistanceCheck(bool checkin, Unit* caster, float dist);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        bool _checkin;
        Unit* _caster;
        float _dist;
    };

    class DestDistanceCheck
    {
    public:
        DestDistanceCheck(bool checkin, Position* dest, float dist);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        bool _checkin;
        Position* _dest;
        float _dist;
    };

    class UnitTypeCheck
    {
    public:
        UnitTypeCheck(bool checkin, uint32 typeMask);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        bool _checkin;
        uint32 _typeMask;
    };

    class UnitSortDistance
    {
    public:
        UnitSortDistance(bool sortlow, WorldObject* caster);
        bool operator()(Unit* unitA, Unit* unitB) const;
        bool operator()(WorldObject* objectA, WorldObject* objectB) const;
    private:
        bool _sortlow;
        WorldObject* _caster;
    };

    class UnitFriendlyCheck
    {
    public:
        UnitFriendlyCheck(bool present, Unit* caster);
        bool operator()(Unit* unit);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };

    class UnitRaidCheck
    {
    public:
        UnitRaidCheck(bool present, Unit* caster);
        bool operator()(Unit* unit);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };

    class UnitPartyCheck
    {
    public:
        UnitPartyCheck(bool present, Unit* caster);
        bool operator()(Unit* unit);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };

    class UnitCheckInLos
    {
    public:
        UnitCheckInLos(bool present, Unit* caster);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };

    class UnitCheckInBetween
    {
    public:
        UnitCheckInBetween(bool present, Unit* caster, Unit* target, float size);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
        Unit* _target;
        float _size;
    };

    class UnitCheckInBetweenShift
    {
    public:
        UnitCheckInBetweenShift(bool present, Unit* caster, Unit* target, float size, float shift, float angleShift);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
        Unit* _target;
        float _size;
        float _shift;
        float _angleShift;
    };

    class UnitCheckCCAura
    {
    public:
        UnitCheckCCAura(bool present, Unit* caster);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };

    class UnitAuraAndCheck
    {
    public:
        UnitAuraAndCheck(int32 aura1, int32 aura2 = 0, int32 aura3 = 0, ObjectGuid casterGUID = ObjectGuid::Empty);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        int32 _aura1;
        int32 _aura2;
        int32 _aura3;
        ObjectGuid _casterGUID;
    };

    class UnitAuraOrCheck
    {
    public:
        UnitAuraOrCheck(int32 aura1, int32 aura2, int32 aura3, ObjectGuid casterGUID = ObjectGuid::Empty);
        bool operator()(Unit* unit) const;
        bool operator()(WorldObject* object) const;
    private:
        int32 _aura1;
        int32 _aura2;
        int32 _aura3;
        ObjectGuid _casterGUID;
    };

    class UnitEntryCheck
    {
    public:
        UnitEntryCheck(int32 entry1, int32 entry2, int32 entry3);
        bool operator()(WorldObject* object) const;
    private:
        int32 _entry1;
        int32 _entry2;
        int32 _entry3;
    };

    class UnitAttackableCheck
    {
    public:
        UnitAttackableCheck(bool present, Unit* caster);
        bool operator()(Unit* unit);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };

    class EntryCheckPredicate
    {
    public:
        EntryCheckPredicate(uint32 entry);
        bool operator()(ObjectGuid guid);
    private:
        uint32 _entry;
    };

    class UnitFullHPCheck
    {
    public:
        UnitFullHPCheck(bool full);
        bool operator()(Unit* unit);
        bool operator()(WorldObject* object);
    private:
        bool _full;
    };

    class UnitOwnerCheck
    {
    public:
        UnitOwnerCheck(bool present, Unit* caster);
        bool operator()(Unit* unit);
        bool operator()(WorldObject* object);
    private:
        bool _present;
        Unit* _caster;
    };
}
#endif
