#include "BattlegroundDM.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "ChatPackets.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"

void BattlegroundDeathMatch::Reset()
{
    Battleground::Reset();
}

void BattlegroundDeathMatch::StartingEventCloseDoors()
{
    for (uint32 i = 0; i < 6; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);
}

void BattlegroundDeathMatch::StartingEventOpenDoors()
{
    for (uint32 i = 0; i < 6; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
    
    timer_for_end  = 1200000; // 20 minutes
}

bool BattlegroundDeathMatch::SetupBattleground()
{
    for (uint8 i = 0; i < 6; ++i)
    {
        if (!AddObject(i, DM_Buffs[urand(0, 2)], dm_buf_pos[i].GetPositionX(), dm_buf_pos[i].GetPositionY(), dm_buf_pos[i].GetPositionZ(), dm_buf_pos[i].GetOrientation(), 0, 0, 0.7313537f, -0.6819983f, BUFF_RESPAWN_TIME))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn some object Battleground not created!");
            return false;
        }
    }
        
    return true;
}

void BattlegroundDeathMatch::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundDMScore(player->GetGUID(), player->GetBGTeamId());

    if (Group* group = player->GetGroup())
        group->RemoveMember(player->GetGUID());
    
    //if (!IsRated())
    //{
    //    RemoveFromBGFreeSlotQueue();
    //    PreventAddingToQueueAgained(true);
    //}
}

void BattlegroundDeathMatch::OnPlayerEnter(Player* player)
{
    // Battleground::OnPlayerEnter(player);
/*
    if (WorldSafeLocsEntry const* entry = GetClosestGraveYard(player))
        player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);*/

    static uint32 const BgAbGraveyardIds[7] = { 895, 894, 893, 897, 896, 898, 899 };
    if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(BgAbGraveyardIds[0]))
        player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);
    
    player->SetByteValue(PLAYER_FIELD_BYTES_6, PLAYER_BYTES_6_OFFSET_ARENA_FACTION, DMTeam++);
    player->UpdatePvPState(false);
    
    strike[player->GetGUID()] = 0;
    temp_strike[player->GetGUID()] = {0,getMSTime()};
    
    if (DeathMatchScore* dmscore =  player->getDeathMatchScore())
    {
        if (dmscore->selectedMorph)
        {
            player->SetCustomDisplayId(dmscore->selectedMorph);
            player->SetDisplayId(dmscore->selectedMorph, true);
        }
    }
}

void BattlegroundDeathMatch::RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket)
{
    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {        
        std::map<ObjectGuid, BattlegroundScore*> bgsm = GetBattlegroundScoreMap();
        std::map<ObjectGuid, BattlegroundScore*>::iterator itr = bgsm.find(guid);
        if (itr != bgsm.end() /*&& IsRated()*/)
        {
            player->ModifyDeathMatchStats(itr->second->GetScore(SCORE_KILLING_BLOWS), itr->second->GetScore(SCORE_DEATHS), itr->second->GetScore(SCORE_DAMAGE_DONE),
                CalculateRating(itr->second), itr->second->GetScore(SCORE_KILLING_BLOWS));
        }
        
        player->RemoveAura(SPELL_FOCUSED_ASSAULT);
        
        if (DeathMatchScore* dmscore =  player->getDeathMatchScore())
        {
            if (dmscore->selectedMorph)
            {
                player->DeMorph();
                player->ResetCustomDisplayId();
                player->SetObjectScale(1.0f);
            }
        }
    }
    auto itr = strike.find(guid);
    if (itr != strike.end())
        strike.erase(itr);
    
    auto itr_t = temp_strike.find(guid);
    if (itr_t != temp_strike.end())
        temp_strike.erase(itr_t);
    
    Battleground::RemovePlayerAtLeave(guid, Transport, SendPacket);
}

WorldSafeLocsEntry const* BattlegroundDeathMatch::GetClosestGraveYard(Player* /*player*/)
{
    uint32 max_zone = 6248;
    if (GetBattlegroundScoreMap().size() < 20)
        max_zone = 6045;
    else if (GetBattlegroundScoreMap().size() < 30)
        max_zone = 6100;
    else if (GetBattlegroundScoreMap().size() < 40)
        max_zone = 6200;
    if (WorldSafeLocsEntry const* random = sWorldSafeLocsStore.LookupEntry(urand(6032, max_zone)))
        return random;

    return sWorldSafeLocsStore.LookupEntry(6032);
}

void BattlegroundDeathMatch::HandleKillPlayer(Player* victim, Player* killer)
{
    // get strike kills for this player
    uint32 victim_kills = 0;
	if (!strike.empty())
	{
		auto itr = strike.find(victim->GetGUID());
		if (itr != strike.end())
			std::swap(victim_kills, itr->second);
	}
        
    victim->RemoveAura(SPELL_FOCUSED_ASSAULT);
    
    // Add +1 deaths
    UpdatePlayerScore(victim, SCORE_DEATHS, 1, true);

    if (killer)
    {
        // Don't reward credit for killing ourselves, like fall damage of hellfire (warlock)
        if (killer == victim)
            return;

        // if victim was high cost -> announce + reward
        if (victim_kills >= KILLS_FOR_HIGH_COST)
        {
            SendSysMessageToAll(TEXT_LOSE_STRIKE_BY, victim, killer);
            
            //if (IsRated())
            //{
                if (victim_kills >= 2* KILLS_FOR_HIGH_COST)
                    killer->RewardHonor(nullptr, 1, 5);
                else
                    killer->RewardHonor(nullptr, 1, 10);
            //}
        }
        else // common announce
            SendSysMessageToAll(TEXT_KILLED_BY, victim, killer);
        
        // if fast strike, then announce about it
		if (!temp_strike.empty())
		{
			auto itr = temp_strike.find(killer->GetGUID());
			if (itr != temp_strike.end())
			{
				if (GetMSTimeDiffToNow(itr->second.second) < BASE_TIME_FOR_STRIKE) // time isn't up
				{
					switch (++(itr->second.first))
					{
                        case 1: break;
                        case 2: SendSysMessageToAll(TEXT_DOUBLE_KILL, killer);  break;
                        case 3: SendSysMessageToAll(TEXT_TRIPLE_KILL, killer); SendDirectMessageToAll(TEXT_TRIPLE_KILL, killer); break;
                        case 4: SendSysMessageToAll(TEXT_ULTRA_KILL, killer); SendDirectMessageToAll(TEXT_ULTRA_KILL, killer);  break;
                        case 5:
                            SendSysMessageToAll(TEXT_RAMPAGE, killer);
                            SendDirectMessageToAll(TEXT_RAMPAGE, killer);
                            break;
					}
                    itr->second.second = getMSTime();
				}
				else // time is up, let's start again
				{
					itr->second.first = 1;
					itr->second.second = getMSTime();
				}
			}
		}
            
        // if strike, then announce about it
        if (!strike.empty())
		{
			auto itr = strike.find(killer->GetGUID());
			if (itr != strike.end())
			{
				if (++(itr->second) >= 3)
				{
					switch (itr->second)
					{
					    case 3: SendSysMessageToAll(TEXT_KILLING_SPREE, killer); break;
					    case 4: SendSysMessageToAll(TEXT_DOMINATING, killer); SendDirectMessageToAll(TEXT_DOMINATING, killer); break;
					    case 5: SendSysMessageToAll(TEXT_MEGA_KILL, killer); SendDirectMessageToAll(TEXT_MEGA_KILL, killer); break;
					    case 6: SendSysMessageToAll(TEXT_UNSTOPPABLE, killer); SendDirectMessageToAll(TEXT_UNSTOPPABLE, killer); break;
					    case 7: SendSysMessageToAll(TEXT_WICKED_SICK, killer); SendDirectMessageToAll(TEXT_WICKED_SICK, killer); break;
					    case 8: SendSysMessageToAll(TEXT_MONSTER_KILL, killer); SendDirectMessageToAll(TEXT_MONSTER_KILL, killer); break;
					    case 9: SendSysMessageToAll(TEXT_GOD_LIKE, killer); SendDirectMessageToAll(TEXT_GOD_LIKE, killer); break;
					    case 10:
						    SendSysMessageToAll(TEXT_BEYOUND_GOD_LIKE, killer);
						    SendDirectMessageToAll(TEXT_BEYOUND_GOD_LIKE, killer);
						    break;
					    default: // > 10
						    if (urand(1, 3) == 2)
						    {
							    SendSysMessageToAll(TEXT_BEYOUND_GOD_LIKE, killer);
							    SendDirectMessageToAll(TEXT_BEYOUND_GOD_LIKE, killer);
						    }
						    break;
					}
				}
			}
                
            // add aura for increasing taken damage
            if (itr->second >= 5 && itr->second % 2 == 1) // only 5, 7, 9,.......
                killer->AddAura(SPELL_FOCUSED_ASSAULT, killer);
        }
            
        // common update
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1,  true);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1, true);
        
        killer->KilledMonsterCredit(230003);
    }
    
    victim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    RewardXPAtKill(killer, victim);
}


void BattlegroundDeathMatch::EndBattleground(uint32 team)
{   
    Battleground::EndBattleground(WINNER_NONE); // not needed team at Deathmatch
    
    //if (!IsRated())
    //    return;
    PvpRewardTypes type = PvpReward_DeathMatch;
    PvpReward* reward = sBattlegroundMgr->GetPvpReward(type);
    if (reward && GetPlayerScoresSize())
    {
        std::map<ObjectGuid, BattlegroundScore*> bgsm = GetBattlegroundScoreMap();
        uint8 max = GetPlayerScoresSize();
        for (uint8 i = 0; i < (max >= 4 ? 4 : max); ++i)
        {
            std::map<ObjectGuid, BattlegroundScore*>::const_iterator Max_itr = bgsm.begin();
            
            if (Max_itr == bgsm.end()) // end?
                break;
            
            for (std::map<ObjectGuid, BattlegroundScore*>::const_iterator itr = bgsm.begin(); itr != bgsm.end(); ++itr)
            {
                if (i == 3) // last going and we need reward other players
                {
                    if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr->first))
                    {
                        bool isAlliance = player->GetTeam() == ALLIANCE;
                        if (roll_chance_f(reward->ChestChance))
                        {
                            uint32 chestId = isAlliance ? reward->ChestA : reward->ChestH;
                            ItemPosCountVec dest;
                            if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, chestId, 1) == EQUIP_ERR_OK)
                            {
                                if (auto item = player->StoreNewItem(dest, chestId, true))
                                {
                                    player->SendNewItem(item, 1, true, false, true);
                                    //player->SendDisplayToast(chestId, ToastType::ITEM, false, 1, DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
                                }
                            }
                        }
                    }
                }
                else if (itr->second->GetScore(SCORE_KILLING_BLOWS) >= Max_itr->second->GetScore(SCORE_KILLING_BLOWS))  // not last going and we try to find best player
                    Max_itr = itr;
            }
            
            if (i != 3 && Max_itr != bgsm.end()) // i think, that it can't be useless
            {
                if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), Max_itr->first))
                {
                    player->KilledMonsterCredit(240060);
                    // it's like common rewards
                    bool isAlliance = player->GetTeam() == ALLIANCE;
                    
                    uint32 chest_chance = 100; // first place
                    uint32 artifactRewardItem = 147203; 
                    switch(i)
                    {
                        case 1: // second place
                            chest_chance = 50;
                            artifactRewardItem = 147200;
                            break;
                        case 2: // thir place
                            chest_chance = 25;
                            artifactRewardItem = 143680;
                            break;
                    }
                    
                    if (roll_chance_f(chest_chance))
                    {
                        uint32 chestId = isAlliance ? reward->ChestA : reward->ChestH;
                        ItemPosCountVec dest;
                        if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, chestId, 1) == EQUIP_ERR_OK)
                        {
                            if (auto item = player->StoreNewItem(dest, chestId, true))
                            {
                                player->SendNewItem(item, 1, true, false, true);
                                //player->SendDisplayToast(chestId, ToastType::ITEM, false, 1, DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
                            }
                        }
                    }   
                    

                    ItemPosCountVec dest;
                    if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, artifactRewardItem, 1) == EQUIP_ERR_OK)
                    {
                        if (auto item = player->StoreNewItem(dest, artifactRewardItem, true))
                        {
                            player->SendNewItem(item, 1, true, false, true);
                            //player->SendDisplayToast(artifactRewardItem, ToastType::ITEM, false, 1, DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
                        }
                    }
    
                    if (i == 0) // first
                    {
                        if (!roll_chance_f(reward->ItemsChance)) // Roll can take item
                            return;
                            
                        uint32 playerSpecID = player->GetLootSpecID();
                        uint8 playerLevel = player->getLevel();

                        uint32 itemId = 0;
                        uint32 relicId = 0;

                        std::vector<uint32> itemsContainer = isAlliance ? reward->ItemsA : reward->ItemsH;

                        int32 needLevel = reward->BaseLevel;
                        
                        if (!itemsContainer.empty())
                        {
                            std::vector<uint32> possibleLoot;
                            std::vector<uint32> possibleLootClone;
                            for (uint32 itemID : itemsContainer)
                            {
                                auto const& itemTemplate = sObjectMgr->GetItemTemplate(itemID);
                                if (!itemTemplate)
                                    continue;

                                if (!player->CanGetItemForLoot(itemTemplate))
                                    continue;

                                if (player->HasItemCount(itemID, 1, true)) // Prevent drop if item allready exist
                                {
                                    possibleLootClone.emplace_back(itemID);
                                    continue;
                                }

                                possibleLoot.emplace_back(itemID);
                                possibleLootClone.emplace_back(itemID);
                            }

                            if (roll_chance_f(30.0f)) // Not full chance
                                possibleLoot = possibleLootClone;

                            if (!possibleLoot.empty()) // If not have item for looting copy clone
                                possibleLoot = possibleLootClone;

                            if (relicId)
                                possibleLoot.emplace_back(relicId);

                            if (!possibleLoot.empty())
                                itemId = Trinity::Containers::SelectRandomContainerElement(possibleLoot);
                        }

                        auto const& _proto = sObjectMgr->GetItemTemplate(itemId);
                        if (!_proto)
                            return;

                        std::vector<uint32> itemModifiers = sObjectMgr->GetItemBonusTree(itemId, FindBgMap()->GetDifficultyLootItemContext(), playerLevel, 0, 0, needLevel);

                        if (itemId)
                        {
                            ItemPosCountVec dest;
                            if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1) == EQUIP_ERR_OK)
                            {
                                if (auto item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId, playerSpecID), GuidSet(), itemModifiers))
                                {
                                    player->SendNewItem(item, 1, true, false, true);
                                    player->SendDisplayToast(itemId, ToastType::ITEM, false, 1, DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
                                }
                            }
                        }
                    }
                }
                bgsm.erase(Max_itr); // delete this position. don't needed
            }
        }
    }
}


int32 BattlegroundDeathMatch::CalculateRating(uint32 kills, uint32 dies, uint64 dmg)
{
    if (!dmg || !kills)
        return 0;

    // Not in vain I'm studying at the faculty of mathematics  =D
    uint64 effective = kills * MIDDLE_HP / dmg; // this is how we regulate the number of kills with low damage and vice versa

    if (effective == 0)
        return 0;

    int32 coef = effective * STEP_RATING / KILLS_PER_STEP - (dies / effective)*STEP_RATING;
    return coef;
}

void BattlegroundDeathMatch::SendSysMessageToAll(uint32 textid, Player* first, Player* second)
{
    switch(textid) // some sounds
    {
        case TEXT_KILLED_BY:
            PlaySoundToAll(BG_SOUND_FLAG_RESET);
            break;
        case TEXT_LOSE_STRIKE_BY:
            PlaySoundToAll(BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE);
            break;
    }
    
    for (std::map<ObjectGuid, BattlegroundPlayer>::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
    {
        Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr->first);
        if (!player)
            continue;
        
        ChatHandler chH = ChatHandler(player);
        LocaleConstant loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
        
        if (!first)
        {
            WorldPackets::Chat::Chat packet;
            packet.Initialize(CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, player, player, sObjectMgr->GetTrinityString(textid, loc_idx));
            player->SendDirectMessage(packet.Write());
        }
        else
        {
			char str[500];

            if (!second)
                snprintf(str, 500, sObjectMgr->GetTrinityString(textid, loc_idx), chH.GetNameLink(first).c_str());
            else
                snprintf(str, 500, sObjectMgr->GetTrinityString(textid, loc_idx), chH.GetNameLink(first).c_str(), chH.GetNameLink(second).c_str());
            
            WorldPackets::Chat::Chat packet;
            packet.Initialize(urand(1,2) == 1 ? CHAT_MSG_BG_SYSTEM_HORDE : CHAT_MSG_BG_SYSTEM_ALLIANCE, LANG_UNIVERSAL, player, player, str);
            player->SendDirectMessage(packet.Write());
        }
    }
}

void BattlegroundDeathMatch::SendDirectMessageToAll(uint32 textid, Player* first)
{
    if (!first)
        return;
    
    PlaySoundToAll(BG_SOUND_START); // some sounds
    
    for (std::map<ObjectGuid, BattlegroundPlayer>::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
    {
        Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr->first);
        if (!player)
            continue;
        
        ChatHandler chH = ChatHandler(player);
        LocaleConstant loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
        char str[500];
        snprintf(str, 500, sObjectMgr->GetTrinityString(textid, loc_idx), chH.GetNameLink(first).c_str());
        player->SendDirectMessage(WorldPackets::Chat::PrintNotification(str).Write());
    }
}

void BattlegroundDeathMatch::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (timer_for_end <= diff)
            EndBattleground(WINNER_NONE);
        else
        {
            timer_for_end -= diff;
            if (small_delayed_timer <= diff)
            {
                for (std::map<ObjectGuid, BattlegroundPlayer>::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
                {
                    Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr->first);
                    if (!player)
                        continue;
                    
                    ChatHandler chH = ChatHandler(player);
                    
                    chH.PSendSysMessage(20214, timer_for_end/1000);
                }
                small_delayed_timer = urand(20000, 60000);
            }
            else
                small_delayed_timer -= diff;
        }
    }
}
