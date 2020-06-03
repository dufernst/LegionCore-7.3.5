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

#include "GameEventMgr.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "PoolMgr.h"
#include "Language.h"
#include "Log.h"
#include "MapManager.h"
#include <WowTime.hpp>
#include "Player.h"
#include "BattlegroundMgr.h"
#include "UnitAI.h"
#include "GameObjectAI.h"
#include "DatabaseEnv.h"
#include "QuestData.h"
#include "WorldStateMgr.h"
#include "OutdoorPvPMgr.h"

static uint32 timerConstant = 946684800; // 01/01/2000 00:00:00

GameEventMgr::ActiveEvents const& GameEventMgr::GetActiveEventList() const
{
    return m_ActiveEvents;
}

GameEventMgr::GameEventDataMap const& GameEventMgr::GetEventMap() const
{
    return mGameEvent;
}

bool GameEventMgr::CheckOneGameEvent(uint16 entry) const
{
    switch (mGameEvent[entry].state)
    {
    default:
    case GAMEEVENT_NORMAL:
    {
        time_t currenttime = time(nullptr);
        // Get the event information
        return mGameEvent[entry].start < currenttime
            && currenttime < mGameEvent[entry].end
            && (currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * MINUTE) < mGameEvent[entry].length * MINUTE;
    }
    // if the state is conditions or nextphase, then the event should be active
    case GAMEEVENT_WORLD_CONDITIONS:
    case GAMEEVENT_WORLD_NEXTPHASE:
        return true;
        // finished world events are inactive
    case GAMEEVENT_WORLD_FINISHED:
    case GAMEEVENT_INTERNAL:
        return false;
        // if inactive world event, check the prerequisite events
    case GAMEEVENT_WORLD_INACTIVE:
    {
        time_t currenttime = time(nullptr);
        for (auto itr = mGameEvent[entry].prerequisite_events.begin(); itr != mGameEvent[entry].prerequisite_events.end(); ++itr)
        {
            if ((mGameEvent[*itr].state != GAMEEVENT_WORLD_NEXTPHASE && mGameEvent[*itr].state != GAMEEVENT_WORLD_FINISHED) ||   // if prereq not in nextphase or finished state, then can't start this one
                mGameEvent[*itr].nextstart > currenttime)               // if not in nextphase state for long enough, can't start this one
                return false;
        }
        // all prerequisite events are met
        // but if there are no prerequisites, this can be only activated through gm command
        return !(mGameEvent[entry].prerequisite_events.empty());
    }
    }
}

uint32 GameEventMgr::NextCheck(uint16 entry) const
{
    time_t currenttime = time(nullptr);

    // for NEXTPHASE state world events, return the delay to start the next event, so the followup event will be checked correctly
    if ((mGameEvent[entry].state == GAMEEVENT_WORLD_NEXTPHASE || mGameEvent[entry].state == GAMEEVENT_WORLD_FINISHED) && mGameEvent[entry].nextstart >= currenttime)
        return uint32(mGameEvent[entry].nextstart - currenttime);

    // for CONDITIONS state world events, return the length of the wait period, so if the conditions are met, this check will be called again to set the timer as NEXTPHASE event
    if (mGameEvent[entry].state == GAMEEVENT_WORLD_CONDITIONS)
    {
        if (mGameEvent[entry].length)
            return mGameEvent[entry].length * 60;
        return max_ge_check_delay;
    }

    // outdated event: we return max
    if (currenttime > mGameEvent[entry].end)
        return max_ge_check_delay;

    // never started event, we return delay before start
    if (mGameEvent[entry].start > currenttime)
        return uint32(mGameEvent[entry].start - currenttime);

    uint32 delay;
    // in event, we return the end of it
    if ((((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * 60)) < (mGameEvent[entry].length * 60)))
        // we return the delay before it ends
        delay = (mGameEvent[entry].length * MINUTE) - ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * MINUTE));
    else                                                    // not in window, we return the delay before next start
        delay = (mGameEvent[entry].occurence * MINUTE) - ((currenttime - mGameEvent[entry].start) % (mGameEvent[entry].occurence * MINUTE));
    // In case the end is before next check
    if (mGameEvent[entry].end < time_t(currenttime + delay))
        return uint32(mGameEvent[entry].end - currenttime);
    return delay;
}

void GameEventMgr::StartInternalEvent(uint16 event_id)
{
    if (event_id < 1 || event_id >= mGameEvent.size())
        return;

    if (!mGameEvent[event_id].isValid())
        return;

    if (m_ActiveEvents.find(event_id) != m_ActiveEvents.end())
        return;

    StartEvent(event_id);
}

bool GameEventMgr::StartEvent(uint16 event_id, bool overwrite)
{
    GameEventData &data = mGameEvent[event_id];
    if (data.state == GAMEEVENT_NORMAL || data.state == GAMEEVENT_INTERNAL)
    {
        AddActiveEvent(event_id);
        ApplyNewEvent(event_id);
        if (overwrite)
        {
            mGameEvent[event_id].start = time(nullptr);
            if (data.end <= data.start)
                data.end = data.start + data.length;
        }

        // temporally. delete after new year event end
        switch (event_id)
        {
        case 811:
            sWorld->SendWorldText(542143);
            break;
        case 812:
            sWorld->SendWorldText(542137);
            break;
        case 813:
        case 814:
        case 815:
        case 816:
        case 817:
        case 818:
        case 819:
        case 820:
        case 821:
            sWorld->SendWorldText(542138);
            break;
        case 823:
            sWorld->SendWorldText(542148);
            break;
        case 824:
            sWorld->SendWorldText(542149);
            break;
        }

        sOutdoorPvPMgr->HandleGameEventStart(event_id);
        return false;
    }
    if (data.state == GAMEEVENT_WORLD_INACTIVE)
        // set to conditions phase
        data.state = GAMEEVENT_WORLD_CONDITIONS;

    // add to active events
    AddActiveEvent(event_id);
    // add spawns
    ApplyNewEvent(event_id);

    // check if can go to next state
    bool conditions_met = CheckOneGameEventConditions(event_id);
    // save to db
    SaveWorldEventStateToDB(event_id);
    // force game event update to set the update timer if conditions were met from a command
    // this update is needed to possibly start events dependent on the started one
    // or to scedule another update where the next event will be started
    if (overwrite && conditions_met)
        sWorld->ForceGameEventUpdate();

    sOutdoorPvPMgr->HandleGameEventStart(event_id);

    // temporally. delete after new year event end
    switch (event_id)
    {
    case 811:
        sWorld->SendWorldText(542143);
        break;
    case 812:
        sWorld->SendWorldText(542137);
        break;
    case 813:
    case 814:
    case 815:
    case 816:
    case 817:
    case 818:
    case 819:
    case 820:
    case 821:
        sWorld->SendWorldText(542138);
        break;
    case 823:
        sWorld->SendWorldText(542148);
        break;
    case 824:
        sWorld->SendWorldText(542149);
        break;
    }

    return conditions_met;
}

void GameEventMgr::StopEvent(uint16 event_id, bool overwrite)
{
    GameEventData &data = mGameEvent[event_id];
    bool serverwide_evt = data.state != GAMEEVENT_NORMAL && data.state != GAMEEVENT_INTERNAL;

    RemoveActiveEvent(event_id);
    UnApplyEvent(event_id);

    if (overwrite && !serverwide_evt)
    {
        data.start = time(nullptr) - data.length * MINUTE;
        if (data.end <= data.start)
            data.end = data.start + data.length;
    }
    else if (serverwide_evt)
    {
        // if finished world event, then only gm command can stop it
        if (overwrite || data.state != GAMEEVENT_WORLD_FINISHED)
        {
            // reset conditions
            data.nextstart = 0;
            data.state = GAMEEVENT_WORLD_INACTIVE;
            for (auto& condition : data.conditions)
                condition.second.done = 0;

            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ALL_GAME_EVENT_CONDITION_SAVE);
            stmt->setUInt16(0, event_id);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GAME_EVENT_SAVE);
            stmt->setUInt16(0, event_id);
            trans->Append(stmt);

            CharacterDatabase.CommitTransaction(trans);
        }
    }

    if (event_id == 308) //Un'goro Madness (Micro-Holiday)
    {
        time_t t = time(nullptr);
        struct tm * now = localtime(&t);
        uint16 new_year = (now->tm_year + 1900) + 1;

        WorldDatabase.PExecute("UPDATE game_event SET end_time='%u-03-19 23:59:00' WHERE eventEntry IN (309,310,311,312);", new_year);
        WorldDatabase.PExecute("UPDATE game_event SET start_time='%u-03-17 00:00:00' WHERE eventEntry = 308;", new_year);
    }
}

void GameEventMgr::LoadFromDB()
{
    {
        uint32 oldMSTime = getMSTime();
        //       1           2                           3                         4          5       6        7            8
        QueryResult result = WorldDatabase.Query("SELECT eventEntry, UNIX_TIMESTAMP(start_time), UNIX_TIMESTAMP(end_time), occurence, length, holiday, description, world_event FROM game_event");
        if (!result)
        {
            mGameEvent.clear();
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 game events. DB table `game_event` is empty.");

            return;
        }

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();

            uint16 event_id = fields[0].GetUInt16();
            if (event_id == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event` game event entry 0 is reserved and can't be used.");
                continue;
            }

            GameEventData& pGameEvent = mGameEvent[event_id];
            uint64 starttime = fields[1].GetUInt64();
            pGameEvent.start = time_t(starttime);
            uint64 endtime = fields[2].GetUInt64();
            pGameEvent.end = time_t(endtime);
            pGameEvent.occurence = fields[3].GetUInt64();
            pGameEvent.length = fields[4].GetUInt64();
            pGameEvent.holiday_id = HolidayIds(fields[5].GetUInt32());

            pGameEvent.state = static_cast<GameEventState>(fields[7].GetUInt8());
            pGameEvent.nextstart = 0;

            if (pGameEvent.length == 0 && pGameEvent.state == GAMEEVENT_NORMAL)                            // length>0 is validity check
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event` game event id (%i) isn't a world event and has length = 0, thus it can't be used.", event_id);
                continue;
            }

            if (pGameEvent.holiday_id != HOLIDAY_NONE)
            {
                if (!sHolidaysStore.LookupEntry(pGameEvent.holiday_id))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event` game event id (%i) have not existed holiday id %u.", event_id, pGameEvent.holiday_id);
                    pGameEvent.holiday_id = HOLIDAY_NONE;
                }
            }

            pGameEvent.description = fields[6].GetString();

            ++count;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Saves Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                       0       1        2
        QueryResult result = CharacterDatabase.Query("SELECT eventEntry, state, next_start FROM game_event_save");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 game event saves in game events. DB table `game_event_save` is empty.");

        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_save` game event entry (%i) is out of range compared to max event entry in `game_event`", event_id);
                    continue;
                }

                if (mGameEvent[event_id].state != GAMEEVENT_NORMAL && mGameEvent[event_id].state != GAMEEVENT_INTERNAL)
                {
                    mGameEvent[event_id].state = static_cast<GameEventState>(fields[1].GetUInt8());
                    mGameEvent[event_id].nextstart = time_t(fields[2].GetUInt32());
                }
                else
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "game_event_save includes event save for non-worldevent id %u", event_id);
                    continue;
                }

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u game event saves in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Prerequisite Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                   0             1
        QueryResult result = WorldDatabase.Query("SELECT eventEntry, prerequisite_event FROM game_event_prerequisite");
        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 game event prerequisites in game events. DB table `game_event_prerequisite` is empty.");

        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_prerequisite` game event id (%i) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                if (mGameEvent[event_id].state != GAMEEVENT_NORMAL && mGameEvent[event_id].state != GAMEEVENT_INTERNAL)
                {
                    uint16 prerequisite_event = fields[1].GetUInt32();
                    if (prerequisite_event >= mGameEvent.size())
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_prerequisite` game event prerequisite id (%i) is out of range compared to max event id in `game_event`", prerequisite_event);
                        continue;
                    }
                    mGameEvent[event_id].prerequisite_events.insert(prerequisite_event);
                }
                else
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "game_event_prerequisiste includes event entry for non-worldevent id %u", event_id);
                    continue;
                }

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u game event prerequisites in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Creature Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                       1                2
        QueryResult result = WorldDatabase.Query("SELECT creature.guid, game_event_creature.eventEntry FROM creature"
            " JOIN game_event_creature ON creature.guid = game_event_creature.guid");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 creatures in game events. DB table `game_event_creature` is empty");

        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                ObjectGuid::LowType guid = fields[0].GetUInt64();
                int16 event_id = fields[1].GetInt16();

                int32 internal_event_id = mGameEvent.size() + event_id - 1;

                if (internal_event_id < 0 || internal_event_id >= int32(mGameEventCreatureGuids.size()))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_creature` game event id (%i) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                GuidList& crelist = mGameEventCreatureGuids[internal_event_id];
                crelist.push_back(guid);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u creatures in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event GO Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                      0                1
        QueryResult result = WorldDatabase.Query("SELECT gameobject.guid, game_event_gameobject.eventEntry FROM gameobject"
            " JOIN game_event_gameobject ON gameobject.guid=game_event_gameobject.guid");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gameobjects in game events. DB table `game_event_gameobject` is empty.");

        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                ObjectGuid::LowType guid = fields[0].GetUInt64();
                int16 event_id = fields[1].GetInt16();

                int32 internal_event_id = mGameEvent.size() + event_id - 1;

                if (internal_event_id < 0 || internal_event_id >= int32(mGameEventGameobjectGuids.size()))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_gameobject` game event id (%i) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                GuidList& golist = mGameEventGameobjectGuids[internal_event_id];
                golist.push_back(guid);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u gameobjects in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Model/Equipment Change Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                       0                     1                              2                               3
        QueryResult result = WorldDatabase.Query("SELECT creature.guid, creature.id, game_event_model_equip.eventEntry, game_event_model_equip.modelid, game_event_model_equip.equipment_id "
            "FROM creature JOIN game_event_model_equip ON creature.guid=game_event_model_equip.guid");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 model/equipment changes in game events. DB table `game_event_model_equip` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                uint16 event_id = fields[2].GetUInt16();

                if (event_id >= mGameEventModelEquip.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_model_equip` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                ModelEquipList& equiplist = mGameEventModelEquip[event_id];
                ModelEquip newModelEquipSet;
                newModelEquipSet.modelid = fields[3].GetUInt32();
                newModelEquipSet.equipment_id = fields[4].GetUInt8();
                newModelEquipSet.equipement_id_prev = 0;
                newModelEquipSet.modelid_prev = 0;

                if (newModelEquipSet.equipment_id > 0)
                {
                    int8 equipId = static_cast<int8>(newModelEquipSet.equipment_id);
                    if (!sObjectMgr->GetEquipmentInfo(entry, equipId))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `game_event_model_equip` have creature (Guid: " UI64FMTD ") with equipment_id %u not found in table `creature_equip_template`, set to no equipment.",
                            guid, newModelEquipSet.equipment_id);
                        continue;
                    }
                }

                equiplist.push_back(std::make_pair(guid, newModelEquipSet));

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u model/equipment changes in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Quest Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                               0     1      2
        QueryResult result = WorldDatabase.Query("SELECT id, quest, eventEntry FROM game_event_creature_quest");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 quests additions in game events. DB table `game_event_creature_quest` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 id = fields[0].GetUInt32();
                uint32 quest = fields[1].GetUInt32();
                uint16 event_id = fields[2].GetUInt16();

                if (event_id >= mGameEventCreatureQuests.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_creature_quest` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                QuestRelList& questlist = mGameEventCreatureQuests[event_id];
                questlist.push_back(QuestRelation(id, quest));

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u quests additions in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event GO Quest Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                               0     1      2
        QueryResult result = WorldDatabase.Query("SELECT id, quest, eventEntry FROM game_event_gameobject_quest");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 go quests additions in game events. DB table `game_event_gameobject_quest` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 id = fields[0].GetUInt32();
                uint32 quest = fields[1].GetUInt32();
                uint16 event_id = fields[2].GetUInt16();

                if (event_id >= mGameEventGameObjectQuests.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_gameobject_quest` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                QuestRelList& questlist = mGameEventGameObjectQuests[event_id];
                questlist.push_back(QuestRelation(id, quest));

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u quests additions in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Quest Condition Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                 0       1         2             3
        QueryResult result = WorldDatabase.Query("SELECT quest, eventEntry, condition_id, num FROM game_event_quest_condition");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 quest event conditions in game events. DB table `game_event_quest_condition` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 quest = fields[0].GetUInt32();
                uint16 event_id = fields[1].GetUInt16();
                uint32 condition = fields[2].GetUInt32();
                float num = fields[3].GetFloat();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_quest_condition` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                mQuestToEventConditions[quest].event_id = event_id;
                mQuestToEventConditions[quest].condition = condition;
                mQuestToEventConditions[quest].num = num;

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u quest event conditions in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Condition Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                  0          1            2             3                      4
        QueryResult result = WorldDatabase.Query("SELECT eventEntry, condition_id, req_num, max_world_state_field, done_world_state_field FROM game_event_condition");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 conditions in game events. DB table `game_event_condition` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();
                uint32 condition = fields[1].GetUInt32();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_condition` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                mGameEvent[event_id].conditions[condition].reqNum = fields[2].GetFloat();
                mGameEvent[event_id].conditions[condition].done = 0;
                mGameEvent[event_id].conditions[condition].max_world_state = fields[3].GetUInt16();
                mGameEvent[event_id].conditions[condition].done_world_state = fields[4].GetUInt16();

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u conditions in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Condition Save Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                      0           1         2
        QueryResult result = CharacterDatabase.Query("SELECT eventEntry, condition_id, done FROM game_event_condition_save");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 condition saves in game events. DB table `game_event_condition_save` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();
                uint32 condition = fields[1].GetUInt32();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_condition_save` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                auto itr = mGameEvent[event_id].conditions.find(condition);
                if (itr != mGameEvent[event_id].conditions.end())
                {
                    itr->second.done = fields[2].GetFloat();
                }
                else
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "game_event_condition_save contains not present condition evt id %u cond id %u", event_id, condition);
                    continue;
                }

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u condition saves in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event NPCflag Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                0       1        2
        QueryResult result = WorldDatabase.Query("SELECT guid, eventEntry, npcflag FROM game_event_npcflag");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 npcflags in game events. DB table `game_event_npcflag` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint16 event_id = fields[1].GetUInt16();
                uint32 npcflag = fields[2].GetUInt32();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_npcflag` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                mGameEventNPCFlags[event_id].emplace_back(guid, npcflag);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u npcflags in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Seasonal Quest Relations...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                  0          1
        QueryResult result = WorldDatabase.Query("SELECT questId, eventEntry FROM game_event_seasonal_questrelation");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 seasonal quests additions in game events. DB table `game_event_seasonal_questrelation` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 questId = fields[0].GetUInt32();
                uint16 eventEntry = fields[1].GetUInt16();

                if (!sQuestDataStore->GetQuestTemplate(questId))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_seasonal_questrelation` quest id (%u) does not exist in `quest_template`", questId);
                    continue;
                }

                if (eventEntry >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_seasonal_questrelation` event id (%u) is out of range compared to max event in `game_event`", eventEntry);
                    continue;
                }

                _questToEventLinks[questId] = eventEntry;
                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u quests additions in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Vendor Additions Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                    0        1    2       3         4          5          6
        QueryResult result = WorldDatabase.Query("SELECT eventEntry, guid, item, maxcount, incrtime, ExtendedCost, type FROM game_event_npc_vendor ORDER BY guid, slot ASC");

        if (!result)
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 vendor additions in game events. DB table `game_event_npc_vendor` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();

                if (event_id >= mGameEventVendors.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_npc_vendor` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                NPCVendorList& vendors = mGameEventVendors[event_id];
                NPCVendorEntry newEntry;
                ObjectGuid::LowType guid = fields[1].GetUInt32();
                newEntry.item = fields[2].GetUInt32();
                newEntry.maxcount = fields[3].GetUInt32();
                newEntry.incrtime = fields[4].GetUInt32();
                newEntry.ExtendedCost = fields[5].GetUInt32();
                newEntry.Type = fields[6].GetUInt8();
                // get the event npc flag for checking if the npc will be vendor during the event or not
                uint32 event_npc_flag = 0;
                NPCFlagList& flist = mGameEventNPCFlags[event_id];
                for (NPCFlagList::const_iterator itr = flist.begin(); itr != flist.end(); ++itr)
                {
                    if (itr->first == guid)
                    {
                        event_npc_flag = itr->second;
                        break;
                    }
                }
                // get creature entry
                newEntry.entry = 0;

                if (CreatureData const* data = sObjectMgr->GetCreatureData(guid))
                    newEntry.entry = data->id;

                // check validity with event's npcflag
                if (!sObjectMgr->IsVendorItemValid(newEntry.entry, newEntry.item, newEntry.maxcount, newEntry.incrtime, newEntry.ExtendedCost, newEntry.Type, nullptr, nullptr, event_npc_flag))
                    continue;

                vendors.push_back(newEntry);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u vendor additions in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event Pool Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                               0                         1
        QueryResult result = WorldDatabase.Query("SELECT pool_template.entry, game_event_pool.eventEntry FROM pool_template"
            " JOIN game_event_pool ON pool_template.entry = game_event_pool.pool_entry");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 pools for game events. DB table `game_event_pool` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32 entry = fields[0].GetUInt32();
                int16 event_id = fields[1].GetInt16();

                int32 internal_event_id = mGameEvent.size() + event_id - 1;

                if (internal_event_id < 0 || internal_event_id >= int32(mGameEventPoolIds.size()))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_pool` game event id (%i) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                if (!sPoolMgr->CheckPool(entry))
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Pool Id (%u) has all creatures or gameobjects with explicit chance sum <>100 and no equal chance defined. The pool system cannot pick one to spawn.", entry);
                    continue;
                }

                IdList& poollist = mGameEventPoolIds[internal_event_id];
                poollist.push_back(entry);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pools for game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event WorldState Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                    0              1           2         3        4
        QueryResult result = WorldDatabase.Query("SELECT `eventEntry`, `WorldStateID`, `Type`, `TypeID`, `Value`  FROM game_event_worldstate");

        if (!result)
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 npcflags in game events. DB table `game_event_worldstate` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_worldstate` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }
                GameEventWorldState gews;
                gews.WorldStateID = fields[1].GetUInt16();
                gews.Type = fields[2].GetUInt8();
                gews.TypeID = fields[3].GetUInt16();
                gews.Value = fields[4].GetUInt32();

                mGameEventWorldState[event_id].push_back(gews);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u WorldState in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
        for (auto& iter : mGameEventWorldState)
            for (auto& itr : iter)
                sWorldStateMgr.SetWorldState(itr.WorldStateID, 0, 0);
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Game Event WorldQuest Data...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                    0           1           2
        QueryResult result = WorldDatabase.Query("SELECT `eventEntry`, `quest`, `VariableID`  FROM game_event_worldquest");

        if (!result)
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 npcflags in game events. DB table `game_event_worldquest` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint16 event_id = fields[0].GetUInt16();

                if (event_id >= mGameEvent.size())
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "`game_event_worldquest` game event id (%u) is out of range compared to max event id in `game_event`", event_id);
                    continue;
                }

                mGameEventWorldQuest[event_id].emplace_back(fields[1].GetUInt32(), fields[2].GetUInt32());

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u WorldQuest in game events in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }
}

uint32 GameEventMgr::GetNPCFlag(Creature* cr)
{
    uint32 mask = 0;
    ObjectGuid::LowType guid = cr->GetDBTableGUIDLow();

    for (auto activeEvent : m_ActiveEvents)
    {
        for (auto& itr : mGameEventNPCFlags[activeEvent])
            if (itr.first == guid)
                mask |= itr.second;
    }

    return mask;
}

void GameEventMgr::Initialize()
{
    QueryResult result = WorldDatabase.Query("SELECT MAX(eventEntry) FROM game_event");
    if (result)
    {
        Field* fields = result->Fetch();

        uint32 maxEventId = fields[0].GetUInt16();

        // Id starts with 1 and vector with 0, thus increment
        maxEventId++;

        mGameEvent.resize(maxEventId);
        mGameEventCreatureGuids.resize(maxEventId * 2 - 1);
        mGameEventGameobjectGuids.resize(maxEventId * 2 - 1);
        mGameEventCreatureQuests.resize(maxEventId);
        mGameEventGameObjectQuests.resize(maxEventId);
        mGameEventVendors.resize(maxEventId);
        mGameEventPoolIds.resize(maxEventId * 2 - 1);
        mGameEventNPCFlags.resize(maxEventId);
        mGameEventModelEquip.resize(maxEventId);
        mGameEventWorldState.resize(maxEventId);
        mGameEventWorldQuest.resize(maxEventId);
    }
}

uint32 GameEventMgr::StartSystem()                           // return the next event delay in ms
{
    m_ActiveEvents.clear();
    uint32 delay = Update();
    isSystemInit = true;

    return delay;
}

void GameEventMgr::StartArenaSeason()
{
    uint8 season = sWorldStateMgr.GetWorldStateValue(WS_ARENA_SEASON_ID);
    QueryResult result = WorldDatabase.PQuery("SELECT eventEntry FROM game_event_arena_seasons WHERE season = '%i'", season);

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "ArenaSeason (%u) must be an existant Arena Season", season);
        return;
    }

    Field* fields = result->Fetch();
    uint16 eventId = fields[0].GetUInt16();

    if (eventId >= mGameEvent.size())
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "EventEntry %u for ArenaSeason (%u) does not exists", eventId, season);
        return;
    }

    StartEvent(eventId, true);
    TC_LOG_INFO(LOG_FILTER_GAMEEVENTS, "Arena Season %u started...", season);

}

uint32 GameEventMgr::Update()                               // return the next event delay in ms
{
    time_t currenttime = time(nullptr);
    uint32 nextEventDelay = max_ge_check_delay;             // 1 day
    std::set<uint16> activate, deactivate;
    for (size_t itr = 1; itr < mGameEvent.size(); ++itr)
    {
        // must do the activating first, and after that the deactivating
        // so first queue it
        //TC_LOG_ERROR(LOG_FILTER_SQL, "Checking event %u", itr);
        if (CheckOneGameEvent(itr))
        {
            // if the world event is in NEXTPHASE state, and the time has passed to finish this event, then do so
            if (mGameEvent[itr].state == GAMEEVENT_WORLD_NEXTPHASE && mGameEvent[itr].nextstart <= currenttime)
            {
                // set this event to finished, null the nextstart time
                mGameEvent[itr].state = GAMEEVENT_WORLD_FINISHED;
                mGameEvent[itr].nextstart = 0;
                // save the state of this gameevent
                SaveWorldEventStateToDB(itr);
                // queue for deactivation
                if (IsActiveEvent(itr))
                    deactivate.insert(itr);
                // go to next event, this no longer needs an event update timer
                continue;
            }
            if (mGameEvent[itr].state == GAMEEVENT_WORLD_CONDITIONS && CheckOneGameEventConditions(itr))
                // changed, save to DB the gameevent state, will be updated in next update cycle
                SaveWorldEventStateToDB(itr);

            //TC_LOG_DEBUG(LOG_FILTER_GENERAL, "GameEvent %u is active", itr->first);
            // queue for activation
            if (!IsActiveEvent(itr))
                activate.insert(itr);
        }
        else
        {
            //TC_LOG_DEBUG(LOG_FILTER_GENERAL, "GameEvent %u is not active", itr->first);
            if (IsActiveEvent(itr))
                deactivate.insert(itr);
            else
            {
                if (!isSystemInit)
                {
                    int16 event_nid = (-1) * (itr);
                    // spawn all negative ones for this event
                    GameEventSpawn(event_nid);
                }
            }
        }
        uint32 calcDelay = NextCheck(itr);
        if (calcDelay < nextEventDelay)
            nextEventDelay = calcDelay;
    }
    // now activate the queue
    // a now activated event can contain a spawn of a to-be-deactivated one
    // following the activate - deactivate order, deactivating the first event later will leave the spawn in (wont disappear then reappear clientside)
    // start the event
    // returns true the started event completed
    // in that case, initiate next update in 1 second
    for (auto itr : activate)
        if (StartEvent(itr))
            nextEventDelay = 0;

    for (auto itr : deactivate)
        StopEvent(itr);

    TC_LOG_INFO(LOG_FILTER_GAMEEVENTS, "Next game event check in %u seconds.", nextEventDelay + 1);
    return (nextEventDelay + 1) * IN_MILLISECONDS;           // Add 1 second to be sure event has started/stopped at next call
}

bool GameEventMgr::IsActiveEvent(uint16 event_id)
{
    return (m_ActiveEvents.find(event_id) != m_ActiveEvents.end());
}

void GameEventMgr::UnApplyEvent(uint16 event_id)
{
    TC_LOG_INFO(LOG_FILTER_GAMEEVENTS, "GameEvent %u \"%s\" removed.", event_id, mGameEvent[event_id].description.c_str());
    // un-spawn positive event tagged objects
    GameEventUnspawn(event_id);
    // spawn negative event tagget objects
    int16 event_nid = (-1) * event_id;
    GameEventSpawn(event_nid);
    // restore equipment or model
    ChangeEquipOrModel(event_id, false);
    // Remove quests that are events only to non event npc
    UpdateEventQuests(event_id, false);
    // update npcflags in this event
    UpdateEventNPCFlags(event_id);
    // remove vendor items
    UpdateEventNPCVendor(event_id, false);
    // disable worldstate
    UpdateEventWorldState(event_id, false);
    // disable worldquest
    UpdateEventWorldQuest(event_id, false);
}

void GameEventMgr::ApplyNewEvent(uint16 event_id)
{
    switch (sWorld->getIntConfig(CONFIG_EVENT_ANNOUNCE))
    {
    case 0:                                             // disable
        break;
    case 1:                                             // announce events
        sWorld->SendWorldText(LANG_EVENTMESSAGE, mGameEvent[event_id].description.c_str());
        break;
    default:
        break;
    }

    TC_LOG_INFO(LOG_FILTER_GAMEEVENTS, "GameEvent %u \"%s\" started.", event_id, mGameEvent[event_id].description.c_str());

    // spawn positive event tagget objects
    GameEventSpawn(event_id);
    // un-spawn negative event tagged objects
    int16 event_nid = (-1) * event_id;
    GameEventUnspawn(event_nid);
    // Change equipement or model
    ChangeEquipOrModel(event_id, true);
    // Add quests that are events only to non event npc
    UpdateEventQuests(event_id, true);
    // update npcflags in this event
    UpdateEventNPCFlags(event_id);
    // add vendor items
    UpdateEventNPCVendor(event_id, true);
    // enable worldstate
    UpdateEventWorldState(event_id, true);
    // enable worldquest
    UpdateEventWorldQuest(event_id, true);
    // check for seasonal quest reset.
    sWorld->ResetEventSeasonalQuests(event_id);
}

void GameEventMgr::UpdateEventNPCFlags(uint16 event_id)
{
    // go through the creatures whose npcflags are changed in the event
    for (auto& itr : mGameEventNPCFlags[event_id])
    {
        // get the creature data from the low guid to get the entry, to be able to find out the whole guid
        if (CreatureData const* data = sObjectMgr->GetCreatureData(itr.first))
        {
            // if we found the creature, modify its npcflag
            if (Creature* creature = ObjectAccessor::GetObjectInWorld(ObjectGuid::Create<HighGuid::Creature>(data->mapid, data->id, itr.first), static_cast<Creature*>(nullptr)))
            {
                uint32 npcflag = GetNPCFlag(creature);
                uint32 npcflag2 = 0;

                if (const CreatureTemplate* ci = creature->GetCreatureTemplate())
                {
                    npcflag |= ci->npcflag;
                    npcflag2 |= ci->npcflag2;
                }

                creature->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, npcflag);
                creature->SetUInt32Value(UNIT_FIELD_NPC_FLAGS + 1, npcflag2);
            }
        }
    }
}

void GameEventMgr::UpdateEventNPCVendor(uint16 event_id, bool activate)
{
    for (auto& itr : mGameEventVendors[event_id])
    {
        if (activate)
            sObjectMgr->AddVendorItem(itr.entry, itr.item, itr.maxcount, itr.incrtime, itr.ExtendedCost, itr.Type, 0, false);
        else
            sObjectMgr->RemoveVendorItem(itr.entry, itr.item, itr.Type, false);
    }
}

void GameEventMgr::UpdateEventWorldState(uint16 event_id, bool activate)
{
    for (auto& itr : mGameEventWorldState[event_id])
        sWorldStateMgr.SetWorldState(itr.WorldStateID, 0, activate ? itr.Value : 0);
}

void GameEventMgr::UpdateEventWorldQuest(uint16 event_id, bool activate)
{
    for (auto& itr : mGameEventWorldQuest[event_id])
    {
        if (activate)
            sQuestDataStore->GenerateNewWorldQuest(itr.first, itr.second);
        else
            sQuestDataStore->ResetWorldQuest(itr.first);
    }
}

void GameEventMgr::GameEventSpawn(int16 event_id)
{
    int32 internal_event_id = mGameEvent.size() + event_id - 1;

    if (internal_event_id < 0 || internal_event_id >= int32(mGameEventCreatureGuids.size()))
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "GameEventMgr::GameEventSpawn attempt access to out of range mGameEventCreatureGuids element %i (size: %u)",
            internal_event_id, mGameEventCreatureGuids.size());
        return;
    }

    std::set<uint32> NeedReSpawnPool;
    NeedReSpawnPool.clear();

    for (auto& itr : mGameEventCreatureGuids[internal_event_id])
    {
        // Add to correct cell
        if (CreatureData const* data = sObjectMgr->GetCreatureData(itr))
        {
            if (data->pool)
            {
                NeedReSpawnPool.insert(data->pool);
                continue;
            }

            sObjectMgr->AddCreatureToGrid(itr, data);

            // Spawn if necessary (loaded grids only)
            MapEntry const* entry = sMapStore.LookupEntry(data->mapid);
            if (!entry)
                continue;

            Map* map = entry->CanCreatedZone() ? sMapMgr->FindMap(data->mapid, data->zoneId) : sMapMgr->CreateBaseMap(data->mapid);
            if (!map)
                continue;

            // We use spawn coords to spawn
            if (!map->Instanceable() && map->IsGridLoaded(data->posX, data->posY))
            {
                auto* creature = new Creature;
                // TC_LOG_DEBUG(LOG_FILTER_GAMEEVENTS, "Spawning creature %u", *itr);
                if (!creature->LoadCreatureFromDB(itr, map))
                    delete creature;
            }
        }
    }

    if (internal_event_id < 0 || internal_event_id >= int32(mGameEventGameobjectGuids.size()))
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "GameEventMgr::GameEventSpawn attempt access to out of range mGameEventGameobjectGuids element %i (size: %u)",
            internal_event_id, mGameEventGameobjectGuids.size());
        return;
    }

    for (auto& itr : mGameEventGameobjectGuids[internal_event_id])
    {
        // Add to correct cell
        if (GameObjectData const* data = sObjectMgr->GetGOData(itr))
        {
            if (data->pool)
            {
                NeedReSpawnPool.insert(data->pool);
                continue;
            }

            sObjectMgr->AddGameobjectToGrid(itr, data);
            // Spawn if necessary (loaded grids only)
            // this base map checked as non-instanced and then only existed
            MapEntry const* entry = sMapStore.LookupEntry(data->mapid);
            if (!entry)
                continue;

            Map* map = entry->CanCreatedZone() ? sMapMgr->FindMap(data->mapid, data->zoneId) : sMapMgr->CreateBaseMap(data->mapid);
            if (!map)
                continue;

            // We use current coords to unspawn, not spawn coords since creature can have changed grid
            if (!map->Instanceable() && map->IsGridLoaded(data->posX, data->posY))
            {
                GameObject* pGameobject = sObjectMgr->IsStaticTransport(data->id) ? new StaticTransport : new GameObject;
                //TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Spawning gameobject %u", *itr);
                //TODO: find out when it is add to map
                if (!pGameobject->LoadGameObjectFromDB(itr, map, false))
                    delete pGameobject;
                else
                {
                    if (pGameobject->isSpawnedByDefault())
                        map->AddToMapWait(pGameobject);
                }
            }
        }
    }

    if (internal_event_id < 0 || internal_event_id >= int32(mGameEventPoolIds.size()))
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "GameEventMgr::GameEventSpawn attempt access to out of range mGameEventPoolIds element %u (size: %u)",
            internal_event_id, mGameEventPoolIds.size());
        return;
    }

    for (auto& itr : mGameEventPoolIds [internal_event_id])
        sPoolMgr->SpawnPool(itr);

    for (uint32 pool : NeedReSpawnPool)
        sPoolMgr->SpawnPool(pool);
}

void GameEventMgr::GameEventUnspawn(int16 event_id)
{
    int32 internal_event_id = mGameEvent.size() + event_id - 1;

    if (internal_event_id < 0 || internal_event_id >= int32(mGameEventCreatureGuids.size()))
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventCreatureGuids element %i (size: %u)",
            internal_event_id, mGameEventCreatureGuids.size());
        return;
    }

    std::set<uint32> NeedReSpawnPool;
    NeedReSpawnPool.clear();
    for (auto& itr : mGameEventCreatureGuids[internal_event_id])
    {
        // check if it's needed by another event, if so, don't remove
        if (event_id > 0 && hasCreatureActiveEventExcept(itr, event_id))
            continue;
        // Remove the creature from grid
        if (CreatureData const* data = sObjectMgr->GetCreatureData(itr))
        {
            if (data->pool)
            {
                NeedReSpawnPool.insert(data->pool);
                continue;
            }

            sObjectMgr->RemoveCreatureFromGrid(itr, data);

            if (Creature* creature = ObjectAccessor::GetObjectInWorld(ObjectGuid::Create<HighGuid::Creature>(data->mapid, data->id, itr), static_cast<Creature*>(nullptr)))
                creature->AddDelayedEvent(10, [creature]() -> void {if (creature) creature->AddObjectToRemoveList(); });
        }
    }

    if (event_id > 0)
    {
        for (auto& itr : mGameEventCreatureSpawns[event_id])
        {
            // Remove the creature from world
            if (Creature* creature = ObjectAccessor::GetObjectInWorld(itr, static_cast<Creature*>(nullptr)))
                creature->AddDelayedEvent(10, [creature]() -> void {if (creature) creature->AddObjectToRemoveList(); });
        }
        mGameEventCreatureSpawns[event_id].clear();
    }

    if (internal_event_id < 0 || internal_event_id >= int32(mGameEventGameobjectGuids.size()))
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventGameobjectGuids element %i (size: %u)",
            internal_event_id, mGameEventGameobjectGuids.size());
        return;
    }

    for (auto& itr : mGameEventGameobjectGuids[internal_event_id])
    {
        // check if it's needed by another event, if so, don't remove
        if (event_id > 0 && hasGameObjectActiveEventExcept(itr, event_id))
            continue;
        // Remove the gameobject from grid
        if (GameObjectData const* data = sObjectMgr->GetGOData(itr))
        {
            if (data->pool)
            {
                NeedReSpawnPool.insert(data->pool);
                continue;
            }

            sObjectMgr->RemoveGameobjectFromGrid(itr, data);

            if (GameObject* pGameobject = ObjectAccessor::GetObjectInWorld(ObjectGuid::Create<HighGuid::GameObject>(data->mapid, data->id, itr), static_cast<GameObject*>(nullptr)))
                pGameobject->AddDelayedEvent(10, [pGameobject]() -> void {if (pGameobject) pGameobject->AddObjectToRemoveList(); });
        }
    }

    if (event_id > 0)
    {
        for (auto& itr : mGameEventGameobjectSpawns[event_id])
        {
            // Remove the gameobject from world
            if (GameObject* pGameobject = ObjectAccessor::GetObjectInWorld(itr, static_cast<GameObject*>(nullptr)))
                pGameobject->AddDelayedEvent(10, [pGameobject]() -> void {if (pGameobject) pGameobject->AddObjectToRemoveList(); });
        }
        mGameEventGameobjectSpawns[event_id].clear();
    }

    if (internal_event_id < 0 || internal_event_id >= int32(mGameEventPoolIds.size()))
    {
        TC_LOG_ERROR(LOG_FILTER_GAMEEVENTS, "GameEventMgr::GameEventUnspawn attempt access to out of range mGameEventPoolIds element %u (size: %u)", internal_event_id, mGameEventPoolIds.size());
        return;
    }

    for (auto& itr : mGameEventPoolIds [internal_event_id])
    {
        sPoolMgr->DespawnPool(itr);
    }

    for (uint32 pool : NeedReSpawnPool) // respawn
        sPoolMgr->DespawnPool(pool);
}

void GameEventMgr::ChangeEquipOrModel(int16 event_id, bool activate)
{
    for (auto& itr : mGameEventModelEquip[event_id])
    {
        // Remove the creature from grid
        CreatureData const* data = sObjectMgr->GetCreatureData(itr.first);
        if (!data)
            continue;

        // Update if spawned
        if (Creature* creature = ObjectAccessor::GetObjectInWorld(ObjectGuid::Create<HighGuid::Creature>(data->mapid, data->id, itr.first), static_cast<Creature*>(nullptr)))
        {
            if (activate)
            {
                itr.second.equipement_id_prev = creature->GetCurrentEquipmentId();
                itr.second.modelid_prev = creature->GetDisplayId();
                creature->LoadEquipment(itr.second.equipment_id, true);
                if (itr.second.modelid > 0 && itr.second.modelid_prev != itr.second.modelid)
                {
                    CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelInfo(itr.second.modelid);
                    if (minfo)
                    {
                        creature->SetDisplayId(itr.second.modelid);
                        creature->SetNativeDisplayId(itr.second.modelid);
                        creature->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, minfo->bounding_radius);
                        creature->SetFloatValue(UNIT_FIELD_COMBAT_REACH, minfo->combat_reach);
                    }
                }
            }
            else
            {
                creature->LoadEquipment(itr.second.equipement_id_prev, true);
                if (itr.second.modelid_prev > 0 && itr.second.modelid_prev != itr.second.modelid)
                {
                    CreatureModelInfo const* minfo = sObjectMgr->GetCreatureModelInfo(itr.second.modelid_prev);
                    if (minfo)
                    {
                        creature->SetDisplayId(itr.second.modelid_prev);
                        creature->SetNativeDisplayId(itr.second.modelid_prev);
                        creature->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, minfo->bounding_radius);
                        creature->SetFloatValue(UNIT_FIELD_COMBAT_REACH, minfo->combat_reach);
                    }
                }
            }
        }
        // now last step: put in data
                                                            // just to have write access to it
        CreatureData& data2 = sObjectMgr->NewOrExistCreatureData(itr.first);
        if (activate)
        {
            data2.displayid = itr.second.modelid;
            data2.equipmentId = itr.second.equipment_id;
        }
        else
        {
            data2.displayid = itr.second.modelid_prev;
            data2.equipmentId = itr.second.equipement_id_prev;
        }
    }
}

bool GameEventMgr::hasCreatureQuestActiveEventExcept(uint32 quest_id, uint16 event_id)
{
    for (auto activeEvent : m_ActiveEvents)
    {
        if (activeEvent != event_id)
            for (auto& itr : mGameEventCreatureQuests[activeEvent])
                if (itr.second == quest_id)
                    return true;
    }
    return false;
}

bool GameEventMgr::hasGameObjectQuestActiveEventExcept(uint32 quest_id, uint16 event_id)
{
    for (auto activeEvent : m_ActiveEvents)
    {
        if (activeEvent != event_id)
            for (auto& itr : mGameEventGameObjectQuests[activeEvent])
                if (itr.second == quest_id)
                    return true;
    }
    return false;
}

bool GameEventMgr::hasCreatureActiveEventExcept(ObjectGuid::LowType const& creature_id, uint16 event_id)
{
    for (auto activeEvent : m_ActiveEvents)
    {
        if (activeEvent != event_id)
        {
            int32 internal_event_id = mGameEvent.size() + activeEvent - 1;
            for (auto& itr : mGameEventCreatureGuids[internal_event_id])
                if (itr == creature_id)
                    return true;
        }
    }
    return false;
}
bool GameEventMgr::hasGameObjectActiveEventExcept(ObjectGuid::LowType const& go_id, uint16 event_id)
{
    for (auto activeEvent : m_ActiveEvents)
    {
        if (activeEvent != event_id)
        {
            int32 internal_event_id = mGameEvent.size() + activeEvent - 1;
            for (auto& itr : mGameEventGameobjectGuids[internal_event_id])
                if (itr == go_id)
                    return true;
        }
    }
    return false;
}

void GameEventMgr::UpdateEventQuests(uint16 event_id, bool activate)
{
    QuestRelList::iterator itr;
    for (itr = mGameEventCreatureQuests[event_id].begin(); itr != mGameEventCreatureQuests[event_id].end(); ++itr)
    {
        QuestRelations* CreatureQuestMap = sQuestDataStore->GetCreatureQuestRelationMap();
        if (activate)                                           // Add the pair(id, quest) to the multimap
            CreatureQuestMap->insert(std::make_pair(itr->first, itr->second));
        else
        {
            if (!hasCreatureQuestActiveEventExcept(itr->second, event_id))
            {
                // Remove the pair(id, quest) from the multimap
                auto qitr = CreatureQuestMap->find(itr->first);
                if (qitr == CreatureQuestMap->end())
                    continue;
                auto lastElement = CreatureQuestMap->upper_bound(itr->first);
                for (; qitr != lastElement; ++qitr)
                {
                    if (qitr->second == itr->second)
                    {
                        CreatureQuestMap->erase(qitr);          // iterator is now no more valid
                        break;                                  // but we can exit loop since the element is found
                    }
                }
            }
        }
    }
    for (itr = mGameEventGameObjectQuests[event_id].begin(); itr != mGameEventGameObjectQuests[event_id].end(); ++itr)
    {
        QuestRelations* GameObjectQuestMap = sQuestDataStore->GetGOQuestRelationMap();
        if (activate)                                           // Add the pair(id, quest) to the multimap
            GameObjectQuestMap->insert(std::make_pair(itr->first, itr->second));
        else
        {
            if (!hasGameObjectQuestActiveEventExcept(itr->second, event_id))
            {
                // Remove the pair(id, quest) from the multimap
                auto qitr = GameObjectQuestMap->find(itr->first);
                if (qitr == GameObjectQuestMap->end())
                    continue;
                auto lastElement = GameObjectQuestMap->upper_bound(itr->first);
                for (; qitr != lastElement; ++qitr)
                {
                    if (qitr->second == itr->second)
                    {
                        GameObjectQuestMap->erase(qitr);        // iterator is now no more valid
                        break;                                  // but we can exit loop since the element is found
                    }
                }
            }
        }
    }
}

GameEventData::GameEventData() : start(1), end(0), nextstart(0), occurence(0), length(0), holiday_id(HOLIDAY_NONE), state(GAMEEVENT_NORMAL)
{
}

bool GameEventData::isValid() const
{
    return length > 0 || state > GAMEEVENT_NORMAL;
}

GameEventMgr::GameEventMgr() : isSystemInit(false)
{
}

GameEventMgr* GameEventMgr::instance()
{
    static GameEventMgr instance;
    return &instance;
}

void GameEventMgr::HandleQuestComplete(uint32 quest_id)
{
    // translate the quest to event and condition
    auto itr = mQuestToEventConditions.find(quest_id);
    // quest is registered
    if (itr != mQuestToEventConditions.end())
    {
        uint16 event_id = itr->second.event_id;
        uint32 condition = itr->second.condition;
        float num = itr->second.num;

        // the event is not active, so return, don't increase condition finishes
        if (!IsActiveEvent(event_id))
            return;

        // not in correct phase, return
        if (mGameEvent[event_id].state != GAMEEVENT_WORLD_CONDITIONS)
            return;

        auto citr = mGameEvent[event_id].conditions.find(condition);
        // condition is registered
        if (citr != mGameEvent[event_id].conditions.end())
        {
            // increase the done count, only if less then the req
            if (citr->second.done < citr->second.reqNum)
            {
                citr->second.done += num;
                // check max limit
                if (citr->second.done > citr->second.reqNum)
                    citr->second.done = citr->second.reqNum;
                // save the change to db
                SQLTransaction trans = CharacterDatabase.BeginTransaction();

                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GAME_EVENT_CONDITION_SAVE);
                stmt->setUInt16(0, event_id);
                stmt->setUInt32(1, condition);
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GAME_EVENT_CONDITION_SAVE);
                stmt->setUInt16(0, event_id);
                stmt->setUInt32(1, condition);
                stmt->setFloat(2, citr->second.done);
                trans->Append(stmt);
                CharacterDatabase.CommitTransaction(trans);
                // check if all conditions are met, if so, update the event state
                if (CheckOneGameEventConditions(event_id))
                {
                    // changed, save to DB the gameevent state
                    SaveWorldEventStateToDB(event_id);
                    // force update events to set timer
                    sWorld->ForceGameEventUpdate();
                }
            }
        }
    }
}

bool GameEventMgr::CheckOneGameEventConditions(uint16 event_id)
{
    for (GameEventConditionMap::const_iterator itr = mGameEvent[event_id].conditions.begin(); itr != mGameEvent[event_id].conditions.end(); ++itr)
        if (itr->second.done < itr->second.reqNum)
            // return false if a condition doesn't match
            return false;
    // set the phase
    mGameEvent[event_id].state = GAMEEVENT_WORLD_NEXTPHASE;
    // set the followup events' start time
    if (!mGameEvent[event_id].nextstart)
    {
        time_t currenttime = time(nullptr);
        mGameEvent[event_id].nextstart = currenttime + mGameEvent[event_id].length * 60;
    }
    return true;
}

void GameEventMgr::SaveWorldEventStateToDB(uint16 event_id)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GAME_EVENT_SAVE);
    stmt->setUInt16(0, event_id);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GAME_EVENT_SAVE);
    stmt->setUInt16(0, event_id);
    stmt->setUInt8(1, mGameEvent[event_id].state);
    stmt->setUInt32(2, mGameEvent[event_id].nextstart ? uint32(mGameEvent[event_id].nextstart) : 0);
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);
}

void GameEventMgr::SendWorldStateUpdate(Player* player, uint16 event_id)
{
    for (GameEventConditionMap::const_iterator itr = mGameEvent[event_id].conditions.begin(); itr != mGameEvent[event_id].conditions.end(); ++itr)
    {
        if (itr->second.done_world_state)
            player->SendUpdateWorldState(itr->second.done_world_state, static_cast<uint32>(itr->second.done));
        if (itr->second.max_world_state)
            player->SendUpdateWorldState(itr->second.max_world_state, static_cast<uint32>(itr->second.reqNum));
    }
}

void GameEventMgr::AddActiveEvent(uint16 event_id)
{
    m_ActiveEvents.insert(event_id);
}

void GameEventMgr::RemoveActiveEvent(uint16 event_id)
{
    m_ActiveEvents.erase(event_id);
}

uint16 GameEventMgr::GetEventIdForQuest(Quest const* quest) const
{
    if (!quest)
        return 0;

    auto itr = _questToEventLinks.find(quest->GetQuestId());
    if (itr == _questToEventLinks.end())
        return 0;

    return itr->second;
}

bool IsHolidayActive(HolidayIds id)
{
    if (id == HOLIDAY_NONE)
        return false;

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr->GetEventMap();
    GameEventMgr::ActiveEvents const& ae = sGameEventMgr->GetActiveEventList();

    for (auto itr : ae)
        if (events[itr].holiday_id == id)
            return true;

    return false;
}

bool IsEventActive(uint16 event_id)
{
    GameEventMgr::ActiveEvents const& ae = sGameEventMgr->GetActiveEventList();
    return ae.find(event_id) != ae.end();
}

bool GameEventMgr::IsHolidayActive(int32 holidayID, uint32& expirationTime) const
{
    /// WorldStateExpressionFunctions::HolidayActive
    auto entry = sHolidaysStore.LookupEntry(holidayID);
    if (!entry)
        return false;

    int i = 0;
    int currentDuration = 0;

    auto durations = entry->Duration;
    while (i <= 0 || *durations)
    {
        currentDuration = *durations;
        if (!currentDuration)
            currentDuration = 24;

        auto holidayStart = [=]()->int
        {
            if (i > 0 && !entry->Duration[i])
                return 0;

            int choosedDuration = entry->Duration[i];
            if (!choosedDuration)
                choosedDuration = 24;

            time_t l_CurrentTime = sWorld->GetGameTime();
            struct tm l_LocalTime;
            l_LocalTime.tm_isdst = -1;

            localtime_r(&l_CurrentTime, &l_LocalTime);

            MS::Utilities::WowTime gameTime;
            gameTime.SetUTCTimeFromPosixTime(l_CurrentTime);
            gameTime.YearDay = l_LocalTime.tm_yday;

            for (uint32 valueD : entry->Duration)
            {
                MS::Utilities::WowTime time;
                time.Decode(valueD);

                if (entry->Flags & 1)
                {
                    time.YearDay = gameTime.YearDay;
                    time.ComputeRegionTime(time);
                }

                if (time.Minute < 0)
                    time.Minute = gameTime.Minute;
                if (time.Hour < 0)
                    time.Hour = gameTime.Hour;
                if (time.MonthDay < 0)
                    time.MonthDay = gameTime.Minute;
                if (time.Month < 0)
                    time.Month = gameTime.Month;
                if (time.Year < 0)
                {
                    time.Year = gameTime.Year;

                    if (gameTime < time)
                        --time.Year;
                }

                if (entry->Looping)
                {
                    int32 l_i3 = 0;
                    int32 v7 = 0;
                    int32 v8 = 0;
                    do
                    {
                        if (l_i3 >= i)
                        {
                            if (l_i3 > i)
                                v8 += entry->Duration[l_i3];
                        }
                        else
                        {
                            v7 += entry->Duration[l_i3];
                            if (!l_i3 && !entry->Duration[0])
                                v7 += 24;
                        }
                        ++l_i3;
                    } while (l_i3 < std::extent<decltype(entry->Duration)>::value);

                    while (!(gameTime <= time))
                    {
                        time.AddHolidayDuration(MS::Utilities::Globals::InMinutes::Hour * v7);
                        time.AddHolidayDuration(choosedDuration * MS::Utilities::Globals::InMinutes::Hour);
                        if (!(gameTime >= time))
                            return static_cast<int32>(MS::Utilities::Globals::InMinutes::Day * (time.GetDaysSinceEpoch() - gameTime.GetDaysSinceEpoch()) - gameTime.GetHourAndMinutes() + time.GetHourAndMinutes());

                        time.AddHolidayDuration(MS::Utilities::Globals::InMinutes::Hour * v8);
                    }
                    return choosedDuration * MS::Utilities::Globals::InMinutes::Hour;
                }

                if (gameTime <= time)
                {
                    for (uint32 value : entry->Duration)
                    {
                        if (!value)
                            value = 24;

                        time.AddHolidayDuration(value * MS::Utilities::Globals::InMinutes::Hour);
                    }

                    time.AddHolidayDuration(choosedDuration * MS::Utilities::Globals::InMinutes::Hour);

                    if (!(gameTime >= time))
                        return static_cast<int32>(time.DiffTime(gameTime) / MS::Utilities::Globals::InMinutes::Hour);
                }
            }

            return choosedDuration * MS::Utilities::Globals::InMinutes::Hour;
        }();

        if (holidayStart < MS::Utilities::Globals::InMinutes::Hour * currentDuration)
        {
            expirationTime = (MS::Utilities::Globals::InMinutes::Hour * currentDuration) - holidayStart;
            return true;
        }

        ++i;
        durations += 4;

        if (i >= std::extent<decltype(entry->Duration)>::value)
            return false;
    }
    return false;
}

uint32 GameEventMgr::GetCountOfRepeatEvent(uint32 event) const
{
    time_t currenttime = time(nullptr);
    return ceil(float(currenttime - mGameEvent[event].start) / float(mGameEvent[event].occurence * MINUTE));
}
