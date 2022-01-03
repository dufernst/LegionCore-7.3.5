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

#include "GuildPackets.h"
#include "WowTime.hpp"

void WorldPackets::Guild::QueryGuildInfo::Read()
{
    _worldPacket >> GuildGuid;
    _worldPacket >> PlayerGuid;
}

WorldPackets::Guild::QueryGuildInfoResponse::GuildInfo::GuildInfoRank::GuildInfoRank(uint32 id, uint32 order, std::string const& name): RankID(id), RankOrder(order), RankName(name)
{
}

bool WorldPackets::Guild::QueryGuildInfoResponse::GuildInfo::GuildInfoRank::operator<(GuildInfoRank const& right) const
{
    return RankID < right.RankID;
}

WorldPacket const* WorldPackets::Guild::QueryGuildInfoResponse::Write()
{
    _worldPacket << GuildGuid;
    _worldPacket.WriteBit(Info.is_initialized());
    _worldPacket.FlushBits();

    if (Info)
    {
        _worldPacket << Info->GuildGUID;
        _worldPacket << uint32(Info->VirtualRealmAddress);
        _worldPacket << uint32(Info->Ranks.size());
        _worldPacket << Info->EmblemStyle;
        _worldPacket << Info->EmblemColor;
        _worldPacket << Info->BorderStyle;
        _worldPacket << Info->BorderColor;
        _worldPacket << Info->BackgroundColor;
        _worldPacket.WriteBits(Info->GuildName.size(), 7);
        _worldPacket.FlushBits();

        for (GuildInfo::GuildInfoRank const& rank : Info->Ranks)
        {
            _worldPacket << uint32(rank.RankID);
            _worldPacket << uint32(rank.RankOrder);

            _worldPacket.WriteBits(rank.RankName.size(), 7);
            _worldPacket.WriteString(rank.RankName);
        }

        _worldPacket.WriteString(Info->GuildName);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildCommandResult::Write()
{
    _worldPacket << Result;
    _worldPacket << Command;

    _worldPacket.WriteBits(Name.length(), 8);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildRoster::Write()
{
    _worldPacket << NumAccounts;
    _worldPacket << MS::Utilities::WowTime::Encode(CreateDate);
    _worldPacket << GuildFlags;
    _worldPacket << uint32(MemberData.size());
    _worldPacket.WriteBits(WelcomeText.length(), 10);
    _worldPacket.WriteBits(InfoText.length(), 11);
    _worldPacket.FlushBits();

    for (GuildRosterMemberData const& member : MemberData)
        _worldPacket << member;

    _worldPacket.WriteString(WelcomeText);
    _worldPacket.WriteString(InfoText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildRosterUpdate::Write()
{
    _worldPacket << uint32(MemberData.size());

    for (GuildRosterMemberData const& member : MemberData)
        _worldPacket << member;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildUpdateMotdText::Read()
{
    uint32 textLen = _worldPacket.ReadBits(10);
    MotdText = _worldPacket.ReadString(textLen);
}

void WorldPackets::Guild::GuildUpdateInfoText::Read()
{
    uint32 textLen = _worldPacket.ReadBits(11);
    InfoText = _worldPacket.ReadString(textLen);
}

void WorldPackets::Guild::GuildSetMemberNote::Read()
{
    _worldPacket >> NoteeGUID;

    uint32 noteLen = _worldPacket.ReadBits(8);
    IsPublic = _worldPacket.ReadBit();

    Note = _worldPacket.ReadString(noteLen);
}

WorldPacket const* WorldPackets::Guild::GuildMemberUpdateNote::Write()
{
    _worldPacket.reserve(16 + 2 + Note.size());

    _worldPacket << Member;

    _worldPacket.WriteBits(Note.length(), 8);
    _worldPacket.WriteBit(IsPublic);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Note);

    return &_worldPacket;
}

void WorldPackets::Guild::GuildDemoteMember::Read()
{
    _worldPacket >> Demotee;
}

void WorldPackets::Guild::GuildPromoteMember::Read()
{
    _worldPacket >> Promotee;
}

void WorldPackets::Guild::GuildOfficerRemoveMember::Read()
{
    _worldPacket >> Removee;
}

void WorldPackets::Guild::AutoDeclineGuildInvites::Read()
{
    Allow = _worldPacket.ReadBit();
}

void WorldPackets::Guild::GuildInviteByName::Read()
{
    uint32 nameLen = _worldPacket.ReadBits(9);
    Name = _worldPacket.ReadString(nameLen);
}

WorldPacket const* WorldPackets::Guild::GuildInvite::Write()
{
    _worldPacket.WriteBits(InviterName.length(), 6);
    _worldPacket.WriteBits(GuildName.length(), 7);
    _worldPacket.WriteBits(OldGuildName.length(), 7);
    _worldPacket.FlushBits();

    _worldPacket << InviterVirtualRealmAddress;
    _worldPacket << GuildVirtualRealmAddress;
    _worldPacket << GuildGUID;
    _worldPacket << OldGuildVirtualRealmAddress;
    _worldPacket << OldGuildGUID;
    _worldPacket << EmblemStyle;
    _worldPacket << EmblemColor;
    _worldPacket << BorderStyle;
    _worldPacket << BorderColor;
    _worldPacket << Background;
    _worldPacket << AchievementPoints;

    _worldPacket.WriteString(InviterName);
    _worldPacket.WriteString(GuildName);
    _worldPacket.WriteString(OldGuildName);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterProfessionData const& rosterProfessionData)
{
    data << rosterProfessionData.DbID;
    data << rosterProfessionData.Rank;
    data << rosterProfessionData.Step;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRosterMemberData const& rosterMemberData)
{
    data << rosterMemberData.Guid;
    data << rosterMemberData.RankID;
    data << rosterMemberData.AreaID;
    data << rosterMemberData.PersonalAchievementPoints;
    data << rosterMemberData.GuildReputation;
    data << rosterMemberData.LastSave;

    for (uint8 i = 0; i < 2; i++)
        data << rosterMemberData.ProfessionData[i];

    data << rosterMemberData.VirtualRealmAddress;
    data << rosterMemberData.Status;
    data << rosterMemberData.Level;
    data << rosterMemberData.ClassID;
    data << rosterMemberData.Gender;

    data.WriteBits(rosterMemberData.Name.length(), 6);
    data.WriteBits(rosterMemberData.Note.length(), 8);
    data.WriteBits(rosterMemberData.OfficerNote.length(), 8);
    data.WriteBit(rosterMemberData.Authenticated);
    data.WriteBit(rosterMemberData.SorEligible);
    data.FlushBits();

    data.WriteString(rosterMemberData.Name);
    data.WriteString(rosterMemberData.Note);
    data.WriteString(rosterMemberData.OfficerNote);

    return data;
}

WorldPacket const* WorldPackets::Guild::GuildEventPresenceChange::Write()
{
    _worldPacket << Guid;
    _worldPacket << VirtualRealmAddress;

    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.WriteBit(LoggedOn);
    _worldPacket.WriteBit(Mobile);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventMotd::Write()
{
    _worldPacket.WriteBits(MotdText.length(), 10);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(MotdText);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventPlayerJoined::Write()
{
    _worldPacket << Guid;
    _worldPacket << VirtualRealmAddress;

    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventRankChanged::Write()
{
    _worldPacket << RankID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventBankMoneyChanged::Write()
{
    _worldPacket << Money;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventPlayerLeft::Write()
{
    _worldPacket.WriteBit(Removed);
    _worldPacket.WriteBits(LeaverName.length(), 6);
    _worldPacket.FlushBits();

    if (Removed)
    {
        _worldPacket.WriteBits(RemoverName.length(), 6);
        _worldPacket.FlushBits();

        _worldPacket << RemoverGUID;
        _worldPacket << RemoverVirtualRealmAddress;
        _worldPacket.WriteString(RemoverName);
    }

    _worldPacket << LeaverGUID;
    _worldPacket << LeaverVirtualRealmAddress;
    _worldPacket.WriteString(LeaverName);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildPermissionsQueryResults::Write()
{
    _worldPacket << RankID;
    _worldPacket << WithdrawGoldLimit;
    _worldPacket << Flags;
    _worldPacket << NumTabs;
    _worldPacket << uint32(Tab.size());

    for (GuildRankTabPermissions const& tab : Tab)
    {
        _worldPacket << tab.Flags;
        _worldPacket << tab.WithdrawItemLimit;
    }

    return &_worldPacket;
}

void WorldPackets::Guild::GuildSetRankPermissions::Read()
{
    _worldPacket >> RankID;
    _worldPacket >> RankOrder;
    _worldPacket >> Flags;
    _worldPacket >> OldFlags;
    _worldPacket >> WithdrawGoldLimit;

    for (uint8 i = 0; i < GUILD_BANK_MAX_TABS; i++)
    {
        _worldPacket >> TabFlags[i];
        _worldPacket >> TabWithdrawItemLimit[i];
    }

    _worldPacket.ResetBitPos();
    uint32 rankNameLen = _worldPacket.ReadBits(7);

    RankName = _worldPacket.ReadString(rankNameLen);
}

WorldPacket const* WorldPackets::Guild::GuildEventNewLeader::Write()
{
    _worldPacket.WriteBit(SelfPromoted);
    _worldPacket.WriteBits(OldLeaderName.length(), 6);
    _worldPacket.WriteBits(NewLeaderName.length(), 6);
    _worldPacket.FlushBits();

    _worldPacket << OldLeaderGUID;
    _worldPacket << OldLeaderVirtualRealmAddress;
    _worldPacket << NewLeaderGUID;
    _worldPacket << NewLeaderVirtualRealmAddress;

    _worldPacket.WriteString(OldLeaderName);
    _worldPacket.WriteString(NewLeaderName);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventTabModified::Write()
{
    _worldPacket << Tab;

    _worldPacket.WriteBits(Name.length(), 7);
    _worldPacket.WriteBits(Icon.length(), 9);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);
    _worldPacket.WriteString(Icon);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventTabTextChanged::Write()
{
    _worldPacket << Tab;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankQueryTab::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> TabId;

    FullUpdate = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Guild::GuildBankQueryResults::Write()
{
    _worldPacket << Money;
    _worldPacket << Tab;
    _worldPacket << WithdrawalsRemaining;
    _worldPacket << uint32(TabInfo.size());
    _worldPacket << uint32(ItemInfo.size());
    _worldPacket.WriteBit(FullUpdate);
    _worldPacket.FlushBits();

    for (GuildBankTabInfo const& tab : TabInfo)
    {
        _worldPacket << tab.TabIndex;
        _worldPacket.WriteBits(tab.Name.length(), 7);
        _worldPacket.WriteBits(tab.Icon.length(), 9);
        _worldPacket.FlushBits();

        _worldPacket.WriteString(tab.Name);
        _worldPacket.WriteString(tab.Icon);
    }

    for (GuildBankItemInfo const& item : ItemInfo)
    {
        _worldPacket << item.Slot;
        _worldPacket << item.Count;
        _worldPacket << item.EnchantmentID;
        _worldPacket << item.Charges;
        _worldPacket << item.OnUseEnchantmentID;
        _worldPacket << item.Flags;
        _worldPacket << item.Item;
        _worldPacket.WriteBits(item.SocketEnchant.size(), 2);
        _worldPacket.WriteBit(item.Locked);
        _worldPacket.FlushBits();

        for (auto const& v : item.SocketEnchant)
            _worldPacket << v;
    }

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankSwapItemsLegacy::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
    _worldPacket >> BankSlot;
    _worldPacket >> ItemID;
    _worldPacket >> BankTab1;
    _worldPacket >> BankSlot1;
    _worldPacket >> ItemID1;
    _worldPacket >> BankItemCount;
    _worldPacket >> ContainerSlot;
    _worldPacket >> ContainerItemSlot;
    _worldPacket >> ToSlot;
    _worldPacket >> StackCount;

    _worldPacket.ResetBitPos();
    BankOnly = _worldPacket.ReadBit();
    AutoStore = _worldPacket.ReadBit();
}

void WorldPackets::Guild::GuildBankSwapItems::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
    _worldPacket >> BankSlot;
    _worldPacket >> PlayerSlot;
    HasBag = _worldPacket.ReadBit();
    if (HasBag)
        _worldPacket >> PlayerBag;
}

void WorldPackets::Guild::GuildBankSwapItemsAuto::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
    _worldPacket >> BankSlot;
}

void WorldPackets::Guild::GuildBankSwapItemsCount::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
    _worldPacket >> BankSlot;
    _worldPacket >> PlayerSlot;
    _worldPacket >> StackCount;

    HasBag = _worldPacket.ReadBit();
    _worldPacket.ResetBitPos();
    
    if (HasBag)
        _worldPacket >> PlayerBag;
}

void WorldPackets::Guild::GuildBankSwapItemsBankBank::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
    _worldPacket >> BankSlot;
    _worldPacket >> NewBankTab;
    _worldPacket >> NewBankSlot;
}

void WorldPackets::Guild::GuildBankSwapItemsBankBankCount::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
    _worldPacket >> BankSlot;
    _worldPacket >> NewBankTab;
    _worldPacket >> NewBankSlot;
    _worldPacket >> StackCount;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::LFGuildRecruitData const& recruit)
{
    data << recruit.RecruitGUID;
    data << recruit.RecruitVirtualRealm;
    data << recruit.CharacterClass;
    data << recruit.CharacterGender;
    data << recruit.CharacterLevel;
    data << recruit.ClassRoles;
    data << recruit.PlayStyle;
    data << recruit.Availability;
    data << recruit.SecondsSinceCreated;
    data << recruit.SecondsUntilExpiration;
    data.WriteBits(recruit.Name.length(), 6);
    data.WriteBits(recruit.Comment.length(), 10);
    data.WriteString(recruit.Name);
    data.WriteString(recruit.Comment);

    return data;
}

WorldPacket const* WorldPackets::Guild::LFGuildRecruits::Write()
{
    _worldPacket << static_cast<uint32>(Recruits.size());
    _worldPacket << int32(UpdateTime);
    for (auto const& v : Recruits)
        _worldPacket << v;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::LFGuildPost::Write()
{
    _worldPacket.WriteBit(Post.is_initialized());
    _worldPacket.FlushBits();
    if (Post)
    {
        _worldPacket.WriteBit(Post->Active);
        _worldPacket.WriteBits(Post->Comment.length(), 10);
        _worldPacket.FlushBits();

        _worldPacket << Post->PlayStyle;
        _worldPacket << Post->Availability;
        _worldPacket << Post->ClassRoles;
        _worldPacket << Post->LevelRange;
        _worldPacket << Post->SecondsRemaining;
        _worldPacket.WriteString(Post->Comment);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::LFGuildBrowseResponse::Write()
{
    _worldPacket << static_cast<uint32>(Browses.size());
    for (auto const& v : Browses)
    {
        _worldPacket.WriteBits(v.GuildName.length(), 7);
        _worldPacket.WriteBits(v.Comment.length(), 10);
        _worldPacket.FlushBits();
        _worldPacket << v.GuildGUID;
        _worldPacket << v.GuildVirtualRealm;
        _worldPacket << v.GuildMembers;
        _worldPacket << v.GuildAchievementPoints;
        _worldPacket << v.PlayStyle;
        _worldPacket << v.Availability;
        _worldPacket << v.ClassRoles;
        _worldPacket << v.LevelRange;
        _worldPacket << v.EmblemColor;
        _worldPacket << v.EmblemStyle;
        _worldPacket << v.BorderColor;
        _worldPacket << v.BorderStyle;
        _worldPacket << v.Background;
        _worldPacket << v.Cached;
        _worldPacket << v.MembershipRequested;
        _worldPacket.WriteString(v.GuildName);
        _worldPacket.WriteString(v.Comment);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::LFGuildApplication::Write()
{
    _worldPacket << NumRemaining;
    _worldPacket << static_cast<uint32>(Applications.size());
    for (auto const& v : Applications)
    {
        _worldPacket << v.GuildGUID;
        _worldPacket << v.GuildVirtualRealm;
        _worldPacket << v.ClassRoles;
        _worldPacket << v.PlayStyle;
        _worldPacket << v.Availability;
        _worldPacket << v.SecondsSinceCreated;
        _worldPacket.WriteBits(v.GuildName.length(), 7);
        _worldPacket.WriteBits(v.Comment.length(), 10);
        _worldPacket.WriteString(v.GuildName);
        _worldPacket.WriteString(v.Comment);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildReputationReactionChanged::Write()
{
    _worldPacket << MemberGUID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildFlaggedForRename::Write()
{
    _worldPacket.WriteBit(FlagSet);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildChangeNameResult::Write()
{
    _worldPacket.WriteBit(Success);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildNameChanged::Write()
{
    _worldPacket << GuildGUID;
    _worldPacket.WriteBits(GuildName.length(), 7);
    _worldPacket.WriteString(GuildName);

    return &_worldPacket;
}

void WorldPackets::Guild::LFGuildSetGuildPost::Read()
{
    _worldPacket >> PlayStyle;
    _worldPacket >> Availability;
    _worldPacket >> ClassRoles;
    _worldPacket >> LevelRange;
    Active = _worldPacket.ReadBit();
    Comment = _worldPacket.ReadString(_worldPacket.ReadBits(10));
}

void WorldPackets::Guild::RequestGuildPartyState::Read()
{
    _worldPacket >> GuildGUID;
}

WorldPacket const* WorldPackets::Guild::GuildPartyState::Write()
{
    _worldPacket.WriteBit(InGuildParty);
    _worldPacket.FlushBits();

    _worldPacket << NumMembers;
    _worldPacket << NumRequired;
    _worldPacket << GuildXPEarnedMult;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRewardItem const& rewardItem)
{
    data << rewardItem.ItemID;
    data << rewardItem.Unk4;
    data << uint32(rewardItem.AchievementsRequired.size());
    data << rewardItem.RaceMask;
    data << rewardItem.MinGuildLevel;
    data << rewardItem.MinGuildRep;
    data << rewardItem.Cost;

    for (uint8 i = 0; i < rewardItem.AchievementsRequired.size(); i++)
        data << rewardItem.AchievementsRequired[i];

    return data;
}

void WorldPackets::Guild::RequestGuildRewardsList::Read()
{
    _worldPacket >> CurrentVersion;
}

WorldPacket const* WorldPackets::Guild::GuildRewardList::Write()
{
    _worldPacket << Version;
    _worldPacket << uint32(RewardItems.size());

    for (GuildRewardItem const& item : RewardItems)
        _worldPacket << item;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankActivate::Read()
{
    _worldPacket >> Banker;
    FullUpdate = _worldPacket.ReadBit();
}

void WorldPackets::Guild::GuildBankBuyTab::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;
}

void WorldPackets::Guild::GuildBankUpdateTab::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> BankTab;

    _worldPacket.ResetBitPos();
    uint32 nameLen = _worldPacket.ReadBits(7);
    uint32 iconLen = _worldPacket.ReadBits(9);

    Name = _worldPacket.ReadString(nameLen);
    Icon = _worldPacket.ReadString(iconLen);
}

void WorldPackets::Guild::GuildBankDepositMoney::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> Money;
}

void WorldPackets::Guild::GuildBankWithdrawMoney::Read()
{
    _worldPacket >> Banker;
    _worldPacket >> Money;
}

void WorldPackets::Guild::GuildBankLogQuery::Read()
{
    _worldPacket >> TabId;
}

WorldPacket const* WorldPackets::Guild::GuildBankLogQueryResults::Write()
{
    _worldPacket << TabId;
    _worldPacket << uint32(Entry.size());
    _worldPacket.WriteBit(WeeklyBonusMoney.is_initialized());
    _worldPacket.FlushBits();

    for (GuildBankLogEntry const& logEntry : Entry)
    {
        _worldPacket << logEntry.PlayerGUID;
        _worldPacket << logEntry.TimeOffset;
        _worldPacket << logEntry.EntryType;

        _worldPacket.WriteBit(logEntry.Money.is_initialized());
        _worldPacket.WriteBit(logEntry.ItemID.is_initialized());
        _worldPacket.WriteBit(logEntry.Count.is_initialized());
        _worldPacket.WriteBit(logEntry.OtherTab.is_initialized());
        _worldPacket.FlushBits();

        if (logEntry.Money)
            _worldPacket << *logEntry.Money;

        if (logEntry.ItemID)
            _worldPacket << *logEntry.ItemID;

        if (logEntry.Count)
            _worldPacket << *logEntry.Count;

        if (logEntry.OtherTab)
            _worldPacket << *logEntry.OtherTab;
    }

    if (WeeklyBonusMoney)
        _worldPacket << *WeeklyBonusMoney;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankTextQuery::Read()
{
    _worldPacket >> TabId;
}

WorldPacket const* WorldPackets::Guild::GuildBankTextQueryResult::Write()
{
    _worldPacket << TabId;

    _worldPacket.WriteBits(Text.length(), 14);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Text);

    return &_worldPacket;
}

void WorldPackets::Guild::GuildBankSetTabText::Read()
{
    _worldPacket >> TabId;

    _worldPacket.ResetBitPos();
    uint32 tabTextLen = _worldPacket.ReadBits(14);

    TabText = _worldPacket.ReadString(tabTextLen);
}

WorldPacket const* WorldPackets::Guild::GuildBankRemainingWithdrawMoney::Write()
{
    _worldPacket << RemainingWithdrawMoney;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildEventLogQueryResults::Write()
{
    _worldPacket.reserve(4 + Entry.size() * 38);

    _worldPacket << uint32(Entry.size());

    for (GuildEventEntry const& entry : Entry)
    {
        _worldPacket << entry.PlayerGUID;
        _worldPacket << entry.OtherGUID;
        _worldPacket << entry.TransactionType;
        _worldPacket << entry.RankID;
        _worldPacket << entry.TransactionDate;
    }

    return &_worldPacket;
}

void WorldPackets::Guild::GuildQueryNews::Read()
{
    _worldPacket >> GuildGUID;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildNewsEvent const& newsEvent)
{
    data << newsEvent.Id;
    data << MS::Utilities::WowTime::Encode(newsEvent.CompletedDate);
    data << newsEvent.Type;
    data << newsEvent.Flags;

    for (uint8 i = 0; i < 2; i++)
        data << newsEvent.Data[i];

    data << newsEvent.MemberGuid;
    data << uint32(newsEvent.MemberList.size());

    for (ObjectGuid memberGuid : newsEvent.MemberList)
        data << memberGuid;

    data.WriteBit(newsEvent.Item.is_initialized());
    data.FlushBits();

    if (newsEvent.Item)
        data << *newsEvent.Item;  // WorldPackets::Item::ItemInstance

    return data;
}

WorldPacket const* WorldPackets::Guild::GuildNews::Write()
{
    _worldPacket << static_cast<uint32>(NewsEvents.size());
    for (auto const& newsEvent : NewsEvents)
        _worldPacket << newsEvent;

    return &_worldPacket;
}

void WorldPackets::Guild::GuildNewsUpdateSticky::Read()
{
    _worldPacket >> GuildGUID;
    _worldPacket >> NewsID;

    NewsID = _worldPacket.ReadBit();
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Guild::GuildRankData const& rankData)
{
    data << rankData.RankID;
    data << rankData.RankOrder;
    data << rankData.Flags;
    data << rankData.WithdrawGoldLimit;

    for (uint8 i = 0; i < GUILD_BANK_MAX_TABS; i++)
    {
        data << rankData.TabFlags[i];
        data << rankData.TabWithdrawItemLimit[i];
    }

    data.WriteBits(rankData.RankName.length(), 7);
    data.FlushBits();

    data.WriteString(rankData.RankName);

    return data;
}

void WorldPackets::Guild::GuildAddRank::Read()
{
    uint32 nameLen = _worldPacket.ReadBits(7);
    _worldPacket.ResetBitPos();

    _worldPacket >> RankOrder;
    Name = _worldPacket.ReadString(nameLen);
}

void WorldPackets::Guild::GuildAssignMemberRank::Read()
{
    _worldPacket >> Member;
    _worldPacket >> RankOrder;
}

void WorldPackets::Guild::GuildDeleteRank::Read()
{
    _worldPacket >> RankOrder;
}

void WorldPackets::Guild::GuildGetRanks::Read()
{
    _worldPacket >> GuildGUID;
}

WorldPacket const* WorldPackets::Guild::GuildRanks::Write()
{
    _worldPacket << uint32(Ranks.size());

    for (GuildRankData const& rank : Ranks)
        _worldPacket << rank;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildSendRankChange::Write()
{
    _worldPacket << Officer;
    _worldPacket << Other;
    _worldPacket << RankID;

    _worldPacket.WriteBit(Promote);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Guild::GuildShiftRank::Read()
{
    _worldPacket >> RankOrder;
    ShiftUp = _worldPacket.ReadBit();
}

void WorldPackets::Guild::GuildSetGuildMaster::Read()
{
    uint32 nameLen = _worldPacket.ReadBits(9);
    NewMasterName = _worldPacket.ReadString(nameLen);
}

WorldPacket const* WorldPackets::Guild::GuildChallengeUpdated::Write()
{
    for (int8 i = 0; i < 6; i++)
        _worldPacket << CurrentCount[i];

    for (int8 i = 0; i < 6; i++)
        _worldPacket << MaxCount[i];

    for (int8 i = 0; i < 6; i++)
        _worldPacket << Gold[i];

    for (int8 i = 0; i < 6; i++)
        _worldPacket << MaxLevelGold[i];

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildChallengeCompleted::Write()
{
    _worldPacket << ChallengeType;
    _worldPacket << CurrentCount;
    _worldPacket << MaxCount;
    _worldPacket << GoldAwarded;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::PlayerSaveGuildEmblem::Write()
{
    _worldPacket << int32(Error);

    return &_worldPacket;
}

void WorldPackets::Guild::QueryMemberRecipes::Read()
{
    _worldPacket >> GuildMember;
    _worldPacket >> GuildGUID;
    _worldPacket >> SkillLineID;
}

void WorldPackets::Guild::QueryGuildMembersForRecipe::Read()
{
    _worldPacket >> GuildGUID;
    _worldPacket >> SkillLineID;
    _worldPacket >> SpellID;
    _worldPacket >> UniqueBit;
}

WorldPacket const* WorldPackets::Guild::QueryGuildMembersForRecipeReponse::Write()
{
    _worldPacket << SkillLineID;
    _worldPacket << SpellID;
    _worldPacket << static_cast<uint32>(Member.size());
    for (auto const& v : Member)
        _worldPacket << v;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildMemberRecipes::Write()
{
    _worldPacket << Member;
    _worldPacket << SkillLineID;
    _worldPacket << SkillRank;
    _worldPacket << SkillStep;
    for (auto const& v : SkillLineBitArray)
        _worldPacket << v;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Guild::GuildInviteDeclined::Write()
{
    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket << AutoDecline;
    _worldPacket.FlushBits();
    _worldPacket << VirtualRealmAddress;
    _worldPacket << Name;

    return &_worldPacket;
}

void WorldPackets::Guild::QueryRecipes::Read()
{
    _worldPacket >> GuildGUID;
}

void WorldPackets::Guild::LFGuildAddRecruit::Read()
{
    _worldPacket >> GuildGUID;
    _worldPacket >> PlayStyle;
    _worldPacket >> Availability;
    _worldPacket >> ClassRoles;
    Comment = _worldPacket.ReadString(_worldPacket.ReadBits(10));
}

void WorldPackets::Guild::LFGuildBrowse::Read()
{
    _worldPacket >> PlayStyle;
    _worldPacket >> Availability;
    _worldPacket >> ClassRoles;
    _worldPacket >> CharacterLevel;
}

void WorldPackets::Guild::LFGuildRemoveRecruit::Read()
{
    _worldPacket >> GuildGUID;
}

void WorldPackets::Guild::LFGuildGetRecruits::Read()
{
    _worldPacket >> LastUpdate;
}

void WorldPackets::Guild::LFGuildDeclineRecruit::Read()
{
    _worldPacket >> RecruitGUID;
}

void WorldPackets::Guild::SaveGuildEmblem::Read()
{
    _worldPacket >> Vendor;
    _worldPacket >> EStyle;
    _worldPacket >> EColor;
    _worldPacket >> BStyle;
    _worldPacket >> BColor;
    _worldPacket >> Bg;
}

void WorldPackets::Guild::GuildChangeNameRequest::Read()
{
    NewName = _worldPacket.ReadString(_worldPacket.ReadBits(7));
}

void WorldPackets::Guild::GuildSetAchievementTracking::Read()
{
    uint32 count = _worldPacket.read<uint32>();
    for (uint32 i = 0; i < count; ++i)
        AchievementIDs.insert(_worldPacket.read<uint32>());
}