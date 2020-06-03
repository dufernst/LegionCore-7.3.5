#include "well_of_eternity.h"
#include "ScriptedEscortAI.h"

enum eEnums
{
    SAY_AGGRO                      = 1,
    SAY_SLAY                       = 3,
    SAY_DEATH                      = 4,

    SPELL_CORRUPTING_TOUCH         = 104939,
    SPELL_FEL_FLAMES               = 108141,
    SPELL_FEL_DECAY                = 105544,
    SPELL_DRAIN_ESSENCE            = 104905,
    SPELL_ENDLESS_FRENZY           = 105521,
    
    SPELL_RETURN_TO_THE_SHADOWS    = 105635,
    SPELL_HUNTING_LOCK_ON_PLAYER   = 107670,
    SPELL_TRACKED_LOCK_ON_PLAYER   = 105496,
};

class boss_perotharn : public CreatureScript
{
public:
    boss_perotharn() : CreatureScript("boss_perotharn") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_perotharnAI (creature);
    }

    struct boss_perotharnAI : public ScriptedAI
    {
        boss_perotharnAI(Creature* c) : ScriptedAI(c), summons(me)
        {
            instance = c->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 CorruptingTouchTimer;
        uint32 FelFlamesTimer;
        uint32 FelDecayTimer;

        bool isEndlessFrenzy;

        uint32 EventTimer;
        bool event;
        bool eventcompelte;
        uint8 phase;

        SummonList summons;

        void Reset()
        {
            CorruptingTouchTimer = 45000;
            FelFlamesTimer = 10000;
            FelDecayTimer = 20000;

            isEndlessFrenzy = false;

            EventTimer = 0;
            event = false;
            eventcompelte = false;
            phase = 0;

            summons.DespawnAll();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
        }

        void JustSummoned(Creature* summoned)
        {
            summons.Summon(summoned);
        }

        void JustDied(Unit* /*victim*/)
        {
            Talk(SAY_DEATH);

            summons.DespawnAll();
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_SLAY);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (!event)
            {
                if (CorruptingTouchTimer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_CORRUPTING_TOUCH);
                    CorruptingTouchTimer = 45000;
                } else CorruptingTouchTimer -= diff;

                if (FelFlamesTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_FEL_FLAMES);
                    FelFlamesTimer = 10000;
                } else FelFlamesTimer -= diff;

                if (FelDecayTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_FEL_DECAY);
                    FelDecayTimer = 20000;
                } else FelDecayTimer -= diff;

                if (!eventcompelte && me->GetHealthPct() <= 70.0f)
                    event = true;

                if (!isEndlessFrenzy && me->GetHealthPct() <= 25.0f)
                {
                    isEndlessFrenzy = true;
                    DoCast(me, SPELL_ENDLESS_FRENZY);
                }

                DoMeleeAttackIfReady();
            }
            else
            {
                if (EventTimer <= diff)
                {
                    switch (phase)
                    {
                        case 0: // Ваша сущность принадлежит мне.
                            me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_DRAIN_ESSENCE, true);
                            EventTimer = 4000;
                            break;
                        case 1: // Твоя жалкая магия не сравнится с моей.
                            EventTimer = 5000;
                            break;
                        case 2: // Теперь тени служат мне…
                            me->SetUInt32Value(UNIT_FIELD_FLAGS, 2181597184);
                            me->SetAttackTime(BASE_ATTACK, 2001980);
                            me->SetAttackTime(OFF_ATTACK,  2001980);
                            me->SetDisplayId(17743);
                            EventTimer = 2000;
                            break;
                        case 3:
                            // Скорее, обратно в тени!
                            me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_RETURN_TO_THE_SHADOWS, true); // Illidan spell
                            EventTimer = 2000;
                            break;
                        case 4:
                            // Прячьтесь и дрожите от страха.
                            for (uint8 i = 0; i < 5; ++i)
                                me->SummonCreature(55868, 3342.711f, -4893.917f, 181.2851f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40 * IN_MILLISECONDS);
                            EventTimer = 40000;
                            break;
                        case 5: // Неплохо прячетесь. Но долго ли вы сможете противиться судьбе?
                            CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(me->GetEntry());
                            me->SetUInt32Value(UNIT_FIELD_FLAGS, ci->unit_flags);
                            me->SetAttackTime(BASE_ATTACK, ci->baseattacktime);
                            me->SetAttackTime(OFF_ATTACK,  2000);
                            me->SetDisplayId(sObjectMgr->ChooseDisplayId(0, ci));
                            event = false;
                            eventcompelte = true;
                            break;
                    }
                    ++phase;
                } else EventTimer -= diff;
            }
        }
        
        void AttackTarget(Player* plr) // Я тебя вижу.
        {
            CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(me->GetEntry());
            me->SetUInt32Value(UNIT_FIELD_FLAGS, ci->unit_flags);
            me->SetAttackTime(BASE_ATTACK, ci->baseattacktime);
            me->SetAttackTime(OFF_ATTACK,  2000);
            me->SetDisplayId(sObjectMgr->ChooseDisplayId(0, ci));

            DoCast(plr, SPELL_TRACKED_LOCK_ON_PLAYER, true);

            event = false;
            eventcompelte = true;
        }
    };
};

class spell_boss_perotharn_drain_sssence : public SpellScriptLoader
{
    public:
        spell_boss_perotharn_drain_sssence() : SpellScriptLoader("spell_boss_perotharn_drain_sssence") {}

        class spell_boss_perotharn_drain_sssence_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_boss_perotharn_drain_sssence_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit* caster = GetTarget())
                    caster->AddAura(105545, caster);
                    //caster->CastSpell(caster, 105545, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_boss_perotharn_drain_sssence_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_boss_perotharn_drain_sssence_AuraScript();
        }
};

void AddSC_boss_perotharn()
{
    new boss_perotharn();
    new spell_boss_perotharn_drain_sssence();
}
