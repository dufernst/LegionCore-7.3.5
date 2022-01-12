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

#ifndef _PLAYER_H
#define _PLAYER_H

#include "AchievementMgr.h"
#include "Bag.h"
#include "Battleground.h"
#include "Common.h"
#include "DBCEnums.h"
#include "GroupReference.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "MapReference.h"
#include "Object.h"
#include "Packets/VehiclePackets.h"
#include "Pet.h"
#include "PhaseMgr.h"
#include "PlayerTaxi.h"
#include "QuestDef.h"
#include "ReputationMgr.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "Util.h"
#include "GridObject.h"
#include "DatabaseEnvFwd.h"
#include "HashFuctor.h"
#include "CUFProfile.h"
#include <safe_ptr.h>
#include "../../Vignette/VignetteMgr.h"
#include "EquipementSet.h"
#include "LogsSystem.h"

class SpectatorAddonMsg;
struct Mail;
struct ItemExtendedCostEntry;

class Bracket;
class Channel;
class Creature;
class DynamicObject;
class Garrison;
class Group;
class Guild;
class OutdoorPvP;
class Pet;
class PhaseMgr;
class PlayerMenu;
class PlayerSocial;
class SpellCastTargets;
class BattlePet;
class TradeData;
struct WorldQuest;
struct PvpReward;
class PlayerCheatData;

enum GroupCategory : uint8;

namespace lfg
{
    enum LfgRoles : uint8;
    struct LFGDungeonData;
    struct LfgReward;
}

namespace WorldPackets
{
    namespace Spells
    {
        struct SpellCastRequest;
    }
}

typedef std::unordered_map<uint32, LogsSystem::KillCreatureData> KillCreatureMap;
typedef std::vector<LogsSystem::KillCreatureData*> KillCreatureList;

struct GroupUpdateCounter
{
    ObjectGuid GroupGuid;
    int32 UpdateSequenceNumber;
};

typedef std::deque<Mail*> PlayerMails;

#define PLAYER_MAX_SKILLS           128
#define PLAYER_EXPLORED_ZONES_SIZE  320

enum SkillFieldOffset
{
    SKILL_ID_OFFSET                 = 0,
    SKILL_STEP_OFFSET               = 0x40,
    SKILL_RANK_OFFSET               = SKILL_STEP_OFFSET + 0x40,
    SUBSKILL_START_RANK_OFFSET      = SKILL_RANK_OFFSET + 0x40,
    SKILL_MAX_RANK_OFFSET           = SUBSKILL_START_RANK_OFFSET + 0x40,
    SKILL_TEMP_BONUS_OFFSET         = SKILL_MAX_RANK_OFFSET + 0x40,
    SKILL_PERM_BONUS_OFFSET         = SKILL_TEMP_BONUS_OFFSET + 0x40
};

// Note: SPELLMOD_* values is aura types in fact
enum SpellModType
{
    SPELLMOD_FLAT         = 107,                            // SPELL_AURA_ADD_FLAT_MODIFIER
    SPELLMOD_PCT          = 108                             // SPELL_AURA_ADD_PCT_MODIFIER
};

// 2^n values, Player::m_isunderwater is a bitmask. These are Trinity internal values, they are never send to any client
enum PlayerUnderwaterState
{
    UNDERWATER_NONE                     = 0x00,
    UNDERWATER_INWATER                  = 0x01,             // terrain type is water and player is afflicted by it
    UNDERWATER_INLAVA                   = 0x02,             // terrain type is lava and player is afflicted by it
    UNDERWATER_INSLIME                  = 0x04,             // terrain type is lava and player is afflicted by it
    UNDERWARER_INDARKWATER              = 0x08,             // terrain type is dark water and player is afflicted by it

    UNDERWATER_EXIST_TIMERS             = 0x10
};

enum BuyBankSlotResult
{
    ERR_BANKSLOT_FAILED_TOO_MANY    = 0,
    ERR_BANKSLOT_INSUFFICIENT_FUNDS = 1,
    ERR_BANKSLOT_NOTBANKER          = 2,
    ERR_BANKSLOT_OK                 = 3
};

enum PlayerSpellState
{
    PLAYERSPELL_UNCHANGED = 0,
    PLAYERSPELL_CHANGED   = 1,
    PLAYERSPELL_NEW       = 2,
    PLAYERSPELL_REMOVED   = 3,
    PLAYERSPELL_TEMPORARY = 4
};

struct PlayerSpell
{
    PlayerSpell(uint8 _state, bool _active, bool _dependent, bool _disabled)
    : state(PlayerSpellState(_state)), active(_active), dependent(_dependent), disabled(_disabled) {}

    PlayerSpellState state : 8;
    bool active            : 1;                             // show in spellbook
    bool dependent         : 1;                             // learned as result another spell learn, skill grow, quest reward, etc
    bool disabled          : 1;                             // first rank has been learned in result talent learn but currently talent unlearned, save max learned ranks
};

// Spell modifier (used for modify other spells)
struct SpellModifier
{
    SpellModifier(Aura* _ownerAura = nullptr) : op(SPELLMOD_DAMAGE), type(SPELLMOD_FLAT), charges(0), value(0), spellId(0), ownerAura(_ownerAura) {}
    SpellModOp   op   : 8;
    SpellModType type : 8;
    int16 charges     : 16;
    float value;
    flag128 mask;
    uint32 spellId;
    Aura* const ownerAura;
};

enum PlayerCurrencyFlag
{
    PLAYERCURRENCY_FLAG_NONE                = 0x0,
    PLAYERCURRENCY_FLAG_UNK1                = 0x1,  // unused?
    PLAYERCURRENCY_FLAG_UNK2                = 0x2,  // unused?
    PLAYERCURRENCY_FLAG_SHOW_IN_BACKPACK    = 0x4,
    PLAYERCURRENCY_FLAG_UNUSED              = 0x8,

    PLAYERCURRENCY_MASK_USED_BY_CLIENT =
        PLAYERCURRENCY_FLAG_SHOW_IN_BACKPACK |
        PLAYERCURRENCY_FLAG_UNUSED,
};

enum PlayerCurrencyState
{
   PLAYERCURRENCY_UNCHANGED = 0,
   PLAYERCURRENCY_CHANGED   = 1,
   PLAYERCURRENCY_NEW       = 2,
   PLAYERCURRENCY_REMOVED   = 3     //not removed just set count == 0
};

struct PlayerCurrency
{
    PlayerCurrencyState state;
    uint32 totalCount;
    uint32 weekCount;
    uint32 seasonTotal;
    uint8 flags;
    uint32 curentCap;
    CurrencyTypesEntry const * currencyEntry;
};



struct SpellInQueue
{
    SpellInQueue();

    void Clear();

    uint32 GCDEnd;
    uint16 RecoveryCategory;
    WorldPackets::Spells::SpellCastRequest* CastData;
};

enum SceneEventStatus
{
    SCENE_NONE          = 0,
    SCENE_LAUNCH        = 1,
    SCENE_TRIGER        = 2,
    SCENE_COMPLETE      = 3,
};

typedef std::unordered_map<uint32, PlayerSpellState> PlayerTalentMap;
typedef std::unordered_map<uint32, PlayerSpellState> PlayerPvPTalentMap;
typedef cds::container::FeldmanHashMap< cds::gc::HP, uint32, PlayerSpell, uint32Traits > PlayerSpellMap;
typedef std::list<SpellModifier*> SpellModList;
typedef std::list<uint32> ItemSpellList;
typedef std::list<uint32> ExcludeCasterSpellList;
typedef std::list<uint32> CasterAuraStateSpellList;
typedef std::unordered_map<uint32, PlayerCurrency> PlayerCurrenciesMap;

struct SpellCooldown
{
    SpellCooldown(double _end, uint32 _itemid) : end(_end), itemid(_itemid) {}

    double end;
    uint32 itemid;
    float rate = 1.0f;
};

struct RPPMSpellCooldown
{
    uint32 spellId;
    ObjectGuid itemGUID;
    double end_time;
    double lastSuccessfulProc;
    double lastChanceToProc;
};

struct SpellChargeData
{
    int8 charges;
    int8 maxCharges;

    SpellCategoryEntry const* categoryEntry;
    uint32 timer;
    uint32 chargeRegenTime;
    float speed = 1.0f;
    SpellInfo const* spellInfo = nullptr;
};

typedef std::vector<RPPMSpellCooldown> RPPMSpellCooldowns;
typedef std::map<uint32, SpellCooldown> SpellCooldowns;
typedef std::map<uint32 /*categoryId*/, SpellChargeData> SpellChargeDataMap;
typedef std::unordered_map<uint32 /*instanceId*/, time_t/*releaseTime*/> InstanceTimeMap;

enum TrainerSpellState
{
    TRAINER_SPELL_GRAY  = 0,
    TRAINER_SPELL_GREEN = 1,
    TRAINER_SPELL_RED   = 2,
    TRAINER_SPELL_GREEN_DISABLED = 10                       // custom value, not send to client: formally green but learn not allowed
};

enum ActionButtonUpdateState : uint8
{
    ACTIONBUTTON_UNCHANGED = 0,
    ACTIONBUTTON_CHANGED   = 1,
    ACTIONBUTTON_NEW       = 2,
    ACTIONBUTTON_DELETED   = 3
};

enum ActionButtonType
{
    ACTION_BUTTON_SPELL     = 0x00,
    ACTION_BUTTON_C         = 0x01,                         // click?
    ACTION_BUTTON_EQSET     = 0x20,
    ACTION_BUTTON_SUB_BUTTON= 0x30,
    ACTION_BUTTON_MACRO     = 0x40,
    ACTION_BUTTON_PET       = 0x50,
    ACTION_BUTTON_CMACRO    = ACTION_BUTTON_C | ACTION_BUTTON_MACRO,
    ACTION_BUTTON_MOUNT     = 0x60,
    ACTION_BUTTON_ITEM      = 0x80
};

#define ACTION_BUTTON_ACTION(X) (uint32(uint64(X) & 0xFFFFFFFF))
#define ACTION_BUTTON_TYPE(X)   (uint8(uint64(X) >> 56))
#define MAX_ACTION_BUTTON_ACTION_VALUE (0xFFFFFFFF)

struct ActionButton
{
    ActionButton(uint32 action, uint8 type, ActionButtonUpdateState state);
    ActionButtonType GetType() const;
    uint32 GetAction() const;
    void SetActionAndType(uint32 action, ActionButtonType type);

    uint64 packedData;
    uint32 uAction;
    ActionButtonUpdateState uState;
    uint8 uType;
};

#define  MAX_ACTION_BUTTONS 132                             //checked in 5.0.5

typedef std::map<uint16, ActionButton> ActionButtonList;

struct PlayerCreateInfoItem
{
    PlayerCreateInfoItem(uint32 id, uint32 amount, std::vector<uint32> bonusListID) : item_bonusListIDs(bonusListID), item_id(id), item_amount(amount) {}

    std::vector<uint32> item_bonusListIDs;
    uint32 item_id;
    uint32 item_amount;
};

typedef std::list<PlayerCreateInfoItem> PlayerCreateInfoItems;

struct PlayerLevelInfo
{
    PlayerLevelInfo()
    {
        for (uint8 i = 0; i < MAX_STATS; ++i)
            stats[i] = 0;
    }

    uint16 stats[MAX_STATS];
};

typedef std::list<uint32> PlayerCreateInfoSpells;

struct PlayerCreateInfoAction
{
    PlayerCreateInfoAction() : button(0), type(0), action(0) {}
    PlayerCreateInfoAction(uint8 _button, uint32 _action, uint8 _type) : button(_button), type(_type), action(_action) {}

    uint8 button;
    uint8 type;
    uint32 action;
};

typedef std::list<PlayerCreateInfoAction> PlayerCreateInfoActions;
typedef std::list<SkillRaceClassInfoEntry const*> PlayerCreateInfoSkills;

typedef std::list<uint32> PlayerCreateInfoQuests;

struct PlayerInfo
{
    PlayerInfo() : mapId(0), areaId(0), positionX(0.0f), positionY(0.0f), positionZ(0.0f), orientation(0.0f), displayId_m(0), displayId_f(0), levelInfo(nullptr)
    { }

    uint32 mapId;
    uint32 areaId;
    uint32 displayId_m;
    uint32 displayId_f;
    float positionX;
    float positionY;
    float positionZ;
    float orientation;
    PlayerCreateInfoItems item;
    PlayerCreateInfoSpells spell;
    PlayerCreateInfoActions action;
    PlayerCreateInfoSkills skills;
    PlayerCreateInfoQuests quests;
    PlayerLevelInfo* levelInfo;                             //[level-1] 0..MaxPlayerLevel-1
};

struct PvPInfo
{
    PvPInfo() : inHostileArea(false), inNoPvPArea(false), inFFAPvPArea(false), endTimer(0) {}

    bool inHostileArea;
    bool inNoPvPArea;
    bool inFFAPvPArea;
    time_t endTimer;
};

enum DuelState
{
    DUEL_NOT_STARTED = 0,
    DUEL_COUNTDOWN   = 1,
    DUEL_STARTED     = 2,
    DUEL_COMPLETED   = 3,
};

struct DuelInfo
{
    DuelInfo() : initiator(ObjectGuid()), opponent(ObjectGuid()), arbiter(ObjectGuid()), state(DUEL_NOT_STARTED), outOfBoundTimer(0), countdownTimer(3000), isMounted(false) {}

    ObjectGuid initiator;
    ObjectGuid opponent;
    ObjectGuid arbiter;
    DuelState state;
    uint32 countdownTimer;
    uint32 outOfBoundTimer;
    bool isMounted;
};

struct Areas
{
    uint32 areaID;
    uint32 areaFlag;
    float x1;
    float x2;
    float y1;
    float y2;
};

#define MAX_RUNES 7
#define MAX_RECHARGING_RUNES 3

enum RuneCooldowns
{
    RUNE_BASE_COOLDOWN  = 10000,
    RUNE_MISS_COOLDOWN  = 1500,     // cooldown applied on runes when the spell misses
};

struct Runes
{
    std::deque<uint8> CooldownOrder;
    uint32 Cooldown[MAX_RUNES];
    float CooldownCoef;
    uint8 RuneState;

    void SetRuneState(uint8 index, bool set = true);
};

struct EnchantDuration
{
    EnchantDuration() : item(nullptr), slot(MAX_ENCHANTMENT_SLOT), leftduration(0) {};
    EnchantDuration(Item* _item, EnchantmentSlot _slot, uint32 _leftduration) : item(_item), slot(_slot),
        leftduration(_leftduration){ ASSERT(item); };

    Item* item;
    EnchantmentSlot slot;
    uint32 leftduration;
};

typedef std::list<EnchantDuration> EnchantDurationList;
typedef std::list<Item*> ItemDurationList;

enum DrunkenState
{
    DRUNKEN_SOBER   = 0,
    DRUNKEN_TIPSY   = 1,
    DRUNKEN_DRUNK   = 2,
    DRUNKEN_SMASHED = 3,

    MAX_DRUNKEN
};

#define KNOWN_TITLES_SIZE   6
constexpr uint16  MAX_TITLE_INDEX = (KNOWN_TITLES_SIZE * 64);         // 3 uint64 fields

enum PlayerFieldByte2Flags
{
    PLAYER_FIELD_BYTE2_NONE                 = 0x00,
    PLAYER_FIELD_BYTE2_STEALTH              = 0x20,
    PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW    = 0x40
};

enum PlayerBytes1Offsets
{
    PLAYER_BYTES_1_SKIN_ID         = 0,
    PLAYER_BYTES_1_FACE_ID         = 1,
    PLAYER_BYTES_1_HAIR_STYLE_ID   = 2,
    PLAYER_BYTES_1_HAIR_COLOR_ID   = 3
};

enum PlayerBytes2Offsets
{
    PLAYER_BYTES_2_OFFSET_CUSTOM_DISPLAY_OPTION = 0,
    PLAYER_BYTES_2_TATOO                    = 0,
    PLAYER_BYTES_2_HORN                     = 1,
    PLAYER_BYTES_2_OFFSET_BLIND_FOLD        = 2,
    PLAYER_BYTES_2_OFFSET_FACIAL_STYLE      = 3
};

enum PlayerBytes3Offsets
{
    PLAYER_BYTES_3_OFFSET_PARTY_TYPE        = 0,
    PLAYER_BYTES_3_OFFSET_BANK_BAG_SLOTS    = 1,
    PLAYER_BYTES_3_OFFSET_GENDER            = 2,
    PLAYER_BYTES_3_OFFSET_INEBRIATION       = 3
};

enum PlayerBytes4Offsets
{
    PLAYER_BYTES_4_RAF_GRANTABLE_LEVEL   = 0,
    PLAYER_BYTES_4_ACTION_BAR_TOGGLES    = 1,
    PLAYER_BYTES_4_LIFETIME_MAX_PVP_RANK = 2,
    PLAYER_BYTES_4_NUM_RESPECS           = 3
};

enum PlayerBytes5Offsets
{
    PLAYER_BYTES_5_OVERRIDE_SPELLS_ID  = 0     // uint16!
};

enum PlayerBytes6Offsets
{
    PLAYER_BYTES_6_OFFSET_PVP_TITLE     = 0,
    PLAYER_BYTES_6_OFFSET_ARENA_FACTION = 1
};

enum PlayerFieldBytes2Offsets
{
    PLAYER_BYTES_5_IGNORE_POWER_REGEN_PREDICTION_MASK  = 0,
    PLAYER_BYTES_5_AURA_VISION                         = 1,
    PLAYER_BYTES_5_NUM_BACKPACK_SLOTS                  = 2
};

static_assert((PLAYER_BYTES_5_OVERRIDE_SPELLS_ID & 1) == 0, "PLAYER_BYTES_5_OVERRIDE_SPELLS_ID must be aligned to 2 byte boundary");

#define PLAYER_BYTES_2_OVERRIDE_SPELLS_UINT16_OFFSET (PLAYER_BYTES_5_OVERRIDE_SPELLS_ID / 2)

enum PlayerAvgItemLevelOffsets
{
    TotalAvgItemLevel       = 0,
    EquippedAvgItemLevel    = 1,
    NonPvPAvgItemLevel      = 2,
    PvPAvgItemLevel         = 3,

    MaxAvgItemLevel
};

enum MirrorTimerType : int32
{
    DISABLED_MIRROR_TIMER   = -1,
    FATIGUE_TIMER           = 0,
    BREATH_TIMER            = 1,
    FIRE_TIMER              = 2, // feign death

    MAX_TIMERS
};

enum PlayerRestInfoOffsets
{
    REST_STATE_XP       = 0,
    REST_RESTED_XP      = 1,
    REST_STATE_HONOR    = 2,
    REST_RESTED_HONOR   = 3,
};

#define PLAYER_CUSTOM_DISPLAY_SIZE 3

// 2^n values
enum PlayerExtraFlags
{
    // gm abilities
    PLAYER_EXTRA_GM_ON              = 0x0001,
    PLAYER_EXTRA_ACCEPT_WHISPERS    = 0x0004,
    PLAYER_EXTRA_TAXICHEAT          = 0x0008,
    PLAYER_EXTRA_GM_INVISIBLE       = 0x0010,
    PLAYER_EXTRA_GM_CHAT            = 0x0020,               // Show GM badge in chat messages
    PLAYER_EXTRA_INVISIBLE_STATUS   = 0x0040,

    // other states
    PLAYER_EXTRA_PVP_DEATH          = 0x0100                // store PvP death status until corpse creating.
};

enum AtLoginFlags : uint16
{
    AT_LOGIN_NONE                   = 0x000,
    AT_LOGIN_RENAME                 = 0x001,
    AT_LOGIN_RESET_SPELLS           = 0x002,
    AT_LOGIN_RESET_TALENTS          = 0x004,
    AT_LOGIN_CUSTOMIZE              = 0x008,
    AT_LOGIN_RESET_PET_TALENTS      = 0x010,
    AT_LOGIN_FIRST                  = 0x020,
    AT_LOGIN_CHANGE_FACTION         = 0x040,
    AT_LOGIN_CHANGE_RACE            = 0x080,
    AT_LOGIN_UNLOCK                 = 0x100,
    AT_LOGIN_LOCKED_FOR_TRANSFER    = 0x200,
};

typedef std::vector<QuestStatusData*>* QuestStatusVector;
typedef std::map<uint32, QuestStatusData> QuestStatusMap;
typedef std::set<uint32> RewardedQuestSet;

enum QuestSaveType
{
    QUEST_DEFAULT_SAVE_TYPE = 0,
    QUEST_DELETE_SAVE_TYPE,
    QUEST_FORCE_DELETE_SAVE_TYPE
};

//               quest,  keep
typedef std::map<uint32, QuestSaveType> QuestStatusSaveMap;

enum QuestSlotOffsets
{
    QUEST_ID_OFFSET     = 0,
    QUEST_STATE_OFFSET  = 1,
    QUEST_COUNTS_OFFSET = 2,
    QUEST_TIME_OFFSET   = 15,

    MAX_QUEST_OFFSET
};

#define MAX_QUEST_COUNTS 24
#define QUESTS_COMPLETED_BITS_SIZE 1750 // Size of client completed quests bit map

enum QuestSlotStateMask
{
    QUEST_STATE_NONE     = 0x0000,
    QUEST_STATE_COMPLETE = 0x0001,
    QUEST_STATE_FAIL     = 0x0002
};

enum SkillUpdateState
{
    SKILL_UNCHANGED     = 0,
    SKILL_CHANGED       = 1,
    SKILL_NEW           = 2,
    SKILL_DELETED       = 3
};

struct SkillStatusData
{
    SkillStatusData(uint8 _pos, SkillUpdateState _uState) : pos(_pos), uState(_uState)
    {
    }
    uint8 pos;
    SkillUpdateState uState;
};

typedef std::unordered_map<uint32, SkillStatusData> SkillStatusMap;
typedef std::vector<SkillStatusData*>* SkillStatusVector;
typedef std::vector<int16> SkillSpellCount;

class Quest;
class Spell;
class Item;
class WorldSession;

enum PlayerSlots
{
    // first slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_START           = 0,
    // last+1 slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_END             = 195,
    PLAYER_SLOTS_COUNT          = (PLAYER_SLOT_END - PLAYER_SLOT_START)
};

enum ChildEquipmentSlots
{
    CHILD_EQUIPMENT_SLOT_START   = 192,
    CHILD_EQUIPMENT_SLOT_END     = 195,
};

#define INVENTORY_SLOT_BAG_0    255
#define INVENTORY_DEFAULT_SIZE  20

enum EquipmentSlots                                         // 19 slots
{
    EQUIPMENT_SLOT_START        = 0,
    EQUIPMENT_SLOT_HEAD         = 0,
    EQUIPMENT_SLOT_NECK         = 1,
    EQUIPMENT_SLOT_SHOULDERS    = 2,
    EQUIPMENT_SLOT_BODY         = 3,
    EQUIPMENT_SLOT_CHEST        = 4,
    EQUIPMENT_SLOT_WAIST        = 5,
    EQUIPMENT_SLOT_LEGS         = 6,
    EQUIPMENT_SLOT_FEET         = 7,
    EQUIPMENT_SLOT_WRISTS       = 8,
    EQUIPMENT_SLOT_HANDS        = 9,
    EQUIPMENT_SLOT_FINGER1      = 10,
    EQUIPMENT_SLOT_FINGER2      = 11,
    EQUIPMENT_SLOT_TRINKET1     = 12,
    EQUIPMENT_SLOT_TRINKET2     = 13,
    EQUIPMENT_SLOT_BACK         = 14,
    EQUIPMENT_SLOT_MAINHAND     = 15,
    EQUIPMENT_SLOT_OFFHAND      = 16,
    EQUIPMENT_SLOT_RANGED       = 17,
    EQUIPMENT_SLOT_TABARD       = 18,
    EQUIPMENT_SLOT_END          = 19
};

#define VISIBLE_ITEM_ENTRY_OFFSET 0
#define VISIBLE_ITEM_ENCHANTMENT_OFFSET 1

enum InventorySlots : uint8                                 // 4 slots
{
    INVENTORY_SLOT_BAG_START    = 19,
    INVENTORY_SLOT_BAG_END      = 23
};

enum InventoryPackSlots                                     // 16 slots
{
    INVENTORY_SLOT_ITEM_START   = 23,
    INVENTORY_SLOT_ITEM_END     = 47
};

enum BankItemSlots                                          // 28 slots
{
    BANK_SLOT_ITEM_START        = 47,
    BANK_SLOT_ITEM_END          = 75
};

enum BankBagSlots                                           // 7 slots
{
    BANK_SLOT_BAG_START         = 75,
    BANK_SLOT_BAG_END           = 82
};

enum BuyBackSlots                                           // 12 slots
{
    // stored in m_buybackitems
    BUYBACK_SLOT_START          = 82,
    BUYBACK_SLOT_END            = 94
};

enum ReagentSlots
{
    REAGENT_SLOT_START          = 94,
    REAGENT_SLOT_END            = 192,
};

struct ItemPosCount
{
    ItemPosCount(uint16 _pos, uint32 _count) : pos(_pos), count(_count) {}
    bool isContainedIn(std::vector<ItemPosCount> const& vec) const;
    uint16 pos;
    uint32 count;
};
typedef std::vector<ItemPosCount> ItemPosCountVec;

enum TransferAbortReason
{
    TRANSFER_ABORT_ERROR                            = 1,
    TRANSFER_ABORT_MAX_PLAYERS                      = 2,      // Transfer Aborted: instance is full
    TRANSFER_ABORT_TOO_MANY_INSTANCES               = 4,      // You have entered too many instances recently.
    TRANSFER_ABORT_ZONE_IN_COMBAT                   = 6,      // Unable to zone in while an encounter is in progress.
    TRANSFER_ABORT_INSUF_EXPAN_LVL                  = 7,      // You must have <TBC, WotLK> expansion installed to access this area.
    TRANSFER_ABORT_DIFFICULTY                       = 8,      // <Normal, Heroic, Epic> difficulty mode is not available for %s.
    TRANSFER_ABORT_UNIQUE_MESSAGE                   = 9,      // Until you've escaped TLK's grasp, you cannot leave this place!
    TRANSFER_ABORT_TOO_MANY_REALM_INSTANCES         = 10,     // Additional instances cannot be launched, please try again later.
    TRANSFER_ABORT_NEED_GROUP                       = 11,     // Transfer Aborted: you must be in a raid group to enter this instance
    TRANSFER_ABORT_NOT_FOUND                        = 14,     // Transfer Aborted: instance not found
    TRANSFER_ABORT_REALM_ONLY                       = 15,     // All players in the party must be from the same realm to enter %s.
    TRANSFER_ABORT_MAP_NOT_ALLOWED                  = 16,     // Map can't be entered at this time.
    TRANSFER_ABORT_LOCKED_TO_DIFFERENT_INSTANCE     = 18,     // You are already locked to %s
    TRANSFER_ABORT_ALREADY_COMPLETED_ENCOUNTER      = 19,     // You are ineligible to participate in at least one encounter in this instance because you are already locked to an instance in which it has been defeated.
    TRANSFER_ABORT_XREALM_ZONE_DOWN                 = 24,     // Transfer Aborted: cross-realm zone is down
    TRANSFER_ABORT_SOLO_PLAYER_SWITCH_DIFFICULTY    = 26,     // This instance is already in progress. You may only switch difficulties from inside the instance.
};

enum InstanceResetWarningType
{
    RAID_INSTANCE_WARNING_HOURS     = 1,                    // WARNING! %s is scheduled to reset in %d hour(s).
    RAID_INSTANCE_WARNING_MIN       = 2,                    // WARNING! %s is scheduled to reset in %d minute(s)!
    RAID_INSTANCE_WARNING_MIN_SOON  = 3,                    // WARNING! %s is scheduled to reset in %d minute(s). Please exit the zone or you will be returned to your bind location!
    RAID_INSTANCE_WELCOME           = 4,                    // Welcome to %s. This raid instance is scheduled to reset in %s.
    RAID_INSTANCE_EXPIRED           = 5
};

// PLAYER_FIELD_PVP_INFO offsets
/// @Posible that where is only 3 arena slots with 8 fields
enum BracketInfoType
{
    BRACKET_SEASON_GAMES         = 0,
    BRACKET_SEASON_WIN           = 1,
    BRACKET_WEEK_GAMES           = 2,
    BRACKET_WEEK_WIN             = 3,
    BRACKET_WEEK_BEST            = 4,                       // Best rating on this week
    BRACKET_BEST                 = 5,                       // Best rating on this season
    BRACKET_WEEK_BEST_LAST       = 6,                       // Best rating on last week
    BRACKET_END                  = 7
};

class InstanceSave;

enum KillStates
{
    KILL_UNCHANGED  = 0,
    KILL_CHANGED    = 1,
    KILL_NEW        = 2
    // no removed state, all kills are flushed at midnight
};

struct KillInfo
{
    uint8 count;
    KillStates state;
    KillInfo() : count(0), state(KILL_NEW) {}
};

typedef std::map<uint64, KillInfo> KillInfoMap;

struct HonorInfo
{
    HonorInfo() : CurrentHonorAtLevel(0), NextHonorAtLevel(0), PrestigeLevel(0), HonorLevel(0), NextHonorLevel(1) { }

    bool IncreaseHonorLevel()
    {
        if (HonorLevel >= MaxHonorLevel)
            return false;

        HonorLevel++;
        NextHonorLevel = HonorLevel < MaxHonorLevel ? HonorLevel + 1 : MaxHonorLevel;

        return true;
    }

    bool IncreasePrestigeLevel()
    {
        if (PrestigeLevel >= sWorld->getIntConfig(CONFIG_MAX_PRESTIGE_LEVEL))
            return false;

        PrestigeLevel++;
        return true;
    }

    uint16 CurrentHonorAtLevel;
    uint16 NextHonorAtLevel;
    uint8 PrestigeLevel;
    uint8 HonorLevel;
    uint8 NextHonorLevel;

    static uint8 const MaxHonorLevel = 50;
};

struct ChallengeKeyInfo
{
    ChallengeKeyInfo() : InstanceID(0), timeReset(0), ID(0), Level(2), Affix(0), Affix1(0), Affix2(0), KeyIsCharded(1), needSave(false), needUpdate(false) { }

    bool IsActive() { return ID != 0; }

    MapChallengeModeEntry const* challengeEntry = nullptr;
    uint32 InstanceID;
    uint32 timeReset;
    uint16 ID;
    uint8 Level;
    uint8 Affix;
    uint8 Affix1;
    uint8 Affix2;
    uint8 KeyIsCharded;
    bool needSave;
    bool needUpdate;
};

enum RestType
{
    REST_TYPE_NO        = 0,
    REST_TYPE_IN_TAVERN = 1,
    REST_TYPE_IN_CITY   = 2,
    REST_TYPE_IN_FACTION_AREA = 3
};

enum DuelCompleteType
{
    DUEL_INTERRUPTED = 0,
    DUEL_FLED        = 1,
    DUEL_FINISHED    = 2
};

enum TeleportToOptions
{
    TELE_TO_GM_MODE             = 0x01,
    TELE_TO_NOT_LEAVE_TRANSPORT = 0x02,
    TELE_TO_NOT_LEAVE_COMBAT    = 0x04,
    TELE_TO_NOT_UNSUMMON_PET    = 0x08,
    TELE_TO_SPELL               = 0x10,
    TELE_TO_SEAMLESS            = 0x20,
    TELE_TO_NOT_DISABLE_MOVE    = 0x40,
    TELE_TO_ZONE_MAP            = 0x80,
};

/// Type of environmental damages
enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING  = 1,
    DAMAGE_FALL      = 2,
    DAMAGE_LAVA      = 3,
    DAMAGE_SLIME     = 4,
    DAMAGE_FIRE      = 5,
    DAMAGE_FALL_TO_VOID = 6                                 // custom case for fall without durability loss
};

enum AttackSwingReason
{
    ATTACK_SWING_ERROR_CANT_ATTACK      = 0,
    ATTACK_SWING_ERROR_BAD_FACING       = 1,
    ATTACK_SWING_ERROR_NOT_IN_RANGE     = 2,
    ATTACK_SWING_ERROR_DEAD_TARGET      = 3,
};

enum PlayerChatTag
{
    CHAT_TAG_NONE       = 0x00,
    CHAT_TAG_AFK        = 0x01,
    CHAT_TAG_DND        = 0x02,
    CHAT_TAG_GM         = 0x04,
    CHAT_TAG_COM        = 0x08, // Commentator
    CHAT_TAG_DEV        = 0x10,
    CHAT_TAG_BOSS_SOUND = 0x20, // Plays "RaidBossEmoteWarning" sound on raid boss emote/whisper
    CHAT_TAG_MOBILE     = 0x40
};

enum PlayedTimeIndex
{
    PLAYED_TIME_TOTAL = 0,
    PLAYED_TIME_LEVEL = 1,

    MAX_PLAYED_TIME_INDEX
};

// used at player loading query list preparing, and later result selection
enum PlayerLoginQueryIndex
{
    PLAYER_LOGIN_QUERY_LOADFROM                     = 0,
    PLAYER_LOGIN_QUERY_LOADGROUP                    = 1,
    PLAYER_LOGIN_QUERY_LOADBOUNDINSTANCES           = 2,
    PLAYER_LOGIN_QUERY_LOADAURAS                    = 3,
    PLAYER_LOGIN_QUERY_LOADAURAS_EFFECTS            = 4,
    PLAYER_LOGIN_QUERY_LOADSPELLS                   = 5,
    PLAYER_LOGIN_QUERY_LOADQUESTSTATUS              = 6,
    PLAYER_LOGIN_QUERY_LOADDAILYQUESTSTATUS         = 7,
    PLAYER_LOGIN_QUERY_LOADREPUTATION               = 8,
    PLAYER_LOGIN_QUERY_LOADINVENTORY                = 9,
    PLAYER_LOGIN_QUERY_LOADACTIONS                  = 10,
    PLAYER_LOGIN_QUERY_LOADMAILCOUNT                = 11,
    PLAYER_LOGIN_QUERY_LOADMAILDATE                 = 12,
    PLAYER_LOGIN_QUERY_LOADSOCIALLIST               = 13,
    PLAYER_LOGIN_QUERY_LOADHOMEBIND                 = 14,
    PLAYER_LOGIN_QUERY_LOADSPELLCOOLDOWNS           = 15,
    PLAYER_LOGIN_QUERY_LOADDECLINEDNAMES            = 16,
    PLAYER_LOGIN_QUERY_LOADGUILD                    = 17,
    PLAYER_LOGIN_QUERY_LOADACHIEVEMENTS             = 19,
    PLAYER_LOGIN_QUERY_LOADACCOUNTACHIEVEMENTS      = 20,
    PLAYER_LOGIN_QUERY_LOADCRITERIAPROGRESS         = 21,
    PLAYER_LOGIN_QUERY_LOADACCOUNTCRITERIAPROGRESS  = 22,
    PLAYER_LOGIN_QUERY_LOADEQUIPMENTSETS            = 23,
    PLAYER_LOGIN_QUERY_LOADBGDATA                   = 24,
    PLAYER_LOGIN_QUERY_LOAD_GLYPHS                   = 25,
    PLAYER_LOGIN_QUERY_LOADTALENTS                  = 26,
    PLAYER_LOGIN_QUERY_LOADACCOUNTDATA              = 27,
    PLAYER_LOGIN_QUERY_LOADSKILLS                   = 28,
    PLAYER_LOGIN_QUERY_LOADWEEKLYQUESTSTATUS        = 29,
    PLAYER_LOGIN_QUERY_LOADRANDOMBG                 = 30,
    PLAYER_LOGIN_QUERY_LOADBANNED                   = 31,
    PLAYER_LOGIN_QUERY_LOADQUESTSTATUSREW           = 32,
    PLAYER_LOGIN_QUERY_LOADINSTANCELOCKTIMES        = 33,
    PLAYER_LOGIN_QUERY_LOADSEASONALQUESTSTATUS      = 34,
    PLAYER_LOGIN_QUERY_LOADVOIDSTORAGE              = 35,
    PLAYER_LOGIN_QUERY_LOADCURRENCY                 = 36,
    PLAYER_LOGIN_QUERY_LOAD_CUF_PROFILES            = 37,
    PLAYER_LOGIN_QUERY_LOADARCHAELOGY               = 40,
    PLAYER_LOGIN_QUERY_LOAD_ARCHAEOLOGY_FINDS       = 41,
    PLAYER_LOGIN_QUERY_LOAD_PERSONAL_RATE           = 42,
    PLAYER_LOGIN_QUERY_HONOR                        = 43,
    PLAYER_LOGIN_QUERY_LOAD_VISUAL                  = 44,
    PLAYER_LOGIN_QUERY_LOAD_LOOTCOOLDOWN            = 45,
    PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS_OBJECTIVES = 46,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_BLUEPRINTS,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_BUILDINGS,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_FOLLOWERS,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_FOLLOWER_ABILITIES,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_MISSIONS,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_SHIPMENTS,
    PLAYER_LOGIN_QUERY_LOAD_GARRISON_TALENTS,
    PLAYER_LOGIN_QUERY_LOAD_TOYS,
    PLAYER_LOGIN_QUERY_LOAD_HEIRLOOMS,
    PLAYER_LOGIN_QUERY_LOAD_TRANSMOGS,
    PLAYER_LOGIN_QUERY_LOAD_ITEM_FAVORITE_APPEARANCES,
    PLAYER_LOGIN_QUERY_LOAD_ACCOUNT_MOUNTS,
    PLAYER_LOGIN_QUERY_HONOR_INFO,
    PLAYER_LOGIN_QUERY_LOAD_ARTIFACTS,
    PLAYER_LOGIN_QUERY_LOAD_TRANSMOG_OUTFITS,
    PLAYER_LOGIN_QUERY_LOAD_PVP_TALENTS,
    PLAYER_LOGIN_QUERY_ADVENTURE_QUEST,
    PLAYER_LOGIN_QUERY_PETS,
    PLAYER_LOGIN_QUERY_LOAD_VOIDSTORAGE_ITEM,
    PLAYER_LOGIN_QUERY_LOADWORLDQUESTSTATUS,
    PLAYER_LOGIN_QUERY_BATTLE_PETS,
    PLAYER_LOGIN_QUERY_CHALLENGE_KEY,
    PLAYER_LOGIN_QUERY_LOAD_DEATHMATCH_STATS,
    PLAYER_LOGIN_QUERY_LOAD_DEATHMATCH_STORE,
    PLAYER_LOGIN_QUERY_LOAD_CHAT_LOGOS,
    PLAYER_LOGIN_QUERY_ACCOUNT_PROGRESS,
    PLAYER_LOGIN_QUERY_ARMY_TRAINING,
    PLAYER_LOGIN_QUERY_LOAD_LFGCOOLDOWN,
    PLAYER_LOGIN_QUERY_LOAD_KILL_CREATURE,
    PLAYER_LOGIN_QUERY_LOADNOTINVENTORY,
    PLAYER_LOGIN_QUERY_ACCOUNT_QUEST,

    MAX_PLAYER_LOGIN_QUERY
};

enum PlayerDelayedOperations
{
    DELAYED_SAVE_PLAYER         = 0x001,
    DELAYED_RESURRECT_PLAYER    = 0x002,
    DELAYED_SPELL_CAST_DESERTER = 0x004,
    DELAYED_BG_MOUNT_RESTORE    = 0x008,                     ///< Flag to restore mount state after teleport from BG
    DELAYED_BG_TAXI_RESTORE     = 0x010,                     ///< Flag to restore taxi state after teleport from BG
    DELAYED_BG_GROUP_RESTORE    = 0x020,                     ///< Flag to restore group state after teleport from BG
    DELAYED_UPDATE_AFTER_TO_BG  = 0x040,                     ///< Flag to update aura effect after teleport to BG
    DELAYED_PET_BATTLE_INITIAL  = 0x080,
    DELAYED_SPECTATOR_REMOVE    = 0x100,

    DELAYED_END
};

// Player summoning auto-decline time (in secs)
#define MAX_PLAYER_SUMMON_DELAY                   (2*MINUTE)
#define MAX_MONEY_AMOUNT               (UI64LIT(99999999999)) // TODO: Move this restriction to worldserver.conf, default to this value, hardcap at uint64.max

struct InstancePlayerBind
{
    InstanceSave* save;
    bool perm;
    /* permanent PlayerInstanceBinds are created in Raid/Heroic instances for players
       that aren't already permanently bound when they are inside when a boss is killed
       or when they enter an instance that the group leader is permanently bound to. */
    InstancePlayerBind() : save(nullptr), perm(false) {}
};

enum DungeonStatusFlag
{
    DUNGEON_STATUSFLAG_NORMAL = 0x01,
    DUNGEON_STATUSFLAG_HEROIC = 0x02,

    RAID_STATUSFLAG_10MAN_NORMAL = 0x01,
    RAID_STATUSFLAG_25MAN_NORMAL = 0x02,
    RAID_STATUSFLAG_10MAN_HEROIC = 0x04,
    RAID_STATUSFLAG_25MAN_HEROIC = 0x08
};

struct AccessRequirement
{
    int32  mapid;
    uint8  difficulty;
    uint16 dungeonId;
    uint8  levelMin;
    uint8  levelMax;
    uint32 item;
    uint32 item2;
    uint32 quest_A;
    uint32 quest_H;
    uint32 achievement;
    uint32 achievement_A;
    std::string questFailedText;
};

enum CharDeleteMethod
{
    CHAR_DELETE_REMOVE = 0,                      // Completely remove from the database
    CHAR_DELETE_UNLINK = 1                       // The character gets unlinked from the account,
                                                 // the name gets freed up and appears as deleted ingame
};

enum ReferAFriendError
{
    ERR_REFER_A_FRIEND_NONE                          = 0x00,
    ERR_REFER_A_FRIEND_NOT_REFERRED_BY               = 0x01,
    ERR_REFER_A_FRIEND_TARGET_TOO_HIGH               = 0x02,
    ERR_REFER_A_FRIEND_INSUFFICIENT_GRANTABLE_LEVELS = 0x03,
    ERR_REFER_A_FRIEND_TOO_FAR                       = 0x04,
    ERR_REFER_A_FRIEND_DIFFERENT_FACTION             = 0x05,
    ERR_REFER_A_FRIEND_NOT_NOW                       = 0x06,
    ERR_REFER_A_FRIEND_GRANT_LEVEL_MAX_I             = 0x07,
    ERR_REFER_A_FRIEND_NO_TARGET                     = 0x08,
    ERR_REFER_A_FRIEND_NOT_IN_GROUP                  = 0x09,
    ERR_REFER_A_FRIEND_SUMMON_LEVEL_MAX_I            = 0x0A,
    ERR_REFER_A_FRIEND_SUMMON_COOLDOWN               = 0x0B,
    ERR_REFER_A_FRIEND_INSUF_EXPAN_LVL               = 0x0C,
    ERR_REFER_A_FRIEND_SUMMON_OFFLINE_S              = 0x0D,
    ERR_REFER_A_FRIEND_NO_XREALM                     = 0x0E,
};

enum PlayerRestState
{
    REST_STATE_RESTED                                = 0x01,
    REST_STATE_NOT_RAF_LINKED                        = 0x02,
    REST_STATE_RAF_LINKED                            = 0x06
};

enum PlayerCommandStates
{
    CHEAT_NONE          = 0x00,
    CHEAT_GOD           = 0x01,
    CHEAT_CASTTIME      = 0x02,
    CHEAT_COOLDOWN      = 0x04,
    CHEAT_POWER         = 0x08,
    CHEAT_WATERWALK     = 0x10,
    CHEAT_ALL_SPELLS    = 0x20,
};

struct auraEffectData
{
    auraEffectData(uint8 slot, uint8 effect, int32 amount, int32 baseamount) : _slot(slot), _effect(effect), _amount(amount), _baseamount(baseamount)  {}
    uint8 _slot;
    uint8 _effect;
    int32 _amount;
    int32 _baseamount;
};

struct playerLootCooldown
{
    uint32 entry;
    uint8 type;
    uint8 difficultyID;
    uint32 respawnTime;
    bool state;
};

typedef std::map<uint32, std::map<uint8, playerLootCooldown>> PlayerLootCooldownMap;

enum PlayerLootCooldownType
{
    TYPE_GO          = 0,
    TYPE_CREATURE    = 1,
    TYPE_SPELL       = 2,
    TYPE_GUID        = 3,
    TYPE_ZONE        = 4,

    MAX_LOOT_COOLDOWN_TYPE
};

struct playerLfgCooldown
{
    uint32 dungeonId;
    uint32 respawnTime;
    bool state;
};

typedef std::map<uint32, playerLfgCooldown> PlayerLfgCooldownMap;

struct PetInfoData
{
    void UpdateData(Field* fields);

    void UpdateData(Pet* pet);

    uint32 id;
    uint32 entry;
    uint32 modelid;
    uint8 level;
    uint32 exp;
    ReactStates reactstate;
    std::string name;
    bool renamed;
    uint32 curhealth;
    uint32 curmana;
    std::string abdata;
    uint32 savetime;
    uint32 CreatedBySpell;
    PetType pet_type;
    uint16 specialization;
};

typedef std::unordered_map<uint64, PetInfoData> PetInfoDataMap;

class Player;

/// Holder for Battleground data
struct BGData
{
    BGData();

    std::set<uint32> BgAfkReporter;
    WorldLocation JoinPosition;                  ///< From where player entered BG
    time_t BgAfkReportedTimer;
    uint32 TaxiPath[MAX_TEAMS];
    uint32 BgInstanceID;                    ///< This variable is set to bg->m_InstanceID  when player is teleported to BG - (it is battleground's GUID)
    uint32 BgTeam;                          ///< What side the player will be added to
    uint32 MountSpellID;
    uint16 BgTypeID;
    uint16 LastActiveSpecID = 0;
    uint8 BgAfkReportedCount;

    void ClearTaxiPath() { TaxiPath[0] = TaxiPath[1] = 0; }
    bool HasTaxiPath() const { return TaxiPath[0] && TaxiPath[1]; }
};

struct VoidStorageItem
{
    VoidStorageItem();
    VoidStorageItem(uint64 id, Item* _item, bool _change);
    VoidStorageItem(uint64 id, uint32 entry, ObjectGuid const& creator, ItemRandomEnchantmentId randomPropertyId, uint32 suffixFactor, bool _change);
    VoidStorageItem(VoidStorageItem&& vsi, bool _change);

    ObjectGuid CreatorGuid;
    Item* item = nullptr;
    ItemRandomEnchantmentId ItemRandomPropertyId;
    uint64 ItemId = 0;
    uint32 ItemEntry = 0;
    uint32 ItemSuffixFactor = 0;
    uint32 ItemUpgradeId = 0;
    bool change = false;
    bool deleted = false;
};

enum WinTodayType
{
    WIN_TODAY_SKIRMISH,
    WIN_TODAY_RANDOM_BG,
    WIN_TODAY_ARENA_2v2,
    WIN_TODAY_ARENA_3v3,
    WIN_TODAY_RBG,
    WIN_TODAY_UNUSED_1, // 1v1
    WIN_TODAY_UNUSED_2, // Deathmatch
    WIN_TODAY_BRAWL,
    WIN_TODAY_BRAWL_ARENA,
    WIN_TODAY_MAX,
};

struct ResurrectionData
{
    SpellInfo const* ResSpell;
    ObjectGuid GUID;
    WorldLocation Location;
    uint64 Health;
    uint32 Mana;
    uint32 Aura;
};

static uint8 const DefaultTalentRowLevels[MAX_TALENT_TIERS] = { 15, 30, 45, 60, 75, 90, 100 };
static uint8 const DKTalentRowLevels[MAX_TALENT_TIERS] = { 56, 57, 58, 60, 75, 90, 100 };
static uint8 const DHTalentRowLevels[MAX_TALENT_TIERS] = { 99, 100, 102, 104, 106, 108, 110 };

struct SpecializationInfo
{
    SpecializationInfo() : ResetTalentsCost(0), ResetTalentsTime(0), PrimarySpecialization(0), ActiveGroup(0)
    { }

    PlayerTalentMap Talents[MAX_SPECIALIZATIONS];
    PlayerPvPTalentMap PvPTalents[MAX_SPECIALIZATIONS];
    std::vector<uint32> Glyphs[MAX_SPECIALIZATIONS];
    uint32 ResetTalentsCost;
    time_t ResetTalentsTime;
    uint32 PrimarySpecialization;
    uint8 ActiveGroup;

private:
    SpecializationInfo(SpecializationInfo const&) = delete;
    SpecializationInfo& operator=(SpecializationInfo const&) = delete;
};

typedef std::unordered_map<uint8, Bracket*> BracketList;
typedef std::vector<uint32/*PetEntry*/> PlayerPetSlotList;

#pragma pack(push, 1)
struct PlayerDynamicFieldSpellModByLabel
{
    uint32 Mod;
    float Value;
    uint32 Label;
};

struct PlayerDynamicFieldArenaCooldowns
{
    PlayerDynamicFieldArenaCooldowns(uint32 spellId, uint32 castTime, uint32 endTime);

    uint32 SpellId;
    uint32 Unk1;
    uint32 Unk2;
    uint32 Unk3;
    uint32 CastTime;
    uint32 Unk5;
    uint32 EndTime;
};

struct DigSiteInfo
{
    float x, y, z;
    float last_cast_dist;
    uint32 loot_GO_entry;
    uint32 race;
    uint32 pointCount[871];
    uint32 countProject;
    uint32 currentDigest;

    DigSiteInfo()
    {
        x = y = z = 0.0f;
        last_cast_dist = 0;
        loot_GO_entry = 0;
        race = 0;
        countProject = 0;
        currentDigest = 0;
        pointCount[0] = 0;
        pointCount[1] = 0;
        pointCount[530] = 0;
        pointCount[571] = 0;
        pointCount[870] = 0;
    }
};

struct DigSite
{
    uint8 count;
    uint16 site_id;
    uint32 find_id;
    float loot_x;
    float loot_y;

    void clear()
    {
        site_id = find_id = 0;
        loot_x = loot_y = 0.0f;
        count = 0;
    }

    bool empty() { return site_id == 0; }
};

struct CompletedProject
{
    CompletedProject() : entry(nullptr), count(1), date(time(nullptr)) { }
    CompletedProject(ResearchProjectEntry const* _entry) : entry(_entry), count(1), date(time(nullptr)) { }

    ResearchProjectEntry const* entry;
    uint32 count;
    uint32 date;
};

#pragma pack(pop)

typedef std::set<uint32> ResearchSiteSet;
typedef std::list<CompletedProject> CompletedProjectList;
typedef std::set<uint32> ResearchProjectSet;

#define MAX_RESEARCH_SITES 27
#define MAX_DIGSITE_FINDS 6

struct Visuals
{
    Visuals() : m_visHead(1), m_visShoulders(1), m_visChest(1), m_visWaist(1), m_visLegs(1), m_visFeet(1),
    m_visWrists(1), m_visHands(1), m_visBack(1), m_visMainhand(1), m_visOffhand(1), m_visRanged(1), m_visTabard(1), m_visShirt(1), m_altVis(0) { }

    uint32 m_visHead;
    uint32 m_visShoulders;
    uint32 m_visChest;
    uint32 m_visWaist;
    uint32 m_visLegs;
    uint32 m_visFeet;
    uint32 m_visWrists;
    uint32 m_visHands;
    uint32 m_visBack;
    uint32 m_visMainhand;
    uint32 m_visOffhand;
    uint32 m_visRanged;
    uint32 m_visTabard;
    uint32 m_visShirt;

    uint8 m_altVis;
};

typedef std::shared_ptr<CollectionMgr> CollectionPtr;
typedef std::map<ObjectGuid, std::shared_ptr<BattlePet>> BattlePetMap;

struct WargameRequest
{
    WargameRequest() : QueueID(0), CreationDate(0), TournamentRules(false) { }

    ObjectGuid OpposingPartyMemberGUID;
    uint64 QueueID;
    time_t CreationDate;
    bool TournamentRules;
};

struct WorldQuestInfo
{
    uint32 QuestID;
    uint32 resetTime;
    bool needSave = false;
};


struct DeathMatchScore
{
	DeathMatchScore() : kills(0), deaths(0), damage(0), rating(0), matches(0), needSave(false), totalKills(0), selectedMorph(0){};
    uint32 kills;
    uint32 deaths;
    uint64 damage;
    uint32 rating;
    uint32 matches;
    bool needSave;
    
    uint32 totalKills;
    std::set<uint32> buyedMorphs;
    uint32 selectedMorph;
    
    bool HasMorph(uint32 morph)
    {
        if (buyedMorphs.find(morph) != buyedMorphs.end())
            return true;
        
        return false;
    }
    
    bool BuyMorph(uint32 morph, uint32 cost)
    {
        if (totalKills < cost)
            return false;
        
        if (HasMorph(morph))
            return false;
        
        totalKills -= cost;
        
        buyedMorphs.insert(morph);
        
        return true;
    }
};

enum ArmyTrainingUnits
{
    ARMY_UNIT_COMMON,
    ARMY_UNIT_BERSERK,
    ARMY_UNIT_MANA,
    ARMY_UNIT_MAGE,
    
    ARMY_UNITS_MAX,
};

struct ArmyTrainingInfo
{
    std::list<ObjectGuid> currentUnits[ARMY_UNITS_MAX]{};
    uint32 justOpenedUnits[ARMY_UNITS_MAX - 1]{};

    uint8 moreHP{}, moreDMG{};
    bool moreFixate{}, moreBrave{};

    uint32 buyedNowUnits{};

    std::unordered_set<uint32> justFindedChest{};
    bool needSave = false;
};

typedef sf::safe_ptr<AchievementMgr<Player>> AchievementPtr;

class Player : public Unit, public GridObject<Player>
{
    friend class WorldSession;
    friend class BattlePayMgr;
    friend void Item::AddToUpdateQueueOf(Player* player);
    friend void Item::RemoveFromUpdateQueueOf(Player* player);
    public:
        explicit Player (WorldSession* session);
        ~Player();

        void Clear() override;
        void PrintPlayerSize();

        void CleanupsBeforeDelete(bool finalCleanup = true) override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool SafeTeleport(uint32 mapid, float x, float y, float z, float orientation, uint32 options = 0, uint32 spellID = 0);
        bool SafeTeleport(WorldLocation const& loc, uint32 options = 0);
        bool SafeTeleport(uint32 mapid, Position const* pos, uint32 options = 0, uint32 spellID = 0);
        bool TeleportTo(uint32 mapid, float x, float y, float z, float orientation, uint32 options = 0, uint32 spellID = 0);
        bool TeleportTo(uint32 mapid, Position const* pos, uint32 options = 0, uint32 spellID = 0);
        bool TeleportTo(WorldLocation const& loc, uint32 options = 0);
        bool TeleportTo(uint32 locEntry, uint32 options = 0);
        bool TeleportToBGEntryPoint();
        void TeleportToHomeBind();
        void TeleportToChallenge(uint32 mapid, float x, float y, float z, float orientation);

        void SetPing(uint32 val);
        uint32 GetPing() const;

        bool IsForbiddenMapForLevel(uint32 mapid, uint32 zone);
        bool IsLoXpMap(uint32 map);

        void SetSummonPoint(uint32 mapid, float x, float y, float z);
        void SummonIfPossible(bool agree);

        bool Create(ObjectGuid::LowType guidlow, WorldPackets::Character::CharacterCreateInfo* createInfo);

        void Update(uint32 time) override;

        SpellModList& GetSpellModList(SpellModOp op);

        void SetInWater(bool apply);

        bool IsInWater() const override { return m_isInWater; }
        bool IsUnderWater() const override;
        bool IsFalling() { return GetPositionZ() < m_lastFallZ; }

        bool IsInAreaTriggerRadius(AreaTriggerEntry const* trigger) const;
        bool IsInAreaTriggerRadius(uint32 areatriggerID) const;

        void SendInitialPacketsBeforeAddToMap(bool login = true);
        void SendInitialPacketsAfterAddToMap(bool login = true);
        void SendTransferAborted(uint32 mapid, TransferAbortReason reasonId, uint8 arg = 0);
        void SendInstanceResetWarning(uint32 mapid, Difficulty difficulty, uint32 time);

        bool CanInteractWithQuestGiver(Object* questGiver);
        Creature* GetNPCIfCanInteractWith(ObjectGuid guid, uint32 npcflagmask, uint32 npcflagmask2 = 0);
        GameObject* GetGameObjectIfCanInteractWith(ObjectGuid guid, GameobjectTypes type) const;

        bool ToggleAFK();
        bool ToggleDND();
        bool isAFK() const;
        bool isDND() const;
        uint8 GetChatTag() const;
        std::string afkMsg;
        std::string dndMsg;

        Visuals *m_vis;

        std::queue<WorldPacket> m_updatePacket;
        std::queue<WorldPacket> m_updatePacketInRange;
        sf::contention_free_shared_mutex< > i_updatePacketLock;
        sf::contention_free_shared_mutex< > i_updatePacketInRangeLock;
        void SendUpdateData();
        void AddUpdatePacket(WorldPacket const* packet);
        void AddUpdatePacketInRange(WorldPacket const* packet);

        void UpdateInstance(InstanceSave* save);

        uint32 GetBarberShopCost(BarberShopStyleEntry const* newHairStyle, uint8 newHairColor, BarberShopStyleEntry const* newFacialHair, BarberShopStyleEntry const* newSkin, BarberShopStyleEntry const* newFace, std::array<BarberShopStyleEntry const*, PLAYER_CUSTOM_DISPLAY_SIZE> const& newCustomDisplay) const;

        PlayerSocial *GetSocial() { return m_social; }

        bool CanContact();

        PlayerTaxi m_taxi;
        void InitTaxiNodesForLevel() { m_taxi.InitTaxiNodesForLevel(getRace(), getClass(), getLevel()); }
        bool ActivateTaxiPathTo(std::vector<uint32> const& nodes, Creature* npc = nullptr, uint32 spellid = 0, uint32 preferredMountDisplay = 0);
        bool ShortTaxiPathTo(TaxiNodesEntry const* from, TaxiNodesEntry const* to);
        bool ActivateTaxiPathTo(uint32 taxi_path_id, uint32 spellid = 0);
        void CleanupAfterTaxiFlight(bool zoneUpdate = true);
        void ContinueTaxiFlight();
                                                            // mount_id can be used in scripting calls
        bool isAcceptWhispers() const { return (m_ExtraFlags & PLAYER_EXTRA_ACCEPT_WHISPERS) != 0; }
        void SetAcceptWhispers(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_ACCEPT_WHISPERS; else m_ExtraFlags &= ~PLAYER_EXTRA_ACCEPT_WHISPERS; }
        bool isGameMaster() const { return (m_ExtraFlags & PLAYER_EXTRA_GM_ON) != 0; }
        void SetGameMaster(bool on);
        bool isGMChat() const { return (m_ExtraFlags & PLAYER_EXTRA_GM_CHAT) != 0; }
        void SetGMChat(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_GM_CHAT; else m_ExtraFlags &= ~PLAYER_EXTRA_GM_CHAT; }
        bool isTaxiCheater() const { return (m_ExtraFlags & PLAYER_EXTRA_TAXICHEAT) != 0; }
        void SetTaxiCheater(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_TAXICHEAT; else m_ExtraFlags &= ~PLAYER_EXTRA_TAXICHEAT; }
        bool isGMVisible() const { return !(m_ExtraFlags & PLAYER_EXTRA_GM_INVISIBLE); }
        void SetGMVisible(bool on);
        void SetPvPDeath(bool on) { if (on) m_ExtraFlags |= PLAYER_EXTRA_PVP_DEATH; else m_ExtraFlags &= ~PLAYER_EXTRA_PVP_DEATH; }
        bool HasPlayerExtraFlag(uint32 flag);
        void SetPlayerExtraFlag(uint32 flag, bool addFlag = true);
        bool InvisibleStatusRatingRequirements();
        bool InvisibleStatusMapRequirements();
        void SendInvisibleStatusMsg(uint8 msgId);

        bool HaveSpectators();
        void SendSpectatorAddonMsgToBG(SpectatorAddonMsg& msg);
        bool IsSpectateCanceled();
        void CancelSpectate();
        Unit* GetSpectateFrom();
        bool IsSpectator() const;
        bool IsSpectateRemoving() const;
        void SetSpectateRemoving(bool _value);
        void SetSpectate(bool on);

        void GiveXP(uint32 xp, Unit* victim, float groupRate = 1.0f);
        void SetXP(uint32 xp);
        void GiveGatheringXP();
        void GiveLevel(uint8 level);

        void InitStatsForLevel(bool reapplyMods = false);

        // .cheat command related
        bool GetCommandStatus(uint32 command) const { return (_activeCheats & command) != 0; }
        void SetCommandStatusOn(uint32 command) { _activeCheats |= command; }
        void SetCommandStatusOff(uint32 command) { _activeCheats &= ~command; }

        // Played Time Stuff
        time_t m_logintime;
        time_t m_createdtime;
        time_t m_Last_tick;
        uint32 m_Played_time[MAX_PLAYED_TIME_INDEX];
        uint32 GetTotalPlayedTime() { return m_Played_time[PLAYED_TIME_TOTAL]; }
        uint32 GetLevelPlayedTime() { return m_Played_time[PLAYED_TIME_LEVEL]; }
        uint32 m_account_time[MAX_PLAYED_TIME_INDEX];
        uint32 GetTotalAccountTime() { return m_account_time[PLAYED_TIME_TOTAL]; }
        uint32 GetLevelAccountTime() { return m_account_time[PLAYED_TIME_LEVEL]; }

        float CalculateLegendaryDropChance(float rate);
        float GetRateLegendaryDrop(bool isBoss, bool isRareOrGo, bool isOplote, bool isItemEmissary, int8 ExpansionID, uint8 DifficultyID);
        float m_killPoints;

        void setDeathState(DeathState s) override;

        void InnEnter(time_t time, uint32 mapid, float x, float y, float z);
        float GetRestBonus() const { return m_rest_bonus; }
        void SetRestBonus(float rest_bonus_new);

        RestType GetRestType() const { return rest_type; }
        void SetRestType(RestType n_r_type) { rest_type = n_r_type; }

        uint32 GetInnPosMapId() const { return inn_pos_mapid; }
        float GetInnPosX() const { return inn_pos_x; }
        float GetInnPosY() const { return inn_pos_y; }
        float GetInnPosZ() const { return inn_pos_z; }

        time_t GetTimeInnEnter() const { return time_inn_enter; }
        void UpdateInnerTime (time_t time) { time_inn_enter = time; }

        Pet* GetPet() const;
        Pet* SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 despwtime, uint32 spellId = 0);
        void RemovePet(Pet* pet, bool isDelete = false);

        PhaseMgr& GetPhaseMgr() { return phaseMgr; }

        void Say(std::string const& text, const uint32 language, bool isSpamm = false);
        void Yell(std::string const& text, const uint32 language, bool isSpamm = false);
        void TextEmote(std::string const& text, bool isSpamm = false);
        void Whisper(std::string const& text, const uint32 language, ObjectGuid receiver, bool isSpamm = false);
        void WhisperAddon(std::string const& text, std::string const& prefix, Player* receiver);
        void BossWhisper(std::string const& text, const uint32 language, ObjectGuid receiver);

        std::string const& getSelectedChatLogo() const {return selected_chat_logo;}
        void setSelectedChatLogo(std::string const& text);
       // bool BuyChatLogoByTokens(std::string const& logo, uint32 cost);
        bool hasChatLogo(std::string const& logo) const;
        bool BuyChatLogoByDeathMatch(std::string const& logo, uint32 cost);

        /*********************************************************/
        /***                    STORAGE SYSTEM                 ***/
        /*********************************************************/

        void SetVirtualItemSlot(uint8 i, Item* item);
        void SetSheath(SheathState sheathed) override;
        uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const;
        uint8 GetGuessedEquipSlot(ItemTemplate const* proto) const;
        uint32 GetItemCount(uint32 item, bool inBankAlso = false, Item* skipItem = nullptr) const;
        uint32 GetItemCountWithLimitCategory(uint32 limitCategory, Item* skipItem = nullptr) const;
        uint32 GetItemCountByQuality(uint8 Quality, int8 Expansion = -1, bool inBankAlso = true) const;
        Item* GetItemByGuid(ObjectGuid guid) const;
        Item* GetItemByEntry(uint32 entry, bool inBank = false) const;
        std::vector<Item*> GetItemListByEntry(uint32 entry, bool inBankAlso = false) const;
        Item* GetItemByPos(uint16 pos) const;
        Item* GetItemByPos(uint8 bag, uint8 slot) const;
        Bag*  GetBagByPos(uint8 slot) const;
        Item* GetUseableItemByPos(uint8 bag, uint8 slot) const; //Does additional check for disarmed weapons
        Item* GetWeaponForAttack(WeaponAttackType attackType, bool useable = false) const;
        Item* GetShield(bool useable = false) const;
        Item* GetChildItemByGuid(ObjectGuid guid) const;
        static uint8 GetAttackBySlot(uint8 slot);        // MAX_ATTACK if not weapon slot
        std::set<Item*> &GetItemUpdateQueue() { return m_itemUpdateQueue; }
        static bool IsInventoryPos(uint16 pos) { return IsInventoryPos(pos >> 8, pos & 255); }
        static bool IsInventoryPos(uint8 bag, uint8 slot);
        static bool IsEquipmentPos(uint16 pos) { return IsEquipmentPos(pos >> 8, pos & 255); }
        static bool IsEquipmentPos(uint8 bag, uint8 slot);
        static bool IsBagPos(uint16 pos);
        static bool IsBankPos(uint16 pos) { return IsBankPos(pos >> 8, pos & 255); }
        static bool IsBankPos(uint8 bag, uint8 slot);
        static bool IsReagentBankPos(uint16 pos) { return IsReagentBankPos(pos >> 8, pos & 255); }
        static bool IsReagentBankPos(uint8 bag, uint8 slot);
        static bool IsChildEquipmentPos(uint16 pos) { return IsChildEquipmentPos(pos >> 8, pos & 255); }
        static bool IsChildEquipmentPos(uint8 bag, uint8 slot);
        bool IsValidPos(uint16 pos, bool explicit_pos) { return IsValidPos(pos >> 8, pos & 255, explicit_pos); }
        bool IsValidPos(uint8 bag, uint8 slot, bool explicit_pos);
        uint8 GetInventorySlotCount() const { return GetByteValue(PLAYER_FIELD_BYTES_7, PLAYER_BYTES_5_NUM_BACKPACK_SLOTS); }
        uint8 GetInventoryEndSlot() const;
        uint8 _inventoryEndSlot;
        void SetInventorySlotCount(uint8 slots);
        void SetBankBagSlotCount(uint8 count) { SetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_BANK_BAG_SLOTS, count); }
        
        uint32 CustomMultiDonate = 0; // entry for multi-vendors
        bool HasDonateToken(uint32 count) const;
        bool ChangeDonateTokenCount(int64 change, uint8 buyType, uint64 productId);
        void ModifyCanUseDonate(bool apply){ canUseDonate = apply; }
        bool GetCanUseDonate() const { return canUseDonate; }
        std::string GetInfoForDonate() const;
        
        bool HasItemCount(uint32 item, uint32 count = 1, bool inBankAlso = false) const;
        bool HasItemFitToSpellRequirements(SpellInfo const* spellInfo, Item const* ignoreItem = nullptr) const;
        bool CanNoReagentCast(SpellInfo const* spellInfo) const;
        bool HasItemOrGemWithIdEquipped(uint32 item, uint32 count, uint8 except_slot = NULL_SLOT) const;
        bool HasItemOrGemWithLimitCategoryEquipped(uint32 limitCategory, uint32 count, uint8 except_slot = NULL_SLOT) const;
        bool CanUseReagentBank() const;

        InventoryResult CanTakeMoreSimilarItems(Item* pItem) const { return CanTakeMoreSimilarItems(pItem->GetEntry(), pItem->GetCount(), pItem); }
        InventoryResult CanTakeMoreSimilarItems(uint32 entry, uint32 count) const { return CanTakeMoreSimilarItems(entry, count, nullptr); }
        InventoryResult CanStoreNewItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 item, uint32 count, uint32* no_space_count = nullptr, bool quest = false) const
        {
            return CanStoreItem(bag, slot, dest, item, count, nullptr, false, no_space_count, quest);
        }
        InventoryResult CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap = false) const
        {
            if (!pItem)
                return EQUIP_ERR_ITEM_NOT_FOUND;
            uint32 count = pItem->GetCount();
            return CanStoreItem(bag, slot, dest, pItem->GetEntry(), count, pItem, swap, nullptr);

        }
        InventoryResult CanStoreItems(Item** pItem, int count, uint32& ItemID) const;
        InventoryResult CanEquipNewItem(uint8 slot, uint16& dest, uint32 item, bool swap, bool not_loading = true) const;
        InventoryResult CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading = true) const;

        // This method must be called before equipping parent item!
        InventoryResult CanEquipChildItem(Item* parentItem) const;

        InventoryResult CanEquipUniqueItem(Item* pItem, uint8 except_slot = NULL_SLOT, uint32 limit_count = 1) const;
        InventoryResult CanEquipUniqueItem(ItemTemplate const* itemProto, uint8 except_slot = NULL_SLOT, uint32 limit_count = 1) const;
        InventoryResult CanUnequipItems(uint32 item, uint32 count) const;
        InventoryResult CanUnequipItem(uint16 src, bool swap) const;
        InventoryResult CanBankItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap, bool not_loading = true) const;
        InventoryResult CanBankReagentItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap, bool not_loading = true) const;
        InventoryResult CanUseItem(Item* pItem, bool not_loading = true) const;
        InventoryResult CanUseItem(ItemTemplate const* pItem) const;
        std::list<uint32> GetSpecListByClass(uint32 classId) const;
        bool CanGetItemForLoot(ItemTemplate const* proto, bool specCheck = true) const;
        bool CheckGemForSpec(ItemTemplate const* proto, uint32 playerSpecID) const;
        Item* StoreNewItem(ItemPosCountVec const& pos, uint32 item, bool update, ItemRandomEnchantmentId const& randomPropertyId = {}, GuidSet const& allowedLooters = GuidSet(), std::vector<uint32> const& bonusListIDs = std::vector<uint32>(), uint32 context = 0, bool isRefunde = false);
        Item* StoreItem(ItemPosCountVec const& pos, Item* pItem, bool update);
        Item* EquipNewItem(uint16 pos, uint32 item, bool update, int32 randomPropertyId = 0, std::vector<uint32> const& bonusListIDs = std::vector<uint32>(), uint32 context = 0, bool isRefunde = false);
        Item* EquipItem(uint16 pos, Item* pItem, bool update);
        void AutoUnequipOffhandIfNeed(bool force = false);
        void EquipChildItem(uint8 parentBag, uint8 parentSlot, Item* parentItem);
        void AutoUnequipChildItem(Item* parentItem);
        bool StoreNewItemInBestSlots(uint32 item_id, uint32 item_count, bool not_loading = true, std::vector<uint32> const& bonusListIDs = std::vector<uint32>());
        void ApplyOnItems(uint8 type, std::function<bool(Player*, Item*, uint8, uint8)>&& function);

        bool AutoStoreLoot(uint8 bag, uint8 slot, uint32 loot_id, LootStore const& store, bool broadcast = true, uint32 itemContext = 0);
        bool AutoStoreLoot(uint32 loot_id, LootStore const& store, bool broadcast = true, uint32 itemContext = 0) { return AutoStoreLoot(NULL_BAG, NULL_SLOT, loot_id, store, broadcast, itemContext); }
        void StoreLootItem(uint8 lootSlot, Loot* loot);

        void AddTrackingQuestIfNeeded(ObjectGuid sourceGuid);

        void DepositItemToReagentBank();
        void MoveItemReagentBank(Item* pItem);
        PersonalLootMap personalLoot;
        Loot* GetPersonalLoot(ObjectGuid const& guid);
        void RemoveLoot(ObjectGuid const& guid);
        uint32 GetGoldFromLoot();

        InventoryResult CanTakeMoreSimilarItems(uint32 entry, uint32 count, Item* pItem, uint32* no_space_count = nullptr, bool quest = false) const;
        uint8 GetItemLimitCategoryQuantity(ItemLimitCategoryEntry const* limitEntry) const;
        InventoryResult CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 entry, uint32 count, Item* pItem = nullptr, bool swap = false, uint32* no_space_count = nullptr, bool quest = false) const;

        void AddRefundReference(ObjectGuid it);
        void DeleteRefundReference(ObjectGuid it);

        /// send full data about all currencies to client
        void SendCurrencies();
        void SendPvpRewards();
        /// return count of currency witch has plr
        uint32 GetCurrency(uint32 id) const;
        uint32 GetCurrencyOnWeek(uint32 id) const;
        uint32 GetCurrencyOnSeason(uint32 id) const;
        /// return presence related currency
        bool HasCurrency(uint32 id, uint32 count) const;
        bool HasCurrencySeason(uint32 id, uint32 count) const { return GetCurrencyOnSeason(id) >= count; }
        void SetCurrency(uint32 id, uint32 count, bool sendInChat = false);
        uint32 GetCurrencyWeekCap(uint32 id);
        void ResetCurrencyWeekCap();
        void UpdateConquestCurrencyCap(uint32 currency);
        void ModifyCurrencyFlag(uint32 id, uint8 flag);
        void ModifyCurrency(uint32 id, int32 count, bool sendInChat = false, bool ignoreMultipliers = false, bool modifyWeek = true, bool modifySeason = true, bool sendToast = false, bool refund = false);
        uint32 GetTotalCurrencyCap(uint32 currencyID);
        void ModCurrnecyCap(uint32 currencyID, uint32 value);
        void ModifyExcludeCasterAuraSpell(uint32 auraId, bool apply);

        /*********************************************************/
        /***                 ARCHAEOLOGY SYSTEM                ***/
        /*********************************************************/

        void _SaveArchaeology(SQLTransaction& trans);
        void _LoadArchaeology(PreparedQueryResult result);
        void _LoadArchaeologyFinds(PreparedQueryResult result);
        bool HasResearchSite(uint32 id) const
        {
            return _researchSites.find(id) != _researchSites.end();
        }

        bool HasResearchProjectOfBranch(uint32 id) const;
        bool HasResearchProject(uint32 id) const;
        void ReplaceResearchProject(uint32 oldId, uint32 newId);

        static float GetRareArtifactChance(uint32 skillValue);

        void ShowResearchSites();
        void GenerateResearchSites();
        void GenerateResearchSiteInMap(uint32 mapId);
        void GenerateResearchProjects();
        bool SolveResearchProject(uint32 spellId, SpellCastTargets& targets);
        void RandomizeSitesInMap(uint32 mapId, uint8 count);
        bool TeleportToDigsiteInMap(uint32 mapId);
        static bool IsPointInZone(ResearchPOIPoint &test, ResearchPOIPointVector &polygon);
        static uint32 GetBranchByGO(uint32 entry);
        uint16 GetResearchSiteID();
        bool OnSurvey(uint32& entry, float& x, float& y, float& z, float &orientation);
        bool CanResearchWithLevel(uint32 site_id);
        uint8 CanResearchWithSkillLevel(uint32 site_id);
        bool GenerateDigSiteLoot(uint16 zoneid, DigSite &site);
        uint32 AddCompletedProject(ResearchProjectEntry const* entry);
        bool IsCompletedProject(uint32 id, bool onlyRare);
        void SendCompletedProjects();
        void SendSurveyCast(uint32 count, uint32 max, uint32 branchId, bool completed);

        DigSite _digSites[MAX_RESEARCH_SITES];
        ResearchSiteSet _researchSites;
        CompletedProjectList _completedProjects;
        bool _archaeologyChanged;

        // END

        void SetItemCooldown(Item* pItem, uint32 cooldown);
        void QuickEquipItem(uint16 pos, Item* pItem);
        void VisualizeItem(uint8 slot, Item* pItem);
        void SetVisibleItemSlot(uint8 slot, Item* pItem);
        Item* BankItem(ItemPosCountVec const& dest, Item* pItem, bool update)
        {
            return StoreItem(dest, pItem, update);
        }
        void RemoveItem(uint8 bag, uint8 slot, bool update);
        void RemoveItem(Item* pItem, bool update);
        void MoveItemFromInventory(uint8 bag, uint8 slot, bool update);
        void MoveItemFromInventory(Item* pItem, bool update);
                                                            // in trade, auction, guild bank, mail....
        void MoveItemToInventory(ItemPosCountVec const& dest, Item* pItem, bool update, bool in_characterInventoryDB = false);
        void RemoveItemDependentAurasAndCasts(Item* pItem);
        void DestroyItem(uint8 bag, uint8 slot, bool update);
        void DestroyItemCount(uint32 item, uint32 count, bool update, bool unequip_check = false);
        void DestroyItemCount(Item* item, uint32& count, bool update);
        void DestroyConjuredItems(bool update);
        void DestroyZoneLimitedItem(bool update, uint32 new_zone);
        void SplitItem(uint16 src, uint16 dst, uint32 count);
        void SwapItem(uint16 src, uint16 dst);
        void AddItemToBuyBackSlot(Item* pItem);
        Item* GetItemFromBuyBackSlot(uint32 slot);
        void RemoveItemFromBuyBackSlot(uint32 slot, bool del);
        void TakeExtendedCost(uint32 extendedCostId, uint32 count);
        void SendEquipError(InventoryResult msg, Item* pItem = nullptr, Item* pItem2 = nullptr, uint32 itemid = 0);
        void SendBuyError(BuyResult msg, Creature* creature = nullptr, uint32 item = 0);
        void SendSellError(SellResult msg, Creature* creature = nullptr, ObjectGuid guid = ObjectGuid::Empty);
        void AddWeaponProficiency(uint32 newflag) { m_WeaponProficiency |= newflag; }
        void AddArmorProficiency(uint32 newflag) { m_ArmorProficiency |= newflag; }
        uint32 GetWeaponProficiency() const { return m_WeaponProficiency; }
        uint32 GetArmorProficiency() const { return m_ArmorProficiency; }
        bool IsUseEquipedWeapon(bool mainhand) const
        {
            // disarm applied only to mainhand weapon
            return !IsInFeralForm() && (!mainhand || !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED));
        }
        bool IsTwoHandUsed() const
        {
            Item* mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

            if (mainItem && mainItem->GetTemplate()->GetSubClass() == ITEM_SUBCLASS_WEAPON_WAND)
                return false;

            return mainItem && ((mainItem->GetTemplate()->GetInventoryType() == INVTYPE_2HWEAPON && !CanTitanGrip()) || mainItem->GetTemplate()->GetInventoryType() == INVTYPE_RANGED || mainItem->GetTemplate()->GetInventoryType() == INVTYPE_THROWN || mainItem->GetTemplate()->GetInventoryType() == INVTYPE_RANGEDRIGHT);
        }
        void SendNewItem(Item* item, uint32 count, bool received, bool created, bool broadcast = false, bool bonusRoll = false);
        bool BuyItemFromVendorSlot(ObjectGuid vendorguid, uint32 vendorslot, uint32 item, uint8 count, uint8 bag, uint8 slot);
        bool BuyCurrencyFromVendorSlot(ObjectGuid vendorGuid, uint32 vendorSlot, uint32 currency, uint32 count);
        bool _StoreOrEquipNewItem(uint32 vendorslot, uint32 item, uint8 count, uint8 bag, uint8 slot, int64 price, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore);

        float GetReputationPriceDiscount(Creature const* creature) const;

        Player* GetTrader() const;
        TradeData* GetTradeData() const;
        void TradeCancel(bool sendback);

        void RecalculatePvPAmountOfAuras();
        void RecalculateAmountAllAuras();

        void UpdateEnchantTime(uint32 time);
        void UpdateSoulboundTradeItems();
        void AddTradeableItem(Item* item);
        void RemoveTradeableItem(Item* item);
        void UpdateItemDuration(uint32 time, bool realtimeonly = true);
        void AddEnchantmentDurations(Item* item);
        void RemoveEnchantmentDurations(Item* item);
        void RemoveArenaEnchantments(EnchantmentSlot slot);
        void AddEnchantmentDuration(Item* item, EnchantmentSlot slot, uint32 duration);
        void ApplyEnchantment(Item* item, EnchantmentSlot slot, bool apply, bool apply_dur = true, bool ignore_condition = false);
        void ApplyEnchantment(Item* item, bool apply);
        void UpdateSkillEnchantments(uint16 skill_id, uint16 curr_value, uint16 new_value);
        void SendEnchantmentDurations();
        void AddItemDurations(Item* item);
        void RemoveItemDurations(Item* item);
        void SendItemDurations();
        void LoadCorpse();
        void LoadPet();

        bool AddItem(uint32 itemId, uint32 count, uint32* noSpaceForCount = nullptr, ObjectGuid guid = ObjectGuid::Empty );

        /*********************************************************/
        /***                    GOSSIP SYSTEM                  ***/
        /*********************************************************/

        void PrepareGossipMenu(WorldObject* source, uint32 menuId = 0, bool showQuests = false);
        void SendPreparedGossip(WorldObject* source);
        void OnGossipSelect(WorldObject* source, uint32 gossipListId, uint32 menuId);

        uint32 GetGossipTextId(uint32 menuId, WorldObject* source);
        uint32 GetGossipTextId(WorldObject* source);
        static uint32 GetDefaultGossipMenuForSource(WorldObject* source);
        uint32 GetGossipFriendshipFactionID(uint32 menuId, WorldObject* source);

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        int32 GetQuestLevel(Quest const* quest) const
        {
            if (!quest)
                 return getLevel();
            return quest->Level > 0 ? quest->Level : std::min<int32>(getLevel(), quest->MaxScalingLevel);
        }

        void PrepareAreaQuest(uint32 area);
        void PrepareQuestMenu(ObjectGuid guid);
        void SendPreparedQuest(ObjectGuid guid);
        Quest const* GetNextQuest(ObjectGuid guid, Quest const* quest);
        bool CanSeeStartQuest(Quest const* quest);
        bool CanTakeQuest(Quest const* quest, bool msg);
        bool CanAddQuest(Quest const* quest, bool msg);
        bool CanCompleteQuest(uint32 quest_id);
        bool CanCompleteRepeatableQuest(Quest const* quest);
        bool CanRewardQuest(Quest const* quest, bool msg);
        bool CanRewardQuest(Quest const* quest, uint32 reward, bool msg, uint32 packItemId);
        void AddQuest(Quest const* quest, Object* questGiver);
        void CompleteQuest(uint32 quest_id);
        void IncompleteQuest(uint32 quest_id);
        int32 GetQuestMoneyReward(Quest const* quest) const;
        uint32 GetQuestXPReward(Quest const* quest);
        void RewardQuest(Quest const* quest, uint32 reward, Object* questGiver, bool announce = true, uint32 packItemId = 0);
        void FailQuest(uint32 quest_id);
        bool SatisfyQuestSkill(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestLevel(Quest const* qInfo, bool msg);
        bool SatisfyQuestLog(bool msg);
        bool SatisfyQuestPreviousQuest(Quest const* qInfo, bool msg);
        bool SatisfyQuestClass(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestRace(Quest const* qInfo, bool msg);
        bool SatisfyQuestReputation(Quest const* qInfo, bool msg);
        bool SatisfyQuestStatus(Quest const* qInfo, bool msg);
        bool SatisfyQuestConditions(Quest const* qInfo, bool msg);
        bool SatisfyQuestTimed(Quest const* qInfo, bool msg);
        bool SatisfyQuestExclusiveGroup(Quest const* qInfo, bool msg);
        bool SatisfyQuestNextChain(Quest const* qInfo, bool msg);
        bool SatisfyQuestPrevChain(Quest const* qInfo, bool msg);
        bool SatisfyQuestDay(Quest const* qInfo, bool msg);
        bool SatisfyQuestWeek(Quest const* qInfo);
        bool SatisfyQuestSeasonal(Quest const* qInfo);
        bool GiveQuestSourceItem(Quest const* quest);
        bool TakeQuestSourceItem(uint32 questId, bool msg);
        bool GetQuestRewardStatus(uint32 quest_id) const;
        QuestStatus GetQuestStatus(uint32 quest_id) const;
        QuestStatus GetDailyQuestStatus(uint32 quest_id) const;
        void SetQuestStatus(uint32 quest_id, QuestStatus status);
        void RemoveActiveQuest(uint32 quest_id);
        void RemoveRewardedQuest(uint32 quest_id);

        void SetDailyQuestStatus(uint32 quest_id);
        void SetWeeklyQuestStatus(uint32 quest_id);
        void SetSeasonalQuestStatus(uint32 quest_id);
        void DailyReset();
        void ResetWeeklyQuestStatus();
        void ResetSeasonalQuestStatus(uint16 event_id);

        uint16 FindQuestSlot(uint32 quest_id) const;
        uint32 GetQuestSlotQuestId(uint16 slot) const;
        uint32 GetQuestSlotState(uint16 slot)   const;
        uint16 GetQuestSlotCounter(uint16 slot, uint8 counter) const;
        uint32 GetQuestSlotTime(uint16 slot)    const;
        void SetQuestSlot(uint16 slot, uint32 quest_id, uint32 timer = 0);
        void SetQuestSlotCounter(uint16 slot, uint8 counter, uint16 count);
        void SetQuestSlotState(uint16 slot, uint32 state);
        void RemoveQuestSlotState(uint16 slot, uint32 state);
        void SetQuestSlotTimer(uint16 slot, uint32 timer);
        void SetSpecialCriteriaComplete(uint16 slot, uint8 StorageIndex);

        void SetQuestCompletedBit(uint32 questBit, bool completed);
        bool IsQuestBitFlaged(uint32 questBit) const;

        uint16 GetReqKillOrCastCurrentCount(uint32 quest_id, int32 entry);
        void AreaExploredOrEventHappens(uint32 questId);
        void GroupEventHappens(uint32 questId, WorldObject const* pEventObject);
        void ItemAddedQuestCheck(uint32 entry, uint32 count);
        void ItemRemovedQuestCheck(uint32 entry, uint32 count);
        void KilledMonster(CreatureTemplate const* cInfo, ObjectGuid guid);
        void KilledMonsterCredit(uint32 entry, ObjectGuid guid = ObjectGuid::Empty);
        void KilledPlayerCredit();
        void KillCreditGO(uint32 entry, ObjectGuid const& guid);
        void AchieveCriteriaCredit(uint32 criteriaID);
        bool TalkedToCreature(uint32 entry, ObjectGuid guid);
        void MoneyChanged(uint32 value);
        void ReputationChanged(FactionEntry const* factionEntry);
        void CurrencyChanged(uint32 currencyID, int32 change);
        bool HasQuestForItem(uint32 itemid) const;
        bool HasQuestForCreature(CreatureTemplate const* cInfo) const;
        bool HasQuestForGO(int32 GOId) const;
        void UpdateForQuestWorldObjects();
        void UpdateAvailableQuestLines();
        bool CanShareQuest(uint32 quest_id) const;
        void QuestObjectiveSatisfy(uint32 objectId, uint32 amount, QuestObjectiveType type = QUEST_OBJECTIVE_MONSTER, ObjectGuid guid = ObjectGuid::Empty);

        int32 GetQuestObjectiveData(Quest const* quest, int8 storageIndex) const;
        int32 GetQuestObjectiveData(uint32 QuestID, uint32 ObjectiveID) const;
        void SetQuestObjectiveData(Quest const* quest, QuestObjective const* obj, int32 data, bool isLoad = false);
        void UpdateQuestObjectiveData(Quest const* quest);
        bool HasCompletedQuest(uint32 questId) const;
        void SendQuestComplete(Quest const* quest);
        void SendQuestReward(Quest const* quest, uint32 XP, Object* questGiver, int32 moneyReward, Item* item, bool hideChatMessage);
        void SendQuestFailed(uint32 questId, InventoryResult reason = EQUIP_ERR_OK);
        void SendQuestTimerFailed(uint32 quest_id);
        void SendCanTakeQuestResponse(uint32 msg, Quest const* qInfo, std::string = "none") const;
        void SendQuestConfirmAccept(Quest const* quest, Player* pReceiver);
        void SendPushToPartyResponse(Player* player, uint8 msg);
        void SendQuestUpdateAddCredit(Quest const* quest, ObjectGuid guid, QuestObjective const& obj, uint16 count);
        void SendQuestUpdateAddPlayer(Quest const* quest, uint16 old_count, uint16 add_count);
        std::tuple<uint32, uint32> GetWorldQuestBonusTreeMod(WorldQuest const* wq);
        bool WorldQuestCompleted(uint32 QuestID) const;
        void ResetWorldQuest(uint32 QuestID = 0);
        void ClearWorldQuest();

        ObjectGuid GetDivider() { return m_divider; }
        void SetDivider(ObjectGuid guid) { m_divider = guid; }

        uint32 GetInGameTime() { return m_ingametime; }

        void SetInGameTime(uint32 time) { m_ingametime = time; }

        void AddTimedQuest(uint32 quest_id) { m_timedquests.insert(quest_id); }
        void RemoveTimedQuest(uint32 quest_id) { m_timedquests.erase(quest_id); }

        void SendMusic(uint32 soundKitID);
        void SendSound(uint32 soundId, ObjectGuid source);
        void SendSoundToAll(uint32 soundId, ObjectGuid source);

        Item* GetArtifactWeapon();

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        bool LoadFromDB(ObjectGuid guid, SQLQueryHolder *holder);
        bool isBeingLoaded() const override { return GetSession()->PlayerLoading();}

        void Initialize(ObjectGuid::LowType guid);
        static uint32 GetUInt32ValueFromArray(Tokenizer const& data, uint16 index);
        static float  GetFloatValueFromArray(Tokenizer const& data, uint16 index);
        static uint32 GetZoneIdFromDB(ObjectGuid guid);
        static uint32 GetLevelFromDB(ObjectGuid guid);
        static bool   LoadPositionFromDB(uint32& mapid, float& x, float& y, float& z, float& o, bool& in_flight, ObjectGuid guid);

        static bool IsValidGender(uint8 Gender) { return Gender <= GENDER_FEMALE; }
        static bool IsValidClass(uint8 Class) { return ((1 << (Class - 1)) & CLASSMASK_ALL_PLAYABLE) != 0; }
        static bool IsValidRace(uint8 Race) { return ((1 << (Race - 1)) & RACEMASK_ALL_PLAYABLE) != 0; }
        static bool ValidateAppearance(uint8 race, uint8 class_, uint8 gender, uint8 hairID, uint8 hairColor, uint8 faceID, uint8 facialHair, uint8 skinColor, std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> const& customDisplay, bool create = false);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void SaveToDB(bool create = false);
        void SaveInventoryAndGoldToDB(SQLTransaction& trans);                    // fast save function for item/money cheating preventing
        void SaveGoldToDB(SQLTransaction& trans);

        static void SetUInt32ValueInArray(Tokenizer& data, uint16 index, uint32 value);
        static void Customize(ObjectGuid::LowType guid, WorldPackets::Character::CharCustomizeInfo* info);
        static void SavePositionInDB(uint32 mapid, float x, float y, float z, float o, uint32 zone, ObjectGuid guid);

        static void DeleteFromDB(ObjectGuid playerguid, uint32 accountId, bool updateRealmChars = true, bool deleteFinally = false);
        static void DeleteOldCharacters();
        static void DeleteOldCharacters(uint32 keepDays);

        bool m_mailsLoaded;
        bool m_mailsUpdated;

        void SetBindPoint(ObjectGuid guid);
        void SendTalentWipeConfirm(ObjectGuid guid, RespecType type = RESPEC_TYPE_TALENTS);
        void RegenerateAll();
        void Regenerate(Powers power, float regenTimer, PowerTypeEntry const* powerEntry = nullptr);
        void RegenerateHealth();
        void setWeaponChangeTimer(uint32 time) {m_weaponChangeTimer = time;}

        uint64 GetMoney() const;
        bool ModifyMoney(int64 d, bool sendError = true);
        bool HasEnoughMoney(uint64 amount) const;
        bool HasEnoughMoney(int64 amount) const;
        void SetMoney(uint64 value);

        RewardedQuestSet const& getRewardedQuests() const { return m_RewardedQuests; }
        QuestStatusData* getQuestStatus(uint32 questId);

        size_t GetRewardedQuestCount() const { return m_RewardedQuests.size(); }
        bool IsQuestRewarded(uint32 quest_id) const;
        bool HasAccountQuest(uint32 quest_id) const;
        bool IsQuestDFRewarded(uint32 quest_id) const;
        bool IsQuestDailyRewarded(uint32 quest_id) const;
        bool IsQuestWeekRewarded(uint32 quest_id) const;
        bool IsQuestSeasonalRewarded(uint32 quest_id) const;

        ObjectGuid GetSelection() const { return m_curSelection; }
        ObjectGuid GetLastSelection() const { return m_lastSelection; }
        Unit* GetSelectedUnit() const;
        Unit* GetLastSelectedUnit() const;
        Player* GetSelectedPlayer() const;
        void SetSelection(ObjectGuid const& guid);

        void SendMailResult(uint32 mailId, MailResponseType mailAction, MailResponseResult mailError, uint32 equipError = 0, ObjectGuid::LowType item_guid = UI64LIT(0), uint32 item_count = 0);
        void SendNewMail();
        void UpdateNextMailTimeAndUnreads();
        void AddNewMailDeliverTime(time_t deliver_time);
        bool IsMailsLoaded() const { return m_mailsLoaded; }

        void RemoveMail(uint32 id);
        void SafeRemoveMailFromIgnored(ObjectGuid const& ignoredPlayerGuid);

        void AddMail(Mail* mail) { m_mail.push_front(mail);}// for call from WorldSession::SendMailTo
        uint32 GetMailSize() { return m_mail.size();}
        Mail* GetMail(uint32 id);

        PlayerMails::iterator GetMailBegin() { return m_mail.begin();}
        PlayerMails::iterator GetMailEnd() { return m_mail.end();}

        void UpdatePlayerNameData();

        /*********************************************************/
        /*** MAILED ITEMS SYSTEM ***/
        /*********************************************************/

        uint8 unReadMails;
        time_t m_nextMailDelivereTime;

        typedef std::unordered_map<ObjectGuid::LowType, Item*> ItemMap;

        ItemMap mMitems;                                    //template defined in objectmgr.cpp

        Item* GetMItem(ObjectGuid::LowType id);
        void AddMItem(Item* it);
        bool RemoveMItem(ObjectGuid::LowType id);
        bool SearchItemOnMail(uint32 entry);

        void SendOnCancelExpectedVehicleRideAura();

        void PetSpellInitialize();
        void CharmSpellInitialize();
        void PossessSpellInitialize();
        void VehicleSpellInitialize();
        void SendRemoveControlBar();
        void SendKnownSpells();
        bool HasSpell(uint32 spell) override;
        bool HasActiveSpell(uint32 spell);            // show in spellbook
        TrainerSpellState GetTrainerSpellState(TrainerSpell const* trainer_spell) const;
        bool IsSpellFitByClassAndRace(uint32 spell_id) const;
        bool IsNeedCastPassiveSpellAtLearn(SpellInfo const* spellInfo) const;

        void SendProficiency(ItemClass itemClass, uint32 itemSubclassMask);
        bool addSpell(uint32 spellId, bool active, bool learning, bool dependent, bool disabled, bool loading = false, uint32 fromSkill = 0, bool battlePet = false);
        void learnSpell(uint32 spell_id, bool dependent, uint32 fromSkill = 0, bool sendMessage = true);
        void removeSpell(uint32 spell_id, bool disabled = false, bool learn_low_rank = true, bool sendMessage = true);
        void resetSpells();
        SpellInfo const* GetCastSpellInfo(SpellInfo const* spellInfo, uint32 newSpellId) const;
        void AddOverrideSpell(uint32 overridenSpellId, uint32 newSpellId);
        void RemoveOverrideSpell(uint32 overridenSpellId, uint32 newSpellId);
        void LearnSpecializationSpells();
        void UnlearnSpellsFromOtherClasses();
        void RemoveSpecializationSpells();
        void UpdateSkillsForLevel();
        void LearnDefaultSkills();
        void LearnDefaultSpells();
        void LearnDefaultSkill(SkillRaceClassInfoEntry const* rcInfo);
        void learnQuestRewardedSpells();
        void learnQuestRewardedSpells(Quest const* quest);
        void learnSpellHighRank(uint32 spellid);
        void AddTemporarySpell(uint32 spellId);
        void RemoveTemporarySpell(uint32 spellId);
        void SetReputation(uint32 factionentry, uint32 value);
        uint32 GetReputation(uint32 factionentry);
        std::string GetGuildName();
        Guild* GetGuild();
        void SendMountSpells();

        // Talents
        uint32 GetPrimarySpecialization() const { return _specializationInfo.PrimarySpecialization; }
        void SetPrimarySpecialization(uint32 spec) { _specializationInfo.PrimarySpecialization = spec; }
        uint8 GetActiveTalentGroup() const { return _specializationInfo.ActiveGroup; }
        void SetActiveTalentGroup(uint8 group){ _specializationInfo.ActiveGroup = group; }

        uint32 GetSpecializationRole() const;
        RoleForSoloQ GetRoleForSoloQ() const;
        lfg::LfgRoles GetSpecializationRoleMaskForGroup() const;
        bool isInTankSpec() const;
        uint32 GetDefaultSpecId() const;
        bool IsRangedDamageDealer(bool allowHeal = true) const;
        bool IsMeleeDamageDealer(bool allowTank = false) const;
        uint16 GetSpecType();
        static uint8 ConvertLFGRoleToRole(uint8 role);

        bool ResetTalents(bool no_cost = false);
        void InitTalentForLevel();
        void SendTalentsInfoData(bool pet);
        bool LearnTalent(uint32 talentId);
        bool AddTalent(TalentEntry const* talent, uint8 index, bool learning);
        bool HasTalent(uint32 spell_id, uint8 index) const;
        void RemoveTalent(TalentEntry const* talent, bool isDelete = true, bool sendMessage = true, bool allGroups = false);
        uint8 CalculateTalentsPoints() const;
        void ResetTalentSpecialization();

        // PvP talents
        bool LearnPvpTalent(uint16 talentID);
        bool AddPvPTalent(PvpTalentEntry const* talent, uint8 index);
        bool HasPvPTalent(uint32 spellID) const;
        void RemovePvPTalent(uint32 spellID, uint8 group, bool isDelete = true, bool sendMessage = true);
        void RemoveAllPvPTalent(bool isDelete = true);
        void TogglePvpTalents(bool enable);
        void TogglePvpStatsScaling(bool enable);
        bool IsAreaThatActivatesPvpTalents(uint32 areaID) const;
        void EnablePvpRules(bool recalcItems = true);
        void DisablePvpRules(bool recalcItems = true, bool checkZone = true);
        bool HasPvpRulesEnabled();
        bool HasPvpStatsScalingEnabled() const;
        bool HasPvpRulesTimer() const;
        void SetPvpRulesTimer(bool enable);
        bool GetCustomPvPMods(float& val, uint32 type, uint32 specID) const;
        void CalcPvPTemplate(AuraType auratype, float& templateMod, float& otherMod, std::function<bool(AuraEffect const*)> const& predicate);

        void ActivateTalentGroup(ChrSpecializationEntry const* spec);
        void ForceChangeTalentGroup(uint32 specId);

        void SetQueueRoleMask(uint8 bracketId, uint8 roleMask, bool temp = false) { if (temp) m_bgQueueRolesTemp[bracketId] = roleMask; else m_bgQueueRoles[bracketId] = roleMask; };
        uint8 GetQueueRoleMask(uint8 bracketId, bool temp = false) const;
        int8 GetSingleQueueRole(uint8 bracketId) const;

        void ChangeSpecializationForBGIfNeed(uint8 bracketId);
        void ChangeSpecializationifNeed(uint8 role);
        bool IsCanChangeSpecToAnotherRole() const;

        PlayerTalentMap const* GetTalentMap(uint8 index) const { return &_specializationInfo.Talents[index]; }
        PlayerTalentMap* GetTalentMap(uint8 index) { return &_specializationInfo.Talents[index]; }
        PlayerPvPTalentMap const* GetPvPTalentMap(uint8 spec) const { return &_specializationInfo.PvPTalents[spec]; }
        PlayerPvPTalentMap* GetPvPTalentMap(uint8 spec) { return &_specializationInfo.PvPTalents[spec]; }
        std::vector<uint32> const& GetGlyphs(uint8 spec) const { return _specializationInfo.Glyphs[spec]; }
        std::vector<uint32>& GetGlyphs(uint8 spec) { return _specializationInfo.Glyphs[spec]; }
        uint32 GetSpecializationId() const { return GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID); }
        ActionButtonList const& GetActionButtons() const { return m_actionButtons; }

        uint32 GetFreePrimaryProfessionPoints() const { return GetUInt32Value(PLAYER_FIELD_CHARACTER_POINTS); }
        void SetFreePrimaryProfessions(uint16 profs) { SetUInt32Value(PLAYER_FIELD_CHARACTER_POINTS, profs); }
        void InitPrimaryProfessions();

        PlayerSpellMap const& GetSpellMapConst();
        PlayerSpellMap& GetSpellMap();

        const ItemSpellList& GetItemSpellList();
        void HandleItemSpellList(uint32 spellId, bool apply);

        const ExcludeCasterSpellList& GetExcludeCasterSpellList();
        void HandleExcludeCasterSpellList(uint32 spellId, bool apply);

        const CasterAuraStateSpellList& GetCasterAuraStateSpellList();
        void HandleCasterAuraStateSpellList(uint32 spellId, bool apply);

        SpellCooldowns const& GetSpellCooldownMap() const;

        void AddSpellMod(SpellModifier* mod, bool apply);
        void SendSpellMods();
        bool IsAffectedBySpellmod(SpellInfo const* spellInfo, SpellModifier* mod, Spell* spell = nullptr) const;

        template <class T>
        void ApplySpellMod(uint32 spellId, SpellModOp op, T& basevalue, Spell* spell = nullptr) const;

        void ApplyDotMod(uint32 spellId, SpellModOp op, float& basemod, float& baseadd, bool onapply);
        void RemoveSpellMods(Spell* spell, bool casting = false);
        void RestoreSpellMods(Spell* spell, uint32 ownerAuraId = 0, Aura* aura = nullptr);
        void RestoreAllSpellMods(uint32 ownerAuraId = 0, Aura* aura = nullptr);
        void DropModCharge(SpellModifier* mod, Spell* spell) const;
        void SetSpellModTakingSpell(Spell* spell, bool apply);
        bool HasInstantCastModForSpell(SpellInfo const* spellInfo);

        static uint32 const infinityCooldownDelay = MONTH;  // used for set "infinity cooldowns" for spells and check
        static uint32 const infinityCooldownDelayCheck = MONTH/2;
        bool HasSpellCooldown(uint32 spell_id) override;
        double GetSpellCooldownDelay(uint32 spell_id) override;

        double GetRPPMSpellCooldownDelay(uint32 spell_id, ObjectGuid itemGUID);
        double GetLastSuccessfulProc(uint32 spell_id, ObjectGuid itemGUID);
        double GetLastChanceToProc(uint32 spell_id, ObjectGuid itemGUID);
        void SetLastSuccessfulProc(uint32 spell_id, double time, ObjectGuid itemGUID);
        void SetLastChanceToProc(uint32 spell_id, double time, ObjectGuid itemGUID);
        bool GetRPPMProcChance(double &cooldown, float RPPM, const SpellInfo* spellProto, ObjectGuid itemGUID);
        void AddRPPMSpellCooldown(uint32 spell_id, ObjectGuid itemGUID, double end_time);

        void AddSpellAndCategoryCooldowns(SpellInfo const* spellInfo, uint32 itemId, Spell* spell = nullptr, bool onHold = false);
        void AddSpellCooldown(uint32 spellId, uint32 itemId, double cooldownEnd) override;
        void SendCooldownEvent(SpellInfo const* spellInfo, uint32 itemId = 0, Spell* spell = nullptr, bool setCooldown = true);
        void ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs) override;
        void RemoveSpellCooldown(uint32 spell_id, bool update = false);
        void RemoveSpellCategoryCooldown(uint32 cat, bool update = false);
        void SendClearCooldown(uint32 spell_id, Unit* target);
        void ModifySpellCooldown(uint32 spell_id, int32 delta);
        void RemoveCategoryCooldownBySpell(uint32 spell_id, bool update /* = false */);
        float SpellCooldownModByRate(SpellInfo const* spellInfo, bool sendSpeedRate = false);

        void RemoveCategoryCooldown(uint32 cat);
        void RemoveArenaSpellCooldowns(bool removeActivePetCooldowns = false);
        void RemoveAllSpellCooldown();
        void _LoadSpellCooldowns(PreparedQueryResult result);
        void _SaveSpellCooldowns(SQLTransaction& trans);

        bool HasChargesForSpell(SpellInfo const* spellInfo) const;
        uint8 GetMaxSpellCategoryCharges(SpellCategoryEntry const* categoryEntry) const;
        uint32 GetSpellCategoryChargesTimer(SpellCategoryEntry const* categoryEntry, SpellInfo const* spellInfo = nullptr, bool sendSpeedRate = false) const;
        uint8 GetMaxSpellCategoryCharges(uint32 category) const;
        void TakeSpellCharge(SpellInfo const* spellInfo);
        void UpdateSpellCharges(uint32 diff);
        void RecalculateSpellCategoryCharges(uint32 category);
        void RecalculateSpellCategoryRegenTime(uint32 category);
        void RestoreSpellCategoryCharges(uint32 categoryId = 0);
        void ModSpellCharge(uint32 SpellID, int32 num);
        void ModSpellChargeCooldown(uint32 SpellID, int32 delta);
        uint8 GetChargesForSpell(SpellInfo const* spellInfo) const;
        uint32 GetChargesCooldown(uint32 SpellID) const;

        uint32 GetLastPotionId() { return m_lastPotionId; }
        void SetLastPotionId(uint32 item_id) { m_lastPotionId = item_id; }
        void UpdatePotionCooldown(Spell* spell = nullptr);

        // visible items
        bool HandleChangeSlotModel(uint32 newItem, uint16 pos);
        void HandleAltVisSwitch();

        void SetResurrectRequestData(Unit* caster, uint64 health, uint32 mana, uint32 appliedAura, SpellInfo const* resSpell = nullptr);
        void ClearResurrectRequestData();
        bool IsRessurectRequestedBy(ObjectGuid guid) const;
        bool IsRessurectRequested() const;
        void ResurectUsingRequestData();

        uint8 getCinematic()
        {
            return m_cinematic;
        }
        void setCinematic(uint8 cine)
        {
            m_cinematic = cine;
        }

        bool addActionButton(uint8 button, uint32 action, uint8 type, ActionButtonUpdateState state = ACTIONBUTTON_NEW);
        void removeActionButton(uint8 button);
        ActionButton const* GetActionButton(uint8 button);
        void SendInitialActionButtons() { SendActionButtons(0); }
        void SendActionButtons(uint32 state);
        bool IsActionButtonDataValid(uint8 button, uint32 action, uint8 type);

        int8 GetFreeActionButton();

        PvPInfo pvpInfo;
        void UpdatePvPState(bool onlyFFA = false);
        void SetPvP(bool state) override;
        void UpdatePvP(bool state, bool override=false);
        void UpdateZone(uint32 newZone, uint32 newArea);
        void UpdateArea(uint32 newArea);
        void ChaeckSeamlessTeleport(uint32 newZoneOrArea, bool isArea = false);
        void ZoneTeleport(uint32 zoneId);
        bool InFFAPvPArea();

        void UpdateZoneDependentAuras(uint32 zone_id);    // zones
        void UpdateAreaDependentAuras(uint32 area_id);    // subzones
        void UpdateAreaQuestTasks(uint32 newAreaId, uint32 oldAreaId);

        void UpdateAfkReport(time_t currTime);
        void UpdatePvPFlag(time_t currTime);
        void UpdateContestedPvP(uint32 currTime);
        void SetContestedPvPTimer(int32 newTime) {m_contestedPvPTimer = newTime;}
        void ResetContestedPvP();

        /** todo: -maybe move UpdateDuelFlag+DuelComplete to independent DuelHandler.. **/
        DuelInfo* duel;
        void UpdateDuelFlag(uint32 diff);
        void CheckDuelDistance();
        void DuelComplete(DuelCompleteType type);
        void SendDuelCountdown(uint32 counter);

        bool IsGroupVisibleFor(Player const* p) const;
        bool IsInSameGroupWith(Player const* p) const;
        bool IsInSameRaidWith(Player const* p) const { return p == this || (GetGroup() != nullptr && GetGroup() == p->GetGroup()); }
        void UninviteFromGroup();
        static void RemoveFromGroup(Group* group, ObjectGuid guid, RemoveMethod method = GROUP_REMOVEMETHOD_DEFAULT, ObjectGuid kicker = ObjectGuid::Empty, const char* reason = nullptr);
        void RemoveFromGroup(RemoveMethod method = GROUP_REMOVEMETHOD_DEFAULT) { RemoveFromGroup(GetGroup(), GetGUID(), method); }
        void SendUpdateToOutOfRangeGroupMembers();

        void SetInGuild(ObjectGuid::LowType guildId);
        void SetRank(uint8 rankId) { SetUInt32Value(PLAYER_FIELD_GUILD_RANK_ID, rankId); }
        uint32 GetRank() { return GetUInt32Value(PLAYER_FIELD_GUILD_RANK_ID); }
        void SetGuildLevel(uint32 level) { SetUInt32Value(PLAYER_FIELD_GUILD_LEVEL, level); }
        uint32 GetGuildLevel() { return GetUInt32Value(PLAYER_FIELD_GUILD_LEVEL); }
        void SetGuildIdInvited(ObjectGuid::LowType GuildId, ObjectGuid guid = ObjectGuid::Empty) { m_GuildIdInvited = GuildId; m_GuildInviterGuid = guid; }
        ObjectGuid GetGuildInviterGuid() const { return m_GuildInviterGuid; }
        ObjectGuid::LowType GetGuildId() const { return GetGuidValue(OBJECT_FIELD_DATA).GetCounter(); }
        ObjectGuid GetGuildGUID() const { return GetGuidValue(OBJECT_FIELD_DATA); }
        static ObjectGuid::LowType GetGuildIdFromDB(ObjectGuid guid);
        static uint8 GetRankFromDB(ObjectGuid guid);
        ObjectGuid::LowType GetGuildIdInvited() { return m_GuildIdInvited; }
        static void RemovePetitionsAndSigns(ObjectGuid guid);

        // Bracket System
        void InitBrackets();

        //! Use for get all data
        uint32 GetBracketInfo(uint8 slot, BracketInfoType type) const { return GetUInt32Value(PLAYER_FIELD_PVP_INFO + (slot * BRACKET_END) + type); }
        void SetBracketInfoField(uint8 slot, BracketInfoType type, uint32 value)
        {
            SetUInt32Value(PLAYER_FIELD_PVP_INFO + (slot * BRACKET_END) + type, value);
        }

        Difficulty GetDifficultyID(MapEntry const* mapEntry) const;
        Difficulty GetDungeonDifficultyID() const { return m_dungeonDifficulty; }
        Difficulty GetRaidDifficultyID() const { return m_raidDifficulty; }
        Difficulty GetLegacyRaidDifficultyID() const { return m_legacyRaidDifficulty; }
        void SetDungeonDifficultyID(Difficulty dungeon_difficulty) { m_dungeonDifficulty = dungeon_difficulty; }
        void SetRaidDifficultyID(Difficulty raid_difficulty) { m_raidDifficulty = raid_difficulty; }
        void SetLegacyRaidDifficultyID(Difficulty raid_difficulty) { m_legacyRaidDifficulty = raid_difficulty; }
        static Difficulty CheckLoadedDungeonDifficultyID(Difficulty difficulty);
        static Difficulty CheckLoadedRaidDifficultyID(Difficulty difficulty);
        static Difficulty CheckLoadedLegacyRaidDifficultyID(Difficulty difficulty);
        void SendRaidGroupOnlyMessage(RaidGroupReason reason, int32 delay) const;

        bool UpdateSkill(uint32 skill_id, uint32 step);
        bool UpdateSkillPro(uint16 SkillId, int32 Chance, uint32 step);

        bool UpdateCraftSkill(uint32 spellid);
        bool UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator = 1);
        bool UpdateFishingSkill();

        float GetHealthBonusFromStamina();

        void UpdateStatsByMask();
        bool UpdateStats(Stats stat) override;
        float GetTotalStatValue(Stats stat) override;
        float GetPvpStatScalar() const;
        uint32 GetExtraPvpStatSpells(uint16 specID) const;
        float GetPvpArmorTemplate(uint16 specID) const;
        float GetPvpDamageTemplate(uint16 specID) const;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateMaxHealth() override;
        void UpdateMaxPower(Powers power) override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void UpdateDamagePhysical(WeaponAttackType attType) override;
        void ApplySpellPowerBonus(int32 amount, bool apply);
        void UpdateSpellDamageAndHealingBonus();
        void ApplyRatingMod(CombatRating cr, int32 value, bool apply);
        void UpdateRating(CombatRating cr);
        void UpdateItemLevels();

        void UpdateCastHastMods(float auraMods);
        void UpdateMeleeHastMod(float auraMods);
        void UpdateHastMod(float auraMods);
        void UpdateRangeHastMod(float auraMods);
        void UpdateHastRegen(float auraMods);
        void UpdateHast();

        void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& min_damage, float& max_damage);

        float GetRatingBonusValue(CombatRating cr) const;
        uint32 GetBaseSpellPowerBonus() const { return m_baseSpellPower; }
        int32 GetSpellPenetrationItemMod() const { return m_spellPenetrationItemMod; }
        float GetExpertiseDodgeOrParryReduction(WeaponAttackType attType) const;

        void UpdateBlockPercentage();
        void UpdateCritPercentage(WeaponAttackType attType);
        void UpdateAllCritPercentages();
        void UpdateParryPercentage();
        void UpdateDodgePercentage();
        void UpdateMeleeHitChances();
        void UpdateRangedHitChances();
        void UpdateSpellHitChances();
        void UpdateShieldBlock();

        void HandleArenaDeserter();

        void SendUpdateStat(uint32 updateStatMask); // for example: USM_AGILITY
        void SendUpdateCR(int32 updateCRMask);
        void SendOperationsAfterDelay(uint32 mask);

        void UpdateSpellCritChance();
        void UpdateExpertise();
        void ApplyManaRegenBonus(int32 amount, bool apply);
        void ApplyHealthRegenBonus(int32 amount, bool apply);
        void UpdateMasteryAuras();
        void UpdateVersality(CombatRating cr = CR_AMPLIFY);
        void UpdateCRSpeed();
        void UpdateLifesteal();
        void UpdateAvoidance();

        void SetLootSpecID(uint32 spec) { SetUInt32Value(PLAYER_FIELD_LOOT_SPEC_ID, spec); }
        uint32 GetFieldLootSpecID() const { return GetUInt32Value(PLAYER_FIELD_LOOT_SPEC_ID); }
        uint32 GetLootSpecID() const;
        uint32 GetDefaultLootSpecID() const;
        ObjectGuid GetLootGUID() const { return GetGuidValue(PLAYER_FIELD_LOOT_TARGET_GUID); }
        void SetLootGUID(ObjectGuid guid) { SetGuidValue(PLAYER_FIELD_LOOT_TARGET_GUID, guid); }

        void RemovedInsignia(Player* looterPlr);

        WorldSession* GetSession() const { return m_session; }

        void BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const override;
        void DestroyForPlayer(Player* target) const override;
        void SendLogXPGain(uint32 GivenXP, Unit* victim, uint32 BonusXP, bool recruitAFriend = false, float group_rate=1.0f);

        // notifiers
        void SendAttackSwingError(AttackSwingReason error);
        void SendCancelAutoRepeat(Unit* target);
        void SendExplorationExperience(uint32 Area, uint32 Experience);

        void SendDungeonDifficulty(int32 forcedDifficulty = -1);
        void SendRaidDifficulty(bool Legacy, int32 forcedDifficulty = -1);
        void ResetInstances(uint8 method, bool isRaid, bool isLegacy);
        void SendResetInstanceSuccess(uint32 MapId);
        void SendResetInstanceFailed(ResetFailedReason reason, uint32 MapId) const;
        void SendResetFailedNotify();

        bool UpdatePosition(float x, float y, float z, float orientation, bool teleport = false, bool stop = false) override;
        bool UpdatePosition(const Position &pos, bool teleport = false) override { return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport); }
        void UpdateUnderwaterState(Map* m, float x, float y, float z) override;

        void SendMessageToSet(WorldPacket const* data, bool self, GuidUnorderedSet const& ignoredList = GuidUnorderedSet()) override {SendMessageToSetInRange(data, GetVisibilityRange(), self, ignoredList); }
        void SendMessageToSetInRange(WorldPacket const* data, float fist, bool self, GuidUnorderedSet const& ignoredList = GuidUnorderedSet()) override;
        void SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool own_team_only, GuidUnorderedSet const& ignoredList = GuidUnorderedSet());
        void SendMessageToSet(WorldPacket const* data, Player const* skipped_rcvr, GuidUnorderedSet const& ignoredList = GuidUnorderedSet()) override;
        void SendChatMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool own_team_only, GuidUnorderedSet const& ignoredList = GuidUnorderedSet());

        Corpse* GetCorpse() const;
        void SpawnCorpseBones();
        void CreateCorpse();
        void KillPlayer();
        void InitializeSelfResurrectionSpells();
        void ResurrectPlayer(float restore_percent, bool applySickness = false);
        void BuildPlayerRepop();
        void RepopAtGraveyard(bool outInstance = false);

        void DurabilityLossAll(double percent, bool inventory, bool withMods);
        void DurabilityLoss(Item* item, double percent);
        void DurabilityPointsLossAll(int32 points, bool inventory);
        void DurabilityPointsLoss(Item* item, int32 points);
        void DurabilityPointLossForEquipSlot(EquipmentSlots slot);
        uint32 DurabilityRepairAll(bool cost, float discountMod, bool guildBank);
        uint32 DurabilityRepair(uint16 pos, bool cost, float discountMod, bool guildBank);

        void UpdateMirrorTimers();
        void StopMirrorTimers();
        bool IsMirrorTimerActive(MirrorTimerType type);

        bool CanJoinConstantChannelInZone(ChatChannelsEntry const* channel, AreaTableEntry const* zone);

        void JoinedChannel(Channel* c);
        void LeftChannel(Channel* c);
        void CleanupChannels();
        void UpdateLocalChannels(uint32 newZone);
        void LeaveLFGChannel();

        void SetSkill(uint16 id, uint16 step = 0, uint16 currVal = 0, uint16 maxVal = 0);
        uint16 GetMaxSkillValue(uint32 skill);        // max + perm. bonus + temp bonus
        uint16 GetPureMaxSkillValue(uint32 skill);    // max
        uint16 GetSkillValue(uint32 skill);           // skill value + perm. bonus + temp bonus
        uint16 GetBaseSkillValue(uint32 skill);       // skill value + perm. bonus
        uint16 GetPureSkillValue(uint32 skill);       // skill value
        int16 GetSkillPermBonusValue(uint32 skill);
        int16 GetSkillTempBonusValue(uint32 skill);
        uint16 GetSkillStep(uint16 skill);            // 0...6
        bool HasSkill(uint32 skill);
        void learnSkillRewardedSpells(uint32 id, uint32 value);

        Map* GetTeleportMap() { return m_teleport_target_map; }
        void ResetTeleMap() { m_teleport_target_map = nullptr; }
        WorldLocation& GetTeleportDest() { return m_teleport_dest; }
        uint32 GetTeleportOptions() { return m_teleport_options; }
        bool IsBeingTeleported() const { return mSemaphoreTeleport_Near || mSemaphoreTeleport_Far; }
        bool IsBeingTeleportedNear() const { return mSemaphoreTeleport_Near; }
        bool IsBeingTeleportedFar() const { return mSemaphoreTeleport_Far; }
        bool IsChangeMap() const { return m_change_map; }
        bool IsBeingTeleportedSeamlessly() const { return IsBeingTeleportedFar() && m_teleport_options & TELE_TO_SEAMLESS; }
        void SetSemaphoreTeleportNear(bool semphsetting) { mSemaphoreTeleport_Near = semphsetting; }
        void SetSemaphoreTeleportFar(bool semphsetting) { mSemaphoreTeleport_Far = semphsetting; }
        void SetChangeMap(bool change) { m_change_map = change; }
        void SetRemoveFromMap(bool state) { m_removeFromMap = state; }
        bool IsRemoveFromMap() const { return m_removeFromMap; }
        void ProcessDelayedOperations();

        bool HasAreaExplored(uint32 AreaID);
        void CheckAreaExploreAndOutdoor();

        static uint32 TeamForRace(uint8 race);
        uint32 GetTeam() const { return m_team; }
        TeamId GetTeamId() const { if (m_team == ALLIANCE) return TEAM_ALLIANCE; if (m_team == HORDE) return TEAM_HORDE; return TEAM_NEUTRAL; }
        void setFactionForRace(uint8 race);

        void InitDisplayIds();

        bool IsAtGroupRewardDistance(WorldObject const* pRewardSource) const;
        bool IsAtRecruitAFriendDistance(WorldObject const* pOther) const;
        void RewardPlayerAndGroupAtKill(Unit* victim, bool isBattleGround);
        void RewardPlayerAndGroupAtEvent(uint32 creature_id, WorldObject* pRewardSource);
        bool isHonorOrXPTarget(Unit* victim);

        float GetsAFriendBonus(bool forXP);
        uint8 GetGrantableLevels() { return m_grantableLevels; }
        void SetGrantableLevels(uint8 val) { m_grantableLevels = val; }

        ReputationMgr&       GetReputationMgr()       { return m_reputationMgr; }
        ReputationMgr const& GetReputationMgr() const { return m_reputationMgr; }
        ReputationRank GetReputationRank(uint32 faction_id) const;
        void RewardReputation(Unit* victim, float rate, bool killer = true);
        void RewardReputation(Quest const* quest);
        void RewardGuildReputation(Quest const* quest);

        void ModifySkillBonus(uint32 skillid, int32 val, bool talent);

        void ResetLootCooldown();

        /*********************************************************/
        /***                  PVP SYSTEM                       ***/
        /*********************************************************/
        void UpdateHonorFields(bool loading = false);
        bool RewardHonor(Unit* victim, uint32 groupsize, int32 honor = -1, bool pvptoken = false);
        uint32 GetMaxPersonalArenaRatingRequirement(uint8 minarenaslot) const;

        //End of PvP System

        SpellCooldowns* GetSpellCooldowns() { return &m_spellCooldowns; }
        SpellChargeDataMap* GetSpellChargeDatas() { return &m_spellChargeData; }

        uint8 GetSkinValue() const { return GetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_SKIN_ID); }
        uint8 GetFaceValue() const { return GetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_FACE_ID); }
        uint8 GetHairStyleValue() const { return GetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_HAIR_STYLE_ID); }
        uint8 GetHairColorValue() const { return GetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_HAIR_COLOR_ID); }

        uint8 GetTatooValue() const { return GetByteValue(PLAYER_FIELD_BYTES_2, PLAYER_BYTES_2_TATOO); }
        uint8 GetHornValue() const { return GetByteValue(PLAYER_FIELD_BYTES_2, PLAYER_BYTES_2_HORN); }
        uint8 GetBlindFoldValue() const { return GetByteValue(PLAYER_FIELD_BYTES_2, PLAYER_BYTES_2_OFFSET_BLIND_FOLD); }
        uint8 GetFacialStyleValue() const { return GetByteValue(PLAYER_FIELD_BYTES_2, PLAYER_BYTES_2_OFFSET_FACIAL_STYLE); }

        uint8 GetPartyTypeValue() const { return GetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_PARTY_TYPE); }
        uint8 GetBankBagSlotsValue() const { return GetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_BANK_BAG_SLOTS); }
        uint8 GetGenderValue() const { return GetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER); }
        uint8 GetDrunkValue() const { return GetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_INEBRIATION); }

        uint8 GetRafGrantableLevelValue() const { return GetByteValue(PLAYER_FIELD_BYTES_4, PLAYER_BYTES_4_RAF_GRANTABLE_LEVEL); }
        uint8 GetActionBarTogglesValue() const { return GetByteValue(PLAYER_FIELD_BYTES_4, PLAYER_BYTES_4_ACTION_BAR_TOGGLES); }
        uint8 GetLifetimeMaxPvPRankValue() const { return GetByteValue(PLAYER_FIELD_BYTES_4, PLAYER_BYTES_4_LIFETIME_MAX_PVP_RANK); }

        uint8 GetAuraVisionValue() const { return GetByteValue(PLAYER_FIELD_BYTES_7, PLAYER_BYTES_5_AURA_VISION); }
        uint8 GetOverrideSpellsValue() const { return GetByteValue(PLAYER_FIELD_BYTES_5, PLAYER_BYTES_5_OVERRIDE_SPELLS_ID); }

        void SetDrunkValue(uint8 newDrunkValue, uint32 itemId = 0);

        static DrunkenState GetDrunkenstateByValue(uint8 value);

        uint32 GetDeathTimer() const { return m_deathTimer; }
        uint32 GetCorpseReclaimDelay(bool pvp) const;
        void UpdateCorpseReclaimDelay();
        void SendCorpseReclaimDelay(bool load = false);

        uint32 GetBlockPercent() override { return GetUInt32Value(PLAYER_FIELD_SHIELD_BLOCK); }
        bool CanParry() const { return m_canParry; }
        void SetCanParry(bool value);
        bool CanBlock() const { return m_canBlock; }
        void SetCanBlock(bool value);
        bool CanTitanGrip() const { return m_canTitanGrip; }
        void SetCanTitanGrip(bool value) { m_canTitanGrip = value; }
        bool CanTameExoticPets() const { return isGameMaster() || HasAuraType(SPELL_AURA_ALLOW_TAME_PET_TYPE); }

        void SetRegularAttackTime();
        void SetBaseModValue(BaseModGroup modGroup, BaseModType modType, float value) { m_auraBaseMod[modGroup][modType] = value; }
        void HandleBaseModValue(BaseModGroup modGroup, BaseModType modType, float amount, bool apply);
        float GetBaseModValue(BaseModGroup modGroup, BaseModType modType) const;
        float GetTotalBaseModValue(BaseModGroup modGroup) const;
        float GetTotalPercentageModValue(BaseModGroup modGroup) const { return m_auraBaseMod[modGroup][FLAT_MOD] + m_auraBaseMod[modGroup][PCT_MOD]; }
        void _ApplyAllStatBonuses();
        void _RemoveAllStatBonuses();

        void ResetAllPowers();

        void _ApplyOrRemoveItemEquipDependentAuras(ObjectGuid const& itemGUID = ObjectGuid::Empty, bool apply = true);
        bool CheckItemEquipDependentSpell(SpellInfo const* spellInfo = nullptr, ObjectGuid const& itemGUID = ObjectGuid::Empty);
        bool CheckItemForArmorSpecialization(SpellInfo const* spellInfo) const;

        void _ApplyItemMods(Item* item, uint8 slot, bool apply);
        void _RemoveAllItemMods();
        void _ApplyAllItemMods();
        void RescaleAllItemsIfNeeded(bool keepHPPct = false);
        void _ApplyItemBonuses(Item* item, uint8 slot, bool apply);
        void _ApplyWeaponDamage(uint8 slot, Item* item, bool apply);
        bool EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot);
        void ToggleMetaGemsActive(uint8 exceptslot, bool apply);
        void CorrectMetaGemEnchants(uint8 slot, bool apply);
        void InitDataForForm(bool reapplyMods = false);

        void ApplyItemEquipSpell(Item* item, bool apply, bool form_change = false);
        void ApplyEquipSpell(SpellInfo const* spellInfo, Item* item, bool apply, bool form_change = false);
        void UpdateEquipSpellsAtFormChange();
        void ApplyArtifactPowers(Item* item, bool apply);
        void ApplyArtifactPowerRank(Item* artifact, ArtifactPowerRankEntry const* artifactPowerRank, bool apply);
        void CastItemCombatSpell(Unit* target, WeaponAttackType attType, uint32 procVictim, uint32 procEx);
        void CastItemUseSpell(Item* item, SpellCastTargets const& targets, int32* misc, ObjectGuid SpellGuid);
        void CastItemCombatSpell(Unit* target, WeaponAttackType attType, uint32 procVictim, uint32 procEx, Item* item, ItemTemplate const* proto);

        void SendEquipmentSetList();
        void SetEquipmentSet(EquipmentSetInfo::EquipmentSetData const& newEqSet);
        void DeleteEquipmentSet(uint64 setGuid);

        void SendInitWorldStates(uint32 zone, uint32 area);
        void SendUpdateWorldState(uint32 variableID, uint32 value, bool hidden = false);

        time_t const& GetLastWorldStateUpdateTime() { return m_lastWSUpdateTime; };
        void SetLastWorldStateUpdateTime(time_t _time) { m_lastWSUpdateTime = _time; };
        
        void SendDirectMessage(WorldPacket const* data) const;

        void SendAurasForTarget(Unit* target);
        void SendSpellHistoryData();
        void SendSpellChargeData();
        void SendCategoryCooldownMods();
        void SendModifyCooldown(uint32 spellId, int32 value);

        PlayerMenu* PlayerTalkClass;
        std::vector<ItemSetEffect*>* ItemSetEff;

        void SendLoot(ObjectGuid guid, LootType loot_type, bool AoeLoot = false, uint8 pool = 0);
        void SendLootRelease(ObjectGuid guid, ObjectGuid lguid = ObjectGuid::Empty);
        void SendNotifyLootItemRemoved(uint8 lootSlot, Loot* loot);
        void SendNotifyLootMoneyRemoved(Loot* loot);
        void SendLootError(ObjectGuid guid, ObjectGuid lGuid, LootError error);

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        bool InBattleground() const;
        bool InArena() const;
        bool HasTournamentRules() const;
        bool InRBG() const;
        uint32 GetBattlegroundId()  const;
        uint16 GetBattlegroundTypeId() const;
        Battleground* GetBattleground() const;
        bool InTournaments() const;

        void SetCrowdControlSpellId(uint32 spellId);
        uint32 GetCrowdControlSpellId() const;

        void SendMessageToPlayer(const char *format);

        uint32 GetBattlegroundQueueJoinTime(uint8 bgQueueTypeId) const;
        bool InBattlegroundQueue() const;
        uint8 GetBattlegroundQueueTypeId(uint32 index) const;
        uint32 GetBattlegroundQueueIndex(uint8 bgQueueTypeId) const;
        bool IsInvitedForBattlegroundQueueType(uint8 bgQueueTypeId) const;
        bool InBattlegroundQueueForBattlegroundQueueType(uint8 bgQueueTypeId) const;
        void SetBattlegroundId(uint32 val, uint16 bgTypeId);
        uint32 AddBattlegroundQueueId(uint8 val);
        bool HasFreeBattlegroundQueueId();
        void RemoveBattlegroundQueueId(uint8 val);
        void SetInviteForBattlegroundQueueType(uint8 bgQueueTypeId, uint32 instanceId);
        bool IsInvitedForBattlegroundInstance(uint32 instanceId) const;
        WorldLocation const& GetBattlegroundEntryPoint() const;
        void SetBattlegroundEntryPoint();
        void SetBGTeam(uint32 team);
        uint32 GetBGTeam() const;
        TeamId GetBGTeamId() const;
        uint16 GetLastActiveSpec(bool lfgOrBg = false) const;
        void SaveLastSpecialization(bool lfgOrBg = false);

        void LeaveBattleground(bool teleportToEntryPoint = true);
        bool CanJoinToBattleground(uint8 pvpIternalType) const;
        bool CanReportAfkDueToLimit();
        void ReportedAfkBy(Player* reporter);
        void ClearAfkReports() { m_bgData.BgAfkReporter.clear(); }

        bool GetBGAccessByLevel(uint16 bgTypeId) const;
        bool CanUseBattlegroundObject();
        bool isTotalImmune();
        bool CanCaptureTowerPoint();

        bool HasWinToday(uint8 type) { return _hasWinToday[type]; }
        void SetWinToday(bool isWinner, uint8 type = 0, bool all = true);
        
        void ModifyDeathMatchStats(uint32 kills, uint32 deaths, uint64 damage, int32 rating, uint32 totalKills, uint32 matches = 1);
        DeathMatchScore* getDeathMatchScore() { return &dmScore; }
        /*********************************************************/
        /***               OUTDOOR PVP SYSTEM                  ***/
        /*********************************************************/

        // returns true if the player is in active state for outdoor pvp objective capturing, false otherwise
        bool IsOutdoorPvPActive();

        /*********************************************************/
        /***                    REST SYSTEM                    ***/
        /*********************************************************/

        bool isRested() const { return GetRestTime() >= 10*IN_MILLISECONDS; }
        uint32 GetXPRestBonus(uint32 xp);
        uint32 GetRestTime() const { return m_restTime;}
        void SetRestTime(uint32 v) { m_restTime = v;}

        /*********************************************************/
        /***              ENVIROMENTAL SYSTEM                  ***/
        /*********************************************************/

        bool IsImmuneToEnvironmentalDamage();
        uint32 EnvironmentalDamage(EnviromentalDamage type, uint32 damage);

        /*********************************************************/
        /***               FLOOD FILTER SYSTEM                 ***/
        /*********************************************************/

        void UpdateSpeakTime();
        bool CanSpeak() const;
          
        std::string lastLfgPhrase{};

        /// Anticheat
        PlayerCheatData* _cheatData;
        PlayerCheatData* GetCheatData() const { return _cheatData; }
        void OnDisconnected();
        void RelocateToLastClientPosition();
        void GetSafePosition(float &x, float &y, float &z, Transport* onTransport = NULL) const;

        std::string GetShortDescription() const;

        // Anti undermap
        void SaveNoUndermapPosition(float x, float y, float z);
        bool UndermapRecall();
        float _lastSafeX;
        float _lastSafeY;
        float _lastSafeZ;
        bool  _undermapPosValid;

        // knockback/jumping states
        bool IsLaunched() { return launched; }
        void SetLaunched(bool apply) { launched = apply; }
        float GetXYSpeed() { return xy_speed; }
        void SetXYSpeed(float speed) { xy_speed = speed; }

        // knockback/jumping states
        bool launched;
        // not null only is player has knockback state
        float xy_speed;

        /*********************************************************/
        /***                 VARIOUS SYSTEMS                   ***/
        /*********************************************************/
        Unit* m_mover;
        WorldObject* m_seer;

        void UpdateFallInformationIfNeed(MovementInfo const& minfo, uint16 opcode);
        void SetFallInformation(uint32 time, float z);
        void HandleFall(MovementInfo const& movementInfo);

        void SetClientControl(Unit* target, bool allowMove);

        void SetMover(Unit* target);

        void SetSeer(WorldObject* target) { m_seer = target; }
        void SetViewpoint(WorldObject* target, bool apply);
        WorldObject* GetViewpoint() const;
        void StopCastingCharm();
        void StopCastingBindSight();

        void SendPetTameResult(PetTameResult result);

        uint32 GetSaveTimer() const { return m_nextSave; }
        void   SetSaveTimer(uint32 timer) { m_nextSave = timer; }

        // Recall position
        WorldLocation m_recallLoc;
        void   SaveRecallPosition();

        void SetHomebind(WorldLocation const& loc, uint32 area_id);
        void SendBindPointUpdate();

        // Homebind coordinates
        uint32 m_homebindMapId;
        uint16 m_homebindAreaId;
        float m_homebindX;
        float m_homebindY;
        float m_homebindZ;

        std::set<uint32> m_profSpells[MAX_GUILD_PROFESSIONS];

        WorldLocation GetStartPosition() const;

        // current pet slot
        uint32 m_currentPetNumber;
        PetSlot m_currentSummonedSlot;

        void setPetSlot(PetSlot slot, uint32 petID);
        int16 SetOnAnyFreeSlot(uint32 petID);
        void cleanPetSlotForMove(PetSlot slot, uint32 petID);
        void SwapPetSlot(PetSlot oldSlot, PetSlot newSlot);
        uint32 getPetIdBySlot(uint32 slot) const { return m_PetSlots[slot]; }
        const PlayerPetSlotList &GetPetSlotList() { return m_PetSlots; }
        PetSlot getSlotForNewPet(bool full = false);
        PetSlot GetSlotForPetId(uint32 petID);
        PetSlot GetMaxCurentPetSlot() const;

        uint8 GetClassFamily() const;

        bool CanSummonPet(uint32 entry) const;
        // currently visible objects at player client
        GuidSet m_clientGUIDs;
        GuidSet m_extraLookList;
        sf::contention_free_shared_mutex< > i_clientGUIDLock;
        std::recursive_mutex i_killMapLock;
        std::recursive_mutex i_personalLootLock;

        bool HaveAtClient(WorldObject const* u);
        void AddClient(ObjectGuid guid);
        void RemoveClient(ObjectGuid guid);
        GuidSet& GetClient();
        void ClearClient();

        // some hack :( now impossible implemented correct build of object update packet
        void SendForceUpdateToClient();

        ///! Extra look method not alow remove some creatures from player visibility by grid VisibleNotifier
        void AddToExtraLook(ObjectGuid guid) { m_extraLookList.insert(guid); } 
        void RemoveFromExtraLook(ObjectGuid guid) { m_extraLookList.erase(guid); } 
        bool HaveExtraLook(ObjectGuid guid) const { return m_extraLookList.find(guid) != m_extraLookList.end(); }

        bool IsNeverVisible(WorldObject const* seer = nullptr) const override;

        bool IsVisibleGloballyFor(Player const* player) const;

        void SendInitialVisiblePackets(Unit* target);
        void UpdateVisibilityForPlayer();
        void UpdateVisibilityOf(WorldObject* target);
        void UpdateTriggerVisibility();
        void UpdateCustomField();

        template<class T>
        void UpdateVisibilityOf(T* target, UpdateData& data, std::set<Unit*>& visibleNow);

        bool IsPlayerLootCooldown(uint32 entry, uint8 type = 0, uint8 diff = 0);
        void AddPlayerLootCooldown(uint32 entry, uint32 guid, uint8 type = 0, bool respawn = true, uint8 diff = 0);

        bool IsLfgCooldown(uint32 dungeonId);
        void AddLfgCooldown(uint32 dungeonId);

        uint8 m_forced_speed_changes[MAX_MOVE_TYPE];

        bool HasAtLoginFlag(AtLoginFlags f) const { return (m_atLoginFlags & f) != 0; }
        void SetAtLoginFlag(AtLoginFlags f) { m_atLoginFlags |= f; }
        void RemoveAtLoginFlag(AtLoginFlags flags, bool persist = false);

        bool isUsingLfg();
        bool CanKickFromChallenge();
        void SetLfgBonusFaction(uint32 factionId);
        uint32 GetLfgBonusFaction() const;

        typedef std::set<uint32> DFQuestsDoneList;
        DFQuestsDoneList m_DFQuests;

        // Temporarily removed pet cache
        uint32 GetTemporaryUnsummonedPetNumber() const { return m_temporaryUnsummonedPetNumber; }
        void SetTemporaryUnsummonedPetNumber(uint32 petnumber) { m_temporaryUnsummonedPetNumber = petnumber; }
        void UnsummonPetTemporaryIfAny();
        void ResummonPetTemporaryUnSummonedIfAny();
        bool IsPetNeedBeTemporaryUnsummoned() const { return !IsInWorld() || !isAlive() || IsMounted() /*+in flight*/; }

        void SendCinematicStart(uint32 CinematicSequenceId);
        void SendMovieStart(uint32 MovieId);

        uint32 realmTransferid;
        void SetTransferId(uint32 Id) { realmTransferid = Id; }
        uint32 GetTransferId() { return realmTransferid; }

        /*********************************************************/
        /***                 INSTANCE SYSTEM                   ***/
        /*********************************************************/

        typedef std::unordered_map< uint32 /*mapId*/, InstancePlayerBind > BoundInstancesMap;

        void UpdateHomebindTime(uint32 time);

        uint32 m_HomebindTimer;
        bool m_InstanceValid;
        // permanent binds and solo binds by difficulty
        BoundInstancesMap m_boundInstances[MAX_BOUND];
        InstancePlayerBind* GetBoundInstance(uint32 mapid, Difficulty difficulty);
        BoundInstancesMap& GetBoundInstances(Difficulty difficulty);
        InstanceSave* GetInstanceSave(uint32 mapid);
        void UnbindInstance(uint32 mapid, Difficulty difficulty, bool unload = false);
        void UnbindInstance(BoundInstancesMap::iterator &itr, Difficulty difficulty, bool unload = false);
        InstancePlayerBind* BindToInstance(InstanceSave* save, bool permanent, bool load = false);
        void BindToInstance();
        void SetPendingBind(uint32 instanceId, uint32 bindTimer) { _pendingBindId = instanceId; _pendingBindTimer = bindTimer; }
        bool HasPendingBind() const { return _pendingBindId > 0; }
        void SendRaidInfo();
        void SendSavedInstances();
        static void ConvertInstancesToGroup(Player* player, Group* group, bool switchLeader);
        bool Satisfy(AccessRequirement const* ar, uint32 target_map, bool report = false);
        bool CheckInstanceLoginValid();

        PetInfoData* GetPetInfo(uint32 petentry, uint32 petnumber);
        void AddPetInfo(Pet* pet);
        PetInfoDataMap* GetPetInfoData() { return &m_petInfo; }
        void DeletePetInfo(uint32 petnumber) { m_petInfo.erase(petnumber); }

        // last used pet number (for BG's)
        uint32 GetLastPetNumber() const { return m_lastpetnumber; }
        void SetLastPetNumber(uint32 petnumber) { m_lastpetnumber = petnumber; }

        uint32 GetLastPetEntry() const { return m_LastPetEntry; }
        void SetLastPetEntry(uint32 entry) { m_LastPetEntry = entry; }

        uint32 GetEncounterMask(lfg::LFGDungeonData const* dungeonData, lfg::LfgReward const* reward);

        /*********************************************************/
        /***                   GROUP SYSTEM                    ***/
        /*********************************************************/

        Group* GetGroupInvite() { return m_groupInvite; }
        void SetGroupInvite(Group* group) { m_groupInvite = group; }
        Group* GetGroup() { return m_group.getTarget(); }
        const Group* GetGroup() const { return static_cast<const Group*>(m_group.getTarget()); }
        GroupReference& GetGroupRef() { return m_group; }
        void SetGroup(Group* group, int8 subgroup = -1);
        uint8 GetSubGroup() const { return m_group.getSubGroup(); }
        uint32 GetGroupUpdateFlag() const;
        void SetGroupUpdateFlag(uint32 flag);
        void RemoveGroupUpdateFlag(uint32 flag);
        Player* GetNextRandomRaidMember(float radius);
        PartyResult CanUninviteFromGroup() const;
        int32 NextGroupUpdateSequenceNumber(GroupCategory category);
        void SetPartyType(GroupCategory category, uint8 type);
        void ResetGroupUpdateSequenceIfNeeded(Group const* group);

        // Battleground / Battlefield Group System
        void SetBattlegroundOrBattlefieldRaid(Group* group, int8 subgroup = -1);
        void RemoveFromBattlegroundOrBattlefieldRaid();
        Group* GetOriginalGroup() { return m_originalGroup.getTarget(); }
        GroupReference& GetOriginalGroupRef() { return m_originalGroup; }
        uint8 GetOriginalSubGroup() const { return m_originalGroup.getSubGroup(); }
        void SetOriginalGroup(Group* group, int8 subgroup = -1);

        void SetPassOnGroupLoot(bool bPassOnGroupLoot) { m_bPassOnGroupLoot = bPassOnGroupLoot; }
        bool GetPassOnGroupLoot() const { return m_bPassOnGroupLoot; }

        MapReference &GetMapRef() { return m_mapRef; }

        // Set map to player and add reference
        void SetMap(Map* map) override;
        void ResetMap() override;

        bool isAllowedToLoot(const Creature* creature);

        DeclinedName const* GetDeclinedNames() const { return m_declinedname; }
        uint8 GetRunesState() const { return m_runes.RuneState; }
        uint32 GetRuneCooldown(uint8 index) const;
        float GetRuneCooldownCoef() const { return m_runes.CooldownCoef; }
        void SetRuneCooldownCoef(float Coef) { m_runes.CooldownCoef = Coef; }
        void SetRuneCooldown(uint8 index, uint32 cooldown);
        void ModRuneCooldown(uint8 index, uint32 regenTime);
        float GetRuneBaseCooldown() const;
        void ResyncRunes(uint8 count);
        void AddRunePower(uint8 index);
        void InitRunes();

        AchievementPtr GetAchievementMgr() { return m_achievementMgr; }
        AchievementPtr const GetAchievementMgr() const { return m_achievementMgr; }
        void UpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 = 0, uint32 miscValue2 = 0, uint32 miscValue3 = 0, Unit* unit = nullptr, bool ignoreGroup = false);
        bool HasAchieved(uint32 achievementId);
        void CompletedAchievement(AchievementEntry const* entry);
        uint32 GetAchievementPoints() const;
        bool CanUpdateCriteria(uint32 /*criteriaTreeId*/, uint32 /*recursTree*/ = 0) const { return true; }

        bool HasTitle(uint32 bitIndex);
        bool HasTitle(CharTitlesEntry const* title) { return HasTitle(title->MaskID); }
        void SetTitle(CharTitlesEntry const* title, bool lost = false);

        bool canSeeSpellClickOn(Creature const* creature) const;

        uint32 GetChampioningFaction() const { return m_ChampioningFaction; }
        uint32 GetChampioningFactionDungeonLevel() const { return m_ChampioningFactionDungeonLevel; }
        void SetChampioningFaction(uint32 faction, uint32 dungeonLevel = 0)
        {
            m_ChampioningFaction = faction;
            m_ChampioningFactionDungeonLevel = dungeonLevel;
        }
        
        Spell* m_spellModTakingSpell;

        float GetAverageItemLevelEquipped() const;
        float GetAverageItemLevelTotal(bool needCalculate = true, bool onlyFromScan = false) const;
        float GetAverageItemLevelTotalWithOrWithoutPvPBonus(bool pvp) const;

        bool isDebugAreaTriggers;

        void ClearWhisperWhiteList() { WhisperList.clear(); }
        void AddWhisperWhiteList(ObjectGuid guid) { WhisperList.push_back(guid); }
        bool IsInWhisperWhiteList(ObjectGuid guid);

        void ValidateMovementInfo(MovementInfo* mi);
        void SendMovementForce(AreaTrigger const* at, Position pos = Position(), float magnitude = 0.0f, uint32 type = 0, bool apply = false);
        void SendMovementForceAura(ObjectGuid triggerGuid, Position wind, Position center, float magnitude, uint8 type, bool apply);

        void SendMovementSetCollisionHeight(float height, uint8 Reason = 1);

        bool CanFly() const override { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY); }

        //! Return collision height sent to client
        float GetCollisionHeight(bool mounted);

        // Void Storage
        bool IsVoidStorageUnlocked() const;
        void UnlockVoidStorage();
        void LockVoidStorage();
        uint8 GetNextVoidStorageFreeSlot() const;
        uint8 GetNumOfVoidStorageFreeSlots() const;
        uint32 AddVoidStorageItem(VoidStorageItem&& item);
        void DeleteVoidStorageItem(uint8 slot);
        bool SwapVoidStorageItem(uint8 oldSlot, uint8 newSlot);
        VoidStorageItem* GetVoidStorageItem(uint8 slot) const;
        VoidStorageItem* GetVoidStorageItem(uint64 id, uint8& slot) const;

        void SaveCUFProfile(uint8 id, std::nullptr_t);
        void SaveCUFProfile(uint8 id, std::unique_ptr<CUFProfile> profile);
        CUFProfile* GetCUFProfile(uint8 id) const;
        uint8 GetCUFProfilesCount() const;

        uint32 GetLastTargetedGO() { return _lastTargetedGO; }
        void SetLastTargetedGO(uint32 lastTargetedGO) { _lastTargetedGO = lastTargetedGO; }
        void ShowNeutralPlayerFactionSelectUI();

        float GetPersonnalXpRate() { return m_PersonnalXpRate; }
        void SetPersonnalXpRate(float PersonnalXpRate);

        void SetKnockBackTime(uint32 timer) { m_knockBackTimer = timer; }
        uint32 GetKnockBackTime() const { return m_knockBackTimer; }

        void SetQuestUpdate(uint32 quest_id);

        bool CanSpeakLanguage(uint32 lang_id) const;

        uint32 SelectArfiactSpellForSpec(uint32 specID);
        void SendDisplayPlayerChoice(ObjectGuid sender, int32 choiceId);

        Bracket* getBracket(uint8 slot) const;
        std::tuple<uint32, uint32> GetItemDataForRatedQuest(uint32 bracketType);
        void SendPvpRatedStats();
        void PvpRatedQuestReward(uint32 quest_id);
        std::vector<uint32> GetPvPRewardItem(uint32& itemID, uint8 type, uint32 rating, bool elit, uint32 needLevel);
        void GetPvPRatingAndLevel(PvpReward* reward, uint8 type, uint32& rating, uint32& needLevel, bool elit);
        void GetPvPRatingAndLevelOld(PvpReward* reward, uint8 type, uint32& rating, uint32& needLevel, bool elit);

        /*********************************************************/
        /***              GARRISON SYSTEM                      ***/
        /*********************************************************/
        void CreateGarrison(uint32 garrSiteId, bool skip = false);
        void DeleteGarrison();
        Garrison* GetGarrisonPtr() { return _garrison; }
        Garrison* GetGarrison() { return _garrison; }
        bool CheckShipment(CharShipmentEntry const* shipmentEntry);

        /*********************************************************/
        /***              Collection SYSTEM                    ***/
        /*********************************************************/
        CollectionMgr* GetCollectionMgr() const { return _collectionMgr; }
        void AddNonVisibleItemToCollect();
        void UnLockThirdSocketIfNeed(Item* item);
        uint32 GetQuestForUnLockThirdSocket();
        uint32 GetQuestForUnLockSecondTier();

        void ScheduleDelayedOperation(uint32 operation);

        /*********************************************************/
        /***              E.T.C.                               ***/
        /*********************************************************/
        bool isWatchingMovie() const { return m_watching_movie; }
        void setWatchinMovie(bool s) { m_watching_movie = s; }

        const SpellInQueue* GetSpellInQueue() const;
        void HandleSpellInQueue();
        void SetSpellInQueue(uint32 gcdEnd, uint16 recoveryCategory, WorldPackets::Spells::SpellCastRequest* castData);
        void CancelCastSpell(bool spellInQueue, uint32 spellId = 0, uint32 spellVisual = 0, ObjectGuid spellGuid = ObjectGuid::Empty);
        void CastSpellInQueue();

        //
        AreaTriggerEntry const* GetLastAreaTrigger() { return LastAreaTrigger; }
        void SetLastAreaTrigger(AreaTriggerEntry const* at) { LastAreaTrigger = at; }

        /*********************************************************/
        /***              SCENE SYSTEM                         ***/
        /*********************************************************/
        void SendSpellScene(uint32 miscValue, SpellInfo const* spellInfo, bool apply, Position* pos);
        void SceneCompleted(uint32 sceneID);
        bool HasSceneStatus(uint32 sceneID, SceneEventStatus const& status) const;
        void TrigerScene(uint32 instanceID, std::string const Event);

        /*********************************************************/
        /***                  LFG SYSTEM                     ***/
        /*********************************************************/
        void SendLfgUpdatePlayer();
        void SendLfgDisabled();
        void SendLfgOfferContinue(uint32 slot);
        void SendLfgTeleportError(uint8 err);

        void SendCalendarRaidLockout(InstanceSave const* save, bool add);
        void SendCalendarRaidLockoutUpdated(InstanceSave const* save);

        bool IsAdvancedCombatLoggingEnabled() const { return _advancedCombatLoggingEnabled; }
        void SetAdvancedCombatLogging(bool enabled) { _advancedCombatLoggingEnabled = enabled; }

        // client version server check and helpers
        void SendVersionMismatchWarinings();

        uint32 GetGoVisualQuestData(GameObject const* go, uint32 field) const;

        uint16 GetScenarioId() const { return m_scenarioId; }
        void SetScenarioId(uint16 scenarioId) { m_scenarioId = scenarioId; }

        // Adventures.
        uint16 getAdventureQuestID();
        void setAdventureQuestID(uint16 questID);

        int32 GetCommandCooldown() const;
        void ResetCommandCooldown();

        void EnterInTimeWalk(LFGDungeonsEntry const* dbc, bool apply = false);
        void RescaleAllForTimeWalk(uint32 level = 0, uint32 ilevelMax = 0, uint32 ilevelMin = 0);

        bool IsInArmyTraining() const { return GetMapId() == 1626; }
        ArmyTrainingInfo armyTrainingInfo;

        /*********************************************************/
        /***              ANTICHEAT                            ***/
        /*********************************************************/
        void SpecialDamageLogs(Unit* victim, uint32 damage, DamageEffectType damagetype, SpellInfo const* spellProto);
        void SpecialLootLogs(ObjectGuid sourceGuid, uint32 itemEntry);

        void AddUpdateItem(Item* item);
        void RemoveUpdateItem(Item* item);
        void AddDeleteItem(Item* item);
        void RemoveDeleteItem(Item* item);
        void UpdateItem();

        void _LoadMail();

        void DeliveryRewardPack(uint32 rewardPackID);

        void CreateDefaultPet();

    protected:
        // Gamemaster whisper whitelist
        GuidList WhisperList;

        int32 m_contestedPvPTimer;
        int32 m_pvpAuraCheckTimer;
        uint16 m_statsUpdateTimer;
        uint32 m_updateStatsMask;
        uint32 m_operationsAfterDelayMask;
        int32 m_updateCRsMask;
        std::atomic<bool> m_duelLock;

        int32 m_playerCommandCooldown;

        SpellInQueue m_spellInQueue;

        std::unordered_map<uint32, uint32> m_sceneInstanceID;
        uint32 sceneInstanceID = 0;

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        /*
        this is an array of BG queues (BgTypeIDs) in which is player
        */
        struct BgBattlegroundQueueID_Rec
        {
            uint32 invitedToInstance;
            uint32 joinTime;
            uint8 bgQueueTypeId;
        };

        BgBattlegroundQueueID_Rec m_bgBattlegroundQueueID[PLAYER_MAX_BATTLEGROUND_QUEUES];
        BGData                    m_bgData;

        uint32 m_crowdControlSpellId;

        bool _hasWinToday[WIN_TODAY_MAX];

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        //We allow only one timed quest active at the same time. Below can then be simple value instead of set.
        typedef std::set<uint32> QuestSet;
        typedef std::set<uint32> SeasonalQuestSet;
        typedef std::unordered_map<uint32,SeasonalQuestSet> SeasonalEventQuestMap;
        typedef std::unordered_map<uint32, WorldQuestInfo> WorldQuestStatusMap;
        QuestSet m_timedquests;
        QuestSet m_weeklyquests;
        QuestSet m_weeklyquestSaves;
        QuestSet m_dailyquests;
        SeasonalEventQuestMap m_seasonalquests;
        SeasonalEventQuestMap m_seasonalquestSaves;
        WorldQuestStatusMap m_worldquests;

        QuestStatusVector m_QuestStatusVector;
        QuestStatusMap m_QuestStatus;
        QuestStatusSaveMap m_QuestStatusSave;

        RewardedQuestSet m_RewardedQuests;
        RewardedQuestSet m_accuntQuests;
        QuestStatusSaveMap m_RewardedQuestsSave;

        ObjectGuid m_divider;
        uint32 m_ingametime;

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        void _LoadActions(PreparedQueryResult result);
        void _LoadAuras(PreparedQueryResult result, PreparedQueryResult resultEffect, uint32 timediff);
        void _LoadGlyphAuras();
        void _LoadBoundInstances(PreparedQueryResult result);
        void _LoadInventory(PreparedQueryResult result, PreparedQueryResult artifactsResult, uint32 timeDiff);
        void _LoadNotInventory(PreparedQueryResult result, uint32 timeDiff);
        void _LoadVoidStorage(PreparedQueryResult result, PreparedQueryResult resultItem);
        void _LoadMailInit(PreparedQueryResult resultUnread, PreparedQueryResult resultDelivery);
        void _LoadMailedItems(Mail* mail);
        void _LoadQuestStatus(PreparedQueryResult result);
        void _LoadQuestStatusObjectives(PreparedQueryResult result);
        void _LoadQuestStatusRewarded(PreparedQueryResult result);
        void _LoadDailyQuestStatus(PreparedQueryResult result);
        void _LoadWeeklyQuestStatus(PreparedQueryResult result);
        void _LoadSeasonalQuestStatus(PreparedQueryResult result);
        void _LoadAdventureQuestStatus(PreparedQueryResult result);
        void _LoadAccountQuest(PreparedQueryResult result);
        void _LoadRandomBGStatus(PreparedQueryResult result);
        void _LoadGroup(PreparedQueryResult result);
        void _LoadSkills(PreparedQueryResult result);
        void _LoadSpells(PreparedQueryResult result);
        void _LoadSpellRewards();
        bool _LoadHomeBind(PreparedQueryResult result);
        void _LoadDeclinedNames(PreparedQueryResult result);
        void _LoadEquipmentSets(PreparedQueryResult result);
        void _LoadTransmogOutfits(PreparedQueryResult result);
        void _LoadBGData(PreparedQueryResult result);
        void _LoadGlyphs(PreparedQueryResult result);
        void _LoadTalents(PreparedQueryResult result, PreparedQueryResult result2);
        void _LoadCurrency(PreparedQueryResult result);
        void _LoadCUFProfiles(PreparedQueryResult result);
        void _LoadHonor(PreparedQueryResult result, PreparedQueryResult result2);
        void _LoadLootCooldown(PreparedQueryResult result);
        void _LoadPetData(PreparedQueryResult result);
        void _LoadWorldQuestStatus(PreparedQueryResult result);
        void _LoadChallengeKey(PreparedQueryResult result);
        void _LoadAccountProgress(PreparedQueryResult result);
        void _LoadLfgCooldown(PreparedQueryResult result);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void _SaveActions(SQLTransaction& trans);
        void _SaveAuras(SQLTransaction& trans);
        void _SaveInventory(SQLTransaction& trans);
        void _SaveVoidStorage(SQLTransaction& trans);
        void _SaveMail(SQLTransaction& trans);
        void _SaveQuestStatus(SQLTransaction& trans);
        void _SaveDailyQuestStatus(SQLTransaction& trans);
        void _SaveWeeklyQuestStatus(SQLTransaction& trans);
        void _SaveSeasonalQuestStatus(SQLTransaction& trans);
        void _SaveAdventureQuest(SQLTransaction& trans);
        void _SaveSkills(SQLTransaction& trans);
        void _SaveSpells(SQLTransaction& trans);
        void _SaveEquipmentSets(SQLTransaction& trans);
        void _SaveBGData(SQLTransaction& trans);
        void _SaveGlyphs(SQLTransaction& trans);
        void _SaveTalents(SQLTransaction& trans);
        void _SaveCurrency(SQLTransaction& trans);
        void _SaveBrackets(SQLTransaction& trans);
        void _SaveCUFProfiles(SQLTransaction& trans);
        void _SaveHonor();
        void _SaveLootCooldown(SQLTransaction& trans);
        void _SaveWorldQuestStatus(SQLTransaction& trans);
        void _SaveChallengeKey(SQLTransaction& trans);
        void _SaveDeathMatchStats(SQLTransaction& trans);
        void _SaveChatLogos(SQLTransaction& trans);
        void _SaveArmyTrainingInfo(SQLTransaction& trans);
        void _SaveAccountProgress(SQLTransaction& trans);
        void _SaveLfgCooldown(SQLTransaction& trans);

        /*********************************************************/
        /***              ENVIRONMENTAL SYSTEM                 ***/
        /*********************************************************/
        void HandleSobering();
        void SendMirrorTimer(MirrorTimerType type, uint32 maxValue, uint32 currentValue, int32 regen);
        void StopMirrorTimer(MirrorTimerType Type);
        void HandleDrowning(uint32 time_diff);
        int32 getMaxTimer(MirrorTimerType timer);

        /*********************************************************/
        /***                  HONOR SYSTEM                     ***/
        /*********************************************************/
        time_t m_lastHonorUpdateTime;

        void outDebugValues() const;

        uint32 m_team;
        uint32 m_nextSave;
        time_t m_speakTime;
        uint32 m_speakCount;
        Difficulty m_dungeonDifficulty;
        Difficulty m_raidDifficulty;
        Difficulty m_legacyRaidDifficulty;
        Difficulty m_prevMapDifficulty;

        uint32 m_atLoginFlags;

        Item* m_items[PLAYER_SLOTS_COUNT];
        uint32 m_currentBuybackSlot;

        PlayerCurrenciesMap _currencyStorage;

        VoidStorageItem* _voidStorageItems[VOID_STORAGE_MAX_SLOT];
        std::array<std::unique_ptr<CUFProfile>, MAX_CUF_PROFILES> _CUFProfiles;

        std::set<Item*> m_itemUpdateQueue;
        bool m_itemUpdateQueueBlocked;

        std::set<Item*> m_itemUpdate;
        std::set<Item*> m_itemDelete;
        std::recursive_mutex i_itemUpdate_lock;

        uint32 m_ExtraFlags;
        ObjectGuid m_curSelection;
        ObjectGuid m_lastSelection;
        DigSiteInfo m_digsite;

        SkillStatusMap mSkillStatus;
        SkillStatusVector mSkillStatusVector;
        SkillSpellCount mSkillSpellCount;

        ObjectGuid::LowType m_GuildIdInvited;
        ObjectGuid m_GuildInviterGuid;

        PlayerMails m_mail;
        PlayerSpellMap m_spells;
        std::map<uint32 /*overridenSpellId*/, std::unordered_set<uint32> /*newSpellId*/> m_overrideSpells;
        std::map<uint32 /*overridenSpellId*/, std::unordered_set<uint32> /*newSpellId*/> m_spellOverrides;
        ItemSpellList m_itemSpellList;
        ExcludeCasterSpellList m_excludeCasterSpellList;
        CasterAuraStateSpellList m_casterAuraStateSSpellList;
        uint32 m_lastPotionId;                              // last used health/mana potion in combat, that block next potion use

        SpecializationInfo _specializationInfo;

        ActionButtonList m_actionButtons;

        float m_auraBaseMod[BASEMOD_END][MOD_END];
        uint32 m_baseSpellPower;
        uint32 m_baseManaRegen;
        uint32 m_baseHealthRegen;
        int32 m_spellPenetrationItemMod;

        SpellModList m_spellMods[MAX_SPELLMOD];

        EnchantDurationList m_enchantDuration;
        ItemDurationList m_itemDuration;
        GuidSet m_itemSoulboundTradeable;

        void ResetTimeSync();
        void SendTimeSync();

        ResurrectionData* _resurrectionData;

        WorldSession* m_session;

        typedef std::list<Channel*> JoinedChannelsList;
        JoinedChannelsList m_channels;

        uint8 m_cinematic;

        TradeData* m_trade;

        bool   m_DailyQuestChanged;
        bool   m_WeeklyQuestChanged;
        bool   m_SeasonalQuestChanged;
        bool   m_AdventureQuestChanged;
        time_t m_lastDailyQuestTime;

        uint32 m_drunkTimer;
        uint32 m_weaponChangeTimer;

        uint32 m_deathTimer;
        time_t m_deathExpireTime;

        time_t m_lastWSUpdateTime;

        uint32 m_restTime;

        uint32 m_WeaponProficiency;
        uint32 m_ArmorProficiency;
        bool m_canParry;
        bool m_canBlock;
        bool m_canTitanGrip;
        uint8 m_swingErrorMsg;

        ////////////////////Rest System/////////////////////
        time_t time_inn_enter;
        uint32 inn_pos_mapid;
        float  inn_pos_x;
        float  inn_pos_y;
        float  inn_pos_z;
        float m_rest_bonus;
        RestType rest_type;
        ////////////////////Rest System/////////////////////

        ////////////////////Pet System/////////////////////
        void LoadPetSlot(std::string const &data);
        PlayerPetSlotList m_PetSlots;
        ////////////////////Rest System/////////////////////

    public:
        void SendCustomMessage(std::string const& n);
        void SendCustomMessage(std::string const& n, std::ostringstream const& data);
        void SendCustomMessage(std::string const& n, std::vector<std::string> const& data);

        void SetWargameRequest(WargameRequest* p_Request) { _wargameRequest = p_Request; };
        bool HasWargameRequest() const { return _wargameRequest != nullptr; }
        WargameRequest* GetWargameRequest() const { return _wargameRequest; }
        void ApplyWargameItemModifications();

        uint32 GetBattlePetTrapLevel();
        void SaveBattlePets(SQLTransaction& trans);
        void UnsummonCurrentBattlePetIfAny(bool p_Unvolontary);
        void PetBattleCountBattleSpecies();
        uint8 GetBattlePetCountForSpecies(uint32 speciesID);
        bool HasBattlePetTraining();
        uint32 GetUnlockedPetBattleSlot();
        void SummonBattlePet(ObjectGuid journalID);
        Creature* GetSummonedBattlePet();
        void SummonLastSummonedBattlePet();
        BattlePetMap* GetBattlePets();
        std::shared_ptr<BattlePet> GetBattlePet(ObjectGuid journalID);
        std::shared_ptr<BattlePet>* GetBattlePetCombatTeam();
        uint32 GetBattlePetCombatSize();
        void UpdateBattlePetCombatTeam();
        BattlePetMap _battlePets;
        bool AddBattlePetWithSpeciesId(BattlePetSpeciesEntry const* entry, uint16 flags = 0, bool sendUpdate = true, bool sendDiliveryUpdate = false);
        bool AddBattlePet(uint32 spellID, uint16 flags = 0, bool sendUpdate = true);
        bool AddBattlePetByCreatureId(uint32 creatureId, bool sendUpdate = true, bool sendDiliveryUpdate = false);

        uint32 GetTimeSync() const;
        uint32 GetTimeSyncClient() const;

        HonorInfo* GetHonorInfo() { return &m_honorInfo; }
        uint32 GetPrestigeLevel() { return m_honorInfo.PrestigeLevel; }
        uint32 GetHonorLevel() { return m_honorInfo.HonorLevel; }
        void OnEnterMap();

        ChallengeKeyInfo m_challengeKeyInfo;
        bool InitChallengeKey(Item* item);
        void UpdateChallengeKey(Item* item);
        void CreateChallengeKey(Item* item);
        void ResetChallengeKey();
        void ChallengeKeyCharded(Item* item, uint32 challengeLevel, bool runRand = true);

        std::unordered_set<std::pair<ObjectGuid, uint32>> AllArtifacts; // for found lost artifacts
        std::unordered_set<uint32> AllLegendarys; // for fast check Legendary item

        Vignette::Manager& GetVignetteMgr() { return _vignetteMgr; }

        bool NeedPhaseRecalculate;
        bool NeedPhaseUpdate;
        bool NeedUpdateVisibility;

        ////////////////////Stat System/////////////////////
        KillCreatureMap m_killMap;
        KillCreatureList m_killList;
        void _LoadKillCreature(PreparedQueryResult result);
        void _SaveKillCreature(SQLTransaction& trans);
        void AddKillCreature(uint32 entry, uint32 count, bool encounter);
        uint32 GetKillCreature(uint32 entry) const;
        float GetKillCreaturePoints(uint32 entry) const;
        ////////////////////Stat System/////////////////////

    protected:
        bool _LoadPetBattles(PreparedQueryResult result);
        ObjectGuid _battlePetSummon;
        uint64 _lastSummonedBattlePet;
        std::shared_ptr<BattlePet> _battlePetCombatTeam[3];
        std::set<std::pair<uint32, uint32>> _oldPetBattleSpellToMerge;

        bool _pvpStatsScalingEnabled;
        bool _pvpRulesTimer;
        //kill honor sistem
        KillInfoMap m_killsPerPlayer;
        bool m_flushKills;
        bool m_saveKills;

        HonorInfo m_honorInfo;

        uint32 ping;

        Vignette::Manager _vignetteMgr;

        // Social
        PlayerSocial *m_social;

        // Groups
        GroupReference m_group;
        GroupReference m_originalGroup;
        Group* m_groupInvite;
        uint32 m_groupUpdateMask;
        bool m_bPassOnGroupLoot;
        std::array<GroupUpdateCounter, 2> m_groupUpdateSequences;

        // last used pet number (for BG's)
        uint32 m_lastpetnumber;
        uint32 m_LastPetEntry;

        // Player summoning
        time_t m_summon_expire;
        uint32 m_summon_mapid;
        float  m_summon_x;
        float  m_summon_y;
        float  m_summon_z;

        DeclinedName *m_declinedname;
        Runes m_runes;
        EquipmentSetContainer _equipmentSets;

        bool CanAlwaysSee(WorldObject const* obj) const override;

        bool IsAlwaysDetectableFor(WorldObject const* seer) const override;

        uint8 m_grantableLevels;

    private:
        // internal common parts for CanStore/StoreItem functions
        InventoryResult CanStoreItem_InSpecificSlot(uint8 bag, uint8 slot, ItemPosCountVec& dest, ItemTemplate const* pProto, uint32& count, bool swap, Item* pSrcItem) const;
        InventoryResult CanStoreItem_InBag(uint8 bag, ItemPosCountVec& dest, ItemTemplate const* pProto, uint32& count, bool merge, bool non_specialized, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const;
        InventoryResult CanStoreItem_InInventorySlots(uint8 slot_begin, uint8 slot_end, ItemPosCountVec& dest, ItemTemplate const* pProto, uint32& count, bool merge, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const;
        Item* _StoreItem(uint16 pos, Item* pItem, uint32 count, bool clone, bool update);
        Item* _LoadItem(SQLTransaction& trans, uint32 zoneId, uint32 timeDiff, Field* fields);

        GuidSet m_refundableItems;
        void SendRefundInfo(Item* item);
        void RefundItem(Item* item);
        void SendItemRefundResult(Item* item, ItemExtendedCostEntry const* iece, uint8 error);

        int32 CalculateReputationGain(uint32 creatureOrQuestLevel, int32 rep, int32 faction, Quest const* quest, bool noQuestBonus = false);
        void AdjustQuestReqItemCount(Quest const* quest);

        bool IsCanDelayTeleport() const { return m_bCanDelayTeleport; }
        void SetCanDelayTeleport(bool setting) { m_bCanDelayTeleport = setting; }
        bool IsHasDelayedTeleport() const { return m_bHasDelayedTeleport; }
        void SetDelayedTeleportFlag(bool setting) { m_bHasDelayedTeleport = setting; }
        bool IsHasGlobalTeleport() const { return m_bHasglobalTeleport; }
        void SetGlobalTeleport(bool setting) { m_bHasglobalTeleport = setting; }

        MapReference m_mapRef;

        void UpdateCharmedAI();

        uint32 m_lastFallTime;
        float  m_lastFallZ;

        int32 m_MirrorTimer[MAX_TIMERS];
        uint8 m_MirrorTimerFlags;
        uint8 m_MirrorTimerFlagsLast;
        bool m_isInWater;

        // Current teleport data
        Map* m_teleport_target_map;
        WorldLocation m_teleport_dest;
        uint32 m_teleport_options;
        bool mSemaphoreTeleport_Near;
        bool mSemaphoreTeleport_Far;
        bool m_change_map;
        bool m_removeFromMap;

        uint32 m_DelayedOperations;
        bool m_bCanDelayTeleport;
        bool m_bHasDelayedTeleport;
        bool m_bHasglobalTeleport;

        // Temporary removed pet cache
        uint32 m_temporaryUnsummonedPetNumber;
        uint32 m_oldpetspell;

        AchievementPtr m_achievementMgr;
        ReputationMgr  m_reputationMgr;

        RPPMSpellCooldowns m_rppmspellCooldowns;
        SpellCooldowns m_spellCooldowns;
        SpellChargeDataMap m_spellChargeData;

        uint32 m_ChampioningFaction;
        uint32 m_ChampioningFactionDungeonLevel;

        std::queue<uint32> m_timeSyncQueue;
        uint32 m_timeSyncTimer;
        uint32 m_timeSyncClient;
        uint32 m_timeSyncServer;

        uint32 _pendingBindId;
        uint32 _pendingBindTimer;

        uint32 _activeCheats;

        PhaseMgr phaseMgr;

        uint32 _lastTargetedGO;
        float m_PersonnalXpRate;
        bool canUseDonate = true;

        uint32 m_knockBackTimer;

        uint32 m_groupUpdateDelay;

        uint32 m_clientCheckDelay;
        uint32 m_clientKickDelay;

        BracketList m_BracketsList;

        bool _spectatorFlag;
        bool _spectateCanceled;
        Unit* _spectateFrom;
        bool _spectateRemoving;

        PlayerLootCooldownMap m_playerLootCooldown[MAX_LOOT_COOLDOWN_TYPE];
        PlayerLfgCooldownMap m_playerLfgCooldown;

        bool m_watching_movie;

        AreaTriggerEntry const *LastAreaTrigger;
        std::unordered_map<uint32, SceneEventStatus> m_sceneStatus;

        bool _advancedCombatLoggingEnabled;

        Garrison* _garrison;
        CollectionMgr* _collectionMgr;

        uint16 m_scenarioId = 0;
        uint16 m_adventure_questID = 0;

        PetInfoDataMap m_petInfo;

        WargameRequest* _wargameRequest;
        std::unordered_map<uint32, std::vector<ItemDynamicFieldArtifactPowers>> GlobalArtifactData;
        DeathMatchScore dmScore;
        std::string selected_chat_logo;
        std::map<std::string, bool> buyed_chat_logos;
        std::unordered_map<uint8, uint8>  m_bgQueueRoles{};
        std::unordered_map<uint8, uint8>  m_bgQueueRolesTemp{};
        uint32  m_lastActiveLFGRole{};
};

void AddItemsSetItem(Player*player, Item* item);
void RemoveItemsSetItem(Player*player, ItemTemplate const* proto);

#endif
