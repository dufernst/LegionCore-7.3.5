/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#ifndef CollectionPackets_h__
#define CollectionPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Collections
    {
        enum CollectionType : int32
        {
            NONE            = -1,
            TOYBOX          = 1,
            APPEARANCE      = 3,
            TRANSMOG_SET    = 4
        };

        class CollectionItemSetFavorite final : public ClientPacket
        {
        public:
            CollectionItemSetFavorite(WorldPacket&& packet) : ClientPacket(CMSG_COLLECTION_ITEM_SET_FAVORITE, std::move(packet)) { }

            void Read() override;

            CollectionType Type = NONE;
            uint32 ID = 0;
            bool IsFavorite = false;
        };

        class MountClearFanfare final : public ClientPacket
        {
        public:
            MountClearFanfare(WorldPacket&& packet) : ClientPacket(CMSG_MOUNT_CLEAR_FANFARE, std::move(packet)) { }

            void Read() override;
            uint32 spellID = 0;
        };

        class BattlePetClearFanfare final : public ClientPacket
        {
        public:
            BattlePetClearFanfare(WorldPacket&& packet) : ClientPacket(CMSG_BATTLE_PET_CLEAR_FANFARE, std::move(packet)) { }

            void Read() override;
            ObjectGuid BattlePetGUID;
        };
    }
}

#endif // CollectionPackets_h__
