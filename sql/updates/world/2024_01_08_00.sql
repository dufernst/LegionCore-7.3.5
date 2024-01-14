-- Fix loot chance for the following quest items

-- 33085 = Bloodfen Feather
-- 59057 = Poobah's Tiara
-- 59058 = Poobah's Scepter
-- 59059 = Poobah's Slippers
-- 59060 = Poobah's Diary

DELETE FROM `creature_loot_template` WHERE `item` IN (33085,59057,59058,59059,59060);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`, `shared`) VALUES
(4356, 33085, -33, 0, 0, 1, 1, 0),
(4357, 33085, -33, 0, 0, 1, 1, 0),
(23873, 33085, -25, 0, 0, 1, 1, 0),

(2521, 59057, -50, 0, 0, 1, 1, 0),
(43417, 59057, -50, 0, 0, 1, 1, 0),

(2521, 59058, -50, 0, 0, 1, 1, 0),
(43417, 59058, -50, 0, 0, 1, 1, 0),

(2521, 59059, -50, 0, 0, 1, 1, 0),
(43417, 59059, -50, 0, 0, 1, 1, 0),

(2521, 59060, -50, 0, 0, 1, 1, 0),
(43417, 59060, -50, 0, 0, 1, 1, 0);

-- Fix quests that should NOT be repeatable!

-- 2438 = The Emerald Dreamcatcher

DELETE FROM `quest_template` WHERE `id` IN (2438);
INSERT INTO `quest_template` (`ID`, `QuestType`, `QuestLevel`, `QuestMaxScalingLevel`, `QuestPackageID`, `MinLevel`, `QuestSortID`, `QuestInfoID`, `SuggestedGroupNum`, `RewardNextQuest`, `RewardXPDifficulty`, `RewardXPMultiplier`, `RewardMoney`, `RewardMoneyDifficulty`, `RewardMoneyMultiplier`, `RewardBonusMoney`, `RewardDisplaySpell1`, `RewardDisplaySpell2`, `RewardDisplaySpell3`, `RewardSpell`, `RewardHonor`, `RewardKillHonor`, `RewardArtifactXP`, `RewardArtifactXPMultiplier`, `RewardArtifactCategoryID`, `StartItem`, `Flags`, `FlagsEx`, `RewardItem1`, `RewardAmount1`, `RewardItem2`, `RewardAmount2`, `RewardItem3`, `RewardAmount3`, `RewardItem4`, `RewardAmount4`, `ItemDrop1`, `ItemDropQuantity1`, `ItemDrop2`, `ItemDropQuantity2`, `ItemDrop3`, `ItemDropQuantity3`, `ItemDrop4`, `ItemDropQuantity4`, `RewardChoiceItemID1`, `RewardChoiceItemQuantity1`, `RewardChoiceItemDisplayID1`, `RewardChoiceItemID2`, `RewardChoiceItemQuantity2`, `RewardChoiceItemDisplayID2`, `RewardChoiceItemID3`, `RewardChoiceItemQuantity3`, `RewardChoiceItemDisplayID3`, `RewardChoiceItemID4`, `RewardChoiceItemQuantity4`, `RewardChoiceItemDisplayID4`, `RewardChoiceItemID5`, `RewardChoiceItemQuantity5`, `RewardChoiceItemDisplayID5`, `RewardChoiceItemID6`, `RewardChoiceItemQuantity6`, `RewardChoiceItemDisplayID6`, `POIContinent`, `POIx`, `POIy`, `POIPriority`, `RewardTitle`, `RewardArenaPoints`, `RewardSkillLineID`, `RewardNumSkillUps`, `PortraitGiver`, `PortraitTurnIn`, `RewardFactionID1`, `RewardFactionValue1`, `RewardFactionOverride1`, `FactionCapIn1`, `RewardFactionID2`, `RewardFactionValue2`, `RewardFactionOverride2`, `FactionCapIn2`, `RewardFactionID3`, `RewardFactionValue3`, `RewardFactionOverride3`, `FactionCapIn3`, `RewardFactionID4`, `RewardFactionValue4`, `RewardFactionOverride4`, `FactionCapIn4`, `RewardFactionID5`, `RewardFactionValue5`, `RewardFactionOverride5`, `FactionCapIn5`, `RewardFactionFlags`, `RewardCurrencyID1`, `RewardCurrencyQty1`, `RewardCurrencyID2`, `RewardCurrencyQty2`, `RewardCurrencyID3`, `RewardCurrencyQty3`, `RewardCurrencyID4`, `RewardCurrencyQty4`, `AcceptedSoundKitID`, `CompleteSoundKitID`, `AreaGroupID`, `TimeAllowed`, `AllowableRaces`, `QuestRewardID`, `Expansion`, `LogTitle`, `LogDescription`, `QuestDescription`, `AreaDescription`, `QuestCompletionLog`, `PortraitGiverText`, `PortraitGiverName`, `PortraitTurnInText`, `PortraitTurnInName`, `StartScript`, `CompleteScript`, `VerifiedBuild`) VALUES
(2438, 2, 6, 20, 0, 1, 141, 0, 0, 2459, 4, 1, 100, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 69, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 890, 878, 0, 0, 824181837, 0, 0, 'The Emerald Dreamcatcher', 'Retrieve the Emerald Dreamcatcher.', 'I was once given an emerald dreamcatcher from Gaerolas Talvethen, the warden of the druids in the Ban''ethil Barrow Den. This powerful amulet is able to siphon energy from the Emerald Dream, bestowing luck upon those who carry it.$B$BSadly, I have not been able to retrieve it from my dresser in Starbreeze Village... Although Starbreeze was once a tranquil place, it has now succumbed to the corruption of the furbolg that reside there.$B$BPerhaps you would be willing to recover my dreamcatcher, $c? ', '', 'Return to Tallonkai Swiftroot in Dolanaar.', '', '', '', '', 0, 0, 26124);

DELETE FROM `quest_template_addon` WHERE `id` IN (2438);
INSERT INTO `quest_template_addon` (`ID`, `MaxLevel`, `AllowableClasses`, `SourceSpellID`, `PrevQuestID`, `NextQuestID`, `ExclusiveGroup`, `RewardMailTemplateID`, `RewardMailDelay`, `RewardMailTitle`, `RequiredSkillID`, `RequiredSkillPoints`, `RequiredMinRepFaction`, `RequiredMaxRepFaction`, `RequiredMinRepValue`, `RequiredMaxRepValue`, `ProvidedItemCount`, `SpecialFlags`) VALUES
(2438, 0, 0, 0, 0, 2459, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, 0, 0);

-- Fix broken "Counter-Plague Research" quest (can't loot the rotberries, missing entry)
DELETE FROM `conditions` WHERE `SourceEntry` = 61364;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(4, 205559, 61364, 0, 0, 9, 0, 27531, 0, 0, 0, 0, '', '');

DELETE FROM `gameobject_template` WHERE `entry` = 205559;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `faction`, `flags`, `size`, `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `Data24`, `Data25`, `Data26`, `Data27`, `Data28`, `Data29`, `Data30`, `Data31`, `Data32`, `unkInt32`, `AIName`, `ScriptName`, `WorldEffectID`, `StateWorldEffectID`, `SpellVisualID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `MaxVisible`, `IgnoreDynLos`, `MinGold`, `MaxGold`, `VerifiedBuild`) VALUES
(205559, 3, 28, 'Rotberry Bush', '', 'Gathering', '', 0, 4, 0.4, 61364, 0, 0, 0, 0, 0, 259, 34719, 0, 1, 0, 0, 0, 185563, 0, 0, 0, 0, 0, 0, 24982, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26124);

-- Fix broken "Greasing the Wheel" quest (can't loot the banshee's bells, missing entry)
DELETE FROM `conditions` WHERE `SourceEntry` = 60984;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(4, 205423, 60984, 0, 0, 9, 0, 27369, 0, 0, 0, 0, '', '');

DELETE FROM `gameobject_template` WHERE `entry` = 205423;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `faction`, `flags`, `size`, `questItem1`, `questItem2`, `questItem3`, `questItem4`, `questItem5`, `questItem6`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `Data24`, `Data25`, `Data26`, `Data27`, `Data28`, `Data29`, `Data30`, `Data31`, `Data32`, `unkInt32`, `AIName`, `ScriptName`, `WorldEffectID`, `StateWorldEffectID`, `SpellVisualID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `MaxVisible`, `IgnoreDynLos`, `MinGold`, `MaxGold`, `VerifiedBuild`) VALUES
(205423, 3, 9889, 'Banshee''s Bells', '', 'Collecting', '', 0, 4, 0.3, 60984, 0, 0, 0, 0, 0, 259, 34661, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19676, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26124);

-- Fix duplicated NPCs

-- 2502 = "Shaky" Phillipe
-- 44100 = Goris
-- 44391 = Innkeeper Shyria

DELETE FROM `creature` WHERE `id` IN (2502,44100,44391);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `dynamicflags`, `AiID`, `MovementID`, `MeleeID`, `isActive`, `skipClone`, `personal_size`, `isTeemingSpawn`, `unit_flags3`) VALUES
(4219, 2502, 0, 5287, 35, 1, 1, '', 0, 0, -14297.87, 508.75, 8.9645, 2.2568, 120, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(3364, 44100, 0, 5287, 5408, 1, 1, '', 0, 0, -13616.18, -61.9, 35.239, 6.0715, 120, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(167453, 44391, 1, 357, 5072, 1, 1, '', 6553, 0, -4462.53, 2245.71, 27.6743, 4.39185, 300, 0, 0, 1003, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

-- Fix battle pets that cannot be fought

-- 61459 = Little Black Ram
-- 62257 = Sand Kitten

DELETE FROM `creature_template` WHERE `entry` IN (61459,62257);
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
(61459, 0, 5, 5, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, ''),
(62257, 0, 1, 1, 0, 0, 0, 0, 0, 4, 188, 1073741824, 0, 1, 1.14286, 1.14286, 1, 1, 2, 0, 0, 1, 2000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 3, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, '');