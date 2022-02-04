-- remove second LFG queue, already gets triggered by end of scene
-- INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (227058, 'spell_q42740');
DELETE FROM spell_script_names WHERE spell_id = 227058 AND ScriptName = 'spell_q42740';

-- disable custom event (modified uwow Legion Invasion - Westfall/Barrens)
UPDATE game_event SET start_time = '0000-00-00 00:00:00', end_time = '0000-00-00 00:00:00', length = '1' WHERE eventEntry = 811;

-- disable rep gain from always spawned world quest npc 92128 (Felskorn Pilferer)
-- was: INSERT INTO creature_onkill_reputation (`creature_id`, `RewFaction`, `RewValue`, `MaxStanding`) VALUES (92128, 1948, 75, 7);
DELETE FROM creature_onkill_reputation WHERE creature_id = 92128 AND RewFaction = 1948;

-- fix droprate of greenies in Legion zones
-- was for world zones: UPDATE zone_loot_template SET ChanceOrQuestChance = 0.05 WHERE item = 463 AND entry IN (7334, 7502, 7503, 7541, 7543, 7558, 7578, 7637, 7656, 7731, 8392);
-- was for dungeons: UPDATE zone_loot_template SET ChanceOrQuestChance = 0.5 WHERE item = 463 AND entry IN (7546, 7588, 7672, 7673, 7787, 7796, 7805, 7812, 7855, 8017, 8025, 8026, 8040, 8079, 8262, 8277, 8443, 8570, 8596, 8625, 8646);
UPDATE zone_loot_template SET ChanceOrQuestChance = 2 WHERE item = 463 AND entry IN (7334, 7502, 7503, 7541, 7543, 7558, 7578, 7637, 7656, 7731, 8392);
UPDATE zone_loot_template SET ChanceOrQuestChance = 4 WHERE item = 463 AND entry IN (7546, 7588, 7672, 7673, 7787, 7796, 7805, 7812, 7855, 8017, 8025, 8026, 8040, 8079, 8262, 8277, 8443, 8570, 8596, 8625, 8646);
