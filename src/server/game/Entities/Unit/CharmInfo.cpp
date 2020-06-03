/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "CharmInfo.h"
#include "Unit.h"
#include "MoveSpline.h"

UnitActionBarEntry::UnitActionBarEntry(): packedData(uint32(ACT_DISABLED) << 24)
{
}

ActiveStates UnitActionBarEntry::GetType() const
{
    return ActiveStates(UNIT_ACTION_BUTTON_TYPE(packedData));
}

uint32 UnitActionBarEntry::GetAction() const
{
    return UNIT_ACTION_BUTTON_ACTION(packedData);
}

bool UnitActionBarEntry::IsActionBarForSpell() const
{
    ActiveStates Type = GetType();
    return Type == ACT_DISABLED || Type == ACT_ENABLED || Type == ACT_PASSIVE;
}

void UnitActionBarEntry::SetActionAndType(uint32 action, ActiveStates type)
{
    packedData = MAKE_UNIT_ACTION_BUTTON(action, type);
}

void UnitActionBarEntry::SetType(ActiveStates type)
{
    packedData = MAKE_UNIT_ACTION_BUTTON(UNIT_ACTION_BUTTON_ACTION(packedData), type);
}

void UnitActionBarEntry::SetAction(uint32 action)
{
    packedData = (packedData & 0xFF000000) | UNIT_ACTION_BUTTON_ACTION(action);
}

CharmInfo::CharmInfo(Unit* unit) : m_unit(unit), m_CommandState(COMMAND_FOLLOW), m_petnumber(0), m_isCommandAttack(false), m_isAtStay(false), m_isFollowing(false), m_isReturning(false)
{
    for (auto& itr : m_charmspells)
        itr.SetActionAndType(0, ACT_DISABLED);

    if (m_unit->IsCreature())
    {
        m_oldReactState = m_unit->ToCreature()->GetReactState();
        m_unit->ToCreature()->SetReactState(REACT_PASSIVE);
    }
}

CharmInfo::~CharmInfo() = default;

void CharmInfo::RestoreState()
{
    if (m_unit->IsCreature())
        if (Creature* creature = m_unit->ToCreature())
            creature->SetReactState(m_oldReactState);
}

uint32 CharmInfo::GetPetNumber() const
{
    return m_petnumber;
}

void CharmInfo::InitPetActionBar()
{
    for (uint32 i = 0; i < ACTION_BAR_INDEX_PET_SPELL_START - ACTION_BAR_INDEX_START; ++i)
    {
        if (i < 2)
            SetActionBar(ACTION_BAR_INDEX_START + i, COMMAND_ATTACK - i, ACT_COMMAND);
        else
            SetActionBar(ACTION_BAR_INDEX_START + i, COMMAND_MOVE_TO, ACT_COMMAND);
    }

    for (uint32 i = 0; i < ACTION_BAR_INDEX_PET_SPELL_END - ACTION_BAR_INDEX_PET_SPELL_START; ++i)
        SetActionBar(ACTION_BAR_INDEX_PET_SPELL_START + i, 0, ACT_PASSIVE);

    SetActionBar(ACTION_BAR_INDEX_PET_SPELL_END + 0, REACT_HELPER, ACT_REACTION);
    SetActionBar(ACTION_BAR_INDEX_PET_SPELL_END + 1, REACT_DEFENSIVE, ACT_REACTION);
    SetActionBar(ACTION_BAR_INDEX_PET_SPELL_END + 2, REACT_PASSIVE, ACT_REACTION);
}

void CharmInfo::InitEmptyActionBar(bool withAttack)
{
    if (withAttack)
        SetActionBar(ACTION_BAR_INDEX_START, COMMAND_ATTACK, ACT_COMMAND);
    else
        SetActionBar(ACTION_BAR_INDEX_START, 0, ACT_PASSIVE);

    for (uint32 x = ACTION_BAR_INDEX_START + 1; x < ACTION_BAR_INDEX_END; ++x)
        SetActionBar(x, 0, ACT_PASSIVE);
}

CommandStates CharmInfo::GetCommandState() const
{
    return m_CommandState;
}

bool CharmInfo::HasCommandState(CommandStates state) const
{
    return m_CommandState == state;
}

void CharmInfo::InitPossessCreateSpells()
{
    InitEmptyActionBar();

    if (!m_unit->IsCreature())
        return;

    if (auto creature = m_unit->ToCreature())
    {
        // TODO: remove all spells and passives in instances after 70 lvl - bugged creature spells
        if (creature->getLevel() >= 70 && creature->GetMap() && creature->GetMap()->Instanceable())
            return;

        for (auto itr : creature->m_templateSpells)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr);
            if (spellInfo && !spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD))
            {
                if (spellInfo->IsPassive())
                    m_unit->CastSpell(m_unit, spellInfo, true);
                else
                    AddSpellToActionBar(spellInfo, ACT_PASSIVE);
            }
        }
    }
}

void CharmInfo::InitCharmCreateSpells()
{
    if (m_unit->IsPlayer())
    {
        InitEmptyActionBar();
        return;
    }

    // TODO: remove all spells and passives in instances after 70 lvl - bugged creature spells
    if (Creature* creature = m_unit->ToCreature())
    {
        if (creature->getLevel() >= 70 && creature->GetMap() && creature->GetMap()->Instanceable() && (!m_unit->GetOwner()->IsPlayer() && !m_unit->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN)))
        {
            InitEmptyActionBar();
            return;
        }
    }

    InitPetActionBar();

    for (uint32 x = 0; x < std::extent<decltype(m_charmspells)>::value; ++x)
    {
        uint32 spellId = m_unit->ToCreature()->m_templateSpells[x];
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

        if (!spellInfo || spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD))
        {
            m_charmspells[x].SetActionAndType(spellId, ACT_DISABLED);
            continue;
        }

        if (spellInfo->IsPassive())
        {
            m_unit->CastSpell(m_unit, spellInfo, true);
            m_charmspells[x].SetActionAndType(spellId, ACT_PASSIVE);
        }
        else
        {
            m_charmspells[x].SetActionAndType(spellId, ACT_DISABLED);

            ActiveStates newstate = ACT_PASSIVE;

            if (!spellInfo->IsAutocastable())
                newstate = ACT_PASSIVE;
            else
            {
                if (spellInfo->NeedsExplicitUnitTarget())
                {
                    newstate = ACT_ENABLED;
                    ToggleCreatureAutocast(spellInfo, true);
                }
                else
                    newstate = ACT_DISABLED;
            }

            AddSpellToActionBar(spellInfo, newstate);
        }
    }
}

bool CharmInfo::AddSpellToActionBar(SpellInfo const* spellInfo, ActiveStates newstate)
{
    if (spellInfo->HasAttribute(SPELL_ATTR0_HIDDEN_CLIENTSIDE) || spellInfo->HasAttribute(SPELL_ATTR4_HIDDEN_SPELLBOOK))
        return false;

    // pet cannot summon mob
    if (spellInfo->Id != 49297)
    {
        for (auto j = 0; j < std::extent<decltype(spellInfo->Effects)>::value; ++j)
        {
            if (spellInfo->EffectMask < uint32(1 << j))
                break;

            switch (spellInfo->Effects[j]->Effect)
            {
                case SPELL_EFFECT_SUMMON:
                    return false;
                default:
                    break;
            }
        }
    }

    uint32 spell_id = spellInfo->Id;
    uint32 first_id = spellInfo->GetFirstRankSpell()->Id;

    for (auto& i : PetActionBar)
    {
        if (uint32 action = i.GetAction())
        {
            if (i.IsActionBarForSpell() && sSpellMgr->GetFirstSpellInChain(action) == first_id)
            {
                i.SetAction(spell_id);
                return true;
            }
        }
    }

    for (uint8 i = 0; i < std::extent<decltype(PetActionBar)>::value; ++i)
    {
        if (!PetActionBar[i].GetAction() && PetActionBar[i].IsActionBarForSpell())
        {
            SetActionBar(i, spell_id, newstate == ACT_DECIDE ? spellInfo->IsAutocastable() ? ACT_DISABLED : ACT_PASSIVE : newstate);
            return true;
        }
    }
    return false;
}

bool CharmInfo::RemoveSpellFromActionBar(uint32 spell_id)
{
    uint32 first_id = sSpellMgr->GetFirstSpellInChain(spell_id);

    for (uint8 i = 0; i < std::extent<decltype(PetActionBar)>::value; ++i)
    {
        if (uint32 action = PetActionBar[i].GetAction())
        {
            if (PetActionBar[i].IsActionBarForSpell() && sSpellMgr->GetFirstSpellInChain(action) == first_id)
            {
                SetActionBar(i, 0, ACT_PASSIVE);
                return true;
            }
        }
    }

    return false;
}

void CharmInfo::ToggleCreatureAutocast(SpellInfo const* spellInfo, bool apply)
{
    if (spellInfo->IsPassive())
        return;

    for (auto& itr : m_charmspells)
        if (spellInfo->Id == itr.GetAction())
            itr.SetType(apply ? ACT_ENABLED : ACT_DISABLED);
}

CharmSpellInfo* CharmInfo::GetCharmSpell(uint8 index)
{
    return &m_charmspells[index];
}

void CharmInfo::SetPetNumber(uint32 petnumber, bool statwindow)
{
    m_petnumber = petnumber;

    if (statwindow)
        m_unit->SetUInt32Value(UNIT_FIELD_PET_NUMBER, m_petnumber);
    else
        m_unit->SetUInt32Value(UNIT_FIELD_PET_NUMBER, 0);
}

void CharmInfo::SetCommandState(CommandStates st)
{
    m_CommandState = st;
}

void CharmInfo::LoadPetActionBar(std::string const& data)
{
    InitPetActionBar();

    Tokenizer tokens(data, ' ');
    if (tokens.size() != (ACTION_BAR_INDEX_END - ACTION_BAR_INDEX_START) * 2)
        return;

    auto iter = tokens.begin();
    for (uint8 index = ACTION_BAR_INDEX_START; index < ACTION_BAR_INDEX_END; ++iter, ++index)
    {
        auto type = ActiveStates(atol(*iter));
        ++iter;
        auto action = uint32(atol(*iter));

        PetActionBar[index].SetActionAndType(action, type);

        if (PetActionBar[index].IsActionBarForSpell())
        {
            SpellInfo const* spelInfo = sSpellMgr->GetSpellInfo(PetActionBar[index].GetAction());
            if (!spelInfo)
                SetActionBar(index, 0, ACT_PASSIVE);
            else if (!spelInfo->IsAutocastable())
                SetActionBar(index, PetActionBar[index].GetAction(), ACT_PASSIVE);
        }
    }
}

void CharmInfo::SetSpellAutocast(SpellInfo const* spellInfo, bool state)
{
    for (auto& i : PetActionBar)
    {
        if (spellInfo->Id == i.GetAction() && i.IsActionBarForSpell())
        {
            i.SetType(state ? ACT_ENABLED : ACT_DISABLED);
            break;
        }
    }
}

void CharmInfo::SetActionBar(uint8 index, uint32 spellOrAction, ActiveStates type)
{
    PetActionBar[index].SetActionAndType(spellOrAction, type);
}

UnitActionBarEntry const* CharmInfo::GetActionBarEntry(uint8 index) const
{
    return &PetActionBar[index];
}

void CharmInfo::SetIsCommandAttack(bool val)
{
    m_isCommandAttack = val;
}

bool CharmInfo::IsCommandAttack()
{
    return m_isCommandAttack;
}

void CharmInfo::SaveStayPosition()
{
    //! At this point a new spline destination is enabled because of Unit::StopMoving()
    G3D::Vector3 stayPos = m_unit->movespline->FinalDestination();

    if (m_unit->movespline->onTransport)
        if (TransportBase* transport = m_unit->GetDirectTransport())
            transport->CalculatePassengerPosition(stayPos.x, stayPos.y, stayPos.z);

    _stay = Position(stayPos.x, stayPos.y, stayPos.z);
}

void CharmInfo::GetStayPosition(float &x, float &y, float &z)
{
    x = _stay.GetPositionX();
    y = _stay.GetPositionY();
    z = _stay.GetPositionZ();
}

void CharmInfo::SetIsAtStay(bool val)
{
    m_isAtStay = val;
}

bool CharmInfo::IsAtStay()
{
    return m_isAtStay;
}

void CharmInfo::SetIsFollowing(bool val)
{
    m_isFollowing = val;
}

bool CharmInfo::IsFollowing()
{
    return m_isFollowing;
}

void CharmInfo::SetIsReturning(bool val)
{
    m_isReturning = val;
}

bool CharmInfo::IsReturning()
{
    return m_isReturning;
}
