/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
Name: npc_commandscript
%Complete: 100
Comment: All npc related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Transport.h"
#include "CreatureGroups.h"
#include "TargetedMovementGenerator.h"                      // for HandleNpcUnFollowCommand
#include "CreatureAI.h"

template<typename E, typename T = char const*>
struct EnumName
{
    E Value;
    T Name;
};

#define CREATE_NAMED_ENUM(VALUE) { VALUE, STRINGIZE(VALUE) }

#define NPCFLAG_COUNT       24
#define FLAGS_EXTRA_COUNT   16
#define MAX_UNIT_FLAGS      33
#define MAX_UNIT_FLAGS2     29
#define MAX_UNIT_DYNFLAGS   10

EnumName<Mechanics> const mechanicImmunes[MAX_MECHANIC] =
{
    CREATE_NAMED_ENUM(MECHANIC_CHARM),
    CREATE_NAMED_ENUM(MECHANIC_DISORIENTED),
    CREATE_NAMED_ENUM(MECHANIC_DISARM),
    CREATE_NAMED_ENUM(MECHANIC_DISTRACT),
    CREATE_NAMED_ENUM(MECHANIC_FEAR),
    CREATE_NAMED_ENUM(MECHANIC_GRIP),
    CREATE_NAMED_ENUM(MECHANIC_ROOT),
    CREATE_NAMED_ENUM(MECHANIC_COMBAT_SLOW),
    CREATE_NAMED_ENUM(MECHANIC_SILENCE),
    CREATE_NAMED_ENUM(MECHANIC_SLEEP),
    CREATE_NAMED_ENUM(MECHANIC_SNARE),
    CREATE_NAMED_ENUM(MECHANIC_STUN),
    CREATE_NAMED_ENUM(MECHANIC_FREEZE),
    CREATE_NAMED_ENUM(MECHANIC_INCAPACITATE),
    CREATE_NAMED_ENUM(MECHANIC_BLEED),
    CREATE_NAMED_ENUM(MECHANIC_PROVOKE),
    CREATE_NAMED_ENUM(MECHANIC_POLYMORPH),
    CREATE_NAMED_ENUM(MECHANIC_BANISH),
    CREATE_NAMED_ENUM(MECHANIC_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SHACKLE),
    CREATE_NAMED_ENUM(MECHANIC_MOUNT),
    CREATE_NAMED_ENUM(MECHANIC_ENCOUNTER),
    CREATE_NAMED_ENUM(MECHANIC_TURN),
    CREATE_NAMED_ENUM(MECHANIC_HORROR),
    CREATE_NAMED_ENUM(MECHANIC_INVULNERABILITY),
    CREATE_NAMED_ENUM(MECHANIC_INTERRUPT),
    CREATE_NAMED_ENUM(MECHANIC_DAZE),
    CREATE_NAMED_ENUM(MECHANIC_DISCOVERY),
    CREATE_NAMED_ENUM(MECHANIC_MAGICAL_IMMUNITY),
    CREATE_NAMED_ENUM(MECHANIC_SAPPED),
    CREATE_NAMED_ENUM(MECHANIC_ENRAGED),
    CREATE_NAMED_ENUM(MECHANIC_WOUNDED)
};

EnumName<UnitFlags> const unitFlags[MAX_UNIT_FLAGS] =
{
    CREATE_NAMED_ENUM(UNIT_FLAG_SERVER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NON_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_REMOVE_CLIENT_CONTROL),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_RENAME),
    CREATE_NAMED_ENUM(UNIT_FLAG_PREPARATION),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_6),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_ATTACKABLE_1),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_PC),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_NPC),
    CREATE_NAMED_ENUM(UNIT_FLAG_LOOTING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PET_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP),
    CREATE_NAMED_ENUM(UNIT_FLAG_SILENCED),
    CREATE_NAMED_ENUM(UNIT_FLAG_CANNOT_SWIM),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_15),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_16),
    CREATE_NAMED_ENUM(UNIT_FLAG_PACIFIED),
    CREATE_NAMED_ENUM(UNIT_FLAG_STUNNED),
    CREATE_NAMED_ENUM(UNIT_FLAG_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_TAXI_FLIGHT),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISARMED),
    CREATE_NAMED_ENUM(UNIT_FLAG_CONFUSED),
    CREATE_NAMED_ENUM(UNIT_FLAG_FLEEING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PLAYER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_SELECTABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_SKINNABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_SELECTABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_PREVENT_KNEELING_WHEN_LOOTING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PREVENT_EMOTES),
    CREATE_NAMED_ENUM(UNIT_FLAG_SHEATHE),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_31)
};

EnumName<UnitFlags2> const unitFlags2[MAX_UNIT_FLAGS2] =
{
    CREATE_NAMED_ENUM(UNIT_FLAG2_FEIGN_DEATH),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK1),
    CREATE_NAMED_ENUM(UNIT_FLAG2_IGNORE_REPUTATION),
    CREATE_NAMED_ENUM(UNIT_FLAG2_COMPREHEND_LANG),
    CREATE_NAMED_ENUM(UNIT_FLAG2_MIRROR_IMAGE),
    CREATE_NAMED_ENUM(UNIT_FLAG2_INSTANTLY_APPEAR_MODEL),
    CREATE_NAMED_ENUM(UNIT_FLAG2_FORCE_MOVEMENT),
    CREATE_NAMED_ENUM(UNIT_FLAG2_DISARM_OFFHAND),
    CREATE_NAMED_ENUM(UNIT_FLAG2_DISABLE_PRED_STATS),
    CREATE_NAMED_ENUM(UNIT_FLAG2_DISARM_RANGED),
    CREATE_NAMED_ENUM(UNIT_FLAG2_REGENERATE_POWER),
    CREATE_NAMED_ENUM(UNIT_FLAG2_RESTRICT_PARTY_INTERACTION),
    CREATE_NAMED_ENUM(UNIT_FLAG2_PREVENT_SPELL_CLICK),
    CREATE_NAMED_ENUM(UNIT_FLAG2_ALLOW_ENEMY_INTERACT),
    CREATE_NAMED_ENUM(UNIT_FLAG2_DISABLE_TURN),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK2),
    CREATE_NAMED_ENUM(UNIT_FLAG2_PLAY_DEATH_ANIM),
    CREATE_NAMED_ENUM(UNIT_FLAG2_ALLOW_CHEAT_SPELLS),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK3),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK4),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK5),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK6),
    CREATE_NAMED_ENUM(UNIT_FLAG2_NO_ACTIONS),
    CREATE_NAMED_ENUM(UNIT_FLAG2_SWIM_PREVENT),
    CREATE_NAMED_ENUM(UNIT_FLAG2_HIDE_IN_COMBAT_LOG),
    CREATE_NAMED_ENUM(UNIT_FLAG2_PREVENT_SELECT_NPC),
    CREATE_NAMED_ENUM(UNOT_FLAG2_IGNORE_SPELL_MIN_RANGE_RESTRICTIONS),
    CREATE_NAMED_ENUM(UNIT_FLAG2_UNK7),
};
EnumName<CreatureFlagsExtra> const flagsExtra[FLAGS_EXTRA_COUNT] =
{
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_INSTANCE_BIND),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_CIVILIAN),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_PARRY),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_BLOCK),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_CRUSH),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_XP_AT_KILL),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_TRIGGER),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_TAUNT),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_WORLDEVENT),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_GUARD),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_CRIT),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_NO_SKILLGAIN),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_TAUNT_DIMINISH),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_ALL_DIMINISH),
    CREATE_NAMED_ENUM(CREATURE_FLAG_EXTRA_DUNGEON_BOSS)
};

EnumName<UnitDynFlags> const dynFlags[MAX_UNIT_DYNFLAGS] =
{
    CREATE_NAMED_ENUM(UNIT_DYNFLAG_HIDE_MODEL),
    CREATE_NAMED_ENUM(UNIT_DYNFLAG_LOOTABLE),
    CREATE_NAMED_ENUM(UNIT_DYNFLAG_TRACK_UNIT),
    CREATE_NAMED_ENUM(UNIT_DYNFLAG_TAPPED),
    CREATE_NAMED_ENUM(UNIT_DYNFLAG_SPECIALINFO),
    CREATE_NAMED_ENUM(UNIT_DYNFLAG_REFER_A_FRIEND),
};

class npc_commandscript : public CommandScript
{
public:
    npc_commandscript() : CommandScript("npc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> npcAddCommandTable =
        {
            { "formation",      SEC_MODERATOR,      false, &HandleNpcAddFormationCommand,      ""},
            { "item",           SEC_GAMEMASTER,     false, &HandleNpcAddVendorItemCommand,     ""},
            { "move",           SEC_GAMEMASTER,     false, &HandleNpcAddMoveCommand,           ""},
            { "temp",           SEC_GAMEMASTER,     false, &HandleNpcAddTempSpawnCommand,      ""},
            //{ TODO: fix or remove this command
            { "weapon",         SEC_ADMINISTRATOR,  false, &HandleNpcAddWeaponCommand,         ""},
            //}
            { "",               SEC_GAMEMASTER,     false, &HandleNpcAddCommand,               ""}
        };
        static std::vector<ChatCommand> npcDeleteCommandTable =
        {
            { "item",           SEC_GAMEMASTER,     false, &HandleNpcDeleteVendorItemCommand,  ""},
            { "",               SEC_GAMEMASTER,     false, &HandleNpcDeleteCommand,            ""}
        };
        static std::vector<ChatCommand> npcFollowCommandTable =
        {
            { "stop",           SEC_GAMEMASTER,     false, &HandleNpcUnFollowCommand,          ""},
            { "",               SEC_GAMEMASTER,     false, &HandleNpcFollowCommand,            ""}
        };
        static std::vector<ChatCommand> npcSetCommandTable =
        {
            { "allowmove",      SEC_ADMINISTRATOR,  false, &HandleNpcSetAllowMovementCommand,  ""},
            { "entry",          SEC_ADMINISTRATOR,  false, &HandleNpcSetEntryCommand,          ""},
            { "factionid",      SEC_GAMEMASTER,     false, &HandleNpcSetFactionIdCommand,      ""},
            { "flag",           SEC_GAMEMASTER,     false, &HandleNpcSetFlagCommand,           ""},
            { "level",          SEC_GAMEMASTER,     false, &HandleNpcSetLevelCommand,          ""},
            { "link",           SEC_GAMEMASTER,     false, &HandleNpcSetLinkCommand,           ""},
            { "model",          SEC_GAMEMASTER,     false, &HandleNpcSetModelCommand,          ""},
            { "movetype",       SEC_GAMEMASTER,     false, &HandleNpcSetMoveTypeCommand,       ""},
            { "phase",          SEC_GAMEMASTER,     false, &HandleNpcSetPhaseCommand,          ""},
            { "spawndist",      SEC_GAMEMASTER,     false, &HandleNpcSetSpawnDistCommand,      ""},
            { "spawntime",      SEC_GAMEMASTER,     false, &HandleNpcSetSpawnTimeCommand,      ""},
            { "data",           SEC_ADMINISTRATOR,  false, &HandleNpcSetDataCommand,           ""},
            //{ TODO: fix or remove these commands
            { "name",           SEC_GAMEMASTER,     false, &HandleNpcSetNameCommand,           ""},
            { "subname",        SEC_GAMEMASTER,     false, &HandleNpcSetSubNameCommand,        ""},
            { "size",           SEC_GAMEMASTER,     false, &HandleNpcSetSizeCommand,           ""}
            //}
        };
        static std::vector<ChatCommand> npcCommandTable =
        {
            { "info",           SEC_ADMINISTRATOR,  false, &HandleNpcInfoCommand,              ""},
            { "flags",          SEC_ADMINISTRATOR,  false, &HandleNpcFlagsInfoCommand,         ""},
            { "move",           SEC_GAMEMASTER,     false, &HandleNpcMoveCommand,              ""},
            { "playemote",      SEC_ADMINISTRATOR,  false, &HandleNpcPlayEmoteCommand,         ""},
            { "say",            SEC_MODERATOR,      false, &HandleNpcSayCommand,               ""},
            { "textemote",      SEC_MODERATOR,      false, &HandleNpcTextEmoteCommand,         ""},
            { "whisper",        SEC_MODERATOR,      false, &HandleNpcWhisperCommand,           ""},
            { "yell",           SEC_MODERATOR,      false, &HandleNpcYellCommand,              ""},
            { "tame",           SEC_GAMEMASTER,     false, &HandleNpcTameCommand,              ""},
            { "map_activate",   SEC_GAMEMASTER,     false, &HandleNpcActivateCommand,          ""},
            { "near",           SEC_GAMEMASTER,     false, &HandleNpcNearCommand,              ""},
            { "add",            SEC_GAMEMASTER,     false, NULL,                 "", npcAddCommandTable },
            { "delete",         SEC_GAMEMASTER,     false, NULL,              "", npcDeleteCommandTable },
            { "follow",         SEC_GAMEMASTER,     false, NULL,              "", npcFollowCommandTable },
            { "set",            SEC_GAMEMASTER,     false, NULL,                 "", npcSetCommandTable },
            { "summon",         SEC_ADMINISTRATOR,  false, &HandleNpcSummonGroupCommand,       ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "npc",            SEC_MODERATOR,      false, NULL,                    "", npcCommandTable }
        };
        return commandTable;
    }

    //add spawn of creature
    static bool HandleNpcAddCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* charID = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!charID)
            return false;

        char* team = strtok(NULL, " ");
        int32 teamval = 0;
        if (team) { teamval = atoi(team); }
        if (teamval < 0) { teamval = 0; }

        uint32 id  = atoi(charID);

        Player* chr = handler->GetSession()->GetPlayer();
        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float o = chr->GetOrientation();
        Map* map = chr->GetMap();

        if (Transport* trans = chr->GetTransport())
        {
            ObjectGuid::LowType guid = sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate();
            CreatureData& data = sObjectMgr->NewOrExistCreatureData(guid);
            data.id = id;
            data.phaseMask = chr->GetPhaseMask();
            data.posX = chr->GetTransOffsetX();
            data.posY = chr->GetTransOffsetY();
            data.posZ = chr->GetTransOffsetZ();
            data.orientation = chr->GetTransOffsetO();

            Creature* creature = trans->CreateNPCPassenger(guid, &data);

            creature->SaveToDB(trans->GetGOInfo()->moTransport.SpawnMap, UI64LIT(1) << map->GetSpawnMode(), chr->GetPhaseMask());

            sObjectMgr->AddCreatureToGrid(guid, &data);
            return true;
        }

        Creature* creature = new Creature();
        if (!creature->Create(sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate(), map, chr->GetPhaseMgr().GetPhaseMaskForSpawn(), id, 0, (uint32)teamval, x, y, z, o))
        {
            delete creature;
            return false;
        }

        creature->SaveToDB(map->GetId(), (UI64LIT(1) << map->GetSpawnMode()), chr->GetPhaseMgr().GetPhaseMaskForSpawn());

        uint32 db_guid = creature->GetDBTableGUIDLow();

        // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
        // current "creature" variable is deleted and created fresh new, otherwise old values might trigger asserts or cause undefined behavior
        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadCreatureFromDB(db_guid, map))
        {
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
        return true;
    }

    //add item in vendorlist
    static bool HandleNpcAddVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        char* pitem  = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 item_int = atol(pitem);
        if (item_int <= 0)
            return false;

        uint32 itemId = item_int;

        char* fmaxcount = strtok(NULL, " ");                    //add maxcount, default: 0
        uint32 maxcount = 0;
        if (fmaxcount)
            maxcount = atol(fmaxcount);

        char* fincrtime = strtok(NULL, " ");                    //add incrtime, default: 0
        uint32 incrtime = 0;
        if (fincrtime)
            incrtime = atol(fincrtime);

        char* fextendedcost = strtok(NULL, " ");                //add ExtendedCost, default: 0
        uint32 extendedcost = fextendedcost ? atol(fextendedcost) : 0;
        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* fmoney = strtok(NULL, " ");                    //add money, default: 0
        uint64 money = 0;
        if (fmoney)
            money = atol(fmoney);

        uint32 vendor_entry = vendor ? vendor->GetEntry() : 0;

        if (!sObjectMgr->IsVendorItemValid(vendor_entry, itemId, maxcount, incrtime, extendedcost, type, handler->GetSession()->GetPlayer()))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->AddVendorItem(vendor_entry, itemId, maxcount, incrtime, extendedcost, type, money);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_LIST, itemId, itemTemplate->GetName()->Str[handler->GetSessionDbLocaleIndex()], maxcount, incrtime, extendedcost);
        return true;
    }

    //add move for creature
    static bool HandleNpcAddMoveCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* guidStr = strtok((char*)args, " ");
        char* waitStr = strtok((char*)NULL, " ");

        uint32 lowGuid = atoi((char*)guidStr);

        Creature* creature = NULL;

        /* FIXME: impossible without entry
        if (lowguid)
            creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HighGuid::Creature));
        */

        // attempt check creature existence by DB data
        if (!creature)
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(lowGuid);
            if (!data)
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowGuid);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            // obtain real GUID for DB operations
            lowGuid = creature->GetDBTableGUIDLow();
        }

        int wait = waitStr ? atoi(waitStr) : 0;

        if (wait < 0)
            wait = 0;

        // Update movement type
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_MOVEMENT_TYPE);

        stmt->setUInt8(0, uint8(WAYPOINT_MOTION_TYPE));
        stmt->setUInt64(1, lowGuid);

        WorldDatabase.Execute(stmt);

        if (creature && creature->GetWaypointPath())
        {
            creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
            creature->GetMotionMaster()->Initialize();
            if (creature->isAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn(true);
            }
            creature->SaveToDB();
        }

        handler->SendSysMessage(LANG_WAYPOINT_ADDED);

        return true;
    }

    static bool HandleNpcSetAllowMovementCommand(ChatHandler* handler, const char* /*args*/)
    {
        if (sWorld->getAllowMovement())
        {
            sWorld->SetAllowMovement(false);
            handler->SendSysMessage(LANG_CREATURE_MOVE_DISABLED);
        }
        else
        {
            sWorld->SetAllowMovement(true);
            handler->SendSysMessage(LANG_CREATURE_MOVE_ENABLED);
        }
        return true;
    }

    static bool HandleNpcSetEntryCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 newEntryNum = atoi(args);
        if (!newEntryNum)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();
        if (creature->UpdateEntry(newEntryNum))
            handler->SendSysMessage(LANG_DONE);
        else
            handler->SendSysMessage(LANG_ERROR);
        return true;
    }

    //change level of creature or pet
    static bool HandleNpcSetLevelCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint8 lvl = (uint8) atoi((char*)args);
        if (lvl < 1 || lvl > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (creature->isPet())
        {
            if (((Pet*)creature)->getPetType() == HUNTER_PET)
            {
                creature->SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, sObjectMgr->GetXPForLevel(lvl)/4);
                creature->SetUInt32Value(UNIT_FIELD_PET_EXPERIENCE, 0);
            }
            ((Pet*)creature)->GivePetLevel(lvl);
        }
        else
        {
            creature->SetMaxHealth(100 + 30*lvl);
            creature->SetHealth(100 + 30*lvl);
            creature->SetLevel(lvl);
            creature->SaveToDB();
        }

        return true;
    }

    static bool HandleNpcDeleteCommand(ChatHandler* handler, const char* args)
    {
        Creature* unit = NULL;

        if (*args)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = strtoull(cId, nullptr, 10);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
                unit = handler->GetSession()->GetPlayer()->GetMap()->GetCreature(ObjectGuid::Create<HighGuid::Creature>(cr_data->mapid, cr_data->id, lowguid));
        }
        else
            unit = handler->getSelectedCreature();

        if (!unit || unit->isPet() || unit->isTotem())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Delete the creature
        unit->CombatStop();
        unit->DeleteFromDB();
        unit->AddObjectToRemoveList();

        handler->SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

        return true;
    }

    //del item from vendor list
    static bool HandleNpcDeleteVendorItemCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor || !vendor->isVendor())
        {
            handler->SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pitem  = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 itemId = atol(pitem);

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        if (!sObjectMgr->RemoveVendorItem(vendor->GetEntry(), itemId, type))
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_IN_LIST, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST, itemId, itemTemplate->GetName()->Str[handler->GetSessionDbLocaleIndex()]);
        return true;
    }

    //set faction of creature
    static bool HandleNpcSetFactionIdCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 factionId = (uint32) atoi((char*)args);

        if (!sFactionTemplateStore.LookupEntry(factionId))
        {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->setFaction(factionId);

        // Faction is set in creature_template - not inside creature

        // Update in memory..
        if (CreatureTemplate const* cinfo = creature->GetCreatureTemplate())
        {
            const_cast<CreatureTemplate*>(cinfo)->faction = factionId;
        }

        // ..and DB
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_FACTION);

        stmt->setUInt16(0, uint16(factionId));
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        return true;
    }

    //set npcflag of creature
    static bool HandleNpcSetFlagCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 npcFlags = (uint32) atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, npcFlags);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_NPCFLAG);

        stmt->setUInt32(0, npcFlags);
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        handler->SendSysMessage(LANG_VALUE_SAVED_REJOIN);

        return true;
    }

    //set data of creature for testing scripting
    static bool HandleNpcSetDataCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* arg1 = strtok((char*)args, " ");
        char* arg2 = strtok((char*)NULL, "");

        if (!arg1 || !arg2)
            return false;

        uint32 data_1 = (uint32)atoi(arg1);
        uint32 data_2 = (uint32)atoi(arg2);

        if (!data_1 || !data_2)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->AI()->SetData(data_1, data_2);
        std::string AIorScript = creature->GetNPCAIName() != "" ? "AI type: " + creature->GetNPCAIName() : (creature->GetScriptName() != "" ? "Script Name: " + creature->GetScriptName() : "No AI or Script Name Set");
        handler->PSendSysMessage(LANG_NPC_SETDATA, creature->GetGUID(), creature->GetEntry(), creature->GetName(), data_1, data_2, AIorScript.c_str());
        return true;
    }

    //npc follow handling
    static bool HandleNpcFollowCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Follow player - Using pet's default dist and angle
        creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

        handler->PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName());
        return true;
    }

    static bool HandleNpcInfoCommand(ChatHandler* handler, const char* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 faction = target->getFaction();
        uint32 npcflags = target->GetUInt32Value(UNIT_FIELD_NPC_FLAGS);
        uint32 npcflags2 = target->GetUInt32Value(UNIT_FIELD_NPC_FLAGS2);
        uint32 displayid = target->GetDisplayId();
        uint32 nativeid = target->GetNativeDisplayId();
        uint32 Entry = target->GetEntry();
        CreatureTemplate const* cInfo = target->GetCreatureTemplate();

        int64 curRespawnDelay = target->GetRespawnTimeEx()-time(NULL);
        if (curRespawnDelay < 0)
            curRespawnDelay = 0;
        std::string curRespawnDelayStr = secsToTimeString(uint64(curRespawnDelay), true);
        std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

        handler->PSendSysMessage(LANG_NPCINFO_CHAR,  uint32(target->GetDBTableGUIDLow()), target->GetGUID().ToString().c_str(), faction, Entry, displayid, nativeid);

        {
            std::ostringstream ss_flags;

            if (npcflags)
            {
                ss_flags << npcflags << " : ";

                if (npcflags & UNIT_NPC_FLAG_GOSSIP)
                    ss_flags << "Gossip ";
                if (npcflags & UNIT_NPC_FLAG_QUESTGIVER)
                    ss_flags << "Questgiver ";
                if (npcflags & UNIT_NPC_FLAG_UNK1)
                    ss_flags << "Unk1 ";
                if (npcflags & UNIT_NPC_FLAG_UNK2)
                    ss_flags << "Unk2 ";
                if (npcflags & UNIT_NPC_FLAG_TRAINER)
                    ss_flags << "Trainer ";
                if (npcflags & UNIT_NPC_FLAG_TRAINER_CLASS)
                    ss_flags << "TrainerClass ";
                if (npcflags & UNIT_NPC_FLAG_TRAINER_PROFESSION)
                    ss_flags << "TrainerProfession ";
                if (npcflags & UNIT_NPC_FLAG_VENDOR)
                    ss_flags << "Vendor ";
                if (npcflags & UNIT_NPC_FLAG_VENDOR_AMMO)
                    ss_flags << "VendorAmmo ";
                if (npcflags & UNIT_NPC_FLAG_VENDOR_FOOD)
                    ss_flags << "VendorFood ";
                if (npcflags & UNIT_NPC_FLAG_VENDOR_POISON)
                    ss_flags << "VendorPoison ";
                if (npcflags & UNIT_NPC_FLAG_VENDOR_REAGENT)
                    ss_flags << "VendorReagent ";
                if (npcflags & UNIT_NPC_FLAG_REPAIR)
                    ss_flags << "Repair ";
                if (npcflags & UNIT_NPC_FLAG_FLIGHTMASTER)
                    ss_flags << "Flightmaster ";
                if (npcflags & UNIT_NPC_FLAG_SPIRITHEALER)
                    ss_flags << "Spirithealer ";
                if (npcflags & UNIT_NPC_FLAG_SPIRITGUIDE)
                    ss_flags << "Spiritguide ";
                if (npcflags & UNIT_NPC_FLAG_INNKEEPER)
                    ss_flags << "Innkeeper ";
                if (npcflags & UNIT_NPC_FLAG_BANKER)
                    ss_flags << "Banker ";
                if (npcflags & UNIT_NPC_FLAG_PETITIONER)
                    ss_flags << "Petitioner ";
                if (npcflags & UNIT_NPC_FLAG_TABARDDESIGNER)
                    ss_flags << "Tabarddesigner ";
                if (npcflags & UNIT_NPC_FLAG_BATTLEMASTER)
                    ss_flags << "Battlemaster ";
                if (npcflags & UNIT_NPC_FLAG_AUCTIONEER)
                    ss_flags << "Auctioneer ";
                if (npcflags & UNIT_NPC_FLAG_STABLEMASTER)
                    ss_flags << "Stablemaster ";
                if (npcflags & UNIT_NPC_FLAG_GUILD_BANKER)
                    ss_flags << "GuildBanker ";
                if (npcflags & UNIT_NPC_FLAG_SPELLCLICK)
                    ss_flags << "Spellclick ";
                if (npcflags & UNIT_NPC_FLAG_PLAYER_VEHICLE)
                    ss_flags << "PlayerVehicle ";
                if (npcflags & UNIT_NPC_FLAG_MAILBOX)
                    ss_flags << "Mailbox ";
                if (npcflags & UNIT_NPC_FLAG_ARTIFACT_POWER_RESPEC)
                    ss_flags << "ArtifactPowerRespec ";
                if (npcflags & UNIT_NPC_FLAG_TRANSMOGRIFIER)
                    ss_flags << "Transmogrifier ";
                if (npcflags & UNIT_NPC_FLAG_VAULTKEEPER)
                    ss_flags << "Vaultkeeper ";
                if (npcflags & UNIT_NPC_FLAG_WILD_BATTLE_PET)
                    ss_flags << "WildBattlePet ";
                if (npcflags & UNIT_NPC_FLAG_BLACK_MARKET)
                    ss_flags << "BlackMarket";
            }
            else
                ss_flags << "0";

            handler->PSendSysMessage("NpcFlags: %s", ss_flags.str().c_str());
        }

        {
            std::ostringstream ss_flags;

            if (npcflags2)
            {
                ss_flags << npcflags2 << " : ";

                if (npcflags2 & UNIT_NPC_FLAG2_UPGRADE_MASTER)
                    ss_flags << "UpgradeMaster ";
                if (npcflags2 & UNIT_NPC_FLAG2_GARRISON_ARCHITECT)
                    ss_flags << "GarrisonArchitect ";
                if (npcflags2 & UNIT_NPC_FLAG2_AI_OBSTACLE)
                    ss_flags << "AIObstacle ";
                if (npcflags2 & UNIT_NPC_FLAG2_STEERING)
                    ss_flags << "Steering ";
                if (npcflags2 & UNIT_NPC_FLAG2_SHIPYARD_MISSION_NPC)
                    ss_flags << "ShipyardMissionNPC ";
                if (npcflags2 & UNIT_NPC_FLAG2_SHIPMENT_CRAFTER)
                    ss_flags << "ShipmentCrafter ";
                if (npcflags2 & UNIT_NPC_FLAG2_GARRISON_MISSION_NPC)
                    ss_flags << "GarrisonMissionNPC ";
                if (npcflags2 & UNIT_NPC_FLAG2_TRADESKILL_NPC)
                    ss_flags << "TradeskillNPC ";
                if (npcflags2 & UNIT_NPC_FLAG2_RECRUITER)
                    ss_flags << "Recruiter ";
                if (npcflags2 & UNIT_NPC_FLAG2_CLASS_HALL_UPGRADE)
                    ss_flags << "ClassHallUpgrade ";
                if (npcflags2 & UNIT_NPC_FLAG2_CONTRIBUTION_NPC)
                    ss_flags << "ContributionNPC";
            }
            else
                ss_flags << "0";

            handler->PSendSysMessage("NpcFlags2: %s", ss_flags.str().c_str());
        }

        {
            std::ostringstream ss_flags;

            if (uint32 unitFlag = target->GetUInt32Value(UNIT_FIELD_FLAGS))
            {
                ss_flags << unitFlag << " : ";

                if (unitFlag & UNIT_FLAG_SERVER_CONTROLLED)
                    ss_flags << "ServerControlled ";
                if (unitFlag & UNIT_FLAG_NON_ATTACKABLE)
                    ss_flags << "NonAttackable ";
                if (unitFlag & UNIT_FLAG_REMOVE_CLIENT_CONTROL)
                    ss_flags << "RemoveClientControl ";
                if (unitFlag & UNIT_FLAG_PVP_ATTACKABLE)
                    ss_flags << "PvPAttackable ";
                if (unitFlag & UNIT_FLAG_RENAME)
                    ss_flags << "Rename ";
                if (unitFlag & UNIT_FLAG_PREPARATION)
                    ss_flags << "Preparation ";
                if (unitFlag & UNIT_FLAG_UNK_6)
                    ss_flags << "Unk6 ";
                if (unitFlag & UNIT_FLAG_NOT_ATTACKABLE_1)
                    ss_flags << "NotAttackable1 ";
                if (unitFlag & UNIT_FLAG_IMMUNE_TO_PC)
                    ss_flags << "ImmuneToPC ";
                if (unitFlag & UNIT_FLAG_IMMUNE_TO_NPC)
                    ss_flags << "ImmuneToNPC ";
                if (unitFlag & UNIT_FLAG_LOOTING)
                    ss_flags << "Looting ";
                if (unitFlag & UNIT_FLAG_PET_IN_COMBAT)
                    ss_flags << "PetInCombat ";
                if (unitFlag & UNIT_FLAG_PVP)
                    ss_flags << "PVP ";
                if (unitFlag & UNIT_FLAG_SILENCED)
                    ss_flags << "Silenced ";
                if (unitFlag & UNIT_FLAG_CANNOT_SWIM)
                    ss_flags << "CannotSwim ";
                if (unitFlag & UNIT_FLAG_UNK_15)
                    ss_flags << "Unk15 ";
                if (unitFlag & UNIT_FLAG_UNK_16)
                    ss_flags << "Unk16 ";
                if (unitFlag & UNIT_FLAG_PACIFIED)
                    ss_flags << "Pacified ";
                if (unitFlag & UNIT_FLAG_STUNNED)
                    ss_flags << "Stunned ";
                if (unitFlag & UNIT_FLAG_IN_COMBAT)
                    ss_flags << "InCombat ";
                if (unitFlag & UNIT_FLAG_TAXI_FLIGHT)
                    ss_flags << "TaxiFlight ";
                if (unitFlag & UNIT_FLAG_DISARMED)
                    ss_flags << "Disarmed ";
                if (unitFlag & UNIT_FLAG_CONFUSED)
                    ss_flags << "Confused ";
                if (unitFlag & UNIT_FLAG_FLEEING)
                    ss_flags << "Fleeing ";
                if (unitFlag & UNIT_FLAG_PLAYER_CONTROLLED)
                    ss_flags << "PlayerControlled ";
                if (unitFlag & UNIT_FLAG_NOT_SELECTABLE)
                    ss_flags << "NotSelectable ";
                if (unitFlag & UNIT_FLAG_SKINNABLE)
                    ss_flags << "Skinnable ";
                if (unitFlag & UNIT_FLAG_MOUNT)
                    ss_flags << "Mount ";
                if (unitFlag & UNIT_FLAG_PREVENT_KNEELING_WHEN_LOOTING)
                    ss_flags << "PreventKWL ";
                if (unitFlag & UNIT_FLAG_PREVENT_EMOTES)
                    ss_flags << "PreventEmotes ";
                if (unitFlag & UNIT_FLAG_SHEATHE)
                    ss_flags << "Sheathe ";
                if (unitFlag & UNIT_FLAG_UNK_31)
                    ss_flags << "Unk31";
            }
            else
                ss_flags << "0";

            handler->PSendSysMessage("UnitFlags: %s", ss_flags.str().c_str());
        }

        {
            std::ostringstream ss_flags;

            if (uint32 dynamicFlags = target->GetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS))
            {
                ss_flags << dynamicFlags << " : ";

                if (dynamicFlags & UNIT_DYNFLAG_HIDE_MODEL)
                    ss_flags << "HideModel ";
                if (dynamicFlags & UNIT_DYNFLAG_NOT_SELECTABLE_MODEL)
                    ss_flags << "NotSlectableModel ";
                if (dynamicFlags & UNIT_DYNFLAG_LOOTABLE)
                    ss_flags << "Lootable ";
                if (dynamicFlags & UNIT_DYNFLAG_TRACK_UNIT)
                    ss_flags << "TrackUnit ";
                if (dynamicFlags & UNIT_DYNFLAG_TAPPED)
                    ss_flags << "Tapped ";
                if (dynamicFlags & UNIT_DYNFLAG_SPECIALINFO)
                    ss_flags << "SpecialInfo ";
                if (dynamicFlags & UNIT_DYNFLAG_REFER_A_FRIEND)
                    ss_flags << "ReferFriend ";
                if (dynamicFlags & UNIT_DYNFLAG_DISABLE_SAME_INTARACT)
                    ss_flags << "DisableSameIntaract";
            }
            else
                ss_flags << "0";

            handler->PSendSysMessage("DynamicFlags: %s", ss_flags.str().c_str());
        }

        handler->PSendSysMessage(LANG_NPCINFO_LEVEL, target->getLevel());
        handler->PSendSysMessage(LANG_NPCINFO_HEALTH, target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
        handler->PSendSysMessage(LANG_NPCINFO_FLAGS, target->getFaction());
        handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
        handler->PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->lootid, cInfo->pickpocketLootId, cInfo->SkinLootId);
        handler->PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());
        handler->PSendSysMessage(LANG_NPCINFO_PHASEMASK, target->GetPhaseMask());
        handler->PSendSysMessage(LANG_NPCINFO_ARMOR, target->GetArmor());
        handler->PSendSysMessage(LANG_NPCINFO_POSITION, float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()));
        handler->PSendSysMessage(LANG_NPCINFO_AIINFO, target->GetNPCAIName().c_str(), target->GetScriptName().c_str());

        if (cInfo->VehicleId)
            handler->PSendSysMessage("Vehicle Id: %u", cInfo->VehicleId);

        return true;
    }

    static bool HandleNpcFlagsInfoCommand(ChatHandler* handler, const char* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = target->GetCreatureTemplate();
        uint32 mechanicImmuneMask = cInfo->MechanicImmuneMask;
        uint32 dynamicFlags = target->GetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS);
        uint32 fieldFlags = target->GetUInt32Value(UNIT_FIELD_FLAGS);
        uint32 fieldFlags2 = target->GetUInt32Value(UNIT_FIELD_FLAGS_2);

        handler->PSendSysMessage(LANG_NPCINFO_FLAGS, fieldFlags, fieldFlags2, dynamicFlags, target->getFaction());
        for (uint8 i = 0; i < MAX_UNIT_FLAGS; ++i)
            if (fieldFlags & unitFlags[i].Value)
                handler->PSendSysMessage("%s (0x%X)", unitFlags[i].Name, unitFlags[i].Value);

        for (uint8 i = 0; i < MAX_UNIT_FLAGS2; ++i)
            if (fieldFlags2 & unitFlags2[i].Value)
                handler->PSendSysMessage("%s (0x%X)", unitFlags2[i].Name, unitFlags2[i].Value);

        for (uint8 i = 0; i < MAX_UNIT_DYNFLAGS; ++i)
            if (dynamicFlags & dynFlags[i].Value)
                handler->PSendSysMessage("%s (0x%X)", dynFlags[i].Name, dynFlags[i].Value);

        handler->PSendSysMessage(LANG_NPCINFO_FLAGS_EXTRA, cInfo->flags_extra);
        for (uint8 i = 0; i < FLAGS_EXTRA_COUNT; ++i)
            if (cInfo->flags_extra & flagsExtra[i].Value)
                handler->PSendSysMessage("%s (0x%X)", flagsExtra[i].Name, flagsExtra[i].Value);

        handler->PSendSysMessage(LANG_NPCINFO_MECHANIC_IMMUNE, mechanicImmuneMask);
        for (uint8 i = 1; i < MAX_MECHANIC; ++i)
            if (mechanicImmuneMask & (1 << (mechanicImmunes[i].Value - 1)))
                handler->PSendSysMessage("%s (0x%X)", mechanicImmunes[i].Name, mechanicImmunes[i].Value);

        return true;
    }

    //move selected creature
    static bool HandleNpcMoveCommand(ChatHandler* handler, const char* args)
    {
        ObjectGuid::LowType lowguid = UI64LIT(0);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            lowguid = strtoull(cId, nullptr, 10);

            /* FIXME: impossible without entry
            if (lowguid)
                creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HighGuid::Creature));
            */

            // Attempting creature load from DB data
            if (!creature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if (!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                uint32 map_id = data->mapid;

                if (handler->GetSession()->GetPlayer()->GetMapId() != map_id)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = creature->GetDBTableGUIDLow();
            }
        }
        else
        {
            lowguid = creature->GetDBTableGUIDLow();
        }

        float x = handler->GetSession()->GetPlayer()->GetPositionX();
        float y = handler->GetSession()->GetPlayer()->GetPositionY();
        float z = handler->GetSession()->GetPlayer()->GetPositionZ();
        float o = handler->GetSession()->GetPlayer()->GetOrientation();

        if (creature)
        {
            if (CreatureData const* data = sObjectMgr->GetCreatureData(creature->GetDBTableGUIDLow()))
            {
                const_cast<CreatureData*>(data)->posX = x;
                const_cast<CreatureData*>(data)->posY = y;
                const_cast<CreatureData*>(data)->posZ = z;
                const_cast<CreatureData*>(data)->orientation = o;
            }
            creature->NearTeleportTo(x, y, z, o);
            creature->SetPosition(x, y, z, o);
            creature->GetMotionMaster()->Initialize();
            if (creature->isAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn();
            }
        }

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_POSITION);

        stmt->setFloat(0, x);
        stmt->setFloat(1, y);
        stmt->setFloat(2, z);
        stmt->setFloat(3, o);
        stmt->setUInt64(4, lowguid);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_CREATUREMOVED);
        return true;
    }

    //play npc emote
    static bool HandleNpcPlayEmoteCommand(ChatHandler* handler, const char* args)
    {
        uint32 emote = atoi((char*)args);

        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, emote);

        return true;
    }

    //set model of creature
    static bool HandleNpcSetModelCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 displayId = (uint32) atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature || creature->isPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetDisplayId(displayId);
        creature->SetNativeDisplayId(displayId);
        creature->SetOutfit(displayId);
        creature->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);

        creature->SaveToDB();

        return true;
    }

    /**HandleNpcSetMoveTypeCommand
    * Set the movement type for an NPC.<br/>
    * <br/>
    * Valid movement types are:
    * <ul>
    * <li> stay - NPC wont move </li>
    * <li> random - NPC will move randomly according to the spawndist </li>
    * <li> way - NPC will move with given waypoints set </li>
    * </ul>
    * additional parameter: NODEL - so no waypoints are deleted, if you
    *                       change the movement type
    */
    static bool HandleNpcSetMoveTypeCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        // 3 arguments:
        // GUID (optional - you can also select the creature)
        // stay|random|way (determines the kind of movement)
        // NODEL (optional - tells the system NOT to delete any waypoints)
        //        this is very handy if you want to do waypoints, that are
        //        later switched on/off according to special events (like escort
        //        quests, etc)
        char* guid_str = strtok((char*)args, " ");
        char* type_str = strtok((char*)NULL, " ");
        char* dontdel_str = strtok((char*)NULL, " ");

        bool doNotDelete = false;

        if (!guid_str)
            return false;

        uint32 lowguid = 0;
        Creature* creature = NULL;

        if (dontdel_str)
        {
            //TC_LOG_ERROR(LOG_FILTER_GENERAL, "DEBUG: All 3 params are set");

            // All 3 params are set
            // GUID
            // type
            // doNotDEL
            if (stricmp(dontdel_str, "NODEL") == 0)
            {
                //TC_LOG_ERROR(LOG_FILTER_GENERAL, "DEBUG: doNotDelete = true;");
                doNotDelete = true;
            }
        }
        else
        {
            // Only 2 params - but maybe NODEL is set
            if (type_str)
            {
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "DEBUG: Only 2 params ");
                if (stricmp(type_str, "NODEL") == 0)
                {
                    //TC_LOG_ERROR(LOG_FILTER_GENERAL, "DEBUG: type_str, NODEL ");
                    doNotDelete = true;
                    type_str = NULL;
                }
            }
        }

        if (!type_str)                                           // case .setmovetype $move_type (with selected creature)
        {
            type_str = guid_str;
            creature = handler->getSelectedCreature();
            if (!creature || creature->isPet())
                return false;
            lowguid = creature->GetDBTableGUIDLow();
        }
        else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
        {
            lowguid = strtoull(guid_str, nullptr, 10);

            /* impossible without entry
            if (lowguid)
                creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HighGuid::Creature));
            */

            // attempt check creature existence by DB data
            if (!creature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if (!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = creature->GetDBTableGUIDLow();
            }
        }

        // now lowguid is low guid really existed creature
        // and creature point (maybe) to this creature or NULL

        MovementGeneratorType move_type;

        std::string type = type_str;

        if (type == "stay")
            move_type = IDLE_MOTION_TYPE;
        else if (type == "random")
            move_type = RANDOM_MOTION_TYPE;
        else if (type == "way")
            move_type = WAYPOINT_MOTION_TYPE;
        else
            return false;

        // update movement type
        //if (doNotDelete == false)
        //    WaypointMgr.DeletePath(lowguid);

        if (creature)
        {
            // update movement type
            if (doNotDelete == false)
                creature->LoadPath(0);

            creature->SetDefaultMovementType(move_type);
            creature->GetMotionMaster()->Initialize();
            if (creature->isAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn();
            }
            creature->SaveToDB();
        }
        if (doNotDelete == false)
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET, type_str);
        }
        else
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET_NODEL, type_str);
        }

        return true;
    }

    //npc phasemask handling
    //change phasemask of creature or pet
    static bool HandleNpcSetPhaseCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 phasemask = (uint32) atoi((char*)args);
        if (phasemask == 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetPhaseMask(phasemask, true);

        if (!creature->isPet())
            creature->SaveToDB();

        return true;
    }

    //set spawn dist of creature
    static bool HandleNpcSetSpawnDistCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        float option = (float)(atof((char*)args));
        if (option < 0.0f)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            return false;
        }

        MovementGeneratorType mtype = IDLE_MOTION_TYPE;
        if (option >0.0f)
            mtype = RANDOM_MOTION_TYPE;

        Creature* creature = handler->getSelectedCreature();
        uint32 guidLow = 0;

        if (creature)
            guidLow = creature->GetDBTableGUIDLow();
        else
            return false;

        creature->SetRespawnRadius((float)option);
        creature->SetDefaultMovementType(mtype);
        creature->GetMotionMaster()->Initialize();
        if (creature->isAlive())                                // dead creature will reset movement generator at respawn
        {
            creature->setDeathState(JUST_DIED);
            creature->Respawn();
        }

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_DISTANCE);

        stmt->setFloat(0, option);
        stmt->setUInt8(1, uint8(mtype));
        stmt->setUInt64(2, guidLow);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_SPAWNDIST, option);
        return true;
    }

    //spawn time handling
    static bool HandleNpcSetSpawnTimeCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* stime = strtok((char*)args, " ");

        if (!stime)
            return false;

        int spawnTime = atoi((char*)stime);

        if (spawnTime < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        uint32 guidLow = 0;

        if (creature)
            guidLow = creature->GetDBTableGUIDLow();
        else
            return false;

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS);

        stmt->setUInt32(0, uint32(spawnTime));
        stmt->setUInt64(1, guidLow);

        WorldDatabase.Execute(stmt);

        creature->SetRespawnDelay((uint32)spawnTime);
        handler->PSendSysMessage(LANG_COMMAND_SPAWNTIME, spawnTime);

        return true;
    }

    static bool HandleNpcSayCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->MonsterSay(args, LANG_UNIVERSAL, ObjectGuid::Empty);

        // make some emotes
        char lastchar = args[strlen(args) - 1];
        switch (lastchar)
        {
        case '?':   creature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
        case '!':   creature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
        default:    creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
        }

        return true;
    }

    //show text emote by creature in chat
    static bool HandleNpcTextEmoteCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->MonsterTextEmote(args, ObjectGuid::Empty);

        return true;
    }

    //npc unfollow handling
    static bool HandleNpcUnFollowCommand(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (/*creature->GetMotionMaster()->empty() ||*/
            creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName());
            handler->SetSentErrorMessage(true);
            return false;
        }

        FollowMovementGenerator<Creature> const* mgen = static_cast<FollowMovementGenerator<Creature> const*>((creature->GetMotionMaster()->top()));

        if (mgen->GetTarget() != player)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // reset movement
        creature->GetMotionMaster()->MovementExpired(true);

        handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName());
        return true;
    }

    // make npc whisper to player
    static bool HandleNpcWhisperCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* receiver_str = strtok((char*)args, " ");
        char* text = strtok(NULL, "");

        ObjectGuid guid = handler->GetSession()->GetPlayer()->GetSelection();
        Creature* creature = handler->GetSession()->GetPlayer()->GetMap()->GetCreature(guid);

        if (!creature || !receiver_str || !text)
        {
            return false;
        }

        ObjectGuid receiver_guid = ObjectGuid::Create<HighGuid::Player>(atol(receiver_str));

        // check online security
        if (handler->HasLowerSecurity(ObjectAccessor::FindPlayer(receiver_guid), ObjectGuid::Empty))
            return false;

        creature->MonsterWhisper(text, receiver_guid);

        return true;
    }

    static bool HandleNpcYellCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->MonsterYell(args, LANG_UNIVERSAL, ObjectGuid::Empty);

        // make an emote
        creature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

        return true;
    }

    // add creature, temp only
    static bool HandleNpcAddTempSpawnCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;
        char* charID = strtok((char*)args, " ");
        if (!charID)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();

        uint32 id = atoi(charID);
        if (!id)
            return false;

        chr->SummonCreature(id, *chr, TEMPSUMMON_CORPSE_DESPAWN, 120);

        return true;
    }

    //npc tame handling
    static bool HandleNpcTameCommand(ChatHandler* handler, const char* /*args*/)
    {
        Creature* creatureTarget = handler->getSelectedCreature();
        if (!creatureTarget || creatureTarget->isPet())
        {
            handler->PSendSysMessage (LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage (true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        if (player->GetPetGUID())
        {
            handler->SendSysMessage (LANG_YOU_ALREADY_HAVE_PET);
            handler->SetSentErrorMessage (true);
            return false;
        }

        CreatureTemplate const* cInfo = creatureTarget->GetCreatureTemplate();

        if (!cInfo->isTameable(player))
        {
            handler->PSendSysMessage (LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage (true);
            return false;
        }

        // Everything looks OK, create new pet
        Pet* pet = player->CreateTamedPetFrom(creatureTarget);
        if (!pet)
        {
            handler->PSendSysMessage (LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage (true);
            return false;
        }

        // place pet before player
        float x, y, z;
        player->GetClosePoint (x, y, z, creatureTarget->GetObjectSize(), CONTACT_DISTANCE);
        pet->Relocate(x, y, z, M_PI-player->GetOrientation());

        // set pet to defensive mode by default (some classes can't control controlled pets in fact).
        pet->SetReactState(REACT_DEFENSIVE);

        // calculate proper level
        uint8 level = (creatureTarget->getLevel() < (player->getLevel() - 5)) ? (player->getLevel() - 5) : creatureTarget->getLevel();

        // prepare visual effect for levelup
        pet->SetLevel(level - 1);

        // add to world
        pet->GetMap()->AddToMap(pet->ToCreature());

        // visual effect for levelup
        pet->SetLevel(level);
        pet->SetEffectiveLevel(player->GetEffectiveLevel());

        // caster have pet now
        player->SetMinion(pet, true);

        pet->SavePetToDB();
        player->PetSpellInitialize();

        return true;
    }

    static bool HandleNpcAddFormationCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 leaderGUID = strtoull(args, nullptr, 10);
        Creature* creature = handler->getSelectedCreature();

        if (!creature || !creature->GetDBTableGUIDLow())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 lowguid = creature->GetDBTableGUIDLow();
        if (creature->GetFormation())
        {
            handler->PSendSysMessage("Selected creature is already member of group %u", creature->GetFormation()->GetId());
            return false;
        }

        if (!lowguid)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();
        FormationInfo* group_member;

        group_member                 = new FormationInfo;
        group_member->follow_angle   = (creature->GetAngle(chr) - chr->GetOrientation()) * 180 / M_PI;
        group_member->follow_dist    = sqrtf(pow(chr->GetPositionX() - creature->GetPositionX(), int(2))+pow(chr->GetPositionY() - creature->GetPositionY(), int(2)));
        group_member->leaderGUID     = leaderGUID;
        group_member->groupAI        = 0;

        sFormationMgr->CreatureGroupMap[lowguid] = group_member;
        creature->SearchFormation();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE_FORMATION);

        stmt->setUInt64(0, leaderGUID);
        stmt->setUInt64(1, lowguid);
        stmt->setFloat(2, group_member->follow_dist);
        stmt->setFloat(3, group_member->follow_angle);
        stmt->setUInt32(4, uint32(group_member->groupAI));

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage("Creature %u added to formation with leader %u", lowguid, leaderGUID);

        return true;
    }

    static bool HandleNpcSetLinkCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 linkguid = strtoull(args, nullptr, 10);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!creature->GetDBTableGUIDLow())
        {
            handler->PSendSysMessage("Selected creature %u isn't in creature table", creature->GetGUID().GetGUIDLow());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sObjectMgr->SetCreatureLinkedRespawn(creature->GetDBTableGUIDLow(), linkguid))
        {
            handler->PSendSysMessage("Selected creature can't link with guid '%u'", linkguid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("LinkGUID '%u' added to creature with DBTableGUID: '%u'", linkguid, creature->GetDBTableGUIDLow());
        return true;
    }

    //TODO: NpcCommands that need to be fixed :
    static bool HandleNpcAddWeaponCommand(ChatHandler* /*handler*/, const char* /*args*/)
    {
        /*if (!*args)
            return false;

        ObjectGuid guid = handler->GetSession()->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            return true;
        }

        Creature* creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), guid);

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        char* pSlotID = strtok((char*)args, " ");
        if (!pSlotID)
            return false;

        char* pItemID = strtok(NULL, " ");
        if (!pItemID)
            return false;

        uint32 ItemID = atoi(pItemID);
        uint32 SlotID = atoi(pSlotID);

        ItemTemplate* tmpItem = sObjectMgr->GetItemTemplate(ItemID);

        bool added = false;
        if (tmpItem)
        {
            switch (SlotID)
            {
                case 1:
                    creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ItemID);
                    added = true;
                    break;
                case 2:
                    creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ItemID);
                    added = true;
                    break;
                case 3:
                    creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ItemID);
                    added = true;
                    break;
                default:
                    handler->PSendSysMessage(LANG_ITEM_SLOT_NOT_EXIST, SlotID);
                    added = false;
                    break;
            }

            if (added)
                handler->PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT, ItemID, tmpItem->Name1, SlotID);
        }
        else
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_FOUND, ItemID);
            return true;
        }
        */
        return true;
    }

    static bool HandleNpcSetNameCommand(ChatHandler* /*handler*/, const char* /*args*/)
    {
        /* Temp. disabled
        if (!*args)
            return false;

        if (strlen((char*)args)>75)
        {
            handler->PSendSysMessage(LANG_TOO_LONG_NAME, strlen((char*)args)-75);
            return true;
        }

        for (uint8 i = 0; i < strlen(args); ++i)
        {
            if (!isalpha(args[i]) && args[i] != ' ')
            {
                handler->SendSysMessage(LANG_CHARS_ONLY);
                return false;
            }
        }

        ObjectGuid guid;
        guid = handler->GetSession()->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            return true;
        }

        Creature* creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), guid);

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        creature->SetName(args);
        uint32 idname = sObjectMgr->AddCreatureTemplate(creature->GetName());
        creature->SetUInt32Value(OBJECT_FIELD_ENTRY_ID, idname);

        creature->SaveToDB();
        */

        return true;
    }

    static bool HandleNpcSetSubNameCommand(ChatHandler* /*handler*/, const char* /*args*/)
    {
        /* Temp. disabled

        if (!*args)
            args = "";

        if (strlen((char*)args)>75)
        {
            handler->PSendSysMessage(LANG_TOO_LONG_SUBNAME, strlen((char*)args)-75);
            return true;
        }

        for (uint8 i = 0; i < strlen(args); i++)
        {
            if (!isalpha(args[i]) && args[i] != ' ')
            {
                handler->SendSysMessage(LANG_CHARS_ONLY);
                return false;
            }
        }
        ObjectGuid guid;
        guid = handler->GetSession()->GetPlayer()->GetSelection();
        if (guid == 0)
        {
            handler->SendSysMessage(LANG_NO_SELECTION);
            return true;
        }

        Creature* creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), guid);

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            return true;
        }

        uint32 idname = sObjectMgr->AddCreatureSubName(creature->GetName(), args, creature->GetUInt32Value(UNIT_FIELD_DISPLAY_ID));
        creature->SetUInt32Value(OBJECT_FIELD_ENTRY_ID, idname);

        creature->SaveToDB();
        */
        return true;
    }

    static bool HandleNpcActivateCommand(ChatHandler* handler, const char* args)
    {
        Creature* unit = NULL;

        if (*args)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = strtoull(cId, nullptr, 10);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
                unit = handler->GetSession()->GetPlayer()->GetMap()->GetCreature(ObjectGuid::Create<HighGuid::Creature>(cr_data->mapid, cr_data->id, lowguid));
        }
        else
            unit = handler->getSelectedCreature();

        if (!unit || unit->isPet() || unit->isTotem())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Activate
        unit->setActive(!unit->isActiveObject());
        WorldDatabase.PExecute("UPDATE creature SET isActive = %u WHERE guid = %u", uint8(unit->isActiveObject()), unit->GetGUID().GetGUIDLow());

        if (unit->isActiveObject())
            handler->PSendSysMessage("Creature added to actived creatures !");
        else
            handler->PSendSysMessage("Creature removed from actived creatures !");

        return true;
    }

    static bool HandleNpcNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : (float)(atof(args));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_NEAREST);
        stmt->setFloat(0, player->GetPositionX());
        stmt->setFloat(1, player->GetPositionY());
        stmt->setFloat(2, player->GetPositionZ());
        stmt->setUInt32(3, player->GetMapId());
        stmt->setFloat(4, player->GetPositionX());
        stmt->setFloat(5, player->GetPositionY());
        stmt->setFloat(6, player->GetPositionZ());
        stmt->setFloat(7, distance * distance);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();

                CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(entry);

                if (!cInfo)
                    continue;

                handler->PSendSysMessage(LANG_NPC_LIST_CHAT, guid, entry, guid, cInfo->Name[0].c_str(), x, y, z, mapId);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE, distance, count);
        return true;
    }

    static bool HandleNpcSummonGroupCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        char* groupIdtr = strtok((char*)args, " ");
        char* delStr = strtok(NULL, " ");

        uint32 groupId = groupIdtr ? atoi(groupIdtr) : 0;
        uint32 del = delStr ? atoi(delStr) : 0;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if(del)
            creature->SummonCreatureGroupDespawn(groupId);
        else
            creature->SummonCreatureGroup(groupId);

        handler->PSendSysMessage("Summon groupId '%u'", groupId);
        return true;
    }
    
    static bool HandleNpcSetSizeCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        float scale = (float)atof((char*)args);

        if (scale < 0.0f)
        {
            handler->PSendSysMessage("Size of creture cant'be negative");
            return false;
        }

        if (scale == 0.0f)
        {
            CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(creature->GetEntry());
            if (!cinfo)
                return false;
            
            creature->SetObjectScale(cinfo->scale);
        }
        else
            creature->SetObjectScale(scale);
        
        if (!creature->isPet())
        {
            CreatureData& data = sObjectMgr->NewOrExistCreatureData(creature->GetDBTableGUIDLow());
            data.personalSize = scale;
            
            creature->SaveToDB();
        }
        return true;
    }
};

void AddSC_npc_commandscript()
{
    new npc_commandscript();
}
