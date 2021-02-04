-- transformation spell is triggered by Brood Queen Tyranna's Queen's Bite ability and is not self cast
DELETE FROM creature_template_spell WHERE entry = 97244 AND spell = 197505;
DELETE FROM creature_template_spell WHERE entry = 97959 AND spell = 197505;
DELETE FROM creature_template_spell WHERE entry = 97962 AND spell = 197598;
DELETE FROM creature_template_spell WHERE entry = 98712 AND spell = 197598;

-- add the required spell scripts for quest 38728 - The Keystone
DELETE FROM spell_script_names WHERE spell_id IN (197486, 208121, 197523, 197505, 197598);
INSERT INTO spell_script_names (spell_id, ScriptName) VALUES
(197486, 'spell_legion_197486'),
(208121, 'spell_legion_208121'),
(197523, 'spell_legion_197523'),
(197505, 'spell_legion_197505_197598'),
(197598, 'spell_legion_197505_197598');

-- Demon Hunter questline - Vault of the Wardens
-- translations for the quest text
DELETE FROM quest_request_items_locale WHERE ID = 38669 AND Locale = 'enUS';
INSERT INTO quest_request_items_locale (ID, Locale, CompletionText) VALUES
(38669, 'enUS', 'We must hurry.');
DELETE FROM quest_offer_reward_locale WHERE ID = 38669 AND Locale = 'enUS';
INSERT INTO quest_offer_reward_locale (ID, Locale, OfferRewardText) VALUES
(38669, 'enUS', 'Good. You spent many years in stasis, but I see that your senses have not dulled.\r\n\r\n<Maiev eyes your weapons warily.>\r\n\r\nRaise your hand against me, demon hunter, and you will find out why I was your master\'s jailor.');

-- fix gossip options for Kayn and Altruis
UPDATE gossip_menu_option SET ActionMenuID = 19234 WHERE OptionBroadcastTextID = 104591;
UPDATE gossip_menu_option SET ActionMenuID = 19233 WHERE OptionBroadcastTextID = 104593;

-- translations for talking npc's
UPDATE creature_text SET BroadcastTextID = 101976 WHERE Entry = 92776 AND GroupID = 4;
DELETE FROM creature_text WHERE Entry = 92776 AND GroupID = 13;

-- visuals for the quest - Beam Me Up (39684)
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry IN (191933, 191934, 191935, 196363, 197724);
INSERT INTO conditions (SourceTypeOrReferenceId, SourceGroup, SourceEntry, SourceId, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorTextId, ScriptName, `Comment`) VALUES 
(13, 1, 191933, 0, 0, 31, 0, 3, 950005, 0, 0, 0, '', 'DH - Vault of the Wardens - Glazer target Beam dummy 1'),
(13, 1, 191934, 0, 0, 31, 0, 3, 950006, 0, 0, 0, '', 'DH - Vault of the Wardens - Beam dummy 1 target Beam dummy 2'),
(13, 1, 191935, 0, 0, 31, 0, 3, 950007, 0, 0, 0, '', 'DH - Vault of the Wardens - Beam dummy 2 target Beam dummy 3'),
(13, 1, 196363, 0, 0, 31, 0, 3, 950008, 0, 0, 0, '', 'DH - Vault of the Wardens - Beam dummy 3 target Beam dummy 4'),
(13, 1, 197724, 0, 0, 31, 0, 3, 950009, 0, 0, 0, '', 'DH - Vault of the Wardens - Beam dummy 4 target Beam dummy 5'),
(13, 1, 197724, 0, 0, 1, 0, 197724, 0, 0, 1, 0, '', 'DH - Vault of the Wardens - Beam dummy 4 target Beam dummy 5 without aura already');

DELETE FROM smart_scripts WHERE source_type = 0 AND entryorguid IN (96680, -365928, -365929, 950005, 950006, 950007, 950008);
INSERT INTO smart_scripts (entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o, `comment`) VALUES
(-365928, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 191933, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast eye beam spell on spawn'),
(-365928, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 191915, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast focusing on spawn'),
(-365928, 0, 2, 0, 1, 0, 100, 0, 2000, 2000, 10000, 12000, 11, 196460, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "OOC - Cast Lingering Gaze"),
(-365928, 0, 3, 0, 1, 0, 100, 0, 3000, 4000, 20000, 21000, 11, 196462, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "OOC - Cast Pulse"),
(-365928, 0, 4, 0, 60, 0, 100, 0, 1, 10, 1000, 1000, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Prevent Combat'),
(950005, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 191934, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast eye beam spell on spawn'),
(950006, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 191935, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast eye beam spell on spawn'),
(950007, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 196363, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast eye beam spell on spawn'),
(950008, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 197724, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast eye beam spell on spawn'),
(950008, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 197724, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast eye beam spell on spawn');

UPDATE creature_template SET AIName = 'SmartAI' WHERE entry IN (96680, 950005, 950006, 950007, 950008);

-- set phaseMask for the creatures/gameobjects that rotating the mirror affects
UPDATE gameobject SET phaseMask = 2 WHERE guid = 131228;
UPDATE gameobject SET phaseMask = 4 WHERE guid = 131227;
UPDATE creature SET phaseMask = 2 WHERE guid IN (366793, 365928, 367023, 366795, 366797);
UPDATE creature SET phaseMask = 4 WHERE guid IN (366811, 365929, 367024, 366810, 366812);
-- put some of the dummies in all 3 phase masks 1|2|4 to make sure they can be targeted
UPDATE creature SET phaseMask = 1|2|4 WHERE guid IN (367019, 367020, 367021, 367022);

-- set phaseMask in phase definitions to correspond to the spawn changes
-- the relevant conditions to activate phase definitions already exist
UPDATE phase_definitions SET phasemask = 1|2 WHERE zoneId = 7814 AND entry = 6;
UPDATE phase_definitions SET phasemask = 1|4 WHERE zoneId = 7814 AND entry = 7;

-- outro event for Beam Me Up quest
-- summon only visible to summoning player
DELETE FROM event_scripts WHERE id = 46447;
INSERT INTO event_scripts (id, delay, command, datalong, datalong2, dataint, x, y, z, o) VALUES
(46447, 0, 10, 102391, 29000, 1, 4419.76, -661.37, 117.233, 5.55236);
-- delete static spawns, spawning them as needed on player completing the quest
-- INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
-- (366810, 96656, 1468, 7814, 7868, 1, 4, '5976 5443 5407 5401 5157', 0, 0, 4445.42, -679.495, 117.316, 5.73285, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (366811, 102391, 1468, 7814, 7868, 1, 4, '5976 5443 5407 5401 5157', 0, 0, 4448.68, -675.276, 117.149, 6.22189, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
-- (366812, 96656, 1468, 7814, 7868, 1, 4, '5976 5443 5407 5401 5157', 0, 0, 4453.46, -680.844, 117.284, 4.0283, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid IN (366810, 366811, 366812);

DELETE FROM script_waypoint WHERE entry = 102391;
INSERT INTO script_waypoint (entry, pointid, location_x, location_y, location_z, waittime, point_comment) VALUES
(102391, 1, 4426.13, -660.673, 117.15, 0, ''),
(102391, 2, 4440.76, -666.145, 117.15, 0, ''),
(102391, 3, 4448.49, -675.222, 117.15, 6000, 'says 1 line here'),
(102391, 4, 4448.57, -614.155, 117.172, 0, ''),
(102391, 5, 4443.28, -608.303, 119.441, 0, ''),
(102391, 6, 4442.39, -602.86, 121.627, 0, ''),
(102391, 7, 4447.28, -596.071, 121.616, 0, ''),
(102391, 8, 4450.68, -580.237, 126.114, 0, ''),
(102391, 9, 4450.99, -542.229, 126.181, 0, '');

-- every 6 seconds 1-3x 196504 (triggered by 196460), which should go to random positions (but those 1-3 to nearby positions)
-- the area formed should slow, this should be the areatrigger from 196503 which casts 196502
DELETE FROM areatrigger_actions WHERE entry = 5269 AND customEntry = 9991;
INSERT INTO areatrigger_actions (entry, customEntry, id, moment, actionType, targetFlags, spellId, maxCharges, hasAura, hasAura2, hasAura3, hasspell, chargeRecoveryTime, scaleStep, scaleMin, scaleMax, scaleVisualUpdate, hitMaxCount, amount, onDespawn, auraCaster, minDistance, `comment`) VALUES
(5269, 9991, 0, 1, 0, 8, 196502, 0, -196502, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 'Demon Hunter Quest, Beam Me Up, Lingering Gaze Apply'),
(5269, 9991, 1, 2, 1, 8, 196502, 0, 196502, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 'Demon Hunter Quest, Beam Me Up, Lingering Gaze Remove');

-- also casts pulse, 194853, how often though? and what happens when it gets used up?
-- should be triggered by 196462
DELETE FROM spell_script_names WHERE spell_id IN (196460, 196462);
INSERT INTO spell_script_names (spell_id, ScriptName) VALUES
(196460, 'spell_196460'),
(196462, 'spell_196462');
