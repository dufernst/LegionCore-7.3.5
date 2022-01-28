-- fix Lord Maxwell Tyrosus text when paladins enter Legion Dalaran for the first time
UPDATE creature_text SET Text = 'My sincere apologies for the insistence, $p, but we must speak immediately.', BroadcastTextID = '114359' WHERE Entry = 92909 AND GroupID = 0;
DELETE FROM creature_text WHERE Entry = 92909 AND GroupID = 1;
INSERT INTO creature_text (`Entry`, `GroupID`, `Text`, `Type`, `Probability`, `BroadcastTextID`) VALUES
(92909, 1, 'Excellent. Meet me at Krasus\' Landing when you are ready.', 12, 100, 114364);

-- fix drop (rate) of Artifact Power giving items
DELETE FROM item_loot_template WHERE entry = 140591 AND item = 141701;
-- even though world quest creatures should drop this, better to remove this hack, because now all npcs in Legion drop it
-- INSERT INTO world_loot_template VALUES (6, 138782, 2, 0, 0, 1, 1);
DELETE FROM world_loot_template WHERE entry = 6 AND item = 138782;

-- add missing Patchwerk spawn for quest 13166 (The Battle For The Ebon Hold)
DELETE FROM creature WHERE guid = 1000012;
INSERT INTO creature (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(1000012, 31099, 0, 139, 4281, 1, 256, '', 0, 0, 2460.37, -5592.69, 414.122, 3.7114, 300, 0, 0, 224300, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- info for stormheim horde intro quest quest 39698 Making the Rounds Sylvanas for this quest is 96686
-- salute spell is 220291, spell bar override spell is 220293 (should be cast by player on player when she talks about Nathanos training new rangers
DELETE FROM smart_scripts WHERE entryorguid = 96686 AND source_type = 9;
INSERT INTO smart_scripts VALUES
(96686, 9, 0, 0, 0, 0, 100, 1, 15000, 15000, 0, 0, 53, 0, 966861, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'TS - SWP'),
(96686, 9, 1, 0, 0, 0, 100, 1, 0, 0, 0, 0, 1, 3, 6000, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 2, 0, 0, 0, 100, 1, 6000, 6000, 0, 0, 1, 4, 8000, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 3, 0, 0, 0, 100, 1, 12000, 12000, 0, 0, 1, 5, 8000, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 4, 0, 0, 0, 100, 1, 0, 0, 0, 0, 85, 220293, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Override salute spell on player spellbar'),
(96686, 9, 5, 0, 0, 0, 100, 1, 15000, 15000, 0, 0, 53, 0, 966862, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'TS - SWP'),
(96686, 9, 6, 0, 0, 0, 100, 1, 0, 0, 0, 0, 1, 6, 10000, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 7, 0, 0, 0, 100, 1, 12000, 12000, 0, 0, 1, 7, 6000, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 8, 0, 0, 0, 100, 1, 6000, 6000, 0, 0, 1, 8, 6000, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 9, 0, 0, 0, 100, 1, 6000, 6000, 0, 0, 45, 1, 1, 0, 0, 0, 0, 10, 146728233, 96689, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 10, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 1, 9, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'TS - ST'),
(96686, 9, 11, 0, 0, 0, 100, 1, 6000, 6000, 0, 0, 33, 96689, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'TS - KC'),
(96686, 9, 12, 0, 0, 0, 100, 1, 0, 0, 0, 0, 11, 210262, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'TS - CS'),
(96686, 9, 13, 0, 0, 0, 100, 1, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'TS - D');

-- add some missing spawns for The Splintered Fleet scenario (spawnMask 4096?)
DELETE FROM creature WHERE guid BETWEEN 1000004 AND 1000011;
INSERT INTO creature (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(1000004, 94517, 1475, 7658, 7658, 4096, 1, '', 0, 0, 4623.87, 2874.82, 7.88567, 4.74059, 300, 0, 0, 519634, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000005, 94421, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4620.54, 2854.03, 7.99947, 1.55547, 300, 0, 0, 2078534, 220000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000006, 94420, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4622.91, 2859.12, 7.86001, 4.72847, 300, 0, 0, 15589006, 2420000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000007, 94419, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4623.04, 2853.6, 7.86223, 1.58486, 300, 0, 0, 1150587008, 4400000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000008, 93490, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4617.12, 2856.57, 7.96929, 0.0101309, 300, 0, 0, 2598168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000009, 93490, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4629.4, 2859.09, 7.96559, 3.53265, 300, 0, 0, 2598168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000010, 93490, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4658.45, 2868.22, 23.9151, 5.88885, 300, 0, 0, 2598168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(1000011, 93486, 1475, 7658, 7683, 4096, 1, '', 0, 1, 4654.66, 2863.35, 23.2823, 0.0444242, 300, 0, 0, 4157068, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- by default nobody should be on this vehicle (Nathanos' pet) was 95309
UPDATE vehicle_template_accessory SET accessory_entry = 0 WHERE EntryOrAura = 94517 AND seat_id = 0;
-- was 3 (walking and swimming) but it's a flying pet :|, so lets add 4 (air)
UPDATE creature_template SET InhabitType = 7 WHERE entry = 94517;
-- add/correct Sylvanas text, add/correct Nathanos text
UPDATE creature_text SET Text = 'Organize our defenses beginning with the rear. I want bat-riders in the air and catapults at the ready!' WHERE Entry = 94419 AND GroupID = 3;
DELETE FROM creature_text WHERE Entry = 94419 AND GroupID IN (1, 2);
INSERT creature_text (`Entry`, `GroupID`, `Text`, `Type`, `Probability`, `Emote`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `comment`) VALUES
(94419, 1, 'The Alliance will be upon us soon, and we have nowhere to run.', 12, 100, 0, 52948, 96255, 11865, 11865, ''),
(94419, 2, 'Do not question me, Nathanos. Now go - make for the Eternity, and bring this hero with you.', 12, 100, 2, 52949, 96260, 11865, 11865, '');
UPDATE creature_text SET Text = 'Yes, of course. Come along... "hero."' WHERE Entry = 94420 AND GroupID = 3;
DELETE FROM creature_text WHERE Entry = 94420 AND GroupID = 1;
INSERT creature_text (`Entry`, `GroupID`, `Text`, `Type`, `Probability`, `Emote`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `comment`) VALUES
(94420, 1, 'My Queen, your safety must be our first concern. Surely we can leave a few ships behind to-', 12, 100, 1, 51737, 96262, 11865, 11865, '');
-- set Sylvanas to have Gossip so we can catch the event with SmartAI
UPDATE creature_template SET npcflag = 1, AIName = 'SmartAI' WHERE entry = 94419;

-- Sylvanas (94419) 1 (4 sec) 96255 The Alliance will be upon us soon, and we have nowhere to run.
-- Sylvanas (94419) 3 (5 sec) Organize our defenses beginning with the rear. I want bat-riders in the air and catapults at the ready!
-- Nathanos (94420) 1 (6 sec) 96262 My Queen, your safety must be our first concern. Surely we can leave a few ships behind to-
-- Sylvanas (94419) 2 (6 sec) 96260 Do not question me, Nathanos. Now go - make for the Eternity, and bring this hero with you.
-- Nathanos (94420) 3 (4 sec) 96263 Yes, of course. Come along... "hero."
-- Nathanos after 3 sec walk to Bloodwing and enter vehicle slot 1, complete stage 1, enable interaction with Bloodwing
-- on mount fly path
DELETE FROM smart_scripts WHERE entryorguid = 94419 AND source_type IN (0, 9);
INSERT INTO smart_scripts (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(94419, 0, 0, 1, 64, 0, 100, 0, 0, 0, 0, 0, 80, 94419, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'on gossip hello - start scenario dialogue'),
(94419, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'on gossip hello - close gossip menu'),
(94419, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 83, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'on gossip hello - remove gossip npc flag'),
(94419, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'sylvanas and nathanos conversation 1'),
(94419, 9, 1, 0, 0, 0, 100, 0, 4500, 4500, 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'sylvanas and nathanos conversation 2'),
(94419, 9, 2, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 84, 1, 0, 0, 0, 0, 0, 10, 1000006, 94420, 0, 0, 0, 0, 0, 'sylvanas and nathanos conversation 3'),
(94419, 9, 3, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'sylvanas and nathanos conversation 4'),
(94419, 9, 4, 0, 0, 0, 100, 0, 6000, 6000, 0, 0, 84, 3, 0, 0, 0, 0, 0, 10, 1000006, 94420, 0, 0, 0, 0, 0, 'sylvanas and nathanos conversation 5'),
(94419, 9, 5, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 33, 113320, 0, 0, 0, 0, 0, 18, 50, 0, 0, 0, 0, 0, 0, 'kill credit for quest after conversation'),
(94419, 9, 6, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 62, 1220, 0, 0, 0, 0, 0, 18, 50, 0, 0, 4110.959, 2944.6587, 28.25, 0, 'teleport player after conversation');

-- fix credit for taking the portal to the Skyfire for quest 38035 (A Royal Summons)
UPDATE quest_objectives SET ObjectID = 97549 WHERE ID = 280241;
-- and correct spell target position (now slightly lower so we don't end up on top of the skyfire
-- was: INSERT INTO spell_target_position VALUES (192465, 0, -8473.82, 1403.17, 213.18, 3.26342);
UPDATE spell_target_position SET target_position_x = -8479.7, target_position_y = 1402.06, target_position_z = 205 WHERE id = 192465;
