/* Bóveda de las Celadoras */

/* Arreglos 

1- Implementada la luz de Elune en la zona de la Tumba del Penitente y añadida la condicion que al pasar la puerta pierdas la luz.

2- La luz de Elune ya debilita a las criaturas 
- Madre de linaje Aranasi [NPCID:97678]
- Espíritu de venganza [NPCID:100364]
- Arañuelo espinoso [NPCID:97677]

3- Arreglado los modelos de los Centinela y añadido nuevos spaw

*/


/* Elune's Light */


/* Evento Luz de elune */

DELETE FROM `creature_template` WHERE `entry` = 95474;
INSERT INTO `creature_template`(`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES 
(95474, 0, 1, 1, 0, 0, 0, 0, 0, 0, 35, 0, 0, 1, 1.14286, 1.14286, 0.1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'SmartAI', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, '');

/* Spam bunny 95474 */ 

DELETE FROM `creature` WHERE `id` = 103342;
DELETE FROM `creature` WHERE `id` = 95474;

INSERT INTO `creature`(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES 
(6760135, 95474, 1493, 7787, 7787, 8388870, 1, '', 0, 0, 4449.83, -321.806, -241.812, 6.0958, 300, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Elune's Light (201359)

DELETE FROM `creature_template_spell` WHERE `entry` = 95474 AND `spell` = 201359;
INSERT INTO `creature_template_spell`(`entry`, `spell`, `difficultyMask`, `timerCast`, `comment`) VALUES 
(95474, 201359, 7, 0, '@luz de Elune');

SET @bunnycast := 95474;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@bunnycast AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@bunnycast,0,0,0,10,0,100,1,1,20,0,0,11,201359,0,0,0,0,0,7,0,0,0,0,0,0,0,"Cast Luz"),
(@bunnycast,0,1,0,61,0,100,0,0,0,0,0,41,1,0,0,0,0,0,1,0,0,0,0,0,0,0,"Despaw");

/* Dolencia a la luz de las arañas  */

DELETE FROM `creature_difficulty_stat` WHERE `entry` = 97677;
INSERT INTO `creature_difficulty_stat`(`entry`, `difficulty`, `dmg_multiplier`, `HealthModifier`) VALUES (97677, 2, 1, 4.875);
INSERT INTO `creature_difficulty_stat`(`entry`, `difficulty`, `dmg_multiplier`, `HealthModifier`) VALUES (97677, 23, 1, 6.3375);
INSERT INTO `creature_difficulty_stat`(`entry`, `difficulty`, `dmg_multiplier`, `HealthModifier`) VALUES (97677, 8, 1, 8.5556);


DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 100525 AND `spell_id` = 228210;
INSERT INTO `npc_spellclick_spells`(`npc_entry`, `spell_id`, `cast_flags`, `user_type`, `add_npc_flag`) VALUES 
(100525, 228210, 1, 0, 1);

DELETE FROM `creature_template_wdb` WHERE `Entry` = 100525;
INSERT INTO `creature_template_wdb`(`Entry`, `Name1`, `Name2`, `Name3`, `Name4`, `NameAlt1`, `NameAlt2`, `NameAlt3`, `NameAlt4`, `Title`, `TitleAlt`, `CursorName`, `TypeFlags`, `TypeFlags2`, `Type`, `Family`, `Classification`, `KillCredit1`, `KillCredit2`, `VignetteID`, `Displayid1`, `Displayid2`, `Displayid3`, `Displayid4`, `HpMulti`, `PowerMulti`, `Leader`, `QuestItem1`, `QuestItem2`, `QuestItem3`, `QuestItem4`, `QuestItem5`, `QuestItem6`, `QuestItem7`, `QuestItem8`, `QuestItem9`, `QuestItem10`, `MovementInfoID`, `RequiredExpansion`, `FlagQuest`, `VerifiedBuild`) VALUES 
(100525, 'Glowing Sentry', '', '', '', '', '', '', '', '', '', 'questinteract', 1075839064, 6, 4, 0, 1, 0, 0, 0, 66736, 0, 0, 0, 10, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 22566);

DELETE FROM `smart_scripts` WHERE `entryorguid` = 100525;
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (100525, 0, 1, 0, 54, 0, 100, 1, 10, 10, 1000, 1000, 11, 197941, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '');
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (100525, 0, 2, 3, 73, 0, 100, 0, 0, 0, 0, 0, 86, 192656, 0, 7, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, '');
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (100525, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '');
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (100525, 0, 0, 0, 54, 0, 100, 1, 10, 10, 1000, 1000, 11, 197910, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '');


DELETE FROM `smart_scripts` WHERE `entryorguid` = 103342;
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (103342, 0, 1, 0, 54, 0, 100, 1, 10, 10, 1000, 1000, 11, 197941, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '');
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (103342, 0, 2, 3, 73, 0, 100, 0, 0, 0, 0, 0, 86, 192656, 0, 7, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, '');
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (103342, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 41, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '');
INSERT INTO `smart_scripts`(`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (103342, 0, 0, 0, 54, 0, 100, 1, 10, 10, 1000, 1000, 11, 197910, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '');

/* Varios */

UPDATE `creature_template` SET `AIName` = 'SmarAI' WHERE `entry` = 102566;

-- <Torment> SAI
SET @Torment := 102566;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@Torment AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@Torment,0,0,0,0,0,100,0,5000,6000,5000,6000,11,202608,0,0,0,0,0,2,0,0,0,0,0,0,0,"Npc - Event - Action (phase) (dungeon difficulty)"),
(@Torment,0,1,0,0,0,100,0,5000,6000,5000,6000,11,202615,0,0,0,0,0,2,0,0,0,0,0,0,0,"Npc - Event - Action (phase) (dungeon difficulty)");

/* Set model */
UPDATE `creature_template_wdb` SET 
`Displayid1` = 66736, 
`Displayid2` = 0 WHERE 
`Entry` = 96524;

/* Spawn */
DELETE FROM `creature` WHERE `guid` = 6760130;
INSERT INTO `creature`(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES 
(6760130, 96524, 1493, 7787, 7787, 8388870, 1, '', 0, 0, 4404.94, -229.678, -257.723, 4.16033, 300, 0, 0, 12833270, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `creature` WHERE `guid` = 6760131;
INSERT INTO `creature`(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES 
(6760131, 96524, 1493, 7787, 7787, 8388870, 1, '', 0, 0, 4469.57, -293.947, -240.117, 4.11014, 300, 0, 0, 10751230, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

