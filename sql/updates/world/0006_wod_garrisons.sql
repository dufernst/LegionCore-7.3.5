-- Add missing gameobject_template data
INSERT INTO gameobject_template (`entry`, `type`, `displayId`, `name`, `Data0`) VALUES
(230471, 38, 18572, 'Garrison Building Horde Stable V3', 1348);

-- used to be "4177 4086 3962 3592 3324 3213 3196 3026 3025 3023 3021 3007 2537 2406" for 109 npcs
-- from videos showing the completion of "Establish Your Garrison" it seems as if these npc's should be spawned without conditions
UPDATE creature SET PhaseId = "" WHERE zoneId = 7004 AND `map` = 1152;
-- same for the gameobjects (47)
UPDATE gameobject SET PhaseId = "" WHERE zoneId = 7004 AND `map` = 1152;

-- remove double spawns from Horde Garrison LVL 1
-- INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
-- (146801691, 78466, 1152, 7004, 7096, 2, 1, '', 0, 1, 5566.65, 4514.58, 132.09, 0.119902, 300, 0, 0, 73390, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (146851281, 80225, 1152, 7004, 7096, 2, 65535, '', 0, 1, 5579.15, 4596.13, 136.587, 2.92535, 300, 10, 0, 35231, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (146851283, 86775, 1152, 7004, 7096, 2, 65535, '', 0, 1, 5737.91, 4538.1, 137.95, 1.74411, 300, 0, 0, 35231, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (146851295, 80600, 1152, 7004, 7096, 2, 1, '', 0, 0, 5563.89, 4517.33, 132.069, 0.300546, 300, 0, 0, 73390, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (146851297, 79740, 1152, 7004, 7096, 2, 65535, '', 0, 0, 5557.01, 4507.43, 132.677, 0.353162, 300, 0, 0, 217454, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (146851766, 80432, 1152, 7004, 7096, 2, 1, '', 0, 0, 5559.03, 4507.51, 132.688, 3.28426, 300, 0, 0, 73390, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (146851768, 79774, 1152, 7004, 7096, 2, 1, '', 0, 1, 5594.51, 4568.77, 135.827, 4.10264, 300, 0, 0, 81545, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146801691 AND id = 78466;
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146851281 AND id = 80225;
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146851283 AND id = 86775;
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146851295 AND id = 80600;
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146851297 AND id = 79740;
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146851766 AND id = 80432;
DELETE FROM creature WHERE zoneId = 7004 AND guid = 146851768 AND id = 79774;

-- Horde Garrison LVL 2
-- correct npcflag for Command Table, use UNIT_NPC_FLAG2_SHIPMENT_CRAFTER (currently 32) instead of UNIT_NPC_FLAG2_GARRISON_MISSION_NPC (currently 64)
-- the client sends CMSG_OPEN_TRADESKILL_NPC when an npc has UNIT_NPC_FLAG2_GARRISON_MISSION_NPC (currently 64)
UPDATE creature_template SET npcflag2 = 32 WHERE entry = 86031;

-- Horde Garrison LVL 3
-- fix command table npcflag (was 1), otherwise won't open the actual mission menu
UPDATE creature_template SET npcflag = 0 WHERE entry = 85805;

-- Alliance Garrison - Keeping it Together
UPDATE creature_template SET gossip_menu_id = 16613 WHERE entry = 84455;

-- English text when accepting = Build Your Barrack
UPDATE creature_text SET BroadcastTextID = 83681 WHERE Entry = 77209 AND GroupID = 3 AND ID = 0;

-- Fix disguise for quest 33080 (Going Undercover)
DELETE FROM spell_area WHERE spell IN (150455, 150456);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `classmask`, `active_event`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(150455, 0, 0, 0, 148593, 0, 0, 0, 1, 1, 64, 11),
(150456, 0, 0, 0, 148593, 0, 0, 0, 0, 1, 64, 11);

-- Add missing Yrel spawn in level 1 Alliance garrison
DELETE FROM creature WHERE guid IN (1000003);
INSERT INTO creature (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(1000003, 80568, 1158, 7078, 7085, 2, 1, '', 0, 1, 1858.39, 224.75, 76.6343, 0.267199, 300, 0, 0, 381400, 65000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Make A Hero's Welcome (33075) depend on Delegating on Draenor (34692, the first use of the command table)
UPDATE quest_template_addon SET PrevQuestID = 34692 WHERE ID = 33075;

-- De-disable Fast Expansion quest (33814)
-- INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES (1, 33814, 0, '', '', '');
DELETE FROM disables WHERE sourceType = 1 AND entry = 33814;
-- and add previous quest requirements
UPDATE quest_template_addon SET ExclusiveGroup = -33076, NextQuestID = 33814 WHERE ID IN(33076, 33081);
-- De-disable Bigger is Better (36592)
-- INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES (1, 36592, 0, '', '', '');
DELETE FROM disables WHERE sourceType = 1 AND entry = 36592;
