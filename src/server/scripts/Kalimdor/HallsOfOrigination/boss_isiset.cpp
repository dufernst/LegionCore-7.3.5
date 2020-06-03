#include "halls_of_origination.h"

enum ScriptTexts
{
    SAY_DEATH       = 0,
    SAY_AGGRO       = 1,
    SAY_SUPERNOVA   = 2,
    SAY_KILL        = 3
};

enum Spells
{
    SPELL_SUPERNOVA             = 74136,
    SPELL_ASTRAL_RAIN1          = 74134,
    SPELL_ASTRAL_RAIN2          = 74365,
    SPELL_ASTRAL_RAIN3          = 74371,
    SPELL_CELESTIAL_CALL1       = 74362,
    SPELL_CELESTIAL_CALL2       = 74355,
    SPELL_CELESTIAL_CALL3       = 74364,
    SPELL_VEIL_OF_SKY1          = 74133,
    SPELL_VEIL_OF_SKY2          = 74372,
    SPELL_VEIL_OF_SKY3          = 74373,
    SPELL_ARCANE_BARRAGE        = 74374,
    SPELL_MIRROR_IMAGE_VS       = 74261, // summon veil of sky
    SPELL_MIRROR_IMAGE_AR       = 74262, // summon astral rain
    SPELL_MIRROR_IMAGE_CC       = 74263 // summon celestial call
};

enum Events
{
    EVENT_SUPERNOVA         = 1,
    EVENT_ASTRAL_RAIN       = 2,
    EVENT_CELESTIAL_CALL    = 3,
    EVENT_VEIL_OF_SKY       = 4,
    EVENT_ENTER_COMBAT      = 5, // for images
    EVENT_ARCANE_BARRAGE    = 7
};

enum Adds
{
    NPC_ASTRAL_RAIN             = 39720, // 74265
    NPC_CELESTIAL_CALL          = 39721, // 74289
    NPC_VEIL_OF_SKY             = 39722,
    NPC_ASTRAL_FAMILIAR         = 39795,
    NPC_SPATIAL_FLUX            = 48707, // spawnmask=0
    NPC_ISISET_ADD_CONTROLLER   = 42382
};

struct boss_isiset : public BossAI
{
    explicit boss_isiset(Creature* creature) : BossAI(creature, DATA_ISISET)
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
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
    }

    uint8 phase;
    bool Phased;
    bool AstralRain, VeilOfSky, CelestialCall;

    void Reset() override
    {
        _Reset();

        phase = 0;
        AstralRain = true;
        VeilOfSky = true;
        CelestialCall = true;

        me->SetVisible(true);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);

        events.ScheduleEvent(EVENT_SUPERNOVA, urand(7000, 10000));
        events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(9000, 15000));
        events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(10000, 17000));
        events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(6000, 10000));

        instance->SetBossState(DATA_ISISET, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Talk(SAY_KILL);
    }

    void JustDied(Unit* /*Killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        BossAI::SummonedCreatureDespawn(summon);

        if (phase == 1 || phase == 3)
        {
            switch (summon->GetEntry())
            {
            case NPC_ASTRAL_RAIN:
                AstralRain = false;
                phase++;
                summons.DespawnEntry(NPC_CELESTIAL_CALL);
                summons.DespawnEntry(NPC_VEIL_OF_SKY);
                me->SetVisible(true);
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_SUPERNOVA, urand(7000, 10000));
                events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(9000, 15000));
                events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(10000, 17000));
                events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(6000, 10000));
                break;
            case NPC_CELESTIAL_CALL:
                CelestialCall = false;
                phase++;
                summons.DespawnEntry(NPC_ASTRAL_RAIN);
                summons.DespawnEntry(NPC_VEIL_OF_SKY);
                me->SetVisible(true);
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_SUPERNOVA, urand(7000, 10000));
                events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(9000, 15000));
                events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(10000, 17000));
                events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(6000, 10000));
                break;
            case NPC_VEIL_OF_SKY:
                VeilOfSky = false;
                phase++;
                summons.DespawnEntry(NPC_CELESTIAL_CALL);
                summons.DespawnEntry(NPC_ASTRAL_RAIN);
                me->SetVisible(true);
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_SUPERNOVA, urand(7000, 10000));
                events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(9000, 15000));
                events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(10000, 17000));
                events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(6000, 10000));
                break;
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if ((me->HealthBelowPct(67) && phase == 0) ||
            (me->HealthBelowPct(34) && phase == 2))
        {
            phase++;
            events.Reset();
            me->AttackStop();
            me->SetReactState(REACT_PASSIVE);

            if (CelestialCall)
                DoCast(SPELL_MIRROR_IMAGE_CC);

            if (AstralRain)
                DoCast(SPELL_MIRROR_IMAGE_AR);

            if (VeilOfSky)
                DoCast(SPELL_MIRROR_IMAGE_VS);

            me->SetVisible(false);
            return;
        }

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SUPERNOVA:
                Talk(SAY_SUPERNOVA);
                DoCast(SPELL_SUPERNOVA);
                events.ScheduleEvent(EVENT_SUPERNOVA, urand(20000, 25000));
                break;
            case EVENT_ASTRAL_RAIN:
                if (AstralRain)
                {
                    if (phase == 0)
                        DoCast(SPELL_ASTRAL_RAIN1);
                    else if (phase == 2)
                        DoCast(SPELL_ASTRAL_RAIN2);
                    else if (phase == 4)
                        DoCast(SPELL_ASTRAL_RAIN3);

                    events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(15000, 22000));
                }
                break;
            case EVENT_CELESTIAL_CALL:
                if (CelestialCall)
                {
                    if (phase == 0)
                        DoCast(SPELL_CELESTIAL_CALL1);
                    else if (phase == 2)
                        DoCast(SPELL_CELESTIAL_CALL2);
                    else if (phase == 4)
                        DoCast(SPELL_CELESTIAL_CALL3);

                    events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(19000, 24000));
                }
                break;
            case EVENT_VEIL_OF_SKY:
                if (VeilOfSky)
                {
                    if (phase == 0)
                        DoCast(SPELL_VEIL_OF_SKY1);
                    else if (phase == 2)
                        DoCast(SPELL_VEIL_OF_SKY2);
                    else if (phase == 4)
                        DoCast(SPELL_VEIL_OF_SKY3);

                    events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(25000, 27000));
                }
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_isiset_astral_rain : public ScriptedAI
{
    explicit npc_isiset_astral_rain(Creature* creature) : ScriptedAI(creature)
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
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
        me->SetReactState(REACT_PASSIVE);
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/)
    {
        events.ScheduleEvent(EVENT_ASTRAL_RAIN, urand(5000, 15000));
        events.ScheduleEvent(EVENT_ENTER_COMBAT, 2000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ENTER_COMBAT:
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    me->GetMotionMaster()->MoveChase(victim);
                break;
            case EVENT_ASTRAL_RAIN:
                DoCast(SPELL_ASTRAL_RAIN1);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_isiset_celestial_call : public ScriptedAI
{
    explicit npc_isiset_celestial_call(Creature* creature) : ScriptedAI(creature)
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
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
        me->SetReactState(REACT_PASSIVE);
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_CELESTIAL_CALL, urand(5000, 15000));
        events.ScheduleEvent(EVENT_ENTER_COMBAT, 2000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ENTER_COMBAT:
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    me->GetMotionMaster()->MoveChase(victim);
                break;
            case EVENT_CELESTIAL_CALL:
                DoCast(SPELL_CELESTIAL_CALL1);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_isiset_veil_of_sky : public ScriptedAI
{
    explicit npc_isiset_veil_of_sky(Creature* creature) : ScriptedAI(creature)
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
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
        me->SetReactState(REACT_PASSIVE);
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_VEIL_OF_SKY, urand(5000, 15000));
        events.ScheduleEvent(EVENT_ENTER_COMBAT, 2000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ENTER_COMBAT:
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    me->GetMotionMaster()->MoveChase(victim);
                break;
            case EVENT_VEIL_OF_SKY:
                DoCast(SPELL_VEIL_OF_SKY1);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_isiset_astral_familiar : public ScriptedAI
{
    explicit npc_isiset_astral_familiar(Creature* creature) : ScriptedAI(creature)
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
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(3000, 7000));
    }

    void JustDied(Unit* /*killer*/)
    {
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ARCANE_BARRAGE:
                DoCast(SPELL_ARCANE_BARRAGE);
                events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(8000, 15000));
                break;
            }
        }
    }
};

class spell_isiset_supernova_dis : public SpellScript
{
    PrepareSpellScript(spell_isiset_supernova_dis);

    void FilterTargets(std::list<WorldObject*>& unitList)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        unitList.remove_if([caster](WorldObject* object)
        {
            if (!object->ToUnit()->isInFront(caster, 2.5f))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_isiset_supernova_dis::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

class spell_isiset_supernova_dmg : public SpellScript
{
    PrepareSpellScript(spell_isiset_supernova_dmg);

    void FilterTargets(std::list<WorldObject*>& unitList)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        unitList.remove_if([caster](WorldObject* object)
        {
            if (!object->ToUnit()->isInFront(caster, 2.5f))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_isiset_supernova_dmg::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

void AddSC_boss_isiset()
{
    RegisterCreatureAI(boss_isiset);
    RegisterCreatureAI(npc_isiset_astral_rain);
    RegisterCreatureAI(npc_isiset_celestial_call);
    RegisterCreatureAI(npc_isiset_veil_of_sky);
    RegisterCreatureAI(npc_isiset_astral_familiar);
    RegisterSpellScript(spell_isiset_supernova_dis);
    RegisterSpellScript(spell_isiset_supernova_dmg);
}
