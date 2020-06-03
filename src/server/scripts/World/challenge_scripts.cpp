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

#include "ChallengeMgr.h"
#include "GameTables.h"
#include "ScenarioMgr.h"
#include "ScriptMgr.h"

/// 206150 - Challenger's Might
class spell_challengers_might : public AuraScript
{
    PrepareAuraScript(spell_challengers_might);

    uint8 volcanicTimer = 0;
    uint32 felExplosivesTimer = 0;
    uint32 necroticProcDelay = 0;

    void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
    {
        auto caster = GetCaster();

        Scenario* progress = sScenarioMgr->GetScenario(caster->GetInstanceId());
        if (!progress)
            return;

        auto const& challenge = progress->GetChallenge();
        if (!challenge)
            return;

        uint32 challengeLevel = challenge->GetChallengeLevel();
        GtChallengeModeHealthEntry const* gtHealth = sChallengeModeHealthTable.GetRow(challengeLevel);
        GtChallengeModeDamageEntry const* gtDamage = sChallengeModeDamageTable.GetRow(challengeLevel);
        if (!gtHealth || !gtDamage)
            return;

        float modHealth = gtHealth->Scalar;
        float modDamage = gtDamage->Scalar;

        bool isDungeonBoss = false;
        auto creature = caster->ToCreature();
        if (creature->IsDungeonBoss())
            isDungeonBoss = true;

        if (isDungeonBoss)
        {
            if (challenge->HasAffix(Affixes::Tyrannical))
            {
                modHealth *= 1.4f;
                modDamage *= 1.15f;
            }
        }
        else if (challenge->HasAffix(Affixes::Fortified))
        {
            modHealth *= 1.2f;
            modDamage *= 1.3f;
        }

        modHealth = (modHealth - 1.0f) * 100.0f;
        modDamage = (modDamage - 1.0f) * 100.0f;

        switch (aurEff->GetEffIndex())
        {
            case EFFECT_0:
                amount = modHealth;
                break;
            case EFFECT_1:
                amount = modDamage;
                break;
            case EFFECT_2:
                if (creature->IsAffixDisabled(Affixes::Raging))
                    amount = 0;
                else
                    amount = (challenge->HasAffix(Affixes::Raging) && !isDungeonBoss) ? 1 : 0;
                break;
            case EFFECT_3:
                if (creature->IsAffixDisabled(Affixes::Bolstering))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Bolstering) ? 1 : 0;
                break;
            case EFFECT_4:
                if (creature->IsAffixDisabled(Affixes::Tyrannical))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Tyrannical) && isDungeonBoss ? 1 : 0;
                break;
            case EFFECT_7:
                if (creature->IsAffixDisabled(Affixes::Volcanic))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Volcanic) ? 1 : 0;
                break;
            case EFFECT_8:
                if (creature->IsAffixDisabled(Affixes::Necrotic))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Necrotic) ? 1 : 0;
                break;
            case EFFECT_9:
                if (creature->IsAffixDisabled(Affixes::Fortified))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Fortified) && !isDungeonBoss ? 1 : 0;
                break;
            case EFFECT_10:
                if (creature->IsAffixDisabled(Affixes::Sanguine))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Sanguine) ? 1 : 0;
                break;
            case EFFECT_11:
                if (creature->IsAffixDisabled(Affixes::Quaking))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Quaking) ? 1 : 0;
                break;
            case EFFECT_12:
                if (creature->IsAffixDisabled(Affixes::FelExplosives))
                    amount = 0;
                else
                {
                    amount = challenge->HasAffix(Affixes::FelExplosives) ? 1 : 0;
                    felExplosivesTimer = urandms(2, 10);
                }
                break;
            case EFFECT_13:
                if (creature->IsAffixDisabled(Affixes::Bursting))
                    amount = 0;
                else
                    amount = challenge->HasAffix(Affixes::Bursting) ? 1 : 0;
                break;
            default:
                break;
        }
    }

    // Volcanic
    void OnTick(AuraEffect const* aurEff)
    {
        if (!aurEff->GetAmount() || !GetCaster()->isInCombat())
            return;

        if (volcanicTimer == 7)
            volcanicTimer = 0;
        else
        {
            ++volcanicTimer;
            return;
        }

        auto caster = GetCaster()->ToCreature();
        if (!caster || caster->IsAffixDisabled(Affixes::Volcanic) || (caster->AI() && caster->AI()->IsInControl()) || !caster->IsHostileToPlayers())
            return;

        if (auto owner = caster->GetAnyOwner())
        {
            if (owner->IsPlayer())
                return;

            if (owner->IsCreature() && !owner->ToCreature()->IsDungeonBoss())
                return;
        }

        auto _map = caster->GetMap();
        if (!_map)
            return;

        Map::PlayerList const& players = _map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            if (auto player = itr->getSource())
            {
                if (player->isInCombat() && player->IsValidAttackTarget(caster))
                    if (caster->GetDistance(player) > 15.0f && caster->GetDistance(player) < 60.0f) // Offlike 10m
                        caster->CastSpell(player, ChallengerSummonVolcanicPlume, true);
            }
        }
    }

    // Necrotic
    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        if (necroticProcDelay)
            PreventDefaultAction();
        else
            necroticProcDelay = 1000;
    }

    // Fel Explosives
    void OnUpdate(uint32 diff, AuraEffect* aurEff)
    {
        if (necroticProcDelay)
        {
            if (necroticProcDelay <= diff)
                necroticProcDelay = 0;
            else
                necroticProcDelay -= diff;
        }

        if (!aurEff->GetAmount() || aurEff->GetEffIndex() != EFFECT_12 || !GetCaster()->isInCombat())
            return;

        if (felExplosivesTimer <= diff)
            felExplosivesTimer = urandms(8, 16);
        else
        {
            felExplosivesTimer -= diff;
            return;
        }

        auto caster = GetCaster()->ToCreature();
        if (!caster || caster->IsAffixDisabled(Affixes::FelExplosives) || (caster->AI() && caster->AI()->IsInControl()) || !caster->IsHostileToPlayers())
            return;

        if (auto owner = caster->GetAnyOwner())
        {
            if (owner->IsPlayer())
                return;

            if (owner->IsCreature() && !owner->ToCreature()->IsDungeonBoss())
                return;
        }

        auto _map = caster->GetMap();
        if (!_map)
            return;

        Map::PlayerList const& players = _map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            if (auto player = itr->getSource())
            {
                if (player->isInCombat() && player->IsValidAttackTarget(caster))
                    if (caster->GetDistance(player) < 60.0f)
                    {
                        caster->CastSpell(caster, SPELL_FEL_EXPLOSIVES_SUMMON_2, true);
                        return;
                    }
            }
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_2, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_3, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_4, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_7, SPELL_AURA_PERIODIC_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_8, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_9, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_10, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_11, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_12, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_might::CalculateAmount, EFFECT_13, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_challengers_might::OnTick, EFFECT_7, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_challengers_might::OnProc, EFFECT_8, SPELL_AURA_DUMMY);
        OnEffectUpdate += AuraEffectUpdateFn(spell_challengers_might::OnUpdate, EFFECT_11, SPELL_AURA_DUMMY);
        OnEffectUpdate += AuraEffectUpdateFn(spell_challengers_might::OnUpdate, EFFECT_12, SPELL_AURA_DUMMY);
    }
};

/// 206151 - Challenger's Burden
class spell_challengers_burden : public AuraScript
{
    PrepareAuraScript(spell_challengers_burden);

    void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        auto player = caster->ToPlayer();
        if (!player)
            return;

        auto progress = sScenarioMgr->GetScenario(caster->GetInstanceId());
        if (!progress)
            return;

        auto const& challenge = progress->GetChallenge();
        if (!challenge)
            return;

        switch (aurEff->GetEffIndex())
        {
            case EFFECT_1:
                amount = challenge->HasAffix(Affixes::Overflowing);
                break;
            case EFFECT_2:
                amount = challenge->HasAffix(Affixes::Skittish) && player->isInTankSpec();
                break;
            case EFFECT_3:
                amount = challenge->HasAffix(Affixes::Grievous);
                break;
            default:
                break;
        }
    }

    // Grievous
    void OnTick(AuraEffect const* aurEff)
    {
        if (!aurEff->GetAmount() || !GetUnitOwner() || !GetUnitOwner()->isAlive())
            return;

        if (GetUnitOwner()->HealthBelowPct(90))
        {
            if (!GetUnitOwner()->HasAura(ChallengerGrievousWound))
                GetUnitOwner()->CastSpell(GetUnitOwner(), ChallengerGrievousWound, true);
        }
        else
            GetUnitOwner()->RemoveAurasDueToSpell(ChallengerGrievousWound);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_burden::CalculateAmount, EFFECT_1, SPELL_AURA_DUMMY);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_challengers_burden::CalculateAmount, EFFECT_2, SPELL_AURA_MOD_THREAT);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_challengers_burden::OnTick, EFFECT_3, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//240443
class spell_challengers_burst : public AuraScript
{
    PrepareAuraScript(spell_challengers_burst);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(243237))
        {
            float bp = spellInfo->GetEffect(EFFECT_0)->BasePoints * target->GetAuraCount(GetId());
            target->CastCustomSpell(target, 243237, &bp, NULL, NULL, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_challengers_burst::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//240559
class spell_challengers_grievous_wound : public AuraScript
{
    PrepareAuraScript(spell_challengers_grievous_wound);

    void OnTick(AuraEffect const* aurEff)
    {
        if (auto aura = aurEff->GetBase())
            aura->ModStackAmount(1);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_challengers_grievous_wound::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
    }
};

// 209859 - Bolster
class spell_challengers_bolster : public SpellScript
{
    PrepareSpellScript(spell_challengers_bolster);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        std::list<WorldObject*> temp;
        for (auto object : targets)
        {
            if (auto unit = object->ToUnit())
            {
                if (!unit->isInCombat() || unit->IsPlayer())
                    continue;

                auto owner = unit->GetAnyOwner();
                if (owner && owner->IsPlayer())
                    continue;

                if (Creature* creature = unit->ToCreature())
                    if (creature->IsDungeonBoss())
                        continue;

                temp.push_back(object);
            }
        }
        targets = temp;
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_challengers_bolster::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_challengers_bolster::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_challengers_bolster::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_challengers_bolster::FilterTargets, EFFECT_3, TARGET_UNIT_SRC_AREA_ALLY);
    }
};

// 105877 - Volcanic Plume
struct npc_challenger_volcanic_plume : ScriptedAI
{
    npc_challenger_volcanic_plume(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        events.Reset();
        events.ScheduleEvent(EVENT_1, 250);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                    me->CastSpell(me, ChallengerVolcanicPlume, false);
                    break;
                default:
                    break;
            }
        }
    }
};

// 120651 - Fel Explosives
struct npc_challenger_fel_explosives : ScriptedAI
{
    npc_challenger_fel_explosives(Creature* creature) : ScriptedAI(creature)
    {
        me->AddUnitTypeMask(UNIT_MASK_TOTEM);
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void Reset() override {}

    void IsSummonedBy(Unit* summoner) override
    {
        DoCast(me, SPELL_FEL_EXPLOSIVES_VISUAL, true);
        events.ScheduleEvent(EVENT_1, 500);
    }

    void SpellFinishCast(SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id == SPELL_FEL_EXPLOSIVES_DMG)
            me->DespawnOrUnsummon(100);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                    DoCast(SPELL_FEL_EXPLOSIVES_DMG);
                    break;
            }
        }
    }
};

//
class item_challenge_key : public ItemScript
{
public:
    item_challenge_key() : ItemScript("item_challenge_key") { }

    bool OnCreate(Player* player, Item* item) override
    {
        if (player->HasItemCount(138019, 1, true))
            player->InitChallengeKey(item);
        else
            player->CreateChallengeKey(item);
        return false;
    }
};

void AddSC_challenge_scripts()
{
    RegisterAuraScript(spell_challengers_might);
    RegisterAuraScript(spell_challengers_burden);
    RegisterAuraScript(spell_challengers_burst);
    RegisterAuraScript(spell_challengers_grievous_wound);
    RegisterSpellScript(spell_challengers_bolster);
    RegisterCreatureAI(npc_challenger_volcanic_plume);
    RegisterCreatureAI(npc_challenger_fel_explosives);
    new item_challenge_key();
}
