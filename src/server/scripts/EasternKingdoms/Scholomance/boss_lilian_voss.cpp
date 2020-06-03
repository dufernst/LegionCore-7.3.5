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
SDName: Boss_Lord_Alexei_Barov
SD%Complete: 100
SDComment: aura applied/defined in database
SDCategory: Scholomance
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scholomance.h"

enum
{
    SPELL_DEATH_GRASP      = 111570,
    SPELL_SHADOW_SHIV      = 111775, 
    SPELL_DARK_BLAZE_S     = 111585,
    SPELL_DARK_BLAZE_AURA  = 111587,
};
class boss_lilian_voss : public CreatureScript
{
public:
    boss_lilian_voss() : CreatureScript("boss_lilian_voss") { }

    struct boss_lilian_vossAI : public BossAI
    {
        boss_lilian_vossAI(Creature* creature) : BossAI(creature, DATA_LILIAN) {}

        uint32 grasp;
        uint32 shiv;

        void Reset()
        {
           _Reset();
           me->SetReactState(REACT_DEFENSIVE);
           grasp = 0;
           shiv = 0; 
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            grasp = 15000;
            shiv = 10000;
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            if (instance)
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 30834);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (grasp <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                    DoCast(target, SPELL_DEATH_GRASP);
                grasp = 20000;
            }
            else
                grasp -= diff;

            if (shiv <= diff)
            {
                if (me->getVictim())
                {
                    DoCast(me->getVictim(), SPELL_SHADOW_SHIV);
                    shiv = 10000;
                }
            }
            else
                shiv -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_lilian_vossAI (creature);
    }

};

class npc_dark_blaze : public CreatureScript
{
    public:
        npc_dark_blaze() : CreatureScript("npc_dark_blaze") {}

        struct npc_dark_blazeAI : public ScriptedAI
        {
            npc_dark_blazeAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            InstanceScript* instance;
            uint32 unsummon;

            void Reset()
            {
                me->AddAura(SPELL_DARK_BLAZE_AURA, me);
                unsummon = 30000;
            }

            void EnterEvadeMode(){}

            void EnterCombat(Unit* who){}

            void UpdateAI(uint32 diff)
            {
                if (unsummon)
                {
                    if (unsummon <= diff)
                    {
                        me->DespawnOrUnsummon();
                        unsummon = 0; 
                    }
                    else
                        unsummon -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dark_blazeAI(creature);
        }
};

class spell_death_grasp : public SpellScriptLoader
{
    public:
        spell_death_grasp() : SpellScriptLoader("spell_death_grasp") { }

        class spell_death_grasp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_death_grasp_SpellScript);

            void HandleGrip(SpellEffIndex /*effIndex*/)
            {
                int32 spell = 49575;
                float x,y,z;
                GetCaster()->GetPosition(x, y, z);
                if (Map* map = GetCaster()->GetMap())
                    if (map->IsDungeon())
                    {
                        Map::PlayerList const &players = map->GetPlayers();
                        for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                        {
                            if (Player* pPlayer = i->getSource())
                            {
                                if (pPlayer->isAlive())
                                {
                                    pPlayer->CastSpell(x, y, z, spell, true);
                                    GetCaster()->CastSpell(pPlayer, SPELL_DARK_BLAZE_S);
                                }
                                
                            }
                        }
                    }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_death_grasp_SpellScript::HandleGrip, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_death_grasp_SpellScript();
        }
};

void AddSC_boss_lilian_voss()
{
    new boss_lilian_voss();
    new npc_dark_blaze();
    new spell_death_grasp();
}
