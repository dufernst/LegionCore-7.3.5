#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scholomance.h"

enum Spells
{
    SPELL_BOILING_BLOODTHIRST_FILTER = 114148,
    SPELL_BOILING_BLOODTHIRST        = 114155,
    SPELL_BOILING_BLOODTHIRST_STACK  = 114141,
    SPELL_ACHIEV_CREDIT              = 115427,
};

class npc_krastinovian_carver : public CreatureScript
{
    public:
        npc_krastinovian_carver() : CreatureScript("npc_krastinovian_carver") {}

        struct npc_krastinovian_carverAI : public CreatureAI
        {
            npc_krastinovian_carverAI(Creature* creature) : CreatureAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;

            void Reset()
            {
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCast(SPELL_BOILING_BLOODTHIRST_FILTER);
                if (Aura* aura = me->GetAura(SPELL_BOILING_BLOODTHIRST_STACK))
                    if (aura->GetStackAmount() == 99)
                        DoCast(me, SPELL_ACHIEV_CREDIT, true);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_krastinovian_carverAI(creature);
        }
};

class npc_professor_slate : public CreatureScript
{
    public:
        npc_professor_slate() : CreatureScript("npc_professor_slate") {}

        struct npc_professor_slateAI : public CreatureAI
        {
            npc_professor_slateAI(Creature* creature) : CreatureAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;

            void Reset()
            {
            }

            void JustDied(Unit* /*killer*/)
            {
                if (GameObject* go = me->FindNearestGameObject(211669, 60.0f))
                    go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_professor_slateAI(creature);
        }
};

class spell_boiling_bloodthirst : public SpellScriptLoader
{
    public:
        spell_boiling_bloodthirst() : SpellScriptLoader("spell_boiling_bloodthirst") { }

        class spell_boiling_bloodthirst_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_boiling_bloodthirst_SpellScript);

            void HandleOnCast()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Aura* aura = caster->GetAura(SPELL_BOILING_BLOODTHIRST_STACK))
                {
                    uint32 stackcount;
                    stackcount = aura->GetStackAmount();

                    for (uint8 i = 0; i < stackcount; ++i)
                        caster->CastSpell(caster, SPELL_BOILING_BLOODTHIRST, true);
                }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_boiling_bloodthirst_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_boiling_bloodthirst_SpellScript();
        }
};

void AddSC_scholomance()
{
    new npc_krastinovian_carver();
    new npc_professor_slate();
    new spell_boiling_bloodthirst();
}