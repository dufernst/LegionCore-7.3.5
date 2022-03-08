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

#include "WorldSession.h"
#include "Garrison.h"
#include "GarrisonPackets.h"
#include "NPCPackets.h"
#include "GossipDef.h"

void WorldSession::HandleGetGarrisonInfo(WorldPackets::Garrison::GetGarrisonInfo& /*packet*/)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendInfo();
}

void WorldSession::HandleGarrisonPurchaseBuilding(WorldPackets::Garrison::GarrisonPurchaseBuilding& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->PlaceBuilding(packet.PlotInstanceID, packet.BuildingID);
}

void WorldSession::HandleGarrisonCancelConstruction(WorldPackets::Garrison::GarrisonCancelConstruction& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->CancelBuildingConstruction(packet.PlotInstanceID);
}

void WorldSession::HandleGarrisonRequestBlueprintAndSpecializationData(WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationData& /*packet*/)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendBlueprintAndSpecializationData();
}

void WorldSession::HandleGarrisonGetBuildingLandmarks(WorldPackets::Garrison::GarrisonGetBuildingLandmarks& /*packet*/)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendBuildingLandmarks(_player);
}

void WorldSession::HandleGarrisonMissionBonusRoll(WorldPackets::Garrison::GarrisonMissionBonusRoll& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC | UNIT_NPC_FLAG2_SHIPMENT_CRAFTER))
        return;

    auto const& garrison = _player->GetGarrison();
    if (!garrison)
        return;

    auto const& mission = garrison->GetMissionByRecID(packet.MissionRecID);
    if (!mission)
        return;

    auto canBonusRoll([&mission]() -> bool
    {
        if (!sGarrMissionStore.LookupEntry(mission->PacketInfo.RecID))
            return false;

        if (mission->PacketInfo.State != MISSION_STATE_WAITING_BONUS && mission->PacketInfo.State != MISSION_STATE_WAITING_OWERMAX_BONUS)
            return false;

        if (mission->PacketInfo.StartTime + mission->PacketInfo.Duration > time(nullptr))
            return false;

        return true;
    });

    if (canBonusRoll())
        mission->BonusRoll(_player);
    else
    {
        WorldPackets::Garrison::GarrisonMissionBonusRollResult res;
        res.MissionData = mission->PacketInfo;
        res.MissionRecID = mission->PacketInfo.RecID;
        res.Result = GARRISON_SUCCESS; // which code thre?
        _player->SendDirectMessage(res.Write());
    }
}

void WorldSession::HandleGarrisonRequestLandingPageShipmentInfo(WorldPackets::Garrison::GarrisonRequestLandingPageShipmentInfo& /*packet*/)
{ 
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendGarrisonShipmentLandingPage();
}

void WorldSession::HandleGarrisonCheckUpgradeable(WorldPackets::Garrison::GarrisonCheckUpgradeable& packet)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendGarrisonUpgradebleResult(_player, packet.GarrSiteID);
}

void WorldSession::HandleGarrisonStartMission(WorldPackets::Garrison::GarrisonStartMission& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC | UNIT_NPC_FLAG2_SHIPMENT_CRAFTER))
        return;

    if (!sGarrMissionStore.LookupEntry(packet.MissionRecID))
        return;

    auto garrison = _player->GetGarrison();
    if (!garrison)
        return;

    if (auto const& mission = garrison->GetMissionByRecID(packet.MissionRecID))
        mission->Start(_player, packet.FollowerDBIDs);
}

void WorldSession::HandleGarrisonSwapBuildings(WorldPackets::Garrison::GarrisonSwapBuildings& packet)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->Swap(packet.PlotId1, packet.PlotId2);
}

void WorldSession::HandleGarrisonCompleteMission(WorldPackets::Garrison::GarrisonCompleteMission& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC | UNIT_NPC_FLAG2_SHIPMENT_CRAFTER))
        return;

    if (!sGarrMissionStore.LookupEntry(packet.MissionRecID))
        return;

    if (Garrison* garrison = _player->GetGarrison())
    {
        if (Mission* mission = garrison->GetMissionByRecID(packet.MissionRecID))
        {
            if (mission->PacketInfo.State != MISSION_STATE_IN_PROGRESS)
                return;

            if (mission->PacketInfo.StartTime + mission->PacketInfo.Duration <= time(nullptr))
                mission->Complete(_player);
        }
    }
}

void WorldSession::HandleCreateShipment(WorldPackets::Garrison::CreateShipment& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_AI_OBSTACLE | UNIT_NPC_FLAG2_RECRUITER))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->CreateShipment(packet.NpcGUID, packet.Count);
}

void WorldSession::HandleGarrisonRequestShipmentInfo(WorldPackets::Garrison::GarrisonRequestShipmentInfo& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_AI_OBSTACLE | UNIT_NPC_FLAG2_RECRUITER))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendShipmentInfo(packet.NpcGUID);
}

void WorldSession::HandleGarrisonResearchTalent(WorldPackets::Garrison::GarrisonRequestResearchTalent& packet)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->StartClassHallUpgrade(packet.TalentID);
    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleGarrisonOpenMissionNpc(WorldPackets::Garrison::GarrisonOpenMissionNpcRequest& packet)
{
    switch (packet.GarrFollowerTypeID)
    {
    case GarrisonConst::FollowerType::Garrison:
    {
        if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_MISSION_NPC | UNIT_NPC_FLAG2_SHIPMENT_CRAFTER))
            return;

        WorldPackets::Garrison::GarrisonOpenMissionNpc response;
        response.GarrTypeID = GARRISON_TYPE_GARRISON;
        //response.PreventXmlOpenMissionEvent = true;

        if (Garrison* garrison = _player->GetGarrison())
        {
            for (const std::pair<uint64, Mission>& mission : garrison->GetMissions(GARRISON_TYPE_GARRISON))
            {
                // TODO: going from uint64 to int32 (see GarrisonPackets.cpp), will this ever be a problem?
                response.Missions.push_back(static_cast<int32>(mission.first));
            }
        }
        SendPacket(response.Write());

        if (auto garrison = _player->GetGarrison())
            garrison->SendMissionListUpdate(true);
        break;
    }
    case GarrisonConst::FollowerType::Follower:
    {
        if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_SHIPMENT_CRAFTER))
            return;

        WorldPackets::Garrison::GarrisonOpenMissionNpc response;
        response.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
        response.PreventXmlOpenMissionEvent = true;
        SendPacket(response.Write());

        if (auto garrison = _player->GetGarrison())
            garrison->CheckBasicRequirements();
        break;
    }
    default:
        break;
    }
}

void WorldSession::HandleRequestClassSpecCategoryInfo(WorldPackets::Garrison::GarrisonRequestClassSpecCategoryInfo& packet)
{
    WorldPackets::Garrison::GarrisonResponseClassSpecCategoryInfo resp;
    resp.GarrFollowerTypeID = packet.GarrFollowerTypeID;

    /*WorldPackets::Garrison::GarrisonResponseClassSpecCategoryInfo::FollowersClassSpecStruct &d1 = resp.Datas.back();
    d1.Category = 99;
    d1.Option = 43;

    resp.Datas.emplace_back();
    WorldPackets::Garrison::GarrisonResponseClassSpecCategoryInfo::FollowersClassSpecStruct &d2 = resp.Datas.back();
    d2.Category = 100;
    d2.Option = 44; */

    SendPacket(resp.Write());
}

void WorldSession::HandleUpgradeGarrison(WorldPackets::Garrison::UpgradeGarrison& packet)
{
    if (!_player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->Upgrade();
}

void WorldSession::HandleTrophyData(WorldPackets::Garrison::TrophyData& packet)
{
    switch (packet.GetOpcode())
    {
        case CMSG_REPLACE_TROPHY:
            break;
        case CMSG_CHANGE_MONUMENT_APPEARANCE:
            break;
        default:
            break;
    }
}

void WorldSession::HandleRevertTrophy(WorldPackets::Garrison::RevertTrophy& /*packet*/)
{ }

void WorldSession::HandleGetTrophyList(WorldPackets::Garrison::GetTrophyList& /*packet*/)
{ }

void WorldSession::HandleGarrisonSetFollowerInactive(WorldPackets::Garrison::GarrisonSetFollowerInactive& packet)
{
    if (Garrison* garrison = _player->GetGarrison())
    {
        if (auto follower = garrison->GetFollower(packet.FollowerDBID))
        {
            if (packet.Inactive)
            {
                if (follower->PacketInfo.FollowerStatus & GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_INACTIVE)
                    return;

                follower->PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_INACTIVE;

                WorldPackets::Garrison::GarrisonFollowerChangeStatus packetStatus;
                packetStatus.Follower = follower->PacketInfo;
                packetStatus.Result = 0;
                _player->SendDirectMessage(packetStatus.Write());

                WorldPackets::Garrison::GarrisonRemoveFollowerFromBuildingResult packetResult;
                packetResult.FollowerDBID = packet.FollowerDBID;
                packetResult.Result = 0;
                _player->SendDirectMessage(packetResult.Write());
            }
            else
            {
                if (!_player->HasEnoughMoney(int64(2500000)))
                    return;

                if (!(follower->PacketInfo.FollowerStatus & GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_INACTIVE))
                    return;

                follower->PacketInfo.FollowerStatus &= ~GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_INACTIVE;

                WorldPackets::Garrison::GarrisonFollowerChangeStatus packetStatus;
                packetStatus.Follower = follower->PacketInfo;
                packetStatus.Result = 0;
                _player->SendDirectMessage(packetStatus.Write());

                WorldPackets::Garrison::GarrisonNumFollowerActivationsRemaining packetResult;
                packetResult.Amount = 0;
                packetResult.UnkInt = 0;
                _player->SendDirectMessage(packetResult.Write());
                _player->ModifyMoney(-int64(2500000));
            }
            follower->DbState = DB_STATE_CHANGED;
        }
    }
}

void WorldSession::HandleGarrisonRemoveFollowerFromBuilding(WorldPackets::Garrison::GarrisonRemoveFollowerFromBuilding& /*packet*/)
{ }

void WorldSession::HandleGarrisonAssignFollowerToBuilding(WorldPackets::Garrison::GarrisonAssignFollowerToBuilding& /*packet*/)
{ }

void WorldSession::HandleGarrisonGenerateRecruits(WorldPackets::Garrison::GarrisonGenerateRecruits& /*packet*/)
{ }

void WorldSession::HandleGarrisonRecruitFollower(WorldPackets::Garrison::GarrisonRecruitFollower& /*packet*/)
{ }

void WorldSession::HandleGarrisonRemoveFollower(WorldPackets::Garrison::GarrisonRemoveFollower& /*packet*/)
{ }

void WorldSession::HandleGarrisonRenameFollower(WorldPackets::Garrison::GarrisonRenameFollower& /*packet*/)
{ }

void WorldSession::HandleGarrisonSetRecruitmentPreferences(WorldPackets::Garrison::GarrisonSetRecruitmentPreferences& /*packet*/)
{ }
