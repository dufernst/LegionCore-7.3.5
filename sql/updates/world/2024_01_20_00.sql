-- Fix loot chance for the following quest items

-- 4503 = Witherbark Tusk
-- 4515 = Marez's Head
-- 4516 = Otto's Head
-- 4517 = Falconcrest's Head
-- 4522 = Witherbark Medicine Pouch
-- 58779 = Shell of Shadra
-- 60737 = Stabthistle Seed

DELETE FROM `creature_loot_template` WHERE `item` IN (4503,4515,4516,4517,4522,58779,60737);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(2552, 4503, -40, 0, 0, 1, 1, 0),
(2553, 4503, -40, 0, 0, 1, 1, 0),
(2554, 4503, -40, 0, 0, 1, 1, 0),
(2555, 4503, -40, 0, 0, 1, 1, 0),
(2556, 4503, -40, 0, 0, 1, 1, 0),
(2557, 4503, -40, 0, 0, 1, 1, 0),
(2558, 4503, -40, 0, 0, 1, 1, 0),
(2605, 4503, -100, 0, 0, 1, 1, 0),
(51631, 4503, -40, 0, 0, 1, 1, 0),
(51633, 4503, -40, 0, 0, 1, 1, 0),

(2783, 4515, -100, 0, 0, 1, 1, 0),
(2599, 4516, -100, 0, 0, 1, 1, 0),
(2597, 4517, -100, 0, 0, 1, 1, 0),

(2555, 4522, -40, 0, 0, 1, 1, 0),
(51633, 4522, -40, 0, 0, 1, 1, 0),

(42919, 58779, -100, 0, 0, 1, 1, 0),

(44635, 60737, -2, 0, 0, 1, 1, 0),
(44638, 60737, -2, 0, 0, 1, 1, 0);