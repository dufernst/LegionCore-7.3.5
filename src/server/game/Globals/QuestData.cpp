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

#include "QuestData.h"
#include "DatabaseEnv.h"
#include "GameEventMgr.h"
#include "ObjectMgr.h"
#include "DisableMgr.h"
#include "AreaTriggerData.h"
#include "PoolMgr.h"
#include "WorldStateMgr.h"
#include "Configuration/Config.h"

#define _TRINITY_CORE_CONFIG  "worldserver.conf"

QuestPOI::QuestPOI() : BlobIndex(0), ObjectiveIndex(0), QuestObjectiveID(0), QuestObjectID(0), MapID(0), WorldMapAreaID(0), Floor(0), Priority(0), Flags(0), WorldEffectID(0), PlayerConditionID(0), SpawnTrackingID(0), AlwaysAllowMergingBlobs{false} { }

QuestPOI::QuestPOI(int32 _BlobIndex, int32 _ObjectiveIndex, int32 _QuestObjectiveID, int32 _QuestObjectID, int32 _MapID, int32 _WorldMapAreaID, int32 _Foor, int32 _Priority, int32 _Flags, int32 _WorldEffectID, int32 _PlayerConditionID, int32 _SpawnTrackingID, bool _AlwaysAllowMergingBlobs):
    BlobIndex(_BlobIndex), ObjectiveIndex(_ObjectiveIndex), QuestObjectiveID(_QuestObjectiveID), QuestObjectID(_QuestObjectID), MapID(_MapID), WorldMapAreaID(_WorldMapAreaID), Floor(_Foor), Priority(_Priority), Flags(_Flags), WorldEffectID(_WorldEffectID), PlayerConditionID(_PlayerConditionID), SpawnTrackingID(_SpawnTrackingID), AlwaysAllowMergingBlobs(_AlwaysAllowMergingBlobs) { }

QuestDataStoreMgr::QuestDataStoreMgr()
{
    _maxQuestId = 0;
    needWait = false;
}

QuestDataStoreMgr::~QuestDataStoreMgr()
{
    for (QuestMap::iterator i = _questTemplates.begin(); i != _questTemplates.end(); ++i)
        delete i->second;
}

QuestDataStoreMgr* QuestDataStoreMgr::instance()
{
    static QuestDataStoreMgr instance;
    return &instance;
}

WorldQuestMap const* QuestDataStoreMgr::GetWorldQuestMap() const
{
    if (needWait)
        return nullptr;
    return &_worldQuest;
}

void QuestDataStoreMgr::LoadWorldQuestTemplates()
{
    _worldQuestAreaTaskStore.clear();
    _worldQuestTemplate.clear();
    for (uint8 i = 0; i < QUEST_INFO_MAX; ++i)
    {
        _worldQuestUpdate[i].clear();
        _worldQuestSet[i].clear();
    }

    //                                                  0               1       2      3         4              5              6                 7          8            9          10              11
    QueryResult result = WorldDatabase.Query("SELECT `QuestInfoID`, `ZoneID`, `Min`, `Max`, `CurrencyID`, `CurrencyID_A`, `CurrencyID_H`, `CurrencyMin`, `CurrencyMax`, `GoldMin`, `GoldMax`, `ItemCAList`,"
        //   12          13       14            15
        "`HasArmor`, `Chance`, `AllMax`, `ItemResourceList`, `BonusLevel`, `modTreeID`, `PrimaryID`, `Currency`, `CurrencyCount`, `ArmorList`, `MinItemLevel`, `IsPvP` FROM `world_quest_template` order by `QuestInfoID` ASC, `ZoneID` DESC");

    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint8 QuestInfoID = fields[i++].GetUInt8();
            WorldQuestTemplate wqt;
            wqt.QuestInfoID = QuestInfoID;
            wqt.ZoneID = fields[i++].GetUInt16();
            wqt.Min = fields[i++].GetUInt8();
            wqt.Max = fields[i++].GetUInt8();
            wqt.CurrencyID = fields[i++].GetUInt16();
            wqt.CurrencyID_A = fields[i++].GetUInt16();
            wqt.CurrencyID_H = fields[i++].GetUInt16();
            wqt.CurrencyMin = fields[i++].GetUInt32();
            wqt.CurrencyMax = fields[i++].GetUInt32();
            wqt.GoldMin = fields[i++].GetUInt32();
            wqt.GoldMax = fields[i++].GetUInt32();

            Tokenizer ItemCA(fields[i++].GetString(), ' ');
            for (auto& x : ItemCA)
                wqt.ItemCAList.emplace_back(atoi(x));

            wqt.HasArmor = fields[i++].GetBool();
            wqt.Chance = fields[i++].GetFloat();
            wqt.AllMax = fields[i++].GetUInt8();

            Tokenizer ItemRes(fields[i++].GetString(), ' ');
            for (uint16 j = 0; j < ItemRes.size();)
            {
                WorldQuestTemplateItem wqti;
                wqti.ItemID = atoi(ItemRes[j]);
                wqti.ItemCountMin = atoi(ItemRes[j + 1]);
                wqti.ItemCountMax = atoi(ItemRes[j + 2]);
                wqti.chance = atof(ItemRes[j + 3]);
                wqt.ItemResourceList.emplace_back(wqti);
                j += 4;
            }
            wqt.BonusLevel = fields[i++].GetUInt8();
            wqt.modTreeID = fields[i++].GetUInt8();
            wqt.PrimaryID = fields[i++].GetUInt8();
            wqt.Currency = fields[i++].GetUInt16();
            wqt.CurrencyCount = fields[i++].GetUInt32();

            Tokenizer tokenArmor(fields[i++].GetString(), ' ');
            for (auto& x : tokenArmor)
                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(atoi(x)))
                    wqt.ArmorList.emplace_back(proto);

            wqt.MinItemLevel = fields[i++].GetUInt16();
            wqt.IsPvP = fields[i++].GetBool();

            if (!wqt.CurrencyID_A)
                wqt.CurrencyID_A = wqt.CurrencyID;

            if (!wqt.CurrencyID_H)
                wqt.CurrencyID_H = wqt.CurrencyID;

            if (!wqt.CurrencyID)
                wqt.CurrencyID = std::max(wqt.CurrencyID_A, wqt.CurrencyID_H);

             _worldQuestTemplate[QuestInfoID].push_back(std::move(wqt));
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded %u world_quest_template data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded 0 world_quest_template data. DB table `world_quest_template` is empty.");

    //                                      0          1           2              3           4        5          6        7
    result = WorldDatabase.Query("SELECT `QuestID`, `Timer`, `VariableID`, `VariableID1`, `Value`, `Value1`, `EventID`, `AreaID` FROM `world_quest_update`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 QuestID = fields[i++].GetUInt32();
            Quest const* quest = GetQuestTemplate(QuestID);
            if (!quest || quest->QuestSortID < 1)
                continue;

            WorldQuestTemplate const* worldQuest = GetWorldQuestTemplate(quest);
            if (!worldQuest)
                continue;

            std::set<WorldQuestState> state;
            WorldQuestUpdate wqu;
            wqu.Timer = fields[i++].GetUInt32();
            wqu.VariableID = fields[i++].GetUInt32();
            wqu.VariableID1 = fields[i++].GetUInt32();
            wqu.Value = fields[i++].GetUInt32();
            wqu.Value1 = fields[i++].GetUInt32();
            wqu.EventID = fields[i++].GetUInt32();
            wqu.QuestID = quest->GetQuestId();
            wqu.quest = quest;
            wqu.worldQuest = worldQuest;

            state.insert(std::make_pair(wqu.VariableID, wqu.Value));
            if (wqu.VariableID1)
                state.insert(std::make_pair(wqu.VariableID1, wqu.Value1));

            Tokenizer AreaList(fields[i++].GetString(), ' ');
            if (!AreaList.empty())
                for (Tokenizer::const_iterator itr = AreaList.begin(); itr != AreaList.end(); ++itr)
                    wqu.AreaIDs.push_back(uint32(atoi(*itr)));

            _worldQuestUpdate[quest->QuestInfoID][QuestID].push_back(wqu);

            if (QuestV2CliTaskEntry const* questTask = sQuestV2CliTaskStore.LookupEntry(QuestID))
                if (questTask->WorldStateExpressionID)
                    if (WorldStateExpressionEntry const* worldStateExpEntry = sWorldStateExpressionStore.LookupEntry(questTask->WorldStateExpressionID))
                        worldStateExpEntry->Eval(nullptr, &state);

            for (std::set<WorldQuestState>::const_iterator itrState = state.begin(); itrState != state.end(); ++itrState)
                sWorldStateMgr.AddTemplate(itrState->first, WorldStatesData::Types::World, 0, 1 << WorldStatesData::Flags::InitialState, 0);

            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded %u world_quest_update data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded 0 world_quest_update data. DB table `world_quest_update` is empty.");

    //                                      0          1           2            3               4
    result = WorldDatabase.Query("SELECT `QuestID`, `ItemID`, `ItemCount`, `NotNeedSpell`, `NeedSpell` FROM `world_quest_item`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 QuestID = fields[i++].GetUInt32();
            Quest const* quest = GetQuestTemplate(QuestID);
            if (!quest)
                continue;

            WorldQuestRecipe& wqr = _worldQuestRecipes[QuestID];
            wqr.ItemID = fields[i++].GetUInt32();
            wqr.ItemCount = fields[i++].GetUInt32();
            wqr.NotNeedSpell = fields[i++].GetUInt32();
            wqr.NeedSpell = fields[i++].GetUInt32();

            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded %u world_quest_item data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded 0 world_quest_item data. DB table `world_quest_item` is empty.");

    //                                           0           1           2        3         4            5              6              7           8         9           10
    result = CharacterDatabase.Query("SELECT `QuestID`, `VariableID`, `Value`, `Timer`, `StartTime`, `ResetTime`, `CurrencyID`, `CurrencyCount`, `Gold`, `ItemList`, `RewardType` FROM `world_quest`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 QuestID = fields[i++].GetUInt32();
            Quest const* quest = GetQuestTemplate(QuestID);
            if (!quest)
                continue;
            WorldQuestTemplate const* worldQuest = GetWorldQuestTemplate(quest);
            if (!worldQuest)
                continue;

            if (quest->IsLegionInvasion())
                WorldLegionInvasionZoneID = quest->QuestSortID;

            WorldQuest& Wq = _worldQuest[quest->QuestSortID][QuestID];
            Wq.QuestID = QuestID;
            Wq.VariableID = fields[i++].GetUInt32();
            Wq.Value = fields[i++].GetUInt32();
            Wq.State.insert(std::make_pair(Wq.VariableID, Wq.Value));
            if (WorldQuestUpdate const* questUpdate = GetWorldQuestUpdate(QuestID, quest->QuestInfoID))
                if (questUpdate->VariableID1)
                    Wq.State.insert(std::make_pair(questUpdate->VariableID1, questUpdate->Value1));
            Wq.Timer = fields[i++].GetUInt32();
            Wq.StartTime = fields[i++].GetUInt32();
            Wq.ResetTime = fields[i++].GetUInt32();
            Wq.CurrencyID = fields[i++].GetUInt32();
            Wq.CurrencyCount = fields[i++].GetUInt32();
            Wq.Gold = fields[i++].GetUInt32();
            Wq.quest = quest;
            Wq.worldQuest = worldQuest;

            if (QuestV2CliTaskEntry const* questTask = sQuestV2CliTaskStore.LookupEntry(QuestID))
                if (questTask->WorldStateExpressionID)
                    if (WorldStateExpressionEntry const* worldStateExpEntry = sWorldStateExpressionStore.LookupEntry(questTask->WorldStateExpressionID))
                        worldStateExpEntry->Eval(nullptr, &Wq.State);

            for (std::set<WorldQuestState>::const_iterator itrState = Wq.State.begin(); itrState != Wq.State.end(); ++itrState)
                sWorldStateMgr.SetWorldState(itrState->first, 0, itrState->second);

            uint8 classId = 0;
            uint8 numIt = 0;
            Tokenizer ItemList(fields[i++].GetString(), ' ');
            for (Tokenizer::const_iterator itr = ItemList.begin(); itr != ItemList.end(); ++itr)
            {
                switch (numIt)
                {
                    case 0:
                        Wq.ItemList[classId].ItemIDH = uint32(atoi(*itr));
                        numIt++;
                        break;
                    case 1:
                        Wq.ItemList[classId].ItemIDA = uint32(atoi(*itr));
                        numIt++;
                        break;
                    case 2:
                        Wq.ItemList[classId].ItemCount = uint32(atoi(*itr));
                        classId++;
                        numIt = 0;
                        break;
                    default:
                        break;
                }
            }

            Wq.typeReward = WorldQuestTypeReward(fields[i++].GetUInt32());

            Wq.Recipe = GetRecipesForQuest(Wq.QuestID);

            if (!quest->IsEmissary())
                AddWorldQuestTask(quest);

            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded %u world_quest data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadWorldQuestTemplates() >> Loaded 0 world_quest data. DB table `world_quest` is empty.");

    std::set<uint32> const* itemListIDs = sDB2Manager.GetItemsByBonusTree(455); // For 137 item can be 367 and 455
    if (itemListIDs && !itemListIDs->empty())
    {
        TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "LoadWorldQuestTemplates() >> Loaded world_quest_update itemListIDs %u", itemListIDs->size());

        for (auto& itemID : *itemListIDs)
        {
            if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemID))
            {
                if (proto->GetClass() == ITEM_CLASS_GEM && proto->GetSubClass() == ITEM_SUBCLASS_GEM_ARTIFACT_RELIC)
                    _worldQuestRelic.push_back(proto);
                else
                    _worldQuestItem.push_back(proto);
            }
        }
    }
    itemListIDs = sDB2Manager.GetItemsByBonusTree(463);
    if (itemListIDs && !itemListIDs->empty())
    {
        TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "LoadWorldQuestTemplates() >> Loaded world_quest_update itemListIDs for dungeon %u", itemListIDs->size());

        for (auto& itemID : *itemListIDs)
        {
            if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemID))
            {
                if (proto->GetClass() != ITEM_CLASS_GEM)
                    _worldQuestItemDungeon.push_back(proto);
            }
        }
    }

    result = WorldDatabase.Query("SELECT * FROM `world_quest_faction_analogs`");

    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 aquest = fields[0].GetUInt32();
            uint32 hquest = fields[1].GetUInt32();

            (_worldQuestsFactionAnalogs[0])[aquest] = hquest;
            (_worldQuestsFactionAnalogs[1])[hquest] = aquest;
        } while (result->NextRow());

        TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "LoadWorldQuestTemplates() >> Loaded world_quest_faction_analogs %u", counter);
    }
    else
        TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "LoadWorldQuestTemplates() >> Loaded world_quest_faction_analogs %u", 0);


    for (uint8 QuestInfoID = QUEST_INFO_GROUP; QuestInfoID < QUEST_INFO_MAX; ++QuestInfoID)
        for (auto& v : _worldQuestUpdate[QuestInfoID])
            for (auto& vTest : v.second)
            {
                if (GetWorldQuestFactionAnalog(vTest.QuestID, false) != 0) // there we add only horde's quests and after adding these quest into list add analogs for alliance
                    continue;

                _worldQuestSet[vTest.quest->QuestInfoID][vTest.quest->IsEmissary() ? 0 : vTest.quest->QuestSortID].insert(&vTest);
            }

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "LoadWorldQuestTemplates() >> Loaded world_quest_update _worldQuestRelic %u _worldQuestItem %u", _worldQuestRelic.size(), _worldQuestItem.size());
    // for (uint8 i = 0; i < QUEST_INFO_MAX; ++i)
        // TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "LoadWorldQuestTemplates() >> Loaded world_quest_update Info %u _worldQuestUpdate %u _worldQuestSet %u", i, _worldQuestUpdate[i].size(), _worldQuestSet[i].size());
}

void QuestDataStoreMgr::LoadPointOfInterestLocales()
{
    uint32 oldMSTime = getMSTime();

    _pointOfInterestLocaleStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT entry, icon_name_loc1, icon_name_loc2, icon_name_loc3, icon_name_loc4, icon_name_loc5, icon_name_loc6, icon_name_loc7, icon_name_loc8, icon_name_loc9, icon_name_loc10 FROM locales_points_of_interest");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        PointOfInterestLocale& data = _pointOfInterestLocaleStore[entry];

        for (uint8 i = 1; i < TOTAL_LOCALES; ++i)
            sObjectMgr->AddLocaleString(fields[i].GetString(), LocaleConstant(i), data.IconName);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadPointOfInterestLocales() >> Loaded %lu points_of_interest locale strings in %u ms", (unsigned long)_pointOfInterestLocaleStore.size(), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestRelations()
{
    LoadGameobjectQuestRelations();
    LoadGameobjectInvolvedRelations();
    LoadCreatureQuestRelations();
    LoadAreaQuestRelations();
    LoadCreatureInvolvedRelations();
}

void QuestDataStoreMgr::LoadQuests()
{
    for (auto itr : _questTemplates)
        delete itr.second;

    _questTemplates.clear();
    _questAreaTaskStore.clear();
    mExclusiveQuestGroups.clear();
    _questVTemplates.clear();
    _maxQuestId = 0;

    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT "
        "ID, QuestType, QuestLevel, QuestPackageID, MinLevel, QuestSortID, QuestInfoID, SuggestedGroupNum, RewardNextQuest, RewardXPDifficulty, RewardXPMultiplier, RewardMoney, RewardMoneyDifficulty, "
        "RewardMoneyMultiplier, RewardBonusMoney, RewardDisplaySpell1, RewardDisplaySpell2, RewardDisplaySpell3, RewardSpell, RewardHonor, RewardKillHonor, `RewardArtifactXP`, `RewardArtifactXPMultiplier`, RewardArtifactCategoryID, "
        "StartItem, Flags, FlagsEx, RewardItem1, RewardAmount1, RewardItem2, RewardAmount2, RewardItem3, RewardAmount3, RewardItem4, RewardAmount4, "
        "ItemDrop1, ItemDropQuantity1, ItemDrop2, ItemDropQuantity2, ItemDrop3, ItemDropQuantity3, ItemDrop4, ItemDropQuantity4, "
        "RewardChoiceItemID1, RewardChoiceItemQuantity1, RewardChoiceItemDisplayID1, RewardChoiceItemID2, RewardChoiceItemQuantity2, RewardChoiceItemDisplayID2, RewardChoiceItemID3, RewardChoiceItemQuantity3, RewardChoiceItemDisplayID3, "
        "RewardChoiceItemID4, RewardChoiceItemQuantity4, RewardChoiceItemDisplayID4, RewardChoiceItemID5, RewardChoiceItemQuantity5, RewardChoiceItemDisplayID5, RewardChoiceItemID6, RewardChoiceItemQuantity6, RewardChoiceItemDisplayID6, "
        "POIContinent, POIx, POIy, POIPriority, RewardTitle, RewardArenaPoints, RewardSkillLineID, RewardNumSkillUps, PortraitGiver, PortraitTurnIn, "
        "RewardFactionID1, RewardFactionValue1, RewardFactionOverride1, FactionCapIn1, RewardFactionID2, RewardFactionValue2, RewardFactionOverride2, FactionCapIn2, RewardFactionID3, RewardFactionValue3, RewardFactionOverride3, FactionCapIn3, RewardFactionID4, RewardFactionValue4, RewardFactionOverride4, FactionCapIn4, RewardFactionID5, RewardFactionValue5, RewardFactionOverride5, FactionCapIn5, RewardFactionFlags, "
        "RewardCurrencyID1, RewardCurrencyQty1, RewardCurrencyID2, RewardCurrencyQty2, RewardCurrencyID3, RewardCurrencyQty3, RewardCurrencyID4, RewardCurrencyQty4, "
        "AcceptedSoundKitID, CompleteSoundKitID, AreaGroupID, TimeAllowed, AllowableRaces, QuestRewardID, Expansion, "
        "LogTitle, LogDescription, QuestDescription, AreaDescription, QuestCompletionLog, PortraitGiverText, PortraitGiverName, PortraitTurnInText, PortraitTurnInName, "
        "QuestMaxScalingLevel, "
        "StartScript, CompleteScript, VerifiedBuild"
        " FROM quest_template");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded 0 quests definitions. DB table `quest_template` is empty.");

        return;
    }

    // create multimap previous quest for each existed quest
    // some quests can have many previous maps set by NextQuestID in previous quest
    // for example set of race quests can lead to single not race specific quest
    do
    {
        Field* fields = result->Fetch();

        Quest* newQuest = new Quest(fields);
        if (_maxQuestId < newQuest->GetQuestId())
            _maxQuestId = newQuest->GetQuestId() + 1;
        if (_questVTemplates.size() <= _maxQuestId)
            _questVTemplates.resize(_maxQuestId + 1, nullptr);

        _questTemplates[newQuest->GetQuestId()] = newQuest;
        _questVTemplates[newQuest->GetQuestId()] = newQuest;
        if (newQuest->Type == QUEST_TYPE_TASK && newQuest->AreaGroupID > 0)
        {
            std::vector<uint32> areaGroupMembers = sDB2Manager.GetAreasForGroup(newQuest->AreaGroupID);
            for (uint32 areaId : areaGroupMembers)
            {
                if (!newQuest->IsWorld())
                    _questAreaTaskStore[areaId].insert(newQuest);
            }
        }
    } while (result->NextRow());

    // Load `quest_details` -  SMSG_QUESTGIVER_QUEST_DETAILS
    //                                   0   1       2       3       4       5            6            7            8
    result = WorldDatabase.Query("SELECT ID, Emote1, Emote2, Emote3, Emote4, EmoteDelay1, EmoteDelay2, EmoteDelay3, EmoteDelay4 FROM quest_details");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded 0 quest details. DB table `quest_details` is empty.");
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[0].GetUInt32();

            auto itr = _questTemplates.find(questId);
            if (itr != _questTemplates.end())
                itr->second->LoadQuestDetails(fields);
            else
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Table `quest_details` has data for quest %u but such quest does not exist", questId);
        } while (result->NextRow());
    }

    // Load `quest_request_items` - SMSG_QUESTGIVER_REQUEST_ITEMS
    //                                   0   1                2                  3                     4                       5
    result = WorldDatabase.Query("SELECT ID, EmoteOnComplete, EmoteOnIncomplete, EmoteOnCompleteDelay, EmoteOnIncompleteDelay, CompletionText FROM quest_request_items");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 quest request items. DB table `quest_request_items` is empty.");
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[0].GetUInt32();

            auto itr = _questTemplates.find(questId);
            if (itr != _questTemplates.end())
                itr->second->LoadQuestRequestItems(fields);
            else
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Table `quest_request_items` has data for quest %u but such quest does not exist", questId);
        } while (result->NextRow());
    }

    // Load `quest_offer_reward` - SMSG_QUESTGIVER_OFFER_REWARD
    //                                   0   1       2       3       4       5            6            7            8            9
    result = WorldDatabase.Query("SELECT ID, Emote1, Emote2, Emote3, Emote4, EmoteDelay1, EmoteDelay2, EmoteDelay3, EmoteDelay4, RewardText FROM quest_offer_reward");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded 0 quest reward emotes. DB table `quest_offer_reward` is empty.");
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[0].GetUInt32();

            auto itr = _questTemplates.find(questId);
            if (itr != _questTemplates.end())
                itr->second->LoadQuestOfferReward(fields);
            else
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >>Table `quest_offer_reward` has data for quest %u but such quest does not exist", questId);
        } while (result->NextRow());
    }

    // Load `quest_template_addon`
    //                                   0   1         2                 3              4            5            6               7                     8
    result = WorldDatabase.Query("SELECT ID, MaxLevel, AllowableClasses, SourceSpellID, PrevQuestID, NextQuestID, ExclusiveGroup, RewardMailTemplateID, RewardMailDelay, "
        //9               10                   11                     12                     13                   14                   15                 16                17
        "RequiredSkillID, RequiredSkillPoints, RequiredMinRepFaction, RequiredMaxRepFaction, RequiredMinRepValue, RequiredMaxRepValue, ProvidedItemCount, SpecialFlags, RewardMailTitle FROM quest_template_addon");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded 0 quest template addons. DB table `quest_template_addon` is empty.");
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[0].GetUInt32();

            auto itr = _questTemplates.find(questId);
            if (itr != _questTemplates.end())
                itr->second->LoadQuestTemplateAddon(fields);
            else
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Table `quest_template_addon` has data for quest %u but such quest does not exist", questId);
        } while (result->NextRow());
    }

    // Load `quest_objectives` order by descending storage index to reduce resizes
    //                                   0   1        2     3             4         5       6      7       8         9
    result = WorldDatabase.Query("SELECT ID, QuestID, Type, StorageIndex, ObjectID, Amount, Flags, Flags2, TaskStep, Description FROM quest_objectives ORDER BY StorageIndex ASC");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded 0 quest objectives. DB table `quest_objectives` is empty.");
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[1].GetUInt32();

            auto itr = _questTemplates.find(questId);
            if (itr != _questTemplates.end())
            {
                itr->second->LoadQuestObjective(fields);

                QuestObjective obj;
                obj.QuestID = questId;
                obj.ID = fields[0].GetUInt32();
                obj.Type = fields[2].GetUInt8();
                obj.StorageIndex = fields[3].GetInt8();
                obj.ObjectID = fields[4].GetInt32();
                obj.Amount = fields[5].GetInt32();
                obj.Flags = fields[6].GetUInt32();
                obj.Flags2 = fields[7].GetUInt32();
                obj.TaskStep = fields[8].GetFloat();
                obj.Description = fields[9].GetString();
                _questObjectiveByType[QuestObjectiveType(obj.Type)].push_back(obj);
            }
            else
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Table `quest_objectives` has objective for quest %u but such quest does not exist", questId);
        } while (result->NextRow());
    }

    // Load `quest_visual_effect` join table with quest_objectives because visual effects are based on objective ID (core stores objectives by their index in quest)
    //                                   0     1     2          3        4
    result = WorldDatabase.Query("SELECT v.ID, o.ID, o.QuestID, v.Index, v.VisualEffect FROM quest_visual_effect AS v LEFT JOIN quest_objectives AS o ON v.ID = o.ID ORDER BY v.Index DESC");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded 0 quest visual effects. DB table `quest_visual_effect` is empty.");
    }
    else
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 vID = fields[0].GetUInt32();
            uint32 oID = fields[1].GetUInt32();

            if (!vID)
            {
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Table `quest_visual_effect` has visual effect for null objective id");
                WorldDatabase.PExecute("DELETE FROM quest_visual_effect WHERE ID = %u", vID);
                continue;
            }

            // objID will be null if match for table join is not found
            if (vID != oID)
            {
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Table `quest_visual_effect` has visual effect for objective %u but such objective does not exist.", vID);
                WorldDatabase.PExecute("DELETE FROM quest_visual_effect WHERE ID = %u", vID);
                continue;
            }

            uint32 questId = fields[2].GetUInt32();

            // Do not throw error here because error for non existing quest is thrown while loading quest objectives. we do not need duplication
            auto itr = _questTemplates.find(questId);
            if (itr != _questTemplates.end())
                itr->second->LoadQuestObjectiveVisualEffect(fields);
        } while (result->NextRow());
    }

    auto SkillByQuestSort = [](int32 QuestSort) -> uint32
    {
        switch (QuestSort)
        {
        case QUEST_SORT_HERBALISM:      return SKILL_HERBALISM;
        case QUEST_SORT_FISHING:        return SKILL_FISHING;
        case QUEST_SORT_BLACKSMITHING:  return SKILL_BLACKSMITHING;
        case QUEST_SORT_ALCHEMY:        return SKILL_ALCHEMY;
        case QUEST_SORT_LEATHERWORKING: return SKILL_LEATHERWORKING;
        case QUEST_SORT_ENGINEERING:    return SKILL_ENGINEERING;
        case QUEST_SORT_TAILORING:      return SKILL_TAILORING;
        case QUEST_SORT_COOKING:        return SKILL_COOKING;
        case QUEST_SORT_FIRST_AID:      return SKILL_FIRST_AID;
        case QUEST_SORT_JEWELCRAFTING:  return SKILL_JEWELCRAFTING;
        case QUEST_SORT_INSCRIPTION:    return SKILL_INSCRIPTION;
        case QUEST_SORT_ARCHAEOLOGY:    return SKILL_ARCHAEOLOGY;
        default:
            return 0;
        }
    };

    std::map<uint32, uint32> usedMailTemplates;
    for (auto iter : _questTemplates)
    {
        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, iter.first))
            continue;

        Quest* qinfo = iter.second;

        if (qinfo->Type >= MAX_QUEST_TYPES)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `Method` = %u, expected values are 0, 1 or 2.", qinfo->GetQuestId(), qinfo->Type);

        if (qinfo->SpecialFlags & ~QUEST_SPECIAL_FLAGS_DB_ALLOWED)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `SpecialFlags` = %u > max allowed value. Correct `SpecialFlags` to value <= %u", qinfo->GetQuestId(), qinfo->SpecialFlags, QUEST_SPECIAL_FLAGS_DB_ALLOWED);
            qinfo->SpecialFlags &= QUEST_SPECIAL_FLAGS_DB_ALLOWED;
        }

        if (qinfo->IsDaily() && qinfo->IsWeekly())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Weekly Quest %u is marked as daily quest in `Flags`, removed daily flag.", qinfo->GetQuestId());
            qinfo->Flags &= ~QUEST_FLAGS_DAILY;
        }

        if (qinfo->IsDaily() && !qinfo->IsRepeatable())
        {
            //WorldDatabase.PExecute("UPDATE `quest_template_addon` SET `SpecialFlags` = SpecialFlags | 1 WHERE `ID` = %u", qinfo->GetQuestId());
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Daily Quest %u not marked as repeatable in `SpecialFlags`, added.", qinfo->GetQuestId());
            qinfo->SpecialFlags |= QUEST_SPECIAL_FLAGS_REPEATABLE;
        }

        if ((qinfo->Flags & QUEST_FLAGS_WEEKLY) && !qinfo->IsRepeatable())
        {
            //WorldDatabase.PExecute("UPDATE `quest_template_addon` SET `SpecialFlags` = SpecialFlags | 1 WHERE `ID` = %u", qinfo->GetQuestId());
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Weekly Quest %u not marked as repeatable in `SpecialFlags`, added.", qinfo->GetQuestId());
            qinfo->SpecialFlags |= QUEST_SPECIAL_FLAGS_REPEATABLE;
        }

        if (qinfo->Flags & QUEST_FLAGS_TRACKING)
            for (uint8 j = 1; j < QUEST_REWARD_CHOICES_COUNT; ++j)
                if (uint32 id = qinfo->RewardChoiceItemId[j])
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardChoiceItemId%d` = %u but item from `RewardChoiceItemId%d` can't be rewarded with quest flag QUEST_FLAGS_TRACKING.", qinfo->GetQuestId(), j + 1, id, j + 1);

        if (qinfo->MinLevel == uint32(-1) || qinfo->MinLevel > MAX_LEVEL)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u should be disabled because `MinLevel` = %i", qinfo->GetQuestId(), int32(qinfo->MinLevel));

        if (qinfo->QuestSortID > 0)
            if (!sAreaTableStore.LookupEntry(qinfo->QuestSortID))
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `QuestSortID` = %u (zone case) but zone with this id does not exist.",
                    qinfo->GetQuestId(), qinfo->QuestSortID);

        if (qinfo->QuestSortID < 0)
        {
            if (!sQuestSortStore.LookupEntry(-int32(qinfo->QuestSortID)))
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `QuestSortID` = %i (sort case) but quest sort with this id does not exist.", qinfo->GetQuestId(), qinfo->QuestSortID);

            if (uint32 skill_id = SkillByQuestSort(-int32(qinfo->QuestSortID)))
                if (qinfo->RequiredSkillId != skill_id)
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `QuestSortID` = %i but `RequiredSkillId` does not have a corresponding value (%d).", qinfo->GetQuestId(), qinfo->QuestSortID, skill_id);
        }

        if (qinfo->AllowableClasses)
        {
            if (!(qinfo->AllowableClasses & CLASSMASK_ALL_PLAYABLE))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u does not contain any playable classes in `AllowableClasses` (%u), value set to 0 (all classes).", qinfo->GetQuestId(), qinfo->AllowableClasses);
                qinfo->AllowableClasses = 0;
            }
        }

        if (qinfo->AllowableRaces != uint64(-1))
        {
            if (!(qinfo->AllowableRaces & RACEMASK_ALL_PLAYABLE))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u does not contain any playable races in `AllowableRaces` (%u), value set to 0 (all races).", qinfo->GetQuestId(), qinfo->AllowableRaces);
                qinfo->AllowableRaces = uint64(-1);
            }
        }

        if (qinfo->RequiredSkillId)
            if (!sSkillLineStore.LookupEntry(qinfo->RequiredSkillId))
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredSkillId` = %u but this skill does not exist", qinfo->GetQuestId(), qinfo->RequiredSkillId);

        if (qinfo->RequiredSkillPoints && (qinfo->RequiredSkillPoints > sWorld->GetConfigMaxSkillValue()))
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredSkillPoints` = %u but max possible skill is %u, quest can't be done.", qinfo->GetQuestId(), qinfo->RequiredSkillPoints, sWorld->GetConfigMaxSkillValue());

        if (qinfo->RequiredMinRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredMinRepFaction))
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredMinRepFaction` = %u but faction template %u does not exist, quest can't be done.",  qinfo->GetQuestId(), qinfo->RequiredMinRepFaction, qinfo->RequiredMinRepFaction);

        if (qinfo->RequiredMaxRepFaction && !sFactionStore.LookupEntry(qinfo->RequiredMaxRepFaction))
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredMaxRepFaction` = %u but faction template %u does not exist, quest can't be done.", qinfo->GetQuestId(), qinfo->RequiredMaxRepFaction, qinfo->RequiredMaxRepFaction);

        if (qinfo->RequiredMinRepValue && qinfo->RequiredMinRepValue > Reputation_Cap)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredMinRepValue` = %d but max reputation is %u, quest can't be done.",
                qinfo->GetQuestId(), qinfo->RequiredMinRepValue, Reputation_Cap);

        if (qinfo->RequiredMinRepValue && qinfo->RequiredMaxRepValue && qinfo->RequiredMaxRepValue <= qinfo->RequiredMinRepValue)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredMaxRepValue` = %d and `RequiredMinRepValue` = %d, quest can't be done.", qinfo->GetQuestId(), qinfo->RequiredMaxRepValue, qinfo->RequiredMinRepValue);

        if (!qinfo->RequiredMinRepFaction && qinfo->RequiredMinRepValue != 0)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredMinRepValue` = %d but `RequiredMinRepFaction` is 0, value has no effect", qinfo->GetQuestId(), qinfo->RequiredMinRepValue);

        if (!qinfo->RequiredMaxRepFaction && qinfo->RequiredMaxRepValue != 0)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RequiredMaxRepValue` = %d but `RequiredMaxRepFaction` is 0, value has no effect", qinfo->GetQuestId(), qinfo->RequiredMaxRepValue);

        if (qinfo->RewardTitleId && !sCharTitlesStore.LookupEntry(qinfo->RewardTitleId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardTitleId` = %u but CharTitle Id %u does not exist, quest can't be rewarded with title.", qinfo->GetQuestId(), qinfo->RewardTitleId, qinfo->RewardTitleId);
            qinfo->RewardTitleId = 0;
        }

        if (qinfo->SourceItemId)
        {
            if (!sObjectMgr->GetItemTemplate(qinfo->SourceItemId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `SourceItemId` = %u but item with entry %u does not exist, quest can't be done.", qinfo->GetQuestId(), qinfo->SourceItemId, qinfo->SourceItemId);
                qinfo->SourceItemId = 0;
                //WorldDatabase.PExecute("UPDATE `quest_template` SET `StartItem` = 0 WHERE `ID` = %u", qinfo->GetQuestId());
                //WorldDatabase.PExecute("UPDATE `quest_template_addon` SET `ProvidedItemCount` = 0 WHERE `ID` = %u", qinfo->GetQuestId());
            }
            else if (qinfo->SourceItemIdCount == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `SourceItemId` = %u but `SourceItemIdCount` = 0, set to 1 but need fix in DB.", qinfo->GetQuestId(), qinfo->SourceItemId);
                qinfo->SourceItemIdCount = 1; // update to 1 for allow quest work for backward compatibility with DB
                //WorldDatabase.PExecute("UPDATE `quest_template_addon` SET `ProvidedItemCount` = 1 WHERE `ID` = %u", qinfo->GetQuestId());
            }
        }
        else if (qinfo->SourceItemIdCount > 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `SourceItemId` = 0 but `SourceItemIdCount` = %u, useless value.", qinfo->GetQuestId(), qinfo->SourceItemIdCount);
            qinfo->SourceItemIdCount = 0;
            //WorldDatabase.PExecute("UPDATE `quest_template_addon` SET `ProvidedItemCount` = 0 WHERE `ID` = %u", qinfo->GetQuestId());
        }

        if (qinfo->SourceSpellID)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(qinfo->SourceSpellID);
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `SourceSpellID` = %u but spell %u doesn't exist, quest can't be done.", qinfo->GetQuestId(), qinfo->SourceSpellID, qinfo->SourceSpellID);
                qinfo->SourceSpellID = 0;
            }
            else if (!SpellMgr::IsSpellValid(spellInfo))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `SourceSpellID` = %u but spell %u is broken, quest can't be done.", qinfo->GetQuestId(), qinfo->SourceSpellID, qinfo->SourceSpellID);
                qinfo->SourceSpellID = 0;
            }
        }

        for (uint8 j = 0; j < QUEST_ITEM_COUNT; ++j)
        {
            if (uint32 id = qinfo->ItemDrop[j])
            {
                if (!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `ItemDrop%d` = %u but item with entry %u does not exist, quest can't be done. Set 0", qinfo->GetQuestId(), j + 1, id, id);
                    qinfo->ItemDrop[j] = 0;
                    //WorldDatabase.PExecute("UPDATE `quest_template` SET `ItemDrop1` = %u, `ItemDrop2` = %u, `ItemDrop3` = %u, `ItemDrop4` = %u WHERE `ID` = %u", qinfo->ItemDrop[0], qinfo->ItemDrop[1], qinfo->ItemDrop[2], qinfo->ItemDrop[3], qinfo->GetQuestId());
                }
            }
            else if (qinfo->ItemDropQuantity[j] > 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `ItemDrop%d` = 0 but `ItemDropQuantity%d` = %u. Set 0", qinfo->GetQuestId(), j + 1, j + 1, qinfo->ItemDropQuantity[j]);
                qinfo->ItemDropQuantity[j] = 0;
            }
        }

        for (QuestObjective const& obj : qinfo->GetObjectives())
        {
            if (obj.StorageIndex < 0)
            {
                switch (obj.Type)
                {
                    case QUEST_OBJECTIVE_MONSTER:
                    case QUEST_OBJECTIVE_ITEM:
                    case QUEST_OBJECTIVE_GAMEOBJECT:
                    case QUEST_OBJECTIVE_TALKTO:
                    case QUEST_OBJECTIVE_PLAYERKILLS:
                    case QUEST_OBJECTIVE_AREATRIGGER:
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has invalid StorageIndex = %d for objective type %u", qinfo->GetQuestId(), obj.ID, obj.StorageIndex, obj.Type);
                        break;
                    default:
                        break;
                }
            }

            switch (obj.Type)
            {
                case QUEST_OBJECTIVE_ITEM:
                    qinfo->SetSpecialFlag(QUEST_SPECIAL_FLAGS_DELIVER);
                    if (!sObjectMgr->GetItemTemplate(obj.ObjectID))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has non existing item entry %u, quest can't be done.", qinfo->GetQuestId(), obj.ID, obj.ObjectID);
                        //WorldDatabase.PExecute("delete from `quest_objectives` WHERE `ObjectId` = %u and `QuestID` = %u and type = 1", obj.ObjectID, qinfo->GetQuestId());
                    }
                    break;
                case QUEST_OBJECTIVE_MONSTER:
                    qinfo->SetSpecialFlag(QUEST_SPECIAL_FLAGS_KILL | QUEST_SPECIAL_FLAGS_CAST);
                    if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(obj.ObjectID))
                    {
                        if (cInfo->RequiredExpansion >= EXPANSION_WARLORDS_OF_DRAENOR)
                            const_cast<CreatureTemplate*>(cInfo)->QuestPersonalLoot = true;
                    }
                    else
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has non existing creature entry %u, quest can't be done.", qinfo->GetQuestId(), obj.ID, uint32(obj.ObjectID));
                    break;
                case QUEST_OBJECTIVE_GAMEOBJECT:
                    qinfo->SetSpecialFlag(QUEST_SPECIAL_FLAGS_KILL | QUEST_SPECIAL_FLAGS_CAST);
                    if (!sObjectMgr->GetGameObjectTemplate(obj.ObjectID))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has non existing gameobject entry %u, quest can't be done.", qinfo->GetQuestId(), obj.ID, uint32(obj.ObjectID));
                        //WorldDatabase.PExecute("delete from `quest_objectives` WHERE `ObjectId` = %u and `QuestID` = %u and type = 2", obj.ObjectID, qinfo->GetQuestId());
                    }
                    break;
                case QUEST_OBJECTIVE_TALKTO:
                    qinfo->SetSpecialFlag(QUEST_SPECIAL_FLAGS_CAST | QUEST_SPECIAL_FLAGS_SPEAKTO);
                    break;
                case QUEST_OBJECTIVE_MIN_REPUTATION:
                case QUEST_OBJECTIVE_MAX_REPUTATION:
                    if (!sFactionStore.LookupEntry(obj.ObjectID))
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has non existing faction id %u", qinfo->GetQuestId(), obj.ID, obj.ObjectID);
                    break;
                case QUEST_OBJECTIVE_PLAYERKILLS:
                    qinfo->SetSpecialFlag(QUEST_SPECIAL_FLAGS_PLAYER_KILL);
                    if (obj.Amount <= 0)
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has invalid player kills count %d", qinfo->GetQuestId(), obj.ID, obj.Amount);
                    break;
                case QUEST_OBJECTIVE_OBTAIN_CURRENCY:
                case QUEST_OBJECTIVE_HAVE_CURRENCY:
                case QUEST_OBJECTIVE_CURRENCY:
                    if (!sCurrencyTypesStore.LookupEntry(obj.ObjectID))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has non existing currency %u", qinfo->GetQuestId(), obj.ID, obj.ObjectID);
                        //WorldDatabase.PExecute("delete from `quest_objectives` WHERE `ObjectId` = %u and `QuestID` = %u and type = 4", obj.ObjectID, qinfo->GetQuestId());
                    }
                    if (obj.Amount <= 0)
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has invalid currency amount %d", qinfo->GetQuestId(), obj.ID, obj.Amount);
                    break;
                case QUEST_OBJECTIVE_LEARNSPELL:
                    if (!sSpellMgr->GetSpellInfo(obj.ObjectID))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has not exist SpellID: %u in ObjectID field ", qinfo->GetQuestId(), obj.ObjectID);
                        //WorldDatabase.PExecute("delete from `quest_objectives` WHERE `ObjectId` = %u and `QuestID` = %u and type = 5", obj.ObjectID, qinfo->GetQuestId());
                    }
                    break;
                case QUEST_OBJECTIVE_AREATRIGGER:
                    if (!qinfo->HasSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT))
                        const_cast<Quest*>(qinfo)->SetSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT);
                    if (sAreaTriggerStore.LookupEntry(uint32(obj.ObjectID)))
                        sAreaTriggerDataStore->AddDataToQuestAreatriggerStore(qinfo->Id, obj.ObjectID);
                    else if (obj.ObjectID != -1)
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has non existing areatrigger id %d", qinfo->GetQuestId(), obj.ID, obj.ObjectID);
                    break;
                case QUEST_OBJECTIVE_MONEY:
                case QUEST_OBJECTIVE_DEFEATBATTLEPET:
                case QUEST_OBJECTIVE_PET_BATTLE_VICTORIES:
                    break;
                case QUEST_OBJECTIVE_COMPLETE_CRITERIA_TREE:
                    if (!sCriteriaTreeStore.LookupEntry(obj.ObjectID))
                        TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has not exist CriteriaTreeID: %u in ObjectID field ", qinfo->GetQuestId(), obj.ObjectID);
                    break;
                case QUEST_OBJECTIVE_TASK_IN_ZONE:
                    qinfo->SpecialFlags |= QUEST_SPECIAL_FLAGS_AUTO_REWARD;
                    break;
                case QUEST_OBJECTIVE_PET_TRAINER_DEFEAT:
                    if (!sObjectMgr->GetCreatureTemplate(obj.ObjectID))
                        continue;
                    break;
                default:
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u objective %u has unhandled type %u", qinfo->GetQuestId(), obj.ID, obj.Type);
            }
        }

        for (uint8 j = 0; j < QUEST_REWARD_CHOICES_COUNT; ++j)
        {
            if (uint32 id = qinfo->RewardChoiceItemId[j])
            {
                if (!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardChoiceItemId%d` = %u but item with entry %u does not exist, quest will not reward this item.", qinfo->GetQuestId(), j + 1, id, id);
                    qinfo->RewardChoiceItemId[j] = 0;
                }

                if (!qinfo->RewardChoiceItemCount[j])
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardChoiceItemId%d` = %u but `RewardChoiceItemCount%d` = 0, quest can't be done.", qinfo->GetQuestId(), j + 1, id, j + 1);
            }
            else if (qinfo->RewardChoiceItemCount[j] > 0)
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardChoiceItemId%d` = 0 but `RewardChoiceItemCount%d` = %u.", qinfo->GetQuestId(), j + 1, j + 1, qinfo->RewardChoiceItemCount[j]);
        }

        for (uint8 j = 0; j < QUEST_ITEM_COUNT; ++j)
        {
            if (uint32 id = qinfo->RewardItemId[j])
            {
                if (!sObjectMgr->GetItemTemplate(id))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardItemId%d` = %u but item with entry %u does not exist, quest will not reward this item.", qinfo->GetQuestId(), j + 1, id, id);
                    qinfo->RewardItemId[j] = 0;
                    //WorldDatabase.PExecute("UPDATE `quest_template` SET `RewardItem1` = %u, `RewardItem2` = %u WHERE `ID` = %u", qinfo->RewardItemId[0], qinfo->RewardItemId[1], qinfo->GetQuestId());
                }

                if (!qinfo->RewardItemCount[j])
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardItemId%d` = %u but `RewardItemCount%d` = 0, quest will not reward this item.", qinfo->GetQuestId(), j + 1, id, j + 1);
            }
            else if (qinfo->RewardItemCount[j] > 0)
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardItemId%d` = 0 but `RewardItemCount%d` = %u.", qinfo->GetQuestId(), j + 1, j + 1, qinfo->RewardItemCount[j]);
        }

        for (uint8 j = 0; j < QUEST_REWARD_REPUTATIONS_COUNT; ++j)
        {
            if (qinfo->RewardFactionId[j])
            {
                if (abs(qinfo->RewardFactionValue[j]) > 10)
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has RewardFactionValue%d = %i. That is outside the range of valid values (-9 to 9).", qinfo->GetQuestId(), j + 1, qinfo->RewardFactionValue[j]);

                if (!sFactionStore.LookupEntry(qinfo->RewardFactionId[j]))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardFactionId%d` = %u but raw faction (faction.dbc) %u does not exist, quest will not reward reputation for this faction.", qinfo->GetQuestId(), j + 1, qinfo->RewardFactionId[j], qinfo->RewardFactionId[j]);
                    qinfo->RewardFactionId[j] = 0;
                }
            }

            else if (qinfo->RewardFactionOverride[j] != 0)
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardFactionId%d` = 0 but `RewardFactionOverride%d` = %i.", qinfo->GetQuestId(), j + 1, j + 1, qinfo->RewardFactionOverride[j]);
        }

        for (uint32 i = 0; i < QUEST_REWARD_DISPLAY_SPELL_COUNT; ++i)
        {
            if (qinfo->RewardDisplaySpell[i])
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(qinfo->RewardDisplaySpell[i]);
                if (!spellInfo)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardDisplaySpell%u` = %u but spell %u does not exist, spell removed as display reward.", qinfo->GetQuestId(), i, qinfo->RewardDisplaySpell[i], qinfo->RewardDisplaySpell[i]);
                    qinfo->RewardDisplaySpell[i] = 0;
                    //WorldDatabase.PExecute("UPDATE `quest_template` SET `RewardDisplaySpell1` = %u, `RewardDisplaySpell2` = %u, `RewardDisplaySpell3` = %u WHERE `id` = %u", qinfo->RewardDisplaySpell[0], qinfo->RewardDisplaySpell[1], qinfo->RewardDisplaySpell[2], qinfo->GetQuestId());
                }
                else if (!SpellMgr::IsSpellValid(spellInfo))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardDisplaySpell%u` = %u but spell %u is broken, quest will not have a spell reward.", qinfo->GetQuestId(), i, qinfo->RewardDisplaySpell[i], qinfo->RewardDisplaySpell[i]);
                    qinfo->RewardDisplaySpell[i] = 0;
                    //WorldDatabase.PExecute("UPDATE `quest_template` SET `RewardDisplaySpell1` = %u, `RewardDisplaySpell2` = %u, `RewardDisplaySpell3` = %u WHERE `id` = %u", qinfo->RewardDisplaySpell[0], qinfo->RewardDisplaySpell[1], qinfo->RewardDisplaySpell[2], qinfo->GetQuestId());
                }
            }
        }

        if (qinfo->RewardSpell > 0)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(qinfo->RewardSpell);
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardSpell` = %u but spell %u does not exist, quest will not have a spell reward.", qinfo->GetQuestId(), qinfo->RewardSpell, qinfo->RewardSpell);
                qinfo->RewardSpell = 0;
                //WorldDatabase.PExecute("UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `id` = %u", qinfo->GetQuestId());
            }
            else if (!SpellMgr::IsSpellValid(spellInfo))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardSpell` = %u but spell %u is broken, quest will not have a spell reward.", qinfo->GetQuestId(), qinfo->RewardSpell, qinfo->RewardSpell);
                qinfo->RewardSpell = 0;
                //WorldDatabase.PExecute("UPDATE `quest_template` SET `RewardSpell` = 0 WHERE `id` = %u", qinfo->GetQuestId());
            }
        }

        if (qinfo->RewardMailTemplateId)
        {
            if (!sMailTemplateStore.LookupEntry(qinfo->RewardMailTemplateId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardMailTemplateId` = %u but mail template  %u does not exist, quest will not have a mail reward.", qinfo->GetQuestId(), qinfo->RewardMailTemplateId, qinfo->RewardMailTemplateId);
                qinfo->RewardMailTemplateId = 0;
                qinfo->RewardMailDelay = 0;
            }
            else if (usedMailTemplates.find(qinfo->RewardMailTemplateId) != usedMailTemplates.end())
            {
                std::map<uint32, uint32>::const_iterator used_mt_itr = usedMailTemplates.find(qinfo->RewardMailTemplateId);
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardMailTemplateId` = %u but mail template  %u already used for quest %u, quest will not have a mail reward.", qinfo->GetQuestId(), qinfo->RewardMailTemplateId, qinfo->RewardMailTemplateId, used_mt_itr->second);
                qinfo->RewardMailTemplateId = 0;
                qinfo->RewardMailDelay = 0;
            }
            else
                usedMailTemplates[qinfo->RewardMailTemplateId] = qinfo->GetQuestId();
        }

        if (qinfo->NextQuestIdChain)
        {
            QuestMap::iterator qNextItr = _questTemplates.find(qinfo->NextQuestIdChain);
            if (qNextItr == _questTemplates.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `NextQuestIdChain` = %u but quest %u does not exist, quest chain will not work.", qinfo->GetQuestId(), qinfo->NextQuestIdChain, qinfo->NextQuestIdChain);
                qinfo->NextQuestIdChain = 0;
                //WorldDatabase.PExecute("UPDATE `quest_template` SET `RewardNextQuest` = 0 WHERE `ID` = %u", qinfo->GetQuestId());
            }
            else
                qNextItr->second->prevChainQuests.push_back(qinfo->GetQuestId());
        }

        for (uint8 j = 0; j < QUEST_REWARD_CURRENCY_COUNT; ++j)
        {
            if (qinfo->RewardCurrencyId[j])
            {
                if (qinfo->RewardCurrencyCount[j] == 0)
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardCurrencyId%d` = %u but `RewardCurrencyCount%d` = 0, quest can't be done.", qinfo->GetQuestId(), j + 1, qinfo->RewardCurrencyId[j], j + 1);

                if (!sCurrencyTypesStore.LookupEntry(qinfo->RewardCurrencyId[j]))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardCurrencyId%d` = %u but currency with entry %u does not exist, quest can't be done.", qinfo->GetQuestId(), j + 1, qinfo->RewardCurrencyId[j], qinfo->RewardCurrencyId[j]);
                    qinfo->RewardCurrencyCount[j] = 0;
                }
            }
            else if (qinfo->RewardCurrencyCount[j] > 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardCurrencyId%d` = 0 but `RewardCurrencyCount%d` = %u, quest can't be done.", qinfo->GetQuestId(), j + 1, j + 1, qinfo->RewardCurrencyCount[j]);
                qinfo->RewardCurrencyCount[j] = 0;
            }
        }

        if (qinfo->RewardSkillId)
        {
            if (!sSkillLineStore.LookupEntry(qinfo->RewardSkillId))
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardSkillId` = %u but this skill does not exist", qinfo->GetQuestId(), qinfo->RewardSkillId);

            if (!qinfo->RewardSkillPoints)
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardSkillId` = %u but `RewardSkillPoints` is 0", qinfo->GetQuestId(), qinfo->RewardSkillId);
        }

        if (qinfo->RewardSkillPoints)
        {
            if (qinfo->RewardSkillPoints > sWorld->GetConfigMaxSkillValue())
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardSkillPoints` = %u but max possible skill is %u, quest can't be done.", qinfo->GetQuestId(), qinfo->RewardSkillPoints, sWorld->GetConfigMaxSkillValue());

            if (!qinfo->RewardSkillId)
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %u has `RewardSkillPoints` = %u but `RewardSkillId` is 0", qinfo->GetQuestId(), qinfo->RewardSkillPoints);
        }

        if (qinfo->PrevQuestID)
        {
            if (_questTemplates.find(abs(qinfo->PrevQuestID)) == _questTemplates.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %d has PrevQuestID %i, but no such quest", qinfo->GetQuestId(), qinfo->PrevQuestID);
                //WorldDatabase.PExecute("update `quest_template_addon` set PrevQuestID = 0 WHERE `ID` = %u", qinfo->GetQuestId());
            }
            else
                qinfo->prevQuests.push_back(qinfo->PrevQuestID);
        }

        if (qinfo->NextQuestID)
        {
            QuestMap::iterator qNextItr = _questTemplates.find(abs(qinfo->NextQuestID));
            if (qNextItr == _questTemplates.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Quest %d has NextQuestID %i, but no such quest", qinfo->GetQuestId(), qinfo->NextQuestID);
                //WorldDatabase.PExecute("update `quest_template_addon` set NextQuestID = 0 WHERE `ID` = %u", qinfo->GetQuestId());
            }
            else
                qNextItr->second->prevQuests.push_back(qinfo->NextQuestID < 0 ? -int32(qinfo->GetQuestId()) : int32(qinfo->GetQuestId()));
        }

        if (qinfo->ExclusiveGroup)
            mExclusiveQuestGroups.insert(std::make_pair(qinfo->ExclusiveGroup, qinfo->GetQuestId()));

        if (qinfo->LimitTime)
            qinfo->SetSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED);
    }

    for (uint32 i = 0; i < sSpellMgr->GetSpellInfoStoreSize(); ++i)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(i);
        if (!spellInfo)
            continue;

        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->Effects[j]->Effect != SPELL_EFFECT_QUEST_COMPLETE)
                continue;

            uint32 questID = spellInfo->Effects[j]->MiscValue;
            Quest const* quest = GetQuestTemplate(questID);
            if (!quest)
                continue;

            if (!quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuests() >> Spell (id: %u) have SPELL_EFFECT_QUEST_COMPLETE for quest %u, but quest not have flag QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT. Quest flags must be fixed, quest modified to enable objective.", spellInfo->Id, questID);
                const_cast<Quest*>(quest)->SetSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT);
                //WorldDatabase.PExecute("UPDATE `quest_template_addon` SET `SpecialFlags` = SpecialFlags | 2 WHERE `ID` = %u", quest->GetQuestId());
            }
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuests() >> Loaded %lu quests definitions in %u ms", (unsigned long)_questTemplates.size(), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestTemplateLocale()
{
    uint32 oldMSTime = getMSTime();

    _questTemplateLocaleStore.clear();
    //                                               0     1
    QueryResult result = WorldDatabase.Query("SELECT Id, locale, "
        //      2           3                 4                5                 6                  7                   8                   9                  10
        "LogTitle, LogDescription, QuestDescription, AreaDescription, PortraitGiverText, PortraitGiverName, PortraitTurnInText, PortraitTurnInName, QuestCompletionLog"
        " FROM quest_template_locale");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant locale = GetLocaleByName(fields[1].GetString());
        if (locale == LOCALE_none)
            continue;

        QuestTemplateLocale& data = _questTemplateLocaleStore[fields[0].GetUInt32()];
        sObjectMgr->AddLocaleString(fields[2].GetString(), locale, data.LogTitle);
        sObjectMgr->AddLocaleString(fields[3].GetString(), locale, data.LogDescription);
        sObjectMgr->AddLocaleString(fields[4].GetString(), locale, data.QuestDescription);
        sObjectMgr->AddLocaleString(fields[5].GetString(), locale, data.AreaDescription);
        sObjectMgr->AddLocaleString(fields[6].GetString(), locale, data.PortraitGiverText);
        sObjectMgr->AddLocaleString(fields[7].GetString(), locale, data.PortraitGiverName);
        sObjectMgr->AddLocaleString(fields[8].GetString(), locale, data.PortraitTurnInText);
        sObjectMgr->AddLocaleString(fields[9].GetString(), locale, data.PortraitTurnInName);
        sObjectMgr->AddLocaleString(fields[10].GetString(), locale, data.QuestCompletionLog);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuestTemplateLocale() Loaded %u Quest Tempalate locale strings in %u ms", uint32(_questTemplateLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestRequestItemsLocale()
{
    uint32 oldMSTime = getMSTime();

    _questRequestItemsLocaleStore.clear();
    //                                               0     1
    QueryResult result = WorldDatabase.Query("SELECT ID, Locale, CompletionText FROM quest_request_items_locale");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant localeConstant = GetLocaleByName(fields[1].GetString());
        if (localeConstant == LOCALE_none)
            continue;

        QuestRequestItemsLocale& data = _questRequestItemsLocaleStore[fields[0].GetUInt32()];
        sObjectMgr->AddLocaleString(fields[2].GetString(), localeConstant, data.RequestItemsText);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuestRequestItemsLocale() >> Loaded %u quest request items locale strings in %u ms", uint32(_questRequestItemsLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestOfferRewardLocale()
{
    uint32 oldMSTime = getMSTime();

    _questOfferRewardLocaleStore.clear();
    //                                               0     1
    QueryResult result = WorldDatabase.Query("SELECT ID, Locale, OfferRewardText FROM quest_offer_reward_locale");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant localeConstant = GetLocaleByName(fields[1].GetString());
        if (localeConstant == LOCALE_none)
            continue;

        QuestOfferRewardLocale& data = _questOfferRewardLocaleStore[fields[0].GetUInt32()];
        sObjectMgr->AddLocaleString(fields[2].GetString(), localeConstant, data.OfferRewardText);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuestOfferRewardLocale() >> Loaded %u quest offer reward locale strings in %u ms", uint32(_questOfferRewardLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestObjectivesLocale()
{
    uint32 oldMSTime = getMSTime();

    _questObjectivesLocaleStore.clear();
                                         //                                               0     1          2
    QueryResult result = WorldDatabase.Query("SELECT Id, locale, Description FROM quest_objectives_locale");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();

        LocaleConstant localeConstant = GetLocaleByName(fields[1].GetString());
        if (localeConstant == LOCALE_none)
            continue;

        QuestObjectivesLocale& data = _questObjectivesLocaleStore[fields[0].GetUInt32()];
        sObjectMgr->AddLocaleString(fields[2].GetString(), localeConstant, data.Description);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuestObjectivesLocale() >> Loaded %u Quest Objectives locale strings in %u ms", uint32(_questObjectivesLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadPointsOfInterest()
{
    uint32 oldMSTime = getMSTime();

    _pointsOfInterestStore.clear();

    uint32 count = 0;

    //                                                  0   1  2   3      4     5       6
    QueryResult result = WorldDatabase.Query("SELECT entry, x, y, icon, flags, data, icon_name FROM points_of_interest");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadPointsOfInterest() >> Loaded 0 Points of Interest definitions. DB table `points_of_interest` is empty.");

        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 point_id = fields[0].GetUInt32();

        PointOfInterest POI;
        POI.x = fields[1].GetFloat();
        POI.y = fields[2].GetFloat();
        POI.icon = fields[3].GetUInt32();
        POI.flags = fields[4].GetUInt32();
        POI.data = fields[5].GetUInt32();
        POI.icon_name = fields[6].GetString();
        POI.entry = 0;

        if (!Trinity::IsValidMapCoord(POI.x, POI.y))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadPointsOfInterest() >> Table `points_of_interest` (Entry: %u) have invalid coordinates (X: %f Y: %f), ignored.", point_id, POI.x, POI.y);
            continue;
        }

        _pointsOfInterestStore[point_id] = POI;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadPointsOfInterest() >> Loaded %u Points of Interest definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestPOI()
{
    std::string configFile = _TRINITY_CORE_CONFIG;

    uint32 oldMSTime = getMSTime();

    _questPOIStore.clear();

    uint32 count = 0;

    //                                                   0        1        2            3           4                 5           6         7            8       9       10         11              12             13               14
    QueryResult result = WorldDatabase.Query("SELECT QuestID, BlobIndex, Idx1, ObjectiveIndex, QuestObjectiveID, QuestObjectID, MapID, WorldMapAreaId, Floor, Priority, Flags, WorldEffectID, PlayerConditionID, WoDUnk1, AlwaysAllowMergingBlobs FROM quest_poi order by QuestID, Idx1");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuestPOI() >> Loaded 0 quest POI definitions. DB table `quest_poi` is empty.");

        return;
    }

    //                                                0        1    2  3
    QueryResult points = WorldDatabase.Query("SELECT QuestID, Idx1, X, Y FROM quest_poi_points ORDER BY QuestID DESC, Idx1, Idx2");

    std::vector<std::vector<std::vector<QuestPOIPoint>>> POIs;

    if (points)
    {
        // The first result should have the highest questId
        Field* fields = points->Fetch();
        int32 questIdMax = fields[0].GetInt32();
        POIs.resize(questIdMax + 1);

        do
        {
            fields = points->Fetch();

            int32 QuestID = fields[0].GetInt32();
            int32 Idx1 = fields[1].GetInt32();
            int32 X = fields[2].GetInt32();
            int32 Y = fields[3].GetInt32();

            if (questIdMax <= QuestID)
                continue;

            if (int32(POIs[QuestID].size()) <= Idx1 + 1)
                POIs[QuestID].resize(Idx1 + 10);

            QuestPOIPoint point(X, Y);
            POIs[QuestID][Idx1].push_back(point);
        } while (points->NextRow());
    }

    do
    {
        Field* fields = result->Fetch();

        int32 QuestID = fields[0].GetInt32();
        int32 BlobIndex = fields[1].GetInt32();
        int32 Idx1 = fields[2].GetInt32();
        int32 ObjectiveIndex = fields[3].GetInt32();
        int32 QuestObjectiveID = fields[4].GetInt32();
        int32 QuestObjectID = fields[5].GetInt32();
        int32 MapID = fields[6].GetInt32();
        int32 WorldMapAreaId = fields[7].GetInt32();
        int32 Floor = fields[8].GetInt32();
        int32 Priority = fields[9].GetInt32();
        int32 Flags = fields[10].GetInt32();
        int32 WorldEffectID = fields[11].GetInt32();
        int32 PlayerConditionID = fields[12].GetInt32();
        int32 SpawnTrackingID = fields[13].GetInt32();
        bool AlwaysAllowMergingBlobs = fields[14].GetBool();

        if (QuestID >= POIs.size())
            continue;

        QuestPOI POI(BlobIndex, ObjectiveIndex, QuestObjectiveID, QuestObjectID, MapID, WorldMapAreaId, Floor, Priority, Flags, WorldEffectID, PlayerConditionID, SpawnTrackingID, AlwaysAllowMergingBlobs);
        if (QuestID < int32(POIs.size()) && Idx1 < int32(POIs[QuestID].size()))
        {
            POI.points = POIs[QuestID][Idx1];
            _questPOIStore[QuestID].push_back(POI);
        }
        else
        {
            if (!sConfigMgr->GetIntDefault("QuestPOI.DisableErrors", 0))
                TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuestPOI() >> Table quest_poi references unknown quest points for quest %i POI id %i", QuestID, BlobIndex);
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuestPOI() >> Loaded %u quest POI definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadQuestRelationsHelper(QuestRelations& map, QuestStarter& _map, std::string const& table, bool starter, bool go)
{
    uint32 oldMSTime = getMSTime();

    map.clear();

    if (starter)
        _map.clear();

    uint32 count = 0;

    QueryResult result = WorldDatabase.PQuery("SELECT id, quest, pool_entry FROM %s qr LEFT JOIN pool_quest pq ON qr.quest = pq.entry", table.c_str());

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "LoadQuestRelationsHelper >> Loaded 0 quest relations from `%s`, table is empty.", table.c_str());

        return;
    }

    PooledQuestRelation* poolRelationMap = go ? &sPoolMgr->mQuestGORelation : &sPoolMgr->mQuestCreatureRelation;
    if (starter)
        poolRelationMap->clear();

    do
    {
        uint32 id = result->Fetch()[0].GetUInt32();
        uint32 quest = result->Fetch()[1].GetUInt32();
        uint32 poolId = result->Fetch()[2].GetUInt32();

        if (_questTemplates.find(quest) == _questTemplates.end())
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadQuestRelationsHelper >> Table `%s`: Quest %u listed for entry %u does not exist.", table.c_str(), quest, id);
            //WorldDatabase.PExecute("DELETE FROM creature_questender WHERE quest = %u", quest);
            //WorldDatabase.PExecute("DELETE FROM creature_queststarter WHERE quest = %u", quest);
            continue;
        }

        if (!poolId || !starter)
            map.insert(std::make_pair(id, quest));
        else if (starter)
            poolRelationMap->insert(std::make_pair(quest, id));

        if (starter)
            _map[quest].insert(id);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadQuestRelationsHelper >> Loaded %u quest relations from %s in %u ms", count, table.c_str(), GetMSTimeDiffToNow(oldMSTime));
}

void QuestDataStoreMgr::LoadGameobjectQuestRelations()
{
    LoadQuestRelationsHelper(_goQuestRelations, _goQuestStarter, "gameobject_queststarter", true, true);

    for (QuestRelations::iterator itr = _goQuestRelations.begin(); itr != _goQuestRelations.end(); ++itr)
    {
        GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(itr->first);
        if (!goInfo)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadGameobjectQuestRelations() >> Table `gameobject_queststarter` have data for not existed gameobject entry (%u) and existed quest %u", itr->first, itr->second);
        else if (goInfo->type != GAMEOBJECT_TYPE_QUESTGIVER)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadGameobjectQuestRelations() >> Table `gameobject_queststarter` have data gameobject entry (%u) for quest %u, but GO is not GAMEOBJECT_TYPE_QUESTGIVER", itr->first, itr->second);
            //WorldDatabase.PExecute("DELETE FROM gameobject_queststarter WHERE id = %u and quest = %u", itr->first, itr->second);
        }
    }
}

void QuestDataStoreMgr::LoadGameobjectInvolvedRelations()
{
    LoadQuestRelationsHelper(_goQuestInvolvedRelations, _goQuestStarter, "gameobject_questender", false, true);

    for (QuestRelations::iterator itr = _goQuestInvolvedRelations.begin(); itr != _goQuestInvolvedRelations.end(); ++itr)
    {
        GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(itr->first);
        if (!goInfo)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadGameobjectInvolvedRelations() >> Table `gameobject_questender` have data for not existed gameobject entry (%u) and existed quest %u", itr->first, itr->second);
        else if (goInfo->type != GAMEOBJECT_TYPE_QUESTGIVER)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadGameobjectInvolvedRelations() >>  Table `gameobject_questender` have data gameobject entry (%u) for quest %u, but GO is not GAMEOBJECT_TYPE_QUESTGIVER", itr->first, itr->second);
            //WorldDatabase.PExecute("DELETE FROM gameobject_questender WHERE id = %u and quest = %u", itr->first, itr->second);
        }

        _goQuestInvolvedRelationsByQuest.insert({ itr->second, itr->first });
    }
}

void QuestDataStoreMgr::LoadCreatureQuestRelations()
{
    LoadQuestRelationsHelper(_creatureQuestRelations, _creatureQuestStarter, "creature_queststarter", true, false);

    for (QuestRelations::iterator itr = _creatureQuestRelations.begin(); itr != _creatureQuestRelations.end(); ++itr)
    {
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(itr->first);
        if (!cInfo)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadCreatureQuestRelations() >> Table `creature_queststarter` have data for not existed creature entry (%u) and existed quest %u", itr->first, itr->second);
        //else if (!(cInfo->npcflag & UNIT_NPC_FLAG_QUESTGIVER))
        //    TC_LOG_ERROR(LOG_FILTER_SQL, "LoadCreatureQuestRelations() >> Table `creature_queststarter` has creature entry (%u) for quest %u, but npcflag does not include UNIT_NPC_FLAG_QUESTGIVER", itr->first, itr->second);
    }
}

void QuestDataStoreMgr::LoadAreaQuestRelations()
{
    LoadQuestRelationsHelper(_areaQuestRelations, _areaQuestStarter, "area_queststart", false, false);

    for (QuestRelations::iterator itr = _areaQuestRelations.begin(); itr != _areaQuestRelations.end(); ++itr)
        if (!sAreaTableStore.LookupEntry(itr->first))
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadAreaQuestRelations() >> Table `area_questrelation` have data for not existed area entry (%u) and existed quest %u", itr->first, itr->second);
}

void QuestDataStoreMgr::LoadCreatureInvolvedRelations()
{
    LoadQuestRelationsHelper(_creatureQuestInvolvedRelations, _creatureQuestStarter, "creature_questender", false, false);

    for (QuestRelations::iterator itr = _creatureQuestInvolvedRelations.begin(); itr != _creatureQuestInvolvedRelations.end(); ++itr)
    {
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(itr->first);
        if (!cInfo)
            TC_LOG_ERROR(LOG_FILTER_SQL, "LoadCreatureInvolvedRelations() >> Table `creature_questender` have data for not existed creature entry (%u) and existed quest %u", itr->first, itr->second);
        //else if (!(cInfo->npcflag & UNIT_NPC_FLAG_QUESTGIVER))
        //    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `creature_questender` has creature entry (%u) for quest %u, but npcflag does not include UNIT_NPC_FLAG_QUESTGIVER", itr->first, itr->second);
    
        _creatureQuestInvolvedRelationsByQuest.insert({ itr->second, itr->first });
    }
}

void QuestDataStoreMgr::LoadGameObjectForQuests()
{
    uint32 oldMSTime = getMSTime();

    _gameObjectForQuestStore.clear();

    if (sObjectMgr->GetGameObjectTemplates()->empty())
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadGameObjectForQuests() >> Loaded 0 GameObjects for quests");
        return;
    }

    uint32 count = 0;

    // collect GO entries for GO that must activated
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplates();
    for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
    {
        switch (itr->second.type)
        {
            // scan GO chest with loot including quest items
            case GAMEOBJECT_TYPE_CHEST:
            {
                uint32 loot_id = (itr->second.entry);

                // find quest loot for GO
                if (itr->second.chest.questID || LootTemplates_Gameobject.HaveQuestLootFor(loot_id))
                {
                    _gameObjectForQuestStore.insert(itr->second.entry);
                    ++count;
                }
                break;
            }
            case GAMEOBJECT_TYPE_GENERIC:
            {
                if (itr->second.generic.questID > 0)            //quests objects
                {
                    _gameObjectForQuestStore.insert(itr->second.entry);
                    count++;
                }
                break;
            }
            case GAMEOBJECT_TYPE_GOOBER:
            {
                if (itr->second.goober.questID > 0)              //quests objects
                {
                    _gameObjectForQuestStore.insert(itr->second.entry);
                    count++;
                }
                break;
            }
            case GAMEOBJECT_TYPE_SPELL_FOCUS:
            {
                //
                if (itr->second.spellFocus.questID > 0)              //quests objects
                {
                    _gameObjectForQuestStore.insert(itr->second.entry);
                    count++;
                }
                break;
            }
            default:
                break;
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadGameObjectForQuests() >> Loaded %u GameObjects for quests in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

Quest const* QuestDataStoreMgr::GetQuestTemplate(uint32 quest_id) const
{
    if (quest_id > _maxQuestId)
        return nullptr;
    return _questVTemplates[quest_id];
}

uint32 QuestDataStoreMgr::GetMaxQuestID()
{
    return _maxQuestId;
}

QuestMap const& QuestDataStoreMgr::GetQuestTemplates() const
{
    return _questTemplates;
}

bool QuestDataStoreMgr::IsGameObjectForQuests(uint32 entry) const
{
    return _gameObjectForQuestStore.find(entry) != _gameObjectForQuestStore.end();
}

QuestPOIVector const* QuestDataStoreMgr::GetQuestPOIVector(int32 QuestID)
{
    return Trinity::Containers::MapGetValuePtr(_questPOIStore, QuestID);
}

std::vector<QuestObjective> QuestDataStoreMgr::GetQuestObjectivesByType(QuestObjectiveType type)
{
    return _questObjectiveByType[type];
}

QuestRelations* QuestDataStoreMgr::GetGOQuestRelationMap()
{
    return &_goQuestRelations;
}

QuestRelationBounds QuestDataStoreMgr::GetGOQuestRelationBounds(uint32 go_entry)
{
    return _goQuestRelations.equal_range(go_entry);
}

QuestRelationBounds QuestDataStoreMgr::GetGOQuestInvolvedRelationBoundsByQuest(uint32 questId)
{
    return _goQuestInvolvedRelationsByQuest.equal_range(questId);
}

QuestRelationBounds QuestDataStoreMgr::GetGOQuestInvolvedRelationBounds(uint32 go_entry)
{
    return _goQuestInvolvedRelations.equal_range(go_entry);
}

QuestRelations* QuestDataStoreMgr::GetCreatureQuestRelationMap()
{
    return &_creatureQuestRelations;
}

QuestRelationBounds QuestDataStoreMgr::GetCreatureQuestRelationBounds(uint32 creature_entry)
{
    return _creatureQuestRelations.equal_range(creature_entry);
}

QuestRelationBounds QuestDataStoreMgr::GetCreatureQuestInvolvedRelationBoundsByQuest(uint32 questId)
{
    return _creatureQuestInvolvedRelationsByQuest.equal_range(questId);
}

QuestRelationBounds QuestDataStoreMgr::GetCreatureQuestInvolvedRelationBounds(uint32 creature_entry)
{
    return _creatureQuestInvolvedRelations.equal_range(creature_entry);
}

QuestRelationBounds QuestDataStoreMgr::GetAreaQuestRelationBounds(uint32 area)
{
    return _areaQuestRelations.equal_range(area);
}

QuestTemplateLocale const* QuestDataStoreMgr::GetQuestLocale(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_questTemplateLocaleStore, entry);
}

QuestRequestItemsLocale const* QuestDataStoreMgr::GetQuestRequestItemsLocale(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_questRequestItemsLocaleStore, entry);
}

QuestOfferRewardLocale const* QuestDataStoreMgr::GetQuestOfferRewardLocale(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_questOfferRewardLocaleStore, entry);
}

QuestObjectivesLocale const* QuestDataStoreMgr::GetQuestObjectivesLocale(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_questObjectivesLocaleStore, entry);
}

std::set<Quest const*> const* QuestDataStoreMgr::GetQuestTask(uint32 areaId) const
{
    return Trinity::Containers::MapGetValuePtr(_questAreaTaskStore, areaId);
}

PointOfInterest const* QuestDataStoreMgr::GetPointOfInterest(uint32 id) const
{
    return Trinity::Containers::MapGetValuePtr(_pointsOfInterestStore, id);
}

PointOfInterestLocale const* QuestDataStoreMgr::GetPointOfInterestLocale(uint32 poi_id) const
{
    return Trinity::Containers::MapGetValuePtr(_pointOfInterestLocaleStore, poi_id);
}

WorldQuestTemplate const* QuestDataStoreMgr::GetWorldQuestTemplate(Quest const* quest)
{
    WorldQuestTemplateMap::const_iterator itr = _worldQuestTemplate.find(quest->QuestInfoID);
    if (itr == _worldQuestTemplate.end())
        return nullptr;

    for (std::vector<WorldQuestTemplate>::const_iterator iter = itr->second.begin(); iter != itr->second.end(); ++iter)
    {
        if (WorldQuestTemplate const* worldQuest = &(*iter))
            if (worldQuest->ZoneID == 0 || worldQuest->ZoneID == quest->QuestSortID)
                return worldQuest;
    }
    return nullptr;
}

WorldQuestUpdate const* QuestDataStoreMgr::GetWorldQuestUpdate(uint32 QuestID, uint32 QuestInfoID, uint32 VariableID)
{
    std::vector<WorldQuestUpdate>* const WSV = Trinity::Containers::MapGetValuePtr(_worldQuestUpdate[QuestInfoID], QuestID);
    if (!WSV || WSV->empty())
        return nullptr;

    if ((*WSV).size() != 1)
    {
        if (VariableID)
        {
            for (uint8 i = 0; i < (*WSV).size(); ++i)
                if ((*WSV)[i].VariableID == VariableID)
                    return &(*WSV)[i];
        }
        for (uint8 i = 0; i < (*WSV).size(); ++i)
            if ((*WSV)[i].QuestID == QuestID)
                return &(*WSV)[i];
    }

    return &(*WSV)[0];
}

void QuestDataStoreMgr::GenerateWorldQuestUpdate()
{
    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate");

    // 7637 - suramar, 7558 - valsharah, 7541 - stormheim, 7503 - highmountain, 7334 - azsuna, 7543 - Broken Shore
    ResetWorldQuest();

    for (auto& worldQuestTempl : _worldQuestTemplate)
    {
        for (std::vector<WorldQuestTemplate>::iterator iter = worldQuestTempl.second.begin(); iter != worldQuestTempl.second.end(); ++iter)
        {
            WorldQuestTemplate const* wqTemplate = &(*iter);

            TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate wqTemplate QuestInfoID %u Min %u Max %u _worldQuestSet %u", wqTemplate->QuestInfoID, wqTemplate->Min, wqTemplate->Max, _worldQuestSet[wqTemplate->QuestInfoID].size());

            if (_worldQuestSet[wqTemplate->QuestInfoID].empty() || (wqTemplate->Chance && !roll_chance_f(wqTemplate->Chance)))
                continue;

            if (wqTemplate->PrimaryID || wqTemplate->QuestInfoID == QUEST_INFO_INVASION_POINT || wqTemplate->QuestInfoID == QUEST_INFO_GREATER_INVASION_POINT) // Activate other case, only if activated primary QuestInfoID
                continue;

            int16 countMaxQuest = wqTemplate->AllMax;

            if (wqTemplate->QuestInfoID == QUEST_INFO_LEGION_INVASION_WORLD_QUEST_WRAPPER && !WorldLegionInvasionZoneID)
            {
                // disable legion invasions till patch 7.2+
                if (sWorld->getIntConfig(CONFIG_LEGION_ENABLED_PATCH) >= 2)
                {
                    auto v = Trinity::Containers::SelectRandomContainerElement(_worldQuestSet[wqTemplate->QuestInfoID]);
                    WorldLegionInvasionZoneID = v.first;
                }
            }

            for (auto& v : _worldQuestSet[wqTemplate->QuestInfoID])
            {
                uint32 ZoneID = v.first;
                std::set<WorldQuestUpdate const*> _wQS = v.second;
                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate ZoneID %u _wQS size %u wqTemplate->ZoneID %u", ZoneID, _wQS.size(), wqTemplate->ZoneID);

                if (_wQS.empty() || (wqTemplate->ZoneID && wqTemplate->ZoneID != ZoneID))
                    continue;

                int16 countQuest = urand(wqTemplate->Min, wqTemplate->Max);
                if (countQuest > _wQS.size())
                    countQuest = _wQS.size();

                for (auto& vwq : _worldQuest[ZoneID])
                {
                    WorldQuest& Wq = vwq.second;
                    if (Wq.quest->QuestInfoID == wqTemplate->QuestInfoID)
                    {
                        countQuest--;
                        countMaxQuest--;
                    }
                }

                if (countMaxQuest <= 0 && wqTemplate->AllMax)
                    break;

                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate QuestInfoID %u countQuest %i _wQS %u ZoneID %u countMaxQuest %u WorldLegionInvasionZoneID %u", wqTemplate->QuestInfoID, countQuest, _wQS.size(), ZoneID, countMaxQuest, WorldLegionInvasionZoneID);

                while (countQuest > 0 && !_wQS.empty())
                {
                    if (wqTemplate->QuestInfoID == QUEST_INFO_DUNGEON_WORLD_QUEST)
                        if (!roll_chance_f(wqTemplate->Chance)) // Doble proc check
                            continue;

                    WorldQuestUpdate const* questUpdate = Trinity::Containers::SelectRandomContainerElement(_wQS);
                    _wQS.erase(questUpdate);

                    if (!questUpdate->quest || questUpdate->quest->QuestSortID < 1 || (!questUpdate->quest->IsWorld() && !questUpdate->quest->IsEmissary() && !questUpdate->quest->IsLegionInvasion()))
                        continue;

                    if (questUpdate->quest->IsLegionInvasion() && WorldLegionInvasionZoneID != ZoneID)
                        continue;

                    if (_worldQuest[questUpdate->quest->QuestSortID].find(questUpdate->QuestID) != _worldQuest[questUpdate->quest->QuestSortID].end())
                        continue;

                    if (!CanBeActivate(wqTemplate, questUpdate))
                        continue;

                    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate add QuestID %u QuestSortID %i Timer %u countQuest %u", questUpdate->QuestID, questUpdate->quest->QuestSortID, questUpdate->Timer, countQuest);

                    GenerateWorldQuest(questUpdate, wqTemplate);

                    countQuest--;
                    countMaxQuest--;
                }
            }
        }
    }

    for (auto& worldQuestTempl : _worldQuestTemplate)
    {
        for (std::vector<WorldQuestTemplate>::iterator iter = worldQuestTempl.second.begin(); iter != worldQuestTempl.second.end(); ++iter)
        {
            WorldQuestTemplate const* wqTemplate = &(*iter);

            TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate wqTemplate QuestInfoID %u Min %u Max %u _worldQuestSet %u PrimaryID %u",
            wqTemplate->QuestInfoID, wqTemplate->Min, wqTemplate->Max, _worldQuestSet[wqTemplate->QuestInfoID].size(), wqTemplate->PrimaryID);

            if (_worldQuestSet[wqTemplate->QuestInfoID].empty() || (wqTemplate->Chance && !roll_chance_f(wqTemplate->Chance)))
                continue;

            if (!wqTemplate->PrimaryID) // Activate only have primary QuestInfoID
                continue;

            TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate _worldQuestSetPrimaryID %u", _worldQuestSet[wqTemplate->PrimaryID].size());

            if (_worldQuestSet[wqTemplate->PrimaryID].empty())
                continue;

            int16 countMaxQuest = wqTemplate->AllMax;

            TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate countMaxQuest %u WorldLegionInvasionZoneID %u", countMaxQuest, WorldLegionInvasionZoneID);

            for (auto& v : _worldQuestSet[wqTemplate->QuestInfoID])
            {
                uint32 ZoneID = v.first;
                std::set<WorldQuestUpdate const*> _wQS = v.second;
                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate ZoneID %u _wQS size %u", ZoneID, _wQS.size());

                if (_wQS.empty() || WorldLegionInvasionZoneID != ZoneID || (wqTemplate->ZoneID && wqTemplate->ZoneID != ZoneID))
                    continue;

                int16 countQuest = urand(wqTemplate->Min, wqTemplate->Max);
                if (countQuest > _wQS.size())
                    countQuest = _wQS.size();

                for (auto& vwq : _worldQuest[ZoneID])
                {
                    WorldQuest& Wq = vwq.second;
                    if (Wq.quest->QuestInfoID == wqTemplate->QuestInfoID)
                    {
                        countQuest--;
                        countMaxQuest--;
                    }
                }

                if (countMaxQuest <= 0 && wqTemplate->AllMax)
                    break;

                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate QuestInfoID %u countQuest %i _wQS %u ZoneID %u countMaxQuest %u", wqTemplate->QuestInfoID, countQuest, _wQS.size(), ZoneID, countMaxQuest);

                while (countQuest > 0 && !_wQS.empty())
                {
                    WorldQuestUpdate const* questUpdate = Trinity::Containers::SelectRandomContainerElement(_wQS);
                    _wQS.erase(questUpdate);

                    if (!questUpdate->quest || questUpdate->quest->QuestSortID < 1 || (!questUpdate->quest->IsWorld() && !questUpdate->quest->IsEmissary()) || questUpdate->quest->QuestSortID != WorldLegionInvasionZoneID)
                        continue;

                    if (_worldQuest[questUpdate->quest->QuestSortID].find(questUpdate->QuestID) != _worldQuest[questUpdate->quest->QuestSortID].end())
                        continue;

                    if (!CanBeActivate(wqTemplate, questUpdate))
                        continue;

                    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateWorldQuestUpdate add QuestID %u QuestSortID %i Timer %u countQuest %u", questUpdate->QuestID, questUpdate->quest->QuestSortID, questUpdate->Timer, countQuest);

                    GenerateWorldQuest(questUpdate, wqTemplate);

                    countQuest--;
                    countMaxQuest--;
                }
            }
        }
    }


    for (const auto& wqByZones : _worldQuest)
        for (const auto& wqByQuests : wqByZones.second)
            if (uint32 questid = GetWorldQuestFactionAnalog(wqByQuests.first, true))
            {
                if (wqByZones.second.find(questid) != wqByZones.second.end()) // just exist ???
                    continue;

                GenerateNewWorldQuest(questid);
            }

    SaveWorldQuest();
}

void QuestDataStoreMgr::GenerateInvasionPointUpdate()
{
    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateInvasionPointUpdate");

    ResetWorldQuest();

    for (uint8 QuestInfoID = QUEST_INFO_INVASION_POINT; QuestInfoID <= QUEST_INFO_GREATER_INVASION_POINT; ++QuestInfoID)
    {
        for (std::vector<WorldQuestTemplate>::iterator iter = _worldQuestTemplate[QuestInfoID].begin(); iter != _worldQuestTemplate[QuestInfoID].end(); ++iter)
        {
            WorldQuestTemplate const* wqTemplate = &(*iter);

            TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateInvasionPointUpdate wqTemplate QuestInfoID %u Min %u Max %u _worldQuestSet %u", QuestInfoID, wqTemplate->Min, wqTemplate->Max, _worldQuestSet[QuestInfoID].size());

            if (_worldQuestSet[QuestInfoID].empty() || (wqTemplate->Chance && !roll_chance_f(wqTemplate->Chance)))
                continue;

            int16 countMaxQuest = wqTemplate->AllMax;

            for (auto& v : _worldQuestSet[QuestInfoID])
            {
                uint32 ZoneID = v.first;
                std::set<WorldQuestUpdate const*> _wQS = v.second;
                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateInvasionPointUpdate ZoneID %u _wQS size %u wqTemplate->ZoneID %u", ZoneID, _wQS.size(), wqTemplate->ZoneID);

                if (_wQS.empty() || (wqTemplate->ZoneID && wqTemplate->ZoneID != ZoneID))
                    continue;

                int16 countQuest = urand(wqTemplate->Min, wqTemplate->Max);
                if (countQuest > _wQS.size())
                    countQuest = _wQS.size();

                for (auto& vwq : _worldQuest[ZoneID])
                {
                    WorldQuest& Wq = vwq.second;
                    if (Wq.quest->QuestInfoID == QuestInfoID)
                    {
                        countQuest--;
                        countMaxQuest--;
                    }
                }

                if (countMaxQuest <= 0 && wqTemplate->AllMax)
                    break;

                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateInvasionPointUpdate QuestInfoID %u countQuest %i _wQS %u ZoneID %u countMaxQuest %u", QuestInfoID, countQuest, _wQS.size(), ZoneID, countMaxQuest);

                while (countQuest > 0 && !_wQS.empty())
                {
                    WorldQuestUpdate const* questUpdate = Trinity::Containers::SelectRandomContainerElement(_wQS);
                    _wQS.erase(questUpdate);

                    if (!questUpdate->quest || questUpdate->quest->QuestSortID < 1)
                        continue;

                    if (_worldQuest[questUpdate->quest->QuestSortID].find(questUpdate->QuestID) != _worldQuest[questUpdate->quest->QuestSortID].end())
                        continue;

                    if (!CanBeActivate(wqTemplate, questUpdate))
                        continue;

                    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateInvasionPointUpdate add QuestID %u QuestSortID %i Timer %u countQuest %u", questUpdate->QuestID, questUpdate->quest->QuestSortID, questUpdate->Timer, countQuest);

                    WorldQuest& Wq = _worldQuest[questUpdate->quest->QuestSortID][questUpdate->QuestID];
                    Wq.QuestID = questUpdate->QuestID;
                    Wq.VariableID = questUpdate->VariableID;
                    Wq.Value = questUpdate->Value;
                    Wq.State.insert(std::make_pair(Wq.VariableID, Wq.Value));
                    if (questUpdate->VariableID1)
                        Wq.State.insert(std::make_pair(questUpdate->VariableID1, questUpdate->Value1));
                    Wq.Timer = questUpdate->Timer;
                    Wq.StartTime = time(nullptr);
                    Wq.ResetTime = time(nullptr) + questUpdate->Timer;
                    Wq.quest = questUpdate->quest;
                    Wq.worldQuest = wqTemplate;

                    CalculateWorldQuestReward(wqTemplate, &Wq);
                    AddWorldQuestTask(questUpdate->quest);

                    for (std::set<WorldQuestState>::const_iterator itrState = Wq.State.begin(); itrState != Wq.State.end(); ++itrState)
                        sWorldStateMgr.SetWorldState(itrState->first, 0, itrState->second);

                    countQuest--;
                    countMaxQuest--;
                }
            }
        }
    }

    SaveWorldQuest();
}

void QuestDataStoreMgr::CalculateWorldQuestReward(WorldQuestTemplate const* qTemplate, WorldQuest* Wq)
{
    std::set<uint8> classListH = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_HUNTER, CLASS_ROGUE, CLASS_PRIEST, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_MAGE, CLASS_WARLOCK, CLASS_MONK, CLASS_DRUID, CLASS_DEMON_HUNTER };
    std::set<uint8> classListA = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_HUNTER, CLASS_ROGUE, CLASS_PRIEST, CLASS_DEATH_KNIGHT, CLASS_SHAMAN, CLASS_MAGE, CLASS_WARLOCK, CLASS_MONK, CLASS_DRUID, CLASS_DEMON_HUNTER };

    switch (GetWorldQuestTypeReward(qTemplate))
    {
        case WORLD_QUEST_TYPE_REWARD_ITEM:
        {
            switch (GetWorldQuestReward(qTemplate))
            {
                case WORLD_QUEST_TYPE_REWARD_ARTIFACT_POWER:
                {
                    Wq->ItemList[CLASS_NONE].ItemIDH = Trinity::Containers::SelectRandomContainerElement(qTemplate->ItemCAList);
                    Wq->ItemList[CLASS_NONE].ItemIDA = Wq->ItemList[CLASS_NONE].ItemIDH;
                    Wq->ItemList[CLASS_NONE].ItemCount = 1;
                    Wq->typeReward = WORLD_QUEST_TYPE_REWARD_ARTIFACT_POWER;
                    break;
                }
                case WORLD_QUEST_TYPE_REWARD_ARTIFACT_RELIC:
                {
                    bool _run = !_worldQuestRelic.empty();
                    while (_run)
                    {
                        ItemTemplate const* proto = Trinity::Containers::SelectRandomContainerElement(_worldQuestRelic);
                        if (!proto)
                            continue;

                        CheckGemForClass(classListH, classListH, proto, Wq);

                        _run = !classListH.empty();
                    }
                    classListA = classListH;
                    Wq->typeReward = WORLD_QUEST_TYPE_REWARD_ARTIFACT_RELIC;
                    break;
                }
                case WORLD_QUEST_TYPE_REWARD_RESOURCE:
                {
                    auto selectedItr = Trinity::Containers::SelectRandomWeightedContainerElement(qTemplate->ItemResourceList, [](WorldQuestTemplateItem const& it) -> double { return double(it.chance); });

                    Wq->ItemList[CLASS_NONE].ItemIDH = selectedItr->ItemID;
                    Wq->ItemList[CLASS_NONE].ItemIDA = selectedItr->ItemID;
                    Wq->ItemList[CLASS_NONE].ItemCount = urand(selectedItr->ItemCountMin, selectedItr->ItemCountMax);
                    if (Wq->ItemList[CLASS_NONE].ItemCount > 10)
                        Wq->ItemList[CLASS_NONE].ItemCount = uint32(Wq->ItemList[CLASS_NONE].ItemCount / 5) * 5; // Must be a multiple 5

                    Wq->typeReward = WORLD_QUEST_TYPE_REWARD_RESOURCE;
                    break;
                }
                case WORLD_QUEST_TYPE_REWARD_ARMOR:
                {
                    WorldQuestItemList tempList;
                    bool _run = !_worldQuestItem.empty() || !qTemplate->ArmorList.empty();
                    if (qTemplate->QuestInfoID == QUEST_INFO_DUNGEON_WORLD_QUEST)
                    {
                        _run = !_worldQuestItemDungeon.empty();
                        tempList = _worldQuestItemDungeon;
                    }
                    else if (!qTemplate->ArmorList.empty())
                        tempList = qTemplate->ArmorList;
                    else if (!_worldQuestItem.empty())
                        tempList = _worldQuestItem;

                    while (_run)
                    {
                        ItemTemplate const* proto = Trinity::Containers::SelectRandomContainerElement(tempList);
                        if (!proto)
                            continue;

                        CheckGemForClass(classListH, classListA, proto, Wq);
                        CheckItemForClass(classListH, classListA, proto, Wq);

                        _run = !classListH.empty() || !classListA.empty();
                    }
                    Wq->typeReward = WORLD_QUEST_TYPE_REWARD_ARMOR;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case WORLD_QUEST_TYPE_REWARD_GOLD:
            Wq->Gold = uint32(urand(qTemplate->GoldMin, qTemplate->GoldMax) / 100) * 100; // Must be a multiple 100
            Wq->typeReward = WORLD_QUEST_TYPE_REWARD_GOLD;
            break;
        case WORLD_QUEST_TYPE_REWARD_CURRENCY:
            switch (Wq->quest->AllowableRaces)
            {
            case 824181837://alliance
                Wq->CurrencyID = qTemplate->CurrencyID_A;
                break;
            case 234881970://horde
                Wq->CurrencyID = qTemplate->CurrencyID_H;
                break;
            default:
                Wq->CurrencyID = qTemplate->CurrencyID;
                break;
            }

            Wq->CurrencyCount = uint32(urand(qTemplate->CurrencyMin, qTemplate->CurrencyMax) / 25) * 25; // Must be a multiple 25
            Wq->typeReward = WORLD_QUEST_TYPE_REWARD_CURRENCY;
            break;
        default:
            break;
    }

    Wq->Recipe = GetRecipesForQuest(Wq->QuestID);

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CalculateWorldQuestReward >> CurrencyID %u CurrencyCount %u Gold %u ItemID %u ItemID %u", Wq->CurrencyID, Wq->CurrencyCount, Wq->Gold, Wq->ItemList[CLASS_NONE].ItemIDH, Wq->ItemList[CLASS_NONE].ItemCount);
}

WorldQuestTypeReward QuestDataStoreMgr::GetWorldQuestTypeReward(WorldQuestTemplate const* qTemplate)
{
    // 2456 -- all Count
    // 1820 - item - 74%
    // 326 - currency - 13%
    // 310 - gold - 13%

    WorldQuestTypeReward rewardType = WORLD_QUEST_TYPE_REWARD_ITEM;

    float chance = 12.5f;
    if (!qTemplate->HasArmor && qTemplate->ItemResourceList.empty() && qTemplate->ItemCAList.empty()) // If loot item not exist set default currency
    {
        rewardType = WORLD_QUEST_TYPE_REWARD_CURRENCY;
        chance = qTemplate->CurrencyID ? 50.0f : 100.0f;
    }

    if (qTemplate->GoldMax > 0 && roll_chance_f(chance)) // Gold chance
        rewardType = WORLD_QUEST_TYPE_REWARD_GOLD;
    else if (qTemplate->CurrencyID && roll_chance_f(chance)) // Currency chance
        rewardType = WORLD_QUEST_TYPE_REWARD_CURRENCY;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GetWorldQuestTypeReward >> rewardType %u HasArmor %u GoldMax %u CurrencyID %u", rewardType, qTemplate->HasArmor, qTemplate->GoldMax, qTemplate->CurrencyID);

    return rewardType;
}

WorldQuestTypeReward QuestDataStoreMgr::GetWorldQuestReward(WorldQuestTemplate const* qTemplate)
{
    // 1279 -- all Count
    // 597 - AP - 47%
    // 91 - relic - 7%
    // 152 - 124124 - 11%
    // 439 - armor - 34

    WorldQuestTypeReward rewardType = WORLD_QUEST_TYPE_REWARD_ARTIFACT_POWER;

    if (qTemplate->HasArmor && roll_chance_f(7.0f))
        rewardType = WORLD_QUEST_TYPE_REWARD_ARTIFACT_RELIC;
    else if (!qTemplate->ItemResourceList.empty() && roll_chance_f(11.0f))
        rewardType = WORLD_QUEST_TYPE_REWARD_RESOURCE;
    else if (qTemplate->HasArmor && roll_chance_f(34.0f))
        rewardType = WORLD_QUEST_TYPE_REWARD_ARMOR;

    if (qTemplate->ItemCAList.empty() && qTemplate->HasArmor)
        rewardType = WORLD_QUEST_TYPE_REWARD_ARMOR;
    else if (!qTemplate->ItemResourceList.empty() && qTemplate->ItemCAList.empty() && !qTemplate->HasArmor)
        rewardType = WORLD_QUEST_TYPE_REWARD_RESOURCE;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GetWorldQuestReward >> rewardType %u HasArmor %u ItemResourceList %u ItemCAList %u", rewardType, qTemplate->HasArmor, qTemplate->ItemResourceList.size(), qTemplate->ItemCAList.size());

    return rewardType;
}

void QuestDataStoreMgr::CheckGemForClass(std::set<uint8>& classListH, std::set<uint8>& classListA, ItemTemplate const* proto, WorldQuest* Wq)
{
    if (!proto)
        return;

    GemPropertiesEntry const* gem = sGemPropertiesStore.LookupEntry(proto->GetGemProperties());
    if (!gem)
        return;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckGemForClass start classListH %u classListA %u gem->Type %u", classListH.size(), classListA.size(), gem->Type);

    if (gem->Type & SOCKET_COLOR_RELIC_IRON)
    {
        for (auto const classId : {CLASS_WARRIOR, CLASS_DEATH_KNIGHT, CLASS_DEMON_HUNTER, CLASS_HUNTER, CLASS_MONK, CLASS_PALADIN, CLASS_ROGUE, CLASS_SHAMAN})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_BLOOD)
    {
        for (auto const classId : {CLASS_DEATH_KNIGHT, CLASS_HUNTER, CLASS_DRUID, CLASS_PRIEST, CLASS_ROGUE, CLASS_WARRIOR})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_SHADOW)
    {
        for (auto const classId : {CLASS_WARRIOR, CLASS_DEATH_KNIGHT, CLASS_DEMON_HUNTER, CLASS_PRIEST, CLASS_ROGUE, CLASS_WARLOCK})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_FEL)
    {
        for (auto const classId : {CLASS_DEMON_HUNTER, CLASS_ROGUE, CLASS_WARLOCK})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_ARCANE)
    {
        for (auto const classId : {CLASS_DEMON_HUNTER, CLASS_MAGE, CLASS_DRUID, CLASS_PALADIN, CLASS_HUNTER})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_FROST)
    {
        for (auto const classId : {CLASS_MAGE, CLASS_DEATH_KNIGHT, CLASS_DRUID, CLASS_MONK, CLASS_SHAMAN})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_FIRE)
    {
        for (auto const classId : {CLASS_DEATH_KNIGHT, CLASS_DRUID, CLASS_MAGE, CLASS_PALADIN, CLASS_SHAMAN, CLASS_WARLOCK, CLASS_WARRIOR})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_WATER)
    {
        for (auto const classId : {CLASS_MAGE, CLASS_DRUID, CLASS_SHAMAN, CLASS_MONK})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_LIFE)
    {
        for (auto const classId : {CLASS_PALADIN, CLASS_DRUID, CLASS_HUNTER, CLASS_PRIEST, CLASS_SHAMAN, CLASS_MONK})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_WIND)
    {
        for (auto const classId : {CLASS_WARRIOR, CLASS_ROGUE, CLASS_MONK, CLASS_HUNTER, CLASS_SHAMAN})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }
    if (gem->Type & SOCKET_COLOR_RELIC_HOLY)
    {
        for (auto const classId : {CLASS_PALADIN, CLASS_PRIEST})
        {
            classListH.erase(classId);
            classListA.erase(classId);
            Wq->ItemList[classId].ItemIDH = proto->GetId();
            Wq->ItemList[classId].ItemIDA = proto->GetId();
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckGemForClass end classListH %u classListA %u", classListH.size(), classListA.size());
}

void QuestDataStoreMgr::CheckItemForClass(std::set<uint8>& classListH, std::set<uint8>& classListA, ItemTemplate const* proto, WorldQuest* Wq)
{
    if (!proto || sGemPropertiesStore.LookupEntry(proto->GetGemProperties()))
        return;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckItemForClass start ItemId %u classListH %u classListA %u AllowableClass %i Class %i SubClass %i ItemSpecExist %i",
    proto->GetId(), classListH.size(), classListA.size(), proto->AllowableClass, proto->GetClass(), proto->GetSubClass(), proto->ItemSpecExist);

    if (!proto->AllowableClass || proto->AllowableClass > CLASSMASK_ALL_PLAYABLE)
    {
        if (proto->ItemSpecExist)
        {
            CheckItemForSpec(classListH, classListA, proto, Wq);
            return;
        }

        if (proto->GetClass() == ITEM_CLASS_ARMOR && proto->GetSubClass() > ITEM_SUBCLASS_ARMOR_MISCELLANEOUS && proto->GetSubClass() < ITEM_SUBCLASS_ARMOR_COSMETIC && proto->GetInventoryType() != INVTYPE_CLOAK)
        {
            if (proto->GetSubClass() == ITEM_SUBCLASS_ARMOR_PLATE)
            {
                for (auto const classId : {CLASS_WARRIOR, CLASS_PALADIN, CLASS_DEATH_KNIGHT})
                {
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(classId);
                        Wq->ItemList[classId].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(classId);
                        Wq->ItemList[classId].ItemIDA = proto->GetId();
                    }
                }
            }
            if (proto->GetSubClass() == ITEM_SUBCLASS_ARMOR_MAIL)
            {
                for (auto const classId : {CLASS_HUNTER, CLASS_SHAMAN})
                {
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(classId);
                        Wq->ItemList[classId].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(classId);
                        Wq->ItemList[classId].ItemIDA = proto->GetId();
                    }
                }
            }
            if (proto->GetSubClass() == ITEM_SUBCLASS_ARMOR_LEATHER)
            {
                for (auto const classId : {CLASS_ROGUE, CLASS_DRUID, CLASS_DEMON_HUNTER, CLASS_MONK})
                {
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(classId);
                        Wq->ItemList[classId].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(classId);
                        Wq->ItemList[classId].ItemIDA = proto->GetId();
                    }
                }
            }
            if (proto->GetSubClass() == ITEM_SUBCLASS_ARMOR_CLOTH)
            {
                for (auto const classId : {CLASS_MAGE, CLASS_PRIEST, CLASS_WARLOCK})
                {
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(classId);
                        Wq->ItemList[classId].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(classId);
                        Wq->ItemList[classId].ItemIDA = proto->GetId();
                    }
                }
            }
            return;
        }
        if (CheckItemForHorde(proto))
        {
            classListH.clear();
            Wq->ItemList[CLASS_NONE].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.clear();
            Wq->ItemList[CLASS_NONE].ItemIDA = proto->GetId();
        }
        TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckItemForClass >> clear");
    }

    if (proto->AllowableClass & (1 << (CLASS_WARRIOR - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_WARRIOR);
            Wq->ItemList[CLASS_WARRIOR].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_WARRIOR);
            Wq->ItemList[CLASS_WARRIOR].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_PALADIN - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_PALADIN);
            Wq->ItemList[CLASS_PALADIN].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_PALADIN);
            Wq->ItemList[CLASS_PALADIN].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_HUNTER - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_HUNTER);
            Wq->ItemList[CLASS_HUNTER].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_HUNTER);
            Wq->ItemList[CLASS_HUNTER].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_ROGUE - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_ROGUE);
            Wq->ItemList[CLASS_ROGUE].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_ROGUE);
            Wq->ItemList[CLASS_ROGUE].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_PRIEST - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_PRIEST);
            Wq->ItemList[CLASS_PRIEST].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_PRIEST);
            Wq->ItemList[CLASS_PRIEST].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_DEATH_KNIGHT - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_DEATH_KNIGHT);
            Wq->ItemList[CLASS_DEATH_KNIGHT].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_DEATH_KNIGHT);
            Wq->ItemList[CLASS_DEATH_KNIGHT].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_SHAMAN - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_SHAMAN);
            Wq->ItemList[CLASS_SHAMAN].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_SHAMAN);
            Wq->ItemList[CLASS_SHAMAN].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_MAGE - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_MAGE);
            Wq->ItemList[CLASS_MAGE].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_MAGE);
            Wq->ItemList[CLASS_MAGE].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_WARLOCK - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_WARLOCK);
            Wq->ItemList[CLASS_WARLOCK].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_WARLOCK);
            Wq->ItemList[CLASS_WARLOCK].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_MONK - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_MONK);
            Wq->ItemList[CLASS_MONK].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_MONK);
            Wq->ItemList[CLASS_MONK].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_DRUID - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_DRUID);
            Wq->ItemList[CLASS_DRUID].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_DRUID);
            Wq->ItemList[CLASS_DRUID].ItemIDA = proto->GetId();
        }
    }
    if (proto->AllowableClass & (1 << (CLASS_DEMON_HUNTER - 1)))
    {
        if (CheckItemForHorde(proto))
        {
            classListH.erase(CLASS_DEMON_HUNTER);
            Wq->ItemList[CLASS_DEMON_HUNTER].ItemIDH = proto->GetId();
        }
        if (CheckItemForAlliance(proto))
        {
            classListA.erase(CLASS_DEMON_HUNTER);
            Wq->ItemList[CLASS_DEMON_HUNTER].ItemIDA = proto->GetId();
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckItemForClass end classListH %u classListA %u", classListH.size(), classListA.size());
}

void QuestDataStoreMgr::CheckItemForSpec(std::set<uint8>& classListH, std::set<uint8>& classListA, ItemTemplate const* proto, WorldQuest* Wq)
{
    if (!proto)
        return;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckItemForSpec >> start classListH size %u classListA size %u", classListH.size(), classListA.size());

    for (auto const specId : { SPEC_MAGE_ARCANE, SPEC_MAGE_FIRE, SPEC_MAGE_FROST, SPEC_PALADIN_HOLY, SPEC_PALADIN_PROTECTION, SPEC_PALADIN_RETRIBUTION, SPEC_WARRIOR_ARMS, SPEC_WARRIOR_FURY, SPEC_WARRIOR_PROTECTION, SPEC_DRUID_BALANCE, SPEC_DRUID_CAT, SPEC_DRUID_BEAR, SPEC_DRUID_RESTORATION, SPEC_DK_BLOOD, SPEC_DK_FROST, SPEC_DK_UNHOLY, SPEC_HUNTER_BEASTMASTER, SPEC_HUNTER_MARKSMAN, SPEC_HUNTER_SURVIVAL, SPEC_PRIEST_DISCIPLINE, SPEC_PRIEST_HOLY, SPEC_PRIEST_SHADOW, SPEC_ROGUE_ASSASSINATION, SPEC_ROGUE_COMBAT, SPEC_ROGUE_SUBTLETY, SPEC_SHAMAN_ELEMENTAL, SPEC_SHAMAN_ENHANCEMENT, SPEC_SHAMAN_RESTORATION, SPEC_WARLOCK_AFFLICTION, SPEC_WARLOCK_DEMONOLOGY, SPEC_WARLOCK_DESTRUCTION, SPEC_MONK_BREWMASTER, SPEC_MONK_WINDWALKER, SPEC_MONK_MISTWEAVER, SPEC_DEMON_HUNER_HAVOC, SPEC_DEMON_HUNER_VENGEANCE, SPEC_MAGE_ARCANE, SPEC_MAGE_FIRE, SPEC_MAGE_FROST, SPEC_PALADIN_HOLY, SPEC_PALADIN_PROTECTION, SPEC_PALADIN_RETRIBUTION, SPEC_WARRIOR_ARMS, SPEC_WARRIOR_FURY, SPEC_WARRIOR_PROTECTION, SPEC_DRUID_BALANCE, SPEC_DRUID_CAT, SPEC_DRUID_BEAR, SPEC_DRUID_RESTORATION, SPEC_DK_BLOOD, SPEC_DK_FROST, SPEC_DK_UNHOLY, SPEC_HUNTER_BEASTMASTER, SPEC_HUNTER_MARKSMAN, SPEC_HUNTER_SURVIVAL, SPEC_PRIEST_DISCIPLINE, SPEC_PRIEST_HOLY, SPEC_PRIEST_SHADOW, SPEC_ROGUE_ASSASSINATION, SPEC_ROGUE_COMBAT, SPEC_ROGUE_SUBTLETY, SPEC_SHAMAN_ELEMENTAL, SPEC_SHAMAN_ENHANCEMENT, SPEC_SHAMAN_RESTORATION, SPEC_WARLOCK_AFFLICTION, SPEC_WARLOCK_DEMONOLOGY, SPEC_WARLOCK_DESTRUCTION, SPEC_MONK_BREWMASTER, SPEC_MONK_WINDWALKER, SPEC_MONK_MISTWEAVER, SPEC_DEMON_HUNER_HAVOC, SPEC_DEMON_HUNER_VENGEANCE })
    {
        if (proto->IsUsableBySpecialization(specId, 110))
        {
            switch (specId)
            {
                case SPEC_MAGE_ARCANE:
                case SPEC_MAGE_FIRE:
                case SPEC_MAGE_FROST:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_MAGE);
                        Wq->ItemList[CLASS_MAGE].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_MAGE);
                        Wq->ItemList[CLASS_MAGE].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_PALADIN_HOLY:
                case SPEC_PALADIN_PROTECTION:
                case SPEC_PALADIN_RETRIBUTION:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_PALADIN);
                        Wq->ItemList[CLASS_PALADIN].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_PALADIN);
                        Wq->ItemList[CLASS_PALADIN].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_WARRIOR_ARMS:
                case SPEC_WARRIOR_FURY:
                case SPEC_WARRIOR_PROTECTION:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_WARRIOR);
                        Wq->ItemList[CLASS_WARRIOR].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_WARRIOR);
                        Wq->ItemList[CLASS_WARRIOR].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_DRUID_BALANCE:
                case SPEC_DRUID_CAT:
                case SPEC_DRUID_BEAR:
                case SPEC_DRUID_RESTORATION:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_DRUID);
                        Wq->ItemList[CLASS_DRUID].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_DRUID);
                        Wq->ItemList[CLASS_DRUID].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_DK_BLOOD:
                case SPEC_DK_FROST:
                case SPEC_DK_UNHOLY:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_DEATH_KNIGHT);
                        Wq->ItemList[CLASS_DEATH_KNIGHT].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_DEATH_KNIGHT);
                        Wq->ItemList[CLASS_DEATH_KNIGHT].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_HUNTER_BEASTMASTER:
                case SPEC_HUNTER_MARKSMAN:
                case SPEC_HUNTER_SURVIVAL:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_HUNTER);
                        Wq->ItemList[CLASS_HUNTER].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_HUNTER);
                        Wq->ItemList[CLASS_HUNTER].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_PRIEST_DISCIPLINE:
                case SPEC_PRIEST_HOLY:
                case SPEC_PRIEST_SHADOW:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_PRIEST);
                        Wq->ItemList[CLASS_PRIEST].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_PRIEST);
                        Wq->ItemList[CLASS_PRIEST].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_ROGUE_ASSASSINATION:
                case SPEC_ROGUE_COMBAT:
                case SPEC_ROGUE_SUBTLETY:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_ROGUE);
                        Wq->ItemList[CLASS_ROGUE].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_ROGUE);
                        Wq->ItemList[CLASS_ROGUE].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_SHAMAN_ELEMENTAL:
                case SPEC_SHAMAN_ENHANCEMENT:
                case SPEC_SHAMAN_RESTORATION:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_SHAMAN);
                        Wq->ItemList[CLASS_SHAMAN].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_SHAMAN);
                        Wq->ItemList[CLASS_SHAMAN].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_WARLOCK_AFFLICTION:
                case SPEC_WARLOCK_DEMONOLOGY:
                case SPEC_WARLOCK_DESTRUCTION:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_WARLOCK);
                        Wq->ItemList[CLASS_WARLOCK].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_WARLOCK);
                        Wq->ItemList[CLASS_WARLOCK].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_MONK_BREWMASTER:
                case SPEC_MONK_WINDWALKER:
                case SPEC_MONK_MISTWEAVER:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_MONK);
                        Wq->ItemList[CLASS_MONK].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_MONK);
                        Wq->ItemList[CLASS_MONK].ItemIDA = proto->GetId();
                    }
                    break;
                case SPEC_DEMON_HUNER_HAVOC:
                case SPEC_DEMON_HUNER_VENGEANCE:
                    if (CheckItemForHorde(proto))
                    {
                        classListH.erase(CLASS_DEMON_HUNTER);
                        Wq->ItemList[CLASS_DEMON_HUNTER].ItemIDH = proto->GetId();
                    }
                    if (CheckItemForAlliance(proto))
                    {
                        classListA.erase(CLASS_DEMON_HUNTER);
                        Wq->ItemList[CLASS_DEMON_HUNTER].ItemIDA = proto->GetId();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CheckItemForSpec >> end classListH size %u classListA size %u", classListH.size(), classListA.size());
}

bool QuestDataStoreMgr::CheckItemForHorde(ItemTemplate const* proto)
{
    if (proto->GetFlags2() & ITEM_FLAG2_FACTION_ALLIANCE)
        return false;

    return true;
}

bool QuestDataStoreMgr::CheckItemForAlliance(ItemTemplate const* proto)
{
    if (proto->GetFlags2() & ITEM_FLAG2_FACTION_HORDE)
        return false;

    return true;
}

void QuestDataStoreMgr::SaveWorldQuest()
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = nullptr;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "SaveWorldQuest() _worldQuest size %u", _worldQuest.size());

    for (auto& v : _worldQuest)
    {
        TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "SaveWorldQuest() QuestSortID %u size %u", v.first, v.second.size());

        for (auto& vs : v.second)
        {
            WorldQuest* worldQuest = &vs.second;
            if (!worldQuest)
                continue;

            uint8 index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_WORLD_QUEST);
            stmt->setUInt32(index++, worldQuest->QuestID);
            stmt->setUInt32(index++, worldQuest->quest->QuestInfoID);
            stmt->setInt32(index++, worldQuest->quest->QuestSortID);
            stmt->setUInt32(index++, worldQuest->VariableID);
            stmt->setUInt32(index++, worldQuest->Value);
            stmt->setUInt32(index++, worldQuest->Timer);
            stmt->setUInt32(index++, worldQuest->StartTime);
            stmt->setUInt32(index++, worldQuest->ResetTime);
            stmt->setUInt32(index++, worldQuest->CurrencyID);
            stmt->setUInt32(index++, worldQuest->CurrencyCount);
            stmt->setUInt32(index++, worldQuest->Gold);

            std::ostringstream ss;
            ss << worldQuest->ItemList[CLASS_NONE].ItemIDH;
            ss << ' ' << worldQuest->ItemList[CLASS_NONE].ItemIDA;
            ss << ' ' << worldQuest->ItemList[CLASS_NONE].ItemCount;
            for (uint8 i = CLASS_WARRIOR; i < MAX_CLASSES; ++i)
            {
                ss << ' ' << worldQuest->ItemList[i].ItemIDH;
                ss << ' ' << worldQuest->ItemList[i].ItemIDA;
                ss << ' ' << worldQuest->ItemList[i].ItemCount;
            }

            stmt->setString(index++, ss.str());
            stmt->setUInt32(index++, worldQuest->typeReward);

            trans->Append(stmt);
        }
    }
    CharacterDatabase.CommitTransaction(trans);
}

void QuestDataStoreMgr::ResetWorldQuest()
{
    needWait = true;
    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "ResetWorldQuest()");

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    std::ostringstream ss;
    ss << "DELETE FROM world_quest WHERE ResetTime <= UNIX_TIMESTAMP() + " << MINUTE * 5;
    trans->Append(ss.str().c_str());
    ss.str("");
    ss << "DELETE FROM character_queststatus_world WHERE resetTime <= UNIX_TIMESTAMP() + " << MINUTE * 5;
    trans->Append(ss.str().c_str());

    SessionMap const& sessionAll = sWorld->GetAllSessions();
    for (SessionMap::const_iterator iter = sessionAll.begin(); iter != sessionAll.end(); ++iter)
        if (Player* player = iter->second->GetPlayer())
            player->ResetWorldQuest();

    for (WorldQuestMap::iterator itr = _worldQuest.begin(); itr != _worldQuest.end(); ++itr)
    {
        for (std::map<uint32 /*QuestID*/, WorldQuest>::iterator iter = itr->second.begin(); iter != itr->second.end();)
        {
            WorldQuest* worldQuest = &iter->second;
            if (worldQuest->ResetTime <= (time(nullptr) + MINUTE * 5))
            {
                if (worldQuest->quest && (worldQuest->quest->IsEmissary() || worldQuest->quest->IsLegionInvasion()))
                {
                    ss.str("");
                    ss << "DELETE FROM character_queststatus WHERE quest = '" << iter->first << "';";
                    trans->Append(ss.str().c_str());
                }

                if (worldQuest->quest->IsLegionInvasion())
                    WorldLegionInvasionZoneID = 0;

                for (std::set<WorldQuestState>::const_iterator itrState = worldQuest->State.begin(); itrState != worldQuest->State.end(); ++itrState)
                    sWorldStateMgr.SetWorldState(itrState->first, 0, 0);

                RemoveWorldQuestTask(worldQuest->quest);
                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "ResetWorldQuest() >> QuestID %u ResetTime %u time(nullptr) %u", worldQuest->QuestID, worldQuest->ResetTime, time(nullptr));
                itr->second.erase(iter++);
                continue;
            }
            ++iter;
        }
    }
    CharacterDatabase.CommitTransaction(trans);
    CharacterDatabase.WaitExecution();
    needWait = false;
}

void QuestDataStoreMgr::ClearWorldQuest()
{
    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "ClearWorldQuest()");

    CharacterDatabase.Execute("TRUNCATE world_quest;");
    CharacterDatabase.Execute("TRUNCATE character_queststatus_world;");

    SessionMap const& sessionAll = sWorld->GetAllSessions();
    for (SessionMap::const_iterator iter = sessionAll.begin(); iter != sessionAll.end(); ++iter)
        if (Player* player = iter->second->GetPlayer())
            player->ClearWorldQuest();

    for (WorldQuestMap::iterator itr = _worldQuest.begin(); itr != _worldQuest.end(); ++itr)
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        for (std::map<uint32 /*QuestID*/, WorldQuest>::iterator iter = itr->second.begin(); iter != itr->second.end();)
        {
            WorldQuest* worldQuest = &iter->second;
            if (worldQuest->quest && (worldQuest->quest->IsEmissary() || worldQuest->quest->IsLegionInvasion()))
            {
                std::ostringstream ss;
                ss << "DELETE FROM character_queststatus WHERE quest = '" << iter->first << "';";
                trans->Append(ss.str().c_str());
                std::ostringstream sss;
                sss << "DELETE FROM character_queststatus_objectives WHERE quest = '" << iter->first << "';";
                trans->Append(sss.str().c_str());
            }

            if (worldQuest->quest->IsLegionInvasion())
                WorldLegionInvasionZoneID = 0;

            for (std::set<WorldQuestState>::const_iterator itrState = worldQuest->State.begin(); itrState != worldQuest->State.end(); ++itrState)
                sWorldStateMgr.SetWorldState(itrState->first, 0, 0);

            RemoveWorldQuestTask(worldQuest->quest);
            TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "ClearWorldQuest() >> QuestID %u ResetTime %u time(nullptr) %u", worldQuest->QuestID, worldQuest->ResetTime, time(nullptr));
            itr->second.erase(iter++);
        }
        CharacterDatabase.CommitTransaction(trans);
        CharacterDatabase.WaitExecution();
    }
}

WorldQuest const* QuestDataStoreMgr::GetWorldQuest(Quest const* quest)
{
    if (!quest)
        return nullptr;

    return Trinity::Containers::MapGetValuePtr(_worldQuest[quest->QuestSortID], quest->GetQuestId());
}

WorldQuest const* QuestDataStoreMgr::GenerateNewWorldQuest(uint32 QuestID, uint32 VariableID)
{
    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "GenerateNewWorldQuest QuestID %u", QuestID);

    Quest const* quest = GetQuestTemplate(QuestID);
    if (!quest)
        return nullptr;
    WorldQuestTemplate const* worldQuest = GetWorldQuestTemplate(quest);
    if (!worldQuest)
        return nullptr;
    WorldQuestUpdate const* worldQuestU = GetWorldQuestUpdate(QuestID, quest->QuestInfoID, VariableID);
    if (!worldQuestU)
        return nullptr;

    auto result = GenerateWorldQuest(worldQuestU, worldQuest);
    SaveWorldQuest();
    return result;
}

WorldQuest const* QuestDataStoreMgr::GenerateWorldQuest(WorldQuestUpdate const* questUpdate, WorldQuestTemplate const* wqTemplate)
{
    WorldQuest& Wq = _worldQuest[questUpdate->quest->QuestSortID][questUpdate->QuestID];
    Wq.QuestID = questUpdate->QuestID;
    Wq.VariableID = questUpdate->VariableID;
    Wq.Value = questUpdate->Value;
    Wq.State.insert(std::make_pair(Wq.VariableID, Wq.Value));
    if (questUpdate->VariableID1)
        Wq.State.insert(std::make_pair(questUpdate->VariableID1, questUpdate->Value1));
    Wq.Timer = questUpdate->Timer;
    Wq.StartTime = time(nullptr);
    Wq.ResetTime = time(nullptr) + questUpdate->Timer;
    Wq.quest = questUpdate->quest;
    Wq.worldQuest = wqTemplate;

    if (QuestV2CliTaskEntry const* questTask = sQuestV2CliTaskStore.LookupEntry(questUpdate->QuestID))
        if (questTask->WorldStateExpressionID)
            if (WorldStateExpressionEntry const* worldStateExpEntry = sWorldStateExpressionStore.LookupEntry(questTask->WorldStateExpressionID))
                worldStateExpEntry->Eval(nullptr, &Wq.State);

    CalculateWorldQuestReward(wqTemplate, &Wq);

    if (!questUpdate->quest->IsEmissary())
        AddWorldQuestTask(questUpdate->quest);

    for (std::set<WorldQuestState>::const_iterator itrState = Wq.State.begin(); itrState != Wq.State.end(); ++itrState)
        sWorldStateMgr.SetWorldState(itrState->first, 0, itrState->second);

    return &Wq;
}

void QuestDataStoreMgr::ResetWorldQuest(uint32 QuestID)
{
    needWait = true;
    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "ResetWorldQuest %u", QuestID);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    std::ostringstream ss;
    ss << "DELETE FROM world_quest WHERE QuestID =  " << QuestID;
    trans->Append(ss.str().c_str());
    ss.str("");
    ss << "DELETE FROM character_queststatus_world WHERE quest = " << QuestID;
    trans->Append(ss.str().c_str());

    SessionMap const& sessionAll = sWorld->GetAllSessions();
    for (SessionMap::const_iterator iter = sessionAll.begin(); iter != sessionAll.end(); ++iter)
        if (Player* player = iter->second->GetPlayer())
            player->ResetWorldQuest(QuestID);

    for (WorldQuestMap::iterator itr = _worldQuest.begin(); itr != _worldQuest.end(); ++itr)
    {
        for (std::map<uint32 /*QuestID*/, WorldQuest>::iterator iter = itr->second.begin(); iter != itr->second.end();)
        {
            WorldQuest* worldQuest = &iter->second;
            if (worldQuest->QuestID == QuestID)
            {
                if (worldQuest->quest && (worldQuest->quest->IsEmissary() || worldQuest->quest->IsLegionInvasion()))
                {
                    ss.str("");
                    ss << "DELETE FROM character_queststatus WHERE quest = '" << iter->first << "';";
                    trans->Append(ss.str().c_str());
                }

                if (worldQuest->quest->IsLegionInvasion())
                    WorldLegionInvasionZoneID = 0;

                for (std::set<WorldQuestState>::const_iterator itrState = worldQuest->State.begin(); itrState != worldQuest->State.end(); ++itrState)
                    sWorldStateMgr.SetWorldState(itrState->first, 0, 0);

                RemoveWorldQuestTask(worldQuest->quest);
                TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "ResetWorldQuest() >> QuestID %u ResetTime %u time(nullptr) %u", worldQuest->QuestID, worldQuest->ResetTime, time(nullptr));
                itr->second.erase(iter++);
                continue;
            }
            ++iter;
        }
    }
    CharacterDatabase.CommitTransaction(trans);
    CharacterDatabase.WaitExecution();
    needWait = false;
}

uint32 QuestDataStoreMgr::GetWorldQuestFactionAnalog(uint32 questId, bool findAllianceQuest) const
{
    auto itr = _worldQuestsFactionAnalogs[uint8(findAllianceQuest)].find(questId);
    if (itr != _worldQuestsFactionAnalogs[uint8(findAllianceQuest)].end())
        return itr->second;

    return 0;
}

void QuestDataStoreMgr::AddWorldQuestTask(Quest const* quest)
{
    if (quest->AreaGroupID > 0)
    {
        std::vector<uint32> areaGroupMembers = sDB2Manager.GetAreasForGroup(quest->AreaGroupID);
        for (uint32 areaId : areaGroupMembers)
            _worldQuestAreaTaskStore[areaId].insert(quest);
    }
    else
    {
        WorldQuestUpdate const* worldQuest = GetWorldQuestUpdate(quest->GetQuestId(), quest->QuestInfoID);
        if (worldQuest && !worldQuest->AreaIDs.empty())
        {
            for (std::vector<uint32>::const_iterator itr = worldQuest->AreaIDs.begin(); itr != worldQuest->AreaIDs.end(); ++itr)
                _worldQuestAreaTaskStore[*itr].insert(quest);
            return;
        }

        for (QuestObjective const& obj : quest->GetObjectives())
        {
            switch (obj.Type)
            {
                case QUEST_OBJECTIVE_MONSTER:
                    if (QueryResult result = WorldDatabase.PQuery("SELECT `areaId` FROM `creature` WHERE `id` = '%u' AND `zoneId` = '%u';", obj.ObjectID, quest->QuestSortID))
                    {
                        do
                        {
                            _worldQuestAreaTaskStore[(*result)[0].GetUInt16()].insert(quest);
                        } while (result->NextRow());
                    }
                    break;
                case QUEST_OBJECTIVE_GAMEOBJECT:
                    if (QueryResult result = WorldDatabase.PQuery("SELECT `areaId` FROM `gameobject` WHERE `id` = '%u' AND `zoneId` = '%u';", obj.ObjectID, quest->QuestSortID))
                    {
                        do
                        {
                            _worldQuestAreaTaskStore[(*result)[0].GetUInt16()].insert(quest);
                        } while (result->NextRow());
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void QuestDataStoreMgr::RemoveWorldQuestTask(Quest const* quest)
{
    if (!quest)
        return;

    for (auto& taskSet : _worldQuestAreaTaskStore)
        taskSet.second.erase(quest);
}

std::set<Quest const*> const* QuestDataStoreMgr::GetWorldQuestTask(uint32 areaId) const
{
    return Trinity::Containers::MapGetValuePtr(_worldQuestAreaTaskStore, areaId);
}

bool QuestDataStoreMgr::CanBeActivate(WorldQuestTemplate const* qTemplate, WorldQuestUpdate const* questUpdate)
{
    if (!qTemplate)
        return false;

    Quest const* quest = questUpdate->quest;

    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CanBeActivate QuestInfoID %u QuestInfoID %u QuestID %u QuestSortID %u VariableID %u QuestID %u EventID %u", quest->QuestInfoID, qTemplate->QuestInfoID, quest->GetQuestId(), quest->QuestSortID, questUpdate->VariableID, questUpdate->QuestID, questUpdate->EventID);

    if (questUpdate->EventID)
        return false;

    switch (qTemplate->QuestInfoID)
    {
        case QUEST_INFO_EMISSARY_QUEST:
        {
            for (auto& v : _worldQuest)
            {
                for (auto& vs : v.second)
                {
                    WorldQuest* worldQuest = &vs.second;
                    if (!worldQuest || !worldQuest->quest || !worldQuest->quest->IsEmissary())
                        continue;

                    TC_LOG_DEBUG(LOG_FILTER_WORLD_QUEST, "CanBeActivate >> ResetTime %u QuestID %u if %u", worldQuest->ResetTime, worldQuest->QuestID, worldQuest->ResetTime >= (time(nullptr) + WORLD_QUEST_2_DAY));

                    if (worldQuest->ResetTime >= (time(nullptr) + WORLD_QUEST_2_DAY)) // Only one Emissary quest peer day
                        return false;
                }
            }
            break;
        }
        default:
            break;
    }
    return true;
}

WorldQuestRecipe const* QuestDataStoreMgr::GetRecipesForQuest(uint32 QuestID)
{
    auto itr = _worldQuestRecipes.find(QuestID);
    if (itr != _worldQuestRecipes.end())
        return &itr->second;
    return nullptr;
}
