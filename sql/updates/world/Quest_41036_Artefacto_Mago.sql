-- Completo Scenario  Pimera Mision Arma de Artefacto Mago in Dalaran Quest 41036 (EL SEÑOR DEL TERROR ARTEFACTO MAGO)

--  SAI
SET @ENTRY := 102700;
UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry`=@ENTRY;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@ENTRY AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@ENTRY,0,1,0,19,0,100,0,41036,0,0,0,1,0,6000,0,0,0,0,7,0,0,0,0,0,0,0,"Quest Accept - Say Text"),
(@ENTRY,0,2,0,52,0,100,0,0,0,0,0,53,1,102700,0,0,0,0,1,0,0,0,0,0,0,0,"Waypoint"),
(@ENTRY,0,3,0,40,0,100,0,2,0,0,0,1,15,1500,0,0,0,0,1,0,0,0,0,0,0,0,"NPC-PRIMER TEXTO CAMINANDO"),
(@ENTRY,0,4,0,40,0,100,0,6,0,0,0,1,1,2500,0,0,0,0,1,0,0,0,0,0,0,0,"NPC-SEGUNDO TEXTO CAMINANDO)"),
(@ENTRY,0,5,0,40,0,100,0,9,0,0,0,1,16,4000,0,0,0,0,1,0,0,0,0,0,0,0,"NPC TERCER TEXTO CAMINANDO"),
(@ENTRY,0,6,0,58,0,100,0,13,0,0,0,12,@ENTRY*100+00,8,1000,0,0,0,8,0,0,0,-952.657,4334.45,740.228,-2.2474,"NPC FINAL CUANDO TERMINA DE CAMINAR"),
(@ENTRY,0,7,0,17,0,100,0,@ENTRY*100+00,0,0,0,41,5,0,0,0,0,0,1,0,0,0,-952.657,4334.45,740.228,-2.2474,"Final del Texto "),
(@ENTRY,0,9,0,61,0,100,0,0,0,0,0,204,1034,0,0,0,0,0,7,0,0,0,0,0,0,0,"Link - Set Scenario"),
(@ENTRY,0,10,0,20,0,100,0,41036,0,0,0,85,204287,0,0,0,0,0,7,0,0,0,0,0,0,0,"Quest Reward - Invoker Cast Spell"),
(@ENTRY,0,11,6,62,0,100,0,20432,0,0,0,1,12,6000,0,0,0,0,7,0,0,0,0,0,0,0,"Gossip Select - Say Text"),
(@ENTRY,0,12,0,61,0,100,0,0,0,0,0,81,2,0,0,0,0,0,1,0,0,0,0,0,0,0,"Link - Set Npc Flag"),
(@ENTRY,0,13,0,52,0,100,0,12,102700,0,0,45,1,1,0,0,0,0,19,102846,50,0,0,0,0,0,"Text Over - Send Data"),
(@ENTRY,0,14,0,38,0,100,0,2,2,0,0,1,13,6000,0,0,0,0,7,0,0,0,0,0,0,0,"Update Data - Say Text"),
(@ENTRY,0,15,10,52,0,100,0,13,102700,0,0,45,3,3,0,0,0,0,19,102846,50,0,0,0,0,0,"Text Over - Send Data"),
(@ENTRY,0,16,0,61,0,100,0,0,0,0,0,81,3,0,0,0,0,0,1,0,0,0,0,0,0,0,"Link - Set NPC Flag"),
(@ENTRY,0,17,0,19,0,100,0,41125,0,0,0,85,196500,0,0,0,0,0,7,0,0,0,0,0,0,0,"QA - ICS"),
(@ENTRY,0,18,13,19,0,100,0,41124,0,0,0,1,8,12000,0,0,0,0,7,0,0,0,0,0,0,0,"QA - ST"),
(@ENTRY,0,19,0,61,0,100,0,0,0,0,0,33,103193,0,0,0,0,0,7,0,0,0,0,0,0,0,"Link - KC"),
(@ENTRY,0,20,0,52,0,100,0,8,102700,0,0,1,9,12000,0,0,0,0,7,0,0,0,0,0,0,0,"TO - ST"),
(@ENTRY,0,21,0,52,0,100,0,9,102700,0,0,1,10,0,0,0,0,0,7,0,0,0,0,0,0,0,"TO - ST"),
(@ENTRY,0,22,0,64,0,100,0,0,0,0,0,33,107589,0,0,0,0,0,7,0,0,0,0,0,0,0,"TO - ST");

--  SAI
SET @ENTRY := 10270000;
UPDATE `creature_template` SET `AIName`="SmartAI" WHERE `entry`=@ENTRY;
DELETE FROM `smart_scripts` WHERE `entryorguid`=@ENTRY AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(@ENTRY,0,0,0,64,0,100,0,2043200,0,0,0,85,203241,99,0,0,0,0,7,0,0,0,0,0,0,0,"Cast Spell Hacia el Scenario Spawneado");

delete from `creature_template_wdb` where `Entry` = '10270000';
INSERT INTO `creature_template_wdb` (`Entry`, `Name1`, `Name2`, `Name3`, `Name4`, `NameAlt1`, `NameAlt2`, `NameAlt3`, `NameAlt4`, `Title`, `TitleAlt`, `CursorName`, `TypeFlags`, `TypeFlags2`, `Type`, `Family`, `Classification`, `KillCredit1`, `KillCredit2`, `VignetteID`, `Displayid1`, `Displayid2`, `Displayid3`, `Displayid4`, `HpMulti`, `PowerMulti`, `Leader`, `QuestItem1`, `QuestItem2`, `QuestItem3`, `QuestItem4`, `QuestItem5`, `QuestItem6`, `QuestItem7`, `QuestItem8`, `QuestItem9`, `QuestItem10`, `MovementInfoID`, `RequiredExpansion`, `FlagQuest`, `VerifiedBuild`) VALUES ('10270000', 'Meryl Felstorm', '', '', '', '', '', '', '', '', '', '', '134479872', '0', '6', '0', '0', '0', '0', '0', '67760', '0', '0', '0', '3', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '6', '6', '22566');

delete from `creature_template` where `entry` = '10270000';
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `SandboxScalingID`, `ScaleLevelMin`, `ScaleLevelMax`, `ScaleLevelDelta`, `ScaleLevelDuration`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `InhabitType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES ('10270000', '2043200', '110', '110', '0', '98', '110', '0', '100', '6', '2263', '3', '0', '1', '1.14286', '1.14286', '1', '0', '0', '0', '0', '1', '2000', '2000', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 'SmartAI', '0', '3', '1', '1', '1', '1', '0', '0', '-1', '', '', '0', '0', '0', '0', '0', '0', '0', '');

delete from `waypoints` where `Entry` = '102700';
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '1', '-842.729', '4429.32', '742.537', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '2', '-842.031', '4425.24', '741.872', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '3', '-841.323', '4416.66', '739.887', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '4', '-840.614', '4409.99', '739.962', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '5', '-838.913', '4393.98', '737.773', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '6', '-850.268', '4395.76', '737.572', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '7', '-870.426', '4410.08', '737.52', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '8', '-874.659', '4413.71', '737.549', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '9', '-895.274', '4406.58', '737.122', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '10', '-904.104', '4395.74', '738.74', '0');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '11', '-914.199', '4382.26', '739.851', NULL);
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '12', '-931.834', '4360.38', '740.292', '');
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES ('102700', '13', '-952.657', '4334.45', '740.228', NULL);


delete from `creature_text` where `Entry` = '102700';
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '15', '0', 'In ages past, the Forge of the Guardian was a source of great power for the Council of Tirisfal.', '12', '0', '100', '0', '0', '0', '120624', '0', '0', '0', '');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '17', '0', 'It seems my fears were well-founded! The dreadlord is already inside. We must hurry!', '12', '0', '100', '0', '0', '0', '120632', '0', '0', '0', '');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '16', '0', 'It was secreted to the Violet Hold. Not only was this the most secure location in Dalaran, it was a place few would think to look.', '12', '0', '100', '0', '0', '0', '120626', '0', '0', '0', '');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '13', '0', 'Let us recreate Arrexis\'s ritual! If we start the ritual at an invasion point, that will attract Balaadur\'s attention. He will likely ambush us, but we will be ready.', '12', '0', '100', '1', '0', '0', '112327', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '12', '0', 'So Arrexis was ambushed by this eredar, Balaadur?  He is probably in possession of the staff now then...', '12', '0', '100', '1', '0', '0', '112317', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '1', '0', 'When the Legion learned of the forge\'s existence, the Council knew it had to be hidden somewhere safe.', '12', '0', '100', '0', '0', '0', '120625', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '10', '0', 'А теперь ступай в Зал Войны и познакомься с нашим новым союзником из гоблинов. Можете сразу начинать планировать ваши дальнейшие шаги.', '12', '0', '100', '0', '0', '0', '0', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '0', '0', 'Времени в обрез. Этот портал перенесет нас прямо в Аметистовую крепость.', '12', '0', '100', '0', '0', '0', '120623', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '7', '0', 'Значит, решено.', '12', '0', '100', '0', '0', '0', '46157', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '3', '0', 'Маги Азерота! Я собрал вас здесь пред лицом угрозы всему нашему миру!', '12', '0', '100', '0', '0', '0', '0', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '11', '0', 'Мне нужно отлучиться по личному делу...', '12', '0', '100', '0', '0', '0', '105993', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '5', '0', 'Мы должны отправиться за ним в Круговерть Пустоты и отыскать его там. Лишь тогда мы будем уверены, что он не сможет использовать свои знания против нас.', '12', '0', '100', '0', '0', '0', '105826', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '2', '0', 'Ну что, Аказамзарак, все координаты у тебя. Сможешь провесить порталы?', '12', '0', '100', '0', '0', '0', '105822', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '8', '0', 'Отлично. В присутствии величайших магов Азерота нарекаю тебя $n, $gКолдун:Колдунья; Стражей Тирисфаля.', '12', '0', '100', '0', '0', '0', '0', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '4', '0', 'Повелитель ужаса Катра\'натир сбежал в Круговерть Пустоты, унеся с собой секреты Совета Тирисфаля.', '12', '0', '100', '0', '0', '0', '0', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '6', '0', 'С этой целью я восстанавливаю орден Стражей Тирисфаля. И я призываю вас присоединиться к нам и поднять оружие против Легиона! Вы с нами?!', '12', '0', '100', '0', '0', '0', '0', '0', '0', '0', 'Мерил Буря Скверны to Player');
INSERT INTO `creature_text` (`Entry`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextID`, `MinTimer`, `MaxTimer`, `SpellID`, `comment`) VALUES ('102700', '9', '0', 'Этот древний титул в свое время носили старейшие из магов Азерота, и он символизирует огромную ответственность, возложенную на Стражей Тирисфаля. Носи же его с гордостью и честью.', '12', '0', '100', '0', '0', '0', '0', '0', '0', '0', 'Мерил Буря Скверны to Player');

