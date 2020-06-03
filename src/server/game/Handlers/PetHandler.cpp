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

#include "PetPackets.h"
#include "QueryPackets.h"
#include "CharmInfo.h"
#include "Group.h"
#include "CharacterData.h"
#include "DatabaseEnv.h"
#include "SpellPackets.h"
#include "CreatureAI.h"

void WorldSession::HandleDismissCritter(WorldPackets::PetPackets::DismissCritter& packet)
{
    Unit* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, packet.CritterGUID);
    if (!pet)
        return;

    if (_player->GetCritterGUID() == pet->GetGUID())
    {
        if (pet->IsCreature() && pet->ToCreature()->isSummon())
            pet->ToTempSummon()->UnSummon();
    }
}

void WorldSession::HandlePetAction(WorldPackets::PetPackets::PetAction& packet)
{
    uint32 spellID = UNIT_ACTION_BUTTON_ACTION(packet.Action);
    uint8 flag = UNIT_ACTION_BUTTON_TYPE(packet.Action); //delete = 0x07 CastSpell = C1

    TC_LOG_INFO(LOG_FILTER_NETWORKIO, "HandlePetAction: Pet %u - flag: %u, spellID: %u, target: %u.", packet.PetGUID.GetGUIDLow(), uint32(flag), spellID, packet.TargetGUID.GetGUIDLow());

    if (_player->IsMounted())
        return;

    Unit* pet = ObjectAccessor::GetUnit(*_player, packet.PetGUID);
    if (!pet)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetAction: Pet (GUID: %u) doesn't exist for player '%s'", packet.PetGUID.GetGUIDLow(), GetPlayer()->GetName());
        return;
    }

    if (pet != GetPlayer()->GetFirstControlled())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetAction: Pet (GUID: %u) does not belong to player '%s'", packet.PetGUID.GetGUIDLow(), GetPlayer()->GetName());
        return;
    }

    if (!pet->isAlive())
    {
        SpellInfo const* spell = (flag == ACT_ENABLED || flag == ACT_PASSIVE) ? sSpellMgr->GetSpellInfo(spellID) : nullptr;
        if (!spell)
            return;

        if (!(spell->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD)))
            return;
    }

    if (pet->HasAuraType(SPELL_AURA_DISABLE_AUTO_ATTACK) || pet->HasAuraType(SPELL_AURA_DISABLE_ATTACK_AND_CAST))
        return;

    //TODO: allow control charmed player?
    if (pet->IsPlayer() && !(flag == ACT_COMMAND && spellID == COMMAND_ATTACK))
        return;

    if (GetPlayer()->m_Controlled.size() == 1)
        HandlePetActionHelper(pet, packet.PetGUID, spellID, flag, packet.TargetGUID, packet.ActionPosition);
    else
    {
        //If a pet is dismissed, m_Controlled will change
        std::vector<Unit*> controlled;
        for (auto const& guid : GetPlayer()->m_Controlled)
        {
            Unit* unit = ObjectAccessor::GetUnit(*GetPlayer(), guid);
            if (!unit)
                continue;
            if ((unit->GetEntry() == pet->GetEntry() || unit->ToCreature() && unit->ToCreature()->m_isHati) && unit->isAlive())
            {
                if (unit->ToCreature())
                {
                    if (unit->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
                        controlled.push_back(unit);
                }
                else
                    controlled.push_back(unit);
            }
        }

        for (auto const& itr : controlled)
            HandlePetActionHelper(itr, packet.PetGUID, spellID, flag, packet.TargetGUID, packet.ActionPosition);
    }
}

void WorldSession::HandleStopAttack(WorldPackets::PetPackets::StopAttack& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Unit* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*player, packet.PetGUID);
    if (!pet)
        return;

    if (pet != player->GetPet() && pet != player->GetCharm())
        return;

    if (!pet->isAlive())
        return;

    pet->AttackStop();
}

void WorldSession::HandleQueryPetName(WorldPackets::Query::QueryPetName& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    WorldPackets::Query::QueryPetNameResponse response;
    response.UnitGUID = packet.UnitGUID;

    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*player, packet.UnitGUID);
    if (!pet)
    {
        player->SendDirectMessage(response.Write());
        return;
    }

    response.Allow = pet->isPet();
    response.Name = pet->GetName();

    if (Pet* playerPet = pet->ToPet())
    {
        if (DeclinedName const* declinedNames = playerPet->GetDeclinedNames())
        {
            response.HasDeclined = true;
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                if (!declinedNames->name[i].empty())
                    response.DeclinedNames.name[i] = declinedNames->name[i];
        }
        else
            response.HasDeclined = false;

        response.Timestamp = playerPet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP);
    }

    player->SendDirectMessage(response.Write());
}

void WorldSession::HandlePetSetAction(WorldPackets::PetPackets::PetSetAction& packet)
{
    Unit* pet = ObjectAccessor::GetUnit(*_player, packet.PetGUID);
    if (!pet || pet != _player->GetFirstControlled())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetSetAction: Unknown pet (GUID: %u) or pet owner (GUID: %u)", packet.PetGUID.GetGUIDLow(), _player->GetGUIDLow());
        return;
    }

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::HandlePetSetAction: object (GUID: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!", pet->GetGUIDLow(), pet->GetTypeId());
        return;
    }

    if (packet.Index >= MAX_UNIT_ACTION_BAR_INDEX)
        return;

    uint32 spellID = UNIT_ACTION_BUTTON_ACTION(packet.Action);
    uint8 actState = UNIT_ACTION_BUTTON_TYPE(packet.Action);

    if (actState == ACT_DECIDE && !charmInfo->GetActionBarEntry(packet.Index))
        return;

    TC_LOG_INFO(LOG_FILTER_NETWORKIO, "Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X HasSpell %i",
        _player->GetName(), packet.Index, spellID, uint32(actState), pet->HasSpell(spellID));

    //if it's act for spell (en/disable/cast) and there is a spell given (0 = remove spell) which pet doesn't know, don't add
    if (!((actState == ACT_ENABLED || actState == ACT_DISABLED || actState == ACT_PASSIVE) && spellID && !pet->HasSpell(spellID)))
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID))
        {
            if (actState == ACT_ENABLED)
            {
                if (pet->GetCharmInfo())
                {
                    if (Pet* playerPet = pet->ToPet())
                        playerPet->ToggleAutocast(spellInfo, true);
                }
                else
                    for (auto const& guid : GetPlayer()->m_Controlled)
                        if(Unit* unit = ObjectAccessor::GetUnit(*GetPlayer(), guid))
                            if (unit->GetEntry() == pet->GetEntry())
                                unit->GetCharmInfo()->ToggleCreatureAutocast(spellInfo, true);
            }
            else if (actState == ACT_DISABLED)
            {
                if (pet->GetCharmInfo())
                {
                    if (Pet* playerPet = pet->ToPet())
                        playerPet->ToggleAutocast(spellInfo, false);
                }
                else
                    for (auto const& guid : GetPlayer()->m_Controlled)
                        if(Unit* unit = ObjectAccessor::GetUnit(*GetPlayer(), guid))
                            if (unit->GetEntry() == pet->GetEntry())
                                unit->GetCharmInfo()->ToggleCreatureAutocast(spellInfo, false);
            }
        }

        charmInfo->SetActionBar(packet.Index, spellID, ActiveStates(actState));
    }
}

void WorldSession::HandlePetRename(WorldPackets::PetPackets::PetRename& packet)
{
    ObjectGuid petguid = packet.RenameData.PetGUID;

    DeclinedName* declinedname = packet.RenameData.DeclinedNames.get_ptr();

    Pet* pet = ObjectAccessor::GetPet(*_player, petguid);
    if (!pet || !pet->isPet() || pet->getPetType() != HUNTER_PET ||!pet->HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_CAN_BE_RENAMED) || pet->GetOwnerGUID() != _player->GetGUID() || !pet->GetCharmInfo())
        return;

    PetNameInvalidReason res = sCharacterDataStore->CheckPetName(packet.RenameData.NewName);
    if (res != PET_NAME_SUCCESS)
    {
        SendPetNameInvalid(res, petguid, packet.RenameData.NewName);
        return;
    }

    if (sCharacterDataStore->IsReservedName(packet.RenameData.NewName))
    {
        SendPetNameInvalid(PET_NAME_RESERVED, petguid, packet.RenameData.NewName);
        return;
    }

    pet->SetName(packet.RenameData.NewName);

    pet->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

    pet->RemoveByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_CAN_BE_RENAMED);

    if (declinedname && sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        std::wstring wname;
        if (!Utf8toWStr(packet.RenameData.NewName, wname))
            return;

        if (!sCharacterDataStore->CheckDeclinedNames(wname, *declinedname))
        {
            SendPetNameInvalid(PET_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME, petguid, packet.RenameData.NewName, declinedname);
            return;
        }
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if (declinedname && sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_DECLINEDNAME);
        stmt->setUInt32(0, pet->GetCharmInfo()->GetPetNumber());
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_ADD_CHAR_PET_DECLINEDNAME);
        stmt->setUInt32(0, pet->GetCharmInfo()->GetPetNumber());
        stmt->setUInt64(1, _player->GetGUID().GetGUIDLow());

        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; i++)
            stmt->setString(i + 1, declinedname->name[i]);

        trans->Append(stmt);
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_NAME);
    stmt->setString(0, packet.RenameData.NewName);
    stmt->setUInt64(1, _player->GetGUID().GetGUIDLow());
    stmt->setUInt32(2, pet->GetCharmInfo()->GetPetNumber());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr))); // cast can't be helped
}

void WorldSession::HandlePetAbandon(WorldPackets::PetPackets::PetAbandon& packet)
{
    if (!_player->IsInWorld())
        return;

    if (auto pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, packet.Pet))
    {
        if (pet->isPet())
        {
            _player->RemovePet(pet->ToPet(), true);
            _player->GetSession()->SendStablePet();
        }
        else if (pet->GetGUID() == _player->GetCharmGUID())
            _player->StopCastingCharm();
    }
}

void WorldSession::HandlePetSpellAutocast(WorldPackets::PetPackets::PetSpellAutocast& packet)
{
    if (!_player->GetGuardianPet() && !_player->GetCharm())
        return;

    if (ObjectAccessor::FindPlayer(packet.PetGUID))
        return;

    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, packet.PetGUID);
    if (!pet || (pet != _player->GetGuardianPet() && pet != _player->GetCharm()))
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetSpellAutocast.Pet %u isn't pet of player %s .", packet.PetGUID.GetGUIDLow(), GetPlayer()->GetName());
        return;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(packet.SpellID);
    if (!pet->HasSpell(packet.SpellID) || spellInfo->IsAutocastable())
        return;

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::HandlePetSpellAutocastOpcod: object (GUID: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!", pet->GetGUIDLow(), pet->GetTypeId());
        return;
    }

    if (pet->isPet())
        pet->ToPet()->ToggleAutocast(spellInfo, packet.AutocastEnabled);
    else
        pet->GetCharmInfo()->ToggleCreatureAutocast(spellInfo, packet.AutocastEnabled);

    charmInfo->SetSpellAutocast(spellInfo, packet.AutocastEnabled);
}

void WorldSession::HandlePetCastSpellOpcode(WorldPackets::Spells::PetCastSpell& cast)
{
    if (!_player->GetGuardianPet() && !_player->GetCharm())
        return;

    Unit* caster = ObjectAccessor::GetUnit(*_player, cast.PetGUID);

    if (!caster || (caster != _player->GetGuardianPet() && caster != _player->GetCharm()))
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetCastSpellOpcode: Pet %u isn't pet of player %s .", cast.PetGUID.GetGUIDLow(), GetPlayer()->GetName());
        return;
    }

    if (caster->HasAuraType(SPELL_AURA_DISABLE_ATTACK_AND_CAST))
        return;

    SpellCastTargets targets(caster, cast.Cast);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(cast.Cast.SpellID);
    if (!spellInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: unknown PET spell id %i", cast.Cast.SpellID);
        return;
    }

    bool triggered = false;
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->EffectMask < uint32(1 << i))
            break;

        if (spellInfo->Effects[i]->TargetA.GetTarget() == TARGET_DEST_TRAJ || spellInfo->Effects[i]->TargetB.GetTarget() == TARGET_DEST_TRAJ || spellInfo->Effects[i]->Effect == SPELL_EFFECT_TRIGGER_MISSILE)
            triggered = true;
    }

    // do not cast not learned spells
    if (!caster->HasSpell(cast.Cast.SpellID) || spellInfo->IsPassive())
    {
        if (!triggered)
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetCastSpellOpcode: !HasSpell or IsPassive");
            return;
        }
    }
    else
        triggered = false;

    caster->ClearUnitState(UNIT_STATE_FOLLOW);

    uint32 triggeredCastFlags = triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE;
    triggeredCastFlags &= ~TRIGGERED_IGNORE_POWER_AND_REAGENT_COST;

    TriggerCastData triggerData;
    triggerData.triggerFlags = TriggerCastFlags(triggeredCastFlags);
    triggerData.miscData0 = cast.Cast.Misc[0];
    triggerData.miscData1 = cast.Cast.Misc[1];
    triggerData.spellGuid = cast.Cast.SpellGuid;

    Spell* spell = new Spell(caster, spellInfo, triggerData);
    spell->m_targets = targets;

    if (spellInfo->Categories.StartRecoveryCategory) // Check if spell is affected by GCD
    {
        if (caster->IsCreature() && caster->GetGlobalCooldownMgr().HasGlobalCooldown(spellInfo))
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetCastSpellOpcode: Check if spell is affected by GCD");
            spell->SendCastResult(GetPlayer(), spellInfo, SPELL_FAILED_NOT_READY, SPELL_CUSTOM_ERROR_NONE, nullptr, true);
            spell->finish(false);
            delete spell;
            return;
        }
    }

    // TODO: need to check victim?
    SpellCastResult result;
    if (caster->m_movedPlayer)
        result = spell->CheckPetCast(caster->m_movedPlayer->GetSelectedUnit());
    else
        result = spell->CheckPetCast(nullptr);
    if (result == SPELL_CAST_OK)
    {
        if (caster->IsCreature())
        {
            Creature* pet = caster->ToCreature();
            pet->AddCreatureSpellCooldown(cast.Cast.SpellID);
            if (pet->isPet())
            {
                auto p = pet->ToPet();
                // 10% chance to play special pet attack talk, else growl
                // actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
                if (p->getPetType() == SUMMON_PET && (urand(0, 100) < 10))
                    pet->SendPetTalk(static_cast<uint32>(PET_TALK_SPECIAL_SPELL));
                else
                    pet->SendPetAIReaction(cast.PetGUID);
            }
        }

        spell->prepare(&(spell->m_targets));
    }
    else
    {
        Creature* pet = caster->ToCreature();
        bool sendPet = !pet || !(pet->isPossessed() || pet->IsVehicle());
        spell->SendCastResult(GetPlayer(), spellInfo, result, SPELL_CUSTOM_ERROR_NONE, nullptr, sendPet);

        if (caster->IsPlayer())
        {
            if (!caster->ToPlayer()->HasSpellCooldown(cast.Cast.SpellID))
                GetPlayer()->SendClearCooldown(cast.Cast.SpellID, caster);
        }
        else
        {
            if (!caster->ToCreature()->HasCreatureSpellCooldown(cast.Cast.SpellID))
                GetPlayer()->SendClearCooldown(cast.Cast.SpellID, caster);
        }

        spell->finish(false);
        delete spell;
    }
}

void WorldSession::HanleSetPetSlot(WorldPackets::PetPackets::SetPetSlot& packet)
{
    if (!GetPlayer()->GetNPCIfCanInteractWith(packet.NpcGUID, UNIT_NPC_FLAG_STABLEMASTER))
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Stablemaster (GUID:%u) not found or you can't interact with him.", packet.NpcGUID.GetGUIDLow());
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (packet.NewSlot > MAX_PET_STABLES)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Pet* pet = _player->GetPet();
    if (pet && pet->GetCharmInfo() && pet->GetCharmInfo()->GetPetNumber() == packet.PetIndex)
        _player->RemovePet(pet);

    PetSlot curentSlot = GetPlayer()->GetSlotForPetId(GetPlayer()->m_currentPetNumber);
    if (pet && curentSlot == packet.NewSlot)
        _player->RemovePet(pet);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_BY_ID);
    stmt->setUInt64(0, _player->GetGUIDLow());
    stmt->setUInt32(1, packet.PetIndex);

    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSession::HandleStableChangeSlotCallback, this, std::placeholders::_1, packet.PetIndex)));
}

void WorldSession::HandleStableChangeSlotCallback(PreparedQueryResult const& result, uint8 new_slot)
{
    if (!GetPlayer())
        return;

    if (!result)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    Field *fields = result->Fetch();

    uint32 pet_entry = fields[0].GetUInt32();
    uint32 pet_number = fields[1].GetUInt32();
    //bool isHunter = fields[2].GetUInt8() == HUNTER_PET;

    PetSlot slot = GetPlayer()->GetSlotForPetId(pet_number);

    if (!pet_entry)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(pet_entry);
    if (!creatureInfo || !creatureInfo->isTameable(_player))
    {
        // if problem in exotic pet
        if (creatureInfo && creatureInfo->isTameable(_player))
            SendStableResult(STABLE_ERR_EXOTIC);
        else
            SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    // Update if its a Hunter pet
    if (new_slot != 100)
    {
        // We need to remove and add the new pet to there diffrent slots
        GetPlayer()->SwapPetSlot(slot, static_cast<PetSlot>(new_slot));
    }

    SendStableResult(STABLE_SUCCESS_STABLE);
}

void WorldSession::SendStableResult(StableResultCode res)
{
    WorldPackets::PetPackets::StableResult stableResult;
    stableResult.Result = res;
    SendPacket(stableResult.Write());
}

void WorldSession::HandlePetActionHelper(Unit* pet, ObjectGuid petGuid, uint32 spellid, uint16 flag, ObjectGuid targetGuid, Position const& pos)
{
    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::HandlePetAction(petGuid: " UI64FMTD ", tagGuid: " UI64FMTD ", spellId: %u, flag: %u): object (entry: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!", petGuid.GetCounter(), targetGuid.GetCounter(), spellid, flag, pet->GetGUIDLow(), pet->GetTypeId());
        return;
    }

    switch (flag)
    {
        case ACT_COMMAND:                                   //0x07
            switch (spellid)
            {
                case COMMAND_STAY:                          //flat=1792  //STAY
                    // pet->AttackStop();
                    pet->CombatStop(true);
                    pet->StopMoving();
                    pet->GetMotionMaster()->Clear(false);
                    pet->GetMotionMaster()->MoveIdle();
                    charmInfo->SetCommandState(COMMAND_STAY);

                    charmInfo->SetIsCommandAttack(false);
                    charmInfo->SetIsAtStay(true);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsReturning(false);
                    charmInfo->SaveStayPosition();
                    break;
                case COMMAND_FOLLOW:                        //spellid=1792  //FOLLOW
                    // pet->AttackStop();
                    pet->CombatStop(true);
                    // pet->InterruptNonMeleeSpells(false);
                    pet->GetMotionMaster()->MoveFollow(_player, pet->GetFollowDistance(), pet->GetFollowAngle());
                    charmInfo->SetCommandState(COMMAND_FOLLOW);

                    charmInfo->SetIsCommandAttack(false);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsReturning(true);
                    charmInfo->SetIsFollowing(false);
                    break;
                case COMMAND_ATTACK:                        //spellid=1792  //ATTACK
                {
                    // Can't attack if owner is pacified
                    if (_player->HasAuraType(SPELL_AURA_MOD_PACIFY))
                    {
                        //pet->SendPetCastFail(spellid, SPELL_FAILED_PACIFIED);
                        //TODO: Send proper error message to client
                        return;
                    }

                    // only place where pet can be player
                    Unit* TargetUnit = ObjectAccessor::GetUnit(*_player, targetGuid);
                    if (!TargetUnit)
                        return;

                    if (Unit* owner = pet->GetOwner())
                        if (owner->GetDistance(TargetUnit) > SIGHT_RANGE_UNIT || !owner->IsValidAttackTarget(TargetUnit))
                            return;

                    // Not let attack through obstructions
                    if (sWorld->getBoolConfig(CONFIG_PET_LOS))
                    {
                        if (!pet->IsWithinLOSInMap(TargetUnit))
                            return;
                    }

                    pet->ClearUnitState(UNIT_STATE_FOLLOW);
                    // This is true if pet has no target or has target but targets differs.
                    if (pet->getVictim() != TargetUnit || (pet->getVictim() == TargetUnit && !pet->GetCharmInfo()->IsCommandAttack()))
                    {
                        if (pet->getVictim())
                            pet->AttackStop();

                        if (!pet->IsPlayer() && pet->ToCreature()->IsAIEnabled)
                        {
                            charmInfo->SetIsCommandAttack(true);
                            charmInfo->SetIsAtStay(false);
                            charmInfo->SetIsFollowing(false);
                            charmInfo->SetIsReturning(false);

                            pet->ToCreature()->AI()->AttackStart(TargetUnit);

                            //10% chance to play special pet attack talk, else growl
                            if (pet->ToCreature()->isPet() && pet->ToPet()->getPetType() == SUMMON_PET && pet != TargetUnit && urand(0, 100) < 10)
                                pet->SendPetTalk(static_cast<uint32>(PET_TALK_ATTACK));
                            else
                                pet->SendPetAIReaction(petGuid); // 90% chance for pet and 100% chance for charmed creature
                        }
                        else                                // charmed player
                        {
                            if (pet->getVictim() && pet->getVictim() != TargetUnit)
                                pet->AttackStop();

                            charmInfo->SetIsCommandAttack(true);
                            charmInfo->SetIsAtStay(false);
                            charmInfo->SetIsFollowing(false);
                            charmInfo->SetIsReturning(false);

                            pet->Attack(TargetUnit, true);
                            pet->SendPetAIReaction(petGuid);
                        }
                    }
                    break;
                }
                case COMMAND_ABANDON:                       // abandon (hunter pet) or dismiss (summoned pet)
                    if (pet->GetCharmerGUID() == GetPlayer()->GetGUID())
                        _player->StopCastingCharm();
                    else if (pet->GetOwnerGUID() == GetPlayer()->GetGUID())
                    {
                        ASSERT(pet->IsCreature());
                        if (pet->isPet())
                            GetPlayer()->RemovePet(pet->ToPet());
                        else if (pet->HasUnitTypeMask(UNIT_MASK_MINION))
                            dynamic_cast<Minion*>(pet)->UnSummon();
                    }
                    break;
                case COMMAND_MOVE_TO:
                    pet->CombatStop(true);
                    pet->StopMoving();
                    pet->GetMotionMaster()->Clear(false);
                    pet->GetMotionMaster()->MovePoint(0, pos);
                    charmInfo->SetCommandState(COMMAND_MOVE_TO);
                    charmInfo->SetIsCommandAttack(false);
                    charmInfo->SetIsAtStay(true);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsReturning(false);
                    charmInfo->SaveStayPosition();
                    break;

                default:
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: unknown PET flag Action %i and spellid %i.", uint32(flag), spellid);
            }
            break;
        case ACT_REACTION:                                  // 0x6
            switch (spellid)
            {
                case REACT_PASSIVE:                         //passive
                    pet->CombatStop(true);
                    // pet->AttackStop();
                    //pet->GetMotionMaster()->Clear();
                    pet->GetMotionMaster()->MoveFollow(_player, pet->GetFollowDistance(), pet->GetFollowAngle());
                    charmInfo->SetIsReturning(true);
                case REACT_DEFENSIVE:                       //recovery
                case REACT_HELPER:
                case REACT_AGGRESSIVE:
                    if (pet->IsCreature())
                        pet->ToCreature()->SetReactState(ReactStates(spellid));
                    break;
                default:
                    break;
            }
            break;
        case ACT_DISABLED:                                  // 0x81    spell (disabled), ignore
        case ACT_PASSIVE:                                   // 0x01
        case ACT_ENABLED:                                   // 0xC1    spell
        {
            Unit* unit_target = nullptr;

            if (targetGuid)
                unit_target = ObjectAccessor::GetUnit(*_player, targetGuid);

            // do not cast unknown spells
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: unknown PET spell id %i", spellid);
                return;
            }

            if (spellInfo->Categories.StartRecoveryCategory)
            {
                if (pet->GetGlobalCooldownMgr().HasGlobalCooldown(spellInfo))
                    return;
            }

            for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (spellInfo->EffectMask < uint32(1 << i))
                    break;

                if (spellInfo->Effects[i]->TargetA.GetTarget() == TARGET_UNIT_SRC_AREA_ENEMY || spellInfo->Effects[i]->TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY || spellInfo->Effects[i]->TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
                    return;
            }

            // do not cast not learned spells
            if (!pet->HasSpell(spellid) || spellInfo->IsPassive())
                return;

            //  Clear the flags as if owner clicked 'attack'. AI will reset them
            //  after AttackStart, even if spell failed
            if (pet->GetCharmInfo())
            {
                pet->GetCharmInfo()->SetIsAtStay(false);
                pet->GetCharmInfo()->SetIsCommandAttack(true);
                pet->GetCharmInfo()->SetIsReturning(false);
                pet->GetCharmInfo()->SetIsFollowing(false);
            }

            TriggerCastData triggerData;
            Spell* spell = new Spell(pet, spellInfo, triggerData);
            spell->preparePetCast(&(spell->m_targets), unit_target, pet, petGuid, GetPlayer());
            break;
        }
        default:
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: unknown PET flag Action %i and spellid %i.", uint32(flag), spellid);
    }
}

void WorldSession::SendPetNameInvalid(uint32 error, ObjectGuid const& guid, std::string const& name, DeclinedName *declinedName /*= nullptr*/)
{
    WorldPackets::PetPackets::PetNameInvalid petNameInvalid;
    petNameInvalid.Result = error;
    petNameInvalid.RenameData.NewName = name;
    petNameInvalid.RenameData.PetGUID = guid;
    if (declinedName)
        petNameInvalid.RenameData.DeclinedNames = *declinedName;

    SendPacket(petNameInvalid.Write());
}

void WorldSession::SendStablePet(ObjectGuid const& /*guid*/ /*= ObjectGuid::Empty*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    WorldPackets::PetPackets::StableList list;

    std::set<uint32> stableNumber;
    PetInfoDataMap* petMap = player->GetPetInfoData();
    if (!petMap->empty())
    {
        for (auto& petData : *petMap)
        {
            PetSlot petSlot = player->GetSlotForPetId(petData.second.id);
            stableNumber.insert(petData.second.id);

            if (petSlot > PET_SLOT_STABLE_LAST)
                continue;

            if (petSlot == PET_SLOT_FULL_LIST)
                petSlot = static_cast<PetSlot>(player->SetOnAnyFreeSlot(petData.second.id));

            if (petSlot >= PET_SLOT_HUNTER_FIRST &&  petSlot < PET_SLOT_STABLE_LAST)
            {
                WorldPackets::PetPackets::StableInfo info;
                info.PetSlot = petSlot;
                info.PetNumber = petData.second.id;
                info.CreatureID = petData.second.entry;
                info.DisplayID = petData.second.modelid;
                info.ExperienceLevel = petData.second.level;
                info.PetFlags = petSlot < PET_SLOT_STABLE_FIRST ? 1 : 3;
                info.PetName = petData.second.name;
                list.Stables.push_back(info);
            }
        }
    }

    if (player->getClass() == CLASS_HUNTER)
    {
        SendPacket(list.Write());
        SendStableResult(STABLE_ERR_NONE);
    }

    PlayerPetSlotList const& petSlots = player->GetPetSlotList();
    for (uint32 i = uint32(PET_SLOT_HUNTER_FIRST); i < uint32(PET_SLOT_STABLE_LAST); ++i)
    {
        if (!petSlots[i])
            continue;

        auto find = stableNumber.find(petSlots[i]);
        if (find == stableNumber.end())
            player->cleanPetSlotForMove(PetSlot(i), petSlots[i]);
    }
}