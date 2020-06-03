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

#include "GuildMgr.h"
#include "CharacterData.h"
#include "GuildMgr.h"
#include "WordFilterMgr.h"
#include "GuildPackets.h"
#include "GlobalFunctional.h"
#include "PlayerDefines.h"
#include "AchievementPackets.h"

bool WorldSession::CanOpenGuildBank(ObjectGuid guid)
{
    if (guid.IsGameObject())
    {
        if (!_player->GetGameObjectIfCanInteractWith(guid, GAMEOBJECT_TYPE_GUILD_BANK))
            return false;
    }
    else if (guid.IsAnyTypeCreature())
    {
        if (!_player->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_GUILD_BANKER))
            return false;
    }
    else
        return false;

    return true;
}

inline Guild* _GetPlayerGuild(WorldSession* session, bool sendError = false)
{
    if (ObjectGuid::LowType guildId = session->GetPlayer()->GetGuildId())
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
            return guild;

    if (sendError)
        Guild::SendCommandResult(session, GUILD_CREATE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD);

    return nullptr;
}

void WorldSession::HandleGuildQueryOpcode(WorldPackets::Guild::QueryGuildInfo& packet)
{
    if (Guild* guild = sGuildMgr->GetGuildByGuid(packet.GuildGuid))
    {
        guild->SendQueryResponse(this);
        return;
    }

    WorldPackets::Guild::QueryGuildInfoResponse response;
    response.GuildGuid = packet.GuildGuid;
    SendPacket(response.Write());

    Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildInviteByName(WorldPackets::Guild::GuildInviteByName& packet)
{
    if (normalizePlayerName(packet.Name))
        if (Guild* guild = _GetPlayerGuild(this, true))
            guild->HandleInviteMember(this, packet.Name);
}

void WorldSession::HandleGuildOfficerRemoveMember(WorldPackets::Guild::GuildOfficerRemoveMember& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRemoveMember(this, packet.Removee);
}

void WorldSession::HandleGuildAcceptInvite(WorldPackets::Guild::AcceptGuildInvite& /*packet*/)
{
    if (_GetPlayerGuild(this, true))
        return;

    if (Guild* guild = sGuildMgr->GetGuildById(GetPlayer()->GetGuildIdInvited()))
        guild->HandleAcceptMember(this);
}

void WorldSession::HandleGuildDeclineInvitation(WorldPackets::Guild::GuildDeclineInvitation& /*decline*/)
{
    if (Player* inviter = ObjectAccessor::FindPlayer(GetPlayer()->GetGuildInviterGuid()))
    {
        WorldPackets::Guild::GuildInviteDeclined packet;
        packet.Name = GetPlayer()->GetName();
        packet.VirtualRealmAddress = GetVirtualRealmAddress();
        inviter->SendDirectMessage(packet.Write());
    }

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);
}

void WorldSession::HandleGuildRosterOpcode(WorldPackets::Guild::GuildGetRoster& /*packet*/)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->SendRoster(this);
}

void WorldSession::HandleGuildPromoteMember(WorldPackets::Guild::GuildPromoteMember& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleUpdateMemberRank(this, packet.Promotee, false);
}

void WorldSession::HandleGuildDemoteMember(WorldPackets::Guild::GuildDemoteMember& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleUpdateMemberRank(this, packet.Demotee, true);
}

void WorldSession::HandleGuildAssignRank(WorldPackets::Guild::GuildAssignMemberRank& packet)
{
    if (packet.RankOrder < 0 || packet.RankOrder > GUILD_RANKS_MAX_COUNT)
        return;

    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMemberRank(this, packet.Member, GetPlayer()->GetGUID(), packet.RankOrder);
}

void WorldSession::HandleGuildLeave(WorldPackets::Guild::GuildLeave& /*packet*/)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleLeaveMember(this);
}

void WorldSession::HandleGuildDisbandOpcode(WorldPackets::Guild::GuildDelete& /*packet*/)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleDisband(this);
}

void WorldSession::HandleGuildSetGuildMaster(WorldPackets::Guild::GuildSetGuildMaster& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetNewGuildMaster(this, packet.NewMasterName);
}

void WorldSession::HandleGuildUpdateMotdText(WorldPackets::Guild::GuildUpdateMotdText& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMOTD(this, packet.MotdText);
}

void WorldSession::HandleShiftRank(WorldPackets::Guild::GuildShiftRank& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleShiftRank(this, packet.RankOrder, packet.ShiftUp);
}

void WorldSession::HandleGuildSetMemberNote(WorldPackets::Guild::GuildSetMemberNote& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetMemberNote(this, packet.Note, packet.NoteeGUID, packet.IsPublic);
}

void WorldSession::HandleGuildGetRanks(WorldPackets::Guild::GuildGetRanks& packet)
{
    if (Guild* guild = sGuildMgr->GetGuildByGuid(packet.GuildGUID))
        if (guild->IsMember(_player->GetGUID()))
            guild->SendGuildRankInfo(this);
}

void WorldSession::HandleGuildAddRank(WorldPackets::Guild::GuildAddRank& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleAddNewRank(this, packet.Name); //, rankId);
}

void WorldSession::HandleGuildDeleteRank(WorldPackets::Guild::GuildDeleteRank& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleRemoveRank(this, packet.RankOrder);
}

void WorldSession::HandleGuildUpdateInfoText(WorldPackets::Guild::GuildUpdateInfoText& packet)
{
    if (Guild* guild = _GetPlayerGuild(this, true))
        guild->HandleSetInfo(this, packet.InfoText);
}

void WorldSession::HandleSaveGuildEmblem(WorldPackets::Guild::SaveGuildEmblem& packet)
{
    if (!GetPlayer()->GetNPCIfCanInteractWith(packet.Vendor, UNIT_NPC_FLAG_TABARDDESIGNER))
    {
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_INVALIDVENDOR);
        return;
    }

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    EmblemInfo emblemInfo(packet.EStyle, packet.EColor, packet.BStyle, packet.BColor, packet.Bg);

    if (!emblemInfo.ValidateEmblemColors())
    {
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_INVALID_TABARD_COLORS);
        return;
    }

    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetEmblem(this, emblemInfo);
    else
        Guild::SendSaveEmblemResult(this, ERR_GUILDEMBLEM_NOGUILD);
}

void WorldSession::HandleGuildEventLogQuery(WorldPackets::Guild::GuildEventLogQuery& /*packet*/)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendEventLog(this);
}

void WorldSession::HandleGuildBankMoneyWithdrawn(WorldPackets::Guild::GuildBankRemainingWithdrawMoneyQuery& /* packet */)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendMoneyInfo(this);
}

void WorldSession::HandleGuildPermissions(WorldPackets::Guild::GuildPermissionsQuery& /* packet */)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendPermissions(this);
}

void WorldSession::HandleGuildBankActivate(WorldPackets::Guild::GuildBankActivate& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankList(this, 0, packet.FullUpdate);
    else
        Guild::SendCommandResult(this, GUILD_BANK, ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildBankQueryTab(WorldPackets::Guild::GuildBankQueryTab& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

        if (Guild* guild = _GetPlayerGuild(this))
            guild->SendBankList(this, packet.TabId, packet.FullUpdate);
}

void WorldSession::HandleGuildBankDepositMoney(WorldPackets::Guild::GuildBankDepositMoney& packet)
{
    if (Player* player = GetPlayer())
    {
        if (!CanOpenGuildBank(packet.Banker))
            return;

        if (player->HasEnoughMoney(packet.Money))
            if (Guild* guild = _GetPlayerGuild(this))
                guild->HandleMemberDepositMoney(this, packet.Money);
    }
}

void WorldSession::HandleGuildBankWithdrawMoney(WorldPackets::Guild::GuildBankWithdrawMoney& packet)
{
    if (Player* player = GetPlayer())
    {
        if (!CanOpenGuildBank(packet.Banker))
            return;

        if (Guild* guild = _GetPlayerGuild(this))
            guild->HandleMemberWithdrawMoney(this, packet.Money);
    }
}

void WorldSession::HandleGuildBankSwapItemsLegacy(WorldPackets::Guild::GuildBankSwapItemsLegacy& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    auto guild = _GetPlayerGuild(this);
    if (!guild)
        return;

    if (packet.BankOnly)
        guild->SwapItems(GetPlayer(), packet.BankTab1, packet.BankSlot1, packet.BankTab, packet.BankSlot, packet.StackCount);
    else
    {
        // Player <-> Bank
        // Allow to work with inventory only
        if (!Player::IsInventoryPos(packet.ContainerSlot, packet.ContainerItemSlot) && !(packet.ContainerSlot == NULL_BAG && packet.ContainerItemSlot == NULL_SLOT) && !packet.AutoStore)
            GetPlayer()->SendEquipError(EQUIP_ERR_INTERNAL_BAG_ERROR);
        else
            guild->SwapItemsWithInventory(GetPlayer(), packet.ToSlot != 0, packet.BankTab, packet.BankSlot, packet.ContainerSlot, packet.ContainerItemSlot, packet.StackCount);
    }
}

void WorldSession::HandleGuildBankMoveItemsPlayerBank(WorldPackets::Guild::GuildBankSwapItems& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), false, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, 0);
}

void WorldSession::HandleGuildBankMoveItemsBankPlayer(WorldPackets::Guild::GuildBankSwapItems& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), true, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, 0);
}

void WorldSession::HandleGuildBankMoveItemsBankBank(WorldPackets::Guild::GuildBankSwapItemsBankBank& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItems(GetPlayer(), packet.BankTab, packet.BankSlot, packet.NewBankTab, packet.NewBankSlot, 0);
}

void WorldSession::HandleGuildBankMoveItemsPlayerBankCount(WorldPackets::Guild::GuildBankSwapItemsCount& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), false, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, packet.StackCount);
}

void WorldSession::HandleGuildBankMoveItemsBankPlayerCount(WorldPackets::Guild::GuildBankSwapItemsCount& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), true, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, packet.StackCount);
}

void WorldSession::HandleGuildBankMoveItemsBankPlayerAuto(WorldPackets::Guild::GuildBankSwapItemsAuto& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), true, packet.BankTab, packet.BankSlot, NULL_BAG, NULL_SLOT, 0);
}

void WorldSession::HandleGuildBankMoveItemsBankBankCount(WorldPackets::Guild::GuildBankSwapItemsBankBankCount& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItems(GetPlayer(), packet.BankTab, packet.BankSlot, packet.NewBankTab, packet.NewBankSlot, packet.StackCount);
}

void WorldSession::HandleGuildBankMergeItemsPlayerBank(WorldPackets::Guild::GuildBankSwapItemsCount& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), false, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, packet.StackCount);
}

void WorldSession::HandleGuildBankMergeItemsBankPlayer(WorldPackets::Guild::GuildBankSwapItemsCount& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), true, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, packet.StackCount);
}

void WorldSession::HandleGuildBankMergeItemsBankBank(WorldPackets::Guild::GuildBankSwapItemsBankBankCount& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItems(GetPlayer(), packet.BankTab, packet.BankSlot, packet.NewBankTab, packet.NewBankSlot, packet.StackCount);
}

void WorldSession::HandleGuildBankSwapItemsBankPlayer(WorldPackets::Guild::GuildBankSwapItems& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItemsWithInventory(GetPlayer(), true, packet.BankTab, packet.BankSlot, packet.HasBag ? packet.PlayerBag : INVENTORY_SLOT_BAG_0, packet.PlayerSlot, 0);
}

void WorldSession::HandleGuildBankSwapItemsBankBank(WorldPackets::Guild::GuildBankSwapItemsBankBank& packet)
{
    if (!CanOpenGuildBank(packet.Banker))
        return;

    if (auto guild = GetPlayer()->GetGuild())
        guild->SwapItems(GetPlayer(), packet.BankTab, packet.BankSlot, packet.NewBankTab, packet.NewBankSlot, 0);
}

void WorldSession::HandleGuildBankBuyTab(WorldPackets::Guild::GuildBankBuyTab& packet)
{
    if (!packet.Banker || CanOpenGuildBank(packet.Banker))
        if (Guild* guild = _GetPlayerGuild(this))
            guild->HandleBuyBankTab(this, packet.BankTab);
}

void WorldSession::HandleGuildBankUpdateTab(WorldPackets::Guild::GuildBankUpdateTab& packet)
{
    if (!packet.Name.empty() && !packet.Icon.empty())
    {
        if (!CanOpenGuildBank(packet.Banker))
            return;

        if (Guild* guild = _GetPlayerGuild(this))
            guild->HandleSetBankTabInfo(this, packet.BankTab, packet.Name, packet.Icon);
    }
}

void WorldSession::HandleGuildBankLogQuery(WorldPackets::Guild::GuildBankLogQuery& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankLog(this, packet.TabId);
}

void WorldSession::HandleGuildBankTextQuery(WorldPackets::Guild::GuildBankTextQuery& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendBankTabText(this, packet.TabId);
}

void WorldSession::HandleGuildBankSetTabText(WorldPackets::Guild::GuildBankSetTabText& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SetBankTabText(packet.TabId, packet.TabText);
}

void WorldSession::HandleGuildSetRankPermissions(WorldPackets::Guild::GuildSetRankPermissions& packet)
{
    Guild* guild = _GetPlayerGuild(this, true);
    if (!guild)
        return;

    GuildBankRightsAndSlotsVec rightsAndSlots(GUILD_BANK_MAX_TABS);
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
        rightsAndSlots[tabId] = GuildBankRightsAndSlots(tabId, uint8(packet.TabFlags[tabId]), uint32(packet.TabWithdrawItemLimit[tabId]));

    guild->HandleSetRankInfo(this, packet.RankOrder, packet.RankName, packet.Flags, packet.WithdrawGoldLimit * GOLD, rightsAndSlots);
}

void WorldSession::HandleGuildRequestPartyState(WorldPackets::Guild::RequestGuildPartyState& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
        if (guild->GetGUID() == packet.GuildGUID)
            guild->HandleGuildPartyRequest(this);
}

void WorldSession::HandleAutoDeclineGuildInvites(WorldPackets::Guild::AutoDeclineGuildInvites& packet)
{
    GetPlayer()->ApplyModFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_AUTO_DECLINE_GUILD, packet.Allow);
}

void WorldSession::HandleGuildRewardsQueryOpcode(WorldPackets::Guild::RequestGuildRewardsList& packet)
{
    // from sniffs, strange but testing
    if (packet.CurrentVersion != 0)
    {
        WorldPackets::Guild::GuildRewardList resp;
        resp.Version = packet.CurrentVersion;
        resp.RewardItems.clear();

        SendPacket(resp.Write());
        return;
    }

    if (sGuildMgr->GetGuildById(_player->GetGuildId()))
    {
        std::vector<GuildReward> const& rewards = sGuildMgr->GetGuildRewards();

        WorldPackets::Guild::GuildRewardList rewardList;
        rewardList.Version = time(nullptr);
        rewardList.RewardItems.reserve(rewards.size());

        for (const auto& reward : rewards)
        {
            WorldPackets::Guild::GuildRewardItem rewardItem;
            rewardItem.ItemID = reward.Entry;
            if (reward.AchievementsRequired.size() >= 2)
                rewardItem.Unk4 = 2;
            rewardItem.RaceMask = reward.Racemask;
            rewardItem.MinGuildLevel = 0;
            rewardItem.MinGuildRep = reward.Standing;
            rewardItem.AchievementsRequired = reward.AchievementsRequired;
            rewardItem.Cost = reward.Price;
            rewardList.RewardItems.push_back(rewardItem);
        }

        SendPacket(rewardList.Write());
    }
}

void WorldSession::HandleGuildQueryNews(WorldPackets::Guild::GuildQueryNews& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
        if (guild->GetGUID() == packet.GuildGUID)
            guild->SendNewsUpdate(this);
}

void WorldSession::HandleGuildNewsUpdateStickyOpcode(WorldPackets::Guild::GuildNewsUpdateSticky& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->HandleNewsSetSticky(this, packet.NewsID, packet.Sticky);
}

void WorldSession::HandleQueryRecipes(WorldPackets::Guild::QueryRecipes& /*packet*/)
{
    Guild* guild = _player->GetGuild();
    if (!guild)
        return;

    Guild::KnownRecipesMap recipesMap = guild->GetGuildRecipes();

    //! 6.0.3
    auto data = new WorldPacket(SMSG_GUILD_KNOWN_RECIPES, 2 + recipesMap.size() * (300 + 4));
    uint32 pos = data->wpos();
    uint32 count = 0;
    *data << uint32(count);

    for (Guild::KnownRecipesMap::const_iterator itr = recipesMap.begin(); itr != recipesMap.end(); ++itr)
    {
        if (itr->second.IsEmpty())
            continue;

        *data << uint32(itr->first);
        data->append(itr->second.recipesMask, KNOW_RECIPES_MASK_SIZE);
        ++count;
    }

    data->put<uint32>(pos, count);

    _player->SendDirectMessage(data);
}

void WorldSession::HandleQueryGuildMembersForRecipe(WorldPackets::Guild::QueryGuildMembersForRecipe& packet)
{
    if (Guild* guild = _player->GetGuild())
        guild->SendGuildMembersForRecipeResponse(this, packet.SkillLineID, packet.SpellID);
}

void WorldSession::HandleQyeryMemberRecipes(WorldPackets::Guild::QueryMemberRecipes& packet)
{
    Guild* guild = _player->GetGuild();
    if (!guild || !guild->IsMember(packet.GuildMember))
        return;

    guild->SendGuildMemberRecipesResponse(this, packet.GuildMember, packet.SkillLineID);
}

void WorldSession::HandleGuildRequestChallengeUpdate(WorldPackets::Guild::GuildChallengeUpdateRequest& /*packet*/)
{
    if (Guild* guild = _GetPlayerGuild(this))
        guild->SendGuildChallengeUpdated(this);
}

void WorldSession::HandleGuildSetFocusedAchievement(WorldPackets::Achievement::GuildSetFocusedAchievement& packet)
{
    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
        guild->GetAchievementMgr().SendAchievementInfo(_player, packet.AchievementID);
}

void WorldSession::HandleGuildAchievementProgressQuery(WorldPackets::Achievement::GuildGetAchievementMembers& packet)
{
    if (Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId()))
        guild->GetAchievementMgr().SendAchievementInfo(_player, packet.AchievementID);
}

void WorldSession::HandleSetAchievementsHidden(WorldPackets::Achievement::SetAchievementsHidden& /*packet*/)
{ }

void WorldSession::HandleGuildChangeNameRequest(WorldPackets::Guild::GuildChangeNameRequest& packet)
{
    if (Guild* guild = _GetPlayerGuild(this))
    {
        if (guild->GetLeaderGUID() != _player->GetGUID())
            return;

        bool success = true;
        if (guild->GetName() == packet.NewName || sCharacterDataStore->IsReservedName(packet.NewName) || !sCharacterDataStore->IsValidCharterName(packet.NewName, GetSessionDbLocaleIndex()) || (sWorld->getBoolConfig(CONFIG_WORD_FILTER_ENABLE) && !sWordFilterMgr->FindBadWord(packet.NewName).empty()))
            success = false;

        WorldPackets::Guild::GuildChangeNameResult result;
        result.Success = success;
        SendPacket(result.Write());

        if (success)
        {
            guild->SetGuildName(packet.NewName);
            guild->SetRename(false);
        }
    }
}

void WorldSession::HandleReplaceGuildMaster(WorldPackets::Guild::ReplaceGuildMaster& /*packet*/)
{ }

void WorldSession::HandleGuildSetAchievementTracking(WorldPackets::Guild::GuildSetAchievementTracking& packet)
{
    if (Guild* guild = GetPlayer()->GetGuild())
        guild->HandleSetAchievementTracking(this, packet.AchievementIDs);
}

void WorldSession::HandleGuildAutoDeclineInvitation(WorldPackets::Guild::GuildAutoDeclineInvitation& /*packet*/)
{
    if (Player* inviter = ObjectAccessor::FindPlayer(GetPlayer()->GetGuildInviterGuid()))
    {
        WorldPackets::Guild::GuildInviteDeclined packet;
        packet.Name = GetPlayer()->GetName();
        packet.AutoDecline = true;
        packet.VirtualRealmAddress = GetVirtualRealmAddress();
        inviter->SendDirectMessage(packet.Write());
    }

    GetPlayer()->SetGuildIdInvited(0);
    GetPlayer()->SetInGuild(0);
}
