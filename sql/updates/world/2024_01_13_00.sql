-- Fix loot chance for the following quest items

-- 11510 = Lar'korwi's Head
-- 50410 = Durrin's Archaeological Findings (should be negative)
-- 63135 = Razor-Sharp Scorpid Barb
-- 63136 = Ember Worg Hide

DELETE FROM `creature_loot_template` WHERE `item` IN (11510,50410,63135,63136);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(9684, 11510, -100, 0, 0, 1, 1, 0),
(38329, 50410, -100, 0, 0, 1, 1, 0),
(9691, 63135, -50, 0, 0, 1, 1, 0),

(9690, 63136, -100, 0, 0, 1, 1, 0),
(9697, 63136, -100, 0, 0, 1, 1, 0);

-- Fix battle pets that should not be green to you

-- 61317 = Long-tailed Mole
-- 62256 = Stinkbug
-- 62258 = Silithid Hatchling
-- 62364 = Ash Lizard
-- 62370 = Spotted Bell Frog

DELETE FROM `creature_template` WHERE `entry` IN (61317,62256,62258,62364,62370);
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
(61317, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(62256, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(62258, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(62364, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(62370, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, '');