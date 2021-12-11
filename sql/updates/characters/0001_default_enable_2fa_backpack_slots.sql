-- set the default inventory slots to 20
ALTER TABLE characters
	CHANGE COLUMN `inventorySlots` `inventorySlots` TINYINT(3) UNSIGNED NOT NULL DEFAULT '20' AFTER `horn`;

-- update the inventory slots for existing characters
UPDATE characters SET inventorySlots = 20;
