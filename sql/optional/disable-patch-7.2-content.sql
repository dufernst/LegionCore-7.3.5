-- disable map 1676 Tomb of Sargeras
-- disable map 1677 Cathedral of Eternal Night
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1676, 1677);
INSERT INTO disables (`sourceType`, `entry`, `comment`) VALUES
(2, 1676, '7.2 content - Tomb of Sargeras'),
(2, 1677, '7.2 content - Cathedral of Eternal Night');

-- disable quest 46730/48641 Armies of Legionfall (starting quest of 7.2 content)
DELETE FROM disables WHERE sourceType = 1 AND entry IN (46730, 48641);
INSERT INTO disables (`sourceType`, `entry`, `comment`) VALUES
(1, 46730, '7.2 content - Armies of Legionfall'),
(1, 48641, '7.2 content - Armies of Legionfall');

-- despawn everything in zone 7543 (Broken Shore) except for area 8143 (Darkstone Isle)
-- for all creatures in the queried zone/areas the current value for the spawnMask = 1
UPDATE creature SET spawnMask = 0 WHERE zoneId = 7543 AND areaId != 8143;
UPDATE gameobject SET spawnMask = 0 WHERE zoneId = 7543 AND areaId != 8143;
