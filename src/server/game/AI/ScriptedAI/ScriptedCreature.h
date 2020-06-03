/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#ifndef SCRIPTEDCREATURE_H_
#define SCRIPTEDCREATURE_H_

#include "Creature.h"
#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "InstanceScript.h"
#include "TaskScheduler.h"

#define CAST_PLR(a)     (dynamic_cast<Player*>(a))
#define CAST_CRE(a)     (dynamic_cast<Creature*>(a))
#define CAST_AI(a, b)   (dynamic_cast<a*>(b))

#define GET_SPELL(a)    (const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(a)))

class InstanceScript;

class SummonList : public GuidList
{
    public:
        explicit SummonList(Creature* creature) : me(creature) {}
        void Summon(Creature* summon);
        void Despawn(Creature* summon) { remove(summon->GetGUID()); }
        void DespawnEntry(uint32 entry);
        void DespawnAll();
        void SetReactState(ReactStates state);
        template <class Predicate> void DoAction(int32 info, Predicate& predicate, uint16 max = 0)
        {
            // We need to use a copy of SummonList here, otherwise original SummonList would be modified
            GuidList listCopy = *this;
            if (!max)
                max = listCopy.size();
            Trinity::Containers::RandomResizeList(listCopy, predicate, max);
            for (iterator i = listCopy.begin(); i != listCopy.end(); )
            {
                Creature* summon = Unit::GetCreature(*me, *i++);
                if (summon && summon->IsAIEnabled)
                    summon->AI()->DoAction(info);
            }
        }

        void DoZoneInCombat(uint32 entry = 0);
        void RemoveNotExisting();
        bool HasEntry(uint32 entry);
        void KillAll();
        Creature* GetCreature(uint32 entry);
        std::recursive_mutex m_lock;

    private:
        Creature* me;
};

class SummonListGO : public GuidList
{
    public:
        explicit SummonListGO(Creature* creature) : me(creature) {}
        void Summon(GameObject* go) {push_back(go->GetGUID()); };
        void Despawn(GameObject* go) { remove(go->GetGUID()); }
        void DespawnEntry(uint32 entry);
        void DespawnAll();

        void RemoveNotExisting();
        bool HasEntry(uint32 entry);
        std::recursive_mutex m_lock_go;

    private:
        Creature* me;
};

class EntryCheckPredicate
{
    public:
        EntryCheckPredicate(uint32 entry) : _entry(entry) {}
        bool operator()(ObjectGuid guid) { return guid.GetEntry() == _entry; }

    private:
        uint32 _entry;
};

class DummyEntryCheckPredicate
{
    public:
        bool operator()(ObjectGuid) { return true; }
};

struct ScriptedAI : public CreatureAI
{
    explicit ScriptedAI(Creature* creature);
    virtual ~ScriptedAI() {}

    // *************
    //CreatureAI Functions
    // *************

    void InitializeAI();
    void AttackStartNoMove(Unit* target);
    void AttackStart(Unit* who);

    // Called at any Damage from any attacker (before damage apply)
    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType dmgType) {}

    //Called at World update tick
    virtual void UpdateAI(uint32 diff);

    //Called at creature death
    void JustDied(Unit* /*killer*/) {}

    void RecalcStats() {}

    void ComonOnHome() {}

    //Called at creature killing another unit
    void KilledUnit(Unit* /*victim*/) {}

    // Called when the creature summon successfully other creature
    void JustSummoned(Creature* /*summon*/) {}
    
    // Called when the creature summon successfully gameobject
    void JustSummonedGO(GameObject* /*summon*/) {}

    // Called when a summoned creature is despawned
    void SummonedCreatureDespawn(Creature* /*summon*/) {}

    // Called when hit by a spell
    void SpellHit(Unit* /*caster*/, SpellInfo const* /*spell*/) {}

    // Called when spell hits a target
    void SpellHitTarget(Unit* /*target*/, SpellInfo const* /*spell*/) {}

    /// Called when spell miss a target
    void SpellMissTarget(Unit* /*target*/, SpellInfo const* /*spellInfo*/, SpellMissInfo /*missInfo*/) { }

    //Called at waypoint reached or PointMovement end
    void MovementInform(uint32 /*type*/, uint32 /*id*/) {}

    // Called when AI is temporarily replaced or put back when possess is applied or removed
    void OnPossess(bool /*apply*/) {}

    // Called at any threat added from any attacker (before threat apply)
    void OnAddThreat(Unit* /*victim*/, float& /*fThreat*/, SpellSchoolMask /*schoolMask*/, SpellInfo const* /*threatSpell*/) {}

    // *************
    // Variables
    // *************

    //Pointer to creature we are manipulating
    Creature* me;

    //For fleeing
    bool IsFleeing;

    // *************
    //Pure virtual functions
    // *************

    //Called at creature reset either by death or evade
    void Reset() {}

    //Called at creature aggro either by MoveInLOS or Attack Start
    void EnterCombat(Unit* /*victim*/) {}

    // *************
    //AI Helper Functions
    // *************

    //Start movement toward victim
    void DoStartMovement(Unit* target, float distance = 0.0f, float angle = 0.0f);

    //Start no movement on victim
    void DoStartNoMovement(Unit* target);

    //Stop attack of current victim
    void DoStopAttack();

    //Cast spell by spell info
    void DoCastSpell(Unit* target, SpellInfo const* spellInfo, bool triggered = false);

    //Plays a sound to all nearby players
    void DoPlaySoundToSet(WorldObject* source, uint32 soundId);

    //Drops all threat to 0%. Does not remove players from the threat list
    void DoResetThreat();

    float DoGetThreat(Unit* unit);
    void DoModifyThreatPercent(Unit* unit, int32 pct);

    void DoTeleportTo(float x, float y, float z, uint32 time = 0);
    void DoTeleportTo(float const pos[4]);

    //Teleports a player without dropping threat (only teleports to same map)
    void DoTeleportPlayer(Unit* unit, float x, float y, float z, float o);
    void DoTeleportAll(float x, float y, float z, float o);

    //Returns friendly unit with the most amount of hp missing from max hp
    Unit* DoSelectLowestHpFriendly(float range, uint32 minHPDiff = 1);

    //Returns a list of friendly CC'd units within range
    std::list<Creature*> DoFindFriendlyCC(float range);

    //Returns a list of all friendly units missing a specific buff within range
    std::list<Creature*> DoFindFriendlyMissingBuff(float range, uint32 spellId);

    //Return a player with at least minimumRange from me
    Player* GetPlayerAtMinimumRange(float minRange);

    //Spawns a creature relative to me
    Creature* DoSpawnCreature(uint32 entry, float offsetX, float offsetY, float offsetZ, float angle, uint32 type, uint32 despawntime);

    void SummonCreatureDelay(uint32 delayTimer, uint32 entry, const Position &pos, TempSummonType spawnType = TEMPSUMMON_MANUAL_DESPAWN, uint32 despawnTime = 0)
    {
        SummonCreatureDelay(delayTimer, entry, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation, spawnType, despawnTime);
    }
    void SummonCreatureDelay(uint32 delayTimer, uint32 entry, float x, float y, float z, float orient = 0.0f, TempSummonType spawnType = TEMPSUMMON_MANUAL_DESPAWN, uint32 despawnTime = 0);

    bool HealthBelowPct(uint32 pct) const;
    bool HealthAbovePct(uint32 pct) const;
    float GetHealthPct(uint32 damage) const;
    float GetHealthPctWithHeal(uint32 heal) const;

    //Returns spells that meet the specified criteria from the creatures spell list
    SpellInfo const* SelectSpell(Unit* target, uint32 school, uint32 mechanic, SelectTargetType targets, uint32 powerCostMin, uint32 powerCostMax, float rangeMin, float rangeMax, SelectEffect effect);

    void SetEquipmentSlots(bool loadDefault, int32 mainHand = EQUIP_NO_CHANGE, int32 offHand = EQUIP_NO_CHANGE, int32 ranged = EQUIP_NO_CHANGE);

    //Generally used to control if MoveChase() is to be used or not in AttackStart(). Some creatures does not chase victims
    void SetCombatMovement(bool allowMovement);
    bool IsCombatMovementAllowed() const;

    bool EnterEvadeIfOutOfCombatArea(uint32 const diff);
    bool CheckHomeDistToEvade(uint32 diff, float dist = 0.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f, bool onlyZ = false);

    // return true for heroic mode. i.e.
    //   - for dungeon in mode 10-heroic,
    //   - for raid in mode 10-Heroic
    //   - for raid in mode 25-heroic
    // DO NOT USE to check raid in mode 25-normal.
    bool IsHeroic() const;

    // return the dungeon or raid difficulty
    Difficulty GetDifficultyID() const;

    // return true for 25 man or 25 man heroic mode
    bool Is25ManRaid() const;

    bool IsLfrRaid() const;
    bool IsNormalRaid() const;
    bool IsHeroicRaid() const;
    bool IsMythicRaid() const;
    bool IsHeroicPlusRaid() const;

    template<class T>
    const T& DUNGEON_MODE(const T& normal5, const T& heroic10) const
    {
        switch (_difficulty)
        {
            case DIFFICULTY_NORMAL:
                return normal5;
            case DIFFICULTY_HEROIC:
                return heroic10;
            default:
                break;
        }

        return heroic10;
    }

    template<class T>
    const T& RAID_MODE(const T& normal10, const T& normal25) const
    {
        switch (_difficulty)
        {
            case DIFFICULTY_10_N:
                return normal10;
            case DIFFICULTY_25_N:
                return normal25;
            default:
                break;
        }

        return normal25;
    }

    template<class T>
    const T& RAID_MODE(const T& normal10, const T& normal25, const T& heroic10, const T& heroic25) const
    {
        switch (_difficulty)
        {
            case DIFFICULTY_10_N:
                return normal10;
            case DIFFICULTY_25_N:
                return normal25;
            case DIFFICULTY_10_HC:
                return heroic10;
            case DIFFICULTY_25_HC:
                return heroic25;
            default:
                break;
        }

        return heroic25;
    }

    private:
        Difficulty _difficulty;
        uint32 _evadeCheckCooldown;
        uint32 _checkHomeTimer;
        bool _isCombatMovementAllowed;
        bool _isHeroic;
};

struct Scripted_NoMovementAI : public ScriptedAI
{
    Scripted_NoMovementAI(Creature* creature) : ScriptedAI(creature) {}
    virtual ~Scripted_NoMovementAI() {}

    //Called at each attack of me by any victim
    void AttackStart(Unit* target);
};

class BossAI : public ScriptedAI
{
public:
    BossAI(Creature* creature, uint32 bossId);
    virtual ~BossAI();

    InstanceScript* const instance;
    BossBoundaryMap const* GetBoundary() const;

    void JustSummoned(Creature* summon) override;
    void SummonedCreatureDespawn(Creature* summon) override;

    void UpdateAI(uint32 diff) override;

    void DoZoneInCombatCheck(uint32 diff);

    // Hook used to execute events scheduled into EventMap without the need
    // to override UpdateAI
    // note: You must re-schedule the event within this method if the event
    // is supposed to run more than once
    virtual void ExecuteEvent(uint32 const /*eventId*/) {}
    virtual void ScheduleTasks() {}

    void Reset() override;
    void EnterCombat(Unit* /*who*/) override;
    void JustDied(Unit* /*killer*/) override;
    void JustReachedHome() override;

    //Summons action
    void DoActionSummon(uint32 entry, int32 actionID);

protected:
    void _Reset();
    void _EnterCombat();
    void _JustDied();
    void _JustReachedHome();

    void _DespawnAtEvade(uint32 delayToRespawn = 30, Creature* who = nullptr);

    template<class _Rep, class _Period>
    void _DespawnAtEvade(std::chrono::duration<_Rep, _Period> const& delayToRespawn = Seconds(2), Creature* who = nullptr)
    {
        _DespawnAtEvade(std::chrono::duration_cast<Milliseconds>(delayToRespawn).count(), who);
    }

    bool CheckInRoom();
    bool CheckInArea(uint32 diff, uint32 areaId);
    bool CheckBoundary(Unit* who);

    EventMap events;
    SummonList summons;
    TaskScheduler scheduler;

private:
    virtual bool _EnterEvadeMode();

    BossBoundaryMap const* const _boundary;
    uint32 const _bossId;
    uint32 _checkareaTimer;
    uint32 _checkZoneInCombatTimer;
};

class WorldBossAI : public ScriptedAI
{
    public:
        WorldBossAI(Creature* creature);
        virtual ~WorldBossAI() {}

        void JustSummoned(Creature* summon);
        void SummonedCreatureDespawn(Creature* summon);

        virtual void UpdateAI(uint32 diff);

        // Hook used to execute events scheduled into EventMap without the need
        // to override UpdateAI
        // note: You must re-schedule the event within this method if the event
        // is supposed to run more than once
        virtual void ExecuteEvent(uint32 const /*eventId*/) { }

        void Reset() { _Reset(); }
        void EnterCombat(Unit* /*who*/) { _EnterCombat(); }
        void JustDied(Unit* /*killer*/) { _JustDied(); }

    protected:
        void _Reset();
        void _EnterCombat();
        void _JustDied();

        EventMap events;
        SummonList summons;
};

class BrawlersBossAI : public ScriptedAI
{
    public:
        BrawlersBossAI(Creature* creature);
        virtual ~BrawlersBossAI() {}

        void JustSummoned(Creature* summon);

        void JustDied(Unit* who) { _Reset(); if (who) _WinRound(); }
        void EnterEvadeMode() { _Reset(); _LoseRound();}
        void KilledUnit(Unit*  who);

    protected:
        void _Reset();
        void _WinRound();
        void _LoseRound();

        EventMap events;
        SummonList summons;
};

// SD2 grid searchers.
Creature* GetClosestCreatureWithEntry(WorldObject* source, uint32 entry, float maxSearchRange, bool alive = true);
GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange);
void GetCreatureListWithEntryInGrid(std::list<Creature*>& list, WorldObject* source, uint32 entry, float maxSearchRange);
void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& list, WorldObject* source, uint32 entry, float maxSearchRange);
void GetPlayerListInGrid(std::list<Player*>& list, WorldObject* source, float maxSearchRange);

void GetPositionWithDistInOrientation(Unit* pUnit, float dist, float orientation, float& x, float& y);
void GetPosInRadiusWithRandomOrientation(Unit* unit, float dist, float &x, float &y);
void GetRandPosFromCenterInDist(float centerX, float centerY, float dist, float& x, float& y);

#endif // SCRIPTEDCREATURE_H_
