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

#include "AuthenticationPackets.h"
#include "BattlenetRpcErrorCodes.h"
#include "ObjectMgr.h"
#include "CharacterData.h"
#include "ClientConfigPackets.h"
#include "SystemPackets.h"
#include "CharacterPackets.h"
#include "TokenPackets.h"
#include "BattlePayMgr.h"

void WorldSession::SendAuthWaitQue(uint32 position)
{
    if (position)
    {
        WorldPackets::Auth::WaitQueueUpdate waitQueueUpdate;
        waitQueueUpdate.WaitInfo.WaitCount = position;
        waitQueueUpdate.WaitInfo.WaitTime = 0;
        waitQueueUpdate.WaitInfo.HasFCM = false;
        SendPacket(waitQueueUpdate.Write());
    }
    else
        SendPacket(WorldPackets::Auth::WaitQueueFinish().Write());
}

void WorldSession::SendAuthResponse(uint8 code, bool queued /*= false*/, uint32 queuePos /*= 0*/)
{
    WorldPackets::Auth::AuthResponse response;
    response.Result = code;

    if (code == ERROR_OK)
    {
        response.SuccessInfo = boost::in_place();

        response.SuccessInfo->AccountExpansionLevel = Expansion();
        response.SuccessInfo->ActiveExpansionLevel = Expansion();
        response.SuccessInfo->VirtualRealmAddress = GetVirtualRealmAddress();
        response.SuccessInfo->CurrencyID = GetBattlePayMgr()->GetShopCurrency();
        response.SuccessInfo->VirtualRealms.emplace_back(GetVirtualRealmAddress(), true, false, sObjectMgr->GetRealmName(realm.Id.Realm), sObjectMgr->GetNormalizedRealmName(realm.Id.Realm));
        response.SuccessInfo->AvailableClasses = &sObjectMgr->GetClassExpansionRequirements();
        response.SuccessInfo->Time = int32(time(nullptr));


            for (auto& templ : charTemplateData)
            {
                if (!templ.second.active)
                    continue;

                auto charTemplate = sCharacterDataStore->GetCharacterTemplate(templ.second.templateId);
                if (!charTemplate)
                    continue;

                WorldPackets::Auth::AuthResponse::CharacterTemplateData templateData;
                templateData.TemplateSetID = templ.second.id;
                for (auto x : charTemplate->Classes)
                    templateData.Classes.emplace_back(x.FactionGroup, x.ClassID);
                for (auto z : charTemplate->Items)
                    templateData.Items.emplace_back(z.ItemID, z.Count, z.ClassID, z.FactionGroup);
                templateData.Name = { std::to_string(templ.second.level) + " level " + std::to_string(templ.second.iLevel) + " ilevel" };
                templateData.Description = { "Create character with " + std::to_string(templ.second.level) + " level and " + std::to_string(templ.second.iLevel) + "ilvl items" };
                response.SuccessInfo->Templates.emplace_back(templateData);
            }
        
    }

    if (queued)
    {
        response.WaitInfo = boost::in_place();
        response.WaitInfo->WaitCount = queuePos;
    }

    SendPacket(response.Write());
}

void WorldSession::SendClientCacheVersion(uint32 version)
{
    WorldPackets::ClientConfig::ClientCacheVersion cache;
    cache.CacheVersion = version;
    SendPacket(cache.Write());
}

void WorldSession::SendFeatureSystemStatusGlueScreen()
{
    WorldPackets::System::FeatureSystemStatusGlueScreen features;
    features.TokenPollTimeSeconds = 300;
    features.TokenRedeemIndex = 0; // ForSubAmount30Days
    features.BpayStoreAvailable = sWorld->getBoolConfig(CONFIG_FEATURE_SYSTEM_BPAY_STORE_ENABLED);
    features.BpayStoreDisabledByParentalControls = false;
    features.CharUndeleteEnabled = HasAuthFlag(AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER);
    features.BpayStoreEnabled = sWorld->getBoolConfig(CONFIG_FEATURE_SYSTEM_BPAY_STORE_ENABLED);
    features.CommerceSystemEnabled = true;
    features.Unk14 = true;
    features.WillKickFromWorld = false;
    features.KioskModeEnabled = false;
    features.TrialBoostEnabled = false;
    features.IsExpansionPreorderInStore = false;
    features.CompetitiveModeEnabled = false;
    features.TokenBalanceEnabled = true;
    features.LiveRegionCharacterListEnabled = false;
    features.LiveRegionCharacterCopyEnabled = false;
    features.LiveRegionAccountCopyEnabled = false;
    features.TokenBalanceAmount = 5500000;
    features.BpayStoreProductDeliveryDelay = 180;
    features.UnkInt1 = 3;
    features.UnkInt2 = 2;
    features.UnkInt3 = 5;
    features.UnkInt4 = 7;
    SendPacket(features.Write());
}

void WorldSession::HandleWowTokenMarketPrice(WorldPackets::Token::RequestWowTokenMarketPrice& packet)
{
    WorldPackets::Token::WowTokenMarketPriceResponse response;
    response.CurrentMarketPrice = 60000 * GOLD;
    response.Result = TOKEN_RESULT_ERROR_DISABLED;
    response.UnkInt = packet.UnkInt;
    response.UnkInt2 = 14400;
    SendPacket(response.Write());
}

void WorldSession::HandleUpdateListedAuctionableTokens(WorldPackets::Token::UpdateListedAuctionableTokens& packet)
{
    WorldPackets::Token::UpdateListedAuctionableTokensResponse response;
    response.UnkInt = packet.Type;
    response.Result = TOKEN_RESULT_ERROR_DISABLED;
    response.AuctionableTokenAuctionableList.resize(0);
    //for (uint8 v : {0})
    {
        WorldPackets::Token::UpdateListedAuctionableTokensResponse::AuctionableTokenAuctionable token;
        token.BuyoutPrice = 60000 * GOLD;
        token.DistributionID = 0;
        token.DateCreated = 0;
        token.Owner = 0;
        token.EndTime = 0;
        response.AuctionableTokenAuctionableList.push_back(token);
    }
    SendPacket(response.Write());
}

void WorldSession::HandleCheckVeteranTokenEligibility(WorldPackets::Token::CheckVeteranTokenEligibility& packet)
{
    WorldPackets::Token::WowTokenCanVeteranBuyResult result;
    result.UnkLong = 0;
    result.UnkInt = packet.UnkInt;
    result.UnkInt2 = 1;
    SendPacket(result.Write());
}
