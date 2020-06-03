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

#include "WorldSession.h"
#include "Containers.h"
#include "HotfixPackets.h"
#include "DB2Store.h"

void WorldSession::HandleHotfixRequest(WorldPackets::Hotfix::HotfixRequest& packet)
{
    auto const& hotfixes = sDB2Manager.GetHotfixData();
    WorldPackets::Hotfix::HotfixResponse hotfixQueryResponse;
    hotfixQueryResponse.Hotfixes.reserve(packet.Hotfixes.size());
    for (auto hotfixId : packet.Hotfixes)
    {
        if (auto hotfix = Trinity::Containers::MapGetValuePtr(hotfixes, hotfixId))
        {
            auto storage = sDB2Manager.GetStorage(PAIR64_HIPART(hotfixId));
            if (!storage)
                continue;

            WorldPackets::Hotfix::HotfixData hotfixData;
            hotfixData.ID = hotfixId;
            hotfixData.RecordID = *hotfix;
            if (storage->HasRecord(hotfixData.RecordID))
            {
                hotfixData.Data = boost::in_place();
                storage->WriteRecord(hotfixData.RecordID, GetSessionDbcLocale(), *hotfixData.Data);
            }

            hotfixQueryResponse.Hotfixes.emplace_back(std::move(hotfixData));
        }
    }

    SendPacket(hotfixQueryResponse.Write());
}

void WorldSession::HandleDBQueryBulk(WorldPackets::Hotfix::DBQueryBulk& packet)
{
    auto store = sDB2Manager.GetStorage(packet.TableHash);
    if (!store)
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "DBQueryBulk:: client requested unused db2 storage: %u; can by finded in DB2Hashes", packet.TableHash);
        return;
    }

    for (auto const& rec : packet.Queries)
    {
        WorldPackets::Hotfix::DBReply response;
        response.TableHash = packet.TableHash;
        response.RecordID = rec.RecordID;

        if (store->HasRecord(rec.RecordID))
        {
            response.Allow = true;
            response.Timestamp = sWorld->GetGameTime();
            store->WriteRecord(rec.RecordID, GetSessionDbcLocale(), response.Data);
        }
        else
            response.Timestamp = time(nullptr);

        SendPacket(response.Write());
    }
}

void WorldSession::SendHotfixList(int32 version)
{
    SendPacket(WorldPackets::Hotfix::AvailableHotfixes(version, sDB2Manager.GetHotfixData()).Write());
}
