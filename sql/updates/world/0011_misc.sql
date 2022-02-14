-- remove xp gain from NPC that is too easy to kill
UPDATE creature_template SET flags_extra = 64 WHERE entry = 110032;

-- for quest 40339 (Candle of Command) also remove questgiver aura when the vehicle despawns
DELETE FROM smart_scripts WHERE entryorguid = 99724 AND source_type = 0 AND id = 10;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(99724, 0, 10, 0, 28, 0, 100, 1, 0, 0, 0, 0, 45, 4, 4, 0, 0, 0, 0, 19, 100191, 0, 0, 0, 0, 0, 0, 'L - SD');
DELETE FROM smart_scripts WHERE entryorguid = 100191 AND source_type = 0 AND id IN (3, 4);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(100191, 0, 3, 0, 38, 0, 100, 0, 4, 4, 0, 0, 67, 4, 500, 500, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'UD - U'),
(100191, 0, 4, 0, 59, 0, 100, 1, 4, 0, 0, 0, 28, 196235, 0, 0, 0, 0, 0, 18, 10, 0, 0, 0, 0, 0, 0, 'UD - U');

-- fix loot from item 137209 (armor enhancement token) with spell loot for spell 212000
DELETE FROM spell_loot_template WHERE entry = 212000;
INSERT INTO spell_loot_template (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`) VALUES
(212000, 147348, 60, 1, 1, 1, 1),
(212000, 147349, 30, 1, 1, 1, 1),
(212000, 147350, 10, 1, 1, 1, 1);
