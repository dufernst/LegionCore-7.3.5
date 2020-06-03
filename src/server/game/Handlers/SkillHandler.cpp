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

#include "TalentPackets.h"
#include "MiscPackets.h"

void WorldSession::HandleLearnTalent(WorldPackets::Talent::LearnTalent& packet)
{
    Player* player = GetPlayer();
    if (player->GetMap() && player->GetMap()->isChallenge())
        return;

    for (auto talentId : packet.Talents)
        if (player->LearnTalent(talentId))
            player->SendTalentsInfoData(false);
}

void WorldSession::HandleLearnPvpTalents(WorldPackets::Talent::LearnPvpTalents& packet)
{
    Player* player = GetPlayer();
    for (auto talentId : packet.TalentIDs)
        if (player->LearnPvpTalent(talentId))
            player->SendTalentsInfoData(false);
}

void WorldSession::HandleConfirmRespecWipe(WorldPackets::Misc::ConfirmRespecWipe& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    // Hack
    if (player->HasAura(33786))
        return;

    if (player->HasUnitState(UNIT_STATE_DIED))
        player->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    switch(packet.respecType)
    {
        case RESPEC_TYPE_TALENTS:
            if (!player->ResetTalents())
                player->SendTalentWipeConfirm(packet.RespecMaster, RESPEC_TYPE_TALENTS);
            break;
        case RESPEC_TYPE_SPEC:
        case RESPEC_TYPE_GLYPH:
            break;
        default:
            break;
    }

    player->SendTalentsInfoData(false);

    // spell: "Untalent Visual Effect"
    if (Unit* unit = player->GetSelectedUnit())
        unit->CastSpell(player, 14867, true);  
}

void WorldSession::HandleShowTradeSkill(WorldPackets::Misc::ShowTradeSkill& packet)
{
    if (!sSkillLineStore.LookupEntry(packet.SkillLineID) || !sSpellMgr->GetSpellInfo(packet.SpellID))
        return;

    Player* player = sObjectAccessor->FindPlayer(packet.PlayerGUID);
    if (!player)
        return;

    if (!player->CanContact())
        return;

    std::set<uint32> relatedSkills;
    relatedSkills.insert(packet.SkillLineID);

    for (SkillLineEntry const* skillLine : sSkillLineStore)
    {
        if (skillLine->ParentSkillLineID != packet.SkillLineID)
            continue;

        if (!player->HasSkill(skillLine->ParentSkillLineID))
            continue;

        relatedSkills.insert(skillLine->ParentSkillLineID);
    }

    std::set<uint32> profSpells;
    for (auto const& v : player->GetSpellMapConst())
    {
        if (v.second.state == PLAYERSPELL_REMOVED)
            continue;

        if (!v.second.active || v.second.disabled)
            continue;

        for (auto const& s : relatedSkills)
            if (IsPartOfSkillLine(s, v.first))
                profSpells.insert(v.first);
    }

    if (profSpells.empty())
        return;

    WorldPackets::Misc::ShowTradeSkillResponse response;
    response.PlayerGUID = packet.PlayerGUID;
    response.SpellId = packet.SpellID;

    for (uint32 const& x : profSpells)
        response.KnownAbilitySpellIDs.push_back(x);

    for (uint32 const& v : relatedSkills)
    {
        response.SkillLineIDs.push_back(v);
        response.SkillRanks.push_back(player->GetSkillValue(v));
        response.SkillMaxRanks.push_back(player->GetMaxSkillValue(v));
    }

    _player->SendDirectMessage(response.Write());
}
