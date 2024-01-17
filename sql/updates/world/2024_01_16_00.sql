-- Fix loot chance for the following quest items

-- 10005 = Margol's Gigantic Horn
-- 23217 = Ravager Egg
-- 23218 = Condensed Voidwalker Essence
-- 23239 = Plump Buzzard Wing
-- 23270 = Tainted Helboar Meat
-- 30157 = Cursed Talisman
-- 58202 = Stolen Powder Keg
-- 62805 = Tempered Flywheel
-- 62807 = Dark Iron Memo
-- 62809 = Glassweb Venom
-- 62914 = Fire-Gizzard
-- 62916 = Dark Iron Bullet
-- 63028 = Rasha'krak's Bracers of Binding
-- 63421 = Obsidian Ashes

DELETE FROM `creature_loot_template` WHERE `item` IN (10005,23217,23218,23239,23270,30157,58202,62805,62807,62809,62914,62916,63028,63421);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(5833, 10005, -100, 0, 0, 1, 1, 0),

(16932, 23217, -1, 0, 0, 1, 1, 0),
(16933, 23217, -22, 0, 0, 1, 1, 0),
(19189, 23217, -4, 0, 0, 1, 1, 0),

(16974, 23218, -80, 0, 0, 1, 1, 0),
(16975, 23218, -80, 0, 0, 1, 1, 0),

(16972, 23239, -50, 0, 0, 1, 1, 0),

(16844, 23270, -10, 0, 0, 1, 1, 0),
(16857, 23270, -10, 0, 0, 1, 1, 0),
(16863, 23270, -100, 0, 0, 1, 1, 0),
(16879, 23270, -100, 0, 0, 1, 1, 0),
(16880, 23270, -100, 0, 0, 1, 1, 0),

(16871, 30157, -50, 0, 0, 1, 1, 0),
(16873, 30157, -50, 0, 0, 1, 1, 0),
(16879, 30157, -15, 0, 0, 1, 1, 0),
(16907, 30157, -50, 0, 0, 1, 1, 0),
(19422, 30157, -50, 0, 0, 1, 1, 0),
(19423, 30157, -15, 0, 0, 1, 1, 0),
(19424, 30157, -50, 0, 0, 1, 1, 0),
(19442, 30157, -20, 0, 0, 1, 1, 0),
(19457, 30157, -15, 0, 0, 1, 1, 0),
(19458, 30157, -15, 0, 0, 1, 1, 0),
(19459, 30157, -15, 0, 0, 1, 1, 0),

(42221, 58202, -100, 0, 0, 1, 1, 0),
(42222, 58202, -100, 0, 0, 1, 1, 0),

(5853, 62805, -100, 0, 0, 2, 4, 0),
(47270, 62807, -100, 0, 0, 1, 1, 0),
(5856, 62809, -100, 0, 0, 1, 1, 0),

(8338, 62916, -100, 0, 0, 2, 19, 0),
(8566, 62916, -100, 0, 0, 2, 6, 0),

(9318, 62914, -85, 0, 0, 1, 1, 0),
(47553, 63028, -100, 0, 0, 1, 1, 0),
(7032, 63421, -100, 0, 0, 1, 1, 0);

-- Fix battle pets that should not be green to you

-- 61370 = Swamp Moth
-- 61372 = Moccasin

DELETE FROM `creature_template` WHERE `entry` IN (61370,61372);
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
(61370, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 7, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(61372, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, '');