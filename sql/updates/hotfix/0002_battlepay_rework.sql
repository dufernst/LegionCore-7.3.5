-- add missing level boost item data
DELETE FROM `item_sparse` WHERE `ID` = 141410;
INSERT INTO `item_sparse` (`ID`, `AllowableRace`, `Display`, `Display1`, `Display2`, `Display3`, `Description`, `Stackable`, `ItemLevel`, `OverallQualityID`, `RequiredLevel`, `Bonding`) VALUES
(141410, -1, 'Invasion Survival Kit', '', '', '', 'Standard military grade.  Contains gold, potions and rations.', 1, 1, 1, 1, 1);
