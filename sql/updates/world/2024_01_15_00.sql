-- Disable deprecated quests

-- 24752 = "The Arts of a Mage" (Horde)
-- 26198 = "The Arts of a Mage" (Alliance)

DELETE FROM `disables` WHERE `sourceType` = 1 AND `entry` IN (24752,26198);
INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES
(1, 24752, 0, '', '', 'Deprecated quest: The Arts of a Mage (Horde)'),
(1, 26198, 0, '', '', 'Deprecated quest: The Arts of a Mage (Alliance)');