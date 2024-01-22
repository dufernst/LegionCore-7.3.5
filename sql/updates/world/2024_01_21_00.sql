-- Fix loot chance for the following quest items

-- 46768 = Sploder's Head
-- 55239 = Cragjaw's Huge Tooth
-- 55280 = Deepmoss Venom Sac

DELETE FROM `creature_loot_template` WHERE `item` IN (46768,55239,55280);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(34591, 46768, -100, 0, 0, 1, 1, 0),
(41064, 55239, -100, 0, 0, 1, 1, 0),

(4005, 55280, -100, 0, 0, 1, 1, 0),
(4006, 55280, -100, 0, 0, 1, 1, 0),
(4007, 55280, -100, 0, 0, 1, 1, 0),
(41185, 55280, -100, 0, 0, 1, 1, 0);