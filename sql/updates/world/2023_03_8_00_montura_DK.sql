/* Montura de Clase DK */

DELETE FROM `creature` WHERE (`guid`='11841453');
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES 
('11841453', '120163', '1220', '7679', '7679', '1', '65535', '', '0', '0', '-1511.26', '1066.17', '443.224', '0.148182', '300', '0', '0', '1039267', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0');

/*Update phaseid */
UPDATE `creature` SET `PhaseId`='5140' WHERE (`guid`='339323');

/*Evento Torre de dragones */
--  SAI Nomo 
SET @Nomo := 27938;
UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry`=@Nomo;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@Nomo AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@Nomo,0,0,0,62,0,100,0,20913,0,0,0,85,239518,0,0,0,0,0,7,0,0,0,0,0,0,0,"gossip - spellcast");

--  SAI Dragon
SET @DragonB := 26443;
UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry`=@DragonB;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@DragonB AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@DragonB,0,0,0,62,0,100,0,9455,2,0,0,80,@DragonB*100+00,0,0,0,0,0,1,0,0,0,0,0,0,0,"gossip - actions"),
(@DragonB,0,1,0,62,0,100,0,9455,0,0,0,52,878,0,0,0,0,0,7,0,0,0,0,0,0,0,"gossip - ActiveTaxi"),
(@DragonB,0,2,0,62,0,100,0,9455,1,0,0,52,883,0,0,0,0,0,7,0,0,0,0,0,0,0,"gossip - ActiveTaxi");


/* Sagrario Rubi */
DELETE FROM `quest_objectives` WHERE (`QuestID`='46812');
INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES ('289660', '46812', '0', '0', '27938', '1', '0', '0', '0', 'interroga', '24367');
INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES ('289662', '46812', '0', '1', '26443', '1', '2', '0', '0', 'habla con el dragon', '24367');
INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES ('289663', '46812', '0', '2', '120220', '1', '2', '0', '0', 'kill credis S Rubi', '24367');
INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES ('289664', '46812', '0', '3', '120370', '1', '2', '0', '0', 'Libro', '24367');

/*Quest 46813  final*/

DELETE FROM `quest_objectives` WHERE (`QuestID`='46813');
INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES ('289678', '46813', '0', '0', '120309', '1', '0', '0', '0', NULL, '24367');
INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES ('289679', '46813', '0', '1', '120324', '1', '2', '0', '0', NULL, '24367');

--  SAI
SET @Hielo := 120309;
UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry`=@Hielo;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@Hielo AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@Hielo,0,0,0,54,0,100,0,0,0,0,0,49,0,0,0,0,0,0,18,100,0,0,0,0,0,0,"justSum - attackP");


/* SAI npc scena */
DELETE FROM `smart_scripts` WHERE 
(`entryorguid`='362') AND (`source_type`='10') AND (`id`='0') AND (`link`='0');
DELETE FROM `smart_scripts` WHERE 
(`entryorguid`='121128') AND (`source_type`='0') AND (`id`='0') AND (`link`='0');
