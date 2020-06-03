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

#include "SocialPackets.h"

void WorldPackets::Social::SendContactList::Read()
{
    _worldPacket >> Flags;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Social::ContactInfo const& contact)
{
    data << contact.Guid;
    data << contact.WowAccountGuid;
    data << uint32(contact.VirtualRealmAddr);
    data << uint32(contact.NativeRealmAddr);
    data << uint32(contact.TypeFlags);
    data << uint8(contact.Status);
    data << uint32(contact.AreaID);
    data << uint32(contact.Level);
    data << uint32(contact.ClassID);
    data.WriteBits(contact.Notes.length(), 10);
    data.WriteString(contact.Notes);

    return data;
}

WorldPacket const* WorldPackets::Social::ContactList::Write()
{
    _worldPacket << Flags;
    _worldPacket.WriteBits(Contacts.size(), 8);
    for (auto const& v : Contacts)
        _worldPacket << v;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Social::FriendStatus::Write()
{
    _worldPacket << uint8(FriendResult);
    _worldPacket << Guid;
    _worldPacket << WowAccountGuid;
    _worldPacket << uint32(VirtualRealmAddress);
    _worldPacket << uint8(Status);
    _worldPacket << uint32(AreaID);
    _worldPacket << uint32(Level);
    _worldPacket << uint32(ClassID);
    _worldPacket.WriteBits(Notes.length(), 10);
    _worldPacket.WriteString(Notes);

    return &_worldPacket;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Social::QualifiedGUID& qGuid)
{
    data >> qGuid.VirtualRealmAddress;
    data >> qGuid.Guid;

    return data;
}

void WorldPackets::Social::AddFriend::Read()
{
    uint32 nameLength = _worldPacket.ReadBits(9);
    uint32 noteslength = _worldPacket.ReadBits(10);
    Name = _worldPacket.ReadString(nameLength);
    Notes = _worldPacket.ReadString(noteslength);
}

void WorldPackets::Social::DelFriend::Read()
{
    _worldPacket >> Player;
}

void WorldPackets::Social::SetContactNotes::Read()
{
    _worldPacket >> Player;
    Notes = _worldPacket.ReadString(_worldPacket.ReadBits(10));
}

void WorldPackets::Social::AddIgnore::Read()
{
    Name = _worldPacket.ReadString(_worldPacket.ReadBits(9));
}

void WorldPackets::Social::DelIgnore::Read()
{
    _worldPacket >> Player;
}

WorldPacket const* WorldPackets::Social::SocialQueueUpdateNotify::Write()
{
    _worldPacket << QueueID;

    _worldPacket << FriendGuid;
    _worldPacket << ApplicationGuid;
    _worldPacket << ApplicationLeaderGuid;

    _worldPacket << static_cast<uint32>(SocialQueueUpdates.size());

    _worldPacket.WriteBit(UnkBit);
    _worldPacket.FlushBits();

    for (auto const& v : SocialQueueUpdates)
    {
        _worldPacket.WriteBit(v.UnkBit);
        _worldPacket.WriteBits(v.Type, 2);
        _worldPacket.FlushBits();

        switch (v.Type)
        {
            case 2:
                _worldPacket << v.UnkData.UnkInt1;
                _worldPacket << v.UnkData.UnkInt2;
                _worldPacket << v.UnkData.UnkInt3;

                _worldPacket << v.UnkData.UnkByte;

                _worldPacket << v.UnkData.UnkInt4;

                _worldPacket.WriteBit(v.UnkData.UnkBit1);
                _worldPacket.WriteBit(v.UnkData.UnkBit2);
                _worldPacket.FlushBits();
                break;
            case 1:
                _worldPacket << v.SearchResult;
                break;
            default:
            {
                _worldPacket << static_cast<uint32>(v.UnkData2.UnkInts1.size());
                _worldPacket << static_cast<uint32>(v.UnkData2.UnkInts2.size());
                _worldPacket << static_cast<uint32>(v.UnkData2.UnkBytes.size());

                for (auto const& x : v.UnkData2.UnkInts1)
                    _worldPacket << x;

                for (auto const& x : v.UnkData2.UnkInts2)
                    _worldPacket << x;

                for (auto const& x : v.UnkData2.UnkBytes)
                    _worldPacket << x;

                _worldPacket.WriteBit(v.UnkData2.UnkBit1);
                _worldPacket.WriteBit(v.UnkData2.UnkBit2);
                _worldPacket.FlushBits();
                break;
            }
        }
    }

    return &_worldPacket;
}

void WorldPackets::Social::QuickJoinAutoAcceptRequests::Read()
{
    EnableAutoAccept = _worldPacket.ReadBit();
}

void WorldPackets::Social::QuickJoinRequestInvite::Read()
{
    uint16 strlen1 = _worldPacket.ReadBits(8);
    ApplyAsTank = _worldPacket.ReadBit();
    uint16 strlen2 = _worldPacket.ReadBits(8);
    ApplyAsHealer = _worldPacket.ReadBit();
    ApplyAsDamage = _worldPacket.ReadBit();
    _worldPacket >> UnkInt1;
    _worldPacket >> GroupGUID;
    _worldPacket >> UnkInt2;
    UnkString1 = _worldPacket.ReadString(strlen1);
    UnkString2 = _worldPacket.ReadString(strlen2);
}

void WorldPackets::Social::QuickJoinRespondToInvite::Read()
{
    _worldPacket >> GroupGUID;
    _worldPacket >> GUID;
    Accept = _worldPacket.ReadBit();
}

void WorldPackets::Social::QuickJoinSignalToastDisplayed::Read()
{
    _worldPacket >> GroupGUID;
    _worldPacket >> Priority;
    UnkGuids.resize(_worldPacket.read<uint32>());
    for (auto& v : UnkGuids)
        _worldPacket >> v;

    UnkBit1 = _worldPacket.ReadBit();
    UnkBit2 = _worldPacket.ReadBit();
}
