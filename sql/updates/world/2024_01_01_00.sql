-- Fix creatures in Stranglethorn, previously wrong level and did not attack

-- 52224 = Jungle Serpent
-- 52604 = Digsite Zombie

DELETE FROM `creature_template` WHERE `entry` IN (52224,52604);
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
(52224, 0, 24, 25, 0, 25, 60, 0, 0, 3, 16, 0, 0, 1, 1.14286, 1.14286, 1, 967.1, 1297.7, 0, 1530, 1, 2000, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 52224, 0, 52224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 114, 'SmartAI', 0, 1, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(52604, 0, 31, 32, 0, 30, 60, 0, 0, 3, 16, 0, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'SmartAI', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, '');

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

DELETE FROM `creature_loot_template` WHERE `item` IN (2797,3897,3910,3919,3932,4016,4029,23681,58225,58812,58813,58901,60380);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(1514, 2797, -100, 0, 0, 1, 1, 0),

(1561, 3897, -10, 0, 0, 1, 1, 0),
(1562, 3897, -10, 0, 0, 1, 1, 0),
(1563, 3897, -10, 0, 0, 1, 1, 0),
(1564, 3897, -10, 0, 0, 1, 1, 0),
(1565, 3897, -10, 0, 0, 1, 1, 0),
(1653, 3897, -10, 0, 0, 1, 1, 0),
(2545, 3897, -10, 0, 0, 1, 1, 0),
(2546, 3897, -10, 0, 0, 1, 1, 0),
(2547, 3897, -10, 0, 0, 1, 1, 0),
(2548, 3897, -10, 0, 0, 1, 1, 0),
(2549, 3897, -10, 0, 0, 1, 1, 0),
(2551, 3897, -100, 0, 0, 1, 1, 0),  -- Brutus has a guaranteed chance judging from Wowhead!
(4505, 3897, -10, 0, 0, 1, 1, 0),
(4506, 3897, -10, 0, 0, 1, 1, 0),
(43152, 3897, -10, 0, 0, 1, 1, 0),
(43364, 3897, -10, 0, 0, 1, 1, 0),
(43454, 3897, -10, 0, 0, 1, 1, 0),
(43542, 3897, -10, 0, 0, 1, 1, 0),
(43636, 3897, -10, 0, 0, 1, 1, 0),
(43726, 3897, -10, 0, 0, 1, 1, 0),
(44178, 3897, -10, 0, 0, 1, 1, 0),
(44179, 3897, -10, 0, 0, 1, 1, 0),

(1561, 3910, -80, 0, 0, 1, 1, 0),
(1562, 3910, -80, 0, 0, 1, 1, 0),
(1563, 3910, -80, 0, 0, 1, 1, 0),
(1564, 3910, -80, 0, 0, 1, 1, 0),
(1565, 3910, -80, 0, 0, 1, 1, 0),
(1653, 3910, -80, 0, 0, 1, 1, 0),
(2545, 3910, -80, 0, 0, 1, 1, 0),
(2546, 3910, -80, 0, 0, 1, 1, 0),
(2547, 3910, -80, 0, 0, 1, 1, 0),
(2548, 3910, -80, 0, 0, 1, 1, 0),
(2549, 3910, -80, 0, 0, 1, 1, 0),
(2551, 3910, -80, 0, 0, 1, 1, 0),
(4505, 3910, -80, 0, 0, 1, 1, 0),
(4506, 3910, -80, 0, 0, 1, 1, 0),
(43364, 3910, -80, 0, 0, 1, 1, 0),
(43454, 3910, -80, 0, 0, 1, 1, 0),
(43542, 3910, -80, 0, 0, 1, 1, 0),
(43636, 3910, -80, 0, 0, 1, 1, 0),
(43726, 3910, -80, 0, 0, 1, 1, 0),
(44178, 3910, -80, 0, 0, 1, 1, 0),
(44179, 3910, -80, 0, 0, 1, 1, 0),
(44182, 3910, -80, 0, 0, 1, 1, 0),

(1557, 3919, -40, 0, 0, 1, 1, 0),
(1492, 3932, -100, 0, 0, 1, 1, 0),

(1488, 4016, -33, 0, 0, 1, 1, 0),
(1489, 4016, -33, 0, 0, 1, 1, 0),
(1490, 4016, -33, 0, 0, 1, 1, 0),
(1491, 4016, -33, 0, 0, 1, 1, 0),
(2530, 4016, -33, 0, 0, 1, 1, 0),
(2535, 4016, -33, 0, 0, 1, 1, 0),
(43223, 4016, -33, 0, 0, 1, 1, 0),

(1907, 4029, -60, 0, 0, 1, 1, 0),
(17207, 23681, -100, 0, 0, 1, 1, 0),
(42858, 58225, -100, 0, 0, 1, 1, 0),
(772, 58812, -100, 0, 0, 1, 1, 0),
(1713, 58813, -100, 0, 0, 1, 1, 0),
(1490, 58901, -50, 0, 0, 1, 1, 0),
(44113, 60380, -100, 0, 0, 1, 1, 0);