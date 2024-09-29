-- Fix loot chance for the following quest items

-- 23677 = Moongraze Buck Hide

DELETE FROM `creature_loot_template` WHERE `item` IN (23677);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(17201, 23677, -80, 0, 0, 1, 1, 0);