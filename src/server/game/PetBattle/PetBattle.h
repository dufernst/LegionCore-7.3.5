////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common.h"

class Field;

#define MAX_PETBATTLE_SLOTS 3
#define MAX_PETBATTLE_TEAM 2
#define MAX_PETBATTLE_ABILITIES 3
#define MAX_PETBATTLE_ABILITY_TURN 10

#define PETBATTLE_ENTER_MOVE_SPLINE_ID 0xA42BA70B

#define PETBATTLE_NULL_ID 0
#define PETBATTLE_NULL_SLOT -1

#define PETBATTLE_TEAM_1 0
#define PETBATTLE_TEAM_2 1
#define PETBATTLE_PVE_TEAM_ID 1

enum BattlePetMisc
{
    BATTLE_PET_CAGE_ITEM_ID = 82800,

    BATTLEPET_MAX_LEVEL = 25,
    MAX_BATTLE_PETS_PER_SPECIES = 3,

};

enum PetBattleType : uint8
{
    PETBATTLE_TYPE_PVE,
    PETBATTLE_TYPE_PVP_DUEL,
    PETBATTLE_TYPE_PVP_MATCHMAKING
};

enum PvePetBattleType
{
    PVE_PETBATTLE_WILD,
    PVE_PETBATTLE_TRAINER
};

enum ePetBattleStatus
{
    PETBATTLE_STATUS_CREATION,
    PETBATTLE_STATUS_RUNNING,
    PETBATTLE_STATUS_FINISHED,
    PETBATTLE_STATUS_PENDING_DELETE
};

enum ePetBattleQualities
{
    BATTLEPET_QUALITY_POOR      = 0,
    BATTLEPET_QUALITY_COMMON    = 1,
    BATTLEPET_QUALITY_UNCOMMON  = 2,
    BATTLEPET_QUALITY_RARE      = 3,
    BATTLEPET_QUALITY_EPIC      = 4,
    BATTLEPET_QUALITY_LEGENDARY = 5,
    BATTLEPET_QUALITY_INHERITED = 7,
};

enum eBattlePetTypes : int8
{
    BATTLEPET_PETTYPE_ALL           = -1,
    BATTLEPET_PETTYPE_HUMANOID      = 0,
    BATTLEPET_PETTYPE_DRAGONKIN     = 1,
    BATTLEPET_PETTYPE_FLYING        = 2,
    BATTLEPET_PETTYPE_UNDEAD        = 3,
    BATTLEPET_PETTYPE_CRITTER       = 4,
    BATTLEPET_PETTYPE_MAGIC         = 5,
    BATTLEPET_PETTYPE_ELEMENTAL     = 6,
    BATTLEPET_PETTYPE_BEAST         = 7,
    BATTLEPET_PETTYPE_AQUATIC       = 8,
    BATTLEPET_PETTYPE_MECHANICAL    = 9,
    NUM_BATTLEPET_PETTYPES
};

enum eBattlePetFlags
{
    BATTLEPET_FLAG_FAVORITE         = 0x01,
    PETBATTLE_FLAG_CAPTURED         = 0x01,
    BATTLEPET_FLAG_ABILITY_1_SECOND = 0x10,
    BATTLEPET_FLAG_ABILITY_2_SECOND = 0x20,
    BATTLEPET_FLAG_ABILITY_3_SECOND = 0x40,
    BATTLEPET_FLAG_GIFT             = 0x80
};

enum class BattlePetError : uint16
{
    ERR_CANT_HAVE_MORE_PETS_OF_THAT_TYPE    = 3,
    ERR_CANT_HAVE_MORE_PETS                 = 4,
    ERR_PET_TOO_HIGH_LEVEL_TO_UNCAGE        = 7,
};

enum BattlePetState
{
    BATTLEPET_STATE_Internal_InitialHealth          = 3,
    BATTLEPET_STATE_Internal_InitialLevel           = 17,
    BATTLEPET_STATE_Internal_CaptureBoost           = 90,
    BATTLEPET_STATE_Internal_EffectSucceeded        = 91,
    BATTLEPET_STATE_Internal_HealthBeforeInstakill  = 145,

    BATTLEPET_STATE_Is_Dead                         =   1,
    BATTLEPET_STATE_maxHealthBonus                  =   2,
    BATTLEPET_STATE_Stat_Kharma                     =   4,
    BATTLEPET_STATE_Stat_Power                      =  18,
    BATTLEPET_STATE_Stat_Stamina                    =  19,
    BATTLEPET_STATE_Stat_Speed                      =  20,
    BATTLEPET_STATE_Mechanic_IsPoisoned             =  21,
    BATTLEPET_STATE_Mechanic_IsStunned              =  22,
    BATTLEPET_STATE_Mod_DamageDealtPercent          =  23,
    BATTLEPET_STATE_Mod_DamageTakenPercent          =  24,
    BATTLEPET_STATE_Mod_SpeedPercent                =  25,
    BATTLEPET_STATE_Ramping_DamageID                =  26,
    BATTLEPET_STATE_Ramping_DamageUses              =  27,
    BATTLEPET_STATE_Condition_WasDamagedThisTurn    =  28,
    BATTLEPET_STATE_untargettable                   =  29,
    BATTLEPET_STATE_Mechanic_IsUnderground          =  30,
    BATTLEPET_STATE_Last_HitTaken                   =  31,
    BATTLEPET_STATE_Last_HitDealt                   =  32,
    BATTLEPET_STATE_Mechanic_IsFlying               =  33,
    BATTLEPET_STATE_Mechanic_IsBurning              =  34,
    BATTLEPET_STATE_turnLock                        =  35,
    BATTLEPET_STATE_swapOutLock                     =  36,
    BATTLEPET_STATE_Stat_CritChance                 =  40,
    BATTLEPET_STATE_Stat_Accuracy                   =  41,
    BATTLEPET_STATE_Passive_Critter                 =  42,
    BATTLEPET_STATE_Passive_Beast                   =  43,
    BATTLEPET_STATE_Passive_Humanoid                =  44,
    BATTLEPET_STATE_Passive_Flying                  =  45,
    BATTLEPET_STATE_Passive_Dragon                  =  46,
    BATTLEPET_STATE_Passive_Elemental               =  47,
    BATTLEPET_STATE_Passive_Mechanical              =  48,
    BATTLEPET_STATE_Passive_Magic                   =  49,
    BATTLEPET_STATE_Passive_Undead                  =  50,
    BATTLEPET_STATE_Passive_Aquatic                 =  51,
    BATTLEPET_STATE_Mechanic_IsChilled              =  52,
    BATTLEPET_STATE_Weather_BurntEarth              =  53,
    BATTLEPET_STATE_Weather_ArcaneStorm             =  54,
    BATTLEPET_STATE_Weather_Moonlight               =  55,
    BATTLEPET_STATE_Weather_Darkness                =  56,
    BATTLEPET_STATE_Weather_Sandstorm               =  57,
    BATTLEPET_STATE_Weather_Blizzard                =  58,
    BATTLEPET_STATE_Weather_Mud                     =  59,
    BATTLEPET_STATE_Weather_Rain                    =  60,
    BATTLEPET_STATE_Weather_Sunlight                =  61,
    BATTLEPET_STATE_Weather_LightningStorm          =  62,
    BATTLEPET_STATE_Weather_Windy                   =  63,
    BATTLEPET_STATE_Mechanic_IsWebbed               =  64,
    BATTLEPET_STATE_Mod_HealingDealtPercent         =  65,
    BATTLEPET_STATE_Mod_HealingTakenPercent         =  66,
    BATTLEPET_STATE_Mechanic_IsInvisible            =  67,
    BATTLEPET_STATE_unkillable                      =  68,
    BATTLEPET_STATE_Mechanic_IsObject               =  69,
    BATTLEPET_STATE_Special_Plant                   =  70,
    BATTLEPET_STATE_Add_FlatDamageTaken             =  71,
    BATTLEPET_STATE_Add_FlatDamageDealt             =  72,
    BATTLEPET_STATE_Stat_Dodge                      =  73,
    BATTLEPET_STATE_Special_BlockedAttackCount      =  74,
    BATTLEPET_STATE_Special_ObjectRedirectionAuraID =  75,
    BATTLEPET_STATE_Mechanic_IsBleeding             =  77,
    BATTLEPET_STATE_Stat_Gender                     =  78,
    BATTLEPET_STATE_Mechanic_IsBlind                =  82,
    BATTLEPET_STATE_Cosmetic_Stealthed              =  84,
    BATTLEPET_STATE_Cosmetic_WaterBubbled           =  85,
    BATTLEPET_STATE_Mod_PetTypeDamageDealtPercent   =  87,
    BATTLEPET_STATE_Mod_PetTypeDamageTakenPercent   =  88,
    BATTLEPET_STATE_Mod_PetType_ID                  =  89,
    BATTLEPET_STATE_Special_IsCockroach             =  93,
    BATTLEPET_STATE_swapInLock                      =  98,
    BATTLEPET_STATE_Mod_MaxHealthPercent            =  99,
    BATTLEPET_STATE_Clone_Active                    = 100,
    BATTLEPET_STATE_Clone_PBOID                     = 101,
    BATTLEPET_STATE_Clone_PetAbility1               = 102,
    BATTLEPET_STATE_Clone_PetAbility2               = 103,
    BATTLEPET_STATE_Clone_PetAbility3               = 104,
    BATTLEPET_STATE_Clone_Health                    = 105,
    BATTLEPET_STATE_Clone_MaxHealth                 = 106,
    BATTLEPET_STATE_Clone_LastAbilityID             = 107,
    BATTLEPET_STATE_Clone_LastAbilityTurn           = 108,
    BATTLEPET_STATE_Special_IsCharging              = 113,
    BATTLEPET_STATE_Special_IsRecovering            = 114,
    BATTLEPET_STATE_Clone_CloneAbilityID            = 117,
    BATTLEPET_STATE_Clone_CloneAuraID               = 118,
    BATTLEPET_STATE_DarkSimulacrum_AbilityID        = 119,
    BATTLEPET_STATE_Special_ConsumedCorpse          = 120,
    BATTLEPET_STATE_Ramping_PBOID                   = 121,
    BATTLEPET_STATE_reflecting                      = 122,
    BATTLEPET_STATE_Special_BlockedFriendlyMode     = 123,
    BATTLEPET_STATE_Special_TypeOverride            = 124,
    BATTLEPET_STATE_Mechanic_IsWall                 = 126,
    BATTLEPET_STATE_Condition_DidDamageThisRound    = 127,
    BATTLEPET_STATE_Cosmetic_FlyTier                = 128,
    BATTLEPET_STATE_Cosmetic_FetishMask             = 129,
    BATTLEPET_STATE_Mechanic_IsBomb                 = 136,
    BATTLEPET_STATE_Special_IsCleansing             = 141,
    BATTLEPET_STATE_Cosmetic_Bigglesworth           = 144,
    BATTLEPET_STATE_resilient                       = 149,
    BATTLEPET_STATE_Passive_Elite                   = 153,
    BATTLEPET_STATE_Cosmetic_Chaos                  = 158,
    BATTLEPET_STATE_Passive_Boss                    = 162,
    BATTLEPET_STATE_Cosmetic_TreasureGoblin         = 176,
    BATTLEPET_STATE_Ignore_Damage_Below_Threshold   = 191,
    BATTLEPET_STATE_Cosmetic_Spectral_Blue          = 196,
    BATTLEPET_STATE_Special_Egg                     = 199,
    BATTLEPET_STATE_Ignore_Damage_Above_Threshold   = 200,
    NUM_BATTLEPET_STATES
};

enum PetBattleCastTriggerFlag
{
    PETBATTLE_CAST_TRIGGER_NONE             = 0x00,
    PETBATTLE_CAST_TRIGGER_IGNORE_COOLDOWN  = 0x01,

    PETBATTLE_CAST_TRIGGER_ALL              = 0xFFFFFFFF
};

enum PetBattleCastResult
{
    PETBATTLE_CAST_OK,
    PETBATTLE_CAST_INVALID_ID,
    PETBATTLE_CAST_INTERNAL_ERROR
};

enum PetBattleResult
{
    PETBATTLE_RESULT_WON,
    PETBATTLE_RESULT_LOOSE,
    PETBATTLE_RESULT_ABANDON
};

enum ePetBattleRounds
{
    PETBATTLE_ROUND_RUNNING,
    PETBATTLE_ROUND_FINISHED
};

enum PetBattleRoundResult
{
    PETBATTLE_ROUND_RESULT_NONE             = 0,
    PETBATTLE_ROUND_RESULT_NORMAL           = 2,
    PETBATTLE_ROUND_RESULT_CATCH_OR_KILL    = 3
};

/// Extracted from file FrameXML/SharedPetBattleTemplates.lua
enum ePetBattleAbilities
{
    PETBATTLE_ABILITY_TURN0_PROC_ON_APPLY           = 0,
    PETBATTLE_ABILITY_TURN0_PROC_ON_DAMAGE_TAKEN    = 1,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_DAMAGE_DEALT    = 2,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_HEAL_TAKEN      = 3,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_HEAL_DEALT      = 4,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_AURA_REMOVED    = 5,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_ROUND_START     = 6,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_ROUND_END       = 7,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_TURN            = 8,
    PETBATTLE_ABILITY_TURN0_PROC_ON_ABILITY         = 9,        /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_SWAP_IN         = 10,       /// implemented
    PETBATTLE_ABILITY_TURN0_PROC_ON_SWAP_OUT        = 11,       /// implemented

    PETBATTLE_ABILITY_TURN0_PROC_ON_NONE = 0xFF      /// Custom value
};

class BattlePet
{
public:
    virtual ~BattlePet() = default;

    void Load(Field* fields);
    void CloneFrom(std::shared_ptr<BattlePet> & p_BattlePet);
    void Save(SQLTransaction& trans);

    ObjectGuid::LowType AddToPlayer(Player* player);
    void AddToPlayer(Player* player, SQLTransaction& trans);
    void Remove(Player* player);
    void UpdateAbilities();
    void UpdateStats();

    uint32 AccountID;                              ///< Owner account ID
    ObjectGuid JournalID;                              ///< As companion (db/journal id)
    int32 Slot;                                   ///< Battle slot
    std::string Name;                                   ///< Name
    uint32 NameTimeStamp;                          ///< Name timestamp
    uint32 Species;                                ///< Species ID
    uint32 DisplayModelID;                         ///< Display id (no real usage, client can deduce it for some species)
    uint32 Breed;                                  ///< Breed quality (factor for some states)
    uint32 Quality;                                ///< Pet quality (factor for some states)
    uint32 Abilities[MAX_PETBATTLE_ABILITIES];     ///< Available abilities
    int32 Health;                                 ///< Current health
    uint32 Level;                                  ///< Pet level
    uint32 XP;                                     ///< Pet XP
    uint32 Flags;                                  ///< Flags
    int32 InfoPower;                              ///< Info power (need UpdateStats calls)
    int32 InfoMaxHealth;                          ///< Info max health (need UpdateStats calls)
    int32 InfoSpeed;                              ///< Info speed (need UpdateStats calls)
    int32 InfoGender;                             ///< Info gender (need UpdateStats calls)
    std::string DeclinedNames[MAX_DECLINED_NAME_CASES]; ///< Declined names
    bool needSave = false;
    bool needDelete = false;
};

class PetBattle;

class BattlePetInstance : public BattlePet
{
public:
    BattlePetInstance();
    virtual ~BattlePetInstance() = default;

    static std::shared_ptr<BattlePetInstance> CloneForBattle(std::shared_ptr<BattlePetInstance> const& p_BattlePet);

    bool IsAlive();
    bool CanAttack();

    int32 GetMaxHealth();
    int32 GetSpeed();
    uint32 GetMaxXPForCurrentLevel();
    uint32 GetXPEarn(uint32 targetPetID);

    void UpdateOriginalInstance(Player* l_Player);

    bool Caged = false;
    bool Captured = false;

    uint32 TeamID;                                 ///< Team ID
    uint32 ID;                                     ///< Rel id for battle (0 - 1 - 2 - 3 - 4 - 5)

    int32 Cooldowns[MAX_PETBATTLE_ABILITIES];     ///< Pet cooldowns
    int32 Lockdowns[MAX_PETBATTLE_ABILITIES];     ///< Pet lockdowns
    int32 States[NUM_BATTLEPET_STATES];           ///< Pet states

    PetBattle* PetBattleInstance;                      ///< Pet battle instance helper

    uint32 OldLevel;
    uint32 OldXP;

    std::shared_ptr<BattlePet> OriginalBattlePet;
    ObjectGuid OriginalCreature;
};

struct PetBattleEventUpdate
{
    PetBattleEventUpdate();

    uint32  UpdateType;     ///< Update type
    int8    TargetPetID;    ///< Target pet id

    /// Update data here
    union
    {
        int32 Health;
        int32 MaxHealth;
        int32 TriggerAbilityId;
        int32 Speed;

        /// Buff update data
        struct
        {
            uint32  ID;                     ///< Aura display slot
            uint32  AbilityID;              ///< Aura ability ID
            int32   Duration;               ///< Remaining duration
            uint32  Turn;                   ///< Aura turn
        } Buff;

        /// State npc emote
        struct
        {
            uint32  ID;                     ///< State ID
            int32   Value;                  ///< State value
        } State;

        /// State npc emote
        struct
        {
            uint32  BroadcastTextID;        ///< State ID
        } NpcEmote;
    };
};

typedef std::list<PetBattleEventUpdate> PetBattleEventUpdateList;

/// Event types
enum ePetBattleEvents
{
    PETBATTLE_EVENT_SET_HEALTH              = 0,
    PETBATTLE_EVENT_BUFF_APPLY              = 1,
    PETBATTLE_EVENT_BUFF_CANCEL             = 2,
    PETBATTLE_EVENT_BUFF_CHANGE             = 3,
    PETBATTLE_EVENT_PET_SWAP                = 4,
    PETBATTLE_EVENT_CATCH                   = 5,
    PETBATTLE_EVENT_SET_STATE               = 6,
    PETBATTLE_EVENT_SET_MAX_HEALTH          = 7,
    PETBATTLE_EVENT_SET_SPEED               = 8,
    PETBATTLE_EVENT_SET_POWER               = 9,
    PETBATTLE_EVENT_TRIGGER_ABILITY         = 10,
    PETBATTLE_EVENT_ABILITY_CHANGE          = 11,
    PETBATTLE_EVENT_NPC_EMOTE               = 12,
    PETBATTLE_EVENT_AURA_PROCESSING_BEGIN   = 13,
    PETBATTLE_EVENT_AURA_PROCESSING_END     = 14
};

/// Event flags
enum ePetBattleEventFlags
{
    PETBATTLE_EVENT_FLAG_SKIP_TURN  = 0x0001,
    PETBATTLE_EVENT_FLAG_MISS       = 0x0002,
    PETBATTLE_EVENT_FLAG_CRITICAL   = 0x0004,
    PETBATTLE_EVENT_FLAG_BLOCKED    = 0x0008,
    PETBATTLE_EVENT_FLAG_DODGE      = 0x0010,
    PETBATTLE_EVENT_FLAG_HEAL       = 0x0020,
    PETBATTLE_EVENT_FLAG_REFLECT    = 0x0080,
    PETBATTLE_EVENT_FLAG_ABSORB     = 0x0100,
    PETBATTLE_EVENT_FLAG_IMMUNE     = 0x0200,
    PETBATTLE_EVENT_FLAG_STRONG     = 0x0400,
    PETBATTLE_EVENT_FLAG_WEAK       = 0x0800,
    PETBATTLE_EVENT_FLAG_UNK_KILL   = 0x1000,

    PETBATTLE_EVENT_FLAG_PERIODIC   = 0x00010000 // Not exist in client, flags field is actually on 16 bits
};

struct PetBattleEvent
{
    PetBattleEvent(uint32 eventType = 0, int32 sourcePetID = PETBATTLE_NULL_ID, uint32 flags = 0, uint32 abilityEffectID = 0, uint32 roundTurn = 0, uint32 buffTurn = 0, uint32 stackDepth = 0);

    PetBattleEvent& UpdateHealth(int8 targetPetID, int32 p_Health);
    PetBattleEvent& UpdateMaxHealth(int8 targetPetID, int32 p_MaxHealth);
    PetBattleEvent& UpdateState(int8 targetPetID, uint32 stateID, int32 value);
    PetBattleEvent& UpdateFrontPet(int8 newFrontPet = PETBATTLE_NULL_ID);
    PetBattleEvent& UpdateBuff(int8 targetPetID, uint32 p_ID, uint32 abilityID, int32 duration, uint32 turn);
    PetBattleEvent& UpdateSpeed(int8 targetPetID, int32 p_Speed);
    PetBattleEvent& Trigger(int8 targetPetID, uint32 abilityID);

    uint32 EventType;               ///< Type of event (PETBATTLE_EVENT_SET_HEALTH, PETBATTLE_EVENT_BUFF_APPLY, PETBATTLE_EVENT_BUFF_CANCEL,PETBATTLE_EVENT_BUFF_CHANGE, PETBATTLE_EVENT_PET_SWAP, ...)
    uint32 Flags;                   ///< Event flags (PETBATTLE_EVENT_FLAG_SKIP_TURN,PETBATTLE_EVENT_FLAG_MISS, PETBATTLE_EVENT_FLAG_CRITICAL, PETBATTLE_EVENT_FLAG_BLOCKED, ...)
    int32  SourcePetID;             ///< Caster pet id
    uint32 AbilityEffectID;         ///< Id of an ability effect (used for client animation)
    uint32 BuffTurn;                ///< Buff rel turn count/id
    uint32 RoundTurn;               ///< Turn in round turn see  PetBattle::RoundTurn (used for order sync)
    uint32 StackDepth;              ///< unk

    PetBattleEventUpdateList Updates;   ///< Event updates, client support more than 1 update pet event, but never seen more than 1 update per event on retails
};

typedef std::list<PetBattleEvent> PetBattleEventList;

struct PetBattleRequest
{
    PetBattleRequest() = default;

    ObjectGuid RequesterGuid;
    ObjectGuid OpponentGuid;
    Position TeamPosition[MAX_PETBATTLE_TEAM] = { };
    Position PetBattleCenterPosition;
    uint32 LocationResult = false;
    PetBattleType RequestType = PETBATTLE_TYPE_PVE;
    bool IsPvPReady[MAX_PETBATTLE_TEAM] = { };
};

class PetBattleAura
{
public:
    void Apply(PetBattle* petBattle);
    void Remove(PetBattle* petBattle);

    void Process(PetBattle* petBattle);
    void Expire(PetBattle* petBattle);

    uint32 AbilityID;              ///< Ability ID
    uint32 TriggerId;              /// Ability effect id
    uint32 CasterPetID;            ///< Caster pet id
    uint32 TargetPetID;            ///< Target pet id
    int32 Turn;                   ///< Turn (increment every aura process)
    uint32 ID;                     ///< Slot (client side)
    int32 Duration;               ///< Remaining duration (client sinc)
    int32 MaxDuration;            ///< Max aura duration
    bool Expired;                ///< Aura is expired ?
};

/// Team flags 1
enum PetBattleTeamInputFlags
{
    PETBATTLE_TEAM_INPUT_FLAG_LOCK_ABILITIES_1  = 0x01,
    PETBATTLE_TEAM_INPUT_FLAG_LOCK_ABILITIES_2  = 0x02,
    PETBATTLE_TEAM_INPUT_FLAG_LOCK_PET_SWAP     = 0x04,
    PETBATTLE_TEAM_INPUT_FLAG_SELECT_NEW_PET    = 0x08,
};

/// Team flags 2
enum PetBattleTeamCatchFlags
{
    PETBATTLE_TEAM_CATCH_FLAG_ENABLE_TRAP           = 0x01,
    PETBATTLE_TEAM_CATCH_FLAG_NEED_LVL3_PET         = 0x02,
    PETBATTLE_TEAM_CATCH_FLAG_TOO_MUCH_HP           = 0x04,
    PETBATTLE_TEAM_CATCH_FLAG_ONE_CATCH_PER_FIGHT   = 0x08,
};

class PetBattleTeam
{
public:
    bool Update();

    void DoCasts(uint32 p_Turn0ProcCond = PETBATTLE_ABILITY_TURN0_PROC_ON_NONE);

    bool HasPendingMultiTurnCast();

    bool CanCastAny();
    bool CanSwap(int8 replacementPet = PETBATTLE_NULL_ID);
    uint8 CanCatchOpponentTeamFrontPet();

    uint32 GetTeamInputFlags();
    uint32 GetTeamTrapStatus();
    std::vector<uint32> GetAvailablesPets();

    uint32 GetCatchAbilityID();

    ObjectGuid OwnerGuid;                                       ///< Team owner guid
    ObjectGuid PlayerGuid;                                      ///< Team player owner guid

    PetBattle* PetBattleInstance;                          ///< Pet battle instance

    std::shared_ptr<BattlePetInstance> TeamPets[MAX_PETBATTLE_SLOTS];   ///< Team pets
    uint32 TeamPetCount;                                    ///< Team pet count

    std::map<uint32, uint32> CapturedSpeciesCount;          ///< Captured species count

    int32 ActivePetID;                                     ///< Team active pet

    uint32 ActiveAbilityId;
    uint32 activeAbilityTurn;
    uint8 activeAbilityTurnMax;

    int8 CapturedPet;                                       ///< Captured pet id

    bool Ready;                                             ///< Team is ready to process next round
    bool isRun;                                             ///< Team is ready to run
};

class PetBattle
{
public:
    PetBattle();
    ~PetBattle();

    void AddPet(uint32 teamID, std::shared_ptr<BattlePetInstance> pet);

    void Begin();
    void ProceedRound();
    void Finish(uint32 winnerTeamID, bool aborted, bool ignoreAbandonPenalty);

    void Update(uint32 diff);

    void SwapPet(uint32 teamID, int32 newFrontPetID, bool initial = false);

    bool CanCast(uint32 teamID, uint32 abilityID);
    void PrepareCast(uint32 teamID, uint32 abilityID);
    PetBattleCastResult Cast(uint32 casterPetID, uint32 abilityID, uint32 turn, uint32 p_Turn0ProcCondition, uint32 triggerFlag);

    bool AddAura(uint32 casterPetID, uint32 targetPetID, uint32 abilityID, int32 duration, int32 p_MaxAllowed, uint32 fromAbilityEffectID, uint32 flags);
    void SetPetState(uint32 sourcePetID, uint32 targetPetID, uint32 fromAbilityEffectID, uint32 stateID, int32 value, bool fromCapture = false, uint32 flags = 0);
    void Kill(int8 killer, int8 target, uint32 killerAbibilityEffectID, bool fromCapture = false, uint32 flags = 0);
    void Catch(int8 p_Catcher, int8 p_CatchedTarget, uint32 fromAbilityEffectID);

    uint32 GetFirstAttackingTeam();

    int32 GetForfeitHealthPenalityPct();

    ObjectGuid ID;                                                          ///< Battle global unique ID
    PetBattleType BattleType;                                               ///< Battle type (PETBATTLE_TYPE_PVE / PETBATTLE_TYPE_PVP_DUEL / PETBATTLE_TYPE_PVP_MATCHMAKING)
    PvePetBattleType PveBattleType;                                         ///< PVE battle type (PVE_PETBATTLE_WILD / PVE_PETBATTLE_TRAINER)
    uint32 Turn;                                                            ///< Battle current turn id
    PetBattleResult CombatResult;                                           ///< Combat result (PETBATTLE_RESULT_WON, PETBATTLE_RESULT_LOOSE, PETBATTLE_RESULT_ABANDON)
    PetBattleRequest PvPMatchMakingRequest;                                 ///< PVP request

    uint32 BattleStatus;                                                    ///< PETBATTLE_STATUS_CREATION / PETBATTLE_STATUS_RUNNING / PETBATTLE_STATUS_FINISHED

    uint32 RoundStatus;                                                     ///< Current round status (PETBATTLE_ROUND_RUNNING / PETBATTLE_ROUND_FINISHED)
    uint32 RoundTurn;                                                       ///< Current round turn for spells cast (independant of PetBattle::Turn)
    PetBattleRoundResult RoundResult;                                       ///< Current round result
    uint32 RoundFirstTeamCasting;                                           ///< Team id who has the priority in ProceedRound (base on active pets speed)
    PetBattleEventList RoundEvents;                                         ///< Current round event queue (for client update)
    std::vector<uint8> PetXDied;                                            ///< Pets who died during this round
    std::vector<std::pair<uint32, uint32>> RoundPetSpeedUpdate;             ///< Round pet speed update <petid, abilityeffectid>

    PetBattleTeam * Teams[MAX_PETBATTLE_TEAM];                              ///< Battle teams
    std::shared_ptr<BattlePetInstance> Pets[MAX_PETBATTLE_TEAM * MAX_PETBATTLE_SLOTS];      ///< All pets involved in the battle
    uint32 TotalPetCount;                                                   ///< Battle total pet count

    std::list<PetBattleAura*> PetAuras;                                             ///< Current battle pets auras
    uint32 WeatherAbilityId;                                                ///< Only one weather at a time on battle

    int32 WinnerTeamId;
    std::map<uint8, bool> FightedPets;
    int8 CatchedPetId;
    ObjectGuid InitialWildPetGUID;

private:
    IntervalTimer m_UpdateTimer;

};

enum LFBUpdateStatus : uint32
{
    LFB_NONE                                    = 0,
    LFB_JOIN_QUEUE                              = 1, // ERR_PETBATTLE_QUEUE_QUEUED
    LFB_UPDATE_STATUS                           = 2,
    LFB_ALREADY_QUEUED                          = 3, // ERR_PETBATTLE_QUEUE_ALREADY_QUEUED
    LFB_CANT_JOIN_QUEUE                         = 4, // ERR_PETBATTLE_QUEUE_JOIN_FAILED
    LFB_CANT_JOINT_QUEUE_DUE_TO_PET_STATUS      = 5,
    LFB_PET_ATLAS_IS_UNAVAILABLE_DURING_BATTLE  = 6, // ERR_PETBATTLE_QUEUE_JOURNAL_LOCK
    LFB_CANT_JOIN_DUE_TO_UNSELECTED_FACTION     = 7, // ERR_PETBATTLE_QUEUE_NOT_WHILE_NEUTRAL
    LFB_PROPOSAL_BEGIN                          = 8,
    LFB_PROPOSAL_DECLINED                       = 9, // ERR_PETBATTLE_QUEUE_PROPOSAL_DECLINED
    LFB_OPPONENT_PROPOSAL_DECLINED              = 10, // ERR_PETBATTLE_QUEUE_OPPONENT_DECLINED
    LFB_PROPOSAL_FAILED                         = 11, // ERR_PETBATTLE_QUEUE_PROPOSAL_TIMEOUT
    LFB_LEAVE_QUEUE                             = 12, // ERR_PETBATTLE_QUEUE_REMOVED
    LFB_QUEUE_ERROR                             = 13, // ERR_PETBATTLE_QUEUE_REQUEUED_INTERNAL
    LFB_OPPONENT_IS_UNAVAILABLE                 = 14, // ERR_PETBATTLE_QUEUE_REQUEUED_REMOVED
    // 18
    // 19
    LFB_PET_BATTLE_IS_STARTED                   = 20, // ERR_PETBATTLE_IN_BATTLE
    LFB_INVALIDE_LOCATION                       = 21 // ERR_PETBATTLE_NOT_HERE
};

enum LFBProposalState : uint32
{
    LFB_PROPOSAL_STATE_INITIATING   = 0,
    LFB_PROPOSAL_STATE_FAILED       = 1,
    LFB_PROPOSAL_STATE_SUCCESS      = 2
};
