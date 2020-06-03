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

#include "Guild.h"
#include "CharacterData.h"
#include "GuildMgr.h"
#include "WordFilterMgr.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "PetitionPackets.h"

enum Misc
{
    GUILD_CHARTER = 5863,
    GUILD_CHARTER_COST = 1000,
};

void WorldSession::HandlePetitionBuy(WorldPackets::Petition::PetitionBuy& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* creature = player->GetNPCIfCanInteractWith(packet.Unit, UNIT_NPC_FLAG_PETITIONER);
    if (!creature)
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    uint32 charterid = 0;
    if (creature->isTabardDesigner())
    {
        if (player->GetGuildId())
            return;

        charterid = GUILD_CHARTER;
    }

    if (sGuildMgr->GetGuildByName(packet.Title))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_EXISTS_S, packet.Title);
        return;
    }

    if (sCharacterDataStore->IsReservedName(packet.Title) || !sCharacterDataStore->IsValidCharterName(packet.Title, GetSessionDbLocaleIndex()) || (sWorld->getBoolConfig(CONFIG_WORD_FILTER_ENABLE) && !sWordFilterMgr->FindBadWord(packet.Title).empty()))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_INVALID, packet.Title);
        return;
    }

    if (sWorld->getBoolConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
    {
        stripLineInvisibleChars(packet.Title);

        if (strchr(packet.Title.c_str(), '|'))
        {
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return;
        }
    }

    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(charterid);
    if (!pProto)
    {
        player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, nullptr, charterid);
        return;
    }

    if (!player->HasEnoughMoney(uint64(GUILD_CHARTER_COST)))
    {
        player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, creature, charterid);
        return;
    }

    ItemPosCountVec dest;
    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, charterid, pProto->VendorStackCount);
    if (msg != EQUIP_ERR_OK)
    {
        player->SendEquipError(msg, nullptr, nullptr, charterid);
        return;
    }

    player->ModifyMoney(-GUILD_CHARTER_COST);
    Item* charter = player->StoreNewItem(dest, charterid, true);
    if (!charter)
        return;

    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT, charter->GetGUIDLow());
    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT + 1, 0);
    charter->SetState(ITEM_CHANGED, player);
    player->SendNewItem(charter, 1, true, false);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_BY_OWNER);
    stmt->setUInt64(0, player->GetGUIDLow());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    std::ostringstream ssInvalidPetitionGUIDs;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            ssInvalidPetitionGUIDs << '\'' << fields[0].GetUInt64() << "', ";
        }
        while (result->NextRow());
    }

    ssInvalidPetitionGUIDs << '\'' << charter->GetGUIDLow() << '\'';

    TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Invalid petition GUIDs: %s", ssInvalidPetitionGUIDs.str().c_str());
    CharacterDatabase.EscapeString(packet.Title);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM petition WHERE petitionguid IN (%s)", ssInvalidPetitionGUIDs.str().c_str());
    trans->PAppend("DELETE FROM petition_sign WHERE petitionguid IN (%s)", ssInvalidPetitionGUIDs.str().c_str());

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PETITION);
    stmt->setUInt64(0, player->GetGUIDLow());
    stmt->setUInt64(1, charter->GetGUIDLow());
    stmt->setString(2, packet.Title);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void WorldSession::HandlePetitionShowSignatures(WorldPackets::Petition::PetitionShowSignatures& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    uint8 signs = 0;

    ObjectGuid::LowType petitionGuidLow = packet.Item.GetCounter();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_TYPE);
    stmt->setUInt64(0, petitionGuidLow);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    if (player->GetGuildId())
        return;

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);
    stmt->setUInt64(0, petitionGuidLow);

    result = CharacterDatabase.Query(stmt);
    if (result)
        signs = uint8(result->GetRowCount());


    WorldPackets::Petition::ServerPetitionShowSignatures signaturesPacket;
    signaturesPacket.Item = packet.Item;
    signaturesPacket.Owner = player->GetGUID();
    signaturesPacket.OwnerAccountID = ObjectGuid::Create<HighGuid::WowAccount>(ObjectMgr::GetPlayerAccountIdByGUID(player->GetGUID()));
    signaturesPacket.PetitionID = uint32(packet.Item.GetCounter());

    signaturesPacket.Signatures.reserve(signs);
    for (uint8 i = 1; i <= signs; ++i)
    {
        Field* fields2 = result->Fetch();
        ObjectGuid signerGUID = ObjectGuid::Create<HighGuid::Player>(fields2[0].GetUInt64());

        WorldPackets::Petition::ServerPetitionShowSignatures::PetitionSignature signature;
        signature.Signer = signerGUID;
        signature.Choice = 0;
        signaturesPacket.Signatures.push_back(signature);

        if (!result->NextRow())
            break;
    }

    SendPacket(signaturesPacket.Write());
}

void WorldSession::HandleQueryPetition(WorldPackets::Petition::QueryPetition& packet)
{
    SendPetitionQueryOpcode(packet.ItemGUID);
}

void WorldSession::SendPetitionQueryOpcode(ObjectGuid petitionguid)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    ObjectGuid ownerguid;
    std::string name = "NO_NAME_FOR_GUID";

    WorldPackets::Petition::QueryPetitionResponse responsePacket;
    responsePacket.PetitionID = uint32(petitionguid.GetCounter());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION);
    stmt->setUInt64(0, petitionguid.GetCounter());
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        ownerguid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
        name = fields[1].GetString();
    }
    else
    {
        responsePacket.Allow = false;
        SendPacket(responsePacket.Write());
        return;
    }

    int32 reqSignatures = sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS);

    WorldPackets::Petition::PetitionInfo petitionInfo;
    petitionInfo.PetitionID = int32(petitionguid.GetCounter());
    petitionInfo.Petitioner = ownerguid;
    petitionInfo.MinSignatures = reqSignatures;
    petitionInfo.MaxSignatures = reqSignatures;
    petitionInfo.Title = name;

    responsePacket.Allow = true;
    responsePacket.Info = petitionInfo;

    SendPacket(responsePacket.Write());
}

void WorldSession::HandlePetitionRenameGuild(WorldPackets::Petition::PetitionRenameGuild& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetItemByGuid(packet.PetitionGuid))
        return;

    if (sGuildMgr->GetGuildByName(packet.NewGuildName))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_EXISTS_S, packet.NewGuildName);
        return;
    }

    if (sCharacterDataStore->IsReservedName(packet.NewGuildName) || !sCharacterDataStore->IsValidCharterName(packet.NewGuildName, GetSessionDbLocaleIndex()) || (sWorld->getBoolConfig(CONFIG_WORD_FILTER_ENABLE) && !sWordFilterMgr->FindBadWord(packet.NewGuildName).empty()))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_INVALID, packet.NewGuildName);
        return;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PETITION_NAME);
    stmt->setString(0, packet.NewGuildName);
    stmt->setUInt64(1, packet.PetitionGuid.GetCounter());
    CharacterDatabase.Execute(stmt);

    WorldPackets::Petition::PetitionRenameGuildResponse renameResponse;
    renameResponse.PetitionGuid = packet.PetitionGuid;
    renameResponse.NewGuildName = packet.NewGuildName;
    SendPacket(renameResponse.Write());
}

void WorldSession::HandleSignPetition(WorldPackets::Petition::SignPetition& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    ObjectGuid playerGUID = player->GetGUID();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURES);
    stmt->setUInt64(0, packet.PetitionGUID.GetCounter());
    stmt->setUInt64(1, packet.PetitionGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    Field* fields = result->Fetch();
    ObjectGuid ownerGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
    uint64 signs = fields[1].GetUInt64();
    if (playerGUID == ownerGuid)
        return;

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != sObjectMgr->GetPlayerTeamByGUID(ownerGuid))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (player->GetGuildId())
    {
        Guild::SendCommandResult(this, GUILD_INVITE_S, ERR_ALREADY_IN_GUILD_S, player->GetName());
        return;
    }

    if (player->GetGuildIdInvited())
    {
        Guild::SendCommandResult(this, GUILD_INVITE_S, ERR_ALREADY_INVITED_TO_GUILD_S, player->GetName());
        return;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIG_BY_ACCOUNT);
    stmt->setUInt32(0, GetAccountId());
    stmt->setUInt64(1, packet.PetitionGUID.GetCounter());
    result = CharacterDatabase.Query(stmt);
    if (result)
    {
        SendPetitionSignResult(playerGUID, packet.PetitionGUID, PETITION_SIGN_ALREADY_SIGNED);
        return;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PETITION_SIGNATURE);
    stmt->setUInt64(0, ownerGuid.GetCounter());
    stmt->setUInt64(1, packet.PetitionGUID.GetCounter());
    stmt->setUInt64(2, playerGUID.GetCounter());
    stmt->setUInt32(3, GetAccountId());
    CharacterDatabase.Execute(stmt);

    SendPetitionSignResult(playerGUID, packet.PetitionGUID, PETITION_SIGN_OK);

    if (Player* owner = ObjectAccessor::FindPlayer(ownerGuid))
    {
        if (Item* item = owner->GetItemByGuid(packet.PetitionGUID))
            item->SetUInt32Value(ITEM_FIELD_ENCHANTMENT + 1, signs);

        owner->GetSession()->SendPetitionSignResult(playerGUID, packet.PetitionGUID, PETITION_SIGN_OK);
    }
}

void WorldSession::SendPetitionSignResult(ObjectGuid const& playerGuid, ObjectGuid const& petitionGuid, uint8 result)
{
    WorldPackets::Petition::PetitionSignResults signResult;
    signResult.Item = petitionGuid;
    signResult.Player = playerGuid;
    signResult.Error = result;
    SendPacket(signResult.Write());
}

void WorldSession::HandleDeclinePetition(WorldPackets::Petition::DeclinePetition& packet)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_OWNER_BY_GUID);
    stmt->setUInt64(0, packet.PetitionGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    //Player* owner = ObjectAccessor::FindConnectedPlayer(ObjectGuid::Create<HighGuid::Player>(result->Fetch()[0].GetUInt64()));
    //if (owner)
    //{
    //   Disabled because packet isn't handled by the client in any way
    //    WorldPackets::Petition::PetitionDeclined packet;
    //    packet.Decliner = _player->GetGUID();
    //    owner->SendDirectMessage(packet.Write());
    //}
}

void WorldSession::HandleOfferPetition(WorldPackets::Petition::OfferPetition& packet)
{
    Player* player = ObjectAccessor::FindPlayer(packet.TargetPlayer);
    if (!player)
        return;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_TYPE);
    stmt->setUInt64(0, packet.ItemGUID.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (player->GetGuildId())
    {
        Guild::SendCommandResult(this, GUILD_INVITE_S, ERR_ALREADY_IN_GUILD_S, _player->GetName());
        return;
    }

    if (player->GetGuildIdInvited())
    {
        Guild::SendCommandResult(this, GUILD_INVITE_S, ERR_ALREADY_INVITED_TO_GUILD_S, _player->GetName());
        return;
    }

    uint8 signs = 0;

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);
    stmt->setUInt64(0, packet.ItemGUID.GetCounter());

    result = CharacterDatabase.Query(stmt);
    if (result)
        signs = uint8(result->GetRowCount());

    WorldPackets::Petition::ServerPetitionShowSignatures signaturesPacket;
    signaturesPacket.Item = packet.ItemGUID;
    signaturesPacket.Owner = _player->GetGUID();
    signaturesPacket.OwnerAccountID = ObjectGuid::Create<HighGuid::WowAccount>(player->GetSession()->GetAccountId());
    signaturesPacket.PetitionID = int32(packet.ItemGUID.GetCounter());  // @todo verify that...

    signaturesPacket.Signatures.reserve(signs);
    for (uint8 i = 0; i < signs; ++i)
    {
        Field* fields2 = result->Fetch();
        ObjectGuid signerGUID = ObjectGuid::Create<HighGuid::Player>(fields2[0].GetUInt64());

        WorldPackets::Petition::ServerPetitionShowSignatures::PetitionSignature signature;
        signature.Signer = signerGUID;
        signature.Choice = 0;
        signaturesPacket.Signatures.push_back(signature);

        if (!result->NextRow())
            break;
    }

    player->SendDirectMessage(signaturesPacket.Write());
}

void WorldSession::HandlePetitionShowList(WorldPackets::Petition::PetitionShowList& packet)
{
    SendPetitionShowList(packet.PetitionUnit);
}

void WorldSession::SendPetitionShowList(ObjectGuid guid)
{
    Creature* creature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_PETITIONER);
    if (!creature)
        return;

    if (!creature->isTabardDesigner())

        return;

    WorldPackets::Petition::ServerPetitionShowList packet;
    packet.Unit = guid;
    packet.Price = GUILD_CHARTER_COST;
    SendPacket(packet.Write());
}

void WorldSession::HandleTurnInPetition(WorldPackets::Petition::TurnInPetition& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Item* item = player->GetItemByGuid(packet.Item);
    if (!item)
        return;

    ObjectGuid::LowType ownerguidlo;
    std::string name;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION);
    stmt->setUInt64(0, packet.Item.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (result)
    {
        Field* fields = result->Fetch();
        ownerguidlo = fields[0].GetUInt64();
        name = fields[1].GetString();
    }
    else
        return;

    if (player->GetGUIDLow() != ownerguidlo)
        return;

    if (player->GetGuildId())
    {
        player->SendDirectMessage(WorldPackets::Petition::TurnInPetitionResult(int32(PETITION_TURN_ALREADY_IN_GUILD)).Write());
        return;
    }

    if (sGuildMgr->GetGuildByName(name))
    {
        Guild::SendCommandResult(this, GUILD_CREATE_S, ERR_GUILD_NAME_EXISTS_S, name);
        return;
    }

    uint8 signatures = 0;

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PETITION_SIGNATURE);
    stmt->setUInt64(0, packet.Item.GetCounter());
    if (result = CharacterDatabase.Query(stmt))
        signatures = uint8(result->GetRowCount());

    if (signatures < sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS))
    {
        SendPacket(WorldPackets::Petition::TurnInPetitionResult(int32(PETITION_TURN_NEED_MORE_SIGNATURES)).Write());
        return;
    }

    player->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);

    Guild* guild = new Guild;
    if (!guild->Create(player, name))
    {
        delete guild;
        return;
    }

    sGuildMgr->AddGuild(guild);

    for (uint8 i = 0; i < signatures; ++i)
    {
        Field* fields = result->Fetch();
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
        if (Player* member = ObjectAccessor::FindPlayer(guid))
        {
            AddDelayedEvent(10, [guild, guid]() -> void
            {
                if (guild)
                    guild->AddMember(guid);
            });
        }
        else
            guild->AddMember(guid);
        result->NextRow();
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PETITION_BY_GUID);
    stmt->setUInt64(0, packet.Item.GetCounter());
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PETITION_SIGNATURE_BY_GUID);
    stmt->setUInt64(0, packet.Item.GetCounter());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    SendPacket(WorldPackets::Petition::TurnInPetitionResult(int32(PETITION_TURN_OK)).Write());
}
