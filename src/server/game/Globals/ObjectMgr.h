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

#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Conversation.h"
#include "Log.h"
#include "Object.h"
#include "Creature.h"
#include "Player.h"
#include "GameObject.h"
#include "Corpse.h"
#include "ItemTemplate.h"
#include "NPCHandler.h"
#include "Mail.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "VehicleDefines.h"
#include <limits>
#include <utility>
#include "ConditionMgr.h"
#include "PhaseMgr.h"

class Item;
class PhaseMgr;

struct EventObjectData;

#pragma pack(push, 1)

struct PageText
{
    std::string Text;
    uint32 NextPageID;
    int32 PlayerConditionID;
    uint8 Flags;
};

#pragma pack(pop)

// Benchmarked: Faster than std::unordered_map (insert/find)
typedef std::map<uint32, PageText> PageTextContainer;

// Benchmarked: Faster than std::map (insert/find)
typedef std::unordered_map<uint16, InstanceTemplate> InstanceTemplateContainer;
typedef std::vector<InstanceTemplate*> InstanceTemplateVector;

struct GameTele
{
    float  position_x;
    float  position_y;
    float  position_z;
    float  orientation;
    uint32 mapId;
    std::string name;
    std::wstring wnameLow;
};

typedef std::unordered_map<uint32, GameTele > GameTeleContainer;

struct CreatureOutfit
{
    static constexpr uint32 max_outfit_displays = 11;
    static constexpr uint32 max_custom_displays = 3;
    uint8 race;
    uint8 Class;
    uint8 gender;
    uint8 face;
    uint8 skin;
    uint8 hair;
    uint8 facialhair;
    uint8 haircolor;
    uint8 customdisplay[3];
    uint32 displayId;
    uint32 outfit[max_outfit_displays];
};

typedef std::unordered_map<uint32, CreatureOutfit> CreatureOutfitContainer;

enum VisibleDistanceType
{
    TYPE_VISIBLE_MAP  = 0,
    TYPE_VISIBLE_ZONE = 1,
    TYPE_VISIBLE_AREA = 2,
    TYPE_VISIBLE_MAX  = 3,
};

typedef std::unordered_map<uint32 /*id*/, float /*distance*/> VisibleDistanceMap;
extern VisibleDistanceMap sVisibleDistance[TYPE_VISIBLE_MAX];
float GetVisibleDistance(uint32 type, uint32 id);

enum RateType
{
    TYPE_RATE_MAP  = 0,
    TYPE_RATE_ZONE = 1,
    TYPE_RATE_AREA = 2,
    TYPE_RATE_MAX  = 3,
};

typedef std::unordered_map<uint32 /*id*/, float /*rate*/> WorldRateMap;
extern WorldRateMap sWorldRateInfo[TYPE_RATE_MAX];
float GetRateInfo(Creature* creature);

extern int32 objectCountInWorld[50]; // HighGuid::Count
extern std::vector<int32> creatureCountInWorld;
extern std::vector<int32> spellCountInWorld;
extern int32 creatureCountInArea[10000];

extern int32 playerCountInArea[10000];
void AddPlayerToArea(uint16 areaId);
void RemovePlayerFromArea(uint16 areaId);
int32 GetPlayerFromArea(uint16 areaId);

struct SpellClickInfo
{
    uint32 spellId;
    uint8 castFlags;
    SpellClickUserTypes userType;

    // helpers
    bool IsFitToRequirements(Unit const* clicker, Unit const* clickee) const;
};

struct DonVenCat
{
    int32 action;
    std::string Names[12];
    // std::string NameEn;
    // std::string NameKo;
    // std::string NameFr;
    // std::string NameDe;
    // std::string NameCn;
    // std::string NameTw;
    // std::string NameEs;
    // std::string NameMx;
    // std::string NameRu;
    // std::string NamePt;
    // std::string NameBr;
    // std::string NameIt;
    uint32 type;
    uint8 faction;
    bool is_available_for_preview = false;
};

struct DonVenAdd
{
    uint32 action;
    uint32 cost;
    uint32 type;
    uint32 storeId;
    uint8 faction = 0;
    
    std::string Names[12];
    // std::string NameEn;
    // std::string NameKo;
    // std::string NameFr;
    // std::string NameDe;
    // std::string NameCn;
    // std::string NameTw;
    // std::string NameEs;
    // std::string NameMx;
    // std::string NameRu;
    // std::string NamePt;
    // std::string NameBr;
    // std::string NameIt;
    
};


enum DMStoreTypes
{
    DM_TYPE_MORPH  = 0,
    DM_TYPE_LOGO,
    
    DM_TYPE_MAX
};

enum DonateStoreTypes // + 1 from db
{
    DONATE_TYPE_ITEM = 1,       // 0 db
    DONATE_TYPE_TITLE = 2,      // 1 db
    DONATE_TYPE_ACHIEVEMENTS = 3, //! 2 db. Not realised
    DONATE_TYPE_MORPH = 4,      // 3 db

    DONATE_TYPE_MAX
};

struct DonVenCatBase
{
    int32 parent = 0;
    uint8 type = DONATE_TYPE_ITEM;
    int32 next_page = -1, prev_page = -1;
    bool is_available_for_preview = false;
    
    std::vector<DonVenCat> categories{};
    std::vector<DonVenAdd> additionals{};
};

struct DeathMatchStore
{
    DeathMatchStore(std::string  name_, uint8 type_, std::string logo_, uint32 cost_, uint32 personal_id_) : type(type_), logo(logo_), product(0), cost(cost_), personal_id(personal_id_), name(std::move(name_))
    {
        std::ostringstream ss;
        ss << logo_ << " Cost = " << cost_ << " DeathMatch's currency";
        name = ss.str();
    }

    DeathMatchStore(std::string const& name_, uint8 type_, uint32 product_, uint32 cost_, uint32 personal_id_) : type(type_), product(product_), cost(cost_), personal_id(personal_id_), name(name_)
    {
        std::ostringstream ss;
        ss << name_ << " Cost = " << cost_ << " DeathMatch's currency";
        name = ss.str();
    }
    
    ~DeathMatchStore() = default;
    
    uint8 type;
    
    std::string logo;
    uint32 product;
    
    uint32 cost;
    uint32 personal_id;
    std::string name;
};
typedef std::multimap<uint32, SpellClickInfo> SpellClickInfoContainer;
typedef std::pair<SpellClickInfoContainer::const_iterator, SpellClickInfoContainer::const_iterator> SpellClickInfoMapBounds;

struct ScenarioData
{
    uint32 ScenarioID;
    uint32 MapID;
    uint32 DifficultyID;
    uint32 Team;
    uint32 Class;
    uint32 LfgDungeonID;
};

struct ScenarioSpellData
{
    uint32 StepId;
    uint32 Spells;
};

typedef std::unordered_map<uint32/*ScenarioID*/, ScenarioData> ScenarioDataMap;
typedef std::unordered_map<uint32 /*MapID*/, std::list<ScenarioData* >> ScenarioDataListMap;
typedef std::vector<std::vector<ScenarioSpellData>> ScenarioDataSpellsStep;

struct CreatureActionData
{
    uint32 entry;
    uint32 target;
    uint32 spellId;
    uint8 type;
    uint8 action;
};

typedef std::unordered_map<uint32/*entry*/, std::vector<CreatureActionData> > CreatureActionDataMap;

struct RaceUnlockRequirement
 {
    uint8 Expansion;
    uint32 AchievementId;
};

struct SeamlessTeleportData
{
    uint16 ZoneID;
    uint16 AreaID;
    uint16 FromMapID;
    uint16 ToMapID;
};

typedef std::unordered_map<uint16 /*ZoneID*/, SeamlessTeleportData> SeamlessTeleportMap;

struct PlayerChoiceResponseRewardItem
{
    PlayerChoiceResponseRewardItem() : Id(0), Quantity(0) { }
    PlayerChoiceResponseRewardItem(uint32 id, std::vector<uint32> bonusListIDs, int32 quantity) : Id(id), BonusListIDs(std::move(bonusListIDs)), Quantity(quantity) { }

    std::vector<uint32> BonusListIDs;
    uint32 Id;
    int32 Quantity;
};

struct PlayerChoiceResponseRewardEntry
{
    PlayerChoiceResponseRewardEntry() : Id(0), Quantity(0) { }
    PlayerChoiceResponseRewardEntry(uint32 id, int32 quantity) : Id(id), Quantity(quantity) { }

    uint32 Id;
    int32 Quantity;
};

struct PlayerChoiceResponseReward
{
    std::vector<PlayerChoiceResponseRewardItem> Items;
    std::vector<PlayerChoiceResponseRewardEntry> Currency;
    std::vector<PlayerChoiceResponseRewardEntry> Faction;
    uint64 Money;
    int32 TitleId;
    int32 PackageId;
    int32 SkillLineId;
    uint32 SkillPointCount;
    uint32 ArenaPointCount;
    uint32 HonorPointCount;
    uint32 Xp;
    int32 SpellID; // custom
};

struct PlayerChoiceResponse
{
    Optional<PlayerChoiceResponseReward> Reward;
    int32 ResponseId;
    int32 ChoiceArtFileId;
    std::string Header;
    std::string Answer;
    std::string Description;
    std::string Confirmation;
};

struct PlayerChoice
{
    std::vector<PlayerChoiceResponse> Responses;
    int32 ChoiceId;
    int32 UiTextureKitId;
    std::string Question;
    bool HideWarboardHeader;

    PlayerChoiceResponse const* GetResponse(int32 responseId) const
    {
        auto itr = std::find_if(Responses.begin(), Responses.end(), [responseId](PlayerChoiceResponse const& playerChoiceResponse) { return playerChoiceResponse.ResponseId == responseId; });
        return itr != Responses.end() ? &(*itr) : nullptr;
    }
};
struct PlayerChoiceResponseLocale
{
    std::vector<std::string> Answer;
    std::vector<std::string> Header;
    std::vector<std::string> Description;
    std::vector<std::string> Confirmation;
};

struct PlayerChoiceLocale
{
    std::vector<std::string> Question;
    std::unordered_map<int32 /*ResponseId*/, PlayerChoiceResponseLocale> Responses;
};

struct GameObjectActionData
{
    uint32 MapID;
    uint32 SpellID;
    uint32 WorldSafeLocID;
    uint32 Distance;
    float X;
    float Y;
    float Z;
    float O;
    uint8  ActionType;
};

typedef std::unordered_map<uint32/*entry*/, std::vector<GameObjectActionData> > GameObjectActionMap;

typedef std::set<ObjectGuid::LowType> CellGuidSet;
typedef std::map<ObjectGuid/*player guid*/, uint32/*instance*/> CellCorpseMap;
struct CellObjectGuids
{
    CellGuidSet eventobject;
    CellGuidSet conversation;
    CellGuidSet creatures;
    CellGuidSet gameobjects;
    CellGuidSet statictransports;
    CellCorpseMap corpses;
};
typedef std::map<uint32/*cell_id*/, CellObjectGuids> CellObjectGuidsMap;
typedef std::vector<std::vector<CellObjectGuidsMap>> MapObjectGuids;

// Trinity string ranges
#define MIN_TRINITY_STRING_ID           1                    // 'trinity_string'
#define MAX_TRINITY_STRING_ID           2000000000
#define MIN_DB_SCRIPT_STRING_ID        MAX_TRINITY_STRING_ID // 'db_script_string'
#define MAX_DB_SCRIPT_STRING_ID        2000010000
#define MIN_CREATURE_AI_TEXT_STRING_ID (-1)                 // 'creature_ai_texts'
#define MAX_CREATURE_AI_TEXT_STRING_ID (-1000000)

// Trinity Trainer Reference start range
#define TRINITY_TRAINER_START_REF      200000

struct TrinityStringLocale
{
    StringVector Content;
};

typedef std::unordered_map<ObjectGuid, ObjectGuid> LinkedRespawnContainer;
typedef std::unordered_map<ObjectGuid::LowType, CreatureData> CreatureDataContainer;
typedef std::unordered_map<ObjectGuid::LowType, GameObjectData> GameObjectDataContainer;
typedef std::map<TempSummonGroupKey, std::vector<TempSummonData> > TempSummonDataContainer;
typedef std::vector<std::vector<WorldLocation> > InstanceGraveYardContainer;

struct PersonalLootData
{
    uint32 entry;
    uint32 lootspellId;
    uint32 bonusspellId;
    uint32 cooldownid;
    uint32 goEntry;
    uint8 type;
    uint8 chance; 
    uint8 cooldowntype;
    uint8 respawn;
};

enum PersonalRespawnType
{
    TYPE_NORESPAWN      = 0,
    TYPE_RESPAWN        = 1,
    TYPE_NODESPAWN      = 2
};

typedef std::unordered_map<uint32, CreatureLocale> CreatureLocaleContainer;
typedef std::unordered_map<uint32, GameObjectLocale> GameObjectLocaleContainer;
typedef std::unordered_map<uint32, NpcTextLocale> NpcTextLocaleContainer;
typedef std::unordered_map<uint32, PageTextLocale> PageTextLocaleContainer;
typedef std::unordered_map<int32, TrinityStringLocale> TrinityStringLocaleContainer;
typedef std::unordered_map<uint32, PersonalLootData> PersonalLootContainer;

struct PetStats
{
    int32 ap_type;
    int32 school_mask;
    int32 state;
    int32 energy;
    int32 energy_type;
    int32 type;
    int32 maxspdorap;
    int32 haste;
    float hp;
    float ap;
    float spd;
    float armor;
    float damage;
};

struct MailLevelReward
{
    MailLevelReward() : raceMask(0), mailTemplateId(0), senderEntry(0) {}
    MailLevelReward(uint32 _raceMask, uint32 _mailTemplateId, uint32 _senderEntry) : raceMask(_raceMask), mailTemplateId(_mailTemplateId), senderEntry(_senderEntry) {}

    uint32 raceMask;
    uint32 mailTemplateId;
    uint32 senderEntry;
};

typedef std::list<MailLevelReward> MailLevelRewardList;
typedef std::unordered_map<uint8, MailLevelRewardList> MailLevelRewardContainer;

// We assume the rate is in general the same for all three types below, but chose to keep three for scalability and customization
struct RepRewardRate
{
    float quest_rate;                                       // We allow rate = 0.0 in database. For this case, it means that
    float creature_rate;                                    // no reputation are given at all for this faction/rate type.
    float spell_rate;
};

struct ReputationOnKillEntry
{
    uint32 RepFaction;
    int32 RepValue;
    uint32 ReputationCap;
};

struct RepSpilloverTemplate
{
    uint32 faction[MAX_SPILLOVER_FACTIONS];
    float faction_rate[MAX_SPILLOVER_FACTIONS];
    uint32 faction_rank[MAX_SPILLOVER_FACTIONS];
};




struct ScenarioPOIPoint
{
    int32 x;
    int32 y;

    ScenarioPOIPoint() : x(0), y(0) {}
    ScenarioPOIPoint(int32 _x, int32 _y) : x(_x), y(_y) {}
};

struct ScenarioPOI
{
    uint32 BlobID;
    uint32 MapID;
    uint32 WorldMapAreaID;
    uint32 Floor;
    uint32 Priority;
    uint32 Flags;
    uint32 WorldEffectID;
    uint32 PlayerConditionID;
    std::vector<ScenarioPOIPoint> points;

    ScenarioPOI() : BlobID(0), MapID(0), WorldMapAreaID(0), Floor(0), Priority(0), Flags(0), WorldEffectID(0), PlayerConditionID(0) {}
    ScenarioPOI(uint32 _BlobID, uint32 _MapID, uint32 _WorldMapAreaID, uint32 _Floor, uint32 _Priority, uint32 _Flags, uint32 _WorldEffectID, uint32 _PlayerConditionID) :
        BlobID(_BlobID), MapID(_MapID), WorldMapAreaID(_WorldMapAreaID), Floor(_Floor), Priority(_Priority), Flags(_Flags), WorldEffectID(_WorldEffectID), PlayerConditionID(_PlayerConditionID) { }
};

typedef std::vector<ScenarioPOI> ScenarioPOIVector;
typedef std::unordered_map<uint32, ScenarioPOIVector> ScenarioPOIContainer;

struct GraveYardData
{
    uint32 safeLocId;
    uint32 team;
};

typedef std::multimap<uint32, GraveYardData> GraveYardContainer;


typedef std::unordered_map<int32, VendorItemData> CacheVendorItemContainer;
typedef std::unordered_map<int32, VendorItemData> CacheDonateVendorItemContainer;
typedef std::unordered_map<uint32, TrainerSpellData> CacheTrainerSpellContainer;
typedef std::vector<float> PriceForLevelUp;
typedef std::vector<float> PriceForArtLevelUp;
typedef std::unordered_map<uint32, float> PriceForAddBonus;

typedef std::unordered_map<uint8, uint8> ExpansionRequirementContainer;
typedef std::unordered_map<uint32, std::string> RealmNameContainer;

enum SkillRangeType
{
    SKILL_RANGE_LANGUAGE,                                   // 300..300
    SKILL_RANGE_LEVEL,                                      // 1..max skill for level
    SKILL_RANGE_MONO,                                       // 1..1, grey monolite bar
    SKILL_RANGE_RANK,                                       // 1..skill for known rank
    SKILL_RANGE_NONE,                                       // 0..0 always
};

#define MAX_SKILL_STEP 16

struct SkillTiersEntry
{
    uint32      ID;                                         // 0
    uint32      Value[MAX_SKILL_STEP];                      // 1-16
};

SkillRangeType GetSkillRangeType(SkillRaceClassInfoEntry const* rcEntry);


struct DungeonEncounter
{
    DungeonEncounter(DungeonEncounterEntry const* _dbcEntry, EncounterCreditType _creditType, uint32 _creditEntry, uint32 _lastEncounterDungeon)
        : dbcEntry(_dbcEntry), creditType(_creditType), creditEntry(_creditEntry), lastEncounterDungeon(_lastEncounterDungeon) { }

    DungeonEncounterEntry const* dbcEntry;
    EncounterCreditType creditType;
    uint32 creditEntry;
    uint32 lastEncounterDungeon;
};

typedef std::list<DungeonEncounter const*> DungeonEncounterList;
typedef std::unordered_map<uint32, DungeonEncounterList> DungeonEncounterContainer;
typedef std::vector<DungeonEncounterList*> DungeonEncounterVector;
typedef std::unordered_map<uint32, uint16> CreatureToDungeonEncounterMap;
typedef std::vector<uint32> DungeonEncounterToCreatureMap;

class PlayerDumpReader;

struct ItemSpecStats
{
    uint32 ItemType;
    uint32 ItemSpecStatTypes[MAX_ITEM_PROTO_STATS];
    uint32 ItemSpecStatCount;

    ItemSpecStats(ItemEntry const* item, ItemSparseEntry const* sparse);
    void AddStat(ItemSpecStat statType);
    void AddModStat(int32 itemStatType);
};

class ObjectMgr
{
    friend class PlayerDumpReader;

        ObjectMgr();
        ~ObjectMgr();

    public:
        typedef std::unordered_map<uint32, Item*> ItemMap;
        
        typedef std::unordered_map<int32, DonVenCatBase> DonateVendorCat;
        typedef std::unordered_map<int32, std::vector<int32>> FakeDonateVendorCat;
        typedef std::unordered_map<int32, int32> ReversFakeDonateVendorCat;
        typedef std::unordered_map<uint32, std::map<uint32, DonVenAdd> > DonVenAdds; // [type][entry]
        typedef std::unordered_map<uint8, std::vector<DeathMatchStore>> DeathMatchProducts;
        typedef std::unordered_map<uint32, std::vector<DeathMatchStore>> DeathMatchProductsById;
        
        typedef std::map<AccessRequirementKey, AccessRequirement> AccessRequirementContainer;
        typedef std::unordered_map<uint32, RepRewardRate > RepRewardRateContainer;
        typedef std::unordered_map<uint32, std::vector<ReputationOnKillEntry>> RepOnKillContainer;
        typedef std::unordered_map<uint32, RepSpilloverTemplate> RepSpilloverTemplateContainer;
        typedef StringVector ScriptNameContainer;
        typedef std::list<CurrencyLoot> CurrencysLoot;
        typedef std::unordered_map<uint32, uint32> CreatureSpellBonusList;
        typedef std::unordered_map<uint32, NpcText> NpcTextContainer;
        typedef std::map<uint32, PetStats> PetStatsContainer;
        typedef std::vector<uint32> PlayerXPperLevel;       // [level]
        typedef std::map<uint32, uint32> BaseXPContainer;          // [area level][base xp]
        typedef std::map<uint32, int32> FishingBaseSkillContainer; // [areaId][base skill level]
        typedef std::map<uint32, StringVector> HalfNameContainer;

    static ObjectMgr* instance();
        std::list<CurrencyLoot> GetCurrencyLoot(uint32 entry, uint8 type, uint8 spawnMode);

        Player* GetPlayerByLowGUID(ObjectGuid::LowType const& lowguid) const;

        bool IsStaticTransport(uint32 entry);
        GameObjectTemplate const* GetGameObjectTemplate(uint32 entry);
        GameObjectTemplateContainer const* GetGameObjectTemplates() const { return &_gameObjectTemplateStore; }
        int LoadReferenceVendor(int32 vendor, int32 item, uint8 type, std::set<uint32> *skip_vendors);

        void LoadGameObjectTemplate();
        void LoadGameObjectQuestVisual();

        const std::vector<CreatureDifficultyStat>* GetDifficultyStat(uint32 entry) const;
        CreatureDifficultyStat const* GetCreatureDifficultyStat(uint32 entry, uint8 diff) const;

        CreatureTemplate const* GetCreatureTemplate(uint32 entry);
        CreatureTemplateContainerMap const* GetCreatureTemplates() const { return &_creatureTemplateStoreMap; }
        CreatureModelInfo const* GetCreatureModelInfo(uint32 modelId);
        CreatureModelInfo const* GetCreatureModelRandomGender(uint32* displayID);
        static int32 ChooseDisplayId(uint32 team, const CreatureTemplate* cinfo, const CreatureData* data = nullptr);
        static void ChooseCreatureFlags(const CreatureTemplate* cinfo, uint32& npcflag, uint32& npcflag2, uint32& unit_flags, uint32& unit_flags3, uint32& dynamicflags, const CreatureData* data = nullptr);
        EquipmentInfo const* GetEquipmentInfo(uint32 entry, int8& id);
        CreatureAddon const* GetCreatureAddon(ObjectGuid::LowType const& lowguid);
        CreatureAddon const* GetCreatureTemplateAddon(uint32 entry);
        ItemTemplate const* GetItemTemplate(uint32 entry);
        ItemTemplateContainer const* GetItemTemplateStore() const { return &_itemTemplateStore; }

        CreatureSpellBonusList _creatureSpellBonus;

        uint32 GetEntryByBonusSpell(uint32 spellId) const;

        InstanceTemplate const* GetInstanceTemplate(uint32 mapId);

        PetStats const* GetPetStats(uint32 creature_id) const;

        uint32 GetPlayerClassLevelInfo(uint32 class_, uint8 level) const;

        PlayerInfo const* GetPlayerInfo(uint32 race, uint32 class_) const;

        void GetPlayerLevelInfo(uint32 race, uint32 class_, uint8 level, PlayerLevelInfo* info) const;

        static ObjectGuid GetPlayerGUIDByName(std::string name);
        static bool GetPlayerNameByGUID(ObjectGuid const& guid, std::string &name);
        uint32 GetPlayerTeamByGUID(ObjectGuid const& guid) const;
        static uint32 GetPlayerAccountIdByGUID(ObjectGuid const& guid);
        static uint32 GetPlayerAccountIdByPlayerName(std::string const& name);

        uint32 GetNearestTaxiNode(float x, float y, float z, uint32 mapid, Player* player);
        void GetTaxiPath(uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint32 GetTaxiMountDisplayId(uint32 id, uint32 team, bool allowed_alt_team = false);

        NpcText const* GetNpcText(uint32 textID) const;

        WorldSafeLocsEntry const* GetDefaultGraveYard(uint32 team);
        WorldSafeLocsEntry const* GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team, bool outInstance = false);
        bool AddGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool persist = true);
        void RemoveGraveYardLink(uint32 id, uint32 zoneId, uint32 team, bool persist = false);
        void LoadGraveyardZones();
        GraveYardData const* FindGraveYardData(uint32 id, uint32 zone);
        void AddInstanceGraveYard(uint32 mapId, float x, float y, float z, float o);
        void ClearInstanceGraveYardStore() { _instanceGraveYardStore.clear(); }
        std::vector<WorldLocation> const* GetInstanceGraveYard(uint32 mapId) const;

        AccessRequirement const* GetAccessRequirement(int32 mapid, Difficulty difficulty, uint16 dungeonId = 0) const;

        RepRewardRate const* GetRepRewardRate(uint32 factionId) const
        {
            return Trinity::Containers::MapGetValuePtr(_repRewardRateStore, factionId);
        }

        std::vector<ReputationOnKillEntry> const* GetReputationOnKilEntry(uint32 id) const
        {
            return Trinity::Containers::MapGetValuePtr(_repOnKillStore, id);
        }

        RepSpilloverTemplate const* GetRepSpilloverTemplate(uint32 factionId) const
        {
            return Trinity::Containers::MapGetValuePtr(_repSpilloverTemplateStore, factionId);
        }

        ScenarioPOIVector const* GetScenarioPOIVector(uint32 criteriaTreeId)
        {
            return Trinity::Containers::MapGetValuePtr(_scenarioPOIStore, criteriaTreeId);
        }

        VehicleAccessoryList const* GetVehicleAccessoryList(Vehicle* veh) const;
        VehicleAttachmentOffset const* GetVehicleAttachmentOffset(Vehicle* veh, int8 SeatId = 0) const;

        DungeonEncounterList const* GetDungeonEncounterList(uint32 mapId, Difficulty difficulty);
        uint16 GetDungeonEncounterByCreature(uint32 creatureId);
        uint32 GetCreatureByDungeonEncounter(uint32 EncounterID);

        float GetMapDifficultyStat(uint32 mapId, uint8 difficultyId);

        void LoadCurrencysLoot();

        bool LoadTrinityStrings(char const* table, int32 min_value, int32 max_value);
        bool LoadTrinityStrings();
        void LoadCreatureClassLevelStats();
        void LoadWorldVisibleDistance();
        void LoadWorldMapDiffycultyStat();
        void LoadCreatureLocales();
        void LoadCreatureDifficultyStat();
        void LoadCreatureTemplates();
        void LoadWDBCreatureTemplates();
        void LoadCreatureTemplateAddons();
        void CheckCreatureTemplate(CreatureTemplate const* cInfo);
        void CheckCreatureTemplateWDB(CreatureTemplate* cInfo);
        void RestructCreatureGUID();
        void RestructGameObjectGUID();
        void LoadTempSummons();
        void LoadCreatures();
        void LoadCreatureAIInstance();
        void LoadCreatureActionData();
        void LoadDisplayChoiceData();
    void LoadPlayerChoicesLocale();
    void LoadLinkedRespawn();
        bool SetCreatureLinkedRespawn(ObjectGuid::LowType const& guid, ObjectGuid::LowType const& linkedGuid);
        void LoadCreatureAddons();
        void LoadCreatureModelInfo();
        void LoadEquipmentTemplates();
        void LoadGameObjectLocales();
        void LoadGameobjects();
        void LoadItemTemplates();
        void LoadItemTemplateAddon();
        void LoadItemScriptNames();
        void LoadPageTextLocales();
        void LoadInstanceTemplate();
        void LoadInstanceEncounters();
        void LoadMailLevelRewards();
        void LoadVehicleTemplateAccessories();
        void LoadPersonalLootTemplate();
        void LoadWorldRateInfo();
        void LoadSeamlessTeleportData();

        void LoadNPCText();
        void LoadAccessRequirements();

        void LoadGameObjectActionData();

        void LoadPageTexts();
        PageText const* GetPageText(uint32 pageEntry);

        void LoadPlayerInfo();
        void LoadPetStats();
        void LoadExplorationBaseXP();
        void LoadPetNames();
        void LoadPetNumber();
        void LoadCorpses();
        void LoadFishingBaseSkillLevel();

        void LoadSkillTiers();
        void LoadReputationRewardRate();
        void LoadReputationOnKill();
        void LoadReputationSpilloverTemplate();

        void LoadScenarioPOI();

        void LoadNPCSpellClickSpells();

        void LoadGameTele();

        void LoadVendors();
        void LoadDonateVendors();
        void LoadTrainerSpell();
        void AddSpellToTrainer(uint32 entry, uint32 spell, uint32 spellCost, uint32 reqSkill, uint32 reqSkillValue, uint32 reqLevel);

        void LoadPhaseDefinitions();
        void LoadSpellPhaseInfo();

        void LoadResearchSiteToZoneData();
        void LoadDigSitePositions();

        void LoadScenarioData();
        void LoadScenarioSpellData();
        
        void LoadCreatureOutfits();
        
        void LoadDeathMatchStore();

        PhaseDefinitionStore const* GetPhaseDefinitionStore() { return &_PhaseDefinitionStore; }
        SpellPhaseStore const* GetSpellPhaseStore() { return &_SpellPhaseStore; }

        std::string GeneratePetName(uint32 entry);
        uint32 GetBaseXP(uint8 level);
        uint32 GetXPForLevel(uint8 level) const;

        int32 GetFishingBaseSkillLevel(uint32 entry) const;

        SkillTiersEntry const* GetSkillTier(uint32 skillTierId) const;

        void ReturnOrDeleteOldMails(bool serverUp);

        CreatureBaseStats const* GetCreatureBaseStats(uint8 level, uint8 unitClass);

        void SetHighestGuids();
        template<HighGuid type>
        ObjectGuidGenerator<type>* GetGenerator();
        uint32 GenerateAuctionID();
        uint64 GenerateEquipmentSetGuid();
        uint32 GenerateMailID();
        uint32 GeneratePetNumber();
        uint64 GenerateVoidStorageItemId();
        uint64 GenerateReportComplaintID();
        uint64 GenerateSupportTicketSubmitBugID();

        MailLevelReward const* GetMailLevelReward(uint32 level, uint32 raceMask);

        CellObjectGuids const* GetCellObjectGuids(uint16 mapid, uint8 spawnMode, uint32 cell_id) const;
        CellObjectGuidsMap const* GetMapObjectGuids(uint16 mapid, uint8 spawnMode) const;

        std::vector<TempSummonData> const* GetSummonGroup(uint32 summonerId, SummonerType summonerType, uint8 group) const;


        CreatureData const* GetCreatureData(ObjectGuid::LowType const& guid) const
        {
            return Trinity::Containers::MapGetValuePtr(_creatureDataStore, guid);
        }

        CreatureDataContainer const* GetCreatures() const
        {
            return &_creatureDataStore;
        }

        CreatureAIInstance const* GetCreatureAIInstaceData(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_creatureAIInstance, entry);
        }

        CreatureData& NewOrExistCreatureData(ObjectGuid::LowType const& guid) { return _creatureDataStore[guid]; }
        void DeleteCreatureData(ObjectGuid::LowType const& guid);

        PersonalLootData const* GetPersonalLootData(uint32 id, uint32 type = 0) const
        {
            return Trinity::Containers::MapGetValuePtr(_PersonalLootStore[type], id);
        }

        PersonalLootData const* GetPersonalLootDataBySpell(uint32 spellId) const
        {
            return Trinity::Containers::MapGetValuePtr(_PersonalLootBySpellStore, spellId);
        }

    ObjectGuid GetLinkedRespawnGuid(ObjectGuid const& guid) const;

        CreatureLocale const* GetCreatureLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_creatureLocaleStore, entry);
        }

        GameObjectLocale const* GetGameObjectLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_gameObjectLocaleStore, entry);
        }

        NpcTextLocale const* GetNpcTextLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_npcTextLocaleStore, entry);
        }

        PageTextLocale const* GetPageTextLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_pageTextLocaleStore, entry);
        }

        GameObjectData const* GetGOData(ObjectGuid::LowType const& guid) const
        {
            return Trinity::Containers::MapGetValuePtr(_gameObjectDataStore, guid);
        }

        GameObjectData& NewGOData(ObjectGuid::LowType const& guid) { return _gameObjectDataStore[guid]; }
        void DeleteGOData(ObjectGuid::LowType const& guid);

        TrinityStringLocale const* GetTrinityStringLocale(int32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_trinityStringLocaleStore, entry);
        }

        const char *GetTrinityString(int32 entry, LocaleConstant locale_idx) const;
        const char *GetTrinityStringForDBCLocale(int32 entry) const { return GetTrinityString(entry, DBCLocaleIndex); }
        LocaleConstant GetDBCLocaleIndex() const { return DBCLocaleIndex; }
        void SetDBCLocaleIndex(LocaleConstant locale) { DBCLocaleIndex = locale; }

        const std::vector<CreatureActionData>* GetCreatureActionData(uint32 entry, uint8 action = 0) const;

        const SeamlessTeleportData* GetSeamlessTeleportZone(uint16 zoneId) const;
        const SeamlessTeleportData* GetSeamlessTeleportArea(uint16 areaId) const;

        PlayerChoice const* GetPlayerChoice(int32 choiceId) const;
        PlayerChoiceLocale const* GetPlayerChoiceLocale(int32 choiceId) const;

        const std::vector<GameObjectActionData>* GetGameObjectActionData(uint32 entry) const;
        std::vector<uint32> GetItemBonusTree(uint32 ItemID, uint32 itemBonusTreeMod, uint32 ownerLevel, int32 levelBonus = 0, int32 challengeLevel = 0, int32 needLevel = 0, bool onlyBonus = false);
        std::vector<uint32> GetItemBonusForLevel(uint32 ItemID, uint32 itemBonusTreeMod, uint32 ownerLevel, int32 needLevel = 0);
        uint32 GetItemBonusLevel(uint32 ItemID, uint32 ownerLevel, uint8& Quality, std::vector<uint32>& bonusListIDs);
        uint32 FindChallengeLevel(std::vector<uint32>& bonusListIDs);
        uint32 ReplaceChallengeLevel(std::vector<uint32>& bonusListIDs, int32 challengeLevel);
        void DeleteBonusUpLevel(std::vector<uint32>& bonusListIDs);
        void DeleteBugBonus(std::vector<uint32>& bonusListIDs, bool canSecond, bool canThird, bool hasStat);
        bool CheckDuplicateBonusUpLevel(std::vector<uint32>& bonusListIDs);

        void AddCorpseCellData(uint32 mapid, uint32 cellid, ObjectGuid player_guid, uint32 instance);
        void DeleteCorpseCellData(uint32 mapid, uint32 cellid, ObjectGuid player_guid);

        // grid objects
        void AddEventObjectToGrid(ObjectGuid::LowType const& guid, EventObjectData const* data);
        void AddConversationToGrid(ObjectGuid::LowType const& guid, ConversationSpawnData const* data);
        void AddCreatureToGrid(ObjectGuid::LowType const& guid, CreatureData const* data);
        void RemoveCreatureFromGrid(ObjectGuid::LowType const& guid, CreatureData const* data);
        void AddGameobjectToGrid(ObjectGuid::LowType const& guid, GameObjectData const* data);
        void RemoveGameobjectFromGrid(ObjectGuid::LowType const& guid, GameObjectData const* data);
        ObjectGuid::LowType AddGOData(uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0, float rotation0 = 0, float rotation1 = 0, float rotation2 = 0, float rotation3 = 0, uint32 aid = 0);
        ObjectGuid::LowType AddCreData(uint32 entry, uint32 team, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0);
        bool MoveCreData(ObjectGuid::LowType const& guid, uint32 map, Position pos);

        GameTele const* GetGameTele(uint32 id) const
        {
            return Trinity::Containers::MapGetValuePtr(_gameTeleStore, id);
        }
        GameTele const* GetGameTele(std::string const& name) const;
        GameTeleContainer const& GetGameTeleMap() const { return _gameTeleStore; }
        bool AddGameTele(GameTele& data);
        bool DeleteGameTele(std::string const& name);
        
        const CreatureOutfitContainer& GetCreatureOutfitMap() const { return _creatureOutfitStore; }

        TrainerSpellData const* GetNpcTrainerSpells(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_cacheTrainerSpellStore, entry);
        }

        VendorItemData const* GetNpcVendorItemList(uint32 entry) const;
        VendorItemData const* GetNpcDonateVendorItemList(int32 entry) const;
        DonVenCatBase const* GetDonateVendorCat(int32 entry) const;
        bool IsActivateDonateService() const;
        std::vector<int32> const* GetFakeDonateVendorCat(int32 entry) const;
        int32 const* GetRealDonateVendorCat(int32 entry) const;
        DonVenAdd const* GetDonateVendorAdditionalInfo(uint32 type, uint32 entry) const;
        std::vector<DeathMatchStore> const* GetDeathMatchStore(uint8 type) const;
        std::vector<DeathMatchStore> const* GetDeathMatchStoreById(uint32 id) const;
        float const* GetPriceForLevelUp(uint8 level);
        float const* GetPriceForArtLevelUp(uint8 level);
        PriceForAddBonus const* GetPricesForAddBonus();
        const float& GetDonateDiscount() const;

        void AddVendorItem(uint32 entry, uint32 item, int32 maxcount, uint32 incrtime, uint32 extendedCost, uint8 type, uint64 money, bool persist = true); // for event
        bool RemoveVendorItem(uint32 entry, uint32 item, uint8 type, bool persist = true); // for event
        bool IsVendorItemValid(uint32 vendor_entry, uint32 id, int32 maxcount, uint32 ptime, uint32 ExtendedCost, uint8 type, Player* player = nullptr, std::set<uint32>* skip_vendors = nullptr, uint32 ORnpcflag = 0) const;

        void LoadScriptNames();
        ScriptNameContainer& GetScriptNames();
        std::string const& GetScriptName(uint32 id) const;
        uint32 GetScriptId(const char *name);

        SpellClickInfoMapBounds GetSpellClickInfoMapBounds(uint32 creature_id) const;

        // for wintergrasp only
        GraveYardContainer GraveYardStore;

        static void AddLocaleString(std::string&& value, LocaleConstant localeConstant, StringVector& data);
        static void GetLocaleString(StringVector const& data, LocaleConstant localeConstant, std::string& value);

        GameObjectDataContainer _gameObjectDataStore;
        //Get item count from spawnmode
        uint8 GetCountFromSpawn(uint8 spawnmode, uint32 expansion);

        bool IsTransportMap(uint32 mapId) const { return _transportMaps.count(mapId) != 0; }

        void LoadRaceAndClassExpansionRequirements();
        void LoadRealmNames();

        std::string GetRealmName(uint32 realm) const;
        std::string GetNormalizedRealmName(uint32 realm) const;
        std::string GetRealCharName(std::string name);

        std::unordered_map<uint8, RaceUnlockRequirement> const& GetRaceUnlockRequirements() const { return _raceUnlockRequirementStore; }
        RaceUnlockRequirement const* GetRaceUnlockRequirement(uint8 race) const;
        ExpansionRequirementContainer const& GetClassExpansionRequirements() const;
        uint8 GetClassExpansionRequirement(uint8 class_) const;

        //Get boundType from difficulty
        uint8 GetboundTypeFromDifficulty(uint8 difficulty);

        bool HasScenarioInMap(uint32 mapId) const
        {
            return _scenarioDataList.find(mapId) != _scenarioDataList.end();
        }

        ScenarioData const* GetScenarioOnMap(uint32 mapId, uint32 difficultyID = 0, uint32 factionID = 0, uint32 pl_class = 0, uint32 dungeonID = 0) const;
        
        std::vector<ScenarioSpellData> const* GetScenarioSpells(int32 ScenarioId) const
        {
            if (_scenarioDataSpellStep.size() <= ScenarioId)
                return nullptr;
            return &_scenarioDataSpellStep[ScenarioId];
        }

        std::vector<ItemTemplate const*> LegendaryItemList;
        std::vector<ItemTemplate const*> LegendaryTokenList;
        uint32 GetRandomLegendaryItem(Player* lootOwner) const;
        uint32 GetCreatureDisplay(int32 modelid) const;

    private:
        // first free id for selected id type
        uint32 _auctionId;
        uint64 _equipmentSetGuid;
        uint32 _itemTextId;
        uint32 _mailId;
        uint32 _hiPetNumber;
        ObjectGuid::LowType _voidItemId;
        uint64 _reportComplaintID;
        uint64 _supportTicketSubmitBugID;
        float donateDiscount = 1;

        // first free low guid for selected guid type
        ObjectGuidGenerator<HighGuid::Player> _playerGuidGenerator;
        ObjectGuidGenerator<HighGuid::Creature> _creatureGuidGenerator;
        ObjectGuidGenerator<HighGuid::Pet> _petGuidGenerator;
        ObjectGuidGenerator<HighGuid::Vehicle> _vehicleGuidGenerator;
        ObjectGuidGenerator<HighGuid::Item> _itemGuidGenerator;
        ObjectGuidGenerator<HighGuid::GameObject> _gameObjectGuidGenerator;
        ObjectGuidGenerator<HighGuid::DynamicObject> _dynamicObjectGuidGenerator;
        ObjectGuidGenerator<HighGuid::Corpse> _corpseGuidGenerator;
        ObjectGuidGenerator<HighGuid::LootObject> _lootObjectGuidGenerator;
        ObjectGuidGenerator<HighGuid::AreaTrigger> _areaTriggerGuidGenerator;
        ObjectGuidGenerator<HighGuid::Transport> _moTransportGuidGenerator;
        ObjectGuidGenerator<HighGuid::BattlePet> _BattlePetGuidGenerator;
        ObjectGuidGenerator<HighGuid::PetBattle> _PetBattleGuidGenerator;
        ObjectGuidGenerator<HighGuid::Conversation> _conversationGuidGenerator;
        ObjectGuidGenerator<HighGuid::Cast> _castGuidGenerator;
        ObjectGuidGenerator<HighGuid::LFGList> _LFGListGuidGenerator;
        ObjectGuidGenerator<HighGuid::LFGObject> _LFGObjectGuidGenerator;
        ObjectGuidGenerator<HighGuid::EventObject> _EventObjectGuidGenerator;
        ObjectGuidGenerator<HighGuid::Scenario> _scenarioGuidGenerator;
        ObjectGuidGenerator<HighGuid::Vignette> _vignetteGuidGenerator;


        NpcTextContainer _npcTextStore;
        AccessRequirementContainer _accessRequirementStore;
        DungeonEncounterContainer _dungeonEncounterStore;
        DungeonEncounterVector _dungeonEncounterVector[MAX_DIFFICULTY];
        CreatureToDungeonEncounterMap _creatureToDungeonEncounter;
        DungeonEncounterToCreatureMap _dungeonEncounterToCreature;
        InstanceGraveYardContainer _instanceGraveYardStore;

        RepRewardRateContainer _repRewardRateStore;
        RepOnKillContainer _repOnKillStore;
        RepSpilloverTemplateContainer _repSpilloverTemplateStore;

        ScenarioPOIContainer _scenarioPOIStore;


        GameTeleContainer _gameTeleStore;

        ScriptNameContainer _scriptNamesStore;

        SpellClickInfoContainer _spellClickInfoStore;

        VehicleAccessoryTemplateContainer _vehicleTemplateAccessoryStore;
        VehicleAccessoryContainer _vehicleAccessoryStore;
        VehicleAttachmentOffsetContainer _vehicleAttachmentOffsetStore;

        LocaleConstant DBCLocaleIndex;

        PageTextContainer _pageTextStore;
        InstanceTemplateContainer _instanceTemplateStore;
        InstanceTemplateVector _instanceTemplateVector;
        
        CreatureOutfitContainer _creatureOutfitStore;

        PhaseDefinitionStore _PhaseDefinitionStore;
        SpellPhaseStore _SpellPhaseStore;

        uint32 _skipUpdateCount;
        void PlayerCreateInfoAddItemHelper(uint32 race_, uint32 class_, uint32 itemId, int32 count, std::vector<uint32> bonusListIDs);
        void PlayerCreateInfoAddQuestHelper(uint32 race_, uint32 class_, uint32 questId);
        void PlayerCreateInfoAddSpellHelper(uint32 race_, uint32 class_, uint32 spellId);
        void PlayerCreateInfoAddActionHelper(uint32 race_, uint32 class_, PlayerCreateInfoAction action);

        MailLevelRewardContainer _mailLevelRewardStore;

        CreatureBaseStatsContainer _creatureBaseStatsStore;

        PetStatsContainer _petStatsStore;                            // [creature_id][level]

        void BuildPlayerLevelInfo(uint8 race, uint8 class_, uint8 level, PlayerLevelInfo* plinfo) const;

        PlayerInfo* _playerInfo[MAX_RACES][MAX_CLASSES];

        PlayerXPperLevel _playerXPperLevel;

        BaseXPContainer _baseXPTable;

        FishingBaseSkillContainer _fishingBaseForAreaStore;
        std::unordered_map<uint32, SkillTiersEntry> _skillTiers;

        HalfNameContainer _petHalfName0;
        HalfNameContainer _petHalfName1;

        MapObjectGuids _mapObjectGuidsStore;

        CreatureDataContainer _creatureDataStore;
        CreatureTemplateContainer _creatureTemplateStore;
        CreatureTemplateContainerMap _creatureTemplateStoreMap;
        CreatureDifficultyStatContainer _creatureDifficultyStatStore;
        CreatureModelContainer _creatureModelStore;
        CreatureAddonContainer _creatureAddonStore;
        CreatureTemplateAddonContainer _creatureTemplateAddonStore;
        CreatureAIInstanceContainer _creatureAIInstance;
        EquipmentInfoContainer _equipmentInfoStore;
        LinkedRespawnContainer _linkedRespawnStore;
        CreatureLocaleContainer _creatureLocaleStore;
        GameObjectLocaleContainer _gameObjectLocaleStore;
        GameObjectTemplateContainer _gameObjectTemplateStore;
        PersonalLootContainer _PersonalLootStore[MAX_LOOT_COOLDOWN_TYPE];
        PersonalLootContainer _PersonalLootBySpellStore;
        TempSummonDataContainer _tempSummonDataStore;
        std::vector<std::vector<float>> _mapDifficultyStat;

        ItemTemplateContainer _itemTemplateStore;
        NpcTextLocaleContainer _npcTextLocaleStore;
        PageTextLocaleContainer _pageTextLocaleStore;
        TrinityStringLocaleContainer _trinityStringLocaleStore;

        CacheVendorItemContainer _cacheVendorItemStore;
        CacheDonateVendorItemContainer _cacheDonateVendorItemStore;
        CacheTrainerSpellContainer _cacheTrainerSpellStore;
        PriceForLevelUp _priceforlevelupStore;
        PriceForArtLevelUp _priceforArtlevelupStore;
        PriceForAddBonus _priceforAddBonusStore;
        mutable std::recursive_mutex m_donate_lock;
        std::atomic<bool> m_donate_waite;

        std::unordered_map<uint8, RaceUnlockRequirement> _raceUnlockRequirementStore;
        ExpansionRequirementContainer _classExpansionRequirementStore;
        RealmNameContainer _realmNameStore;

        enum CreatureLinkedRespawnType
        {
            CREATURE_TO_CREATURE,
            CREATURE_TO_GO,         // Creature is dependant on GO
            GO_TO_GO,
            GO_TO_CREATURE,         // GO is dependant on creature
        };


        ScenarioDataMap _scenarioData;
        ScenarioDataListMap _scenarioDataList;
        ScenarioDataSpellsStep _scenarioDataSpellStep;

        CreatureActionDataMap _creatureActionDataList[CREATURE_ACTION_TYPE_MAX];

        SeamlessTeleportMap _seamlessZoneTeleport;
        SeamlessTeleportMap _seamlessAreaTeleport;

        std::unordered_map<int32 /*choiceId*/, PlayerChoice> _playerChoices;
        std::unordered_map<int32, PlayerChoiceLocale> _playerChoiceLocales;

        GameObjectActionMap _gameObjectActionMap;

        DonateVendorCat _donvendorcat;
        FakeDonateVendorCat _fakedonvendorcat;
        ReversFakeDonateVendorCat _reversfakedonvendorcat;
        DonVenAdds _donvenadds;
        
        DeathMatchProducts _dmProducts;
        DeathMatchProductsById _dmProductsById;
        CurrencysLoot  _currencysLoot;

        std::set<uint32> _transportMaps; // Helper container storing map ids that are for transports only, loaded from gameobject_template
};

#define sObjectMgr ObjectMgr::instance()

// scripting access functions
bool LoadTrinityStrings(char const* table, int32 start_value = MAX_CREATURE_AI_TEXT_STRING_ID, int32 end_value = std::numeric_limits<int32>::min());

#endif
