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

#include "Object.h"
#include "Player.h"
#include "BattlegroundKotmoguTemplate.h"
#include "Creature.h"
#include "GameObject.h"
#include "BattlegroundMgr.h"
#include "WorldStatePackets.h"
#include "MapManager.h"
#include "SpellScript.h"
#include "ScriptMgr.h"

BattlegroundKotmoguTemplate::BattlegroundKotmoguTemplate() : _reputationCapture(0)
{
    _updatePointsTimer = 3 * IN_MILLISECONDS;
    _lastCapturedOrbTeam = TEAM_NONE;
}

BattlegroundKotmoguTemplate::~BattlegroundKotmoguTemplate()
{ }

void BattlegroundKotmoguTemplate::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (_updatePointsTimer <= diff)
        {
            for (uint8 i = 0; i < MAX_ORBS; ++i)
            {
                ObjectGuid guid = _orbKeepers[i];

                if (guid.IsEmpty() || _playersZone.find(guid) == _playersZone.end())
                    continue;

                _playersZone[guid] = CheckOrbKeepersPosition(guid);

                auto playerZone = _playersZone[guid];

                if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), guid))
                {
                    if (playerZone > KT_ZONE_MAX)
                        continue;

                    TeamId teamID = player->GetBGTeamId();
                    if (teamID >= TEAM_NEUTRAL)
                        continue;

                    m_TeamScores[teamID] += BG_KT_TickPoints[playerZone] * (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix ? 2 : 1);

                    if (m_TeamScores[teamID] > BG_KT_MAX_TEAM_SCORE)
                        m_TeamScores[teamID] = BG_KT_MAX_TEAM_SCORE;

                    UpdateWorldState(teamID == TEAM_ALLIANCE ? WorldStates::BG_KT_ORB_POINTS_A : WorldStates::BG_KT_ORB_POINTS_H, m_TeamScores[teamID]);

                    if (m_TeamScores[teamID] == BG_KT_MAX_TEAM_SCORE)
                        EndBattleground(MS::Battlegrounds::GetTeamByTeamId(teamID));

                    UpdatePlayerScore(player, SCORE_ORB_SCORE, BG_KT_TickPoints[playerZone] * (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix ? 2 : 1));
                    player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, BG_KT_OBJECTIVE_ORB_COUNT, playerZone + 3);
                    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()]);
                }
            }

            _updatePointsTimer = 2 * IN_MILLISECONDS;
        }
        else
            _updatePointsTimer -= diff;
    }
}

void BattlegroundKotmoguTemplate::StartingEventCloseDoors()
{
    SpawnBGObject(BG_KT_OBJECT_A_DOOR, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_KT_OBJECT_H_DOOR, RESPAWN_IMMEDIATELY);

    DoorsClose(BG_KT_OBJECT_A_DOOR, BG_KT_OBJECT_H_DOOR);

    if (GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
    {
        for (uint8 i = 0; i < MAX_ORBS; ++i)
            SpawnBGObject(BG_KT_OBJECT_ORB_GREEN + i, RESPAWN_ONE_DAY);
    }
    else
    {
        SpawnBGObject(BG_KT_OBJECT_ORB_GREEN, RESPAWN_ONE_DAY);
        SpawnBGObject(BG_KT_OBJECT_ORB_ORANGE, RESPAWN_ONE_DAY);
    }
}

void BattlegroundKotmoguTemplate::StartingEventOpenDoors()
{
    DoorsOpen(BG_KT_OBJECT_A_DOOR, BG_KT_OBJECT_H_DOOR);

    if (GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
    {
        for (uint8 i = 0; i < MAX_ORBS; ++i)
            SpawnBGObject(BG_KT_OBJECT_ORB_GREEN + i, RESPAWN_IMMEDIATELY);
    }
    else
    {
        SpawnBGObject(BG_KT_OBJECT_ORB_GREEN, RESPAWN_IMMEDIATELY);
        SpawnBGObject(BG_KT_OBJECT_ORB_ORANGE, RESPAWN_IMMEDIATELY);
    }

    // Players that join battleground after start are not eligible to get achievement.
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, BG_EVENT_START_BATTLE);
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, BG_EVENT_START_BATTLE);
}

void BattlegroundKotmoguTemplate::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattleGroundKTScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(BG_KT_MAX_TEAM_SCORE).Write());
    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);

    _playersZone[player->GetGUID()] = KT_ZONE_OUT;

    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate)
        player->CastSpell(player, BG_KT_SPELL_HOTMOGU, true);
}


void BattlegroundKotmoguTemplate::GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const
{
    for (uint8 i = 0; i < MAX_ORBS; ++i)
    {
        Player* player = ObjectAccessor::FindPlayer(GetBgMap(), GetFlagPickerGUID(i));
        if (!player)
            continue;

        WorldPackets::Battleground::BattlegroundPlayerPosition position;
        position.Guid = player->GetGUID();
        position.Pos = player->GetPosition();
        position.IconID = player->GetBGTeamId() == TEAM_ALLIANCE ? PLAYER_POSITION_ICON_ALLIANCE_FLAG : PLAYER_POSITION_ICON_HORDE_FLAG;
        position.ArenaSlot = i + 2;
        positions->push_back(position);
    }
}

void BattlegroundKotmoguTemplate::EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!source->IsWithinDistInMap(object, 10))
        return;

    uint32 index = object->GetEntry() - BG_KT_OBJECT_ORB_1_ENTRY;

    // If this orb is already keeped by a player, there is a problem
    if (index > MAX_ORBS || !_orbKeepers[index].IsEmpty())
        return;

    // Check if the player already have an orb
    for (uint8 i = 0; i < MAX_ORBS; ++i)
        if (_orbKeepers[i] == source->GetGUID())
            return;

    TeamId teamID = source->GetBGTeamId();

    source->CastSpell(source, BG_KT_ORBS_SPELLS[index], true);
    source->CastSpell(source, teamID == TEAM_ALLIANCE ? BG_KT_ALLIANCE_INSIGNIA : BG_KT_HORDE_INSIGNIA, true);

    UpdatePlayerScore(source, SCORE_ORB_HANDLES, 1);

    _orbKeepers[index] = source->GetGUID();
    UpdateWorldState(OrbsWS[index][teamID], 1);
    UpdateWorldState(OrbsIcons[index], 0);
    SpawnBGObject(BG_KT_OBJECT_ORB_GREEN + index, RESPAWN_ONE_DAY);

    if (Creature* aura = GetBGCreature(BG_KT_CREATURE_ORB_AURA_1 + index))
        aura->RemoveAllAuras();

    PlayeCapturePointSound(NODE_STATUS_ASSAULT, teamID);
    SendBroadcastText(BgKtBroadCastTextOrbPickedUp[index], teamID == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, source);
    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate)
        source->CastSpell(source, BG_KT_SPELL_BRAWL_OVERRIDE, true);
}

void BattlegroundKotmoguTemplate::EventPlayerDroppedFlag(Player* source)
{
    if (!source || GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 index = 0;

    for (; index <= MAX_ORBS; ++index)
    {
        if (index == MAX_ORBS)
            return;

        if (_orbKeepers[index] == source->GetGUID())
            break;
    }

    TeamId teamID = source->GetBGTeamId();

    source->RemoveAurasDueToSpell(BG_KT_ORBS_SPELLS[index]);
    source->RemoveAurasDueToSpell(BG_KT_ALLIANCE_INSIGNIA);
    source->RemoveAurasDueToSpell(BG_KT_HORDE_INSIGNIA);

    _orbKeepers[index].Clear();
    SpawnBGObject(BG_KT_OBJECT_ORB_GREEN + index, RESPAWN_IMMEDIATELY);

    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[BG_KT_OBJECT_ORB_GREEN + index]))
        obj->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);

    if (Creature* aura = GetBGCreature(BG_KT_CREATURE_ORB_AURA_1 + index))
        aura->AddAura(BG_KT_ORBS_AURA[index], aura);

    UpdateWorldState(OrbsWS[index][teamID], 0);
    UpdateWorldState(OrbsIcons[index], 1);

    PlayeCapturePointSound(NODE_STATUS_ASSAULT, teamID);
    SendBroadcastText(BgKtBroadCastTextOrbDropped[index], teamID == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, source);
    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate)
        source->RemoveAurasDueToSpell(BG_KT_SPELL_BRAWL_OVERRIDE);
}

void BattlegroundKotmoguTemplate::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32)
{
    if (!player)
        return;

    EventPlayerDroppedFlag(player);
    _playersZone.erase(player->GetGUID());

    player->RemoveAurasDueToSpell(BG_KT_SPELL_BRAWL_OVERRIDE);
    player->RemoveAurasDueToSpell(BG_KT_SPELL_HOTMOGU);
}

void BattlegroundKotmoguTemplate::_CheckPositions(uint32 diff)
{
    Battleground::_CheckPositions(diff);
}

void BattlegroundKotmoguTemplate::HandlePlayerResurrect(Player *player)
{
    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate)
        player->CastSpell(player, BG_KT_SPELL_HOTMOGU, true);
}

uint8 BattlegroundKotmoguTemplate::CheckOrbKeepersPosition(ObjectGuid guid)
{
    if (Player* plr = ObjectAccessor::FindPlayer(GetBgMap(), guid))
    {
        if (plr->GetPositionX() > 1750.27f && plr->GetPositionX() < 1816.04f && plr->GetPositionY() > 1287.34f && plr->GetPositionY() < 1379.34f && plr->GetPositionZ() < 10.0f)
            return KT_ZONE_MIDDLE;

        if (plr->GetPositionX() > 1716.78f && plr->GetPositionX() < 1850.29f && plr->GetPositionY() > 1249.93f && plr->GetPositionY() < 1416.77f)
            return KT_ZONE_IN;
    }

    return KT_ZONE_OUT;
}

bool BattlegroundKotmoguTemplate::SetupBattleground()
{
    if (!AddObject(BG_KT_OBJECT_A_DOOR, BG_KT_OBJECT_DOOR_ENTRY, BG_KT_DoorPositions[0][0], BG_KT_DoorPositions[0][1], BG_KT_DoorPositions[0][2], BG_KT_DoorPositions[0][3], 0, 0, sin(BG_KT_DoorPositions[0][3] / 2), cos(BG_KT_DoorPositions[0][3] / 2), RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_KT_OBJECT_H_DOOR, BG_KT_OBJECT_DOOR_ENTRY, BG_KT_DoorPositions[1][0], BG_KT_DoorPositions[1][1], BG_KT_DoorPositions[1][2], BG_KT_DoorPositions[1][3], 0, 0, sin(BG_KT_DoorPositions[1][3] / 2), cos(BG_KT_DoorPositions[1][3] / 2), RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_KT_OBJECT_BERSERKBUFF_1, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1856.635f, 1333.741f, 10.555f, 3.150048f, 0, 0, 0.5591929f, 0.8290376f, BUFF_RESPAWN_TIME) ||
        !AddObject(BG_KT_OBJECT_BERSERKBUFF_2, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1710.284f, 1333.345f, 10.554f, 0.116051f, 0, 0, 0.9396926f, -0.3420201f, BUFF_RESPAWN_TIME) ||
        !AddSpiritGuide(BG_KT_CREATURE_SPIRIT_1, BG_KT_SpiritPositions[0], TEAM_ALLIANCE) ||
        !AddSpiritGuide(BG_KT_CREATURE_SPIRIT_2, BG_KT_SpiritPositions[1], TEAM_HORDE))
        return false;

    if (GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
    {
        for (uint8 i = 0; i < MAX_ORBS; ++i)
        {
            if (!AddObject(BG_KT_OBJECT_ORB_GREEN + i, BG_KT_OBJECT_ORB_1_ENTRY + i, BG_KT_OrbPositions[i][0], BG_KT_OrbPositions[i][1], BG_KT_OrbPositions[i][2], BG_KT_OrbPositions[i][3], 0, 0, sin(BG_KT_OrbPositions[i][3] / 2), cos(BG_KT_OrbPositions[i][3] / 2), RESPAWN_ONE_DAY))
                return false;

            if (Creature* trigger = AddCreature(WORLD_TRIGGER, BG_KT_CREATURE_ORB_AURA_1 + i, TEAM_NEUTRAL, BG_KT_OrbPositions[i][0], BG_KT_OrbPositions[i][1], BG_KT_OrbPositions[i][2], BG_KT_OrbPositions[i][3], RESPAWN_IMMEDIATELY))
                trigger->AddAura(BG_KT_ORBS_AURA[i], trigger);
        }

        for (uint8 i = 0; i < MAX_ORBS; ++i)
            if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[BG_KT_OBJECT_ORB_GREEN + i]))
                obj->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
    }
    else
    {
        if (!AddObject(BG_KT_OBJECT_ORB_GREEN, BG_KT_OBJECT_ORB_1_ENTRY, 1716.747f, 1333.411f, 13.18124f, BG_KT_OrbPositions[0][3], 0, 0, sin(BG_KT_OrbPositions[0][3] / 2), cos(BG_KT_OrbPositions[0][3] / 2), RESPAWN_ONE_DAY))
            return false;

        if (Creature* trigger = AddCreature(WORLD_TRIGGER, BG_KT_CREATURE_ORB_AURA_1, TEAM_NEUTRAL, 1716.885f, 1333.413f, 13.2889f, 1.58825f, RESPAWN_IMMEDIATELY))
            trigger->AddAura(BG_KT_ORBS_AURA[0], trigger);

        if (!AddObject(BG_KT_OBJECT_ORB_GREEN + 2, BG_KT_OBJECT_ORB_1_ENTRY + 2, 1849.882f, 1333.389f, 13.32879f, BG_KT_OrbPositions[2][3], 0, 0, sin(BG_KT_OrbPositions[2][3] / 2), cos(BG_KT_OrbPositions[2][3] / 2), RESPAWN_ONE_DAY))
            return false;

        if (Creature* trigger = AddCreature(WORLD_TRIGGER, BG_KT_CREATURE_ORB_AURA_1 + 2, TEAM_NEUTRAL, 1850.306f, 1333.281f, 13.49556f, 1.58825f, RESPAWN_IMMEDIATELY))
            trigger->AddAura(BG_KT_ORBS_AURA[2], trigger);

        for (uint8 i : {0, 2})
            if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[BG_KT_OBJECT_ORB_GREEN + i]))
                obj->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
    }

    return true;
}

void BattlegroundKotmoguTemplate::Reset()
{
    Battleground::Reset();
    BgObjects.resize(BG_KT_OBJECT_MAX);
    BgCreatures.resize(BG_KT_CREATURE_MAX);

    for (uint32 i = 0; i < MAX_ORBS; ++i)
        _orbKeepers[i].Clear();

    _lastCapturedOrbTeam = TEAM_NONE;
}

void BattlegroundKotmoguTemplate::EndBattleground(uint32 winner)
{
    if (winner == ALLIANCE)
        CastSpellOnTeam(135788, ALLIANCE); // Quest credit "The Lion Roars"

    if (winner == HORDE)
        CastSpellOnTeam(135788, HORDE); // Quest credit "The Lion Roars"

    Battleground::EndBattleground(winner);
}

void BattlegroundKotmoguTemplate::HandleKillPlayer(Player *player, Player *killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);

    Battleground::HandleKillPlayer(player, killer);
}

WorldSafeLocsEntry const* BattlegroundKotmoguTemplate::GetClosestGraveYard(Player* player)
{
    if (player->GetBGTeamId() == TEAM_ALLIANCE)
        return sWorldSafeLocsStore.LookupEntry(GetStatus() == STATUS_IN_PROGRESS ? KT_GRAVEYARD_RECTANGLEA1 : KT_GRAVEYARD_RECTANGLEA2);
    return sWorldSafeLocsStore.LookupEntry(GetStatus() == STATUS_IN_PROGRESS ? KT_GRAVEYARD_RECTANGLEH1 : KT_GRAVEYARD_RECTANGLEH2);
}

void BattlegroundKotmoguTemplate::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::BG_KT_ICON_GREEN_ORB_ICON, _orbKeepers[BG_KT_OBJECT_ORB_GREEN - 2].IsEmpty() ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_KT_ICON_ORANGE_ORB_ICON, _orbKeepers[BG_KT_OBJECT_ORB_ORANGE - 2].IsEmpty() ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_KT_ORB_POINTS_A, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::BG_KT_ORB_POINTS_H, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStates::BG_KT_GREEN_ORB_C, _orbKeepers[BG_KT_OBJECT_ORB_GREEN - 2].IsEmpty());
    packet.Worldstates.emplace_back(WorldStates::BG_KT_ORANGE_ORB_C, _orbKeepers[BG_KT_OBJECT_ORB_ORANGE - 2].IsEmpty());

    if (GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
    {
        packet.Worldstates.emplace_back(WorldStates::BG_KT_ICON_PURPLE_ORB_ICON, _orbKeepers[BG_KT_OBJECT_ORB_PURPLE - 2].IsEmpty() ? 1 : 0);
        packet.Worldstates.emplace_back(WorldStates::BG_KT_ICON_BLUE_ORB_ICON, _orbKeepers[BG_KT_OBJECT_ORB_BLUE - 2].IsEmpty() ? 1 : 0);
        packet.Worldstates.emplace_back(WorldStates::BG_KT_PURPLE_ORB_C, _orbKeepers[BG_KT_OBJECT_ORB_PURPLE - 2].IsEmpty());
        packet.Worldstates.emplace_back(WorldStates::BG_KT_BLUE_ORB_C, _orbKeepers[BG_KT_OBJECT_ORB_BLUE - 2].IsEmpty());
    }
    else
        packet.Worldstates.emplace_back(BG_KT_BRAWL_TWO_ORBS, 1);
}

ObjectGuid BattlegroundKotmoguTemplate::GetFlagPickerGUID(int32 index) const
{
    if (index < MAX_ORBS)
        return _orbKeepers[index];

    return ObjectGuid::Empty;
}

void BattlegroundKotmoguTemplate::SetFlagPickerGUID(ObjectGuid guid, int32 index)
{
    _orbKeepers[index] = guid;
}


// 229977
class spell_brawl_pass_the_orb : public SpellScript
{
    PrepareSpellScript(spell_brawl_pass_the_orb);

    SpellCastResult CheckTarget()
    {
        Unit* target = GetExplTargetUnit();
        Unit* caster = GetCaster();
        if (!target || !caster)
            return SPELL_FAILED_CUSTOM_ERROR;

        if (!target->IsPlayer() || !target->isAlive() || target->ToPlayer()->GetBGTeamId() != caster->ToPlayer()->GetBGTeamId())
            return SPELL_FAILED_CUSTOM_ERROR;

        if (Battleground* tempBg = caster->ToPlayer()->GetBattleground())
        {
            if (tempBg->GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate)
                return SPELL_FAILED_CUSTOM_ERROR;

            BattlegroundKotmoguTemplate* bg = reinterpret_cast<BattlegroundKotmoguTemplate*>(tempBg);
            
            for (uint8 i = 0; i < MAX_ORBS; ++i)
                if (bg->GetFlagPickerGUID(i) == target->GetGUID())
                    return SPELL_FAILED_CUSTOM_ERROR;
        }
        else
            caster->RemoveAurasDueToSpell(BG_KT_SPELL_BRAWL_OVERRIDE);

        caster->SendSpellCooldown(229977, 0, 15000);

        return SPELL_CAST_OK;
    }

    void HandleOnHit()
    {
        Unit* target = GetHitUnit();
        Unit* caster = GetCaster();
        if (!target || !caster || !target->isAlive())
            return;

        Battleground* tempBg = caster->ToPlayer()->GetBattleground();
        if (!tempBg)
            return;

        if (tempBg->GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate)
            return;

        BattlegroundKotmoguTemplate* bg = reinterpret_cast<BattlegroundKotmoguTemplate*>(tempBg);

        for (uint8 i = 0; i < MAX_ORBS; ++i) // when sphere is flying, player can get new sphere
            if (bg->GetFlagPickerGUID(i) == target->GetGUID())
                return;

        uint8 index = 0;

        for (; index <= MAX_ORBS; ++index)
        {
            if (index == MAX_ORBS)
                return;

            if (bg->GetFlagPickerGUID(index) == caster->GetGUID())
                break;
        }

        if (!caster->HasAura(BG_KT_ORBS_SPELLS[index]))
            return;

        bg->SetFlagPickerGUID(target->GetGUID(), index);

        caster->RemoveAurasDueToSpell(BG_KT_ORBS_SPELLS[index]);
        caster->RemoveAurasDueToSpell(BG_KT_ALLIANCE_INSIGNIA);
        caster->RemoveAurasDueToSpell(BG_KT_HORDE_INSIGNIA);

        target->CastSpell(target, BG_KT_ORBS_SPELLS[index], true);
        target->CastSpell(target, target->ToPlayer()->GetBGTeamId() == TEAM_ALLIANCE ? BG_KT_ALLIANCE_INSIGNIA : BG_KT_HORDE_INSIGNIA, true);
        target->CastSpell(target, BG_KT_SPELL_BRAWL_OVERRIDE, true);

        caster->RemoveAurasDueToSpell(BG_KT_SPELL_BRAWL_OVERRIDE);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_brawl_pass_the_orb::CheckTarget);
        OnHit += SpellHitFn(spell_brawl_pass_the_orb::HandleOnHit);
    }
};



void AddSC_battleground_kotmogu()
{
    RegisterSpellScript(spell_brawl_pass_the_orb);
}