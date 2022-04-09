-- correct zoneId/areaId of certain NPCs in Acherus (DK class hall)
-- otherwise they get despawned when disabling patch 7.2 content
-- was zoneId 7543 and areaId 7973, both should be 7679
UPDATE creature SET zoneId = 7679, areaId = 7679 WHERE guid IN
(11657613, 11657612, 11657445, 11657443, 11657752, 11657753, 11657751, 11657750, 11657749, 11657755, 11657754, 383067, 11657368);

-- incorrect queststarter for quest 42533 (The Ruined Kingdom) (was questiver 400011, should be 93437)
DELETE FROM creature_queststarter WHERE quest = 42533 AND id IN (93437, 400011);
INSERT INTO creature_queststarter (`id`, `quest`) VALUES (93437, 42533);

-- loot fixes for Razorscale (Ulduar)
DELETE FROM creature_loot_template WHERE entry = 33186;
INSERT INTO creature_loot_template (entry, item, ChanceOrQuestChance, lootmode, groupid, mincountOrRef, maxcount) VALUES
(33186, 45038, 6.6571, 1, 0, 1, 1),
(33186, 45087, 25.8255, 1, 0, 1, 1),
(33186, 45137, 0, 1, 0, 1, 1),
(33186, 45138, 0, 1, 0, 1, 1),
(33186, 45139, 0, 1, 0, 1, 1),
(33186, 45140, 0, 1, 0, 1, 1),
(33186, 45141, 0, 1, 0, 1, 1),
(33186, 45142, 0, 1, 0, 1, 1),
(33186, 45143, 0, 1, 0, 1, 1),
(33186, 45146, 0, 1, 0, 1, 1),
(33186, 45147, 0, 1, 0, 1, 1),
(33186, 45148, 0, 1, 0, 1, 1),
(33186, 45149, 0, 1, 0, 1, 1),
(33186, 45150, 0, 1, 0, 1, 1),
(33186, 45151, 0, 1, 0, 1, 1),
(33186, 45298, 0, 1, 0, 1, 1),
(33186, 45299, 0, 1, 0, 1, 1),
(33186, 45301, 0, 1, 0, 1, 1),
(33186, 45302, 0, 1, 0, 1, 1),
(33186, 45303, 0, 1, 0, 1, 1),
(33186, 45304, 0, 1, 0, 1, 1),
(33186, 45305, 0, 1, 0, 1, 1),
(33186, 45306, 0, 1, 0, 1, 1),
(33186, 45307, 0, 1, 0, 1, 1),
(33186, 45308, 0, 1, 0, 1, 1),
(33186, 142087, 4.36, 1, 0, 1, 1);

-- Festergut/Rotface blood drop chance (Icecrown Citadel)
DELETE FROM creature_loot_template WHERE item IN (50226, 50231);
INSERT INTO creature_loot_template (entry, item, ChanceOrQuestChance, lootmode, groupid, mincountOrRef, maxcount, shared) VALUES
(36626, 50226, -100, 10, 0, 1, 1, 0),
(36627, 50231, -100, 10, 0, 1, 1, 0);

-- Fix for Assembly of Iron's loot (Ulduar)
DELETE FROM creature_loot_template WHERE entry IN (32857, 32867, 32927);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(32857, 45038, 8.1459, 1, 0, 1, 1, 0),
(32857, 45087, 36.4224, 1, 0, 1, 1, 0),
(32857, 45088, 0.6134, 1, 0, 1, 1, 0),
(32857, 45089, 0.2572, 1, 0, 1, 1, 0),
(32857, 45090, 0.2638, 1, 0, 1, 1, 0),
(32857, 45091, 0.343, 1, 0, 1, 1, 0),
(32857, 45092, 0.4485, 1, 0, 1, 1, 0),
(32857, 45093, 0.4683, 1, 0, 1, 1, 0),
(32857, 45094, 0.5672, 1, 0, 1, 1, 0),
(32857, 45095, 0.4551, 1, 0, 1, 1, 0),
(32857, 45096, 0.2045, 1, 0, 1, 1, 0),
(32857, 45097, 0.8509, 1, 0, 1, 1, 0),
(32857, 45098, 0.7321, 1, 0, 1, 1, 0),
(32857, 45099, 0.3496, 1, 0, 1, 1, 0),
(32857, 45100, 0.5475, 1, 0, 1, 1, 0),
(32857, 45101, 0.1913, 1, 0, 1, 1, 0),
(32857, 45102, 0.3694, 1, 0, 1, 1, 0),
(32857, 45103, 0.5013, 1, 0, 1, 1, 0),
(32857, 45104, 0.5343, 1, 0, 1, 1, 0),
(32857, 45105, 0.5936, 1, 0, 1, 1, 0),
(32857, 45193, 0, 1, 0, 1, 1, 0),
(32857, 45224, 0, 1, 0, 1, 1, 0),
(32857, 45225, 0, 1, 0, 1, 1, 0),
(32857, 45226, 0, 1, 0, 1, 1, 0),
(32857, 45227, 0, 1, 0, 1, 1, 0),
(32857, 45228, 0, 1, 0, 1, 1, 0),
(32857, 45232, 0, 1, 0, 1, 1, 0),
(32857, 45233, 0, 1, 0, 1, 1, 0),
(32857, 45234, 0, 1, 0, 1, 1, 0),
(32857, 45235, 0, 1, 0, 1, 1, 0),
(32857, 45236, 0, 1, 0, 1, 1, 0),
(32857, 45237, 0, 1, 0, 1, 1, 0),
(32857, 45238, 0, 1, 0, 1, 1, 0),
(32857, 45239, 0, 1, 0, 1, 1, 0),
(32857, 45240, 0, 1, 0, 1, 1, 0),
(32857, 45241, 0, 1, 0, 1, 1, 0),
(32857, 45242, 0, 1, 0, 1, 1, 0),
(32857, 45243, 0, 1, 0, 1, 1, 0),
(32857, 45244, 0, 1, 0, 1, 1, 0),
(32857, 45245, 0, 1, 0, 1, 1, 0),
(32857, 45322, 0, 1, 0, 1, 1, 0),
(32857, 45324, 0, 1, 0, 1, 1, 0),
(32857, 45329, 0, 1, 0, 1, 1, 0),
(32857, 45330, 0, 1, 0, 1, 1, 0),
(32857, 45331, 0, 1, 0, 1, 1, 0),
(32857, 45332, 0, 1, 0, 1, 1, 0),
(32857, 45333, 0, 1, 0, 1, 1, 0),
(32857, 45378, 0, 1, 0, 1, 1, 0),
(32857, 45418, 0, 1, 0, 1, 1, 0),
(32857, 45423, 0, 1, 0, 1, 1, 0),
(32857, 45447, 0, 1, 0, 1, 1, 0),
(32857, 45448, 0, 1, 0, 1, 1, 0),
(32857, 45449, 0, 1, 0, 1, 1, 0),
(32857, 45455, 0, 1, 0, 1, 1, 0),
(32857, 45456, 0, 1, 0, 1, 1, 0),
(32857, 45607, 0, 1, 0, 1, 1, 0),
(32857, 46027, 0.3562, 1, 0, 1, 1, 0),
(32857, 46348, 0.2704, 1, 0, 1, 1, 0),
(32857, 142088, 7.8496, 1, 0, 1, 1, 0),
(32867, 45038, 12.3544, 1, 0, 1, 1, 0),
(32867, 45087, 67.9305, 1, 0, 1, 3, 0),
(32867, 45088, 0.3487, 1, 0, 1, 1, 0),
(32867, 45089, 0.371, 1, 0, 1, 1, 0),
(32867, 45090, 0.7123, 1, 0, 1, 1, 0),
(32867, 45091, 0.3005, 1, 0, 1, 1, 0),
(32867, 45092, 0.5602, 1, 0, 1, 1, 0),
(32867, 45093, 0.2931, 1, 0, 1, 1, 0),
(32867, 45094, 0.2152, 1, 0, 1, 1, 0),
(32867, 45095, 0.3079, 1, 0, 1, 1, 0),
(32867, 45096, 0.3525, 1, 0, 1, 1, 0),
(32867, 45097, 0.3487, 1, 0, 1, 1, 0),
(32867, 45098, 0.4638, 1, 0, 1, 1, 0),
(32867, 45099, 0.4007, 1, 0, 1, 1, 0),
(32867, 45100, 0.5565, 1, 0, 1, 1, 0),
(32867, 45101, 0.4007, 1, 0, 1, 1, 0),
(32867, 45102, 0.4934, 1, 0, 1, 1, 0),
(32867, 45103, 0.6864, 1, 0, 1, 1, 0),
(32867, 45104, 0.3673, 1, 0, 1, 1, 0),
(32867, 45105, 0.3487, 1, 0, 1, 1, 0),
(32867, 45193, 0, 1, 0, 1, 1, 0),
(32867, 45224, 0, 1, 0, 1, 1, 0),
(32867, 45225, 0, 1, 0, 1, 1, 0),
(32867, 45226, 0, 1, 0, 1, 1, 0),
(32867, 45227, 0, 1, 0, 1, 1, 0),
(32867, 45228, 0, 1, 0, 1, 1, 0),
(32867, 45232, 0, 1, 0, 1, 1, 0),
(32867, 45233, 0, 1, 0, 1, 1, 0),
(32867, 45234, 0, 1, 0, 1, 1, 0),
(32867, 45235, 0, 1, 0, 1, 1, 0),
(32867, 45236, 0, 1, 0, 1, 1, 0),
(32867, 45237, 0, 1, 0, 1, 1, 0),
(32867, 45238, 0, 1, 0, 1, 1, 0),
(32867, 45239, 0, 1, 0, 1, 1, 0),
(32867, 45240, 0, 1, 0, 1, 1, 0),
(32867, 45241, 0, 1, 0, 1, 1, 0),
(32867, 45242, 0, 1, 0, 1, 1, 0),
(32867, 45243, 0, 1, 0, 1, 1, 0),
(32867, 45244, 0, 1, 0, 1, 1, 0),
(32867, 45245, 0, 1, 0, 1, 1, 0),
(32867, 45322, 0, 1, 0, 1, 1, 0),
(32867, 45324, 0, 1, 0, 1, 1, 0),
(32867, 45329, 0, 1, 0, 1, 1, 0),
(32867, 45330, 0, 1, 0, 1, 1, 0),
(32867, 45331, 0, 1, 0, 1, 1, 0),
(32867, 45332, 0, 1, 0, 1, 1, 0),
(32867, 45333, 0, 1, 0, 1, 1, 0),
(32867, 45378, 0, 1, 0, 1, 1, 0),
(32867, 45418, 0, 1, 0, 1, 1, 0),
(32867, 45423, 0, 1, 0, 1, 1, 0),
(32867, 45447, 0, 1, 0, 1, 1, 0),
(32867, 45448, 0, 1, 0, 1, 1, 0),
(32867, 45449, 0, 1, 0, 1, 1, 0),
(32867, 45455, 0, 1, 0, 1, 1, 0),
(32867, 45456, 0, 1, 0, 1, 1, 0),
(32867, 45506, 19.9303, 1, 0, 1, 1, 0),
(32867, 45607, 0, 1, 0, 1, 1, 0),
(32867, 45857, 41.6413, 1, 0, 1, 1, 0),
(32867, 46027, 0.3525, 1, 0, 1, 1, 0),
(32867, 46348, 0.3116, 1, 0, 1, 1, 0),
(32867, 142088, 3.9664, 1, 0, 1, 1, 0),
(32927, 45038, 6.9125, 1, 0, 1, 1, 0),
(32927, 45087, 69.2381, 1, 0, 1, 3, 0),
(32927, 45089, 0.2354, 1, 0, 1, 1, 0),
(32927, 45090, 0.4794, 1, 0, 1, 1, 0),
(32927, 45091, 0.4097, 1, 0, 1, 1, 0),
(32927, 45092, 0.1831, 1, 0, 1, 1, 0),
(32927, 45093, 0.2789, 1, 0, 1, 1, 0),
(32927, 45094, 0.3051, 1, 0, 1, 1, 0),
(32927, 45095, 0.4271, 1, 0, 1, 1, 0),
(32927, 45096, 0.2179, 1, 0, 1, 1, 0),
(32927, 45097, 0.2528, 1, 0, 1, 1, 0),
(32927, 45098, 0.2354, 1, 0, 1, 1, 0),
(32927, 45099, 0.5492, 1, 0, 1, 1, 0),
(32927, 45100, 0.2615, 1, 0, 1, 1, 0),
(32927, 45101, 0.1743, 1, 0, 1, 1, 0),
(32927, 45102, 0.523, 1, 0, 1, 1, 0),
(32927, 45103, 0.3923, 1, 0, 1, 1, 0),
(32927, 45104, 0.3748, 1, 0, 1, 1, 0),
(32927, 45105, 0.1308, 1, 0, 1, 1, 0),
(32927, 45193, 0, 1, 0, 1, 1, 0),
(32927, 45224, 0, 1, 0, 1, 1, 0),
(32927, 45225, 0, 1, 0, 1, 1, 0),
(32927, 45226, 0, 1, 0, 1, 1, 0),
(32927, 45227, 0, 1, 0, 1, 1, 0),
(32927, 45228, 0, 1, 0, 1, 1, 0),
(32927, 45232, 0, 1, 0, 1, 1, 0),
(32927, 45233, 0, 1, 0, 1, 1, 0),
(32927, 45234, 0, 1, 0, 1, 1, 0),
(32927, 45235, 0, 1, 0, 1, 1, 0),
(32927, 45236, 0, 1, 0, 1, 1, 0),
(32927, 45237, 0, 1, 0, 1, 1, 0),
(32927, 45238, 0, 1, 0, 1, 1, 0),
(32927, 45239, 0, 1, 0, 1, 1, 0),
(32927, 45240, 0, 1, 0, 1, 1, 0),
(32927, 45241, 0, 1, 0, 1, 1, 0),
(32927, 45242, 0, 1, 0, 1, 1, 0),
(32927, 45243, 0, 1, 0, 1, 1, 0),
(32927, 45244, 0, 1, 0, 1, 1, 0),
(32927, 45245, 0, 1, 0, 1, 1, 0),
(32927, 45322, 0, 1, 0, 1, 1, 0),
(32927, 45324, 0, 1, 0, 1, 1, 0),
(32927, 45329, 0, 1, 0, 1, 1, 0),
(32927, 45330, 0, 1, 0, 1, 1, 0),
(32927, 45331, 0, 1, 0, 1, 1, 0),
(32927, 45332, 0, 1, 0, 1, 1, 0),
(32927, 45333, 0, 1, 0, 1, 1, 0),
(32927, 45378, 0, 1, 0, 1, 1, 0),
(32927, 45418, 0, 1, 0, 1, 1, 0),
(32927, 45423, 0, 1, 0, 1, 1, 0),
(32927, 45447, 0, 1, 0, 1, 1, 0),
(32927, 45448, 0, 1, 0, 1, 1, 0),
(32927, 45449, 0, 1, 0, 1, 1, 0),
(32927, 45455, 0, 1, 0, 1, 1, 0),
(32927, 45456, 0, 1, 0, 1, 1, 0),
(32927, 45506, 19.2469, 1, 0, 1, 1, 0),
(32927, 45607, 0, 1, 0, 1, 1, 0),
(32927, 45857, 37.7615, 1, 0, 1, 1, 0),
(32927, 46027, 0.2266, 1, 0, 1, 1, 0),
(32927, 46348, 0.3748, 1, 0, 1, 1, 0),
(32927, 142088, 7.4633, 1, 0, 1, 1, 0);

-- Fix for Ulduar's boss loot lootmode
UPDATE creature_loot_template SET lootmode = 1 WHERE entry IN
(
SELECT lootid FROM creature_template WHERE lootid > 0
AND entry IN (33113, 33118, 33186, 33293, 32857, 32867, 32927, 32930, 33515, 32906, 32845, 33350, 32865, 33271, 33288, 32871)
);

-- fix Shattrath portal in Dalaran for Alliance (was faction 1735 (only allow Horde))
UPDATE gameobject_template SET faction = 0 WHERE entry = 246011;

-- Change drop chance of pets to wowhead's values (Trial of the Crusader)
UPDATE creature_loot_template SET ChanceOrQuestChance = 16.84 WHERE item = 142083 AND entry = 34797;
UPDATE creature_loot_template SET ChanceOrQuestChance = 2.61 WHERE item = 142084 AND entry = 34797;
UPDATE creature_loot_template SET ChanceOrQuestChance = 11.40 WHERE item = 142085 AND entry = 34564;
-- Change drop chance of pets to wowhead's values (Ulduar)
UPDATE creature_loot_template SET ChanceOrQuestChance = 10.44 WHERE item = 142086 AND entry = 33118;
UPDATE creature_loot_template SET ChanceOrQuestChance = 4.36 WHERE item = 142087 AND entry = 33186;
UPDATE creature_loot_template SET ChanceOrQuestChance = 7.85 WHERE item = 142088 AND entry = 32857;
UPDATE creature_loot_template SET ChanceOrQuestChance = 3.97 WHERE item = 142088 AND entry = 32867;
UPDATE creature_loot_template SET ChanceOrQuestChance = 7.46 WHERE item = 142088 AND entry = 32927;
UPDATE creature_loot_template SET ChanceOrQuestChance = 10.87 WHERE item = 142089 AND entry = 33515;
UPDATE creature_loot_template SET ChanceOrQuestChance = 14.54 WHERE item = 142090 AND entry = 32845;
UPDATE gameobject_loot_template SET ChanceOrQuestChance = 1.51 WHERE item = 142091 AND entry = 194324;
UPDATE gameobject_loot_template SET ChanceOrQuestChance = 21.69 WHERE item = 142092 AND entry = 194789;
UPDATE creature_loot_template SET ChanceOrQuestChance = 3.25 WHERE item = 142093 AND entry = 33288;
-- Change drop chance of pets to wowhead's values (Icecrown Citadel)
UPDATE creature_loot_template SET ChanceOrQuestChance = 8.80 WHERE item = 142094 AND entry = 36612;
DELETE FROM gameobject_loot_template WHERE entry = 202238 AND item = 142095;
INSERT INTO gameobject_loot_template (entry, item, ChanceOrQuestChance, lootmode, groupid, mincountOrRef, maxcount) VALUES
(202238, 142095, 15.67, 15, 0, 1, 1);
UPDATE creature_loot_template SET ChanceOrQuestChance = 8.27 WHERE item = 142096 AND entry = 36678;
UPDATE creature_loot_template SET ChanceOrQuestChance = 8.28 WHERE item = 142097 AND entry = 36853;
UPDATE creature_loot_template SET ChanceOrQuestChance = 1.44 WHERE item = 142098 AND entry = 36597;
UPDATE creature_loot_template SET ChanceOrQuestChance = 7.72 WHERE item = 142099 AND entry = 36597;

-- Trial of the Crusader - Faction Champions chest loot fix
UPDATE gameobject_template SET `Data1` = 195631 WHERE entry IN (195631, 195632, 195633, 195635);

-- Fix quests for The Oracles (Sholazar Basin)
UPDATE quest_template SET RewardFactionOverride1 = 70000 WHERE ID = 12705;
UPDATE quest_template SET RewardFactionOverride1 = 70000 WHERE ID = 12761;
UPDATE quest_template SET RewardFactionOverride1 = 70000 WHERE ID = 12762;
UPDATE quest_template SET RewardFactionValue1 = 7 WHERE ID = 12704;
UPDATE quest_template SET RewardFactionValue1 = 7 WHERE ID = 12726;
UPDATE quest_template SET RewardFactionValue1 = 7 WHERE ID = 12735;
UPDATE quest_template SET RewardFactionValue1 = 7 WHERE ID = 12736;
UPDATE quest_template SET RewardFactionValue1 = 7 WHERE ID = 12737;
UPDATE creature_template SET faction = 14, minlevel = 80, maxlevel = 80 WHERE entry = 29034;
