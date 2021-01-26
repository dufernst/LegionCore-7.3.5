DROP TABLE `store_statistics`;
DROP TABLE `store_history`;

CREATE TABLE `account_donate_token_log` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP(),
	`accountId` INT(10) UNSIGNED NOT NULL,
	`realmId` INT(10) UNSIGNED NOT NULL,
	`characterId` BIGINT(20) UNSIGNED NOT NULL,
	`change` INT(11) NOT NULL,
	`type` TINYINT(3) UNSIGNED NOT NULL,
	`productId` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`id`)
)
COLLATE='latin1_swedish_ci';

ALTER TABLE `account`
	CHANGE COLUMN `balans` `balans` INT(11) NOT NULL DEFAULT '0' AFTER `hwid`;
