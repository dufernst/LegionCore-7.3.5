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

#include "BattlePayPackets.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductDisplayInfo const& displayInfo)
{
    data.WriteBit(displayInfo.CreatureDisplayInfoID.is_initialized());
    data.WriteBit(displayInfo.VisualsId.is_initialized());

    data.WriteBits(displayInfo.Name1.length(), 10);
    data.WriteBits(displayInfo.Name2.length(), 10);
    data.WriteBits(displayInfo.Name3.length(), 13);
    data.WriteBits(displayInfo.Name4.length(), 13);

    data.WriteBit(displayInfo.Flags.is_initialized());
    data.WriteBit(displayInfo.UnkInt1.is_initialized());
    data.WriteBit(displayInfo.UnkInt2.is_initialized());
    data.WriteBit(displayInfo.UnkInt3.is_initialized());
    data.FlushBits();

    data << static_cast<uint32>(displayInfo.Visuals.size());

    if (displayInfo.CreatureDisplayInfoID)
        data << *displayInfo.CreatureDisplayInfoID;

    if (displayInfo.VisualsId)
        data << *displayInfo.VisualsId;

    data.WriteString(displayInfo.Name1);
    data.WriteString(displayInfo.Name2);
    data.WriteString(displayInfo.Name3);
    data.WriteString(displayInfo.Name4);

    if (displayInfo.Flags)
        data << *displayInfo.Flags;

    if (displayInfo.UnkInt1)
        data << *displayInfo.UnkInt1;

    if (displayInfo.UnkInt2)
        data << *displayInfo.UnkInt2;

    if (displayInfo.UnkInt3)
        data << *displayInfo.UnkInt3;

    for (auto const& itr : displayInfo.Visuals)
    {
        data.WriteBits(itr.ProductName.length(), 10);
        data.FlushBits();
        data << itr.DisplayId;
        data << itr.VisualId;
        data << itr.ProductName;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayProduct const& product)
{
    data << product.ProductID;

    data << product.Type;
    data << product.Flags;
    data << product.UnkInt1;
    data << product.DisplayId;
    data << product.ItemId;
    data << product.UnkInt4;
    data << product.UnkInt5;

    data.WriteBits(product.UnkString.size(), 8);
    data.WriteBit(product.UnkBit);
    data.WriteBit(product.UnkBits.is_initialized());
    data.WriteBits(product.Items.size(), 7);
    data.WriteBit(product.DisplayInfo.is_initialized());

    if (product.UnkBits)
        data.WriteBits(*product.UnkBits, 4);

    data.FlushBits();

    for (auto const& productItem : product.Items)
    {
        data << productItem.ID;
        data << productItem.UnkByte;
        data << productItem.ItemID;
        data << productItem.Quantity;
        data << productItem.UnkInt1;
        data << productItem.UnkInt2;

        data.WriteBit(productItem.HasPet);
        data.WriteBit(productItem.PetResult.is_initialized());
        data.WriteBit(productItem.DisplayInfo.is_initialized());

        if (productItem.PetResult)
            data.WriteBits(*productItem.PetResult, 4);

        data.FlushBits();

        if (productItem.DisplayInfo)
            data << *productItem.DisplayInfo;
    }

    if (product.DisplayInfo)
        data << *product.DisplayInfo;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayDistributionObject const& object)
{
    data << object.DistributionID;

    data << object.Status;
    data << object.ProductID;

    data << object.TargetPlayer;
    data << object.TargetVirtualRealm;
    data << object.TargetNativeRealm;

    data << object.PurchaseID;
    data.WriteBit(object.Product.is_initialized());
    data.WriteBit(object.Revoked);
    data.FlushBits();

    if (object.Product)
        data << *object.Product;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::BattlePayPurchase const& purchase)
{
    data << purchase.PurchaseID;
    data << purchase.Status;
    data << purchase.ResultCode;
    data << purchase.ProductID;
    data << purchase.UnkLong;
    data << purchase.UnkLong2;
    data << purchase.UnkInt;
    
    data.WriteBits(purchase.WalletName.length(), 8);
    data.WriteString(purchase.WalletName);

    return data;
}

WorldPacket const* WorldPackets::BattlePay::PurchaseListResponse::Write()
{
    _worldPacket << Result;
    _worldPacket << static_cast<uint32>(Purchase.size());
    for (auto const& purchaseData : Purchase)
        _worldPacket << purchaseData;

    return &_worldPacket;
}

std::vector<uint8_t> hardCodedDistributionListResponse =
{
    0, 0, 0, 0, 0, 32, 1, 0, 0, 0, 0, 128, 119, 30, 1, 0, 0, 0, 109, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 128, 119, 30,
    128, 109, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 129, 128, 0, 35, 224, 0, 0,
    0, 0, 0, 0, 84, 97, 9, 0, 76, 101, 118, 101, 108, 32, 57, 48, 32, 67, 104, 97,
    114, 97, 99, 116, 101, 114, 32, 66, 111, 111, 115, 116, 84, 104, 101, 114, 101, 32, 99, 111,
    109, 101, 115, 32, 97, 32, 116, 105, 109, 101, 32, 105, 110, 32, 101, 118, 101, 114, 121, 32,
    104, 101, 114, 111, 226, 128, 153, 115, 32, 113, 117, 101, 115, 116, 32, 119, 104, 101, 110, 32,
    116, 104, 101, 121, 32, 110, 101, 101, 100, 32, 97, 32, 108, 105, 116, 116, 108, 101, 32, 98,
    111, 111, 115, 116, 32, 116, 111, 32, 104, 101, 108, 112, 32, 103, 101, 116, 32, 116, 104, 101,
    109, 32, 111, 118, 101, 114, 32, 116, 104, 101, 32, 104, 117, 109, 112, 32, 97, 110, 100, 32,
    98, 97, 99, 107, 32, 105, 110, 116, 111, 32, 116, 104, 101, 32, 97, 99, 116, 105, 111, 110,
    46, 32, 87, 105, 116, 104, 32, 97, 32, 76, 101, 118, 101, 108, 32, 57, 48, 32, 67, 104,
    97, 114, 97, 99, 116, 101, 114, 32, 66, 111, 111, 115, 116, 44, 32, 121, 111, 117, 32, 99,
    97, 110, 32, 103, 114, 97, 110, 116, 32, 111, 110, 101, 32, 99, 104, 97, 114, 97, 99, 116,
    101, 114, 32, 97, 32, 111, 110, 101, 45, 116, 105, 109, 101, 32, 98, 111, 111, 115, 116, 32,
    116, 111, 32, 108, 101, 118, 101, 108, 32, 57, 48, 46, 32, 66, 114, 105, 110, 103, 32, 121,
    111, 117, 114, 32, 104, 101, 114, 111, 32, 117, 112, 32, 116, 111, 32, 115, 112, 101, 101, 100,
    32, 97, 110, 100, 32, 106, 111, 105, 110, 32, 116, 104, 101, 32, 102, 105, 103, 104, 116, 32,
    111, 110, 32, 116, 104, 101, 32, 102, 114, 111, 110, 116, 32, 108, 105, 110, 101, 115, 46
};

WorldPacket const* WorldPackets::BattlePay::DistributionListResponse::Write()
{
    for (const uint8_t byte : hardCodedDistributionListResponse)
        _worldPacket << byte;

    /*
    _worldPacket << Result;
    _worldPacket.WriteBits(DistributionObject.size(), 11);
    for (BattlePayDistributionObject const& objectData : DistributionObject)
        _worldPacket << objectData;
    */

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::DistributionUpdate::Write()
{
    _worldPacket << DistributionObject;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::ProductInfoStruct const& info)
{
    data << info.ProductID;
    data << info.NormalPriceFixedPoint;
    data << info.CurrentPriceFixedPoint;
    data << static_cast<uint32>(info.ProductIDs.size());
    data << info.UnkInt2;
    data << static_cast<uint32>(info.UnkInts.size());

    for (auto z : info.ProductIDs)
        data << z;

    for (auto z : info.UnkInts)
        data << z;

    data.WriteBits(info.ChoiceType, 7);
    data.WriteBit(info.DisplayInfo.is_initialized());
    if (info.DisplayInfo)
        data << *info.DisplayInfo;

    return data;
}

std::vector<uint8_t> hardCodedProductListResponse =
{
    0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0,
    1, 0, 0, 0, 109, 0, 0, 0, 64, 66, 15, 0, 0, 0, 0, 0, 64, 66, 15, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 109, 0, 0, 0,
    5, 129, 128, 0, 35, 224, 0, 0, 0, 0, 0, 0, 84, 97, 9, 0, 76, 101, 118, 101,
    108, 32, 57, 48, 32, 67, 104, 97, 114, 97, 99, 116, 101, 114, 32, 66, 111, 111, 115, 116,
    84, 104, 101, 114, 101, 32, 99, 111, 109, 101, 115, 32, 97, 32, 116, 105, 109, 101, 32, 105,
    110, 32, 101, 118, 101, 114, 121, 32, 104, 101, 114, 111, 226, 128, 153, 115, 32, 113, 117, 101,
    115, 116, 32, 119, 104, 101, 110, 32, 116, 104, 101, 121, 32, 110, 101, 101, 100, 32, 97, 32,
    108, 105, 116, 116, 108, 101, 32, 98, 111, 111, 115, 116, 32, 116, 111, 32, 104, 101, 108, 112,
    32, 103, 101, 116, 32, 116, 104, 101, 109, 32, 111, 118, 101, 114, 32, 116, 104, 101, 32, 104,
    117, 109, 112, 32, 97, 110, 100, 32, 98, 97, 99, 107, 32, 105, 110, 116, 111, 32, 116, 104,
    101, 32, 97, 99, 116, 105, 111, 110, 46, 32, 87, 105, 116, 104, 32, 97, 32, 76, 101, 118,
    101, 108, 32, 57, 48, 32, 67, 104, 97, 114, 97, 99, 116, 101, 114, 32, 66, 111, 111, 115,
    116, 44, 32, 121, 111, 117, 32, 99, 97, 110, 32, 103, 114, 97, 110, 116, 32, 111, 110, 101,
    32, 99, 104, 97, 114, 97, 99, 116, 101, 114, 32, 97, 32, 111, 110, 101, 45, 116, 105, 109,
    101, 32, 98, 111, 111, 115, 116, 32, 116, 111, 32, 108, 101, 118, 101, 108, 32, 57, 48, 46,
    32, 66, 114, 105, 110, 103, 32, 121, 111, 117, 114, 32, 104, 101, 114, 111, 32, 117, 112, 32,
    116, 111, 32, 115, 112, 101, 101, 100, 32, 97, 110, 100, 32, 106, 111, 105, 110, 32, 116, 104,
    101, 32, 102, 105, 103, 104, 116, 32, 111, 110, 32, 116, 104, 101, 32, 102, 114, 111, 110, 116,
    32, 108, 105, 110, 101, 115, 46, 109, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 129,
    128, 0, 35, 224, 0, 0, 0, 0, 0, 0, 84, 97, 9, 0, 76, 101, 118, 101, 108, 32,
    57, 48, 32, 67, 104, 97, 114, 97, 99, 116, 101, 114, 32, 66, 111, 111, 115, 116, 84, 104,
    101, 114, 101, 32, 99, 111, 109, 101, 115, 32, 97, 32, 116, 105, 109, 101, 32, 105, 110, 32,
    101, 118, 101, 114, 121, 32, 104, 101, 114, 111, 226, 128, 153, 115, 32, 113, 117, 101, 115, 116,
    32, 119, 104, 101, 110, 32, 116, 104, 101, 121, 32, 110, 101, 101, 100, 32, 97, 32, 108, 105,
    116, 116, 108, 101, 32, 98, 111, 111, 115, 116, 32, 116, 111, 32, 104, 101, 108, 112, 32, 103,
    101, 116, 32, 116, 104, 101, 109, 32, 111, 118, 101, 114, 32, 116, 104, 101, 32, 104, 117, 109,
    112, 32, 97, 110, 100, 32, 98, 97, 99, 107, 32, 105, 110, 116, 111, 32, 116, 104, 101, 32,
    97, 99, 116, 105, 111, 110, 46, 32, 87, 105, 116, 104, 32, 97, 32, 76, 101, 118, 101, 108,
    32, 57, 48, 32, 67, 104, 97, 114, 97, 99, 116, 101, 114, 32, 66, 111, 111, 115, 116, 44,
    32, 121, 111, 117, 32, 99, 97, 110, 32, 103, 114, 97, 110, 116, 32, 111, 110, 101, 32, 99,
    104, 97, 114, 97, 99, 116, 101, 114, 32, 97, 32, 111, 110, 101, 45, 116, 105, 109, 101, 32,
    98, 111, 111, 115, 116, 32, 116, 111, 32, 108, 101, 118, 101, 108, 32, 57, 48, 46, 32, 66,
    114, 105, 110, 103, 32, 121, 111, 117, 114, 32, 104, 101, 114, 111, 32, 117, 112, 32, 116, 111,
    32, 115, 112, 101, 101, 100, 32, 97, 110, 100, 32, 106, 111, 105, 110, 32, 116, 104, 101, 32,
    102, 105, 103, 104, 116, 32, 111, 110, 32, 116, 104, 101, 32, 102, 114, 111, 110, 116, 32, 108,
    105, 110, 101, 115, 46, 1, 0, 0, 0, 165, 4, 2, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 5, 0, 0, 1, 77, 111, 117, 110, 116, 2, 0, 0, 0, 16, 211, 9, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 80, 101, 116, 115, 3, 0, 0, 0,
    183, 48, 17, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 1, 83, 101, 114,
    118, 105, 99, 101, 115, 10, 0, 0, 0, 4, 199, 15, 0, 0, 11, 0, 0, 0, 0, 0,
    0, 0, 6, 0, 0, 1, 66, 111, 111, 115, 116, 115, 12, 0, 0, 0, 9, 192, 16, 0,
    0, 8, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 1, 72, 101, 105, 114, 108, 111, 111,
    109, 115, 109, 0, 0, 0, 3, 0, 0, 0, 109, 0, 0, 0, 5, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};

WorldPacket const* WorldPackets::BattlePay::ProductListResponse::Write()
{
    for (const uint8_t byte : hardCodedProductListResponse)
        _worldPacket << byte;

/*
    _worldPacket << Result;
    _worldPacket << ProductList.CurrencyID;

    _worldPacket << static_cast<uint32>(ProductList.ProductInfo.size());
    _worldPacket << static_cast<uint32>(ProductList.Product.size());
    _worldPacket << static_cast<uint32>(ProductList.ProductGroup.size());
    _worldPacket << static_cast<uint32>(ProductList.Shop.size());

    for (auto const& v : ProductList.ProductInfo)
        _worldPacket << v;

    for (auto const& productData : ProductList.Product)
        _worldPacket << productData;

    for (auto const& productGroupData : ProductList.ProductGroup)
    {
        _worldPacket << productGroupData.GroupID;
        _worldPacket << productGroupData.IconFileDataID;
        _worldPacket << productGroupData.DisplayType;
        _worldPacket << productGroupData.Ordering;
        _worldPacket << productGroupData.UnkInt;

        _worldPacket.WriteBits(productGroupData.Name.length(), 8);
        _worldPacket.WriteBits(productGroupData.IsAvailableDescription.length() + 1, 24);
        _worldPacket.WriteString(productGroupData.Name);
        if (!productGroupData.IsAvailableDescription.empty())
            _worldPacket << productGroupData.IsAvailableDescription;
    }

    for (BattlePayShopEntry const& shopData : ProductList.Shop)
    {
        _worldPacket << shopData.EntryID;
        _worldPacket << shopData.GroupID;
        _worldPacket << shopData.ProductID;
        _worldPacket << shopData.Ordering;
        _worldPacket << shopData.VasServiceType;
        _worldPacket << shopData.StoreDeliveryType;

        if (_worldPacket.WriteBit(shopData.DisplayInfo.is_initialized()))
            _worldPacket << *shopData.DisplayInfo;

        _worldPacket.FlushBits();
    }
*/
    return &_worldPacket;
}

void WorldPackets::BattlePay::StartPurchase::Read()
{
    _worldPacket >> ClientToken;
    _worldPacket >> ProductID;
    _worldPacket >> TargetCharacter;
}

void WorldPackets::BattlePay::PurchaseProduct::Read()
{
    _worldPacket >> ClientToken;
    _worldPacket >> ProductID;
    _worldPacket >> TargetCharacter;

    uint32 strlen1 = _worldPacket.ReadBits(6);
    uint32 strlen2 = _worldPacket.ReadBits(12);
    WowSytem = _worldPacket.ReadString(strlen1);
    PublicKey = _worldPacket.ReadString(strlen2);
}

WorldPacket const* WorldPackets::BattlePay::StartPurchaseResponse::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << PurchaseResult;
    _worldPacket << ClientToken;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayAckFailed::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << PurchaseResult;
    _worldPacket << ClientToken;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::PurchaseUpdate::Write()
{
    _worldPacket << static_cast<uint32>(Purchase.size());
    for (auto const& purchaseData : Purchase)
        _worldPacket << purchaseData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::ConfirmPurchase::Write()
{
    _worldPacket << PurchaseID;
    _worldPacket << ServerToken;

    return &_worldPacket;
}

void WorldPackets::BattlePay::ConfirmPurchaseResponse::Read()
{
    ConfirmPurchase = _worldPacket.ReadBit();
    _worldPacket >> ServerToken;
    _worldPacket >> ClientCurrentPriceFixedPoint;
}

void WorldPackets::BattlePay::BattlePayAckFailedResponse::Read()
{
    _worldPacket >> ServerToken;
}

WorldPacket const* WorldPackets::BattlePay::DeliveryEnded::Write()
{
    _worldPacket << DistributionID;

    _worldPacket << static_cast<int32>(item.size());
    for (Item::ItemInstance const& itemData : item)
        _worldPacket << itemData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayDeliveryStarted::Write()
{
    _worldPacket << DistributionID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::UpgradeStarted::Write()
{
    _worldPacket << CharacterGUID;

    return &_worldPacket;
}

void WorldPackets::BattlePay::DistributionAssignToTarget::Read()
{
    _worldPacket >> ProductID;
    _worldPacket >> DistributionID;
    _worldPacket >> TargetCharacter;
    _worldPacket >> SpecializationID;
    _worldPacket >> ChoiceID;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePay::VasPurchaseData const& purchase)
{
    data << purchase.PlayerGuid;
    data << purchase.UnkInt;
    data << purchase.UnkInt2;
    data << purchase.UnkLong;
    data.WriteBits(purchase.ItemIDs.size(), 2);
    data.FlushBits();

    for (auto const& itemID : purchase.ItemIDs)
        data << itemID;

    return data;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayVasPurchaseStarted::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << VasPurchase;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::CharacterClassTrialCreate::Write()
{
    _worldPacket << Result;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayCharacterUpgradeQueued::Write()
{
    _worldPacket << Character;
    _worldPacket << static_cast<uint32>(EquipmentItems.size());
    for (auto const& item : EquipmentItems)
        _worldPacket << item;

    return &_worldPacket;
}

void WorldPackets::BattlePay::BattlePayTrialBoostCharacter::Read()
{
    _worldPacket >> Character;
    _worldPacket >> SpecializationID;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayVasPurchaseList::Write()
{
    _worldPacket.WriteBits(VasPurchase.size(), 6);
    _worldPacket.FlushBits();
    for (auto const& itr : VasPurchase)
        _worldPacket << itr;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayPurchaseDetails::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << VasPurchaseProgress;
    _worldPacket << UnkLong;

    _worldPacket.WriteBits(Key.length(), 6);
    _worldPacket.WriteBits(Key2.length(), 6);
    _worldPacket.WriteString(Key);
    _worldPacket.WriteString(Key2);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayPurchaseUnk::Write()
{
    _worldPacket << UnkByte;
    _worldPacket << UnkInt;

    _worldPacket.WriteBits(Key.length(), 7);
    _worldPacket.WriteString(Key);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayBattlePetDelivered::Write()
{
    _worldPacket << DisplayID;
    _worldPacket << BattlePetGuid;

    return &_worldPacket;
}

void WorldPackets::BattlePay::BattlePayPurchaseDetailsResponse::Read()
{
    _worldPacket >> UnkByte;
}

void WorldPackets::BattlePay::BattlePayPurchaseUnkResponse::Read()
{
    auto keyLen = _worldPacket.ReadBits(6);
    auto key2Len = _worldPacket.ReadBits(7);
    Key = _worldPacket.ReadString(keyLen);
    Key2 = _worldPacket.ReadString(key2Len);
}

WorldPacket const* WorldPackets::BattlePay::DisplayPromotion::Write()
{
    _worldPacket << PromotionID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlepayUnk::Write()
{
    _worldPacket << UnkInt;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePay::BattlePayStartDistributionAssignToTargetResponse::Write()
{
    _worldPacket << DistributionID;
    _worldPacket << unkint1;
    _worldPacket << unkint2;

    return &_worldPacket;
}
