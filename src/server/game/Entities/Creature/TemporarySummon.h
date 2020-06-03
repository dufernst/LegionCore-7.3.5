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

#ifndef TEMPSUMMON_H
#define TEMPSUMMON_H

#include "Creature.h"

enum SummonActionType
{
    SUMMON_ACTION_TYPE_DEFAULT               = 0,
    SUMMON_ACTION_TYPE_ROUND_HOME_POS        = 1,
    SUMMON_ACTION_TYPE_ROUND_SUMMONER        = 2,
};

/// Stores data for temp summons
struct TempSummonData
{
    Position pos;           ///< Position, where should be creature spawned
    uint32 time;            ///< Despawn time, usable only with certain temp summon types
    uint32 entry;           ///< Entry of summoned creature
    float distance;         ///< Distance from caster for non default action
    uint8 count;            ///< Summon count  for non default action
    uint8 actionType;       ///< Summon action type, option for any summon options
    TempSummonType sumType; ///< Summon type, see TempSummonType for available types
};

class TempSummon : public Creature
{
    public:
        explicit TempSummon(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject);
        virtual ~TempSummon() {}
        void Update(uint32 time) override;
        virtual void InitStats(uint32 lifetime);
        virtual void InitSummon();
        virtual void UnSummon(uint32 msTime = 0);
        void RemoveFromWorld() override;
        bool InitBaseStat(uint32 creatureId, bool& damageSet);
        void SetTempSummonType(TempSummonType type);
        void SaveToDB(uint32 /*mapid*/, uint64 /*spawnMask*/, uint32 /*phaseMask*/) override {}
        Unit* GetSummoner() const;
        ObjectGuid GetSummonerGUID() const { return m_summonerGUID; }
        void SetSummonerGUID(ObjectGuid guid)  { m_summonerGUID = guid; }
        TempSummonType const& GetSummonType() { return m_type; }
        uint32 GetTimer() { return m_timer; }
		void AddDuration(uint32 time) { m_timer += time; }
        void CastPetAuras(bool current, uint32 spellId = 0);
        bool addSpell(uint32 spellId, ActiveStates active = ACT_DECIDE, PetSpellState state = PETSPELL_NEW, PetSpellType type = PETSPELL_NORMAL);
        bool removeSpell(uint32 spell_id);
        void LearnPetPassives();
        void InitLevelupSpellsForLevel();
        void UpdateAttackPowerAndDamage(bool ranged = false) override;

        bool learnSpell(uint32 spell_id);
        bool unlearnSpell(uint32 spell_id);
        void ToggleAutocast(SpellInfo const* spellInfo, bool apply);

        PetType getPetType() const { return m_petType; }
        void setPetType(PetType type) { m_petType = type; }

        int32 GetRequiredAreas() const { return m_requiredAreasID; }
        void SetRequiredAreas(int32 requiredAreasID) { m_requiredAreasID = requiredAreasID; }
        void CheckLocation();

        int32 GetBonusDamage() { return m_bonusSpellDamage; }

        const SummonPropertiesEntry* const m_Properties;
        bool    m_loading;
        Unit*   m_owner;
        int32 m_bonusSpellDamage;

    private:
        TempSummonType m_type;
        uint32 m_timer;
        uint32 m_lifetime;
        int32 m_timerCheckLocation;
        ObjectGuid m_summonerGUID;
        bool onUnload;
        PetType m_petType;
        int32 m_requiredAreasID;
};

class Minion : public TempSummon
{
    public:
        Minion(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject);
        void InitStats(uint32 duration) override;
        void RemoveFromWorld() override;

        Unit* GetOwner() { return m_owner; }
        bool IsPetGhoul() const {return GetEntry() == 26125;} // Ghoul may be guardian or pet
        bool IsPetGargoyle() const { return GetEntry() == 27829; }
        bool IsWarlockPet() const;
        bool IsGuardianPet() const;
};

class Guardian : public Minion
{
    public:
        Guardian(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject);
        void InitStats(uint32 duration) override;
        bool InitStatsForLevel(uint8 level, bool initSpells = true);
        void InitSummon() override;

        bool UpdateStats(Stats stat) override;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateMaxHealth() override;
        void UpdateMaxPower(Powers power) override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void UpdateDamagePhysical(WeaponAttackType attType) override;

        void SetBonusDamage(int32 SPD);

    protected:
        float   m_statFromOwner[MAX_STATS]{};
};

class Puppet : public Minion
{
    public:
        Puppet(SummonPropertiesEntry const* properties, Unit* owner);
        void InitStats(uint32 duration) override;
        void InitSummon() override;
        void Update(uint32 time) override;
        void RemoveFromWorld() override;
};

class ForcedUnsummonDelayEvent : public BasicEvent
{
public:
    ForcedUnsummonDelayEvent(TempSummon& owner) : BasicEvent(), m_owner(owner) { }
    bool Execute(uint64 e_time, uint32 p_time) override;

private:
    TempSummon& m_owner;
};
#endif
