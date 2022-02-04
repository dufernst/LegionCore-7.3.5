-- enable map 1669 Argus
-- enable map 1712 Antorus the Burning Throne
-- enable map 1753 Seat of the Triumvirate
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1669, 1712, 1753);

-- enable quest 47221/47835/48506/48507 The Hand of Fate (starting quest of 7.3 content)
-- enable quest 47867/47222 Two If By Sea (followup quest from: The Hand of Fate)
-- enable quest 47223 Light's Exodus (here Alliance/Horde converges, block just to be sure)
DELETE FROM disables WHERE sourceType = 1 AND entry IN (47221, 47835, 48506, 48507, 47867, 47222, 47223);

-- enable Invasaion Point Argus Events
UPDATE game_event SET start_time = '2018-08-06 07:57:00' WHERE eventEntry = 180;
UPDATE game_event SET start_time = '2018-08-06 09:57:00' WHERE eventEntry = 181;
UPDATE game_event SET start_time = '2018-08-06 23:57:00' WHERE eventEntry = 182;
UPDATE game_event SET start_time = '2018-08-07 01:57:00' WHERE eventEntry = 183;
UPDATE game_event SET start_time = '2018-08-06 15:57:00' WHERE eventEntry = 184;
UPDATE game_event SET start_time = '2018-08-06 21:57:00' WHERE eventEntry = 185;
UPDATE game_event SET start_time = '2018-08-06 19:57:00' WHERE eventEntry = 186;
UPDATE game_event SET start_time = '2018-08-06 17:57:00' WHERE eventEntry = 187;
UPDATE game_event SET start_time = '2018-08-06 13:57:00' WHERE eventEntry = 188;
UPDATE game_event SET start_time = '2018-08-06 11:57:00' WHERE eventEntry = 189;
UPDATE game_event SET start_time = '2018-08-07 03:57:00' WHERE eventEntry = 190;
UPDATE game_event SET start_time = '2018-08-07 05:57:00' WHERE eventEntry = 191;
