/*
SQLyog Community v13.1.5  (64 bit)
MySQL - 5.6.47 : Database - characters
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `account_achievement` */

DROP TABLE IF EXISTS `account_achievement`;

CREATE TABLE `account_achievement` (
  `account` int(11) unsigned NOT NULL,
  `first_guid` int(11) unsigned NOT NULL,
  `achievement` int(11) unsigned NOT NULL,
  `date` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`achievement`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `account_achievement` */

/*Table structure for table `account_achievement_progress` */

DROP TABLE IF EXISTS `account_achievement_progress`;

CREATE TABLE `account_achievement_progress` (
  `account` int(11) unsigned NOT NULL,
  `criteria` int(11) unsigned NOT NULL,
  `counter` int(11) unsigned NOT NULL,
  `date` bigint(11) unsigned NOT NULL DEFAULT '0',
  `achievID` int(11) unsigned NOT NULL DEFAULT '0',
  `completed` int(3) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`criteria`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `criteria` (`criteria`) USING BTREE,
  KEY `achievID` (`achievID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `account_achievement_progress` */

/*Table structure for table `account_ban_attempts` */

DROP TABLE IF EXISTS `account_ban_attempts`;

CREATE TABLE `account_ban_attempts` (
  `accountId` int(11) unsigned NOT NULL DEFAULT '0',
  `banAttempts` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accountId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_ban_attempts` */

/*Table structure for table `account_battlepet` */

DROP TABLE IF EXISTS `account_battlepet`;

CREATE TABLE `account_battlepet` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `slot` int(11) NOT NULL DEFAULT '-1',
  `name` varchar(50) NOT NULL,
  `nameTimeStamp` int(10) unsigned NOT NULL DEFAULT '0',
  `species` int(10) unsigned NOT NULL DEFAULT '0',
  `quality` int(10) unsigned NOT NULL DEFAULT '0',
  `breed` int(10) unsigned NOT NULL DEFAULT '0',
  `level` int(10) unsigned NOT NULL DEFAULT '0',
  `xp` int(10) unsigned NOT NULL DEFAULT '0',
  `display` int(10) unsigned NOT NULL DEFAULT '0',
  `health` int(11) NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `infoPower` int(11) NOT NULL DEFAULT '0',
  `infoMaxHealth` int(11) NOT NULL DEFAULT '0',
  `infoSpeed` int(11) NOT NULL DEFAULT '0',
  `infoGender` int(11) NOT NULL DEFAULT '0',
  `declinedGenitive` varchar(50) NOT NULL,
  `declinedNative` varchar(50) NOT NULL,
  `declinedAccusative` varchar(50) NOT NULL,
  `declinedInstrumental` varchar(50) NOT NULL,
  `declinedPrepositional` varchar(50) NOT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `account_battlepet` */

/*Table structure for table `account_bot_detected` */

DROP TABLE IF EXISTS `account_bot_detected`;

CREATE TABLE `account_bot_detected` (
  `accountId` int(11) unsigned NOT NULL,
  `playerName` varchar(12) NOT NULL DEFAULT '',
  `botName` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`accountId`,`playerName`,`botName`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_bot_detected` */

/*Table structure for table `account_data` */

DROP TABLE IF EXISTS `account_data`;

CREATE TABLE `account_data` (
  `accountId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `data` blob NOT NULL,
  PRIMARY KEY (`accountId`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_data` */

/*Table structure for table `account_flagged` */

DROP TABLE IF EXISTS `account_flagged`;

CREATE TABLE `account_flagged` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Id',
  `banduration` int(10) unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_flagged` */

/*Table structure for table `account_heirlooms` */

DROP TABLE IF EXISTS `account_heirlooms`;

CREATE TABLE `account_heirlooms` (
  `accountId` int(10) unsigned NOT NULL,
  `itemId` int(11) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accountId`,`itemId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED;

/*Data for the table `account_heirlooms` */

/*Table structure for table `account_item_favorite_appearances` */

DROP TABLE IF EXISTS `account_item_favorite_appearances`;

CREATE TABLE `account_item_favorite_appearances` (
  `battlenetAccountId` int(10) unsigned NOT NULL,
  `itemModifiedAppearanceId` int(10) unsigned NOT NULL,
  PRIMARY KEY (`battlenetAccountId`,`itemModifiedAppearanceId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_item_favorite_appearances` */

/*Table structure for table `account_mounts` */

DROP TABLE IF EXISTS `account_mounts`;

CREATE TABLE `account_mounts` (
  `account` int(11) unsigned NOT NULL,
  `spell` int(11) unsigned NOT NULL DEFAULT '0',
  `flags` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`spell`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_mounts` */

/*Table structure for table `account_progress` */

DROP TABLE IF EXISTS `account_progress`;

CREATE TABLE `account_progress` (
  `account` int(10) unsigned NOT NULL,
  `totaltime` int(10) unsigned NOT NULL DEFAULT '0',
  `leveltime` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_progress` */

/*Table structure for table `account_toys` */

DROP TABLE IF EXISTS `account_toys`;

CREATE TABLE `account_toys` (
  `accountId` int(10) unsigned NOT NULL,
  `itemId` int(11) NOT NULL DEFAULT '0',
  `isFavourite` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`accountId`,`itemId`) USING BTREE,
  KEY `accountId` (`accountId`) USING BTREE,
  KEY `itemId` (`itemId`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED;

/*Data for the table `account_toys` */

/*Table structure for table `account_transmogs` */

DROP TABLE IF EXISTS `account_transmogs`;

CREATE TABLE `account_transmogs` (
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `guid` int(10) unsigned DEFAULT '0',
  `ModelID` int(10) unsigned NOT NULL DEFAULT '0',
  `Condition` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`,`ModelID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_transmogs` */

/*Table structure for table `account_tutorial` */

DROP TABLE IF EXISTS `account_tutorial`;

CREATE TABLE `account_tutorial` (
  `accountId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `tut0` int(10) unsigned NOT NULL DEFAULT '0',
  `tut1` int(10) unsigned NOT NULL DEFAULT '0',
  `tut2` int(10) unsigned NOT NULL DEFAULT '0',
  `tut3` int(10) unsigned NOT NULL DEFAULT '0',
  `tut4` int(10) unsigned NOT NULL DEFAULT '0',
  `tut5` int(10) unsigned NOT NULL DEFAULT '0',
  `tut6` int(10) unsigned NOT NULL DEFAULT '0',
  `tut7` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accountId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_tutorial` */

/*Table structure for table `addons` */

DROP TABLE IF EXISTS `addons`;

CREATE TABLE `addons` (
  `name` varchar(120) NOT NULL DEFAULT '',
  `crc` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Addons';

/*Data for the table `addons` */

/*Table structure for table `auctionhouse` */

DROP TABLE IF EXISTS `auctionhouse`;

CREATE TABLE `auctionhouse` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `auctioneerguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `itemguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `itemowner` bigint(20) unsigned NOT NULL DEFAULT '0',
  `buyoutprice` bigint(20) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `buyguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastbid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startbid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `deposit` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `item_guid` (`itemguid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `auctionhouse` */

/*Table structure for table `bad_sentences` */

DROP TABLE IF EXISTS `bad_sentences`;

CREATE TABLE `bad_sentences` (
  `id` int(20) unsigned NOT NULL,
  `sentence` varchar(255) NOT NULL,
  `hash` bigint(20) unsigned NOT NULL,
  `penalty` int(20) NOT NULL DEFAULT '10',
  `sourceMask` int(20) unsigned NOT NULL DEFAULT '1',
  `output` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`hash`) USING BTREE,
  KEY `hash` (`hash`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `bad_sentences` */

/*Table structure for table `blackmarket_auctions` */

DROP TABLE IF EXISTS `blackmarket_auctions`;

CREATE TABLE `blackmarket_auctions` (
  `marketId` int(10) NOT NULL DEFAULT '0',
  `currentBid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `time` int(10) NOT NULL DEFAULT '0',
  `numBids` int(10) NOT NULL DEFAULT '0',
  `bidder` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`marketId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `blackmarket_auctions` */

/*Table structure for table `calendar_events` */

DROP TABLE IF EXISTS `calendar_events`;

CREATE TABLE `calendar_events` (
  `EventID` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Owner` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Title` varchar(255) NOT NULL DEFAULT '',
  `Description` varchar(255) NOT NULL DEFAULT '',
  `EventType` tinyint(1) unsigned NOT NULL DEFAULT '4',
  `TextureID` int(10) NOT NULL DEFAULT '-1',
  `Date` int(10) unsigned NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `LockDate` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`EventID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `calendar_events` */

/*Table structure for table `calendar_invites` */

DROP TABLE IF EXISTS `calendar_invites`;

CREATE TABLE `calendar_invites` (
  `InviteID` bigint(20) unsigned NOT NULL DEFAULT '0',
  `EventID` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Invitee` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Sender` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Status` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `ResponseTime` int(10) unsigned NOT NULL DEFAULT '0',
  `ModerationRank` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `Note` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`InviteID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `calendar_invites` */

/*Table structure for table `challenge` */

DROP TABLE IF EXISTS `challenge`;

CREATE TABLE `challenge` (
  `ID` int(11) NOT NULL,
  `GuildID` bigint(20) NOT NULL DEFAULT '0',
  `MapID` mediumint(9) NOT NULL DEFAULT '0',
  `ChallengeID` mediumint(6) NOT NULL DEFAULT '0',
  `RecordTime` int(11) NOT NULL DEFAULT '0',
  `Date` int(11) NOT NULL DEFAULT '0',
  `ChallengeLevel` smallint(6) NOT NULL DEFAULT '0',
  `TimerLevel` smallint(6) NOT NULL DEFAULT '0',
  `Affixes` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `ChestID` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`) USING BTREE,
  KEY `mapID` (`MapID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `challenge` */

/*Table structure for table `challenge_key` */

DROP TABLE IF EXISTS `challenge_key`;

CREATE TABLE `challenge_key` (
  `guid` int(10) NOT NULL DEFAULT '0',
  `ID` mediumint(6) unsigned NOT NULL DEFAULT '0',
  `Level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Affix` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Affix1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Affix2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `KeyIsCharded` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `timeReset` int(10) unsigned NOT NULL DEFAULT '0',
  `InstanceID` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `timeReset` (`timeReset`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `challenge_key` */

/*Table structure for table `challenge_member` */

DROP TABLE IF EXISTS `challenge_member`;

CREATE TABLE `challenge_member` (
  `id` int(11) NOT NULL,
  `member` bigint(20) NOT NULL DEFAULT '0',
  `specID` mediumint(6) NOT NULL DEFAULT '0',
  `ChallengeLevel` smallint(6) NOT NULL DEFAULT '0',
  `Date` int(10) NOT NULL DEFAULT '0',
  `ChestID` int(10) NOT NULL DEFAULT '0',
  KEY `id` (`id`,`member`) USING BTREE,
  KEY `member` (`member`) USING BTREE,
  KEY `Date` (`Date`) USING BTREE,
  KEY `ChallengeLevel` (`ChallengeLevel`) USING BTREE,
  KEY `ChestID` (`ChestID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci ROW_FORMAT=COMPACT;

/*Data for the table `challenge_member` */

/*Table structure for table `challenge_oplote_loot` */

DROP TABLE IF EXISTS `challenge_oplote_loot`;

CREATE TABLE `challenge_oplote_loot` (
  `guid` int(10) NOT NULL DEFAULT '0',
  `chestListID` varchar(150) NOT NULL DEFAULT '',
  `date` int(10) NOT NULL DEFAULT '0',
  `ChallengeLevel` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `challenge_oplote_loot` */

/*Table structure for table `channels` */

DROP TABLE IF EXISTS `channels`;

CREATE TABLE `channels` (
  `name` varchar(128) NOT NULL,
  `team` int(10) unsigned NOT NULL,
  `announce` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `ownership` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `password` varchar(32) DEFAULT NULL,
  `bannedList` text,
  `lastUsed` int(10) unsigned NOT NULL,
  PRIMARY KEY (`name`,`team`) USING BTREE,
  KEY `name` (`name`) USING BTREE,
  KEY `team` (`team`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Channel System';

/*Data for the table `channels` */

/*Table structure for table `character_account_data` */

DROP TABLE IF EXISTS `character_account_data`;

CREATE TABLE `character_account_data` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `data` blob NOT NULL,
  PRIMARY KEY (`guid`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_account_data` */

/*Table structure for table `character_achievement` */

DROP TABLE IF EXISTS `character_achievement`;

CREATE TABLE `character_achievement` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `achievement` int(11) unsigned NOT NULL,
  `date` int(11) unsigned NOT NULL,
  PRIMARY KEY (`guid`,`achievement`) USING BTREE,
  KEY `achievement` (`achievement`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_achievement` */

/*Table structure for table `character_achievement_progress` */

DROP TABLE IF EXISTS `character_achievement_progress`;

CREATE TABLE `character_achievement_progress` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `criteria` int(11) unsigned NOT NULL,
  `counter` int(11) unsigned NOT NULL,
  `date` bigint(11) unsigned NOT NULL DEFAULT '0',
  `achievID` int(11) unsigned NOT NULL DEFAULT '0',
  `completed` int(3) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`criteria`) USING BTREE,
  KEY `criteria` (`criteria`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `achievID` (`achievID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_achievement_progress` */

/*Table structure for table `character_action` */

DROP TABLE IF EXISTS `character_action`;

CREATE TABLE `character_action` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `button` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `action` int(10) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spec`,`button`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_action` */

/*Table structure for table `character_adventure_quest` */

DROP TABLE IF EXISTS `character_adventure_quest`;

CREATE TABLE `character_adventure_quest` (
  `guid` bigint(20) unsigned NOT NULL,
  `questID` int(11) unsigned NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `questID` (`questID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `character_adventure_quest` */

/*Table structure for table `character_archaeology` */

DROP TABLE IF EXISTS `character_archaeology`;

CREATE TABLE `character_archaeology` (
  `guid` bigint(20) NOT NULL,
  `sites` text NOT NULL,
  `counts` text NOT NULL,
  `projects` text NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Archaeology System';

/*Data for the table `character_archaeology` */

/*Table structure for table `character_archaeology_finds` */

DROP TABLE IF EXISTS `character_archaeology_finds`;

CREATE TABLE `character_archaeology_finds` (
  `guid` bigint(20) NOT NULL,
  `id` int(11) NOT NULL,
  `count` int(11) NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`guid`,`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_archaeology_finds` */

/*Table structure for table `character_army_training_info` */

DROP TABLE IF EXISTS `character_army_training_info`;

CREATE TABLE `character_army_training_info` (
  `guid` bigint(20) NOT NULL,
  `opened_berserk` int(11) NOT NULL DEFAULT '0',
  `opened_mana_wanted` int(11) NOT NULL DEFAULT '0',
  `opened_mage` int(11) NOT NULL DEFAULT '0',
  `opened_hp` tinyint(1) NOT NULL DEFAULT '0',
  `opened_dmg` tinyint(1) NOT NULL DEFAULT '0',
  `opened_fixate` tinyint(1) NOT NULL DEFAULT '0',
  `opened_brave` tinyint(1) NOT NULL DEFAULT '0',
  `opened_chests` varchar(255) NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_army_training_info` */

/*Table structure for table `character_aura` */

DROP TABLE IF EXISTS `character_aura`;

CREATE TABLE `character_aura` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `caster_guid` binary(16) NOT NULL COMMENT 'Full Global Unique Identifier',
  `item_guid` binary(16) NOT NULL,
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `effect_mask` mediumint(3) unsigned NOT NULL DEFAULT '0',
  `recalculate_mask` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stackcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `maxduration` int(11) NOT NULL DEFAULT '0',
  `remaintime` int(11) NOT NULL DEFAULT '0',
  `remaincharges` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`caster_guid`,`item_guid`,`spell`,`effect_mask`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `caster_guid` (`caster_guid`) USING BTREE,
  KEY `item_guid` (`item_guid`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE,
  KEY `effect_mask` (`effect_mask`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_aura` */

/*Table structure for table `character_aura_effect` */

DROP TABLE IF EXISTS `character_aura_effect`;

CREATE TABLE `character_aura_effect` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `effect` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `baseamount` int(11) NOT NULL DEFAULT '0',
  `amount` int(11) NOT NULL,
  PRIMARY KEY (`guid`,`slot`,`effect`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `slot` (`slot`) USING BTREE,
  KEY `effect` (`effect`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `character_aura_effect` */

/*Table structure for table `character_banned` */

DROP TABLE IF EXISTS `character_banned`;

CREATE TABLE `character_banned` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `bandate` int(10) unsigned NOT NULL DEFAULT '0',
  `unbandate` int(10) unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  `active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`guid`,`bandate`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `bandate` (`bandate`) USING BTREE,
  KEY `unbandate` (`unbandate`) USING BTREE,
  KEY `active` (`active`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Ban List';

/*Data for the table `character_banned` */

/*Table structure for table `character_battleground_data` */

DROP TABLE IF EXISTS `character_battleground_data`;

CREATE TABLE `character_battleground_data` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL COMMENT 'Instance Identifier',
  `team` smallint(5) unsigned NOT NULL,
  `joinX` float NOT NULL DEFAULT '0',
  `joinY` float NOT NULL DEFAULT '0',
  `joinZ` float NOT NULL DEFAULT '0',
  `joinO` float NOT NULL DEFAULT '0',
  `joinMapId` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `taxiStart` int(10) unsigned NOT NULL DEFAULT '0',
  `taxiEnd` int(10) unsigned NOT NULL DEFAULT '0',
  `mountSpell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `lastActiveSpec` smallint(6) DEFAULT NULL,
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_battleground_data` */

/*Table structure for table `character_battleground_random` */

DROP TABLE IF EXISTS `character_battleground_random`;

CREATE TABLE `character_battleground_random` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `bg` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rbg` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `skirmish` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `arena` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `arena2v2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `arena3v3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `brawl` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `brawlArena` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_battleground_random` */

/*Table structure for table `character_brackets_info` */

DROP TABLE IF EXISTS `character_brackets_info`;

CREATE TABLE `character_brackets_info` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `bracket` smallint(6) NOT NULL,
  `rating` mediumint(9) NOT NULL DEFAULT '0',
  `oldRating` mediumint(9) NOT NULL DEFAULT '0',
  `best` mediumint(9) NOT NULL DEFAULT '0',
  `oldBest` mediumint(9) NOT NULL DEFAULT '0',
  `bestWeek` smallint(9) NOT NULL DEFAULT '0',
  `mmr` mediumint(9) NOT NULL DEFAULT '0',
  `oldMmr` mediumint(9) NOT NULL DEFAULT '0',
  `games` int(11) NOT NULL DEFAULT '0',
  `oldGames` int(11) NOT NULL DEFAULT '0',
  `wins` int(11) NOT NULL DEFAULT '0',
  `oldWins` int(11) NOT NULL DEFAULT '0',
  `weekGames` mediumint(9) NOT NULL DEFAULT '0',
  `weekWins` mediumint(9) NOT NULL DEFAULT '0',
  `bestWeekLast` mediumint(9) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`bracket`) USING BTREE,
  KEY `BracketID` (`bracket`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_brackets_info` */

/*Table structure for table `character_chat_logos` */

DROP TABLE IF EXISTS `character_chat_logos`;

CREATE TABLE `character_chat_logos` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `buyed_logo` varchar(255) NOT NULL,
  `active` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`buyed_logo`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_chat_logos` */

/*Table structure for table `character_cuf_profiles` */

DROP TABLE IF EXISTS `character_cuf_profiles`;

CREATE TABLE `character_cuf_profiles` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `profileId` tinyint(3) unsigned NOT NULL,
  `profileName` varchar(12) NOT NULL,
  `frameHeight` smallint(5) unsigned NOT NULL DEFAULT '0',
  `frameWidth` smallint(5) unsigned NOT NULL DEFAULT '0',
  `sortBy` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `healthText` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `someOptions` int(10) unsigned NOT NULL DEFAULT '0',
  `unk146` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `unk147` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `unk148` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `unk150` smallint(5) unsigned NOT NULL DEFAULT '0',
  `unk152` smallint(5) unsigned NOT NULL DEFAULT '0',
  `unk154` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`profileId`) USING BTREE,
  KEY `profileId` (`profileId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_cuf_profiles` */

/*Table structure for table `character_currency` */

DROP TABLE IF EXISTS `character_currency`;

CREATE TABLE `character_currency` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `currency` smallint(5) unsigned NOT NULL,
  `total_count` int(11) unsigned NOT NULL,
  `week_count` int(11) unsigned NOT NULL,
  `season_total` int(11) DEFAULT '0',
  `flags` int(11) DEFAULT '0',
  `curentcap` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`currency`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_currency` */

/*Table structure for table `character_custom_event_reapeter` */

DROP TABLE IF EXISTS `character_custom_event_reapeter`;

CREATE TABLE `character_custom_event_reapeter` (
  `guid` bigint(20) NOT NULL,
  `event` int(10) unsigned NOT NULL,
  `repeat_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`guid`,`event`,`repeat_id`) USING BTREE,
  KEY `select` (`guid`,`event`,`repeat_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_custom_event_reapeter` */

/*Table structure for table `character_deathmatch` */

DROP TABLE IF EXISTS `character_deathmatch`;

CREATE TABLE `character_deathmatch` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `kills` int(11) NOT NULL DEFAULT '0',
  `deaths` int(11) NOT NULL DEFAULT '0',
  `damage` bigint(20) NOT NULL DEFAULT '0',
  `rating` int(11) NOT NULL DEFAULT '0',
  `matches` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_deathmatch` */

/*Table structure for table `character_deathmatch_products` */

DROP TABLE IF EXISTS `character_deathmatch_products`;

CREATE TABLE `character_deathmatch_products` (
  `name` varchar(255) NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `product` varchar(255) NOT NULL,
  `cost` int(11) NOT NULL,
  PRIMARY KEY (`product`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_deathmatch_products` */

/*Table structure for table `character_deathmatch_store` */

DROP TABLE IF EXISTS `character_deathmatch_store`;

CREATE TABLE `character_deathmatch_store` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `total_kills` int(11) NOT NULL DEFAULT '0',
  `selected_morph` int(11) NOT NULL DEFAULT '0',
  `buyed_morphs` varchar(255) NOT NULL,
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_deathmatch_store` */

/*Table structure for table `character_declinedname` */

DROP TABLE IF EXISTS `character_declinedname`;

CREATE TABLE `character_declinedname` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `genitive` varchar(15) NOT NULL DEFAULT '',
  `dative` varchar(15) NOT NULL DEFAULT '',
  `accusative` varchar(15) NOT NULL DEFAULT '',
  `instrumental` varchar(15) NOT NULL DEFAULT '',
  `prepositional` varchar(15) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_declinedname` */

/*Table structure for table `character_donate` */

DROP TABLE IF EXISTS `character_donate`;

CREATE TABLE `character_donate` (
  `owner_guid` int(10) unsigned NOT NULL,
  `itemguid` int(10) unsigned NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `itemEntry` int(10) unsigned NOT NULL,
  `efircount` int(10) unsigned NOT NULL,
  `count` int(10) unsigned NOT NULL,
  `state` int(11) NOT NULL DEFAULT '0',
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `deletedate` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `account` int(11) NOT NULL DEFAULT '0',
  UNIQUE KEY `index` (`owner_guid`,`itemguid`,`type`) USING BTREE,
  KEY `owner_guid` (`owner_guid`) USING BTREE,
  KEY `itemguid` (`itemguid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `state` (`state`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_donate` */

/*Table structure for table `character_equipmentsets` */

DROP TABLE IF EXISTS `character_equipmentsets`;

CREATE TABLE `character_equipmentsets` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `setguid` bigint(20) NOT NULL AUTO_INCREMENT,
  `setindex` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `name` varchar(31) NOT NULL,
  `iconname` varchar(100) NOT NULL,
  `ignore_mask` int(11) unsigned NOT NULL DEFAULT '0',
  `AssignedSpecIndex` int(11) NOT NULL DEFAULT '-1',
  `item0` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item1` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item3` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item4` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item5` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item6` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item7` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item8` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item9` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item10` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item11` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item12` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item13` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item14` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item15` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item16` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item17` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item18` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`setguid`) USING BTREE,
  UNIQUE KEY `idx_set` (`guid`,`setguid`,`setindex`) USING BTREE,
  KEY `Idx_setindex` (`setindex`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_equipmentsets` */

/*Table structure for table `character_garrison` */

DROP TABLE IF EXISTS `character_garrison`;

CREATE TABLE `character_garrison` (
  `CharacterGuid` bigint(20) unsigned NOT NULL,
  `SiteLevelId` int(10) unsigned NOT NULL DEFAULT '0',
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `FollowerActivationsRemainingToday` int(10) unsigned NOT NULL DEFAULT '0',
  `NumFollowerActivationRegenTimestamp` int(10) unsigned NOT NULL DEFAULT '0',
  `CacheLastUsage` int(11) NOT NULL DEFAULT '0',
  `_MissionGen` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`CharacterGuid`,`SiteLevelId`,`GarrTypeId`) USING BTREE,
  KEY `guid` (`CharacterGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison` */

/*Table structure for table `character_garrison_blueprints` */

DROP TABLE IF EXISTS `character_garrison_blueprints`;

CREATE TABLE `character_garrison_blueprints` (
  `guid` bigint(20) unsigned NOT NULL,
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `buildingId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`buildingId`,`GarrTypeId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `buildingId` (`buildingId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_blueprints` */

/*Table structure for table `character_garrison_buildings` */

DROP TABLE IF EXISTS `character_garrison_buildings`;

CREATE TABLE `character_garrison_buildings` (
  `guid` bigint(20) unsigned NOT NULL,
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `plotInstanceId` int(10) unsigned NOT NULL DEFAULT '0',
  `buildingId` int(10) unsigned NOT NULL DEFAULT '0',
  `timeBuilt` bigint(20) unsigned NOT NULL,
  `data` varchar(64) DEFAULT NULL,
  `active` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`GarrTypeId`,`plotInstanceId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `plotInstanceId` (`plotInstanceId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_buildings` */

/*Table structure for table `character_garrison_follower_abilities` */

DROP TABLE IF EXISTS `character_garrison_follower_abilities`;

CREATE TABLE `character_garrison_follower_abilities` (
  `dbId` bigint(20) unsigned NOT NULL,
  `abilityId` int(10) unsigned NOT NULL,
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `isItemTrait` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`,`slot`,`GarrTypeId`) USING BTREE,
  KEY `dbId` (`dbId`) USING BTREE,
  KEY `abilityId` (`abilityId`) USING BTREE,
  KEY `slot` (`slot`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_follower_abilities` */

/*Table structure for table `character_garrison_followers` */

DROP TABLE IF EXISTS `character_garrison_followers`;

CREATE TABLE `character_garrison_followers` (
  `dbId` bigint(20) unsigned NOT NULL,
  `guid` bigint(20) unsigned NOT NULL,
  `followerId` int(10) unsigned NOT NULL DEFAULT '0',
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `Durability` smallint(3) unsigned NOT NULL DEFAULT '0',
  `quality` int(10) unsigned NOT NULL DEFAULT '2',
  `level` int(10) unsigned NOT NULL DEFAULT '90',
  `itemLevelWeapon` int(10) unsigned NOT NULL DEFAULT '600',
  `itemLevelArmor` int(10) unsigned NOT NULL DEFAULT '600',
  `xp` int(10) unsigned NOT NULL DEFAULT '0',
  `currentBuilding` int(10) unsigned NOT NULL DEFAULT '0',
  `currentMission` int(10) unsigned NOT NULL DEFAULT '0',
  `status` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`) USING BTREE,
  KEY `idx_guid_id` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_followers` */

/*Table structure for table `character_garrison_missions` */

DROP TABLE IF EXISTS `character_garrison_missions`;

CREATE TABLE `character_garrison_missions` (
  `dbId` bigint(20) unsigned NOT NULL,
  `guid` bigint(20) unsigned NOT NULL,
  `missionRecID` int(10) unsigned NOT NULL DEFAULT '0',
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `offerTime` int(10) unsigned NOT NULL DEFAULT '0',
  `offerDuration` int(10) unsigned NOT NULL DEFAULT '0',
  `startTime` int(10) unsigned NOT NULL DEFAULT '0',
  `travelDuration` int(10) unsigned NOT NULL DEFAULT '0',
  `missionDuration` int(10) unsigned NOT NULL DEFAULT '0',
  `missionState` int(10) unsigned NOT NULL DEFAULT '0',
  `chance` smallint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`) USING BTREE,
  UNIQUE KEY `idx_guid_id` (`guid`,`missionRecID`) USING BTREE,
  KEY `dbId` (`dbId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `missionRecID` (`missionRecID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_missions` */

/*Table structure for table `character_garrison_shipment` */

DROP TABLE IF EXISTS `character_garrison_shipment`;

CREATE TABLE `character_garrison_shipment` (
  `dbId` bigint(20) NOT NULL,
  `guid` bigint(10) NOT NULL,
  `shipmentID` smallint(5) NOT NULL,
  `GarrTypeId` smallint(5) NOT NULL DEFAULT '0',
  `start` int(11) unsigned NOT NULL DEFAULT '0',
  `end` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`) USING BTREE,
  KEY `dbId` (`dbId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_shipment` */

/*Table structure for table `character_garrison_talents` */

DROP TABLE IF EXISTS `character_garrison_talents`;

CREATE TABLE `character_garrison_talents` (
  `guid` bigint(20) NOT NULL,
  `GarrTypeId` int(10) unsigned NOT NULL DEFAULT '0',
  `talentID` int(11) NOT NULL,
  `orderTime` int(11) NOT NULL,
  `flags` int(11) NOT NULL,
  PRIMARY KEY (`guid`,`GarrTypeId`,`talentID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `character_garrison_talents` */

/*Table structure for table `character_gifts` */

DROP TABLE IF EXISTS `character_gifts`;

CREATE TABLE `character_gifts` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `item_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_gifts` */

/*Table structure for table `character_glyphs` */

DROP TABLE IF EXISTS `character_glyphs`;

CREATE TABLE `character_glyphs` (
  `guid` bigint(20) NOT NULL,
  `talentGroup` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `glyphId` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talentGroup`,`glyphId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `talentGroup` (`talentGroup`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_glyphs` */

/*Table structure for table `character_homebind` */

DROP TABLE IF EXISTS `character_homebind`;

CREATE TABLE `character_homebind` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mapId` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `zoneId` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Zone Identifier',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_homebind` */

/*Table structure for table `character_honor` */

DROP TABLE IF EXISTS `character_honor`;

CREATE TABLE `character_honor` (
  `Guid` bigint(20) NOT NULL DEFAULT '0',
  `CurrentHonorAtLevel` mediumint(10) NOT NULL DEFAULT '0',
  `HonorLevel` smallint(5) NOT NULL DEFAULT '0',
  `PrestigeLevel` smallint(5) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Honor info system';

/*Data for the table `character_honor` */

/*Table structure for table `character_instance` */

DROP TABLE IF EXISTS `character_instance`;

CREATE TABLE `character_instance` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `instance` int(10) unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `difficulty` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `completedEncounters` int(10) unsigned NOT NULL DEFAULT '0',
  `data` tinytext NOT NULL,
  `Extended` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `resetTime` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`instance`) USING BTREE,
  KEY `instance` (`instance`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_instance` */

/*Table structure for table `character_inventory` */

DROP TABLE IF EXISTS `character_inventory`;

CREATE TABLE `character_inventory` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `bag` bigint(20) unsigned NOT NULL DEFAULT '0',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `item` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item`) USING BTREE,
  UNIQUE KEY `guid` (`guid`,`bag`,`slot`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `bag` (`bag`) USING BTREE,
  KEY `slot` (`slot`) USING BTREE,
  KEY `item` (`item`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_inventory` */

/*Table structure for table `character_kill` */

DROP TABLE IF EXISTS `character_kill`;

CREATE TABLE `character_kill` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `victim_guid` bigint(20) NOT NULL DEFAULT '0',
  `count` tinyint(3) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`victim_guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `victim_guid` (`victim_guid`) USING BTREE,
  KEY `count` (`count`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Kills Yesterday';

/*Data for the table `character_kill` */

/*Table structure for table `character_lfg_cooldown` */

DROP TABLE IF EXISTS `character_lfg_cooldown`;

CREATE TABLE `character_lfg_cooldown` (
  `guid` int(10) NOT NULL DEFAULT '0',
  `dungeonId` int(10) NOT NULL DEFAULT '0',
  `respawnTime` int(10) DEFAULT '0',
  PRIMARY KEY (`guid`,`dungeonId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_lfg_cooldown` */

/*Table structure for table `character_loot_cooldown` */

DROP TABLE IF EXISTS `character_loot_cooldown`;

CREATE TABLE `character_loot_cooldown` (
  `guid` int(11) unsigned NOT NULL,
  `type` tinyint(3) NOT NULL DEFAULT '0',
  `entry` int(11) NOT NULL DEFAULT '0',
  `difficultyID` tinyint(3) NOT NULL DEFAULT '0',
  `respawnTime` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`type`,`entry`,`difficultyID`) USING BTREE,
  KEY `entry` (`entry`) USING BTREE,
  KEY `respawnTime` (`respawnTime`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_loot_cooldown` */

/*Table structure for table `character_pet` */

DROP TABLE IF EXISTS `character_pet`;

CREATE TABLE `character_pet` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `owner` bigint(20) unsigned NOT NULL DEFAULT '0',
  `modelid` int(10) unsigned DEFAULT '0',
  `CreatedBySpell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `PetType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` smallint(5) unsigned NOT NULL DEFAULT '1',
  `exp` int(10) unsigned NOT NULL DEFAULT '0',
  `Reactstate` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `name` varchar(21) NOT NULL DEFAULT 'Pet',
  `renamed` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `curhealth` int(10) unsigned NOT NULL DEFAULT '1',
  `curmana` int(10) unsigned NOT NULL DEFAULT '0',
  `savetime` int(10) unsigned NOT NULL DEFAULT '0',
  `abdata` text,
  `specialization` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `owner` (`owner`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `entry` (`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Pet System';

/*Data for the table `character_pet` */

/*Table structure for table `character_pet_declinedname` */

DROP TABLE IF EXISTS `character_pet_declinedname`;

CREATE TABLE `character_pet_declinedname` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `owner` bigint(20) unsigned NOT NULL DEFAULT '0',
  `genitive` varchar(12) NOT NULL DEFAULT '',
  `dative` varchar(12) NOT NULL DEFAULT '',
  `accusative` varchar(12) NOT NULL DEFAULT '',
  `instrumental` varchar(12) NOT NULL DEFAULT '',
  `prepositional` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `owner_key` (`owner`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_pet_declinedname` */

/*Table structure for table `character_pvp_talent` */

DROP TABLE IF EXISTS `character_pvp_talent`;

CREATE TABLE `character_pvp_talent` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `talent` mediumint(8) unsigned NOT NULL,
  `spec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talent`,`spec`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `talent` (`talent`) USING BTREE,
  KEY `spec` (`spec`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_pvp_talent` */

/*Table structure for table `character_queststatus` */

DROP TABLE IF EXISTS `character_queststatus`;

CREATE TABLE `character_queststatus` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0',
  `status` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `timer` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_queststatus` */

/*Table structure for table `character_queststatus_daily` */

DROP TABLE IF EXISTS `character_queststatus_daily`;

CREATE TABLE `character_queststatus_daily` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(11) NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_queststatus_daily` */

/*Table structure for table `character_queststatus_objectives` */

DROP TABLE IF EXISTS `character_queststatus_objectives`;

CREATE TABLE `character_queststatus_objectives` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0',
  `objective` tinyint(3) NOT NULL DEFAULT '0',
  `data` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`,`objective`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `quest` (`quest`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_queststatus_objectives` */

/*Table structure for table `character_queststatus_rewarded` */

DROP TABLE IF EXISTS `character_queststatus_rewarded`;

CREATE TABLE `character_queststatus_rewarded` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(11) NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_queststatus_rewarded` */

/*Table structure for table `character_queststatus_seasonal` */

DROP TABLE IF EXISTS `character_queststatus_seasonal`;

CREATE TABLE `character_queststatus_seasonal` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(11) NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  `event` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Event Identifier',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_queststatus_seasonal` */

/*Table structure for table `character_queststatus_weekly` */

DROP TABLE IF EXISTS `character_queststatus_weekly`;

CREATE TABLE `character_queststatus_weekly` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(11) NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Quest Identifier',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `idx_guid` (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_queststatus_weekly` */

/*Table structure for table `character_queststatus_world` */

DROP TABLE IF EXISTS `character_queststatus_world`;

CREATE TABLE `character_queststatus_world` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `account` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` int(10) unsigned NOT NULL DEFAULT '0',
  `resetTime` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`quest`) USING BTREE,
  KEY `resetTime` (`resetTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_queststatus_world` */

/*Table structure for table `character_rates` */

DROP TABLE IF EXISTS `character_rates`;

CREATE TABLE `character_rates` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `rate` tinyint(2) DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `character_rates` */

/*Table structure for table `character_reputation` */

DROP TABLE IF EXISTS `character_reputation`;

CREATE TABLE `character_reputation` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `standing` int(11) NOT NULL DEFAULT '0',
  `flags` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`faction`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `faction` (`faction`) USING BTREE,
  KEY `standing` (`standing`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_reputation` */

/*Table structure for table `character_reward` */

DROP TABLE IF EXISTS `character_reward`;

CREATE TABLE `character_reward` (
  `guid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `owner_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `type` int(5) unsigned NOT NULL DEFAULT '0',
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `count` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `owner_guid` (`owner_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_reward` */

/*Table structure for table `character_skills` */

DROP TABLE IF EXISTS `character_skills`;

CREATE TABLE `character_skills` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `skill` smallint(5) unsigned NOT NULL,
  `value` smallint(5) unsigned NOT NULL,
  `max` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`guid`,`skill`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `skill` (`skill`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_skills` */

/*Table structure for table `character_social` */

DROP TABLE IF EXISTS `character_social`;

CREATE TABLE `character_social` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `friend` bigint(20) unsigned NOT NULL DEFAULT '0',
  `flags` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Friend Flags',
  `note` varchar(48) NOT NULL DEFAULT '' COMMENT 'Friend Note',
  PRIMARY KEY (`guid`,`friend`,`flags`) USING BTREE,
  KEY `friend` (`friend`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_social` */

/*Table structure for table `character_spell` */

DROP TABLE IF EXISTS `character_spell`;

CREATE TABLE `character_spell` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `disabled` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `character_spell` */

/*Table structure for table `character_spell_cooldown` */

DROP TABLE IF EXISTS `character_spell_cooldown`;

CREATE TABLE `character_spell_cooldown` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spell` mediumint(8) NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `item` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Item Identifier',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_spell_cooldown` */

/*Table structure for table `character_stat_kill_creature` */

DROP TABLE IF EXISTS `character_stat_kill_creature`;

CREATE TABLE `character_stat_kill_creature` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned DEFAULT '0',
  `point` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`entry`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `entry` (`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_stat_kill_creature` */

/*Table structure for table `character_talent` */

DROP TABLE IF EXISTS `character_talent`;

CREATE TABLE `character_talent` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `talent` mediumint(8) unsigned NOT NULL,
  `spec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talent`,`spec`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `spec` (`spec`) USING BTREE,
  KEY `talent` (`talent`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_talent` */

/*Table structure for table `character_transmog_outfits` */

DROP TABLE IF EXISTS `character_transmog_outfits`;

CREATE TABLE `character_transmog_outfits` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `setguid` bigint(20) NOT NULL AUTO_INCREMENT,
  `setindex` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `name` varchar(128) NOT NULL,
  `iconname` varchar(256) NOT NULL,
  `ignore_mask` int(11) NOT NULL DEFAULT '0',
  `appearance0` int(10) NOT NULL DEFAULT '0',
  `appearance1` int(10) NOT NULL DEFAULT '0',
  `appearance2` int(10) NOT NULL DEFAULT '0',
  `appearance3` int(10) NOT NULL DEFAULT '0',
  `appearance4` int(10) NOT NULL DEFAULT '0',
  `appearance5` int(10) NOT NULL DEFAULT '0',
  `appearance6` int(10) NOT NULL DEFAULT '0',
  `appearance7` int(10) NOT NULL DEFAULT '0',
  `appearance8` int(10) NOT NULL DEFAULT '0',
  `appearance9` int(10) NOT NULL DEFAULT '0',
  `appearance10` int(10) NOT NULL DEFAULT '0',
  `appearance11` int(10) NOT NULL DEFAULT '0',
  `appearance12` int(10) NOT NULL DEFAULT '0',
  `appearance13` int(10) NOT NULL DEFAULT '0',
  `appearance14` int(10) NOT NULL DEFAULT '0',
  `appearance15` int(10) NOT NULL DEFAULT '0',
  `appearance16` int(10) NOT NULL DEFAULT '0',
  `appearance17` int(10) NOT NULL DEFAULT '0',
  `appearance18` int(10) NOT NULL DEFAULT '0',
  `mainHandEnchant` int(10) NOT NULL DEFAULT '0',
  `offHandEnchant` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`setguid`) USING BTREE,
  UNIQUE KEY `idx_set` (`guid`,`setguid`,`setindex`) USING BTREE,
  KEY `Idx_setindex` (`setindex`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_transmog_outfits` */

/*Table structure for table `character_visuals` */

DROP TABLE IF EXISTS `character_visuals`;

CREATE TABLE `character_visuals` (
  `guid` bigint(20) NOT NULL DEFAULT '0',
  `head` int(11) NOT NULL DEFAULT '0',
  `shoulders` int(11) NOT NULL DEFAULT '0',
  `chest` int(11) NOT NULL DEFAULT '0',
  `waist` int(11) NOT NULL DEFAULT '0',
  `legs` int(11) NOT NULL DEFAULT '0',
  `feet` int(11) NOT NULL DEFAULT '0',
  `wrists` int(11) NOT NULL DEFAULT '0',
  `hands` int(11) NOT NULL DEFAULT '0',
  `back` int(11) NOT NULL DEFAULT '0',
  `main` int(11) NOT NULL DEFAULT '0',
  `off` int(11) NOT NULL DEFAULT '0',
  `ranged` int(11) NOT NULL DEFAULT '0',
  `tabard` int(11) unsigned NOT NULL DEFAULT '0',
  `shirt` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_visuals` */

/*Table structure for table `character_void_storage` */

DROP TABLE IF EXISTS `character_void_storage`;

CREATE TABLE `character_void_storage` (
  `itemId` bigint(20) unsigned NOT NULL,
  `playerGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `itemEntry` mediumint(8) unsigned NOT NULL,
  `slot` int(10) unsigned NOT NULL,
  `creatorGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `randomPropertyType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `randomProperty` int(10) unsigned NOT NULL DEFAULT '0',
  `suffixFactor` int(10) unsigned NOT NULL DEFAULT '0',
  `itemGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemId`) USING BTREE,
  UNIQUE KEY `idx_player_slot` (`playerGuid`,`slot`) USING BTREE,
  KEY `idx_player` (`playerGuid`) USING BTREE,
  KEY `randomProperty` (`randomProperty`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `character_void_storage` */

/*Table structure for table `characters` */

DROP TABLE IF EXISTS `characters`;

CREATE TABLE `characters` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `account` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account Identifier',
  `name` varchar(12) NOT NULL,
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `xp` int(10) unsigned NOT NULL DEFAULT '0',
  `money` bigint(20) unsigned NOT NULL DEFAULT '0',
  `skin` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `face` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `hairStyle` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `hairColor` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `tattoo` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `horn` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `inventorySlots` tinyint(3) unsigned NOT NULL DEFAULT '16',
  `blindfold` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `facialStyle` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `bankSlots` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `drunk` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `playerFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `playerFlagsEx` int(10) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `instance_id` int(10) unsigned NOT NULL DEFAULT '0',
  `dungeonDifficulty` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `raidDifficulty` tinyint(3) unsigned NOT NULL DEFAULT '14',
  `legacyRaidDifficulty` tinyint(3) unsigned NOT NULL DEFAULT '3',
  `orientation` float NOT NULL DEFAULT '0',
  `taximask` text NOT NULL,
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `cinematic` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `totaltime` int(10) unsigned NOT NULL DEFAULT '0',
  `leveltime` int(10) unsigned NOT NULL DEFAULT '0',
  `created_time` int(10) NOT NULL DEFAULT '0',
  `logout_time` int(10) unsigned NOT NULL DEFAULT '0',
  `is_logout_resting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rest_bonus` float NOT NULL DEFAULT '0',
  `trans_x` float NOT NULL DEFAULT '0',
  `trans_y` float NOT NULL DEFAULT '0',
  `trans_z` float NOT NULL DEFAULT '0',
  `trans_o` float NOT NULL DEFAULT '0',
  `transguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `extra_flags` smallint(5) unsigned NOT NULL DEFAULT '0',
  `at_login` smallint(5) unsigned NOT NULL DEFAULT '0',
  `zone` smallint(5) unsigned NOT NULL DEFAULT '0',
  `death_expire_time` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_path` text,
  `totalKills` int(10) unsigned NOT NULL DEFAULT '0',
  `todayKills` smallint(5) unsigned NOT NULL DEFAULT '0',
  `yesterdayKills` smallint(5) unsigned NOT NULL DEFAULT '0',
  `killPoints` float NOT NULL DEFAULT '0',
  `chosenTitle` int(10) unsigned NOT NULL DEFAULT '0',
  `watchedFaction` int(10) NOT NULL DEFAULT '-1',
  `lfgBonusFaction` int(10) unsigned NOT NULL DEFAULT '0',
  `health` int(10) unsigned NOT NULL DEFAULT '0',
  `mana` int(10) unsigned NOT NULL DEFAULT '0',
  `latency` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `activespec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `specialization` int(11) NOT NULL,
  `lootspecialization` int(11) NOT NULL DEFAULT '0',
  `exploredZones` longtext,
  `equipmentCache` longtext,
  `knownTitles` longtext,
  `actionBars` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `currentpetnumber` int(11) NOT NULL,
  `petslot` longtext NOT NULL,
  `grantableLevels` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `deleteInfos_Account` int(10) unsigned DEFAULT NULL,
  `deleteInfos_Name` varchar(12) DEFAULT NULL,
  `deleteDate` int(10) unsigned DEFAULT NULL,
  `LastCharacterUndelete` int(10) unsigned DEFAULT NULL,
  `transfer` tinyint(3) NOT NULL DEFAULT '0',
  `transfer_request` int(11) unsigned NOT NULL DEFAULT '0',
  `transfer_history` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `online` (`online`) USING BTREE,
  KEY `name` (`name`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `characters` */

/*Table structure for table `corpse` */

DROP TABLE IF EXISTS `corpse`;

CREATE TABLE `corpse` (
  `corpseGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `mapId` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Map Identifier',
  `phaseMask` smallint(5) unsigned NOT NULL DEFAULT '1',
  `displayId` int(10) unsigned NOT NULL DEFAULT '0',
  `itemCache` text NOT NULL,
  `bytes1` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `dynFlags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  `corpseType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`corpseGuid`) USING BTREE,
  KEY `idx_type` (`corpseType`) USING BTREE,
  KEY `idx_instance` (`instanceId`) USING BTREE,
  KEY `idx_player` (`guid`) USING BTREE,
  KEY `idx_time` (`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Death System';

/*Data for the table `corpse` */

/*Table structure for table `creature_respawn` */

DROP TABLE IF EXISTS `creature_respawn`;

CREATE TABLE `creature_respawn` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `respawnTime` int(10) unsigned NOT NULL DEFAULT '0',
  `mapId` smallint(10) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`guid`,`instanceId`) USING BTREE,
  KEY `instanceId` (`instanceId`) USING BTREE,
  KEY `respawnTime` (`respawnTime`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `mapId` (`mapId`) USING BTREE,
  KEY `all` (`guid`,`respawnTime`,`mapId`,`instanceId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Grid Loading System';

/*Data for the table `creature_respawn` */

/*Table structure for table `game_event_condition_save` */

DROP TABLE IF EXISTS `game_event_condition_save`;

CREATE TABLE `game_event_condition_save` (
  `eventEntry` mediumint(6) unsigned NOT NULL,
  `condition_id` int(10) unsigned NOT NULL DEFAULT '0',
  `done` float DEFAULT '0',
  PRIMARY KEY (`eventEntry`,`condition_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `game_event_condition_save` */

/*Table structure for table `game_event_save` */

DROP TABLE IF EXISTS `game_event_save`;

CREATE TABLE `game_event_save` (
  `eventEntry` mediumint(6) unsigned NOT NULL,
  `state` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `next_start` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`eventEntry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `game_event_save` */

/*Table structure for table `gameobject_respawn` */

DROP TABLE IF EXISTS `gameobject_respawn`;

CREATE TABLE `gameobject_respawn` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `respawnTime` int(10) unsigned NOT NULL DEFAULT '0',
  `mapId` smallint(10) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Instance Identifier',
  PRIMARY KEY (`guid`,`instanceId`) USING BTREE,
  KEY `instanceId` (`instanceId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `mapId` (`mapId`) USING BTREE,
  KEY `respawnTime` (`respawnTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Grid Loading System';

/*Data for the table `gameobject_respawn` */

/*Table structure for table `gm_subsurveys` */

DROP TABLE IF EXISTS `gm_subsurveys`;

CREATE TABLE `gm_subsurveys` (
  `surveyId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `subsurveyId` int(10) unsigned NOT NULL DEFAULT '0',
  `rank` int(10) unsigned NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  PRIMARY KEY (`surveyId`,`subsurveyId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `gm_subsurveys` */

/*Table structure for table `gm_surveys` */

DROP TABLE IF EXISTS `gm_surveys`;

CREATE TABLE `gm_surveys` (
  `surveyId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mainSurvey` int(10) unsigned NOT NULL DEFAULT '0',
  `overallComment` longtext NOT NULL,
  `createTime` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`surveyId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `gm_surveys` */

/*Table structure for table `gm_tickets` */

DROP TABLE IF EXISTS `gm_tickets`;

CREATE TABLE `gm_tickets` (
  `ticketId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `name` varchar(12) NOT NULL COMMENT 'Name of ticket creator',
  `message` text NOT NULL,
  `createTime` int(10) unsigned NOT NULL DEFAULT '0',
  `mapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `lastModifiedTime` int(10) unsigned NOT NULL DEFAULT '0',
  `closedBy` bigint(20) NOT NULL DEFAULT '0',
  `assignedTo` bigint(20) unsigned NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  `completed` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `escalated` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `viewed` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ticketId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player System';

/*Data for the table `gm_tickets` */

/*Table structure for table `group_instance` */

DROP TABLE IF EXISTS `group_instance`;

CREATE TABLE `group_instance` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `instance` int(10) unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `difficulty` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `completedEncounters` int(10) unsigned NOT NULL DEFAULT '0',
  `resetTime` int(10) unsigned NOT NULL DEFAULT '0',
  `data` tinytext NOT NULL,
  PRIMARY KEY (`guid`,`instance`) USING BTREE,
  KEY `instance` (`instance`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `group_instance` */

/*Table structure for table `group_member` */

DROP TABLE IF EXISTS `group_member`;

CREATE TABLE `group_member` (
  `guid` int(10) unsigned NOT NULL,
  `memberGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `memberFlags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `subgroup` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `roles` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`memberGuid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `memberGuid` (`memberGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Groups';

/*Data for the table `group_member` */

/*Table structure for table `groups` */

DROP TABLE IF EXISTS `groups`;

CREATE TABLE `groups` (
  `guid` int(10) unsigned NOT NULL,
  `leaderGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lootMethod` tinyint(3) unsigned NOT NULL,
  `looterGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lootThreshold` tinyint(3) unsigned NOT NULL,
  `icon1` binary(16) NOT NULL,
  `icon2` binary(16) NOT NULL,
  `icon3` binary(16) NOT NULL,
  `icon4` binary(16) NOT NULL,
  `icon5` binary(16) NOT NULL,
  `icon6` binary(16) NOT NULL,
  `icon7` binary(16) NOT NULL,
  `icon8` binary(16) NOT NULL,
  `groupType` tinyint(3) unsigned NOT NULL,
  `difficulty` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `raidDifficulty` tinyint(3) unsigned NOT NULL DEFAULT '14',
  `legacyRaidDifficulty` tinyint(3) unsigned NOT NULL DEFAULT '3',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `leaderGuid` (`leaderGuid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Groups';

/*Data for the table `groups` */

/*Table structure for table `guild` */

DROP TABLE IF EXISTS `guild`;

CREATE TABLE `guild` (
  `guildid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL DEFAULT '',
  `leaderguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `flags` int(11) unsigned NOT NULL DEFAULT '0',
  `EmblemStyle` int(10) NOT NULL DEFAULT '-1',
  `EmblemColor` int(10) NOT NULL DEFAULT '-1',
  `BorderStyle` int(10) NOT NULL DEFAULT '-1',
  `BorderColor` int(10) NOT NULL DEFAULT '-1',
  `BackgroundColor` int(10) NOT NULL DEFAULT '-1',
  `info` text NOT NULL,
  `motd` varchar(128) NOT NULL DEFAULT '',
  `createdate` int(10) unsigned NOT NULL DEFAULT '0',
  `BankMoney` bigint(20) unsigned NOT NULL DEFAULT '0',
  `level` int(10) unsigned DEFAULT '1',
  PRIMARY KEY (`guildid`) USING BTREE,
  KEY `guildid` (`guildid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Guild System';

/*Data for the table `guild` */

/*Table structure for table `guild_achievement` */

DROP TABLE IF EXISTS `guild_achievement`;

CREATE TABLE `guild_achievement` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `achievement` int(11) unsigned NOT NULL,
  `date` int(11) unsigned NOT NULL DEFAULT '0',
  `guids` text NOT NULL,
  PRIMARY KEY (`guildId`,`achievement`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_achievement` */

/*Table structure for table `guild_achievement_progress` */

DROP TABLE IF EXISTS `guild_achievement_progress`;

CREATE TABLE `guild_achievement_progress` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `criteria` int(11) unsigned NOT NULL,
  `counter` int(11) unsigned NOT NULL,
  `date` bigint(11) unsigned NOT NULL DEFAULT '0',
  `completedGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `achievID` int(10) unsigned NOT NULL,
  `completed` int(3) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`criteria`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_achievement_progress` */

/*Table structure for table `guild_bank_eventlog` */

DROP TABLE IF EXISTS `guild_bank_eventlog`;

CREATE TABLE `guild_bank_eventlog` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `LogGuid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Log record identificator - auxiliary column',
  `TabId` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Guild bank TabId',
  `EventType` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Event type',
  `PlayerGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ItemOrMoney` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ItemStackCount` smallint(5) unsigned NOT NULL DEFAULT '0',
  `DestTabId` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Destination Tab Id',
  `TimeStamp` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Event UNIX time',
  PRIMARY KEY (`guildId`,`LogGuid`,`TabId`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE,
  KEY `Idx_PlayerGuid` (`PlayerGuid`) USING BTREE,
  KEY `Idx_LogGuid` (`LogGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_bank_eventlog` */

/*Table structure for table `guild_bank_item` */

DROP TABLE IF EXISTS `guild_bank_item`;

CREATE TABLE `guild_bank_item` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SlotId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `item_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`TabId`,`SlotId`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE,
  KEY `Idx_item_guid` (`item_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_bank_item` */

/*Table structure for table `guild_bank_right` */

DROP TABLE IF EXISTS `guild_bank_right`;

CREATE TABLE `guild_bank_right` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gbright` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SlotPerDay` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`TabId`,`rid`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_bank_right` */

/*Table structure for table `guild_bank_tab` */

DROP TABLE IF EXISTS `guild_bank_tab`;

CREATE TABLE `guild_bank_tab` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `TabId` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TabName` varchar(16) NOT NULL DEFAULT '',
  `TabIcon` varchar(100) NOT NULL DEFAULT '',
  `TabText` varchar(500) DEFAULT NULL,
  PRIMARY KEY (`guildId`,`TabId`) USING BTREE,
  KEY `guildid_key` (`guildId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_bank_tab` */

/*Table structure for table `guild_challenges` */

DROP TABLE IF EXISTS `guild_challenges`;

CREATE TABLE `guild_challenges` (
  `GuildId` int(10) unsigned NOT NULL,
  `ChallengeType` int(1) NOT NULL DEFAULT '0',
  `ChallengeCount` int(1) DEFAULT NULL,
  PRIMARY KEY (`GuildId`,`ChallengeType`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_challenges` */

/*Table structure for table `guild_eventlog` */

DROP TABLE IF EXISTS `guild_eventlog`;

CREATE TABLE `guild_eventlog` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `LogGuid` int(10) unsigned NOT NULL COMMENT 'Log record identificator - auxiliary column',
  `EventType` tinyint(3) unsigned NOT NULL COMMENT 'Event type',
  `PlayerGuid1` bigint(20) unsigned NOT NULL DEFAULT '0',
  `PlayerGuid2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `NewRank` tinyint(3) unsigned NOT NULL COMMENT 'New rank(in case promotion/demotion)',
  `TimeStamp` int(10) unsigned NOT NULL COMMENT 'Event UNIX time',
  PRIMARY KEY (`guildId`,`LogGuid`) USING BTREE,
  KEY `Idx_PlayerGuid1` (`PlayerGuid1`) USING BTREE,
  KEY `Idx_PlayerGuid2` (`PlayerGuid2`) USING BTREE,
  KEY `Idx_LogGuid` (`LogGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Guild Eventlog';

/*Data for the table `guild_eventlog` */

/*Table structure for table `guild_finder_applicant` */

DROP TABLE IF EXISTS `guild_finder_applicant`;

CREATE TABLE `guild_finder_applicant` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `playerGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `availability` tinyint(3) unsigned DEFAULT '0',
  `classRole` tinyint(3) unsigned DEFAULT '0',
  `interests` tinyint(3) unsigned DEFAULT '0',
  `comment` varchar(255) DEFAULT NULL,
  `submitTime` int(10) unsigned DEFAULT NULL,
  UNIQUE KEY `guildId` (`guildId`,`playerGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `guild_finder_applicant` */

/*Table structure for table `guild_finder_guild_settings` */

DROP TABLE IF EXISTS `guild_finder_guild_settings`;

CREATE TABLE `guild_finder_guild_settings` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `availability` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `classRoles` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `interests` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `listed` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `comment` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`guildId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `guild_finder_guild_settings` */

/*Table structure for table `guild_member` */

DROP TABLE IF EXISTS `guild_member`;

CREATE TABLE `guild_member` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `rank` tinyint(3) unsigned NOT NULL,
  `pnote` varchar(31) NOT NULL DEFAULT '',
  `offnote` varchar(31) NOT NULL DEFAULT '',
  `BankResetTimeMoney` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemMoney` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab0` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab0` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab1` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab1` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab2` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab2` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab3` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab3` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab4` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab4` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab5` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab5` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab6` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab6` int(10) unsigned NOT NULL DEFAULT '0',
  `BankResetTimeTab7` int(10) unsigned NOT NULL DEFAULT '0',
  `BankRemSlotsTab7` int(10) unsigned NOT NULL DEFAULT '0',
  `XpContrib` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total guild XP contributed.',
  `XpContribWeek` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Guild XP contributed this week.',
  `AchPoint` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Achievement Points',
  `profId1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `profValue1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `profRank1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `recipesMask1` varchar(1300) NOT NULL DEFAULT '',
  `profId2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `profValue2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `profRank2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `recipesMask2` varchar(1300) NOT NULL DEFAULT '',
  `RepWeek` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'Guild rep earned this week.',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guildid_rank_key` (`guildId`,`rank`) USING BTREE,
  KEY `guildId` (`guildId`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Guild System';

/*Data for the table `guild_member` */

/*Table structure for table `guild_newslog` */

DROP TABLE IF EXISTS `guild_newslog`;

CREATE TABLE `guild_newslog` (
  `guildid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `LogGuid` int(10) unsigned NOT NULL DEFAULT '0',
  `EventType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `PlayerGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `Value` int(10) unsigned NOT NULL DEFAULT '0',
  `TimeStamp` int(10) unsigned NOT NULL DEFAULT '0',
  `Data` text NOT NULL,
  PRIMARY KEY (`guildid`,`LogGuid`) USING BTREE,
  KEY `guildid_key` (`guildid`) USING BTREE,
  KEY `Idx_PlayerGuid` (`PlayerGuid`) USING BTREE,
  KEY `Idx_LogGuid` (`LogGuid`) USING BTREE,
  KEY `TimeStamp` (`TimeStamp`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `guild_newslog` */

/*Table structure for table `guild_rank` */

DROP TABLE IF EXISTS `guild_rank`;

CREATE TABLE `guild_rank` (
  `guildId` bigint(20) unsigned NOT NULL DEFAULT '0',
  `rid` tinyint(3) unsigned NOT NULL,
  `rname` varchar(20) NOT NULL DEFAULT '',
  `rights` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `BankMoneyPerDay` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`,`rid`) USING BTREE,
  KEY `Idx_rid` (`rid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Guild System';

/*Data for the table `guild_rank` */

/*Table structure for table `item_instance` */

DROP TABLE IF EXISTS `item_instance`;

CREATE TABLE `item_instance` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `itemEntry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `owner_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `creatorGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `giftCreatorGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned NOT NULL DEFAULT '1',
  `duration` int(10) NOT NULL DEFAULT '0',
  `charges` tinytext,
  `flags` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `enchantments` text NOT NULL,
  `randomPropertyType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `randomPropertyId` int(10) unsigned NOT NULL DEFAULT '0',
  `durability` smallint(5) unsigned NOT NULL DEFAULT '0',
  `playedTime` int(10) unsigned NOT NULL DEFAULT '0',
  `text` text,
  `upgradeId` int(10) unsigned NOT NULL DEFAULT '0',
  `battlePetSpeciesId` int(10) unsigned NOT NULL DEFAULT '0',
  `battlePetBreedData` int(10) unsigned NOT NULL DEFAULT '0',
  `battlePetLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `battlePetDisplayId` int(10) unsigned NOT NULL DEFAULT '0',
  `bonusListIDs` text,
  `itemLevel` mediumint(6) NOT NULL DEFAULT '0',
  `dungeonEncounterID` mediumint(6) NOT NULL DEFAULT '0',
  `contextID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `createdTime` int(10) unsigned NOT NULL DEFAULT '0',
  `challengeMapID` int(10) unsigned NOT NULL DEFAULT '0',
  `challengeLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `challengeAffix` int(10) unsigned NOT NULL DEFAULT '0',
  `challengeAffix1` int(10) unsigned NOT NULL DEFAULT '0',
  `challengeAffix2` int(10) unsigned NOT NULL DEFAULT '0',
  `challengeKeyIsCharded` int(10) unsigned NOT NULL DEFAULT '0',
  `isdonateitem` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `owner_guid` (`owner_guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `randomPropertyId` (`randomPropertyId`) USING BTREE,
  KEY `contextID` (`contextID`) USING BTREE,
  KEY `dungeonEncounterID` (`dungeonEncounterID`) USING BTREE,
  KEY `createdTime` (`createdTime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Item System';

/*Data for the table `item_instance` */

/*Table structure for table `item_instance_artifact` */

DROP TABLE IF EXISTS `item_instance_artifact`;

CREATE TABLE `item_instance_artifact` (
  `itemGuid` bigint(20) unsigned NOT NULL,
  `xp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `artifactAppearanceId` int(10) unsigned NOT NULL DEFAULT '0',
  `itemEntry` int(10) NOT NULL,
  `tier` int(10) NOT NULL DEFAULT '0',
  `char_guid` bigint(20) NOT NULL,
  `totalrank` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemEntry`,`char_guid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `tier` (`tier`) USING BTREE,
  KEY `char_guid` (`char_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `item_instance_artifact` */

/*Table structure for table `item_instance_artifact_powers` */

DROP TABLE IF EXISTS `item_instance_artifact_powers`;

CREATE TABLE `item_instance_artifact_powers` (
  `itemGuid` bigint(20) unsigned NOT NULL,
  `artifactPowerId` int(10) unsigned NOT NULL,
  `purchasedRank` tinyint(3) unsigned DEFAULT '0',
  `itemEntry` int(10) NOT NULL,
  `char_guid` bigint(20) NOT NULL,
  `totalrank` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemEntry`,`char_guid`,`artifactPowerId`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE,
  KEY `itemEntry` (`itemEntry`) USING BTREE,
  KEY `char_guid` (`char_guid`) USING BTREE,
  KEY `artifactPowerId` (`artifactPowerId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `item_instance_artifact_powers` */

/*Table structure for table `item_instance_gems` */

DROP TABLE IF EXISTS `item_instance_gems`;

CREATE TABLE `item_instance_gems` (
  `itemGuid` bigint(20) unsigned NOT NULL,
  `gemItemId1` int(10) unsigned NOT NULL DEFAULT '0',
  `gemBonuses1` text,
  `gemContext1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gemScalingLevel1` int(10) unsigned NOT NULL DEFAULT '0',
  `gemItemId2` int(10) unsigned NOT NULL DEFAULT '0',
  `gemBonuses2` text,
  `gemScalingLevel2` int(10) unsigned NOT NULL DEFAULT '0',
  `gemContext2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gemItemId3` int(10) unsigned NOT NULL DEFAULT '0',
  `gemBonuses3` text,
  `gemContext3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `gemScalingLevel3` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `item_instance_gems` */

/*Table structure for table `item_instance_modifiers` */

DROP TABLE IF EXISTS `item_instance_modifiers`;

CREATE TABLE `item_instance_modifiers` (
  `itemGuid` bigint(20) unsigned NOT NULL,
  `fixedScalingLevel` int(10) unsigned DEFAULT '0',
  `artifactKnowledgeLevel` int(10) unsigned DEFAULT '0',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `item_instance_modifiers` */

/*Table structure for table `item_instance_relics` */

DROP TABLE IF EXISTS `item_instance_relics`;

CREATE TABLE `item_instance_relics` (
  `itemGuid` bigint(20) unsigned NOT NULL,
  `char_guid` bigint(20) NOT NULL,
  `first_relic` text NOT NULL,
  `second_relic` text NOT NULL,
  `third_relic` text NOT NULL,
  PRIMARY KEY (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `item_instance_relics` */

/*Table structure for table `item_instance_transmog` */

DROP TABLE IF EXISTS `item_instance_transmog`;

CREATE TABLE `item_instance_transmog` (
  `itemGuid` bigint(20) unsigned NOT NULL,
  `itemModifiedAppearanceAllSpecs` int(11) NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec1` int(11) NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec2` int(11) NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec3` int(11) NOT NULL DEFAULT '0',
  `itemModifiedAppearanceSpec4` int(11) NOT NULL DEFAULT '0',
  `spellItemEnchantmentAllSpecs` int(11) NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec1` int(11) NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec2` int(11) NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec3` int(11) NOT NULL DEFAULT '0',
  `spellItemEnchantmentSpec4` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `item_instance_transmog` */

/*Table structure for table `item_refund_instance` */

DROP TABLE IF EXISTS `item_refund_instance`;

CREATE TABLE `item_refund_instance` (
  `item_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `player_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `paidMoney` int(10) unsigned NOT NULL DEFAULT '0',
  `paidExtendedCost` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`,`player_guid`) USING BTREE,
  KEY `item_guid` (`item_guid`) USING BTREE,
  KEY `player_guid` (`player_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Item Refund System';

/*Data for the table `item_refund_instance` */

/*Table structure for table `item_soulbound_trade_data` */

DROP TABLE IF EXISTS `item_soulbound_trade_data`;

CREATE TABLE `item_soulbound_trade_data` (
  `itemGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `allowedPlayers` text NOT NULL COMMENT 'Space separated GUID list of players who can receive this item in trade',
  PRIMARY KEY (`itemGuid`) USING BTREE,
  KEY `itemGuid` (`itemGuid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Item Refund System';

/*Data for the table `item_soulbound_trade_data` */

/*Table structure for table `lfg_data` */

DROP TABLE IF EXISTS `lfg_data`;

CREATE TABLE `lfg_data` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `dungeon` int(10) unsigned NOT NULL DEFAULT '0',
  `state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='LFG Data';

/*Data for the table `lfg_data` */

/*Table structure for table `log_faction_change` */

DROP TABLE IF EXISTS `log_faction_change`;

CREATE TABLE `log_faction_change` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `guid` bigint(20) NOT NULL,
  `account` int(11) NOT NULL,
  `OldRace` int(11) NOT NULL,
  `NewRace` int(11) NOT NULL,
  `date` datetime NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=MyISAM AUTO_INCREMENT=3901 DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

/*Data for the table `log_faction_change` */

/*Table structure for table `log_rename` */

DROP TABLE IF EXISTS `log_rename`;

CREATE TABLE `log_rename` (
  `guid` bigint(20) unsigned NOT NULL,
  `date` datetime NOT NULL,
  `oldName` tinytext CHARACTER SET latin1 NOT NULL,
  `newName` tinytext CHARACTER SET latin1 NOT NULL,
  KEY `guid` (`guid`) USING BTREE,
  KEY `oldName` (`oldName`(50)) USING BTREE,
  KEY `newName` (`newName`(50)) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `log_rename` */

/*Table structure for table `mail` */

DROP TABLE IF EXISTS `mail`;

CREATE TABLE `mail` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Identifier',
  `messageType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stationery` tinyint(3) NOT NULL DEFAULT '41',
  `mailTemplateId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `sender` bigint(20) unsigned NOT NULL DEFAULT '0',
  `receiver` bigint(20) unsigned NOT NULL DEFAULT '0',
  `subject` longtext,
  `body` longtext,
  `has_items` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `expire_time` int(10) unsigned NOT NULL DEFAULT '0',
  `deliver_time` int(10) unsigned NOT NULL DEFAULT '0',
  `money` bigint(20) unsigned NOT NULL DEFAULT '0',
  `cod` bigint(20) unsigned NOT NULL DEFAULT '0',
  `checked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `receiver` (`receiver`) USING BTREE,
  KEY `checked` (`checked`) USING BTREE,
  KEY `deliver_time` (`deliver_time`) USING BTREE,
  KEY `expire_time` (`expire_time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Mail System';

/*Data for the table `mail` */

/*Table structure for table `mail_items` */

DROP TABLE IF EXISTS `mail_items`;

CREATE TABLE `mail_items` (
  `mail_id` int(10) unsigned NOT NULL DEFAULT '0',
  `item_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `receiver` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item_guid`) USING BTREE,
  KEY `idx_receiver` (`receiver`) USING BTREE,
  KEY `idx_mail_id` (`mail_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `mail_items` */

/*Table structure for table `mailbox_queue` */

DROP TABLE IF EXISTS `mailbox_queue`;

CREATE TABLE `mailbox_queue` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `messageType` int(10) unsigned NOT NULL DEFAULT '0',
  `stationery` int(10) unsigned NOT NULL DEFAULT '0',
  `sender_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `receiver_guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `subject` varchar(200) DEFAULT 'Support Message',
  `message` varchar(500) DEFAULT 'Support Message',
  `money` int(10) unsigned NOT NULL DEFAULT '0',
  `item` int(10) unsigned NOT NULL DEFAULT '0',
  `item_count` int(10) unsigned NOT NULL DEFAULT '0',
  `store_history` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `mailbox_queue` */

/*Table structure for table `pet_aura` */

DROP TABLE IF EXISTS `pet_aura`;

CREATE TABLE `pet_aura` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `caster_guid` binary(16) NOT NULL COMMENT 'Full Global Unique Identifier',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `effect_mask` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `recalculate_mask` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stackcount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `maxduration` int(11) NOT NULL DEFAULT '0',
  `remaintime` int(11) NOT NULL DEFAULT '0',
  `remaincharges` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`,`effect_mask`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Pet System';

/*Data for the table `pet_aura` */

/*Table structure for table `pet_aura_effect` */

DROP TABLE IF EXISTS `pet_aura_effect`;

CREATE TABLE `pet_aura_effect` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `effect` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `amount` int(11) NOT NULL DEFAULT '0',
  `baseamount` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`slot`,`effect`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `pet_aura_effect` */

/*Table structure for table `pet_spell` */

DROP TABLE IF EXISTS `pet_spell`;

CREATE TABLE `pet_spell` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `active` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE,
  KEY `guid` (`guid`) USING BTREE,
  KEY `spell` (`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Pet System';

/*Data for the table `pet_spell` */

/*Table structure for table `pet_spell_cooldown` */

DROP TABLE IF EXISTS `pet_spell_cooldown`;

CREATE TABLE `pet_spell_cooldown` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier, Low part',
  `spell` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier',
  `time` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spell`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `pet_spell_cooldown` */

/*Table structure for table `petition` */

DROP TABLE IF EXISTS `petition`;

CREATE TABLE `petition` (
  `ownerguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `petitionguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`type`) USING BTREE,
  UNIQUE KEY `index_ownerguid_petitionguid` (`ownerguid`,`petitionguid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Guild System';

/*Data for the table `petition` */

/*Table structure for table `petition_sign` */

DROP TABLE IF EXISTS `petition_sign`;

CREATE TABLE `petition_sign` (
  `ownerguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `petitionguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `playerguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `player_account` int(10) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`petitionguid`,`playerguid`) USING BTREE,
  KEY `Idx_playerguid` (`playerguid`) USING BTREE,
  KEY `Idx_ownerguid` (`ownerguid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Guild System';

/*Data for the table `petition_sign` */

/*Table structure for table `pool_quest_save` */

DROP TABLE IF EXISTS `pool_quest_save`;

CREATE TABLE `pool_quest_save` (
  `pool_id` int(10) unsigned NOT NULL DEFAULT '0',
  `quest_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`pool_id`,`quest_id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `pool_quest_save` */

/*Table structure for table `report_bugreport` */

DROP TABLE IF EXISTS `report_bugreport`;

CREATE TABLE `report_bugreport` (
  `ID` bigint(20) unsigned NOT NULL COMMENT 'Identifier',
  `PlayerGuid` bigint(20) unsigned NOT NULL,
  `Note` longtext NOT NULL,
  `MapID` smallint(5) NOT NULL,
  `PosX` float NOT NULL,
  `PosY` float NOT NULL,
  `PosZ` float NOT NULL,
  PRIMARY KEY (`ID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Bug report system';

/*Data for the table `report_bugreport` */

/*Table structure for table `report_complaints` */

DROP TABLE IF EXISTS `report_complaints`;

CREATE TABLE `report_complaints` (
  `ID` bigint(20) unsigned NOT NULL COMMENT 'Identifier',
  `ReportPlayer` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ReportAccount` int(10) unsigned NOT NULL DEFAULT '0',
  `ReportTime` int(10) unsigned NOT NULL DEFAULT '0',
  `SpammerGuid` bigint(20) unsigned NOT NULL,
  `ComplaintType` smallint(3) unsigned NOT NULL DEFAULT '0',
  `MailID` int(10) unsigned NOT NULL DEFAULT '0',
  `TimeSinceOffence` int(10) unsigned NOT NULL DEFAULT '0',
  `MessageLog` longtext NOT NULL,
  `JustBanned` int(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Spam report system';

/*Data for the table `report_complaints` */

/*Table structure for table `reserved_name` */

DROP TABLE IF EXISTS `reserved_name`;

CREATE TABLE `reserved_name` (
  `name` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`name`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Player Reserved Names';

/*Data for the table `reserved_name` */

/*Table structure for table `version` */

DROP TABLE IF EXISTS `version`;

CREATE TABLE `version` (
  `core_version` varchar(120) DEFAULT NULL COMMENT 'Core revision dumped at startup.',
  `core_revision` varchar(120) DEFAULT NULL,
  `db_version` varchar(120) DEFAULT NULL COMMENT 'Version of characters DB.'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Version Notes';

/*Data for the table `version` */

insert  into `version`(`core_version`,`core_revision`,`db_version`) values 
('LegionCore 2020-04-03 (Win64, Release)','','LegionCore Characters Database 2020-04-03');

/*Table structure for table `world_quest` */

DROP TABLE IF EXISTS `world_quest`;

CREATE TABLE `world_quest` (
  `QuestID` int(10) unsigned NOT NULL DEFAULT '0',
  `QuestInfoID` int(10) unsigned NOT NULL DEFAULT '0',
  `QuestSortID` int(10) NOT NULL DEFAULT '0',
  `VariableID` int(10) NOT NULL DEFAULT '0',
  `Value` int(10) NOT NULL DEFAULT '0',
  `Timer` int(10) NOT NULL DEFAULT '0',
  `StartTime` int(10) NOT NULL DEFAULT '0',
  `ResetTime` int(10) NOT NULL DEFAULT '0',
  `CurrencyID` int(10) NOT NULL DEFAULT '0',
  `CurrencyCount` int(10) NOT NULL DEFAULT '0',
  `Gold` int(10) NOT NULL DEFAULT '0',
  `RewardType` int(10) NOT NULL DEFAULT '0',
  `ItemList` longtext NOT NULL,
  PRIMARY KEY (`QuestID`) USING BTREE,
  KEY `ResetTime` (`ResetTime`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

/*Data for the table `world_quest` */

/*Table structure for table `worldstate_data` */

DROP TABLE IF EXISTS `worldstate_data`;

CREATE TABLE `worldstate_data` (
  `VariableID` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'WorldState ID (can be 0)',
  `InstanceID` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'WorldState instance',
  `Type` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'WorldStatesData::Types',
  `ConditionID` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Condition (dependent from type)',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Current flags see WorldStatesData::Falgs',
  `Value` int(10) unsigned NOT NULL DEFAULT '0',
  `RenewTime` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Time of last renew of WorldState',
  PRIMARY KEY (`VariableID`,`InstanceID`,`Type`,`ConditionID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='WorldState data storage';

/*Data for the table `worldstate_data` */

/*Table structure for table `worldstates` */

DROP TABLE IF EXISTS `worldstates`;

CREATE TABLE `worldstates` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `value` int(10) unsigned NOT NULL DEFAULT '0',
  `comment` tinytext,
  PRIMARY KEY (`entry`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Variable Saves';

/*Data for the table `worldstates` */

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
