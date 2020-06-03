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

#ifndef UpdatePackets_h__
#define UpdatePackets_h__

#include "Packet.h"

namespace WorldPackets
{
    namespace Update
    {
        class DestroyArenaUnit final : public ServerPacket
        {
        public:
            DestroyArenaUnit() : ServerPacket(SMSG_DESTROY_ARENA_UNIT, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
        };

        class MapObjEvents final : public ServerPacket
        {
        public:
            MapObjEvents() : ServerPacket(SMSG_MAP_OBJ_EVENTS, 12) { }

            WorldPacket const* Write() override;

            uint32 UniqueID = 0;
            uint32 DataSize = 0;
            std::vector<uint8> Unk2;
        };

        class SetAnimTimer final : public ServerPacket
        {
        public:
            SetAnimTimer() : ServerPacket(SMSG_SET_ANIM_TIER, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint8 Tier = 0;
        };

        struct VignetteInstanceIDList
        {
            GuidVector IDs;
        };

        struct VignetteClientData
        {
            VignetteClientData(ObjectGuid guid, Position pos, int32 vignetteID, int32 zoneID) : ObjGUID(guid), Pos(pos), VignetteID(vignetteID), ZoneID(zoneID) { }

            ObjectGuid ObjGUID;
            TaggedPosition<Position::XYZ> Pos;
            int32 VignetteID = 0;
            int32 ZoneID = 0;
        };

        struct VignetteClientDataSet
        {
            VignetteInstanceIDList IdList;
            std::vector<VignetteClientData> Data;
        };

        class VignetteUpdate  final : public ServerPacket
        {
        public:
            VignetteUpdate() : ServerPacket(SMSG_VIGNETTE_UPDATE, 20 + 1) { }
            VignetteUpdate(bool update) : ServerPacket(SMSG_VIGNETTE_UPDATE, 20 + 1), ForceUpdate(update) { }

            WorldPacket const* Write() override;

            VignetteClientDataSet Updated;
            VignetteClientDataSet Added;
            VignetteInstanceIDList Removed;
            bool ForceUpdate = false;
        };
    }
}

#endif // UpdatePackets_h__
