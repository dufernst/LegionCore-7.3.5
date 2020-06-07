/*
SQLyog Community v13.1.5  (64 bit)
MySQL - 5.6.47 : Database - auth
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
/*Table structure for table `account` */

DROP TABLE IF EXISTS `account`;

CREATE TABLE `account` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `username` varchar(32) NOT NULL DEFAULT '',
  `sha_pass_hash` varchar(40) NOT NULL DEFAULT '',
  `sessionkey` varchar(512) NOT NULL DEFAULT '',
  `v` varchar(64) NOT NULL DEFAULT '',
  `s` varchar(64) NOT NULL DEFAULT '',
  `email` varchar(254) NOT NULL DEFAULT '',
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_ip` varchar(15) NOT NULL DEFAULT '127.0.0.1',
  `failed_logins` int(10) unsigned NOT NULL DEFAULT '0',
  `locked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `lock_country` varchar(2) NOT NULL DEFAULT '00',
  `last_login` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `expansion` tinyint(3) unsigned NOT NULL DEFAULT '6',
  `mutetime` bigint(20) NOT NULL DEFAULT '0',
  `locale` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `os` varchar(10) NOT NULL DEFAULT '',
  `recruiter` int(10) unsigned NOT NULL DEFAULT '0',
  `battlenet_account` int(10) unsigned DEFAULT NULL,
  `battlenet_index` tinyint(3) unsigned DEFAULT NULL,
  `mutereason` varchar(255) NOT NULL DEFAULT '',
  `muteby` varchar(50) NOT NULL DEFAULT '',
  `AtAuthFlag` smallint(3) unsigned NOT NULL DEFAULT '0',
  `coins` int(11) NOT NULL DEFAULT '0',
  `hwid` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `username` (`username`) USING BTREE,
  UNIQUE KEY `bnet_acc` (`battlenet_account`,`battlenet_index`) USING BTREE,
  KEY `recruiter` (`recruiter`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `battlenet_account` (`battlenet_account`) USING BTREE,
  KEY `battlenet_index` (`battlenet_index`) USING BTREE,
  KEY `username_idx` (`username`) USING BTREE,
  KEY `hwid` (`hwid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Account System';

/*Data for the table `account` */

insert  into `account`(`id`,`username`,`sha_pass_hash`,`sessionkey`,`v`,`s`,`email`,`joindate`,`last_ip`,`failed_logins`,`locked`,`lock_country`,`last_login`,`online`,`expansion`,`mutetime`,`locale`,`os`,`recruiter`,`battlenet_account`,`battlenet_index`,`mutereason`,`muteby`,`AtAuthFlag`,`coins`,`hwid`) values 
(1,'GM@GM','586EF64D6BCF71292B55C8805E465172D876E0C7','','','','','2020-03-30 12:36:05','127.0.0.1',0,0,'00','0000-00-00 00:00:00',0,6,0,0,'',0,1,NULL,'','',0,0,0);

/*Table structure for table `account_access` */

DROP TABLE IF EXISTS `account_access`;

CREATE TABLE `account_access` (
  `id` int(10) unsigned NOT NULL,
  `gmlevel` tinyint(3) unsigned NOT NULL,
  `RealmID` int(11) NOT NULL DEFAULT '-1',
  `comments` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`,`RealmID`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `RealmID` (`RealmID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_access` */

insert  into `account_access`(`id`,`gmlevel`,`RealmID`,`comments`) values 
(1,6,-1,'');

/*Table structure for table `account_banned` */

DROP TABLE IF EXISTS `account_banned`;

CREATE TABLE `account_banned` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account id',
  `bandate` int(10) unsigned NOT NULL DEFAULT '0',
  `unbandate` int(10) unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  `active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`bandate`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `bandate` (`bandate`) USING BTREE,
  KEY `unbandate` (`unbandate`) USING BTREE,
  KEY `active` (`active`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Ban List';

/*Data for the table `account_banned` */

/*Table structure for table `account_character_template` */

DROP TABLE IF EXISTS `account_character_template`;

CREATE TABLE `account_character_template` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `account` int(10) NOT NULL DEFAULT '0',
  `bnet_account` int(10) NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '100',
  `iLevel` mediumint(6) NOT NULL DEFAULT '810',
  `money` int(10) unsigned NOT NULL DEFAULT '100',
  `artifact` tinyint(1) NOT NULL DEFAULT '0',
  `transferId` int(10) NOT NULL DEFAULT '0',
  `charGuid` int(10) NOT NULL DEFAULT '0',
  `realm` int(10) NOT NULL DEFAULT '0',
  `templateId` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `bnet_account` (`bnet_account`) USING BTREE,
  KEY `transferId` (`transferId`) USING BTREE,
  KEY `charGuid` (`charGuid`) USING BTREE,
  KEY `realm` (`realm`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_character_template` */

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

/*Table structure for table `account_ip_access` */

DROP TABLE IF EXISTS `account_ip_access`;

CREATE TABLE `account_ip_access` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `pid` int(11) unsigned DEFAULT NULL,
  `ip` varchar(18) DEFAULT NULL,
  `min` varchar(15) NOT NULL DEFAULT '',
  `max` varchar(15) NOT NULL DEFAULT '',
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `pid_ip` (`pid`,`ip`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_ip_access` */

/*Table structure for table `account_last_played_character` */

DROP TABLE IF EXISTS `account_last_played_character`;

CREATE TABLE `account_last_played_character` (
  `accountId` int(10) unsigned NOT NULL,
  `region` tinyint(3) unsigned NOT NULL,
  `battlegroup` tinyint(3) unsigned NOT NULL,
  `realmId` int(10) unsigned DEFAULT NULL,
  `characterName` varchar(12) DEFAULT NULL,
  `characterGUID` bigint(20) unsigned DEFAULT NULL,
  `lastPlayedTime` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`accountId`,`region`,`battlegroup`) USING BTREE,
  KEY `accountId` (`accountId`) USING BTREE,
  KEY `region` (`region`) USING BTREE,
  KEY `battlegroup` (`battlegroup`) USING BTREE,
  KEY `realmId` (`realmId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_last_played_character` */

/*Table structure for table `account_mute` */

DROP TABLE IF EXISTS `account_mute`;

CREATE TABLE `account_mute` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier',
  `mutedate` int(10) unsigned NOT NULL DEFAULT '0',
  `mutetime` int(10) unsigned NOT NULL DEFAULT '0',
  `mutedby` varchar(50) NOT NULL,
  `mutereason` varchar(255) NOT NULL,
  PRIMARY KEY (`guid`,`mutedate`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='mute List';

/*Data for the table `account_mute` */

/*Table structure for table `account_muted` */

DROP TABLE IF EXISTS `account_muted`;

CREATE TABLE `account_muted` (
  `id` int(11) NOT NULL DEFAULT '0' COMMENT 'Account id',
  `bandate` bigint(40) NOT NULL DEFAULT '0',
  `unbandate` bigint(40) NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  `active` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`bandate`) USING BTREE,
  KEY `bandate` (`bandate`) USING BTREE,
  KEY `unbandate` (`unbandate`) USING BTREE,
  KEY `active` (`active`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Ban List';

/*Data for the table `account_muted` */

/*Table structure for table `account_rates` */

DROP TABLE IF EXISTS `account_rates`;

CREATE TABLE `account_rates` (
  `account` int(11) NOT NULL DEFAULT '0',
  `bnet_account` int(11) unsigned NOT NULL DEFAULT '0',
  `realm` int(11) unsigned NOT NULL DEFAULT '0',
  `rate` int(11) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `unique` (`account`,`bnet_account`,`realm`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `account_rates` */

/*Table structure for table `battlenet_account_bans` */

DROP TABLE IF EXISTS `battlenet_account_bans`;

CREATE TABLE `battlenet_account_bans` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account id',
  `bandate` int(10) unsigned NOT NULL DEFAULT '0',
  `unbandate` int(10) unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  `active` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`bandate`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `bandate` (`bandate`) USING BTREE,
  KEY `unbandate` (`unbandate`) USING BTREE,
  KEY `active` (`active`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Ban List';

/*Data for the table `battlenet_account_bans` */

/*Table structure for table `battlenet_accounts` */

DROP TABLE IF EXISTS `battlenet_accounts`;

CREATE TABLE `battlenet_accounts` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `email` varchar(255) NOT NULL,
  `email_blocked` int(11) unsigned NOT NULL DEFAULT '0',
  `sha_pass_hash` varchar(512) NOT NULL DEFAULT '',
  `balans` int(10) unsigned NOT NULL DEFAULT '0',
  `karma` int(10) unsigned NOT NULL DEFAULT '0',
  `activate` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `verify` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `tested` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `donate` int(10) unsigned NOT NULL DEFAULT '0',
  `phone` varchar(255) NOT NULL DEFAULT '',
  `phone_hash` varchar(32) NOT NULL DEFAULT '',
  `telegram_lock` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `telegram_id` int(11) unsigned NOT NULL DEFAULT '0',
  `v` varchar(512) NOT NULL DEFAULT '',
  `s` varchar(512) NOT NULL DEFAULT '',
  `sessionKey` varchar(512) NOT NULL DEFAULT '',
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_ip` varchar(15) NOT NULL DEFAULT '127.0.0.1',
  `access_ip` int(10) unsigned NOT NULL DEFAULT '0',
  `failed_logins` int(10) unsigned NOT NULL DEFAULT '0',
  `locked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `lock_country` varchar(2) NOT NULL DEFAULT '00',
  `last_login` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `last_email` timestamp NULL DEFAULT NULL,
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `locale` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `os` varchar(10) NOT NULL DEFAULT '',
  `recruiter` int(11) NOT NULL DEFAULT '0',
  `invite` varchar(32) NOT NULL DEFAULT '',
  `lang` enum('tw','cn','en','ua','ru') NOT NULL DEFAULT 'en',
  `referer` varchar(255) NOT NULL DEFAULT '',
  `unsubscribe` varchar(32) NOT NULL DEFAULT '0',
  `dt_vote` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `email` (`email`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `recruiter` (`recruiter`) USING BTREE,
  KEY `email_idx` (`email`) USING BTREE,
  KEY `sha_pass_hash` (`sha_pass_hash`(255)) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Account System';

/*Data for the table `battlenet_accounts` */

insert  into `battlenet_accounts`(`id`,`email`,`email_blocked`,`sha_pass_hash`,`balans`,`karma`,`activate`,`verify`,`tested`,`donate`,`phone`,`phone_hash`,`telegram_lock`,`telegram_id`,`v`,`s`,`sessionKey`,`joindate`,`last_ip`,`access_ip`,`failed_logins`,`locked`,`lock_country`,`last_login`,`last_email`,`online`,`locale`,`os`,`recruiter`,`invite`,`lang`,`referer`,`unsubscribe`,`dt_vote`) values 
(1,'GM@GM',0,'586EF64D6BCF71292B55C8805E465172D876E0C79968F925A0DCE5D9B4BAA492',0,0,1,0,0,0,'','',0,0,'','','','2020-03-30 12:36:05','127.0.0.1',0,0,0,'00','0000-00-00 00:00:00',NULL,0,0,'',0,'','en','','0',NULL);

/*Table structure for table `build_info` */

DROP TABLE IF EXISTS `build_info`;

CREATE TABLE `build_info` (
  `build` int(11) NOT NULL,
  `majorVersion` int(11) DEFAULT NULL,
  `minorVersion` int(11) DEFAULT NULL,
  `bugfixVersion` int(11) DEFAULT NULL,
  `hotfixVersion` char(3) DEFAULT NULL,
  `winAuthSeed` varchar(32) DEFAULT NULL,
  `win64AuthSeed` varchar(32) DEFAULT NULL,
  `mac64AuthSeed` varchar(32) DEFAULT NULL,
  `winChecksumSeed` varchar(40) DEFAULT NULL,
  `macChecksumSeed` varchar(40) DEFAULT NULL,
  PRIMARY KEY (`build`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

/*Data for the table `build_info` */

insert  into `build_info`(`build`,`majorVersion`,`minorVersion`,`bugfixVersion`,`hotfixVersion`,`winAuthSeed`,`win64AuthSeed`,`mac64AuthSeed`,`winChecksumSeed`,`macChecksumSeed`) values 
(5875,1,12,1,NULL,NULL,NULL,NULL,'95EDB27C7823B363CBDDAB56A392E7CB73FCCA20','8D173CC381961EEBABF336F5E6675B101BB513E5'),
(6005,1,12,2,NULL,NULL,NULL,NULL,NULL,NULL),
(6141,1,12,3,NULL,NULL,NULL,NULL,NULL,NULL),
(8606,2,4,3,NULL,NULL,NULL,NULL,'319AFAA3F2559682F9FF658BE01456255F456FB1','D8B0ECFE534BC1131E19BAD1D4C0E813EEE4994F'),
(9947,3,1,3,NULL,NULL,NULL,NULL,NULL,NULL),
(10505,3,2,2,'a',NULL,NULL,NULL,NULL,NULL),
(11159,3,3,0,'a',NULL,NULL,NULL,NULL,NULL),
(11403,3,3,2,NULL,NULL,NULL,NULL,NULL,NULL),
(11723,3,3,3,'a',NULL,NULL,NULL,NULL,NULL),
(12340,3,3,5,'a',NULL,NULL,NULL,'CDCBBD5188315E6B4D19449D492DBCFAF156A347','B706D13FF2F4018839729461E3F8A0E2B5FDC034'),
(13623,4,0,6,'a',NULL,NULL,NULL,NULL,NULL),
(13930,3,3,5,'a',NULL,NULL,NULL,NULL,NULL),
(14545,4,2,2,NULL,NULL,NULL,NULL,NULL,NULL),
(15595,4,3,4,NULL,NULL,NULL,NULL,NULL,NULL),
(19116,6,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(19243,6,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(19342,6,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(19702,6,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(19802,6,1,2,NULL,NULL,NULL,NULL,NULL,NULL),
(19831,6,1,2,NULL,NULL,NULL,NULL,NULL,NULL),
(19865,6,1,2,NULL,NULL,NULL,NULL,NULL,NULL),
(20182,6,2,0,'a',NULL,NULL,NULL,NULL,NULL),
(20201,6,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(20216,6,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(20253,6,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(20338,6,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(20444,6,2,2,NULL,NULL,NULL,NULL,NULL,NULL),
(20490,6,2,2,'a',NULL,NULL,NULL,NULL,NULL),
(20574,6,2,2,'a',NULL,NULL,NULL,NULL,NULL),
(20726,6,2,3,NULL,NULL,NULL,NULL,NULL,NULL),
(20779,6,2,3,NULL,NULL,NULL,NULL,NULL,NULL),
(20886,6,2,3,NULL,NULL,NULL,NULL,NULL,NULL),
(21355,6,2,4,NULL,NULL,NULL,NULL,NULL,NULL),
(21463,6,2,4,NULL,NULL,NULL,NULL,NULL,NULL),
(21742,6,2,4,NULL,NULL,NULL,NULL,NULL,NULL),
(22248,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22293,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22345,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22410,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22423,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22498,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22522,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22566,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22594,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22624,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22747,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22810,7,0,3,NULL,NULL,NULL,NULL,NULL,NULL),
(22900,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(22908,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(22950,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(22995,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(22996,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(23171,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(23222,7,1,0,NULL,NULL,NULL,NULL,NULL,NULL),
(23360,7,1,5,NULL,NULL,NULL,NULL,NULL,NULL),
(23420,7,1,5,NULL,NULL,NULL,NULL,NULL,NULL),
(23911,7,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(23937,7,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(24015,7,2,0,NULL,NULL,NULL,NULL,NULL,NULL),
(24330,7,2,5,NULL,NULL,NULL,NULL,NULL,NULL),
(24367,7,2,5,NULL,NULL,NULL,NULL,NULL,NULL),
(24415,7,2,5,NULL,NULL,NULL,NULL,NULL,NULL),
(24430,7,2,5,NULL,NULL,NULL,NULL,NULL,NULL),
(24461,7,2,5,NULL,NULL,NULL,NULL,NULL,NULL),
(24742,7,2,5,NULL,NULL,NULL,NULL,NULL,NULL),
(25549,7,3,2,NULL,'FE594FC35E7F9AFF86D99D8A364AB297','1252624ED8CBD6FAC7D33F5D67A535F3','66FC5E09B8706126795F140308C8C1D8',NULL,NULL),
(25996,7,3,5,NULL,'23C59C5963CBEF5B728D13A50878DFCB','C7FF932D6A2174A3D538CA7212136D2B','210B970149D6F56CAC9BADF2AAC91E8E',NULL,NULL),
(26124,7,3,5,NULL,'F8C05AE372DECA1D6C81DA7A8D1C5C39','46DF06D0147BA67BA49AF553435E093F','C9CA997AB8EDE1C65465CB2920869C4E',NULL,NULL),
(26365,7,3,5,NULL,'2AAC82C80E829E2CA902D70CFA1A833A','59A53F307288454B419B13E694DF503C','DBE7F860276D6B400AAA86B35D51A417',NULL,NULL),
(26654,7,3,5,NULL,'FAC2D693E702B9EC9F750F17245696D8','A752640E8B99FE5B57C1320BC492895A','9234C1BD5E9687ADBD19F764F2E0E811',NULL,NULL),
(26822,7,3,5,NULL,'283E8D77ECF7060BE6347BE4EB99C7C7','2B05F6D746C0C6CC7EF79450B309E595','91003668C245D14ECD8DF094E065E06B',NULL,NULL),
(26899,7,3,5,NULL,'F462CD2FE4EA3EADF875308FDBB18C99','3551EF0028B51E92170559BD25644B03','8368EFC2021329110A16339D298200D4',NULL,NULL),
(26972,7,3,5,NULL,'797ECC19662DCBD5090A4481173F1D26','6E212DEF6A0124A3D9AD07F5E322F7AE','341CFEFE3D72ACA9A4407DC535DED66A',NULL,NULL);

/*Table structure for table `hwid_penalties` */

DROP TABLE IF EXISTS `hwid_penalties`;

CREATE TABLE `hwid_penalties` (
  `hwid` bigint(20) unsigned NOT NULL,
  `penalties` int(10) NOT NULL DEFAULT '0',
  `last_reason` varchar(255) NOT NULL,
  PRIMARY KEY (`hwid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `hwid_penalties` */

/*Table structure for table `ip_banned` */

DROP TABLE IF EXISTS `ip_banned`;

CREATE TABLE `ip_banned` (
  `ip` varchar(32) NOT NULL DEFAULT '127.0.0.1',
  `bandate` bigint(40) NOT NULL,
  `unbandate` bigint(40) NOT NULL,
  `bannedby` varchar(50) NOT NULL DEFAULT '[Console]',
  `banreason` varchar(255) NOT NULL DEFAULT 'no reason',
  PRIMARY KEY (`ip`,`bandate`) USING BTREE,
  KEY `ip` (`ip`) USING BTREE,
  KEY `bandate` (`bandate`) USING BTREE,
  KEY `unbandate` (`unbandate`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Banned IPs';

/*Data for the table `ip_banned` */

/*Table structure for table `logs` */

DROP TABLE IF EXISTS `logs`;

CREATE TABLE `logs` (
  `time` int(14) NOT NULL,
  `realm` int(4) NOT NULL,
  `type` int(4) NOT NULL,
  `level` int(11) NOT NULL DEFAULT '0',
  `string` text,
  KEY `time` (`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

/*Data for the table `logs` */

/*Table structure for table `online` */

DROP TABLE IF EXISTS `online`;

CREATE TABLE `online` (
  `realmID` int(11) unsigned NOT NULL DEFAULT '0',
  `online` int(11) unsigned NOT NULL DEFAULT '0',
  `diff` int(11) unsigned NOT NULL DEFAULT '0',
  `uptime` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmID`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `online` */

/*Table structure for table `realmcharacters` */

DROP TABLE IF EXISTS `realmcharacters`;

CREATE TABLE `realmcharacters` (
  `realmid` int(10) unsigned NOT NULL DEFAULT '0',
  `acctid` int(10) unsigned NOT NULL,
  `numchars` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmid`,`acctid`) USING BTREE,
  KEY `acctid` (`acctid`) USING BTREE,
  KEY `realmid` (`realmid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Realm Character Tracker';

/*Data for the table `realmcharacters` */

/*Table structure for table `realmlist` */

DROP TABLE IF EXISTS `realmlist`;

CREATE TABLE `realmlist` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL DEFAULT '',
  `address` varchar(255) NOT NULL DEFAULT '127.0.0.1',
  `port` smallint(5) unsigned NOT NULL DEFAULT '8085',
  `gamePort` int(11) NOT NULL DEFAULT '8086',
  `portCount` mediumint(4) unsigned NOT NULL DEFAULT '1',
  `icon` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `flag` tinyint(3) unsigned NOT NULL DEFAULT '2',
  `timezone` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `allowedSecurityLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `population` float unsigned NOT NULL DEFAULT '0',
  `gamebuild` int(10) unsigned NOT NULL DEFAULT '26972',
  `Region` tinyint(3) unsigned NOT NULL DEFAULT '2',
  `Battlegroup` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `localAddress` varchar(255) NOT NULL DEFAULT '127.0.0.1',
  `localSubnetMask` varchar(255) NOT NULL DEFAULT '255.255.255.0',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `idx_name` (`name`) USING BTREE,
  KEY `id` (`id`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Realm System';

/*Data for the table `realmlist` */

insert  into `realmlist`(`id`,`name`,`address`,`port`,`gamePort`,`portCount`,`icon`,`flag`,`timezone`,`allowedSecurityLevel`,`population`,`gamebuild`,`Region`,`Battlegroup`,`localAddress`,`localSubnetMask`) values 
(1,'LegionCore','127.0.0.1',8085,8086,1,0,2,1,0,0,26972,2,1,'127.0.0.1','255.255.255.0');

/*Table structure for table `store_categories` */

DROP TABLE IF EXISTS `store_categories`;

CREATE TABLE `store_categories` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pid` int(11) unsigned NOT NULL,
  `type` smallint(10) NOT NULL DEFAULT '0',
  `sort` int(11) unsigned NOT NULL DEFAULT '0',
  `faction` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `expansion` tinyint(1) unsigned NOT NULL DEFAULT '6',
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `enable` (`enable`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `sort` (`sort`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_categories` */

/*Table structure for table `store_category_locales` */

DROP TABLE IF EXISTS `store_category_locales`;

CREATE TABLE `store_category_locales` (
  `category` int(11) NOT NULL DEFAULT '0',
  `name_us` varchar(255) NOT NULL DEFAULT '',
  `name_gb` varchar(255) NOT NULL DEFAULT '',
  `name_kr` varchar(255) NOT NULL DEFAULT '',
  `name_fr` varchar(255) NOT NULL DEFAULT '',
  `name_de` varchar(255) NOT NULL DEFAULT '',
  `name_cn` varchar(255) NOT NULL DEFAULT '',
  `name_tw` varchar(255) NOT NULL DEFAULT '',
  `name_es` varchar(255) NOT NULL DEFAULT '',
  `name_mx` varchar(255) NOT NULL DEFAULT '',
  `name_ru` varchar(255) NOT NULL DEFAULT '',
  `name_pt` varchar(255) NOT NULL DEFAULT '',
  `name_br` varchar(255) NOT NULL DEFAULT '',
  `name_it` varchar(255) NOT NULL DEFAULT '',
  `name_ua` varchar(255) NOT NULL DEFAULT '',
  `description_us` varchar(255) NOT NULL DEFAULT '',
  `description_gb` varchar(255) NOT NULL DEFAULT '',
  `description_kr` varchar(255) NOT NULL DEFAULT '',
  `description_fr` varchar(255) NOT NULL DEFAULT '',
  `description_de` varchar(255) NOT NULL DEFAULT '',
  `description_cn` varchar(255) NOT NULL DEFAULT '',
  `description_tw` varchar(255) NOT NULL DEFAULT '',
  `description_es` varchar(255) NOT NULL DEFAULT '',
  `description_mx` varchar(255) NOT NULL DEFAULT '',
  `description_ru` varchar(255) NOT NULL DEFAULT '',
  `description_pt` varchar(255) NOT NULL DEFAULT '',
  `description_br` varchar(255) NOT NULL DEFAULT '',
  `description_it` varchar(255) NOT NULL DEFAULT '',
  `description_ua` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`category`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_category_locales` */

/*Table structure for table `store_category_realms` */

DROP TABLE IF EXISTS `store_category_realms`;

CREATE TABLE `store_category_realms` (
  `category` int(11) NOT NULL DEFAULT '0',
  `realm` int(11) unsigned NOT NULL DEFAULT '0',
  `return` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '1',
  UNIQUE KEY `unique` (`category`,`realm`) USING BTREE,
  KEY `category` (`category`) USING BTREE,
  KEY `realm` (`realm`) USING BTREE,
  KEY `enable` (`enable`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_category_realms` */

/*Table structure for table `store_discounts` */

DROP TABLE IF EXISTS `store_discounts`;

CREATE TABLE `store_discounts` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `realm` int(11) unsigned NOT NULL DEFAULT '0',
  `category` int(11) NOT NULL DEFAULT '0',
  `product` int(11) NOT NULL DEFAULT '0',
  `start` timestamp NULL DEFAULT NULL,
  `end` timestamp NULL DEFAULT NULL,
  `rate` float(5,2) unsigned NOT NULL DEFAULT '0.00',
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_discounts` */

/*Table structure for table `store_history` */

DROP TABLE IF EXISTS `store_history`;

CREATE TABLE `store_history` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `realm` int(11) unsigned NOT NULL,
  `account` int(11) unsigned NOT NULL,
  `bnet_account` int(11) unsigned NOT NULL DEFAULT '0',
  `char_guid` int(11) unsigned NOT NULL DEFAULT '0',
  `char_level` int(11) unsigned NOT NULL DEFAULT '0',
  `art_level` varchar(255) NOT NULL DEFAULT '',
  `guild_name` varchar(255) NOT NULL DEFAULT '',
  `item_guid` int(11) unsigned DEFAULT NULL,
  `item` int(11) NOT NULL DEFAULT '0',
  `bonus` varchar(11) DEFAULT NULL,
  `product` int(11) NOT NULL DEFAULT '0',
  `count` int(11) unsigned NOT NULL DEFAULT '1',
  `token` int(11) unsigned NOT NULL,
  `karma` int(1) unsigned NOT NULL DEFAULT '0',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `type` enum('cp','game') NOT NULL DEFAULT 'game',
  `trans_project` varchar(255) NOT NULL DEFAULT '',
  `trans_realm` int(11) unsigned NOT NULL DEFAULT '0',
  `dt_buy` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `dt_return` timestamp NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `item_guid` (`item_guid`) USING BTREE,
  KEY `realm` (`realm`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `bnet_account` (`bnet_account`) USING BTREE,
  KEY `status` (`status`) USING BTREE,
  KEY `char_guid` (`char_guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_history` */

/*Table structure for table `store_level_prices` */

DROP TABLE IF EXISTS `store_level_prices`;

CREATE TABLE `store_level_prices` (
  `type` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `realm` int(11) unsigned NOT NULL DEFAULT '0',
  `level` smallint(4) unsigned NOT NULL DEFAULT '0',
  `token` int(11) unsigned NOT NULL DEFAULT '0',
  `karma` int(11) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `unique` (`type`,`realm`,`level`,`token`) USING BTREE,
  KEY `type` (`type`) USING BTREE,
  KEY `realm` (`realm`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_level_prices` */

/*Table structure for table `store_product_locales` */

DROP TABLE IF EXISTS `store_product_locales`;

CREATE TABLE `store_product_locales` (
  `product` int(11) NOT NULL DEFAULT '0',
  `type` smallint(10) NOT NULL DEFAULT '0',
  `us` varchar(255) NOT NULL DEFAULT '',
  `gb` varchar(255) NOT NULL DEFAULT '',
  `kr` varchar(255) NOT NULL DEFAULT '',
  `fr` varchar(255) NOT NULL DEFAULT '',
  `de` varchar(255) NOT NULL DEFAULT '',
  `cn` varchar(255) NOT NULL DEFAULT '',
  `tw` varchar(255) NOT NULL DEFAULT '',
  `es` varchar(255) NOT NULL DEFAULT '',
  `mx` varchar(255) NOT NULL DEFAULT '',
  `ru` varchar(255) NOT NULL DEFAULT '',
  `pt` varchar(255) NOT NULL DEFAULT '',
  `br` varchar(255) NOT NULL DEFAULT '',
  `it` varchar(255) NOT NULL DEFAULT '',
  `ua` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`product`,`type`) USING BTREE,
  UNIQUE KEY `unique` (`product`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_product_locales` */

/*Table structure for table `store_product_realms` */

DROP TABLE IF EXISTS `store_product_realms`;

CREATE TABLE `store_product_realms` (
  `product` int(11) NOT NULL DEFAULT '0',
  `realm` int(11) unsigned NOT NULL DEFAULT '0',
  `token` int(11) unsigned NOT NULL DEFAULT '0',
  `karma` int(11) unsigned NOT NULL DEFAULT '0',
  `return` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE KEY `unique` (`realm`,`product`) USING BTREE,
  KEY `product` (`product`) USING BTREE,
  KEY `realm` (`realm`) USING BTREE,
  KEY `enable` (`enable`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_product_realms` */

/*Table structure for table `store_products` */

DROP TABLE IF EXISTS `store_products`;

CREATE TABLE `store_products` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `category` int(11) NOT NULL DEFAULT '0',
  `item` int(11) NOT NULL DEFAULT '0',
  `bonus` varchar(255) NOT NULL DEFAULT '',
  `icon` varchar(255) NOT NULL DEFAULT '',
  `quality` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `display` int(11) unsigned NOT NULL DEFAULT '0',
  `slot` int(11) unsigned NOT NULL DEFAULT '0',
  `type` int(11) unsigned NOT NULL DEFAULT '0',
  `token` int(11) unsigned NOT NULL DEFAULT '0',
  `karma` int(11) unsigned NOT NULL DEFAULT '0',
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `dt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `faction` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `unique` (`category`,`item`,`bonus`) USING BTREE,
  KEY `id` (`id`) USING BTREE,
  KEY `category` (`category`) USING BTREE,
  KEY `enable` (`enable`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_products` */

/*Table structure for table `store_statistics` */

DROP TABLE IF EXISTS `store_statistics`;

CREATE TABLE `store_statistics` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `product` int(11) NOT NULL DEFAULT '0',
  `realm` int(11) unsigned NOT NULL DEFAULT '0',
  `rating_count` int(11) unsigned NOT NULL DEFAULT '0',
  `rating_value` int(11) unsigned NOT NULL DEFAULT '0',
  `buy` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE KEY `unique` (`realm`,`product`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `store_statistics` */

/*Table structure for table `transfer_requests` */

DROP TABLE IF EXISTS `transfer_requests`;

CREATE TABLE `transfer_requests` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `acid` int(11) unsigned NOT NULL,
  `bacid` int(11) unsigned NOT NULL DEFAULT '0',
  `user_name` varchar(32) NOT NULL DEFAULT '',
  `email` varchar(64) NOT NULL DEFAULT '',
  `guid` int(11) unsigned DEFAULT NULL,
  `char_faction` tinyint(1) unsigned DEFAULT NULL,
  `char_class` tinyint(3) unsigned DEFAULT NULL,
  `char_set` int(11) unsigned DEFAULT NULL,
  `realm` tinyint(3) unsigned NOT NULL,
  `dump` mediumtext,
  `promo_code` varchar(32) DEFAULT '',
  `client_expansion` tinyint(1) unsigned DEFAULT NULL,
  `client_build` smallint(5) unsigned DEFAULT NULL,
  `client_locale` varchar(4) DEFAULT '',
  `site` varchar(32) NOT NULL DEFAULT '',
  `realmlist` varchar(32) NOT NULL DEFAULT '',
  `transfer_user_name` varchar(32) NOT NULL DEFAULT '',
  `password` varchar(255) NOT NULL DEFAULT '',
  `transfer_realm` varchar(32) NOT NULL DEFAULT '',
  `char_name` varchar(12) NOT NULL DEFAULT '',
  `dump_version` varchar(255) DEFAULT '',
  `dt_create` timestamp NULL DEFAULT NULL,
  `dt_update` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `moderator` int(11) unsigned DEFAULT NULL,
  `comment` varchar(255) DEFAULT '',
  `cost` int(11) unsigned NOT NULL DEFAULT '0',
  `type` enum('fee','free') NOT NULL DEFAULT 'free',
  `test` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `status` enum('check','test','paid','cancel','4','2','0','reject','payment','verify','new') NOT NULL DEFAULT 'new',
  `parser` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `transfer_requests` */

/*Table structure for table `transferts` */

DROP TABLE IF EXISTS `transferts`;

CREATE TABLE `transferts` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `account` int(11) NOT NULL DEFAULT '0',
  `perso_guid` int(11) NOT NULL DEFAULT '0',
  `from` int(11) NOT NULL DEFAULT '0',
  `to` int(11) NOT NULL DEFAULT '0',
  `toacc` int(11) NOT NULL DEFAULT '0',
  `dump` longtext NOT NULL,
  `nb_attempt` int(11) NOT NULL DEFAULT '0',
  `state` int(10) DEFAULT '0',
  `error` int(10) DEFAULT '0',
  `revision` int(10) DEFAULT '0',
  `transferId` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `account` (`account`) USING BTREE,
  KEY `perso_guid` (`perso_guid`) USING BTREE,
  KEY `from` (`from`) USING BTREE,
  KEY `to` (`to`) USING BTREE,
  KEY `state` (`state`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `transferts` */

/*Table structure for table `transferts_logs` */

DROP TABLE IF EXISTS `transferts_logs`;

CREATE TABLE `transferts_logs` (
  `id` int(11) DEFAULT NULL,
  `account` int(11) DEFAULT '0',
  `perso_guid` int(11) DEFAULT '0',
  `from` int(2) DEFAULT '0',
  `to` int(2) DEFAULT '0',
  `dump` longtext,
  `toacc` int(11) NOT NULL DEFAULT '0',
  `newguid` int(11) NOT NULL DEFAULT '0',
  `transferId` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

/*Data for the table `transferts_logs` */

/*Table structure for table `uptime` */

DROP TABLE IF EXISTS `uptime`;

CREATE TABLE `uptime` (
  `realmid` int(11) unsigned NOT NULL,
  `starttime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startstring` varchar(255) DEFAULT NULL,
  `uptime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `maxplayers` smallint(5) unsigned NOT NULL DEFAULT '0',
  `revision` varchar(255) NOT NULL DEFAULT 'LegionCore',
  PRIMARY KEY (`realmid`,`starttime`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT COMMENT='Uptime system';

/*Data for the table `uptime` */

/*Table structure for table `version` */

DROP TABLE IF EXISTS `version`;

CREATE TABLE `version` (
  `core_version` varchar(120) DEFAULT NULL COMMENT 'Core revision dumped at startup.',
  `core_revision` varchar(120) DEFAULT NULL,
  `db_version` varchar(120) DEFAULT NULL COMMENT 'Version of auth DB.'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Version Notes';

/*Data for the table `version` */

insert  into `version`(`core_version`,`core_revision`,`db_version`) values 
('LegionCore 2020-04-03 (Win64, Release)','','LegionCore Auth Database 2020-04-03');

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
