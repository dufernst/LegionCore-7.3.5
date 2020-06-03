/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#ifndef GarrisonPackets_h__
#define GarrisonPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Garrison
    {
        struct GarrisonPlotInfo
        {
            TaggedPosition<Position::XYZO> PlotPos;
            uint32 GarrPlotInstanceID = 0;
            uint32 PlotType = 0;
        };

        struct GarrisonBuildingInfo
        {
            time_t TimeBuilt = time_t(0);
            time_t TimeSpecCooldown = time_t(2288912640);   // 06/07/1906 18:35:44 - another in the series of magic blizz dates
            uint32 GarrPlotInstanceID = 0;
            uint32 GarrBuildingID = 0;
            uint32 CurrentGarSpecID = 0;
            bool Active = false;
        };

        struct GarrisonFollower
        {
            std::vector<uint32> AbilityID;
            uint64 DbID = 0;
            uint32 GarrFollowerID = 0;
            uint32 Quality = 0;
            uint32 FollowerLevel = 0;
            uint32 ItemLevelWeapon = 0;
            uint32 ItemLevelArmor = 0;
            uint32 Xp = 0;
            uint32 Vitality = 0;
            uint32 CurrentBuildingID = 0;
            uint32 CurrentMissionID = 0;
            uint32 ZoneSupportSpellID = 0;
            uint32 FollowerStatus = 0;
            std::string CustomName;
        };

        struct GarrisonMission
        {
            uint64 DbID = 0;
            time_t StartTime = time_t(0);
            time_t OfferTime = time_t(0);
            uint32 RecID = 0;
            uint32 OfferDuration = 0;
            uint32 TravelDuration = 0;
            uint32 Duration = 0;
            uint32 State = 0; // MS::Garrison::Mission::State
            uint32 SuccesChance = 0;
            uint32 UnkInt2 = 0;
        };

        struct Shipment
        {
            Shipment() = default;
            uint64 FollowerDBID = 0;
            uint64 ShipmentID = 0;
            uint32 ShipmentRecID = 0;
            uint32 BuildingTypeID = 0;
            time_t CreationTime = time(nullptr);
            int32 ShipmentDuration = 0;

            bool finished = false;
            ObjectDBState DbState = DB_STATE_NEW;
            uint32 end = 0;
        };

        struct GarrisonMissionReward
        {
            int32 ItemID = 0;
            uint32 ItemQuantity = 0;
            int32 ItemFileDataID = 0;
            int32 CurrencyID = 0;
            uint32 CurrencyQuantity = 0;
            uint32 FollowerXP = 0;
            uint32 BonusAbilityID = 0;
        };

        struct GarrisonMissionAreaBonus
        {
            uint32 GarrMssnBonusAbilityID = 0;
            time_t StartTime = time_t(0);
        };

        struct GarrisonTalent
        {
            int32 GarrTalentID = 0;
            time_t ResearchStartTime = time_t(0);
            int32 Flags = 0;
            ObjectDBState DbState = DB_STATE_UNCHANGED;
        };

        struct GarrisonInfo
        {
            std::vector<std::vector<GarrisonMissionReward>> MissionRewards;
            std::vector<std::vector<GarrisonMissionReward>> MissionOvermaxRewards;
            std::vector<GarrisonPlotInfo*> Plots;
            std::vector<GarrisonBuildingInfo const*> Buildings;
            std::vector<GarrisonFollower const*> Followers;
            std::vector<GarrisonMission const*> Missions;
            std::vector<GarrisonMissionAreaBonus const*> MissionAreaBonuses;
            std::list<GarrisonTalent> Talents;
            std::vector<int32> ArchivedMissions;
            std::vector<bool> CanStartMission;
            uint32 GarrTypeID = 0;
            uint32 GarrSiteID = 0;
            uint32 GarrSiteLevelID = 0;
            uint32 NumFollowerActivationsRemaining = 0;
            uint32 NumMissionsStartedToday = 0;   // might mean something else, but sending 0 here enables follower abilities "Increase success chance of the first mission of the day by %."
        };

        struct GarrisonRemoteBuildingInfo
        {
            GarrisonRemoteBuildingInfo();
            GarrisonRemoteBuildingInfo(uint32 plotInstanceId, uint32 buildingId);

            uint32 GarrPlotInstanceID;
            uint32 GarrBuildingID;
        };

        struct GarrisonRemoteSiteInfo
        {
            std::vector<GarrisonRemoteBuildingInfo> Buildings;
            uint32 GarrSiteLevelID = 0;
        };

        struct GarrisonBuildingLandmark
        {
            GarrisonBuildingLandmark();
            GarrisonBuildingLandmark(uint32 buildingPlotInstId, Position const& pos);

            TaggedPosition<Position::XYZ> Pos;
            uint32 GarrBuildingPlotInstID = 0;
        };

        struct GarrMissionFollowerData
        {
            uint64 FollowerDbID = 0;
            uint32 unk32 = 0;
        };

        struct GarrTradeSkill
        {
            std::vector<uint32> SkillLineIDs;
            std::vector<uint32> SkillRanks;
            std::vector<uint32> SkillMaxRanks;
            std::vector<uint32> KnownAbilitySpellIDs;
            uint32 SpellID = 0;
        };

        struct FollowersClassSpecInfo
        {
            uint32 Category = 0;
            uint32 Option = 0;
        };

        struct TrophyListDisplayInfo
        {
            uint32 Unk1 = 0;
            uint32 Unk2 = 0;
        };

        struct TrophyListInfo
        {
            uint32 TrophyID = 0;
            uint32 Unk1 = 0;
            uint32 Unk2 = 0;
        };

        class GarrisonCreateResult final : public ServerPacket
        {
        public:
            GarrisonCreateResult() : ServerPacket(SMSG_GARRISON_CREATE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 GarrSiteLevelID = 0;
            uint32 Result = 0;
        };

        class GarrisonDeleteResult final : public ServerPacket
        {
        public:
            GarrisonDeleteResult() : ServerPacket(SMSG_GARRISON_DELETE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            uint32 GarrSiteID = 0;
        };

        class GetGarrisonInfo final : public ClientPacket
        {
        public:
            GetGarrisonInfo(WorldPacket&& packet) : ClientPacket(CMSG_GET_GARRISON_INFO, std::move(packet)) { }

            void Read() override { }
        };

        struct FollowerSoftCapInfo
        {
            FollowerSoftCapInfo(int32 typeID, uint32 count);
            int32 GarrFollowerTypeID = 0;
            uint32 Count = 0;
        };

        class GetGarrisonInfoResult final : public ServerPacket
        {
        public:
            GetGarrisonInfoResult() : ServerPacket(SMSG_GET_GARRISON_INFO_RESULT, 12) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonInfo> Garrisons;
            std::vector<FollowerSoftCapInfo> FollowerSoftCaps;
            uint32 FactionIndex = 0;
        };

        class GarrisonMissionUpdate final : public ServerPacket
        {
        public:
            GarrisonMissionUpdate() : ServerPacket(SMSG_GARRISON_MISSION_UPDATE_CAN_START, 8) { }

            WorldPacket const* Write() override;

            std::vector<int32> ArchivedMissions;
            std::vector<bool> CanStartMission;
        };

        class GarrisonRemoteInfo final : public ServerPacket
        {
        public:
            GarrisonRemoteInfo() : ServerPacket(SMSG_GARRISON_REMOTE_INFO, 4) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonRemoteSiteInfo> Sites;
        };

        class GarrisonPurchaseBuilding final : public ClientPacket
        {
        public:
            GarrisonPurchaseBuilding(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_PURCHASE_BUILDING, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 BuildingID = 0;
            uint32 PlotInstanceID = 0;
        };

        class GarrisonPlaceBuildingResult final : public ServerPacket
        {
        public:
            GarrisonPlaceBuildingResult() : ServerPacket(SMSG_GARRISON_PLACE_BUILDING_RESULT, sizeof(GarrisonBuildingInfo) + 4 + 1 + 4) { }

            WorldPacket const* Write() override;

            GarrisonBuildingInfo BuildingInfo;
            uint32 Result = 0;
            uint32 GarrTypeID = 0;
            bool PlayActivationCinematic = false;
        };

        class GarrisonCancelConstruction final : public ClientPacket
        {
        public:
            GarrisonCancelConstruction(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_CANCEL_CONSTRUCTION, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 PlotInstanceID = 0;
        };

        class GarrisonBuildingRemoved final : public ServerPacket
        {
        public:
            GarrisonBuildingRemoved() : ServerPacket(SMSG_GARRISON_BUILDING_REMOVED, 4 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
            uint32 GarrPlotInstanceID = 0;
            uint32 GarrBuildingID = 0;
            uint32 GarrTypeID = 0;
        };

        class GarrisonLearnBlueprintResult final : public ServerPacket
        {
        public:
            GarrisonLearnBlueprintResult() : ServerPacket(SMSG_GARRISON_LEARN_BLUEPRINT_RESULT, 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 BuildingID = 0;
            uint32 GarrTypeID = 0;
            uint32 Result = 0;
        };

        class GarrisonUnlearnBlueprintResult final : public ServerPacket
        {
        public:
            GarrisonUnlearnBlueprintResult() : ServerPacket(SMSG_GARRISON_UNLEARN_BLUEPRINT_RESULT, 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 BuildingID = 0;
            uint32 Result = 0;
            uint32 GarrTypeID = 0;
        };

        class GarrisonRequestBlueprintAndSpecializationData final : public ClientPacket
        {
        public:
            GarrisonRequestBlueprintAndSpecializationData(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_BLUEPRINT_AND_SPECIALIZATION_DATA, std::move(packet)) { }

            void Read() override { }
        };

        class GarrisonRequestBlueprintAndSpecializationDataResult final : public ServerPacket
        {
        public:
            GarrisonRequestBlueprintAndSpecializationDataResult() : ServerPacket(SMSG_GARRISON_REQUEST_BLUEPRINT_AND_SPECIALIZATION_DATA_RESULT, 400) { }

            WorldPacket const* Write() override;

            std::unordered_set<uint32> const* SpecializationsKnown = nullptr;
            std::unordered_set<uint32> const* BlueprintsKnown = nullptr;
            uint32 GarrTypeID = 0;
        };

        class GarrisonGetBuildingLandmarks final : public ClientPacket
        {
        public:
            GarrisonGetBuildingLandmarks(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_GET_BUILDING_LANDMARKS, std::move(packet)) { }

            void Read() override { }
        };

        class GarrisonBuildingLandmarks final : public ServerPacket
        {
        public:
            GarrisonBuildingLandmarks() : ServerPacket(SMSG_GARRISON_BUILDING_LANDMARKS, 4) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonBuildingLandmark> Landmarks;
        };

        class GarrisonPlotPlaced final : public ServerPacket
        {
        public:
            GarrisonPlotPlaced() : ServerPacket(SMSG_GARRISON_PLOT_PLACED, sizeof(GarrisonPlotInfo) + 4) { }

            WorldPacket const* Write() override;

            GarrisonPlotInfo* PlotInfo = nullptr;
            uint32 GarrTypeID = 0;
        };

        class GarrisonPlotRemoved final : public ServerPacket
        {
        public:
            GarrisonPlotRemoved() : ServerPacket(SMSG_GARRISON_PLOT_REMOVED, 4) { }

            WorldPacket const* Write() override;

            uint32 GarrPlotInstanceID = 0;
        };

        class GarrisonAddFollowerResult final : public ServerPacket
        {
        public:
            GarrisonAddFollowerResult() : ServerPacket(SMSG_GARRISON_ADD_FOLLOWER_RESULT, sizeof(GarrisonFollower) + 8) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
            uint32 Result = 0;
            uint32 GarrTypeID = 0;
        };

        class GarrisonFollowerChangedItemLevel final : public ServerPacket
        {
        public:
            GarrisonFollowerChangedItemLevel() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_ITEM_LEVEL, sizeof(GarrisonFollower)) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
        };
        
        class GarrisonFollowerChangedDurability final : public ServerPacket
        {
        public:
            GarrisonFollowerChangedDurability() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_DURABILITY, sizeof(GarrisonFollower) + 4) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
            uint32 UnkInt = 0;
        };

        class GarrisonFollowerChangedAbilities final : public ServerPacket
        {
        public:
            GarrisonFollowerChangedAbilities() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_ABILITIES, sizeof(GarrisonFollower)) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
        };

        class GarrisonBuildingActivated final : public ServerPacket
        {
        public:
            GarrisonBuildingActivated() : ServerPacket(SMSG_GARRISON_BUILDING_ACTIVATED, 4) { }

            WorldPacket const* Write() override;

            uint32 GarrPlotInstanceID = 0;
        };

        class GarrisonRequestLandingPageShipmentInfo final : public ClientPacket
        {
        public:
            GarrisonRequestLandingPageShipmentInfo(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_LANDING_PAGE_SHIPMENT_INFO, std::move(packet)) { }

            void Read() override { }
        };

        class GarrisonCheckUpgradeable final : public ClientPacket
        {
        public:
            GarrisonCheckUpgradeable(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_CHECK_UPGRADEABLE, std::move(packet)) { }

            void Read() override;

            int32 GarrSiteID = 0;
        };

        class GarrisonMissionBonusRoll final : public ClientPacket
        {
        public:
            GarrisonMissionBonusRoll(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_MISSION_BONUS_ROLL, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 MissionRecID = 0;
        };

        class GarrisonMissionBonusRollResult final : public ServerPacket
        {
        public:
            GarrisonMissionBonusRollResult() : ServerPacket(SMSG_GARRISON_MISSION_BONUS_ROLL_RESULT, sizeof(GarrisonMission) + 4 + 4) { }

            WorldPacket const* Write() override;

            GarrisonMission MissionData;
            uint32 MissionRecID = 0;
            uint32 Result = 0;
        };

        class GarrisonStartMission final : public ClientPacket
        {
        public:
            GarrisonStartMission(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_START_MISSION, std::move(packet)) { }

            void Read() override;

            std::vector<uint64> FollowerDBIDs;
            ObjectGuid NpcGUID;
            uint32 MissionRecID = 0;
        };

        class GarrisonSwapBuildings final : public ClientPacket
        {
        public:
            GarrisonSwapBuildings(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_SWAP_BUILDINGS, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 PlotId1 = 0;
            uint32 PlotId2 = 0;
        };

        class GarrisonCompleteMission final : public ClientPacket
        {
        public:
            GarrisonCompleteMission(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_COMPLETE_MISSION, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 MissionRecID = 0;
        };

        class GarrisonAssignFollowerToBuildingResult final : public ServerPacket
        {
        public:
            GarrisonAssignFollowerToBuildingResult() : ServerPacket(SMSG_GARRISON_ASSIGN_FOLLOWER_TO_BUILDING_RESULT, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 FollowerDBID = 0;
            int32 Result = 0;
            int32 PlotInstanceID = 0;
        };

        class GarrisonLandingPage final : public ServerPacket
        {
        public:
            GarrisonLandingPage() : ServerPacket(SMSG_GARRISON_LANDING_PAGE_SHIPMENT_INFO, 4) { }

            WorldPacket const* Write() override;

            std::vector<Shipment> MsgData;
            uint32 Result = 0;
        };

        class GarrisonAddMissionResult final : public ServerPacket
        {
        public:
            GarrisonAddMissionResult() : ServerPacket(SMSG_GARRISON_ADD_MISSION_RESULT, 16 + 1 + 1 + sizeof(GarrisonMission)) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonMissionReward> Reward;
            std::vector<GarrisonMissionReward> BonusReward;
            GarrisonMission MissionData;
            uint32 Result = 0;
            uint32 GarrTypeID = 0;
            uint8 unk = 0;
            bool UnkBit = false;
        };

        class GarrisonUpgradeResult final : public ServerPacket
        {
        public:
            GarrisonUpgradeResult() : ServerPacket(SMSG_GARRISON_UPGRADE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 GarrSiteLevelID = 0;
            uint32 Result = 0;
        };

        class GarrisonStartMissionResult final : public ServerPacket
        {
        public:
            GarrisonStartMissionResult() : ServerPacket(SMSG_GARRISON_START_MISSION_RESULT, sizeof(GarrisonMission) + 4 + 4) { }

            WorldPacket const* Write() override;
            
            GarrisonMission MissionData;
            std::vector<uint64> FollowerDBIDs;
            uint32 Result = 0;
            uint16 Unk = 0;
        };

        class GarrisonCompleteMissionResult final : public ServerPacket
        {
        public:
            GarrisonCompleteMissionResult() : ServerPacket(SMSG_GARRISON_COMPLETE_MISSION_RESULT, sizeof(GarrisonMission) + 4 + 4 + 1) { }

            WorldPacket const* Write() override;
            
            GarrisonMission MissionData;
            uint32 Result = 0;
            uint32 MissionRecID = 0;
            bool Succeeded = false;
        };

        //!  SMSG_GARRISON_UNK_2  was. Possible SMSG_GARRISON_MISSION_REWARD_RESPONSE?
        class GarrisonCompleteMissionResultNew final : public ServerPacket
        {
        public:
            GarrisonCompleteMissionResultNew() : ServerPacket(SMSG_GARRISON_COMPLETE_MISSION_RESULT_NEW, sizeof(GarrisonMission) + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            std::vector<GarrMissionFollowerData> followerData;
            GarrisonMission MissionData;
            uint32 Result = 0;
            uint32 MissionRecID = 0;
            bool Succeeded = false;
        };

        class GarrisonIsUpgradeableResult final : public ServerPacket
        {
        public:
            GarrisonIsUpgradeableResult() : ServerPacket(SMSG_GARRISON_IS_UPGRADEABLE_RESULT, 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
        };

        class CreateShipment final : public ClientPacket
        {
        public:
            CreateShipment(WorldPacket&& packet) : ClientPacket(CMSG_CREATE_SHIPMENT, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint32 Count = 0;
        };

        class GarrisonRequestShipmentInfo final : public ClientPacket
        {
        public:
            GarrisonRequestShipmentInfo(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_SHIPMENT_INFO, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
        };

        class GarrisonRequestResearchTalent final : public ClientPacket
        {
        public:
            GarrisonRequestResearchTalent(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_RESEARCH_TALENT, std::move(packet)) { }

            void Read() override;

            uint32 TalentID = 0;
        };

        class GarrisonOpenMissionNpcRequest final : public ClientPacket
        {
        public:
            GarrisonOpenMissionNpcRequest(WorldPacket&& packet) : ClientPacket(CMSG_OPEN_MISSION_NPC, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            int32 GarrFollowerTypeID = 0;
        };

        class GetShipmentInfoResponse final : public ServerPacket
        {
        public:
            GetShipmentInfoResponse() : ServerPacket(SMSG_GET_SHIPMENT_INFO_RESPONSE, 1 + 4 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;
            
            std::vector<Shipment> Shipments;
            uint32 ShipmentID = 0;
            uint32 MaxShipments = 0;
            uint32 PlotInstanceID = 0;
            bool Success = false;
        };

        class CreateShipmentResponse final : public ServerPacket
        {
        public:
            CreateShipmentResponse() : ServerPacket(SMSG_CREATE_SHIPMENT_RESPONSE, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 ShipmentID = 0;
            uint32 ShipmentRecID = 0;
            uint32 Result = 0;
        };

        class OpenShipmentNPCFromGossip final : public ServerPacket
        {
        public:
            OpenShipmentNPCFromGossip() : ServerPacket(SMSG_OPEN_SHIPMENT_NPC_FROM_GOSSIP, 20) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGUID;
            uint32 CharShipmentContainerID = 0;
        };

        class SetupTrophy final : public ServerPacket
        {
        public:
            SetupTrophy() : ServerPacket(SMSG_GET_DISPLAYED_TROPHY_LIST_RESPONSE, 4) { }

            WorldPacket const* Write() override;

            std::vector<TrophyListDisplayInfo> Trophys;
        };

        class UpgradeGarrison final : public ClientPacket
        {
        public:
            UpgradeGarrison(WorldPacket&& packet) : ClientPacket(CMSG_UPGRADE_GARRISON, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
        };

        //< CMSG_REPLACE_TROPHY
        //< CMSG_CHANGE_MONUMENT_APPEARANCE
        class TrophyData final : public ClientPacket
        {
        public:
            TrophyData(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            ObjectGuid TrophyGUID;
            uint32 NewTrophyID = 0;
        };

        class RevertTrophy final : public ClientPacket
        {
        public:
            RevertTrophy(WorldPacket&& packet) : ClientPacket(CMSG_REVERT_MONUMENT_APPEARANCE, std::move(packet)) { }

            void Read() override;

            ObjectGuid TrophyGUID;
        };

        class GetTrophyList final : public ClientPacket
        {
        public:
            GetTrophyList(WorldPacket&& packet) : ClientPacket(CMSG_GET_TROPHY_LIST, std::move(packet)) { }

            void Read() override;

            uint32 TrophyTypeID = 0;
        };

        class GarrisonSetFollowerInactive final : public ClientPacket
        {
        public:
            GarrisonSetFollowerInactive(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_SET_FOLLOWER_INACTIVE, std::move(packet)) { }

            void Read() override;

            uint64 FollowerDBID = 0;
            bool Inactive = false;
        };

        class GarrisonRemoveFollowerFromBuilding final : public ClientPacket
        {
        public:
            GarrisonRemoveFollowerFromBuilding(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REMOVE_FOLLOWER_FROM_BUILDING, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint64 FollowerDBID = 0;
        };

        class GarrisonAssignFollowerToBuilding final : public ClientPacket
        {
        public:
            GarrisonAssignFollowerToBuilding(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_ASSIGN_FOLLOWER_TO_BUILDING, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGUID;
            uint64 FollowerDBID = 0;
            uint32 PlotInstanceID = 0;
        };

        class GetTrophyListResponse final : public ServerPacket
        {
        public:
            GetTrophyListResponse() : ServerPacket(SMSG_GET_TROPHY_LIST_RESPONSE, 1 + 4) { }

            WorldPacket const* Write() override;

            std::vector<TrophyListInfo> MsgData;
            bool Success = false;
        };

        class ReplaceTrophyResponse final : public ServerPacket
        {
        public:
            ReplaceTrophyResponse() : ServerPacket(SMSG_REPLACE_TROPHY_RESPONSE, 1) { }

            WorldPacket const* Write() override;

            bool Success = false;
        };

        class GarrisonOpenArchitect final : public ServerPacket
        {
        public:
            GarrisonOpenArchitect() : ServerPacket(SMSG_GARRISON_OPEN_ARCHITECT, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGUID;
        };

        class GarrisonOpenUpgradeNpcResponse final : public ServerPacket
        {
        public:
            GarrisonOpenUpgradeNpcResponse() : ServerPacket(SMSG_GARRISON_OPEN_TALENT_NPC, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGUID;
        };

        class GarrisonRemoveFollowerResult final : public ServerPacket
        {
        public:
            GarrisonRemoveFollowerResult() : ServerPacket(SMSG_GARRISON_REMOVE_FOLLOWER_RESULT, 8 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 FollowerDBID = 0;
            uint32 GarrTypeID = 0;
            uint32 Result = 0;
            uint32 Destroyed = 0;
        };

        class GarrisonOpenRecruitmentNpc final : public ServerPacket
        {
        public:
            GarrisonOpenRecruitmentNpc() : ServerPacket(SMSG_GARRISON_OPEN_RECRUITMENT_NPC, sizeof(GarrisonFollower) * 3 + 16 + 8 + 2) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonFollower> Followers;
            ObjectGuid Guid;
            uint32 GarrTypeID = 0;
            uint32 Result = 0;
            bool CanRecruitFollower = false;
            bool UnkBit = false;
        };

        class GarrisonRecruitmentFollowersGenerated final : public ServerPacket
        {
        public:
            GarrisonRecruitmentFollowersGenerated() : ServerPacket(SMSG_GARRISON_RECRUITMENT_FOLLOWERS_GENERATED, sizeof(GarrisonFollower) * 3 + 4) { }

            WorldPacket const* Write() override;

            std::vector<GarrisonFollower> Followers;
            uint32 Result = 0;
        };

        class GarrisonResearchTalent final : public ServerPacket
        {
        public:
            GarrisonResearchTalent() : ServerPacket(SMSG_GARRISON_RESEARCH_TALENT, 20) { }

            WorldPacket const* Write() override;

            int32 Result = 0;
            uint32 GarrTypeID = 0;
            uint32 TalentID = 0;
            uint32 ResearchTime = 0;
            uint32 Flags = 0;
        };

        class GarrisonFollowerChangedXP final : public ServerPacket
        {
        public:
            GarrisonFollowerChangedXP() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_XP, sizeof(GarrisonFollower) * 2 + 8) { }

            WorldPacket const* Write() override;
            
            GarrisonFollower Follower;
            GarrisonFollower Follower2;
            int32 Result = 0;
            uint32 TotalXp = 0;
        };

        class GarrisonFollowerChangeStatus final : public ServerPacket
        {
        public:
            GarrisonFollowerChangeStatus() : ServerPacket(SMSG_GARRISON_FOLLOWER_CHANGED_STATUS, sizeof(GarrisonFollower) + 4) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
            int32 Result = 0;
        };

        class GarrisonNumFollowerActivationsRemaining final : public ServerPacket
        {
        public:
            GarrisonNumFollowerActivationsRemaining() : ServerPacket(SMSG_GARRISON_NUM_FOLLOWER_ACTIVATIONS_REMAINING, 4 + 4) { }

            WorldPacket const* Write() override;

            int32 Amount = 0;
            uint32 UnkInt = 0;
        };

        class GarrisonRemoveFollowerFromBuildingResult final : public ServerPacket
        {
        public:
            GarrisonRemoveFollowerFromBuildingResult() : ServerPacket(SMSG_GARRISON_REMOVE_FOLLOWER_FROM_BUILDING_RESULT, 12) { }

            WorldPacket const* Write() override;

            uint64 FollowerDBID = 0;
            int32 Result = 0;
        };

        class QueryGarrisonCreatureNameResponse final : public ServerPacket
        {
        public:
            QueryGarrisonCreatureNameResponse() : ServerPacket(SMSG_QUERY_GARRISON_CREATURE_NAME_RESPONSE, 16 + 8 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGUID;
            uint64 InqueKey = 0;
            Optional<std::string> Name;
        };

        class GarrisonTradeSkillResponse final : public ServerPacket
        {
        public:
            GarrisonTradeSkillResponse() : ServerPacket(SMSG_GARRISON_OPEN_TRADESKILL_NPC, 16 + sizeof(GarrTradeSkill) + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid GUID;
            GarrTradeSkill TradeSkill;
            std::vector<uint32> PlayerConditionID;
        };

        class GarrisonResponseClassSpecCategoryInfo final : public ServerPacket
        {
        public:
            GarrisonResponseClassSpecCategoryInfo() : ServerPacket(SMSG_GARRISON_RESPONSE_CLASS_SPEC_CASTEGORY_INFO, 4 + 4) { }

            WorldPacket const* Write() override;

            int32 GarrFollowerTypeID = 0;
            std::vector<FollowersClassSpecInfo> Datas;
        };

        class GarrisonRequestClassSpecCategoryInfo final : public ClientPacket
        {
        public:
            GarrisonRequestClassSpecCategoryInfo(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_CLASS_SPEC_CATEGORY_INFO, std::move(packet)) { }

            void Read() override;

            int32 GarrFollowerTypeID = 0;
        };

        class GarrisonOpenMissionNpc final : public ServerPacket
        {
        public:
            GarrisonOpenMissionNpc() : ServerPacket(SMSG_GARRISON_OPEN_MISSION_NPC, 36 + 4 + 4) { }

            WorldPacket const* Write() override;

            std::vector<uint32> Missions;
            uint32 GarrTypeID = 0;
            uint32 Result = 0;
            bool UnkBit1 = false;
            bool PreventXmlOpenMissionEvent = false;
        };

        class GarrisonRequestMissionNpc final : public ServerPacket
        {
        public:
            GarrisonRequestMissionNpc() : ServerPacket(SMSG_GARRISON_REQUEST_MISSION_NPC, 36 + 4 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid NpcGuid;
            uint32 GarrFollowerTypeID = 0;
        };

        class GarrisonGenerateRecruits final : public ClientPacket
        {
        public:
            GarrisonGenerateRecruits(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_GENERATE_RECRUITS, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGuid;
            int32 MechanicTypeID = 0;
            int32 TraitID = 0;
        };

        class GarrisonRenameFollower final : public ClientPacket
        {
        public:
            GarrisonRenameFollower(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_RENAME_FOLLOWER, std::move(packet)) { }

            void Read() override;

            uint64 FollowerDBID = 0;
            std::string FollowerName;
        };

        class GarrisonSetRecruitmentPreferences final : public ClientPacket
        {
        public:
            GarrisonSetRecruitmentPreferences(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_SET_RECRUITMENT_PREFERENCES, std::move(packet)) { }

            void Read() override;

            ObjectGuid NpcGuid;
            int32 MechanicTypeID = 0;
            int32 TraitID = 0;
        };

        class GarrisonRecruitFollower final : public ClientPacket
        {
        public:
            GarrisonRecruitFollower(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_RECRUIT_FOLLOWER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
            int32 FollowerIndex = 0;
        };

        class GarrisonRecruitFollowerResult final : public ServerPacket
        {
        public:
            GarrisonRecruitFollowerResult() : ServerPacket(SMSG_GARRISON_RECRUIT_FOLLOWER_RESULT, 36 + 4 + 4) { }

            WorldPacket const* Write() override;

            GarrisonFollower Follower;
            int32 Result = 0;
        };

        class GarrisonRemoveFollower final : public ClientPacket
        {
        public:
            GarrisonRemoveFollower(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REMOVE_FOLLOWER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
            uint64 FollowerDBID = 0;
        };

        class GarrisonRequestScoutingMap final : public ClientPacket
        {
        public:
            GarrisonRequestScoutingMap(WorldPacket&& packet) : ClientPacket(CMSG_GARRISON_REQUEST_SCOUTING_MAP, std::move(packet)) { }

            void Read() override;

            uint32 ID = 0;
        };

        class GarrisonScoutingMapResult final : public ServerPacket
        {
        public:
            GarrisonScoutingMapResult() : ServerPacket(SMSG_GARRISON_SCOUTING_MAP_RESULT, 5) { }

            WorldPacket const* Write() override;

            uint32 ID = 0;
            bool Active = true;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrTradeSkill const& tradeSkill);

#endif // GarrisonPackets_h__
