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

/*
playerdump.cpp if you get pdump
unknown errors try using DTT_CHAR_TABLE.
*/

#include "Common.h"
#include "PlayerDump.h"
#include "DatabaseEnv.h"
#include "UpdateFields.h"
#include "ObjectMgr.h"
#include "AccountMgr.h"
#include "GarrisonMgr.h"
#include "CharacterData.h"
#include "GlobalFunctional.h"

#define DUMP_TABLE_COUNT 32
struct DumpTable
{
    char const* name;
    DumpTableType type;
    char const* select;
};

//REMEMBER TO UPDATE DUMP_TABLE_COUNT!!!
static DumpTable dumpTables[DUMP_TABLE_COUNT] =
{
    { "characters",                       DTT_CHARACTER, "`guid`, `account`, `name`, `slot`, `race`, `class`, `gender`, `level`, `xp`, `money`, `skin`, `face`, `hairStyle`, `hairColor`, `tattoo`, `horn`, `inventorySlots`, `blindfold`, `facialStyle`, `bankSlots`, `drunk`, `playerFlags`, `playerFlagsEx`, `position_x`, `position_y`, `position_z`, `map`, `instance_id`, `dungeonDifficulty`, `raidDifficulty`, `legacyRaidDifficulty`, `orientation`, `taximask`, `online`, `cinematic`, `totaltime`, `leveltime`, `created_time`, `logout_time`, `is_logout_resting`, `rest_bonus`, `trans_x`, `trans_y`, `trans_z`, `trans_o`, `transguid`, `extra_flags`, `at_login`, `zone`, `death_expire_time`, `taxi_path`, `totalKills`, `todayKills`, `yesterdayKills`, `killPoints`, `chosenTitle`, `watchedFaction`, `lfgBonusFaction`, `health`, `mana`, `latency`, `activespec`, `specialization`, `lootspecialization`, `exploredZones`, `equipmentCache`, `knownTitles`, `actionBars`, `currentpetnumber`, `petslot`, `grantableLevels`, `deleteInfos_Account`, `deleteInfos_Name`, `deleteDate`, `LastCharacterUndelete`, `transfer`, `transfer_request`, `transfer_history`"},
    { "character_donate",                 DTT_DONA_TABLE, "`owner_guid`, `itemguid`, `type`, `itemEntry`, `efircount`, `count`, `state`, `date`, `deletedate`, `account`"},
    { "character_achievement",            DTT_CHAR_TABLE, "`guid`, `achievement`, `date`"},
    { "character_achievement_progress",   DTT_CHAR_TABLE, "`guid`, `criteria`, `counter`, `date`"},
    { "character_action",                 DTT_CHAR_TABLE, "`guid`, `spec`, `button`, `action`, `type`"},
    { "character_currency",               DTT_CHAR_TABLE, "`guid`, `currency`, `total_count`, `week_count`, `season_total`, `flags`, `curentcap`"},
    { "character_declinedname",           DTT_CHAR_TABLE, "`guid`, `genitive`, `dative`, `accusative`, `instrumental`, `prepositional`"},
    { "character_equipmentsets",          DTT_EQSET_TABLE,"`guid`, `setguid`, `setindex`, `name`, `iconname`, `ignore_mask`, `item0`, `item1`, `item2`, `item3`, `item4`, `item5`, `item6`, `item7`, `item8`, `item9`, `item10`, `item11`, `item12`, `item13`, `item14`, `item15`, `item16`, `item17`, `item18`"},
    { "character_homebind",               DTT_CHAR_TABLE, "`guid`, `mapId`, `zoneId`, `posX`, `posY`, `posZ`"},
    { "character_inventory",              DTT_INVENTORY,  "`guid`, `bag`, `slot`, `item`"},
    { "character_pet",                    DTT_PET,        "`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `curhealth`, `curmana`, `savetime`, `abdata`, `specialization`"},
    { "character_pet_declinedname",       DTT_PET,        "`id`, `owner`, `genitive`, `dative`, `accusative`, `instrumental`, `prepositional`"},
    { "character_queststatus",            DTT_CHAR_TABLE, "`guid`, `account`, `quest`, `status`, `timer`"},
    { "character_queststatus_rewarded",   DTT_CHAR_TABLE, "`guid`, `account`, `quest`"},
    { "character_queststatus_objectives", DTT_CHAR_TABLE, "`guid`, `account`, `quest`, `objective`, `data`"},
    { "character_reputation",             DTT_CHAR_TABLE, "`guid`, `faction`, `standing`, `flags`"},
    { "character_skills",                 DTT_CHAR_TABLE, "`guid`, `skill`, `value`, `max`"},
    { "character_spell",                  DTT_CHAR_TABLE, "`guid`, `spell`, `active`, `disabled`"},
    { "character_spell_cooldown",         DTT_CHAR_TABLE, "`guid`, `spell`, `item`, `time`"},
    { "character_talent",                 DTT_CHAR_TABLE, "`guid`, `talent`, `spec`"},
    { "character_void_storage",           DTT_VS_TABLE,   "`itemId`, `playerGuid`, `itemEntry`, `slot`, `creatorGuid`, `randomProperty`, `suffixFactor`, `randomPropertyType`, `itemGuid`"},
    { "item_instance",                    DTT_ITEM,       "`guid`, `itemEntry`, `owner_guid`, `creatorGuid`, `giftCreatorGuid`, `count`, `duration`, `charges`, `flags`, `enchantments`, `randomPropertyType`, `randomPropertyId`, `durability`, `playedTime`, `text`, `upgradeId`, `battlePetSpeciesId`, `battlePetBreedData`, `battlePetLevel`, `battlePetDisplayId`, `bonusListIDs`, `itemLevel`, `dungeonEncounterID`, `contextID`, `createdTime`, `isdonateitem`"},
    { "mail",                             DTT_MAIL,       "`id`, `messageType`, `stationery`, `mailTemplateId`, `sender`, `receiver`, `subject`, `body`, `has_items`, `expire_time`, `deliver_time`, `money`, `cod`, `checked`"},
    { "mail_items",                       DTT_MAIL_ITEM,  "`mail_id`, `item_guid`, `receiver`"},
    { "pet_spell",                        DTT_PET_TABLE,  "`guid`, `spell`, `active`"},
    { "pet_spell_cooldown",               DTT_PET_TABLE,  "`guid`, `spell`, `time`"},
    { "character_archaeology",            DTT_CHAR_TABLE, "`guid`, `sites`, `counts`, `projects`"},
    { "character_archaeology_finds",      DTT_CHAR_TABLE, "`guid`, `id`, `count`, `date`"},
    //{ "character_garrison",               DTT_CHAR_TABLE, "`guid`, `siteLevelId`, `followerActivationsRemainingToday`, `lastResTaken`, `_MissionGen`"},
    //{ "character_garrison_blueprints",    DTT_CHAR_TABLE, "`guid`, `buildingId`"},
    //{ "character_garrison_buildings",     DTT_CHAR_TABLE, "`guid`, `plotInstanceId`, `buildingId`, `timeBuilt`, `data`, `active`"},
    //{ "character_garrison_followers",     DTT_CHAR_VS_FOLLOW, "`dbId`, `guid`, `followerId`, `quality`, `level`, `itemLevelWeapon`, `itemLevelArmor`, `xp`, `currentBuilding`, `currentMission`, `status`"},
    //{ "character_garrison_shipment",      DTT_CHAR_VS_SHIP, "`dbId`, `guid`, `shipmentID`, `orderTime`"},
    { "character_transmog_outfits",       DTT_CHAR_TRANSMOG, "`guid`, `setguid`, `setindex`, `name`, `iconname`, `ignore_mask`, `appearance0`, `appearance1`, `appearance2`, `appearance3`, `appearance4`, `appearance5`, `appearance6`, `appearance7`, `appearance8`, `appearance9`, `appearance10`, `appearance11`, `appearance12`, `appearance13`, `appearance14`, `appearance15`, `appearance16`, `appearance17`, `appearance18`, `mainHandEnchant`, `offHandEnchant`"},
    //{ "item_instance_artifact",           DTT_ARTI_TABLE, "`itemGuid`, `xp`, `artifactAppearanceId`, `itemEntry`, `tier`, `char_guid`, `totalrank`"},
    //{ "item_instance_artifact_powers",    DTT_ARTI_TABLE, "`itemGuid`, `char_guid`, `artifactPowerId`, `purchasedRank`, `itemEntry`"},
    { "item_instance_gems",               DTT_ITEM_TABLE, "`itemGuid`, `gemItemId1`, `gemBonuses1`, `gemContext1`, `gemScalingLevel1`, `gemItemId2`, `gemBonuses2`, `gemScalingLevel2`, `gemContext2`, `gemItemId3`, `gemBonuses3`, `gemContext3`, `gemScalingLevel3`"},
    { "item_instance_modifiers",          DTT_ITEM_TABLE, "`itemGuid`, `fixedScalingLevel`, `artifactKnowledgeLevel`"},
    { "item_instance_transmog",           DTT_ITEM_TABLE, "`itemGuid`, `itemModifiedAppearanceAllSpecs`, `itemModifiedAppearanceSpec1`, `itemModifiedAppearanceSpec2`, `itemModifiedAppearanceSpec3`, `itemModifiedAppearanceSpec4`, `spellItemEnchantmentAllSpecs`, `spellItemEnchantmentSpec1`, `spellItemEnchantmentSpec2`, `spellItemEnchantmentSpec3`, `spellItemEnchantmentSpec4`"}
};

// Low level functions
static bool FindTokNth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    int i; s = e = 0;
    std::string::size_type size = str.size();
    for (i = 1; s < size && i < n; s++) if (str[s] == ' ') ++i;
    if (i < n)
        return false;

    e = str.find(' ', s);

    return e != std::string::npos;
}

std::string GetTokNth(std::string &str, int n)
{
    std::string::size_type s = 0, e = 0;
    if (!FindTokNth(str, n, s, e))
        return "";

    return str.substr(s, e-s);
}

bool FindNth(std::string &str, int n, std::string::size_type &s, std::string::size_type &e)
{
    s = str.find("VALUES ('")+9;
    if (s == std::string::npos) return false;

    do
    {
        e = str.find('\'', s);
        if (e == std::string::npos) return false;
    } while (str[e-1] == '\\');

    for (int i = 1; i < n; ++i)
    {
        do
        {
            s = e+4;
            e = str.find('\'', s);
            if (e == std::string::npos) return false;
        } while (str[e-1] == '\\');
    }
    return true;
}

std::string GetTableName(std::string &str)
{
    std::string::size_type const s = 13;
    std::string::size_type e = str.find('`', s);
    if (e == std::string::npos)
        return "";

    return str.substr(s, e - s);
}

bool ChangeNth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s, e;
    if (!FindNth(str, n, s, e))
        return false;

    if (nonzero && str.substr(s, e-s) == "0")
        return true;                                        // not an error
    if (!insert)
        str.replace(s, e-s, with);
    else
        str.insert(s, with);

    return true;
}

std::string GetNth(std::string &str, int n)
{
    std::string::size_type s, e;
    if (!FindNth(str, n, s, e))
        return "";

    return str.substr(s, e-s);
}

bool ChangeTokNth(std::string &str, int n, const char *with, bool insert = false, bool nonzero = false)
{
    std::string::size_type s = 0, e = 0;
    if (!FindTokNth(str, n, s, e))
        return false;
    if (nonzero && str.substr(s, e-s) == "0")
        return true;                                        // not an error
    if (!insert)
        str.replace(s, e-s, with);
    else
        str.insert(s, with);

    return true;
}

uint64 RegisterNewGuid(uint64 oldGuid, std::map<uint64, uint64> &guidMap, uint64 hiGuid)
{
    std::map<uint64, uint64>::const_iterator itr = guidMap.find(oldGuid);
    if (itr != guidMap.end())
        return itr->second;

    uint64 newguid = hiGuid + guidMap.size();
    guidMap[oldGuid] = newguid;
    return newguid;
}

bool ChangeGuid(std::string &str, int n, std::map<uint64, uint64> &guidMap, uint32 hiGuid, bool nonzero = false)
{
    char chritem[21];
    ObjectGuid::LowType oldGuid = strtoull(GetNth(str, n).c_str(), nullptr, 10);
    if (nonzero && !oldGuid)
        return true;                                        // not an error

    uint64 newGuid = RegisterNewGuid(oldGuid, guidMap, hiGuid);
    snprintf(chritem, 21, UI64FMTD, newGuid);

    return ChangeNth(str, n, chritem, false, nonzero);
}

std::string CreateDumpString(char const* tableName, QueryResult result, char const* tableSelect)
{
    if (!tableName || !result) return "";
    std::ostringstream ss;
    ss << "INSERT INTO "<< _TABLE_SIM_ << tableName << _TABLE_SIM_ << " (" << tableSelect << ") VALUES (";
    Field* fields = result->Fetch();
    for (uint32 i = 0; i < result->GetFieldCount(); ++i)
    {
        if (i == 0) ss << '\'';
        else ss << ", '";

        std::string s = fields[i].GetString();
        CharacterDatabase.EscapeString(s);
        ss << s;

        ss << '\'';
    }
    ss << ");\n";
    return ss.str();
}

std::string PlayerDumpWriter::GenerateWhereStr(char const* field, ObjectGuid::LowType guid)
{
    std::ostringstream wherestr;
    wherestr << field << " = '" << guid << '\'';
    return wherestr.str();
}

std::string PlayerDumpWriter::GenerateWhereStr(char const* field, GUIDs const& guids, GUIDs::const_iterator& itr)
{
    std::ostringstream wherestr;
    wherestr << field << " IN ('";
    for (; itr != guids.end(); ++itr)
    {
        wherestr << *itr;

        if (wherestr.str().size() > MAX_QUERY_LEN - 50)      // near to max query
        {
            ++itr;
            break;
        }

        GUIDs::const_iterator itr2 = itr;
        if (++itr2 != guids.end())
            wherestr << "', '";
    }
    wherestr << "')";
    return wherestr.str();
}

void StoreGUID(QueryResult result, uint32 field, std::set<uint32>& guids)
{
    Field* fields = result->Fetch();
    ObjectGuid::LowType guid = fields[field].GetUInt64();
    if (guid)
        guids.insert(guid);
}

void StoreGUID(QueryResult result, uint32 data, uint32 field, std::set<uint32>& guids)
{
    Field* fields = result->Fetch();
    std::string dataStr = fields[data].GetString();
    ObjectGuid::LowType guid = strtoull(GetTokNth(dataStr, field).c_str(), nullptr, 10);
    if (guid)
        guids.insert(guid);
}

// Writing - High-level functions
bool PlayerDumpWriter::DumpTable(std::string& dump, ObjectGuid::LowType guid, char const*tableFrom, char const*tableTo, DumpTableType type, char const*tableSelect)
{
    GUIDs const* guids = nullptr;
    char const* fieldname = nullptr;

    switch (type)
    {
        case DTT_ITEM:          fieldname = "guid";      guids = &items; break;
        case DTT_ITEM_GIFT:     fieldname = "item_guid"; guids = &items; break;
        case DTT_ITEM_TABLE:    fieldname = "itemGuid";  guids = &items; break;
        case DTT_PET:           fieldname = "owner";                     break;
        case DTT_PET_TABLE:     fieldname = "guid";      guids = &pets;  break;
        case DTT_MAIL:          fieldname = "receiver";                  break;
        case DTT_MAIL_ITEM:     fieldname = "mail_id";   guids = &mails; break;
        case DTT_DONA_TABLE:    fieldname = "owner_guid";                break;
        case DTT_VS_TABLE:      fieldname = "playerGuid";                break;
        case DTT_CHAR_VS_FOLLOW:fieldname = "dbId";                      break;
        case DTT_FOLLOW:        fieldname = "dbId";  guids = &follower;  break;
        default:                fieldname = "guid";                      break;
    }

    // for guid set stop if set is empty
    if (guids && guids->empty())
        return true;                                        // nothing to do

    // setup for guids case start position
    GUIDs::const_iterator guids_itr;
    if (guids)
        guids_itr = guids->begin();

    do
    {
        std::string wherestr;

        if (guids)                                           // set case, get next guids string
            wherestr = GenerateWhereStr(fieldname, *guids, guids_itr);
        else                                                // not set case, get single guid string
            wherestr = GenerateWhereStr(fieldname, guid);

        QueryResult result = CharacterDatabase.PQuery("SELECT %s FROM %s WHERE %s", tableSelect, tableFrom, wherestr.c_str());
        if (!result)
            return true;

        do
        {
            // collect guids
            switch (type)
            {
                case DTT_INVENTORY:
                    StoreGUID(result, 3, items);                // item guid collection (character_inventory.item)
                    break;
                case DTT_PET:
                    StoreGUID(result, 0, pets);                 // pet petnumber collection (character_pet.id)
                    break;
                case DTT_MAIL:
                    StoreGUID(result, 0, mails);                // mail id collection (mail.id)
                    break;
                case DTT_MAIL_ITEM:
                    StoreGUID(result, 1, items);                // item guid collection (mail_items.item_guid)
                    break;
                case DTT_FOLLOW:
                    StoreGUID(result, 0, follower);                // item guid collection (mail_items.item_guid)
                    break;
                default:
                    break;
            }

            dump += CreateDumpString(tableTo, result, tableSelect);
        }
        while (result->NextRow());
    }
    while (guids && guids_itr != guids->end());              // not set case iterate single time, set case iterate for all guids
    return true;
}

bool PlayerDumpWriter::GetDump(ObjectGuid::LowType guid, std::string &dump)
{
    dump = "";

    //dump += "IMPORTANT NOTE: THIS DUMPFILE IS MADE FOR USE WITH THE 'PDUMP' COMMAND ONLY - EITHER THROUGH INGAME CHAT OR ON CONSOLE!;\n";
    //dump += "IMPORTANT NOTE: DO NOT apply it directly - it will irreversibly DAMAGE and CORRUPT your database! You have been warned!;\n\n";

    for (int i = 0; i < DUMP_TABLE_COUNT; ++i)
        if (!DumpTable(dump, guid, dumpTables[i].name, dumpTables[i].name, dumpTables[i].type, dumpTables[i].select))
            return false;

    // TODO: Add instance/group..
    // TODO: Add a dump level option to skip some non-important tables

    return true;
}

DumpReturn PlayerDumpWriter::WriteDump(std::string const& file, ObjectGuid::LowType guid)
{
    FILE *fout = fopen(file.c_str(), "w");
    if (!fout)
        return DUMP_FILE_OPEN_ERROR;

    DumpReturn ret = DUMP_SUCCESS;
    std::string dump;
    if (!GetDump(guid, dump))
        ret = DUMP_CHARACTER_DELETED;

    fprintf(fout, "%s\n", dump.c_str());
    fclose(fout);
    return ret;
}

DumpReturn PlayerDumpWriter::WriteDump(ObjectGuid::LowType guid, std::string& dump)
{
    if (!GetDump(guid, dump))
        return DUMP_CHARACTER_DELETED;
    return DUMP_SUCCESS;
}

// Reading - High-level functions
#define ROLLBACK(DR) {fclose(fin); return (DR);}

void fixNULLfields(std::string &line)
{
    std::string nullString("'NULL'");
    size_t pos = line.find(nullString);
    while (pos != std::string::npos)
    {
        line.replace(pos, nullString.length(), "NULL");
        pos = line.find(nullString);
    }
}

DumpReturn PlayerDumpReader::LoadDump(std::string const& file, uint32 account, std::string name, ObjectGuid::LowType guid)
{
    uint32 charcount = AccountMgr::GetCharactersCount(account);
    if (charcount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
        return DUMP_TOO_MANY_CHARS;

    FILE* fin = fopen(file.c_str(), "r");
    if (!fin)
        return DUMP_FILE_OPEN_ERROR;

    char newguid[20], chraccount[20], newpetid[20], currpetid[20], lastpetid[20], lastfollowerid[20];

    // make sure the same guid doesn't already exist and is safe to use
    bool incHighest = true;
    if (guid != 0 && guid < sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_GUID);
        stmt->setUInt32(0, guid);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (result)
            guid = sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed();                     // use first free if exists
        else incHighest = false;
    }
    else
        guid = sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed();

    // normalize the name if specified and check if it exists
    if (!normalizePlayerName(name))
        name = "";

    if (sCharacterDataStore->CheckPlayerName(name, sWorld->GetDefaultDbcLocale(), true) == CHAR_NAME_SUCCESS)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, name);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
            name = "";                                      // use the one from the dump
    }
    else
        name = "";

    // name encoded or empty

    snprintf(newguid, 20, "%llu", guid);
    snprintf(chraccount, 20, "%u", account);
    snprintf(newpetid, 20, "%u", sObjectMgr->GeneratePetNumber());
    snprintf(lastpetid, 20, "%s", "");
    snprintf(lastfollowerid, 20, "%llu", sGarrisonMgr.GenerateFollowerDbId());

    std::map<uint64, uint64> items;
    std::map<uint64, uint64> mails;
    char buf[32000] = "";

    typedef std::map<uint32, uint32> PetIds;                // old->new petid relation
    typedef PetIds::value_type PetIdsPair;
    PetIds petids;

    uint8 gender = GENDER_NONE;
    uint8 race = RACE_NONE;
    uint8 playerClass = 0;
    uint8 level = 0;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    while (!feof(fin))
    {
        if (!fgets(buf, 32000, fin))
        {
            if (feof(fin))
                break;
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        std::string line; line.assign(buf);

        // skip empty strings
        size_t nw_pos = line.find_first_not_of(" \t\n\r\7");
        if (nw_pos == std::string::npos)
            continue;

        // skip logfile-side dump start notice, the important notes and dump end notices
        if ((line.substr(nw_pos, 16) == "== START DUMP ==") ||
            (line.substr(nw_pos, 15) == "IMPORTANT NOTE:") ||
            (line.substr(nw_pos, 14) == "== END DUMP =="))
            continue;

        // determine table name and load type
        std::string tn = GetTableName(line);
        if (tn.empty())
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: Can't extract table name from line: '%s'!", line.c_str());
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        DumpTableType type = DumpTableType(0);
        uint8 i;
        for (i = 0; i < DUMP_TABLE_COUNT; ++i)
        {
            if (tn == dumpTables[i].name)
            {
                type = dumpTables[i].type;
                break;
            }
        }

        if (i == DUMP_TABLE_COUNT)
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: Unknown table: '%s'!", tn.c_str());
            ROLLBACK(DUMP_FILE_BROKEN);
        }

        // change the data to server values
        switch (type)
        {
            case DTT_CHARACTER:
            {
                if (!ChangeNth(line, 1, newguid))           // characters.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);

                if (!ChangeNth(line, 2, chraccount))        // characters.account update
                    ROLLBACK(DUMP_FILE_BROKEN);

                race = uint8(atol(GetNth(line, 5).c_str()));
                playerClass = uint8(atol(GetNth(line, 6).c_str()));
                gender = uint8(atol(GetNth(line, 7).c_str()));
                level = uint8(atol(GetNth(line, 8).c_str()));
                if (name == "")
                {
                    // check if the original name already exists
                    name = GetNth(line, 3);

                    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
                    stmt->setString(0, name);
                    PreparedQueryResult result = CharacterDatabase.Query(stmt);

                    if (result)
                        if (!ChangeNth(line, 48, "1"))       // characters.at_login set to "rename on login"
                            ROLLBACK(DUMP_FILE_BROKEN);
                }
                else if (!ChangeNth(line, 3, name.c_str())) // characters.name
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_CHAR_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))           // character_*.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_EQSET_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_equipmentsets.guid

                char newSetGuid[24];
                snprintf(newSetGuid, 24, UI64FMTD, sObjectMgr->GenerateEquipmentSetGuid());
                if (!ChangeNth(line, 2, newSetGuid))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_equipmentsets.setguid
                break;
            }
            case DTT_INVENTORY:
            {
                if (!ChangeNth(line, 1, newguid))           // character_inventory.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);

                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed(), true))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_inventory.bag update
                if (!ChangeGuid(line, 4, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_inventory.item update
                break;
            }
            case DTT_VS_TABLE:
            {
                uint64 newItemIdNum = sObjectMgr->GenerateVoidStorageItemId();
                char newItemId[20];
                snprintf(newItemId, 20, "%llu", newItemIdNum);

                if (!ChangeNth(line, 1, newItemId))           // character_void_storage.itemId update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeNth(line, 2, newguid))           // character_void_storage.playerGuid update
                    ROLLBACK(DUMP_FILE_BROKEN);            // character_void_storage.itemGuid update
                if (!ChangeGuid(line, 9, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);  
                break;
            }
            case DTT_MAIL:                                  // mail
            {
                if (!ChangeGuid(line, 1, mails, sObjectMgr->_mailId))
                    ROLLBACK(DUMP_FILE_BROKEN);             // mail.id update
                if (!ChangeNth(line, 6, newguid))           // mail.receiver update
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_MAIL_ITEM:                             // mail_items
            {
                if (!ChangeGuid(line, 1, mails, sObjectMgr->_mailId))
                    ROLLBACK(DUMP_FILE_BROKEN);             // mail_items.id
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // mail_items.item_guid
                if (!ChangeNth(line, 3, newguid))           // mail_items.receiver
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_ITEM:
            {
                // item, owner, data field:item, owner guid
                if (!ChangeGuid(line, 1, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                   ROLLBACK(DUMP_FILE_BROKEN);              // item_instance.guid update
                if (!ChangeNth(line, 3, newguid))           // item_instance.owner_guid update
                    ROLLBACK(DUMP_FILE_BROKEN);
                break;
            }
            case DTT_ITEM_GIFT:
            {
                if (!ChangeNth(line, 1, newguid))           // character_gifts.guid update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_gifts.item_guid update
                break;
            }
            case DTT_DONA_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))           // character_donate.owner_guid update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                    ROLLBACK(DUMP_FILE_BROKEN);             // character_donate.itemguid update
                break;
            }
            case DTT_PET:
            {
                //store a map of old pet id to new inserted pet id for use by type 5 tables
                snprintf(currpetid, 20, "%s", GetNth(line, 1).c_str());
                if (*lastpetid == '\0')
                    snprintf(lastpetid, 20, "%s", currpetid);
                if (strcmp(lastpetid, currpetid) != 0)
                {
                    snprintf(newpetid, 20, "%d", sObjectMgr->GeneratePetNumber());
                    snprintf(lastpetid, 20, "%s", currpetid);
                }

                std::map<uint32, uint32> :: const_iterator petids_iter = petids.find(atoi(currpetid));

                if (petids_iter == petids.end())
                {
                    petids.insert(PetIdsPair(atoi(currpetid), atoi(newpetid)));
                }

                if (!ChangeNth(line, 1, newpetid))          // character_pet.id update
                    ROLLBACK(DUMP_FILE_BROKEN);
                if (!ChangeNth(line, 3, newguid))           // character_pet.owner update
                    ROLLBACK(DUMP_FILE_BROKEN);

                break;
            }
            case DTT_PET_TABLE:                             // pet_aura, pet_spell, pet_spell_cooldown
            {
                snprintf(currpetid, 20, "%s", GetNth(line, 1).c_str());

                // lookup currpetid and match to new inserted pet id
                std::map<uint32, uint32> :: const_iterator petids_iter = petids.find(atoi(currpetid));
                if (petids_iter == petids.end())             // couldn't find new inserted id
                    ROLLBACK(DUMP_FILE_BROKEN);

                snprintf(newpetid, 20, "%d", petids_iter->second);

                if (!ChangeNth(line, 1, newpetid))
                    ROLLBACK(DUMP_FILE_BROKEN);

                break;
            }
            case DTT_CHAR_VS_FOLLOW:
            {
                if (!ChangeNth(line, 2, newguid))           // character_*.guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_garrison_followers.guid line: '%s'!", line.c_str());
                    ROLLBACK(DUMP_FILE_BROKEN);
                }
                if (!ChangeNth(line, 1, lastfollowerid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_garrison_followers.dbId update line: '%s'!", line.c_str());
                    ROLLBACK(DUMP_FILE_BROKEN);
                }
                break;
            }
            default:
                TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Unknown dump table type: %u", type);
                break;
        }

        fixNULLfields(line);

        trans->Append(line.c_str());
    }

    CharacterDatabase.CommitTransaction(trans);

    // in case of name conflict player has to rename at login anyway
    sWorld->AddCharacterInfo(guid, name, gender, race, playerClass, level, account);

    sObjectMgr->GetGenerator<HighGuid::Item>()->Set(sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed() + items.size());
    sObjectMgr->_mailId     += mails.size();

    if (incHighest)
        sObjectMgr->GetGenerator<HighGuid::Player>()->Generate();

    fclose(fin);

    return DUMP_SUCCESS;
}

DumpReturn PlayerDumpReader::LoadDump(uint32 account, std::string& dump, std::string name, uint32& guid)
{
    uint32 charcount = AccountMgr::GetCharactersCount(account);
    if (charcount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
        return DUMP_TOO_MANY_CHARS;

    char newguid[20], chraccount[20], newpetid[20], currpetid[20], lastpetid[20], lastfollowerid[20];

    // make sure the same guid doesn't already exist and is safe to use
    bool incHighest = true;
    if (guid != 0 && guid < sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_GUID);
        stmt->setUInt32(0, guid);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (result)
            guid = sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed();                     // use first free if exists
        else incHighest = false;
    }
    else
        guid = sObjectMgr->GetGenerator<HighGuid::Player>()->GetNextAfterMaxUsed();

    // normalize the name if specified and check if it exists
    if (!normalizePlayerName(name))
        name = "";

    if (sCharacterDataStore->CheckPlayerName(name, sWorld->GetDefaultDbcLocale(), true) == CHAR_NAME_SUCCESS)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, name);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        if (result)
            name = "";                                      // use the one from the dump
    }
    else
        name = "";

    // name encoded or empty

    snprintf(newguid, 20, "%u", guid);
    snprintf(chraccount, 20, "%u", account);
    snprintf(newpetid, 20, "%u", sObjectMgr->GeneratePetNumber());
    snprintf(lastpetid, 20, "%s", "");
    snprintf(lastfollowerid, 20, "%llu", sGarrisonMgr.GenerateFollowerDbId());

    std::map<uint64, uint64> items;
    std::map<uint64, uint64> mails;

    typedef std::map<uint32, uint32> PetIds;                // old->new petid relation
    typedef PetIds::value_type PetIdsPair;
    PetIds petids;

    uint8 gender = GENDER_NONE;
    uint8 race = RACE_NONE;
    uint8 playerClass = 0;
    uint8 level = 0;
    uint64 money = 0;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    SQLTransaction loginTrans = LoginDatabase.BeginTransaction();

    dump += "INSERT INTO";
    std::string insertString(";INSERT INTO");
    size_t lestpos = 0;
    size_t pos = dump.find(insertString);
    while (pos != std::string::npos)
    {
        std::string line = dump.substr(lestpos, (pos - lestpos) + 1);
        lestpos = pos + 1;
        pos = dump.find(insertString, lestpos);

        // skip empty strings
        size_t nw_pos = line.find_first_not_of(" \t\n\r\7");
        if (nw_pos == std::string::npos)
            continue;

        // determine table name and load type
        std::string tn = GetTableName(line);
        if (tn.empty())
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: Can't extract table name from line: '%s'!", line.c_str());
            return DUMP_FILE_BROKEN;
        }

        DumpTableType type = DumpTableType(0);
        uint8 i;
        for (i = 0; i < DUMP_TABLE_COUNT; ++i)
        {
            if (tn == dumpTables[i].name)
            {
                type = dumpTables[i].type;
                break;
            }
        }

        if (i == DUMP_TABLE_COUNT)
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: Unknown table: '%s'!", tn.c_str());
            continue;
        }

        // change the data to server values
        switch (type)
        {
            case DTT_CHARACTER:
            {
                if (!ChangeNth(line, 1, newguid))           // characters.guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: characters.guid update line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                if (!ChangeNth(line, 2, chraccount))        // characters.account update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: characters.account update line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                race = uint8(atol(GetNth(line, 5).c_str()));
                playerClass = uint8(atol(GetNth(line, 6).c_str()));
                gender = uint8(atol(GetNth(line, 7).c_str()));
                level = uint8(atol(GetNth(line, 8).c_str()));
                money = uint64(atol(GetNth(line, 10).c_str()));
                if(money > sWorld->getIntConfig(CONFIG_TRANSFER_GOLD_LIMIT))
                {
                    char golddata[50];
                    sprintf(golddata, "%u", sWorld->getIntConfig(CONFIG_TRANSFER_GOLD_LIMIT));
                    ChangeNth(line, 10, golddata);
                }
                if (name == "")
                {
                    // check if the original name already exists
                    name = GetNth(line, 3);

                    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
                    stmt->setString(0, name);
                    PreparedQueryResult result = CharacterDatabase.Query(stmt);

                    if (result)
                        if (!ChangeNth(line, 48, "1"))       // characters.at_login set to "rename on login"
                        {
                            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: characters.at_login set to rename on login line: '%s'!", line.c_str());
                            return DUMP_FILE_BROKEN;
                        }
                }
                else if (!ChangeNth(line, 3, name.c_str())) // characters.name
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: characters.name line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_CHAR_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))           // character_*.guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_EQSET_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_equipmentsets.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                char newSetGuid[24];
                snprintf(newSetGuid, 24, UI64FMTD, sObjectMgr->GenerateEquipmentSetGuid());
                if (!ChangeNth(line, 2, newSetGuid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_equipmentsets.setguid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_INVENTORY:
            {
                if (!ChangeNth(line, 1, newguid))           // character_inventory.guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_inventory.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed(), true))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_inventory.bag line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeGuid(line, 4, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_inventory.item line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_VS_TABLE:
            {
                uint64 newItemIdNum = sObjectMgr->GenerateVoidStorageItemId();
                char newItemId[20];
                snprintf(newItemId, 20, "%llu", newItemIdNum);

                if (!ChangeNth(line, 1, newItemId))           // character_void_storage.itemId update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_void_storage.itemId line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 2, newguid))           // character_void_storage.playerGuid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_void_storage.playerGuid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }            // character_void_storage.itemGuid update
                if (!ChangeGuid(line, 9, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_void_storage.itemGuid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_MAIL:                                  // mail
            {
                if (!ChangeGuid(line, 1, mails, sObjectMgr->_mailId))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: mail.id line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 6, newguid))           // mail.receiver update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: mail.receiver line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_MAIL_ITEM:                             // mail_items
            {
                if (!ChangeGuid(line, 1, mails, sObjectMgr->_mailId))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: mail_items.id line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: mail_items.item_guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 3, newguid))           // mail_items.receiver
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: mail_items.receiver line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_ITEM:
            {
                // item, owner, data field:item, owner guid
                if (!ChangeGuid(line, 1, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: item_instance.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 3, newguid))           // item_instance.owner_guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: item_instance.owner_guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_ITEM_GIFT:
            {
                if (!ChangeNth(line, 1, newguid))           // character_gifts.guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_gifts.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_gifts.item_guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_DONA_TABLE:
            {
                if (!ChangeNth(line, 1, newguid))           // character_donate.owner_guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_donate.owner_guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeGuid(line, 2, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_donate.itemguid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_PET:
            {
                //store a map of old pet id to new inserted pet id for use by type 5 tables
                snprintf(currpetid, 20, "%s", GetNth(line, 1).c_str());
                if (*lastpetid == '\0')
                    snprintf(lastpetid, 20, "%s", currpetid);
                if (strcmp(lastpetid, currpetid) != 0)
                {
                    snprintf(newpetid, 20, "%d", sObjectMgr->GeneratePetNumber());
                    snprintf(lastpetid, 20, "%s", currpetid);
                }

                std::map<uint32, uint32> :: const_iterator petids_iter = petids.find(atoi(currpetid));

                if (petids_iter == petids.end())
                {
                    petids.insert(PetIdsPair(atoi(currpetid), atoi(newpetid)));
                }

                if (!ChangeNth(line, 1, newpetid))          // character_pet.id update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_pet.id line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 3, newguid))           // character_pet.owner update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_pet.owne line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                break;
            }
            case DTT_PET_TABLE:                             // pet_aura, pet_spell, pet_spell_cooldown
            {
                snprintf(currpetid, 20, "%s", GetNth(line, 1).c_str());

                // lookup currpetid and match to new inserted pet id
                std::map<uint32, uint32> :: const_iterator petids_iter = petids.find(atoi(currpetid));
                if (petids_iter == petids.end())             // couldn't find new inserted id
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: pet line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                snprintf(newpetid, 20, "%d", petids_iter->second);

                if (!ChangeNth(line, 1, newpetid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: pet line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_CHAR_VS_FOLLOW:
            {
                if (!ChangeNth(line, 2, newguid))           // character_*.guid update
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_garrison_followers.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 1, lastfollowerid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_garrison_followers.dbId update line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_ITEM_TABLE:
            {
                if (!ChangeGuid(line, 1, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: item_instance_*.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_ARTI_TABLE:
            {
                // item, owner, data field:item, owner guid
                if (!ChangeGuid(line, 1, items, sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed()))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: item_instance_artifact.itemGuid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                if (!ChangeNth(line, 2, newguid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: item_instance_artifact.char_guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            case DTT_CHAR_TRANSMOG:
            {
                if (!ChangeNth(line, 1, newguid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_transmog_outfits.guid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }

                char newSetGuid[24];
                snprintf(newSetGuid, 24, UI64FMTD, sObjectMgr->GenerateEquipmentSetGuid());
                if (!ChangeNth(line, 2, newSetGuid))
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "LoadPlayerDump: character_transmog_outfits.setguid line: '%s'!", line.c_str());
                    return DUMP_FILE_BROKEN;
                }
                break;
            }
            default:
                TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Unknown dump table type: %u", type);
                break;
        }

        fixNULLfields(line);

        trans->Append(line.c_str());
    }

    CharacterDatabase.CommitTransaction(trans);
    LoginDatabase.CommitTransaction(loginTrans);

    // in case of name conflict player has to rename at login anyway
    sWorld->AddCharacterInfo(guid, name, gender, race, playerClass, level, account);

    sObjectMgr->GetGenerator<HighGuid::Item>()->Set(sObjectMgr->GetGenerator<HighGuid::Item>()->GetNextAfterMaxUsed() + items.size());
    sObjectMgr->_mailId     += mails.size();

    if (incHighest)
        sObjectMgr->GetGenerator<HighGuid::Player>()->Generate();

    return DUMP_SUCCESS;
}
