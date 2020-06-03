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

#include "CharmInfo.h"
#include "Common.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "Group.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "PetPackets.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "SpellPackets.h"
#include "Unit.h"
#include "Util.h"
#include "WorldPacket.h"

Pet::Pet(Player* owner, PetType type) : Guardian(nullptr, owner, true),
m_removed(false), m_duration(0), m_specialization(0), m_groupUpdateMask(0), m_declinedname(nullptr)
{
    m_slot = PET_SLOT_UNK_SLOT;
    m_owner = static_cast<Unit*>(owner);
    m_loading = false;
    if(m_owner && m_owner->getClass() == CLASS_HUNTER)
    {
        type = HUNTER_PET;
        setPetType(type);
    }
    m_unitTypeMask &= ~UNIT_MASK_MINION;

    m_unitTypeMask |= UNIT_MASK_PET;
    if (type == HUNTER_PET)
        m_unitTypeMask |= UNIT_MASK_HUNTER_PET;

    if (!(m_unitTypeMask & UNIT_MASK_CONTROLABLE_GUARDIAN))
    {
        m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
        InitCharmInfo();
    }

    m_name = "Pet";
    m_isUpdate = false;
    objectCountInWorld[uint8(HighGuid::Pet)]++;
}

Pet::~Pet()
{
    delete m_declinedname;
    objectCountInWorld[uint8(HighGuid::Pet)]--;
}

void Pet::AddToWorld()
{
    ///- Register the pet for guid lookup
    if (!IsInWorld())
    {
        ///- Register the pet for guid lookup
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        AIM_Initialize();
    }

    // Prevent stuck pets when zoning. Pets default to "follow" when added to world
    // so we'll reset flags and let the AI handle things
    if (GetCharmInfo() && GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        GetCharmInfo()->SetIsCommandAttack(false);
        GetCharmInfo()->SetIsAtStay(false);
        GetCharmInfo()->SetIsFollowing(false);
        GetCharmInfo()->SetIsReturning(false);
    }

}

void Pet::RemoveFromWorld()
{
    ///- Remove the pet from the accessor
    if (IsInWorld())
    {
        ///- Don't call the function for Creature, normal mobs + totems go in a different storage
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool Pet::LoadPetFromDB(Player* owner, uint32 petentry, uint32 petnumber)
{
    if (owner->getClass() == CLASS_WARLOCK)
        if (owner->HasAura(196099))
            owner->RemoveAura(196099);

    if (owner->IsPlayer() && isControlled() && !isTemporarySummoned())
        owner->SetLastPetEntry(petentry);

    m_loading = true;
    uint32 ownerid = owner->GetGUIDLow();

    // find number
    if (!petnumber && !petentry)
    {
        if(owner->m_currentSummonedSlot < PET_SLOT_HUNTER_FIRST)
        {
            TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error m_currentSummonedSlot < 0");
            m_loading = false;
            return false;
        }
        petnumber = owner->getPetIdBySlot(owner->m_currentSummonedSlot);
    }

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "LoadPetFromDB petentry %i, petnumber %i, ownerid %i slot %i m_currentPet %i", petentry, petnumber, ownerid, owner->m_currentSummonedSlot, owner->m_currentPetNumber);

    PetInfoData* petInfo = owner->GetPetInfo(petentry, petnumber);
    if (!petInfo)
    {
        TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error result");
        m_loading = false;
        return false;
    }

    // update for case of current pet "slot = 0"
    petentry = petInfo->entry;
    if (!petentry)
    {
        TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error petentry");
        m_loading = false;
        return false;
    }

    // Double call if use Player::SummonPet
    // But this option for check through Player::LoadPet
    if (!owner->CanSummonPet(petentry))
    {
        TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error CanSummonPet");
        m_loading = false;
        return false;
    }

    uint32 summon_spell_id = petInfo->CreatedBySpell;
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(summon_spell_id);

    bool is_temporary_summoned = spellInfo && spellInfo->GetDuration(GetSpawnMode()) > 0;

    // check temporary summoned pets like mage water elemental
    if (is_temporary_summoned)
    {
        m_loading = false;
        TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error is_temporary_summoned %i", is_temporary_summoned);
        return false;
    }

    setPetType(petInfo->pet_type);

    if (owner->IsPlayer() && isControlled() && !isTemporarySummoned())
        owner->SetLastPetEntry(petentry);

    if (petInfo->pet_type == HUNTER_PET)
    {
        CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(petentry);
        if (!creatureInfo || !creatureInfo->isTameable(owner))
            return false;
    }

    if (owner->IsPetNeedBeTemporaryUnsummoned())
    {
        TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error IsPetNeedBeTemporaryUnsummoned");
        owner->SetTemporaryUnsummonedPetNumber(petInfo->id);
        return false;
    }

    Map* map = owner->GetMap();
    ObjectGuid::LowType guid = sObjectMgr->GetGenerator<HighGuid::Pet>()->Generate();
    if (!Create(guid, map, owner->GetPhaseMask(), petentry, petInfo->id))
    {
        TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::LoadPetFromDB error Create");
        return false;
    }

    if(petnumber)
    {
        Position pos;
        owner->GetFirstCollisionPosition(pos, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        Relocate(pos.m_positionX, pos.m_positionY, pos.m_positionZ, owner->GetOrientation());

        if (petInfo->pet_type == HUNTER_PET)
            owner->m_currentPetNumber = petnumber;
    }

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_PETS, "Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    SetPhaseId(owner->GetPhases(), false);
    setFaction(owner->getFaction());
    SetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL, summon_spell_id);

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo->Type == CREATURE_TYPE_CRITTER)
    {
        SetTratsport(owner->GetTransport());
        map->AddToMap(this->ToCreature());
        return true;
    }

    m_charmInfo->SetPetNumber(petInfo->id, IsPermanentPetFor(owner));

    SetDisplayId(petInfo->modelid);
    SetNativeDisplayId(petInfo->modelid);
    SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    SetName(petInfo->name);

    // ignore model info data for 
    SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, DEFAULT_WORLD_OBJECT_SIZE);
    SetFloatValue(UNIT_FIELD_COMBAT_REACH, DEFAULT_COMBAT_REACH);

    switch (getPetType())
    {
        case SUMMON_PET:
        {
            petInfo->level = owner->GetEffectiveLevel();
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet dismiss, cancel)
            if (owner->getClass() == CLASS_WARLOCK)
                SetClass(CLASS_ROGUE);
            break;
        }
        case HUNTER_PET:
        {
            SetSheath(SHEATH_STATE_MELEE);
            SetClass(CLASS_HUNTER);
            SetGender(GENDER_NONE);

            SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, petInfo->renamed ? UNIT_CAN_BE_ABANDONED : UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet abandon, cancel)
            SetSpecialization(petInfo->specialization);
            break;
        }
        default:
            if (!IsPetGhoul())
                TC_LOG_ERROR(LOG_FILTER_PETS, "Pet have incorrect type (%u) for pet loading.", getPetType());
            break;
    }

    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr))); // cast can't be helped here
    SetCreatorGUID(owner->GetGUID());

    if (owner->IsPlayer())
        AddUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER);

    SetReactState(petInfo->reactstate);
    if (GetReactState() == REACT_AGGRESSIVE)
        SetReactState(REACT_DEFENSIVE);

    InitStatsForLevel(petInfo->level);
    SetUInt32Value(UNIT_FIELD_PET_EXPERIENCE, petInfo->exp);

    SynchronizeLevelWithOwner();
    SetCanModifyStats(true);

    uint32 savedhealth = petInfo->curhealth;
    uint32 savedmana = petInfo->curmana;

    if (!savedhealth && getPetType() == HUNTER_PET)
        setDeathState(JUST_DIED);
    else if (getPetType() == HUNTER_PET)
    {
        SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : savedhealth);
        SetPower(getPowerType(), savedmana > uint32(GetMaxPower(getPowerType())) ? GetMaxPower(getPowerType()) : savedmana);
    }

    SetTratsport(owner->GetTransport());

    owner->SetMinion(this, true);
    map->AddToMap(this->ToCreature());
    setActive(true);

    SetSlot(owner->m_currentSummonedSlot);

    auto timediff = uint32(time(nullptr) - petInfo->savetime);
    _LoadAuras(timediff);

    if (owner->IsPlayer() && owner->ToPlayer()->InArena())
        RemoveArenaAuras();

    // load action bar, if data broken will fill later by default spells.
    m_charmInfo->LoadPetActionBar(petInfo->abdata);

    _LoadSpells();
    _LoadSpellCooldowns();
    LearnPetPassives();

    if (owner->HasSpell(108415)) // active talent Soul Link
        CastSpell(this, 108446, true);

    if (owner->HasItemOrGemWithIdEquipped(137037, EFFECT_1)) // active Uvanimor, the Unbeautiful
        owner->CastSpell(this, 208786, true);

    if (AuraEffect* aurEff = owner->GetAuraEffect(238073, EFFECT_0)) // Left Hand of Darkness
    {
        float bp0 = owner->HasAura(152107) ? aurEff->GetAmount() / 2 : aurEff->GetAmount();
        CastCustomSpell(this, 238998, &bp0, nullptr, nullptr, true);
    }

    if (AuraEffect* aurEff = owner->GetAuraEffect(203669, EFFECT_0)) // Fluffy, Go
    {
        float bp0 = aurEff->GetAmount();
        CastCustomSpell(this, 218955, &bp0, nullptr, nullptr, true);
    }

    CleanupActionBar();                                     // remove unknown spells from action bar after load
    owner->PetSpellInitialize();

    TC_LOG_DEBUG(LOG_FILTER_PETS, "New Pet has guid %u", GetGUIDLow());

    SetGroupUpdateFlag(GROUP_UPDATE_PET_FULL);

    owner->SendTalentsInfoData(true);

    if (getPetType() == HUNTER_PET)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_DECLINED_NAME);
        stmt->setUInt64(0, owner->GetGUIDLow());
        stmt->setUInt32(1, GetCharmInfo()->GetPetNumber());
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
        {
            delete m_declinedname;
            m_declinedname = new DeclinedName;
            Field* fields2 = result->Fetch();
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            {
                m_declinedname->name[i] = fields2[i].GetString();
            }
        }
        CheckSpecialization();
    }

    //set last used pet number (for use in BG's)
    if (owner->IsPlayer() && isControlled() && !isTemporarySummoned() && getPetType() == HUNTER_PET)
        owner->ToPlayer()->SetLastPetNumber(petInfo->id);

    CastPetAuras(getDeathState() == ALIVE);
    m_loading = false;
    //owner->SetFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CONTROLLING_PET);

    return true;
}

void Pet::SavePetToDB(bool isDelete)
{
    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Pet SavePetToDB");
    if (!GetEntry())
        return;

    // save only fully controlled creature
    if (!isControlled())
        return;

    //Don`t save temporary pets
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL));
    bool is_temporary_summoned = spellInfo && spellInfo->GetDuration(GetSpawnMode()) > 0;
    if (is_temporary_summoned)
        return;

    if (!GetOwner())
        return;

    auto owner = GetOwner()->ToPlayer();
    if (!owner || !owner->GetGUID().IsPlayer()) // not save not player pets
        return;

    // In some cases pet could get 0 id (stamped case)
    if (!m_charmInfo->GetPetNumber())
        return;

    PetSlot curentSlot = GetSlot();

    //not delete, just remove from curent slot
    if((m_owner->getClass() == CLASS_WARLOCK || m_owner->getClass() == CLASS_DEATH_KNIGHT) && isDelete)
        owner->m_currentPetNumber = 0;

    // not save pet as current if another pet temporary unsummoned
    if (owner->GetTemporaryUnsummonedPetNumber() && owner->GetTemporaryUnsummonedPetNumber() != m_charmInfo->GetPetNumber())
    {
        // pet will lost anyway at restore temporary unsummoned
        if (getPetType() == HUNTER_PET)
            return;
    }

    uint32 curhealth = GetHealth();
    uint32 curmana = GetPower(POWER_MANA);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // save auras before possibly removing them
    if (getPetType() == HUNTER_PET)
        _SaveAuras(trans);
    _SaveSpells(trans);
    _SaveSpellCooldowns(trans);
    CharacterDatabase.CommitTransaction(trans);

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SavePetToDB petentry %i, petnumber %i, slotID %i ownerid %i GetNativeDisplayId %u", GetEntry(), m_charmInfo->GetPetNumber(), curentSlot, GetOwnerGUID().GetGUIDLow(), GetNativeDisplayId());

    // current/stable/not_in_slot
    if (!isDelete)
    {
        // save pet
        uint8 index = 0;
        std::ostringstream ss;

        PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET);
        stmt->setUInt32(index++, m_charmInfo->GetPetNumber());
        stmt->setUInt32(index++, GetEntry());
        stmt->setUInt64(index++, GetOwnerGUID().GetGUIDLow());
        stmt->setUInt32(index++, GetNativeDisplayId());
        stmt->setUInt32(index++, getLevel());
        stmt->setUInt32(index++, GetUInt32Value(UNIT_FIELD_PET_EXPERIENCE));
        stmt->setUInt8(index++, GetReactState());
        stmt->setString(index++, GetName());
        stmt->setUInt8(index++, HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_CAN_BE_RENAMED) ? 0 : 1);
        stmt->setUInt32(index++, curhealth);
        stmt->setUInt32(index++, curmana);

        ss.str("");
        for (uint32 i = ACTION_BAR_INDEX_START; i < ACTION_BAR_INDEX_END; ++i)
        {
            ss << uint32(m_charmInfo->GetActionBarEntry(i)->GetType()) << ' '
                << uint32(m_charmInfo->GetActionBarEntry(i)->GetAction()) << ' ';
        };

        stmt->setString(index++, ss.str());

        stmt->setUInt32(index++, time(nullptr));
        stmt->setUInt32(index++, GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL));
        stmt->setUInt8(index++, getPetType());
        stmt->setUInt32(index++, GetSpecializationId());

        CharacterDatabase.Execute(stmt);
    }
    // delete
    else
    {
        owner->DeletePetInfo(m_charmInfo->GetPetNumber());
        if((curentSlot >= PET_SLOT_HUNTER_FIRST && curentSlot <= owner->GetMaxCurentPetSlot()))
            owner->cleanPetSlotForMove(curentSlot, m_charmInfo->GetPetNumber());     //could be already remove by early call this function
        RemoveAllAuras();
        DeleteFromDB(m_charmInfo->GetPetNumber());
    }
}

void Pet::DeleteFromDB(uint32 guidlow)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_BY_ID);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_DECLINED_BY_ID);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_AURA_BY_ID);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_SPELL_BY_ID);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_SPELL_COOLDOWN_BY_ID);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void Pet::setDeathState(DeathState s)                       // overwrite virtual Creature::setDeathState and Unit::setDeathState
{
    Creature::setDeathState(s);
    if (getDeathState() == CORPSE)
    {
        if (getPetType() == HUNTER_PET)
        {
            // pet corpse non lootable and non skinnable
            SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            //SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }
    }
    else if (getDeathState() == ALIVE)
    {
        if (getPetType() == HUNTER_PET)
        {
            if (Unit* owner = GetOwner())
                if (Player* player = owner->ToPlayer())
                    player->StopCastingCharm();
        }
    }
    if (s == ALIVE)
        CastPetAuras(true);
    else if (s == JUST_DIED)
        CastPetAuras(false);
}

void Pet::Update(uint32 diff)
{
    if (m_removed || !IsInWorld() || IsDelete() || IsPreDelete())
        return;

    if (m_loading)
        return;
    
    Unit* owner = GetOwner();
    if (!owner || !owner->IsPlayer() || m_isUpdate)
        return;

    m_isUpdate = true;

    switch (m_deathState)
    {
        case CORPSE:
        {
            if (getPetType() != HUNTER_PET || m_corpseRemoveTime <= time(nullptr))
            {
                Remove();               //hunters' pets never get removed because of death, NEVER!
                m_isUpdate = false;
                return;
            }
            break;
        }
        case ALIVE:
        {
            // unsummon pet that lost owner
            Unit* petOwner = GetOwner();
            if (!petOwner || (!IsWithinDistInMap(petOwner, GetMap()->GetVisibilityRange()) && !isPossessed()) || (isControlled() && !petOwner->GetPetGUID()))
            {
                if (!petOwner || !GetTransport() || GetTransport() != petOwner->GetTransport()) // waiting full teleport owner
                {
                    Remove();
                    m_isUpdate = false;
                    return;
                }
            }

            if (isControlled())
            {
                if (petOwner->GetPetGUID() != GetGUID()) // Stampede
                {
                    TC_LOG_ERROR(LOG_FILTER_PETS, "Pet %u is not pet of owner %s, removed", GetEntry(), m_owner->GetName());
                    Remove();
                    m_isUpdate = false;
                    return;
                }
            }

            if (m_duration > 0)
            {
                if (uint32(m_duration) > diff)
                    m_duration -= diff;
                else
                {
                    Remove();
                    m_isUpdate = false;
                    return;
                }
            }
            break;
        }
        default:
            break;
    }
    m_isUpdate = false;
    Creature::Update(diff);
}

void Creature::Regenerate(Powers power, uint32 diff)
{
    int32 maxValue = GetMaxPower(power);

    uint32 powerIndex = GetPowerIndex(power);
    if (powerIndex == MAX_POWERS)
        return;

    int32 saveCur = GetPower(power);
    int32 curValue = saveCur;

    if (!maxValue || curValue == maxValue)
        return;

    float addvalue = 0.0f;
    float regenTypeAndMod = 1.0f; // start type regen + or - for power

    switch (power)
    {
        case POWER_MANA:
        {
            // Combat and any controlled creature
            if (isInCombat() || GetCharmerOrOwnerGUID())
                addvalue = uint32(5.0f + 17.0f);
            else
                addvalue = maxValue / 3.0f;
            break;
        }
        // Regenerate Rage
        case POWER_RAGE:
        {
            /*if (!HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
            {
                float RageDecreaseRate = sWorld->getRate(RATE_POWER_RAGE_LOSS);
                addvalue -= 25.0f * RageDecreaseRate / GetFloatValue(UNIT_MOD_HASTE);                // 2.5 rage by tick (= 2 seconds => 1.25 rage/sec)
                if (!isInCombat()) // Defensive Stance add rage only in combat
                    regenTypeAndMod = 0.0f;
            }*/
            break;
        }
        case POWER_FOCUS:
        {
            float defaultreg = 0.01f * diff;
            addvalue += defaultreg * sWorld->getRate(RATE_POWER_FOCUS);
            break;
        }
        case POWER_ENERGY:
        {
            float defaultreg = 0.01f * diff;
            addvalue += defaultreg * sWorld->getRate(RATE_POWER_ENERGY);
            break;
        }
        default:
            return;
    }

    // Apply modifiers (if any).
    if (isInCombat())
        addvalue += GetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER + powerIndex) * (0.001f * diff) * regenTypeAndMod;
    else
        addvalue += GetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER + powerIndex) * (0.001f * diff) * regenTypeAndMod;

    addvalue += m_powerFraction[powerIndex];

    if (addvalue <= 0.0f)
    {
        if (curValue == 0)
            return;
    }
    else if (addvalue > 0.0f)
    {
        if (saveCur == maxValue)
            return;
    }
    else
        return;

    if (this->IsAIEnabled)
        this->AI()->RegeneratePower(power, addvalue);

    auto integerValue = int32(fabs(addvalue));

    if (addvalue < 0.0f)
    {
        if (curValue > integerValue)
        {
            curValue -= integerValue;
            m_powerFraction[powerIndex] = addvalue + integerValue;
        }
        else
        {
            curValue = 0;
            m_powerFraction[powerIndex] = 0;
        }

        //Visualization for power
        //VisualForPower(power, curValue, int32(integerValue)*-1, true);
    }
    else
    {
        curValue += integerValue;

        if (curValue > maxValue)
        {
            curValue = maxValue;
            m_powerFraction[powerIndex] = 0;
        }
        else
            m_powerFraction[powerIndex] = addvalue - integerValue;

        //Visualization for power
        //VisualForPower(power, curValue, integerValue, true);
    }

    if ((saveCur != maxValue && curValue == maxValue) || m_regenTimerCount >= GetRegenerateInterval())
    {
        SetInt32Value(UNIT_FIELD_POWER + powerIndex, curValue);
    }
    else
    {
        UpdateInt32Value(UNIT_FIELD_POWER + powerIndex, curValue);
    }
}

void Pet::Remove()
{
    if (Player* plr = m_owner->ToPlayer())
        plr->RemovePet(this);
}

void Pet::GivePetLevel(uint8 level)
{
    if (!level || level == getLevel())
        return;

    if (getPetType()==HUNTER_PET)
    {
        SetUInt32Value(UNIT_FIELD_PET_EXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, 2147483647);
    }

    InitStatsForLevel(level);
}

bool Pet::CreateBaseAtCreature(Creature* creature)
{
    ASSERT(creature);

    if (!CreateBaseAtTamed(creature->GetCreatureTemplate(), creature->GetMap(), creature->GetPhaseMask()))
        return false;

    Relocate(*creature);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_PETS, "Pet (guidlow %d, entry %d) not created base at creature. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (!cinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_PETS, "CreateBaseAtCreature() failed, creatureInfo is missing!");
        return false;
    }

    SetDisplayId(creature->GetDisplayId());
    SetNativeDisplayId(creature->GetNativeDisplayId());

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->Family))
        SetName(cFamily->Name->Str[sObjectMgr->GetDBCLocaleIndex()]);
    else
        SetName(creature->GetNameForLocaleIdx(sObjectMgr->GetDBCLocaleIndex()));

    return true;
}

bool Pet::CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner)
{
    if (!CreateBaseAtTamed(cinfo, owner->GetMap(), owner->GetPhaseMask()))
        return false;

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->Family))
        SetName(cFamily->Name->Str[sObjectMgr->GetDBCLocaleIndex()]);

    Relocate(*owner);

    return true;
}

bool Pet::CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask)
{
    TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::CreateBaseForTamed");
    ObjectGuid::LowType guid = sObjectMgr->GetGenerator<HighGuid::Pet>()->Generate();
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if (!Create(guid, map, phaseMask, cinfo->Entry, pet_number))
        return false;

    setPowerType(POWER_FOCUS);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    SetUInt32Value(UNIT_FIELD_PET_EXPERIENCE, 0);
    SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, 2147483647);
    SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    if (cinfo->Type == CREATURE_TYPE_BEAST)
    {
        SetClass(CLASS_WARRIOR);
        SetGender(GENDER_NONE);
        SetFieldPowerType(POWER_FOCUS);
        SetUInt32Value(UNIT_FIELD_DISPLAY_POWER, 0x2);
        SetSheath(SHEATH_STATE_MELEE);
        SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);
    }

    return true;
}

// TODO: Move stat mods code to pet passive auras
bool Guardian::InitStatsForLevel(uint8 petlevel, bool initSpells)
{
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    ASSERT(cinfo);

    SetLevel(petlevel);
    Unit* owner = GetOwner();
    SetEffectiveLevel(owner ? owner->GetEffectiveLevel() : 0);
    bool damageSet = false;

    //Determine pet type
    PetType petType = getPetType();

    uint32 creature_ID = 0;
    if (petType == HUNTER_PET)
        creature_ID = 1;
    else
        creature_ID = cinfo->Entry;

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "Guardian::InitStatsForLevel owner %u creature_ID %i petlevel %i", owner ? owner->GetGUID() : 0, creature_ID, petlevel);

    SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));

    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(petlevel*50));

    UpdateMeleeHastMod();
    UpdateHastMod();
    UpdateRangeHastMod();
    UpdateCastHastMods();

    SetUInt32Value(UNIT_FIELD_FLAGS_2, cinfo->unit_flags2);

    // Hunters pet should not inherit resistances from creature_template, they have separate auras for that
    if (!isHunterPet())
        for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            SetModifierValue(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(cinfo->resistance[i]));

    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(petlevel, cinfo->unit_class);

    //health, mana, armor and resistance
    if(!InitBaseStat(creature_ID, damageSet))
    {
        SetCreateHealth(stats->BaseHealth[cinfo->RequiredExpansion]);
        SetCreateMana(stats->BaseMana);
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    }

    if(owner && (owner->getClass() == CLASS_HUNTER || owner->getClass() == CLASS_WARLOCK))
    {
        SetCreateStat(STAT_STRENGTH, 22 + (petlevel * 6.2));
        SetCreateStat(STAT_AGILITY, 23 + (petlevel * 13.3));
        SetCreateStat(STAT_STAMINA, 22 + (petlevel * 4.4));
        SetCreateStat(STAT_INTELLECT, 20 + (petlevel * 0.6));
    }
    else
    {
        SetCreateStat(STAT_STRENGTH, 21 + (petlevel * 5.566));
        SetCreateStat(STAT_AGILITY, 17 + (petlevel * 29.92));
        SetCreateStat(STAT_STAMINA, 21 + (petlevel * 1.277));
        SetCreateStat(STAT_INTELLECT, 14 + (petlevel * 4.45));
    }

    SetBonusDamage(0);
    UpdateAllStats();

    if(!damageSet)
    {
        SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
        SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
    }

    if (petType == HUNTER_PET)
    {
        SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, 2147483647);
        if (Player * plr = m_owner->ToPlayer())
        {
            float ratingBonusVal = plr->GetRatingBonusValue(CR_HASTE_RANGED);
            float val = 1.0f;

            ApplyPercentModFloatVar(val, ratingBonusVal, false);
            CalcAttackTimePercentMod(BASE_ATTACK, val);
        }
    }
    else
        SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, 2147483647);

    if (initSpells)
        InitLevelupSpellsForLevel();

    SetFullHealth();
    return true;
}

void TempSummon::ToggleAutocast(SpellInfo const* spellInfo, bool apply)
{
    if (!spellInfo->IsAutocastable())
        return;

    uint32 spellid = spellInfo->Id;

    auto itr = m_spells.find(spellid);
    if (itr == m_spells.end())
        return;

    uint32 i;

    if (apply)
    {
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i)
            ;                                               // just search

        if (i == m_autospells.size())
        {
            m_autospells.push_back(spellid);

            if (itr->second.active != ACT_ENABLED)
            {
                itr->second.active = ACT_ENABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
    else
    {
        auto itr2 = m_autospells.begin();
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i, ++itr2)
            ;                                               // just search

        if (i < m_autospells.size())
        {
            m_autospells.erase(itr2);
            if (itr->second.active != ACT_DISABLED)
            {
                itr->second.active = ACT_DISABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
}

bool Pet::HaveInDiet(ItemTemplate const* item) const
{
    if (!item->FoodType)
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->Family);
    if (!cFamily)
        return false;

    uint32 diet = cFamily->PetFoodMask;
    uint32 FoodMask = 1 << (item->FoodType-1);
    return (diet & FoodMask) != 0;
}

uint32 Pet::GetCurrentFoodBenefitLevel(uint32 itemlevel)
{
    // -5 or greater food level
    if (getLevel() <= itemlevel + 5)                         //possible to feed level 60 pet with level 55 level food for full effect
        return 35000;
    // -10..-6
    if (getLevel() <= itemlevel + 10)                   //pure guess, but sounds good
        return 17000;
        // -14..-11
    if (getLevel() <= itemlevel + 14)                   //level 55 food gets green on 70, makes sense to me
        return 8000;
        // -15 or less
    return 0;
    //food too low level
}

void Pet::_LoadSpellCooldowns()
{
    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SPELL_COOLDOWN);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        time_t curTime = time(nullptr);

        WorldPackets::Spells::SpellCooldown cooldowns;
        cooldowns.Caster = GetGUID();
        cooldowns.Flags = 0;
        
        do
        {
            Field* fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();
            auto db_time  = time_t(fields[1].GetUInt32());

            if (!sSpellMgr->GetSpellInfo(spell_id))
            {
                TC_LOG_ERROR(LOG_FILTER_PETS, "Pet %u have unknown spell %u in `pet_spell_cooldown`, skipping.", m_charmInfo->GetPetNumber(), spell_id);
                continue;
            }

            // skip outdated cooldown
            if (db_time <= curTime)
                continue;

            cooldowns.SpellCooldowns.emplace_back(spell_id, uint32(db_time-curTime) * IN_MILLISECONDS);

            _AddCreatureSpellCooldown(spell_id, db_time);

            TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet (Number: %u) spell %u cooldown loaded (%u secs).", m_charmInfo->GetPetNumber(), spell_id, uint32(db_time-curTime));
        }
        while (result->NextRow());

        auto owner = GetOwner()->ToPlayer();
        if (!m_CreatureSpellCooldowns.empty() && owner)
            owner->SendDirectMessage(cooldowns.Write());
    }
}

void Pet::_SaveSpellCooldowns(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_COOLDOWNS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    time_t curTime = time(nullptr);

    // remove oudated and save active
    for (auto itr = m_CreatureSpellCooldowns.begin(); itr != m_CreatureSpellCooldowns.end();)
    {
        if (itr->second <= curTime)
            m_CreatureSpellCooldowns.erase(itr++);
        else
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SPELL_COOLDOWN);
            stmt->setUInt32(0, m_charmInfo->GetPetNumber());
            stmt->setUInt32(1, itr->first);
            stmt->setUInt32(2, uint32(itr->second));
            trans->Append(stmt);

            ++itr;
        }
    }
}

void Pet::_LoadSpells()
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_SPELL);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            addSpell(fields[0].GetUInt32(), ActiveStates(fields[1].GetUInt8()), PETSPELL_UNCHANGED);
        }
        while (result->NextRow());
    }
}

void Pet::_SaveSpells(SQLTransaction& trans)
{
    for (auto itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        ++next;

        // prevent saving family passives to DB
        if (itr->second.type == PETSPELL_FAMILY)
            continue;

        PreparedStatement* stmt;

        switch (itr->second.state)
        {
            case PETSPELL_REMOVED:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_BY_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                trans->Append(stmt);

                m_spells.erase(itr);
                continue;
            case PETSPELL_CHANGED:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_BY_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                stmt->setUInt8(2, itr->second.active);
                trans->Append(stmt);

                break;
            case PETSPELL_NEW:
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_SPELL);
                stmt->setUInt32(0, m_charmInfo->GetPetNumber());
                stmt->setUInt32(1, itr->first);
                stmt->setUInt8(2, itr->second.active);
                trans->Append(stmt);
                break;
            case PETSPELL_UNCHANGED:
                continue;
        }
        itr->second.state = PETSPELL_UNCHANGED;
    }
}

void Pet::_LoadAuras(uint32 timediff)
{
    TC_LOG_DEBUG(LOG_FILTER_PETS, "Loading auras for pet %u", GetGUIDLow());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_AURA);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_AURA_EFFECT);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    PreparedQueryResult resultEffect = CharacterDatabase.Query(stmt);

    std::list<auraEffectData> auraEffectList;
    if(resultEffect)
    {
        do
        {
            Field* fields = resultEffect->Fetch();
            uint8 slot = fields[0].GetUInt8();
            uint8 effect = fields[1].GetUInt8();
            uint32 amount = fields[2].GetUInt32();
            uint32 baseamount = fields[3].GetUInt32();

            auraEffectList.emplace_back(slot, effect, amount, baseamount);
        }
        while (resultEffect->NextRow());
    }

    ObjectGuid caster_guid;
    if (result)
    {
        do
        {
            int32 damage[32];
            float baseDamage[32];
            Field* fields = result->Fetch();
            uint8 slot = fields[0].GetUInt8();
            caster_guid.SetRawValue(fields[1].GetBinary());
            // NULL guid stored - pet is the caster of the spell - see Pet::_SaveAuras
            if (caster_guid.IsEmpty())
                caster_guid = GetGUID();
            uint32 spellid = fields[2].GetUInt32();
            uint32 effmask = fields[3].GetUInt32();
            uint32 recalculatemask = fields[4].GetUInt32();
            uint8 stackcount = fields[5].GetUInt8();
            int32 maxduration = fields[6].GetInt32();
            int32 remaintime = fields[7].GetInt32();
            uint8 remaincharges = fields[8].GetUInt8();

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_PETS, "Unknown aura (spellid %u), ignore.", spellid);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && !spellInfo->IsPositive())
            {
                if (remaintime/IN_MILLISECONDS <= int32(timediff))
                    continue;

                remaintime -= timediff*IN_MILLISECONDS;
            }

            // prevent wrong values of remaincharges
            if (spellInfo->GetAuraOptions(GetSpawnMode())->ProcCharges)
            {
                if (remaincharges <= 0 || remaincharges > spellInfo->GetAuraOptions(GetSpawnMode())->ProcCharges)
                    remaincharges = spellInfo->GetAuraOptions(GetSpawnMode())->ProcCharges;
            }
            else
                remaincharges = 0;

            for (auto& itr : auraEffectList)
            {
                if(itr._slot == slot)
                {
                    damage[itr._effect] = itr._amount;
                    baseDamage[itr._effect] = itr._baseamount;
                }
            }

            Aura* aura = Aura::TryCreate(spellInfo, effmask, this, nullptr, &baseDamage[0], nullptr, caster_guid);
            if (aura != nullptr)
            {
                if (!aura->CanBeSaved())
                {
                    aura->Remove();
                    continue;
                }
                aura->SetLoadedState(maxduration, remaintime, remaincharges, stackcount, recalculatemask, &damage[0]);
                aura->ApplyForTargets();
                TC_LOG_DEBUG(LOG_FILTER_PETS, "Added aura spellid %u, effectmask %u", spellInfo->Id, effmask);
            }
        }
        while (result->NextRow());
    }
}

void Pet::_SaveAuras(SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS_EFFECTS);
    stmt->setUInt32(0, m_charmInfo->GetPetNumber());
    trans->Append(stmt);

    for (AuraMap::const_iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end(); ++itr)
    {
        // check if the aura has to be saved
        if (!itr->second->CanBeSaved() || IsPetAura(itr->second))
            continue;

        Aura* aura = itr->second;
        AuraApplication * foundAura = GetAuraApplication(aura->GetId(), aura->GetCasterGUID(), aura->GetCastItemGUID());

        if(!foundAura)
            continue;

        uint32 effMask = 0;
        uint32 recalculateMask = 0;
        uint8 index = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetEffect(i))
            {
                index = 0;
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_AURA_EFFECT);
                stmt->setUInt32(index++, m_charmInfo->GetPetNumber());
                stmt->setUInt8(index++, foundAura->GetSlot());
                stmt->setUInt8(index++, i);
                stmt->setInt32(index++, aura->GetEffect(i)->GetBaseAmount());
                stmt->setInt32(index++, aura->GetEffect(i)->GetAmount());

                trans->Append(stmt);

                effMask |= (1<<i);
                if (aura->GetEffect(i)->CanBeRecalculated())
                    recalculateMask |= (1<<i);
            }
        }

        // don't save guid of caster in case we are caster of the spell - guid for pet is generated every pet load, so it won't match saved guid anyways
        ObjectGuid casterGUID = (itr->second->GetCasterGUID() == GetGUID()) ? ObjectGuid::Empty : itr->second->GetCasterGUID();

        index = 0;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PET_AURA);
        stmt->setUInt32(index++, m_charmInfo->GetPetNumber());
        stmt->setUInt8(index++, foundAura->GetSlot());
        stmt->setBinary(index++, casterGUID.GetRawValue());
        stmt->setUInt32(index++, itr->second->GetId());
        stmt->setUInt8(index++, effMask);
        stmt->setUInt8(index++, recalculateMask);
        stmt->setUInt8(index++, itr->second->GetStackAmount());
        stmt->setInt32(index++, itr->second->GetMaxDuration());
        stmt->setInt32(index++, itr->second->GetDuration());
        stmt->setUInt8(index++, itr->second->GetCharges());

        trans->Append(stmt);
    }
}

bool TempSummon::addSpell(uint32 spellId, ActiveStates active /*= ACT_DECIDE*/, PetSpellState state /*= PETSPELL_NEW*/, PetSpellType type /*= PETSPELL_NORMAL*/)
{
    //TC_LOG_ERROR(LOG_FILTER_PETS, "TempSummon::addSpell spellId %i Entry %i", spellId, GetEntry());

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        // do pet spell book cleanup
        if (state == PETSPELL_UNCHANGED)                    // spell load case
        {
            TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::addSpell: Non-existed in SpellStore spell #%u request, deleting for all pets in `pet_spell`.", spellId);
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_PET_SPELL);
            stmt->setUInt32(0, spellId);
            CharacterDatabase.Execute(stmt);
        }
        else
            TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::addSpell: Non-existed in SpellStore spell #%u request.", spellId);

        return false;
    }

    auto itr = m_spells.find(spellId);
    if (itr != m_spells.end())
    {
        if (itr->second.state == PETSPELL_REMOVED)
        {
            m_spells.erase(itr);
            state = PETSPELL_CHANGED;
        }
        else if (state == PETSPELL_UNCHANGED && itr->second.state != PETSPELL_UNCHANGED)
        {
            // can be in case spell loading but learned at some previous spell loading
            itr->second.state = PETSPELL_UNCHANGED;

            if (active == ACT_ENABLED)
                ToggleAutocast(spellInfo, true);
            else if (active == ACT_DISABLED)
                ToggleAutocast(spellInfo, false);

            return false;
        }
        else
            return false;
    }

    PetSpell newspell;
    newspell.state = state;
    newspell.type = type;

    if (active == ACT_DECIDE)                               // active was not used before, so we save it's autocast/passive state here
    {
        SpellInfo const* spellInfoDur = sSpellMgr->GetSpellInfo(GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL));
        if(spellInfoDur && spellInfoDur->GetDuration(GetSpawnMode()) > 0 && spellInfo->GetMaxRange(false))
            newspell.active = ACT_ENABLED;
        else if (spellInfo->IsAutocastable())
        {
            if (isPet())
                newspell.active = ACT_DISABLED;
            else
                newspell.active = ACT_ENABLED;
        }
        else
            newspell.active = ACT_PASSIVE;
    }
    else
        newspell.active = active;

    m_spells[spellId] = newspell;

    if(m_charmInfo)
    {
        if (spellInfo->IsPassive() && (!spellInfo->AuraRestrictions.CasterAuraState || HasAuraState(AuraStateType(spellInfo->AuraRestrictions.CasterAuraState))))
            CastSpell(this, spellId, true);
        else
            m_charmInfo->AddSpellToActionBar(spellInfo);
    }
    else
    {
        if (spellInfo->IsPassive() && (!spellInfo->AuraRestrictions.CasterAuraState || HasAuraState(AuraStateType(spellInfo->AuraRestrictions.CasterAuraState))))
            CastSpell(this, spellId, true);
        else
            AddPetCastSpell(spellId);

        if(GetCasterPet() && spellInfo->GetMaxRange(false) > GetAttackDist() && (spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_DIRECT_DAMAGE) && !spellInfo->IsTargetingAreaCast())
            SetAttackDist(spellInfo->GetMaxRange(false));

        //TC_LOG_DEBUG(LOG_FILTER_PETS, "TempSummon::addSpell guard GetMaxRange %f GetAttackDist %f GetCasterPet %i", spellInfo->GetMaxRange(false), GetAttackDist(), GetCasterPet());

        return false; //No info in spell for guard pet
    }

    if (newspell.active == ACT_ENABLED)
        ToggleAutocast(spellInfo, true);

    if(GetCasterPet() && spellInfo->GetMaxRange(false) > GetAttackDist() && spellInfo->IsAutocastable() && (spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_DIRECT_DAMAGE) && !spellInfo->IsTargetingAreaCast())
        SetAttackDist(spellInfo->GetMaxRange(false));

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "TempSummon::addSpell pet GetMaxRange %f, active %i GetAttackDist %f GetCasterPet %i", spellInfo->GetMaxRange(false), newspell.active, GetAttackDist(), GetCasterPet());

    return true;
}

bool TempSummon::learnSpell(uint32 spellID)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    if (!spellInfo)
        return false;

    if (!addSpell(spellID))
        return false;

    if (!m_loading && m_owner->ToPlayer() && !spellInfo->HasAttribute(SPELL_ATTR0_HIDDEN_CLIENTSIDE) && !spellInfo->HasAttribute(SPELL_ATTR4_HIDDEN_SPELLBOOK))
    {
        m_owner->ToPlayer()->SendDirectMessage(WorldPackets::PetPackets::LearnedRemovedSpells(SMSG_PET_LEARNED_SPELLS, { spellID }).Write());
        m_owner->ToPlayer()->PetSpellInitialize();
    }

    return true;
}

void TempSummon::InitLevelupSpellsForLevel()
{
    uint8 level = getLevel();

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "TempSummon::InitLevelupSpellsForLevel level %i", level);

    if(m_charmInfo)
    {
        if (PetLevelupSpellSet const* levelupSpells = GetCreatureTemplate()->Family ? sSpellMgr->GetPetLevelupSpellList(GetCreatureTemplate()->Family) : nullptr)
        {
            // PetLevelupSpellSet ordered by levels, process in reversed order
            for (auto itr = levelupSpells->rbegin(); itr != levelupSpells->rend(); ++itr)
            {
                // will called first if level down
                if (itr->first > level)
                    unlearnSpell(itr->second);                 // will learn prev rank if any
                // will called if level up
                else
                    learnSpell(itr->second);                        // will unlearn prev rank if any
            }
        }

        int32 petSpellsId = GetCreatureTemplate()->PetSpellDataId ? -static_cast<int32>(GetCreatureTemplate()->PetSpellDataId) : GetEntry();

        // default spells (can be not learned if pet level (as owner level decrease result for example) less first possible in normal game)
        if (PetDefaultSpellsEntry const* defSpells = sSpellMgr->GetPetDefaultSpellsEntry(petSpellsId))
        {
            for (auto i : defSpells->spellid)
            {
                SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(i);
                if (!spellEntry)
                    continue;

                // will called first if level down
                if (spellEntry->SpellLevel > level)
                    unlearnSpell(spellEntry->Id);
                // will called if level up
                else
                    learnSpell(spellEntry->Id);
            }
        }
    }
    else
    {
        for (auto spellID : m_templateSpells)
        {
            if (!spellID)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
            if (!spellInfo)
                continue;

            learnSpell(spellInfo->Id);
        }
    }
}

bool TempSummon::unlearnSpell(uint32 spellID)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    if (!spellInfo)
        return false;

    if (removeSpell(spellID))
    {
        if (!m_loading && m_owner->ToPlayer() && !spellInfo->HasAttribute(SPELL_ATTR0_HIDDEN_CLIENTSIDE) && !spellInfo->HasAttribute(SPELL_ATTR4_HIDDEN_SPELLBOOK))
            if (Player* player = m_owner->ToPlayer())
                player->SendDirectMessage(WorldPackets::PetPackets::LearnedRemovedSpells(SMSG_PET_UNLEARNED_SPELLS, { spellID }).Write());

        return true;
    }

    return false;
}

bool TempSummon::removeSpell(uint32 spell_id)
{
    auto itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return false;

    if (itr->second.state == PETSPELL_REMOVED)
        return false;

    if (itr->second.state == PETSPELL_NEW)
        m_spells.erase(itr);
    else
        itr->second.state = PETSPELL_REMOVED;

    RemoveAurasDueToSpell(spell_id);

    // if remove last rank or non-ranked then update action bar at server and client if need
    if (m_charmInfo && m_charmInfo->RemoveSpellFromActionBar(spell_id))
    {
        if (!m_loading)
        {
            // need update action bar for last removed rank
            if (Unit* owner = GetOwner())
                if (owner->IsPlayer())
                    owner->ToPlayer()->PetSpellInitialize();
        }
    }

    return true;
}

void Pet::CleanupActionBar()
{
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        if (UnitActionBarEntry const* ab = m_charmInfo->GetActionBarEntry(i))
            if (ab->GetAction() && ab->IsActionBarForSpell())
            {
                if (!HasSpell(ab->GetAction()))
                    m_charmInfo->SetActionBar(i, 0, ACT_PASSIVE);
                else if (ab->GetType() == ACT_ENABLED)
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ab->GetAction()))
                        ToggleAutocast(spellInfo, true);
                }
            }
}

void Pet::InitPetCreateSpells()
{
    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Pet InitPetCreateSpells");
    m_charmInfo->InitPetActionBar();
    m_spells.clear();

    LearnPetPassives();
    InitLevelupSpellsForLevel();
}

bool Pet::IsPermanentPetFor(Player* owner)
{
    switch (getPetType())
    {
        case SUMMON_PET:
            switch (owner->getClass())
            {
                case CLASS_WARLOCK:
                    return GetCreatureTemplate()->Type == CREATURE_TYPE_DEMON;
                case CLASS_DEATH_KNIGHT:
                    return GetCreatureTemplate()->Type == CREATURE_TYPE_UNDEAD;
                case CLASS_MAGE:
                    return GetCreatureTemplate()->Type == CREATURE_TYPE_ELEMENTAL;
                default:
                    return false;
            }
        case HUNTER_PET:
            return true;
        default:
            return false;
    }
}

bool Pet::Create(ObjectGuid::LowType const& guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 pet_number)
{
    ASSERT(map);
    SetMap(map);

    SetPhaseMask(phaseMask, false);
    Object::_Create(ObjectGuid::Create<HighGuid::Pet>(map->GetId(), Entry, guidlow));

    m_movementInfo.Guid = GetGUID();

    m_DBTableGuid = guidlow;
    m_originalEntry = Entry;

    if (!InitEntry(Entry))
        return false;

    SetSheath(SHEATH_STATE_MELEE);

    return true;
}

bool Pet::HasSpell(uint32 spell)
{
    PetSpellMap::const_iterator itr = m_spells.find(spell);
    return itr != m_spells.end() && itr->second.state != PETSPELL_REMOVED;
}

void TempSummon::LearnPetPassives()
{
    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return;

    // sCreatureFamilyStore.AssertEntry(cInfo->Family);

    DB2Manager::PetFamilySpellsSet const* spells = sDB2Manager.GetPetFamilySpells(cInfo->Family);
    if (!spells)
        return;

    for (auto spell : *spells)
        addSpell(spell, ACT_DECIDE, PETSPELL_NEW, PETSPELL_FAMILY);
}

void TempSummon::CastPetAuras(bool apply, uint32 spellId)
{
    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Pet::CastPetAuras guid %u, apply %u, GetEntry() %u IsInWorld %u", GetGUIDLow(), apply, GetEntry(), IsInWorld());

    Unit* owner = GetAnyOwner();
    if (!owner || !owner->ToPlayer() || !owner->ToPlayer()->GetSession() || owner->ToPlayer()->GetSession()->PlayerLogout())
        return;

    uint32 createdSpellId = GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL);

    if (std::vector<PetAura> const* petSpell = sSpellMgr->GetPetAura(GetEntry()))
    {
        for (const auto& itr : *petSpell)
        {
            Unit* _target = this;
            Unit* _caster = this;
            Unit* _targetaura = this;

            if(itr.target == 1 || itr.target == 4) //get target owner
                _target = owner;

            if(itr.target == 2 || itr.target == 4) //set caster owner
                _caster = owner;

            if(itr.target == 3) //get target from spell chain
                _target = _target->GetTargetUnit();

            if(itr.target == 5) //get target from spell chain
                _target = owner->getVictim();

            if(itr.target == 6 && owner->ToPlayer()) //get target from spell chain
                _target = owner->ToPlayer()->GetSelectedUnit();

            if(itr.targetaura == 1) //get target for aura owner
                _targetaura = owner;

            if(_target == nullptr)
                _target = this;
            if(_caster == nullptr)
                _caster = this;

            if(itr.aura > 0 && !_targetaura->HasAura(itr.aura))
                continue;
            if(itr.aura < 0 && _targetaura->HasAura(abs(itr.aura)))
                continue;
            if(itr.casteraura > 0 && !_caster->HasAura(itr.casteraura))
                continue;
            if(itr.casteraura < 0 && _caster->HasAura(abs(itr.casteraura)))
                continue;
            if(spellId != 0 && spellId != abs(itr.fromspell))
                continue;

            if (itr.createdspell != 0)
            {
                if (itr.createdspell > 0)
                {
                    if (itr.createdspell != createdSpellId)
                        continue;
                }
                else
                {
                    if (std::abs(itr.createdspell) == createdSpellId)
                        continue;
                }
            }

            float bp0 = itr.bp0;
            float bp1 = itr.bp1;
            float bp2 = itr.bp2;

            // TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::CastPetAuras PetAura bp0 %i, bp1 %i, bp2 %i, target %i spellId %i option %u", bp0, bp1, bp2, itr->target, itr->spellId, itr->option);

            if(itr.spellId > 0)
            {
                if (!apply)
                {
                    switch (itr.option)
                    {
                        case 4: //learn spell
                        {
                            if(Player* _lplayer = _target->ToPlayer())
                                _lplayer->removeSpell(itr.spellId);
                            else
                                removeSpell(itr.spellId);
                            break;
                        }
                        default:
                            _target->RemoveAurasDueToSpell(itr.spellId);
                            break;
                    }
                }
                else
                {
                    switch (itr.option)
                    {
                        case 0: //cast spell without option
                            _caster->CastSpell(_target, itr.spellId, true);
                            break;
                        case 1: //cast custom spell option
                            _caster->CastCustomSpell(_target, itr.spellId, &bp0, &bp1, &bp2, true);
                            break;
                        case 2: //add aura
                            if (Aura* aura = _caster->AddAura(itr.spellId, _target))
                            {
                                if (bp0 && aura->GetEffect(0))
                                    aura->GetEffect(0)->SetAmount(bp0);
                                if (bp1 && aura->GetEffect(1))
                                    aura->GetEffect(1)->SetAmount(bp1);
                                if (bp2 && aura->GetEffect(2))
                                    aura->GetEffect(2)->SetAmount(bp2);
                            }
                            break;
                        case 3: //cast spell not triggered
                            _caster->CastSpell(_target, itr.spellId, false);
                            break;
                        case 4: //learn spell
                        {
                            if (Player* _lplayer = _target->ToPlayer())
                                _lplayer->learnSpell(itr.spellId, false);
                            else
                                learnSpell(itr.spellId);
                            break;
                        }
                        case 5: // remove minion
                        {
                            if (GuidList* summonList = _target->GetSummonList(bp0))
                                for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                                    if(Creature* summon = ObjectAccessor::GetCreature(*this, (*iter)))
                                        summon->DespawnOrUnsummon(1000);
                            break;
                        }
                        case 6: // Delay cast
                        {
                            ObjectGuid targetGUID = _target->GetGUID();
                            uint32 _spellId = abs(itr.spellId);
                            _caster->AddDelayedEvent(bp0, [_caster, _spellId, targetGUID]() -> void
                            {
                                if (_caster)
                                    return;

                                if (Unit* target = ObjectAccessor::GetUnit(*_caster, targetGUID))
                                    _caster->CastSpell(target, _spellId, true);
                            });
                            break;
                        }
                        case 7: // Move to position
                        {
                            GetMotionMaster()->MovePoint(0, *_target);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            else
            {
                if (apply)
                {
                    switch (itr.option)
                    {
                        case 4: //learn spell
                        {
                            if(Player* _lplayer = _target->ToPlayer())
                                _lplayer->removeSpell(abs(itr.spellId));
                            else
                                removeSpell(abs(itr.spellId));
                            break;
                        }
                        default:
                            _target->RemoveAurasDueToSpell(abs(itr.spellId));
                            break;
                    }
                }
                else
                {
                    switch (itr.option)
                    {
                        case 0: //cast spell without option
                            _caster->CastSpell(_target, abs(itr.spellId), true);
                            break;
                        case 1: //cast custom spell option
                            _caster->CastCustomSpell(_target, abs(itr.spellId), &bp0, &bp1, &bp2, true);
                            break;
                        case 2: //add aura
                            if(Aura* aura = _caster->AddAura(abs(itr.spellId), _target))
                            {
                                if(bp0 && aura->GetEffect(0))
                                    aura->GetEffect(0)->SetAmount(bp0);
                                if(bp1 && aura->GetEffect(1))
                                    aura->GetEffect(1)->SetAmount(bp1);
                                if(bp2 && aura->GetEffect(2))
                                    aura->GetEffect(2)->SetAmount(bp2);
                            }
                            break;
                        case 3: //cast spell not triggered
                            _caster->CastSpell(_target, abs(itr.spellId), false);
                            break;
                        case 4: //learn spell
                        {
                            if(Player* _lplayer = _target->ToPlayer())
                                _lplayer->learnSpell(abs(itr.spellId), false);
                            else
                                learnSpell(abs(itr.spellId));
                            break;
                        }
                        case 5: // remove minion
                        {
                            if (GuidList* summonList = _target->GetSummonList(bp0))
                                for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                                    if(Creature* summon = ObjectAccessor::GetCreature(*this, (*iter)))
                                        summon->DespawnOrUnsummon(1000);
                            break;
                        }
                        case 6: // Delay cast
                        {
                            ObjectGuid targetGUID = _target->GetGUID();
                            uint32 _spellId = abs(itr.spellId);
                            _caster->AddDelayedEvent(bp0, [_caster, _spellId, targetGUID]() -> void
                            {
                                if (_caster)
                                    return;

                                if (Unit* target = ObjectAccessor::GetUnit(*_caster, targetGUID))
                                    _caster->CastSpell(target, _spellId, true);
                            });
                            break;
                        }
                        case 7: // Move to position
                        {
                            GetMotionMaster()->MovePoint(0, *_target);
                            break;
                        }
                        case 8: // owner is original caster
                        {
                            if (owner)
                                _caster->CastSpell(_target, abs(itr.spellId), true, nullptr, nullptr, owner->GetGUID());
                        }
                        default:
                            break;
                    }
                }
            }
        }
    }

    //for all hunters pets
    if(createdSpellId == 13481 || createdSpellId == 83245 || createdSpellId == 83244 || createdSpellId == 83243 || createdSpellId == 83242 || createdSpellId == 883)
    if (std::vector<PetAura> const* petSpell = sSpellMgr->GetPetAura(-1))
    {
        for (const auto& itr : *petSpell)
        {
            Unit* _target = this;
            Unit* _caster = this;
            Unit* _targetaura = this;

            //TC_LOG_DEBUG(LOG_FILTER_PETS, "CastPetAuras spellId %i", itr->spellId);

            if(itr.target == 1 || itr.target == 4) //get target owner
                _target = owner;

            if(itr.target == 2 || itr.target == 4) //set caster owner
                _caster = owner;

            if(itr.target == 3) //get target from spell chain
                _target = _target->GetTargetUnit();

            if(itr.target == 5) //get target from spell chain
                _target = owner->getVictim();

            if(itr.target == 6 && owner->ToPlayer()) //get target from spell chain
                _target = owner->ToPlayer()->GetSelectedUnit();

            if(itr.targetaura == 1) //get target for aura owner
                _targetaura = owner;

            if(_target == nullptr)
                _target = this;
            if(_caster == nullptr)
                _caster = this;

            if(itr.aura > 0 && !_targetaura->HasAura(itr.aura))
                continue;
            if(itr.aura < 0 && _targetaura->HasAura(abs(itr.aura)))
                continue;
            if(itr.casteraura > 0 && !_caster->HasAura(itr.casteraura))
                continue;
            if(itr.casteraura < 0 && _caster->HasAura(abs(itr.casteraura)))
                continue;
            if(itr.createdspell != 0 && itr.createdspell != createdSpellId)
                continue;

            float bp0 = itr.bp0;
            float bp1 = itr.bp1;
            float bp2 = itr.bp2;

            //TC_LOG_DEBUG(LOG_FILTER_PETS, "Pet::CastPetAuras PetAura bp0 %i, bp1 %i, bp2 %i, target %i", bp0, bp1, bp2, itr->target);

            if(itr.spellId > 0)
            {
                if (!apply)
                {
                    switch (itr.option)
                    {
                        case 4: //learn spell
                        {
                            if(Player* _lplayer = _target->ToPlayer())
                                _lplayer->removeSpell(itr.spellId);
                            else
                                removeSpell(itr.spellId);
                            break;
                        }
                        default:
                            _target->RemoveAurasDueToSpell(itr.spellId);
                            break;
                    }
                    continue;
                }
                if(spellId != 0 && spellId != abs(itr.fromspell))
                    continue;
                switch (itr.option)
                {
                    case 0: //cast spell without option
                        _caster->CastSpell(_target, itr.spellId, true);
                        break;
                    case 1: //cast custom spell option
                        _caster->CastCustomSpell(_target, itr.spellId, &bp0, &bp1, &bp2, true);
                        break;
                    case 2: //add aura
                        _caster->AddAura(itr.spellId, _target);
                        break;
                    case 3: //cast spell not triggered
                        _caster->CastSpell(_target, itr.spellId, false);
                        break;
                    case 4: //learn spell
                    {
                        if(Player* _lplayer = _target->ToPlayer())
                            _lplayer->learnSpell(itr.spellId, false);
                        else
                            learnSpell(itr.spellId);
                        break;
                    }
                    case 6: // Delay cast
                    {
                        ObjectGuid targetGUID = _target->GetGUID();
                        uint32 _spellId = abs(itr.spellId);
                        _caster->AddDelayedEvent(bp0, [_caster, _spellId, targetGUID]() -> void
                        {
                            if (_caster)
                                return;

                            if (Unit* target = ObjectAccessor::GetUnit(*_caster, targetGUID))
                                _caster->CastSpell(target, _spellId, true);
                        });
                        break;
                    }
                    case 7: // Move to position
                    {
                        GetMotionMaster()->MovePoint(0, *_target);
                        break;
                    }
                    default:
                        break;
                }
            }
            else
            {
                if (apply)
                {
                    switch (itr.option)
                    {
                        case 4: //learn spell
                        {
                            if(Player* _lplayer = _target->ToPlayer())
                                _lplayer->removeSpell(abs(itr.spellId));
                            else
                                removeSpell(abs(itr.spellId));
                            break;
                        }
                        default:
                            _target->RemoveAurasDueToSpell(abs(itr.spellId));
                            break;
                    }
                    continue;
                }
                if(spellId != 0 && spellId != abs(itr.fromspell))
                    continue;
                switch (itr.option)
                {
                    case 0: //cast spell without option
                        _caster->CastSpell(_target, abs(itr.spellId), true);
                        break;
                    case 1: //cast custom spell option
                        _caster->CastCustomSpell(_target, abs(itr.spellId), &bp0, &bp1, &bp2, true);
                        break;
                    case 2: //add aura
                        _caster->AddAura(abs(itr.spellId), _target);
                        break;
                    case 3: //cast spell not triggered
                        _caster->CastSpell(_target, abs(itr.spellId), false);
                        break;
                    case 4: //learn spell
                    {
                        if(Player* _lplayer = _target->ToPlayer())
                            _lplayer->learnSpell(abs(itr.spellId), false);
                        else
                            learnSpell(abs(itr.spellId));
                        break;
                    }
                    case 6: // Delay cast
                    {
                        ObjectGuid targetGUID = _target->GetGUID();
                        uint32 _spellId = abs(itr.spellId);
                        _caster->AddDelayedEvent(bp0, [_caster, _spellId, targetGUID]() -> void
                        {
                            if (_caster)
                                return;

                            if (Unit* target = ObjectAccessor::GetUnit(*_caster, targetGUID))
                                _caster->CastSpell(target, _spellId, true);
                        });
                        break;
                    }
                    case 7: // Move to position
                    {
                        GetMotionMaster()->MovePoint(0, *_target);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

bool Pet::IsPetAura(Aura const* aura)
{
    // if the owner has that pet aura, return true
    // pet auras
    if (std::vector<PetAura> const* petSpell = sSpellMgr->GetPetAura(GetEntry()))
        for (const auto& itr : *petSpell)
            if (itr.spellId == aura->GetId())
                return true;

    return false;
}

void Pet::SynchronizeLevelWithOwner()
{
    Unit* owner = GetOwner();
    if (!owner || !owner->IsPlayer())
        return;

    GivePetLevel(owner->GetEffectiveLevel());
}

void Pet::LearnSpecializationSpell()
{
    for (SpecializationSpellsEntry const* specializationEntry : sSpecializationSpellsStore)
        if (specializationEntry->SpecID == GetSpecializationId())
            learnSpell(specializationEntry->SpellID);
}

void Pet::UnlearnSpecializationSpell()
{
    for (SpecializationSpellsEntry const* specializationEntry : sSpecializationSpellsStore)
    {
        if (specializationEntry->SpecID != GetSpecializationId())
            continue;

        unlearnSpell(specializationEntry->SpellID);
    }
}

void Pet::CheckSpecialization()
{
    Unit* owner = GetOwner();
    if (!owner)
        return;

    Player* player = owner->ToPlayer();
    if (!player)
        return;

    bool overrideSpec = false;
    if (HasAuraType(SPELL_AURA_OVERRIDE_PET_SPECS))
        overrideSpec = true;
    uint32 newSpecId = 0;

    switch(m_specialization)
    {
        case SPEC_PET_FEROCITY:
            if (overrideSpec)
                newSpecId = SPEC_PET_ADAPTATION_FEROCITY;
            break;
        case SPEC_PET_TENACITY:
            if (overrideSpec)
                newSpecId = SPEC_PET_ADAPTATION_TENACITY;
            break;
        case SPEC_PET_CUNNING:
            if (overrideSpec)
                newSpecId = SPEC_PET_ADAPTATION_CUNNING;
            break;
        case SPEC_PET_ADAPTATION_FEROCITY:
            if (!overrideSpec)
                newSpecId = SPEC_PET_FEROCITY;
            break;
        case SPEC_PET_ADAPTATION_TENACITY:
            if (!overrideSpec)
                newSpecId = SPEC_PET_TENACITY;
            break;
        case SPEC_PET_ADAPTATION_CUNNING:
            if (!overrideSpec)
                newSpecId = SPEC_PET_CUNNING;
            break;
        default:
            break;
    }

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Pet::CheckSpecialization newSpecId %i m_specialization %i overrideSpec %i", newSpecId, m_specialization, overrideSpec);

    if (newSpecId)
    {
        UnlearnSpecializationSpell();
        SetSpecialization(newSpecId);
        LearnSpecializationSpell();
        player->PetSpellInitialize();
        player->SendTalentsInfoData(true);
    }
}

void Pet::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    time_t curTime = time(nullptr);

    WorldPackets::Spells::SpellCooldown cooldowns;
    cooldowns.Caster = GetGUID();
    cooldowns.Flags = 0;

    for (PetSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second.state == PETSPELL_REMOVED)
            continue;

        uint32 unSpellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(unSpellId);
        if (!spellInfo)
            continue;

        // Not send cooldown for this spells
        if (spellInfo->HasAttribute(SPELL_ATTR0_DISABLED_WHILE_ACTIVE))
            continue;

        if (!(spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE))
            continue;

        if ((idSchoolMask & spellInfo->GetSchoolMask()) && _GetSpellCooldownDelay(unSpellId) < unTimeMs)
        {
            cooldowns.SpellCooldowns.emplace_back(unSpellId, unTimeMs);
            _AddCreatureSpellCooldown(unSpellId, curTime + unTimeMs/IN_MILLISECONDS);
        }
    }

    if (GetOwner())
        if (auto owner = GetOwner()->ToPlayer())
            owner->SendDirectMessage(cooldowns.Write());
}

void Pet::SetGroupUpdateFlag(uint32 flag)
{
    return;

    Player* player = GetOwner()->ToPlayer();
    if (!player)
        return;

    if (player->GetGroup())
    {
        m_groupUpdateMask |= flag;
        player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET);
    }
}

void Pet::ResetGroupUpdateFlag()
{
    return;
    m_groupUpdateMask = GROUP_UPDATE_FLAG_PET_NONE;

    if (Player* player = GetOwner()->ToPlayer())
        player->RemoveGroupUpdateFlag(GROUP_UPDATE_FLAG_PET);
}

bool Pet::isControlled() const
{
    return bool(getPetType() == SUMMON_PET || getPetType() == HUNTER_PET);
}

bool Pet::isTemporarySummoned() const
{
    return m_duration > 0;
}
