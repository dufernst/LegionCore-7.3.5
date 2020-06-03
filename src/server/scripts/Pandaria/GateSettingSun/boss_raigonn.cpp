/*==============
==============*/

#include "gate_setting_sun.h"
#include "Spline.h"

enum eSpells
{
    // Raigonn
    SPELL_IMPERVIOUS_CARAPACE       = 107118,
    
    SPELL_BATTERING_HEADBUTT_EMOTE  = 118685,
    SPELL_BATTERING_HEADBUTT        = 111668,
    SPELL_BATTERING_STUN            = 130772,

    SPELL_BROKEN_CARAPACE           = 111742,
    SPELL_BROKEN_CARAPACE_DAMAGE    = 107146,
    SPELL_FIXATE                    = 78617,
    SPELL_STOMP                     = 34716,

    // Protectorat
    SPELL_HIVE_MIND                 = 107314,

    // Engulfer
    SPELL_ENGULFING_WINDS           = 107274,

    // Swarm Bringer
    SPELL_SCREECHING_SWARM          = 111600
};

enum ePhases
{
    PHASE_WEAK_SPOT     = 1,
    PHASE_VULNERABILITY = 2
};

enum eActions
{
    ACTION_WEAK_SPOT_DEAD   = 1
};

enum eEvents
{
    EVENT_CHANGE_PHASE          = 1,
    EVENT_RAIGONN_CHARGE        = 2,

    EVENT_SUMMON_PROTECTORAT    = 3,
    EVENT_SUMMON_ENGULFER       = 4,
    EVENT_SUMMON_SWARM_BRINGER  = 5,

    EVENT_FIXATE                = 6,
    EVENT_FIXATE_STOP           = 7,

    EVENT_STOMP                 = 8
};

enum eMovements
{
    POINT_MAIN_DOOR     = 1,
    POINT_HERSE         = 2
};

Position chargePos[4] =
{
    { 958.30f, 2386.92f, 297.43f, 0.0f },
    { 958.30f, 2458.59f, 300.29f, 0.0f },
    { 958.30f, 2241.68f, 296.10f, 0.0f },
    { 958.30f, 2330.15f, 296.18f, 0.0f }
};

struct boss_raigonn : public BossAI
{
    explicit boss_raigonn(Creature* creature) : BossAI(creature, DATA_RAIGONN)
    {
        me->SetVisible(false);
    }

    uint8 eventChargeProgress;
    uint32 eventChargeTimer;
    uint8 Phase;
    bool inFight;

    void Reset() override
    {
        _Reset();
        me->SetReactState(REACT_AGGRESSIVE);
        me->CombatStop();
        SetCanSeeEvenInPassiveMode(true);
        Phase = PHASE_WEAK_SPOT;
        inFight = false;
        eventChargeProgress = 0;
        me->RemoveAurasDueToSpell(SPELL_BROKEN_CARAPACE);
        me->RemoveAurasDueToSpell(SPELL_BROKEN_CARAPACE_DAMAGE);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        events.RescheduleEvent(EVENT_SUMMON_PROTECTORAT, urand(15000, 30000), 0, PHASE_WEAK_SPOT);
        events.RescheduleEvent(EVENT_SUMMON_ENGULFER, urand(15000, 30000), 0, PHASE_WEAK_SPOT);
        events.RescheduleEvent(EVENT_SUMMON_SWARM_BRINGER, urand(15000, 30000), 0, PHASE_WEAK_SPOT);
        events.RescheduleEvent(EVENT_CHANGE_PHASE, 60000, 0, PHASE_WEAK_SPOT);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_WEAK_SPOT_DEAD)
        {
            Phase = PHASE_VULNERABILITY;
            me->SetSpeed(MOVE_RUN, 1.1f, true);
            me->CastStop();
            me->RemoveAurasDueToSpell(SPELL_IMPERVIOUS_CARAPACE);
            DoCast(me, SPELL_BROKEN_CARAPACE_DAMAGE, true);
            events.CancelEventGroup(PHASE_WEAK_SPOT);
            events.RescheduleEvent(EVENT_FIXATE, 30000, PHASE_VULNERABILITY);
            events.RescheduleEvent(EVENT_STOMP, 16000, PHASE_VULNERABILITY);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!instance)
            return;

        events.Update(diff);

        switch (events.ExecuteEvent())
        {
        case EVENT_CHANGE_PHASE:
            events.CancelEvent(EVENT_CHANGE_PHASE);
            DoAction(ACTION_WEAK_SPOT_DEAD);
            break;
        case EVENT_SUMMON_PROTECTORAT:
            for (uint8 i = 0; i < 8; ++i)
                if (auto summon = me->SummonCreature(NPC_KRIKTHIK_PROTECTORAT, frand(941.0f, 974.0f), 2374.85f, 296.67f, 4.73f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        summon->AI()->AttackStart(target);

            events.RescheduleEvent(EVENT_SUMMON_PROTECTORAT, urand(30000, 45000), PHASE_WEAK_SPOT);
            break;
        case EVENT_SUMMON_ENGULFER:
            for (uint8 i = 0; i < 3; ++i)
                me->SummonCreature(NPC_KRIKTHIK_ENGULFER, frand(941.0f, 974.0f), me->GetPositionY(), me->GetPositionZ() + 30.0f, 4.73f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);

            events.RescheduleEvent(EVENT_SUMMON_ENGULFER, urand(95000, 105000), PHASE_WEAK_SPOT);
            break;
        case EVENT_SUMMON_SWARM_BRINGER:
            if (auto summon = me->SummonCreature(NPC_KRIKTHIK_SWARM_BRINGER, frand(941.0f, 974.0f), 2374.85f, 296.67f, 4.73f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000))
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    summon->AI()->AttackStart(target);

            events.RescheduleEvent(EVENT_SUMMON_ENGULFER, urand(35000, 50000), PHASE_WEAK_SPOT);
            break;
        case EVENT_FIXATE:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1))
            {
                DoCast(target, SPELL_FIXATE, true);
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->MoveChase(target);
            }
            events.RescheduleEvent(EVENT_FIXATE_STOP, 15000, PHASE_VULNERABILITY);
            break;
        case EVENT_FIXATE_STOP:
            me->SetReactState(REACT_AGGRESSIVE);
            me->GetMotionMaster()->Clear();

            if (auto target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                me->AI()->AttackStart(target);

            events.RescheduleEvent(EVENT_FIXATE, 30000, PHASE_VULNERABILITY);
            break;
        case EVENT_STOMP:
            DoCast(SPELL_STOMP);
            events.RescheduleEvent(EVENT_STOMP, 30000, PHASE_VULNERABILITY);
            break;
        default:
            break;
        }
        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }
};

struct npc_raigonn_weak_spot : public ScriptedAI
{
    npc_raigonn_weak_spot(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (damage >= me->GetHealth())
            if (instance)
                if (auto raigonn = instance->instance->GetCreature(instance->GetGuidData(NPC_RAIGONN)))
                    raigonn->AI()->DoAction(ACTION_WEAK_SPOT_DEAD);
    }
};

struct npc_krikthik_protectorat : public ScriptedAI
{
    explicit npc_krikthik_protectorat(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    bool hasCastHiveMind;

    void Reset() override
    {
        hasCastHiveMind = false;
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (!hasCastHiveMind && me->HealthBelowPctDamaged(20, damage))
        {
            DoCast(me, SPELL_HIVE_MIND, true);
            hasCastHiveMind = true;
        }
    }
};

struct npc_krikthik_engulfer : public ScriptedAI
{
    explicit npc_krikthik_engulfer(Creature* creature) : ScriptedAI(creature) {}

    uint32 engulfingTimer;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->MoveRandom(25.0f);
        DoZoneInCombat();

        engulfingTimer = 10000;
    }

    void UpdateAI(uint32 diff) override
    {
        if (engulfingTimer <= diff)
        {
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                DoCast(target, SPELL_ENGULFING_WINDS, false);

            engulfingTimer = urand(7500, 12500);
        }
        else
            engulfingTimer -= diff;
    }
};

struct npc_krikthik_swarm_bringer : public ScriptedAI
{
    npc_krikthik_swarm_bringer(Creature* creature) : ScriptedAI(creature) {}

    uint32 swarmTimer;

    void Reset() override
    {
        DoZoneInCombat();
        swarmTimer = 10000;
    }

    void UpdateAI(uint32 diff) override
    {
        if (swarmTimer <= diff)
        {
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                DoCast(target, SPELL_SCREECHING_SWARM, false);

            swarmTimer = urand(17500, 22500);
        }
        else
            swarmTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

class vehicle_artillery : public VehicleScript
{
    public:
        vehicle_artillery() : VehicleScript("vehicle_artillery") {}

        void OnAddPassenger(Vehicle* veh, Unit* /*passenger*/, int8 /*seatId*/) override
        {
            if (veh->GetBase())
                veh->GetBase()->GetAI()->DoAction(0);
        }

        struct vehicle_artilleryAI : public ScriptedAI
        {
            explicit vehicle_artilleryAI(Creature* creature) : ScriptedAI(creature)
            {
               instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            uint32 launchEventTimer;

            void Reset() override
            {
                launchEventTimer = 0;
            }

            void DoAction(int32 const action) override
            {
                launchEventTimer = 2500;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!launchEventTimer)
                    return;

                if (launchEventTimer <= diff)
                {
                    if (auto weakSpot = instance->instance->GetCreature(instance->GetGuidData(NPC_WEAK_SPOT)))
                    {
                        if (weakSpot->GetVehicleKit())
                        {
                            if (me->GetVehicleKit())
                            {
                                if (auto passenger = me->GetVehicleKit()->GetPassenger(0))
                                {
                                    passenger->ExitVehicle();

                                    uint32 maxSeatCount = 2;
                                    uint32 availableSeatCount = weakSpot->GetVehicleKit()->GetAvailableSeatCount();
                                    passenger->EnterVehicle(weakSpot,  maxSeatCount - availableSeatCount);
                                }
                            }
                        }
                    }
                    launchEventTimer = 0;
                }
                else
                    launchEventTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new vehicle_artilleryAI(creature);
        }
};

void AddSC_boss_raigonn()
{
    RegisterCreatureAI(boss_raigonn);
    RegisterCreatureAI(npc_raigonn_weak_spot);
    RegisterCreatureAI(npc_krikthik_protectorat);
    RegisterCreatureAI(npc_krikthik_engulfer);
    RegisterCreatureAI(npc_krikthik_swarm_bringer);
    new vehicle_artillery();
}
