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

#include "Garrison.h"
#include "Creature.h"
#include "GameObject.h"
#include "GarrisonMgr.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "VehicleDefines.h"
#include "MiscPackets.h"
#include "InstanceScript.h"
#include "LootPackets.h"
#include "DatabaseEnv.h"
#include "NPCPackets.h"
#include <G3D/Quat.h>

#define SALVAGE_ITEM 114119
#define SALVAGE_ITEM_BIG 114120
#define GARRISON_CHEST_FORMULA_DEBUG false

uint32 idxShipment(CharShipmentContainerEntry const* c)
{
    return c->GarrBuildingType ? c->GarrBuildingType : c->ID;
}

uint32 getSiteLevelIdById(uint32 team, uint8 lvl)
{
    switch (lvl)
    {
        case 1:
            return team == ALLIANCE ? 5 : 258;
        case 2:
            return team == ALLIANCE ? 444 : 445;
        case 3:
            return team == ALLIANCE ? 6 : 259;
    }

    ASSERT(false);
    return 0;
}

uint8 getGarrisoneTypeBySite(uint16 siteID)
{
    switch (siteID)
    {
        case SITE_ID_GARRISON_ALLIANCE:
        case SITE_ID_GARRISON_HORDE:
            return GARRISON_TYPE_GARRISON;
        case SITE_ID_CLASS_ORDER_ALLIANCE:
        case SITE_ID_CLASS_ORDER_HORDE:
            return GARRISON_TYPE_CLASS_ORDER;
    }
    return GARRISON_TYPE_UNK;
}

uint32 getQuestIdReqForShipment(uint32 siteID, uint32 buildingType)
{
    switch (buildingType)
    {
        case GARR_BTYPE_ALCHEMY_LAB:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36641 : 37568;
        case GARR_BTYPE_TAILORING:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36643 : 37575;
        case GARR_BTYPE_FORGE:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 35168 : 37569;
        case GARR_BTYPE_TANNERY:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36642 : 37574;
        case GARR_BTYPE_GEM:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36644 : 37573;
        case GARR_BTYPE_ENCHANTERS:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36645 : 37570;
        case GARR_BTYPE_ENGINEERING:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36646 : 37571;
        case GARR_BTYPE_SCRIBE:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36647 : 37572;
        case GARR_BTYPE_LUMBER_MILL:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 36189 : 36137;
        case GARR_BTYPE_TRADING_POST:
            return siteID == SITE_ID_GARRISON_ALLIANCE ? 37088 : 37062;
        default:
            return 0;
    }
    return 0;
}

//! Shipment for requarement.
uint32 getProgressShipment(uint32 questID)
{
    switch (questID)
    {
        //GARR_BTYPE_ALCHEMY_LAB
        case 36641: return 114;
        case 37568: return 122;
        //GARR_BTYPE_TAILORING
        case 36643: return 120;
        case 37575: return 136;
        //GARR_BTYPE_FORGE
        case 35168: return 113;
        case 37569: return 123;
        //GARR_BTYPE_TANNERY
        case 36642: return 119;
        case 37574: return 134;
        //GARR_BTYPE_GEM
        case 36644: return 118;
        case 37573: return 131;
        //GARR_BTYPE_ENCHANTERS
        case 36645: return 115;
        case 37570: return 126;
        //GARR_BTYPE_ENGINEERING
        case 36646: return 116;
        case 37571: return 128;
        //GARR_BTYPE_SCRIBE
        case 36647: return 117;
        case 37572: return 130;
        //GARR_BTYPE_LUMBER_MILL
        case 36189: case 36137: return 0;
        //GARR_BTYPE_TRADING_POST:
        case 37088: case 37062: return 0;
    }
    ASSERT(false);
    return 0;
}

Garrison::Garrison(Player* owner) : _owner(owner), _followerActivationsRemainingToday(1), _lastResTaken(0)
{
    updateTimer.SetInterval(15 * IN_MILLISECONDS);
    updateTimer.Reset();

    for (auto& site : _siteLevel)
        site = nullptr;

    memset(_troopCount, 0, sizeof(_troopCount));
}

bool Garrison::LoadFromDB(PreparedQueryResult const& garrison, PreparedQueryResult const& blueprints, PreparedQueryResult const& buildings, PreparedQueryResult const& followers, PreparedQueryResult const& abilities, PreparedQueryResult const& missions, PreparedQueryResult const& shipments, PreparedQueryResult const& talents)
{
    if (!garrison)
        return false;

    Field* fields = nullptr;

    do
    {
        fields = garrison->Fetch();

        auto siteLevel = sGarrSiteLevelStore.LookupEntry(fields[0].GetUInt32());
        if (!siteLevel)
            return false;

        uint8 type = getGarrisoneTypeBySite(siteLevel->GarrSiteID);

        //! important not allow add sitelevel to _siteLevel if exist.
        if (type == GARRISON_TYPE_GARRISON && _siteLevel[type])
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BY_SITE);
            stmt->setUInt64(0, _owner->GetGUIDLow());
            stmt->setUInt32(1, _siteLevel[type]->GarrLevel > siteLevel->GarrLevel ? siteLevel->ID : _siteLevel[type]->ID);
            CharacterDatabase.Execute(stmt);

            if (_siteLevel[type]->GarrLevel > siteLevel->GarrLevel)
                continue;
        }

        _siteLevel[type] = siteLevel;
        _MissionGen = fields[3].GetUInt32();

        //! ToDo: possible port to struct and read for enother type too.
        if (type == GARRISON_TYPE_GARRISON)
        {
            InitializePlots(type);

            _followerActivationsRemainingToday = fields[1].GetUInt32();
            _lastResTaken = fields[2].GetUInt32();

            if (siteLevel->GarrSiteID == SITE_ID_GARRISON_ALLIANCE && _owner->GetTeam() == HORDE ||
                siteLevel->GarrSiteID == SITE_ID_GARRISON_HORDE && _owner->GetTeam() == ALLIANCE)
            {
                siteLevel = sGarrSiteLevelStore.LookupEntry(getSiteLevelIdById(_owner->GetTeam(), siteLevel->GarrLevel));

                if (!siteLevel)
                    return false;
            }
        }

    } while (garrison->NextRow());

    //SELECT talentID, researchStartTime, flags  FROM character_garrison_talents WHERE guid = ?
    if (talents)
    {
        do
        {
            fields = talents->Fetch();
            AddTalentToStore(fields[0].GetUInt32(), fields[1].GetUInt32(), fields[2].GetUInt32());

        } while (talents->NextRow());
    }

    if (blueprints)
    {
        do
        {
            fields = blueprints->Fetch();
            if (auto building = sGarrBuildingStore.LookupEntry(fields[0].GetUInt32()))
                _knownBuildings.insert(building->ID);

        } while (blueprints->NextRow());
    }

    // only GARRISON_TYPE_GARRISON
    if (buildings)
    {
        do
        {
            fields = buildings->Fetch();
            uint32 plotInstanceId = fields[0].GetUInt32();
            uint32 buildingId = fields[1].GetUInt32();
            time_t timeBuilt = time_t(fields[2].GetUInt64());
            bool active = fields[3].GetBool();
            std::string data = fields[4].GetString();

            Plot* plot = GetPlot(plotInstanceId);
            if (!plot)
                continue;

            auto building = sGarrBuildingStore.LookupEntry(buildingId);
            if (!building)
                continue;

            plot->BuildingInfo.PacketInfo = boost::in_place();
            plot->BuildingInfo.PacketInfo->GarrPlotInstanceID = plotInstanceId;
            plot->BuildingInfo.PacketInfo->GarrBuildingID = buildingId;
            plot->BuildingInfo.PacketInfo->TimeBuilt = timeBuilt;
            plot->BuildingInfo.PacketInfo->Active = active;

            Tokenizer tokens(data, ' ');

            uint8 index = 0;

            for (auto iter = tokens.begin(); index < MAX_BUILDING_SAVE_DATA && iter != tokens.end(); ++iter, ++index)
                _buildingData[building->BuildingType][index] = uint32(atol(*iter));

            plot->db_state_building = DB_STATE_UNCHANGED;

            if (plot->BuildingInfo.PacketInfo->Active)
            {
                // only garrison.
                if (!_siteLevel[GARRISON_TYPE_GARRISON])
                    continue;

                uint32 questID = getQuestIdReqForShipment(_siteLevel[GARRISON_TYPE_GARRISON]->GarrSiteID, building->BuildingType);
                if (!questID)
                    continue;

                switch (building->BuildingType)
                {
                    case GARR_BTYPE_LUMBER_MILL:
                        if (_owner->GetQuestStatus(questID) == QUEST_STATUS_REWARDED)
                        {
                            _owner->learnSpell(167911, true);
                            _owner->learnSpell(167895, true);
                            _owner->learnSpell(167898, true);
                            
                        }
                        break;
                    default:
                        break;
                }
            }
        } while (buildings->NextRow());
    }

    //          0          1          2              3          4               5               6             7         8
    // SELECT dbId, missionRecID, offerTime, offerDuration, startTime, travelDuration, missionDuration, missionState, chance FROM character_garrison_missions WHERE guid = ?
    if (missions)
    {
        do
        {
            fields = missions->Fetch();

            uint64 dbId = fields[0].GetUInt64();
            uint32 missionRecID = fields[1].GetUInt32();
            uint32 offerTime = fields[2].GetUInt32();
            uint32 offerDuration = fields[3].GetUInt32();
            uint32 StartTime = fields[4].GetUInt32();

            auto missionInfoEntry = sGarrMissionStore.LookupEntry(missionRecID);
            if (!missionInfoEntry)
                continue;

            if (missionInfoEntry->GarrTypeID != GARRISON_TYPE_GARRISON && missionInfoEntry->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
                continue;

            if (!offerDuration && missionInfoEntry->OfferDuration)
                offerDuration = missionInfoEntry->OfferDuration;

            // time is over
            if (!StartTime && offerDuration && (offerTime + offerDuration <= time(nullptr)))
            {
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_MISSIONS_DB_ID);
                stmt->setUInt64(0, dbId);
                CharacterDatabase.Execute(stmt);
                continue;
            }

            _missionIds[missionInfoEntry->GarrTypeID].insert(missionRecID);
            Mission& mission = _missions[missionInfoEntry->GarrTypeID][dbId];
            mission.PacketInfo.DbID = dbId;
            mission.PacketInfo.RecID = missionRecID;
            mission.PacketInfo.OfferTime = offerTime;
            mission.PacketInfo.OfferDuration = offerDuration;
            mission.PacketInfo.StartTime = StartTime;
            mission.PacketInfo.TravelDuration = fields[5].GetUInt32();
            mission.PacketInfo.Duration = fields[6].GetUInt32();
            mission.PacketInfo.State = fields[7].GetUInt32();
            mission.PacketInfo.SuccesChance = fields[8].GetUInt16();

            if (!sGarrisonMgr.GetMissionRewardByRecID(missionRecID))
                continue;

            mission.DbState = DB_STATE_UNCHANGED;
        } while (missions->NextRow());
    }

    //           0           1        2      3                4               5   6                7               8       9
    // SELECT dbId, followerId, quality, level, itemLevelWeapon, itemLevelArmor, xp, currentBuilding, currentMission, status FROM character_garrison_followers WHERE guid = ?
    if (followers)
    {
        do
        {
            fields = followers->Fetch();

            auto followerId = fields[1].GetUInt32();
            auto foloowerEntry = sGarrFollowerStore.LookupEntry(followerId);
            if (!foloowerEntry)
                continue;

            if (foloowerEntry->GarrTypeID != GARRISON_TYPE_GARRISON && foloowerEntry->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
                continue;

            auto dbId = fields[0].GetUInt64();
            _followerIds[foloowerEntry->GarrTypeID].insert(followerId);
            auto& follower = _followers[foloowerEntry->GarrTypeID][dbId];
            follower.PacketInfo.DbID = dbId;
            follower.PacketInfo.GarrFollowerID = followerId;
            follower.PacketInfo.Quality = fields[2].GetUInt32();

            switch (foloowerEntry->GarrFollowerTypeID)
            {
            case GarrisonConst::FollowerType::Garrison:
                follower.PacketInfo.FollowerLevel = std::min(fields[3].GetUInt32(), uint32(GarrisonConst::Globals::MaxFollowerLevel));
                break;
            case GarrisonConst::FollowerType::Follower:
                follower.PacketInfo.FollowerLevel = std::min(fields[3].GetUInt32(), uint32(GarrisonConst::Globals::MaxFollowerLevelHall));
                break;
            default:
                break;
            }

            follower.PacketInfo.ItemLevelWeapon = fields[4].GetUInt32();
            follower.PacketInfo.ItemLevelArmor = fields[5].GetUInt32();
            follower.PacketInfo.Xp = fields[6].GetUInt32();
            follower.PacketInfo.CurrentBuildingID = fields[7].GetUInt32();
            follower.PacketInfo.CurrentMissionID = fields[8].GetUInt32();
            follower.PacketInfo.FollowerStatus = fields[9].GetUInt32();
            follower.PacketInfo.Vitality = fields[10].GetUInt16();

            follower.TypeID = foloowerEntry->GarrFollowerTypeID;
            follower.DbState = DB_STATE_UNCHANGED;

            if (!sGarrBuildingStore.LookupEntry(follower.PacketInfo.CurrentBuildingID))
            {
                follower.DbState = DB_STATE_CHANGED;
                follower.PacketInfo.CurrentBuildingID = 0;
            }

            if (follower.PacketInfo.CurrentMissionID)
            {
                bool clearMission = true;
                Mission* mission = GetMissionByRecID(follower.PacketInfo.CurrentMissionID);
                if (mission && mission->PacketInfo.StartTime)
                {
                    if (GarrMissionEntry const* missionEntry = sGarrMissionStore.LookupEntry(follower.PacketInfo.CurrentMissionID))
                    {
                        if (missionEntry->MaxFollowers > mission->CurrentFollowerDBIDs.size())
                        {
                            mission->CurrentFollowerDBIDs.push_back(follower.PacketInfo.DbID);
                            clearMission = false;
                        }
                    }
                }
                if (clearMission)
                {
                    follower.DbState = DB_STATE_CHANGED;
                    follower.PacketInfo.CurrentMissionID = 0;
                }
            }

            if (_missionIds[foloowerEntry->GarrTypeID].empty())
                if (auto mission = sGarrisonMgr.GetMissionAtFollowerTaking(followerId))
                    AddMission(mission->ID);

            if (foloowerEntry->Vitality)
            {
                follower.PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_TROOP;
                follower.PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_NO_XP_GAIN;

                // disable more then limit.
                uint32 limit = GetTroopLimit(foloowerEntry->Vitality, _owner->GetTeam() == ALLIANCE ? foloowerEntry->AllianceGarrClassSpecID : foloowerEntry->HordeGarrClassSpecID);
                if (_troopCount[foloowerEntry->Vitality] > limit)
                    follower.PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_INACTIVE;

                ASSERT(foloowerEntry->Vitality < 5);
                ++_troopCount[foloowerEntry->Vitality];
            }

        } while (followers->NextRow());

        if (abilities)
        {
            do
            {
                fields = abilities->Fetch();
                GarrAbilityEntry const* ability = sGarrAbilityStore.LookupEntry(fields[1].GetUInt32());
                if (!ability)
                    continue;

                uint64 dbId = fields[0].GetUInt64();
                auto itr = _followers[GARRISON_TYPE_GARRISON].find(dbId);
                if (itr == _followers[GARRISON_TYPE_GARRISON].end())
                {
                    itr = _followers[GARRISON_TYPE_CLASS_ORDER].find(dbId);
                    if (itr == _followers[GARRISON_TYPE_CLASS_ORDER].end())
                        continue;
                }
                itr->second.db_state_ability = DB_STATE_UNCHANGED;
                itr->second.PacketInfo.AbilityID.push_back(ability->ID);
            } while (abilities->NextRow());
        }
    }

    //SELECT dbId, shipmentID, orderTime FROM character_garrison_shipment WHERE guid = ? ORDER BY `orderTime` ASC
    if (shipments)
    {
        do
        {
            fields = shipments->Fetch();

            uint64 dbId = fields[0].GetUInt64();
            uint32 shipmentID = fields[1].GetUInt16();
            uint32 start = fields[2].GetUInt32();
            uint32 end = fields[3].GetUInt32();
            PlaceShipment(shipmentID, start, end, dbId);

        } while (shipments->NextRow());
    }

    ///!< check generator mission immidieatly.
    if (time(nullptr) > _MissionGen)
    {
        _MissionGen = time(nullptr) + DAY;
        GenerateRandomMission();
    }

    //@TODO ICE - not needed if all data in other place nice ?
    ///!< Check follower abilities.
    for (int32 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
        for (auto& v : _followers[i])
            if (v.second.PacketInfo.AbilityID.size() < 1/*GarrisonMgr::GetMinAbilityCount(v.second.PacketInfo.Quality)*/)
                ReTrainFollower(nullptr, v.second.PacketInfo.GarrFollowerID);

    return true;
}

void Garrison::SaveToDB(SQLTransaction const& trans)
{
    bool canSaveBuild = false;

    PreparedStatement* stmt = nullptr;
    for (auto v : _siteLevel)
    {
        if (!v)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON);
        stmt->setUInt64(0, _owner->GetGUIDLow());
        stmt->setUInt32(1, v->ID);
        stmt->setUInt32(2, _followerActivationsRemainingToday);
        stmt->setUInt32(3, _lastResTaken);
        stmt->setUInt32(4, _MissionGen);
        trans->Append(stmt);
        canSaveBuild = true;
    }

    if (!canSaveBuild)
        return;

    for (uint32 building : _buildingsToSave)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_BLUEPRINTS);
        stmt->setUInt64(0, _owner->GetGUIDLow());
        stmt->setUInt32(1, building);
        trans->Append(stmt);
    }

    std::ostringstream ss;

    for (auto &p : _plots)
    {
        Plot &plot = p.second;

        // if we have DB_STATE_REMOVED we already have reset the BuildingInfo for this plot
        if (plot.db_state_building == DB_STATE_REMOVED)
        {
            plot.db_state_building = DB_STATE_UNCHANGED;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BUILDINGS_BY_PLOT);
            stmt->setUInt64(0, _owner->GetGUIDLow());
            stmt->setUInt32(1, plot.BuildingInfo.PacketInfo->GarrPlotInstanceID);
            trans->Append(stmt);
            continue;
        }

        if (plot.BuildingInfo.PacketInfo)
        {
            auto buildingEntry = sGarrBuildingStore.LookupEntry(plot.BuildingInfo.PacketInfo->GarrBuildingID);
            if (!buildingEntry)
                continue;

            if (plot.db_state_building != DB_STATE_CHANGED && plot.db_state_building != DB_STATE_NEW)
                continue;

            plot.db_state_building = DB_STATE_UNCHANGED;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_BUILDINGS);
            stmt->setUInt64(0, _owner->GetGUIDLow());
            stmt->setUInt32(1, plot.BuildingInfo.PacketInfo->GarrPlotInstanceID);
            stmt->setUInt32(2, plot.BuildingInfo.PacketInfo->GarrBuildingID);
            stmt->setUInt64(3, plot.BuildingInfo.PacketInfo->TimeBuilt);
            stmt->setBool(4, plot.BuildingInfo.PacketInfo->Active);

            ss.str("");
            auto data = _buildingData.find(buildingEntry->BuildingType);
            if (data != _buildingData.end() && !data->second.empty())
                for (uint8 i = 0; i < MAX_BUILDING_SAVE_DATA; ++i)
                    ss << uint32(data->second[i]) << " ";

            stmt->setString(5, ss.str());
            trans->Append(stmt);
        }
    }

    for (int32 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        for (std::unordered_map<uint64, Follower>::iterator p = _followers[i].begin(), next; p != _followers[i].end(); p = next)
        {
            next = p;
            ++next;

            Follower& follower = p->second;

            if (follower.DbState == DB_STATE_CHANGED || follower.DbState == DB_STATE_NEW)
            {
                uint8 index = 0;
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_FOLLOWERS);
                stmt->setUInt64(index++, follower.PacketInfo.DbID);
                stmt->setUInt64(index++, _owner->GetGUIDLow());
                stmt->setUInt32(index++, follower.PacketInfo.GarrFollowerID);
                stmt->setUInt32(index++, follower.PacketInfo.Quality);
                stmt->setUInt32(index++, follower.PacketInfo.FollowerLevel);
                stmt->setUInt32(index++, follower.PacketInfo.ItemLevelWeapon);
                stmt->setUInt32(index++, follower.PacketInfo.ItemLevelArmor);
                stmt->setUInt32(index++, follower.PacketInfo.Xp);
                stmt->setUInt32(index++, follower.PacketInfo.CurrentBuildingID);
                stmt->setUInt32(index++, follower.PacketInfo.CurrentMissionID);
                stmt->setUInt32(index++, follower.PacketInfo.FollowerStatus);
                stmt->setUInt16(index++, follower.PacketInfo.Vitality);
                trans->Append(stmt);
                follower.DbState = DB_STATE_UNCHANGED;
            }
            else if (follower.DbState == DB_STATE_REMOVED)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWER);
                stmt->setUInt64(0, follower.PacketInfo.DbID);
                trans->Append(stmt);

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWER_ABILITIES);
                stmt->setUInt64(0, follower.PacketInfo.DbID);
                trans->Append(stmt);

                _followers[i].erase(p);
                continue;
            }

            if (follower.db_state_ability != DB_STATE_CHANGED && follower.db_state_ability != DB_STATE_NEW)
                continue;

            uint8 slot = 0;
            for (auto& abilityId : follower.PacketInfo.AbilityID)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_FOLLOWER_ABILITIES);
                stmt->setUInt64(0, follower.PacketInfo.DbID);
                stmt->setUInt32(1, abilityId);
                stmt->setUInt8(2, slot++);
                trans->Append(stmt);
            }
            follower.db_state_ability = DB_STATE_UNCHANGED;
        }
    }

    for (int32 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        for (auto &m : _missions[i])
        {
            Mission &mission = m.second;
            uint8 index = 0;

            if (mission.DbState != DB_STATE_NEW && mission.DbState != DB_STATE_CHANGED)
                continue;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_MISSIONS);
            stmt->setUInt64(index++, mission.PacketInfo.DbID);
            stmt->setUInt64(index++, _owner->GetGUIDLow());
            stmt->setUInt32(index++, mission.PacketInfo.RecID);
            stmt->setUInt32(index++, mission.PacketInfo.OfferTime);
            stmt->setUInt32(index++, mission.PacketInfo.OfferDuration);
            stmt->setUInt32(index++, mission.PacketInfo.StartTime);
            stmt->setUInt32(index++, mission.PacketInfo.TravelDuration);
            stmt->setUInt32(index++, mission.PacketInfo.Duration);
            stmt->setUInt32(index++, mission.PacketInfo.State);
            stmt->setUInt16(index++, mission.PacketInfo.SuccesChance);
            trans->Append(stmt);
            mission.DbState = DB_STATE_UNCHANGED;
        }
    }

    for (auto &x : _shipments)
    {
        for (auto &shipment : x.second)
        {
            if (shipment.DbState != DB_STATE_NEW && shipment.DbState != DB_STATE_CHANGED)
                continue;

            uint8 index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_SHIPMENTS);

            stmt->setUInt64(index++, shipment.ShipmentID);
            stmt->setUInt64(index++, _owner->GetGUIDLow());
            stmt->setUInt16(index++, shipment.ShipmentRecID);
            stmt->setUInt32(index++, shipment.CreationTime);
            stmt->setUInt32(index++, shipment.end);

            trans->Append(stmt);
            shipment.DbState = DB_STATE_UNCHANGED;
        }
    }

    for (auto &data : _classHallTalentStore)
    {
        if (data.DbState == DB_STATE_UNCHANGED)
            continue;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_GARRISON_TALENTS);

        uint8 index = 0;
        stmt->setUInt64(index++, _owner->GetGUIDLow());
        stmt->setUInt32(index++, data.GarrTalentID);
        stmt->setUInt32(index++, data.ResearchStartTime);
        stmt->setUInt32(index++, data.Flags);
        trans->Append(stmt);
        data.DbState = DB_STATE_UNCHANGED;
    }
}

void Garrison::DeleteFromDB(ObjectGuid::LowType ownerGuid, SQLTransaction const& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BLUEPRINTS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BUILDINGS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWERS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_MISSIONS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_SHIPMENTS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_TALENTS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);
}

bool Garrison::Create(uint32 garrSiteId, bool skip_teleport /* = false*/)
{
    uint8 garType = getGarrisoneTypeBySite(garrSiteId);

    if (_siteLevel[garType])
        return false;

    auto siteLevel = sGarrisonMgr.GetGarrSiteLevelEntry(garrSiteId, 1);
    if (!siteLevel)
        return false;

    _siteLevel[garType] = siteLevel;
    InitializePlots(garType);

    WorldPackets::Garrison::GarrisonCreateResult garrisonCreateResult;
    garrisonCreateResult.GarrSiteLevelID = siteLevel->ID;
    _owner->SendDirectMessage(garrisonCreateResult.Write());

    //_owner->GetPhaseMgr().RemoveUpdateFlag(PHASE_UPDATE_FLAG_AREA_UPDATE); update phase send at quest credit.
    if (siteLevel->UpgradeMovieID && !skip_teleport)
        _owner->TeleportTo(GetGarrisonMapID(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ(), _owner->GetOrientation(), TELE_TO_SEAMLESS);
    SendRemoteInfo();
    return true;
}

void Garrison::Delete()
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    DeleteFromDB(_owner->GetGUIDLow(), trans);
    CharacterDatabase.CommitTransaction(trans);

    for (uint8 i = GARRISON_TYPE_GARRISON; i <= GARRISON_TYPE_CLASS_ORDER; ++i)
    {
        if (!_siteLevel[i])
            continue;

        WorldPackets::Garrison::GarrisonDeleteResult garrisonDelete;
        garrisonDelete.Result = GARRISON_SUCCESS;
        garrisonDelete.GarrSiteID = _siteLevel[i]->GarrSiteID;
        _owner->SendDirectMessage(garrisonDelete.Write());
    }
}

void Garrison::InitializePlots(uint8 type)
{
    if (!_siteLevel[type])
        return;

    if (std::vector<GarrSiteLevelPlotInstEntry const*> const* plots = sGarrisonMgr.GetGarrPlotInstForSiteLevel(_siteLevel[type]->ID))
    {
        for (auto plot : *plots)
        {
            uint32 garrPlotInstanceId = plot->GarrPlotInstanceID;
            GarrPlotInstanceEntry const* plotInstance = sGarrPlotInstanceStore.LookupEntry(garrPlotInstanceId);
            GameObjectsEntry const* gameObject = sGarrisonMgr.GetPlotGameObject(_siteLevel[type]->MapID, garrPlotInstanceId);
            if (!plotInstance || !gameObject)
                continue;

            auto plotEntry = sGarrPlotStore.LookupEntry(plotInstance->GarrPlotID);
            if (!plotEntry)
                continue;

            Plot& plotInfo = _plots[garrPlotInstanceId];
            plotInfo.PacketInfo.GarrPlotInstanceID = garrPlotInstanceId;

            float orientation = 2 * std::acos(gameObject->Rotation.O);

            if (_siteLevel[type]->GarrSiteID == SITE_ID_GARRISON_ALLIANCE)
            {
                if (garrPlotInstanceId == 18)
                    orientation = 4.991641f;
                if (garrPlotInstanceId == 22)
                    orientation = 4.799657f;
                if (garrPlotInstanceId == 24)
                    orientation = 5.628688f;
            }
            else if (_siteLevel[type]->GarrSiteID == SITE_ID_GARRISON_HORDE)
            {
                if (garrPlotInstanceId == 18)
                    orientation = 2.042035f;
                if (garrPlotInstanceId == 19)
                    orientation = 3.534301f;
                if (garrPlotInstanceId == 20)
                    orientation = 0.802851f;
            }

            plotInfo.PacketInfo.PlotPos = Position(gameObject->Position.X, gameObject->Position.Y, gameObject->Position.Z, orientation);
            plotInfo.RotationX = gameObject->Rotation.X;
            plotInfo.RotationY = gameObject->Rotation.Y;
            plotInfo.RotationZ = gameObject->Rotation.Z;
            plotInfo.RotationW = gameObject->Rotation.O;

            plotInfo.PacketInfo.PlotType = plotEntry->PlotType;
            plotInfo.EmptyGameObjectId = gameObject->ID;
            plotInfo.GarrSiteLevelPlotInstId = plot->ID;
        }
    }
}

//! ToDo. site.
void Garrison::Upgrade()
{
    auto site = _siteLevel[GARRISON_TYPE_GARRISON];
    if (!site)
        return;

    WorldPackets::Garrison::GarrisonUpgradeResult result;

    uint32 canUpgradeResult = CanUpgrade(_owner, site);
    if (canUpgradeResult != GARRISON_SUCCESS)
    {
        result.Result = canUpgradeResult;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    GarrSiteLevelEntry const* newSiteLevel = nullptr;
    for (GarrSiteLevelEntry const* v : sGarrSiteLevelStore)
        if (v->GarrSiteID == site->GarrSiteID && v->GarrLevel == site->GarrLevel + 1)
            newSiteLevel = v;

    if (newSiteLevel && newSiteLevel->GarrLevel == site->GarrLevel)
    {
        result.Result = GARRISON_ERROR_NO_BUILDING;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    // cost taken from old site, not from new
    if (site->UpgradeCost && !_owner->HasCurrency(CURRENCY_TYPE_GARRISON_RESOURCES, site->UpgradeCost))
    {
        result.Result = GARRISON_ERROR_NOT_ENOUGH_CURRENCY;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    // cost taken from old site, not from new
    if (site->UpgradeGoldCost && !_owner->HasEnoughMoney(uint64(site->UpgradeGoldCost) * GOLD))
    {
        result.Result = GARRISON_ERROR_NOT_ENOUGH_GOLD;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    const auto oldSite = site;

    //Set new site level.
    _siteLevel[GARRISON_TYPE_GARRISON] = newSiteLevel;
    site = newSiteLevel;

    InitializePlots(GARRISON_TYPE_GARRISON);

    switch (site->ID)
    {
        //< alliance
        case 444:   // 2 lvl
            _owner->TeleportTo(site->MapID, 1779.97f, 199.439f, 70.8068f, 0.3276943f, TELE_TO_SEAMLESS);
            break;
        case 6:     // 3 lvl
            _owner->TeleportTo(site->MapID, 1733.25f, 156.995f, 75.40f, 0.79f, TELE_TO_SEAMLESS);
            break;
            //< horde
        case 445:   // 2 lvl
        case 259:   // 3 lvl
            _owner->TeleportTo(site->MapID, 5756.79f, 4493.11f, 132.311f, 2.862183f, TELE_TO_SEAMLESS);
            break;
        default:
            _owner->TeleportTo(site->MapID, _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ(), _owner->GetOrientation(), TELE_TO_SEAMLESS);
            break;
    }

    WorldPackets::Misc::StreamingMovie movie;
    movie.MovieIDs.push_back(site->UpgradeMovieID);
    _owner->SendDirectMessage(movie.Write());

    result.Result = GARRISON_SUCCESS;
    result.GarrSiteLevelID = site->ID;
    _owner->SendDirectMessage(result.Write());

    if (oldSite->UpgradeCost)
        _owner->ModifyCurrency(CURRENCY_TYPE_GARRISON_RESOURCES, -oldSite->UpgradeCost, false, true);

    if (oldSite->UpgradeGoldCost)
        _owner->ModifyMoney(int64(oldSite->UpgradeGoldCost) * GOLD * -1, false);

    // complete Bigger is Better or Building my own Castle/Fortress
    if (oldSite->GarrLevel == 1)
        _owner->AchieveCriteriaCredit(_owner->GetTeam() == ALLIANCE ? 38378 : 38354);
    if (oldSite->GarrLevel == 2)
        _owner->AchieveCriteriaCredit(_owner->GetTeam() == ALLIANCE ? 38529 : 38531);

    // Save us
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_GARRISON);
    stmt->setUInt32(0, newSiteLevel->ID);
    stmt->setUInt64(1, _owner->GetGUIDLow());
    stmt->setUInt32(2, oldSite->ID);
    CharacterDatabase.Execute(stmt);

    SendRemoteInfo();
}

//! ToDo: site.
void Garrison::Enter() const
{
    auto site = _siteLevel[GARRISON_TYPE_GARRISON];
    if (!site)
        return;

    WorldLocation loc(site->MapID);
    loc.Relocate(_owner);
    _owner->TeleportTo(loc, TELE_TO_SEAMLESS);
}

//! ToDo: site.
void Garrison::Leave() const
{
    auto site = _siteLevel[GARRISON_TYPE_GARRISON];
    if (!site)
        return;

    if (MapEntry const* map = sMapStore.LookupEntry(site->MapID))
    {
        WorldLocation loc(map->ParentMapID);
        loc.Relocate(_owner);
        _owner->TeleportTo(loc, TELE_TO_SEAMLESS);
    }
}

void Garrison::Update(uint32 diff)
{
    updateTimer.Update(diff);
    if (!updateTimer.Passed())
        return;

    updateTimer.Reset();

    for (std::pair<const uint32, Plot>& plot : _plots)
    {
        if (plot.second.BuildingInfo.PacketInfo && !plot.second.BuildingInfo.PacketInfo->Active && !plot.second.BuildingInfo.FinalizerGuid && plot.second.BuildingInfo.CanActivate())
        {
            if (FinalizeGarrisonPlotGOInfo const* finalizeInfo = sGarrisonMgr.GetPlotFinalizeGOInfo(plot.second.PacketInfo.GarrPlotInstanceID))
            {
                if (Map* map = FindMap())
                {
                    Position const& pos2 = finalizeInfo->FactionInfo[GetFaction()].Pos;
                    GameObject* finalizer = sObjectMgr->IsStaticTransport(finalizeInfo->FactionInfo[GetFaction()].GameObjectId) ? new StaticTransport : new GameObject;
                    if (finalizer->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), finalizeInfo->FactionInfo[GetFaction()].GameObjectId, map, 1,
                        pos2, G3D::Matrix3::fromEulerAnglesZYX(pos2.GetOrientation(), 0.0f, 0.0f), 255, GO_STATE_READY) && finalizer->IsPositionValid() && map->AddToMap(finalizer))
                    {
                        // set some spell id to make the object delete itself after use
                        finalizer->SetSpellId(finalizer->GetGOInfo()->goober.spell);

                        finalizer->SetRespawnTime(0);
                        if (uint16 animKit = finalizeInfo->FactionInfo[GetFaction()].AnimKitId)
                            finalizer->SetAnimKitId(animKit, true);

                        plot.second.BuildingInfo.FinalizerGuid = finalizer->GetGUID();
                    }
                    else
                        delete finalizer;
                }
            }
        }
    }

    for (auto data : _shipments)
    {
        for (WorldPackets::Garrison::Shipment &ship_data : _shipments[data.first])
        {
            if (ship_data.finished || ship_data.end > time(nullptr))
                continue;

            ship_data.finished = true;
            //! update near go GAMEOBJECT_TYPE_GARRISON_SHIPMENT
            std::list<GameObject*> gameobjectList;
            _owner->GetNearGameObjectListInGrid(gameobjectList, _owner->GetVisibilityRange());
            for (auto go : gameobjectList)
            {
                if (go->GetGOInfo()->type == GAMEOBJECT_TYPE_GARRISON_SHIPMENT)
                {
                    go->SetGoState(GO_STATE_READY);
                    go->ForceValuesUpdateAtIndex(GAMEOBJECT_FIELD_DISPLAY_ID);
                    go->ForceValuesUpdateAtIndex(GAMEOBJECT_FIELD_SPELL_VISUAL_ID);
                }
            }
        }
    }

    if (time(nullptr) > _MissionGen)
    {
        _MissionGen = time(nullptr) + DAY;
        GenerateRandomMission();
    }
}

uint32 Garrison::GoBuildValuesUpdate(GameObject const* go, uint16 index, uint32 value)
{
    //! speed up work.
    if (index != GAMEOBJECT_FIELD_DISPLAY_ID && index != GAMEOBJECT_FIELD_SPELL_VISUAL_ID)
        return value;

    auto shipmentConteinerEntry = sCharShipmentContainerStore.LookupEntry(go->GetGOInfo()->garrisonShipment.ShipmentContainer);
    if (!shipmentConteinerEntry)
        return value;

    bool ready = false;
    for (WorldPackets::Garrison::Shipment &ship_data : _shipments[idxShipment(shipmentConteinerEntry)])
        if (ship_data.finished)
            ready = true;

    switch (index)
    {
        case GAMEOBJECT_FIELD_DISPLAY_ID:
            if (!ready)
                return value;
            return _owner->GetTeam() == HORDE ? shipmentConteinerEntry->MediumDisplayInfoID : shipmentConteinerEntry->SmallDisplayInfoID;
        case GAMEOBJECT_FIELD_SPELL_VISUAL_ID:
            if (ready && shipmentConteinerEntry->WorkingSpellVisualID)
                return shipmentConteinerEntry->WorkingSpellVisualID;
            return 0;
        default:
            break;
    }
    //! never comes here.
    return value;
}

GarrisonFactionIndex Garrison::GetFaction() const
{
    return _owner->GetTeam() == HORDE ? GARRISON_FACTION_INDEX_HORDE : GARRISON_FACTION_INDEX_ALLIANCE;
}

std::vector<Plot*> Garrison::GetPlots()
{
    std::vector<Plot*> plots;
    plots.reserve(_plots.size());
    for (auto& p : _plots)
        plots.push_back(&p.second);

    return plots;
}

Plot* Garrison::GetPlot(uint32 garrPlotInstanceId)
{
    return Trinity::Containers::MapGetValuePtr(_plots, garrPlotInstanceId);
}

Plot const* Garrison::GetPlot(uint32 garrPlotInstanceId) const
{
    return Trinity::Containers::MapGetValuePtr(_plots, garrPlotInstanceId);
}

Plot* Garrison::GetPlotWithBuildingType(uint32 BuildingTypeID)
{
    for (auto &p : _plots)
        if (p.second.BuildingInfo.PacketInfo && sGarrBuildingStore.AssertEntry(p.second.BuildingInfo.PacketInfo->GarrBuildingID)->BuildingType == BuildingTypeID)
            return &p.second;
    return nullptr;
}

Plot* Garrison::GetPlotWithNpc(uint32 entry)
{
    for (auto &p : _plots)
        for (auto& guid : p.second.BuildingInfo.Spawns)
            if (guid.GetEntry() == entry)
                return &p.second;
    return nullptr;
}

bool Garrison::LearnBlueprint(uint32 garrBuildingId)
{
    WorldPackets::Garrison::GarrisonLearnBlueprintResult learnBlueprintResult;
    learnBlueprintResult.GarrTypeID = GARRISON_TYPE_GARRISON;
    learnBlueprintResult.BuildingID = garrBuildingId;
    learnBlueprintResult.Result = GARRISON_SUCCESS;

    if (!sGarrBuildingStore.LookupEntry(garrBuildingId))
        learnBlueprintResult.Result = GARRISON_ERROR_INVALID_BUILDINGID;
    else if (_knownBuildings.count(garrBuildingId))
        learnBlueprintResult.Result = GARRISON_ERROR_BLUEPRINT_EXISTS;
    else
    {
        _knownBuildings.insert(garrBuildingId);
        _buildingsToSave.insert(garrBuildingId);
    }

    _owner->SendDirectMessage(learnBlueprintResult.Write());

    if (learnBlueprintResult.Result != GARRISON_SUCCESS)
        return false;
    return true;
}

void Garrison::UnlearnBlueprint(uint32 garrBuildingId)
{
    WorldPackets::Garrison::GarrisonUnlearnBlueprintResult unlearnBlueprintResult;
    unlearnBlueprintResult.GarrTypeID = GARRISON_TYPE_GARRISON;
    unlearnBlueprintResult.BuildingID = garrBuildingId;
    unlearnBlueprintResult.Result = GARRISON_SUCCESS;

    if (!sGarrBuildingStore.LookupEntry(garrBuildingId))
        unlearnBlueprintResult.Result = GARRISON_ERROR_INVALID_BUILDINGID;
    else if (!_knownBuildings.count(garrBuildingId))
        unlearnBlueprintResult.Result = GARRISON_ERROR_REQUIRES_BLUEPRINT;
    else
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BLUEPRINTS_BY_BUILD);
        stmt->setUInt64(0, _owner->GetGUIDLow());
        stmt->setUInt32(1, garrBuildingId);
        CharacterDatabase.Execute(stmt);

        _knownBuildings.erase(garrBuildingId);
    }

    _owner->SendDirectMessage(unlearnBlueprintResult.Write());
}

void Garrison::Swap(uint32 plot1, uint32 plot2)
{
    uint32 BuildingId1 = 0, BuildingId2 = 0;
    Plot* p1 = GetPlot(plot1);
    if (p1->BuildingInfo.PacketInfo)
        BuildingId1 = p1->BuildingInfo.PacketInfo->GarrBuildingID;

    Plot* p2 = GetPlot(plot2);
    if (p2->BuildingInfo.PacketInfo)
        BuildingId2 = p2->BuildingInfo.PacketInfo->GarrBuildingID;

    if (BuildingId1)
    {
        //clear plot1
        WorldPackets::Garrison::GarrisonBuildingRemoved buildingRemoved;
        buildingRemoved.Result = GARRISON_SUCCESS;
        buildingRemoved.GarrPlotInstanceID = plot1;
        buildingRemoved.GarrBuildingID = BuildingId1;
        _owner->SendDirectMessage(buildingRemoved.Write());

        if (Map* map = FindMap())
        {
            p1->DeleteGameObject(map);
            p1->ClearBuildingInfo(_owner);
            if (GameObject* go = p1->CreateGameObject(map, GetFaction(), this))
                map->AddToMap(go);
        }
        // set on plot2
        PlaceBuilding(plot2, BuildingId1, true, true);
    }

    if (BuildingId2)
    {
        if (!BuildingId1)
        {
            //clear plot2
            WorldPackets::Garrison::GarrisonBuildingRemoved buildingRemoved;
            buildingRemoved.Result = GARRISON_SUCCESS;
            buildingRemoved.GarrPlotInstanceID = plot2;
            buildingRemoved.GarrBuildingID = BuildingId2;
            _owner->SendDirectMessage(buildingRemoved.Write());

            if (Map* map = FindMap())
            {
                p2->DeleteGameObject(map);
                p2->ClearBuildingInfo(_owner);
                if (GameObject* go = p2->CreateGameObject(map, GetFaction(), this))
                    map->AddToMap(go);
            }
        }
        // set on plot1
        PlaceBuilding(plot1, BuildingId2, true, true);
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    SaveToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

void Garrison::PlaceBuilding(uint32 garrPlotInstanceId, uint32 garrBuildingId, bool quest/* = false*/, bool swap/* = false*/)
{
    WorldPackets::Garrison::GarrisonPlaceBuildingResult placeBuildingResult;
    placeBuildingResult.GarrTypeID = GARRISON_TYPE_GARRISON;
    placeBuildingResult.Result = CheckBuildingPlacement(garrPlotInstanceId, garrBuildingId, quest);
    if (placeBuildingResult.Result == GARRISON_SUCCESS)
    {
        placeBuildingResult.BuildingInfo.GarrPlotInstanceID = garrPlotInstanceId;
        placeBuildingResult.BuildingInfo.GarrBuildingID = garrBuildingId;
        placeBuildingResult.BuildingInfo.TimeBuilt = time(nullptr);

        Plot* plot = GetPlot(garrPlotInstanceId);

        uint32 oldBuildingId = 0;
        Map* map = FindMap();
        GarrBuildingEntry const* buildingEntry = sGarrBuildingStore.AssertEntry(garrBuildingId);
        if (map)
        {
            if (InstanceMap* m = map->ToInstanceMap())
                if (InstanceScript* inst = m->GetInstanceScript())
                    inst->OnPlaceBuilding(_owner, this, garrBuildingId, garrPlotInstanceId, placeBuildingResult.BuildingInfo.TimeBuilt);

            plot->DeleteGameObject(map);
        }

        if (plot->BuildingInfo.PacketInfo)
        {
            oldBuildingId = plot->BuildingInfo.PacketInfo->GarrBuildingID;
            if (sGarrBuildingStore.AssertEntry(oldBuildingId)->BuildingType != buildingEntry->BuildingType)
                plot->ClearBuildingInfo(_owner);
        }

        // If build by quest - skip building state and spawn building.
        if (quest)
        {
            placeBuildingResult.PlayActivationCinematic = !swap;
            placeBuildingResult.BuildingInfo.Active = true;
        }
        else
        {
            if (buildingEntry->CurrencyQty)
                _owner->ModifyCurrency(buildingEntry->CurrencyTypeID, -buildingEntry->CurrencyQty, false, true);

            if (buildingEntry->GoldCost)
                _owner->ModifyMoney(int64(buildingEntry->GoldCost) * GOLD * -1, false);
        }

        plot->SetBuildingInfo(placeBuildingResult.BuildingInfo, _owner);
        plot->db_state_building = DB_STATE_CHANGED;
        if (map)
        {
            if (GameObject* go = plot->CreateGameObject(map, GetFaction(), this))
                map->AddToMap(go);
        }

        if (oldBuildingId)
        {
            WorldPackets::Garrison::GarrisonBuildingRemoved buildingRemoved;
            buildingRemoved.GarrTypeID = GARRISON_TYPE_GARRISON;
            buildingRemoved.Result = GARRISON_SUCCESS;
            buildingRemoved.GarrPlotInstanceID = garrPlotInstanceId;
            buildingRemoved.GarrBuildingID = oldBuildingId;
            _owner->SendDirectMessage(buildingRemoved.Write());
        }

        _owner->UpdateAchievementCriteria(CRITERIA_TYPE_PLACE_GARRISON_BUILDING, garrBuildingId);
    }

    _owner->SendDirectMessage(placeBuildingResult.Write());
}

void Garrison::CancelBuildingConstruction(uint32 garrPlotInstanceId)
{
    WorldPackets::Garrison::GarrisonBuildingRemoved buildingRemoved;
    buildingRemoved.GarrTypeID = GARRISON_TYPE_GARRISON;
    buildingRemoved.Result = CheckBuildingRemoval(garrPlotInstanceId);
    if (buildingRemoved.Result == GARRISON_SUCCESS)
    {
        Plot* plot = GetPlot(garrPlotInstanceId);

        buildingRemoved.GarrPlotInstanceID = garrPlotInstanceId;
        buildingRemoved.GarrBuildingID = plot->BuildingInfo.PacketInfo->GarrBuildingID;

        Map* map = FindMap();
        if (map)
            plot->DeleteGameObject(map);

        plot->ClearBuildingInfo(_owner);
        _owner->SendDirectMessage(buildingRemoved.Write());

        GarrBuildingEntry const* constructing = sGarrBuildingStore.AssertEntry(buildingRemoved.GarrBuildingID);
        // Refund construction/upgrade cost
        _owner->ModifyCurrency(constructing->CurrencyTypeID, constructing->CurrencyQty, true, true);
        _owner->ModifyMoney(int64(constructing->GoldCost) * GOLD, false);

        if (constructing->UpgradeLevel > 1)
        {
            // Restore previous level building
            GarrBuildingEntry const* restored = sGarrisonMgr.GetPreviousLevelBuilding(constructing->BuildingType, constructing->UpgradeLevel);
            ASSERT(restored);

            WorldPackets::Garrison::GarrisonPlaceBuildingResult placeBuildingResult;
            placeBuildingResult.GarrTypeID = GARRISON_TYPE_GARRISON;
            placeBuildingResult.Result = GARRISON_SUCCESS;
            placeBuildingResult.BuildingInfo.GarrPlotInstanceID = garrPlotInstanceId;
            placeBuildingResult.BuildingInfo.GarrBuildingID = restored->ID;
            placeBuildingResult.BuildingInfo.TimeBuilt = time(nullptr);
            placeBuildingResult.BuildingInfo.Active = true;

            plot->SetBuildingInfo(placeBuildingResult.BuildingInfo, _owner);
            plot->db_state_building = DB_STATE_CHANGED;
            _owner->SendDirectMessage(placeBuildingResult.Write());
        }

        if (map)
            if (GameObject* go = plot->CreateGameObject(map, GetFaction(), this))
                map->AddToMap(go);
    }
    else
        _owner->SendDirectMessage(buildingRemoved.Write());
}

void Garrison::ActivateBuilding(uint32 garrPlotInstanceId)
{
    if (Plot* plot = GetPlot(garrPlotInstanceId))
    {
        if (plot->BuildingInfo.CanActivate() && plot->BuildingInfo.PacketInfo && !plot->BuildingInfo.PacketInfo->Active)
        {
            plot->BuildingInfo.PacketInfo->Active = true;
            plot->BuildingInfo.FinalizerGuid.Clear();

            if (Map* map = FindMap())
            {
                plot->DeleteGameObject(map);
                if (GameObject* go = plot->CreateGameObject(map, GetFaction(), this))
                    map->AddToMap(go);
            }

            WorldPackets::Garrison::GarrisonBuildingActivated buildingActivated;
            buildingActivated.GarrPlotInstanceID = garrPlotInstanceId;
            _owner->SendDirectMessage(buildingActivated.Write());

            _owner->UpdateAchievementCriteria(CRITERIA_TYPE_CONSTRUCT_GARRISON_BUILDING, plot->BuildingInfo.PacketInfo->GarrBuildingID);

            plot->db_state_building = DB_STATE_CHANGED;
            SendInfo();
        }
    }
}

uint32 Garrison::GetCountOfBluePrints() const
{
    return _knownBuildings.size();
}

uint32 Garrison::GetCountOFollowers() const
{
    uint8 type = _owner->GetMap()->GetEntry()->ExpansionID == 5 ? GARRISON_TYPE_GARRISON : GARRISON_TYPE_CLASS_ORDER;
    return _followers[type].size();
}

uint32 Garrison::GetBuildingData(uint32 buildingType, uint32 idx)
{
    return _buildingData[buildingType][idx];
}

void Garrison::SetBuildingData(uint32 buildingType, uint32 idx, uint32 value)
{
    _buildingData[buildingType][idx] = value;
}

void Garrison::ResetFollowerActivationLimit()
{
    _followerActivationsRemainingToday = 1;
}

int32 Garrison::GetGarrisonMapID() const
{
    return _siteLevel[GARRISON_TYPE_GARRISON] ? _siteLevel[GARRISON_TYPE_GARRISON]->MapID : -1;
}

uint8 Garrison::GetGarrisonLevel() const
{
    return _siteLevel[GARRISON_TYPE_GARRISON] ? _siteLevel[GARRISON_TYPE_GARRISON]->GarrLevel : 0;
}

bool Garrison::HasGarrison(GarrisonType type)
{
    return _siteLevel[type] != nullptr;
}

void Garrison::AddFollower(uint32 garrFollowerId)
{
    GarrFollowerEntry const* followerEntry = sGarrFollowerStore.LookupEntry(garrFollowerId);
    if (!followerEntry || (followerEntry->GarrTypeID != GARRISON_TYPE_GARRISON && followerEntry->GarrTypeID != GARRISON_TYPE_CLASS_ORDER))
        return;

    WorldPackets::Garrison::GarrisonAddFollowerResult addFollowerResult;
    if (followerEntry->ChrClassID > 0 && followerEntry->ChrClassID != _owner->getClass())
    {
        addFollowerResult.Result = GARRISON_ERROR_INVALID_CLASS;
        _owner->SendDirectMessage(addFollowerResult.Write());
        return;
    }

    if (followerEntry->Vitality) // Hack, for exist follwer
        _owner->UpdateAchievementCriteria(CRITERIA_TYPE_RECRUIT_GARRISON_TROOP, 1);

    _owner->UpdateAchievementCriteria(CRITERIA_TYPE_RECRUIT_TRANSPORT_FOLLOWER, garrFollowerId, 1);

    //! troop's can add more then one time.
    if (_followerIds[followerEntry->GarrTypeID].count(garrFollowerId) && !followerEntry->Vitality)
    {
        addFollowerResult.Result = GARRISON_ERROR_FOLLOWER_EXISTS;
        _owner->SendDirectMessage(addFollowerResult.Write());
        return;
    }
    addFollowerResult.GarrTypeID = followerEntry->GarrTypeID;

    _followerIds[followerEntry->GarrTypeID].insert(garrFollowerId);
    auto dbId = sGarrisonMgr.GenerateFollowerDbId();
    auto& follower = _followers[followerEntry->GarrTypeID][dbId];
    follower.DbState = DB_STATE_NEW;
    follower.db_state_ability = DB_STATE_NEW;
    follower.PacketInfo.DbID = dbId;
    follower.PacketInfo.GarrFollowerID = garrFollowerId;
    if (!followerEntry->Vitality && followerEntry->GarrFollowerTypeID != GarrisonConst::FollowerType::Follower)
        follower.PacketInfo.Quality = follower.RollQuality(followerEntry->Quality);
    else
        follower.PacketInfo.Quality = followerEntry->Quality;
    follower.PacketInfo.FollowerLevel = followerEntry->FollowerLevel;
    follower.PacketInfo.ItemLevelWeapon = followerEntry->ItemLevelWeapon;
    follower.PacketInfo.ItemLevelArmor = followerEntry->ItemLevelArmor;
    follower.PacketInfo.AbilityID = sGarrisonMgr.RollFollowerAbilities(followerEntry, follower.PacketInfo.Quality, GetFaction(), true);
    follower.PacketInfo.FollowerStatus = GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_BASE;
    follower.TypeID = followerEntry->GarrFollowerTypeID;

    if (followerEntry->Vitality)
    {
        follower.PacketInfo.Vitality = followerEntry->Vitality;
        follower.PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_TROOP;
        follower.PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_NO_XP_GAIN;

        //! increase troop counter.
        ASSERT(followerEntry->Vitality < 5);
        ++_troopCount[followerEntry->Vitality];

        // disable more then limit.
        uint32 limit = GetTroopLimit(followerEntry->Vitality, _owner->GetTeam() == ALLIANCE ? followerEntry->AllianceGarrClassSpecID : followerEntry->HordeGarrClassSpecID);
        if (_troopCount[followerEntry->Vitality] > limit)
            follower.PacketInfo.FollowerStatus |= GarrisonConst::GarrisonFollowerFlags::FOLLOWER_STATUS_INACTIVE;
    }

    addFollowerResult.Follower = follower.PacketInfo;
    _owner->SendDirectMessage(addFollowerResult.Write());

    _owner->UpdateAchievementCriteria(CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER, garrFollowerId);
    _owner->UpdateAchievementCriteria(CRITERIA_TYPE_RECRUIT_GARRISON_FOLLOWER_COUNT, garrFollowerId, _followerIds[followerEntry->GarrTypeID].count(garrFollowerId));

    if (auto mission = sGarrisonMgr.GetMissionAtFollowerTaking(garrFollowerId))
        AddMission(mission->ID);

    _owner->UpdateForQuestWorldObjects();
}

//! basic follower has rand 4.  for shaman look at http://ru.wowhead.com/champion=608#english-comments
void Garrison::CheckBasicRequirements()
{
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 q = _owner->GetQuestSlotQuestId(i);
        if (!q)
            continue;

        if (uint32 m = sGarrisonMgr.getMissionAtQuestTake(q))
            AddMission(m);
    }

    switch (_owner->getClass())
    {
        case CLASS_DEMON_HUNTER:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(722))
                AddFollower(722);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(595))
                AddFollower(595);
            break;
        case CLASS_DRUID:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(639))
                AddFollower(639);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(640))
                AddFollower(640);
            break;
        case CLASS_MONK:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(588))
                AddFollower(588);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(596))
                AddFollower(596);
            break;
        case CLASS_WARLOCK:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(616))
                AddFollower(616);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(589))
                AddFollower(589);
            break;
        case CLASS_SHAMAN:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(608))
                AddFollower(616);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(609))
                AddFollower(589);
            break;
        case CLASS_DEATH_KNIGHT:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(584))
                AddFollower(584);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(586))
                AddFollower(586);
            //599
            break;
        case CLASS_PRIEST:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(856))
                AddFollower(856);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(857))
                AddFollower(857);
            break;
        case CLASS_ROGUE:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(778))
                AddFollower(778);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(591))
                AddFollower(591);
            break;
        case CLASS_HUNTER:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(593))
                AddFollower(593);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(742))
                AddFollower(742);
            break;
        case CLASS_PALADIN:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(480))
                AddFollower(480);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(478))
                AddFollower(478);
            break;
        case CLASS_WARRIOR:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(708))
                AddFollower(708);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(709))
                AddFollower(709);
            break;
        case CLASS_MAGE:
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(716))
                AddFollower(716);
            if (!_followerIds[GARRISON_TYPE_CLASS_ORDER].count(717))
                AddFollower(717);
            break;
        default:
            break;
    }
}

void Garrison::AddMission(uint32 missionRecID, bool sendLog/* = true*/)
{
    auto missionEntry = sGarrMissionStore.LookupEntry(missionRecID);
    if (!missionEntry)
        return;

    WorldPackets::Garrison::GarrisonAddMissionResult addMissionResult;

    addMissionResult.GarrTypeID = missionEntry->GarrTypeID;

    if (_missionIds[missionEntry->GarrTypeID].count(missionRecID))
    {
        addMissionResult.Result = GARRISON_ERROR_FOLLOWER_EXISTS;
        if (sendLog)
            _owner->SendDirectMessage(addMissionResult.Write());
        return;
    }

    _missionIds[missionEntry->GarrTypeID].insert(missionRecID);
    uint64 dbId = sGarrisonMgr.GenerateMissionDbId();
    Mission& mission = _missions[missionEntry->GarrTypeID][dbId];
    mission.PacketInfo.DbID = dbId;
    mission.PacketInfo.RecID = missionRecID;
    mission.PacketInfo.OfferTime = time(nullptr);
    mission.PacketInfo.OfferDuration = 0;
    mission.PacketInfo.StartTime = 0;
    mission.PacketInfo.TravelDuration = 0;
    mission.PacketInfo.Duration = 0/*missionEntry->MissionDuration*/;
    mission.PacketInfo.State = MISSION_STATE_AVAILABLE;
    mission.PacketInfo.SuccesChance = 0;
    mission.PacketInfo.UnkInt2 = 0;
    mission.DbState = DB_STATE_NEW;

    addMissionResult.unk = 1;
    addMissionResult.MissionData = mission.PacketInfo;
    if (sendLog)
        _owner->SendDirectMessage(addMissionResult.Write());
}

void Garrison::GenerateRandomMission(uint16 count /*=0*/)
{
    for (uint8 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        uint16 lvlMax = GetMaxFolowerLvl(i);
        uint16 lvlMin = GetMinFolowerLvl(i);

        uint16 itemMaxLvl = GetMaxFolowerItemLvl(i);
        if (!lvlMax)
            continue;

        if (!count)
            count = _owner->getLevel() >= 99 ? 10 : 5;

        if (_missionIds[i].size() >= count)
            continue;

        uint32 check = 0;
        int32 l_ShuffleCount = std::rand() % 4;

        while (true)
        {
            if (++check > 200)
                break;

            int idx = rand() % sDB2Manager._garrMissionsMap[i].size();
            if (GarrMissionEntry const* mission = sDB2Manager._garrMissionsMap[i][idx])
            {
                //! 0x80 - quest missions.
                if (mission->GarrTypeID != i || /*mission->OfferDuration >= 9999999 || */mission->MissionDuration <= 10/* || mission->Flags & 0x80*/)
                    continue;

                if (_missionIds[i].count(mission->ID))
                    continue;

                if (mission->MaxFollowers > _followers[i].size())
                    continue;

                if (!ConditionMgr::IsPlayerMeetingCondition(_owner, mission->PlayerConditionID))
                    continue;

                if (mission->MaxFollowers > GarrisonConst::Globals::MaxFollowerPerMission)
                    continue;

                bool loc_check = true;
                switch (mission->UiTextureKitID)
                {
                    ///< Alliance garrison location.
                    case 106:
                        loc_check = _owner->GetTeamId() == TEAM_ALLIANCE;
                        break;
                    ///< Horde garrison location.
                    case 101:
                        loc_check = _owner->GetTeamId() == TEAM_HORDE;
                        break;
                    /// Not visible for client ????
                    case 0:
                        loc_check = false;
                        break;
                }

                /// Location check fail
                if (!loc_check)
                    continue;

                /// Max Level cap : 2
                if (mission->TargetLevel > static_cast<int32>(lvlMax + 2))
                    continue;

                if (mission->TargetItemLevel > static_cast<int32>(itemMaxLvl))
                    continue;

                /// We have less chances to get a low lvl mission if your followers' lowest lvl is higher
                if (mission->TargetLevel < lvlMin)
                {
                    if (urand(0, 100) <= 25)
                        continue;
                }

                //////////////////////////////////////////////
                ///---   GarrMissionTypeID check     ------///

                ///< class halls. possible added by quest.
                if (mission->GarrMissionTypeID == GarrisonConst::Mission::CH_Campaign)
                    continue;

                if (mission->GarrMissionTypeID == GarrisonConst::Mission::CH_Quest) // wrong name - it's related to raid quests in some variations, think should be disabled while no support
                    continue;
                
                ///< garrison. mission added after building barracks.
                if (mission->GarrMissionTypeID == GarrisonConst::Mission::Patrol && !GetPlotWithBuildingType(GARR_BTYPE_BARRACKS))
                    continue;

                if (!sGarrisonMgr.GetMissionRewardByRecID(mission->ID))
                    continue;

                AddMission(mission->ID);
                check = 0;

                //! shuffle count expire or num mission 
                if (l_ShuffleCount-- <= 0 || _missionIds[i].size() >= count)
                    break;
            }
        }
    }
}

uint16 Garrison::GetMaxFolowerLvl(uint8 i)
{
    uint16 lvl = 0;
    for (auto& v : _followers[i])
        if (v.second.PacketInfo.FollowerLevel > lvl)
            lvl = v.second.PacketInfo.FollowerLevel;

    return lvl;
}

uint16 Garrison::GetMinFolowerLvl(uint8 i)
{
    uint16 lvl = GarrisonConst::Globals::MaxFollowerLevelHall;
    for (auto& v : _followers[i])
        if (v.second.PacketInfo.FollowerLevel < lvl)
            lvl = v.second.PacketInfo.FollowerLevel;

    return lvl;
}

//! possible we should culc by minimum lvl. Now use max.
uint16 Garrison::GetMaxFolowerItemLvl(uint8 i)
{
    uint16 lvl = 0;
    for (auto& v : _followers[i])
        if (v.second.PacketInfo.ItemLevelWeapon > lvl || v.second.PacketInfo.ItemLevelArmor > lvl)
            lvl = v.second.PacketInfo.ItemLevelWeapon > v.second.PacketInfo.ItemLevelArmor ? v.second.PacketInfo.ItemLevelWeapon : v.second.PacketInfo.ItemLevelArmor;

    return lvl;
}

uint16 Garrison::GetCountFolowerItemLvl(uint32 minLevel)
{
    uint16 count = 0;
    for (auto& v : _followers[GARRISON_TYPE_GARRISON])
    {
        uint16 level = (v.second.PacketInfo.ItemLevelWeapon + v.second.PacketInfo.ItemLevelArmor) / 2;
        if (level >= minLevel)
            count++;
    }

    return count;
}

/*
https://www.icy-veins.com/wow/class-order-halls-troops
There is no limit to how many troops you can recruit, but there is a limit to how many can be active and alive at one time. The standard size of force is 3 lesser troops and 2 greater troops. For the special troops that the 4 classes listed above can make, there can only be 1 active at any time. The only way to increase these limits is via the Order Hall Advancement tiers, in which you can increase the limit by one in your 3rd tier.
*/
uint16 Garrison::GetTroopLimit(uint32 category, uint32 classSpec) const
{
    uint16 limit = GarrisonConst::TroopLimits[category];
    for (auto id : _abilities)
        for (GarrAbilityEffectEntry const* eff : *sDB2Manager.GetGarrEffect(id))
            if (eff->AbilityAction == GarrisonConst::AbilityEffectTypes::ModTroopNumber && classSpec == eff->ActionRecordID)
                limit += eff->ActionValueFlat;
    return limit;
}

void Garrison::ReTrainFollower(SpellInfo const* spellInfo, uint32 followerID)
{
    auto follower = GetFollowerByID(followerID);
    if (!follower)
        return;

    auto followerEntry = sGarrFollowerStore.LookupEntry(follower->PacketInfo.GarrFollowerID);
    if (!followerEntry)
        return;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWER_ABILITIES);
    stmt->setUInt64(0, follower->PacketInfo.DbID);
    CharacterDatabase.Execute(stmt);

    follower->PacketInfo.AbilityID = sGarrisonMgr.RollFollowerAbilities(followerEntry, follower->PacketInfo.Quality, GetFaction(), spellInfo && spellInfo->Effects[0]->MiscValue == 1 ? false : true);
    follower->db_state_ability = DB_STATE_NEW;

    WorldPackets::Garrison::GarrisonFollowerChangedAbilities followers;
    followers.Follower = follower->PacketInfo;
    _owner->SendDirectMessage(followers.Write());
}

void Garrison::ChangeFollowerVitality(SpellInfo const* spellInfo, uint32 followerID)
{
    auto follower = GetFollowerByID(followerID);
    if (!follower)
        return;

    auto followerEntry = sGarrFollowerStore.LookupEntry(follower->PacketInfo.GarrFollowerID);
    if (!followerEntry)
        return;

    if (followerEntry->GarrFollowerTypeID != spellInfo->Effects[0]->MiscValueB)
        return;

    auto addedVitality = spellInfo->Effects[0]->BasePoints;
    follower->PacketInfo.Vitality += addedVitality;
    follower->db_state_ability = DB_STATE_NEW;

    WorldPackets::Garrison::GarrisonFollowerChangedDurability packet;
    packet.Follower = follower->PacketInfo;
    packet.UnkInt = addedVitality;
    _owner->SendDirectMessage(packet.Write());
}

Mission const* Garrison::GetMission(uint64 dbId) const
{
    for (int32 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        auto itr = _missions[i].find(dbId);
        if (itr != _missions[i].end())
            return &itr->second;
    }

    return nullptr;
}

Mission* Garrison::GetMissionByRecID(uint32 missionRecID)
{
    for (int32 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        for (auto& m : _missions[i])
            if (m.second.PacketInfo.RecID == missionRecID)
                return &m.second;
    }

    return nullptr;
}

void Garrison::RemoveMissionByGuid(uint64 DbID)
{
    for (int32 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        for (auto itr = _missions[i].begin(); itr != _missions[i].end(); ++itr)
            if (itr->second.PacketInfo.DbID == DbID)
            {
                _missionIds[i].erase(itr->second.PacketInfo.RecID);
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_MISSIONS_DB_ID);
                stmt->setUInt64(0, DbID);
                CharacterDatabase.Execute(stmt);

                _missions[i].erase(itr);
                return;
            }
    }
}

void Garrison::SendInfo()
{
    uint8 type = _owner->GetMap()->GetEntry()->ExpansionID == 5 ? GARRISON_TYPE_GARRISON : GARRISON_TYPE_CLASS_ORDER;
    if (!_siteLevel[type])
        return;

    WorldPackets::Garrison::GetGarrisonInfoResult garrisonInfo;
    garrisonInfo.FactionIndex = GetFaction();
    garrisonInfo.Garrisons.emplace_back();

    garrisonInfo.FollowerSoftCaps.emplace_back(1, 25); // has baracks? fuck it, dosntwork at all
    garrisonInfo.FollowerSoftCaps.emplace_back(2, 10); // @hasTallent 
    garrisonInfo.FollowerSoftCaps.emplace_back(4, 5);

    WorldPackets::Garrison::GarrisonInfo& garrison = garrisonInfo.Garrisons.back();
    garrison.GarrTypeID = getGarrisoneTypeBySite(_siteLevel[type]->GarrSiteID);
    garrison.GarrSiteID = _siteLevel[type]->GarrSiteID;
    garrison.GarrSiteLevelID = _siteLevel[type]->ID;
    garrison.NumFollowerActivationsRemaining = _followerActivationsRemainingToday;

    if (type == GARRISON_TYPE_CLASS_ORDER)
        for (auto &t : _classHallTalentStore)
            garrison.Talents.push_back(t);

    for (auto& p : _plots)
    {
        Plot& plot = p.second;
        garrison.Plots.push_back(&plot.PacketInfo);
        if (plot.BuildingInfo.PacketInfo)
            garrison.Buildings.push_back(plot.BuildingInfo.PacketInfo.get_ptr());
    }

    for (auto const& p : _followers[type])
    {
        garrison.Followers.push_back(&p.second.PacketInfo);

        WorldPackets::Garrison::GarrisonFollowerChangedAbilities followers;
        followers.Follower = p.second.PacketInfo;
        _owner->SendDirectMessage(followers.Write());
    }

    if (!_missions[type].empty())
    {
        garrison.MissionOvermaxRewards.reserve(_missions[type].size());
        garrison.Missions.reserve(_missions[type].size());
        garrison.MissionRewards.reserve(_missions[type].size());
        garrison.CanStartMission.reserve(_missions[type].size());
        //! rewarded missions.
        //garrison.ArchivedMissions.reserve(_missions[type].size());
    }

    for (auto const& i : _missions[type])
    {
        garrison.Missions.push_back(&i.second.PacketInfo);
        garrison.CanStartMission.push_back(i.second.PacketInfo.State == MISSION_STATE_AVAILABLE);
        //garrison.ArchivedMissions.push_back(i.second.PacketInfo.RecID);

        //! Now we support only one reward at time. 1 item + 1 cur  + 1 bonusAbility. Is really need more? But this way is really easy.
        garrison.MissionRewards.emplace_back();
        auto& list = garrison.MissionRewards.back();
        list.emplace_back();
        auto& rew = list.back();

        if (GarrMissionRewardEntry const* entry = sGarrisonMgr.GetMissionRewardByRecID(i.second.PacketInfo.RecID))
        {
            rew.FollowerXP = entry->RewardXP;
            rew.ItemID = entry->RewardItemID;
            rew.ItemQuantity = entry->ItemAmount;
            rew.CurrencyID = entry->CurrencyID;
            rew.CurrencyQuantity = entry->CurrencyValue;
            rew.BonusAbilityID = entry->BonusAbilityID;   //ToDo: find correct.
            rew.ItemFileDataID = entry->Unknown;
        }
        else
        {
            rew.FollowerXP = 0;
            rew.ItemID = 0;
            rew.ItemQuantity = 0;
            rew.CurrencyID = 0;
            rew.CurrencyQuantity = 1000;
            rew.BonusAbilityID = 0;   //ToDo: find correct.
            rew.ItemFileDataID = 0;
        }

        garrison.MissionOvermaxRewards.emplace_back();
        if (GarrMissionRewardEntry const* entry = sGarrisonMgr.GetMissionOwermaxRewardByRecID(i.second.PacketInfo.RecID))
        {
            auto& list_ower = garrison.MissionOvermaxRewards.back();
            list_ower.emplace_back();

            auto& rew_owermax = list_ower.back();
            rew_owermax.FollowerXP = entry->RewardXP;
            rew_owermax.ItemID = entry->RewardItemID;
            rew_owermax.ItemQuantity = entry->ItemAmount;
            rew_owermax.CurrencyID = entry->CurrencyID;
            rew_owermax.CurrencyQuantity = entry->CurrencyValue;
            rew_owermax.BonusAbilityID = entry->BonusAbilityID;   //ToDo: find correct.
            rew_owermax.ItemFileDataID = entry->Unknown;
        }
    }

    _owner->SendDirectMessage(garrisonInfo.Write());
}

void Garrison::SendRemoteInfo() const
{
    uint8 type = _owner->GetMap()->GetEntry()->ExpansionID == 5 ? GARRISON_TYPE_GARRISON : GARRISON_TYPE_CLASS_ORDER;

    if (!_siteLevel[type])
        return;

    MapEntry const* garrisonMap = sMapStore.LookupEntry(_siteLevel[type]->MapID);
    if (!garrisonMap || int32(_owner->GetMapId()) != garrisonMap->ParentMapID)
        return;

    WorldPackets::Garrison::GarrisonRemoteInfo remoteInfo;

    WorldPackets::Garrison::GarrisonRemoteSiteInfo remoteSiteInfo;
    remoteSiteInfo.GarrSiteLevelID = _siteLevel[type]->ID;
    for (auto const& p : _plots)
        if (p.second.BuildingInfo.PacketInfo)
            remoteSiteInfo.Buildings.emplace_back(p.first, p.second.BuildingInfo.PacketInfo->GarrBuildingID);

    remoteInfo.Sites.push_back(remoteSiteInfo);

    _owner->SendDirectMessage(remoteInfo.Write());
}

void Garrison::SendBlueprintAndSpecializationData()
{
    WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationDataResult data;
    data.GarrTypeID = GARRISON_TYPE_GARRISON;
    data.BlueprintsKnown = &_knownBuildings;
    _owner->SendDirectMessage(data.Write());
}

void Garrison::SendBuildingLandmarks(Player* receiver) const
{
    WorldPackets::Garrison::GarrisonBuildingLandmarks buildingLandmarks;
    buildingLandmarks.Landmarks.reserve(_plots.size());

    for (auto const& p : _plots)
    {
        Plot const& plot = p.second;
        if (plot.BuildingInfo.PacketInfo)
            if (uint32 garrBuildingPlotInstId = sGarrisonMgr.GetGarrBuildingPlotInst(plot.BuildingInfo.PacketInfo->GarrBuildingID, plot.GarrSiteLevelPlotInstId))
                buildingLandmarks.Landmarks.emplace_back(garrBuildingPlotInstId, plot.PacketInfo.PlotPos);
    }

    receiver->SendDirectMessage(buildingLandmarks.Write());
}

uint32 Garrison::CanUpgrade(Player* receiver, GarrSiteLevelEntry const* site) const
{
    uint32 result = GARRISON_SUCCESS;
    switch (site->GarrLevel)
    {
        case 1: 
            // require bigger is better quest
            if (receiver->GetQuestStatus(receiver->GetTeam() == ALLIANCE ? 36592 : 36567) != QUEST_STATUS_INCOMPLETE)
                result = GARRISON_ERROR_UPGRADE_CONDITION_FAILED;
            break;
        case 2:
            // require building my own castle/fortress
            if (receiver->GetQuestStatus(receiver->GetTeam() == ALLIANCE ? 36615 : 36614) != QUEST_STATUS_INCOMPLETE)
                result = GARRISON_ERROR_UPGRADE_CONDITION_FAILED;
            break;
        default:
            result = GARRISON_ERROR_UPGRADE_LEVEL_EXCEEDS_GARRISON_LEVEL;
            break;
    }

    if (sWorld->getBoolConfig(CONFIG_DISABLE_GARE_UPGRADE))
        result = GARRISON_ERROR_UPGRADE_LEVEL_EXCEEDS_GARRISON_LEVEL;

    return result;
}

void Garrison::SendGarrisonUpgradebleResult(Player* receiver, int32 garrSiteID) const
{
    auto site = _siteLevel[GARRISON_TYPE_GARRISON];
    if (!site)
        return;

    WorldPackets::Garrison::GarrisonIsUpgradeableResult result;
    result.Result = site->GarrSiteID == garrSiteID ? CanUpgrade(receiver, site) : GARRISON_ERROR_INVALID_SITE_ID;
    receiver->SendDirectMessage(result.Write());
}

//! ToDo: chesc for GARRISON_TYPE_CLASS_ORDER
Map* Garrison::FindMap() const
{
    if (auto site = _siteLevel[GARRISON_TYPE_GARRISON])
        return sMapMgr->FindMap(site->MapID, _owner->GetGUIDLow());
    return nullptr;
}

//! ToDo: chesc for GARRISON_TYPE_CLASS_ORDER
GarrisonError Garrison::CheckBuildingPlacement(uint32 garrPlotInstanceId, uint32 garrBuildingId, bool byQuest/*=false*/) const
{
    auto site = _siteLevel[GARRISON_TYPE_GARRISON];

    GarrPlotInstanceEntry const* plotInstance = sGarrPlotInstanceStore.LookupEntry(garrPlotInstanceId);
    Plot const* plot = GetPlot(garrPlotInstanceId);
    if (!plotInstance || !plot)
        return GARRISON_ERROR_INVALID_PLOT_INSTANCEID;

    GarrBuildingEntry const* building = sGarrBuildingStore.LookupEntry(garrBuildingId);
    if (!building)
        return GARRISON_ERROR_INVALID_BUILDINGID;

    if (!sGarrisonMgr.IsPlotMatchingBuilding(plotInstance->GarrPlotID, garrBuildingId))
        return GARRISON_ERROR_INVALID_PLOT_BUILDING;

    // Cannot place buldings of higher level than garrison level
    if (site && building->UpgradeLevel > site->GarrLevel)
        return GARRISON_ERROR_INVALID_BUILDINGID;

    if (building->Flags & GARRISON_BUILDING_FLAG_NEEDS_PLAN)
    {
        if (!_knownBuildings.count(garrBuildingId))
            return GARRISON_ERROR_REQUIRES_BLUEPRINT;
    }
    else if (!byQuest) // Building is built as a quest reward
        return GARRISON_ERROR_INVALID_BUILDINGID;

    // Check all plots to find if we already have this building
    for (auto const& p : _plots)
    {
        if (p.second.BuildingInfo.PacketInfo)
        {
            GarrBuildingEntry const* existingBuilding = sGarrBuildingStore.AssertEntry(p.second.BuildingInfo.PacketInfo->GarrBuildingID);
            if (existingBuilding->BuildingType == building->BuildingType)
                if (p.first != garrPlotInstanceId || existingBuilding->UpgradeLevel + 1 != building->UpgradeLevel)    // check if its an upgrade in same plot
                    return GARRISON_ERROR_BUILDING_EXISTS;
        }
    }

    if (!byQuest && building->CurrencyQty && !_owner->HasCurrency(building->CurrencyTypeID, building->CurrencyQty))
        return GARRISON_ERROR_NOT_ENOUGH_CURRENCY;

    if (building->GoldCost && !_owner->HasEnoughMoney(uint64(building->GoldCost) * GOLD))
        return GARRISON_ERROR_NOT_ENOUGH_GOLD;

    // New building cannot replace another building currently under construction
    if (plot->BuildingInfo.PacketInfo)
        if (!plot->BuildingInfo.PacketInfo->Active)
            return GARRISON_ERROR_NO_BUILDING;

    return GARRISON_SUCCESS;
}

GarrisonError Garrison::CheckBuildingRemoval(uint32 garrPlotInstanceId) const
{
    Plot const* plot = GetPlot(garrPlotInstanceId);
    if (!plot)
        return GARRISON_ERROR_INVALID_PLOT_INSTANCEID;

    if (!plot->BuildingInfo.PacketInfo)
        return GARRISON_ERROR_NO_BUILDING;

    if (plot->BuildingInfo.CanActivate())
        return GARRISON_ERROR_BUILDING_EXISTS;

    return GARRISON_SUCCESS;
}

bool Garrison::GetAreaIdForTeam(uint32 team, AreaTableEntry const* area)
{
    if (!area)
        return false;

    switch (team)
    {
        case ALLIANCE:
            if (area->ID == 7078 || area->ParentAreaID == 7078)
                return true;
            break;
        case HORDE:
            if (area->ID == 7004 || area->ParentAreaID == 7004)
                return true;
            break;
    }
    return false;
}

uint32 Garrison::GetMissionSuccessChance(Mission* mission, GarrMissionEntry const* missionEntry)
{
    const auto missionRecID = mission->PacketInfo.RecID;

    std::vector<uint32> encounters;
    std::vector<std::pair<uint32, uint32>> encoutersMechanics;
    std::vector<Follower*> missionFollowers = mission->GetMissionFollowers(this);
    std::map<uint64, double> followersBiasMap;

    //std::vector<uint32> l_PassiveEffects = GetBuildingsPassiveAbilityEffects();   //!< ToDo

    auto missionEntryPacket = sGarrMissionStore.LookupEntry(missionRecID);
    if (!missionEntryPacket)
        return 0;

    bool const isHall = missionEntryPacket->GarrTypeID == GARRISON_TYPE_CLASS_ORDER;
    uint8 const MaxFollowerLevel = isHall ? GarrisonConst::Globals::MaxFollowerLevelHall : GarrisonConst::Globals::MaxFollowerLevel;

    //! ToDo: speed it up.
    for (auto const& missionXEncounterEntry : sGarrMissionXEncounterStore)
    {
        if (missionXEncounterEntry && missionXEncounterEntry->GarrMissionID == missionRecID)
        {
            if (missionXEncounterEntry->GarrEncounterID)
                encounters.push_back(missionXEncounterEntry->GarrEncounterID);

            //! ToDo: select from garrEncounterSEtXEncounter.db2
            if (missionXEncounterEntry->GarrEncounterSetID)
            {
                if (auto data = sDB2Manager.getXEncounter(missionXEncounterEntry->GarrEncounterSetID))
                    for (uint32 encounter : *data)
                        encounters.push_back(encounter);
            }
        }
    }

    for (auto & encounter : encounters)
    {
        for (auto const& encounterXMechanicEntry : sGarrEncounterXMechanicStore)
        {
            if (encounterXMechanicEntry && encounterXMechanicEntry->GarrEncounterID == encounter)
            {
                if (encounterXMechanicEntry->GarrMechanicID)
                    encoutersMechanics.emplace_back(encounter, encounterXMechanicEntry->GarrMechanicID);

                //! ToDo: select from GarrMechanicSetXMechanic.db2
                if (encounterXMechanicEntry->GarrMechanicID)
                {
                    if (auto data = sDB2Manager.getXMechanic(encounterXMechanicEntry->GarrMechanicID))
                        for (uint32 mechanic : *data)
                            encoutersMechanics.emplace_back(encounter, mechanic);
                }
            }
        }
    }

    for (auto & missionFollower : missionFollowers)
    {
        double followerBias = (missionFollower->PacketInfo.FollowerLevel - missionEntry->TargetLevel) * 0.33333334;

        if (missionEntry->TargetLevel >= MaxFollowerLevel)
        {
            if (missionEntry->TargetItemLevel > 0)
            {
                uint32 totalFollowerItemLevel = missionFollower->PacketInfo.ItemLevelArmor + missionFollower->PacketInfo.ItemLevelWeapon;
                followerBias = (((totalFollowerItemLevel >> 1) - missionEntry->TargetItemLevel) * 0.06666667) + followerBias;
            }
        }

        if (followerBias < -1.0)
            followerBias = -1.0;
        else if (followerBias > 1.0)
            followerBias = 1.0;

        followersBiasMap[missionFollower->PacketInfo.DbID] = followerBias;


    }

    double l_Float8 = 100.0;
    double l_FloatC = 150.0;

    double l_V8 = missionEntry->MaxFollowers * l_Float8;
    double l_V60 = missionEntry->MaxFollowers * l_Float8;

    for (auto & encoutersMechanic : encoutersMechanics)
    {
        auto mechanicEntry = sGarrMechanicStore.LookupEntry(encoutersMechanic.second);
        if (!mechanicEntry)
            continue;

        auto mechanicTypeEntry = sGarrMechanicTypeStore.LookupEntry(mechanicEntry->GarrMechanicTypeID);
        if (mechanicTypeEntry && mechanicTypeEntry->Category != GarrisonConst::MechanicTypes::Ability)
            l_V8 = l_V60;
        else
        {
            l_V8 = mechanicEntry->Factor + l_V60;
            l_V60 = mechanicEntry->Factor + l_V60;
        }
    }

    double currentAdditionalWinChance = 0;

    double l_V11 = 100.0 / l_V8;
    double l_V62 = 100.0 / l_V8;

    /// OK 100%
    #pragma region Followers Bias
    for (auto & missionFollower : missionFollowers)
    {
        double seil = 0;

        if (followersBiasMap[missionFollower->PacketInfo.DbID] >= 0.0)
            seil = (l_FloatC - l_Float8) * followersBiasMap[missionFollower->PacketInfo.DbID] + l_Float8;
        else
            seil = (followersBiasMap[missionFollower->PacketInfo.DbID] + 1.0) * l_Float8;

        l_V8 = (seil * l_V11) + currentAdditionalWinChance; ///< l_V8 is never read 01/18/16
        currentAdditionalWinChance = (seil * l_V11) + currentAdditionalWinChance;


    }
    #pragma endregion

    /// OK 100%
    #pragma region Counter mechanic
    for (auto & encoutersMechanic : encoutersMechanics)
    {
        auto mechanicEntry = sGarrMechanicStore.LookupEntry(encoutersMechanic.second);
        if (!mechanicEntry)
            continue;

        auto mechanicTypeEntry = sGarrMechanicTypeStore.LookupEntry(mechanicEntry->GarrMechanicTypeID);
        if (!mechanicTypeEntry)
            continue;

        if (mechanicTypeEntry->Category == GarrisonConst::MechanicTypes::Ability)
        {
            double l_Unk1 = mechanicEntry->Factor; ///< l_Unk1 is never read 01/18/16
            double l_Unk2 = mechanicEntry->Factor;

            if (!missionFollowers.empty())
            {
                for (auto & missionFollower : missionFollowers)
                {
                    for (auto& abilityId : missionFollower->PacketInfo.AbilityID)
                    {
                        auto eff = sDB2Manager.GetGarrEffect(abilityId);
                        if (!eff)
                            continue;

                        for (GarrAbilityEffectEntry const* abilityEffectEntry : *eff)
                        {
                            if (abilityEffectEntry->GarrMechanicTypeID == mechanicTypeEntry->ID && !(abilityEffectEntry->Flags & 1))
                            {
                                l_Unk1 = l_Unk2; ///< l_Unk1 is never read 01/18/16
                                if (l_Unk2 != 0.0)
                                {
                                    float seil = 0;

                                    if (followersBiasMap[missionFollower->PacketInfo.DbID] >= 0.0)
                                        seil = (abilityEffectEntry->CombatWeightMax - abilityEffectEntry->CombatWeightBase) * followersBiasMap[missionFollower->PacketInfo.DbID] + abilityEffectEntry->CombatWeightBase;
                                    else
                                        seil = (followersBiasMap[missionFollower->PacketInfo.DbID] + 1.0) * abilityEffectEntry->CombatWeightBase;

                                    l_Unk1 = mechanicEntry->Factor;

                                    if (seil <= l_Unk1)
                                        l_Unk1 = seil;

                                    l_Unk2 = l_Unk2 - l_Unk1;
                                }
                            }
                        }
                    }
                }
            }

            if (l_Unk2 < 0.0)
                l_Unk2 = 0.0;

            l_Unk1 = mechanicEntry->Factor;
            l_Unk1 = (l_Unk1 - l_Unk2) * l_V62;
            currentAdditionalWinChance = l_Unk1 + currentAdditionalWinChance;


        }
    }
    #pragma endregion

    /// UNTESTED
    #pragma region Race Ability Counter
    for (auto & encoutersMechanic : encoutersMechanics)
    {
        auto mechanicEntry = sGarrMechanicStore.LookupEntry(encoutersMechanic.second);
        if (!mechanicEntry)
            continue;

        auto mechanicTypeEntry = sGarrMechanicTypeStore.LookupEntry(mechanicEntry->GarrMechanicTypeID);
        if (!mechanicTypeEntry)
            continue;

        if (mechanicTypeEntry->Category == GarrisonConst::MechanicTypes::Racial)
        {
            for (auto & missionFollower : missionFollowers)
            {
                for (auto& abilityId : missionFollower->PacketInfo.AbilityID)
                {
                    auto eff = sDB2Manager.GetGarrEffect(abilityId);
                    if (!eff)
                        continue;

                    for (GarrAbilityEffectEntry const* abilityEffectEntry : *eff)
                    {
                        if (abilityEffectEntry->GarrMechanicTypeID == missionEntry->EnvGarrMechanicTypeID)
                        {
                            double seil = 0;

                            if (followersBiasMap[missionFollower->PacketInfo.DbID] >= 0.0)
                                seil = (abilityEffectEntry->CombatWeightMax - abilityEffectEntry->CombatWeightBase) * followersBiasMap[missionFollower->PacketInfo.DbID] + abilityEffectEntry->CombatWeightBase;
                            else
                                seil = (followersBiasMap[missionFollower->PacketInfo.DbID] + 1.0) * abilityEffectEntry->CombatWeightBase;

                            currentAdditionalWinChance = (seil * l_V62) + currentAdditionalWinChance;


                        }
                    }
                }
            }
        }
    }
    #pragma endregion

    /// OK 100%
    #pragma region Environment Ability
    for (auto & missionFollower : missionFollowers)
    {
        for (auto& abilityId : missionFollower->PacketInfo.AbilityID)
        {
            auto eff = sDB2Manager.GetGarrEffect(abilityId);
            if (!eff)
                continue;

            for (GarrAbilityEffectEntry const* abilityEffectEntry : *eff)
            {
                if (abilityEffectEntry->GarrMechanicTypeID == missionEntry->EnvGarrMechanicTypeID)
                {
                    double seil = 0;

                    if (followersBiasMap[missionFollower->PacketInfo.DbID] >= 0.0)
                        seil = (abilityEffectEntry->CombatWeightMax - abilityEffectEntry->CombatWeightBase) * followersBiasMap[missionFollower->PacketInfo.DbID] + abilityEffectEntry->CombatWeightBase;
                    else
                        seil = (followersBiasMap[missionFollower->PacketInfo.DbID] + 1.0) * abilityEffectEntry->CombatWeightBase;

                    currentAdditionalWinChance = (seil * l_V62) + currentAdditionalWinChance;


                }
            }
        }
    }
    #pragma endregion

    /// OK 100%
    #pragma region Follower Trait
    double missionDuration = missionEntry->MissionDuration/*GetMissionDuration(missionRecID)*/; ///!< ToDo
    double missionTravelTime = 0/*missionEntry-> GetMissionTravelDuration(missionRecID)*/;

    /*std::set<int32> abilities;
    for (auto ab : _abilities)
        abilities.insert(ab);*/

    for (uint32 l_Y = 0; l_Y < missionFollowers.size(); ++l_Y)
    {
        for (auto& abilityId : missionFollowers[l_Y]->PacketInfo.AbilityID)
        {
            auto eff = sDB2Manager.GetGarrEffect(abilityId);
            if (!eff)
                continue;

            for (GarrAbilityEffectEntry const* abilityEffectEntry : *eff)
            {
                auto canProc = false;

                switch (abilityEffectEntry->AbilityAction)
                {
                    case GarrisonConst::AbilityEffectTypes::ModWinRateSolo: /// Proc if MissionFollowerCount == 1
                        canProc = missionFollowers.size() == 1;
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRate: /// Proc every time, no condition
                        canProc = true;
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRateClass: /// Proc if Find(MissionFollowers[Class], ActionRace) != NULL
                        for (uint32 l_W = 0; l_W < missionFollowers.size(); ++l_W)
                        {
                            if (l_W != l_Y)
                            {
                                auto followerEntry = sGarrFollowerStore.LookupEntry(missionFollowers[l_W]->PacketInfo.GarrFollowerID);
                                if (followerEntry && (followerEntry->HordeCreatureID == abilityEffectEntry->ActionRace) || (followerEntry->AllianceCreatureID == abilityEffectEntry->ActionRace))
                                {
                                    canProc = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRateDurationMore: /// Proc if Duration > (3600 * MiscValueB)
                        canProc = missionDuration > (3600 * abilityEffectEntry->ActionHours);
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRateDurationLess: /// Proc if Duration < (3600 * MiscValueB)
                        canProc = missionDuration < (3600 * abilityEffectEntry->ActionHours);
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRateTravelDurationMore: /// Proc if TravelDuration > (3600 * MiscValueB)
                        canProc = missionTravelTime >(3600 * abilityEffectEntry->ActionHours);
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRateTravelDurationLess: /// Proc if TravelDuration < (3600 * MiscValueB)
                        canProc = missionTravelTime < (3600 * abilityEffectEntry->ActionHours);
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModWinRateFirstDaySend:
                        canProc = true;  //todo. check only first at day.
                        break;
                    case GarrisonConst::AbilityEffectTypes::ModUnk0:
                    case GarrisonConst::AbilityEffectTypes::ModTravelTime:
                    case GarrisonConst::AbilityEffectTypes::ModXpGain:
                    case GarrisonConst::AbilityEffectTypes::ModGarrCurrencyDrop:
                    case GarrisonConst::AbilityEffectTypes::ModUnk11:
                    case GarrisonConst::AbilityEffectTypes::ModDummyProduction:
                    case GarrisonConst::AbilityEffectTypes::ModBronzeTreasureDrop:
                    case GarrisonConst::AbilityEffectTypes::ModSilverTreasureDrop:
                    case GarrisonConst::AbilityEffectTypes::ModGoldTreasureDrop:
                    case GarrisonConst::AbilityEffectTypes::ModChestDropRate:
                    case GarrisonConst::AbilityEffectTypes::ModMissionDuration:
                        break;
                    default:
                        break;
                }

                if (!canProc)
                    break;

                auto seil = 0.0;

                if (followersBiasMap[missionFollowers[l_Y]->PacketInfo.DbID] >= 0.0)
                    seil = (abilityEffectEntry->CombatWeightMax - abilityEffectEntry->CombatWeightBase) * followersBiasMap[missionFollowers[l_Y]->PacketInfo.DbID] + abilityEffectEntry->CombatWeightBase;
                else
                    seil = (followersBiasMap[missionFollowers[l_Y]->PacketInfo.DbID] + 1.0) * abilityEffectEntry->CombatWeightBase;

                currentAdditionalWinChance = (seil * l_V62) + currentAdditionalWinChance;

            }
        }
    }
    #pragma endregion

    ///// UNTESTED
    //#pragma region Passive Effect
    //for (uint32 l_Y = 0; l_Y < l_PassiveEffects.size(); ++l_Y)
    //{
    //    GarrAbilityEffectEntry const* abilityEffectEntry = sGarrAbilityEffectStore.LookupEntry(l_PassiveEffects[l_Y]);

    //    if (!abilityEffectEntry)
    //        continue;

    //    if (abilityEffectEntry->EffectType == AbilityEffectTypes::ModWinRate)
    //    {
    //        currentAdditionalWinChance = (abilityEffectEntry->ModMin * l_V62) + currentAdditionalWinChance;

    //    #ifdef GARRISON_CHEST_FORMULA_DEBUG
    //        printf("Added %.2f to success due to passive effect %u.\n", abilityEffectEntry->ModMin * l_V62, abilityEffectEntry->AbilityID);
    //    #endif // GARRISON_CHEST_FORMULA_DEBUG
    //    }
    //}
    //#pragma endregion

    currentAdditionalWinChance = (((100.0 - missionEntry->FollowerDeathChance) * currentAdditionalWinChance) * 0.0099999998) + missionEntry->FollowerDeathChance;

    if (missionEntry->ID == 926)
        currentAdditionalWinChance += 90;



    return currentAdditionalWinChance;
}

void Garrison::SendMissionListUpdate(bool /*openMissionNpc*/) const
{
    uint8 type = _owner->GetMap()->GetEntry()->ExpansionID == 5 ? GARRISON_TYPE_GARRISON : GARRISON_TYPE_CLASS_ORDER;

    if (!_siteLevel[type])
        return;

    WorldPackets::Garrison::GarrisonMissionUpdate res;

    if (!_missions[type].empty())
    {
        res.CanStartMission.reserve(_missions[type].size());
        res.ArchivedMissions.reserve(_missions[type].size());
    }

    for (auto const& i : _missions[type])
    {
        res.CanStartMission.push_back(true);
        res.ArchivedMissions.push_back(i.second.PacketInfo.RecID);
    }

    _owner->SendDirectMessage(res.Write());
}

void Garrison::RewardMission(uint32 missionRecID, bool owermax/* = false*/)
{
    if (auto mission = sGarrisonMgr.GetNextMissionInQuestLine(missionRecID))
        AddMission(mission->ID);

    GenerateRandomMission(1);

    auto missionRewardEntry = owermax ? sGarrisonMgr.GetMissionOwermaxRewardByRecID(missionRecID) : sGarrisonMgr.GetMissionRewardByRecID(missionRecID);
    if (!missionRewardEntry)
        return;

    if (missionRewardEntry->HasKillCredit())
        _owner->KilledMonsterCredit(missionRewardEntry->KillCredit);

    if (missionRewardEntry->HasMoneyReward())
        _owner->ModifyMoney(missionRewardEntry->CurrencyValue);

    if (missionRewardEntry->HasCurrencyReward())
        _owner->ModifyCurrency(missionRewardEntry->CurrencyID, missionRewardEntry->CurrencyValue, true);

    if (!missionRewardEntry->HasItemReward())
        return;

    ItemPosCountVec dest;
    if (_owner->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, missionRewardEntry->RewardItemID, missionRewardEntry->ItemAmount) == EQUIP_ERR_OK)
        if (Item* item = _owner->StoreNewItem(dest, missionRewardEntry->RewardItemID, true, Item::GenerateItemRandomPropertyId(missionRewardEntry->RewardItemID, _owner->GetLootSpecID())))
            _owner->SendNewItem(item, missionRewardEntry->ItemAmount, true, false);

    if (auto sYard = GetPlotWithBuildingType(GARR_BTYPE_SALVAGE_YARD))
    {
        float chance = 30.0f;
        uint32 itemID = SALVAGE_ITEM;
        switch (sGarrBuildingStore.AssertEntry(sYard->BuildingInfo.PacketInfo->GarrBuildingID)->UpgradeLevel)
        {
        case 2:
            chance = 60.0f;
            break;
        case 3:
            chance = 60.0f;
            itemID = SALVAGE_ITEM_BIG;
            break;
        default:
            break;
        }

        if (roll_chance_f(chance))
        {
            ItemPosCountVec dest2;
            if (_owner->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest2, itemID, 1) == EQUIP_ERR_OK)
                if (Item* item = _owner->StoreNewItem(dest2, itemID, true, Item::GenerateItemRandomPropertyId(itemID, _owner->GetLootSpecID())))
                    _owner->SendNewItem(item, itemID, true, false);
        }
    }
}

/*
The Garrison Cache next to your Town Hall accumulates  Garrison Resources (GR)
at a rate of 1 GR every 10 minutes of real time (6 per hour),
which works out to 144 GR every full day (6 x 24hrs = 144 GR).

�� 3-� ����� � 10 ����� ����� ��������� ����� � 500 ��������.

��� ������� � �������������  �������� ����������: �������-����� (��������� � ���������� ��������) ����� ������ ���������� ������ 1000 ��������. = 6 ����� 20 �����.
*/
uint32 Garrison::GetResNumber() const
{
    // ToDo: set get congig
    #define default_resource_num 50
    #define limit_cap 500
    #define min_counter 10

    if (!_lastResTaken)
        return default_resource_num;

    uint32 res = (time(nullptr) - _lastResTaken) / (min_counter * MINUTE);
    return res > limit_cap ? limit_cap : res;
}

void Garrison::UpdateResTakenTime()
{
    _lastResTaken = time(nullptr);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    SaveToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

//! Activate some buildings at finishing quests as complete spells not exist on dbc. For example: spellID 165077
void Garrison::OnQuestReward(uint32 questID)
{
    switch (questID)
    {
        // Open Herb Garden PlotID = 63 & BuildID = 29
        case 36404:
        case 34193:
            PlaceBuilding(63, 29, true);
            break;
        // Open Mines PlotID = 59 & BuildID = 61
        case 35154:
        case 34192:
            PlaceBuilding(59, 61, true);
            break;
        // Open Fishing PlotID = 67 & BuildID = 64
        case 36870:
        case 36612:
            PlaceBuilding(67, 64, true);
            break;
        // Open Pet Menagerie PlotID = 81 & BuildID = 42
        case 36483:
        case 36662:
            PlaceBuilding(81, 42, true);
            break;
        default:
            break;
    }
}

//! ToDo: chesc for GARRISON_TYPE_CLASS_ORDER
bool Garrison::canAddShipmentOrder(Creature* source)
{
    if (!source->HasFlag(UNIT_FIELD_NPC_FLAGS2, UNIT_NPC_FLAG2_AI_OBSTACLE | UNIT_NPC_FLAG2_RECRUITER))
        return false;

    auto data = sGarrisonMgr.GetGarrShipment(source->GetEntry(), SHIPMENT_GET_BY_NPC, _owner->getClass());
    if (!data || !data->cEntry)
        return false;

    auto site = _siteLevel[data->cEntry->GarrTypeID];
    if (!site)
        return false;

    if (uint32 questID = getQuestIdReqForShipment(site->GarrSiteID, data->cEntry->GarrBuildingType))
        if (_owner->GetQuestStatus(questID) == QUEST_STATUS_NONE)
            return false;

    if (!data->selectShipment(_owner))
        return false;

    // check if we have aready build building.
    for (auto const& p : _plots)
        if (p.second.BuildingInfo.PacketInfo && p.second.BuildingInfo.PacketInfo->Active)
            if (sGarrBuildingStore.AssertEntry(p.second.BuildingInfo.PacketInfo->GarrBuildingID)->BuildingType == data->cEntry->GarrBuildingType)
                return true;

    return data->cEntry->GarrTypeID == GARRISON_TYPE_CLASS_ORDER;
}

bool Garrison::canStartUpgrade()
{
    enum q
    {
        QUEST_DRUID = 42588, //50175
        QUEST_MAGE = 42696, //50181
        QUEST_WARRIOR = 42611,//50178
        QUEST_PRIEST = 43277, //50172
        QUEST_MONK = 42191, //49435
        QUEST_HUNTER = 42526,//50163
        QUEST_DH = 42683, //50184
        QUEST_PALADIN = 42850, //50166
        QUEST_ROGUE = 43015, //50187
        QUEST_WARLOCK = 42601, //50169
        QUEST_SHAMAN = 41740, //50160
        QUEST_DK = 43268, //50190

    };

    uint32 q = 0;
    switch (_owner->getClass())
    {
        case CLASS_DEMON_HUNTER:
            q = QUEST_DH;
            break;
        case CLASS_DRUID:
            q = QUEST_DRUID;
            break;
        case CLASS_MONK:
            q = QUEST_MONK;
            break;
        case CLASS_WARLOCK:
            q = QUEST_WARLOCK;
            break;
        case CLASS_SHAMAN:
            q = QUEST_SHAMAN;
            break;
        case CLASS_DEATH_KNIGHT:
            q = QUEST_DK;
            break;
        case CLASS_PRIEST:
            q = QUEST_PRIEST;
            break;
        case CLASS_ROGUE:
            q = QUEST_ROGUE;
            break;
        case CLASS_HUNTER:
            q = QUEST_HUNTER;
            break;
        case CLASS_PALADIN:
            q = QUEST_PALADIN;
            break;
        case CLASS_WARRIOR:
            q = QUEST_WARRIOR;
            break;
        case CLASS_MAGE:
            q = QUEST_MAGE;
            break;
        default:
            break;
    }

    if (_owner->GetQuestStatus(q) == QUEST_STATUS_NONE)
        return false;

    return true;
}

void Garrison::StartClassHallUpgrade(uint32 tallentID)
{
    WorldPackets::Garrison::GarrisonUpgradeResult result;

    if (!canStartUpgrade() || talentResearchTimer > 0)
    {
        result.Result = GARRISON_ERROR_ALREADY_RESEARCHING_TALENT;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    GarrTalentEntry const* entry = sGarrTalentStore.LookupEntry(tallentID);
    if (!entry)
    {
        result.Result = GARRISON_ERROR_INVALID_TALENT;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    if (!ConditionMgr::IsPlayerMeetingCondition(_owner, entry->PlayerConditionID))
    {
        result.Result = GARRISON_ERROR_INVALID_TALENT;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    GarrTalentEntry const* toRemove = nullptr;
    for (auto data : _classHallTalentStore)
    {
        GarrTalentEntry const* talentEntry = sGarrTalentStore.AssertEntry(data.GarrTalentID);
        if (talentEntry->Tier == entry->Tier)
        {
            //! this data will be remove at Garrison::AddTalentToStore second check of _classHallTalentStore as we can't start upgrade if no currency.
            toRemove = talentEntry;
            break;
        }
    }

    uint32 const currency = toRemove ? entry->RespecCostCurrencyTypesID : entry->ResearchCostCurrencyTypesID;
    uint32 const cost = toRemove ? entry->RespecCost : entry->ResearchCost;

    // cost taken from old site, not from new
    if (!_owner->HasCurrency(currency, cost))
    {
        result.Result = GARRISON_ERROR_NOT_ENOUGH_CURRENCY;
        _owner->SendDirectMessage(result.Write());
        return;
    }

    _owner->ModifyCurrency(currency, -1 * cost, false, true);

    WorldPackets::Garrison::GarrisonResearchTalent data;
    data.GarrTypeID = 3;
    data.TalentID = tallentID;
    data.ResearchTime = time(nullptr);
    _owner->SendDirectMessage(data.Write());

    AddTalentToStore(tallentID, data.ResearchTime, toRemove ? GarrisonConst::ClassHallTalentFlag::CLASS_HALL_TALENT_CHANGE : GarrisonConst::ClassHallTalentFlag::CLASS_HALL_TALENT_IN_RESEARCH, DB_STATE_NEW);

    const uint32 criteria_upgrade[MAX_CLASSES] =
    {
        0, 50178, 50166, 50163, 50187, 50172, 50190, 50160, 50181, 50169, 49435, 50175, 50184
    };
    _owner->AchieveCriteriaCredit(criteria_upgrade[_owner->getClass()]);
    _owner->UpdateAchievementCriteria(CRITERIA_TYPE_LEARN_GARRISON_TALENT, tallentID);
}

void Garrison::AddTalentToStore(uint32 talentID, uint32 _time, uint32 flags, ObjectDBState DbState/* = DB_STATE_UNCHANGED*/)
{
    auto talentEntry = sGarrTalentStore.LookupEntry(talentID);
    if (!talentEntry)
        return;

    //! second check.
    for (auto data = _classHallTalentStore.begin(); data != _classHallTalentStore.end(); ++data)
    {
        GarrTalentEntry const* __entry = sGarrTalentStore.AssertEntry(data->GarrTalentID);
        if (__entry->Tier == talentEntry->Tier)
        {
            auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_TALENT_BY_ID);
            stmt->setUInt64(0, _owner->GetGUIDLow());
            stmt->setUInt32(1, __entry->ID);
            CharacterDatabase.Execute(stmt);

            //! ToDo: remove _abilities
            _classHallTalentStore.erase(data);
            break;
        }
    }

    uint32 researchTime = GarrisonConst::ClassHallTalentFlag::CLASS_HALL_TALENT_CHANGE &  flags ? talentEntry->RespecDurationSecs : talentEntry->ResearchDurationSecs;
    researchTime += _time;

    WorldPackets::Garrison::GarrisonTalent talentData;
    talentData.GarrTalentID = talentID;
    talentData.ResearchStartTime = _time;
    talentData.Flags = flags;
    talentData.DbState = DbState;

    //! handle abilities added by talents.
    if (talentEntry->GarrAbilityID)
        _abilities.insert(talentEntry->GarrAbilityID);

    if (time(nullptr) < researchTime)
        talentResearchTimer = researchTime;
    else
    {
        talentData.Flags |= GarrisonConst::ClassHallTalentFlag::CLASS_HALL_TALENT_READY;

        if (talentEntry->PerkSpellID)
            _owner->CastSpell(_owner, talentEntry->PerkSpellID, true);
    }

    _classHallTalentStore.push_back(talentData);
}

bool Garrison::hasTallent(uint32 talentID) const
{
    auto talentEntry = sGarrTalentStore.LookupEntry(talentID);
    if (!talentEntry)
        return false;

    for (auto data : _classHallTalentStore)
    {
        if (data.GarrTalentID == talentID)
        {
            uint32 const researchTime = data.Flags & GarrisonConst::ClassHallTalentFlag::CLASS_HALL_TALENT_CHANGE ? talentEntry->RespecDurationSecs : talentEntry->ResearchDurationSecs;
            return time(nullptr) > (researchTime + data.ResearchStartTime);
        }
    }

    return false;
};

uint32 Garrison::GetPlotInstanceForBuildingType(uint32 type) const
{
    for (auto const& plot : _plots)
        if (plot.second.BuildingInfo.PacketInfo && sGarrBuildingStore.AssertEntry(plot.second.BuildingInfo.PacketInfo->GarrBuildingID)->BuildingType == type)
            return plot.second.BuildingInfo.PacketInfo->GarrPlotInstanceID;

    return 0;
}

void Garrison::OnGossipSelect(WorldObject* source)
{
    if (!source->HasFlag(UNIT_FIELD_NPC_FLAGS2, UNIT_NPC_FLAG2_AI_OBSTACLE | UNIT_NPC_FLAG2_RECRUITER))
        return;

    auto data = sGarrisonMgr.GetGarrShipment(source->GetEntry(), SHIPMENT_GET_BY_NPC, _owner->getClass());
    if (!data)
        return;

    //! GarrShipment should have garrType.
    auto site = _siteLevel[data->cEntry->GarrTypeID];

    if (uint32 questID = getQuestIdReqForShipment(site->GarrSiteID, data->cEntry->GarrBuildingType))
        if (_owner->GetQuestStatus(questID) == QUEST_STATUS_NONE)
            return;

    if (!data->selectShipment(_owner))
        return;

    WorldPackets::Garrison::OpenShipmentNPCFromGossip openShipment;
    openShipment.NpcGUID = source->GetGUID();
    openShipment.CharShipmentContainerID = data->ContainerID;
    _owner->SendDirectMessage(openShipment.Write());
}

void Garrison::SendShipmentInfo(ObjectGuid const& guid)
{
    GarrShipment const* shipment = sGarrisonMgr.GetGarrShipment(guid.GetEntry(), SHIPMENT_GET_BY_NPC, _owner->getClass());
    const Plot* plot = GetPlotWithBuildingType(shipment->cEntry->GarrBuildingType);

    auto site = _siteLevel[shipment->cEntry->GarrTypeID];

    uint32 questID = getQuestIdReqForShipment(site->GarrSiteID, shipment->cEntry->GarrBuildingType);
    uint32 shipmentID = sGarrisonMgr.GetShipmentID(shipment);

    if (!shipmentID)
        shipmentID = shipment->selectShipment(_owner);

    //SMSG_GET_SHIPMENT_INFO_RESPONSE
    WorldPackets::Garrison::GetShipmentInfoResponse shipmentResponse;
    shipmentResponse.Success = shipment && (plot || shipment->cEntry->GarrTypeID == GARRISON_TYPE_CLASS_ORDER) && (!questID || questID && _owner->GetQuestStatus(questID) != QUEST_STATUS_NONE);

    //! placeholder for check is allowed shipment.
    uint32 sh = shipment->ShipmentID;
    sh = sGarrisonMgr.GetShipmentID(shipment);
    if (!sh)
        sh = shipment->selectShipment(_owner);

    auto shipmentEntry = sCharShipmentStore.LookupEntry(sh);
    if (shipmentEntry)
        shipmentResponse.Success = _owner->CheckShipment(shipmentEntry);

    if (shipmentResponse.Success)
    {
        // check if has finish quest for activate. if rewardet - use usual state. if in progress - send 
        shipmentResponse.ShipmentID = (!questID || _owner->GetQuestStatus(questID) == QUEST_STATUS_REWARDED) ? shipmentID : getProgressShipment(questID);
        shipmentResponse.Shipments.assign(_shipments[idxShipment(shipment->cEntry)].begin(), _shipments[idxShipment(shipment->cEntry)].end());

        if (shipment->cEntry->GarrTypeID == GARRISON_TYPE_CLASS_ORDER && shipmentEntry)
            shipmentResponse.MaxShipments = shipmentEntry->MaxShipments;
        else
        {
            shipmentResponse.MaxShipments = sGarrBuildingStore.AssertEntry(plot->BuildingInfo.PacketInfo->GarrBuildingID)->ShipmentCapacity + GetShipmentMaxMod();
            shipmentResponse.PlotInstanceID = plot->BuildingInfo.PacketInfo->GarrPlotInstanceID;
        }
    }

    _owner->SendDirectMessage(shipmentResponse.Write());
}

void Garrison::CreateShipment(ObjectGuid const& guid, uint32 count)
{
    auto shipment = sGarrisonMgr.GetGarrShipment(guid.GetEntry(), SHIPMENT_GET_BY_NPC, _owner->getClass());
    if (!shipment)
        return;

    auto site = _siteLevel[shipment->cEntry->GarrTypeID];

    const Plot* plot = GetPlotWithBuildingType(shipment->cEntry->GarrBuildingType);
    if (!plot && shipment->cEntry->GarrTypeID == GARRISON_TYPE_GARRISON)
        return;

    uint32 sh = shipment->ShipmentID;
    sh = sGarrisonMgr.GetShipmentID(shipment);

    if (uint32 questID = getQuestIdReqForShipment(site->GarrSiteID, shipment->cEntry->GarrBuildingType))
        if (_owner->GetQuestStatus(questID) != QUEST_STATUS_REWARDED)
            sh = getProgressShipment(questID);

    if (!sh)
        sh = shipment->selectShipment(_owner);

    CharShipmentEntry const* shipmentEntry = sCharShipmentStore.LookupEntry(sh);
    if (!shipmentEntry)
        return;

    //! placeholder for artifact check.
    if (!_owner->CheckShipment(shipmentEntry)) //! return if not allowed.
        return;

    uint32 max = 0;
    if (shipment->cEntry->GarrTypeID == GARRISON_TYPE_CLASS_ORDER)
        max = shipmentEntry->MaxShipments;
    else
        max = sGarrBuildingStore.AssertEntry(plot->BuildingInfo.PacketInfo->GarrBuildingID)->ShipmentCapacity + GetShipmentMaxMod();

    auto t_shipments = _shipments[idxShipment(shipment->cEntry)];
    if (t_shipments.size() >= max)
        return;

    uint32 spellCast = 0;
    for (uint32 i = 0; i < count; ++i)
    {
        spellCast = shipmentEntry->SpellID;
        switch (sh)
        {
            case 60:  spellCast = 172840; break;
            case 103: spellCast = 171959; break;
            case 106: spellCast = 172841; break;
            case 107: spellCast = 172842; break;
            case 108: spellCast = 172843; break;
            case 105: spellCast = 172844; break;
            case 104: spellCast = 172845; break;
            case 72:  spellCast = 172846; break;
            case 125: spellCast = 173065; break;
            case 123: spellCast = 173066; break;
            case 121: spellCast = 173067; break;
            case 135: spellCast = 173068; break;
            case 133: spellCast = 173069; break;
            case 131: spellCast = 173070; break;
            case 129: spellCast = 173071; break;
            case 127: spellCast = 173072; break;
        }
        if (spellCast)
            _owner->CastSpell(_owner, spellCast, false);

        //! if no shipment in spell - just add it after cast.
        if (SpellInfo const* excludeTargetSpellInfo = sSpellMgr->GetSpellInfo(spellCast))
            if (!excludeTargetSpellInfo->HasEffect(SPELL_EFFECT_CREATE_SHIPMENT))
                CreateGarrisonShipment(shipmentEntry->ID);
    }
}

void Garrison::CreateGarrisonShipment(uint32 shipmentID)
{
    uint64 dbID = PlaceShipment(shipmentID, time(nullptr));

    WorldPackets::Garrison::CreateShipmentResponse shipmentResponse;
    shipmentResponse.ShipmentRecID = shipmentID;
    shipmentResponse.ShipmentID = dbID;
    shipmentResponse.Result = dbID ? GARRISON_SUCCESS : GARRISON_ERROR_NO_GARRISON;

    _owner->SendDirectMessage(shipmentResponse.Write());
}

uint32 Garrison::GetShipmentMaxMod()
{
    if (auto store = GetPlotWithBuildingType(GARR_BTYPE_STOREHOUSE))
        return sGarrBuildingStore.AssertEntry(store->BuildingInfo.PacketInfo->GarrBuildingID)->UpgradeLevel == 3 ? 15 : 5;

    return 0;
}

/**
    @start - shipment place time -> CreationTime
    @end -  end of current shipment -> end
*/
uint64 Garrison::PlaceShipment(uint32 shipmentID, uint32 start, uint32 end/* = 0*/, uint64 dbID/* = 0*/)
{
    CharShipmentEntry const* shipmentEntry = sCharShipmentStore.LookupEntry(shipmentID);
    if (!shipmentEntry)
        return 0;

    CharShipmentContainerEntry const* shipmentConteinerEntry = sCharShipmentContainerStore.LookupEntry(shipmentEntry->ContainerID);
    if (!shipmentConteinerEntry)
        return 0;

    if (shipmentConteinerEntry->GarrTypeID == GARRISON_TYPE_GARRISON)
    {
        Plot* plot = GetPlotWithBuildingType(shipmentConteinerEntry->GarrBuildingType);
        if (!plot)
            return 0;

        GarrBuildingEntry const* existingBuilding = sGarrBuildingStore.LookupEntry(plot->BuildingInfo.PacketInfo->GarrBuildingID);
        if (!existingBuilding)
            return 0;

        if (_shipments[shipmentConteinerEntry->GarrBuildingType].size() >= (existingBuilding->ShipmentCapacity + GetShipmentMaxMod()))
            return 0;
    }
    else
    {
        if (_shipments[shipmentConteinerEntry->ID].size() >= (shipmentEntry->MaxShipments + GetShipmentMaxMod()))
            return 0;
    }

    ObjectDBState state = DB_STATE_UNCHANGED;

    if (!start)
        start = time(nullptr);

    // find last finishing time.
    if (!end)
    {
        uint32 prev_end = start;
        state = DB_STATE_NEW;
        for (auto const& x : _shipments[idxShipment(shipmentConteinerEntry)])
        {
            uint32 __end = x.end;
            if (__end > prev_end)
                prev_end = __end;
        }

        // last in progress. so out complition is incrased.
        uint32 time_for = shipmentEntry->Duration;
        if (shipmentEntry->DummyItemID == 139390)
            time_for = sWorld->getIntConfig(CONFIG_ARTIFACT_RESEARCH_TIMER);
        end = prev_end + time_for;
    }

    WorldPackets::Garrison::Shipment shipmentResponce;
    shipmentResponce.ShipmentID = dbID ? dbID : sGarrisonMgr.GenerateShipmentDbId();
    shipmentResponce.ShipmentRecID = shipmentID;
    shipmentResponce.BuildingTypeID = shipmentConteinerEntry->GarrBuildingType;
    shipmentResponce.CreationTime = start;
    shipmentResponce.end = end;
    shipmentResponce.ShipmentDuration = end - start;
    shipmentResponce.DbState = state;
    _shipments[idxShipment(shipmentConteinerEntry)].push_back(shipmentResponce);
    return shipmentResponce.ShipmentID;
}

void Garrison::SendGarrisonShipmentLandingPage()
{
    WorldPackets::Garrison::GarrisonLandingPage packet;
    for (auto const& x : _shipments)
        for (auto const& shipment : x.second)
            packet.MsgData.push_back(shipment);
    _owner->SendDirectMessage(packet.Write());
}

void Garrison::CompleteShipments(GameObject *go)
{
    CharShipmentContainerEntry const* shipmentConteinerEntry = sCharShipmentContainerStore.LookupEntry(go->GetGOInfo()->garrisonShipment.ShipmentContainer);
    if (!shipmentConteinerEntry)
        return;

    std::map<uint32, uint32> loot_items;
    for (auto const& sh : _shipments[idxShipment(shipmentConteinerEntry)])
        if (sh.finished)
        {
            if (CharShipmentEntry const* shipmentEntry = sCharShipmentStore.LookupEntry(sh.ShipmentRecID))
            {
                if (shipmentEntry->DummyItemID)
                    ++loot_items[shipmentEntry->DummyItemID];

                // cast at get. kill redits and etc.
                if (shipmentEntry->OnCompleteSpellID)
                    _owner->CastSpell(_owner, shipmentEntry->OnCompleteSpellID, false);

                if (shipmentEntry->GarrFollowerID)
                    AddFollower(shipmentEntry->GarrFollowerID);
            }
        }

    // This is hardcode of Player::SendLoot
    Loot* loot = &go->loot;

    if (loot_items.empty() && loot->items.empty())
    {
        FreeShipmentChest(idxShipment(shipmentConteinerEntry));
        //go->SetLootState(GO_ACTIVATED, _owner);
        go->ForceValuesUpdateAtIndex(GAMEOBJECT_FIELD_DISPLAY_ID);
        go->ForceValuesUpdateAtIndex(GAMEOBJECT_FIELD_SPELL_VISUAL_ID);
        return;
    }

    loot->shipmentBuildingType = idxShipment(shipmentConteinerEntry);

    // fill loot only once... after we should add only continue.
    bool update = !loot->items.empty();
    if (!update)
    {
        loot->clear();
        loot->objType = 3;

        loot->FillLoot(go->GetEntry(), LootTemplates_Gameobject, _owner, true, false, go);
        loot->AddLooter(_owner->GetGUID());
    }
    else
        loot_items.clear();

    for (auto data : loot_items)
    {
        //HARDCORE.
        switch (data.first)
        {
            //Mine Shipment
            case 116055:
            {
                loot->AddOrReplaceItem(109119, data.second, false);
                loot->AddOrReplaceItem(109118, data.second, false);
                break;
            }
            //Garden Shipment
            case 116054:
                loot->AddOrReplaceItem(109125, data.second, false);
                loot->AddOrReplaceItem(109124, data.second, false);
                break;
            case 114677:
            case 118111:
                loot->AddOrReplaceItem(CURRENCY_TYPE_GARRISON_RESOURCES, data.second * 30, true);
                break;
            //WarMill
            case 120204:
                loot->AddOrReplaceItem(113681, data.second, false);
                break;
            default:
                loot->AddOrReplaceItem(data.first, data.second, false);
                break;
        }
    }

    for (auto item : loot->items)
    {
        if (item.currency)
        {
            _owner->ModifyCurrency(item.item.ItemID, item.count, true);
            continue;
        }

        ItemPosCountVec dest;
        InventoryResult msg = _owner->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item.item.ItemID, item.count);
        if (msg == EQUIP_ERR_OK)
        {
            if (Item* pItem = _owner->StoreNewItem(dest, item.item.ItemID, true, item.item.RandomPropertiesID, GuidSet(), item.item.ItemBonus.BonusListIDs, item.item.ItemBonus.Context))
                _owner->SendNewItem(pItem, item.count, false, false, true);
        }
        else
        {
            //send by mail.
            //! ToDo
            //Item* pItem = Item::CreateItem(item.item.ItemID, item.count, _owner);
        }
    }

    loot->clear();
    FreeShipmentChest(idxShipment(shipmentConteinerEntry));
    //go->SetGoState(GO_STATE_ACTIVE);
    go->ForceValuesUpdateAtIndex(GAMEOBJECT_FIELD_DISPLAY_ID);
    go->ForceValuesUpdateAtIndex(GAMEOBJECT_FIELD_SPELL_VISUAL_ID);

}

void Garrison::FreeShipmentChest(uint32 idx)
{
    if (!GetSpecialSpawnBuildingTime(idx))
        SetBuildingData(idx, BUILDING_DATA_SPECIAL_SPAWN, time(nullptr) + DAY);

    ShipmentSet &set = _shipments[idx];
    for (auto itr = set.begin(); itr != set.end();)
    {
        if (itr->finished)
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_SHIPMENTS_DBID);
            stmt->setUInt64(0, itr->ShipmentID);
            CharacterDatabase.Execute(stmt);

            // possible is better to delete it by delay, but i not understand why... it's complext to understand, but no need to delete it by delay.
            //! but, i set it in any way.
            itr->DbState = DB_STATE_REMOVED;

            set.erase(itr++);
        }
        else
            ++itr;
    }
}

uint32 Garrison::GetSpecialSpawnBuildingTime(uint32 idx)
{
    if (uint32 _time = GetBuildingData(idx, BUILDING_DATA_SPECIAL_SPAWN))
    {
        int diff = _time - time(nullptr);
        if (diff > 0)
            return diff;
    }
    return 0;
}

void Garrison::OnGossipTradeSkill(WorldObject* source)
{
    if (!source->HasFlag(UNIT_FIELD_NPC_FLAGS2, UNIT_NPC_FLAG2_TRADESKILL_NPC))
        return;

    TradeskillList const* trade = sGarrisonMgr.GetTradeSkill(source->GetEntry());
    if (!trade)
        return;

    //! ToDo: link npc with plot or something else about it.
    const Plot* plot = GetPlotWithNpc(source->GetEntry());
    if (!plot || !plot->BuildingInfo.PacketInfo)
        return;

    GarrBuildingEntry const* existingBuilding = sGarrBuildingStore.AssertEntry(plot->BuildingInfo.PacketInfo->GarrBuildingID);

    //! SMSG_GARRISON_OPEN_TRADESKILL_NPC
    WorldPackets::Garrison::GarrisonTradeSkillResponse tradeSkillPacket;
    tradeSkillPacket.GUID = source->GetGUID();
    for (auto const& tr : *trade)
    {
        bool find = false;
        for (uint32 &d : tradeSkillPacket.TradeSkill.SkillLineIDs)
        {
            if (d == tr.skillID)
            {
                find = true;
                break;
            }
        }
        if (!find)
            tradeSkillPacket.TradeSkill.SkillLineIDs.push_back(tr.skillID);
        tradeSkillPacket.TradeSkill.KnownAbilitySpellIDs.push_back(tr.spellID);
        tradeSkillPacket.PlayerConditionID.push_back(existingBuilding->UpgradeLevel < tr.reqBuildingLvl ? tr.conditionID : 0);
    }

    _owner->SendDirectMessage(tradeSkillPacket.Write());
}

bool Garrison::CanCastTradeSkill(ObjectGuid const& guid, uint32 spellID)
{
    Creature* source = ObjectAccessor::GetCreatureOrPetOrVehicle(*_owner, guid);
    if (!source)
        return false;

    TradeskillList const* trade = sGarrisonMgr.GetTradeSkill(source->GetEntry());
    if (!trade)
        return false;

    //! ToDo: link npc with plot or something else about it.
    const Plot* plot = GetPlotWithNpc(source->GetEntry());
    if (!plot || !plot->BuildingInfo.PacketInfo)
        return false;

    auto existingBuilding = sGarrBuildingStore.AssertEntry(plot->BuildingInfo.PacketInfo->GarrBuildingID);

    for (auto const& tr : *trade)
        if (tr.spellID == spellID)
            return (existingBuilding->UpgradeLevel >= tr.reqBuildingLvl);
    return false;
}

//! ClassHall upgrade: Artifact Power items gained from world quests and missions have a chance to grant double Artifact Power.
bool Garrison::hasLegionFall() const
{
    switch (_owner->getClass())
    {
        case CLASS_DEMON_HUNTER:
            return hasTallent(491);
        case CLASS_DRUID:
            return hasTallent(494);
        case CLASS_MONK:
            return hasTallent(500);
        case CLASS_WARLOCK:
            return hasTallent(512);
        case CLASS_SHAMAN:
            return hasTallent(509);
        case CLASS_DEATH_KNIGHT:
            return hasTallent(488);
        case CLASS_PRIEST:
            return hasTallent(503);
        case CLASS_ROGUE:
            return hasTallent(506);
        case CLASS_HUNTER:
            return hasTallent(497);
        case CLASS_PALADIN:
            return hasTallent(482);
        case CLASS_WARRIOR:
            return hasTallent(515);
        case CLASS_MAGE:
            return hasTallent(485);
        default:
            break;
    }
    return false;
}

//! Increase the number of Legendary items you can equip by 1.
bool Garrison::hasLegendLimitUp() const
{
    switch (_owner->getClass())
    {
        case CLASS_DEMON_HUNTER:
            return hasTallent(423);
        case CLASS_DRUID:
            return hasTallent(357);
        case CLASS_MONK:
            return hasTallent(258);
        case CLASS_WARLOCK:
            return hasTallent(368);
        case CLASS_SHAMAN:
            return hasTallent(42);
        case CLASS_DEATH_KNIGHT:
            return hasTallent(434);
        case CLASS_PRIEST:
            return hasTallent(456);
        case CLASS_ROGUE:
            return hasTallent(445);
        case CLASS_HUNTER:
            return hasTallent(379);
        case CLASS_PALADIN:
            return hasTallent(401);
        case CLASS_WARRIOR:
            return hasTallent(412);
        case CLASS_MAGE:
            return hasTallent(390);
        default:
            break;
    }
    return false;
}