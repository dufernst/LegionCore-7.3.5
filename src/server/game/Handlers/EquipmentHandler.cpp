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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "EquipmentSetPackets.h"
#include "CollectionMgr.h"

void WorldSession::HandleSaveEquipmentSet(WorldPackets::EquipmentSet::SaveEquipmentSet& packet)
{
    if (packet.Set.SetID >= MAX_EQUIPMENT_SET_INDEX)
        return;

    if (packet.Set.Type > EquipmentSetInfo::TRANSMOG)
        return;

    Player* player = GetPlayer();

    for (uint8 i = 0; i < EQUIPEMENT_SET_SLOTS; ++i)
    {
        if (!(packet.Set.IgnoreMask & (1 << i)))
        {
            if (packet.Set.Type == EquipmentSetInfo::EQUIPMENT)
            {
                packet.Set.Appearances[i] = 0;

                ObjectGuid const& itemGuid = packet.Set.Pieces[i];

                if (!itemGuid.IsEmpty())
                {
                    Item* item = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, i); /// cheating check 1 (item equipped but sent empty guid)
                    if (!item)
                        return;

                    if (item->GetGUID() != itemGuid) /// cheating check 2 (sent guid does not match equipped item)
                        return;
                }
                else
                    packet.Set.IgnoreMask |= 1 << i;
            }
            else
            {
                uint32 app = packet.Set.Appearances[i];
                packet.Set.Pieces[i].Clear();
                if (packet.Set.Appearances[i])
                {
                    if (!sItemModifiedAppearanceStore.LookupEntry(packet.Set.Appearances[i]))
                        return;

                    // not needed, checked on retail
                    //if (!player->GetCollectionMgr()->HasItemAppearance(app))
                        //return;
                }
                else
                    packet.Set.IgnoreMask |= 1 << i;
            }
        }
        else
        {
            packet.Set.Pieces[i].Clear();
            packet.Set.Appearances[i] = 0;
        }
    }

    packet.Set.IgnoreMask &= 0x7FFFF;
    if (packet.Set.Type == EquipmentSetInfo::EQUIPMENT)
    {
        packet.Set.Enchants[0] = 0;
        packet.Set.Enchants[1] = 0;
    }
    else
    {
        auto validateIllusion = [this](uint32 enchantId) -> bool
        {
            SpellItemEnchantmentEntry const* illusion = sSpellItemEnchantmentStore.LookupEntry(enchantId);
            if (!illusion)
                return false;

            if (!illusion->ItemVisual || !(illusion->Flags & ENCHANTMENT_COLLECTABLE))
                return false;

            if (!sConditionMgr->IsPlayerMeetingCondition(_player, illusion->TransmogPlayerConditionID))
                return false;

            if (illusion->ScalingClassRestricted > 0 && uint8(illusion->ScalingClassRestricted) != _player->getClass())
                return false;

            return true;
        };

        if (packet.Set.Enchants[0] && !validateIllusion(packet.Set.Enchants[0]))
            return;

        if (packet.Set.Enchants[1] && !validateIllusion(packet.Set.Enchants[1]))
            return;
    }

    player->SetEquipmentSet(packet.Set);
}

void WorldSession::HandleDeleteEquipmentSet(WorldPackets::EquipmentSet::DeleteEquipmentSet& packet)
{
    _player->DeleteEquipmentSet(packet.ID);
}

void WorldSession::HandleEquipmentSetUse(WorldPackets::EquipmentSet::UseEquipmentSet& packet)
{
    ObjectGuid ignoredItemGuid;
    ignoredItemGuid.SetRawValue(0x0C00040000000000, 0xFFFFFFFFFFFFFFFF);

    for (uint8 i = 0; i < EQUIPEMENT_SET_SLOTS; ++i)
    {
        if (packet.Items[i].Item == ignoredItemGuid)
            continue;

        if (_player->isInCombat() && i != EQUIPMENT_SLOT_MAINHAND && i != EQUIPMENT_SLOT_OFFHAND)
            continue;

        Item* item = _player->GetItemByGuid(packet.Items[i].Item);

        uint16 dstPos = i | (INVENTORY_SLOT_BAG_0 << 8);

        if (!item)
        {
            Item* uItem = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!uItem)
                continue;

            ItemPosCountVec itemPosCountVec;
            InventoryResult inventoryResult = _player->CanStoreItem(NULL_BAG, NULL_SLOT, itemPosCountVec, uItem, false);
            if (inventoryResult == EQUIP_ERR_OK)
            {
                _player->RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                _player->StoreItem(itemPosCountVec, uItem, true);
            }
            else
                _player->SendEquipError(inventoryResult, uItem);
            continue;
        }

        if (item->GetPos() == dstPos)
            continue;

        _player->SwapItem(item->GetPos(), dstPos);
    }

    WorldPackets::EquipmentSet::UseEquipmentSetResult result;
    result.GUID = packet.GUID;
    result.Reason = 0; // 4 - inventory is full, 0 - ok, else failed                                     
    SendPacket(result.Write());
}

void WorldSession::HandleAssignEquipmentSetSpec(WorldPackets::EquipmentSet::AssignEquipmentSetSpec& /*packet*/)
{

}
