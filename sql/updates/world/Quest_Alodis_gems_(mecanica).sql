-- Arreglo de la mecanica de la mision (Las gemas de alodi)
DELETE FROM `creature_loot_template` WHERE `entry` = 108710 AND `item` = 141317;
INSERT INTO `creature_loot_template` (`entry`, `item`) VALUES 
('108710', '141317');

UPDATE `creature_template` SET `lootid`='108710' WHERE (`entry`='108710');
UPDATE `creature` SET `modelid`='55233' WHERE (`guid`='272862');
UPDATE `creature` SET `modelid`='55233' WHERE (`guid`='272864');
UPDATE `creature` SET `modelid`='55233' WHERE (`guid`='272875');
DELETE FROM `gameobject` WHERE (`guid`='136342'); -- elimino las gemas spawneadas incorrectamente en la banca de alodi
DELETE FROM `gameobject` WHERE (`guid`='136343');
DELETE FROM `gameobject` WHERE (`guid`='136344');