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

#ifndef GOPackets_h__
#define GOPackets_h__

#include "Packet.h"
#include "WorldSession.h"

namespace WorldPackets
{
    namespace GameObject
    {
        class GameObjectUse final : public ClientPacket
        {
        public:
            GameObjectUse(WorldPacket&& packet) : ClientPacket(CMSG_GAME_OBJ_USE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
        };

        class GameObjReportUse final : public ClientPacket
        {
        public:
            GameObjReportUse(WorldPacket&& packet) : ClientPacket(CMSG_GAME_OBJ_REPORT_USE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
        };

        class GameObjectDespawn final : public ServerPacket
        {
        public:
            GameObjectDespawn(ObjectGuid guid) : ServerPacket(SMSG_GAME_OBJECT_DESPAWN, 16), ObjectGUID(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
        };

        class GameObjectUiAction final : public ServerPacket
        {
        public:
            GameObjectUiAction(ObjectGuid guid, uint32 Action) : ServerPacket(SMSG_GAME_OBJECT_UI_ACTION, 20), ObjectGUID(guid), ActionID(Action) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
            uint32 ActionID = 0;
        };

        class PageText final : public ServerPacket
        {
        public:
            PageText(ObjectGuid guid) : ServerPacket(SMSG_PAGE_TEXT, 16), GameObjectGUID(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid GameObjectGUID;
        };

        class GameObjectActivateAnimKit final : public ServerPacket
        {
        public:
            GameObjectActivateAnimKit() : ServerPacket(SMSG_GAME_OBJECT_ACTIVATE_ANIM_KIT, 16 + 4 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
            int32 AnimKitID = 0;
            bool Maintain = false;
        };

        class GoCustomAnim final : public ServerPacket
        {
        public:
            GoCustomAnim() : ServerPacket(SMSG_GAME_OBJECT_CUSTOM_ANIM, 21) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
            int32 CustomAnim = 0;
            bool PlayAsDespawn = false;
        };

        class GameObjectResetState final : public ServerPacket
        {
        public:
            GameObjectResetState() : ServerPacket(SMSG_GAME_OBJECT_RESET_STATE, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
        };

        class GameObjectPlaySpellVisual final : public ServerPacket
        {
        public:
            GameObjectPlaySpellVisual() : ServerPacket(SMSG_GAME_OBJECT_PLAY_SPELL_VISUAL, 16 + 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
            ObjectGuid ActivatorGUID;
            int32 SpellVisualID = 0;
        };

        class DestructibleBuildingDamage final : public ServerPacket
        {
        public:
            DestructibleBuildingDamage() : ServerPacket(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 16 * 3 + 8) { }

            WorldPacket const* Write() override;

            ObjectGuid Target;
            ObjectGuid Caster;
            ObjectGuid Owner;
            int32 Damage = 0;
            uint32 SpellID = 0;
        };

        class PlayObjectSound final : public ServerPacket
        {
        public:
            PlayObjectSound() : ServerPacket(SMSG_PLAY_OBJECT_SOUND, 32 + 16) { }

            WorldPacket const* Write() override;

            ObjectGuid SourceObjectGUID;
            ObjectGuid TargetObjectGUID;
            TaggedPosition<Position::XYZ> Pos;
            uint32 SoundId = 0;
        };

        class FishNotHooked final : public ServerPacket
        {
        public:
            FishNotHooked() : ServerPacket(SMSG_FISH_NOT_HOOKED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class FishEscaped final : public ServerPacket
        {
        public:
            FishEscaped() : ServerPacket(SMSG_FISH_ESCAPED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class GameObjectSetState final : public ServerPacket
        {
        public:
            GameObjectSetState() : ServerPacket(SMSG_GAME_OBJECT_SET_STATE, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid ObjectGUID;
            uint8 StateID = 0;
        };
    }
}
#endif // GOPackets_h__
