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

#include "Totem.h"
#include "Group.h"
#include "Player.h"
#include "SpellMgr.h"
#include "SpellInfo.h"

Totem::Totem(SummonPropertiesEntry const* properties, Unit* owner) : Minion(properties, owner, false)
{
    m_unitTypeMask |= UNIT_MASK_TOTEM;
    m_duration = 0;
    m_type = TOTEM_PASSIVE;
}

void Totem::Update(uint32 time)
{
    if (m_isUpdate)
        return;

    m_isUpdate = true;

    if (!m_owner->isAlive() || !isAlive())
    {
        UnSummon();                                         // remove self
        m_isUpdate = false;
        return;
    }

    if (m_duration <= time)
    {
        UnSummon();                                         // remove self
        m_isUpdate = false;
        return;
    }
    m_duration -= time;

    m_isUpdate = false;
    Creature::Update(time);
}

void Totem::InitStats(uint32 duration)
{
    bool damageSet = false;
    uint32 spellId = GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL);

    if (m_owner->IsPlayer() && m_Properties->Slot >= SUMMON_SLOT_TOTEM && m_Properties->Slot < MAX_TOTEM_SLOT)
    {
        // Totemic Encirclement
        if (m_owner->HasAura(58057)
            && spellId != 120214
            && spellId != 120217
            && spellId != 120218
            && spellId != 120219)
        {
            for (int i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
            {
                if (i == m_Properties->Slot)
                    continue;
                if(m_owner->m_SummonSlot[i])
                    if(GetMap()->GetCreature(m_owner->m_SummonSlot[i]))
                        continue;
                switch (i)
                {
                    case 1:
                        m_owner->CastSpell(m_owner, 120217, true); // Fake Fire Totem
                        break;
                    case 2:
                        m_owner->CastSpell(m_owner, 120218, true); // Fake Earth Totem
                        break;
                    case 3:
                        m_owner->CastSpell(m_owner, 120214, true); // Fake Water Totem
                        break;
                    case 4:
                        m_owner->CastSpell(m_owner, 120219, true); // Fake Wind Totem
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // set display id depending on caster's race
    if (uint32 display = m_owner->GetModelForTotem(spellId))
        SetDisplayId(display);

    Minion::InitStats(duration);

    // Get spell cast by totem
    if (SpellInfo const* totemSpell = sSpellMgr->GetSpellInfo(GetSpell()))
        if (totemSpell->CalcCastTime())   // If spell has cast time -> its an active totem
            m_type = TOTEM_ACTIVE;

    if (GetEntry() == SENTRY_TOTEM_ENTRY)
        SetReactState(REACT_AGGRESSIVE);

    m_duration = duration;

    SetLevel(m_owner->getLevel());
    SetEffectiveLevel(m_owner->GetEffectiveLevel());

    InitBaseStat(GetEntry(), damageSet);
}

void Totem::InitSummon()
{
    if (m_type == TOTEM_PASSIVE && GetSpell())
        CastSpell(this, GetSpell(), true);

    // Some totems can have both instant effect and passive spell
    if (GetSpell(1))
        CastSpell(this, GetSpell(1), true);
}

void Totem::UnSummon(uint32 msTime)
{
    if (msTime)
    {
        m_Events.AddEvent(new ForcedUnsummonDelayEvent(*this), m_Events.CalculateTime(msTime));
        return;
    }

    CombatStop();
    RemoveAurasDueToSpell(GetSpell(), GetGUID());
    CastPetAuras(false);

    // clear owner's totem slot
    for (int i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
    {
        if (m_owner->m_SummonSlot[i] == GetGUID())
        {
            m_owner->m_SummonSlot[i].Clear();
            break;
        }
    }

    m_owner->RemoveAurasDueToSpell(GetSpell(), GetGUID());

    // Remove Sentry Totem Aura
    if (GetEntry() == SENTRY_TOTEM_ENTRY)
        m_owner->RemoveAurasDueToSpell(SENTRY_TOTEM_SPELLID);

    //remove aura all party members too
    if (Player* owner = m_owner->ToPlayer())
    {
        owner->SendCancelAutoRepeat(this);

        if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL)))
            owner->SendCooldownEvent(spell, 0, nullptr, false);

        if (Group* group = owner->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* target = itr->getSource();
                if (target && group->SameSubGroup(owner, target))
                    target->RemoveAurasDueToSpell(GetSpell(), GetGUID());
            }
        }
    }

    AddObjectToRemoveList();
}

bool Totem::IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const
{
    // TODO: possibly all negative auras immune?
    if (GetEntry() == 5925)
        return false;

    switch (spellInfo->Effects[index]->ApplyAuraName)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_MOD_FEAR_2:
        case SPELL_AURA_TRANSFORM:
            return true;
        default:
            break;
    }

    return Creature::IsImmunedToSpellEffect(spellInfo, index);
}
