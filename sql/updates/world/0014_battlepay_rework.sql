CREATE TABLE `battlepay_tokens` (
	`tokenType` TINYINT(3) NOT NULL,
	`name` VARCHAR(255) NOT NULL DEFAULT '',
	`loginMessage` TEXT NULL DEFAULT NULL,
	`listIfNone` TINYINT(1) UNSIGNED NOT NULL DEFAULT '1',
	PRIMARY KEY (`tokenType`)
)
COLLATE='latin1_swedish_ci';

-- insert default token type
DELETE FROM battlepay_tokens WHERE `tokenType` = 1;
INSERT INTO battlepay_tokens (`tokenType`, `name`) VALUES (1, 'Battle Coins');

-- add TokenType column for each group
ALTER TABLE `battlepay_product_group`
	ADD COLUMN `TokenType` TINYINT(3) UNSIGNED NOT NULL AFTER `Ordering`,
	ADD COLUMN `IngameOnly` TINYINT(1) UNSIGNED NOT NULL DEFAULT '1' AFTER `TokenType`,
	ADD COLUMN `OwnsTokensOnly` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0' AFTER `IngameOnly`;

-- set default group to use tokenType 1
UPDATE `battlepay_product_group` SET TokenType = 1 WHERE GroupID = 1;

-- missing spell loot for Invasian Survival Kit (item id 141410)
-- not adding the 500 gold you should get from it, this is already
-- provided by the battlepay script
DELETE FROM `spell_loot_template` WHERE `entry` = 227022;
INSERT INTO `spell_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`) VALUES
(227022, 140276, 100, 1, 0, 20, 20),
(227022, 141527, 100, 1, 0, 20, 20),
(227022, 136569, 100, 1, 0, 5, 5);

DELETE FROM `trinity_string` WHERE `entry` = 14092;
INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES
(14092, 'Your level is too high to buy this service.');
