-- Genn Greymane gossip after Broken Shore scenario
DELETE FROM npc_text WHERE ID = 70000;
INSERT INTO npc_text (ID, BroadcastTextID0) VALUES (70000, 104057);
DELETE FROM gossip_menu WHERE Entry = 70000;
INSERT INTO gossip_menu (Entry, TextID, FriendshipFactionID) VALUES (70000, 70000, 0);
UPDATE gossip_menu_option SET ActionMenuID = 70000 WHERE OptionBroadcastTextID = 102170;

-- correct questchain/phasing for Horde (none Demon Hunters)
-- questline: 43926, 44281, 40518, 40522, 40760, 40607, 40605, 44663
-- deprecated: 44091
-- relevant zones: Orgrimmar (zoneID: 1637) and Durotar (zoneID: 14)
UPDATE quest_template_addon SET AllowableClasses = 1|2|4|8|16|32|64|128|256|512|1024 WHERE ID = 43926;
UPDATE quest_template SET RewardNextQuest = 44663 WHERE ID = 40605;

-- correct questchain/phasing for Horde Demon Hunters
-- questline: 40976, 40982, 40983, 41002, 44663
-- deprecated: 44091
UPDATE quest_template_addon SET AllowableClasses = 2048 WHERE ID = 40976;
UPDATE quest_template_addon SET NextQuestID = 44663 WHERE ID = 41002;
UPDATE quest_template SET RewardNextQuest = 44663 WHERE ID = 41002;

-- correct questchain/phasing for Alliance (none Demon Hunters)
-- questline: 40519, 42782, 42740, 40517, 40593, 44120, 44663
-- deprecated: 43635
UPDATE quest_template_addon SET AllowableClasses = 1|2|4|8|16|32|64|128|256|512|1024 WHERE ID = 40519;
UPDATE quest_template SET RewardNextQuest = 40593 WHERE ID = 40517;
UPDATE quest_template SET RewardNextQuest = 44663 WHERE ID = 44120;

-- correct questchain/phasing for Alliance Demon hunters
-- questline: 39691, 44471, 44463, 44473, 44663
-- deprecated: 43635
UPDATE quest_template_addon SET AllowableClasses = 2048 WHERE ID = 39691;
UPDATE quest_template_addon SET NextQuestID = 44663 WHERE ID = 44473;
UPDATE quest_template SET RewardNextQuest = 44663 WHERE ID = 44473;

-- query for work on phasing
-- SELECT c.SourceEntry, c.ElseGroup, ttof.description, tcd.description, c.ConditionValue1 AS QuestOrScene, c.ConditionValue2 AS Objective,
-- qtl.LogTitle, qo.ID, qol.Description, pd.phasemask, pd.flags, pd.phaseId
-- FROM conditions AS c
-- JOIN quest_template_locale AS qtl ON c.ConditionValue1 = qtl.ID
-- LEFT JOIN quest_objectives AS qo ON (c.ConditionValue2 = qo.ObjectID AND c.ConditionValue1 = qo.QuestID)
-- LEFT JOIN quest_objectives_locale AS qol ON qo.ID = qol.ID
-- LEFT JOIN temp_conditions_descriptions AS tcd ON c.ConditionTypeOrReference = tcd.ID
-- LEFT JOIN temp_true_or_false AS ttof ON c.NegativeCondition = ttof.`int`
-- LEFT JOIN phase_definitions AS pd ON (c.SourceEntry = pd.entry AND c.SourceGroup = pd.zoneId)
-- WHERE c.SourceTypeOrReferenceId = 23 AND c.SourceGroup = 1519
-- ORDER BY c.SourceEntry ASC, c.ElseGroup;

-- Alliance part phasing
-- relevant zones: Stormwind and Stormwind Harbor (zoneID: 1519)
-- The Legion Returns (40519) not none till completed/rewarded 42740, phase_definitions entry: 6 and phaseId: 7047
-- old:
-- INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
-- (23, 1519, 6, 0, 1, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion. is non 44663'),
-- (23, 1519, 6, 0, 1, 14, 0, 40519, 0, 0, 1, 0, '', 'Legion. is non 40519');
-- new:
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 1519 AND SourceEntry = 6;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 1519, 6, 0, 1, 14, 0, 40519, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_NONE The Legion Returns'),
(23, 1519, 6, 0, 1, 28, 0, 42740, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED The Battle for Broken Shore'),
(23, 1519, 6, 0, 1, 8, 0, 42740, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED The Battle for Broken Shore');
-- The Battle for Broken Shore (42740) completed/rewarded till completed/rewarded 44663, phase definitions entry: 7 and phaseId: 7053
-- old:
-- INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
-- (23, 1519, 7, 0, 1, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion. is non 44663');
-- (23, 1519, 7, 0, 1, 14, 0, 42740, 0, 0, 1, 0, '', 'Legion. is non 42740');
-- new:
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 1519 AND SourceEntry = 7;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 1519, 7, 0, 1, 28, 0, 42740, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_COMPLETED The Battle for Broken Shore'),
(23, 1519, 7, 0, 1, 28, 0, 44663, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED In the Blink of an Eye'),
(23, 1519, 7, 0, 1, 8, 0, 44663, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED In the Blink of an Eye'),
(23, 1519, 7, 0, 2, 8, 0, 42740, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED The Battle for Broken Shore'),
(23, 1519, 7, 0, 2, 28, 0, 44663, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED In the Blink of an Eye'),
(23, 1519, 7, 0, 2, 8, 0, 44663, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED In the Blink of an Eye');
-- The first one below is optional, things are done not entirely properly in the current database, just keep it as it is
-- The Battle for Broken Shore (42740) completed/rewarded till Demons Among Us/Them (after ask jayce objective complete, 41, 111585, for quest 40593 and 44463),
-- phase_definitions entry: none existing yet and phaseId: 7510
-- for both above add the equivalent in the Demon Hunter version
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 1519, 7, 0, 3, 14, 0, 39691, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_NONE The Call of War'),
(23, 1519, 7, 0, 3, 28, 0, 44663, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED In the Blink of an Eye'),
(23, 1519, 7, 0, 3, 8, 0, 44663, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED In the Blink of an Eye');
-- till rewarded 39691 (DH first quest) spawn 39691 questiver for DH
-- Demons Among Us/Them (after ask jayce objective complete, 41, 111585, for quest 40593 and 44463) till 44663 not none phase_definition entry: 1250 and phaseId: 5552
-- old:
-- INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
-- (23, 1519, 1250, 0, 0, 14, 0, 42740, 0, 0, 1, 0, '', NULL);
-- (23, 1519, 1250, 0, 0, 14, 0, 44663, 0, 0, 0, 0, '', NULL);
-- new:
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 1519 AND SourceEntry = 1250;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 1519, 1250, 0, 1, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 1519, 1250, 0, 1, 41, 0, 40593, 111585, 0, 0, 0, '', 'Legion Start Questline - QUEST_OBJECTIVE_DONE Demons Among Us'),
(23, 1519, 1250, 0, 2, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 1519, 1250, 0, 2, 41, 0, 44463, 111585, 0, 0, 0, '', 'Legion Start Questline - QUEST_OBJECTIVE_DONE Demons Among Them'),
(23, 1519, 1250, 0, 3, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 1519, 1250, 0, 3, 8, 0, 40593, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Demons Among Us'),
(23, 1519, 1250, 0, 4, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 1519, 1250, 0, 4, 8, 0, 44463, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Demons Among Them');

-- make The Battle for Broken Shore mandatory again
UPDATE quest_objectives SET Amount = 1 WHERE QuestID = 42740;

-- spawn questgivers/enders for demon hunter questline in Stormwind/Alliance
-- not bothering with phasing as it is a single npc and we have bigger things to worry about
-- TODO: fix phasing
DELETE FROM creature WHERE guid IN (1000000, 1000001);
INSERT INTO creature (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(1000000, 97296, 0, 1519, 5390, 1, 1921, '', 0, 1, -8540.8, 462.49, 104.718, 5.25264, 300, 0, 0, 9145554, 5500000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000001, 102585, 0, 1519, 6292, 1, 1921, '', 0, 0, -8386.86, 256.781, 155.347, 5.54717, 300, 0, 0, 870, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM creature_queststarter WHERE id = 97296 AND quest = 39691;
INSERT INTO creature_queststarter (id, quest) VALUES (97296, 39691);
DELETE FROM creature_queststarter WHERE id = 102585 AND quest IN(44471, 44463);
INSERT INTO creature_queststarter (id, quest) VALUES
(102585, 44471),
(102585, 44463);
DELETE FROM creature_queststarter WHERE id = 100429 AND quest = 44473;
INSERT INTO creature_queststarter (id, quest) VALUES (100429, 44473);
DELETE FROM creature_questender WHERE id = 97749 AND quest = 39691;
DELETE FROM creature_questender WHERE id = 100429 AND quest = 44463;
INSERT INTO creature_questender (id, quest) VALUES (100429, 44463);

UPDATE creature_template SET ScriptName = 'npc_102585_jace_for_dh_questline' WHERE entry = 102585;

-- add gossip menu (including conditions for when to use which) to legion start questline Anduin
-- it seems like there is a separate spawn for the Demon Hunter version of the questline
-- but as this is not implemented anyway we might as well make things easy by using the same spawn :)
DELETE FROM gossip_menu WHERE Entry = 19075 AND TextID = 30591;
INSERT INTO gossip_menu (Entry, TextID) VALUES (19075, 30591);
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 19075 AND SourceEntry IN (27884, 30591);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(14, 19075, 27884, 0, 0, 15, 0, 2048, 0, 0, 1, 0, '', 'Legion Start Questline - Anduin None Demon Hunter Gossip Menu'),
(14, 19075, 30591, 0, 0, 15, 0, 2048, 0, 0, 0, 0, '', 'Legion Start Questline - Anduin Demon Hunter Gossip Menu');

-- add menu options for demon hunter quest
DELETE FROM gossip_menu_option WHERE MenuID = 19075 AND OptionIndex = 1;
INSERT INTO gossip_menu_option (MenuID, OptionIndex, OptionText, BoxText, OptionBroadcastTextID) VALUES
(19075, 1, 'This cannot wait. There are demons among your ranks. Let me show you.', '', 122662);

DELETE FROM conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup = 19075 AND SourceEntry = 1;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(15, 19075, 1, 0, 0, 9, 0, 44463, 0, 0, 0, 0, '', 'Legion Start Questline - Incomplete Quest (44463) Demons Among Them'),
(15, 19075, 1, 0, 0, 41, 0, 44463, 111585, 1, 1, 0, '', 'Legion Start Questline - Not yet warned Anduin for Quest (44463) Demons Among Them');

DELETE FROM smart_scripts WHERE entryorguid = 100429 AND id = 6;
INSERT INTO smart_scripts (entryorguid, id, event_type, event_param1, event_param2, action_type, action_param1, target_type, `comment`) VALUES
(100429, 6, 62, 19075, 1, 85, 225500, 7, 'At gossip select, cast Demon Attack scene');

-- Horde part phasing
-- relevant zones: Durotar (zoneID: 14) Orgrimmar (1637)
-- The Legion Returns (43926) not none till completed/rewarded 40518, phase_definitions entry: 2 and phaseId: 7554 7553
-- old:
-- INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
-- (23, 14, 2, 0, 0, 14, 0, 43926, 0, 0, 1, 0, '', 'start BI'),
-- (23, 14, 2, 0, 0, 14, 0, 40522, 0, 0, 0, 0, '', 'end BI');
-- phase_definitions old: 7554 7553 2366 2361 2360 2359 2284
-- phase_definitions new: 7554 7553
-- move some spawns to be only in those 2 phases - old: 7554 7553 2366 2361 2360 2359 2284
-- we just ignore game objects, those are not going to interact anyway with npcs
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 14 AND SourceEntry = 2;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 14, 2, 0, 1, 14, 0, 43926, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_NONE The Legion Returns'),
(23, 14, 2, 0, 1, 28, 0, 40518, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED The Battle for Broken Shore'),
(23, 14, 2, 0, 1, 8, 0, 40518, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED The Battle for Broken Shore');
UPDATE phase_definitions SET phaseId = '7554 7553' WHERE zoneId = 14 AND entry = 2;
UPDATE creature SET PhaseId = '7554 7553' WHERE zoneId = 14 AND `map` = 1 AND PhaseId LIKE "%7554 7553%";
-- The Battle for Broken Shore (40518) completed/rewarded till Fate of the Horde (40522) rewarded or objective (100934) done
-- or Horde DK and not completed/rewarded Audience with the Warchief (40976)
-- phase_definitions entry: 4 and phaseId: 7425
UPDATE phase_definitions SET phasemask = '2', phaseId = '7425' WHERE zoneId = 14 AND entry = 4;
UPDATE creature SET PhaseId = '7425' WHERE zoneId = 14 AND `map` = 1 AND PhaseId LIKE "%7425 7422%";
UPDATE creature SET PhaseId = '7554 7553 7425' WHERE guid = 350403;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 14 AND SourceEntry = 4;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 14, 4, 0, 1, 28, 0, 40518, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_COMPLETED The Battle for Broken Shore'),
(23, 14, 4, 0, 1, 41, 0, 40522, 100934, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_OBJECTIVE_DONE Fate of the Horde'),
(23, 14, 4, 0, 1, 8, 0, 40522, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Fate of the Horde'),
(23, 14, 4, 0, 2, 8, 0, 40518, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED The Battle for Broken Shore'),
(23, 14, 4, 0, 2, 41, 0, 40522, 100934, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_OBJECTIVE_DONE Fate of the Horde'),
(23, 14, 4, 0, 2, 8, 0, 40522, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Fate of the Horde'),
(23, 14, 4, 0, 3, 8, 0, 39690, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Horde - Illidari, We Are Leaving'),
(23, 14, 4, 0, 3, 8, 0, 40976, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Audience with the Warchief'),
(23, 14, 4, 0, 3, 28, 0, 40976, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED Audience with the Warchief');
-- also add for relevant Orgrimmar phases
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 1637 AND SourceEntry = 6 AND ElseGroup = 0;
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 1637 AND SourceEntry = 54 AND ElseGroup = 1;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 1637, 6, 0, 0, 8, 0, 39690, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Horde - Illidari, We Are Leaving'),
(23, 1637, 6, 0, 0, 8, 0, 40976, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Audience with the Warchief'),
(23, 1637, 6, 0, 0, 28, 0, 40976, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED Audience with the Warchief'),
(23, 1637, 54, 0, 1, 8, 0, 39690, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Horde - Illidari, We Are Leaving'),
(23, 1637, 54, 0, 1, 8, 0, 40976, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Audience with the Warchief'),
(23, 1637, 54, 0, 1, 28, 0, 40976, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_COMPLETED Audience with the Warchief');
-- Audience with the Warchief (40976) Fate of the Horde (40522) rewarded or objective (100934 (for both of these quests), learn the fate of the horde) done till
-- Demons Among Them (40983) rewarded or objective done (100866, warn Warchief Sylvanas) or Demons Among Us (40607) rewarded or objective done (112731, speak to Allari)
-- phase_definitions entry: 3 and phaseId: 7531
-- old:
-- INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
-- (23, 14, 3, 0, 1, 14, 0, 40522, 0, 0, 1, 0, '', 'start BI - 3'),
-- (23, 14, 3, 0, 1, 41, 0, 40522, 100934, 1, 0, 0, '', 'start BI - 3'),
-- (23, 14, 3, 0, 2, 8, 0, 40522, 0, 0, 0, 0, '', 'start BI - 3'),
-- (23, 14, 3, 0, 2, 14, 0, 40607, 0, 0, 0, 0, '', 'start BI - 3'),
-- (23, 14, 3, 0, 3, 9, 0, 40607, 0, 0, 0, 0, '', 'start BI - 3'),
-- (23, 14, 3, 0, 3, 41, 0, 40607, 112731, 1, 1, 0, '', 'start BI - 3'),
-- (23, 14, 3, 0, 4, 8, 0, 40605, 0, 0, 1, 0, '', NULL),
-- (23, 14, 3, 0, 4, 8, 0, 40760, 0, 0, 0, 0, '', NULL);
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 14 AND SourceEntry = 3;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 14, 3, 0, 1, 8, 0, 40976, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Audience with the Warchief'),
(23, 14, 3, 0, 1, 41, 0, 40983, 100866, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_OBJECTIVE_DONE Demons Among Them'),
(23, 14, 3, 0, 1, 8, 0, 40983, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Demons Among Them'),
(23, 14, 3, 0, 2, 41, 0, 40976, 100934, 0, 0, 0, '', 'Legion Start Questline - QUEST_OBJECTIVE_DONE Audience with the Warchief'),
(23, 14, 3, 0, 2, 41, 0, 40983, 100866, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_OBJECTIVE_DONE Demons Among Them'),
(23, 14, 3, 0, 2, 8, 0, 40983, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Demons Among Them'),
(23, 14, 3, 0, 3, 8, 0, 40522, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Fate of the Horde'),
(23, 14, 3, 0, 3, 41, 0, 40607, 112731, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_OBJECTIVE_DONE Demons Among Us'),
(23, 14, 3, 0, 3, 8, 0, 40607, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Demons Among Us'),
(23, 14, 3, 0, 4, 41, 0, 40522, 100934, 0, 0, 0, '', 'Legion Start Questline - QUEST_OBJECTIVE_DONE Fate of the Horde'),
(23, 14, 3, 0, 4, 41, 0, 40607, 112731, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_OBJECTIVE_DONE Demons Among Us'),
(23, 14, 3, 0, 4, 8, 0, 40607, 0, 0, 1, 0, '', 'Legion Start Questline - Not QUEST_REWARDED Demons Among Us');
UPDATE phase_definitions SET phaseId = '7531' WHERE zoneId = 14 AND entry = 3;
UPDATE creature SET PhaseId = '7531' WHERE zoneId = 14 AND `map` = 1 AND PhaseId LIKE "%7531%";
-- Demons Among Them (40983) rewarded or objective done (100866, warn Warchief Sylvanas) or Demons Among Us (40607) rewarded or objective done (112731, speak to Allari) till
-- not none In the Blink of an Eye (44663) phase_definitions entry: 1 and phaseId 7422
-- old:
-- INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
-- (23, 14, 1, 0, 0, 40, 0, 1447, 0, 0, 0, 0, '', 'scene comp. 1447');
-- (23, 14, 1, 0, 0, 14, 0, 40605, 0, 0, 0, 0, '', 'and not rew 40607');
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 23 AND SourceGroup = 14 AND SourceEntry = 1;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(23, 14, 1, 0, 1, 41, 0, 40983, 100866, 0, 0, 0, '', 'Legion Start Questline - QUEST_OBJECTIVE_DONE Demons Among Them'),
(23, 14, 1, 0, 1, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 14, 1, 0, 2, 8, 0, 40983, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Demons Among Them'),
(23, 14, 1, 0, 2, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 14, 1, 0, 3, 41, 0, 40607, 112731, 0, 0, 0, '', 'Legion Start Questline - QUEST_OBJECTIVE_DONE Demons Among Us'),
(23, 14, 1, 0, 3, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye'),
(23, 14, 1, 0, 4, 8, 0, 40607, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_REWARDED Demons Among Us'),
(23, 14, 1, 0, 4, 14, 0, 44663, 0, 0, 0, 0, '', 'Legion Start Questline - QUEST_NONE In the Blink of an Eye');
UPDATE phase_definitions SET phaseId = '7422' WHERE zoneId = 14 AND entry = 1;
UPDATE creature SET PhaseId = '7422' WHERE zoneId = 14 AND `map` = 1 AND PhaseId LIKE "%7422%";

-- make The Battle for Broken Shore mandatory again
UPDATE quest_objectives SET Amount = 1 WHERE QuestID = 40518;

-- use the default phase mask for all horde intro questline phases (that way we keep all the normal Durotar spawns)
UPDATE phase_definitions SET phasemask = 1 WHERE zoneId = 14 AND entry IN (1, 2, 3, 4);
UPDATE creature SET phaseMask = 1 WHERE PhaseId IN ("7422", "7531", "7425", "7554 7553", "7554 7553 7425") AND zoneId = 14;
UPDATE gameobject SET phaseMask = 1 WHERE phaseMask = 2 AND zoneId = 14;

-- spawn questgivers/enders for demon hunter questline in Stormwind/Alliance
-- not bothering with phasing as it is a single npc and we have bigger things to worry about
-- TODO: fix phasing
DELETE FROM creature WHERE guid IN (1000002);
INSERT INTO creature (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(1000002, 97296, 1, 1637, 1637, 1, 2, '1230', 0, 1, 1463.74, -4420.06, 25.4536, 0.0972591, 300, 0, 0, 9145554, 5500000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- High Overlord Saurfang, also give Demon Hunters the option to enter the hold
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup = 19116 AND SourceEntry = 0 AND ElseGroup = 1;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(15, 19116, 0, 0, 1, 9, 0, 40976, 0, 0, 0, 0, '', 'QUEST_TAKEN Audience with the Warchief'),
(15, 19116, 0, 0, 1, 28, 0, 40976, 0, 0, 1, 0, '', 'Not QUEST_COMPLETED Audience with the Warchief');

-- add gossip menu (including conditions for when to use which) to legion start questline High Overlord Saurfang
DELETE FROM gossip_menu WHERE Entry = 19116 AND TextID = 28253;
INSERT INTO gossip_menu (Entry, TextID) VALUES (19116, 28253);
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 19116 AND SourceEntry IN (28026, 28253);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(14, 19116, 28026, 0, 0, 15, 0, 2048, 0, 0, 1, 0, '', 'Legion Start Questline - Saurfang None Demon Hunter Gossip Menu'),
(14, 19116, 28253, 0, 0, 15, 0, 2048, 0, 0, 0, 0, '', 'Legion Start Questline - Saurfang Demon Hunter Gossip Menu');

-- Remove Quest deprecated Audience with the Warchief quest objective:
-- Speak to the Warchief
-- INSERT INTO `quest_objectives` (`ID`, `QuestID`, `Type`, `StorageIndex`, `ObjectID`, `Amount`, `Flags`, `Flags2`, `TaskStep`, `Description`, `VerifiedBuild`) VALUES (281538, 40976, 0, 3, 100985, 1, 2, 0, 0, NULL, 21287);
-- INSERT INTO `quest_objectives_locale` (`ID`, `locale`, `QuestId`, `StorageIndex`, `Description`, `VerifiedBuild`) VALUES (281538, 'enUS', 40976, 3, 'Speak to the Warchief', 21287);
DELETE FROM quest_objectives WHERE ID = 281538 AND QuestID = 40976;
DELETE FROM quest_objectives_locale WHERE ID = 281538 AND QuestId = 40976;
-- fix quest ender
UPDATE creature_questender SET id = 100873 WHERE id = 100866 AND quest = 40976;
-- abuse Illidari Enforcer spawns to run a script that completes the Second Sight quest when Spectral Sight gets used
UPDATE creature_template SET ScriptName = 'npc_100874_illidari_enforcer_dh_questline' WHERE entry = 100874;

-- add gossip menu (including conditions for when to use which) to legion start questline Sylvanas
DELETE FROM gossip_menu WHERE Entry = 19176 AND TextID = 28256;
INSERT INTO gossip_menu (Entry, TextID) VALUES (19176, 28256);
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 14 AND SourceGroup = 19176 AND SourceEntry IN (28107, 28256);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(14, 19176, 28107, 0, 0, 15, 0, 2048, 0, 0, 1, 0, '', 'Legion Start Questline - Sylvanas None Demon Hunter Gossip Menu'),
(14, 19176, 28256, 0, 0, 15, 0, 2048, 0, 0, 0, 0, '', 'Legion Start Questline - Sylvanas Demon Hunter Gossip Menu');

-- add menu options for demon hunter quest
DELETE FROM gossip_menu_option WHERE MenuID = 19176 AND OptionIndex = 1;
INSERT INTO gossip_menu_option (MenuID, OptionIndex, OptionText, BoxText, OptionBroadcastTextID) VALUES
(19176, 1, 'This cannot wait. There are demons among your ranks. Let me show you.', '', 122662);

DELETE FROM conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup = 19176 AND SourceEntry = 1;
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES
(15, 19176, 1, 0, 0, 9, 0, 40983, 0, 0, 0, 0, '', 'Legion Start Questline - Taken Quest (40983) Demons Among Them'),
(15, 19176, 1, 0, 0, 41, 0, 40983, 100866, 1, 1, 0, '', 'Legion Start Questline - Not yet warned Sylvanas for Quest (40983) Demons Among Them');

DELETE FROM smart_scripts WHERE entryorguid = 100866 AND id IN (3, 4);
INSERT INTO smart_scripts (entryorguid, id, link, event_type, event_param1, event_param2, action_type, action_param1, target_type, `comment`) VALUES
(100866, 3, 4, 62, 19176, 1, 85, 226700, 7, 'At gossip select, cast Demon Attack scene'),
(100866, 4, 0, 61, 0, 0, 33, 100866, 7, 'At gossip select, kill credit Sylvanas');

-- fix quest ender (only should be Sylvanas)
DELETE FROM creature_questender WHERE id = 101006 AND quest = 40983;

-- remove deprecated quest
DELETE FROM creature_queststarter WHERE id = 95234 AND quest = 44091;
DELETE FROM creature_queststarter WHERE id = 100973 AND quest = 43635;
