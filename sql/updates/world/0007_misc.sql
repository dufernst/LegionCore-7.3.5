-- replace Russian text for quest 42421 (The Nightfallen)
UPDATE quest_request_items SET CompletionText = 'Assist us, and we will reward you.' WHERE ID = 42421;
-- replace Russian text for quest 41989 (Blood of My Blood)
UPDATE page_text SET Text = 'Ran\'thos Lunastre$B$BHead of House Lunastre. Father of Ly\'leth and Anarys.$B$BDied in honorable service to Grand Magistrix Elisande.' WHERE ID = 5279;
-- replace Russian text for random statue 243559 (Statue of Liftbrul)
UPDATE page_text SET Text = 'Liftbrul, greatest of the weightlifters ("No, scratch that part out!") among all drogbar, champion of the Stonedark.\n\nImmortalized in stone by chief Rynox, second-strongest drogbar of his time ("What are you writing there, Stonecarver?").\n\nThis is not a statue, it is Liftbrul, Rynox is a Stoneshaper ("Does it say something nice about me?").' WHERE ID = 5158;

-- reduce Bristlefur Bear (96146) gold drop was 171504, which is correct according
-- to wowhead but cannot find evidence that wowhead is correct
UPDATE creature_template SET mingold = 0, maxgold = 0 WHERE entry = 96146;

-- remove custom spawns
-- INSERT INTO creature VALUES (14550949, 600071, 1220, 7502, 7504, 1, 1, '', 0, 0, -846.696, 4564.27, 730.795, 0.0479738, 300, 0, 0, 1247120384, 55000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14550949 AND id = 600071;
-- INSERT INTO creature VALUES (14536502, 600023, 1220, 7502, 7502, 1, 1, '', 0, 0, -844.031, 4570.44, 728.029, 6.28015, 300, 0, 0, 1247120384, 55000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14536502 AND id = 600023;
-- INSERT INTO creature VALUES (1785686, 600022, 1220, 7502, 7502, 1, 1, '', 0, 0, -844.161, 4567.93, 728.019, 6.22518, 300, 0, 0, 1247120384, 55000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 1785686 AND id = 600022;
-- INSERT INTO creature VALUES (14608637, 542140, 1220, 7502, 7502, 1, 1, '', 0, 0, -821.458, 4561.91, 728.069, 5.63221, 300, 0, 0, 1039267, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14608637 AND id = 542140;
-- INSERT INTO creature VALUES (14611849, 542138, 1220, 7502, 7502, 1, 1, '5435', 0, 1, -814.43, 4567.25, 727.843, 2.46705, 300, 0, 0, 1334656, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14611849 AND id = 542138;
-- INSERT INTO creature VALUES (14611851, 542137, 1220, 7502, 7502, 1, 1, '5435', 0, 0, -812.319, 4569.75, 727.796, 2.38458, 300, 0, 0, 1039267, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14611851 AND id = 542137;
-- INSERT INTO creature VALUES (14593597, 542001, 1220, 7502, 7502, 1, 1, '', 0, 0, -819.598, 4560.52, 727.752, 2.50319, 300, 0, 0, 1334656, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14593597 AND id = 542001;
-- INSERT INTO creature VALUES (14603139, 542000, 1220, 7502, 7502, 1, 1, '', 0, 0, -816.823, 4564.46, 727.862, 3.12679, 300, 0, 0, 1334656, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE guid = 14603139 AND id = 542000;
-- INSERT INTO creature VALUES (146800436, 329600, 389, 2437, 2437, 2, 1, '', 0, 1, -16.5183, -62.239, -21.3715, 2.71355, 300, 0, 0, 779450, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (146800443, 329600, 1107, 7875, 7875, 1, 1, '', 0, 1, 3117.92, 1113.62, 286.675, 3.853, 300, 0, 0, 779450, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE id = 329600;
-- INSERT INTO creature VALUES (146764932, 230003, 369, 2257, 6618, 1, 65535, '', 0, 0, -91.1702, 2485.17, -43.1106, 0.108346, 300, 0, 0, 1644167168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (146764936, 230003, 571, 4395, 4564, 1, 65535, '', 0, 0, 5810.13, 503.451, 657.513, 5.24968, 300, 0, 0, 1644167168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (376535, 230003, 1220, 7502, 7505, 1, 1, '', 0, 0, -844.762, 4330.84, 745.159, 5.65093, 300, 0, 0, 1000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (146764834, 230003, 1043, 6298, 6298, 1, 1, '', 0, 0, 2062.07, -4761.28, 86.7767, 0.195497, 300, 0, 0, 1644167168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE id = 230003;
-- INSERT INTO creature VALUES (201603, 200403, 556, 3791, 3791, 6, 1, '', 0, 0, 80.9748, 286.945, 26.6293, 3.13611, 86400, 0, 0, 202215, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201604, 200404, 571, 495, 495, 1, 2, '', 0, 0, 2893.39, -4560.06, 273.741, 3.23362, 7200, 0, 0, 140007808, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201605, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2907.93, -4541.23, 273.912, 2.97837, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201606, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2905.87, -4545.43, 276.038, 3.7779, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201607, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2910.48, -4544.67, 275.534, 3.65618, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201608, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2848.97, -4586.69, 275.108, 1.61022, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201609, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2841.16, -4590.43, 276.995, 2.69407, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201610, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2827.86, -4541.74, 273.603, 0.154092, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201611, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2932.04, -4581.79, 274.896, 1.33061, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201612, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2933.33, -4577.66, 276.38, 6.16867, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (201613, 200409, 571, 495, 495, 1, 2, '', 0, 0, 2938.72, -4579.85, 276.345, 5.81524, 300, 0, 0, 120010672, 0, 0, 0, 0, 33685636, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE id IN (200403, 200404, 200409);
-- INSERT INTO creature VALUES (146764933, 190000, 369, 2257, 6618, 1, 65535, '', 0, 0, -89.7246, 2490.7, -43.1134, 6.21718, 300, 0, 0, 230117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (373118, 190000, 571, 2817, 4553, 1, 1, '', 0, 0, 5819.3, 622.577, 610.512, 2.5983, 1, 0, 0, 1220, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (146762753, 190000, 1220, 7502, 7505, 1, 1, '', 0, 0, -838.552, 4334.61, 745.027, 4.9976, 300, 0, 0, 230117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (146764835, 190000, 1043, 6298, 6298, 1, 1, '', 0, 0, 2062.53, -4765.81, 86.7767, 0.454678, 300, 0, 0, 230117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
-- INSERT INTO creature VALUES (146764937, 190000, 571, 4395, 4564, 1, 65535, '', 0, 0, 5822.25, 503.272, 657.366, 4.70932, 300, 0, 0, 230117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM creature WHERE id = 190000;
