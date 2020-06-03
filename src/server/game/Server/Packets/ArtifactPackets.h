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

#ifndef ArtifactPackets_h__
#define ArtifactPackets_h__

#include "Packet.h"

namespace WorldPackets
{
    namespace Artifact
    {
        class XpGain final : public ServerPacket
        {
        public:
            XpGain() : ServerPacket(SMSG_ARTIFACT_XP_GAIN, 16 + 8) { }

            WorldPacket const* Write() override;

            ObjectGuid ArtifactGUID;
            uint64 Xp = 0;
        };

        class AddPower final : public ClientPacket
        {
        public:
            AddPower(WorldPacket&& packet) : ClientPacket(CMSG_ARTIFACT_ADD_POWER, std::move(packet)) { }

            void Read() override;

            struct DataStruct
            {
                uint32 PowerID = 0;
                uint8 Rank = 0;
            };

            std::vector<DataStruct> Powers;
            ObjectGuid ArtifactGUID;
            ObjectGuid GameObjectGUID;
        };

        class ConfirmRespec final : public ClientPacket
        {
        public:
            ConfirmRespec(WorldPacket&& packet) : ClientPacket(CMSG_CONFIRM_ARTIFACT_RESPEC, std::move(packet)) { }

            void Read() override;

            ObjectGuid ArtifactGUID;
            ObjectGuid NpcGUID;
        };

        class SetAppearance final : public ClientPacket
        {
        public:
            SetAppearance(WorldPacket&& packet) : ClientPacket(CMSG_ARTIFACT_SET_APPEARANCE, std::move(packet)) { }

            void Read() override;

            ObjectGuid ArtifactGUID;
            ObjectGuid GameObjectGUID;
            uint32 AppearanceID = 0;
        };

        class ForgeOpenResult final : public ServerPacket
        {
        public:
            ForgeOpenResult() : ServerPacket(SMSG_ARTIFACT_FORGE_OPENED, 16 + 16) { }

            WorldPacket const* Write() override;
            
            ObjectGuid ArtifactGUID;
            ObjectGuid GameObjectGUID;
        };

        class ArtifactKnowledge final : public ServerPacket
        {
        public:
            ArtifactKnowledge() : ServerPacket(SMSG_ARTIFACT_KNOWLEDGE, 1 + 4) { }

            WorldPacket const* Write() override;

            int32 ArtifactCategoryID = 0;
            int8 KnowledgeLevel = 0;
        };

        class RespecResult final : public ServerPacket
        {
        public:
            RespecResult() : ServerPacket(SMSG_ARTIFACT_RESPEC_CONFIRM, 16 + 16) { }

            WorldPacket const* Write() override;
            
            ObjectGuid ArtifactGUID;
            ObjectGuid NpcGUID;
        };

        //< SMSG_ARTIFACT_FORGE_CLOSE
        class NullSmsg final : public ServerPacket
        {
        public:
            NullSmsg(OpcodeServer opcode) : ServerPacket(opcode, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class ArtifactTraitsRefunded final : public ServerPacket
        {
        public:
            ArtifactTraitsRefunded() : ServerPacket(SMSG_ARTIFACT_TRAITS_REFUNDED, 16 + 8) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            uint32 UnkInt = 0;
            uint32 UnkInt2 = 0;
        };

        class ArtifactAddRelicTalent final : public ClientPacket
        {
        public:
            ArtifactAddRelicTalent(WorldPacket&& packet) : ClientPacket(CMSG_ARTIFACT_ADD_RELIC_TALENT, std::move(packet)) { }

            void Read() override;

            ObjectGuid ArtifactGUID;
            ObjectGuid GameObjectGUID;
            uint32 SlotIndex = 0;
            uint8 TalentIndex = 0;
        };

        class ArtifactAttuneSocketedRelic final : public ClientPacket
        {
        public:
            ArtifactAttuneSocketedRelic(WorldPacket&& packet) : ClientPacket(CMSG_ARTIFACT_ATTUNE_SOCKETED_RELIC, std::move(packet)) { }

            void Read() override;

            ObjectGuid ArtifactGUID;
            ObjectGuid GameObjectGUID;
            uint32 RelicSlotIndex = 0;
        };

        class ArtifactAttunePreviewdRelic final : public ClientPacket
        {
        public:
            ArtifactAttunePreviewdRelic(WorldPacket&& packet) : ClientPacket(CMSG_ARTIFACT_ATTUNE_PREVIEW_RELIC, std::move(packet)) { }

            void Read() override;

            ObjectGuid RelicGUID;
            ObjectGuid GameObjectGUID;
        };

        class ArtifactAttuneSocketedRelicData final : public ServerPacket
        {
        public:
            ArtifactAttuneSocketedRelicData() : ServerPacket(SMSG_ARTIFACT_ATTUNE_SOCKETED_RELIC_DATA, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid ArtifactGUID;
            uint32 Result = 0; // not 100% sure
        };
    }
}

#endif // ArtifactPackets_h__
