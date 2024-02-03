-- Fix loot chance for the following quest items

-- Most of these should be 100%
-- 884 = Ghoul Rib
-- 981 = Bernice's Necklace
-- 1006 = Brass Collar
-- 1946 = Mary's Looking Glass
-- 1968 = Ogre's Monocle
-- 2713 = Ol' Sooty's Head
-- 3616 = Mind's Eye
-- 3876 = Fang of Bhag'thera
-- 3877 = Talon of Tethis
-- 3879 = Paw of Sin'Dall
-- 3880 = Head of Bangalash
-- 4106 = Tumbled Crystal
-- 22934 = Lasher Sample
-- 58179 = Gan'zulah's Body
-- 59522 = Key of Ilgalar
-- 60207 = Widow Venom Sac
-- 60213 = Crystal Spine Basilisk Blood
-- 60263 = Whispering Blue Stone
-- 60274 = Sea Salt
-- 60334 = Black Bear Brain
-- 60792 = Pristine Flight Feather (bump from 35% to 50%)
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -40 WHERE `item` = 884;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 60792;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -75 WHERE `item` IN (4106, 60263, 60274);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -90 WHERE `item` IN (60213, 60334);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (981, 1006, 1946, 1968, 2713, 3616, 3876, 3877, 3879, 3880, 22934, 58179, 59522, 60207);

-- Fix broken "Rescue the Survivors!" quest (can't heal the Draenei Survivor, wrong faction and NPC flag)
UPDATE `creature_template` SET `faction` = 1638, `npcflag` = 0 WHERE `entry` = 16483;

-- Fix broken "Thistle While You Work" quest (can't loot the seeds)
DELETE FROM `conditions` WHERE `SourceEntry` = 60737;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(4, 205089, 60737, 0, 0, 9, 0, 27025, 0, 0, 0, 0, '', '');

UPDATE `gameobject_template` SET `type` = 3, `castBarCaption` = 'Collecting', `Data8` = 27025, `Data14` = 19676, `VerifiedBuild` = 26124 WHERE `entry` = 205089;

-- Fix bad quest POI point

-- Mistmantle's Revenge
-- Adjusted this one (OLD: 26674, 1, 0, -10512, -1301, 22908)
UPDATE `quest_poi_points` SET `X` = -10382, `Y` = -1246, `VerifiedBuild` = 26124 WHERE `QuestID` = 26674 AND `Idx1` = 1;

-- Fix creatures in Stranglethorn, previously wrong level and did not attack

-- 52224 = Jungle Serpent
-- 52604 = Digsite Zombie

UPDATE `creature_template` SET `minlevel` = 24, `maxlevel` = 25, `ScaleLevelMin` = 25, `ScaleLevelMax` = 60, `faction` = 16, `unit_flags` = 0 WHERE `entry` = 52224;
UPDATE `creature_template` SET `minlevel` = 31, `maxlevel` = 32, `ScaleLevelMin` = 30, `ScaleLevelMax` = 60, `faction` = 16, `unit_flags` = 0 WHERE `entry` = 52604;

-- Fix loot chance for the following quest items

-- 2797 = Heart of Mokk (should be negative)
-- 3897 = Dizzy's Eye
-- 3910 = Snuff
-- 3919 = Mistvale Giblets
-- 3932 = Smotts' Chest
-- 4016 = Zanzil's Mixture
-- 4029 = Akiris Reed
-- 23681 = Heart of Naias (should be negative)
-- 58225 = Braddok's Big Brain
-- 58812 = Supple Tigress Fur
-- 58813 = Velvety Panther Fur
-- 58901 = Zanzil's Formulation
-- 60380 = Ironjaw Humour

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -10 WHERE `item` = 3897;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 3897 AND `entry` = 2551;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (2797, 3932, 23681, 58225, 58812, 58813, 60380);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -33 WHERE `item` = 4016;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -40 WHERE `item` = 3919;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 58901;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -60 WHERE `item` = 4029;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -80 WHERE `item` = 3910;

DELETE FROM `creature_loot_template` WHERE `item` = 3897 AND `entry` IN (2547, 43152);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(2547, 3897, -10, 0, 0, 1, 1, 0),
(43152, 3897, -10, 0, 0, 1, 1, 0);

-- Fix loot chance for the following quest items

-- 46692 = Elune's Torch

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 46692;

-- Fix loot chance for the following quest items

-- 23677 = Moongraze Buck Hide

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -80 WHERE `item` = 23677;

-- Fix "Stranglethorn Fever" quest, it should NOT be repeatable!

UPDATE `quest_template_addon` SET `SpecialFlags` = 0 WHERE `id` = 26597;

-- Fix creature in Stranglethorn, previously wrong level and did not attack

-- 53011 = Hideaway Zombie

UPDATE `creature_template` SET `minlevel` = 31, `maxlevel` = 32, `ScaleLevelMin` = 30, `ScaleLevelMax` = 60, `faction` = 16 WHERE `entry` = 53011;

-- Fix loot chance for the following quest items

-- 54855 = Gargantapid's Poison Gland
-- 60851 = Side of Bear Meat

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -40 WHERE `item` = 60851;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 54855;

-- Fix broken creatures

-- 1783 = Skeletal Flayer (scale min/max not set)
-- 1784 = Skeletal Sorcerer (scale min/max not set)
-- 1847 = Foulmane (scale min/max not set)
-- 44473 = Shaggy Black Bear (could not move, did not drop quest items, did not give XP)
-- 108815 = Anguished Spectre (bad min/max level)
-- 108830 = Risen Legionnaire (bad min/max level)
-- 108847 = Disturbed Resident (bad min/max level)

UPDATE `creature_template` SET `minlevel` = 35, `maxlevel` = 36, `mingold` = 163, `maxgold` = 163 WHERE `entry` IN (108815, 108830, 108847);
UPDATE `creature_template` SET `ScaleLevelMin` = 35, `ScaleLevelMax` = 60 WHERE `entry` IN (1783, 1784, 1847, 108815, 108830, 108847);
UPDATE `creature_template` SET `unit_flags` = 0 WHERE `entry` = 44473;

-- Remove deprecated quest

-- 24527 = Your Path Begins Here (Removed in 7.0.3 per Wowpedia)

DELETE FROM `disables` WHERE `sourceType` = 1 AND `entry` = 24527;
INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES
(1, 24527, 0, '', '', 'Deprecated quest: Your Path Begins Here');

-- Fix loot chance for the following quest items

-- 9371 = Gordunni Orb
-- 9530 = Horn of Hatetalon (supposed to drop from more things AND be quest only!)
-- 13157 = Fetid Skull
-- 15785 = Zaeldarr's Head
-- 18961 = Zukk'ash Carapace
-- 53136 = Soul Essence
-- 54856 = Duneclaw Stinger
-- 60983 = Crypt Bile
-- 60987 = Joseph's Hunting Blade
-- 62028 = Browman's Wrappings

DELETE FROM `creature_loot_template` WHERE `item` = 9530;
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(5362, 9530, -100, 0, 0, 1, 1, 0),
(5363, 9530, -100, 0, 0, 1, 1, 0),
(5364, 9530, -100, 0, 0, 1, 1, 0),
(5366, 9530, -100, 0, 0, 1, 1, 0);

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -60 WHERE `item` = 53136;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -80 WHERE `item` = 13157;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -85 WHERE `item` = 60983;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -90 WHERE `item` = 54856;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (9371, 15785, 18961, 60987, 62028);

-- Fix quests that should NOT be repeatable!
-- 27388 = Heroes of Darrowshire
-- 28755 = Annals of the Silver Hand

UPDATE `quest_template_addon` SET `SpecialFlags` = 0 WHERE `id` IN (27388, 28755);

-- Fix loot chance for the following quest items

-- 33085 = Bloodfen Feather
-- 59057 = Poobah's Tiara
-- 59058 = Poobah's Scepter
-- 59059 = Poobah's Slippers
-- 59060 = Poobah's Diary

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -33 WHERE `item` = 33085 AND `entry` IN (4356, 4357);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -25 WHERE `item` = 33085 AND `entry` = 23873;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` IN (59057, 59058, 59059, 59060);

-- Fix quests that should NOT be repeatable!

-- 2438 = The Emerald Dreamcatcher

UPDATE `quest_template` SET `Flags` = 8, `VerifiedBuild` = 26124 WHERE `id` = 2438;
UPDATE `quest_template_addon` SET `SpecialFlags` = 0 WHERE `id` = 2438;

-- Fix broken "Counter-Plague Research" quest (can't loot the rotberries, missing entry)
DELETE FROM `conditions` WHERE `SourceEntry` = 61364;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(4, 205559, 61364, 0, 0, 9, 0, 27531, 0, 0, 0, 0, '', '');

UPDATE `gameobject_template` SET `type` = 3, `VerifiedBuild` = 26124 WHERE `entry` = 205559;

-- Fix broken "Greasing the Wheel" quest (can't loot the banshee's bells, missing entry)
DELETE FROM `conditions` WHERE `SourceEntry` = 60984;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(4, 205423, 60984, 0, 0, 9, 0, 27369, 0, 0, 0, 0, '', '');

UPDATE `gameobject_template` SET `type` = 3, `VerifiedBuild` = 26124 WHERE `entry` = 205423;

-- Fix duplicated NPCs

-- 2502 = "Shaky" Phillipe
-- 44100 = Goris
-- 44391 = Innkeeper Shyria

DELETE FROM `creature` WHERE `guid` IN (360237, 360149, 167921);
UPDATE `creature` SET `position_x` = -14297.87, `position_y` = 508.75, `position_z` = 8.9645, `orientation` = 2.2568 WHERE `guid` = 4219;
UPDATE `creature` SET `position_x` = -13616.18, `position_y` = -61.9, `position_z` = 35.239, `orientation` = 6.0715 WHERE `guid` = 3364;

-- Fix battle pets that cannot be fought

-- 61459 = Little Black Ram
-- 62257 = Sand Kitten

UPDATE `creature_template` SET `faction` = 188 WHERE `entry` IN (61459, 62257);

-- Fix battle pets that cannot be fought

-- The following cast recall spells when you try to fight them...

-- 7555 = Hawk Owl
-- 61757 = Red-Tailed Chipmunk
-- 62178 = Elfin Rabbit
-- 69352 = Vampiric Cave Bat
-- 99394 = Fetid Waveling

UPDATE `creature_template` SET `faction` = 188, `unit_flags` = 0, `unit_flags2` = 0 WHERE `entry` IN (61757, 62178);
DELETE FROM `npc_spellclick_spells` WHERE spell_id IN (1, 3) AND `npc_entry` IN (7555, 61757, 62178, 69352, 99394);

-- Fix loot chance for the following quest items

-- 3514 = Mor'Ladim's Skull
-- 4103 = Shackle Key
-- 9237 = Woodpaw Gnoll Mane
-- 62510 = Shadowstout

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -65 WHERE `item` = 62510;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -75 WHERE `item` = 9237;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (3514, 4103);

-- Fix loot chance for the following quest items

-- 61313 = Mossflayer Eye
-- 62390 = Scalding Whelp Corpse

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -85 WHERE `item` = 61313;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -45 WHERE `item` = 61313 AND `entry` = 10822;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 62390;

-- Fix loot chance for the following quest items

-- 11114 = Dinosaur Bone
-- 11477 = White Ravasaur Claw
-- 11478 = Un'Goro Gorilla Pelt
-- 11479 = Un'Goro Stomper Pelt
-- 11480 = Un'Goro Thunderer Pelt
-- 11509 = Ravasaur Pheromone Gland
-- 11831 = Webbed Pterrordax Scale
-- 50371 = Silithid Leg (supposed to be guaranteed and negative chance)
-- 51778 = Hyena Chunk
-- 52281 = Meatface's Locked Chest
-- 52282 = Turtle-Digested Key

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -25 WHERE `item` = 52282;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -30 WHERE `item` = 11114;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -35 WHERE `item` IN (11478, 11479, 11480);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 11831;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -75 WHERE `item` = 51778;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (11477, 11509, 50371, 52281);

-- Fix loot chance for the following quest items

-- NOTE: These SHOULD be negative but they do not drop for some reason!!!
-- Need to come back to this and figure out why these break when marked as quest only (like they should be).

-- 2797 = Heart of Mokk
-- 23681 = Heart of Naias

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 100 WHERE `item` IN (2797, 23681);

-- Fix loot chance for the following quest items

-- 11510 = Lar'korwi's Head
-- 50410 = Durrin's Archaeological Findings (should be negative)
-- 63135 = Razor-Sharp Scorpid Barb
-- 63136 = Ember Worg Hide

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 63135;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (11510, 50410, 63136);

-- Fix battle pets that should not be green to you

-- 61317 = Long-tailed Mole
-- 62256 = Stinkbug
-- 62258 = Silithid Hatchling
-- 62364 = Ash Lizard
-- 62370 = Spotted Bell Frog

UPDATE `creature_template` SET `faction` = 188 WHERE `entry` IN (61317, 62256, 62258, 62364, 62370);

-- Disable deprecated quests

-- 24752 = "The Arts of a Mage" (Horde)
-- 26198 = "The Arts of a Mage" (Alliance)

DELETE FROM `disables` WHERE `sourceType` = 1 AND `entry` IN (24752, 26198);
INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES
(1, 24752, 0, '', '', 'Deprecated quest: The Arts of a Mage (Horde)'),
(1, 26198, 0, '', '', 'Deprecated quest: The Arts of a Mage (Alliance)');

-- Fix loot chance for the following quest items

-- 2676 = Shimmerweed
-- 3084 = Gyromechanic Gear
-- 3627 = Fang of Vagash
-- 10005 = Margol's Gigantic Horn
-- 23217 = Ravager Egg
-- 23218 = Condensed Voidwalker Essence
-- 23239 = Plump Buzzard Wing
-- 23270 = Tainted Helboar Meat
-- 30157 = Cursed Talisman
-- 56264 = Dark Iron Attack Plans
-- 58202 = Stolen Powder Keg
-- 60496 = Tender Boar Ribs
-- 62805 = Tempered Flywheel
-- 62807 = Dark Iron Memo
-- 62809 = Glassweb Venom
-- 62914 = Fire-Gizzard
-- 62916 = Dark Iron Bullet
-- 63028 = Rasha'krak's Bracers of Binding
-- 63421 = Obsidian Ashes

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -1 WHERE `item` = 23217 AND `entry` = 16932;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -4 WHERE `item` = 23217 AND `entry` = 19189;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -10 WHERE `item` = 23270 AND `entry` IN (16844, 16857);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -15 WHERE `item` = 30157 AND `entry` IN (16879, 19423, 19457, 19458, 19459);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -15 WHERE `item` = 30157 AND `entry` = 19442;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -22 WHERE `item` = 23217 AND `entry` = 16933;

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -40 WHERE `item` = 2676;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 23239;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 30157 AND `entry` IN (16871, 16873, 16907, 19422, 19424);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -80 WHERE `item` = 23218;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -85 WHERE `item` = 62914;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 23270 AND `entry` IN (16863, 16879, 16880);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (3084, 3627, 10005, 56264, 58202, 60496, 62805, 62807, 62809, 62916, 63028, 63421);

-- Fix battle pets that should not be green to you

-- 61370 = Swamp Moth
-- 61372 = Moccasin

UPDATE `creature_template` SET `faction` = 188 WHERE `entry` IN (61370, 61372);

-- Fix loot chance for the following quest items

-- 2636 = Carved Stone Idol
-- 55232 = Threshadon Chunk
-- 57131 = Intact Crocolisk Jaw
-- 60402 = Mosshide Ear
-- 60404 = Foreman Sharpsneer's Head
-- 60497 = Bear Rump
-- 60511 = Murloc Scent Gland
-- 60754 = Glassy Hornet Wing
-- 60755 = Fluffy Fox Tail

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -65 WHERE `item` IN (57131, 60511, 60754);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -75 WHERE `item` = 2636;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -90 WHERE `item` = 60497;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (55232, 60402, 60404, 60755);

-- Fix loot chance for the following quest items

-- 2629 = Intrepid Strongbox Key
-- 3183 = Mangy Claw
-- 3618 = Gobbler's Head
-- 4503 = Witherbark Tusk
-- 4515 = Marez's Head
-- 4516 = Otto's Head
-- 4517 = Falconcrest's Head
-- 4522 = Witherbark Medicine Pouch
-- 23687 = Blacktalon's Claws
-- 28513 = Demonic Rune Stone (should be negative)
-- 29795 = Burning Legion Gate Key
-- 30158 = Morkh's Shattered Armor
-- 31347 = Bleeding Hollow Torch (drops when it should not, loot chance was all over with one negative and rest positive)
-- 52305 = Humming Electrogizard
-- 55234 = Dumpy Level
-- 55988 = Glowerglare's Beard
-- 56013 = Meaty Crawler Claw
-- 56083 = Fossilized Bone
-- 56087 = Marshy Crocolisk Hide
-- 56088 = Ironforge Ingot
-- 56089 = Horrorjaw's Hide
-- 58779 = Shell of Shadra
-- 60737 = Stabthistle Seed

DELETE FROM `creature_loot_template` WHERE `item` = 28513 AND `entry` IN (18679, 18977, 19335);
DELETE FROM `creature_loot_template` WHERE `item` = 30158 AND `entry` IN (16907, 19422, 19424);
DELETE FROM `creature_loot_template` WHERE `item` = 31347 AND `entry` IN (16871, 16873, 16879, 16964, 19422, 19424);

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -25 WHERE `item` = 28513 AND `entry` = 19282;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -60 WHERE `item` = 28513 AND `entry` IN (16950, 18981);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -85 WHERE `item` = 28513 AND `entry` IN (18975, 19190);

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -40 WHERE `item` IN (4503, 4522);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 4503 AND `entry` = 2605;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (2629, 3183, 3618, 4515, 4516, 4517, 23687, 30158, 56083, 56087, 56089, 58779, 55988);

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -2 WHERE `item` = 60737;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -15 WHERE `item` = 55234;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -25 WHERE `item` = 52305;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` = 56013;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -80 WHERE `item` IN (31347, 56088);

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -0.1699 WHERE `item` = 29795 AND `entry` = 16946;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -0.0916 WHERE `item` = 29795 AND `entry` = 16947;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -0.0267 WHERE `item` = 29795 AND `entry` = 16954;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -0.0187 WHERE `item` = 29795 AND `entry` = 16960;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 29795 AND `entry` = 19298;

-- Fix "Young Murk Thresher" drop table (which will fix the "Thresher Oil" quest drop)

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -60 WHERE `entry` = 4388 AND `item` = 33126;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 10 WHERE `entry` = 4388 AND `item` = 2608;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = 22 WHERE `entry` = 4388 AND `item` = 5516;

-- Fix loot chance for the following quest items

-- 46768 = Sploder's Head
-- 55239 = Cragjaw's Huge Tooth
-- 55280 = Deepmoss Venom Sac

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` IN (46768, 55239, 55280);

-- Fix loot chance for the following quest items

-- 6245 = Karnitol's Satchel
-- 23588 = Kaliri Feather
-- 23589 = Mag'har Ancestral Beads
-- 56040 = Ram Haunch
-- 56042 = Boulderslide Cheese
-- 56187 = Sentinel's Glaive
-- 56223 = Black Dragon Whelp Filet
-- 56224 = Blazing Heart of Fire

DELETE FROM `creature_loot_template` WHERE `item` = 23588 AND `entry` IN (16966, 16967, 17084);
DELETE FROM `creature_loot_template` WHERE `item` = 23589 AND `entry` IN (16911, 16912);

UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -35 WHERE `item` = 23588 AND `entry` IN (17035, 17039, 17053);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -50 WHERE `item` IN (6245, 56042, 56223);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -55 WHERE `item` = 23588 AND `entry` = 17034;
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -75 WHERE `item` IN (23589, 56040, 56224);
UPDATE `creature_loot_template` SET `ChanceOrQuestChance` = -100 WHERE `item` = 56187;
