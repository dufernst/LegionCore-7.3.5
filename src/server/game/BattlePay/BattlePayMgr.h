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

#ifndef __TRINITY_BATTLEPAYMGR_H
#define __TRINITY_BATTLEPAYMGR_H

#include "Packets/BattlePayPackets.h"

class LoginQueryHolder;
class WorldSession;

//namespace BattlepayProductDecorator
//{
//    enum : uint8
//    {
//        Boost = 0,
//        Expansion = 1,
//        WoWToken = 2,
//        VasService = 3
//    };
//};
//
//namespace VasServiceType
//{
//    enum : uint8
//    {
//        NameChange = 0,
//        FactionChange = 1,
//        AppearanceChange = 2,
//        RaceChange = 3,
//        CharacterTransfer = 4,
//        FactionTransfer = 5,
//    };
//};
//
//namespace BattlepayDisplayFlag
//{
//    enum : uint8
//    {
//        None = 0x00,
//        CardDoesNotShowModel = 0x02,
//        CardAlwaysShowsTexture = 0x04,
//        HiddenPrice = 0x08,
//        UseHorizontalLayoutForFullCard = 0x10,
//    };
//};
//
//namespace StoreDeliveryType
//{
//    enum : uint32
//    {
//        Item = 0,
//        Service = 2, /// -> VasServiceType
//        RestoreCharacter = 3,
//        FactionChange = 4,
//        RaceChange = 5,
//
//        //"StoreDeliveryType" Item = 0
//        //"StoreDeliveryType" Mount = 1
//        //"StoreDeliveryType" Battlepet = 2
//        //"StoreDeliveryType" Collection = 3
//    };
//}
//


/*
sub_79B9DA(&a1, st7_0, "StoreError", "InvalidPaymentMethod", 0);
sub_79B9DA(&a1, st7_0, "StoreError", "PaymentFailed", 1);
sub_79B9DA(&a1, st7_0, "StoreError", "WrongCurrency", 2);
sub_79B9DA(&a1, st7_0, "StoreError", "BattlepayDisabled", 3);
sub_79B9DA(&a1, st7_0, "StoreError", "InsufficientBalance", 4);
sub_79B9DA(&a1, st7_0, "StoreError", "Other", 5);
sub_79B9DA(&a1, st7_0, "StoreError", "AlreadyOwned", 6);
sub_79B9DA(&a1, st7_0, "StoreError", "ParentalControlsNoPurchase", 7);
sub_79B9DA(&a1, st7_0, "StoreError", "PurchaseDenied", 8);
sub_79B9DA(&a1, st7_0, "StoreError", "ConsumableTokenOwned", 9);
sub_79B9DA(&a1, st7_0, "StoreError", "TooManyTokens", 10);
sub_79B9DA(&a1, st7_0, "StoreError", "ItemUnavailable", 11);

sub_79B9DA(&a1, st7_0, "VasError", "InvalidDestinationAccount", 6);
sub_79B9DA(&a1, st7_0, "VasError", "InvalidSourceAccount", 7);
sub_79B9DA(&a1, st7_0, "VasError", "DisallowedSourceAccount", 8);
sub_79B9DA(&a1, st7_0, "VasError", "DisallowedDestinationAccount", 9);
sub_79B9DA(&a1, st7_0, "VasError", "LowerBoxLevel", 11);
sub_79B9DA(&a1, st7_0, "VasError", "RealmNotEligible", 43);
sub_79B9DA(&a1, st7_0, "VasError", "CannotMoveGuildMaster", 44);
sub_79B9DA(&a1, st7_0, "VasError", "MaxCharactersOnServer", 45);
sub_79B9DA(&a1, st7_0, "VasError", "DuplicateCharacterName", 47);
sub_79B9DA(&a1, st7_0, "VasError", "HasMail", 48);
sub_79B9DA(&a1, st7_0, "VasError", "UnderMinLevelReq", 53);
sub_79B9DA(&a1, st7_0, "VasError", "CharacterTransferTooSoon", 55);
sub_79B9DA(&a1, st7_0, "VasError", "TooMuchMoneyForLevel", 64);
sub_79B9DA(&a1, st7_0, "VasError", "HasAuctions", 65);
sub_79B9DA(&a1, st7_0, "VasError", "NameNotAvailable", 83);
sub_79B9DA(&a1, st7_0, "VasError", "LastRenameTooRecent", 84);
sub_79B9DA(&a1, st7_0, "VasError", "CustomizeAlreadyRequested", 89);
sub_79B9DA(&a1, st7_0, "VasError", "LastCustomizeTooRecent", 90);
sub_79B9DA(&a1, st7_0, "VasError", "FactionChangeTooSoon", 93);
sub_79B9DA(&a1, st7_0, "VasError", "RaceClassComboIneligible", 94);
sub_79B9DA(&a1, st7_0, "VasError", "IneligibleMapID", 108);
sub_79B9DA(&a1, st7_0, "VasError", "BattlepayDeliveryPending", 110);
sub_79B9DA(&a1, st7_0, "VasError", "HasWoWToken", 111);
sub_79B9DA(&a1, st7_0, "VasError", "CharLocked", 58);
sub_79B9DA(&a1, st7_0, "VasError", "LastSaveTooRecent", 66);
sub_79B9DA(&a1, st7_0, "VasError", "HasHeirloom", 112);
sub_79B9DA(&a1, st7_0, "VasError", "LastSaveTooDistant", 115);
sub_79B9DA(&a1, st7_0, "VasError", "HasCagedBattlePet", 116);
sub_79B9DA(&a1, st7_0, "VasError", "BoostedTooRecently", 117);

"VasQueueStatus" UnderAnHour = 0
"VasQueueStatus" OneToThreeHours = 1
"VasQueueStatus" ThreeToSixHours = 2
"VasQueueStatus" SixToTwelveHours = 3
"VasQueueStatus" OverTwelveHours = 4
"VasQueueStatus" Over1Days = 5
"VasQueueStatus" Over2Days = 6
"VasQueueStatus" Over3Days = 7
"VasQueueStatus" Over4Days = 8
"VasQueueStatus" Over5Days = 9
"VasQueueStatus" Over6Days = 10
"VasQueueStatus" Over7Days = 11

*/

namespace Battlepay
{
    const float g_CurrencyPrecision = 10000.0f;

    namespace BattlepayCustomType
    {
        enum : uint8
        {
            BattlePayShop,
            VendorBuyCurrency,
            VendorBuyItem,
        };
    };

    namespace BattlepayProductGroupFlag
    {
        enum : uint8
        {
            None = 0x00,
            HideOwnedProducts = 0x01,
            EnabledForTrial = 0x2,
            DisableOwnedProducts = 0x04,
            EnabledForVeteran = 0x08,
            HideForNonveterans = 0x10,
        };
    };

    /// Client error enum See Blizzard_StoreUISecure.lua Last update : 6.2.3 20779
    enum Error
    {
        InvalidPaymentMethod = 25,
        PaymentFailed = 2,
        WrongCurrency = 12,
        BattlepayDisabled = 13,    ///< Also 14
        InsufficientBalance = 28,    ///< Also 29
        Other = 3,
        //AlreadyOwned               = ,
        ParentalControlsNoPurchase = 34,
        PurchaseDenied = 1,
        ConsumableTokenOwned = 46,
        TooManyTokens = 47,
        Ok = 0,
        /// ItemUnavailable
    };

    namespace DistributionStatus
    {
        enum 
        {
            BATTLE_PAY_DIST_STATUS_NONE = 0,
            BATTLE_PAY_DIST_STATUS_AVAILABLE = 1,
            BATTLE_PAY_DIST_STATUS_ADD_TO_PROCESS = 2,
            BATTLE_PAY_DIST_STATUS_PROCESS_COMPLETE = 3,    //send SMSG_BATTLE_PAY_VAS_PURCHASE_STARTED
            BATTLE_PAY_DIST_STATUS_FINISHED = 4
        };
    };

    namespace VasPurchaseProgress
    {
        enum : uint8
        {
            Invalid = 0,
            PrePurchase = 1,
            PaymentPending = 2,
            ApplyingLicense = 3,
            WaitingOnQueue = 4,
            Ready = 5,
            ProcessingFactionChange = 6,
            Complete = 7,
        };
    }

    /// Result of SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE see @BattlePay::SendProductList
    namespace ProductListResult
    {
        enum
        {
            Available = 0,       ///< The shop is available
            LockUnk1 = 1,       ///< The shop is locked, unknow reason (i've see nothing in client for case "1", all result > 0 lock the shop anyway)
            LockUnk2 = 2,       ///< The shop is locked, unknow reason (i've see nothing in client for case "1", all result > 0 lock the shop anyway)
            RegionLocked = 3        ///< The shop is locked because the region of the player is locked
        };
    }

    /// Update status of SMSG_BATTLE_PAY_PURCHASE_UPDATE see @Battlepay::SendPurchaseUpdate
    namespace UpdateStatus
    {
        enum
        {
            Loading = 9, // real 11 ?
            Ready = 6,
            Finish = 3
        };
    }

    namespace CustomMessage
    {
        enum
        {
            StoreBalance,
            AccountName,
            StoreBuyFailed
        };

        static const char* CustomMessage[] =
        {
            "STORE_BALANCE",
            "ACCOUNT_NAME",
            "STORE_BUY_FAILED"
        };

        inline const char* GetCustomMessage(uint8 id)
        {
            return CustomMessage[id];
        }
    }

    enum BattlePayCurrency
    {
        Unknow = 0,
        Usd = 1,
        Gbp = 2,
        Krw = 3,
        Eur = 4,
        Rub = 5,
        Ars = 8,
        CLP = 9,
        Mxn = 10,
        Brl = 11,
        Aud = 12,
        Cpt = 14,
        Tpt = 15,
        Beta = 16,
        Jpy = 28,
        Cad = 29,
        Nzd = 30
    };

    enum String
    {
        AtGoldLimit = 14090,
        NeedToBeInGame = 14091,
        TooHighLevel = 14092,
        YouAlreadyOwnThat = 14093,
        Level90Required = 14094,
        ReachPrimaryProfessionLimit = 14095,
        NotEnoughFreeBagSlots = 14096
    };

    enum WebsiteType
    {
        Category = 0,
        Spell = 1,
        Title = 2,
        Item = 3,
        Currency = 4,
        Rename = 5,
        GuildRename = 6,
        Gold = 7,
        Level = 8,
        Faction = 9,
        Race = 10,
        PremadeCharacter = 11,
        RealmTransfer = 12,
        ExpansionTransfer = 13,
        Premium = 14,
        DeletedCharacter = 15,
        ItemProfession = 16,
        Transmogrification = 17,
        PackItems = 18,
        CategoryProfession = 19,
        CategoryPremade = 20,
        ItemMount = 21,
        Customization = 22,
        CategoryCharacterManagement = 23,
        CategoryRealmTransfer = 24,
        CategoryExpansionTransfer = 25,
        CategoryGold = 26,
        CharacterBoost = 29,
        BattlePet = 30,

        MaxWebsiteType
    };

    namespace ProductGroups
    {
        enum : uint32
        {
            Mount = 1,
            Pets = 2,
            Services = 3,
            Golds = 4,
            Professions = 5,
            Armors = 7,
            Weapons = 8,
            Toys = 9,
            Boosts = 10,
            Bags = 11,
            Heirlooms = 12,
        };
    }

    namespace BattlepayGroupDisplayType
    {
        enum : uint8
        {
            Default = 0,
            Splash = 1,
            DoubleWide = 2,
        };
    }

    struct ProductGroup
    {
        uint32 GroupID;
        int32 IconFileDataID;
        int32 Ordering;
        std::string Name;
        uint8 DisplayType; ///< BattlepayGroupDisplayType
    };

    struct DisplayInfo
    {
        uint32 CreatureDisplayInfoID;
        uint32 VisualsId;
        uint32 Flags;
        std::string Name1;
        std::string Name2;
        std::string Name3;
        std::string Name4;
    };

    struct ProductItem
    {
        uint32 ID;
        uint32 ItemID;
        uint32 Quantity;
        uint32 DisplayInfoID;
        uint8 PetResult;
        bool HasPet;
    };

    struct Product
    {
        /// Databases fields
        std::vector<ProductItem> Items;
        uint64 NormalPriceFixedPoint;
        uint64 CurrentPriceFixedPoint;
        uint32 ProductID;
        uint32 Flags;
        uint32 DisplayInfoID;
        uint32 ClassMask;
        std::string ScriptName;
        uint8 WebsiteType;
        uint8 Type;
        uint8 ChoiceType;

        /// Custom fields
        std::string CustomData;
        std::string Serialize() const;
    };

    struct ShopEntry
    {
        uint32 EntryID;
        uint32 GroupID;
        uint32 ProductID;
        uint32 Flags;
        uint32 DisplayInfoID;
        int32 Ordering;
        uint8 BannerType;
    };

    struct Purchase
    {
        Purchase()
        {
            memset(this, 0, sizeof(Purchase));
        }

        ObjectGuid TargetCharacter;
        uint64 DistributionId;
        uint64 PurchaseID;
        uint64 CurrentPrice;
        uint32 ClientToken;
        uint32 ServerToken;
        uint32 ProductID;
        uint8 Status;
        bool Lock;
    };

    struct ProductGroupLocale
    {
        StringVector Name;
    };
}

class BattlepayManager
{
    Battlepay::Purchase _actualTransaction;
    std::map<uint32, Battlepay::Product> _existProducts;
    
    WorldSession* _session;
    uint64 _purchaseIDCount;
    uint64 _distributionIDCount;
    std::string _walletName;
public:
    explicit BattlepayManager(WorldSession* session);
    ~BattlepayManager();

    Battlepay::BattlePayCurrency GetShopCurrency() const;
    bool IsAvailable() const;
    bool AlreadyOwnProduct(uint32 itemId) const;
    void ProcessDelivery(Battlepay::Purchase* purchase);
    void RegisterStartPurchase(Battlepay::Purchase purchase);
    uint64 GenerateNewPurchaseID();
    uint64 GenerateNewDistributionId();
    Battlepay::Purchase* GetPurchase();
    std::string const& GetDefaultWalletName() const;
    std::tuple<bool, WorldPackets::BattlePay::ProductDisplayInfo> WriteDisplayInfo(uint32 displayInfoID, LocaleConstant localeIndex, uint32 productId = 0);
    auto ProductFilter(Battlepay::Product product) -> bool;
    void SendProductList();
    void SendPointsBalance();
    void SendBattlePayDistribution(uint32 productId, uint8 status, uint64 distributionId, ObjectGuid targetGuid = ObjectGuid::Empty);
    void AssignDistributionToCharacter(ObjectGuid const& targetCharGuid, uint64 distributionId, uint32 productId, uint16 specialization_id, uint16 choice_id);
    void Update(uint32 diff);
};

#endif