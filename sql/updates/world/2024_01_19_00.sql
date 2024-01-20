-- Fix loot chance for the following quest items

-- 2636 = Carved Stone Idol
-- 57131 = Intact Crocolisk Jaw
-- 60402 = Mosshide Ear
-- 60404 = Foreman Sharpsneer's Head
-- 60497 = Bear Rump
-- 60754 = Glassy Hornet Wing
-- 60755 = Fluffy Fox Tail

DELETE FROM `creature_loot_template` WHERE `item` IN (2636,57131,60402,60404,60497,60754,60755);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(1165, 2636, -75, 0, 0, 1, 1, 0),
(1167, 2636, -75, 0, 0, 1, 1, 0),

(1693, 57131, -65, 0, 0, 1, 1, 0),

(44161, 60402, -100, 0, 0, 1, 1, 0),
(44162, 60402, -100, 0, 0, 1, 1, 0),
(45384, 60402, -100, 0, 0, 1, 1, 0),

(44198, 60404, -100, 0, 0, 1, 1, 0),
(1186, 60497, -90, 0, 0, 1, 1, 0),

(44620, 60754, -65, 0, 0, 1, 1, 0),
(45402, 60754, -65, 0, 0, 1, 1, 0),

(44635, 60755, -100, 0, 0, 1, 1, 0),
(45380, 60755, -100, 0, 0, 1, 1, 0);