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

#include "ItemPackets.h"

void WorldPackets::Item::BuyBackItem::Read()
{
    _worldPacket >> VendorGUID;
    _worldPacket >> Slot;
}

void WorldPackets::Item::ItemRefundInfo::Read()
{
    _worldPacket >> ItemGUID;
}

void WorldPackets::Item::RepairItem::Read()
{
    _worldPacket >> NpcGUID;
    _worldPacket >> ItemGUID;
    UseGuildBank = _worldPacket.ReadBit();
}

void WorldPackets::Item::SellItem::Read()
{
    _worldPacket >> VendorGUID;
    _worldPacket >> ItemGUID;
    _worldPacket >> Amount;
}

WorldPacket const* WorldPackets::Item::ItemTimeUpdate::Write()
{
    _worldPacket << ItemGuid;
    _worldPacket << DurationLeft;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::SetProficiency::Write()
{
    _worldPacket << ProficiencyMask;
    _worldPacket << ProficiencyClass;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemBonusInstanceData const& itemBonusInstanceData)
{
    data << itemBonusInstanceData.Context;
    data << static_cast<uint32>(itemBonusInstanceData.BonusListIDs.size());
    for (uint32 const& bonusID : itemBonusInstanceData.BonusListIDs)
        data << bonusID;

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Item::ItemBonusInstanceData& itemBonusInstanceData)
{

    data >> itemBonusInstanceData.Context;
    uint32 bonusListIdSize = data.read<uint32>();
    for (uint32 i = 0u; i < bonusListIdSize; ++i)
        itemBonusInstanceData.BonusListIDs.push_back(data.read<uint32>());

    return data;
}

bool WorldPackets::Item::ItemBonusInstanceData::operator==(ItemBonusInstanceData const& r) const
{
    if (Context != r.Context)
        return false;

    if (BonusListIDs.size() != r.BonusListIDs.size())
        return false;

    return std::is_permutation(BonusListIDs.begin(), BonusListIDs.end(), r.BonusListIDs.begin());
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemGemData const& itemGemInstanceData)
{
    data << uint8(itemGemInstanceData.Slot);
    data << itemGemInstanceData.Item;

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Item::ItemGemData& itemGemInstanceData)
{
    data >> itemGemInstanceData.Slot;
    data >> itemGemInstanceData.Item;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemEnchantData const& itemEnchantData)
{
    data << int32(itemEnchantData.ID);
    data << uint32(itemEnchantData.Expiration);
    data << int32(itemEnchantData.Charges);
    data << uint8(itemEnchantData.Slot);

    return data;
}

bool WorldPackets::Item::ItemInstance::operator==(ItemInstance const& r) const
{
    if (ItemID != r.ItemID || RandomPropertiesID != r.RandomPropertiesID || RandomPropertiesSeed != r.RandomPropertiesSeed)
        return false;

    if (ItemBonus.is_initialized() != r.ItemBonus.is_initialized() || Modifications.is_initialized() != r.Modifications.is_initialized())
        return false;

    if (Modifications.is_initialized() && *Modifications != *r.Modifications)
        return false;

    if (ItemBonus.is_initialized() && *ItemBonus != *r.ItemBonus)
        return false;

    return true;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemInstance const& itemInstance)
{
    data << int32(itemInstance.ItemID);
    data << int32(itemInstance.RandomPropertiesSeed);
    data << int32(itemInstance.RandomPropertiesID);

    data.WriteBit(itemInstance.ItemBonus.is_initialized());
    data.WriteBit(itemInstance.Modifications.is_initialized());
    data.FlushBits();

    if (itemInstance.ItemBonus)
        data << *itemInstance.ItemBonus;

    if (itemInstance.Modifications)
        data << *itemInstance.Modifications;

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Item::ItemInstance& itemInstance)
{
    data >> itemInstance.ItemID;
    data >> itemInstance.RandomPropertiesSeed;
    data >> itemInstance.RandomPropertiesID;

    bool hasItemBonus = data.ReadBit();
    bool hasModifications = data.ReadBit();
    data.ResetBitPos();

    if (hasItemBonus)
    {
        itemInstance.ItemBonus = boost::in_place();
        data >> *itemInstance.ItemBonus;
    }

    if (hasModifications)
    {
        itemInstance.Modifications = boost::in_place();
        data >> *itemInstance.Modifications;
    }

    return data;
}

void WorldPackets::Item::ItemInstance::Initialize(::Item const* item)
{
    if (!item)
        return;

    ItemID = item->GetEntry();
    RandomPropertiesSeed = item->GetItemSuffixFactor();
    RandomPropertiesID = item->GetItemRandomPropertyId();
    std::vector<uint32> const& bonusListIds = item->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);
    if (!bonusListIds.empty())
    {
        ItemBonus = boost::in_place();
        ItemBonus->BonusListIDs.insert(ItemBonus->BonusListIDs.end(), bonusListIds.begin(), bonusListIds.end());
        ItemBonus->Context = item->GetUInt32Value(ITEM_FIELD_CONTEXT);
    }

    if (uint32 mask = item->GetUInt32Value(ITEM_FIELD_MODIFIERS_MASK))
    {
        Modifications = boost::in_place();

        for (size_t i = 0; mask != 0; mask >>= 1, ++i)
            if ((mask & 1) != 0)
                Modifications->Insert(i, item->GetModifier(ItemModifier(i)));
    }
}

void WorldPackets::Item::ItemInstance::Initialize(LootItem const& lootItem)
{
    ItemID = lootItem.item.ItemID;
    RandomPropertiesSeed = lootItem.item.RandomPropertiesSeed;
    if (lootItem.item.RandomPropertiesID.Type != ItemRandomEnchantmentType::BonusList)
        RandomPropertiesID = lootItem.item.RandomPropertiesID.Id;
    if (!lootItem.item.ItemBonus.BonusListIDs.empty())
    {
        ItemBonus = boost::in_place();
        ItemBonus->BonusListIDs = lootItem.item.ItemBonus.BonusListIDs;
        ItemBonus->Context = lootItem.item.ItemBonus.Context;
    }

    if (lootItem.item.UpgradeID)
    {
        Modifications = boost::in_place();
        Modifications->Insert(ITEM_MODIFIER_UPGRADE_ID, lootItem.item.UpgradeID);
    }
}

void WorldPackets::Item::ItemInstance::Initialize(VoidStorageItem const* voidItem)
{
    ItemID = voidItem->ItemEntry;
    RandomPropertiesSeed = voidItem->ItemSuffixFactor;
    if (voidItem->ItemRandomPropertyId.Type != ItemRandomEnchantmentType::BonusList)
        RandomPropertiesID = voidItem->ItemRandomPropertyId.Id;
    if (voidItem->ItemUpgradeId)
    {
        Modifications = boost::in_place();
        Modifications->Insert(ITEM_MODIFIER_UPGRADE_ID, voidItem->ItemUpgradeId);
    }
}

void WorldPackets::Item::ItemInstance::Initialize(Roll const* roll)
{
    ItemID = roll->item.ItemID;
    RandomPropertiesSeed = roll->item.RandomPropertiesSeed;
    if (roll->item.RandomPropertiesID.Type != ItemRandomEnchantmentType::BonusList)
        RandomPropertiesID = roll->item.RandomPropertiesID.Id;
    if (!roll->item.ItemBonus.BonusListIDs.empty())
    {
        ItemBonus = boost::in_place();
        ItemBonus->BonusListIDs = roll->item.ItemBonus.BonusListIDs;
        ItemBonus->Context = roll->item.ItemBonus.Context;
    }
}

void WorldPackets::Item::ItemInstance::Initialize(ItemDynamicFieldGems const* gem)
{
    ItemID = gem->ItemId;

    ItemBonusInstanceData bonus;
    bonus.Context = gem->Context;
    for (uint16 bonusListId : gem->BonusListIDs)
        if (bonusListId)
            bonus.BonusListIDs.push_back(bonusListId);

    if (bonus.Context || !bonus.BonusListIDs.empty())
        ItemBonus = bonus;
}

void WorldPackets::Item::ItemInstance::Initialize(VendorItem const* vendorItem)
{
    ItemID = vendorItem->item;

    ItemBonusInstanceData bonus;
    bonus.Context = vendorItem->Context;
    RandomPropertiesSeed = vendorItem->RandomPropertiesSeed;
    if (vendorItem->RandomPropertiesID.Type != ItemRandomEnchantmentType::BonusList)
        RandomPropertiesID = vendorItem->RandomPropertiesID.Id;

    for (uint16 bonusListId : vendorItem->BonusListIDs)
        if (bonusListId)
            bonus.BonusListIDs.push_back(bonusListId);

    // for (uint16 itemModifier : vendorItem->ItemModifiers)
        // if (itemModifier)
            // bonus.Modifications.push_back(itemModifier);

    if (bonus.Context || !bonus.BonusListIDs.empty())
        ItemBonus = bonus;
}

void WorldPackets::Item::ItemInstance::Initialize(std::string item)
{
    ItemBonusInstanceData bonus;
    Tokenizer tokenListIDs(item, ' ');
    int8 count = 0;
    for (char const* token : tokenListIDs)
    {
        switch (count)
        {
            case 0:
                RandomPropertiesSeed = uint32(atoul(token));
                break;
            case 1:
                RandomPropertiesID = uint32(atoul(token));
                break;
            case 2:
                bonus.Context = uint32(atoul(token));
                break;
            default:
                bonus.BonusListIDs.push_back(uint32(atoul(token)));
                break;
        }
        count++;
    }

    if (bonus.Context || !bonus.BonusListIDs.empty())
        ItemBonus = bonus;
}

ByteBuffer& WorldPackets::Item::operator>>(ByteBuffer& data, InvUpdate& invUpdate)
{
    invUpdate.Items.resize(data.ReadBits(2));
    for (size_t i = 0; i < invUpdate.Items.size(); ++i)
    {
        data >> invUpdate.Items[i].ContainerSlot;
        data >> invUpdate.Items[i].Slot;
    }

    return data;
}

WorldPacket const* WorldPackets::Item::InventoryChangeFailure::Write()
{
    _worldPacket << int8(BagResult);
    _worldPacket << Item[0];
    _worldPacket << Item[1];
    _worldPacket << uint8(ContainerBSlot);

    switch (BagResult)
    {
        case EQUIP_ERR_CANT_EQUIP_LEVEL_I:
        case EQUIP_ERR_PURCHASE_LEVEL_TOO_LOW:
            _worldPacket << int32(Level);
            break;
        case EQUIP_ERR_EVENT_AUTOEQUIP_BIND_CONFIRM:
            _worldPacket << SrcContainer;
            _worldPacket << int32(SrcSlot);
            _worldPacket << DstContainer;
            break;
        case EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED_IS:
        case EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED_IS:
        case EQUIP_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED_IS:
            _worldPacket << int32(LimitCategory);
            break;
        default:
            break;
    }

    return &_worldPacket;
}

void WorldPackets::Item::SplitItem::Read()
{
    _worldPacket >> Inv;
    _worldPacket >> FromPackSlot;
    _worldPacket >> FromSlot;
    _worldPacket >> ToPackSlot;
    _worldPacket >> ToSlot;
    _worldPacket >> Quantity;
}

void WorldPackets::Item::SwapInvItem::Read()
{
    _worldPacket >> Inv;
    _worldPacket >> Slot2;
    _worldPacket >> Slot1;
}

void WorldPackets::Item::SwapItem::Read()
{
    _worldPacket >> Inv;
    _worldPacket>> ContainerSlotB;
    _worldPacket>> ContainerSlotA;
    _worldPacket>> SlotB;
    _worldPacket>> SlotA;
}

void WorldPackets::Item::AutoEquipItem::Read()
{
    _worldPacket >> Inv;
    _worldPacket >> PackSlot;
    _worldPacket >> Slot;
}

void WorldPackets::Item::AutoBankReagent::Read()
{
    _worldPacket >> Inv;
    _worldPacket >> PackSlot;
    _worldPacket >> Slot;
}

void WorldPackets::Item::AutoStoreBagItem::Read()
{
    _worldPacket >> Inv;
    _worldPacket >> ContainerSlotA;
    _worldPacket >> ContainerSlotB;
    _worldPacket >> SlotA;
}

void WorldPackets::Item::DestroyItem::Read()
{
    _worldPacket >> Count
        >> ContainerId
        >> SlotNum;
}

void WorldPackets::Item::BuyItem::Read()
{
    _worldPacket >> VendorGUID;
    _worldPacket >> ContainerGUID;

    _worldPacket >> Quantity;
    _worldPacket >> Muid;
    _worldPacket >> Slot;

    _worldPacket >> Item;

    _worldPacket.ResetBitReader();
    ItemType = static_cast<ItemVendorType>(_worldPacket.ReadBits(2));
}

WorldPacket const* WorldPackets::Item::ItemPushResult::Write()
{
    _worldPacket << PlayerGUID;
    _worldPacket << uint8(Slot);
    _worldPacket << int32(SlotInBag);
    _worldPacket << uint32(QuestLogItemID);
    _worldPacket << int32(Quantity);
    _worldPacket << int32(QuantityInInventory);
    _worldPacket << int32(DungeonEncounterID);
    _worldPacket << int32(BattlePetSpeciesID);
    _worldPacket << int32(BattlePetBreedID);
    _worldPacket << uint32(BattlePetBreedQuality);
    _worldPacket << int32(BattlePetLevel);
    _worldPacket << ItemGUID;
    _worldPacket.WriteBit(Pushed);
    _worldPacket.WriteBit(Created);
    _worldPacket.WriteBits(static_cast<uint16>(DisplayText), 3);
    _worldPacket.WriteBit(IsBonusRoll);
    _worldPacket.WriteBit(IsEncounterLoot);
    _worldPacket.FlushBits();
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::BuyFailed::Write()
{
    _worldPacket << VendorGUID;
    _worldPacket << uint32(Muid);
    _worldPacket << uint8(Reason);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::SellResponse::Write()
{
    _worldPacket << VendorGUID;
    _worldPacket << ItemGUID;
    _worldPacket << uint8(Reason);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::BuySucceeded::Write()
{
    _worldPacket << VendorGUID;
    _worldPacket << uint32(Muid);
    _worldPacket << int32(NewQuantity);
    _worldPacket << uint32(QuantityBought);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::ItemCooldown::Write()
{
    _worldPacket << ItemGuid;
    _worldPacket << SpellID;
    _worldPacket << Cooldown;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::EnchantmentLog::Write()
{
    _worldPacket << Caster;
    _worldPacket << Owner;
    _worldPacket << ItemGUID;
    _worldPacket << ItemID;
    _worldPacket << Enchantment;
    _worldPacket << EnchantSlot;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::ItemEnchantTimeUpdate::Write()
{
    _worldPacket << ItemGuid;
    _worldPacket << Duration;
    _worldPacket << Slot;
    _worldPacket << PlayerGuid;

    return &_worldPacket;
}

void WorldPackets::Item::UseCritterItem::Read()
{
    _worldPacket >> ItemGuid;
}

WorldPacket const* WorldPackets::Item::ItemExpirePurchaseRefund::Write()
{
    _worldPacket << ItemGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::ReadItemResultOk::Write()
{
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::ReadItemResultFailed::Write()
{
    _worldPacket << Item;
    _worldPacket << Delay;
    _worldPacket.WriteBits(Subcode, 2);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemPurchaseRefundItem& refundItem)
{
    data << refundItem.ItemID;
    data << refundItem.ItemCount;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemPurchaseRefundCurrency& refundCurrency)
{
    data << refundCurrency.CurrencyID;
    data << refundCurrency.CurrencyCount;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Item::ItemPurchaseContents& purchaseContents)
{
    data << purchaseContents.Money;
    for (uint32 i = 0; i < 5; ++i)
        data << purchaseContents.Items[i];

    for (uint32 i = 0; i < 5; ++i)
        data << purchaseContents.Currencies[i];

    return data;
}

WorldPacket const* WorldPackets::Item::SetItemPurchaseData::Write()
{
    _worldPacket << ItemGUID;
    _worldPacket << Contents;
    _worldPacket << Flags;
    _worldPacket << PurchaseTime;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::ItemPurchaseRefundResult::Write()
{
    _worldPacket << ItemGUID;
    _worldPacket << uint8(Result);
    _worldPacket.WriteBit(Contents.is_initialized());
    _worldPacket.FlushBits();
    if (Contents)
        _worldPacket << *Contents;

    return &_worldPacket;
}

void WorldPackets::Item::AutoEquipItemSlot::Read()
{
    _worldPacket >> Inv;
    _worldPacket >> Item;
    _worldPacket >> ItemDstSlot;
}

void WorldPackets::Item::CancelTempEnchantment::Read()
{
    _worldPacket >> Slot;
}

void WorldPackets::Item::WrapItem::Read()
{
    _worldPacket >> Inv;
}

void WorldPackets::Item::ItemPurchaseRefund::Read()
{
    _worldPacket >> ItemGUID;
}

void WorldPackets::Item::UpgradeItem::Read()
{
    _worldPacket >> ItemMaster;
    _worldPacket >> ItemGUID;
    _worldPacket >> UpgradeID;
    _worldPacket >> ContainerSlot;
    _worldPacket >> Slot;
}

void WorldPackets::Item::ReadItem::Read()
{
    _worldPacket >> PackSlot;
    _worldPacket >> Slot;
}

void WorldPackets::Item::SocketGems::Read()
{
    _worldPacket >> ItemGuid;
    for (ObjectGuid &gem : GemItem)
        _worldPacket >> gem;
}

WorldPacket const* WorldPackets::Item::SocketGemsResult::Write()
{
    _worldPacket << Item;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Item::SocketGemsFailure::Write()
{
    _worldPacket << Item;

    return &_worldPacket;
}

void WorldPackets::Item::RemoveNewItem::Read()
{
    _worldPacket >> ItemGuid;
}
