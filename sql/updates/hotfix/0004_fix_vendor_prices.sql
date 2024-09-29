-- First fix bad database structure
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat1` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat2` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat3` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat4` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat5` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat6` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat7` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat8` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat9` SMALLINT SIGNED NOT NULL default '0';
ALTER TABLE `item_sparse` MODIFY `StatModifierBonusStat10` SMALLINT SIGNED NOT NULL default '0';

-- Fix incorrect vendor item faction requirements
-- 67525 = Bilgewater Satchel (Horde)
-- 67526 = Darnassus Satchel (Alliance)
-- 67528 = Ironforge Satchel (Alliance)
-- 67529 = Undercity Satchel (Horde)
-- 67530 = Gnomeregan Satchel (Alliance)
-- 67531 = Stormwind Satchel (Alliance)
-- 67532 = Gilnean Satchel (Alliance)
-- 67533 = Orgrimmar Satchel (Horde)
-- 67535 = Silvermoon Satchel (Horde)
-- 67536 = Darkspear Satchel (Horde)
-- 92070 = Huojin Satchel (Horde)
-- 92071 = Tushui Satchel (Alliance)
DELETE FROM `item_sparse` WHERE `ID` IN (67525, 67526, 67528, 67529, 67530, 67531, 67532, 67533, 67535, 67536, 92070, 92071);
INSERT INTO `item_sparse` (`ID`, `AllowableRace`, `Display`, `Display1`, `Display2`, `Display3`, `Description`, `Flags1`, `Flags2`, `Flags3`, `Flags4`, `PriceRandomValue`, `PriceVariance`, `VendorStackCount`, `BuyPrice`, `SellPrice`, `RequiredAbility`, `MaxCount`, `Stackable`, `StatPercentEditor1`, `StatPercentEditor2`, `StatPercentEditor3`, `StatPercentEditor4`, `StatPercentEditor5`, `StatPercentEditor6`, `StatPercentEditor7`, `StatPercentEditor8`, `StatPercentEditor9`, `StatPercentEditor10`, `StatPercentageOfSocket1`, `StatPercentageOfSocket2`, `StatPercentageOfSocket3`, `StatPercentageOfSocket4`, `StatPercentageOfSocket5`, `StatPercentageOfSocket6`, `StatPercentageOfSocket7`, `StatPercentageOfSocket8`, `StatPercentageOfSocket9`, `StatPercentageOfSocket10`, `ItemRange`, `BagFamily`, `QualityModifier`, `DurationInInventory`, `DmgVariance`, `AllowableClass`, `ItemLevel`, `RequiredSkill`, `RequiredSkillRank`, `MinFactionID`, `ItemStatValue1`, `ItemStatValue2`, `ItemStatValue3`, `ItemStatValue4`, `ItemStatValue5`, `ItemStatValue6`, `ItemStatValue7`, `ItemStatValue8`, `ItemStatValue9`, `ItemStatValue10`, `ScalingStatDistributionID`, `ItemDelay`, `PageID`, `StartQuestID`, `LockID`, `RandomSelect`, `ItemRandomSuffixGroupID`, `ItemSet`, `ZoneBound`, `InstanceBound`, `TotemCategoryID`, `SocketMatch_enchantment_id`, `GemProperties`, `LimitCategory`, `RequiredHoliday`, `RequiredTransmogHoliday`, `ItemNameDescriptionID`, `OverallQualityID`, `InventoryType`, `RequiredLevel`, `RequiredPVPRank`, `RequiredPVPMedal`, `MinReputation`, `ContainerSlots`, `StatModifierBonusStat1`, `StatModifierBonusStat2`, `StatModifierBonusStat3`, `StatModifierBonusStat4`, `StatModifierBonusStat5`, `StatModifierBonusStat6`, `StatModifierBonusStat7`, `StatModifierBonusStat8`, `StatModifierBonusStat9`, `StatModifierBonusStat10`, `DamageDamageType`, `Bonding`, `LanguageID`, `PageMaterialID`, `Material`, `SheatheType`, `SocketType1`, `SocketType2`, `SocketType3`, `SpellWeightCategory`, `SpellWeight`, `ArtifactID`, `ExpansionID`, `VerifiedBuild`) VALUES
(67525, -1, 'Bilgewater Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9705, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 1133, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67526, -1, 'Darnassus Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9742, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67528, -1, 'Ironforge Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9815, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67529, -1, 'Undercity Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9852, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67530, -1, 'Gnomeregan Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9889, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67531, -1, 'Stormwind Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9925, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67532, -1, 'Gilnean Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9962, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 1134, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67533, -1, 'Orgrimmar Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9999, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67535, -1, 'Silvermoon Satchel', '', '', '', '', 0, 8193, 0, 0, 1.0072, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 911, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(67536, -1, 'Darkspear Satchel', '', '', '', '', 0, 8193, 0, 0, 1.0109, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 530, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(92070, -1, 'Huojin Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9701, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 1352, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365),
(92071, -1, 'Tushui Satchel', '', '', '', '', 0, 8193, 0, 0, 0.9738, 1, 1, 20000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 63, 0, 0, 1353, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 18, 0, 0, 0, 6, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 26365);

-- Insert matching hotfix data
-- Reference for 'TableHash': https://github.com/TrinityCore/WowPacketParser/blob/master/WowPacketParser/Enums/DB2Hash.cs
DELETE FROM `hotfix_data` WHERE `Id` IN (67525, 67526, 67528, 67529, 67530, 67531, 67532, 67533, 67535, 67536, 92070, 92071);
INSERT INTO `hotfix_data` (`Id`, `TableHash`, `RecordID`, `Timestamp`, `Deleted`) VALUES
(67525, 2442913102, 67525, 0, 0),
(67526, 2442913102, 67526, 0, 0),
(67528, 2442913102, 67528, 0, 0),
(67529, 2442913102, 67529, 0, 0),
(67530, 2442913102, 67530, 0, 0),
(67531, 2442913102, 67531, 0, 0),
(67532, 2442913102, 67532, 0, 0),
(67533, 2442913102, 67533, 0, 0),
(67535, 2442913102, 67535, 0, 0),
(67536, 2442913102, 67536, 0, 0),
(92070, 2442913102, 92070, 0, 0),
(92071, 2442913102, 92071, 0, 0);