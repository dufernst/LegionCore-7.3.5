/*
    The Seat of the Triumvirate: Viceroy Nezhar [heroic & mythic]
    95% DONE
*/

#include "the_seat_of_the_triumvirate.h"

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_DEATH                   = 1,
    SAY_CAST                    = 2,
    SAY_KILL                    = 3,
};

enum Spells
{
    //Nezhar
    SPELL_DARK_BLAST            = 244750,
    SPELL_CALL_FOR_HELP         = 249306,
    SPELL_ENERGIZE              = 245028,
    SPELL_TENTACLES_TS          = 245038,
    SPELL_TENTACLES_SUM         = 244770,
    SPELL_FEAR                  = 244751,
    SPELL_ENTROPIC_FORCE        = 246324,
    //mythic
    SPELL_ETERNAL_TWILIGHT      = 248736,
    SPELL_SUM_VOIDTENDER_L      = 249335,
    SPELL_SUM_VOIDTENDER_R      = 249336,
    SPELL_DARK_PROTECT          = 248804,
    //tentacles
    SPELL_TENTACLE_VISUAL       = 244924,
    SPELL_UNSTABLE_ENTRANCE     = 249082,
    SPELL_VOID_LASHING          = 244916,
    SPELL_VOID_LASHING_DEBUFF   = 248733,

    SPELL_COLLAPSINGVOID_AT     = 244789, // Need script for this at

    //trash
    SPELL_DARK_FLAY             = 248184,
    SPELL_DARK_MATTER           = 248227,
    SPELL_COLLAPSE              = 248228,
    SPELL_DARK_SPLICE           = 248239,
    SPELL_DARK_SPLICE_1         = 248249,
    SPELL_ESSENCE_SPLIT         = 248232,
    SPELL_ENTROPIC_MIST         = 245522,
    SPELL_DARK_OUTBREAK         = 246900,
    SPELL_CORRUPT_TOUCH         = 245746,
};

enum eEvents
{
    EVENT_TENTACLES             = 1,
    EVENT_FEAR                  = 2,
    EVENT_ENTROPIC_FORCE        = 3,
};

Position const posC = { 6142.81f, 10388.20f, 19.87f, 0.0f };

//122056
class boss_viceroy_nezhar : public CreatureScript
{
public:
    boss_viceroy_nezhar() : CreatureScript("boss_viceroy_nezhar") {}

    struct boss_viceroy_nezharAI : public BossAI
    {
        boss_viceroy_nezharAI(Creature* creature) : BossAI(creature, DATA_NEZHAR)
        {
            me->SetPower(POWER_ENERGY, 0);
            pause = false;
            cast = false;
        }

        uint32 PowerUpdate = 500;
        uint8 adddied = 0;
        bool pause;
        bool cast;

        void Reset() override 
        {
            _Reset();
            me->RemoveAurasDueToSpell(SPELL_ENERGIZE);
            me->SetPower(POWER_ENERGY, 0);
            pause = false;
            cast = false;
            adddied = 0;
        }

        void EnterCombat(Unit* /*who*/) override 
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                DoCast(me, SPELL_ENERGIZE, true);
            if (Creature* center = me->SummonCreature(122832, posC))
                center->CastSpell(center, SPELL_COLLAPSINGVOID_AT, true);
            DoCast(SPELL_CALL_FOR_HELP);
            events.RescheduleEvent(EVENT_TENTACLES, 12000);
            events.RescheduleEvent(EVENT_FEAR, 16000);
            events.RescheduleEvent(EVENT_ENTROPIC_FORCE, 32000);
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->ToPlayer())
                Talk(SAY_KILL);
        }

        void JustDied(Unit* /*killer*/) override 
        {
            _JustDied();
            Talk(SAY_DEATH);
            me->AddDelayedEvent(500, [this] {
                me->SummonCreature(NPC_LURA_INTROPORTAL, 6122.06f, 10366.60f, 19.92f, 3.93f);
            });
        }

        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
        {
            if (spellId == SPELL_DARK_PROTECT && !apply)
            {
                ++adddied;
                if (adddied == 2)
                {
                    adddied = 0;
                    pause = false;
                }
            }

            if (spellId == SPELL_ENTROPIC_FORCE && !apply)
                me->RemoveAllAreaObjects();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_DARK_PROTECT && !pause)
            {
                pause = true;
                cast = true;
                Talk(SAY_CAST);
                me->AddDelayedEvent(500, [this] {
                    me->InterruptSpell(CURRENT_GENERIC_SPELL);
                    DoCast(me, SPELL_ETERNAL_TWILIGHT);
                });
                me->AddDelayedEvent(5000, [this] {
                    cast = false;
                });
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_ETERNAL_TWILIGHT)
                cast = false;
        }

        void Tentacles()
        {
            uint8 count = 3;
            for (uint8 i = 1; i <= count; ++i)
            {
                me->AddDelayedEvent(10 * i, [this]() -> void
                {
                    Unit* target = me->FindNearestCreature(122832, 100.0f);
                    if (!target)
                        return;

                    Position pos(target->GetPosition());
                    me->MovePosition(pos, frand(12, 25), frand(14, 25));
                    me->SummonCreature(122827, pos);
                });
            }
        }

        void UpdateAI(uint32 diff) override 
        {
            if (!UpdateVictim())
                return;

            if (CheckHomeDistToEvade(diff, 60.0f))
                return;

            //if (me->HasUnitState(UNIT_STATE_CASTING))
                //return;

            if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
            {
                if (PowerUpdate)
                {
                    if (PowerUpdate <= diff)
                    {
                        if (me->GetPower(POWER_ENERGY) == 100 && !me->HasUnitState(UNIT_STATE_CASTING))
                        {
                            me->SetPower(POWER_ENERGY, 0);
                            DoCast(me, SPELL_SUM_VOIDTENDER_L, true);
                            DoCast(me, SPELL_SUM_VOIDTENDER_R, true);
                            PowerUpdate = 500;
                        }
                    }
                    else
                        PowerUpdate -= diff;
                }
            }

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_TENTACLES:
                    Tentacles();
                    events.RescheduleEvent(EVENT_TENTACLES, 30000);
                    break;
                case EVENT_FEAR:
                    if (!cast)
                        me->InterruptSpell(CURRENT_GENERIC_SPELL);
                    DoCast(SPELL_FEAR);
                    events.RescheduleEvent(EVENT_FEAR, 33000);
                    break;
                case EVENT_ENTROPIC_FORCE:
                    if (!cast)
                        me->InterruptSpell(CURRENT_GENERIC_SPELL);
                    DoCast(SPELL_ENTROPIC_FORCE);
                    events.RescheduleEvent(EVENT_ENTROPIC_FORCE, 61000);
                    break;
                }
            }
            if (!pause || !cast)
                DoSpellAttackIfReady(SPELL_DARK_BLAST);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_viceroy_nezharAI (creature);
    }
};

//122832
class npc_nezhar_center_point : public CreatureScript
{
public:
    npc_nezhar_center_point() : CreatureScript("npc_nezhar_center_point") {}

    struct npc_nezhar_center_pointAI : public ScriptedAI
    {
        npc_nezhar_center_pointAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_center_pointAI(creature);
    }
};

//125102
class npc_sott_void_portal : public CreatureScript
{
public:
    npc_sott_void_portal() : CreatureScript("npc_sott_void_portal") {}

    struct npc_sott_void_portalAI : public ScriptedAI
    {
        npc_sott_void_portalAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void IsSummonedBy(Unit* summoner) override 
        { 
            if (summoner->GetEntry() == NPC_NEZHAR)
            {
                me->AddDelayedEvent(500, [this] {
                    me->SummonCreature(NPC_ALLERIA_E1, 6114.09f, 10367.71f, 19.89f, 3.89f);
                    me->SummonCreature(NPC_LURA_LOCUSWALKER, 6120.97f, 10359.79f, 19.89f, 3.87f);
                });
                me->AddDelayedEvent(2000, [this] {
                    me->DespawnOrUnsummon();
                });
            }
            if (summoner->GetEntry() == NPC_LURA)
            {
                if (me->GetPositionX() == 5993.89f && me->GetPositionY() == 10259.29f)
                    me->SummonCreature(NPC_LURA_RIFT_WARDEN, 5992.85f, 10250.09f, 14.64f, 1.55f);

                if (me->GetPositionX() == 6017.91f && me->GetPositionY() == 10236.09f)
                    me->SummonCreature(NPC_LURA_RIFT_WARDEN, 6007.29f, 10234.70f, 14.01f, 0.49f);
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == NPC_LURA_RIFT_WARDEN)
                me->Kill(me);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sott_void_portalAI(creature);
    }
};

//122827
class npc_nezhar_tentacle : public CreatureScript
{
public:
    npc_nezhar_tentacle() : CreatureScript("npc_nezhar_tentacle") {}

    struct npc_nezhar_tentacleAI : public ScriptedAI
    {
        npc_nezhar_tentacleAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->AddAura(SPELL_TENTACLE_VISUAL, me);
            me->SetControlled(1, UNIT_STATE_NOT_MOVE);
        }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, urand(2500, 5000));
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->AddDelayedEvent(500, [this] {
                DoCast(me, SPELL_UNSTABLE_ENTRANCE);
            });
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(SPELL_VOID_LASHING);
                    events.RescheduleEvent(EVENT_1, 6000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_tentacleAI(creature);
    }
};

//125615
class npc_nezhar_shadowguard_voidtender : public CreatureScript
{
public:
    npc_nezhar_shadowguard_voidtender() : CreatureScript("npc_nezhar_shadowguard_voidtender") {}

    struct npc_nezhar_shadowguard_voidtenderAI : public ScriptedAI
    {
        npc_nezhar_shadowguard_voidtenderAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->AddDelayedEvent(1500, [this] {
                if (Creature* nezhar = me->FindNearestCreature(NPC_NEZHAR, 100.0f, true))
                    DoCast(nezhar, SPELL_DARK_PROTECT);
            });
        }

        void UpdateAI(uint32 diff) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_shadowguard_voidtenderAI(creature);
    }
};

//124947
class npc_nezhar_void_flayer : public CreatureScript
{
public:
    npc_nezhar_void_flayer() : CreatureScript("npc_nezhar_void_flayer") {}

    struct npc_nezhar_void_flayerAI : public ScriptedAI
    {
        npc_nezhar_void_flayerAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetControlled(1, UNIT_STATE_NOT_MOVE);
        }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 1500);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_CALL_FOR_HELP)
                DoZoneInCombat();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(SPELL_DARK_FLAY);
                    events.RescheduleEvent(EVENT_1, 8000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_void_flayerAI(creature);
    }
};

//122423
class npc_nezhar_grand_shadow_weaver : public CreatureScript
{
public:
    npc_nezhar_grand_shadow_weaver() : CreatureScript("npc_nezhar_grand_shadow_weaver") {}

    struct npc_nezhar_grand_shadow_weaverAI : public ScriptedAI
    {
        npc_nezhar_grand_shadow_weaverAI(Creature* creature) : ScriptedAI(creature), summons(me) { }

        EventMap events;
        SummonList summons;

        void Reset() override
        {
            events.Reset();
            summons.DespawnAll();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 4000);
            events.RescheduleEvent(EVENT_2, 10000);
            events.RescheduleEvent(EVENT_3, 12000);
        }

        void JustSummoned(Creature* summoned) override
        {
            summons.Summon(summoned);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_CALL_FOR_HELP)
                DoZoneInCombat();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(SPELL_DARK_MATTER);
                    events.RescheduleEvent(EVENT_1, 16000);
                    break;
                case EVENT_2:
                    DoCast(SPELL_DARK_SPLICE);
                    events.RescheduleEvent(EVENT_2, 20000);
                    break;
                case EVENT_3:
                    DoCast(SPELL_ESSENCE_SPLIT);
                    events.RescheduleEvent(EVENT_3, 25000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_grand_shadow_weaverAI(creature);
    }
};

//124964
class npc_nezhar_unstable_dark_matter : public CreatureScript
{
public:
    npc_nezhar_unstable_dark_matter() : CreatureScript("npc_nezhar_unstable_dark_matter") {}

    struct npc_nezhar_unstable_dark_matterAI : public ScriptedAI
    {
        npc_nezhar_unstable_dark_matterAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        EventMap events;

        void IsSummonedBy(Unit* summoner) override
        {
            events.RescheduleEvent(EVENT_1, 1000);
        }

        void Reset() override { }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(SPELL_COLLAPSE);
                    events.RescheduleEvent(EVENT_1, 5500);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_unstable_dark_matterAI(creature);
    }
};

//124967
class npc_nezhar_shadow_weaver_essence : public CreatureScript
{
public:
    npc_nezhar_shadow_weaver_essence() : CreatureScript("npc_nezhar_shadow_weaver_essence") {}

    struct npc_nezhar_shadow_weaver_essenceAI : public ScriptedAI
    {
        npc_nezhar_shadow_weaver_essenceAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 3000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(SPELL_DARK_SPLICE_1);
                    events.RescheduleEvent(EVENT_1, urand(8000,10000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_shadow_weaver_essenceAI(creature);
    }
};

//122404
class npc_nezhar_shadowguard_voidbender : public CreatureScript
{
public:
    npc_nezhar_shadowguard_voidbender() : CreatureScript("npc_nezhar_shadowguard_voidbender") {}

    struct npc_nezhar_shadowguard_voidbenderAI : public ScriptedAI
    {
        npc_nezhar_shadowguard_voidbenderAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, urand(3000, 6000));
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_CALL_FOR_HELP)
                DoZoneInCombat();
        }

        void UpdateAI(uint32 diff) override
        {

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(target, SPELL_ENTROPIC_MIST);
                    events.RescheduleEvent(EVENT_1, urand(5000, 8000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nezhar_shadowguard_voidbenderAI(creature);
    }
};

//122478
class npc_sott_void_discharge : public CreatureScript
{
public:
    npc_sott_void_discharge() : CreatureScript("npc_sott_void_discharge") {}

    struct npc_sott_void_dischargeAI : public ScriptedAI
    {
        npc_sott_void_dischargeAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 2000);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_CALL_FOR_HELP)
                DoZoneInCombat();
        }

        void JustDied(Unit* /*killer*/) override
        {
            DoCast(me, SPELL_DARK_OUTBREAK);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    DoCast(me, SPELL_CORRUPT_TOUCH);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sott_void_dischargeAI(creature);
    }
};

//244916
class spell_nezhar_void_lashing : public SpellScriptLoader
{
public:
    spell_nezhar_void_lashing() : SpellScriptLoader("spell_nezhar_void_lashing") { }

    class spell_nezhar_void_lashing_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_nezhar_void_lashing_AuraScript);

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            auto target = GetTarget();

            if (!target)
                return;

            target->CastSpell(target, SPELL_VOID_LASHING_DEBUFF, true);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_nezhar_void_lashing_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_nezhar_void_lashing_AuraScript();
    }
};

void AddSC_boss_viceroy_nezhar()
{
    new boss_viceroy_nezhar();
    new npc_nezhar_center_point();
    new npc_nezhar_tentacle();
    new npc_nezhar_shadowguard_voidtender();
    new npc_nezhar_void_flayer();
    new npc_nezhar_grand_shadow_weaver();
    new npc_nezhar_unstable_dark_matter();
    new npc_nezhar_shadow_weaver_essence();
    new npc_nezhar_shadowguard_voidbender();
    new npc_sott_void_discharge();
    new npc_sott_void_portal();
    new spell_nezhar_void_lashing();
}