#include "ScriptsData.h"
#include "Timer.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "DatabaseEnv.h"
#include "QuestData.h"
#include "SpellScript.h"

ScriptMapMap sQuestEndScripts;
ScriptMapMap sQuestStartScripts;
ScriptMapMap sSpellScripts;
ScriptMapMap sGameObjectScripts;
ScriptMapMap sEventScripts;
ScriptMapMap sWaypointScripts;

std::string GetScriptsTableNameByType(ScriptsType type)
{
    std::string res;
    switch (type)
    {
    case SCRIPTS_QUEST_END:     res = "quest_end_scripts";  break;
    case SCRIPTS_QUEST_START:   res = "quest_start_scripts"; break;
    case SCRIPTS_SPELL:         res = "spell_scripts";      break;
    case SCRIPTS_GAMEOBJECT:    res = "gameobject_scripts"; break;
    case SCRIPTS_EVENT:         res = "event_scripts";      break;
    case SCRIPTS_WAYPOINT:      res = "waypoint_scripts";   break;
    default: break;
    }
    return res;
}

ScriptMapMap* GetScriptsMapByType(ScriptsType type)
{
    ScriptMapMap* res = nullptr;
    switch (type)
    {
    case SCRIPTS_QUEST_END:     res = &sQuestEndScripts;    break;
    case SCRIPTS_QUEST_START:   res = &sQuestStartScripts;  break;
    case SCRIPTS_SPELL:         res = &sSpellScripts;       break;
    case SCRIPTS_GAMEOBJECT:    res = &sGameObjectScripts;  break;
    case SCRIPTS_EVENT:         res = &sEventScripts;       break;
    case SCRIPTS_WAYPOINT:      res = &sWaypointScripts;    break;
    default: break;
    }
    return res;
}

std::string GetScriptCommandName(ScriptCommands command)
{
    std::string res;
    switch (command)
    {
    case SCRIPT_COMMAND_TALK: res = "SCRIPT_COMMAND_TALK"; break;
    case SCRIPT_COMMAND_EMOTE: res = "SCRIPT_COMMAND_EMOTE"; break;
    case SCRIPT_COMMAND_FIELD_SET: res = "SCRIPT_COMMAND_FIELD_SET"; break;
    case SCRIPT_COMMAND_MOVE_TO: res = "SCRIPT_COMMAND_MOVE_TO"; break;
    case SCRIPT_COMMAND_FLAG_SET: res = "SCRIPT_COMMAND_FLAG_SET"; break;
    case SCRIPT_COMMAND_FLAG_REMOVE: res = "SCRIPT_COMMAND_FLAG_REMOVE"; break;
    case SCRIPT_COMMAND_TELEPORT_TO: res = "SCRIPT_COMMAND_TELEPORT_TO"; break;
    case SCRIPT_COMMAND_QUEST_EXPLORED: res = "SCRIPT_COMMAND_QUEST_EXPLORED"; break;
    case SCRIPT_COMMAND_KILL_CREDIT: res = "SCRIPT_COMMAND_KILL_CREDIT"; break;
    case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT: res = "SCRIPT_COMMAND_RESPAWN_GAMEOBJECT"; break;
    case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE: res = "SCRIPT_COMMAND_TEMP_SUMMON_CREATURE"; break;
    case SCRIPT_COMMAND_OPEN_DOOR: res = "SCRIPT_COMMAND_OPEN_DOOR"; break;
    case SCRIPT_COMMAND_CLOSE_DOOR: res = "SCRIPT_COMMAND_CLOSE_DOOR"; break;
    case SCRIPT_COMMAND_ACTIVATE_OBJECT: res = "SCRIPT_COMMAND_ACTIVATE_OBJECT"; break;
    case SCRIPT_COMMAND_REMOVE_AURA: res = "SCRIPT_COMMAND_REMOVE_AURA"; break;
    case SCRIPT_COMMAND_CAST_SPELL: res = "SCRIPT_COMMAND_CAST_SPELL"; break;
    case SCRIPT_COMMAND_PLAY_SOUND: res = "SCRIPT_COMMAND_PLAY_SOUND"; break;
    case SCRIPT_COMMAND_CREATE_ITEM: res = "SCRIPT_COMMAND_CREATE_ITEM"; break;
    case SCRIPT_COMMAND_DESPAWN_SELF: res = "SCRIPT_COMMAND_DESPAWN_SELF"; break;
    case SCRIPT_COMMAND_LOAD_PATH: res = "SCRIPT_COMMAND_LOAD_PATH"; break;
    case SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT: res = "SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT"; break;
    case SCRIPT_COMMAND_KILL: res = "SCRIPT_COMMAND_KILL"; break;
    case SCRIPT_COMMAND_ORIENTATION: res = "SCRIPT_COMMAND_ORIENTATION"; break;
    case SCRIPT_COMMAND_EQUIP: res = "SCRIPT_COMMAND_EQUIP"; break;
    case SCRIPT_COMMAND_MODEL: res = "SCRIPT_COMMAND_MODEL"; break;
    case SCRIPT_COMMAND_CLOSE_GOSSIP: res = "SCRIPT_COMMAND_CLOSE_GOSSIP"; break;
    case SCRIPT_COMMAND_PLAYMOVIE: res = "SCRIPT_COMMAND_PLAYMOVIE"; break;
    case SCRIPT_COMMAND_PLAYSCENE: res = "SCRIPT_COMMAND_PLAYSCENE"; break;
    case SCRIPT_COMMAND_STOPSCENE: res = "SCRIPT_COMMAND_STOPSCENE"; break;
    default:
    {
        char sz[32];
        sprintf(sz, "Unknown command: %u", command);
        res = sz;
        break;
    }
    }
    return res;
}

std::string ScriptInfo::GetDebugInfo() const
{
    char sz[256];
    sprintf(sz, "%s ('%s' script id: %u)", GetScriptCommandName(command).c_str(), GetScriptsTableNameByType(type).c_str(), id);
    return std::string(sz);
}

ScriptDataStoreMgr::ScriptDataStoreMgr() = default;

ScriptDataStoreMgr::~ScriptDataStoreMgr() = default;

ScriptDataStoreMgr* ScriptDataStoreMgr::instance()
{
    static ScriptDataStoreMgr instance;
    return &instance;
}

void ScriptDataStoreMgr::CheckScripts(ScriptsType type, std::set<int32>& ids)
{
    ScriptMapMap* scripts = GetScriptsMapByType(type);
    if (!scripts)
        return;

    for (ScriptMapMap::const_iterator itrMM = scripts->begin(); itrMM != scripts->end(); ++itrMM)
    {
        for (ScriptMap::const_iterator itrM = itrMM->second.begin(); itrM != itrMM->second.end(); ++itrM)
        {
            switch (itrM->second.command)
            {
            case SCRIPT_COMMAND_TALK:
            {
                if (!sObjectMgr->GetTrinityStringLocale(itrM->second.Talk.TextID))
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` references invalid text id %u from `db_script_string`, script id: %u.", GetScriptsTableNameByType(type).c_str(), itrM->second.Talk.TextID, itrMM->first);

                if (ids.find(itrM->second.Talk.TextID) != ids.end())
                    ids.erase(itrM->second.Talk.TextID);
            }
            default:
                break;
            }
        }
    }
}

void ScriptDataStoreMgr::LoadDbScriptStrings()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading Scripts text locales...");

    LoadTrinityStrings("db_script_string", MIN_DB_SCRIPT_STRING_ID, MAX_DB_SCRIPT_STRING_ID);

    std::set<int32> ids;

    for (int32 i = MIN_DB_SCRIPT_STRING_ID; i < MAX_DB_SCRIPT_STRING_ID; ++i)
        if (sObjectMgr->GetTrinityStringLocale(i))
            ids.insert(i);

    for (int type = SCRIPTS_FIRST; type < SCRIPTS_LAST; ++type)
        CheckScripts(ScriptsType(type), ids);

    for (std::set<int32>::const_iterator itr = ids.begin(); itr != ids.end(); ++itr)
        TC_LOG_ERROR(LOG_FILTER_SQL, "Table `db_script_string` has unused string id  %u", *itr);
}

void ScriptDataStoreMgr::LoadScripts(ScriptsType type)
{
    uint32 oldMSTime = getMSTime();

    ScriptMapMap* scripts = GetScriptsMapByType(type);
    if (!scripts)
        return;

    std::string tableName = GetScriptsTableNameByType(type);
    if (tableName.empty())
        return;

    if (sScriptMgr->IsScriptScheduled())                    // function cannot be called when scripts are in use.
        return;

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading %s...", tableName.c_str());

    scripts->clear();                                       // need for reload support

    bool isSpellScriptTable = (type == SCRIPTS_SPELL);
    //                                                 0    1       2         3         4          5    6  7  8  9
    QueryResult result = WorldDatabase.PQuery("SELECT id, delay, command, datalong, datalong2, dataint, x, y, z, o%s FROM %s", isSpellScriptTable ? ", effIndex" : "", tableName.c_str());

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 script definitions. DB table `%s` is empty!", tableName.c_str());
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        ScriptInfo tmp;
        tmp.type = type;
        tmp.id = fields[0].GetUInt32();
        if (isSpellScriptTable)
            tmp.id |= fields[10].GetUInt8() << 24;
        tmp.delay = fields[1].GetUInt32();
        tmp.command = ScriptCommands(fields[2].GetUInt32());
        tmp.Raw.nData[0] = fields[3].GetUInt32();
        tmp.Raw.nData[1] = fields[4].GetUInt32();
        tmp.Raw.nData[2] = fields[5].GetInt32();
        tmp.Raw.fData[0] = fields[6].GetFloat();
        tmp.Raw.fData[1] = fields[7].GetFloat();
        tmp.Raw.fData[2] = fields[8].GetFloat();
        tmp.Raw.fData[3] = fields[9].GetFloat();

        // generic command args check
        switch (tmp.command)
        {
        case SCRIPT_COMMAND_TALK:
        {
            if (tmp.Talk.ChatType > CHAT_TYPE_WHISPER && tmp.Talk.ChatType != CHAT_MSG_RAID_BOSS_WHISPER)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid talk type (datalong = %u) in SCRIPT_COMMAND_TALK for script id %u",
                    tableName.c_str(), tmp.Talk.ChatType, tmp.id);
                continue;
            }
            if (!tmp.Talk.TextID)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid talk text id (dataint = %i) in SCRIPT_COMMAND_TALK for script id %u",
                    tableName.c_str(), tmp.Talk.TextID, tmp.id);
                continue;
            }
            if (tmp.Talk.TextID < MIN_DB_SCRIPT_STRING_ID || tmp.Talk.TextID >= MAX_DB_SCRIPT_STRING_ID)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has out of range text id (dataint = %i expected %u-%u) in SCRIPT_COMMAND_TALK for script id %u",
                    tableName.c_str(), tmp.Talk.TextID, MIN_DB_SCRIPT_STRING_ID, MAX_DB_SCRIPT_STRING_ID, tmp.id);
                continue;
            }

            break;
        }

        case SCRIPT_COMMAND_EMOTE:
        {
            if (!sEmotesStore.LookupEntry(tmp.Emote.EmoteID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid emote id (datalong = %u) in SCRIPT_COMMAND_EMOTE for script id %u",
                    tableName.c_str(), tmp.Emote.EmoteID, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_TELEPORT_TO:
        {
            if (!sMapStore.LookupEntry(tmp.TeleportTo.MapID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid map (Id: %u) in SCRIPT_COMMAND_TELEPORT_TO for script id %u",
                    tableName.c_str(), tmp.TeleportTo.MapID, tmp.id);
                continue;
            }

            if (!Trinity::IsValidMapCoord(tmp.TeleportTo.DestX, tmp.TeleportTo.DestY, tmp.TeleportTo.DestZ, tmp.TeleportTo.Orientation))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid coordinates (X: %f Y: %f Z: %f O: %f) in SCRIPT_COMMAND_TELEPORT_TO for script id %u",
                    tableName.c_str(), tmp.TeleportTo.DestX, tmp.TeleportTo.DestY, tmp.TeleportTo.DestZ, tmp.TeleportTo.Orientation, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_QUEST_EXPLORED:
        {
            Quest const* quest = sQuestDataStore->GetQuestTemplate(tmp.QuestExplored.QuestID);
            if (!quest)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u",
                    tableName.c_str(), tmp.QuestExplored.QuestID, tmp.id);
                continue;
            }

            if (!quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has quest (ID: %u) in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, but quest not have flag QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT in quest flags. Script command or quest flags wrong. Quest modified to require objective.",
                    tableName.c_str(), tmp.QuestExplored.QuestID, tmp.id);

                // this will prevent quest completing without objective
                const_cast<Quest*>(quest)->SetSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT);

                // continue; - quest objective requirement set and command can be allowed
            }

            if (float(tmp.QuestExplored.Distance) > DEFAULT_VISIBILITY_DISTANCE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u",
                    tableName.c_str(), tmp.QuestExplored.Distance, tmp.id);
                continue;
            }

            if (tmp.QuestExplored.Distance && float(tmp.QuestExplored.Distance) > DEFAULT_VISIBILITY_DISTANCE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has too large distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, max distance is %f or 0 for disable distance check",
                    tableName.c_str(), tmp.QuestExplored.Distance, tmp.id, DEFAULT_VISIBILITY_DISTANCE);
                continue;
            }

            if (tmp.QuestExplored.Distance && float(tmp.QuestExplored.Distance) < INTERACTION_DISTANCE)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has too small distance (%u) for exploring objective complete in `datalong2` in SCRIPT_COMMAND_QUEST_EXPLORED in `datalong` for script id %u, min distance is %f or 0 for disable distance check",
                    tableName.c_str(), tmp.QuestExplored.Distance, tmp.id, INTERACTION_DISTANCE);
                continue;
            }

            break;
        }

        case SCRIPT_COMMAND_KILL_CREDIT:
        {
            if (!sObjectMgr->GetCreatureTemplate(tmp.KillCredit.CreatureEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid creature (Entry: %u) in SCRIPT_COMMAND_KILL_CREDIT for script id %u",
                    tableName.c_str(), tmp.KillCredit.CreatureEntry, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
        {
            GameObjectData const* data = sObjectMgr->GetGOData(tmp.RespawnGameobject.GOGuid);
            if (!data)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid gameobject (GUID: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",
                    tableName.c_str(), tmp.RespawnGameobject.GOGuid, tmp.id);
                continue;
            }

            GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(data->id);
            if (!info)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",
                    tableName.c_str(), tmp.RespawnGameobject.GOGuid, data->id, tmp.id);
                continue;
            }

            if (info->type == GAMEOBJECT_TYPE_FISHINGNODE ||
                info->type == GAMEOBJECT_TYPE_FISHINGHOLE ||
                info->type == GAMEOBJECT_TYPE_DOOR ||
                info->type == GAMEOBJECT_TYPE_BUTTON ||
                info->type == GAMEOBJECT_TYPE_TRAP)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` have gameobject type (%u) unsupported by command SCRIPT_COMMAND_RESPAWN_GAMEOBJECT for script id %u",
                    tableName.c_str(), info->entry, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
        {
            if (!Trinity::IsValidMapCoord(tmp.TempSummonCreature.PosX, tmp.TempSummonCreature.PosY, tmp.TempSummonCreature.PosZ, tmp.TempSummonCreature.Orientation))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid coordinates (X: %f Y: %f Z: %f O: %f) in SCRIPT_COMMAND_TEMP_SUMMON_CREATURE for script id %u",
                    tableName.c_str(), tmp.TempSummonCreature.PosX, tmp.TempSummonCreature.PosY, tmp.TempSummonCreature.PosZ, tmp.TempSummonCreature.Orientation, tmp.id);
                continue;
            }

            if (!sObjectMgr->GetCreatureTemplate(tmp.TempSummonCreature.CreatureEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid creature (Entry: %u) in SCRIPT_COMMAND_TEMP_SUMMON_CREATURE for script id %u",
                    tableName.c_str(), tmp.TempSummonCreature.CreatureEntry, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_OPEN_DOOR:
        case SCRIPT_COMMAND_CLOSE_DOOR:
        {
            GameObjectData const* data = sObjectMgr->GetGOData(tmp.ToggleDoor.GOGuid);
            if (!data)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has invalid gameobject (GUID: %u) in %s for script id %u",
                    tableName.c_str(), tmp.ToggleDoor.GOGuid, GetScriptCommandName(tmp.command).c_str(), tmp.id);
                continue;
            }

            GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(data->id);
            if (!info)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has gameobject with invalid entry (GUID: %u Entry: %u) in %s for script id %u",
                    tableName.c_str(), tmp.ToggleDoor.GOGuid, data->id, GetScriptCommandName(tmp.command).c_str(), tmp.id);
                continue;
            }

            if (info->type != GAMEOBJECT_TYPE_DOOR)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has gameobject type (%u) non supported by command %s for script id %u",
                    tableName.c_str(), info->entry, GetScriptCommandName(tmp.command).c_str(), tmp.id);
                continue;
            }

            break;
        }

        case SCRIPT_COMMAND_REMOVE_AURA:
        {
            if (!sSpellMgr->GetSpellInfo(tmp.RemoveAura.SpellID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` using non-existent spell (id: %u) in SCRIPT_COMMAND_REMOVE_AURA for script id %u",
                    tableName.c_str(), tmp.RemoveAura.SpellID, tmp.id);
                continue;
            }
            if (tmp.RemoveAura.Flags & ~0x1)                    // 1 bits (0, 1)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` using unknown flags in datalong2 (%u) in SCRIPT_COMMAND_REMOVE_AURA for script id %u",
                    tableName.c_str(), tmp.RemoveAura.Flags, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_CAST_SPELL:
        {
            if (!sSpellMgr->GetSpellInfo(tmp.CastSpell.SpellID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` using non-existent spell (id: %u) in SCRIPT_COMMAND_CAST_SPELL for script id %u",
                    tableName.c_str(), tmp.CastSpell.SpellID, tmp.id);
                continue;
            }
            if (tmp.CastSpell.Flags > 4)                      // targeting type
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` using unknown target in datalong2 (%u) in SCRIPT_COMMAND_CAST_SPELL for script id %u",
                    tableName.c_str(), tmp.CastSpell.Flags, tmp.id);
                continue;
            }
            if (tmp.CastSpell.Flags != 4 && tmp.CastSpell.CreatureEntry & ~0x1)                      // 1 bit (0, 1)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` using unknown flags in dataint (%u) in SCRIPT_COMMAND_CAST_SPELL for script id %u",
                    tableName.c_str(), tmp.CastSpell.CreatureEntry, tmp.id);
                continue;
            }
            if (tmp.CastSpell.Flags == 4 && !sObjectMgr->GetCreatureTemplate(tmp.CastSpell.CreatureEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` using invalid creature entry in dataint (%u) in SCRIPT_COMMAND_CAST_SPELL for script id %u",
                    tableName.c_str(), tmp.CastSpell.CreatureEntry, tmp.id);
                continue;
            }
            break;
        }

        case SCRIPT_COMMAND_CREATE_ITEM:
        {
            if (!sObjectMgr->GetItemTemplate(tmp.CreateItem.ItemEntry))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` has nonexistent item (entry: %u) in SCRIPT_COMMAND_CREATE_ITEM for script id %u", tableName.c_str(), tmp.CreateItem.ItemEntry, tmp.id);
                continue;
            }
            if (!tmp.CreateItem.Amount)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `%s` SCRIPT_COMMAND_CREATE_ITEM but amount is %u for script id %u", tableName.c_str(), tmp.CreateItem.Amount, tmp.id);
                continue;
            }
            break;
        }
        default:
            break;
        }

        if (scripts->find(tmp.id) == scripts->end())
        {
            ScriptMap emptyMap;
            (*scripts)[tmp.id] = emptyMap;
        }
        (*scripts)[tmp.id].insert(std::make_pair(tmp.delay, tmp));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u script definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ScriptDataStoreMgr::LoadGameObjectScripts()
{
    LoadScripts(SCRIPTS_GAMEOBJECT);

    // check ids
    for (ScriptMapMap::const_iterator itr = sGameObjectScripts.begin(); itr != sGameObjectScripts.end(); ++itr)
    {
        if (!sObjectMgr->GetGOData(itr->first))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gameobject_scripts` has not existing gameobject (GUID: %u) as script id", itr->first);
    }
}

void ScriptDataStoreMgr::LoadQuestEndScripts()
{
    LoadScripts(SCRIPTS_QUEST_END);

    // check ids
    for (ScriptMapMap::const_iterator itr = sQuestEndScripts.begin(); itr != sQuestEndScripts.end(); ++itr)
    {
        if (!sQuestDataStore->GetQuestTemplate(itr->first))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `quest_end_scripts` has not existing quest (Id: %u) as script id", itr->first);
    }
}

void ScriptDataStoreMgr::LoadQuestStartScripts()
{
    LoadScripts(SCRIPTS_QUEST_START);

    // check ids
    for (ScriptMapMap::const_iterator itr = sQuestStartScripts.begin(); itr != sQuestStartScripts.end(); ++itr)
    {
        if (!sQuestDataStore->GetQuestTemplate(itr->first))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `quest_start_scripts` has not existing quest (Id: %u) as script id", itr->first);
    }
}

void ScriptDataStoreMgr::LoadSpellScripts()
{
    LoadScripts(SCRIPTS_SPELL);

    // check ids
    for (ScriptMapMap::const_iterator itr = sSpellScripts.begin(); itr != sSpellScripts.end(); ++itr)
    {
        uint32 spellId = uint32(itr->first) & 0x00FFFFFF;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `spell_scripts` has not existing spell (Id: %u) as script id", spellId);
            continue;
        }

        uint8 i = static_cast<uint8>((uint32(itr->first) >> 24) & 0x000000FF);
        //check for correct spellEffect
        if (!spellInfo->Effects[i]->Effect || (spellInfo->Effects[i]->Effect != SPELL_EFFECT_SCRIPT_EFFECT && spellInfo->Effects[i]->Effect != SPELL_EFFECT_DUMMY))
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `spell_scripts` - spell %u effect %u is not SPELL_EFFECT_SCRIPT_EFFECT or SPELL_EFFECT_DUMMY", spellId, i);
    }
}

void ScriptDataStoreMgr::LoadEventScripts()
{
    LoadScripts(SCRIPTS_EVENT);

    std::set<uint32> evt_scripts;
    // Load all possible script entries from gameobjects
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplates();
    for (GameObjectTemplateContainer::const_iterator itr = gotc->begin(); itr != gotc->end(); ++itr)
        if (uint32 eventId = itr->second.GetEventScriptId())
            evt_scripts.insert(eventId);

    // Load all possible script entries from spells
    for (uint32 i = 1; i < sSpellMgr->GetSpellInfoStoreSize(); ++i)
        if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(i))
            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                if (spell->Effects[j]->Effect == SPELL_EFFECT_SEND_EVENT)
                    if (spell->Effects[j]->MiscValue)
                        evt_scripts.insert(spell->Effects[j]->MiscValue);

    for (size_t path_idx = 0; path_idx < sTaxiPathNodesByPath.size(); ++path_idx)
    {
        for (size_t node_idx = 0; node_idx < sTaxiPathNodesByPath[path_idx].size(); ++node_idx)
        {
            TaxiPathNodeEntry const* node = sTaxiPathNodesByPath[path_idx][node_idx];

            if (node->ArrivalEventID)
                evt_scripts.insert(node->ArrivalEventID);

            if (node->DepartureEventID)
                evt_scripts.insert(node->DepartureEventID);
        }
    }

    // Then check if all scripts are in above list of possible script entries
    for (ScriptMapMap::const_iterator itr = sEventScripts.begin(); itr != sEventScripts.end(); ++itr)
    {
        std::set<uint32>::const_iterator itr2 = evt_scripts.find(itr->first);
        if (itr2 == evt_scripts.end())
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `event_scripts` has script (Id: %u) not referring to any gameobject_template type 10 data2 field, type 3 data6 field, type 13 data 2 field or any spell effect %u",
                itr->first, SPELL_EFFECT_SEND_EVENT);
    }
}

void ScriptDataStoreMgr::LoadWaypointScripts()
{
    LoadScripts(SCRIPTS_WAYPOINT);

    std::set<uint32> actionSet;

    for (ScriptMapMap::const_iterator itr = sWaypointScripts.begin(); itr != sWaypointScripts.end(); ++itr)
        actionSet.insert(itr->first);

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WOLRD_SEL_WAYPOINT_DATA_ACTION);
    if (PreparedQueryResult result = WorldDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            actionSet.erase(fields[0].GetUInt32());
        } while (result->NextRow());
    }

    for (std::set<uint32>::iterator itr = actionSet.begin(); itr != actionSet.end(); ++itr)
        TC_LOG_ERROR(LOG_FILTER_SQL, "There is no waypoint which links to the waypoint script %u", *itr);
}

SpellScriptsBounds ScriptDataStoreMgr::GetSpellScriptsBounds(uint32 spellID)
{
    return SpellScriptsBounds(_spellScriptsStore.equal_range(spellID));
}

void ScriptDataStoreMgr::LoadSpellScriptNames()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading spell script names...");

    uint32 oldMSTime = getMSTime();

    _spellScriptsStore.clear();                            // need for reload case

    QueryResult result = WorldDatabase.Query("SELECT spell_id, ScriptName FROM spell_script_names");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell script names. DB table `spell_script_names` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();

        auto spellId = fields[0].GetInt32();
        auto const scriptName = fields[1].GetString();

        auto allRanks = false;
        if (spellId < 0)
        {
            allRanks = true;
            spellId = -spellId;
        }

        auto spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Scriptname: `%s` spell (Id: %d) does not exist.", scriptName.c_str(), fields[0].GetInt32());
            WorldDatabase.PExecute("DELETE FROM `spell_script_names` WHERE spell_id = %i", fields[0].GetInt32());
            continue;
        }

        if (allRanks)
        {
            if (!spellInfo->IsRanked())
                TC_LOG_ERROR(LOG_FILTER_SQL, "Scriptname: `%s` spell (Id: %d) has no ranks of spell.", scriptName.c_str(), fields[0].GetInt32());

            if (spellInfo->GetFirstRankSpell()->Id != uint32(spellId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Scriptname: `%s` spell (Id: %d) is not first rank of spell.", scriptName.c_str(), fields[0].GetInt32());
                continue;
            }
            while (spellInfo)
            {
                _spellScriptsStore.insert(SpellScriptsContainer::value_type(spellInfo->Id, std::make_pair(sObjectMgr->GetScriptId(scriptName.c_str()), true)));
                spellInfo = spellInfo->GetNextRankSpell();
            }
        }
        else
        {
            if (spellInfo->IsRanked())
                TC_LOG_ERROR(LOG_FILTER_SQL, "Scriptname: `%s` spell (Id: %d) is ranked spell. Perhaps not all ranks are assigned to this script.", scriptName.c_str(), spellId);

            _spellScriptsStore.insert(SpellScriptsContainer::value_type(spellInfo->Id, std::make_pair(sObjectMgr->GetScriptId(scriptName.c_str()), true)));
        }

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell script names in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ScriptDataStoreMgr::ValidateSpellScripts()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Validating spell scripts...");

    uint32 oldMSTime = getMSTime();

    if (_spellScriptsStore.empty())
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Validated 0 scripts.");
        return;
    }

    uint32 count = 0;

    for (auto spell : _spellScriptsStore)
    {
        auto spellEntry = sSpellMgr->GetSpellInfo(spell.first);
        auto const bounds = GetSpellScriptsBounds(spell.first);

        for (auto itr = bounds.first; itr != bounds.second; ++itr)
        {
            if (auto spellScriptLoader = sScriptMgr->GetSpellScriptLoader(itr->second.first))
            {
                ++count;

                std::unique_ptr<SpellScript> spellScript(spellScriptLoader->GetSpellScript());
                std::unique_ptr<AuraScript> auraScript(spellScriptLoader->GetAuraScript());

                if (!spellScript && !auraScript)
                {
                    TC_LOG_ERROR(LOG_FILTER_TSCR, "Functions GetSpellScript() and GetAuraScript() of script `%s` do not return objects - script skipped", sObjectMgr->GetScriptName(itr->second.first).c_str());
                    itr->second.second = false;
                    continue;
                }

                if (spellScript)
                {
                    spellScript->_Init(&spellScriptLoader->GetName(), spellEntry->Id);
                    spellScript->_Register();

                    if (!spellScript->_Validate(spellEntry))
                    {
                        itr->second.second = false;
                        continue;
                    }
                }

                if (auraScript)
                {
                    auraScript->_Init(&spellScriptLoader->GetName(), spellEntry->Id);
                    auraScript->_Register();

                    if (!auraScript->_Validate(spellEntry))
                    {
                        itr->second.second = false;
                        continue;
                    }
                }

                itr->second.second = true;
            }
            else
                itr->second.second = false;
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Validated %u scripts in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}
