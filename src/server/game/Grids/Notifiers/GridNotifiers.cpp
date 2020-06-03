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

#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Transport.h"
#include "ObjectAccessor.h"
#include "CellImpl.h"
#include "SocialMgr.h"

namespace {

bool shouldCallMoveInLineOfSight(Creature const *c, Unit const *u)
{
    return c != u && c->IsAIEnabled && c->isAlive() && u->isAlive() && !u->isInFlight() && !c->HasUnitState(UNIT_STATE_SIGHTLESS) && (c->HasReactState(REACT_AGGRESSIVE) || c->AI()->CanSeeEvenInPassiveMode()) && c->canSeeOrDetect(u, false, true);
}

} // namespace

using namespace Trinity;

VisibleNotifier::VisibleNotifier(Player& player) : i_player(player), i_data(player.GetMapId()), vis_guids(player.m_clientGUIDs)
{
}

void VisibleNotifier::AddMaxVisible()
{
    if (Map* map = i_player.GetMap())
    {
        for (auto object : map->m_MaxVisibleList)
        {
            if (!object->IsInWorld())
                continue;

            if (!i_player.InSamePhase((WorldObject*)object))
                continue;

            vis_guids.erase(object->GetGUID());
            switch (object->GetTypeId())
            {
                case TYPEID_GAMEOBJECT:
                    i_player.UpdateVisibilityOf(object->ToGameObject(), i_data, i_visibleNow);
                    break;
                case TYPEID_UNIT:
                    i_player.UpdateVisibilityOf(object->ToCreature(), i_data, i_visibleNow);
                    break;
                default:
                    break;
            }
        }
    }
}

void VisibleNotifier::SendToSelf()
{
    // at this moment i_clientGUIDs have guids that not iterate at grid level checks
    // but exist one case when this possible and object not out of range: transports
    Transport* transport = i_player.GetTransport();
    if (transport && transport->GetMap() == i_player.GetMap())
    {
        for (WorldObjectSet::iterator itr = transport->GetPassengers().begin(); itr != transport->GetPassengers().end(); ++itr)
        {
            WorldObject* obj = (*itr);
            if (!obj || !obj->IsInWorld())
                continue;

            if (vis_guids.find((*itr)->GetGUID()) != vis_guids.end())
            {
                vis_guids.erase((*itr)->GetGUID());

                switch ((*itr)->GetTypeId())
                {
                    case TYPEID_GAMEOBJECT:
                        i_player.UpdateVisibilityOf((*itr)->ToGameObject(), i_data, i_visibleNow);
                        break;
                    case TYPEID_PLAYER:
                        i_player.UpdateVisibilityOf((*itr)->ToPlayer(), i_data, i_visibleNow);
                        (*itr)->ToPlayer()->UpdateVisibilityOf(&i_player);
                        break;
                    case TYPEID_UNIT:
                        i_player.UpdateVisibilityOf((*itr)->ToCreature(), i_data, i_visibleNow);
                        break;
                    case TYPEID_DYNAMICOBJECT:
                        i_player.UpdateVisibilityOf((*itr)->ToDynObject(), i_data, i_visibleNow);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    for (auto it = vis_guids.begin(); it != vis_guids.end(); ++it)
    {
        // extralook shouldn't be removed by missing creature in grid where is curently player
        if (i_player.IsOnVehicle() || i_player.GetVehicleKit() != nullptr)
            if (i_player.HaveExtraLook(*it))
                continue;

        i_player.RemoveClient(*it);
        i_data.AddOutOfRangeGUID(*it);

        if ((*it).IsPlayer())
        {
            Player* player = ObjectAccessor::FindPlayer(*it);
            if (player && player->IsInWorld()/* && !player->onVisibleUpdate()*/)
                player->UpdateVisibilityOf(&i_player);
        }
    }

    if (!i_data.HasData())
        return;

    WorldPacket packet;
    if (i_data.BuildPacket(&packet))
        i_player.GetSession()->SendPacket(&packet);

    for (std::set<Unit*>::const_iterator it = i_visibleNow.begin(); it != i_visibleNow.end(); ++it)
        i_player.SendInitialVisiblePackets(*it);
}

void VisibleChangesNotifier::Visit(PlayerMapType &m)
{
    for (auto &source : m)
    {
        if (source == &i_object)
            continue;

        source->UpdateVisibilityOf(&i_object);

        if (source->GetSharedVisionList().empty())
            continue;

        for (auto &player : source->GetSharedVisionList())
            if (player->m_seer == source)
                player->UpdateVisibilityOf(&i_object);
    }
}

VisibleChangesNotifier::VisibleChangesNotifier(WorldObject& object) : i_object(object)
{
}

void VisibleChangesNotifier::Visit(GameObjectMapType &m)
{
    for (auto &source : m)
    {
        if (i_object.IsPlayer() && source->IsGameObject())
        {
            float dist = source->GetDistance(&i_object);
            if (source->m_RateUpdateTimer > dist)
            {
                source->m_RateUpdateTimer = dist;
                source->m_RateUpdateWait = 15 * IN_MILLISECONDS;
            }
        }
    }
}

void VisibleChangesNotifier::Visit(CreatureMapType &m)
{
    for (auto &source : m)
    {
        if (i_object.IsPlayer() && source->IsCreature())
        {
            float dist = source->GetDistance(&i_object);
            if (source->m_RateUpdateTimer > dist)
            {
                source->m_RateUpdateTimer = dist;
                source->m_RateUpdateWait = 15 * IN_MILLISECONDS;
            }
        }

        if (source->GetSharedVisionList().empty())
            continue;

        for (auto &player : source->GetSharedVisionList())
            if (player->m_seer == source)
                player->UpdateVisibilityOf(&i_object);
    }
}

void VisibleChangesNotifier::Visit(DynamicObjectMapType &m)
{
    for (auto &obj : m)
    {
        auto const guid = obj->GetCasterGUID();
        if (!guid.IsPlayer())
            continue;

        auto const caster = obj->GetCaster()->ToPlayer();
        if (caster && caster->m_seer == obj)
            caster->UpdateVisibilityOf(&i_object);
    }
}

void VisibleChangesNotifier::Visit(AreaTriggerMapType &m)
{
    for (auto &obj : m)
    {
        auto const guid = obj->GetCasterGUID();
        if (!guid.IsPlayer())
            continue;

        auto const caster = obj->GetCaster()->ToPlayer();
        if (caster && caster->m_seer == obj)
            caster->UpdateVisibilityOf(&i_object);
    }
}

void VisibleChangesNotifier::Visit(ConversationMapType &m)
{
    for (auto &obj : m)
    {
        auto const guid = obj->GetCasterGUID();
        if (!guid.IsPlayer())
            continue;

        auto const caster = obj->GetCaster()->ToPlayer();
        if (caster && caster->m_seer == obj)
            caster->UpdateVisibilityOf(&i_object);
    }
}

AIRelocationNotifier::AIRelocationNotifier(Unit& unit) : unit_(&unit)
{
}

void AIRelocationNotifier::Visit(CreatureMapType &m)
{
    if (unit_->IsCreature())
    {
        for (auto &creature : m)
        {
            if (shouldCallMoveInLineOfSight(creature, unit_))
            {
                auto const both = shouldCallMoveInLineOfSight(static_cast<Creature*>(unit_), creature);
                std::lock_guard<std::recursive_mutex> guard(i_movedInLosLock);
                movedInLos_.emplace_back(creature, both);
            }
        }
    }
    else
    {
        for (auto &creature : m)
        {
            if (unit_->IsPlayer())
            {
                float dist = creature->GetDistance(unit_);
                if (creature->m_RateUpdateTimer > dist)
                {
                    creature->m_RateUpdateTimer = dist;
                    creature->m_RateUpdateWait = 15 * IN_MILLISECONDS;
                }
            }
            if (shouldCallMoveInLineOfSight(creature, unit_))
            {
                std::lock_guard<std::recursive_mutex> guard(i_movedInLosLock);
                movedInLos_.emplace_back(creature, false);
            }
        }
    }

    if (movedInLos_.empty())
        return;

    for (auto &pair : movedInLos_)
    {
        pair.first->AI()->MoveInLineOfSight_Safe(unit_);
        if (pair.second)
            static_cast<Creature*>(unit_)->AI()->MoveInLineOfSight_Safe(pair.first);
    }

    std::lock_guard<std::recursive_mutex> guard(i_movedInLosLock);
    movedInLos_.clear();
}

void AIRelocationNotifier::Visit(EventObjectMapType &m)
{
    for (auto &event : m)
        event->MoveInLineOfSight(unit_);
}

MessageDistDeliverer::MessageDistDeliverer(WorldObject* src, WorldPacket const* msg, float dist, bool own_team_only, Player const* skipped, GuidUnorderedSet ignoredSet) :
    i_source(src), i_message(msg), i_phaseMask(src->GetPhaseMask()), i_distSq(dist * dist), team((own_team_only && src->IsPlayer()) ? src->ToPlayer()->GetTeam() : 0), skipped_receiver(skipped), m_IgnoredGUIDs(ignoredSet)
{
}

void MessageDistDeliverer::Visit(PlayerMapType &m)
{
    for (auto &target : m)
    {
        if (!target->InSamePhase(target))
            continue;

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        // Send packet to all who are sharing the player's vision
        if (!target->GetSharedVisionList().empty())
        {
            for (auto &player : target->GetSharedVisionList())
                if (player->m_seer == target)
                    SendPacket(player);
        }

        if (target->m_seer == target || target->GetVehicle())
            SendPacket(target);
    }
}

void MessageDistDeliverer::Visit(CreatureMapType &m)
{
    for (auto &target : m)
    {
        if (!target->InSamePhase(target))
            continue;

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        // Send packet to all who are sharing the creature's vision
        if (!target->GetSharedVisionList().empty())
        {
            for (auto &player : target->GetSharedVisionList())
                if (player->m_seer == target)
                    SendPacket(player);
        }
    }
}

void MessageDistDeliverer::Visit(DynamicObjectMapType &m)
{
    for (auto &target : m)
    {
        if (!target->InSamePhase(target))
            continue;

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        if (target->GetCasterGUID().IsPlayer())
        {
            // Send packet back to the caster if the caster has vision of dynamic object
            auto const caster = target->GetCaster()->ToPlayer();
            if (caster && caster->m_seer == target)
                SendPacket(caster);
        }
    }
}

void MessageDistDeliverer::SendPacket(Player* player)
{
    // never send packet to self
    if (player == i_source || (team && player->GetTeam() != team) || skipped_receiver == player)
        return;

    if (m_IgnoredGUIDs.find(player->GetGUID()) != m_IgnoredGUIDs.end())
        return;

    if (!player->HaveAtClient(i_source))
        return;

    if (i_message->GetOpcode() == SMSG_CHAT && player->GetSocial()->HasIgnore(i_source->GetGUID()))
        return;

    player->SendDirectMessage(i_message);
}

UnfriendlyMessageDistDeliverer::UnfriendlyMessageDistDeliverer(Unit const* src, WorldPacket* msg, float dist) : i_source(src), i_message(msg), i_phaseMask(src->GetPhaseMask()), i_distSq(dist * dist)
{
}

void UnfriendlyMessageDistDeliverer::Visit(PlayerMapType &m)
{
    for (auto &target : m)
    {
        if (!target->InSamePhase(target))
            continue;

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        // Send packet to all who are sharing the player's vision
        if (!target->GetSharedVisionList().empty())
        {
            for (auto &player : target->GetSharedVisionList())
                if (player->m_seer == target)
                    SendPacket(player);
        }

        if (target->m_seer == target || target->GetVehicle())
            SendPacket(target);
    }
}

void UnfriendlyMessageDistDeliverer::SendPacket(Player* player)
{
    // never send packet to self
    if (player == i_source || (player->IsFriendlyTo(i_source)))
        return;

    if (!player->HaveAtClient(i_source))
        return;

    player->SendDirectMessage(i_message);

    if (i_message->GetOpcode() == SMSG_CLEAR_TARGET)
    {
        for (uint32 i = CURRENT_MELEE_SPELL; i < CURRENT_AUTOREPEAT_SPELL; ++i)
        {
            Spell* spell = player->GetCurrentSpell(i);
            if (spell && spell->m_targets.GetUnitTarget() == i_source && spell->getState() == SPELL_STATE_CASTING)
                spell->cancel();
        }
    }
}

AnyDeadUnitObjectInRangeCheck::AnyDeadUnitObjectInRangeCheck(Unit* searchObj, float range) : i_searchObj(searchObj), i_range(range)
{
}

bool AnyDeadUnitObjectInRangeCheck::operator()(Player* u)
{
    return !u->isAlive() && !u->HasAuraType(SPELL_AURA_GHOST) && i_searchObj->IsWithinDistInMap(u, i_range);
}

bool AnyDeadUnitObjectInRangeCheck::operator()(Corpse* u)
{
    return u->GetType() != CORPSE_BONES && i_searchObj->IsWithinDistInMap(u, i_range);
}

bool AnyDeadUnitObjectInRangeCheck::operator()(Creature* u)
{
    return !u->isAlive() && i_searchObj->IsWithinDistInMap(u, i_range);
}

AnyDeadUnitSpellTargetInRangeCheck::AnyDeadUnitSpellTargetInRangeCheck(Unit* searchObj, float range, SpellInfo const* spellInfo, SpellTargetCheckTypes check) :
    AnyDeadUnitObjectInRangeCheck(searchObj, range), i_spellInfo(spellInfo), i_check(searchObj, searchObj, spellInfo, check, nullptr)
{
}

bool AnyDeadUnitSpellTargetInRangeCheck::operator()(Player* u)
{
    return AnyDeadUnitObjectInRangeCheck::operator()(u) && i_check(u);
}

bool AnyDeadUnitSpellTargetInRangeCheck::operator()(Corpse* u)
{
    return AnyDeadUnitObjectInRangeCheck::operator()(u) && i_check(u);
}

bool AnyDeadUnitSpellTargetInRangeCheck::operator()(Creature* u)
{
    return AnyDeadUnitObjectInRangeCheck::operator()(u) && i_check(u);
}

GameObjectFocusCheck::GameObjectFocusCheck(Unit const* unit, uint32 focusId) : i_unit(unit), i_focusId(focusId)
{
}

bool GameObjectFocusCheck::operator()(GameObject* go) const
{
    if (go->GetGOInfo()->type != GAMEOBJECT_TYPE_SPELL_FOCUS && go->GetGOInfo()->type != GAMEOBJECT_TYPE_UI_LINK)
        return false;

    if (go->GetGOInfo()->spellFocus.spellFocusType != i_focusId && go->GetGOInfo()->UILink.spellFocusType != i_focusId)
        return false;

    float dist = go->GetGOInfo()->type == GAMEOBJECT_TYPE_SPELL_FOCUS ? static_cast<float>(go->GetGOInfo()->spellFocus.radius) : static_cast<float>(go->GetGOInfo()->UILink.radius);
    return go->IsWithinDistInMap(i_unit, dist);
}

NearestGameObjectFishingHole::NearestGameObjectFishingHole(WorldObject const& obj, float range) : i_obj(obj), i_range(range)
{
}

bool NearestGameObjectFishingHole::operator()(GameObject* go)
{
    if (go->GetGOInfo()->type == GAMEOBJECT_TYPE_FISHINGHOLE && go->isSpawned() && i_obj.IsWithinDistInMap(go, i_range) && i_obj.IsWithinDistInMap(go, static_cast<float>(go->GetGOInfo()->fishingHole.radius)))
    {
        i_range = i_obj.GetDistance(go);
        return true;
    }
    return false;
}

NearestGameObjectCheck::NearestGameObjectCheck(WorldObject const& obj) : i_obj(obj), i_range(999)
{
}

bool NearestGameObjectCheck::operator()(GameObject* go)
{
    if (i_obj.IsWithinDistInMap(go, i_range))
    {
        i_range = i_obj.GetDistance(go); // use found GO range as new range limit for next check
        return true;
    }
    return false;
}

NearestGameObjectEntryInObjectRangeCheck::NearestGameObjectEntryInObjectRangeCheck(WorldObject const& obj, uint32 entry, float range) : i_obj(obj), i_entry(entry), i_range(range)
{
}

bool NearestGameObjectEntryInObjectRangeCheck::operator()(GameObject* go)
{
    if (go->GetEntry() == i_entry && i_obj.IsWithinDistInMap(go, i_range))
    {
        i_range = i_obj.GetDistance(go); // use found GO range as new range limit for next check
        return true;
    }
    return false;
}

NearestGameObjectTypeInObjectRangeCheck::NearestGameObjectTypeInObjectRangeCheck(WorldObject const& obj, GameobjectTypes type, float range) : i_obj(obj), i_type(type), i_range(range)
{
}

bool NearestGameObjectTypeInObjectRangeCheck::operator()(GameObject* go)
{
    if (go->GetGoType() == i_type && i_obj.IsWithinDistInMap(go, i_range))
    {
        i_range = i_obj.GetDistance(go); // use found GO range as new range limit for next check
        return true;
    }
    return false;
}

GameObjectWithDbGUIDCheck::GameObjectWithDbGUIDCheck(ObjectGuid::LowType db_guid) : i_db_guid(db_guid)
{
}

bool GameObjectWithDbGUIDCheck::operator()(GameObject const* go) const
{
    return go->GetDBTableGUIDLow() == i_db_guid;
}

MostHPMissingInRange::MostHPMissingInRange(Unit const* obj, float range, uint32 hp) : i_obj(obj), i_range(range), i_hp(hp)
{
}

bool MostHPMissingInRange::operator()(Unit* u)
{
    if (u->isAlive() && u->isInCombat() && !i_obj->IsHostileTo(u) && i_obj->IsWithinDistInMap(u, i_range) && u->GetMaxHealth() - u->GetHealth() > i_hp)
    {
        i_hp = u->GetMaxHealth() - u->GetHealth();
        return true;
    }
    return false;
}

FriendlyCCedInRange::FriendlyCCedInRange(Unit const* obj, float range) : i_obj(obj), i_range(range)
{
}

bool FriendlyCCedInRange::operator()(Unit* u)
{
    if (u->isAlive() && u->isInCombat() && !i_obj->IsHostileTo(u) && i_obj->IsWithinDistInMap(u, i_range) && (u->isFeared() || u->isCharmed() || u->isFrozen() || u->HasUnitState(UNIT_STATE_STUNNED) || u->HasUnitState(UNIT_STATE_CONFUSED)))
    {
        return true;
    }
    return false;
}

FriendlyMissingBuffInRange::FriendlyMissingBuffInRange(Unit const* obj, float range, uint32 spellid) : i_obj(obj), i_range(range), i_spell(spellid)
{
}

bool FriendlyMissingBuffInRange::operator()(Unit* u)
{
    if (u->isAlive() && u->isInCombat() && !i_obj->IsHostileTo(u) && i_obj->IsWithinDistInMap(u, i_range) && !(u->HasAura(i_spell)))
    {
        return true;
    }
    return false;
}

AnyUnfriendlyUnitInObjectRangeCheck::AnyUnfriendlyUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range) : i_obj(obj), i_funit(funit), i_range(range)
{
}

bool AnyUnfriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->isAlive() && i_obj->IsWithinDistInMap(u, i_range) && !i_funit->IsFriendlyTo(u))
        return true;
    return false;
}

AnyUnfriendlyNoTotemUnitInObjectRangeCheck::AnyUnfriendlyNoTotemUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range) : i_obj(obj), i_funit(funit), i_range(range)
{
}

bool AnyUnfriendlyNoTotemUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (!u->isAlive())
        return false;

    if (u->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET)
        return false;

    if (u->IsCreature() && static_cast<Creature*>(u)->isTotem())
        return false;

    if (!u->isTargetableForAttack(false))
        return false;

    return i_obj->IsWithinDistInMap(u, i_range) && !i_funit->IsFriendlyTo(u);
}

AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck::AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck(Unit const* funit, float range, bool checkin) : i_funit(funit), i_range(range), i_checkin(checkin)
{
}

bool AnyUnfriendlyAttackableVisibleUnitInObjectRangeCheck::operator()(const Unit* u)
{
    return (u->isAlive() && i_funit->IsWithinDistInMap(u, i_range) && !i_funit->IsFriendlyTo(u) && i_funit->IsValidAttackTarget(u) && u->GetCreatureType() != CREATURE_TYPE_CRITTER && i_funit->canSeeOrDetect(u)) == i_checkin;
}

CreatureWithDbGUIDCheck::CreatureWithDbGUIDCheck(ObjectGuid::LowType const& lowguid) : i_lowguid(lowguid)
{
}

bool CreatureWithDbGUIDCheck::operator()(Creature* u)
{
    return u->GetDBTableGUIDLow() == i_lowguid;
}

AnyFriendlyUnitInObjectRangeCheck::AnyFriendlyUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range) : i_obj(obj), i_funit(funit), i_range(range)
{
}

bool AnyFriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->isAlive() && i_obj->IsWithinDistInMap(u, i_range) && i_funit->IsFriendlyTo(u))
        return true;
    return false;
}

AnyUnitHavingBuffInObjectRangeCheck::AnyUnitHavingBuffInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range, uint32 spellid, bool isfriendly) : i_obj(obj), i_funit(funit), i_range(range), i_spellid(spellid), i_friendly(isfriendly)
{
}

bool AnyUnitHavingBuffInObjectRangeCheck::operator()(Unit* u)
{
    if (u->isAlive() && i_obj->IsWithinDistInMap(u, i_range) && i_funit->IsFriendlyTo(u) == i_friendly && u->HasAura(
        i_spellid, i_obj->GetGUID()))
        return true;
    return false;
}

AnyGroupedUnitInObjectRangeCheck::AnyGroupedUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range, bool raid) : _source(obj), _refUnit(funit), _range(range), _raid(raid)
{
}

bool AnyGroupedUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (_raid)
    {
        if (!_refUnit->IsInRaidWith(u))
            return false;
    }
    else if (!_refUnit->IsInPartyWith(u))
        return false;

    return !_refUnit->IsHostileTo(u) && u->isAlive() && _source->IsWithinDistInMap(u, _range);
}

AnyUnitInObjectRangeCheck::AnyUnitInObjectRangeCheck(WorldObject const* obj, float range, bool aliveOnly) : i_obj(obj), i_range(range), i_aliveOnly(aliveOnly)
{
}

bool AnyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->IsPlayer() && u->ToPlayer()->isGameMaster())
        return false;

    if ((!i_aliveOnly || u->isAlive()) && i_obj->IsWithinDistInMap(u, i_range) && !u->isTotem())
        return true;

    return false;
}

AreaTriggerWithEntryInObjectRangeCheck::AreaTriggerWithEntryInObjectRangeCheck(WorldObject const* obj, uint32 entry, ObjectGuid casterGuid, float range) : i_obj(obj), i_range(range), i_entry(entry), i_casterGuid(casterGuid)
{
}

bool AreaTriggerWithEntryInObjectRangeCheck::operator()(AreaTrigger* at)
{
    if (i_obj->IsWithinDistInMap(at, i_range) && i_entry == at->GetEntry() && (i_casterGuid.IsEmpty() || i_casterGuid == at->GetCasterGUID()))
        return true;

    return false;
}

NearestAttackableUnitInObjectRangeCheck::NearestAttackableUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range) : i_obj(obj), i_funit(funit), i_range(range)
{
}

bool NearestAttackableUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->isTargetableForAttack() && i_obj->IsWithinDistInMap(u, i_range) && !i_funit->IsFriendlyTo(u)/* && i_funit->canSeeOrDetect(u)*/)
    {
        i_range = i_obj->GetDistance(u); // use found unit range as new range limit for next check
        return true;
    }

    return false;
}

NearestAttackableNoCCUnitInObjectRangeCheck::NearestAttackableNoCCUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range) : i_obj(obj), i_funit(funit), i_range(range)
{
}

bool NearestAttackableNoCCUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->isTargetableForAttack() && i_obj->IsWithinDistInMap(u, i_range) && !i_funit->IsFriendlyTo(u) && !u->HasCrowdControlAura() && !u->HasAuraType(SPELL_AURA_MOD_CONFUSE))
    {
        i_range = i_obj->GetDistance(u); // use found unit range as new range limit for next check
        return true;
    }

    return false;
}

AnyAoETargetUnitInObjectRangeCheck::AnyAoETargetUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range) : i_obj(obj), i_funit(funit), _spellInfo(nullptr), i_range(range)
{
    Unit const* check = i_funit;
    Unit const* owner = i_funit->GetOwner();
    if (owner)
        check = owner;
    i_targetForPlayer = (check->IsPlayer());
    if (i_obj->IsDynObject())
        _spellInfo = sSpellMgr->GetSpellInfo(((DynamicObject*)i_obj)->GetSpellId());
}

bool AnyAoETargetUnitInObjectRangeCheck::operator()(Unit* u)
{
    // Check contains checks for: live, non-selectable, non-attackable flags, flight check and GM check, ignore totems
    if (u->IsCreature() && static_cast<Creature*>(u)->isTotem())
        return false;

    if (i_funit->_IsValidAttackTarget(u, _spellInfo, i_obj->IsDynObject() ? i_obj : nullptr) && i_obj->
        IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

CallOfHelpCreatureInRangeDo::CallOfHelpCreatureInRangeDo(Unit* funit, Unit* enemy, float range) : i_funit(funit), i_enemy(enemy), i_range(range)
{
}

void CallOfHelpCreatureInRangeDo::operator()(Creature* u)
{
    if (u == i_funit)
        return;

    if (!u->CanAssistTo(i_funit, i_enemy, false))
        return;

    // too far
    if (!u->IsWithinDistInMap(i_funit, i_range))
        return;

    // only if see assisted creature's enemy
    if (!u->IsWithinLOSInMap(i_enemy))
        return;

    if (u->AI())
        u->AI()->AttackStart(i_enemy);
}

bool AnyDeadUnitCheck::operator()(Unit* u)
{
    return !u->isAlive();
}

NearestHostileUnitCheck::NearestHostileUnitCheck(Unit const* creature, float dist) : me(creature)
{
    m_range = (dist == 0 ? 9999 : dist);
}

bool NearestHostileUnitCheck::operator()(Unit* u)
{
    if (!me->IsWithinDistInMap(u, m_range))
        return false;

    if (!me->IsValidAttackTarget(u))
        return false;

    m_range = me->GetDistance(u); // use found unit range as new range limit for next check
    return true;
}

NearestHostileNoCCUnitCheck::NearestHostileNoCCUnitCheck(Creature const* creature, float dist) : me(creature)
{
    m_range = (dist == 0 ? 9999 : dist);
}

bool NearestHostileNoCCUnitCheck::operator()(Unit* u)
{
    if (!me->IsWithinDistInMap(u, m_range))
        return false;

    if (!me->IsValidAttackTarget(u))
        return false;

    if (u->HasCrowdControlAura())
        return false;

    if (u->HasAuraType(SPELL_AURA_MOD_CONFUSE))
        return false;

    m_range = me->GetDistance(u); // use found unit range as new range limit for next check
    return true;
}

NearestHostileUnitInAttackDistanceCheck::NearestHostileUnitInAttackDistanceCheck(Creature const* creature, float dist) : me(creature)
{
    m_range = (dist == 0 ? 9999 : dist);
    m_force = (dist == 0 ? false : true);
}

bool NearestHostileUnitInAttackDistanceCheck::operator()(Unit* u)
{
    if (!me->IsWithinDistInMap(u, m_range))
        return false;

    if (!me->canSeeOrDetect(u))
        return false;

    if (m_force)
    {
        if (!me->IsValidAttackTarget(u))
            return false;
    }
    else if (!me->canStartAttack(u, false))
        return false;

    m_range = me->GetDistance(u); // use found unit range as new range limit for next check
    return true;
}

NearestHostileUnitInAggroRangeCheck::NearestHostileUnitInAggroRangeCheck(Creature const* creature, bool useLOS) : _me(creature), _useLOS(useLOS)
{
}

bool NearestHostileUnitInAggroRangeCheck::operator()(Unit* u)
{
    if (!u->IsHostileTo(_me))
        return false;

    if (!u->IsWithinDistInMap(_me, _me->GetAttackDistance(u)))
        return false;

    if (!_me->IsValidAttackTarget(u))
        return false;

    if (_useLOS && !u->IsWithinLOSInMap(_me))
        return false;

    return true;
}

AnyAssistCreatureInRangeCheck::
AnyAssistCreatureInRangeCheck(Unit* funit, Unit* enemy, float range) : i_funit(funit), i_enemy(enemy), i_range(range)
{
}

bool AnyAssistCreatureInRangeCheck::operator()(Creature* u)
{
    if (u == i_funit)
        return false;

    if (u->IsAlreadyCallAssistance())
        return false;

    if (!u->CanAssistTo(i_funit, i_enemy))
        return false;

    // too far
    if (!i_funit->IsWithinDistInMap(u, i_range))
        return false;

    // only if see assisted creature
    if (!i_funit->IsWithinLOSInMap(u))
        return false;

    return true;
}

NearestAssistCreatureInCreatureRangeCheck::NearestAssistCreatureInCreatureRangeCheck(Creature* obj, Unit* enemy, float range) : i_obj(obj), i_enemy(enemy), i_range(range)
{
}

bool NearestAssistCreatureInCreatureRangeCheck::operator()(Creature* u)
{
    if (u == i_obj)
        return false;
    if (!u->CanAssistTo(i_obj, i_enemy))
        return false;

    if (!i_obj->IsWithinDistInMap(u, i_range))
        return false;

    if (!i_obj->IsWithinLOSInMap(u))
        return false;

    i_range = i_obj->GetDistance(u); // use found unit range as new range limit for next check
    return true;
}

NearestCreatureEntryWithLiveStateInObjectRangeCheck::NearestCreatureEntryWithLiveStateInObjectRangeCheck(WorldObject const& obj, uint32 entry, bool alive, float range) : i_obj(obj), i_entry(entry), i_alive(alive), i_range(range)
{
}

bool NearestCreatureEntryWithLiveStateInObjectRangeCheck::operator()(Creature* u)
{
    if (u->GetEntry() == i_entry && u->isAlive() == i_alive && i_obj.IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj.GetDistance(u); // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

AnyPlayerInObjectRangeCheck::
AnyPlayerInObjectRangeCheck(WorldObject const* obj, float range, bool reqAlive) : _obj(obj), _range(range), _reqAlive(reqAlive)
{
}

bool AnyPlayerInObjectRangeCheck::operator()(Player* u)
{
    if (_reqAlive && !u->isAlive())
        return false;

    if (!_obj->IsWithinDistInMap(u, _range))
        return false;

    return true;
}

NearestPlayerInObjectRangeCheck::NearestPlayerInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range)
{
}

bool NearestPlayerInObjectRangeCheck::operator()(Player* u)
{
    if (u->isAlive() && i_obj->IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj->GetDistance(u);
        return true;
    }

    return false;
}

NearestPlayerNotGMInObjectRangeCheck::NearestPlayerNotGMInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range)
{
}

bool NearestPlayerNotGMInObjectRangeCheck::operator()(Player* u)
{
    if (!u->isGameMaster() && u->isAlive() && i_obj->IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj->GetDistance(u);
        return true;
    }

    return false;
}

AllFriendlyCreaturesInGrid::AllFriendlyCreaturesInGrid(Unit const* obj) : unit(obj)
{
}

bool AllFriendlyCreaturesInGrid::operator()(Unit* u)
{
    if (u->isAlive() && u->IsVisible() && u->IsFriendlyTo(unit))
        return true;

    return false;
}

AllGameObjectsWithEntryInRange::AllGameObjectsWithEntryInRange(const WorldObject* object, uint32 entry, float maxRange) : m_pObject(object), m_uiEntry(entry), m_fRange(maxRange)
{
}

bool AllGameObjectsWithEntryInRange::operator()(GameObject* go)
{
    if (go->GetEntry() == m_uiEntry && m_pObject->IsWithinDist(go, m_fRange, false))
        return true;

    return false;
}

AllCreaturesOfEntryInRange::AllCreaturesOfEntryInRange(const WorldObject* object, uint32 entry, float maxRange) : m_pObject(object), m_uiEntry(entry), m_fRange(maxRange)
{
}

bool AllCreaturesOfEntryInRange::operator()(Unit* unit)
{
    if (unit->GetEntry() == m_uiEntry && m_pObject->IsWithinDist(unit, m_fRange, false))
        return true;

    return false;
}

AllCreaturesInRange::AllCreaturesInRange(const WorldObject* object, float maxRange) : m_pObject(object), m_fRange(maxRange)
{
}

bool AllCreaturesInRange::operator()(Unit* unit)
{
    if (m_pObject->IsWithinDist(unit, m_fRange, false))
        return true;

    return false;
}

AllAreaTriggeresOfEntryInRange::AllAreaTriggeresOfEntryInRange(const WorldObject* object, uint32 entry, float maxRange) : m_pObject(object), m_uiEntry(entry), m_fRange(maxRange)
{
}

bool AllAreaTriggeresOfEntryInRange::operator()(AreaTrigger* at)
{
    if (at->GetEntry() == m_uiEntry && m_pObject->IsWithinDist(at, m_fRange, false))
        return true;

    return false;
}

AllAliveCreaturesOfEntryInRange::AllAliveCreaturesOfEntryInRange(const WorldObject* object, uint32 entry, float maxRange) : m_pObject(object), m_uiEntry(entry), m_fRange(maxRange)
{
}

bool AllAliveCreaturesOfEntryInRange::operator()(Unit* unit)
{
    if (unit->GetEntry() == m_uiEntry && m_pObject->IsWithinDist(unit, m_fRange, false) && unit->isAlive())
        return true;

    return false;
}

SearchCorpseCreatureCheck::SearchCorpseCreatureCheck(const WorldObject* object, float range) : m_owner(nullptr), m_pObject(object), i_range(range)
{
    m_owner = const_cast<Player*>(m_pObject->ToPlayer());
    if (!m_owner)
    {
        if (const Creature* c = m_pObject->ToCreature())
            if (const Unit* owner = c->GetOwner())
                m_owner = const_cast<Player*>(owner->ToPlayer());
    }
}

bool SearchCorpseCreatureCheck::operator()(Creature* u)
{
    if (!m_owner)
        return false;

    if (u->getDeathState() != CORPSE || !m_owner->isAllowedToLoot(u) || !u->HasFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
        return false;

    return m_pObject->IsWithinDistInMap(u, i_range);
}

AllDeadCreaturesInRange::AllDeadCreaturesInRange(const WorldObject* object, float maxRange, ObjectGuid excludeGUID) : m_pObject(object), m_fRange(maxRange), m_excludeGUID(excludeGUID)
{
}

bool AllDeadCreaturesInRange::operator()(Unit const* unit) const
{
    return unit->IsCreature() && unit->GetGUID() != m_excludeGUID && !unit->isAlive() && m_pObject->IsWithinDist(unit, m_fRange, false);
}

PlayerAtMinimumRangeAway::PlayerAtMinimumRangeAway(Unit const* unit, float fMinRange) : unit(unit), fRange(fMinRange)
{
}

bool PlayerAtMinimumRangeAway::operator()(Player* player)
{
    //No threat list check, must be done explicit if expected to be in combat with creature
    if (!player->isGameMaster() && player->isAlive() && !unit->IsWithinDist(player, fRange, false))
        return true;

    return false;
}

GameObjectInRangeCheck::GameObjectInRangeCheck(float _x, float _y, float _z, float _range, uint32 _entry) : x(_x), y(_y), z(_z), range(_range), entry(_entry)
{
}

bool GameObjectInRangeCheck::operator()(GameObject* go)
{
    if (!entry || (go->GetGOInfo() && go->GetGOInfo()->entry == entry))
        return go->IsInRange(x, y, z, range);
    return false;
}

AllWorldObjectsInRange::AllWorldObjectsInRange(const WorldObject* object, float maxRange) : m_pObject(object), m_fRange(maxRange)
{
}

bool AllWorldObjectsInRange::operator()(WorldObject* go)
{
    return m_pObject->IsWithinDist(go, m_fRange, false) && m_pObject->InSamePhase(go);
}

ObjectTypeIdCheck::ObjectTypeIdCheck(TypeID typeId, bool equals) : _typeId(typeId), _equals(equals)
{
}

bool ObjectTypeIdCheck::operator()(WorldObject* object)
{
    return (object->GetTypeId() == _typeId) == _equals;
}

ObjectGUIDCheck::ObjectGUIDCheck(ObjectGuid GUID) : _GUID(GUID)
{
}

bool ObjectGUIDCheck::operator()(WorldObject* object)
{
    return object->GetGUID() == _GUID;
}

UnitAuraCheck::UnitAuraCheck(bool present, uint32 spellId, ObjectGuid casterGUID) : _present(present), _spellId(spellId), _casterGUID(casterGUID)
{
}

bool UnitAuraCheck::operator()(Unit* unit) const
{
    return unit->HasAura(_spellId, _casterGUID) == _present;
}

bool UnitAuraCheck::operator()(WorldObject* object) const
{
    return object->IsUnit() && object->ToUnit()->HasAura(_spellId, _casterGUID) == _present;
}

UnitAuraTypeCheck::UnitAuraTypeCheck(bool present, AuraType type) : _present(present), _type(type)
{
}

bool UnitAuraTypeCheck::operator()(Unit* unit) const
{
    return unit->HasAuraType(_type) == _present;
}

bool UnitAuraTypeCheck::operator()(WorldObject* object) const
{
    return object->IsUnit() && object->ToUnit()->HasAuraType(_type) == _present;
}

MailBoxMasterCheck::MailBoxMasterCheck(Player* plr) : _plr(plr)
{
}

bool MailBoxMasterCheck::operator()(GameObject* u)
{
    if (!_plr->IsInWorld())
        return false;

    if (_plr->isInFlight())
        return false;

    if (!u->IsWithinDistInMap(_plr, INTERACTION_DISTANCE))
        return false;

    return true;
}

SummonTimerOrderPred::SummonTimerOrderPred(bool ascending) : m_ascending(ascending)
{
}

bool SummonTimerOrderPred::operator()(const Unit* a, const Unit* b) const
{
    if (!a->isSummon() || !b->isSummon())
        return (urand(0, 1) ? false : true);

    uint32 rA = ((TempSummon*)a)->GetTimer();
    uint32 rB = ((TempSummon*)b)->GetTimer();
    return m_ascending ? rA < rB : rA > rB;
}

UnitHealthState::UnitHealthState(bool sortlow) : _sortlow(sortlow)
{
}

bool UnitHealthState::operator()(Unit* unitA, Unit* unitB) const
{
    return (unitA->GetHealthPct() < unitB->GetHealthPct()) == _sortlow;
}

bool UnitHealthState::operator()(WorldObject* objectA, WorldObject* objectB) const
{
    return objectA->IsUnit() && objectB->IsUnit() && (objectA->ToUnit()->GetHealthPct() < objectB->ToUnit()->GetHealthPct()) == _sortlow;
}

UnitDistanceCheck::UnitDistanceCheck(bool checkin, Unit* caster, float dist) : _checkin(checkin), _caster(caster), _dist(dist)
{
}

bool UnitDistanceCheck::operator()(Unit* unit) const
{
    return (_caster->GetExactDist2d(unit) > _dist) == _checkin;
}

bool UnitDistanceCheck::operator()(WorldObject* object) const
{
    return (_caster->GetExactDist2d(object) > _dist) == _checkin;
}

DestDistanceCheck::DestDistanceCheck(bool checkin, Position* dest, float dist) : _checkin(checkin), _dest(dest), _dist(dist)
{
}

bool DestDistanceCheck::operator()(Unit* unit) const
{
    return (_dest->GetExactDist2d(unit) > _dist) == _checkin;
}

bool DestDistanceCheck::operator()(WorldObject* object) const
{
    return (_dest->GetExactDist2d(object) > _dist) == _checkin;
}

UnitTypeCheck::UnitTypeCheck(bool checkin, uint32 typeMask) : _checkin(checkin), _typeMask(typeMask)
{
}

bool UnitTypeCheck::operator()(Unit* unit) const
{
    return bool(_typeMask & (1 << unit->GetTypeId())) == _checkin;
}

bool UnitTypeCheck::operator()(WorldObject* object) const
{
    return bool(_typeMask & (1 << object->GetTypeId())) == _checkin;
}

UnitSortDistance::UnitSortDistance(bool sortlow, WorldObject* caster) : _sortlow(sortlow), _caster(caster)
{
}

bool UnitSortDistance::operator()(Unit* unitA, Unit* unitB) const
{
    return (_caster->GetExactDist2d(unitA) < _caster->GetExactDist2d(unitB)) == _sortlow;
}

bool UnitSortDistance::operator()(WorldObject* objectA, WorldObject* objectB) const
{
    return (_caster->GetExactDist2d(objectA) < _caster->GetExactDist2d(objectB)) == _sortlow;
}

UnitFriendlyCheck::UnitFriendlyCheck(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitFriendlyCheck::operator()(Unit* unit)
{
    return unit->IsFriendlyTo(_caster) == _present;
}

bool UnitFriendlyCheck::operator()(WorldObject* object)
{
    return object->IsUnit() && object->ToUnit()->IsFriendlyTo(_caster) == _present;
}

UnitRaidCheck::UnitRaidCheck(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitRaidCheck::operator()(Unit* unit)
{
    return unit->IsInRaidWith(_caster) == _present;
}

bool UnitRaidCheck::operator()(WorldObject* object)
{
    return object->IsUnit() && object->ToUnit()->IsInRaidWith(_caster) == _present;
}

UnitPartyCheck::UnitPartyCheck(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitPartyCheck::operator()(Unit* unit)
{
    return unit->IsInPartyWith(_caster) == _present;
}

bool UnitPartyCheck::operator()(WorldObject* object)
{
    return object->IsUnit() && object->ToUnit()->IsInPartyWith(_caster) == _present;
}

UnitCheckInLos::UnitCheckInLos(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitCheckInLos::operator()(WorldObject* object)
{
    return object->IsWithinLOSInMap(_caster) == _present;
}

UnitCheckInBetween::UnitCheckInBetween(bool present, Unit* caster, Unit* target, float size) : _present(present), _caster(caster), _target(target), _size(size)
{
}

bool UnitCheckInBetween::operator()(WorldObject* object)
{
    return object->IsInBetween(_caster, _target, _size) == _present;
}

UnitCheckInBetweenShift::UnitCheckInBetweenShift(bool present, Unit* caster, Unit* target, float size, float shift, float angleShift) : _present(present), _caster(caster), _target(target), _size(size), _shift(shift), _angleShift(angleShift)
{
}

bool UnitCheckInBetweenShift::operator()(WorldObject* object)
{
    return object->IsInBetweenShift(_caster, _target, _size, _shift, _angleShift) == _present;
}

UnitCheckCCAura::UnitCheckCCAura(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitCheckCCAura::operator()(WorldObject* object)
{
    return object->IsUnit() && object->ToUnit()->HasCrowdControlAura(_caster) == _present;
}

UnitAuraAndCheck::UnitAuraAndCheck(int32 aura1, int32 aura2, int32 aura3, ObjectGuid casterGUID) : _aura1(aura1), _aura2(aura2), _aura3(aura3), _casterGUID(casterGUID)
{
}

bool UnitAuraAndCheck::operator()(Unit* unit) const
{
    if (_aura1 < 0 && unit->HasAura(abs(_aura1), _casterGUID))
        return true;
    if (_aura1 > 0 && !unit->HasAura(_aura1, _casterGUID))
        return true;

    if (_aura2 < 0 && unit->HasAura(abs(_aura2), _casterGUID))
        return true;
    if (_aura2 > 0 && !unit->HasAura(_aura2, _casterGUID))
        return true;

    if (_aura3 < 0 && unit->HasAura(abs(_aura3), _casterGUID))
        return true;
    if (_aura3 > 0 && !unit->HasAura(_aura3, _casterGUID))
        return true;

    return false;
}

bool UnitAuraAndCheck::operator()(WorldObject* object) const
{
    if (Unit* unit = object->ToUnit())
    {
        if (_aura1 < 0 && unit->HasAura(abs(_aura1), _casterGUID))
            return true;
        if (_aura1 > 0 && !unit->HasAura(_aura1, _casterGUID))
            return true;

        if (_aura2 < 0 && unit->HasAura(abs(_aura2), _casterGUID))
            return true;
        if (_aura2 > 0 && !unit->HasAura(_aura2, _casterGUID))
            return true;

        if (_aura3 < 0 && unit->HasAura(abs(_aura3), _casterGUID))
            return true;
        if (_aura3 > 0 && !unit->HasAura(_aura3, _casterGUID))
            return true;

        return false;
    }
    return true;
}

UnitAuraOrCheck::UnitAuraOrCheck(int32 aura1, int32 aura2, int32 aura3, ObjectGuid casterGUID) : _aura1(aura1), _aura2(aura2), _aura3(aura3), _casterGUID(casterGUID)
{
}

bool UnitAuraOrCheck::operator()(Unit* unit) const
{
    if (_aura1 < 0 && !unit->HasAura(abs(_aura1), _casterGUID))
        return false;
    if (_aura1 > 0 && unit->HasAura(_aura1, _casterGUID))
        return false;

    if (_aura2 < 0 && !unit->HasAura(abs(_aura2), _casterGUID))
        return false;
    if (_aura2 > 0 && unit->HasAura(_aura2, _casterGUID))
        return false;

    if (_aura3 < 0 && !unit->HasAura(abs(_aura3), _casterGUID))
        return false;
    if (_aura3 > 0 && unit->HasAura(_aura3, _casterGUID))
        return false;

    return true;
}

bool UnitAuraOrCheck::operator()(WorldObject* object) const
{
    if (Unit* unit = object->ToUnit())
    {
        if (_aura1 < 0 && !unit->HasAura(abs(_aura1), _casterGUID))
            return false;
        if (_aura1 > 0 && unit->HasAura(_aura1, _casterGUID))
            return false;

        if (_aura2 < 0 && !unit->HasAura(abs(_aura2), _casterGUID))
            return false;
        if (_aura2 > 0 && unit->HasAura(_aura2, _casterGUID))
            return false;

        if (_aura3 < 0 && !unit->HasAura(abs(_aura3), _casterGUID))
            return false;
        if (_aura3 > 0 && unit->HasAura(_aura3, _casterGUID))
            return false;

        return true;
    }
    return true;
}

UnitEntryCheck::UnitEntryCheck(int32 entry1, int32 entry2, int32 entry3) : _entry1(entry1), _entry2(entry2), _entry3(entry3)
{
}

bool UnitEntryCheck::operator()(WorldObject* object) const
{
    if (_entry1 > 0)
    {
        if (_entry1 > 0 && object->GetEntry() == _entry1)
            return false;
        if (_entry2 > 0 && object->GetEntry() == _entry2)
            return false;
        if (_entry3 > 0 && object->GetEntry() == _entry3)
            return false;

        return true;
    }
    if (_entry1 < 0)
    {
        if (_entry1 < 0 && object->GetEntry() == abs(_entry1))
            return true;
        if (_entry2 < 0 && object->GetEntry() == abs(_entry2))
            return true;
        if (_entry3 < 0 && object->GetEntry() == abs(_entry3))
            return true;

        return false;
    }

    return true;
}

UnitAttackableCheck::UnitAttackableCheck(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitAttackableCheck::operator()(Unit* unit)
{
    return _caster->IsValidAttackTarget(unit) == _present;
}

bool UnitAttackableCheck::operator()(WorldObject* object)
{
    return object->IsUnit() && _caster->IsValidAttackTarget(object->ToUnit()) == _present;
}

EntryCheckPredicate::EntryCheckPredicate(uint32 entry) : _entry(entry)
{
}

bool EntryCheckPredicate::operator()(ObjectGuid guid)
{
    return guid.GetEntry() == _entry;
}

UnitFullHPCheck::UnitFullHPCheck(bool full) : _full(full)
{
}

bool UnitFullHPCheck::operator()(Unit* unit)
{
    return unit->IsFullHealth() == _full;
}

bool UnitFullHPCheck::operator()(WorldObject* object)
{
    return object->IsUnit() && object->ToUnit()->IsFullHealth() == _full;
}

UnitOwnerCheck::UnitOwnerCheck(bool present, Unit* caster) : _present(present), _caster(caster)
{
}

bool UnitOwnerCheck::operator()(Unit* unit)
{
    return unit->IsOwnerOrSelf(_caster) == _present;
}

bool UnitOwnerCheck::operator()(WorldObject* object)
{
    return object->ToUnit() && object->ToUnit()->IsOwnerOrSelf(_caster) == _present;
}

