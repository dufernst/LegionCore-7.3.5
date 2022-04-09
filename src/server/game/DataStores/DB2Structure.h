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

#ifndef TRINITY_DB2STRUCTURE_H
#define TRINITY_DB2STRUCTURE_H

#include "Common.h"
#include "DBCEnums.h"
#include "Util.h"

class Player;

#define MAX_HOLIDAY_DURATIONS 10
#define MAX_HOLIDAY_DATES 16
#define MAX_HOLIDAY_FLAGS 10
#define MAX_ITEM_EXT_COST_ITEMS 5
#define MAX_ITEM_EXT_COST_CURRENCIES 5
#define MAX_ITEM_ENCHANTS 5
#define MAX_EFFECT_PROPERTIES 6
#define KEYCHAIN_SIZE 32
#define MAX_OVERRIDE_SPELL 10
#define GO_DBC_DATA_COUNT 8
#define MAX_SPELL_TOTEMS 2
#define MAX_SPELL_REAGENTS 8
#define MAX_OUTFIT_ITEMS 24
#define MAX_WORLD_MAP_OVERLAY_AREA_IDX 4
#define MaxAttributes 14
#define MAX_MASTERY_SPELLS 2
#define MAX_ITEM_SET_ITEMS 17
#define MAX_LOCK_CASE 8
#define MAX_SHAPESHIFT_SPELLS 8
#define MAX_FACTION_RELATIONS 4
#define MAX_ITEM_ENCHANTMENT_EFFECTS 3
#define MAX_VEHICLE_SEATS 8

// #define MAX_ITEM_PROTO_FLAGS 4
// #define MAX_ITEM_PROTO_SOCKETS 3
// #define MAX_ITEM_PROTO_STATS 10

#pragma pack(push, 1)

typedef std::pair<uint16, int16> WorldQuestState;

// FileOptions: None
struct AchievementEntry
{
    LocalizedString* Title;
    LocalizedString* Description;
    LocalizedString* Reward;
    int32       Flags;
    int16       InstanceID;
    int16       Supercedes;
    int16       Category;
    int16       UiOrder;
    int16       SharesCriteria;
    int8        Faction;
    int8        Points;
    int8        MinimumCriteria;
    uint32      ID;
    int32       IconFileID;
    uint32      CriteriaTree;
};

// FileOptions: None
struct Achievement_CategoryEntry
{
    LocalizedString* Name;
    uint16      Parent;
    uint8       UiOrder;
    int32       ID;
};

// FileOptions: Index, None
struct AdventureJournalEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    LocalizedString* ButtonText;
    LocalizedString* RewardDescription;
    LocalizedString* ContinueDescription;
    int32       TextureFileDataID;
    int32       ItemID;
    uint16      LfgDungeonID;
    uint16      QuestID;
    uint16      BattleMasterListID;
    uint16      BonusPlayerConditionID[2];
    uint16      CurrencyType;
    uint16      WorldMapAreaID;
    uint8       Type;
    uint8       Flags;
    uint8       ButtonActionType;
    uint8       PriorityMin;
    uint8       PriorityMax;
    uint8       BonusValue[2];
    uint8       CurrencyQuantity;
    int32       PlayerConditionID;
    int32       ItemQuantity;
};

// FileOptions: Index, None
struct AdventureMapPOIEntry
{
    int32       ID;
    LocalizedString* Title;
    LocalizedString* Description;
    float       WorldPosition[2];
    int32       RewardItemID;
    uint8       Type;
    int32       PlayerConditionID;
    int32       QuestID;
    int32       LfgDungeonID;
    int32       UiTextureAtlasMemberID;
    int32       UiTextureKitID;
    int32       WorldMapAreaID;
    int32       DungeonMapID;
    int32       AreaTableID;
};

// FileOptions: None
struct AlliedRaceEntry
{
    int32       BannerColor;
    int32       ID;
    int32       RaceID;
    int32       CrestTextureID;
    int32       ModelBackgroundTextureID;
    int32       MaleCreatureDisplayID;
    int32       FemaleCreatureDisplayID;
    int32       UiUnlockAchievementID;
};

// FileOptions: Index, None
struct AlliedRaceRacialAbilityEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    uint8       OrderIndex;
    int32       IconFileDataID;
};

// FileOptions: Index, None
struct AnimationDataEntry
{
    int32       ID;
    uint32      Flags;
    uint16      Fallback;
    uint16      BehaviorID;
    uint8       BehaviorTier;
};

// FileOptions: Index, None
struct AnimKitEntry
{
    int32       ID;
    int32       OneShotDuration;
    uint16      OneShotStopAnimKitID;
    uint16      LowDefAnimKitID;
};

// FileOptions: Index, None
struct AnimKitBoneSetEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       BoneDataID;
    uint8       ParentAnimKitBoneSetID;
    uint8       ExtraBoneCount;
    uint8       AltAnimKitBoneSetID;
};

// FileOptions: Index, None
struct AnimKitBoneSetAliasEntry
{
    int32       ID;
    uint8       BoneDataID;
    uint8       AnimKitBoneSetID;
};

// FileOptions: Index, None
struct AnimKitConfigEntry
{
    int32       ID;
    int32       ConfigFlags;
};

// FileOptions: Index, None
struct AnimKitConfigBoneSetEntry
{
    int32       ID;
    uint16      AnimKitPriorityID;
    uint8       AnimKitBoneSetID;
};

// FileOptions: Index, None
struct AnimKitPriorityEntry
{
    int32       ID;
    uint8       Priority;
};

// FileOptions: None
struct AnimKitReplacementEntry
{
    uint16      SrcAnimKitID;
    uint16      DstAnimKitID;
    uint16      Flags;
    int32       ID;
};

// FileOptions: Index, None
struct AnimKitSegmentEntry
{
    int32       ID;
    int32       AnimStartTime;
    int32       EndConditionParam;
    int32       EndConditionDelay;
    float       Speed;
    uint32      OverrideConfigFlags;
    uint16      ParentAnimKitID;
    uint16      AnimID;
    uint16      AnimKitConfigID;
    uint16      SegmentFlags;
    uint16      BlendInTimeMs;
    uint16      BlendOutTimeMs;
    uint8       OrderIndex;
    uint8       StartCondition;
    uint8       StartConditionParam;
    uint8       EndCondition;
    uint8       ForcedVariation;
    uint8       LoopToSegmentIndex;
    int32       StartConditionDelay;
};

// FileOptions: None
struct AnimReplacementEntry
{
    uint16      SrcAnimID;
    uint16      DstAnimID;
    uint16      Flags;
    int32       ID;
};

// FileOptions: Index, None
struct AnimReplacementSetEntry
{
    int32       ID;
    uint8       ExecOrder;
};

// FileOptions: None
struct AreaFarClipOverrideEntry
{
    int32       AreaID;
    float       MinFarClip;
    float       MinHorizonStart;
    int32       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct AreaGroupMemberEntry
{
    int32       ID;
    uint16      AreaID;
    uint16      AreaGroupID;
};

// FileOptions: Index, None
struct AreaPOIEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    int32       Flags;
    float       Pos[3];
    int32       PoiDataType;
    int32       PoiData;
    uint16      ContinentID;
    uint16      AreaID;
    uint16      WorldStateID;
    uint8       Importance;
    uint8       Icon;
    int32       PlayerConditionID;
    int32       PortLocID;
    int32       UiTextureAtlasMemberID;
    int32       MapFloor;
    int32       WmoGroupID;
};

// FileOptions: Index, None
struct AreaPOIStateEntry
{
    int32       ID;
    LocalizedString* Description;
    uint8       WorldStateValue;
    uint8       IconEnumValue;
    int32       UiTextureAtlasMemberID;
};

// FileOptions: Index, None
struct AreaTableEntry
{
    int32       ID;
    LocalizedString* ZoneName;
    LocalizedString* AreaName;
    uint32      Flags[2];
    float       AmbientMultiplier;
    uint16      ContinentID;
    uint16      ParentAreaID;
    int16       AreaBit;
    uint16      AmbienceID;
    uint16      ZoneMusic;
    uint16      IntroSound;
    uint16      LiquidTypeID[4];
    uint16      UwZoneMusic;
    uint16      UwAmbience;
    uint16      PvpCombatWorldStateID;
    uint8       SoundProviderPref;
    uint8       SoundProviderPrefUnderwater;
    int8        ExplorationLevel;
    uint8       FactionGroupMask;
    uint8       MountFlags;
    uint8       WildBattlePetLevelMin;
    uint8       WildBattlePetLevelMax;
    uint8       WindSettingsID;
    int32       UwIntroSound;

    bool IsSanctuary() const;
    bool ActivatesPvpTalents() const;
};

// FileOptions: None
struct AreaTriggerEntry
{
    DBCPosition3D Pos;
    float       Radius;
    float       BoxLength;
    float       BoxWidth;
    float       BoxHeight;
    float       BoxYaw;
    uint16      ContinentID;
    uint16      PhaseID;
    uint16      PhaseGroupID;
    uint16      ShapeID;
    uint16      AreaTriggerActionSetID;
    uint8       PhaseUseFlags;
    uint8       ShapeType;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct AreaTriggerActionSetEntry
{
    int32       ID;
    uint16      Flags;
};

// FileOptions: Index, None
struct AreaTriggerBoxEntry
{
    int32       ID;
    float       Extents[3];
};

// FileOptions: Index, None
struct AreaTriggerCylinderEntry
{
    int32       ID;
    float       Radius;
    float       Height;
    float       ZOffset;
};

// FileOptions: Index, None
struct AreaTriggerSphereEntry
{
    int32       ID;
    float       MaxRadius;
};

// FileOptions: Index, None
struct ArmorLocationEntry
{
    int32       ID;
    float       Clothmodifier;
    float       Leathermodifier;
    float       Chainmodifier;
    float       Platemodifier;
    float       Modifier;
};

// FileOptions: Index, None
struct ArtifactEntry
{
    int32       ID;
    LocalizedString* Name;
    uint32      UiBarOverlayColor;
    uint32      UiBarBackgroundColor;
    uint32      UiNameColor;
    uint16      UiTextureKitID;
    uint16      ChrSpecializationID;
    uint8       ArtifactCategoryID;
    uint8       Flags;
    int32       UiModelSceneID;
    int32       SpellVisualKitID;
};

// FileOptions: None
struct ArtifactAppearanceEntry
{
    LocalizedString* Name;
    uint32      UiSwatchColor;
    float       UiModelSaturation;
    float       UiModelOpacity;
    int32       OverrideShapeshiftDisplayID;
    uint16      ArtifactAppearanceSetID;
    uint16      UiCameraID;
    uint8       DisplayIndex;
    uint8       ItemAppearanceModifierID;
    uint8       Flags;
    uint8       OverrideShapeshiftFormID;
    int32       ID;
    int32       UnlockPlayerConditionID;
    int32       UiItemAppearanceID;
    int32       UiAltItemAppearanceID;
};

// FileOptions: None
struct ArtifactAppearanceSetEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    uint16      UiCameraID;
    uint16      AltHandUICameraID;
    uint8       DisplayIndex;
    uint8       ForgeAttachmentOverride;
    uint8       Flags;
    int32       ID;
    uint8       ArtifactID;
};

// FileOptions: Index, None
struct ArtifactCategoryEntry
{
    int32       ID;
    uint16      XpMultCurrencyID;
    uint16      XpMultCurveID;
};

// FileOptions: None
struct ArtifactPowerEntry
{
    float       DisplayPos[2];
    uint8       ArtifactID;
    uint8       Flags;
    uint8       MaxPurchasableRank;
    uint8       Tier;
    int32       ID;
    int32       Label;
};

// FileOptions: Index, None
struct ArtifactPowerLinkEntry
{
    int32       ID;
    uint16      PowerA;
    uint16      PowerB;
};

// FileOptions: Index, None
struct ArtifactPowerPickerEntry
{
    int32       ID;
    int32       PlayerConditionID;
};

// FileOptions: Index, None
struct ArtifactPowerRankEntry
{
    int32       ID;
    int32       SpellID;
    float       AuraPointsOverride;
    uint16      ItemBonusListID;
    uint8       RankIndex;
    uint16      ArtifactPowerID;
};

// FileOptions: Index, None
struct ArtifactQuestXPEntry
{
    int32       ID;
    int32       Difficulty[10];
};

// FileOptions: Index, None
struct ArtifactTierEntry
{
    int32       ID;
    int32       ArtifactTier;
    int32       MaxNumTraits;
    int32       MaxArtifactKnowledge;
    int32       KnowledgePlayerCondition;
    int32       MinimumEmpowerKnowledge;
};

// FileOptions: Index, None
struct ArtifactUnlockEntry
{
    int32       ID;
    uint16      ItemBonusListID;
    uint8       PowerRank;
    int32       PowerID;
    int32       PlayerConditionID;
    uint8       ArtifactID;
};

// FileOptions: Index, None
struct AuctionHouseEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      FactionID;
    uint8       DepositRate;
    uint8       ConsignmentRate;
};

// FileOptions: Index, None
struct BankBagSlotPricesEntry
{
    int32       ID;
    int32       Cost;
};

// FileOptions: Index, None
struct BannedAddonsEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Version;
    uint8       Flags;
};

// FileOptions: None
struct BarberShopStyleEntry
{
    LocalizedString* DisplayName;
    LocalizedString* Description;
    float       CostModifier;
    uint8       Type;
    uint8       Race;
    uint8       Sex;
    uint8       Data;
    int32       ID;
};

// FileOptions: Index, None
struct BattlemasterListEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* GameType;
    LocalizedString* ShortDescription;
    LocalizedString* LongDescription;
    int32       IconFileDataID;
    int16       MapID[16];
    uint16      HolidayWorldState;
    uint16      RequiredPlayer_Condition_ID;
    uint8       InstanceType;
    uint8       GroupsAllowed;
    uint8       MaxGroupSize;
    uint8       MinLevel;
    uint8       MaxLevel;
    uint8       RatedPlayers;
    uint8       MinPlayers;
    uint8       MaxPlayers;
    uint8       Flags;
};

// FileOptions: Index, None
struct BattlePetAbilityEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    int32       IconFileDataID;
    uint16      BattlePetVisualID;
    int8        PetTypeEnum;
    uint8       Flags;
    int32       Cooldown;
};

// FileOptions: None
struct BattlePetAbilityEffectEntry
{
    uint16      BattlePetAbilityTurnID;
    uint16      BattlePetVisualID;
    uint16      AuraBattlePetAbilityID;
    uint16      BattlePetEffectPropertiesID;
    uint16      Param[6];
    uint8       OrderIndex;
    int32       ID;
};

// FileOptions: Index, None
struct BattlePetAbilityStateEntry
{
    int32       ID;
    int32       Value;
    uint8       BattlePetStateID;
    uint16      BattlePetAbilityID;
};

// FileOptions: None
struct BattlePetAbilityTurnEntry
{
    uint16      BattlePetAbilityID;
    uint16      BattlePetVisualID;
    uint8       OrderIndex;
    uint8       TurnTypeEnum;
    uint8       EventTypeEnum;
    int32       ID;
};

// FileOptions: Index, None
struct BattlePetBreedQualityEntry
{
    int32       ID;
    float       StateMultiplier;
    uint8       QualityEnum;
};

// FileOptions: Index, None
struct BattlePetBreedStateEntry
{
    int32       ID;
    uint16      Value;
    uint8       BattlePetStateID;
    uint8       BreedID;
};

// FileOptions: Index, None
struct BattlePetDisplayOverrideEntry
{
    int32       ID;
    int32       BattlePetSpeciesID;
    int32       PlayerConditionID;
    int32       CreatureDisplayInfoID;
    uint8       PriorityCategory;
};

// FileOptions: Index, None
struct BattlePetEffectPropertiesEntry
{
    int32       ID;
    LocalizedString* ParamLabel[6];
    uint16      BattlePetVisualID;
    uint8       ParamTypeEnum[6];
};

// FileOptions: Index, None
struct BattlePetNPCTeamMemberEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: None
struct BattlePetSpeciesEntry
{
    LocalizedString* SourceText;
    LocalizedString* Description;
    int32       CreatureID;
    int32       IconFileDataID;
    int32       SummonSpellID;
    uint16      Flags;
    uint8       PetTypeEnum;
    uint8       SourceTypeEnum;
    int32       ID;
    int32       CardUIModelSceneID;
    int32       LoadoutUIModelSceneID;
};

// FileOptions: Index, None
struct BattlePetSpeciesStateEntry
{
    int32       ID;
    uint32      Value;
    uint8       BattlePetStateID;
    uint16      SpeciesID;
};

// FileOptions: Index, None
struct BattlePetSpeciesXAbilityEntry
{
    int32       ID;
    uint16      BattlePetAbilityID;
    uint8       RequiredLevel;
    int8        SlotEnum;
    uint16      BattlePetSpeciesID;
};

// FileOptions: Index, None
struct BattlePetStateEntry
{
    int32       ID;
    LocalizedString* LuaName;
    uint16      BattlePetVisualID;
    uint16      Flags;
};

// FileOptions: Index, None
struct BattlePetVisualEntry
{
    int32       ID;
    LocalizedString* SceneScriptFunction;
    int32       SpellVisualID;
    uint16      CastMilliSeconds;
    uint16      ImpactMilliSeconds;
    uint16      SceneScriptPackageID;
    uint8       RangeTypeEnum;
    uint8       Flags;
};

// FileOptions: Index, None
struct BeamEffectEntry
{
    int32       ID;
    int32       BeamID;
    float       SourceMinDistance;
    float       FixedLength;
    int32       Flags;
    int32       SourceOffset;
    int32       DestOffset;
    int32       SourceAttachID;
    int32       DestAttachID;
    int32       SourcePositionerID;
    int32       DestPositionerID;
};

// FileOptions: Index, None
struct BoneWindModifierModelEntry
{
    int32       ID;
    int32       FileDataID;
    int32       BoneWindModifierID;
};

// FileOptions: Index, None
struct BoneWindModifiersEntry
{
    int32       ID;
    float       Multiplier[3];
    float       PhaseMultiplier;
};

// FileOptions: Index, None
struct BountyEntry
{
    int32       ID;
    int32       IconFileDataID;
    uint16      QuestID;
    uint16      FactionID;
    int32       TurninPlayerConditionID;
};

// FileOptions: Index, None
struct BountySetEntry
{
    int32       ID;
    uint16      LockedQuestID;
    int32       VisiblePlayerConditionID;
};

// FileOptions: Index, None
struct BroadcastTextEntry
{
    int32       ID;
    LocalizedString* Text;
    LocalizedString* Text1;
    uint16      EmoteID[3];
    uint16      EmoteDelay[3];
    uint16      EmotesID;
    uint8       LanguageID;
    uint8       Flags;
    int32       ConditionID;
    int32       SoundEntriesID[2];
};

// FileOptions: Index, None
struct CameraEffectEntry
{
    int32       ID;
    uint8       Flags;
};

// FileOptions: Index, None
struct CameraEffectEntryEntry
{
    int32       ID;
    float       Duration;
    float       Delay;
    float       Phase;
    float       Amplitude;
    float       AmplitudeB;
    float       Frequency;
    float       RadiusMin;
    float       RadiusMax;
    uint16      AmplitudeCurveID;
    uint8       OrderIndex;
    uint8       Flags;
    uint8       EffectType;
    uint8       DirectionType;
    uint8       MovementType;
    uint8       AttenuationType;
};

// FileOptions: Index, None
struct CameraModeEntry
{
    int32       ID;
    float       PositionOffset[3];
    float       TargetOffset[3];
    float       PositionSmoothing;
    float       RotationSmoothing;
    float       FieldOfView;
    uint16      Flags;
    uint8       Type;
    uint8       LockedPositionOffsetBase;
    uint8       LockedPositionOffsetDirection;
    uint8       LockedTargetOffsetBase;
    uint8       LockedTargetOffsetDirection;
};

// FileOptions: Index, None
struct CastableRaidBuffsEntry
{
    int32       ID;
    int32       CastingSpellID;
};

// FileOptions: None
struct CelestialBodyEntry
{
    int32       BaseFileDataID;
    int32       LightMaskFileDataID;
    int32       GlowMaskFileDataID[2];
    int32       AtmosphericMaskFileDataID;
    int32       AtmosphericModifiedFileDataID;
    int32       GlowModifiedFileDataID[2];
    float       ScrollURate[2];
    float       ScrollVRate[2];
    float       RotateRate;
    float       GlowMaskScale[2];
    float       AtmosphericMaskScale;
    float       Position[3];
    float       BodyBaseScale;
    uint16      SkyArrayBand;
    int32       ID;
};

// FileOptions: Index, None
struct Cfg_CategoriesEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      LocaleMask;
    uint8       CreateCharsetMask;
    uint8       ExistingCharsetMask;
    uint8       Flags;
};

// FileOptions: Index, None
struct Cfg_ConfigsEntry
{
    int32       ID;
    float       MaxDamageReductionPctPhysical;
    uint16      PlayerAttackSpeedBase;
    uint8       PlayerKillingAllowed;
    uint8       Roleplaying;
};

// FileOptions: Index, None
struct Cfg_RegionsEntry
{
    int32       ID;
    LocalizedString* Tag;
    int32       Raidorigin;
    int32       ChallengeOrigin;
    uint16      RegionID;
    uint8       RegionGroup_mask;
};

// FileOptions: Index, None
struct CharacterFaceBoneSetEntry
{
    int32       ID;
    int32       BoneSetFileDataID;
    uint8       SexID;
    uint8       FaceVariationIndex;
    uint8       Resolution;
};

// FileOptions: Index, None
struct CharacterFacialHairStylesEntry
{
    int32       ID;
    uint32      Geoset[5];
    uint8       RaceID;
    uint8       SexID;
    uint8       VariationID;
};

// FileOptions: Index, None
struct CharacterLoadoutEntry
{
    int32       ID;
    int64       RaceMask;
    uint8       ChrClassID;
    uint8       Purpose;
};

// FileOptions: Index, None
struct CharacterLoadoutItemEntry
{
    int32       ID;
    int32       ItemID;
    uint16      CharacterLoadoutID;
};

// FileOptions: Index, None
struct CharacterServiceInfoEntry
{
    int32       ID;
    LocalizedString* FlowTitle;
    LocalizedString* PopupTitle;
    LocalizedString* PopupDescription;
    int32       BoostType;
    int32       IconFileDataID;
    int32       Priority;
    int32       Flags;
    int32       ProfessionLevel;
    int32       BoostLevel;
    int32       Expansion;
    int32       PopupUITextureKitID;
};

// FileOptions: Index, None
struct CharBaseInfoEntry
{
    int32       ID;
    uint8       RaceID;
    uint8       ClassID;
};

// FileOptions: Index, None
struct CharBaseSectionEntry
{
    int32       ID;
    uint8       VariationEnum;
    uint8       ResolutionVariationEnum;
    uint8       LayoutResType;
};

// FileOptions: Index, None
struct CharComponentTextureLayoutsEntry
{
    int32       ID;
    uint16      Width;
    uint16      Height;
};

// FileOptions: Index, None
struct CharComponentTextureSectionsEntry
{
    int32       ID;
    int32       OverlapSectionMask;
    uint16      X;
    uint16      Y;
    uint16      Width;
    uint16      Height;
    uint8       CharComponentTextureLayoutID;
    uint8       SectionType;
};

// FileOptions: Index, None
struct CharHairGeosetsEntry
{
    int32       ID;
    int32       HdCustomGeoFileDataID;
    uint8       RaceID;
    uint8       SexID;
    uint8       VariationID;
    uint8       VariationType;
    uint8       GeosetID;
    uint8       GeosetType;
    uint8       Showscalp;
    uint8       ColorIndex;
    int32       CustomGeoFileDataID;
};

// FileOptions: Index, None
struct CharSectionsEntry
{
    int32       ID;
    int32       MaterialResourcesID[3];
    uint16      Flags;
    uint8       RaceID;
    uint8       SexID;
    uint8       BaseSection;
    uint8       VariationIndex;
    uint8       ColorIndex;
};

// FileOptions: Index, None
struct CharShipmentEntry
{
    int32       ID;
    int32       TreasureID;
    int32       Duration;
    int32       SpellID;
    int32       DummyItemID;
    int32       OnCompleteSpellID;
    uint16      ContainerID;
    uint16      GarrFollowerID;
    uint8       MaxShipments;
    uint8       Flags;
};

// FileOptions: Index, None
struct CharShipmentContainerEntry
{
    int32       ID;
    LocalizedString* PendingText;
    LocalizedString* Description;
    int32       WorkingSpellVisualID;
    uint16      UiTextureKitID;
    uint16      WorkingDisplayInfoID;
    uint16      SmallDisplayInfoID;
    uint16      MediumDisplayInfoID;
    uint16      LargeDisplayInfoID;
    uint16      CrossFactionID;
    uint8       BaseCapacity;
    uint8       GarrBuildingType;
    uint8       GarrTypeID;
    uint8       MediumThreshold;
    uint8       LargeThreshold;
    int8        Faction;
    int32       CompleteSpellVisualID;
};

// FileOptions: Index, None
struct CharStartOutfitEntry
{
    int32       ID;
    int32       ItemID[24];
    int32       PetDisplayID;
    uint8       ClassID;
    uint8       SexID;
    uint8       OutfitID;
    uint8       PetFamilyID;
    uint8       RaceID;
};

// FileOptions: Index, None
struct CharTitlesEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Name1;
    int16       MaskID;
    int8        Flags;
};

// FileOptions: Index, None
struct ChatChannelsEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Shortcut;
    int32       Flags;
    uint8       FactionGroup;
};

// FileOptions: Index, None
struct ChatProfanityEntry
{
    int32       ID;
    LocalizedString* Text;
    uint8       Language;
};

// FileOptions: None
struct ChrClassesEntry
{
    LocalizedString* PetNameToken;
    LocalizedString* Name;
    LocalizedString* NameFemale;
    LocalizedString* NameMale;
    LocalizedString* Filename;
    int32       CreateScreenFileDataID;
    int32       SelectScreenFileDataID;
    int32       LowResScreenFileDataID;
    int32       IconFileDataID;
    int32       StartingLevel;
    uint16      Flags;
    uint16      CinematicSequenceID;
    uint16      DefaultSpec;
    uint8       DisplayPower;
    uint8       SpellClassSet;
    uint8       AttackPowerPerStrength;
    uint8       AttackPowerPerAgility;
    uint8       RangedAttackPowerPerAgility;
    uint8       PrimaryStatPriority;
    int32       ID;
};

// FileOptions: Index, None
struct ChrClassesXPowerTypesEntry
{
    int32       ID;
    uint8       PowerType;
    uint8       ClassID;
};

// FileOptions: Index, None
struct ChrClassRaceSexEntry
{
    int32       ID;
    uint8       ClassID;
    uint8       RaceID;
    uint8       Sex;
    int32       Flags;
    int32       SoundID;
    int32       VoiceSoundFilterID;
};

// FileOptions: Index, None
struct ChrClassTitleEntry
{
    int32       ID;
    LocalizedString* NameMale;
    LocalizedString* NameFemale;
    uint8       ChrClassID;
};

// FileOptions: Index, None
struct ChrClassUIDisplayEntry
{
    int32       ID;
    uint8       ChrClassesID;
    int32       AdvGuidePlayerConditionID;
    int32       SplashPlayerConditionID;
};

// FileOptions: Index, None
struct ChrClassVillainEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       ChrClassID;
    uint8       Gender;
};

// FileOptions: Index, None
struct ChrCustomizationEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       Sex;
    int32       BaseSection;
    int32       UiCustomizationType;
    int32       Flags;
    int32       ComponentSection[3];
};

// FileOptions: None
struct ChrRacesEntry
{
    LocalizedString* ClientPrefix;
    LocalizedString* ClientFileString;
    LocalizedString* Name;
    LocalizedString* NameFemale;
    LocalizedString* NameLowercase;
    LocalizedString* NameFemale_lowercase;
    int32       Flags;
    int32       MaleDisplayID;
    int32       FemaleDisplayID;
    int32       CreateScreenFileDataID;
    int32       SelectScreenFileDataID;
    float       MaleCustomizeOffset[3];
    float       FemaleCustomizeOffset[3];
    int32       LowResScreenFileDataID;
    int32       StartingLevel;
    int32       UiDisplayOrder;
    uint16      FactionID;
    uint16      ResSicknessSpellID;
    uint16      SplashSoundID;
    uint16      CinematicSequenceID;
    uint8       BaseLanguage;
    uint8       CreatureType;
    uint8       Alliance;
    uint8       RaceRelated;
    uint8       UnalteredVisualRaceID;
    uint8       CharComponentTextureLayoutID;
    uint8       DefaultClassID;
    uint8       NeutralRaceID;
    uint8       DisplayRaceID;
    uint8       CharComponentTexLayoutHiResID;
    int32       ID;
    int32       HighResMaleDisplayID;
    int32       HighResFemaleDisplayID;
    int32       HeritageArmorAchievementID;
    int32       MaleSkeletonFileDataID;
    int32       FemaleSkeletonFileDataID;
    int32       AlteredFormStartVisualKitID[3];
    int32       AlteredFormFinishVisualKitID[3];
};

// FileOptions: None
struct ChrSpecializationEntry
{
    LocalizedString* Name;
    LocalizedString* FemaleName;
    LocalizedString* Description;
    int32       MasterySpellID[2];
    uint8       ClassID;
    uint8       OrderIndex;
    uint8       PetTalentType;
    uint8       Role;
    uint8       PrimaryStatPriority;
    int32       ID;
    int32       SpellIconFileID;
    int32       Flags;
    int32       AnimReplacements;
};

// FileOptions: None
struct ChrUpgradeBucketEntry
{
    uint16      ChrSpecializationID;
    int32       ID;
};

// FileOptions: Index, None
struct ChrUpgradeBucketSpellEntry
{
    int32       ID;
    int32       SpellID;
};

// FileOptions: None
struct ChrUpgradeTierEntry
{
    LocalizedString* DisplayName;
    uint8       OrderIndex;
    uint8       NumTalents;
    int32       ID;
};

// FileOptions: Index, None
struct CinematicCameraEntry
{
    int32       ID;
    int32       SoundID;
    float       Origin[3];
    float       OriginFacing;
    int32       FileDataID;
};

// FileOptions: Index, None
struct CinematicSequencesEntry
{
    int32       ID;
    int32       SoundID;
    uint16      Camera[8];
};

// FileOptions: Index, None
struct CloakDampeningEntry
{
    int32       ID;
    float       Angle[5];
    float       Dampening[5];
    float       TailAngle[2];
    float       TailDampening[2];
    float       TabardAngle;
    float       TabardDampening;
    float       ExpectedWeaponSize;
};

// FileOptions: Index, None
struct CombatConditionEntry
{
    int32       ID;
    uint16      WorldStateExpressionID;
    uint16      SelfConditionID;
    uint16      TargetConditionID;
    uint16      FriendConditionID[2];
    uint16      EnemyConditionID[2];
    uint8       FriendConditionOp[2];
    uint8       FriendConditionCount[2];
    uint8       FriendConditionLogic;
    uint8       EnemyConditionOp[2];
    uint8       EnemyConditionCount[2];
    uint8       EnemyConditionLogic;
};

// FileOptions: Index, None
struct CommentatorStartLocationEntry
{
    int32       ID;
    float       Pos[3];
    int32       MapID;
};

// FileOptions: Index, None
struct CommentatorTrackedCooldownEntry
{
    int32       ID;
    uint8       Priority;
    uint8       Flags;
    int32       SpellID;
};

// FileOptions: Index, None
struct ComponentModelFileDataEntry
{
    int32       ID;
    uint8       GenderIndex;
    uint8       ClassID;
    uint8       RaceID;
    uint8       PositionIndex;
};

// FileOptions: Index, None
struct ComponentTextureFileDataEntry
{
    int32       ID;
    uint8       GenderIndex;
    uint8       ClassID;
    uint8       RaceID;
};

// FileOptions: Index, None
struct ConfigurationWarningEntry
{
    int32       ID;
    LocalizedString* Warning;
    int32       Type;
};

// FileOptions: None
struct ContributionEntry
{
    LocalizedString* Description;
    LocalizedString* Name;
    int32       ID;
    int32       ManagedWorldStateInputID;
    int32       UiTextureAtlasMemberID[4];
    int32       OrderIndex;
};

// FileOptions: DataOffset, Index, None
struct ConversationLineEntry
{
    int32       ID;
    int32       BroadcastTextID;
    int32       SpellVisualKitID;
    uint32      AdditionalDuration;
    int16       NextConversationLineID;
    uint16      AnimKitID;
    uint8       SpeechType;
    uint8       StartAnimation;
    uint8       EndAnimation;
};

// FileOptions: Index, None
struct CreatureEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* NameAlt;
    LocalizedString* Title;
    LocalizedString* TitleAlt;
    int32       AlwaysItem[3];
    int32       MountCreatureID;
    int32       DisplayID[4];
    float       DisplayProbability[4];
    uint8       CreatureType;
    uint8       CreatureFamily;
    uint8       Classification;
    uint8       StartAnimState;
};

// FileOptions: Index, None
struct CreatureDifficultyEntry
{
    int32       ID;
    uint32      Flags[7];
    int16       FactionID;
    int8        ExpansionID;
    int8        MinLevel;
    int8        MaxLevel;
    int32       CreatureID; // or ContentTuningID from ContentTuninh.db2 @sandBoxScalingID
};

// FileOptions: None
struct CreatureDisplayInfoEntry
{
    int32       ID;
    float       CreatureModelScale;
    uint16      ModelID;
    uint16      NPCSoundID;
    uint8       SizeClass;
    uint8       Flags;
    int8        Gender;
    int32       ExtendedDisplayInfoID;
    int32       PortraitTextureFileDataID;
    uint8       CreatureModelAlpha;
    uint16      SoundID;
    float       PlayerOverrideScale;
    int32       PortraitCreatureDisplayInfoID;
    uint8       BloodID;
    uint16      ParticleColorID;
    uint32      DissolveEffectID;
    uint16      ObjectEffectPackageID;
    uint16      AnimReplacementSetID;
    int8        UnarmedWeaponType;
    int32       StateSpellVisualKitID;
    float       PetInstanceScale;
    int32       MountPoofSpellVisualKitID;
    int32       TextureVariationFileDataID[3];
};

// FileOptions: Index, None
struct CreatureDisplayInfoCondEntry
{
    int32       ID;
    int64       RaceMask;
    uint32      CustomOption0Mask[2];
    int32       CustomOption1Mask[2];
    int32       CustomOption2Mask[2];
    uint8       OrderIndex;
    uint8       Gender;
    int32       ClassMask;
    int32       SkinColorMask;
    int32       HairColorMask;
    int32       HairStyleMask;
    int32       FaceStyleMask;
    int32       FacialHairStyleMask;
    int32       CreatureModelDataID;
    int32       TextureVariationFileDataID[3];
};

// FileOptions: Index, None
struct CreatureDisplayInfoEvtEntry
{
    int32       ID;
    int32       Fourcc;
    int32       SpellVisualKitID;
    uint8       Flags;
};

// FileOptions: Index, None
struct CreatureDisplayInfoExtraEntry
{
    int32       ID;
    int32       BakeMaterialResourcesID;
    int32       HDBakeMaterialResourcesID;
    uint8       DisplayRaceID;
    uint8       DisplaySexID;
    uint8       DisplayClassID;
    uint8       SkinID;
    uint8       FaceID;
    uint8       HairStyleID;
    uint8       HairColorID;
    uint8       FacialHairID;
    uint8       CustomDisplayOption[3];
    uint8       Flags;
};

// FileOptions: Index, None
struct CreatureDisplayInfoTrnEntry
{
    int32       ID;
    int32       DstCreatureDisplayInfoID;
    float       MaxTime;
    int32       DissolveEffectID;
    int32       StartVisualKitID;
    int32       FinishVisualKitID;
};

// FileOptions: Index, None
struct CreatureDispXUiCameraEntry
{
    int32       ID;
    int32       CreatureDisplayInfoID;
    uint16      UiCameraID;
};

// FileOptions: Index, None
struct CreatureFamilyEntry
{
    int32       ID;
    LocalizedString* Name;
    float       MinScale;
    float       MaxScale;
    int32       IconFileID;
    uint16      SkillLine[2];
    uint16      PetFoodMask;
    uint8       MinScaleLevel;
    uint8       MaxScaleLevel;
    uint8       PetTalentType;
};

// FileOptions: Index, None
struct CreatureImmunitiesEntry
{
    int32       ID;
    int32       Mechanic[2];
    uint8       School;
    uint8       MechanicsAllowed;
    uint8       EffectsAllowed;
    uint8       StatesAllowed;
    uint8       Flags;
    int32       DispelType;
    int32       Effect[9];
    int32       State[16];
};

// FileOptions: Index, None
struct CreatureModelDataEntry
{
    int32       ID;
    float       ModelScale;
    float       FootprintTextureLength;
    float       FootprintTextureWidth;
    float       FootprintParticleScale;
    float       CollisionWidth;
    float       CollisionHeight;
    float       MountHeight;
    float       GeoBox[6];
    float       WorldEffectScale;
    float       AttachedEffectScale;
    float       MissileCollisionRadius;
    float       MissileCollisionPush;
    float       MissileCollisionRaise;
    float       OverrideLootEffectScale;
    float       OverrideNameScale;
    float       OverrideSelectionRadius;
    float       TamedPetBaseScale;
    float       HoverHeight;
    int32       Flags;
    int32       FileDataID;
    int32       SizeClass;
    int32       BloodID;
    int32       FootprintTextureID;
    int32       FoleyMaterialID;
    int32       FootstepCameraEffectID;
    int32       DeathThudCameraEffectID;
    int32       SoundID;
    int32       CreatureGeosetDataID;
};

// FileOptions: Index, None
struct CreatureMovementInfoEntry
{
    int32       ID;
    float       SmoothFacingChaseRate;
};

// FileOptions: Index, None
struct CreatureSoundDataEntry
{
    int32       ID;
    float       FidgetDelaySecondsMin;
    float       FidgetDelaySecondsMax;
    uint8       CreatureImpactType;
    int32       SoundExertionID;
    int32       SoundExertionCriticalID;
    int32       SoundInjuryID;
    int32       SoundInjuryCriticalID;
    int32       SoundInjuryCrushingBlowID;
    int32       SoundDeathID;
    int32       SoundStunID;
    int32       SoundStandID;
    int32       SoundFootstepID;
    int32       SoundAggroID;
    int32       SoundWingFlapID;
    int32       SoundWingGlideID;
    int32       SoundAlertID;
    int32       NPCSoundID;
    int32       LoopSoundID;
    int32       SoundJumpStartID;
    int32       SoundJumpEndID;
    int32       SoundPetAttackID;
    int32       SoundPetOrderID;
    int32       SoundPetDismissID;
    int32       BirthSoundID;
    int32       SpellCastDirectedSoundID;
    int32       SubmergeSoundID;
    int32       SubmergedSoundID;
    int32       CreatureSoundDataIDPet;
    int32       WindupSoundID;
    int32       WindupCriticalSoundID;
    int32       ChargeSoundID;
    int32       ChargeCriticalSoundID;
    int32       BattleShoutSoundID;
    int32       BattleShoutCriticalSoundID;
    int32       TauntSoundID;
    int32       SoundFidget[5];
    int32       CustomAttack[4];
};

// FileOptions: Index, None
struct CreatureTypeEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       Flags;
};

// FileOptions: None
struct CreatureXContributionEntry
{
    int32       ID;
    int32       ContributionID;
    uint32      CreatureId;
};

// FileOptions: Index, None
struct CriteriaEntry
{
    int32       ID;
    int32       Asset;
    int32       StartAsset;
    uint32      FailAsset;
    int32       ModifierTreeId;
    uint16      StartTimer;
    int16       EligibilityWorldStateId;
    uint8       Type;
    uint8       StartEvent;
    uint8       FailEvent;
    uint8       Flags;
    uint8       EligibilityWorldStateValue;
};

// FileOptions: Index, None
struct CriteriaTreeEntry
{
    int32       ID;
    LocalizedString* Description;
    int32       Amount;
    int16       Flags;
    int8        Operator;
    int32       CriteriaID;
    int32       Parent;
    int32       OrderIndex;
};

// FileOptions: Index, None
struct CriteriaTreeXEffectEntry
{
    int32       ID;
    uint16      WorldEffectID;
};

// FileOptions: Index, None
struct CurrencyCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       Flags;
    uint8       ExpansionID;
};

// FileOptions: Index, None
struct CurrencyTypesEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    int32       MaxQty;
    int32       MaxEarnablePerWeek;
    int32       Flags;
    uint8       CategoryID;
    uint8       SpellCategory;
    uint8       Quality;
    int32       InventoryIconFileID;
    int32       SpellWeight;
};

// FileOptions: Index, None
struct CurveEntry
{
    int32       ID;
    uint8       Type;
    uint8       Flags;
};

// FileOptions: Index, None
struct CurvePointEntry
{
    int32       ID;
    DBCPosition2D Pos;
    uint16      CurveID;
    uint8       OrderIndex;
};

// FileOptions: Index, None
struct DeathThudLookupsEntry
{
    int32       ID;
    uint8       SizeClass;
    uint8       TerrainTypeSoundID;
    int32       SoundEntryID;
    int32       SoundEntryIDWater;
};

// FileOptions: None
struct DecalPropertiesEntry
{
    int32       ID;
    int32       FileDataID;
    float       InnerRadius;
    float       OuterRadius;
    float       Rim;
    float       Gain;
    float       ModX;
    float       Scale;
    float       FadeIn;
    float       FadeOut;
    uint8       Priority;
    uint8       BlendMode;
    int32       TopTextureBlendSetID;
    int32       BotTextureBlendSetID;
    int32       GameFlags;
    int32       Flags;
    int32       CasterDecalPropertiesID;
};

// FileOptions: None
struct DeclinedWordEntry
{
    LocalizedString* Word;
    int32       ID;
};

// FileOptions: Index, None
struct DeclinedWordCasesEntry
{
    int32       ID;
    LocalizedString* DeclinedWord;
    uint8       CaseIndex;
};

// FileOptions: Index, None
struct DestructibleModelDataEntry
{
    int32       ID;
    uint16      State0Wmo;
    uint16      State1Wmo;
    uint16      State2Wmo;
    uint16      State3Wmo;
    uint16      HealEffectSpeed;
    uint8       State0ImpactEffectDoodadSet;
    uint8       State0AmbientDoodadSet;
    uint8       State0NameSet;
    uint8       State1DestructionDoodadSet;
    uint8       State1ImpactEffectDoodadSet;
    uint8       State1AmbientDoodadSet;
    uint8       State1NameSet;
    uint8       State2DestructionDoodadSet;
    uint8       State2ImpactEffectDoodadSet;
    uint8       State2AmbientDoodadSet;
    uint8       State2NameSet;
    uint8       State3InitDoodadSet;
    uint8       State3AmbientDoodadSet;
    uint8       State3NameSet;
    uint8       EjectDirection;
    uint8       DoNotHighlight;
    uint8       HealEffect;
};

// FileOptions: Index, None
struct DeviceBlacklistEntry
{
    int32       ID;
    uint16      VendorID;
    uint16      DeviceID;
};

// FileOptions: Index, None
struct DeviceDefaultSettingsEntry
{
    int32       ID;
    uint16      VendorID;
    uint16      DeviceID;
    uint8       DefaultSetting;
};

// FileOptions: Index, None
struct DifficultyEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      GroupSizeHealthCurveID;
    uint16      GroupSizeDmgCurveID;
    uint16      GroupSizeSpellPointsCurveID;
    uint8       FallbackDifficultyID;
    uint8       InstanceType;
    uint8       MinPlayers;
    uint8       MaxPlayers;
    int8        OldEnumValue;
    uint8       Flags;
    uint8       ToggleDifficultyID;
    uint8       ItemContext;
    uint8       OrderIndex;
};

// FileOptions: Index, None
struct DissolveEffectEntry
{
    int32       ID;
    float       Ramp;
    float       StartValue;
    float       EndValue;
    float       FadeInTime;
    float       FadeOutTime;
    float       Duration;
    float       Scale;
    float       FresnelIntensity;
    uint8       AttachID;
    uint8       ProjectionType;
    int32       TextureBlendSetID;
    int32       Flags;
    int32       CurveID;
    int32       Priority;
};

// FileOptions: Index, None
struct DriverBlacklistEntry
{
    int32       ID;
    int32       DriverVersionHi;
    int32       DriverVersionLow;
    uint16      VendorID;
    uint8       DeviceID;
    uint8       OsVersion;
    uint8       OsBits;
    uint8       Flags;
};

// FileOptions: None
struct DungeonEncounterEntry
{
    LocalizedString* Name;
    int32       CreatureDisplayID;
    uint16      MapID;
    uint8       DifficultyID;
    uint8       Bit;
    uint8       Flags;
    int32       ID;
    int32       OrderIndex;
    int32       SpellIconFileID;
};

// FileOptions: None
struct DungeonMapEntry
{
    float       Min[2];
    float       Max[2];
    uint16      MapID;
    uint16      ParentWorldMapID;
    uint8       FloorIndex;
    uint8       RelativeHeightIndex;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct DungeonMapChunkEntry
{
    int32       ID;
    float       MinZ;
    int32       DoodadPlacementID;
    uint16      MapID;
    uint16      WmoGroupID;
    uint16      DungeonMapID;
};

// FileOptions: Index, None
struct DurabilityCostsEntry
{
    int32       ID;
    uint16      WeaponSubClassCost[21];
    uint16      ArmorSubClassCost[8];
};

// FileOptions: Index, None
struct DurabilityQualityEntry
{
    int32       ID;
    float       Data;
};

// FileOptions: Index, None
struct EdgeGlowEffectEntry
{
    int32       ID;
    float       Duration;
    float       FadeIn;
    float       FadeOut;
    float       FresnelCoefficient;
    float       GlowRed;
    float       GlowGreen;
    float       GlowBlue;
    float       GlowAlpha;
    float       GlowMultiplier;
    float       InitialDelay;
    uint8       Flags;
    int32       CurveID;
    int32       Priority;
};

// FileOptions: Index, None
struct EmotesEntry
{
    int32       ID;
    int64       RaceMask;
    LocalizedString* EmoteSlashCommand;
    int32       EmoteFlags;
    int32       SpellVisualKitID;
    uint16      AnimID;
    uint8       EmoteSpecProc;
    int32       ClassMask;
    int32       EmoteSpecProcParam;
    int32       EventSoundID;
};

// FileOptions: Index, None
struct EmotesTextEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      EmoteID;
};

// FileOptions: Index, None
struct EmotesTextDataEntry
{
    int32       ID;
    LocalizedString* Text;
    uint8       RelationshipFlags;
};

// FileOptions: Index, None
struct EmotesTextSoundEntry
{
    int32       ID;
    uint8       RaceID;
    uint8       SexID;
    uint8       ClassID;
    int32       SoundID;
};

// FileOptions: Index, None
struct EnvironmentalDamageEntry
{
    int32       ID;
    uint16      VisualKitID;
    uint8       EnumID;
};

// FileOptions: None
struct ExhaustionEntry
{
    LocalizedString* Name;
    LocalizedString* CombatLogText;
    uint32      Xp;
    float       Factor;
    float       OutdoorHours;
    float       InnHours;
    float       Threshold;
    int32       ID;
};

// FileOptions: None
struct FactionEntry
{
    int64     ReputationRaceMask[4];
    LocalizedString* Name;
    LocalizedString* Description;
    int32       ID;
    int32       ReputationBase[4];
    float       ParentFactionMod[2];
    uint32      ReputationMax[4];
    int16       ReputationIndex;
    uint16      ReputationClassMask[4];
    uint16      ReputationFlags[4];
    uint16      ParentFactionID;
    uint16      ParagonFactionID;
    uint8       ParentFactionCap[2];
    uint8       Expansion;
    uint8       Flags;
    uint8       FriendshipRepID;

    bool CanHaveReputation() const;
    bool CanBeLfgBonus() const;
};

// FileOptions: None
struct FactionGroupEntry
{
    LocalizedString* InternalName;
    LocalizedString* Name;
    int32       ID;
    uint8       MaskID;
    int32       HonorCurrencyTextureFileID;
    int32       ConquestCurrencyTextureFileID;
};

// FileOptions: Index, None
struct FactionTemplateEntry
{
    int32       ID;
    uint16      Faction;
    uint16      Flags;
    uint16      Enemies[4];
    uint16      Friend[4];
    uint8       FactionGroup;
    uint8       FriendGroup;
    uint8       EnemyGroup;

    bool IsFriendlyTo(FactionTemplateEntry const& entry) const;
    bool IsHostileTo(FactionTemplateEntry const& entry) const;
    bool IsHostileToPlayers() const;
    bool IsNeutralToAll() const;
    bool IsContestedGuardFaction() const { return (Flags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD) != 0; }
};

// FileOptions: Index, None
struct FootprintTexturesEntry
{
    int32       ID;
    int32       TextureBlendsetID;
    int32       Flags;
    int32       FileDataID;
};

// FileOptions: Index, None
struct FootstepTerrainLookupEntry
{
    int32       ID;
    uint16      CreatureFootstepID;
    uint8       TerrainSoundID;
    int32       SoundID;
    int32       SoundIDSplash;
};

// FileOptions: Index, None
struct FriendshipRepReactionEntry
{
    int32       ID;
    LocalizedString* Reaction;
    uint16      ReactionThreshold;
    uint8       FriendshipRepID;
};

// FileOptions: None
struct FriendshipReputationEntry
{
    LocalizedString* Description;
    int32       TextureFileID;
    uint16      FactionID;
    int32       ID;
};

// FileOptions: Index, None
struct FullScreenEffectEntry
{
    int32       ID;
    float       Saturation;
    float       GammaRed;
    float       GammaGreen;
    float       GammaBlue;
    float       MaskOffsetY;
    float       MaskSizeMultiplier;
    float       MaskPower;
    float       ColorMultiplyRed;
    float       ColorMultiplyGreen;
    float       ColorMultiplyBlue;
    float       ColorMultiplyOffsetY;
    float       ColorMultiplyMultiplier;
    float       ColorMultiplyPower;
    float       ColorAdditionRed;
    float       ColorAdditionGreen;
    float       ColorAdditionBlue;
    float       ColorAdditionOffsetY;
    float       ColorAdditionMultiplier;
    float       ColorAdditionPower;
    float       BlurIntensity;
    float       BlurOffsetY;
    float       BlurMultiplier;
    float       BlurPower;
    int32       Flags;
    int32       TextureBlendSetID;
    int32       EffectFadeInMs;
    int32       EffectFadeOutMs;
};

// FileOptions: Index, None
struct GameObjectArtKitEntry
{
    int32       ID;
    int32       AttachModelFileID;
    int32       TextureVariationFileID[3];
};

// FileOptions: Index, None
struct GameObjectDiffAnimMapEntry
{
    int32       ID;
    uint16      AttachmentDisplayID;
    uint8       DifficultyID;
    uint8       Animation;
};

// FileOptions: Index, None
struct GameObjectDisplayInfoEntry
{
    int32       ID;
    int32       FileDataID;
    DBCPosition3D GeoBoxMin;
    DBCPosition3D GeoBoxMax;
    float       OverrideLootEffectScale;
    float       OverrideNameScale;
    int16       ObjectEffectPackageID;
};

// FileOptions: Index, None
struct GameObjectDisplayInfoXSoundKitEntry
{
    int32       ID;
    uint8       EventIndex;
    int32       SoundKitID;
};

// FileOptions: None
struct GameObjectsEntry
{
    LocalizedString* Name;
    DBCPosition3D Position;
    DBCPosition4D Rotation;
    float       Scale;
    uint32      PropValue[8];
    uint16      OwnerID;
    uint16      DisplayID;
    uint16      PhaseID;
    uint16      PhaseGroupID;
    uint8       PhaseUseFlags;
    uint8       TypeID;
    int32       ID;
};

// FileOptions: Index, None
struct GameTipsEntry
{
    int32       ID;
    LocalizedString* Text;
    uint16      MinLevel;
    uint16      MaxLevel;
    uint8       SortIndex;
};

// FileOptions: None
struct GarrAbilityEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    int32       IconFileDataID;
    uint16      Flags;
    uint16      FactionChangeGarrAbilityID;
    uint8       GarrAbilityCategoryID;
    uint8       GarrFollowerTypeID;
    int32       ID;
};

// FileOptions: Index, None
struct GarrAbilityCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: None
struct GarrAbilityEffectEntry
{
    float       CombatWeightBase;
    float       CombatWeightMax;
    float       ActionValueFlat;
    int32       ActionRecordID;
    uint16      GarrAbilityID;
    uint8       AbilityAction;
    uint8       AbilityTargetType;
    uint8       GarrMechanicTypeID;
    uint8       Flags;
    uint8       ActionRace;
    uint8       ActionHours;
    int32       ID;
};

// FileOptions: Index, None
struct GarrBuildingEntry
{
    int32       ID;
    LocalizedString* AllianceName;
    LocalizedString* HordeName;
    LocalizedString* Description;
    LocalizedString* Tooltip;
    int32       HordeGameObjectID;
    int32       AllianceGameObjectID;
    int32       IconFileDataID;
    uint16      CurrencyTypeID;
    uint16      HordeUiTextureKitID;
    uint16      AllianceUiTextureKitID;
    uint16      AllianceSceneScriptPackageID;
    uint16      HordeSceneScriptPackageID;
    uint16      GarrAbilityID;
    uint16      BonusGarrAbilityID;
    int16       GoldCost;
    uint8       GarrSiteID;
    uint8       BuildingType;
    uint8       UpgradeLevel;
    uint8       Flags;
    uint8       ShipmentCapacity;
    uint8       GarrTypeID;
    int32       BuildSeconds;
    int32       CurrencyQty;
    int32       MaxAssignments;
};

// FileOptions: Index, None
struct GarrBuildingDoodadSetEntry
{
    int32       ID;
    uint8       GarrBuildingID;
    uint8       Type;
    uint8       AllianceDoodadSet;
    uint8       HordeDoodadSet;
    uint8       SpecializationID;
};

// FileOptions: None
struct GarrBuildingPlotInstEntry
{
    DBCPosition2D MapOffset;
    uint16      UiTextureAtlasMemberID;
    uint16      GarrSiteLevelPlotInstID;
    uint8       GarrBuildingID;
    int32       ID;
};

// FileOptions: None
struct GarrClassSpecEntry
{
    LocalizedString* ClassSpec;
    LocalizedString* ClassSpecMale;
    LocalizedString* ClassSpecFemale;
    uint16      UiTextureAtlasMemberID;
    uint16      GarrFollItemSetID;
    uint8       FollowerClassLimit;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct GarrClassSpecPlayerCondEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       IconFileDataID;
    uint8       OrderIndex;
    int32       GarrClassSpecID;
    int32       PlayerConditionID;
    int32       FlavorGarrStringID;
};

// FileOptions: None
struct GarrEncounterEntry
{
    LocalizedString* Name;
    int32       CreatureID;
    float       UiAnimScale;
    float       UiAnimHeight;
    int32       PortraitFileDataID;
    int32       ID;
    int32       UiTextureKitID;
};

// FileOptions: None
struct GarrEncounterSetXEncounterEntry
{
    int32       ID;
    uint32      xEncounter;
    uint32      Encounter;
};

// FileOptions: Index, None
struct GarrEncounterXMechanicEntry
{
    int32       ID;
    uint8       GarrMechanicID;
    uint8       GarrMechanicSetID;
    uint16      GarrEncounterID;
};

// FileOptions: Index, None
struct GarrFollItemSetMemberEntry
{
    int32       ID;
    int32       ItemID;
    uint16      MinItemLevel;
    uint8       ItemSlot;
    uint16      GarrFollItemSetID;
};

// FileOptions: None
struct GarrFollowerEntry
{
    LocalizedString* HordeSourceText;
    LocalizedString* AllianceSourceText;
    LocalizedString* TitleName;
    int32       HordeCreatureID;
    int32       AllianceCreatureID;
    int32       HordeIconFileDataID;
    int32       AllianceIconFileDataID;
    int32       HordeSlottingBroadcastTextID;
    int32       AllySlottingBroadcastTextID;
    uint16      HordeGarrFollItemSetID;
    uint16      AllianceGarrFollItemSetID;
    uint16      ItemLevelWeapon;
    uint16      ItemLevelArmor;
    uint16      HordeUITextureKitID;
    uint16      AllianceUITextureKitID;
    uint8       GarrFollowerTypeID;
    uint8       HordeGarrFollRaceID;
    uint8       AllianceGarrFollRaceID;
    uint8       Quality;
    uint8       HordeGarrClassSpecID;
    uint8       AllianceGarrClassSpecID;
    uint8       FollowerLevel;
    uint8       Gender;
    uint8       Flags;
    uint8       HordeSourceTypeEnum;
    uint8       AllianceSourceTypeEnum;
    uint8       GarrTypeID;
    uint8       Vitality;
    uint8       ChrClassID;
    uint8       HordeFlavorGarrStringID;
    uint8       AllianceFlavorGarrStringID;
    int32       ID;
};

// FileOptions: Index, None
struct GarrFollowerLevelXPEntry
{
    int32       ID;
    uint16      XpToNextLevel;
    uint16      ShipmentXP;
    uint8       FollowerLevel;
    uint8       GarrFollowerTypeID;
};

// FileOptions: Index, None
struct GarrFollowerQualityEntry
{
    int32       ID;
    int32       XpToNextQuality;
    uint16      ShipmentXP;
    uint8       Quality;
    uint8       AbilityCount;
    uint8       TraitCount;
    uint8       GarrFollowerTypeID;
    int32       ClassSpecID;
};

// FileOptions: Index, None
struct GarrFollowerSetXFollowerEntry
{
    int32       ID;
    int32       GarrFollowerID;
    uint32      GarrFollowerSetID;
};

// FileOptions: Index, None
struct GarrFollowerTypeEntry
{
    int32       ID;
    uint16      MaxItemLevel;
    uint8       MaxFollowers;
    uint8       MaxFollowerBuildingType;
    uint8       GarrTypeID;
    uint8       LevelRangeBias;
    uint8       ItemLevelRangeBias;
    uint8       Flags;
};

// FileOptions: Index, None
struct GarrFollowerUICreatureEntry
{
    int32       ID;
    int32       CreatureID;
    float       Scale;
    uint8       FactionIndex;
    uint8       OrderIndex;
    uint8       Flags;
};

// FileOptions: Index, None
struct GarrFollowerXAbilityEntry
{
    int32       ID;
    uint16      GarrAbilityID;
    uint8       FactionIndex;
    uint16      GarrFollowerID;
};

// FileOptions: Index, None
struct GarrFollSupportSpellEntry
{
    int32       ID;
    int32       AllianceSpellID;
    int32       HordeSpellID;
    uint8       OrderIndex;
    uint32      GarrFollowerID;
};

// FileOptions: None
struct GarrItemLevelUpgradeDataEntry
{
    int32       ID;
    int32       Operation;
    int32       MinItemLevel;
    int32       MaxItemLevel;
    int32       FollowerTypeID;
};

// FileOptions: Index, None
struct GarrMechanicEntry
{
    int32       ID;
    float       Factor;
    uint8       GarrMechanicTypeID;
    int32       GarrAbilityID;
};

// FileOptions: None
struct GarrMechanicSetXMechanicEntry
{
    uint8       GarrMechanicID;
    int32       ID;
    uint32      GarrMechanicSetId;
};

// FileOptions: None
struct GarrMechanicTypeEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    int32       IconFileDataID;
    uint8       Category;
    int32       ID;
};

// FileOptions: None
struct GarrMissionEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    LocalizedString* Location;
    int32       MissionDuration;
    int32       OfferDuration;
    float       MapPos_[2];
    float       WorldPos[2];
    uint16      TargetItemLevel;
    uint16      UiTextureKitID;
    uint16      MissionCostCurrencyTypesID;
    uint8       TargetLevel;
    uint8       EnvGarrMechanicTypeID;
    uint8       MaxFollowers;
    uint8       OfferedGarrMissionTextureID;
    uint8       GarrMissionTypeID;
    uint8       GarrFollowerTypeID;
    uint8       BaseCompletionChance;
    uint8       FollowerDeathChance;
    uint8       GarrTypeID;
    int32       ID;
    int32       TravelDuration;
    int32       PlayerConditionID;
    int32       MissionCost;
    int32       Flags;
    int32       BaseFollowerXP;
    int32       AreaID;
    int32       OvermaxRewardPackID;
    int32       EnvGarrMechanicID;
    int32       RelationshipData;
};

// FileOptions: Index, None
struct GarrMissionTextureEntry
{
    int32       ID;
    float       Pos[2];
    uint16      UiTextureKitID;
};

// FileOptions: Index, None
struct GarrMissionTypeEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      UiTextureAtlasMemberID;
    uint16      UiTextureKitID;
};

// FileOptions: None
struct GarrMissionXEncounterEntry
{
    uint8       OrderIndex;
    int32       ID;
    uint32      GarrMissionID;
    int32       GarrEncounterID;
    int32       GarrEncounterSetID;
};

// FileOptions: Index, None
struct GarrMissionXFollowerEntry
{
    int32       ID;
    int32       GarrFollowerID;
    int32       GarrFollowerSetID;
    uint32      GarrMissionID;
};

// FileOptions: Index, None
struct GarrMssnBonusAbilityEntry
{
    int32       ID;
    float       Radius;
    int32       DurationSecs;
    uint16      GarrAbilityID;
    uint8       GarrFollowerTypeID;
    uint8       GarrMissionTextureID;
};

// FileOptions: Index, None
struct GarrPlotEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       AllianceConstructObjID;
    int32       HordeConstructObjID;
    uint8       UiCategoryID;
    uint8       PlotType;
    uint8       Flags;
    int32       UpgradeRequirement[2];
};

// FileOptions: Index, None
struct GarrPlotBuildingEntry
{
    int32       ID;
    uint8       GarrPlotID;
    uint8       GarrBuildingID;
};

// FileOptions: Index, None
struct GarrPlotInstanceEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       GarrPlotID;
};

// FileOptions: Index, None
struct GarrPlotUICategoryEntry
{
    int32       ID;
    LocalizedString* CategoryName;
    uint8       PlotType;
};

// FileOptions: Index, None
struct GarrSiteLevelEntry
{
    int32       ID;
    float       TownHallUiPos[2];
    uint16      MapID;
    uint16      UiTextureKitID;
    uint16      UpgradeMovieID;
    uint16      UpgradeCost;
    uint16      UpgradeGoldCost;
    uint8       GarrLevel;
    uint8       GarrSiteID;
    uint8       MaxBuildingLevel;
};

// FileOptions: Index, None
struct GarrSiteLevelPlotInstEntry
{
    int32       ID;
    float       UiMarkerPos[2];
    uint16      GarrSiteLevelID;
    uint8       GarrPlotInstanceID;
    uint8       UiMarkerSize;
};

// FileOptions: Index, None
struct GarrSpecializationEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Tooltip;
    int32       IconFileDataID;
    float       Param[2];
    uint8       BuildingType;
    uint8       SpecType;
    uint8       RequiredUpgradeLevel;
};

// FileOptions: Index, None
struct GarrStringEntry
{
    int32       ID;
    LocalizedString* Text;
};

// FileOptions: None
struct GarrTalentEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    int32       IconFileDataID;
    int32       ResearchDurationSecs;
    uint8       Tier;
    uint8       UiOrder;
    uint8       Flags;
    int32       ID;
    int32       GarrTalentTreeID;
    int32       GarrAbilityID;
    int32       PlayerConditionID;
    int32       ResearchCost;
    int32       ResearchCostCurrencyTypesID;
    int32       ResearchGoldCost;
    int32       PerkSpellID;
    int32       PerkPlayerConditionID;
    int32       RespecCost;
    int32       RespecCostCurrencyTypesID;
    int32       RespecDurationSecs;
    int32       RespecGoldCost;
};

// FileOptions: Index, None
struct GarrTalentTreeEntry
{
    int32       ID;
    uint16      UiTextureKitID;
    uint8       MaxTiers;
    uint8       UiOrder;
    int32       ClassID;
    int32       GarrTypeID;
};

// FileOptions: Index, None
struct GarrTypeEntry
{
    int32       ID;
    int32       Flags;
    int32       PrimaryCurrencyTypeID;
    int32       SecondaryCurrencyTypeID;
    int32       ExpansionID;
    int32       MapIDs[2];
};

// FileOptions: Index, None
struct GarrUiAnimClassInfoEntry
{
    int32       ID;
    float       ImpactDelaySecs;
    uint8       GarrClassSpecID;
    uint8       MovementType;
    int32       CastKit;
    int32       ImpactKit;
    int32       TargetImpactKit;
};

// FileOptions: Index, None
struct GarrUiAnimRaceInfoEntry
{
    int32       ID;
    float       MaleScale;
    float       MaleHeight;
    float       MaleSingleModelScale;
    float       MaleSingleModelHeight;
    float       MaleFollowerPageScale;
    float       MaleFollowerPageHeight;
    float       FemaleScale;
    float       FemaleHeight;
    float       FemaleSingleModelScale;
    float       FemaleSingleModelHeight;
    float       FemaleFollowerPageScale;
    float       FemaleFollowerPageHeight;
    uint8       GarrFollRaceID;
};

// FileOptions: Index, None
struct GemPropertiesEntry
{
    int32       ID;
    int32       Type;
    uint16      EnchantID;
    uint16      MinItemLevel;
};

// FileOptions: Index, None
struct GlobalStringsEntry
{
    int32       ID;
    LocalizedString* BaseTag;
    LocalizedString* TagText;
    uint8       Flags;
};

// FileOptions: Index, None
struct GlyphBindableSpellEntry
{
    int32       ID;
    uint32      SpellID;
    uint16      GlyphPropertiesID;
};

// FileOptions: Index, None
struct GlyphExclusiveCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct GlyphPropertiesEntry
{
    int32       ID;
    int32       SpellID;
    uint16      SpellIconID;
    uint8       GlyphType;
    uint8       GlyphExclusiveCategoryID;
};

// FileOptions: Index, None
struct GlyphRequiredSpecEntry
{
    int32       ID;
    uint16      ChrSpecializationID;
    uint16      GlyphPropertiesID;
};

// FileOptions: Index, None
struct GMSurveyAnswersEntry
{
    int32       ID;
    LocalizedString* Answer;
    uint8       SortIndex;
};

// FileOptions: Index, None
struct GMSurveyCurrentSurveyEntry
{
    int32       ID;
    uint8       GmsurveyID;
};

// FileOptions: Index, None
struct GMSurveyQuestionsEntry
{
    int32       ID;
    LocalizedString* Question;
};

// FileOptions: Index, None
struct GMSurveySurveysEntry
{
    int32       ID;
    uint8       Q[15];
};

// FileOptions: Index, None
struct GroundEffectDoodadEntry
{
    int32       ID;
    float       Animscale;
    float       PushScale;
    uint8       Flags;
    int32       ModelFileID;
};

// FileOptions: Index, None
struct GroundEffectTextureEntry
{
    int32       ID;
    uint16      DoodadID[4];
    uint8       DoodadWeight[4];
    uint8       Sound;
    int32       Density;
};

// FileOptions: Index, None
struct GroupFinderActivityEntry
{
    int32       ID;
    LocalizedString* FullName;
    LocalizedString* ShortName;
    uint16      MinGearLevelSuggestion;
    uint16      MapID;
    uint16      AreaID;
    uint8       GroupFinderCategoryID;
    uint8       GroupFinderActivityGrpID;
    uint8       OrderIndex;
    uint8       MinLevel;
    uint8       MaxLevelSuggestion;
    uint8       DifficultyID;
    uint8       Flags;
    uint8       DisplayType;
    uint8       MaxPlayers;
};

// FileOptions: Index, None
struct GroupFinderActivityGrpEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       OrderIndex;
};

// FileOptions: Index, None
struct GroupFinderCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       Flags;
    uint8       OrderIndex;
};

// FileOptions: Index, None
struct GuildColorBackgroundEntry
{
    int32       ID;
    uint8       Red;
    uint8       Green;
    uint8       Blue;
};

// FileOptions: Index, None
struct GuildColorBorderEntry
{
    int32       ID;
    uint8       Red;
    uint8       Green;
    uint8       Blue;
};

// FileOptions: Index, None
struct GuildColorEmblemEntry
{
    int32       ID;
    uint8       Red;
    uint8       Green;
    uint8       Blue;
};

// FileOptions: Index, None
struct GuildPerkSpellsEntry
{
    int32       ID;
    int32       SpellID;
};

// FileOptions: None
struct HeirloomEntry
{
    LocalizedString* SourceText;
    int32       ItemID;
    int32       LegacyItemID;
    int32       LegacyUpgradedItemID;
    int32       StaticUpgradedItemID;
    int32       UpgradeItemID[3];
    uint16      UpgradeItemBonusListID[3];
    uint8       Flags;
    uint8       SourceTypeEnum;
    int32       ID;
};

// FileOptions: Index, None
struct HelmetAnimScalingEntry
{
    int32       ID;
    float       Amount;
    int32       RaceID;
};

// FileOptions: Index, None
struct HelmetGeosetVisDataEntry
{
    int32       ID;
    uint32      HideGeoset[9];
};

// FileOptions: Index, None
struct HighlightColorEntry
{
    int32       ID;
    uint32      StartColor;
    uint32      MidColor;
    uint32      EndColor;
    uint8       Type;
    uint8       Flags;
};

// FileOptions: Index, None
struct HolidayDescriptionsEntry
{
    int32       ID;
    LocalizedString* Description;
};

// FileOptions: Index, None
struct HolidayNamesEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: None
struct HolidaysEntry
{
    int32       ID;
    int32       Date[16];
    uint16      Duration[10];
    uint16      Region;
    uint8       Looping;
    uint8       CalendarFlags[10];
    uint8       Priority;
    int8        CalendarFilterType;
    uint8       Flags;
    int32       HolidayNameID;
    int32       HolidayDescriptionID;
    int32       TextureFileDataID[3];
};

// FileOptions: Index, None
struct HotfixEntry
{
    int32       ID;
    LocalizedString* Tablename;
    int32       ObjectID;
    int32       Flags;
};

// FileOptions: Index, None
struct ImportPriceArmorEntry
{
    int32       ID;
    float       ClothModifier;
    float       LeatherModifier;
    float       ChainModifier;
    float       PlateModifier;
};

// FileOptions: Index, None
struct ImportPriceQualityEntry
{
    int32       ID;
    float       Data;
};

// FileOptions: Index, None
struct ImportPriceShieldEntry
{
    int32       ID;
    float       Data;
};

// FileOptions: Index, None
struct ImportPriceWeaponEntry
{
    int32       ID;
    float       Data;
};

// FileOptions: None
struct InvasionClientDataEntry
{
    LocalizedString* Name;
    float       IconLocation[2];
    int32       ID;
    int32       WorldStateID;
    int32       UiTextureAtlasMemberID;
    int32       ScenarioID;
    int32       WorldQuestID;
    int32       WorldStateValue;
    int32       InvasionEnabledWorldStateID;
};

// FileOptions: Index, None
struct ItemEntry
{
    int32       ID;
    int32       IconFileDataID;
    uint8       ClassID;
    uint8       SubclassID;
    int8        SoundOverrideSubclass;
    int8        Material;
    uint8       InventoryType;
    uint8       SheatheType;
    uint8       ItemGroupSoundsID;
};

// FileOptions: Index, None
struct ItemAppearanceEntry
{
    int32       ID;
    int32       ItemDisplayInfoID;
    int32       DefaultIconFileDataID;
    int32       UiOrder;
    uint8       DisplayType;
};

// FileOptions: Index, None
struct ItemAppearanceXUiCameraEntry
{
    int32       ID;
    uint16      ItemAppearanceID;
    uint16      UiCameraID;
};

// FileOptions: Index, None
struct ItemArmorQualityEntry
{
    int32       ID;
    float       Qualitymod[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemArmorShieldEntry
{
    int32       ID;
    float       Quality[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemArmorTotalEntry
{
    int32       ID;
    float       Cloth;
    float       Leather;
    float       Mail;
    float       Plate;
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemBagFamilyEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct ItemBonusEntry
{
    int32       ID;
    int32       Value[3];
    uint16      ParentItemBonusListID;
    uint8       Type;
    uint8       OrderIndex;
};

// FileOptions: None
struct ItemBonusListLevelDeltaEntry
{
    int16       ItemLevelDelta;
    int32       ID;
};

// FileOptions: Index, None
struct ItemBonusTreeNodeEntry
{
    int32       ID;
    uint16      ChildItemBonusTreeID;
    uint16      ChildItemBonusListID;
    uint16      ChildItemLevelSelectorID;
    uint8       ItemContext;
    uint16      BonusTreeID;
};

// FileOptions: Index, None
struct ItemChildEquipmentEntry
{
    int32       ID;
    int32       ChildItemID;
    int8        ChildItemEquipSlot;
    uint32      ItemID;
};

// FileOptions: Index, None
struct ItemClassEntry
{
    int32       ID;
    LocalizedString* ClassName;
    float       PriceModifier;
    uint8       OldEnumValue;
    uint8       Class;
};

// FileOptions: Index, None
struct ItemCurrencyCostEntry
{
    int32       ID;
    int32       ItemID;
};

// FileOptions: Index, None
struct ItemDamageAmmoEntry
{
    int32       ID;
    float       Quality[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemDamageOneHandEntry
{
    int32       ID;
    float       Quality[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemDamageOneHandCasterEntry
{
    int32       ID;
    float       Quality[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemDamageTwoHandEntry
{
    int32       ID;
    float       Quality[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemDamageTwoHandCasterEntry
{
    int32       ID;
    float       Quality[7];
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemDisenchantLootEntry
{
    int32       ID;
    uint16      MinLevel;
    uint16      MaxLevel;
    uint16      SkillRequired;
    int8        Subclass;
    uint8       Quality;
    int8        ExpansionID;
    uint8       ItemClass;
};

// FileOptions: Index, None
struct ItemDisplayInfoEntry
{
    int32       ID;
    int32       Flags;
    int32       ItemRangedDisplayInfoID;
    int32       ItemVisual;
    int32       ParticleColorID;
    int32       OverrideSwooshSoundKitID;
    int32       SheatheTransformMatrixID;
    int32       ModelType1;
    int32       StateSpellVisualKitID;
    int32       SheathedSpellVisualKitID;
    int32       UnsheathedSpellVisualKitID;
    int32       ModelResourcesID[2];
    int32       ModelMaterialResourcesID[2];
    int32       GeosetGroup[4];
    int32       AttachmentGeosetGroup[4];
    int32       HelmetGeosetVis[2];
};

// FileOptions: Index, None
struct ItemDisplayInfoMaterialResEntry
{
    int32       ID;
    int32       MaterialResourcesID;
    uint8       ComponentSection;
};

// FileOptions: Index, None
struct ItemDisplayXUiCameraEntry
{
    int32       ID;
    int32       ItemDisplayInfoID;
    uint16      UiCameraID;
};

// FileOptions: Index, None
struct ItemEffectEntry
{
    int32       ID;
    int32       SpellID;
    int32       CoolDownMSec;
    int32       CategoryCoolDownMSec;
    int16       Charges;
    uint16      SpellCategoryID;
    uint16      ChrSpecializationID;
    uint8       LegacySlotIndex;
    uint8       TriggerType;
    uint32      ItemID;
};

// FileOptions: Index, None
struct ItemExtendedCostEntry
{
    int32       ID;
    int32       ItemID[5];
    int32       CurrencyCount[5];
    uint16      ItemCount[5];
    uint16      RequiredArenaRating;
    uint16      CurrencyID[5];
    uint8       ArenaBracket;
    uint8       MinFactionID;
    uint8       MinReputation;
    uint8       Flags;
    uint8       RequiredAchievement;

    bool IsSeasonCurrencyRequirement(uint32 i) const;
};

// FileOptions: Index, None
struct ItemGroupSoundsEntry
{
    int32       ID;
    int32       Sound[4];
};

// FileOptions: Index, None
struct ItemLevelSelectorEntry
{
    int32       ID;
    uint16      MinItemLevel;
    uint16      ItemLevelSelectorQualitySetID;
};

// FileOptions: Index, None
struct ItemLevelSelectorQualityEntry
{
    int32       ID;
    int32       QualityItemBonusListID;
    uint8       Quality;
    uint16      ItemLevelSelectorQualitySetID;
};

// FileOptions: Index, None
struct ItemLevelSelectorQualitySetEntry
{
    int32       ID;
    uint16      IlvlRare;
    uint16      IlvlEpic;
};

// FileOptions: Index, None
struct ItemLimitCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       Quantity;
    uint8       Flags;
};

// FileOptions: Index, None
struct ItemLimitCategoryConditionEntry
{
    int32       ID;
    int8        AddQuantity;
    int32       PlayerConditionID;
    int32       ParentItemLimitCategoryID;
};

// FileOptions: None
struct ItemModifiedAppearanceEntry
{
    int32       ItemID;
    int32       ID;
    uint8       ItemAppearanceModifierID;
    uint16      ItemAppearanceID;
    uint8       OrderIndex;
    uint8       TransmogSourceTypeEnum;
};

// FileOptions: Index, None
struct ItemModifiedAppearanceExtraEntry
{
    int32       ID;
    int32       IconFileDataID;
    int32       UnequippedIconFileDataID;
    uint8       SheatheType;
    uint8       DisplayWeaponSubclassID;
    uint8       DisplayInventoryType;
};

// FileOptions: Index, None
struct ItemNameDescriptionEntry
{
    int32       ID;
    LocalizedString* Description;
    uint32      Color;
};

// FileOptions: Index, None
struct ItemPetFoodEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct ItemPriceBaseEntry
{
    int32       ID;
    float       Armor;
    float       Weapon;
    uint16      ItemLevel;
};

// FileOptions: Index, None
struct ItemRandomPropertiesEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      Enchantment[5];
};

// FileOptions: Index, None
struct ItemRandomSuffixEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      Enchantment[5];
    uint16      AllocationPct[5];
};

// FileOptions: Index, None
struct ItemRangedDisplayInfoEntry
{
    int32       ID;
    int32       MissileSpellVisualEffectNameID;
    int32       QuiverFileDataID;
    int32       CastSpellVisualID;
    int32       AutoAttackSpellVisualID;
};

// FileOptions: None
struct ItemSearchNameEntry
{
    int64       AllowableRace;
    LocalizedString* Display;
    int32       ID;
    uint32      Flags[3];
    uint16      ItemLevel;
    uint8       OverallQualityID;
    uint8       ExpansionID;
    uint8       RequiredLevel;
    uint16      MinFactionID;
    uint8       MinReputation;
    int32       AllowableClass;
    uint16      RequiredSkill;
    uint16      RequiredSkillRank;
    int32       RequiredAbility;
};

// FileOptions: Index, None
struct ItemSetEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       ItemID[17];
    uint16      RequiredSkillRank;
    int32       RequiredSkill;
    int32       SetFlags;
};

// FileOptions: Index, None
struct ItemSetSpellEntry
{
    int32       ID;
    int32       SpellID;
    uint16      ChrSpecID;
    uint8       Threshold;
    uint16      ItemSetID;
};

// FileOptions: DataOffset, Index, None
struct ItemSparseEntry
{
    int32       ID;
    int64       AllowableRace;
    LocalizedString* Display;
    LocalizedString* Display1;
    LocalizedString* Display2;
    LocalizedString* Display3;
    LocalizedString* Description;
    uint32      Flags[4];
    float       PriceRandomValue;
    float       PriceVariance;
    int32       VendorStackCount;
    int32       BuyPrice;
    int32       SellPrice;
    int32       RequiredAbility;
    int32       MaxCount;
    int32       Stackable;
    int32       StatPercentEditor[10];
    float       StatPercentageOfSocket[10];
    float       ItemRange;
    int32       BagFamily;
    float       QualityModifier;
    int32       DurationInInventory;
    float       DmgVariance;
    uint16      AllowableClass;
    uint16      ItemLevel;
    uint16      RequiredSkill;
    uint16      RequiredSkillRank;
    uint16      MinFactionID;
    int16       ItemStatValue[10];
    uint16      ScalingStatDistributionID;
    uint16      ItemDelay;
    uint16      PageID;
    uint16      StartQuestID;
    uint16      LockID;
    uint16      RandomSelect;
    uint16      ItemRandomSuffixGroupID;
    uint16      ItemSet;
    uint16      ZoneBound;
    uint16      InstanceBound;
    uint16      TotemCategoryID;
    uint16      SocketMatch_enchantment_id;
    uint16      GemProperties;
    uint16      LimitCategory;
    uint16      RequiredHoliday;
    uint16      RequiredTransmogHoliday;
    uint16      ItemNameDescriptionID;
    uint8       OverallQualityID;
    uint8       InventoryType;
    int8        RequiredLevel;
    uint8       RequiredPVPRank;
    uint8       RequiredPVPMedal;
    uint8       MinReputation;
    uint8       ContainerSlots;
    int8        StatModifierBonusStat[10];
    uint8       DamageDamageType;
    uint8       Bonding;
    uint8       LanguageID;
    uint8       PageMaterialID;
    int8        Material;
    uint8       SheatheType;
    uint8       SocketType[3];
    uint8       SpellWeightCategory;
    uint8       SpellWeight;
    uint8       ArtifactID;
    uint8       ExpansionID;
};

// FileOptions: Index, None
struct ItemSpecEntry
{
    int32       ID;
    uint16      SpecializationID;
    uint8       MinLevel;
    uint8       MaxLevel;
    uint8       ItemType;
    uint8       PrimaryStat;
    uint8       SecondaryStat;
};

// FileOptions: Index, None
struct ItemSpecOverrideEntry
{
    int32       ID;
    uint16      SpecID;
    uint32      ItemID;
};

// FileOptions: Index, None
struct ItemSubClassEntry
{
    int32       ID;
    LocalizedString* DisplayName;
    LocalizedString* VerboseName;
    uint16      Flags;
    uint8       ClassID;
    uint8       SubClassID;
    uint8       PrerequisiteProficiency;
    uint8       PostrequisiteProficiency;
    uint8       DisplayFlags;
    uint8       WeaponSwingSize;
    uint8       AuctionHouseSortOrder;
};

// FileOptions: Index, None
struct ItemSubClassMaskEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       Mask;
    uint8       ClassID;
};

// FileOptions: Index, None
struct ItemUpgradeEntry
{
    int32       ID;
    int32       CurrencyAmount;
    uint16      PrerequisiteID;
    uint16      CurrencyType;
    uint8       ItemUpgradePathID;
    uint8       ItemLevelIncrement;
};

// FileOptions: Index, None
struct ItemVisualsEntry
{
    int32       ID;
    int32       ModelFileID[5];
};

// FileOptions: Index, None
struct ItemXBonusTreeEntry
{
    int32       ID;
    uint16      ItemBonusTreeID;
    uint32      ItemID;
};

// FileOptions: Index, None
struct JournalEncounterEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    float       Map[2];
    uint16      DungeonMapID;
    uint16      WorldMapAreaID;
    uint16      FirstSectionID;
    uint16      JournalInstanceID;
    uint8       DifficultyMask;
    uint8       Flags;
    int32       OrderIndex;
    int32       MapDisplayConditionID;
};

// FileOptions: None
struct JournalEncounterCreatureEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    int32       CreatureDisplayInfoID;
    int32       FileDataID;
    int32       UiModelSceneID;
    uint16      JournalEncounterID;
    uint8       OrderIndex;
    int32       ID;
};

// FileOptions: None
struct JournalEncounterItemEntry
{
    int32       ItemID;
    uint16      JournalEncounterID;
    uint8       DifficultyMask;
    uint8       FactionMask;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct JournalEncounterSectionEntry
{
    int32       ID;
    LocalizedString* Title;
    LocalizedString* BodyText;
    int32       IconCreatureDisplayInfoID;
    int32       SpellID;
    int32       IconFileDataID;
    uint16      JournalEncounterID;
    uint16      NextSiblingSectionID;
    uint16      FirstChildSectionID;
    uint16      ParentSectionID;
    uint16      Flags;
    uint16      IconFlags;
    uint8       OrderIndex;
    uint8       Type;
    uint8       DifficultyMask;
    int32       UiModelSceneID;
};

// FileOptions: Index, None
struct JournalEncounterXDifficultyEntry
{
    int32       ID;
    uint8       DifficultyID;
};

// FileOptions: Index, None
struct JournalEncounterXMapLocEntry
{
    int32       ID;
    float       Map[2];
    uint8       Flags;
    int32       JournalEncounterID;
    int32       DungeonMapID;
    int32       MapDisplayConditionID;
};

// FileOptions: None
struct JournalInstanceEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    int32       ButtonFileDataID;
    int32       ButtonSmallFileDataID;
    int32       BackgroundFileDataID;
    int32       LoreFileDataID;
    uint16      MapID;
    uint16      AreaID;
    uint8       OrderIndex;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct JournalItemXDifficultyEntry
{
    int32       ID;
    uint8       DifficultyID;
};

// FileOptions: Index, None
struct JournalSectionXDifficultyEntry
{
    int32       ID;
    uint8       DifficultyID;
};

// FileOptions: Index, None
struct JournalTierEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct JournalTierXInstanceEntry
{
    int32       ID;
    uint16      JournalTierID;
    uint16      JournalInstanceID;
};

// FileOptions: Index, None
struct KeychainEntry
{
    int32       ID;
    uint8       Key[32];
};

// FileOptions: Index, None
struct KeystoneAffixEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    int32       Filedataid;
};

// FileOptions: None
struct LanguagesEntry
{
    LocalizedString* Name;
    int32       ID;
};

// FileOptions: Index, None
struct LanguageWordsEntry
{
    int32       ID;
    LocalizedString* Word;
    uint8       LanguageID;
};

// FileOptions: Index, None
struct LFGDungeonExpansionEntry
{
    int32       ID;
    uint16      RandomID;
    uint8       ExpansionLevel;
    uint8       HardLevelMin;
    uint8       HardLevelMax;
    int32       TargetLevelMin;
    int32       TargetLevelMax;
};

// FileOptions: Index, None
struct LFGDungeonGroupEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      OrderIndex;
    uint8       ParentGroupId;
    uint8       Typeid;
};

// FileOptions: Index, None
struct LFGDungeonsEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Description;
    uint32      Flags;
    float       MinGear;
    uint16      MaxLevel;
    uint16      TargetLevelMax;
    int16       MapID;
    uint16      RandomID;
    uint16      ScenarioID;
    uint16      FinalEncounterID;
    uint16      BonusReputationAmount;
    uint16      MentorItemLevel;
    uint16      RequiredPlayerConditionId;
    uint8       MinLevel;
    uint8       TargetLevel;
    uint8       TargetLevelMin;
    uint8       DifficultyID;
    uint8       TypeID;
    int8        Faction;
    uint8       ExpansionLevel;
    uint8       OrderIndex;
    uint8       GroupID;
    uint8       CountTank;
    uint8       CountHealer;
    uint8       CountDamage;
    uint8       MinCountTank;
    uint8       MinCountHealer;
    uint8       MinCountDamage;
    uint8       Substruct;
    uint8       MentorCharLevel;
    int32       IconTextureFileID;
    int32       RewardsBgTextureFileID;
    int32       PopupBgTextureFileID;

    uint32 Entry() const;
    bool IsScenario() const;
    bool IsChallenge() const;
    bool IsRaidFinder() const;
    uint32 GetMinGroupSize() const;
    uint32 GetMaxGroupSize() const;
    bool IsValid() const;
    LfgType GetInternalType() const;
    bool CanBeRewarded() const;
    bool FitsTeam(uint32 team) const;
    bool IsRaidType() const;
};

// FileOptions: Index, None
struct LfgDungeonsGroupingMapEntry
{
    int32       ID;
    uint16      RandomLfgDungeonsID;
    uint8       GroupID;
};

// FileOptions: Index, None
struct LFGRoleRequirementEntry
{
    int32       ID;
    uint8       RoleType;
    int32       PlayerConditionID;
    uint16      LfgDungeonsId;
};

// FileOptions: Index, None
struct LightEntry
{
    int32       ID;
    DBCPosition3D GameCoords;
    float       GameFalloffStart;
    float       GameFalloffEnd;
    uint16      ContinentID;
    uint16      LightParamsID[8];
};

// FileOptions: Index, None
struct LightDataEntry
{
    int32       ID;
    int32       DirectColor;
    int32       AmbientColor;
    uint32      SkyTopColor;
    int32       SkyMiddleColor;
    int32       SkyBand1Color;
    uint32      SkyBand2Color;
    int32       SkySmogColor;
    int32       SkyFogColor;
    int32       SunColor;
    int32       CloudSunColor;
    int32       CloudEmissiveColor;
    int32       CloudLayer1AmbientColor;
    int32       CloudLayer2AmbientColor;
    int32       OceanCloseColor;
    int32       OceanFarColor;
    int32       RiverCloseColor;
    int32       RiverFarColor;
    uint32      ShadowOpacity;
    float       FogEnd;
    float       FogScaler;
    float       CloudDensity;
    float       FogDensity;
    float       FogHeight;
    float       FogHeightScaler;
    float       FogHeightDensity;
    float       SunFogAngle;
    float       EndFogColorDistance;
    int32       SunFogColor;
    int32       EndFogColor;
    int32       FogHeightColor;
    int32       ColorGradingFileDataID;
    int32       HorizonAmbientColor;
    int32       GroundAmbientColor;
    uint16      LightParamID;
    uint16      Time;
};

// FileOptions: None
struct LightParamsEntry
{
    float       Glow;
    float       WaterShallowAlpha;
    float       WaterDeepAlpha;
    float       OceanShallowAlpha;
    float       OceanDeepAlpha;
    float       OverrideCelestialSphere[3];
    uint16      LightSkyboxID;
    uint8       HighlightSky;
    uint8       CloudTypeID;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct LightSkyboxEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       CelestialSkyboxFileDataID;
    int32       SkyboxFileDataID;
    uint8       Flags;
};

// FileOptions: Index, None
struct LiquidMaterialEntry
{
    int32       ID;
    uint8       LVF;
    uint8       Flags;
};

// FileOptions: Index, None
struct LiquidObjectEntry
{
    int32       ID;
    float       FlowDirection;
    float       FlowSpeed;
    uint16      LiquidTypeID;
    uint8       Fishable;
    uint8       Reflection;
};

// FileOptions: Index, None
struct LiquidTypeEntry
{
    int32       ID;
    LocalizedString* Name;
    char const* Texture[6];
    uint32      SpellID;
    float       MaxDarkenDepth;
    float       FogDarkenIntensity;
    float       AmbDarkenIntensity;
    float       DirDarkenIntensity;
    float       ParticleScale;
    uint32      Color[2];
    float       Float[18];
    uint32      Int[4];
    uint16      Flags;
    uint16      LightID;
    uint8       SoundBank;
    uint8       ParticleMovement;
    uint8       ParticleTexSlots;
    uint8       MaterialID;
    uint8       FrameCountTexture[6];
    int32       SoundID;
};

// FileOptions: Index, None
struct LoadingScreensEntry
{
    int32       ID;
    int32       NarrowScreenFileDataID;
    int32       WideScreenFileDataID;
    int32       WideScreen169FileDataID;
};

// FileOptions: Index, None
struct LoadingScreenTaxiSplinesEntry
{
    int32       ID;
    float       LocX[10];
    float       LocY[10];
    uint16      PathID;
    uint16      LoadingScreenID;
    uint8       LegIndex;
};

// FileOptions: Index, None
struct LocaleEntry
{
    int32       ID;
    int32       FontFileDataID;
    uint8       WowLocale;
    uint8       Secondary;
    uint8       ClientDisplayExpansion;
};

// FileOptions: Index, None
struct LocationEntry
{
    int32       ID;
    float       Pos[3];
    float       Rot[3];
};

// FileOptions: Index, None
struct LockEntry
{
    int32       ID;
    int32       Index[8];
    uint16      Skill[8];
    uint8       Type[8];
    uint8       Action[8];
};

// FileOptions: None
struct LockTypeEntry
{
    LocalizedString* Name;
    LocalizedString* ResourceName;
    LocalizedString* Verb;
    LocalizedString* CursorName;
    int32       ID;
};

// FileOptions: Index, None
struct LookAtControllerEntry
{
    int32       ID;
    float       ReactionEnableDistance;
    float       ReactionGiveupDistance;
    float       TorsoSpeedFactor;
    float       HeadSpeedFactor;
    uint16      ReactionEnableFOVDeg;
    uint16      ReactionGiveupTimeMS;
    uint16      ReactionIgnoreTimeMinMS;
    uint16      ReactionIgnoreTimeMaxMS;
    uint8       MaxTorsoYaw;
    uint8       MaxTorsoYawWhileMoving;
    uint8       MaxHeadYaw;
    uint8       MaxHeadPitch;
    uint8       Flags;
    int32       ReactionWarmUpTimeMSMin;
    int32       ReactionWarmUpTimeMSMax;
    int32       ReactionGiveupFOVDeg;
    int32       MaxTorsoPitchUp;
    int32       MaxTorsoPitchDown;
};

// FileOptions: Index, None
struct MailTemplateEntry
{
    int32       ID;
    LocalizedString* Body;
};

// FileOptions: None
struct ManagedWorldStateEntry
{
    int32       CurrentStageWorldStateID;
    int32       ProgressWorldStateID;
    int32       UpTimeSecs;
    int32       DownTimeSecs;
    int32       OccurrencesWorldStateID;
    int32       AccumulationStateTargetValue;
    int32       DepletionStateTargetValue;
    int32       AccumulationAmountPerMinute;
    int32       DepletionAmountPerMinute;
    int32       ID;
};

// FileOptions: Index, None
struct ManagedWorldStateBuffEntry
{
    int32       ID;
    uint32      OccurrenceValue;
    int32       BuffSpellID;
    int32       PlayerConditionID;
    int32       ManagedWorldStateID;
};

// FileOptions: Index, None
struct ManagedWorldStateInputEntry
{
    int32       ID;
    int32       ManagedWorldStateID;
    int32       QuestID;
    int32       ValidInputConditionID;
};

// FileOptions: None
struct ManifestInterfaceActionIconEntry
{
    int32       ID;
};

// FileOptions: Index, None
struct ManifestInterfaceDataEntry
{
    int32       ID;
    LocalizedString* FilePath;
    LocalizedString* FileName;
};

// FileOptions: None
struct ManifestInterfaceItemIconEntry
{
    int32       ID;
};

// FileOptions: Index, None
struct ManifestInterfaceTOCDataEntry
{
    int32       ID;
    LocalizedString* FilePath;
};

// FileOptions: None
struct ManifestMP3Entry
{
    int32       ID;
};

// FileOptions: Index, None
struct MapEntry
{
    int32       ID;
    LocalizedString* Directory;
    LocalizedString* MapName;
    LocalizedString* MapDescription0;
    LocalizedString* MapDescription1;
    LocalizedString* PvpShortDescription;
    LocalizedString* PvpLongDescription;
    uint32      Flags[2];
    float       MinimapIconScale;
    DBCPosition2D CorpsePos;
    uint16      AreaTableID;
    uint16      LoadingScreenID;
    int16       CorpseMapID;
    uint16      TimeOfDayOverride;
    int16       ParentMapID;
    int16       CosmeticParentMapID;
    uint16      WindSettingsID;
    uint8       InstanceType;
    uint8       MapType;
    uint8       ExpansionID;
    uint8       MaxPlayers;
    uint8       TimeOffset;

    bool IsDungeon() const;
    bool IsNonRaidDungeon() const;
    bool Instanceable() const;
    bool IsRaid() const;
    bool IsBattleground() const;
    bool IsBattleArena() const;
    bool IsBattlegroundOrArena() const;
    bool IsWorldMap() const;
    bool IsScenario() const;
    bool GetEntrancePos(int32& mapid, float& x, float& y) const;
    bool IsContinent() const;
    bool IsDynamicDifficultyMap() const;
    bool IsGarrison() const;
    bool IsDifficultyModeSupported(uint32 difficulty) const;
    bool CanCreatedZone() const;
    bool Is5pplDungeonOrRaid() const;
};

// FileOptions: Index, None
struct MapCelestialBodyEntry
{
    int32       ID;
    uint16      CelestialBodyID;
    int32       PlayerConditionID;
};

// FileOptions: None
struct MapChallengeModeEntry
{
    LocalizedString* Name;
    int32       ID;
    uint16      MapID;
    uint16      CriteriaCount[3];
    uint8       Flags;
};

// FileOptions: Index, None
struct MapDifficultyEntry
{
    int32       ID;
    LocalizedString* Message;
    uint8       DifficultyID;
    uint8       ResetInterval;
    uint8       MaxPlayers;
    uint8       LockID;
    uint8       Flags;
    uint8       ItemContext;
    int32       ItemContextPickerID;
    int16       MapID;

    uint32 GetRaidDuration() const;
};

// FileOptions: Index, None
struct MapDifficultyXConditionEntry
{
    int32       ID;
    LocalizedString* FailureDescription;
    uint32      PlayerConditionID;
    int32       OrderIndex;
    uint32      MapDifficultyId;    
};

// FileOptions: Index, None
struct MapLoadingScreenEntry
{
    int32       ID;
    float       Min[2];
    float       Max[2];
    int32       LoadingScreenID;
    int32       OrderIndex;
};

// FileOptions: Index, None
struct MarketingPromotionsXLocaleEntry
{
    int32       ID;
    LocalizedString* AcceptURL;
    int32       AdTexture;
    int32       LogoTexture;
    int32       AcceptButtonTexture;
    int32       DeclineButtonTexture;
    uint8       PromotionID;
    uint8       LocaleID;
};

// FileOptions: Index, None
struct MaterialEntry
{
    int32       ID;
    uint8       Flags;
    int32       FoleySoundID;
    int32       SheatheSoundID;
    int32       UnsheatheSoundID;
};

// FileOptions: Index, None
struct MissileTargetingEntry
{
    int32       ID;
    float       TurnLingering;
    float       PitchLingering;
    float       MouseLingering;
    float       EndOpacity;
    float       ArcSpeed;
    float       ArcRepeat;
    float       ArcWidth;
    float       ImpactRadius[2];
    float       ImpactTexRadius;
    int32       ArcTextureFileID;
    int32       ImpactTextureFileID;
    int32       ImpactModelFileID[2];
};

// FileOptions: Index, None
struct ModelAnimCloakDampeningEntry
{
    int32       ID;
    int32       AnimationDataID;
    int32       CloakDampeningID;
};

// FileOptions: None
struct ModelFileDataEntry
{
    uint8       Flags;
    uint8       LodCount;
    int32       ID;
    int32       ModelResourcesID;
};

// FileOptions: Index, None
struct ModelRibbonQualityEntry
{
    int32       ID;
    uint8       RibbonQualityID;
};

// FileOptions: Index, None
struct ModifierTreeEntry
{
    int32       ID;
    uint32      Asset;
    int32       SecondaryAsset;
    int32       Parent;
    uint8       Type;
    uint8       TertiaryAsset;
    uint8       Operator;
    uint8       Amount;
};

// FileOptions: None
struct MountEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    LocalizedString* SourceText;
    int32       SourceSpellID;
    float       MountFlyRideHeight;
    uint16      MountTypeID;
    uint16      Flags;
    uint8       SourceTypeEnum;
    int32       ID;
    int32       PlayerConditionID;
    int32       UiModelSceneID;
};

// FileOptions: None
struct MountCapabilityEntry
{
    int32       ReqSpellKnownID;
    int32       ModSpellAuraID;
    uint16      ReqRidingSkill;
    uint16      ReqAreaID;
    int16       ReqMapID;
    uint8       Flags;
    int32       ID;
    int32       ReqSpellAuraID;
};

// FileOptions: Index, None
struct MountTypeXCapabilityEntry
{
    int32       ID;
    uint16      MountTypeID;
    uint16      MountCapabilityID;
    uint8       OrderIndex;
};

// FileOptions: Index, None
struct MountXDisplayEntry
{
    int32       ID;
    int32       CreatureDisplayInfoID;
    int32       PlayerConditionID;
    uint32      MountID;
};

// FileOptions: Index, None
struct MovieEntry
{
    int32       ID;
    int32       AudioFileDataID;
    int32       SubtitleFileDataID;
    uint8       Volume;
    uint8       KeyID;
};

// FileOptions: Index, None
struct MovieFileDataEntry
{
    int32       ID;
    uint16      Resolution;
};

// FileOptions: Index, None
struct MovieVariationEntry
{
    int32       ID;
    int32       FileDataID;
    int32       OverlayFileDataID;
};

// FileOptions: Index, None
struct NameGenEntry
{
    int32       ID;
    char const* Name;
    uint8       RaceID;
    uint8       Sex;
};

// FileOptions: Index, None
struct NamesProfanityEntry
{
    int32       ID;
    char const* Name;
    int8        Language;
};

// FileOptions: Index, None
struct NamesReservedEntry
{
    int32       ID;
    char const* Name;
};

// FileOptions: Index, None
struct NamesReservedLocaleEntry
{
    int32       ID;
    char const* Name;
    uint8       LocaleMask;
};

// FileOptions: Index, None
struct NPCModelItemSlotDisplayInfoEntry
{
    int32       ID;
    int32       ItemDisplayInfoID;
    uint8       ItemSlot;
};

// FileOptions: Index, None
struct NPCSoundsEntry
{
    int32       ID;
    int32       SoundID[4];
};

// FileOptions: Index, None
struct ObjectEffectEntry
{
    int32       ID;
    float       Offset[3];
    uint16      ObjectEffectGroupID;
    uint8       TriggerType;
    uint8       EventType;
    uint8       EffectRecType;
    uint8       Attachment;
    int32       EffectRecID;
    int32       ObjectEffectModifierID;
};

// FileOptions: Index, None
struct ObjectEffectModifierEntry
{
    int32       ID;
    float       Param[4];
    uint8       InputType;
    uint8       MapType;
    uint8       OutputType;
};

// FileOptions: Index, None
struct ObjectEffectPackageElemEntry
{
    int32       ID;
    uint16      ObjectEffectPackageID;
    uint16      ObjectEffectGroupID;
    uint16      StateType;
};

// FileOptions: Index, None
struct OutlineEffectEntry
{
    int32       ID;
    float       Range;
    int32       UnitConditionID;
    int32       PassiveHighlightColorID;
    int32       HighlightColorID;
    int32       Priority;
    int32       Flags;
};

// FileOptions: Index, None
struct OverrideSpellDataEntry
{
    int32       ID;
    int32       Spells[10];
    uint32      PlayerActionBarFileDataID;
    uint8       Flags;
};

// FileOptions: Index, None
struct PageTextMaterialEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct PaperDollItemFrameEntry
{
    int32       ID;
    LocalizedString* ItemButtonName;
    uint8       SlotNumber;
    int32       SlotIconFileID;
};

// FileOptions: Index, None
struct ParagonReputationEntry
{
    int32       ID;
    int32       LevelThreshold;
    int32       QuestID;
    int32       FactionID;
};

// FileOptions: Index, None
struct ParticleColorEntry
{
    int32       ID;
    uint32      Start[3];
    uint32      Mid[3];
    uint32      End[3];
};

// FileOptions: Index, None
struct PathEntry
{
    int32       ID;
    uint8       Type;
    uint8       SplineType;
    uint8       Red;
    uint8       Green;
    uint8       Blue;
    uint8       Alpha;
    uint8       Flags;
};

// FileOptions: None
struct PathNodeEntry
{
    int32       ID;
    int32       LocationID;
    uint16      PathID;
    uint16      Sequence;
};

// FileOptions: None
struct PathNodePropertyEntry
{
    uint16      PathID;
    uint16      Sequence;
    uint8       PropertyIndex;
    int32       ID;
    int32       Value;
};

// FileOptions: None
struct PathPropertyEntry
{
    int32       Value;
    uint16      PathID;
    uint8       PropertyIndex;
    int32       ID;
};

// FileOptions: Index, None
struct PhaseEntry
{
    int32       ID;
    uint16      Flags;
};

// FileOptions: Index, None
struct PhaseShiftZoneSoundsEntry
{
    int32       ID;
    uint16      AreaID;
    uint16      PhaseID;
    uint16      PhaseGroupID;
    uint16      SoundAmbienceID;
    uint16      UwSoundAmbienceID;
    uint8       WmoAreaID;
    uint8       PhaseUseFlags;
    uint8       SoundProviderPreferencesID;
    uint8       UwSoundProviderPreferencesID;
    int32       ZoneIntroMusicID;
    int32       ZoneMusicID;
    int32       UwZoneIntroMusicID;
    int32       UwZoneMusicID;
};

// FileOptions: Index, None
struct PhaseXPhaseGroupEntry
{
    int32       ID;
    uint16      PhaseID;
    uint16      PhaseGroupID;
};

// FileOptions: None
struct PlayerConditionEntry
{
    int64       RaceMask;
    LocalizedString* FailureDescription;
    int32       ID;
    uint8       Flags;
    uint16      MinLevel;
    uint16      MaxLevel;
    int32       ClassMask;
    int8        Gender;
    int8        NativeGender;
    int32       SkillLogic;
    uint8       LanguageID;
    uint8       MinLanguage;
    int32       MaxLanguage;
    uint16      MaxFactionID;
    uint8       MaxReputation;
    int32       ReputationLogic;
    uint8       CurrentPvpFaction;
    uint8       MinPVPRank;
    uint8       MaxPVPRank;
    uint8       PvpMedal;
    int32       PrevQuestLogic;
    int32       CurrQuestLogic;
    int32       CurrentCompletedQuestLogic;
    int32       SpellLogic;
    int32       ItemLogic;
    uint8       ItemFlags;
    int32       AuraSpellLogic;
    uint16      WorldStateExpressionID;
    uint8       WeatherID;
    uint8       PartyStatus;
    uint8       LifetimeMaxPVPRank;
    int32       AchievementLogic;
    int32       LfgLogic;
    int32       AreaLogic;
    int32       CurrencyLogic;
    uint16      QuestKillID;
    int32       QuestKillLogic;
    int8        MinExpansionLevel;
    int8        MaxExpansionLevel;
    int8        MinExpansionTier;
    int8        MaxExpansionTier;
    uint8       MinGuildLevel;
    uint8       MaxGuildLevel;
    uint8       PhaseUseFlags;
    uint16      PhaseID;
    int32       PhaseGroupID;
    int32       MinAvgItemLevel;
    int32       MaxAvgItemLevel;
    uint16      MinAvgEquippedItemLevel;
    uint16      MaxAvgEquippedItemLevel;
    int8        ChrSpecializationIndex;
    int8        ChrSpecializationRole;
    int8        PowerType;
    uint8       PowerTypeComp;
    uint8       PowerTypeValue;
    int32       ModifierTreeID;
    int32       WeaponSubclassMask;
    uint16      SkillID[4];
    uint16      MinSkill[4];
    uint16      MaxSkill[4];
    int32       MinFactionID[3];
    uint8       MinReputation[3];
    uint16      PrevQuestID[4];
    uint16      CurrQuestID[4];
    uint16      CurrentCompletedQuestID[4];
    int32       SpellID[4];
    int32       ItemID[4];
    int32       ItemCount[4];
    uint16      Explored[2];
    int32       Time[2];
    int32       AuraSpellID[4];
    uint8       AuraStacks[4];
    uint16      Achievement[4];
    uint8       LfgStatus[4];
    uint8       LfgCompare[4];
    int32       LfgValue[4];
    uint16      AreaID[4];
    int32       CurrencyID[4];
    int32       CurrencyCount[4];
    int32       QuestKillMonster[6];
    int32       MovementFlags[2];
};

// FileOptions: Index, None
struct PositionerEntry
{
    int32       ID;
    float       StartLife;
    uint16      FirstStateID;
    uint8       Flags;
    uint8       StartLifePercent;
};

// FileOptions: Index, None
struct PositionerStateEntry
{
    int32       ID;
    float       EndLife;
    uint8       EndLifePercent;
    int32       NextStateID;
    int32       TransformMatrixID;
    int32       PosEntryID;
    int32       RotEntryID;
    int32       ScaleEntryID;
    int32       Flags;
};

// FileOptions: Index, None
struct PositionerStateEntryEntry
{
    int32       ID;
    float       ParamA;
    float       ParamB;
    uint16      SrcValType;
    uint16      SrcVal;
    uint16      DstValType;
    uint16      DstVal;
    uint8       EntryType;
    uint8       Style;
    uint8       SrcType;
    uint8       DstType;
    int32       CurveID;
};

// FileOptions: Index, None
struct PowerDisplayEntry
{
    int32       ID;
    LocalizedString* GlobalStringBaseTag;
    uint8       ActualType;
    uint8       Red;
    uint8       Green;
    uint8       Blue;
};

// FileOptions: Index, None
struct PowerTypeEntry
{
    int32       ID;
    LocalizedString* NameGlobalStringTag;
    LocalizedString* CostGlobalStringTag;
    float       RegenPeace;
    float       RegenCombat;
    uint16      MaxBasePower;
    uint16      RegenInterruptTimeMS;
    uint16      Flags;
    uint8       PowerTypeEnum;
    uint8       MinPower;
    uint8       CenterPower;
    uint8       DefaultPower;
    uint8       DisplayModifier;
};

// FileOptions: Index, None
struct PrestigeLevelInfoEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       BadgeTextureFileDataID;
    uint8       PrestigeLevel;
    uint8       Flags;
};

// FileOptions: Index, None
struct PVPBracketTypesEntry
{
    int32       ID;
    uint8       BracketID;
    int32       WeeklyQuestID[4];
};

// FileOptions: Index, None
struct PVPDifficultyEntry
{
    int32       ID;
    uint8       RangeIndex;
    uint8       MinLevel;
    uint8       MaxLevel;
    uint16      MapID;
};

// FileOptions: Index, None
struct PVPItemEntry
{
    int32       ID;
    int32       ItemID;
    uint8       ItemLevelDelta;
};

// FileOptions: Index, None
struct PvpRewardEntry
{
    int32       ID;
    int32       HonorLevel;
    int32       PrestigeLevel;
    int32       RewardPackID;
};

// FileOptions: Index, None
struct PvpScalingEffectEntry
{
    int32       ID;
    float       Value;
    int32       PvpScalingEffectTypeID;
    int32       SpecializationID;
};

// FileOptions: Index, None
struct PvpScalingEffectTypeEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: None
struct PvpTalentEntry
{
    uint32      ID;
    LocalizedString* Description;
    uint32      SpellID;
    uint32      OverrideSpellID;
    uint32      ExtraSpellID;
    uint32      TierID;
    uint32      ColumnIndex;
    uint32      Flags;
    uint32      ClassID;
    uint32      SpecID;
    uint32      Role;
};

// FileOptions: Index, None
struct PvpTalentUnlockEntry
{
    uint32      ID;
    uint32      TierID;
    uint32      ColumnIndex;
    uint32      HonorLevel;
};

// FileOptions: Index, None
struct QuestFactionRewardEntry
{
    int32       ID;
    uint16      Difficulty[10];
};

// FileOptions: Index, None
struct QuestFeedbackEffectEntry
{
    int32       ID;
    int32       FileDataID;
    uint16      MinimapAtlasMemberID;
    uint8       AttachPoint;
    uint8       PassiveHighlightColorType;
    uint8       Priority;
    uint8       Flags;
};

// FileOptions: Index, None
struct QuestInfoEntry
{
    int32       ID;
    LocalizedString* InfoName;
    uint16      Profession;
    uint8       Type;
    uint8       Modifiers;
};

// FileOptions: Index, None
struct QuestLineEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct QuestLineXQuestEntry
{
    int32       ID;
    uint16      QuestLineID;
    uint16      QuestID;
    uint8       OrderIndex;
};

// FileOptions: Index, None
struct QuestMoneyRewardEntry
{
    int32       ID;
    int32       Difficulty[10];
};

// FileOptions: Index, None
struct QuestObjectiveEntry
{
    int32       ID;
    LocalizedString* Description;
    int32       Amount;
    int32       ObjectID;
    uint8       Type;
    uint8       OrderIndex;
    uint8       StorageIndex;
    uint8       Flags;
    uint16      QuestID;
};

// FileOptions: Index, None
struct QuestPackageItemEntry
{
    int32       ID;
    int32       ItemID;
    uint16      PackageID;
    uint8       DisplayType;
    int32       ItemQuantity;
};

// FileOptions: None
struct QuestPOIBlobEntry
{
    int32       ID;
    uint16      MapID;
    uint16      WorldMapAreaID;
    uint8       NumPoints;
    uint8       Floor;
    int32       PlayerConditionID;
    int32       QuestID;
    int32       ObjectiveIndex;
};

// FileOptions: None
struct QuestPOIPointEntry
{
    uint32      QuestPOIBlobID;
    int16       X;
    int16       Y;
    int32       ID;
};

// FileOptions: Index, None
struct QuestSortEntry
{
    int32       ID;
    LocalizedString* SortName;
    uint8       UiOrderIndex;
};

// FileOptions: Index, None
struct QuestV2Entry
{
    int32       ID;
    uint16      UniqueBitFlag;
};

// FileOptions: None
struct QuestV2CliTaskEntry
{
    int64       FiltRaces;
    LocalizedString* QuestTitle;
    LocalizedString* BulletText;
    int32       StartItem;
    uint16      UniqueBitFlag;
    uint16      ConditionID;
    uint16      FiltClasses;
    uint16      FiltCompletedQuest[3];
    uint16      FiltMinSkillID;
    uint16      WorldStateExpressionID;
    uint8       FiltActiveQuest;
    uint8       FiltCompletedQuestLogic;
    uint8       FiltMaxFactionID;
    uint8       FiltMaxFactionValue;
    int8        FiltMaxLevel;
    uint8       FiltMinFactionID;
    uint8       FiltMinFactionValue;
    int8        FiltMinLevel;
    uint8       FiltMinSkillValue;
    uint8       FiltNonActiveQuest;
    int32       ID;
    int32       BreadCrumbID;
    int32       QuestInfoID;
    int32       SandboxScalingID;
};

// FileOptions: Index, None
struct QuestXGroupActivityEntry
{
    int32       ID;
    int32       QuestID;
    int32       GroupFinderActivityID;
};

// FileOptions: Index, None
struct QuestXPEntry
{
    int32       ID;
    uint16      Difficulty[10];
};

// FileOptions: Index, None
struct RandPropPointsEntry
{
    int32       ID;
    int32       Epic[5];
    int32       Superior[5];
    int32       Good[5];
};

// FileOptions: Index, None
struct RelicSlotTierRequirementEntry
{
    int32       ID;
    int32       PlayerConditionID;
    uint8       RelicIndex;
    uint8       RelicTier;
};

// FileOptions: Index, None
struct RelicTalentEntry
{
    int32       ID;
    uint16      ArtifactPowerID;
    uint8       ArtifactPowerLabel;
    int32       Type;
    int32       PVal;
    int32       Flags;
};

// FileOptions: Index, None
struct ResearchBranchEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       ItemID;
    uint16      CurrencyID;
    uint8       ResearchFieldID;
    int32       TextureFileID;
    int32       BigTextureFileID;
};

// FileOptions: None
struct ResearchFieldEntry
{
    LocalizedString* Name;
    uint8       Slot;
    int32       ID;
};

// FileOptions: None
struct ResearchProjectEntry
{
    LocalizedString* Name;
    LocalizedString* Description;
    int32       SpellID;
    uint16      ResearchBranchID;
    uint8       Rarity;
    uint8       NumSockets;
    int32       ID;
    int32       TextureFileID;
    int32       RequiredWeight;

    bool IsVaid() const;
};

// FileOptions: Index, None
struct ResearchSiteEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       QuestPoiBlobID;
    int16       MapID;
    int32       AreaPOIIconEnum;

    bool IsValid() const;
};

// FileOptions: Index, None
struct ResistancesEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       Flags;
    int32       FizzleSoundID;
};

// FileOptions: Index, None
struct RewardPackEntry
{
    int32       ID;
    int32       Money;
    float       ArtifactXPMultiplier;
    uint8       ArtifactXPDifficulty;
    uint8       ArtifactXPCategoryID;
    int32       CharTitleID;
    int32       TreasurePickerID;
};

// FileOptions: Index, None
struct RewardPackXCurrencyTypeEntry
{
    int32       ID;
    int32       CurrencyTypeID;
    int32       Quantity;
    uint32      RewardPackID;
};

// FileOptions: Index, None
struct RewardPackXItemEntry
{
    int32       ID;
    int32       ItemID;
    int32       ItemQuantity;
    uint32      RewardPackID;
};

// FileOptions: Index, None
struct RibbonQualityEntry
{
    int32       ID;
    float       MaxSampleTimeDelta;
    float       AngleThreshold;
    float       MinDistancePerSlice;
    uint8       NumStrips;
    int32       Flags;
};

// FileOptions: Index, None
struct RulesetItemUpgradeEntry
{
    int32       ID;
    int32       ItemID;
    uint16      ItemUpgradeID;
};

// FileOptions: Index, None
struct SandboxScalingEntry
{
    int32       ID;
    uint32      MinLevel;
    uint32      MaxLevel;
    uint32      Flags;
};

// FileOptions: Index, None
struct ScalingStatDistributionEntry
{
    int32       ID;
    uint16      PlayerLevelToItemLevelCurveID;
    uint32      MinLevel;
    uint32      MaxLevel;
};

// FileOptions: Index, None
struct ScenarioEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      AreaTableID;
    uint8       Flags;
    uint8       Type;
};

// FileOptions: Index, None
struct ScenarioStepEntry
{
    int32       ID;
    LocalizedString* Description;
    LocalizedString* Title;
    uint16      ScenarioID;
    uint16      Supersedes;
    uint16      RewardQuestID;
    uint8       OrderIndex;
    uint8       Flags;
    int32       Criteriatreeid;
    int32       RelatedStep;

    bool IsBonusObjective() const { return Flags & SCENARIO_STEP_FLAG_BONUS_OBJECTIVE; }
};

// FileOptions: Index, None
struct SceneScriptEntry
{
    int32       ID;
    uint16      FirstSceneScriptID;
    uint16      NextSceneScriptID;
};

// FileOptions: Index, None
struct SceneScriptGlobalTextEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Script;
};

// FileOptions: Index, None
struct SceneScriptPackageEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct SceneScriptPackageMemberEntry
{
    int32       ID;
    uint16      SceneScriptPackageID;
    uint16      SceneScriptID;
    uint16      ChildSceneScriptPackageID;
    uint8       OrderIndex;
};

// FileOptions: DataOffset, Index, None
struct SceneScriptTextEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Script;
};

// FileOptions: Index, None
struct ScheduledIntervalEntry
{
    int32       ID;
    int32       Flags;
    int32       RepeatType;
    int32       DurationSecs;
    int32       OffsetSecs;
    int32       DateAlignmentType;
};

// FileOptions: Index, None
struct ScheduledWorldStateEntry
{
    int32       ID;
    int32       ScheduledWorldStateGroupID;
    int32       WorldStateID;
    int32       Value;
    int32       DurationSecs;
    int32       Weight;
    int32       UniqueCategory;
    int32       Flags;
    int32       OrderIndex;
};

// FileOptions: Index, None
struct ScheduledWorldStateGroupEntry
{
    int32       ID;
    int32       Flags;
    int32       ScheduledIntervalID;
    int32       SelectionType;
    int32       SelectionCount;
    int32       Priority;
};

// FileOptions: None
struct ScheduledWorldStateXUniqCatEntry
{
    int32       ID;
    int32       ScheduledUniqueCategoryID;
};

// FileOptions: Index, None
struct ScreenEffectEntry
{
    int32       ID;
    LocalizedString* Name;
    uint32      Param[4];
    uint16      LightParamsID;
    uint16      LightParamsFadeIn;
    uint16      LightParamsFadeOut;
    uint16      TimeOfDayOverride;
    uint8       Effect;
    uint8       LightFlags;
    uint8       EffectMask;
    int32       FullScreenEffectID;
    int32       SoundAmbienceID;
    int32       ZoneMusicID;
};

// FileOptions: Index, None
struct ScreenLocationEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct SDReplacementModelEntry
{
    int32       ID;
    int32       SdFileDataID;
};

// FileOptions: Index, None
struct SeamlessSiteEntry
{
    int32       ID;
    int32       MapID;
};

// FileOptions: Index, None
struct ServerMessagesEntry
{
    int32       ID;
    LocalizedString* Text;
};

// FileOptions: Index, None
struct ShadowyEffectEntry
{
    int32       ID;
    uint32      PrimaryColor;
    uint32      SecondaryColor;
    float       Duration;
    float       Value;
    float       FadeInTime;
    float       FadeOutTime;
    float       InnerStrength;
    float       OuterStrength;
    float       InitialDelay;
    uint8       AttachPos;
    uint8       Flags;
    int32       CurveID;
    int32       Priority;
};

// FileOptions: Index, None
struct SkillLineEntry
{
    int32       ID;
    LocalizedString* DisplayName;
    LocalizedString* Description;
    LocalizedString* AlternateVerb;
    uint16      Flags;
    uint8       CategoryID;
    uint8       CanLink;
    int32       SpellIconFileID;
    int32       ParentSkillLineID;
};

// FileOptions: None
struct SkillLineAbilityEntry
{
    uint64      RaceMask;
    int32       ID;
    uint32      Spell;
    uint32      SupercedesSpell;
    uint16      SkillLine;
    uint16      TrivialSkillLineRankHigh;
    uint16      TrivialSkillLineRankLow;
    uint16      UniqueBit;
    uint16      TradeSkillCategoryID;
    uint8       NumSkillUps;
    int32       ClassMask;
    uint16      MinSkillLineRank;
    uint8       AcquireMethod;
    uint8       Flags;
};

// FileOptions: Index, None
struct SkillRaceClassInfoEntry
{
    int32       ID;
    int64       RaceMask;
    uint16      SkillID;
    uint16      Flags;
    uint16      SkillTierID;
    uint8       Availability;
    uint8       MinLevel;
    int32       ClassMask;
};

// FileOptions: Index, None
struct SoundAmbienceEntry
{
    int32       ID;
    uint8       Flags;
    int32       SoundFilterID;
    int32       FlavorSoundFilterID;
    int32       AmbienceID[2];
};

// FileOptions: Index, None
struct SoundAmbienceFlavorEntry
{
    int32       ID;
    int32       SoundEntriesIDDay;
    int32       SoundEntriesIDNight;
};

// FileOptions: None
struct SoundBusEntry
{
    float       DefaultVolume;
    uint8       Flags;
    uint8       DefaultPlaybackLimit;
    uint8       DefaultPriority;
    uint8       DefaultPriorityPenalty;
    uint8       BusEnumID;
    int32       ID;
};

// FileOptions: None
struct SoundBusOverrideEntry
{
    int32       ID;
    float       Volume;
    uint8       PlaybackLimit;
    uint8       Priority;
    uint8       PriorityPenalty;
    int32       SoundBusID;
    int32       PlayerConditionID;
};

// FileOptions: Index, None
struct SoundEmitterPillPointsEntry
{
    int32       ID;
    float       Position[3];
    uint16      SoundEmittersID;
};

// FileOptions: None
struct SoundEmittersEntry
{
    LocalizedString* Name;
    float       Position[3];
    float       Direction[3];
    uint16      WorldStateExpressionID;
    uint16      PhaseID;
    uint8       EmitterType;
    uint8       PhaseUseFlags;
    uint8       Flags;
    int32       ID;
    int32       SoundEntriesID;
    int32       PhaseGroupID;
};

// FileOptions: Index, None
struct SoundEnvelopeEntry
{
    int32       ID;
    int32       SoundKitID;
    int32       CurveID;
    uint16      DecayIndex;
    uint16      SustainIndex;
    uint16      ReleaseIndex;
    uint8       EnvelopeType;
    int32       Flags;
};

// FileOptions: Index, None
struct SoundFilterEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct SoundFilterElemEntry
{
    int32       ID;
    float       Params[9];
    uint8       FilterType;
};

// FileOptions: None
struct SoundKitEntry
{
    int32       ID;
    float       VolumeFloat;
    float       MinDistance;
    float       DistanceCutoff;
    uint16      Flags;
    uint16      SoundEntriesAdvancedID;
    uint8       SoundType;
    uint8       DialogType;
    uint8       EAXDef;
    float       VolumeVariationPlus;
    float       VolumeVariationMinus;
    float       PitchVariationPlus;
    float       PitchVariationMinus;
    float       PitchAdjust;
    uint16      BusOverwriteID;
    uint8       MaxInstances;
};

// FileOptions: None
struct SoundKitAdvancedEntry
{
    int32       ID;
    float       InnerRadius2D;
    float       DuckToSFX;
    float       DuckToMusic;
    float       InnerRadiusOfInfluence;
    float       OuterRadiusOfInfluence;
    int32       TimeToDuck;
    int32       TimeToUnduck;
    float       OuterRadius2D;
    uint8       Usage;
    int32       SoundKitID;
    int32       TimeA;
    int32       TimeB;
    int32       TimeC;
    int32       TimeD;
    int32       RandomOffsetRange;
    int32       TimeIntervalMin;
    int32       TimeIntervalMax;
    int32       DelayMin;
    int32       DelayMax;
    uint8       VolumeSliderCategory;
    float       DuckToAmbience;
    float       InsideAngle;
    float       OutsideAngle;
    float       OutsideVolume;
    uint8       MinRandomPosOffset;
    uint16      MaxRandomPosOffset;
    float       DuckToDialog;
    float       DuckToSuppressors;
    int32       MsOffset;
    int32       TimeCooldownMin;
    int32       TimeCooldownMax;
    uint8       MaxInstancesBehavior;
    uint8       VolumeControlType;
    int32       VolumeFadeInTimeMin;
    int32       VolumeFadeInTimeMax;
    int32       VolumeFadeInCurveID;
    int32       VolumeFadeOutTimeMin;
    int32       VolumeFadeOutTimeMax;
    int32       VolumeFadeOutCurveID;
    float       ChanceToPlay;
};

// FileOptions: Index, None
struct SoundKitChildEntry
{
    int32       ID;
    int32       ParentSoundKitID;
    int32       SoundKitID;
};

// FileOptions: Index, None
struct SoundKitEntryEntry
{
    int32       ID;
    int32       SoundKitID;
    int32       FileDataID;
    uint8       Frequency;
    float       Volume;
};

// FileOptions: Index, None
struct SoundKitFallbackEntry
{
    int32       ID;
    int32       SoundKitID;
    int32       FallbackSoundKitID;
};

// FileOptions: Index, None
struct SoundKitNameEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct SoundOverrideEntry
{
    int32       ID;
    uint16      ZoneIntroMusicID;
    uint16      ZoneMusicID;
    uint16      SoundAmbienceID;
    uint8       SoundProviderPreferencesID;
};

// FileOptions: Index, None
struct SoundProviderPreferencesEntry
{
    int32       ID;
    LocalizedString* Description;
    float       EAXDecayTime;
    float       EAX2EnvironmentSize;
    float       EAX2EnvironmentDiffusion;
    float       EAX2DecayHFRatio;
    float       EAX2ReflectionsDelay;
    float       EAX2ReverbDelay;
    float       EAX2RoomRolloff;
    float       EAX2AirAbsorption;
    float       EAX3DecayLFRatio;
    float       EAX3EchoTime;
    float       EAX3EchoDepth;
    float       EAX3ModulationTime;
    float       EAX3ModulationDepth;
    float       EAX3HFReference;
    float       EAX3LFReference;
    uint16      Flags;
    uint16      EAX2Room;
    uint16      EAX2RoomHF;
    uint16      EAX2Reflections;
    uint16      EAX2Reverb;
    uint8       EAXEnvironmentSelection;
    uint8       EAX3RoomLF;
};

// FileOptions: Index, None
struct SourceInfoEntry
{
    int32       ID;
    LocalizedString* SourceText;
    uint8       SourceTypeEnum;
    uint8       PvpFaction;
};

// FileOptions: Index, None
struct SpamMessagesEntry
{
    int32       ID;
    LocalizedString* Text;
};

// FileOptions: None
struct SpecializationSpellsEntry
{
    LocalizedString* Description;
    int32       SpellID;
    int32       OverridesSpellID;
    uint16      SpecID;
    uint8       DisplayOrder;
    int32       ID;
};

// FileOptions: Index, None
struct SpellActionBarPrefEntry
{
    int32       ID;
    int32       SpellID;
    uint16      PreferredActionBarMask;
};

// FileOptions: Index, None
struct SpellActivationOverlayEntry
{
    int32       ID;
    int32       SpellID;
    int32       OverlayFileDataID;
    uint32      Color;
    float       Scale;
    flag128     IconHighlightSpellClassMask;
    uint8       ScreenLocationID;
    uint8       TriggerType;
    int32       SoundEntriesID;
};

// FileOptions: Index, None
struct SpellAuraOptionsEntry
{
    int32       ID;
    uint32      ProcCharges;
    uint32      ProcTypeMask;
    int32       ProcCategoryRecovery;
    uint16      CumulativeAura;
    uint16      SpellProcsPerMinuteID;
    uint8       DifficultyID;
    uint8       ProcChance;
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellAuraRestrictionsEntry
{
    int32       ID;
    int32       CasterAuraSpell;
    int32       TargetAuraSpell;
    int32       ExcludeCasterAuraSpell;
    int32       ExcludeTargetAuraSpell;
    uint8       DifficultyID;
    uint8       CasterAuraState;
    uint8       TargetAuraState;
    uint8       ExcludeCasterAuraState;
    uint8       ExcludeTargetAuraState;
    uint32      SpellID;
};

// FileOptions: None
struct SpellAuraVisibilityEntry
{
    uint8       Type;
    uint8       Flags;
    int32       ID;
};

// FileOptions: Index, None
struct SpellAuraVisXChrSpecEntry
{
    int32       ID;
    uint16      ChrSpecializationID;
};

// FileOptions: Index, None
struct SpellCastingRequirementsEntry
{
    int32       ID;
    int32       SpellID;
    uint16      MinFactionID;
    int16       RequiredAreasID;
    uint16      RequiresSpellFocus;
    uint8       FacingCasterFlags;
    uint8       MinReputation;
    uint8       RequiredAuraVision;
};

// FileOptions: Index, None
struct SpellCastTimesEntry
{
    int32       ID;
    int32       Base;
    int32       Minimum;
    int16       PerLevel;
};

// FileOptions: Index, None
struct SpellCategoriesEntry
{
    int32       ID;
    uint16      Category;
    uint16      StartRecoveryCategory;
    uint16      ChargeCategory;
    uint8       DifficultyID;
    uint8       DefenseType;
    uint8       DispelType;
    uint8       Mechanic;
    uint8       PreventionType;
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       ChargeRecoveryTime;
    uint8       Flags;
    uint8       UsesPerWeek;
    uint8       MaxCharges;
    int32       TypeMask;
};

// FileOptions: Index, None
struct SpellChainEffectsEntry
{
    int32       ID;
    float       AvgSegLen;
    float       NoiseScale;
    float       TexCoordScale;
    int32       SegDuration;
    int32       Flags;
    float       JointOffsetRadius;
    float       MinorJointScale;
    float       MajorJointScale;
    float       JointMoveSpeed;
    float       JointSmoothness;
    float       MinDurationBetweenJointJumps;
    float       MaxDurationBetweenJointJumps;
    float       WaveHeight;
    float       WaveFreq;
    float       WaveSpeed;
    float       MinWaveAngle;
    float       MaxWaveAngle;
    float       MinWaveSpin;
    float       MaxWaveSpin;
    float       ArcHeight;
    float       MinArcAngle;
    float       MaxArcAngle;
    float       MinArcSpin;
    float       MaxArcSpin;
    float       DelayBetweenEffects;
    float       MinFlickerOnDuration;
    float       MaxFlickerOnDuration;
    float       MinFlickerOffDuration;
    float       MaxFlickerOffDuration;
    float       PulseSpeed;
    float       PulseOnLength;
    float       PulseFadeLength;
    float       WavePhase;
    float       TimePerFlipFrame;
    float       VariancePerFlipFrame;
    float       TextureCoordScaleU[3];
    float       TextureCoordScaleV[3];
    float       TextureRepeatLengthU[3];
    float       TextureRepeatLengthV[3];
    int32       TextureParticleFileDataID;
    float       StartWidth;
    float       EndWidth;
    float       ParticleScaleMultiplier;
    float       ParticleEmissionRateMultiplier;
    uint16      SegDelay;
    uint16      JointCount;
    uint16      SpellChainEffectID[11];
    uint16      WidthScaleCurveID;
    uint8       JointsPerMinorJoint;
    uint8       MinorJointsPerMajorJoint;
    uint8       Alpha;
    uint8       Red;
    uint8       Green;
    uint8       Blue;
    uint8       BlendMode;
    uint8       RenderLayer;
    uint8       NumFlipFramesU;
    uint8       NumFlipFramesV;
    int32       SoundKitID;
    int32       TextureFileDataID[3];
};

// FileOptions: Index, None
struct SpellClassOptionsEntry
{
    int32       ID;
    int32       SpellID;
    flag128     SpellClassMask;
    uint8       SpellClassSet;
    int32       ModalNextSpell;
};

// FileOptions: Index, None
struct SpellCooldownsEntry
{
    int32       ID;
    int32       CategoryRecoveryTime;
    int32       RecoveryTime;
    int32       StartRecoveryTime;
    uint8       DifficultyID;
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellDescriptionVariablesEntry
{
    int32       ID;
    LocalizedString* Variables;
};

// FileOptions: Index, None
struct SpellDispelTypeEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* InternalName;
    uint8       Mask;
    uint8       ImmunityPossible;
};

// FileOptions: Index, None
struct SpellDurationEntry
{
    int32       ID;
    int32       Duration;
    int32       MaxDuration;
    int32       DurationPerLevel;
};

// FileOptions: Index, None
struct SpellEffectEntry
{
    int32       ID;
    int32       Effect;
    int32       EffectBasePoints;
    int32       EffectIndex;
    int32       EffectAura;
    int32       DifficultyID;
    float       EffectAmplitude;
    int32       EffectAuraPeriod;
    float       EffectBonusCoefficient;
    float       EffectChainAmplitude;
    int32       EffectChainTargets;
    int32       EffectDieSides;
    int32       EffectItemType;
    int32       EffectMechanic;
    float       EffectPointsPerResource;
    float       EffectRealPointsPerLevel;
    int32       EffectTriggerSpell;
    float       EffectPosFacing;
    int32       EffectAttributes;
    float       BonusCoefficientFromAP;
    float       PvpMultiplier;
    float       Coefficient;
    float       Variance;
    float       ResourceCoefficient;
    float       GroupSizeBasePointsCoefficient;
    flag128     EffectSpellClassMask;
    int32       EffectMiscValue[2];
    int32       EffectRadiusIndex[2];
    int32       ImplicitTarget[2];
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* NameSubtext;
    LocalizedString* Description;
    LocalizedString* AuraDescription;

    SpellEffectEntry const* GetSpellEffect(uint32 eff, uint8 diff = 0) const;
};

// FileOptions: Index, None
struct SpellEffectEmissionEntry
{
    int32       ID;
    float       EmissionRate;
    float       ModelScale;
    uint16      AreaModelID;
    uint8       Flags;
};

// FileOptions: Index, None
struct SpellEquippedItemsEntry
{
    int32       ID;
    int32       SpellID;
    int32       EquippedItemInvTypes;
    int32       EquippedItemSubclass;
    uint8       EquippedItemClass;
};

// FileOptions: Index, None
struct SpellFlyoutEntry
{
    int32       ID;
    int64       RaceMask;
    LocalizedString* Name;
    LocalizedString* Description;
    uint8       Flags;
    int32       ClassMask;
    int32       SpellIconFileID;
};

// FileOptions: Index, None
struct SpellFlyoutItemEntry
{
    int32       ID;
    int32       SpellID;
    uint8       Slot;
};

// FileOptions: Index, None
struct SpellFocusObjectEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct SpellInterruptsEntry
{
    int32       ID;
    uint8       DifficultyID;
    uint16      InterruptFlags;
    int32       AuraInterruptFlags[2];
    int32       ChannelInterruptFlags[2];
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellItemEnchantmentEntry
{
    int32       ID;
    LocalizedString* Name;
    uint32      EffectArg[3];
    float       EffectScalingPoints[3];
    uint32      TransmogCost;
    uint32      TextureFileDataID;
    uint16      EffectPointsMin[3];
    uint16      ItemVisual;
    uint16      Flags;
    uint16      RequiredSkillID;
    uint16      RequiredSkillRank;
    uint16      ItemLevel;
    uint8       Charges;
    uint8       Effect[3];
    uint8       ConditionID;
    uint8       MinLevel;
    uint8       MaxLevel;
    int8        ScalingClass;
    int8        ScalingClassRestricted;
    int32       TransmogPlayerConditionID;
};

// FileOptions: Index, None
struct SpellItemEnchantmentConditionEntry
{
    int32       ID;
    int32       LtOperand[5];
    uint8       LtOperandType[5];
    uint8       Operator[5];
    uint8       RtOperandType[5];
    uint8       RtOperand[5];
    uint8       Logic[5];
};

// FileOptions: Index, None
struct SpellKeyboundOverrideEntry
{
    int32       ID;
    LocalizedString* Function;
    int32       Data;
    uint8       Type;
};

// FileOptions: Index, None
struct SpellLabelEntry
{
    int32       ID;
    int32       LabelID;
};

// FileOptions: Index, None
struct SpellLearnSpellEntry
{
    int32       ID;
    int32       SpellID;
    int32       LearnSpellID;
    int32       OverridesSpellID;
};

// FileOptions: Index, None
struct SpellLevelsEntry
{
    int32       ID;
    uint16      BaseLevel;
    uint16      MaxLevel;
    uint16      SpellLevel;
    uint8       DifficultyID;
    uint8       MaxPassiveAuraLevel;
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellMechanicEntry
{
    int32       ID;
    LocalizedString* StateName;
};

// FileOptions: Index, None
struct SpellMiscEntry
{
    int32       ID;
    uint16      CastingTimeIndex;
    uint16      DurationIndex;
    uint16      RangeIndex;
    uint8       SchoolMask;
    int32       SpellIconFileDataID;
    float       Speed;
    int32       ActiveIconFileDataID;
    float       LaunchDelay;
    uint8       DifficultyID;
    int32       Attributes[14];
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellMissileEntry
{
    int32       ID;
    int32       SpellID;
    float       DefaultPitchMin;
    float       DefaultPitchMax;
    float       DefaultSpeedMin;
    float       DefaultSpeedMax;
    float       RandomizeFacingMin;
    float       RandomizeFacingMax;
    float       RandomizePitchMin;
    float       RandomizePitchMax;
    float       RandomizeSpeedMin;
    float       RandomizeSpeedMax;
    float       Gravity;
    float       MaxDuration;
    float       CollisionRadius;
    uint8       Flags;
};

// FileOptions: Index, None
struct SpellMissileMotionEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* ScriptBody;
    uint8       Flags;
    uint8       MissileCount;
};

// FileOptions: None
struct SpellPowerEntry
{
    int32       ManaCost;
    float       PowerCostPct;
    float       PowerPctPerSecond;
    int32       RequiredAuraSpellID;
    float       PowerCostMaxPct;
    uint8       OrderIndex;
    int8        PowerType;
    int32       ID;
    uint32      ManaCostPerLevel;
    int32       ManaPerSecond;
    int32       OptionalCost;
    int32       PowerDisplayID;
    int32       AltPowerBarID;
    uint32      SpellID;
};

// FileOptions: None
struct SpellPowerDifficultyEntry
{
    uint8       DifficultyID;
    uint8       OrderIndex;
    int32       ID;
};

// FileOptions: None
struct SpellProceduralEffectEntry
{
    float       Value[4];
    uint8       Type;
    int32       ID;
};

// FileOptions: Index, None
struct SpellProcsPerMinuteEntry
{
    int32       ID;
    float       BaseProcRate;
    uint8       Flags;
};

// FileOptions: Index, None
struct SpellProcsPerMinuteModEntry
{
    int32       ID;
    float       Coeff;
    uint16      Param;
    uint8       Type;
    uint16      SpellProcsPerMinuteID;
};

// FileOptions: Index, None
struct SpellRadiusEntry
{
    int32       ID;
    float       Radius;
    float       RadiusPerLevel;
    float       RadiusMin;
    float       RadiusMax;
};

// FileOptions: Index, None
struct SpellRangeEntry
{
    int32       ID;
    LocalizedString* DisplayName;
    LocalizedString* DisplayNameShort;
    float       RangeMin[2];
    float       RangeMax[2];
    uint8       Flags;
};

// FileOptions: Index, None
struct SpellReagentsEntry
{
    int32       ID;
    int32       SpellID;
    int32       Reagent[8];
    uint16      ReagentCount[8];
};

// FileOptions: Index, None
struct SpellReagentsCurrencyEntry
{
    int32       ID;
    int32       SpellID;
    uint16      CurrencyTypesID;
    uint16      CurrencyCount;
};

// FileOptions: Index, None
struct SpellScalingEntry
{
    int32       ID;
    int32       SpellID;
    uint16      ScalesFromItemLevel;
    int32       Class;
    int32       MinScalingLevel;
    int32       MaxScalingLevel;
};

// FileOptions: Index, None
struct SpellShapeshiftEntry
{
    int32       ID;
    int32       SpellID;
    uint32      ShapeshiftExclude[2];
    uint32      ShapeshiftMask[2];
    int8        StanceBarOrder;
};

// FileOptions: Index, None
struct SpellShapeshiftFormEntry
{
    int32       ID;
    LocalizedString* Name;
    float       DamageVariance;
    int32       Flags;
    uint16      CombatRoundTime;
    uint16      MountTypeID;
    int8        CreatureType;
    uint8       BonusActionBar;
    int32       AttackIconFileID;
    int32       CreatureDisplayID[4];
    int32       PresetSpellID[8];
};

// FileOptions: Index, None
struct SpellSpecialUnitEffectEntry
{
    int32       ID;
    uint16      SpellVisualEffectNameID;
    int32       PositionerID;
};

// FileOptions: Index, None
struct SpellTargetRestrictionsEntry
{
    int32       ID;
    float       ConeDegrees;
    float       Width;
    int32       Targets;
    uint16      TargetCreatureType;
    uint8       DifficultyID;
    uint8       MaxTargets;
    int32       MaxTargetLevel;
    uint32      SpellID;
};

// FileOptions: Index, None
struct SpellTotemsEntry
{
    int32       ID;
    int32       SpellID;
    int32       Totem[2];
    uint16      RequiredTotemCategoryID[2];
};

// FileOptions: Index, None
struct SpellVisualEntry
{
    int32       ID;
    float       MissileCastOffset[3];
    float       MissileImpactOffset[3];
    int32       Flags;
    uint16      SpellVisualMissileSetID;
    uint8       MissileDestinationAttachment;
    uint8       MissileAttachment;
    int32       MissileCastPositionerID;
    int32       MissileImpactPositionerID;
    int32       MissileTargetingKit;
    int32       AnimEventSoundID;
    uint16      DamageNumberDelay;
    int32       HostileSpellVisualID;
    int32       CasterSpellVisualID;
    int32       LowViolenceSpellVisualID;
};

// FileOptions: Index, None
struct SpellVisualAnimEntry
{
    int32       ID;
    uint16      InitialAnimID;
    uint16      LoopAnimID;
    uint16      AnimKitID;
};

// FileOptions: Index, None
struct SpellVisualColorEffectEntry
{
    int32       ID;
    float       Duration;
    uint32      Color;
    float       ColorMultiplier;
    uint16      RedCurveID;
    uint16      GreenCurveID;
    uint16      BlueCurveID;
    uint16      AlphaCurveID;
    uint16      OpacityCurveID;
    uint8       Flags;
    uint8       Type;
    int32       PositionerID;
};

// FileOptions: Index, None
struct SpellVisualEffectNameEntry
{
    int32       ID;
    int32       ModelFileDataID;
    float       EffectRadius;
    float       BaseMissileSpeed;
    float       Scale;
    float       MinAllowedScale;
    float       MaxAllowedScale;
    float       Alpha;
    int32       Flags;
    int32       Type;
    int32       GenericID;
    int32       TextureFileDataID;
    int32       RibbonQualityID;
    int32       DissolveEffectID;
    int32       Unknown13;
};

// FileOptions: Index, None
struct SpellVisualEventEntry
{
    int32       ID;
    int32       StartEvent;
    int32       StartMinOffsetMs;
    int32       StartMaxOffsetMs;
    int32       EndEvent;
    int32       EndMinOffsetMs;
    int32       EndMaxOffsetMs;
    int32       TargetType;
    int32       SpellVisualKitID;
};

// FileOptions: Index, None
struct SpellVisualKitEntry
{
    int32       ID;
    int32       Flags;
    float       FallbackPriority;
    int32       FallbackSpellVisualKitID;
    uint16      DelayMin;
    uint16      DelayMax;
};

// FileOptions: Index, None
struct SpellVisualKitAreaModelEntry
{
    int32       ID;
    int32       ModelFileDataID;
    float       EmissionRate;
    float       Spacing;
    float       ModelScale;
    uint16      LifeTime;
    uint8       Flags;
};

// FileOptions: Index, None
struct SpellVisualKitEffectEntry
{
    int32       ID;
    int32       EffectType;
    int32       Effect;
};

// FileOptions: None
struct SpellVisualKitModelAttachEntry
{
    float       Offset[3];
    float       OffsetVariation[3];
    int32       ID;
    uint16      SpellVisualEffectNameID;
    uint8       AttachmentID;
    uint8       Flags;
    uint16      PositionerID;
    float       Yaw;
    float       Pitch;
    float       Roll;
    float       YawVariation;
    float       PitchVariation;
    float       RollVariation;
    float       Scale;
    float       ScaleVariation;
    uint16      StartAnimID;
    uint16      AnimID;
    uint16      EndAnimID;
    uint16      AnimKitID;
    int32       LowDefModelAttachID;
    float       StartDelay;
};

// FileOptions: None
struct SpellVisualMissileEntry
{
    uint32      FollowGroundHeight;
    int32       FollowGroundDropSpeed;
    int32       Flags;
    float       CastOffset[3];
    float       ImpactOffset[3];
    uint16      SpellVisualEffectNameID;
    uint16      CastPositionerID;
    uint16      ImpactPositionerID;
    uint16      FollowGroundApproach;
    uint16      SpellMissileMotionID;
    uint8       Attachment;
    uint8       DestinationAttachment;
    int32       ID;
    int32       SoundEntriesID;
    int32       AnimKitID;
};

// FileOptions: Index, None
struct SpellXDescriptionVariablesEntry
{
    int32       ID;
    int32       SpellID;
    int32       SpellDescriptionVariablesID;
};

// FileOptions: None
struct SpellXSpellVisualEntry
{
    int32       SpellVisualID;
    int32       ID;
    float       Probability;
    uint16      CasterPlayerConditionID;
    uint16      CasterUnitConditionID;
    uint16      ViewerPlayerConditionID;
    uint16      ViewerUnitConditionID;
    int32       SpellIconFileID;
    int32       ActiveIconFileID;
    uint8       Flags;
    uint8       DifficultyID;
    uint8       Priority;
    uint32      SpellID;
};

// FileOptions: Index, None
struct StartupFilesEntry
{
    int32       ID;
    int32       FileDataID;
    int32       Locale;
    int32       BytesRequired;
};

// FileOptions: Index, None
struct Startup_StringsEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Message;
};

// FileOptions: Index, None
struct StationeryEntry
{
    int32       ID;
    uint8       Flags;
    int32       ItemID;
    int32       TextureFileDataID[2];
};

// FileOptions: Index, None
struct SummonPropertiesEntry
{
    int32       ID;
    uint32      Flags;
    int32       Control;
    int32       Faction;
    int32       Title;
    int32       Slot;
};

// FileOptions: Index, None
struct TactKeyEntry
{
    int32       ID;
    uint8       Key[16];
};

// FileOptions: Index, None
struct TactKeyLookupEntry
{
    int32       ID;
    uint8       TACTID[8];
};

// FileOptions: Index, None
struct TalentEntry
{
    int32       ID;
    LocalizedString* Description;
    int32       SpellID;
    int32       OverridesSpellID;
    uint16      SpecID;
    uint8       TierID;
    uint8       ColumnIndex;
    uint8       Flags;
    uint8       CategoryMask[2];
    uint8       ClassID;
};

// FileOptions: Index, None
struct TaxiNodesEntry
{
    uint32      ID;
    LocalizedString* Name;
    DBCPosition3D Pos;
    int32       MountCreatureID[2];
    float       MapOffset[2];
    float       Facing;
    float       FlightMapOffset[2];
    uint16      ContinentID;
    uint16      ConditionID;
    uint16      CharacterBitNumber;
    uint8       Flags;
    int32       UiTextureKitID;
    int32       SpecialIconConditionID;
};

// FileOptions: None
struct TaxiPathEntry
{
    uint16      FromTaxiNode;
    uint16      ToTaxiNode;
    int32       ID;
    int32       Cost;
};

// FileOptions: None
struct TaxiPathNodeEntry
{
    DBCPosition3D   Loc;
    uint16      PathID;
    uint16      ContinentID;
    uint8       NodeIndex;
    int32       ID;
    uint8       Flags;
    int32       Delay;
    uint16      ArrivalEventID;
    uint16      DepartureEventID;
};

// FileOptions: Index, None
struct TerrainMaterialEntry
{
    int32       ID;
    uint8       Shader;
    int32       EnvMapDiffuseFileID;
    int32       EnvMapSpecularFileID;
};

// FileOptions: Index, None
struct TerrainTypeEntry
{
    int32       ID;
    LocalizedString* TerrainDesc;
    uint16      FootstepSprayRun;
    uint16      FootstepSprayWalk;
    uint8       SoundID;
    uint8       Flags;
};

// FileOptions: Index, None
struct TerrainTypeSoundsEntry
{
    int32       ID;
    LocalizedString* Name;
};

// FileOptions: Index, None
struct TextureBlendSetEntry
{
    int32       ID;
    int32       TextureFileDataID[3];
    float       TextureScrollRateU[3];
    float       TextureScrollRateV[3];
    float       TextureScaleU[3];
    float       TextureScaleV[3];
    float       ModX[4];
    uint8       SwizzleRed;
    uint8       SwizzleGreen;
    uint8       SwizzleBlue;
    uint8       SwizzleAlpha;
};

// FileOptions: None
struct TextureFileDataEntry
{
    int32       ID;
    int32       MaterialResourcesID;
    uint8       UsageType;
};

// FileOptions: Index, None
struct TotemCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
    uint32      TotemCategoryMask;
    uint8       TotemCategoryType;
};

// FileOptions: None
struct ToyEntry
{
    LocalizedString* SourceText;
    int32       ItemID;
    uint8       Flags;
    uint8       SourceTypeEnum;
    int32       ID;
};

// FileOptions: Index, None
struct TradeSkillCategoryEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* HordeName;
    uint16      SkillLineID;
    uint16      ParentTradeSkillCategoryID;
    uint16      OrderIndex;
    uint8       Flags;
};

// FileOptions: Index, None
struct TradeSkillItemEntry
{
    int32       ID;
    uint16      ItemLevel;
    uint8       RequiredLevel;
};

// FileOptions: Index, None
struct TransformMatrixEntry
{
    int32       ID;
    float       Pos[3];
    float       Yaw;
    float       Pitch;
    float       Roll;
    float       Scale;
};

// FileOptions: None
struct TransmogHolidayEntry
{
    int32       ID;
    int32       RequiredTransmogHoliday;
};

// FileOptions: None
struct TransmogSetEntry
{
    LocalizedString* Name;
    uint16      ParentTransmogSetID;
    uint16      UiOrder;
    uint8       ExpansionID;
    int32       ID;
    int32       Flags;
    int32       TrackingQuestID;
    int32       ClassMask;
    int32       ItemNameDescriptionID;
    int32       TransmogSetGroupID;
};

// FileOptions: None
struct TransmogSetGroupEntry
{
    LocalizedString* Name;
    int32       ID;
};

// FileOptions: None
struct TransmogSetItemEntry
{
    int32       ID;
    int32       TransmogSetID;
    int32       ItemModifiedAppearanceID;
    int32       Flags;
};

// FileOptions: Index, None
struct TransportAnimationEntry
{
    int32       ID;
    int32       TimeIndex;
    DBCPosition3D Pos;
    uint8       SequenceID;
    uint32      TransportID;
};

// FileOptions: Index, None
struct TransportPhysicsEntry
{
    int32       ID;
    float       WaveAmp;
    float       WaveTimeScale;
    float       RollAmp;
    float       RollTimeScale;
    float       PitchAmp;
    float       PitchTimeScale;
    float       MaxBank;
    float       MaxBankTurnSpeed;
    float       SpeedDampThresh;
    float       SpeedDamp;
};

// FileOptions: Index, None
struct TransportRotationEntry
{
    int32       ID;
    int32       TimeIndex;
    float       X;
    float       Y;
    float       Z;
    float       W;
    uint32      TransportID;
};

// FileOptions: Index, None
struct TrophyEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      GameObjectDisplayInfoID;
    uint8       TrophyTypeID;
    int32       PlayerConditionID;
};

// FileOptions: Index, None
struct UiCameraEntry
{
    int32       ID;
    LocalizedString* Name;
    float       Pos[3];
    float       LookAt[3];
    float       Up[3];
    uint16      AnimFrame;
    uint8       UiCameraTypeID;
    uint8       AnimVariation;
    uint8       Flags;
    int32       AnimID;
};

// FileOptions: Index, None
struct UiCameraTypeEntry
{
    int32       ID;
    LocalizedString* Name;
    int32       Width;
    int32       Height;
};

// FileOptions: Index, None
struct UiCamFbackTransmogChrRaceEntry
{
    int32       ID;
    uint16      UiCameraID;
    uint8       ChrRaceID;
    uint8       Gender;
    uint8       InventoryType;
    uint8       Variation;
};

// FileOptions: Index, None
struct UiCamFbackTransmogWeaponEntry
{
    int32       ID;
    uint16      UiCameraID;
    uint8       ItemClass;
    uint8       ItemSubclass;
    uint8       InventoryType;
};

// FileOptions: Index, None
struct UIExpansionDisplayInfoEntry
{
    int32       ID;
    int32       ExpansionLogo;
    int32       ExpansionBanner;
    int32       ExpansionLevel;
};

// FileOptions: Index, None
struct UIExpansionDisplayInfoIconEntry
{
    int32       ID;
    LocalizedString* FeatureDescription;
    int32       ParentID;
    int32       FeatureIcon;
};

// FileOptions: None
struct UiMapPOIEntry
{
    int32       ContinentID;
    float       WorldLoc[3];
    int32       UiTextureAtlasMemberID;
    int32       Flags;
    uint16      PoiDataType;
    uint16      PoiData;
    int32       ID;
};

// FileOptions: Index, None
struct UiModelSceneEntry
{
    int32       ID;
    uint8       UiSystemType;
    uint8       Flags;
};

// FileOptions: None
struct UiModelSceneActorEntry
{
    LocalizedString* ScriptTag;
    float       Position[3];
    float       OrientationYaw;
    float       OrientationPitch;
    float       OrientationRoll;
    float       NormalizedScale;
    uint8       Flags;
    int32       ID;
    int32       UiModelSceneActorDisplayID;
};

// FileOptions: Index, None
struct UiModelSceneActorDisplayEntry
{
    int32       ID;
    float       AnimSpeed;
    float       Alpha;
    float       Scale;
    int32       AnimationID;
    int32       SequenceVariation;
};

// FileOptions: None
struct UiModelSceneCameraEntry
{
    LocalizedString* ScriptTag;
    float       Target[3];
    float       ZoomedTargetOffset[3];
    float       Yaw;
    float       Pitch;
    float       Roll;
    float       ZoomedYawOffset;
    float       ZoomedPitchOffset;
    float       ZoomedRollOffset;
    float       ZoomDistance;
    float       MinZoomDistance;
    float       MaxZoomDistance;
    uint8       Flags;
    uint8       CameraType;
    int32       ID;
};

// FileOptions: Index, None
struct UiTextureAtlasEntry
{
    int32       ID;
    int32       FileDataID;
    uint16      AtlasHeight;
    uint16      AtlasWidth;
};

// FileOptions: None
struct UiTextureAtlasMemberEntry
{
    LocalizedString* CommittedName;
    int32       ID;
    uint16      UiTextureAtlasID;
    uint16      CommittedLeft;
    uint16      CommittedRight;
    uint16      CommittedTop;
    uint16      CommittedBottom;
    uint8       CommittedFlags;
};

// FileOptions: Index, None
struct UiTextureKitEntry
{
    int32       ID;
    LocalizedString* KitPrefix;
};

// FileOptions: Index, None
struct UnitBloodEntry
{
    int32       ID;
    int32       PlayerCritBloodSpurtID;
    int32       PlayerHitBloodSpurtID;
    int32       DefaultBloodSpurtID;
    int32       PlayerOmniCritBloodSpurtID;
    int32       PlayerOmniHitBloodSpurtID;
    int32       DefaultOmniBloodSpurtID;
};

// FileOptions: Index, None
struct UnitBloodLevelsEntry
{
    int32       ID;
    uint8       Violencelevel[3];
};

// FileOptions: Index, None
struct UnitConditionEntry
{
    int32       ID;
    int32       Value[8];
    uint8       Flags;
    uint8       Variable[8];
    uint8       Op[8];
};

// FileOptions: Index, None
struct UnitPowerBarEntry
{
    int32       ID;
    LocalizedString* Name;
    LocalizedString* Cost;
    LocalizedString* OutOfError;
    LocalizedString* ToolTip;
    float       RegenerationPeace;
    float       RegenerationCombat;
    int32       FileDataID[6];
    uint32      Color[6];
    float       StartInset;
    float       EndInset;
    uint16      StartPower;
    uint16      Flags;
    uint8       CenterPower;
    uint8       BarType;
    int32       MinPower;
    int32       MaxPower;
};

// FileOptions: Index, None
struct VehicleEntry
{
    int32       ID;
    uint32      Flags;
    float       TurnSpeed;
    float       PitchSpeed;
    float       PitchMin;
    float       PitchMax;
    float       MouseLookOffsetPitch;
    float       CameraFadeDistScalarMin;
    float       CameraFadeDistScalarMax;
    float       CameraPitchOffset;
    float       FacingLimitRight;
    float       FacingLimitLeft;
    float       CameraYawOffset;
    uint16      SeatID[8];
    uint16      VehicleUIIndicatorID;
    uint16      PowerDisplayID[3];
    uint8       FlagsB;
    uint8       UiLocomotionType;
    int32       MissileTargetingID;
};

// FileOptions: Index, None
struct VehicleSeatEntry
{
    int32       ID;
    uint32      Flags;
    uint32      FlagsB;
    uint32      FlagsC;
    DBCPosition3D AttachmentOffset;
    float       EnterPreDelay;
    float       EnterSpeed;
    float       EnterGravity;
    float       EnterMinDuration;
    float       EnterMaxDuration;
    float       EnterMinArcHeight;
    float       EnterMaxArcHeight;
    float       ExitPreDelay;
    float       ExitSpeed;
    float       ExitGravity;
    float       ExitMinDuration;
    float       ExitMaxDuration;
    float       ExitMinArcHeight;
    float       ExitMaxArcHeight;
    float       PassengerYaw;
    float       PassengerPitch;
    float       PassengerRoll;
    float       VehicleEnterAnimDelay;
    float       VehicleExitAnimDelay;
    float       CameraEnteringDelay;
    float       CameraEnteringDuration;
    float       CameraExitingDelay;
    float       CameraExitingDuration;
    DBCPosition3D CameraOffset;
    float       CameraPosChaseRate;
    float       CameraFacingChaseRate;
    float       CameraEnteringZoom;
    float       CameraSeatZoomMin;
    float       CameraSeatZoomMax;
    int32       UiSkinFileDataID;
    uint16      EnterAnimStart;
    uint16      EnterAnimLoop;
    uint16      RideAnimStart;
    uint16      RideAnimLoop;
    uint16      RideUpperAnimStart;
    uint16      RideUpperAnimLoop;
    uint16      ExitAnimStart;
    uint16      ExitAnimLoop;
    uint16      ExitAnimEnd;
    uint16      VehicleEnterAnim;
    uint16      VehicleExitAnim;
    uint16      VehicleRideAnimLoop;
    uint16      EnterAnimKitID;
    uint16      RideAnimKitID;
    uint16      ExitAnimKitID;
    uint16      VehicleEnterAnimKitID;
    uint16      VehicleRideAnimKitID;
    uint16      VehicleExitAnimKitID;
    uint16      CameraModeID;
    uint8       AttachmentID;
    uint8       PassengerAttachmentID;
    uint8       VehicleEnterAnimBone;
    uint8       VehicleExitAnimBone;
    uint8       VehicleRideAnimLoopBone;
    uint8       VehicleAbilityDisplay;
    int32       EnterUISoundID;
    int32       ExitUISoundID;

    bool CanEnterOrExit() const;
    bool CanSwitchFromSeat() const;
    bool IsUsableByOverride() const;
    bool IsEjectable() const;
};

// FileOptions: Index, None
struct VehicleUIIndicatorEntry
{
    int32       ID;
    int32       BackgroundTextureFileID;
};

// FileOptions: Index, None
struct VehicleUIIndSeatEntry
{
    int32       ID;
    float       XPos;
    float       YPos;
    uint8       VirtualSeatIndex;
};

// FileOptions: Index, None
struct VignetteEntry
{
    int32       ID;
    LocalizedString* Name;
    float       MaxHeight;
    float       MinHeight;
    int32       QuestFeedbackEffectID;
    int32       Flags;
    int32       PlayerConditionID;
    int32       VisibleTrackingQuestID;
};

// FileOptions: Index, None
struct VirtualAttachmentEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      PositionerID;
};

// FileOptions: Index, None
struct VirtualAttachmentCustomizationEntry
{
    int32       ID;
    int32       FileDataID;
    uint16      VirtualAttachmentID;
    uint16      PositionerID;
};

// FileOptions: Index, None
struct VocalUISoundsEntry
{
    int32       ID;
    uint8       VocalUIEnum;
    uint8       RaceID;
    uint8       ClassID;
    int32       NormalSoundID[2];
};

// FileOptions: Index, None
struct WbAccessControlListEntry
{
    int32       ID;
    LocalizedString* URL;
    uint16      GrantFlags;
    uint8       RevokeFlags;
    uint8       WowEditInternal;
    uint8       RegionID;
};

// FileOptions: Index, None
struct WbCertWhitelistEntry
{
    int32       ID;
    LocalizedString* Domain;
    uint8       GrantAccess;
    uint8       RevokeAccess;
    uint8       WowEditInternal;
};

// FileOptions: Index, None
struct WeaponImpactSoundsEntry
{
    int32       ID;
    uint8       WeaponSubClassID;
    uint8       ParrySoundType;
    uint8       ImpactSource;
    int32       ImpactSoundID[11];
    int32       CritImpactSoundID[11];
    int32       PierceImpactSoundID[11];
    int32       PierceCritImpactSoundID[11];
};

// FileOptions: Index, None
struct WeaponSwingSounds2Entry
{
    int32       ID;
    uint8       SwingType;
    uint8       Crit;
    int32       SoundID;
};

// FileOptions: Index, None
struct WeaponTrailEntry
{
    int32       ID;
    int32       FileDataID;
    float       Yaw;
    float       Pitch;
    float       Roll;
    int32       TextureFileDataID[3];
    float       TextureScrollRateU[3];
    float       TextureScrollRateV[3];
    float       TextureScaleU[3];
    float       TextureScaleV[3];
};

// FileOptions: Index, None
struct WeaponTrailModelDefEntry
{
    int32       ID;
    int32       LowDefFileDataID;
    uint16      WeaponTrailID;
};

// FileOptions: Index, None
struct WeaponTrailParamEntry
{
    int32       ID;
    float       Duration;
    float       FadeOutTime;
    float       EdgeLifeSpan;
    float       InitialDelay;
    float       SmoothSampleAngle;
    uint8       Hand;
    uint8       OverrideAttachTop;
    uint8       OverrideAttachBot;
    uint8       Flags;
};

// FileOptions: Index, None
struct WeatherEntry
{
    int32       ID;
    float       Intensity[2];
    float       TransitionSkyBox;
    float       EffectColor[3];
    float       Scale;
    float       Volatility;
    float       TwinkleIntensity;
    float       FallModifier;
    float       RotationalSpeed;
    int32       ParticulateFileDataID;
    uint16      SoundAmbienceID;
    uint8       Type;
    uint8       EffectType;
    uint8       WindSettingsID;
    int32       AmbienceID;
    int32       EffectTextureFileDataID;
};

// FileOptions: Index, None
struct WindSettingsEntry
{
    int32       ID;
    float       BaseMag;
    float       BaseDir[3];
    float       VarianceMagOver;
    float       VarianceMagUnder;
    float       VarianceDir[3];
    float       MaxStepMag;
    float       MaxStepDir[3];
    float       Frequency;
    float       Duration;
    uint8       Flags;
};

// FileOptions: None
struct WMOAreaTableEntry
{
    LocalizedString* AreaName;
    int32       WmoGroupID;
    uint16      AmbienceID;
    uint16      ZoneMusic;
    uint16      IntroSound;
    uint16      AreaTableID;
    uint16      UwIntroSound;
    uint16      UwAmbience;
    uint8       NameSetID;
    uint8       SoundProviderPref;
    uint8       SoundProviderPrefUnderwater;
    uint8       Flags;
    int32       ID;
    int32       UwZoneMusic;
    uint16      WmoID;
};

// FileOptions: DataOffset, Unknown, Index, None
struct WMOMinimapTextureEntry
{
    int32       ID;
    int32       FileDataID;
    uint16      GroupNum;
    uint8       BlockX;
    uint8       BlockY;
};

// FileOptions: Index, None
struct WorldBossLockoutEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      TrackingQuestID;
};

// FileOptions: Index, None
struct WorldChunkSoundsEntry
{
    int32       ID;
    uint16      MapID;
    uint8       ChunkX;
    uint8       ChunkY;
    uint8       SubChunkX;
    uint8       SubChunkY;
    uint8       SoundOverrideID;
};

// FileOptions: Index, None
struct WorldEffectEntry
{
    int32       ID;
    uint32      TargetAsset;
    uint16      CombatConditionID;
    uint8       TargetType;
    uint8       WhenToDisplay;
    int32       QuestFeedbackEffectID;
    int32       PlayerConditionID;
};

// FileOptions: Index, None
struct WorldElapsedTimerEntry
{
    int32       ID;
    LocalizedString* Name;
    uint8       Flags;
    uint8       Type;
};

// FileOptions: None
struct WorldMapAreaEntry
{
    LocalizedString* AreaName;
    float       LocLeft;
    float       LocRight;
    float       LocTop;
    float       LocBottom;
    int32       Flags;
    int16       MapID;
    uint16      AreaID;
    int16       DisplayMapID;
    int16       DefaultDungeonFloor;
    int16       ParentWorldMapID;
    uint8       LevelRangeMin;
    uint8       LevelRangeMax;
    uint8       BountySetID;
    uint8       BountyDisplayLocation;
    int32       ID;
    int32       VisibilityPlayerConditionID;
};

// FileOptions: Index, None
struct WorldMapContinentEntry
{
    int32       ID;
    float       ContinentOffset[2];
    float       Scale;
    float       TaxiMin[2];
    float       TaxiMax[2];
    uint16      MapID;
    uint16      WorldMapID;
    uint8       LeftBoundary;
    uint8       RightBoundary;
    uint8       TopBoundary;
    uint8       BottomBoundary;
    uint8       Flags;
};

// FileOptions: None
struct WorldMapOverlayEntry
{
    LocalizedString* TextureName;
    int32       ID;
    uint16      TextureWidth;
    uint16      TextureHeight;
    int32       MapAreaID;
    int32       OffsetX;
    int32       OffsetY;
    int32       HitRectTop;
    int32       HitRectLeft;
    int32       HitRectBottom;
    int32       HitRectRight;
    int32       PlayerConditionID;
    int32       Flags;
    int32       AreaID[4];
};

// FileOptions: Index, None
struct WorldMapTransformsEntry
{
    int32       ID;
    DBCPosition3D RegionMin;
    DBCPosition3D RegionMax;
    DBCPosition2D RegionOffset;
    float       RegionScale;
    uint16      MapID;
    uint16      AreaID;
    uint16      NewMapID;
    uint16      NewDungeonMapID;
    uint16      NewAreaID;
    uint8       Flags;
    int32       Priority;
};

// FileOptions: Index
struct WorldSafeLocsEntry
{
    int32       ID;
    LocalizedString* Name;
    DBCPosition4D Loc;
    uint16      MapID;
};

// FileOptions: Index, None
struct WorldStateExpressionEntry
{
    int32       ID;
    char const* Expression;

    bool Eval(Player const* player, std::set<WorldQuestState>* state = nullptr) const;
};

// FileOptions: None
struct WorldStateUIEntry
{
    LocalizedString* Icon;
    LocalizedString* ExtendedUI;
    LocalizedString* DynamicTooltip;
    LocalizedString* String;
    LocalizedString* Tooltip;
    uint16      MapID;
    uint16      AreaID;
    uint16      PhaseID;
    uint16      PhaseGroupID;
    uint16      StateVariable;
    uint16      ExtendedUIStateVariable[3];
    uint8       OrderIndex;
    uint8       PhaseUseFlags;
    uint8       Type;
    int32       ID;
    int32       DynamicIconFileID;
    int32       DynamicFlashIconFileID;
};

// FileOptions: Index, None
struct WorldStateZoneSoundsEntry
{
    int32       ID;
    int32       WmoAreaID;
    uint16      WorldStateID;
    uint16      WorldStateValue;
    uint16      AreaID;
    uint16      ZoneIntroMusicID;
    uint16      ZoneMusicID;
    uint16      SoundAmbienceID;
    uint8       SoundProviderPreferencesID;
};

// FileOptions: Index, None
struct World_PVP_AreaEntry
{
    int32       ID;
    uint16      AreaID;
    uint16      NextTimeWorldstate;
    uint16      GameTimeWorldstate;
    uint16      BattlePopulate_time;
    uint16      MapID;
    uint8       MinLevel;
    uint8       MaxLevel;
};

// FileOptions: Index, None
struct ZoneIntroMusicTableEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      MinDelayMinutes;
    uint8       Priority;
    int32       SoundID;
};

// FileOptions: Index, None
struct ZoneLightEntry
{
    int32       ID;
    LocalizedString* Name;
    uint16      MapID;
    uint16      LightID;
    uint8       Flags;
};

// FileOptions: Index, None
struct ZoneLightPointEntry
{
    int32       ID;
    float       Pos[2];
    uint8       PointOrder;
};

// FileOptions: Index, None
struct ZoneMusicEntry
{
    int32       ID;
    LocalizedString* SetName;
    int32       SilenceIntervalMin[2];
    int32       SilenceIntervalMax[2];
    int32       Sounds[2];
};

// FileOptions: Index, None
struct ZoneStoryEntry
{
    int32       ID;
    int32       DisplayAchievementID;
    int32       DisplayWorldMapAreaID;
    uint8       PlayerFactionGroupID;
};

#pragma pack(pop)

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination();
    TaxiPathBySourceAndDestination(uint32 _id, uint32 _price);

    uint32    ID;
    uint32    price;
};

typedef std::map<uint32, TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32, TaxiPathSetForSource> TaxiPathSetBySource;

typedef std::vector<TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

#define TaxiMaskSize 258
typedef std::array<uint8, TaxiMaskSize> TaxiMask;

#endif