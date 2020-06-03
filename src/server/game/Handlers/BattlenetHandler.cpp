/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#include "WorldserverServiceDispatcher.h"
#include "BattlenetPackets.h"

void WorldSession::HandleBattlenetRequest(WorldPackets::Battlenet::Request& request)
{
    sServiceDispatcher.Dispatch(this, request.Method.GetServiceHash(), request.Method.Token, request.Method.GetMethodId(), std::move(request.Data));
}

void WorldSession::HandleBattlenetRequestRealmListTicket(WorldPackets::Battlenet::RequestRealmListTicket& requestRealmListTicket)
{
    SetRealmListSecret(requestRealmListTicket.Secret);

    WorldPackets::Battlenet::RealmListTicket realmListTicket;
    realmListTicket.Token = requestRealmListTicket.Token;
    realmListTicket.Allow = true;
    realmListTicket.Ticket << "WorldserverRealmListTicket";

    SendPacket(realmListTicket.Write());
}

void WorldSession::SendBattlenetResponse(uint32 serviceHash, uint32 methodId, uint32 token, pb::Message const* response)
{
    WorldPackets::Battlenet::Response bnetResponse;
    bnetResponse.BnetStatus = ERROR_OK;
    bnetResponse.Method.Type = MAKE_PAIR64(methodId, serviceHash);
    bnetResponse.Method.ObjectId = 1;
    bnetResponse.Method.Token = token;

    if (methodId == 1 && serviceHash == 1259359315)
    {
        bnetResponse.Method.ObjectId = 0;
        bnetResponse.Data << uint8(13);
        bnetResponse.Data << uint8(88);
        bnetResponse.Data << uint8(88);
        bnetResponse.Data << uint8(0);
        bnetResponse.Data << uint8(0);
        bnetResponse.Data << uint8(21);
        bnetResponse.Data << uint8(121);
        bnetResponse.Data << uint8(116);
        bnetResponse.Data << uint8(102);
        bnetResponse.Data << uint8(112);
        bnetResponse.Data << uint8(26);
        bnetResponse.Data << uint8(32);
        bnetResponse.Data << uint8(58);
        bnetResponse.Data << uint8(33);
        bnetResponse.Data << uint8(45);
        bnetResponse.Data << uint8(178);
        bnetResponse.Data << uint8(180);
        bnetResponse.Data << uint8(242);
        bnetResponse.Data << uint8(185);
        bnetResponse.Data << uint8(84);
        bnetResponse.Data << uint8(25);
        bnetResponse.Data << uint8(67);
        bnetResponse.Data << uint8(161);
        bnetResponse.Data << uint8(163);
        bnetResponse.Data << uint8(204);
        bnetResponse.Data << uint8(26);
        bnetResponse.Data << uint8(172);
        bnetResponse.Data << uint8(195);
        bnetResponse.Data << uint8(90);
        bnetResponse.Data << uint8(48);
        bnetResponse.Data << uint8(140);
        bnetResponse.Data << uint8(128);
        bnetResponse.Data << uint8(95);
        bnetResponse.Data << uint8(135);
        bnetResponse.Data << uint8(80);
        bnetResponse.Data << uint8(124);
        bnetResponse.Data << uint8(207);
        bnetResponse.Data << uint8(151);
        bnetResponse.Data << uint8(22);
        bnetResponse.Data << uint8(94);
        bnetResponse.Data << uint8(213);
        bnetResponse.Data << uint8(136);
        bnetResponse.Data << uint8(46);
        bnetResponse.Data << uint8(211);
    }
    else if (response->ByteSize())
    {
        bnetResponse.Data.resize(response->ByteSize());
        response->SerializePartialToArray(bnetResponse.Data.contents(), response->ByteSize());
    }

    SendPacket(bnetResponse.Write());
}

void WorldSession::SendBattlenetResponse(uint32 serviceHash, uint32 methodId, uint32 token, uint32 status)
{
    WorldPackets::Battlenet::Response bnetResponse;
    bnetResponse.BnetStatus = BattlenetRpcErrorCode(status);
    bnetResponse.Method.Type = MAKE_PAIR64(methodId, serviceHash);
    bnetResponse.Method.ObjectId = 1;
    bnetResponse.Method.Token = token;

    SendPacket(bnetResponse.Write());
}

void WorldSession::SendBattlenetRequest(uint32 serviceHash, uint32 methodId, pb::Message const* request, std::function<void(MessageBuffer)> callback)
{
    _battlenetResponseCallbacks[_battlenetRequestToken] = std::move(callback);
    SendBattlenetRequest(serviceHash, methodId, request);
}

void WorldSession::SendBattlenetRequest(uint32 serviceHash, uint32 methodId, pb::Message const* request)
{
    WorldPackets::Battlenet::Notification notification;
    notification.Method.Type = MAKE_PAIR64(methodId, serviceHash);
    notification.Method.ObjectId = 1;
    notification.Method.Token = _battlenetRequestToken++;

    if (request->ByteSize())
    {
        notification.Data.resize(request->ByteSize());
        request->SerializePartialToArray(notification.Data.contents(), request->ByteSize());
    }

    SendPacket(notification.Write());
}
