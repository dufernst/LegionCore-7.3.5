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

#include "LootPackets.h"
#include "Corpse.h"
#include "ChallengeMgr.h"

void WorldSession::HandleAutostoreLootItemOpcode(WorldPackets::Loot::AutoStoreLootItem& packet)
{
    Player* player = GetPlayer();
    Loot* loot = nullptr;

    if (!player || !player->CanContact() || !player->isAlive())
        return;

    for (WorldPackets::Loot::LootRequest const& req : packet.Loot)
    {
        ObjectGuid lguid = req.Object;

        if (lguid.IsGameObject())
        {
            GameObject* go = player->GetMap()->GetGameObject(lguid);
            // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
            if (!go || ((go->GetOwnerGUID() != player->GetGUID() && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(player, INTERACTION_DISTANCE)))
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &go->loot;
            //TC_LOG_DEBUG(LOG_FILTER_LOOT, "HandleAutostoreLootItemOpcode lguid %u, pguid %u lguid %u", lguid, player->personalLoot.GetGUID(), loot->GetGUID());
        }
        else if (lguid.IsItem())
        {
            Item* pItem = player->GetItemByGuid(lguid);

            if (!pItem)
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &pItem->loot;
        }
        else if (lguid.IsCorpse())
        {
            Corpse* bones = ObjectAccessor::GetCorpse(*player, lguid);
            if (!bones)
            {
                player->SendLootRelease(lguid);
                return;
            }

            loot = &bones->loot;
        }
        else if (lguid.IsLoot())
        {
            loot = sLootMgr->GetLoot(lguid);
            if(!loot)
            {
                player->SendLootRelease(lguid);
                return;
            }
        }
        else
        {
            Creature* creature = player->GetMap()->GetCreature(lguid);

            bool lootAllowed = creature && creature->isAlive() == (player->getClass() == CLASS_ROGUE && creature->lootForPickPocketed);

            Unit *looter = creature ? creature->GetOtherRecipient() : nullptr;
            if (!looter)
                looter = player;

            if (!lootAllowed || !creature->IsWithinDistInMap(looter, LOOT_DISTANCE))
            {
                player->SendLootError(lguid, lguid, lootAllowed ? LOOT_ERROR_TOO_FAR : LOOT_ERROR_DIDNT_KILL);
                return;
            }

            loot = &creature->loot;
        }

        if (!loot || !player->CanContact())
            return;

        if(Group* group = player->GetGroup())
        {
            // Already rolled?
            if (!loot->personal && group->isRolledSlot(req.LootListID-1))
                return;
        }

        LootItem* item = loot->LootItemInSlot(req.LootListID-1, player, nullptr, nullptr, nullptr, nullptr);
        volatile uint32 ItemID = item ? item->item.ItemID : 0;
        volatile uint32 count = item ? item->count : 0;
        volatile uint32 objEntry = loot->objEntry;
        volatile ObjectGuid objGuid = loot->objGuid;

        // Since 6.x client sends loot starting from 1 hence the -1
        player->StoreLootItem(req.LootListID - 1, loot);

        // If player is removing the last LootItem, delete the empty container.
        if (loot->isLooted() && lguid.IsItem())
            DoLootRelease(lguid);
    }
}

void WorldSession::HandleLootMoney(WorldPackets::Loot::LootMoney& /*packet*/)
{
    GetPlayer()->GetGoldFromLoot();
}

void WorldSession::HandleLootUnit(WorldPackets::Loot::LootUnit& packet)
{
    if (!GetPlayer()->isAlive() || !packet.Unit.IsCreatureOrVehicle())
        return;

    LootCorps(packet.Unit);

    if (GetPlayer()->IsNonMeleeSpellCast(false))
        GetPlayer()->InterruptNonMeleeSpells(false);
}

void WorldSession::LootCorps(ObjectGuid corpsGUID, WorldObject* lootedBy)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->isAlive())
        return;

    WorldObject* _looted = lootedBy ? lootedBy : player;

    Creature* creature = player->GetMap()->GetCreature(corpsGUID);
    if (!creature)
        return;

    std::list<Creature*> corpesList;
    _looted->GetCorpseCreatureInGrid(corpesList, LOOT_DISTANCE);

    WorldPackets::Loot::AELootTargets targets;
    targets.Count = corpesList.size() * 2 - 1; // Count all AOE package loot, not count targets
    player->SendDirectMessage(targets.Write());

    creature->SetOtherLootRecipient(lootedBy ? lootedBy->GetGUID() : ObjectGuid::Empty);
    player->SendLoot(corpsGUID, LOOT_CORPSE, false);

    for (std::list<Creature*>::const_iterator itr = corpesList.begin(); itr != corpesList.end(); ++itr)
    {
        if (Creature* creature_ = (*itr))
        {
            creature_->SetOtherLootRecipient(lootedBy ? lootedBy->GetGUID() : ObjectGuid::Empty);

            if (corpsGUID != creature_->GetGUID())
                player->SendLoot(creature_->GetGUID(), LOOT_CORPSE, true, 1);
        }
    }

    if (Group* group = player->GetGroup())
        group->ClearAoeSlots();

    if (player->IsNonMeleeSpellCast(false))
        player->InterruptNonMeleeSpells(false);
}

void WorldSession::HandleLootRelease(WorldPackets::Loot::LootRelease& packet)
{
    DoLootRelease(packet.Unit);
}

void WorldSession::DoLootRelease(ObjectGuid lguid)
{
    Player* player = GetPlayer();
    Loot* loot = nullptr;
    Loot* lootPesonal = nullptr;

    TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: DoLootRelease lguid %u", lguid.GetCounter());

    player->SetLootGUID(ObjectGuid::Empty);

    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);

    if (!player->IsInWorld())
        return;

    if (lguid.IsGameObject())
    {
        GameObject* go = GetPlayer()->GetMap()->GetGameObject(lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
        if (!go || ((go->GetOwnerGUID() != _player->GetGUID() && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(_player, INTERACTION_DISTANCE)))
            return;

        lootPesonal = player->GetPersonalLoot(lguid);
        loot = &go->loot;

        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR)
        {
            // locked doors are opened with spelleffect openlock, prevent remove its as looted
            go->UseDoorOrButton();
        }
        else if (loot->isLooted() || go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            // GO is mineral vein? so it is not removed after its looted
            if (go->GetGoType() == GAMEOBJECT_TYPE_CHEST)
            {
                uint32 go_min = go->GetGOInfo()->chest.minRestock;
                uint32 go_max = go->GetGOInfo()->chest.maxRestock;

                // only vein pass this check
                if (go_min != 0 && go_max > go_min)
                {
                    float amount_rate = sWorld->getRate(RATE_MINING_AMOUNT);
                    float min_amount = go_min*amount_rate;
                    float max_amount = go_max*amount_rate;

                    go->AddUse();
                    auto uses = float(go->GetUseCount());

                    if (uses < max_amount)
                    {
                        if (uses >= min_amount)
                        {
                            float chance_rate = sWorld->getRate(RATE_MINING_NEXT);

                            int32 ReqValue = 175;
                            LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->GetLockId());
                            if (lockInfo)
                                ReqValue = lockInfo->Skill[0];
                            float skill = float(player->GetSkillValue(SKILL_MINING))/(ReqValue+25);
                            double chance = pow(0.8*chance_rate, 4*(1/double(max_amount))*double(uses));
                            if (roll_chance_f(static_cast<float>(100 * chance + skill)))
                                go->SetLootState(GO_READY);
                            else                            // not have more uses
                                go->SetLootState(GO_JUST_DEACTIVATED);
                        }
                        else                                // 100% chance until min uses
                            go->SetLootState(GO_READY);
                    }
                    else                                    // max uses already
                        go->SetLootState(GO_JUST_DEACTIVATED);
                }
                else if (go->IsPersonal())
                {
                    if (lootPesonal && !lootPesonal->isLooted())
                        lootPesonal->AutoStoreItems(true);

                    go->DestroyForPlayer(player);
                    go->UpdateObjectVisibility();
                }
                else                                        // not vein
                    go->SetLootState(GO_JUST_DEACTIVATED, static_cast<Unit*>(player));
            }
            else if (go->GetGoType() == GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD)
            {
                if (lootPesonal && !lootPesonal->isLooted())
                    lootPesonal->AutoStoreItems(true);

                sChallengeMgr->DeleteOploteLoot(player->GetGUID());
                go->DestroyForPlayer(player);
                go->UpdateObjectVisibility();
            }
            else if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
            {                                               // The fishing hole used once more
                go->AddUse();                               // if the max usage is reached, will be despawned in next tick
                if (go->GetUseCount() >= urand(go->GetGOInfo()->fishingHole.minRestock, go->GetGOInfo()->fishingHole.maxRestock))
                    go->SetLootState(GO_JUST_DEACTIVATED);
                else
                    go->SetLootState(GO_READY);
            }
            else // not chest (or vein/herb/etc)
                go->SetLootState(GO_JUST_DEACTIVATED);

            if(lootPesonal)
            {
                player->RemoveLoot(lguid);
                lootPesonal = nullptr;
            }

            loot->clear();
        }
        else
        {
            // If player go out - reset state. for future using.
            if (player->GetMap()->IsDungeon())
            {
                // not fully looted object
                go->SetLootState(GO_ACTIVATED, player);
            }
            else if (go->IsPersonal())
            {
                if (lootPesonal && !lootPesonal->isLooted())
                    lootPesonal->AutoStoreItems(true);

                go->UpdateObjectVisibility();

                if (go->GetGoType() == GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD)
                    sChallengeMgr->DeleteOploteLoot(player->GetGUID());
            }
            else
            {
                go->SetLootState(GO_JUST_DEACTIVATED);
                loot->clear();
            }

            // if the round robin player release, reset it.
            if (player->GetGUID() == loot->roundRobinPlayer)
            {
                if (Group* group = player->GetGroup())
                {
                    if (group->GetLootMethod() != MASTER_LOOT)
                    {
                        loot->roundRobinPlayer.Clear();
                    }
                }
                else
                    loot->roundRobinPlayer.Clear();
            }
        }
    }
    else if (lguid.IsCorpse())        // ONLY remove insignia at BG
    {
        Corpse* corpse = ObjectAccessor::GetCorpse(*player, lguid);
        if (!corpse || !corpse->IsWithinDistInMap(_player, LOOT_DISTANCE))
            return;

        loot = &corpse->loot;

        if (loot->isLooted())
        {
            loot->clear();
            corpse->RemoveFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);
        }
    }
    else if (lguid.IsItem())
    {
        Item* pItem = player->GetItemByGuid(lguid);
        if (!pItem)
            return;

        ItemTemplate const* proto = pItem->GetTemplate();

        // destroy only 5 items from stack in case prospecting and milling
        if (proto->GetFlags() & (ITEM_FLAG_IS_PROSPECTABLE | ITEM_FLAG_IS_MILLABLE))
        {
            pItem->m_lootGenerated = false;
            pItem->loot.clear();

            uint32 count = pItem->GetCount();

            // >=5 checked in spell code, but will work for cheating cases also with removing from another stacks.
            if (count > 5)
                count = 5;

            player->DestroyItemCount(pItem, count, true);
        }
        else
        {
            pItem->m_lootGenerated = false;
            pItem->loot.clear();

            uint32 count = 1;

            player->DestroyItemCount(pItem, count, true);
            // FIXME: item must not be deleted in case not fully looted state. But this pre-request implement loot saving in DB at item save. Or cheating possible.
            // player->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
        }
        player->SendLootRelease(lguid);
        return;                                             // item can be looted only single player
    }
    else
    {
        Creature* creature = player->GetMap()->GetCreature(lguid);

        bool lootAllowed = creature && creature->isAlive() == (player->getClass() == CLASS_ROGUE && creature->lootForPickPocketed);

        Unit *looter = creature ? creature->GetOtherRecipient() : nullptr;
        if (!looter)
            looter = player;

        // Restore Fetch state for pet.
        if (looter->isPet())
        {
            if(Unit* _petowner = looter->GetOwner())
                if (_petowner == player)
                    looter->GetMotionMaster()->MoveFollow(player, looter->GetFollowDistance(), looter->GetFollowAngle());
        }

        if (!lootAllowed || !creature->IsWithinDistInMap(looter, LOOT_DISTANCE))
            return;

        lootPesonal = player->GetPersonalLoot(lguid);
        loot = &creature->loot;

        if(lootPesonal)
        {
            lootPesonal->isOpen = false;
            if(lootPesonal->isLooted())
            {
                if (loot->isLooted())
                {
                    creature->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                    creature->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                    creature->RemoveFromLootList(player->GetGUID());
                }
                creature->RemoveThreatTarget(player->GetGUID());
                player->RemoveLoot(lguid);
                lootPesonal = nullptr;
            }
        }

        if (loot->isLooted() && creature->lootList.empty())
        {
            // skip pickpocketing loot for speed, skinning timer reduction is no-op in fact
            if (!creature->isAlive())
                creature->AllLootRemovedFromCorpse();

            creature->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            loot->clear();
        }
        else
        {
            // if the round robin player release, reset it.
            if (player->GetGUID() == loot->roundRobinPlayer)
            {
                if (Group* group = player->GetGroup())
                {
                    if (group->GetLootMethod() != MASTER_LOOT)
                    {
                        loot->roundRobinPlayer.Clear();
                        group->SendLooter(creature, nullptr);

                        // force update of dynamic flags, otherwise other group's players still not able to loot.
                        creature->ForceValuesUpdateAtIndex(OBJECT_FIELD_DYNAMIC_FLAGS);
                    }
                }
                else
                    loot->roundRobinPlayer.Clear();
            }
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    loot->RemoveLooter(player->GetGUID());
}

void WorldSession::HandleSetLootSpecialization(WorldPackets::Loot::SetLootSpecialization& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (packet.SpecID == 0)
    {
        player->SetLootSpecID(packet.SpecID);
        return;
    }

    ChrSpecializationEntry const* specialization = sChrSpecializationStore.LookupEntry(packet.SpecID);
    if (specialization && specialization->ClassID == player->getClass())
        player->SetLootSpecID(packet.SpecID);
}

void WorldSession::HandleLootRoll(WorldPackets::Loot::LootRoll& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Group* group = player->GetGroup();
    if (!group)
        return;

    group->CountRollVote(player->GetGUID(), packet.LootListID, packet.RollType);

    switch (packet.RollType)
    {
        case ROLL_NEED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_ROLL_NEED, 1);
            break;
        case ROLL_GREED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_ROLL_GREED, 1);
            break;
        default:
            break;
    }
}

void WorldSession::HandleDoMasterLootRoll(WorldPackets::Loot::DoMasterLootRoll& packet)
{
    if (!_player->GetGroup() || _player->GetGroup()->GetLooterGuid() != _player->GetGUID())
    {
        _player->SendLootRelease(packet.LootObj);
        return;
    }

    Loot* loot = nullptr;
    if (packet.LootObj.IsCreatureOrVehicle())
    {
        Creature* creature = GetPlayer()->GetMap()->GetCreature(packet.LootObj);
        if (creature)
            loot = &creature->loot;
    }
    else if (packet.LootObj.IsGameObject())
    {
        GameObject* pGO = GetPlayer()->GetMap()->GetGameObject(packet.LootObj);
        if (pGO)
            loot = &pGO->loot;
    }
    else if (packet.LootObj.IsLoot())
    {
        loot = sLootMgr->GetLoot(packet.LootObj);
        if (!loot)
            return;
    }

    packet.LootListID -= 1; //restore slot index;
    if (packet.LootListID >= loot->items.size() + loot->quest_items.size())
    {
        // TC_LOG_DEBUG(LOG_FILTER_LOOT, "MasterLootItem: Player %s might be using a hack! (slot %d, size %lu)", GetPlayer()->GetName(), packet.LootListID, (unsigned long)loot->items.size());
        return;
    }

    LootItem& item = packet.LootListID >= loot->items.size() ? loot->quest_items[packet.LootListID - loot->items.size()] : loot->items[packet.LootListID];
    _player->GetGroup()->DoRollForAllMembers(packet.LootObj, packet.LootListID, _player->GetMapId(), loot, item, _player);
}

void WorldSession::HandleMasterLootItem(WorldPackets::Loot::MasterLootItem& packet)
{
    Group* group = _player->GetGroup();
    if (!group || group->isLFGGroup() || group->GetLooterGuid() != _player->GetGUID())
    {
        _player->SendLootRelease(GetPlayer()->GetLootGUID());
        return;
    }

    for (auto const& lootData : packet.Loot)
    {
        Player* target = ObjectAccessor::FindPlayer(packet.Target);
        if (!target)
            continue;

        Loot* loot = nullptr;
        if (lootData.Object.IsCreatureOrVehicle())
        {
            Creature* creature = GetPlayer()->GetMap()->GetCreature(lootData.Object);
            if (!creature)
                continue;

            loot = &creature->loot;
        }
        else if (lootData.Object.IsGameObject())
        {
            GameObject* pGO = GetPlayer()->GetMap()->GetGameObject(lootData.Object);
            if (!pGO)
                continue;

            loot = &pGO->loot;
        }
        else if (lootData.Object.IsLoot())
        {
            loot = sLootMgr->GetLoot(lootData.Object);
            if (!loot)
                continue;
        }

        if (!loot)
            continue;

        uint8 _LootListID = lootData.LootListID - 1;    //restore slot index; WTF?
        if (_LootListID >= loot->items.size() + loot->quest_items.size())
        {
            TC_LOG_DEBUG(LOG_FILTER_LOOT, "MasterLootItem: Player %s might be using a hack! (slot %d, size %lu)",
                GetPlayer()->GetName(), _LootListID, static_cast<uint32>(loot->items.size()));
            return;
        }

        LootItem& item = _LootListID >= static_cast<uint8>(loot->items.size()) ? loot->quest_items[_LootListID - static_cast<uint8>(loot->items.size())] : loot->items[_LootListID];
        if (item.currency)
        {
            TC_LOG_DEBUG(LOG_FILTER_LOOT, "WorldSession::HandleMasterLootItem: player %s tried to give currency via master loot! Hack alert! Slot %u, currency id %u",
                GetPlayer()->GetName(), _LootListID, item.item.ItemID);
            return;
        }

        ItemPosCountVec dest;
        InventoryResult msg = target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item.item.ItemID, item.count);
        if (item.follow_loot_rules && !loot->AllowedForPlayer(target, item.item.ItemID, item.type, item.needs_quest, &item))
            msg = EQUIP_ERR_CANT_EQUIP_EVER;
        if (msg != EQUIP_ERR_OK)
        {
            target->SendEquipError(msg, nullptr, nullptr, item.item.ItemID);
            _player->SendEquipError(msg, nullptr, nullptr, item.item.ItemID);
            return;
        }

        // delete roll's in progress for this aoeSlot
        group->ErraseRollbyRealSlot(_LootListID, loot);

        // ToDo: check for already rolled items. This could posible on packet spaming (special tools should be writen, no so important now)

        // list of players allowed to receive this item in trade
        // not move item from loot to target inventory
        Item* newitem = target->StoreNewItem(dest, item.item.ItemID, true, item.item.RandomPropertiesID, item.GetAllowedLooters(), item.item.ItemBonus.BonusListIDs, item.item.ItemBonus.Context);
        target->SendNewItem(newitem, uint32(item.count), false, false, true);
        target->UpdateAchievementCriteria(CRITERIA_TYPE_LOOT_ITEM, item.item.ItemID, item.count);
        target->UpdateAchievementCriteria(CRITERIA_TYPE_LOOT_TYPE, loot->loot_type, item.count);
        target->UpdateAchievementCriteria(CRITERIA_TYPE_LOOT_EPIC_ITEM, item.item.ItemID, item.count);

        // mark as looted
        item.count = 0;
        item.is_looted = true;

        loot->NotifyItemRemoved(_LootListID);
        --loot->unlootedCount;
    }
}

void WorldSession::HandleCancelMasterLootRoll(WorldPackets::Loot::CancelMasterLootRoll& /*packet*/)
{ }
