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

#include "BattlegroundMgr.h"
#include "CellImpl.h"
#include "CombatPackets.h"
#include "Common.h"
#include "Chat.h"
#include "Creature.h"
#include <utility>
#include "CreatureAI.h"
#include "CreatureAISelector.h"
#include "CreatureGroups.h"
#include "CreatureTextMgr.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "GameEventMgr.h"
#include "GossipDef.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "PoolMgr.h"
#include "QuestData.h"
#include "QuestDef.h"
#include "ScenarioMgr.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "Util.h"
#include "Vehicle.h"
#include "WaypointMovementGenerator.h"
#include "WildBattlePet.h"
#include "World.h"
#include "WorldPacket.h"

#define ZONE_UPDATE_INTERVAL (10*IN_MILLISECONDS)

bool TrainerSpell::IsCastable() const
{
    return learnedSpell[0] != spell;
}

TrainerSpell const* TrainerSpellData::Find(uint32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(spellList, spell_id);
}

VendorItem::VendorItem(int32 _item, int32 _maxcount, uint32 _incrtime, uint32 _ExtendedCost, uint8 _Type, uint64 _Money, uint32 _randomPropertiesSeed, ItemRandomEnchantmentId _randomPropertiesID, std::vector<uint32> _bonusListIDs, std::vector<int32> _itemModifiers, bool _DoNotFilterOnVendor, uint8 _context, uint32 _PlayerConditionID, int32 _DonateStoreId, uint32 _DonateCost) : item(_item), maxcount(_maxcount), incrtime(_incrtime), PlayerConditionID(_PlayerConditionID), ExtendedCost(_ExtendedCost), Type(_Type),
Money(_Money), RandomPropertiesSeed(_randomPropertiesSeed), RandomPropertiesID(_randomPropertiesID), BonusListIDs(std::move(_bonusListIDs)), ItemModifiers(std::move(_itemModifiers)), DoNotFilterOnVendor(_DoNotFilterOnVendor), Context(_context), DonateStoreId(_DonateStoreId), DonateCost(_DonateCost)
{
}

bool VendorItem::IsGoldRequired(ItemTemplate const* pProto) const
{
    return pProto->GetFlags2() & ITEM_FLAG2_DONT_IGNORE_BUY_PRICE || !ExtendedCost || ExtendedCost >= 15000;
}

VendorItem* VendorItemData::GetItem(uint32 slot) const
{
    if (slot >= m_items.size())
        return nullptr;
    return m_items[slot];
}

bool VendorItemData::RemoveItem(uint32 itemId, uint8 type)
{
    auto newEnd = std::remove_if(m_items.begin(), m_items.end(), [=](VendorItem const* vendorItem)
    {
        return vendorItem && vendorItem->item == itemId && vendorItem->Type == type;
    });

    bool found = newEnd != m_items.end();
    m_items.erase(newEnd, m_items.end());
    return found;
}

VendorItem const* VendorItemData::FindItemCostPair(uint32 itemId, uint32 extendedCost, uint8 type) const
{
    for (auto itr : m_items)
        if (itr->item == itemId && itr->ExtendedCost == extendedCost && itr->Type == type)
            return itr;
    return nullptr;
}

TrainerSpell::TrainerSpell(): spell(0), spellCost(0), reqSkill(0), reqSkillValue(0), reqLevel(0)
{
    for (auto& i : learnedSpell)
        i = 0;
}

uint8 CreatureTemplate::GetDiffFromSpawn(uint8 spawnmode)
{
    switch (spawnmode)
    {
        case DIFFICULTY_NONE:
        case DIFFICULTY_NORMAL:
        case DIFFICULTY_10_N:
        case DIFFICULTY_40:
        case DIFFICULTY_N_SCENARIO:
        case DIFFICULTY_NORMAL_RAID:
        case DIFFICULTY_EVENT_RAID:
        case DIFFICULTY_EVENT_DUNGEON:
        case DIFFICULTY_EVENT_SCENARIO:
            return 0;
        case DIFFICULTY_HEROIC:
        case DIFFICULTY_25_N:
        case DIFFICULTY_HC_SCENARIO:
        case DIFFICULTY_HEROIC_RAID:
            return 1;
        case DIFFICULTY_10_HC:
        case DIFFICULTY_MYTHIC_RAID:
        case DIFFICULTY_MYTHIC_DUNGEON:
        case DIFFICULTY_MYTHIC_KEYSTONE: //ToDo move to 
            return 2;
        case DIFFICULTY_25_HC:
        case DIFFICULTY_TIMEWALKING:
            return 3;
        case DIFFICULTY_LFR:
        case DIFFICULTY_LFR_RAID:
            return 4;
        case DIFFICULTY_TIMEWALKING_RAID:
            return 5;
        default:
            break;
    }
    return 0;
}

SkillType CreatureTemplate::GetRequiredLootSkill() const
{
    if (TypeFlags[0] & CREATURE_TYPEFLAGS_HERBLOOT)
        return SKILL_HERBALISM;
    if (TypeFlags[0] & CREATURE_TYPEFLAGS_MININGLOOT)
        return SKILL_MINING;
    if (TypeFlags[0] & CREATURE_TYPEFLAGS_ENGINEERLOOT)
        return SKILL_ENGINEERING;
    return SKILL_SKINNING;
    // normal case
}

uint32 HacksMorphesOne[31]
{
    35392,
    35913,
    34776,
    31666,
    31912,
    37067,
    35047,
    36982,
    35693,
    36970,
    36625,
    33775,
    36991,
    21874,
    36477,
    33624,
    34107,
    35241,
    36252,
    32303,
    36586,
    28111,
    35698,
    36277,
    30088,
    16236,
    25437,
    35035,
    34737,
    21143,
    25407
};

uint32 HacksMorphesTwo[31]
{
    33775,
    35215,
    17420,
    28800,
    36626,
    33609,
    37091,
    34107,
    31947,
    35241,
    21143,
    36970,
    31912,
    35392,
    21874,
    37067,
    33387,
    25407,
    25437,
    30088,
    36917,
    31629,
    34737,
    36277,
    34864,
    36991,
    36540,
    37170,
    36352,
    35913,
    34948
};

int32 CreatureTemplate::GetRandomValidModelId() const
{
    uint8 c = 0;
    int32 modelIDs[4];

    if (Modelid[0])
    {
        if (Entry == 119047)
            modelIDs[c++] = HacksMorphesOne[urand(0, 30)];
        else if (Entry == 119046)
            modelIDs[c++] = HacksMorphesTwo[urand(0, 30)];
        else
            modelIDs[c++] = Modelid[0];
    }
    
    if (Modelid[1])
        modelIDs[c++] = Modelid[1];
    if (Modelid[2])
        modelIDs[c++] = Modelid[2];
    if (Modelid[3])
        modelIDs[c++] = Modelid[3];
    
    return ((c>0) ? modelIDs[urand(0, c - 1)] : 0);
}

int32 CreatureTemplate::GetFirstValidModelId() const
{
    for (auto i : Modelid)
        if (i)
            return i;

    return 0;
}

uint32 CreatureTemplate::GetFirstInvisibleModel() const
{
    for (auto i : Modelid)
    {
        auto modelInfo = sObjectMgr->GetCreatureModelInfo(sObjectMgr->GetCreatureDisplay(i));
        if (modelInfo && modelInfo->is_trigger)
            return sObjectMgr->GetCreatureDisplay(i);
    }

    return 11686;
}

uint32 CreatureTemplate::GetFirstVisibleModel() const
{
    for (auto i : Modelid)
    {
        auto modelInfo = sObjectMgr->GetCreatureModelInfo(sObjectMgr->GetCreatureDisplay(i));
        if (modelInfo && !modelInfo->is_trigger)
            return sObjectMgr->GetCreatureDisplay(i);
    }

    return 17519;
}

bool CreatureTemplate::isTameable(Player const* caster) const
{
    if (!caster)
        return false;

    if (Family == 0 || (TypeFlags[0] & CREATURE_TYPEFLAGS_TAMEABLE) == 0)
        return false;

    if (const_cast<Player*>(caster)->HasSpell(205154) || caster->getRace() == RACE_GNOME)
    {
        if (Type != CREATURE_TYPE_BEAST && Type != CREATURE_TYPE_MECHANICAL)
            return false;
    }
    else
        if (Type != CREATURE_TYPE_BEAST)
            return false;

    // if can tame exotic then can tame any temable
    return caster->CanTameExoticPets() || (TypeFlags[0] & CREATURE_TYPEFLAGS_EXOTIC) == 0;
}

uint64 CreatureBaseStats::GenerateHealth(CreatureTemplate const* info, CreatureDifficultyStat const* diffStats) const
{
    if (diffStats)
        return uint64((BaseHealth[info->RequiredExpansion] * diffStats->ModHealth) + 0.5f);
    return uint64((BaseHealth[info->RequiredExpansion] * info->HpMulti) + 0.5f);
}

float CreatureBaseStats::GenerateBaseDamage(CreatureTemplate const* info) const
{
    return BaseDamage[info->RequiredExpansion];
}

uint32 CreatureBaseStats::GenerateMana(CreatureTemplate const* info) const
{
    // Mana can be 0.
    if (!BaseMana)
        return 0;
    return uint32((BaseMana * info->PowerMulti * info->ModManaExtra) + 0.5f);
}

uint32 CreatureBaseStats::GenerateArmor(CreatureTemplate const* info) const
{
    return uint32((BaseArmor * info->ModArmor) + 0.5f);
}

CreatureTemplate::CreatureTemplate(): Entry(0), KillCredit{}, Modelid{}, QuestItem{}, VignetteID(0), FlagQuest(0), VerifiedBuild(0), Classification(0), MovementInfoID(0), Family(0), RequiredExpansion(0), TypeFlags{}, Type(0), PowerMulti(0)
{
    for (auto& i : resistance)
        i = 0;

    for (auto& spell : spells)
        spell = 0;
}

bool AssistDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    if (auto victim = Unit::GetUnit(m_owner, m_victim))
    {
        while (!m_assistants.empty())
        {
            auto assistant = Unit::GetCreature(m_owner, *m_assistants.begin());
            m_assistants.pop_front();

            if (assistant && !assistant->IsAlreadyCallAssistance() && assistant->CanAssistTo(&m_owner, victim))
            {
                //assistant->SetNoCallAssistance(true);
                assistant->CombatStart(victim);
                if (assistant->IsAIEnabled)
                    assistant->AI()->AttackStart(victim);
            }
        }
    }
    return true;
}

bool ForcedDespawnDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.m_despawn = false;
    m_owner.DespawnOrUnsummon(0, m_respawnTimer);    // since we are here, we are not TempSummon as object type cannot change during runtime
    return true;
}

bool SetImuneDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    m_owner.InitializeReactState();
    return true;
}

Creature::Creature(bool isWorldObject) : Unit(isWorldObject), lootForPickPocketed(false), lootForBody(false), CreatureSpells(nullptr), m_groupLootTimer(0), m_PlayerDamageReq(0), m_actionData{}, m_CanCallAssistance(false), m_callAssistanceText(0),
 m_onVehicleAccessory(false), m_corpseRemoveTime(0), m_respawnTime(0), m_respawnChallenge(0), m_respawnDelay(300), m_corpseDelay(60), m_respawnradius(0.0f), m_reactState(REACT_AGGRESSIVE), m_defaultMovementType(IDLE_MOTION_TYPE), m_DBTableGuid(0),
 m_equipmentId(0), m_originalEquipmentId(0), m_AlreadyCallAssistance(false), m_AlreadySearchedAssistance(false), m_regenHealth(true), m_AI_locked(false), m_meleeDamageSchoolMask(SPELL_SCHOOL_MASK_NORMAL), m_originalEntry(0),
 m_creatureInfo(nullptr), m_creatureData(nullptr), m_waypointID(0), m_path_id(0), m_formation(nullptr), outfitId(0)
{
    m_followAngle = PET_FOLLOW_ANGLE;
    m_regenTimer = CREATURE_REGEN_INTERVAL;
    m_valuesCount = UNIT_END;
    _dynamicValuesCount = UNIT_DYNAMIC_END;
    isCasterPet = false;
    m_updateFlag = UPDATEFLAG_LIVING;

    for (auto& templateSpell : m_templateSpells)
        templateSpell = 0;

    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();
    m_CreatureProcCooldowns.clear();
    m_CreatureSchoolCooldowns.clear();
    DisableReputationGain = false;

    m_SightDistance = sWorld->getFloatConfig(CONFIG_SIGHT_MONSTER);
    m_CombatDistance = 0.0f; //MELEE_RANGE;
    m_regenTimerCount = 0;

    m_IfUpdateTimer = 0;
    m_RateUpdateTimer = 40/*MAX_VISIBILITY_DISTANCE*/;
    m_RateUpdateWait = 0;

    TriggerJustRespawned = false;
    m_isTempWorldObject = false;
    bossid = 0;
    m_difficulty = 0;
    m_playerCount = 0;
    m_despawn = false;
    m_isHati = false;
    disableAffix = 0;
    m_respawnCombatDelay = 0;

    m_creatureDiffData = nullptr;

    m_zoneUpdateTimer = 0;
    m_zoneUpdateAllow = false;

    m_sendInterval = 0;
    objectCountInWorld[uint8(HighGuid::Creature)]++;
}

Creature::~Creature()
{
    m_vendorItemCounts.clear();

    delete i_AI;
    i_AI = nullptr;

    //if (m_uint32Values)
    //    TC_LOG_ERROR(LOG_FILTER_UNITS, "Deconstruct Creature entry = %u", GetEntry());
    objectCountInWorld[uint8(HighGuid::Creature)]--;
    if (m_creatureInfo)
        creatureCountInWorld[m_creatureInfo->Entry]--;
    creatureCountInArea[m_areaId]--;
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if (!IsInWorld())
    {
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        SearchFormation();
        if (!m_Teleports)
        {
            if (IsVehicle())
                GetVehicleKit()->Install();
            AIM_Initialize();
        }
        
        if (m_zoneScript)
        {
            m_zoneScript->OnCreatureCreate(this);
            m_zoneScript->OnCreatureCreateForScript(this);
        }

        if (auto bg = GetBattleground())
            bg->OnCreatureCreate(this);

        if (MaxVisible && !isAnySummons())
            if (Map* mapInfo = GetMap())
                mapInfo->AddMaxVisible(this);
    }
}

void Creature::RemoveFromWorld()
{
    if (IsInWorld())
    {
        volatile uint32 creatureEntry = GetEntry();
        Map* mapInfo = GetMap();
        if (mapInfo && !mapInfo->IsMapUnload()) // Don`t delete if map unload, this is autoclear when delete map
        {
            if (m_zoneScript)
            {
                m_zoneScript->OnCreatureRemove(this);
                m_zoneScript->OnCreatureRemoveForScript(this);
            }

            if (auto bg = GetBattleground())
                bg->OnCreatureRemove(this);

            if (MaxVisible && !isAnySummons())
                mapInfo->RemoveMaxVisible(this);

        }
        if (m_formation)
            sFormationMgr->RemoveCreatureFromGroup(m_formation, this);
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

Battleground* Creature::GetBattleground()
{
    auto map = GetMap();
    if (!map)
        return nullptr;

    auto bgMap = map->ToBgMap();
    if (!bgMap)
        return nullptr;

    return bgMap->GetBG();
}

void Creature::PrintCreatureSize(Player* player)
{
    uint32 size = sizeof(Creature);

    uint32 sizeObject = Object::GetSize();
    uint32 sizeUnit = Unit::GetSize();

    ChatHandler(player).PSendSysMessage("Creature Class size %u sizeObject %u sizeUnit %u", size, sizeObject, sizeUnit);

    size += sizeObject;
    size += sizeUnit;

    size += m_CreatureSpellCooldowns.size() * sizeof(CreatureSpellCooldowns);
    size += m_CreatureCategoryCooldowns.size() * sizeof(CreatureSpellCooldowns);
    size += m_CreatureProcCooldowns.size() * sizeof(CreatureSpellCooldowns);
    size += m_CreatureSchoolCooldowns.size() * sizeof(CreatureSpellCooldowns);
    size += rewardedPlayer.size() * sizeof(GuidSet);
    size += lootList.size() * sizeof(GuidList);
    size += m_vendorItemCounts.size() * sizeof(VendorItemCounts);

    ChatHandler(player).PSendSysMessage("Creature All size %u", size);
}

void Creature::SetOutfit(int32 outfit)
{
    outfitId = outfit;
}

int32 Creature::GetOutfit() const
{
    return outfitId;
}

bool Creature::IsMirrorImage() const
{
    return outfitId < 0;
}

void Creature::DisappearAndDie()
{
    DestroyForNearbyPlayers();
    //SetVisibility(VISIBILITY_OFF);
    //ObjectAccessor::UpdateObjectVisibility(this);
    if (isAlive())
        setDeathState(JUST_DIED);
    RemoveCorpse(false);
}

void Creature::SearchFormation()
{
    if (isSummon())
        return;

    ObjectGuid::LowType lowguid = GetDBTableGUIDLow();
    if (!lowguid)
        return;

    auto frmdata = sFormationMgr->CreatureGroupMap.find(lowguid);
    if (frmdata != sFormationMgr->CreatureGroupMap.end())
        sFormationMgr->AddCreatureToGroup(frmdata->second->leaderGUID, this);
}

void Creature::RemoveCorpse(bool setSpawnTime)
{
    if (getDeathState() != CORPSE)
        return;

    m_corpseRemoveTime = time(nullptr);
    setDeathState(DEAD);
    RemoveAllAuras();
    UpdateObjectVisibility();
    ClearLootList();
    ClearSaveThreatTarget();
    uint32 respawnDelay = m_respawnDelay;
    if (IsAIEnabled)
        AI()->CorpseRemoved(respawnDelay);

    // Should get removed later, just keep "compatibility" with scripts
    if (setSpawnTime)
        m_respawnTime = time(nullptr) + respawnDelay;

    float x, y, z, o;
    GetRespawnPosition(x, y, z, &o);
    SetHomePosition(x, y, z, o);
    GetMap()->CreatureRelocation(this, x, y, z, o, true);
}

/**
 * change the entry of creature until respawn
 */
bool Creature::InitEntry(uint32 entry, uint32 /*team*/, const CreatureData* data)
{
    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(entry);
    if (!cinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature::InitEntry creature entry %u does not exist.", entry);
        return false;
    }

    // get difficulty 1 mode entry
    m_difficulty = CreatureTemplate::GetDiffFromSpawn(m_spawnMode);

    SetEntry(entry);                                        // normal entry always
    m_creatureInfo = cinfo;                                 // map mode related always
    m_creatureDiffData = sObjectMgr->GetCreatureDifficultyStat(cinfo->Entry, m_spawnMode); // map mode related always

    // equal to player Race field, but creature does not have race
    SetByteValue(UNIT_FIELD_BYTES_0, 0, 0);

    // known valid are: CLASS_WARRIOR, CLASS_PALADIN, CLASS_ROGUE, CLASS_MAGE
    SetClass(cinfo->unit_class);

    // Cancel load if no model defined
    if (!(cinfo->GetFirstValidModelId()))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (entry: %u) has no model defined in table `creature_template`, can't load. ", entry);
        return false;
    }

    SetOutfit(sObjectMgr->ChooseDisplayId(0, cinfo, data));
    uint32 displayID = sObjectMgr->GetCreatureDisplay(GetOutfit());
    if (IsMirrorImage())
        displayID = 11686; // invisible in beginning if a mirror image
    RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    
    CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelRandomGender(&displayID);
    if (!minfo)                                             // Cancel load if no model defined
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (entry: %u) has no model defined in table `creature_template`, can't load. ", entry);
        return false;
    }

    SetDisplayId(displayID);
    SetNativeDisplayId(displayID);
    SetGender(minfo->gender);

    // Load creature equipment
    if (!data || data->equipmentId == 0)
        LoadEquipment(); // use default equipment (if available)
    else if (data && data->equipmentId != 0)                // override, 0 means no equipment
    {
        m_originalEquipmentId = data->equipmentId;
        LoadEquipment(data->equipmentId);
    }

    SetName((minfo->gender == GENDER_MALE || cinfo->NameAlt[0].empty()) ? cinfo->Name[0] : cinfo->NameAlt[0]);

    //Set default
    SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, 1.0f);
    SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, 1.0f);
    SetFloatValue(UNIT_FIELD_MOD_TIME_RATE, 1.0f);
    SetFloatValue(UNIT_FIELD_MOD_HASTE, 1.0f);
    SetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN, 1.0f);
    SetFloatValue(UNIT_FIELD_MOD_RANGED_HASTE, 1.0f);
    SetFloatValue(UNIT_FIELD_HOVER_HEIGHT, cinfo->HoverHeight);

    SetSpeed(MOVE_WALK,     cinfo->speed_walk);
    SetSpeed(MOVE_RUN,      cinfo->speed_run);
    SetSpeed(MOVE_SWIM,     1.0f); // using 1.0 rate
    SetSpeed(MOVE_FLIGHT,   cinfo->speed_fly);

    if (data && data->personalSize > 0.0f)
        SetObjectScale(data->personalSize);
    else
        SetObjectScale(cinfo->scale);


    // checked at loading
    m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    if (!m_respawnradius && m_defaultMovementType == RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    CreatureSpells = &cinfo->CreatureSpells;
    for (uint8 i=0; i < CREATURE_MAX_SPELLS; ++i)
        m_templateSpells[i] = cinfo->spells[i];

    m_CanCallAssistance = sCreatureTextMgr->HasBroadCastText(entry, BROADCAST_TEXT_FLEE_FOR_ASSIST, m_callAssistanceText); // cinfo->Type == CREATURE_TYPE_HUMANOID && cinfo->Family == 0;

    m_actionData[CREATURE_ACTION_TYPE_ATTACK] = sObjectMgr->GetCreatureActionData(entry);
    m_actionData[CREATURE_ACTION_TYPE_CAST] = sObjectMgr->GetCreatureActionData(entry, CREATURE_ACTION_TYPE_CAST);
    m_actionData[CREATURE_ACTION_TYPE_SUMMON] = sObjectMgr->GetCreatureActionData(entry, CREATURE_ACTION_TYPE_SUMMON);

    return true;
}

bool Creature::UpdateEntry(uint32 entry, uint32 team, const CreatureData* data)
{
    if (!InitEntry(entry, team, data))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    m_regenHealth = cInfo->RegenHealth;

    // creatures always have melee weapon ready if any unless specified otherwise
    if (!GetCreatureAddon())
        SetSheath(SHEATH_STATE_MELEE);

    uint32 SandboxScalingID = 0;
    if (!GetMap()->IsDungeon() || GetMap()->IsCanScale() || cInfo->RequiredExpansion < EXPANSION_LEGION)
    {
        SandboxScalingID = cInfo->SandboxScalingID;

        // Need before set level
        if (cInfo->ScaleLevelMin)
            ScaleLevelMin = cInfo->ScaleLevelMin;
        if (cInfo->ScaleLevelMax)
            ScaleLevelMax = cInfo->ScaleLevelMax;
        else
            SandboxScalingID = GetScalingID();
    }

    // TODO: This should probably be reworked, in which cases should a creature being summoned/charmed/owned
    // mean that level scaling is not applied? Maybe only in case of GetOwner() != NULL?
    // Without this exception the NPCs summoned in quest 40604 (Disturbing the Past) do not get lvl scaled.
    if (entry != 100735)
    {
        if (Unit* owner = GetAnyOwner())
        {
            if (owner->IsPlayer())
            {
                ScaleLevelMin = 0;
                ScaleLevelMax = 0;
            }
        }
    }

    SelectLevel(cInfo);
    setFaction(cInfo->faction);

    uint32 npcflag, npcflag2, unit_flags, unit_flags3, dynamicflags;
    ObjectMgr::ChooseCreatureFlags(cInfo, npcflag, npcflag2, unit_flags, unit_flags3, dynamicflags, data);

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        SetUInt32Value(UNIT_FIELD_NPC_FLAGS, npcflag | sGameEventMgr->GetNPCFlag(this));
    else
        SetUInt32Value(UNIT_FIELD_NPC_FLAGS, npcflag);

    SetUInt32Value(UNIT_FIELD_NPC_FLAGS2, npcflag2);

    SetAttackTime(BASE_ATTACK,  cInfo->baseattacktime);
    SetAttackTime(OFF_ATTACK,   cInfo->baseattacktime);
    SetAttackTime(RANGED_ATTACK, cInfo->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS, unit_flags);
    SetUInt32Value(UNIT_FIELD_FLAGS_2, cInfo->unit_flags2);
    SetUInt32Value(UNIT_FIELD_FLAGS_3, unit_flags3);
	
    if (IsMirrorImage())
        new MirrorImageUpdate(this);

    SetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS, dynamicflags);

    if (ScaleLevelMin)
        SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_MIN, ScaleLevelMin);
    if (ScaleLevelMax)
        SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_MAX, ScaleLevelMax);
    if (cInfo->ScaleLevelDelta)
        SetUInt32Value(UNIT_FIELD_SCALING_LEVEL_DELTA, cInfo->ScaleLevelDelta);
    if (cInfo->ScaleLevelDuration)
        SetUInt32Value(UNIT_FIELD_SCALE_DURATION, cInfo->ScaleLevelDuration);
    if (cInfo->ControllerID)
        SetUInt32Value(UNIT_FIELD_LOOK_AT_CONTROLLER_ID, cInfo->ControllerID);
    if (SandboxScalingID)
        SetUInt32Value(UNIT_FIELD_SANDBOX_SCALING_ID, SandboxScalingID);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    SetMeleeDamageSchool(SpellSchools(cInfo->dmgschool));
    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(GetEffectiveLevel(), cInfo->unit_class);
    auto armor = static_cast<float>(stats->GenerateArmor(cInfo)); // TODO: Why is this treated as uint32 when it's a float?
    SetModifierValue(UNIT_MOD_ARMOR,             BASE_VALUE, armor);
    SetModifierValue(UNIT_MOD_RESISTANCE_HOLY,   BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_HOLY]));
    SetModifierValue(UNIT_MOD_RESISTANCE_FIRE,   BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_FIRE]));
    SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_NATURE]));
    SetModifierValue(UNIT_MOD_RESISTANCE_FROST,  BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_FROST]));
    SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_SHADOW]));
    SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(cInfo->resistance[SPELL_SCHOOL_ARCANE]));

    SetCanModifyStats(true);
    UpdateAllStats();

    if (FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(cInfo->faction))
    {
        if (factionTemplate->Flags & FACTION_TEMPLATE_FLAG_PVP)
            SetPvP(true);
        else
            SetPvP(false);
    }

    // trigger creature is always not selectable and can not be attacked
    if (isTrigger())
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    InitializeReactState();

    if (isDead())
    {
        AddUnitState(UNIT_STATE_DIED);
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        SetReactState(REACT_PASSIVE);
    }

    if (cInfo->flags_extra & CREATURE_FLAG_EXTRA_NO_TAUNT)
    {
        ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
    }

    //! Suspect it works this way:
    //! If creature can walk and fly (usually with pathing)
    //! Set MOVEMENTFLAG_CAN_FLY. Otherwise if it can only fly
    //! Set MOVEMENTFLAG_DISABLE_GRAVITY
    //! The only time I saw Movement Flags: DisableGravity, CanFly, Flying (50332672) on the same unit
    //! it was a vehicle
    if (cInfo->InhabitType & INHABIT_AIR)
    {
        SetCanFly(true);
        SetDisableGravity(true);
    }

    // TODO: Shouldn't we check whether or not the creature is in water first?
    SetSwim(cInfo->InhabitType & INHABIT_WATER && IsInWater());
    return true;
}

void Creature::UpdateStat()
{
    m_difficulty = CreatureTemplate::GetDiffFromSpawn(m_spawnMode);

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    m_creatureDiffData = sObjectMgr->GetCreatureDifficultyStat(cInfo->Entry, m_spawnMode);                                 // map mode related always

    CreatureDifficultyStat const* diffStats = GetCreatureDiffStat();

    // level
    uint8 level = 0;
    if(m_difficulty == 1 || m_difficulty == 2)
        level = cInfo->maxlevel;
    else
        level = cInfo->minlevel;

    SetLevel(level);
    SetEffectiveLevel(0);

    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(level, cInfo->unit_class);

    // health
    float healthmod = _GetHealthMod(cInfo->Classification);
    if (!diffStats)
        healthmod *= _GetHealthModForDiff();

    uint64 basehp = stats->GenerateHealth(cInfo, diffStats);
    auto health = uint64(basehp * healthmod);

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);
    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, static_cast<float>(health));

    UpdateMaxHealth();
    UpdateAttackPowerAndDamage();

    if (m_zoneScript)
        m_zoneScript->OnCreatureUpdateDifficulty(this);
}

void Creature::Update(uint32 diff)
{
    if (m_isUpdate || !IsInWorld() || IsDelete() || IsPreDelete())
        return;

    m_isUpdate = true;

    volatile uint32 creatureEntry = GetEntry();

    if (!isInCombat()) // Update creature if need
    {
        // if (m_RateUpdateWait <= diff)
        // {
            // m_RateUpdateTimer = MAX_VISIBILITY_DISTANCE;
            // m_RateUpdateWait = 15 * IN_MILLISECONDS;
        // }
        // else
            // m_RateUpdateWait -= diff;

        uint32 updateDiff = m_RateUpdateTimer * 5;
        if (updateDiff < sWorld->getIntConfig(CONFIG_INTERVAL_OBJECT_UPDATE))
            updateDiff = sWorld->getIntConfig(CONFIG_INTERVAL_OBJECT_UPDATE);

        m_IfUpdateTimer += diff;
        if (m_IfUpdateTimer > updateDiff)
        {
            diff = m_IfUpdateTimer;
            m_IfUpdateTimer = 0;
        }
        else
        {
            m_isUpdate = false;
            return;
        }
    }

    if (m_zoneUpdateTimer > 0 && m_zoneUpdateAllow)
    {
        if (diff >= m_zoneUpdateTimer)
        {
            m_zoneUpdateAllow = false;

            uint32 newzone, newarea;
            GetZoneAndAreaId(newzone, newarea);

            if (m_zoneId != newzone)
            {
                m_oldZoneId = m_zoneId;
                m_zoneId = newzone;
            }
            if (m_areaId != newarea)
            {
                m_areaId = newarea;
                m_oldAreaId = m_areaId;
            }
            m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;
        }
        else
            m_zoneUpdateTimer -= diff;
    }

    if (IsInWater())
    {
        if (CanSwim())
            AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    }
    else
    {
        if (CanWalk())
            RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    }

    if (IsAIEnabled && TriggerJustRespawned)
    {
        TriggerJustRespawned = false;
        AI()->JustRespawned();
        if (m_vehicleKit)
            m_vehicleKit->Reset();
    }

    switch (m_deathState)
    {
        case JUST_RESPAWNED:
            // Must not be called, see Creature::setDeathState JUST_RESPAWNED -> ALIVE promoting.
            TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature (GUID: %u entry: %u) in wrong state: JUST_RESPAWNED (4)", GetGUIDLow(), GetEntry());
            break;
        case JUST_DIED:
            // Must not be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature (GUID: %u entry: %u) in wrong state: JUST_DEAD (1)", GetGUIDLow(), GetEntry());
            break;
        case DEAD:
        {
            time_t now = time(nullptr);
            if (m_respawnTime <= now)
            {
                bool allowed = IsAIEnabled ? AI()->CanRespawn() : true;     // First check if there are any scripts that object to us respawning
                if (!allowed)                                               // Will be rechecked on next Update call
                    break;

                ObjectGuid dbtableHighGuid = ObjectGuid::Create<HighGuid::Creature>(GetMapId(), GetEntry(), m_DBTableGuid);
                time_t linkedRespawntime = GetMap()->GetLinkedRespawnTime(dbtableHighGuid);
                if (!linkedRespawntime)             // Can respawn
                    Respawn();
                else                                // the master is dead
                {
                    ObjectGuid targetGuid = sObjectMgr->GetLinkedRespawnGuid(dbtableHighGuid);
                    if (targetGuid == dbtableHighGuid) // if linking self, never respawn (check delayed to next day)
                        SetRespawnTime(DAY);
                    else
                        m_respawnTime = (now > linkedRespawntime ? now : linkedRespawntime)+urand(5, MINUTE); // else copy time from master and add a little
                    SaveRespawnTime(); // also save to DB immediately
                }
            }
            break;
        }
        case CORPSE:
        {
            Unit::Update(diff);
            // deathstate changed on spells update, prevent problems
            if (m_deathState != CORPSE)
                break;

            if (m_groupLootTimer && lootingGroupLowGUID)
            {
                if (m_groupLootTimer <= diff)
                {
                    if (Group* group = sGroupMgr->GetGroupByGUID(lootingGroupLowGUID))
                        group->EndRoll(&loot);

                    m_groupLootTimer = 0;
                    lootingGroupLowGUID.Clear();
                }
                else m_groupLootTimer -= diff;
            }
            else if (m_corpseRemoveTime <= time(nullptr))
            {
                RemoveCorpse(false);
                TC_LOG_DEBUG(LOG_FILTER_UNITS, "Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY_ID));
            }
            break;
        }
        case ALIVE:
        {
            Unit::Update(diff);

            // if creature is charmed, switch to charmed AI (and back)
            if (NeedChangeAI)
            {
                UpdateCharmAI();
                NeedChangeAI = false;
                IsAIEnabled = true;
                if (!IsInEvadeMode() && LastCharmerGUID)
                    if (Unit* charmer = ObjectAccessor::GetUnit(*this, LastCharmerGUID))
                        i_AI->AttackStart(charmer);

                LastCharmerGUID.Clear();
            }

            if (!IsInEvadeMode() && IsAIEnabled)
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                i_AI->UpdateAI(diff);
                m_AI_locked = false;
            }

            if (m_regenTimer > 0)
            {
                if (diff >= m_regenTimer)
                    m_regenTimer = 0;
                else
                    m_regenTimer -= diff;
            }

            if(HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER))
            {
                m_regenTimerCount += diff;
                Regenerate(getPowerType(), diff);
                
                if (m_regenTimerCount >= GetRegenerateInterval())
                    m_regenTimerCount -= m_sendInterval;
            }

            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;

            //Check current difficulty map for change stats
            if (GetMap()->Instanceable() && !isPet())
            {
                if (GetMap()->GetSpawnMode() != GetSpawnMode())
                    UpdateStat();
                else if (GetMap()->IsNeedRecalc() && GetMap()->GetPlayersCountExceptGMs() > GetMap()->GetMinPlayer() && GetMap()->GetPlayersCountExceptGMs() != GetPlayerCount()) //For dynamic stats
                {
                    m_playerCount = GetMap()->GetPlayersCountExceptGMs();
                    UpdateMaxHealth();
                    UpdateAttackPowerAndDamage();
                }
            }

            if (m_respawnCombatDelay)
            {
                if (m_respawnCombatDelay <= diff)
                    m_respawnCombatDelay = 0;
                else
                    m_respawnCombatDelay -= diff;
            }

            if (AI() && IsDungeonBoss() && isInCombat())
                AI()->DoAggroPulse(diff);

            if (m_regenTimer != 0)
               break;

            bool bInCombat = isInCombat() && (!getVictim() ||                                        // if isInCombat() is true and this has no victim
                             !getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() ||                // or the victim/owner/charmer is not a player
                             !getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself()->isGameMaster()); // or the victim/owner/charmer is not a GameMaster

            if (!IsInEvadeMode() && (!bInCombat || IsPolymorphed())) // regenerate health if not in combat or if polymorphed
                RegenerateHealth();

            m_regenTimer = isAnySummons() ? PET_FOCUS_REGEN_INTERVAL : CREATURE_REGEN_INTERVAL;
            break;
        }
        default:
            break;
    }

    if (!isAlive())
        if (GetMap()->Instanceable() && !isPet())
            if(GetMap()->GetSpawnMode() != GetSpawnMode() || GetMap()->IsNeedRespawn(m_respawnChallenge))
            {
                m_respawnChallenge = GetMap()->m_respawnChallenge;
                Respawn(true);
            }

    m_isUpdate = false;
}

void Creature::RegenerateHealth()
{
    if (!isRegeneratingHealth())
        return;

    uint64 curValue = GetHealth();
    uint64 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    uint64 addvalue = 0;

    // Not only pet, but any controlled creature
    if (GetCharmerOrOwnerGUID())
        addvalue = uint32(24.376 * sWorld->getRate(RATE_HEALTH));
    else
        addvalue = maxValue / 1.2;

    addvalue *= GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
    addvalue += GetTotalAuraModifier(SPELL_AURA_MOD_REGEN) * CREATURE_REGEN_INTERVAL  / (5 * IN_MILLISECONDS);

    ModifyHealth(addvalue);
}

uint32 Creature::GetRegenerateInterval()
{
    if (!m_sendInterval)
        m_sendInterval = isAnySummons() ? PET_FOCUS_REGEN_INTERVAL : (GetCreatureTemplate()->Classification == CREATURE_CLASSIFICATION_WORLDBOSS ? BOSS_REGEN_INTERVAL : CREATURE_REGEN_INTERVAL);
    
    return m_sendInterval;
}

void Creature::DoFleeToGetAssistance()
{
    Unit* victim = getVictim();
    if (!victim)
        return;

    if (HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    float radius = sWorld->getFloatConfig(CONFIG_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS);
    if (radius >0)
    {
        Creature* creature = nullptr;

        CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
        Cell cell(p);
        cell.SetNoCreate();
        Trinity::NearestAssistCreatureInCreatureRangeCheck u_check(this, victim, radius);
        Trinity::CreatureLastSearcher<Trinity::NearestAssistCreatureInCreatureRangeCheck> searcher(this, creature, u_check);

        cell.Visit(p, Trinity::makeGridVisitor(searcher), *GetMap(), *this, radius);

        SetNoSearchAssistance(true);
        UpdateSpeed(MOVE_RUN, false);

        if (!creature)
            //SetFeared(true, getVictim()->GetGUID(), 0, sWorld->getIntConfig(CONFIG_CREATURE_FAMILY_FLEE_DELAY));
            //TODO: use 31365
            SetControlled(true, UNIT_STATE_FLEEING);
        else
            GetMotionMaster()->MoveSeekAssistance(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());

        if (m_CanCallAssistance)
            sCreatureTextMgr->SendChat(this, m_callAssistanceText, victim->GetGUID());
    }
}

bool Creature::AIM_Initialize(CreatureAI* ai)
{
    // make sure nothing can change the AI during AI update
    if (m_AI_locked)
    {
        TC_LOG_DEBUG(LOG_FILTER_TSCR, "AIM_Initialize: failed to init, locked.");
        return false;
    }

    UnitAI* oldAI = i_AI;

    Motion_Initialize();

    i_AI = ai ? ai : FactorySelector::selectAI(this);
    delete oldAI;
    IsAIEnabled = true;

    if (i_AI && isAlive())
        i_AI->InitializeAI();
    
    // Initialize vehicle
    if (GetVehicleKit() && !m_onVehicleAccessory)
        GetVehicleKit()->Reset();
    return true;
}

void Creature::Motion_Initialize()
{
    if (!m_formation)
        i_motionMaster.Initialize();
    else if (m_formation->getLeader() == this)
    {
        m_formation->FormationReset(false);
        i_motionMaster.Initialize();
    }
    else if (m_formation->isFormed())
        i_motionMaster.MoveIdle(); //wait the order of leader
    else
        i_motionMaster.Initialize();
}


bool Creature::Create(ObjectGuid::LowType guidlow, Map* map, uint32 phaseMask, uint32 entry, int32 vehId, uint32 team, float x, float y, float z, float ang, const CreatureData* data)
{
    ASSERT(map);
    SetMap(map);
    SetPhaseMask(phaseMask, false);

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(entry);
    if (!cinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature::Create(): creature template (guidlow: %u, entry: %u) does not exist.", guidlow, entry);
        return false;
    }

    //! Relocate before CreateFromProto, to initialize coords and allow
    //! returning correct zone id for selecting OutdoorPvP/Battlefield script
    Relocate(x, y, z, ang);

    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    if (!CreateFromProto(guidlow, entry, vehId, team, data))
        return false;

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature::Create(): given coordinates for creature (guidlow %d, entry %d) are not valid (X: %f, Y: %f, Z: %f, O: %f)", guidlow, entry, x, y, z, ang);
        return false;
    }

    bool isInstance = GetMap()->Instanceable();

    switch (cinfo->Classification)
    {
        case CREATURE_CLASSIFICATION_RARE:
            m_corpseDelay = isInstance ? sWorld->getIntConfig(CONFIG_CORPSE_DECAY_RARE) : 120;
            break;
        case CREATURE_CLASSIFICATION_ELITE:
            m_corpseDelay = isInstance ? sWorld->getIntConfig(CONFIG_CORPSE_DECAY_ELITE) : 120;
            break;
        case CREATURE_CLASSIFICATION_RARE_ELITE:
            m_corpseDelay = isInstance ? sWorld->getIntConfig(CONFIG_CORPSE_DECAY_RAREELITE) : 120;
            break;
        case CREATURE_CLASSIFICATION_WORLDBOSS:
            m_corpseDelay = isInstance ? sWorld->getIntConfig(CONFIG_CORPSE_DECAY_WORLDBOSS) : 120;
            break;
        default:
            m_corpseDelay = sWorld->getIntConfig(CONFIG_CORPSE_DECAY_NORMAL);
            break;
    }

    // All rare npc have max visibility distance.
    if (cinfo->Classification > CREATURE_CLASSIFICATION_ELITE && cinfo->Classification <= CREATURE_CLASSIFICATION_RARE)
        m_SightDistance = MAX_VISIBILITY_DISTANCE;

    if (cinfo->isWorldBoss() || cinfo->MaxVisible)
        MaxVisible = true;

    switch(GetMapId())
    {
        case 609: // start DK
        case 648: // start goblin
        case 654: // start worgen
        case 860: // start pandaren
            m_corpseDelay /= 3;
            break;
        default:
            break;
    }

    LoadCreaturesAddon();

    if (data)
    {
        if (data->AiID)
            SetAnimKitId(data->AiID);
        if (data->MovementID)
            SetMovementAnimKitId(data->MovementID);
        if (data->MeleeID)
            SetMeleeAnimKitId(data->MeleeID);
    }

    //! Need to be called after LoadCreaturesAddon - MOVEMENTFLAG_HOVER is set there
    if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
    {
        z += GetFloatValue(UNIT_FIELD_HOVER_HEIGHT);
        //! Relocate again with updated Z coord
        Relocate(x, y, z, ang);
    }

    if(CreatureAIInstance const* aiinstdata = sObjectMgr->GetCreatureAIInstaceData(entry))
        bossid = aiinstdata->bossid;

    uint32 displayID = GetNativeDisplayId();
    CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelRandomGender(&displayID);
    if (minfo && !isTotem())                               // Cancel load if no model defined or if totem
    {
        SetDisplayId(displayID);
        SetNativeDisplayId(displayID);
        SetGender(minfo->gender);
    }

    LastUsedScriptID = cinfo->ScriptID;

    // TODO: Replace with spell, handle from DB
    if (isSpiritHealer() || isSpiritGuide())
    {
        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
        m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
    }

    if (entry == VISUAL_WAYPOINT)
        SetVisible(false);

    m_areaId = GetMap()->GetAreaId(x, y, z);
    m_zoneId = GetMap()->GetZoneId(x, y, z);

    // code block for underwater state update
    if (!m_lastUnderWatterPos.IsInDist(this, World::Relocation_UpdateUnderwateLimit))
    {
        m_lastUnderWatterPos = GetPosition();
        UpdateUnderwaterState(GetMap(), x, y, z);
    }

    disableAffix = GetAffixState();
    if (disableAffix == 0)
    {
        if (IsDungeonBoss())
            disableAffix = ((1 << Affixes::Sanguine) | (1 << Affixes::Bursting) | (1 << Affixes::Bolstering) | (1 << Affixes::FelExplosives));
        else if (GetAnyOwner() && GetAnyOwner()->IsCreature())
        {
            if (auto owner = GetAnyOwner()->ToCreature())
            {
                if (owner->IsDungeonBoss())
                    disableAffix = AFFIXES_ALL;
                else
                    disableAffix = owner->GetAffixState();
            }
        }
    }

    creatureCountInWorld[entry]++;
    creatureCountInArea[m_areaId]++;

    return true;
}

void Creature::SetReactState(ReactStates st, uint32 delay /*= 0*/)
{
    if (m_reactState == st)
        return;

    if (delay && isInCombat())
    {
        AddDelayedCombat(delay, [this, st] () -> void
        {
            if (this && isInCombat())
                m_reactState = st;
        });
    }
    else
        m_reactState = st;
}

ReactStates Creature::GetReactState()
{
    return m_reactState;
}

bool Creature::HasReactState(ReactStates state) const
{
    return m_reactState == state;
}

void Creature::InitializeReactState()
{
    if (isTotem() || isTrigger() || GetCreatureType() == CREATURE_TYPE_CRITTER || isSpiritService() || GetEntry() == 100868)
        SetReactState(REACT_ATTACK_OFF);
    else
        SetReactState(REACT_AGGRESSIVE);
}

bool Creature::isCanTrainingOf(Player* player, bool msg) const
{
    if (!isTrainer())
        return false;

    TrainerSpellData const* trainer_spells = GetTrainerSpells();

    if ((!trainer_spells || trainer_spells->spellList.empty()) && GetCreatureTemplate()->trainer_type != TRAINER_TYPE_PETS)
    {
        //TC_LOG_ERROR(LOG_FILTER_SQL, "Creature %u (entry: %u) have UNIT_NPC_FLAG_TRAINER but have empty trainer spell list.",
            //GetGUIDLow(), GetEntry());
        return false;
    }

    switch (GetCreatureTemplate()->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if (player->getClass() != GetCreatureTemplate()->trainer_class)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureTemplate()->trainer_class)
                    {
                        case CLASS_DRUID:  player->PlayerTalkClass->SendGossipMenu(4913, GetGUID()); break;
                        case CLASS_HUNTER: player->PlayerTalkClass->SendGossipMenu(10090, GetGUID()); break;
                        case CLASS_MAGE:   player->PlayerTalkClass->SendGossipMenu(328, GetGUID()); break;
                        case CLASS_PALADIN:player->PlayerTalkClass->SendGossipMenu(1635, GetGUID()); break;
                        case CLASS_PRIEST: player->PlayerTalkClass->SendGossipMenu(4436, GetGUID()); break;
                        case CLASS_ROGUE:  player->PlayerTalkClass->SendGossipMenu(4797, GetGUID()); break;
                        case CLASS_SHAMAN: player->PlayerTalkClass->SendGossipMenu(5003, GetGUID()); break;
                        case CLASS_WARLOCK:player->PlayerTalkClass->SendGossipMenu(5836, GetGUID()); break;
                        case CLASS_WARRIOR:player->PlayerTalkClass->SendGossipMenu(4985, GetGUID()); break;
                        default:
                            break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if (player->getClass() != CLASS_HUNTER)
            {
                player->PlayerTalkClass->ClearMenus();
                player->PlayerTalkClass->SendGossipMenu(3620, GetGUID());
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if (GetCreatureTemplate()->trainer_race && player->getRace() != GetCreatureTemplate()->trainer_race)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureTemplate()->trainer_race)
                    {
                    case RACE_DWARF:        player->PlayerTalkClass->SendGossipMenu(5865, GetGUID()); break;
                    case RACE_GNOME:        player->PlayerTalkClass->SendGossipMenu(4881, GetGUID()); break;
                    case RACE_HUMAN:        player->PlayerTalkClass->SendGossipMenu(5861, GetGUID()); break;
                    case RACE_NIGHTELF:     player->PlayerTalkClass->SendGossipMenu(5862, GetGUID()); break;
                    case RACE_ORC:          player->PlayerTalkClass->SendGossipMenu(5863, GetGUID()); break;
                    case RACE_TAUREN:       player->PlayerTalkClass->SendGossipMenu(5864, GetGUID()); break;
                    case RACE_TROLL:        player->PlayerTalkClass->SendGossipMenu(5816, GetGUID()); break;
                    case RACE_UNDEAD_PLAYER:player->PlayerTalkClass->SendGossipMenu(624, GetGUID()); break;
                    case RACE_BLOODELF:     player->PlayerTalkClass->SendGossipMenu(5862, GetGUID()); break;
                    case RACE_DRAENEI:      player->PlayerTalkClass->SendGossipMenu(5864, GetGUID()); break;
                    //case RACE_PANDAREN_HORDE player->PlayerTalkClass->SendGossipMenu( ???, GetGUID()); break; //TODO Find gossipID
                    //case RACE_PANDAREN_ALLIANCE  player->PlayerTalkClass->SendGossipMenu( ???, GetGUID()); break; //TODO Find gossipID
                    default:
                        break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if (GetCreatureTemplate()->trainer_spell/* && !player->HasSpell(GetCreatureTemplate()->trainer_spell)*/)
            {
                if (msg)
                {
                    player->PlayerTalkClass->ClearMenus();
                    player->PlayerTalkClass->SendGossipMenu(11031, GetGUID());
                }
                return false;
            }
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::isCanInteractWithBattleMaster(Player* player, bool msg) const
{
    if (!isBattleMaster())
        return false;

    uint16 bgTypeID = sBattlegroundMgr->GetBattleMasterBG(GetEntry());
    if (!msg)
        return player->GetBGAccessByLevel(bgTypeID);

    if (!player->GetBGAccessByLevel(bgTypeID))
    {
        player->PlayerTalkClass->ClearMenus();
        switch (bgTypeID)
        {
            case MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley:  player->PlayerTalkClass->SendGossipMenu(7616, GetGUID()); break;
            case MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch:  player->PlayerTalkClass->SendGossipMenu(7599, GetGUID()); break;
            case MS::Battlegrounds::BattlegroundTypeId::BattlegroundArathiBasin:  player->PlayerTalkClass->SendGossipMenu(7642, GetGUID()); break;
            //case MS::Battlegrounds::BattlegroundTypeId::BattlegroundEyeOfTheStorm:
            //case MS::Battlegrounds::BattlegroundTypeId::ArenaNagrandArena:
            //case MS::Battlegrounds::BattlegroundTypeId::ArenaBladesEdge:
            //case MS::Battlegrounds::BattlegroundTypeId::ArenaAll:
            //case MS::Battlegrounds::BattlegroundTypeId::ArenaRuinsOfLordaeron:
            //case MS::Battlegrounds::BattlegroundTypeId::BattlegroundIsleOfConquest:
            //case MS::Battlegrounds::BattlegroundTypeId::ArenaDalaranSewers: player->PlayerTalkClass->SendGossipMenu(10024, GetGUID()); break;
            default: break;
        }
        return false;
    }
    return true;
}

bool Creature::isCanTrainingAndResetTalentsOf(Player* player) const
{
    return player->getLevel() >= 15 && GetCreatureTemplate()->trainer_type == TRAINER_TYPE_CLASS && player->getClass() == GetCreatureTemplate()->trainer_class;
}

Player* Creature::GetLootRecipient() const
{
    if (!m_lootRecipient)
        return nullptr;
    return ObjectAccessor::FindPlayer(m_lootRecipient);
}

Group* Creature::GetLootRecipientGroup() const
{
    if (!m_lootRecipientGroup)
        return nullptr;
    return sGroupMgr->GetGroupByGUID(m_lootRecipientGroup);
}

Unit* Creature::GetOtherRecipient() const
{
    if (!m_LootOtherRecipient)
        return nullptr;
    return ObjectAccessor::GetUnit(*this, m_LootOtherRecipient);
}

void Creature::SetOtherLootRecipient(ObjectGuid guid)
{
    m_LootOtherRecipient = guid;
}

void Creature::SetLootRecipient(Unit* unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears

    if (!unit)
    {
        m_lootRecipient.Clear();
        m_lootRecipientGroup.Clear();
        RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE | UNIT_DYNFLAG_TAPPED);
        return;
    }

    if (!unit->IsPlayer() && !unit->IsVehicle())
        return;

    Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!player)                                             // normal creature, no player involved
        return;

    m_lootRecipient = player->GetGUID();
    if (Group* group = player->GetGroup())
        m_lootRecipientGroup = group->GetGUID();

    SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED);
}

bool Creature::hasLootRecipient() const
{
    return m_lootRecipient || m_lootRecipientGroup;
}

// return true if this creature is tapped by the player or by a member of his group.
bool Creature::isTappedBy(Player const* player) const
{
    if (player->GetGUID() == m_lootRecipient)
        return true;

    if (IsPersonal()) // Not Tapped to all for personal loot
        return true;

    if (IsPersonalForQuest(player)) // Not Tapped for all quest mobs in new addon(test)
        return true;

    Group const* playerGroup = player->GetGroup();
    if (!playerGroup || playerGroup != GetLootRecipientGroup()) // if we dont have a group we arent the recipient
        return false;                                           // if creature doesnt have group bound it means it was solo killed by someone else

    return true;
}

void Creature::SaveToDB()
{
    // this should only be used when the creature has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    CreatureData const* data = sObjectMgr->GetCreatureData(m_DBTableGuid);
    if (!data)
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature::SaveToDB failed, cannot get creature data!");
        return;
    }

    uint32 mapId = GetTransport() ? GetTransport()->GetGOInfo()->moTransport.SpawnMap : GetMapId();
    SaveToDB(mapId, data->spawnMask, GetPhaseMask());
}

void Creature::SaveToDB(uint32 mapid, uint64 spawnMask, uint32 phaseMask)
{
    // update in loaded data
    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUIDLow();

    CreatureData& data = sObjectMgr->NewOrExistCreatureData(m_DBTableGuid);

    uint32 displayId = GetNativeDisplayId();
    if (IsMirrorImage())
        displayId = 0; // For mirror images dont save displayid, it comes from outfit
    
    uint32 npcflag = GetUInt32Value(UNIT_FIELD_NPC_FLAGS);
    uint32 npcflag2 = GetUInt32Value(UNIT_FIELD_NPC_FLAGS2);
    uint32 unit_flags = GetUInt32Value(UNIT_FIELD_FLAGS);
    uint32 unit_flags3 = GetUInt32Value(UNIT_FIELD_FLAGS_3);
    uint32 dynamicflags = GetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS);

    // check if it's a custom model and if not, use 0 for displayId
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo)
    {
        if (displayId == sObjectMgr->GetCreatureDisplay(cinfo->Modelid[0]) || sObjectMgr->GetCreatureDisplay(displayId == cinfo->Modelid[1]) ||
            displayId == sObjectMgr->GetCreatureDisplay(cinfo->Modelid[2]) || sObjectMgr->GetCreatureDisplay(displayId == cinfo->Modelid[3]))
            displayId = 0;

        if (npcflag == cinfo->npcflag)
            npcflag = 0;

        if (npcflag2 == cinfo->npcflag2)
            npcflag2 = 0;

        if (unit_flags == cinfo->unit_flags)
            unit_flags = 0;

        if (unit_flags3 == cinfo->unit_flags3)
            unit_flags3 = 0;

        if (dynamicflags == cinfo->dynamicflags)
            dynamicflags = 0;
    }

    uint32 zoneId = 0;
    uint32 areaId = 0;
    sMapMgr->GetZoneAndAreaId(zoneId, areaId, mapid, GetPositionX(), GetPositionY(), GetPositionZ());

    // data->guid = guid must not be updated at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.zoneId = zoneId;
    data.areaId = areaId;
    data.phaseMask = phaseMask;
    data.displayid = displayId;
    data.equipmentId = GetCurrentEquipmentId();
    if (!GetTransport())
    {
        data.posX = GetPositionX();
        data.posY = GetPositionY();
        data.posZ = GetPositionZH();
        data.orientation = GetOrientation();
    }
    else
    {
        data.posX = GetTransOffsetX();
        data.posY = GetTransOffsetY();
        data.posZ = GetTransOffsetZ();
        data.orientation = GetTransOffsetO();
    }

    data.spawntimesecs = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType() == IDLE_MOTION_TYPE ? 0.0f : m_respawnradius;
    data.currentwaypoint = 0;
    data.curhealth = GetHealth();
    data.curmana = GetPower(POWER_MANA);
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType() == RANDOM_MOTION_TYPE ? IDLE_MOTION_TYPE : GetDefaultMovementType();
    data.spawnMask = spawnMask;
    data.npcflag = npcflag;
    data.npcflag2 = npcflag2;
    data.unit_flags = unit_flags;
    data.unit_flags3 = unit_flags3;
    data.dynamicflags = dynamicflags;
    data.isActive = isActiveObject();
    // data.MaxVisible = cinfo->MaxVisible;

    // update in DB
    SQLTransaction trans = WorldDatabase.BeginTransaction();

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    uint8 index = 0;

    stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE);
    stmt->setUInt64(index++, m_DBTableGuid);
    stmt->setUInt32(index++, GetEntry());
    stmt->setUInt16(index++, uint16(mapid));
    stmt->setUInt32(index++, zoneId);
    stmt->setUInt32(index++, areaId);
    stmt->setUInt64(index++, spawnMask);
    stmt->setUInt16(index++, uint16(GetPhaseMask()));
    stmt->setUInt32(index++, displayId);
    stmt->setUInt8(index++, GetCurrentEquipmentId());
    stmt->setFloat(index++,  GetPositionX());
    stmt->setFloat(index++,  GetPositionY());
    stmt->setFloat(index++,  GetPositionZH());
    stmt->setFloat(index++,  GetOrientation());
    stmt->setUInt32(index++, m_respawnDelay);
    stmt->setFloat(index++,  m_respawnradius);
    stmt->setUInt32(index++, 0);
    stmt->setUInt32(index++, GetHealth());
    stmt->setUInt32(index++, GetPower(POWER_MANA));
    stmt->setUInt8(index++,  uint8(GetDefaultMovementType()));
    stmt->setUInt32(index++, npcflag);
    stmt->setUInt32(index++, unit_flags);
    stmt->setUInt32(index++, unit_flags3);
    stmt->setUInt32(index++, dynamicflags);
    stmt->setBool(index++, isActiveObject());
    stmt->setFloat(index++, data.personalSize);
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

void Creature::SelectLevel(const CreatureTemplate* cInfo)
{
    uint32 rank = isPet() ? 0 : cInfo->Classification;

    CreatureDifficultyStat const* diffStats = GetCreatureDiffStat();

    // level
    uint8 level = 0;
    if(m_difficulty == 1 || m_difficulty == 2)
        level = cInfo->maxlevel;
    else
        level = cInfo->minlevel;

    if (BattlegroundMap* map = GetMap()->ToBgMap())
    {
        if (map->GetBG())
            level = map->GetBG()->GetMaxLevel();
    }

    SetLevel(level);
    SetEffectiveLevel(0);

    CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(level, cInfo->unit_class);
    ASSERT(stats);

    // health
    float healthmod = _GetHealthMod(rank);
    if (!diffStats)
        healthmod *= _GetHealthModForDiff();

    uint64 basehp = stats->GenerateHealth(cInfo, diffStats);
    auto health = uint64(basehp * healthmod);

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);
    ResetPlayerDamageReq();

    // mana
    uint32 mana = stats->GenerateMana(cInfo);
    SetCreateMana(mana);

    switch (getClass())
    {
        case CLASS_WARRIOR:
            setPowerType(POWER_RAGE);
            //SetMaxPower(POWER_RAGE, GetCreatePowers(POWER_RAGE));
            //SetPower(POWER_RAGE, GetCreatePowers(POWER_RAGE));
            break;
        case CLASS_ROGUE:
            setPowerType(POWER_ENERGY);
            SetMaxPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
            SetPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
            break;
        default:
            setPowerType(POWER_MANA);
            SetMaxPower(POWER_MANA, mana);
            SetPower(POWER_MANA, mana);
            break;
    }

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, static_cast<float>(health));
    SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, static_cast<float>(mana));

    //damage
    float maxDmgMod = 1.5f;
    if (GetMap() && GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
        maxDmgMod = 1.2f;

    float basedamage = stats->GenerateBaseDamage(cInfo) * _GetDamageModForDiff();

    float weaponBaseMinDamage = basedamage;
    float weaponBaseMaxDamage = basedamage * maxDmgMod;

    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, weaponBaseMinDamage);
    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, weaponBaseMaxDamage);

    SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, weaponBaseMinDamage);
    SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, weaponBaseMaxDamage);

    SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, weaponBaseMinDamage);
    SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, weaponBaseMaxDamage);

    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, stats->AttackPower);
    SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, stats->RangedAttackPower);

    if (!ScaleLevelMin || !ScaleLevelMax)
    {
        if (level >= m_levelStat.size())
            m_levelStat.resize(level + 1);

        CreatureLevelStat& levelStat = m_levelStat[level];
        levelStat.baseHP = basehp;
        levelStat.baseMP = mana;
        levelStat.healthMax = health;
        levelStat.baseMinDamage = weaponBaseMinDamage;
        levelStat.baseMaxDamage = weaponBaseMaxDamage;
        levelStat.AttackPower = stats->AttackPower;
        levelStat.RangedAttackPower = stats->RangedAttackPower;
        levelStat.BaseArmor = stats->GenerateArmor(cInfo);
    }
    else
        GenerateScaleLevelStat(cInfo);
}

void Creature::GenerateScaleLevelStat(const CreatureTemplate* cInfo)
{
    uint32 rank = isPet() ? 0 : cInfo->Classification;
    float healthmod = _GetHealthMod(rank);
    CreatureDifficultyStat const* diffStats = GetCreatureDiffStat();
    if (!diffStats)
        healthmod *= _GetHealthModForDiff();

    float maxDmgMod = 1.5f;
    if (GetMap() && GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
        maxDmgMod = 1.2f;

    for (uint8 level = ScaleLevelMin; level <= ScaleLevelMax; ++level)
    {
        CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(level, cInfo->unit_class);

        // health
        uint64 basehp = stats->GenerateHealth(cInfo, diffStats);
        auto health = uint64(basehp * healthmod);

        // mana
        uint32 mana = stats->GenerateMana(cInfo);

        //damage
        float basedamage = stats->GenerateBaseDamage(cInfo) * _GetDamageModForDiff();

        //armor
        uint32 armor = stats->GenerateArmor(cInfo);

        if (level >= m_levelStat.size())
            m_levelStat.resize(level + 1);

        CreatureLevelStat& levelStat = m_levelStat[level];
        levelStat.baseHP = basehp;
        levelStat.baseMP = mana;
        levelStat.healthMax = health;
        levelStat.baseMinDamage = basedamage;
        levelStat.baseMaxDamage = basedamage * maxDmgMod;
        levelStat.AttackPower = stats->AttackPower;
        levelStat.RangedAttackPower = stats->RangedAttackPower;
        levelStat.BaseArmor = armor;
    }
}

CreatureLevelStat const* Creature::GetScaleLevelStat(uint8 level)
{
    if (level >= m_levelStat.size())
        return nullptr;

    return &m_levelStat[level];
}

float Creature::_GetHealthMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_CLASSIFICATION_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_HP);
        case CREATURE_CLASSIFICATION_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_CLASSIFICATION_RARE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_CLASSIFICATION_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_CLASSIFICATION_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_HP);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_HP);
    }
}

float Creature::_GetHealthModForDiff()
{
    switch (m_spawnMode)
    {
        case DIFFICULTY_LFR_RAID:
            return 0.5f;
        case DIFFICULTY_HC_SCENARIO:
        case DIFFICULTY_HEROIC:
            return 1.35f;
        case DIFFICULTY_MYTHIC_DUNGEON:
        case DIFFICULTY_MYTHIC_KEYSTONE:
            return 1.65f;
        case DIFFICULTY_HEROIC_RAID:
            return 1.55f;
        case DIFFICULTY_MYTHIC_RAID:
            return 1.95f;
        case DIFFICULTY_25_HC:
            return 5.0f;
        case DIFFICULTY_25_N:
            return 3.0f;
        case DIFFICULTY_LFR:
            return 2.0f;
        default:
            return 1.0f;
    }
}

float Creature::_GetDamageModForDiff()
{
    //Hack. Example Spell 243042
    if (GetAnyOwner() && GetAnyOwner()->IsPlayer())
    {
        if (TempSummon* tempSum = ToTempSummon())
            if (tempSum->m_Properties && tempSum->m_Properties->Flags & SUMMON_PROP_FLAG_UNK18) //Companions?
                return 1.0f;
    }

    switch (m_spawnMode)
    {
        case DIFFICULTY_LFR_RAID:
            return 0.66f;
        case DIFFICULTY_HC_SCENARIO:
            return 1.90f;
        case DIFFICULTY_HEROIC:
            return 1.25;
        case DIFFICULTY_MYTHIC_DUNGEON:
        case DIFFICULTY_MYTHIC_KEYSTONE:
            return 1.85f;
        case DIFFICULTY_HEROIC_RAID:
            return 1.25f;
        case DIFFICULTY_MYTHIC_RAID:
            return 1.5f;
        case DIFFICULTY_10_HC:
        case DIFFICULTY_25_HC:
            return 2.0f;
        case DIFFICULTY_LFR:
            return 0.6f;
        default:
            return 1.0f;
    }
}

float Creature::_GetHealthModPersonal(uint32 &count)
{
    switch (GetCreatureTemplate()->Classification)
    {
        case CREATURE_CLASSIFICATION_NORMAL:
        case CREATURE_CLASSIFICATION_ELITE:
        case CREATURE_CLASSIFICATION_RARE_ELITE:
        case CREATURE_CLASSIFICATION_RARE:
        {
            if (GetCreatureTemplate()->RequiredExpansion < EXPANSION_WARLORDS_OF_DRAENOR)
                break;

            if (!GetMap()->Instanceable())
            {
                count -= 1; //first player
                return 0.7f; //From WOD hp increment 70% by player
            }
            
            if (GetMap()->IsNeedRecalc() && count > GetMap()->GetMinPlayer()) //Base hp for min player if > min need increment hp
            {
                count -= GetMap()->GetMinPlayer();
                return 0.1f;
            }
        }
        case CREATURE_CLASSIFICATION_WORLDBOSS:
        {
            if (GetCreatureTemplate()->RequiredExpansion < EXPANSION_WARLORDS_OF_DRAENOR)
                break;

            if (GetMap()->IsNeedRecalc() && count > GetMap()->GetMinPlayer()) //Base hp for min player if > min need increment hp
            {
                count -= GetMap()->GetMinPlayer();
                return 0.105f;
            }
        }
        default:
            break;
    }

    return 0.0f;
}

float Creature::_GetDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_CLASSIFICATION_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_DAMAGE);
        case CREATURE_CLASSIFICATION_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
        case CREATURE_CLASSIFICATION_RARE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_DAMAGE);
        case CREATURE_CLASSIFICATION_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE);
        case CREATURE_CLASSIFICATION_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_DAMAGE);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_DAMAGE);
    }
}

float Creature::GetSpellDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_CLASSIFICATION_NORMAL:
            return sWorld->getRate(RATE_CREATURE_NORMAL_SPELLDAMAGE);
        case CREATURE_CLASSIFICATION_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
        case CREATURE_CLASSIFICATION_RARE_ELITE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE);
        case CREATURE_CLASSIFICATION_WORLDBOSS:
            return sWorld->getRate(RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE);
        case CREATURE_CLASSIFICATION_RARE:
            return sWorld->getRate(RATE_CREATURE_ELITE_RARE_SPELLDAMAGE);
        default:
            return sWorld->getRate(RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
    }
}

bool Creature::CreateFromProto(ObjectGuid::LowType guidlow, uint32 entry, int32 vehId, uint32 team, const CreatureData* data)
{
    SetZoneScript();
    if (m_zoneScript && data)
    {
        entry = m_zoneScript->GetCreatureEntry(guidlow, data);
        if (!entry)
            return false;
    }

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(entry);
    if (!cinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature::CreateFromProto(): creature template (guidlow: %u, entry: %u) does not exist.", guidlow, entry);
        return false;
    }

    SetOriginalEntry(entry);

    //Privent setup own accessory if we are part of accessory enother vehicle.. do it after enter.
    m_onVehicleAccessory = vehId == -1;
    if (vehId <= 0)
        vehId = cinfo->VehicleId;

    m_creatureInfo = cinfo;

    if (vehId || cinfo->VehicleId)
        Object::_Create(ObjectGuid::Create<HighGuid::Vehicle>(GetMapId(), entry, guidlow));
    else
        Object::_Create(ObjectGuid::Create<HighGuid::Creature>(GetMapId(), entry, guidlow));

    m_movementInfo.Guid = GetGUID();

    if (!UpdateEntry(entry, team, data))
        return false;

    if (data)
        SetPhaseId(data->PhaseID, false);

    if (vehId)
        CreateVehicleKit(vehId, entry, 0, true);

    if(m_creatureInfo->StateWorldEffectID)
        SetUInt32Value(UNIT_FIELD_STATE_WORLD_EFFECT_ID, m_creatureInfo->StateWorldEffectID);
    if(m_creatureInfo->SpellStateVisualID)
        SetUInt32Value(UNIT_FIELD_STATE_WORLD_EFFECT_ID, m_creatureInfo->SpellStateVisualID);
    if(m_creatureInfo->SpellStateAnimID)
        SetUInt32Value(UNIT_FIELD_STATE_WORLD_EFFECT_ID, m_creatureInfo->SpellStateAnimID);
    if(m_creatureInfo->SpellStateAnimKitID)
        SetUInt32Value(UNIT_FIELD_STATE_WORLD_EFFECT_ID, m_creatureInfo->SpellStateAnimKitID);

    for (auto WorldEffect : m_creatureInfo->WorldEffects)
        AddDynamicValue(UNIT_DYNAMIC_FIELD_WORLD_EFFECTS, WorldEffect);

    for (auto PassiveSpell : m_creatureInfo->PassiveSpells)
        AddDynamicValue(UNIT_DYNAMIC_FIELD_PASSIVE_SPELLS, PassiveSpell);

    return true;
}

bool Creature::LoadCreatureFromDB(ObjectGuid::LowType guid, Map* map, bool addToMap)
{
    CreatureData const* data = sObjectMgr->GetCreatureData(guid);
    if (!data)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: %u) not found in table `creature`, can't load. ", guid);
        return false;
    }

    m_DBTableGuid = guid;

    if (map->GetInstanceId() == 0)
    {
        if (map->GetCreature(ObjectGuid::Create<HighGuid::Creature>(data->mapid, data->id, guid)))
            return false;
    }
    else
    {
        if (data->isTeemingSpawn)
        {
            if (auto progress = sScenarioMgr->GetScenario(map->GetInstanceId()))
            {
                auto const& challenge = progress->GetChallenge();
                if (!challenge)
                    return false;

                if (!challenge->HasAffix(Affixes::Teeming))
                    return false;
            }
        }

        guid = sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate();
    }

    uint16 team = 0;
    if (!Create(guid, map, data->phaseMask, data->id, 0, team, data->posX, data->posY, data->posZ, data->orientation, data))
        return false;

    //We should set first home position, because then AI calls home movement
    SetHomePosition(data->posX, data->posY, data->posZ, data->orientation);

    m_respawnradius = data->spawndist;

    m_respawnDelay = data->spawntimesecs;
    m_deathState = ALIVE;

    m_respawnTime = GetMap()->GetCreatureRespawnTime(m_DBTableGuid);
    if (m_respawnTime)                          // respawn on Update
    {
        m_deathState = DEAD;
        if (CanFly())
        {
            float tz = map->GetHeight(GetPhases(), data->posX, data->posY, data->posZ, false);
            if (data->posZ - tz > 0.1f)
                Relocate(data->posX, data->posY, tz);
        }
    }

    uint64 curhealth;

    if (!m_regenHealth)
    {
        curhealth = data->curhealth;
        if (curhealth)
        {
            curhealth = uint64(curhealth * _GetHealthMod(GetCreatureTemplate()->Classification));
            if (curhealth < 1)
                curhealth = 1;
        }

        switch (getClass())
        {
            case CLASS_WARRIOR:
                setPowerType(POWER_RAGE);
                //SetMaxPower(POWER_RAGE, GetCreatePowers(POWER_RAGE));
                //SetPower(POWER_RAGE, GetCreatePowers(POWER_RAGE));
                break;
            case CLASS_ROGUE:
                setPowerType(POWER_ENERGY);
                SetMaxPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
                SetPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
                break;
            default:
                SetPower(POWER_MANA, data->curmana);
                break;
        }
    }
    else
    {
        curhealth = GetMaxHealth();
        switch (getClass())
        {
            case CLASS_WARRIOR:
                setPowerType(POWER_RAGE);
                //SetMaxPower(POWER_RAGE, GetCreatePowers(POWER_RAGE));
                //SetPower(POWER_RAGE, GetCreatePowers(POWER_RAGE));
                break;
            case CLASS_ROGUE:
                setPowerType(POWER_ENERGY);
                SetMaxPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
                SetPower(POWER_ENERGY, GetCreatePowers(POWER_ENERGY));
                break;
            default:
                SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
                break;
        }
    }

    SetHealth(m_deathState == ALIVE ? curhealth : 0);

    // checked at creature_template loading
    if (isDead())
        m_defaultMovementType = MovementGeneratorType(0);
    else
        m_defaultMovementType = MovementGeneratorType(data->movementType);

    m_creatureData = data;

    setActive(data->isActive);

    if (addToMap && !GetMap()->AddToMap(this))
        return false;

    GetMap()->AddBattlePet(this);

    volatile uint32 npcEntry = GetEntry();
    volatile uint32 npcGuidLow = GetGUIDLow();

    if (data->gameEvent && m_DBTableGuid != guid)
        sGameEventMgr->mGameEventCreatureSpawns[data->gameEvent].push_back(GetGUID());

    return true;
}

void Creature::LoadEquipment(int8 id, bool force)
{
    if (id == 0)
    {
        if (force)
        {
            for (uint8 i = 0; i < MAX_EQUIPMENT_ITEMS; ++i)
                SetVirtualItem(i, 0);
            m_equipmentId = 0;
        }
        return;
    }

    EquipmentInfo const* einfo = sObjectMgr->GetEquipmentInfo(GetEntry(), id);
    if (!einfo)
        return;

    m_equipmentId = id;
    for (uint8 i = 0; i < MAX_EQUIPMENT_ITEMS; ++i)
        SetVirtualItem(i, einfo->ItemEntry[i*2], einfo->ItemEntry[(i*2)+1]);
}

bool Creature::hasQuest(uint32 quest_id) const
{
    QuestRelationBounds qr = sQuestDataStore->GetCreatureQuestRelationBounds(GetEntry());
    for (auto itr = qr.first; itr != qr.second; ++itr)
        if (itr->second == quest_id)
            return true;

    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelationBounds qir = sQuestDataStore->GetCreatureQuestInvolvedRelationBounds(GetEntry());
    for (auto itr = qir.first; itr != qir.second; ++itr)
        if (itr->second == quest_id)
            return true;

    return false;
}

uint8 Creature::GetPetAutoSpellSize() const
{
    return m_autospells.size();
}

uint32 Creature::GetPetAutoSpellOnPos(uint8 pos) const
{
    if (pos >= m_autospells.size())
        return 0;
    return m_autospells[pos];
}

uint8 Creature::GetPetCastSpellSize() const
{
    return m_castspells.size();
}

void Creature::AddPetCastSpell(uint32 spellid)
{
    m_castspells.push_back(spellid);
}

uint32 Creature::GetPetCastSpellOnPos(uint8 pos) const
{
    if (pos >= m_castspells.size())
        return 0;
    return m_castspells[pos];
}

void Creature::DeleteFromDB()
{
    if (!m_DBTableGuid)
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "Trying to delete not saved creature! LowGUID: %u, entry: %u", GetGUIDLow(), GetEntry());
        return;
    }

    GetMap()->RemoveCreatureRespawnTime(m_DBTableGuid);
    sObjectMgr->DeleteCreatureData(m_DBTableGuid);

    SQLTransaction trans = WorldDatabase.BeginTransaction();

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_CREATURE_ADDON);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAME_EVENT_CREATURE);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAME_EVENT_MODEL_EQUIP);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

bool Creature::IsInvisibleDueToDespawn() const
{
    if (Unit::IsInvisibleDueToDespawn())
        return true;

    return !(isAlive() || m_corpseRemoveTime > time(nullptr));
}

bool Creature::CanAlwaysSee(WorldObject const* obj) const
{
    return IsAIEnabled && AI() && AI()->CanSeeAlways(obj);
}

bool Creature::IsNeverVisible(WorldObject const* /*seer*/) const
{
    return WorldObject::IsNeverVisible();
}

bool Creature::canStartAttack(Unit const* who, bool force) const
{
    if (isCivilian())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC))
        return false;

    // Do not attack non-combat pets
    if (who->IsCreature() && who->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET)
        return false;

    if (!CanFly() && (GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE + m_CombatDistance))
        //|| who->IsControlledByPlayer() && who->IsFlying()))
        // we cannot check flying for other creatures, too much map/vmap calculation
        // TODO: should switch to range attack
        return false;

    if (!force)
    {
        if (!_IsTargetAcceptable(who))
            return false;

        if (who->isInCombat() && IsWithinDist(who, ATTACK_DISTANCE))
            if (Unit* victim = who->getAttackerForHelper())
                if (IsWithinDistInMap(victim, sWorld->getFloatConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS)))
                    force = true;

        // this check never happent in LoS for neutralToALL creatures as they by default have reactorAI or PassiveAI
        if (!force && (/*IsNeutralToAll() || */!IsWithinDistInMap(who, GetAttackDistance(who) + m_CombatDistance, true, true)))
            return false;
    }

    if (!canCreatureAttack(who, force))
        return false;

    if (!force && m_respawnCombatDelay)
        return false;

    return IsWithinLOSInMap(who);
}

float Creature::GetAttackDistance(Unit const* target) const
{
    float aggroRate = sWorld->getRate(RATE_CREATURE_AGGRO);
    if (aggroRate == 0)
        return 0.0f;

    uint32 targetLevel = target->getLevelForTarget(this);
    uint32 creatureLevel = getLevelForTarget(target);

    int32 leveldif = int32(targetLevel) - int32(creatureLevel);

    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 30 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    if (leveldif < -25)
        leveldif = -25;

    float RetDistance = GetBoundingRadius() + GetObjectSize() + target->GetCombatReach() - GetModelSize();
    if (!IsDungeonBoss() && RetDistance < 15.0f)
        RetDistance = 15.0f;

    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if targetLevel < creatureLevel
    RetDistance -= static_cast<float>(leveldif);

    if (creatureLevel+5 <= sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        RetDistance += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);
        RetDistance += target->GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);
    }

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if (RetDistance < 5.0f)
        RetDistance = 5.0f;

    if (auto cre = target->ToCreature())
    {
        bool canGain = cre->GetCreatureType() == CREATURE_TYPE_CRITTER;
        if (!canGain)
            if (auto faction = cre->getFactionTemplateEntry())
                canGain = faction->ID == 28 || faction->Friend[1] == 28;

        if (canGain)
            RetDistance += 25.0f;
    }

    return (RetDistance*aggroRate);
}

void Creature::setDeathState(DeathState s)
{
    Unit::setDeathState(s);

    if (s == JUST_DIED)
    {
        // Disable hover after die
        if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
            SetHover(false);

        m_corpseRemoveTime = time(nullptr) + m_corpseDelay;
        int32 _respawnDelay = m_respawnDelay;

        if (sWorld->getBoolConfig(CONFIG_RESPAWN_FROM_PLAYER_ENABLED))
        {
            if (_respawnDelay <= 600 && !GetMap()->Instanceable()) // квестовые мобы и прочий шлак
            {
                uint32 targetCount = GetPlayerFromArea(m_areaId);
                if (targetCount)
                {
                    if (targetCount >= sWorld->getIntConfig(CONFIG_RESPAWN_FROM_PLAYER_COUNT))
                        _respawnDelay /= targetCount; // грубый рассчет, конечно, но лучше уж..

                    if (_respawnDelay < 10)
                        _respawnDelay = urand(10, 15);
                }
            }
        }
        m_respawnTime = time(nullptr) + _respawnDelay + m_corpseDelay;

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY) || isWorldBoss())
            SaveRespawnTime();

        SetTarget(ObjectGuid::Empty);                // remove target selection in any cases (can be set at aura remove in Unit::setDeathState)
        SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

        setActive(false);

        if (!isPet() && GetCreatureTemplate()->SkinLootId)
            if (LootTemplates_Skinning.HaveLootFor(GetCreatureTemplate()->SkinLootId))
                if (hasLootRecipient())
                    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        if (HasSearchedAssistance())
        {
            SetNoSearchAssistance(false);
            UpdateSpeed(MOVE_RUN, false);
        }

        //Dismiss group if is leader
        if (m_formation && m_formation->getLeader() == this)
            m_formation->FormationReset(true);

        if (CanFly() || IsFlying() || (GetMiscStandValue() & UNIT_BYTE1_FLAG_HOVER))
            i_motionMaster.MoveFall();

        Unit::setDeathState(CORPSE);
    }
    else if (s == JUST_RESPAWNED)
    {
        //if (isPet())
        //    setActive(true);
        SetFullHealth();
        SetLootRecipient(nullptr);
        ResetPlayerDamageReq();
        CreatureTemplate const* cinfo = GetCreatureTemplate();
        SetWalk(true);
        if (cinfo->InhabitType & INHABIT_AIR && cinfo->InhabitType & INHABIT_GROUND)
            SetCanFly(true);
        else if (cinfo->InhabitType & INHABIT_AIR)
            SetDisableGravity(true);
        if (cinfo->InhabitType & INHABIT_WATER)
            AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);

        //Enable hover when respawn
        if (GetMiscStandValue() & UNIT_BYTE1_FLAG_HOVER)
            SetHover(true);

        SetUInt32Value(UNIT_FIELD_NPC_FLAGS, cinfo->npcflag);
        SetUInt32Value(UNIT_FIELD_NPC_FLAGS2, cinfo->npcflag2);
        ClearUnitState(uint32(UNIT_STATE_ALL_STATE));
        SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));
        AddDelayedEvent(10, [this]() -> void
        {
            LoadCreaturesAddon(true);
        });
        Motion_Initialize();
        if (GetCreatureData() && GetPhaseMask() != GetCreatureData()->phaseMask)
            SetPhaseMask(GetCreatureData()->phaseMask, false);
        Unit::setDeathState(ALIVE);
    }
}

void Creature::Respawn(bool force, uint32 timer /*= 3*/)
{
    Movement::MoveSplineInit(*this).Stop(true);
    DestroyForNearbyPlayers();

    if (!GetMap()->IsDungeon())
        m_respawnCombatDelay = 5000;

    if (force)
    {
        if (isAlive())
            setDeathState(JUST_DIED);
        else if (getDeathState() != CORPSE)
            setDeathState(CORPSE);
    }

    if (getDeathState() == CORPSE)
    {
        RemoveCorpse(false);

        if (!isSummon())
        {
            if (timer)
                m_respawnTime = time(nullptr) + timer;

            return;
        }
    }

    m_despawn = false;

    if (getDeathState() == DEAD)
    {
        if (m_DBTableGuid)
            GetMap()->RemoveCreatureRespawnTime(m_DBTableGuid);

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Respawning creature %s (GuidLow: %u, Full GUID: %s entry: %u)", GetName(), GetGUIDLow(), GetGUID().ToString().c_str(), GetEntry());
        m_respawnTime = 0;
        lootForPickPocketed = false;
        lootForBody         = false;

        if (m_originalEntry != GetEntry())
            UpdateEntry(m_originalEntry);

        CreatureTemplate const* cinfo = GetCreatureTemplate();
        SelectLevel(cinfo);

        setDeathState(JUST_RESPAWNED);

        uint32 displayID = GetNativeDisplayId();
        if (GetEntry() == 119047 || GetEntry() == 119046)
            displayID = cinfo->GetRandomValidModelId();
    
        CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelRandomGender(&displayID);
        if (minfo)                                             // Cancel load if no model defined
        {
            SetDisplayId(displayID);
            SetNativeDisplayId(displayID);
            if (IsMirrorImage())
                new MirrorImageUpdate(this);
            SetGender(minfo->gender);
        }

        GetMotionMaster()->InitDefault();

        //Call AI respawn virtual function
        if (IsAIEnabled)
        {
            //reset the AI to be sure no dirty or uninitialized values will be used till next tick
            AI()->Reset();
            TriggerJustRespawned = true;//delay event to next tick so all creatures are created on the map before processing
        }

        uint32 poolid = GetDBTableGUIDLow() ? sPoolMgr->IsPartOfAPool<Creature>(GetDBTableGUIDLow()) : 0;
        if (poolid)
            sPoolMgr->UpdatePool<Creature>(poolid, GetDBTableGUIDLow());

        //Re-initialize reactstate that could be altered by movementgenerators
        InitializeReactState();
    }

    UpdateObjectVisibility();
}

void Creature::ForcedDespawn(uint32 timeMSToDespawn /*= 0*/, Seconds const& forceRespawnTimer /*= Seconds(0)*/)
{
    if (m_despawn || !IsInWorld())
        return;

    if (timeMSToDespawn)
    {
        auto pEvent = new ForcedDespawnDelayEvent(*this, forceRespawnTimer);
        m_Events.AddEvent(pEvent, m_Events.CalculateTime(timeMSToDespawn));
        return;
    }

    if (forceRespawnTimer > Seconds::zero())
    {
        if (isAlive())
        {
            auto respawnDelay = m_respawnDelay;
            auto corpseDelay = m_corpseDelay;
            m_respawnDelay = forceRespawnTimer.count();
            m_corpseDelay = 0;
            setDeathState(JUST_DIED);
            m_respawnDelay = respawnDelay;
            m_corpseDelay = corpseDelay;
        }
        else
        {
            m_corpseRemoveTime = time(nullptr);
            m_respawnTime = time(nullptr) + forceRespawnTimer.count();
        }
    }
    else if (isAlive())
        setDeathState(JUST_DIED);

    RemoveCorpse(false);
}

void Creature::DespawnOrUnsummon(uint32 msTimeToDespawn /*= 0*/, Seconds const& forceRespawnTimer /*= 0*/)
{
    if (!this || m_despawn || !IsInWorld())
        return;

    if (TempSummon* summon = this->ToTempSummon())
        summon->UnSummon(msTimeToDespawn);
    else
        ForcedDespawn(msTimeToDespawn, forceRespawnTimer);

    m_despawn = true;
}

bool Creature::IsImmunedToSpell(SpellInfo const* spellInfo)
{
    if (!spellInfo)
        return false;

    // Creature is immune to main mechanic of the spell
    if (!isPet() && (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->Categories.Mechanic - 1))))
        return true;

    //if (spellInfo->HasAttribute(SPELL_ATTR5_CANT_IMMUNITY_SPELL))
    //    return false;

    // This check must be done instead of 'if (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->Mechanic - 1)))' for not break
    // the check of mechanic immunity on DB (tested) because GetCreatureTemplate()->MechanicImmuneMask and m_spellImmune[IMMUNITY_MECHANIC] don't have same data.
    bool immunedToAllEffects = true;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->EffectMask < uint32(1 << i))
            break;

        if (!spellInfo->GetEffect(i, GetSpawnMode())->IsEffect())
            continue;

        if (!IsImmunedToSpellEffect(spellInfo, i))
        {
            immunedToAllEffects = false;
            break;
        }
    }
    if (immunedToAllEffects)
        return true;

    return Unit::IsImmunedToSpell(spellInfo);
}

bool Creature::IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const
{
    if ((spellInfo->EffectMask & (1 << index)) == 0)
        return false;

    if (!isPet() && (GetCreatureTemplate()->MechanicImmuneMask & (1 << (spellInfo->GetEffect(index, GetSpawnMode())->Mechanic - 1))))
        return true;

    //if (spellInfo->HasAttribute(SPELL_ATTR5_CANT_IMMUNITY_SPELL))
    //    return false;

    if (GetCreatureTemplate()->Type == CREATURE_TYPE_MECHANICAL && spellInfo->GetEffect(index, GetSpawnMode())->Effect == SPELL_EFFECT_HEAL)
        return true;

    return Unit::IsImmunedToSpellEffect(spellInfo, index);
}

bool Creature::isElite() const
{
    if (isPet())
        return false;

    uint32 rank = GetCreatureTemplate()->Classification;
    return rank != CREATURE_CLASSIFICATION_NORMAL && rank != CREATURE_CLASSIFICATION_RARE;
}

bool Creature::isWorldBoss() const
{
    if (isPet())
        return false;

    return GetCreatureTemplate()->isWorldBoss();
}

SpellInfo const* Creature::reachWithSpellAttack(Unit* victim)
{
    if (!victim)
        return nullptr;

    for (auto spellID : m_templateSpells)
    {
        if (!spellID)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_UNITS, "WORLD: unknown spell id %i", spellID);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; j++)
        {
            if (spellInfo->EffectMask < uint32(1 << j))
                break;

            if ((spellInfo->Effects[j]->Effect == SPELL_EFFECT_SCHOOL_DAMAGE) || (spellInfo->Effects[j]->Effect == SPELL_EFFECT_INSTAKILL) || (spellInfo->Effects[j]->Effect == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) || (spellInfo->Effects[j]->Effect == SPELL_EFFECT_HEALTH_LEECH) )
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue)
            continue;

        if (static_cast<uint32>(spellInfo->Power.PowerCost) > static_cast<uint32>(GetPower(POWER_MANA)))
            continue;

        float range = spellInfo->GetMaxRange(false);
        float minrange = spellInfo->GetMinRange(false);
        float dist = GetDistance(victim);
        if (dist > range || dist < minrange)
            continue;

        if (spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;

        if (spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_PACIFY && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            continue;

        return spellInfo;
    }
    return nullptr;
}

SpellInfo const* Creature::reachWithSpellCure(Unit* victim)
{
    if (!victim)
        return nullptr;

    for (auto spellID : m_templateSpells)
    {
        if (!spellID)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_UNITS, "WORLD: unknown spell id %i", spellID);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; j++)
        {
            if (spellInfo->EffectMask < uint32(1 << j))
                break;

            if ((spellInfo->Effects[j]->Effect == SPELL_EFFECT_HEAL))
            {
                bcontinue = false;
                break;
            }
        }

        if (bcontinue)
            continue;

        if (static_cast<uint32>(spellInfo->Power.PowerCost) > static_cast<uint32>(GetPower(POWER_MANA)))
            continue;

        float range = spellInfo->GetMaxRange(true);
        float minrange = spellInfo->GetMinRange(true);
        float dist = GetDistance(victim);

        if (dist > range || dist < minrange)
            continue;

        if (spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;

        if (spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_PACIFY && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            continue;

        return spellInfo;
    }
    return nullptr;
}

// select nearest hostile unit within the given distance (regardless of threat list).
Unit* Creature::SelectNearestTarget(float dist) const
{
    CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Unit* target = nullptr;

    {
        if (dist == 0.0f)
            dist = MAX_VISIBILITY_DISTANCE;

        Trinity::NearestHostileUnitCheck u_check(this, dist);
        Trinity::UnitLastSearcher<Trinity::NearestHostileUnitCheck> searcher(this, target, u_check);

        cell.Visit(p, Trinity::makeWorldVisitor(searcher), *GetMap(), *this, dist);
        cell.Visit(p, Trinity::makeGridVisitor(searcher), *GetMap(), *this, dist);
    }

    return target;
}

// select nearest hostile unit within the given distance and without cc on it (regardless of threat list).
Unit* Creature::SelectNearestTargetNoCC(float dist) const
{
    CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Unit *target = nullptr;

    {
        if (dist == 0.0f)
            dist = MAX_VISIBILITY_DISTANCE;

        Trinity::NearestHostileNoCCUnitCheck u_check(this, dist);
        Trinity::UnitLastSearcher<Trinity::NearestHostileNoCCUnitCheck> searcher(this, target, u_check);

        cell.Visit(p, Trinity::makeWorldVisitor(searcher), *GetMap(), *this, dist);
        cell.Visit(p, Trinity::makeGridVisitor(searcher), *GetMap(), *this, dist);
    }

    return target;
}

// select nearest hostile unit within the given attack distance (i.e. distance is ignored if > than ATTACK_DISTANCE), regardless of threat list.
Unit* Creature::SelectNearestTargetInAttackDistance(float dist) const
{
    CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Unit* target = nullptr;

    if (dist > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature (GUID: %u entry: %u) SelectNearestTargetInAttackDistance called with dist > MAX_VISIBILITY_DISTANCE. Distance set to ATTACK_DISTANCE.", GetGUIDLow(), GetEntry());
        dist = ATTACK_DISTANCE;
    }

    {
        Trinity::NearestHostileUnitInAttackDistanceCheck u_check(this, dist);
        Trinity::UnitLastSearcher<Trinity::NearestHostileUnitInAttackDistanceCheck> searcher(this, target, u_check);

        cell.Visit(p, Trinity::makeWorldVisitor(searcher), *GetMap(), *this, ATTACK_DISTANCE > dist ? ATTACK_DISTANCE : dist);
        cell.Visit(p, Trinity::makeGridVisitor(searcher), *GetMap(), *this, ATTACK_DISTANCE > dist ? ATTACK_DISTANCE : dist);
    }

    return target;
}

Player* Creature::SelectNearestPlayer(float distance) const
{
    Player* target = nullptr;

    Trinity::NearestPlayerInObjectRangeCheck checker(this, distance);
    Trinity::PlayerLastSearcher<Trinity::NearestPlayerInObjectRangeCheck> searcher(this, target, checker);
    Trinity::VisitNearbyObject(this, distance, searcher);

    return target;
}

Player* Creature::SelectNearestPlayerNotGM(float distance) const
{
    Player* target = nullptr;

    Trinity::NearestPlayerNotGMInObjectRangeCheck checker(this, distance);
    Trinity::PlayerLastSearcher<Trinity::NearestPlayerNotGMInObjectRangeCheck> searcher(this, target, checker);
    Trinity::VisitNearbyObject(this, distance, searcher);

    return target;
}

void Creature::SendAIReaction(AiReaction reactionType)
{
    WorldPackets::Combat::AIReaction packet;
    packet.UnitGUID = GetGUID();
    packet.Reaction = reactionType;
    SendMessageToSet(packet.Write(), true);
}

void Creature::CallAssistance()
{
    if (!IsAlreadyCallAssistance() && getVictim() && !isPet() && !isCharmed())
    {
        SetNoCallAssistance(true);

        float radius = sWorld->getFloatConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS);

        if (radius > 0.f)
        {
            std::list<Creature*> assistList;

            if (GetMap())
            {
                CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
                Cell cell(p);
                cell.SetNoCreate();

                Trinity::AnyAssistCreatureInRangeCheck u_check(this, getVictim(), radius);
                Trinity::CreatureListSearcher<Trinity::AnyAssistCreatureInRangeCheck> searcher(this, assistList, u_check);

                cell.Visit(p, Trinity::makeGridVisitor(searcher), *GetMap(), *this, radius);
            }

            if (!assistList.empty() && getVictim())
            {
                auto e = new AssistDelayEvent(getVictim()->GetGUID(), *this);
                while (!assistList.empty())
                {
                    // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
                    e->AddAssistant((*assistList.begin())->GetGUID());
                    assistList.pop_front();
                }
                m_Events.AddEvent(e, m_Events.CalculateTime(sWorld->getIntConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY)));
            }
        }
    }
}

void Creature::CallForHelp(float radius)
{
    if (radius <= 0.0f || !getVictim() || isPet() || isCharmed())
        return;

    CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    cell.SetNoCreate();

    Trinity::CallOfHelpCreatureInRangeDo u_do(this, getVictim(), radius);
    Trinity::CreatureWorker<Trinity::CallOfHelpCreatureInRangeDo> worker(this, u_do);

    cell.Visit(p, Trinity::makeGridVisitor(worker), *GetMap(), *this, radius);
}

bool Creature::CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction /*= true*/) const
{
    // is it true?
    if (!HasReactState(REACT_AGGRESSIVE))
        return false;

    // we don't need help from zombies :)
    if (!isAlive())
        return false;

    // we don't need help from non-combatant ;)
    if (isCivilian())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC))
        return false;

    // skip fighting creature
    if (isInCombat())
        return false;

    // only free creature
    if (GetCharmerOrOwnerGUID())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getFaction() != u->getFaction())
            return false;
    }
    else
    {
        if (!IsFriendlyTo(u))
            return false;
    }

    // skip non hostile to caster enemy creatures
    if (!IsHostileTo(enemy))
        return false;

    return true;
}

// use this function to avoid having hostile creatures attack
// friendlies and other mobs they shouldn't attack
bool Creature::_IsTargetAcceptable(const Unit* target) const
{
    if (!IsInWorld() || !target || !target->IsInWorld() || IsDespawn())
        return false;

    Unit* veh = GetVehicleBase();
    // if the target cannot be attacked, the target is not acceptable
    if (IsFriendlyTo(target)
        || !target->isTargetableForAttack(false)
        || (veh && (IsOnVehicle(target)
        || veh->IsOnVehicle(target))))
        return false;

    if (target->HasUnitState(UNIT_STATE_DIED))
    {
        // guards can detect fake death
        return isGuard() && target->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
    }

    const Unit* myVictim = getAttackerForHelper();
    const Unit* targetVictim = target->getAttackerForHelper();

    // if I'm already fighting target, or I'm hostile towards the target, the target is acceptable
    if (myVictim == target || targetVictim == this || IsHostileTo(target))
        return true;

    // if the target's victim is friendly, and the target is neutral, the target is acceptable
    if (targetVictim && IsFriendlyTo(targetVictim))
        return true;

    // if the target's victim is not friendly, or the target is friendly, the target is not acceptable
    return false;
}

void Creature::StopAttack(bool clearMove /*= false*/, bool interruptSpells /*= false*/)
{
    if (!isMoving())
        Movement::MoveSplineInit(*this).Stop(false, true);

    SetReactState(REACT_PASSIVE);
    AttackStop();

    if (interruptSpells)
        InterruptNonMeleeSpells(true);

    if (!clearMove)
        GetMotionMaster()->MoveIdle();
    else
        GetMotionMaster()->Clear();
}

void Creature::SaveRespawnTime()
{
    if (isSummon() || !m_DBTableGuid || (m_creatureData && !m_creatureData->dbData))
        return;

    GetMap()->SaveCreatureRespawnTime(m_DBTableGuid, m_respawnTime);
}

// this should not be called by petAI or
bool Creature::canCreatureAttack(Unit const* victim, bool /*force*/) const
{
    if (!victim->IsInMap(this) || IsNeverVisible())
        return false;

    if (!IsValidAttackTarget(victim))
        return false;

    if (!victim->isInAccessiblePlaceFor(this))
        return false;

    if (IsAIEnabled && !AI()->CanAIAttack(victim))
        return false;

    if (sMapStore.LookupEntry(GetMapId())->IsDungeon())
        return true;

    //Use AttackDistance in distance check if threat radius is lower. This prevents creature bounce in and out of combat every update tick.
    float dist = std::max(GetAttackDistance(victim), sWorld->getFloatConfig(CONFIG_THREAT_RADIUS)) + m_CombatDistance;

    switch (GetEntry())
    {
        //The August Celestials
        case 71952:
        case 71953:
        case 71955:
        case 71954:
            dist = 130.0f;
            break;
        case 118277:
        case 118271:
            dist = 150.0f;
            break;
        default:
            break;
    }

    if (Unit* unit = GetCharmerOrOwner())
        return victim->IsWithinDist(unit, dist);

    return victim->IsInDist(&m_homePosition, dist);
}

CreatureAddon const* Creature::GetCreatureAddon() const
{
    if (m_DBTableGuid)
    {
        if (CreatureAddon const* addon = sObjectMgr->GetCreatureAddon(m_DBTableGuid))
            return addon;
    }

    // dependent from difficulty mode entry
    return sObjectMgr->GetCreatureTemplateAddon(GetCreatureTemplate()->Entry);
}

//creature_addon table
bool Creature::LoadCreaturesAddon(bool /*reload*/)
{
    CreatureAddon const* cainfo = GetCreatureAddon();
    if (!cainfo)
        return false;

    if (cainfo->mount != 0)
        Mount(cainfo->mount);

    if (cainfo->bytes1 != 0)
    {
        // 0 StandState
        // 1 FreeTalentPoints   Pet only, so always 0 for default creature
        // 2 StandFlags
        // 3 StandMiscFlags

        setStandStateValue(cainfo->bytes1 & 0xFF);
        //SetByteValue(UNIT_FIELD_BYTES_1, 1, uint8((cainfo->bytes1 >> 8) & 0xFF));
        SetByteValue(UNIT_FIELD_BYTES_1, 1, 0);
        SetStandVisValue(uint8(cainfo->bytes1 >> 16) & 0xFF);
        SetMiscStandValue(uint8((cainfo->bytes1 >> 24) & 0xFF));

        //! Suspected correlation between UNIT_FIELD_BYTES_1, offset 3, value 0x2:
        //! If no inhabittype_fly (if no MovementFlag_DisableGravity flag found in sniffs)
        //! Set MovementFlag_Hover. Otherwise do nothing.
        if ((GetMiscStandValue() & UNIT_BYTE1_FLAG_HOVER) && isAlive())
            SetHover(true);
    }

    if (cainfo->bytes2 != 0)
    {
        // 0 SheathState
        // 1 Bytes2Flags
        // 2 UnitRename         Pet only, so always 0 for default creature
        // 3 ShapeshiftForm     Must be determined/set by shapeshift spell/aura

        SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE, uint8(cainfo->bytes2 & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, uint8((cainfo->bytes2 >> 8) & 0xFF));
        //SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, uint8((cainfo->bytes2 >> 16) & 0xFF));
        SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, 0);
        //SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM, uint8((cainfo->bytes2 >> 24) & 0xFF));
        SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM, 0);
    }

    if (cainfo->emote != 0)
        SetUInt32Value(UNIT_FIELD_EMOTE_STATE, cainfo->emote);

    //Load Path
    if (cainfo->path_id != 0)
        m_path_id = cainfo->path_id;

    if (!cainfo->auras.empty())
    {
        for (auto aura : cainfo->auras)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(aura);
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: %u entry: %u) has wrong spell %u defined in `auras` field.", GetGUIDLow(), GetEntry(), aura);
                continue;
            }

            // skip already applied aura
            if (HasAura(aura))
            {
                //if (!reload)
                    //TC_LOG_ERROR(LOG_FILTER_SQL, "Creature (GUID: %u Entry: %u) has duplicate aura (spell %u) in `auras` field.", GetGUIDLow(), GetEntry(), *itr);

                continue;
            }

            if (spellInfo->IsChanneled())
                AddDelayedEvent(100, [this, spellInfo]() -> void { CastSpell(this, spellInfo, true); });
            else
                AddAura(aura, this);
            TC_LOG_DEBUG(LOG_FILTER_UNITS, "Spell: %u added to creature (GUID: %u entry: %u GetGUID %s)", aura, GetGUIDLow(), GetEntry(), GetGUID().ToString().c_str());
        }
    }

    return true;
}

void Creature::SendZoneUnderAttackMessage(Player* attacker)
{
    uint32 enemy_team = attacker->GetTeam();

    WorldPackets::Misc::ZoneUnderAttack unerAttack;
    unerAttack.AreaID = GetAreaId();
    sWorld->SendGlobalMessage(unerAttack.Write(), nullptr, (enemy_team == ALLIANCE ? HORDE : ALLIANCE));
}

void Creature::SetInCombatWithZone()
{
    if (!CanHaveThreatList())
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature entry %u call SetInCombatWithZone but creature cannot have threat list.", GetEntry());
        return;
    }

    Map* map = GetMap();
    if (!map->IsDungeon())
    {
        TC_LOG_ERROR(LOG_FILTER_UNITS, "Creature entry %u call SetInCombatWithZone for map (id: %u) that isn't an instance.", GetEntry(), map->GetId());
        return;
    }

    map->ApplyOnEveryPlayer([&](Player* player)
    {
        if (!player->isGameMaster() && player->isAlive() && (!player->getHostileRefManager().HasTarget(this) || !player->isInCombat()))
        {
            this->SetInCombatWith(player);
            player->SetInCombatWith(this);
            AddThreat(player, 0.0f);
        }
    });
}

void Creature::_AddCreatureSpellCooldown(uint32 spell_id, time_t end_time)
{
    m_CreatureSpellCooldowns[spell_id] = end_time;
}

void Creature::AddSpellCooldown(uint32 spell_id, uint32 /*itemid*/, double end_time)
{
    m_CreatureProcCooldowns[spell_id] = end_time;
}

void Creature::_AddCreatureCategoryCooldown(uint32 category, time_t apply_time)
{
    m_CreatureCategoryCooldowns[category] = apply_time;
}

void Creature::AddCreatureSpellCooldown(uint32 spellid)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return;

    uint32 baseCD = 6000; //for pet prevented spamm spell

    if(spellInfo->CalcCastTime() || spellInfo->Power.PowerType < MAX_POWERS)
        baseCD = 0;

    uint32 cooldown = spellInfo->GetRecoveryTime();

    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellid, SPELLMOD_COOLDOWN, cooldown);

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "Creature::AddCreatureSpellCooldown cooldown %i, baseCD %i", cooldown, baseCD);

    if (cooldown)
        _AddCreatureSpellCooldown(spellid, time(nullptr) + cooldown/IN_MILLISECONDS);
    else if(baseCD)
        _AddCreatureSpellCooldown(spellid, time(nullptr) + baseCD/IN_MILLISECONDS);

    if (spellInfo->Categories.Category)
        _AddCreatureCategoryCooldown(spellInfo->Categories.Category, time(nullptr));
}

bool Creature::HasCategoryCooldown(uint32 spell_id) const
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
    if (!spellInfo)
        return false;

    auto itr = m_CreatureCategoryCooldowns.find(spellInfo->Categories.Category);
    return(itr != m_CreatureCategoryCooldowns.end() && time_t(itr->second + (spellInfo->Cooldowns.CategoryRecoveryTime / IN_MILLISECONDS)) > time(nullptr));
}

bool Creature::HasSchoolMaskCooldown(SpellSchoolMask schoolMask) const
{
    for (uint8 i = 0; (1 << i) < SPELL_SCHOOL_MASK_ALL; ++i)
    {
        if ((1 << i) & schoolMask)
        {
            auto itr = m_CreatureSchoolCooldowns.find((1 << i));
            if (itr == m_CreatureSchoolCooldowns.end())
                continue;

            if ((*itr).second >= time(NULL))
                return true;
        }
    }
    return false;
}

uint32 Creature::_GetSpellCooldownDelay(uint32 spell_id) const
{
    auto itr = m_CreatureSpellCooldowns.find(spell_id);
    time_t t = time(nullptr);
    return uint32(itr != m_CreatureSpellCooldowns.end() && itr->second > t ? itr->second - t : 0);
}

void Creature::RemoveCreatureSpellCooldown(uint32 spell_id)
{
    auto itr = m_CreatureSpellCooldowns.find(spell_id);
    if (itr != m_CreatureSpellCooldowns.end())
        m_CreatureSpellCooldowns.erase(itr);
}

bool Creature::HasCreatureSpellCooldown(uint32 spell_id) const
{
    auto itr = m_CreatureSpellCooldowns.find(spell_id);
    return (itr != m_CreatureSpellCooldowns.end() && itr->second > time(nullptr)) || HasCategoryCooldown(spell_id);
}

bool Creature::HasSpellCooldown(uint32 spell_id)
{
    CreatureSpellCooldowns::const_iterator itr = m_CreatureProcCooldowns.find(spell_id);
    return (itr != m_CreatureProcCooldowns.end() && itr->second > time(nullptr));
}

bool Creature::HasSpell(uint32 spellID)
{
    uint8 i;
    for (i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if (spellID == m_templateSpells[i])
            break;
    return i < CREATURE_MAX_SPELLS;                         //broke before end of iteration of known spells
}

time_t Creature::GetRespawnTimeEx() const
{
    time_t now = time(nullptr);
    if (m_respawnTime > now)
        return m_respawnTime;
    return now;
}

void Creature::GetRespawnPosition(float &x, float &y, float &z, float* ori, float* dist) const
{
    if (m_DBTableGuid)
    {
        if (CreatureData const* data = sObjectMgr->GetCreatureData(GetDBTableGUIDLow()))
        {
            x = data->posX;
            y = data->posY;
            z = data->posZ;
            if (ori)
                *ori = data->orientation;
            if (dist)
                *dist = data->spawndist;

            return;
        }
    }

    x = GetPositionX();
    y = GetPositionY();
    z = GetPositionZ();
    if (ori)
        *ori = GetOrientation();
    if (dist)
        *dist = 0;
}

void Creature::AllLootRemovedFromCorpse()
{
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        time_t now = time(nullptr);
        if (m_corpseRemoveTime <= now)
            return;

        CreatureTemplate const* cinfo = GetCreatureTemplate();

        float decayRate = sWorld->getRate(RATE_CORPSE_DECAY_LOOTED);
        auto diff = uint32((m_corpseRemoveTime - now) * decayRate);

        m_respawnTime -= diff;

        // corpse skinnable, but without skinning flag, and then skinned, corpse will despawn next update
        if (cinfo && cinfo->SkinLootId)
            m_corpseRemoveTime = time(nullptr);
        else
            m_corpseRemoveTime -= diff;
    }
}

std::string Creature::GetAIName() const
{
    return sObjectMgr->GetCreatureTemplate(GetEntry())->AIName;
}

std::string Creature::GetScriptName() const
{
    return sObjectMgr->GetScriptName(GetScriptId());
}

uint32 Creature::GetScriptId() const
{
    return sObjectMgr->GetCreatureTemplate(GetEntry())->ScriptID;
}

VendorItemData const* Creature::GetVendorItems() const
{
    return sObjectMgr->GetNpcVendorItemList(GetEntry());
}

uint32 Creature::GetVendorItemCurrentCount(VendorItem const* vItem)
{
    if (!vItem->maxcount)
        return vItem->maxcount;

    auto itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
        return vItem->maxcount;

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(nullptr);

    if (time_t(vCount->lastIncrementTime + vItem->incrtime) <= ptime)
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(vItem->item);

        auto diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->VendorStackCount) >= vItem->maxcount)
        {
            m_vendorItemCounts.erase(itr);
            return vItem->maxcount;
        }

        vCount->count += diff * pProto->VendorStackCount;
        vCount->lastIncrementTime = ptime;
    }

    return vCount->count;
}

uint32 Creature::UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count)
{
    if (!vItem->maxcount)
        return 0;

    auto itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
    {
        uint32 new_count = vItem->maxcount > used_count ? vItem->maxcount-used_count : 0;
        m_vendorItemCounts.push_back(VendorItemCount(vItem->item, new_count));
        return new_count;
    }

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(nullptr);

    if (time_t(vCount->lastIncrementTime + vItem->incrtime) <= ptime)
    {
        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(vItem->item);

        auto diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->VendorStackCount) < vItem->maxcount)
            vCount->count += diff * pProto->VendorStackCount;
        else
            vCount->count = vItem->maxcount;
    }

    vCount->count = vCount->count > used_count ? vCount->count-used_count : 0;
    vCount->lastIncrementTime = ptime;
    return vCount->count;
}

TrainerSpellData const* Creature::GetTrainerSpells() const
{
    return sObjectMgr->GetNpcTrainerSpells(GetEntry());
}

// overwrite WorldObject function for proper name localization
const char* Creature::GetNameForLocaleIdx(LocaleConstant locale) const
{
    if (locale != DEFAULT_LOCALE && locale != LOCALE_none)
    {
        auto localeID = uint8(locale);
        if (CreatureLocale const* cl = sObjectMgr->GetCreatureLocale(GetEntry()))
            if (cl->Name[0].size() > localeID && !cl->Name[0][localeID].empty())
                return cl->Name[0][localeID].c_str();
    }

    return GetName();
}

//Do not if this works or not, moving creature to another map is very dangerous
void Creature::FarTeleportTo(Map* map, float X, float Y, float Z, float O)
{
    CleanupBeforeRemoveFromMap(false);
    GetMap()->RemoveFromMap(this, false);
    Relocate(X, Y, Z, O);
    SetMap(map);
    GetMap()->AddToMap(this);
}

void Creature::SetPosition(float x, float y, float z, float o)
{
    // prevent crash when a bad coord is sent by the client
    if (!Trinity::IsValidMapCoord(x, y, z, o))
    {
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Creature::SetPosition(%f, %f, %f) .. bad coordinates!", x, y, z);
        return;
    }

    GetMap()->CreatureRelocation(ToCreature(), x, y, z, o);
    if (IsVehicle())
        GetVehicleKit()->RelocatePassengers();
}

void Creature::SetPosition(Position const& pos)
{
    SetPosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
}

void Creature::SetHomePosition(float x, float y, float z, float o)
{
    m_homePosition.Relocate(x, y, z, o);
}

void Creature::SetHomePosition(Position const& pos)
{
    m_homePosition.Relocate(pos);
}

void Creature::GetHomePosition(float& x, float& y, float& z, float& ori) const
{
    m_homePosition.GetPosition(x, y, z, ori);
}

Position Creature::GetHomePosition() const
{
    return m_homePosition;
}

void Creature::SetTransportHomePosition(float x, float y, float z, float o)
{
    m_transportHomePosition.Relocate(x, y, z, o);
}

void Creature::SetTransportHomePosition(Position const& pos)
{
    m_transportHomePosition.Relocate(pos);
}

void Creature::GetTransportHomePosition(float& x, float& y, float& z, float& ori)
{
    m_transportHomePosition.GetPosition(x, y, z, ori);
}

Position Creature::GetTransportHomePosition()
{
    return m_transportHomePosition;
}

bool Creature::IsDungeonBoss() const
{
    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(GetEntry());
    return cinfo && (cinfo->flags_extra & (CREATURE_FLAG_EXTRA_DUNGEON_BOSS | CREATURE_FLAG_EXTRA_INSTANCE_BIND));
}

bool Creature::IsPersonal() const
{
    return (GetTrackingQuestID() || isWorldBoss() || (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_PERSONAL_LOOT || GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_EVENT_NPC)) &&
        GetCreatureTemplate()->RequiredExpansion > EXPANSION_WARLORDS_OF_DRAENOR;
}

bool Creature::IsPersonalForQuest(Player const* player) const
{
    if(GetCreatureTemplate()->QuestPersonalLoot)
        return player->HasQuestForCreature(GetCreatureTemplate());
    return false;
}

bool Creature::IsEventCreature() const
{
    return GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_EVENT_NPC;
}

void Creature::CalculateMoney(uint32& mingold, uint32& maxgold)
{
    if (m_difficulty == 0 && (GetCreatureTemplate()->maxlevel - GetCreatureTemplate()->minlevel) > 15) // Check gold for heroic and normal diff
    {
        mingold /= 10;
        maxgold /= 10;
    }
    else if (GetCreatureTemplate()->RequiredExpansion <= EXPANSION_MISTS_OF_PANDARIA && GetMap()->Instanceable() && GetMap()->GetMapMaxPlayers())
    {
        mingold /= GetMap()->GetMapMaxPlayers();
        maxgold /= GetMap()->GetMapMaxPlayers();
    }
}

void Creature::ClearLootList()
{
    for (auto const& _guid : lootList)
        if (Player* looter = ObjectAccessor::GetPlayer(*this, _guid))
            looter->RemoveLoot(GetGUID());

    rewardedPlayer.clear();
    lootList.clear();
    loot.clear();
}

bool Creature::HasInLootList(ObjectGuid targetGuid) const
{
    for (auto const& _guid : lootList)
        if (targetGuid == _guid)
            return true;

    return false;
}

bool Creature::IsPlayerRewarded(ObjectGuid targetGuid) const
{
    auto it = rewardedPlayer.find(targetGuid);
    return it != rewardedPlayer.end();
}

void Creature::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    for (uint8 i = 0; i < GetPetCastSpellSize(); ++i)
    {
        uint32 spellID = GetPetCastSpellOnPos(i);
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
        if (!spellInfo)
            continue;

        // Not send cooldown for this spells
        if (spellInfo->HasAttribute(SPELL_ATTR0_DISABLED_WHILE_ACTIVE))
            continue;

        if (!(spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE))
            continue;

        if ((idSchoolMask & spellInfo->GetSchoolMask()) && _GetSpellCooldownDelay(spellID) < unTimeMs)
            _AddCreatureSpellCooldown(spellID, time(nullptr) + unTimeMs/IN_MILLISECONDS);
    }

    for (uint8 i = 0; (1 << i) < SPELL_SCHOOL_MASK_ALL; ++i)
        if ((1 << i) & idSchoolMask)
            m_CreatureSchoolCooldowns[(1 << i)] = time(nullptr) + unTimeMs / IN_MILLISECONDS;
}

bool Creature::IsNoDamage() const
{
    return (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_HP_85_PERC) || (m_actionData[0] && !m_actionData[0]->empty());
}

MirrorImageUpdate::MirrorImageUpdate(Creature* creature) : creature(creature)
{
    static uint32 delay = 1;
    creature->m_Events.AddEvent(this, creature->m_Events.CalculateTime(delay));
    creature->SetDisplayId(11686); // invisible in beginning if a mirror image
    creature->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
}

bool MirrorImageUpdate::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    // From AuraEffect::HandleAuraCloneCaster
    int32 outfitId = creature->GetOutfit();
    if (outfitId < 0)
    {
        const CreatureOutfitContainer& outfits = sObjectMgr->GetCreatureOutfitMap();
        auto it = outfits.find(-outfitId);
        if (it != outfits.end())
        {
            creature->SetDisplayId(it->second.displayId);
            creature->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
            return true;
        }
    }
    creature->SetDisplayId(creature->GetNativeDisplayId());
    creature->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
    return true;
}

uint32 Creature::GetScalingID()
{
    // Zone
    if (WorldMapAreaEntry const* wma = sDB2Manager.GetWorldMapArea(m_zoneId))
    {
        if (wma->LevelRangeMax <= 100) // Limit by WOD
        {
            ScaleLevelMin = wma->LevelRangeMin;
            ScaleLevelMax = wma->LevelRangeMax;
            return sDB2Manager.GetScalingByLevel(ScaleLevelMin, ScaleLevelMax);
        }
    }

    // Dungeon
    if (auto const& dungeons = sDB2Manager.GetLFGDungeonsByMapDIff(GetMapId(), m_spawnMode))
    {
        if (dungeons->MaxLevel <= 100) // Limit by WOD
        {
            ScaleLevelMin = dungeons->MinLevel;
            ScaleLevelMax = dungeons->MaxLevel;
            return sDB2Manager.GetScalingByLevel(ScaleLevelMin, ScaleLevelMax);
        }
    }

    return 0;
}

CreatureSpell const* Creature::GetCreatureSpell(uint32 SpellID)
{
    auto spells = CreatureSpells->find(SpellID);
    if (spells != CreatureSpells->end())
        return &spells->second;
    return nullptr;
}

bool Creature::DistanceCheck()
{
    return !MaxVisible;
}

bool CreatureTemplate::isWorldBoss() const
{
    return TypeFlags[0] & CREATURE_TYPEFLAGS_BOSS || Classification == CREATURE_CLASSIFICATION_WORLDBOSS;
}
