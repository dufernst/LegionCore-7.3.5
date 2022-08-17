-- disable map 1669 Argus
-- disable map 1712 Antorus the Burning Throne
-- disable map 1753 Seat of the Triumvirate
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1669, 1712, 1753);
INSERT INTO disables (`sourceType`, `entry`, `comment`) VALUES
(2, 1669, '7.3 content - Argus'),
(2, 1712, '7.3 content - Antorus the Burning Throne'),
(2, 1753, '7.3 content - Seat of the Triumvirate');

-- disable quest 47221/47835/48506/48507 The Hand of Fate (starting quest of 7.3 content)
-- disable quest 47867/47222 Two If By Sea (followup quest from: The Hand of Fate)
-- disable quest 47223 Light's Exodus (here Alliance/Horde converges, block just to be sure)
-- disable quest 48639/48642 Emissary quests from Army of the Light and Argussian Reach (7.3 factions) 
DELETE FROM disables WHERE sourceType = 1 AND entry IN (47221, 47835, 48506, 48507, 47867, 47222, 47223, 48639, 48642);
INSERT INTO disables (`sourceType`, `entry`, `comment`) VALUES
(1, 47221, '7.3 content - The Hand of Fate'),
(1, 47835, '7.3 content - The Hand of Fate'),
(1, 48506, '7.3 content - The Hand of Fate'),
(1, 48507, '7.3 content - The Hand of Fate'),
(1, 47867, '7.3 content - Two If By Sea'),
(1, 47222, '7.3 content - Two If By Sea'),
(1, 47223, '7.3 content - Lights Exodus'),
(1, 48639, '7.3 content - Army of the Light Emissary quest'),
(1, 48642, '7.3 content - Argussian Reach Emissary quest');

-- disable Invasaion Point Argus Events
UPDATE game_event SET start_time = '2035-07-04 17:00:00' WHERE eventEntry IN(180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191);

-- set the minimum item level to 0 (scaling with current gear) for pvp world quests and dungeon world quests
UPDATE world_quest_template SET MinItemLevel = 0 WHERE ZoneId = 0 AND QuestInfoId IN (113, 137);
