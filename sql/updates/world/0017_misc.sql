-- require Justice Rains from Above to be completed before unlocking The Underking
UPDATE quest_template_addon SET NextQuestID = 39780 WHERE ID = 40594;

-- remove 2 of triple spawned Lieutenant Desdel Stareye (was spawnMask = 1)
UPDATE creature SET spawnMask = 0 WHERE guid IN (285975, 286837);

-- Fix drop chance of quest item Brass Collar (Elwynn Forest)
UPDATE creature_loot_template SET ChanceOrQuestChance = -100 WHERE item = 1006 AND entry = 330;

-- Add some missing spawns
DELETE FROM creature WHERE id IN (72940, 73763, 74635, 77853, 80058, 82222, 82225, 82302, 82662, 82886, 84747, 85089, 88530);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(1000013, 72940, 1116, 6720, 6779, 1, 65535, '', 0, 1, 5762.47, 3841.95, 124.635, 4.73313, 300, 0, 0, 261, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000014, 73763, 1116, 6720, 6867, 1, 65535, '', 0, 0, 7215.81, 6103.96, 117.454, 2.83355, 300, 0, 0, 146320, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000015, 74635, 1116, 6720, 6867, 1, 65535, '', 0, 1, 7109.02, 6013.09, 133.845, 2.79979, 300, 0, 0, 25694, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000016, 77853, 1116, 6662, 7157, 1, 65535, '', 0, 0, 3167.41, 788.644, 77.8363, 5.88782, 300, 0, 0, 1495660, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000017, 80058, 1116, 6662, 6662, 1, 65535, '', 0, 1, 3600, 1818.86, 214.319, 3.09207, 300, 0, 0, 103875, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000018, 82222, 1116, 6721, 7304, 1, 65535, '', 0, 1, 6921.21, 789.775, 155.946, 1.87658, 300, 0, 0, 120162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000019, 82225, 1116, 6721, 7304, 1, 65535, '', 0, 1, 6768.81, 745.892, 189.264, 5.69431, 300, 0, 0, 120162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000020, 82302, 1116, 6721, 7190, 1, 65535, '', 0, 0, 7000.22, 955.938, 80.093, 4.88712, 300, 0, 0, 120162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000021, 82662, 1116, 6662, 7422, 1, 65535, '', 0, 1, 3117.73, 3307.68, 46.5452, 4.25648, 300, 0, 0, 3365230, 640000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000022, 82886, 1116, 6722, 7274, 1, 1, '', 0, 0, -965.845, 988.727, 9.26919, 5.77909, 300, 0, 0, 50133, 138000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000023, 84747, 1116, 6720, 6774, 1, 65535, '', 0, 0, 6300.45, 4083.24, 92.1538, 2.96057, 300, 0, 0, 25694, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000024, 85089, 1116, 6721, 7190, 1, 65535, '', 0, 1, 6972.98, 872.132, 93.2216, 5.57301, 300, 0, 0, 120162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000025, 88530, 1116, 6721, 7319, 1, 1, '', 0, 0, 7238.09, 1712.42, 97.8764, 2.13879, 300, 0, 0, 504785, 32000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Remove Val'Sharah creature_action spam, undo with:
-- INSERT INTO `creature_action` (`entry`, `target`, `type`, `spellId`, `action`) VALUES
-- (92788, 94014, 0, 0, 0),
-- (92788, 94137, 0, 0, 0),
-- (92789, 94014, 0, 0, 0),
-- (92789, 94137, 0, 0, 0),
-- (94014, 92788, 0, 0, 0),
-- (94014, 92789, 0, 0, 0),
-- (94014, 105902, 0, 0, 0),
-- (94137, 92788, 0, 0, 0),
-- (94137, 92789, 0, 0, 0);
DELETE FROM creature_action WHERE `entry` IN (92789, 92788, 94137, 94014);

-- Fix drop chance of quest items Loramus' Head, Loramus' Torso and Loramus' Legs (Blasted Lands). Fix provided by Jenkaa.
UPDATE gameobject_loot_template SET ChanceOrQuestChance = -100 WHERE item = 55829 AND entry = 203204;
DELETE FROM gameobject_loot_template WHERE item = 55836 AND entry = 203204;
DELETE FROM gameobject_loot_template WHERE item = 55837 AND entry = 203204;
