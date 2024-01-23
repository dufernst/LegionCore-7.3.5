-- Fix loot chance for the following quest items

-- 23588 = Kaliri Feather
-- 23589 = Mag'har Ancestral Beads
-- 56040 = Ram Haunch
-- 56042 = Boulderslide Cheese
-- 56187 = Sentinel's Glaive
-- 56223 = Black Dragon Whelp Filet
-- 56224 = Blazing Heart of Fire

DELETE FROM `creature_loot_template` WHERE `item` IN (23588,23589,56040,56042,56187,56223,56224);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(17034, 23588, -55, 0, 0, 1, 1, 0),
(17035, 23588, -35, 0, 0, 1, 1, 0),
(17039, 23588, -35, 0, 0, 1, 1, 0),
(17053, 23588, -35, 0, 0, 1, 1, 0),

(16846, 23589, -75, 0, 0, 1, 1, 0),
(16847, 23589, -75, 0, 0, 1, 1, 0),

(34894, 56040, -75, 0, 0, 1, 1, 0),

(11915, 56042, -50, 0, 0, 1, 1, 0),
(11917, 56042, -50, 0, 0, 1, 1, 0),
(11918, 56042, -50, 0, 0, 1, 1, 0),

(34969, 56187, -100, 0, 0, 1, 1, 0),
(34898, 56223, -50, 0, 0, 1, 1, 0),
(34911, 56224, -75, 0, 0, 1, 1, 0);