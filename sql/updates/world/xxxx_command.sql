UPDATE `command` SET
    `help` = 'Syntax: .server idlerestart #delay [#exit_code] [reason]\n\nRestart the server after #delay seconds if no active connections are present (no players). Use #exit_code or 2 as program exit code.'
WHERE
    `name` = 'server idlerestart';

UPDATE `command` SET
    `help` = 'Syntax: .server idleshutdown #delay [#exit_code] [reason]\n\nShut the server down after #delay seconds if no active connections are present (no players). Use #exit_code or 0 as program exist code.'
WHERE
    `name` = 'server idleshutdown';

UPDATE `command` SET
    `help` = 'Syntax: .server restart #delay [#exit_code] [reason]\n\nRestart the server after #delay seconds. Use #exit_code or 2 as program exist code.'
WHERE
    `name` = 'server restart';

UPDATE `command` SET
    `help` = 'Syntax: .server shutdown #delay [#exit_code] [reason]\n\nShut the server down after #delay seconds. Use #exit_code or 0 as program exit code.'
WHERE
    `name` = 'server shutdown';
    
DELETE FROM `command` WHERE `name` IN ('server shutdown force','server restart force');
INSERT INTO `command` (`name`,`security`) VALUES ('server shutdown force', 6),('server restart force', 6);
UPDATE `command` SET `help`="Syntax: .server shutdown [force] #delay [#exit_code] [reason]
Shut the server down after #delay seconds. Use #exit_code or 0 as program exit code. Specify 'force' to allow short-term shutdown despite other players being connected." WHERE `name` IN ('server shutdown','server shutdown force');
UPDATE `command` SET `help`="Syntax: .server restart [force] #delay [#exit_code] [reason]
Restart the server after #delay seconds. Use #exit_code or 2 as program exit code. Specify 'force' to allow short-term shutdown despite other players being connected." WHERE `name` IN ('server restart','server restart force');

DELETE FROM `trinity_string` WHERE `entry` IN (11017,11018);
INSERT INTO `trinity_string` (`entry`,`content_default`) VALUES
(11017,"Server shutdown delayed to %d seconds as other users are still connected. Specify 'force' to override."),
(11018,"Server shutdown scheduled for T+%d seconds was successfully cancelled.");
