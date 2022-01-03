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

#include "GarrisonPackets.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonPlotInfo const* plotInfo)
{
    data << uint32(plotInfo->GarrPlotInstanceID);
    data << plotInfo->PlotPos;
    data << uint32(plotInfo->PlotType);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonBuildingInfo const& buildingInfo)
{
    data << uint32(buildingInfo.GarrPlotInstanceID);
    data << uint32(buildingInfo.GarrBuildingID);
    data << uint32(buildingInfo.TimeBuilt);
    data << uint32(buildingInfo.CurrentGarSpecID);
    data << uint32(buildingInfo.TimeSpecCooldown);
    data.WriteBit(buildingInfo.Active);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonFollower const& follower)
{
    data << uint64(follower.DbID);
    data << uint32(follower.GarrFollowerID);
    data << uint32(follower.Quality);
    data << uint32(follower.FollowerLevel);
    data << uint32(follower.ItemLevelWeapon);
    data << uint32(follower.ItemLevelArmor);
    data << uint32(follower.Xp);
    data << uint32(follower.Vitality);
    data << uint32(follower.CurrentBuildingID);
    data << uint32(follower.CurrentMissionID);
    data << static_cast<uint32>(follower.AbilityID.size());
    data << uint32(follower.ZoneSupportSpellID);
    data << uint32(follower.FollowerStatus);
    for (auto ability : follower.AbilityID)
        data << uint32(ability);

    data.WriteString(follower.CustomName, 7);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonMission const& mission)
{
    data << uint64(mission.DbID);
    data << uint32(mission.RecID);
    data << uint32(mission.OfferTime);
    data << uint32(mission.OfferDuration);
    data << uint32(mission.StartTime);
    data << uint32(mission.TravelDuration);
    data << uint32(mission.Duration);
    data << uint32(mission.State);
    data << uint32(mission.SuccesChance);
    data << uint32(mission.UnkInt2);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonMissionReward const& missionRewardItem)
{
    data << int32(missionRewardItem.ItemID);
    data << uint32(missionRewardItem.ItemQuantity);
    data << int32(missionRewardItem.CurrencyID);
    data << uint32(missionRewardItem.CurrencyQuantity);
    data << uint32(missionRewardItem.FollowerXP);
    data << uint32(missionRewardItem.BonusAbilityID);
    data << int32(missionRewardItem.ItemFileDataID);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonMissionAreaBonus const& areaBonus)
{
    data << uint32(areaBonus.GarrMssnBonusAbilityID);
    data << uint32(areaBonus.StartTime);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonTalent const& talent)
{
    data << int32(talent.GarrTalentID);
    data << int32(talent.ResearchStartTime);
    data << int32(talent.Flags);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonInfo const& garrison)
{
    /*
    TODO:: recheck missions code
    ASSERT(garrison.Missions.size() == garrison.MissionRewards.size());
    ASSERT(garrison.Missions.size() == garrison.MissionOvermaxRewards.size());
    ASSERT(garrison.Missions.size() == garrison.CanStartMission.size());*/

    data << int32(garrison.GarrTypeID);
    data << int32(garrison.GarrSiteID);
    data << int32(garrison.GarrSiteLevelID);
    data << uint32(garrison.Buildings.size());
    data << uint32(garrison.Plots.size());
    data << uint32(garrison.Followers.size());
    data << uint32(garrison.Missions.size());
    data << uint32(garrison.MissionRewards.size());
    data << uint32(garrison.MissionOvermaxRewards.size());
    data << uint32(garrison.MissionAreaBonuses.size());
    data << uint32(garrison.Talents.size());
    data << uint32(garrison.CanStartMission.size());
    data << uint32(garrison.ArchivedMissions.size());
    data << int32(garrison.NumFollowerActivationsRemaining);
    data << uint32(garrison.NumMissionsStartedToday);

    for (auto const& plot : garrison.Plots)
        data << plot;

    for (auto const& mission : garrison.Missions)
        data << *mission;

    for (auto const& missionReward : garrison.MissionRewards)
    {
        data << uint32(missionReward.size());
        for (auto const& missionRewardItem : missionReward)
            data << missionRewardItem;
    }

    for (auto const& missionReward : garrison.MissionOvermaxRewards)
    {
        data << uint32(missionReward.size());
        for (auto const& missionRewardItem : missionReward)
            data << missionRewardItem;
    }

    for (auto const& areaBonus : garrison.MissionAreaBonuses)
        data << *areaBonus;

    for (auto const& talent : garrison.Talents)
        data << talent;

    if (!garrison.ArchivedMissions.empty())
        data.append(garrison.ArchivedMissions.data(), garrison.ArchivedMissions.size());

    for (auto const& building : garrison.Buildings)
        data << *building;

    for (auto canStartMission : garrison.CanStartMission)
        data.WriteBit(canStartMission);

    data.FlushBits();

    for (auto const& follower : garrison.Followers)
        data << *follower;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::FollowerSoftCapInfo const& followerSoftCapInfo)
{
    data << int32(followerSoftCapInfo.GarrFollowerTypeID);
    data << uint32(followerSoftCapInfo.Count);
    return data;
}

WorldPackets::Garrison::GarrisonRemoteBuildingInfo::GarrisonRemoteBuildingInfo() : GarrPlotInstanceID(0), GarrBuildingID(0) { }
WorldPackets::Garrison::GarrisonRemoteBuildingInfo::GarrisonRemoteBuildingInfo(uint32 plotInstanceId, uint32 buildingId) : GarrPlotInstanceID(plotInstanceId), GarrBuildingID(buildingId) { }

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonRemoteBuildingInfo const& building)
{
    data << uint32(building.GarrPlotInstanceID);
    data << uint32(building.GarrBuildingID);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::Shipment const& shipment)
{
    data << shipment.ShipmentRecID;
    data << shipment.ShipmentID;
    data << shipment.FollowerDBID;
    data << uint32(shipment.CreationTime);
    data << shipment.ShipmentDuration;
    data << shipment.BuildingTypeID;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonRemoteSiteInfo const& site)
{
    data << uint32(site.GarrSiteLevelID);
    data << static_cast<uint32>(site.Buildings.size());
    for (auto const& building : site.Buildings)
        data << building;

    return data;
}

WorldPackets::Garrison::GarrisonBuildingLandmark::GarrisonBuildingLandmark() = default;
WorldPackets::Garrison::GarrisonBuildingLandmark::GarrisonBuildingLandmark(uint32 buildingPlotInstId, Position const& pos): Pos(pos), GarrBuildingPlotInstID(buildingPlotInstId) { }

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Garrison::GarrisonBuildingLandmark& landmark)
{
    data << uint32(landmark.GarrBuildingPlotInstID);
    data << landmark.Pos;

    return data;
}

WorldPacket const* WorldPackets::Garrison::GarrisonCreateResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrSiteLevelID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonDeleteResult::Write()
{
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrSiteID);

    return &_worldPacket;
}

WorldPackets::Garrison::FollowerSoftCapInfo::FollowerSoftCapInfo(int32 typeID, uint32 count)
{
    GarrFollowerTypeID = typeID;
    Count = count;
}

WorldPacket const* WorldPackets::Garrison::GetGarrisonInfoResult::Write()
{
    _worldPacket << int32(FactionIndex);
    _worldPacket << uint32(Garrisons.size());

    _worldPacket << uint32(FollowerSoftCaps.size());
    for (auto const& softCap : FollowerSoftCaps)
        _worldPacket << softCap;

    for (auto const& garrison : Garrisons)
        _worldPacket << garrison;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonMissionUpdate::Write()
{
    _worldPacket << uint32(ArchivedMissions.size());
    _worldPacket << uint32(CanStartMission.size());

    if (!ArchivedMissions.empty())
        _worldPacket.append(ArchivedMissions.data(), ArchivedMissions.size());

    for (bool canStartMission : CanStartMission)
        _worldPacket.WriteBit(canStartMission);

    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRemoteInfo::Write()
{
    _worldPacket << static_cast<uint32>(Sites.size());
    for (auto const& site : Sites)
        _worldPacket << site;

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonPurchaseBuilding::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> PlotInstanceID;
    _worldPacket >> BuildingID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonPlaceBuildingResult::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << uint32(Result);
    _worldPacket << BuildingInfo;
    _worldPacket.WriteBit(PlayActivationCinematic);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonCancelConstruction::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> PlotInstanceID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonBuildingRemoved::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << uint32(Result);
    _worldPacket << uint32(GarrPlotInstanceID);
    _worldPacket << uint32(GarrBuildingID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonLearnBlueprintResult::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << uint32(Result);
    _worldPacket << uint32(BuildingID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonUnlearnBlueprintResult::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << uint32(Result);
    _worldPacket << uint32(BuildingID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationDataResult::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << uint32(BlueprintsKnown ? BlueprintsKnown->size() : 0);
    _worldPacket << uint32(SpecializationsKnown ? SpecializationsKnown->size() : 0);
    if (BlueprintsKnown)
        for (auto blueprint : *BlueprintsKnown)
            _worldPacket << uint32(blueprint);

    if (SpecializationsKnown)
        for (auto specialization : *SpecializationsKnown)
            _worldPacket << uint32(specialization);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonBuildingLandmarks::Write()
{
    _worldPacket << static_cast<uint32>(Landmarks.size());
    for (GarrisonBuildingLandmark& landmark : Landmarks)
        _worldPacket << landmark;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonPlotPlaced::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << PlotInfo;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonPlotRemoved::Write()
{
    _worldPacket << uint32(GarrPlotInstanceID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonAddFollowerResult::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << uint32(Result);
    _worldPacket << Follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangedItemLevel::Write()
{
    _worldPacket << Follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangedAbilities::Write()
{
    _worldPacket << Follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangedDurability::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonBuildingActivated::Write()
{
    _worldPacket << uint32(GarrPlotInstanceID);

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonMissionBonusRoll::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> MissionRecID;
}

void WorldPackets::Garrison::GarrisonCheckUpgradeable::Read()
{
    _worldPacket >> GarrSiteID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonMissionBonusRollResult::Write()
{
    _worldPacket << MissionData;
    _worldPacket << MissionRecID;
    _worldPacket << Result;

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonStartMission::Read()
{
    _worldPacket >> NpcGUID;
    auto followerCount = _worldPacket.read<uint32>();
    _worldPacket >> MissionRecID;
    FollowerDBIDs.clear();
    for (uint8 i = 0; i < followerCount; ++i)
        FollowerDBIDs.push_back(_worldPacket.read<uint64>());
}

void WorldPackets::Garrison::GarrisonSwapBuildings::Read()
{
    _worldPacket >> NpcGUID >> PlotId1 >> PlotId2;
}

void WorldPackets::Garrison::GarrisonCompleteMission::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> MissionRecID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonAssignFollowerToBuildingResult::Write()
{
    _worldPacket << FollowerDBID;
    _worldPacket << Result;
    _worldPacket << PlotInstanceID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonLandingPage::Write()
{
    _worldPacket << Result;
    _worldPacket << static_cast<uint32>(MsgData.size());
    for (auto const& map : MsgData)
        _worldPacket << map;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonAddMissionResult::Write()
{
    _worldPacket << GarrTypeID;
    _worldPacket << Result;
    _worldPacket << unk; // 2 == GARRISON_RANDOM_MISSION_ADDED = 904
    _worldPacket << MissionData;

    _worldPacket << static_cast<uint32>(Reward.size());
    _worldPacket << static_cast<uint32>(BonusReward.size());

    for (auto const& v : Reward)
        _worldPacket << v;

    for (auto const& v : BonusReward)
        _worldPacket << v;

    _worldPacket << UnkBit;
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonUpgradeResult::Write()
{
    _worldPacket << Result;
    _worldPacket << GarrSiteLevelID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonStartMissionResult::Write()
{
    _worldPacket << Result;
    _worldPacket << Unk;
    _worldPacket << MissionData;
    _worldPacket << static_cast<uint32>(FollowerDBIDs.size());
    for (auto const& map : FollowerDBIDs)
        _worldPacket << map;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonCompleteMissionResult::Write()
{
    _worldPacket << Result;
    _worldPacket << MissionData;
    _worldPacket << MissionRecID;
    _worldPacket.WriteBit(Succeeded);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonCompleteMissionResultNew::Write()
{
    _worldPacket << Result;
    _worldPacket << MissionData;
    _worldPacket << MissionRecID;
    _worldPacket << static_cast<uint32>(followerData.size());
    for (auto data : followerData)
    {
        _worldPacket << data.FollowerDbID;
        _worldPacket << data.unk32;
    }
    _worldPacket.WriteBit(Succeeded);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonIsUpgradeableResult::Write()
{
    _worldPacket << Result;

    return &_worldPacket;
}

void WorldPackets::Garrison::CreateShipment::Read()
{
    _worldPacket >> NpcGUID >> Count;
}

void WorldPackets::Garrison::GarrisonRequestShipmentInfo::Read()
{
    _worldPacket >> NpcGUID;
}

void WorldPackets::Garrison::GarrisonRequestResearchTalent::Read()
{
    _worldPacket >> TalentID;
}

void WorldPackets::Garrison::GarrisonOpenMissionNpcRequest::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> GarrFollowerTypeID;
}

WorldPacket const* WorldPackets::Garrison::GetShipmentInfoResponse::Write()
{
    _worldPacket.WriteBit(Success);
    _worldPacket.FlushBits();

    _worldPacket << ShipmentID;
    _worldPacket << MaxShipments;
    _worldPacket << static_cast<uint32>(Shipments.size());
    _worldPacket << PlotInstanceID;
    for (auto const& map : Shipments)
        _worldPacket << map;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::CreateShipmentResponse::Write()
{
    _worldPacket << ShipmentID;
    _worldPacket << ShipmentRecID;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::OpenShipmentNPCFromGossip::Write()
{
    _worldPacket << NpcGUID;
    _worldPacket << CharShipmentContainerID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::SetupTrophy::Write()
{
    _worldPacket << uint32(Trophys.size());
    for (auto const& map : Trophys)
    {
        _worldPacket << map.Unk1;
        _worldPacket << map.Unk2;
    }

    return &_worldPacket;
}

void WorldPackets::Garrison::UpgradeGarrison::Read()
{
    _worldPacket >> NpcGUID;
}

void WorldPackets::Garrison::TrophyData::Read()
{
    _worldPacket >> TrophyGUID;
    _worldPacket >> NewTrophyID;
}

void WorldPackets::Garrison::RevertTrophy::Read()
{
    _worldPacket >> TrophyGUID;
}

void WorldPackets::Garrison::GetTrophyList::Read()
{
    _worldPacket >> TrophyTypeID;
}

void WorldPackets::Garrison::GarrisonSetFollowerInactive::Read()
{
    _worldPacket >> FollowerDBID;
    Inactive = _worldPacket.ReadBit();
}

void WorldPackets::Garrison::GarrisonRemoveFollowerFromBuilding::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> FollowerDBID;
}

void WorldPackets::Garrison::GarrisonAssignFollowerToBuilding::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> PlotInstanceID;
    _worldPacket >> FollowerDBID;
}

WorldPacket const* WorldPackets::Garrison::GetTrophyListResponse::Write()
{
    _worldPacket.WriteBit(Success);
    _worldPacket.FlushBits();

    _worldPacket << static_cast<uint32>(MsgData.size());
    for (auto const& map : MsgData)
    {
        _worldPacket << map.TrophyID;
        _worldPacket << map.Unk1;
        _worldPacket << map.Unk2;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::ReplaceTrophyResponse::Write()
{
    _worldPacket.WriteBit(Success);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonOpenArchitect::Write()
{
    _worldPacket << NpcGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonOpenUpgradeNpcResponse::Write()
{
    _worldPacket << NpcGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRemoveFollowerResult::Write()
{
    _worldPacket << FollowerDBID;
    _worldPacket << Result;
    _worldPacket << Destroyed;
    _worldPacket << GarrTypeID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonOpenRecruitmentNpc::Write()
{
    _worldPacket << Guid;
    _worldPacket << GarrTypeID;
    _worldPacket << Result;
    for (auto const& follower : Followers)
        _worldPacket << follower;
    _worldPacket << CanRecruitFollower;
    _worldPacket << UnkBit;
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRecruitmentFollowersGenerated::Write()
{
    _worldPacket << Result;
    for (auto const& follower : Followers)
        _worldPacket << follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonResearchTalent::Write()
{
    _worldPacket << Result;
    _worldPacket << GarrTypeID;
    _worldPacket << TalentID;
    _worldPacket << ResearchTime;
    _worldPacket << Flags;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangedXP::Write()
{
    _worldPacket << TotalXp;
    _worldPacket << Result;
    _worldPacket << Follower;
    _worldPacket << Follower2;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonFollowerChangeStatus::Write()
{
    _worldPacket << Result;
    _worldPacket << Follower;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonNumFollowerActivationsRemaining::Write()
{
    _worldPacket << Amount;
    _worldPacket << UnkInt;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRemoveFollowerFromBuildingResult::Write()
{
    _worldPacket << FollowerDBID;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::QueryGarrisonCreatureNameResponse::Write()
{
    _worldPacket << InqueKey;
    _worldPacket << NpcGUID;
    _worldPacket.WriteBit(Name.is_initialized());
    _worldPacket.FlushBits();

    if (Name)
        _worldPacket.WriteString(*Name, 8);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRequestMissionNpc::Write()
{
    _worldPacket << NpcGuid;
    _worldPacket << GarrFollowerTypeID;

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonGenerateRecruits::Read()
{
    _worldPacket >> NpcGuid;
    _worldPacket >> MechanicTypeID;
    _worldPacket >> TraitID;
}

void WorldPackets::Garrison::GarrisonRenameFollower::Read()
{
    _worldPacket >> FollowerDBID;
    FollowerName = _worldPacket.ReadString(_worldPacket.ReadBits(7));
}

void WorldPackets::Garrison::GarrisonSetRecruitmentPreferences::Read()
{
    _worldPacket >> NpcGuid;
    _worldPacket >> MechanicTypeID;
    _worldPacket >> TraitID;
}

void WorldPackets::Garrison::GarrisonRecruitFollower::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> FollowerIndex;
}

WorldPacket const* WorldPackets::Garrison::GarrisonRecruitFollowerResult::Write()
{
    _worldPacket << Result;
    _worldPacket << Follower;

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonRemoveFollower::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> FollowerDBID;
}

ByteBuffer& operator<<(ByteBuffer& _worldPacket, WorldPackets::Garrison::GarrTradeSkill const& tradeSkill)
{
    _worldPacket << tradeSkill.SpellID;
    _worldPacket << static_cast<uint32>(tradeSkill.SkillLineIDs.size());
    _worldPacket << static_cast<uint32>(tradeSkill.SkillRanks.size());
    _worldPacket << static_cast<uint32>(tradeSkill.SkillMaxRanks.size());
    _worldPacket << static_cast<uint32>(tradeSkill.KnownAbilitySpellIDs.size());

    for (auto const& data : tradeSkill.SkillLineIDs)
        _worldPacket << data;

    for (auto const& data : tradeSkill.SkillRanks)
        _worldPacket << data;

    for (auto const& data : tradeSkill.SkillMaxRanks)
        _worldPacket << data;

    for (auto const& data : tradeSkill.KnownAbilitySpellIDs)
        _worldPacket << data;

    return _worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonTradeSkillResponse::Write()
{
    _worldPacket << GUID;
    _worldPacket << TradeSkill;

    _worldPacket << static_cast<uint32>(PlayerConditionID.size());
    for (auto const& data : PlayerConditionID)
        _worldPacket << data;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Garrison::GarrisonResponseClassSpecCategoryInfo::Write()
{
    _worldPacket << GarrFollowerTypeID;
    _worldPacket << static_cast<uint32>(Datas.size());
    for (auto const& v : Datas)
    {
        _worldPacket << v.Category;
        _worldPacket << v.Option;
    }

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonRequestClassSpecCategoryInfo::Read()
{
    _worldPacket >> GarrFollowerTypeID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonOpenMissionNpc::Write()
{
    _worldPacket << int32(GarrTypeID);
    _worldPacket << int32(Result);
    _worldPacket << static_cast<uint32>(Missions.size());
    for (auto const& missionID : Missions)
        _worldPacket << int32(missionID);

    _worldPacket.WriteBit(UnkBit1);
    _worldPacket.WriteBit(PreventXmlOpenMissionEvent);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Garrison::GarrisonRequestScoutingMap::Read()
{
    _worldPacket >> ID;
}

WorldPacket const* WorldPackets::Garrison::GarrisonScoutingMapResult::Write()
{
    _worldPacket << ID;
    _worldPacket.WriteBit(Active);
    _worldPacket.FlushBits();

    return &_worldPacket;
}
