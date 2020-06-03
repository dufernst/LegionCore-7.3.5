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

#include "BattlegroundSilvershardMines.h"
#include "Creature.h"
#include "GameObject.h"
#include "Object.h"
#include "BattlegroundMgr.h"
#include "Player.h"
#include "MoveSplineInit.h"
#include "CreatureAIImpl.h"
#include "WorldStatePackets.h"

void BattleGroundSSMScore::UpdateScore(uint32 type, uint32 value)
{
    switch (type)
    {
        case SCORE_CARTS_HELPED:
            CartsTaken += value;
            break;
        default:
            BattlegroundScore::UpdateScore(type, value);
            break;
    }
}

void BattleGroundSSMScore::BuildObjectivesBlock(std::vector<int32>& stats)
{
    stats.push_back(CartsTaken);
}

BattlegroundSilvershardMines::BattlegroundSilvershardMines() : _honorTics(0), _mineCartSpawnTimer(0), _mineCartCheckTimer(0), _mineCartAddPointsTimer(0), _waterfallPathDone(false)
{
    m_BuffChange = true;
    BgObjects.resize(BG_SSM_OBJECT_MAX);
    BgCreatures.resize(BG_SSM_CREATURES_MAX);
}

BattlegroundSilvershardMines::~BattlegroundSilvershardMines() { }

void BattlegroundSilvershardMines::Reset()
{
    Battleground::Reset();

    _honorScoreTics[TEAM_ALLIANCE] = 0;
    _honorScoreTics[TEAM_HORDE] = 0;
    _mineCartCheckTimer = 1000;
    _honorTics = 5;
    _mineCartSpawnTimer = 10 * IN_MILLISECONDS;
    _mineCartAddPointsTimer = 2000;
    _waterfallPathDone = false;
    _trackSwitch[SSM_EAST_TRACK_SWITCH] = false;
    _trackSwitch[SSM_NORTH_TRACK_SWITCH] = false;
    _trackSwitchClickTimer[SSM_EAST_TRACK_SWITCH] = 3000;
    _trackSwitchClickTimer[SSM_NORTH_TRACK_SWITCH] = 3000;
    _trackSwitchCanInterract[SSM_EAST_TRACK_SWITCH] = true;
    _trackSwitchCanInterract[SSM_NORTH_TRACK_SWITCH] = true;

    for (uint8 i = 0; i < MaxCarts; ++i)
    {
        _mineCartsProgressBar[i] = BG_SSM_PROGRESS_BAR_NEUTRAL;
        _mineCartReachedDepot[i] = false;
        _mineCartNearDepot[i] = false;
    }

    for (uint8 i = 0; i < 4; ++i)
    {
        _depot[i] = false;
        _depotCloseTimer[i] = 3000;
    }

    _pathDone[SSM_EAST_PATH][0] = false;
    _pathDone[SSM_EAST_PATH][1] = false;
    _pathDone[SSM_NORTH_PATH][0] = false;
    _pathDone[SSM_NORTH_PATH][1] = false;

    _cartsMap.clear();
}

void BattlegroundSilvershardMines::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        CheckPlayerNearMineCart(diff);
        MineCartAddPoints(diff);
        CheckTrackSwitch(diff);
    }

    SummonMineCart(diff);
    CheckMineCartNearDepot(diff);
    EventReopenDepot(diff);

    if (!_cartsMap[BG_SSM_MINE_CART_1 - 1])
    {
        _pathDone[SSM_EAST_PATH][0] = false;
        _pathDone[SSM_EAST_PATH][1] = false;
    }

    if (!_cartsMap[BG_SSM_MINE_CART_2 - 1])
        _waterfallPathDone = false;

    if (!_cartsMap[BG_SSM_MINE_CART_3 - 1])
    {
        _pathDone[SSM_NORTH_PATH][0] = false;
        _pathDone[SSM_NORTH_PATH][1] = false;
    }

    if (!_trackSwitchCanInterract[SSM_EAST_TRACK_SWITCH])
    {
        if (_trackSwitchClickTimer[SSM_EAST_TRACK_SWITCH] <= 0)
        {
            if (Creature* track = GetBgMap()->GetCreature(BgCreatures[SSM_TRACK_SWITCH_EAST]))
            {
                //for (auto itr : GetPlayers())
                //    if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                //        if (player->GetExactDist2d(track->GetPositionX(), track->GetPositionY()) <= 10.0f)
                //            player->PlayerTalkClass->SendCloseGossip();

                track->RemoveAurasDueToSpell(BG_SSM_PREVENTION_AURA);
                _trackSwitchCanInterract[SSM_EAST_TRACK_SWITCH] = true;
            }
        }
        else
            _trackSwitchClickTimer[SSM_EAST_TRACK_SWITCH] -= diff;
    }

    if (!_trackSwitchCanInterract[SSM_NORTH_TRACK_SWITCH])
    {
        if (_trackSwitchClickTimer[SSM_NORTH_TRACK_SWITCH] <= 0)
        {
            if (Creature* track = GetBgMap()->GetCreature(BgCreatures[SSM_TRACK_SWITCH_NORTH]))
            {
                // for (auto itr : GetPlayers())
                //    if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                //        if (player->GetExactDist2d(track->GetPositionX(), track->GetPositionY()) <= 10.0f)
                //            player->PlayerTalkClass->SendCloseGossip();

                track->RemoveAurasDueToSpell(BG_SSM_PREVENTION_AURA);
                _trackSwitchCanInterract[SSM_NORTH_TRACK_SWITCH] = true;
            }
        }
        else
            _trackSwitchClickTimer[SSM_NORTH_TRACK_SWITCH] -= diff;
    }

    if (!_pathDone[SSM_EAST_PATH][0])
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_1))
        {
            cart->GetMotionMaster()->MovePath(60378100, false);
            _pathDone[SSM_EAST_PATH][0] = true;
        }

    if (_pathDone[SSM_EAST_PATH][0] && !_pathDone[SSM_EAST_PATH][1])
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_1))
            if (cart->GetExactDist2d(717.169312f, 114.258339f) < 5.0f) // East pos
                if (Creature* track = GetBgMap()->GetCreature(BgCreatures[SSM_TRACK_SWITCH_EAST]))
                    //if (track->HasAura(BG_SSM_TRACK_SWITCH_OPENED))
                {
                    cart->GetMotionMaster()->Clear(true);
                    cart->GetMotionMaster()->MovePath(60378101, false);
                    _pathDone[SSM_EAST_PATH][1] = true;
                }
    //else
    //{
    //    cart->GetMotionMaster()->Clear(true);
    //    cart->GetMotionMaster()->MovePath(60378101/*60378102*/, false);
    //    _pathDone[SSM_EAST_PATH][1] = true;
    //}

    if (!_waterfallPathDone)
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_2))
        {
            cart->GetMotionMaster()->MovePath(60379100, false);
            _waterfallPathDone = true;
        }

    if (!_pathDone[SSM_NORTH_PATH][0])
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_3))
        {
            cart->GetMotionMaster()->MovePath(60380100, false);
            _pathDone[SSM_NORTH_PATH][0] = true;
        }

    if (_pathDone[SSM_NORTH_PATH][0] && !_pathDone[SSM_NORTH_PATH][1])
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_3))
            if (cart->GetExactDist2d(834.727234f, 299.809753f) < 5.0f) // North pos
                if (Creature* track = GetBgMap()->GetCreature(BgCreatures[SSM_TRACK_SWITCH_NORTH]))
                    //if (track->HasAura(BG_SSM_TRACK_SWITCH_CLOSED))
                {
                    cart->GetMotionMaster()->Clear(true);
                    cart->GetMotionMaster()->MovePath(60380101, false);
                    _pathDone[SSM_NORTH_PATH][1] = true;
                }
    //else
    //{
    //    cart->GetMotionMaster()->Clear(true);
    //    cart->GetMotionMaster()->MovePath(60380101/*60380102*/, false);
    //    _pathDone[SSM_NORTH_PATH][1] = true;
    //}
}

void BattlegroundSilvershardMines::StartingEventCloseDoors()
{
    for (uint8 doorType = BG_SSM_OBJECT_DOOR_A_1; doorType <= BG_SSM_OBJECT_DOOR_H_2; ++doorType)
    {
        DoorClose(doorType);
        SpawnBGObject(doorType, RESPAWN_IMMEDIATELY);
    }

    for (uint8 i = BG_SSM_OBJECT_WATERFALL_DEPOT; i < BG_SSM_OBJECT_MAX; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);
}

void BattlegroundSilvershardMines::CheckTrackSwitch(uint32 diff)
{
    if (_trackSwitchCanInterract[SSM_EAST_TRACK_SWITCH])
    {
        if (GetBgMap()->GetCreature(BgCreatures[SSM_MINE_CART_TRIGGER]))
        {
            if (Creature* track = GetBgMap()->GetCreature(BgCreatures[SSM_TRACK_SWITCH_EAST]))
            {
                if (track->HasAura(BG_SSM_TRACK_SWITCH_OPENED) && !_trackSwitch[SSM_EAST_TRACK_SWITCH])
                {
                    SendBroadcastText(60030, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                    _trackSwitchClickTimer[SSM_EAST_TRACK_SWITCH] = 3000;
                    _trackSwitch[SSM_EAST_TRACK_SWITCH] = true;
                    _trackSwitchCanInterract[SSM_EAST_TRACK_SWITCH] = false;
                    UpdateWorldState(WorldStates::WS_SSM_EAST_TRACK_SWITCH, _trackSwitch[SSM_EAST_TRACK_SWITCH] ? 2 : 1);
                }

                if (track->HasAura(BG_SSM_TRACK_SWITCH_CLOSED) && _trackSwitch[SSM_EAST_TRACK_SWITCH])
                {
                    SendBroadcastText(60030, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                    _trackSwitchClickTimer[SSM_EAST_TRACK_SWITCH] = 3000;
                    _trackSwitch[SSM_EAST_TRACK_SWITCH] = false;
                    _trackSwitchCanInterract[SSM_EAST_TRACK_SWITCH] = false;
                    UpdateWorldState(WorldStates::WS_SSM_EAST_TRACK_SWITCH, _trackSwitch[SSM_EAST_TRACK_SWITCH] ? 1 : 2);
                }
            }
        }
    }

    if (_trackSwitchCanInterract[SSM_NORTH_TRACK_SWITCH])
    {
        if (GetBgMap()->GetCreature(BgCreatures[SSM_MINE_CART_TRIGGER]))
        {
            if (Creature* track = GetBgMap()->GetCreature(BgCreatures[SSM_TRACK_SWITCH_NORTH]))
            {
                if (track->HasAura(BG_SSM_TRACK_SWITCH_CLOSED) && _trackSwitch[SSM_NORTH_TRACK_SWITCH])
                {
                    SendBroadcastText(60032, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                    _trackSwitchClickTimer[SSM_NORTH_TRACK_SWITCH] = 3000;
                    _trackSwitch[SSM_NORTH_TRACK_SWITCH] = false;
                    _trackSwitchCanInterract[SSM_NORTH_TRACK_SWITCH] = false;
                    UpdateWorldState(WorldStates::WS_SSM_NORTH_TRACK_SWITCH, _trackSwitch[SSM_NORTH_TRACK_SWITCH] ? 2 : 1);
                }

                if (track->HasAura(BG_SSM_TRACK_SWITCH_OPENED) && !_trackSwitch[SSM_NORTH_TRACK_SWITCH])
                {
                    SendBroadcastText(60032, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                    _trackSwitchClickTimer[SSM_NORTH_TRACK_SWITCH] = 3000;
                    _trackSwitch[SSM_NORTH_TRACK_SWITCH] = true;
                    _trackSwitchCanInterract[SSM_NORTH_TRACK_SWITCH] = false;
                    UpdateWorldState(WorldStates::WS_SSM_NORTH_TRACK_SWITCH, _trackSwitch[SSM_NORTH_TRACK_SWITCH] ? 1 : 2);
                }
            }
        }
    }
}

void BattlegroundSilvershardMines::StartingEventOpenDoors()
{
    for (uint8 doorType = BG_SSM_OBJECT_DOOR_A_1; doorType <= BG_SSM_OBJECT_DOOR_H_2; ++doorType)
        DoorOpen(doorType);

    for (uint8 i = BG_SSM_OBJECT_WATERFALL_DEPOT; i < BG_SSM_OBJECT_MAX; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
}

void BattlegroundSilvershardMines::SummonMineCart(uint32 diff)
{
    if (_mineCartSpawnTimer <= 0)
    {
        uint8 mineCart = 0;
        if (!IsBrawl())
        {
            if (!GetMineCart(BG_SSM_MINE_CART_1))
                mineCart = BG_SSM_MINE_CART_1;
            else if (!GetMineCart(BG_SSM_MINE_CART_2))
                mineCart = BG_SSM_MINE_CART_2;
            else if (!GetMineCart(BG_SSM_MINE_CART_3))
                mineCart = BG_SSM_MINE_CART_3;
            else
                return;
        }
        else
        {
            uint8 cartCount = 0;
            for (uint8 i = BG_SSM_MINE_CART_1; i <= BG_SSM_MINE_CART_3; ++i)
                if (GetMineCart(i))
                    ++cartCount;

            if (cartCount < 2)
                mineCart = m_brawlCarSelector.GetCurrentCar();
            else
                return;
        }

        mineCart -= 1;

        if (Creature* trigger = GetBgMap()->GetCreature(BgCreatures[SSM_MINE_CART_TRIGGER]))
        {
            struct MineCart
            {
                BgSSMMineCarts Cart;
                Position Pos;
                uint32 ControlSpellID;
            };

            MineCart _carts[BG_SSM_MINE_CARTS_MAX] =
            {
                {BG_SSM_MINE_CART_1, Position(744.542053f, 183.545883f, 319.658203f, 4.356342f), BG_SSM_SPELL_CART_CONTROL_CAPTURE_POINT_UNIT_NORTH},
                {BG_SSM_MINE_CART_2, Position(739.400330f, 203.598511f, 319.603333f, 2.308198f), BG_SSM_SPELL_CART_CONTROL_CAPTURE_POINT_UNIT_SOUTH},
                {BG_SSM_MINE_CART_3, Position(760.184509f, 198.844742f, 319.446655f, 0.351249f), BG_SSM_SPELL_CART_CONTROL_CAPTURE_POINT_UNIT_EAST}
            };

            if (Creature* cart = trigger->SummonCreature(NPC_MINE_CART, _carts[mineCart].Pos))
            {
                _cartsMap[mineCart] = cart->GetGUID();

                _mineCartsProgressBar[mineCart] = BG_SSM_PROGRESS_BAR_NEUTRAL;

                PlaySoundToAll(SoundKitCartSpawn);

                cart->SetUnitMovementFlags(MOVEMENTFLAG_BACKWARD);
                cart->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                cart->SetSpeed(MOVE_WALK, 1.2f); // not blizzlike, but seems as 2.0 too fast

                cart->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);
                cart->SetFlag(UNIT_FIELD_FLAGS_2, 1141374976);
                cart->SetFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_UNK0);

                //Trinity::ChatData chatData;
                //chatData.SenderGUID = cart->GetGUID();
                //chatData.SlashCmd = CHAT_MSG_RAID_BOSS_EMOTE;
                ////chatData.PartyGUID;
                //chatData.ChatFlags = 32;
                //chatData.SenderName = cart->GetName();
                //SendBroadcastText(BroadcastCartSpawn, chatData);
                /// WTF?

                cart->CastSpell(cart, BG_SSM_CONTROL_VISUAL_NEUTRAL, true);
                cart->CastSpell(cart, BG_SSM_SPELL_DEFENDING_CART_AURA, true);
                cart->CastSpell(cart, _carts[mineCart].ControlSpellID, true);
            }
        }

        _mineCartSpawnTimer = 15 * IN_MILLISECONDS;
    }
    else
        _mineCartSpawnTimer -= diff;
}

void BattlegroundSilvershardMines::CheckPlayerNearMineCart(uint32 diff)
{
    if (_mineCartCheckTimer <= 0)
    {
        for (auto itr : GetPlayers())
        {
            if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
            {
                if (player->isDead() || player->HasAuraType(AuraType::SPELL_AURA_MOD_STEALTH))
                {
                    player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_SHOW, BG_SSM_PROGRESS_BAR_DONT_SHOW);
                    continue;
                }

                bool cartControlled = false;

                if (player->GetBGTeam() == ALLIANCE)
                {
                    for (uint8 i = 0; i < 3; i++)
                    {
                        if (Creature* cart = GetBgMap()->GetCreature(_cartsMap[i]))
                        {
                            if (player->IsInDist2d(&*cart, 22.0f))
                            {
                                player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_SHOW, BG_SSM_PROGRESS_BAR_SHOW);

                                if (_mineCartsProgressBar[i] >= BG_SSM_PROGRESS_BAR_ALI_CONTROLLED)
                                {
                                    _mineCartsProgressBar[i] = BG_SSM_PROGRESS_BAR_ALI_CONTROLLED;
                                    player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_STATUS, _mineCartsProgressBar[i]);
                                }
                                else
                                {
                                    _mineCartsProgressBar[i]++;
                                    player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_STATUS, _mineCartsProgressBar[i]);
                                }

                                if (_mineCartsProgressBar[i] > BG_SSM_PROGRESS_BAR_NEUTRAL)
                                {
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_NEUTRAL))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_NEUTRAL, _cartsMap[i]);
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_HORDE))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_HORDE, _cartsMap[i]);
                                    if (!cart->HasAura(BG_SSM_CONTROL_VISUAL_ALLIANCE))
                                    {
                                        cart->CastSpell(cart, BG_SSM_CONTROL_VISUAL_ALLIANCE, true);
                                        SendBroadcastText(60441, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                                    }
                                }
                                if (_mineCartsProgressBar[i] == BG_SSM_PROGRESS_BAR_NEUTRAL)
                                {
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_ALLIANCE))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_ALLIANCE, _cartsMap[i]);
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_HORDE))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_HORDE, _cartsMap[i]);
                                    if (!cart->HasAura(BG_SSM_CONTROL_VISUAL_NEUTRAL))
                                        cart->CastSpell(cart, BG_SSM_CONTROL_VISUAL_NEUTRAL, true);
                                }
                                cartControlled = true;
                                break;
                            }
                        }
                    }
                }
                else // player->GetBGTeam() == HORDE
                {
                    for (uint8 i = 0; i < 3; i++)
                    {
                        if (Creature* cart = GetBgMap()->GetCreature(_cartsMap[i]))
                        {
                            if (player->IsInDist2d(&*cart, 22.0f))
                            {
                                player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_SHOW, BG_SSM_PROGRESS_BAR_SHOW);

                                if (_mineCartsProgressBar[i] <= 0)
                                {
                                    _mineCartsProgressBar[i] = 0;
                                    player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_STATUS, _mineCartsProgressBar[i]);
                                }
                                else
                                {
                                    _mineCartsProgressBar[i]--;
                                    player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_STATUS, _mineCartsProgressBar[i]);
                                }
                                if (_mineCartsProgressBar[i] < BG_SSM_PROGRESS_BAR_NEUTRAL)
                                {
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_NEUTRAL))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_NEUTRAL, cart->GetGUID());
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_ALLIANCE))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_ALLIANCE, cart->GetGUID());
                                    if (!cart->HasAura(BG_SSM_CONTROL_VISUAL_HORDE))
                                    {
                                        cart->CastSpell(cart, BG_SSM_CONTROL_VISUAL_HORDE, true);
                                        SendBroadcastText(60442, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                                    }
                                }
                                if (_mineCartsProgressBar[i] == BG_SSM_PROGRESS_BAR_NEUTRAL)
                                {
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_ALLIANCE))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_ALLIANCE, cart->GetGUID());
                                    if (cart->HasAura(BG_SSM_CONTROL_VISUAL_HORDE))
                                        cart->RemoveAurasDueToSpell(BG_SSM_CONTROL_VISUAL_HORDE, cart->GetGUID());
                                    if (!cart->HasAura(BG_SSM_CONTROL_VISUAL_NEUTRAL))
                                        cart->CastSpell(cart, BG_SSM_CONTROL_VISUAL_NEUTRAL, true);
                                }
                                cartControlled = true;
                                break;
                            }
                        }
                    }
                }
                if (!cartControlled)
                    player->SendUpdateWorldState(WorldStates::SSM_PROGRESS_BAR_SHOW, BG_SSM_PROGRESS_BAR_DONT_SHOW);
            }
        }
        _mineCartCheckTimer = 1000;
    }
    else
        _mineCartCheckTimer -= diff;
}

void BattlegroundSilvershardMines::CheckMineCartNearDepot(uint32 diff)
{
    if (!_mineCartNearDepot[BG_SSM_MINE_CART_1 - 1])
    {
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_1))
        {
            if (cart->GetExactDist2d(BgSSMDepotPos[SSM_LAVA_DEPOT][0], BgSSMDepotPos[SSM_LAVA_DEPOT][1]) <= 6.0f)
            {
                _depot[SSM_LAVA_DEPOT] = true;
                EventTeamCapturedMineCart(BG_SSM_MINE_CART_1);
                _mineCartNearDepot[BG_SSM_MINE_CART_1 - 1] = true;
            }

            if (cart->GetExactDist2d(BgSSMDepotPos[SSM_DIAMOND_DEPOT][0], BgSSMDepotPos[SSM_DIAMOND_DEPOT][1]) <= 6.0f)
            {
                _depot[SSM_DIAMOND_DEPOT] = true;
                EventTeamCapturedMineCart(BG_SSM_MINE_CART_1);
                _mineCartNearDepot[BG_SSM_MINE_CART_1 - 1] = true;
            }
        }
    }

    if (!_mineCartNearDepot[BG_SSM_MINE_CART_2 - 1])
    {
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_2))
        {
            if (cart->GetExactDist2d(BgSSMDepotPos[SSM_WATERFALL_DEPOT][0], BgSSMDepotPos[SSM_WATERFALL_DEPOT][1]) <= 6.0f)
            {
                _depot[SSM_WATERFALL_DEPOT] = true;
                EventTeamCapturedMineCart(BG_SSM_MINE_CART_2);
                _mineCartNearDepot[BG_SSM_MINE_CART_2 - 1] = true;
            }
        }
    }

    if (!_mineCartNearDepot[BG_SSM_MINE_CART_3 - 1])
    {
        if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_3))
        {
            if (cart->GetExactDist2d(BgSSMDepotPos[SSM_DIAMOND_DEPOT][0], BgSSMDepotPos[SSM_DIAMOND_DEPOT][1]) <= 6.0f)
            {
                _depot[SSM_DIAMOND_DEPOT] = true;
                EventTeamCapturedMineCart(BG_SSM_MINE_CART_3);
                _mineCartNearDepot[BG_SSM_MINE_CART_3 - 1] = true;
            }

            if (cart->GetExactDist2d(BgSSMDepotPos[SSM_TROLL_DEPOT][0], BgSSMDepotPos[SSM_TROLL_DEPOT][1]) <= 6.0f)
            {
                _depot[SSM_TROLL_DEPOT] = true;
                EventTeamCapturedMineCart(BG_SSM_MINE_CART_3);
                _mineCartNearDepot[BG_SSM_MINE_CART_3 - 1] = true;
            }
        }
    }
}

void BattlegroundSilvershardMines::EventTeamCapturedMineCart(uint8 mineCart)
{
    TeamId teamId = GetMineCartTeamKeeper(mineCart);
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        for (auto itr : GetPlayers())
        {
            if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
            {
                if (player->GetBGTeamId() == teamId)
                {
                    UpdatePlayerScore(player, SCORE_CARTS_HELPED, 1);
                    player->RewardHonor(player, 1, 20);
                }
            }
        }
    }

    switch (mineCart)
    {
        case BG_SSM_MINE_CART_1:
            if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_1))
            {
                if (_depot[SSM_LAVA_DEPOT])
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_LAVA_DEPOT]))
                    {
                        cart->StopMoving();
                        depot->UseDoorOrButton();
                        _mineCartReachedDepot[BG_SSM_MINE_CART_1 - 1] = true;
                    }
                }

                if (_depot[SSM_DIAMOND_DEPOT])
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_DIAMOND_DEPOT]))
                    {
                        cart->StopMoving();
                        depot->UseDoorOrButton();
                        _mineCartReachedDepot[BG_SSM_MINE_CART_1 - 1] = true;
                    }
                }
            }
            break;
        case BG_SSM_MINE_CART_2:
            if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_2))
            {
                if (_depot[SSM_WATERFALL_DEPOT])
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_WATERFALL_DEPOT]))
                    {
                        cart->StopMoving();
                        depot->UseDoorOrButton();
                        _mineCartReachedDepot[BG_SSM_MINE_CART_2 - 1] = true;
                    }
                }
            }
            break;
        case BG_SSM_MINE_CART_3:
            if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_3))
            {
                if (_depot[SSM_DIAMOND_DEPOT])
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_DIAMOND_DEPOT]))
                    {
                        cart->StopMoving();
                        depot->UseDoorOrButton();
                        _mineCartReachedDepot[BG_SSM_MINE_CART_3 - 1] = true;
                    }
                }

                if (_depot[SSM_TROLL_DEPOT])
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_TROLL_DEPOT]))
                    {
                        cart->StopMoving();
                        depot->UseDoorOrButton();
                        _mineCartReachedDepot[BG_SSM_MINE_CART_3 - 1] = true;
                    }
                }
            }
            break;
        default:
            break;
    }
}

void BattlegroundSilvershardMines::EventReopenDepot(uint32 diff)
{
    if (_mineCartReachedDepot[BG_SSM_MINE_CART_1 - 1])
    {
        if (_depot[SSM_LAVA_DEPOT])
        {
            if (_depotCloseTimer[SSM_LAVA_DEPOT] <= 0)
            {
                if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_1))
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_LAVA_DEPOT]))
                    {
                        SendBroadcastText(GetMineCartTeamKeeper(BG_SSM_MINE_CART_1) == TEAM_ALLIANCE ? 59689 : 59690, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                        PlaySoundToAll(GetMineCartTeamKeeper(BG_SSM_MINE_CART_1) == TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE : BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE);

                        if (_mineCartsProgressBar[BG_SSM_MINE_CART_1 - 1] != BG_SSM_PROGRESS_BAR_NEUTRAL)
                            AddPoints(GetMineCartTeamKeeper(BG_SSM_MINE_CART_1), PointsPerMineCart);

                        ResetDepotsAndMineCarts(SSM_LAVA_DEPOT, BG_SSM_MINE_CART_1);
                        depot->ResetDoorOrButton();
                        cart->DespawnOrUnsummon();
                    }
                }
            }
            else
                _depotCloseTimer[SSM_LAVA_DEPOT] -= diff;
        }

        if (_depot[SSM_DIAMOND_DEPOT])
        {
            if (_depotCloseTimer[SSM_DIAMOND_DEPOT] <= 0)
            {
                if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_1))
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_DIAMOND_DEPOT]))
                    {
                        SendBroadcastText(GetMineCartTeamKeeper(BG_SSM_MINE_CART_1) == TEAM_ALLIANCE ? 59689 : 59690, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                        PlaySoundToAll(GetMineCartTeamKeeper(BG_SSM_MINE_CART_1) == TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE : BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE);

                        if (_mineCartsProgressBar[BG_SSM_MINE_CART_1 - 1] != BG_SSM_PROGRESS_BAR_NEUTRAL)
                            AddPoints(GetMineCartTeamKeeper(BG_SSM_MINE_CART_1), PointsPerMineCart);

                        ResetDepotsAndMineCarts(SSM_DIAMOND_DEPOT, BG_SSM_MINE_CART_1);
                        depot->ResetDoorOrButton();
                        cart->DespawnOrUnsummon();
                    }
                }
            }
            else
                _depotCloseTimer[SSM_DIAMOND_DEPOT] -= diff;
        }
    }

    if (_mineCartReachedDepot[BG_SSM_MINE_CART_2 - 1])
    {
        if (_depot[SSM_WATERFALL_DEPOT])
        {
            if (_depotCloseTimer[SSM_WATERFALL_DEPOT] <= 0)
            {
                if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_2))
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_WATERFALL_DEPOT]))
                    {
                        SendBroadcastText(GetMineCartTeamKeeper(BG_SSM_MINE_CART_2) == TEAM_ALLIANCE ? 59689 : 59690, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                        PlaySoundToAll(GetMineCartTeamKeeper(BG_SSM_MINE_CART_2) == TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE : BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE);

                        if (_mineCartsProgressBar[BG_SSM_MINE_CART_2 - 1] != BG_SSM_PROGRESS_BAR_NEUTRAL)
                            AddPoints(GetMineCartTeamKeeper(BG_SSM_MINE_CART_2), PointsPerMineCart);

                        ResetDepotsAndMineCarts(SSM_WATERFALL_DEPOT, BG_SSM_MINE_CART_2);
                        depot->ResetDoorOrButton();
                        cart->DespawnOrUnsummon();
                    }
                }
            }
            else
                _depotCloseTimer[SSM_WATERFALL_DEPOT] -= diff;
        }
    }

    if (_mineCartReachedDepot[BG_SSM_MINE_CART_3 - 1])
    {
        if (_depot[SSM_DIAMOND_DEPOT])
        {
            if (_depotCloseTimer[SSM_DIAMOND_DEPOT] <= 0)
            {
                if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_3))
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_DIAMOND_DEPOT]))
                    {
                        SendBroadcastText(GetMineCartTeamKeeper(BG_SSM_MINE_CART_3) == TEAM_ALLIANCE ? 59689 : 59690, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                        PlaySoundToAll(GetMineCartTeamKeeper(BG_SSM_MINE_CART_3) == TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE : BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE);

                        if (_mineCartsProgressBar[BG_SSM_MINE_CART_3 - 1] != BG_SSM_PROGRESS_BAR_NEUTRAL)
                            AddPoints(GetMineCartTeamKeeper(BG_SSM_MINE_CART_3), PointsPerMineCart);

                        ResetDepotsAndMineCarts(SSM_DIAMOND_DEPOT, BG_SSM_MINE_CART_3);
                        depot->ResetDoorOrButton();
                        cart->DespawnOrUnsummon();
                    }
                }
            }
            else
                _depotCloseTimer[SSM_DIAMOND_DEPOT] -= diff;
        }

        if (_depot[SSM_TROLL_DEPOT])
        {
            if (_depotCloseTimer[SSM_TROLL_DEPOT] <= 0)
            {
                if (Creature* cart = GetMineCart(BG_SSM_MINE_CART_3))
                {
                    if (GameObject* depot = GetBgMap()->GetGameObject(BgObjects[BG_SSM_OBJECT_TROLL_DEPOT]))
                    {
                        SendBroadcastText(GetMineCartTeamKeeper(BG_SSM_MINE_CART_3) == TEAM_ALLIANCE ? 59689 : 59690, CHAT_MSG_BG_SYSTEM_NEUTRAL, cart);
                        PlaySoundToAll(GetMineCartTeamKeeper(BG_SSM_MINE_CART_3) == TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE : BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE);

                        if (_mineCartsProgressBar[BG_SSM_MINE_CART_3 - 1] != BG_SSM_PROGRESS_BAR_NEUTRAL)
                            AddPoints(GetMineCartTeamKeeper(BG_SSM_MINE_CART_3), PointsPerMineCart);

                        ResetDepotsAndMineCarts(SSM_TROLL_DEPOT, BG_SSM_MINE_CART_3);
                        depot->ResetDoorOrButton();
                        cart->DespawnOrUnsummon();
                    }
                }
            }
            else
                _depotCloseTimer[SSM_TROLL_DEPOT] -= diff;
        }
    }
}

TeamId BattlegroundSilvershardMines::GetMineCartTeamKeeper(uint8 mineCart)
{
    if (_mineCartsProgressBar[mineCart - 1] > BG_SSM_PROGRESS_BAR_NEUTRAL)
        return TEAM_ALLIANCE;

    if (_mineCartsProgressBar[mineCart - 1] < BG_SSM_PROGRESS_BAR_NEUTRAL)
        return TEAM_HORDE;

    return TEAM_NEUTRAL;
}

Creature* BattlegroundSilvershardMines::GetMineCart(uint8 cart)
{
    return GetBgMap()->GetCreature(_cartsMap[cart - 1]);
}

void BattlegroundSilvershardMines::MineCartAddPoints(uint32 diff)
{
    if (_mineCartAddPointsTimer <= 0)
    {
        TeamId mineCart1TeamId = GetMineCartTeamKeeper(BG_SSM_MINE_CART_1);
        TeamId mineCart2TeamId = GetMineCartTeamKeeper(BG_SSM_MINE_CART_2);
        TeamId mineCart3TeamId = GetMineCartTeamKeeper(BG_SSM_MINE_CART_3);

        if (mineCart1TeamId == TEAM_ALLIANCE && mineCart2TeamId == TEAM_ALLIANCE && mineCart3TeamId == TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 5);
        else if (mineCart1TeamId != TEAM_ALLIANCE && mineCart2TeamId == TEAM_ALLIANCE && mineCart3TeamId == TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 3);
        else if (mineCart1TeamId == TEAM_ALLIANCE && mineCart2TeamId != TEAM_ALLIANCE && mineCart3TeamId == TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 3);
        else if (mineCart1TeamId == TEAM_ALLIANCE && mineCart2TeamId == TEAM_ALLIANCE && mineCart3TeamId != TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 3);
        else if (mineCart1TeamId != TEAM_ALLIANCE && mineCart2TeamId != TEAM_ALLIANCE && mineCart3TeamId == TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 2);
        else if (mineCart1TeamId != TEAM_ALLIANCE && mineCart2TeamId == TEAM_ALLIANCE && mineCart3TeamId != TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 2);
        else if (mineCart1TeamId == TEAM_ALLIANCE && mineCart2TeamId != TEAM_ALLIANCE && mineCart3TeamId != TEAM_ALLIANCE)
            AddPoints(TEAM_ALLIANCE, 2);

        if (mineCart1TeamId == TEAM_HORDE && mineCart2TeamId == TEAM_HORDE && mineCart3TeamId == TEAM_HORDE)
            AddPoints(TEAM_HORDE, 5);
        else if (mineCart1TeamId != TEAM_HORDE && mineCart2TeamId == TEAM_HORDE && mineCart3TeamId == TEAM_HORDE)
            AddPoints(TEAM_HORDE, 3);
        else if (mineCart1TeamId == TEAM_HORDE && mineCart2TeamId != TEAM_HORDE && mineCart3TeamId == TEAM_HORDE)
            AddPoints(TEAM_HORDE, 3);
        else if (mineCart1TeamId == TEAM_HORDE && mineCart2TeamId == TEAM_HORDE && mineCart3TeamId != TEAM_HORDE)
            AddPoints(TEAM_HORDE, 3);
        else if (mineCart1TeamId != TEAM_HORDE && mineCart2TeamId != TEAM_HORDE && mineCart3TeamId == TEAM_HORDE)
            AddPoints(TEAM_HORDE, 2);
        else if (mineCart1TeamId != TEAM_HORDE && mineCart2TeamId == TEAM_HORDE && mineCart3TeamId != TEAM_HORDE)
            AddPoints(TEAM_HORDE, 2);
        else if (mineCart1TeamId == TEAM_HORDE && mineCart2TeamId != TEAM_HORDE && mineCart3TeamId != TEAM_HORDE)
            AddPoints(TEAM_HORDE, 2);

        _mineCartAddPointsTimer = 2000;
    }
    else
        _mineCartAddPointsTimer -= diff;
}

void BattlegroundSilvershardMines::ResetDepotsAndMineCarts(uint8 depot, uint8 mineCart)
{
    _depotCloseTimer[depot] = 3000;
    _depot[depot] = false;
    _mineCartsProgressBar[mineCart - 1] = BG_SSM_PROGRESS_BAR_NEUTRAL;
    _mineCartNearDepot[mineCart - 1] = false;
    _mineCartReachedDepot[mineCart - 1] = false;
    _cartsMap[mineCart - 1] = ObjectGuid::Empty;
}

void BattlegroundSilvershardMines::AddPoints(TeamId teamId, uint32 points)
{
    if (IsBrawl())
    {
        if (points > 50)
            points *= 2;
        else
            points /= 2;
    }

    m_TeamScores[teamId] += points;
    _honorScoreTics[teamId] += points;
    if (_honorScoreTics[teamId] >= _honorTics)
    {
        //RewardHonorToTeam(uint32(GetBonusHonorFromKill(1)), MS::Battlegrounds::GetTeamByTeamId(teamId)); //Deprecated ?
        _honorScoreTics[teamId] -= _honorTics;
    }

    if (m_TeamScores[teamId] >= SSM_MAX_TEAM_POINTS)
    {
        m_TeamScores[teamId] = SSM_MAX_TEAM_POINTS;
        EndBattleground(MS::Battlegrounds::GetTeamByTeamId(teamId));
        CastSpellOnTeam(135787, teamId); // Quest credit "The Lion Roars"
    }

    Battleground::SendBattleGroundPoints(teamId != TEAM_ALLIANCE, m_TeamScores[teamId]);
    UpdateWorldState(teamId == TEAM_ALLIANCE ? WorldStates::SSM_POINTS_ALLIANCE : WorldStates::SSM_POINTS_HORDE, m_TeamScores[teamId]);
}

void BattlegroundSilvershardMines::EndBattleground(uint32 winner)
{
    Battleground::EndBattleground(winner);

    for (auto v : _cartsMap)
        if (Creature* cart = GetBgMap()->GetCreature(v.second))
            cart->DespawnOrUnsummon();

    _cartsMap.clear();
}

void BattlegroundSilvershardMines::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattleGroundSSMScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(SSM_MAX_TEAM_POINTS).Write());

    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);
}

bool BattlegroundSilvershardMines::SetupBattleground()
{
    if (!AddObject(BG_SSM_OBJECT_DIAMOND_DEPOT, OBJECT_BG_SSM_THE_DESPOSITS_OF_DIAMONDS, BgSSMDepotPos[0][0], BgSSMDepotPos[0][1], BgSSMDepotPos[0][2], BgSSMDepotPos[0][3], BgSSMDepotPos[0][4], BgSSMDepotPos[0][5], BgSSMDepotPos[0][6], BgSSMDepotPos[0][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_WATERFALL_DEPOT, OBJECT_BG_SSM_RESERVOIR, BgSSMDepotPos[1][0], BgSSMDepotPos[1][1], BgSSMDepotPos[1][2], BgSSMDepotPos[1][3], BgSSMDepotPos[1][4], BgSSMDepotPos[1][5], BgSSMDepotPos[1][6], BgSSMDepotPos[1][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_LAVA_DEPOT, OBJECT_BG_SSM_THE_DESPOSITION_OF_LAVA, BgSSMDepotPos[2][0], BgSSMDepotPos[2][1], BgSSMDepotPos[2][2], BgSSMDepotPos[2][3], BgSSMDepotPos[2][4], BgSSMDepotPos[2][5], BgSSMDepotPos[2][6], BgSSMDepotPos[2][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_TROLL_DEPOT, OBJECT_BG_SSM_BACKLOG_TROLLS, BgSSMDepotPos[3][0], BgSSMDepotPos[3][1], BgSSMDepotPos[3][2], BgSSMDepotPos[3][3], BgSSMDepotPos[3][4], BgSSMDepotPos[3][5], BgSSMDepotPos[3][6], BgSSMDepotPos[3][7], RESPAWN_IMMEDIATELY) ||

        !AddObject(BG_SSM_OBJECT_DOOR_A_1, OBJECT_BG_SSM_DOOR1, BgSSMDoorPos[0][0], BgSSMDoorPos[0][1], BgSSMDoorPos[0][2], BgSSMDoorPos[0][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_DOOR_A_2, OBJECT_BG_SSM_DOOR2, BgSSMDoorPos[1][0], BgSSMDoorPos[1][1], BgSSMDoorPos[1][2], BgSSMDoorPos[1][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_DOOR_H_1, OBJECT_BG_SSM_DOOR3, BgSSMDoorPos[2][0], BgSSMDoorPos[2][1], BgSSMDoorPos[2][2], BgSSMDoorPos[2][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_DOOR_H_2, OBJECT_BG_SSM_DOOR4, BgSSMDoorPos[3][0], BgSSMDoorPos[3][1], BgSSMDoorPos[3][2], BgSSMDoorPos[3][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||

        !AddObject(BG_SSM_OBJECT_BERSERKING_BUFF_EAST, BG_OBJECTID_BERSERKERBUFF_ENTRY, BgSSMBuffPos[0][0], BgSSMBuffPos[0][1], BgSSMBuffPos[0][2], BgSSMBuffPos[0][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_BERSERKING_BUFF_WEST, BG_OBJECTID_BERSERKERBUFF_ENTRY, BgSSMBuffPos[1][0], BgSSMBuffPos[1][1], BgSSMBuffPos[1][2], BgSSMBuffPos[1][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_RESTORATION_BUFF_WATERFALL, BG_OBJECTID_REGENBUFF_ENTRY, BgSSMBuffPos[2][0], BgSSMBuffPos[2][1], BgSSMBuffPos[2][2], BgSSMBuffPos[2][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SSM_OBJECT_RESTORATION_BUFF_LAVA, BG_OBJECTID_REGENBUFF_ENTRY, BgSSMBuffPos[3][0], BgSSMBuffPos[3][1], BgSSMBuffPos[3][2], BgSSMBuffPos[3][3], 0, 0, 0.710569f, -0.703627f, RESPAWN_IMMEDIATELY))
        return false;

    if (Creature* track = AddCreature(NPC_TRACK_SWITCH, SSM_TRACK_SWITCH_EAST, TEAM_NEUTRAL, BgSSMTrackPos[SSM_EAST_PATH][0], BgSSMTrackPos[SSM_EAST_PATH][1], BgSSMTrackPos[SSM_EAST_PATH][2], BgSSMTrackPos[SSM_EAST_PATH][3]))
    {
        track->CastSpell(track, BG_SSM_FEIGN_DEATH_STUN, true);
        track->CastSpell(track, BG_SSM_TRACK_SWITCH_OPENED, true);
    }
    else
        return false;

    if (Creature* track = AddCreature(NPC_TRACK_SWITCH, SSM_TRACK_SWITCH_NORTH, TEAM_NEUTRAL, BgSSMTrackPos[SSM_NORTH_PATH][0], BgSSMTrackPos[SSM_NORTH_PATH][1], BgSSMTrackPos[SSM_NORTH_PATH][2], BgSSMTrackPos[SSM_NORTH_PATH][3]))
    {
        track->CastSpell(track, BG_SSM_FEIGN_DEATH_STUN, true);
        track->CastSpell(track, BG_SSM_TRACK_SWITCH_CLOSED, true);
    }
    else
        return false;

    if (Creature* trigger = AddCreature(NPC_TRACK_SWITCH, SSM_MINE_CART_TRIGGER, TEAM_NEUTRAL, BgSSMTrackPos[SSM_NORTH_PATH][0], BgSSMTrackPos[SSM_NORTH_PATH][1], BgSSMTrackPos[SSM_NORTH_PATH][2], BgSSMTrackPos[SSM_NORTH_PATH][3]))
        trigger->SetVisible(false);
    else
        return false;

    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(SSM_GRAVEYARD_MAIN_ALLIANCE);
    if (!sg || !AddSpiritGuide(SSM_SPIRIT_ALLIANCE, sg->Loc, TEAM_ALLIANCE))
        return false;

    sg = sWorldSafeLocsStore.LookupEntry(SSM_GRAVEYARD_MAIN_HORDE);
    if (!sg || !AddSpiritGuide(SSM_SPIRIT_HORDE, sg->Loc, TEAM_HORDE))
        return false;

    return true;
}

WorldSafeLocsEntry const * BattlegroundSilvershardMines::GetClosestGraveYard(Player* player)
{
    return sWorldSafeLocsStore.LookupEntry(player->GetBGTeamId() == TEAM_ALLIANCE ? SSM_GRAVEYARD_MAIN_ALLIANCE : SSM_GRAVEYARD_MAIN_HORDE);
}

void BattlegroundSilvershardMines::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::SSM_INIT_POINTS_HORDE, 1);
    packet.Worldstates.emplace_back(WorldStates::SSM_INIT_POINTS_ALLIANCE, 1);
    packet.Worldstates.emplace_back(WorldStates::SSM_POINTS_ALLIANCE, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::SSM_POINTS_HORDE, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStates::SSM_MINE_CARTS_DISPLAY, 1);
    packet.Worldstates.emplace_back(WorldStates::SSM_MINE_CART_1, 1);
    packet.Worldstates.emplace_back(WorldStates::SSM_MINE_CART_2, 1);
    packet.Worldstates.emplace_back(WorldStates::SSM_MINE_CART_3, 1);
    packet.Worldstates.emplace_back(WorldStates::SSM_PROGRESS_BAR_SHOW, BG_SSM_PROGRESS_BAR_DONT_SHOW);
    packet.Worldstates.emplace_back(WorldStates::SSM_PROGRESS_BAR_PERCENT_GREY, BG_SSM_PROGRESS_BAR_DONT_SHOW);
    packet.Worldstates.emplace_back(WorldStates::SSM_PROGRESS_BAR_STATUS, BG_SSM_PROGRESS_BAR_NEUTRAL);
    packet.Worldstates.emplace_back(WorldStates::WS_SSM_EAST_TRACK_SWITCH, _trackSwitch[SSM_EAST_TRACK_SWITCH] ? 1 : 2);
    packet.Worldstates.emplace_back(WorldStates::WS_SSM_NORTH_TRACK_SWITCH, _trackSwitch[SSM_NORTH_TRACK_SWITCH] ? 2 : 1);
}

void BattlegroundSilvershardMines::EventPlayerClickedOnFlag(Player* player, Unit* target)
{
    if (GetStatus() != STATUS_IN_PROGRESS || !player || !target)
        return;

    if (target->HasAura(BG_SSM_PREVENTION_AURA))
        return;

    target->CastSpell(target, BG_SSM_PREVENTION_AURA, true);

    if (!target->HasAura(BG_SSM_TRACK_SWITCH_OPENED) && !target->HasAura(BG_SSM_TRACK_SWITCH_CLOSED))
        target->CastSpell(target, BG_SSM_TRACK_SWITCH_OPENED, false);
    else if (target->HasAura(BG_SSM_TRACK_SWITCH_OPENED))
    {
        target->CastSpell(target, BG_SSM_TRACK_SWITCH_CLOSED, false);
        target->RemoveAura(BG_SSM_TRACK_SWITCH_OPENED);
    }
    else if (target->HasAura(BG_SSM_TRACK_SWITCH_CLOSED))
    {
        target->CastSpell(target, BG_SSM_TRACK_SWITCH_OPENED, false);
        target->RemoveAura(BG_SSM_TRACK_SWITCH_CLOSED);
    }
}
