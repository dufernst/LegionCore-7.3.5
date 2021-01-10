/*
 * Copyright (C) 2011 TrintiyCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_DB2STORES_H
#define TRINITY_DB2STORES_H

#include "DB2Store.h"
#include "DB2Structure.h"
#include "SharedDefines.h"
#include "Hash.h"

#include <boost/regex_fwd.hpp>

extern DB2Storage<AchievementEntry>                         sAchievementStore;
extern DB2Storage<AdventureMapPOIEntry>                     sAdventureMapPOIStore;
extern DB2Storage<AdventureJournalEntry>                    sAdventureJournalStore;
extern DB2Storage<AnimKitEntry>                             sAnimKitStore;
extern DB2Storage<AreaTableEntry>                           sAreaTableStore;
extern DB2Storage<AreaTriggerEntry>                         sAreaTriggerStore;
extern DB2Storage<ArmorLocationEntry>                       sArmorLocationStore;
extern DB2Storage<ArtifactAppearanceEntry>                  sArtifactAppearanceStore;
extern DB2Storage<ArtifactAppearanceSetEntry>               sArtifactAppearanceSetStore;
extern DB2Storage<ArtifactCategoryEntry>                    sArtifactCategoryStore;
extern DB2Storage<ArtifactEntry>                            sArtifactStore;
extern DB2Storage<ArtifactPowerEntry>                       sArtifactPowerStore;
extern DB2Storage<ArtifactPowerLinkEntry>                   sArtifactPowerLinkStore;
extern DB2Storage<ArtifactPowerPickerEntry>                 sArtifactPowerPickerStore;
extern DB2Storage<ArtifactPowerRankEntry>                   sArtifactPowerRankStore;
extern DB2Storage<ArtifactQuestXPEntry>                     sArtifactQuestXPStore;
extern DB2Storage<ArtifactUnlockEntry>                      sArtifactUnlockStore;
extern DB2Storage<AuctionHouseEntry>                        sAuctionHouseStore;
extern DB2Storage<BankBagSlotPricesEntry>                   sBankBagSlotPricesStore;
extern DB2Storage<BannedAddonsEntry>                        sBannedAddOnsStore;
extern DB2Storage<BarberShopStyleEntry>                     sBarberShopStyleStore;
extern DB2Storage<BattlemasterListEntry>                    sBattlemasterListStore;
extern DB2Storage<BattlePetAbilityEffectEntry>              sBattlePetAbilityEffectStore;
extern DB2Storage<BattlePetAbilityEntry>                    sBattlePetAbilityStore;
extern DB2Storage<BattlePetAbilityStateEntry>               sBattlePetAbilityStateStore;
extern DB2Storage<BattlePetAbilityTurnEntry>                sBattlePetAbilityTurnStore;
extern DB2Storage<BattlePetBreedQualityEntry>               sBattlePetBreedQualityStore;
extern DB2Storage<BattlePetBreedStateEntry>                 sBattlePetBreedStateStore;
extern DB2Storage<BattlePetEffectPropertiesEntry>           sBattlePetEffectPropertiesStore;
extern DB2Storage<BattlePetSpeciesEntry>                    sBattlePetSpeciesStore;
extern DB2Storage<BattlePetSpeciesStateEntry>               sBattlePetSpeciesStateStore;
extern DB2Storage<BattlePetSpeciesXAbilityEntry>            sBattlePetSpeciesXAbilityStore;
extern DB2Storage<BattlePetStateEntry>                      sBattlePetStateStore;
extern DB2Storage<BroadcastTextEntry>                       sBroadcastTextStore;
extern DB2Storage<CharShipmentContainerEntry>               sCharShipmentContainerStore;
extern DB2Storage<CharShipmentEntry>                        sCharShipmentStore;
extern DB2Storage<CharStartOutfitEntry>                     sCharStartOutfitStore;
extern DB2Storage<CharTitlesEntry>                          sCharTitlesStore;
extern DB2Storage<ChatChannelsEntry>                        sChatChannelsStore;
extern DB2Storage<ChrClassesEntry>                          sChrClassesStore;
extern DB2Storage<ChrClassesXPowerTypesEntry>               sChrClassesXPowerTypesStore;
extern DB2Storage<ChrRacesEntry>                            sChrRacesStore;
extern DB2Storage<ChrSpecializationEntry>                   sChrSpecializationStore;
extern DB2Storage<ConversationLineEntry>                    sConversationLineStore;
extern DB2Storage<CreatureDisplayInfoEntry>                 sCreatureDisplayInfoStore;
extern DB2Storage<CreatureDisplayInfoExtraEntry>            sCreatureDisplayInfoExtraStore;
extern DB2Storage<CreatureFamilyEntry>                      sCreatureFamilyStore;
extern DB2Storage<CreatureModelDataEntry>                   sCreatureModelDataStore;
extern DB2Storage<CreatureTypeEntry>                        sCreatureTypeStore;
extern DB2Storage<CriteriaEntry>                            sCriteriaStore;
extern DB2Storage<CriteriaTreeEntry>                        sCriteriaTreeStore;
extern DB2Storage<CurrencyTypesEntry>                       sCurrencyTypesStore;
extern DB2Storage<CurveEntry>                               sCurveStore;
extern DB2Storage<CurvePointEntry>                          sCurvePointStore;
extern DB2Storage<DestructibleModelDataEntry>               sDestructibleModelDataStore;
extern DB2Storage<DifficultyEntry>                          sDifficultyStore;
extern DB2Storage<DungeonEncounterEntry>                    sDungeonEncounterStore;
extern DB2Storage<DurabilityCostsEntry>                     sDurabilityCostsStore;
extern DB2Storage<DurabilityQualityEntry>                   sDurabilityQualityStore;
extern DB2Storage<EmotesEntry>                              sEmotesStore;
extern DB2Storage<EmotesTextEntry>                          sEmotesTextStore;
extern DB2Storage<FactionEntry>                             sFactionStore;
extern DB2Storage<FactionTemplateEntry>                     sFactionTemplateStore;
extern DB2Storage<GameObjectDisplayInfoEntry>               sGameObjectDisplayInfoStore;
extern DB2Storage<GameObjectsEntry>                         sGameObjectsStore;
extern DB2Storage<GarrAbilityEffectEntry>                   sGarrAbilityEffectStore;
extern DB2Storage<GarrAbilityEntry>                         sGarrAbilityStore;
extern DB2Storage<GarrBuildingEntry>                        sGarrBuildingStore;
extern DB2Storage<GarrBuildingPlotInstEntry>                sGarrBuildingPlotInstStore;
extern DB2Storage<GarrClassSpecEntry>                       sGarrClassSpecStore;
extern DB2Storage<GarrEncounterEntry>                       sGarrEncounterStore;
extern DB2Storage<GarrEncounterSetXEncounterEntry>          sGarrEncounterSetXEncounterStore;
extern DB2Storage<GarrEncounterXMechanicEntry>              sGarrEncounterXMechanicStore;
extern DB2Storage<GarrFollowerEntry>                        sGarrFollowerStore;
extern DB2Storage<GarrFollowerTypeEntry>                    sGarrFollowerTypeStore;
extern DB2Storage<GarrFollowerLevelXPEntry>                 sGarrFollowerLevelXPStore;
extern DB2Storage<GarrFollowerQualityEntry>                 sGarrFollowerQualityStore;
extern DB2Storage<GarrFollowerXAbilityEntry>                sGarrFollowerXAbilityStore;
extern DB2Storage<GarrItemLevelUpgradeDataEntry>            sGarrItemLevelUpgradeDataStore;
extern DB2Storage<GarrMechanicEntry>                        sGarrMechanicStore;
extern DB2Storage<GarrMechanicSetXMechanicEntry>            sGarrMechanicSetXMechanicStore;
extern DB2Storage<GarrMechanicTypeEntry>                    sGarrMechanicTypeStore;
extern DB2Storage<GarrMissionEntry>                         sGarrMissionStore;
extern DB2Storage<GarrMissionXEncounterEntry>               sGarrMissionXEncounterStore;
extern DB2Storage<GarrPlotBuildingEntry>                    sGarrPlotBuildingStore;
extern DB2Storage<GarrPlotEntry>                            sGarrPlotStore;
extern DB2Storage<GarrPlotInstanceEntry>                    sGarrPlotInstanceStore;
extern DB2Storage<GarrSiteLevelEntry>                       sGarrSiteLevelStore;
extern DB2Storage<GarrSiteLevelPlotInstEntry>               sGarrSiteLevelPlotInstStore;
extern DB2Storage<GarrTalentEntry>                          sGarrTalentStore;
extern DB2Storage<GarrTalentTreeEntry>                      sGarrTalentTreeStore;
extern DB2Storage<GemPropertiesEntry>                       sGemPropertiesStore;
extern DB2Storage<GlyphPropertiesEntry>                     sGlyphPropertiesStore;
extern DB2Storage<GroupFinderActivityEntry>                 sGroupFinderActivityStore;
extern DB2Storage<GroupFinderActivityGrpEntry>              sGroupFinderActivityGrpStore;
extern DB2Storage<GroupFinderCategoryEntry>                 sGroupFinderCategoryStore;
extern DB2Storage<GuildPerkSpellsEntry>                     sGuildPerkSpellsStore;
extern DB2Storage<HeirloomEntry>                            sHeirloomStore;
extern DB2Storage<HolidayNamesEntry>                        sHolidayNamesStore;
extern DB2Storage<HolidaysEntry>                            sHolidaysStore;
extern DB2Storage<ImportPriceArmorEntry>                    sImportPriceArmorStore;
extern DB2Storage<ImportPriceQualityEntry>                  sImportPriceQualityStore;
extern DB2Storage<ImportPriceShieldEntry>                   sImportPriceShieldStore;
extern DB2Storage<ImportPriceWeaponEntry>                   sImportPriceWeaponStore;
extern DB2Storage<ItemArmorQualityEntry>                    sItemArmorQualityStore;
extern DB2Storage<ItemArmorShieldEntry>                     sItemArmorShieldStore;
extern DB2Storage<ItemArmorTotalEntry>                      sItemArmorTotalStore;
extern DB2Storage<ItemChildEquipmentEntry>                  sItemChildEquipmentStore;
extern DB2Storage<ItemCurrencyCostEntry>                    sItemCurrencyCostStore;
extern DB2Storage<ItemDamageAmmoEntry>                      sItemDamageAmmoStore;
extern DB2Storage<ItemDamageOneHandCasterEntry>             sItemDamageOneHandCasterStore;
extern DB2Storage<ItemDamageOneHandEntry>                   sItemDamageOneHandStore;
extern DB2Storage<ItemDamageTwoHandCasterEntry>             sItemDamageTwoHandCasterStore;
extern DB2Storage<ItemDamageTwoHandEntry>                   sItemDamageTwoHandStore;
extern DB2Storage<ItemDisenchantLootEntry>                  sItemDisenchantLootStore;
extern DB2Storage<ItemEffectEntry>                          sItemEffectStore;
extern DB2Storage<ItemEntry>                                sItemStore;
extern DB2Storage<ItemExtendedCostEntry>                    sItemExtendedCostStore;
extern DB2Storage<ItemLimitCategoryEntry>                   sItemLimitCategoryStore;
extern DB2Storage<ItemModifiedAppearanceEntry>              sItemModifiedAppearanceStore;
extern DB2Storage<ItemPriceBaseEntry>                       sItemPriceBaseStore;
extern DB2Storage<ItemRandomPropertiesEntry>                sItemRandomPropertiesStore;
extern DB2Storage<ItemRandomSuffixEntry>                    sItemRandomSuffixStore;
extern DB2Storage<ItemSetEntry>                             sItemSetStore;
extern DB2Storage<ItemSparseEntry>                          sItemSparseStore;
extern DB2Storage<ItemSpecEntry>                            sItemSpecStore;
extern DB2Storage<ItemSpecOverrideEntry>                    sItemSpecOverrideStore;
extern DB2Storage<ItemUpgradeEntry>                         sItemUpgradeStore;
extern DB2Storage<JournalEncounterCreatureEntry>            sJournalEncounterCreatureStore;
extern DB2Storage<JournalEncounterEntry>                    sJournalEncounterStore;
extern DB2Storage<JournalEncounterItemEntry>                sJournalEncounterItemStore;
extern DB2Storage<JournalInstanceEntry>                     sJournalInstanceStore;
extern DB2Storage<KeychainEntry>                            sKeyChainStore;
extern DB2Storage<LFGDungeonsEntry>                         sLfgDungeonsStore;
extern DB2Storage<LiquidTypeEntry>                          sLiquidTypeStore;
extern DB2Storage<LockEntry>                                sLockStore;
extern DB2Storage<MailTemplateEntry>                        sMailTemplateStore;
extern DB2Storage<ManagedWorldStateBuffEntry>               sManagedWorldStateBuffStore;
extern DB2Storage<ManagedWorldStateEntry>                   sManagedWorldStateStore;
extern DB2Storage<ManagedWorldStateInputEntry>              sManagedWorldStateInputStore;
extern DB2Storage<MapChallengeModeEntry>                    sMapChallengeModeStore;
extern DB2Storage<MapEntry>                                 sMapStore;
extern DB2Storage<ModifierTreeEntry>                        sModifierTreeStore;
extern DB2Storage<MountCapabilityEntry>                     sMountCapabilityStore;
extern DB2Storage<MountEntry>                               sMountStore;
extern DB2Storage<MovieEntry>                               sMovieStore;
extern DB2Storage<OverrideSpellDataEntry>                   sOverrideSpellDataStore;
extern DB2Storage<ParagonReputationEntry>                   sParagonReputationStore;
extern DB2Storage<PhaseEntry>                               sPhaseStore;
extern DB2Storage<PlayerConditionEntry>                     sPlayerConditionStore;
extern DB2Storage<PowerDisplayEntry>                        sPowerDisplayStore;
extern DB2Storage<PowerTypeEntry>                           sPowerTypeStore;
extern DB2Storage<PVPItemEntry>                             sPvpItemStore;
extern DB2Storage<PvpRewardEntry>                           sPvpRewardStore;
extern DB2Storage<PvpScalingEffectEntry>                    sPvpScalingEffectStore;
extern DB2Storage<PvpScalingEffectTypeEntry>                sPvpScalingEffectTypeStore;
extern DB2Storage<PvpTalentEntry>                           sPvpTalentStore;
extern DB2Storage<QuestFactionRewardEntry>                  sQuestFactionRewardStore;
extern DB2Storage<QuestLineEntry>                           sQuestLineStore;
extern DB2Storage<QuestLineXQuestEntry>                     sQuestLineXQuestStore;
extern DB2Storage<QuestMoneyRewardEntry>                    sQuestMoneyRewardStore;
extern DB2Storage<QuestObjectiveEntry>                      sQuestObjectiveStore;
extern DB2Storage<QuestSortEntry>                           sQuestSortStore;
extern DB2Storage<QuestV2CliTaskEntry>                      sQuestV2CliTaskStore;
extern DB2Storage<QuestV2Entry>                             sQuestV2Store;
extern DB2Storage<QuestXPEntry>                             sQuestXPStore;
extern DB2Storage<RandPropPointsEntry>                      sRandPropPointsStore;
extern DB2Storage<RelicTalentEntry>                         sRelicTalentStore;
extern DB2Storage<ResearchBranchEntry>                      sResearchBranchStore;
extern DB2Storage<ResearchProjectEntry>                     sResearchProjectStore;
extern DB2Storage<RewardPackEntry>                          sRewardPackStore;
extern DB2Storage<RewardPackXCurrencyTypeEntry>             sRewardPackXCurrencyTypeStore;
extern DB2Storage<RewardPackXItemEntry>                     sRewardPackXItemStore;
extern DB2Storage<RulesetItemUpgradeEntry>                  sRulesetItemUpgradeStore;
extern DB2Storage<SandboxScalingEntry>                      sSandboxScalingStore;
extern DB2Storage<ScalingStatDistributionEntry>             sScalingStatDistributionStore;
extern DB2Storage<ScenarioEntry>                            sScenarioStore;
extern DB2Storage<ScenarioStepEntry>                        sScenarioStepStore;
extern DB2Storage<SkillLineAbilityEntry>                    sSkillLineAbilityStore;
extern DB2Storage<SkillLineEntry>                           sSkillLineStore;
extern DB2Storage<SkillRaceClassInfoEntry>                  sSkillRaceClassInfoStore;
extern DB2Storage<SpecializationSpellsEntry>                sSpecializationSpellsStore;
extern DB2Storage<SpellAuraOptionsEntry>                    sSpellAuraOptionsStore;
extern DB2Storage<SpellAuraRestrictionsEntry>               sSpellAuraRestrictionsStore;
extern DB2Storage<SpellCastingRequirementsEntry>            sSpellCastingRequirementsStore;
extern DB2Storage<SpellCastTimesEntry>                      sSpellCastTimesStore;
extern DB2Storage<SpellCategoriesEntry>                     sSpellCategoriesStore;
extern DB2Storage<SpellCategoryEntry>                       sSpellCategoryStore;
extern DB2Storage<SpellClassOptionsEntry>                   sSpellClassOptionsStore;
extern DB2Storage<SpellCooldownsEntry>                      sSpellCooldownsStore;
extern DB2Storage<SpellDurationEntry>                       sSpellDurationStore;
extern DB2Storage<SpellEntry>                               sSpellStore;
extern DB2Storage<SpellEquippedItemsEntry>                  sSpellEquippedItemsStore;
extern DB2Storage<SpellFocusObjectEntry>                    sSpellFocusObjectStore;
extern DB2Storage<SpellInterruptsEntry>                     sSpellInterruptsStore;
extern DB2Storage<SpellItemEnchantmentConditionEntry>       sSpellItemEnchantmentConditionStore;
extern DB2Storage<SpellItemEnchantmentEntry>                sSpellItemEnchantmentStore;
extern DB2Storage<SpellLearnSpellEntry>                     sSpellLearnSpellStore;
extern DB2Storage<SpellLevelsEntry>                         sSpellLevelsStore;
extern DB2Storage<SpellMiscEntry>                           sSpellMiscStore;
extern DB2Storage<SpellPowerEntry>                          sSpellPowerStore;
extern DB2Storage<SpellProcsPerMinuteEntry>                 sSpellProcsPerMinuteStore;
extern DB2Storage<SpellProcsPerMinuteModEntry>              sSpellProcsPerMinuteModStore;
extern DB2Storage<SpellRadiusEntry>                         sSpellRadiusStore;
extern DB2Storage<SpellRangeEntry>                          sSpellRangeStore;
extern DB2Storage<SpellReagentsCurrencyEntry>               sSpellReagentsCurrencyStore;
extern DB2Storage<SpellReagentsEntry>                       sSpellReagentsStore;
extern DB2Storage<SpellScalingEntry>                        sSpellScalingStore;
extern DB2Storage<SpellShapeshiftEntry>                     sSpellShapeshiftStore;
extern DB2Storage<SpellShapeshiftFormEntry>                 sSpellShapeshiftFormStore;
extern DB2Storage<SpellTargetRestrictionsEntry>             sSpellTargetRestrictionsStore;
extern DB2Storage<SpellTotemsEntry>                         sSpellTotemsStore;
extern DB2Storage<SpellVisualEntry>                         sSpellVisualStore;
extern DB2Storage<SpellXSpellVisualEntry>                   sSpellXSpellVisualStore;
extern DB2Storage<SummonPropertiesEntry>                    sSummonPropertiesStore;
extern DB2Storage<TalentEntry>                              sTalentStore;
extern DB2Storage<TaxiNodesEntry>                           sTaxiNodesStore;
extern DB2Storage<TaxiPathEntry>                            sTaxiPathStore;
extern DB2Storage<TransmogHolidayEntry>                     sTransmogHolidayStore;
extern DB2Storage<TransmogSetEntry>                         sTransmogSetStore;
extern DB2Storage<TransmogSetGroupEntry>                    sTransmogSetGroupStore;
extern DB2Storage<TransmogSetItemEntry>                     sTransmogSetItemStore;
extern DB2Storage<TransportAnimationEntry>                  sTransportAnimationStore;
extern DB2Storage<TransportRotationEntry>                   sTransportRotationStore;
extern DB2Storage<UnitPowerBarEntry>                        sUnitPowerBarStore;
extern DB2Storage<VehicleEntry>                             sVehicleStore;
extern DB2Storage<VehicleSeatEntry>                         sVehicleSeatStore;
extern DB2Storage<VignetteEntry>                            sVignetteStore;
extern DB2Storage<WorldEffectEntry>                         sWorldEffectStore;
extern DB2Storage<WorldMapAreaEntry>                        sWorldMapAreaStore;
extern DB2Storage<WorldMapOverlayEntry>                     sWorldMapOverlayStore;
extern DB2Storage<WorldSafeLocsEntry>                       sWorldSafeLocsStore;
extern DB2Storage<WorldStateExpressionEntry>                sWorldStateExpressionStore;
extern DB2Storage<WorldStateUIEntry>                        sWorldStateUIStore;
extern DB2Storage<FriendshipRepReactionEntry>               sFriendshipRepReactionStore;
extern DB2Storage<FriendshipReputationEntry>                sFriendshipReputationStore;
extern DB2Storage<GameObjectArtKitEntry>                    sGameObjectArtKitStore;
extern DB2Storage<GuildColorBackgroundEntry>                sGuildColorBackgroundStore;
extern DB2Storage<GuildColorBorderEntry>                    sGuildColorBorderStore;
extern DB2Storage<GuildColorEmblemEntry>                    sGuildColorEmblemStore;

extern TaxiMask                                             sTaxiNodesMask;
extern std::vector<uint8>                                   sTaxiNodesMaskV;
extern TaxiMask                                             sOldContinentsNodesMask;
extern TaxiMask                                             sHordeTaxiNodesMask;
extern TaxiMask                                             sAllianceTaxiNodesMask;
extern TaxiPathSetBySource                                  sTaxiPathSetBySource;
extern TaxiPathNodesByPath                                  sTaxiPathNodesByPath;

struct ResearchPOIPoint
{
    ResearchPOIPoint();
    ResearchPOIPoint(int32 _x, int32 _y);

    int32 x;
    int32 y;
};

struct DigSitePosition
{
    DigSitePosition();
    DigSitePosition(float _x, float _y);

    float x;
    float y;
};

typedef std::vector<ResearchPOIPoint> ResearchPOIPointVector;
typedef std::vector<DigSitePosition> DigSitePositionVector;

struct ResearchSiteData
{
    ResearchSiteData();

    ResearchSiteEntry const* entry;
    uint32 find_id; // is a GO entry
    uint16 zone;
    uint16 branch_id;
    uint8 level;
    ResearchPOIPointVector points;
    DigSitePositionVector digSites;
};

struct SpellEffect
{
    SpellEffect();
    SpellEffectEntry const* effects[MAX_SPELL_EFFECTS];
};

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g);

    bool operator <(const WMOAreaTableTripple& b) const;

    // ordered by entropy; that way memcmp will have a minimal medium runtime
    int32 groupId;
    int32 rootId;
    int32 adtId;
};

static uint32 legionPvpItem[8][2]
{
    // Season 0
    { 0, 0 },
    // Season 1
    { 13226, 13227 },
    // Season 2
    { 13287, 13288 },
    // Season 3
    { 13295, 13296 },
    // Season 4
    { 13297, 13298 },
    // Season 5
    { 13299, 13300 },
    // Season 6 not need
    { 13311, 13313 },
    // Season 7 not need
    { 13312, 13314 },
};

template<typename T>
class DB2HotfixGenerator;

#define DEFINE_DB2_SET_COMPARATOR(structure) \
    struct structure ## Comparator \
    { \
        bool operator()(structure const* left, structure const* right) const { return Compare(left, right); } \
        static bool Compare(structure const* left, structure const* right); \
    };

#define PET_SPEC_OVERRIDE_CLASS_INDEX MAX_CLASSES

class DB2Manager
{
    template<typename T>
    friend class DB2HotfixGenerator;

    uint32 _maxHotfixId = 0;

public:
    void InsertNewHotfix(uint32 tableHash, uint32 recordId);

    DEFINE_DB2_SET_COMPARATOR(MountTypeXCapabilityEntry)

    typedef std::vector<ItemBonusEntry const*> ItemBonusList;
    typedef std::unordered_map<uint32, std::unordered_map<uint32, MapDifficultyEntry const*>> MapDifficultyContainer;
    typedef std::set<MountTypeXCapabilityEntry const*, MountTypeXCapabilityEntryComparator> MountTypeXCapabilitySet;
    typedef std::vector<MountXDisplayEntry const*> MountXDisplayContainer;
    typedef std::map<uint32 /*word length*/, StringVector> LanguageWordsContainer;
    typedef std::set<uint32> PetFamilySpellsSet;
    typedef std::unordered_map<uint32, PetFamilySpellsSet > PetFamilySpellsContainer;
    typedef std::vector<uint32> SimpleFactionsList;
    typedef std::map<uint32, SimpleFactionsList> FactionTeamContainer;
    typedef std::set<GarrAbilityEffectEntry const* /*effect*/> GarrEffectContainer;
    typedef std::unordered_map<uint32 /*AbilityID*/, GarrEffectContainer> GarrAbilityEffectContainer;
    typedef std::multimap<uint32 /*ContainerID*/, CharShipmentEntry const*> ShipmentConteinerMap;
    typedef std::pair<ShipmentConteinerMap::const_iterator, ShipmentConteinerMap::const_iterator> ShipmentConteinerMapBounds;
    typedef std::map<uint16/*optionIdx*/, GarrTalentEntry const*> GarrTalentOptionMap;
    typedef std::map<uint16/*lineIdx*/, GarrTalentOptionMap> GarrTalentLineMap;
    typedef std::map<uint32/*class*/, GarrTalentLineMap> GarrTalentClassMap;
    typedef std::vector<ItemSetSpellEntry const*> ItemSetSpells;
    typedef std::unordered_map<uint32, ItemSetSpells> ItemSetSpellsContainer;
    typedef std::set<ResearchProjectEntry const*> ResearchProjectContainer;
    typedef std::map<uint32 /*site_id*/, ResearchSiteData> ResearchSiteDataMap;
    typedef std::unordered_map<uint32 /*frame*/, TransportAnimationEntry const*> TransportAnimationEntryMap;
    typedef std::unordered_map<uint32, TransportAnimationEntryMap> TransportAnimationsByEntryContainer;
    typedef std::set<uint32> SpellCategorySet;
    typedef std::unordered_map<uint32, SpellCategorySet> SpellCategoryContainer;
    typedef std::vector<TalentEntry const*> TalentsByPositionContainer[MAX_CLASSES][MAX_TALENT_TIERS][MAX_TALENT_COLUMNS];
    typedef std::vector<GarrMissionEntry const*> GarrMissionList;
    typedef std::map<uint8/*garrType*/, GarrMissionList> GarrMissionsMap;
    typedef std::unordered_map<uint32 /*ArtifactID*/, ArtifactUnlockEntry const*> ArtifactToUnlockContainer;
    typedef std::set<uint32> XData;
    typedef std::unordered_map<uint32, XData> XContainer;
    typedef std::unordered_map<uint32 /*FactionID*/, ParagonReputationEntry const*> ParagonReputationContainer;
    typedef std::vector<std::vector<SkillLineAbilityEntry const*>> SkillLineAbilityContainer;

    static DB2Manager& Instance();

    void LoadStores(std::string const& dataPath, uint32 defaultLocale);
    void InitDB2CustomStores();
    static DB2StorageBase const* GetStorage(uint32 type);
    void LoadingExtraHotfixData();

    void LoadHotfixData();
    static std::map<uint64, int32> const& GetHotfixData();

    std::vector<uint32> GetAreasForGroup(uint32 areaGroupId);
    std::vector<uint32> GetGroupsForArea(uint32 areaId);
    static bool IsInArea(uint32 objectAreaId, uint32 areaId);
    std::list<uint32> GetGameObjectsList();
    uint32 GetRulesetItemUpgrade(uint32 itemId) const;
    float GetCurveValueAt(uint32 curveId, float x) const;
    uint32 GetItemDisplayId(uint32 itemId, uint32 appearanceModId) const;
    uint32 GetItemDIconFileDataId(uint32 itemId, uint32 appearanceModId = 0) const;
    std::vector<ItemLimitCategoryConditionEntry const*> const* GetItemLimitCategoryConditions(uint32 categoryId) const;
    ItemModifiedAppearanceEntry const* GetItemModifiedAppearance(uint32 itemId, uint32 appearanceModId) const;
    ItemModifiedAppearanceEntry const* GetDefaultItemModifiedAppearance(uint32 itemId) const;
    uint32 GetTransmogId(uint32 itemId, uint8 context = 0) const;
    std::vector<uint32> GetAllTransmogsByItemId(uint32 itemId) const;
    ItemBonusList const* GetItemBonusList(uint32 bonusListId) const;
    uint32 GetItemBonusListForItemLevelDelta(int16 delta) const;
    LanguageWordsContainer const* GetLanguageWordMap(uint32 landID);
    StringVector const* GetLanguageWordsBySize(uint32 landID, uint32 size);
    std::vector<QuestPackageItemEntry const*> const* GetQuestPackageItems(uint32 questPackageID) const;
    MountEntry const* GetMount(uint32 spellId) const;
    static MountEntry const* GetMountById(uint32 id);
    MountTypeXCapabilitySet const* GetMountCapabilities(uint32 mountType) const;
    std::vector<uint32> GetItemBonusTree(uint32 itemId, uint32 itemBonusTreeMod, uint32& itemLevel) const;
    std::set<uint32> const* GetItemsByBonusTree(uint32 itemBonusTreeMod) const;
    std::set<ItemBonusTreeNodeEntry const*> const* GetItemBonusSet(uint32 itemBonusTree) const;
    HeirloomEntry const* GetHeirloomByItemId(uint32 itemId) const;
    bool IsToyItem(uint32 toy) const;
    uint32 GetXPForNextFollowerLevel(uint32 level, uint8 followerTypeID);
    uint32 GetXPForNextFollowerQuality(uint32 quality, uint8 followerTypeID);
    uint8 GetNextFollowerQuality(uint32 quality, uint8 followerTypeID);
    static char const* GetBroadcastTextValue(BroadcastTextEntry const* broadcastText, LocaleConstant locale = DEFAULT_LOCALE, uint8 gender = GENDER_MALE, bool forceGender = false);
    AchievementEntry const* GetsAchievementByTreeList(uint32 criteriaTree);
    std::array<std::vector<uint32>, 2> GetItemLoadOutItemsByClassID(uint32 classID, uint8 type = 0);
    std::vector<uint32> GetLowestIdItemLoadOutItemsBy(uint32 classID, uint8 type);
    std::vector<CriteriaTreeEntry const*> const* GetCriteriaTreeList(uint32 parent);
    std::vector<ModifierTreeEntry const*> const* GetModifierTreeList(uint32 parent);
    std::string GetNameGenEntry(uint8 race, uint8 gender) const;
    ResponseCodes ValidateName(std::wstring const& name, LocaleConstant locale) const;
    static uint32 GetQuestUniqueBitFlag(uint32 questID);
    ResearchSiteEntry const* GetResearchSiteEntryById(uint32 id);
    void DeterminaAlternateMapPosition(uint32 mapId, float x, float y, float z, uint32* newMapId = nullptr, DBCPosition2D* newPos = nullptr);
    static bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId);
    SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_);
    std::vector<SpecializationSpellsEntry const*> const* GetSpecializationSpells(uint32 specId);
    std::vector<SpellProcsPerMinuteModEntry const*> GetSpellProcsPerMinuteMods(uint32 spellprocsPerMinuteID) const;
    SpellTargetRestrictionsEntry const* GetSpellTargetRestrioctions(uint32 spellid, uint16 difficulty);
    uint32 GetLearnSpell(uint32 trigerSpell);
    uint32 GetSpellByTrigger(uint32 trigerSpell);
    SpellEffectEntry const* GetSpellEffectEntry(uint32 spellId, uint32 effect, uint8 difficulty);
    std::set<uint32> const* GetSpellCategory(uint32 category);
    std::vector<ItemSpecOverrideEntry const*> const* GetItemSpecOverrides(uint32 itemId) const;
    static PVPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapID, uint32 level);
    static PVPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapID, uint8 id);
    ChrSpecializationEntry const* GetChrSpecializationByIndex(uint8 classID, uint32 ID);
    ChrSpecializationEntry const* GetDefaultChrSpecializationForClass(uint32 class_) const;
    PetFamilySpellsSet const* GetPetFamilySpells(uint32 family);
    uint32 GetPowerIndexByClass(uint32 powerType, uint32 classId) const;
    AreaTableEntry const* FindAreaEntry(uint32 area);
    uint32 GetParentZoneOrSelf(uint32 zone);
    static char const* GetPetName(uint32 petfamily, LocaleConstant localeConstant);
    MapDifficultyEntry const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty);
    MapDifficultyEntry const* GetDefaultMapDifficulty(uint32 mapID);
    DungeonEncounterEntry const* GetDungeonEncounterByDisplayID(uint32 displayID);
    MapDifficultyEntry const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty);
    MapDifficultyContainer GetAllMapsDifficultyes();
    uint32 GetPlayerConditionForMapDifficulty(uint32 difficultyID);
    uint32 GetSpellMisc(uint32 spellID);
    std::vector<QuestLineXQuestEntry const*> const* GetQuestsByQuestLine(uint32 lineID) const;
    bool HasCharacterFacialHairStyle(uint8 race, uint8 gender, uint8 variationId) const;
    bool HasCharSections(uint8 race, uint8 gender, CharBaseSectionVariation variation) const;
    CharSectionsEntry const* GetCharSectionEntry(uint8 race, uint8 gender, CharBaseSectionVariation variation, uint8 variationIndex, uint8 color) const;
    SimpleFactionsList const* GetFactionTeamList(uint32 faction);
    ParagonReputationEntry const* GetFactionParagon(uint32 factionID);
    ParagonReputationEntry const* GetQuestParagon(uint32 questID);
    WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid);
    static uint32 GetLiquidFlags(uint32 liquidType);
    static uint32 GetDefaultMapLight(uint32 mapID);
    uint32 GetRequiredHonorLevelForPvpTalent(PvpTalentEntry const* talentInfo) const;
    PvpTalentEntry const* GetPvpTalentBySpellID(uint32 spellID);
    std::vector<ArtifactAppearanceSetEntry const*> const* GetArtifactAppearance(uint32 ArtifactID) const;
    std::vector<ArtifactAppearanceEntry const*> const* GetArtifactAppearanceBySet(uint32 AppearanceSetID) const;
    std::unordered_set<uint32> const* GetArtifactPowerLinks(uint32 artifactPowerId) const;
    std::vector<ArtifactPowerEntry const*> GetArtifactPowers(uint8 artifactId) const;
    ArtifactPowerRankEntry const* GetArtifactPowerRank(uint32 artifactPowerId, uint8 rank) const;
    ItemChildEquipmentEntry const* GetItemChildEquipment(uint32 itemId) const;
    bool IsChildItem(uint32 itemId) const;
    ItemClassEntry const* GetItemClassByOldEnum(uint32 itemClass) const;
    bool HasItemCurrencyCost(uint32 itemId) const;
    std::vector<TransmogSetItemEntry const*> const* GetTransmogSetItems(uint32 transmogSetId) const;
    std::vector<TransmogSetEntry const*> const* GetTransmogSetsForItemModifiedAppearance(uint32 itemModifiedAppearanceId) const;
    PowerTypeEntry const* GetPowerType(uint8 PowerTypeEnum);
    static float GetCurrencyPrecision(uint32 currencyId);
    WorldMapAreaEntry const* GetWorldMapArea(uint16 ZoneID);
    std::vector<uint16> const* GetWorldMapZone(uint16 MapID);
    GarrEffectContainer const* GetGarrEffect(uint32 AbilityID) const;
    ArtifactUnlockEntry const* GetArtifactUnlock(uint32 ArtifactID) const;
    uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);
    void Zone2MapCoordinates(float &x, float &y, uint32 zone);
    void Map2ZoneCoordinates(float &x, float &y, uint32 zone);
    std::vector<uint32> const* GetGlyphBindableSpells(uint32 glyphPropertiesId) const;
    std::vector<uint32> const* GetGlyphRequiredSpecs(uint32 glyphPropertiesId) const;
    static bool HasBattlePetSpeciesFlag(uint16 species, uint16 flag);
    MapChallengeModeEntry const* GetChallengeModeByMapID(uint32 mapID);
    std::vector<uint32> GetChallngeMaps();
    std::vector<double> GetChallngesWeight();
    double GetChallngeWeight(uint32 mapID);
    std::vector<PvpTalentEntry const*> GetPvpTalentByPosition(uint8 playerClass, uint8 row, uint8 column);
    uint32 GetHonorLevelRewardPack(uint16 honorLevel, uint8 prestigeLevel);
    RewardPackXItemEntry const* GetRewardPackXItem(uint32 rewardPackID);
    RewardPackXCurrencyTypeEntry const* GetRewardPackXCurrency(uint32 rewardPackID);
    MountXDisplayContainer const* GetMountDisplays(uint32 mountId) const;
    float GetPvpScalingValueByEffectType(uint32 type, uint32 specID);
    std::vector<uint32> GetLootItemsForInstanceByMapID(uint32 mapID);
    ShipmentConteinerMapBounds GetShipmentConteinerBounds(uint32 conteinerID) const;
    GarrTalentLineMap const* GetGarrTalentLine(uint16 __class);
    GarrTalentOptionMap const* GetGarrTalentOptionMap(uint16 __class, uint16 __line);
    XData const* getXMechanic(uint32 X) const;
    XData const* getXEncounter(uint32 X) const;
    std::set<uint32> GetPhasesForGroup(uint32 group) const;
    SkillLineAbilityEntry const* GetSkillBySpell(uint32 SpellID) const;
    BattlePetSpeciesEntry const* GetSpeciesBySpell(uint32 SpellID) const;
    BattlePetSpeciesEntry const* GetSpeciesByCreatureID(uint32 CreatureID) const;
    LFGDungeonsEntry const* GetLFGDungeonsByMapDIff(int16 MapID, uint8 DifficultyID) const;
    uint32 LFGRoleRequirementCondition(uint32 lfgDungeonsId, uint8 roleType);
    uint32 GetScalingByLevel(uint8 MinLevel, uint16 MaxLevel) const;

    uint32 GetHostileSpellVisualId(uint32 spellVisualId);

    ItemSetSpellsContainer _itemSetSpells;
    ResearchProjectContainer _researchProjectContainer;
    ResearchSiteDataMap _researchSiteDataMap;
    TransportAnimationsByEntryContainer _transportAnimationsByEntry;
    SpellCategoryContainer _spellCategory;
    TalentsByPositionContainer _talentByPos;
    ShipmentConteinerMap _charShipmentConteiner;
    GarrMissionsMap _garrMissionsMap;
    SkillLineAbilityContainer _skillLineAbilityContainer;
};

#define sDB2Manager DB2Manager::Instance()

#endif