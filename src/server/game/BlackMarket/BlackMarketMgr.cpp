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

#include "BlackMarketMgr.h"
#include "BlackMarketPackets.h"
#include "DatabaseEnv.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "World.h"

BlackMarketMgr::BlackMarketMgr(): _LastUpdate(0) {}

BlackMarketMgr::~BlackMarketMgr()
{
    for (auto& itr : mAuctions)
        delete itr.second;

    for (auto& itr : mTemplates)
        delete itr.second;
}

BlackMarketMgr* BlackMarketMgr::instance()
{
    static BlackMarketMgr instance;
    return &instance;
}

void BlackMarketMgr::LoadTemplates()
{
    uint32 oldMSTime = getMSTime();

    if (!mTemplates.empty())
    {
        for (auto& itr : mTemplates)
            delete itr.second;

        mTemplates.clear();
    }

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_BLACKMARKET_TEMPLATE);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
        return;

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        auto templ = new BlackMarketTemplate;

        if (!templ->LoadFromDB(fields))
        {
            delete templ;
            continue;
        }

        AddTemplate(templ);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u black market templates in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void BlackMarketMgr::LoadAuctions()
{
    uint32 oldMSTime = getMSTime();

    if (!mAuctions.empty())
    {
        for (auto& itr : mAuctions)
            delete itr.second;

        mAuctions.clear();
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_BLACKMARKET_AUCTIONS);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 black market auctions. DB table `blackmarket_auctions` is empty.");
        return;
    }

    uint32 count = 0;
    _LastUpdate = time(nullptr);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    do
    {
        Field* fields = result->Fetch();
        auto auction = new BlackMarketEntry;

        if (!auction->LoadFromDB(fields))
        {
            auction->DeleteFromDB(trans);
            delete auction;
            continue;
        }

        AddAuction(auction);
        ++count;
    }
    while (result->NextRow());

    CharacterDatabase.CommitTransaction(trans);

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u black market auctions in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void BlackMarketMgr::Update(bool updateTime)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    time_t now = time(nullptr);
    for (auto itr = mAuctions.begin(); itr != mAuctions.end();)
    {
        BlackMarketEntry* auction = itr->second;

        if (!auction->IsCompleted())
        {
            ++itr;
            continue;
        }

        if (auction->GetBidder())
            SendAuctionWonMail(auction, trans);

        auction->DeleteFromDB(trans);
        itr = mAuctions.erase(itr);
        delete auction;
    }

    if (updateTime)
        _LastUpdate = now;

    CharacterDatabase.CommitTransaction(trans);
}

void BlackMarketMgr::RefreshAuctions()
{
    int32 const auctionDiff = sWorld->getIntConfig(CONFIG_BLACKMARKET_MAXAUCTIONS) - mAuctions.size();

    // We are already at max auctions, do nothing
    if (auctionDiff <= 0)
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    std::list<BlackMarketTemplate const*> templates;
    for (auto const& pair : mTemplates)
    {
        if (GetAuctionByID(pair.second->MarketID))
            continue;

        if (!roll_chance_f(pair.second->Chance))
            continue;

        templates.push_back(pair.second);
    }

    Trinity::Containers::RandomResizeList(templates, auctionDiff);

    for (BlackMarketTemplate const* templat : templates)
    {
        auto* entry = new BlackMarketEntry();
        entry->Initialize(templat->MarketID);
        entry->SaveToDB(trans);
        AddAuction(entry);
    }

    CharacterDatabase.CommitTransaction(trans);

    Update(true);
}

time_t BlackMarketMgr::GetLastUpdate() const
{
    return _LastUpdate;
}

bool BlackMarketMgr::IsEnabled() const
{
    return sWorld->getBoolConfig(CONFIG_BLACKMARKET_ENABLED);
}

void BlackMarketMgr::BuildItemsResponse(WorldPackets::BlackMarket::BlackMarketRequestItemsResult& packet, Player* player)
{
    packet.LastUpdateID = _LastUpdate;
    packet.Items.reserve(mAuctions.size());
    for (auto const& mAuction : mAuctions)
    {
        BlackMarketTemplate const* templ = mAuction.second->GetTemplate();

        WorldPackets::BlackMarket::BlackMarketItem item;

        item.MarketID = mAuction.second->GetMarketId();
        item.SellerNPC = templ->SellerNPC;
        item.Item = templ->Item;
        item.Quantity = templ->Quantity;

        // No bids yet
        if (!mAuction.second->GetNumBids())
        {
            item.MinBid = templ->MinBid;
            item.MinIncrement = 1;
        }
        else
        {
            item.MinIncrement = mAuction.second->GetMinIncrement(); // 5% increment minimum
            item.MinBid = mAuction.second->GetCurrentBid() + item.MinIncrement;
        }

        item.CurrentBid = mAuction.second->GetCurrentBid();
        item.SecondsRemaining = mAuction.second->GetSecondsRemaining();
        item.HighBid = (mAuction.second->GetBidder() == player->GetGUID().GetCounter());
        item.NumBids = mAuction.second->GetNumBids();

        packet.Items.push_back(item);
    }
}

void BlackMarketMgr::AddAuction(BlackMarketEntry* auction)
{
    mAuctions.insert(std::make_pair(auction->GetMarketId(), auction));
}

void BlackMarketMgr::AddTemplate(BlackMarketTemplate* templ)
{
    mTemplates.insert(std::make_pair(templ->MarketID, templ));
}

void BlackMarketMgr::SendAuctionWonMail(BlackMarketEntry* entry, SQLTransaction& trans)
{
    // Mail already sent
    if (entry->GetMailSent())
        return;

    ObjectGuid bidderGuid = ObjectGuid::Create<HighGuid::Player>(entry->GetBidder());
    Player* bidder = ObjectAccessor::FindPlayer(bidderGuid);

    BlackMarketTemplate const* templ = entry->GetTemplate();
    Item* item = Item::CreateItem(templ->Item.ItemID, templ->Quantity);
    if (!item)
        return;

    if (templ->Item.ItemBonus)
        for (uint32 bonusList : templ->Item.ItemBonus->BonusListIDs)
            item->AddBonuses(bonusList);

    item->SetOwnerGUID(bidderGuid);
    item->SaveToDB(trans);

    if (bidder)
        bidder->GetSession()->SendBlackMarketWonNotification(entry, item);

    MailDraft(entry->BuildAuctionMailSubject(BMAH_AUCTION_WON), entry->BuildAuctionMailBody())
        .AddItem(item)
        .SendMailTo(trans, MailReceiver(bidder, entry->GetBidder()), entry, MAIL_CHECK_MASK_COPIED);

    entry->MailSent();

    TC_LOG_DEBUG(LOG_FILTER_GOLD, "Player %s (GUID: %u) Won Item: %u in Black Market on cost " UI64FMTD "", bidder->GetName(), bidder->GetGUIDLow(), item->GetEntry(), entry->GetCurrentBid());
}

void BlackMarketMgr::SendAuctionOutbidMail(BlackMarketEntry* entry, SQLTransaction& trans)
{
    ObjectGuid oldBidderGuid = ObjectGuid::Create<HighGuid::Player>(entry->GetBidder());
    Player* oldBidder = ObjectAccessor::FindPlayer(oldBidderGuid);

    uint32 oldBidderAccID = 0;
    if (!oldBidder)
        oldBidderAccID = ObjectMgr::GetPlayerAccountIdByGUID(oldBidderGuid);

    if (!oldBidder && !oldBidderAccID)
        return;

    if (oldBidder)
        oldBidder->GetSession()->SendBlackMarketOutbidNotification(entry->GetTemplate());

    MailDraft(entry->BuildAuctionMailSubject(BMAH_AUCTION_OUTBID), entry->BuildAuctionMailBody())
        .AddMoney(entry->GetCurrentBid())
        .SendMailTo(trans, MailReceiver(oldBidder, entry->GetBidder()), entry, MAIL_CHECK_MASK_COPIED);
}

BlackMarketEntry* BlackMarketMgr::GetAuctionByID(int32 marketId) const
{
    return Trinity::Containers::MapGetValuePtr(mAuctions, marketId);
}

BlackMarketTemplate* BlackMarketMgr::GetTemplateByID(int32 marketId) const
{
    return Trinity::Containers::MapGetValuePtr(mTemplates, marketId);
}

bool BlackMarketTemplate::LoadFromDB(Field* fields)
{
    MarketID = fields[0].GetInt32();
    SellerNPC = fields[1].GetInt32();
    Item.ItemID = fields[2].GetUInt32();
    Quantity = fields[3].GetInt32();
    MinBid = fields[4].GetUInt64();
    Duration = static_cast<time_t>(fields[5].GetUInt32());
    Chance = fields[6].GetFloat();

    Tokenizer bonusListIDsTok(fields[7].GetString(), ' ');
    std::vector<uint32> bonusListIDs;

    for (char const* token : bonusListIDsTok)
        bonusListIDs.push_back(atol(token));

    if (!bonusListIDs.empty())
    {
        Item.ItemBonus = boost::in_place();
        Item.ItemBonus->BonusListIDs = bonusListIDs;
    }

    if (!sObjectMgr->GetCreatureTemplate(SellerNPC))
        return false;

    if (!sObjectMgr->GetItemTemplate(Item.ItemID))
        return false;

    return true;
}

void BlackMarketEntry::Initialize(int32 marketId)
{
    _marketId = marketId;
    _startTime = time(nullptr);
}

BlackMarketTemplate* BlackMarketEntry::GetTemplate() const
{
    return sBlackMarketMgr->GetTemplateByID(GetMarketId());
}

int32 BlackMarketEntry::GetMarketId() const
{
    return _marketId;
}

uint64 BlackMarketEntry::GetCurrentBid() const
{
    return _currentBid;
}

void BlackMarketEntry::SetCurrentBid(uint32 bid)
{
    _currentBid = bid;
}

int32 BlackMarketEntry::GetNumBids() const
{
    return _numBids;
}

void BlackMarketEntry::SetNumBids(int32 numBids)
{
    _numBids = numBids;
}

ObjectGuid::LowType BlackMarketEntry::GetBidder() const
{
    return _bidder;
}

void BlackMarketEntry::SetBidder(ObjectGuid::LowType bidder)
{
    _bidder = bidder;
}

time_t BlackMarketEntry::GetSecondsRemaining() const
{
    return std::max(0, int32(GetExpirationTime()) - int32(time(nullptr)));
}

time_t BlackMarketEntry::GetExpirationTime() const
{
    return _startTime + GetTemplate()->Duration;
}

bool BlackMarketEntry::IsCompleted() const
{
    return GetSecondsRemaining() <= 0;
}

bool BlackMarketEntry::LoadFromDB(Field* fields)
{
    _marketId = fields[0].GetInt32();

    BlackMarketTemplate* templ = sBlackMarketMgr->GetTemplateByID(_marketId);
    if (!templ)
        return false;

    _currentBid = fields[1].GetUInt64();
    _startTime = static_cast<time_t>(fields[2].GetInt32());
    _numBids = fields[3].GetInt32();
    _bidder = fields[4].GetUInt64();

    return true;
}

uint64 BlackMarketEntry::GetMinIncrement() const
{
    return (_currentBid / 20) - ((_currentBid / 20) % GOLD);
}

void BlackMarketEntry::SaveToDB(SQLTransaction& trans) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_BLACKMARKET_AUCTIONS);

    stmt->setInt32(0, _marketId);
    stmt->setUInt64(1, _currentBid);
    stmt->setInt32(2, GetExpirationTime());
    stmt->setInt32(3, _numBids);
    stmt->setUInt64(4, _bidder);

    trans->Append(stmt);
}

void BlackMarketEntry::DeleteFromDB(SQLTransaction& trans) const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_BLACKMARKET_AUCTIONS);
    stmt->setInt32(0, _marketId);
    trans->Append(stmt);
}

bool BlackMarketEntry::ValidateBid(uint64 bid) const
{
    if (bid <= _currentBid)
        return false;

    if (bid < _currentBid + GetMinIncrement())
        return false;

    if (bid >= BMAH_MAX_BID)
        return false;

    return true;
}

void BlackMarketEntry::PlaceBid(uint64 bid, Player* player, SQLTransaction& trans)
{
    if (bid < _currentBid)
        return;

    _currentBid = bid;
    ++_numBids;

    _bidder = player->GetGUID().GetCounter();

    player->ModifyMoney(-static_cast<int64>(bid));

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_BLACKMARKET_AUCTIONS);

    stmt->setUInt64(0, _currentBid);
    stmt->setInt32(1, GetExpirationTime());
    stmt->setInt32(2, _numBids);
    stmt->setUInt64(3, _bidder);
    stmt->setInt32(4, _marketId);

    trans->Append(stmt);

    TC_LOG_DEBUG(LOG_FILTER_GOLD, "Player %s (GUID: %u) Place Bid (Item: %u) in Black Market on cost " UI64FMTD "", player->GetName(), player->GetGUIDLow(), GetTemplate()->Item.ItemID, _currentBid);

    sBlackMarketMgr->Update(true);
}

std::string BlackMarketEntry::BuildAuctionMailSubject(BMAHMailAuctionAnswers response) const
{
    std::ostringstream strm;
    strm << GetTemplate()->Item.ItemID << ":0:" << response << ':' << GetMarketId() << ':' << GetTemplate()->Quantity;
    return strm.str();
}

std::string BlackMarketEntry::BuildAuctionMailBody()
{
    std::ostringstream strm;
    strm << GetTemplate()->SellerNPC << ':' << _currentBid;

    return strm.str();
}

void BlackMarketEntry::MailSent()
{
    _mailSent = true;
}

bool BlackMarketEntry::GetMailSent() const
{
    return _mailSent;
}
