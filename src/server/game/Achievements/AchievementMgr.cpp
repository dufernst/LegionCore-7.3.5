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

#include "AchievementMgr.h"
#include "AchievementPackets.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Bracket.h"
#include "CellImpl.h"
#include "ChatTextBuilder.h"
#include "Common.h"
#include "CollectionMgr.h"
#include "DatabaseEnv.h"
#include "DatabaseEnvFwd.h"
#include "DBCEnums.h"
#include "DisableMgr.h"
#include "Formulas.h"
#include "GameEventMgr.h"
#include "Garrison.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "InstanceScript.h"
#include "Language.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PetBattle.h"
#include "ScenarioMgr.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "ObjectVisitors.hpp"
#include "WorldStateMgr.h"

CriteriaProgress::CriteriaProgress() : Counter(0), date(0), changed(false), updated(false), completed(false), deactiveted(true)
, deleted{false}, achievement(nullptr), criteriaTree(nullptr), parent(nullptr), criteria(nullptr), _criteria(nullptr)
{
}

AchievementCache::AchievementCache(Player* _player, Unit* _target, CriteriaTypes _type, uint32 _miscValue1, uint32 _miscValue2, uint32 _miscValue3)
{
    player = _player;
    type = _type;
    miscValue1 = _miscValue1;
    miscValue2 = _miscValue2;
    miscValue3 = _miscValue3;
    ClassID = player->getClass();
    RaceID = player->getRace();
    Gender = player->getGender();
    HealthPct = player->GetHealthPct();
    Level = player->getLevelForTarget(_target);
    MapID = player->GetMapId();
    ZoneID = player->GetCurrentZoneID();
    AreaID = player->GetCurrentAreaID();
    InstanceId = player->GetInstanceId();
    Team = player->GetTeam();
    TeamID = player->GetTeamId();
    DrunkValue = player->GetDrunkValue();
    guildId = player->GetGuildId();

    if (Map* map = player->GetMap())
    {
        if (map->Instanceable())
            PlayersCount = map->GetPlayersCountExceptGMs();

        Difficulty = map->GetDifficultyID();
    }

    if (Group* group = player->GetGroup())
    {
        MembersCount = player->GetGroup()->GetMembersCount();
        HaveGroup = true;
        isLFGGroup = group->isLFGGroup();
        IsGuildGroup = group->IsGuildGroup();
    }

    if (Battleground* bg = player->GetBattleground())
    {
        OnBG = true;
        IsArena = bg->IsArena();
        IsRated = bg->IsRated();
        JoinType = bg->GetJoinType();
        IsRBG = bg->IsRBG();
    }
    switch (type)
    {
        case CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
        case CRITERIA_TYPE_LEARN_SKILL_LINE:
        {
            MountCount = player->GetCollectionMgr()->GetAccountMountsCount();
            BattleCount = player->GetBattlePets()->size();
            break;
        }
        case CRITERIA_TYPE_OWN_TOY_COUNT:
        {
            ToysCount = player->GetCollectionMgr()->GetAccountToysCount();
            break;
        }
        case CRITERIA_TYPE_COLLECT_BATTLEPET:
        {
            std::set<uint32> _uniquPets;
            BattlePetMap* pets = player->GetBattlePets();
            for (auto & pet : *pets)
                _uniquPets.insert(pet.second->Species);

            UniquePets = _uniquPets.size();
            break;
        }
        case CRITERIA_TYPE_KILL_CREATURE_TYPE:
        {
            if (_target)
                target.isYields = Trinity::XP::Gain(_player, _target);
            break;
        }
        default:
            break;
    }
    if (_target)
    {
        target.unit = _target;
        target.Entry = _target->GetEntry();
        target.ClassID = _target->getClass();
        target.RaceID = _target->getRace();
        target.Level = _target->getLevelForTarget(player);
        target.Gender = _target->getGender();
        target.HealthPct = _target->GetHealthPct();
        target.isAlive = _target->isAlive();
        target.IsMounted = _target->IsMounted();
        target.ZoneID = _target->GetCurrentZoneID();
        target.AreaID = _target->GetCurrentAreaID();
        if (_target->ToPlayer())
            target.Team = _target->ToPlayer()->GetTeam();
        target.isCreature = _target->IsCreature();
        target.Init = true;
    }
}

AchievementCriteriaData::AchievementCriteriaData() : dataType(ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE)
{
    raw.value1 = 0;
    raw.value2 = 0;
    ScriptId = 0;
}

AchievementCriteriaData::AchievementCriteriaData(uint32 _dataType, uint32 _value1, uint32 _value2, uint32 _scriptId) : dataType(AchievementCriteriaDataType(_dataType))
{
    raw.value1 = _value1;
    raw.value2 = _value2;
    ScriptId = _scriptId;
}

bool AchievementCriteriaData::IsValid(CriteriaEntry const* criteria)
{
    if (dataType >= MAX_ACHIEVEMENT_CRITERIA_DATA_TYPE)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` for criteria (Entry: %u) has wrong data type (%u), ignored.", criteria->ID, dataType);
        return false;
    }

    switch (criteria->Type)
    {
        case CRITERIA_TYPE_KILL_CREATURE:
        case CRITERIA_TYPE_WIN_BG:
        case CRITERIA_TYPE_WIN_BATTLEGROUND:
        case CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case CRITERIA_TYPE_COMPLETE_QUEST:          // only hardcoded list
        case CRITERIA_TYPE_CAST_SPELL:
        case CRITERIA_TYPE_WIN_RATED_ARENA:
        case CRITERIA_TYPE_DO_EMOTE:
        case CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case CRITERIA_TYPE_WIN_DUEL:
        case CRITERIA_TYPE_LOOT_TYPE:
        case CRITERIA_TYPE_CAST_SPELL2:
        case CRITERIA_TYPE_BE_SPELL_TARGET:
        case CRITERIA_TYPE_BE_SPELL_TARGET2:
        case CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        case CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case CRITERIA_TYPE_HONORABLE_KILL:
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST:    // only Children's Week achievements
        case CRITERIA_TYPE_USE_ITEM:                // only Children's Week achievements
        case CRITERIA_TYPE_GET_KILLING_BLOWS:
        case CRITERIA_TYPE_REACH_LEVEL:
        case CRITERIA_TYPE_EXPLORE_AREA:
            break;
        default:
            if (dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` has data for non-supported criteria type (Entry: %u Type: %u), ignored.", criteria->ID, criteria->Type);
                return false;
            }
            break;
    }

    switch (dataType)
    {
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE:
        case ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT:
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE:
            if (!creature.id || !sObjectMgr->GetCreatureTemplate(creature.id))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_CREATURE (%u) has non-existing creature id in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, creature.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE:
            if (classRace.class_id && ((1 << (classRace.class_id - 1)) & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) has non-existing class in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.class_id);
                return false;
            }
            if (classRace.race_id && ((1 << (classRace.race_id - 1)) & RACEMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE (%u) has non-existing race in value2 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.race_id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH:
            if (health.percent < 1 || health.percent > 100)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_PLAYER_LESS_HEALTH (%u) has wrong percent value in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, health.percent);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA:
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA:
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(aura.spell_id);
            if (!spellEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has wrong spell id in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA ? "ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA" : "ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.spell_id);
                return false;
            }
            if (aura.effect_idx >= 3)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has wrong spell effect index in value2 (%u), ignored.",
                    criteria->ID, criteria->Type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA ? "ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA" : "ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.effect_idx);
                return false;
            }
            if (!spellEntry->Effects[aura.effect_idx]->ApplyAuraName)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type %s (%u) has non-aura spell effect (ID: %u Effect: %u), ignores.",
                    criteria->ID, criteria->Type, (dataType == ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA ? "ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA" : "ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA"), dataType, aura.spell_id, aura.effect_idx);
                return false;
            }
            return true;
        }
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AREA:
            if (!sAreaTableStore.LookupEntry(area.id))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AREA (%u) has wrong area id in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, area.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL:
            if (level.minlevel > STRONG_MAX_LEVEL)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL (%u) has wrong minlevel in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, level.minlevel);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER:
            if (gender.gender > GENDER_NONE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER (%u) has wrong gender in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, gender.gender);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT:
            if (!ScriptId)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT (%u) does not have ScriptName set, ignored.",
                    criteria->ID, criteria->Type, dataType);
                return false;
            }
            return true;
            /*Todo:
            case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_DIFFICULTY:
            */
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT:
            if (map_players.maxcount <= 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT (%u) has wrong max players count in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, map_players.maxcount);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM:
            if (team.team != ALLIANCE && team.team != HORDE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM (%u) has unknown team in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, team.team);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK:
            if (drunk.state >= MAX_DRUNKEN)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK (%u) has unknown drunken state in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, drunk.state);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY:
            if (!sHolidaysStore.LookupEntry(holiday.id))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY (%u) has unknown holiday in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, holiday.id);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE:
            return true;                                    // not check correctness node indexes
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM:
            if (equipped_item.item_quality >= MAX_ITEM_QUALITY)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_requirement` (Entry: %u Type: %u) for requirement ACHIEVEMENT_CRITERIA_REQUIRE_S_EQUIPED_ITEM (%u) has unknown quality state in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, equipped_item.item_quality);
                return false;
            }
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE:
            if (!classRace.class_id && !classRace.race_id)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) must not have 0 in either value field, ignored.",
                    criteria->ID, criteria->Type, dataType);
                return false;
            }
            if (classRace.class_id && ((1 << (classRace.class_id - 1)) & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) has non-existing class in value1 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.class_id);
                return false;
            }
            if (classRace.race_id && ((1 << (classRace.race_id - 1)) & RACEMASK_ALL_PLAYABLE) == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) for data type ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE (%u) has non-existing race in value2 (%u), ignored.",
                    criteria->ID, criteria->Type, dataType, classRace.race_id);
                return false;
            }
            return true;
        default:
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` (Entry: %u Type: %u) has data for non-supported data type (%u), ignored.", criteria->ID, criteria->Type, dataType);
            return false;
    }
}

bool AchievementCriteriaData::Meets(uint32 criteria_id, AchievementCachePtr cachePtr) const
{
    switch (dataType)
    {
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE:
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE:
            if (!cachePtr->HasTarget() || !cachePtr->IsCreature())
                return false;
            return cachePtr->GetEntry() == creature.id;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE:
            if (!cachePtr->HasTarget() || cachePtr->IsCreature())
                return false;
            if (classRace.class_id && classRace.class_id != cachePtr->target.ClassID)
                return false;
            if (classRace.race_id && classRace.race_id != cachePtr->target.RaceID)
                return false;
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE:
            if (classRace.class_id && classRace.class_id != cachePtr->ClassID)
                return false;
            if (classRace.race_id && classRace.race_id != cachePtr->RaceID)
                return false;
            return true;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH:
            if (!cachePtr->HasTarget() || cachePtr->IsCreature())
                return false;
            return cachePtr->target.HealthPct > health.percent;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA:
            return cachePtr->player->HasAura(aura.spell_id);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AREA:
            return area.id == cachePtr->ZoneID || area.id == cachePtr->AreaID;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA:
            if (!cachePtr->HasTarget())
                return false;
            return cachePtr->target.unit->HasAura(aura.spell_id);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE:
            return cachePtr->miscValue1 >= value.minvalue;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL:
            if (!cachePtr->HasTarget())
                return false;
            return cachePtr->target.Level >= level.minlevel;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER:
            if (!cachePtr->HasTarget())
                return false;
            return cachePtr->target.Gender == gender.gender;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT:
            return sScriptMgr->OnCriteriaCheck(this, cachePtr->player->ToPlayer(), const_cast<Unit*>(cachePtr->target.unit));
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT:
            return cachePtr->PlayersCount <= map_players.maxcount;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM:
            if (!cachePtr->HasTarget() || cachePtr->IsCreature())
                return false;
            return cachePtr->target.Team == team.team;
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK:
            return Player::GetDrunkenstateByValue(cachePtr->DrunkValue) >= DrunkenState(drunk.state);
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY:
            return IsHolidayActive(HolidayIds(holiday.id));
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE:
        {
            Battleground* bg = cachePtr->player->GetBattleground();
            if (!bg)
                return false;
            return bg->IsTeamScoreInRange(cachePtr->Team == ALLIANCE ? HORDE : ALLIANCE, bg_loss_team_score.min_score, bg_loss_team_score.max_score);
        }
        case ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT:
        {
            if (!cachePtr->player->IsInWorld())
                return false;
            Map* map = cachePtr->player->GetMap();
            if (!map->IsDungeon())
            {
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Achievement system call ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (%u) for achievement criteria %u for non-dungeon/non-raid map %u",
                    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT, criteria_id, map->GetId());
                return false;
            }
            InstanceScript* instance = (static_cast<InstanceMap*>(map))->GetInstanceScript();
            if (!instance)
            {
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Achievement system call ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT (%u) for achievement criteria %u for map %u but map does not have a instance script",
                    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT, criteria_id, map->GetId());
                return false;
            }
            return instance->CheckAchievementCriteriaMeet(criteria_id, cachePtr->player, cachePtr->target.unit, cachePtr->miscValue1);
        }
        case ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM:
        {
            ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(cachePtr->miscValue1);
            if (!pProto)
                return false;
            return pProto->ItemLevel >= equipped_item.item_level && pProto->GetQuality() >= equipped_item.item_quality;
        }
        default:
            break;
    }
    return false;
}

AchievementCriteriaDataSet::AchievementCriteriaDataSet() : criteria_id(0)
{
}

void AchievementCriteriaDataSet::Add(AchievementCriteriaData const& data)
{
    storage.push_back(data);
}

bool AchievementCriteriaDataSet::Meets(AchievementCachePtr cachePtr) const
{
    for (auto itr : storage)
        if (!itr.Meets(criteria_id, cachePtr))
            return false;

    return true;
}

void AchievementCriteriaDataSet::SetCriteriaId(uint32 id)
{
    criteria_id = id;
}

CompletedAchievementData::CompletedAchievementData() : date(0), first_guid(0), changed(false), isAccountAchievement(false)
{
}

template<class T>
AchievementMgr<T>::AchievementMgr(T* owner) : _owner(owner), _achievementPoints(0), _achievementBattlePetPoints{0}
{
    _completedAchievementsArr.assign(MAX_ACHIEVEMENT, nullptr);
    _criteriaProgressArr.assign(MAX_CRITERIA_TREE, nullptr);
    _timeCriteriaTreesArr.assign(MAX_CRITERIA_TREE, nullptr);

    if (GetCriteriaSort() == SCENARIO_CRITERIA)
        m_canUpdateAchiev = 1;
}

template<class T>
AchievementMgr<T>::~AchievementMgr() = default;

template<class T>
void AchievementMgr<T>::SendPacket(WorldPacket const* data) const
{ }

template<>
void AchievementMgr<Scenario>::SendPacket(WorldPacket const* data) const
{
    // FIXME
}

template<>
void AchievementMgr<Guild>::SendPacket(WorldPacket const* data) const
{
    GetOwner()->BroadcastPacket(data);
}

template<>
void AchievementMgr<Player>::SendPacket(WorldPacket const* data) const
{
    GetOwner()->AddUpdatePacket(data);
}

template<>
void AchievementMgr<Player>::SendAllTrackedCriterias(Player* /*receiver*/, std::set<uint32> const& /*trackedCriterias*/)
{ }

template <class T>
CriteriaProgressMap const* AchievementMgr<T>::GetCriteriaProgressMap()
{
    return &_criteriaProgress;
}

template<class T>
void AchievementMgr<T>::RemoveCriteriaProgress(CriteriaTree const* criteriaTree)
{
    CriteriaProgress* criteria = _criteriaProgressArr[criteriaTree->ID];
    if (!criteria)
        return;

    SendPacket(WorldPackets::Achievement::CriteriaDeleted(criteriaTree->CriteriaID).Write());

    criteria->Counter = 0;
    criteria->deleted = true;
}

template<>
void AchievementMgr<Scenario>::RemoveCriteriaProgress(const CriteriaTree* /*criteriaTree*/)
{
    // FIXME
}

template<>
void AchievementMgr<Scenario>::SendAllTrackedCriterias(Player* /*receiver*/, std::set<uint32> const& /*trackedCriterias*/)
{ }

template<>
void AchievementMgr<Guild>::RemoveCriteriaProgress(const CriteriaTree* criteriaTree)
{
    CriteriaProgress* criteria = _criteriaProgressArr[criteriaTree->ID];
    if (!criteria)
        return;

    _criteriaProgressArr[criteriaTree->ID] = nullptr;

    WorldPackets::Achievement::GuildCriteriaDeleted criteriaDeleted;
    criteriaDeleted.GuildGUID = GetOwner()->GetGUID();
    criteriaDeleted.CriteriaID = criteriaTree->CriteriaID;
    SendPacket(criteriaDeleted.Write());

    criteria->Counter = 0;
    criteria->deleted = true;
}

template<class T>
void AchievementMgr<T>::ResetAchievementCriteria(CriteriaTypes type, uint32 miscValue1, uint32 miscValue2, bool evenIfCriteriaComplete)
{
    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr::ResetAchievementCriteria(%u, %u, %u)", type, miscValue1, miscValue2);

    // disable for gamemasters with GM-mode enabled
    if (GetOwner()->isGameMaster()/* || !GetOwner()->CanContact() */) //CanContact
        return;

    CriteriaTreeList const& criteriaTreeList = sAchievementMgr->GetCriteriaTreeByType(type, GetCriteriaSort());
    for (CriteriaTree const* criteriaTree : criteriaTreeList)
    {
        if (!criteriaTree->Criteria)
            continue;

        CriteriaEntry const* criteria = criteriaTree->Criteria->Entry;
        AchievementEntry const* achievement = criteriaTree->Achievement;
        CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(criteriaTree->ID);
        if (!achievement || !tree)
            continue;

        // don't update already completed criteria if not forced or achievement already complete
        if ((IsCompletedCriteriaTree(criteriaTree) && !evenIfCriteriaComplete)/* || HasAchieved(achievement->ID, GetOwner()->GetGUIDLow())*/)
            continue;

        if (criteria->StartEvent == miscValue1 && (!criteria->StartAsset || criteria->StartAsset == miscValue2))
        {
            RemoveCriteriaProgress(tree);
            continue;
        }

        if (criteria->FailEvent == miscValue1 && (!criteria->FailAsset || criteria->FailAsset == miscValue2))
        {
            RemoveCriteriaProgress(tree);
            continue;
        }
    }
}

template<>
void AchievementMgr<Scenario>::ResetAchievementCriteria(CriteriaTypes /*type*/, uint32 /*miscValue1*/, uint32 /*miscValue2*/, bool /*evenIfCriteriaComplete*/)
{
    // Not needed
}

template<>
void AchievementMgr<Guild>::ResetAchievementCriteria(CriteriaTypes /*type*/, uint32 /*miscValue1*/, uint32 /*miscValue2*/, bool /*evenIfCriteriaComplete*/)
{
    // Not needed
}

template<class T>
void AchievementMgr<T>::DeleteFromDB(ObjectGuid /*lowguid*/, uint32 /*accountId*/)
{ }

template<>
void AchievementMgr<Player>::DeleteFromDB(ObjectGuid guid, uint32 accountId)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_ACHIEVEMENT);
    stmt->setUInt64(0, guid.GetCounter());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

template<>
void AchievementMgr<Guild>::DeleteFromDB(ObjectGuid guid, uint32 accountId)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENTS);
    stmt->setUInt64(0, guid.GetCounter());
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENT_CRITERIA);
    stmt->setUInt64(0, guid.GetCounter());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

template<class T>
void AchievementMgr<T>::SaveToDB(SQLTransaction& /*trans*/)
{ }

template<>
void AchievementMgr<Player>::SaveToDB(SQLTransaction& trans)
{
    if (!GetOwner())
        return;

    if (!_completedAchievements.empty())
    {
        bool need_execute = false;
        bool need_execute_acc = false;

        std::ostringstream ssAccIns;
        std::ostringstream ssCharIns;

        for (auto & _completedAchievement : _completedAchievements)
        {
            CompletedAchievementData* ca = &_completedAchievement.second;
            if (!ca)
                continue;

            if (!ca->changed)
                continue;

            /// first new/changed record prefix
            if (!need_execute)
            {
                ssCharIns << "REPLACE INTO character_achievement (guid, achievement, date) VALUES ";
                need_execute = true;
            }
            /// next new/changed record prefix
            else
                ssCharIns << ',';

            if (!need_execute_acc)
            {
                ssAccIns << "REPLACE INTO account_achievement (account, first_guid, achievement, date) VALUES ";
                need_execute_acc = true;
            }
            else
                ssAccIns << ',';

            // new/changed record data
            ssAccIns << '(' << GetOwner()->GetSession()->GetAccountId() << ',' << ca->first_guid << ',' << _completedAchievement.first << ',' << ca->date << ')';
            ssCharIns << '(' << GetOwner()->GetGUIDLow() << ',' << _completedAchievement.first << ',' << ca->date << ')';

            /// mark as saved in db
            ca->changed = false;
        }

        if (need_execute)
            trans->Append(ssCharIns.str().c_str());

        if (need_execute_acc)
            trans->Append(ssAccIns.str().c_str());
    }

    if (_criteriaProgress.empty())
        return;

    {
        uint64 guid = GetOwner()->GetGUIDLow();
        uint32 accountId = GetOwner()->GetSession()->GetAccountId();

        for (CriteriaProgressMap::iterator iter = _criteriaProgress.begin(); iter != _criteriaProgress.end(); ++iter)
        {
            CriteriaProgress* progress = &iter->second;
            if (!progress || progress->deactiveted || (!progress->changed && !progress->updated && !progress->deleted))
                continue;

            /// prepare deleting and insert
            bool need_execute_ins = false;
            bool need_execute_account = false;

            bool isAccountAchievement = false;

            bool alreadyOneCharInsLine = false;
            bool alreadyOneAccInsLine = false;

            std::ostringstream ssAccins;
            std::ostringstream ssCharins;

            //disable? active before test achivement system
            AchievementEntry const* achievement = progress->achievement;
            if (achievement && (achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT))
            {
                isAccountAchievement = true;
                need_execute_account = true;
            }
            else
                isAccountAchievement = false;

            // store data only for real progress
            bool hasAchieve = false;
            if (achievement)
                hasAchieve = HasAchieved(achievement->ID, GetOwner()->GetGUIDLow()) || (achievement->Supercedes && !HasAchieved(achievement->Supercedes, GetOwner()->GetGUIDLow()));

            if (progress->Counter != 0 && !hasAchieve)
            {
                uint32 achievID = achievement ? achievement->ID : 0;
                if (progress->changed)
                {
                    /// first new/changed record prefix
                    if (!need_execute_ins)
                    {
                        ssAccins << "REPLACE INTO account_achievement_progress   (account, criteria, counter, date, achievID, completed) VALUES ";
                        ssCharins << "REPLACE INTO character_achievement_progress (guid,    criteria, counter, date, achievID, completed) VALUES ";
                        need_execute_ins = true;
                    }
                    /// next new/changed record prefix
                    else
                    {
                        if (isAccountAchievement)
                        {
                            if (alreadyOneAccInsLine)
                                ssAccins << ',';
                        }
                        else
                        {
                            if (alreadyOneCharInsLine)
                                ssCharins << ',';
                        }
                    }

                    // new/changed record data
                    if (isAccountAchievement)
                    {
                        ssAccins << '(' << accountId << ',' << iter->first << ',' << progress->Counter << ',' << progress->date << ',' << achievID << ',' << progress->completed << ')';
                        alreadyOneAccInsLine = true;
                    }
                    else
                    {
                        ssCharins << '(' << guid << ',' << iter->first << ',' << progress->Counter << ',' << progress->date << ',' << achievID << ',' << progress->completed << ')';
                        alreadyOneCharInsLine = true;
                    }
                }
                else if (progress->updated)
                {
                    std::ostringstream ssUpd;
                    if (isAccountAchievement)
                        ssUpd << "UPDATE account_achievement_progress SET counter = " << progress->Counter << ", date = " << progress->date << ", achievID = " << achievID << ", completed = " << progress->completed << " WHERE account = " << accountId << " AND criteria = " << iter->first << ';';
                    else
                        ssUpd << "UPDATE character_achievement_progress SET counter = " << progress->Counter << ", date = " << progress->date << ", achievID = " << achievID << ", completed = " << progress->completed << " WHERE guid = " << guid << " AND criteria = " << iter->first << ';';
                    trans->Append(ssUpd.str().c_str());
                }
            }
            else if (progress->deleted)
            {
                std::ostringstream ssDel;
                if (isAccountAchievement)
                    ssDel << "DELETE FROM account_achievement_progress WHERE account = " << accountId << " AND criteria = " << iter->first << ';';
                else
                    ssDel << "DELETE FROM character_achievement_progress WHERE guid = " << guid << " AND criteria = " << iter->first << ';';
                trans->Append(ssDel.str().c_str());
                progress->deactiveted = true;
                progress->deleted = false;
            }

            /// mark as updated in db
            progress->changed = false;
            progress->updated = false;
            if (need_execute_ins)
            {
                if (need_execute_account && alreadyOneAccInsLine)
                    trans->Append(ssAccins.str().c_str());

                if (alreadyOneCharInsLine)
                    trans->Append(ssCharins.str().c_str());
            }
        }
    }
}

template<>
void AchievementMgr<Guild>::SaveToDB(SQLTransaction& trans)
{
    PreparedStatement* stmt;
    std::ostringstream guidstr;
    for (auto & _completedAchievement : _completedAchievements)
    {
        if (!_completedAchievement.second.changed)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GUILD_ACHIEVEMENT);
        stmt->setUInt64(0, GetOwner()->GetId());
        stmt->setUInt32(1, _completedAchievement.first);
        stmt->setUInt32(2, _completedAchievement.second.date);
        for (auto guid : _completedAchievement.second.guids)
            guidstr << guid.GetCounter() << ',';

        stmt->setString(3, guidstr.str());
        trans->Append(stmt);
        _completedAchievement.second.changed = false;

        guidstr.str("");
    }

    {
        for (CriteriaProgressMap::iterator itr = _criteriaProgress.begin(); itr != _criteriaProgress.end(); ++itr)
        {
            if (itr->second.deactiveted)
                continue;

            if (itr->second.changed)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GUILD_ACHIEVEMENT_CRITERIA);
                stmt->setUInt64(0, GetOwner()->GetId());
                stmt->setUInt32(1, itr->first);
                stmt->setUInt32(2, itr->second.Counter);
                stmt->setUInt32(3, itr->second.date);
                stmt->setUInt64(4, itr->second.PlayerGUID.GetCounter());
                stmt->setUInt32(5, itr->second.achievement ? itr->second.achievement->ID : 0);
                stmt->setUInt32(6, itr->second.completed);
                trans->Append(stmt);
                itr->second.changed = false;
            }

            if (itr->second.updated)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_ACHIEVEMENT_CRITERIA);
                stmt->setUInt32(0, itr->second.Counter);
                stmt->setUInt32(1, itr->second.date);
                stmt->setUInt32(2, itr->second.achievement ? itr->second.achievement->ID : 0);
                stmt->setUInt32(3, itr->second.completed);
                stmt->setUInt32(4, GetOwner()->GetId());
                stmt->setUInt32(5, itr->first);
                trans->Append(stmt);
                itr->second.updated = false;
            }
        }
    }
}

template<class T>
void AchievementMgr<T>::LoadFromDB(PreparedQueryResult /*achievementResult*/, PreparedQueryResult /*criteriaResult*/, PreparedQueryResult /*achievementAccountResult*/, PreparedQueryResult /*criteriaAccountResult*/)
{ }

template<>
void AchievementMgr<Player>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult achievementAccountResult, PreparedQueryResult criteriaAccountResult)
{
    if (achievementAccountResult)
    {
        do
        {
            Field* fields = achievementAccountResult->Fetch();
            uint32 first_guid = fields[0].GetUInt32();
            uint32 achievementid = fields[1].GetUInt32();

            // must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementid);
            if (!achievement)
                continue;

            CompletedAchievementData& ca = _completedAchievements[achievementid];
            ca.date = time_t(fields[2].GetUInt32());
            ca.changed = false;
            ca.first_guid = first_guid;
            ca.isAccountAchievement = achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT;
            _completedAchievementsArr[achievementid] = &ca;

            _achievementPoints += achievement->Points;
            if (achievement->Category == 15117) // BattlePet category
                _achievementBattlePetPoints += achievement->Points;

            // title achievement rewards are retroactive
            if (AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement))
            {
                if (uint32 titleId = reward->titleId[reward->genderTitle ? GetOwner()->getGender() : (GetOwner()->GetTeam() == ALLIANCE ? 0 : 1)])
                    if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
                        GetOwner()->SetTitle(titleEntry);
            }

        } while (achievementAccountResult->NextRow());
    }

    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt32();

            // must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementid);
            if (!achievement)
                continue;

            // not added on account?
            if (!_completedAchievementsArr[achievementid])
            {
                CompletedAchievementData& ca = _completedAchievements[achievementid];
                ca.changed = true;
                ca.first_guid = GetOwner()->GetGUIDLow();
                ca.date = time_t(fields[1].GetUInt32());
                _achievementPoints += achievement->Points;
                if (achievement->Category == 15117) // BattlePet category
                    _achievementBattlePetPoints += achievement->Points;
                _completedAchievementsArr[achievementid] = &ca;
            }
            else
            {
                CompletedAchievementData* ca = _completedAchievementsArr[achievementid];
                ca->changed = false;
                ca->first_guid = GetOwner()->GetGUIDLow();
                ca->date = time_t(fields[1].GetUInt32());
            }

            // title achievement rewards are retroactive
            if (AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement))
            {
                if (uint32 titleId = reward->titleId[reward->genderTitle ? GetOwner()->getGender() : (GetOwner()->GetTeam() == ALLIANCE ? 0 : 1)])
                    if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
                        GetOwner()->SetTitle(titleEntry);
            }

        } while (achievementResult->NextRow());
    }

    if (criteriaResult)
    {
        time_t now = time(nullptr);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 char_criteria_id = fields[0].GetUInt32();
            auto date = time_t(fields[2].GetUInt32());
            uint32 achievementID = fields[3].GetUInt32();
            bool completed = fields[4].GetUInt32();
            bool update = false;

            CriteriaTreeEntry const* criteriaTree = sCriteriaTreeStore.LookupEntry(char_criteria_id);
            if (!criteriaTree)
            {
                // we will remove not existed criteriaTree for all characters
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteriaTree %u data removed from table `character_achievement_progress`.", char_criteria_id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, char_criteria_id);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            Criteria const* criteria = sAchievementMgr->GetCriteria(criteriaTree->CriteriaID);
            if (!criteria)
            {
                // we will remove not existed criteria for all characters
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria %u data removed from table `character_achievement_progress`.", char_criteria_id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, char_criteria_id);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            AchievementEntry const* achievement = nullptr;
            if (!achievementID)
            {
                uint32 parent = sAchievementMgr->GetParantTreeId(criteriaTree->Parent);
                achievement = sDB2Manager.GetsAchievementByTreeList(parent);
                update = true;
            }
            else
                achievement = sAchievementStore.LookupEntry(achievementID);

            bool hasAchieve = false;
            if (achievement)
                hasAchieve = HasAchieved(achievement->ID, GetOwner()->GetGUIDLow());

            if (hasAchieve)
            {
                // we will remove already completed criteria
                // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "Achievement %s with progress char_criteria_id %u data removed from table `character_achievement_progress` ", achievement ? "completed" : "not exist", char_criteria_id);
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, char_criteria_id);
                stmt->setUInt32(1, GetOwner()->GetGUIDLow());
                CharacterDatabase.Execute(stmt);
                continue;
            }

            if (criteria->Entry->StartTimer && time_t(date + criteria->Entry->StartTimer) < now)
                continue;

            _criteriaProgress.insert(char_criteria_id);
            CriteriaProgressMap::guarded_ptr itr = _criteriaProgress.get(char_criteria_id);
            CriteriaProgress& progress = itr->second;
            _criteriaProgressArr[char_criteria_id] = &progress;
            progress.Counter = fields[1].GetUInt32();
            progress.date = date;
            progress.changed = false;
            progress.updated = update;
            progress.completed = completed;
            progress.deactiveted = false;
            progress.achievement = achievement;
            progress.criteriaTree = criteriaTree;
            progress._criteria = criteria;
            progress.criteria = criteria->Entry;
            progress.parent = sCriteriaTreeStore.LookupEntry(criteriaTree->Parent);
        } while (criteriaResult->NextRow());
    }

    if (criteriaAccountResult)
    {
        time_t now = time(nullptr);
        do
        {
            Field* fields = criteriaAccountResult->Fetch();
            uint32 acc_criteria_id = fields[0].GetUInt32();
            auto date = time_t(fields[2].GetUInt32());
            uint32 achievementID = fields[3].GetUInt32();
            bool completed = fields[4].GetUInt32();
            bool update = false;

            CriteriaTreeEntry const* criteriaTree = sCriteriaTreeStore.LookupEntry(acc_criteria_id);
            if (!criteriaTree)
            {
                // we will remove not existed criteria for all accounts
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria %u data removed from table `account_achievement_progress`.", acc_criteria_id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACC_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, acc_criteria_id);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            Criteria const* criteria = sAchievementMgr->GetCriteria(criteriaTree->CriteriaID);
            if (!criteria)
            {
                // we will remove not existed criteria for all accounts
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria %u data removed from table `account_achievement_progress`.", acc_criteria_id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACC_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, acc_criteria_id);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            AchievementEntry const* achievement = nullptr;
            if (!achievementID)
            {
                uint32 parent = sAchievementMgr->GetParantTreeId(criteriaTree->Parent);
                achievement = sDB2Manager.GetsAchievementByTreeList(parent);
                update = true;
            }
            else
                achievement = sAchievementStore.LookupEntry(achievementID);

            bool hasAchieve = false;
            if (achievement)
                hasAchieve = HasAchieved(achievement->ID);

            if (hasAchieve)
            {
                // we will remove already completed criteria
                // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "Achievement %s with progress acc_criteria_id %u data removed from table `account_achievement_progress` ", achievement ? "completed" : "not exist", acc_criteria_id);
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ACC_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, acc_criteria_id);
                stmt->setUInt32(1, GetOwner()->GetSession()->GetAccountId());
                CharacterDatabase.Execute(stmt);
                continue;
            }

            if (criteria->Entry->StartTimer && time_t(date + criteria->Entry->StartTimer) < now)
                continue;

            // Achievement in both account & characters achievement_progress, problem
            if (_criteriaProgressArr[acc_criteria_id] != nullptr)
            {
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Achievement '%u' in both account & characters achievement_progress", acc_criteria_id);
                continue;
            }
            _criteriaProgress.insert(acc_criteria_id);
            CriteriaProgressMap::guarded_ptr itr = _criteriaProgress.get(acc_criteria_id);
            CriteriaProgress& progress = itr->second;
            _criteriaProgressArr[acc_criteria_id] = &progress;
            progress.Counter = fields[1].GetUInt32();
            progress.date = date;
            progress.changed = false;
            progress.updated = update;
            progress.completed = completed;
            progress.deactiveted = false;
            progress.achievement = achievement;
            progress.criteriaTree = criteriaTree;
            progress._criteria = criteria;
            progress.criteria = criteria->Entry;
            progress.parent = sCriteriaTreeStore.LookupEntry(criteriaTree->Parent);
        } while (criteriaAccountResult->NextRow());
    }
}

template<>
void AchievementMgr<Guild>::LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult /*achievementAccountResult*/, PreparedQueryResult /*criteriaAccountResult*/)
{
    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32 achievementid = fields[0].GetUInt32();

            // must not happen: cleanup at server startup in sAchievementMgr->LoadCompletedAchievements()
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementid);
            if (!achievement)
                continue;

            CompletedAchievementData& ca = _completedAchievements[achievementid];
            ca.date = time_t(fields[1].GetUInt32());
            Tokenizer guids(fields[2].GetString(), ' ');
            for (auto & guid : guids)
                ca.guids.insert(ObjectGuid::Create<HighGuid::Player>(atol(guid)));

            ca.changed = false;
            _achievementPoints += achievement->Points;
            _completedAchievementsArr[achievementid] = &ca;

            if (achievement->Flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
                sAchievementMgr->SetRealmCompleted(achievement);

        } while (achievementResult->NextRow());
    }

    if (criteriaResult)
    {
        time_t now = time(nullptr);
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32 guild_criteriaTree_id = fields[0].GetUInt32();
            auto date = time_t(fields[2].GetUInt32());
            ObjectGuid::LowType guid = fields[3].GetUInt64();
            uint64 achievementID = fields[4].GetUInt32();
            bool completed = fields[5].GetUInt32();
            bool update = false;

            CriteriaTreeEntry const* criteriaTree = sCriteriaTreeStore.LookupEntry(guild_criteriaTree_id);
            if (!criteriaTree)
            {
                // we will remove not existed criteria for all guilds
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria %u data removed from table `guild_achievement_progress`.", guild_criteriaTree_id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, guild_criteriaTree_id);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            Criteria const* criteria = sAchievementMgr->GetCriteria(criteriaTree->CriteriaID);
            if (!criteria)
            {
                // we will remove not existed criteria for all guilds
                TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement criteria %u data removed from table `guild_achievement_progress`.", guild_criteriaTree_id);

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_INVALID_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, guild_criteriaTree_id);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            AchievementEntry const* achievement = nullptr;
            if (!achievementID)
            {
                uint32 parent = sAchievementMgr->GetParantTreeId(criteriaTree->Parent);
                achievement = sDB2Manager.GetsAchievementByTreeList(parent);
                update = true;
            }
            else
                achievement = sAchievementStore.LookupEntry(achievementID);

            bool hasAchieve = !achievement || HasAchieved(achievement->ID);
            if (hasAchieve)
            {
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACHIEV_PROGRESS_CRITERIA);
                stmt->setUInt32(0, guild_criteriaTree_id);
                stmt->setUInt32(1, GetOwner()->GetId());
                CharacterDatabase.Execute(stmt);
                continue;
            }

            if (criteria->Entry->StartTimer && time_t(date + criteria->Entry->StartTimer) < now)
                continue;

            _criteriaProgress.insert(guild_criteriaTree_id);
            CriteriaProgressMap::guarded_ptr itr = _criteriaProgress.get(guild_criteriaTree_id);
            CriteriaProgress& progress = itr->second;
            _criteriaProgressArr[guild_criteriaTree_id] = &progress;
            progress.Counter = fields[1].GetUInt32();
            progress.date = date;
            progress.PlayerGUID = ObjectGuid::Create<HighGuid::Player>(guid);        //Plr or Guild?
            progress.changed = false;
            progress.completed = completed;
            progress.deactiveted = false;
            progress.updated = update;
            progress.achievement = achievement;
            progress.criteriaTree = criteriaTree;
            progress._criteria = criteria;
            progress.criteria = criteria->Entry;
            progress.parent = sCriteriaTreeStore.LookupEntry(criteriaTree->Parent);
        } while (criteriaResult->NextRow());
    }
    m_canUpdateAchiev = 1;
}

template<class T>
void AchievementMgr<T>::Reset()
{ }

template<>
void AchievementMgr<Scenario>::Reset()
{
    // FIXME
}

template<>
void AchievementMgr<Player>::Reset()
{
    for (auto & _completedAchievement : _completedAchievements)
    {
        WorldPackets::Achievement::AchievementDeleted achDeleted;
        achDeleted.AchievementID = _completedAchievement.first;
        SendPacket(achDeleted.Write());
    }

    _completedAchievements.clear();
    _achievementPoints = 0;
    _achievementBattlePetPoints = 0;
    DeleteFromDB(GetOwner()->GetGUID());

    if (_criteriaProgress.empty())
        return;

    _criteriaProgressArr.assign(MAX_CRITERIA_TREE, nullptr);

    {
        for (CriteriaProgressMap::iterator iter = _criteriaProgress.begin(); iter != _criteriaProgress.end(); ++iter)
            SendPacket(WorldPackets::Achievement::CriteriaDeleted(iter->second.criteriaTree->CriteriaID).Write());
        _criteriaProgress.clear();
    }
    // re-fill data
    CheckAllAchievementCriteria(GetOwner());
}

template<class T>
void AchievementMgr<T>::ClearMap()
{
    m_canUpdateAchiev = 0;

    _criteriaProgress.clear();
    _completedAchievements.clear();
    _criteriaProgressArr.clear();
    _completedAchievementsArr.clear();
    _timeCriteriaTrees.clear();
    _timeCriteriaTreesArr.clear();
}

template<class T>
uint32 AchievementMgr<T>::GetSize()
{
    uint32 size = sizeof(AchievementMgr);

    size += _criteriaProgress.size() * sizeof(CriteriaProgress);
    size += _completedAchievements.size() * sizeof(CompletedAchievementData);
    size += _completedAchievementsArr.size() * sizeof(CompletedAchievementData*);
    size += _timeCriteriaTrees.size() * sizeof(TimedAchievementMap);
    size += _timeCriteriaTreesArr.size() * sizeof(uint32*);

    return size;
}

template<>
void AchievementMgr<Guild>::Reset()
{
    ObjectGuid guid = GetOwner()->GetGUID();
    for (auto & _completedAchievement : _completedAchievements)
    {
        WorldPackets::Achievement::GuildAchievementDeleted guildAchievementDeleted;
        guildAchievementDeleted.AchievementID = _completedAchievement.first;
        guildAchievementDeleted.GuildGUID = guid;
        guildAchievementDeleted.TimeDeleted = _completedAchievement.second.date;
        SendPacket(guildAchievementDeleted.Write());
    }

    _completedAchievements.clear();
    DeleteFromDB(guid);

    if (_criteriaProgress.empty())
        return;

    {
        for (CriteriaProgressMap::iterator iter = _criteriaProgress.begin(); iter != _criteriaProgress.end(); ++iter)
            if (CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(iter->second.criteriaTree->ID))
                RemoveCriteriaProgress(tree);
        _criteriaProgress.clear();
    }
    _achievementPoints = 0;
    _achievementBattlePetPoints = 0;
}

template<class T>
void AchievementMgr<T>::SendAchievementEarned(AchievementEntry const* achievement)
{
    Player* player = GetOwner();
    // Don't send for achievements with ACHIEVEMENT_FLAG_HIDDEN
    if (achievement->Flags & ACHIEVEMENT_FLAG_HIDDEN)
        return;

    if (Guild* guild = sGuildMgr->GetGuildById(GetOwner()->GetGuildId()))
    {
        Trinity::BroadcastTextBuilder _builder(GetOwner(), CHAT_MSG_GUILD_ACHIEVEMENT, BROADCAST_TEXT_ACHIEVEMENT_EARNED, GetOwner(), achievement->ID);
        Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> _localizer(_builder);
        guild->BroadcastWorker(_localizer, GetOwner());
    }

    ObjectGuid ownerGuid = GetOwner()->GetGUID();

    if (achievement->Flags & (ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_REACH))
    {
        WorldPackets::Achievement::ServerFirstAchievement sFirstAch;
        sFirstAch.PlayerGUID = ownerGuid;
        sFirstAch.Name = GetOwner()->GetName();
        sFirstAch.AchievementID = achievement->ID;
        sFirstAch.GuildAchievement = true;
        sWorld->SendGlobalMessage(sFirstAch.Write());
    }
    // if player is in world he can tell his friends about new achievement
    else if (GetOwner()->IsInWorld())
    {
        player->AddDelayedEvent(10, [player, achievement]() -> void
        {
            Trinity::BroadcastTextBuilder _builder(player, CHAT_MSG_ACHIEVEMENT, BROADCAST_TEXT_ACHIEVEMENT_EARNED, player, achievement->ID);
            Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> _localizer(_builder);
            Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> > _worker(player, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), _localizer);
            Trinity::VisitNearbyWorldObject(player, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY), _worker);
        });
    }

    ObjectGuid firstPlayerOnAccountGuid = ownerGuid;
    if (HasAccountAchieved(achievement->ID))
        firstPlayerOnAccountGuid = ObjectGuid::Create<HighGuid::Player>(GetFirstAchievedCharacterOnAccount(achievement->ID));

    WorldPackets::Achievement::AchievementEarned achievementEarned;
    achievementEarned.Sender = ownerGuid;
    achievementEarned.Earner = firstPlayerOnAccountGuid;
    achievementEarned.EarnerNativeRealm = achievementEarned.EarnerVirtualRealm = GetVirtualRealmAddress();
    achievementEarned.AchievementID = achievement->ID;
    achievementEarned.Time = time(nullptr);
    player->AddUpdatePacketInRange(achievementEarned.Write());
}

template<>
void AchievementMgr<Scenario>::SendAchievementEarned(AchievementEntry const* achievement)
{
    // FIXME
}

template<>
void AchievementMgr<Guild>::SendAchievementEarned(AchievementEntry const* achievement)
{
    if (achievement->Flags & (ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_REACH))
    {
        WorldPackets::Achievement::ServerFirstAchievement serverFirstAchievement;
        serverFirstAchievement.Name = GetOwner()->GetName();
        serverFirstAchievement.PlayerGUID = GetOwner()->GetGUID();
        serverFirstAchievement.AchievementID = achievement->ID;
        serverFirstAchievement.GuildAchievement = true;
        sWorld->SendGlobalMessage(serverFirstAchievement.Write());
    }

    WorldPackets::Achievement::GuildAchievementEarned guildAchievementEarned;
    guildAchievementEarned.AchievementID = achievement->ID;
    guildAchievementEarned.GuildGUID = GetOwner()->GetGUID();
    guildAchievementEarned.TimeEarned = time(nullptr);
    SendPacket(guildAchievementEarned.Write());
}

template<class T>
void AchievementMgr<T>::SendCriteriaUpdate(CriteriaProgress const* /*progress*/, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{ }

template<class T>
void AchievementMgr<T>::SendAccountCriteriaUpdate(CriteriaProgress const* /*progress*/, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{ }

template<>
void AchievementMgr<Player>::SendCriteriaUpdate(CriteriaProgress const* progress, uint32 timeElapsed, bool timedCompleted) const
{
    if (!GetOwner() || !progress || !progress->criteria)
        return;

    WorldPackets::Achievement::CriteriaUpdate criteriaUpdate;

    criteriaUpdate.CriteriaID = progress->criteria->ID;
    criteriaUpdate.Quantity = progress->Counter;
    criteriaUpdate.PlayerGUID = GetOwner()->GetGUID();
    if (progress->criteria->StartTimer)
        criteriaUpdate.Flags = timedCompleted ? 1 : 0; // 1 is for keeping the Counter at 0 in client

    criteriaUpdate.CurrentTime = progress->date;
    criteriaUpdate.ElapsedTime = timeElapsed;
    criteriaUpdate.CreationTime = timeElapsed;

    SendPacket(criteriaUpdate.Write());

    InstanceMap* inst = GetOwner()->GetMap()->ToInstanceMap();
    if (uint32 instanceId = inst ? inst->GetInstanceId() : 0)
        if (Scenario* scenario = sScenarioMgr->GetScenario(instanceId))
            scenario->SendCriteriaUpdate(progress, timeElapsed);
}

template<>
void AchievementMgr<Player>::SendAccountCriteriaUpdate(CriteriaProgress const* progress, uint32 timeElapsed, bool timedCompleted) const
{
    if (!GetOwner() || !progress || !progress->criteria)
        return;

    WorldPackets::Achievement::CriteriaUpdateAccount account;
    account.Data.Id = progress->criteria->ID;
    account.Data.Quantity = progress->Counter;
    account.Data.Player = GetOwner()->GetGUID();
    account.Data.Flags = 0;
    account.Data.Date = progress->date;
    account.Data.TimeFromStart = timeElapsed; // not sure
    account.Data.TimeFromCreate = timeElapsed; // not sure
    SendPacket(account.Write());
}

template<>
void AchievementMgr<Scenario>::SendCriteriaUpdate(CriteriaProgress const* progress, uint32 /*timeElapsed*/, bool timedCompleted) const
{
    // FIXME
    GetOwner()->SendCriteriaUpdate(progress);
    // if (timedCompleted)
    GetOwner()->UpdateCurrentStep(false);
}

template<>
void AchievementMgr<Guild>::SendCriteriaUpdate(CriteriaProgress const* progress, uint32 /*timeElapsed*/, bool /*timedCompleted*/) const
{
    if (!GetOwner() || !progress || !progress->criteria)
        return;

    WorldPackets::Achievement::GuildCriteriaUpdate guildCriteriaUpdate;
    guildCriteriaUpdate.Progress.resize(1);

    WorldPackets::Achievement::GuildCriteriaProgress guildCriteriaProgressdata;
    guildCriteriaProgressdata.CriteriaID = progress->criteria->ID;
    guildCriteriaProgressdata.DateCreated = progress->date;
    guildCriteriaProgressdata.DateStarted = progress->date;
    guildCriteriaProgressdata.DateUpdated = ::time(nullptr) - progress->date;
    guildCriteriaProgressdata.Quantity = progress->Counter;
    guildCriteriaProgressdata.PlayerGUID = progress->PlayerGUID;
    guildCriteriaProgressdata.Flags = 0;
    guildCriteriaUpdate.Progress.push_back(guildCriteriaProgressdata);
    SendPacket(guildCriteriaUpdate.Write());
}

template<>
void AchievementMgr<Guild>::SendAllTrackedCriterias(Player* receiver, std::set<uint32> const& trackedCriterias)
{
    WorldPackets::Achievement::GuildCriteriaUpdate guildCriteriaUpdate;
    guildCriteriaUpdate.Progress.reserve(trackedCriterias.size());

    for (uint32 criteriaId : trackedCriterias)
    {
        auto progress = _criteriaProgressArr[criteriaId];
        if (progress == nullptr)
            continue;

        WorldPackets::Achievement::GuildCriteriaProgress guildCriteriaProgress;
        guildCriteriaProgress.CriteriaID = criteriaId;
        guildCriteriaProgress.DateCreated = 0;
        guildCriteriaProgress.DateStarted = 0;
        guildCriteriaProgress.DateUpdated = progress->date;
        guildCriteriaProgress.Quantity = 0;
        //guildCriteriaProgress.PlayerGUID = ObjectGuid::Create<HighGuid::Player>(progress->PlayerGUID);
        guildCriteriaProgress.Flags = 0;

        guildCriteriaUpdate.Progress.push_back(guildCriteriaProgress);
    }

    receiver->SendDirectMessage(guildCriteriaUpdate.Write());
}

/**
 * called at player login. The player might have fulfilled some achievements when the achievement system wasn't working yet
 */
template<class T>
void AchievementMgr<T>::CheckAllAchievementCriteria(Player* referencePlayer)
{
    // suppress sending packets
    for (uint32 i = 0; i < CRITERIA_TYPE_TOTAL; ++i)
        UpdateAchievementCriteria(CriteriaTypes(i), 0, 0, 0, nullptr, referencePlayer, true);

    m_canUpdateAchiev = 1;
}

static const uint32 achievIdByArenaSlot[MAX_ARENA_SLOT] = {1057, 1107, 1108};
static const uint32 achievIdForDungeon[][4] =
{
    // ach_cr_id, is_dungeon, is_raid, is_heroic_dungeon
    { 321,       true,      true,   true  },
    { 916,       false,     true,   false },
    { 917,       false,     true,   false },
    { 918,       true,      false,  false },
    { 2219,      false,     false,  true  },
    { 0,         false,     false,  false }
};

/**
 * this function will be called whenever the user might have done a criteria relevant action
 */
 
template<class T>
void AchievementMgr<T>::UpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 /*= 0*/, uint32 miscValue2 /*= 0*/, uint32 miscValue3 /*= 0*/, Unit* unit /*= NULL*/, Player* referencePlayer /*= NULL*/, bool init /*=false*/)
{
    AchievementCachePtr referenceCache = std::make_shared<AchievementCache>(referencePlayer, unit, type, miscValue1, miscValue2, miscValue3);
    UpdateAchievementCriteria(referenceCache);
}

template<class T>
void AchievementMgr<T>::UpdateAchievementCriteria(AchievementCachePtr cachePtr, bool init /*=false*/)
{
    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "UpdateAchievementCriteria type %u (%u, %u, %u) CriteriaSort %u", cachePtr->type, cachePtr->miscValue1, cachePtr->miscValue2, cachePtr->miscValue3, GetCriteriaSort());

    // Prevent update if player not loading
    if (!this || !CanUpdate())
        return;

    // disable for gamemasters with GM-mode enabled
    if (cachePtr->player->isGameMaster() || !cachePtr->player->CanContact())
        return;

    // Lua_GetGuildLevelEnabled() is checked in achievement UI to display guild tab
    if (GetCriteriaSort() == GUILD_CRITERIA && !sWorld->getBoolConfig(CONFIG_GUILD_LEVELING_ENABLED))
        return;

    CriteriaTreeList const& criteriaList = sAchievementMgr->GetCriteriaTreeByType(cachePtr->type, GetCriteriaSort());

    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "UpdateAchievementCriteria type %u criteriaList %u", cachePtr->type, criteriaList.size());

    if (criteriaList.empty())
        return;

    Player* referencePlayer = cachePtr->player;
    uint32 miscValue1 = cachePtr->miscValue1;
    uint32 miscValue2 = cachePtr->miscValue2;
    uint32 miscValue3 = cachePtr->miscValue3;

    for (CriteriaTree const* tree : criteriaList)
    {
        CriteriaTreeEntry const* criteriaTree = tree->Entry;
        CriteriaEntry const* criteria = tree->Criteria ? tree->Criteria->Entry : nullptr;
        AchievementEntry const* achievement = tree->Achievement;

        if (!criteriaTree || !criteria)
            continue;

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_CRITERIA, criteria->ID, referencePlayer))
            continue;

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_CRITERIA_TREE, criteriaTree->ID, referencePlayer))
            continue;

        if (achievement && DisableMgr::IsDisabledFor(DISABLE_TYPE_ACHIEVEMENT, achievement->ID, referencePlayer))
            continue;

        bool canComplete = false;
        if (!CanUpdateCriteria(tree, cachePtr))
            continue;

        // requirements not found in the dbc
        if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(tree))
            if (!data->Meets(cachePtr))
                continue;

        if (!CanUpdate())
            return;

        switch (cachePtr->type)
        {
            // std. case: increment at 1
            case CRITERIA_TYPE_WIN_BG:
            case CRITERIA_TYPE_WIN_BATTLEGROUND:
            case CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
            case CRITERIA_TYPE_LOSE_DUEL:
            case CRITERIA_TYPE_CREATE_AUCTION:
            case CRITERIA_TYPE_WON_AUCTIONS:    /* FIXME: for online player only currently */
            case CRITERIA_TYPE_ROLL_NEED:
            case CRITERIA_TYPE_ROLL_GREED:
            case CRITERIA_TYPE_QUEST_ABANDONED:
            case CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
            case CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
            case CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
            case CRITERIA_TYPE_LOOT_EPIC_ITEM:
            case CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
            case CRITERIA_TYPE_DEATH:
            case CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
            case CRITERIA_TYPE_DEATH_AT_MAP:
            case CRITERIA_TYPE_DEATH_IN_DUNGEON:
            case CRITERIA_TYPE_KILLED_BY_CREATURE:
            case CRITERIA_TYPE_KILLED_BY_PLAYER:
            case CRITERIA_TYPE_DEATHS_FROM:
            case CRITERIA_TYPE_BE_SPELL_TARGET:
            case CRITERIA_TYPE_BE_SPELL_TARGET2:
            case CRITERIA_TYPE_CAST_SPELL:
            case CRITERIA_TYPE_CAST_SPELL2:
            case CRITERIA_TYPE_WIN_RATED_ARENA:
            case CRITERIA_TYPE_USE_ITEM:
            case CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
            case CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            case CRITERIA_TYPE_DO_EMOTE:
            case CRITERIA_TYPE_USE_GAMEOBJECT:
            case CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            case CRITERIA_TYPE_WIN_DUEL:
            case CRITERIA_TYPE_HK_CLASS:
            case CRITERIA_TYPE_HK_RACE:
            case CRITERIA_TYPE_HONORABLE_KILL:
            case CRITERIA_TYPE_SPECIAL_PVP_KILL:
            case CRITERIA_TYPE_GET_KILLING_BLOWS:
            case CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            case CRITERIA_TYPE_INSTANSE_MAP_ID:
            case CRITERIA_TYPE_WIN_ARENA:
            case CRITERIA_TYPE_SCRIPT_EVENT:
            case CRITERIA_TYPE_SCRIPT_EVENT_3:
            case CRITERIA_TYPE_COMPLETE_INSTANCE:
            case CRITERIA_TYPE_PLAY_ARENA:
            case CRITERIA_TYPE_OWN_RANK:
                //case CRITERIA_TYPE_EARNED_PVP_TITLE:
            case CRITERIA_TYPE_KILL_CREATURE_TYPE:
            case CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
            case CRITERIA_TYPE_CATCH_FROM_POOL:
            case CRITERIA_TYPE_BUY_GUILD_EMBLEM:
            case CRITERIA_TYPE_CAPTURE_SPECIFIC_BATTLEPET:
            case CRITERIA_TYPE_CAPTURE_PET_IN_BATTLE:
            case CRITERIA_TYPE_CAPTURE_BATTLE_PET_CREDIT:
            case CRITERIA_TYPE_ADD_BATTLE_PET_JOURNAL:
            case CRITERIA_TYPE_BATTLEPET_WIN:
            case CRITERIA_TYPE_PLACE_GARRISON_BUILDING:
            case CRITERIA_TYPE_CONSTRUCT_GARRISON_BUILDING:
            case CRITERIA_TYPE_START_GARRISON_MISSION:
            case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION:
            case CRITERIA_TYPE_OWN_HEIRLOOMS:
            case CRITERIA_TYPE_COMPLETE_SCENARIO_COUNT:
            case CRITERIA_TYPE_ENTER_AREA:
            case CRITERIA_TYPE_LEAVE_AREA:
            case CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER:
            case CRITERIA_TYPE_ARCHAEOLOGY_SITE_COMPLETE:
            case CRITERIA_TYPE_CHECK_CRITERIA_SELF:
            case CRITERIA_TYPE_COMPLETE_CHALLENGE:
            case CRITERIA_TYPE_OPEN_ARTIFACT_POWERS:
            case CRITERIA_TYPE_RECRUIT_GARRISON_TROOP:
            case CRITERIA_TYPE_DUNGEON_ENCOUNTER_COUNTER:
            case CRITERIA_TYPE_REACH_SCENARIO_BOSS:
            case CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION_COUNT:
            case CRITERIA_TYPE_WIN_BRAWL:
                canComplete = SetCriteriaProgress(tree, init ? 0 : 1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
                // std case: increment at miscValue1
            case CRITERIA_TYPE_MONEY_FROM_VENDORS:
            case CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
            case CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
            case CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
            case CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            case CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
            case CRITERIA_TYPE_LOOT_MONEY:
            case CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:/* FIXME: for online player only currently */
            case CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
            case CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
            // case CRITERIA_TYPE_WIN_BG:
            case CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            case CRITERIA_TYPE_DAMAGE_DONE:
            case CRITERIA_TYPE_HEALING_DONE:
            case CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
            case CRITERIA_TYPE_COMPLETE_QUESTS_COUNT:
            case CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
            case CRITERIA_TYPE_GAIN_ARTIFACT_POWER:
            case CRITERIA_TYPE_HONOR_LEVEL_UP:
            case CRITERIA_TYPE_PRESTIGE_LEVEL_UP:
            case CRITERIA_TYPE_COMPLETE_WORLD_QUEST:
            case CRITERIA_TYPE_TRANSMOG_SET_UNLOCKED:
                canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            case CRITERIA_TYPE_SCRIPT_EVENT_2:
                if (!miscValue2)
                    miscValue2 = 1;
            case CRITERIA_TYPE_KILL_CREATURE:
            case CRITERIA_TYPE_LOOT_TYPE:
            case CRITERIA_TYPE_OWN_ITEM:
            case CRITERIA_TYPE_LOOT_ITEM:
            case CRITERIA_TYPE_CURRENCY:
            case CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            case CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
            case CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
            case CRITERIA_TYPE_ARCHAEOLOGY_GAMEOBJECT:
            case CRITERIA_TYPE_CREATE_ITEM:
            case CRITERIA_TYPE_OWN_TOY:
            case CRITERIA_TYPE_COMPLETE_SCENARIO:
            case CRITERIA_TYPE_RECRUIT_TRANSPORT_FOLLOWER:
            case CRITERIA_TYPE_APPEARANCE_UNLOCKED_BY_SLOT:
                canComplete = SetCriteriaProgress(tree, miscValue2, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            case CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER_COUNT:
            case CRITERIA_TYPE_LEVEL_BATTLE_PET_CREDIT:
            case CRITERIA_TYPE_RELIC_TALENT_UNLOCKED:
                canComplete = SetCriteriaProgress(tree, miscValue2, referencePlayer, PROGRESS_SET);
                break;
                // std case: high value at miscValue1
            case CRITERIA_TYPE_HIGHEST_AUCTION_BID:
            case CRITERIA_TYPE_HIGHEST_AUCTION_SOLD: /* FIXME: for online player only currently */
            case CRITERIA_TYPE_HIGHEST_HIT_DEALT:
            case CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
            case CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
            case CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
            case CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
            case CRITERIA_TYPE_REACH_RBG_RATING:
            case CRITERIA_TYPE_LEARN_GARRISON_TALENT:
                canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
                break;
            case CRITERIA_TYPE_REACH_LEVEL:
                canComplete = SetCriteriaProgress(tree, referencePlayer->getLevel(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_REACH_SKILL_LEVEL:
                if (uint32 skillvalue = referencePlayer->GetBaseSkillValue(criteria->Asset))
                    canComplete = SetCriteriaProgress(tree, skillvalue, referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_LEARN_SKILL_LEVEL:
                if (uint32 maxSkillvalue = referencePlayer->GetPureMaxSkillValue(criteria->Asset))
                    canComplete = SetCriteriaProgress(tree, maxSkillvalue, referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetRewardedQuestCount(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
            {
                time_t nextDailyResetTime = sWorld->GetNextDailyQuestsResetTime();
                CriteriaProgress* progress = GetCriteriaProgress(criteriaTree->ID);

                if (!miscValue1) // Login case.
                {
                    // reset if player missed one day.
                    if (progress && progress->date < (nextDailyResetTime - 2 * DAY))
                        canComplete = SetCriteriaProgress(tree, 0, referencePlayer, PROGRESS_SET);
                    continue;
                }

                ProgressType progressType;
                if (!progress)
                    // 1st time. Start count.
                    progressType = PROGRESS_SET;
                else if (progress->date < (nextDailyResetTime - 2 * DAY))
                    // last progress is older than 2 days. Player missed 1 day => Restart count.
                    progressType = PROGRESS_SET;
                else if (progress->date < (nextDailyResetTime - DAY))
                    // last progress is between 1 and 2 days. => 1st time of the day.
                    progressType = PROGRESS_ACCUMULATE;
                else
                    // last progress is within the day before the reset => Already counted today.
                    continue;

                canComplete = SetCriteriaProgress(tree, 1, referencePlayer, progressType);
                break;
            }
            case CRITERIA_TYPE_FALL_WITHOUT_DYING:
            case CRITERIA_TYPE_PLAYER_LEVEL_UP:
            case CRITERIA_TYPE_BATTLEPET_LEVEL_UP:
                // miscValue1 is the ingame fallheight*100 as stored in dbc
                canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_COMPLETE_QUEST:
            case CRITERIA_TYPE_LEARN_SPELL:
            case CRITERIA_TYPE_EXPLORE_AREA:
            case CRITERIA_TYPE_VISIT_BARBER_SHOP:
            case CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            case CRITERIA_TYPE_EQUIP_ITEM:
            case CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            case CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER:
                canComplete = SetCriteriaProgress(tree, 1, referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_BUY_BANK_SLOT:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetBankBagSlotsValue(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_GAIN_REPUTATION:
            {
                int32 reputation = referencePlayer->GetReputationMgr().GetReputation(criteria->Asset);
                if (reputation > 0)
                    canComplete = SetCriteriaProgress(tree, reputation, referencePlayer, PROGRESS_SET);
                break;
            }
            case CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetExaltedFactionCount(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            case CRITERIA_TYPE_LEARN_SKILL_LINE:
            {
                if (!referencePlayer->CanContact())
                    return;

                uint32 spellCount = miscValue1;
                if (miscValue1 == SKILL_MOUNTS)
                    spellCount = cachePtr->MountCount;
                else if (miscValue1 == SKILL_COMPANIONS)
                    spellCount = cachePtr->BattleCount;
                canComplete = SetCriteriaProgress(tree, spellCount, referencePlayer, PROGRESS_SET);
                break;
            }
            case CRITERIA_TYPE_OWN_TOY_COUNT:
            {
                canComplete = SetCriteriaProgress(tree, cachePtr->ToysCount, referencePlayer, PROGRESS_SET);
                break;
            }
            case CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetReveredFactionCount(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetHonoredFactionCount(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_KNOWN_FACTIONS:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetReputationMgr().GetVisibleFactionCount(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_EARN_HONORABLE_KILL:
                if (!miscValue1)
                    canComplete = SetCriteriaProgress(tree, referencePlayer->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS), referencePlayer, PROGRESS_HIGHEST);
                else
                    canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            case CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetMoney(), referencePlayer, PROGRESS_HIGHEST);
                break;
            case CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            case CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
                if (!miscValue1)
                    canComplete = SetCriteriaProgress(tree, _achievementPoints, referencePlayer, PROGRESS_SET);
                else
                    canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            case CRITERIA_TYPE_BATTLEPET_ACHIEVEMENT_POINTS:
                if (!miscValue1)
                    canComplete = SetCriteriaProgress(tree, _achievementBattlePetPoints, referencePlayer, PROGRESS_SET);
                else
                    canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_ACCUMULATE);
                break;
            case CRITERIA_TYPE_HIGHEST_TEAM_RATING:
            {
                if (miscValue1)
                {
                    if (miscValue2 != criteria->Asset)
                        continue;
                    canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
                }
                else // login case
                {
                    for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                    {
                        Bracket* bracket = referencePlayer->getBracket(arena_slot);
                        if (!bracket || arena_slot != criteria->Asset)
                            continue;
                        canComplete = SetCriteriaProgress(tree, bracket->getRating(), referencePlayer, PROGRESS_HIGHEST);
                        break;
                    }
                }
            }
            break;
            case CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
            {
                if (miscValue1)
                {
                    if (miscValue2 != criteria->Asset)
                        continue;

                    canComplete = SetCriteriaProgress(tree, miscValue1, referencePlayer, PROGRESS_HIGHEST);
                }
                else // login case
                {
                    for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                    {
                        Bracket* bracket = referencePlayer->getBracket(arena_slot);
                        if (!bracket || arena_slot != criteria->Asset)
                            continue;
                        canComplete = SetCriteriaProgress(tree, bracket->getRating(), referencePlayer, PROGRESS_HIGHEST);
                    }
                }

                break;
            }
            case CRITERIA_TYPE_REACH_GUILD_LEVEL:
                canComplete = SetCriteriaProgress(tree, referencePlayer->GetGuildLevel(), referencePlayer, PROGRESS_SET);
                break;
            case CRITERIA_TYPE_COLLECT_BATTLEPET:
            {
                canComplete = SetCriteriaProgress(tree, cachePtr->UniquePets, referencePlayer, PROGRESS_SET);
                break;
            }
            // FIXME: not triggered in code as result, need to implement
            case CRITERIA_TYPE_COMPLETED_LFG_DUNGEONS:
            case CRITERIA_TYPE_INITIATED_KICK_IN_LFG:
            case CRITERIA_TYPE_VOTED_KICK_IN_LFG:
            case CRITERIA_TYPE_BEING_KICKED_IN_LFG:
            case CRITERIA_TYPE_ABANDONED_LFG_DUNGEONS:
            case CRITERIA_TYPE_UNK137:
            case CRITERIA_TYPE_COMPLETE_GUILD_DUNGEON_CHALLENGES:
            case CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGES:
            case CRITERIA_TYPE_UNK140:
            case CRITERIA_TYPE_UNK141:
            case CRITERIA_TYPE_UNK142:
            case CRITERIA_TYPE_UNK144:
            case CRITERIA_TYPE_COMPLETED_LFR_DUNGEONS:
            case CRITERIA_TYPE_ABANDONED_LFR_DUNGEONS:
            case CRITERIA_TYPE_INITIATED_KICK_IN_LFR:
            case CRITERIA_TYPE_VOTED_KICK_IN_LFR:
            case CRITERIA_TYPE_BEING_KICKED_IN_LFR:
            case CRITERIA_TYPE_COUNT_OF_LFR_QUEUE_BOOSTS_BY_TANK:
            case CRITERIA_TYPE_UNK154:
            case CRITERIA_TYPE_UNK159:
            case CRITERIA_TYPE_UPGRADE_GARRISON_BUILDING:
            case CRITERIA_TYPE_UPGRADE_GARRISON:
            // case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION_COUNT:
            case CRITERIA_TYPE_LEARN_GARRISON_BLUEPRINT_COUNT:
            case CRITERIA_TYPE_COMPLETE_GARRISON_SHIPMENT:
            case CRITERIA_TYPE_RAISE_GARRISON_FOLLOWER_ITEM_LEVEL:
            case CRITERIA_TYPE_RAISE_GARRISON_FOLLOWER_LEVEL:
                break;                                   // Not implemented yet :(
            default:
                break;
        }

        if (!CanUpdate())
            return;

        if (!achievement || !canComplete)
            continue;

        // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "UpdateAchievementCriteria criteriaTree %u, achievement %u, criteria %u canComplete %u", criteriaTree->ID, achievement->ID, criteria->ID, canComplete);
        // Counter can never complete
        if (IsCompletedAchievement(achievement, referencePlayer))
            CompletedAchievement(achievement, referencePlayer);

        if (AchievementEntryList const* achRefList = sAchievementMgr->GetAchievementByReferencedId(achievement->ID))
        {
            for (auto itr : *achRefList)
            {
                // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "UpdateAchievementCriteria achievement %u achRef %u", achievement->ID, (*itr)->ID);
                if (IsCompletedAchievement(itr, referencePlayer))
                    CompletedAchievement(itr, referencePlayer);
            }
        }
    }
}

template<class T>
bool AchievementMgr<T>::CanCompleteCriteria(AchievementEntry const* achievement)
{
    return true;
}

template<>
bool AchievementMgr<Player>::CanCompleteCriteria(AchievementEntry const* achievement)
{
    if (achievement && achievement->Flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
    {
        // someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;

        if (GetOwner())
            if (GetOwner()->GetSession())
                if (GetOwner()->GetSession()->GetSecurity())
                    return false;
    }

    return true;
}

template<>
bool AchievementMgr<Guild>::CanCompleteCriteria(AchievementEntry const* achievement)
{
    if (achievement && achievement->Flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
    {
        // someone on this realm has already completed that achievement
        if (sAchievementMgr->IsRealmCompleted(achievement))
            return false;
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::CanUpdateCriteriaTree(CriteriaTree const* tree, Player* player)
{
    return !((tree->Entry->Flags & CRITERIA_TREE_FLAG_HORDE_ONLY && player->GetTeam() != HORDE) || (tree->Entry->Flags & CRITERIA_TREE_FLAG_ALLIANCE_ONLY && player->GetTeam() != ALLIANCE));
}

template<class T>
bool AchievementMgr<T>::IsCompletedCriteria(CriteriaTree const* tree, uint64 requiredAmount)
{
    if (!tree->Criteria)
        return false;

    CriteriaProgress* progress = GetCriteriaProgress(tree->ID);
    if (!progress)
        return false;

    if (!requiredAmount)
        requiredAmount = 1;

    switch (tree->Criteria->Entry->Type)
    {
        case CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            progress->completed = progress->Counter >= (requiredAmount * 75); // skillLevel * 75
            break;
        case CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
        case CRITERIA_TYPE_COMPLETE_QUEST:
        case CRITERIA_TYPE_LEARN_SPELL:
        case CRITERIA_TYPE_EXPLORE_AREA:
        case CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER:
        case CRITERIA_TYPE_TRANSMOG_SET_UNLOCKED:
            progress->completed = progress->Counter >= 1;
            break;
        case CRITERIA_TYPE_WIN_ARENA:
            progress->completed = requiredAmount && progress->Counter >= requiredAmount;
            break;
        case CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            progress->completed = progress->Counter >= 9000;
            break;
        case CRITERIA_TYPE_EARNED_PVP_TITLE:
        case CRITERIA_TYPE_HONOR_LEVEL_UP:
        case CRITERIA_TYPE_PRESTIGE_LEVEL_UP:
        case CRITERIA_TYPE_LEARN_GARRISON_TALENT:
            progress->completed = true;
            break;
        case CRITERIA_TYPE_RELIC_TALENT_UNLOCKED:
        case CRITERIA_TYPE_WIN_BG:
        case CRITERIA_TYPE_WIN_BATTLEGROUND:
        case CRITERIA_TYPE_COMPLETE_INSTANCE:
        case CRITERIA_TYPE_PLAY_ARENA:
        case CRITERIA_TYPE_OWN_RANK:
        case CRITERIA_TYPE_KILL_CREATURE_TYPE:
        case CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
        case CRITERIA_TYPE_CATCH_FROM_POOL:
        case CRITERIA_TYPE_BUY_GUILD_EMBLEM:
        case CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
        case CRITERIA_TYPE_KILL_CREATURE:
        case CRITERIA_TYPE_REACH_LEVEL:
        case CRITERIA_TYPE_REACH_GUILD_LEVEL:
        case CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        case CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        case CRITERIA_TYPE_DAMAGE_DONE:
        case CRITERIA_TYPE_HEALING_DONE:
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        case CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case CRITERIA_TYPE_BE_SPELL_TARGET:
        case CRITERIA_TYPE_BE_SPELL_TARGET2:
        case CRITERIA_TYPE_CAST_SPELL:
        case CRITERIA_TYPE_CAST_SPELL2:
        case CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
        case CRITERIA_TYPE_HONORABLE_KILL:
        case CRITERIA_TYPE_EARN_HONORABLE_KILL:
        case CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
        case CRITERIA_TYPE_OWN_ITEM:
        case CRITERIA_TYPE_WIN_RATED_ARENA:
        case CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        case CRITERIA_TYPE_USE_ITEM:
        case CRITERIA_TYPE_LOOT_ITEM:
        case CRITERIA_TYPE_BUY_BANK_SLOT:
        case CRITERIA_TYPE_GAIN_REPUTATION:
        case CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        case CRITERIA_TYPE_HK_CLASS:
        case CRITERIA_TYPE_HK_RACE:
        case CRITERIA_TYPE_DO_EMOTE:
        case CRITERIA_TYPE_EQUIP_ITEM:
        case CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        case CRITERIA_TYPE_LOOT_MONEY:
        case CRITERIA_TYPE_USE_GAMEOBJECT:
        case CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
        case CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
        case CRITERIA_TYPE_WIN_DUEL:
        case CRITERIA_TYPE_LOOT_TYPE:
        case CRITERIA_TYPE_LEARN_SKILL_LINE:
        case CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
        case CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        case CRITERIA_TYPE_GET_KILLING_BLOWS:
        case CRITERIA_TYPE_CURRENCY:
        case CRITERIA_TYPE_INSTANSE_MAP_ID:
        case CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
        case CRITERIA_TYPE_COMPLETE_QUESTS_COUNT:
        case CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
        case CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
        case CRITERIA_TYPE_REACH_RBG_RATING:
        case CRITERIA_TYPE_SCRIPT_EVENT:
        case CRITERIA_TYPE_SCRIPT_EVENT_2:
        case CRITERIA_TYPE_SCRIPT_EVENT_3:
        case CRITERIA_TYPE_ADD_BATTLE_PET_JOURNAL:
        case CRITERIA_TYPE_CAPTURE_SPECIFIC_BATTLEPET:
        case CRITERIA_TYPE_COLLECT_BATTLEPET:
        case CRITERIA_TYPE_LEVEL_BATTLE_PET_CREDIT:
        case CRITERIA_TYPE_CAPTURE_PET_IN_BATTLE:
        case CRITERIA_TYPE_BATTLEPET_WIN:
        case CRITERIA_TYPE_CAPTURE_BATTLE_PET_CREDIT:
        case CRITERIA_TYPE_BATTLEPET_LEVEL_UP:
        case CRITERIA_TYPE_PLACE_GARRISON_BUILDING:
        case CRITERIA_TYPE_CONSTRUCT_GARRISON_BUILDING:
        case CRITERIA_TYPE_START_GARRISON_MISSION:
        case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION:
        case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION_COUNT:
        case CRITERIA_TYPE_OWN_HEIRLOOMS:
        case CRITERIA_TYPE_COMPLETE_SCENARIO:
        case CRITERIA_TYPE_COMPLETE_SCENARIO_COUNT:
        case CRITERIA_TYPE_ENTER_AREA:
        case CRITERIA_TYPE_LEAVE_AREA:
        case CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER:
        case CRITERIA_TYPE_ARCHAEOLOGY_GAMEOBJECT:
        case CRITERIA_TYPE_ARCHAEOLOGY_SITE_COMPLETE:
        case CRITERIA_TYPE_CHECK_CRITERIA_SELF:
        case CRITERIA_TYPE_COMPLETE_CHALLENGE:
        case CRITERIA_TYPE_CREATE_ITEM:
        case CRITERIA_TYPE_BATTLEPET_ACHIEVEMENT_POINTS:
        case CRITERIA_TYPE_OWN_TOY:
        case CRITERIA_TYPE_OWN_TOY_COUNT:
        case CRITERIA_TYPE_GAIN_ARTIFACT_POWER:
        case CRITERIA_TYPE_OPEN_ARTIFACT_POWERS:
        case CRITERIA_TYPE_PLAYER_LEVEL_UP:
        case CRITERIA_TYPE_RECRUIT_GARRISON_TROOP:
        case CRITERIA_TYPE_COMPLETE_WORLD_QUEST:
        case CRITERIA_TYPE_DUNGEON_ENCOUNTER_COUNTER:
        case CRITERIA_TYPE_REACH_SCENARIO_BOSS:
        case CRITERIA_TYPE_RECRUIT_TRANSPORT_FOLLOWER:
        case CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER_COUNT:
        case CRITERIA_TYPE_APPEARANCE_UNLOCKED_BY_SLOT:
        case CRITERIA_TYPE_WIN_BRAWL:
            progress->completed = progress->Counter >= requiredAmount;
            break;
            // handle all statistic-only criteria here
        case CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
        case CRITERIA_TYPE_DEATH_AT_MAP:
        case CRITERIA_TYPE_DEATH:
        case CRITERIA_TYPE_DEATH_IN_DUNGEON:
        case CRITERIA_TYPE_KILLED_BY_CREATURE:
        case CRITERIA_TYPE_KILLED_BY_PLAYER:
        case CRITERIA_TYPE_DEATHS_FROM:
        case CRITERIA_TYPE_HIGHEST_TEAM_RATING:
        case CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case CRITERIA_TYPE_LOSE_DUEL:
        case CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
        case CRITERIA_TYPE_CREATE_AUCTION:
        case CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
        case CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
        case CRITERIA_TYPE_WON_AUCTIONS:
        case CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
        case CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
        case CRITERIA_TYPE_KNOWN_FACTIONS:
        case CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        case CRITERIA_TYPE_ROLL_NEED:
        case CRITERIA_TYPE_ROLL_GREED:
        case CRITERIA_TYPE_QUEST_ABANDONED:
        case CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
        default:
            break;
    }

    return progress->completed;
}

template<class T>
bool AchievementMgr<T>::IsCompletedCriteriaTree(CriteriaTree const* tree)
{
    if (!tree)
        return false;

    // if (!CanCompleteCriteriaTree(tree))
        // return false;

    uint64 requiredCount = tree->Entry->Amount;
    switch (tree->Entry->Operator)
    {
        case CRITERIA_TREE_OPERATOR_SINGLE:
            return tree->Criteria && IsCompletedCriteria(tree, requiredCount);
        case CRITERIA_TREE_OPERATOR_SINGLE_NOT_COMPLETED:
            return !tree->Criteria || !IsCompletedCriteria(tree, requiredCount);
        case CRITERIA_TREE_OPERATOR_ALL:
            for (CriteriaTree const* node : tree->Children)
                if (!IsCompletedCriteriaTree(node))
                    return false;
            return true;
        case CRITERIA_TREE_OPERAROR_SUM_CHILDREN:
        {
            uint64 progress = 0;
            AchievementGlobalMgr::WalkCriteriaTree(tree, [this, &progress](CriteriaTree const* criteriaTree)
            {
                if (criteriaTree->Criteria)
                    if (CriteriaProgress const* criteriaProgress = GetCriteriaProgress(criteriaTree->ID))
                        progress += criteriaProgress->Counter;
            });
            return progress >= requiredCount;
        }
        case CRITERIA_TREE_OPERATOR_SCENARIO:
        {
            uint64 progress = 0;
            AchievementGlobalMgr::WalkCriteriaTree(tree, [this, &progress](CriteriaTree const* criteriaTree)
            {
                if (criteriaTree->Criteria)
                    if (CriteriaProgress const* criteriaProgress = GetCriteriaProgress(criteriaTree->ID))
                        progress += criteriaProgress->Counter * criteriaTree->Entry->Amount;
            });
            return progress >= requiredCount;
        }
        case CRITERIA_TREE_OPERATOR_MAX_CHILD:
        {
            uint64 progress = 0;
            AchievementGlobalMgr::WalkCriteriaTree(tree, [this, &progress](CriteriaTree const* criteriaTree)
            {
                if (criteriaTree->Criteria)
                    if (CriteriaProgress const* criteriaProgress = GetCriteriaProgress(criteriaTree->ID))
                        if (criteriaProgress->Counter > progress)
                            progress = criteriaProgress->Counter;
            });
            return progress >= requiredCount;
        }
        case CRITERIA_TREE_OPERATOR_COUNT_DIRECT_CHILDREN:
        {
            uint64 progress = 0;
            for (CriteriaTree const* node : tree->Children)
                if (node->Criteria)
                    if (CriteriaProgress const* criteriaProgress = GetCriteriaProgress(node->ID))
                        if (criteriaProgress->Counter >= 1)
                            if (++progress >= requiredCount)
                                return true;

            return false;
        }
        case CRITERIA_TREE_OPERATOR_ANY:
        {
            uint64 progress = 0;
            for (CriteriaTree const* node : tree->Children)
                if (IsCompletedCriteriaTree(node))
                    if (++progress >= requiredCount)
                        return true;

            return false;
        }
        default:
            break;
    }

    return false;
}

template<class T>
bool AchievementMgr<T>::IsCompletedScenarioTree(CriteriaTreeEntry const* criteriaTree)
{
    CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(criteriaTree->ID);
    if (!tree)
        return false;

    return IsCompletedCriteriaTree(tree);
}

template <class T>
bool AchievementMgr<T>::CanUpdate()
{
    return m_canUpdateAchiev == 1;
}

template <class T>
CompletedAchievementMap const* AchievementMgr<T>::GetCompletedAchievementsList()
{
    return &_completedAchievements;
}

template<class T>
bool AchievementMgr<T>::IsCompletedAchievement(AchievementEntry const* achievement, Player* player)
{
    // Counter can never complete
    if ((achievement->Flags & ACHIEVEMENT_FLAG_COUNTER) || HasAchieved(achievement->ID, player->GetGUIDLow()))
        return false;

    CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(achievement->CriteriaTree);
    if (!tree)
        return false;

    if (!CanCompleteCriteria(achievement))
        return false;

    // For SUMM achievements, we have to count the progress of each criteria of the achievement.
    // Oddly, the target count is NOT contained in the achievement, but in each individual criteria
    if (achievement->Flags & ACHIEVEMENT_FLAG_SUMM)
    {
        uint64 progress = 0;
        AchievementGlobalMgr::WalkCriteriaTree(tree, [this, &progress](CriteriaTree const* criteriaTree)
        {
            if (CriteriaProgress const* progres = this->GetCriteriaProgress(criteriaTree->ID))
                progress += progres->Counter;
        });
        return progress >= tree->Entry->Amount;
    }

    return IsCompletedCriteriaTree(tree);
}

template<class T>
CriteriaProgress* AchievementMgr<T>::GetCriteriaProgress(uint32 entry, bool create)
{
    CriteriaProgress* progress = _criteriaProgressArr[entry];
    if (progress)
        return progress;

    if (create)
    {
        _criteriaProgress.insert(entry);
        if (CriteriaProgressMap::guarded_ptr ptr = _criteriaProgress.get(entry))
        {
            _criteriaProgressArr[entry] = &ptr->second;
            return &ptr->second;
        }
    }

    return nullptr;
}

template<class T>
bool AchievementMgr<T>::SetCriteriaProgress(CriteriaTree const* tree, uint32 changeValue, Player* player, ProgressType ptype)
{
    AchievementEntry const* achievement = tree->Achievement;

    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "SetCriteriaProgress tree %u CriteriaID %u achievement %u", tree->ID, tree->CriteriaID, achievement ? achievement->ID : 0);

    if (!CanUpdate())
        return false;

    Criteria const* criteria = tree->Criteria;
    if (!criteria)
        return false;

    auto timedIter = _timeCriteriaTreesArr[tree->ID];
    // Don't allow to cheat - doing timed achievements without timer active
    if (criteria->Entry->StartTimer && timedIter == nullptr)
        return false;

    if (GetCriteriaSort() == PLAYER_CRITERIA && player && tree->Flags & CRITERIA_TREE_CUSTOM_FLAG_QUEST)
        if (player->GetQuestStatus(sAchievementMgr->_criteriaTreeForQuest[tree->ID]) != QUEST_STATUS_INCOMPLETE)
            return false;

    CriteriaProgress* progress = _criteriaProgressArr[tree->ID];
    if (!progress)
    {
        _criteriaProgress.insert(tree->ID);
        CriteriaProgressMap::guarded_ptr ptr = _criteriaProgress.get(tree->ID);
        progress = &ptr->second;
        _criteriaProgressArr[tree->ID] = progress;
        // not create record for 0 counter but allow it for timed achievements
        // we will need to send 0 progress to client to start the timer
        if (changeValue == 0 && !criteria->Entry->StartTimer)
            return false;

    #ifdef _MSC_VER
        // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "SetCriteriaProgress(%u, %u) new CriteriaSort %u achievement %u treeEntry %u", tree->ID, changeValue, GetCriteriaSort(), achievement ? achievement->ID : 0, criteria->ID);
    #endif

        progress->Counter = changeValue;
        progress->changed = true;
        progress->deactiveted = false;
        progress->completed = false;
        progress->achievement = achievement;
        progress->criteriaTree = tree->Entry;
        progress->_criteria = criteria;
        progress->criteria = criteria->Entry;
        progress->parent = sCriteriaTreeStore.LookupEntry(tree->Entry->Parent);
    }
    else
    {
        #ifdef _MSC_VER
            // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "SetCriteriaProgress(%u, %u) old CriteriaSort %u achievement %u", criteria->ID, changeValue, GetCriteriaSort(), achievement ? achievement->ID : 0);
        #endif

        if (progress->completed && achievement && achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT) // Don`t update criteria if already completed
            return true;

        uint32 newValue = 0;
        switch (ptype)
        {
            case PROGRESS_SET:
                newValue = changeValue;
                break;
            case PROGRESS_ACCUMULATE:
            {
                // avoid overflow
                uint32 max_value = std::numeric_limits<uint32>::max();
                newValue = max_value - progress->Counter > changeValue ? progress->Counter + changeValue : max_value;
                break;
            }
            case PROGRESS_HIGHEST:
                newValue = progress->Counter < changeValue ? changeValue : progress->Counter;
                break;
        }

        // not update (not mark as changed) if Counter will have same value
        if (progress->Counter == newValue && !criteria->Entry->StartTimer)
            return false;

        progress->Counter = newValue;
        progress->updated = true;
        progress->deactiveted = false;
    }

    if (!progress) // If thread prioblem
        return false;

    CriteriaTree const* treeParent = sAchievementMgr->GetCriteriaTree(progress->parent ? progress->parent->ID : 0);
    progress->date = time(nullptr); // set the date to the latest update.

    //if (!achievement)
        //return;

    uint32 timeElapsed = 0;
    bool timedCompleted = IsCompletedCriteriaTree(tree) || progress->completed || IsCompletedCriteriaTree(treeParent);
    if (criteria->Entry->StartTimer)
    {
        // Client expects this in packet
        timeElapsed = criteria->Entry->StartTimer - ((*timedIter) / IN_MILLISECONDS);

        // Remove the timer, we wont need it anymore
        if (progress->completed)
        {
            _timeCriteriaTreesArr[tree->ID] = nullptr;
            std::lock_guard<std::recursive_mutex> guard(i_timeCriteriaTreesLock);
            _timeCriteriaTrees.erase(tree->ID);
        }
    }

    if (GetCriteriaSort() == PLAYER_CRITERIA && player && timedCompleted && treeParent)
    {
        if (treeParent->Flags & CRITERIA_TREE_CUSTOM_FLAG_QUEST && IsCompletedCriteriaTree(treeParent))
        {
            uint32 criteriaID = sAchievementMgr->GetParantTreeId(progress->parent->ID);
            if (player->CanContact())
                player->AddDelayedEvent(100, [player, criteriaID]() -> void { if (player) player->AchieveCriteriaCredit(criteriaID); });
        }
    }

#ifdef _MSC_VER
    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "SetCriteriaProgress criteria %u achievement %u treeEntry %u completed %i timeElapsed %i StartTimer %i timedCompleted %u",
        // criteria->ID, achievement ? achievement->ID : 0, criteria->ID, progress->completed, timeElapsed, criteria->Entry->StartTimer, timedCompleted);
#endif

    if (progress->completed && achievement && achievement->Flags & ACHIEVEMENT_FLAG_SHOW_CRITERIA_MEMBERS && !progress->PlayerGUID)
        progress->PlayerGUID = player->GetGUID();

    if (achievement && achievement->Supercedes && !HasAchieved(achievement->Supercedes, player->GetGUIDLow())) //Don`t send update criteria to client if parent achievment not complete
        return false;

    if (achievement && achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT)
        SendAccountCriteriaUpdate(progress, timeElapsed, progress->completed);
    else
        SendCriteriaUpdate(progress, timeElapsed, timedCompleted);

    return timedCompleted;
}

template<class T>
void AchievementMgr<T>::UpdateTimedAchievements(uint32 timeDiff)
{
    if (!_timeCriteriaTrees.empty())
    {
        std::lock_guard<std::recursive_mutex> guard(i_timeCriteriaTreesLock);
        for (auto itr = _timeCriteriaTrees.begin(); itr != _timeCriteriaTrees.end();)
        {
            // Time is up, remove timer and reset progress
            if (itr->second <= timeDiff)
            {
                CriteriaTree const* criteriaTree = sAchievementMgr->GetCriteriaTree(itr->first);
                if (!criteriaTree)
                    return;

                if (criteriaTree->Criteria)
                    RemoveCriteriaProgress(criteriaTree);

                _timeCriteriaTreesArr[itr->first] = nullptr;
                itr = _timeCriteriaTrees.erase(itr);
            }
            else
            {
                itr->second -= timeDiff;
                ++itr;
            }
        }
    }
}

template<class T>
void AchievementMgr<T>::StartTimedAchievement(CriteriaTimedTypes type, uint32 entry, uint16 timeLost /*= 0*/)
{
    // disable for gamemasters with GM-mode enabled
    // if (GetOwner()->isGameMaster() || !GetOwner()->CanContact())
        // return;

    CriteriaTreeList const& criteriaList = sAchievementMgr->GetTimedCriteriaByType(type);
    for (CriteriaTree const* tree : criteriaList)
    {
        if (tree->Criteria->Entry->StartAsset != entry)
            continue;

        bool canStart = false;
        if (_timeCriteriaTreesArr[tree->ID] == nullptr && !IsCompletedCriteriaTree(tree))
        {
            // Start the timer
            if (tree->Criteria->Entry->StartTimer * uint32(IN_MILLISECONDS) > timeLost)
            {
                std::lock_guard<std::recursive_mutex> guard(i_timeCriteriaTreesLock);
                _timeCriteriaTrees[tree->ID] = tree->Criteria->Entry->StartTimer * IN_MILLISECONDS - timeLost;
                _timeCriteriaTreesArr[tree->ID] = &_timeCriteriaTrees[tree->ID];
                canStart = true;
            }
        }

        if (!canStart)
            continue;

        // and at client too
        SetCriteriaProgress(tree, 0, nullptr, PROGRESS_SET);
        /*if (CriteriaProgress* progress = _criteriaProgressArr[tree->ID])
            SendCriteriaUpdate(progress, timeLost, false);*/
    }
}

template<class T>
void AchievementMgr<T>::RemoveTimedAchievement(CriteriaTimedTypes type, uint32 entry)
{
    CriteriaTreeList const& criteriaList = sAchievementMgr->GetTimedCriteriaByType(type);
    for (CriteriaTree const* tree : criteriaList)
    {
        if (tree->Criteria->Entry->StartAsset != entry)
            continue;

        _timeCriteriaTreesArr[tree->ID] = nullptr;
        {
            std::lock_guard<std::recursive_mutex> guard(i_timeCriteriaTreesLock);
            _timeCriteriaTrees.erase(tree->ID);
        }

        // remove progress
        RemoveCriteriaProgress(tree);
    }
}

template <class T>
uint32 AchievementMgr<T>::GetAchievementPoints() const
{
    return _achievementPoints;
}

template<class T>
void AchievementMgr<T>::CompletedAchievement(AchievementEntry const* achievement, Player* player)
{
    // disable for gamemasters with GM-mode enabled
    if (GetOwner()->isGameMaster())
        return;

    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "CompletedAchievement achievement %u", achievement->ID);

    auto const& ownerSession = GetOwner()->GetSession();

    CompletedAchievementData* ca = nullptr;
    {
        std::lock_guard<std::recursive_mutex> guard(i_completedAchievementsLock);

        if (achievement->Flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS)
            if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
                guild->AddGuildNews(GUILD_NEWS_PLAYER_ACHIEVEMENT, player->GetGUID(), achievement->Flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER, achievement->ID);

        if (!GetOwner()->GetSession()->PlayerLoading())
            SendAchievementEarned(achievement);

        ca = &_completedAchievements[achievement->ID];
        ca->date = time(nullptr);
        ca->first_guid = GetOwner()->GetGUIDLow();
        ca->changed = true;
        _completedAchievementsArr[achievement->ID] = ca;
    }

    ownerSession->SetAchievement(achievement->ID);

    // don't insert for ACHIEVEMENT_FLAG_REALM_FIRST_KILL since otherwise only the first group member would reach that achievement
    // TODO: where do set this instead?
    if (!(achievement->Flags & ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
        sAchievementMgr->SetRealmCompleted(achievement);

    _achievementPoints += achievement->Points;
    if (achievement->Category == 15117) // BattlePet category
        _achievementBattlePetPoints += achievement->Points;

    player->AddDelayedEvent(100, [player, achievement]() -> void 
    {
        if (!player)
            return;
        if (achievement->Category == 15117) // BattlePet category
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BATTLEPET_ACHIEVEMENT_POINTS, achievement->Points, 0, 0, nullptr, player);

        player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_ACHIEVEMENT, 0, 0, 0, nullptr, player); 
        player->UpdateAchievementCriteria(CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS, achievement->Points, 0, 0, nullptr, player);
    });

    switch (achievement->ID)
    {
        case 7433: ///< Newbie
        case 6566: ///< Just a Pup
            ownerSession->SendPetBattleSlotUpdates(true);
            ownerSession->SendBattlePetLicenseChanged();
            break;
        case 6556: ///< Going to Need More Traps
        case 6581: ///< Pro Pet Crew
            ownerSession->SendBattlePetTrapLevel();
            break;
        default:
            break;
    }

    // reward items and titles if any
    AchievementReward const* reward = sAchievementMgr->GetAchievementReward(achievement);
    // no rewards
    if (!reward)
        return;

    //! Custom reward handlong
    sScriptMgr->OnRewardCheck(reward, GetOwner());

    // titles
    //! Currently there's only one achievement that deals with gender-specific titles.
    //! Since no common attributes were found, (not even in titleRewardFlags field)
    //! we explicitly check by ID. Maybe in the future we could move the achievement_reward
    //! condition fields to the condition system.
    if (uint32 titleId = reward->titleId[reward->genderTitle ? GetOwner()->getGender() : (GetOwner()->GetTeam() == ALLIANCE ? 0 : 1)])
        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(titleId))
            GetOwner()->SetTitle(titleEntry);

    // mail
    if (reward->sender)
    {
        uint32 itemID = sScriptMgr->OnSelectItemReward(reward, GetOwner());
        if (!itemID)
            itemID = reward->itemId;   //OnRewardSelectItem return 0 if no script

        Item* item = itemID ? Item::CreateItem(itemID, 1, GetOwner()) : nullptr;

        LocaleConstant localeConstant = ownerSession->GetSessionDbLocaleIndex();

        // subject and text
        std::string subject = reward->subject;
        std::string text = reward->text;
        if (localeConstant >= 0)
        {
            if (AchievementRewardLocale const* loc = sAchievementMgr->GetAchievementRewardLocale(achievement))
            {
                ObjectMgr::GetLocaleString(loc->subject, localeConstant, subject);
                ObjectMgr::GetLocaleString(loc->text, localeConstant, text);
            }
        }

        MailDraft draft(subject, text);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        if (item)
        {
            // save new item before send
            item->SaveToDB(trans);                               // save for prevent lost at next mail load, if send fail then item will deleted

            // item
            draft.AddItem(item);
        }

        draft.SendMailTo(trans, GetOwner(), MailSender(MAIL_CREATURE, uint64(reward->sender)));
        CharacterDatabase.CommitTransaction(trans);
    }

    if (reward->learnSpell)
        player->AddDelayedEvent(100, [player, reward]() -> void { if (player) player->learnSpell(reward->learnSpell, false); });

    if (reward->castSpell)
        player->AddDelayedEvent(100, [player, reward]() -> void { if (player) player->CastSpell(player, reward->castSpell, true); });
}

template<>
void AchievementMgr<Scenario>::CompletedAchievement(AchievementEntry const* achievement, Player* player)
{
    // not needed
}

template<>
void AchievementMgr<Guild>::CompletedAchievement(AchievementEntry const* achievement, Player* player)
{
    if (achievement->Flags & ACHIEVEMENT_FLAG_COUNTER || HasAchieved(achievement->ID))
        return;

    CompletedAchievementData* ca = nullptr;
    {
        std::lock_guard<std::recursive_mutex> guard(i_completedAchievementsLock);

        if (achievement->Flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS)
            if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
                guild->AddGuildNews(GUILD_NEWS_GUILD_ACHIEVEMENT, ObjectGuid::Empty, achievement->Flags & ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER, achievement->ID);

        SendAchievementEarned(achievement);

        ca = &_completedAchievements[achievement->ID];
        ca->date = time(nullptr);
        ca->changed = true;
        _completedAchievementsArr[achievement->ID] = ca;

        if (achievement->Flags & ACHIEVEMENT_FLAG_SHOW_GUILD_MEMBERS)
        {
            if (player->GetGuildId() == GetOwner()->GetGUID().GetGUIDLow())
                ca->guids.insert(player->GetGUID());

            if (Group const* group = player->GetGroup())
                for (GroupReference const* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
                    if (Player const* groupMember = ref->getSource())
                        if (groupMember->GetGuildId() == GetOwner()->GetGUID().GetGUIDLow())
                            ca->guids.insert(groupMember->GetGUID());
        }
    }

    sAchievementMgr->SetRealmCompleted(achievement);

    _achievementPoints += achievement->Points;
    if (achievement->Category == 15117) // BattlePet category
    {
        _achievementBattlePetPoints += achievement->Points;
        UpdateAchievementCriteria(CRITERIA_TYPE_BATTLEPET_ACHIEVEMENT_POINTS, achievement->Points, 0, 0, nullptr, player);
    }

    UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_ACHIEVEMENT, 0, 0, 0, nullptr, player);
    UpdateAchievementCriteria(CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS, achievement->Points, 0, 0, nullptr, player);
    UpdateAchievementCriteria(CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS, achievement->Points, 0, 0, nullptr, player);
}

struct VisibleAchievementPred
{
    bool operator()(CompletedAchievementMap::value_type const& val) const
    {
        AchievementEntry const* achievement = sAchievementStore.LookupEntry(val.first);
        return achievement && !(achievement->Flags & ACHIEVEMENT_FLAG_HIDDEN);
    }
};

template<class T>
void AchievementMgr<T>::SendAllAchievementData(Player* /*receiver*/)
{
    if (_criteriaProgress.empty())
        return;

    VisibleAchievementPred isVisible;
    WorldPackets::Achievement::AllAchievements achievementData;
    achievementData.Earned.reserve(_completedAchievements.size());
    achievementData.Progress.reserve(_criteriaProgress.size());

    {
        for (auto itr = _completedAchievements.begin(); itr != _completedAchievements.end(); ++itr)
        {
            if (!isVisible(*itr))
                continue;

            WorldPackets::Achievement::EarnedAchievement earned;
            earned.Id = itr->first;
            earned.Date = itr->second.date;
            //if (!(achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT))
            {
                earned.Owner = GetOwner()->GetGUID();
                earned.VirtualRealmAddress = earned.NativeRealmAddress = GetVirtualRealmAddress();
            }
            achievementData.Earned.push_back(earned);
        }
    }

    {
        for (auto itr = _criteriaProgress.begin(); itr != _criteriaProgress.end(); ++itr)
        {
            CriteriaTreeEntry const* criteriaTree = itr->second.criteriaTree;
            if (!criteriaTree)
                continue;

            WorldPackets::Achievement::CriteriaTreeProgress progress;
            progress.Id = criteriaTree->CriteriaID;
            progress.Quantity = itr->second.Counter;
            progress.Player = itr->second.PlayerGUID;
            progress.Flags = 0;
            progress.Date = itr->second.date;
            progress.TimeFromStart = 0;
            progress.TimeFromCreate = 0;
            achievementData.Progress.push_back(progress);
        }
    }

    SendPacket(achievementData.Write());
}

template<>
void AchievementMgr<Scenario>::SendAllAchievementData(Player* receiver)
{
    // not needed
}

template<>
void AchievementMgr<Guild>::SendAllAchievementData(Player* receiver)
{
    WorldPackets::Achievement::AllGuildAchievements allGuildAchievements;
    allGuildAchievements.Earned.reserve(_completedAchievements.size());

    {
        for (auto & _completedAchievement : _completedAchievements)
        {
            WorldPackets::Achievement::EarnedAchievement earned;
            earned.Id = _completedAchievement.first;
            earned.Date = _completedAchievement.second.date;
            allGuildAchievements.Earned.push_back(earned);
        }
    }

    receiver->SendDirectMessage(allGuildAchievements.Write());
}

template<class T>
void AchievementMgr<T>::SendAllAccountCriteriaData(Player* /*receiver*/)
{
    if (_criteriaProgress.empty())
        return;

    ObjectGuid guid = GetOwner()->GetGUID();
    WorldPackets::Achievement::AllAchievementCriteriaDataAccount accountData;
    for (CriteriaProgressMap::iterator itr = _criteriaProgress.begin(); itr != _criteriaProgress.end(); ++itr)
    {
        CriteriaTreeEntry const* criteriaTree = itr->second.criteriaTree;
        if (!criteriaTree)
            continue;

        AchievementEntry const* achievement = itr->second.achievement;

        if (!achievement || !(achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT))
            continue;

        WorldPackets::Achievement::CriteriaTreeProgress progress;
        progress.Id = criteriaTree->CriteriaID;
        progress.Quantity = itr->second.Counter;
        progress.Player = guid/*itr->second.PlayerGUID*/;
        progress.Flags = 0;
        progress.Date = itr->second.date;
        progress.TimeFromStart = 0;
        progress.TimeFromCreate = 0;
        accountData.Data.push_back(progress);
    }

    SendPacket(accountData.Write());
}

template<>
void AchievementMgr<Scenario>::SendAllAccountCriteriaData(Player* /*receiver*/)
{ }

template<class T>
void AchievementMgr<T>::SendAchievementInfo(Player* receiver, uint32 achievementId /*= 0*/)
{ }

template<>
void AchievementMgr<Player>::SendAchievementInfo(Player* receiver, uint32 /*achievementId = 0 */)
{
    if (_criteriaProgress.empty())
        return;

    VisibleAchievementPred isVisible;

    WorldPackets::Achievement::RespondInspectAchievements packet;
    packet.Player = GetOwner()->GetGUID();
    packet.Data.Earned.reserve(_completedAchievements.size());
    packet.Data.Progress.reserve(_criteriaProgress.size());

    {
        for (auto & _completedAchievement : _completedAchievements)
        {
            if (!isVisible(_completedAchievement))
                continue;

            WorldPackets::Achievement::EarnedAchievement earned;
            earned.Id = _completedAchievement.first;
            earned.Date = _completedAchievement.second.date;
            //if (!(achievement->Flags & ACHIEVEMENT_FLAG_ACCOUNT))
            {
                earned.Owner = GetOwner()->GetGUID();
                earned.VirtualRealmAddress = earned.NativeRealmAddress = GetVirtualRealmAddress();
            }
            packet.Data.Earned.push_back(earned);
        }
    }

    {
        for (CriteriaProgressMap::iterator iter = _criteriaProgress.begin(); iter != _criteriaProgress.end(); ++iter)
        {
            CriteriaTreeEntry const* criteriaTree = iter->second.criteriaTree;
            if (!criteriaTree)
                continue;

            WorldPackets::Achievement::CriteriaTreeProgress progress;
            progress.Id = criteriaTree->CriteriaID;
            progress.Quantity = iter->second.Counter;
            progress.Player = iter->second.PlayerGUID;
            progress.Date = iter->second.date;
            packet.Data.Progress.push_back(progress);
        }
    }

    receiver->SendDirectMessage(packet.Write());
}

template<>
void AchievementMgr<Guild>::SendAchievementInfo(Player* receiver, uint32 achievementId /*= 0*/)
{
    WorldPackets::Achievement::GuildCriteriaUpdate guildCriteriaUpdate;

    AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementId);
    if (!achievement)
    {
        receiver->SendDirectMessage(WorldPackets::Achievement::GuildCriteriaUpdate().Write());
        return;
    }

    for (CriteriaProgressMap::iterator itr = _criteriaProgress.begin(); itr != _criteriaProgress.end(); ++itr)
    {
        if (!itr->second.achievement || itr->second.achievement->ID != achievementId)
            continue;

        CriteriaTreeEntry const* criteriaTree = itr->second.criteriaTree;
        if (!criteriaTree)
            continue;

        WorldPackets::Achievement::GuildCriteriaProgress guildCriteriaProgress;
        guildCriteriaProgress.CriteriaID = criteriaTree->CriteriaID;
        guildCriteriaProgress.DateCreated = itr->second.date;
        guildCriteriaProgress.DateStarted = itr->second.date;
        guildCriteriaProgress.DateUpdated = ::time(nullptr) - itr->second.date;
        guildCriteriaProgress.Quantity = itr->second.Counter;
        guildCriteriaProgress.PlayerGUID = itr->second.PlayerGUID;
        guildCriteriaProgress.Flags = 0;

        guildCriteriaUpdate.Progress.push_back(guildCriteriaProgress);
    }

    receiver->SendDirectMessage(guildCriteriaUpdate.Write());
}

template<class T>
bool AchievementMgr<T>::HasAchieved(uint32 achievementId, uint64 guid /*= 0*/)
{
    CompletedAchievementData* ca = _completedAchievementsArr[achievementId];
    if (!ca)
        return false;

    if (ca->isAccountAchievement)
        return true;

    return GetCriteriaSort() == PLAYER_CRITERIA ? ca->first_guid == guid : true;
}

template<class T>
bool AchievementMgr<T>::HasAccountAchieved(uint32 achievementId)
{
    return _completedAchievementsArr[achievementId] != nullptr;
}

template<class T>
uint64 AchievementMgr<T>::GetFirstAchievedCharacterOnAccount(uint32 achievementId)
{
    CompletedAchievementData* ca = _completedAchievementsArr[achievementId];
    if (!ca)
        return 0LL;

    return ca->first_guid;
}

template <class T>
T* AchievementMgr<T>::GetOwner() const
{
    return _owner;
}

template<class T>
bool AchievementMgr<T>::CanUpdateCriteria(CriteriaTree const* tree, AchievementCachePtr cachePtr)
{
    AchievementEntry const* achievement = tree->Achievement;
    CriteriaEntry const* criteria = tree->Criteria->Entry;

    if (achievement)                                                                      // Do not touch!
        if (achievement->Faction != ACHIEVEMENT_FACTION_ANY && achievement->Faction != !cachePtr->TeamID)
            return false;

    if (!criteria)
        return false;

    if (!CanUpdateCriteriaTree(tree, cachePtr->player))
        return false;

    if (!RequirementsSatisfied(tree, cachePtr))
        return false;

    if (tree->Criteria->Modifier && !AdditionalRequirementsSatisfied(tree->Criteria->Modifier, cachePtr))
        return false;

    if (!ConditionsSatisfied(tree->Criteria, cachePtr->player))
        return false;

    if (!GetOwner()->CanUpdateCriteria(tree->ID))
        return false;

    return true;
}

template<class T>
bool AchievementMgr<T>::ConditionsSatisfied(Criteria const* criteria, Player* player) const
{
    if (criteria->Entry->StartEvent)
    {
        switch (criteria->Entry->StartEvent)
        {
            case CRITERIA_CONDITION_BG_MAP:
                if (player->GetMapId() != criteria->Entry->StartAsset)
                    return false;
                break;
            case CRITERIA_CONDITION_NOT_IN_GROUP:
                if (player->GetGroup())
                    return false;
                break;
            default:
                break;
        }
    }

    if (criteria->Entry->FailEvent)
    {
        switch (criteria->Entry->FailEvent)
        {
            case CRITERIA_CONDITION_BG_MAP:
                if (player->GetMapId() != criteria->Entry->FailAsset && criteria->Entry->FailAsset != 0)
                    return false;
                break;
            case CRITERIA_CONDITION_NOT_IN_GROUP:
                if (player->GetGroup())
                    return false;
                break;
            default:
                break;
        }
    }

    return true;
}

template<class T>
bool AchievementMgr<T>::RequirementsSatisfied(CriteriaTree const* tree, AchievementCachePtr cachePtr)
{
    CriteriaEntry const* criteria = tree->Criteria ? tree->Criteria->Entry : nullptr;
    if (!criteria)
        return false;

    AchievementEntry const* achievement = tree->Achievement;
    Player* referencePlayer = cachePtr->player;
    uint32 miscValue1 = cachePtr->miscValue1;
    uint32 miscValue2 = cachePtr->miscValue2;
    uint32 miscValue3 = cachePtr->miscValue3;

    switch (CriteriaTypes(criteria->Type))
    {
        case CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
        case CRITERIA_TYPE_CREATE_AUCTION:
        case CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
        case CRITERIA_TYPE_GET_KILLING_BLOWS:
        case CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
        case CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
        case CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
        case CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
        case CRITERIA_TYPE_HIGHEST_AUCTION_BID:
        case CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
        case CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
        case CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
        case CRITERIA_TYPE_HIGHEST_HIT_DEALT:
        case CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
        case CRITERIA_TYPE_HONORABLE_KILL:
        case CRITERIA_TYPE_LOOT_MONEY:
        case CRITERIA_TYPE_LOSE_DUEL:
        case CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
        case CRITERIA_TYPE_MONEY_FROM_VENDORS:
        case CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
        case CRITERIA_TYPE_QUEST_ABANDONED:
        case CRITERIA_TYPE_ROLL_GREED:
        case CRITERIA_TYPE_ROLL_NEED:
        case CRITERIA_TYPE_SPECIAL_PVP_KILL:
        case CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
        case CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
        case CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
        case CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case CRITERIA_TYPE_WIN_DUEL:
        case CRITERIA_TYPE_WIN_RATED_ARENA:
        case CRITERIA_TYPE_WON_AUCTIONS:
        case CRITERIA_TYPE_WIN_BATTLEGROUND:
        case CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
        case CRITERIA_TYPE_REACH_RBG_RATING:
        case CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
        case CRITERIA_TYPE_CATCH_FROM_POOL:
        case CRITERIA_TYPE_EARNED_PVP_TITLE:
        case CRITERIA_TYPE_ARCHAEOLOGY_SITE_COMPLETE:
        case CRITERIA_TYPE_CREATE_ITEM:
        case CRITERIA_TYPE_GAIN_ARTIFACT_POWER:
        case CRITERIA_TYPE_HONOR_LEVEL_UP:
        case CRITERIA_TYPE_PRESTIGE_LEVEL_UP:
        case CRITERIA_TYPE_LEARN_GARRISON_TALENT:
            if (!miscValue1)
                return false;
            break;
        case CRITERIA_TYPE_BUY_BANK_SLOT:
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
        case CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
        case CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
        case CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
        case CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
        case CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
        case CRITERIA_TYPE_HIGHEST_TEAM_RATING:
        case CRITERIA_TYPE_KNOWN_FACTIONS:
        case CRITERIA_TYPE_REACH_LEVEL:
        case CRITERIA_TYPE_KILL_CREATURE_TYPE:
        case CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
        case CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
        case CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
        case CRITERIA_TYPE_BUY_GUILD_EMBLEM:
        case CRITERIA_TYPE_COMPLETE_QUESTS_COUNT:
        case CRITERIA_TYPE_CAPTURE_SPECIFIC_BATTLEPET:
        case CRITERIA_TYPE_COLLECT_BATTLEPET:
        case CRITERIA_TYPE_BATTLEPET_WIN:
        case CRITERIA_TYPE_OWN_HEIRLOOMS:
        case CRITERIA_TYPE_COMPLETE_CHALLENGE:
        case CRITERIA_TYPE_BATTLEPET_ACHIEVEMENT_POINTS:
        case CRITERIA_TYPE_OWN_TOY_COUNT:
            break;
        case CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            if (_completedAchievementsArr[criteria->Asset] == nullptr)
                return false;
            break;
        case CRITERIA_TYPE_WIN_BG:
            if (!miscValue1 || criteria->Asset != cachePtr->MapID)
                return false;
            break;
        case CRITERIA_TYPE_KILL_CREATURE:
        case CRITERIA_TYPE_USE_ITEM:
        case CRITERIA_TYPE_CHECK_CRITERIA_SELF:
        case CRITERIA_TYPE_OWN_TOY:
        case CRITERIA_TYPE_PLAYER_LEVEL_UP:
            if (!miscValue1 || criteria->Asset != miscValue1)
                return false;
            break;
        case CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case CRITERIA_TYPE_LEARN_SKILL_LEVEL:
        case CRITERIA_TYPE_GAIN_REPUTATION:
        case CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
        case CRITERIA_TYPE_LEARN_SKILL_LINE:
            if (miscValue1 && miscValue1 != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            if (miscValue2 && miscValue2 != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
        case CRITERIA_TYPE_DEATH_AT_MAP:
            if (!miscValue1 || cachePtr->MapID != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_DEATH:
        {
            if (!miscValue1 || !cachePtr->OnBG || !cachePtr->IsArena)
                return false;
            // skip wrong arena achievements, if not achievIdByArenaSlot then normal total death counter
            bool notfit = false;
            for (int j = 0; j < MAX_ARENA_SLOT; ++j)
            {
                if (achievement && achievIdByArenaSlot[j] == achievement->ID)
                {
                    if (MS::Battlegrounds::GetBracketByJoinType(cachePtr->JoinType) != j)
                        notfit = true;
                    break;
                }
            }
            if (notfit)
                return false;
            break;
        }
        case CRITERIA_TYPE_DEATH_IN_DUNGEON:
        {
            if (!miscValue1)
                return false;

            Map const* map = referencePlayer->IsInWorld() ? referencePlayer->GetMap() : sMapMgr->FindMap(referencePlayer->GetMapId(), referencePlayer->GetInstanceId());
            if (!map || !map->IsDungeon())
                return false;

            // search case
            bool found = false;
            for (int j = 0; achievIdForDungeon[j][0]; ++j)
            {
                if (achievIdForDungeon[j][0] == achievement->ID)
                {
                    if (map->IsRaid())
                    {
                        // if raid accepted (ignore difficulty)
                        if (!achievIdForDungeon[j][2])
                            break;                      // for
                    }
                    else if (referencePlayer->GetDungeonDifficultyID() == DIFFICULTY_NORMAL)
                    {
                        // dungeon in normal mode accepted
                        if (!achievIdForDungeon[j][1])
                            break;                      // for
                    }
                    else
                    {
                        // dungeon in heroic mode accepted
                        if (!achievIdForDungeon[j][3])
                            break;                      // for
                    }

                    found = true;
                    break;                              // for
                }
            }
            if (!found)
                return false;

            //FIXME: work only for instances where max == min for players
            if ((static_cast<InstanceMap const*>(map))->GetMaxPlayers() != criteria->Asset)
                return false;
            break;
        }
        case CRITERIA_TYPE_KILLED_BY_PLAYER:
            if (!miscValue1)
                return false;

            // if team check required: must kill by opposition faction
            if (miscValue2 == cachePtr->Team)
                return false;
            break;
        case CRITERIA_TYPE_DEATHS_FROM:
        case CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            if (!miscValue1 || miscValue2 != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_COMPLETE_QUEST:
        {
            // if miscValues != 0, it contains the questID.
            if (miscValue1)
            {
                if (miscValue1 != criteria->Asset)
                    return false;
            }
            else
            {
                // login case.
                if (!referencePlayer->GetQuestRewardStatus(criteria->Asset))
                    return false;
            }

            if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(tree))
                if (!data->Meets(cachePtr))
                    return false;
            break;
        }
        case CRITERIA_TYPE_KILLED_BY_CREATURE:
        case CRITERIA_TYPE_BE_SPELL_TARGET:
        case CRITERIA_TYPE_BE_SPELL_TARGET2:
        case CRITERIA_TYPE_CAST_SPELL:
        case CRITERIA_TYPE_CAST_SPELL2:
        case CRITERIA_TYPE_LOOT_ITEM:
        case CRITERIA_TYPE_DO_EMOTE:
        case CRITERIA_TYPE_EQUIP_ITEM:
        case CRITERIA_TYPE_USE_GAMEOBJECT:
        case CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
        case CRITERIA_TYPE_HK_CLASS:
        case CRITERIA_TYPE_HK_RACE:
        case CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
        case CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
        case CRITERIA_TYPE_INSTANSE_MAP_ID:
        case CRITERIA_TYPE_WIN_ARENA:
        case CRITERIA_TYPE_COMPLETE_INSTANCE:
        case CRITERIA_TYPE_PLAY_ARENA:
        case CRITERIA_TYPE_OWN_RANK:
        case CRITERIA_TYPE_SCRIPT_EVENT:
        case CRITERIA_TYPE_SCRIPT_EVENT_2:
        case CRITERIA_TYPE_SCRIPT_EVENT_3:
        case CRITERIA_TYPE_ADD_BATTLE_PET_JOURNAL:
        case CRITERIA_TYPE_PLACE_GARRISON_BUILDING:
        case CRITERIA_TYPE_CONSTRUCT_GARRISON_BUILDING:
        case CRITERIA_TYPE_COMPLETE_SCENARIO:
        case CRITERIA_TYPE_ENTER_AREA:
        case CRITERIA_TYPE_LEAVE_AREA:
        case CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER:
        case CRITERIA_TYPE_ARCHAEOLOGY_GAMEOBJECT:
        case CRITERIA_TYPE_DUNGEON_ENCOUNTER_COUNTER:
        case CRITERIA_TYPE_REACH_SCENARIO_BOSS:
        case CRITERIA_TYPE_RECRUIT_TRANSPORT_FOLLOWER:
            if (!miscValue1 || miscValue1 != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_LEARN_SPELL:
            if (miscValue1 && miscValue1 != criteria->Asset)
                return false;

            if (!referencePlayer->HasSpell(criteria->Asset))
                return false;
            break;
        case CRITERIA_TYPE_LOOT_TYPE:
            // miscValue1 = loot_type (note: 0 = LOOT_CORPSE and then it ignored)
            // miscValue2 = count of item loot
            if (!miscValue1 || !miscValue2 || miscValue1 != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_OWN_ITEM:
        case CRITERIA_TYPE_RELIC_TALENT_UNLOCKED:
            if (miscValue1 && criteria->Asset != miscValue1)
                return false;
            break;
        case CRITERIA_TYPE_EXPLORE_AREA:
        {
            WorldMapOverlayEntry const* worldOverlayEntry = sWorldMapOverlayStore.LookupEntry(criteria->Asset);
            if (!worldOverlayEntry)
                break;

            // if (!sConditionMgr->IsPlayerMeetingCondition(const_cast<Unit*>(unit), worldOverlayEntry->PlayerConditionID))
                // return false;

                // those requirements couldn't be found in the dbc
            if (AchievementCriteriaDataSet const* data = sAchievementMgr->GetCriteriaDataSet(tree))
                if (data->Meets(cachePtr))
                    return true;

            bool matchFound = false;
            for (uint32 j : worldOverlayEntry->AreaID)
            {
                AreaTableEntry const* area = sAreaTableStore.LookupEntry(j);
                if (!area)
                    break;

                if (area->AreaBit < 0)
                    continue;

                uint32 playerIndexOffset = uint32(area->AreaBit) / 32;
                if (playerIndexOffset >= PLAYER_EXPLORED_ZONES_SIZE)
                    continue;

                uint32 mask = 1 << (uint32(area->AreaBit) % 32);
                if (referencePlayer->GetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + playerIndexOffset) & mask)
                {
                    matchFound = true;
                    break;
                }
            }

            if (!matchFound)
                return false;
            break;
        }
        case CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
        {
            // miscValue1 = itemid miscValue2 = diced value
            if (!miscValue1 || miscValue2 != criteria->Asset)
                return false;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(miscValue1);
            if (!proto)
                return false;
            break;
        }
        case CRITERIA_TYPE_DAMAGE_DONE:
        case CRITERIA_TYPE_HEALING_DONE:
            if (!miscValue1)
                return false;

            if (criteria->StartEvent == CRITERIA_CONDITION_BG_MAP)
            {
                if (cachePtr->MapID != criteria->StartAsset)
                    return false;

                // map specific case (BG in fact) expected player targeted damage/heal
                if (!cachePtr->HasTarget() || cachePtr->IsCreature())
                    return false;
            }
            break;
        case CRITERIA_TYPE_LOOT_EPIC_ITEM:
        case CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
        {
            if (!miscValue1)
                return false;
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(miscValue1);
            if (!proto || proto->GetQuality() < ITEM_QUALITY_EPIC)
                return false;
            break;
        }
        case CRITERIA_TYPE_CURRENCY:
            if (!miscValue1 || !miscValue2 || int64(miscValue2) < 0 || miscValue1 != criteria->Asset)
                return false;
            break;
        case CRITERIA_TYPE_LEVEL_BATTLE_PET_CREDIT:
        {
            if (!miscValue1 || miscValue2 < uint32(criteria->Asset))
                return false;          
            break;
        }
        case CRITERIA_TYPE_BATTLEPET_LEVEL_UP:
        {
            if (miscValue1 < uint32(criteria->Asset))
                return false;
            break;
        }
        case CRITERIA_TYPE_START_GARRISON_MISSION:
        case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION:
            return true;
        default:
            break;
    }
    return true;
}

template<class T>
bool AchievementMgr<T>::CheckModifierTree(uint32 modifierTreeId, Player* referencePlayer)
{
    AchievementCachePtr referenceCache = std::make_shared<AchievementCache>(referencePlayer);

    ModifierTreeNode const* tree = sAchievementMgr->GetModifierTree(modifierTreeId);
    return AdditionalRequirementsSatisfied(tree, referenceCache);
}

template<class T>
bool AchievementMgr<T>::CheckModifierTree(uint32 modifierTreeId, AchievementCachePtr cachePtr)
{
    ModifierTreeNode const* tree = sAchievementMgr->GetModifierTree(modifierTreeId);
    return AdditionalRequirementsSatisfied(tree, cachePtr);
}

template<class T>
bool AchievementMgr<T>::AdditionalRequirementsSatisfied(ModifierTreeNode const* tree, AchievementCachePtr cachePtr)
{
    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr::AdditionalRequirementsSatisfied start miscValue1 %u%u, miscValue2 %u%u miscValue3 %u%u ModifierTree %u", cachePtr->miscValue1, cachePtr->miscValue2, cachePtr->miscValue3, tree->Entry->ID);

    if (m_canUpdateAchiev != 1)
        return false;

    if (tree->Entry->ID == 1355 && sGameEventMgr->IsActiveEvent(192)) // hack
        return true;

    int32 saveReqType = -1;
    int32 count = 0;
    bool saveCheck = false;

    Player* referencePlayer = cachePtr->player;
    uint32 miscValue1 = cachePtr->miscValue1;
    uint32 miscValue2 = cachePtr->miscValue2;
    uint32 miscValue3 = cachePtr->miscValue3;

    for (ModifierTreeNode const* node : tree->Children)
    {
        if (!node->Children.empty())
            if (!AdditionalRequirementsSatisfied(node, cachePtr))
                continue;

        uint32 reqType = node->Entry->Type;
        if (!reqType)
            return true;

        uint32 reqValue = node->Entry->Asset;
        uint32 reqCount = node->Entry->SecondaryAsset;
        bool check = true;

        switch (CriteriaAdditionalCondition(reqType))
        {
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_DRUNK_VALUE: // 1
                if (cachePtr->DrunkValue < reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_PLAYER_CONDITION: // 2
            {
                if (PlayerConditionEntry const* condition = sPlayerConditionStore.LookupEntry(reqValue))
                    check = sConditionMgr->IsPlayerMeetingCondition(referencePlayer, condition);
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_ITEM_LEVEL: // 3
            {
                // miscValue1 is itemid
                Item* item = referencePlayer->GetItemByEntry(miscValue1);
                if (!item || item->GetItemLevel(cachePtr->Level) < reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_ENTRY: // 4
                if (!cachePtr->HasTarget() || cachePtr->target.Entry != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_PLAYER: // 5
                if (!cachePtr->HasTarget() || cachePtr->IsCreature())
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_DEAD: // 6
                if (!cachePtr->HasTarget() || cachePtr->target.isAlive)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_ENEMY: // 7
                if (!cachePtr->HasTarget())
                    check = false;
                else if (cachePtr->target.Team == cachePtr->Team)
                        check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_HAS_AURA: // 8
                if (!cachePtr->player->HasAura(reqValue))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_HAS_AURA: // 10
                if (!cachePtr->HasTarget() || !cachePtr->target.unit->HasAura(reqValue))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_MUST_BE_MOUNTED: // 11
                if (!cachePtr->HasTarget() || !cachePtr->target.IsMounted)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_ITEM_QUALITY_MIN: // 14
            case CRITERIA_ADDITIONAL_CONDITION_ITEM_QUALITY_EQUALS: // 15
            {
                // miscValue1 is itemid
                Item* item = referencePlayer->GetItemByEntry(miscValue1);
                if (!item || item->GetQuality() < reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_AREA_OR_ZONE: // 17
                if (referencePlayer->GetCurrentZoneID() != reqValue && referencePlayer->GetCurrentAreaID() != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_AREA_OR_ZONE: // 18
            {
                if (!cachePtr->HasTarget())
                    check = false;
                else
                {
                    if (cachePtr->target.ZoneID != reqValue && cachePtr->target.AreaID != reqValue)
                        check = false;
                }
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_ITEM_ENTRY: // 19
                if (miscValue1 != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_MAP_DIFFICULTY: // 20
                if (CreatureTemplate::GetDiffFromSpawn(cachePtr->Difficulty) != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_YIELDS_XP: // 21
            {
                if (!cachePtr->HasTarget())
                    check = false;
                else
                    check = cachePtr->target.isYields;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_ARENA_TEAM_SIZE: // 24
            {
                Battleground* bg = referencePlayer->GetBattleground();
                if (!cachePtr->OnBG || !cachePtr->IsArena || !cachePtr->IsRated || cachePtr->JoinType != reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_RACE: // 25
                if (cachePtr->RaceID != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_CLASS: // 26
                if (cachePtr->ClassID != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_RACE: // 27
                if (!cachePtr->HasTarget() || cachePtr->IsCreature() || cachePtr->target.RaceID != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_CLASS: // 28
                if (!cachePtr->HasTarget() || cachePtr->IsCreature() || cachePtr->target.ClassID != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_MAX_GROUP_MEMBERS: // 29
                if (cachePtr->HaveGroup && cachePtr->MembersCount >= reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_CREATURE_TYPE: // 30
                if (miscValue1 != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_MAP: // 32
                if (cachePtr->MapID != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BUILD_VERSION: // 33
                if (reqValue >= 70199) // Current version
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_LEVEL_IN_SLOT: // 34
                if (!referencePlayer)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_WITHOUT_GROUP: // 35
                if (cachePtr->HaveGroup)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_MIN_PERSONAL_RATING: // 37
            {
                if (!cachePtr->OnBG || !cachePtr->IsArena || !cachePtr->IsRated)
                    check = false;
                else
                {
                    Bracket* bracket = referencePlayer->getBracket(MS::Battlegrounds::GetBracketByJoinType(cachePtr->JoinType));
                    if (!bracket || bracket->getRating() < reqValue)
                        check = false;
                }
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_TITLE_BIT_INDEX: // 38
                // miscValue1 is title's bit index
                if (miscValue1 != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_LEVEL: // 39
                if (cachePtr->Level != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_LEVEL: // 40
                if (!cachePtr->HasTarget() || cachePtr->target.Level != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_ZONE: // 41
                if (cachePtr->ZoneID != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_SOURCE_NOT_ZONE: // 42
                if (cachePtr->ZoneID == reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_HEALTH_PERCENT_BELOW: // 46
                if (!cachePtr->HasTarget() || cachePtr->HealthPct >= reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_MIN_ACHIEVEMENT_POINTS: // 56
                if (static_cast<int32>(_achievementPoints) < reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_REQUIRES_LFG_GROUP: // 58
                if (!cachePtr->HaveGroup || !cachePtr->isLFGGroup)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_REQUIRES_GUILD_GROUP: // 61
            {
                if (!cachePtr->HaveGroup || !cachePtr->guildId || !cachePtr->IsGuildGroup)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_GUILD_REPUTATION: // 62
                if (referencePlayer->GetReputationMgr().GetReputation(REP_GUILD) < int32(reqValue))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_RATED_BATTLEGROUND: // 63
            {
                if (!cachePtr->OnBG || !cachePtr->IsRBG)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_PROJECT_RARITY: // 65
            {
                if (!miscValue1)
                {
                    check = false;
                    break;
                }

                ResearchProjectEntry const* rp = sResearchProjectStore.LookupEntry(miscValue1);
                if (!rp)
                    check = false;
                else
                {
                    if (rp->Rarity != reqValue)
                        check = false;

                    if (referencePlayer->IsCompletedProject(rp->ID, false))
                        check = false;
                }
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_PROJECT_RACE: // 66
            {
                if (!miscValue1)
                {
                    check = false;
                    break;
                }

                ResearchProjectEntry const* rp = sResearchProjectStore.LookupEntry(miscValue1);
                if (!rp)
                    check = false;
                else if (rp->ResearchBranchID != reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_WORLD_STATE_EXPRESSION:  // 67
            {
                //WorldStateExpressionEntry const* worldStateExpEntry = sWorldStateExpressionStore.LookupEntry(reqValue);
                //if (!worldStateExpEntry || !worldStateExpEntry->Eval(referencePlayer))
                //    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_DUNGEON_DIFFICULTY: // 68
                if (cachePtr->Difficulty != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_MIN_LEVEL: // 69
                if (cachePtr->Level >= reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_MIN_LEVEL: // 70
                if (cachePtr->target.Level >= reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_TARGET_MAX_LEVEL: // 71
                if (cachePtr->target.Level < reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_ACTIVE_SCENARIO: // 74
            {
                if (!referencePlayer || !referencePlayer->InInstance())
                {
                    check = false;
                    break;
                }

                InstanceMap* inst = referencePlayer->GetMap()->ToInstanceMap();
                if (uint32 instanceId = inst ? inst->GetInstanceId() : 0)
                    if (Scenario* progress = sScenarioMgr->GetScenario(instanceId))
                        if (progress->GetScenarioId() != reqValue)
                            check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_ACHIEV_POINTS: // 76
                if (static_cast<int32>(_achievementPoints) < reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_FEMALY: // 78
                if (!miscValue3 || miscValue3 != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_HP_LOW_THAT: // 79
                check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_COUNT_OF_GUILD_MEMBER_IN_GROUP:     // 80
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }

                Group* group = referencePlayer->GetGroup();
                uint32 guildId = referencePlayer->GetGuildId();
                if (!guildId || !group)
                {
                    check = false;
                    break;
                }

                uint32 counter = 0;
                for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                {
                    if (Player* player = groupRef->getSource())
                    {
                        if (player->GetGuildId() == guildId)
                            ++counter;
                    }
                }

                if (counter < reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_MASTER_PET_TAMER: // 81
                if (miscValue2 != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_CHALANGER_RATE: // 83
                if (!miscValue2)
                    check = false;
                else if (reqValue > miscValue2)            // Medal check
                    check = false;
                break;
            // case CRITERIA_ADDITIONAL_CONDITION_INCOMPLETE_QUEST: // 84
            // {
                // if (!referencePlayer)
                // {
                    // check = false;
                    // break;
                // }

                // QuestStatusData* q_status = referencePlayer->getQuestStatus(reqValue);
                // if (!q_status)
                    // check = false;
                // else if (q_status->Status != QUEST_STATUS_INCOMPLETE)
                    // check = false;
                // break;
            // }
            // case CRITERIA_ADDITIONAL_CONDITION_COMPLETE_ACHIEVEMENT: // 86 need more research
            case CRITERIA_ADDITIONAL_CONDITION_NOT_COMPLETE_ACHIEVEMENT: // 87
                if (_completedAchievementsArr[reqValue] == nullptr)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_REPUTATION_UNK: // 88
                if (!referencePlayer)
                    check = false;
                else if (referencePlayer->GetReputationMgr().GetReputation(miscValue1) < int32(reqValue))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_QUALITY: // 89
                if (!miscValue2 || (miscValue2 < (reqValue - 7)))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_WIN_IN_PVP: // 90
                if (!miscValue1 || miscValue1 != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_SPECIES: // 91
            {
                if (!miscValue3)
                {
                    check = false;
                    break;
                }

                auto const& speciesInfo = sBattlePetSpeciesStore.LookupEntry(miscValue3);
                if (!speciesInfo)
                {
                    check = false;
                    break;
                }

                if (speciesInfo->ID != reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_EXPANSION_LESS: // 92
                if (reqValue >= static_cast<int32>(CURRENT_EXPANSION))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_REPUTATION: // 95
                if (referencePlayer->GetReputationMgr().GetReputation(reqValue) < int32(reqCount))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_ITEM_CLASS_AND_SUBCLASS: // 96
            {
                // miscValue1 is itemid
                ItemTemplate const * item = sObjectMgr->GetItemTemplate(miscValue1);
                if (!item || item->GetClass() != reqValue || item->GetSubClass() != reqCount)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_REACH_SKILL_LEVEL: // 99
                if (referencePlayer->GetBaseSkillValue(reqValue) < reqCount)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_NOT_HAVE_ACTIVE_SPELL: // 104
                if (!referencePlayer)
                    check = false;
                else if (referencePlayer->HasActiveSpell(reqValue))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_HAS_ITEM_COUNT: // 105
            case CRITERIA_ADDITIONAL_CONDITION_ITEM_COUNT: // 114
                if (!referencePlayer)
                    check = false;
                else if (referencePlayer->GetItemCount(reqValue) != reqCount)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_REQ_ADDON: // 106
                if (reqValue != 7) // legion 7.0
                    check = false;
                break;
            // case CRITERIA_ADDITIONAL_CONDITION_QUEST_STATUS: // 110
            // {
                // if (!referencePlayer)
                // {
                    // check = false;
                    // break;
                // }

                // if (referencePlayer->GetQuestStatus(reqValue) == QUEST_STATUS_NONE)
                    // check = false;
                // break;
            // }
            case CRITERIA_ADDITIONAL_CONDITION_CURRENCY_COUNT: // 119
                if (static_cast<int32>(referencePlayer->GetCurrency(reqValue)) != reqCount)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_CURRENCY_ON_SEASON: // 121
                if (static_cast<int32>(referencePlayer->GetCurrencyOnSeason(reqValue)) <= reqCount)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_DEATH_COUNTER:   // 122
                if (!referencePlayer || !referencePlayer->InInstance() || referencePlayer->isAlive())
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_BONUS_EVENT_WEEKEND: // 123
                if (referencePlayer->GetMap() && (referencePlayer->GetMap()->GetLootDifficulty() != DIFFICULTY_TIMEWALKING))
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_ARENA_SEASON: // 125
                if (sWorldStateMgr.GetWorldStateValue(WS_ARENA_SEASON_ID) != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_GARRISON_FOLLOWER: // 144
            case CRITERIA_ADDITIONAL_CONDITION_GARRISON_FOLLOWER_ID: // 157
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }
                Garrison* garrison = referencePlayer->GetGarrisonPtr();
                if (!garrison)
                {
                    check = false;
                    break;
                }
                if (!miscValue1 || miscValue1 != reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_GARRISON_FOLLOWER_QUALITY: // 145
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }
                Garrison* garrison = referencePlayer->GetGarrisonPtr();
                if (!garrison)
                {
                    check = false;
                    break;
                }
                
                auto const& follower = garrison->GetFollower(miscValue1);
                if (!follower || follower->PacketInfo.Quality != reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_GARRISON_FOLLOWER_LEVEL: // 146
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }
                Garrison* garrison = referencePlayer->GetGarrisonPtr();
                if (!garrison)
                {
                    check = false;
                    break;
                }
                
                auto const& follower = garrison->GetFollower(miscValue1);
                if (!follower || follower->PacketInfo.FollowerLevel < reqValue)
                    check = false;

                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_SPECIES_IN_SLOT: // 151
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }

                auto petSlots = referencePlayer->GetBattlePetCombatTeam();
                uint8 counter = 0;
                for (auto i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
                {
                    if (!petSlots[i])
                        continue;

                    if (petSlots[i]->Species == reqCount)
                        counter++;
                }

                if (counter < reqValue)
                    check = false;

                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_BATTLEPET_TYPE_IN_SLOT: // 152
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }

                auto petSlots = referencePlayer->GetBattlePetCombatTeam();
                uint8 counter = 0;
                for (auto i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
                {
                    if (!petSlots[i])
                        continue;

                    auto const& speciesInfo = sBattlePetSpeciesStore.LookupEntry(petSlots[i]->Species);
                    if (speciesInfo && speciesInfo->PetTypeEnum == reqCount)
                        counter++;
                }

                if (counter < reqValue)
                    check = false;

                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_GROUP_SIZE: // 171
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }
                if (!cachePtr->HaveGroup)
                {
                    check = false;
                    break;
                }

                if (cachePtr->MembersCount != reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_CURRENCY: // 172
                if (reqValue != miscValue1)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_GARRISON_TALENT: // 202
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }

                auto garrison = referencePlayer->GetGarrisonPtr();
                if (!garrison)
                {
                    check = false;
                    break;
                }

                if (!garrison->hasTallent(reqValue))
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_GARRISON_FOLLOWERS_ILVL: // 184
            {
                if (!referencePlayer)
                {
                    check = false;
                    break;
                }
                Garrison* garrison = referencePlayer->GetGarrisonPtr();
                if (!garrison)
                {
                    check = false;
                    break;
                }
                if (garrison->GetCountFolowerItemLvl(reqCount) < reqValue)
                    check = false;
                break;
            }
            case CRITERIA_ADDITIONAL_CONDITION_HONOR_LEVEL: // 193
                if (!referencePlayer || referencePlayer->GetHonorLevel() != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_PRESTIGE_LEVEL: // 194
                if (!referencePlayer || referencePlayer->GetPrestigeLevel() != reqValue)
                    check = false;
                break;
            case CRITERIA_ADDITIONAL_CONDITION_WORLD_QUEST_TYPE: // 206
                if (miscValue3 != reqValue)
                    check = false;
                break;
            default:
                break;
        }

        // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr::AdditionalRequirementsSatisfied cheker end Modify %i, reqType %i reqValue %i reqCount %i saveCheck %i saveReqType %i check %i count %u Amount %u",
        // node->Entry->ID, reqType, reqValue, reqCount, saveCheck, saveReqType, check, count, tree->Entry->Amount);

        if (saveReqType == -1)
        {
            if (check && reqType) //don`t save if false
                count++;

            saveReqType = reqType;
            saveCheck = check;
        }
        else if (saveReqType != reqType)
        {
            if (!saveCheck || !check)
                return false;
        }
        else if (saveReqType == reqType)
        {
            if (check)
                count++;
            if (count >= tree->Entry->Amount)
                return true;
        }
    }

    // TC_LOG_DEBUG(LOG_FILTER_ACHIEVEMENTSYS, "AchievementMgr::AdditionalRequirementsSatisfied end ModifierTreeId %i, saveCheck %u saveReqType %i count %u Amount %u", tree->Entry->ID, saveCheck, saveReqType, count, tree->Entry->Amount);
    return saveCheck;
}

template <typename Func>
void AchievementGlobalMgr::WalkCriteriaTree(CriteriaTree const* tree, Func const& func)
{
    for (CriteriaTree const* node : tree->Children)
        WalkCriteriaTree(node, func);
    func(tree);
}

template<class T>
CriteriaSort AchievementMgr<T>::GetCriteriaSort() const
{
    return PLAYER_CRITERIA;
}

template<>
CriteriaSort AchievementMgr<Guild>::GetCriteriaSort() const
{
    return GUILD_CRITERIA;
}

template<>
CriteriaSort AchievementMgr<Scenario>::GetCriteriaSort() const
{
    return SCENARIO_CRITERIA;
}

char const* AchievementGlobalMgr::GetCriteriaTypeString(uint32 type)
{
    return GetCriteriaTypeString(CriteriaTypes(type));
}

AchievementGlobalMgr* AchievementGlobalMgr::instance()
{
    static AchievementGlobalMgr instance;
    return &instance;
}

CriteriaTreeList const& AchievementGlobalMgr::GetCriteriaTreeByType(CriteriaTypes type, CriteriaSort sort) const
{
    if (sort == PLAYER_CRITERIA)
        return _criteriasByType[type];
    if (sort == GUILD_CRITERIA)
        return _guildCriteriasByType[type];
    return _scenarioCriteriasByType[type];
}

CriteriaTreeList const* AchievementGlobalMgr::GetCriteriaTreesByCriteria(uint32 criteriaId) const
{
    return _criteriaTreeByCriteriaVector[criteriaId];
}

CriteriaTreeList const& AchievementGlobalMgr::GetTimedCriteriaByType(CriteriaTimedTypes type) const
{
    return _criteriasByTimedType[type];
}

AchievementEntryList const* AchievementGlobalMgr::GetAchievementByReferencedId(uint32 id) const
{
    return &m_AchievementListByReferencedId[id];
}

AchievementReward const* AchievementGlobalMgr::GetAchievementReward(AchievementEntry const* achievement) const
{
    return m_achievementRewardVector[achievement->ID];
}

AchievementRewardLocale const* AchievementGlobalMgr::GetAchievementRewardLocale(AchievementEntry const* achievement) const
{
    return Trinity::Containers::MapGetValuePtr(m_achievementRewardLocales, achievement->ID);
}

AchievementCriteriaDataSet const* AchievementGlobalMgr::GetCriteriaDataSet(CriteriaTree const* Criteria) const
{
    return m_criteriaDataVector[Criteria->CriteriaID];
}

bool AchievementGlobalMgr::IsRealmCompleted(AchievementEntry const* achievement) const
{
    return m_allCompletedAchievements[achievement->ID];
}

void AchievementGlobalMgr::SetRealmCompleted(AchievementEntry const* achievement)
{
    m_allCompletedAchievements[achievement->ID] = true;
}

bool AchievementGlobalMgr::IsGroupCriteriaType(CriteriaTypes type) const
{
    switch (type)
    {
        case CRITERIA_TYPE_KILL_CREATURE:
        case CRITERIA_TYPE_WIN_BG:
        case CRITERIA_TYPE_BE_SPELL_TARGET: // NYI
        case CRITERIA_TYPE_WIN_RATED_ARENA:
        case CRITERIA_TYPE_BE_SPELL_TARGET2: // NYI
        case CRITERIA_TYPE_WIN_BATTLEGROUND: // NYI
            return true;
        default:
            break;
    }
    return false;
}

char const* AchievementGlobalMgr::GetCriteriaTypeString(CriteriaTypes type)
{
    switch (type)
    {
        case CRITERIA_TYPE_KILL_CREATURE:
            return "KILL_CREATURE";
        case CRITERIA_TYPE_WIN_BG:
            return "TYPE_WIN_BG";
        case CRITERIA_TYPE_COMPLETE_ARCHAEOLOGY_PROJECTS:
            return "COMPLETE_RESEARCH";
        case CRITERIA_TYPE_REACH_LEVEL:
            return "REACH_LEVEL";
        case CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return "REACH_SKILL_LEVEL";
        case CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return "COMPLETE_ACHIEVEMENT";
        case CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            return "COMPLETE_QUEST_COUNT";
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY:
            return "COMPLETE_DAILY_QUEST_DAILY";
        case CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return "COMPLETE_QUESTS_IN_ZONE";
        case CRITERIA_TYPE_CURRENCY:
            return "CURRENCY";
        case CRITERIA_TYPE_DAMAGE_DONE:
            return "DAMAGE_DONE";
        case CRITERIA_TYPE_COMPLETE_DAILY_QUEST:
            return "COMPLETE_DAILY_QUEST";
        case CRITERIA_TYPE_COMPLETE_BATTLEGROUND:
            return "COMPLETE_BATTLEGROUND";
        case CRITERIA_TYPE_DEATH_AT_MAP:
            return "DEATH_AT_MAP";
        case CRITERIA_TYPE_DEATH:
            return "DEATH";
        case CRITERIA_TYPE_DEATH_IN_DUNGEON:
            return "DEATH_IN_DUNGEON";
        case CRITERIA_TYPE_COMPLETE_INSTANCE:
            return "COMPLETE_RAID";
        case CRITERIA_TYPE_KILLED_BY_CREATURE:
            return "KILLED_BY_CREATURE";
        case CRITERIA_TYPE_KILLED_BY_PLAYER:
            return "KILLED_BY_PLAYER";
        case CRITERIA_TYPE_FALL_WITHOUT_DYING:
            return "FALL_WITHOUT_DYING";
        case CRITERIA_TYPE_DEATHS_FROM:
            return "DEATHS_FROM";
        case CRITERIA_TYPE_COMPLETE_QUEST:
            return "COMPLETE_QUEST";
        case CRITERIA_TYPE_BE_SPELL_TARGET:
            return "BE_SPELL_TARGET";
        case CRITERIA_TYPE_CAST_SPELL:
            return "CAST_SPELL";
        case CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE:
            return "BG_OBJECTIVE_CAPTURE";
        case CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return "HONORABLE_KILL_AT_AREA";
        case CRITERIA_TYPE_WIN_ARENA:
            return "WIN_ARENA";
        case CRITERIA_TYPE_PLAY_ARENA:
            return "PLAY_ARENA";
        case CRITERIA_TYPE_LEARN_SPELL:
            return "LEARN_SPELL";
        case CRITERIA_TYPE_HONORABLE_KILL:
            return "HONORABLE_KILL";
        case CRITERIA_TYPE_OWN_ITEM:
            return "OWN_ITEM";
        case CRITERIA_TYPE_WIN_RATED_ARENA:
            return "WIN_RATED_ARENA";
        case CRITERIA_TYPE_HIGHEST_TEAM_RATING:
            return "HIGHEST_TEAM_RATING";
        case CRITERIA_TYPE_HIGHEST_PERSONAL_RATING:
            return "HIGHEST_PERSONAL_RATING";
        case CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return "LEARN_SKILL_LEVEL";
        case CRITERIA_TYPE_USE_ITEM:
            return "USE_ITEM";
        case CRITERIA_TYPE_LOOT_ITEM:
            return "LOOT_ITEM";
        case CRITERIA_TYPE_EXPLORE_AREA:
            return "EXPLORE_AREA";
        case CRITERIA_TYPE_OWN_RANK:
            return "OWN_RANK";
        case CRITERIA_TYPE_BUY_BANK_SLOT:
            return "BUY_BANK_SLOT";
        case CRITERIA_TYPE_GAIN_REPUTATION:
            return "GAIN_REPUTATION";
        case CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            return "GAIN_EXALTED_REPUTATION";
        case CRITERIA_TYPE_VISIT_BARBER_SHOP:
            return "VISIT_BARBER_SHOP";
        case CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            return "EQUIP_EPIC_ITEM";
        case CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
            return "ROLL_NEED_ON_LOOT";
        case CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return "GREED_ON_LOOT";
        case CRITERIA_TYPE_HK_CLASS:
            return "HK_CLASS";
        case CRITERIA_TYPE_HK_RACE:
            return "HK_RACE";
        case CRITERIA_TYPE_DO_EMOTE:
            return "DO_EMOTE";
        case CRITERIA_TYPE_HEALING_DONE:
            return "HEALING_DONE";
        case CRITERIA_TYPE_GET_KILLING_BLOWS:
            return "GET_KILLING_BLOWS";
        case CRITERIA_TYPE_EQUIP_ITEM:
            return "EQUIP_ITEM";
        case CRITERIA_TYPE_MONEY_FROM_VENDORS:
            return "MONEY_FROM_VENDORS";
        case CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS:
            return "GOLD_SPENT_FOR_TALENTS";
        case CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS:
            return "NUMBER_OF_TALENT_RESETS";
        case CRITERIA_TYPE_MONEY_FROM_QUEST_REWARD:
            return "MONEY_FROM_QUEST_REWARD";
        case CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING:
            return "GOLD_SPENT_FOR_TRAVELLING";
        case CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            return "GOLD_SPENT_AT_BARBER";
        case CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL:
            return "GOLD_SPENT_FOR_MAIL";
        case CRITERIA_TYPE_LOOT_MONEY:
            return "LOOT_MONEY";
        case CRITERIA_TYPE_USE_GAMEOBJECT:
            return "USE_GAMEOBJECT";
        case CRITERIA_TYPE_BE_SPELL_TARGET2:
            return "BE_SPELL_TARGET2";
        case CRITERIA_TYPE_SPECIAL_PVP_KILL:
            return "SPECIAL_PVP_KILL";
        case CRITERIA_TYPE_FISH_IN_GAMEOBJECT:
            return "FISH_IN_GAMEOBJECT";
        case CRITERIA_TYPE_EARNED_PVP_TITLE:
            return "EARNED_PVP_TITLE";
        case CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS:
            return "LEARN_SKILLLINE_SPELLS";
        case CRITERIA_TYPE_WIN_DUEL:
            return "WIN_DUEL";
        case CRITERIA_TYPE_LOSE_DUEL:
            return "LOSE_DUEL";
        case CRITERIA_TYPE_KILL_CREATURE_TYPE:
            return "KILL_CREATURE_TYPE";
        case CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS:
            return "GOLD_EARNED_BY_AUCTIONS";
        case CRITERIA_TYPE_CREATE_AUCTION:
            return "CREATE_AUCTION";
        case CRITERIA_TYPE_HIGHEST_AUCTION_BID:
            return "HIGHEST_AUCTION_BID";
        case CRITERIA_TYPE_WON_AUCTIONS:
            return "WON_AUCTIONS";
        case CRITERIA_TYPE_HIGHEST_AUCTION_SOLD:
            return "HIGHEST_AUCTION_SOLD";
        case CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED:
            return "HIGHEST_GOLD_VALUE_OWNED";
        case CRITERIA_TYPE_GAIN_REVERED_REPUTATION:
            return "GAIN_REVERED_REPUTATION";
        case CRITERIA_TYPE_GAIN_HONORED_REPUTATION:
            return "GAIN_HONORED_REPUTATION";
        case CRITERIA_TYPE_KNOWN_FACTIONS:
            return "KNOWN_FACTIONS";
        case CRITERIA_TYPE_LOOT_EPIC_ITEM:
            return "LOOT_EPIC_ITEM";
        case CRITERIA_TYPE_RECEIVE_EPIC_ITEM:
            return "RECEIVE_EPIC_ITEM";
        case CRITERIA_TYPE_ROLL_NEED:
            return "ROLL_NEED";
        case CRITERIA_TYPE_ROLL_GREED:
            return "ROLL_GREED";
        case CRITERIA_TYPE_HIGHEST_HIT_DEALT:
            return "HIT_DEALT";
        case CRITERIA_TYPE_HIGHEST_HIT_RECEIVED:
            return "HIT_RECEIVED";
        case CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED:
            return "TOTAL_DAMAGE_RECEIVED";
        case CRITERIA_TYPE_HIGHEST_HEAL_CASTED:
            return "HIGHEST_HEAL_CASTED";
        case CRITERIA_TYPE_TOTAL_HEALING_RECEIVED:
            return "TOTAL_HEALING_RECEIVED";
        case CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED:
            return "HIGHEST_HEALING_RECEIVED";
        case CRITERIA_TYPE_QUEST_ABANDONED:
            return "QUEST_ABANDONED";
        case CRITERIA_TYPE_FLIGHT_PATHS_TAKEN:
            return "FLIGHT_PATHS_TAKEN";
        case CRITERIA_TYPE_LOOT_TYPE:
            return "LOOT_TYPE";
        case CRITERIA_TYPE_CAST_SPELL2:
            return "CAST_SPELL2";
        case CRITERIA_TYPE_LEARN_SKILL_LINE:
            return "LEARN_SKILL_LINE";
        case CRITERIA_TYPE_EARN_HONORABLE_KILL:
            return "EARN_HONORABLE_KILL";
        case CRITERIA_TYPE_ACCEPTED_SUMMONINGS:
            return "ACCEPTED_SUMMONINGS";
        case CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS:
            return "EARN_ACHIEVEMENT_POINTS";
        case CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS:
            return "USE_LFD_TO_GROUP_WITH_PLAYERS";
        case CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS:
            return "SPENT_GOLD_GUILD_REPAIRS";
        case CRITERIA_TYPE_REACH_GUILD_LEVEL:
            return "REACH_GUILD_LEVEL";
        case CRITERIA_TYPE_CRAFT_ITEMS_GUILD:
            return "CRAFT_ITEMS_GUILD";
        case CRITERIA_TYPE_CATCH_FROM_POOL:
            return "CATCH_FROM_POOL";
        case CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS:
            return "BUY_GUILD_BANK_SLOTS";
        case CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS:
            return "EARN_GUILD_ACHIEVEMENT_POINTS";
        case CRITERIA_TYPE_WIN_BATTLEGROUND:
            return "WIN_BATTLEGROUND";
        case CRITERIA_TYPE_REACH_RBG_RATING:
            return "REACH_BG_RATING";
        case CRITERIA_TYPE_BUY_GUILD_EMBLEM:
            return "BUY_GUILD_TABARD";
        case CRITERIA_TYPE_COMPLETE_QUESTS_COUNT:
            return "COMPLETE_QUESTS_COUNT";
        case CRITERIA_TYPE_HONORABLE_KILLS_GUILD:
            return "HONORABLE_KILLS_GUILD";
        case CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD:
            return "KILL_CREATURE_TYPE_GUILD";
        case CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER:
            return "COMPLETE_DUNGEON_ENCOUNTER";
        case CRITERIA_TYPE_PLACE_GARRISON_BUILDING:
            return "PLACE_GARRISON_BUILDING";
        case CRITERIA_TYPE_UPGRADE_GARRISON_BUILDING:
            return "UPGRADE_GARRISON_BUILDING";
        case CRITERIA_TYPE_CONSTRUCT_GARRISON_BUILDING:
            return "CONSTRUCT_GARRISON_BUILDING";
        case CRITERIA_TYPE_UPGRADE_GARRISON:
            return "UPGRADE_GARRISON";
        case CRITERIA_TYPE_START_GARRISON_MISSION:
            return "START_GARRISON_MISSION";
        case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION_COUNT:
            return "COMPLETE_GARRISON_MISSION_COUNT";
        case CRITERIA_TYPE_COMPLETE_GARRISON_MISSION:
            return "COMPLETE_GARRISON_MISSION";
        case CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER_COUNT:
            return "RECRUIT_GARRISON_FOLLOWER_COUNT";
        case CRITERIA_TYPE_LEARN_GARRISON_BLUEPRINT_COUNT:
            return "LEARN_GARRISON_BLUEPRINT_COUNT";
        case CRITERIA_TYPE_COMPLETE_GARRISON_SHIPMENT:
            return "COMPLETE_GARRISON_SHIPMENT";
        case CRITERIA_TYPE_RAISE_GARRISON_FOLLOWER_ITEM_LEVEL:
            return "RAISE_GARRISON_FOLLOWER_ITEM_LEVEL";
        case CRITERIA_TYPE_RAISE_GARRISON_FOLLOWER_LEVEL:
            return "RAISE_GARRISON_FOLLOWER_LEVEL";
        case CRITERIA_TYPE_OWN_TOY:
            return "OWN_TOY";
        case CRITERIA_TYPE_OWN_TOY_COUNT:
            return "OWN_TOY_COUNT";
        case CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER:
            return "RECRUIT_GARRISON_FOLLOWER";
        case CRITERIA_TYPE_RECRUIT_TRANSPORT_FOLLOWER:
            return "RECRUIT_TRANSPORT_FOLLOWER";
        case CRITERIA_TYPE_OWN_HEIRLOOMS:
            return "OWN_HEIRLOOMS";
        case CRITERIA_TYPE_REACH_SCENARIO_BOSS:
            return "REACH_SCENARIO_BOSS";
        default:
            return "MISSING_TYPE";
    }
}

template class AchievementMgr<Scenario>;
template class AchievementMgr<Guild>;
template class AchievementMgr<Player>;

//==========================================================
template<typename T>
T GetEntry(std::unordered_map<uint32, T> const& map, CriteriaTreeEntry const* tree)
{
    CriteriaTreeEntry const* cur = tree;
    auto itr = map.find(tree->ID);
    while (itr == map.end())
    {
        if (!cur->Parent)
            break;

        cur = sCriteriaTreeStore.LookupEntry(cur->Parent);
        if (!cur)
            break;

        itr = map.find(cur->ID);
    }

    if (itr == map.end())
        return nullptr;

    return itr->second;
};

void AchievementGlobalMgr::LoadCriteriaList()
{
    uint32 oldMSTime = getMSTime();

    if (sCriteriaStore.GetNumRows() == 0)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement criteria.");
        return;
    }

    _criteriaModifiers.assign(MAX_MODIFIER_TREE, nullptr);

    if (sModifierTreeStore.GetNumRows() != 0)
    {
        // Load modifier tree nodes
        for (ModifierTreeEntry const* tree : sModifierTreeStore)
        {
            auto node = new ModifierTreeNode();
            node->Entry = tree;
            _criteriaModifiers[node->Entry->ID] = node;
        }

        // Build tree
        for (auto & _criteriaModifier : _criteriaModifiers)
        {
            if (_criteriaModifier == nullptr || !_criteriaModifier->Entry->Parent)
                continue;

            auto parent = _criteriaModifiers[_criteriaModifier->Entry->Parent];
            if (parent != nullptr)
                parent->Children.push_back(_criteriaModifier);
        }
    }

    _criteriaTreeByCriteriaVector.assign(MAX_CRITERIA, nullptr);
    _criteriaTrees.assign(MAX_CRITERIA_TREE, nullptr);
    _criteria.assign(MAX_CRITERIA, nullptr);

    _criteriaTreeForQuest.assign(MAX_CRITERIA_TREE, 0);

    //support for QUEST_OBJECTIVE_COMPLETE_CRITERIA_TREE
    if (QueryResult result = WorldDatabase.PQuery("SELECT ObjectID, QuestID FROM `quest_objectives` WHERE `Type` = %u", QUEST_OBJECTIVE_COMPLETE_CRITERIA_TREE))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 treeID = fields[0].GetUInt32();
            uint32 QuestID = fields[1].GetUInt32();
            //! if no achieve - add to list
            _criteriaTreeForQuest[treeID] = QuestID;

            if (std::vector<CriteriaTreeEntry const*> const* data = sDB2Manager.GetCriteriaTreeList(treeID))
            {
                for (auto child : *data)
                {
                    _criteriaTreeForQuest[child->ID] = QuestID;
                    if (std::vector<CriteriaTreeEntry const*> const* data2 = sDB2Manager.GetCriteriaTreeList(child->ID))
                        for (auto child2 : *data2)
                        {
                            _criteriaTreeForQuest[child2->ID] = QuestID;
                            if (std::vector<CriteriaTreeEntry const*> const* data3 = sDB2Manager.GetCriteriaTreeList(child2->ID))
                                for (auto child3 : *data3)
                                    _criteriaTreeForQuest[child3->ID] = QuestID;
                        }
                }
            }

        } while (result->NextRow());
    }

    std::unordered_map<uint32 /*criteriaTreeID*/, AchievementEntry const*> achievementCriteriaTreeIds;
    for (AchievementEntry const* achievement : sAchievementStore)
        if (achievement->CriteriaTree)
            achievementCriteriaTreeIds[achievement->CriteriaTree] = achievement;

    std::unordered_map<uint32 /*criteriaTreeID*/, ScenarioStepEntry const*> scenarioCriteriaTreeIds;
    for (ScenarioStepEntry const* scenarioStep : sScenarioStepStore)
        if (scenarioStep->Criteriatreeid)
            scenarioCriteriaTreeIds[scenarioStep->Criteriatreeid] = scenarioStep;

    uint32 criterias = 0;
    uint32 guildCriterias = 0;
    uint32 scenarioCriterias = 0;

    // Load criteria tree nodes
    for (CriteriaTreeEntry const* tree : sCriteriaTreeStore)
    {
        // Find linked achievement
        AchievementEntry const* achievement = GetEntry(achievementCriteriaTreeIds, tree);
        ScenarioStepEntry const* scenarioStep = GetEntry(scenarioCriteriaTreeIds, tree);

        if (!achievement && !scenarioStep && !_criteriaTreeForQuest[tree->ID])
            continue;

        auto criteriaTree = new CriteriaTree();
        criteriaTree->ID = tree->ID;
        criteriaTree->Achievement = achievement;
        criteriaTree->ScenarioStep = scenarioStep;
        criteriaTree->Entry = tree;
        if (_criteriaTreeForQuest[tree->ID])
            criteriaTree->Flags |= CRITERIA_TREE_CUSTOM_FLAG_QUEST;

        _criteriaTrees[criteriaTree->Entry->ID] = criteriaTree;

        if (CriteriaEntry const* criteriaEntry = sCriteriaStore.LookupEntry(tree->CriteriaID))
        {
            if (achievement || _criteriaTreeForQuest[tree->ID])
            {
                if (achievement && achievement->Flags & ACHIEVEMENT_FLAG_GUILD)
                    ++guildCriterias, _guildCriteriasByType[criteriaEntry->Type].push_back(criteriaTree);
                else
                    ++criterias, _criteriasByType[criteriaEntry->Type].push_back(criteriaTree);
            }
            else if (scenarioStep)
                ++scenarioCriterias, _scenarioCriteriasByType[criteriaEntry->Type].push_back(criteriaTree);

            if (criteriaEntry->StartTimer)
                _criteriasByTimedType[criteriaEntry->StartEvent].push_back(criteriaTree);
        }
    }

    // Build tree
    for (auto & _criteriaTree : _criteriaTrees)
    {
        if (_criteriaTree == nullptr || !_criteriaTree->Entry->Parent)
            continue;

        auto parent = _criteriaTrees[_criteriaTree->Entry->Parent];
        if (parent != nullptr)
        {
            parent->Children.push_back(_criteriaTree);
            while (parent != nullptr)
            {
                auto cur = parent;
                parent = _criteriaTrees[parent->Entry->Parent];
                if (parent != nullptr)
                {
                    if (sCriteriaStore.LookupEntry(_criteriaTree->Entry->CriteriaID))
                    {
                        _criteriaTreeByCriteria[_criteriaTree->Entry->CriteriaID].push_back(cur);
                        _criteriaTreeByCriteriaVector[_criteriaTree->Entry->CriteriaID] = &_criteriaTreeByCriteria[_criteriaTree->Entry->CriteriaID];
                    }
                }
            }
        }
        else if (sCriteriaStore.LookupEntry(_criteriaTree->Entry->CriteriaID))
        {
            _criteriaTreeByCriteria[_criteriaTree->Entry->CriteriaID].push_back(_criteriaTree);
            _criteriaTreeByCriteriaVector[_criteriaTree->Entry->CriteriaID] = &_criteriaTreeByCriteria[_criteriaTree->Entry->CriteriaID];
        }
    }

    // Load criteria
    for (CriteriaEntry const* criteriaEntry : sCriteriaStore)
    {
        auto criteria = new Criteria();
        criteria->ID = criteriaEntry->ID;
        criteria->Entry = criteriaEntry;
        auto mod = _criteriaModifiers[criteriaEntry->ModifierTreeId];
        if (mod != nullptr)
            criteria->Modifier = mod;

        _criteria[criteria->ID] = criteria;
    }

    uint32 criter = 0;
    for (auto& p : _criteriaTrees)
    {
        if (!p)
            continue;

        if (Criteria const* criteria = GetCriteria(p->Entry->CriteriaID))
        {
            p->Criteria = criteria;
            p->CriteriaID = p->Entry->CriteriaID;
            criter++;
        }
    }


    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u criteria, %u guild and %u scenario criter %u in %u ms", criterias, guildCriterias, scenarioCriterias, criter, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadAchievementReferenceList()
{
    uint32 oldMSTime = getMSTime();

    if (sAchievementStore.GetNumRows() == 0)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement references.");
        return;
    }

    uint32 count = 0;
    m_AchievementListByReferencedId.resize(MAX_ACHIEVEMENT);

    for (AchievementEntry const* achievement : sAchievementStore)
    {
        if (!achievement || !achievement->SharesCriteria)
            continue;

        m_AchievementListByReferencedId[achievement->SharesCriteria].push_back(achievement);
        ++count;
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u achievement references in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadAchievementCriteriaData()
{
    uint32 oldMSTime = getMSTime();

    m_criteriaDataMap.clear();                              // need for reload case
    m_criteriaDataVector.assign(MAX_CRITERIA_TREE, nullptr);

    QueryResult result = WorldDatabase.Query("SELECT criteria_id, type, value1, value2, ScriptName FROM achievement_criteria_data");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 additional achievement criteria data. DB table `achievement_criteria_data` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 criteria_id = fields[0].GetUInt32();

        CriteriaEntry const* criteria = sCriteriaStore.LookupEntry(criteria_id);

        if (!criteria)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` has data for non-existing criteria (Entry: %u), ignore.", criteria_id);
            WorldDatabase.PExecute("DELETE FROM `achievement_criteria_data` WHERE criteria_id = %u", criteria_id);
            continue;
        }

        uint32 dataType = fields[1].GetUInt8();
        const char* scriptName = fields[4].GetCString();
        uint32 scriptId = 0;
        if (strcmp(scriptName, "") != 0) // not empty
        {
            if (dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_criteria_data` has ScriptName set for non-scripted data type (Entry: %u, type %u), useless data.", criteria_id, dataType);
            else
                scriptId = sObjectMgr->GetScriptId(scriptName);
        }

        AchievementCriteriaData data(dataType, fields[2].GetUInt32(), fields[3].GetUInt32(), scriptId);

        if (!data.IsValid(criteria))
            continue;

        // this will allocate empty data set storage
        AchievementCriteriaDataSet& dataSet = m_criteriaDataMap[criteria_id];
        dataSet.SetCriteriaId(criteria_id);
        m_criteriaDataVector[criteria_id] = &dataSet;

        // add real data only for not NONE data types
        if (data.dataType != ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE)
            dataSet.Add(data);

        // counting data by and data types
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u additional achievement criteria data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadCompletedAchievements()
{
    uint32 oldMSTime = getMSTime();
    m_allCompletedAchievements.assign(MAX_ACHIEVEMENT, false);

    QueryResult result = CharacterDatabase.Query("SELECT achievement FROM character_achievement GROUP BY achievement");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 completed achievements. DB table `character_achievement` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 achievementId = fields[0].GetUInt32();
        const AchievementEntry* achievement = sAchievementStore.LookupEntry(achievementId);
        if (!achievement)
        {
            // Remove non existent achievements from all characters
            TC_LOG_ERROR(LOG_FILTER_ACHIEVEMENTSYS, "Non-existing achievement %u data removed from table `character_achievement`.", achievementId);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_INVALID_ACHIEVMENT);

            stmt->setUInt32(0, uint32(achievementId));

            CharacterDatabase.Execute(stmt);
            continue;
        }

        if (achievement->Flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
            m_allCompletedAchievements[achievementId] = true;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu completed achievements in %u ms", static_cast<uint64>(m_allCompletedAchievements.size()), GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadRewards()
{
    uint32 oldMSTime = getMSTime();

    m_achievementRewards.clear();                           // need for reload case
    m_achievementRewardVector.assign(MAX_ACHIEVEMENT, nullptr);

    //                                                 0      1         2         3            4           5       6      7       8       9        10
    QueryResult result = WorldDatabase.Query("SELECT entry, title_A, title_H, genderTitle, learnSpell, castSpell, item, sender, subject, text, ScriptName FROM achievement_reward");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement rewards. DB table `achievement_reward` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        const AchievementEntry* pAchievement = sAchievementStore.LookupEntry(entry);
        if (!pAchievement)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` has wrong achievement (Entry: %u), ignored.", entry);
            continue;
        }

        AchievementReward reward;
        reward.titleId[0] = fields[1].GetUInt32();
        reward.titleId[1] = fields[2].GetUInt32();
        reward.genderTitle = fields[3].GetUInt8();
        reward.learnSpell = fields[4].GetUInt32();
        reward.castSpell = fields[5].GetUInt32();
        reward.itemId = fields[6].GetUInt32();
        reward.sender = fields[7].GetUInt32();
        reward.subject = fields[8].GetString();
        reward.text = fields[9].GetString();

        const char* scriptName = fields[10].GetCString();
        uint32 scriptId = 0;

        if (strcmp(scriptName, "") != 0) // not empty
            scriptId = sObjectMgr->GetScriptId(scriptName);

        reward.ScriptId = scriptId;

        // must be title or mail at least
        if (!reward.titleId[0] && !reward.titleId[1] && !reward.sender && !reward.learnSpell && !reward.castSpell && !reward.ScriptId)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have title or item reward data, ignored.", entry);
            continue;
        }

        if (pAchievement->Faction == ACHIEVEMENT_FACTION_ANY && ((reward.titleId[0] == 0) != (reward.titleId[1] == 0)))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has title (A: %u H: %u) for only one team.", entry, reward.titleId[0], reward.titleId[1]);

        if (reward.titleId[0])
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(reward.titleId[0]);
            if (!titleEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid title id (%u) in `title_A`, set to 0", entry, reward.titleId[0]);
                reward.titleId[0] = 0;
            }
        }

        if (reward.titleId[1])
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(reward.titleId[1]);
            if (!titleEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid title id (%u) in `title_H`, set to 0", entry, reward.titleId[1]);
                reward.titleId[1] = 0;
            }
        }

        //check mail data before item for report including wrong item case
        if (reward.sender)
        {
            if (!sObjectMgr->GetCreatureTemplate(reward.sender))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid creature entry %u as sender, mail reward skipped.", entry, reward.sender);
                reward.sender = 0;
            }
        }
        else if (reward.learnSpell)
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(reward.learnSpell);
            if (!spellEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) have not existent learn spell %i.", entry, reward.learnSpell);
                continue;
            }
        }
        else if (reward.castSpell)
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(reward.castSpell);
            if (!spellEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) have not existent cast spell %i.", entry, reward.castSpell);
                continue;
            }
        }
        else
        {
            if (reward.itemId)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have sender data but has item reward, item will not be rewarded.", entry);

            if (!reward.subject.empty())
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have sender data but has mail subject.", entry);

            if (!reward.text.empty())
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) does not have sender data but has mail text.", entry);
        }

        if (reward.itemId)
        {
            if (!sObjectMgr->GetItemTemplate(reward.itemId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `achievement_reward` (Entry: %u) has invalid item id %u, reward mail will not contain item.", entry, reward.itemId);
                reward.itemId = 0;
            }
        }

        m_achievementRewards[entry] = reward;
        m_achievementRewardVector[entry] = &m_achievementRewards[entry];
        ++count;

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u achievement rewards in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AchievementGlobalMgr::LoadRewardLocales()
{
    uint32 oldMSTime = getMSTime();

    m_achievementRewardLocales.clear();                       // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT entry, subject_loc1, text_loc1, subject_loc2, text_loc2, subject_loc3, text_loc3, subject_loc4, text_loc4, "
        "subject_loc5, text_loc5, subject_loc6, text_loc6, subject_loc7, text_loc7, subject_loc8, text_loc8, subject_loc9, text_loc9,"
        "subject_loc10, text_loc10 FROM locales_achievement_reward");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 achievement reward locale strings.  DB table `locales_achievement_reward` is empty");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (m_achievementRewardVector[entry] == nullptr)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `locales_achievement_reward` (Entry: %u) has locale strings for non-existing achievement reward.", entry);
            continue;
        }

        AchievementRewardLocale& data = m_achievementRewardLocales[entry];

        for (int i = 1; i < TOTAL_LOCALES; ++i)
        {
            auto locale = static_cast<LocaleConstant>(i);
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1)].GetString(), locale, data.subject);
            ObjectMgr::AddLocaleString(fields[1 + 2 * (i - 1) + 1].GetString(), locale, data.text);
        }
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %lu achievement reward locale strings in %u ms", static_cast<uint64>(m_achievementRewardLocales.size()), GetMSTimeDiffToNow(oldMSTime));
}

uint32 AchievementGlobalMgr::GetParantTreeId(uint32 parent)
{
    if (CriteriaTreeEntry const* pTree = sCriteriaTreeStore.LookupEntry(parent))
    {
        if (pTree->Parent == 0)
            return pTree->ID;

        return GetParantTreeId(pTree->Parent);
    }
    return parent;
}

CriteriaTree const* AchievementGlobalMgr::GetCriteriaTree(uint32 criteriaTreeId) const
{
    return _criteriaTrees[criteriaTreeId];
}

Criteria const* AchievementGlobalMgr::GetCriteria(uint32 criteriaId) const
{
    return _criteria[criteriaId];
}

ModifierTreeNode const* AchievementGlobalMgr::GetModifierTree(uint32 modifierTreeId) const
{
    return _criteriaModifiers[modifierTreeId];
}
