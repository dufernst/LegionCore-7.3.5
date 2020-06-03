#include "bastion_of_twilight.h"
#include "ObjectVisitors.hpp"

enum SinestraScriptTexts
{
    SAY_SINESTRA_AGGRO      = 0,
    SAY_SINESTRA_DEATH      = 1,
    SAY_SINESTRA_KILL       = 2,
    SAY_SINESTRA_PHASE_2_2  = 3,
    SAY_SINESTRA_PHASE_2_1  = 4,
    SAY_SINESTRA_PHASE_3    = 6,
    SAY_SINESTRA_WHELP      = 7,
};

enum CalenScriptTexts
{
    SAY_CALEN_INTRO         = 0,
    SAY_CALEN_LOSE          = 1,
    SAY_CALEN_PHASE_2       = 2,
    SAY_CALEN_PHASE_3       = 3,
    SAY_CALEN_RECHARGE      = 4,
    SAY_CALEN_PHASE_2_2     = 5,
    SAY_CALEN_DEATH         = 6,
};

enum Spells
{
    // Sinestra
    SPELL_MANA_BARRIER              = 87299,
    SPELL_FLAME_BREATH              = 90125,
    SPELL_CALL_FLAMES               = 95855,
    SPELL_TWILIGHT_BLAST_DUMMY      = 87947,
    SPELL_TWILIGHT_BLAST_DMG        = 89280,
    SPELL_TWILIGHT_EXTINCTION       = 86227,
    SPELL_TWILIGHT_EXTINCTION2      = 86226,
    SPELL_TWILIGHT_EXTINCTION3      = 87945,
    SPELL_WRACK                     = 89421,
    SPELL_TWILIGHT_POWER            = 87220,
    SPELL_TWILIGHT_FLAMES           = 95823,
    SPELL_TWILIGHT_FLAME_DMG        = 95822,
    SPELL_DRAINED                   = 89350,

    // Shadow Orb
    SPELL_TWILIGHT_PULSE            = 92957,
    SPELL_TWILIGHT_SLICER           = 92851,
    SPELL_TWILIGHT_INFUSION_M       = 95564,

    // Pulsing Twilight Egg
    SPELL_TWILIGHT_CARAPACE         = 87654,
    SPELL_TWILIGHT_INFUSION         = 87655,

    // Twilight Spitecaller
    SPELL_INDOMITABLE_DUMMY         = 90044,
    SPELL_UNLEASH_ESSENCE           = 90028,
    SPELL_INDOMITABLE_DMG           = 90045,

    // Twilight Drake
    SPELL_ABSORB_ESSENCE            = 90107,
    SPELL_TWILIGHT_BREATH           = 90083,

    // Twilight Whelp
    SPELL_TWILIGHT_SPIT             = 89299,
    SPELL_TWILIGHT_ESSENCE          = 89284,
    SPELL_TWILIGHT_ESSENCE_GROW     = 89288,

    // Calen
    SPELL_PYRRHIC_FOCUS             = 87323,
    SPELL_ESSENCE_OF_THE_RED        = 87946,
    SPELL_FIERY_RESOLVE             = 87221,
    SPELL_FIERY_BARRIER             = 87229,
    SPELL_FIERY_BARRIER_AURA        = 87231,
    SPELL_FIERY_BARRIER_DUMMY       = 95791,
};

enum Adds
{
    NPC_BARRIER_COSMETIC_STALKER    = 51608,
    NPC_CALEN                       = 46277,
    NPC_PULSING_TWILIGHT_EGG        = 46842,
    NPC_TWILIGHT_SPITECALLER        = 48415,
    NPC_TWILIGHT_WHELP              = 47265,
    NPC_TWILIGHT_DRAKE              = 48436,
    NPC_SHADOW_ORB                  = 49863,
    NPC_TWILIGHT_ESSENCE            = 48018,
    NPC_CONVECTIVE_FLAMES           = 46588, // wrong stalker
};

enum Events
{
    EVENT_FLAME_BREATH              = 1,
    EVENT_WRACK                     = 2,
    EVENT_SUMMON_DRAKE              = 3,
    EVENT_SUMMON_WHELP              = 4,
    EVENT_SUMMON_CALLER             = 5,
    EVENT_TWILIGHT_EXTINCTION       = 6,
    EVENT_CALL_FLAMES               = 7,
    EVENT_SUMMON_CALEN              = 8,
    EVENT_TALK_SINESTRA_PHASE_2_2   = 9,
    EVENT_CONTINUE_PHASE_2          = 10,
    EVENT_TWILIGHT_POWER            = 11,
    EVENT_SUMMON_ORB                = 12,
    EVENT_TWILIGHT_SLICER           = 13,
    EVENT_KILL_CALEN                = 14,
    EVENT_START_PHASE_3             = 15,
    EVENT_WIPE                      = 16,
    EVENT_MELEE_CHECK               = 17,
    

    EVENT_UNLEASH_ESSENCE,
    EVENT_INDOMITABLE,

    EVENT_FIERY_RESOLVE,
    EVENT_CALEN_BLAST,
    EVENT_CALEN_BUFF,
    EVENT_CALEN_DEATH,

    EVENT_SELECT_TARGET,
    EVENT_START_MOVE,

    EVENT_START_ATTACK,
    EVENT_TWILIGHT_SPIT,
    EVENT_SUMMON_ESSENCE,
    EVENT_CHECK_RESURRECT,
    EVENT_RESURRECT,
    
    EVENT_ESSENCE_GROW,

    EVENT_TWILIGHT_BREATH,
    //EVENT_CHECK_ESSENCE,
};

enum Others
{
    TYPE_PHASE,
    TYPE_RESURRECT,
    ACTION_EGG,
    ACTION_WIPE,
    ACTION_START_EGG,
    ACTION_END_EGG,
};

const Position addsPos[16] = 
{
    {-1007.830f, -807.280f, 438.600f, 0.870f},  // 0 Calen

    {-904.470f, -769.060f, 440.880f, 0.00f},    // 1 Egg 1
    {-928.210f, -774.860f, 440.010f, 0.00f},    // 2 Flame 1
    {-997.100f, -685.830f, 440.600f, 0.00f},    // 3 Egg 2
    {-997.770f, -719.610f, 438.500f, 0.00f},    // 4 Flame 2


    {-917.980f, -818.250f, 464.710f, 2.780f},   // 5 Drake fly 1
    {-1049.920f, -708.420f, 472.740f, 5.270f},  // 6 Drake fly 2
    {-969.860f, -795.980f, 438.600f, 2.630f},   // 7 Drake land 1
    {-1005.740f, -766.530f, 438.600f, 5.750f},  // 8 Drake land 2
    
    {-1035.88f, -841.07f, 442.99f, 0.97f},      // 9 Caller spawn
    {-1012.74f, -812.81f, 438.60f, 0.86f},      // 10 Caller land

    {-1008.120f, -783.720f, 438.600f, 0.130f},  // 11 Whelp 1
    {-995.670f, -784.270f, 438.600f, 6.220f},   // 12 Whelp 2
    {-983.870f, -790.680f, 438.600f, 5.620f},   // 13 Whelp 3

    {-997.45f, -796.70f, 446.01f, 0.0f},        // 14 Stalker near Calen
    {-976.52f, -776.35f, 458.58f, 0.0f},        // 15 Stalker near Sinestra
//    {-989.62f, -788.95f, 449.97f, 0.0f},      // 16 Stalker between
};

#define SINESTRA_HEALTH_10H 25767600
#define SINESTRA_HEALTH_25H 77302800

// Custom check for "dead" whelps
/*class WhelpCheck
{
    public:
        WhelpCheck(Unit const* obj, uint32 entry, ObjectGuid guid) : i_obj(obj), i_entry(entry), i_guid(guid) {}
        bool operator()(Creature const* creature) const
        {
            return (creature->GetEntry() == i_entry && creature->AI()->GetData(TYPE_RESURRECT) && creature->GetGUID() != i_guid);
        }
    private:
        Unit const* i_obj;
        uint32 i_entry;
        ObjectGuid i_guid;
};*/

class TwilightSlicerTargetSelector
{
    public:
        TwilightSlicerTargetSelector(Unit* caster, Unit* target) : i_caster(caster), i_target(target) { }

        bool operator()(WorldObject* unit)
        {
            if (unit->IsInBetween(i_caster, i_target, 1.0f))
                return false;
            return true;
        }

        Unit* i_caster;
        Unit* i_target;
};

class ExactDistanceCheck
{
    public:
        ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) {}

        bool operator()(WorldObject* unit)
        {
            return _source->GetExactDist2d(unit) > _dist;
        }

    private:
        Unit* _source;
        float _dist;
};

class boss_sinestra : public CreatureScript
{
    public:
        boss_sinestra() : CreatureScript("boss_sinestra") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_sinestraAI(pCreature);
        }

        struct boss_sinestraAI : public Scripted_NoMovementAI
        {
            boss_sinestraAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature), summons(me)
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
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;
            SummonList summons;
            uint8 phase;
            Creature* pEgg1;
            Creature* pEgg2;
            Creature* pCalen;
            Creature* pOrb1;
            Creature* pOrb2;
            uint8 eggs;

            void Reset()
            {
                summons.DespawnAll();
                events.Reset();

                DoCast(me, SPELL_DRAINED);
                phase = 0;
                eggs = 0;

                me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 15);
                me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 15);

                me->SetHealth(RAID_MODE(SINESTRA_HEALTH_10H, SINESTRA_HEALTH_25H, SINESTRA_HEALTH_10H, SINESTRA_HEALTH_25H));

                pEgg1 = me->SummonCreature(NPC_PULSING_TWILIGHT_EGG, addsPos[1], TEMPSUMMON_DEAD_DESPAWN);
                pEgg2 = me->SummonCreature(NPC_PULSING_TWILIGHT_EGG, addsPos[3], TEMPSUMMON_DEAD_DESPAWN);

                pOrb1 = NULL;
                pOrb2 = NULL;
                pCalen = NULL;

                if (!instance)
                    return;
                instance->SetBossState(DATA_SINESTRA, NOT_STARTED);
            }

            void EnterCombat(Unit* attacker)
            {
                if (instance)
                {
                    if (!instance->CheckRequiredBosses(DATA_SINESTRA, me->GetEntry()))
                    {
                        EnterEvadeMode();

                        Map::PlayerList const &pList = me->GetMap()->GetPlayers();
                        if (pList.isEmpty())
                            return;

                        for (Map::PlayerList::const_iterator itr = pList.begin(); itr != pList.end(); ++itr)
                            if (Player* pPlayer = itr->getSource())
                                pPlayer->NearTeleportTo(enterPos.GetPositionX(), enterPos.GetPositionY(), enterPos.GetPositionZ(), enterPos.GetOrientation());

                        
                        return;
                    }
                }
                me->SummonCreature(NPC_CONVECTIVE_FLAMES, addsPos[2]);
                me->SummonCreature(NPC_CONVECTIVE_FLAMES, addsPos[4]);

                summons.DoZoneInCombat(NPC_PULSING_TWILIGHT_EGG);

                events.RescheduleEvent(EVENT_CALL_FLAMES, 1500);
                events.RescheduleEvent(EVENT_FLAME_BREATH, 25000);
                events.RescheduleEvent(EVENT_SUMMON_ORB, 27000);
                events.RescheduleEvent(EVENT_SUMMON_WHELP, 10000);
                events.RescheduleEvent(EVENT_WRACK, 15000);
                events.RescheduleEvent(EVENT_MELEE_CHECK, 5000);


                Talk(SAY_SINESTRA_AGGRO);

                if (!instance)
                    return;
                DoZoneInCombat();
                instance->SetBossState(DATA_SINESTRA, IN_PROGRESS);
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_EGG)
                {
                    eggs++;
                    if (eggs >= 2)
                    {
                        Talk(SAY_SINESTRA_PHASE_3);
                        me->InterruptNonMeleeSpells(false);
                        if (Creature* pStalker = me->FindNearestCreature(NPC_BARRIER_COSMETIC_STALKER, 200.0f))
                            pStalker->DespawnOrUnsummon();
                        events.Reset();
                        events.RescheduleEvent(EVENT_KILL_CALEN, 11000);
                    }
                }
                else if (action == ACTION_WIPE)
                {
                    events.RescheduleEvent(EVENT_WIPE, 5000);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_SINESTRA_DEATH);

                summons.DespawnAll();

                if (!instance)
                    return;
                instance->SetBossState(DATA_SINESTRA, DONE);
            }
            
            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                if (me->isInCombat())
                    summon->SetInCombatWithZone();
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void KilledUnit(Unit* who)
            {
                Talk(SAY_SINESTRA_KILL);
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case TYPE_PHASE:
                        return phase;
                }
                return 0;
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING) && (phase == 0 || phase == 3))
                    return;

                if (me->HealthBelowPct(30) && phase == 0)
                {
                    phase = 1;
                    events.Reset();
                    me->RemoveAllAuras();
                    me->SetReactState(REACT_PASSIVE);
                    Talk(SAY_SINESTRA_PHASE_2_1);
                    DoCast(me, SPELL_MANA_BARRIER);
                    events.RescheduleEvent(EVENT_SUMMON_CALEN, 4000);
                    events.RescheduleEvent(EVENT_TWILIGHT_EXTINCTION, 3000);
                    events.RescheduleEvent(EVENT_TWILIGHT_POWER, 21000);
                    summons.DespawnEntry(NPC_CONVECTIVE_FLAMES);
                    return;
                }
                else if (me->GetPower(POWER_MANA) < 1 && phase == 1)
                {
                    phase = 2;
                    if (pCalen)
                        pCalen->AI()->Talk(SAY_CALEN_PHASE_2_2);
                    if (Creature* pStalker = me->FindNearestCreature(NPC_BARRIER_COSMETIC_STALKER, 100.0f))
                    {
                        pStalker->GetMotionMaster()->MovementExpired(false);
                        pStalker->GetMotionMaster()->MovePoint(0, addsPos[15]);
                    }
                    events.RescheduleEvent(EVENT_TALK_SINESTRA_PHASE_2_2, 6000);
                    return;
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MELEE_CHECK:
                            if (!me->IsWithinMeleeRange(me->getVictim()))
                                DoCast(me->getVictim(), SPELL_TWILIGHT_BLAST_DMG);
                            events.RescheduleEvent(EVENT_MELEE_CHECK, 2000);
                            break;
                        case EVENT_WIPE:
                            DoCastAOE(SPELL_TWILIGHT_EXTINCTION3);
                            EnterEvadeMode();
                            break;
                        case EVENT_CALL_FLAMES:
                            DoCastAOE(SPELL_CALL_FLAMES);
                            break;
                        case EVENT_FLAME_BREATH:
                            DoCastAOE(SPELL_FLAME_BREATH);
                            events.RescheduleEvent(EVENT_FLAME_BREATH, 25000);
                            break;
                        case EVENT_SUMMON_ORB:
                        {

                            std::list<Unit*> targetList;

                            const std::list<HostileReference*> &threatlist = me->getThreatManager().getThreatList();

                            if (threatlist.empty())
                                return;

                            DefaultTargetSelector targetSelector(me, 0.0f, true, 0);
                            for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                                if (targetSelector((*itr)->getTarget()) && me->getVictim() != (*itr)->getTarget())
                                    targetList.push_back((*itr)->getTarget());

                            if (targetList.size() < 2)
                                return;

                            Trinity::Containers::RandomResizeList(targetList, 2);

                            std::list<Unit*>::const_iterator iter = targetList.begin();
                            pOrb1 = me->SummonCreature(NPC_SHADOW_ORB, (*iter)->GetPositionX(), (*iter)->GetPositionY(), (*iter)->GetPositionZ(), (*iter)->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 14000);
                            
                            ++iter;
                            pOrb2 = me->SummonCreature(NPC_SHADOW_ORB, (*iter)->GetPositionX(), (*iter)->GetPositionY(), (*iter)->GetPositionZ(), (*iter)->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 14000);

                            events.RescheduleEvent(EVENT_TWILIGHT_SLICER, 5500);
                            events.RescheduleEvent(EVENT_SUMMON_ORB, 30000);
                            break;
                        }
                        case EVENT_TWILIGHT_SLICER:
                            if (pOrb1 && pOrb2)
                            {
                                
                                pOrb1->CastSpell(pOrb2, SPELL_TWILIGHT_SLICER, true);
                                pOrb1->ClearUnitState(UNIT_STATE_CASTING);
                            }
                            break;
                        case EVENT_SUMMON_WHELP:
                            Talk(SAY_SINESTRA_WHELP);
                            for (uint8 i = 0; i < 5; ++i)
                            {
                                me->SummonCreature(NPC_TWILIGHT_WHELP, addsPos[urand(11, 13)]);
                            }
                            events.RescheduleEvent(EVENT_SUMMON_WHELP, urand(45000, 50000));
                            break;
                        case EVENT_WRACK:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(pTarget, SPELL_WRACK);
                            events.RescheduleEvent(EVENT_WRACK, 61000);
                            break;
                        case EVENT_TWILIGHT_EXTINCTION:
                            DoCast(me, SPELL_TWILIGHT_EXTINCTION);
                            break;
                        case EVENT_SUMMON_CALEN:
                            pCalen = me->SummonCreature(NPC_CALEN, addsPos[0]);
                            break;
                        case EVENT_TWILIGHT_POWER:
                            if (Creature* pStalker = me->FindNearestCreature(NPC_BARRIER_COSMETIC_STALKER, 200.0f))
                                DoCast(pStalker, SPELL_TWILIGHT_POWER);
                            events.RescheduleEvent(EVENT_SUMMON_CALLER, 25000);
                            events.RescheduleEvent(EVENT_SUMMON_DRAKE, 50000);
                            break;
                        case EVENT_TALK_SINESTRA_PHASE_2_2:
                        {
                            Talk(SAY_SINESTRA_PHASE_2_2);
                            EntryCheckPredicate pred(NPC_PULSING_TWILIGHT_EGG);
                            summons.DoAction(ACTION_START_EGG, pred);
                            
                            me->SetPower(POWER_MANA, me->GetMaxPower(POWER_MANA));
                            events.RescheduleEvent(EVENT_CONTINUE_PHASE_2, 30000);
                            break;
                        }
                        case EVENT_CONTINUE_PHASE_2:
                        {
                            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_INFUSION);
                            EntryCheckPredicate pred(NPC_PULSING_TWILIGHT_EGG);
                            summons.DoAction(ACTION_END_EGG, pred);
                            if (Creature* pStalker = me->FindNearestCreature(NPC_BARRIER_COSMETIC_STALKER, 100.0f))
                            {
                                pStalker->GetMotionMaster()->MovementExpired(false);
                                pStalker->GetMotionMaster()->MovePoint(0, addsPos[14]);
                            }
                            phase = 1;
                            break;
                        }
                        case EVENT_SUMMON_CALLER:
                            if (Creature* pCaller = me->SummonCreature(NPC_TWILIGHT_SPITECALLER, addsPos[9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                                pCaller->GetMotionMaster()->MovePoint(0, addsPos[10]);
                            events.RescheduleEvent(EVENT_SUMMON_CALLER, 35000);
                            break;
                        case EVENT_SUMMON_DRAKE:
                        {
                            uint8 i = urand(0, 1);
                            if (Creature* pDrake = me->SummonCreature(NPC_TWILIGHT_DRAKE, addsPos[5+i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                                pDrake->GetMotionMaster()->MovePoint(0, addsPos[7+i]);
                            events.RescheduleEvent(EVENT_SUMMON_DRAKE, 50000);
                            break;
                        }
                        case EVENT_KILL_CALEN:
                            if (pCalen)
                                DoCast(pCalen, SPELL_TWILIGHT_BLAST_DUMMY);
                            events.RescheduleEvent(EVENT_START_PHASE_3, 18000);
                            break;
                        case EVENT_START_PHASE_3:
                            phase = 3;
                            me->RemoveAurasDueToSpell(SPELL_MANA_BARRIER);
                            me->SummonCreature(NPC_CONVECTIVE_FLAMES, addsPos[2]);
                            me->SummonCreature(NPC_CONVECTIVE_FLAMES, addsPos[4]);
                            DoCastAOE(SPELL_CALL_FLAMES);
                            events.RescheduleEvent(EVENT_FLAME_BREATH, 25000);
                            events.RescheduleEvent(EVENT_SUMMON_ORB, 27000);
                            events.RescheduleEvent(EVENT_SUMMON_WHELP, 10000);
                            events.RescheduleEvent(EVENT_WRACK, 15000);
                            events.RescheduleEvent(EVENT_MELEE_CHECK, 5000);
                            break;
                    }
                }

                if (phase == 0 || phase == 3)
                    DoMeleeAttackIfReady();
            }
        };
};

class npc_sinestra_calen : public CreatureScript
{
    public:
        npc_sinestra_calen() : CreatureScript("npc_sinestra_calen") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_calenAI(pCreature);
        }

        struct npc_sinestra_calenAI : public Scripted_NoMovementAI
        {
            npc_sinestra_calenAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature), summons(me)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            EventMap events;
            SummonList summons;
            bool bEventDeath;
            bool bLowHealth;

            void Reset()
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* summoner)
            {
                bEventDeath = false;
                bLowHealth = false;
                Talk(SAY_CALEN_INTRO);
                DoCast(me, SPELL_FIERY_BARRIER_DUMMY, true);
                DoCast(me, SPELL_FIERY_BARRIER);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                if (summon->GetEntry() == NPC_BARRIER_COSMETIC_STALKER)
                {
                    events.RescheduleEvent(EVENT_FIERY_RESOLVE, 21000);
                }
            }

            void SpellHit(Unit* caster, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_TWILIGHT_BLAST_DUMMY)
                {
                    me->RemoveAllAuras();
                    bEventDeath = true;
                    events.RescheduleEvent(EVENT_CALEN_BLAST, 2000);
                }
            }

            void JustDied(Unit* killer)
            {
                summons.DespawnAll();
                
                if (!bEventDeath)
                {
                    Talk(SAY_CALEN_DEATH);
                    if (Creature* pSinestra = me->FindNearestCreature(NPC_SINESTRA, 100.0f))
                        pSinestra->AI()->DoAction(ACTION_WIPE);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (me->HealthBelowPct(35) && !bLowHealth)
                {
                    Talk(SAY_CALEN_LOSE);
                    bLowHealth = true;
                }
                else if (me->GetHealthPct() == 100 && bLowHealth)
                    bLowHealth = false;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIERY_RESOLVE:
                            Talk(SAY_CALEN_PHASE_2);
                            me->RemoveAurasDueToSpell(SPELL_FIERY_BARRIER_DUMMY);
                            me->RemoveAurasDueToSpell(SPELL_FIERY_BARRIER);
                            DoCast(me, SPELL_PYRRHIC_FOCUS, true);
                            if (Creature* pStalker = me->FindNearestCreature(NPC_BARRIER_COSMETIC_STALKER, 200.0f))
                                DoCast(pStalker, SPELL_FIERY_RESOLVE);
                            break;
                        case EVENT_CALEN_BLAST:
                            Talk(SAY_CALEN_PHASE_3);
                            events.RescheduleEvent(EVENT_CALEN_BUFF, 5000);
                            break;
                        case EVENT_CALEN_BUFF:
                            DoCastAOE(SPELL_ESSENCE_OF_THE_RED);
                            events.RescheduleEvent(EVENT_CALEN_DEATH, 2000);
                            break;
                        case EVENT_CALEN_DEATH:
                            me->Kill(me);
                            break;
                    }
                }
            }
        };
};

class npc_sinestra_twilight_whelp : public CreatureScript
{
    public:
        npc_sinestra_twilight_whelp() : CreatureScript("npc_sinestra_twilight_whelp") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_twilight_whelpAI(pCreature);
        }

        struct npc_sinestra_twilight_whelpAI : public ScriptedAI
        {
            npc_sinestra_twilight_whelpAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
            }

            EventMap events;
            bool bDead;

            void Reset()
            {
                events.Reset();
                DoZoneInCombat();
            }

            void IsSummonedBy(Unit* summoner)
            {
                bDead = false;
                events.RescheduleEvent(EVENT_TWILIGHT_SPIT, urand(7000, 15000));
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth() && !bDead)
                {
                    bDead = true;
                    damage = 0;
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    events.Reset();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    me->RemoveAllAuras();
                    events.RescheduleEvent(EVENT_SUMMON_ESSENCE, 1500);
                    events.RescheduleEvent(EVENT_RESURRECT, 5000);
                }
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TWILIGHT_SPIT:
                            DoCast(me->getVictim(), SPELL_TWILIGHT_SPIT);
                            events.RescheduleEvent(EVENT_TWILIGHT_SPIT, urand(7000, 15000));
                            break;
                        case EVENT_SUMMON_ESSENCE:
                            if (Creature* pSinestra = me->FindNearestCreature(NPC_SINESTRA, 200.0f))
                                pSinestra->SummonCreature(NPC_TWILIGHT_ESSENCE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                            break; 
                        case EVENT_RESURRECT:
                            me->SetReactState(REACT_AGGRESSIVE);
                            events.RescheduleEvent(EVENT_TWILIGHT_SPIT, urand(7000, 15000));
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            me->SetHealth(me->GetMaxHealth());
                            me->GetMotionMaster()->MoveChase(me->getVictim());
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_sinestra_twilight_flames : public CreatureScript
{
    public:
        npc_sinestra_twilight_flames() : CreatureScript("npc_sinestra_twilight_flames") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_twilight_flamesAI(pCreature);
        }

        struct npc_sinestra_twilight_flamesAI : public Scripted_NoMovementAI
        {
            npc_sinestra_twilight_flamesAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void SpellHit(Unit* caster, const SpellInfo* SpellInfo)
            {
                if (SpellInfo->Id == SPELL_CALL_FLAMES)
                    DoCast(me, SPELL_TWILIGHT_FLAMES, true);
            }
        };
};

class npc_sinestra_twilight_essence : public CreatureScript
{
    public:
        npc_sinestra_twilight_essence() : CreatureScript("npc_sinestra_twilight_essence") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_twilight_essenceAI(pCreature);
        }

        struct npc_sinestra_twilight_essenceAI : public Scripted_NoMovementAI
        {
            npc_sinestra_twilight_essenceAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
            }

            EventMap events;

            void Reset()
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                DoCast(me, SPELL_TWILIGHT_ESSENCE, true);
                events.RescheduleEvent(EVENT_ESSENCE_GROW, 30000);
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ESSENCE_GROW:
                            DoCast(me, SPELL_TWILIGHT_ESSENCE_GROW, true);
                            events.RescheduleEvent(EVENT_ESSENCE_GROW, 30000);
                            break;
                    }
                }
            }
        };
};

class npc_sinestra_barrier_stalker : public CreatureScript
{
    public:
        npc_sinestra_barrier_stalker() : CreatureScript("npc_sinestra_barrier_stalker") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_barrier_stalkerAI(pCreature);
        }

        struct npc_sinestra_barrier_stalkerAI : public Scripted_NoMovementAI
        {
            npc_sinestra_barrier_stalkerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetSpeed(MOVE_FLIGHT, 0.5f);
                me->SetSpeed(MOVE_WALK, 0.5f);
                me->SetSpeed(MOVE_RUN, 0.5f);
                me->SetReactState(REACT_PASSIVE);
            }

            Unit* pTarget;

            void Reset()
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->NearTeleportTo(addsPos[14].GetPositionX(), addsPos[14].GetPositionY(), addsPos[14].GetPositionZ(), addsPos[14].GetOrientation());
            }

            void UpdateAI(uint32 diff)
            {
            }
        };
};

class npc_sinestra_shadow_orb : public CreatureScript
{
    public:
        npc_sinestra_shadow_orb() : CreatureScript("npc_sinestra_shadow_orb") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_shadow_orbAI(pCreature);
        }

        struct npc_sinestra_shadow_orbAI : public Scripted_NoMovementAI
        {
            npc_sinestra_shadow_orbAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetSpeed(MOVE_FLIGHT, 0.65f);
                me->SetSpeed(MOVE_WALK, 0.65f);
                me->SetSpeed(MOVE_RUN, 0.65f);
                me->SetReactState(REACT_PASSIVE);
            }

            EventMap events;
            Unit* pTarget;

            void Reset()
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                events.RescheduleEvent(EVENT_SELECT_TARGET, 2000);
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SELECT_TARGET:
                            pTarget = me->SelectNearestTarget();
                            if (pTarget)
                            {
                                DoCast(pTarget, SPELL_TWILIGHT_INFUSION_M);
                                me->ClearUnitState(UNIT_STATE_CASTING);
                                events.RescheduleEvent(EVENT_START_MOVE, 3000);
                                me->GetMotionMaster()->MoveFollow(pTarget, 0.0f, 0.0f);
                            }
                            else
                                me->DespawnOrUnsummon();
                            break;
                        case EVENT_START_MOVE:
                            DoCast(me, SPELL_TWILIGHT_PULSE, true);                                
                            break;
                    }
                }
            }
        };
};

class npc_sinestra_pulsing_twilight_egg : public CreatureScript
{
    public:
        npc_sinestra_pulsing_twilight_egg() : CreatureScript("npc_sinestra_pulsing_twilight_egg") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_pulsing_twilight_eggAI(pCreature);
        }

        struct npc_sinestra_pulsing_twilight_eggAI : public Scripted_NoMovementAI
        {
            npc_sinestra_pulsing_twilight_eggAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                DoCast(me, SPELL_TWILIGHT_CARAPACE);
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_START_EGG)
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveAllAuras();
                    if (Creature* pSinestra = me->FindNearestCreature(NPC_SINESTRA, 200.0f))
                        DoCast(pSinestra, SPELL_TWILIGHT_INFUSION);
                }
                else if (action == ACTION_END_EGG)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    DoCast(me, SPELL_TWILIGHT_CARAPACE, true);
                }
            }

            void JustDied(Unit* killer)
            {
                if (Creature* pSinestra = me->FindNearestCreature(NPC_SINESTRA, 200.0f))
                    pSinestra->AI()->DoAction(ACTION_EGG);
                me->DespawnOrUnsummon();
            }
        };
};

class npc_sinestra_twilight_spitecaller : public CreatureScript{
    public:
        npc_sinestra_twilight_spitecaller() : CreatureScript("npc_sinestra_twilight_spitecaller") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_twilight_spitecallerAI(pCreature);
        }

        struct npc_sinestra_twilight_spitecallerAI : public ScriptedAI 
        {
            npc_sinestra_twilight_spitecallerAI(Creature * pCreature) : ScriptedAI(pCreature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* owner)
            {
                DoCast(me, SPELL_INDOMITABLE_DUMMY, true);
                events.RescheduleEvent(EVENT_UNLEASH_ESSENCE, 8000);
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_UNLEASH_ESSENCE:
                            DoCastAOE(SPELL_UNLEASH_ESSENCE);
                            events.RescheduleEvent(EVENT_UNLEASH_ESSENCE, urand(8000, 15000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_sinestra_twilight_drake : public CreatureScript{
    public:
        npc_sinestra_twilight_drake() : CreatureScript("npc_sinestra_twilight_drake") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_sinestra_twilight_drakeAI(pCreature);
        }

        struct npc_sinestra_twilight_drakeAI : public ScriptedAI 
        {
            npc_sinestra_twilight_drakeAI(Creature * pCreature) : ScriptedAI(pCreature)
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

            EventMap events;

            void Reset()
            {
            }

            void EnterCombat(Unit* attacker)
            {

            }

            void JustDied(Unit* killer)
            {
                me->SetCanFly(false);
            }

            void IsSummonedBy(Unit* summoner)
            {
                events.RescheduleEvent(EVENT_TWILIGHT_BREATH, urand(12000, 15000));
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_TWILIGHT_BREATH:
                            DoCast(me, SPELL_TWILIGHT_BREATH);
                            events.RescheduleEvent(EVENT_TWILIGHT_BREATH, urand(12000, 15000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class spell_sinestra_wrack : public SpellScriptLoader
{
    public:
        spell_sinestra_wrack() : SpellScriptLoader("spell_sinestra_wrack") { }

        class spell_sinestra_wrack_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_wrack_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
                    return;

                std::list<Player*> targets;
                Trinity::AnyPlayerInObjectRangeCheck checker(GetTarget(), 100.0f, true);
                Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(GetTarget(), targets, checker); 
                Trinity::VisitNearbyWorldObject(GetTarget(), 100.0f, searcher);
                targets.sort(Trinity::ObjectDistanceOrderPred(GetTarget()));
                uint8 count = 0;
                if (!targets.empty())
                    for (std::list<Player*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    {
                        if ((*itr) == GetTarget())
                            continue;

                        count++;
                        if (count > 2)
                            break;

                        if (Aura* aur = GetTarget()->AddAura(SPELL_WRACK, (*itr)))
                        {
                            //aur->SetMaxDuration(GetDuration());
                            aur->SetDuration(GetDuration());
                        } 
                    }
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_sinestra_wrack_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_wrack_AuraScript();
        }
};

class spell_sinestra_twilight_extinction : public SpellScriptLoader
{
    public:
        spell_sinestra_twilight_extinction() : SpellScriptLoader("spell_sinestra_twilight_extinction") { }

        class spell_sinestra_twilight_extinction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_twilight_extinction_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_TWILIGHT_EXTINCTION2, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_sinestra_twilight_extinction_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_twilight_extinction_AuraScript();
        }
};

class spell_sinestra_indomitable : public SpellScriptLoader
{
    public:
        spell_sinestra_indomitable() : SpellScriptLoader("spell_sinestra_indomitable") { }

        class spell_sinestra_indomitable_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_indomitable_AuraScript);

            void HandleScript(AuraEffect const* /*aurEff*/)
            {
                if (!GetCaster())
                    return;

                static uint8 controlTimer;

                if (GetCaster()->CanFreeMove())
                {
                    controlTimer = 0;
                    return;
                }

                controlTimer++;

                if (controlTimer >= 5)
                {
                    GetCaster()->CastSpell(GetTarget(), SPELL_INDOMITABLE_DMG, true);
                    return;
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sinestra_indomitable_AuraScript::HandleScript, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_indomitable_AuraScript();
        }
};

class spell_sinestra_indomitable_aura : public SpellScriptLoader
{
    public:
        spell_sinestra_indomitable_aura() : SpellScriptLoader("spell_sinestra_indomitable_aura") { }

        class spell_sinestra_indomitable_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_indomitable_aura_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->ApplyUberImmune(0, true);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->ApplyUberImmune(0, false);
            }

            void Register()
            {
                OnEffectApply += AuraEffectRemoveFn(spell_sinestra_indomitable_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY_MASK, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_sinestra_indomitable_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY_MASK, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_indomitable_aura_AuraScript();
        }
};

class spell_sinestra_mana_barrier : public SpellScriptLoader
{
    public:
        spell_sinestra_mana_barrier() : SpellScriptLoader("spell_sinestra_mana_barrier") { }

        class spell_sinestra_mana_barrier_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_mana_barrier_AuraScript);

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                {
                    int32 missingHealth = int32(caster->GetMaxHealth() - caster->GetHealth());
                    caster->ModifyHealth(missingHealth);
                    if (caster->IsAIEnabled && caster->GetAI()->GetData(TYPE_PHASE) != 1)
                        return;

                    caster->ModifyPower(POWER_MANA, -missingHealth);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sinestra_mana_barrier_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_mana_barrier_AuraScript();
        }
};

class spell_sinestra_pyrrhic_focus : public SpellScriptLoader
{
    public:
        spell_sinestra_pyrrhic_focus() : SpellScriptLoader("spell_sinestra_pyrrhic_focus") { }

        class spell_sinestra_pyrrhic_focus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_pyrrhic_focus_AuraScript);

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                {
                    uint32 addHealth = uint32(0.01f * caster->GetMaxHealth());
                    if (caster->GetHealth() <= addHealth)
                        caster->Kill(caster);
                    else
                        caster->ModifyHealth(-int32(addHealth));
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_sinestra_pyrrhic_focus_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_pyrrhic_focus_AuraScript();
        }
};

class spell_sinestra_twilight_slicer : public SpellScriptLoader
{
    public:
        spell_sinestra_twilight_slicer() : SpellScriptLoader("spell_sinestra_twilight_slicer") { }

        class spell_sinestra_twilight_slicer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sinestra_twilight_slicer_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (Creature* pSinestra = GetCaster()->FindNearestCreature(NPC_SINESTRA, 200.0f))
                {
                    if (boss_sinestra::boss_sinestraAI* SinestraAI = CAST_AI(boss_sinestra::boss_sinestraAI, pSinestra->GetAI()))
                    {
                        if (SinestraAI->pOrb1 && SinestraAI->pOrb2)
                        {
                            targets.remove_if(TwilightSlicerTargetSelector(SinestraAI->pOrb1, SinestraAI->pOrb2));
                        }
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sinestra_twilight_slicer_SpellScript::FilterTargets, EFFECT_0,TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sinestra_twilight_slicer_SpellScript();
        }
};

class spell_sinestra_twilight_essence : public SpellScriptLoader
{
    public:
        spell_sinestra_twilight_essence() : SpellScriptLoader("spell_sinestra_twilight_essence") { }

        class spell_sinestra_twilight_essence_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sinestra_twilight_essence_SpellScript);

            void CorrectRange(std::list<WorldObject*>& targets)
            {
                targets.remove_if(ExactDistanceCheck(GetCaster(), 5.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE)));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sinestra_twilight_essence_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sinestra_twilight_essence_SpellScript::CorrectRange, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        class spell_sinestra_twilight_essence_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_sinestra_twilight_essence_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster() || !GetTarget())
                    return;

                if (GetTarget()->GetEntry() == NPC_TWILIGHT_DRAKE)
                    GetTarget()->CastSpell(GetTarget(), SPELL_ABSORB_ESSENCE, true);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_sinestra_twilight_essence_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_sinestra_twilight_essence_SpellScript();
        }

        AuraScript* GetAuraScript() const
        {
            return new spell_sinestra_twilight_essence_AuraScript();
        }
};

void AddSC_boss_sinestra()
{
    new boss_sinestra();
    new npc_sinestra_calen();
    new npc_sinestra_twilight_spitecaller();
    new npc_sinestra_twilight_whelp();
    new npc_sinestra_twilight_drake();
    new npc_sinestra_twilight_essence();
    new npc_sinestra_twilight_flames();
    new npc_sinestra_barrier_stalker();
    new npc_sinestra_shadow_orb();
    new npc_sinestra_pulsing_twilight_egg();
    new spell_sinestra_twilight_extinction();
    new spell_sinestra_wrack();
    new spell_sinestra_indomitable();
    new spell_sinestra_indomitable_aura();
    new spell_sinestra_mana_barrier();
    new spell_sinestra_pyrrhic_focus();
    new spell_sinestra_twilight_slicer();
    new spell_sinestra_twilight_essence();
}