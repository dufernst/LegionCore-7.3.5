-- Fix incorrect heirloom chest vendor prices
-- 122379 = Champion's Deathdealer Breastplate
-- 122380 = Mystical Vest of Elements
-- 122381 = Polished Breastplate of Valor
-- 122382 = Preened Ironfeather Breastplate
-- 122383 = Stained Shadowcraft Tunic
-- 122384 = Tattered Dreadmist Robe
-- 122387 = Burnished Breastplate of Might
-- 127010 = Pristine Lightforge Breastplate
UPDATE `npc_vendor` SET `money` = 5000000 WHERE `ExtendedCost` = 0 AND `item` IN (122379, 122380, 122381, 122382, 122383, 122384, 122387, 127010);

-- Fix incorrect heirloom shoulder vendor prices
-- 122355 = Polished Spaulders of Valor
-- 122356 = Champion Herod's Shoulder
-- 122357 = Mystical Pauldrons of Elements
-- 122358 = Stained Shadowcraft Spaulders
-- 122359 = Preened Ironfeather Shoulders
-- 122360 = Tattered Dreadmist Mantle
-- 122388 = Burnished Pauldrons of Might
UPDATE `npc_vendor` SET `money` = 5000000 WHERE `ExtendedCost` = 0 AND `item` IN (122355, 122356, 122357, 122358, 122359, 122360, 122388);

-- Fix incorrect heirloom trinket vendor prices
-- 122361 = Swift Hand of Justice
-- 122362 = Discerning Eye of the Beast
UPDATE `npc_vendor` SET `money` = 7000000 WHERE `ExtendedCost` = 0 AND `item` IN (122361, 122362);

-- Fix incorrect heirloom one-handed vendor prices
-- 122364 = Sharpened Scarlet Kris
-- 122367 = The Blessed Hammer of Grace
-- 122369 = Battleworn Thrash Blade
UPDATE `npc_vendor` SET `money` = 10000000 WHERE `ExtendedCost` = 0 AND `item` IN (122364, 122367, 122369);

-- Fix incorrect heirloom two-handed vendor prices
-- 122365 = Reforged Truesilver Champion
-- 122366 = Upgraded Dwarven Hand Cannon
-- 122368 = Grand Staff of Jordan
-- 140773 = Eagletalon Spear
UPDATE `npc_vendor` SET `money` = 15000000 WHERE `ExtendedCost` = 0 AND `item` IN (122365, 122366, 122368, 140773);

-- Fix incorrect heirloom off-hand vendor prices
-- 122390 = Musty Tome of the Lost
UPDATE `npc_vendor` SET `money` = 5000000 WHERE `ExtendedCost` = 0 AND `item` IN (122390);