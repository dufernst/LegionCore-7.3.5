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

#include "Common.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "AuctionHouseMgr.h"
#include "AuctionHouseBot.h"
#include "Item.h"
#include "Language.h"
#include "Log.h"
#include "ItemPackets.h"
#include "AuctionHousePackets.h"

enum eAuctionHouse
{
    AH_MINIMUM_DEPOSIT = 100
};

AuctionHouseMgr::AuctionHouseMgr() = default;

AuctionHouseMgr::~AuctionHouseMgr()
{
    for (auto& item : mAitems)
        delete item.second;
}

AuctionHouseMgr* AuctionHouseMgr::instance()
{
    static AuctionHouseMgr instance;
    return &instance;
}

AuctionHouseObject* AuctionHouseMgr::GetAuctionsMap(uint32 factionTemplateId)
{
    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
        return &mNeutralAuctions;

    // team have linked auction houses
    FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(factionTemplateId);
    if (!u_entry)
        return &mNeutralAuctions;
    if (u_entry->FactionGroup & FACTION_MASK_ALLIANCE)
        return &mAllianceAuctions;
    if (u_entry->FactionGroup & FACTION_MASK_HORDE)
        return &mHordeAuctions;
    return &mNeutralAuctions;
}

Item* AuctionHouseMgr::GetAItem(ObjectGuid::LowType const& id)
{
    return Trinity::Containers::MapGetValuePtr(mAitems, id);
}

uint32 AuctionHouseMgr::GetAuctionDeposit(AuctionHouseEntry const* entry, uint32 time, Item* pItem, uint32 count)
{
    uint32 MSV = pItem->GetSellPrice();

    if (MSV <= 0)
        return AH_MINIMUM_DEPOSIT;

    float multiplier = CalculatePct(float(entry->DepositRate), 3);
    uint32 timeHr = (((time / 60) / 60) / 12);
    auto deposit = uint32(((multiplier * MSV * count / 3) * timeHr * 3) * sWorld->getRate(RATE_AUCTION_DEPOSIT));

    TC_LOG_DEBUG(LOG_FILTER_AUCTIONHOUSE, "MSV:        %u", MSV);
    TC_LOG_DEBUG(LOG_FILTER_AUCTIONHOUSE, "Items:      %u", count);
    TC_LOG_DEBUG(LOG_FILTER_AUCTIONHOUSE, "Multiplier: %f", multiplier);
    TC_LOG_DEBUG(LOG_FILTER_AUCTIONHOUSE, "Deposit:    %u", deposit);

    if (deposit < AH_MINIMUM_DEPOSIT)
        return AH_MINIMUM_DEPOSIT;
    return deposit;
}

//does not clear ram
void AuctionHouseMgr::SendAuctionWonMail(AuctionEntry* auction, SQLTransaction& trans)
{
    Item* pItem = GetAItem(auction->itemGUIDLow);
    if (!pItem)
        return;

    uint32 bidder_accId = 0;
    ObjectGuid bidderGuid = ObjectGuid::Create<HighGuid::Player>(auction->bidder);
    Player* bidder = ObjectAccessor::FindPlayer(bidderGuid);
    // data for gm.log
    if (sWorld->getBoolConfig(CONFIG_GM_LOG_TRADE))
    {
        uint32 bidder_security = 0;
        std::string bidder_name;
        if (bidder)
        {
            bidder_accId = bidder->GetSession()->GetAccountId();
            bidder_security = bidder->GetSession()->GetSecurity();
            bidder_name = bidder->GetName();
        }
        else
        {
            bidder_accId = ObjectMgr::GetPlayerAccountIdByGUID(bidderGuid);
            bidder_security = AccountMgr::GetSecurity(bidder_accId, realm.Id.Realm);

            if (!AccountMgr::IsPlayerAccount(bidder_security)) // not do redundant DB requests
            {
                if (!ObjectMgr::GetPlayerNameByGUID(bidderGuid, bidder_name))
                    bidder_name = sObjectMgr->GetTrinityStringForDBCLocale(LANG_UNKNOWN);
            }
        }
        if (!AccountMgr::IsPlayerAccount(bidder_security))
        {
            std::string owner_name;
            ObjectGuid ownerGuid = ObjectGuid::Create<HighGuid::Player>(auction->owner);
            if (!ObjectMgr::GetPlayerNameByGUID(ownerGuid, owner_name))
                owner_name = sObjectMgr->GetTrinityStringForDBCLocale(LANG_UNKNOWN);

            uint32 owner_accid = ObjectMgr::GetPlayerAccountIdByGUID(ownerGuid);

            sLog->outCommand(bidder_accId, "GM %s (Account: %u) won item in auction: %s (Entry: %u Count: %u) and pay money: %u. Original owner %s (Account: %u)",
                bidder_name.c_str(), bidder_accId, pItem->GetTemplate()->GetName()->Str[bidder ? bidder->GetSession()->GetSessionDbLocaleIndex() : DEFAULT_LOCALE], pItem->GetEntry(), pItem->GetCount(), auction->bid, owner_name.c_str(), owner_accid);
        }
    }

    if(auction->bid >= sWorld->getIntConfig(CONFIG_LOG_GOLD_FROM))
    {
        ObjectGuid ownerGuid = ObjectGuid::Create<HighGuid::Player>(auction->owner);
        TC_LOG_DEBUG(LOG_FILTER_GOLD, "AuctionHouse: GUID %u won item in auction: %s (Entry: %u Count: %u) and pay money: " UI64FMTD ". Original ownerGuid %u",
            bidderGuid.GetGUIDLow(), pItem->GetTemplate()->GetName()->Str[bidder ? bidder->GetSession()->GetSessionDbLocaleIndex() : DEFAULT_LOCALE], pItem->GetEntry(), pItem->GetCount(), auction->bid, ownerGuid.GetGUIDLow());
    }

    // receiver exist
    if ((bidder || bidder_accId) && !sAuctionBotConfig->IsBotChar(auction->bidder))
    {
        // set owner to bidder (to prevent delete item with sender char deleting)
        // owner in `data` will set at mail receive and item extracting
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ITEM_OWNER);
        stmt->setUInt64(0, auction->bidder);
        stmt->setUInt64(1, pItem->GetGUIDLow());
        trans->Append(stmt);

        if (bidder)
        {
            bidder->GetSession()->SendAuctionWonNotification(auction, pItem);
            // FIXME: for offline player need also
            bidder->UpdateAchievementCriteria(CRITERIA_TYPE_WON_AUCTIONS, 1);
        }

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_WON), AuctionEntry::BuildAuctionMailBody(auction->owner, auction->bid, auction->buyout, 0, 0))
            .AddItem(pItem)
            .SendMailTo(trans, MailReceiver(bidder, auction->bidder), auction, MAIL_CHECK_MASK_COPIED);
    }
}

void AuctionHouseMgr::SendAuctionSalePendingMail(AuctionEntry* auction, SQLTransaction& trans)
{
    ObjectGuid owner_guid = ObjectGuid::Create<HighGuid::Player>(auction->owner);
    Player* owner = ObjectAccessor::FindPlayer(owner_guid);
    uint32 owner_accId = ObjectMgr::GetPlayerAccountIdByGUID(owner_guid);
    // owner exist (online or offline)
    if ((owner || owner_accId) && !sAuctionBotConfig->IsBotChar(auction->owner))
        MailDraft(auction->BuildAuctionMailSubject(AUCTION_SALE_PENDING), AuctionEntry::BuildAuctionMailBody(auction->bidder, auction->bid, auction->buyout, auction->deposit, auction->GetAuctionCut()))
            .SendMailTo(trans, MailReceiver(owner, auction->owner), auction, MAIL_CHECK_MASK_COPIED);
}

//call this method to send mail to auction owner, when auction is successful, it does not clear ram
void AuctionHouseMgr::SendAuctionSuccessfulMail(AuctionEntry* auction, SQLTransaction& trans)
{
    ObjectGuid owner_guid = ObjectGuid::Create<HighGuid::Player>(auction->owner);
    Player* owner = ObjectAccessor::FindPlayer(owner_guid);
    uint32 owner_accId = ObjectMgr::GetPlayerAccountIdByGUID(owner_guid);
    Item* item = GetAItem(auction->itemGUIDLow);
    if (!sAuctionBotConfig->IsBotChar(auction->owner) && item && (owner || owner_accId))
    {
        uint64 profit = auction->bid + auction->deposit - auction->GetAuctionCut();

        //FIXME: what do if owner offline
        if (owner)
        {
            owner->UpdateAchievementCriteria(CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS, profit);
            owner->UpdateAchievementCriteria(CRITERIA_TYPE_HIGHEST_AUCTION_SOLD, auction->bid);
            owner->GetSession()->SendAuctionClosedNotification(auction, static_cast<float>(sWorld->getIntConfig(CONFIG_MAIL_DELIVERY_DELAY)), true, item);
        }

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_SUCCESSFUL), AuctionEntry::BuildAuctionMailBody(auction->bidder, auction->bid, auction->buyout, auction->deposit, auction->GetAuctionCut()))
            .AddMoney(profit)
            .SendMailTo(trans, MailReceiver(owner, auction->owner), auction, MAIL_CHECK_MASK_COPIED, sWorld->getIntConfig(CONFIG_MAIL_DELIVERY_DELAY));
    }
}

//does not clear ram
void AuctionHouseMgr::SendAuctionExpiredMail(AuctionEntry* auction, SQLTransaction& trans)
{
    //return an item in auction to its owner by mail
    Item* pItem = GetAItem(auction->itemGUIDLow);
    if (!pItem)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "SendAuctionExpiredMail %u has not a existing item : %u", auction->Id, auction->itemGUIDLow);
        return;
    }

    ObjectGuid owner_guid = ObjectGuid::Create<HighGuid::Player>(auction->owner);
    Player* owner = ObjectAccessor::FindPlayer(owner_guid);
    uint32 owner_accId = ObjectMgr::GetPlayerAccountIdByGUID(owner_guid);
    // owner exist
    if ((owner || owner_accId) && !sAuctionBotConfig->IsBotChar(auction->owner))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ITEM_OWNER);
        stmt->setUInt64(0, auction->owner);
        stmt->setUInt64(1, pItem->GetGUIDLow());
        trans->Append(stmt);

        if (owner)
            owner->GetSession()->SendAuctionClosedNotification(auction, 0.0f, false, pItem);

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_EXPIRED), AuctionEntry::BuildAuctionMailBody(0, 0, auction->buyout, auction->deposit, 0))
            .AddItem(pItem)
            .SendMailTo(trans, MailReceiver(owner, auction->owner), auction, MAIL_CHECK_MASK_COPIED, 0);
    }
}

//this function sends mail to old bidder
void AuctionHouseMgr::SendAuctionOutbiddedMail(AuctionEntry* auction, uint64 const& /*newPrice*/, Player* /*newBidder*/, SQLTransaction& trans)
{
    ObjectGuid oldBidder_guid = ObjectGuid::Create<HighGuid::Player>(auction->bidder);
    Player* oldBidder = ObjectAccessor::FindPlayer(oldBidder_guid);
    Item* item = GetAItem(auction->itemGUIDLow);
    uint32 oldBidder_accId = 0;
    if (!oldBidder)
        oldBidder_accId = ObjectMgr::GetPlayerAccountIdByGUID(oldBidder_guid);

    if ((oldBidder || oldBidder_accId) && !sAuctionBotConfig->IsBotChar(auction->bidder))
    {
        if (oldBidder && item)
            oldBidder->GetSession()->SendAuctionOutBidNotification(auction, item);

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_OUTBIDDED), AuctionEntry::BuildAuctionMailBody(auction->owner, auction->bid, auction->buyout, auction->deposit, auction->GetAuctionCut()))
            .AddMoney(auction->bid)
            .SendMailTo(trans, MailReceiver(oldBidder, auction->bidder), auction, MAIL_CHECK_MASK_COPIED);
    }
}

//this function sends mail, when auction is cancelled to old bidder
void AuctionHouseMgr::SendAuctionCancelledToBidderMail(AuctionEntry* auction, SQLTransaction& trans, Item* item)
{
    ObjectGuid bidder_guid = ObjectGuid::Create<HighGuid::Player>(auction->bidder);
    Player* bidder = ObjectAccessor::FindPlayer(bidder_guid);

    uint32 bidder_accId = 0;
    if (!bidder)
        bidder_accId = ObjectMgr::GetPlayerAccountIdByGUID(bidder_guid);

    if (bidder)
    {
        bidder->GetSession()->SendAuctionOutBidNotification(auction, item);
        //bidder->GetSession()->SendAuctionRemovedNotification(auction->Id, auction->itemEntry, item->GetItemRandomPropertyId());
    }

    // bidder exist
    if ((bidder || bidder_accId) && !sAuctionBotConfig->IsBotChar(auction->bidder))
        MailDraft(auction->BuildAuctionMailSubject(AUCTION_CANCELLED_TO_BIDDER), AuctionEntry::BuildAuctionMailBody(auction->owner, auction->bid, auction->buyout, auction->deposit, 0))
            .AddMoney(auction->bid)
            .SendMailTo(trans, MailReceiver(bidder, auction->bidder), auction, MAIL_CHECK_MASK_COPIED);
}

void AuctionHouseMgr::LoadAuctionItems()
{
    uint32 oldMSTime = getMSTime();

    // data needs to be at first place for Item::LoadFromDB
    //          0           1               2           3       4       5       6           7                   8           9           10      11              12          13              14              15          16
    //SELECT creatorGuid, giftCreatorGuid, count, duration, charges, flags, enchantments, randomPropertyId, durability, playedTime, text, transmogrification, upgradeId, enchantIllusion, bonusListIDs, itemguid, itemEntry
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_AUCTION_ITEMS);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 auction items. DB table `auctionhouse` or `item_instance` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        ObjectGuid::LowType item_guid = fields[0].GetUInt64();
        uint32 itemEntry    = fields[1].GetUInt32();

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
        if (!proto)
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "AuctionHouseMgr::LoadAuctionItems: Unknown item (GUID: " UI64FMTD " id: #%u) in auction, skipped.", item_guid, itemEntry);
            continue;
        }

        Item* item = NewItemOrBag(proto);
        if (!item->LoadFromDB(item_guid, ObjectGuid::Empty, fields, itemEntry))
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "AuctionHouseMgr::LoadAuctionItems: Error item load (GUID: " UI64FMTD " id: #%u) in auction, skipped.", item_guid, itemEntry);
            delete item;
            continue;
        }
        AddAItem(item);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u auction items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void AuctionHouseMgr::LoadAuctions()
{
    uint32 oldMSTime = getMSTime();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_AUCTIONS);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 auctions. DB table `auctionhouse` is empty.");
        return;
    }

    uint32 count = 0;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    do
    {
        Field* fields = result->Fetch();

        auto aItem = new AuctionEntry();
        if (!aItem->LoadFromDB(fields))
        {
            aItem->DeleteFromDB(trans);
            delete aItem;
            continue;
        }

        GetAuctionsMap(aItem->factionTemplateId)->AddAuction(aItem);
        count++;
    } while (result->NextRow());

    CharacterDatabase.CommitTransaction(trans);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u auctions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void AuctionHouseMgr::AddAItem(Item* it)
{
    ASSERT(it);
    ASSERT(mAitems.find(it->GetGUIDLow()) == mAitems.end());
    mAitems[it->GetGUIDLow()] = it;
}

bool AuctionHouseMgr::RemoveAItem(ObjectGuid::LowType const& id)
{
    auto i = mAitems.find(id);
    if (i == mAitems.end())
        return false;

    mAitems.erase(i);
    return true;
}

void AuctionHouseMgr::Update()
{
    mHordeAuctions.Update();
    mAllianceAuctions.Update();
    mNeutralAuctions.Update();
}

AuctionHouseEntry const* AuctionHouseMgr::GetAuctionHouseEntry(uint32 factionTemplateId, uint32* houseId)
{
    uint32 houseid = 7; // goblin auction house

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        //FIXME: found way for proper auctionhouse selection by another way
        // AuctionHouse.dbc have faction field with _player_ factions associated with auction house races.
        // but no easy way convert creature faction to player race faction for specific city
        switch (factionTemplateId)
        {
            case   12: houseid = 1; break; // human
            case   29: houseid = 6; break; // orc, and generic for horde
            case   55: houseid = 2; break; // dwarf, and generic for alliance
            case   68: houseid = 4; break; // undead
            case   80: houseid = 3; break; // n-elf
            case  104: houseid = 5; break; // trolls
            case  120: houseid = 7; break; // booty bay, neutral
            case  474: houseid = 7; break; // gadgetzan, neutral
            case  855: houseid = 7; break; // everlook, neutral
            case 1604: houseid = 6; break; // b-elfs,
            default:                       // for unknown case
            {
                FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(factionTemplateId);
                if (!u_entry)
                    houseid = 7; // goblin auction house
                else if (u_entry->FactionGroup & FACTION_MASK_ALLIANCE)
                    houseid = 1; // human auction house
                else if (u_entry->FactionGroup & FACTION_MASK_HORDE)
                    houseid = 6; // orc auction house
                else
                    houseid = 7; // goblin auction house
                break;
            }
        }
    }

    if (houseId)
        *houseId = houseid;

    return sAuctionHouseStore.LookupEntry(houseid);
}

AuctionEntry* AuctionHouseObject::GetAuction(uint32 id) const
{
    return Trinity::Containers::MapGetValuePtr(AuctionsMap, id);
}

void AuctionHouseObject::AddAuction(AuctionEntry* auction)
{
    ASSERT(auction);

    AuctionsMap[auction->Id] = auction;
    sScriptMgr->OnAuctionAdd(this, auction);
}

bool AuctionHouseObject::RemoveAuction(AuctionEntry* auction, uint32 /*itemEntry*/)
{
    bool wasInMap = AuctionsMap.erase(auction->Id) != 0;

    sScriptMgr->OnAuctionRemove(this, auction);

    // we need to delete the entry, it is not referenced any more
    delete auction;
    return wasInMap;
}

void AuctionHouseObject::Update()
{
    time_t curTime = sWorld->GetGameTime();
    ///- Handle expired auctions

    // If storage is empty, no need to update. next == NULL in this case.
    if (AuctionsMap.empty())
        return;

    for (PlayerGetAllThrottleMap::const_iterator itr = GetAllThrottleMap.begin(); itr != GetAllThrottleMap.end();)
    {
        if (itr->second.NextAllowedReplication <= curTime)
            itr = GetAllThrottleMap.erase(itr);
        else
            ++itr;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_AUCTION_BY_TIME);
    stmt->setUInt32(0, static_cast<uint32>(curTime) + 60);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
        return;

    do
    {
        // from auctionhousehandler.cpp, creates auction pointer & player pointer
        AuctionEntry* auction = GetAuction(result->Fetch()->GetUInt32());
        if (!auction)
            continue;

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        ///- Either cancel the auction if there was no bidder
        if (auction->bidder == 0)
        {
            sAuctionMgr->SendAuctionExpiredMail(auction, trans);
            sScriptMgr->OnAuctionExpire(this, auction);
        }
        ///- Or perform the transaction
        else
        {
            //we should send an "item sold" message if the seller is online
            //we send the item to the winner
            //we send the money to the seller
            sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
            sAuctionMgr->SendAuctionWonMail(auction, trans);
            sScriptMgr->OnAuctionSuccessful(this, auction);
        }

        uint32 itemEntry = auction->itemEntry;

        ///- In any case clear the auction
        auction->DeleteFromDB(trans);
        CharacterDatabase.CommitTransaction(trans);

        RemoveAuction(auction, itemEntry);
        sAuctionMgr->RemoveAItem(auction->itemGUIDLow);
    }
    while (result->NextRow());
}

void AuctionHouseObject::BuildListBidderItems(WorldPackets::AuctionHouse::AuctionListBidderItemsResult& packet, Player* player, uint32& totalcount)
{
    for (auto const& v : AuctionsMap)
        if (AuctionEntry* Aentry = v.second)
            if (Aentry->bidder == player->GetGUIDLow())
            {
                Aentry->BuildAuctionInfo(packet.Items, false);
                ++totalcount;
            }
}

void AuctionHouseObject::BuildListOwnerItems(WorldPackets::AuctionHouse::AuctionListOwnerItemsResult& packet, Player* player, uint32& totalcount)
{
    for (auto const& v : AuctionsMap)
        if (AuctionEntry* Aentry = v.second)
            if (Aentry->owner == player->GetGUIDLow())
            {
                Aentry->BuildAuctionInfo(packet.Items, false);
                ++totalcount;
            }
}

void AuctionHouseObject::BuildListAuctionItems(WorldPackets::AuctionHouse::AuctionListItemsResult& packet, Player* player, std::wstring const& searchedname, uint32 listfrom, uint8 levelmin, uint8 levelmax, bool usable, Optional<AuctionSearchFilters> const& filters, uint32 quality)
{
    LocaleConstant localeConstant = player->GetSession()->GetSessionDbLocaleIndex();
    time_t curTime = sWorld->GetGameTime();

    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin(); itr != AuctionsMap.end(); ++itr)
    {
        AuctionEntry* Aentry = itr->second;
        if (!Aentry)
            continue;

        if (Aentry->expire_time < curTime)
            continue;

        Item* item = sAuctionMgr->GetAItem(Aentry->itemGUIDLow);
        if (!item)
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        if (filters)
        {
            if (filters->Classes[proto->GetClass()].SubclassMask == AuctionSearchFilters::FILTER_SKIP_CLASS)
                continue;

            if (filters->Classes[proto->GetClass()].SubclassMask != AuctionSearchFilters::FILTER_SKIP_SUBCLASS)
            {
                if (!(filters->Classes[proto->GetClass()].SubclassMask & (1 << proto->GetSubClass())))
                    continue;

                if (!(filters->Classes[proto->GetClass()].InvTypes[proto->GetSubClass()] & (1 << proto->GetInventoryType())))
                    continue;
            }
        }

        if (quality != 0xffffffff && item->GetQuality() != quality)
            continue;

        if (levelmin != 0 && (item->GetRequiredLevel() < levelmin || (levelmax != 0 && item->GetRequiredLevel() > levelmax)))
            continue;

        if (usable && player->CanUseItem(item) != EQUIP_ERR_OK)
            continue;

        // Allow search by suffix (ie: of the Monkey) or partial name (ie: Monkey)
        // No need to do any of this if no search term was entered
        if (!searchedname.empty())
        {
            std::string name = proto->GetName()->Str[localeConstant];
            if (name.empty())
                continue;

            // DO NOT use GetItemEnchantMod(proto->GetRandomSelect()) as it may return a result
            //  that matches the search but it may not equal item->GetItemRandomPropertyId()
            //  used in BuildAuctionInfo() which then causes wrong items to be listed
            if (int32 propRefID = item->GetItemRandomPropertyId())
            {
                char const* suffix = nullptr;
                if (propRefID < 0)
                {
                    if (ItemRandomSuffixEntry const* itemRandSuffix = sItemRandomSuffixStore.LookupEntry(-propRefID))
                        suffix = itemRandSuffix->Name->Str[localeConstant];
                }
                else if (ItemRandomPropertiesEntry const* itemRandProp = sItemRandomPropertiesStore.LookupEntry(propRefID))
                    suffix = itemRandProp->Name->Str[localeConstant];

                // dbc local name
                if (suffix)
                {
                    name += ' ';
                    name += suffix;
                }
            }

            // Perform the search (with or without suffix)
            if (!Utf8FitTo(name, searchedname))
                continue;
        }

        // Add the item if no search term or if entered search term was found
        if (packet.Items.size() < 50 && packet.TotalCount >= listfrom)
            Aentry->BuildAuctionInfo(packet.Items, true, item);

        ++packet.TotalCount;
    }
}

void AuctionHouseObject::BuildReplicate(WorldPackets::AuctionHouse::AuctionReplicateResponse& auctionReplicateResult, Player* player, uint32 global, uint32 cursor, uint32 tombstone, uint32 count)
{
    time_t curTime = sWorld->GetGameTime();

    auto throttleItr = GetAllThrottleMap.find(player->GetGUID());
    if (throttleItr != GetAllThrottleMap.end())
    {
        if (throttleItr->second.Global != global || throttleItr->second.Cursor != cursor || throttleItr->second.Tombstone != tombstone)
            return;

        if (!throttleItr->second.IsReplicationInProgress() && throttleItr->second.NextAllowedReplication > curTime)
            return;
    }
    else
    {
        throttleItr = GetAllThrottleMap.insert({player->GetGUID(), PlayerGetAllThrottleData{ }}).first;
        throttleItr->second.NextAllowedReplication = curTime + 900;
        throttleItr->second.Global = uint32(curTime);
    }

    if (AuctionsMap.empty() || !count)
        return;

    auto itr = AuctionsMap.upper_bound(cursor);
    for (; itr != AuctionsMap.end(); ++itr)
    {
         AuctionEntry* auction = itr->second;
        if (!auction)
            continue;

        if (auction->expire_time < curTime)
            continue;

        Item* item = sAuctionMgr->GetAItem(auction->itemGUIDLow);
        if (!item)
            continue;

        auction->BuildAuctionInfo(auctionReplicateResult.Items, true, item);
        if (!--count)
            break;
    }

    auctionReplicateResult.ChangeNumberGlobal = throttleItr->second.Global;
    auctionReplicateResult.ChangeNumberCursor = throttleItr->second.Cursor = !auctionReplicateResult.Items.empty() ? auctionReplicateResult.Items.back().AuctionItemID : 0;
    auctionReplicateResult.ChangeNumberTombstone = throttleItr->second.Tombstone = !count ? AuctionsMap.rbegin()->first : 0;
}

void AuctionEntry::BuildAuctionInfo(std::vector<WorldPackets::AuctionHouse::AuctionItem>& items, bool listAuctionItems, Item* sourceItem /*= nullptr*/) const
{
    Item* item = sourceItem ? sourceItem : sAuctionMgr->GetAItem(itemGUIDLow);
    if (!item)
        return;

    WorldPackets::AuctionHouse::AuctionItem auctionItem;
    auctionItem.AuctionItemID = Id;
    auctionItem.Item.Initialize(item);
    auctionItem.BuyoutPrice = buyout;
    auctionItem.CensorBidInfo = false;
    auctionItem.CensorServerSideInfo = listAuctionItems;
    auctionItem.Charges = item->GetSpellCharges();
    auctionItem.Count = item->GetCount();
    auctionItem.DeleteReason = 0; // Always 0 ?
    auctionItem.DurationLeft = (expire_time - time(nullptr)) * IN_MILLISECONDS;
    auctionItem.EndTime = expire_time;
    auctionItem.Flags = 0; // todo
    auctionItem.ItemGuid = item->GetGUID();
    auctionItem.MinBid = startbid;
    auctionItem.Owner = ObjectGuid::Create<HighGuid::Player>(owner);
    auctionItem.OwnerAccountID = ObjectGuid::Create<HighGuid::WowAccount>(ObjectMgr::GetPlayerAccountIdByGUID(auctionItem.Owner));
    if (bidder)
    {
        auctionItem.MinIncrement = GetAuctionOutBid();
        auctionItem.Bidder = ObjectGuid::Create<HighGuid::Player>(bidder);
        auctionItem.BidAmount = bid;
    }

    for (uint8 i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; i++)
        if (uint32 enchantmentID = item->GetEnchantmentId(EnchantmentSlot(i)))
            auctionItem.Enchantments.emplace_back(enchantmentID, item->GetEnchantmentDuration(EnchantmentSlot(i)), item->GetEnchantmentCharges(EnchantmentSlot(i)), i);

    uint8 i = 0;
    for (ItemDynamicFieldGems const& gemData : item->GetGems())
    {
        if (gemData.ItemId)
        {
            WorldPackets::Item::ItemGemData gem;
            gem.Slot = i;
            gem.Item.Initialize(&gemData);
            auctionItem.Gems.push_back(gem);
        }
        ++i;
    }

    items.emplace_back(auctionItem);
}

uint32 AuctionEntry::GetHouseId() const
{
    return houseId;
}

uint32 AuctionEntry::GetHouseFaction() const
{
    return auctionHouseEntry->FactionID;
}

uint64 AuctionEntry::GetAuctionCut() const
{
    auto cut = int64(CalculatePct(bid, auctionHouseEntry->ConsignmentRate) * sWorld->getRate(RATE_AUCTION_CUT));
    return cut > 0 ? cut : 0;
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint64 AuctionEntry::GetAuctionOutBid() const
{
    uint64 outbid = CalculatePct(bid, 5);
    return outbid ? outbid : 1;
}

void AuctionEntry::DeleteFromDB(SQLTransaction& trans) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_AUCTION);
    stmt->setUInt32(0, Id);
    trans->Append(stmt);
}

void AuctionEntry::SaveToDB(SQLTransaction& trans) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_AUCTION);
    stmt->setUInt32(0, Id);
    stmt->setUInt64(1, auctioneer);
    stmt->setUInt64(2, itemGUIDLow);
    stmt->setUInt64(3, owner);
    stmt->setInt64 (4, int64(buyout));
    stmt->setUInt64(5, int64(expire_time));
    stmt->setUInt64(6, bidder);
    stmt->setInt64 (7, int64(bid));
    stmt->setInt64 (8, int64(startbid));
    stmt->setInt64 (9, int64(deposit));
    trans->Append(stmt);
}

bool AuctionEntry::LoadFromDB(Field* fields)
{
    uint8 index = 0;

    Id          = fields[index++].GetUInt32();
    auctioneer  = uint32(fields[index++].GetUInt64());
    itemGUIDLow = fields[index++].GetUInt64();
    itemEntry   = fields[index++].GetUInt32();
    itemCount   = fields[index++].GetUInt32();
    owner       = fields[index++].GetUInt64();
    buyout      = fields[index++].GetUInt64();
    expire_time = fields[index++].GetUInt32();
    bidder      = fields[index++].GetUInt64();
    bid         = fields[index++].GetUInt64();
    startbid    = fields[index++].GetUInt64();
    deposit     = fields[index++].GetUInt64();

    factionTemplateId = 474; // if auctioneer is buged set neutrall

    CreatureTemplate const* auctioneerInfo = sObjectMgr->GetCreatureTemplate(auctioneer);
    if (!auctioneerInfo)
        TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "Auction %u has not a existing auctioneer Entry: %u", Id, auctioneer);
    else
        factionTemplateId = auctioneerInfo->faction;

    auctionHouseEntry = AuctionHouseMgr::GetAuctionHouseEntry(factionTemplateId, &houseId);
    if (!auctionHouseEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "Auction %u has auctioneer Entry: %u with wrong faction %u", Id, auctioneer, factionTemplateId);
        return false;
    }

    // check if sold item exists for guid
    // and itemEntry in fact (GetAItem will fail if problematic in result check in AuctionHouseMgr::LoadAuctionItems)
    if (!sAuctionMgr->GetAItem(itemGUIDLow))
    {
        TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "Auction %u has not a existing item : %u", Id, itemGUIDLow);
        return false;
    }
    return true;
}

bool AuctionEntry::LoadFromFieldList(Field* fields)
{
    // Loads an AuctionEntry item from a field list. Unlike "LoadFromDB()", this one
    //  does not require the AuctionEntryMap to have been loaded with items. It simply
    //  acts as a wrapper to fill out an AuctionEntry struct from a field list

    uint8 index = 0;

    Id          = fields[index++].GetUInt32();
    auctioneer  = uint32(fields[index++].GetUInt64());
    itemGUIDLow = fields[index++].GetUInt64();
    itemEntry   = fields[index++].GetUInt32();
    itemCount   = fields[index++].GetUInt32();
    owner       = fields[index++].GetUInt64();
    buyout      = fields[index++].GetUInt64();
    expire_time = fields[index++].GetUInt32();
    bidder      = fields[index++].GetUInt64();
    bid         = fields[index++].GetUInt64();
    startbid    = fields[index++].GetUInt64();
    deposit     = fields[index++].GetUInt64();

    factionTemplateId = 474; // if auctioneer is buged set neutrall

    CreatureTemplate const* auctioneerInfo = sObjectMgr->GetCreatureTemplate(auctioneer);
    if (!auctioneerInfo)
        TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "AuctionEntry::LoadFromFieldList() - Auction %u has not a existing auctioneer Entry: %u", Id, auctioneer);
    else
        factionTemplateId = auctioneerInfo->faction;

    auctionHouseEntry = AuctionHouseMgr::GetAuctionHouseEntry(factionTemplateId, &houseId);

    if (!auctionHouseEntry)
    {
		TC_LOG_DEBUG(LOG_FILTER_SERVER_LOADING, "AuctionEntry::LoadFromFieldList() - Auction %u has auctioneer Entry: %u with wrong faction %u", Id, auctioneer, factionTemplateId);
        return false;
    }

    return true;
}

std::string AuctionEntry::BuildAuctionMailSubject(MailAuctionAnswers response) const
{
    std::ostringstream strm;
    strm << itemEntry << ":0:" << response << ':' << Id << ':' << itemCount;
    return strm.str();
}

std::string AuctionEntry::BuildAuctionMailBody(uint32 lowGuid, uint64 bid, uint64 buyout, uint64 deposit, uint64 cut)
{
    std::ostringstream strm;
    strm.width(16);
    strm << std::right << std::hex << ObjectGuid::Create<HighGuid::Player>(lowGuid);   // HighGuid::Player always present, even for empty guids
    strm << std::dec << ':' << bid << ':' << buyout;
    strm << ':' << deposit << ':' << cut;
    return strm.str();
}

AuctionHouseObject::AuctionHouseObject()
{
    next = AuctionsMap.begin();
}

AuctionHouseObject::~AuctionHouseObject()
{
    for (auto& itr : AuctionsMap)
        delete itr.second;
}

bool AuctionHouseObject::PlayerGetAllThrottleData::IsReplicationInProgress() const
{
    return Cursor != Tombstone && Global != 0;
}

uint32 AuctionHouseObject::Getcount() const
{
    return AuctionsMap.size();
}
