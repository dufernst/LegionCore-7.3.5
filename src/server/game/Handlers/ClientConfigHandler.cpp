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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ClientConfigPackets.h"
#include "zlib.h"

void WorldSession::HandleRequestAccountData(WorldPackets::ClientConfig::RequestAccountData& request)
{
    if (request.DataType >= NUM_ACCOUNT_DATA_TYPES)
        return;

    AccountData const* adata = GetAccountData(AccountDataType(request.DataType));

    WorldPackets::ClientConfig::UpdateAccountData data;
    data.Player = _player ? _player->GetGUID() : ObjectGuid::Empty;
    data.Time = adata->Time;
    data.Size = adata->Data.size();
    data.DataType = request.DataType;

    if (data.Size)
    {
        auto destSize = compressBound(data.Size);
        data.CompressedData.resize(destSize);
        if (compress(data.CompressedData.contents(), &destSize, reinterpret_cast<uint8 const*>(adata->Data.c_str()), data.Size) != Z_OK)
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "RAD: Failed to compress account data");
            return;
        }

        data.CompressedData.resize(destSize);
    }

    SendPacket(data.Write());
}

void WorldSession::HandleUpdateAccountData(WorldPackets::ClientConfig::UserClientUpdateAccountData& packet)
{
    if (packet.DataType > NUM_ACCOUNT_DATA_TYPES)
        return;

    if (packet.Size == 0)
    {
        SetAccountData(AccountDataType(packet.DataType));
        return;
    }

    if (packet.Size > 0xFFFF)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "UAD: Account data packet too big, size %u", packet.Size);
        return;
    }

    ByteBuffer dest;
    dest.resize(packet.Size);

    uLongf realSize = packet.Size;
    if (uncompress(dest.contents(), &realSize, packet.CompressedData.contents(), packet.CompressedData.size()) != Z_OK)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "UAD: Failed to decompress account data");
        return;
    }

    std::string adata;
    dest >> adata;

    SetAccountData(AccountDataType(packet.DataType), packet.Time, adata);
}

void WorldSession::HandleUpdateClientSettings(WorldPackets::ClientConfig::UpdateClientSettings& /*packet*/)
{ }

void WorldSession::HandleSaveClientVariables(WorldPackets::ClientConfig::SaveClientVariables& /*packet*/)
{ }

void WorldSession::HandleGetRemainingGameTime(WorldPackets::ClientConfig::GetRemainingGameTime& /*packet*/)
{ }
