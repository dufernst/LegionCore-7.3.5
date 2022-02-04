-- enable map 1676 Tomb of Sargeras
-- enable map 1677 Cathedral of Eternal Night
DELETE FROM disables WHERE sourceType = 2 AND entry IN (1676, 1677);

-- enable quest 46730/48641 Armies of Legionfall (starting quest of 7.2 content)
DELETE FROM disables WHERE sourceType = 1 AND entry IN (46730, 48641);

-- spawn everything in zone 7543 (Broken Shore) except for area 8143 (Darkstone Isle)
-- for all creatures/gameobjects in the queried zone/areas the value of the spawnMask before despawning was 1
UPDATE creature SET spawnMask = 1 WHERE zoneId = 7543 AND areaId != 8143;
UPDATE gameobject SET spawnMask = 1 WHERE zoneId = 7543 AND areaId != 8143;
