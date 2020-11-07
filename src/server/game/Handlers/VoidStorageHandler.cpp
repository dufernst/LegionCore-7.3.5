/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
#include "World.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include <utility>
#include "ItemPackets.h"
#include "VoidStoragePackets.h"
#include "DatabaseEnv.h"

void WorldSession::SendVoidStorageTransferResult(VoidTransferError result)
{
    SendPacket(WorldPackets::VoidStorage::VoidTransferResult(result).Write());
}

void WorldSession::SendVoidStorageFailed(int8 reason /*= 0*/)
{
    SendPacket(WorldPackets::VoidStorage::VoidStorageFailed(reason << 7).Write());
}

void WorldSession::HandleVoidStorageUnlock(WorldPackets::VoidStorage::UnlockVoidStorage& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetNPCIfCanInteractWith(packet.Npc, UNIT_NPC_FLAG_VAULTKEEPER))
        return;

    if (player->IsVoidStorageUnlocked())
        return;

    player->ModifyMoney(-int64(VOID_STORAGE_UNLOCK));
    player->UnlockVoidStorage();
}

void WorldSession::HandleVoidStorageQuery(WorldPackets::VoidStorage::QueryVoidStorage& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetNPCIfCanInteractWith(packet.Npc, UNIT_NPC_FLAG_TRANSMOGRIFIER | UNIT_NPC_FLAG_VAULTKEEPER);
    if (!unit)
    {
        SendVoidStorageFailed(1);
        return;
    }

    if (!player->IsVoidStorageUnlocked())
    {
        SendVoidStorageFailed(1);
        return;
    }

    WorldPackets::VoidStorage::VoidStorageContents voidStorageContents;
    voidStorageContents.Items.reserve(VOID_STORAGE_MAX_SLOT);

    for (uint8 i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
    {
        VoidStorageItem* item = player->GetVoidStorageItem(i);
        if (!item)
            continue;

        WorldPackets::VoidStorage::VoidItem voidItem;
        voidItem.Guid = ObjectGuid::Create<HighGuid::Item>(item->ItemId);
        voidItem.Creator = item->item ? item->item->GetGuidValue(ITEM_FIELD_CREATOR) : item->CreatorGuid;
        voidItem.Slot = i;
        if (item->item)
            voidItem.Item.Initialize(item->item);
        else
            voidItem.Item.Initialize(item);

        voidStorageContents.Items.push_back(voidItem);
    }

    SendPacket(voidStorageContents.Write());
}

void WorldSession::HandleVoidStorageTransfer(WorldPackets::VoidStorage::VoidStorageTransfer& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetNPCIfCanInteractWith(packet.Npc, UNIT_NPC_FLAG_VAULTKEEPER))
        return;

    if (!player->IsVoidStorageUnlocked())
        return;

    if (uint8(packet.Deposits.size()) > player->GetNumOfVoidStorageFreeSlots())
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_FULL);
        return;
    }

    uint32 freeBagSlots = 0;
    if (!packet.Withdrawals.empty())
    {
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if (Bag* bag = player->GetBagByPos(i))
                freeBagSlots += bag->GetFreeSlots();

        uint8 inventoryEnd = INVENTORY_SLOT_ITEM_START + _player->GetInventorySlotCount();
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < inventoryEnd; i++)
            if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                ++freeBagSlots;
    }

    if (packet.Withdrawals.size() > freeBagSlots)
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
        return;
    }

    int64 cost = uint64(packet.Deposits.size() * VOID_STORAGE_STORE_ITEM);
    if (!player->HasEnoughMoney(cost))
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NOT_ENOUGH_MONEY);
        return;
    }

    WorldPackets::VoidStorage::VoidStorageTransferChanges voidStorageTransferChanges;
    voidStorageTransferChanges.AddedItems.reserve(VOID_STORAGE_MAX_DEPOSIT);
    voidStorageTransferChanges.RemovedItems.reserve(VOID_STORAGE_MAX_DEPOSIT);

    uint8 depositCount = 0;
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for (size_t i = 0; i < packet.Deposits.size(); ++i)
    {
        Item* item = player->GetItemByGuid(packet.Deposits[i]);
        if (!item)
            continue;

        VoidStorageItem itemVS(sObjectMgr->GenerateVoidStorageItemId(), item, true);

        WorldPackets::VoidStorage::VoidItem voidItem;
        voidItem.Guid = ObjectGuid::Create<HighGuid::Item>(itemVS.ItemId);
        voidItem.Creator = item->GetGuidValue(ITEM_FIELD_CREATOR);
        voidItem.Item.Initialize(item);
        voidItem.Slot = player->AddVoidStorageItem(std::move(itemVS));

        voidStorageTransferChanges.AddedItems.push_back(voidItem);

        item->SetNotRefundable(player); // makes the item no longer refundable
        player->MoveItemFromInventory(item, true);

        item->DeleteFromInventoryDB(trans);     // deletes item from character's inventory
        item->SaveToDB(trans);                  // recursive and not have transaction guard into self, item not in inventory and can be save standalone

        ++depositCount;
    }

    player->ModifyMoney(-(depositCount * cost));

    for (size_t i = 0; i < packet.Withdrawals.size(); ++i)
    {
        uint8 slot = 0;
        VoidStorageItem* itemVS = player->GetVoidStorageItem(packet.Withdrawals[i].GetCounter(), slot);
        if (!itemVS)
            continue;

        Item* item = itemVS->item;
        if(item)
        {
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
                return;
            }
            item = player->StoreItem(dest, item, true);
        }
        else
        {
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemVS->ItemEntry, 1);
            if (msg != EQUIP_ERR_OK)
            {
                SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
                return;
            }
            item = player->StoreNewItem(dest, itemVS->ItemEntry, true, itemVS->ItemRandomPropertyId, GuidSet());

            item->SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, itemVS->ItemSuffixFactor);
            item->SetGuidValue(ITEM_FIELD_CREATOR, itemVS->CreatorGuid);
            item->SetModifier(ITEM_MODIFIER_UPGRADE_ID, itemVS->ItemUpgradeId);
        }
        if (!item)
            return;

        item->SetBinding(true);

        voidStorageTransferChanges.RemovedItems.push_back(ObjectGuid::Create<HighGuid::Item>(itemVS->ItemId));
        itemVS->item = nullptr;

        player->DeleteVoidStorageItem(slot);
    }

    SendPacket(voidStorageTransferChanges.Write());

    SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NO_ERROR);
    player->SaveInventoryAndGoldToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

void WorldSession::HandleVoidSwapItem(WorldPackets::VoidStorage::SwapVoidItem& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (player->GetNPCIfCanInteractWith(packet.Npc, UNIT_NPC_FLAG_VAULTKEEPER))
        return;

    if (!player->IsVoidStorageUnlocked())
        return;

    uint8 oldSlot;
    if (!player->GetVoidStorageItem(packet.VoidItemGuid.GetCounter(), oldSlot))
        return;

    bool usedDestSlot = player->GetVoidStorageItem(packet.DstSlot) != nullptr;
    ObjectGuid itemIdDest;
    if (usedDestSlot)
        itemIdDest = ObjectGuid::Create<HighGuid::Item>(player->GetVoidStorageItem(packet.DstSlot)->ItemId);

    if (!player->SwapVoidStorageItem(oldSlot, packet.DstSlot))
    {
        SendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return;
    }

    WorldPackets::VoidStorage::VoidItemSwapResponse voidItemSwapResponse;
    voidItemSwapResponse.VoidItemA = packet.VoidItemGuid;
    voidItemSwapResponse.VoidItemSlotA = packet.DstSlot;
    if (usedDestSlot)
    {
        voidItemSwapResponse.VoidItemB = itemIdDest;
        voidItemSwapResponse.VoidItemSlotB = oldSlot;
    }

    SendPacket(voidItemSwapResponse.Write());
}
