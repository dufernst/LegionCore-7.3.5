-- Fix loot chance for the following quest items

-- 61313 = Mossflayer Eye
-- 62390 = Scalding Whelp Corpse

DELETE FROM `creature_loot_template` WHERE `item` IN (61313,62390);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(8560, 61313, -85, 0, 0, 2, 2, 0),
(8561, 61313, -85, 0, 0, 2, 2, 0),
(8562, 61313, -85, 0, 0, 2, 2, 0),
(10822, 61313, -45, 0, 0, 2, 2, 0),
(12261, 61313, -85, 0, 0, 1, 2, 0),

(2725, 62390, -100, 0, 0, 1, 1, 0);