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

#include "CharacterPackets.h"
#include "ConditionMgr.h"
#include "GameEventMgr.h"
#include "Garrison.h"
#include "InstanceScript.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "ScenarioMgr.h"
#include "Group.h"
#include "DatabaseEnv.h"
#include "AreaTriggerData.h"
#include "QuestData.h"
#include "GlobalFunctional.h"
#include "GossipData.h"

ConditionSourceInfo::ConditionSourceInfo(WorldObject* target0, WorldObject* target1, WorldObject* target2)
{
    mConditionTargets[0] = target0;
    mConditionTargets[1] = target1;
    mConditionTargets[2] = target2;
    mLastFailedCondition = nullptr;
}

Condition::Condition()
{
    SourceType = CONDITION_SOURCE_TYPE_NONE;
    SourceGroup = 0;
    SourceEntry = 0;
    ElseGroup = 0;
    ConditionType = CONDITION_NONE;
    ConditionTarget = 0;
    ConditionValue1 = 0;
    ConditionValue2 = 0;
    ConditionValue3 = 0;
    ReferenceId = 0;
    ErrorTextId = 0;
    ScriptId = 0;
    SourceId = 0;
    NegativeCondition = false;
}

// Checks if object meets the condition
// Can have CONDITION_SOURCE_TYPE_NONE && !mReferenceId if called from a special event (ie: SmartAI)
bool Condition::Meets(ConditionSourceInfo& sourceInfo)
{
    // ASSERT(ConditionTarget < MAX_CONDITION_TARGETS);
    if(ConditionTarget >= MAX_CONDITION_TARGETS)
        return false;

    WorldObject* object = sourceInfo.mConditionTargets[ConditionTarget];
    // object not present, return false
    if (!object)
    {
        TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "Condition object not found for condition (Entry: %u Type: %u Group: %u)", SourceEntry, SourceType, SourceGroup);
        return false;
    }
    bool condMeets = false;
    switch (ConditionType)
    {
        case CONDITION_NONE:
            condMeets = true;                                    // empty condition, always met
            break;
        case CONDITION_AURA:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->HasAuraEffect(ConditionValue1, ConditionValue2);
            break;
        }
        case CONDITION_ITEM:
        {
            if (Player* player = object->ToPlayer())
            {
                // don't allow 0 items (it's checked during table load)
                ASSERT(ConditionValue2);
                bool checkBank = ConditionValue3 ? true : false;
                condMeets = player->HasItemCount(ConditionValue1, ConditionValue2, checkBank);
            }
            break;
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasItemOrGemWithIdEquipped(ConditionValue1, 1);
            break;
        }
        case CONDITION_ZONEID:
            condMeets = object->GetCurrentZoneID() == ConditionValue1;
            break;
        case CONDITION_REPUTATION_RANK:
        {
            if (Player* player = object->ToPlayer())
            {
                if (FactionEntry const* faction = sFactionStore.LookupEntry(ConditionValue1))
                    condMeets = (ConditionValue2 & (1 << player->GetReputationMgr().GetRank(faction))) != 0;
            }
            break;
        }
        case CONDITION_ACHIEVEMENT:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasAchieved(ConditionValue1);
            break;
        }
        case CONDITION_TEAM:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->GetTeam() == ConditionValue1;
            break;
        }
        case CONDITION_CLASS:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = (unit->getClassMask() & ConditionValue1) != 0;
            break;
        }
        case CONDITION_RACE:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = (unit->getRaceMask() & ConditionValue1) != 0;
            break;
        }
        case CONDITION_GENDER:
            {
                if (Player* player = object->ToPlayer())
                    condMeets = player->getGender() == ConditionValue1;
                break;
            }
        case CONDITION_SKILL:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasSkill(ConditionValue1) && player->GetBaseSkillValue(ConditionValue1) >= ConditionValue2;
            break;
        }
        case CONDITION_QUESTREWARDED:
        {
            if (Player* player = object->ToPlayer())
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_REWARDED);
            }
            break;
        }
        case CONDITION_QUESTTAKEN:
        {
            Player* player = nullptr;
            // if its creature than try to find targeted player
            if (object->IsCreature())
            {
                if (Unit* target = object->ToUnit()->getThreatManager().getHostilTarget())
                {
                    if (target->ToPlayer())
                        player = target->ToPlayer();
                    else if (target->isPet() && target->GetOwner() && target->GetOwner()->ToPlayer())
                        player = target->GetOwner()->ToPlayer();
                }
            }
            else
                player = object->ToPlayer();

            if (player)
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_INCOMPLETE);
            }
            break;
        }
        case CONDITION_QUEST_COMPLETE:
        {
            if (Player* player = object->ToPlayer())
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(ConditionValue1));
            }
            break;
        }
        case CONDITION_QUEST_NONE:
        {
            if (Player* player = object->ToPlayer())
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_NONE);
            }
            break;
        }
        case CONDITION_AREA_EXPLORED:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasAreaExplored(ConditionValue1);
            break;
        }
        case CONDITION_SCENE_SEEN:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasSceneStatus(ConditionValue1, SCENE_COMPLETE);
            break;
        }
        case CONDITION_SCENE_TRIGER_EVENT:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasSceneStatus(ConditionValue1, SCENE_TRIGER);
            break;
        }
        case CONDITION_GARRRISON_BUILDING:
        {
            condMeets = false;
            if (Player* player = object->ToPlayer())
                if (Garrison* g = player->GetGarrisonPtr())
                    if (auto const& plot = g->GetPlotWithBuildingType(ConditionValue1))
                        if (plot->BuildingInfo.PacketInfo->Active)
                            condMeets = sGarrBuildingStore.AssertEntry(plot->BuildingInfo.PacketInfo->GarrBuildingID)->UpgradeLevel >= ConditionValue2;
            break;
        }
        case CONDITION_ACTIVE_EVENT:
            condMeets = sGameEventMgr->IsActiveEvent(ConditionValue1);
            break;
        case CONDITION_QUEST_OBJECTIVE_DONE:
        {
            if (Player* player = object->ToPlayer())
            {
                uint32 count = player->GetQuestObjectiveData(ConditionValue1, ConditionValue2);
                condMeets = ConditionValue3 ? count >= ConditionValue3 : count;
            }
            break;
        }
        case CONDITION_INSTANCE_INFO:
        {
            Map* map = object->GetMap();
            if (map && map->IsDungeon())
            {
                if (InstanceScript const* instance = static_cast<InstanceMap*>(map)->GetInstanceScript())
                {
                    switch (ConditionValue3)
                    {
                        case INSTANCE_INFO_DATA:
                            condMeets = static_cast<InstanceMap*>(map)->GetInstanceScript()->GetData(ConditionValue1) == ConditionValue2;
                            break;
                            //128bit.
                            //case INSTANCE_INFO_GUID_DATA:
                            //    condMeets = instance->GetGuidData(ConditionValue1) == ObjectGuid(uint64(ConditionValue2));
                            //    break;
                        case INSTANCE_INFO_BOSS_STATE:
                            condMeets = instance->GetBossState(ConditionValue1) == EncounterState(ConditionValue2);
                            break;
                        case INSTANCE_INFO_SCENARION_STEP:
                            condMeets = instance->getScenarionStep() == ConditionValue1;
                            break;
                        default:
                            break;
                    }
                }
            }
            break;
        }
        case CONDITION_MAPID:
            condMeets = object->GetMapId() == ConditionValue1;
            break;
        case CONDITION_AREAID:
            condMeets = object->GetAreaId() == ConditionValue1;
            break;
        case CONDITION_SPELL:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasSpell(ConditionValue1);
            break;
        }
        case CONDITION_LEVEL:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue2), static_cast<uint32>(unit->getLevel()), static_cast<uint32>(ConditionValue1));
            break;
        }
        case CONDITION_DRUNKENSTATE:
        {
            if (Player* player = object->ToPlayer())
                condMeets = static_cast<uint32>(Player::GetDrunkenstateByValue(player->GetDrunkValue())) >= ConditionValue1;
            break;
        }
        case CONDITION_NEAR_CREATURE:
        {
            condMeets = GetClosestCreatureWithEntry(object, ConditionValue1, static_cast<float>(ConditionValue2)) ? true : false;
            break;
        }
        case CONDITION_NEAR_GAMEOBJECT:
        {
            condMeets = GetClosestGameObjectWithEntry(object, ConditionValue1, static_cast<float>(ConditionValue2)) ? true : false;
            break;
        }
        case CONDITION_OBJECT_ENTRY:
        {
            if (uint32(object->GetTypeId()) == ConditionValue1)
                condMeets = !ConditionValue2 || (object->GetEntry() == ConditionValue2);
            break;
        }
        case CONDITION_TYPE_MASK:
        {
            condMeets = object->isType(ConditionValue1);
            break;
        }
        case CONDITION_RELATION_TO:
        {
            if (WorldObject* toObject = sourceInfo.mConditionTargets[ConditionValue1])
            {
                Unit* toUnit = toObject->ToUnit();
                Unit* unit = object->ToUnit();
                if (toUnit && unit)
                {
                    switch (ConditionValue2)
                    {
                        case RELATION_SELF:
                            condMeets = unit == toUnit;
                            break;
                        case RELATION_IN_PARTY:
                            condMeets = unit->IsInPartyWith(toUnit);
                            break;
                        case RELATION_IN_RAID_OR_PARTY:
                            condMeets = unit->IsInRaidWith(toUnit);
                            break;
                        case RELATION_OWNED_BY:
                            condMeets = unit->GetSummonedByGUID() == toUnit->GetGUID();
                            break;
                        case RELATION_PASSENGER_OF:
                            condMeets = unit->IsOnVehicle(toUnit);
                            break;
                        case RELATION_CREATED_BY:
                            condMeets = unit->GetOwnerGUID() == toUnit->GetGUID();
                            break;
                        default:
                            break;
                    }
                }
            }
            break;
        }
        case CONDITION_REACTION_TO:
        {
            if (WorldObject* toObject = sourceInfo.mConditionTargets[ConditionValue1])
            {
                Unit* toUnit = toObject->ToUnit();
                Unit* unit = object->ToUnit();
                if (toUnit && unit)
                    condMeets = ((1 << unit->GetReactionTo(toUnit)) & ConditionValue2) != 0;
            }
            break;
        }
        case CONDITION_DISTANCE_TO:
        {
            if (WorldObject* toObject = sourceInfo.mConditionTargets[ConditionValue1])
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue3), object->GetDistance(toObject), static_cast<float>(ConditionValue2));
            break;
        }
        case CONDITION_ALIVE:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->isAlive();
            break;
        }
        case CONDITION_HP_VAL:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue2), unit->GetHealth(), static_cast<uint64>(ConditionValue1));
            break;
        }
        case CONDITION_HP_PCT:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue2), unit->GetHealthPct(), static_cast<float>(ConditionValue1));
            break;
        }
        case CONDITION_WORLD_STATE:
        {
            condMeets = ConditionValue2 == sWorld->getWorldState(ConditionValue1);
            break;
        }
        case CONDITION_PHASEMASK:
        {
            condMeets = (object->GetPhaseMask() & ConditionValue1) != 0;
            break;
        }
        case CONDITION_TITLE:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasTitle(ConditionValue1);
            break;
        }
        case CONDITION_SPAWNMASK:
        {
            condMeets = ((UI64LIT(1) << object->GetMap()->GetSpawnMode()) & ConditionValue1) != 0;
            break;
        }
        case CONDITION_SCENARION_STEP:
        {
            if (!object->InInstance())
                break;

            InstanceMap* inst = object->GetMap()->ToInstanceMap();
            if (uint32 instanceId = inst ? inst->GetInstanceId() : 0)
                if (Scenario* progress = sScenarioMgr->GetScenario(instanceId))
                    condMeets = ConditionValue1 == progress->GetScenarioId() && ConditionValue2 == progress->GetCurrentStep();
            break;
        }
        case CONDITION_REALM_ACHIEVEMENT:
        {
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(ConditionValue1);
            if (achievement && sAchievementMgr->IsRealmCompleted(achievement))
                condMeets = true;
            break;
        }
        case CONDITION_IN_WATER:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->IsInWater();
            break;
        }
        case CONDITION_TERRAIN_SWAP:
        {
            condMeets = object->IsInTerrainSwap(ConditionValue1);
            break;
        }
        case CONDITION_STAND_STATE:
        {
            if (Unit* unit = object->ToUnit())
            {
                if (ConditionValue1 == 0)
                    condMeets = (unit->getStandState() == UnitStandStateType(ConditionValue2));
                else if (ConditionValue2 == 0)
                    condMeets = unit->IsStandState();
                else if (ConditionValue2 == 1)
                    condMeets = unit->IsSitState();
            }
            break;
        }
        case CONDITION_CLASS_HALL_ADVANCEMENT:
        {
            if (Player* player = object->ToPlayer())
                if (Garrison* garr = player->GetGarrisonPtr())
                    condMeets = garr->hasTallent(ConditionValue1);
            break;
        }
        case CONDITION_CURRENCY:
        {
            if (Player* player = object->ToPlayer())
            {
                uint32 count = player->GetCurrency(ConditionValue1);
                condMeets = count >= ConditionValue2 && (!ConditionValue3 || count < ConditionValue3);
            }
            break;
        }
        case CONDITION_CURRENCY_ON_WEEK:
        {
            if (Player* player = object->ToPlayer())
            {
                uint32 count = player->GetCurrencyOnWeek(ConditionValue1);
                condMeets = count >= ConditionValue2 && (!ConditionValue3 || count < ConditionValue3);
            }
            break;
        }
        case CONDITION_CRITERIA:
        {
            if (Player* player = object->ToPlayer())
            {
                if (CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(ConditionValue1))
                    condMeets = tree->Criteria && tree->Entry && player->GetAchievementMgr()->IsCompletedCriteria(tree, tree->Entry->Amount);
            }
            break;
        }
        case CONDITION_CRITERIA_TREE:
        {
            if (!object->InInstance())
                break;

            if (Player* player = object->ToPlayer())
            {
                if (CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(ConditionValue1))
                {
                    condMeets = player->GetAchievementMgr()->IsCompletedCriteriaTree(tree);
                    if (!condMeets)
                    {
                        if (Scenario* progress = sScenarioMgr->GetScenario(player->GetInstanceId()))
                            condMeets = progress->GetAchievementMgr().IsCompletedCriteriaTree(tree);
                    }
                }
            }
            else
                if (CriteriaTree const* tree = sAchievementMgr->GetCriteriaTree(ConditionValue1))
                {
                    if (Scenario* progress = sScenarioMgr->GetScenario(object->GetInstanceId()))
                        condMeets = progress->GetAchievementMgr().IsCompletedCriteriaTree(tree);
                }
            break;
        }
        case CONDITION_MODIFIER_TREE:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->GetAchievementMgr()->CheckModifierTree(ConditionValue1, player);
            break;
        }
        case CONDITION_ARTIFACT_LEVEL:
        {
            if (Player* player = object->ToPlayer())
            {
                if (Item* item = player->GetItemByEntry(ConditionValue1))
                {
                    uint32 count = 0;
                    if (item->IsEquipped())
                        count = item->GetTotalPurchasedArtifactPowers();

                    condMeets = count >= ConditionValue2 && (!ConditionValue3 || count < ConditionValue3);
                }
            }
            break;
        }
        case CONDITION_ARTIFACT_POWER:
        {
            if (Player* player = object->ToPlayer())
            {
                if (ConditionValue1)
                {
                    if (Item* artifact = player->GetItemByEntry(ConditionValue1))
                    {
                        uint32 count = 0;
                        if (artifact->IsEquipped())
                            count = artifact->GetUInt64Value(ITEM_FIELD_ARTIFACT_XP);

                        condMeets = count >= ConditionValue2 && (!ConditionValue3 || count < ConditionValue3);
                    }
                }
                else
                {
                    if (Item* artifact = player->GetArtifactWeapon())
                    {
                        uint32 count = artifact->GetUInt64Value(ITEM_FIELD_ARTIFACT_XP);
                        condMeets = count >= ConditionValue2 && (!ConditionValue3 || count < ConditionValue3);
                    }
                }
            }
            break;
        }
        case CONDITION_SPEC_ID:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == ConditionValue1;
            break;
        }
        case CONDITION_ON_TRANSPORT:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->IsOnVehicle();
            break;
        }
        case CONDITION_IN_RAID_OR_GROUP:
        {
            if (Player* player = object->ToPlayer())
            {
                if (ConditionValue1)
                {
                    if (ConditionValue2)
                        condMeets = player->GetGroup() && player->GetGroup()->isRaidGroup();
                    if (ConditionValue3)
                        condMeets = player->GetGroup();
                }
                else
                {
                    if (ConditionValue2)
                        condMeets = !player->GetGroup() || !player->GetGroup()->isRaidGroup();
                    if (ConditionValue3)
                        condMeets = !player->GetGroup();
                }
            }
            break;
        }
        case CONDITION_WORLD_QUEST:
        {
            if (Quest const* quest = sQuestDataStore->GetQuestTemplate(ConditionValue1))
                condMeets = sQuestDataStore->GetWorldQuest(quest);
            break;
        }
        case CONDITION_GAMEMASTER:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->isGameMaster();
            break;
        }
        case CONDITION_HAS_EMOTE_STATE:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->GetUInt32Value(UNIT_FIELD_EMOTE_STATE);
            break;
        }
        case CONDITION_HAS_POWER:
        {
            if (Unit* unit = object->ToUnit())
            {
                condMeets = unit->GetPower(Powers(ConditionValue1)) >= ConditionValue2;
                if (ConditionValue3)
                    condMeets = unit->GetPower(Powers(ConditionValue1)) <= ConditionValue3;
            }
            break;
        }
        case CONDITION_IN_COMBAT:
        {
            if (auto unit = object->ToUnit())
                condMeets = unit->isInCombat();
            break;
        }
        case CONDITION_GET_AMOUNT_STACK_AURA:
        {
            if (Unit* unit = object->ToUnit())
                if (Aura* aura = unit->GetAura(ConditionValue1))
                    condMeets = aura->GetStackAmount() == ConditionValue2;
            break;
        }
        case CONDITION_TIMEWALKING:
        {
            if (auto player = object->ToPlayer())
                condMeets = player->GetMap()->GetLootDifficulty() == DIFFICULTY_TIMEWALKING || player->GetMap()->GetLootDifficulty() == DIFFICULTY_TIMEWALKING_RAID;
            break;
        }
        case CONDITION_ACOUNT_QUEST:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasAccountQuest(ConditionValue1);
            break;
        }
        default:
            condMeets = false;
            break;
    }

    if (NegativeCondition)
        condMeets = !condMeets;

    if (!condMeets)
        sourceInfo.mLastFailedCondition = this;

    bool script = sScriptMgr->OnConditionCheck(this, sourceInfo); // Returns true by default.
    return condMeets && script;
}

uint32 Condition::GetSearcherTypeMaskForCondition()
{
    // build mask of types for which condition can return true
    // this is used for speeding up gridsearches
    if (NegativeCondition)
        return (GRID_MAP_TYPE_MASK_ALL);
    uint32 mask = 0;
    switch (ConditionType)
    {
        case CONDITION_NONE:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_AURA:
        case CONDITION_GET_AMOUNT_STACK_AURA:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_ITEM:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_ITEM_EQUIPPED:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_ZONEID:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_REPUTATION_RANK:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_ACHIEVEMENT:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_TEAM:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_CLASS:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_RACE:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_GENDER:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_SKILL:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_QUESTREWARDED:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_QUESTTAKEN:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_QUEST_COMPLETE:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_QUEST_NONE:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_ACTIVE_EVENT:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_INSTANCE_INFO:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_MAPID:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_AREAID:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_SPELL:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_LEVEL:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_DRUNKENSTATE:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_NEAR_CREATURE:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_NEAR_GAMEOBJECT:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_OBJECT_ENTRY:
            switch (ConditionValue1)
            {
                case TYPEID_UNIT:
                    mask |= GRID_MAP_TYPE_MASK_CREATURE;
                    break;
                case TYPEID_PLAYER:
                    mask |= GRID_MAP_TYPE_MASK_PLAYER;
                    break;
                case TYPEID_GAMEOBJECT:
                    mask |= GRID_MAP_TYPE_MASK_GAMEOBJECT;
                    break;
                case TYPEID_CORPSE:
                    mask |= GRID_MAP_TYPE_MASK_CORPSE;
                    break;
                case TYPEID_AREATRIGGER:
                    mask |= GRID_MAP_TYPE_MASK_AREATRIGGER;
                    break;
                case TYPEID_CONVERSATION:
                    mask |= GRID_MAP_TYPE_MASK_CONVERSATION;
                    break;
                case TYPEID_EVENTOBJECT:
                    mask |= GRID_MAP_TYPE_MASK_EVENTOBJECT;
                    break;
                default:
                    break;
            }
        case CONDITION_TYPE_MASK:
            if (ConditionValue1 & TYPEMASK_UNIT)
                mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            if (ConditionValue1 & TYPEMASK_PLAYER)
                mask |= GRID_MAP_TYPE_MASK_PLAYER;
            if (ConditionValue1 & TYPEMASK_GAMEOBJECT)
                mask |= GRID_MAP_TYPE_MASK_GAMEOBJECT;
            if (ConditionValue1 & TYPEMASK_CORPSE)
                mask |= GRID_MAP_TYPE_MASK_CORPSE;
            if (ConditionValue1 & TYPEMASK_AREATRIGGER)
                mask |= GRID_MAP_TYPE_MASK_AREATRIGGER;
            if (ConditionValue1 & TYPEMASK_CONVERSATION)
                mask |= GRID_MAP_TYPE_MASK_CONVERSATION;
            if (ConditionValue1 & TYPEMASK_EVENTOBJECT)
                mask |= GRID_MAP_TYPE_MASK_EVENTOBJECT;
            break;
        case CONDITION_RELATION_TO:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_REACTION_TO:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_DISTANCE_TO:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_ALIVE:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_HP_VAL:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_HP_PCT:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_WORLD_STATE:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_PHASEMASK:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_ON_TRANSPORT:
        case CONDITION_TITLE:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_SPAWNMASK:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_AREA_EXPLORED:
        case CONDITION_SCENE_SEEN:
        case CONDITION_SCENE_TRIGER_EVENT:
        case CONDITION_QUEST_OBJECTIVE_DONE:
        case CONDITION_CLASS_HALL_ADVANCEMENT:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_REALM_ACHIEVEMENT:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_IN_WATER:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_TERRAIN_SWAP:
            mask |= GRID_MAP_TYPE_MASK_ALL;
            break;
        case CONDITION_STAND_STATE:
        case CONDITION_HAS_POWER:
            mask |= GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER;
            break;
        case CONDITION_CURRENCY:
        case CONDITION_CRITERIA:
        case CONDITION_CRITERIA_TREE:
        case CONDITION_MODIFIER_TREE:
        case CONDITION_ARTIFACT_LEVEL:
        case CONDITION_SPEC_ID:
        case CONDITION_CURRENCY_ON_WEEK:
        case CONDITION_WORLD_QUEST:
        case CONDITION_ACOUNT_QUEST:
            mask |= GRID_MAP_TYPE_MASK_PLAYER;
            break;
        default:
            ASSERT(false && "Condition::GetSearcherTypeMaskForCondition - missing condition handling!");
            break;
    }
    return mask;
}

bool ConditionMgr::IsObjectMeetingSmartEventConditions(int64 entryOrGuid, uint32 eventId, uint32 sourceType, Unit* unit, WorldObject* baseObject) const
{
	SmartEventConditionContainer::const_iterator itr = SmartEventConditionStore.find(std::make_pair(entryOrGuid, sourceType));
	if (itr != SmartEventConditionStore.end())
	{
		ConditionTypeContainer::const_iterator i = (*itr).second.find(eventId);
		if (i != itr->second.end())
		{
			TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "condition", "GetConditionsForSmartEvent: found conditions for Smart Event entry or guid " SI64FMTD " eventId %u", entryOrGuid, eventId);
			ConditionSourceInfo sourceInfo(unit, baseObject);
			//SPP NEED FIX
			//return IsObjectMeetToConditions(sourceInfo, i->second.end);
			return true;
		}
	}
	return true;
}

bool Condition::isLoaded() const
{
    return ConditionType > CONDITION_NONE || ReferenceId;
}

uint32 Condition::GetMaxAvailableConditionTargets()
{
    // returns number of targets which are available for given source type
    switch (SourceType)
    {
        case CONDITION_SOURCE_TYPE_SPELL:
        case CONDITION_SOURCE_TYPE_SPELL_IMPLICIT_TARGET:
        case CONDITION_SOURCE_TYPE_CREATURE_TEMPLATE_VEHICLE:
        case CONDITION_SOURCE_TYPE_VEHICLE_SPELL:
        case CONDITION_SOURCE_TYPE_SPELL_CLICK_EVENT:
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU:
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION:
        case CONDITION_SOURCE_TYPE_SMART_EVENT:
        case CONDITION_SOURCE_TYPE_NPC_VENDOR:
        case CONDITION_SOURCE_TYPE_SPELL_PROC:
        case CONDITION_SOURCE_TYPE_AREATRIGGER_ACTION:
            return 2;
        default:
            return 1;
    }
}

ConditionMgr::ConditionMgr()
{
}

ConditionMgr::~ConditionMgr()
{
    Clean();
}

ConditionMgr* ConditionMgr::instance()
{
    static ConditionMgr instance;
    return &instance;
}

ConditionList ConditionMgr::GetConditionReferences(uint32 refId)
{
    ConditionList conditions;
    ConditionReferenceContainer::const_iterator ref = ConditionReferenceStore.find(refId);
    if (ref != ConditionReferenceStore.end())
        conditions = (*ref).second;
    return conditions;
}

uint32 ConditionMgr::GetSearcherTypeMaskForConditionList(ConditionList const& conditions)
{
    if (conditions.empty())
        return GRID_MAP_TYPE_MASK_ALL;
    //     groupId, typeMask
    std::map<uint32, uint32> ElseGroupStore;
    for (ConditionList::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
    {
        // no point of having not loaded conditions in list
        ASSERT((*i)->isLoaded() && "ConditionMgr::GetSearcherTypeMaskForConditionList - not yet loaded condition found in list");
        std::map<uint32, uint32>::const_iterator itr = ElseGroupStore.find((*i)->ElseGroup);
        // group not filled yet, fill with widest mask possible
        if (itr == ElseGroupStore.end())
            ElseGroupStore[(*i)->ElseGroup] = GRID_MAP_TYPE_MASK_ALL;
        // no point of checking anymore, empty mask
        else if (!(*itr).second)
            continue;

        if ((*i)->ReferenceId) // handle reference
        {
            ConditionReferenceContainer::const_iterator ref = ConditionReferenceStore.find((*i)->ReferenceId);
            ASSERT(ref != ConditionReferenceStore.end() && "ConditionMgr::GetSearcherTypeMaskForConditionList - incorrect reference");
            ElseGroupStore[(*i)->ElseGroup] &= GetSearcherTypeMaskForConditionList((*ref).second);
        }
        else // handle normal condition
        {
            // object will match conditions in one ElseGroupStore only when it matches all of them
            // so, let's find a smallest possible mask which satisfies all conditions
            ElseGroupStore[(*i)->ElseGroup] &= (*i)->GetSearcherTypeMaskForCondition();
        }
    }
    // object will match condition when one of the checks in ElseGroupStore is matching
    // so, let's include all possible masks
    uint32 mask = 0;
    for (std::map<uint32, uint32>::const_iterator i = ElseGroupStore.begin(); i != ElseGroupStore.end(); ++i)
        mask |= i->second;

    return mask;
}

bool ConditionMgr::IsObjectMeetToConditionList(ConditionSourceInfo& sourceInfo, ConditionList const& conditions)
{
    //     groupId, groupCheckPassed
    std::map<uint32, bool> ElseGroupStore;
    for (ConditionList::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
    {
        TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "ConditionMgr::IsPlayerMeetToConditionList condType: %u val1: %u", (*i)->ConditionType, (*i)->ConditionValue1);
        if ((*i)->isLoaded())
        {
            //! Find ElseGroup in ElseGroupStore
            std::map<uint32, bool>::const_iterator itr = ElseGroupStore.find((*i)->ElseGroup);
            //! If not found, add an entry in the store and set to true (placeholder)
            if (itr == ElseGroupStore.end())
                ElseGroupStore[(*i)->ElseGroup] = true;
            else if (!(*itr).second)
                continue;

            if ((*i)->ReferenceId)//handle reference
            {
                ConditionReferenceContainer::const_iterator ref = ConditionReferenceStore.find((*i)->ReferenceId);
                if (ref != ConditionReferenceStore.end())
                {
                    if (!IsObjectMeetToConditionList(sourceInfo, (*ref).second))
                        ElseGroupStore[(*i)->ElseGroup] = false;
                }
                else
                {
                    TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "IsPlayerMeetToConditionList: Reference template -%u not found",
                        (*i)->ReferenceId);//checked at loading, should never happen
                }

            }
            else //handle normal condition
            {
                if (!(*i)->Meets(sourceInfo))
                    ElseGroupStore[(*i)->ElseGroup] = false;
            }
        }
    }
    for (std::map<uint32, bool>::const_iterator i = ElseGroupStore.begin(); i != ElseGroupStore.end(); ++i)
        if (i->second)
            return true;

    return false;
}

bool ConditionMgr::IsObjectMeetToConditions(WorldObject* object, ConditionList const& conditions)
{
    ConditionSourceInfo srcInfo = ConditionSourceInfo(object);
    return IsObjectMeetToConditions(srcInfo, conditions);
}

bool ConditionMgr::IsObjectMeetToConditions(WorldObject* object1, WorldObject* object2, ConditionList const& conditions)
{
    ConditionSourceInfo srcInfo = ConditionSourceInfo(object1, object2);
    return IsObjectMeetToConditions(srcInfo, conditions);
}

bool ConditionMgr::IsObjectMeetToConditions(ConditionSourceInfo& sourceInfo, ConditionList const& conditions)
{
    if (conditions.empty())
        return true;

    TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "ConditionMgr::IsObjectMeetToConditions");
    return IsObjectMeetToConditionList(sourceInfo, conditions);
}

bool ConditionMgr::CanHaveSourceGroupSet(ConditionSourceType sourceType) const
{
    return (sourceType == CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_GOSSIP_MENU ||
            sourceType == CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION ||
            sourceType == CONDITION_SOURCE_TYPE_VEHICLE_SPELL ||
            sourceType == CONDITION_SOURCE_TYPE_SPELL_IMPLICIT_TARGET ||
            sourceType == CONDITION_SOURCE_TYPE_SPELL_CLICK_EVENT ||
            sourceType == CONDITION_SOURCE_TYPE_SMART_EVENT ||
            sourceType == CONDITION_SOURCE_TYPE_NPC_VENDOR ||
            sourceType == CONDITION_SOURCE_TYPE_PHASE_DEFINITION ||
            sourceType == CONDITION_SOURCE_TYPE_AREATRIGGER_ACTION ||
            sourceType == CONDITION_SOURCE_TYPE_WORLD_LOOT_TEMPLATE ||
            sourceType == CONDITION_SOURCE_TYPE_PLAYER_CHOICE ||
            sourceType == CONDITION_SOURCE_TYPE_PLAYER_CHOICE_RESPONS ||
            sourceType == CONDITION_SOURCE_TYPE_WORLD_STATE ||
            sourceType == CONDITION_SOURCE_TYPE_LOOT_ITEM);
}

bool ConditionMgr::CanHaveSourceIdSet(ConditionSourceType sourceType) const
{
    return (sourceType == CONDITION_SOURCE_TYPE_SMART_EVENT);
}

ConditionList ConditionMgr::GetConditionsForNotGroupedEntry(ConditionSourceType sourceType, uint32 entry)
{
    ConditionList spellCond;
    if (sourceType > CONDITION_SOURCE_TYPE_NONE && sourceType < CONDITION_SOURCE_TYPE_MAX)
    {
        ConditionContainer::const_iterator itr = ConditionStore.find(sourceType);
        if (itr != ConditionStore.end())
        {
            ConditionTypeContainer::const_iterator i = (*itr).second.find(entry);
            if (i != (*itr).second.end())
            {
                spellCond = (*i).second;
                TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForNotGroupedEntry: found conditions for type %u and entry %u", uint32(sourceType), entry);
            }
        }
    }
    return spellCond;
}

ConditionList ConditionMgr::GetConditionsForSpellClickEvent(uint32 creatureId, uint32 spellId)
{
    ConditionList cond;
    CreatureSpellConditionContainer::const_iterator itr = SpellClickEventConditionStore.find(creatureId);
    if (itr != SpellClickEventConditionStore.end())
    {
        ConditionTypeContainer::const_iterator i = (*itr).second.find(spellId);
        if (i != (*itr).second.end())
        {
            cond = (*i).second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForSpellClickEvent: found conditions for Vehicle entry %u spell %u", creatureId, spellId);
        }
    }
    return cond;
}

ConditionList ConditionMgr::GetConditionsForVehicleSpell(uint32 creatureId, uint32 spellId)
{
    ConditionList cond;
    CreatureSpellConditionContainer::const_iterator itr = VehicleSpellConditionStore.find(creatureId);
    if (itr != VehicleSpellConditionStore.end())
    {
            ConditionTypeContainer::const_iterator i = (*itr).second.find(spellId);
        if (i != (*itr).second.end())
        {
            cond = (*i).second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForVehicleSpell: found conditions for Vehicle entry %u spell %u", creatureId, spellId);
        }
    }
    return cond;
}

ConditionList ConditionMgr::GetConditionsForSmartEvent(int64 entryOrGuid, uint32 eventId, uint32 sourceType)
{
    ConditionList cond;
    SmartEventConditionContainer::const_iterator itr = SmartEventConditionStore.find(std::make_pair(entryOrGuid, sourceType));
    if (itr != SmartEventConditionStore.end())
    {
        ConditionTypeContainer::const_iterator i = (*itr).second.find(eventId + 1);
        if (i != (*itr).second.end())
        {
            cond = (*i).second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForSmartEvent: found conditions for Smart Event entry or guid %d event_id %u", entryOrGuid, eventId);
        }
    }
    return cond;
}

ConditionList ConditionMgr::GetConditionsForNpcVendorEvent(uint32 creatureId, uint32 itemId)
{
    ConditionList cond;
    NpcVendorConditionContainer::const_iterator itr = NpcVendorConditionContainerStore.find(creatureId);
    if (itr != NpcVendorConditionContainerStore.end())
    {
        ConditionTypeContainer::const_iterator i = (*itr).second.find(itemId);
        if (i != (*itr).second.end())
        {
            cond = (*i).second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForNpcVendorEvent: found conditions for creature entry %u item %u", creatureId, itemId);
        }
    }
    return cond;
}

ConditionList ConditionMgr::GetConditionsForPhaseDefinition(uint32 zone, uint32 entry)
{
    ConditionList cond;
    PhaseDefinitionConditionContainer::const_iterator itr = PhaseDefinitionsConditionStore.find(zone);
    if (itr != PhaseDefinitionsConditionStore.end())
    {
        ConditionTypeContainer::const_iterator i = (*itr).second.find(entry);
        if (i != (*itr).second.end())
        {
            cond = (*i).second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForPhaseDefinition: found conditions for zone %u entry %u size %u", zone, entry, cond.size());
        }
    }

    return cond;
}

ConditionList ConditionMgr::GetConditionsForAreaTriggerAction(uint32 areaTriggerId, uint32 actionId)
{
    ConditionList cond;
    AreaTriggerConditionContainer::const_iterator itr = AreaTriggerConditionStore.find(areaTriggerId);
    if (itr != AreaTriggerConditionStore.end())
    {
        ConditionTypeContainer::const_iterator i = itr->second.find(actionId);
        if (i != itr->second.end())
        {
            cond = i->second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForAreaTriggerAction: found conditions for areatrigger id %u entry %u spell %u", areaTriggerId, actionId);
        }
    }

    return cond;
}

ConditionList ConditionMgr::GetConditionsForItemLoot(uint32 creatureId, uint32 itemId)
{
    ConditionList cond;
    ItemLootConditionContainer::const_iterator itr = ItemLootConditionStore.find(creatureId);
    if (itr != ItemLootConditionStore.end())
    {
        ConditionTypeContainer::const_iterator i = itr->second.find(itemId);
        if (i != itr->second.end())
        {
            cond = i->second;
            TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForItemLoot: found conditions for creatureId %u itemId %u", creatureId, itemId);
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_CONDITIONSYS, "GetConditionsForItemLoot: conditions for creatureId %u itemId %u", creatureId, itemId);

    return cond;
}

void ConditionMgr::LoadConditions(bool isReload)
{
    uint32 oldMSTime = getMSTime();

    Clean();

    //must clear all custom handled cases (groupped types) before reload
    if (isReload)
    {
        TC_LOG_INFO(LOG_FILTER_GENERAL, "Reseting Loot Conditions...");
        LootTemplates_Creature.ResetConditions();
        LootTemplates_Fishing.ResetConditions();
        LootTemplates_Gameobject.ResetConditions();
        LootTemplates_Item.ResetConditions();
        LootTemplates_Mail.ResetConditions();
        LootTemplates_Milling.ResetConditions();
        LootTemplates_Pickpocketing.ResetConditions();
        LootTemplates_Reference.ResetConditions();
        LootTemplates_Skinning.ResetConditions();
        LootTemplates_Disenchant.ResetConditions();
        LootTemplates_Prospecting.ResetConditions();
        LootTemplates_Spell.ResetConditions();
        LootTemplates_World.ResetConditions();

        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `gossip_menu` Table for Conditions!");
        sGossipDataStore->LoadGossipMenu();

        TC_LOG_INFO(LOG_FILTER_GENERAL, "Re-Loading `gossip_menu_option` Table for Conditions!");
        sGossipDataStore->LoadGossipMenuItems();
        sSpellMgr->UnloadSpellInfoImplicitTargetConditionLists();
    }

    QueryResult result = WorldDatabase.Query("SELECT SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, "
                                             " ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName FROM conditions");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 conditions. DB table `conditions` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        Condition* cond = new Condition();
        int32 iSourceTypeOrReferenceId   = fields[0].GetInt32();
        cond->SourceGroup               = fields[1].GetUInt32();
        cond->SourceEntry               = fields[2].GetUInt32();
        cond->SourceId                  = fields[3].GetInt32();
        cond->ElseGroup                 = fields[4].GetUInt32();
        int32 iConditionTypeOrReference  = fields[5].GetInt32();
        cond->ConditionTarget           = fields[6].GetUInt8();
        cond->ConditionValue1           = fields[7].GetUInt32();
        cond->ConditionValue2           = fields[8].GetUInt32();
        cond->ConditionValue3           = fields[9].GetUInt32();
        cond->NegativeCondition         = fields[10].GetUInt8() != 0;
        cond->ErrorTextId                 = fields[11].GetUInt32();
        cond->ScriptId                  = sObjectMgr->GetScriptId(fields[12].GetCString());

        if (iConditionTypeOrReference >= 0)
            cond->ConditionType = ConditionTypes(iConditionTypeOrReference);

        if (iSourceTypeOrReferenceId >= 0)
            cond->SourceType = ConditionSourceType(iSourceTypeOrReferenceId);

        if (iConditionTypeOrReference < 0)//it has a reference
        {
            if (iConditionTypeOrReference == iSourceTypeOrReferenceId)//self referencing, skip
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition reference %i is referencing self, skipped", iSourceTypeOrReferenceId);
                delete cond;
                continue;
            }
            cond->ReferenceId = uint32(abs(iConditionTypeOrReference));

            const char* rowType = "reference template";
            if (iSourceTypeOrReferenceId >= 0)
                rowType = "reference";
            //check for useless data
            if (cond->ConditionTarget)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in ConditionTarget (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionTarget);
            if (cond->ConditionValue1)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in value1 (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionValue1);
            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in value2 (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in value3 (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionValue3);
            if (cond->NegativeCondition)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in NegativeCondition (%u)!", rowType, iSourceTypeOrReferenceId, cond->NegativeCondition);
            if (cond->SourceGroup && iSourceTypeOrReferenceId < 0)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in SourceGroup (%u)!", rowType, iSourceTypeOrReferenceId, cond->SourceGroup);
            if (cond->SourceEntry && iSourceTypeOrReferenceId < 0)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition %s %i has useless data in SourceEntry (%u)!", rowType, iSourceTypeOrReferenceId, cond->SourceEntry);
        }
        else if (!isConditionTypeValid(cond))//doesn't have reference, validate ConditionType
        {
            delete cond;
            continue;
        }

        if (iSourceTypeOrReferenceId < 0)//it is a reference template
        {
            uint32 uRefId = abs(iSourceTypeOrReferenceId);
            if (ConditionReferenceStore.find(uRefId) == ConditionReferenceStore.end())//make sure we have a list for our conditions, based on reference id
            {
                ConditionList mCondList;
                ConditionReferenceStore[uRefId] = mCondList;
            }
            ConditionReferenceStore[uRefId].push_back(cond);//add to reference storage
            count++;
            continue;
        }//end of reference templates

        //if not a reference and SourceType is invalid, skip
        if (iConditionTypeOrReference >= 0 && !isSourceTypeValid(cond))
        {
            delete cond;
            continue;
        }

        //Grouping is only allowed for some types (loot templates, gossip menus, gossip items)
        if (cond->SourceGroup && !CanHaveSourceGroupSet(cond->SourceType))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Condition type %u has not allowed value of SourceGroup = %u!", uint32(cond->SourceType), cond->SourceGroup);
            delete cond;
            continue;
        }
        if (cond->SourceId && !CanHaveSourceIdSet(cond->SourceType))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Condition type %u has not allowed value of SourceId = %u!", uint32(cond->SourceType), cond->SourceId);
            delete cond;
            continue;
        }

        if (cond->SourceGroup)
        {
            bool valid = false;
            // handle grouped conditions
            switch (cond->SourceType)
            {
                case CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Creature.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Disenchant.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Fishing.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Gameobject.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Item.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Mail.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Milling.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Pickpocketing.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Prospecting.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Reference.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Skinning.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Spell.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_WORLD_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_World.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_GOSSIP_MENU:
                    valid = addToGossipMenus(cond);
                    break;
                case CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION:
                    valid = addToGossipMenuItems(cond);
                    break;
                case CONDITION_SOURCE_TYPE_SPELL_CLICK_EVENT:
                {
                    SpellClickEventConditionStore[cond->SourceGroup][cond->SourceEntry].push_back(cond);
                    valid = true;
                    ++count;
                    continue;   // do not add to m_AllocatedMemory to avoid double deleting
                }
                case CONDITION_SOURCE_TYPE_SPELL_IMPLICIT_TARGET:
                    valid = addToSpellImplicitTargetConditions(cond);
                    break;
                case CONDITION_SOURCE_TYPE_VEHICLE_SPELL:
                {
                    VehicleSpellConditionStore[cond->SourceGroup][cond->SourceEntry].push_back(cond);
                    valid = true;
                    ++count;
                    continue;   // do not add to m_AllocatedMemory to avoid double deleting
                }
                case CONDITION_SOURCE_TYPE_SMART_EVENT:
                {
                    //! TODO: PAIR_32 ?
                    std::pair<int32, uint32> key = std::make_pair(cond->SourceEntry, cond->SourceId);
                    SmartEventConditionStore[key][cond->SourceGroup].push_back(cond);
                    valid = true;
                    ++count;
                    continue;
                }
                case CONDITION_SOURCE_TYPE_NPC_VENDOR:
                {
                    NpcVendorConditionContainerStore[cond->SourceGroup][cond->SourceEntry].push_back(cond);
                    valid =  true;
                    ++count;
                    continue;
                }
                case CONDITION_SOURCE_TYPE_PHASE_DEFINITION:
                {
                    PhaseDefinitionsConditionStore[cond->SourceGroup][cond->SourceEntry].push_back(cond);
                    valid = true;
                    ++count;
                    continue;
                }
                case CONDITION_SOURCE_TYPE_AREATRIGGER_ACTION:
                {
                    AreaTriggerConditionStore[cond->SourceGroup][cond->SourceEntry].push_back(cond);
                    valid = true;
                    ++count;
                    continue;
                }
                case CONDITION_SOURCE_TYPE_LOOT_ITEM:
                {
                    ItemLootConditionStore[cond->SourceGroup][cond->SourceEntry].push_back(cond);
                    valid = true;
                    ++count;
                    continue;
                }
                default:
                    break;
            }

            if (!valid)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Not handled grouped condition, SourceGroup %u", cond->SourceGroup);
                delete cond;
            }
            else
            {
                AllocatedMemoryStore.push_back(cond);
                ++count;
            }
            continue;
        }

        //handle not grouped conditions
        //make sure we have a storage list for our SourceType
        if (ConditionStore.find(cond->SourceType) == ConditionStore.end())
        {
            ConditionTypeContainer mTypeMap;
            ConditionStore[cond->SourceType] = mTypeMap;//add new empty list for SourceType
        }

        //make sure we have a condition list for our SourceType's entry
        if (ConditionStore[cond->SourceType].find(cond->SourceEntry) == ConditionStore[cond->SourceType].end())
        {
            ConditionList mCondList;
            ConditionStore[cond->SourceType][cond->SourceEntry] = mCondList;
        }

        //add new Condition to storage based on Type/Entry
        ConditionStore[cond->SourceType][cond->SourceEntry].push_back(cond);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u conditions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

bool ConditionMgr::addToLootTemplate(Condition* cond, LootTemplate* loot)
{
    if (!loot)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "ConditionMgr: LootTemplate %u not found", cond->SourceGroup);
        return false;
    }

    if (loot->addConditionItem(cond))
        return true;

    TC_LOG_ERROR(LOG_FILTER_SQL, "ConditionMgr: Item %u not found in LootTemplate %u", cond->SourceEntry, cond->SourceGroup);
    WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceEntry = %u AND SourceGroup = %u", cond->SourceEntry, cond->SourceGroup);
    return false;
}

bool ConditionMgr::addToGossipMenus(Condition* cond)
{
    auto menuBounds = sGossipDataStore->GetGossipMenusMapBoundsNonConst(cond->SourceGroup);
    if (menuBounds.first != menuBounds.second)
    {
        for (auto itr = menuBounds.first; itr != menuBounds.second; ++itr)
        {
            if ((*itr).second.Entry == cond->SourceGroup && (*itr).second.TextID == uint32(cond->SourceEntry))
            {
                (*itr).second.Conditions.push_back(cond);
                return true;
            }
        }
    }

    TC_LOG_ERROR(LOG_FILTER_SQL, "addToGossipMenus: GossipMenu %u not found", cond->SourceGroup);
    return false;
}

bool ConditionMgr::addToGossipMenuItems(Condition* cond)
{
    auto menuItemBounds = sGossipDataStore->GetGossipMenuItemsMapBoundsNonConst(cond->SourceGroup);
    if (menuItemBounds.first != menuItemBounds.second)
    {
        for (auto itr = menuItemBounds.first; itr != menuItemBounds.second; ++itr)
        {
            if ((*itr).second.MenuID == cond->SourceGroup && (*itr).second.OptionIndex == uint32(cond->SourceEntry))
            {
                (*itr).second.Conditions.push_back(cond);
                return true;
            }
        }
    }

    TC_LOG_ERROR(LOG_FILTER_SQL, "addToGossipMenuItems: GossipMenuId %u Item %u not found", cond->SourceGroup, cond->SourceEntry);
    return false;
}

bool ConditionMgr::addToSpellImplicitTargetConditions(Condition* cond)
{
    uint32 conditionEffMask = cond->SourceGroup;
    SpellInfo* spellInfo = const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(cond->SourceEntry));
    ASSERT(spellInfo);
    std::list<uint32> sharedMasks;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->EffectMask < uint32(1 << i))
            break;

        // check if effect is already a part of some shared mask
        bool found = false;
        for (std::list<uint32>::iterator itr = sharedMasks.begin(); itr != sharedMasks.end(); ++itr)
        {
            if ((1<<i) & *itr)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        // build new shared mask with found effect
        uint32 sharedMask = (1<<i);
        ConditionList* cmp = spellInfo->Effects[i]->ImplicitTargetConditions;
        for (uint8 effIndex = i+1; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        {
            if (spellInfo->EffectMask < uint32(1 << effIndex))
                break;

            if (spellInfo->Effects[effIndex]->ImplicitTargetConditions == cmp)
                sharedMask |= 1<<effIndex;
        }
        sharedMasks.push_back(sharedMask);
    }

    for (std::list<uint32>::iterator itr = sharedMasks.begin(); itr != sharedMasks.end(); ++itr)
    {
        // some effect indexes should have same data
        if (uint32 commonMask = *itr & conditionEffMask)
        {
            uint8 firstEffIndex = 0;
            for (; firstEffIndex < MAX_SPELL_EFFECTS; ++firstEffIndex)
                if ((1<<firstEffIndex) & *itr)
                    break;

            if ((spellInfo->EffectMask & (1 << firstEffIndex)) == 0)
                continue;

            // get shared data
            ConditionList* sharedList = spellInfo->Effects[firstEffIndex]->ImplicitTargetConditions;

            // there's already data entry for that sharedMask
            if (sharedList)
            {
                // we have overlapping masks in db
                if (conditionEffMask != *itr)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, has incorrect SourceGroup %u (spell effectMask) set - "
                        "effect masks are overlapping (all SourceGroup values having given bit set must be equal) - ignoring.", cond->SourceEntry, cond->SourceGroup);
                    return false;
                }
            }
            // no data for shared mask, we can create new submask
            else
            {
                // add new list, create new shared mask
                sharedList = new ConditionList();
                for (uint8 i = firstEffIndex; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (spellInfo->EffectMask < uint32(1 << i))
                        break;

                    if ((1<<i) & commonMask)
                        spellInfo->Effects[i]->ImplicitTargetConditions = sharedList;
                }
            }
            sharedList->push_back(cond);
            break;
        }
    }
    return true;
}

bool ConditionMgr::isSourceTypeValid(Condition* cond)
{
    if (cond->SourceType == CONDITION_SOURCE_TYPE_NONE || cond->SourceType >= CONDITION_SOURCE_TYPE_MAX)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid ConditionSourceType %u in `condition` table, ignoring.", uint32(cond->SourceType));
        return false;
    }

    switch (cond->SourceType)
    {
        case CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Creature.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `creature_loot_template`, ignoring.", cond->SourceGroup);
                WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceGroup = %u AND SourceTypeOrReferenceId = %u", cond->SourceGroup, cond->SourceType);
                return false;
            }

            LootTemplate* loot = LootTemplates_Creature.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Disenchant.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `disenchant_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Disenchant.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Fishing.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `fishing_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Fishing.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Gameobject.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `gameobject_loot_template`, ignoring.", cond->SourceGroup);
                WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceGroup = %u AND SourceTypeOrReferenceId = %u", cond->SourceGroup, cond->SourceType);
                return false;
            }

            LootTemplate* loot = LootTemplates_Gameobject.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Item.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `item_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Item.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Mail.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `mail_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Mail.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Milling.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `milling_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Milling.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Pickpocketing.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `pickpocketing_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Pickpocketing.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Prospecting.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `prospecting_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Prospecting.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Reference.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `reference_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Reference.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Skinning.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `skinning_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Skinning.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Spell.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `spell_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Spell.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_WORLD_LOOT_TEMPLATE:
        {
            if (!LootTemplates_World.HaveLootFor(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceGroup %u in `condition` table, does not exist in `spell_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_World.GetLootForConditionFill(cond->SourceGroup);
            ItemTemplate const* pItemProto = sObjectMgr->GetItemTemplate(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SPELL_IMPLICIT_TARGET:
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(cond->SourceEntry);
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `spell.dbc`, ignoring.", cond->SourceEntry);
                WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceEntry = %u", cond->SourceEntry);
                return false;
            }

            if ((cond->SourceGroup > MAX_EFFECT_MASK) || !cond->SourceGroup)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, has incorrect SourceGroup %u (spell effectMask) set , ignoring.", cond->SourceEntry, cond->SourceGroup);
                return false;
            }

            uint32 origGroup = cond->SourceGroup;

            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (spellInfo->EffectMask < uint32(1 << i))
                    break;

                if (!((1<<i) & cond->SourceGroup))
                    continue;

                switch (spellInfo->Effects[i]->TargetA.GetSelectionCategory())
                {
                    case TARGET_SELECT_CATEGORY_NEARBY:
                    case TARGET_SELECT_CATEGORY_CONE:
                    case TARGET_SELECT_CATEGORY_AREA:
                    case TARGET_SELECT_CATEGORY_DEFAULT:
                        continue;
                    default:
                        break;
                }

                switch (spellInfo->Effects[i]->TargetB.GetSelectionCategory())
                {
                    case TARGET_SELECT_CATEGORY_NEARBY:
                    case TARGET_SELECT_CATEGORY_CONE:
                    case TARGET_SELECT_CATEGORY_AREA:
                    case TARGET_SELECT_CATEGORY_DEFAULT:
                        continue;
                    default:
                        break;
                }

                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u SourceGroup %u in `condition` table - spell %u does not have implicit targets of types: _AREA_, _CONE_, _NEARBY_ for effect %u, SourceGroup needs correction, ignoring.", cond->SourceEntry, origGroup, cond->SourceEntry, uint32(i));
                cond->SourceGroup &= ~(1<<i);
            }
            // all effects were removed, no need to add the condition at all
            if (!cond->SourceGroup)
                return false;
            break;
        }
        case CONDITION_SOURCE_TYPE_CREATURE_TEMPLATE_VEHICLE:
        {
            if (!sObjectMgr->GetCreatureTemplate(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `creature_template`, ignoring.", cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SPELL:
        case CONDITION_SOURCE_TYPE_SPELL_PROC:
        {
            SpellInfo const* spellProto = sSpellMgr->GetSpellInfo(cond->SourceEntry);
            if (!spellProto)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `spell.dbc`, ignoring.", cond->SourceEntry);
                WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceEntry = %u", cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_QUEST_ACCEPT:
            if (!sQuestDataStore->GetQuestTemplate(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "CONDITION_SOURCE_TYPE_QUEST_ACCEPT specifies non-existing quest (%u), skipped", cond->SourceEntry);
                return false;
            }
            break;
        case CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK:
            if (!sQuestDataStore->GetQuestTemplate(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK specifies non-existing quest (%u), skipped", cond->SourceEntry);
                return false;
            }
            break;
        case CONDITION_SOURCE_TYPE_VEHICLE_SPELL:
            if (!sObjectMgr->GetCreatureTemplate(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `creature_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            if (!sSpellMgr->GetSpellInfo(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `spell.dbc`, ignoring.", cond->SourceEntry);
                WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceEntry = %u", cond->SourceEntry);
                return false;
            }
            break;
        case CONDITION_SOURCE_TYPE_SPELL_CLICK_EVENT:
            if (!sObjectMgr->GetCreatureTemplate(cond->SourceGroup))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `creature_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            if (!sSpellMgr->GetSpellInfo(cond->SourceEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `spell.dbc`, ignoring.", cond->SourceEntry);
                WorldDatabase.PExecute("DELETE FROM `conditions` WHERE SourceEntry = %u", cond->SourceEntry);
                return false;
            }
            break;
        case CONDITION_SOURCE_TYPE_NPC_VENDOR:
            {
                // Now we can use 0 entry NPC for all npc
                // if (!sObjectMgr->GetCreatureTemplate(cond->SourceGroup))
                // {
                    // TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `creature_template`, ignoring.", cond->SourceGroup);
                    // return false;
                // }
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(cond->SourceEntry);
                if (!itemTemplate)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceEntry);
                    return false;
                }
            }
            break;
        case CONDITION_SOURCE_TYPE_PHASE_DEFINITION:
            if (!PhaseMgr::IsConditionTypeSupported(cond->ConditionType))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Condition source type `CONDITION_SOURCE_TYPE_PHASE_DEFINITION` does not support condition type %u, ignoring.", cond->ConditionType);
                return false;
            }
            break;
        case CONDITION_SOURCE_TYPE_AREATRIGGER_ACTION:
        {
            AreaTriggerInfo const* info = sAreaTriggerDataStore->GetAreaTriggerInfo(cond->SourceGroup);
            if (!info || info->actions.empty())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Source group %u for `CONDITION_SOURCE_TYPE_AREATRIGGER_ACTION` is not valid because there is no areatrigger actions associated.", cond->SourceGroup);
                return false;
            }
            bool found = false;
            for (AreaTriggerActionList::const_iterator itr = info->actions.begin(); itr != info->actions.end(); ++itr)
                if (itr->id == cond->SourceEntry)
                {
                    found = true;
                    break;
                }
            if (!found)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Source group %u for `CONDITION_SOURCE_TYPE_AREATRIGGER_ACTION` has entry %u - action id that does not exist.", cond->SourceGroup, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_LOOT_ITEM:
        {
            if (cond->SourceGroup == 1)
            {
                ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(cond->SourceEntry);
                if (!itemTemplate)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceEntry);
                    return false;
                }
            }
            if (cond->SourceGroup == 2)
            {
                CurrencyTypesEntry const * currencyEntry = sCurrencyTypesStore.LookupEntry(cond->SourceEntry);
                if (!currencyEntry)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "SourceEntry %u in `condition` table, does not exist in `CurrencyTypesEntry`, ignoring.", cond->SourceEntry);
                    return false;
                }
            }
        }
        break;
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU:
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION:
        case CONDITION_SOURCE_TYPE_SMART_EVENT:
        case CONDITION_SOURCE_TYPE_PLAYER_CHOICE:
        case CONDITION_SOURCE_TYPE_PLAYER_CHOICE_RESPONS:
        case CONDITION_SOURCE_TYPE_WORLD_STATE:
        case CONDITION_SOURCE_TYPE_NONE:
        default:
            break;
    }

    return true;
}

bool ConditionMgr::isConditionTypeValid(Condition* cond)
{
    if (cond->ConditionType == CONDITION_NONE || cond->ConditionType >= CONDITION_MAX)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid ConditionType %u at SourceEntry %u in `condition` table, ignoring.", uint32(cond->ConditionType), cond->SourceEntry);
        return false;
    }

    if (cond->ConditionTarget >= cond->GetMaxAvailableConditionTargets())
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "SourceType %u, SourceEntry %u in `condition` table, has incorrect ConditionTarget set, ignoring.", cond->SourceType, cond->SourceEntry);
        return false;
    }

    switch (cond->ConditionType)
    {
        case CONDITION_AURA:
        {
            if (!sSpellMgr->GetSpellInfo(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Aura condition has non existing spell (Id: %d), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2 > EFFECT_2)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Aura condition has non existing effect index (%u) (must be 0..2), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Aura condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_ITEM:
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(cond->ConditionValue1);
            if (!proto)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Item condition has non existing item (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (!cond->ConditionValue2)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Item condition has 0 set for item count in value2 (%u), skipped", cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(cond->ConditionValue1);
            if (!proto)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ItemEquipped condition has non existing item (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ItemEquipped condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ItemEquipped condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_ZONEID:
        {
            AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(cond->ConditionValue1);
            if (!areaEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ZoneID condition has non existing area (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (areaEntry->ParentAreaID != 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ZoneID condition requires to be in area (%u) which is a subzone but zone expected, skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ZoneID condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ZoneID condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_REPUTATION_RANK:
        {
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(cond->ConditionValue1);
            if (!factionEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Reputation condition has non existing faction (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Reputation condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_TEAM:
        {
            if (cond->ConditionValue1 != ALLIANCE && cond->ConditionValue1 != HORDE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Team condition specifies unknown team (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Team condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Team condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_SKILL:
        {
            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(cond->ConditionValue1);
            if (!pSkill)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Skill condition specifies non-existing skill (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2 < 1 || cond->ConditionValue2 > sWorld->GetConfigMaxSkillValue())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Skill condition specifies invalid skill value (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Skill condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_QUESTREWARDED:
        case CONDITION_QUESTTAKEN:
        case CONDITION_QUEST_NONE:
        case CONDITION_QUEST_COMPLETE:
        case CONDITION_WORLD_QUEST:
        case CONDITION_ACOUNT_QUEST:
        {
            if (!sQuestDataStore->GetQuestTemplate(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Quest condition specifies non-existing quest (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2 > 1)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Quest condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Quest condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_QUEST_OBJECTIVE_DONE:
        {
            Quest const* quest = sQuestDataStore->GetQuestTemplate(cond->ConditionValue1);
            if (!quest)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Quest condition specifies non-existing quest (%u), skipped (objective_done)", cond->ConditionValue1 );
                return false;
            }

            if (cond->ConditionValue2 > 0)
            {
                bool found = false;
                for (QuestObjective const& obj : quest->GetObjectives())
                {
                    if (obj.ObjectID == cond->ConditionValue2)
                        found = true;
                }
                if (!found)
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Quest condition specifies non-existing quest (%d) non existen objective %d, skipped", cond->ConditionValue1, cond->ConditionValue2);
            }
            break;
        }
        case CONDITION_AREA_EXPLORED:
        {
            AreaTableEntry const* areaEntry = sDB2Manager.FindAreaEntry(cond->ConditionValue1);
            if (!areaEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Area explored condition specifies non-existing area (%u), skipped", cond->ConditionValue1);
                return false;
            }   
        }
        case CONDITION_ACTIVE_EVENT:
        {
            GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();

            if (cond->ConditionValue1 >= events.size())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ActiveEvent condition has non existing event id (%u), skipped max arr. size %u", cond->ConditionValue1, events.size());
                return false;
            }

            const GameEventData d = events[cond->ConditionValue1];
            if (!d.isValid())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ActiveEvent condition has non existing event id (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ActiveEvent condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ActiveEvent condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_ACHIEVEMENT:
        {
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(cond->ConditionValue1);
            if (!achievement)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Achivement condition has non existing achivement id (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Achivement condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Achivement condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_CLASS:
        {
            if (!(cond->ConditionValue1 & CLASSMASK_ALL_PLAYABLE))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Class condition has non existing classmask (%u), skipped", cond->ConditionValue1 & ~CLASSMASK_ALL_PLAYABLE);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Class condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Class condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_RACE:
        {
            if (!(cond->ConditionValue1 & RACEMASK_ALL_PLAYABLE))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Race condition has non existing racemask (%u), skipped", cond->ConditionValue1 & ~RACEMASK_ALL_PLAYABLE);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Race condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Race condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_GENDER:
        {
            if (!Player::IsValidGender(uint8(cond->ConditionValue1)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Gender condition has invalid gender (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Gender condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Gender condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_MAPID:
        {
            MapEntry const* me = sMapStore.LookupEntry(cond->ConditionValue1);
            if (!me)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Map condition has non existing map (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Map condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Map condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_SPELL:
        {
            if (!sSpellMgr->GetSpellInfo(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell condition has non existing spell (Id: %d), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_LEVEL:
        {
            if (cond->ConditionValue2 >= COMP_TYPE_MAX)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Level condition has invalid option (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Level condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_DRUNKENSTATE:
        {
            if (cond->ConditionValue1 > DRUNKEN_SMASHED)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DrunkState condition has invalid state (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue2)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DrunkState condition has useless data in value2 (%u)!", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "DrunkState condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_NEAR_CREATURE:
        {
            if (!sObjectMgr->GetCreatureTemplate(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "NearCreature condition has non existing creature template entry (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "NearCreature condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_NEAR_GAMEOBJECT:
        {
            if (!sObjectMgr->GetGameObjectTemplate(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "NearGameObject condition has non existing gameobject template entry (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "NearGameObject condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_OBJECT_ENTRY:
        {
            switch (cond->ConditionValue1)
            {
                case TYPEID_UNIT:
                    if (cond->ConditionValue2 && !sObjectMgr->GetCreatureTemplate(cond->ConditionValue2))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "ObjectEntry condition has non existing creature template entry  (%u), skipped", cond->ConditionValue2);
                        return false;
                    }
                    break;
                case TYPEID_GAMEOBJECT:
                    if (cond->ConditionValue2 && !sObjectMgr->GetGameObjectTemplate(cond->ConditionValue2))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "ObjectEntry condition has non existing game object template entry  (%u), skipped", cond->ConditionValue2);
                        return false;
                    }
                    break;
                case TYPEID_PLAYER:
                case TYPEID_CORPSE:
                    if (cond->ConditionValue2)
                        TC_LOG_ERROR(LOG_FILTER_SQL, "ObjectEntry condition has useless data in value2 (%u)!", cond->ConditionValue2);
                    break;
                default:
                    TC_LOG_ERROR(LOG_FILTER_SQL, "ObjectEntry condition has wrong typeid set (%u), skipped", cond->ConditionValue1);
                    return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "ObjectEntry condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_TYPE_MASK:
        {
            if (!cond->ConditionValue1 || (cond->ConditionValue1 & ~(TYPEMASK_UNIT | TYPEMASK_PLAYER | TYPEMASK_GAMEOBJECT | TYPEMASK_CORPSE)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "TypeMask condition has invalid typemask set (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "TypeMask condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "TypeMask condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_RELATION_TO:
        {
            if (cond->ConditionValue1 >= cond->GetMaxAvailableConditionTargets())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "RelationTo condition has invalid ConditionValue1(ConditionTarget selection) (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue1 == cond->ConditionTarget)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "RelationTo condition has ConditionValue1(ConditionTarget selection) set to self (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue2 >= RELATION_MAX)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "RelationTo condition has invalid ConditionValue2(RelationType) (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "RelationTo condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_REACTION_TO:
        {
            if (cond->ConditionValue1 >= cond->GetMaxAvailableConditionTargets())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ReactionTo condition has invalid ConditionValue1(ConditionTarget selection) (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue1 == cond->ConditionTarget)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "ReactionTo condition has ConditionValue1(ConditionTarget selection) set to self (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (!cond->ConditionValue2)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "mConditionValue2 condition has invalid ConditionValue2(rankMask) (%u), skipped", cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_DISTANCE_TO:
        {
            if (cond->ConditionValue1 >= cond->GetMaxAvailableConditionTargets())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DistanceTo condition has invalid ConditionValue1(ConditionTarget selection) (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue1 == cond->ConditionTarget)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DistanceTo condition has ConditionValue1(ConditionTarget selection) set to self (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue3 >= COMP_TYPE_MAX)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DistanceTo condition has invalid ComparisionType (%u), skipped", cond->ConditionValue3);
                return false;
            }
            break;
        }
        case CONDITION_ALIVE:
        {
            if (cond->ConditionValue1)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Alive condition has useless data in value1 (%u)!", cond->ConditionValue1);
            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Alive condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Alive condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_HP_VAL:
        {
            if (cond->ConditionValue2 >= COMP_TYPE_MAX)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "HpVal condition has invalid ComparisionType (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "HpVal condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_HP_PCT:
        {
            if (cond->ConditionValue1 > 100)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "HpPct condition has too big percent value (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue2 >= COMP_TYPE_MAX)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "HpPct condition has invalid ComparisionType (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "HpPct condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_AREAID:
        case CONDITION_INSTANCE_INFO:
            break;
        case CONDITION_WORLD_STATE:
        {
            if (!sWorld->getWorldState(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "World state condition has non existing world state in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "World state condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_PHASEMASK:
        {
            if (cond->ConditionValue2)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Phasemask condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                TC_LOG_ERROR(LOG_FILTER_SQL, "Phasemask condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_TITLE:
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(cond->ConditionValue1);
            if (!titleEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Title condition has non existing title in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_SPAWNMASK:
            {
                if (cond->ConditionValue1 >= (UI64LIT(1) << MAX_DIFFICULTY))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Map Difficulty condition has non existing map difficulty in value1 (%u), skipped", cond->ConditionValue1);
                    return false;
                }
                break;
            }
        case CONDITION_UNUSED_21:
            TC_LOG_ERROR(LOG_FILTER_SQL, "Found ConditionTypeOrReference = CONDITION_UNUSED_21 in `conditions` table - ignoring");
            return false;
        case CONDITION_UNUSED_24:
            TC_LOG_ERROR(LOG_FILTER_SQL, "Found ConditionTypeOrReference = CONDITION_UNUSED_24 in `conditions` table - ignoring");
            return false;
        case CONDITION_REALM_ACHIEVEMENT:
        {
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(cond->ConditionValue1);
            if (!achievement)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "AchievementEntry has non existing realm first achivement id (%u), skipped.", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_IN_WATER:
            break;
        case CONDITION_STAND_STATE:
        {
            bool valid = false;
            switch (cond->ConditionValue1)
            {
                case 0:
                    valid = cond->ConditionValue2 <= UNIT_STAND_STATE_SUBMERGED;
                    break;
                case 1:
                    valid = cond->ConditionValue2 <= 1;
                    break;
                default:
                    valid = false;
                    break;
            }
            if (!valid)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "StandState has non-existing stand state (%u,%u), skipped.", cond->ConditionValue1, cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_CLASS_HALL_ADVANCEMENT:
        {
            GarrTalentEntry const* entry = sGarrTalentStore.LookupEntry(cond->ConditionValue1);
            if (!entry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "CONDITION_CLASS_HALL_ADVANCEMENT has non-existing class hall tallentID  - %u, skipped.", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_CURRENCY:
        case CONDITION_CURRENCY_ON_WEEK:
        {
            CurrencyTypesEntry const * currencyEntry = sCurrencyTypesStore.LookupEntry(cond->ConditionValue1);
            if (!currencyEntry)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Currency condition has non existing currency (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_ARTIFACT_LEVEL:
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(cond->ConditionValue1);
            if (!proto)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Item condition has non existing item (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_GET_AMOUNT_STACK_AURA:
        {
            if (!sSpellMgr->GetSpellInfo(cond->ConditionValue1))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Aura (CONDITION_GET_AMOUNT_STACK_AURA) condition has non existing spell (Id: %d), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void ConditionMgr::Clean()
{
    for (ConditionReferenceContainer::iterator itr = ConditionReferenceStore.begin(); itr != ConditionReferenceStore.end(); ++itr)
    {
        for (ConditionList::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;
        itr->second.clear();
    }

    ConditionReferenceStore.clear();

    for (ConditionContainer::iterator itr = ConditionStore.begin(); itr != ConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    ConditionStore.clear();

    for (CreatureSpellConditionContainer::iterator itr = VehicleSpellConditionStore.begin(); itr != VehicleSpellConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    VehicleSpellConditionStore.clear();

    for (SmartEventConditionContainer::iterator itr = SmartEventConditionStore.begin(); itr != SmartEventConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    SmartEventConditionStore.clear();

    for (CreatureSpellConditionContainer::iterator itr = SpellClickEventConditionStore.begin(); itr != SpellClickEventConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    SpellClickEventConditionStore.clear();

    for (NpcVendorConditionContainer::iterator itr = NpcVendorConditionContainerStore.begin(); itr != NpcVendorConditionContainerStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    NpcVendorConditionContainerStore.clear();

    for (PhaseDefinitionConditionContainer::iterator itr = PhaseDefinitionsConditionStore.begin(); itr != PhaseDefinitionsConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    PhaseDefinitionsConditionStore.clear();

    for (AreaTriggerConditionContainer::iterator itr = AreaTriggerConditionStore.begin(); itr != AreaTriggerConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    AreaTriggerConditionStore.clear();

    for (ItemLootConditionContainer::iterator itr = ItemLootConditionStore.begin(); itr != ItemLootConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    ItemLootConditionStore.clear();

    // this is a BIG hack, feel free to fix it if you can figure out the ConditionMgr ;)
    for (std::list<Condition*>::const_iterator itr = AllocatedMemoryStore.begin(); itr != AllocatedMemoryStore.end(); ++itr)
        delete *itr;

    AllocatedMemoryStore.clear();
}

inline bool PlayerConditionCompare(int32 comparisonType, int32 value1, int32 value2)
{
    switch (comparisonType)
    {
        case 1:
            return value1 == value2;
        case 2:
            return value1 != value2;
        case 3:
            return value1 > value2;
        case 4:
            return value1 >= value2;
        case 5:
            return value1 < value2;
        case 6:
            return value1 <= value2;
        default:
            break;
    }
    return false;
}

template<std::size_t N>
bool PlayerConditionLogic(uint32 logic, std::array<bool, N>& results)
{
    static_assert(N < 16, "Logic array size must be equal to or less than 16");

    for (std::size_t i = 0; i < results.size(); ++i)
        if ((logic >> (16 + i)) & 1)
            results[i] ^= true;

    bool result = results[0];
    for (std::size_t i = 1; i < results.size(); ++i)
    {
        switch ((logic >> (2 * (i - 1))) & 3)
        {
            case 1:
                result = result && results[i];
                break;
            case 2:
                result = result || results[i];
                break;
            default:
                break;
        }
    }

    return result;
}

bool ConditionMgr::IsPlayerMeetingCondition(Unit* unit, int32 conditionID, bool send)
{
    if (!conditionID)
        return true;

    PlayerConditionEntry const* condition = sPlayerConditionStore.LookupEntry(conditionID);
    if (!condition)
        return true;

    if (!unit)
        return false;

    if (IsPlayerMeetingCondition(unit, condition))
        return true;

    if (send && unit->ToPlayer())
        unit->ToPlayer()->SendDirectMessage(WorldPackets::Character::FailedPlayerCondition(conditionID).Write());

    return false;
}

bool ConditionMgr::IsPlayerMeetingCondition(Unit* unit, PlayerConditionEntry const* condition)
{
    if (!unit)
        return false;

    Player* player = unit->ToPlayer();

    if (condition->MinLevel && unit->getLevel() < condition->MinLevel)
        return false;

    if (condition->MaxLevel && unit->getLevel() > condition->MaxLevel)
        return false;

    if (condition->RaceMask && !(unit->getRaceMask() & condition->RaceMask))
        return false;

    if (condition->ClassMask && !(unit->getClassMask() & condition->ClassMask))
        return false;

    if (condition->Gender >= 0 && unit->getGender() != condition->Gender)
        return false;

    if (condition->NativeGender >= 0 && unit->getGender() != condition->NativeGender)
        return false;

    if (player && condition->ModifierTreeID)
        if (!player->GetAchievementMgr()->CheckModifierTree(condition->ModifierTreeID, player))
            return false;

    if (condition->PowerType != -1 && condition->PowerTypeComp)
    {
        int32 requiredPowerValue = condition->Flags & 4 ? unit->GetMaxPower(Powers(condition->PowerType)) : condition->PowerTypeValue;
        if (!PlayerConditionCompare(condition->PowerTypeComp, unit->GetPower(Powers(condition->PowerType)), requiredPowerValue))
            return false;
    }

    if (player && (condition->ChrSpecializationIndex >= 0 || condition->ChrSpecializationRole >= 0))
    {
        if (ChrSpecializationEntry const* spec = sChrSpecializationStore.LookupEntry(player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID)))
        {
            if (condition->ChrSpecializationIndex >= 0 && spec->OrderIndex != uint32(condition->ChrSpecializationIndex))
                return false;

            if (condition->ChrSpecializationRole >= 0 && spec->Role != uint32(condition->ChrSpecializationRole))
                return false;
        }
    }

    if (player && (condition->SkillID[0] || condition->SkillID[1] || condition->SkillID[2] || condition->SkillID[3]))
    {
        using SkillCount = std::extent<decltype(condition->SkillID)>;

        std::array<bool, SkillCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < SkillCount::value; ++i)
        {
            if (condition->SkillID[i])
            {
                uint16 skillValue = player->GetSkillValue(condition->SkillID[i]);
                results[i] = skillValue != 0 && skillValue > condition->MinSkill[i] && (!condition->MaxSkill[i] || skillValue < condition->MaxSkill[i]);
            }
        }

        if (!PlayerConditionLogic(condition->SkillLogic, results))
            return false;
    }

    if (player && condition->LanguageID)
    {
        if (LanguageDesc const* lang = GetLanguageDescByID(condition->LanguageID))
        {
            uint32 languageSkill = player->GetSkillValue(lang->skill_id);
            if (!languageSkill && player->HasAuraTypeWithMiscvalue(SPELL_AURA_COMPREHEND_LANGUAGE, condition->LanguageID))
                languageSkill = 300;

            if (condition->MinLanguage && languageSkill < condition->MinLanguage)
                return false;

            if (condition->MaxLanguage && languageSkill > condition->MaxLanguage)
                return false;
        }
    }

    if (player && condition->MinFactionID[0] && condition->MinFactionID[1] && condition->MinFactionID[2] && condition->MaxFactionID)
    {
        if (!condition->MinFactionID[0] && !condition->MinFactionID[1] && !condition->MinFactionID[2])
        {
            if (ReputationRank const* forcedRank = player->GetReputationMgr().GetForcedRankIfAny(condition->MaxFactionID))
            {
                if (*forcedRank > ReputationRank(condition->MaxReputation))
                    return false;
            }
            else if (player->GetReputationRank(condition->MaxFactionID) > ReputationRank(condition->MaxReputation))
                return false;
        }
        else
        {
            using FactionCount = std::extent<decltype(condition->MinFactionID)>;

            std::array<bool, FactionCount::value + 1> results;
            results.fill(true);
            for (std::size_t i = 0; i < FactionCount::value; ++i)
            {
                if (condition->MinFactionID[i])
                {
                    if (ReputationRank const* forcedRank = player->GetReputationMgr().GetForcedRankIfAny(condition->MinFactionID[i]))
                        results[i] = *forcedRank >= ReputationRank(condition->MinReputation[i]);
                    else
                        results[i] = player->GetReputationRank(condition->MinFactionID[i]) >= ReputationRank(condition->MinReputation[i]);
                }
            }

            if (ReputationRank const* forcedRank = player->GetReputationMgr().GetForcedRankIfAny(condition->MaxFactionID))
                results[3] = *forcedRank <= ReputationRank(condition->MaxReputation);
            else
                results[3] = player->GetReputationRank(condition->MaxFactionID) <= ReputationRank(condition->MaxReputation);

            if (!PlayerConditionLogic(condition->ReputationLogic, results))
                return false;
        }
    }

    if (player && condition->PvpMedal && !((1 << (condition->PvpMedal - 1)) & player->GetUInt32Value(PLAYER_FIELD_PVP_MEDALS)))
        return false;

    if (player && condition->LifetimeMaxPVPRank && player->GetLifetimeMaxPvPRankValue() != condition->LifetimeMaxPVPRank)
        return false;

    if (player && condition->PartyStatus)
    {
        Group* group = player->GetGroup();
        switch (condition->PartyStatus)
        {
            case 1:
                if (group)
                    return false;
                break;
            case 2:
                if (!group)
                    return false;
                break;
            case 3:
                if (!group || group->isRaidGroup())
                    return false;
                break;
            case 4:
                if (!group || !group->isRaidGroup())
                    return false;
                break;
            case 5:
                if (group && group->isRaidGroup())
                    return false;
                break;
            default:
                break;
        }
    }

    if (player && condition->PrevQuestID[0])
    {
        using PrevQuestCount = std::extent<decltype(condition->PrevQuestID)>;

        std::array<bool, PrevQuestCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < PrevQuestCount::value; ++i)
            if (uint32 questBit = sDB2Manager.GetQuestUniqueBitFlag(condition->PrevQuestID[i]))
                results[i] = (player->GetUInt32Value(PLAYER_FIELD_QUEST_COMPLETED + ((questBit - 1) >> 5)) & (1 << ((questBit - 1) & 31))) != 0;

        if (!PlayerConditionLogic(condition->PrevQuestLogic, results))
            return false;
    }

    if (player && condition->CurrQuestID[0])
    {
        using CurrQuestCount = std::extent<decltype(condition->CurrQuestID)>;

        std::array<bool, CurrQuestCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < CurrQuestCount::value; ++i)
            if (condition->CurrQuestID[i])
                results[i] = player->FindQuestSlot(condition->CurrQuestID[i]) != MAX_QUEST_LOG_SIZE;

        if (!PlayerConditionLogic(condition->CurrQuestLogic, results))
            return false;
    }

    if (player && condition->CurrentCompletedQuestID[0])
    {
        using CurrentCompletedQuestCount = std::extent<decltype(condition->CurrentCompletedQuestID)>;

        std::array<bool, CurrentCompletedQuestCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < CurrentCompletedQuestCount::value; ++i)
            if (condition->CurrentCompletedQuestID[i])
                results[i] = player->GetQuestStatus(condition->CurrentCompletedQuestID[i]) == QUEST_STATUS_COMPLETE;

        if (!PlayerConditionLogic(condition->CurrentCompletedQuestLogic, results))
            return false;
    }

    if (player && condition->SpellID[0])
    {
        using SpellCount = std::extent<decltype(condition->SpellID)>;

        std::array<bool, SpellCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < SpellCount::value; ++i)
            if (condition->SpellID[i])
                results[i] = player->HasSpell(condition->SpellID[i]);

        if (!PlayerConditionLogic(condition->SpellLogic, results))
            return false;
    }

    if (player && condition->ItemID[0])
    {
        using ItemCount = std::extent<decltype(condition->ItemID)>;

        std::array<bool, ItemCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < ItemCount::value; ++i)
            if (condition->ItemID[i])
                results[i] = player->GetItemCount(condition->ItemID[i], condition->ItemFlags != 0) >= condition->ItemCount[i];

        if (!PlayerConditionLogic(condition->ItemLogic, results))
            return false;
    }

    if (player && condition->CurrencyID[0])
    {
        using CurrencyCount = std::extent<decltype(condition->CurrencyID)>;

        std::array<bool, CurrencyCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < CurrencyCount::value; ++i)
            if (condition->CurrencyID[i])
                results[i] = player->GetCurrency(condition->CurrencyID[i]) >= condition->CurrencyCount[i];

        if (!PlayerConditionLogic(condition->CurrencyLogic, results))
            return false;
    }

    if (player && (condition->Explored[0] || condition->Explored[1]))
    {
        using ExploredCount = std::extent<decltype(condition->Explored)>;

        for (std::size_t i = 0; i < ExploredCount::value; ++i)
        {
            if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(condition->Explored[i]))
                if (area->AreaBit != -1 && !(player->GetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + area->AreaBit / 32) & (1 << (uint32(area->AreaBit) % 32))))
                    return false;
        }
    }

    if (condition->AuraSpellID[0])
    {
        using AuraCount = std::extent<decltype(condition->AuraSpellID)>;

        std::array<bool, AuraCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < AuraCount::value; ++i)
            if (condition->AuraSpellID[i])
            {
                if (condition->AuraStacks[i])
                    results[i] = unit->GetAuraCount(condition->AuraSpellID[i]) >= condition->AuraStacks[i];
                else
                    results[i] = unit->HasAura(condition->AuraSpellID[i]);
            }

        if (!PlayerConditionLogic(condition->AuraSpellLogic, results))
            return false;
    }

    // TODO: time condition
    // TODO (or not): world state expression condition
    // TODO: weather condition

    if (player && condition->Achievement[0])
    {
        using AchievementCount = std::extent<decltype(condition->Achievement)>;

        std::array<bool, AchievementCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < AchievementCount::value; ++i)
        {
            if (condition->Achievement[i])
            {
                // if (condition->Flags & 2) { any character on account completed it } else { current character only }
                // TODO: part of accountwide achievements
                // results[i] = player->HasAchieved(condition->Achievement[i]);
                results[i] = player->GetAchievementMgr()->HasAccountAchieved(condition->Achievement[i]); // On visual artifact activate if exist achiv on account
            }
        }

        if (!PlayerConditionLogic(condition->AchievementLogic, results))
            return false;
    }

    // TODO: research lfg status for player conditions

    if (condition->AreaID[0])
    {
        using AreaCount = std::extent<decltype(condition->AreaID)>;

        std::array<bool, AreaCount::value> results;
        results.fill(true);
        for (std::size_t i = 0; i < AreaCount::value; ++i)
            if (condition->AreaID[i])
                results[i] = unit->GetAreaId() == condition->AreaID[i] || unit->GetCurrentZoneID() == condition->AreaID[i];

        if (!PlayerConditionLogic(condition->AreaLogic, results))
            return false;
    }

    if (player && condition->MinExpansionLevel != -1 && player->GetSession()->Expansion() < condition->MinExpansionLevel)
        return false;

    if (player && condition->MaxExpansionLevel != -1 && player->GetSession()->Expansion() > condition->MaxExpansionLevel)
        return false;

    // if (player && condition->WorldStateExpressionID)
    // {
        // auto expressionEntry = sWorldStateExpressionStore.LookupEntry(condition->WorldStateExpressionID);
        // if (!expressionEntry || !expressionEntry->Eval(player))
            // return false;
    // }

    if (player && condition->MinExpansionLevel != -1 && condition->MinExpansionTier != -1 && !player->isGameMaster()
        && ((condition->MinExpansionLevel == CURRENT_EXPANSION) && condition->MinExpansionTier > 0) || condition->MinExpansionLevel > CURRENT_EXPANSION)
        return false;

    if (condition->PhaseID && !unit->HasPhaseId(condition->PhaseID))
        return false;

    if (player && condition->PhaseGroupID)
    {
        if (!player->InSamePhaseId(sDB2Manager.GetPhasesForGroup(condition->PhaseGroupID), false))
            return false;
    }

    if (player && condition->QuestKillID)
    {
        Quest const* quest = sQuestDataStore->GetQuestTemplate(condition->QuestKillID);
        if (quest && player->GetQuestStatus(condition->QuestKillID) != QUEST_STATUS_COMPLETE)
        {
            using QuestKillCount = std::extent<decltype(condition->QuestKillMonster)>;

            std::array<bool, QuestKillCount::value> results;
            results.fill(true);
            for (std::size_t i = 0; i < QuestKillCount::value; ++i)
            {
                if (condition->QuestKillMonster[i])
                {
                    auto objectiveItr = std::find_if(quest->GetObjectives().begin(), quest->GetObjectives().end(), [condition, i](QuestObjective const& objective) -> bool
                    {
                        return objective.Type == QUEST_OBJECTIVE_MONSTER && uint32(objective.ObjectID) == condition->QuestKillMonster[i];
                    });
                    if (objectiveItr != quest->GetObjectives().end())
                        results[i] = player->GetQuestObjectiveData(quest, objectiveItr->StorageIndex) >= objectiveItr->Amount;
                }
            }

            if (!PlayerConditionLogic(condition->QuestKillLogic, results))
                return false;
        }
    }

    if (player && (condition->MinAvgItemLevel && uint32(player->GetAverageItemLevelTotal(false)) < condition->MinAvgItemLevel))
        return false;

    if (player && (condition->MaxAvgItemLevel && uint32(player->GetAverageItemLevelTotal(false)) > condition->MaxAvgItemLevel))
        return false;

    if (player && (condition->MinAvgEquippedItemLevel && uint32(player->GetAverageItemLevelEquipped()) < condition->MinAvgEquippedItemLevel))
        return false;

    if (player && (condition->MaxAvgEquippedItemLevel && uint32(player->GetAverageItemLevelEquipped()) > condition->MaxAvgEquippedItemLevel))
        return false;

    if (player && condition->MovementFlags[0] && !(player->GetUnitMovementFlags() & condition->MovementFlags[0]))
        return false;

    if (player && condition->MovementFlags[1] && !(player->GetExtraUnitMovementFlags() & condition->MovementFlags[1]))
        return false;

    if (player && condition->WeaponSubclassMask)
    {
        Item* mainHand = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (!mainHand || !((1 << mainHand->GetTemplate()->GetSubClass()) & condition->WeaponSubclassMask))
            return false;
    }

    return true;
}

template <class T>
bool CompareValues(ComparisionType type,  T val1, T val2)
{
    switch (type)
    {
        case COMP_TYPE_EQ:
            return val1 == val2;
        case COMP_TYPE_HIGH:
            return val1 > val2;
        case COMP_TYPE_LOW:
            return val1 < val2;
        case COMP_TYPE_HIGH_EQ:
            return val1 >= val2;
        case COMP_TYPE_LOW_EQ:
            return val1 <= val2;
        default: // incorrect parameter
            ASSERT(false);
            return false;
    }
}
