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

#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"
#include "TotemPackets.h"
#include "CharmInfo.h"

TempSummon::TempSummon(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject) : Creature(isWorldObject), m_Properties(properties), m_loading(false), m_owner(owner), m_bonusSpellDamage(0), m_type(TEMPSUMMON_MANUAL_DESPAWN), m_timer(0), m_lifetime(0), m_timerCheckLocation(3000), onUnload(false), m_petType(), m_requiredAreasID(0)
{
    m_summonerGUID = owner ? owner->GetGUID() : ObjectGuid::Empty;
    m_unitTypeMask |= UNIT_MASK_SUMMON;
}

Unit* TempSummon::GetSummoner() const
{
    return !m_summonerGUID.IsEmpty() ? ObjectAccessor::GetUnit(*this, m_summonerGUID) : nullptr;
}

void TempSummon::Update(uint32 diff)
{
    Creature::Update(diff);

    if (m_deathState == DEAD)
    {
        UnSummon();
        return;
    }

    switch (m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= diff)
            {
                UnSummon();
                return;
            }

            m_timer -= diff;
            break;
        }
        case TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }
        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if (m_deathState == CORPSE)
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat() && isAlive())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        default:
            UnSummon();
            TC_LOG_ERROR(LOG_FILTER_UNITS, "Temporary summoned creature (entry: %u) have unknown type %u of ", GetEntry(), m_type);
            break;
    }

    if (m_timerCheckLocation <= int32(diff))
    {
        CheckLocation();
        m_timerCheckLocation = 3000;
    }
    else
        m_timerCheckLocation -= diff;
}

void TempSummon::InitStats(uint32 duration)
{
    ASSERT(!isPet());

    m_timer = duration;
    m_lifetime = duration;
    uint32 spellID = GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL);
    SpellInfo const* spellInfo = nullptr;
    if (spellID)
        spellInfo = sSpellMgr->GetSpellInfo(spellID);
    if (spellInfo)
        SetRequiredAreas(spellInfo->CastingReq.RequiredAreasID);

    if (m_type == TEMPSUMMON_MANUAL_DESPAWN)
        m_type = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

    Unit* owner = GetSummoner();

    if (owner && isTrigger() && m_templateSpells[0])
    {
        setFaction(owner->getFaction());
        SetLevel(owner->getLevel());
        SetEffectiveLevel(owner->GetEffectiveLevel());
        if (owner->IsPlayer())
            m_ControlledByPlayer = true;
    }

    bool damageSet = false;
    InitBaseStat(GetEntry(), damageSet);

    if (!m_Properties)
        return;

    if (owner)
    {
        bool canUnsummon = false;
        int32 slot = m_Properties->Slot;
        if (m_Properties->Title == 17) //hack for spirit copy
            slot = 17;

        if (slot > MAX_SUMMON_SLOT)
            slot = 0;

        switch (spellID)
        {
            case ResonanceTotem:
                slot = MAX_SUMMON_SLOT - 1;
                canUnsummon = true;
                break;
            case StormTotem:
                slot = MAX_SUMMON_SLOT - 2;
                canUnsummon = true;
                break;
            case EmberTotem:
                slot = MAX_SUMMON_SLOT - 3;
                canUnsummon = true;
                break;
            case TailwindTotem:
                slot = MAX_SUMMON_SLOT - 4;
                canUnsummon = true;
                break;
            default:
                break;
        }

        switch (GetEntry())
        {
            case 76168: // Warrior: Ravager
                slot = 13;
                break;
            case 59271: // Warlock purge gateway
                slot = MAX_SUMMON_SLOT - 1;
                canUnsummon = true;
                break;
            case 59262:
                slot = MAX_SUMMON_SLOT - 2;
                canUnsummon = true;
                break;
            case 47319: // Demonic Gateway
                slot = MAX_SUMMON_SLOT - 3;
                canUnsummon = true;
                break;
            case 61146: // Summon Black Ox Statue
            case 47649: // Efflorescence
            case 60849: // Summon Jade Serpent Statue
            case 105690: // Windfury Totem (PvP Talent)
                canUnsummon = true;
                break;
            default:
                break;
        }

        if (m_Properties->ID == 2915)
        {
            slot = 16;
            canUnsummon = true;
        }

        if (slot)
        {
            if (slot > 0)
            {
                if (canUnsummon)
                {
                    if (owner->m_SummonSlot[slot] && owner->m_SummonSlot[slot] != GetGUID())
                    {
                        Creature* oldSummon = GetMap()->GetCreature(owner->m_SummonSlot[slot]);
                        if (oldSummon && oldSummon->isSummon())
                            oldSummon->ToTempSummon()->UnSummon();
                    }
                }
                owner->m_SummonSlot[slot] = GetGUID();
            }
            else if (slot < 0)
            {
                //Auto get free slot
                slot = SUMMON_SLOT_TOTEM;
                for (int32 i = SUMMON_SLOT_TOTEM; i <= MAX_SUMMON_SLOT; ++i)
                {
                    if (canUnsummon && owner->m_SummonSlot[slot] && owner->m_SummonSlot[slot].GetEntry() == GetEntry())
                    {
                        if (owner->m_SummonSlot[slot] != GetGUID())
                        {
                            Creature* oldSummon = GetMap()->GetCreature(owner->m_SummonSlot[slot]);
                            if (oldSummon && oldSummon->isSummon())
                                oldSummon->ToTempSummon()->UnSummon();
                        }
                        slot = i;
                        break;
                    }
                    if (!owner->m_SummonSlot[i])
                    {
                        slot = i;
                        break;
                    }
                }
                owner->m_SummonSlot[slot] = GetGUID();
            }

            if (slot >= SUMMON_SLOT_TOTEM && slot <= MAX_TOTEM_SLOT)
            {
                if (Player* player = owner->ToPlayer())
                {
                    WorldPackets::Totem::TotemCreated totemCreated;
                    totemCreated.Totem = GetGUID();
                    totemCreated.SpellID = spellID;
                    totemCreated.Duration = duration;
                    totemCreated.Slot = slot - SUMMON_SLOT_TOTEM;
                    totemCreated.TimeMod = 1.0f;
                    totemCreated.CannotDismiss = false;
                    player->SendDirectMessage(totemCreated.Write());
                }
            }
        }
    }

    if (m_Properties->Faction)
        setFaction(m_Properties->Faction);
    else if (IsVehicle() && owner) // properties should be vehicle
        setFaction(owner->getFaction());

    if (owner)
        owner->tempSummonList[GetEntry()].push_back(GetGUID());
}

void TempSummon::InitSummon()
{
    auto owner = GetSummoner();
    if (!owner)
        return;

    if (owner->IsCreature() && owner->ToCreature()->IsAIEnabled)
        owner->ToCreature()->AI()->JustSummoned(this);

    if (IsAIEnabled)
        AI()->IsSummonedBy(owner);
}

bool TempSummon::InitBaseStat(uint32 creatureId, bool& damageSet)
{
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(GetEffectiveLevel(), cinfo->unit_class);
    Unit* owner = GetAnyOwner();

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "TempSummon::InitBaseStat owner %u creatureId %i", owner ? owner->GetGUID() : 0, creatureId);

    PetStats const* pStats = sObjectMgr->GetPetStats(creatureId);
    if (pStats)                                      // exist in DB
    {
        if (pStats->hp && owner)
        {
            SetCreateHealth(int32(owner->GetMaxHealth() * pStats->hp));
            SetMaxHealth(GetCreateHealth());
            SetHealth(GetCreateHealth());
        }
        else if (owner)
        {
            SetCreateHealth(stats->BaseHealth[cinfo->RequiredExpansion]);
            SetMaxHealth(int32(owner->GetMaxHealth() * pStats->hp));
            SetHealth(GetCreateHealth());
        }

        if (getPowerType() != pStats->energy_type)
            setPowerType(Powers(pStats->energy_type));

        if (pStats->energy_type)
        {
            SetMaxPower(Powers(pStats->energy_type), pStats->energy);
            SetPower(Powers(pStats->energy_type), pStats->energy);
        }
        else if (!pStats->energy_type && pStats->energy == 1)
        {
            SetMaxPower(Powers(pStats->energy_type), 0);
            SetPower(Powers(pStats->energy_type), 0);
        }
        else
        {
            if (pStats->energy && owner)
            {
                auto manaMax = int32(owner->GetMaxPower(Powers(pStats->energy_type)) * float(pStats->energy / 100.0f));
                if (!pStats->energy_type)
                    SetCreateMana(manaMax);
                SetMaxPower(Powers(pStats->energy_type), manaMax);
                SetPower(Powers(pStats->energy_type), GetMaxPower(Powers(pStats->energy_type)));
            }
            else
            {
                if (!pStats->energy_type)
                {
                    SetCreateMana(stats->BaseMana);
                    SetMaxPower(Powers(pStats->energy_type), stats->BaseMana);
                }
                SetPower(Powers(pStats->energy_type), GetMaxPower(Powers(pStats->energy_type)));
            }
        }
        if (pStats->damage && owner)
        {
            damageSet = true;
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(owner->GetFloatValue(UNIT_FIELD_MIN_DAMAGE) * pStats->damage));
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(owner->GetFloatValue(UNIT_FIELD_MIN_DAMAGE) * pStats->damage));
        }
        if (pStats->type)
            SetCasterPet(true);
        //Not scale haste for any pets
        if (!pStats->haste)
        {
            SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, 1.0f);
            SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, 1.0f);
            SetFloatValue(UNIT_FIELD_MOD_HASTE, 1.0f);
            SetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN, 1.0f);
            SetFloatValue(UNIT_FIELD_MOD_RANGED_HASTE, 1.0f);
            SetFloatValue(UNIT_FIELD_MOD_TIME_RATE, 1.0f);
        }

        return true;
    }
    return false;
}

void TempSummon::SetTempSummonType(TempSummonType type)
{
    m_type = type;
}

void TempSummon::UnSummon(uint32 msTime)
{
    if (onUnload)
        return;

    if (!IsDespawn())
        m_despawn = true;

    if (msTime)
    {
        m_Events.KillAllEvents(false);
        m_Events.AddEvent(new ForcedUnsummonDelayEvent(*this), m_Events.CalculateTime(msTime));
        return;
    }

    onUnload = true;

    CastPetAuras(false);
    //ASSERT(!isPet());
    if (isPet())
    {
        ToPet()->Remove();
        ASSERT(!IsInWorld());
        return;
    }

    Unit* owner = GetSummoner();

    if (owner && owner->IsPlayer() && GetCreatureTemplate()->Type == CREATURE_TYPE_WILD_PET)
    {
        owner->SetGuidValue(PLAYER_FIELD_SUMMONED_BATTLE_PET_GUID, ObjectGuid::Empty);
        owner->SetUInt32Value(PLAYER_FIELD_CURRENT_BATTLE_PET_BREED_QUALITY, 0);
        owner->SetUInt32Value(UNIT_FIELD_WILD_BATTLE_PET_LEVEL, 0);
    }

    if (owner && owner->IsCreature() && owner->ToCreature()->IsAIEnabled)
        owner->ToCreature()->AI()->SummonedCreatureDespawn(this);

    AddObjectToRemoveList();
}

bool ForcedUnsummonDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.UnSummon();
    return true;
}

void TempSummon::RemoveFromWorld()
{
    onUnload = true;

    if (!IsInWorld())
        return;

    if (m_Properties)
        if (uint32 slot = m_Properties->Slot)
        {
            if (slot > MAX_SUMMON_SLOT)
                slot = 0;

            if (Unit* owner = GetSummoner())
                if (owner->m_SummonSlot[slot] == GetGUID())
                    owner->m_SummonSlot[slot].Clear();
        }

    if (Unit* owner = GetSummoner())
    {
        owner->m_Controlled.erase(GetGUID());
        owner->tempSummonList[GetEntry()].remove(GetGUID());
    }

    //if (GetOwnerGUID())
    //    TC_LOG_ERROR(LOG_FILTER_UNITS, "Unit %u has owner guid when removed from world", GetEntry());

    Creature::RemoveFromWorld();
}

void TempSummon::CheckLocation()
{
    if (!IsInWorld())
        return;

    if (m_requiredAreasID > 0)
    {
        uint32 zone_id, area_id;
        GetZoneAndAreaId(zone_id, area_id);
        bool found = false;
        std::vector<uint32> areaGroupMembers = sDB2Manager.GetAreasForGroup(m_requiredAreasID);
        for (uint32 areaId : areaGroupMembers)
        {
            if (areaId == zone_id || areaId == area_id)
            {
                found = true;
                break;
            }
        }

        if (!found)
            UnSummon(2000); // need wait, can crashed if remove summon from aura
    }
}

Minion::Minion(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject) : TempSummon(properties, owner, isWorldObject)
{
    m_owner = owner;
    ASSERT(m_owner);
    m_unitTypeMask |= UNIT_MASK_MINION;
}

void Minion::InitStats(uint32 duration)
{
    TempSummon::InitStats(duration);

    SetReactState(REACT_PASSIVE);

    SetCreatorGUID(m_owner->GetGUID());
    setFaction(m_owner->getFaction());

    if (m_owner->IsPlayer())
        AddUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER);

    m_owner->SetMinion(this, true);
}

void Minion::RemoveFromWorld()
{
    if (!IsInWorld() || !this)
        return;

    if (Unit* owner = GetSummoner())
        owner->SetMinion(this, false);

    TempSummon::RemoveFromWorld();
}

bool Minion::IsGuardianPet() const
{
    return isPet() || (m_Properties && m_Properties->Control == SUMMON_CATEGORY_PET);
}

bool Minion::IsWarlockPet() const
{
    if (isPet())
    {
        if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            return true;
    }

    return false;
}

Guardian::Guardian(SummonPropertiesEntry const* properties, Unit* owner, bool isWorldObject) : Minion(properties, owner, isWorldObject)
{
    m_owner = owner;
    bool controlable = true;
    memset(m_statFromOwner, 0, sizeof(float)*MAX_STATS);
    m_unitTypeMask |= UNIT_MASK_GUARDIAN;

    if (properties)
    {
        switch (properties->ID)
        {
            case 3459:
            case 3097:
                controlable = false;
                break;
            case 3725:
                m_isHati = true;
                break;
            default:
                break;
        }

        if ((properties->Title == SUMMON_TYPE_PET || properties->Control == SUMMON_CATEGORY_PET) && controlable)
        {
            m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
            InitCharmInfo();
        }
    }
}

void Guardian::InitStats(uint32 duration)
{
    Minion::InitStats(duration);

    InitStatsForLevel(m_owner->GetEffectiveLevel());

    if (m_owner->IsPlayer() && HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
        m_charmInfo->InitCharmCreateSpells();

	if (GetEntry() != 100868)
		SetReactState(REACT_AGGRESSIVE);
	else
		SetReactState(REACT_ATTACK_OFF);
}

void Guardian::InitSummon()
{
    TempSummon::InitSummon();

    if (m_owner->IsPlayer() && m_owner->GetMinionGUID() == GetGUID() && !m_owner->GetCharmGUID() && !m_isHati)
        m_owner->ToPlayer()->CharmSpellInitialize();
}

Puppet::Puppet(SummonPropertiesEntry const* properties, Unit* owner) : Minion(properties, owner, false) //maybe true?
{
    m_owner = owner;
    ASSERT(owner->IsPlayer());
    m_unitTypeMask |= UNIT_MASK_PUPPET;
}

void Puppet::InitStats(uint32 duration)
{
    Minion::InitStats(duration);
    SetLevel(m_owner->getLevel());
    SetEffectiveLevel(m_owner->GetEffectiveLevel());
    SetReactState(REACT_PASSIVE);
}

void Puppet::InitSummon()
{
    Minion::InitSummon();
    SetCharmedBy(m_owner, CHARM_TYPE_POSSESS);
    //if (!SetCharmedBy(m_owner, CHARM_TYPE_POSSESS))
    //ASSERT(false);
}

void Puppet::Update(uint32 time)
{
    Minion::Update(time);
    //check if caster is channelling?
    if (IsInWorld())
    {
        if (!isAlive())
        {
            UnSummon();
            // TODO: why long distance .die does not remove it
        }
    }
}

void Puppet::RemoveFromWorld()
{
    if (!IsInWorld())
        return;

    RemoveCharmedBy(nullptr);
    Minion::RemoveFromWorld();
}
