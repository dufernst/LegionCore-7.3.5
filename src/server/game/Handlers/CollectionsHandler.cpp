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

#include "WorldSession.h"
#include "CollectionMgr.h"
#include "CollectionPackets.h"
#include "PetBattle.h"

void WorldSession::HandleCollectionItemSetFavorite(WorldPackets::Collections::CollectionItemSetFavorite& collectionItemSetFavorite)
{
    switch (collectionItemSetFavorite.Type)
    {
        case WorldPackets::Collections::TOYBOX:
            _player->GetCollectionMgr()->ToySetFavorite(collectionItemSetFavorite.ID, collectionItemSetFavorite.IsFavorite);
            break;
        case WorldPackets::Collections::APPEARANCE:
            if (_player->GetCollectionMgr()->HasItemAppearance(collectionItemSetFavorite.ID))
                _player->GetCollectionMgr()->SetAppearanceIsFavorite(collectionItemSetFavorite.ID, collectionItemSetFavorite.IsFavorite);
            break;
        case WorldPackets::Collections::TRANSMOG_SET:
            break;
        default:
            break;
    }
}

void WorldSession::HandleMountClearFanfare(WorldPackets::Collections::MountClearFanfare& packet)
{
    if (_player->GetCollectionMgr()->HasMount(packet.spellID))
        _player->GetCollectionMgr()->UpdateAccountMounts(packet.spellID, MOUNT_FLAG_NONE);
}

void WorldSession::HandleBattlePetClearFanfare(WorldPackets::Collections::BattlePetClearFanfare& packet)
{
    if (auto battlePet = _player->GetBattlePet(packet.BattlePetGUID))
    {
        battlePet->Flags = battlePet->Flags & ~BATTLEPET_FLAG_GIFT;;
        battlePet->needSave = true;
    }
}
