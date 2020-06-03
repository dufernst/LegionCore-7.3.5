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

#include "BattlefieldMgr.h"
#include "BattlefieldWG.h"
#include "BattlegroundIsleOfConquest.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "GameEventMgr.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "QuestData.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellAuraDefines.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"

bool IsPrimaryProfessionSkill(uint32 skill)
{
    SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
    if (!pSkill)
        return false;

    if (pSkill->CategoryID != SKILL_CATEGORY_PROFESSION)
        return false;

    return true;
}

bool IsWeaponSkill(uint32 skill)
{
    SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
    if (!pSkill)
        return false;

    if (pSkill->CategoryID != SKILL_CATEGORY_WEAPON)
        return false;

    return true;
}

bool IsPartOfSkillLine(uint32 skillId, uint32 spellId)
{
    auto skillBounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
    for (auto itr = skillBounds.first; itr != skillBounds.second; ++itr)
        if (itr->second->SkillLine == skillId)
            return true;

    return false;
}

DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellInfo const* spellproto, bool triggered)
{
    if (!spellproto || spellproto->IsPositive())
        return DIMINISHING_NONE;

    if (spellproto->HasAura(SPELL_AURA_MOD_TAUNT))
        return DIMINISHING_TAUNT;

    switch (spellproto->Id)
    {
        case 5782: // Fear
        case 87204:
        case 105771: // Charge
        case 143301:
        case 125500:
        case 112955:
        case 162652:
        case 196364:
        case 204399:
        case 157997:
        case 228600:
        case 213688:
        case 199042:
            return DIMINISHING_NONE;
        default:
            break;
    }

    switch (spellproto->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_GENERIC:
            break;
        case SPELLFAMILY_MAGE:
            break;
        case SPELLFAMILY_PRIEST:
            break;
        case SPELLFAMILY_WARRIOR:
            break;
        case SPELLFAMILY_WARLOCK:
            break;
        case SPELLFAMILY_PET_ABILITY:
            break;
        case SPELLFAMILY_DRUID:
            break;
        case SPELLFAMILY_ROGUE:
            break;
        case SPELLFAMILY_HUNTER:
            break;
        case SPELLFAMILY_PALADIN:
            break;
        case SPELLFAMILY_DEATHKNIGHT:
            break;
        case SPELLFAMILY_MONK:
            break;
        case SPELLFAMILY_EVENTS:
            return DIMINISHING_NONE;
        default:
            break;
    }

    // Lastly - Set diminishing depending on mechanic
    uint32 mechanic = spellproto->GetAllEffectsMechanicMask();

    if (mechanic & ((1 << MECHANIC_DISORIENTED) | (1 << MECHANIC_SHACKLE) | (1 << MECHANIC_CHARM) | (1 << MECHANIC_FEAR) | (1 << MECHANIC_SLEEP)))
        return DIMINISHING_DISORIENT;

    // Mechanic Knockout, except Blast Wave
    if (mechanic & ((1 << MECHANIC_INCAPACITATE) | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_BANISH) | (1 << MECHANIC_FREEZE) | (1 << MECHANIC_HORROR) | (1 << MECHANIC_SAPPED)))
        return DIMINISHING_INCAPACITATE;

    if (mechanic & (1 << MECHANIC_SNARE))
        return DIMINISHING_LIMITONLY;

    if (mechanic & (1 << MECHANIC_STUN))
        return DIMINISHING_STUN;

    if (mechanic & (1 << MECHANIC_ROOT))
        return DIMINISHING_ROOT;

    if (mechanic & (1 << MECHANIC_SILENCE))
        return DIMINISHING_SILENCE;

    return DIMINISHING_NONE;
}

DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_TAUNT:
        case DIMINISHING_STUN:
            return DRTYPE_ALL;
        case DIMINISHING_LIMITONLY:
        case DIMINISHING_NONE:
            return DRTYPE_NONE;
        default:
            return DRTYPE_PLAYER;
    }
}

DiminishingLevels GetDiminishingReturnsMaxLevel(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_TAUNT:
            return DIMINISHING_LEVEL_TAUNT_IMMUNE;
        default:
            return DIMINISHING_LEVEL_IMMUNE;
    }
}

int32 GetDiminishingReturnsLimitDuration(DiminishingGroup group, SpellInfo const* spellproto)
{
    // Explicit diminishing duration
    switch (spellproto->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_ROGUE:
        {
            if (spellproto->ClassOptions.SpellClassMask[0] & 0x80)
                return 8 * IN_MILLISECONDS;
            break;
        }
        case SPELLFAMILY_DEMON_HUNTER:
        {
            switch (spellproto->Id)
            {
                case 217832: // Imprison
                case 221527: // Imprison (PvP talent)
                    return 4 * IN_MILLISECONDS;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Wyvern Sting
            if (spellproto->ClassOptions.SpellClassMask[1] & 0x1000)
                return 6 * IN_MILLISECONDS;
            // Binding Shot
            if (spellproto->Id == 117526)
                return 3 * IN_MILLISECONDS;
            // Freezing Trap
            if (spellproto->Id == 3355)
                return 8 * IN_MILLISECONDS;
            // Tar Trap
            if (spellproto->Id == 135299)
                return spellproto->GetDuration();
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Repentance
            if (spellproto->ClassOptions.SpellClassMask[0] & 0x4)
                return 8 * IN_MILLISECONDS;
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (spellproto->ClassOptions.SpellClassMask[1] & 0x8000)
                return 8 * IN_MILLISECONDS;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Banish - limit to 6 seconds in PvP
            if (spellproto->ClassOptions.SpellClassMask[1] & 0x8000000)
                return 6 * IN_MILLISECONDS;
            switch (spellproto->Id)
            {
                case 170995: // Cripple
                    return 4 * IN_MILLISECONDS;
                case 118699: // Fear
                    return 6 * IN_MILLISECONDS;
                case 6358: // Seduction
                case 115268: // Mesmerize
                    return 8 * IN_MILLISECONDS;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            switch (spellproto->Id)
            {
                case 5246: // Intimidating Shout
                    return 6 * IN_MILLISECONDS;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_MONK:
        {
            switch (spellproto->Id)
            {
                case 115078: // Paralysis
                case 116706: // Disable (Root)
                    return 4 * IN_MILLISECONDS;
                case 198909: // Song of Chi-Ji
                    return 6 * IN_MILLISECONDS;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            if (spellproto->ClassOptions.SpellClassMask[0] & 0x1000000)
                return 8 * IN_MILLISECONDS;
            switch (spellproto->Id)
            {
                case 82691: // Ring of Frost
                    return 8 * IN_MILLISECONDS;
                case 31661: // Dragon's Breath
                    return 3 * IN_MILLISECONDS;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            switch (spellproto->Id)
            {
                case 8122: // Psychic Scream
                    return 6 * IN_MILLISECONDS;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    if (!IsDiminishingReturnsGroupDurationLimited(group))
        return 0;

    return 8 * IN_MILLISECONDS;
}

bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_DISORIENT:
        case DIMINISHING_ROOT:
        case DIMINISHING_STUN:
        case DIMINISHING_LIMITONLY:
            return true;
        default:
            return false;
    }
}

SpellMgr::SpellMgr() = default;

SpellMgr::~SpellMgr()
{
    UnloadSpellInfoStore();
}

bool SpellMgr::IsSpellValid(SpellInfo const* spellInfo, Player* player, bool msg)
{
    if (!spellInfo)
        return false;

    bool need_check_reagents = false;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->EffectMask < uint32(1 << i))
            break;

        switch (spellInfo->Effects[i]->Effect)
        {
            case SPELL_EFFECT_CREATE_ITEM:
            case SPELL_EFFECT_CREATE_ITEM_2:
            {
                if (spellInfo->Effects[i]->ItemType == 0)
                {
                    if (!spellInfo->IsLootCrafting()) // skip auto-loot crafting spells, its not need explicit item info (but have special fake items sometime)
                    {
                        if (msg)
                        {
                            if (player)
                                ChatHandler(player).PSendSysMessage("Craft spell %u not have create item entry.", spellInfo->Id);
                            else
                                TC_LOG_ERROR(LOG_FILTER_SQL, "Craft spell %u not have create item entry.", spellInfo->Id);
                        }
                        return false;
                    }

                }
                else if (!sObjectMgr->GetItemTemplate(spellInfo->Effects[i]->ItemType)) // also possible IsLootCrafting case but fake item must exist anyway
                {
                    if (msg)
                    {
                        if (player)
                            ChatHandler(player).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Effects[i]->ItemType);
                        else
                            TC_LOG_ERROR(LOG_FILTER_SQL, "Craft spell %u create not-exist in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Effects[i]->ItemType);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                if (!IsSpellValid(sSpellMgr->GetSpellInfo(spellInfo->Effects[i]->TriggerSpell), player, msg))
                {
                    if (msg)
                    {
                        if (player)
                            ChatHandler(player).PSendSysMessage("Spell %u learn to broken spell %u, and then...", spellInfo->Id, spellInfo->Effects[i]->TriggerSpell);
                        else
                            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u learn to invalid spell %u, and then...", spellInfo->Id, spellInfo->Effects[i]->TriggerSpell);
                    }
                    return false;
                }
                break;
            }
            default:
                break;
        }
    }

    if (need_check_reagents)
    {
        for (uint8 j = 0; j < MAX_SPELL_REAGENTS; ++j)
        {
            if (spellInfo->Reagents.Reagent[j] > 0 && !sObjectMgr->GetItemTemplate(spellInfo->Reagents.Reagent[j]))
            {
                if (msg)
                {
                    if (player)
                        ChatHandler(player).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Reagents.Reagent[j]);
                    else
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...", spellInfo->Id, spellInfo->Reagents.Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

bool SpellMgr::IsSpellForbidden(uint32 spellid)
{
    for (auto& spell : mForbiddenSpells)
        if (spell == spellid)
            return true;

    return false;
}

SpellChainNode const* SpellMgr::GetSpellChainNode(uint32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellChains, spell_id);
}

uint32 SpellMgr::GetFirstSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->first->Id;

    return spell_id;
}

uint32 SpellMgr::GetLastSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->last->Id;

    return spell_id;
}

uint32 SpellMgr::GetNextSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if (node->next)
            return node->next->Id;

    return 0;
}

uint32 SpellMgr::GetPrevSpellInChain(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        if (node->prev)
            return node->prev->Id;

    return 0;
}

uint8 SpellMgr::GetSpellRank(uint32 spell_id) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
        return node->rank;

    return 0;
}

uint32 SpellMgr::GetSpellWithRank(uint32 spell_id, uint32 rank, bool strict) const
{
    if (SpellChainNode const* node = GetSpellChainNode(spell_id))
    {
        if (rank != node->rank)
            return GetSpellWithRank(node->rank < rank ? node->next->Id : node->prev->Id, rank, strict);
    }
    else if (strict && rank > 1)
        return 0;
    return spell_id;
}

Trinity::IteratorPair<SpellRequiredMap::const_iterator> SpellMgr::GetSpellsRequiredForSpellBounds(uint32 spell_id) const
{
    return Trinity::Containers::MapEqualRange(mSpellReq, spell_id);
}

Trinity::IteratorPair<SpellsRequiringSpellMap::const_iterator> SpellMgr::GetSpellsRequiringSpellBounds(uint32 spell_id) const
{
    return Trinity::Containers::MapEqualRange(mSpellsReqSpell, spell_id);
}

bool SpellMgr::IsSpellRequiringSpell(uint32 spellid, uint32 req_spellid) const
{
    for (auto const& pair : GetSpellsRequiringSpellBounds(req_spellid))
        if (pair.second == spellid)
            return true;

    return false;
}

SpellsRequiringSpellMap SpellMgr::GetSpellsRequiringSpell()
{
    return this->mSpellsReqSpell;
}

uint32 SpellMgr::GetSpellRequired(uint32 spell_id) const
{
    auto itr = mSpellReq.find(spell_id);
    if (itr == mSpellReq.end())
        return 0;

    return itr->second;
}

SpellLearnSkillNode const* SpellMgr::GetSpellLearnSkill(uint32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellLearnSkills, spell_id);
}

SpellLearnSpellMapBounds SpellMgr::GetSpellLearnSpellMapBounds(uint32 spellID) const
{
    return mSpellLearnSpells.equal_range(spellID);
}

bool SpellMgr::IsSpellLearnSpell(uint32 spell_id) const
{
    return mSpellLearnSpells.find(spell_id) != mSpellLearnSpells.end();
}

bool SpellMgr::IsSpellLearnToSpell(uint32 spell_id1, uint32 spell_id2) const
{
    SpellLearnSpellMapBounds bounds = GetSpellLearnSpellMapBounds(spell_id1);
    for (auto i = bounds.first; i != bounds.second; ++i)
        if (i->second.spell == spell_id2)
            return true;
    return false;
}

SpellTargetPosition const* SpellMgr::GetSpellTargetPosition(uint32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellTargetPositions, spell_id);
}

SpellSpellGroupMapBounds SpellMgr::GetSpellSpellGroupMapBounds(uint32 spellID) const
{
    spellID = GetFirstSpellInChain(spellID);
    return mSpellSpellGroup.equal_range(spellID);
}

uint32 SpellMgr::IsSpellMemberOfSpellGroup(uint32 spellID, SpellGroup groupid) const
{
    auto spellGroup = GetSpellSpellGroupMapBounds(spellID);
    for (auto itr = spellGroup.first; itr != spellGroup.second; ++itr)
        if (itr->second == groupid)
            return true;
    return false;
}

SpellGroupSpellMapBounds SpellMgr::GetSpellGroupSpellMapBounds(SpellGroup group_id) const
{
    return mSpellGroupSpell.equal_range(group_id);
}

void SpellMgr::GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells) const
{
    std::set<SpellGroup> usedGroups;
    GetSetOfSpellsInSpellGroup(group_id, foundSpells, usedGroups);
}

void SpellMgr::GetSetOfSpellsInSpellGroup(SpellGroup group_id, std::set<uint32>& foundSpells, std::set<SpellGroup>& usedGroups) const
{
    if (usedGroups.find(group_id) != usedGroups.end())
        return;
    usedGroups.insert(group_id);

    SpellGroupSpellMapBounds groupSpell = GetSpellGroupSpellMapBounds(group_id);
    for (SpellGroupSpellMap::const_iterator itr = groupSpell.first; itr != groupSpell.second; ++itr)
    {
        if (itr->second < 0)
            GetSetOfSpellsInSpellGroup(static_cast<SpellGroup>(abs(itr->second)), foundSpells, usedGroups);
        else
            foundSpells.insert(itr->second);
    }
}

SpellGroupStackRule SpellMgr::GetSpellGroupStackRule(SpellGroup group) const
{
    SpellGroupStackMap::const_iterator itr = mSpellGroupStack.find(group);
    if (itr != mSpellGroupStack.end())
        return itr->second;

    return SPELL_GROUP_STACK_RULE_DEFAULT;
}

bool SpellMgr::AddSameEffectStackRuleSpellGroups(SpellInfo const* spellInfo, AuraEffect* eff, std::multimap<SpellGroup, AuraEffect*>& groups) const
{
    if (!spellInfo || !eff)
        return false;

    SpellSpellGroupMapBounds spellGroup = GetSpellSpellGroupMapBounds(spellInfo->GetFirstRankSpell()->Id);
    for (SpellSpellGroupMap::const_iterator itr = spellGroup.first; itr != spellGroup.second; ++itr)
    {
        SpellGroup group = itr->second;
        if (GetSpellGroupStackRule(group) == SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT)
        {
            if (groups.find(group) == groups.end())
                groups.insert(std::make_pair(group, eff));
            else
            {
                for (std::multimap<SpellGroup, AuraEffect*>::iterator iter = groups.lower_bound(group); iter != groups.upper_bound(group);)
                {
                    if (abs(iter->second->GetAmount()) <= abs(eff->GetAmount()))
                    {
                        groups.erase(iter);
                        groups.insert(std::make_pair(group, eff));
                    }
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

bool SpellMgr::AddSameEffectStackRuleSpellGroups(SpellInfo const* spellInfo, int32 amount, std::map<SpellGroup, int32>& groups) const
{
    if (!spellInfo)
        return false;

    SpellSpellGroupMapBounds spellGroup = GetSpellSpellGroupMapBounds(spellInfo->GetFirstRankSpell()->Id);
    for (SpellSpellGroupMap::const_iterator itr = spellGroup.first; itr != spellGroup.second; ++itr)
    {
        SpellGroup group = itr->second;
        if (GetSpellGroupStackRule(group) == SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT)
        {
            // Put the highest amount in the map
            if (groups.find(group) == groups.end())
                groups[group] = amount;
            else
            {
                // Take absolute value because this also counts for the highest negative aura
                if (abs(groups[group]) < abs(amount))
                    groups[group] = amount;
            }
            // return because a spell should be in only one SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT group
            return true;
        }
    }

    return false;
}

SpellGroupStackRule SpellMgr::CheckSpellGroupStackRules(SpellInfo const* spellInfo1, SpellInfo const* spellInfo2) const
{
    if (!spellInfo1 || !spellInfo2)
        return SPELL_GROUP_STACK_RULE_DEFAULT;

    uint32 spellid_1 = spellInfo1->GetFirstRankSpell()->Id;
    uint32 spellid_2 = spellInfo2->GetFirstRankSpell()->Id;
    if (spellid_1 == spellid_2)
        return SPELL_GROUP_STACK_RULE_DEFAULT;

    SpellSpellGroupMapBounds spellGroup1 = GetSpellSpellGroupMapBounds(spellid_1);
    std::set<SpellGroup> groups;
    for (SpellSpellGroupMap::const_iterator itr = spellGroup1.first; itr != spellGroup1.second; ++itr)
    {
        if (IsSpellMemberOfSpellGroup(spellid_2, itr->second))
        {
            bool add = true;
            SpellGroupSpellMapBounds groupSpell = GetSpellGroupSpellMapBounds(itr->second);
            for (SpellGroupSpellMap::const_iterator itr2 = groupSpell.first; itr2 != groupSpell.second; ++itr2)
            {
                if (itr2->second < 0)
                {
                    SpellGroup currGroup = static_cast<SpellGroup>(abs(itr2->second));
                    if (IsSpellMemberOfSpellGroup(spellid_1, currGroup) && IsSpellMemberOfSpellGroup(spellid_2, currGroup))
                    {
                        add = false;
                        break;
                    }
                }
            }

            if (add)
                groups.insert(itr->second);
        }
    }

    SpellGroupStackRule rule = SPELL_GROUP_STACK_RULE_DEFAULT;
    for (std::set<SpellGroup>::iterator itr = groups.begin(); itr != groups.end(); ++itr)
    {
        SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(*itr);
        if (found != mSpellGroupStack.end())
            rule = found->second;

        if (rule)
            break;
    }

    return rule;
}

const std::vector<SpellProcEventEntry>* SpellMgr::GetSpellProcEvent(uint32 spellId) const
{
    return !mSpellProcEventMap[spellId].empty() ? &mSpellProcEventMap[spellId] : nullptr;
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const* spellProcEvent, uint32 EventProcFlag, SpellInfo const* procSpell, uint32 procFlags, uint32 procExtra, bool active)
{
    // No extra req need
    uint32 procEvent_procEx = PROC_EX_NONE;

    // check prockFlags for condition
    if ((procFlags & EventProcFlag) == 0)
        return false;

    /// Quick Check - If PROC_FLAG_TAKEN_DAMAGE is set for aura and procSpell dealt damage, proc no matter what kind of spell that deals the damage.
    if (procFlags & PROC_FLAG_TAKEN_DAMAGE && EventProcFlag & PROC_FLAG_TAKEN_DAMAGE)
        return true;

    if (procFlags & PROC_FLAG_DONE_PERIODIC && EventProcFlag & PROC_FLAG_DONE_PERIODIC)
    {
        if (procExtra & PROC_EX_INTERNAL_HOT)
        {
            if (!(EventProcFlag & (PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS)))
                return false;
        }
        /// Aura must have negative or neutral(PROC_FLAG_DONE_PERIODIC only) procflags for a DOT to proc
        else if (EventProcFlag != PROC_FLAG_DONE_PERIODIC)
            if (!(EventProcFlag & (PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG | PROC_FLAG_DONE_TRAP_ACTIVATION)))
                return false;
    }

    if (procFlags & PROC_FLAG_TAKEN_PERIODIC && EventProcFlag & PROC_FLAG_TAKEN_PERIODIC)
    {
        if (procExtra & PROC_EX_INTERNAL_HOT)
        {
            /// No aura that only has PROC_FLAG_TAKEN_PERIODIC can proc from a HOT.
            if (EventProcFlag == PROC_FLAG_TAKEN_PERIODIC)
                return false;
            /// Aura must have positive procflags for a HOT to proc
            if (!(EventProcFlag & (PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS)))
                return false;
        }
        /// Aura must have negative or neutral(PROC_FLAG_TAKEN_PERIODIC only) procflags for a DOT to proc
        else if (EventProcFlag != PROC_FLAG_TAKEN_PERIODIC)
            if (!(EventProcFlag & (PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG)))
                return false;
    }

    // Trap casts are active by default
    if (procFlags & PROC_FLAG_DONE_TRAP_ACTIVATION)
        active = true;

    // Always trigger for this
    if (procFlags & (PROC_FLAG_KILLED | PROC_FLAG_KILL | PROC_FLAG_DEATH))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // Store extra req
        procEvent_procEx = spellProcEvent->procEx;

        // For melee triggers
        if (procSpell == nullptr)
        {
            // Check (if set) for school (melee attack have Normal school)
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Check (if set) for school
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & procSpell->GetMisc()->MiscData.SchoolMask) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if (spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->ClassOptions.SpellClassSet))
                return false;

            // spellFamilyName is Ok need check for spellFamilyMask if present
            if (spellProcEvent->spellFamilyMask)
            {
                if (!(spellProcEvent->spellFamilyMask & procSpell->ClassOptions.SpellClassMask))
                    return false;

                // Some spells are not considered as active even with have spellfamilyflags
                if (!(procEvent_procEx & PROC_EX_ONLY_ACTIVE_SPELL))
                    active = true;
            }
        }
    }

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // No extra req, so can trigger only for hit/crit - spell has to be active
        if ((procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT))/* && active*/)
            return true;
    }
    else // Passive spells hits here only if resist/reflect/immune/evade
    {
        if (procExtra & AURA_SPELL_PROC_EX_MASK)
        {
            // if spell marked as procing only from not active spells
            if (active && procEvent_procEx & PROC_EX_NOT_ACTIVE_SPELL)
                return false;
            // if spell marked as procing only from active spells
            if (!active && procEvent_procEx & PROC_EX_ONLY_ACTIVE_SPELL)
                return false;
            // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
            if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
                return true;
            // PROC_EX_NOT_ACTIVE_SPELL and PROC_EX_ONLY_ACTIVE_SPELL flags handle: if passed checks before
            if ((procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT)) && ((procEvent_procEx & (AURA_SPELL_PROC_EX_MASK)) == 0))
                return true;
        }
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

SpellProcEntry const* SpellMgr::GetSpellProcEntry(uint32 spellId) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellProcMap, spellId);
}

bool SpellMgr::CanSpellTriggerProcOnEvent(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo)
{
    // proc type doesn't match
    if (!(eventInfo.GetTypeMask() & procEntry.typeMask))
        return false;

    // check XP or honor target requirement
    if (procEntry.attributesMask & PROC_ATTR_REQ_EXP_OR_HONOR)
        if (Player* actor = eventInfo.GetActor()->ToPlayer())
            if (eventInfo.GetActionTarget() && !actor->isHonorOrXPTarget(eventInfo.GetActionTarget()))
                return false;

    // always trigger for these types
    if (eventInfo.GetTypeMask() & (PROC_FLAG_KILLED | PROC_FLAG_KILL | PROC_FLAG_DEATH))
        return true;

    // check school mask (if set) for other trigger types
    if (procEntry.schoolMask && !(eventInfo.GetSchoolMask() & procEntry.schoolMask))
        return false;

    // check spell family name/flags (if set) for spells
    if (eventInfo.GetTypeMask() & (PERIODIC_PROC_FLAG_MASK | SPELL_PROC_FLAG_MASK | PROC_FLAG_DONE_TRAP_ACTIVATION))
    {
        if (procEntry.spellFamilyName && (procEntry.spellFamilyName != eventInfo.GetSpellInfo()->ClassOptions.SpellClassSet))
            return false;

        if (procEntry.spellFamilyMask && !(procEntry.spellFamilyMask & eventInfo.GetSpellInfo()->ClassOptions.SpellClassMask))
            return false;
    }

    // check spell type mask (if set)
    if (eventInfo.GetTypeMask() & (SPELL_PROC_FLAG_MASK | PERIODIC_PROC_FLAG_MASK))
    {
        if (procEntry.spellTypeMask && !(eventInfo.GetSpellTypeMask() & procEntry.spellTypeMask))
            return false;
    }

    // check spell phase mask
    if (eventInfo.GetTypeMask() & REQ_SPELL_PHASE_PROC_FLAG_MASK)
    {
        if (!(eventInfo.GetSpellPhaseMask() & procEntry.spellPhaseMask))
            return false;
    }

    // check hit mask (on taken hit or on done hit, but not on spell cast phase)
    if ((eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK) || ((eventInfo.GetTypeMask() & DONE_HIT_PROC_FLAG_MASK) && !(eventInfo.GetSpellPhaseMask() & PROC_SPELL_PHASE_CAST)))
    {
        uint32 hitMask = procEntry.hitMask;
        // get default values if hit mask not set
        if (!hitMask)
        {
            // for taken procs allow normal + critical hits by default
            if (eventInfo.GetTypeMask() & TAKEN_HIT_PROC_FLAG_MASK)
                hitMask |= PROC_HIT_NORMAL | PROC_HIT_CRITICAL;
            // for done procs allow normal + critical + absorbs by default
            else
                hitMask |= PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_ABSORB;
        }
        if (!(eventInfo.GetHitMask() & hitMask))
            return false;
    }

    return true;
}

SpellBonusEntry const* SpellMgr::GetSpellBonusData(uint32 spellId) const
{
    SpellBonusEntry const* bonus = mSpellBonusVector[spellId];
    if (bonus)
        return bonus;

    if (uint32 rank_1 = GetFirstSpellInChain(spellId))
    {
        bonus = mSpellBonusVector[rank_1];
        if (bonus)
            return bonus;
    }

    return nullptr;
}

SpellThreatEntry const* SpellMgr::GetSpellThreatEntry(uint32 spellID) const
{
    if (auto itr = Trinity::Containers::MapGetValuePtr(mSpellThreatMap, spellID))
        return itr;

    if (auto itr2 = Trinity::Containers::MapGetValuePtr(mSpellThreatMap, GetFirstSpellInChain(spellID)))
        return itr2;

    return nullptr;
}

SkillLineAbilityMapBounds SpellMgr::GetSkillLineAbilityMapBounds(uint32 spellID) const
{
    return mSkillLineAbilityMap.equal_range(spellID);
}

const std::vector<PetAura>* SpellMgr::GetPetAura(int32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellPetAuraMap, entry);
}

SpellEnchantProcEntry const* SpellMgr::GetSpellEnchantProcEvent(uint32 enchId) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellEnchantProcEventMap, enchId);
}

bool SpellMgr::IsArenaAllowedEnchancment(uint32 ench_id) const
{
    return mEnchantCustomAttr[ench_id];
}

const std::vector<SpellLinked>* SpellMgr::GetSpellLinked(int8 type, int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellLinkedMap[type], spell_id);
}

const std::vector<SpellTalentLinked>* SpellMgr::GetSpelltalentLinked(int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellTalentLinkedMap, spell_id);
}

const std::vector<SpellConcatenateAura>* SpellMgr::GetSpellConcatenateApply(int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellConcatenateApplyMap, spell_id);
}

const std::vector<SpellConcatenateAura>* SpellMgr::GetSpellConcatenateUpdate(int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellConcatenateUpdateMap, spell_id);
}

const std::vector<SpellVisual>* SpellMgr::GetPlaySpellVisualData(int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellVisualMap, spell_id);
}

const std::vector<SpellVisualPlayOrphan>* SpellMgr::GetSpellVisualPlayOrphan(int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellVisualPlayOrphanMap, spell_id);
}

const std::vector<SpellVisualKit>* SpellMgr::GetSpellVisualKit(int32 spell_id) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellVisualKitMap, spell_id);
}

const SpellScene* SpellMgr::GetSpellScene(int32 miscValue) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellSceneMap, miscValue);
}

const std::vector<SceneTriggerEvent>* SpellMgr::GetSceneTriggerEvent(int32 miscValue) const
{
    return Trinity::Containers::MapGetValuePtr(mSceneTriggerEventMap, miscValue);
}

const std::vector<SpellPendingCast>* SpellMgr::GetSpellPendingCast(int32 spellID) const
{
    return Trinity::Containers::MapGetValuePtr(mSpellPendingCastMap, spellID);
}

const std::vector<SpellPrcoCheck>* SpellMgr::GetSpellPrcoCheck(int32 spell_id) const
{
    return !mSpellPrcoCheckMap[spell_id].empty() ? &mSpellPrcoCheckMap[spell_id] : nullptr;
}

const std::vector<SpellTriggered>* SpellMgr::GetSpellTriggered(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellTriggeredVector.empty())
        return nullptr;
    return mSpellTriggeredVector[spell_id];
}

const std::vector<SpellDummyTrigger>* SpellMgr::GetSpellDummyTrigger(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellDummyTriggerVector.empty())
        return nullptr;
    return mSpellDummyTriggerVector[spell_id];
}

const std::vector<AuraPeriodickTrigger>* SpellMgr::GetSpellAuraTrigger(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellAuraTriggerVector.empty())
        return nullptr;
    return mSpellAuraTriggerVector[spell_id];
}

const std::vector<SpellAuraDummy>* SpellMgr::GetSpellAuraDummy(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellAuraDummyVector.empty())
        return nullptr;
    return mSpellAuraDummyVector[spell_id];
}

const std::vector<SpellTargetFilter>* SpellMgr::GetSpellTargetFilter(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellTargetFilterVector.empty())
        return nullptr;
    return mSpellTargetFilterVector[spell_id];
}

const std::vector<SpellCheckCast>* SpellMgr::GetSpellCheckCast(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellCheckCastVector.empty())
        return nullptr;
    return mSpellCheckCastVector[spell_id];
}

const std::vector<SpellTriggerDelay>* SpellMgr::GetSpellTriggerDelay(int32 spell_id) const
{
    if (spell_id >= int32(sSpellStore.GetNumRows()) || spell_id < 0)
        return nullptr;
    if (mSpellTriggerDelayVector.empty())
        return nullptr;
    return mSpellTriggerDelayVector[spell_id];
}

PetLevelupSpellSet const* SpellMgr::GetPetLevelupSpellList(uint32 petFamily) const
{
    return Trinity::Containers::MapGetValuePtr(mPetLevelupSpellMap, petFamily);
}

PetDefaultSpellsEntry const* SpellMgr::GetPetDefaultSpellsEntry(int32 id) const
{
    return Trinity::Containers::MapGetValuePtr(mPetDefaultSpellsMap, id);
}

SpellAreaMapBounds SpellMgr::GetSpellAreaMapBounds(uint32 spell_id) const
{
    return mSpellAreaMap.equal_range(spell_id);
}

SpellAreaForQuestMapBounds SpellMgr::GetSpellAreaForQuestMapBounds(uint32 quest_id) const
{
    return mSpellAreaForQuestMap.equal_range(quest_id);
}

SpellAreaForQuestMapBounds SpellMgr::GetSpellAreaForQuestEndMapBounds(uint32 quest_id) const
{
    return mSpellAreaForQuestEndMap.equal_range(quest_id);
}

SpellAreaForAuraMapBounds SpellMgr::GetSpellAreaForAuraMapBounds(uint32 spell_id) const
{
    return mSpellAreaForAuraMap.equal_range(spell_id);
}

SpellAreaForAreaMapBounds SpellMgr::GetSpellAreaForAreaMapBounds(uint32 area_id) const
{
    return mSpellAreaForAreaMap.equal_range(area_id);
}

SpellInfo const* SpellMgr::GetSpellInfo(uint32 spellId) const
{
    if (spellId < GetSpellInfoStoreSize() && spellId > 0)
        return mSpellInfoMap[spellId];

    if (spellId)
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "%s: Try to get spellInfo for non-existing spell %u", __FUNCTION__, spellId);
    return nullptr;
}

uint32 SpellMgr::GetSpellInfoStoreSize() const
{
    return mSpellInfoMap.size();
}

std::list<uint32> const* SpellMgr::GetSpellOverrideInfo(uint32 spellId)
{
    return mSpellOverrideInfo.find(spellId) == mSpellOverrideInfo.end() ? nullptr : &mSpellOverrideInfo[spellId];
}

SpellInfo const* SpellMgr::AssertSpellInfo(uint32 spellId) const
{
    ASSERT(spellId < GetSpellInfoStoreSize());
    SpellInfo const* spellInfo = mSpellInfoMap[spellId];
    ASSERT(spellInfo);
    return spellInfo;
}

bool SpellMgr::IsTalent(uint32 spellId)
{
    return mTalentSpellInfo.find(spellId) != mTalentSpellInfo.end();
}

std::list<SkillLineAbilityEntry const*> const& SpellMgr::GetTradeSpellFromSkill(uint32 skillLineId)
{
    if (_skillTradeSpells.find(skillLineId) == _skillTradeSpells.end())
        _skillTradeSpells[skillLineId] = {};

    return _skillTradeSpells[skillLineId];
}

bool SpellArea::IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const
{
    if (areaId > 0)                                  // not in expected zone
        if (newZone != areaId && newArea != areaId)
            return false;

    if (player)
    {
        if (areaId < 0)
            if (player->GetMapId() != abs(areaId))
                return false;

        if (gender != GENDER_NONE)                   // not in expected gender
            if (gender != player->getGender())
                return false;

        if (raceMask)                                // not in expected race
            if (!(raceMask & player->getRaceMask()))
                return false;

        if (classmask)                                // not in expected class
        {
            uint32 checkClassMask = classmask > 0 ? classmask : (CLASSMASK_ALL_PLAYABLE + classmask);

            if (classmask > 0 && !(checkClassMask & player->getClassMask()))
                return false;
            if (classmask < 0 && !(checkClassMask & player->getClassMask()))
                return false;
        }

        if (questStart)                              // not in expected required quest state
            if (((questStartStatus & (1 << player->GetQuestStatus(questStart))) == 0))
                return false;

        if (questEnd)                                // not in expected forbidden quest state
            if ((questEndStatus & (1 << player->GetQuestStatus(questEnd))))
                return false;

        if (auraSpell)                               // not have expected aura
            if ((auraSpell > 0 && !player->HasAura(auraSpell)) || (auraSpell < 0 && player->HasAura(-auraSpell)))
                return false;

        if (!areaId && RequiredAreasID > 0)
        {
            uint32 oldZoneId = player->GetOldZoneID();
            uint32 oldAreaId = player->GetOldAreaID();

            if (!areaGroupMembers.empty())
                for (uint32 areaId : areaGroupMembers)
                    if (areaId == oldZoneId || areaId == oldAreaId)
                        return false;
        }

        if (active_event && !sGameEventMgr->IsActiveEvent(active_event))
            return false;
        
        
        // Extra conditions -- leaving the possibility add extra conditions...
        switch (spellId)
        {
            case 58600: // No fly Zone - Dalaran
            {
                AreaTableEntry const* pArea = sAreaTableStore.LookupEntry(player->GetCurrentAreaID());
                if (!(pArea && pArea->Flags[0] & AREA_FLAG_NO_FLY_ZONE))
                    return false;

                if (!player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !player->HasAuraType(SPELL_AURA_FLY))
                    return false;

                break;
            }
            case 91604: // No fly Zone - Wintergrasp
            {
                //Battlefield* Bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetCurrentZoneID());
                if (/*!Bf || Bf->CanFlyIn() || */(!player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) && !player->HasAuraType(SPELL_AURA_FLY)))
                    return false;
                break;
            }
            case 68719: // Oil Refinery - Isle of Conquest.
            case 68720: // Quarry - Isle of Conquest.
            {
                if (player->GetBattlegroundTypeId() != MS::Battlegrounds::BattlegroundTypeId::BattlegroundIsleOfConquest || !player->GetBattleground())
                    return false;

                uint8 nodeType = spellId == 68719 ? NODE_TYPE_REFINERY : NODE_TYPE_QUARRY;
                uint8 nodeState = player->GetTeamId() == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H;
                return static_cast<BattlegroundIsleOfConquest*>(player->GetBattleground())->GetNodeState(nodeType) == nodeState;
            }
            case 56618: // Horde Controls Factory Phase Shift
            case 56617: // Alliance Controls Factory Phase Shift
            {
                Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetCurrentZoneID());

                if (!bf || bf->GetTypeId() != BATTLEFIELD_WG)
                    return false;

                // team that controls the workshop in the specified area
                uint32 team = bf->GetData(newArea);

                if (team == TEAM_HORDE)
                    return spellId == 56618;
                if (team == TEAM_ALLIANCE)
                    return spellId == 56617;
                break;
            }
            case 57940: // Essence of Wintergrasp - Northrend
            case 58045: // Essence of Wintergrasp - Wintergrasp
            {
                if (Battlefield* battlefieldWG = sBattlefieldMgr->GetBattlefieldByBattleId(BATTLEFIELD_BATTLEID_WG))
                    return battlefieldWG->IsEnabled() && (player->GetTeamId() == battlefieldWG->GetDefenderTeam()) && !battlefieldWG->IsWarTime();
                break;
            }
            case 74411: // Battleground - Dampening
            {
                if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetCurrentZoneID()))
                    return bf->IsWarTime();
                break;
            }
            case 240010: //Class Hall Banishing
            {
                switch (player->GetCurrentAreaID())
                {
                    case 7679: //DK Hall
                        if (player->getClass() == CLASS_DEATH_KNIGHT)
                            return false;
                        break;
                    case 7752: //Shaman Hall
                    case 7753:
                        if (player->getClass() == CLASS_SHAMAN)
                            return false;
                        break;
                    case 7813: //Warrior Hall
                        if (player->getClass() == CLASS_WARRIOR)
                            return false;
                        break;
                    case 7879: //Mage Hall
                        if (player->getClass() == CLASS_MAGE)
                            return false;
                        break;
                    case 8012: //Rogue Hall
                        if (player->getClass() == CLASS_ROGUE)
                            return false;
                        break;
                    case 7875: //Warlock Hall
                        if (player->getClass() == CLASS_WARLOCK)
                            return false;
                        break;
                    case 7903: //Monk Hall
                        if (player->getClass() == CLASS_MONK)
                            return false;
                        break;
                    case 7834: //Priest Hall
                    case 8356:
                        if (player->getClass() == CLASS_PRIEST)
                            return false;
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    return true;
}

void SpellMgr::LoadSpellRanks()
{
    uint32 oldMSTime = getMSTime();

    std::map<uint32 /*spell*/, uint32 /*next*/> chains;
    std::set<uint32> hasPrev;
    for (SkillLineAbilityEntry const* skillAbility : sSkillLineAbilityStore)
    {
        if (!skillAbility->SupercedesSpell)
            continue;

        if (!GetSpellInfo(skillAbility->SupercedesSpell) || !GetSpellInfo(skillAbility->Spell))
            continue;

        chains[skillAbility->SupercedesSpell] = skillAbility->Spell;
        hasPrev.insert(skillAbility->Spell);
    }

    for (auto const& itr : chains)
    {
        if (hasPrev.count(itr.first))
            continue;

        if (itr.first >= GetSpellInfoStoreSize())
            continue;

        if (itr.second >= GetSpellInfoStoreSize())
            continue;

        SpellInfo const* first = AssertSpellInfo(itr.first);
        SpellInfo const* next = AssertSpellInfo(itr.second);

        mSpellChains[itr.first].first = first;
        mSpellChains[itr.first].prev = nullptr;
        mSpellChains[itr.first].next = next;
        mSpellChains[itr.first].last = next;
        mSpellChains[itr.first].rank = 1;
        mSpellInfoMap[itr.first]->ChainEntry = &mSpellChains[itr.first];

        mSpellChains[itr.second].first = first;
        mSpellChains[itr.second].prev = first;
        mSpellChains[itr.second].next = nullptr;
        mSpellChains[itr.second].last = next;
        mSpellChains[itr.second].rank = 2;
        mSpellInfoMap[itr.second]->ChainEntry = &mSpellChains[itr.second];

        uint8 rank = 3;
        auto nextItr = chains.find(itr.second);
        while (nextItr != chains.end())
        {
            SpellInfo const* prev = AssertSpellInfo(nextItr->first); // already checked in previous iteration (or above, in case this is the first one)
            SpellInfo const* last = AssertSpellInfo(nextItr->second);

            mSpellChains[nextItr->first].next = last;

            mSpellChains[nextItr->second].first = first;
            mSpellChains[nextItr->second].prev = prev;
            mSpellChains[nextItr->second].next = nullptr;
            mSpellChains[nextItr->second].last = last;
            mSpellChains[nextItr->second].rank = rank++;
            mSpellInfoMap[nextItr->second]->ChainEntry = &mSpellChains[nextItr->second];

            // fill 'last'
            do
            {
                mSpellChains[prev->Id].last = last;
                prev = mSpellChains[prev->Id].prev;
            } while (prev);

            nextItr = chains.find(nextItr->second);
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell rank records in %u ms", uint32(mSpellChains.size()), GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellRequired()
{
    uint32 oldMSTime = getMSTime();

    mSpellsReqSpell.clear();                                   // need for reload case
    mSpellReq.clear();                                         // need for reload case

    //                                                   0        1
    QueryResult result = WorldDatabase.Query("SELECT spell_id, req_spell from spell_required");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell required records. DB table `spell_required` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellInfo const* spell = GetSpellInfo(spell_id);
        if (!spell)
        {
            WorldDatabase.PExecute("DELETE FROM spell_required WHERE spell_id = %u;", spell_id);
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_required` does not exist", spell_id);
            continue;
        }

        uint32 spell_req = fields[1].GetUInt32();

        SpellInfo const* req_spell = GetSpellInfo(spell_req);
        if (!req_spell)
        {
            WorldDatabase.PExecute("DELETE FROM spell_required WHERE req_spell = %u;", spell_id);
            TC_LOG_ERROR(LOG_FILTER_SQL, "req_spell %u listed in `spell_required` does not exist", spell_id);
            continue;
        }

        if (GetFirstSpellInChain(spell_id) == GetFirstSpellInChain(spell_req))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "req_spell %u and spell_id %u in `spell_required` table are ranks of the same spell, entry not needed, skipped", spell_req, spell_id);
            continue;
        }

        if (IsSpellRequiringSpell(spell_id, spell_req))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "duplicated entry of req_spell %u and spell_id %u in `spell_required`, skipped", spell_req, spell_id);
            continue;
        }

        mSpellReq.insert(std::make_pair(spell_id, spell_req));
        mSpellsReqSpell.insert(std::make_pair(spell_req, spell_id));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell required records in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellLearnSkills()
{
    uint32 oldMSTime = getMSTime();

    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < GetSpellInfoStoreSize(); ++spell)
    {
        SpellInfo const* entry = GetSpellInfo(spell);
        if (!entry || !entry->HasEffectBit.test(SPELL_EFFECT_SKILL))
            continue;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (entry->EffectMask < uint32(1 << i))
                break;

            if (entry->Effects[i]->Effect == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill = entry->Effects[i]->MiscValue;
                dbc_node.step = entry->Effects[i]->CalcValue();
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value = 1;
                else
                    dbc_node.value = dbc_node.step * 75;
                dbc_node.maxvalue = dbc_node.step * 75;
                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u Spell Learn Skills from DBC in %u ms", dbc_count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellLearnSpells()
{
    uint32 oldMSTime = getMSTime();

    mSpellLearnSpells.clear();                              // need for reload case

    //                                                  0      1        2
    QueryResult result = WorldDatabase.Query("SELECT entry, SpellID, Active FROM spell_learn_spell");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell learn spells. DB table `spell_learn_spell` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        if (!GetSpellInfo(spell_id))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_learn_spell` does not exist", spell_id);
            WorldDatabase.PExecute("DELETE FROM `spell_learn_spell` WHERE entry = %u", spell_id);
            continue;
        }

        SpellLearnSpellNode node;
        node.spell = fields[1].GetUInt32();

        if (!GetSpellInfo(node.spell))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_learn_spell` learning not existed spell %u", spell_id, node.spell);
            WorldDatabase.PExecute("DELETE FROM `spell_learn_spell` WHERE SpellID = %u", node.spell);
            continue;
        }

        node.overridesSpell = 0;
        node.active = fields[2].GetBool();
        node.autoLearned = false;

        mSpellLearnSpells.insert(std::make_pair(spell_id, node));

        ++count;
    } while (result->NextRow());

    // copy state loaded from db
    SpellLearnSpellMap dbSpellLearnSpells = mSpellLearnSpells;

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < GetSpellInfoStoreSize(); ++spell)
    {
        SpellInfo const* entry = GetSpellInfo(spell);
        if (!entry || !entry->HasEffectBit.test(SPELL_EFFECT_LEARN_SPELL))
            continue;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (entry->EffectMask < uint32(1 << i))
                break;

            if (entry->Effects[i]->Effect == SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell = entry->Effects[i]->TriggerSpell;
                dbc_node.active = true;                     // all dbc based learned spells is active (show in spell book or hide by client itself)
                dbc_node.overridesSpell = 0;

                // ignore learning not existed spells (broken/outdated/or generic learnig spell 483
                if (!GetSpellInfo(dbc_node.spell))
                    continue;

                // talent or passive spells or skill-step spells auto-casted and not need dependent learning,
                // pet teaching spells must not be dependent learning (casted)
                // other required explicit dependent learning
                dbc_node.autoLearned = entry->Effects[i]->TargetA.GetTarget() == TARGET_UNIT_PET || entry->IsPassive() || entry->HasEffect(SPELL_EFFECT_SKILL_STEP);

                SpellLearnSpellMapBounds db_node_bounds = dbSpellLearnSpells.equal_range(spell);

                bool found = false;
                for (SpellLearnSpellMap::const_iterator itr = db_node_bounds.first; itr != db_node_bounds.second; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.", spell, dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(std::make_pair(spell, dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    for (SpellLearnSpellEntry const* spellLearnSpell : sSpellLearnSpellStore)
    {
        if (!GetSpellInfo(spellLearnSpell->SpellID))
            continue;

        SpellLearnSpellMapBounds db_node_bounds = dbSpellLearnSpells.equal_range(spellLearnSpell->LearnSpellID);
        bool found = false;
        for (SpellLearnSpellMap::const_iterator itr = db_node_bounds.first; itr != db_node_bounds.second; ++itr)
        {
            if (itr->second.spell == spellLearnSpell->SpellID)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Found redundant record (entry: %u, SpellID: %u) in `spell_learn_spell`, spell added automatically from SpellLearnSpell.db2", spellLearnSpell->LearnSpellID, spellLearnSpell->SpellID);
                found = true;
                break;
            }
        }

        if (found)
            continue;

        // Check if it is already found in Spell.dbc, ignore silently if yes
        SpellLearnSpellMapBounds dbc_node_bounds = GetSpellLearnSpellMapBounds(spellLearnSpell->LearnSpellID);
        found = false;
        for (SpellLearnSpellMap::const_iterator itr = dbc_node_bounds.first; itr != dbc_node_bounds.second; ++itr)
        {
            if (itr->second.spell == spellLearnSpell->SpellID)
            {
                found = true;
                break;
            }
        }

        if (found)
            continue;

        SpellLearnSpellNode dbcLearnNode;
        dbcLearnNode.spell = spellLearnSpell->LearnSpellID;
        dbcLearnNode.overridesSpell = spellLearnSpell->OverridesSpellID;
        dbcLearnNode.active = true;
        dbcLearnNode.autoLearned = false;

        mSpellLearnSpells.insert(std::make_pair(spellLearnSpell->SpellID, dbcLearnNode));
        ++dbc_count;
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell learn spells, %u found in Spell.dbc in %u ms", count, dbc_count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellTargetPositions()
{
    uint32 oldMSTime = getMSTime();

    mSpellTargetPositions.clear();                                // need for reload case

    //                                                0      1              2                  3                  4                  5
    QueryResult result = WorldDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell target coordinates. DB table `spell_target_position` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId = fields[1].GetUInt16();
        st.target_X = fields[2].GetFloat();
        st.target_Y = fields[3].GetFloat();
        st.target_Z = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if (!mapEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", Spell_ID, st.target_mapId);
            continue;
        }

        if (st.target_X == 0 && st.target_Y == 0 && st.target_Z == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell (ID:%u) target coordinates not provided.", Spell_ID);
            continue;
        }

        SpellInfo const* spellInfo = GetSpellInfo(Spell_ID);
        if (!spellInfo)
        {
            WorldDatabase.PExecute("DELETE FROM spell_target_position WHERE id = %u;", Spell_ID);
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_target_position` does not exist", Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;
        ++count;

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell teleport coordinates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellGroups()
{
    uint32 oldMSTime = getMSTime();

    mSpellSpellGroup.clear();                                  // need for reload case
    mSpellGroupSpell.clear();

    //                                                0     1
    QueryResult result = WorldDatabase.Query("SELECT id, spell_id FROM spell_group");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell group definitions. DB table `spell_group` is empty.");
        return;
    }

    std::set<uint32> groups;
    do
    {
        Field* fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        if (group_id <= SPELL_GROUP_DB_RANGE_MIN && group_id >= SPELL_GROUP_CORE_RANGE_MAX)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "SpellGroup id %u listed in `spell_group` is in core range, but is not defined in core!", group_id);
            continue;
        }

        groups.insert(group_id);
        mSpellGroupSpell.insert(std::make_pair(static_cast<SpellGroup>(group_id), fields[1].GetInt32()));

    } while (result->NextRow());

    for (SpellGroupSpellMap::iterator itr = mSpellGroupSpell.begin(); itr != mSpellGroupSpell.end();)
    {
        if (itr->second < 0)
        {
            if (groups.find(abs(itr->second)) == groups.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "SpellGroup id %i listed in `spell_group` does not exist", itr->second);
                //WorldDatabase.PExecute("DELETE FROM `spell_group` WHERE spell_id = %i", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
        else
        {
            SpellInfo const* spellInfo = GetSpellInfo(itr->second);

            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_group` does not exist", itr->second);
                //WorldDatabase.PExecute("DELETE FROM `spell_group` WHERE spell_id = %u", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            else if (spellInfo->GetRank() > 1)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_group` is not first rank of spell", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
    }

    for (std::set<uint32>::iterator groupItr = groups.begin(); groupItr != groups.end(); ++groupItr)
    {
        std::set<uint32> spells;
        GetSetOfSpellsInSpellGroup(SpellGroup(*groupItr), spells);

        for (std::set<uint32>::iterator spellItr = spells.begin(); spellItr != spells.end(); ++spellItr)
            mSpellSpellGroup.insert(std::make_pair(*spellItr, SpellGroup(*groupItr)));
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell group definitions in %u ms", uint32(mSpellSpellGroup.size()), GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellGroupStackRules()
{
    uint32 oldMSTime = getMSTime();

    mSpellGroupStack.clear();                                  // need for reload case

    //                                                       0         1
    QueryResult result = WorldDatabase.Query("SELECT group_id, stack_rule FROM spell_group_stack_rules");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell group stack rules. DB table `spell_group_stack_rules` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint8 stack_rule = fields[1].GetInt8();
        if (stack_rule >= SPELL_GROUP_STACK_RULE_MAX)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "SpellGroupStackRule %u listed in `spell_group_stack_rules` does not exist", stack_rule);
            continue;
        }

        uint32 group_id = fields[0].GetUInt32();
        SpellGroupSpellMapBounds spellGroup = GetSpellGroupSpellMapBounds(static_cast<SpellGroup>(group_id));

        if (spellGroup.first == spellGroup.second)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "SpellGroup id %u listed in `spell_group_stack_rules` does not exist", group_id);
            continue;
        }

        mSpellGroupStack[static_cast<SpellGroup>(group_id)] = static_cast<SpellGroupStackRule>(stack_rule);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell group stack rules in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadForbiddenSpells()
{
    uint32 oldMSTime = getMSTime();

    mForbiddenSpells.clear();

    uint32 count = 0;

    QueryResult result = WorldDatabase.Query("SELECT spell_id FROM spell_forbidden");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell group definitions", count);
        return;
    }

    do
    {
        mForbiddenSpells.push_back(result->Fetch()[0].GetUInt32());
        ++count;
    } while (result->NextRow());


    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u forbidden spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellProcEvents()
{
    uint32 oldMSTime = getMSTime();

    mSpellProcEventMap.clear();                             // need for reload case
    mSpellProcEventMap.resize(GetSpellInfoStoreSize());

    //                                               0      1           2                3                 4                 5                 6                 7          8       9        10            11        12
    QueryResult result = WorldDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask0, SpellFamilyMask1, SpellFamilyMask2, SpellFamilyMask3, procFlags, procEx, ppmRate, CustomChance, Cooldown, effectmask FROM spell_proc_event");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell proc event conditions. DB table `spell_proc_event` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 customProc = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        SpellInfo const* spell = GetSpellInfo(entry);
        if (!spell)
        {
            WorldDatabase.PExecute("DELETE FROM spell_proc_event WHERE entry = %u;", entry);
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask = fields[1].GetInt8();
        spe.spellFamilyName = fields[2].GetUInt16();
        spe.spellFamilyMask[0] = fields[3].GetUInt32();
        spe.spellFamilyMask[1] = fields[4].GetUInt32();
        spe.spellFamilyMask[2] = fields[5].GetUInt32();
        spe.spellFamilyMask[3] = fields[6].GetUInt32();
        spe.procFlags = fields[7].GetUInt32();
        spe.procEx = fields[8].GetUInt32();
        spe.ppmRate = fields[9].GetFloat();
        spe.customChance = fields[10].GetFloat();
        spe.cooldown = fields[11].GetFloat();
        spe.effectMask = fields[12].GetUInt32();

        mSpellProcEventMap[entry].push_back(spe);

        if (spell->GetAuraOptions()->ProcTypeMask == 0)
        {
            if (spe.procFlags == 0)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_proc_event` probally not triggered spell", entry);
                continue;
            }

            const_cast<SpellInfo*>(spell)->GetAuraOptions()->IsProcAura = true;
            customProc++;
        }
        ++count;
    } while (result->NextRow());

    if (customProc)
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u extra and %u custom spell proc event conditions in %u ms", count, customProc, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u extra spell proc event conditions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void SpellMgr::LoadSpellProcs()
{
    uint32 oldMSTime = getMSTime();

    mSpellProcMap.clear();                             // need for reload case

    //                                               0        1           2                3                 4                 5                 6                 7         8              9               10       11              12             13      14        15       16
    QueryResult result = WorldDatabase.Query("SELECT spellId, schoolMask, spellFamilyName, spellFamilyMask0, spellFamilyMask1, spellFamilyMask2, spellFamilyMask3, typeMask, spellTypeMask, spellPhaseMask, hitMask, attributesMask, ratePerMinute, chance, cooldown, charges, modcharges FROM spell_proc");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell proc conditions and data. DB table `spell_proc` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 spellId = fields[0].GetInt32();

        bool allRanks = false;
        if (spellId <= 0)
        {
            allRanks = true;
            spellId = -spellId;
        }

        SpellInfo const* spellEntry = GetSpellInfo(spellId);
        if (!spellEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_proc` does not exist", spellId);
            continue;
        }

        if (allRanks)
        {
            if (GetFirstSpellInChain(spellId) != uint32(spellId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_proc` is not first rank of spell.", fields[0].GetInt32());
                continue;
            }
        }

        SpellProcEntry baseProcEntry;

        baseProcEntry.schoolMask = fields[1].GetInt8();
        baseProcEntry.spellFamilyName = fields[2].GetUInt16();
        baseProcEntry.spellFamilyMask[0] = fields[3].GetUInt32();
        baseProcEntry.spellFamilyMask[1] = fields[4].GetUInt32();
        baseProcEntry.spellFamilyMask[2] = fields[5].GetUInt32();
        baseProcEntry.spellFamilyMask[3] = fields[6].GetUInt32();
        baseProcEntry.typeMask = fields[7].GetUInt32();
        baseProcEntry.spellTypeMask = fields[8].GetUInt32();
        baseProcEntry.spellPhaseMask = fields[9].GetUInt32();
        baseProcEntry.hitMask = fields[10].GetUInt32();
        baseProcEntry.attributesMask = fields[11].GetUInt32();
        baseProcEntry.ratePerMinute = fields[12].GetFloat();
        baseProcEntry.chance = fields[13].GetFloat();
        baseProcEntry.cooldown = fields[14].GetFloat();
        baseProcEntry.charges = fields[15].GetUInt32();
        baseProcEntry.modcharges = fields[16].GetUInt32();

        while (true)
        {
            if (mSpellProcMap.find(spellId) != mSpellProcMap.end())
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_proc` has duplicate entry in the table", spellId);
                break;
            }
            SpellProcEntry procEntry = SpellProcEntry(baseProcEntry);

            // take defaults from dbcs
            if (!procEntry.typeMask)
                procEntry.typeMask = spellEntry->GetAuraOptions()->ProcTypeMask;
            if (!procEntry.charges)
                procEntry.charges = spellEntry->GetAuraOptions()->ProcCharges;
            if (!procEntry.chance && !procEntry.ratePerMinute)
                procEntry.chance = float(spellEntry->GetAuraOptions()->ProcChance);

            // validate data
            if (procEntry.schoolMask & ~SPELL_SCHOOL_MASK_ALL)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has wrong `schoolMask` set: %u", spellId, procEntry.schoolMask);
            if (procEntry.spellFamilyName && (procEntry.spellFamilyName < 3 || procEntry.spellFamilyName > 17 || procEntry.spellFamilyName == 14 || procEntry.spellFamilyName == 16))
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has wrong `spellFamilyName` set: %u", spellId, procEntry.spellFamilyName);
            if (procEntry.chance < 0.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has negative value in `chance` field", spellId);
                procEntry.chance = 0.0f;
            }
            if (procEntry.ratePerMinute < 0.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has negative value in `ratePerMinute` field", spellId);
                procEntry.ratePerMinute = 0.0f;
            }
            if (baseProcEntry.cooldown < 0.0f)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has negative value in `cooldown` field", spellId);
                procEntry.cooldown = 0.0f;
            }
            if (procEntry.chance == 0.0f && procEntry.ratePerMinute == 0.0f)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u doesn't have `chance` and `ratePerMinute` values defined, proc will not be triggered", spellId);
            if (procEntry.charges > 99)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has too big value in `charges` field", spellId);
                procEntry.charges = 99;
            }
            if (!procEntry.typeMask)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u doesn't have `typeMask` value defined, proc will not be triggered", spellId);
            if (procEntry.spellTypeMask & ~PROC_SPELL_PHASE_MASK_ALL)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has wrong `spellTypeMask` set: %u", spellId, procEntry.spellTypeMask);
            if (procEntry.spellTypeMask && !(procEntry.typeMask & (SPELL_PROC_FLAG_MASK | PERIODIC_PROC_FLAG_MASK)))
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has `spellTypeMask` value defined, but it won't be used for defined `typeMask` value", spellId);
            if (!procEntry.spellPhaseMask && procEntry.typeMask & REQ_SPELL_PHASE_PROC_FLAG_MASK)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u doesn't have `spellPhaseMask` value defined, but it's required for defined `typeMask` value, proc will not be triggered", spellId);
            if (procEntry.spellPhaseMask & ~PROC_SPELL_PHASE_MASK_ALL)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has wrong `spellPhaseMask` set: %u", spellId, procEntry.spellPhaseMask);
            if (procEntry.spellPhaseMask && !(procEntry.typeMask & REQ_SPELL_PHASE_PROC_FLAG_MASK))
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has `spellPhaseMask` value defined, but it won't be used for defined `typeMask` value", spellId);
            if (procEntry.hitMask & ~PROC_HIT_MASK_ALL)
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has wrong `hitMask` set: %u", spellId, procEntry.hitMask);
            if (procEntry.hitMask && !(procEntry.typeMask & TAKEN_HIT_PROC_FLAG_MASK || (procEntry.typeMask & DONE_HIT_PROC_FLAG_MASK && (!procEntry.spellPhaseMask || procEntry.spellPhaseMask & (PROC_SPELL_PHASE_HIT | PROC_SPELL_PHASE_FINISH)))))
                TC_LOG_ERROR(LOG_FILTER_SQL, "`spell_proc` table entry for spellId %u has `hitMask` value defined, but it won't be used for defined `typeMask` and `spellPhaseMask` values", spellId);

            mSpellProcMap[spellId] = procEntry;

            if (allRanks)
            {
                spellId = GetNextSpellInChain(spellId);
                spellEntry = GetSpellInfo(spellId);
            }
            else
                break;
        }
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell proc conditions and data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellBonusess()
{
    uint32 oldMSTime = getMSTime();

    mSpellBonusMap.clear();                             // need for reload case
    mSpellBonusVector.assign(sSpellStore.GetNumRows(), nullptr);

    //                                                0      1             2          3         4                   5           6
    QueryResult result = WorldDatabase.Query("SELECT entry, direct_bonus, dot_bonus, ap_bonus, ap_dot_bonus, damage_bonus, heal_bonus FROM spell_bonus_data");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell bonus data. DB table `spell_bonus_data` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();

        if (!GetSpellInfo(entry))
        {
            WorldDatabase.PExecute("DELETE FROM spell_bonus_data WHERE entry = %u;", entry);
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_bonus_data` does not exist", entry);
            continue;
        }

        SpellBonusEntry& sbe = mSpellBonusMap[entry];
        mSpellBonusVector[entry] = &sbe;
        sbe.direct_damage = fields[1].GetFloat();
        sbe.dot_damage = fields[2].GetFloat();
        sbe.ap_bonus = fields[3].GetFloat();
        sbe.ap_dot_bonus = fields[4].GetFloat();
        sbe.damage_bonus = fields[5].GetFloat();
        sbe.heal_bonus = fields[6].GetFloat();

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u extra spell bonus data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellThreats()
{
    uint32 oldMSTime = getMSTime();

    mSpellThreatMap.clear();                                // need for reload case

    //                                                0      1        2       3
    QueryResult result = WorldDatabase.Query("SELECT entry, flatMod, pctMod, apPctMod FROM spell_threat");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 aggro generating spells. DB table `spell_threat` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();

        if (!GetSpellInfo(entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_threat` does not exist", entry);
            continue;
        }

        SpellThreatEntry ste;
        ste.flatMod = fields[1].GetInt32();
        ste.pctMod = fields[2].GetFloat();
        ste.apPctMod = fields[3].GetFloat();

        mSpellThreatMap[entry] = ste;
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u SpellThreatEntries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    uint32 oldMSTime = getMSTime();

    mSkillLineAbilityMap.clear();

    for (SkillLineAbilityEntry const* SkillInfo : sSkillLineAbilityStore)
        mSkillLineAbilityMap.insert(std::make_pair(SkillInfo->Spell, SkillInfo));

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u SkillLineAbility MultiMap Data in %u ms", static_cast<uint32>(mSkillLineAbilityMap.size()), GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellPetAuras()
{
    uint32 oldMSTime = getMSTime();

    mSpellPetAuraMap.clear();                                  // need for reload case

    //                                                    0          1         2         3            4         5      6     7        8         9              10            11
    QueryResult result = WorldDatabase.Query("SELECT `petEntry`, `spellId`, `option`, `target`, `targetaura`, `bp0`, `bp1`, `bp2`, `aura`, `casteraura`, `createdspell`, `fromspell` FROM `spell_pet_auras`");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell pet auras. DB table `spell_pet_auras` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 spellId = fields[1].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(spellId));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_pet_auras` does not exist", spellId);
            continue;
        }

        int32 petEntry = fields[0].GetInt32();

        PetAura tempPetAura;
        tempPetAura.petEntry = petEntry;
        tempPetAura.spellId = spellId;
        tempPetAura.option = fields[2].GetInt32();
        tempPetAura.target = fields[3].GetInt32();
        tempPetAura.targetaura = fields[4].GetInt32();
        tempPetAura.bp0 = fields[5].GetFloat();
        tempPetAura.bp1 = fields[6].GetFloat();
        tempPetAura.bp2 = fields[7].GetFloat();
        tempPetAura.aura = fields[8].GetInt32();
        tempPetAura.casteraura = fields[9].GetInt32();
        tempPetAura.createdspell = fields[10].GetInt32();
        tempPetAura.fromspell = fields[11].GetInt32();
        mSpellPetAuraMap[petEntry].push_back(tempPetAura);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell pet auras in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

// Fill custom data about enchancments
void SpellMgr::LoadEnchantCustomAttr()
{
    uint32 oldMSTime = getMSTime();

    uint32 size = sSpellItemEnchantmentStore.GetNumRows();
    mEnchantCustomAttr.resize(size);

    for (uint32 i = 0; i < size; ++i)
        mEnchantCustomAttr[i] = false;

    uint32 count = 0;
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        SpellInfo const* spellInfo = GetSpellInfo(i);
        if (!spellInfo)
            continue;

        // TODO: find a better check
        if (!(spellInfo->HasAttribute(SPELL_ATTR2_PRESERVE_ENCHANT_IN_ARENA)) || !(spellInfo->HasAttribute(SPELL_ATTR0_NOT_SHAPESHIFT)))
            continue;

        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            if (spellInfo->EffectMask < uint32(1 << j))
                break;

            if (spellInfo->Effects[j]->Effect == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY)
            {
                uint32 enchId = spellInfo->Effects[j]->MiscValue;
                if (!sSpellItemEnchantmentStore.LookupEntry(enchId))
                    continue;

                mEnchantCustomAttr[enchId] = true;
                ++count;
                break;
            }
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u custom enchant attributes in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellEnchantProcData()
{
    uint32 oldMSTime = getMSTime();

    mSpellEnchantProcEventMap.clear();                             // need for reload case

    //                                                  0         1           2         3
    QueryResult result = WorldDatabase.Query("SELECT entry, customChance, PPMChance, procEx FROM spell_enchant_proc_data");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell enchant proc event conditions. DB table `spell_enchant_proc_data` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 enchantId = fields[0].GetUInt32();
        if (!sSpellItemEnchantmentStore.LookupEntry(enchantId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Enchancment %u listed in `spell_enchant_proc_data` does not exist", enchantId);
            continue;
        }

        SpellEnchantProcEntry spe;
        spe.customChance = fields[1].GetUInt32();
        spe.PPMChance = fields[2].GetFloat();
        spe.procEx = fields[3].GetUInt32();
        mSpellEnchantProcEventMap[enchantId] = spe;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u enchant proc data definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellLinked()
{
    uint32 oldMSTime = getMSTime();

    for (uint8 i = 0; i < SPELL_LINK_MAX; ++i)
        mSpellLinkedMap[i].clear();    // need for reload case

    //                                                0              1             2      3       4         5          6         7        8       9         10        11          12         13           14             15          16         17        18         19          20         21          22
    QueryResult result = WorldDatabase.Query("SELECT spell_trigger, spell_effect, type, caster, target, hastalent, hastalent2, chance, cooldown, hastype, hitmask, removeMask, hastype2, actiontype, targetCount, targetCountType, `group`, `duration`, `param`, effectMask , hasparam , hasparam2, `randList` FROM spell_linked_spell");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 linked spells. DB table `spell_linked_spell` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 trigger = fields[0].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(trigger));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_linked_spell` does not exist", trigger);
            WorldDatabase.PExecute("DELETE FROM `spell_linked_spell` WHERE spell_trigger = %i", trigger);
            continue;
        }

        int32 effect = fields[1].GetInt32();
        spellInfo = GetSpellInfo(abs(effect));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_linked_spell` does not exist", effect);
            WorldDatabase.PExecute("DELETE FROM `spell_linked_spell` WHERE spell_effect = %i", effect);
            continue;
        }

        int32 type = fields[2].GetUInt8();
        if (type >= SPELL_LINK_MAX)
            continue;

        SpellLinked templink;
        templink.effect = effect;
        templink.hastalent = fields[5].GetInt32();
        templink.hastalent2 = fields[6].GetInt32();
        templink.chance = fields[7].GetInt32();
        templink.cooldown = fields[8].GetInt32();
        templink.hastype = fields[9].GetUInt8();
        templink.caster = fields[3].GetUInt8();
        templink.target = fields[4].GetUInt8();
        templink.hitmask = fields[10].GetUInt32();
        templink.removeMask = fields[11].GetInt32();
        templink.hastype2 = fields[12].GetInt32();
        templink.actiontype = fields[13].GetInt32();
        templink.targetCount = fields[14].GetInt32();
        templink.targetCountType = fields[15].GetInt32();
        templink.group = fields[16].GetInt32();
        templink.duration = fields[17].GetInt32();
        templink.param = fields[18].GetFloat();
        templink.effectMask = fields[19].GetUInt32();
        templink.hasparam = fields[20].GetInt32();
        templink.hasparam2 = fields[21].GetInt32();
        Tokenizer randToken(fields[22].GetString(), ' ');
        for (Tokenizer::const_iterator itr = randToken.begin(); itr != randToken.end(); ++itr)
            templink.randList.push_back(int32(atol(*itr)));

        mSpellLinkedMap[type][trigger].push_back(templink);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u linked spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadTalentSpellLinked()
{
    uint32 oldMSTime = getMSTime();

    mSpellTalentLinkedMap.clear();    // need for reload case

    //                                                  0        1        2       3       4
    QueryResult result = WorldDatabase.Query("SELECT spellid, spelllink, type, target, caster FROM spell_talent_linked_spell");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 linked talent spells. DB table `spell_talent_linked_spell` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 talent = fields[0].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(talent));
        if (!spellInfo && talent)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_talent_linked_spell` does not exist", talent);
            WorldDatabase.PExecute("DELETE FROM `spell_talent_linked_spell` WHERE spellid = %i", talent);
            continue;
        }

        int32 triger = fields[1].GetInt32();
        spellInfo = GetSpellInfo(abs(triger));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_talent_linked_spell` does not exist", triger);
            WorldDatabase.PExecute("DELETE FROM `spell_talent_linked_spell` WHERE spelllink = %i", triger);
            continue;
        }

        SpellTalentLinked templink;
        templink.talent = talent;
        templink.triger = triger;
        templink.type = fields[2].GetUInt8();
        templink.target = fields[3].GetUInt8();
        templink.caster = fields[4].GetUInt8();
        mSpellTalentLinkedMap[talent].push_back(templink);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u linked talent spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellConcatenateAura()
{
    uint32 oldMSTime = getMSTime();

    mSpellConcatenateApplyMap.clear();    // need for reload case
    mSpellConcatenateUpdateMap.clear();    // need for reload case

    //                                                   0            1           2            3          4         5         6
    QueryResult result = WorldDatabase.Query("SELECT `spellid`, `effectSpell`, `auraId`, `effectAura`, `caster`, `target`, `option` FROM spell_concatenate_aura");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 concatenate auraspells. DB table `spell_concatenate_aura` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 spellid = fields[0].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(spellid));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_concatenate_aura` does not exist", spellid);
            continue;
        }

        int32 auraId = fields[2].GetInt32();
        spellInfo = GetSpellInfo(abs(auraId));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_concatenate_aura` does not exist", auraId);
            continue;
        }

        SpellConcatenateAura tempAura;
        tempAura.spellid = spellid;
        tempAura.effectSpell = fields[1].GetUInt8();
        tempAura.auraId = auraId;
        tempAura.effectAura = fields[3].GetUInt8();
        tempAura.caster = fields[4].GetUInt8();
        tempAura.target = fields[5].GetUInt8();
        tempAura.option = fields[6].GetUInt8();
        mSpellConcatenateUpdateMap[spellid].push_back(tempAura);
        mSpellConcatenateApplyMap[auraId].push_back(tempAura);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u concatenate aura spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellVisual()
{
    uint32 oldMSTime = getMSTime();

    mSpellVisualMap.clear();    // need for reload case
    mSpellVisualPlayOrphanMap.clear();    // need for reload case

    //                                                  0            1            2            3             4            5        6        7
    QueryResult result = WorldDatabase.Query("SELECT spellId, SpellVisualID, MissReason, ReflectStatus, TravelSpeed, SpeedAsTime, type, HasPosition FROM spell_visual");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 visual spells. DB table `spell_visual` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 spellId = fields[0].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(spellId));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_visual` does not exist", abs(spellId));
            continue;
        }

        SpellVisual templink;
        templink.spellId = spellId;
        templink.SpellVisualID = fields[1].GetInt32();
        templink.MissReason = fields[2].GetUInt16();
        templink.ReflectStatus = fields[3].GetUInt16();
        templink.TravelSpeed = fields[4].GetFloat();
        templink.SpeedAsTime = bool(fields[5].GetUInt8());
        templink.type = fields[6].GetUInt8();
        templink.HasPosition = bool(fields[7].GetUInt8());
        mSpellVisualMap[spellId].push_back(templink);

        ++count;
    } while (result->NextRow());

    //                                      0            1            2            3          4      5  6  7    8
    result = WorldDatabase.Query("SELECT spellId, SpellVisualID, TravelSpeed, SpeedAsTime, UnkFloat, X, Y, Z, `type` FROM spell_visual_play_orphan");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 visual spells. DB table `spell_visual_play_orphan` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        int32 spellId = fields[0].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(spellId));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_visual_play_orphan` does not exist", abs(spellId));
            continue;
        }

        SpellVisualPlayOrphan templink;
        templink.spellId = spellId;
        templink.SpellVisualID = fields[1].GetInt32();
        templink.TravelSpeed = fields[2].GetFloat();
        templink.SpeedAsTime = bool(fields[3].GetUInt8());
        templink.UnkFloat = fields[4].GetFloat();
        templink.type = fields[8].GetInt8();
        templink.SourceOrientation.Relocate(fields[5].GetFloat(), fields[6].GetFloat(), fields[7].GetFloat());
        mSpellVisualPlayOrphanMap[spellId].push_back(templink);

        ++count;
    } while (result->NextRow());

    //                                      0        1         2        3
    result = WorldDatabase.Query("SELECT spellId, KitType, KitRecID, Duration FROM spell_visual_kit");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 visual spells. DB table `spell_visual_kit` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        int32 spellId = fields[0].GetInt32();

        if (!GetSpellInfo(abs(spellId)))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_visual_kit` does not exist", abs(spellId));
            continue;
        }

        SpellVisualKit templink;
        templink.spellId = spellId;
        templink.KitType = fields[1].GetInt32();
        templink.KitRecID = fields[2].GetInt32();
        templink.Duration = fields[3].GetInt32();
        mSpellVisualKitMap[spellId].push_back(templink);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u visual spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellScene()
{
    uint32 oldMSTime = getMSTime();

    mSpellSceneMap.clear();
    mSceneTriggerEventMap.clear();

    //                                                        0                1           2              3               4
    QueryResult result = WorldDatabase.Query("SELECT SceneScriptPackageID, MiscValue, PlaybackFlags, CustomDuration, ScriptName FROM spell_scene");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 visual spells. DB table `spell_scene` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        int8 ind = 0;

        SpellScene templink;
        templink.SceneScriptPackageID = fields[ind++].GetInt32();
        templink.MiscValue = fields[ind++].GetInt32();
        templink.PlaybackFlags = fields[ind++].GetInt32();
        templink.CustomDuration = fields[ind++].GetInt32();
        templink.scriptID = sObjectMgr->GetScriptId(fields[ind++].GetString().c_str());

        mSpellSceneMap[templink.MiscValue] = templink;

        ++count;
    } while (result->NextRow());

    //                                       0           1             2          3
    result = WorldDatabase.Query("SELECT MiscValue, trigerSpell, MonsterCredit, Event FROM spell_scene_event");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 visual spells. DB table `spell_scene_event` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        int8 ind = 0;

        SceneTriggerEvent templink;
        templink.MiscValue = fields[ind++].GetInt32();
        templink.trigerSpell = fields[ind++].GetInt32();
        templink.MonsterCredit = fields[ind++].GetInt32();
        templink.Event = fields[ind++].GetString();

        mSceneTriggerEventMap[templink.MiscValue].push_back(templink);

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u visual spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellPendingCast()
{
    uint32 oldMSTime = getMSTime();

    mSpellPendingCastMap.clear();    // need for reload case

    //                                                  0          1          2      3
    QueryResult result = WorldDatabase.Query("SELECT `spell_id`, `pending_id`, `option`, `check` FROM `spell_pending_cast`");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 pending spells. DB table `spell_pending_cast` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 spell_id = fields[0].GetInt32();
        SpellInfo const* spellInfo = GetSpellInfo(abs(spell_id));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "spell_id %u listed in `spell_pending_cast` does not exist", abs(spell_id));
            continue;
        }

        int32 pending_id = fields[1].GetInt32();
        spellInfo = GetSpellInfo(abs(pending_id));
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "pending_id %u listed in `spell_pending_cast` does not exist", abs(pending_id));
            continue;
        }

        SpellPendingCast templink;
        templink.spell_id = spell_id;
        templink.pending_id = pending_id;
        templink.option = fields[2].GetUInt8();
        templink.check = fields[3].GetInt32();
        mSpellPendingCastMap[spell_id].push_back(templink);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pending spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellPrcoCheck()
{
    uint32 oldMSTime = getMSTime();

    mSpellPrcoCheckMap.clear();    // need for reload case
    mSpellPrcoCheckMap.resize(GetSpellInfoStoreSize());

    //                                                0        1       2      3             4         5      6          7           8         9        10       11            12              13          14       15          16           17             18             19
    QueryResult result = WorldDatabase.Query("SELECT entry, entry2, entry3, checkspell, hastalent, chance, target, effectmask, powertype, dmgclass, specId, spellAttr0, targetTypeMask, mechanicMask, fromlevel, perchp, spelltypeMask, combopoints, deathstateMask, hasDuration FROM spell_proc_check");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 proc check spells. DB table `spell_proc_check` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        int32 entry = fields[0].GetInt32();
        if (!GetSpellInfo(abs(entry)))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_proc_check` does not exist", abs(entry));
            WorldDatabase.PExecute("DELETE FROM `spell_proc_check` WHERE entry = %u", abs(entry));
            continue;
        }

        int32 entry2 = fields[1].GetInt32();
        int32 entry3 = fields[2].GetInt32();
        SpellPrcoCheck templink;
        templink.checkspell = fields[3].GetInt32();
        templink.hastalent = fields[4].GetInt32();
        templink.chance = fields[5].GetInt32();
        templink.target = fields[6].GetInt32();
        templink.powertype = fields[8].GetInt32();
        templink.dmgclass = fields[9].GetInt32();
        templink.effectmask = fields[7].GetInt32();
        templink.specId = fields[10].GetInt32();
        templink.spellAttr0 = fields[11].GetInt32();
        templink.targetTypeMask = fields[12].GetInt32();
        templink.mechanicMask = fields[13].GetUInt32();
        templink.fromlevel = fields[14].GetInt32();
        templink.perchp = fields[15].GetInt32();
        templink.spelltypeMask = fields[16].GetInt32();
        templink.combopoints = fields[17].GetInt32();
        templink.deathstateMask = fields[18].GetInt32();
        templink.hasDuration = fields[19].GetInt32();
        mSpellPrcoCheckMap[entry].push_back(templink);

        if (entry2)
            mSpellPrcoCheckMap[entry2].push_back(templink);
        if (entry3)
            mSpellPrcoCheckMap[entry3].push_back(templink);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u proc check spells in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellTriggered()
{
    uint32 oldMSTime = getMSTime();

    mSpellTriggeredMap.clear();    // need for reload case
    mSpellTriggeredVector.assign(sSpellStore.GetNumRows(), nullptr);
    mSpellDummyTriggerMap.clear();    // need for reload case
    mSpellDummyTriggerVector.assign(sSpellStore.GetNumRows(), nullptr);
    mSpellAuraTriggerMap.clear();    // need for reload case
    mSpellAuraTriggerVector.assign(sSpellStore.GetNumRows(), nullptr);
    mSpellAuraDummyMap.clear();    // need for reload case
    mSpellAuraDummyVector.assign(sSpellStore.GetNumRows(), nullptr);
    mSpellTargetFilterMap.clear();    // need for reload case
    mSpellTargetFilterVector.assign(sSpellStore.GetNumRows(), nullptr);
    mSpellCheckCastMap.clear();    // need for reload case
    mSpellCheckCastVector.assign(sSpellStore.GetNumRows(), nullptr);
    mSpellTriggerDelayMap.clear();    // need for reload case
    mSpellTriggerDelayVector.assign(sSpellStore.GetNumRows(), nullptr);

    uint32 count = 0;
    //                                                    0           1                    2           3         4          5          6          7      8      9         10         11       12       13         14          15            16            17           18          19           20              21          22          23           24        25
    QueryResult result = WorldDatabase.Query("SELECT `spell_id`, `spell_trigger`, `spell_cooldown`, `option`, `target`, `caster`, `targetaura`, `bp0`, `bp1`, `bp2`, `effectmask`, `aura`, `chance`, `group`, `procFlags`, `procEx`, `check_spell_id`, `addptype`, `schoolMask`, `dummyId`, `dummyEffect`, `targetaura2`, `aura2`, `CreatureType`, `slot`, `randList` FROM `spell_trigger`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            int32 spell_id = fields[0].GetInt32();
            int32 spell_trigger = fields[1].GetInt32();
            int32 spell_cooldown = fields[2].GetInt32();
            int32 option = fields[3].GetInt32();
            int32 target = fields[4].GetInt32();
            int32 caster = fields[5].GetInt32();
            int32 targetaura = fields[6].GetInt32();
            float bp0 = fields[7].GetFloat();
            float bp1 = fields[8].GetFloat();
            float bp2 = fields[9].GetFloat();
            int32 effectmask = fields[10].GetInt32();
            int32 aura = fields[11].GetInt32();
            int32 chance = fields[12].GetInt32();
            int32 group = fields[13].GetInt32();
            int32 procFlags = fields[14].GetInt32();
            int32 procEx = fields[15].GetInt32();
            int32 check_spell_id = fields[16].GetInt32();
            int32 addptype = fields[17].GetInt32();
            int32 schoolMask = fields[18].GetInt32();
            int32 dummyId = fields[19].GetInt32();
            int32 dummyEffect = fields[20].GetInt32();
            int32 targetaura2 = fields[21].GetInt32();
            int32 aura2 = fields[22].GetInt32();
            int32 CreatureType = fields[23].GetInt32();
            int32 slot = fields[24].GetInt32();
            Tokenizer randToken(fields[25].GetString(), ' ');

            SpellInfo const* spellInfo = GetSpellInfo(abs(spell_id));
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell_id %i listed in `spell_trigger` does not exist", spell_id);
                WorldDatabase.PExecute("DELETE FROM `spell_trigger` WHERE spell_id = %i", spell_id);
                continue;
            }
            spellInfo = GetSpellInfo(abs(spell_trigger));
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell_trigger %i listed in `spell_trigger` does not exist", spell_trigger);
                WorldDatabase.PExecute("DELETE FROM `spell_trigger` WHERE spell_trigger = %i", spell_trigger);
                continue;
            }
            spellInfo = GetSpellInfo(abs(dummyId));
            if (dummyId && !spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "DummyId %i listed in `spell_trigger` does not exist", dummyId);
                continue;
            }

            SpellTriggered temptrigger;
            temptrigger.spell_id = spell_id;
            temptrigger.spell_trigger = spell_trigger;
            temptrigger.spell_cooldown = spell_cooldown;
            temptrigger.option = option;
            temptrigger.target = target;
            temptrigger.caster = caster;
            temptrigger.targetaura = targetaura;
            temptrigger.bp0 = bp0;
            temptrigger.bp1 = bp1;
            temptrigger.bp2 = bp2;
            temptrigger.effectmask = effectmask;
            temptrigger.aura = aura;
            temptrigger.chance = chance;
            temptrigger.group = group;
            temptrigger.procFlags = procFlags;
            temptrigger.procEx = procEx;
            temptrigger.check_spell_id = check_spell_id;
            temptrigger.addptype = addptype;
            temptrigger.schoolMask = schoolMask;
            temptrigger.dummyId = dummyId;
            temptrigger.dummyEffect = dummyEffect;
            temptrigger.targetaura2 = targetaura2;
            temptrigger.aura2 = aura2;
            temptrigger.CreatureType = CreatureType;
            temptrigger.slot = slot;
            for (Tokenizer::const_iterator itr = randToken.begin(); itr != randToken.end(); ++itr)
                temptrigger.randList.push_back(int32(atol(*itr)));

            mSpellTriggeredMap[spell_id].push_back(temptrigger);
            mSpellTriggeredVector[spell_id] = &mSpellTriggeredMap[spell_id];

            ++count;
        } while (result->NextRow());
    }


    //                                        0             1             2         3         4          5          6      7      8         9          10      11          12
    result = WorldDatabase.Query("SELECT `spell_id`, `spell_trigger`, `option`, `target`, `caster`, `targetaura`, `bp0`, `bp1`, `bp2`, `effectmask`, `aura`, `group`, `handlemask` FROM `spell_dummy_trigger`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            int32 spell_id = fields[0].GetInt32();
            int32 spell_trigger = fields[1].GetInt32();
            int32 option = fields[2].GetInt32();
            int32 target = fields[3].GetInt32();
            int32 caster = fields[4].GetInt32();
            int32 targetaura = fields[5].GetInt32();
            float bp0 = fields[6].GetFloat();
            float bp1 = fields[7].GetFloat();
            float bp2 = fields[8].GetFloat();
            int32 effectmask = fields[9].GetInt32();
            int32 aura = fields[10].GetInt32();
            int8 group = fields[11].GetInt8();
            int32 handlemask = fields[12].GetInt32();

            if (!GetSpellInfo(abs(spell_id)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_trigger_dummy` does not exist", abs(spell_id));
                //WorldDatabase.PExecute("DELETE FROM `spell_trigger_dummy` WHERE spell_id = %u", abs(spell_id));
                continue;
            }

            SpellDummyTrigger tempDummy;
            tempDummy.spell_id = spell_id;
            tempDummy.spell_trigger = spell_trigger;
            tempDummy.option = option;
            tempDummy.target = target;
            tempDummy.caster = caster;
            tempDummy.targetaura = targetaura;
            tempDummy.bp0 = bp0;
            tempDummy.bp1 = bp1;
            tempDummy.bp2 = bp2;
            tempDummy.effectmask = effectmask;
            tempDummy.aura = aura;
            tempDummy.group = group;
            tempDummy.handlemask = handlemask;
            mSpellDummyTriggerMap[spell_id].push_back(tempDummy);
            mSpellDummyTriggerVector[spell_id] = &mSpellDummyTriggerMap[spell_id];

            ++count;
        } while (result->NextRow());
    }

    //                                        0             1             2         3         4       5      6      7         8           9        10       11          12           13          14           15           16          17
    result = WorldDatabase.Query("SELECT `spell_id`, `spell_trigger`, `option`, `target`, `caster`, `bp0`, `bp1`, `bp2`, `effectmask`, `chance`, `slot`, `hastype`, `hastalent`, `hasparam`, `hastype2`, `hastalent2`, `hasparam2`, `option2` FROM `spell_aura_trigger`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            int32 spell_id = fields[0].GetInt32();
            int32 spell_trigger = fields[1].GetInt32();
            int32 option = fields[2].GetInt32();
            int32 target = fields[3].GetInt32();
            int32 caster = fields[4].GetInt32();
            float bp0 = fields[5].GetFloat();
            float bp1 = fields[6].GetFloat();
            float bp2 = fields[7].GetFloat();
            int32 effectmask = fields[8].GetInt32();
            int32 chance = fields[9].GetInt32();
            int32 slot = fields[10].GetInt32();

            if (!GetSpellInfo(abs(spell_id)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_aura_trigger` does not exist", abs(spell_id));
                //WorldDatabase.PExecute("DELETE FROM `spell_aura_trigger` WHERE spell_id = %u", abs(spell_id));
                continue;
            }

            AuraPeriodickTrigger tempAura;
            tempAura.spell_id = spell_id;
            tempAura.spell_trigger = spell_trigger;
            tempAura.option = option;
            tempAura.target = target;
            tempAura.caster = caster;
            tempAura.bp0 = bp0;
            tempAura.bp1 = bp1;
            tempAura.bp2 = bp2;
            tempAura.effectmask = effectmask;
            tempAura.chance = chance;
            tempAura.slot = slot;
            tempAura.hastype = fields[11].GetInt8();
            tempAura.hastalent = fields[12].GetInt32();
            tempAura.hasparam = fields[13].GetInt32();
            tempAura.hastype2 = fields[14].GetInt8();
            tempAura.hastalent2 = fields[15].GetInt32();
            tempAura.hasparam2 = fields[16].GetInt32();
            tempAura.option2 = fields[17].GetInt8();
            mSpellAuraTriggerMap[spell_id].push_back(tempAura);
            mSpellAuraTriggerVector[spell_id] = &mSpellAuraTriggerMap[spell_id];

            ++count;
        } while (result->NextRow());
    }

    //                                        0             1          2         3         4           5              6             7           8        9          10
    result = WorldDatabase.Query("SELECT `spellId`, `spellDummyId`, `option`, `target`, `caster`, `targetaura`, `effectmask`, `effectDummy`, `aura`, `removeAura`, `attr`,"
    //   11          12         13        14         15           16          17           18           19
    "`attrValue`, `custombp`, `type`, `hastype`, `hastalent`, `hasparam`, `hastype2`, `hastalent2`, `hasparam2` FROM `spell_aura_dummy`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            int32 spellId = fields[0].GetInt32();

            if (!GetSpellInfo(abs(spellId)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_aura_dummy` does not exist", spellId);
                WorldDatabase.PExecute("DELETE FROM `spell_aura_dummy` WHERE spellId = %i", spellId);
                continue;
            }

            SpellAuraDummy tempdummy;
            tempdummy.spellId = spellId;
            tempdummy.spellDummyId = fields[1].GetInt32();
            tempdummy.option = fields[2].GetInt32();
            tempdummy.target = fields[3].GetInt32();
            tempdummy.caster = fields[4].GetInt32();
            tempdummy.targetaura = fields[5].GetInt32();
            tempdummy.effectmask = fields[6].GetInt32();
            tempdummy.effectDummy = fields[7].GetInt32();
            tempdummy.aura = fields[8].GetInt32();
            tempdummy.removeAura = fields[9].GetInt32();
            tempdummy.attr = fields[10].GetInt32();
            tempdummy.attrValue = fields[11].GetInt32();
            tempdummy.custombp = fields[12].GetFloat();
            tempdummy.type = fields[13].GetInt32();
            tempdummy.hastype = fields[14].GetInt8();
            tempdummy.hastalent = fields[15].GetInt32();
            tempdummy.hasparam = fields[16].GetInt32();
            tempdummy.hastype2 = fields[17].GetInt8();
            tempdummy.hastalent2 = fields[18].GetInt32();
            tempdummy.hasparam2 = fields[19].GetInt32();

            mSpellAuraDummyMap[spellId].push_back(tempdummy);
            mSpellAuraDummyVector[spellId] = &mSpellAuraDummyMap[spellId];

            ++count;
        } while (result->NextRow());
    }

    //                                       0          1          2        3        4           5              6          7         8           9           10          11        12        13
    result = WorldDatabase.Query("SELECT `spellId`, `targetId`, `option`, `aura`, `chance`, `effectMask`, `resizeType`, `count`, `maxcount`, `addcount`, `addcaster`, `param1`, `param2`, `param3` FROM `spell_target_filter` ORDER BY `index`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            int32 spellId = fields[0].GetInt32();
            int32 targetId = fields[1].GetInt32();
            int32 option = fields[2].GetInt32();
            int32 aura = fields[3].GetInt32();
            int32 chance = fields[4].GetInt32();
            int32 effectMask = fields[5].GetInt32();
            int32 resizeType = fields[6].GetInt32();
            int32 count2 = fields[7].GetInt32();
            int32 maxcount = fields[8].GetInt32();
            int32 addcount = fields[9].GetInt32();
            int32 addcaster = fields[10].GetInt32();
            float param1 = fields[11].GetFloat();
            float param2 = fields[12].GetFloat();
            float param3 = fields[13].GetFloat();

            if (!GetSpellInfo(abs(spellId)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_target_filter` does not exist", spellId);
                WorldDatabase.PExecute("DELETE FROM `spell_target_filter` WHERE spellId = %i", spellId);
                continue;
            }

            SpellTargetFilter tempfilter;
            tempfilter.spellId = spellId;
            tempfilter.targetId = targetId;
            tempfilter.option = option;
            tempfilter.aura = aura;
            tempfilter.chance = chance;
            tempfilter.effectMask = effectMask;
            tempfilter.resizeType = resizeType;
            tempfilter.count = count2;
            tempfilter.maxcount = maxcount;
            tempfilter.addcount = addcount;
            tempfilter.addcaster = addcaster;
            tempfilter.param1 = param1;
            tempfilter.param2 = param2;
            tempfilter.param3 = param3;
            mSpellTargetFilterMap[spellId].push_back(tempfilter);
            mSpellTargetFilterVector[spellId] = &mSpellTargetFilterMap[spellId];

            ++count;
        } while (result->NextRow());
    }

    //                                       0        1         2             3            4         5           6           7            8            9           10        11        12
    result = WorldDatabase.Query("SELECT `spellId`, `type`, `errorId`, `customErrorId`, `caster`, `target`, `checkType`, `dataType`, `checkType2`, `dataType2`, `param1`, `param2`, `param3` FROM `spell_check_cast`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            int32 spellId = fields[0].GetInt32();

            if (!GetSpellInfo(abs(spellId)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_check_cast` does not exist", spellId);
                WorldDatabase.PExecute("DELETE FROM `spell_check_cast` WHERE spellId = %i", spellId);
                continue;
            }

            SpellCheckCast checkCast;
            checkCast.spellId = spellId;
            checkCast.type = fields[1].GetInt32();
            checkCast.errorId = fields[2].GetInt32();
            checkCast.customErrorId = fields[3].GetInt32();
            checkCast.caster = fields[4].GetInt32();
            checkCast.target = fields[5].GetInt32();
            checkCast.checkType = fields[6].GetInt32();
            checkCast.dataType = fields[7].GetInt32();
            checkCast.checkType2 = fields[8].GetInt32();
            checkCast.dataType2 = fields[9].GetInt32();
            checkCast.param1 = fields[10].GetInt32();
            checkCast.param2 = fields[11].GetInt32();
            checkCast.param3 = fields[12].GetInt32();
            mSpellCheckCastMap[spellId].push_back(checkCast);
            mSpellCheckCastVector[spellId] = &mSpellCheckCastMap[spellId];

            ++count;
        } while (result->NextRow());
    }

    //                                       0              1            2           3           4         5            6         7
    result = WorldDatabase.Query("SELECT `spell_id`, `spell_trigger`, `option`, `effectmask`, `target`, `caster`, `targetaura`, `aura` FROM `spell_trigger_delay`");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            int32 spellId = fields[0].GetInt32();

            if (!GetSpellInfo(abs(spellId)))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %i listed in `spell_trigger_delay` does not exist", spellId);
                WorldDatabase.PExecute("DELETE FROM `spell_trigger_delay` WHERE spellId = %i", spellId);
                continue;
            }

            SpellTriggerDelay triggerDelay;
            triggerDelay.spell_id = spellId;
            triggerDelay.spell_trigger = fields[1].GetInt32();
            triggerDelay.option = fields[2].GetInt32();
            triggerDelay.effectMask = fields[3].GetInt32();
            triggerDelay.target = fields[4].GetInt32();
            triggerDelay.caster = fields[5].GetInt32();
            triggerDelay.targetaura = fields[6].GetInt32();
            triggerDelay.aura = fields[7].GetInt32();
            mSpellTriggerDelayMap[spellId].push_back(triggerDelay);
            mSpellTriggerDelayVector[spellId] = &mSpellTriggerDelayMap[spellId];

            ++count;
        } while (result->NextRow());
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u triggered spell in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadPetLevelupSpellMap()
{
    uint32 oldMSTime = getMSTime();

    mPetLevelupSpellMap.clear();                                   // need for reload case

    uint32 count = 0;
    uint32 family_count = 0;

    for (CreatureFamilyEntry const* creatureFamily : sCreatureFamilyStore)
    {
        for (uint8 j = 0; j < 2; ++j)
        {
            if (!creatureFamily->SkillLine[j])
                continue;

            for (SkillLineAbilityEntry const* skillLine : sSkillLineAbilityStore)
            {
                //if (skillLine->skillId != creatureFamily->SkillLine[0] &&
                //    (!creatureFamily->SkillLine[1] || skillLine->skillId != creatureFamily->SkillLine[1]))
                //    continue;

                if (skillLine->SkillLine != creatureFamily->SkillLine[j])
                    continue;

                if (skillLine->AcquireMethod != SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN)
                    continue;

                SpellInfo const* spell = GetSpellInfo(skillLine->Spell);
                if (!spell) // not exist or triggered or talent
                    continue;

                PetLevelupSpellSet& spellSet = mPetLevelupSpellMap[creatureFamily->ID];
                if (spellSet.empty())
                    ++family_count;

                spellSet.insert(std::make_pair(spell->SpellLevel, spell->Id));
                ++count;
            }
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u pet levelup and default spells for %u families in %u ms", count, family_count, GetMSTimeDiffToNow(oldMSTime));
}

bool LoadPetDefaultSpells_helper(CreatureTemplate const* cInfo, PetDefaultSpellsEntry& petDefSpells)
{
    // skip empty list;
    bool have_spell = false;
    for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
    {
        if (petDefSpells.spellid[j])
        {
            have_spell = true;
            break;
        }
    }
    if (!have_spell)
        return false;

    // remove duplicates with levelupSpells if any
    if (PetLevelupSpellSet const* levelupSpells = cInfo->Family ? sSpellMgr->GetPetLevelupSpellList(cInfo->Family) : nullptr)
    {
        for (auto& spellId : petDefSpells.spellid)
        {
            if (!spellId)
                continue;

            for (const auto& levelupSpell : *levelupSpells)
            {
                if (levelupSpell.second == spellId)
                {
                    spellId = 0;
                    break;
                }
            }
        }
    }

    // skip empty list;
    have_spell = false;
    for (auto spellId : petDefSpells.spellid)
    {
        if (spellId)
        {
            have_spell = true;
            break;
        }
    }

    return have_spell;
}

void SpellMgr::LoadPetDefaultSpells()
{
    uint32 oldMSTime = getMSTime();

    mPetDefaultSpellsMap.clear();

    uint32 countCreature = 0;

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading summonable creature templates...");
    oldMSTime = getMSTime();

    // different summon spells
    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        SpellInfo const* spellEntry = GetSpellInfo(i);
        if (!spellEntry)
            continue;

        for (uint8 k = 0; k < MAX_SPELL_EFFECTS; ++k)
        {
            if (spellEntry->EffectMask < uint32(1 << k))
                break;

            if (spellEntry->Effects[k]->Effect == SPELL_EFFECT_SUMMON || spellEntry->Effects[k]->Effect == SPELL_EFFECT_SUMMON_PET)
            {
                CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(spellEntry->Effects[k]->MiscValue);
                if (!cInfo)
                    continue;

                // already loaded
                if (cInfo->PetSpellDataId)
                    continue;

                if (mPetDefaultSpellsMap.find(cInfo->Entry) != mPetDefaultSpellsMap.end())
                    continue;

                PetDefaultSpellsEntry petDefSpells;
                for (uint8 j = 0; j < MAX_CREATURE_SPELL_DATA_SLOT; ++j)
                    petDefSpells.spellid[j] = cInfo->spells[j];

                if (LoadPetDefaultSpells_helper(cInfo, petDefSpells))
                {
                    mPetDefaultSpellsMap[cInfo->Entry] = petDefSpells;
                    ++countCreature;
                }
            }
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u summonable creature templates in %u ms", countCreature, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellAreas()
{
    uint32 oldMSTime = getMSTime();

    mSpellAreaMap.clear();                                  // need for reload case
    mSpellAreaForQuestMap.clear();
    mSpellAreaForActiveQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    //                                                  0     1         2              3               4                 5          6          7       8         9         10         11
    QueryResult result = WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_status, quest_end_status, quest_end, aura_spell, racemask, gender, autocast, classmask, active_event FROM spell_area");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell area requirements. DB table `spell_area` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId = spell;
        spellArea.areaId = fields[1].GetInt32();
        spellArea.questStart = fields[2].GetUInt32();
        spellArea.questStartStatus = fields[3].GetUInt32();
        spellArea.questEndStatus = fields[4].GetUInt32();
        spellArea.questEnd = fields[5].GetUInt32();
        spellArea.auraSpell = fields[6].GetInt32();
        spellArea.raceMask = fields[7].GetUInt32();
        spellArea.gender = Gender(fields[8].GetUInt8());
        spellArea.autocast = fields[9].GetBool();
        spellArea.classmask = fields[10].GetInt32();
        spellArea.active_event = fields[11].GetUInt32();

        spellArea.spellInfo = GetSpellInfo(spell);
        if (spellArea.spellInfo)
        {
            if (spellArea.autocast)
                const_cast<SpellInfo*>(spellArea.spellInfo)->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_CANT_CANCEL;
        }
        else
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` does not exist", spell);
            continue;
        }

        spellArea.RequiredAreasID = spellArea.spellInfo->CastingReq.RequiredAreasID;

        bool ok = true;
        SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
        for (auto itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
        {
            if (spellArea.spellId != itr->second.spellId)
                continue;
            if (spellArea.areaId != itr->second.areaId)
                continue;
            if (spellArea.questStart != itr->second.questStart)
                continue;
            if (spellArea.auraSpell != itr->second.auraSpell)
                continue;
            if ((spellArea.raceMask & itr->second.raceMask) == 0)
                continue;
            if (spellArea.gender != itr->second.gender)
                continue;

            // duplicate by requirements
            ok = false;
            break;
        }

        if (!ok)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` already listed with similar requirements.", spell);
            continue;
        }

        if (spellArea.areaId > 0 && !sAreaTableStore.LookupEntry(spellArea.areaId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong area (%u) requirement", spell, spellArea.areaId);
            continue;
        }

        if (spellArea.areaId < 0 && !sMapStore.LookupEntry(abs(spellArea.areaId)))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong mapid (%u) requirement", spell, abs(spellArea.areaId));
            continue;
        }

        if (spellArea.questStart && !sQuestDataStore->GetQuestTemplate(spellArea.questStart))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong start quest (%u) requirement", spell, spellArea.questStart);
            continue;
        }

        if (spellArea.questEnd)
        {
            if (!sQuestDataStore->GetQuestTemplate(spellArea.questEnd))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong end quest (%u) requirement", spell, spellArea.questEnd);
                continue;
            }
        }

        if (spellArea.auraSpell)
        {
            SpellInfo const* spellInfo = GetSpellInfo(abs(spellArea.auraSpell));
            if (!spellInfo)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong aura spell (%u) requirement", spell, abs(spellArea.auraSpell));
                continue;
            }

            if (uint32(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have aura spell (%u) requirement for itself", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for (SpellAreaForAuraMap::const_iterator itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for (SpellAreaMap::const_iterator itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }
            }
        }

        if (spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE) == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong race mask (%u) requirement", spell, spellArea.raceMask);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_FEMALE && spellArea.gender != GENDER_MALE)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Spell %u listed in `spell_area` have wrong gender (%u) requirement", spell, spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(std::make_pair(spell, spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if (spellArea.areaId)
            mSpellAreaForAreaMap.insert(std::make_pair(spellArea.areaId, sa));

        // for search by current zone/subzone at zone/subzone change
        if (!spellArea.areaId && spellArea.RequiredAreasID > 0)
        {
            spellArea.areaGroupMembers = sDB2Manager.GetAreasForGroup(spellArea.RequiredAreasID);
            for (uint32 areaId : spellArea.areaGroupMembers)
                mSpellAreaForAreaMap.insert(std::make_pair(areaId, sa));
        }

        // for search at quest start/reward
        if (spellArea.questStart)
            mSpellAreaForQuestMap.insert(std::make_pair(spellArea.questStart, sa));

        // for search at quest start/reward
        if (spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(std::make_pair(spellArea.questEnd, sa));

        // for search at aura apply
        if (spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(std::make_pair(abs(spellArea.auraSpell), sa));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell area requirements in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadSpellInfoStore()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading SpellInfo store...");

    uint32 oldMSTime = getMSTime();

    std::unordered_map<uint32, SpellVisualMap> visualsBySpell;

    spellCountInWorld.resize(sSpellStore.GetNumRows() + 1, 0);

    UnloadSpellInfoStore();
    mSpellInfoMap.resize(sSpellStore.GetNumRows(), nullptr);
    std::unordered_map<uint32, SpellInfoLoadHelper> loadData;

    for (SpellAuraOptionsEntry const* auraOptions : sSpellAuraOptionsStore)
        loadData[auraOptions->SpellID].AuraOptions.insert(auraOptions);

    for (SpellAuraRestrictionsEntry const* auraRestrictions : sSpellAuraRestrictionsStore)
        if (!auraRestrictions->DifficultyID)    // TODO: implement
            loadData[auraRestrictions->SpellID].AuraRestrictions = auraRestrictions;

    for (SpellCastingRequirementsEntry const* castingRequirements : sSpellCastingRequirementsStore)
        loadData[castingRequirements->SpellID].CastingRequirements = castingRequirements;

    for (SpellCategoriesEntry const* categories : sSpellCategoriesStore)
        if (!categories->DifficultyID)  // TODO: implement
            loadData[categories->SpellID].Categories = categories;

    for (SpellClassOptionsEntry const* classOptions : sSpellClassOptionsStore)
        loadData[classOptions->SpellID].ClassOptions = classOptions;

    for (SpellCooldownsEntry const* cooldowns : sSpellCooldownsStore)
        if (!cooldowns->DifficultyID)   // TODO: implement
            loadData[cooldowns->SpellID].Cooldowns = cooldowns;

    for (SpellEquippedItemsEntry const* equippedItems : sSpellEquippedItemsStore)
        loadData[equippedItems->SpellID].EquippedItems = equippedItems;

    for (SpellInterruptsEntry const* interrupts : sSpellInterruptsStore)
        if (!interrupts->DifficultyID)  // TODO: implement
            loadData[interrupts->SpellID].Interrupts = interrupts;

    for (SpellLevelsEntry const* levels : sSpellLevelsStore)
        if (!levels->DifficultyID)  // TODO: implement
            loadData[levels->SpellID].Levels = levels;

    for (SpellReagentsEntry const* reagents : sSpellReagentsStore)
        loadData[reagents->SpellID].Reagents = reagents;

    for (SpellReagentsCurrencyEntry const* ReagentsCurrency : sSpellReagentsCurrencyStore)
        loadData[ReagentsCurrency->SpellID].ReagentsCurrency = ReagentsCurrency;

    for (SpellScalingEntry const* scaling : sSpellScalingStore)
        loadData[scaling->SpellID].Scaling = scaling;

    for (SpellShapeshiftEntry const* shapeshift : sSpellShapeshiftStore)
        loadData[shapeshift->SpellID].Shapeshift = shapeshift;

    for (SpellTotemsEntry const* totems : sSpellTotemsStore)
        loadData[totems->SpellID].Totems = totems;

    for (SpellMiscEntry const* misc : sSpellMiscStore)
        loadData[misc->SpellID].Misc.insert(misc);

    for (SpellXSpellVisualEntry const* visual : sSpellXSpellVisualStore)
    {
        if (visualsBySpell[visual->SpellID].size() < MAX_DIFFICULTY)
            visualsBySpell[visual->SpellID].resize(MAX_DIFFICULTY);

        visualsBySpell[visual->SpellID][visual->DifficultyID].push_back(visual);
    }

    for (SpellEntry const* spellEntry : sSpellStore)
        mSpellInfoMap[spellEntry->ID] = new SpellInfo(loadData[spellEntry->ID], spellEntry, &visualsBySpell[spellEntry->ID]);

    for (SpellPowerEntry const* spellPower : sSpellPowerStore)
    {
        if (spellPower->SpellID >= GetSpellInfoStoreSize())
            continue;

        SpellInfo* spell = mSpellInfoMap[spellPower->SpellID];
        if (!spell)
            continue;

        spell->Power.PowerType = spellPower->PowerType;
        spell->Power.PowerCost = spellPower->ManaCost;
        spell->Power.PowerCostPercentage = spellPower->PowerCostPct;
        spell->Power.PowerCostPerSecond = spellPower->ManaPerSecond;
        spell->Power.PowerCostPercentagePerSecond = spellPower->PowerPctPerSecond;
        spell->Power.RequiredAura = spellPower->RequiredAuraSpellID;
        spell->Power.HealthCostPercentage = spellPower->PowerCostMaxPct;

        if (!spell->AddPowerData(spellPower))
            TC_LOG_INFO(LOG_FILTER_WORLDSERVER, "Spell - %u has more powers > 4.", spell->Id);
    }

    for (TalentEntry const* talentInfo : sTalentStore)
        if (talentInfo->SpellID < GetSpellInfoStoreSize())
            if (SpellInfo* spellEntry = mSpellInfoMap[talentInfo->SpellID])
                spellEntry->talentId = talentInfo->ID;

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded SpellInfo store in %u seconds", GetMSTimeDiffToNow(oldMSTime) / 1000);
}

void SpellMgr::UnloadSpellInfoStore()
{
    for (size_t i = 0; i < mSpellInfoMap.size(); ++i)
        if (mSpellInfoMap[i])
            delete mSpellInfoMap[i];

    mSpellInfoMap.clear();
}

void SpellMgr::UnloadSpellInfoImplicitTargetConditionLists()
{
    for (size_t i = 0; i < mSpellInfoMap.size(); ++i)
        if (mSpellInfoMap[i])
            mSpellInfoMap[i]->_UnloadImplicitTargetConditionLists();
}

inline void ApplySpellFix(std::initializer_list<uint32> spellIds, void(*fix)(SpellInfo*))
{
    for (auto spellID : spellIds)
    {
        auto spellInfo = sSpellMgr->GetSpellInfo(spellID);
        if (!spellInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Spell info correction specified for non-existing spell %u", spellID);
            continue;
        }

        fix(const_cast<SpellInfo*>(spellInfo));
    }
}

void SpellMgr::LoadSpellCustomAttr()
{
    uint32 oldMSTime = getMSTime();

    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        auto const& spellInfo = mSpellInfoMap[i];
        if (!spellInfo)
            continue;

        if (spellInfo->GetAuraOptions()->CumulativeAura > 1)
            spellInfo->AttributesCu[1] |= SPELL_ATTR1_CU_IS_USING_STACKS;

        for (auto effect : spellInfo->Effects)
            if (effect->ApplyAuraName == SPELL_AURA_MOD_DAMAGE_PERCENT_DONE || effect->ApplyAuraName == SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN)
                if (effect->MiscValue == 0) // if zero, then we can use this effect for all masks
                    effect->MiscValue = 127;
    }

    // Cheat Death
    ApplySpellFix({45182}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
    });

    // Chain Lightning
    ApplySpellFix({45297}, [](SpellInfo* spellInfo)
    {
        spellInfo->MaxLevel = 0;
        spellInfo->ClassOptions.SpellClassMask[0] = 0x2;
    });

    // Lava Beam
    ApplySpellFix({114074, 114738}, [](SpellInfo* spellInfo)
    {
        spellInfo->ClassOptions.SpellClassMask[0] = 0x2;
    });

    // Kill Command
    ApplySpellFix({83381}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK;
    });

    ApplySpellFix({154932, 156743, 188189}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_DEBUFF;
    });

    // Sha Cloud
    ApplySpellFix({145591}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_CANT_USED_IN_COMBAT;
        spellInfo->GetMisc()->MiscData.Attributes[4] &= ~SPELL_ATTR4_TRIGGERED;
    });

    // Spinning Crane Kick
    ApplySpellFix({101546}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] &= ~SPELL_ATTR0_ABILITY;
    });

    // Touch of Karma
    ApplySpellFix({122470}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] &= ~SPELL_ATTR6_NOT_LIMIT_ABSORB;
    });

    ApplySpellFix({
        300000, // custom
        300001, // custom
        137594, // Fortitude Trigger
        137592, // Haste Trigger
        137595 // Lightning Strike Charges Trigger
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[4] |= SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG;
    });

    ApplySpellFix({146025, // Readiness (Prot)
        145955, // Readiness (DD Plate)
        146019, // Readiness (Other DD)
        190900  // Healing Surge (power bonus)
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_DEATH_PERSISTENT;
    });

    // Disable (Root)
    ApplySpellFix({116706}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetAuraOptions()->CumulativeAura = 0;
        spellInfo->GetAuraOptions()->ProcTypeMask = 0;
    });

    // Vindicator
    ApplySpellFix({200373}, [](SpellInfo* spellInfo)
    {
        spellInfo->ClassOptions.SpellClassMask[2] = 0;
    });

    ApplySpellFix({89733}, [](SpellInfo* spellInfo) // Path of Illidan
    {
        spellInfo->AuraInterruptFlags[0] = 0;
    });

    ApplySpellFix({121471, 239932}, [](SpellInfo* spellInfo) // Shadow Blades, Felclaws
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_OVERRIDE_AUTOATTACK;
    });

    ApplySpellFix({ 234458 }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] = AURA_INTERRUPT_FLAG_MOUNT;
    });

    ApplySpellFix({247938}, [](SpellInfo* spellInfo) // Chaos Blades
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_OVERRIDE_AUTOATTACK;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 211796;
        spellInfo->Effects[EFFECT_0]->MiscValue = 211797;
    });

    ApplySpellFix({
        234180,
        241511,
        212753, // Corpse Shield
        148008, // Essence of Yu'lon
        252750  // Bloody Gash
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] &= ~SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS;
    });

    ApplySpellFix({
        148022, // Icicle
        227869, // Six-Feather Fan
        51640,  // Taunt Flag Targeting
        205532, // Volcanic Inferno
        211729, // Thal'kiel's Discord
        107223  // Sunfire Rays
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
    });

    // Talon Strike
    ApplySpellFix({203560}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_TARGET_ENEMY;
    });

    ApplySpellFix({178153}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 5;
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
    });

    ApplySpellFix({178236}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 0;
        spellInfo->Effects[EFFECT_0]->MiscValueB = 125;
    });

    ApplySpellFix({ 134522,  222256 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 0;
    });

    // Defensive Spikes
    ApplySpellFix({212871}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_PARRY_PERCENT;
    });

    ApplySpellFix({
        130121, // Item - Scotty's Lucky Coin
        198509, // Stance of the Mountain
        120361, // Barrage
        157644, // Enhanced Pyrotechnics
        115399, // Black Ox Brew
        251952  // Hammer-Forged
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[4] &= ~SPELL_ATTR4_TRIGGERED;
    });

    // Marked for Death
    ApplySpellFix({137619}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_DEATH_PERSISTENT;
    });
   
    ApplySpellFix({
        45181,  // Cheated Death
        87024,  // Cauterized
        123981, // Perdition
        209261  // Uncontained Fel
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_DEATH_PERSISTENT;
    });

    // Avoidance
    ApplySpellFix({146343}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = SPELL_SCHOOL_MASK_ALL;
    });

    // Ocean's Embrace
    ApplySpellFix({242459, 242460, 242461, 242462, 242463, 242464, 242465, 242467}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_PROC_ONLY_ON_CAST;
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 7.20225f;
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_CAN_PROC_WITH_TRIGGERED;
    });

    // Collapsing Shadow
    ApplySpellFix({215476}, [](SpellInfo* spellInfo)
    {
        for (int i = 0; i < 3; ++i)
        {
            spellInfo->Effects[i]->Scaling.Coefficient = 1.25f;
        }
    });

    // Frost Enchant
    ApplySpellFix({225729}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 0.f;
    });

    // Fetid Regurgitation
    ApplySpellFix({224437}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 2.85361f;
    });

    // Feedback Loop
    ApplySpellFix({253268}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 0.356442f;
    });
    
    // Volatile Magic
    ApplySpellFix({215884}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 8.2356f;
    });

    // Volatile Energy
    ApplySpellFix({230242}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 27.4534f;
    });

    // Tormenting Cyclone
    ApplySpellFix({221865}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 4.706f;
    });

    // Loose Mana
    ApplySpellFix({230144}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 9.51295f;
    });

    // Nightmarish Ichor
    ApplySpellFix({222027}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 1.37835f;
    });

    // Infested Ground
    ApplySpellFix({221804}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 6.69951f;
    });

    // Leeching Pestilence
    ApplySpellFix({221805}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 0.624573f;
    });

    // Shadow Wave
    ApplySpellFix({215047}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 46.2287f;
    });

    // Gaseous Bubble
    ApplySpellFix({214972}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 55.8313f;
    });

    // Fragile Echo
    ApplySpellFix({215270}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Scaling.Coefficient = 1.99786f;
    });

    // Spectral Owl
    ApplySpellFix({242570}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->Scaling.Coefficient = 18.3234f;
    });

    // Light's Reckoning 
    ApplySpellFix({256896}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] &= ~SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_DONE_BONUS;
        spellInfo->Effects[EFFECT_0]->BasePoints = spellInfo->Effects[EFFECT_1]->BasePoints;
    });

    // Defensive Stance (visual eff)
    ApplySpellFix({147925}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[8] |= SPELL_ATTR8_AURA_SEND_AMOUNT;
    });

    // Prayer of Mending
    ApplySpellFix({155793}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] &= ~SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
    });

    // Retribution 
    ApplySpellFix({183435}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
        spellInfo->Effects[EFFECT_0]->MaxRadiusEntry = nullptr;
    });

    ApplySpellFix({132464, // Chi Wave (Pos)
        121093 // Monk - Gift of the Naaru
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->ClassOptions.SpellClassSet = SPELLFAMILY_MONK;
    });

    ApplySpellFix({116000,
        197637, // Stellar Empowerment
        197209, // Lightning Rod
        188089, // Earthen Spike
        253593, // Inexorable Assault
        103785, // Black Blood of the Earth dmg
        194384, // Atonement
        214206, // Atonement (Honor Talent)
        164815, // Sunfire (Solar)
        206491, // Nemesis
        185365, // Hunter's Mark
        205546, // Odyn's Fury
        236641, // Aimed Shot
        210320, // Devotion Aura
        53563,  // Beacon of Light
        156910, // Beacon of Faith
        200025, // Beacon of Virtue
        196414, // Eradication
        106830, // Thrash
        158790, // Thrash (Marker Aura)
        80240,  // Havoc
        200548, // Bane of Havoc (Honor Talent)
        197548, // Strength of Soul
        195403, // Gale Burst
        122470  // Touch of Karma
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_STACK_FOR_DIFF_CASTERS;
    });

    ApplySpellFix({
        108366, // Soul Leech
        143597, // Generate rage energize
        114714, // Grilled Plainshawk Leg
        236065  // The Desolate Host: Tormented Cries
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CASTER;
    });

    ApplySpellFix({6203,  // Soulstone
        2641 // Dismiss Pet
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_DEAD;
    });

    // Zen Flight
    ApplySpellFix({125883}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_MELEE_ATTACK;
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_DIRECT_DAMAGE;
    });

    //Affix: Bolster
    ApplySpellFix({209859, 241593, 221299 }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_LEAVE_COMBAT;
    });

    // Atonement
    ApplySpellFix({81751}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] &= ~SPELL_ATTR2_CANT_CRIT;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_INITIAL_AGGRO;
    });

    ApplySpellFix({
        192434, // From the Shadows
        194238, // Sphere of Insanity
        127802, // Touch of the Grave
        117899, // Soothing Mist (Visual)
        113092, // Frost Bomb
        253593, // Inexorable Assault
        18153,  // Kodo Kombobulator
        145110, // Ysera's Gift
        196734, // Special Delivery
        44461,  // Living Bomb
        76194
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_INITIAL_AGGRO;
    });

    ApplySpellFix({
        127802, // Touch of the Grave
        145718  // Gusting Bomb
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Speed = 25.f;
    });

    ApplySpellFix({
        36032,  // Arcane Charge
        49576,  // Death Grip
        108446, // Soul Link
        45470,  // Death Strike
        1329,   // Mutilate
        1966,   // Feint
        145109, // Ysera's Gift
        145110, // Ysera's Gift
        205224  // Consumption
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_CANT_TRIGGER_PROC;
    });

    ApplySpellFix({21987, // Lash of Pain
        58563 // Assassinate Restless Lookout
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET;
    });

    ApplySpellFix({127424}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENEMY_54;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    // Gouge // Head Smack
    ApplySpellFix({1776, 12540, 13579, 24698, 28456, 29425, 34940, 36862, 38863, 52743}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REQ_TARGET_FACING_CASTER;
    });

    // Fists of Fury, Fury of the Eagle
    ApplySpellFix({117418, 120086, 203413}, [](SpellInfo* spellInfo)
    {
        spellInfo->TargetRestrictions.ConeAngle = M_PI;
    });

    // Lay of Hands, Greater Blessing of Kings, Greater Blessing of Wisdom, Death from Above, Soul Rip, Stormlash, Stormlash (Honor Talent), Mark of Shifting
    ApplySpellFix({633, 203538, 203539, 152150, 220893, 195256, 213307, 224392}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_CANT_TRIGGER_PROC;
    });

    // Void Shift
    ApplySpellFix({118594}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_CANT_TRIGGER_PROC;
    });

    // Spanning Singularity
    ApplySpellFix({209168}, [](SpellInfo* spellInfo)
    {
        spellInfo->TargetRestrictions.MaxAffectedTargets = 0;
    });

    // Shado-Pan Dragon Gun
    ApplySpellFix({120751, 120876, 120964, 124347}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENEMY_54;
    });

    ApplySpellFix({37433, // Spout
        43140, // Flame Breath
        43215, // Flame Breath
        70461, // Coldflame Trap
        72133 // Pain and Suffering
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CONE_LINE;
    });

    ApplySpellFix({
        239058, // Touch of Sargeras
        239742, // Dark Mark
        235569, // Hammer of Creation
        235573, // Hammer of Obliteration
        206340, // Bounds of Fel
        206370,  // Bounds of Fel
        231854, // Unchecked Rage
        24340, // Meteor
        26558, // Meteor
        28884, // Meteor
        36837, // Meteor
        38903, // Meteor
        41276, // Meteor
        57467, // Meteor
        26789, // Shard of the Fallen Star
        31436, // Malevolent Cleave
        35181, // Dive Bomb
        40810, // Saber Lash
        43267, // Saber Lash
        43268, // Saber Lash
        42384, // Brutal Swipe
        45150, // Meteor Slash
        64688, // Sonic Screech
        72373, // Shared Suffering
        71904, // Chaos Bane
        70492, // Ooze Eruption
        145944, // Sha Smash
        106375, // Unstable Twilight
        107439, // Twilight Barrage
        106401, // Twilight Onslaught
        103414, // Stomp
        136216, // Caustic Gas
        114083, // Ascendance
        135703, // Static shock tr ef dmg
        98474, // Flame Scythe
        81280, // Blood Burst
        142890, //Blood Rage Dmg
        143962, //Inferno Strike
        157503, //Cloudburst
        155152, //Prismatic Crystal
        153564, //Meteor
        197969, //Roaring Cacophony
        184689, //Shield of Vengeance
        191685, //Virulent Eruption
        206369, //Corruption Meteor
        228852, //Shared Suffering
        232044, //Colossal Slam
        217070, //Rage of the Illidari
        235188, //Archimonde's Hatred Reborn
        235967, //Velen's Future Sight
        212494, //Aluriel: Annihilated
        242525, //Terror From Below
        246464, //Dread Torrent
        253278, //Felshield
        257286, //Ravaging Storm
        253282, //Tarratus Keystone
        230403, //Aluriel: Fel Lash
        246706, //Kingaroth: Demolish
        244583, //Felhounds: Siphoned
        196512  //Claw Frenzy
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_SHARE_DAMAGE;
    });

    ApplySpellFix({18500, // Wing Buffet
        33086, // Wild Bite
        49749, // Piercing Blow
        52890, // Penetrating Strike
        53454, // Impale
        59446, // Impale
        62383, // Shatter
        64777, // Machine Gun
        65239, // Machine Gun
        65919, // Impale
        74439, // Machine Gun
        63278, // Mark of the Faceless (General Vezax)
        62544, // Thrust (Argent Tournament)
        62709, // Counterattack! (Argent Tournament)
        62626, // Break-Shield (Argent Tournament, Player)
        64590, // Break-Shield (Argent Tournament, Player)
        64342, // Break-Shield (Argent Tournament, NPC)
        64686, // Break-Shield (Argent Tournament, NPC)
        65147, // Break-Shield (Argent Tournament, NPC)
        68504, // Break-Shield (Argent Tournament, NPC)
        62874, // Charge (Argent Tournament, Player)
        68498, // Charge (Argent Tournament, Player)
        64591, // Charge (Argent Tournament, Player)
        63003, // Charge (Argent Tournament, NPC)
        63010, // Charge (Argent Tournament, NPC)
        68321, // Charge (Argent Tournament, NPC)
        72255, // Mark of the Fallen Champion (Deathbringer Saurfang)
        118000, // Dragon Roar
        107029, // Impale Aspect
        106548, // Agonizing Pain
        114089, // Windlash
        114093, // Windlash
        115357, // Windstrike
        115360, // Windstrike
        206656, // Xavius, Nightmare Blades
        229980, // Touch of Death
        22482   // Blade Flurry
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_IGNORE_ARMOR;
    });

    // Sonic Screech (Auriaya) // Unseen Strike // Massive Attacks
    ApplySpellFix({64422,
        122994,
        117921}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_SHARE_DAMAGE;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_IGNORE_ARMOR;
    });

    // Mark of the Fallen Champion (Deathbringer Saurfang)
    ApplySpellFix({72293}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF0;
    });

    // Snowman
    ApplySpellFix({21847}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF0;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF1;
    });

    // Ring of Frost
    ApplySpellFix({82691}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] |= SPELL_ATTR1_CANT_BE_REFLECTED;
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_SINGLE_TARGET_SPELL;
    });

    // Fear Effect
    ApplySpellFix({118699}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_IGNORE_HIT_RESULT;
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_FEAR;
    });

    // Nature's Vigil (Damage)
    ApplySpellFix({124991}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_DONE_BONUS;
        spellInfo->GetMisc()->MiscData.Attributes[6] &= ~SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    // Nature's Vigil (Heal)
    ApplySpellFix({124988}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ALLY;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    // Healing Tide
    ApplySpellFix({114942}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CANT_TARGET_SELF;
        spellInfo->ClassOptions.SpellClassMask[0] = 0x2000;
    });

    // Earthgrab
    ApplySpellFix({116943}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    // Raise Ally
    ApplySpellFix({61999}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_TARGET_ALLY;
    });

    // Rapid Innervation
    ApplySpellFix({202842}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ALLY;
    });

    ApplySpellFix({122802,
        122804,
        122806,
        122807,
        122809,
        122811,
        126213,
        126214,
        126215,
        126216,
        132764}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_DEST_TARGET_RANDOM;
    });

    ApplySpellFix({106853}, [](SpellInfo* spellInfo) // Fists of Fury
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    ApplySpellFix({86674}, [](SpellInfo* spellInfo) // Ancient Healer
    {
        spellInfo->GetAuraOptions()->ProcCharges = 5;
    });

    ApplySpellFix({86657}, [](SpellInfo* spellInfo) // Ancient Guardian
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ANY;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
    });

    ApplySpellFix({45204}, [](SpellInfo* spellInfo) // Mirror Image - Clone Me!
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] |= SPELL_ATTR6_CAN_TARGET_INVISIBLE;
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
    });

    ApplySpellFix({41055,  // Copy Weapon Spells
        45206,
        63416,
        69891,
        69892}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
        spellInfo->Categories.Mechanic = 0;
    });

    // Capturing - interrupt
    ApplySpellFix({156098, 234016, 240675}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CHANNELED_1;
    });

    // Subterfuge
    ApplySpellFix({108208}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->Effects[EFFECT_0]->MiscValue = 0;
    });

    ApplySpellFix({ 247586 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 87;
        spellInfo->Effects[EFFECT_0]->TargetA = 0;
    });

    ApplySpellFix({137573,  // Burst of Speed (IMMUNITY)
        1160,    // Demoralizing Shout
        1966,    // Feint
        185311,  // Crimson Vial
        1784
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] |= SPELL_ATTR1_NOT_BREAK_STEALTH;
    });

    ApplySpellFix({
        79808,  // Arcane Missiles
        170571, // Arcane Missiles
        170572, // Arcane Missiles
        197915, // Lifecycles
        246153, // Precision
        136050  // Malformed Blood
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetAuraOptions()->ProcCharges = 0;
    });

    ApplySpellFix({
        6770,   // Sap
        16870,  // Clearcasting
        143333, // Water Strider Water Walking
        48108,  // Hot Streak
        124430, // Divine Insight (Shadow)
        144569, // Bastion of Power
        144871, // Sage Mender
        157228, // Empowered Moonkin
        206333, // Taste for Blood
        211160, // Natural Defenses
        236446, // Butcher's Bone Apron
        236746, // Control of Lava
        242286, // Crashing Lightning
        208913, // Sentinel's Sight
        235712, // Gyroscopic Stabilization
        248195, // Precise Strikes
        248625, // Shattered Defenses
        245640, // Shuriken Combo
        242952, // Bloody Rage
        242953, // Bloody Rage
        253383, // Weighted Blade
        253806, // Sacred Judgment
        252285, // Sharpened Sabers
        257945, // Shadow Gestures
        246261, // Ignition
        242251, // Critical Massive
        252094, // Exposed Flank
        252095, // In for the Kill
        246519, // Penitent
        247226, // Empty Mind
        253257, // Arctic Blast
        253437, // Answered Prayers
        253443, // Everlasting Hope
        225947, // Stone Heart
        208764, // Nobundo's Redemption
        210607, // Jonat's Focus
        211440, // Al'maiesh, the Cord of Hope
        211442, // Al'maiesh, the Cord of Hope
        211443, // Al'maiesh, the Cord of Hope
        207844, // Kakushan's Stormscale Gauntlets
        252141, // Earthen Strength
        194329  // Pyretic Incantation
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetAuraOptions()->ProcCharges = 1;
    });

    // Sustained Sanity
    ApplySpellFix({219772}, [](SpellInfo* spellInfo)
    {
        spellInfo->ClassOptions.SpellClassSet = SPELLFAMILY_PRIEST;
    });
    ApplySpellFix({6358,   // Seduce (succubus)
        115268 // Mesmerize (succubus)
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->ClassOptions.SpellClassSet = SPELLFAMILY_WARLOCK;
    });

    ApplySpellFix({
        210027,  // Share in the Light
        196917,  // Light of the Martyr
        145109,  // Ysera's Gift
        145110,  // Ysera's Gift
        114908,  // Spirit Shell
        77535,   // Mastery, Blood Shield
        195318,  // Good Karma
        146347,  // Life Steal
        143924,  // Life Steal
        193320,  // Umbilicus Eternus
        235551   // Spirit of the Darkness Flame
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_DONE_BONUS;
    });

    // Mark of Shifting
    ApplySpellFix({224392}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_HEAL;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_DONE_BONUS;
    });

    ApplySpellFix({
        205022, // Arcane Familiar
        138334, // Fatal strike
        124845  // Calamity
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = 0;
    });

    // Fortifying Brew
    ApplySpellFix({120954}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT;
        spellInfo->Effects[EFFECT_0]->BasePoints = 20;
    });

    // Anti-Magic Barrier
    ApplySpellFix({205725}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = spellInfo->Effects[EFFECT_1]->BasePoints;
    });

    ApplySpellFix({129428,  //Stone Guards - Dummy Searcher(cobalt mine)
        154462 //Ritual of Bones
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(29);
    });

    ApplySpellFix({106736,
        106113}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_UNIT_TARGET_ENEMY;
    });

    ApplySpellFix({119922,  //Shockwave
        119929,
        119930,
        119931,
        119932,
        119933}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Speed = 5.0f;
    });

    ApplySpellFix({106112}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(22); //200yards.
    });

    ApplySpellFix({106847,
        106588,  // Expose Weakness
        106600,
        106613,
        106624,
        104855,  // Overpacked Firework
        145212 //Unleashed Anger dmg
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ANY;
    });

    ApplySpellFix({106334,
        99508,  //Bloated Frog
        187356, //Mystic Image
        207228, //Warp Nightwell 
        228335, //Warp Nightwell
        219823, //Power Overwhelming
        232902, //Sasszine: Befouling Ink
        74634
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_ONLY_TARGET_PLAYERS;
    });

    ApplySpellFix({120552}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(16);
    });

    ApplySpellFix({119684}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENEMY_24;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_CONE_ENEMY_24;
        spellInfo->Effects[EFFECT_1]->TargetB = 0;
    });

    ApplySpellFix({112060}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    ApplySpellFix({227463}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetB = 0;
    });

    ApplySpellFix({ 75664, 82850 }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_USABLE_WHILE_MOVING;
    });

    ApplySpellFix({225484}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_NONE;
        spellInfo->Effects[EFFECT_1]->TargetB = TARGET_NONE;
    });

    ApplySpellFix({118685}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(5);
    });

    ApplySpellFix({60670}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
        spellInfo->Effects[EFFECT_2]->TriggerSpell = 0;
    });

    ApplySpellFix({114746}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->TargetA = TARGET_UNIT_TARGET_ALLY;
        spellInfo->Effects[EFFECT_2]->TargetB = 0;
    });

    //Puddle Void Zone
    ApplySpellFix({119941}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(22);
    });

    //Blade Rush Dmg trigger(sword)
    ApplySpellFix({124290}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(26);
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
    });

    //Blade Rush Charge
    ApplySpellFix({124312,218599,243383,243829,235679}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 25;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    //Templest
    ApplySpellFix({119875}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);
    });

    //Gravity Flux
    ApplySpellFix({114062}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = 0;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = 0;
    });

    ApplySpellFix({113996,  //Bone Armor
        116606 //Troll Rush
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_DUMMY;
    });

    //Shadow blaze dmg
    ApplySpellFix({111628}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(44);
    });

    ApplySpellFix({116364,  //Arcane Velocity
        116018,  //Epicenter
        116157,  //Lightning fists
        116374,  //Lightning fists (trigger dmg)
        136324,  //Rising Anger
        211073 //Desiccating Stomp
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.CasterAuraSpell = 0;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_SHARE_DAMAGE;
    });

    //Arcane Resonance
    ApplySpellFix({116417}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 6;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = 6;
        spellInfo->Effects[EFFECT_1]->TargetB = 0;
    });

    //Epicenter(trigger dmg)
    ApplySpellFix({116040}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);//60 yards
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);
        spellInfo->Effects[EFFECT_2]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);
    });

    //Arcane Velocrity (trigger dmg)
    ApplySpellFix({116365}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);//60 yards
    });

    //Arcane Resonance(trigger dmg)
    ApplySpellFix({116434}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = 30;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(13);
    });

    //Energy tendrols (trigger spell - grip)
    ApplySpellFix({129724}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_JUMP;
    });

    //Titan Gase (trigger spell)
    ApplySpellFix({116782,
        116803}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(41); //150yards
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(41); //150yards
    });

    //Titan Gase (trigger spell)
    ApplySpellFix({118327}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(41); //150yards
    });

    //Emergizing Smash
    ApplySpellFix({116550}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(13); //10yards
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(13); //10yards
        spellInfo->Effects[EFFECT_2]->RadiusEntry = sSpellRadiusStore.LookupEntry(13); //10yards
    });

    ApplySpellFix({116161}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 2; // Set Phase to 2
        spellInfo->Effects[EFFECT_3]->Effect = 0; // No need to summon
    });

    ApplySpellFix({116272}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 2; // Set Phase to 2
    });

    ApplySpellFix({118303}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ANY;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_DUMMY;
    });

    ApplySpellFix({15850,  // Chilled
        16927,  // Chilled
        20005 // Chilled 
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.Mechanic = MECHANIC_SNARE;
    });

    ApplySpellFix({ 222206 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.Mechanic = MECHANIC_FEAR;
    });

    // Narrow Escape, Wisp Form, Spirit Owl Form
    ApplySpellFix({128405, 241849, 242070}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_TAKE_DAMAGE;
    });

    //Pheromones
    ApplySpellFix({122835}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->Effects[3]->TriggerSpell = 0;
    });

    //Pheromones trail tr ef
    ApplySpellFix({123120}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(44); //0.5yard
    });

    //Heal
    ApplySpellFix({122193}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.TargetAuraSpell = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = 25;
    });

    //Heal trigger
    ApplySpellFix({122147}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.TargetAuraSpell = 0;
        spellInfo->Effects[EFFECT_0]->TargetA = 25;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    //Massive Stomp
    ApplySpellFix({122408}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(18);//15 yards
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(18);
    });

    //Amber Scalpel trigger spell
    ApplySpellFix({121995}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.TargetAuraSpell = 0;
        spellInfo->Effects[EFFECT_0]->TargetA = 25;
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(15);//3yards
    });

    //Explose
    ApplySpellFix({122532}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(14);//8yards
        spellInfo->Effects[EFFECT_1]->Effect = 0;
    });

    //Cry of terror
    ApplySpellFix({123788}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = 1;
    });

    //Dread screetch
    ApplySpellFix({123735}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = 1;
        spellInfo->Effects[EFFECT_1]->TargetB = 0;
    });

    //Dread screetch trigger spell
    ApplySpellFix({123743}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(8);//5yards
    });

    ApplySpellFix({66289,  // Glaive
        67439, // Boulder
        119489 //Unleashed 
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(10);//30yards
    });

    //Sha Corruption
    ApplySpellFix({117052}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->Effect = 0;
    });

    ApplySpellFix({122767,  //Dread Shadows
        144776,  //Ground Pound
        154294, //Power Conduit
        227566,
        227539,
        227570
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
    });

    //SunBeam trigger aura
    ApplySpellFix({122789}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(26);//4yards
    });

    //Sun Breath
    ApplySpellFix({122855}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENTRY;
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_UNIT_SRC_AREA_ENTRY;
        spellInfo->Effects[EFFECT_1]->Effect = 0;
    });

    //Spray
    ApplySpellFix({123121}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(7);//2yards
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(7);//2yards
    });

    //Eerie skull trigger spell
    ApplySpellFix({119495}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(7);//2yards
    });

    //Penetrating bolt trigger spell
    ApplySpellFix({119086}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(26);//4yards
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(26);//4yards
    });

    ApplySpellFix({137162,  //Static burst
        144115,  //Flame Coating
        119610,  //Bitter thoughts
        139901 //Stormcloud tr ef - dmg
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_UNIT_SRC_AREA_ENEMY;
    });

    ApplySpellFix({137261,  //Lightning storm tr ef - dmg
        140819 //Lightning storm tr ef = dummy
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
    });

    //Stormcloud
    ApplySpellFix({139900}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    //Double swipe tr ef
    ApplySpellFix({136740}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CONE_BACK;
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(4);
    });

    ApplySpellFix({136991, //Bitting cold tr ef 
        144766 //Detonation sequence
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_UNIT_SRC_AREA_ALLY;
    });

    //Reckless charge (point dmg)
    ApplySpellFix({137122}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->Effects[EFFECT_1]->TargetA = 22;
        spellInfo->Effects[EFFECT_1]->TargetB = 15;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(8);//5yards
    });

    ApplySpellFix({ 235673 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 150;
        spellInfo->Effects[EFFECT_0]->MiscValue = 150;
    });

    ApplySpellFix({ 236335 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_FORCE_CAST;
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
    });

    ApplySpellFix({ 236339 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->BasePoints = 1000000;
    });

    //Quake stomp
    ApplySpellFix({134920}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[2]->TriggerSpell = 0;
    });

    //Spinning shell dmg
    ApplySpellFix({134011}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 6;
        spellInfo->Effects[EFFECT_1]->TargetA = 6;
        spellInfo->Effects[2]->TargetA = 6;
    });

    //Furios stone breath tr ef dmg(nerf)
    ApplySpellFix({133946}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 34124;
    });

    //Plasma Sphere: 2 - blizzlike
    ApplySpellFix({218520}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 2;
    });

    //Drain the weak tr ef dmg
    ApplySpellFix({135101}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 6;
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(2);
    });

    //Cinders dot
    ApplySpellFix({139822}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 6;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = 6;
        spellInfo->Effects[EFFECT_1]->TargetB = 0;
    });

    //Cinders void zone dmg
    ApplySpellFix({139836}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetB = 15;
    });

    //Acidic explosion tr ef dmg
    ApplySpellFix({136220}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_SHARE_DAMAGE;
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(8);
    });

    //Explosive slam
    ApplySpellFix({138569}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->Effects[EFFECT_1]->TargetB = 15;
    });

    //Throw spear tr ef
    ApplySpellFix({134926}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = 0;
        spellInfo->Effects[EFFECT_1]->MiscValue = 0;
        spellInfo->Effects[EFFECT_1]->MiscValueB = 0;
    });

    //Beast of nightmares target aura
    ApplySpellFix({137341}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_1]->TargetA = 1;
        spellInfo->Effects[2]->TargetA = 1;
        spellInfo->Effects[3]->TargetA = 1;
        spellInfo->Effects[4]->TargetA = 1;
    });

    //Tears of Sun
    ApplySpellFix({137405}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.ExcludeTargetAuraSpell = 0;
    });

    //Ice Comet tr ef
    ApplySpellFix({137419}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = 0;
        spellInfo->Effects[EFFECT_1]->MiscValue = 0;
        spellInfo->Effects[EFFECT_1]->MiscValueB = 0;
    });

    ApplySpellFix({134912,  //Decapitate base aura
        198245 //Brutal Haymaker
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(3); //20yards
    });

    //Static shock base aura
    ApplySpellFix({135695}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 6;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    //Material of creation
    ApplySpellFix({138321}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    //Unleashed anime
    ApplySpellFix({138329}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Effects[EFFECT_1]->TargetA = 1;
        spellInfo->Effects[EFFECT_1]->TargetB = 0;
    });

    //Sha pool dummy
    ApplySpellFix({143461}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
    });

    //Sha splash
    ApplySpellFix({143297}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        //spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(65); // 1.5s
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(7); //2yards
    });

    //Sha splash Dummy
    ApplySpellFix({130063}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_DUMMY;
        //spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(21);
    });

    ApplySpellFix({145377,  //Erupting water
        143524 //Purified residue
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetB = TARGET_UNIT_SRC_AREA_ALLY;
    });

    // Light's Beacon
    ApplySpellFix({53651}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = -1;
        spellInfo->GetMisc()->MiscData.Attributes[0] &= ~SPELL_ATTR0_PASSIVE;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CANT_BE_SAVED_IN_DB;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_DEATH_PERSISTENT;
    });

    ApplySpellFix({ 246809 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 246817;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    //Swirl
    ApplySpellFix({113762}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 125925;
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_PASSIVE;
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_HIDDEN_CLIENTSIDE;
    });

    //Swirl dmg
    ApplySpellFix({143412}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 1;
        spellInfo->Effects[EFFECT_1]->TargetA = 1;
        spellInfo->Effects[EFFECT_0]->Effect = 2;
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = 0;
    });

    //Swirlr tr ef (Cone Searcher)
    ApplySpellFix({125925}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENEMY_110;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(13); //200yards
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(22); //200yards
    });

    //Swelling corruption
    ApplySpellFix({143574}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_PROC_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 143579;
    });

    ApplySpellFix({ 148581 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_SCHOOL_DAMAGE;
    });

    ApplySpellFix({221728,
        209454, 
        221603,
        221606, // fel of sargeras
        233963, // Demonic Upheaval
        229693,  // Poison Fang
        228252,  // Shadow Rend
        223912,  // Crush Armor
        212943,  // Lightning Rod
        217206,  // Gust of Wind
        193607,  // Double Strike
        143579,  // Immerseus,  Sha Corruption
        103534,  // Resonating Crystal,  Danger
        103536,  // Resonating Crystal,  Warning
        103541,  // Resonating Crystal,  Safe
        105479,  // Corruption,  Searing Plasma
        144774,  // Sha of Pride,  Reaching Attack
        100941,  // CATA,  Ragnaros,  Dreadflame
        146703,  // Amalgam of Corruption,  Bottomless Pit
        148310,  // Bombard Stun
        148311,  // Bombard Stun
        82881,   // Mortality
        136992,  // Bitting cold
        134916,  // Decapitate tr ef
        146325,  // Cutter Laser Visual Target
        144918,  // Cutter Laser Dmg
        159226,  // Solar Storm
        197556,  // Fenryr,  Ravenous Leap
        202231,  // Leech
        194956,  // Reap Soul
        198635,  // Unerring Shear
        215449,  // Necrotic Venom
        221028,  // Unstable Decay
        212570,  // Surrendered Soul
        202217,  // Mandible Strike
        227742,  // Moroes,  Garrote
        227465,  // Power Discharge
        206617,  // Time Bomb
        85576,   // Withering Winds
        85578,   // Chilling Winds
        85573,   // Deafening Winds
        213148,  // Searing Brand
        167615,  // Pierced Armor
        156152,  // Gushing Wounds
        161345,  // Suppression Field
        218342,  // Parasitic Fixate
        182879,
        183634,
        181306,
        180389,
        188929,
        186785,
        209615,  // Elisande: Ablation
        209973,  // Elisande: Ablation
        211887,  // Elisande: Ablation
        184369,
        227491,
        227490,
        227498,
        227499,
        227500,
        247742,
        247361,
        228891,
        254429, //Felhounds: Weight of Darkness
        250262,
        248332, //Eonar: Rain of Fel
        227545, //Mana Drain
        244410, //Worldbreaker: Decimation
        246920, //Worldbreaker: Decimation
        164965,
        169658,
        156841,
        236226,
        225732,
        194960
    },[](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE;
    });

    ApplySpellFix({192048,  //Expel Light
        204044,  //Shadow Burst
        204470, //Volatile Rot
        231277,
        230989
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CAN_BE_CASTED_ON_ALLIES;
    });

    ApplySpellFix({144396,  //Vengeful Strikes. WTF. SPELL_AURA_MOD_POSSESS_PET
        205233 //Lord Betrug,  Execution veh
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->Effect = 0;
        spellInfo->Effects[0]->ApplyAuraName = 0;
    });

    //Dark Meditation
    ApplySpellFix({143730}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 143546;
    });

    //Mark of Anguish. WTF. SPELL_AURA_MOD_POSSESS_PET
    ApplySpellFix({143840}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[3]->Effect = 0;
        spellInfo->Effects[3]->ApplyAuraName = 0;
    });

    // hack, useless triggers. I just need to perform all the checks for 1 trigger.
    ApplySpellFix({ 231561 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->TriggerSpell = 0;
        spellInfo->Effects[EFFECT_3]->TriggerSpell = 0;
    });

    //Lingering Corruption
    ApplySpellFix({144514}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.SchoolMask |= SPELL_SCHOOL_MASK_NORMAL;
    });

    // DH damage spells custom SM. wtf? why some havoc spells get holy+normal school? Magic absorbs not working with damage
    ApplySpellFix({199547, 222031, 201428, 227518, 211796, 211797, 192611, 202388, 202446, 203704, 217070}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.SchoolMask = SPELL_SCHOOL_MASK_SPELL;
    });

    //Burst of Anger
    ApplySpellFix({147082}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.TargetAuraSpell = 144421;
    });

    //Unleashed Anger
    ApplySpellFix({145214}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.CasterAuraSpell = 0;
        spellInfo->Effects[2]->TargetA = 25;
    });

    //Blind Hatred Dummy
    ApplySpellFix({145573}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 500;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 145227;
    });

    //Blind Hatred Dmg
    ApplySpellFix({145227}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->Effects[EFFECT_1]->TargetB = 15;
    });

    //Icy Fear Dmg, Residual Corruption
    ApplySpellFix({145735, 145073}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.TargetAuraSpell = 0;
    });

    //Corruption
    ApplySpellFix({144421}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_APPLY_AURA;
    });

    //Tear Reality
    ApplySpellFix({144482}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENEMY_104;
    });

    //Titanic Smash
    ApplySpellFix({144628}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CONE_ENEMY_24;
        spellInfo->Effects[2]->TargetA = TARGET_UNIT_CONE_ENEMY_24;
    });

    //Borer Drill Dmg
    ApplySpellFix({144218}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->Effect = SPELL_EFFECT_APPLY_AURA;
        spellInfo->Effects[0]->TargetA = TARGET_UNIT_TARGET_ANY;
    });

    ApplySpellFix({ 225762 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->Effect = SPELL_EFFECT_LOOT_CORPSE;
    });

    //Scatter Laser
    ApplySpellFix({144458}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_0]->TargetB = NULL;
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_1]->TargetB = NULL;
    });

    ApplySpellFix({144555,  //Mortar Barrage
        143848 //Essence of yshaarj
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = TARGET_UNIT_CASTER;
        spellInfo->Effects[0]->TargetB = 0;
    });

    //Froststorm strike 
    ApplySpellFix({144215}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(13); //200yards
    });

    //Iron Tomb dmg
    ApplySpellFix({144334}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = TARGET_DEST_CASTER_FRONT;
    });

    ApplySpellFix({
        48503,   // Living Seed
        201633,  // Earthen Shield
        200851,  // Rage of the Sleeper
        209426,  // Darkness
        195181,  // Bone Shield
        207319,  // Corpse Shield
        206977,  // Blood Mirror
        203975,  // Earthwarden
        208771,  // Smite
        197268,  // Ray of Hope (PvP Talent)
        144331,  // Iron Prison
        142906,  // Ancient Miasma Dmg
        29604,   // Jom Gabbar
        204266,  // Swelling Waves (PvP Talent)
        108366,  // Soul Leech
        233263,  // Embrace of the Eclipse
        233264,  // Embrace of the Eclipse
        215300,  // Web of Pain
        215307   // Web of Pain
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] |= SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS;
    });

    //Ravager Summon
    ApplySpellFix({143872}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[1]->TargetA = TARGET_DEST_TARGET_ENEMY;
    });

    //Heroic Shockwave
    ApplySpellFix({143716}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_CASTER;
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 143715; //Summon
    });

    ApplySpellFix({143420,  // Ironstorm
        105847,  // Seal Armor Breach
        105848},
        [](SpellInfo* spellInfo)
    {
        spellInfo->InterruptFlags = 0;
    });

    // Arcane Slash
    ApplySpellFix({206641}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_SHARE_DAMAGE;
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(4);
        spellInfo->InterruptFlags = 0;
    });

    //Hunter's Mark
    ApplySpellFix({143882}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.ExcludeTargetAuraSpell = 0;
    });

    ApplySpellFix({146257,  //Path of Blossoms Dmg
        146289  //Mass Paralyses
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = 22;
        spellInfo->Effects[0]->TargetB = 15;
    });

    ApplySpellFix({146189,  //Eminence
        146068 //Blade of the hundred steps (tank buff)
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->ApplyAuraName = SPELL_AURA_PROC_TRIGGER_SPELL;
        spellInfo->Effects[0]->TriggerSpell = 0;
    });

    //Shadow Volley Dummy
    ApplySpellFix({148515}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[1]->TargetA = 22;
        spellInfo->Effects[1]->TargetB = 15;
    });

    //Jade Tempest AT
    ApplySpellFix({148582}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[2]->Effect = 0;
        spellInfo->Effects[3]->Effect = 0;
        spellInfo->Effects[4]->Effect = 0;
    });

    //Jade Tempest Dmg
    ApplySpellFix({148583}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->BasePoints = 105000;
    });

    ApplySpellFix({147607}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = 25;
        spellInfo->Effects[0]->TargetB = 0;
        spellInfo->Effects[1]->TargetA = 25;
        spellInfo->Effects[1]->TargetB = 0;
    });

    //Cannon Ball Dest Dmg
    ApplySpellFix({147906}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[4]->RadiusEntry = sSpellRadiusStore.LookupEntry(14);//8yards
    });

    //Tail lash
    ApplySpellFix({143428}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CONE_BACK;
    });

    //Clump Check
    ApplySpellFix({143430}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.TargetAuraSpell = 0;
        spellInfo->Effects[0]->RadiusEntry = sSpellRadiusStore.LookupEntry(22); //200yards
        spellInfo->Effects[0]->Effect = SPELL_EFFECT_DUMMY;
        spellInfo->Effects[0]->TargetA = 22;
        spellInfo->Effects[0]->TargetB = 15;
    });

    //Fixate
    ApplySpellFix({143445}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = 25;
        spellInfo->Effects[0]->TargetB = 0;
        spellInfo->Effects[1]->TargetA = 25;
        spellInfo->Effects[1]->TargetB = 0;
    });

    //Enrage
    ApplySpellFix({146982}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = 1;
        spellInfo->Effects[0]->TargetB = 0;
        spellInfo->Effects[1]->TargetA = 1;
        spellInfo->Effects[1]->TargetB = 0;
        spellInfo->Effects[2]->TargetA = 1;
        spellInfo->Effects[2]->TargetB = 0;
        spellInfo->Effects[3]->TargetA = 1;
        spellInfo->Effects[3]->TargetB = 0;
        spellInfo->Effects[4]->TargetA = 1;
        spellInfo->Effects[4]->TargetB = 0;
    });

    //Anger
    ApplySpellFix({119487}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 22;
        spellInfo->Effects[EFFECT_0]->TargetB = 15;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(30);
        spellInfo->Effects[EFFECT_1]->TargetA = 22;
        spellInfo->Effects[EFFECT_1]->TargetB = 15;
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(30);
    });

    //Stomp
    ApplySpellFix({121787}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(12);
        spellInfo->Effects[EFFECT_2]->RadiusEntry = sSpellRadiusStore.LookupEntry(12);
    });

    //Barrage Dmg
    ApplySpellFix({121600}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(13);
        spellInfo->Effects[EFFECT_2]->RadiusEntry = sSpellRadiusStore.LookupEntry(13);
    });

    //Static shield
    ApplySpellFix({136341}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
    });

    // Life Spirit
    ApplySpellFix({130649}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 10000;
    });

    // Water Spirit
    ApplySpellFix({130650}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 5000;
    });

    // Master Mana Potion
    ApplySpellFix({105709}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 30000;
    });

    //Vanish
    ApplySpellFix({11327}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[4] |= SPELL_ATTR4_TRIGGERED;
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_MELEE_ATTACK;
    });

    //Argus: Golganneth's Wrath
    ApplySpellFix({256674}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] &= ~SPELL_ATTR3_ONLY_TARGET_PLAYERS;
    });

    //Argus: Golganneth's Wrath
    ApplySpellFix({255646}, [](SpellInfo* spellInfo)
    {
        for (auto diff : { DIFFICULTY_NONE, DIFFICULTY_NORMAL_RAID, DIFFICULTY_HEROIC_RAID, DIFFICULTY_MYTHIC_RAID, DIFFICULTY_LFR_RAID })
        {
            const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_0, diff))->TargetA = TARGET_UNIT_SRC_AREA_ENTRY;
            const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_0, diff))->TargetB = TARGET_NONE;
        }
    });

    ApplySpellFix({ 247551 }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] &= ~AURA_INTERRUPT_FLAG_LEAVE_COMBAT;
    });

    //Lightning Bolt
    ApplySpellFix({45284}, [](SpellInfo* spellInfo)
    {
        spellInfo->ClassOptions.SpellClassMask[0] = 0x1;
        spellInfo->ClassOptions.SpellClassMask[2] = 0;
    });

    // Summon Ghosts from Urns - Summons
    ApplySpellFix({122137}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 5;
    });

    ApplySpellFix({216937, 216938,  227261,
        74793, // Summoning Ritual
        134210 // Scenario,  Pursuing the Black Harvest - Memory of the Reliquary 
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->CastingReq.RequiredAreasID = -1;
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_NONE;
    });

    // Summon Charbringer
    ApplySpellFix({75478}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_CASTER_RANDOM;
    });

    // Stealth
    ApplySpellFix({1784}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_IS_STEALTH_AURA;
    });

    // Selfless Healer
    ApplySpellFix({128863}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[1] &= ~SPELL_ATTR1_CU_IS_USING_STACKS;
    });

    // Feast On Turkey
    ApplySpellFix({61784}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 61842;
        spellInfo->GetAuraOptions()->ProcChance = 100;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_MASTER;
    });

    // Feast On Sweet Potatoes
    ApplySpellFix({61786}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 61844;
        spellInfo->GetAuraOptions()->ProcChance = 100;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_MASTER;
    });

    // Feast On Stuffing
    ApplySpellFix({61788}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 61843;
        spellInfo->GetAuraOptions()->ProcChance = 100;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_MASTER;
    });

    // Feast On Pie
    ApplySpellFix({61787}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 61845;
        spellInfo->GetAuraOptions()->ProcChance = 100;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_MASTER;
    });

    // Feast On Cranberries
    ApplySpellFix({61785}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_TRIGGER_SPELL;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 61841;
        spellInfo->GetAuraOptions()->ProcChance = 100;
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_MASTER;
    });

    ApplySpellFix({20711}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_DUMMY;
        spellInfo->Effects[EFFECT_0]->BasePoints = 1;
        spellInfo->Effects[EFFECT_0]->MiscValue = 0;
    });

    ApplySpellFix({119914, //Felstorm
        119915 //Felstorm
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
    });

    //Dancing Blade
    ApplySpellFix({193235}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_NONE;
    });

    ApplySpellFix({70781, // Light's Hammer Teleport
        70856, // Oratory of the Damned Teleport
        70857, // Rampart of Skulls Teleport
        70858, // Deathbringer's Rise Teleport
        70859, // Upper Spire Teleport
        70860, // Frozen Throne Teleport
        70861, // Sindragosa's Lair Teleport
        108786,// Summon Stack of Reeds
        108808,// Mop, quest
        108830,// Mop, quest
        108827,// Mop, quest
        104450,// Mop, quest
        108845,// Mop, quest
        108847,// Mop, quest
        108857,// Mop, quest
        108858,// Mop, quest
        109335,// Mop, quest
        105002,// Mop, quest
        117497,// Mop, quest
        117597,// Mop, quest
        69971, //q.24502
        69976,
        69977,
        69978,
        69979,
        69980,
        69981,
        69982}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_DB;
    });

    // Mop, quest
    ApplySpellFix({115435}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValueB = 1;
    });

    ApplySpellFix({84964,  // Rayne's Seed
        101847, // Shoe Baby
        65203,  // Throw Oil
        50493 // D.I.S.C.O.
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_SRC_AREA_ENTRY;
    });

    // Gather Lumber
    ApplySpellFix({66795}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_MASTER;
    });

    ApplySpellFix({
        498,    // Divine Protection
        47788,  // Guardian Spirit
        51490,  // Thunderstorm
        204406, // Thunderstorm
        213664  // Nimble Brew (Honor Talent)
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] |= SPELL_ATTR5_USABLE_WHILE_STUNNED;
    });

    ApplySpellFix({20066, // Repentance
        198898, // Song of Chi-Ji
        116670, // Vivify
        152118, // Clarity of Will
        207778, // Gift of the Queen
        211714, // Thal'kiel's Consumption
        202767, // New Moon
        202768, // Half Moon
        202771, // Full Moon
        207946, // Light's Wrath
        214634, // Ebonbolt
        228193, // Burning Blaze
        205300, // Corruption
        205495, // Stormkeeper  
        248831  // Dread Screech
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
    });

    ApplySpellFix(
    {
        5277,    // Evasion
        118038,  // Die by the Sword
        236385   // Die by the Sword
    },
        [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 200;
    });

    // Sheilun's Gift, Comet Storm
    ApplySpellFix({205406, 153595}, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.PreventionType = SPELL_PREVENTION_TYPE_SILENCE;
        spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
    });

    // Lightning Burst Aura
    ApplySpellFix({224121, 224117}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_LANDING;
    });

    // Aegis of Aggramar
    ApplySpellFix({193783}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 653;
    });

    // Decomposing Aura, Fragile Echoes
    ApplySpellFix({199721, 225281, 225292, 225294, 225297, 225298, 225366}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 10 * IN_MILLISECONDS;
    });

    // Kidney Shot
    ApplySpellFix({408}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.MaxDuration = 6 * IN_MILLISECONDS;
    });

    // Sprint , Mutilated Flesh
    ApplySpellFix({245752, 245756, 211672}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 8 * IN_MILLISECONDS;
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 2000;
    });

    // Felhounds: Desolate Path
    ApplySpellFix({244768}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 8000;
    });

    ApplySpellFix({ 228174, 241866 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 20 * IN_MILLISECONDS;
    });

    ApplySpellFix({ 241844 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 19 * IN_MILLISECONDS;
    });

    // Vantus Rune
    ApplySpellFix({ 229174,
        229175,
        229176,
        192767,
        192768,
        192769,
        192770,
        192771,
        192772,
        192773,
        192774,
        192775,
        192776,
        192761,
        192762,
        192763,
        192764,
        192765,
        192766,
        191464,
        237820,
        237821,
        237822,
        237823,
        237824,
        237825,
        237826,
        237827,
        237828,
        250153,
        250156,
        250167,
        250160,
        250150,
        250158,
        250148,
        250165,
        250163,
        250144,
        250146 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 604800 * IN_MILLISECONDS;
    });

    // Shadow Techniques
    ApplySpellFix({196911}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->BasePoints = 8;
    });

    // Spider Sting (PvP Talent)
    ApplySpellFix({202933}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CASTER;
    });

    ApplySpellFix({23035, // Battle Standard (Horde)
        23034 // Battle Standard (Alliance)
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValueB = 3291;  //SUMMON_TYPE_BANNER
    });

    ApplySpellFix({68441,
        68440}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ChainTargets = 60;
    });

    ApplySpellFix({71919,
        71918,
        83115,
        83116}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValueB = 3302;  //SUMMON_TYPE_MINIPET
    });

    // Toss Stink Bomb Credit
    ApplySpellFix({96117}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_PASSENGER_0;
    });

    // Teleport outside (Isle of Conquest)
    ApplySpellFix({66550}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(1);
    });

    // Frozen Thoughts
    ApplySpellFix({146557}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->SpellClassMask[0] |= 131616;
    });

    // Hour of Twilight
    ApplySpellFix({106371}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 103327;
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 106174;
    });

    // Harpoon
    ApplySpellFix({108038}, [](SpellInfo* spellInfo)
    {
        spellInfo->SetRangeIndex(13); // 5000 yards
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ANY;
    });

    // Earthen Vortex
    ApplySpellFix({103821}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_APPLY_AURA;
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_STUN;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(48); // 60 yard
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);
        spellInfo->Effects[2]->RadiusEntry = sSpellRadiusStore.LookupEntry(48);
    });

    // Earths Vengeance dmg
    ApplySpellFix({103178}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 23 * IN_MILLISECONDS;
    });

    ApplySpellFix({105241, // Absorb Blood
        106444 // Impale
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(26); // 4 yards
    });

    ApplySpellFix({106663, // Carrying Winds
        106668,
        106670,
        106672,
        106674,
        106676}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_DEST;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->AuraRestrictions.ExcludeCasterAuraSpell = 0;
        spellInfo->Misc.Duration.Duration = 1 * IN_MILLISECONDS;
    });

    ApplySpellFix({106028,
        106027,
        106457,
        106464,
        106029}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 10 * IN_MILLISECONDS;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
    });

    // Spellweaving
    ApplySpellFix({106040}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetAuraOptions()->ProcChance = 10;
        spellInfo->Misc.Duration.Duration = 10 * IN_MILLISECONDS;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
    });

    ApplySpellFix({105825,
        105823,
        106456,
        106463,
        106026,
        106039}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 7000;
    });

    // Cataclysm screen
    ApplySpellFix({106527}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 10 * IN_MILLISECONDS;
    });

    // Lightning Conduit dmg
    ApplySpellFix({105369}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 1 * IN_MILLISECONDS;
        spellInfo->GetMisc()->MiscData.Attributes[5] |= SPELL_ATTR5_HIDE_DURATION;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->GetMisc()->MiscData.Attributes[8] |= SPELL_ATTR8_DONT_RESET_PERIODIC_TIMER;
    });

    // Lightning Conduit dummy 1
    ApplySpellFix({105367}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 1 * IN_MILLISECONDS;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_APPLY_AURA;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_PERIODIC_DUMMY;
        spellInfo->Effects[EFFECT_1]->ApplyAuraPeriod = 1000;
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_TARGET_ANY;
    });

    // Lightning Conduit dummy 2
    ApplySpellFix({105371}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 1 * IN_MILLISECONDS;
        spellInfo->GetMisc()->MiscData.Attributes[5] |= SPELL_ATTR5_HIDE_DURATION;
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
    });

    // Relentless Strikes
    ApplySpellFix({58423}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->SpellClassMask[1] |= 8;
    });

    ApplySpellFix({
        22842,  // Frenzied Regeneration
        77489,  // Echo of Light
        184250, // Divine Intervention
        210380, // Aura of Sacrifice
        210383, // Aura of Sacrifice
        198116, // Temporal Shield
        195381, // Healing Winds
        143924, // life Steal
        204262, // Spectral Recovery
        242597, // Rethu's Incessant Courage
        235967, // Velen's Future Sight
        158188, // Stealth
        213858, // Battle Trance (PvP Talent)
        199668, // Blessings of Yu'lon
        199483, // Camouflage
        198249, // Elemental Healing
        212640, // Mending Bandage (PvP Talent)
        203924, // Healing Shell
        203539, // Greater Blessing of Wisdom
        248522, // Fury of Nature
        196356  // Trust in the Light
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CANT_CRIT;
    });

    // Spirit World aura
    ApplySpellFix({96689}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 2;
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_UNIT_SRC_AREA_ENEMY;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_IGNORE_HIT_RESULT;
    });

    ApplySpellFix({54355, // Achiev Mine Sweeper
        54402}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CASTER;
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_CASTER;
    });

    // Release Aberrations
    ApplySpellFix({77569}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_GAMEOBJECT_SRC_AREA;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(28);
    });

    // Release All
    ApplySpellFix({77991}, [](SpellInfo* spellInfo)
    {
        spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_INTERRUPT;
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_GAMEOBJECT_SRC_AREA;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(28);
        spellInfo->Effects[EFFECT_1]->Effect = 0;
    });

    // Ling-Ting's Herbal Journey
    ApplySpellFix({115037}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 3;
    });

    ApplySpellFix({239081, // Rain of Blades
        119377 // Siege Explosive
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_TARGET_RADIUS;
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(10);
    });

    ApplySpellFix({54724, // Teleport to Hall of Command
        54725,
        54745,
        54746,
        54699, // Teleport to Heart of Acherus
        54700,
        54742,
        54744}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] |= SPELL_ATTR6_CAN_TARGET_INVISIBLE;
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_DEAD;
    });

    // Wod, Q34429
    ApplySpellFix({166216}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = -1;
    });

    // Wod, lost scene id.
    ApplySpellFix({163772}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 801;
    });

    ApplySpellFix({44614, 1953, 119996}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CAST_DIRECTLY;
    });

    // Saber Slash (Trigger)
    ApplySpellFix({197834}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_3]->Effect = SPELL_EFFECT_DUMMY;
        spellInfo->Effects[EFFECT_3]->MiscValue = 0;
        spellInfo->Effects[EFFECT_3]->ApplyAuraName = 0;
        spellInfo->Effects[EFFECT_3]->BasePoints = 0;
        spellInfo->Misc.Duration.Duration = 0;
    });

    // Stagger
    ApplySpellFix({115069}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->BasePoints = 0;
        spellInfo->Effects[EFFECT_3]->BasePoints = 0;
    });

    //Wod, Q, 34439 Blizz use script for change summon entry. But i did it by this.
    ApplySpellFix({161166}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 79315;
    });

    ApplySpellFix({161065}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 78430;
    });

    ApplySpellFix({161167}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 79675;
    });

    ApplySpellFix({158818}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 79767;
    });

    // Darkmoon, Using Steam Tonk Controller
    ApplySpellFix({100752}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetB = TARGET_DEST_DB;
    });

    // Forging
    ApplySpellFix({138869}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Amplitude = 655 * 2;
    });

    // Dig Rat Stew
    ApplySpellFix({109659}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 6417;
    });

    ApplySpellFix({153238}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 2500;
    });

    // Q, 34364
    ApplySpellFix({169422}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 604;
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_ACTIVATE_SCENE;
    });

    ApplySpellFix({163452}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CASTER;
    });

    ApplySpellFix({52042, // Healing Tide
        185453, // Teleport to Bottom  // From sniff player casted self
        185454 // Teleport to Middle  // From sniff player casted self
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CANT_TARGET_SELF;
    });

    ApplySpellFix({168082, // Revitalizing Waters
        168105, // Rapid Tides
        177497, // Bramble Patch
        168041, // Briarskin
        168375, // Grasping Vine
        175997, // Noxious Eruption
        198263, // Odyn, Radiant Tempest
        198077, // Odyn, Shatter Spears
        197961// Odyn, Runic Brand
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.CasterAuraSpell = 0;
    });

    ApplySpellFix({160856, // Q, 34582
        178790}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_CASTABLE_WHILE_MOUNTED;
    });

    // Shadowy Duel (Honor Talent), Rake
    ApplySpellFix({207736, 1822}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->TriggerSpell = 0;
    });

    // Shroud of Arcane Echoes 
    ApplySpellFix({248779}, [](SpellInfo* spellInfo)
    {
        spellInfo->CastingReq.RequiredAreasID = 5389;
        spellInfo->Effects[EFFECT_2]->TriggerSpell = 0;
    });

    // Incarnation, Tree of Life
    ApplySpellFix({33891}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_2]->ApplyAuraName = SPELL_AURA_NONE;
        spellInfo->Effects[EFFECT_2]->TriggerSpell = 0;
    });

    // Enraged Maim (PvP Talent)
    ApplySpellFix({236026}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_3]->MiscValue = 400;
    });

    ApplySpellFix({197484, //Dark Rush
        204371, //Skropyron, Call of the Scorpid
        204468, //Skropyron, Focused Blast
        205200 //Skropyron, Arcanoslash
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = 0;
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
    });

    ApplySpellFix({235420, // Fel Munition
        197687  //Illysanna, Dark Rush
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(6); //100yards
    });

    //Beam, Akaari's Soul, Fists of Fury Visual Target
    ApplySpellFix({202046, 209836, 209837, 123154}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 1000;
    });

    //Cordana, Deepening Shadows
    ApplySpellFix({213411}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 213397;
    });

    //Lightning Strike
    ApplySpellFix({192794,247361, 233983}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_ONLY_TARGET_PLAYERS;
    });

    ApplySpellFix({204513, //Sigil of Flame
        227523 //Energy Void
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_DEST;
    });

    //Soulgorge
    ApplySpellFix({196930}, [](SpellInfo* spellInfo)
    {
        spellInfo->SetRangeIndex(6); // 100 yards
    });

    ApplySpellFix({
        193781, // Aegis of Aggramar
        212036, // Mass Resurrection
        212040, // Revitalize
        212048, // Ancestral Vision
        212051, // Reawaken
        212056, // Absolution
        196960, // Balnazzar blink
        81269,  // Efflorescence
        238490, // Fel Blast
        241622,  // Approaching Doom
        43671
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
    });

    //Summon Ossunet
    ApplySpellFix({232756, 232905}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_IGNORE_HIT_RESULT;
    });

    ApplySpellFix({
        139569, // Combo Point Delayed
        193840, // Chaos Strike
        197341, // Ray of Hope (Honor Talent)
        207288  // Queen Ascendant
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[4] |= SPELL_ATTR4_HAS_DELAY;
    });

    // Demonic Empowerment, Renewing Mist
    ApplySpellFix({193396, 119611}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[13] |= SPELL_ATTR13_PANDEMIA;
    });

    ApplySpellFix({
        184362, // Enrage
        185422, // Shadow Dance (shapeshift trigger)
        207690, // Bloodlet
        162264, // Metamorphosis
        228287, // Cyclone Strikes
        215479, // Ironskin Brew
        192081  // Ironfur
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[13] &= ~SPELL_ATTR13_PANDEMIA;
    });

    // Fel Devastation
    ApplySpellFix({212084}, [](SpellInfo* spellInfo)
    {
        spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_MOVEMENT;
    });

    // Raging Blow
    ApplySpellFix({85288}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraRestrictions.CasterAuraState = AURA_STATE_ENRAGE;
        spellInfo->AuraRestrictions.CasterAuraSpell = 0;
    });

    // Thrash
    ApplySpellFix({106830}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BonusCoefficient = 0.0f;
    });

    ApplySpellFix({
        136769, // Horridon charge
        144278, // Generate rage
        53563,  // Beacon of Light
        156910, // Beacon of Faith
        143462, // Sha pool
        196659, // Shadow Barrage
        209835, // Akaari's Soul
        207354, // Caress of the Tidemother
        234867, // Sephuz's Secret
        236763, // Sephuz's Secret
        194462, // Highblade's Will
        233556, //
        240623, //
        207356, // Refreshing Currents
        196941, // Judgment of Light (Triggered from base for use correct caster)
        207835, // Stormlash (Honor Talent)
        235659
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
    });

    // Bonestorm
    ApplySpellFix({194844}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_POSITIVE_FOR_CASTER;
    });

    // Blighted Rune Weapon
    ApplySpellFix({195758}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF0;
    });

    // Fists of Fury
    ApplySpellFix({113656}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_IGNORE_HIT_RESULT;
        spellInfo->Effects[EFFECT_4]->Effect = 0;
        spellInfo->Effects[EFFECT_1]->ApplyAuraPeriod = 1000;
        spellInfo->Effects[EFFECT_2]->ApplyAuraPeriod = 1000;
    });

    // Essence Font
    ApplySpellFix({191837}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 1000;
    });

    // Arena Dampening
    ApplySpellFix({110310}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->ApplyAuraPeriod = 10000;
    });

    ApplySpellFix({152175, // Whirling Dragon Punch
        200050 // Apocalyptic Fire
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 500;
    });

    //Fel Firebomb, Gift of the Sky, Gift of the Sea
    ApplySpellFix({190666, 253884, 253889}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_DEST;
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_NONE;
    });

    // Dominate Mind
    ApplySpellFix({216690}, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.Mechanic = MECHANIC_STUN;
    });

    // Ranger's Net
    ApplySpellFix({200108}, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.Mechanic = MECHANIC_ROOT;
    });

    ApplySpellFix({227678, // Gifted Student
        199407, // Light on Your Feet
        198293, // Wind Strikes
        190515, // Survival of the Fittest
        195391, // Jouster
        198300, // Gathering Storms
        214478, // Shroud of Mist
        199387, // Spirit Tether
        214871, // Odyn's Fury
        209950, // Caress of the Tidemother
        211903, // Faith's Armor
        198111, // Temporal Shield
        198240  // Spirit of the Maelstrom
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
    });

    // Tactical Advance
    ApplySpellFix({209484}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
        spellInfo->Effects[EFFECT_1]->BasePoints = 0;
    });

    //Il'gynoth Summons, Mind Flay
    ApplySpellFix({208697}, [](SpellInfo* spellInfo)
    {
        spellInfo->ChannelInterruptFlags[0] |= CHANNEL_INTERRUPT_FLAG_INTERRUPT;
    });

    ApplySpellFix({768, 783, 197625, 5487, 24858, 210053}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_DISABLED_WHILE_ACTIVE;
    });

    //Cenarius, Cleansed Ground
    ApplySpellFix({212639}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 60000;
    });

    // Annihilation
    ApplySpellFix({201427}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_2]->TargetA = TARGET_UNIT_TARGET_ENEMY;
    });

    // Ashamane's Frenzy
    ApplySpellFix({210722}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_TARGET_ENEMY;
    });

    // Metamorphosis disable buged animkit
    ApplySpellFix({162264}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_8]->Effect = SPELL_EFFECT_NONE;
    });

    // Fly to Broken Shore (teleport)
    ApplySpellFix({195237}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_PASSENGER_0;
    });

    // Auto Shot
    ApplySpellFix({75}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] |= SPELL_ATTR5_USABLE_WHILE_MOVING;
    });

    // Cauterize
    ApplySpellFix({86949}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->Effect = 0;
    });

    ApplySpellFix({ 192635 }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_CHANGE_MAP;
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_TELEPORTED;
    });

    // Crash Lightning
    ApplySpellFix({187874}, [](SpellInfo* spellInfo)
    {
        spellInfo->EquippedItemInventoryTypeMask = 2097152; // have SPELL_ATTR3_MAIN_HAND but find INVTYPE_WEAPONOFFHAND Oo
    });

    ApplySpellFix({
        204477, // Windburst
        225275, // Prayer of Mending
        201467, // Fury of the Illidari 
        206817, // Sentinel
        242553, // Umbral Glaive Storm
        253590  // Freezing Death
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Speed = 0.f;
    });

    // Corruption, Crushing Shadows
    ApplySpellFix({208860}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_IGNORE_ARMOR;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_IGNORE_HIT_RESULT;
    });

    // Voidtalon of the Dark Star
    ApplySpellFix({186228}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 179478;
    });

    // Expel Soul
    ApplySpellFix({213625}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 2000;
    });

    // Infested
    ApplySpellFix({204504}, [](SpellInfo* spellInfo)
    {
        const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_0, DIFFICULTY_HEROIC_RAID))->ApplyAuraPeriod = 2000;
        const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_0, DIFFICULTY_MYTHIC_RAID))->ApplyAuraPeriod = 2000;
    });

    // Ysondre, Gloom
    ApplySpellFix({206758}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 3000;
    });

    // Avoidance
    ApplySpellFix({190019}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE;
    });

    ApplySpellFix({93326, 121820, 101641}, [](SpellInfo* spellInfo) // Mounts (try to give invis morph)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 24417;
    });

    ApplySpellFix({23334}, [](SpellInfo* spellInfo) // New Drop Flag Horde (Silverwing && Warsong Flags)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 227744;
    });

    ApplySpellFix({23336}, [](SpellInfo* spellInfo) // New Drop Flag Alliance (Silverwing && Warsong Flags)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 227745;
    });

    ApplySpellFix(
    {   203102, //Ysondre Marks
        203121,
        203124,
        203125,
        213622, //Entombed in Ice
        213621,
        211615, //Sterilize
        208499,
        214573, //Stuffed
        200284, //Tangled Web
        206607, //Chronometric Particles
        227856, //Elisande: Epocheric Vulnerability
        208944, //Elisande: Time Stop
        209136, //Elisande: Time Stop
        206480, //Tichondrius: Carrion Plague
        230201, //Sasszine: Burden of Pain
        247388, //Imonar: Pulse Grenade
        247681, //Imonar: Pulse Grenade
        247641, //Imonar: Stasis Trap
        247962, //Imonar: Blastwire
        247949, //Imonar: Shrapnel Blast
        244892, //Admiral Svirax: Exploit Weakness
        244016, //Hasabel: Reality Tear
        245509, //Kil'jaeden: Felclaws
        236494, //Fallen Avatar: Desolate
        231998, //Harjatan: Jagged Abrasion
        214529  //Cenarius: Spear of Nightmares
    },
        [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_IGNORE_AVOID_MECHANIC;
    });

    ApplySpellFix({219493}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 104;
    });

    // Seadog's Scuttle (need charge)
    ApplySpellFix({213588}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_CHARGE;
    });

    // Soul Rend
    ApplySpellFix({213606}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 16;
    });

    // Fel Geyzer, Demonic Gateway
    ApplySpellFix({218872, 113890}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 125;
    });

    // Ashamane's Frenzy
    ApplySpellFix({210723}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->BonusCoefficientFromAP = spellInfo->Effects[EFFECT_2]->BonusCoefficientFromAP;
        spellInfo->Effects[EFFECT_2]->BonusCoefficientFromAP = 0.0f;
        spellInfo->Effects[EFFECT_2]->Effect = 0;
        spellInfo->Effects[EFFECT_2]->ApplyAuraName = 0;
    });

    ApplySpellFix({184707, 132169, 201364, 132168, 178741, 187727, 200685, 210155}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_TRIGGERED_CAN_TRIGGER_PROC_2;
    });

    ApplySpellFix({148017, 148018, 148019, 148020, 148021}, [](SpellInfo* spellInfo) // Icicles Visual
    {
        spellInfo->GetMisc()->MiscData.Speed = 20.f;
    });

    ApplySpellFix(
    {
        221299,
        232407,
        245636,
        251672,
        245635,
        251579,
        131527,
        131528,
        237738,
        199412,
        238485,
        239223,
        239224,
        239227,
        252509,
        255102,
        252508,
        252510,
        205473,
        214130,
        214127,
        214126,
        214125,
        214124,
        148012,
        148013,
        148014,
        148015,
        148016,
        212219,
        235213,
        235240,
        208822,
        227261,
        233430,
        236283,
        227032,
        33763,
        108446,
        68338,
        69303,
        72885,
        104571,
        114695,
        1066,
        144607,
        33943,
        165961,
        40120,
        163524,
        163525,
        163526,
        163527,
        163528,
        163529,
        163530,
        159126,
        159127,
        166646,
        116014,
        169482,
        108366,
        172380,
        216695,
        116267,
        206150,
        206151,
        195843,
        195838,
        202160,
        197912,
        228696,
        198439,
        158185,
        235027,
        17940,
        146739,
        235732,
        235734,
        235621,
        257233
    }, 
        [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CANT_BE_SAVED_IN_DB;
    });

    // Player Damage Reduction Level 95 etc.
    ApplySpellFix({142689, 178840, 178839}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CANT_BE_LEARNED;
    });

    ApplySpellFix({
        211309, // Artificial Stamina
        224146, // Nightwell Arcanum
        228448, // Fortitude of the Nightborne
        224148, // Jacin's Ruse
        224150  // Traitor's Oath
    }, 
        [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[8] |= SPELL_ATTR8_NOT_IN_BG_OR_ARENA;
        spellInfo->GetMisc()->MiscData.Attributes[4] |= SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG;
    });

    ApplySpellFix({185627}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_FEIGN_DEATH;
    });

    // Geroic Leap
    ApplySpellFix({6544}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 150;
    });

    // Rampage
    ApplySpellFix({184367}, [](SpellInfo* spellInfo)
    {
        spellInfo->Cooldowns.StartRecoveryTime = 1500;
        spellInfo->ClassOptions.SpellClassMask[3] &= ~0x1000;
    });

    // Carnage , Rampage
    ApplySpellFix({202922, 206316}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->SpellClassMask[3] |= 0x1000000;
        spellInfo->Effects[EFFECT_0]->SpellClassMask[3] &= ~0x1000;
    });

    // Master Poisoner
    ApplySpellFix({196864}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->SpellClassMask[3] &= ~0x2;
    });

    // Necrotic Venom
    ApplySpellFix({215460}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_IGNORE_HIT_RESULT;
    });

    // Annihilated, Hopelessness
    ApplySpellFix({215458, 237728}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_IGNORE_HIT_RESULT;
    });

    ApplySpellFix({213166}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY;
    });

    // Volcanic Plume
    ApplySpellFix({209861}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_DEST_TARGET_ENEMY;
    });

    // PvP Stats - Spec Stat Template - All Specs
    ApplySpellFix({198439}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] &= ~AURA_INTERRUPT_FLAG_CHANGE_MAP;
    });

    // Principles of War
    ApplySpellFix({197912, 228696}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_CHANGE_MAP;
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_USABLE_WHILE_MOVING;
    });

    // Well Fed
    ApplySpellFix({216343, 216353}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_CHANGE_MAP;
    });

    // Summon Arcane Raven
    ApplySpellFix({243303}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValueB = 495;
    });

    // Flurry, Leg Sweep, Fear
    ApplySpellFix({44614, 119381, 5782}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Mechanic = MECHANIC_NONE;
    });

    ApplySpellFix({234660, // Dread Beam
        237671 // Fel-Fire Ejection
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetB = TARGET_DEST_CASTER_FRONT;
    });

    // Fel Bombardment
    ApplySpellFix({235085}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
    });

    // Storm's Reach Greatstag Mount
    ApplySpellFix({218815}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 109942;
    });

    // Storm's Reach Cliffwalker Mount
    ApplySpellFix({212421}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 107020;
    });

    // Storm's Reach Warbear Mount
    ApplySpellFix({218891}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 109967;
    });

    // Storm's Reach Worg Mount
    ApplySpellFix({213147}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 107463;
    });
    
    // Stormtalon
    ApplySpellFix({218964}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 109994;
    });

    ApplySpellFix(
    {
        212874, //Temporal Orbs
        227217, //Temporal Orbs
        213650, //Echoes of the Void
    }, 
        [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 20000;
    });

    // Sigil of Flame
    ApplySpellFix({204598}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_2]->ApplyAuraName = SPELL_AURA_NONE;
        spellInfo->Effects[EFFECT_1]->BonusCoefficientFromAP = spellInfo->Effects[EFFECT_2]->BonusCoefficientFromAP;
    });

    // Scatter Shot
    ApplySpellFix({213691}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_CAN_PROC_WITH_TRIGGERED;
        spellInfo->Categories.Mechanic = MECHANIC_INCAPACITATE;
        spellInfo->Effects[EFFECT_1]->Mechanic = MECHANIC_INCAPACITATE;
        spellInfo->EffectMechanicMask = MECHANIC_NONE;
    });

    // Rage of the Illidari, Joy of Spring, Heart of the Void, Kakushan's Stormscale Gauntlets, Spirit of the Crane, Teachings of the Monastery, Lightning Rod
    ApplySpellFix({201472, 238086, 214858, 248296, 207841, 210802, 202090, 197209}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_CAN_PROC_WITH_TRIGGERED;
    });

    // Burn Body
    ApplySpellFix({42793}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->MiscValue = 24008; // Fallen Combatant
    });

    ApplySpellFix({
        44544,  // Fingers of Frost
        210126, // Arcane Familiar
        76613,  // Mastery: Icicles
        199484, // Psychic Link (Honor Talent)
        51564,  // Tidal Waves
        212018, // Item - Warlock T19 Destruction 2P Bonus
        207633, // Whisper of the Nathrezim
        238062, // Righteous Verdict
        234299, // Fist of Justice
        186788, // Echo of the Highlord
        63733,  // Serendipity
        206950, // Al'maiesh, the Cord of Hope
        49028,  // Dancing Rune Weapon
        192566, // Dancing Rune Weapon
        208190, // The Emerald Dreamcatcher
        112965, // Fingers of Frost
        190447, // Brain Freeze
        189849, // Dreamwalker
        189870, // Power of the Archdruid
        211557, // Justice Gaze
        200986, // Odyn's Champion
        223817, // Divine Purpose
        197354, // Surge of the Stormgod
        109186, // Surge of Light
        209781, // Shadow Nova
        210676, // Shadow Thrash
        213014, // Varo'then's Restraint
        199887, // The Mists of Sheilun
        193884, // Soothing Mist
        200482, // Second Sunrise
        196684, // Invoke the Naaru
        196705, // Invoke the Naaru
        208283, // Promise of Elune, the Moon Goddess
        203550, // Blind Fury
        210706, // Gore
        205702, // Feretory of Souls
        212580, // Eye of the Observer
        206889, // Ullr's Featherweight Snowshoes
        235691, // Gyroscopic Stabilization
        231895, // Crusade
        184783, // Tactician
        191861, // Power of the Maelstrom
        193533, // Steady Focus
        210707, // Aftershock
        208895, // Duskwalker Footpads
        191512, // Elementalist
        209256, // Drinking Horn Cover
        209354, // Delusions of Grandeur
        208199, // Manipulated Fel Energy
        211331, // Item - Hunter T19 Marksmanship 2P Bonus
        233766, // Control the Mists
        152278, // Anger Management
        238111, // Soul of the Slaughter
        248033, // The Topless Tower
        248034, // Chameleon Song
        248163, // Radiant Moonlight
        248081, // Behemoth Headdress
        248036, // Fire in the Deep
        248029, // Smoldering Heart
        248099, // Contained Infernal Core
        248100, // Shattered Fragments of Sindragosa
        248035, // Doorway to Nowhere
        248101, // The Wind Blows
        248037, // Inner Hallation
        248295, // The Alabaster Lady
        248113, // The Master Harvester
        242301, // Item - Warrior T20 Fury 4P Bonus
        242262, // Item - Paladin T20 Holy 4P Bonus
        251868, // Item - Paladin T21 Retribution 4P Bonus
        242057, // Item - Death Knight T20 Frost 2P Bonus
        242225, // Item - Death Knight T20 Unholy 4P Bonus
        242233, // Item - Druid T20 Balance 4P Bonus
        251809, // Item - Druid T21 Balance 4P Bonus
        251757, // Item - Shaman T21 Elemental 2P Bonus
        242249, // Item - Mage T20 Fire 2P Bonus
        242252, // Item - Mage T20 Frost 2P Bonus
        242242, // Item - Hunter T20 Marksmanship 2P Bonus
        242241, // Item - Hunter T20 Marksmanship 4P Bonus
        246126, // Item - Hunter T20 Beast Mastery 2P Bonus Driver
        242271, // Item - Priest T20 Holy 4P Bonus
        242293, // Item - Warlock T20 Demonology 2P Bonus
        210689, // Lightning Rod
        208908, // Mannoroth's Bloodletting Manacles
        203275, // Tinder (PvP Talent)
        202090, // Teachings of the Monastery
        235870, // Alexstrasza's Fury
        194912, // Gathering Storm
        238126, // Time and Space
        199441, // Avenging Light
        210540, // Pure of Heart (Honor Talent)
        209405, // Oneth's Intuition
        202941  // Moon and Stars
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_PROC_ONLY_ON_CAST;
    });

    // Divine Storm
    ApplySpellFix({53385}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetB = TARGET_UNIT_CASTER_AREA_RAID;
    });

    // Blessings of Yu'lon
    ApplySpellFix({199671}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValueB = 707;
    });

    // Shadowy Reflection
    ApplySpellFix({222481}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValueB = 3973;
    });

    // Starfall
    ApplySpellFix({191034}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME;
    });

    ApplySpellFix({
        193537, // Weaponmaster
        231834, // Shield Slam
        238093, // Stave Off
        238112, // Oathblood
        238121, // Pawsitive Outlook
        239042, // Concordance of the Legionfall
        248072  // Chaos Theory
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_CAN_PROC_WITH_TRIGGERED;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_PROC_ONLY_ON_CAST;
    });

    // Bulwark of Order
    ApplySpellFix({209388}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetAuraOptions()->CumulativeAura = 0;
        spellInfo->GetMisc()->MiscData.Attributes[6] |= SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS;
    });
    
    // Icebound Fortitude
    ApplySpellFix({48792}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_3]->ApplyAuraName = SPELL_AURA_ADD_PCT_MODIFIER;
    });

    // Glyph of the Luminous Charger
    ApplySpellFix({126666}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= 0x40;
    });

    // Aura of Sacrifice 
    ApplySpellFix({183416}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_NONE;
        spellInfo->Effects[EFFECT_4]->BasePoints = 0;
    });

    // Gift of the Ox
    ApplySpellFix({124503, 124506, 213458, 213460}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(8); //5yards
    });
   
    // Way of the Crane (Honor Talent), Avenging Crusader (Honor Talent)
    ApplySpellFix({216161, 216371, 216372}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RealPointsPerLevel = 0.f;
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_DONE_BONUS;
    });

    // Feet of Wind, Purification
    ApplySpellFix({240777, 250074}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] |= SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY;
    });

    // Ravager
    ApplySpellFix({227876}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] |= SPELL_ATTR5_START_PERIODIC_AT_APPLY;
    });

    // Castigation
    ApplySpellFix({193134}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = -66;
    });

    // War Banner (Honor Talent)
    ApplySpellFix({236321}, [](SpellInfo* spellInfo)
    {
        for (int i = 0; i < 16; ++i)
        {
            spellInfo->Effects[i]->Effect = SPELL_EFFECT_APPLY_AREA_AURA_RAID;
            spellInfo->Effects[i]->TargetA = TARGET_UNIT_CASTER;
            spellInfo->Effects[i]->RadiusEntry = sSpellRadiusStore.LookupEntry(10);
            spellInfo->Effects[i]->MaxRadiusEntry = sSpellRadiusStore.LookupEntry(10);
        }
    });

    ApplySpellFix({138121, 138123}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 15000;
    });

    // Spatial Rift(Racial)
    ApplySpellFix({257040}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
    });

    // Warlord's Fortitude
    ApplySpellFix({214622}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->ApplyAuraName = SPELL_AURA_DUMMY;
        spellInfo->Effects[EFFECT_2]->TriggerSpell = 0;
    });

    // Sindragosa's Fury
    ApplySpellFix({190778}, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.ChargeCategory = 0;
    });

    // Roll
    ApplySpellFix({107427}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 277;
        spellInfo->Effects[EFFECT_2]->BasePoints = 377;
        spellInfo->Misc.Duration.Duration = 578;
    });

    // Chi Torpedo
    ApplySpellFix({115008}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 310;
        spellInfo->Effects[EFFECT_2]->BasePoints = 410;
        spellInfo->Misc.Duration.Duration = 797;
    });

    ApplySpellFix({232252}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = 138;
    });

    // rock-n-roll
    ApplySpellFix({133204}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(180);//85
    });

    // rock-n-roll
    ApplySpellFix({133202}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 100;
    });

    // Flame On
    ApplySpellFix({205029}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->BasePoints = -17;
    });

    // Ignite
    ApplySpellFix({12654}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->ApplyAuraPeriod = 2000;
    });

    ApplySpellFix({
        148022, // Mastery: Icicles (Damage)
        196364, // Unstable Affliction
        199486, // Psychic Link (Honor Talent)
        235999, // Kil'jaeden's Burning Wish
        118459, // Beast Cleave
        199443, // Avenging Light
        210220, // Holy Wrath
        229980, // Touch of Death
        203286, // Greater Pyroblast
        210141, // Zombie Explosion
        212327, // Cremation
        203728, // Thorns
        213983, // Cold Blood
        204167, // Chill Streak
        206650, // Eye of Leotheras
        206966, // Fel Lance
        203704, // Mana Break
        246867, // Lawbringer
        212620, // Singe Magic
        202127, // Hot Trub
        213688, // Fel Cleave
        212356, // Soulshatter
        235904, // Mana Rift
        252143, // Earth Shock Overload
        256561  // Frost Shock Overload
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[3] |= SPELL_ATTR3_NO_DONE_BONUS;
        spellInfo->GetMisc()->MiscData.Attributes[6] &= ~SPELL_ATTR6_NO_DONE_PCT_DAMAGE_MODS;
    });

    // Stampede
    ApplySpellFix({201631}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CHANNELED_2;
    });

    // blue crush
    ApplySpellFix({212084}, [](SpellInfo* spellInfo)
    {
        spellInfo->InterruptFlags |= 0x0000000D;
        spellInfo->ChannelInterruptFlags[0] |= 0x00003C0C;
    });

    // Draught of Seawalking
    ApplySpellFix({196338}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_4]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_4]->ApplyAuraName = SPELL_AURA_NONE;
    });

    // Helya: Smash
    ApplySpellFix({196534}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_3]->Effect = SPELL_EFFECT_NONE;
    });

    ApplySpellFix({ 241848, 241865 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_TRIGGER_SPELL_4;
        spellInfo->Effects[EFFECT_1]->MiscValue = 300;
    });

    // Solitude (force trigger)
    ApplySpellFix({211508}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(18); //15 yards
    });

    ApplySpellFix({ 203741 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetB = 138;
        spellInfo->Effects[EFFECT_0]->MiscValue = 450;
        spellInfo->Effects[EFFECT_0]->MiscValueB = 450;
    });

    // Wild Transmutations
    ApplySpellFix({
        188800,
        188801,
        188802
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ItemType = 0;
        spellInfo->Effects[EFFECT_0]->Mechanic = MECHANIC_DISCOVERY;
        spellInfo->Categories.Mechanic = MECHANIC_DISCOVERY;
    });

    // Darkmoon Card of the Legion
    ApplySpellFix({
        191659,
        192859,
        192890
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ItemType = 0;
    });

    // Draenor Profession Generate Items
    ApplySpellFix({
        170700,
        156587,
        171690,
        168835
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 8;
    });

    // Mass Mill Draenor Herbs
    ApplySpellFix({
        190383,
        190384,
        190381,
        190385,
        190382,
        190386
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 4;
    });
    
    // Discovery New Recipes 3 lvl for Alchemy
    ApplySpellFix({
        188299,
        188305,
        188302,
        247620,
        188326,
        188335,
        188332,
        188329,
        229218,
        251651,
        188323,
        188314,
        188317,
        188310,
        188308,
        188320,
        247690,
        188350,
        188338,
        188341,
        188347,
        188343
        }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Mechanic = MECHANIC_DISCOVERY;
        spellInfo->Categories.Mechanic = MECHANIC_DISCOVERY;
    });

    ApplySpellFix({ 
        197895, // Focused Thunder
        208535, // Plunder Armor (PvP Talent)
        89751,  // Felstorm (Special Ability)
        115831, // Felstorm (Special Ability)
        203534
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_NONE;
    });

    // Thunder Focus Tea
    ApplySpellFix({116680}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetAuraOptions()->ProcCharges = 1;
        spellInfo->AttributesCu[1] |= SPELL_ATTR1_CU_IS_USING_STACKS;
    });

    // Sudden Doom, Clearcasting (Feral), Predatory Swiftness, Surge of Light
    ApplySpellFix({81340, 135700, 69369, 114255}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[1] |= SPELL_ATTR1_CU_IS_USING_STACKS;
    });

    ApplySpellFix({
        48020,  // Demonic Circle
        36554,  // Shadow Step
        106839, // Skull Bash
        186260, // Harpoon
        92832,  // Leap of Faith
        198793, // Vengeful Retreat
        232893, // Felblade
        101545, // Flying Serpent Kick
        115008, // Chi Torpedo
        189110, // Infernal Strike
        130393, // Blink Strike
        109132  // Roll
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[1] |= SPELL_ATTR1_CU_CANT_USE_WHEN_ROOTED;
    });

    ApplySpellFix({
        115313, // Summon Jade Serpent Statue
        116844, // Ring of Peace
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[1] |= SPELL_ATTR1_CU_USE_PATH_CHECK_FOR_CAST;
    });

    // Warlock
    ApplySpellFix({157902}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 157898;
        spellInfo->Effects[EFFECT_1]->BasePoints = 157757;
    });

    // Flame Rift
    ApplySpellFix({ 243046 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_PERIODIC_DUMMY;
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 500;
    });

    // Stolen Power
    ApplySpellFix({211529}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_CASTER;
    });

    // Word of Glory
    ApplySpellFix({210191}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->TargetA = TARGET_UNIT_CASTER;
    });

    ApplySpellFix({207778}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->TargetB = TARGET_UNIT_DEST_AREA_ALLY;
    });

    // Power Word: Fortitude (Honor Talent), Atonement (Honor Talent)
    ApplySpellFix({211681, 220841, 214206}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_SINGLE_TARGET_SPELL;
    });

    // Ray of Frost
    ApplySpellFix({208141,208166, 221603, 221606, 229908 }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_CANT_CANCEL;
    });

    // Sphere of Insanity
    ApplySpellFix({194200}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 0;
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
    });
    
    ApplySpellFix({167614}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[1]->TargetA = TARGET_UNIT_TARGET_ENEMY;
    });

    ApplySpellFix({1719, // Battle Cry
        13750,  // Adrenaline Rush
        19574,  // Bestial Wrath
        48707,  // Anti-Magic Shell
        190456, // Ignore Pain
        102543, // Incarnation: King of the Jungle
        184362, // Enrage
        101546, // Spinning Crane Kick
        116847, // Rushing Jade Wind
        207982  // Focused Rage
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.DefenseType = SPELL_DAMAGE_CLASS_NONE;
    });

    // Demonic Gateway
    ApplySpellFix({113896, 120729}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[4] &= ~SPELL_ATTR4_TRIGGERED;
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS;
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_USABLE_WHILE_FEARED;
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_USABLE_WHILE_STUNNED;
        spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_USABLE_WHILE_CONFUSED;        
    });

    ApplySpellFix({ 170978 }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[6] |= SPELL_ATTR6_NOT_IN_RAID_INSTANCE;
    });

    ApplySpellFix( // Frigidity of the Tirisgarde and etc.
    {
        241121,
        241124,
        241125,
        241145,
        241146,
        241147,
        241264,
        241269,
        241270,
        241099,
        241100,
        241101,
        241102,
        241018,
        241047,
        241050,
        241110,
        241114,
        241115,
        241148,
        241149,
        241150,
        241152,
        241153,
        241154,
        241202,
        241203,
        241205,
        241257,
        241252,
        241253,
        241131,
        241136,
        241134,
        241090,
        241091
    },
    [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_EXTRA_PVP_STAT_SPELL;
    });

    // Vulnerable
    ApplySpellFix({187131}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[13] &= ~SPELL_ATTR13_PANDEMIA;
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_ALWAYS_RESET_TIMER_AND_TICK;
    });
    
    ApplySpellFix({167138}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_NONE;
    });
    
    ApplySpellFix({167140, 167335, 167996}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
    });

    // Rain from Above
    ApplySpellFix({206959}, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_NOT_ABOVEWATER;
    });

    // Templar's Verdict, Divine Storm
    ApplySpellFix({224266, 224239}, [](SpellInfo* spellInfo)
    {
        spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_BONUS_FROM_ORIGINAL_CASTER;
    });

    // Shroud of Concealment
    ApplySpellFix({115834}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] |= SPELL_ATTR1_CANT_TARGET_IN_COMBAT;
    });

    // Fallen Crusader, Stoneskin Gargoyle
    ApplySpellFix({166441, 62157}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[2] |= SPELL_ATTR2_PRESERVE_ENCHANT_IN_ARENA;
    });

    // Cyclone (Honor Talent)
    ApplySpellFix({33786,209753}, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.Mechanic = MECHANIC_DISORIENTED;
        spellInfo->Effects[EFFECT_0]->Mechanic = MECHANIC_NONE;
        spellInfo->Effects[EFFECT_1]->Mechanic = MECHANIC_NONE;
        spellInfo->Effects[EFFECT_2]->Mechanic = MECHANIC_NONE;
    });
    
    ApplySpellFix({197333}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 3000;
    });
    
    ApplySpellFix({197250}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 70;
        spellInfo->Effects[EFFECT_1]->BasePoints = 70;
    });

    ApplySpellFix({ 209017 }, [](SpellInfo* spellInfo)
    {
        spellInfo->InterruptFlags = 0x00000029;
    });

    ApplySpellFix({
        201871, // Bloodsail Bombardment
        191427, // Metamorphosis
        19574,  // Bestial Wrath
        44614,  // Flurry
        155741  // Dread Raven
    }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
    });
 
    // Scornful Gaze
    ApplySpellFix({240547}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_MOD_STUN;
    });
    
    // felsoul cleave
    ApplySpellFix({236543}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_CONE_ENTRY;
    });

    // Fishing
    ApplySpellFix({131474}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CHANNELED_1;
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CHANNEL_TRACK_TARGET;
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_IS_FISHING;
        spellInfo->GetMisc()->MiscData.Attributes[1] &= ~SPELL_ATTR1_CHANNEL_DISPLAY_SPELL_NAME;
    });
    
    // Liquid Hellfire
    ApplySpellFix({206554}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 206555;
    });
    
    // empowered Liquid Hellf
    ApplySpellFix({206586}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 206581;
    });
    
    // Empowered Eye of Guldan
    ApplySpellFix({221731}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
        spellInfo->Effects[EFFECT_0]->MiscValue = 0;
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = 0;
        spellInfo->Effects[EFFECT_0]->BasePoints = 0;
        spellInfo->Misc.Duration.Duration = 0;
    });
    
    // Eye of Amantul
    ApplySpellFix({227495}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 100; // delay for cast
    });
    
    ApplySpellFix({151120}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 10000;
    });
    
    // hack
    ApplySpellFix({227674, 197214}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = SPELL_EFFECT_NONE;
        spellInfo->Effects[EFFECT_1]->ApplyAuraName = SPELL_AURA_NONE;
    });
    
    // storm of the destroyer
    ApplySpellFix({167819,
        167935,
        177380}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 1; // delay for cast
    });
    
    ApplySpellFix({161121}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
        spellInfo->Misc.Duration.Duration = 2000+2000+2000+4000;
    });
    
    ApplySpellFix({32939, 242983}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 11000;
    });
    
    // Resonant Barrier  (need absorb FULL dmg)
    ApplySpellFix({210296}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 30000000;
    });
    
    ApplySpellFix({206985}, [](SpellInfo* spellInfo)
    {
        for (auto diff : { DIFFICULTY_NONE, DIFFICULTY_NORMAL_RAID, DIFFICULTY_HEROIC_RAID, DIFFICULTY_MYTHIC_RAID, DIFFICULTY_LFR_RAID })
        {
            const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_1, diff))->Effect = SPELL_EFFECT_NONE;
            const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_1, diff))->ApplyAuraName = SPELL_AURA_NONE;
        }
    });

    //Deepening Shadows
    ApplySpellFix({213397}, [](SpellInfo* spellInfo)
    {
        for (auto diff : { DIFFICULTY_NONE, DIFFICULTY_NORMAL, DIFFICULTY_HEROIC, DIFFICULTY_MYTHIC_DUNGEON, DIFFICULTY_MYTHIC_KEYSTONE })
        {
            const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_1, diff))->Effect = SPELL_EFFECT_NONE;
        }
    });

    ApplySpellFix({216961}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TriggerSpell = 208545;
    });

    // Shatter Soul (for Havoc)
    ApplySpellFix({228533, 209651, 237867}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_DEST_CASTER_RANDOM;
    });

    // Shatter Soul (for Vengeance)
    ApplySpellFix({209980, 209981, 210038}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->RadiusEntry = sSpellRadiusStore.LookupEntry(32); //12yards
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_DEST_CASTER_RANDOM;
    });

    // Tightening Grasp
    ApplySpellFix({143375}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(14); //8yards
        spellInfo->Effects[EFFECT_0]->TargetB = TARGET_UNIT_DEST_AREA_ENEMY;
    });

    // Soothing Mist
    ApplySpellFix({115175}, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[4] |= SPELL_ATTR4_NOT_STEALABLE;
    });

    // Death Grip
    ApplySpellFix({51399, 49575, 178305, 178306, 178307}, [](SpellInfo* spellInfo)
    {
        spellInfo->Categories.Mechanic = 0;
    });

    // Living Bomb
    ApplySpellFix({217694}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->BasePoints = 1;
    });

    // Heartstop Aura (Honor Talent)
    ApplySpellFix({214975}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->BasePoints = 0;
    });

    ApplySpellFix({206516}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 100;
        spellInfo->Effects[EFFECT_0]->MiscValue = 100;
    });

    ApplySpellFix({227300}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->BasePoints = 100;
        spellInfo->Effects[EFFECT_1]->MiscValue = 100;
    });
    
    ApplySpellFix({221337}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_DUMMY;
    });
    
    ApplySpellFix({221317}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1] = spellInfo->Effects[EFFECT_0];
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 221382;
    });
    
    ApplySpellFix({206310}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_6]->Effect = SPELL_EFFECT_APPLY_AURA;
        spellInfo->Effects[EFFECT_6]->ApplyAuraName = SPELL_AURA_MOD_STUN;
    });
    
    ApplySpellFix({221434}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->MiscValue = 111222;
    });
    
    ApplySpellFix({206366}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 1500;
    });
    
    ApplySpellFix({226980}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 1000; // delay for cast
    });

    // Player Damage Reduction
    ApplySpellFix({115043}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->BasePoints = 4000;
        spellInfo->GetMisc()->MiscData.Attributes[8] |= SPELL_ATTR8_NOT_IN_BG_OR_ARENA;
        spellInfo->GetMisc()->MiscData.Attributes[4] |= SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG;
    });

    // update for custom pvp mods (7.3.5)
    ApplySpellFix({ 33763, 202497 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 1.5f;
    });

    ApplySpellFix({ 200851 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_3]->PvPMultiplier = 0.1f;
        spellInfo->Effects[EFFECT_4]->PvPMultiplier = 0.1f;
    });

    ApplySpellFix({ 6807 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->PvPMultiplier = 0.53f;
    });

    ApplySpellFix({ 238122 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.57143f;
    });

    ApplySpellFix({ 18562 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 1.4f;
    });

    ApplySpellFix({ 5487, 24858 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->PvPMultiplier = 0.63f;
    });

    ApplySpellFix({ 1178 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->PvPMultiplier = 0.73f;
    });

    ApplySpellFix({ 49998 , 158188 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->PvPMultiplier = 0.5f;
    });

    ApplySpellFix({ 32175, 32176, 115357, 115360, 205414, 222029, 201628, 201789, 193455, 212621, 224239, 224266 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->PvPMultiplier = spellInfo->Effects[EFFECT_0]->PvPMultiplier;
    });

    ApplySpellFix({ 210042, 198533, 209525 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.8f;
    });

    ApplySpellFix({ 11366 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.7f;
    });

    ApplySpellFix({ 228598 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 1.2f;
    });

    // Catlike Reflexes
    ApplySpellFix({197241, 210144, 210145, 210147, 210148, 239290, 239291}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.25f;
    });

    ApplySpellFix({ 195321, 123996, 114089, 114093 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.6f;
    });

    ApplySpellFix({ 202743, 206416 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.5f;
    });

    ApplySpellFix({ 8092 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 1.35f;
    });

    ApplySpellFix({ 15407 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 1.55f;
    });

    ApplySpellFix({ 130493 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.5f;
        spellInfo->Effects[EFFECT_1]->PvPMultiplier = 0.5f;
    });

    ApplySpellFix({ 218571 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->PvPMultiplier = 0.33f;
    });

    ApplySpellFix({238244}, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.MaxDuration = 1000;
    });

    // Argus Dungeon First Boss Channel Visual
    ApplySpellFix({ 246922 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(20);
        spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(4);
        spellInfo->GetAuraOptions()->CumulativeAura = 0;
    });

    ApplySpellFix({241590}, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->ApplyAuraPeriod = 1000;
    });

    ApplySpellFix({ 247174 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->RadiusEntry = sSpellRadiusStore.LookupEntry(9);//20 yards
    });
    
    ApplySpellFix({ 249114 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->Effect = 0;
        spellInfo->Effects[EFFECT_2]->Effect = 0;
    });

    ApplySpellFix({ 232661 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 3000;
    });

    // Lich King Shadow Trap (visual)
    ApplySpellFix({ 73530 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Misc.Duration.Duration = 5000;
    });

    ApplySpellFix({ 235272 }, [](SpellInfo* spellInfo)
    {
        const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_3, DIFFICULTY_NONE))->BasePoints = 30;
        const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_3, DIFFICULTY_MYTHIC_RAID))->BasePoints = 20;

    });

    //Felhounds: Desolate Path
    ApplySpellFix({ 244767 }, [](SpellInfo* spellInfo)
    {
        const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_1, DIFFICULTY_NONE))->BasePoints = 150;
        const_cast<SpellEffectInfo*>(spellInfo->GetEffect(EFFECT_1, DIFFICULTY_NORMAL_RAID))->BasePoints = 150;
    });

    ApplySpellFix({ 233104, 221299 }, [](SpellInfo* spellInfo)
    {
        spellInfo->AuraInterruptFlags[0] = AURA_INTERRUPT_FLAG_CHANGE_MAP;
    });

    ApplySpellFix({ 233435 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_2]->MiscValue = 0;
    });

    // hack bcoz death npc give kill credit!
    ApplySpellFix({ 191546 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->MiscValue = 0;
    });

    ApplySpellFix({ 233431, 241624, 241634 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->TargetA = TARGET_UNIT_TARGET_ENEMY;
        spellInfo->Effects[EFFECT_0]->TargetB = 0;
    });

    ApplySpellFix({ 233430 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_4]->BasePoints = 90.0f;
    });

    ApplySpellFix({ 233652 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_13]->BasePoints = 0;
        spellInfo->Effects[EFFECT_13]->MiscValue = 0;
    });

    ApplySpellFix({ 250879 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_0]->Effect = SPELL_EFFECT_APPLY_AURA;
        spellInfo->Effects[EFFECT_0]->ApplyAuraName = SPELL_AURA_DUMMY;
        spellInfo->GetMisc()->MiscData.Attributes[0] |= SPELL_ATTR0_CANT_USED_IN_COMBAT;
    });

    ApplySpellFix({ 238318 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TriggerSpell = 0;
    });

    ApplySpellFix({ 236571 }, [](SpellInfo* spellInfo)
    {
        spellInfo->GetMisc()->MiscData.Attributes[1] |= SPELL_ATTR1_CHANNEL_TRACK_TARGET;
    });

    ApplySpellFix({ 236973 }, [](SpellInfo* spellInfo)
    {
        spellInfo->Effects[EFFECT_1]->TargetA = TARGET_UNIT_CONE_ENEMY_24;
        spellInfo->Effects[EFFECT_2]->TargetA = TARGET_UNIT_CONE_ENEMY_24;
    });

	// Fixed aura stack bugs like rejuvenation stacking instead of refreshing the buff etc
	// TODO there might be more spells that stack so add more spells onto here if any are found
	ApplySpellFix({ 1079,  // Rip
					774,   // Rejuvenation
					139,   // Renew
					703,   // Garrote
					1943,  // Rupture
					603,   // Doom
					32612, // Invisibility buff
					66,    // Invisibility spell
					136,   // Mend pet
					1604,  // Dazed
					1850   // Dash
		}, [](SpellInfo* spellInfo)
	{
		spellInfo->GetAuraOptions()->ProcCharges = 0;
		spellInfo->GetAuraOptions()->CumulativeAura = 0;
	});

	//40 yards instead of 30 yards to match 198067
	ApplySpellFix({ 188592, // Fire Elemental
					118291, // Primal Fire Elemental
					188616  // Earth Elemental
		}, [](SpellInfo* spellInfo)
		{
			spellInfo->Misc.RangeEntry = sSpellRangeStore.LookupEntry(5);
		});

	// T18 Resto 4P lifebloom 2 targets
	ApplySpellFix({ 188550 }, [](SpellInfo* spellInfo)
		{
			spellInfo->GetMisc()->MiscData.Attributes[5] &= ~SPELL_ATTR5_SINGLE_TARGET_SPELL;
		});

	// Fixed duration for frozen orb mage
	/*ApplySpellFix({ 84721 }, [](SpellInfo* spellInfo)
	{
		spellInfo->Misc.Duration.Duration = 15 * IN_MILLISECONDS;
		spellInfo->Misc.Duration.MaxDuration = 15 * IN_MILLISECONDS;
		spellInfo->GetMisc()->Duration.Duration = 8;
		spellInfo->GetMisc()->Duration.MaxDuration = 8;
	});*/

    for (uint32 i = 0; i < GetSpellInfoStoreSize(); ++i)
    {
        auto const& spellInfo = mSpellInfoMap[i];
        if (!spellInfo)
            continue;

        // If spell not exist, not use him
        if (spellInfo->AuraRestrictions.CasterAuraSpell && !mSpellInfoMap[spellInfo->AuraRestrictions.CasterAuraSpell])
            spellInfo->AuraRestrictions.CasterAuraSpell = 0;
        if (spellInfo->AuraRestrictions.TargetAuraSpell && !mSpellInfoMap[spellInfo->AuraRestrictions.TargetAuraSpell])
            spellInfo->AuraRestrictions.TargetAuraSpell = 0;
        if (spellInfo->AuraRestrictions.ExcludeCasterAuraSpell && !mSpellInfoMap[spellInfo->AuraRestrictions.ExcludeCasterAuraSpell])
            spellInfo->AuraRestrictions.ExcludeCasterAuraSpell = 0;
        if (spellInfo->AuraRestrictions.ExcludeTargetAuraSpell && !mSpellInfoMap[spellInfo->AuraRestrictions.ExcludeTargetAuraSpell])
            spellInfo->AuraRestrictions.ExcludeTargetAuraSpell = 0;

        // Remove feather fall auras on ground
        if (spellInfo->HasAuraInterruptFlag(AURA_INTERRUPT_FLAG2_UNK3) && spellInfo->HasAura(SPELL_AURA_FEATHER_FALL) && !spellInfo->HasAura(SPELL_AURA_HOVER))
            spellInfo->AuraInterruptFlags[0] |= AURA_INTERRUPT_FLAG_LANDING;

        bool noCombat = false;
        for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SpellMgr::LoadSpellCustomAttr Id %u j %u", spellInfo->Id, j);

            switch (spellInfo->Effects[j]->ApplyAuraName)
            {
                //case SPELL_AURA_MOD_POSSESS:
                //case SPELL_AURA_MOD_CHARM:
                //case SPELL_AURA_AOE_CHARM:
                case SPELL_AURA_MOD_STUN:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_AURA_CC;
                case SPELL_AURA_MOD_CONFUSE:
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_ROOTED:
                case SPELL_AURA_MOD_FEAR:
                case SPELL_AURA_MOD_FEAR_2:
                case SPELL_AURA_TRANSFORM:
                    noCombat = true;
                    break;
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_PERIODIC_WEAPON_PERCENT_DAMAGE:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_OBS_MOD_HEALTH:
                case SPELL_AURA_OBS_MOD_POWER:
                case SPELL_AURA_POWER_BURN:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NO_INITIAL_THREAT;
                    break;
                case SPELL_AURA_ACTIVATE_SCENE:
                case SPELL_AURA_OVERRIDE_SPELLS:
                case SPELL_AURA_MOD_TIME_RATE:
                case SPELL_AURA_CREATE_AREATRIGGER:
                case SPELL_AURA_MOD_NEXT_SPELL:
                case SPELL_AURA_CONTROL_VEHICLE: // Can't save vehicle auras, it requires both caster & target to be in world
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CANT_BE_SAVED_IN_DB;
                    break;
                case SPELL_AURA_MOD_STEALTH:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_IS_STEALTH_AURA;
                    break;
                case SPELL_AURA_PROC_MELEE_TRIGGER_SPELL:
                    if (!spellInfo->GetAuraOptions()->ProcTypeMask)
                    {
                        spellInfo->GetAuraOptions()->IsProcAura = true;
                        spellInfo->GetAuraOptions()->ProcChance = 100;
                        spellInfo->GetAuraOptions()->ProcTypeMask = PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS | PROC_FLAG_DONE_MELEE_AUTO_ATTACK;
                    }
                    break;
                default:
                    break;
            }

            switch (spellInfo->Effects[j]->Effect)
            {
                case SPELL_EFFECT_SUMMON:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CANT_BE_SAVED_IN_DB;
                    break;
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                case SPELL_EFFECT_HEAL:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_DIRECT_DAMAGE;
                    break;
                case SPELL_EFFECT_POWER_DRAIN:
                case SPELL_EFFECT_POWER_BURN:
                case SPELL_EFFECT_HEAL_MAX_HEALTH:
                case SPELL_EFFECT_HEALTH_LEECH:
                case SPELL_EFFECT_HEAL_PCT:
                case SPELL_EFFECT_ENERGIZE_PCT:
                case SPELL_EFFECT_ENERGIZE:
                case SPELL_EFFECT_HEAL_MECHANICAL:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NO_INITIAL_THREAT;
                    break;
                case SPELL_EFFECT_CHARGE:
                case SPELL_EFFECT_CHARGE_DEST:
                case SPELL_EFFECT_JUMP:
                case SPELL_EFFECT_JUMP_DEST:
                case SPELL_EFFECT_LEAP_BACK:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CHARGE;
                    break;
                case SPELL_EFFECT_PICKPOCKET:
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_PICKPOCKET;
                    break;
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC:
                case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                {
                    // only enchanting profession enchantments procs can stack
                    if (IsPartOfSkillLine(SKILL_ENCHANTING, i))
                    {
                        uint32 enchantId = spellInfo->Effects[j]->MiscValue;
                        SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);
                        for (uint8 s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
                        {
                            if (enchant->Effect[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                continue;

                            auto procInfo = const_cast<SpellInfo*>(GetSpellInfo(enchant->EffectArg[s]));
                            if (!procInfo)
                                continue;

                            // if proced directly from enchantment, not via proc aura
                            // NOTE: Enchant Weapon - Blade Ward also has proc aura spell and is proced directly
                            // however its not expected to stack so this check is good
                            if (procInfo->HasAura(SPELL_AURA_PROC_TRIGGER_SPELL))
                                continue;

                            procInfo->AttributesCu[0] |= SPELL_ATTR0_CU_ENCHANT_PROC;
                        }
                    }
                    break;
                }
                case SPELL_EFFECT_CREATE_ITEM:
                case SPELL_EFFECT_CREATE_ITEM_2:
                    mSpellCreateItemList.push_back(i);
                    break;
                default:
                    break;
            }
        }

        if (noCombat && !(spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_DIRECT_DAMAGE) && !(spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_NO_INITIAL_THREAT))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NO_COMBAT;

        if (!spellInfo->_IsPositiveEffect(EFFECT_0, false))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF0;

        if (!spellInfo->_IsPositiveEffect(EFFECT_1, false))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF1;

        if (!spellInfo->_IsPositiveEffect(EFFECT_2, false))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF2;

        if (!spellInfo->_IsPositiveEffect(EFFECT_3, false))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF3;

        if (!spellInfo->_IsPositiveEffect(EFFECT_4, false))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_NEGATIVE_EFF4;

        if (spellInfo->HasEffect(SPELL_EFFECT_APPLY_AREA_AURA_ENEMY))
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_POSITIVE_FOR_CASTER;

        if (spellInfo->GetSpellVisual() == 3879)
            spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_CONE_BACK;

        switch (spellInfo->ClassOptions.SpellClassSet)
        {
            case SPELLFAMILY_DEMON_HUNTER:
            {
                if (spellInfo->Id == 217832)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                if (spellInfo->Id == 20066)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                if (spellInfo->Id == 213691)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                if (spellInfo->ClassOptions.SpellClassMask.HasFlag(8))
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                if (spellInfo->ClassOptions.SpellClassMask.HasFlag(16777216))
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                break;
            }
            case SPELLFAMILY_MAGE:
            {
                if (spellInfo->ClassOptions.SpellClassMask.HasFlag(16777216))
                {
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                    spellInfo->GetAuraOptions()->ProcTypeMask &= ~0x8000;
                }
                if (spellInfo->Id == 82691)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                break;
            }
            case SPELLFAMILY_MONK:
            {
                if (spellInfo->ClassOptions.SpellClassMask.HasFlag(0, 8))
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                break;
            }
            case SPELLFAMILY_WARRIOR:
                // Shout
                if (spellInfo->ClassOptions.SpellClassMask[0] & 0x20000 || spellInfo->ClassOptions.SpellClassMask[1] & 0x20)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_AURA_CC;
                break;
            case SPELLFAMILY_DRUID:
            {
                if (spellInfo->Id == 236025)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE;
                // Roar
                if (spellInfo->ClassOptions.SpellClassMask[0] & 0x8)
                    spellInfo->AttributesCu[0] |= SPELL_ATTR0_CU_AURA_CC;
                break;
            }
            default:
                break;
        }

        // This must be re-done if targets changed since the spellinfo load
        spellInfo->ExplicitTargetMask = spellInfo->_GetExplicitTargetMask();

        switch (spellInfo->Id)
        {
            case 107223:
                spellInfo->ExplicitTargetMask = TARGET_FLAG_UNIT_MASK;
                break;
            case 106736:
                spellInfo->ExplicitTargetMask = TARGET_FLAG_UNIT_MASK;
                break;
            case 106112:
                spellInfo->ExplicitTargetMask |= TARGET_FLAG_DEST_LOCATION;
                break;
            case 106113:
                spellInfo->ExplicitTargetMask = TARGET_FLAG_UNIT_MASK;
                break;
            default:
                break;
        }

        if (spellInfo->HasAttribute(SPELL_ATTR0_TRADESPELL))
        {
            auto bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellInfo->Id);
            for (auto itr = bounds.first; itr != bounds.second; ++itr)
                _skillTradeSpells[itr->second->SkillLine].push_back(itr->second);
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded spell custom attributes in %u ms", GetMSTimeDiffToNow(oldMSTime));

    oldMSTime = getMSTime();
    //                                                   0            1             2              3              4              5
    auto result = WorldDatabase.Query("SELECT spell_id, effectradius0, effectradius1, effectradius2, effectradius3, effectradius4 from spell_radius");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 spell effect radius records. DB table `spell_radius` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        auto fields = result->Fetch();

        auto spellId = fields[0].GetUInt32();
        if (spellId >= GetSpellInfoStoreSize())
            continue;

        auto spellInfo2 = mSpellInfoMap[spellId];
        if (!spellInfo2)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "spellId %u in `spell_radius` table is not found in dbcs, skipped", spellId);
            continue;
        }

        if (auto entry = sSpellRadiusStore[fields[1].GetUInt32()])
            spellInfo2->Effects[EFFECT_0]->RadiusEntry = entry;
        if (auto entry = sSpellRadiusStore[fields[2].GetUInt32()])
            spellInfo2->Effects[EFFECT_1]->RadiusEntry = entry;
        if (auto entry = sSpellRadiusStore[fields[3].GetUInt32()])
            spellInfo2->Effects[EFFECT_2]->RadiusEntry = entry;
        if (auto entry = sSpellRadiusStore[fields[4].GetUInt32()])
            spellInfo2->Effects[EFFECT_3]->RadiusEntry = entry;
        if (auto entry = sSpellRadiusStore[fields[5].GetUInt32()])
            spellInfo2->Effects[EFFECT_4]->RadiusEntry = entry;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u spell effect radius records in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void SpellMgr::LoadTalentSpellInfo()
{
    for (TalentEntry const* talent : sTalentStore)
        mTalentSpellInfo.insert(talent->SpellID);
}

void SpellMgr::LoadSpellInfoSpellSpecificAndAuraState()
{
    uint32 oldMSTime = getMSTime();

    for (auto const& spellInfo : mSpellInfoMap)
    {
        if (!spellInfo)
            continue;

        spellInfo->_LoadSpellSpecific();
        spellInfo->_LoadAuraState();
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded SpellInfo SpellSpecific and AuraState in %u ms", GetMSTimeDiffToNow(oldMSTime));
}