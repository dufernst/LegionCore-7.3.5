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

#include "ObjectMgr.h"
#include "ItemPackets.h"
#include "TradeData.h"
#include "DatabaseEnv.h"
#include "BankPackets.h"
#include "SpellPackets.h"
#include "PlayerDefines.h"
#include "ArtifactPackets.h"

void WorldSession::HandleSplitItemOpcode(WorldPackets::Item::SplitItem& splitItem)
{
    if (!splitItem.Inv.Items.empty())
        return;

    uint16 src = ((splitItem.FromPackSlot << 8) | splitItem.FromSlot);
    uint16 dst = ((splitItem.ToPackSlot << 8) | splitItem.ToSlot);

    if (src == dst)
        return;

    if (!splitItem.Quantity)
        return;

    if (!_player->IsValidPos(splitItem.FromPackSlot, splitItem.FromSlot, true))
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!_player->IsValidPos(splitItem.ToPackSlot, splitItem.ToSlot, false))
    {
        _player->SendEquipError(EQUIP_ERR_WRONG_SLOT);
        return;
    }

    _player->SplitItem(src, dst, splitItem.Quantity);
}

void WorldSession::HandleSwapInvItemOpcode(WorldPackets::Item::SwapInvItem& swapInvItem)
{
    if (swapInvItem.Inv.Items.size() != 2)
        return;

    if (swapInvItem.Slot1 == swapInvItem.Slot2)
        return;

    if (!_player->IsValidPos(INVENTORY_SLOT_BAG_0, swapInvItem.Slot1, true))
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!_player->IsValidPos(INVENTORY_SLOT_BAG_0, swapInvItem.Slot2, true))
    {
        _player->SendEquipError(EQUIP_ERR_WRONG_SLOT);
        return;
    }

    if (_player->IsBankPos(INVENTORY_SLOT_BAG_0, swapInvItem.Slot1) && !CanUseBank())
        return;

    if (_player->IsBankPos(INVENTORY_SLOT_BAG_0, swapInvItem.Slot2) && !CanUseBank())
        return;

    if (_player->IsReagentBankPos(INVENTORY_SLOT_BAG_0, swapInvItem.Slot1) && !_player->CanUseReagentBank())
        return;

    if (_player->IsReagentBankPos(INVENTORY_SLOT_BAG_0, swapInvItem.Slot2) && !_player->CanUseReagentBank())
        return;

    uint16 src = ((INVENTORY_SLOT_BAG_0 << 8) | swapInvItem.Slot1);
    uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | swapInvItem.Slot2);

    _player->SwapItem(src, dst);
}

void WorldSession::HandleAutoEquipItemSlotOpcode(WorldPackets::Item::AutoEquipItemSlot& autoEquipItemSlot)
{
    if (autoEquipItemSlot.Inv.Items.size() != 1 || !Player::IsEquipmentPos(INVENTORY_SLOT_BAG_0, autoEquipItemSlot.ItemDstSlot))
        return;

    Item* item = _player->GetItemByGuid(autoEquipItemSlot.Item);
    uint16 dstPos = autoEquipItemSlot.ItemDstSlot | (INVENTORY_SLOT_BAG_0 << 8);
    uint16 srcPos = autoEquipItemSlot.Inv.Items[0].Slot | (uint32(autoEquipItemSlot.Inv.Items[0].ContainerSlot) << 8);
    
    if (!item || item->GetPos() != srcPos || srcPos == dstPos)
        return;

    _player->SwapItem(srcPos, dstPos);
}

void WorldSession::HandleSwapItem(WorldPackets::Item::SwapItem& swapItem)
{
    if (swapItem.Inv.Items.size() != 2)
        return;

    uint16 src = ((swapItem.ContainerSlotA << 8) | swapItem.SlotA);
    uint16 dst = ((swapItem.ContainerSlotB << 8) | swapItem.SlotB);
    if (src == dst)
        return;

    if (!_player->IsValidPos(swapItem.ContainerSlotA, swapItem.SlotA, true))
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!_player->IsValidPos(swapItem.ContainerSlotB, swapItem.SlotB, true))
    {
        _player->SendEquipError(EQUIP_ERR_WRONG_SLOT);
        return;
    }

    if (_player->IsBankPos(swapItem.ContainerSlotA, swapItem.SlotA) && !CanUseBank())
        return;

    if (_player->IsBankPos(swapItem.ContainerSlotB, swapItem.SlotB) && !CanUseBank())
        return;

    _player->SwapItem(src, dst);
}

void WorldSession::HandleAutoEquipItem(WorldPackets::Item::AutoEquipItem& autoEquipItem)
{
    if (autoEquipItem.Inv.Items.size() != 1)
        return;

    Item* pSrcItem  = _player->GetItemByPos(autoEquipItem.PackSlot, autoEquipItem.Slot);
    if (!pSrcItem)
        return;

    uint16 dest;
    InventoryResult msg = _player->CanEquipItem(NULL_SLOT, dest, pSrcItem, !pSrcItem->IsBag());
    if (msg != EQUIP_ERR_OK)
    {
        _player->SendEquipError(msg, pSrcItem);
        return;
    }

    uint16 src = pSrcItem->GetPos();
    if (dest == src)                                           // prevent equip in same slot, only at cheat
        return;

    Item* pDstItem = _player->GetItemByPos(dest);
    if (!pDstItem)                                         // empty slot, simple case
    {
        if (!pSrcItem->GetChildItem().IsEmpty())
        {
            InventoryResult childEquipResult = _player->CanEquipChildItem(pSrcItem);
            if (childEquipResult != EQUIP_ERR_OK)
            {
                _player->SendEquipError(msg, pSrcItem);
                return;
            }
        }

        _player->RemoveItem(autoEquipItem.PackSlot, autoEquipItem.Slot, true);
        _player->EquipItem(dest, pSrcItem, true);
        if (!pSrcItem->GetChildItem().IsEmpty())
            _player->EquipChildItem(autoEquipItem.PackSlot, autoEquipItem.Slot, pSrcItem);

        _player->AutoUnequipOffhandIfNeed();
    }
    else                                                    // have currently equipped item, not simple case
    {
        uint8 dstbag = pDstItem->GetBagSlot();
        uint8 dstslot = pDstItem->GetSlot();

        msg = _player->CanUnequipItem(dest, !pSrcItem->IsBag());
        if (msg != EQUIP_ERR_OK)
        {
            _player->SendEquipError(msg, pDstItem);
            return;
        }

        if (!pDstItem->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_CHILD))
        {
            // check dest->src move possibility
            ItemPosCountVec sSrc;
            uint16 eSrc = 0;
            if (_player->IsInventoryPos(src))
            {
                msg = _player->CanStoreItem(autoEquipItem.PackSlot, autoEquipItem.Slot, sSrc, pDstItem, true);
                if (msg != EQUIP_ERR_OK)
                    msg = _player->CanStoreItem(autoEquipItem.PackSlot, NULL_SLOT, sSrc, pDstItem, true);
                if (msg != EQUIP_ERR_OK)
                    msg = _player->CanStoreItem(NULL_BAG, NULL_SLOT, sSrc, pDstItem, true);
            }
            else if (_player->IsBankPos(src))
            {
                msg = _player->CanBankItem(autoEquipItem.PackSlot, autoEquipItem.Slot, sSrc, pDstItem, true);
                if (msg != EQUIP_ERR_OK)
                    msg = _player->CanBankItem(autoEquipItem.PackSlot, NULL_SLOT, sSrc, pDstItem, true);
                if (msg != EQUIP_ERR_OK)
                    msg = _player->CanBankItem(NULL_BAG, NULL_SLOT, sSrc, pDstItem, true);
            }
            else if (_player->IsEquipmentPos(src))
            {
                msg = _player->CanEquipItem(autoEquipItem.Slot, eSrc, pDstItem, true);
                if (msg == EQUIP_ERR_OK)
                    msg = _player->CanUnequipItem(eSrc, true);
            }

            if (msg == EQUIP_ERR_OK && Player::IsEquipmentPos(dest) && !pSrcItem->GetChildItem().IsEmpty())
                msg = _player->CanEquipChildItem(pSrcItem);

            if (msg != EQUIP_ERR_OK)
            {
                _player->SendEquipError(msg, pDstItem, pSrcItem);
                return;
            }

            _player->RemoveItem(dstbag, dstslot, false);
            _player->RemoveItem(autoEquipItem.PackSlot, autoEquipItem.Slot, false);

            // add to dest
            _player->EquipItem(dest, pSrcItem, true);

            if (_player->IsInventoryPos(src))
                _player->StoreItem(sSrc, pDstItem, true);
            else if (_player->IsBankPos(src))
                _player->BankItem(sSrc, pDstItem, true);
            else if (_player->IsEquipmentPos(src))
                _player->EquipItem(eSrc, pDstItem, true);

            if (Player::IsEquipmentPos(dest) && !pSrcItem->GetChildItem().IsEmpty())
                _player->EquipChildItem(autoEquipItem.PackSlot, autoEquipItem.Slot, pSrcItem);
        }
        else
        {
            if (Item* parentItem = _player->GetItemByGuid(pDstItem->GetGuidValue(ITEM_FIELD_CREATOR)))
            {
                if (Player::IsEquipmentPos(dest))
                {
                    _player->AutoUnequipChildItem(parentItem);
                    // dest is now empty
                    _player->SwapItem(src, dest);
                    // src is now empty
                    _player->SwapItem(parentItem->GetPos(), src);
                }
            }
        }

        _player->AutoUnequipOffhandIfNeed();
    }
}

void WorldSession::HandleDestroyItemOpcode(WorldPackets::Item::DestroyItem& destroyItem)
{
    uint16 pos = (destroyItem.ContainerId << 8) | destroyItem.SlotNum;

    if (_player->IsEquipmentPos(pos) || _player->IsBagPos(pos))
    {
        InventoryResult msg = _player->CanUnequipItem(pos, false);
        if (msg != EQUIP_ERR_OK)
        {
            _player->SendEquipError(msg, _player->GetItemByPos(pos));
            return;
        }
    }

    Item* item  = _player->GetItemByPos(destroyItem.ContainerId, destroyItem.SlotNum);
    if (!item || !item->IsInWorld())
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (item->GetTemplate()->GetFlags() & ITEM_FLAG_NO_USER_DESTROY)
    {
        _player->SendEquipError(EQUIP_ERR_DROP_BOUND_ITEM);
        return;
    }
    
    if (TradeData* tradeData = _player->GetTradeData())
    {
        if (tradeData->GetTradeSlotForItem(item->GetGUID()) != TRADE_SLOT_INVALID)
        {
            _player->SendEquipError(EQUIP_ERR_OBJECT_IS_BUSY);
            return;
        }
    }

    if (destroyItem.Count)
    {
        uint32 i_count = destroyItem.Count;
        _player->DestroyItemCount(item, i_count, true);
    }
    else
        _player->DestroyItem(destroyItem.ContainerId, destroyItem.SlotNum, true);
}

void WorldSession::HandleReadItem(WorldPackets::Item::ReadItem& packet)
{
    Item* item = _player->GetItemByPos(packet.PackSlot, packet.Slot);
    if (item && item->GetTemplate()->GetPageID())
    {
        InventoryResult msg = _player->CanUseItem(item);
        if (msg == EQUIP_ERR_OK)
            SendPacket(WorldPackets::Item::ReadItemResultOk(item->GetGUID()).Write());
        else
        {
            WorldPackets::Item::ReadItemResultFailed failed;
            failed.Item = item->GetGUID();
            failed.Delay = 2;
            failed.Subcode = WorldPackets::Item::ITEM_FAILURE_UNK_1;
            SendPacket(failed.Write());
            _player->SendEquipError(msg, item);
        }
    }
    else
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
}

void WorldSession::HandleSellItemOpcode(WorldPackets::Item::SellItem& packet)
{
    Player* player = GetPlayer();
    Creature* creature = player->GetNPCIfCanInteractWith(packet.VendorGUID, UNIT_NPC_FLAG_VENDOR);
    if (!creature)
    {
        player->SendSellError(SELL_ERR_VENDOR_HATES_YOU, creature, packet.ItemGUID);
        return;
    }

    if (packet.ItemGUID.IsEmpty())
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (Item* item = player->GetItemByGuid(packet.ItemGUID))
    {
        if (player->GetGUID() != item->GetOwnerGUID())
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }

        if (item->IsNotEmptyBag())
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }

        if (player->GetLootGUID() == item->GetGUID())
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }

        if (item->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_REFUNDABLE))
        {
            player->RefundItem(item);
            return;
        }

        if (item->GetQuality() == ITEM_QUALITY_ARTIFACT)
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }

        if (item->GetTemplate()->IsLegionLegendary())
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }
        
        if (item->GetDonateItem())
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }

        if (packet.Amount == 0)
            packet.Amount = item->GetCount();
        else if (packet.Amount > item->GetCount())
        {
            player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
            return;
        }

        if (ItemTemplate const* itemTemplate = item->GetTemplate())
        {
            uint32 SellPrice = item->GetSellPrice();

            if (SellPrice > 0)
            {
                if (packet.Amount < item->GetCount())
                {
                    Item* newItem = item->CloneItem(packet.Amount, player);
                    if (!newItem)
                    {
                        player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);
                        return;
                    }

                    item->SetCount(item->GetCount() - packet.Amount);
                    player->ItemRemovedQuestCheck(item->GetEntry(), packet.Amount);
                    if (player->IsInWorld())
                        item->SendUpdateToPlayer(player);
                    item->SetState(ITEM_CHANGED, player);

                    player->AddItemToBuyBackSlot(newItem);
                    if (player->IsInWorld())
                        newItem->SendUpdateToPlayer(player);
                }
                else
                {
                    player->ItemRemovedQuestCheck(item->GetEntry(), item->GetCount());
                    item->RemoveFromWorld();
                    player->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
                    item->RemoveFromUpdateQueueOf(player);
                    player->AddItemToBuyBackSlot(item);
                }

                uint32 money = SellPrice * packet.Amount;
                player->ModifyMoney(money);
                player->UpdateAchievementCriteria(CRITERIA_TYPE_MONEY_FROM_VENDORS, money);
                player->SendSellError(SELL_ERR_OK, creature, packet.ItemGUID);
            }
            else
                player->SendSellError(SELL_ERR_CANT_SELL_ITEM, creature, packet.ItemGUID);

            return;
        }
    }

    player->SendSellError(SELL_ERR_CANT_FIND_ITEM, creature, packet.ItemGUID);
    return;
}

void WorldSession::HandleBuybackItem(WorldPackets::Item::BuyBackItem& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* creature = player->GetNPCIfCanInteractWith(packet.VendorGUID, UNIT_NPC_FLAG_VENDOR);
    if (!creature)
    {
        player->SendSellError(SELL_ERR_VENDOR_HATES_YOU);
        return;
    }

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Item* item = player->GetItemFromBuyBackSlot(packet.Slot);
    if (item)
    {
        uint32 price = player->GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE + packet.Slot - BUYBACK_SLOT_START);
        if (!player->HasEnoughMoney(uint64(price)))
        {
            player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, creature, item->GetEntry());
            return;
        }
    
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg == EQUIP_ERR_OK)
        {
            player->ModifyMoney(-(int32)price);
            player->RemoveItemFromBuyBackSlot(packet.Slot, false);
            player->ItemAddedQuestCheck(item->GetEntry(), item->GetCount());
            player->UpdateAchievementCriteria(CRITERIA_TYPE_RECEIVE_EPIC_ITEM, item->GetEntry(), item->GetCount());
            player->StoreItem(dest, item, true);
        }
        else
            player->SendEquipError(msg, item);

        return;
    }
    player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, creature);
}

void WorldSession::HandleBuyItemOpcode(WorldPackets::Item::BuyItem& packet)
{
    if (packet.Muid > 0)
        --packet.Muid;
    else
        return;
    
    switch (packet.ItemType)
    {
        case ITEM_VENDOR_TYPE_ITEM:
        {
            Item* bagItem = _player->GetItemByGuid(packet.ContainerGUID);
            uint8 bag = NULL_BAG;
            if (bagItem && bagItem->IsBag())
            {
                // check equipped bags
                uint8 bagSlot = bagItem->GetSlot();

                if (bagSlot >= INVENTORY_SLOT_BAG_START && bagSlot < INVENTORY_SLOT_BAG_END)
                    bag = bagSlot;
            }
            else if (packet.ContainerGUID == GetPlayer()->GetGUID())
                bag = INVENTORY_SLOT_BAG_0;

            GetPlayer()->BuyItemFromVendorSlot(packet.VendorGUID, packet.Muid, packet.Item.ItemID, packet.Quantity, bag, packet.Slot);
            break;
        }
        case ITEM_VENDOR_TYPE_CURRENCY:
            GetPlayer()->BuyCurrencyFromVendorSlot(packet.VendorGUID, packet.Muid, packet.Item.ItemID, packet.Quantity);
            break;
        default:
        {
            TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: received wrong itemType (%u) in HandleBuyItemOpcode", packet.ItemType);
            break;
        }
    }
}

void WorldSession::HandleAutoStoreBagItem(WorldPackets::Item::AutoStoreBagItem& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!packet.Inv.Items.empty())
        return;

    Item* item = player->GetItemByPos(packet.ContainerSlotA, packet.SlotA);
    if (!item)
        return;

    if (!player->IsValidPos(packet.ContainerSlotB, NULL_SLOT, false))
    {
        player->SendEquipError(EQUIP_ERR_WRONG_SLOT);
        return;
    }

    uint16 src = item->GetPos();
    if (player->IsEquipmentPos (src) || player->IsBagPos (src))
    {
        InventoryResult msg = player->CanUnequipItem(src, !player->IsBagPos (src));
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }
    }

    ItemPosCountVec dest;
    InventoryResult msg = player->CanStoreItem(packet.ContainerSlotB, NULL_SLOT, dest, item, false);
    if (msg != EQUIP_ERR_OK)
    {
        player->SendEquipError(msg, item);
        return;
    }

    if (dest.size() == 1 && dest[0].pos == src)
    {
        player->SendEquipError(EQUIP_ERR_INTERNAL_BAG_ERROR, item);
        return;
    }

    player->RemoveItem(packet.ContainerSlotA, packet.SlotA, true);
    player->StoreItem(dest, item, true);
}

void WorldSession::HandleBuyBankSlot(WorldPackets::Bank::BuyBankSlot& /*packet*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    uint32 slot = player->GetBankBagSlotsValue();

    ++slot;

    BankBagSlotPricesEntry const* slotEntry = sBankBagSlotPricesStore.LookupEntry(slot);
    if (!slotEntry)
        return;

    uint32 price = slotEntry->Cost;
    if (!player->HasEnoughMoney(uint64(price)))
        return;

    player->SetBankBagSlotCount(slot);
    player->ModifyMoney(-int64(price));
    player->UpdateAchievementCriteria(CRITERIA_TYPE_BUY_BANK_SLOT);
}

void WorldSession::HandleAutoBankItem(WorldPackets::Bank::AutoBankItem& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Item* item = player->GetItemByPos(packet.Bag, packet.Slot);
    if (!item)
        return;

    ItemPosCountVec dest;
    if (player->IsBankPos(packet.Bag, packet.Slot))
    {
        InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }

        player->RemoveItem(packet.Bag, packet.Slot, true);
        player->StoreItem(dest, item, true);
        player->ItemAddedQuestCheck(item->GetEntry(), item->GetCount());
    }
    else
    {
        InventoryResult msg = player->CanBankItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }

        if (dest.size() == 1 && dest[0].pos == item->GetPos())
        {
            player->SendEquipError(EQUIP_ERR_CANT_SWAP, item);
            return;
        }

        player->RemoveItem(packet.Bag, packet.Slot, true);
        player->ItemRemovedQuestCheck(item->GetEntry(), item->GetCount());
        player->BankItem(dest, item, true);
    }
}

void WorldSession::HandleAutoStoreBankItem(WorldPackets::Bank::AutoStoreBankItem& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Item* item = player->GetItemByPos(packet.Bag, packet.Slot);
    if (!item)
        return;

    ItemPosCountVec dest;

    if (player->IsBankPos(packet.Bag, packet.Slot))
    {
        InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }

        player->RemoveItem(packet.Bag, packet.Slot, true);
        player->StoreItem(dest, item, true);
        player->ItemAddedQuestCheck(item->GetEntry(), item->GetCount());
    }
    else
    {
        InventoryResult msg = player->CanBankItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }

        player->RemoveItem(packet.Bag, packet.Slot, true);
        player->BankItem(dest, item, true);
    }
}

void WorldSession::HandleWrapItem(WorldPackets::Item::WrapItem& packet)
{
    if (packet.Inv.Items.size() != 2)
        return;

    Item* gift = _player->GetItemByPos(packet.Inv.Items[0].ContainerSlot, packet.Inv.Items[0].Slot);
    if (!gift)
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, gift);
        return;
    }

    if (!(gift->GetTemplate()->GetFlags() & ITEM_FLAG_IS_WRAPPER)) // cheating: non-wrapper wrapper
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, gift);
        return;
    }

    Item* item = _player->GetItemByPos(packet.Inv.Items[1].ContainerSlot, packet.Inv.Items[1].Slot);
    if (!item)
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, item);
        return;
    }

    if (item == gift)
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_WRAPPED, item);
        return;
    }

    if (item->IsEquipped())
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_EQUIPPED, item);
        return;
    }

    if (!item->GetGuidValue(ITEM_FIELD_GIFT_CREATOR).IsEmpty())
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_WRAPPED, item);
        return;
    }

    if (item->IsBag())
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_BAGS, item);
        return;
    }

    if (item->IsSoulBound())
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_BOUND, item);
        return;
    }

    if (item->GetMaxStackCount() != 1)
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_STACKABLE, item);
        return;
    }

    if (item->GetTemplate()->GetMaxCount() > 0)
    {
        _player->SendEquipError(EQUIP_ERR_CANT_WRAP_UNIQUE, item);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_GIFT);
    stmt->setUInt64(0, item->GetOwnerGUID().GetCounter());
    stmt->setUInt64(1, item->GetGUIDLow());
    stmt->setUInt32(2, item->GetEntry());
    stmt->setUInt32(3, item->GetUInt32Value(ITEM_FIELD_DYNAMIC_FLAGS));
    trans->Append(stmt);

    item->SetEntry(gift->GetEntry());

    switch (item->GetEntry())
    {
        case 5042:  item->SetEntry(5043); break;
        case 5048:  item->SetEntry(5044); break;
        case 17303: item->SetEntry(17302); break;
        case 17304: item->SetEntry(17305); break;
        case 17307: item->SetEntry(17308); break;
        case 21830: item->SetEntry(21831); break;
        default:
            break;
    }
    item->SetGuidValue(ITEM_FIELD_GIFT_CREATOR, _player->GetGUID());
    item->SetUInt32Value(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_WRAPPED);
    item->SetState(ITEM_CHANGED, _player);

    if (item->GetState() == ITEM_NEW)                          // save new item, to have alway for `character_gifts` record in `item_instance`
    {
        // after save it will be impossible to remove the item from the queue
        item->RemoveFromUpdateQueueOf(_player);
        item->SaveToDB(trans);                                   // item gave inventory record unchanged and can be save standalone
    }
    CharacterDatabase.CommitTransaction(trans);

    _player->DestroyItem(gift->GetBagSlot(), gift->GetSlot(), true);
}

void WorldSession::HandleSocketGems(WorldPackets::Item::SocketGems& packet)
{
    Player* player = GetPlayer();

    if (!packet.ItemGuid)
        return;

    if ((!packet.GemItem[0].IsEmpty() && (packet.GemItem[0] == packet.GemItem[1] || packet.GemItem[0] == packet.GemItem[2])) ||
        (!packet.GemItem[1].IsEmpty() && (packet.GemItem[1] == packet.GemItem[2])))
        return;

    Item* itemTarget = player->GetItemByGuid(packet.ItemGuid);
    if (!itemTarget || !itemTarget->IsInWorld() || !player->IsInWorld())
        return;

    ItemTemplate const* itemProto = itemTarget->GetTemplate();
    if (!itemProto)
        return;

    uint8 slot = itemTarget->IsEquipped() ? itemTarget->GetSlot() : uint8(NULL_SLOT);

    Item* gems[MAX_GEM_SOCKETS];
    memset(gems, 0, sizeof(gems));

    ItemDynamicFieldGems gemData[MAX_GEM_SOCKETS];
    memset(gemData, 0, sizeof(gemData));

    GemPropertiesEntry const* gemProperties[MAX_GEM_SOCKETS];
    memset(gemProperties, 0, sizeof(gemProperties));

    ItemDynamicFieldGems const* oldGemData[MAX_GEM_SOCKETS];
    memset(oldGemData, 0, sizeof(oldGemData));

    for (uint32 i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        if (Item* gem = player->GetItemByGuid(packet.GemItem[i]))
        {
            gems[i] = gem;
            gemData[i].ItemId = gem->GetEntry();
            gemData[i].Context = gem->GetUInt32Value(ITEM_FIELD_CONTEXT);
            for (std::size_t b = 0; b < gem->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS).size() && b < 16; ++b)
                gemData[i].BonusListIDs[b] = gem->GetDynamicValue(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS, b);

            gemProperties[i] = sGemPropertiesStore.LookupEntry(gem->GetTemplate()->GetGemProperties());
        }

        oldGemData[i] = itemTarget->GetGem(i);
    }

    uint32 firstPrismatic = 0;
    while (firstPrismatic < MAX_GEM_SOCKETS && itemTarget->GetSocketColor(firstPrismatic))
        ++firstPrismatic;

    for (uint32 i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        if (!gemProperties[i])
            continue;

        if (!itemTarget->GetSocketColor(i))
        {
            if (!itemTarget->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT))
                return;

            if (i != firstPrismatic)
                return;
        }

        if (SocketColorToGemTypeMask[itemTarget->GetSocketColor(i)] != gemProperties[i]->Type)
            if (!(SocketColorToGemTypeMask[itemTarget->GetSocketColor(i)] & SOCKET_COLOR_PRISMATIC) || !(gemProperties[i]->Type & SOCKET_COLOR_PRISMATIC))
                return;
    }

    for (uint32 i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        if (!gems[i])
            continue;

        ItemTemplate const* iGemProto = gems[i]->GetTemplate();
        if (iGemProto->GetFlags() & ITEM_FLAG_UNIQUE_EQUIPPABLE)
        {
            for (uint32 j = 0; j < MAX_GEM_SOCKETS; ++j)
            {
                if (i == j)                                    // skip self
                    continue;

                if (gems[j])
                {
                    if (iGemProto->GetId() == gems[j]->GetEntry())
                    {
                        player->SendEquipError(EQUIP_ERR_ITEM_UNIQUE_EQUIPPABLE_SOCKETED, itemTarget);
                        return;
                    }
                }
                else if (oldGemData[j])
                {
                    if (iGemProto->GetId() == oldGemData[j]->ItemId)
                    {
                        player->SendEquipError(EQUIP_ERR_ITEM_UNIQUE_EQUIPPABLE_SOCKETED, itemTarget);
                        return;
                    }
                }
            }
        }

        int32 limit_newcount = 0;
        if (iGemProto->GetLimitCategory())
        {
            if (auto limitEntry = sItemLimitCategoryStore.LookupEntry(iGemProto->GetLimitCategory()))
            {
                for (auto j = 0; j < MAX_GEM_SOCKETS; ++j)
                {
                    if (gems[j])
                    {
                        if (iGemProto->GetLimitCategory() == gems[j]->GetTemplate()->GetLimitCategory())
                            ++limit_newcount;
                    }
                    else if (oldGemData[j])
                    {
                        if (auto jProto = sObjectMgr->GetItemTemplate(oldGemData[j]->ItemId))
                            if (iGemProto->GetLimitCategory() == jProto->GetLimitCategory())
                                ++limit_newcount;
                    }
                }

                if (limit_newcount > 0 && uint32(limit_newcount) > _player->GetItemLimitCategoryQuantity(limitEntry))
                {
                    player->SendEquipError(EQUIP_ERR_ITEM_UNIQUE_EQUIPPABLE_SOCKETED, itemTarget);
                    return;
                }
            }
        }

        if (itemTarget->IsEquipped())
            if (InventoryResult res = player->CanEquipUniqueItem(gems[i], slot, std::max(limit_newcount, 0)))
            {
                player->SendEquipError(res, itemTarget);
                return;
            }
    }

    bool SocketBonusActivated = itemTarget->GemsFitSockets();
    player->ToggleMetaGemsActive(slot, false);

    if (itemTarget->IsEquipped())
        _player->_ApplyItemMods(itemTarget, itemTarget->GetSlot(), false);

    if (Item* childItem = _player->GetChildItemByGuid(itemTarget->GetChildItem()))
    {
        if (childItem->IsEquipped())
            _player->_ApplyItemMods(childItem, childItem->GetSlot(), false);
        childItem->CopyArtifactDataFromParent(itemTarget);
        if (childItem->IsEquipped())
            _player->_ApplyItemMods(childItem, childItem->GetSlot(), true);
    }

    for (uint16 i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        if (gems[i])
        {
            uint32 gemScalingLevel = _player->getLevel();
            if (uint32 fixedLevel = gems[i]->GetModifier(ITEM_MODIFIER_SCALING_STAT_DISTRIBUTION_FIXED_LEVEL))
                gemScalingLevel = fixedLevel;

            itemTarget->SetGem(i, &gemData[i], gemScalingLevel);

            if (gemProperties[i] && gemProperties[i]->EnchantID)
                itemTarget->SetEnchantment(EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + i), gemProperties[i]->EnchantID, 0, 0, player->GetGUID());

            if (itemTarget->GetTemplate()->GetArtifactID())
            {
                WorldPackets::Artifact::ArtifactAttuneSocketedRelicData data; // clear old data
                data.ArtifactGUID = itemTarget->GetGUID();
                data.Result = 16;
                SendPacket(data.Write());

                auto current_relics = itemTarget->GetArtifactSockets();
                if (current_relics.find(i + 2) != current_relics.end())
                {
                    for (uint8 j = 0; j <= 5; ++j)
                        if ((1 << j) & current_relics[i + 2].firstTier)
                            itemTarget->AddOrRemoveSocketTalent(j, false, i + 2); // delete old data
                }

                auto relicks = gems[i]->GetArtifactSockets();
                if (!relicks.empty())
                {
                    uint8 offset = i * 6;
                    itemTarget->SetDynamicValue(ITEM_DYNAMIC_FIELD_RELIC_TALENT_DATA, offset++, relicks[2].unk1);
                    itemTarget->SetDynamicValue(ITEM_DYNAMIC_FIELD_RELIC_TALENT_DATA, offset++, i + 2);
                    itemTarget->SetDynamicValue(ITEM_DYNAMIC_FIELD_RELIC_TALENT_DATA, offset++, relicks[2].firstTier);
                    itemTarget->SetDynamicValue(ITEM_DYNAMIC_FIELD_RELIC_TALENT_DATA, offset++, relicks[2].secondTier);
                    itemTarget->SetDynamicValue(ITEM_DYNAMIC_FIELD_RELIC_TALENT_DATA, offset++, relicks[2].thirdTier);
                    itemTarget->SetDynamicValue(ITEM_DYNAMIC_FIELD_RELIC_TALENT_DATA, offset++, relicks[2].additionalThirdTier);

                    for (uint8 j = 0; j <= 5; ++j)
                        if ((1 << j) & relicks[i + 2].firstTier)
                            itemTarget->AddOrRemoveSocketTalent(j, true, i + 2); // add new data
                }
                else
                    itemTarget->CreateSocketTalents(i + 2);
            }

            uint32 gemCount = 1;
            player->DestroyItemCount(gems[i], gemCount, true);
        }
    }

    if (itemTarget->IsEquipped())
        _player->_ApplyItemMods(itemTarget, itemTarget->GetSlot(), true);

    if (Item* childItem = _player->GetChildItemByGuid(itemTarget->GetChildItem()))
    {
        if (childItem->IsEquipped())
            _player->_ApplyItemMods(childItem, childItem->GetSlot(), false);
        childItem->CopyArtifactDataFromParent(itemTarget);
        if (childItem->IsEquipped())
            _player->_ApplyItemMods(childItem, childItem->GetSlot(), true);
    }

    bool SocketBonusToBeActivated = itemTarget->GemsFitSockets();
    if (SocketBonusActivated ^ SocketBonusToBeActivated)
    {
        player->ApplyEnchantment(itemTarget, BONUS_ENCHANTMENT_SLOT, false);
        itemTarget->SetEnchantment(BONUS_ENCHANTMENT_SLOT, (SocketBonusToBeActivated ? itemTarget->GetTemplate()->GetSocketBonus() : 0), 0, 0, player->GetGUID());
        player->ApplyEnchantment(itemTarget, BONUS_ENCHANTMENT_SLOT, true);
    }

    player->ToggleMetaGemsActive(slot, true);

    player->RemoveTradeableItem(itemTarget);
    itemTarget->ClearSoulboundTradeable(player);
    itemTarget->SetState(ITEM_CHANGED, player);

    SendPacket(WorldPackets::Item::SocketGemsResult(itemTarget->GetGUID()).Write());
}

void WorldSession::HandleCancelTempEnchantmentOpcode(WorldPackets::Item::CancelTempEnchantment& packet)
{
    if (!Player::IsEquipmentPos(INVENTORY_SLOT_BAG_0, packet.Slot))
        return;

    Item* item = GetPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, packet.Slot);

    if (!item)
        return;

    if (!item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        return;

    GetPlayer()->ApplyEnchantment(item, TEMP_ENCHANTMENT_SLOT, false);
    item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
}

void WorldSession::HandleGetItemPurchaseData(WorldPackets::Item::ItemRefundInfo& packet)
{
    Item* item = _player->GetItemByGuid(packet.ItemGUID);
    if (!item)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Item refund: item not found!");
        return;
    }

    GetPlayer()->SendRefundInfo(item);
}

void WorldSession::HandleItemPurchaseRefund(WorldPackets::Item::ItemPurchaseRefund& packet)
{
    Item* item = _player->GetItemByGuid(packet.ItemGUID);
    if (!item)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Item refund: item not found!");
        return;
    }

    GetPlayer()->RefundItem(item);
}

void WorldSession::HandleOpenItem(WorldPackets::Spells::OpenItem& packet)
{
    Player* player = GetPlayer();

    if (player->m_mover != player)
        return;

    Item* item = player->GetItemByPos(packet.Slot, packet.PackSlot);
    if (!item/* || item->GetGUID() != itemGUID*/)
    {
        player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    ItemTemplate const* proto = item->GetTemplate();
    if (!proto)
    {
        player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, item);
        return;
    }

    if (!(proto->GetFlags() & ITEM_FLAG_HAS_LOOT) && !item->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_WRAPPED))
    {
        player->SendEquipError(EQUIP_ERR_CLIENT_LOCKED_OUT, item);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Possible hacking attempt: Player %s [guid: %u] tried to open item [guid: %u, entry: %u] which is not openable!",
                player->GetName(), player->GetGUIDLow(), item->GetGUIDLow(), proto->GetId());
        return;
    }

    uint32 lockId = proto->GetLockID();
    if (lockId)
    {
        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);

        if (!lockInfo)
        {
            player->SendEquipError(EQUIP_ERR_ITEM_LOCKED, item);
            return;
        }

        if (item->IsLocked())
        {
            player->SendEquipError(EQUIP_ERR_ITEM_LOCKED, item);
            return;
        }
    }

    if (item->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_WRAPPED))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_GIFT_BY_ITEM);

        stmt->setUInt64(0, item->GetGUIDLow());

        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 flags = fields[1].GetUInt32();

            item->SetGuidValue(ITEM_FIELD_GIFT_CREATOR, ObjectGuid::Empty);
            item->SetEntry(entry);
            item->SetUInt32Value(ITEM_FIELD_DYNAMIC_FLAGS, flags);
            item->SetState(ITEM_CHANGED, player);
        }
        else
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Wrapped item %u don't have record in character_gifts table and will deleted", item->GetGUIDLow());
            player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
            return;
        }

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GIFT);

        stmt->setUInt64(0, item->GetGUIDLow());

        CharacterDatabase.Execute(stmt);
    }
    else
        player->SendLoot(item->GetGUID(), LOOT_CORPSE);
}

void WorldSession::HandleUpgradeItem(WorldPackets::Item::UpgradeItem& packet)
{
    Player* player = GetPlayer();
    if (!player->GetNPCIfCanInteractWith(packet.ItemMaster, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG2_UPGRADE_MASTER))
        return;

    Item* item = player->GetItemByGuid(packet.ItemGUID);
    if (!item)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleUpgradeItem - Can't find item (GUID: %u).", packet.ItemGUID.GetGUIDLow());
        return;
    }

    auto const& itemTemplate = sObjectMgr->GetItemTemplate(item->GetEntry());
    if (!itemTemplate)
        return;

    if ((itemTemplate->GetFlags3() & ITEM_FLAG3_ITEM_CAN_BE_UPGRADED) == 0)
        return;

    ItemUpgradeEntry const* itemUpgradeEntry = sItemUpgradeStore.LookupEntry(packet.UpgradeID);
    if (!itemUpgradeEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleUpgradeItems - ItemUpgradeEntry (%u) not found.", packet.UpgradeID);
        return;
    }

    if (!_player->HasCurrency(itemUpgradeEntry->CurrencyType, itemUpgradeEntry->CurrencyAmount))
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleUpgradeItems - Player has not enougth currency (ID: %u, Cost: %u) not found.", itemUpgradeEntry->CurrencyType, itemUpgradeEntry->CurrencyAmount);
        return;
    }

    uint32 currentUpgradeId = item->GetModifier(ITEM_MODIFIER_UPGRADE_ID);
    if (currentUpgradeId != itemUpgradeEntry->PrerequisiteID)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleUpgradeItems - ItemUpgradeEntry (%u) is not related to this ItemUpgradePath (%u).", itemUpgradeEntry->ID, currentUpgradeId);
        return;
    }

    if (item->IsEquipped())
        player->_ApplyItemBonuses(item, item->GetSlot(), false);

    item->SetModifier(ITEM_MODIFIER_UPGRADE_ID, itemUpgradeEntry->ID);

    if (item->IsEquipped())
        player->_ApplyItemBonuses(item, item->GetSlot(), true);

    item->SetState(ITEM_CHANGED, player);
    player->ModifyCurrency(itemUpgradeEntry->CurrencyType, -int32(itemUpgradeEntry->CurrencyAmount));
}

bool WorldSession::CanUseBank(ObjectGuid bankerGUID) const
{
    // bankerGUID parameter is optional, set to 0 by default.
    if (bankerGUID.IsEmpty())
        bankerGUID = m_currentBankerGUID;

    bool isUsingBankCommand = (bankerGUID == GetPlayer()->GetGUID() && bankerGUID == m_currentBankerGUID);

    if (!isUsingBankCommand)
    {
        Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(bankerGUID, UNIT_NPC_FLAG_BANKER);
        if (!creature)
            return false;
    }

    return true;
}

void WorldSession::HandleRepairItem(WorldPackets::Item::RepairItem& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_REPAIR);
    if (!unit)
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    float discountMod = player->GetReputationPriceDiscount(unit);

    if (packet.ItemGUID)
    {
        if (Item* item = player->GetItemByGuid(packet.ItemGUID))
            player->DurabilityRepair(item->GetPos(), true, discountMod, packet.UseGuildBank);
    }
    else
        player->DurabilityRepairAll(true, discountMod, packet.UseGuildBank);
}

bool StoreItemAndStack(Player* player, Item* item, uint8 bagSlot)
{
    ItemPosCountVec dest;
    if (player->CanStoreItem(bagSlot, NULL_SLOT, dest, item, false) == EQUIP_ERR_OK && !(dest.size() == 1 && dest[0].pos == item->GetPos()))
    {
        player->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
        player->StoreItem(dest, item, true);

        return true;
    }

    return false;
}

void StoreItemInBags(Player* player, Item* item)
{
    if (StoreItemAndStack(player, item, INVENTORY_SLOT_BAG_0))
        return;

    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < player->GetInventoryEndSlot(); i++)
        if (StoreItemAndStack(player, item, i))
            break;
}

bool BankItemAndStack(Player* player, Item* item, uint8 bagSlot)
{
    ItemPosCountVec dest;
    if (player->CanBankItem(bagSlot, NULL_SLOT, dest, item, false) != EQUIP_ERR_OK)
        return false;

    player->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
    player->BankItem(dest, item, true);

    return true;
}

void StoreItemInBanks(Player* player, Item* item)
{
    if (BankItemAndStack(player, item, NULL_SLOT))
        return;

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        if (BankItemAndStack(player, item, i))
            break;
}

void WorldSession::HandleSortBags(WorldPackets::Item::SortBags& /*packet*/)
{
    SendPacket(WorldPackets::Item::SortBagsResult().Write());
    return;
    _player->ApplyOnItems(1, [](Player* player, Item* item, uint8 /*bagSlot*/, uint8)
    {
        StoreItemInBags(player, item);
        return true;
    });

    std::unordered_map<uint32, uint32> itemsQuality;
    std::multimap<uint32, Item*> items;

    _player->ApplyOnItems(1, [&items, &itemsQuality](Player* player, Item* item, uint8 /*bagSlot*/, uint8)
    {
        if (!item)
            return false;

        if (!sObjectMgr->GetItemTemplate(item->GetEntry()))
            return true;

        items.insert(std::make_pair(item->GetEntry(), item));
        itemsQuality[item->GetEntry()] = item->GetItemLevel();

        return true;
    });

    std::multimap<uint32, std::pair<uint32, Item*>> resultMap;
    for (auto const& v : items)
        resultMap.insert(std::make_pair(itemsQuality[v.first], v));

    auto itr = std::begin(resultMap);
    _player->ApplyOnItems(1, [&resultMap, &itr](Player* player, Item* /*item*/, uint8 bagSlot, uint8 itemSlot)
    {
        if (itr == std::end(resultMap) || !itr->second.second)
            return false;

        player->SwapItem(itr->second.second->GetPos(), (bagSlot << 8) | itemSlot);
        ++itr;

        return true;
    });
}

void WorldSession::HandleSortBankBags(WorldPackets::Item::SortBankBags& /*packet*/)
{
    SendPacket(WorldPackets::Item::SortBagsResult().Write());
    return;
    _player->ApplyOnItems(2, [](Player* player, Item* item, uint8 /*bagSlot*/, uint8)
    {
        StoreItemInBanks(player, item);
        return true;
    });

    std::unordered_map<uint32, uint32> bankItemsQuality;
    std::multimap<uint32, Item*> bankItems;

    _player->ApplyOnItems(2, [&bankItems, &bankItemsQuality](Player* player, Item* item, uint8 /*bagSlot*/, uint8)
    {
        if (!item)
            return false;

        if (!sObjectMgr->GetItemTemplate(item->GetEntry()))
            return true;

        bankItems.insert(std::make_pair(item->GetEntry(), item));
        bankItemsQuality[item->GetEntry()] = item->GetItemLevel();

        return true;
    });

    std::multimap<uint32, std::pair<uint32, Item*>> bankResultMap;
    for (auto const& v : bankItems)
        bankResultMap.insert(std::make_pair(bankItemsQuality[v.first], v));

    auto itr = std::begin(bankResultMap);
    _player->ApplyOnItems(2, [&bankResultMap, &itr](Player* player, Item* /*item*/, uint8 bagSlot, uint8 itemSlot)
    {
        if (itr == std::end(bankResultMap))
            return false;

        player->SwapItem(itr->second.second->GetPos(), (bagSlot << 8) | itemSlot);
        ++itr;

        return true;
    });
}

void WorldSession::HandleSortReagentBankBags(WorldPackets::Item::SortReagentBankBags& /*packet*/)
{
    SendPacket(WorldPackets::Item::SortBagsResult().Write());
    return;

    _player->ApplyOnItems(3, [](Player* player, Item* item, uint8 /*bagSlot*/, uint8)
    {
        StoreItemInBanks(player, item);
        return true;
    });

    std::unordered_map<uint32, uint32> bankItemsQuality;
    std::multimap<uint32, Item*> bankItems;

    _player->ApplyOnItems(3, [&bankItems, &bankItemsQuality](Player* player, Item* item, uint8 /*bagSlot*/, uint8)
    {
        if (!sObjectMgr->GetItemTemplate(item->GetEntry()))
            return true;

        bankItems.insert(std::make_pair(item->GetEntry(), item));
        bankItemsQuality[item->GetEntry()] = item->GetItemLevel();

        return true;
    });

    std::multimap<uint32, std::pair<uint32, Item*>> bankResultMap;
    for (auto const& v : bankItems)
        bankResultMap.insert(std::make_pair(bankItemsQuality[v.first], v));

    auto itr = std::begin(bankResultMap);
    _player->ApplyOnItems(3, [&bankResultMap, &itr](Player* player, Item* /*item*/, uint8 bagSlot, uint8 itemSlot)
    {
        if (itr == std::end(bankResultMap))
            return false;

        player->SwapItem(itr->second.second->GetPos(), (bagSlot << 8) | itemSlot);
        ++itr;

        return true;
    });
}

void WorldSession::HandleUseCritterItem(WorldPackets::Item::UseCritterItem& useCritterItem)
{
    Item* item = _player->GetItemByGuid(useCritterItem.ItemGuid);
    if (!item)
        return;

    if (item->GetTemplate()->Effects.size() < 2)
        return;

    SpellCastTargets targets;
    targets.Update(_player);
    _player->CastItemUseSpell(item, targets, nullptr, ObjectGuid::Empty);

    _player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
}

void WorldSession::HandleAutoBankReagent(WorldPackets::Item::AutoBankReagent& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Item* item = player->GetItemByPos(packet.PackSlot, packet.Slot);
    if (!item)
        return;

    ItemPosCountVec dest;
    InventoryResult msg = player->CanBankReagentItem(NULL_BAG, NULL_SLOT, dest, item, false);
    if (msg != EQUIP_ERR_OK)
    {
        player->SendEquipError(msg, item);
        return;
    }

    if (dest.size() == 1 && dest[0].pos == item->GetPos())
    {
        player->SendEquipError(EQUIP_ERR_CANT_SWAP, item);
        return;
    }

    player->RemoveItem(packet.PackSlot, packet.Slot, true);
    player->ItemRemovedQuestCheck(item->GetEntry(), item->GetCount());
    player->StoreItem(dest, item, true);
}

void WorldSession::HandleAutostoreBankReagent(WorldPackets::Bank::AutostoreBankReagent& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Item* item = player->GetItemByPos(packet.Bag, packet.Slot);
    if (!item)
        return;

    ItemPosCountVec dest;
    if (player->IsReagentBankPos(packet.Bag, packet.Slot))
    {
        InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }

        player->RemoveItem(packet.Bag, packet.Slot, true);
        player->StoreItem(dest, item, true);
        player->ItemAddedQuestCheck(item->GetEntry(), item->GetCount());
    }
    else
    {
        InventoryResult msg = player->CanBankReagentItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            player->SendEquipError(msg, item);
            return;
        }

        player->RemoveItem(packet.Bag, packet.Slot, true);
        player->StoreItem(dest, item, true);
    }
}

void WorldSession::HandleBuyReagentBank(WorldPackets::Bank::BuyReagentBank& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!CanUseBank(packet.Banker))
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    uint64 price = 1000000; // 100 gold

    if (!player->HasEnoughMoney(price))
        return;

    player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS_EX, PLAYER_FLAGS_EX_REAGENT_BANK_UNLOCKED);
    player->ModifyMoney(-int64(price));
}

void WorldSession::HandleDepositReagentBank(WorldPackets::Bank::DepositReagentBank& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!CanUseBank(packet.Banker))
        return;

    if (!player->CanUseReagentBank())
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    player->DepositItemToReagentBank();
}

void WorldSession::HandleRemoveNewItem(WorldPackets::Item::RemoveNewItem& removeNewItem)
{
    auto item = _player->GetItemByGuid(removeNewItem.ItemGuid);
    if (!item)
        return;

    if (item->HasFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_NEW_ITEM))
    {
        item->RemoveFlag(ITEM_FIELD_DYNAMIC_FLAGS, ITEM_FLAG_NEW_ITEM);
        item->SetState(ITEM_CHANGED, _player);
    }
}
