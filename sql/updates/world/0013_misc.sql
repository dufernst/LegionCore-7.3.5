-- remove conditions for summoning Nightbane in Karazhan (
-- INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
-- (22, 1, 194092, 1, 1, 8, 0, 9644, 0, 0, 0, 0, '', 'SAI only if quest rewarded'),
-- (22, 1, 194092, 1, 0, 9, 0, 9644, 0, 0, 0, 0, '', 'SAI only if quest accepted');
DELETE FROM conditions WHERE SourceTypeOrReferenceId = 22 AND SourceGroup = 1 AND SourceEntry = 194092 AND SourceId = 1;

-- Fix drop chance of quest item and of Horde version of Pit Lord's Cuffs (Demon Hunter starting zone)
UPDATE creature_loot_template SET ChanceOrQuestChance = -100 WHERE item = 129957 AND entry = 98986;
UPDATE creature_loot_template SET ChanceOrQuestChance = 90.8 WHERE item = 133313 AND entry = 97370;

-- Fix drop chance of Nightbane's quest chain items and of Fiery Warhorse's Reins (Old Karazhan)
UPDATE creature_loot_template SET ChanceOrQuestChance = -100 WHERE item = 23933 AND entry = 16524;
UPDATE creature_loot_template SET ChanceOrQuestChance = -100 WHERE item = 25462 AND entry = 16807;
UPDATE creature_loot_template SET ChanceOrQuestChance = -100 WHERE item = 24139 AND entry = 17225;
UPDATE creature_loot_template SET ChanceOrQuestChance = 1 WHERE item = 30480 AND entry = 16151;

-- Change drop chance of Cracked Egg's contents to wowhead's values (Sholazar Basin)
UPDATE item_loot_template SET ChanceOrQuestChance = 31.5 WHERE item = 44722 AND entry = 39883;
UPDATE item_loot_template SET ChanceOrQuestChance = 19 WHERE item = 39898 AND entry = 39883;
UPDATE item_loot_template SET ChanceOrQuestChance = 18 WHERE item = 39899 AND entry = 39883;
UPDATE item_loot_template SET ChanceOrQuestChance = 18 WHERE item = 39896 AND entry = 39883;
UPDATE item_loot_template SET ChanceOrQuestChance = 7.5 WHERE item = 44721 AND entry = 39883;
UPDATE item_loot_template SET ChanceOrQuestChance = 6 WHERE item = 44707 AND entry = 39883;

-- Add title for new combined Algalon achievement in Ulduar
DELETE FROM achievement_reward WHERE entry = 12399;
INSERT INTO achievement_reward (entry, title_A, title_H, genderTitle, learnSpell, castSpell, item, sender, subject, text, ScriptName) VALUES
(12399, 165, 165, 0, 0, 0, 0, 0, '', '', '');

-- Fix 2 old Blasted Lands spawns that now seem to be phased only in the new version
-- phaseMask was 2, but they should at least be spawned in 1 (the old Blasted Lands phase)
UPDATE creature SET phaseMask = 3 WHERE id IN (42299, 16841);

-- Hackfix: Make Commander Althea Ebonlocke (entry: 107837) neutral (faction 7, was 14) so she does not kill low lvl players questing there
UPDATE smart_scripts SET action_param1 = 7 WHERE entryorguid = 107837 AND source_type = 0 AND link = 0 AND id IN (2, 5);
