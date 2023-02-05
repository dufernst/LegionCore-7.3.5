ALTER TABLE `account_donate_token_log`
	ADD COLUMN `tokenType` TINYINT(3) UNSIGNED NOT NULL AFTER `change`,
	CHANGE COLUMN `change` `change` BIGINT(20) NOT NULL DEFAULT 0 AFTER `characterId`,
	CHANGE COLUMN `type` `buyType` TINYINT(3) UNSIGNED NOT NULL AFTER `tokenType`;

-- set to default token type for existing log entries
UPDATE `account_donate_token_log` SET `tokenType` = 1;

CREATE TABLE `account_tokens` (
	`account_id` INT(10) UNSIGNED NOT NULL,
	`tokenType` TINYINT(3) UNSIGNED NOT NULL,
	`amount` BIGINT(20) NOT NULL DEFAULT 0,
	PRIMARY KEY (`account_id`, `tokenType`)
)
COLLATE='latin1_swedish_ci';

-- move battle coin balance from account table to account_tokens table
INSERT INTO account_tokens
SELECT id, 1, balans
FROM account
WHERE balans > 0;

-- add first_ip colum to account table
ALTER TABLE `account`
	ADD COLUMN `first_ip` VARCHAR(15) NOT NULL DEFAULT '127.0.0.1' AFTER `joindate`,
	DROP COLUMN `referer`;

-- add first_ip colum to account table
ALTER TABLE `account`
	ADD COLUMN `referer` INT(10) UNSIGNED NOT NULL DEFAULT 0 AFTER `lang`;
