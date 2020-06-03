#include "CommentatorPackets.h"

void WorldPackets::Commentator::CommentatorEnable::Read()
{
    _worldPacket >> Enable;
}

void WorldPackets::Commentator::CommentatorGetMapInfo::Read()
{
    uint8 playerNameLen = _worldPacket.ReadBits(6);
    std::string PlayerName = _worldPacket.ReadString(playerNameLen);
}

void WorldPackets::Commentator::CommentatorGetPlayerInfo::Read()
{
    uint32 MapId;
    _worldPacket >> MapId;
    uint32 Realm;
    uint16 Server;
    uint8 Type;
    _worldPacket >> Realm;
    _worldPacket >> Server;
    _worldPacket >> Type;
}

void WorldPackets::Commentator::CommentatorEnterInstance::Read()
{
    uint32 MapId, InstanceId, DifficultyId;
    _worldPacket >> MapId;
    uint32 Realm;
    uint16 Server;
    uint8 Type;
    _worldPacket >> Realm;
    _worldPacket >> Server;
    _worldPacket >> Type;
    _worldPacket >> InstanceId;
    _worldPacket >> DifficultyId;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::ServerSpec const& server)
{
    data << server.Realm;
    data << server.Server;
    data << server.Type;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::CommentatorPlayer const& player)
{
    data << player.Guid;
    data << player.UserServer;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::CommentatorTeam const& team)
{
    data << team.Guid;
    data << static_cast<int32>(team.Players.size());

    for (auto const& player : team.Players)
        data << player;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::CommentatorInstance const& instance)
{
    data << instance.MapID;
    data << instance.WorldServer;
    data << instance.InstanceID;
    data << instance.Status;

    for (auto const& team : instance.Teams)
        data << team;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::CommentatorMap const& map)
{
    data << map.TeamSize;
    data << map.DifficultyID;
    data << map.MinLevelRange;
    data << map.MaxLevelRange;
    data << static_cast<int32>(map.Instances.size());

    for (auto const& instance : map.Instances)
        data << instance;

    return data;
}

WorldPacket const* WorldPackets::Commentator::CommentatorMapInfo::Write()
{
    _worldPacket << PlayerInstanceID;
    _worldPacket << static_cast<int32>(Maps.size());

    for (auto const& map : Maps)
        _worldPacket << map;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::CommentatorSpellChargeEntry const& entry)
{
    data << entry.Unk;
    data << entry.Unk2;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::SpellHistoryEntry const& entry)
{
    data << entry.Unk;
    data << entry.Unk2;
    data << entry.Unk3;
    data << entry.Unk4;
    data << entry.Unk5;

    data.WriteBit(entry.Unk6);
    data.WriteBit(entry.Unk7);
    data.WriteBit(entry.Unk8);
    data.FlushBits();

    if (entry.Unk6)
        data << uint32(0);

    if (entry.Unk7)
        data << uint32(0);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::SpellChargeEntry const& entry)
{
    data << entry.Unk;
    data << entry.Unk2;
    data << entry.Unk3;
    data << entry.Unk4;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Commentator::CommentatorPlayerData const& pdata)
{
    data << pdata.PlayerGUID;
    data << pdata.FactionIndex;
    data << pdata.DamageDone;
    data << pdata.Kills;
    data << pdata.Deaths;
    data << pdata.DamageTaken;
    data << pdata.HealingDone;
    data << pdata.HealingTaken;
    data << pdata.SpecID;
    data << static_cast<int32>(pdata.SpellHistoryEntries.size());
    data << static_cast<int32>(pdata.SpellChargeEntries.size());
    data << static_cast<int32>(pdata.CommentatorSpellChargeEntries.size());

    for (auto const& entry : pdata.SpellChargeEntries)
        data << entry;

    for (auto const& entry : pdata.CommentatorSpellChargeEntries)
        data << entry;

    for (auto const& entry : pdata.SpellHistoryEntries)
        data << entry;

    return data;
}

WorldPacket const* WorldPackets::Commentator::CommentatorPlayerInfo::Write()
{
    _worldPacket << MapID;
    _worldPacket << WorldServer;
    _worldPacket << InstanceID;
    _worldPacket << static_cast<int32>(PlayerInfo.size());
    _worldPacket.WriteBit(true);

    for (auto const& info : PlayerInfo)
        _worldPacket << info;

    return &_worldPacket;
}

WorldPacket const * WorldPackets::Commentator::CommentatorStateChanged::Write()
{
    _worldPacket << Guid;
    _worldPacket.WriteBit(true);

    return &_worldPacket;
}
