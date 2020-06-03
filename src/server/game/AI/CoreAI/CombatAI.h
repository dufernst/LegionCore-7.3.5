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

#ifndef TRINITY_COMBATAI_H
#define TRINITY_COMBATAI_H

#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "ConditionMgr.h"
#include "CreatureTextMgr.h"

class Creature;

typedef std::map<ObjectGuid, EventMap> PlayerEventMap;

class AggressorAI : public CreatureAI
{
    public:
        explicit AggressorAI(Creature* c) : CreatureAI(c), m_checkTimer(0), CreatureTexts(nullptr), CreatureCombatTexts(nullptr) {}

        void Reset();
        void InitializeAI();
        void UpdateAI(uint32);
        void EnterCombat(Unit* who);
        void JustDied(Unit* killer);
        static int Permissible(const Creature*);
        void DoActionAI(uint32 diff, CreatureActionType type);
        void MoveInLineOfSight(Unit*);

        void AddClientVisibility(ObjectGuid guid) override;
        void RemoveClientVisibility(ObjectGuid guid) override;

        uint32 m_checkTimer;
    protected:
        EventMap spellCasts;
        EventMap events;
        EventMap textCombatEvents;
        PlayerEventMap textEvents;
        CreatureTextGroup const* CreatureTexts;
        CreatureTextGroup const* CreatureCombatTexts;
};

class AnyPetAI : public CreatureAI
{
    public:
        explicit AnyPetAI(Creature* c) : CreatureAI(c), m_updateAlliesTimer(0) {}

        void InitializeAI();
        void UpdateAI(uint32);
        static int Permissible(const Creature*);
        void UpdateAllies();
        GuidSet m_AllySet;
        uint32 m_updateAlliesTimer;
        void MovementInform(uint32 moveType, uint32 data);
};

typedef std::vector<uint32> SpellVct;

class CombatAI : public CreatureAI
{
    public:
        explicit CombatAI(Creature* c) : CreatureAI(c) {}

        void InitializeAI();
        void Reset();
        void EnterCombat(Unit* who);
        void JustDied(Unit* killer);
        void UpdateAI(uint32 diff);
        static int Permissible(const Creature*);
    protected:
        EventMap events;
        SpellVct spells;
};

class CasterAI : public CombatAI
{
    public:
        explicit CasterAI(Creature* c) : CombatAI(c) { m_attackDist = MELEE_RANGE; }
        void InitializeAI();
        void AttackStart(Unit* victim) { AttackStartCaster(victim, m_attackDist); }
        void UpdateAI(uint32 diff);
        void EnterCombat(Unit* /*who*/);
    private:
        float m_attackDist;
};

struct ArcherAI : public CreatureAI
{
    public:
        explicit ArcherAI(Creature* c);
        void AttackStart(Unit* who);
        void UpdateAI(uint32 diff);

        static int Permissible(const Creature*);
    protected:
        float m_minRange;
};

struct TurretAI : public CreatureAI
{
    public:
        explicit TurretAI(Creature* c);
        bool CanAIAttack(const Unit* who) const;
        void AttackStart(Unit* who);
        void UpdateAI(uint32 diff);

        static int Permissible(const Creature*);
    protected:
        float m_minRange;
};

#define VEHICLE_CONDITION_CHECK_TIME 1000
#define VEHICLE_DISMISS_TIME 5000
struct VehicleAI : public CreatureAI
{
    public:
        explicit VehicleAI(Creature* c);

        void UpdateAI(uint32 diff);
        static int Permissible(const Creature*);
        void Reset();
        void MoveInLineOfSight(Unit*) {}
        void AttackStart(Unit*) {}
        void OnCharmed(bool apply);

    private:
        Vehicle* m_vehicle;
        bool m_IsVehicleInUse;
        void LoadConditions();
        void CheckConditions(const uint32 diff);
        ConditionList conditions;
        uint32 m_ConditionsTimer;
        bool m_DoDismiss;
        uint32 m_DismissTimer;
};

class BattlePetAI : public CreatureAI
{
    public:
        explicit BattlePetAI(Creature* c) : CreatureAI(c) {}

        void InitializeAI();
        void UpdateAI(uint32);
        static int Permissible(const Creature*);
        void MovementInform(uint32 moveType, uint32 data);
};

#endif
