/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_jandicebarov
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scholomance.h"



//NPC_ILLUSION  = 59220, 

enum Spells
{
    SPELL_GRAVITY_FLUX       = 114059,
    SPELL_WONDROUS_RAPIDITY  = 114062,
};


class boss_jandice_barov : public CreatureScript
{
public:
    boss_jandice_barov() : CreatureScript("boss_jandice_barov") { }

    struct boss_jandice_barovAI : public BossAI
    {
        boss_jandice_barovAI(Creature* creature) : BossAI(creature, DATA_BAROV)
        {
            InstanceScript* instance = creature->GetInstanceScript();
            me->SetReactState(REACT_DEFENSIVE);
        }

        InstanceScript* instance;
        uint32 gravitytimer;
        uint32 rapiditytimer;

        void Reset()
        {
            _Reset();
            rapiditytimer = 0;
            gravitytimer = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            rapiditytimer = 5000;
            gravitytimer = 12000; 
        }

        void JustDied(Unit* killer)
        {
            _JustDied();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (gravitytimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                    DoCast(target, SPELL_GRAVITY_FLUX);
                gravitytimer = 20000;
            }
            else
                gravitytimer -= diff;

            if (rapiditytimer <= diff)
            {
                if (me->getVictim())
                    DoCast(me->getVictim(), SPELL_WONDROUS_RAPIDITY);
                rapiditytimer = 10000;
            }
            else
                rapiditytimer -= diff;

            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jandice_barovAI (creature);
    }

};

class spell_gravity_flux : public SpellScriptLoader
{
    public:
        spell_gravity_flux() :  SpellScriptLoader("spell_gravity_flux") { }

        class spell_gravity_flux_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_gravity_flux_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;
               
                if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
                    GetTarget()->CastSpell(GetTarget(), 114038, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_gravity_flux_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_gravity_flux_AuraScript();
        }
};

void AddSC_boss_jandice_barov()
{
    new boss_jandice_barov();
    new spell_gravity_flux();
}
