
#ifndef __BattlegroundDefinesH
#define __BattlegroundDefinesH

namespace MS
{
    namespace Battlegrounds
    {
        static uint8 const CountOfPlayersToAverageWaitTime = 10;
        static uint8 const MaxArenaSlot = 2;
        static uint8 const BracketSlots = 4;
        static uint8 const MaxBrackets = 12;

        static TeamId GetTeamIdByTeam(uint32 team) { return team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
        static Team GetTeamByTeamId(TeamId teamID) { return teamID == TEAM_ALLIANCE ? ALLIANCE : HORDE; }
        static TeamId GetOtherTeamID(TeamId teamID) { return teamID == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE; }

        namespace IternalPvpTypes
        {
            enum : uint8
            {
                Battleground,
                Arena,
                Brawl,
                Skirmish,

                BrawlSix,
                Max
            };
        }

        namespace QueueOffsets
        {
            enum : int64
            {
                Battleground = 0x1F10000000000000,
                Wargame = 0x1F10000000020000
            };
        }

        namespace QueueGroupTypes
        {
            enum : uint8
            {
                PremadeAlliance     = 0,
                PremadeHorde        = 1,
                NormalAlliance      = 2,
                NormalHorde         = 3,

                Max,
            };
        }

        namespace PlayerPositionConstants
        {
            enum : uint8
            {
                IconNone = 0,
                IconHordeFlag = 1,
                IconAllianceFlag = 2,

                ArenaSlotNone = 1,
                ArenaSlot1 = 2,
                ArenaSlot2 = 3,
                ArenaSlot3 = 4,
                ArenaSlot4 = 5,
                ArenaSlot5 = 6
            };
        }

        namespace JoinType
        {
            enum : uint8
            {
                None = 0,
                Arena1v1 = 1,
                Arena2v2 = 2,
                Arena3v3 = 3,
                Skirmish2v2 = 4,
                Skirmish3v3 = 5,

                Brawl = 6,
                Arena5v5 = 8,

                RatedBG = 10,
                
                //< custom
                // ArenaSoloQ2v2   = 13,
                ArenaSoloQ3v3   = 13,
                // ArenaSoloQ5v5   = 15,
            };
        }
        
        static bool IsSkirmishJoinTypes(uint8 joinType)
        {
            switch (joinType)
            {
                case JoinType::Skirmish2v2:
                case JoinType::Skirmish3v3:
                    return true;
                default:
                    return false;
            }
        }

        namespace PvpInstanceType
        {
            enum : uint8
            {
                Battleground = 3,
                Arena = 4
            };
        }

        namespace BracketType
        {
            enum : uint8
            {
                Arena2v2 = 0,
                Arena3v3 = 1,
                Arena5v5 = 2,
                RatedBattleground = 3,
                Skirmish2v2 = 4,
                Skirmish3v3 = 5,

                Max = 6,

                ///< custom
                Brawl = 6,
                Arena1v1 = 7,
                ArenaSoloQ3v3 = 8,
            };
        }

        static uint8 GetBracketByJoinType(uint8 joinType)
        {
            switch (joinType)
            {
                case JoinType::Arena1v1:
                    return BracketType::Arena1v1;
                case JoinType::Arena2v2:
                    return BracketType::Arena2v2;
                case JoinType::Arena3v3:
                    return BracketType::Arena3v3;
                case JoinType::RatedBG:
                    return BracketType::RatedBattleground;
                case JoinType::Brawl:
                    return BracketType::Brawl;
                case JoinType::ArenaSoloQ3v3:
                    return BracketType::ArenaSoloQ3v3;
                default:
                    break;
            }

            return BracketType::Max;
        }

        namespace BattlegroundTypeId
        {
            enum : uint16
            {
                None = 0,
                BattlegroundAlteracValley = 1,
                BattlegroundWarsongGulch = 2,
                BattlegroundArathiBasin = 3,
                ArenaOldNagrandArena = 4,
                ArenaOldBladesEdgeArena = 5,
                ArenaAll = 6,
                BattlegroundEyeOfTheStorm = 7,
                ArenaRuinsOfLordaeron = 8,
                BattlegroundStrandOfTheAncients = 9,
                ArenaDalaranSewers = 10,
                ArenaOldRingOfValor = 11,
                BattlegroundIsleOfConquest = 30,
                BattlegroundRandom = 32,
                RatedBattleground = 100,
                RatedBattleground15v15 = 101,
                RatedBattleground25v25 = 102,
                BattlegroundTwinPeaks = 108,
                BattlegroundBattleForGilneas = 120,
                BattlegroundRatedEyeOfTheStorm = 656,
                BattlegroundKotmoguTemplate = 699,
                BATTLEGROUND_CTF3 = 706,
                BattlegroundSilvershardMines = 708,
                ArenaTolvir = 719,
                BattlegroundDeepwindGorge = 754,
                ArenaTheTigersPeak = 757,
                BrawlBattlegroundSouthshoreVsTarrenMill = 789,
                BrawlArenaTheTigersPeak2 = 803,
                ArenaBlackrookHold = 808,
                ArenaNagrandArena = 809,
                ArenaAshamanesFall = 816,
                ArenaBladesEdge = 844,

                BrawlBattleground1 = 846,
                BrawlBattlegroundArathiBasinWinter = 847,
                BrawlBattleground5 = 849,                   // Deepwind Gorge
                BrawlBattlegroundShadoPan = 853,
                BrawlBattleground8 = 857,
                BrawlBattlegroundHotmoguTemplate = 858, 
                BrawlEyeOfStorm = 859,                   // Eye of the Storm
                BrawlBattleground6 = 860,                   // Deepwind Gorge
                BrawlBattlegroundWarsongScramble = 861,
                BrawlBattleground7 = 862,                   // Eye of the Storm
                BrawlArenaAll = 866,
                BrawlArenaRuinsofLordaeron = 868,
                BrawlArenaDalaranSewers = 869,
                BrawlArenaTolViron = 870,
                BrawlArenaTheTigersPeak = 871,
                BrawlArenaBlackRookHold = 872,
                BrawlArenaNagrandArena = 873,
                BrawlArenaAshamanesFall = 874,
                BrawlArenaBladesEdge = 875,
                
                ///< custom
                BattlegroundDeathMatch           = 876,

                BrawlAllSix                         = 879,
                BrawlBattlegroundSilvershardSix        = 883,
                BrawlBattlegroundKotmoguTemplateSix = 884,
                BrawlBattlegroundWarsongSix         = 886,

                BrawlBattlegroundSeethingShore = 890,
                BattlegroundSeethingShore = 894,

                Max
            };
        }

        namespace BattlegroundQueueTypeId
        {
            enum : uint8
            {
                None,
                Arena1v1,
                Arena2v2,
                Arena3v3,
                Arena5v5,
                BattlegroundAlteracValley,
                BattlegroundWarsongGulch,
                BattlegroundArathiBasin,
                BattlegroundEyeOfTheStorm,
                BattlegroundStrandOfTheAncients,
                BattlegroundIsleOfConquest,
                BattlegroundTwinPeaks,
                BattlegroundBattleForGilneas,
                BattlegroundKotmoguTemplate,
                BattlegroundSilvershardMines,
                BattlegroundDeepwindGorge,
                BattlegroundSeethingShore,
                BattlegroundRandom,
                RatedBattleground,
                Brawl,

                ///< custom
                BattlegroundDeathMatch,
                ArenaSoloQ3v3,

                Max
            };
        }

        static bool CheckIsArenaTypeByBgTypeID(uint16 bgTypeID)
        {
            switch (bgTypeID)
            {
                case BattlegroundTypeId::ArenaAll:
                case BattlegroundTypeId::ArenaBladesEdge:
                case BattlegroundTypeId::ArenaNagrandArena:
                case BattlegroundTypeId::ArenaDalaranSewers:
                case BattlegroundTypeId::ArenaRuinsOfLordaeron:
                case BattlegroundTypeId::ArenaTheTigersPeak:
                case BattlegroundTypeId::ArenaBlackrookHold:
                case BattlegroundTypeId::ArenaAshamanesFall:
                case BattlegroundTypeId::ArenaTolvir:
                    return true;
                default:
                    return false;
            }
        }

        static bool CheckIsBattlegroundTypeByBgTypeID(uint16 bgTypeID)
        {
            switch (bgTypeID)
            {
                case BattlegroundTypeId::BattlegroundAlteracValley:
                case BattlegroundTypeId::BattlegroundArathiBasin:
                case BattlegroundTypeId::BattlegroundBattleForGilneas:
                case BattlegroundTypeId::BattlegroundDeepwindGorge:
                case BattlegroundTypeId::BattlegroundEyeOfTheStorm:
                case BattlegroundTypeId::BattlegroundIsleOfConquest:
                case BattlegroundTypeId::BattlegroundKotmoguTemplate:
                case BattlegroundTypeId::BattlegroundRandom:
                case BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm:
                case BattlegroundTypeId::BattlegroundSilvershardMines:
                case BattlegroundTypeId::BattlegroundStrandOfTheAncients:
                case BattlegroundTypeId::BattlegroundTwinPeaks:
                case BattlegroundTypeId::BattlegroundWarsongGulch:
                case BattlegroundTypeId::RatedBattleground: // is rly needed there?
                case BattlegroundTypeId::BattlegroundSeethingShore:
                    return true;
                default:
                    return false;
            }
        }

        static bool CheckIsBrawlByTypeID(uint16 bgTypeID)
        {
            switch (bgTypeID)
            {
                case BattlegroundTypeId::BrawlBattlegroundSouthshoreVsTarrenMill:
                case BattlegroundTypeId::BrawlBattleground1:
                case BattlegroundTypeId::BrawlBattlegroundArathiBasinWinter:
                case BattlegroundTypeId::BrawlBattleground5:
                case BattlegroundTypeId::BrawlBattlegroundShadoPan:
                case BattlegroundTypeId::BrawlBattleground8:
                case BattlegroundTypeId::BrawlBattlegroundKotmoguTemplateSix:
                case BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate:
                case BattlegroundTypeId::BrawlBattlegroundSilvershardSix:
                case BattlegroundTypeId::BrawlBattlegroundWarsongSix:
                case BattlegroundTypeId::BrawlEyeOfStorm:
                case BattlegroundTypeId::BrawlBattleground6:
                case BattlegroundTypeId::BrawlBattlegroundWarsongScramble:
                case BattlegroundTypeId::BrawlBattleground7:
                case BattlegroundTypeId::BrawlArenaAll:
                case BattlegroundTypeId::BrawlArenaRuinsofLordaeron:
                case BattlegroundTypeId::BrawlArenaDalaranSewers:
                case BattlegroundTypeId::BrawlArenaTolViron:
                case BattlegroundTypeId::BrawlArenaTheTigersPeak:
                case BattlegroundTypeId::BrawlArenaBlackRookHold:
                case BattlegroundTypeId::BrawlArenaNagrandArena:
                case BattlegroundTypeId::BrawlArenaAshamanesFall:
                case BattlegroundTypeId::BrawlArenaBladesEdge:
                case BattlegroundTypeId::BrawlAllSix:
                    return true;
                default:
                    return false;
            }
        }

        static uint8 GetJoinTypeByBracketSlot(uint8 slot)
        {
            switch (slot)
            {
                case BracketType::Arena1v1:
                    return JoinType::Arena1v1;
                case BracketType::Arena2v2:
                case BracketType::Skirmish2v2:
                    return JoinType::Arena2v2;
                case BracketType::Arena3v3:
                case BracketType::Skirmish3v3:
                    return JoinType::Arena3v3;
                case BracketType::Arena5v5:
                    return JoinType::Arena5v5;
                case BracketType::RatedBattleground:
                    return JoinType::RatedBG;
                case BracketType::Brawl:
                    return JoinType::Brawl;
                case BracketType::ArenaSoloQ3v3:
                    return JoinType::ArenaSoloQ3v3;
                default:
                    return 0xFF;
            }
        }

        static uint8 GetBgQueueTypeIdByBgTypeID(uint16 bgTypeID, uint8 joinType = 0)
        {
            switch (bgTypeID)
            {
                case BattlegroundTypeId::BattlegroundWarsongGulch:
                    return BattlegroundQueueTypeId::BattlegroundWarsongGulch;
                case BattlegroundTypeId::RatedBattleground:
                    return BattlegroundQueueTypeId::RatedBattleground;
                case BattlegroundTypeId::BattlegroundArathiBasin:
                    return BattlegroundQueueTypeId::BattlegroundArathiBasin;
                case BattlegroundTypeId::BattlegroundAlteracValley:
                    return BattlegroundQueueTypeId::BattlegroundAlteracValley;
                case BattlegroundTypeId::BattlegroundEyeOfTheStorm:
                case BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm:
                    return BattlegroundQueueTypeId::BattlegroundEyeOfTheStorm;
                case BattlegroundTypeId::BattlegroundStrandOfTheAncients:
                    return BattlegroundQueueTypeId::BattlegroundStrandOfTheAncients;
                case BattlegroundTypeId::BattlegroundIsleOfConquest:
                    return BattlegroundQueueTypeId::BattlegroundIsleOfConquest;
                case BattlegroundTypeId::BattlegroundTwinPeaks:
                    return BattlegroundQueueTypeId::BattlegroundTwinPeaks;
                case BattlegroundTypeId::BattlegroundBattleForGilneas:
                    return BattlegroundQueueTypeId::BattlegroundBattleForGilneas;
                case BattlegroundTypeId::BattlegroundRandom:
                    return BattlegroundQueueTypeId::BattlegroundRandom;
                case BattlegroundTypeId::BattlegroundKotmoguTemplate:
                    return BattlegroundQueueTypeId::BattlegroundKotmoguTemplate;
                case BattlegroundTypeId::BattlegroundSilvershardMines:
                    return BattlegroundQueueTypeId::BattlegroundSilvershardMines;
                case BattlegroundTypeId::BattlegroundDeepwindGorge:
                    return BattlegroundQueueTypeId::BattlegroundDeepwindGorge;
                case BattlegroundTypeId::BattlegroundSeethingShore:
                    return BattlegroundQueueTypeId::BattlegroundSeethingShore;
                case BattlegroundTypeId::BrawlBattlegroundWarsongScramble:
                case BattlegroundTypeId::BrawlBattlegroundSouthshoreVsTarrenMill:
                case BattlegroundTypeId::BrawlBattleground1:
                case BattlegroundTypeId::BrawlBattlegroundArathiBasinWinter:
                case BattlegroundTypeId::BrawlBattleground5:                   // Deepwind Gorge
                case BattlegroundTypeId::BrawlBattlegroundShadoPan:
                case BattlegroundTypeId::BrawlBattleground8:
                case BattlegroundTypeId::BrawlBattlegroundKotmoguTemplateSix:
                case BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate:
                case BattlegroundTypeId::BrawlBattlegroundSilvershardSix:
                case BattlegroundTypeId::BrawlBattlegroundWarsongSix:
                case BattlegroundTypeId::BrawlEyeOfStorm:                   // Eye of the Storm
                case BattlegroundTypeId::BrawlBattleground6:                   // Deepwind Gorge
                case BattlegroundTypeId::BrawlBattleground7:                   // Eye of the Storm
                case BattlegroundTypeId::BrawlArenaAll:
                case BattlegroundTypeId::BrawlArenaRuinsofLordaeron:
                case BattlegroundTypeId::BrawlArenaDalaranSewers:
                case BattlegroundTypeId::BrawlArenaTolViron:
                case BattlegroundTypeId::BrawlArenaTheTigersPeak:
                case BattlegroundTypeId::BrawlArenaBlackRookHold:
                case BattlegroundTypeId::BrawlArenaNagrandArena:
                case BattlegroundTypeId::BrawlArenaAshamanesFall:
                case BattlegroundTypeId::BrawlArenaBladesEdge:
                case BattlegroundTypeId::BrawlAllSix:
                    return BattlegroundQueueTypeId::Brawl;
                case BattlegroundTypeId::ArenaAll:
                case BattlegroundTypeId::ArenaNagrandArena:
                case BattlegroundTypeId::ArenaRuinsOfLordaeron:
                case BattlegroundTypeId::ArenaBladesEdge:
                case BattlegroundTypeId::ArenaDalaranSewers:
                case BattlegroundTypeId::ArenaTheTigersPeak:
                case BattlegroundTypeId::ArenaTolvir:
                case BattlegroundTypeId::ArenaBlackrookHold:
                case BattlegroundTypeId::ArenaAshamanesFall:
                    switch (joinType)
                    {
                        case JoinType::Arena1v1: ///< custom
                            return BattlegroundQueueTypeId::Arena1v1;
                        case JoinType::Arena2v2:
                            return BattlegroundQueueTypeId::Arena2v2;
                        case JoinType::Arena3v3:
                            return BattlegroundQueueTypeId::Arena3v3;
                        case JoinType::Arena5v5:
                            return BattlegroundQueueTypeId::Arena5v5;
                        case JoinType::ArenaSoloQ3v3:
                            return BattlegroundQueueTypeId::ArenaSoloQ3v3;
                        default:
                            return BattlegroundQueueTypeId::None;
                    }

                ///< custom
                case BattlegroundTypeId::BattlegroundDeathMatch:
                    return BattlegroundQueueTypeId::BattlegroundDeathMatch;
                default:
                    return BattlegroundQueueTypeId::None;
            }
        }

        static uint16 GetBgTemplateIdByQueueTypeID(uint8 bgQueueTypeId)
        {
            switch (bgQueueTypeId)
            {
                case BattlegroundQueueTypeId::BattlegroundWarsongGulch:
                    return BattlegroundTypeId::BattlegroundWarsongGulch;
                case BattlegroundQueueTypeId::BattlegroundArathiBasin:
                    return BattlegroundTypeId::BattlegroundArathiBasin;
                case BattlegroundQueueTypeId::BattlegroundAlteracValley:
                    return BattlegroundTypeId::BattlegroundAlteracValley;
                case BattlegroundQueueTypeId::BattlegroundEyeOfTheStorm:
                    return BattlegroundTypeId::BattlegroundEyeOfTheStorm; // BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm?
                case BattlegroundQueueTypeId::BattlegroundStrandOfTheAncients:
                    return BattlegroundTypeId::BattlegroundStrandOfTheAncients;
                case BattlegroundQueueTypeId::BattlegroundIsleOfConquest:
                    return BattlegroundTypeId::BattlegroundIsleOfConquest;
                case BattlegroundQueueTypeId::BattlegroundTwinPeaks:
                    return BattlegroundTypeId::BattlegroundTwinPeaks;
                case BattlegroundQueueTypeId::BattlegroundSeethingShore:
                    return BattlegroundTypeId::BattlegroundSeethingShore;
                case BattlegroundQueueTypeId::BattlegroundBattleForGilneas:
                    return BattlegroundTypeId::BattlegroundBattleForGilneas;
                case BattlegroundQueueTypeId::BattlegroundRandom:
                    return BattlegroundTypeId::BattlegroundRandom;
                case BattlegroundQueueTypeId::BattlegroundKotmoguTemplate:
                    return BattlegroundTypeId::BattlegroundKotmoguTemplate;
                case BattlegroundQueueTypeId::BattlegroundSilvershardMines:
                    return BattlegroundTypeId::BattlegroundSilvershardMines;
                case BattlegroundQueueTypeId::BattlegroundDeepwindGorge:
                    return BattlegroundTypeId::BattlegroundDeepwindGorge;
                case BattlegroundQueueTypeId::Arena2v2:
                case BattlegroundQueueTypeId::Arena3v3:
                case BattlegroundQueueTypeId::Arena5v5:
                    return BattlegroundTypeId::ArenaAll;
                case BattlegroundQueueTypeId::RatedBattleground:
                    return BattlegroundTypeId::RatedBattleground;

                    ///< custom
                case BattlegroundQueueTypeId::BattlegroundDeathMatch:
                    return BattlegroundTypeId::BattlegroundDeathMatch;
                case BattlegroundQueueTypeId::ArenaSoloQ3v3:
                case BattlegroundQueueTypeId::Arena1v1:
                    return BattlegroundTypeId::ArenaAll;
                default:
                    return BattlegroundTypeId::None;
            }
        }

        static uint8 GetBgJoinTypeByQueueTypeID(uint8 bgQueueTypeId)
        {
            switch (bgQueueTypeId)
            {
                case BattlegroundQueueTypeId::Arena1v1:
                    return JoinType::Arena1v1;
                case BattlegroundQueueTypeId::Arena2v2:
                    return JoinType::Arena2v2;
                case BattlegroundQueueTypeId::Arena3v3:
                    return JoinType::Arena3v3;
                case BattlegroundQueueTypeId::RatedBattleground:
                    return JoinType::RatedBG;
                case BattlegroundQueueTypeId::Brawl:
                    return JoinType::Brawl;
                case BattlegroundQueueTypeId::ArenaSoloQ3v3:
                    return JoinType::ArenaSoloQ3v3;
                default:
                    return JoinType::None;
            }
        }

        namespace BrawlTypes
        {
            enum : uint8
            {
                None = 0,
                Battleground = 1,
                Arena = 2,
                Lfg = 3,
            };
        }

        namespace GroupJoinBattlegroundResult
        {
            enum : uint8
            {
                ERR_BATTLEGROUND_NONE                           = 0,
                ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS           = 2,        // You cannot join the battleground yet because you or one of your party members is flagged as a Deserter.
                ERR_ARENA_TEAM_PARTY_SIZE                       = 3,        // Incorrect party size for this arena.
                ERR_BATTLEGROUND_TOO_MANY_QUEUES                = 4,        // You can only be queued for 2 battles at once
                ERR_BATTLEGROUND_CANNOT_QUEUE_FOR_RATED         = 5,        // You cannot queue for a rated match while queued for other battles
                ERR_BATTLEDGROUND_QUEUED_FOR_RATED              = 6,        // You cannot queue for another battle while queued for a rated arena match
                ERR_BATTLEGROUND_TEAM_LEFT_QUEUE                = 7,        // Your team has left the arena queue
                ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND            = 8,        // You can't do that in a battleground.
                ERR_BATTLEGROUND_JOIN_XP_GAIN                   = 9,
                ERR_BATTLEGROUND_JOIN_RANGE_INDEX               = 10,       // Cannot join the queue unless all members of your party are in the same battleground level range.
                ERR_BATTLEGROUND_JOIN_TIMED_OUT                 = 11,       // %s was unavailable to join the queue. (uint64 guid exist in client cache)
                //ERR_BATTLEGROUND_JOIN_TIMED_OUT               = 12,       // same as 11
                //ERR_BATTLEGROUND_TEAM_LEFT_QUEUE              = 13,       // same as 7
                ERR_LFG_CANT_USE_BATTLEGROUND                   = 14,       // You cannot queue for a battleground or arena while using the dungeon system.
                ERR_IN_RANDOM_BG                                = 15,       // Can't do that while in a Random Battleground queue.
                ERR_IN_NON_RANDOM_BG                            = 16,       // Can't queue for Random Battleground while in another Battleground queue.
                ERR_BG_DEVELOPER_ONLY                           = 17,
                ERR_BATTLEGROUND_INVITATION_DECLINED            = 18,
                ERR_MEETING_STONE_NOT_FOUND                     = 19,
                ERR_WARGAME_REQUEST_FAILURE                     = 20,
                ERR_BATTLEFIELD_TEAM_PARTY_SIZE                 = 22,
                ERR_NOT_ON_TOURNAMENT_REALM                     = 23,
                ERR_BATTLEGROUND_PLAYERS_FROM_DIFFERENT_REALMS  = 24,
                ERR_LEAVE_QUEUE                                 = 30,       // just leave queue
                ERR_BATTLEGROUND_JOIN_LEVELUP                   = 33,
                ERR_REMOVE_FROM_PVP_QUEUE_FACTION_CHANGE        = 34,
                ERR_BATTLEGROUND_JOIN_FAILED                    = 35,
                ERR_BATTLEGROUND_DUPE_QUEUE                     = 43,
                ERR_BATTLEGROUND_JOIN_NO_VALID_SPEC_FOR_ROLE    = 44,
                ERR_BATTLEGROUND_JOIN_RESPEC                    = 45,
                ERR_BATTLEGROUND_JOIN_MUST_COMPLETE_QUEST       = 47,
                ERR_RESTRICTED_ACCOUNT                          = 48,
                ERR_BATTLEGROUND_JOIN_MERCENARY                 = 49,
                ERR_BATTLEGROUND_JOIN_TOO_MANY_HEALERS          = 51,
                ERR_BATTLEGROUND_JOIN_TOO_MANY_TANKS            = 52,
                ERR_BATTLEGROUND_JOIN_TOO_MANY_DAMAGE           = 53,
                ERR_GROUP_JOIN_BATTLEGROUND_DEAD                = 57
            };
        }

        static std::initializer_list<uint16> RatedBattlegroundsContainer = {BattlegroundTypeId::BattlegroundArathiBasin, BattlegroundTypeId::BattlegroundDeepwindGorge, BattlegroundTypeId::BattlegroundWarsongGulch, BattlegroundTypeId::BattlegroundBattleForGilneas, BattlegroundTypeId::BattlegroundTwinPeaks, BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm, BattlegroundTypeId::BattlegroundSilvershardMines};

        static auto IsRandomGeneratedBg = [](uint32 bgTypeID) -> bool
        {
            switch (bgTypeID)
            {
                case BattlegroundTypeId::BattlegroundRandom:
                case BattlegroundTypeId::ArenaAll:
                case BattlegroundTypeId::RatedBattleground:
                case BattlegroundTypeId::BrawlArenaAll:
                case BattlegroundTypeId::BrawlAllSix:
                    return true;
                default:
                    return false;
            }
        };
    }
}

#endif
