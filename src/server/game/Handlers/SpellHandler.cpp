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

#include "SpellPackets.h"
#include "ScriptMgr.h"
#include "Garrison.h"
#include "GameObjectAI.h"
#include "ScenarioMgr.h"
#include "PetPackets.h"
#include "TotemPackets.h"
#include "Guild.h"
#include "PlayerDefines.h"
#include "GameObjectPackets.h"
#include "SpellAuraEffects.h"
#include "Chat.h"

void WorldSession::HandleUseItemOpcode(WorldPackets::Spells::ItemUse& cast)
{
    // TODO: add targets.read() check
    Player* pUser = _player;

    // ignore for remote control state
    if (pUser->m_mover != pUser)
        return;

    // client provided targets
    SpellCastTargets targets(pUser, cast.Cast);

    Item* pItem = pUser->GetUseableItemByPos(cast.bagIndex, cast.slot);
    if (!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (pItem->GetGUID() != cast.itemGUID)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    ItemTemplate const* proto = pItem->GetTemplate();
    if (!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem);
        return;
    }

    // some item classes can be used only in equipped state
    if (proto->GetInventoryType() != INVTYPE_NON_EQUIP && !pItem->IsEquipped())
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return;
    }

    InventoryResult msg = pUser->CanUseItem(pItem);
    if (msg != EQUIP_ERR_OK)
    {
        pUser->SendEquipError(msg, pItem, nullptr);
        return;
    }

    // only allow conjured consumable, bandage, poisons (all should have the 2^21 item flag set in DB)
    if (proto->GetClass() == ITEM_CLASS_CONSUMABLE && !(proto->GetFlags() & ITEM_FLAG_IGNORE_DEFAULT_ARENA_RESTRICTIONS) && pUser->InArena())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH, pItem);
        return;
    }

    if (proto->GetClass() == ITEM_CLASS_CONSUMABLE && !(proto->GetFlags2() & ITEM_FLAG2_IGNORE_DEFAULT_RATED_BG_RESTRICTIONS) && pUser->InRBG())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH, pItem);
        return;
    }

    if (pItem->IsInUse())
    {
        pUser->SendEquipError(EQUIP_ERR_CLIENT_LOCKED_OUT, pItem);
        return;
    }

    // don't allow items banned in arena
    if (proto->GetFlags() & ITEM_FLAG_NOT_USEABLE_IN_ARENA && pUser->InArena())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH, pItem);
        return;
    }

    if (proto->GetFlags2() & ITEM_FLAG2_NOT_USABLE_IN_RATED_BG && pUser->InRBG())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH, pItem);
        return;
    }

    if (pUser->isInCombat())
        for (auto const& v : proto->Effects)
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(v->SpellID))
                if (!spellInfo->CanBeUsedInCombat())
                {
                    pUser->SendEquipError(EQUIP_ERR_NOT_IN_COMBAT, pItem);
                    return;
                }
    
    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if (pItem->GetBonding() == BIND_WHEN_USE || pItem->GetBonding() == BIND_WHEN_PICKED_UP || pItem->GetBonding() == BIND_QUEST_ITEM)
    {
        if (!pItem->IsSoulBound())
        {
            pItem->SetState(ITEM_CHANGED, pUser);
            pItem->SetBinding(true);
        }
    }

    // detect bugged quest items
    if (pItem->GetBonding() == BIND_QUEST_ITEM)
    {
        if (Map* map = _player->GetMap())
        {
            if (map->IsRaid() || map->isChallenge())
            {
                if (Unit * target = targets.GetUnitTarget())
                    sLog->outWarden("AnticheatSpecial: Player %s (GUID: %u) used quest item %u on target %u on map %u", _player->GetName(), _player->GetGUIDLow(), pItem->GetEntry(), target->GetEntry(), map->GetId());
                else
                    sLog->outWarden("AnticheatSpecial: Player %s (GUID: %u) used quest item %u on map %u", _player->GetName(), _player->GetGUIDLow(), pItem->GetEntry(), map->GetId());
            }
        }
    }

    // Note: If script stop casting it must send appropriate data to client to prevent stuck item in gray state.
    if (!sScriptMgr->OnItemUse(pUser, pItem, targets))
    {
        // no script or script not process request by self
        pItem->SetInUse();
        pUser->CastItemUseSpell(pItem, targets, cast.Cast.Misc, cast.Cast.SpellGuid);
    }
}

void WorldSession::HandleGameObjectUse(WorldPackets::GameObject::GameObjectUse& packet)
{
    if (_player->m_mover != _player || !_player->CanContact() || _player->IsSpectator())
        return;

    if (GameObject* obj = GetPlayer()->GetMap()->GetGameObject(packet.Guid))
        obj->Use(_player);
}

void WorldSession::HandleGameobjectReportUse(WorldPackets::GameObject::GameObjReportUse& packet)
{
    auto go = GetPlayer()->GetMap()->GetGameObject(packet.Guid);
    if (!go)
        return;

    if (_player->m_mover != _player)
    {
        if (!(_player->IsOnVehicle(_player->m_mover) || _player->IsMounted()))
            return;

        if (_player->NeedDismount() && !go->GetGOInfo()->IsUsableMounted())
            return;
    }

    if (!go->IsWithinDistInMap(_player, go->GetInteractionDistance()))
        return;

    switch (go->GetEntry())
    {
        case 193905: //Chest Alexstrasza's Gift | Chest Heart of Magic
        case 193967:
        case 194158:
        case 194159:
            _player->CastSpell(go, 6247, true);
            break;
        case 233382:
            go->Use(_player);
            break;
        default:
            go->AI()->GossipHello(_player);
            break;
    }
    _player->UpdateAchievementCriteria(CRITERIA_TYPE_USE_GAMEOBJECT, go->GetEntry());

    switch (go->GetGOInfo()->type)
    {
        case GAMEOBJECT_TYPE_GARRISON_SHIPMENT:
            if (auto garrison = _player->GetGarrison())
                garrison->CompleteShipments(go);
            break;
        case GAMEOBJECT_TYPE_CHEST: //3
            _player->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
            _player->RemoveAurasByType(SPELL_AURA_MOD_INVISIBILITY);
            break;
        default:
            break;
    }
}

void WorldSession::HandleCastSpellOpcode(WorldPackets::Spells::CastSpell& cast)
{
    Unit* mover = _player->m_mover;
    if (mover != _player && mover->IsPlayer())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: mover != _player id %u", cast.Cast.SpellID);
        return;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(cast.Cast.SpellID);
    if (!spellInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: unknown spell id %u", cast.Cast.SpellID);
        return;
    }

    // hack for quest =C
    if (spellInfo->Id == 126892 && GetPlayer()->GetQuestStatus(12103) == QUEST_STATUS_INCOMPLETE)
        spellInfo = sSpellMgr->GetSpellInfo(194004);


    if (spellInfo->IsPassive())
        return;

    if (mover->HasAuraType(SPELL_AURA_DISABLE_ATTACK_AND_CAST))
        return;

    SpellInfo const* castSpellInfo = nullptr;
    if (cast.Cast.SpellID != cast.Cast.SpellGuid.GetEntry())
        castSpellInfo = _player->GetCastSpellInfo(spellInfo, cast.Cast.SpellGuid.GetEntry());

    if (Player* plr = mover->ToPlayer())
    {
        SpellInfo const* curInfo = castSpellInfo ? castSpellInfo : spellInfo;
        uint32 spellGCDEnd = plr->GetGlobalCooldownMgr().GetGlobalCooldownEnd(curInfo);

        if (getMSTime() < spellGCDEnd)
        {
            if (plr->GetSpellInQueue())
            {
                plr->CancelCastSpell(false, cast.Cast.SpellID, cast.Cast.SpellXSpellVisualID, cast.Cast.SpellGuid);
                return;
            }

            plr->SetSpellInQueue(spellGCDEnd, curInfo->Categories.StartRecoveryCategory, &cast.Cast);
            return;
        }
        
        if (SpellInQueue const* spellInQueue = plr->GetSpellInQueue())
        {
            if (cast.Cast.SpellID == spellInQueue->CastData->SpellID)
            {
                plr->CancelCastSpell(true);
                plr->SendOperationsAfterDelay(OAD_RESET_SPELL_QUEUE);
            }
            else if (curInfo->Categories.StartRecoveryCategory == spellInQueue->RecoveryCategory)
            {
                plr->CancelCastSpell(false, cast.Cast.SpellID, cast.Cast.SpellXSpellVisualID, cast.Cast.SpellGuid);
                return;
            }
        }

        // not have spell in spellbook or spell passive and not casted by client
        if ((!plr->HasActiveSpell(cast.Cast.SpellID) || spellInfo->IsPassive()) && !spellInfo->ResearchProject && cast.Cast.SpellID != 101054 && !spellInfo->HasEffect(SPELL_EFFECT_OPEN_LOCK) && !spellInfo->HasEffect(SPELL_EFFECT_LOOT_BONUS) &&
            !(spellInfo->HasAttribute(SPELL_ATTR8_RAID_MARKER)))
        {
            bool doneChecks = false; 
            // check. Maybe it is spell from scenario ?
            Map* map = plr->GetMap();
            if (map && plr->InInstance())
                if (InstanceMap* instance = map->ToInstanceMap())
                    if (Scenario* progress = sScenarioMgr->GetScenario(instance->GetInstanceId()))
                    {
                        if (std::vector<ScenarioSpellData> const* scSpells = sObjectMgr->GetScenarioSpells(progress->GetScenarioId()))
                        {
                            for (auto scSpell : *scSpells)
                                if (scSpell.StepId == progress->GetCurrentStep())
                                    if (cast.Cast.SpellID == scSpell.Spells)
                                        doneChecks = true; // it is spell from scenario.
                        }
                    }
            if (!doneChecks)
            {
                if (cast.Cast.SpellID == 101603)
                {
                    mover->RemoveAurasDueToSpell(107837);
                    mover->RemoveAurasDueToSpell(101601);
                }else
                if (cast.Cast.SpellID == 119393)
                {
                    mover->RemoveAurasDueToSpell(119388);
                    mover->RemoveAurasDueToSpell(119386);
                }
                else if (!cast.Cast.Charmer.IsEmpty())
                {
                    if (Garrison *garr = plr->GetGarrison())
                        if (!garr->CanCastTradeSkill(cast.Cast.Charmer, cast.Cast.SpellID))
                            return;
                }
                else
                {
                    //cheater? kick? ban?
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: cheater? kick? ban? TYPEID_PLAYER spell id %u", cast.Cast.SpellID);
                    return;
                }
            }
        }
    }
    else
    {
        // spell passive and not casted by client
        if (spellInfo->IsPassive())
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: spell passive and not casted by client id %u", cast.Cast.SpellID);
            return;
        }
        // not have spell in spellbook or spell passive and not casted by client
        if ((mover->IsCreature() && !mover->ToCreature()->HasSpell(cast.Cast.SpellID)) || spellInfo->IsPassive())
            if (mover->IsCreature() && !mover->ToCreature()->HasSpell(cast.Cast.SpellID))
            {
                if (_player->HasActiveSpell(cast.Cast.SpellID))
                    mover = static_cast<Unit*>(_player);
                else
                {
                    //cheater? kick? ban?
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: not have spell in spellbook id %u", cast.Cast.SpellID);
                    return;
                }
            }
    }

    if (castSpellInfo)
        spellInfo = castSpellInfo;

    // Client is resending autoshot cast opcode when other spell is casted during shoot rotation
    // Skip it to prevent "interrupt" message
    if (spellInfo->IsAutoRepeatRangedSpell() && _player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)
        && _player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_spellInfo == spellInfo)
        return;

    // can't use our own spells when we're in possession of another unit,
    if (_player->isPossessing())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: can't use our own spells when we're in possession id %u", cast.Cast.SpellID);
        return;
    }

    // Check possible spell cast overrides
    //603 TODO
    //spellInfo = caster->GetCastSpellInfo(spellInfo);

    // client provided targets
    SpellCastTargets targets(mover, cast.Cast);
    // auto-selection buff level base at target level (in spellInfo)
    if (targets.GetUnitTarget())
    {
        SpellInfo const* actualSpellInfo = spellInfo->GetAuraRankForLevel(targets.GetUnitTarget()->getLevel());

        // if rank not found then function return NULL but in explicit cast case original spell can be casted and later failed with appropriate error message
        if (actualSpellInfo)
            spellInfo = actualSpellInfo;
    }

    if (spellInfo->IsPassive()) // Fix cheat
        return;

    targets.m_weights.resize(cast.Cast.Weight.size());
    for (size_t i = 0; i < cast.Cast.Weight.size(); ++i)
    {
        targets.m_weights[i].type = cast.Cast.Weight[i].Type;
        switch (targets.m_weights[i].type)
        {
            case WEIGHT_KEYSTONE:
                targets.m_weights[i].keystone.itemId = cast.Cast.Weight[i].ID;
                targets.m_weights[i].keystone.itemCount = cast.Cast.Weight[i].Quantity;
                break;
            case WEIGHT_FRAGMENT:
                targets.m_weights[i].fragment.currencyId = cast.Cast.Weight[i].ID;
                targets.m_weights[i].fragment.currencyCount = cast.Cast.Weight[i].Quantity;
                break;
            default:
                targets.m_weights[i].raw.id = cast.Cast.Weight[i].ID;
                targets.m_weights[i].raw.count = cast.Cast.Weight[i].Quantity;
                break;
        }
    }

    if (cast.Cast.MoveUpdate)
        HandleMovementOpcode(CMSG_MOVE_STOP, *cast.Cast.MoveUpdate);

    // uint32 _s = getMSTime();
    TriggerCastData triggerData;
    triggerData.miscData0 = cast.Cast.Misc[0];
    triggerData.miscData1 = cast.Cast.Misc[1];
    triggerData.spellGuid = cast.Cast.SpellGuid;
    triggerData.replaced = true;

    auto spell = new Spell(mover, spellInfo, triggerData);
    spell->m_SpellVisual = cast.Cast.SpellXSpellVisualID;
    spell->SetStartCastTime(spell->GetStartCastTime() - 50);
    spell->prepare(&targets);
    // uint32 _ms = getMSTimeDiff(_s, getMSTime());
    // if (_ms > 50)
        // sLog->outDiff("HandleCastSpellOpcode Diff - %ums Id %u.", _ms, spellInfo->Id);
}

void WorldSession::HandleCancelCast(WorldPackets::Spells::CancelCast& packet)
{
    if (_player->IsNonMeleeSpellCast(false))
        _player->InterruptNonMeleeSpells(false, packet.SpellID, false);
}

void WorldSession::HandleCancelAura(WorldPackets::Spells::CancelAura& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(packet.SpellID);
    if (!spellInfo)
        return;

    if (spellInfo->HasAttribute(SPELL_ATTR0_CANT_CANCEL))
        return;

    if (spellInfo->IsChanneled())
    {
        if (Spell* curSpell = player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            if (spellInfo->HasChannelInterruptFlag(CHANNEL_INTERRUPT_FLAG_UNK13) && spellInfo->HasChannelInterruptFlag(CHANNEL_INTERRUPT_FLAG_UNK12) && spellInfo->HasChannelInterruptFlag(CHANNEL_INTERRUPT_FLAG_UNK1))
                player->RemoveOwnedAura(packet.SpellID, packet.CasterGUID, 0, AURA_REMOVE_BY_CANCEL);

            if (curSpell->m_spellInfo->Id == packet.SpellID)
                player->InterruptSpell(CURRENT_CHANNELED_SPELL, true, true, packet.SpellID);
        }
        return;
    }

    if (!spellInfo->IsPositive() || spellInfo->IsPassive())
        return;

    player->RemoveOwnedAura(packet.SpellID, packet.CasterGUID, 0, AURA_REMOVE_BY_CANCEL);
}

void WorldSession::HandlePetCancelAura(WorldPackets::PetPackets::PetCancelAura& packet)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(packet.SpellID);
    if (!spellInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WORLD: unknown PET spell id %u", packet.SpellID);
        return;
    }

    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, packet.PetGUID);
    if (!pet)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetCancelAura: Attempt to cancel an aura for non-existant pet %u by player '%s'", uint32(packet.PetGUID.GetGUIDLow()), GetPlayer()->GetName());
        return;
    }

    if (pet != GetPlayer()->GetGuardianPet() && pet != GetPlayer()->GetCharm())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandlePetCancelAura: Pet %u is not a pet of player '%s'", uint32(packet.PetGUID.GetGUIDLow()), GetPlayer()->GetName());
        return;
    }

    if (!pet->isAlive())
    {
        pet->SendPetActionFeedback(packet.SpellID, FEEDBACK_PET_DEAD);
        return;
    }

    pet->RemoveOwnedAura(packet.SpellID, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);

    pet->AddCreatureSpellCooldown(packet.SpellID);
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPackets::Spells::CancelAutoRepeatSpell& /*packet*/)
{
    // may be better send SMSG_CANCEL_AUTO_REPEAT?
    // cancel and prepare for deleting
    _player->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::HandleCancelChanneling(WorldPackets::Spells::CancelChannelling& packet)
{
    Unit* mover = _player->m_mover;
    if (mover != _player && mover->IsPlayer())
        return;

    mover->InterruptSpell(CURRENT_CHANNELED_SPELL, true, true, packet.ChannelSpell);
}

void WorldSession::HandleTotemDestroyed(WorldPackets::Totem::TotemDestroyed& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (player->m_mover != player)
        return;

    ++packet.Slot;

    if (packet.Slot >= MAX_TOTEM_SLOT)
        return;

    if (!player->m_SummonSlot[packet.Slot])
        return;

    if (Creature* summon = player->GetMap()->GetCreature(player->m_SummonSlot[packet.Slot]))
    {
        if (uint32 spellId = summon->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL))
            if (AreaTrigger* arTrigger = player->GetAreaObject(spellId))
                arTrigger->SetDuration(0);

        if (summon->GetEntry() == 113845) // Totem Mastery
        {
            if (Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[MAX_SUMMON_SLOT - 1]))
                totem->DespawnOrUnsummon();
            if (Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[MAX_SUMMON_SLOT - 2]))
                totem->DespawnOrUnsummon();
            if (Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[MAX_SUMMON_SLOT - 3]))
                totem->DespawnOrUnsummon();
            if (Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[MAX_SUMMON_SLOT - 4]))
                totem->DespawnOrUnsummon();
        }
        if (summon->GetEntry() == 99738) // Call Dreadstalkers
        {
            if (GuidList* summonList = player->GetSummonList(98035))
                for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                    if(Creature* summonPet = ObjectAccessor::GetCreature(*player, (*iter)))
                        summonPet->DespawnOrUnsummon(250);
            if (GuidList* summonList = player->GetSummonList(99737))
                for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                    if(Creature* summonPet = ObjectAccessor::GetCreature(*player, (*iter)))
                        summonPet->DespawnOrUnsummon(250);
        }
        summon->DespawnOrUnsummon();
    }
}

void WorldSession::HandleSelfRes(WorldPackets::Spells::SelfRes& packet)
{
    if (_player->HasAuraType(SPELL_AURA_PREVENT_RESURRECTION))
        return; // silent return, client should display error by itself and not send this opcode

    auto const& selfResSpells = _player->GetDynamicValues(PLAYER_DYNAMIC_FIELD_SELF_RES_SPELLS);
    if (std::find(selfResSpells.begin(), selfResSpells.end(), packet.SpellID) == selfResSpells.end())
        return;

    if (auto spellInfo = sSpellMgr->GetSpellInfo(packet.SpellID))
        _player->CastSpell(_player, spellInfo, false, nullptr);

    _player->RemoveDynamicValue(PLAYER_DYNAMIC_FIELD_SELF_RES_SPELLS, packet.SpellID);
}

void WorldSession::HandleSpellClick(WorldPackets::Spells::SpellClick& packet)
{
    // this will get something not in world. crash
    Creature* unit = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, packet.SpellClickUnitGuid);
    if (!unit)
        return;

    // TODO: Unit::SetCharmedBy: 28782 is not in world but 0 is trying to charm it! -> crash
    if (!unit->IsInWorld())
        return;

    // flags in Deepwind Gorge
    if (unit->GetEntry() == 53194)
    {
        _player->CastSpell(unit, unit->GetInt32Value(UNIT_FIELD_INTERACT_SPELL_ID));
        return;
    }

    unit->HandleSpellClick(_player);
}

void WorldSession::HandleGetMirrorImageData(WorldPackets::Spells::GetMirrorImageData& packet)
{
    // Get unit for which data is needed by client
    Unit* unit = ObjectAccessor::GetObjectInWorld(packet.UnitGUID, static_cast<Unit*>(nullptr));
    if (!unit)
        return;

    if (Creature* creature = unit->ToCreature())
    {
        int32 outfitId = creature->GetOutfit();
        if (outfitId < 0)
        {
            const auto& outfits = sObjectMgr->GetCreatureOutfitMap();
            auto it = outfits.find(-outfitId);
            if (it != outfits.end())
            {
                auto const& outfit = it->second;
                WorldPackets::Spells::MirrorImageComponentedData mirrorImageComponentedData;
                mirrorImageComponentedData.UnitGUID = packet.UnitGUID;
                mirrorImageComponentedData.DisplayID = outfit.displayId;
                mirrorImageComponentedData.RaceID = outfit.race;
                mirrorImageComponentedData.Gender = outfit.gender;
                mirrorImageComponentedData.ClassID = outfit.Class;

                mirrorImageComponentedData.SkinColor = outfit.skin;
                mirrorImageComponentedData.FaceVariation = outfit.face;
                mirrorImageComponentedData.HairVariation = outfit.hair;
                mirrorImageComponentedData.HairColor = outfit.haircolor;
                mirrorImageComponentedData.BeardVariation = outfit.facialhair;

                static_assert(CreatureOutfit::max_custom_displays == PLAYER_CUSTOM_DISPLAY_SIZE, "Amount of custom displays for player has changed - change it for dressnpcs as well");
                for (uint32 i = 0; i < PLAYER_CUSTOM_DISPLAY_SIZE; ++i)
                    mirrorImageComponentedData.CustomDisplay[i] = outfit.customdisplay[i];
                mirrorImageComponentedData.GuildGUID = ObjectGuid::Empty;

                mirrorImageComponentedData.ItemDisplayID.reserve(11);
                for (auto const& display : it->second.outfit)
                    mirrorImageComponentedData.ItemDisplayID.push_back(display);

                SendPacket(mirrorImageComponentedData.Write());
                return;
            }
        }
    }

    if (!unit->HasAuraType(SPELL_AURA_CLONE_CASTER))
        return;

    auto mirImageData = unit->GetMirrorImageInfo();

    WorldPackets::Spells::MirrorImageComponentedData mirrorImageComponentedData;
    mirrorImageComponentedData.UnitGUID = packet.UnitGUID;
    mirrorImageComponentedData.DisplayID = mirImageData.DisplayID;

    if (mirImageData.isPlayer)
    {
        mirrorImageComponentedData.RaceID = mirImageData.RaceID;
        mirrorImageComponentedData.Gender = mirImageData.Gender;
        mirrorImageComponentedData.ClassID = mirImageData.ClassID;
        mirrorImageComponentedData.SkinColor = mirImageData.SkinColor;
        mirrorImageComponentedData.FaceVariation = mirImageData.FaceVariation;
        mirrorImageComponentedData.HairVariation = mirImageData.HairVariation;
        mirrorImageComponentedData.HairColor = mirImageData.HairColor;
        mirrorImageComponentedData.BeardVariation = mirImageData.BeardVariation;
        mirrorImageComponentedData.GuildGUID = mirImageData.GuildGUID;

        for (uint32 i = 0; i < PLAYER_CUSTOM_DISPLAY_SIZE; ++i)
            mirrorImageComponentedData.CustomDisplay[i] = mirImageData.CustomDisplay[i];

        for (auto const& slot : mirImageData.ItemDisplayID)
            mirrorImageComponentedData.ItemDisplayID.push_back(slot);
    }

    SendPacket(mirrorImageComponentedData.Write());
}

void WorldSession::HandleMissileTrajectoryCollision(WorldPackets::Spells::MissileTrajectoryCollision& packet)
{
    Unit* caster = ObjectAccessor::GetUnit(*_player, packet.Target);
    if (!caster)
        return;

    Spell* spell = caster->FindCurrentSpellBySpellId(packet.SpellID);
    if (!spell || !spell->m_targets.HasDst())
        return;

    Position pos = static_cast<Position>(*spell->m_targets.GetDstPos());
    pos.Relocate(packet.CollisionPos);
    spell->m_targets.ModDst(pos);

    WorldPackets::Spells::NotifyMissileTrajectoryCollision notify;
    notify.Caster = packet.Target;
    notify.CastGUID = packet.CastGUID;
    notify.CollisionPos = packet.CollisionPos;
    caster->SendMessageToSet(notify.Write(), true);
}

void WorldSession::HandleUpdateMissileTrajectory(WorldPackets::Spells::UpdateMissileTrajectory& packet)
{
    Unit* caster = ObjectAccessor::GetUnit(*_player, packet.Guid);
    Spell* spell = caster ? caster->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
    if (!spell || spell->m_spellInfo->Id != uint32(packet.SpellID) || !spell->m_targets.HasDst() || !spell->m_targets.HasSrc())
        return;

    spell->m_targets.ModSrc(packet.FirePos);
    spell->m_targets.ModDst(packet.ImpactPos);
    spell->m_targets.SetPitch(packet.Pitch);
    spell->m_targets.SetSpeed(packet.Speed);

    if (packet.Status)
        HandleMovementOpcode(CMSG_MOVE_STOP, *packet.Status);
}

void WorldSession::HandleRequestCategoryCooldowns(WorldPackets::Spells::RequestCategoryCooldowns& /*packet*/)
{
    _player->SendCategoryCooldownMods();
}

void WorldSession::HandleCancelMountAura(WorldPackets::Spells::CancelMountAura& /*packet*/)
{
    _player->RemoveAurasByType(SPELL_AURA_MOUNTED, [](AuraApplication const* aurApp)
    {
        SpellInfo const* spellInfo = aurApp->GetBase()->GetSpellInfo();
        return !spellInfo->HasAttribute(SPELL_ATTR0_CANT_CANCEL) && spellInfo->IsPositive() && !spellInfo->IsPassive();
    });
}

void WorldSession::HandleCancelGrowthAura(WorldPackets::Spells::CancelGrowthAura& /*packet*/)
{
    _player->RemoveAurasByType(SPELL_AURA_MOD_SCALE, [](AuraApplication const* aurApp)
    {
        SpellInfo const* spellInfo = aurApp->GetBase()->GetSpellInfo();
        return !spellInfo->HasAttribute(SPELL_ATTR0_CANT_CANCEL) && spellInfo->IsPositive() && !spellInfo->IsPassive();
    });
}

void WorldSession::HandleSetActionButtonOpcode(WorldPackets::Spells::SetActionButton& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    uint32 action = packet.Action;
    auto type = uint8(packet.Type >> 24);

    TC_LOG_INFO(LOG_FILTER_NETWORKIO, "BUTTON: %u ACTION: %u TYPE: %u", packet.Index, action, type);

    if (!packet.Action && !type)
        player->removeActionButton(packet.Index);
    else
    {
        switch (type)
        {
            case ACTION_BUTTON_MACRO:
            case ACTION_BUTTON_CMACRO:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added Macro %u into button %u", action, packet.Index);
                break;
            case ACTION_BUTTON_EQSET:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added EquipmentSetInfo %u into button %u", action, packet.Index);
                break;
            case ACTION_BUTTON_SPELL:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added Spell %u into button %u", action, packet.Index);
                break;
            case ACTION_BUTTON_SUB_BUTTON:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added sub buttons %u into button %u", action, packet.Index);
                break;
            case ACTION_BUTTON_ITEM:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added Item %u into button %u", action, packet.Index);
                break;
            case ACTION_BUTTON_PET:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added Pet Spell %u into button %u", action, packet.Index);
                break;
            case ACTION_BUTTON_MOUNT:
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "MISC: Added mount or favorite mount into button %u", action, packet.Index);
                break;
            default:
                TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "MISC: Unknown action button type %u for action %u into button %u for player %s (GUID: %u)", type, action, packet.Index, _player->GetName(), _player->GetGUIDLow());
                break;
        }

        player->addActionButton(packet.Index, action, type);
    }
}

void WorldSession::HandleUnlearnSkill(WorldPackets::Spells::UnlearnSkill& packet)
{
    SkillRaceClassInfoEntry const* rcEntry = sDB2Manager.GetSkillRaceClassInfo(packet.SkillID, GetPlayer()->getRace(), GetPlayer()->getClass());
    if (!rcEntry || !(rcEntry->Flags & SKILL_FLAG_UNLEARNABLE))
        return;

    GetPlayer()->SetSkill(packet.SkillID);
}

void WorldSession::HandleUnlearnSpecialization(WorldPackets::Spells::UnlearnSpecialization& packet)
{
    const std::array<uint32, 5> specialisationsIndex = { 20219, 20222, 28677, 28675, 28672 };
    if (packet.SpecializationIndex < 5)
        GetPlayer()->removeSpell(specialisationsIndex[packet.SpecializationIndex]);
}

void WorldSession::HandleCancelModSpeedNoControlAuras(WorldPackets::Spells::CancelModSpeedNoControlAuras& packet)
{

}

void WorldSession::HandleCancelQueuedSpell(WorldPackets::Spells::CancelQueuedSpell& /*packet*/)
{

}

void WorldSession::HandleUpdateSpellVisualOpcode(WorldPackets::Spells::UpdateSpellVisual& packet)
{
    if (Aura* aura = GetPlayer()->GetAura(packet.SpellID))
    {
        aura->SetSpellVisual(packet.SpellXSpellVisualId);
        aura->SetNeedClientUpdateForTargets();
    }
}
