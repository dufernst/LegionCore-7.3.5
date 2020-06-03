/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_CREATUREAI_H
#define TRINITY_CREATUREAI_H

#include "Creature.h"
#include "UnitAI.h"
#include "Common.h"

class WorldObject;
class Unit;
class Creature;
class Player;
class SpellInfo;

#define TIME_INTERVAL_LOOK   5000

//Spell targets used by SelectSpell
enum SelectTargetType
{
    SELECT_TARGET_DONTCARE = 0,                             //All target types allowed

    SELECT_TARGET_SELF,                                     //Only Self casting

    SELECT_TARGET_SINGLE_ENEMY,                             //Only Single Enemy
    SELECT_TARGET_AOE_ENEMY,                                //Only AoE Enemy
    SELECT_TARGET_ANY_ENEMY,                                //AoE or Single Enemy

    SELECT_TARGET_SINGLE_FRIEND,                            //Only Single Friend
    SELECT_TARGET_AOE_FRIEND,                               //Only AoE Friend
    SELECT_TARGET_ANY_FRIEND,                               //AoE or Single Friend
};

//Spell Effects used by SelectSpell
enum SelectEffect
{
    SELECT_EFFECT_DONTCARE = 0,                             //All spell effects allowed
    SELECT_EFFECT_DAMAGE,                                   //Spell does damage
    SELECT_EFFECT_HEALING,                                  //Spell does healing
    SELECT_EFFECT_AURA,                                     //Spell applies an aura
};

enum SCEquip
{
    EQUIP_NO_CHANGE = -1,
    EQUIP_UNEQUIP   = 0
};

enum CreatureSummonGroup
{
    CREATURE_SUMMON_GROUP_RESET      = 0,
    CREATURE_SUMMON_GROUP_COMBAT     = 1,
};

class CreatureAI : public UnitAI
{
protected:
    Creature* const me;

    bool UpdateVictim();
    bool UpdateVictimWithGaze();

    void SetGazeOn(Unit* target);

    Creature* DoSummon(uint32 entry, Position const& pos, uint32 despawnTime = 30000, TempSummonType summonType = TEMPSUMMON_CORPSE_TIMED_DESPAWN);
    Creature* DoSummon(uint32 entry, WorldObject* obj, float radius = 5.0f, uint32 despawnTime = 30000, TempSummonType summonType = TEMPSUMMON_CORPSE_TIMED_DESPAWN);
    Creature* DoSummonFlyer(uint32 entry, WorldObject* obj, float flightZ, float radius = 5.0f, uint32 despawnTime = 30000, TempSummonType summonType = TEMPSUMMON_CORPSE_TIMED_DESPAWN);

public:
    explicit CreatureAI(Creature* creature);
    virtual ~CreatureAI() = default;

    void DoAggroPulse(uint32 diff);
    bool IsInDisable();
    bool IsInControl();
    void Talk(std::initializer_list<uint8> ids, ObjectGuid WhisperGuid = ObjectGuid::Empty);
    void Talk(uint8 id, ObjectGuid WhisperGuid = ObjectGuid::Empty);
    void TalkAuto(uint8 id, ObjectGuid WhisperGuid = ObjectGuid::Empty);
    void DelayTalk(uint32 delayTimer, uint8 id, ObjectGuid WhisperGuid = ObjectGuid::Empty);
    void ZoneTalk(uint8 id, ObjectGuid WhisperGuid = ObjectGuid::Empty);

    void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function);
    void KillAllDelayedEvents();
    void AddDelayedCombat(uint64 timeOffset, std::function<void()>&& function);
    void KillAllDelayedCombats();

    // Called if IsVisible(Unit* who) is true at each who move, reaction at visibility zone enter
    void MoveInLineOfSight_Safe(Unit* who);

    bool CanSeeEvenInPassiveMode();

    void SetCanSeeEvenInPassiveMode(bool canSeeEvenInPassiveMode);

    // Called in Creature::Update when deathstate = DEAD. Inherited classes may maniuplate the ability to respawn based on scripted events.
    virtual bool CanRespawn() { return true; }

    // Called for reaction at stopping attack at no attackers or targets
    virtual void EnterEvadeMode();

    // Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
    virtual void EnterCombat(Unit* /*victim*/) {}

    // Called when the creature is killed
    virtual void JustDied(Unit* /*killer*/) {}

    // Called when the creature kills a unit
    virtual void KilledUnit(Unit* /*victim*/) {}

    // Called when the creature summon successfully other creature
    virtual void JustSummoned(Creature* /*summon*/) {}
    virtual void IsSummonedBy(Unit* /*summoner*/) {}

    virtual void SummonedCreatureDespawn(Creature* /*summon*/) {}
    virtual void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/) {}

    virtual void DespawnOnRespawn(uint32 uiTimeToDespawn);

    // Called when the creature summon successfully gameobject
    virtual void JustSummonedGO(GameObject* /*summoner*/) {}

    // Called when hit by a spell
    virtual void SpellHit(Unit* /*caster*/, SpellInfo const* /*spell*/) {}

    // Called when spell hits a target
    virtual void SpellHitTarget(Unit* /*target*/, SpellInfo const* /*spell*/) {}

    /// Called when spell miss a target
    virtual void SpellMissTarget(Unit* /*target*/, SpellInfo const* /*spellInfo*/, SpellMissInfo /*missInfo*/) {}

    // Called when on finish cast spell
    virtual void SpellFinishCast(SpellInfo const* /*spell*/) {}

    // Called when the creature is target of hostile action: swing, hostile spell landed, fear/etc). It's can be BEFORE EnterCombat
    virtual void AttackedBy(Unit* attacker) {}
    virtual bool IsEscorted() { return false; }

    // Called when creature is spawned or respawned (for reseting variables)
    virtual void JustRespawned() { Reset(); }

    // Called at waypoint reached or point movement finished
    virtual void MovementInform(uint32 /*type*/, uint32 /*id*/) {}
    virtual void LastWPReached() {}

    virtual void OnCharmed(bool apply);

    void SetFlyMode(bool /*fly*/);

    // Called at reaching home after evade
    virtual void JustReachedHome() {}

    void DoZoneInCombat(Creature* creature = nullptr, float maxRangeToNearestTarget = 50.0f);
    void DoAttackerAreaInCombat(Unit* attacker, float range, Unit* pUnit = nullptr);
    void DoAttackerGroupInCombat(Player* attacker);

    // Called at text emote receive from player
    virtual void ReceiveEmote(Player* /*player*/, uint32 /*emoteId*/) {}

    // Called when owner takes damage
    virtual void OwnerDamagedBy(Unit* /*attacker*/) {}

    // Called when owner attacks something
    virtual void OwnerAttacked(Unit* /*target*/) {}

    // Called when a creature regen one of his power
    virtual void RegeneratePower(Powers power, float& value) {}
    virtual void SetPower(Powers power, int32 value) {}

    virtual void RecalcStats() {}

    virtual void ComonOnHome() {}

    // Called at any threat added from any attacker (before threat apply)
    virtual void OnAddThreat(Unit* /*victim*/, float& /*fThreat*/, SpellSchoolMask /*schoolMask*/, SpellInfo const* /*threatSpell*/) {}

    virtual void CalcExitVehiclePos(Position & pos) {}

    // called when the corpse of this creature gets removed
    virtual void CorpseRemoved(uint32& /*respawnDelay*/) {}

    virtual void PassengerBoarded(Unit* /*passenger*/, int8 /*seatId*/, bool /*apply*/) {}

    virtual void OnSpellClick(Unit* /*clicker*/) { }

    virtual bool CanSeeAlways(WorldObject const* /*obj*/) { return false; }

    virtual void OnQuestReward(Player* /*player*/, Quest const* /*quest*/) {}
    virtual void OnStartQuest(Player* /*player*/, Quest const* /*quest*/) {}

    virtual void OnApplyOrRemoveAura(uint32 /*spellId*/, AuraRemoveMode /*mode*/, bool /*apply*/) {}
    virtual void OnRemoveAuraTarget(Unit* /*target*/, uint32 /*spellId*/, AuraRemoveMode /*mode*/) {}
    virtual void OnInterruptCast(Unit* /*caster*/, uint32 /*spellId*/, uint32 /*curSpellID*/, uint32 /*schoolMask*/) {}
    virtual void OnAreaTriggerCast(Unit* /*caster*/, Unit* /*target*/, uint32 /*spellId*/, uint32 /*createATSpellId*/) {}
    virtual void OnAreaTriggerDespawn(uint32 spellId, Position pos, bool duration) {}

    virtual void AddClientVisibility(ObjectGuid /*guid*/) { }
    virtual void RemoveClientVisibility(ObjectGuid /*guid*/) { }

protected:
    virtual void MoveInLineOfSight(Unit* /*who*/);

    bool _EnterEvadeMode();

private:
    bool m_MoveInLineOfSight_locked;
    bool m_canSeeEvenInPassiveMode;
    uint32 inFightAggroCheck_Timer;
};

enum Permitions
{
    PERMIT_BASE_NO                 = -1,
    PERMIT_BASE_IDLE               = 1,
    PERMIT_BASE_REACTIVE           = 100,
    PERMIT_BASE_PROACTIVE          = 200,
    PERMIT_BASE_FACTION_SPECIFIC   = 400,
    PERMIT_BASE_SPECIAL            = 800
};

#endif
