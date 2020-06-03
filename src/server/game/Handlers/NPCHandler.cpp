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
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "GameObjectAI.h"
#include "GossipDef.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "Pet.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "SpellInfo.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "NPCPackets.h"
#include "ItemPackets.h"
#include "GuildMgr.h"
#include "NPCPackets.h"
#include "MailPackets.h"

void WorldSession::HandleTabardVendorActivate(WorldPackets::NPC::Hello& packet)
{
    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(packet.Unit, UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
        return;

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendTabardVendorActivate(packet.Unit);
}

void WorldSession::SendTabardVendorActivate(ObjectGuid const& guid)
{
    WorldPackets::NPC::PlayerTabardVendorActivate activate;
    activate.Vendor = guid;
    SendPacket(activate.Write());
}

void WorldSession::HandleBankerActivate(WorldPackets::NPC::Hello& packet)
{
    if (!CanUseBank(packet.Unit))
        return;

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendShowBank(packet.Unit);
}

void WorldSession::SendShowBank(ObjectGuid const& guid)
{
    m_currentBankerGUID = guid;

    WorldPackets::NPC::ShowBank bank;
    bank.Guid = guid;
    SendPacket(bank.Write());
}

void WorldSession::SendShowMailBox(ObjectGuid guid)
{
    WorldPackets::Mail::ShowMailbox packet;
    packet.PostmasterGUID = guid;
    SendPacket(packet.Write());
}

void WorldSession::HandleTrainerList(WorldPackets::NPC::Hello& packet)
{
    SendTrainerList(packet.Unit);
}

void WorldSession::SendTrainerList(ObjectGuid const& guid)
{
    std::string str = GetTrinityString(LANG_NPC_TAINER_HELLO);
    SendTrainerList(guid, str);
}

void WorldSession::SendTrainerList(ObjectGuid const& guid, std::string const& strTitle)
{
    TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList");

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can not interact with him.", uint32(guid.GetGUIDLow()));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // trainer list loaded at check;
    if (!unit->isCanTrainingOf(_player, true))
        return;

    CreatureTemplate const* ci = unit->GetCreatureTemplate();

    if (!ci)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!", guid.GetGUIDLow());
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - Training spells not found for creature (GUID: %u Entry: %u)",
            guid.GetGUIDLow(), unit->GetEntry());
        return;
    }

    WorldPackets::NPC::TrainerList packet;
    packet.TrainerGUID = guid;
    packet.TrainerType = trainer_spells->trainerType;
    packet.Greeting = strTitle;

    // reputation discount
    float fDiscountMod = _player->GetReputationPriceDiscount(unit);

    packet.Spells.resize(trainer_spells->spellList.size());
    uint32 count = 0;
    for (const auto & itr : trainer_spells->spellList)
    {
        TrainerSpell const* tSpell = &itr.second;

        bool valid = true;
        for (auto i : tSpell->learnedSpell)
        {
            if (!i)
                continue;

            if (!_player->IsSpellFitByClassAndRace(i))
            {
                valid = false;
                break;
            }
        }

        if (!valid)
            continue;

        // whats the fuck?
        /*if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(tSpell->spell))
        {
            if (spellInfo->HasAttribute(SPELL_ATTR7_HORDE_ONLY) && GetPlayer()->GetTeam() != HORDE)
                continue;
            if (spellInfo->HasAttribute(SPELL_ATTR7_ALLIANCE_ONLY) && GetPlayer()->GetTeam() != ALLIANCE)
                continue;
        }*/

        TrainerSpellState state = _player->GetTrainerSpellState(tSpell);

        WorldPackets::NPC::TrainerListSpell& spell = packet.Spells[count];
        spell.SpellID = tSpell->spell;
        spell.MoneyCost = floor(tSpell->spellCost * fDiscountMod);
        spell.ReqSkillLine = tSpell->reqSkill;
        spell.ReqSkillRank = tSpell->reqSkillValue;
        spell.ReqLevel = tSpell->reqLevel;
        spell.Usable = (state == TRAINER_SPELL_GREEN_DISABLED ? TRAINER_SPELL_GREEN : state);

        uint8 maxReq = 0;
        for (auto i : tSpell->learnedSpell)
        {
            if (!i)
                continue;

            if (maxReq > MAX_TRAINERSPELL_ABILITY_REQS)
                break;

            if (uint32 prevSpellId = sSpellMgr->GetPrevSpellInChain(i))
            {
                spell.ReqAbility[maxReq++] = prevSpellId;
                continue;
            }

            for (auto const& requirePair : sSpellMgr->GetSpellsRequiredForSpellBounds(i))
                spell.ReqAbility[maxReq++] = requirePair.second;
        }
        ++count;
    }

    // Shrink to actual data size
    packet.Spells.resize(count);

    SendPacket(packet.Write());
}

void WorldSession::HandleTrainerBuySpell(WorldPackets::NPC::TrainerBuySpell& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetNPCIfCanInteractWith(packet.TrainerGUID, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (!unit->isCanTrainingOf(player, true))
    {
        SendTrainerService(packet.TrainerGUID, packet.SpellID, 0);
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        SendTrainerService(packet.TrainerGUID, packet.SpellID, 0);
        return;
    }

    TrainerSpell const* trainer_spell = trainer_spells->Find(packet.SpellID);
    if (!trainer_spell)
    {
        SendTrainerService(packet.TrainerGUID, packet.SpellID, 0);
        return;
    }

    if (player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
    {
        SendTrainerService(packet.TrainerGUID, packet.SpellID, 0);
        return;
    }

    auto nSpellCost = uint32(floor(trainer_spell->spellCost * player->GetReputationPriceDiscount(unit)));

    if (!player->HasEnoughMoney(uint64(nSpellCost)))
    {
        SendTrainerService(packet.TrainerGUID, packet.SpellID, 1);
        return;
    }

    player->ModifyMoney(-int64(nSpellCost));

    unit->SendPlaySpellVisualKit(179, 0);       // 53 SpellCastDirected
    player->SendPlaySpellVisualKit(362, 1);    // 113 EmoteSalute

    if (trainer_spell->IsCastable())
        player->CastSpell(player, trainer_spell->spell, true);
    else
        player->learnSpell(packet.SpellID, false);

    SendTrainerService(packet.TrainerGUID, packet.SpellID, 2);
}

void WorldSession::SendTrainerService(ObjectGuid guid, uint32 spellId, uint32 result)
{
    WorldPackets::NPC::TrainerBuyFailed failed;
    failed.TrainerGUID = guid;
    failed.SpellID = spellId;
    failed.TrainerFailedReason = result;
    SendPacket(failed.Write());
}

void WorldSession::HandleGossipHelloOpcode(WorldPackets::NPC::Hello& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetNPCIfCanInteractWith(packet.Unit, UNIT_NPC_FLAG_NONE);
    if (!unit)
        return;

    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        player->GetReputationMgr().SetVisible(factionTemplateEntry);

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);

    if (unit->isArmorer() || unit->isCivilian() || unit->isQuestGiver() || unit->isServiceProvider() || unit->isGuard())
        unit->StopMoving();

    if (unit->isSpiritGuide())
    {
        Battleground* bg = player->GetBattleground();
        if (bg)
        {
            bg->AddPlayerToResurrectQueue(unit->GetGUID(), player->GetGUID());
            sBattlegroundMgr->SendAreaSpiritHealerQuery(player, bg, unit->GetGUID());
            return;
        }
    }

    if (!sScriptMgr->OnGossipHello(player, unit))
    {
        player->PrepareGossipMenu(unit, unit->GetCreatureTemplate()->GossipMenuId, true);
        player->SendPreparedGossip(unit);
    }

    unit->AI()->sGossipHello(player);
}

void WorldSession::HandleGossipSelectOption(WorldPackets::NPC::GossipSelectOption& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    uint32 _s = getMSTime();
    Creature* unit = nullptr;
    GameObject* go = nullptr;
    if (packet.GossipUnit.IsCreatureOrVehicle())
    {
        unit = player->GetNPCIfCanInteractWith(packet.GossipUnit, UNIT_NPC_FLAG_NONE);
        if (!unit)
            return;
    }
    else if (packet.GossipUnit.IsGameObject())
    {
        go = player->GetMap()->GetGameObject(packet.GossipUnit);
        if (!go)
        {
            TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOption - GameObject (GUID: %s) not found.", packet.GossipUnit.ToString());
            return;
        }
    }
    else
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOption - unsupported GUID %s", packet.GossipUnit.ToString());
        return;
    }

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if ((unit && unit->GetCreatureTemplate()->ScriptID != unit->LastUsedScriptID) || (go && go->GetGOInfo()->ScriptId != go->LastUsedScriptID))
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOption - Script reloaded while in use, ignoring and set new scipt id");
        if (unit)
            unit->LastUsedScriptID = unit->GetCreatureTemplate()->ScriptID;
        if (go)
            go->LastUsedScriptID = go->GetGOInfo()->ScriptId;
        player->PlayerTalkClass->SendCloseGossip();
        return;
    }

    if (!packet.PromotionCode.empty())
    {
        if (unit)
        {
            unit->AI()->sGossipSelectCode(player, packet.GossipID, packet.GossipIndex, packet.PromotionCode.c_str());
            if (!sScriptMgr->OnGossipSelectCode(player, unit, player->PlayerTalkClass->GetGossipOptionSender(packet.GossipIndex), player->PlayerTalkClass->GetGossipOptionAction(packet.GossipIndex), packet.PromotionCode.c_str()))
                player->OnGossipSelect(unit, packet.GossipIndex, packet.GossipID);
        }
        else
        {
            go->AI()->GossipSelectCode(player, packet.GossipID, packet.GossipIndex, packet.PromotionCode.c_str());
            sScriptMgr->OnGossipSelectCode(player, go, player->PlayerTalkClass->GetGossipOptionSender(packet.GossipIndex), player->PlayerTalkClass->GetGossipOptionAction(packet.GossipIndex), packet.PromotionCode.c_str());
        }
    }
    else
    {
        if (unit)
        {
            unit->AI()->sGossipSelect(player, packet.GossipID, packet.GossipIndex);
            if (!sScriptMgr->OnGossipSelect(player, unit, player->PlayerTalkClass->GetGossipOptionSender(packet.GossipIndex), player->PlayerTalkClass->GetGossipOptionAction(packet.GossipIndex)))
                player->OnGossipSelect(unit, packet.GossipIndex, packet.GossipID);
        }
        else
        {
            go->AI()->GossipSelect(player, packet.GossipID, packet.GossipIndex);
            if (!sScriptMgr->OnGossipSelect(player, go, player->PlayerTalkClass->GetGossipOptionSender(packet.GossipIndex), player->PlayerTalkClass->GetGossipOptionAction(packet.GossipIndex)))
                player->OnGossipSelect(go, packet.GossipIndex, packet.GossipID);
        }
    }

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 100)
        sLog->outDiff("HandleGossipSelectOption Entry %u GossipIndex %u GossipID %u wait - %ums", unit ? unit->GetEntry() : 0, packet.GossipIndex, packet.GossipID, _ms);
}

void WorldSession::HandleSpiritHealerActivate(WorldPackets::NPC::SpiritHealerActivate& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetNPCIfCanInteractWith(packet.Healer, UNIT_NPC_FLAG_SPIRITHEALER))
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    Player* player = GetPlayer();
    if (!player)
        return;

    player->ResurrectPlayer(0.5f, true);
    player->DurabilityLossAll(0.25f, true, false);

    WorldSafeLocsEntry const* corpseGrave = nullptr;
    Corpse* corpse = player->GetCorpse();
    if (corpse)
        corpseGrave = sObjectMgr->GetClosestGraveYard(
        corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), player->GetTeam());

    player->SpawnCorpseBones();

    if (corpseGrave) // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    {
        WorldSafeLocsEntry const* ghostGrave = sObjectMgr->GetClosestGraveYard(
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam());

        if (corpseGrave != ghostGrave)
            player->TeleportTo(corpseGrave->MapID, corpseGrave->Loc.X, corpseGrave->Loc.Y, corpseGrave->Loc.Z, player->GetOrientation());
        else
            player->UpdateObjectVisibility();
    }
    else
        player->UpdateObjectVisibility();
}

void WorldSession::HandleBinderActivate(WorldPackets::NPC::Hello& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->IsInWorld() || !player->isAlive())
        return;

    Creature* unit = player->GetNPCIfCanInteractWith(packet.Unit, UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature* npc)
{
    // prevent set homebind to instances in any case
    if (GetPlayer()->GetMap()->Instanceable())
        return;

    uint32 bindspell = 3286;

    // update sql homebind
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_PLAYER_HOMEBIND);
    stmt->setUInt16(0, _player->GetMapId());
    stmt->setUInt16(1, _player->GetCurrentAreaID());
    stmt->setFloat(2, _player->GetPositionX());
    stmt->setFloat(3, _player->GetPositionY());
    stmt->setFloat(4, _player->GetPositionZ());
    stmt->setUInt64(5, _player->GetGUIDLow());
    CharacterDatabase.Execute(stmt);

    _player->m_homebindMapId = _player->GetMapId();
    _player->m_homebindAreaId = _player->GetCurrentAreaID();
    _player->m_homebindX = _player->GetPositionX();
    _player->m_homebindY = _player->GetPositionY();
    _player->m_homebindZ = _player->GetPositionZ();

    // send spell for homebinding (3286)
    _player->CastSpell(_player, bindspell, true);

    SendTrainerService(npc->GetGUID(), bindspell, 2);
    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleListInventory(WorldPackets::NPC::Hello& packet)
{
    if (GetPlayer()->isAlive())
        SendListInventory(packet.Unit);
}

void WorldSession::SendListInventory(ObjectGuid const& vendorGuid, uint32 entry)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* vendor = player->GetNPCIfCanInteractWith(vendorGuid, UNIT_NPC_FLAG_VENDOR);
    if (!vendor)
    {
        player->SendSellError(SELL_ERR_VENDOR_HATES_YOU);
        return;
    }

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (vendor->HasUnitState(UNIT_STATE_MOVING))
        vendor->StopMoving();

    VendorItemData const* vendorItems;
    
    if (entry != 0)
       vendorItems = sObjectMgr->GetNpcDonateVendorItemList(entry);
    else
       vendorItems = vendor->GetVendorItems();
   
    uint32 rawItemCount = vendorItems ? vendorItems->GetItemCount() : 0;

    //if (rawItemCount > 300),
    //    rawItemCount = 300; // client cap but uint8 max value is 255

    WorldPackets::NPC::VendorInventory packet;
    packet.Vendor = vendor->GetGUID();

    packet.Items.resize(rawItemCount);

    const float discountMod = player->GetReputationPriceDiscount(vendor);
    uint32 realCount = 0;
    for (uint32 slot = 0; slot < rawItemCount; ++slot)
    {
        if (realCount >= MAX_VENDOR_ITEMS)
            break;

        VendorItem const* vendorItem = vendorItems->GetItem(slot);
        if (!vendorItem)
            continue;

        WorldPackets::NPC::VendorItem& item = packet.Items[realCount];

        if (vendorItem->Type == ITEM_VENDOR_TYPE_ITEM)
        {
            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(vendorItem->item);
            if (!itemTemplate)
                continue;
                        
            uint32 leftInStock = vendorItem->maxcount <= 0 ? 0xFFFFFFFF : vendor->GetVendorItemCurrentCount(vendorItem);
            if (!player->isGameMaster())
            {
                if (leftInStock == 0)
                    continue;

                ConditionList conditions = sConditionMgr->GetConditionsForNpcVendorEvent(vendor->GetEntry(), vendorItem->item);
                if (!sConditionMgr->IsObjectMeetToConditions(player, vendor, conditions))
                    continue;

                // Check item for all NCP
                conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_NPC_VENDOR, vendorItem->item);
                if (!sConditionMgr->IsObjectMeetToConditions(player, vendor, conditions))
                    continue;

                if (vendorItem->DonateCost == 0)
                    if (!(itemTemplate->AllowableClass & player->getClassMask()) && itemTemplate->GetBonding() == BIND_WHEN_PICKED_UP)
                        continue;

                // Custom MoP Script for Pandarens Mounts (Alliance)
                if (itemTemplate->GetClass() == 15 && itemTemplate->GetSubClass() == 5 && player->getRace() != RACE_PANDAREN_ALLIANCE
                    && player->getRace() != RACE_PANDAREN_HORDE && player->getRace() != RACE_PANDAREN_NEUTRAL
                    && vendor->GetEntry() == 65068 && player->GetReputationRank(1353) != REP_EXALTED)
                    continue;

                // Custom MoP Script for Pandarens Mounts (Horde)
                if (itemTemplate->GetClass() == 15 && itemTemplate->GetSubClass() == 5 && player->getRace() != RACE_PANDAREN_ALLIANCE
                    && player->getRace() != RACE_PANDAREN_HORDE && player->getRace() != RACE_PANDAREN_NEUTRAL
                    && vendor->GetEntry() == 66022 && player->GetReputationRank(1352) != REP_EXALTED)
                    continue;

                // Only display items in vendor lists for the team the player is on
                if ((itemTemplate->GetFlags2() & ITEM_FLAG2_FACTION_HORDE && player->GetTeam() == ALLIANCE) ||
                    (itemTemplate->GetFlags2() & ITEM_FLAG2_FACTION_ALLIANCE && player->GetTeam() == HORDE))
                    continue;

                if (itemTemplate->GetClass() == 15 && itemTemplate->GetSubClass() == 5 && itemTemplate->Effects.size() == 1 && itemTemplate->Effects[0])
                    if (auto spellInfo = sSpellMgr->GetSpellInfo(itemTemplate->Effects[0]->SpellID))
                        if ((spellInfo->HasAttribute(SPELL_ATTR7_HORDE_ONLY) && (player->getRaceMask() & RACEMASK_HORDE) == 0) ||
                            (spellInfo->HasAttribute(SPELL_ATTR7_ALLIANCE_ONLY) && (player->getRaceMask() & RACEMASK_ALLIANCE) == 0))
                            continue;

                std::vector<GuildReward> const& rewards = sGuildMgr->GetGuildRewards();
                bool guildRewardCheckPassed = true;
                
                if (!entry)
                {
                    for (const auto & reward : rewards)
                    {
                        if (itemTemplate->GetId() != reward.Entry)
                            continue;

                        Guild* guild = sGuildMgr->GetGuildById(_player->GetGuildId());
                        if (!guild)
                        {
                            guildRewardCheckPassed = false;
                            break;
                        }

                        if (reward.Standing && player->GetReputationRank(REP_GUILD) < reward.Standing)
                        {
                            guildRewardCheckPassed = false;
                            break;
                        }

                        for (uint32 const id : reward.AchievementsRequired)
                            if (!guild->GetAchievementMgr().HasAchieved(id))
                            {
                                guildRewardCheckPassed = false;
                                break;
                            }

                        if (reward.Racemask)
                            if (!(player->getRaceMask() & reward.Racemask))
                            {
                                guildRewardCheckPassed = false;
                                break;
                            }
                    }
                }
                if (!guildRewardCheckPassed)
                    continue;
            }

            uint64 price = 0;
            if (vendorItem->Money != 0)
                price = vendorItem->Money;
            else if (vendorItem->IsGoldRequired(itemTemplate))
                price = uint64(itemTemplate->GetBuyPrice() * discountMod);

            //if (int32 priceMod = player->GetTotalAuraModifier(SPELL_AURA_MOD_VENDOR_ITEMS_PRICES))
                 //price -= CalculatePct(price, priceMod);

            //Hack for donate
            if (vendorItem->DonateCost != 0)
            {
                price = vendorItem->DonateCost * 10000;
            }
            
            item.MuID = slot + 1;
            item.Durability = itemTemplate->MaxDurability ? itemTemplate->MaxDurability : -1;
            item.ExtendedCostID = vendorItem->ExtendedCost;
            item.DoNotFilterOnVendor = vendorItem->DoNotFilterOnVendor;
            if (vendorItem->PlayerConditionID && !sConditionMgr->IsPlayerMeetingCondition(player, vendorItem->PlayerConditionID))
                item.PlayerConditionFailed = vendorItem->PlayerConditionID;
            item.Type = vendorItem->Type;
            item.Quantity = leftInStock;
            item.StackCount = itemTemplate->VendorStackCount;
            item.Price = price;
            item.Item.Initialize(vendorItem);
        }
        else if (vendorItem->Type == ITEM_VENDOR_TYPE_CURRENCY)
        {
            CurrencyTypesEntry const* currencyTemplate = sCurrencyTypesStore.LookupEntry(vendorItem->item);
            if (!currencyTemplate)
                continue;

            if (!vendorItem->ExtendedCost && !vendorItem->Money)
                continue;

            item.MuID = slot + 1; // client expects counting to start at 1
            item.ExtendedCostID = vendorItem->ExtendedCost;
            item.DoNotFilterOnVendor = vendorItem->DoNotFilterOnVendor;
            if (vendorItem->PlayerConditionID && !sConditionMgr->IsPlayerMeetingCondition(player, vendorItem->PlayerConditionID))
                item.PlayerConditionFailed = vendorItem->PlayerConditionID;
            item.Item.ItemID = vendorItem->item;
            item.Type = vendorItem->Type;
            item.Price = vendorItem->Money;
            item.StackCount = vendorItem->maxcount * sDB2Manager.GetCurrencyPrecision(currencyTemplate->ID);
        }

        if (++realCount >= MAX_VENDOR_ITEMS)
            break;
    }

    packet.Items.resize(realCount);
    SendPacket(packet.Write());
}

void WorldSession::HandleRequestStabledPets(WorldPackets::NPC::RequestStabledPets& packet)
{
    if (!GetPlayer()->GetNPCIfCanInteractWith(packet.StableMaster, UNIT_NPC_FLAG_STABLEMASTER))
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Stablemaster (GUID:%u) not found or you can't interact with him.", packet.StableMaster.GetGUIDLow());
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);

    SendStablePet(packet.StableMaster);
}

void WorldSession::SendOpenAlliedRaceDetails(ObjectGuid const& guid, uint32 RaceID)
{
    WorldPackets::NPC::OpenAlliedRaceDetails packet;
    packet.SenderGUID = guid;
    packet.RaceID = RaceID;
    SendPacket(packet.Write());
}
