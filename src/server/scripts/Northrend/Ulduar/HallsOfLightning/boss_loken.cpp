#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "halls_of_lightning.h"
#include "LFGMgr.h"
#include "Group.h"

enum Texts
{
    SAY_INTRO_1                                 = 0,
    SAY_INTRO_2                                 = 1,
    SAY_AGGRO                                   = 2,
    SAY_NOVA                                    = 3,
    SAY_SLAY                                    = 4,
    SAY_75HEALTH                                = 5,
    SAY_50HEALTH                                = 6,
    SAY_25HEALTH                                = 7,
    SAY_DEATH                                   = 8,
    EMOTE_NOVA                                  = 9,
};

enum Spells
{
    SPELL_ARC_LIGHTNING                         = 52921,
    SPELL_LIGHTNING_NOVA                        = 52960,
    SPELL_PULSING_SHOCKWAVE                     = 52961,
    SPELL_PULSING_SHOCKWAVE_AURA                = 59414
};

enum Events
{
    EVENT_ARC_LIGHTNING                         = 1,
    EVENT_LIGHTNING_NOVA,
    EVENT_RESUME_PULSING_SHOCKWAVE,
    EVENT_INTRO_DIALOGUE
};

enum Phases
{
    PHASE_INTRO = 1,
    PHASE_NORMAL
};

enum Misc
{
    ACHIEV_TIMELY_DEATH_START_EVENT             = 20384
};


class boss_loken : public CreatureScript
{
public:
    boss_loken() : CreatureScript("boss_loken") { }

    struct boss_lokenAI : public BossAI
    {
        boss_lokenAI(Creature* creature) : BossAI(creature, DATA_LOKEN)
        {
            Initialize();
            _isIntroDone = false;
        }

        void Initialize()
        {
            _healthAmountModifier = 1;
        }

        void Reset() override
        {
            Initialize();
            _Reset();
            if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                instance->DoStopTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, ACHIEV_TIMELY_DEATH_START_EVENT);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            events.SetPhase(PHASE_NORMAL);
            events.ScheduleEvent(EVENT_ARC_LIGHTNING, 15000);
            events.ScheduleEvent(EVENT_LIGHTNING_NOVA, 20000);
            events.ScheduleEvent(EVENT_RESUME_PULSING_SHOCKWAVE, 1000);
            if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, ACHIEV_TIMELY_DEATH_START_EVENT);
            
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PULSING_SHOCKWAVE_AURA);
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                {
                    Player* pPlayer = players.begin()->getSource();
                    if (pPlayer && pPlayer->GetGroup())
                        if (sLFGMgr->GetQueueId(995))
                            sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 995);
                }
            }
        }

        void KilledUnit(Unit* who) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!_isIntroDone && me->IsValidAttackTarget(who) && me->IsWithinDistInMap(who, 40.0f))
            {
                _isIntroDone = true;
                Talk(SAY_INTRO_1);
                events.ScheduleEvent(EVENT_INTRO_DIALOGUE, 20000, 0, PHASE_INTRO);
            }
            BossAI::MoveInLineOfSight(who);
        }

        void UpdateAI(uint32 diff) override
        {
            if (events.IsInPhase(PHASE_NORMAL) && !UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ARC_LIGHTNING:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_ARC_LIGHTNING);
                    events.ScheduleEvent(EVENT_ARC_LIGHTNING, urand(15000, 16000));
                    break;
                case EVENT_LIGHTNING_NOVA:
                    Talk(SAY_NOVA);
                    Talk(EMOTE_NOVA);
                    DoCastAOE(SPELL_LIGHTNING_NOVA);
                    me->RemoveAurasDueToSpell(SPELL_PULSING_SHOCKWAVE);
                    events.ScheduleEvent(EVENT_RESUME_PULSING_SHOCKWAVE, DUNGEON_MODE(5000, 4000)); // Pause Pulsing Shockwave aura
                    events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(20000, 21000));
                    break;
                case EVENT_RESUME_PULSING_SHOCKWAVE:
                    DoCast(me, SPELL_PULSING_SHOCKWAVE_AURA, true);
                    me->ClearUnitState(UNIT_STATE_CASTING); // This flag breaks movement.
                    DoCast(me, SPELL_PULSING_SHOCKWAVE, true);
                    break;
                case EVENT_INTRO_DIALOGUE:
                    Talk(SAY_INTRO_2);
                    events.SetPhase(PHASE_NORMAL);
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPctDamaged(100 - 25 * _healthAmountModifier, damage))
            {
                switch (_healthAmountModifier)
                {
                case 1:
                    Talk(SAY_75HEALTH);
                    break;
                case 2:
                    Talk(SAY_50HEALTH);
                    break;
                case 3:
                    Talk(SAY_25HEALTH);
                    break;
                default:
                    break;
                }
                ++_healthAmountModifier;
            }
        }

    private:
        uint32 _healthAmountModifier;
        bool _isIntroDone;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_lokenAI(creature);
    }
};

class spell_loken_pulsing_shockwave : public SpellScriptLoader
{
    public:
        spell_loken_pulsing_shockwave() : SpellScriptLoader("spell_loken_pulsing_shockwave") { }

        class spell_loken_pulsing_shockwave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_loken_pulsing_shockwave_SpellScript);

            void CalculateDamage(SpellEffIndex /*effIndex*/)
            {
                if (!GetHitUnit())
                    return;

                float distance = GetCaster()->GetDistance2d(GetHitUnit());
                if (distance > 1.0f)
                    SetHitDamage(int32(GetHitDamage() * distance));
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_loken_pulsing_shockwave_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_loken_pulsing_shockwave_SpellScript();
        }
};

void AddSC_boss_loken()
{
    new boss_loken();
    new spell_loken_pulsing_shockwave();
}
