DELETE FROM `battlepay_display_info`;
INSERT INTO `battlepay_display_info` (`DisplayInfoId`, `CreatureDisplayInfoID`, `FileDataID`, `Flags`, `Name1`, `Name2`, `Name3`, `Name4`) VALUES
(109, 614740, 614740, 0, 'Level 90 Character Boost', '', 'There comes a time in every hero’s quest when they need a little boost to help get them over the hump and back into the action. With a Level 90 Character Boost, you can grant one character a one-time boost to level 90. Bring your hero up to speed and join the fight on the front lines.', '');

DELETE FROM `battlepay_display_info_locales`;
DELETE FROM `battlepay_display_info_visuals`;
DELETE FROM `battlepay_product`;
INSERT INTO `battlepay_product` (`ProductID`, `NormalPriceFixedPoint`, `CurrentPriceFixedPoint`, `Type`, `ChoiceType`, `Flags`, `DisplayInfoID`, `ScriptName`, `ClassMask`, `WebsiteType`) VALUES
(109, 100, 100, 0, 2, 0, 109, 'battlepay_service_level90', 0, 29);

DELETE FROM `battlepay_product_group`;
INSERT INTO `battlepay_product_group` (`GroupID`, `Name`, `IconFileDataID`, `DisplayType`, `Ordering`) VALUES
(3, 'Services', 1126583, 0, 3);

DELETE FROM `battlepay_product_group_locales`;
DELETE FROM `battlepay_product_item`;

DELETE FROM `battlepay_shop_entry`;
INSERT INTO `battlepay_shop_entry` (`EntryID`, `GroupID`, `ProductID`, `Ordering`, `Flags`, `BannerType`, `DisplayInfoID`) VALUES
(109, 3, 109, 5, 0, 0, 0);

UPDATE `trinity_string` SET `content_default`='You do not have enough tokens: %u' WHERE `entry`=20000;
UPDATE `trinity_string` SET `content_default`='|cff00ff00 %d Tokens were taken from your account |r' WHERE `entry`=20062;
DELETE FROM `trinity_string` WHERE `entry` IN (14091, 14092);
INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES
(14091, 'You need to be ingame to buy this service.'),
(14092, 'You need to be level 89 or lower to buy this service.');
