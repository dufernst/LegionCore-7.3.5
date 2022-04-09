ALTER TABLE `garr_mission`
	CHANGE COLUMN `CriteriaID` `RelationshipData` INT(11) NOT NULL DEFAULT '0' AFTER `EnvGarrMechanicID`;
