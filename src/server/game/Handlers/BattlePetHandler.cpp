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

#include "BattlePetPackets.h"
#include "WildBattlePet.h"
#include "CharacterData.h"
#include "GridNotifiers.h"
#include "ObjectVisitors.hpp" 
#include "PlayerDefines.h"
#include "CellImpl.h"
#include "PetBattleSystem.h"
#include "Object.h"
#include "PathGenerator.h"

void WorldSession::HandleBattlePetSummon(WorldPackets::BattlePet::BattlePetGuidRead& packet)
{
    if (_player->IsOnVehicle() || _player->IsSitState())
        return;

    _player->UnsummonCurrentBattlePetIfAny(false);
    if (!_player->GetSummonedBattlePet() || _player->GetSummonedBattlePet()->GetGuidValue(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) != packet.BattlePetGUID)
        _player->SummonBattlePet(packet.BattlePetGUID);
}

void WorldSession::HandleBattlePetNameQuery(WorldPackets::BattlePet::Query& packet)
{
    Creature* creature = Unit::GetCreature(*_player, packet.UnitGUID);
    if (!creature)
        return;

    std::shared_ptr<BattlePet> battlePet = nullptr;
    if (creature->GetOwner() && creature->GetOwner()->IsPlayer())
        battlePet = creature->GetOwner()->ToPlayer()->GetBattlePet(packet.BattlePetID);

    if (!battlePet)
        return;

    bool haveDeclinedNames = false;

    for (auto const& name : battlePet->DeclinedNames)
        if (!name.empty())
        {
            haveDeclinedNames = true;
            break;
        }

    WorldPackets::BattlePet::QueryResponse response;
    response.BattlePetID = packet.BattlePetID;
    response.CreatureID = creature->GetEntry();
    response.Timestamp = creature->GetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP);
    if (creature->GetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP) != 0)
    {
        response.Allow = true;
        response.Name = creature->GetName();
        if (haveDeclinedNames)
        {
            response.HasDeclined = true;
            for (uint32 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                response.DeclinedNames[i] = battlePet->DeclinedNames[i];
        }
    }

    SendPacket(response.Write());
}

void WorldSession::HandleModifyName(WorldPackets::BattlePet::ModifyName& packet)
{
    auto nameInvalidReason = sCharacterDataStore->CheckPetName(packet.Name);
    if (nameInvalidReason != PET_NAME_SUCCESS)
    {
        SendPetNameInvalid(nameInvalidReason, packet.BattlePetGUID, packet.Name, &packet.DeclinedNames);
        return;
    }

    uint32 timeStamp = packet.Name.empty() ? 0 : time(nullptr);

    if (auto battlePet = _player->GetBattlePet(packet.BattlePetGUID))
    {
        battlePet->Name = packet.Name;
        battlePet->NameTimeStamp = timeStamp;
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            battlePet->DeclinedNames[i] = packet.DeclinedNames.name[i];
        battlePet->needSave = true;
    }

    _player->SetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP, timeStamp);

    Creature* creature = _player->GetSummonedBattlePet();
    if (!creature)
        return;

    if (creature->GetGuidValue(UNIT_FIELD_BATTLE_PET_COMPANION_GUID) == packet.BattlePetGUID)
    {
        creature->SetName(packet.Name);
        creature->SetUInt32Value(UNIT_FIELD_BATTLE_PET_COMPANION_NAME_TIMESTAMP, timeStamp);
    }
}

void WorldSession::HandleBattlePetSetFlags(WorldPackets::BattlePet::SetFlags& packet)
{
    if (auto battlePet = _player->GetBattlePet(packet.BattlePetGUID))
    {
        if (battlePet->Flags & packet.Flags)
            battlePet->Flags = battlePet->Flags & ~packet.Flags;
        else
            battlePet->Flags |= packet.Flags;
        battlePet->needSave = true;
    }
}

void WorldSession::HandleCageBattlePet(WorldPackets::BattlePet::BattlePetGuidRead& packet)
{
    // ReSharper disable once CppUnreachableCode
    auto const& battlePet = _player->GetBattlePet(packet.BattlePetGUID);
    if (!battlePet)
        return;

    if (sDB2Manager.HasBattlePetSpeciesFlag(battlePet->Species, BATTLEPET_SPECIES_FLAG_CAGEABLE))
        return;

    ItemPosCountVec dest;
    if (_player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, BATTLE_PET_CAGE_ITEM_ID, 1) != EQUIP_ERR_OK)
        return;

    Item* item = _player->StoreNewItem(dest, BATTLE_PET_CAGE_ITEM_ID, true);
    if (!item)
        return;

    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_SPECIES_ID, battlePet->Species);
    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_BREED_DATA, battlePet->Breed | battlePet->Quality << 24);
    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_LEVEL, battlePet->Level);
    item->SetModifier(ITEM_MODIFIER_BATTLE_PET_DISPLAY_ID, battlePet->DisplayModelID);

    _player->SendNewItem(item, 1, true, true); // FIXME: "You create: ." - item name missing in chat

    SendBattlePetDeleted(packet.BattlePetGUID);
    battlePet->Remove(nullptr);
    _player->_battlePets.erase(packet.BattlePetGUID);
}

void WorldSession::HandleBattlePetSetSlot(WorldPackets::BattlePet::SetBattleSlot& packet)
{
    // TC_LOG_DEBUG(LOG_FILTER_BATTLEPET, "HandleBattlePetSetSlot m_IsPetBattleJournalLocked %u", m_IsPetBattleJournalLocked);

    if (m_IsPetBattleJournalLocked)
        return;

    if (packet.SlotIndex >= MAX_PETBATTLE_SLOTS)
        return;

    if (auto battlePet = _player->GetBattlePet(packet.BattlePetGUID))
    {
        auto petSlots = _player->GetBattlePetCombatTeam();

        for (uint8 i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
        {
            auto& slot = petSlots[i];
            if (slot && slot->Slot == packet.SlotIndex)
            {
                slot->Slot = battlePet->Slot;
                slot->needSave = true;
            }
        }

        battlePet->Slot = packet.SlotIndex;
        battlePet->needSave = true;
    }

    _player->UpdateBattlePetCombatTeam();
    // SendPetBattleSlotUpdates();
}

void WorldSession::HandlePetBattleRequestWild(WorldPackets::BattlePet::RequestWild& packet)
{
    if (!sWorld->getBoolConfig(CONFIG_PET_BATTLES))
        return;

    if (m_IsPetBattleJournalLocked)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_ACCOUNT_LOCK);
        return;
    }

    auto battleRequest = sPetBattleSystem->CreateRequest(_player->GetGUID());
    battleRequest->LocationResult = packet.Battle.Location.LocationResult;
    battleRequest->PetBattleCenterPosition = packet.Battle.Location.BattleOrigin;

    for (auto i = 0; i < MAX_PETBATTLE_TEAM; i++)
        battleRequest->TeamPosition[i] = packet.Battle.Location.PlayerPositions[i];

    battleRequest->RequestType = PETBATTLE_TYPE_PVE;
    battleRequest->OpponentGuid = packet.Battle.TargetGUID;

    auto canEnterResult = sPetBattleSystem->CanPlayerEnterInPetBattle(_player, battleRequest);
    if (canEnterResult != PETBATTLE_REQUEST_OK)
    {
        SendPetBattleRequestFailed(canEnterResult);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    auto wildBattlePetCreature = _player->GetNPCIfCanInteractWith(battleRequest->OpponentGuid, UNIT_NPC_FLAG_WILD_BATTLE_PET);
    if (!wildBattlePetCreature)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    Creature* wildBattlePet = sObjectAccessor->GetCreature(*_player, battleRequest->OpponentGuid);
    if (!wildBattlePet)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    if (!sWildBattlePetMgr->IsWildPet(wildBattlePet))
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    std::shared_ptr<BattlePetInstance> playerPets[MAX_PETBATTLE_SLOTS];
    std::shared_ptr<BattlePetInstance> wildBattlePets[MAX_PETBATTLE_SLOTS];
    size_t playerPetCount = 0;

    for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        playerPets[i] = nullptr;
        wildBattlePets[i] = nullptr;
    }

    auto l_WildBattlePet = sWildBattlePetMgr->GetWildBattlePet(wildBattlePet);
    if (!l_WildBattlePet)
    {
        l_WildBattlePet = nullptr;

        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_NOT_CAPTURABLE);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    wildBattlePets[0] = l_WildBattlePet;

    std::list<Creature*> targets;
    // Trinity::AnyFriendlyCreatureInObjectRangeCheck u_check(wildBattlePet, wildBattlePet, 50.0f);
    // Trinity::CreatureListSearcher<Trinity::AnyFriendlyCreatureInObjectRangeCheck> searcher(wildBattlePet, targets, u_check);
    // Trinity::VisitNearbyObject(wildBattlePet, 40.0f, searcher);

    uint32 wildsPetCount = 1;
    for (auto current : targets)
    {
        if (wildsPetCount >= MAX_PETBATTLE_SLOTS)
            break;

        if (!current->isAlive() || current->GetGUID() == wildBattlePet->GetGUID() || !sWildBattlePetMgr->IsWildPet(current))
            continue;

        if (sWildBattlePetMgr->GetWildBattlePet(current) != nullptr && roll_chance_i(80))
        {
            wildBattlePets[wildsPetCount] = sWildBattlePetMgr->GetWildBattlePet(current);
            wildBattlePets[wildsPetCount]->OriginalCreature = current->GetGUID();
            wildsPetCount++;
        }
    }

    _player->UpdateBattlePetCombatTeam();
    auto petSlots = _player->GetBattlePetCombatTeam();

    for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        if (!petSlots[i])
            continue;

        if (playerPetCount >= MAX_PETBATTLE_SLOTS || playerPetCount >= _player->GetUnlockedPetBattleSlot())
            break;

        playerPets[playerPetCount] = std::make_shared<BattlePetInstance>();
        playerPets[playerPetCount]->CloneFrom(petSlots[i]);
        playerPets[playerPetCount]->Slot = playerPetCount;
        playerPets[playerPetCount]->OriginalBattlePet = petSlots[i];

        ++playerPetCount;
    }

    _player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); ///< Immuned only to NPC
    _player->SetTarget(wildBattlePetCreature->GetGUID());

    SendPetBattleFinalizeLocation(battleRequest);

    _player->SetFacingTo(_player->GetAngle(&battleRequest->TeamPosition[PETBATTLE_TEAM_2]));
    _player->SetRooted(true);

    auto battle = sPetBattleSystem->CreateBattle();

    battle->Teams[PETBATTLE_TEAM_1]->OwnerGuid = _player->GetGUID();
    battle->Teams[PETBATTLE_TEAM_1]->PlayerGuid = _player->GetGUID();

    battle->Teams[PETBATTLE_TEAM_2]->OwnerGuid = wildBattlePet->GetGUID();

    for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        if (playerPets[i])
            battle->AddPet(PETBATTLE_TEAM_1, playerPets[i]);

        if (wildBattlePets[i])
        {
            battle->AddPet(PETBATTLE_TEAM_2, wildBattlePets[i]);

            if (auto currrentCreature = sObjectAccessor->GetCreature(*_player, wildBattlePets[i]->OriginalCreature))
            {
                currrentCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_PC); // Immuned to all
                currrentCreature->SetTarget(_player->GetGUID());
                currrentCreature->SetControlled(true, UNIT_STATE_ROOT);
                currrentCreature->_petBattleId = battle->ID;
                sWildBattlePetMgr->EnterInBattle(currrentCreature);
            }
        }
    }

    battle->BattleType = battleRequest->RequestType;
    battle->PveBattleType = PVE_PETBATTLE_WILD;

    _player->_petBattleId = battle->ID;
    battle->Begin();

    sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);

    for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        if (playerPets[i])
            playerPets[i] = nullptr;

        if (wildBattlePets[i])
            wildBattlePets[i] = nullptr;
    }

    l_WildBattlePet = nullptr;
}

void WorldSession::HandleReplaceFrontPet(WorldPackets::BattlePet::ReplaceFrontPet& packet)
{
    if (!_player->_petBattleId)
    {
        SendPetBattleFinished();
        return;
    }

    PetBattle* petBattle = sPetBattleSystem->GetBattle(_player->_petBattleId);
    if (!petBattle || petBattle->BattleStatus == PETBATTLE_STATUS_FINISHED)
    {
        SendPetBattleFinished();
        return;
    }

    uint32 playerTeamID = 0;
    if (petBattle->Teams[PETBATTLE_TEAM_2]->PlayerGuid == _player->GetGUID())
        playerTeamID = PETBATTLE_TEAM_2;

    if (petBattle->Teams[playerTeamID]->Ready)
        return;

    packet.FrontPet = (playerTeamID == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS : 0) + packet.FrontPet;

    if (!petBattle->Teams[playerTeamID]->CanSwap(packet.FrontPet))
        return;

    petBattle->SwapPet(playerTeamID, packet.FrontPet);
    petBattle->SwapPet(!playerTeamID, petBattle->Teams[!playerTeamID]->ActivePetID);
}

void WorldSession::HandlePetBattleRequestUpdate(WorldPackets::BattlePet::RequestUpdate& packet)
{
    auto battleRequest = sPetBattleSystem->GetRequest(packet.TargetGUID);
    auto opposant = ObjectAccessor::FindPlayer(packet.TargetGUID);

    if (!packet.Canceled && battleRequest && opposant)
    {
        _player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); // Immuned only to NPC
        opposant->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_IMMUNE_TO_NPC); // Immuned only to NPC

        std::shared_ptr<BattlePetInstance> playerPets[MAX_PETBATTLE_SLOTS];
        std::shared_ptr<BattlePetInstance> playerOpposantPets[MAX_PETBATTLE_SLOTS];
        size_t playerPetCount = 0;
        size_t playerOpposantPetCount = 0;

        for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
        {
            playerPets[i] = nullptr;
            playerOpposantPets[i] = nullptr;
        }

        _player->UpdateBattlePetCombatTeam();
        auto petSlots = _player->GetBattlePetCombatTeam();

        for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
        {
            if (!petSlots[i])
                continue;

            if (playerPetCount >= MAX_PETBATTLE_SLOTS || playerPetCount >= _player->GetUnlockedPetBattleSlot())
                break;

            playerPets[playerPetCount] = std::make_shared<BattlePetInstance>();
            playerPets[playerPetCount]->CloneFrom(petSlots[i]);
            playerPets[playerPetCount]->Slot = playerPetCount;
            playerPets[playerPetCount]->OriginalBattlePet = petSlots[i];

            ++playerPetCount;
        }

        opposant->UpdateBattlePetCombatTeam();
        auto petOpposantSlots = opposant->GetBattlePetCombatTeam();

        for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
        {
            if (!petOpposantSlots[i])
                continue;

            if (playerOpposantPetCount >= MAX_PETBATTLE_SLOTS || playerOpposantPetCount >= _player->GetUnlockedPetBattleSlot())
                break;

            playerOpposantPets[playerOpposantPetCount] = std::make_shared<BattlePetInstance>();
            playerOpposantPets[playerOpposantPetCount]->CloneFrom(petOpposantSlots[i]);
            playerOpposantPets[playerOpposantPetCount]->Slot = playerOpposantPetCount;
            playerOpposantPets[playerOpposantPetCount]->OriginalBattlePet = petOpposantSlots[i];

            ++playerOpposantPetCount;
        }

        if (!playerOpposantPetCount || !playerPetCount)
        {
            _player->GetSession()->SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_PETS_IN_SLOT);
            opposant->GetSession()->SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_PETS_IN_SLOT);
            sPetBattleSystem->RemoveRequest(packet.TargetGUID);
            return;
        }

        _player->GetSession()->SendPetBattleFinalizeLocation(battleRequest);
        opposant->GetSession()->SendPetBattleFinalizeLocation(battleRequest);

        _player->SetFacingTo(_player->GetAngle(&battleRequest->TeamPosition[PETBATTLE_TEAM_1]));
        opposant->SetFacingTo(_player->GetAngle(&battleRequest->TeamPosition[PETBATTLE_TEAM_2]));
        _player->SetRooted(true);
        opposant->SetRooted(true);

        auto battle = sPetBattleSystem->CreateBattle();

        battle->Teams[PETBATTLE_TEAM_1]->OwnerGuid = opposant->GetGUID();
        battle->Teams[PETBATTLE_TEAM_1]->PlayerGuid = opposant->GetGUID();
        battle->Teams[PETBATTLE_TEAM_2]->OwnerGuid = _player->GetGUID();
        battle->Teams[PETBATTLE_TEAM_2]->PlayerGuid = _player->GetGUID();

        for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
        {
            if (playerOpposantPets[i])
                battle->AddPet(PETBATTLE_TEAM_1, playerOpposantPets[i]);

            if (playerPets[i])
                battle->AddPet(PETBATTLE_TEAM_2, playerPets[i]);
        }

        battle->BattleType = battleRequest->RequestType;

        // Launch battle
        _player->_petBattleId = battle->ID;
        opposant->_petBattleId = battle->ID;
        battle->Begin();

        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);

        for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
        {
            if (playerPets[i])
                playerPets[i] = std::shared_ptr<BattlePetInstance>();

            if (playerOpposantPets[i])
                playerOpposantPets[i] = std::shared_ptr<BattlePetInstance>();
        }
    }
    else
    {
        if (opposant)
            opposant->GetSession()->SendPetBattleRequestFailed(PETBATTLE_REQUEST_DECLINED);
        sPetBattleSystem->RemoveRequest(packet.TargetGUID);
    }
}

enum ePetBattleActions
{
    PETBATTLE_ACTION_REQUEST_LEAVE = 0,
    PETBATTLE_ACTION_CAST = 1,
    PETBATTLE_ACTION_SWAP_OR_PASS = 2,
    PETBATTLE_ACTION_CATCH = 3,
    PETBATTLE_ACTION_LEAVE_PETBATTLE = 4
};

void WorldSession::HandlePetBattleInput(WorldPackets::BattlePet::PetBattleInput& packet)
{
    if (packet.MoveType == PETBATTLE_ACTION_LEAVE_PETBATTLE)
    {
        SendPetBattleFinished();
        return;
    }

    if (!_player->_petBattleId)
    {
        SendPetBattleFinished();
        return;
    }

    auto petBattle = sPetBattleSystem->GetBattle(_player->_petBattleId);
    if (!petBattle || petBattle->BattleStatus == PETBATTLE_STATUS_FINISHED)
    {
        SendPetBattleFinished();
        return;
    }

    if (packet.Round + 1 != petBattle->Turn)
    {
        sPetBattleSystem->ForfeitBattle(petBattle->ID, _player->GetGUID(), packet.IgnoreAbandonPenalty);
        return;
    }

    if (!packet.MoveType)
        return;

    uint32 playerTeamID = 0;
    if (petBattle->Teams[PETBATTLE_TEAM_2]->PlayerGuid == _player->GetGUID())
        playerTeamID = PETBATTLE_TEAM_2;

    auto& battleTeam = petBattle->Teams[playerTeamID];

    if (petBattle->BattleType == PETBATTLE_TYPE_PVE)
    {
        petBattle->Teams[PETBATTLE_TEAM_1]->isRun = true;
        petBattle->Teams[PETBATTLE_TEAM_2]->isRun = true;
    }
    else
        battleTeam->isRun = true;

    if (battleTeam->Ready)
        return;

    switch (packet.MoveType)
    {
        case PETBATTLE_ACTION_REQUEST_LEAVE:
            sPetBattleSystem->ForfeitBattle(petBattle->ID, _player->GetGUID(), packet.IgnoreAbandonPenalty);
            break;
        case PETBATTLE_ACTION_CAST:
            if (petBattle->CanCast(playerTeamID, packet.AbilityID))
                petBattle->PrepareCast(playerTeamID, packet.AbilityID);
            break;
        case PETBATTLE_ACTION_CATCH:
            if (battleTeam->CanCatchOpponentTeamFrontPet() == PETBATTLE_TEAM_CATCH_FLAG_ENABLE_TRAP)
                petBattle->PrepareCast(playerTeamID, battleTeam->GetCatchAbilityID());
            break;
        case PETBATTLE_ACTION_SWAP_OR_PASS:
        {
            packet.NewFrontPet = (playerTeamID == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS : 0) + packet.NewFrontPet;

            if (!battleTeam->CanSwap(packet.NewFrontPet))
                return;

            petBattle->SwapPet(playerTeamID, packet.NewFrontPet);
            break;
        }
        default:
            break;
    }
}

void WorldSession::HandlePetBattleFinalNotify(WorldPackets::BattlePet::NullCmsg& /*packet*/)
{ }

void WorldSession::HandlePetBattleQuitNotify(WorldPackets::BattlePet::NullCmsg& /*packet*/)
{ }

void WorldSession::HandleBattlePetDelete(WorldPackets::BattlePet::BattlePetGuidRead& packet)
{
    auto battlePet = _player->GetBattlePet(packet.BattlePetGUID);
    if (!battlePet)
        return;

    if (sDB2Manager.HasBattlePetSpeciesFlag(battlePet->Species, BATTLEPET_SPECIES_FLAG_RELEASABLE))
        return;

    SendBattlePetDeleted(packet.BattlePetGUID);
    battlePet->Remove(nullptr);
    _player->_battlePets.erase(packet.BattlePetGUID);
}

void WorldSession::HandleBattlePetRequestJournal(WorldPackets::BattlePet::NullCmsg& /*packet*/)
{
    SendBattlePetJournal();
}

void WorldSession::HandleBattlePetJournalLock(WorldPackets::BattlePet::NullCmsg& /*packet*/)
{
    // if (m_IsPetBattleJournalLocked)
        // SendBattlePetJournalLockAcquired();
    // else
        // SendBattlePetJournalLockDenied();
}

void WorldSession::HandleJoinPetBattleQueue(WorldPackets::BattlePet::NullCmsg& /*packet*/)
{
    if (!sWorld->getBoolConfig(CONFIG_PET_BATTLES))
        return;

    if (_player->_petBattleId)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_IN_BATTLE);
        return;
    }

    if (_player->isInCombat())
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT);
        return;
    }

    std::shared_ptr<BattlePetInstance> playerPets[MAX_PETBATTLE_SLOTS];
    size_t playerPetCount = 0;

    // Temporary pet buffer
    for (auto& playerPet : playerPets)
        playerPet = std::shared_ptr<BattlePetInstance>();

    // Load player pets
    _player->UpdateBattlePetCombatTeam();
    auto petSlots = _player->GetBattlePetCombatTeam();
    uint32 deadPetCount = 0;

    for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
    {
        if (!petSlots[i])
            continue;

        if (playerPetCount >= MAX_PETBATTLE_SLOTS || playerPetCount >= _player->GetUnlockedPetBattleSlot())
            break;

        if (petSlots[i]->Health == 0)
            deadPetCount++;

        playerPets[playerPetCount] = std::make_shared<BattlePetInstance>();
        playerPets[playerPetCount]->CloneFrom(petSlots[i]);
        playerPets[playerPetCount]->Slot = playerPetCount;
        playerPets[playerPetCount]->OriginalBattlePet = petSlots[i];

        ++playerPetCount;
    }

    if (deadPetCount && deadPetCount == playerPetCount)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_ALL_PETS_DEAD);
        return;
    }

    if (!playerPetCount)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NO_PETS_IN_SLOT);
        return;
    }

    sPetBattleSystem->JoinQueue(_player);
}

void WorldSession::HandlePetBattleScriptErrorNotify(WorldPackets::BattlePet::NullCmsg& /*packet*/)
{ }

void WorldSession::HandleBattlePetDeletePetCheat(WorldPackets::BattlePet::BattlePetGuidRead& /*packet*/)
{ }

void WorldSession::HandlePetBattleRequestPVP(WorldPackets::BattlePet::RequestPVP& packet)
{
    if (!sWorld->getBoolConfig(CONFIG_PET_BATTLES))
        return;

    auto battleRequest = sPetBattleSystem->CreateRequest(_player->GetGUID());

    battleRequest->LocationResult = packet.Battle.Location.LocationResult;
    battleRequest->PetBattleCenterPosition = packet.Battle.Location.BattleOrigin;

    for (auto i = 0; i < MAX_PETBATTLE_TEAM; i++)
        battleRequest->TeamPosition[i] = packet.Battle.Location.PlayerPositions[i];

    battleRequest->RequestType = PETBATTLE_TYPE_PVP_DUEL;
    battleRequest->OpponentGuid = packet.Battle.TargetGUID;

    if (_player->_petBattleId)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_IN_BATTLE);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    if (_player->isInCombat())
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    for (auto const& teamPosition : battleRequest->TeamPosition)
    {
        if (_player->GetMap()->getObjectHitPos(_player->GetPhases(), true, battleRequest->PetBattleCenterPosition, teamPosition, 0.0f))
        {
            SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_HERE_UNEVEN_GROUND);
            sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
            return;
        }
    }

    auto opposant = ObjectAccessor::FindPlayer(packet.Battle.TargetGUID);
    if (!opposant)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_TARGET_INVALID);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    if (opposant->_petBattleId)
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_IN_BATTLE);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    if (opposant->isInCombat())
    {
        SendPetBattleRequestFailed(PETBATTLE_REQUEST_NOT_WHILE_IN_COMBAT);
        sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
        return;
    }

    battleRequest->IsPvPReady[PETBATTLE_TEAM_1] = true;
    opposant->GetSession()->SendPetBattlePvPChallenge(battleRequest);
}

void WorldSession::HanldeQueueProposeMatchResult(WorldPackets::BattlePet::QueueProposeMatchResult& packet)
{
    if (!sWorld->getBoolConfig(CONFIG_PET_BATTLES))
        return;

    sPetBattleSystem->ProposalResponse(_player, packet.Accepted);
}

void WorldSession::HandleLeaveQueue(WorldPackets::BattlePet::LeaveQueue& /*packet*/)
{
    sPetBattleSystem->LeaveQueue(_player);
}

void WorldSession::SendPetBattleSlotUpdates(bool newSlotUnlocked /*= false*/)
{
    // TC_LOG_DEBUG(LOG_FILTER_BATTLEPET, "SendPetBattleSlotUpdates");

    auto unlockedSlotCount = _player->GetUnlockedPetBattleSlot();
    auto petSlots = _player->GetBattlePetCombatTeam();

    if (unlockedSlotCount)
        _player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_PET_BATTLES_UNLOCKED);

    WorldPackets::BattlePet::SlotUpdates updates;
    updates.AutoSlotted = true;
    updates.NewSlotUnlocked = newSlotUnlocked;

    for (uint32 i = 0; i < MAX_PETBATTLE_SLOTS; i++)
    {
        WorldPackets::BattlePet::BattlePetSlot slot;
        if (petSlots[i])
            slot.Pet.BattlePetGUID = petSlots[i]->JournalID;
        slot.DisplayID = 0;
        slot.SlotIndex = i;
        slot.Locked = !(i + 1 <= unlockedSlotCount);
        updates.Slots.emplace_back(slot);
    }

    SendPacket(updates.Write());
}

void WorldSession::SendPetBattleRequestFailed(uint8 reason)
{
    SendPacket(WorldPackets::BattlePet::RequestFailed(reason).Write());
}

void WorldSession::SendPetBattlePvPChallenge(PetBattleRequest* petBattleRequest)
{
    WorldPackets::BattlePet::PVPChallenge challengeUpdate;
    challengeUpdate.ChallengerGUID = petBattleRequest->RequesterGuid;
    challengeUpdate.Location.BattleOrigin = petBattleRequest->PetBattleCenterPosition;
    challengeUpdate.Location.LocationResult = petBattleRequest->LocationResult;
    for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; i++)
        challengeUpdate.Location.PlayerPositions[i] = petBattleRequest->TeamPosition[i];

    SendPacket(challengeUpdate.Write());
}

void WorldSession::SendPetBattleFinalizeLocation(PetBattleRequest* petBattleRequest)
{
    WorldPackets::BattlePet::FinalizeLocation locationUpdate;
    locationUpdate.Location.BattleOrigin = petBattleRequest->PetBattleCenterPosition;
    locationUpdate.Location.LocationResult = petBattleRequest->LocationResult;
    for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; i++)
        locationUpdate.Location.PlayerPositions[i] = petBattleRequest->TeamPosition[i];
    SendPacket(locationUpdate.Write());
}

void WorldSession::SendPetBattleInitialUpdate(PetBattle* petBattle)
{
    if (petBattle->BattleType == PETBATTLE_TYPE_PVE)
        petBattle->InitialWildPetGUID = petBattle->Teams[PETBATTLE_PVE_TEAM_ID]->OwnerGuid;

    WorldPackets::BattlePet::PetBattleInitialUpdate update;
    uint16 waitingForFrontPetsMaxSecs = 30;
    uint16 pvpMaxRoundTime = 30;
    uint8 curPetBattleState = 1;
    bool isPVP = petBattle->BattleType != PETBATTLE_TYPE_PVE;

    if (petBattle->BattleType == PETBATTLE_TYPE_PVE && petBattle->PveBattleType == PVE_PETBATTLE_TRAINER)
    {
        if (auto trainer = ObjectAccessor::GetObjectInOrOutOfWorld(petBattle->InitialWildPetGUID, static_cast<Creature*>(nullptr)))
        {
            update.MsgData.NpcCreatureID = trainer->GetEntry();
            update.MsgData.NpcDisplayID = trainer->GetDisplayId();
        }
    }

    //PetBattleEnviroUpdate Enviros[3] = {};
    update.MsgData.InitialWildPetGUID = petBattle->InitialWildPetGUID;
    update.MsgData.CurRound = petBattle->Turn;
    update.MsgData.WaitingForFrontPetsMaxSecs = waitingForFrontPetsMaxSecs;
    update.MsgData.PvpMaxRoundTime = pvpMaxRoundTime;
    update.MsgData.ForfeitPenalty = petBattle->GetForfeitHealthPenalityPct();
    update.MsgData.CurPetBattleState = curPetBattleState;
    update.MsgData.IsPVP = isPVP;
    update.MsgData.CanAwardXP = petBattle->BattleType != PETBATTLE_TYPE_PVP_DUEL;

    for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; i++)
    {
        auto ownerGuid = petBattle->Teams[i]->OwnerGuid;
        if (petBattle->BattleType == PETBATTLE_TYPE_PVE && i == PETBATTLE_PVE_TEAM_ID)
            ownerGuid.Clear();

        WorldPackets::BattlePet::PetBattlePlayerUpdate playerUpdate;
        playerUpdate.CharacterID = ownerGuid;
        playerUpdate.TrapAbilityID = petBattle->Teams[i]->GetCatchAbilityID();
        playerUpdate.TrapStatus = i == PETBATTLE_TEAM_1 ? 5 : 2;
        playerUpdate.RoundTimeSecs = isPVP ? pvpMaxRoundTime : 0;
        playerUpdate.InputFlags = PETBATTLE_TEAM_INPUT_FLAG_LOCK_PET_SWAP | PETBATTLE_TEAM_INPUT_FLAG_LOCK_ABILITIES_2;

        if (i == PETBATTLE_TEAM_1 || petBattle->Teams[i]->ActivePetID == PETBATTLE_NULL_ID)
            playerUpdate.FrontPet = int8(petBattle->Teams[i]->ActivePetID);
        else
            playerUpdate.FrontPet = int8(petBattle->Teams[i]->ActivePetID - (i == PETBATTLE_TEAM_2 ? MAX_PETBATTLE_SLOTS : 0));

        for (uint8 v = 0; v < petBattle->Teams[i]->TeamPetCount; v++)
        {
            auto pet = petBattle->Teams[i]->TeamPets[v];

            WorldPackets::BattlePet::PetBattlePetUpdate petUpdate;
            petUpdate.NpcTeamMemberID = 0;
            petUpdate.StatusFlags = 0;
            petUpdate.Slot = v;
            //std::vector<BattlePetAura> Auras;

            petUpdate.JournalInfo.BattlePetGUID = petBattle->BattleType == PETBATTLE_TYPE_PVE && i == PETBATTLE_PVE_TEAM_ID ? ObjectGuid::Empty : pet->JournalID;
            petUpdate.JournalInfo.SpeciesID = pet->Species;
            if (auto const& speciesInfo = sBattlePetSpeciesStore.LookupEntry(pet->Species))
                petUpdate.JournalInfo.CreatureID = ownerGuid ? 0 : speciesInfo->CreatureID;
            petUpdate.JournalInfo.DisplayID = ownerGuid ? 0 : pet->DisplayModelID;
            petUpdate.JournalInfo.BreedID = pet->Breed;
            petUpdate.JournalInfo.Level = pet->Level;
            petUpdate.JournalInfo.Xp = pet->XP;
            petUpdate.JournalInfo.BattlePetDBFlags = pet->Flags & ~PETBATTLE_FLAG_CAPTURED;
            petUpdate.JournalInfo.Power = pet->InfoPower;

            petUpdate.JournalInfo.Health = pet->Health;
            petUpdate.JournalInfo.MaxHealth = pet->InfoMaxHealth;
            petUpdate.JournalInfo.Speed = pet->InfoSpeed;
            petUpdate.JournalInfo.BreedQuality = pet->Quality;
            petUpdate.JournalInfo.CustomName = pet->Name;
            //petUpdate.JournalInfo.OwnerGuid;
            //petUpdate.JournalInfo.NoRename = false;

            for (uint8 slot = 0; slot < MAX_PETBATTLE_ABILITIES; slot++)
            {
                if (pet->Abilities[slot])
                {
                    WorldPackets::BattlePet::BattlePetAbility abilityUpdate;
                    abilityUpdate.AbilityID = pet->Abilities[slot];
                    abilityUpdate.CooldownRemaining = pet->Cooldowns[slot] != -1 ? pet->Cooldowns[slot] : 0; ///< Sending cooldown at -1 make client disable it
                    abilityUpdate.LockdownRemaining = pet->Lockdowns[slot];
                    abilityUpdate.AbilityIndex = slot;
                    abilityUpdate.Pboid = pet->ID;
                    petUpdate.Abilities.emplace_back(abilityUpdate);
                }
            }

            for (uint32 stateIdx = 0; stateIdx < NUM_BATTLEPET_STATES; ++stateIdx)
            {
                switch (stateIdx)
                {
                case BATTLEPET_STATE_Stat_Power:
                case BATTLEPET_STATE_Stat_Stamina:
                case BATTLEPET_STATE_Stat_Speed:
                case BATTLEPET_STATE_Stat_CritChance:
                case BATTLEPET_STATE_Stat_Accuracy:
                    petUpdate.States[stateIdx] = pet->States[stateIdx];
                    break;
                default:
                    break;
                }
            }

            playerUpdate.Pets.emplace_back(petUpdate);
        }


        update.MsgData.Players[i] = playerUpdate;
    }

    SendPacket(update.Write());
}

void WorldSession::SendPetBattleFirstRound(PetBattle* petBattle)
{
    // TC_LOG_DEBUG(LOG_FILTER_BATTLEPET, "SendPetBattleFirstRound");

    auto isPVP = petBattle->BattleType != PETBATTLE_TYPE_PVE;
    uint16 pvpMaxRoundTime = isPVP ? 30 : 0;

    WorldPackets::BattlePet::BattleRound firstRound(SMSG_PET_BATTLE_FIRST_ROUND);
    firstRound.MsgData.CurRound = petBattle->Turn;
    firstRound.MsgData.NextPetBattleState = petBattle->RoundResult;

    for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; i++)
    {
        firstRound.MsgData.NextInputFlags[i] = petBattle->Teams[i]->GetTeamInputFlags();
        firstRound.MsgData.NextTrapStatus[i] = petBattle->Teams[i]->GetTeamTrapStatus();
        firstRound.MsgData.RoundTimeSecs[i] = pvpMaxRoundTime;
    }

    firstRound.MsgData.PetXDied = petBattle->PetXDied;

    for (auto const& pet : petBattle->Pets)
    {
        if (!pet)
            continue;

        for (uint8 s = 0; s < MAX_PETBATTLE_ABILITIES; s++)
        {
            if (pet->Cooldowns[s] != -1 || pet->Lockdowns[s] != 0)
            {
                WorldPackets::BattlePet::BattlePetAbility abilityUpdate;
                abilityUpdate.AbilityID = pet->Abilities[s];
                abilityUpdate.CooldownRemaining = pet->Cooldowns[s];
                abilityUpdate.LockdownRemaining = pet->Lockdowns[s];
                abilityUpdate.AbilityIndex = s;
                abilityUpdate.Pboid = pet->ID;
                firstRound.MsgData.Ability.emplace_back(abilityUpdate);
            }
        }
    }

    for (auto const& eventIntr : petBattle->RoundEvents)
    {
        WorldPackets::BattlePet::Effect effectUpdate;
        effectUpdate.AbilityEffectID = eventIntr.AbilityEffectID;
        effectUpdate.Flags = eventIntr.Flags;
        effectUpdate.SourceAuraInstanceID = eventIntr.BuffTurn; ///< Can be swap down
        effectUpdate.TurnInstanceID = eventIntr.RoundTurn; ///< Can be swap up
        effectUpdate.EffectType = eventIntr.EventType;
        effectUpdate.CasterPBOID = eventIntr.SourcePetID;
        effectUpdate.StackDepth = eventIntr.StackDepth;

        for (auto const& update : eventIntr.Updates)
        {
            WorldPackets::BattlePet::PetBattleEffectTarget effectTargetUpdate;
            effectTargetUpdate.Type = update.UpdateType;
            effectTargetUpdate.Petx = update.TargetPetID;

            effectTargetUpdate.Params.Aura.AuraInstanceID = update.Buff.ID;
            effectTargetUpdate.Params.Aura.AuraAbilityID = update.Buff.AbilityID;
            effectTargetUpdate.Params.Aura.RoundsRemaining = update.Buff.Duration;
            effectTargetUpdate.Params.Aura.CurrentRound = update.Buff.Turn;
            effectTargetUpdate.Params.State.StateID = update.State.ID;
            effectTargetUpdate.Params.State.StateValue = update.State.Value;
            effectTargetUpdate.Params.Health = update.Health;
            effectTargetUpdate.Params.NewStatValue = update.Speed;
            effectTargetUpdate.Params.TriggerAbilityID = update.TriggerAbilityId;
            effectTargetUpdate.Params.AbilityChange.ChangedAbilityID = 0;
            effectTargetUpdate.Params.AbilityChange.CooldownRemaining = 0;
            effectTargetUpdate.Params.AbilityChange.LockdownRemaining = 0;
            effectTargetUpdate.Params.BroadcastTextID = update.NpcEmote.BroadcastTextID;

            effectUpdate.EffectTargetData.emplace_back(effectTargetUpdate);
        }

        firstRound.MsgData.EffectData.emplace_back(effectUpdate);
    }

    SendPacket(firstRound.Write());
}

void WorldSession::SendPetBattleRoundResult(PetBattle* petBattle)
{
    // TC_LOG_DEBUG(LOG_FILTER_BATTLEPET, "SendPetBattleRoundResult");

    auto isPVP = petBattle->BattleType != PETBATTLE_TYPE_PVE;
    uint16 pvpMaxRoundTime = isPVP ? 30 : 0;

    WorldPackets::BattlePet::BattleRound roundResult(SMSG_PET_BATTLE_ROUND_RESULT);
    roundResult.MsgData.CurRound = petBattle->Turn;
    roundResult.MsgData.NextPetBattleState = petBattle->RoundResult;

    for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; i++)
    {
        roundResult.MsgData.NextInputFlags[i] = petBattle->Teams[i]->GetTeamInputFlags();
        roundResult.MsgData.NextTrapStatus[i] = petBattle->Teams[i]->GetTeamTrapStatus();
        roundResult.MsgData.RoundTimeSecs[i] = pvpMaxRoundTime;
    }

    roundResult.MsgData.PetXDied = petBattle->PetXDied;

    for (auto const& pet : petBattle->Pets)
    {
        if (!pet)
            continue;

        for (uint8 s = 0; s < MAX_PETBATTLE_ABILITIES; s++)
        {
            if (pet->Cooldowns[s] != -1 || pet->Lockdowns[s] != 0)
            {
                WorldPackets::BattlePet::BattlePetAbility abilityUpdate;
                abilityUpdate.AbilityID = pet->Abilities[s];
                abilityUpdate.CooldownRemaining = pet->Cooldowns[s];
                abilityUpdate.LockdownRemaining = pet->Lockdowns[s];
                abilityUpdate.AbilityIndex = s;
                abilityUpdate.Pboid = pet->ID;
                roundResult.MsgData.Ability.emplace_back(abilityUpdate);
            }
        }
    }

    for (auto const& roundEvent : petBattle->RoundEvents)
    {
        WorldPackets::BattlePet::Effect effectUpdate;

        bool isDead = false;
        for (const auto & update : roundEvent.Updates)
        {
            isDead = update.State.ID == BATTLEPET_STATE_Is_Dead;
            WorldPackets::BattlePet::PetBattleEffectTarget effectTargetUpdate;
            effectTargetUpdate.Type = update.UpdateType;
            effectTargetUpdate.Petx = update.TargetPetID;
            effectTargetUpdate.Params.Aura.AuraInstanceID = update.Buff.ID;
            effectTargetUpdate.Params.Aura.AuraAbilityID = isDead ? 0 : update.Buff.AbilityID;
            effectTargetUpdate.Params.Aura.RoundsRemaining = update.Buff.Duration;
            effectTargetUpdate.Params.Aura.CurrentRound = update.Buff.Turn;
            effectTargetUpdate.Params.State.StateID = update.State.ID;
            effectTargetUpdate.Params.State.StateValue = update.State.Value;
            effectTargetUpdate.Params.Health = update.Health;
            effectTargetUpdate.Params.NewStatValue = update.Speed;
            effectTargetUpdate.Params.TriggerAbilityID = update.TriggerAbilityId;
            effectTargetUpdate.Params.AbilityChange.ChangedAbilityID = 0;
            effectTargetUpdate.Params.AbilityChange.CooldownRemaining = 0;
            effectTargetUpdate.Params.AbilityChange.LockdownRemaining = 0;
            effectTargetUpdate.Params.BroadcastTextID = update.NpcEmote.BroadcastTextID;
            effectUpdate.EffectTargetData.emplace_back(effectTargetUpdate);
        }

        effectUpdate.AbilityEffectID = roundEvent.AbilityEffectID;
        effectUpdate.Flags = isDead ? 0 : roundEvent.Flags;
        effectUpdate.SourceAuraInstanceID = roundEvent.BuffTurn; ///< Can be swap down
        effectUpdate.TurnInstanceID = isDead ? 0 : roundEvent.RoundTurn; ///< Can be swap up
        effectUpdate.EffectType = roundEvent.EventType;
        effectUpdate.CasterPBOID = roundEvent.SourcePetID;
        effectUpdate.StackDepth = roundEvent.StackDepth;

        roundResult.MsgData.EffectData.emplace_back(effectUpdate);
    }

    SendPacket(roundResult.Write());
}

void WorldSession::SendPetBattleReplacementMade(PetBattle* petBattle)
{
    auto isPVP = petBattle->BattleType != PETBATTLE_TYPE_PVE;
    uint16 pvpMaxRoundTime = isPVP ? 30 : 0;

    WorldPackets::BattlePet::BattleRound replacementMade(SMSG_PET_BATTLE_REPLACEMENTS_MADE);
    replacementMade.MsgData.CurRound = petBattle->Turn;
    replacementMade.MsgData.NextPetBattleState = petBattle->RoundResult;

    for (uint8 i = 0; i < MAX_PETBATTLE_TEAM; i++)
    {
        replacementMade.MsgData.NextInputFlags[i] = petBattle->Teams[i]->GetTeamInputFlags();
        replacementMade.MsgData.NextTrapStatus[i] = petBattle->Teams[i]->GetTeamTrapStatus();
        replacementMade.MsgData.RoundTimeSecs[i] = pvpMaxRoundTime;
    }

    replacementMade.MsgData.PetXDied = petBattle->PetXDied;

    for (auto const& pet : petBattle->Pets)
    {
        if (!pet)
            continue;

        for (uint8 s = 0; s < MAX_PETBATTLE_ABILITIES; s++)
        {
            if (pet->Cooldowns[s] != -1 || pet->Lockdowns[s] != 0)
            {
                WorldPackets::BattlePet::BattlePetAbility abilityUpdate;
                abilityUpdate.AbilityID = pet->Abilities[s];
                abilityUpdate.CooldownRemaining = pet->Cooldowns[s];
                abilityUpdate.LockdownRemaining = pet->Lockdowns[s];
                abilityUpdate.AbilityIndex = s;
                abilityUpdate.Pboid = pet->ID;
                replacementMade.MsgData.Ability.emplace_back(abilityUpdate);
            }
        }
    }

    for (auto const& roundEvent : petBattle->RoundEvents)
    {
        WorldPackets::BattlePet::Effect effectUpdate;
        effectUpdate.AbilityEffectID = roundEvent.AbilityEffectID;
        effectUpdate.Flags = roundEvent.Flags;
        effectUpdate.SourceAuraInstanceID = roundEvent.BuffTurn; ///< Can be swap down
        effectUpdate.TurnInstanceID = roundEvent.RoundTurn; ///< Can be swap up
        effectUpdate.EffectType = roundEvent.EventType;
        effectUpdate.CasterPBOID = roundEvent.SourcePetID;
        effectUpdate.StackDepth = roundEvent.StackDepth;

        for (auto const& update : roundEvent.Updates)
        {
            WorldPackets::BattlePet::PetBattleEffectTarget effectTargetUpdate;
            effectTargetUpdate.Type = update.UpdateType;
            effectTargetUpdate.Petx = update.TargetPetID;

            effectTargetUpdate.Params.Aura.AuraInstanceID = update.Buff.ID;
            effectTargetUpdate.Params.Aura.AuraAbilityID = update.Buff.AbilityID;
            effectTargetUpdate.Params.Aura.RoundsRemaining = update.Buff.Duration;
            effectTargetUpdate.Params.Aura.CurrentRound = update.Buff.Turn;
            effectTargetUpdate.Params.State.StateID = update.State.ID;
            effectTargetUpdate.Params.State.StateValue = update.State.Value;
            effectTargetUpdate.Params.Health = update.Health;
            effectTargetUpdate.Params.NewStatValue = update.Speed;
            effectTargetUpdate.Params.TriggerAbilityID = update.TriggerAbilityId;
            effectTargetUpdate.Params.AbilityChange.ChangedAbilityID = 0;
            effectTargetUpdate.Params.AbilityChange.CooldownRemaining = 0;
            effectTargetUpdate.Params.AbilityChange.LockdownRemaining = 0;
            effectTargetUpdate.Params.BroadcastTextID = update.NpcEmote.BroadcastTextID;

            effectUpdate.EffectTargetData.emplace_back(effectTargetUpdate);
        }

        replacementMade.MsgData.EffectData.emplace_back(effectUpdate);
    }

    SendPacket(replacementMade.Write());
}

void WorldSession::SendPetBattleFinalRound(PetBattle* petBattle)
{
    // TC_LOG_DEBUG(LOG_FILTER_BATTLEPET, "SendPetBattleFinalRound");

    WorldPackets::BattlePet::PetBattleFinalRound roundUpdate;
    roundUpdate.MsgData.Abandoned = petBattle->CombatResult == PETBATTLE_RESULT_ABANDON;
    roundUpdate.MsgData.PvpBattle = petBattle->BattleType != PETBATTLE_TYPE_PVE;
    for (uint8 teamID = 0; teamID < MAX_PETBATTLE_TEAM; ++teamID)
    {
        roundUpdate.MsgData.Winner[teamID] = petBattle->WinnerTeamId == teamID;
        roundUpdate.MsgData.NpcCreatureID[teamID] = 0;
    }

    for (auto const& pet : petBattle->Pets)
    {
        if (!pet)
            continue;

        WorldPackets::BattlePet::FinalPet petUpdate;
        petUpdate.Guid = pet->JournalID;
        petUpdate.Level = pet->Level;
        petUpdate.Xp = pet->XP;
        petUpdate.Health = pet->Health;
        petUpdate.MaxHealth = pet->InfoMaxHealth;
        petUpdate.InitialLevel = pet->OldLevel;
        petUpdate.Pboid = pet->ID;
        petUpdate.Captured = pet->Captured;
        petUpdate.Caged = pet->Caged;
        petUpdate.AwardedXP = pet->OldXP != pet->XP;
        petUpdate.SeenAction = false /*bool(petUpdate.Guid)*/;
        roundUpdate.MsgData.Pets.emplace_back(petUpdate);
    }

    SendPacket(roundUpdate.Write());
}

void WorldSession::SendPetBattleFinished()
{
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_PET_BATTLE_FINISHED).Write());
}

void WorldSession::SendPetBattleChatRestricted()
{
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_PET_BATTLE_CHAT_RESTRICTED).Write());
}

void WorldSession::SendPetBattleQueueProposeMatch()
{
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_PET_BATTLE_QUEUE_PROPOSE_MATCH).Write());
}

void WorldSession::SendPetBattleQueueStatus(uint32 ticketTime, uint32 tcketID, uint32 status, uint32 avgWaitTime)
{
    WorldPackets::BattlePet::PetBattleQueueStatus statusUpdate;
    statusUpdate.Msg.Ticket.RequesterGuid = GetBattlenetAccountGUID();
    statusUpdate.Msg.Ticket.Id = tcketID;
    statusUpdate.Msg.Ticket.Type = WorldPackets::LFG::RideType::PvPPetBattle;
    statusUpdate.Msg.Ticket.Time = ticketTime;
    statusUpdate.Msg.Status = status;

    if (status != LFB_LEAVE_QUEUE)
    {
        statusUpdate.Msg.ClientWaitTime = avgWaitTime;
        statusUpdate.Msg.AverageWaitTime = time(nullptr) - ticketTime;
    }

    //statusUpdate.Msg.SlotResult; std::vector<uint32> 

    SendPacket(statusUpdate.Write());
}

void WorldSession::SendBattlePetUpdates(BattlePet* pet2 /*= nullptr*/, bool add)
{
    auto pets = _player->GetBattlePets();
    WorldPackets::BattlePet::Updates update;
    update.AddedPet = add;

    WorldPackets::BattlePet::BattlePetJournalInfo petUpdate;
    if (pet2)
    {
        petUpdate.BattlePetGUID = pet2->JournalID;
        petUpdate.SpeciesID = pet2->Species;
        if (auto const& speciesInfo = sBattlePetSpeciesStore.LookupEntry(pet2->Species))
            petUpdate.CreatureID = speciesInfo->CreatureID;
        petUpdate.DisplayID = pet2->DisplayModelID;
        petUpdate.BreedID = pet2->Breed;
        petUpdate.Level = pet2->Level;
        petUpdate.Xp = pet2->XP;
        petUpdate.BattlePetDBFlags = pet2->Flags;
        petUpdate.Power = pet2->InfoPower;
        petUpdate.Health = pet2->Health > pet2->InfoMaxHealth ? pet2->InfoMaxHealth : pet2->Health;
        petUpdate.MaxHealth = pet2->InfoMaxHealth;
        petUpdate.Speed = pet2->InfoSpeed;
        petUpdate.BreedQuality = pet2->Quality;
        petUpdate.CustomName = pet2->Name;
        //ObjectGuid OwnerGuid;
        //bool NoRename = false;
        update.Pets.emplace_back(petUpdate);
    }
    else
    {
        for (auto const& itr : *pets)
        {
            auto pet = itr.second;

            petUpdate.BattlePetGUID = pet->JournalID;
            petUpdate.SpeciesID = pet->Species;
            if (auto const& speciesInfo = sBattlePetSpeciesStore.LookupEntry(pet->Species))
                petUpdate.CreatureID = speciesInfo->CreatureID;
            petUpdate.DisplayID = pet->DisplayModelID;
            petUpdate.BreedID = pet->Breed;
            petUpdate.Level = pet->Level;
            petUpdate.Xp = pet->XP;
            petUpdate.BattlePetDBFlags = pet->Flags;
            petUpdate.Power = pet->InfoPower;
            petUpdate.Health = pet->Health > pet->InfoMaxHealth ? pet->InfoMaxHealth : pet->Health;
            petUpdate.MaxHealth = pet->InfoMaxHealth;
            petUpdate.Speed = pet->InfoSpeed;
            petUpdate.BreedQuality = pet->Quality;
            petUpdate.CustomName = pet->Name;
            //ObjectGuid OwnerGuid;
            //bool NoRename = false;
            update.Pets.emplace_back(petUpdate);
        }
    }

    SendPacket(update.Write());
}

void WorldSession::SendBattlePetTrapLevel()
{
    SendPacket(WorldPackets::BattlePet::BattlePetTrapLevel(_player->GetBattlePetTrapLevel()).Write());
}

void WorldSession::SendBattlePetJournalLockAcquired()
{
    m_IsPetBattleJournalLocked = true;
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_BATTLE_PET_JOURNAL_LOCK_ACQUIRED).Write());
}

void WorldSession::SendBattlePetJournalLockDenied()
{
    m_IsPetBattleJournalLocked = false;
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_BATTLE_PET_JOURNAL_LOCK_DENIED).Write());
}

void WorldSession::SendBattlePetJournal()
{
    auto pets = _player->GetBattlePets();
    auto unlockedSlotCount = _player->GetUnlockedPetBattleSlot();
    auto petSlots = _player->GetBattlePetCombatTeam();

    if (unlockedSlotCount)
        _player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_PET_BATTLES_UNLOCKED);

    WorldPackets::BattlePet::BattlePetJournal responce;
    responce.NumMaxPets = BATTLE_PET_MAX_JOURNAL_PETS;
    responce.TrapLevel = _player->GetBattlePetTrapLevel();
    responce.HasJournalLock = true;

    for (uint32 i = 0; i < MAX_PETBATTLE_SLOTS; i++)
    {
        WorldPackets::BattlePet::BattlePetSlot slot;
        slot.DisplayID = 0;
        slot.SlotIndex = i;
        slot.Locked = !(i + 1 <= unlockedSlotCount);
        if (petSlots[i])
            slot.Pet.BattlePetGUID = petSlots[i]->JournalID;
        responce.Slots.emplace_back(slot);
    }

    for (auto const& pet : *pets)
    {
        auto v = pet.second;

        WorldPackets::BattlePet::BattlePetJournalInfo info;
        info.BattlePetGUID = v->JournalID;
        info.SpeciesID = v->Species;
        if (auto speciesInfo = sBattlePetSpeciesStore.LookupEntry(v->Species))
            info.CreatureID = speciesInfo->CreatureID;
        info.DisplayID = v->DisplayModelID;
        info.BreedID = v->Breed;
        info.Level = v->Level;
        info.Xp = v->XP;
        info.BattlePetDBFlags = v->Flags;
        info.Power = v->InfoPower;
        info.Health = v->Health > v->InfoMaxHealth ? v->InfoMaxHealth : v->Health;
        info.MaxHealth = v->InfoMaxHealth;
        info.Speed = v->InfoSpeed;
        info.BreedQuality = v->Quality;
        info.CustomName = v->Name;
        //info.OwnerGuid;
        //info.NoRename = false;
        responce.Pets.emplace_back(info);
    }

    SendPacket(responce.Write());
}

void WorldSession::SendBattlePetDeleted(ObjectGuid battlePetGUID)
{
    SendPacket(WorldPackets::BattlePet::BattlePetDeleted(battlePetGUID).Write());
}

void WorldSession::SendBattlePetRevoked(ObjectGuid battlePetGUID)
{
    WorldPackets::BattlePet::GuidData update(SMSG_BATTLE_PET_REVOKED);
    update.BattlePetGUID = battlePetGUID;
    SendPacket(update.Write());
}

void WorldSession::SendBattlePetRestored(ObjectGuid battlePetGUID)
{
    WorldPackets::BattlePet::GuidData update(SMSG_BATTLE_PET_RESTORED);
    update.BattlePetGUID = battlePetGUID;
    SendPacket(update.Write());
}

void WorldSession::SendBattlePetsHealed()
{
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_BATTLE_PETS_HEALED).Write());
}

void WorldSession::SendBattlePetLicenseChanged()
{
    SendPacket(WorldPackets::BattlePet::NullSMsg(SMSG_BATTLE_PET_LICENSE_CHANGED).Write());
}

void WorldSession::SendBattlePetError(BattlePetError result, uint32 creatureID)
{
    WorldPackets::BattlePet::BattlePetError errorUpdate;
    errorUpdate.Result = AsUnderlyingType(result);
    errorUpdate.CreatureID = creatureID;
    SendPacket(errorUpdate.Write());
}

void WorldSession::SendBattlePetCageDateError(uint32 secondsUntilCanCage)
{
    SendPacket(WorldPackets::BattlePet::BattlePetCageDateError(secondsUntilCanCage).Write());
}

void WorldSession::HandleBattlePetUpdateNotify(WorldPackets::BattlePet::BattlePetGuidRead& /*packet*/)
{

}
