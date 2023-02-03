ALTER TABLE `battlepay_product_group`
	CHANGE COLUMN `IconFileDataID` `IconFileDataID` INT(11) UNSIGNED NOT NULL AFTER `Name`,
	CHANGE COLUMN `Ordering` `Ordering` INT(11) UNSIGNED NOT NULL AFTER `DisplayType`,
	ADD COLUMN `Flags` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `Ordering`;
