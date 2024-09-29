-- Fix spawn times to be reasonable for groups, it is silly to have to wait 10 minutes in a group of 2 or 3 just to finish a single quest

-- Fix spawn times for quest items that are single container, these will be 3 second respawns
-- 49012 = Abjurer's Manual
UPDATE `gameobject` SET `spawntimesecs` = 3 where `id` = 195584;

-- 49642 = Heart of Arkkoroc (also fixed position)
UPDATE `gameobject` SET `position_x` = 3543.24, `position_y` = -5138.04, `position_z` = 88.5, `orientation` = 1.87, `spawntimesecs` = 3 WHERE `id` = 200298;

-- Fix spawn times for quest items in multiple containers, these will be 15 second respawns
-- 49639 = Highborne Tablet
UPDATE `gameobject` SET `spawntimesecs` = 15 where `id` IN (199329, 199330, 199331, 199332);

-- Fix spawn times for quest items in multiple containers, these will be 20 second respawns
-- 49162 = Kawphi Bean
UPDATE `gameobject` SET `spawntimesecs` = 20 where `id` = 195686;

-- Fix spawn times for quest items in multiple containers, these will be 30 second respawns
-- 48128 = Mountainfoot Iron
UPDATE `gameobject` SET `spawntimesecs` = 30 where `id` IN (195447, 195448);

-- 49082 = Living Ire Thyme
UPDATE `gameobject` SET `spawntimesecs` = 30 where `id` = 195587;

-- 49365 = Briaroot Brew
UPDATE `gameobject` SET `spawntimesecs` = 30 where `id` = 196834;

-- Fix respawn time for chest restocking
-- (TEST LATER) UPDATE `gameobject_template` SET `Data2` = 3 WHERE `questItem1` IN (3920);