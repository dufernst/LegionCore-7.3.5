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

#ifndef CREATURE_H
#define CREATURE_H

#include "Common.h"
#include "GridObject.h"
#include "LootMgr.h"
#include "MapObject.h"
#include "Unit.h"

class BattlePetInstance;
class CreatureAI;
class CreatureGroup;
class Player;
class Quest;
class SpellInfo;
class WorldSession;
struct CreatureActionData;
struct CreatureTextEntry;

enum CreatureFlagsExtra
{
    CREATURE_FLAG_EXTRA_INSTANCE_BIND                   = 0x00000001,       // creature kill bind instance with killer and killer's group
    CREATURE_FLAG_EXTRA_CIVILIAN                        = 0x00000002,       // not aggro (ignore faction/reputation hostility)
    CREATURE_FLAG_EXTRA_NO_PARRY                        = 0x00000004,       // creature can't parry
    CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN                 = 0x00000008,       // creature can't counter-attack at parry
    CREATURE_FLAG_EXTRA_NO_BLOCK                        = 0x00000010,       // creature can't block
    CREATURE_FLAG_EXTRA_NO_CRUSH                        = 0x00000020,       // creature can't do crush attacks
    CREATURE_FLAG_EXTRA_NO_XP_AT_KILL                   = 0x00000040,       // creature kill not provide XP
    CREATURE_FLAG_EXTRA_TRIGGER                         = 0x00000080,       // trigger creature
    CREATURE_FLAG_EXTRA_NO_TAUNT                        = 0x00000100,       // creature is immune to taunt auras and effect attack me
    CREATURE_FLAG_EXTRA_PERSONAL_LOOT                   = 0x00000200,       // Personal loot mobs and increment healths by player
    CREATURE_FLAG_EXTRA_AUTO_LOOT                       = 0x00000400,       // now not use
    CREATURE_FLAG_EXTRA_EVENT_LOOT                      = 0x00000800,       // Generate special item level on kill creature
    CREATURE_FLAG_EXTRA_EVENT_NPC                       = 0x00001000,       // Creature is increase HP by the number of attackers
    CREATURE_FLAG_EXTRA_IMMUNITY_KNOCKBACK              = 0x00002000,       // Creature will immune all knockback effects
    CREATURE_FLAG_EXTRA_WORLDEVENT                      = 0x00004000,       // custom flag for world event creatures (left room for merging)
    CREATURE_FLAG_EXTRA_GUARD                           = 0x00008000,       // Creature is guard
    CREATURE_FLAG_EXTRA_NO_CRIT                         = 0x00020000,       // creature can't do critical strikes
    CREATURE_FLAG_EXTRA_NO_SKILLGAIN                    = 0x00040000,       // creature won't increase weapon skills
    CREATURE_FLAG_EXTRA_TAUNT_DIMINISH                  = 0x00080000,       // Taunt is a subject to diminishing returns on this creautre
    CREATURE_FLAG_EXTRA_ALL_DIMINISH                    = 0x00100000,       // Creature is subject to all diminishing returns as player are
    CREATURE_FLAG_EXTRA_DUNGEON_BOSS                    = 0x10000000,       // creature is a dungeon boss (SET DYNAMICALLY, DO NOT ADD IN DB)
    CREATURE_FLAG_EXTRA_VEHICLE_ATTACKABLE_PASSENGERS   = 0x20000000,       // creature is vehicle, UNIT_STATE_ONVEHICLE will not add to passengers
    CREATURE_FLAG_EXTRA_VEH_INSTANT_DESPAWN_PASSENGERS  = 0x40000000,       // Instant remove creature passengers
    CREATURE_FLAG_EXTRA_HP_85_PERC                      = 0x80000000,       // No damage if HP < 85% for target mob
};

constexpr auto CREATURE_FLAG_EXTRA_DB_ALLOWED (CREATURE_FLAG_EXTRA_INSTANCE_BIND | CREATURE_FLAG_EXTRA_CIVILIAN | \
    CREATURE_FLAG_EXTRA_NO_PARRY | CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN | CREATURE_FLAG_EXTRA_NO_BLOCK | \
    CREATURE_FLAG_EXTRA_NO_CRUSH | CREATURE_FLAG_EXTRA_NO_XP_AT_KILL | CREATURE_FLAG_EXTRA_TRIGGER | \
    CREATURE_FLAG_EXTRA_NO_TAUNT | CREATURE_FLAG_EXTRA_WORLDEVENT | CREATURE_FLAG_EXTRA_NO_CRIT | \
    CREATURE_FLAG_EXTRA_NO_SKILLGAIN | CREATURE_FLAG_EXTRA_TAUNT_DIMINISH | CREATURE_FLAG_EXTRA_ALL_DIMINISH | \
    CREATURE_FLAG_EXTRA_GUARD | CREATURE_FLAG_EXTRA_HP_85_PERC | CREATURE_FLAG_EXTRA_VEHICLE_ATTACKABLE_PASSENGERS | \
    CREATURE_FLAG_EXTRA_VEH_INSTANT_DESPAWN_PASSENGERS | CREATURE_FLAG_EXTRA_PERSONAL_LOOT | CREATURE_FLAG_EXTRA_AUTO_LOOT | \
    CREATURE_FLAG_EXTRA_EVENT_LOOT | CREATURE_FLAG_EXTRA_EVENT_NPC | CREATURE_FLAG_EXTRA_IMMUNITY_KNOCKBACK);

#pragma pack(push, 1)

// Creature Pet entries
#define ENTRY_WATER_ELEMENTAL   78116
#define ENTRY_FELGUARD          17252
#define ENTRY_GHOUL             26125
#define ENTRY_RUNE_WEAPON       27893

#define MAX_KILL_CREDIT 2
#define MAX_TYPE_FLAGS 2
#define MAX_CREATURE_MODELS 4
#define MAX_CREATURE_QUEST_ITEMS 10
#define MAX_CREATURE_NAMES 4
#define CREATURE_MAX_SPELLS 8

#define CREATURE_REGEN_INTERVAL 5 * IN_MILLISECONDS
#define PET_FOCUS_REGEN_INTERVAL 2 * IN_MILLISECONDS
#define BOSS_REGEN_INTERVAL 1 * IN_MILLISECONDS

#define MAX_EQUIPMENT_ITEMS 3

struct CreatureSpell
{
    uint32 SpellID;
    uint32 DifficultyMask;
    uint32 TimerCast;
    CreatureTextEntry const* Text;
};
typedef std::unordered_map<uint32, CreatureSpell> CreatureSpellList;

// from `creature_template` table
struct CreatureTemplate
{
    CreatureTemplate();

    uint32 Entry;
    std::string Name[MAX_CREATURE_NAMES];
    std::string NameAlt[MAX_CREATURE_NAMES];
    std::string Title;
    std::string TitleAlt;
    std::string CursorName;
    uint32 KillCredit[MAX_KILL_CREDIT];
    int32 Modelid[MAX_CREATURE_MODELS];
    uint32 QuestItem[MAX_CREATURE_QUEST_ITEMS];
    uint32 VignetteID;
    uint32 FlagQuest;
    uint32 VerifiedBuild;
    uint32 Classification;
    uint32 MovementInfoID;
    uint32 Family;                                         // enum CreatureFamily
    int8   RequiredExpansion;
    uint32 TypeFlags[2];                                   // enum CreatureTypeFlags[0] mask values [1] unk for now
    uint32 Type;                                           // enum CreatureType values
    float HpMulti = 1.0f;
    float PowerMulti;
    bool Leader = false;

    std::string AIName  = "";
    int32 resistance[MAX_SPELL_SCHOOL];
    uint32 baseattacktime = 2000;
    uint32 dmgschool = 0;
    uint32 dynamicflags = 0;
    uint32 faction = 35;
    uint32 flags_extra = 0;
    uint32 GossipMenuId = 0;
    uint32 InhabitType = 7/*INHABIT_ANYWHERE*/;
    uint32 lootid = 0;
    uint32 maxgold = 0;
    uint32 MechanicImmuneMask = 0;
    uint32 mingold = 0;
    uint32 MovementType = 0;
    uint32 npcflag = 0;
    uint32 npcflag2 = 0;
    uint32 TrackingQuestID = 0;
    uint32 PetSpellDataId = 0;
    uint32 pickpocketLootId = 0;
    uint32 rangeattacktime = 2000;
    uint32 ScriptID = 0;
    uint32 SkinLootId = 0;
    uint32 spells[CREATURE_MAX_SPELLS];
    uint32 trainer_class = 0;
    uint32 trainer_race = 0;
    uint32 trainer_spell = 0;
    uint32 trainer_type = 0;
    uint32 unit_class = UNIT_CLASS_WARRIOR;                // enum Classes. Note only 4 classes are known for creatures.
    uint32 unit_flags = 0;                                 // enum UnitFlags mask values
    uint32 unit_flags2 = 2048;                             // enum UnitFlags2 mask values
    uint32 unit_flags3 = 0;                                // enum UnitFlags3 mask values
    uint32 VehicleId = 0;
    uint16 ScaleLevelDuration = 0;
    uint16 SandboxScalingID = 0;
    uint8 maxlevel = 1;
    uint8 minlevel = 1;
    uint8 ScaleLevelMin = 0;
    uint8 ScaleLevelMax = 0;
    uint8 ScaleLevelDelta = 0;
    int8 ControllerID = 0;
    float dmg_multiplier = 1.0f;
    float HoverHeight = 1.0f;
    float ModArmor = 1.0f;
    float ModManaExtra = 1.0f;                             // Added in 4.x, this value is usually 2 for a small group of creatures with double mana
    float scale = 1.0f;
    float speed_fly = 1.0f;
    float speed_run = 1.14286f;
    float speed_walk = 1.0f;
    bool RegenHealth = true;
    bool QuestPersonalLoot = false;
    bool IgnoreLos = false;
    uint32 AffixState = 0;
    bool MaxVisible = false;

    CreatureSpellList CreatureSpells;

    //Visual effects
    std::list<uint32> WorldEffects;
    std::list<uint32> PassiveSpells;
    uint32  StateWorldEffectID = 0;
    uint32  SpellStateVisualID = 0;
    uint32  SpellStateAnimID = 0;
    uint32  SpellStateAnimKitID = 0;

    static uint8 GetDiffFromSpawn(uint8 spawnmode);
    SkillType GetRequiredLootSkill() const;
    bool isTameable(Player const* caster) const;

    int32  GetRandomValidModelId() const;
    int32  GetFirstValidModelId() const;
    uint32  GetFirstInvisibleModel() const;
    uint32  GetFirstVisibleModel() const;

    bool isWorldBoss() const;
};

// from `creature_difficulty_stat` table
struct CreatureDifficultyStat
{
    uint32  Entry;
    uint8   Difficulty;
    float   dmg_multiplier;
    float   ModHealth;
};

// Benchmarked: Faster than std::map (insert/find)
typedef std::vector<CreatureTemplate*> CreatureTemplateContainer;
typedef std::unordered_map<uint32, CreatureTemplate> CreatureTemplateContainerMap;
typedef std::vector<std::vector<CreatureDifficultyStat> > CreatureDifficultyStatContainer;

struct CreatureLevelStat
{
    uint32   baseHP;
    uint32   baseMP;
    uint64   healthMax;
    float    baseMinDamage;
    float    baseMaxDamage;
    uint32 AttackPower;
    uint32 RangedAttackPower;
    uint32 BaseArmor;
};

typedef std::vector<CreatureLevelStat > CreatureLevelStatContainer;

// Defines base stats for creatures (used to calculate HP/mana/armor).
struct CreatureBaseStats
{
    uint32 BaseHealth[MAX_EXPANSIONS];
    uint32 BaseMana = 1;
    uint32 BaseArmor = 1;
    uint32 AttackPower;
    uint32 RangedAttackPower;
    float BaseDamage[MAX_EXPANSIONS];

    uint64 GenerateHealth(CreatureTemplate const* info, CreatureDifficultyStat const* diffStats = nullptr) const;
    float GenerateBaseDamage(CreatureTemplate const* info) const;
    uint32 GenerateMana(CreatureTemplate const* info) const;
    uint32 GenerateArmor(CreatureTemplate const* info) const;
};

typedef std::unordered_map<uint16, CreatureBaseStats> CreatureBaseStatsContainer;

struct CreatureLocale
{
    StringVector Name[MAX_CREATURE_NAMES];
    StringVector NameAlt[MAX_CREATURE_NAMES];
    StringVector Title;
    StringVector TitleAlt;
};

struct GossipMenuItemsLocale
{
    StringVector OptionText;
    StringVector BoxText;
};

struct PointOfInterestLocale
{
    StringVector IconName;
};

struct EquipmentInfo
{
    uint32 ItemEntry[MAX_EQUIPMENT_ITEMS*2];
};

// Benchmarked: Faster than std::map (insert/find)
typedef std::unordered_map<uint8, EquipmentInfo> EquipmentInfoContainerInternal;
typedef std::unordered_map<uint32, EquipmentInfoContainerInternal> EquipmentInfoContainer;

// from `creature` table
struct CreatureData
{
    explicit CreatureData() : dbData(true) {}
    ObjectGuid::LowType guid = 0;
    uint32 id = 0;                                            // entry in creature_template
    uint16 mapid = 0;
    uint16 zoneId = 0;
    uint16 areaId = 0;
    uint32 phaseMask = 1;
    uint32 displayid = 0;
    int8 equipmentId = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    float orientation = 0.0f;
    uint32 spawntimesecs = 0;
    float spawndist = 0.0f;
    uint32 currentwaypoint = 0;
    uint64 curhealth = 0;
    uint32 curmana = 0;
    uint8 movementType = 0;
    uint64 spawnMask = 1;
    uint32 npcflag = 0;
    uint32 npcflag2 = 0;
    uint32 unit_flags = 0;                                      // enum UnitFlags mask values
    uint32 unit_flags3 = 0;                                     // enum UnitFlags_3 mask values
    uint32 dynamicflags = 0;
    bool isActive = false;
    float personalSize = 0;
    bool isTeemingSpawn = false;

    bool dbData = false;
    bool building = false;                                             // garrison building state
    std::set<uint32> PhaseID;

    uint32 AiID = 0;
    uint32 MovementID = 0;
    uint32 MeleeID = 0;
    bool skipClone = true;
    int16 gameEvent = 0;
    uint32 pool = 0;
    bool MaxVisible = false;
};

// `creature_addon` table
struct CreatureAddon
{
    uint32 path_id;
    uint32 mount;
    uint32 bytes1;
    uint32 bytes2;
    uint32 emote;
    std::vector<uint32> auras;
};

typedef std::unordered_map<ObjectGuid::LowType, CreatureAddon> CreatureAddonContainer;
typedef std::unordered_map<uint32, CreatureAddon> CreatureTemplateAddonContainer;
 

struct CreatureModelInfo
{
    uint32 displayId_other_gender = 0;
    uint32 hostileId = 0;
    float bounding_radius = 0.0f;
    float combat_reach = 0.0f;
    int8 gender = 0;
    bool is_trigger = false;
};

// Benchmarked: Faster than std::map (insert/find)
typedef std::unordered_map<uint32, CreatureModelInfo> CreatureModelContainer;

// `creature_ai_instance` table
struct CreatureAIInstance
{
    uint32 entry;
    uint32 bossid;
    uint32 bossidactivete;
};

typedef std::unordered_map<uint32, CreatureAIInstance> CreatureAIInstanceContainer;

// `creature_ai_instance_door` table

enum DoorType
{
    DOOR_TYPE_ROOM          = 0,    // Door can open if encounter is not in progress
    DOOR_TYPE_PASSAGE       = 1,    // Door can open if encounter is done
    DOOR_TYPE_SPAWN_HOLE    = 2,    // Door can open if encounter is in progress, typically used for spawning places
    MAX_DOOR_TYPES
};

struct DoorData
{
    uint32 entry, bossId;
    DoorType type;
    uint32 boundary;
};

enum BoundaryType
{
    BOUNDARY_NONE = 0,
    BOUNDARY_N,
    BOUNDARY_S,
    BOUNDARY_E,
    BOUNDARY_W,
    BOUNDARY_NE,
    BOUNDARY_NW,
    BOUNDARY_SE,
    BOUNDARY_SW,
    BOUNDARY_MAX_X = BOUNDARY_N,
    BOUNDARY_MIN_X = BOUNDARY_S,
    BOUNDARY_MAX_Y = BOUNDARY_W,
    BOUNDARY_MIN_Y = BOUNDARY_E
};

typedef std::map<BoundaryType, float> BossBoundaryMap;

enum InhabitTypeValues
{
    INHABIT_GROUND = 1,
    INHABIT_WATER  = 2,
    INHABIT_AIR    = 4,
    INHABIT_ANYWHERE = INHABIT_GROUND | INHABIT_WATER | INHABIT_AIR
};

// Enums used by StringTextData::Type (CreatureEventAI)
enum ChatType
{
    CHAT_TYPE_SAY               = 0,
    CHAT_TYPE_YELL              = 1,
    CHAT_TYPE_TEXT_EMOTE        = 2,
    CHAT_TYPE_BOSS_EMOTE        = 3,
    CHAT_TYPE_WHISPER           = 4,
    CHAT_TYPE_BOSS_WHISPER      = 5,
    CHAT_TYPE_ZONE_YELL         = 6,
    CHAT_TYPE_END               = 255
};

enum CreatureActionType
{
    CREATURE_ACTION_TYPE_ATTACK     = 0,
    CREATURE_ACTION_TYPE_CAST       = 1,
    CREATURE_ACTION_TYPE_SUMMON     = 2,
    CREATURE_ACTION_TYPE_MAX        = 3,
};

#pragma pack(pop)

// Vendors
struct VendorItem
{
    VendorItem(int32 _item, int32 _maxcount, uint32 _incrtime, uint32 _ExtendedCost, uint8 _Type, uint64 _Money, uint32 _randomPropertiesSeed, ItemRandomEnchantmentId _randomPropertiesID,
               std::vector<uint32> _bonusListIDs, std::vector<int32> _itemModifiers, bool _DoNotFilterOnVendor, uint8 _context, uint32 _PlayerConditionID, int32 _DonateStoreId, uint32 _DonateCost);

    int32 item;
    uint32 maxcount;                                        // 0 for infinity item amount
    uint32 incrtime;                                        // time for restore items amount if maxcount != 0
    uint32 PlayerConditionID;
    uint32 ExtendedCost;
    uint8  Type;
    uint64 Money;
    uint32 RandomPropertiesSeed;
    ItemRandomEnchantmentId RandomPropertiesID;
    std::vector<uint32> BonusListIDs;
    std::vector<int32> ItemModifiers;
    bool  DoNotFilterOnVendor;
    uint8 Context;
    int32 DonateStoreId;
    uint32 DonateCost;

    //helpers
    bool IsGoldRequired(ItemTemplate const* pProto) const;
};
typedef std::vector<VendorItem*> VendorItemList;

struct VendorItemData
{
    VendorItemList m_items;

    VendorItem* GetItem(uint32 slot) const;

    bool Empty() const { return m_items.empty(); }
    uint32 GetItemCount() const { return m_items.size(); }
    void AddItem(int32 item, int32 maxcount, uint32 ptime, uint32 ExtendedCost, uint8 type, uint64 money, uint32 randomPropertiesSeed = 0, uint32 randomPropertiesID = 0, std::vector<uint32> const& bonusListIDs = std::vector<uint32>(), std::vector<int32> const& itemModifiers = std::vector<int32>(), bool doNotFilterOnVendor = false, uint8 context = 0, uint32 playerConditionID = 0, int32 DonateStoreId = 0, uint32 DonateCost = 0)
    {
        m_items.push_back(new VendorItem(item, maxcount, ptime, ExtendedCost, type, money, randomPropertiesSeed, ItemRandomEnchantmentId(ItemRandomEnchantmentType::Property, randomPropertiesID), bonusListIDs, itemModifiers, doNotFilterOnVendor, context, playerConditionID, DonateStoreId, DonateCost));
    }
    bool RemoveItem(uint32 itemId, uint8 type);
    VendorItem const* FindItemCostPair(uint32 itemId, uint32 extendedCost, uint8 type) const;
    void Clear()
    {
        for (VendorItemList::const_iterator itr = m_items.begin(); itr != m_items.end(); ++itr)
            delete (*itr);
        m_items.clear();
    }
};

struct VendorItemCount
{
    explicit VendorItemCount(uint32 _item, uint32 _count)
        : itemId(_item), count(_count), lastIncrementTime(time(nullptr)) {}

    uint32 itemId;
    uint32 count;
    time_t lastIncrementTime;
};

typedef std::list<VendorItemCount> VendorItemCounts;

#define MAX_TRAINERSPELL_ABILITY_REQS 3

struct TrainerSpell
{
    TrainerSpell();

    uint32 spell;
    uint32 spellCost;
    uint32 reqSkill;
    uint32 reqSkillValue;
    uint32 reqLevel;
    uint32 learnedSpell[MAX_TRAINERSPELL_ABILITY_REQS];

    // helpers
    bool IsCastable() const;
};

typedef std::unordered_map<uint32 /*spellid*/, TrainerSpell> TrainerSpellMap;

struct TrainerSpellData
{
    TrainerSpellData() : trainerType(0) {}
    ~TrainerSpellData() { spellList.clear(); }

    TrainerSpellMap spellList;
    uint32 trainerType;                                     // trainer type based at trainer spells, can be different from creature_template value.
                                                            // req. for correct show non-prof. trainers like weaponmaster, allowed values 0 and 2.
    TrainerSpell const* Find(uint32 spell_id) const;
};

typedef std::map<uint32, time_t> CreatureSpellCooldowns;

enum PetSpellState
{
    PETSPELL_UNCHANGED = 0,
    PETSPELL_CHANGED   = 1,
    PETSPELL_NEW       = 2,
    PETSPELL_REMOVED   = 3
};

enum PetSpellType
{
    PETSPELL_NORMAL = 0,
    PETSPELL_FAMILY = 1,
    PETSPELL_TALENT = 2,
};

enum PetType
{
    SUMMON_PET              = 0,
    HUNTER_PET              = 1,
    MAX_PET_TYPE            = 4,
};

struct PetSpell
{
    ActiveStates active;
    PetSpellState state;
    PetSpellType type;
};

typedef std::unordered_map<uint32, PetSpell> PetSpellMap;
typedef std::vector<uint32> AutoSpellList;

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE 3

#define MAX_VENDOR_ITEMS 150                                // Limitation in 4.x.x item count in SMSG_VENDOR_INVENTORY

class Creature : public Unit, public GridObject<Creature>, public MapObject
{
    public:

        explicit Creature(bool isWorldObject = false);
        virtual ~Creature();

        void AddToWorld() override;
        void RemoveFromWorld() override;
        Battleground* GetBattleground();
        void PrintCreatureSize(Player* player);

        void SetOutfit(int32 outfit);;
        int32 GetOutfit() const;;
        bool IsMirrorImage() const;;

        void DisappearAndDie();

        bool Create(ObjectGuid::LowType guidlow, Map* map, uint32 phaseMask, uint32 Entry, int32 vehId, uint32 team, float x, float y, float z, float ang, const CreatureData* data = nullptr);
        bool LoadCreaturesAddon(bool reload = false);
        void SelectLevel(const CreatureTemplate* cInfo);
        void LoadEquipment(int8 id = 1, bool force=false);

        uint64 GetDBTableGUIDLow() const { return m_DBTableGuid; }

        void Update(uint32 time) override;                         // overwrited Unit::Update
        void GetRespawnPosition(float &x, float &y, float &z, float* ori = nullptr, float* dist = nullptr) const;

        void SetCorpseDelay(uint32 delay) { m_corpseDelay = delay; }
        uint32 GetCorpseDelay() const { return m_corpseDelay; }
        bool isRacialLeader() const { return GetCreatureTemplate()->Leader; }
        bool isCivilian() const { return (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN) != 0; }
        bool isTrigger() const { return (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER) != 0; }
        bool isGuard() const { return (GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_GUARD) != 0; }
        bool CanWalk() const { return (GetCreatureTemplate()->InhabitType & INHABIT_GROUND) != 0; }
        bool CanSwim() const override  { return (GetCreatureTemplate()->InhabitType & INHABIT_WATER) != 0; }
        bool CanFly()  const override { return (GetCreatureTemplate()->InhabitType & INHABIT_AIR) != 0; }
        bool CanShared()  const { return GetCreatureTemplate()->QuestPersonalLoot; }
        bool IsIgnoreLos() const { return GetCreatureTemplate()->IgnoreLos; }

        void SetReactState(ReactStates st, uint32 delay = 0);
        ReactStates GetReactState();
        bool HasReactState(ReactStates state) const;
        void InitializeReactState();

        ///// TODO RENAME THIS!!!!!
        bool isCanTrainingOf(Player* player, bool msg) const;
        bool isCanInteractWithBattleMaster(Player* player, bool msg) const;
        bool isCanTrainingAndResetTalentsOf(Player* player) const;
        bool canCreatureAttack(Unit const* victim, bool force = true) const;
        bool IsImmunedToSpell(SpellInfo const* spellInfo) override;
        bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const override;
        bool isElite() const;
        bool isWorldBoss() const;

        bool IsDungeonBoss() const;
        bool IsPersonal() const;
        bool IsEventCreature() const;
        bool IsPersonalForQuest(Player const* player) const;
        void CalculateMoney(uint32& mingold, uint32& maxgold);

        bool IsInEvadeMode() const { return HasUnitState(UNIT_STATE_EVADE); }

        bool AIM_Initialize(CreatureAI* ai = nullptr);
        void Motion_Initialize();

        CreatureAI* AI() const { return reinterpret_cast<CreatureAI*>(i_AI); }

        SpellSchoolMask GetMeleeDamageSchoolMask() const override { return m_meleeDamageSchoolMask; }
        void SetMeleeDamageSchool(SpellSchools school) { m_meleeDamageSchoolMask = SpellSchoolMask(1 << school); }

        void ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs) override;
        void _AddCreatureSpellCooldown(uint32 spell_id, time_t end_time);
        void _AddCreatureCategoryCooldown(uint32 category, time_t apply_time);
        void AddCreatureSpellCooldown(uint32 spellid);
        bool HasCreatureSpellCooldown(uint32 spell_id) const;
        bool HasCategoryCooldown(uint32 spell_id) const;
        bool HasSchoolMaskCooldown(SpellSchoolMask schoolMask) const;
        uint32 _GetSpellCooldownDelay(uint32 spell_id) const;
        void RemoveCreatureSpellCooldown(uint32 spell_id);
        void AddSpellCooldown(uint32 spell_id, uint32 itemid, double end_time) override;
        bool HasSpellCooldown(uint32 spell_id) override;

        bool HasSpell(uint32 spellID) override;

        bool UpdateEntry(uint32 entry, uint32 team=ALLIANCE, const CreatureData* data= nullptr);
        void UpdateStat();
        bool UpdateStats(Stats stat) override;
        float GetTotalStatValue(Stats stat) override;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateMaxHealth() override;
        void UpdateMaxPower(Powers power) override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void UpdateDamagePhysical(WeaponAttackType attType) override;
        int8 GetOriginalEquipmentId() const { return m_originalEquipmentId; }
        uint8 GetCurrentEquipmentId() const { return m_equipmentId; }
        void SetCurrentEquipmentId(uint8 id) { m_equipmentId = id; }
        float GetSpellDamageMod(int32 Rank);

        VendorItemData const* GetVendorItems() const;
        uint32 GetVendorItemCurrentCount(VendorItem const* vItem);
        uint32 UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count);

        TrainerSpellData const* GetTrainerSpells() const;

        CreatureTemplate const* GetCreatureTemplate() const { return m_creatureInfo; }
        CreatureData const* GetCreatureData() const { return m_creatureData; }
        CreatureDifficultyStat const* GetCreatureDiffStat() const { return m_creatureDiffData; }
        CreatureAddon const* GetCreatureAddon() const;

        std::string GetAIName() const;
        std::string GetNPCAIName() const { return AIName; }
        void SetNPCAIName(std::string name) { AIName = name; }
        std::string GetScriptName() const;
        uint32 GetScriptId() const;

        void Say(int32 textId, uint32 language, ObjectGuid TargetGuid) { MonsterSay(textId, language, TargetGuid); }
        void Yell(int32 textId, uint32 language, ObjectGuid TargetGuid) { MonsterYell(textId, language, TargetGuid); }
        void TextEmote(int32 textId, ObjectGuid TargetGuid, bool IsBossEmote = false) { MonsterTextEmote(textId, TargetGuid, IsBossEmote); }
        void Whisper(int32 textId, ObjectGuid receiver, bool IsBossWhisper = false) { MonsterWhisper(textId, receiver, IsBossWhisper); }
        void YellToZone(int32 textId, uint32 language, ObjectGuid TargetGuid) { MonsterYellToZone(textId, language, TargetGuid); }

        // override WorldObject function for proper name localization
        const char* GetNameForLocaleIdx(LocaleConstant locale_idx) const override;

        void setDeathState(DeathState s) override;                   // override virtual Unit::setDeathState

        bool LoadFromDB(ObjectGuid::LowType guid, Map* map) { return LoadCreatureFromDB(guid, map, false); }
        bool LoadCreatureFromDB(ObjectGuid::LowType guid, Map* map, bool addToMap = true);
        void SaveToDB();
                                                            // overriden in Pet
        virtual void SaveToDB(uint32 mapid, uint64 spawnMask, uint32 phaseMask);
        virtual void DeleteFromDB();                        // overriden in Pet

        Loot loot;
        GuidSet rewardedPlayer;
        GuidList lootList;
        bool lootForPickPocketed;
        bool lootForBody;
        Player* GetLootRecipient() const;
        Group* GetLootRecipientGroup() const;
        Unit* GetOtherRecipient() const;
        bool hasLootRecipient() const;
        bool isTappedBy(Player const* player) const;                          // return true if the creature is tapped by the player or a member of his party.
        void AddToLootList(ObjectGuid targetGuid) { lootList.push_back(targetGuid); }
        void RemoveFromLootList(ObjectGuid targetGuid) { lootList.remove(targetGuid); }
        bool HasInLootList(ObjectGuid targetGuid) const;
        void ClearLootList();
        void AddRewardPlayer(ObjectGuid targetGuid) { rewardedPlayer.insert(targetGuid); }
        bool IsPlayerRewarded(ObjectGuid targetGuid) const;

        void SetLootRecipient (Unit* unit);
        void SetOtherLootRecipient(ObjectGuid guid);
        void AllLootRemovedFromCorpse();

        SpellInfo const* reachWithSpellAttack(Unit* victim);
        SpellInfo const* reachWithSpellCure(Unit* victim);

        CreatureSpell const* GetCreatureSpell(uint32 SpellID);
        CreatureSpellList const* CreatureSpells;
        uint32 m_templateSpells[CREATURE_MAX_SPELLS];
        CreatureSpellCooldowns m_CreatureSpellCooldowns;
        CreatureSpellCooldowns m_CreatureCategoryCooldowns;
        CreatureSpellCooldowns m_CreatureProcCooldowns;
        CreatureSpellCooldowns m_CreatureSchoolCooldowns;

        bool canStartAttack(Unit const* u, bool force) const;
        float GetAttackDistance(Unit const* player) const;

        void SendAIReaction(AiReaction reactionType);

        Unit* SelectNearestTarget(float dist = 0) const;
        Unit* SelectNearestTargetNoCC(float dist = 0) const;
        Unit* SelectNearestTargetInAttackDistance(float dist = 0) const;
        Player* SelectNearestPlayer(float distance = 0) const;
        Player* SelectNearestPlayerNotGM(float distance = 0) const;

        void DoFleeToGetAssistance();
        void CallForHelp(float fRadius);
        void CallAssistance();
        void SetNoCallAssistance(bool val) { m_AlreadyCallAssistance = val; }
        bool IsAlreadyCallAssistance() const { return m_AlreadyCallAssistance; }
        void SetNoSearchAssistance(bool val) { m_AlreadySearchedAssistance = val; }
        bool HasSearchedAssistance() { return m_AlreadySearchedAssistance; }
        bool CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction = true) const;
        bool _IsTargetAcceptable(const Unit* target) const;
        void StopAttack(bool clearMove = false, bool interruptSpells = false);

        MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        void RemoveCorpse(bool setSpawnTime = true);

        void DespawnOrUnsummon(uint32 msTimeToDespawn = 0, Seconds const& forceRespawnTimer = Seconds::zero());

        time_t const& GetRespawnTime() const { return m_respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(nullptr) + respawn : 0; }
        void Respawn(bool force = false, uint32 timer = 3);
        void SaveRespawnTime() override;

        uint32 GetRemoveCorpseDelay() const { return m_corpseRemoveTime; }
        void SetRemoveCorpseDelay(uint32 delay) { m_corpseRemoveTime = delay; }

        uint32 GetRespawnDelay() const { return m_respawnDelay; }
        void SetRespawnDelay(uint32 delay) { m_respawnDelay = delay; }

        float GetRespawnRadius() const { return m_respawnradius; }
        void SetRespawnRadius(float dist) { m_respawnradius = dist; }

        uint32 m_groupLootTimer;                            // (msecs)timer used for group loot
        ObjectGuid lootingGroupLowGUID;                     // used to find group which is looting corpse

        ObjectGuid replacementFromGUID;
        std::shared_ptr<BattlePetInstance> m_battlePetInstance;

        void SendZoneUnderAttackMessage(Player* attacker);

        void SetInCombatWithZone();

        bool hasQuest(uint32 quest_id) const override;
        bool hasInvolvedQuest(uint32 quest_id)  const override;

        bool isRegeneratingHealth() { return m_regenHealth; }
        void setRegeneratingHealth(bool regenHealth) { m_regenHealth = regenHealth; }
        uint8 GetPetAutoSpellSize() const;
        uint32 GetPetAutoSpellOnPos(uint8 pos) const;
        uint8 GetPetCastSpellSize() const;
        void AddPetCastSpell(uint32 spellid);
        uint32 GetPetCastSpellOnPos(uint8 pos) const;

        void SetPosition(float x, float y, float z, float o);
        void SetPosition(const Position& pos);
        void SetHomePosition(float x, float y, float z, float o);
        void SetHomePosition(const Position& pos);
        void GetHomePosition(float& x, float& y, float& z, float& ori) const;
        Position GetHomePosition() const;

        void SetTransportHomePosition(float x, float y, float z, float o);
        void SetTransportHomePosition(const Position& pos);
        void GetTransportHomePosition(float& x, float& y, float& z, float& ori);
        Position GetTransportHomePosition();

        uint32 GetWaypointPath(){return m_path_id;}
        void LoadPath(uint32 pathid) { m_path_id = pathid; }

        uint32 GetCurrentWaypointID(){return m_waypointID;}
        void UpdateWaypointID(uint32 wpID){m_waypointID = wpID;}

        void SearchFormation();
        CreatureGroup* GetFormation() {return m_formation;}
        void SetFormation(CreatureGroup* formation) {m_formation = formation;}

        Unit* SelectVictim();

        void SetDisableReputationGain(bool disable) { DisableReputationGain = disable; }
        bool IsReputationGainDisabled() { return DisableReputationGain; }
        bool IsDamageEnoughForLootingAndReward() const { return m_PlayerDamageReq > 0; }
        void LowerPlayerDamageReq(uint32 unDamage)
        {
            m_PlayerDamageReq += unDamage;
        }
        void ResetPlayerDamageReq() { m_PlayerDamageReq = 0; }
        uint32 m_PlayerDamageReq;

        uint32 GetOriginalEntry() const { return m_originalEntry; }
        void SetOriginalEntry(uint32 entry) { m_originalEntry = entry; }

        static float _GetDamageMod(int32 Rank);

        float m_SightDistance, m_CombatDistance;

        void FarTeleportTo(Map* map, float X, float Y, float Z, float O);

        bool m_isTempWorldObject; //true when possessed
        uint32 GetBossId() const { return bossid; }
        uint8 GetMobDifficulty() const { return m_difficulty; }
        uint32 GetPlayerCount() const { return m_playerCount; }

        float GetFollowAngle() const override { return m_followAngle; }
        void SetFollowAngle(float angle) { m_followAngle = angle; }

        float GetFollowDistance() const override { return m_followDistance; }
        void SetFollowDistance(float dist) { m_followDistance = dist; }

        void ForcedDespawn(uint32 timeMSToDespawn = 0, Seconds const& forceRespawnTimer = Seconds(0));

        void SetLockAI(bool lock) { m_AI_locked = lock; }

        AutoSpellList   m_autospells;
        AutoSpellList   m_castspells;
        PetSpellMap     m_spells;
        bool m_despawn;
        bool m_isHati;
        uint32 m_IfUpdateTimer;
        float m_RateUpdateTimer;
        uint32 m_RateUpdateWait;

        uint32 m_respawnCombatDelay;

        CreatureLevelStatContainer m_levelStat;
        void GenerateScaleLevelStat(const CreatureTemplate* cInfo);
        CreatureLevelStat const* GetScaleLevelStat(uint8 level);

        std::vector<CreatureActionData> const* m_actionData[CREATURE_ACTION_TYPE_MAX];

        bool onVehicleAccessoryInit() const { return m_onVehicleAccessory; }
        void SetVehicleAccessoryInit(bool r) { m_onVehicleAccessory = r; }

        uint32 GetVignetteId() const { return m_creatureInfo ? m_creatureInfo->VignetteID : 0; }
        uint32 GetTrackingQuestID() const { return m_creatureInfo ? m_creatureInfo->TrackingQuestID : 0; }

        bool IsDespawn() const { return m_despawn; }

        bool IsNoDamage() const;

        uint32 GetAffixState() const { return GetCreatureTemplate()->AffixState; }

        uint32 disableAffix;
        bool IsAffixDisabled(uint8 affixe) const { return (disableAffix & (1 << affixe)) != 0; }

        uint8 ScaleLevelMin = 0;
        uint8 ScaleLevelMax = 0;
        uint32 GetScalingID();
        bool m_CanCallAssistance;
        uint8 m_callAssistanceText;

        bool DistanceCheck() override;

    protected:
        bool m_onVehicleAccessory;

        bool CreateFromProto(ObjectGuid::LowType guidlow, uint32 Entry, int32 vehId, uint32 team, const CreatureData* data = nullptr);
        bool InitEntry(uint32 entry, uint32 team=ALLIANCE, const CreatureData* data= nullptr);

        // vendor items
        VendorItemCounts m_vendorItemCounts;

        static float _GetHealthMod(int32 Rank);
        float _GetHealthModPersonal(uint32 &count);
        float _GetHealthModForDiff();
        float _GetDamageModForDiff();

        ObjectGuid m_lootRecipient;
        ObjectGuid m_LootOtherRecipient;                        // Pet lotter for example
        ObjectGuid m_lootRecipientGroup;

        /// Timers
        time_t m_corpseRemoveTime;                          // (msecs)timer for death or corpse disappearance
        time_t m_respawnTime;                               // (secs) time of next respawn
        time_t m_respawnChallenge;                          // (secs) time of next respawn
        uint32 m_respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                               // (secs) delay between death and corpse disappearance
        float m_respawnradius;

        ReactStates m_reactState;                           // for AI, not charmInfo
        void RegenerateHealth();
        void Regenerate(Powers power, uint32 diff);
        uint32 GetRegenerateInterval();
        uint32 m_regenTimerCount;
        uint32 m_sendInterval;
        MovementGeneratorType m_defaultMovementType;
        uint64 m_DBTableGuid;                  ///< For new or temporary creatures is 0 for saved it is lowguid
        uint8 m_equipmentId;
        int8 m_originalEquipmentId; // can be -1

        bool m_AlreadyCallAssistance;
        bool m_AlreadySearchedAssistance;
        bool m_regenHealth;
        bool m_AI_locked;
        std::string AIName;

        SpellSchoolMask m_meleeDamageSchoolMask;
        uint32 m_originalEntry;

        Position m_homePosition;
        Position m_transportHomePosition;

        bool DisableReputationGain;

        CreatureTemplate const* m_creatureInfo;                 // in difficulty mode > 0 can different from sObjectMgr->GetCreatureTemplate(GetEntry())
        CreatureData const* m_creatureData;
        CreatureDifficultyStat const* m_creatureDiffData;

        uint32 bossid;
        uint8 m_difficulty;
        uint32 m_playerCount;
        float m_followAngle;
        float m_followDistance = 1.0f;

        bool IsInvisibleDueToDespawn() const override;
        bool CanAlwaysSee(WorldObject const* obj) const override;
        bool IsNeverVisible(WorldObject const* seer = nullptr) const override;
    private:

        //WaypointMovementGenerator vars
        uint32 m_waypointID;
        uint32 m_path_id;

        //Formation var
        CreatureGroup* m_formation;
        bool TriggerJustRespawned;
        int32 outfitId;
};

class AssistDelayEvent : public BasicEvent
{
    public:
        AssistDelayEvent(ObjectGuid victim, Unit& owner) : BasicEvent(), m_victim(victim), m_owner(owner) { }

        bool Execute(uint64 e_time, uint32 p_time) override;
        void AddAssistant(ObjectGuid guid) { m_assistants.push_back(guid); }
    private:
        AssistDelayEvent() = delete;

        ObjectGuid            m_victim;
        GuidList              m_assistants;
        Unit&                 m_owner;
};

class ForcedDespawnDelayEvent : public BasicEvent
{
public:
    ForcedDespawnDelayEvent(Creature& owner, Seconds const& respawnTimer) : BasicEvent(), m_owner(owner), m_respawnTimer(respawnTimer) { }
    bool Execute(uint64 e_time, uint32 p_time) override;

private:
    Creature & m_owner;
    Seconds const m_respawnTimer;
};

class SetImuneDelayEvent : public BasicEvent
{
    public:
        SetImuneDelayEvent(Creature& owner) : BasicEvent(), m_owner(owner) { }
        bool Execute(uint64 e_time, uint32 p_time) override;

    private:
        Creature& m_owner;
};

struct MirrorImageUpdate : BasicEvent
{
    MirrorImageUpdate(Creature* creature);
    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/) override;
    Creature* creature;
};

#endif
