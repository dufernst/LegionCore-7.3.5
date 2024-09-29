-- Fix broken flight paths
DELETE FROM `taxi_nodes` WHERE `ID` IN (45, 72, 73, 602);
INSERT INTO `taxi_nodes` (`ID`, `Name`, `PosX`, `PosY`, `PosZ`, `MountCreatureID1`, `MountCreatureID2`, `MapOffset1`, `MapOffset2`, `Facing`, `FlightMapOffset1`, `FlightMapOffset2`, `ContinentID`, `ConditionID`, `CharacterBitNumber`, `Flags`, `UiTextureKitID`, `SpecialIconConditionID`) VALUES
(45, 'Nethergarde Keep, Blasted Lands', -11112.3, -3435.74, 79.09, 0, 541, 0, 0.015, 0, 0, 0, 0, 0, 45, 1, 0, 0),
(72, 'Cenarion Hold, Silithus', -6811.39, 836.74, 49.81, 2224, 0, 0, 0, 0, 0, 0, 1, 0, 72, 2, 0, 0),
(73, 'Cenarion Hold, Silithus', -6761.83, 772.03, 88.91, 0, 3837, 0, -0.005, 0, 0, 0, 1, 0, 73, 1, 0, 0),
(602, 'Surwich, Blasted Lands', -12761.9, -2919.04, 7.04836, 0, 3837, -0.008, 0.005, 0, 0, 0, 0, 0, 110, 1, 0, 0);

-- Reference for 'TableHash': https://github.com/TrinityCore/WowPacketParser/blob/master/WowPacketParser/Enums/DB2Hash.cs
DELETE FROM `hotfix_data` WHERE `Id` IN (45, 72, 73, 602);
INSERT INTO `hotfix_data` (`Id`, `TableHash`, `RecordID`, `Timestamp`, `Deleted`) VALUES
(45, 1356405368, 45, 0, 0),
(72, 1356405368, 72, 0, 0),
(73, 1356405368, 73, 0, 0),
(602, 1356405368, 602, 0, 0);