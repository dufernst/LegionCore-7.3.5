/*Night Elf Start Area (Shadowglen)*/

-- Quest and quest givers removed in patch 7.0.3

-- The Following NPC's are no longer quest givers
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry` IN
(3596,  -- Ayanna Everstride <Hunter Trainer>);
3593,  -- Alyissia <Warrior Trainer>);
3594,  -- Frahun Shadewhisper <Rogue Trainer>);
3595,  -- Shanda <Priest Trainer>);
43006, -- Rhyanda <Mage Trainer>);
3597);  -- Mardant Strongoak <Druid Trainer>);

-- Illithaine (2079) no longer starts these quests:
DELETE FROM `creature_queststarter` WHERE `id`= 2079 AND `quest` IN
(3117,  -- Quest:Etched Sigil
3116,  -- Quest:Simple Sigil
3118,  -- Quest:Encrypted Sigil
3110,  -- Quest:Hallowed Sigil
26841, -- Quest:Forbidden Sigil
3120);  -- Quest:Verdant Sigil


-- These NPCs no longer start the following obsolete quests:
DELETE FROM `creature_queststarter` WHERE `id`= 3596 AND `quest`= 26947; -- A Woodsman\'s Training
DELETE FROM `creature_queststarter` WHERE `id`= 3596 AND `quest`= 26945; -- Learning New Techniques
DELETE FROM `creature_queststarter` WHERE `id`= 3596 AND `quest`= 26946; -- A Rogue\'s Advantage
DELETE FROM `creature_queststarter` WHERE `id`= 3595 AND `quest`= 26949; -- Learning the Word

-- These NPCs no longer end the following obsolete quests:
DELETE FROM `creature_questender` WHERE `id`= 3596 AND `quest` IN (3117,26947); -- Ayanna Everstride <Hunter Trainer>)
DELETE FROM `creature_questender` WHERE `id`= 3593 AND `quest` IN (3116,26945); -- Alyissia <Warrior Trainer>)
DELETE FROM `creature_questender` WHERE `id`= 3594 AND `quest` IN (3118,26946); -- Frahun Shadewhisper <Rogue Trainer>)
DELETE FROM `creature_questender` WHERE `id`= 3595 AND `quest` IN (3110,26949); -- Shanda <Priest Trainer>)
DELETE FROM `creature_questender` WHERE `id`= 43006 AND `quest` IN (26841);     -- Rhyanda <Mage Trainer>)
DELETE FROM `creature_questender` WHERE `id`= 3597 AND `quest` IN (3120);       -- Mardant Strongoak <Druid Trainer>)

-- These quests are obsolete and need to be disabled:
DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (3117,26947,3116,26945,3118,26946,3110,26949,26841,3120);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,  3117, 0, '', '', 'Obsolete quest: Etched Sigil'),
(1,  26947, 0, '', '', 'Obsolete quest: A Woodsman\'s Training'),
(1,  3116, 0, '', '', 'Obsolete quest: Simple Sigil'),
(1,  26945, 0, '', '', 'Obsolete quest: Learning New Techniques'),
(1,  3118, 0, '', '', 'Obsolete quest: Encrypted Sigil'),
(1,  26946, 0, '', '', 'Obsolete quest: A Rogue\'s Advantage'),
(1,  3110, 0, '', '', 'Obsolete quest: Hallowed Sigil'),
(1,  26949, 0, '', '', 'Obsolete quest: Learning the Word'),
(1,  26841, 0, '', '', 'Obsolete quest: Forbidden Sigil'),
(1,  3120, 0, '', '', 'Obsolete quest: Verdant Sigil');

-- Rhyanda <Mage Trainer>), had no follow on Quest after Forbidden Sigil
-- Mardant Strongoak <Druid Trainer>), had no follow on Quest after Verdant Sigil
-- Monks have no training available in Shadowglen

/*Human starting area (Northshire Valley)*/

-- Quest and quest givers removed in patch 7.0.3
-- Human starting area (Northshire Valley)

-- The Following NPCs are no longer quest givers
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry` IN
(911,  -- Llane Beshere <Warrior Trainer>
925,   -- Bother Sammuel <Paladin Trainer>
375,   -- Priestess Anetta <Priest Trainer>
43278, -- Ashley Bank <Hunter Trainer>
459,   -- Drusilla La Salle <Warlock Trainer>
198);  -- Khelden Bremen <Mage Trainer>

-- Marshal Mcbride (197) no longer starts these quests:
DELETE FROM `creature_queststarter` WHERE `id`= 192 AND `quest` IN
(3100,  -- Quest:Simple Letter
 3101,  -- Quest:Consecrated Letter
 26910, -- Quest:Etched Letter
 3102,  -- Quest:Encrypted Letter
 3103,  -- Quest:Hallowed Letter
 3104,  -- Quest:Glyphic Letter
 3105); -- Quest:Tainted Letter
 
 -- These NPCs no longer start the following obsolete quests:
DELETE FROM `creature_queststarter` WHERE `id`= 925 AND `quest`= 26918; -- Brother Sammuel
DELETE FROM `creature_queststarter` WHERE `id`= 911 AND `quest`= 26913; -- Llane Beshere
DELETE FROM `creature_queststarter` WHERE `id`= 43278 AND `quest` = 26917; -- Ashley Bank
DELETE FROM `creature_queststarter` WHERE `id`= 915 AND `quest` = 26915; -- Jorik Kerridan
DELETE FROM `creature_queststarter` WHERE `id`= 375 AND `quest` = 26919; -- Priestess Anetta
DELETE FROM `creature_queststarter` WHERE `id`= 459 AND `quest` = 26914; -- Drusilla La Salle
DELETE FROM `creature_queststarter` WHERE `id`= 198 AND `quest` = 26916; -- Drusilla La Salle

-- These NPCs no longer end the following obsolete quests:
DELETE FROM `creature_questender` WHERE `id`= 925 AND `quest` IN (3101,26918); -- Brother Sammuel
DELETE FROM `creature_questender` WHERE `id`= 911 AND `quest` IN (3100,26913); -- Llane Beshere
DELETE FROM `creature_questender` WHERE `id`= 43278 AND `quest` IN (26910,26917); -- Ashley Bank
DELETE FROM `creature_questender` WHERE `id`= 915 AND `quest` IN (3102,26915); -- Jorik Kerridan
DELETE FROM `creature_questender` WHERE `id`= 375 AND `quest` IN (3103,26919); -- Priestess Anetta
DELETE FROM `creature_questender` WHERE `id`= 459 AND `quest` IN (3105,26914); -- Drusilla La Salle
DELETE FROM `creature_questender` WHERE `id`= 459 AND `quest` IN (3104,26916); -- Khelden Bremen

-- These quests are obsolete and need to be disabled:
DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (3100,3101,26918,26913,26910,26917,3102,26915,3103,26919,3104,3105,26914,26916);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,  3100, 0, '', '', 'Obsolete quest: Simple Letter'),
(1,  3101, 0, '', '', 'Obsolete quest: Consecrated Letter'),
(1,  26918, 0, '', '', 'Obsolete quest: The Power of the Light'),
(1,  26913, 0, '', '', 'Obsolete quest: Charging into Battle'),
(1,  26910, 0, '', '', 'Obsolete quest: Etched Letter'),
(1,  26917, 0, '', '', 'Obsolete quest: The Hunter\'s Path'),
(1,  3102, 0, '', '', 'Obsolete quest: Encrypted Letter'),
(1,  26915, 0, '', '', 'Obsolete quest: The Deepest Cut'),
(1,  3103, 0, '', '', 'Obsolete quest: Hallowed Letter'),
(1,  26919, 0, '', '', 'Obsolete quest: Learning the Word'),
(1,  3104, 0, '', '', 'Obsolete quest: Glyphic Letter'),
(1,  3105, 0, '', '', 'Obsolete quest: Tainted Letter'),
(1,  26914, 0, '', '', 'Obsolete quest: Corruption'),
(1,  26916, 0, '', '', 'Obsolete quest: Mastering the Arcane');

/*Orc starting area (Valley of Trials)*/

-- Quests and quest givers removed in patch 7.0.3
-- Orc starting area (Valley of Trials)

-- These NPCs are no longer quest givers as of patch 7.0.3:
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry` IN
(3153,  -- Frang <Warrior Trainer>
 3155,  -- Rwag <Rogue Trainer>
 3157,  -- Shikrik <Shaman Trainer>
39214); -- Karranisha <Hunter Trainer>

-- Gornek (3143) no longer offers these class quests:
DELETE FROM `creature_queststarter` WHERE `id`= 3143 AND `quest` IN
(2383,  -- Quest:Simple Parchment
 3087,  -- Quest:Etched Parchment
 3088,  -- Quest:Encrypted Parchment
 3089); -- Quest:Rune-Inscribed Parchment

DELETE FROM `creature_queststarter` WHERE `id` = 3153 AND `quest`= 25147; -- Frang <Warrior Trainer>, Quest:Charge
DELETE FROM `creature_queststarter` WHERE `id` = 3155 AND `quest`= 25141; -- Rwag <Rogue Trainer>, Quest:Eviscerate
DELETE FROM `creature_queststarter` WHERE `id` = 3157 AND `quest`= 25143; -- Shikrik <Shaman Trainer>, Quest:Primal Strike
DELETE FROM `creature_queststarter` WHERE `id`= 39214 AND `quest`= 25139; -- Karranisha <Hunter Trainer>, Quest:Steady Shot

DELETE FROM `creature_questender` WHERE `id` = 3153 AND `quest` IN (2383,25147); -- Frang <Warrior Trainer>, Quest:Simple Parchment & Charge
DELETE FROM `creature_questender` WHERE `id` = 3155 AND `quest` IN (3088,25141); -- Rwag <Rogue Trainer>, Quest:Encrypted Parchment & Eviscerate
DELETE FROM `creature_questender` WHERE `id` = 3157 AND `quest` IN (3089,25143); -- Shikrik <Shaman Trainer>, Quest:Rune-Inscribed Parchment & Primal Strike
DELETE FROM `creature_questender` WHERE `id`= 39214 AND `quest` IN (3087,25139); -- Karranisha <Hunter Trainer>, Quest:Etched Parchment & Steady Shot

DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (2383,3087,3088,3089,25139,25141,25143,25147);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,  2383, 0, '', '', 'Obsolete quest: Simple Parchment'),
(1,  3087, 0, '', '', 'Obsolete quest: Etched Parchment'),
(1,  3088, 0, '', '', 'Obsolete quest: Encrypted Parchment'),
(1,  3089, 0, '', '', 'Obsolete quest: Rune-Inscribed Parchment'),
(1, 25139, 0, '', '', 'Obsolete quest: Steady Shot'),
(1, 25141, 0, '', '', 'Obsolete quest: Eviscerate'),
(1, 25143, 0, '', '', 'Obsolete quest: Primal Strike'),
(1, 25147, 0, '', '', 'Obsolete quest: Charge');

/*Undead starting area (Deathknell)*/

-- Quests and quest givers removed in patch 7.0.3
-- Undead starting area (Deathknell)

-- The following NPCs are no longer quest givers:
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry` IN
(2119,  -- Dannal Stern <Warrior Trainer>
 2122,  -- David Trias <Rogue Trainer>
 2123,  -- Dark Cleric Duesten <Priest Trainer>
 2124,  -- Isabella <Mage Trainer>
 2126,  -- Maximillion <Warlock Trainer>
38911, -- Xavier the Huntsman <Hunter Trainer>
63272); -- Ting <Monk Trainer>

-- Shadow Priest Sarvis (1569) no longer starts these quests:
DELETE FROM `creature_queststarter` WHERE `id`= 1569 AND `quest` IN
(3095,  -- Quest:Simple Scroll
 3096,  -- Quest:Encrypted Scroll
 3097,  -- Quest:Hallowed Scroll
 3098,  -- Quest:Glyphic Scroll
 3099,  -- Quest:Tainted Scroll
24962); -- Quest:Trail-Worn Scroll

-- These NPCs should no longer offer the following quests:
DELETE FROM `creature_queststarter` WHERE `id` = 2119 AND `quest`= 24969; -- Dannal Stern <Warrior Trainer>, Quest:Charging into Battle
DELETE FROM `creature_queststarter` WHERE `id` = 2122 AND `quest`= 24967; -- David Trias <Rogue Trainer>, Quest:Stab!
DELETE FROM `creature_queststarter` WHERE `id` = 2123 AND `quest`= 24966; -- Dark Cleric Duesten <Priest Trainer>, Quest:Of Light and Shadows
DELETE FROM `creature_queststarter` WHERE `id` = 2124 AND `quest`= 24965; -- Isabella <Mage Trainer>, Quest:Magic Training
DELETE FROM `creature_queststarter` WHERE `id` = 2126 AND `quest`= 24968; -- Maximillion <Warlock Trainer>, Quest:Dark Deeds
DELETE FROM `creature_queststarter` WHERE `id`= 38911 AND `quest`= 24964; -- Xavier the Huntsman <Hunter Trainer>, Quest:The Thrill of the Hunt
DELETE FROM `creature_queststarter` WHERE `id` = 63272 AND `quest`= 31147; -- Tng <Monk Trainer>, Quest: Tiger Palm

-- and they also should no longer end these quests:
DELETE FROM `creature_questender` WHERE `id` = 2119 AND `quest` IN (3095,24969);  -- Dannal Stern <Warrior Trainer>, Quest:Simple Scroll & Charging into Battle
DELETE FROM `creature_questender` WHERE `id` = 2122 AND `quest` IN (3096,24967);  -- David Trias <Rogue Trainer>, Quest:Encrypted Scroll & Stab!
DELETE FROM `creature_questender` WHERE `id` = 2123 AND `quest` IN (3097,24966);  -- Dark Cleric Duesten <Priest Trainer>, Quest:Hallowed Scroll & Of Light and Shadows
DELETE FROM `creature_questender` WHERE `id` = 2124 AND `quest` IN (3098,24965);  -- Isabella <Mage Trainer>, Quest:Glyphic Scroll & Magic Training
DELETE FROM `creature_questender` WHERE `id` = 2126 AND `quest` IN (3099,24968);  -- Maximillion <Warlock Trainer>, Quest:Tainted Scroll & Dark Deeds
DELETE FROM `creature_questender` WHERE `id`= 38911 AND `quest` IN (24962,24964); -- Xavier the Huntsman <Hunter Trainer>, Quest:Trail-Worn Scroll & The Thrill of the Hunt
DELETE FROM `creature_questender` WHERE `id` = 63272 AND `quest`= 31147;  -- Ting <Monk Trainer>, Quest: Tiger Palm

-- Disable all the quests listed above
DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (3095,3096,3097,3098,3099,24962,24964,24965,24966,24967,24968,24969,31147);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,  3095, 0, '', '', 'Obsolete quest: Simple Scroll'),
(1,  3096, 0, '', '', 'Obsolete quest: Encrypted Scroll'),
(1,  3097, 0, '', '', 'Obsolete quest: Hallowed Scroll'),
(1,  3098, 0, '', '', 'Obsolete quest: Glyphic Scroll'),
(1,  3099, 0, '', '', 'Obsolete quest: Tainted Scroll'),
(1, 24962, 0, '', '', 'Obsolete quest: Trail-Worn Scroll'),
(1, 24964, 0, '', '', 'Obsolete quest: The Thrill of the Hunt'),
(1, 24965, 0, '', '', 'Obsolete quest: Magic Training'),
(1, 24966, 0, '', '', 'Obsolete quest: Of Light and Shadows'),
(1, 24967, 0, '', '', 'Obsolete quest: Stab!'),
(1, 24968, 0, '', '', 'Obsolete quest: Dark Deeds'),
(1, 24969, 0, '', '', 'Obsolete quest: Charging into Battle'),
(1, 31147, 0, '', '', 'Obsolete quest: Tiger Palm');

/*Blood Elf Class Quest Fixes*/

-- Quests and quest givers removed in patch 7.0.3

-- The following NPCs are no longer quest givers:
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry` IN
(15279,  -- Julia Sunstriker <Mage Trainer>
 15280,  -- Jesthenis Sunstriker <Paladin Trainer>
 15283,  -- Summoner Teli'Larien <Warlock Trainer>
 15284,  -- Matron Arena <Priest Trainer>
 15285,  -- Pathstalker Kariel <Rogue Trainer>
 15513,  -- Ranger Sallina <Hunter Trainer>
 43010,  -- Delios Silverblade <Warrior Trainer>
 63332); -- Pao <Monk Trainer>

-- Magistrix Erona (15278) no longer starts these quests:
DELETE FROM `creature_queststarter` WHERE `id`= 15278 AND `quest` IN
(8328,  -- Quest:Mage Training
 8329,  -- Quest:Warrior Training
 8563,  -- Quest:Warlock Training
 8564,  -- Quest:Priest Training
 9392,  -- Quest:Rogue Training
 9393,  -- Quest:Hunter Training
 9676); -- Quest:Paladin Training

-- These NPCs no longer start the following obsolete quests:
DELETE FROM `creature_queststarter` WHERE `id`= 15279 AND `quest`= 10068; -- Julia Sunstriker <Mage Trainer>, Quest:Frost Nova
DELETE FROM `creature_queststarter` WHERE `id`= 15280 AND `quest`= 10069; -- Jesthenis Sunstriker <Paladin Trainer>, Quest:Ways of the Light
DELETE FROM `creature_queststarter` WHERE `id`= 15283 AND `quest`= 10073; -- Summoner Teli'Larien <Warlock Trainer>, Quest:Curruption
DELETE FROM `creature_queststarter` WHERE `id`= 15284 AND `quest`= 10072; -- Matron Arena <Priest Trainer>, Quest:Learning the Word
DELETE FROM `creature_queststarter` WHERE `id`= 15285 AND `quest`= 10071;  -- Pathstalker Kariel <Rogue Trainer>, Quest:Evisceration
DELETE FROM `creature_queststarter` WHERE `id`= 15513 AND `quest`= 10070; -- Ranger Sallina <Hunter Trainer>, Quest:Steady Shot
DELETE FROM `creature_queststarter` WHERE `id`= 43010 AND `quest`= 27091; -- Delios Silverblade <Warrior Trainer>, Quest:Charge
DELETE FROM `creature_queststarter` WHERE `id`= 63332 AND `quest`= 31171; -- Pao <Monk Trainer>, Quest:Tiger Palm

-- Well Watcher Solanian (15295) also gives these 2 extra quest IDs for existing quests 8330, 8345:
DELETE FROM `creature_queststarter` WHERE `id`= 15295 AND `quest` IN (37442,37443);
INSERT INTO `creature_queststarter` (`id`,`quest`) VALUES
(15295, 37442), -- The Shrine of Dath'Remar (also 8345)
(15295, 37443); -- Solanian's Belongings (also 8330)

-- These NPCs no longer end the following obsolete quests:
DELETE FROM `creature_questender` WHERE `id`= 15279 AND `quest` IN (8328,10068); -- Julia Sunstriker <Mage Trainer>, Quest:Mage Training & Frost Nova
DELETE FROM `creature_questender` WHERE `id`= 15280 AND `quest` IN (9676,10069); -- Jesthenis Sunstriker <Paladin Trainer>, Quest:Paladin Training & Ways of the Light
DELETE FROM `creature_questender` WHERE `id`= 15283 AND `quest` IN (8344,8563,10073); -- Summoner Teli'Larien <Warlock Trainer>, Quests:Windows to the Source,Warlock Training,Curruption
DELETE FROM `creature_questender` WHERE `id`= 15284 AND `quest` IN (8564,10072); -- Matron Arena <Priest Trainer>, Quest:Priest Training & Learning the Word
DELETE FROM `creature_questender` WHERE `id`= 15285 AND `quest` IN (9392,10071); -- Pathstalker Kariel <Rogue Trainer>, Quest:Rogue Training & Evisceration
DELETE FROM `creature_questender` WHERE `id`= 15513 AND `quest` IN (9393,10070); -- Ranger Sallina <Hunter Trainer>, Quest:Hunter Training & Steady Shot
DELETE FROM `creature_questender` WHERE `id`= 43010 AND `quest` IN (8329, 27091); -- Delios Silverblade <Warrior Trainer>, Quest:Warrior Training & Charge
DELETE FROM `creature_questender` WHERE `id`= 63332 AND `quest` IN (31170, 31171); -- Pao <Monk Trainer>, Quest:Monk Training & Tiger Palm

-- These quests are obsolete and need to be disabled:
DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (8328,8329,8344,8563,8564,9392,9393,9676,10068,10069,10070,10071,10072,10073,27091,31170,31171,37441);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,  8328, 0, '', '', 'Obsolete quest: Mage Training'),
(1,  8329, 0, '', '', 'Obsolete quest: Warrior Training'),
(1,  8344, 0, '', '', 'Obsolete quest: Windows to the Source'),
(1,  8563, 0, '', '', 'Obsolete quest: Warlock Training'),
(1,  8564, 0, '', '', 'Obsolete quest: Priest Training'),
(1,  9392, 0, '', '', 'Obsolete quest: Rogue Training'),
(1,  9393, 0, '', '', 'Obsolete quest: Hunter Training'),
(1,  9676, 0, '', '', 'Obsolete quest: Paladin Training'),
(1, 10068, 0, '', '', 'Obsolete quest: Frost Nova'),
(1, 10069, 0, '', '', 'Obsolete quest: Ways of the Light'),
(1, 10070, 0, '', '', 'Obsolete quest: Steady Shot'),
(1, 10071, 0, '', '', 'Obsolete quest: Evisceration'),
(1, 10072, 0, '', '', 'Obsolete quest: Learning the Word'),
(1, 10073, 0, '', '', 'Obsolete quest: Curruption'),
(1, 27091, 0, '', '', 'Obsolete quest: Charge'),
(1, 31170, 0, '', '', 'Obsolete quest: Monk Training'),
(1, 31171, 0, '', '', 'Obsolete quest: Tiger Palm'),
(1, 37441, 0, '', '', 'Obsolete quest: Solanian\'s Belongings'); -- Replaced by Quest ID 37443 (+ 8330)

-- Julia Sunstriker <Mage Trainer> is no longer a quest giver
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry`= 15279;

-- Quest: Mage Training (8328) removed in patch 7.0.3
DELETE FROM `creature_queststarter` WHERE `id`= 15278 AND `quest`= 8328;
DELETE FROM `creature_questender` WHERE `id`= 15279 AND `quest`= 8328;
-- Quest: Frost Nova (10068) removed in patch 7.0.3
DELETE FROM `creature_queststarter` WHERE `id`= 15279 AND `quest`= 10068;
DELETE FROM `creature_questender` WHERE `id`= 15279 AND `quest`= 10068;

DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (8328,10068);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1,  8328, 0, '', '', 'Quest removed in patch 7.0.3: Mage Training'),
(1, 10068, 0, '', '', 'Quest removed in patch 7.0.3: Frost Nova');

/*-- Tauren starting area ( Mulgore )*/

-- Low-level class quests have been removed in patch 7.0.3
-- Tauren starting area ( Mulgore )

-- The following NPCs are no longer quest givers:
UPDATE `creature_template` SET `npcflag` = `npcflag`& ~2 WHERE `entry` IN
( 3059,    -- Harutt Thunderhorn <Warrior Trainer>
 37737,    -- Sunwalker Helaku <Paladin Trainer>
 37724,    -- Seer Ravenfeather <Priest Trainer>
  3062,    -- Meela Dawnstrider <Shaman Trainer>
  3060,    -- Gart Mistrunner <Druid Trainer>
  3061,    -- Lanka Farshot <Hunter Trainer>
 63327);   -- Shoyu <Monk Trainer>

-- Rohaku Stonehoof (44927) no longer starts the class quests
DELETE FROM `creature_queststarter` WHERE `id`= 44927 AND `quest` IN
( 3091,    -- Quest: Simple Note (Warrior)
  3092,    -- Quest: Etched Note (Hunter)
  3093,    -- Quest: Rune-Inscribed Note (Shaman)
  3094,    -- Quest: Verdant Note (Druid)
-- 31165,    -- Quest: Calligraphed Note (Monk)
 27014,    -- Quest: Hallowed Note (Priest)
 27015);   -- Quest: Consecrated Note (Paladin)

-- These NPCs should no longer offer the following quests:
DELETE FROM `creature_queststarter` WHERE `id` = 3059 AND `quest`= 27020;  -- Harutt Thunderhorn <Warrior Trainer>, Quest: The First Lesson
DELETE FROM `creature_queststarter` WHERE `id` = 37737 AND `quest`= 27023; -- Sunwalker Helaku <Paladin Trainer>, Quest: The Way of the Sunwalkers
DELETE FROM `creature_queststarter` WHERE `id` = 37724 AND `quest`= 27066; -- Seer Ravenfeather <Priest Trainer>, Quest: Learning the Word
DELETE FROM `creature_queststarter` WHERE `id` = 3062 AND `quest`= 27027;  -- Meela Dawnstrider <Shaman Trainer>, Quest: Moonfire
DELETE FROM `creature_queststarter` WHERE `id` = 3060 AND `quest`= 27067;  -- Gart Mistrunner <Druid Trainer>, Quest: Verdant Note
DELETE FROM `creature_queststarter` WHERE `id` = 3061 AND `quest`= 27021;  -- Lanka Farshot <Hunter Trainer>, Quest: The Hunter's Path
DELETE FROM `creature_queststarter` WHERE `id` = 63327 AND `quest`= 31166; -- Shoyu <Monk Trainer>, Quest: Tiger Palm

-- and they also should no longer end these quests:
DELETE FROM `creature_questender` WHERE `id` = 3059 AND `quest` IN (3091,27020);   -- Harutt Thunderhorn <Warrior Trainer>, Quest: Simple Note & The First Lesson
DELETE FROM `creature_questender` WHERE `id` = 37737 AND `quest` IN (27015,27023); -- Sunwalker Helaku <Paladin Trainer>,   Quest: Consecrated Note & The Way of the Sunwalkers
DELETE FROM `creature_questender` WHERE `id` = 37724 AND `quest` IN (27014,27066); -- Seer Ravenfeather <Priest Trainer>,   Quest: Hallowed Note & Learning the Word
DELETE FROM `creature_questender` WHERE `id` = 3062 AND `quest` IN (3093,27027);   -- Meela Dawnstrider <Shaman Trainer>,   Quest: Rune-Inscribed Note & Primal Strike
DELETE FROM `creature_questender` WHERE `id` = 3060 AND `quest` IN (3094,27067);   -- Gart Mistrunner <Druid Trainer>,      Quest: Verdant Note & Moonfire
DELETE FROM `creature_questender` WHERE `id` = 3061 AND `quest` IN (3092,27021);   -- Lanka Farshot <Hunter Trainer>,       Quest: Etched Note & The Hunter's Path
DELETE FROM `creature_questender` WHERE `id` = 63327 AND `quest` IN (31165,31166); -- Shoyu <Monk Trainer>,                 Quest: Consecrated Note & Tiger Palm

-- Disable all the quests listed above
DELETE FROM `disables` WHERE `sourceType`= 1 AND `entry` IN (3091,27020,27015,27023,27014,27066,3093,27027,3094,27067,3092,27021,31165,31166);
INSERT INTO `disables` (`sourceType`,`entry`,`flags`,`params_0`,`params_1`,`comment`) VALUES
(1, 3091, 0, '', '', 'Obsolete quest: Simple Note'),
(1, 27020, 0, '', '', 'Obsolete quest: The First Lesson'),
(1, 27015, 0, '', '', 'Obsolete quest: Consecrated Note'),
(1, 27023, 0, '', '', 'Obsolete quest: The Way of the Sunwalkers'),
(1, 27014, 0, '', '', 'Obsolete quest: Hallowed Note'),
(1, 27066, 0, '', '', 'Obsolete quest: Learning the Word'),
(1, 3093, 0, '', '', 'Obsolete quest: Rune-Inscribed Note'),
(1, 27027, 0, '', '', 'Obsolete quest: Primal Strike'),
(1, 3094, 0, '', '', 'Obsolete quest: Verdant Note'),
(1, 27067, 0, '', '', 'Obsolete quest: Moonfire'),
(1, 3092, 0, '', '', 'Obsolete quest: Etched Note'),
(1, 27021, 0, '', '', 'Obsolete quest: The Hunters Path'),
(1, 31165, 0, '', '', 'Obsolete quest: Consecrated Note'),
(1, 31166, 0, '', '', 'Obsolete quest: Tiger Palm');