/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/// \addtogroup u2w
/// @{
/// \file

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H

#include "AddonMgr.h"
#include "Common.h"
#include "Cryptography/BigNumber.h"
#include "EventProcessor.h"
#include "FunctionProcessor.h"
#include "Opcodes.h"
#include "Packet.h"
#include "SharedDefines.h"
#include "World.h"
#include "WorldPacket.h"
#include "DatabaseEnvFwd.h"

class PetBattle;
class BattlePet;
class BlackMarketEntry;
class Channel;
class CollectionMgr;
class Creature;
class GameObject;
class InstanceSave;
class Item;
class Object;
class ObjectGuid;
class Player;
class Quest;
class SpellCastTargets;
class Unit;
class Warden;
class WorldObject;
class WorldPacket;
class WorldSocket;
class LoginQueryHolder;
class Map;
class BattlepayManager;
struct AreaTableEntry;
struct AuctionEntry;
struct BlackMarketTemplate;
struct DeclinedName;
struct ItemTemplate;
struct MovementInfo;
struct PetBattleRequest;
struct Position;
struct CharacterTemplate;

enum class BattlePetError : uint16;

namespace lfg
{
struct LfgJoinResultData;
struct LfgPlayerBoot;
struct LfgProposal;
struct LfgQueueStatusData;
struct LfgPlayerRewardData;
struct LfgRoleCheck;
struct LfgUpdateData;
}

enum AuthFlags
{
    AT_AUTH_FLAG_NONE                       = 0x0,
    AT_AUTH_FLAG_90_LVL_UP                  = 0x1,
    AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER  = 0x2,
};

namespace WorldPackets
{
    namespace Artifact
    {
        class AddPower;
        class ConfirmRespec;
        class SetAppearance;
        class ArtifactAddRelicTalent;
        class ArtifactAttunePreviewdRelic;
        class ArtifactAttuneSocketedRelic;
    }

    namespace Character
    {
        struct CharacterCreateInfo;
        struct CharacterRenameInfo;
        struct CharCustomizeInfo;
        struct CharRaceOrFactionChangeInfo;
        struct CharacterUndeleteInfo;

        class EnumCharacters;
        class CreateChar;
        class DeleteChar;
        class CharacterRenameRequest;
        class CharCustomize;
        class CharRaceOrFactionChange;
        class GenerateRandomCharacterName;
        class ReorderCharacters;
        class UndeleteCharacter;
        class PlayerLogin;
        class LogoutRequest;
        class LogoutCancel;
        class LogoutInstant;
        class LoadingScreenNotify;
        class GetUndeleteCharacterCooldownStatus;
        class SetActionBarToggles;
        class RequestPlayedTime;
        class SetTitle;
        class AlterApperance;
        class SetFactionAtWar;
        class SetFactionNotAtWar;
        class SetFactionInactive;
        class SetWatchedFaction;
        class EmoteClient;
        class SetPlayerDeclinedNames;
        class CharCustomize;
        class NeutralPlayerSelectFaction;
        class UndeleteCharacter;
        class SetCurrencyFlags;
        class EngineSurvey;

        enum class LoginFailureReason : uint8;
    }

    namespace ClientConfig
    {
        class RequestAccountData;
        class UserClientUpdateAccountData;
        class SetAdvancedCombatLogging;
        class UpdateClientSettings;
        class SaveClientVariables;
        class GetRemainingGameTime;
    }

    namespace Channel
    {
        class ChannelPlayerCommand;
        class JoinChannel;
        class LeaveChannel;
    }

    namespace Chat
    {
        class ChatMessage;
        class ChatMessageWhisper;
        class ChatMessageChannel;
        class ChatAddonMessage;
        class ChatAddonMessageWhisper;
        class ChatAddonMessageChannel;
        class ChatMessageAFK;
        class ChatMessageDND;
        class ChatMessageEmote;
        class CTextEmote;
        class ChatRegisterAddonPrefixes;
        class ChatUnregisterAllAddonPrefixes;
        class ChatReportIgnored;
    }

    namespace Combat
    {
        class AttackSwing;
        class AttackStop;
        class SetSheathed;
    }

    namespace Commentator
    {
        class CommentatorEnable;
        class CommentatorGetMapInfo;
        class CommentatorGetPlayerInfo;
        class CommentatorEnterInstance;
        class CommentatorExitInstance;
    }

    namespace Duel
    {
        class DuelResponse;
        class CanDuel;
    }

    namespace EquipmentSet
    {
        class SaveEquipmentSet;
        class UseEquipmentSet;
        class DeleteEquipmentSet;
        class AssignEquipmentSetSpec;
    }

    namespace GameObject
    {
        class GameObjReportUse;
        class GameObjectUse;
    }

    namespace Guild
    {
        class QueryGuildInfo;
        class GuildInviteByName;
        class GuildOfficerRemoveMember;
        class AcceptGuildInvite;
        class GuildDeclineInvitation;
        class GuildEventLogQuery;
        class GuildGetRoster;
        class GuildPromoteMember;
        class GuildDemoteMember;
        class GuildAssignMemberRank;
        class GuildLeave;
        class GuildDelete;
        class GuildSetGuildMaster;
        class GuildUpdateMotdText;
        class GuildNewsUpdateSticky;
        class GuildSetMemberNote;
        class GuildGetRanks;
        class GuildQueryNews;
        class GuildShiftRank;
        class GuildSetRankPermissions;
        class GuildAddRank;
        class GuildDeleteRank;
        class GuildUpdateInfoText;
        class RequestGuildPartyState;
        class AutoDeclineGuildInvites;
        class RequestGuildRewardsList;
        class QueryMemberRecipes;
        class QueryGuildMembersForRecipe;
        class QueryRecipes;
        class SaveGuildEmblem;
        class GuildChangeNameRequest;
        class ReplaceGuildMaster;
        class GuildSetAchievementTracking;
        class GuildAutoDeclineInvitation;

        class GuildPermissionsQuery;
        class GuildBankRemainingWithdrawMoneyQuery;
        class GuildBankActivate;
        class GuildBankQueryTab;
        class GuildBankLogQuery;
        class GuildBankDepositMoney;
        class GuildBankWithdrawMoney;
        class GuildBankSwapItems;
        class GuildBankSwapItemsBankBank;
        class GuildBankSwapItemsLegacy;
        class GuildBankSwapItemsCount;
        class GuildBankSwapItemsAuto;
        class GuildBankSwapItemsBankBankCount;
        class GuildBankUpdateTab;
        class GuildBankBuyTab;
        class GuildBankTextQuery;
        class GuildBankSetTabText;

        class GuildChallengeUpdateRequest;

        class LFGuildSetGuildPost;
        class LFGuildAddRecruit;
        class LFGuildBrowse;
        class LFGuildRemoveRecruit;
        class LFGuildGetRecruits;
        class LFGuildGetGuildPost;
        class LFGuildGetApplications;
        class LFGuildDeclineRecruit;
    }

    namespace Item
    {
        struct ItemInstance;
        class BuyBackItem;
        class ItemRefundInfo;
        class RepairItem;
        class SellItem;
        class SplitItem;
        class SwapInvItem;
        class SwapItem;
        class AutoEquipItem;
        class AutoBankReagent;
        class DestroyItem;
        class BuyItem;
        class AutoStoreBagItem;
        class VoidStorageContents;
        class UseCritterItem;
        class AutoEquipItemSlot;
        class CancelTempEnchantment;
        class WrapItem;
        class ItemPurchaseRefund;
        class UpgradeItem;
        class ReadItem;
        class SortBags;
        class SortBankBags;
        class SortReagentBankBags;
        class SocketGems;
        class RemoveNewItem;
    }

    namespace Loot
    {
        class LootUnit;
        class AutoStoreLootItem;
        class LootRelease;
        class LootMoney;
        class SetLootSpecialization;
        class LootRoll;
        class DoMasterLootRoll;
        class MasterLootItem;
        class CancelMasterLootRoll;
    }

    namespace Mail
    {
        class MailCreateTextItem;
        class MailDelete;
        class MailGetList;
        class MailMarkAsRead;
        class MailQueryNextMailTime;
        class MailReturnToSender;
        class MailTakeItem;
        class MailTakeMoney;
        class SendMail;
    }

    namespace Misc
    {
        struct PhaseShiftDataPhase;
        class AreaTrigger;
        class SetSelection;
        class ViolenceLevel;
        class TutorialSetFlag;
        class SetDungeonDifficulty;
        class SetRaidDifficulty;
        class PortGraveyard;
        class ReclaimCorpse;
        class RepopRequest;
        class RequestCemeteryList;
        class ResurrectResponse;
        class StandStateChange;
        class UITimeRequest;
        class RandomRollClient;
        class ObjectUpdateFailed;
        class ObjectUpdateRescued;
        class CompleteCinematic;
        class NextCinematicCamera;
        class FarSight;
        class OpeningCinematic;
        class TogglePvP;
        class SetPvP;
        class ConfirmRespecWipe;
        class ShowTradeSkill;
        class CompleteMovie;
        class SaveCUFProfiles;
        class MountSpecialAnim;
        class RequestResearchHistory;
        class ChoiceResponse;
        class CloseInteraction;
        class MountSetFavorite;
        class SetPlayHoverAnim;
        class RequestConsumptionConversionInfo;
        class ContributionCollectorContribute;
        class ContributionGetState;
        class TwitterConnect;
        class TwitterDisconnect;
        class ResetChallengeModeCheat;
    }

    namespace Movement
    {
        enum class NewWorldReason : uint8;
        class ClientPlayerMovement;
        class WorldPortResponse;
        class MoveTeleportAck;
        class MovementSpeedAck;
        class MovementAckMessage;
        class MoveSetCollisionHeightAck;
        class SummonResponse;
        class MoveTimeSkipped;
        class SetActiveMover;
        class MoveSplineDone;
        class MoveRemoveMovementForceAck;
        class MoveApplyMovementForceAck;
        class TimeSyncResponse;
        class DiscardedTimeSyncAcks;
        class TimeSyncResponseDropped;
        class TimeSyncResponseFailed;
        class SuspendTokenResponse;
        class MoveKnockBackAck;
    }

    namespace NPC
    {
        class Hello;
        class GossipSelectOption;
        class SpiritHealerActivate;
        class TrainerBuySpell;
        class RequestStabledPets;
    }

    namespace BattlePay
    {
        class DistributionAssignToTarget;
        class StartPurchase;
        class PurchaseProduct;
        class ConfirmPurchaseResponse;
        class GetProductList;
        class GetPurchaseListQuery;
        class UpdateVasPurchaseStates;
        class BattlePayAckFailedResponse;
        class BattlePayQueryClassTrialResult;
        class BattlePayTrialBoostCharacter;
        class BattlePayPurchaseDetailsResponse;
        class BattlePayPurchaseUnkResponse;
    }

    namespace Query
    {
        class QueryCreature;
        class QueryPlayerName;
        class QueryPageText;
        class QueryNPCText;
        class QueryGameObject;
        class QueryCorpseLocationFromClient;
        class QueryCorpseTransport;
        class QuestPOIQuery;
        class QueryQuestCompletionNPCs;
        class ItemTextQuery;
        class QueryPetName;
        class QueryQuestCompletionNPCs;
        class QueryRealmName;
        class QueryTime;
    }

    namespace Quest
    {
        class QuestGiverStatusQuery;
        class QuestGiverStatusMultipleQuery;
        class QuestGiverHello;
        class QueryQuestInfo;
        class QueryTreasurePicker;
        class QuestGiverChooseReward;
        class QuestGiverCompleteQuest;
        class QuestGiverRequestReward;
        class QuestGiverQueryQuest;
        class QuestPushResult;
        class QuestConfirmAccept;
        class QuestLogRemoveQuest;
        class QuestGiverAcceptQuest;
        class PushQuestToParty;
        class AdventureJournalOpenQuest;
        class AdventureJournalStartQuest;
        class QueryAdventureMapPOI;
        class RequestWorldQuestUpdate;
        class RequestAreaPoiUpdate;
    }

    namespace Transmogrification
    {
        class TransmogrifyItems;
    }

    namespace Social
    {
        class AddFriend;
        class AddIgnore;
        class DelFriend;
        class DelIgnore;
        class SendContactList;
        class SetContactNotes;
        class QuickJoinAutoAcceptRequests;
        class QuickJoinRequestInvite;
        class QuickJoinRespondToInvite;
        class QuickJoinSignalToastDisplayed;
    }

    namespace Spells
    {
        class CancelAura;
        class CastSpell;
        class SetActionButton;
        class PetCastSpell;
        class ItemUse;
        class SpellClick;
        class GetMirrorImageData;
        class SelfRes;
        class OpenItem;
        class CancelCast;
        class RequestCategoryCooldowns;
        class CancelMountAura;
        class CancelGrowthAura;
        class MissileTrajectoryCollision;
        class CancelAutoRepeatSpell;
        class CancelChannelling;
        class UpdateMissileTrajectory;
        class UnlearnSkill;
        class UnlearnSpecialization;
        class CancelModSpeedNoControlAuras;
        class CancelQueuedSpell;
        class UpdateSpellVisual;
    }

    namespace Talent
    {
        class LearnTalent;
        class LearnPvpTalents;
    }

    namespace Trade
    {
        class NullCmsg;
        class AcceptTrade;
        class ClearTradeItem;
        class InitiateTrade;
        class SetTradeGold;
        class SetTradeItem;
        class SetTradeCurrency;
        class TradeStatus;
    }

    namespace Who
    {
        class WhoIsRequest;
        class WhoRequestPkt;
    }

    namespace Battleground
    {
        class ListClient;
        class NullCmsg;
        class Join;
        class JoinArena;
        class AreaSpiritHealerQuery;
        class AreaSpiritHealerQueue;
        class Port;
        class JoinSkirmish;
        class HearthAndResurrect;
        class JoinRatedBattleground;
        class ReportPvPPlayerAFK;
        class RequestPVPRewards;
        class SetCemetryPreferrence;
        class AcceptWargameInvite;
        class StartWargame;
        class RequestPvpBrawlInfo;
        class BattlemasterJoinBrawl;
        class RequstCrowdControlSpell;
    }

    namespace VoidStorage
    {
        class UnlockVoidStorage;
        class VoidStorageTransfer;
        class QueryVoidStorage;
        class SwapVoidItem;
    }

    namespace Collections
    {
        class CollectionItemSetFavorite;
        class MountClearFanfare;
        class BattlePetClearFanfare;
    }

    namespace AuctionHouse
    {
        class AuctionHelloRequest;
        class AuctionListBidderItems;
        class AuctionListItems;
        class AuctionListOwnerItems;
        class AuctionListPendingSales;
        class AuctionPlaceBid;
        class AuctionRemoveItem;
        class AuctionReplicateItems;
        class AuctionSellItem;
    }

    namespace Inspect
    {
        class Inspect;
        class InspectPVPRequest;
        class QueryInspectAchievements;
        class RequestHonorStats;
    }

    namespace Vehicle
    {
        class MoveDismissVehicle;
        class RequestVehiclePrevSeat;
        class RequestVehicleNextSeat;
        class MoveChangeVehicleSeats;
        class RequestVehicleSwitchSeat;
        class RideVehicleInteract;
        class EjectPassenger;
        class RequestVehicleExit;
        class MoveSetVehicleRecIdAck;
    }

    namespace Instance
    {
        class ResetInstances;
        class InstanceLockResponse;
        class QueryWorldCountwodnTimer;
    }

    namespace Toy
    {
        class AccountToysUpdate;
        class AddToy;
        class UseToy;
    }

    namespace Achievement
    {
        class GuildSetFocusedAchievement;
        class GuildGetAchievementMembers;
        class SetAchievementsHidden;
    }

    namespace Scene
    {
        class SceneTriggerEvent;
        class SceneInstance;
    }

    namespace Scenario
    {
        class QueryScenarioPOI;
    }

    namespace Calendar
    {
        class CalendarAddEvent;
        class CalendarCopyEvent;
        class CalendarEventInvite;
        class CalendarEventModeratorStatus;
        class CalendarEventRSVP;
        class CalendarEventSignUp;
        class CalendarEventStatus;
        class CalendarGetCalendar;
        class CalendarGetEvent;
        class CalendarGetNumPending;
        class CalendarGuildFilter;
        class CalendarRemoveEvent;
        class CalendarRemoveInvite;
        class CalendarUpdateEvent;
        class SetSavedInstanceExtend;
        class CalendarComplain;
    }

    namespace ReferAFriend
    {
        class AcceptLevelGrant;
        class GrantLevel;
    }

    namespace Auth
    {
        enum class ConnectToSerial : uint32;
    }

    namespace Bank
    {
        class AutoBankItem;
        class AutoStoreBankItem;
        class BuyBankSlot;
        class AutostoreBankReagent;
        class BuyReagentBank;
        class DepositReagentBank;
    }

    namespace Reputation
    {
        class RequestForcedReactions;
    }

    namespace BlackMarket
    {
        class BlackMarketOpen;
        class BlackMarketRequestItems;
        class BlackMarketBidOnItem;
        class BlackMarketOutbid;
    }

    namespace ChallengeMode
    {
        class Misc;
        class RequestLeaders;
        class StartChallengeMode;
        class ResetChallengeMode;
    }

    namespace Totem
    {
        class TotemDestroyed;
    }

    namespace Party
    {
        class PartyInviteClient;
        class PartyInviteResponse;
        class PartyUninvite;
        class RequestPartyMemberStats;
        class SetPartyLeader;
        class SetRole;
        class LeaveGroup;
        class SetLootMethod;
        class MinimapPingClient;
        class UpdateRaidTarget;
        class ConvertRaid;
        class RequestPartyJoinUpdates;
        class SetAssistantLeader;
        class DoReadyCheck;
        class ReadyCheckResponseClient;
        class RequestRaidInfo;
        class OptOutOfLoot;
        class InitiateRolePoll;
        class SetEveryoneIsAssistant;
        class ChangeSubGroup;
        class SwapSubGroups;
        class ClearRaidMarker;
        class SetPartyAssignment;
    }

    namespace Petition
    {
        class PetitionRenameGuild;
        class OfferPetition;
        class TurnInPetition;
        class DeclinePetition;
        class SignPetition;
        class PetitionShowSignatures;
        class PetitionBuy;
        class PetitionShowList;
        class QueryPetition;
    }

    namespace PetPackets
    {
        class StopAttack;
        class DismissCritter;
        class SetPetSpecialization;
        class Action;
        class NameQuery;
        class PetCancelAura;
        class PetAbandon;
        class PetAction;
        class PetRename;
        class PetSetAction;
        class PetSpellAutocast;
        class RequestPetInfo;
        class SetPetSlot;
    }

    namespace LFG
    {
        class LockInfoRequest;
        class LfgJoin;
        class LfgBootPlayerVote;
        class LfgProposalResponse;
        class LfgTeleport;
        class LfgCompleteRoleCheck;
        class LfgLeave;
        class BonusFactionID;
        class NullCmsg;
        class CompleteReadyCheck;

        struct RideTicket;
    }

    namespace LfgList
    {
        class LfgListApplyToGroup;
        class LfgListCancelApplication;
        class LfgListDeclineApplicant;
        class LfgListInviteApplicant;
        class LfgListUpdateRequest;
        class LfgListGetStatus;
        class LfgListInviteResponse;
        class LfgListJoin;
        class LfgListLeave;
        class LfgListSearch;
        class RequestLfgListBlacklist;
        struct ListRequest;
    }

    namespace BattlePet
    {
        class NullCmsg;
        class Query;
        class BattlePetGuidRead;
        class ModifyName;
        class SetBattleSlot;
        class SetFlags;
        class RequestWild;
        class RequestPVP;
        class ReplaceFrontPet;
        class QueueProposeMatchResult;
        class LeaveQueue;
        class RequestUpdate;
        class PetBattleInput;
    }

    namespace Garrison
    {
        class GetGarrisonInfo;
        class GarrisonPurchaseBuilding;
        class GarrisonCancelConstruction;
        class GarrisonRequestBlueprintAndSpecializationData;
        class GarrisonGetBuildingLandmarks;
        class GarrisonMissionBonusRoll;
        class GarrisonRequestLandingPageShipmentInfo;
        class GarrisonCheckUpgradeable;
        class GarrisonStartMission;
        class GarrisonSwapBuildings;
        class GarrisonCompleteMission;
        class CreateShipment;
        class GarrisonRequestShipmentInfo;
        class GarrisonRequestResearchTalent;
        class GarrisonOpenMissionNpcRequest;
        class UpgradeGarrison;
        class TrophyData;
        class RevertTrophy;
        class GetTrophyList;
        class GarrisonSetFollowerInactive;
        class GarrisonRemoveFollowerFromBuilding;
        class GarrisonRenameFollower;
        class GarrisonAssignFollowerToBuilding;
        class GarrisonSetRecruitmentPreferences;
        class GarrisonRecruitFollower;
        class GarrisonRequestClassSpecCategoryInfo;
        class GarrisonGenerateRecruits;
        class GarrisonRemoveFollower;
        class GarrisonRequestScoutingMap;
    }
    
    namespace Taxi
    {
        class ShowTaxiNodes;
        class TaxiNodeStatusQuery;
        class EnableTaxiNode;
        class TaxiQueryAvailableNodes;
        class ActivateTaxi;
        class TaxiRequestEarlyLanding;
        class SetTaxiBenchmarkMode;
    }

    namespace Token
    {
        class RequestWowTokenMarketPrice;
        class UpdateListedAuctionableTokens;
        class CheckVeteranTokenEligibility;
    }

    namespace Ticket
    {
        class Complaint;
        class SupportTicketSubmitBug;
        class GMTicketGetSystemStatus;
        class GMTicketAcknowledgeSurvey;
        class GMTicketGetCaseStatus;
        class SupportTicketSubmitComplaint;
        class SupportTicketSubmitSuggestion;
    }

    namespace Battlenet
    {
        class Request;
        class RequestRealmListTicket;
    }

    namespace Hotfix
    {
        class HotfixRequest;
        class DBQueryBulk;
    }

    class Null final : public ClientPacket
    {
    public:
        Null(WorldPacket&& packet);

        void Read() override;
    };
}

namespace google
{
    namespace protobuf
    {
        class Message;
    }
}

namespace pb = google::protobuf;

enum AccountDataType
{
    GLOBAL_CONFIG_CACHE             = 0,                    // 0x01 g
    PER_CHARACTER_CONFIG_CACHE      = 1,                    // 0x02 p
    GLOBAL_BINDINGS_CACHE           = 2,                    // 0x04 g
    PER_CHARACTER_BINDINGS_CACHE    = 3,                    // 0x08 p
    GLOBAL_MACROS_CACHE             = 4,                    // 0x10 g
    PER_CHARACTER_MACROS_CACHE      = 5,                    // 0x20 p
    PER_CHARACTER_LAYOUT_CACHE      = 6,                    // 0x40 p
    PER_CHARACTER_CHAT_CACHE        = 7,                    // 0x80 p

    NUM_ACCOUNT_DATA_TYPES
};

#define GLOBAL_CACHE_MASK           0x15
#define PER_CHARACTER_CACHE_MASK    0xEA

enum TutorialAction
{
    TUTORIAL_ACTION_UPDATE  = 0,
    TUTORIAL_ACTION_CLEAR   = 1,
    TUTORIAL_ACTION_RESET   = 2
};

struct AccountData
{
    AccountData() : Time(0), Data("") { }

    time_t Time;
    std::string Data;
};

enum PartyOperation
{
    PARTY_OP_INVITE = 0,
    PARTY_OP_UNINVITE = 1,
    PARTY_OP_LEAVE = 2,
    PARTY_OP_SWAP = 4
};

enum BarberShopResult
{
    BARBER_SHOP_RESULT_SUCCESS      = 0,
    BARBER_SHOP_RESULT_NO_MONEY     = 1,
    BARBER_SHOP_RESULT_NOT_ON_CHAIR = 2,
    BARBER_SHOP_RESULT_NO_MONEY_2   = 3
};

enum PartyResult
{
    ERR_PARTY_RESULT_OK                 = 0,
    ERR_BAD_PLAYER_NAME_S               = 1,
    ERR_TARGET_NOT_IN_GROUP_S           = 2,
    ERR_TARGET_NOT_IN_INSTANCE_S        = 3,
    ERR_GROUP_FULL                      = 4,
    ERR_ALREADY_IN_GROUP_S              = 5,
    ERR_NOT_IN_GROUP                    = 6,
    ERR_NOT_LEADER                      = 7,
    ERR_PLAYER_WRONG_FACTION            = 8,
    ERR_IGNORING_YOU_S                  = 9,
    ERR_LFG_PENDING                     = 12,
    ERR_INVITE_RESTRICTED               = 13,
    ERR_GROUP_SWAP_FAILED               = 14,               // if (PartyOperation == PARTY_OP_SWAP) ERR_GROUP_SWAP_FAILED else ERR_INVITE_IN_COMBAT
    ERR_INVITE_UNKNOWN_REALM            = 15,
    ERR_INVITE_NO_PARTY_SERVER          = 16,
    ERR_INVITE_PARTY_BUSY               = 17,
    ERR_PARTY_TARGET_AMBIGUOUS          = 18,
    ERR_PARTY_LFG_INVITE_RAID_LOCKED    = 19,
    ERR_PARTY_LFG_BOOT_LIMIT            = 20,
    ERR_PARTY_LFG_BOOT_COOLDOWN_S       = 21,
    ERR_PARTY_LFG_BOOT_IN_PROGRESS      = 22,
    ERR_PARTY_LFG_BOOT_TOO_FEW_PLAYERS  = 23,
    ERR_PARTY_LFG_BOOT_NOT_ELIGIBLE_S   = 24,
    ERR_RAID_DISALLOWED_BY_LEVEL        = 25,
    ERR_PARTY_LFG_BOOT_IN_COMBAT        = 26,
    ERR_VOTE_KICK_REASON_NEEDED         = 27,
    ERR_PARTY_LFG_BOOT_DUNGEON_COMPLETE = 28,
    ERR_PARTY_LFG_BOOT_LOOT_ROLLS       = 29,
    ERR_PARTY_LFG_TELEPORT_IN_COMBAT    = 30,
    ERR_PARTY_ALREADY_IN_BATTLEGROUND_QUEUE = 31,
    ERR_PARTY_CONFIRMING_BATTLEGROUND_QUEUE = 32,
    ERR_CROSS_REALM_RAID_INVITE         = 33,
    ERR_RAID_DISALLOWED_BY_CROSS_REALM  = 34,
    ERR_PARTY_ROLE_NOT_AVAILABLE        = 35,
    ERR_PARTY_LFG_BOOT_VOTE_REGISTERED  = 36,
    ERR_PARTY_PRIVATE_GROUP_ONLY        = 37,

};

enum ChatRestrictionType
{
    ERR_CHAT_RESTRICTED = 0,
    ERR_CHAT_THROTTLED  = 1,
    ERR_USER_SQUELCHED  = 2,
    ERR_YELL_RESTRICTED = 3
};

enum DeclinedNameResult
{
    DECLINED_NAMES_RESULT_SUCCESS   = 0,
    DECLINED_NAMES_RESULT_ERROR     = 1
};

struct CharEnumInfoData
{
    ObjectGuid GuildGuid;
    std::string Name;
    uint8 Race = 0;
    uint8 Class = 0;
    uint8 Sex = 0;
    uint8 Skin = 0;
    uint8 Face = 0;
    uint8 HairStyle = 0;
    uint8 HairColor = 0;
    uint8 FacialHair = 0;
    uint8 Level = 0;
    int32 ZoneId = 0;
    int32 MapId = 0;
}; 

typedef std::map<ObjectGuid, CharEnumInfoData> CharEnumMap;

struct CharacterTemplateData
{
    CharacterTemplate const* charTemplate = nullptr;
    uint32 id = 0;
    uint32 iLevel = 0;
    uint32 money = 0;
    uint32 transferId = 0;
    uint32 templateId = 0;
    uint8 level = 0;
    bool artifact = true;
    bool active = true;
}; 

typedef std::map<uint32, CharacterTemplateData> CharacterTemplateDataMap;

/// Player session in the World
class WorldSession
{
    public:
        WorldSession(uint32 id, std::string&& name, const std::shared_ptr<WorldSocket>& sock, AccountTypes sec, uint8 expansion, time_t mute_time, std::string os, LocaleConstant locale, uint32 recruiter, bool isARecruiter, AuthFlags flag, int64 balance);
        ~WorldSession();

        bool PlayerLoading() const { return !m_playerLoading.IsEmpty(); }
        bool PlayerLogout() const { return m_playerLogout; }
        bool PlayerLogoutWithSave() const { return m_playerLogout && m_playerSave; }
        bool PlayerRecentlyLoggedOut() const { return m_playerRecentlyLogout; }

        bool IsAddonRegistered(std::string const& prefix);

        void SendPacket(WorldPacket const* packet, bool forced = false);
        void AddInstanceConnection(std::shared_ptr<WorldSocket> sock) { m_Socket[CONNECTION_TYPE_INSTANCE] = sock; }
        void SendNotification(const char *format, ...) ATTR_PRINTF(2, 3);
        void SendNotification(uint32 string_id, ...);
        void SendPetNameInvalid(uint32 error, ObjectGuid const& guid, std::string const& name, DeclinedName *declinedName = nullptr);
        void SendPartyResult(PartyOperation operation, std::string const& member, PartyResult res, uint32 val = 0);
        void SendSetPhaseShift(std::vector<WorldPackets::Misc::PhaseShiftDataPhase> phases, std::vector<uint16> const& visibleMapIDs, std::vector<uint16> const& uiWorldMapAreaIDSwaps, std::vector<uint16> const& preloadMapIDs, uint32 phaseShiftFlags = 0x1F);
        void SendQueryTimeResponse();

        void SendAuthResponse(uint8 code, bool queued = false, uint32 queuePos = 0);
        void SendClientCacheVersion(uint32 version);
        void InitializeSession();
        void InitializeSessionCallback(SQLQueryHolder* realmHolder, SQLQueryHolder* holder);

        void HandleGetPurchaseListQuery(WorldPackets::BattlePay::GetPurchaseListQuery& packet);
        void HandleBattlePayQueryClassTrialResult(WorldPackets::BattlePay::BattlePayQueryClassTrialResult& packet);
        void HandleBattlePayTrialBoostCharacter(WorldPackets::BattlePay::BattlePayTrialBoostCharacter& packet);
        void HandleBattlePayPurchaseUnkResponse(WorldPackets::BattlePay::BattlePayPurchaseUnkResponse& packet);
        void HandleBattlePayPurchaseDetailsResponse(WorldPackets::BattlePay::BattlePayPurchaseDetailsResponse& packet);
        void HandleUpdateVasPurchaseStates(WorldPackets::BattlePay::UpdateVasPurchaseStates& packet);
        void HandleGetProductList(WorldPackets::BattlePay::GetProductList& packet);
        void SendDisplayPromo(int32 promotionID = 0);

        void SendFeatureSystemStatusGlueScreen();

        AccountTypes GetSecurity() const { return _security; }
        uint32 GetAccountId() const { return _accountId; }
        ObjectGuid GetAccountGUID() const;
        std::string const& GetAccountName() const { return _accountName; }
        uint8 GetAccountExpansion() const { return m_accountExpansion; }
        ObjectGuid GetBattlenetAccountGUID() const;
        Player* GetPlayer() const { return _player; }
        std::string GetPlayerName(bool simple = true) const;

        Map* GetMap() const { return m_map; }
        void SetMap(Map* m) { m_map = m; }

        ObjectGuid::LowType GetGuidLow() const;
        void SetSecurity(AccountTypes security) { _security = security; }
        std::string const& GetRemoteAddress() { return m_Address; }
        void SetPlayer(Player* player);
        uint8 Expansion() const { return m_expansion; }
        std::string const& GetOS() const { return _os; }

        bool InitializeWarden(BigNumber* k, std::string const& os);
        // temp
        uint32 GetCountWardenPacketsInQueue();

        /// Session in auth.queue currently
        void SetInQueue(bool state) { m_inQueue = state; }

        /// Is the user engaged in a log out process?
        bool isLogingOut() const { return _logoutTime || m_playerLogout; }

        /// Engage the logout process for the user
        void LogoutRequest(time_t requestTime)
        {
            _logoutTime = requestTime;
        }

        /// Is logout cooldown expired?
        bool ShouldLogOut(time_t currTime) const
        {
            return (_logoutTime > 0 && currTime >= _logoutTime + 20);
        }

        void LogoutPlayer(bool Save);
        void KickPlayer();
        bool CanLogout() { return canLogout; }
        void SetCanLogout() { canLogout = true; }

        void QueuePacket(WorldPacket* new_packet);
        bool Update(uint32 diff, Map* map = nullptr);

        /// Handle the authentication waiting queue (to be completed)
        void SendAuthWaitQue(uint32 position);

        //void SendTestCreatureQueryOpcode(uint32 entry, ObjectGuid guid, uint32 testvalue);
        void SendNameQueryOpcode(ObjectGuid guid);

        void SendTrainerList(ObjectGuid const& guid);
        void SendTrainerList(ObjectGuid const& guid, std::string const& strTitle);
        void SendListInventory(ObjectGuid const& guid, uint32 entry = 0);
        void SendShowBank(ObjectGuid const& guid);
        bool CanOpenMailBox(ObjectGuid guid);
        void SendShowMailBox(ObjectGuid guid);
        void SendTabardVendorActivate(ObjectGuid const& guid);
        void SendSpiritResurrect();
        void SendBindPoint(Creature* npc);
        void SendOpenTransmogrifier(ObjectGuid const& guid);
        void SendOpenAlliedRaceDetails(ObjectGuid const& guid, uint32 RaceID);
        bool CanOpenGuildBank(ObjectGuid guid);

        void SendTradeStatus(WorldPackets::Trade::TradeStatus& packet);
        void SendUpdateTrade(bool trader_data = true);
        void SendCancelTrade();

        void SendPetitionQueryOpcode(ObjectGuid petitionguid);

        void SendStablePet(ObjectGuid const& guid = ObjectGuid::Empty);
        void SendStableResult(StableResultCode res);

        // Account Data
        AccountData* GetAccountData(AccountDataType type);
        void SetAccountData(AccountDataType type, time_t tm = time_t(0), std::string const& data = "");
        void SendSetTimeZoneInformation();
        void LoadAccountData(PreparedQueryResult const& result, uint32 mask);
        void LoadCharacterTemplates(PreparedQueryResult const& result);

        void LoadTutorialsData(PreparedQueryResult const& result);
        void SendTutorialsData();
        void SaveTutorialsData(SQLTransaction& trans);
        uint32 GetTutorialInt(uint8 index) const;
        void SetTutorialInt(uint8 index, uint32 value);
        //auction
        void SendAuctionHello(ObjectGuid guid, Creature* unit);
        void SendAuctionCommandResult(AuctionEntry* auction, uint32 Action, uint32 ErrorCode, uint32 bidError = 0);
        void SendAuctionOutBidNotification(AuctionEntry const* auction, Item const* item);
        void SendAuctionClosedNotification(AuctionEntry const* auction, float mailDelay, bool sold, Item const* item);
        void SendAuctionWonNotification(AuctionEntry const* auction, Item const* item);
        void SendAuctionOwnerBidNotification(AuctionEntry const* auction, Item const* item);

        void HandleArtifactAddPower(WorldPackets::Artifact::AddPower& packet);
        void HandleArtifactAddRelicTalent(WorldPackets::Artifact::ArtifactAddRelicTalent& packet);
        void HandleArtifactAttuneSocketedRelic(WorldPackets::Artifact::ArtifactAttuneSocketedRelic& packet);
        void HandleArtifactAttunePreviewRelic(WorldPackets::Artifact::ArtifactAttunePreviewdRelic& packet);
        void HandleArtifactConfirmRespec(WorldPackets::Artifact::ConfirmRespec& packet);
        void HandleArtifactSetAppearance(WorldPackets::Artifact::SetAppearance& packet);
        void HandleBlackMarketOpen(WorldPackets::BlackMarket::BlackMarketOpen& packet);
        void SendBlackMarketBidOnItemResult(int32 result, int32 marketId, WorldPackets::Item::ItemInstance& item);
        void SendBlackMarketWonNotification(BlackMarketEntry const* entry, Item const* item);
        void SendBlackMarketOutbidNotification(BlackMarketTemplate const* templ);
        void HandleBlackMarketRequestItems(WorldPackets::BlackMarket::BlackMarketRequestItems& packet);
        void HandleBlackMarketBidOnItem(WorldPackets::BlackMarket::BlackMarketBidOnItem& packet);
        
        //Taxi
        void SendTaxiStatus(ObjectGuid guid);
        void SendTaxiMenu(Creature* unit);
        void SendDoFlight(uint32 mountDisplayId, uint32 path, uint32 pathNode = 0);
        bool SendLearnNewTaxiNode(Creature* unit);
        void SendDiscoverNewTaxiNode(uint32 nodeid);

        // Guild
        void SendPetitionShowList(ObjectGuid guid);

        void DoLootRelease(ObjectGuid lguid);

        // Account mute time
        time_t m_muteTime;

        // Locales
        LocaleConstant GetSessionDbcLocale() const { return m_sessionDbLocaleIndex; }
        LocaleConstant GetSessionDbLocaleIndex() const { return m_sessionDbLocaleIndex == LOCALE_none ? LOCALE_enUS : m_sessionDbLocaleIndex; }
        const char *GetTrinityString(int32 entry) const;

        uint32 GetLatency() const { return m_latency; }
        void SetLatency(uint32 latency) { m_latency = latency; }
        void ResetClientTimeDelay() { m_clientTimeDelay = 0; }

        Warden* GetWarden() { return _warden; }

        uint32 getDialogStatus(Player* player, Object* questgiver, uint32 defstatus);

        std::atomic<int32> m_timeOutTime;

        void UpdateTimeOutTime(uint32 diff)
        {
            m_timeOutTime -= int32(diff);
        }

        void ResetTimeOutTime()
        {
            m_timeOutTime = int32(sWorld->getIntConfig(CONFIG_SOCKET_TIMEOUTTIME));
        }

        bool IsConnectionIdle() const
        {
            return m_timeOutTime <= 0 && !m_inQueue;
        }

        // Recruit-A-Friend Handling
        uint32 GetRecruiterId() const { return recruiterId; }
        bool IsARecruiter() const { return isRecruiter; }

        void ProcessAnticheatAction(const char* detector, const char* reason, uint32 action, uint32 banTime = 0 /* Perm ban */);

        void LookupPlayerSearchCommand(PreparedQueryResult result, int32 limit);
        void BanListHelper(PreparedQueryResult result);

        void Handle_NULL(WorldPackets::Null& null);
        void Handle_EarlyProccess(WorldPacket& recvPacket); // just mark packets processed in WorldSocket::OnRead
        void LogUnprocessedTail(WorldPacket const* packet);

        void HandleCharEnumOpcode(WorldPackets::Character::EnumCharacters& /*enumCharacters*/);
        void HandleCharDeleteOpcode(WorldPackets::Character::DeleteChar& charDelete);
        void HandleCharCreateOpcode(WorldPackets::Character::CreateChar& charCreate);
        void HandlePlayerLoginOpcode(WorldPackets::Character::PlayerLogin& playerLogin);
        void HandleLoadScreenOpcode(WorldPackets::Character::LoadingScreenNotify& loadingScreenNotify);
        void HandleCharEnum(PreparedQueryResult result, bool isDelete);
        void HandlePlayerLogin(LoginQueryHolder * holder);
        void HandleCharRaceOrFactionChange(WorldPackets::Character::CharRaceOrFactionChange& packet);
        void HandleGenerateRandomCharacterName(WorldPackets::Character::GenerateRandomCharacterName& packet);
        void HandleReorderCharacters(WorldPackets::Character::ReorderCharacters& packet);
        void HandleRequestPlayedTime(WorldPackets::Character::RequestPlayedTime& packet);
        void HandleEngineSurvey(WorldPackets::Character::EngineSurvey& packet);

        // cemetery/graveyard related
        void HandlePortGraveyard(WorldPackets::Misc::PortGraveyard& packet);
        void HandleRequestCemeteryList(WorldPackets::Misc::RequestCemeteryList& packet);
        void HandleChoiceResponse(WorldPackets::Misc::ChoiceResponse& packet);
        void HandleCloseInteraction(WorldPackets::Misc::CloseInteraction& packet);

        void HandleInspect(WorldPackets::Inspect::Inspect& packet);

        void HandleRequestHonorStats(WorldPackets::Inspect::RequestHonorStats& packet);

        void HandleInspectPVP(WorldPackets::Inspect::InspectPVPRequest& packet);

        void HandleMountSpecialAnim(WorldPackets::Misc::MountSpecialAnim& packet);

        void HandleRepairItem(WorldPackets::Item::RepairItem& packet);
        void HandleRemoveNewItem(WorldPackets::Item::RemoveNewItem& packet);

        void HandleMoveTeleportAck(WorldPackets::Movement::MoveTeleportAck& packet);
        void HandleForceSpeedChangeAck(WorldPackets::Movement::MovementSpeedAck& packet);
        void HandleMoveKnockBackAck(WorldPackets::Movement::MoveKnockBackAck& packet);
        void HandleMovementAckMessage(WorldPackets::Movement::MovementAckMessage& packet);
        void HandleSetCollisionHeightAck(WorldPackets::Movement::MoveSetCollisionHeightAck& packet);
        void HandleMoveRemoveMovementForceAck(WorldPackets::Movement::MoveRemoveMovementForceAck& packet);
        void HandleMoveApplyMovementForceAck(WorldPackets::Movement::MoveApplyMovementForceAck& packet);
        void HandleDiscardedTimeSyncAcks(WorldPackets::Movement::DiscardedTimeSyncAcks& packet);
        void HandleTimeSyncResponseDropped(WorldPackets::Movement::TimeSyncResponseDropped& packet);
        void HandleTimeSyncResponseFailed(WorldPackets::Movement::TimeSyncResponseFailed& packet);

        void HandleRepopRequest(WorldPackets::Misc::RepopRequest& packet);
        void HandleAutostoreLootItemOpcode(WorldPackets::Loot::AutoStoreLootItem& packet);
        void HandleLootMoney(WorldPackets::Loot::LootMoney& packet);
        void HandleSetLootSpecialization(WorldPackets::Loot::SetLootSpecialization& packet);
        void HandleLootUnit(WorldPackets::Loot::LootUnit& packet);
        void HandleLootRelease(WorldPackets::Loot::LootRelease& packet);
        void HandleMasterLootItem(WorldPackets::Loot::MasterLootItem& packet);
        void HandleWhoOpcode(WorldPackets::Who::WhoRequestPkt& whoRequest);
        void HandleLogoutRequest(WorldPackets::Character::LogoutRequest& packet);
        void HandleLogoutInstant(WorldPackets::Character::LogoutInstant& packet);
        void HandleLogoutCancel(WorldPackets::Character::LogoutCancel& packet);

        void HandleGMTicketGetSystemStatus(WorldPackets::Ticket::GMTicketGetSystemStatus& packet);

        void HandleTogglePvP(WorldPackets::Misc::TogglePvP& packet);
        void HandleSetPvP(WorldPackets::Misc::SetPvP& packet);

        void HandleSetSelectionOpcode(WorldPackets::Misc::SetSelection& packet);
        void HandleStandStateChangeOpcode(WorldPackets::Misc::StandStateChange& packet);
        void HandleEmote(WorldPackets::Character::EmoteClient& packet);
        void HandleContactListOpcode(WorldPackets::Social::SendContactList& packet);
        void HandleAddFriend(WorldPackets::Social::AddFriend& packet);
        void HandleDelFriendOpcode(WorldPackets::Social::DelFriend& packet);
        void HandleAddIgnore(WorldPackets::Social::AddIgnore& packet);
        void HandleDelIgnoreOpcode(WorldPackets::Social::DelIgnore& packet);
        void HandleSetContactNotesOpcode(WorldPackets::Social::SetContactNotes& packet);
        void HandleSupportTicketSubmitBug(WorldPackets::Ticket::SupportTicketSubmitBug& packet);
        void HandleGMTicketAcknowledgeSurvey(WorldPackets::Ticket::GMTicketAcknowledgeSurvey& packet);
        void HandleGMTicketGetCaseStatus(WorldPackets::Ticket::GMTicketGetCaseStatus& packet);
        void HandleSupportTicketSubmitComplaint(WorldPackets::Ticket::SupportTicketSubmitComplaint& packet);
        void HandleSupportTicketSubmitSuggestion(WorldPackets::Ticket::SupportTicketSubmitSuggestion& packet);

        void HandleQuickJoinAutoAcceptRequests(WorldPackets::Social::QuickJoinAutoAcceptRequests& packet);
        void HandleQuickJoinRequestInvite(WorldPackets::Social::QuickJoinRequestInvite& packet);
        void HandleQuickJoinRespondToInvite(WorldPackets::Social::QuickJoinRespondToInvite& packet);
        void HandleQuickJoinSignalToastDisplayed(WorldPackets::Social::QuickJoinSignalToastDisplayed& packet);

        void HandleCanDuel(WorldPackets::Duel::CanDuel& packet);
        void HandleDuelResponse(WorldPackets::Duel::DuelResponse& duelResponse);

        void HandleAreaTrigger(WorldPackets::Misc::AreaTrigger& packet);

        void HandleSetFactionAtWar(WorldPackets::Character::SetFactionAtWar& packet);
        void HandleUnsetFactionAtWar(WorldPackets::Character::SetFactionNotAtWar& packet);
        void HandleBonusFactionID(WorldPackets::LFG::BonusFactionID& packet);

        void HandleRequestLfgListBlackList(WorldPackets::LfgList::RequestLfgListBlacklist& packet);
        void HandleLfgListSearch(WorldPackets::LfgList::LfgListSearch& packet);
        void HandleLfgListJoin(WorldPackets::LfgList::LfgListJoin& packet);
        void HandleLfgListLeave(WorldPackets::LfgList::LfgListLeave& packet);
        void HandleLfgListInviteResponse(WorldPackets::LfgList::LfgListInviteResponse& packet);
        void HandleLfgListGetStatus(WorldPackets::LfgList::LfgListGetStatus& packet);
        void HandleLfgListApplyToGroup(WorldPackets::LfgList::LfgListApplyToGroup& packet);
        void HandleLfgListCancelApplication(WorldPackets::LfgList::LfgListCancelApplication& packet);
        void HandleLfgListDeclineApplicant(WorldPackets::LfgList::LfgListDeclineApplicant& packet);
        void HandleLfgListInviteApplicant(WorldPackets::LfgList::LfgListInviteApplicant& packet);
        void HandleLfgListUpdateRequest(WorldPackets::LfgList::LfgListUpdateRequest& packet);

        void HandleSetWatchedFaction(WorldPackets::Character::SetWatchedFaction& packet);
        void HandleSetFactionInactive(WorldPackets::Character::SetFactionInactive& packet);

        void HandleUpdateAccountData(WorldPackets::ClientConfig::UserClientUpdateAccountData& packet);
        void HandleRequestAccountData(WorldPackets::ClientConfig::RequestAccountData& request);
        void HandleSetAdvancedCombatLogging(WorldPackets::ClientConfig::SetAdvancedCombatLogging& setAdvancedCombatLogging);
        void HandleUpdateClientSettings(WorldPackets::ClientConfig::UpdateClientSettings& packet);
        void HandleGetRemainingGameTime(WorldPackets::ClientConfig::GetRemainingGameTime& packet);
        void HandleSaveClientVariables(WorldPackets::ClientConfig::SaveClientVariables& packet);

        void HandleSetActionButtonOpcode(WorldPackets::Spells::SetActionButton& packet);
        
        void SendConnectToInstance(WorldPackets::Auth::ConnectToSerial serial);
        void HandleContinuePlayerLogin();
        void AbortLogin(WorldPackets::Character::LoginFailureReason reason);

        void HandleGameObjectUse(WorldPackets::GameObject::GameObjectUse& packet);
        void HandleGameobjectReportUse(WorldPackets::GameObject::GameObjReportUse& packet);

        void HandleQueryPlayerName(WorldPackets::Query::QueryPlayerName& packet);

        void HandleQueryTime(WorldPackets::Query::QueryTime& packet);

        void HandleCreatureQuery(WorldPackets::Query::QueryCreature& packet);

        void HandleQueryGameObject(WorldPackets::Query::QueryGameObject& packet);

        void HandleWorldPortResponse(WorldPackets::Movement::WorldPortResponse& packet);
        void HandleWorldPortAck();                // for server-side calls

        void HandleMovementOpcodes(WorldPackets::Movement::ClientPlayerMovement& packet);
        void HandleMovementOpcode(OpcodeClient opcode, MovementInfo& movementInfo);
        void HandleSetActiveMover(WorldPackets::Movement::SetActiveMover& packet);
        void HandleMoveDismissVehicle(WorldPackets::Vehicle::MoveDismissVehicle& packet);
        void HandleRequestVehiclePrevSeat(WorldPackets::Vehicle::RequestVehiclePrevSeat& packet);
        void HandleRequestVehicleNextSeat(WorldPackets::Vehicle::RequestVehicleNextSeat& packet);
        void HandleMoveChangeVehicleSeats(WorldPackets::Vehicle::MoveChangeVehicleSeats& packet);
        void HandleRequestVehicleSwitchSeat(WorldPackets::Vehicle::RequestVehicleSwitchSeat& packet);

        void HandleRequestVehicleExit(WorldPackets::Vehicle::RequestVehicleExit& packet);
        void HandleMoveTimeSkipped(WorldPackets::Movement::MoveTimeSkipped& packet);

        void HandleRequestRaidInfo(WorldPackets::Party::RequestRaidInfo& packet);

        void HandleBattlefieldStatus(WorldPackets::Battleground::NullCmsg& packet);

        void HandlePartyInvite(WorldPackets::Party::PartyInviteClient& packet);
        void HandlePartyInviteResponse(WorldPackets::Party::PartyInviteResponse& packet);
        void HandlePartyUninvite(WorldPackets::Party::PartyUninvite& packet);
        void HandleSetPartyLeader(WorldPackets::Party::SetPartyLeader& packet);
        void HandleSetRole(WorldPackets::Party::SetRole& packet);
        void HandleLeaveGroup(WorldPackets::Party::LeaveGroup& packet);
        void HandleOptOutOfLoot(WorldPackets::Party::OptOutOfLoot& packet);
        void HandleSetLootMethod(WorldPackets::Party::SetLootMethod& packet);
        void HandleLootRoll(WorldPackets::Loot::LootRoll& packet);
        void HandleDoMasterLootRoll(WorldPackets::Loot::DoMasterLootRoll& packet);
        void HandleCancelMasterLootRoll(WorldPackets::Loot::CancelMasterLootRoll& packet);
        void HandleRequestPartyJoinUpdates(WorldPackets::Party::RequestPartyJoinUpdates& packet);
        void HandleRequestPartyMemberStats(WorldPackets::Party::RequestPartyMemberStats& packet);
        void HandleReadyCheckResponse(WorldPackets::Party::ReadyCheckResponseClient& packet);
        void HandleDoReadyCheck(WorldPackets::Party::DoReadyCheck& packet);
        void HandleUpdateRaidTarget(WorldPackets::Party::UpdateRaidTarget& packet);
        void HandleConvertRaid(WorldPackets::Party::ConvertRaid& packet);
        void HandleChangeSubGroup(WorldPackets::Party::ChangeSubGroup& packet);
        void HandleSwapSubGroups(WorldPackets::Party::SwapSubGroups& packet);
        void HandleSetAssistantLeader(WorldPackets::Party::SetAssistantLeader& packet);
        void HandleSetEveryoneIsAssistant(WorldPackets::Party::SetEveryoneIsAssistant& packet);
        void HandleSetPartyAssignment(WorldPackets::Party::SetPartyAssignment& packet);
        void HandleInitiateRolePoll(WorldPackets::Party::InitiateRolePoll& packet);

        void HandlePetitionBuy(WorldPackets::Petition::PetitionBuy& packet);
        void HandlePetitionShowSignatures(WorldPackets::Petition::PetitionShowSignatures& packet);
        void HandleQueryPetition(WorldPackets::Petition::QueryPetition& packet);
        void HandlePetitionRenameGuild(WorldPackets::Petition::PetitionRenameGuild& packet);
        void HandleSignPetition(WorldPackets::Petition::SignPetition& packet);
        void SendPetitionSignResult(ObjectGuid const& playerGuid, ObjectGuid const& petitionGuid, uint8 result);
        void HandleDeclinePetition(WorldPackets::Petition::DeclinePetition& packet);
        void HandleOfferPetition(WorldPackets::Petition::OfferPetition& packet);
        void HandleTurnInPetition(WorldPackets::Petition::TurnInPetition& packet);

        void HandleGuildQueryOpcode(WorldPackets::Guild::QueryGuildInfo& packet);
        void HandleGuildInviteByName(WorldPackets::Guild::GuildInviteByName& packet);
        void HandleGuildOfficerRemoveMember(WorldPackets::Guild::GuildOfficerRemoveMember& packet);
        void HandleGuildAcceptInvite(WorldPackets::Guild::AcceptGuildInvite& /*packet*/);
        void HandleGuildDeclineInvitation(WorldPackets::Guild::GuildDeclineInvitation& /*decline*/);
        void HandleGuildChangeNameRequest(WorldPackets::Guild::GuildChangeNameRequest& packet);
        void HandleGuildEventLogQuery(WorldPackets::Guild::GuildEventLogQuery& /*packet*/);
        void HandleGuildRosterOpcode(WorldPackets::Guild::GuildGetRoster& packet);
        void HandleGuildRewardsQueryOpcode(WorldPackets::Guild::RequestGuildRewardsList& packet);
        void HandleGuildPromoteMember(WorldPackets::Guild::GuildPromoteMember& packet);
        void HandleGuildDemoteMember(WorldPackets::Guild::GuildDemoteMember& packet);
        void HandleGuildAssignRank(WorldPackets::Guild::GuildAssignMemberRank& packet);
        void HandleGuildLeave(WorldPackets::Guild::GuildLeave& /*packet*/);
        void HandleGuildDisbandOpcode(WorldPackets::Guild::GuildDelete& packet);
        void HandleGuildSetGuildMaster(WorldPackets::Guild::GuildSetGuildMaster& packet);
        void HandleGuildUpdateMotdText(WorldPackets::Guild::GuildUpdateMotdText& packet);
        void HandleGuildNewsUpdateStickyOpcode(WorldPackets::Guild::GuildNewsUpdateSticky& packet);
        void HandleGuildSetMemberNote(WorldPackets::Guild::GuildSetMemberNote& packet);
        void HandleGuildGetRanks(WorldPackets::Guild::GuildGetRanks& packet);
        void HandleGuildQueryNews(WorldPackets::Guild::GuildQueryNews& packet);
        void HandleReplaceGuildMaster(WorldPackets::Guild::ReplaceGuildMaster& packet);
        void HandleGuildSetAchievementTracking(WorldPackets::Guild::GuildSetAchievementTracking& packet);
        void HandleGuildAutoDeclineInvitation(WorldPackets::Guild::GuildAutoDeclineInvitation& packet);
        void HandleShiftRank(WorldPackets::Guild::GuildShiftRank& packet);
        void HandleGuildSetRankPermissions(WorldPackets::Guild::GuildSetRankPermissions& packet);
        void HandleGuildAddRank(WorldPackets::Guild::GuildAddRank& packet);
        void HandleGuildDeleteRank(WorldPackets::Guild::GuildDeleteRank& packet);
        void HandleGuildUpdateInfoText(WorldPackets::Guild::GuildUpdateInfoText& packet);
        void HandleSaveGuildEmblem(WorldPackets::Guild::SaveGuildEmblem& packet);
        void HandleGuildRequestPartyState(WorldPackets::Guild::RequestGuildPartyState& packet);
        void HandleGuildSetFocusedAchievement(WorldPackets::Achievement::GuildSetFocusedAchievement& packet);
        void HandleAutoDeclineGuildInvites(WorldPackets::Guild::AutoDeclineGuildInvites& packet);
        void HandleQueryRecipes(WorldPackets::Guild::QueryRecipes& packet);
        void HandleQueryGuildMembersForRecipe(WorldPackets::Guild::QueryGuildMembersForRecipe& packet);
        void HandleQyeryMemberRecipes(WorldPackets::Guild::QueryMemberRecipes& packet);

        void HandleGuildRequestChallengeUpdate(WorldPackets::Guild::GuildChallengeUpdateRequest& /*packet*/);

        void HandleLFGuildAddRecruit(WorldPackets::Guild::LFGuildAddRecruit& packet);
        void HandleLFGuildBrowse(WorldPackets::Guild::LFGuildBrowse& packet);
        void HandleLFGuildDeclineRecruit(WorldPackets::Guild::LFGuildDeclineRecruit& packet);
        void HandleLFGuildGetApplications(WorldPackets::Guild::LFGuildGetApplications& packet);
        void HandleLFGuildGetRecruits(WorldPackets::Guild::LFGuildGetRecruits& packet);
        void HandleLFGuildGetGuildPost(WorldPackets::Guild::LFGuildGetGuildPost& packet);
        void HandleLFGuildRemoveRecruit(WorldPackets::Guild::LFGuildRemoveRecruit& packet);
        void HandleLFGuildSetGuildPost(WorldPackets::Guild::LFGuildSetGuildPost& packet);

        void HandleEnableTaxiNode(WorldPackets::Taxi::EnableTaxiNode& packet);
        void HandleTaxiNodeStatusQuery(WorldPackets::Taxi::TaxiNodeStatusQuery& packet);
        void HandleTaxiQueryAvailableNodes(WorldPackets::Taxi::TaxiQueryAvailableNodes& packet);
        void HandleActivateTaxi(WorldPackets::Taxi::ActivateTaxi& packet);
        void HandleMoveSplineDone(WorldPackets::Movement::MoveSplineDone& packet);
        void SendActivateTaxiReply(ActivateTaxiReply reply);
        void HandleTaxiRequestEarlyLanding(WorldPackets::Taxi::TaxiRequestEarlyLanding& packet);

        void HandleTabardVendorActivate(WorldPackets::NPC::Hello& packet);
        void HandleBankerActivate(WorldPackets::NPC::Hello& packet);
        void HandleBuyBankSlot(WorldPackets::Bank::BuyBankSlot& packet);
        void HandleTrainerList(WorldPackets::NPC::Hello& packet);
        void HandleTrainerBuySpell(WorldPackets::NPC::TrainerBuySpell& packet);
        void HandlePetitionShowList(WorldPackets::Petition::PetitionShowList& packet);
        void HandleGossipHelloOpcode(WorldPackets::NPC::Hello& packet);
        void HandleGossipSelectOption(WorldPackets::NPC::GossipSelectOption& packet);
        void HandleSpiritHealerActivate(WorldPackets::NPC::SpiritHealerActivate& packet);
        void HandleQueryNPCText(WorldPackets::Query::QueryNPCText& packet);
        void HandleBinderActivate(WorldPackets::NPC::Hello& packet);
        void HandleRequestStabledPets(WorldPackets::NPC::RequestStabledPets& packet);
        void HanleSetPetSlot(WorldPackets::PetPackets::SetPetSlot& packet);
        void HandleStableChangeSlotCallback(PreparedQueryResult const& result, uint8 new_slot);
        void SendTrainerService(ObjectGuid guid, uint32 spellId, uint32 trainState);

        void HandleAcceptTrade(WorldPackets::Trade::AcceptTrade& packet);
        void HandleBeginTrade(WorldPackets::Trade::NullCmsg& packet);
        void HandleBusyTrade(WorldPackets::Trade::NullCmsg& packet);
        void HandleCancelTrade(WorldPackets::Trade::NullCmsg& packet);
        void HandleClearTradeItem(WorldPackets::Trade::ClearTradeItem& packet);
        void HandleIgnoreTrade(WorldPackets::Trade::NullCmsg& packet);
        void HandleInitiateTrade(WorldPackets::Trade::InitiateTrade& packet);
        void HandleSetTradeGold(WorldPackets::Trade::SetTradeGold& packet);
        void HandleSetTradeCurrency(WorldPackets::Trade::SetTradeCurrency& packet);
        void HandleSetTradeItem(WorldPackets::Trade::SetTradeItem& packet);
        void HandleUnacceptTrade(WorldPackets::Trade::NullCmsg& packet);

        void HandleAuctionHelloRequest(WorldPackets::AuctionHouse::AuctionHelloRequest& packet);
        void HandleAuctionListItems(WorldPackets::AuctionHouse::AuctionListItems& packet);
        void HandleAuctionListBidderItems(WorldPackets::AuctionHouse::AuctionListBidderItems& packet);
        void HandleAuctionSellItem(WorldPackets::AuctionHouse::AuctionSellItem& packet);
        void HandleAuctionRemoveItem(WorldPackets::AuctionHouse::AuctionRemoveItem& packet);
        void HandleAuctionListOwnerItems(WorldPackets::AuctionHouse::AuctionListOwnerItems& packet);
        void HandleAuctionPlaceBid(WorldPackets::AuctionHouse::AuctionPlaceBid& packet);
        void HandleAuctionListPendingSales(WorldPackets::AuctionHouse::AuctionListPendingSales& packet);
        void HandleReplicateItems(WorldPackets::AuctionHouse::AuctionReplicateItems& packet);

        void HandleGetMailList(WorldPackets::Mail::MailGetList& packet);
        void HandleSendMail(WorldPackets::Mail::SendMail& packet);
        void HandleMailTakeMoney(WorldPackets::Mail::MailTakeMoney& packet);
        void HandleMailTakeItem(WorldPackets::Mail::MailTakeItem& packet);
        void HandleMailMarkAsRead(WorldPackets::Mail::MailMarkAsRead& packet);
        void HandleMailReturnToSender(WorldPackets::Mail::MailReturnToSender& packet);
        void HandleMailDelete(WorldPackets::Mail::MailDelete& packet);
        void HandleMailCreateTextItem(WorldPackets::Mail::MailCreateTextItem& packet);
        void HandleQueryNextMailTime(WorldPackets::Mail::MailQueryNextMailTime& packet);
        void HandleCancelChanneling(WorldPackets::Spells::CancelChannelling& packet);

        void HandleSplitItemOpcode(WorldPackets::Item::SplitItem& splitItem);
        void HandleSwapInvItemOpcode(WorldPackets::Item::SwapInvItem& swapInvItem);
        void HandleDestroyItemOpcode(WorldPackets::Item::DestroyItem& destroyItem);
        void HandleAutoEquipItem(WorldPackets::Item::AutoEquipItem& autoEquipItem);
        void HandleAutoBankReagent(WorldPackets::Item::AutoBankReagent& autoBankReagent);
        void HandleSellItemOpcode(WorldPackets::Item::SellItem& packet);
        void HandleBuyItemOpcode(WorldPackets::Item::BuyItem& packet);
        void HandleListInventory(WorldPackets::NPC::Hello& packet);
        void HandleAutoStoreBagItem(WorldPackets::Item::AutoStoreBagItem& packet);
        void HandleReadItem(WorldPackets::Item::ReadItem& packet);
        void HandleAutoEquipItemSlotOpcode(WorldPackets::Item::AutoEquipItemSlot& autoEquipItemSlot);
        void HandleSwapItem(WorldPackets::Item::SwapItem& swapItem);
        void HandleBuybackItem(WorldPackets::Item::BuyBackItem& packet);
        void HandleAutoBankItem(WorldPackets::Bank::AutoBankItem& packet);
        void HandleAutoStoreBankItem(WorldPackets::Bank::AutoStoreBankItem& packet);
        void HandleAutostoreBankReagent(WorldPackets::Bank::AutostoreBankReagent& packet);
        void HandleBuyReagentBank(WorldPackets::Bank::BuyReagentBank& packet);
        void HandleDepositReagentBank(WorldPackets::Bank::DepositReagentBank& packet);
        void HandleWrapItem(WorldPackets::Item::WrapItem& packet);
        void HandleUseCritterItem(WorldPackets::Item::UseCritterItem& packet);

        void HandleAttackSwing(WorldPackets::Combat::AttackSwing& packet);
        void HandleAttackStop(WorldPackets::Combat::AttackStop& packet);
        void HandleSetSheathed(WorldPackets::Combat::SetSheathed& packet);

        void HandleUseItemOpcode(WorldPackets::Spells::ItemUse& recvPacket);
        void HandleOpenItem(WorldPackets::Spells::OpenItem& packet);
        void HandleCastSpellOpcode(WorldPackets::Spells::CastSpell& castRequest);
        void HandleCancelCast(WorldPackets::Spells::CancelCast& packet);
        void HandleCancelAura(WorldPackets::Spells::CancelAura& packet);
        void HandleCancelAutoRepeatSpellOpcode(WorldPackets::Spells::CancelAutoRepeatSpell& packet);
        void HandleUpdateSpellVisualOpcode(WorldPackets::Spells::UpdateSpellVisual& packet);

        void HandleConfirmRespecWipe(WorldPackets::Misc::ConfirmRespecWipe& packet);
        void HandleLearnTalent(WorldPackets::Talent::LearnTalent& packet);
        void HandleLearnPvpTalents(WorldPackets::Talent::LearnPvpTalents& packet);

        void HandleQuestGiverStatusQuery(WorldPackets::Quest::QuestGiverStatusQuery& packet);
        void HandleQuestgiverStatusMultipleQuery(WorldPackets::Quest::QuestGiverStatusMultipleQuery& packet);
        void HandleRequestAreaPoiUpdate(WorldPackets::Quest::RequestAreaPoiUpdate& packet);
        void HandleQuestGiverHello(WorldPackets::Quest::QuestGiverHello& packet);
        void HandleQuestGiverAcceptQuest(WorldPackets::Quest::QuestGiverAcceptQuest& packet);
        void HandleQuestGiverQueryQuest(WorldPackets::Quest::QuestGiverQueryQuest& packet);
        void HandleQuestGiverChooseReward(WorldPackets::Quest::QuestGiverChooseReward& packet);
        void HandleQuestGiverRequestReward(WorldPackets::Quest::QuestGiverRequestReward& packet);
        void HandleQueryQuestInfo(WorldPackets::Quest::QueryQuestInfo& packet);
        void HandleQueryTreasurePicker(WorldPackets::Quest::QueryTreasurePicker& packet);
        void HandleQuestLogRemoveQuest(WorldPackets::Quest::QuestLogRemoveQuest& packet);
        void HandleQuestConfirmAccept(WorldPackets::Quest::QuestConfirmAccept& packet);
        void HandleQuestgiverCompleteQuest(WorldPackets::Quest::QuestGiverCompleteQuest& packet);
        void HandlePushQuestToParty(WorldPackets::Quest::PushQuestToParty& packet);
        void HandleQuestPushResult(WorldPackets::Quest::QuestPushResult& packet);
        void HandleRequestWorldQuestUpdate(WorldPackets::Quest::RequestWorldQuestUpdate& packet);

        void HandleAdventureJournalOpenQuest(WorldPackets::Quest::AdventureJournalOpenQuest& packet);
        void HandleAdventureJournalStartQuest(WorldPackets::Quest::AdventureJournalStartQuest& packet);
        void HandleQueryAdventureMapPOI(WorldPackets::Quest::QueryAdventureMapPOI& packet);

        void HandleTransmogrifyItems(WorldPackets::Transmogrification::TransmogrifyItems& transmogrifyItems);

        void SendQuestgiverStatusMultipleQuery();

        bool processChatmessageFurtherAfterSecurityChecks(std::string&, uint32);
        void HandleChatMessageOpcode(WorldPackets::Chat::ChatMessage& packet);
        void HandleChatMessageAFK(WorldPackets::Chat::ChatMessageAFK& packet);
        void HandleChatMessageDND(WorldPackets::Chat::ChatMessageDND& packet);
        void HandleChatMessageWhisperOpcode(WorldPackets::Chat::ChatMessageWhisper& packet);
        void HandleChatMessageChannelOpcode(WorldPackets::Chat::ChatMessageChannel& packet);
        void HandleChatMessageEmoteOpcode(WorldPackets::Chat::ChatMessageEmote& packet);
        void HandleChatMessage(ChatMsg type, uint32 lang, std::string msg, std::string target = "");
        void HandleChatAddonMessageOpcode(WorldPackets::Chat::ChatAddonMessage& packet);
        void HandleChatAddonMessageWhisper(WorldPackets::Chat::ChatAddonMessageWhisper& packet);
        void HandleChatAddonMessageChannel(WorldPackets::Chat::ChatAddonMessageChannel& packet);
        void HandleChatAddonMessage(ChatMsg type, std::string const& prefix, std::string& message, std::string const& targetName = "");

        void SendPlayerNotFoundNotice(std::string const& name);
        void SendPlayerAmbiguousNotice(std::string const& name);
        void SendChatRestrictedNotice(ChatRestrictionType restriction);
        void HandleTextEmoteOpcode(WorldPackets::Chat::CTextEmote& packet);
        void HandleChatReportIgnored(WorldPackets::Chat::ChatReportIgnored& packet);

        void HandleChatUnregisterAllAddonPrefixes(WorldPackets::Chat::ChatUnregisterAllAddonPrefixes& packet);
        void HandleChatRegisterAddonPrefixes(WorldPackets::Chat::ChatRegisterAddonPrefixes& packet);

        void HandleReclaimCorpse(WorldPackets::Misc::ReclaimCorpse& packet);
        void HandleQueryCorpseLocation(WorldPackets::Query::QueryCorpseLocationFromClient& packet);
        void HandleQueryCorpseTransport(WorldPackets::Query::QueryCorpseTransport& packet);
        void HandleResurrectResponse(WorldPackets::Misc::ResurrectResponse& packet);
        void HandleSummonResponse(WorldPackets::Movement::SummonResponse& packet);

        void HandleChannelCommandAnnounce(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandBan(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelCommandDeclineInvite(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelCommandList(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelCommandSendWhoOwner(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandInvite(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandKick(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandSetModerator(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandSetMute(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandSetOwner(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandSilenceAll(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandUnBan(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandUnsetModerator(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandUnsetMute(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandUnsilenceAll(WorldPackets::Channel::ChannelPlayerCommand& packet);
        void HandleChannelPlayerCommandPassword(WorldPackets::Channel::ChannelPlayerCommand& packet);
        
        void HandleJoinChannel(WorldPackets::Channel::JoinChannel& packet);
        void HandleLeaveChannel(WorldPackets::Channel::LeaveChannel& packet);

        void HandleCompleteCinematic(WorldPackets::Misc::CompleteCinematic& packet);
        void HandleNextCinematicCamera(WorldPackets::Misc::NextCinematicCamera& packet);
        void HandleCompleteMovie(WorldPackets::Misc::CompleteMovie& packet);

        void HandleQueryPageText(WorldPackets::Query::QueryPageText& packet);

        void HandleTutorialFlag(WorldPackets::Misc::TutorialSetFlag& packet);

        void HandlePetAction(WorldPackets::PetPackets::PetAction& packet);
        void HandleStopAttack(WorldPackets::PetPackets::StopAttack& packet);
        void HandlePetActionHelper(Unit* pet, ObjectGuid petGuid, uint32 spellid, uint16 flag, ObjectGuid targetGuid, Position const& pos);
        void HandleQueryPetName(WorldPackets::Query::QueryPetName& packet);
        void HandlePetSetAction(WorldPackets::PetPackets::PetSetAction& packet);
        void HandlePetAbandon(WorldPackets::PetPackets::PetAbandon& packet);
        void HandlePetRename(WorldPackets::PetPackets::PetRename& packet);
        void HandlePetCancelAura(WorldPackets::PetPackets::PetCancelAura& packet);
        void HandlePetSpellAutocast(WorldPackets::PetPackets::PetSpellAutocast& packet);
        void HandlePetCastSpellOpcode(WorldPackets::Spells::PetCastSpell& recvPacket);

        void HandleSetActionBarToggles(WorldPackets::Character::SetActionBarToggles& packet);

        void HandleCharacterRenameRequest(WorldPackets::Character::CharacterRenameRequest& packet);
        void HandleSetPlayerDeclinedNames(WorldPackets::Character::SetPlayerDeclinedNames& packet);
        void SendSetPlayerDeclinedNamesResult(DeclinedNameResult result, ObjectGuid guid);

        void HandleTotemDestroyed(WorldPackets::Totem::TotemDestroyed& packet);
        void HandleDismissCritter(WorldPackets::PetPackets::DismissCritter& packet);

        void HandleBattlemasterHello(WorldPackets::NPC::Hello& packet);
        void HandleBattlemasterJoin(WorldPackets::Battleground::Join& packet);
        void HandlePVPLogData(WorldPackets::Battleground::NullCmsg& packet);
        void HandleUpdatePrestigeLevel(WorldPackets::Battleground::NullCmsg& packet);
        void HandleBattleFieldPort(WorldPackets::Battleground::Port& packet);
        void HandleBattlefieldList(WorldPackets::Battleground::ListClient& packet);
        void HandleLeaveBattlefield(WorldPackets::Battleground::NullCmsg& packet);
        void HandleBattlemasterJoinArena(WorldPackets::Battleground::JoinArena& packet);
        void HandleJoinSkirmish(WorldPackets::Battleground::JoinSkirmish& packet);
        void HandleJoinRatedBattleground(WorldPackets::Battleground::JoinRatedBattleground& packet);
        void HandleRequestPvpBrawlInfo(WorldPackets::Battleground::RequestPvpBrawlInfo& packet);
        void HandleBattlemasterJoinBrawl(WorldPackets::Battleground::BattlemasterJoinBrawl& packet);
        void JoinBracket(uint8 slot, uint8 rolesMask = ROLES_DEFAULT, uint16 extraBgTypeId = 0);

        void HandleReportPvPPlayerAFK(WorldPackets::Battleground::ReportPvPPlayerAFK& packet);
        void HandleRequestRatedInfo(WorldPackets::Battleground::NullCmsg& packet);
        void HandleRequestPvpOptions(WorldPackets::Battleground::NullCmsg& packet);
        void HandleRequestPvpReward(WorldPackets::Battleground::RequestPVPRewards& packet);

        void HandleMinimapPing(WorldPackets::Party::MinimapPingClient& packet);
        void HandleRandomRollClient(WorldPackets::Misc::RandomRollClient& packet);
        void HandleFarSight(WorldPackets::Misc::FarSight& packet);
        void HandleSetDungeonDifficulty(WorldPackets::Misc::SetDungeonDifficulty& packet);
        void HandleSetRaidDifficulty(WorldPackets::Misc::SetRaidDifficulty& packet);
        void HandleSetTitle(WorldPackets::Character::SetTitle& packet);
        void HandleQueryRealmName(WorldPackets::Query::QueryRealmName& packet);
        void HandleTimeSyncResponse(WorldPackets::Movement::TimeSyncResponse& packet);
        void HandleWhoisOpcode(WorldPackets::Who::WhoIsRequest& packet);
        void HandleResetInstances(WorldPackets::Instance::ResetInstances& packet);
        void HandleHearthAndResurrect(WorldPackets::Battleground::HearthAndResurrect& packet);
        void HandleInstanceLockResponse(WorldPackets::Instance::InstanceLockResponse& packet);
        void HandleQueryWorldCountwodnTimer(WorldPackets::Instance::QueryWorldCountwodnTimer& packet);
        void HandlePersonalRatedInfoRequest(WorldPackets::Battleground::NullCmsg& packet);

        // Battlefield
        void SendBfInvitePlayerToWar(uint64 const& guid,uint32 ZoneId,uint32 time);
        void SendBfInvitePlayerToQueue(uint64 queueId, int8 battleState);
        void SendBfQueueInviteResponse(uint64 queueId, uint32 zoneId, int8 battleStatus, bool canQueue = true, bool loggingIn = false);
        void SendBfEntered(uint64 queueId, bool relocated, bool onOffense);
        void HandleSetCemetryPreferrence(WorldPackets::Battleground::SetCemetryPreferrence& packet);

        void HandleLockInfoRequest(WorldPackets::LFG::LockInfoRequest& packet);
        void HandleLfgJoin(WorldPackets::LFG::LfgJoin& packet);
        void HandleLfgLeave(WorldPackets::LFG::LfgLeave& packet);
        void HandleLfgCompleteRoleCheck(WorldPackets::LFG::LfgCompleteRoleCheck& packet);
        void HandleLfgProposalResponse(WorldPackets::LFG::LfgProposalResponse& packet);
        void HandleLfgBootPlayerVote(WorldPackets::LFG::LfgBootPlayerVote& packet);
        void HandleLfgCompleteReadyCheck(WorldPackets::LFG::CompleteReadyCheck& packet);
        void HandleLfgTeleport(WorldPackets::LFG::LfgTeleport& packet);
        void HandleDfGetJoinStatus(WorldPackets::LFG::NullCmsg& packet);

        void HandleAcceptWargameInvite(WorldPackets::Battleground::AcceptWargameInvite& packet);
        void HandleStartWarGame(WorldPackets::Battleground::StartWargame& packet);
        void HandleRequstCrowdControlSpell(WorldPackets::Battleground::RequstCrowdControlSpell& packet);

        void HandleAreaSpiritHealerQuery(WorldPackets::Battleground::AreaSpiritHealerQuery& packet);
        void HandleAreaSpiritHealerQueue(WorldPackets::Battleground::AreaSpiritHealerQueue& packet);
        void HandleCancelMountAura(WorldPackets::Spells::CancelMountAura& packet);
        void HandleUnlearnSkill(WorldPackets::Spells::UnlearnSkill& packet);
        void HandleUnlearnSpecialization(WorldPackets::Spells::UnlearnSpecialization& packet);
        void HandleCancelGrowthAura(WorldPackets::Spells::CancelGrowthAura& packet);
        void HandleSelfRes(WorldPackets::Spells::SelfRes& packet);
        void HandleComplaint(WorldPackets::Ticket::Complaint& packet);
        void HandleRequestPetInfo(WorldPackets::PetPackets::RequestPetInfo& packet);

        void HandleSocketGems(WorldPackets::Item::SocketGems& packet);

        void HandleSortBags(WorldPackets::Item::SortBags& packet);
        void HandleSortBankBags(WorldPackets::Item::SortBankBags& packet);
        void HandleSortReagentBankBags(WorldPackets::Item::SortReagentBankBags& packet);

        void HandleCancelTempEnchantmentOpcode(WorldPackets::Item::CancelTempEnchantment& packet);

        void HandleGetItemPurchaseData(WorldPackets::Item::ItemRefundInfo& packet);
        void HandleItemPurchaseRefund(WorldPackets::Item::ItemPurchaseRefund& packet);
        void HandleItemTextQuery(WorldPackets::Query::ItemTextQuery& packet);
        void HandleSetTaxiBenchmarkMode(WorldPackets::Taxi::SetTaxiBenchmarkMode& packet);

        // Guild Bank
        void HandleGuildPermissions(WorldPackets::Guild::GuildPermissionsQuery& packet);
        void HandleGuildBankMoneyWithdrawn(WorldPackets::Guild::GuildBankRemainingWithdrawMoneyQuery& packet);
        void HandleGuildBankActivate(WorldPackets::Guild::GuildBankActivate& packet);
        void HandleGuildBankQueryTab(WorldPackets::Guild::GuildBankQueryTab& packet);
        void HandleGuildBankLogQuery(WorldPackets::Guild::GuildBankLogQuery& packet);
        void HandleGuildBankDepositMoney(WorldPackets::Guild::GuildBankDepositMoney& packet);
        void HandleGuildBankWithdrawMoney(WorldPackets::Guild::GuildBankWithdrawMoney& packet);
        void HandleGuildBankSwapItemsLegacy(WorldPackets::Guild::GuildBankSwapItemsLegacy& packet);
        void HandleGuildBankMoveItemsPlayerBank(WorldPackets::Guild::GuildBankSwapItems& packet);
        void HandleGuildBankMoveItemsBankPlayer(WorldPackets::Guild::GuildBankSwapItems& packet);
        void HandleGuildBankMoveItemsBankBank(WorldPackets::Guild::GuildBankSwapItemsBankBank& packet);
        void HandleGuildBankMoveItemsPlayerBankCount(WorldPackets::Guild::GuildBankSwapItemsCount& packet);
        void HandleGuildBankMoveItemsBankPlayerCount(WorldPackets::Guild::GuildBankSwapItemsCount& packet);
        void HandleGuildBankMoveItemsBankPlayerAuto(WorldPackets::Guild::GuildBankSwapItemsAuto& packet);
        void HandleGuildBankMoveItemsBankBankCount(WorldPackets::Guild::GuildBankSwapItemsBankBankCount& packet);
        void HandleGuildBankMergeItemsPlayerBank(WorldPackets::Guild::GuildBankSwapItemsCount& packet);
        void HandleGuildBankMergeItemsBankPlayer(WorldPackets::Guild::GuildBankSwapItemsCount& packet);
        void HandleGuildBankMergeItemsBankBank(WorldPackets::Guild::GuildBankSwapItemsBankBankCount& packet);
        void HandleGuildBankSwapItemsBankPlayer(WorldPackets::Guild::GuildBankSwapItems& packet);
        void HandleGuildBankSwapItemsBankBank(WorldPackets::Guild::GuildBankSwapItemsBankBank& packet);

        void HandleGuildBankUpdateTab(WorldPackets::Guild::GuildBankUpdateTab& packet);
        void HandleGuildBankBuyTab(WorldPackets::Guild::GuildBankBuyTab& packet);
        void HandleGuildBankTextQuery(WorldPackets::Guild::GuildBankTextQuery& packet);
        void HandleGuildBankSetTabText(WorldPackets::Guild::GuildBankSetTabText& packet);

        void HandleGrantLevel(WorldPackets::ReferAFriend::GrantLevel& packet);
        void HandleAcceptGrantLevel(WorldPackets::ReferAFriend::AcceptLevelGrant& packet);

        void HandleCalendarGetCalendar(WorldPackets::Calendar::CalendarGetCalendar& packet);
        void HandleCalendarGetEvent(WorldPackets::Calendar::CalendarGetEvent& packet);
        void HandleCalendarGuildFilter(WorldPackets::Calendar::CalendarGuildFilter& packet);
        void HandleCalendarAddEvent(WorldPackets::Calendar::CalendarAddEvent& packet);
        void HandleCalendarUpdateEvent(WorldPackets::Calendar::CalendarUpdateEvent& packet);
        void HandleCalendarRemoveEvent(WorldPackets::Calendar::CalendarRemoveEvent& packet);
        void HandleCalendarCopyEvent(WorldPackets::Calendar::CalendarCopyEvent& packet);
        void HandleCalendarEventInvite(WorldPackets::Calendar::CalendarEventInvite& packet);
        void HandleCalendarEventRsvp(WorldPackets::Calendar::CalendarEventRSVP& packet);
        void HandleCalendarEventRemoveInvite(WorldPackets::Calendar::CalendarRemoveInvite& packet);
        void HandleCalendarEventStatus(WorldPackets::Calendar::CalendarEventStatus& packet);
        void HandleCalendarEventModeratorStatus(WorldPackets::Calendar::CalendarEventModeratorStatus& packet);
        void HandleCalendarGetNumPending(WorldPackets::Calendar::CalendarGetNumPending& packet);
        void HandleCalendarEventSignup(WorldPackets::Calendar::CalendarEventSignUp& packet);
        void HandleSetSavedInstanceExtend(WorldPackets::Calendar::SetSavedInstanceExtend& packet);
        void HandleCalendarComplain(WorldPackets::Calendar::CalendarComplain& packet);

        // Void Storage
        void HandleVoidStorageUnlock(WorldPackets::VoidStorage::UnlockVoidStorage& packet);
        void HandleVoidStorageQuery(WorldPackets::VoidStorage::QueryVoidStorage& packet);
        void HandleVoidStorageTransfer(WorldPackets::VoidStorage::VoidStorageTransfer& packet);
        void HandleVoidSwapItem(WorldPackets::VoidStorage::SwapVoidItem& packet);
        void SendVoidStorageTransferResult(VoidTransferError result);
        void SendVoidStorageFailed(int8 reason = 0);

        void HandleCollectionItemSetFavorite(WorldPackets::Collections::CollectionItemSetFavorite& collectionItemSetFavorite);
        void HandleMountClearFanfare(WorldPackets::Collections::MountClearFanfare& packet);
        void HandleBattlePetClearFanfare(WorldPackets::Collections::BattlePetClearFanfare& packet);

        void HandleUpgradeItem(WorldPackets::Item::UpgradeItem& packet);

        void HandleBattlePetSetFlags(WorldPackets::BattlePet::SetFlags& packet);
        void HandleModifyName(WorldPackets::BattlePet::ModifyName& packet);
        void HandleBattlePetNameQuery(WorldPackets::BattlePet::Query& packet);
        void HandleCageBattlePet(WorldPackets::BattlePet::BattlePetGuidRead& packet);
        void HandleBattlePetSetSlot(WorldPackets::BattlePet::SetBattleSlot& packet);
        void HandleBattlePetSummon(WorldPackets::BattlePet::BattlePetGuidRead& packet);
        void HandleBattlePetUpdateNotify(WorldPackets::BattlePet::BattlePetGuidRead& packet);
        void HandlePetBattleRequestWild(WorldPackets::BattlePet::RequestWild& packet);
        void HandlePetBattleRequestUpdate(WorldPackets::BattlePet::RequestUpdate& packet);
        void HandlePetBattleInput(WorldPackets::BattlePet::PetBattleInput& packet);
        void HandlePetBattleFinalNotify(WorldPackets::BattlePet::NullCmsg& packet);
        void HandlePetBattleQuitNotify(WorldPackets::BattlePet::NullCmsg& packet);
        void HandleBattlePetDelete(WorldPackets::BattlePet::BattlePetGuidRead& packet);
        void HandleBattlePetRequestJournal(WorldPackets::BattlePet::NullCmsg& packet);
        void HandleBattlePetJournalLock(WorldPackets::BattlePet::NullCmsg& packet);
        void HandleJoinPetBattleQueue(WorldPackets::BattlePet::NullCmsg& packet);
        void HandlePetBattleScriptErrorNotify(WorldPackets::BattlePet::NullCmsg& packet);
        void HandleBattlePetDeletePetCheat(WorldPackets::BattlePet::BattlePetGuidRead& packet);
        void HandlePetBattleRequestPVP(WorldPackets::BattlePet::RequestPVP& packet);
        void HandleReplaceFrontPet(WorldPackets::BattlePet::ReplaceFrontPet& packet);
        void HanldeQueueProposeMatchResult(WorldPackets::BattlePet::QueueProposeMatchResult& packet);
        void HandleLeaveQueue(WorldPackets::BattlePet::LeaveQueue& packet);
        void SendBattlePetUpdates(BattlePet* pet = nullptr, bool add = false);
        void SendBattlePetTrapLevel();
        void SendBattlePetJournalLockAcquired();
        void SendBattlePetJournalLockDenied();
        void SendBattlePetJournal();
        void SendBattlePetDeleted(ObjectGuid battlePetGUID);
        void SendBattlePetRevoked(ObjectGuid battlePetGUID);
        void SendBattlePetRestored(ObjectGuid battlePetGUID);
        void SendBattlePetsHealed();
        void SendBattlePetLicenseChanged();
        void SendBattlePetError(BattlePetError result, uint32 creatureID);
        void SendBattlePetCageDateError(uint32 secondsUntilCanCage);

        void SendPetBattleSlotUpdates(bool newSlotUnlocked = false);
        void SendPetBattleRequestFailed(uint8 p_Reason);
        void SendPetBattlePvPChallenge(PetBattleRequest* petBattleRequest);
        void SendPetBattleFinalizeLocation(PetBattleRequest* petBattleRequest);
        void SendPetBattleInitialUpdate(PetBattle* petBattle);
        void SendPetBattleFirstRound(PetBattle* petBattle);
        void SendPetBattleRoundResult(PetBattle* petBattle);
        void SendPetBattleReplacementMade(PetBattle* petBattle);
        void SendPetBattleFinalRound(PetBattle* petBattle);
        void SendPetBattleFinished();
        void SendPetBattleChatRestricted();
        void SendPetBattleQueueProposeMatch();
        void SendPetBattleQueueStatus(uint32 p_TicketTime, uint32 p_TicketID, uint32 p_Status, uint32 p_AvgWaitTime);

        void HandleSpellClick(WorldPackets::Spells::SpellClick& packet);
        void HandleGetMirrorImageData(WorldPackets::Spells::GetMirrorImageData& packet);
        void HandleAlterAppearance(WorldPackets::Character::AlterApperance& packet);
        void HandleCharCustomize(WorldPackets::Character::CharCustomize& packet);
        void HandleCharCustomizeCallback(std::shared_ptr<WorldPackets::Character::CharCustomizeInfo> customizeInfo, PreparedQueryResult result);
        void HandleUndeleteCharacter(WorldPackets::Character::UndeleteCharacter& packet);
        void HandleQueryInspectAchievements(WorldPackets::Inspect::QueryInspectAchievements& inspect);
        void HandleGuildAchievementProgressQuery(WorldPackets::Achievement::GuildGetAchievementMembers& packet);
        void HandleSetAchievementsHidden(WorldPackets::Achievement::SetAchievementsHidden& packet);
        void HandleSaveEquipmentSet(WorldPackets::EquipmentSet::SaveEquipmentSet& packet);
        void HandleDeleteEquipmentSet(WorldPackets::EquipmentSet::DeleteEquipmentSet& packet);
        void HandleEquipmentSetUse(WorldPackets::EquipmentSet::UseEquipmentSet& packet);
        void HandleAssignEquipmentSetSpec(WorldPackets::EquipmentSet::AssignEquipmentSetSpec& packet);
        void HandleUITimeRequest(WorldPackets::Misc::UITimeRequest& /*request*/);
        void HandleQueryQuestCompletionNPCs(WorldPackets::Query::QueryQuestCompletionNPCs& packet);
        void HandleQuestPOIQuery(WorldPackets::Query::QuestPOIQuery& packet);
        void HandleEjectPassenger(WorldPackets::Vehicle::EjectPassenger& packet);
        void HandleRideVehicleInteract(WorldPackets::Vehicle::RideVehicleInteract& packet);
        void HandleSetVehicleRecId(WorldPackets::Vehicle::MoveSetVehicleRecIdAck& packet);
        void HandleMissileTrajectoryCollision(WorldPackets::Spells::MissileTrajectoryCollision& packet);
        void HandleCancelModSpeedNoControlAuras(WorldPackets::Spells::CancelModSpeedNoControlAuras& packet);
        void HandleCancelQueuedSpell(WorldPackets::Spells::CancelQueuedSpell& packet);
        void HandleDBQueryBulk(WorldPackets::Hotfix::DBQueryBulk& packet);
        void SendHotfixList(int32 version);
        void HandleUpdateMissileTrajectory(WorldPackets::Spells::UpdateMissileTrajectory& packet);
        void HandleViolenceLevel(WorldPackets::Misc::ViolenceLevel& packet);
        void HandleObjectUpdateFailed(WorldPackets::Misc::ObjectUpdateFailed& packet);
        void HandleObjectUpdateRescued(WorldPackets::Misc::ObjectUpdateRescued& packet);
        void HandleOpeningCinematic(WorldPackets::Misc::OpeningCinematic& packet);
        void HandleNeutralPlayerSelectFaction(WorldPackets::Character::NeutralPlayerSelectFaction& packet);
        void HandleRequestResearchHistory(WorldPackets::Misc::RequestResearchHistory& packet);
        void HandleSetCurrencyFlags(WorldPackets::Character::SetCurrencyFlags& packet);
        void HandleRequestCategoryCooldowns(WorldPackets::Spells::RequestCategoryCooldowns& packet);
        void HandleClearRaidMarker(WorldPackets::Party::ClearRaidMarker& packet);
        void HandleShowTradeSkill(WorldPackets::Misc::ShowTradeSkill& packet);
        void HandleForcedReactions(WorldPackets::Reputation::RequestForcedReactions& packet);
        void HandleSaveCUFProfiles(WorldPackets::Misc::SaveCUFProfiles& packet);
        void SendLoadCUFProfiles();
        void HandleBattlePayDistributionAssign(WorldPackets::BattlePay::DistributionAssignToTarget& packet);
        void HandleBattlePayStartPurchase(WorldPackets::BattlePay::StartPurchase& packet);
        void HandleBattlePayPurchaseProduct(WorldPackets::BattlePay::PurchaseProduct& packet);
        void HandleBattlePayConfirmPurchase(WorldPackets::BattlePay::ConfirmPurchaseResponse& packet);
        void HandleBattlePayAckFailedResponse(WorldPackets::BattlePay::BattlePayAckFailedResponse& packet);

        void HandleGetGarrisonInfo(WorldPackets::Garrison::GetGarrisonInfo& packet);
        void HandleGarrisonPurchaseBuilding(WorldPackets::Garrison::GarrisonPurchaseBuilding& packet);
        void HandleGarrisonCancelConstruction(WorldPackets::Garrison::GarrisonCancelConstruction& packet);
        void HandleGarrisonRequestBlueprintAndSpecializationData(WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationData& packet);
        void HandleGarrisonGetBuildingLandmarks(WorldPackets::Garrison::GarrisonGetBuildingLandmarks& packet);
        void HandleGarrisonMissionBonusRoll(WorldPackets::Garrison::GarrisonMissionBonusRoll& packet);
        void HandleGarrisonRequestLandingPageShipmentInfo(WorldPackets::Garrison::GarrisonRequestLandingPageShipmentInfo& packet);
        bool AdventureMapPOIAvailable(uint32 adventureMapPOIID);
        void HandleGarrisonRequestScoutingMap(WorldPackets::Garrison::GarrisonRequestScoutingMap& scoutingMap);
        void HandleGarrisonCheckUpgradeable(WorldPackets::Garrison::GarrisonCheckUpgradeable& packet);
        void HandleGarrisonStartMission(WorldPackets::Garrison::GarrisonStartMission& packet);
        void HandleGarrisonSwapBuildings(WorldPackets::Garrison::GarrisonSwapBuildings& packet);
        void HandleGarrisonCompleteMission(WorldPackets::Garrison::GarrisonCompleteMission& packet);
        void HandleCreateShipment(WorldPackets::Garrison::CreateShipment& packet);
        void HandleGarrisonRequestShipmentInfo(WorldPackets::Garrison::GarrisonRequestShipmentInfo& packet);
        void HandleGarrisonResearchTalent(WorldPackets::Garrison::GarrisonRequestResearchTalent& packet);
        void HandleGarrisonOpenMissionNpc(WorldPackets::Garrison::GarrisonOpenMissionNpcRequest& packet);
        void HandleUpgradeGarrison(WorldPackets::Garrison::UpgradeGarrison& packet);
        void HandleRequestClassSpecCategoryInfo(WorldPackets::Garrison::GarrisonRequestClassSpecCategoryInfo& packet);
        void HandleTrophyData(WorldPackets::Garrison::TrophyData& packet);
        void HandleRevertTrophy(WorldPackets::Garrison::RevertTrophy& packet);
        void HandleGetTrophyList(WorldPackets::Garrison::GetTrophyList& packet);
        void HandleGarrisonSetFollowerInactive(WorldPackets::Garrison::GarrisonSetFollowerInactive& packet);
        void HandleGarrisonRemoveFollowerFromBuilding(WorldPackets::Garrison::GarrisonRemoveFollowerFromBuilding& packet);
        void HandleGarrisonAssignFollowerToBuilding(WorldPackets::Garrison::GarrisonAssignFollowerToBuilding& packet);
        void HandleGarrisonGenerateRecruits(WorldPackets::Garrison::GarrisonGenerateRecruits& packet);
        void HandleGarrisonRecruitFollower(WorldPackets::Garrison::GarrisonRecruitFollower& packet);
        void HandleGarrisonRemoveFollower(WorldPackets::Garrison::GarrisonRemoveFollower& packet);
        void HandleGarrisonRenameFollower(WorldPackets::Garrison::GarrisonRenameFollower& packet);
        void HandleGarrisonSetRecruitmentPreferences(WorldPackets::Garrison::GarrisonSetRecruitmentPreferences& packet);

        void HandleAddToy(WorldPackets::Toy::AddToy& packet);
        void HandleUseToy(WorldPackets::Toy::UseToy& packet);

        // Commentator
        void HandleCommentatorEnable(WorldPackets::Commentator::CommentatorEnable& packet);
        void HandleCommentatorGetMapInfo(WorldPackets::Commentator::CommentatorGetMapInfo& packet);
        void HandleCommentatorGetPlayerInfo(WorldPackets::Commentator::CommentatorGetPlayerInfo& packet);
        void HandleCommentatorEnterInstance(WorldPackets::Commentator::CommentatorEnterInstance& packet);
        void HandleCommentatorExitInstance(WorldPackets::Commentator::CommentatorExitInstance& packet);

        // Scenario
        void HandleQueryScenarioPOI(WorldPackets::Scenario::QueryScenarioPOI& packet);

        void HandleGetChallengeModeRewards(WorldPackets::ChallengeMode::Misc& packet);
        void HandleChallengeModeRequestMapStats(WorldPackets::ChallengeMode::Misc& packet);
        void HandleRequestLeaders(WorldPackets::ChallengeMode::RequestLeaders& packet);
        void HandleStartChallengeMode(WorldPackets::ChallengeMode::StartChallengeMode& packet);
        void HandleResetChallengeMode(WorldPackets::ChallengeMode::ResetChallengeMode& packet);

        void HandleScenePlaybackCanceled(WorldPackets::Scene::SceneInstance& packet);
        void HandleScenePlaybackComplete(WorldPackets::Scene::SceneInstance& packet);
        void HandleSceneTriggerEvent(WorldPackets::Scene::SceneTriggerEvent& packet);
        void HandleMountSetFavorite(WorldPackets::Misc::MountSetFavorite& packet);
        void HandleSuspendTokenResponse(WorldPackets::Movement::SuspendTokenResponse& packet);
        void HandleGetUndeleteCharacterCooldownStatus(WorldPackets::Character::GetUndeleteCharacterCooldownStatus& packet);
        void HandleUndeleteCooldownStatusCallback(PreparedQueryResult const& result);
        void HandleWowTokenMarketPrice(WorldPackets::Token::RequestWowTokenMarketPrice& packet);
        void HandleUpdateListedAuctionableTokens(WorldPackets::Token::UpdateListedAuctionableTokens& packet);
        void HandleRequestConsumptionConversionInfo(WorldPackets::Misc::RequestConsumptionConversionInfo& packet);
        void HandleCheckVeteranTokenEligibility(WorldPackets::Token::CheckVeteranTokenEligibility& packet);
        void LootCorps(ObjectGuid corpsGUID, WorldObject* lootedBy = nullptr);

        void HandleContributionCollectorContribute(WorldPackets::Misc::ContributionCollectorContribute& packet);
        void HandleContributionGetState(WorldPackets::Misc::ContributionGetState& packet);
        void HandleHotfixRequest(WorldPackets::Hotfix::HotfixRequest& packet);

        // Battle Pay
        AuthFlags GetAF() const { return atAuthFlag;  }
        bool HasAuthFlag(AuthFlags f) const { return atAuthFlag & f; }
        void AddAuthFlag(AuthFlags f);
        void RemoveAuthFlag(AuthFlags f);
        void SaveAuthFlag();
        int64 GetBattlePayBalance() { return battlePayBalance; }
        void ChangeBattlePayBalance(int64 change) { battlePayBalance += change; }

        void SendCharacterEnum(bool deleted = false);

        void SetBankerGuid(ObjectGuid const& g) { m_currentBankerGUID = g; }
        void SetWardenModuleFailed(bool s) { wardenModuleFailed = s; }
        bool IsWardenModuleFailed() { return wardenModuleFailed; }

        void HandleTwitterConnect(WorldPackets::Misc::TwitterConnect& packet);
        void HandleTwitterDisconnect(WorldPackets::Misc::TwitterDisconnect& packet);
        void HandleResetChallengeModeCheat(WorldPackets::Misc::ResetChallengeModeCheat& packet);

        void HandleBattlenetRequest(WorldPackets::Battlenet::Request& request);
        void HandleBattlenetRequestRealmListTicket(WorldPackets::Battlenet::RequestRealmListTicket& requestRealmListTicket);
        void SendBattlenetResponse(uint32 serviceHash, uint32 methodId, uint32 token, pb::Message const* response);
        void SendBattlenetResponse(uint32 serviceHash, uint32 methodId, uint32 token, uint32 status);
        void SendBattlenetRequest(uint32 serviceHash, uint32 methodId, pb::Message const* request, std::function<void(MessageBuffer)> callback);
        void SendBattlenetRequest(uint32 serviceHash, uint32 methodId, pb::Message const* request);

        std::array<uint8, 32> const& GetRealmListSecret() const { return _realmListSecret; }
        void SetRealmListSecret(std::array<uint8, 32> const& secret) { memcpy(_realmListSecret.data(), secret.data(), secret.size()); }

        std::unordered_map<uint32, uint8> const& GetRealmCharacterCounts() const { return _realmCharacterCounts; }

        union ConnectToKey
        {
            struct
            {
                uint64 AccountId : 32;
                uint64 ConnectionType : 1;
                uint64 Key : 31;
            } Fields;

            uint64 Raw;
        };

        uint64 GetConnectToInstanceKey() const { return _instanceConnectKey.Raw; }

        void SetforceExit(bool force = true) { forceExit = force; }
        bool IsforceExit() { return forceExit; }
        
		float GetPersonalXPRate() { return PersonalXPRate; }
		void SetPersonalXPRate(float rate);

        void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
        {
            m_Functions.AddDelayedEvent(timeOffset, std::move(function));
        }

        FunctionProcessor m_Functions;
        uint32 _realmID;
        uint64 _hwid;
        int32 _countPenaltiesHwid;

        CharacterTemplateData* GetCharacterTemplateData(uint32 id);

        QueryCallbackProcessor _queryProcessor;

        BattlepayManager* GetBattlePayMgr() const { return _battlePayMgr.get(); }

        std::vector<bool> m_achievement;
        bool HasAchievement(uint32 achievementId);
        void SetAchievement(uint32 achievementId);
        void LoadAchievement(PreparedQueryResult const& result);

        uint32 m_classMask = 0;

    private:
        void ProcessQueryCallbacks();

        QueryResultHolderFuture _realmAccountLoginCallback;
        QueryResultHolderFuture _accountLoginCallback;
        QueryResultHolderFuture _charLoginCallback;

        void moveItems(Item* myItems[], Item* hisItems[]);

        bool CanUseBank(ObjectGuid bankerGUID = ObjectGuid::Empty) const;

        // logging helper
        void LogUnexpectedOpcode(WorldPacket* packet, const char* status, const char *reason);

        // movement
        void RelocateMover(MovementInfo &movementInfo);

        // EnumData helpers
        bool CharCanLogin(ObjectGuid::LowType lowGUID)
        {
            return _allowedCharsToLogin.find(lowGUID) != _allowedCharsToLogin.end();
        }

        std::set<ObjectGuid::LowType> _allowedCharsToLogin;

        ObjectGuid::LowType m_GUIDLow;                                   // set loggined or recently logout player (while m_playerRecentlyLogout set)
        Player* _player;
        std::shared_ptr<WorldSocket> m_Socket[MAX_CONNECTION_TYPES];
        std::string m_Address;
        Map* m_map;

        AccountTypes _security;
        uint32 _accountId;
        uint8 m_expansion;
        uint8 m_accountExpansion;
        std::string _accountName;
        std::string _os;

        std::array<uint8, 32> _realmListSecret;
        std::unordered_map<uint32 /*realmAddress*/, uint8> _realmCharacterCounts;
        std::unordered_map<uint32, std::function<void(MessageBuffer)>> _battlenetResponseCallbacks;
        uint32 _battlenetRequestToken;

        typedef std::list<AddonInfo> AddonsList;

        // Warden
        Warden* _warden;                                    // Remains NULL if Warden system is not enabled by config
        std::shared_ptr<BattlepayManager> _battlePayMgr;

        time_t _logoutTime;
        bool m_inQueue;                                     // session wait in auth.queue
        ObjectGuid m_playerLoading;                         // code processed in LoginPlayer
        bool m_playerLogout;                                // code processed in LogoutPlayer
        bool m_playerRecentlyLogout;
        bool m_playerSave;
        bool m_IsPetBattleJournalLocked;
        LocaleConstant m_sessionDbLocaleIndex;
        std::atomic<uint32> m_latency;
        std::atomic<uint32> m_clientTimeDelay;
        AccountData m_accountData[NUM_ACCOUNT_DATA_TYPES];
        uint32 _tutorials[MAX_ACCOUNT_TUTORIAL_VALUES];
        bool   _tutorialsChanged;
        AddonsList m_addonsList;
        StringSet _registeredAddonPrefixes;
        bool _filterAddonMessages;
        uint32 recruiterId;
        bool isRecruiter;
        LockedQueue<WorldPacket*> _recvQueue;
        time_t timeCharEnumOpcode;
        uint8 playerLoginCounter;
        uint32 expireTime;
        bool forceExit;
        std::atomic<bool> m_sUpdate;
        int64 battlePayBalance;

        bool wardenModuleFailed;

        ObjectGuid m_currentBankerGUID;

        AuthFlags atAuthFlag = AT_AUTH_FLAG_NONE;

        ConnectToKey _instanceConnectKey;
        uint8 m_DHCount = 0;
        uint8 m_DKCount = 0;
        bool m_canDK = false;
        bool m_canDH = false;
        uint32 m_raceMask = 0;
        CharEnumMap charEnumInfo;
        CharacterTemplateDataMap charTemplateData;

        bool canLogout;
        float PersonalXPRate = 0;
};

#endif
/// @}
