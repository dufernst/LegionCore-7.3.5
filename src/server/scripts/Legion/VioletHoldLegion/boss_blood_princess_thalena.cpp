/*
    Dungeon : Violet Hold Legion 100-110
    Encounter: Blood-Princess Thalena
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "violet_hold_legion.h"

enum Says
{
    SAY_DEATH           = 0,
};

enum Spells
{
    SPELL_SHROUD_OF_SORROW          = 203033,
    SPELL_SHROUD_OF_SORROW_DMG      = 203057,
    SPELL_VAMPYR_KISS               = 202652, //Find target
    SPELL_VAMPYR_KISS_JUMP          = 202676,
    SPELL_VAMPYR_KISS_DMG           = 202766,
    SPELL_VAMPYR_ESSENCE_1          = 202779, //Select target
    SPELL_VAMPYR_ESSENCE_2          = 202810, //Self target
    SPELL_VAMPYR_ESSENCE_MOD        = 202781, //Mod dmg, threat...
    SPELL_FRENZIED_BLOODTHIRST      = 202792,
    SPELL_UNCONTROLLABLE_FRENZY     = 202804,
    SPELL_BLOOD_SWARM               = 202659,
    SPELL_BLOOD_SWARM_TRIG          = 202785,
    SPELL_BLOOD_SWARM_AT            = 202944,

    //Heroic+
    SPELL_BLOOD_CALL                = 203381,
    SPELL_BLOOD_CALL_VISUAL         = 203405,
    SPELL_BLOOD_CALL_SUM            = 203452,
    SPELL_BLOOD_DESTRUCT_AT         = 203469,
    SPELL_ETERNAL_HUNGER            = 203462,
};

enum eEvents
{
    EVENT_VAMPYR_KISS               = 1,
    EVENT_BLOOD_SWARM               = 2,
    EVENT_BLOOD_CALL                = 3
};

uint32 vampireAuras[4] = 
{
    SPELL_VAMPYR_ESSENCE_1, 
    SPELL_VAMPYR_ESSENCE_2, 
    SPELL_FRENZIED_BLOODTHIRST,
    SPELL_UNCONTROLLABLE_FRENZY
};

bool IsVampire(Unit* unit)
{
    for (uint8 i = 0; i < 4; ++i)
        if (unit->HasAura(vampireAuras[i]))
            return true;

    return false;
}

//102431
class boss_blood_princess_thalena : public CreatureScript
{
public:
    boss_blood_princess_thalena() : CreatureScript("boss_blood_princess_thalena") {}

    struct boss_blood_princess_thalenaAI : public BossAI
    {
        boss_blood_princess_thalenaAI(Creature* creature) : BossAI(creature, DATA_THALENA) 
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
            removeloot = false;
        }

        bool removeloot;

        void Reset() override
        {
            _Reset();

            for (uint8 i = 0; i < 4; ++i)
                instance->DoRemoveAurasDueToSpellOnPlayers(vampireAuras[i]);

            if (instance->GetData(DATA_MAIN_EVENT_PHASE) == IN_PROGRESS)
                me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* who) override
        //30:22
        {
            Conversation* conversation = new Conversation;
            if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 1231, who, NULL, *who))
                delete conversation;
            _EnterCombat();
            DoCast(me, SPELL_SHROUD_OF_SORROW, true);

            events.RescheduleEvent(EVENT_VAMPYR_KISS, 6000); //30:28. Не повторяется.
            events.RescheduleEvent(EVENT_BLOOD_SWARM, 20000); //30:42, 31:03, 31:24

            if (GetDifficultyID() != DIFFICULTY_NORMAL)
                events.RescheduleEvent(EVENT_BLOOD_CALL, 30000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();

            for (uint8 i = 0; i < 4; ++i)
                instance->DoRemoveAurasDueToSpellOnPlayers(vampireAuras[i]);

            if (removeloot)
                me->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
            {
                if (spell->Id == SPELL_VAMPYR_KISS_JUMP)
                {
                    DoCast(target, SPELL_VAMPYR_KISS_DMG, true);
                    target->CastSpell(target, SPELL_VAMPYR_ESSENCE_MOD, true);
                }
            }

            if (spell->Id == SPELL_BLOOD_CALL_VISUAL)
                target->CastSpell(target, SPELL_BLOOD_CALL_SUM, true, 0, 0, me->GetGUID());
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_REMOVE_LOOT)
                removeloot = true;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_VAMPYR_KISS:
                    {
                        DoCast(me, SPELL_VAMPYR_KISS, true);
                        Unit* target = me->SelectVictim();
                        Conversation* conversation = new Conversation;
                        if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 1232, target, NULL, *target))
                            delete conversation;
                    }
                        break;
                    case EVENT_BLOOD_SWARM:
                        DoCast(SPELL_BLOOD_SWARM);
                        events.RescheduleEvent(EVENT_BLOOD_SWARM, 21000);
                        break;
                    case EVENT_BLOOD_CALL:
                        DoCast(SPELL_BLOOD_CALL);
                        events.RescheduleEvent(EVENT_BLOOD_CALL, 30000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_blood_princess_thalenaAI (creature);
    }
};

//102659
class npc_thalena_pool_of_blood : public CreatureScript
{
public:
    npc_thalena_pool_of_blood() : CreatureScript("npc_thalena_pool_of_blood") {}

    struct npc_thalena_pool_of_bloodAI : public ScriptedAI
    {
        npc_thalena_pool_of_bloodAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override {}

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_BLOOD_CALL_VISUAL)
                me->DespawnOrUnsummon(500);
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_thalena_pool_of_bloodAI(creature);
    }
};

//102941
class npc_thalena_congealed_blood : public CreatureScript
{
public:
    npc_thalena_congealed_blood() : CreatureScript("npc_thalena_congealed_blood") {}

    struct npc_thalena_congealed_bloodAI : public ScriptedAI
    {
        npc_thalena_congealed_bloodAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetSpeed(MOVE_RUN, 0.5f);
            me->SetSpeed(MOVE_WALK, 0.5f);
        }

        EventMap events;

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            events.RescheduleEvent(EVENT_1, 500);
            events.RescheduleEvent(EVENT_2, 1000);
        }

        void OnAreaTriggerCast(Unit* caster, Unit* target, uint32 spellId, uint32 createATSpellId) override
        {
            if (spellId == SPELL_ETERNAL_HUNGER)
                me->DespawnOrUnsummon(500);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(me, SPELL_BLOOD_DESTRUCT_AT, true);
                        break;
                    case EVENT_2:
                        if (Unit* summoner = me->GetAnyOwner())
                            me->GetMotionMaster()->MovePoint(1, summoner->GetPosition());
                        events.RescheduleEvent(EVENT_2, 1000);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_thalena_congealed_bloodAI(creature);
    }
};

//203035
class spell_thalena_shroud_of_sorrow : public SpellScriptLoader
{
    public:
        spell_thalena_shroud_of_sorrow() : SpellScriptLoader("spell_thalena_shroud_of_sorrow") { }

        class spell_thalena_shroud_of_sorrow_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thalena_shroud_of_sorrow_SpellScript);
            
            uint8 vampireCount = 0;
            uint32 spellId = 0;
            float damage = 0;

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                std::list<WorldObject*> targetList;
                for (auto const& target : targets)
                {
                    if (!IsVampire(target->ToPlayer()))
                        targetList.push_back(target);
                    else
                        vampireCount++;
                }

                targets.clear();
                targets = targetList;
            }

            void HandleOnHit()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_SHROUD_OF_SORROW_DMG))
                    damage = spellInfo->Effects[EFFECT_0]->BasePoints;

                if (!vampireCount)
                    vampireCount = 1;

                damage *= vampireCount;
                GetCaster()->CastCustomSpell(GetHitUnit(), SPELL_SHROUD_OF_SORROW_DMG, &damage, 0, 0, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_thalena_shroud_of_sorrow_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnHit += SpellHitFn(spell_thalena_shroud_of_sorrow_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thalena_shroud_of_sorrow_SpellScript();
        }
};

//202805
class spell_thalena_vampiric_bite : public SpellScriptLoader
{
    public:
        spell_thalena_vampiric_bite() : SpellScriptLoader("spell_thalena_vampiric_bite") { }

        class spell_thalena_vampiric_bite_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_thalena_vampiric_bite_SpellScript);

            SpellCastResult CheckTarget()
            {
                if (IsVampire(GetExplTargetUnit()))
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_CANT_TARGET_VAMPIRES);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (!GetCaster() || !GetHitUnit() || GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_VAMPYR_ESSENCE_2, true);
                GetCaster()->RemoveAurasDueToSpell(SPELL_FRENZIED_BLOODTHIRST);
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_VAMPYR_ESSENCE_MOD, true);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_thalena_vampiric_bite_SpellScript::CheckTarget);
                OnHit += SpellHitFn(spell_thalena_vampiric_bite_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_thalena_vampiric_bite_SpellScript();
        }
};

//202792
class spell_thalena_frenzied_bloodthirst : public SpellScriptLoader
{
    public:
        spell_thalena_frenzied_bloodthirst() : SpellScriptLoader("spell_thalena_frenzied_bloodthirst") { }

        class spell_thalena_frenzied_bloodthirst_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_thalena_frenzied_bloodthirst_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                Unit* target = GetTarget();
                if (!target)
                    return;

                if (InstanceScript* instance = target->GetInstanceScript())
                    if (Creature* bloodPrincess = instance->instance->GetCreature(instance->GetGuidData(DATA_THALENA)))
                    {
                        //bloodPrincess->AI()->Talk(SAY_MIND_CONTROL);
                        bloodPrincess->CastSpell(target, SPELL_UNCONTROLLABLE_FRENZY, true);
                    }
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_thalena_frenzied_bloodthirst_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_thalena_frenzied_bloodthirst_AuraScript();
        }
};

void AddSC_boss_blood_princess_thalena()
{
    new boss_blood_princess_thalena();
    new npc_thalena_pool_of_blood();
    new npc_thalena_congealed_blood();
    new spell_thalena_shroud_of_sorrow();
    new spell_thalena_vampiric_bite();
    new spell_thalena_frenzied_bloodthirst();
}