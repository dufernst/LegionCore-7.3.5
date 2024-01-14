-- Fix loot chance for the following quest items

-- NOTE: These SHOULD be negative but they do not drop for some reason!!!
-- Need to come back to this and figure out why these break when marked as quest only (like they should be).

-- 2797 = Heart of Mokk
-- 23681 = Heart of Naias

DELETE FROM `creature_loot_template` WHERE `item` IN (2797,23681);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(1514, 2797, 100, 0, 0, 1, 1, 0),
(17207, 23681, 100, 0, 0, 1, 1, 0);