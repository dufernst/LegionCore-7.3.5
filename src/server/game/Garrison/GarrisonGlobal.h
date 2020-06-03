////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef GARRISON_GLOBAL_H
#define GARRISON_GLOBAL_H

#include "Define.h"

enum GarrisonFactionIndex
{
    GARRISON_FACTION_INDEX_HORDE = 0,
    GARRISON_FACTION_INDEX_ALLIANCE = 1
};

namespace GarrisonConst
{
    namespace Globals
    {
        enum
        {
            MaxLevel = 3,
            MaxFollowerPerMission = 3,
            MaxFollowerLevel = 100,
            MaxFollowerLevelHall = 110,
            FollowerActivationCost = (250 * GOLD),
            FollowerActivationMaxStack = 1,
            BaseMap = 1116,
            PlotInstanceCount = 42,
            CurrencyID = 824,
            CacheMinToken = 5,
            CacheHeftyToken = 200,
            CacheMaxToken = 500,
            CacheTokenGenerateTime = (10 * MINUTE),
            MissionDistributionInterval = (25 * MINUTE),
            DefaultFollowerItemLevel = 600,
            ShipyardBuildingType = 9,
            ShipyardBuildingID = 205,
            ShipyardPlotID = 98,
            MaxActiveFollowerAllowedCount = 20
        };

        static bool isClassHallMap(uint16 ID)
        {
            return ID == 1513 || ID == 1479 || ID == 1107;
        }
    }

    namespace FactionIndex
    {
        enum Type : uint8
        {
            Horde = 0,
            Alliance = 1,
            Max = 2
        };
    }

    namespace Mission
    {
        enum State : uint8
        {
            Available = 0,
            InProgress = 1,
            CompleteSuccess = 2,
            CompleteFailed = 5
        };

        enum Type : uint8
        {
            Combat = 3,
            Generic = 4,
            Salvage = 5,
            Logistics = 6,
            Wildlife = 7,
            Trading = 8,
            Construction = 9,
            Provision = 10,
            Recruitement = 11,
            Training = 12,
            Patrol = 13,
            Research = 14,
            Defense = 15,
            Exploration = 16,
            Siege = 17,
            Alchemy = 18,
            BlackSmithing = 19,
            Enchanting = 20,
            Engineering = 21,
            Inscription = 22,
            JewelCrafting = 23,
            LeatherWorking = 24,
            Tailoring = 25,
            Treasure = 35,
            PetBattle = 36,
            ShipCombat = 37,
            ShipOil = 38,
            ShipTraining = 39,
            ShipTreasure = 40,
            ShipSiegeA = 41,
            ShipSiegeH = 42,
            ShipBonus = 47,
            ShipLegendary = 48,
            ShipSiegeIHA = 49,
            ShipSiegeIHH = 50,
            ZoneSupportA = 51,
            Invasion = 52,
            ArtifactMonk = 53,
            GenericLegion = 54,
            Tutoriallegion = 55,
            ZoneSupportH = 57,
            ArtifactShaman = 58,
            ArtifactDruid = 59,
            ArtifactMage = 60,
            ArtifactHunter = 61,
            ArtifactPaladin = 63,
            ArtifactWarlock = 64,
            ArtifactDH = 65,
            ArtifactRogue = 66,
            ArtifactPrist = 67,
            ArtifactDK = 68,
            ArtifactWarrior = 69,
            CH_Campaign = 70,
            CH_Generic = 71,
            CH_Treasure = 72,
            CH_SprecialRew = 73,
            CH_Quest = 74,
            CH_TreasureDung = 75,
            CH_TreasureRaid = 76,
        };

        enum Flags : uint8
        {
            Rare = 0x01,
            Unk2 = 0x02,
            Exhausting = 0x04
        };

        enum BonusRollResults : uint8
        {
            Ok = 0,
            Error = 1
        };
    }

    namespace FollowerType
    {
        static uint32 MaxLevel[5] = { 0, Globals::MaxFollowerLevel, Globals::MaxFollowerLevel, 0, Globals::MaxFollowerLevelHall };

        enum : uint32
        {
            Garrison = 1,
            Shipyard = 2,
            Follower = 4
        };
    }

    namespace AbilityEffectTypes
    {
        enum Type
        {
            ModUnk0 = 0,    ///< @TODO
            ModWinRateSolo = 1,    ///< Proc if MissionFollowerCount == 1
            ModWinRate = 2,    ///< Proc every time
            ModTravelTime = 3,    ///< Proc every time
            ModXpGain = 4,    ///< Mod the XP earn (self, party)
            ModWinRateClass = 5,    ///< Proc if Find(MissionFollowers[Class], MiscValueA) != NULL
            ModWinRateDurationMore = 6,    ///< Proc if Duration > (3600 * MiscValueB)
            ModWinRateDurationLess = 7,    ///< Proc if Duration < (3600 * MiscValueB)
            ModGarrCurrencyDrop = 8,    ///< Implemented.
            ModWinRateTravelDurationMore = 9,    ///< Proc if TravelDuration > (3600 * MiscValueB)
            ModWinRateTravelDurationLess = 10,   ///< Proc if TravelDuration < (3600 * MiscValueB)
            ModUnk11 = 11,   ///< UNUSED
            ModDummyProduction = 12,   ///< @TODO
            ModBronzeTreasureDrop = 13,   ///< @TODO
            ModSilverTreasureDrop = 14,   ///< @TODO
            ModGoldTreasureDrop = 15,   ///< @TODO
            ModChestDropRate = 16,   ///< @TODO
            ModMissionDuration = 17,    ///< Proc every time
            ModWinRateFirstDaySend = 23,   ///<  first mission you send each day
            ModTroopNumber = 34,
            ModUnk35 = 35,
            ModUnk37 = 37,
            ModUnk39 = 39,
            ModUnk41 = 41
        };
    }

    namespace MechanicTypes
    {
        enum Type
        {
            Environment = 0,
            Racial = 1,
            Ability = 2
        };
    }

    namespace GarrisonFollowerFlags
    {
        enum : uint32
        {
            FOLLOWER_STATUS_BASE = 0x00,
            FOLLOWER_STATUS_FAVORITE = 0x01,
            FOLLOWER_STATUS_EXHAUSTED = 0x02,
            FOLLOWER_STATUS_INACTIVE = 0x04,
            FOLLOWER_STATUS_TROOP = 0x08,
            FOLLOWER_STATUS_NO_XP_GAIN = 0x10
        };
    }

    static const uint16 TroopLimits[5] =
    {
        0,
        1,
        3,
        2,
        0
    };

    namespace ClassHallTalentFlag
    {
        enum : uint8
        {
            CLASS_HALL_TALENT_IN_RESEARCH = 0,
            CLASS_HALL_TALENT_READY = 1,
            CLASS_HALL_TALENT_CHANGE = 2,
        };
    }
}   ///< namespace Garrison

#endif  ///< GARRISON_GLOBAL_H
