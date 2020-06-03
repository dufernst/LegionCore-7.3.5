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

#ifndef _QuestDataStoreh_
#define _QuestDataStoreh_

enum WorldQuestTypeReward
{
    WORLD_QUEST_TYPE_REWARD_NONE            = 0,
    WORLD_QUEST_TYPE_REWARD_ITEM            = 1,
    WORLD_QUEST_TYPE_REWARD_GOLD            = 2,
    WORLD_QUEST_TYPE_REWARD_CURRENCY        = 3,
    WORLD_QUEST_TYPE_REWARD_ARTIFACT_POWER  = 4,
    WORLD_QUEST_TYPE_REWARD_ARMOR           = 5,
    WORLD_QUEST_TYPE_REWARD_ARTIFACT_RELIC  = 6,
    WORLD_QUEST_TYPE_REWARD_RESOURCE        = 7,

    WORLD_QUEST_TYPE_REWARD_MAX
};

enum WorldQuestResetType
{
    WORLD_QUEST_2_HOURS     = 7200,
    WORLD_QUEST_3_HOURS     = 10800,
    WORLD_QUEST_6_HOURS     = 21600,
    WORLD_QUEST_12_HOURS    = 43200,
    WORLD_QUEST_18_HOURS    = 64800,
    WORLD_QUEST_DAY         = 86400,
    WORLD_QUEST_2_DAY       = 172800,
    WORLD_QUEST_3_DAY       = 259200,
    WORLD_QUEST_4_DAY       = 345600,
    WORLD_QUEST_WEEK        = 604800,
    WORLD_QUEST_2_WEEK      = 1209600,
};

typedef std::list<ItemTemplate const*> WorldQuestItemList;

struct WorldQuestTemplateItem
{
    uint32 ItemID = 0;
    uint32 ItemCountMin = 1;
    uint32 ItemCountMax = 1;
    float chance = 100.0f;
};

struct WorldQuestTemplate
{
    std::vector<uint32> ItemCAList;
    std::vector<WorldQuestTemplateItem> ItemResourceList;
    WorldQuestItemList ArmorList;
    uint32 CurrencyMin = 0;
    uint32 CurrencyMax = 0;
    uint32 CurrencyCount = 0;
    uint32 GoldMin = 0;
    uint32 GoldMax = 0;
    float Chance = 0.0f;
    uint16 ZoneID = 0;
    uint16 Currency = 0;
    uint16 CurrencyID = 0;
    uint16 CurrencyID_A = 0;
    uint16 CurrencyID_H = 0;
    uint16 MinItemLevel = 0;
    uint8 QuestInfoID = 0;
    uint8 BonusLevel = 0;
    uint8 modTreeID = 0;
    uint8 PrimaryID = 0;
    uint8 Min = 0;
    uint8 Max = 0;
    uint8 AllMax = 0;
    bool HasArmor = false;
    bool IsPvP = false;
};

struct WorldQuestUpdate
{
    std::vector<uint32> AreaIDs;
    WorldQuestTemplate const* worldQuest = nullptr;
    Quest const* quest = nullptr;
    uint32 QuestID = 0;
    uint32 Timer = 0;
    uint32 VariableID = 0;
    uint32 VariableID1 = 0;
    uint32 Value = 0;
    uint32 Value1 = 0;
    uint32 EventID = 0;
};

struct WorldQuestItem
{
    uint32 ItemIDH = 0;
    uint32 ItemIDA = 0;
    uint32 ItemCount = 1;
};

struct WorldQuestRecipe
{
    uint32 ItemID = 0;
    uint32 ItemCount = 1;
    uint32 NeedSpell = 0;
    uint32 NotNeedSpell = 0;
};

struct WorldQuest
{
    Quest const* quest = nullptr;
    WorldQuestTemplate const* worldQuest = nullptr;
    WorldQuestTypeReward typeReward = WORLD_QUEST_TYPE_REWARD_NONE;
    WorldQuestItem ItemList[MAX_CLASSES];
    WorldQuestRecipe const* Recipe;
    std::set<WorldQuestState> State;
    uint32 QuestID = 0;
    uint32 VariableID = 0;
    uint32 Value = 0;
    uint32 Timer = 0;
    uint32 StartTime = 0;
    uint32 ResetTime = 0;
    uint32 CurrencyID = 0;
    uint32 CurrencyCount = 0;
    uint32 Gold = 0;
};

struct QuestPOIPoint
{
    int32 X;
    int32 Y;

    QuestPOIPoint() : X(0), Y(0) { }
    QuestPOIPoint(int32 _X, int32 _Y) : X(_X), Y(_Y) { }
};

struct QuestPOI
{
    int32 BlobIndex;
    int32 ObjectiveIndex;
    int32 QuestObjectiveID;
    int32 QuestObjectID;
    int32 MapID;
    int32 WorldMapAreaID;
    int32 Floor;
    int32 Priority;
    int32 Flags;
    int32 WorldEffectID;
    int32 PlayerConditionID;
    int32 SpawnTrackingID;
    bool AlwaysAllowMergingBlobs;
    std::vector<QuestPOIPoint> points;

    QuestPOI();
    QuestPOI(int32 _BlobIndex, int32 _ObjectiveIndex, int32 _QuestObjectiveID, int32 _QuestObjectID, int32 _MapID, int32 _WorldMapAreaID, int32 _Foor, int32 _Priority, int32 _Flags, int32 _WorldEffectID, int32 _PlayerConditionID, int32 _SpawnTrackingID, bool _AlwaysAllowMergingBlobs);
};

struct PointOfInterest
{
    uint32 entry;
    float x;
    float y;
    uint32 icon;
    uint32 flags;
    uint32 data;
    std::string icon_name;
};

typedef std::map<uint8, std::vector<WorldQuestTemplate>> WorldQuestTemplateMap;
typedef std::map<uint8, std::vector<WorldQuestUpdate>> WorldQuestUpdateMap;
typedef std::map<uint32 /*ZoneID*/, std::set<WorldQuestUpdate const*>> WorldQuestUpdateSet;
typedef std::map<uint32 /*ZoneID*/, std::map<uint32 /*QuestID*/, WorldQuest>> WorldQuestMap;
typedef std::map<uint32 /*QuestID*/, WorldQuestRecipe> WorldQuestRecipeMap;
typedef std::unordered_map<uint32, std::set<Quest const*>> QuestAreaTaskMap;
typedef std::unordered_map<uint32, QuestTemplateLocale> QuestTemplateLocaleContainer;
typedef std::unordered_map<uint32, QuestRequestItemsLocale> QuestRequestItemsLocaleContainer;
typedef std::unordered_map<uint32, QuestOfferRewardLocale> QuestOfferRewardLocaleContainer;
typedef std::unordered_map<uint32, QuestObjectivesLocale> QuestObjectivesLocaleContainer;
typedef std::multiset<uint32> QuestObject;
typedef std::map<uint32, QuestObject> QuestStarter;
typedef std::multimap<uint32, uint32> QuestRelations;
typedef std::pair<QuestRelations::const_iterator, QuestRelations::const_iterator> QuestRelationBounds;
typedef std::vector<QuestPOI> QuestPOIVector;
typedef std::unordered_map<uint32, QuestPOIVector> QuestPOIContainer;
typedef std::unordered_map<uint32, Quest*> QuestMap;
typedef std::vector<Quest*> QuestVector;
typedef std::set<uint32> GameObjectForQuestContainer;
typedef std::multimap<int32, uint32> ExclusiveQuestGroups;
typedef std::unordered_map<uint32, PointOfInterestLocale> PointOfInterestLocaleContainer;
typedef std::unordered_map<uint32, PointOfInterest> PointOfInterestContainer;

class QuestDataStoreMgr
{
    QuestDataStoreMgr();
    ~QuestDataStoreMgr();

public:
    static QuestDataStoreMgr* instance();

    WorldQuestMap const* GetWorldQuestMap() const;
    WorldQuest const* GetWorldQuest(Quest const* Quest);
    std::set<Quest const*> const* GetWorldQuestTask(uint32 areaId) const;
    void LoadWorldQuestTemplates();
    void GenerateWorldQuestUpdate();
    void GenerateInvasionPointUpdate();
    void ClearWorldQuest();
    void LoadQuests();
    void LoadGameobjectQuestRelations();
    void LoadGameobjectInvolvedRelations();
    void LoadCreatureQuestRelations();
    void LoadCreatureInvolvedRelations();
    void LoadQuestTemplateLocale();
    void LoadQuestOfferRewardLocale();
    void LoadQuestRequestItemsLocale();
    void LoadQuestObjectivesLocale();
    void LoadPointsOfInterest();
    void LoadQuestPOI();
    void LoadPointOfInterestLocales();
    void LoadQuestRelations();
    void LoadGameObjectForQuests();

    Quest const* GetQuestTemplate(uint32 quest_id) const;
    uint32 GetMaxQuestID();
    QuestMap const& GetQuestTemplates() const;
    bool IsGameObjectForQuests(uint32 entry) const;
    QuestPOIVector const* GetQuestPOIVector(int32 QuestID);
    std::vector<QuestObjective> GetQuestObjectivesByType(QuestObjectiveType type);
    QuestRelations* GetGOQuestRelationMap();
    QuestRelationBounds GetGOQuestRelationBounds(uint32 go_entry);
    QuestRelationBounds GetGOQuestInvolvedRelationBoundsByQuest(uint32 questId);
    QuestRelationBounds GetGOQuestInvolvedRelationBounds(uint32 go_entry);
    QuestRelations* GetCreatureQuestRelationMap();
    QuestRelationBounds GetCreatureQuestRelationBounds(uint32 creature_entry);
    QuestRelationBounds GetCreatureQuestInvolvedRelationBoundsByQuest(uint32 questId);
    QuestRelationBounds GetCreatureQuestInvolvedRelationBounds(uint32 creature_entry);
    QuestRelationBounds GetAreaQuestRelationBounds(uint32 area);
    QuestTemplateLocale const* GetQuestLocale(uint32 entry) const;
    QuestRequestItemsLocale const* GetQuestRequestItemsLocale(uint32 entry) const;
    QuestOfferRewardLocale const* GetQuestOfferRewardLocale(uint32 entry) const;
    QuestObjectivesLocale const* GetQuestObjectivesLocale(uint32 entry) const;
    std::set<Quest const*> const* GetQuestTask(uint32 areaId) const;
    PointOfInterest const* GetPointOfInterest(uint32 id) const;
    PointOfInterestLocale const* GetPointOfInterestLocale(uint32 poi_id) const;
    WorldQuest const* GenerateNewWorldQuest(uint32 QuestID, uint32 VariableID = 0);
    void ResetWorldQuest(uint32 QuestID);

    ExclusiveQuestGroups mExclusiveQuestGroups;
    uint32 WorldLegionInvasionZoneID = 0;
    std::atomic<bool> needWait;

private:
    void LoadAreaQuestRelations();
    void LoadQuestRelationsHelper(QuestRelations& map, QuestStarter& _map, std::string const& table, bool starter, bool go);
    void AddWorldQuestTask(Quest const* quest);
    void RemoveWorldQuestTask(Quest const* quest);
    WorldQuestTemplate const* GetWorldQuestTemplate(Quest const* quest);
    WorldQuestUpdate const* GetWorldQuestUpdate(uint32 QuestID, uint32 QuestInfoID, uint32 VariableID = 0);
    void CalculateWorldQuestReward(WorldQuestTemplate const* qTemplate, WorldQuest* Wq);
    WorldQuestTypeReward GetWorldQuestTypeReward(WorldQuestTemplate const* qTemplate);
    WorldQuestTypeReward GetWorldQuestReward(WorldQuestTemplate const* qTemplate);
    void CheckGemForClass(std::set<uint8>& classListH, std::set<uint8>& classListA, ItemTemplate const* proto, WorldQuest* Wq);
    void CheckItemForClass(std::set<uint8>& classListH, std::set<uint8>& classListA, ItemTemplate const* proto, WorldQuest* Wq);
    void CheckItemForSpec(std::set<uint8>& classListH, std::set<uint8>& classListA, ItemTemplate const* proto, WorldQuest* Wq);
    bool CheckItemForHorde(ItemTemplate const* proto);
    bool CheckItemForAlliance(ItemTemplate const* proto);
    void SaveWorldQuest();
    void ResetWorldQuest();
    bool CanBeActivate(WorldQuestTemplate const* qTemplate, WorldQuestUpdate const* questUpdate);
    WorldQuestRecipe const* GetRecipesForQuest(uint32 QuestID);
    uint32 GetWorldQuestFactionAnalog(uint32 questId, bool findAllianceQuest) const;
    WorldQuest const* GenerateWorldQuest(WorldQuestUpdate const* worldQuestU, WorldQuestTemplate const* wqTemplate);

    QuestAreaTaskMap _worldQuestAreaTaskStore;
    WorldQuestTemplateMap _worldQuestTemplate;
    WorldQuestUpdateMap _worldQuestUpdate[QUEST_INFO_MAX];
    WorldQuestUpdateSet _worldQuestSet[QUEST_INFO_MAX];
    WorldQuestItemList _worldQuestRelic;
    WorldQuestItemList _worldQuestItem;
    WorldQuestItemList _worldQuestItemDungeon;
    WorldQuestItemList _worldQuestCAItem;
    WorldQuestMap _worldQuest;
    WorldQuestRecipeMap _worldQuestRecipes;
    QuestTemplateLocaleContainer _questTemplateLocaleStore;
    QuestRequestItemsLocaleContainer _questRequestItemsLocaleStore;
    QuestOfferRewardLocaleContainer _questOfferRewardLocaleStore;
    QuestObjectivesLocaleContainer _questObjectivesLocaleStore;
    QuestAreaTaskMap _questAreaTaskStore;
    QuestMap _questTemplates;
    QuestVector _questVTemplates;
    uint32 _maxQuestId;
    GameObjectForQuestContainer _gameObjectForQuestStore;
    PointOfInterestContainer _pointsOfInterestStore;
    QuestPOIContainer _questPOIStore;
    QuestRelations _goQuestRelations;
    QuestRelations _goQuestInvolvedRelations;
    QuestRelations _goQuestInvolvedRelationsByQuest;
    QuestRelations _creatureQuestRelations;
    QuestRelations _creatureQuestInvolvedRelations;
    QuestRelations _creatureQuestInvolvedRelationsByQuest;
    QuestRelations _areaQuestRelations;
    QuestStarter _goQuestStarter;
    QuestStarter _creatureQuestStarter;
    QuestStarter _areaQuestStarter;
    PointOfInterestLocaleContainer _pointOfInterestLocaleStore;
    std::map<QuestObjectiveType, std::vector<QuestObjective>> _questObjectiveByType;
    std::map<uint32, uint32> _worldQuestsFactionAnalogs[2];
};

#define sQuestDataStore QuestDataStoreMgr::instance()

#endif
