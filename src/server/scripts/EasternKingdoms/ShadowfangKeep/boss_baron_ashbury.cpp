#include"Spell.h"
#include"shadowfang_keep.h"

enum ScriptTexts
{
    SAY_AGGRO                = 0,
    SAY_DEATH                = 1,
    SAY_ASPHYXIATE           = 2,
    SAY_STAY_OF_EXECUTION    = 3,
    SAY_DARK_ARCHANGEL_FORM  = 4,
    SAY_KILL                 = 5,
};

enum Events
{
    EVENT_PAIN_AND_SUFFERING = 1,
    EVENT_ASPHYXIATE         = 2,
    EVENT_STAY_OF_EXECUTION  = 3,
    EVENT_CALAMITY           = 4,
    EVENT_WRACKING_PAIN      = 5,
};

enum Spells
{
    SPELL_PAIN_AND_SUFFERING    = 93581,
    SPELL_PAIN_AND_SUFFERING_H  = 93712,
    SPELL_ASPHYXIATE            = 93423,
    SPELL_ASPHYXIATE_H          = 93710,
    SPELL_ASPHYXIATE_DMG        = 93422,
    SPELL_STAY_OF_EXECUTION     = 93468,
    SPELL_STAY_OF_EXECUTION_H   = 93705,
    SPELL_STAY_OF_EXECUTION_H_T = 93706,
    SPELL_DARK_ARCHANGEL_FORM   = 93757,
    SPELL_DARK_ARCHANGEL_FORM_0 = 93766,
    SPELL_WRACKING_PAIN         = 93720,
    SPELL_CALAMITY              = 93812,
    SPELL_CALAMITY_DMG          = 93810,
};

class boss_baron_ashbury : public CreatureScript
{
    public:
        boss_baron_ashbury() : CreatureScript("boss_baron_ashbury") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_baron_ashburyAI(pCreature);
        }
        struct boss_baron_ashburyAI : public BossAI
        {
            boss_baron_ashburyAI(Creature* pCreature) : BossAI(pCreature, DATA_ASHBURY)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            }

            bool bArchangel;
            bool bCombo;
            bool bHeal;

            void Reset()
            {
                _Reset();

                bArchangel = false;
                bCombo = false;
                bHeal = false;
            }

            void EnterCombat(Unit* pWho)
            {
                Talk(SAY_AGGRO);
                events.RescheduleEvent(EVENT_PAIN_AND_SUFFERING, urand(8000, 9000));
                events.RescheduleEvent(EVENT_ASPHYXIATE, 30000);
                if (IsHeroic())
                    events.RescheduleEvent(EVENT_WRACKING_PAIN, 15000);
                instance->SetBossState(DATA_ASHBURY, IN_PROGRESS);
                DoZoneInCombat();
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
            }

            bool AllowAchieve()
            {
                return !bHeal && IsHeroic();
            }

            void HealReceived(Unit* healer, uint32 &heal)
            {
                if (healer->GetGUID() == me->GetGUID())
                    bHeal = true;
            }

            void KilledUnit(Unit* who)
            {
                Talk(SAY_KILL);
            }

            void JustDied(Unit* pWho)
            {
                _JustDied();

                Talk(SAY_DEATH);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WRACKING_PAIN);
            }
            
            void UpdateAI(uint32 uiDiff)
            {
                if (!UpdateVictim())
                    return;

                if (IsHeroic())
                {
                    if (!HealthAbovePct(20) && !bArchangel)
                    {
                        events.Reset();
                        bArchangel = true;
                        Talk(SAY_DARK_ARCHANGEL_FORM);
                        DoCast(me, SPELL_DARK_ARCHANGEL_FORM);
                        events.RescheduleEvent(EVENT_CALAMITY, 3000);
                    }
                }

                events.Update(uiDiff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                    
                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_PAIN_AND_SUFFERING:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_PAIN_AND_SUFFERING);
                            events.RescheduleEvent(EVENT_PAIN_AND_SUFFERING, urand(8000, 9000));
                            break;
                        case EVENT_ASPHYXIATE:
                            bCombo = true;
                            events.DelayEvents(7000);
                            Talk(SAY_ASPHYXIATE);
                            DoCast(DUNGEON_MODE(SPELL_ASPHYXIATE, SPELL_ASPHYXIATE_H));
                            events.RescheduleEvent(EVENT_STAY_OF_EXECUTION, 6100);
                            events.RescheduleEvent(EVENT_ASPHYXIATE, 45000);
                            break;
                        case EVENT_STAY_OF_EXECUTION:
                            Talk(SAY_STAY_OF_EXECUTION);
                            DoCast(SelectTarget(SELECT_TARGET_NEAREST), SPELL_STAY_OF_EXECUTION);
                            bCombo = false;
                            break;
                        case EVENT_CALAMITY:
                            DoCast(me, SPELL_CALAMITY);
                            break;
                        case EVENT_WRACKING_PAIN:
                            DoCast(me, SPELL_WRACKING_PAIN);
                            events.RescheduleEvent(EVENT_WRACKING_PAIN, 30000);
                            break;
                    }
                }
                if (!me->HasAura(SPELL_DARK_ARCHANGEL_FORM))
                    DoMeleeAttackIfReady();
            }
         };
};

typedef boss_baron_ashbury::boss_baron_ashburyAI AshburyAI;

class achievement_pardon_denied : public AchievementCriteriaScript
{
    public:
        achievement_pardon_denied() : AchievementCriteriaScript("achievement_pardon_denied") { }

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (AshburyAI* ashburyAI = CAST_AI(AshburyAI, target->GetAI()))
                return ashburyAI->AllowAchieve();

            return false;
        }
};

void AddSC_boss_baron_ashbury()
{
    new boss_baron_ashbury();
    new achievement_pardon_denied();
}