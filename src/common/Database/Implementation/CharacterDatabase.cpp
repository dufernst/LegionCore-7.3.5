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

#include "CharacterDatabase.h"

void CharacterDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_CHARACTERDATABASE_STATEMENTS);

#define SelectItemInstanceContent "ii.guid, ii.itemEntry, ii.creatorGuid, ii.giftCreatorGuid, ii.count, ii.duration, ii.charges, ii.flags, ii.enchantments, ii.randomPropertyType, ii.randomPropertyId, " \
        "ii.durability, ii.playedTime, ii.text, ii.upgradeId, ii.battlePetSpeciesId, ii.battlePetBreedData, ii.battlePetLevel, ii.battlePetDisplayId, ii.bonusListIDs, " \
        "iit.itemModifiedAppearanceAllSpecs, iit.itemModifiedAppearanceSpec1, iit.itemModifiedAppearanceSpec2, iit.itemModifiedAppearanceSpec3, iit.itemModifiedAppearanceSpec4, " \
        "iit.spellItemEnchantmentAllSpecs, iit.spellItemEnchantmentSpec1, iit.spellItemEnchantmentSpec2, iit.spellItemEnchantmentSpec3, iit.spellItemEnchantmentSpec4, " \
        "ig.gemItemId1, ig.gemBonuses1, ig.gemContext1, ig.gemScalingLevel1, ig.gemItemId2, ig.gemBonuses2, ig.gemContext2, ig.gemScalingLevel2, ig.gemItemId3, ig.gemBonuses3, ig.gemContext3, ig.gemScalingLevel3, " \
        "im.fixedScalingLevel, im.artifactKnowledgeLevel, iia.xp, iia.artifactAppearanceId, iia.tier, ii.dungeonEncounterID, ii.contextID, ii.createdTime"

    PrepareStatement(CHAR_DEL_QUEST_POOL_SAVE, "DELETE FROM pool_quest_save WHERE pool_id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_QUEST_POOL_SAVE, "INSERT INTO pool_quest_save (pool_id, quest_id) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_NONEXISTENT_GUILD_BANK_ITEM, "DELETE FROM guild_bank_item WHERE guildid = ? AND TabId = ? AND SlotId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_EXPIRED_BANS, "UPDATE character_banned SET active = 0 WHERE unbandate <= UNIX_TIMESTAMP() AND unbandate <> bandate", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_GUID_BY_NAME, "SELECT guid FROM characters WHERE BINARY name = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_CHECK_NAME, "SELECT 1 FROM characters WHERE BINARY name  = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_CHECK_GUID, "SELECT 1 FROM characters WHERE guid = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_SUM_CHARS, "SELECT COUNT(guid) FROM characters WHERE account = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_CHAR_CREATE_INFO, "SELECT level, race, class FROM characters WHERE account = ? LIMIT 0, ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_BAN, "INSERT INTO character_banned VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?, 1)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHARACTER_BAN, "UPDATE character_banned SET active = 0 WHERE guid = ? AND active != 0", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_BAN, "DELETE cb FROM character_banned cb INNER JOIN characters c ON c.guid = cb.guid WHERE c.account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_BANINFO, "SELECT FROM_UNIXTIME(bandate), unbandate-bandate, active, unbandate, banreason, bannedby FROM character_banned WHERE guid = ? ORDER BY bandate ASC", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_GUID_BY_NAME_FILTER, "SELECT guid, name FROM characters WHERE name LIKE CONCAT('%%', ?, '%%')", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_BANINFO_LIST, "SELECT bandate, unbandate, bannedby, banreason FROM character_banned WHERE guid = ? ORDER BY unbandate", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_BANNED_NAME, "SELECT characters.name FROM characters, character_banned WHERE character_banned.guid = ? AND character_banned.guid = characters.guid", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAIL_LIST_COUNT, "SELECT COUNT(id) FROM mail WHERE receiver = ? ", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAIL_LIST_INFO, "SELECT id, sender, (SELECT name FROM characters WHERE guid = sender) AS sendername, receiver, (SELECT name FROM characters WHERE guid = receiver) AS receivername,"
                     "SUBJECT, deliver_time, expire_time, money, has_items FROM mail WHERE receiver = ? ", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAIL_LIST_ITEMS, "SELECT itemEntry,count FROM item_instance WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_ENUM, "SELECT c.guid, c.name, c.race, c.class, c.skin, c.face, c.hairStyle, c.hairColor, c.facialStyle, c.blindfold, c.gender, c.tattoo, c.horn, c.level, c.zone, c.map, c.position_x, c.position_y, c.position_z, gm.guildid, c.playerFlags, c.at_login, cp.entry, cp.modelid, cp.level, c.equipmentCache, cb.guid, c.slot, c.logout_time, c.specialization FROM characters AS c LEFT JOIN character_pet AS cp ON c.guid = cp.owner AND cp.id = c.currentpetnumber LEFT JOIN guild_member AS gm ON c.guid = gm.guid LEFT JOIN character_banned AS cb ON c.guid = cb.guid AND cb.active = 1 WHERE c.account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ENUM_DELETED, "SELECT c.guid, c.name, c.race, c.class, c.skin, c.face, c.hairStyle, c.hairColor, c.facialStyle, c.blindfold, c.gender, c.tattoo, c.horn, c.level, c.zone, c.map, c.position_x, c.position_y, c.position_z, gm.guildid, c.playerFlags, c.at_login, cp.entry, cp.modelid, cp.level, c.equipmentCache, cb.guid, c.slot, c.logout_time, c.specialization FROM characters AS c LEFT JOIN character_pet AS cp ON c.guid = cp.owner AND cp.id = c.currentpetnumber LEFT JOIN guild_member AS gm ON c.guid = gm.guid LEFT JOIN character_banned AS cb ON c.guid = cb.guid AND cb.active = 1 WHERE c.deleteInfos_Account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ENUM_DECLINED_NAME, "SELECT c.guid, c.name, c.race, c.class, c.skin, c.face, c.hairStyle, c.hairColor, c.facialStyle, c.blindfold, c.gender, c.tattoo, c.horn, c.level, c.zone, c.map, c.position_x, c.position_y, c.position_z, gm.guildid, c.playerFlags, c.at_login, cp.entry, cp.modelid, cp.level, c.equipmentCache, cb.guid, c.slot, cd.genitive, c.logout_time, c.specialization FROM characters AS c LEFT JOIN character_pet AS cp ON c.guid = cp.owner AND cp.id = c.currentpetnumber LEFT JOIN character_declinedname AS cd ON c.guid = cd.guid LEFT JOIN guild_member AS gm ON c.guid = gm.guid LEFT JOIN character_banned AS cb ON c.guid = cb.guid AND cb.active = 1 WHERE c.account = ?", CONNECTION_ASYNC);

    // Pet
    PrepareStatement(CHAR_INS_PET, "REPLACE INTO character_pet (id, entry, owner, modelid, level, exp, Reactstate, name, renamed, curhealth, curmana, abdata, savetime, CreatedBySpell, PetType, specialization) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PET_DETAIL, "SELECT id, entry, level, name, modelid FROM character_pet WHERE owner = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PET_BY_ID, "SELECT entry, id, PetType FROM character_pet WHERE owner = ? AND id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PET, "SELECT `id`, `entry`, `owner`, `modelid`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `curhealth`, `curmana`, `abdata`, `savetime`, `CreatedBySpell`, `PetType`, `specialization` FROM character_pet WHERE `owner` = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_BY_ID, "DELETE FROM character_pet WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_DECLINED_BY_ID, "DELETE FROM character_pet_declinedname WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_AURA_BY_ID, "DELETE FROM pet_aura WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_SPELL_BY_ID, "DELETE FROM pet_spell WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_SPELL_COOLDOWN_BY_ID, "DELETE FROM pet_spell_cooldown WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_PET_SPELL, "DELETE FROM pet_spell WHERE spell = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_FREE_NAME, "SELECT guid, name FROM characters WHERE guid = ? AND account = ? AND (at_login & ?) = ? AND NOT EXISTS (SELECT NULL FROM characters WHERE name = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_GUID_RACE_ACC_BY_NAME, "SELECT guid, race, account FROM characters WHERE BINARY name = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_CHAR_RACE, "SELECT race FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_LEVEL, "SELECT level FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_ZONE, "SELECT zone FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_NAME_DATA, "SELECT race, class, gender, level FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_POSITION_XYZ, "SELECT map, position_x, position_y, position_z FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_POSITION, "SELECT position_x, position_y, position_z, orientation, map, taxi_path FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_DAILY, "DELETE FROM character_queststatus_daily", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_WEEKLY, "TRUNCATE character_queststatus_weekly", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_SEASONAL, "DELETE FROM character_queststatus_seasonal WHERE event = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_DAILY_CHAR, "DELETE FROM character_queststatus_daily WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_DAILY_ACC, "DELETE FROM character_queststatus_daily WHERE guid = 0 AND account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_WEEKLY_CHAR, "DELETE FROM character_queststatus_weekly WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_WEEKLY_ACC, "DELETE FROM character_queststatus_weekly WHERE guid = 0 AND account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_SEASONAL_CHAR, "DELETE FROM character_queststatus_seasonal WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_SEASONAL_ACC, "DELETE FROM character_queststatus_seasonal WHERE guid = 0 AND account = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_QUEST_STATUS_C, "DELETE FROM character_queststatus_rewarded WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_DAILY_C, "DELETE FROM character_queststatus_daily WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_WEEKLY_C, "DELETE FROM character_queststatus_weekly WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_SEASONAL_C, "DELETE FROM character_queststatus_seasonal WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_BATTLEGROUND_RANDOM, "DELETE FROM character_battleground_random", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_BATTLEGROUND_RANDOM, "REPLACE INTO character_battleground_random (guid, bg, rbg, arena2v2, arena3v3, skirmish, brawl, brawlArena) VALUES (?,?,?,?,?,?,?,?)", CONNECTION_ASYNC);

    // Start LoginQueryHolder content
    PrepareStatement(CHAR_SEL_CHARACTER, "SELECT guid, account, name, race, class, level, xp, money, skin, face, hairStyle, hairColor, facialStyle, blindfold, gender, tattoo, horn, inventorySlots, bankSlots, drunk, playerFlags, "
    "playerFlagsEx, position_x, position_y, position_z, map, orientation, taximask, cinematic, totaltime, leveltime, rest_bonus, logout_time, is_logout_resting, "
    "trans_x, trans_y, trans_z, trans_o, transguid, extra_flags, at_login, zone, online, death_expire_time, taxi_path, dungeonDifficulty, "
    "totalKills, todayKills, yesterdayKills, chosenTitle, watchedFaction, "
    "health, mana, instance_id, activespec, specialization, lootspecialization, exploredZones, equipmentCache, knownTitles, actionBars, "
    "currentpetnumber, petslot, grantableLevels, lfgBonusFaction, raidDifficulty, legacyRaidDifficulty, created_time, killPoints FROM characters WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_GROUP_MEMBER, "SELECT guid FROM group_member WHERE memberGuid = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_CHARACTER_AURAS, "SELECT caster_guid, slot, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges FROM character_aura WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_AURAS_EFFECTS, "SELECT slot, effect, baseamount, amount FROM character_aura_effect WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_SPELL, "SELECT spell, active, disabled FROM character_spell WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_QUESTSTATUS, "SELECT quest, status, timer, guid FROM character_queststatus WHERE account = ? AND status <> 0 AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_QUESTSTATUS_OBJECTIVES, "SELECT quest, objective, data FROM character_queststatus_objectives WHERE account = ? AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_DAILYQUESTSTATUS, "SELECT quest, time, guid FROM character_queststatus_daily WHERE account = ? AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_WEEKLYQUESTSTATUS, "SELECT quest, guid FROM character_queststatus_weekly WHERE account = ? AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_SEASONALQUESTSTATUS, "SELECT quest, event, guid FROM character_queststatus_seasonal WHERE account = ? AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_DAILYQUESTSTATUS, "REPLACE INTO character_queststatus_daily (guid, quest, time, account) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_WEEKLYQUESTSTATUS, "REPLACE INTO character_queststatus_weekly (guid, quest, account) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_SEASONALQUESTSTATUS, "REPLACE INTO character_queststatus_seasonal (guid, quest, event, account) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_REPUTATION, "SELECT faction, standing, flags FROM character_reputation WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_INVENTORY, "SELECT " SelectItemInstanceContent ", bag, slot, ii.isdonateitem FROM character_inventory ci JOIN item_instance ii ON ci.item = ii.guid "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and ci.guid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid WHERE ci.guid = ? ORDER BY bag, slot", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_NOT_INVENTORY, "SELECT " SelectItemInstanceContent ", ii.isdonateitem FROM item_instance ii "
    "LEFT JOIN character_inventory ci ON ci.item = ii.guid "
    "LEFT JOIN mail_items mi ON mi.item_guid = ii.guid "
    "LEFT JOIN auctionhouse ah ON ah.itemguid = ii.guid "
    "LEFT JOIN character_void_storage cvs ON cvs.itemGuid = ii.guid "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and ii.owner_guid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid WHERE ii.owner_guid = ? AND ci.item IS NULL AND mi.item_guid IS NULL AND cvs.itemGuid IS NULL AND ah.itemguid IS NULL", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ITEM_INFO, "SELECT " SelectItemInstanceContent " FROM item_instance ii "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and ii.owner_guid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid WHERE ii.guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_ACTIONS, "SELECT a.button, a.action, a.type FROM character_action as a, characters as c WHERE a.guid = c.guid AND a.spec = c.activespec AND a.guid = ? ORDER BY button", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_MAILCOUNT, "SELECT COUNT(id) FROM mail WHERE receiver = ? AND (checked & 1) = 0 AND deliver_time <= ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_MAILDATE, "SELECT MIN(deliver_time) FROM mail WHERE receiver = ? AND (checked & 1) = 0", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_MAIL_COUNT, "SELECT COUNT(*) FROM mail WHERE receiver = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_SOCIALLIST, "SELECT friend, flags, note FROM character_social JOIN characters ON characters.guid = character_social.friend WHERE character_social.guid = ? AND deleteinfos_name IS NULL LIMIT 255", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_HOMEBIND, "SELECT mapId, zoneId, posX, posY, posZ FROM character_homebind WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_SPELLCOOLDOWNS, "SELECT spell, item, time FROM character_spell_cooldown WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_DECLINEDNAMES, "SELECT genitive, dative, accusative, instrumental, prepositional FROM character_declinedname WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_GUILD_MEMBER, "SELECT guildid, rank FROM guild_member WHERE guid = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_CHARACTER_ACHIEVEMENTS, "SELECT achievement, date FROM character_achievement WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ACCOUNT_ACHIEVEMENTS, "SELECT first_guid, achievement, date FROM account_achievement WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_CRITERIAPROGRESS, "SELECT criteria, counter, date, achievID, completed FROM character_achievement_progress WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ACCOUNT_CRITERIAPROGRESS, "SELECT criteria, counter, date, achievID, completed FROM account_achievement_progress WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_BGDATA, "SELECT instanceId, team, joinX, joinY, joinZ, joinO, joinMapId, taxiStart, taxiEnd, mountSpell, lastActiveSpec FROM character_battleground_data WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_GLYPHS, "SELECT talentGroup, glyphId FROM character_glyphs WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_TALENTS, "SELECT talent, spec FROM character_talent WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_PVP_TALENTS, "SELECT talent, spec FROM character_pvp_talent WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_SKILLS, "SELECT skill, value, max FROM character_skills WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_RANDOMBG, "SELECT bg, rbg, arena2v2, arena3v3, skirmish, brawl, brawlArena FROM character_battleground_random WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_BANNED, "SELECT guid FROM character_banned WHERE guid = ? AND active = 1", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_QUESTSTATUSREW, "SELECT quest, guid FROM character_queststatus_rewarded WHERE account = ? AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ACCOUNT_QUEST, "SELECT quest FROM character_queststatus_rewarded WHERE account = ? GROUP BY quest", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_QUESTSTATUSREW_NON_ACC, "SELECT quest, guid FROM character_queststatus_rewarded WHERE account = ? AND guid = ?", CONNECTION_SYNCH);
    // End LoginQueryHolder content

    PrepareStatement(CHAR_SEL_CHARACTER_ACTIONS_SPEC, "SELECT button, action, type FROM character_action WHERE guid = ? AND spec = ? ORDER BY button", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAILITEMS, "SELECT " SelectItemInstanceContent ", ii.owner_guid FROM mail_items mi JOIN item_instance ii ON mi.item_guid = ii.guid "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and ii.owner_guid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid WHERE mail_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_AUCTION_ITEMS, "SELECT " SelectItemInstanceContent " FROM auctionhouse ah JOIN item_instance ii ON ah.itemguid = ii.guid "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and ii.owner_guid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_AUCTIONS, "SELECT id, auctioneerguid, itemguid, itemEntry, count, itemowner, buyoutprice, time, buyguid, lastbid, startbid, deposit FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INS_AUCTION, "INSERT INTO auctionhouse (id, auctioneerguid, itemguid, itemowner, buyoutprice, time, buyguid, lastbid, startbid, deposit) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_AUCTION, "DELETE FROM auctionhouse WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_AUCTION_BY_TIME, "SELECT id FROM auctionhouse WHERE time <= ? ORDER BY TIME ASC", CONNECTION_SYNCH);
    PrepareStatement(CHAR_UPD_AUCTION_BID, "UPDATE auctionhouse SET buyguid = ?, lastbid = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_MAIL, "INSERT INTO mail(id, messageType, stationery, mailTemplateId, sender, receiver, subject, body, has_items, expire_time, deliver_time, money, cod, checked) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_MAIL_BY_ID, "DELETE FROM mail WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_MAIL_ITEM, "REPLACE INTO mail_items(mail_id, item_guid, receiver) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_MAIL_ITEM, "DELETE FROM mail_items WHERE item_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_MAIL_ITEM, "DELETE FROM mail_items WHERE item_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_EMPTY_EXPIRED_MAIL, "DELETE FROM mail WHERE expire_time < ? AND has_items = 0 AND body = ''", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_EXPIRED_MAIL, "SELECT id, messageType, sender, receiver, has_items, expire_time, cod, checked, mailTemplateId FROM mail WHERE expire_time < ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_EXPIRED_MAIL_ITEMS, "SELECT item_guid, itemEntry, mail_id FROM mail_items mi INNER JOIN item_instance ii ON ii.guid = mi.item_guid LEFT JOIN mail mm ON mi.mail_id = mm.id WHERE mm.id IS NOT NULL AND mm.expire_time < ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_UPD_MAIL_RETURNED, "UPDATE mail SET sender = ?, receiver = ?, expire_time = ?, deliver_time = ?, cod = 0, checked = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_MAIL_ITEM_RECEIVER, "UPDATE mail_items SET receiver = ? WHERE item_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ITEM_OWNER, "UPDATE item_instance SET owner_guid = ? WHERE guid = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_ITEM_REFUNDS, "SELECT player_guid, paidMoney, paidExtendedCost FROM item_refund_instance WHERE item_guid = ? AND player_guid = ? LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_ITEM_BOP_TRADE, "SELECT allowedPlayers FROM item_soulbound_trade_data WHERE itemGuid = ? LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_ITEM_BOP_TRADE, "DELETE FROM item_soulbound_trade_data WHERE itemGuid = ? LIMIT 1", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ITEM_BOP_TRADE, "INSERT INTO item_soulbound_trade_data VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_INVENTORY_ITEM, "REPLACE INTO character_inventory (guid, bag, slot, item) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_ITEM_INSTANCE, "REPLACE INTO item_instance (itemEntry, owner_guid, creatorGuid, giftCreatorGuid, count, duration, charges, flags, enchantments, randomPropertyType, randomPropertyId, durability, playedTime, text, upgradeId, battlePetSpeciesId, battlePetBreedData, battlePetLevel, battlePetDisplayId, bonusListIDs, itemLevel, dungeonEncounterID,  contextID, createdTime, isdonateitem, guid) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ITEM_INSTANCE, "UPDATE item_instance SET itemEntry = ?, owner_guid = ?, creatorGuid = ?, giftCreatorGuid = ?, count = ?, duration = ?, charges = ?, flags = ?, enchantments = ?, randomPropertyType = ?, randomPropertyId = ?, durability = ?, playedTime = ?, text = ?, upgradeId = ?, battlePetSpeciesId = ?, battlePetBreedData = ?, battlePetLevel = ?, battlePetDisplayId = ?, bonusListIDs = ?, itemLevel = ?, dungeonEncounterID = ?, contextID = ?, isdonateitem = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ITEM_INSTANCE_ON_LOAD, "UPDATE item_instance SET duration = ?, flags = ?, durability = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE, "DELETE FROM item_instance WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ITEM_INSTANCE_GEMS, "REPLACE INTO item_instance_gems (itemGuid, gemItemId1, gemBonuses1, gemContext1, gemScalingLevel1, gemItemId2, gemBonuses2, gemContext2, gemScalingLevel2, gemItemId3, gemBonuses3, gemContext3, gemScalingLevel3) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_GEMS, "DELETE FROM item_instance_gems WHERE itemGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_GEMS_BY_OWNER, "DELETE iig FROM item_instance_gems iig LEFT JOIN item_instance ii ON iig.itemGuid = ii.guid WHERE ii.owner_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ITEM_INSTANCE_TRANSMOG, "INSERT INTO item_instance_transmog (itemGuid, itemModifiedAppearanceAllSpecs, itemModifiedAppearanceSpec1, itemModifiedAppearanceSpec2, itemModifiedAppearanceSpec3, itemModifiedAppearanceSpec4, spellItemEnchantmentAllSpecs, spellItemEnchantmentSpec1, spellItemEnchantmentSpec2, spellItemEnchantmentSpec3, spellItemEnchantmentSpec4) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_TRANSMOG, "DELETE FROM item_instance_transmog WHERE itemGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_TRANSMOG_BY_OWNER, "DELETE iit FROM item_instance_transmog iit LEFT JOIN item_instance ii ON iit.itemGuid = ii.guid WHERE ii.owner_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GIFT_OWNER, "UPDATE character_gifts SET guid = ? WHERE item_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GIFT, "DELETE FROM character_gifts WHERE item_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_GIFT_BY_ITEM, "SELECT entry, flags FROM character_gifts WHERE item_guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_ACCOUNT_BY_NAME, "SELECT account FROM characters WHERE BINARY name = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_ACCOUNT_BY_GUID, "SELECT account FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_ACCOUNT_NAME_BY_GUID, "SELECT account, name FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_NAME_CLASS, "SELECT name, class FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_NAME, "SELECT name FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_COUNT, "SELECT account, COUNT(guid) FROM characters WHERE account = ? GROUP BY account", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_NAME, "UPDATE characters set name = ?, at_login = at_login & ~ ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_NAME_LOG, "INSERT INTO log_rename (`guid`, `date`, `oldName`, `newName`) VALUES (?, NOW(), ?, ?)", CONNECTION_ASYNC);

    // Guild handling
    // 0: uint32, 1: string, 2: uint32, 3: string, 4: string, 5: uint64, 6-10: uint32, 11: uint64
    PrepareStatement(CHAR_INS_GUILD, "INSERT INTO guild (guildid, name, leaderguid, flags, info, motd, createdate, EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, BankMoney, level) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD, "DELETE FROM guild WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    // 0: uint32, 1: uint32, 2: uint8, 4: string, 5: string
    PrepareStatement(CHAR_INS_GUILD_MEMBER, "INSERT INTO guild_member (guildid, guid, rank, pnote, offnote) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_MEMBER, "DELETE FROM guild_member WHERE guid = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_DEL_GUILD_MEMBERS, "DELETE FROM guild_member WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBERS_RANK, "UPDATE guild_member SET rank = ? WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_STATS, "UPDATE guild_member SET AchPoint = ?, profId1 = ?, profValue1 = ?, profRank1 = ?, recipesMask1 = ?, profId2 = ?, profValue2 = ?, profRank2 = ?, recipesMask2 = ? WHERE guid = ?", CONNECTION_ASYNC);
    // 0: uint32, 1: uint8, 3: string, 4: uint32
    PrepareStatement(CHAR_INS_GUILD_RANK, "INSERT INTO guild_rank (guildid, rid, rname, rights) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_RANKS, "DELETE FROM guild_rank WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_DEL_GUILD_RANK, "DELETE FROM guild_rank WHERE guildid = ? AND rid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    PrepareStatement(CHAR_UPD_GUILD_RANK_ID, "UPDATE guild_rank SET rid = ? WHERE guildid = ? AND rid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    PrepareStatement(CHAR_INS_GUILD_BANK_TAB, "INSERT INTO guild_bank_tab (guildid, TabId) VALUES (?, ?)", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    PrepareStatement(CHAR_DEL_GUILD_BANK_TAB, "DELETE FROM guild_bank_tab WHERE guildid = ? AND TabId = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    PrepareStatement(CHAR_DEL_GUILD_BANK_TABS, "DELETE FROM guild_bank_tab WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    // 0: uint32, 1: uint8, 2: uint8, 3: uint32, 4: uint32
    PrepareStatement(CHAR_INS_GUILD_BANK_ITEM, "INSERT INTO guild_bank_item (guildid, TabId, SlotId, item_guid) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_BANK_ITEM, "DELETE FROM guild_bank_item WHERE guildid = ? AND TabId = ? AND SlotId = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8, 2: uint8
    PrepareStatement(CHAR_DEL_GUILD_BANK_ITEMS, "DELETE FROM guild_bank_item WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_SEL_GUILD_BANK_ITEMS, "SELECT " SelectItemInstanceContent ", guildid, TabId, SlotId FROM guild_bank_item gbi INNER JOIN item_instance ii ON gbi.item_guid = ii.guid "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and ii.owner_guid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INS_GUILD_BANK_RIGHT_DEFAULT, "INSERT INTO guild_bank_right (guildid, TabId, rid) VALUES (?, ?, ?)", CONNECTION_ASYNC); // 0: uint32, 1: uint8, 2: uint8
    // 0: uint32, 1: uint8, 2: uint8, 3: uint8, 4: uint32
    PrepareStatement(CHAR_INS_GUILD_BANK_RIGHT, "INSERT INTO guild_bank_right (guildid, TabId, rid, gbright, SlotPerDay) VALUES (?, ?, ?, ?, ?) "
        "ON DUPLICATE KEY UPDATE gbright = VALUES(gbright), SlotPerDay = VALUES(SlotPerDay)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_BANK_RIGHT, "DELETE FROM guild_bank_right WHERE guildid = ? AND TabId = ? AND rid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8, 2: uint8
    PrepareStatement(CHAR_DEL_GUILD_BANK_RIGHTS, "DELETE FROM guild_bank_right WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_DEL_GUILD_BANK_RIGHTS_FOR_RANK, "DELETE FROM guild_bank_right WHERE guildid = ? AND rid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    PrepareStatement(CHAR_UPD_GUILD_BANK_RIGHTS_ID, "UPDATE guild_bank_right SET rid = ? WHERE guildid = ? AND rid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    // 0-1: uint32, 2-3: uint8, 4-5: uint32, 6: uint16, 7: uint8, 8: uint64
    PrepareStatement(CHAR_INS_GUILD_BANK_EVENTLOG, "INSERT INTO guild_bank_eventlog (guildid, LogGuid, TabId, EventType, PlayerGuid, ItemOrMoney, ItemStackCount, DestTabId, TimeStamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_BANK_EVENTLOG, "DELETE FROM guild_bank_eventlog WHERE guildid = ? AND LogGuid = ? AND TabId = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint32, 2: uint8
    PrepareStatement(CHAR_DEL_GUILD_BANK_EVENTLOGS, "DELETE FROM guild_bank_eventlog WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    // 0-1: uint32, 2: uint8, 3-4: uint32, 5: uint8, 6: uint64
    PrepareStatement(CHAR_INS_GUILD_EVENTLOG, "INSERT INTO guild_eventlog (guildid, LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_EVENTLOG, "DELETE FROM guild_eventlog WHERE guildid = ? AND LogGuid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint32
    PrepareStatement(CHAR_DEL_GUILD_EVENTLOGS, "DELETE FROM guild_eventlog WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_PNOTE, "UPDATE guild_member SET pnote = ? WHERE guid = ?", CONNECTION_ASYNC); // 0: string, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_OFFNOTE, "UPDATE guild_member SET offnote = ? WHERE guid = ?", CONNECTION_ASYNC); // 0: string, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_RANK, "UPDATE guild_member SET rank = ? WHERE guid = ?", CONNECTION_ASYNC); // 0: uint8, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_NAME, "UPDATE guild SET name = ? WHERE guildid = ?", CONNECTION_ASYNC); // 0: string, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_MOTD, "UPDATE guild SET motd = ? WHERE guildid = ?", CONNECTION_ASYNC); // 0: string, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_INFO, "UPDATE guild SET info = ? WHERE guildid = ?", CONNECTION_ASYNC); // 0: string, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_FLAGS, "UPDATE guild SET flags = ? WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_LEADER, "UPDATE guild SET leaderguid = ? WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint32
    PrepareStatement(CHAR_UPD_GUILD_RANK_NAME, "UPDATE guild_rank SET rname = ? WHERE rid = ? AND guildid = ?", CONNECTION_ASYNC); // 0: string, 1: uint8, 2: uint32
    PrepareStatement(CHAR_UPD_GUILD_RANK_RIGHTS, "UPDATE guild_rank SET rights = ? WHERE rid = ? AND guildid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8, 2: uint32
    // 0-5: uint32
    PrepareStatement(CHAR_UPD_GUILD_EMBLEM_INFO, "UPDATE guild SET EmblemStyle = ?, EmblemColor = ?, BorderStyle = ?, BorderColor = ?, BackgroundColor = ? WHERE guildid = ?", CONNECTION_ASYNC);
    // 0: string, 1: string, 2: uint32, 3: uint8
    PrepareStatement(CHAR_UPD_GUILD_BANK_TAB_INFO, "UPDATE guild_bank_tab SET TabName = ?, TabIcon = ? WHERE guildid = ? AND TabId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_BANK_MONEY, "UPDATE guild SET BankMoney = ? WHERE guildid = ?", CONNECTION_ASYNC); // 0: uint64, 1: uint32
    // 0: uint8, 1: uint32, 2: uint8, 3: uint32
    PrepareStatement(CHAR_UPD_GUILD_BANK_EVENTLOG_TAB, "UPDATE guild_bank_eventlog SET TabId = ? WHERE guildid = ? AND TabId = ? AND LogGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_MONEY, "UPDATE guild_member SET BankRemMoney = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint32, 2: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_MONEY, "UPDATE guild_member SET BankResetTimeMoney = ?, BankRemMoney = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint32, 2: uint32, 3: uint32
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_RESET_TIME, "UPDATE guild_member SET BankResetTimeMoney = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_MONEY, "UPDATE guild_rank SET BankMoneyPerDay = ? WHERE rid = ? AND guildid = ?", CONNECTION_ASYNC); // 0: uint32, 1: uint8, 2: uint32
    PrepareStatement(CHAR_UPD_GUILD_BANK_TAB_TEXT, "UPDATE guild_bank_tab SET TabText = ? WHERE guildid = ? AND TabId = ?", CONNECTION_ASYNC); // 0: string, 1: uint32, 2: uint8
    // 0: uint32, 1: uint32, 2: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS0, "UPDATE guild_member SET BankRemSlotsTab0 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS1, "UPDATE guild_member SET BankRemSlotsTab1 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS2, "UPDATE guild_member SET BankRemSlotsTab2 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS3, "UPDATE guild_member SET BankRemSlotsTab3 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS4, "UPDATE guild_member SET BankRemSlotsTab4 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS5, "UPDATE guild_member SET BankRemSlotsTab5 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS6, "UPDATE guild_member SET BankRemSlotsTab6 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_REM_SLOTS7, "UPDATE guild_member SET BankRemSlotsTab7 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint32
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS0, "UPDATE guild_member SET BankResetTimeTab0 = ?, BankRemSlotsTab0 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS1, "UPDATE guild_member SET BankResetTimeTab1 = ?, BankRemSlotsTab1 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS2, "UPDATE guild_member SET BankResetTimeTab2 = ?, BankRemSlotsTab2 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS3, "UPDATE guild_member SET BankResetTimeTab3 = ?, BankRemSlotsTab3 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS4, "UPDATE guild_member SET BankResetTimeTab4 = ?, BankRemSlotsTab4 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS5, "UPDATE guild_member SET BankResetTimeTab5 = ?, BankRemSlotsTab5 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS6, "UPDATE guild_member SET BankResetTimeTab6 = ?, BankRemSlotsTab6 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_MEMBER_BANK_TIME_REM_SLOTS7, "UPDATE guild_member SET BankResetTimeTab7 = ?, BankRemSlotsTab7 = ? WHERE guildid = ? AND guid = ?", CONNECTION_ASYNC);
    // 0: uint32, 1: uint8
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME0, "UPDATE guild_member SET BankResetTimeTab0 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME1, "UPDATE guild_member SET BankResetTimeTab1 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME2, "UPDATE guild_member SET BankResetTimeTab2 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME3, "UPDATE guild_member SET BankResetTimeTab3 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME4, "UPDATE guild_member SET BankResetTimeTab4 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME5, "UPDATE guild_member SET BankResetTimeTab5 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME6, "UPDATE guild_member SET BankResetTimeTab6 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_RANK_BANK_TIME7, "UPDATE guild_member SET BankResetTimeTab7 = 0 WHERE guildid = ? AND rank = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHAR_DATA_FOR_GUILD, "SELECT name, level, class, zone, account, re.standing, gender FROM characters c LEFT JOIN character_reputation re ON re.guid = c.guid AND re.faction = 1168 WHERE c.guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_REP_GUILD_ACHIEVEMENT, "REPLACE INTO guild_achievement (guildId, achievement, date, guids) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_GUILD_ACHIEVEMENT_CRITERIA, "REPLACE INTO guild_achievement_progress (guildId, criteria, counter, date, completedGuid, achievID, completed) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GUILD_ACHIEVEMENT_CRITERIA, "UPDATE guild_achievement_progress SET counter = ?, `date` = ?, achievID = ?, completed = ? WHERE guildId = ? AND criteria = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_INVALID_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM guild_achievement_progress WHERE criteria = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM guild_achievement_progress WHERE criteria = ? AND guildId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENTS, "DELETE FROM guild_achievement WHERE guildId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ALL_GUILD_ACHIEVEMENT_CRITERIA, "DELETE FROM guild_achievement_progress WHERE guildId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_GUILD_ACHIEVEMENT, "SELECT achievement, date, guids FROM guild_achievement WHERE guildId = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_GUILD_ACHIEVEMENT_CRITERIA, "SELECT criteria, counter, date, completedGuid, achievID, completed FROM guild_achievement_progress WHERE guildId = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INS_GUILD_NEWS, "INSERT INTO guild_newslog (guildid, LogGuid, EventType, PlayerGuid, Flags, Value, Timestamp, Data) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
        " ON DUPLICATE KEY UPDATE LogGuid = VALUES (LogGuid), EventType = VALUES (EventType), PlayerGuid = VALUES (PlayerGuid), Flags = VALUES (Flags), Value = VALUES (Value), Timestamp = VALUES (Timestamp)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_LOAD_GUILD_CHALLENGES, "SELECT GuildId, ChallengeType, ChallengeCount FROM guild_challenges", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INIT_GUILD_CHALLENGES, "INSERT INTO guild_challenges VALUES (?, ?, 0)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_COMPLETE_GUILD_CHALLENGE, "UPDATE guild_challenges SET ChallengeCount = ? WHERE GuildId = ? AND ChallengeType = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REMOVE_GUILD_CHALLENGES, "DELETE FROM guild_challenges WHERE GuildId = ?", CONNECTION_ASYNC);

    // Archaelogy
    PrepareStatement(CHAR_SEL_PLAYER_ARCHAELOGY, "SELECT sites, counts, projects FROM character_archaeology WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PLAYER_ARCHAEOLOGY_FINDS, "SELECT id, count, UNIX_TIMESTAMP(date) FROM character_archaeology_finds WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_ARCHAEOLOGY, "REPLACE INTO character_archaeology (guid, sites, counts, projects) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_ARCHAEOLOGY_FINDS, "REPLACE INTO character_archaeology_finds (guid, id, count, date) VALUES (?, ?, ?, FROM_UNIXTIME(?))", CONNECTION_ASYNC);

    // Chat channel handling
    PrepareStatement(CHAR_SEL_CHANNEL, "SELECT announce, ownership, password, bannedList FROM channels WHERE name = ? AND team = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INS_CHANNEL, "INSERT INTO channels(name, team, lastUsed) VALUES (?, ?, UNIX_TIMESTAMP())", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHANNEL, "UPDATE channels SET announce = ?, ownership = ?, password = ?, bannedList = ?, lastUsed = UNIX_TIMESTAMP() WHERE name = ? AND team = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHANNEL_USAGE, "UPDATE channels SET lastUsed = UNIX_TIMESTAMP() WHERE name = ? AND team = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHANNEL_OWNERSHIP, "UPDATE channels SET ownership = ? WHERE name LIKE ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_OLD_CHANNELS, "DELETE FROM channels WHERE ownership = 1 AND lastUsed + ? < UNIX_TIMESTAMP()", CONNECTION_ASYNC);

    // Equipment sets
    PrepareStatement(CHAR_SEL_CHARACTER_EQUIPMENTSETS, "SELECT setguid, setindex, name, iconname, ignore_mask, AssignedSpecIndex, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14, item15, item16, item17, item18 FROM character_equipmentsets WHERE guid = ? ORDER BY setindex", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_EQUIP_SET, "UPDATE character_equipmentsets SET name=?, iconname=?, ignore_mask=?, AssignedSpecIndex=?, item0=?, item1=?, item2=?, item3=?, item4=?, item5=?, item6=?, item7=?, item8=?, item9=?, item10=?, item11=?, item12=?, item13=?, item14=?, item15=?, item16=?, item17=?, item18=? WHERE guid=? AND setguid=? AND setindex=?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_EQUIP_SET, "INSERT INTO character_equipmentsets (guid, setguid, setindex, name, iconname, ignore_mask, AssignedSpecIndex, item0, item1, item2, item3, item4, item5, item6, item7, item8, item9, item10, item11, item12, item13, item14, item15, item16, item17, item18) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_EQUIP_SET, "DELETE FROM character_equipmentsets WHERE setguid=?", CONNECTION_ASYNC);
   
    PrepareStatement(CHAR_SEL_CHARACTER_TRANSMOG_OUTFITS, "SELECT setguid, setindex, name, iconname, ignore_mask, appearance0, appearance1, appearance2, appearance3, appearance4, appearance5, appearance6, appearance7, appearance8, appearance9, appearance10, appearance11, appearance12, appearance13, appearance14, appearance15, appearance16, appearance17, appearance18, mainHandEnchant, offHandEnchant FROM character_transmog_outfits WHERE guid = ? ORDER BY setindex", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_TRANSMOG_OUTFIT, "UPDATE character_transmog_outfits SET name=?, iconname=?, ignore_mask=?, appearance0=?, appearance1=?, appearance2=?, appearance3=?, appearance4=?, appearance5=?, appearance6=?, appearance7=?, appearance8=?, appearance9=?, appearance10=?, appearance11=?, appearance12=?, appearance13=?, appearance14=?, appearance15=?, appearance16=?, appearance17=?, appearance18=?, mainHandEnchant=?, offHandEnchant=? WHERE guid=? AND setguid=? AND setindex=?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_TRANSMOG_OUTFIT, "INSERT INTO character_transmog_outfits (guid, setguid, setindex, name, iconname, ignore_mask, appearance0, appearance1, appearance2, appearance3, appearance4, appearance5, appearance6, appearance7, appearance8, appearance9, appearance10, appearance11, appearance12, appearance13, appearance14, appearance15, appearance16, appearance17, appearance18, mainHandEnchant, offHandEnchant) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_TRANSMOG_OUTFIT, "DELETE FROM character_transmog_outfits WHERE setguid=?", CONNECTION_ASYNC);

    // Auras
    PrepareStatement(CHAR_INS_AURA, "INSERT INTO character_aura (guid, slot, caster_guid, item_guid, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_INS_AURA_EFFECT, "INSERT INTO character_aura_effect (guid, slot, effect, baseamount, amount) "
    "VALUES (?, ?, ?, ?, ?)",  CONNECTION_ASYNC);

    // Currency
    PrepareStatement(CHAR_SEL_PLAYER_CURRENCY, "SELECT currency, week_count, total_count, season_total, flags, curentcap FROM character_currency WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_PLAYER_CURRENCY, "UPDATE character_currency SET week_count = ?, total_count = ?, season_total = ?, flags = ?, curentcap = ? WHERE guid = ? AND currency = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_CURRENCY, "REPLACE INTO character_currency (guid, currency, week_count, total_count, season_total, flags, curentcap) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);


    // Account data
    PrepareStatement(CHAR_SEL_ACCOUNT_DATA, "SELECT type, time, data FROM account_data WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_ACCOUNT_DATA, "REPLACE INTO account_data (accountId, type, time, data) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ACCOUNT_DATA, "DELETE FROM account_data WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PLAYER_ACCOUNT_DATA, "SELECT type, time, data FROM character_account_data WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_ACCOUNT_DATA, "REPLACE INTO character_account_data(guid, type, time, data) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_ACCOUNT_DATA, "DELETE FROM character_account_data WHERE guid = ?", CONNECTION_ASYNC);

    // Tutorials
    PrepareStatement(CHAR_SEL_TUTORIALS, "SELECT tut0, tut1, tut2, tut3, tut4, tut5, tut6, tut7 FROM account_tutorial WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_HAS_TUTORIALS, "SELECT 1 FROM account_tutorial WHERE accountId = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INS_TUTORIALS, "INSERT INTO account_tutorial(tut0, tut1, tut2, tut3, tut4, tut5, tut6, tut7, accountId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_TUTORIALS, "UPDATE account_tutorial SET tut0 = ?, tut1 = ?, tut2 = ?, tut3 = ?, tut4 = ?, tut5 = ?, tut6 = ?, tut7 = ? WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_TUTORIALS, "DELETE FROM account_tutorial WHERE accountId = ?", CONNECTION_ASYNC);

    // Instance saves char
    PrepareStatement(CHAR_SEL_CHARACTER_INSTANCE, "SELECT instance, map, difficulty, permanent, completedEncounters, data, resetTime, Extended FROM character_instance WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_INSTANCE_BY_INSTANCE_GUID, "DELETE FROM character_instance WHERE guid = ? AND instance = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_INSTANCE, "UPDATE character_instance SET instance = ?, permanent = ? WHERE guid = ? AND instance = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHAR_INSTANCE, "REPLACE INTO character_instance (guid, instance, map, difficulty, permanent, completedEncounters, data, resetTime, Extended) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_INSTANCE_BY_INSTANCE, "DELETE FROM character_instance WHERE instance = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_CHAR_INSTANCE_BY_MAP_DIFF, "DELETE FROM character_instance WHERE map = ? AND difficulty = ? AND Extended = 0 AND (resetTime + 2592000) <= ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_INSTANCE, "DELETE FROM character_instance WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_INSTANCE_EXTENDED, "UPDATE character_instance SET Extended = 0, resetTime = ? WHERE map = ? AND difficulty = ? AND Extended = 1", CONNECTION_ASYNC);

    // Instance saves group
    PrepareStatement(CHAR_DEL_GROUP_INSTANCE_BY_INSTANCE, "DELETE FROM group_instance WHERE instance = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_DEL_GROUP_INSTANCE_BY_GUID, "DELETE FROM group_instance WHERE guid = ? AND instance = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_GROUP_INSTANCE, "REPLACE INTO group_instance (guid, instance, map, difficulty, permanent, completedEncounters, data, resetTime) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GROUP_INSTANCE_BY_MAP_DIFF, "DELETE FROM group_instance WHERE map = ? AND difficulty = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GROUP_INSTANCE_PERM_BINDING, "DELETE FROM group_instance WHERE guid = ? AND (permanent = 1 OR instance IN (SELECT instance FROM character_instance WHERE guid = ?))", CONNECTION_ASYNC);

    // Game event saves
    PrepareStatement(CHAR_DEL_GAME_EVENT_SAVE, "DELETE FROM game_event_save WHERE eventEntry = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_GAME_EVENT_SAVE, "INSERT INTO game_event_save (eventEntry, state, next_start) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    // Game event condition saves
    PrepareStatement(CHAR_DEL_ALL_GAME_EVENT_CONDITION_SAVE, "DELETE FROM game_event_condition_save WHERE eventEntry = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GAME_EVENT_CONDITION_SAVE, "DELETE FROM game_event_condition_save WHERE eventEntry = ? AND condition_id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_GAME_EVENT_CONDITION_SAVE, "INSERT INTO game_event_condition_save (eventEntry, condition_id, done) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    // Petitions
    PrepareStatement(CHAR_SEL_PETITION, "SELECT ownerguid, name FROM petition WHERE petitionguid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PETITION_SIGNATURE, "SELECT playerguid FROM petition_sign WHERE petitionguid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_ALL_PETITION_SIGNATURES, "DELETE FROM petition_sign WHERE playerguid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PETITION_SIGNATURE, "DELETE FROM petition_sign WHERE playerguid = ? AND type = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PETITION_BY_OWNER, "SELECT petitionguid FROM petition WHERE ownerguid = ? AND type = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PETITION_TYPE, "SELECT type FROM petition WHERE petitionguid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PETITION_SIGNATURES, "SELECT ownerguid, (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = ?) AS signs, type FROM petition WHERE petitionguid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PETITION_SIG_BY_ACCOUNT, "SELECT playerguid FROM petition_sign WHERE player_account = ? AND petitionguid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PETITION_OWNER_BY_GUID, "SELECT ownerguid FROM petition WHERE petitionguid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PETITION_SIG_BY_GUID, "SELECT ownerguid, petitionguid FROM petition_sign WHERE playerguid = ?", CONNECTION_SYNCH);

    // Brackets
    PrepareStatement(CHAR_SEL_CHARACTER_BRACKETS, "SELECT `bracket`, `rating`, `best`, `bestWeek`, `mmr`, `games`, `wins`, `weekGames`, `weekWins` FROM `character_brackets_info` WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHARACTER_BRACKETS_STATS, "REPLACE INTO `character_brackets_info` (`guid`, `bracket`, `rating`, `best`, `bestWeek`, `mmr`, `games`, `wins`, `weekGames`, `weekWins`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHARACTER_BRACKETS_STATS, "UPDATE `character_brackets_info` SET `rating` = ?, `best` = ?, `bestWeek` = ?, `mmr` = ?, `games` = ?, `wins` = ?, `weekGames` = ?, `weekWins` = ? WHERE `guid` = ? AND `bracket` = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_BRACKETS_INFO, "DELETE FROM character_brackets_info WHERE guid = ?", CONNECTION_ASYNC);

    // Character battleground data
    PrepareStatement(CHAR_INS_PLAYER_BGDATA, "INSERT INTO character_battleground_data (guid, instanceId, team, joinX, joinY, joinZ, joinO, joinMapId, taxiStart, taxiEnd, mountSpell, lastActiveSpec) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_BGDATA, "DELETE FROM character_battleground_data WHERE guid = ?", CONNECTION_ASYNC);

    // Character homebind
    PrepareStatement(CHAR_INS_PLAYER_HOMEBIND, "INSERT INTO character_homebind (guid, mapId, zoneId, posX, posY, posZ) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_PLAYER_HOMEBIND, "UPDATE character_homebind SET mapId = ?, zoneId = ?, posX = ?, posY = ?, posZ = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_HOMEBIND, "DELETE FROM character_homebind WHERE guid = ?", CONNECTION_ASYNC);

    // Corpse
    PrepareStatement(CHAR_SEL_CORPSES, "SELECT posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, flags, dynFlags, time, corpseType, instanceId, phaseMask, corpseGuid, guid FROM corpse WHERE corpseType <> 0", CONNECTION_SYNCH);
    PrepareStatement(CHAR_INS_CORPSE, "INSERT INTO corpse (corpseGuid, guid, posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, flags, dynFlags, time, corpseType, instanceId, phaseMask) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CORPSE, "DELETE FROM corpse WHERE corpseGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_CORPSES, "DELETE FROM corpse WHERE guid = ? AND corpseType <> 0", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_OLD_CORPSES, "DELETE FROM corpse WHERE corpseType = 0 OR time < (UNIX_TIMESTAMP(NOW()) - ?)", CONNECTION_ASYNC);

    // Creature respawn
    PrepareStatement(CHAR_SEL_CREATURE_RESPAWNS, "SELECT guid, respawnTime FROM creature_respawn WHERE mapId = ? AND instanceId = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_REP_CREATURE_RESPAWN, "REPLACE INTO creature_respawn (guid, respawnTime, mapId, instanceId) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CREATURE_RESPAWN, "DELETE FROM creature_respawn WHERE guid = ? AND mapId = ? AND instanceId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CREATURE_RESPAWN_BY_INSTANCE, "DELETE FROM creature_respawn WHERE mapId = ? AND instanceId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_MAX_CREATURE_RESPAWNS, "SELECT MAX(respawnTime), instanceId FROM creature_respawn WHERE instanceId > 0 GROUP BY instanceId", CONNECTION_SYNCH);

    // Gameobject respawn
    PrepareStatement(CHAR_SEL_GO_RESPAWNS, "SELECT guid, respawnTime FROM gameobject_respawn WHERE mapId = ? AND instanceId = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_REP_GO_RESPAWN, "REPLACE INTO gameobject_respawn (guid, respawnTime, mapId, instanceId) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GO_RESPAWN, "DELETE FROM gameobject_respawn WHERE guid = ? AND mapId = ? AND instanceId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GO_RESPAWN_BY_INSTANCE, "DELETE FROM gameobject_respawn WHERE mapId = ? AND instanceId = ?", CONNECTION_ASYNC);

    // GM Tickets
    PrepareStatement(CHAR_SEL_GM_TICKETS, "SELECT ticketId, guid, name, message, createTime, mapId, posX, posY, posZ, lastModifiedTime, closedBy, assignedTo, comment, completed, escalated, viewed FROM gm_tickets", CONNECTION_SYNCH);
    PrepareStatement(CHAR_REP_GM_TICKET, "REPLACE INTO gm_tickets (ticketId, guid, name, message, createTime, mapId, posX, posY, posZ, lastModifiedTime, closedBy, assignedTo, comment, completed, escalated, viewed) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GM_TICKET, "DELETE FROM gm_tickets WHERE ticketId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_GM_TICKETS, "DELETE FROM gm_tickets WHERE guid = ?", CONNECTION_ASYNC);

    // GM Survey/subsurvey/lag report
    PrepareStatement(CHAR_INS_GM_SURVEY, "INSERT INTO gm_surveys (guid, surveyId, mainSurvey, overallComment, createTime) VALUES (?, ?, ?, ?, UNIX_TIMESTAMP(NOW()))", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_GM_SUBSURVEY, "INSERT INTO gm_subsurveys (surveyId, subsurveyId, rank, comment) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);

    //  For loading and deleting expired auctions at startup
    PrepareStatement(CHAR_SEL_EXPIRED_AUCTIONS, "SELECT id, auctioneerguid, itemguid, itemEntry, count, itemowner, buyoutprice, time, buyguid, lastbid, startbid, deposit FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid WHERE ah.time <= ?", CONNECTION_SYNCH);

    // LFG Data
    PrepareStatement(CHAR_INS_LFG_DATA, "INSERT INTO lfg_data (guid, dungeon, state) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_LFG_DATA, "DELETE FROM lfg_data WHERE guid = ?", CONNECTION_ASYNC);

    // Player saving
    PrepareStatement(CHAR_INS_CHARACTER, "INSERT INTO characters (guid, account, name, race, class, level, xp, money, skin, face, hairStyle, hairColor, facialStyle, blindfold, gender, tattoo, horn, inventorySlots, bankSlots, drunk, playerFlags, "
    "playerFlagsEx, map, instance_id, dungeonDifficulty, raidDifficulty, legacyRaidDifficulty, position_x, position_y, position_z, orientation, trans_x, trans_y, trans_z, trans_o, transguid, "
    "taximask, cinematic, totaltime, leveltime, rest_bonus, logout_time, is_logout_resting, extra_flags, at_login, zone, "
    "death_expire_time, taxi_path, totalKills, todayKills, yesterdayKills, chosenTitle, watchedFaction, health, mana, "
    "latency, activespec, specialization, lootspecialization, exploredZones, equipmentCache, knownTitles, actionBars, currentpetnumber, petslot, grantableLevels, created_time, killPoints) VALUES "
    "(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_CHARACTER, "UPDATE characters SET name=?,race=?,class=?,level=?,xp=?,money=?,skin=?,face=?,hairStyle=?,hairColor=?,facialStyle=?,blindfold=?,gender=?,tattoo=?,horn=?,inventorySlots=?,bankSlots=?,drunk=?,playerFlags=?,"
    "playerFlagsEx=?,map=?,instance_id=?,dungeonDifficulty=?,raidDifficulty=?,legacyRaidDifficulty=?,position_x=?,position_y=?,position_z=?,orientation=?,trans_x=?,trans_y=?,trans_z=?,trans_o=?,transguid=?,taximask=?,cinematic=?,totaltime=?,leveltime=?,rest_bonus=?,"
    "logout_time=?,is_logout_resting=?,extra_flags=?,at_login=?,zone=?,death_expire_time=?,taxi_path=?,"
    "totalKills=?,todayKills=?,yesterdayKills=?,chosenTitle=?,"
    "watchedFaction=?,health=?,mana=?,latency=?,activespec=?,specialization=?,lootspecialization=?,exploredZones=?,"
    "equipmentCache=?,knownTitles=?,actionBars=?,currentpetnumber=?,petslot=?,grantableLevels=?,online=?,lfgBonusFaction=?,killPoints=? WHERE guid=?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG, "UPDATE characters SET at_login = at_login | ? WHERE guid = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_UPD_REM_AT_LOGIN_FLAG, "UPDATE characters set at_login = at_login & ~ ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ALL_AT_LOGIN_FLAGS, "UPDATE characters SET at_login = at_login | ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_PETITION_NAME, "UPDATE petition SET name = ? WHERE petitionguid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PETITION_SIGNATURE, "INSERT INTO petition_sign (ownerguid, petitionguid, playerguid, player_account) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ACCOUNT_ONLINE, "UPDATE characters SET online = 0 WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_GROUP, "INSERT INTO groups (guid, leaderGuid, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, groupType, difficulty, legacyRaidDifficulty, raiddifficulty) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_GROUP_MEMBER, "INSERT INTO group_member (guid, memberGuid, memberFlags, subgroup, roles) VALUES(?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GROUP_MEMBER, "DELETE FROM group_member WHERE memberGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_LEADER, "UPDATE groups SET leaderGuid = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_TYPE, "UPDATE groups SET groupType = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_MEMBER_SUBGROUP, "UPDATE group_member SET subgroup = ? WHERE memberGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_MEMBER_FLAG, "UPDATE group_member SET memberFlags = ? WHERE memberGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_MEMBER_ROLE, "UPDATE group_member SET roles = ? WHERE memberGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_DIFFICULTY, "UPDATE groups SET difficulty = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_RAID_DIFFICULTY, "UPDATE groups SET raiddifficulty = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GROUP_LEGACY_RAID_DIFFICULTY, "UPDATE groups SET legacyRaidDifficulty = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ALL_GM_TICKETS, "TRUNCATE TABLE gm_tickets", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_SPELL, "DELETE FROM character_spell WHERE spell = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_TALENT, "DELETE FROM character_talent WHERE talent = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_TALENT_PVP, "DELETE FROM character_pvp_talent WHERE talent = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_DELETE_INFO, "UPDATE characters SET deleteInfos_Name = name, deleteInfos_Account = account, deleteDate = UNIX_TIMESTAMP(), name = '', account = 0 WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UDP_RESTORE_DELETE_INFO, "UPDATE characters SET name = ?, account = ?, deleteDate = NULL, deleteInfos_Name = NULL, deleteInfos_Account = NULL WHERE deleteDate IS NOT NULL AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ZONE, "UPDATE characters SET zone = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_LEVEL, "UPDATE characters SET level = ?, xp = 0 WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM character_achievement_progress WHERE criteria = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM character_achievement_progress WHERE criteria = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_ACC_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM account_achievement_progress WHERE criteria = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ACC_ACHIEV_PROGRESS_CRITERIA, "DELETE FROM account_achievement_progress WHERE criteria = ? AND account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_INVALID_ACHIEVMENT, "DELETE FROM character_achievement WHERE achievement = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ADDON, "INSERT INTO addons (name, crc) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_ONLINE, "UPDATE characters SET online = 1 WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_NAME_AT_LOGIN, "UPDATE characters set name = ?, at_login = at_login & ~ ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_WORLDSTATE, "UPDATE worldstates SET value = ? WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_WORLDSTATE, "REPLACE INTO worldstates (entry, value) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_GENDER_PLAYERBYTES, "UPDATE characters SET skin = ?, gender = ?, face = ?, hairStyle = ?, hairColor = ?, facialStyle = ?, tattoo = ?, horn = ?, blindfold = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_SKILL, "DELETE FROM character_skills WHERE guid = ? AND skill = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_ADD_CHARACTER_SOCIAL_FLAGS, "UPDATE character_social SET flags = flags | ? WHERE guid = ? AND friend = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_REM_CHARACTER_SOCIAL_FLAGS, "UPDATE character_social SET flags = flags & ~ ? WHERE guid = ? AND friend = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_SOCIAL, "INSERT INTO character_social (guid, friend, flags) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_SOCIAL, "DELETE FROM character_social WHERE guid = ? AND friend = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHARACTER_SOCIAL_NOTE, "UPDATE character_social SET note = ? WHERE guid = ? AND friend = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHARACTER_POSITION, "UPDATE characters SET position_x = ?, position_y = ?, position_z = ?, orientation = ?, map = ?, zone = ?, trans_x = 0, trans_y = 0, trans_z = 0, transguid = 0, taxi_path = '' WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_AURA_FROZEN, "SELECT characters.name FROM characters LEFT JOIN character_aura ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARACTER_ONLINE, "SELECT name, account, map, zone FROM characters WHERE online > 0", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_DEL_INFO_BY_GUID, "SELECT guid, deleteInfos_Name, deleteInfos_Account, deleteDate FROM characters WHERE deleteDate IS NOT NULL AND guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_DEL_INFO_BY_NAME, "SELECT guid, deleteInfos_Name, deleteInfos_Account, deleteDate FROM characters WHERE deleteDate IS NOT NULL AND deleteInfos_Name LIKE CONCAT('%%', ?, '%%')", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_DEL_INFO, "SELECT guid, deleteInfos_Name, deleteInfos_Account, deleteDate FROM characters WHERE deleteDate IS NOT NULL", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHARS_BY_ACCOUNT_ID, "SELECT guid FROM characters WHERE account = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_PINFO, "SELECT totaltime, level, money, account, race, class, map, zone, logout_time FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PINFO_BANS, "SELECT unbandate, bandate = unbandate, bannedby, banreason FROM character_banned WHERE guid = ? AND active ORDER BY bandate ASC LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_HOMEBIND, "SELECT mapId, zoneId, posX, posY, posZ FROM character_homebind WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_GUID_NAME_BY_ACC, "SELECT guid, name FROM characters WHERE account = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_POOL_QUEST_SAVE, "SELECT quest_id FROM pool_quest_save WHERE pool_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_CUSTOMIZE_INFO, "SELECT name, race, class, gender, at_login FROM characters WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHAR_CLASS_LVL_AT_LOGIN, "SELECT race, class, level, at_login, knownTitles, name  FROM characters WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PET_SPELL_LIST, "SELECT DISTINCT pet_spell.spell FROM pet_spell, character_pet WHERE character_pet.owner = ? AND character_pet.id = pet_spell.guid AND character_pet.id <> ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_PET, "SELECT id FROM character_pet WHERE owner = ? AND id <> ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_PETS, "SELECT id FROM character_pet WHERE owner = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_COD_ITEM_MAIL, "SELECT id, messageType, mailTemplateId, sender, subject, body, money, has_items FROM mail WHERE receiver = ? AND has_items <> 0 AND cod <> 0", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_SOCIAL, "SELECT DISTINCT guid FROM character_social WHERE friend = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PET_AURA, "SELECT slot, caster_guid, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges FROM pet_aura WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PET_AURA_EFFECT, "SELECT slot, effect, amount, baseamount FROM pet_aura_effect WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_OLD_CHARS, "SELECT guid, deleteInfos_Account FROM characters WHERE deleteDate IS NOT NULL AND deleteDate < ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAIL, "SELECT id, messageType, sender, receiver, subject, body, has_items, expire_time, deliver_time, money, cod, checked, stationery, mailTemplateId FROM mail WHERE receiver = ? ORDER BY id DESC", CONNECTION_BOTH);
    PrepareStatement(CHAR_SEL_PET_SPELL, "SELECT spell, active FROM pet_spell WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PET_SPELL_COOLDOWN, "SELECT spell, time FROM pet_spell_cooldown WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_PET_DECLINED_NAME, "SELECT genitive, dative, accusative, instrumental, prepositional FROM character_pet_declinedname WHERE owner = ? AND id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_GUID_BY_NAME, "SELECT guid FROM characters WHERE BINARY name = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_CHAR_AURA_FROZEN, "DELETE FROM character_aura WHERE spell = 9454 AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHAR_INVENTORY_COUNT_ITEM, "SELECT COUNT(itemEntry) FROM character_inventory ci INNER JOIN item_instance ii ON ii.guid = ci.item WHERE itemEntry = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAIL_COUNT_ITEM, "SELECT COUNT(itemEntry) FROM mail_items mi INNER JOIN item_instance ii ON ii.guid = mi.item_guid WHERE itemEntry = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_AUCTIONHOUSE_COUNT_ITEM,"SELECT COUNT(itemEntry) FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid WHERE itemEntry = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_GUILD_BANK_COUNT_ITEM, "SELECT COUNT(itemEntry) FROM guild_bank_item gbi INNER JOIN item_instance ii ON ii.guid = gbi.item_guid WHERE itemEntry = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_CHAR_INVENTORY_ITEM_BY_ENTRY, "SELECT ci.item, cb.slot AS bag, ci.slot, ci.guid, c.account, c.name FROM characters c INNER JOIN character_inventory ci ON ci.guid = c.guid INNER JOIN item_instance ii ON ii.guid = ci.item LEFT JOIN character_inventory cb ON cb.item = ci.bag WHERE ii.itemEntry = ? LIMIT ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_MAIL_ITEMS_BY_ENTRY, "SELECT mi.item_guid, m.sender, m.receiver, cs.account, cs.name, cr.account, cr.name FROM mail m INNER JOIN mail_items mi ON mi.mail_id = m.id INNER JOIN item_instance ii ON ii.guid = mi.item_guid INNER JOIN characters cs ON cs.guid = m.sender INNER JOIN characters cr ON cr.guid = m.receiver WHERE ii.itemEntry = ? LIMIT ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_AUCTIONHOUSE_ITEM_BY_ENTRY, "SELECT  ah.itemguid, ah.itemowner, c.account, c.name FROM auctionhouse ah INNER JOIN characters c ON c.guid = ah.itemowner INNER JOIN item_instance ii ON ii.guid = ah.itemguid WHERE ii.itemEntry = ? LIMIT ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_GUILD_BANK_ITEM_BY_ENTRY, "SELECT gi.item_guid, gi.guildid, g.name FROM guild_bank_item gi INNER JOIN guild g ON g.guildid = gi.guildid INNER JOIN item_instance ii ON ii.guid = gi.item_guid WHERE ii.itemEntry = ? LIMIT ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_ACCOUNT_ACHIEVEMENT, "DELETE FROM account_achievement WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACHIEVEMENT, "DELETE FROM character_achievement WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACHIEVEMENT_PROGRESS, "DELETE FROM character_achievement_progress WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHAR_REPUTATION_BY_FACTION, "REPLACE INTO character_reputation (guid, faction, standing, flags) VALUES (?, ?, ? , ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_REFUND_INSTANCE, "DELETE FROM item_refund_instance WHERE item_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ITEM_REFUND_INSTANCE, "INSERT INTO item_refund_instance (item_guid, player_guid, paidMoney, paidExtendedCost) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GROUP, "DELETE FROM groups WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GROUP_MEMBER_ALL, "DELETE FROM group_member WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_GIFT, "INSERT INTO character_gifts (guid, item_guid, entry, flags) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_MAIL_ITEM_BY_ID, "DELETE FROM mail_items WHERE mail_id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_DECLINEDNAME, "DELETE FROM character_pet_declinedname WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_ADD_CHAR_PET_DECLINEDNAME, "INSERT INTO character_pet_declinedname (id, owner, genitive, dative, accusative, instrumental, prepositional) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_PET_NAME, "UPDATE character_pet SET name = ?, renamed = 1 WHERE owner = ? AND id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PETITION, "INSERT INTO petition (ownerguid, petitionguid, name) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PETITION_BY_GUID, "DELETE FROM petition WHERE petitionguid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PETITION_SIGNATURE_BY_GUID, "DELETE FROM petition_sign WHERE petitionguid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PET_AURAS, "DELETE FROM pet_aura WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PET_AURAS_EFFECTS, "DELETE FROM pet_aura_effect WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PET_SPELLS, "DELETE FROM pet_spell WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PET_SPELL_COOLDOWNS, "DELETE FROM pet_spell_cooldown WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PET_SPELL_COOLDOWN, "INSERT INTO pet_spell_cooldown (guid, spell, time) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PET_SPELL_BY_SPELL, "DELETE FROM pet_spell WHERE guid = ? and spell = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PET_SPELL, "INSERT IGNORE INTO pet_spell (guid, spell, active) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PET_AURA, "INSERT IGNORE INTO pet_aura (guid, slot, caster_guid, spell, effect_mask, recalculate_mask, stackcount, maxduration, remaintime, remaincharges) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PET_AURA_EFFECT, "INSERT IGNORE INTO pet_aura_effect (guid, slot, effect, baseamount, amount) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_DECLINED_NAME, "DELETE FROM character_declinedname WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_DECLINED_NAME, "INSERT INTO character_declinedname (guid, genitive, dative, accusative, instrumental, prepositional) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_FACTION_OR_RACE, "UPDATE characters SET name = ?, race = ?, at_login = at_login & ~ ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_FACTION_OR_RACE_LOG, "INSERT INTO `log_faction_change` (`id`, `guid`, `account`, `OldRace`, `NewRace`, `date`) VALUES (0, ?, ?, ?, ?, NOW())", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SKILL_LANGUAGES, "DELETE FROM character_skills WHERE skill IN (98, 113, 759, 111, 313, 109, 115, 315, 673, 137) AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_SKILL_LANGUAGE, "REPLACE INTO `character_skills` (guid, skill, value, max) VALUES (?, ?, 300, 300)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_TAXI_PATH, "UPDATE characters SET taxi_path = '' WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_TAXIMASK, "UPDATE characters SET taximask = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS, "DELETE FROM character_queststatus WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_OBJECTIVES, "DELETE FROM character_queststatus_objectives WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SOCIAL_BY_GUID, "DELETE FROM character_social WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SOCIAL_BY_FRIEND, "DELETE FROM character_social WHERE friend = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACHIEVEMENT_BY_ACHIEVEMENT, "DELETE FROM character_achievement WHERE achievement = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_ACHIEVEMENT, "UPDATE character_achievement SET achievement = ? where achievement = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_INVENTORY_FACTION_CHANGE, "UPDATE item_instance ii, character_inventory ci SET ii.itemEntry = ? WHERE ii.itemEntry = ? AND ci.guid = ? AND ci.item = ii.guid", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL, "DELETE FROM character_spell WHERE spell = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_SPELL_FACTION_CHANGE, "UPDATE character_spell SET spell = ? where spell = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_REP_BY_FACTION, "DELETE FROM character_reputation WHERE faction = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_REP_FACTION_CHANGE, "UPDATE character_reputation SET faction = ? where faction = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_REP_STANDING_CHANGE, "UPDATE character_reputation SET standing = ? where faction = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_REP_FLAGS_CHANGE, "UPDATE character_reputation SET flags = ? where faction = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_TITLES_FACTION_CHANGE, "UPDATE characters SET knownTitles = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_RES_CHAR_TITLES_FACTION_CHANGE, "UPDATE characters SET chosenTitle = 0 WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SPELL_COOLDOWN, "DELETE FROM character_spell_cooldown WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER, "DELETE FROM characters WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACTION, "DELETE FROM character_action WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_AURA, "DELETE FROM character_aura WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_AURA_EFFECT, "DELETE FROM character_aura_effect WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_GIFT, "DELETE FROM character_gifts WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_INVENTORY, "DELETE FROM character_inventory WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_REWARDED, "DELETE FROM character_queststatus_rewarded WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_REPUTATION, "DELETE FROM character_reputation WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SPELL, "DELETE FROM character_spell WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_MAIL, "DELETE FROM mail WHERE receiver = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_MAIL_ITEMS, "DELETE FROM mail_items WHERE receiver = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_BY_OWNER, "DELETE FROM character_pet WHERE owner = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_PET_DECLINEDNAME_BY_OWNER, "DELETE FROM character_pet_declinedname WHERE owner = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACHIEVEMENTS, "DELETE FROM character_achievement WHERE guid = ? AND achievement NOT BETWEEN '456' AND '467' AND achievement NOT BETWEEN '1400' AND '1427' AND achievement NOT IN(1463, 3117, 3259)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_EQUIPMENTSETS, "DELETE FROM character_equipmentsets WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_EVENTLOG_BY_PLAYER, "DELETE FROM guild_eventlog WHERE PlayerGuid1 = ? OR PlayerGuid2 = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_BANK_EVENTLOG_BY_PLAYER, "DELETE FROM guild_bank_eventlog WHERE PlayerGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_GLYPHS, "DELETE FROM character_glyphs WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_DAILY, "DELETE FROM character_queststatus_daily WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_TALENT, "DELETE FROM character_talent WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_TALENT_PVP, "DELETE FROM character_pvp_talent WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SKILLS, "DELETE FROM character_skills WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UDP_CHAR_MONEY, "UPDATE characters SET money = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_ACTION, "REPLACE INTO character_action (guid, spec, button, action, type) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_ACTION, "UPDATE character_action SET action = ?, type = ? WHERE guid = ? AND button = ? AND spec = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACTION_BY_BUTTON_SPEC, "DELETE FROM character_action WHERE guid = ? and button = ? and spec = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_INVENTORY_BY_ITEM, "DELETE FROM character_inventory WHERE item = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_INVENTORY_BY_BAG_SLOT, "DELETE FROM character_inventory WHERE bag = ? AND slot = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_MAIL, "UPDATE mail SET has_items = ?, expire_time = ?, deliver_time = ?, money = ?, cod = ?, checked = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHAR_QUESTSTATUS, "REPLACE INTO character_queststatus (guid, quest, status, timer, account) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_BY_QUEST, "DELETE FROM character_queststatus WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHAR_QUESTSTATUS_OBJECTIVES, "REPLACE INTO character_queststatus_objectives (guid, quest, objective, data, account) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_OBJECTIVES_BY_QUEST, "DELETE FROM character_queststatus_objectives WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ACC_QUESTSTATUS_BY_QUEST, "DELETE FROM character_queststatus WHERE account = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ACC_QUESTSTATUS_OBJECTIVES_BY_QUEST, "DELETE FROM character_queststatus_objectives WHERE account = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_QUESTSTATUS, "REPLACE INTO character_queststatus_rewarded (guid, quest, account) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_QUESTSTATUS_REWARDED_BY_QUEST, "DELETE FROM character_queststatus_rewarded WHERE guid = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ACC_QUESTSTATUS_REWARDED_BY_QUEST, "DELETE FROM character_queststatus_rewarded WHERE account = ? AND quest = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_SKILL_BY_SKILL, "DELETE FROM character_skills WHERE guid = ? AND skill = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_SKILLS, "REPLACE INTO character_skills (guid, skill, value, max) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UDP_CHAR_SKILLS, "UPDATE character_skills SET value = ?, max = ? WHERE guid = ? AND skill = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_SPELL, "REPLACE INTO character_spell (guid, spell, active, disabled) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PETITION_BY_OWNER, "DELETE FROM petition WHERE ownerguid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PETITION_SIGNATURE_BY_OWNER, "DELETE FROM petition_sign WHERE ownerguid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_GLYPHS, "INSERT INTO character_glyphs VALUES(?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_TALENT_BY_SPELL_SPEC, "DELETE FROM character_talent WHERE guid = ? and talent = ? and spec = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_TALENT, "INSERT INTO character_talent (guid, talent, spec) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_TALENT_PVP_BY_SPELL_SPEC, "DELETE FROM character_pvp_talent WHERE guid = ? and talent = ? and spec = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHAR_TALENT_PVP, "INSERT INTO character_pvp_talent (guid, talent, spec) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_ACTION_EXCEPT_SPEC, "DELETE FROM character_action WHERE spec<>? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHAR_LIST_SLOT, "UPDATE characters SET slot = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHAR_VOID_STORAGE, "SELECT itemId, itemEntry, slot, creatorGuid, randomPropertyType, randomProperty, suffixFactor FROM character_void_storage WHERE playerGuid = ? AND itemGuid = 0", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHAR_VOID_STORAGE_ITEM, "REPLACE INTO character_void_storage (itemId, playerGuid, itemEntry, slot, creatorGuid, randomPropertyType, randomProperty, suffixFactor, itemGuid) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_VOID_STORAGE_ITEM_BY_SLOT, "DELETE FROM character_void_storage WHERE slot = ? AND playerGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHAR_VOID_STORAGE_ITEM, "DELETE FROM character_void_storage WHERE itemId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHAR_VOID_STORAGE_ITEM, "SELECT " SelectItemInstanceContent ", vo.slot, vo.itemId FROM character_void_storage vo JOIN item_instance ii ON vo.itemGuid = ii.guid "
    "LEFT JOIN item_instance_gems ig ON ii.guid = ig.itemGuid "
    "LEFT JOIN item_instance_transmog iit ON ii.guid = iit.itemGuid "
    "LEFT JOIN item_instance_artifact iia ON ii.itemEntry = iia.itemEntry and vo.playerGuid = iia.char_guid "
    "LEFT JOIN item_instance_modifiers im ON ii.guid = im.itemGuid WHERE vo.playerGuid = ? AND vo.itemGuid != 0 ORDER BY slot", CONNECTION_ASYNC);

    // Guild Finder
    PrepareStatement(CHAR_REP_GUILD_FINDER_APPLICANT, "REPLACE INTO guild_finder_applicant (guildId, playerGuid, availability, classRole, interests, comment, submitTime) VALUES(?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_FINDER_APPLICANT, "DELETE FROM guild_finder_applicant WHERE guildId = ? AND playerGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_GUILD_FINDER_GUILD_SETTINGS, "REPLACE INTO guild_finder_guild_settings (guildId, availability, classRoles, interests, level, listed, comment) VALUES(?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_GUILD_FINDER_GUILD_SETTINGS, "DELETE FROM guild_finder_guild_settings WHERE guildId = ?", CONNECTION_ASYNC);

    // Challenge
    PrepareStatement(CHAR_INS_CHALLENGE, "INSERT INTO challenge (`ID`, `GuildID`, `MapID`, `RecordTime`, `Date`, `ChallengeLevel`, `TimerLevel`, `Affixes`, `ChestID`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHALLENGE_MEMBER, "INSERT INTO challenge_member (`id`, `member`, `specID`, `ChallengeLevel`, `Date`, `ChestID`) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHALLENGE_OPLOTE_LOOT, "REPLACE INTO challenge_oplote_loot (`guid`, `chestListID`, `date`, `ChallengeLevel`) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHALLENGE_OPLOTE_LOOT, "DELETE FROM challenge_oplote_loot WHERE date <= UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHALLENGE_OPLOTE_LOOT_BY_GUID, "DELETE FROM challenge_oplote_loot WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHALLENGE_MEMBER, "DELETE FROM challenge_member WHERE member = ?", CONNECTION_ASYNC);

    // CUF
    PrepareStatement(CHAR_SEL_CUF_PROFILES, "SELECT profileId, profileName, frameHeight, frameWidth, sortBy, healthText, someOptions, unk146, unk147, unk148, unk150, unk152, unk154 FROM character_cuf_profiles WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CUF_PROFILES, "REPLACE INTO character_cuf_profiles (guid, profileId, profileName, frameHeight, frameWidth, sortBy, healthText, someOptions, unk146, unk147, unk148, unk150, unk152, unk154) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_BOTH);

    // Battle Pets
#define PETBATTLE_FIELDS "slot, name, nameTimeStamp, species, quality, breed, level, xp, display, health, flags, infoPower, infoMaxHealth, infoSpeed, infoGender, account, declinedGenitive, declinedNative, declinedAccusative, declinedInstrumental, declinedPrepositional"
#define PETBATTLE_FULL_FIELDS "id, " PETBATTLE_FIELDS
    PrepareStatement(CHAR_SEL_PETBATTLE_ACCOUNT, "SELECT " PETBATTLE_FULL_FIELDS " FROM account_battlepet WHERE account = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_REP_PETBATTLE, "REPLACE INTO account_battlepet(id, " PETBATTLE_FIELDS ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", CONNECTION_BOTH);
    PrepareStatement(CHAR_INS_PETBATTLE, "INSERT INTO account_battlepet(" PETBATTLE_FIELDS ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", CONNECTION_BOTH);
    PrepareStatement(CHAR_DEL_PETBATTLE, "DELETE FROM account_battlepet WHERE id = ?", CONNECTION_BOTH);

    // Else
    PrepareStatement(CHAR_SEL_PERSONAL_RATE,       "SELECT rate FROM character_rates WHERE guid=? LIMIT 1",                                         CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PLAYER_VISUAL,       "SELECT head, shoulders, chest, waist, legs, feet, wrists, hands, back, main, off, ranged, tabard, shirt FROM character_visuals WHERE guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_MAILBOX_QUEUE,       "SELECT id, messageType, stationery, sender_guid, receiver_guid, subject, message, money, item, item_count FROM mailbox_queue LIMIT 500",  CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_MAILBOX_QUEUE,       "DELETE FROM mailbox_queue WHERE id = ?",                                       CONNECTION_ASYNC);

    // character_donate
    PrepareStatement(CHAR_ADD_ITEM_DONATE, "REPLACE INTO character_donate (`owner_guid`, `itemguid`, `itemEntry`, `efircount`, `count`, `state`) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_ITEM_DONATE_SET_STATE, "UPDATE character_donate SET state = ?, deletedate = ? WHERE itemguid = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_PLAYER_KILL, "SELECT victim_guid, count FROM character_kill WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_PLAYER_KILL, "DELETE FROM character_kill WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_KILL, "REPLACE INTO character_kill (guid, victim_guid, count) VALUES (?, ? ,?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_PLAYER_KILL, "UPDATE character_kill SET count=? WHERE guid = ? AND victim_guid = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_HONOR_INFO, "SELECT CurrentHonorAtLevel, PrestigeLevel, HonorLevel FROM character_honor WHERE Guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_HONOR_INFO_HONOR, "UPDATE character_honor SET CurrentHonorAtLevel = ? WHERE Guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_HONOR_INFO_HONOR_LEVEL, "UPDATE character_honor SET HonorLevel = ? WHERE Guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_HONOR_INFO_PRESTIGE_LEVEL, "UPDATE character_honor SET PrestigeLevel = ? WHERE Guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_HONOR_INFO, "REPLACE INTO character_honor (Guid, CurrentHonorAtLevel, PrestigeLevel, HonorLevel) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);

    //Loot cooldown system
    PrepareStatement(CHAR_SEL_PLAYER_LOOTCOOLDOWN, "SELECT entry, type, difficultyID, respawnTime FROM character_loot_cooldown WHERE guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PLAYER_LOOTCOOLDOWN, "REPLACE INTO character_loot_cooldown (`guid`, `entry`, `type`, `difficultyID`, `respawnTime`) VALUES (?, ?, ?, ?, ?)",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_PLAYER_LFGCOOLDOWN, "SELECT dungeonId, respawnTime FROM character_lfg_cooldown WHERE guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_PLAYER_LFGCOOLDOWN, "REPLACE INTO character_lfg_cooldown (`guid`, `dungeonId`, `respawnTime`) VALUES (?, ?, ?)",  CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_LAST_CHAR_UNDELETE, "SELECT LastCharacterUndelete FROM characters WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_LAST_CHAR_UNDELETE, "UPDATE characters SET LastCharacterUndelete = UNIX_TIMESTAMP() WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_RESTORE_DELETE_INFO, "UPDATE characters SET name = ?, account = ?, deleteDate = NULL, deleteInfos_Name = NULL, deleteInfos_Account = NULL WHERE deleteDate IS NOT NULL AND guid = ?", CONNECTION_ASYNC);

    // Black Market
    PrepareStatement(CHAR_SEL_BLACKMARKET_AUCTIONS, "SELECT marketId, currentBid, time, numBids, bidder FROM blackmarket_auctions", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_BLACKMARKET_AUCTIONS, "DELETE FROM blackmarket_auctions WHERE marketId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_BLACKMARKET_AUCTIONS, "UPDATE blackmarket_auctions SET currentBid = ?, time = ?, numBids = ?, bidder = ? WHERE marketId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_BLACKMARKET_AUCTIONS, "INSERT INTO blackmarket_auctions (marketId, currentBid, time, numBids, bidder) VALUES (?, ?, ?, ? ,?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_REP_CALENDAR_EVENT, "REPLACE INTO calendar_events (EventID, Owner, Title, Description, EventType, TextureID, Date, Flags, LockDate) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CALENDAR_EVENT, "DELETE FROM calendar_events WHERE EventID = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CALENDAR_INVITE, "REPLACE INTO calendar_invites (InviteID, EventID, Invitee, Sender, Status, ResponseTime, ModerationRank, Note) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CALENDAR_INVITE, "DELETE FROM calendar_invites WHERE InviteID = ?", CONNECTION_ASYNC);

    // Garrison
    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON, "SELECT SiteLevelId, FollowerActivationsRemainingToday, CacheLastUsage, _MissionGen FROM character_garrison WHERE CharacterGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON, "REPLACE INTO character_garrison (CharacterGuid, SiteLevelId, FollowerActivationsRemainingToday, CacheLastUsage, _MissionGen) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHARACTER_GARRISON, "UPDATE character_garrison SET SiteLevelId = ? WHERE CharacterGuid = ? AND SiteLevelId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON, "DELETE FROM character_garrison WHERE CharacterGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_ONLY, "DELETE FROM character_garrison WHERE CharacterGuid = ? and SiteLevelId in (5, 258, 444, 445, 6, 259)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_BY_SITE, "DELETE FROM character_garrison WHERE CharacterGuid = ? AND SiteLevelId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHARACTER_GARRISON_FOLLOWER_ACTIVATIONS, "UPDATE character_garrison SET FollowerActivationsRemainingToday = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_BLUEPRINTS, "SELECT buildingId FROM character_garrison_blueprints WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_BLUEPRINTS, "REPLACE INTO character_garrison_blueprints (guid, buildingId) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_BLUEPRINTS, "DELETE FROM character_garrison_blueprints WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_BLUEPRINTS_BY_BUILD, "DELETE FROM character_garrison_blueprints WHERE guid = ? AND buildingId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_BUILDINGS, "SELECT plotInstanceId, buildingId, timeBuilt, active, data FROM character_garrison_buildings WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_BUILDINGS, "REPLACE INTO character_garrison_buildings (guid, plotInstanceId, buildingId, timeBuilt, active, data) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_BUILDINGS, "DELETE FROM character_garrison_buildings WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_BUILDINGS_BY_PLOT, "DELETE FROM character_garrison_buildings WHERE guid = ? AND plotInstanceId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_FOLLOWERS, "SELECT dbId, followerId, quality, level, itemLevelWeapon, itemLevelArmor, xp, currentBuilding, currentMission, status, Durability FROM character_garrison_followers WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_FOLLOWERS, "REPLACE INTO character_garrison_followers (dbId, guid, followerId, quality, level, itemLevelWeapon, itemLevelArmor, xp, currentBuilding, currentMission, status, Durability) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWERS, "DELETE gfab, gf FROM character_garrison_follower_abilities gfab INNER JOIN character_garrison_followers gf ON gfab.dbId = gf.dbId WHERE gf.guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWER, "DELETE FROM character_garrison_followers WHERE dbId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWER_BY_DBID, "DELETE gfab, gf FROM character_garrison_follower_abilities gfab INNER JOIN character_garrison_followers gf ON gfab.dbId = gf.dbId WHERE gf.dbId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_FOLLOWER_ABILITIES, "SELECT gfab.dbId, gfab.abilityId FROM character_garrison_follower_abilities gfab INNER JOIN character_garrison_followers gf ON gfab.dbId = gf.dbId WHERE guid = ? ORDER BY gfab.slot", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_FOLLOWER_ABILITIES, "REPLACE INTO character_garrison_follower_abilities (dbId, abilityId, slot) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWER_ABILITIES, "DELETE FROM character_garrison_follower_abilities WHERE dbId = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_MISSIONS, "SELECT dbId, missionRecID, offerTime, offerDuration, startTime, travelDuration, missionDuration, missionState, chance FROM character_garrison_missions WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_MISSIONS, "REPLACE INTO character_garrison_missions (dbId, guid, missionRecID, offerTime, offerDuration, startTime, travelDuration, missionDuration, missionState, chance) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_MISSIONS, "DELETE FROM character_garrison_missions WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_MISSIONS_DB_ID, "DELETE FROM character_garrison_missions WHERE dbId = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_SHIPMENTS, "SELECT dbId, shipmentID, start, end FROM character_garrison_shipment WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_SHIPMENTS, "REPLACE INTO character_garrison_shipment (dbId, guid, shipmentID, start, end) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_SHIPMENTS, "DELETE FROM character_garrison_shipment WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_SHIPMENTS_DBID, "DELETE FROM character_garrison_shipment WHERE dbId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_SHIPMENTS_BY_DB_ID, "DELETE FROM character_garrison_shipment WHERE dbId = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHARACTER_GARRISON_TALENTS, "SELECT talentID, orderTime, flags  FROM character_garrison_talents WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_GARRISON_TALENTS, "REPLACE INTO character_garrison_talents (guid, talentID, orderTime, flags) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_TALENTS, "DELETE FROM character_garrison_talents WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_GARRISON_TALENT_BY_ID, "DELETE FROM character_garrison_talents WHERE guid = ? AND talentID = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_TOYS, "SELECT itemId, isFavourite FROM account_toys WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_TOYS, "REPLACE INTO account_toys (accountId, itemId, isFavourite) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_HEIRLOOMS, "SELECT itemId, flags FROM account_heirlooms WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_HEIRLOOMS, "REPLACE INTO account_heirlooms (accountId, itemId, flags) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHARACTER_COMPLAINTS, "SELECT ID, SpammerGuid, ComplaintType, MailID, TimeSinceOffence, MessageLog FROM report_complaints WHERE SpammerGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_COMPLAINTS, "INSERT INTO report_complaints (ID, ReportPlayer, ReportAccount, ReportTime, SpammerGuid, ComplaintType, MailID, TimeSinceOffence, MessageLog) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_TRANSMOGS, "SELECT `ModelID`, `Condition` FROM account_transmogs WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_TRANSMOGS, "REPLACE INTO account_transmogs (`account`, `guid`, `ModelID`, `Condition`) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_TRANSMOG, "DELETE FROM account_transmogs WHERE account = ? AND ModelID = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_BNET_ITEM_FAVORITE_APPEARANCES, "SELECT itemModifiedAppearanceId FROM account_item_favorite_appearances WHERE battlenetAccountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_BNET_ITEM_FAVORITE_APPEARANCE, "INSERT INTO account_item_favorite_appearances (battlenetAccountId, itemModifiedAppearanceId) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_BNET_ITEM_FAVORITE_APPEARANCE, "DELETE FROM account_item_favorite_appearances WHERE battlenetAccountId = ? AND itemModifiedAppearanceId = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_ITEM_INSTANCE_ARTIFACT, "SELECT ap.itemEntry, a.xp, a.artifactAppearanceId, ap.artifactPowerId, ap.purchasedRank, a.itemGuid FROM	item_instance_artifact_powers ap LEFT JOIN item_instance_artifact a ON ap.itemEntry = a.itemEntry and ap.char_guid = a.char_guid where ap.char_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_ITEM_INSTANCE_ARTIFACT, "REPLACE INTO item_instance_artifact (itemGuid, xp, artifactAppearanceId, itemEntry, tier, char_guid, totalrank) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_ARTIFACT, "DELETE FROM item_instance_artifact WHERE itemGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_ARTIFACT_BY_OWNER, "DELETE iia FROM item_instance_artifact iia LEFT JOIN item_instance ii ON iia.itemGuid = ii.guid WHERE ii.owner_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ITEM_INSTANCE_ARTIFACT_XP, "SELECT xp FROM item_instance_artifact where char_guid = ? AND itemEntry = ?", CONNECTION_SYNCH);

    PrepareStatement(CHAR_SEL_ITEM_INSTANCE_ARTIFACT_POWERS, "SELECT artifactPowerId, purchasedRank FROM item_instance_artifact_powers iiap inner join item_instance ii on ii.itemEntry = iiap.itemEntry and ii.owner_guid = iiap.char_guid WHERE ii.guid = ?", CONNECTION_BOTH);
    PrepareStatement(CHAR_REP_ITEM_INSTANCE_ARTIFACT_POWERS, "REPLACE INTO item_instance_artifact_powers (itemGuid, artifactPowerId, purchasedRank, itemEntry, char_guid) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_ARTIFACT_POWERS, "DELETE FROM item_instance_artifact_powers WHERE itemGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_ARTIFACT_POWERS_BY_OWNER, "DELETE iiap FROM item_instance_artifact_powers iiap LEFT JOIN item_instance ii ON iiap.itemGuid = ii.guid WHERE ii.owner_guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ITEM_INSTANCE_MODIFIERS, "INSERT INTO item_instance_modifiers (itemGuid, fixedScalingLevel, artifactKnowledgeLevel) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_REP_ITEM_INSTANCE_RELICS, "REPLACE INTO item_instance_relics (itemGuid, char_guid, first_relic, second_relic, third_relic) VALUES (?, ?, ?, ?, ?);", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_RELICS, "DELETE FROM item_instance_relics where itemGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_ITEM_INSTANCE_RELICS, "SELECT first_relic, second_relic, third_relic FROM item_instance_relics where itemGuid = ?", CONNECTION_BOTH);

    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_MODIFIERS, "DELETE FROM item_instance_modifiers WHERE itemGuid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_ITEM_INSTANCE_MODIFIERS_BY_OWNER, "DELETE im FROM item_instance_modifiers im LEFT JOIN item_instance ii ON im.itemGuid = ii.guid WHERE ii.owner_guid = ?", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_MOUNTS, "SELECT spell, flags FROM account_mounts WHERE account = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_MOUNTS, "REPLACE INTO account_mounts (account, spell, flags) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHARACTER_ADVENTURE_QUEST, "SELECT questID FROM character_adventure_quest WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHARACTER_ADVENTURE_QUEST, "DELETE FROM character_adventure_quest WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_CHARACTER_ADVENTURE_QUEST, "REPLACE INTO character_adventure_quest (guid, questID) VALUES (?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_REP_WORLD_QUEST, "REPLACE INTO world_quest (`QuestID`, `QuestInfoID`, `QuestSortID`, `VariableID`, `Value`, `Timer`, `StartTime`, `ResetTime`, `CurrencyID`, `CurrencyCount`, `Gold`, `ItemList`, `RewardType`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_WORLD_QUEST_BY_ID, "DELETE FROM world_quest WHERE QuestID = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_WORLD_QUEST_BY_TIMER, "DELETE FROM world_quest WHERE ResetTime <= UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_QUEST_STATUS_WORLD, "DELETE FROM character_queststatus_world WHERE ResetTime <= UNIX_TIMESTAMP()", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHARACTER_WORLDQUESTSTATUS, "SELECT quest, guid, resetTime FROM character_queststatus_world WHERE account = ? AND (guid = 0 OR guid = ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHARACTER_WORLDQUESTSTATUS, "REPLACE INTO character_queststatus_world (guid, quest, account, resetTime) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_WORLD_STATE_BY_STATE_INSTANCE, "DELETE FROM worldstate_data WHERE VariableID = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_WORLD_STAT, "INSERT INTO `worldstate_data` (`VariableID`, `Type`, `ConditionID`, `Flags`, `Value`) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_CHALLENGE_KEY, "SELECT ID, Level, Affix, Affix1, Affix2, KeyIsCharded, timeReset, InstanceID FROM challenge_key WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_CHALLENGE_KEY, "UPDATE challenge_key SET ID = ?, Level = ?, Affix = ?, Affix1 = ?, Affix2 = ?, KeyIsCharded = ?, timeReset = ?, InstanceID = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_CHALLENGE_KEY, "REPLACE INTO challenge_key (ID, Level, Affix, Affix1, Affix2, KeyIsCharded, timeReset, InstanceID, guid) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_DEL_CHALLENGE_KEY, "DELETE FROM challenge_key WHERE guid = ?", CONNECTION_ASYNC);
    
    PrepareStatement(CHAR_SEL_PLAYER_DEATHMATCH, "SELECT kills, deaths, damage, rating, matches FROM character_deathmatch WHERE guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_DEATHMATCH, "REPLACE INTO character_deathmatch (guid, kills, deaths, damage, rating, matches) values (?, ?, ?, ?, ?, ?)",  CONNECTION_ASYNC);
    
    PrepareStatement(CHAR_SEL_PLAYER_DEATHMATCH_STORE, "SELECT total_kills, selected_morph, buyed_morphs FROM character_deathmatch_store WHERE guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_DEATHMATCH_STORE, "REPLACE INTO character_deathmatch_store (guid, total_kills, selected_morph, buyed_morphs) values (?, ?, ?, ?)",  CONNECTION_ASYNC);
    
    PrepareStatement(CHAR_SEL_PLAYER_CHAT_LOGOS, "SELECT buyed_logo, active from character_chat_logos where guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_PLAYER_CHAT_LOGOS, "REPLACE INTO character_chat_logos (guid, buyed_logo, active) values (?, ?, ?)",  CONNECTION_ASYNC);
    
    PrepareStatement(CHAR_SEL_ARMY_TRAINING_INFO, "SELECT `opened_berserk`, `opened_mana_wanted`, `opened_mage`, `opened_hp`, `opened_dmg`, `opened_fixate`, `opened_brave`, `opened_chests` from character_army_training_info where guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_ARMY_TRAINING_INFO, "REPLACE INTO character_army_training_info (`guid`, `opened_berserk`, `opened_mana_wanted`, `opened_mage`, `opened_hp`, `opened_dmg`, `opened_fixate`, `opened_brave`, `opened_chests`) values (?, ?, ?, ?, ?, ?, ?, ?, ?)",  CONNECTION_ASYNC);

    PrepareStatement(CHAR_SEL_ACCOUNT_PROGRESS, "SELECT totaltime, leveltime from account_progress where account = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_REP_ACCOUNT_PROGRESS, "REPLACE INTO account_progress (account, totaltime, leveltime) values (?, ?, ?)",  CONNECTION_ASYNC);

    PrepareStatement(CHAR_REP_BAD_SENTENCES, "REPLACE INTO bad_sentences (`id`, `sentence`, `hash`, `penalty`, `sourceMask`,`output`) values (?, ?, ?, ?, ?, ?)",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_BAD_SENTENCES, "update bad_sentences set penalty = ? where id = ?",  CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_WEEKLY_BRACKET, "UPDATE character_brackets_info SET bestWeekLast = bestWeek, bestWeek = 0, weekGames = 0, weekWins = 0", CONNECTION_ASYNC);

    // Warden and anticheat (in future) augmenting ban
    PrepareStatement(CHAR_DEL_WARDEN_BAN_ATTEMPTS, "DELETE FROM account_ban_attempts WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_WARDEN_BAN_ATTEMPTS, "INSERT INTO account_ban_attempts (accountId, banAttempts) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHAR_SEL_WARDEN_BAN_ATTEMPTS, "SELECT banAttempts FROM account_ban_attempts WHERE accountId = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_UPD_WARDEN_BAN_ATTEMPTS, "UPDATE account_ban_attempts SET banAttempts = banAttempts + 1 WHERE accountId = ?", CONNECTION_ASYNC);

    // Flag account system
    PrepareStatement(CHAR_SEL_ACCOUNT_FLAGGED, "SELECT banduration FROM account_flagged WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHAR_SEL_ACCOUNT_FLAGGED_ALL, "SELECT id, banduration, bannedby, banreason FROM account_flagged", CONNECTION_SYNCH);
    PrepareStatement(CHAR_DEL_ACCOUNT_FLAGGED, "DELETE FROM account_flagged WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_ACCOUNT_FLAGGED, "INSERT IGNORE INTO account_flagged VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);

    // Temp for bots
    PrepareStatement(CHAR_INS_BOT_ACCOUNT, "INSERT IGNORE INTO account_bot_detected VALUES (?, ?, ?)", CONNECTION_ASYNC);

    // Stat System
    PrepareStatement(CHAR_SEL_KILL_CREATURE, "SELECT entry, count, point FROM character_stat_kill_creature WHERE guid = ?",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_INS_KILL_CREATURE, "REPLACE INTO character_stat_kill_creature (`guid`, `entry`, `count`, `point`) VALUES (?, ?, ?, ?)",  CONNECTION_ASYNC);
    PrepareStatement(CHAR_UPD_KILL_CREATURE, "UPDATE character_stat_kill_creature SET count = ?, point = ? WHERE guid = ? AND entry = ?", CONNECTION_ASYNC);
}
