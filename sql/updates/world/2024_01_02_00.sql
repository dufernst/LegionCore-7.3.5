-- Fix loot chance for the following quest items

-- 46692 = Elune's Torch

DELETE FROM `creature_loot_template` WHERE `item` IN (46692);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(34385, 46692, -100, 0, 0, 1, 1, 0);