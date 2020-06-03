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
#ifndef __TRINITY_ACHIEVEMENTMGR_H
#define __TRINITY_ACHIEVEMENTMGR_H

#include "Common.h"
#include "DBCEnums.h"
#include "DB2Stores.h"
#include "ObjectGuid.h"
#include "DatabaseEnvFwd.h"
#include <safe_ptr.h>

#include <cds/container/feldman_hashmap_hp.h>
#include "HashFuctor.h"

enum CriteriaTreeCustomFlags : uint16
{
    CRITERIA_TREE_CUSTOM_FLAG_QUEST = 0x01,  // custom flags for quest
};

struct ModifierTreeNode
{
    std::vector<ModifierTreeNode const*> Children;
    ModifierTreeEntry const* Entry;
};

typedef std::vector<AchievementEntry const*> AchievementEntryList;
typedef std::vector<AchievementEntryList> AchievementListByReferencedId;

struct Criteria
{
    CriteriaEntry const* Entry = nullptr;
    ModifierTreeNode const* Modifier = nullptr;
    uint32 ID = 0;
    uint32 FlagsCu = 0;
};

struct CriteriaProgress
{
    CriteriaProgress();

    ObjectGuid PlayerGUID;      // GUID of the player that completed this criteria (guild achievements)
    uint64 Counter;
    time_t date;                // latest update time.
    bool changed;
    bool updated;
    bool completed;
    bool deactiveted;
    bool deleted;

    AchievementEntry const* achievement;
    CriteriaTreeEntry const* criteriaTree;
    CriteriaTreeEntry const* parent;
    CriteriaEntry const* criteria;
    Criteria const* _criteria;
};

struct CriteriaTree
{
    std::vector<CriteriaTree const*> Children;
    CriteriaTreeEntry const* Entry = nullptr;
    AchievementEntry const* Achievement = nullptr;
    ScenarioStepEntry const* ScenarioStep = nullptr;
    struct Criteria const* Criteria = nullptr;
    uint32 ID = 0;
    uint32 CriteriaID = 0;
    uint32 Flags = 0;
};

typedef std::vector<CriteriaTree const*> CriteriaTreeList;

enum AchievementCriteriaDataType
{                                                           // value1         value2        comment
    ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE                = 0, // 0              0
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE          = 1, // creature_id    0
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE = 2, // class_id       race_id
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH= 3, // health_percent 0
    ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA              = 5, // spell_id       effect_idx
    ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AREA              = 6, // area id        0
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA              = 7, // spell_id       effect_idx
    ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE               = 8, // minvalue                     value provided with achievement update must be not less that limit
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL             = 9, // minlevel                     minlevel of target
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER            = 10, // gender                       0=male; 1=female
    ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT              = 11, // scripted requirement
    // REUSE
    ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT    = 13, // count                        "with less than %u people in the zone"
    ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM              = 14, // team                         HORDE(67), ALLIANCE(469)
    ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK             = 15, // drunken_state  0             (enum DrunkenState) of player
    ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY             = 16, // holiday_id     0             event in holiday time
    ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE  = 17, // min_score      max_score     player's team win bg and opposition team have team score in range
    ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT          = 18, // 0              0             maker instance script call for check current criteria requirements fit
    ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM      = 19, // item_level     item_quality  for equipped item in slot to check item level and quality
    ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE = 21, // class_id       race_id
};

#define MAX_ACHIEVEMENT_CRITERIA_DATA_TYPE               22 // maximum value in AchievementCriteriaDataType enum
class Player;
class Unit;
class WorldPacket;


class AchievementCache
{
public:

    AchievementCache(Player* _player, Unit* _target = nullptr, CriteriaTypes _type = CRITERIA_TYPE_KILL_CREATURE, uint32 _miscValue1 = 0, uint32 _miscValue2 = 0, uint32 _miscValue3 = 0);

    bool HasTarget() { return target.Init; }
    bool IsCreature() { return target.Entry != 0; }
    uint32 GetEntry() { return target.Entry; }

    Player* player = nullptr;
    CriteriaTypes type = CRITERIA_TYPE_KILL_CREATURE;
    ObjectGuid ownerGuid;
    ObjectGuid targetGuid;
    float HealthPct = 0.0f;
    uint32 miscValue1 = 0;
    uint32 miscValue2 = 0;
    uint32 miscValue3 = 0;
    uint32 InstanceId = 0;
    uint32 guildId = 0;
    uint32 Team = 0;
    uint16 ZoneID = 0;
    uint16 AreaID = 0;
    uint16 PlayersCount = 0;
    uint16 MapID = 0;
    uint16 MountCount = 0;
    uint16 BattleCount = 0;
    uint16 ToysCount = 0;
    uint16 UniquePets = 0;
    uint8 ClassID = 0;
    uint8 RaceID = 0;
    uint8 Level = 0;
    uint8 Gender = 0;
    uint8 TeamID = 0;
    uint8 DrunkValue = 0;
    uint8 JoinType = 0;
    uint8 Difficulty = 0;
    uint8 MembersCount = 0;
    bool OnBG = false;
    bool IsArena = false;
    bool IsRated = false;
    bool IsRBG = false;
    bool HaveGroup = false;
    bool isLFGGroup = false;
    bool IsGuildGroup = false;

    struct TargetInfo
    {
        Unit* unit = nullptr;
        uint32 Entry = 0;
        uint16 ZoneID = 0;
        uint16 AreaID = 0;
        uint8 ClassID = 0;
        uint8 RaceID = 0;
        uint8 Level = 0;
        uint8 Gender = 0;
        uint8 Team = 0;
        float HealthPct = 0.0f;
        bool isAlive = true;
        bool IsMounted = false;
        bool Init = false;
        bool isCreature = false;
        bool isYields = false;
    } target;
};

typedef std::shared_ptr<AchievementCache> AchievementCachePtr;

struct AchievementCriteriaData
{
    AchievementCriteriaDataType dataType;
    union
    {
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_NONE              = 0 (no data)
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_CREATURE        = 1
        struct
        {
            uint32 id;
        } creature;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_CLASS_RACE = 2
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_S_PLAYER_CLASS_RACE = 21
        struct
        {
            uint32 class_id;
            uint32 race_id;
        } classRace;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_PLAYER_LESS_HEALTH = 3
        struct
        {
            uint32 percent;
        } health;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AURA            = 5
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_AURA            = 7
        struct
        {
            uint32 spell_id;
            uint32 effect_idx;
        } aura;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_S_AREA            = 6
        struct
        {
            uint32 id;
        } area;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_VALUE             = 8
        struct
        {
            uint32 minvalue;
        } value;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_LEVEL           = 9
        struct
        {
            uint32 minlevel;
        } level;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_GENDER          = 10
        struct
        {
            uint32 gender;
        } gender;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_SCRIPT            = 11 (no data)
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_MAP_PLAYER_COUNT  = 13
        struct
        {
            uint32 maxcount;
        } map_players;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_T_TEAM            = 14
        struct
        {
            uint32 team;
        } team;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_S_DRUNK           = 15
        struct
        {
            uint32 state;
        } drunk;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_HOLIDAY           = 16
        struct
        {
            uint32 id;
        } holiday;
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_BG_LOSS_TEAM_SCORE= 17
        struct
        {
            uint32 min_score;
            uint32 max_score;
        } bg_loss_team_score;
        // ACHIEVEMENT_CRITERIA_DATA_INSTANCE_SCRIPT        = 18 (no data)
        // ACHIEVEMENT_CRITERIA_DATA_TYPE_S_EQUIPED_ITEM    = 19
        struct
        {
            uint32 item_level;
            uint32 item_quality;
        } equipped_item;
        // raw
        struct
        {
            uint32 value1;
            uint32 value2;
        } raw;
    };
    uint32 ScriptId;

    AchievementCriteriaData();
    AchievementCriteriaData(uint32 _dataType, uint32 _value1, uint32 _value2, uint32 _scriptId);

    bool IsValid(CriteriaEntry const* criteria);
    bool Meets(uint32 criteria_id, AchievementCachePtr cachePtr) const;
};

struct AchievementCriteriaDataSet
{
        AchievementCriteriaDataSet();

        typedef std::vector<AchievementCriteriaData> Storage;
        void Add(AchievementCriteriaData const& data);
        bool Meets(AchievementCachePtr cachePtr) const;
        void SetCriteriaId(uint32 id);
    private:
        uint32 criteria_id;
        Storage storage;
};

typedef std::unordered_map<uint32, AchievementCriteriaDataSet> AchievementCriteriaDataMap;
typedef std::vector<AchievementCriteriaDataSet*> AchievementCriteriaDataVector;

struct AchievementReward
{
    uint32 titleId[MAX_TEAMS];
    bool genderTitle;
    uint32 learnSpell;
    uint32 castSpell;
    uint32 itemId;
    uint32 sender;
    uint32 ScriptId;
    std::string subject;
    std::string text;
};

typedef std::unordered_map<uint32, AchievementReward> AchievementRewards;
typedef std::vector<AchievementReward*> AchievementRewardVector;

struct AchievementRewardLocale
{
    StringVector subject;
    StringVector text;
};

typedef std::unordered_map<uint32, AchievementRewardLocale> AchievementRewardLocales;

struct CompletedAchievementData
{
    CompletedAchievementData();

    GuidSet guids;
    time_t date;
    uint64 first_guid;
    bool changed;
    bool isAccountAchievement;
};

typedef cds::container::FeldmanHashMap< cds::gc::HP, uint32, CriteriaProgress, uint32Traits > CriteriaProgressMap;
typedef std::unordered_map<uint32, CompletedAchievementData> CompletedAchievementMap;

enum CriteriaSort
{
    PLAYER_CRITERIA     = 0,
    GUILD_CRITERIA      = 1,
    SCENARIO_CRITERIA   = 2,
};

template<class T>
class AchievementMgr
{
    public:
        AchievementMgr(T* owner);
        ~AchievementMgr();

        void Reset();
        void ClearMap();
        uint32 GetSize();
        static void DeleteFromDB(ObjectGuid lowguid, uint32 accountId = 0);
        void LoadFromDB(PreparedQueryResult achievementResult, PreparedQueryResult criteriaResult, PreparedQueryResult achievementAccountResult = nullptr, PreparedQueryResult criteriaAccountResult = nullptr);
        void SaveToDB(SQLTransaction& trans);
        void ResetAchievementCriteria(CriteriaTypes type, uint32 miscValue1 = 0, uint32 miscValue2 = 0, bool evenIfCriteriaComplete = false);
        void UpdateAchievementCriteria(AchievementCachePtr cachePtr, bool init = false);
        void UpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 = 0, uint32 miscValue2 = 0, uint32 miscValue3 = 0,Unit* unit = nullptr, Player* referencePlayer = nullptr, bool init = false);
        void CompletedAchievement(AchievementEntry const* achievement, Player* referencePlayer);
        bool IsCompletedAchievement(AchievementEntry const* achievement, Player* referencePlayer);
        void CheckAllAchievementCriteria(Player* referencePlayer);
        void SendAllAchievementData(Player* receiver);
        void SendAllAccountCriteriaData(Player* receiver);
        void SendAchievementInfo(Player* receiver, uint32 achievementId = 0);
        bool HasAchieved(uint32 achievementId, uint64 guid = 0);
        bool HasAccountAchieved(uint32 achievementId);
        uint64 GetFirstAchievedCharacterOnAccount(uint32 achievementId);
        T* GetOwner() const;

        void UpdateTimedAchievements(uint32 timeDiff);
        void StartTimedAchievement(CriteriaTimedTypes type, uint32 entry, uint16 timeLost = 0);
        void RemoveTimedAchievement(CriteriaTimedTypes type, uint32 entry);   // used for quest and scripted timed achievements
        uint32 GetAchievementPoints() const;
        CriteriaSort GetCriteriaSort() const;
        bool IsCompletedCriteria(CriteriaTree const* tree, uint64 requiredAmount);
        bool IsCompletedCriteriaTree(CriteriaTree const* tree);
        bool CanUpdateCriteriaTree(CriteriaTree const* tree, Player* referencePlayer);
        bool IsCompletedScenarioTree(CriteriaTreeEntry const* criteriaTree);
        bool CanUpdate();

        CompletedAchievementMap const* GetCompletedAchievementsList();

        void SendAllTrackedCriterias(Player* receiver, std::set<uint32> const& trackedCriterias);

        CriteriaProgressMap const* GetCriteriaProgressMap();

        bool CheckModifierTree(uint32 modifierTreeId, Player* referencePlayer);
        bool CheckModifierTree(uint32 modifierTreeId, AchievementCachePtr cachePtr);
        void RemoveCriteriaProgress(CriteriaTree const* criteriaTree);

        uint64 m_canUpdateAchiev = 0;

    private:
        enum ProgressType { PROGRESS_SET, PROGRESS_ACCUMULATE, PROGRESS_HIGHEST };
        void SendAchievementEarned(AchievementEntry const* achievement);
        void SendCriteriaUpdate(CriteriaProgress const* progress, uint32 timeElapsed, bool timedCompleted) const;
        void SendAccountCriteriaUpdate(CriteriaProgress const* progress, uint32 timeElapsed, bool timedCompleted) const;

        CriteriaProgress* GetCriteriaProgress(uint32 entry, bool create = false);
        bool SetCriteriaProgress(CriteriaTree const* tree, uint32 changeValue, Player* referencePlayer, ProgressType ptype);
        bool CanCompleteCriteria(AchievementEntry const* achievement);

        bool CanUpdateCriteria(CriteriaTree const* tree, AchievementCachePtr cachePtr);

        void SendPacket(WorldPacket const* data) const;

        bool ConditionsSatisfied(Criteria const *criteria, Player* referencePlayer) const;
        bool RequirementsSatisfied(CriteriaTree const* tree, AchievementCachePtr cachePtr);
        bool AdditionalRequirementsSatisfied(ModifierTreeNode const* parent, AchievementCachePtr cachePtr);

        T* _owner;
        CriteriaProgressMap _criteriaProgress;
        CompletedAchievementMap _completedAchievements;

        std::recursive_mutex i_completedAchievementsLock;
        std::recursive_mutex i_timeCriteriaTreesLock;
        typedef std::map<uint32, uint32> TimedAchievementMap;
        TimedAchievementMap _timeCriteriaTrees;      // Criteria id/time left in MS
        uint32 _achievementPoints;
        uint32 _achievementBattlePetPoints;

        std::vector<CompletedAchievementData*> _completedAchievementsArr;
        std::vector<CriteriaProgress*> _criteriaProgressArr;
        std::vector<uint32*> _timeCriteriaTreesArr;
};

class AchievementGlobalMgr
{
        AchievementGlobalMgr() { }
        ~AchievementGlobalMgr() { }

    public:
        static char const* GetCriteriaTypeString(CriteriaTypes type);
        static char const* GetCriteriaTypeString(uint32 type);

        static AchievementGlobalMgr* instance();

        CriteriaTreeList const& GetCriteriaTreeByType(CriteriaTypes type, CriteriaSort sort) const;
        CriteriaTreeList const* GetCriteriaTreesByCriteria(uint32 criteriaId) const;
        CriteriaTreeList const& GetTimedCriteriaByType(CriteriaTimedTypes type) const;

        AchievementEntryList const* GetAchievementByReferencedId(uint32 id) const;
        AchievementReward const* GetAchievementReward(AchievementEntry const* achievement) const;
        AchievementRewardLocale const* GetAchievementRewardLocale(AchievementEntry const* achievement) const;
        AchievementCriteriaDataSet const* GetCriteriaDataSet(CriteriaTree const* Criteria) const;

        bool IsRealmCompleted(AchievementEntry const* achievement) const;
        void SetRealmCompleted(AchievementEntry const* achievement);
        bool IsGroupCriteriaType(CriteriaTypes type) const;

        template<typename Func>
        static void WalkCriteriaTree(CriteriaTree const* tree, Func const& func);

        void LoadCriteriaList();
        void LoadAchievementCriteriaData();
        void LoadAchievementReferenceList();
        void LoadCompletedAchievements();
        void LoadRewards();
        void LoadRewardLocales();
        uint32 GetParantTreeId(uint32 parent);
        CriteriaTree const* GetCriteriaTree(uint32 criteriaTreeId) const;
        Criteria const* GetCriteria(uint32 criteriaId) const;
        ModifierTreeNode const* GetModifierTree(uint32 modifierTreeId) const;

        std::vector<uint32> _criteriaTreeForQuest;

    private:
        AchievementCriteriaDataMap m_criteriaDataMap;
        AchievementCriteriaDataVector m_criteriaDataVector;

        std::vector<CriteriaTree*> _criteriaTrees;
        std::vector<Criteria*> _criteria;
        std::vector<ModifierTreeNode*> _criteriaModifiers;

        std::unordered_map<uint32, CriteriaTreeList> _criteriaTreeByCriteria;
        std::vector<CriteriaTreeList*> _criteriaTreeByCriteriaVector;

        // store achievement criterias by type to speed up lookup
        CriteriaTreeList _criteriasByType[CRITERIA_TYPE_TOTAL];
        CriteriaTreeList _guildCriteriasByType[CRITERIA_TYPE_TOTAL];
        CriteriaTreeList _scenarioCriteriasByType[CRITERIA_TYPE_TOTAL];

        CriteriaTreeList _criteriasByTimedType[CRITERIA_TIMED_TYPE_MAX];

        // store achievements by referenced achievement id to speed up lookup
        AchievementListByReferencedId m_AchievementListByReferencedId;

        typedef std::vector<bool>  AllCompletedAchievements;
        AllCompletedAchievements m_allCompletedAchievements;

        AchievementRewards m_achievementRewards;
        AchievementRewardVector m_achievementRewardVector;
        AchievementRewardLocales m_achievementRewardLocales;
};

#define sAchievementMgr AchievementGlobalMgr::instance()

#endif
