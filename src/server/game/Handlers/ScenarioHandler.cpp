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

#include "ScenarioPackets.h"
#include "WorldSession.h"
#include "Scenario.h"

void WorldSession::HandleQueryScenarioPOI(WorldPackets::Scenario::QueryScenarioPOI& packet)
{
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "HandleQueryScenarioPOI MissingScenarioPOITreeIDs %u", packet.MissingScenarioPOITreeIDs.size());

    WorldPackets::Scenario::ScenarioPOIs poIs;
    WorldPackets::Scenario::ScenarioPOIs::POIData& infos = poIs.PoiInfos[packet.MissingScenarioPOITreeIDs.size()];

    for (auto const& treeID : packet.MissingScenarioPOITreeIDs)
    {
        ScenarioPOIVector const* poi = sObjectMgr->GetScenarioPOIVector(treeID);
        if (!poi)
        {
            infos.CriteriaTreeID = treeID;
            continue;
        }

        infos.CriteriaTreeID = treeID;
        WorldPackets::Scenario::ScenarioPOIs::POIData::BlobData& blobData = infos.BlobDatas[poi->size()];

        for (ScenarioPOIVector::const_iterator itr = poi->begin(); itr != poi->end(); ++itr)
        {
            blobData.BlobID = itr->BlobID;
            blobData.MapID = itr->MapID;
            blobData.WorldMapAreaID = itr->WorldMapAreaID;
            blobData.Floor = itr->Floor;
            blobData.Priority = itr->Priority;
            blobData.Flags = itr->Flags;
            blobData.WorldEffectID = itr->WorldEffectID;
            blobData.PlayerConditionID = itr->PlayerConditionID;

            WorldPackets::Scenario::ScenarioPOIs::POIData::BlobData::POIPointData& points = blobData.Points[itr->points.size()];

            for (std::vector<ScenarioPOIPoint>::const_iterator itr2 = itr->points.begin(); itr2 != itr->points.end(); ++itr2)
            {
                points.X = itr2->x;
                points.Y = itr2->y;
            }
        }
    }

    SendPacket(poIs.Write());
}
