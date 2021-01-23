-- transformation spell is triggered by Brood Queen Tyranna's Queen's Bite ability and is not self cast
DELETE FROM creature_template_spell WHERE entry = 97244 AND spell = 197505;
DELETE FROM creature_template_spell WHERE entry = 97959 AND spell = 197505;
DELETE FROM creature_template_spell WHERE entry = 97962 AND spell = 197598;
DELETE FROM creature_template_spell WHERE entry = 98712 AND spell = 197598;

-- add the required spell scripts for quest 38728 - The Keystone
DELETE FROM spell_script_names WHERE spell_id IN (197486, 208121, 197523, 197505, 197598);
INSERT INTO spell_script_names (spell_id, ScriptName) VALUES
(197486, 'spell_legion_197486'),
(208121, 'spell_legion_208121'),
(197523, 'spell_legion_197523'),
(197505, 'spell_legion_197505_197598'),
(197598, 'spell_legion_197505_197598');
