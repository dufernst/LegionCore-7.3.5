

#include "blackwing_descent.h"

enum NefYells
{
    // Nefarian
    SAY_AVOID_WIPE                  = 0,
    SAY_AGGRO                       = 1,
    SAY_AIR_PHASE                   = 2,
    SAY_AIR_PHASE_2                 = 3,
    SAY_FINAL_PHASE                 = 4,
    SAY_SHADOWFLAME                 = 5,
    SAY_KILL                        = 6,
    SAY_DEATH                       = 7
};

enum IntroYells
{
    SAY_INTRO_1                     = 0,
    SAY_INTRO_2                     = 1,
    SAY_INTRO_3                     = 2,
};

enum Spells
{
    SPELL_CHILDREN_OF_DEATHWING_NEF     = 78620,
    SPELL_CHILDREN_OF_DEATHWING_ONY     = 78619,

    // Nefarian
    SPELL_ELECTROCUTE                   = 81272,
    SPELL_SHADOWFLAME_BREATH            = 77826,
    SPELL_ANIMATE_BONES                 = 78122,
    SPELL_HAIL_OF_BONES                 = 78679,
    SPELL_TAIL_LASH                     = 77827,
    SPELL_SHADOWBLAZE_SPARK             = 81031,              
    SPELL_SHADOWFLAME_BARRAGE           = 78621,
    SPELL_H_EXPLOSIVE_CINDERS_PERIODIC  = 79339,
    SPELL_H_EXPLOSIVE_CINDERS_SUMM_DMG  = 79347,
    SPELL_SHADOW_COWARDICE              = 79353,
    SPELL_BERSERK_NEF                   = 26662,

    // Heroic Dominion Mechanic.
    SPELL_DOMINION                      = 79318,
    SPELL_DOMINION_IMMUN                = 95900,
    SPELL_DOMINION_DUMMY                = 94211,
    SPELL_SIPHON_POWER                  = 79319,
    SPELL_STOLEN_POWER                  = 80627,
    SPELL_FREE_MIND                     = 79323,

    // Onyxia
    SPELL_ONYXIA_DISCHARGE_BAR          = 78949,
    SPELL_INCREASE_BAR                  = 98734,
    SPELL_LIGHTNING_DISCHARGE           = 77944,

    // Anim Bone Warriors
    SPELL_NO_REGEN                      = 78725,
    SPELL_NO_REGEN2                     = 72242,
    SPELL_EMPOWER                       = 79329, 
    SPELL_HURL_BONE                     = 81586,
    SPELL_DIE_VISUAL                    = 57626,

    // Chromatic Prototype
    SPELL_NOVA                          = 80734,

    // Shadowblaze Flashpoint
    SPELL_FIREBRUSH_AURA                = 79396,
};

enum Phases
{
    PHASE_NULL                          = 0,
    PHASE_INTRO,
    PHASE_GROUND,
    PHASE_FLIGHT,
    PHASE_FINAL
};

enum Events
{
    // Nefarian
    // Intro
    EVENT_INTRO_1                       = 1,
    EVENT_INTRO_2,
    EVENT_INTRO_3,
    EVENT_SUMMON_ONYXIA,
    EVENT_SUMMON_NEFARIAN,

    EVENT_INTRO,
    EVENT_INTRO2,
    EVENT_HAIL_OF_BONES,
    EVENT_MOVE,
    EVENT_LANDING,
    EVENT_BERSERK,

    // Ground phase
    EVENT_SHADOWFLAME_BREATH,
    EVENT_SHADOW_COWARDICE,
    EVENT_TAIL_LASH,

    EVENT_LIFTOFF,
    EVENT_FLIGHT,
    EVENT_AIR,

    // Air phase
    EVENT_SHADOWFLAME_BARRAGE,
    EVENT_SUMMON_CHROMATIC,
    EVENT_LAND,
    EVENT_RETURN,
    EVENT_GROUND,
    EVENT_EXPLOSION,

    // Final phase
    EVENT_SHADOWBLAZE,
    EVENT_REVIVE_SKELETONS,

    // Onyxia
    EVENT_SF_BREATH,
    EVENT_LIGHTNING_DISCHARGE,
    EVENT_TAL_LASH,
};

enum Npc
{
    NPC_NEFARIAN_INTRO                  = 41379,
    NPC_CHROMATIC_PROTO                 = 41948,
    NPC_ANIM_BONE_WARR                  = 41918, 
    MOB_SHADOWBLAZE                     = 42596,
};

Position const NefarianPositions[6] =
{
    {-167.093f,     -224.479f,      40.399f,        6.278f      },  // lord nefarian intr ! not used yet
    {-135.795151f,  15.569847f,     73.165909f,     4.646072f   },  // Intro ! not used yet
    {-129.176636f,  -10.488489f,    73.079071f,     5.631739f   },  // Ground
    {-106.186249f,  -18.533386f,    72.798332f                  },  // Air intr
    {-126.518f,     -233.342f,      36.358f                     },  // Position on top of raid.
    {-100.123f,     -221.522f,      7.156f                      },  // Move down.
};

const Position MiddleRoomLocation = {-103.057961f, -222.698685f, 18.374910f, 0.0f};

Position ChromaticPositions[3] =
{
    {-86.7713f,     -190.62083f,    14.0571f,        0.00f       },
    {-87.0204f,     -258.6006f,     14.0575f,        0.00f       },
    {-148.177f,     -224.4730f,     14.05815f,       0.00f       },
};

const Position centerPos = {-104.7067f, -226.5108f, 41.4890f, 0.00f};  // center

class boss_bd_nefarian : public CreatureScript
{
public:
    boss_bd_nefarian() : CreatureScript("boss_bd_nefarian") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_bd_nefarianAI (creature);
    }

    struct boss_bd_nefarianAI : public BossAI
    {
        boss_bd_nefarianAI(Creature* creature) : BossAI(creature, DATA_NEFARIAN)
        {
            instance = creature->GetInstanceScript();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }

        InstanceScript* instance;
        Phases phase;
        uint32 m_uiOnyxiaCheckTimer;
        uint32 m_uiDistanceCheckTimer;
        uint32 m_uiChromaticCheckTimer;
        bool onyxiaAlive, said, secondPhase, finalPhase;

        uint8 healthPct;
        GuidList SummonList;
        uint8 SpawnCount;

        void Reset()
        {
            events.Reset();
            healthPct   = 90;
            phase       = PHASE_NULL;
            onyxiaAlive = true;
            said        = false;
            secondPhase = false;
            finalPhase  = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);

            RemoveSummons();
            SpawnCount = 3;

            _Reset();
        }

        void IsSummonedBy(Unit* summoner)
        {
            DoZoneInCombat();

            if (Creature* Onyxia = me->FindNearestCreature(NPC_ONYXIA, 150.0f, true))
                Onyxia->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch(summon->GetEntry())
            {
            case NPC_CHROMATIC_PROTO:
                SpawnCount--;
                break;
            }
        }

        void RemoveSummons()
        {
            if (SummonList.empty())
                return;

            for (GuidList::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
            {
                if (Creature* pTemp = Unit::GetCreature(*me, *itr))
                    if (pTemp)
                        pTemp->DisappearAndDie();
            }
            SummonList.clear();

        }
        void EnterEvadeMode()
        {
            Reset();

            me->DespawnOrUnsummon(3000); // fix this shit
            me->SummonCreature(NPC_NEFARIAN_INTRO, NefarianPositions[0], TEMPSUMMON_MANUAL_DESPAWN); // fix this shit

            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            CreatureAI::EnterEvadeMode();
        }

        void EnterCombat(Unit* pWho)
        {
            EnterPhaseIntro();
            if (Creature* onyxia = me->FindNearestCreature(NPC_ONYXIA, 150.0f))
                onyxia->SetInCombatWithZone();

            me->SetInCombatWithZone();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            _EnterCombat();
        }

        void JustDied(Unit* /*killer*/)
        {
            me->RemoveAllAuras();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

            Talk(SAY_DEATH);

            _JustDied();
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_KILL - urand(0, 1));
        }

        void DamageTaken(Unit* /*who*/, uint32& damage, DamageEffectType dmgType)
        {
            int ptc = (me->GetHealth() - damage) / me->GetMaxHealth() * 100;
            if (ptc < healthPct && healthPct > 0)
            {
                healthPct = (ptc / 10) * 10;

                DoCast(me, SPELL_ELECTROCUTE);
            }
        }

        void EnterPhaseIntro()
        {
            phase = PHASE_INTRO;
            events.SetPhase(PHASE_INTRO);
            initIntroEvents();            
        }

        void EnterPhaseGround()
        {
            phase = PHASE_GROUND;
            events.SetPhase(PHASE_GROUND);
            initEvents(true);
            m_uiOnyxiaCheckTimer    = 5000;
            m_uiDistanceCheckTimer  = 5000;
        }

        void EnterPhaseAir()
        {
            phase = PHASE_FLIGHT;
            events.SetPhase(PHASE_FLIGHT);
            initEvents(false);
            m_uiChromaticCheckTimer = 5000;
        }

        void EnterPhaseFinal()
        {
            phase = PHASE_FINAL;
            events.SetPhase(PHASE_FINAL);
            initFinalEvents();
        }

        void initEvents(bool onGround = true)
        {
            events.Reset();

            if (onGround)
            {
                events.RescheduleEvent(EVENT_SHADOWFLAME_BREATH,  5000,   PHASE_GROUND);
                events.RescheduleEvent(EVENT_SHADOW_COWARDICE,    20000,  PHASE_GROUND);
            }
            else
            {
                events.RescheduleEvent(EVENT_SHADOWFLAME_BARRAGE, 4000,   PHASE_FLIGHT);
                events.RescheduleEvent(EVENT_SUMMON_CHROMATIC,    10000,  PHASE_FLIGHT);
            }
        }

        void initIntroEvents()
        {
            events.Reset();

            events.RescheduleEvent(EVENT_INTRO,   100);
            events.RescheduleEvent(EVENT_INTRO2,  9900);
            events.RescheduleEvent(EVENT_MOVE,    25000);
            events.RescheduleEvent(EVENT_BERSERK, 60000 * 10);
        }

        void initFinalEvents()
        {
            events.Reset();

            events.RescheduleEvent(EVENT_TAIL_LASH,           4000, PHASE_FINAL);
            events.RescheduleEvent(EVENT_SHADOWFLAME_BREATH,  5000, PHASE_FINAL);
            events.RescheduleEvent(EVENT_SHADOWBLAZE,         9000, PHASE_FINAL);
            events.RescheduleEvent(EVENT_REVIVE_SKELETONS,    1000, PHASE_FINAL);
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
            case NPC_CHROMATIC_PROTO:
                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    summon->AI()->AttackStart(pTarget);
                SummonList.push_back(summon->GetGUID());
                break;
            case NPC_NEFARIAN:
                if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 100.0f, true))
                {
                    summon->GetMotionMaster()->MoveChase(target);
                    summon->Attack(target, true);
                }
                break;
            default:
                summon->AI()->DoZoneInCombat();
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (Creature* Onyxia = me->FindNearestCreature(NPC_ONYXIA, 150.0f, true))
            {
                if (Onyxia && !Onyxia->isInCombat() && said == false)
                {
                    Talk(SAY_AVOID_WIPE);
                    said = true;
                }
            }

            if (phase == PHASE_GROUND && m_uiDistanceCheckTimer <= diff)
            {
                if (me->FindNearestCreature(NPC_ONYXIA, 50.0f, true) && onyxiaAlive && !me->HasAura(SPELL_CHILDREN_OF_DEATHWING_ONY))
                    me->AddAura(SPELL_CHILDREN_OF_DEATHWING_ONY, me);

                else if (!me->FindNearestCreature(NPC_ONYXIA, 50.0f, true) && onyxiaAlive)
                if (me->HasAura(SPELL_CHILDREN_OF_DEATHWING_ONY))
                    me->RemoveAura(SPELL_CHILDREN_OF_DEATHWING_ONY);

                m_uiDistanceCheckTimer = 5000;
            }
            else m_uiDistanceCheckTimer -= diff;

            if (phase == PHASE_GROUND && m_uiOnyxiaCheckTimer <= diff && !secondPhase)
            {
                if (me->FindNearestCreature(NPC_ONYXIA, 250.0f, true))
                    onyxiaAlive = true;
                else
                {
                    onyxiaAlive = false;
                    Talk(SAY_AIR_PHASE);

                    if (me->HasAura(SPELL_CHILDREN_OF_DEATHWING_ONY))
                        me->RemoveAura(SPELL_CHILDREN_OF_DEATHWING_ONY);

                    events.RescheduleEvent(EVENT_LIFTOFF, 1000, PHASE_GROUND);
                    secondPhase = true;
                }
                m_uiOnyxiaCheckTimer = 1000;
            }
            else m_uiOnyxiaCheckTimer -= diff;

            if (phase == PHASE_FLIGHT && m_uiChromaticCheckTimer <= diff && !finalPhase)
            {
                if (!me->FindNearestCreature(NPC_CHROMATIC_PROTO, 150.0f))
                {
                    events.RescheduleEvent(EVENT_LAND, 10000, PHASE_FLIGHT);
                    finalPhase = true;
                }
                m_uiChromaticCheckTimer = 1000;
            }
            else m_uiChromaticCheckTimer -= diff;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_INTRO:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                        me->SetDisableGravity(true);
                        me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                        me->SetCanFly(true);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(1, -126.518f, -233.342f, 36.358f); // Position on top of raid.
                        break;

                    case EVENT_INTRO2:
                        Talk(SAY_AGGRO);
                        events.RescheduleEvent(EVENT_HAIL_OF_BONES, 100);
                        break;

                    case EVENT_BERSERK:
                        me->AddAura(SPELL_BERSERK_NEF, me);
                        break;

                    case EVENT_HAIL_OF_BONES:
                        DoCast(me, SPELL_HAIL_OF_BONES);
                        break;

                    case EVENT_MOVE:
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(1, -100.123f, -221.522f, 7.156f); // Move down.
                        events.RescheduleEvent(EVENT_LANDING, 8000);
                        break;

                    case EVENT_LANDING:
                        EnterPhaseGround();

                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);

                        me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetDisableGravity(false);
                        me->SetCanFly(false);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveChase(me->getVictim());
                        break;

                    case EVENT_SHADOWFLAME_BREATH:
                        Talk(SAY_SHADOWFLAME);
                        DoCastVictim(SPELL_SHADOWFLAME_BREATH);
                        events.RescheduleEvent(EVENT_SHADOWFLAME_BREATH, urand(10000, 12000));
                        break;

                    case EVENT_SHADOW_COWARDICE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_SHADOW_COWARDICE);
                        events.RescheduleEvent(EVENT_SHADOW_COWARDICE, urand(9000, 10000));
                        break;

                    case EVENT_LIFTOFF:
                        Talk(SAY_AIR_PHASE_2);
                        if (GameObject* elevator = instance->instance->GetGameObject(instance->GetGuidData(DATA_NEFARIAN_FLOOR)))
                            elevator->SetGoState(GO_STATE_ACTIVE);

                        me->GetMotionMaster()->Clear();
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                        me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                        me->SetDisableGravity(true);
                        me->SetCanFly(true);

                        events.RescheduleEvent(EVENT_FLIGHT, 1000);
                        events.RescheduleEvent(EVENT_AIR, 1000);
                        break;

                    case EVENT_FLIGHT:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 50.0f);
                        break;

                    case EVENT_AIR:
                        EnterPhaseAir();
                        break;

                    case EVENT_SUMMON_CHROMATIC:
                        me->SummonCreature(NPC_CHROMATIC_PROTO, ChromaticPositions[0], TEMPSUMMON_CORPSE_DESPAWN);
                        me->SummonCreature(NPC_CHROMATIC_PROTO, ChromaticPositions[1], TEMPSUMMON_CORPSE_DESPAWN);
                        me->SummonCreature(NPC_CHROMATIC_PROTO, ChromaticPositions[2], TEMPSUMMON_CORPSE_DESPAWN);
                        break;

                    case EVENT_SHADOWFLAME_BARRAGE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                            DoCast(target, SPELL_SHADOWFLAME_BARRAGE);
                        events.RescheduleEvent(EVENT_SHADOWFLAME_BARRAGE, urand(8000, 11000));
                        break;

                    case EVENT_LAND:
                        me->GetMotionMaster()->Clear();
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                        me->SetDisableGravity(false);
                        me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                        me->SetCanFly(false);
                        events.RescheduleEvent(EVENT_RETURN, 1000);
                        events.RescheduleEvent(EVENT_GROUND, 1500);
                        break;

                    case EVENT_RETURN:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() - 50.0f);
                        break;

                    case EVENT_GROUND:
                        Talk(SAY_FINAL_PHASE);
                        EnterPhaseFinal();
                        me->SetReactState(REACT_AGGRESSIVE);
                        AttackStart(me->getVictim());
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveChase(me->getVictim());

                        if (GameObject* elevator = instance->instance->GetGameObject(instance->GetGuidData(DATA_NEFARIAN_FLOOR)))
                            elevator->SetGoState(GO_STATE_READY);
                        break;

                    case EVENT_TAIL_LASH:
                        DoCast(me, SPELL_TAIL_LASH);
                        events.RescheduleEvent(EVENT_TAIL_LASH, urand(8000, 11000));
                        break;

                    case EVENT_SHADOWBLAZE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                            DoCast(target, SPELL_SHADOWBLAZE_SPARK);
                        events.RescheduleEvent(EVENT_TAIL_LASH, urand(18000, 21000));
                        break;

                    case EVENT_REVIVE_SKELETONS:
                        std::list<Creature*> creatures;
                        GetCreatureListWithEntryInGrid(creatures, me, NPC_ANIM_BONE_WARR, 200.0f);
                        if (!creatures.empty())
                        for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                        {
                            DoCast((*iter), SPELL_ANIMATE_BONES, true);
                            (*iter)->SetReactState(REACT_AGGRESSIVE);
                            (*iter)->RemoveAura(SPELL_DIE_VISUAL);
                        }
                        break;
                }
            }
            if (phase == PHASE_GROUND || phase == PHASE_FINAL)
                DoMeleeAttackIfReady();
        }
    };
};

class boss_bd_onyxia : public CreatureScript
{
public:
    boss_bd_onyxia() : CreatureScript("boss_bd_onyxia") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_bd_onyxiaAI (creature);
    }

    struct boss_bd_onyxiaAI : public ScriptedAI
    {
        boss_bd_onyxiaAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        uint32 m_uiDistancesCheckTimer;
        uint32 m_uiPowerTimer;

        void Reset()
        {
            events.Reset();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!me->IsWithinDistInMap(who, 30.0f, false))
                return;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void EnterCombat(Unit* pWho)
        {
            if (Creature* nefarian = me->FindNearestCreature(NPC_NEFARIAN, 150.0f))
                nefarian->SetInCombatWithZone();

            me->SetInCombatWithZone();
            DoCast(me, SPELL_ONYXIA_DISCHARGE_BAR);
            m_uiPowerTimer          = 2000;
            m_uiDistancesCheckTimer = 10000;
            events.RescheduleEvent(EVENT_SF_BREATH,           urand(7000, 9000));
            events.RescheduleEvent(EVENT_LIGHTNING_DISCHARGE, urand(12000, 15000));
            events.RescheduleEvent(EVENT_TAL_LASH,            urand(4000, 6000));
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon(3000);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (m_uiPowerTimer <= diff)
            {
                DoCast(me, SPELL_INCREASE_BAR);
                m_uiPowerTimer = 2000;
            }
            else m_uiPowerTimer -= diff;

            if (m_uiDistancesCheckTimer <= diff)
            {
                if (me->FindNearestCreature(NPC_NEFARIAN, 50.0f, true) && !me->HasAura(SPELL_CHILDREN_OF_DEATHWING_ONY))
                    me->AddAura(SPELL_CHILDREN_OF_DEATHWING_NEF, me);

                else if (!me->FindNearestCreature(NPC_NEFARIAN, 50.0f, true))
                    if (me->HasAura(SPELL_CHILDREN_OF_DEATHWING_NEF))
                        me->RemoveAura(SPELL_CHILDREN_OF_DEATHWING_NEF);

                m_uiDistancesCheckTimer = 5000;
            }
            else m_uiDistancesCheckTimer -= diff;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SF_BREATH:
                    DoCastVictim(SPELL_SHADOWFLAME_BREATH);
                    events.RescheduleEvent(EVENT_SF_BREATH, urand(13000, 17000));
                    break;                       

                case EVENT_LIGHTNING_DISCHARGE:
                    DoCast(me, SPELL_LIGHTNING_DISCHARGE);
                    events.RescheduleEvent(EVENT_LIGHTNING_DISCHARGE, urand(25000, 30000));
                    break;

                case EVENT_TAL_LASH:
                    DoCast(me, SPELL_TAIL_LASH);
                    events.RescheduleEvent(EVENT_TAL_LASH, urand(8000, 11000));
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_nefarian_intro : public CreatureScript
{
public:
    npc_nefarian_intro() : CreatureScript("npc_nefarian_intro") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nefarian_introAI (creature);
    }

    struct npc_nefarian_introAI : public ScriptedAI
    {
        npc_nefarian_introAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        InstanceScript* instance;
        EventMap events;
        bool introDone;

        void Reset()
        {
            if (GameObject* elevator = instance->instance->GetGameObject(instance->GetGuidData(DATA_NEFARIAN_FLOOR)))
                if (elevator->GetGoState() == GO_STATE_READY)
                    elevator->SetGoState(GO_STATE_ACTIVE);

            events.Reset();
            introDone = false;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (introDone)
                return;

            if (!me->IsWithinDistInMap(who, 30.0f, false))
                return;

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            events.RescheduleEvent(EVENT_INTRO_1, 2000);
            me->SetReactState(REACT_PASSIVE);
        }

        void JustSummoned(Creature* summon)
        {
            if (summon->GetEntry() == NPC_NEFARIAN)
            {
                summon->SetInCombatWithZone();
                summon->AI()->DoZoneInCombat();
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_INTRO_1:
                    Talk(SAY_INTRO_1);
                    if (GameObject* elevator = me->FindNearestGameObject(GO_NEFARIAN_FLOOR, 200.0f))
                        elevator->SetGoState(GO_STATE_READY);

                    events.RescheduleEvent(EVENT_SUMMON_ONYXIA,   14000);
                    events.RescheduleEvent(EVENT_INTRO_2,         15000);
                    break;

                case EVENT_SUMMON_ONYXIA:
                    if (!me->FindNearestCreature(NPC_ONYXIA, 150.0f))
                        me->SummonCreature(NPC_ONYXIA, -104.713f, -225.264f, 7.156f, 3.122f, TEMPSUMMON_MANUAL_DESPAWN);
                    break;

                case EVENT_INTRO_2:
                    Talk(SAY_INTRO_2);
                    events.RescheduleEvent(EVENT_INTRO_3, 10000);
                    break;

                case EVENT_INTRO_3:
                    Talk(SAY_INTRO_3);
                    introDone = true;
                    events.RescheduleEvent(EVENT_SUMMON_NEFARIAN, 7500);
                    me->DespawnOrUnsummon(8000);
                    break;

                case EVENT_SUMMON_NEFARIAN:
                    me->SummonCreature(NPC_NEFARIAN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                    Creature* Nefarian_cr = me->FindNearestCreature(NPC_NEFARIAN, 150.0f, true);
                    Creature* Onyxia_cr = me->FindNearestCreature(NPC_ONYXIA, 150.0f, true);
                    if (Onyxia_cr && Nefarian_cr && !Onyxia_cr->isInCombat())
                    {
                        Nefarian_cr->GetMotionMaster()->Clear();
                        Nefarian_cr->GetMotionMaster()->MovePoint(177, NefarianPositions[4]);
                    }
                    break;
                }
            }
        }

        void DamageTaken(Unit* /*who*/, uint32& /*damage*/, DamageEffectType dmgType)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
        }
    };
};

class npc_animated_bone_warrior : public CreatureScript
{
public:
    npc_animated_bone_warrior() : CreatureScript("npc_animated_bone_warrior") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_animated_bone_warriorAI (creature);
    }

    struct npc_animated_bone_warriorAI : public ScriptedAI
    {
        npc_animated_bone_warriorAI(Creature* creature) : ScriptedAI(creature) 
        {
            ASSERT(creature->GetVehicleKit());
            creature->SetPower(POWER_ENERGY,    100);
            creature->SetMaxPower(POWER_ENERGY, 100);
        }

        uint32 timerHurlBone;

        void EnterCombat(Unit* /*who*/)
        {
            timerHurlBone = urand(4000, 9000);
            me->AddAura(SPELL_NO_REGEN, me);
            me->AddAura(SPELL_NO_REGEN2, me);
            DoCast(me, SPELL_ANIMATE_BONES);
            DoCast(me, SPELL_EMPOWER);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->GetPower(POWER_ENERGY) == 0)
            {
                me->SetReactState(REACT_PASSIVE);
                me->RemoveAura(SPELL_ANIMATE_BONES);
                DoCast(me, SPELL_DIE_VISUAL);
                me->SetPower(POWER_ENERGY, 0);        
            }

            if (timerHurlBone <= diff)
            {
                DoCastVictim(SPELL_HURL_BONE);
                timerHurlBone = urand(8000, 14000);
            }
            else timerHurlBone -= diff;

            if (me->HasAura(SPELL_ANIMATE_BONES))
                DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon(3000);
        }
    };
};

class npc_chromatic_prototype : public CreatureScript
{
public:
    npc_chromatic_prototype() : CreatureScript("npc_chromatic_prototype") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chromatic_prototypeAI (creature);
    }

    struct npc_chromatic_prototypeAI : public ScriptedAI
    {
        npc_chromatic_prototypeAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        EventMap events;

        void EnterCombat(Unit* /*who*/)
        {
            me->AddAura(SPELL_NOVA, me);
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_EXPLOSION, urand(10000, 15000));
        }

        void Reset()
        {
            events.Reset();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_EXPLOSION:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        me->CastSpell(pTarget, SPELL_H_EXPLOSIVE_CINDERS_PERIODIC);
                    break;
                }
            }
            
        }

        void JustDied(Unit* /*killer*/) 
        {
            me->DespawnOrUnsummon(5000);
        }
    };
};

class npc_shadowflame_flashfire : public CreatureScript
{
public:
    npc_shadowflame_flashfire() : CreatureScript("npc_shadowflame_flashfire") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadowflame_flashfireAI (creature);
    }

    struct npc_shadowflame_flashfireAI : public ScriptedAI
    {
        npc_shadowflame_flashfireAI(Creature* creature) : ScriptedAI(creature)
        {
            timerMove       = 500;
            timerDespawn    = 20000;
            timerSpawn      = 1500;
            creature->SetReactState(REACT_PASSIVE);
        }

        uint32 timerMove;
        uint32 timerDespawn;
        uint32 timerSpawn;

        void UpdateAI(uint32 diff)
        {
            if (timerMove <= diff)
            {
                me->SetSpeed(MOVE_WALK, 0.8f, true);
                me->SetSpeed(MOVE_RUN, 0.8f, true);
                float x, y, z;
                me->GetClosePoint(x, y, z, me->GetObjectSize() / 3, 50.0f);
                me->GetMotionMaster()->MovePoint(1, x, y, z);
            }
            else timerMove -= diff;

            if (timerDespawn <= diff)
            {
                me->DespawnOrUnsummon();
            }
            else timerDespawn -= diff;

            if (timerSpawn <= diff)
            {
                me->SummonCreature(MOB_SHADOWBLAZE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 3000);
            }
            else timerDespawn -= diff;
        }
    };
};

class npc_shadowblaze : public CreatureScript
{
public:
    npc_shadowblaze() : CreatureScript("npc_shadowblaze") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadowblazeAI (creature);
    }

    struct npc_shadowblazeAI : public ScriptedAI
    {
        npc_shadowblazeAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 timerCheckskeleton;
        uint32 timerDespawn;

        void IsSummonedBy(Unit* summoner)
        {
            timerCheckskeleton  = 100;
            timerDespawn        = 6100;
            me->AddAura(SPELL_FIREBRUSH_AURA, me);
        }

        void UpdateAI(uint32 diff)
        {
            if (timerCheckskeleton <= diff)
            {
                if (Unit* skeleton = me->FindNearestCreature(NPC_ANIM_BONE_WARR, 4.0f, true))
                    skeleton->CastSpell(skeleton, SPELL_ANIMATE_BONES, true);
                timerCheckskeleton = 980;
            }
            else timerCheckskeleton -= diff;

            if (timerDespawn <= diff)
            {
                me->DespawnOrUnsummon();
            }
            else timerDespawn -= diff;
        }

        void JustDied(Unit* /*killer*/) { }
    };
};

class spell_onyxia_lightning_discharge : public SpellScriptLoader
{
public:
    spell_onyxia_lightning_discharge() : SpellScriptLoader("spell_onyxia_lightning_discharge") { }

    class spell_onyxia_lightning_discharge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_onyxia_lightning_discharge_SpellScript);

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;

            if (GetHitUnit()->isInFront(GetCaster(), GetCaster()->GetObjectSize() / 3) ||
                GetHitUnit()->isInBack(GetCaster(), GetCaster()->GetObjectSize() / 3))

                SetHitDamage(0);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_onyxia_lightning_discharge_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_onyxia_lightning_discharge_SpellScript();
    }
};

void AddSC_boss_bwd_nefarian()
{
    new boss_bd_nefarian();
    new boss_bd_onyxia();

    new npc_nefarian_intro();
    new npc_animated_bone_warrior();
    new npc_chromatic_prototype();
    //new npc_shadowflame_flashfire();
    new npc_shadowblaze();

    new spell_onyxia_lightning_discharge();
}

/* Not needed, kept as building new transport future reference.

Transport* t = new Transport(21867, 0);
std::set<uint32> unused;
uint32 theguid = sObjectMgr->GenerateLowGuid(HIGHGUID_MO_TRANSPORT);
t->Create(theguid, GO_NEFARIAN_FLOOR, 669, -107.213f, -224.62f, -6.867f, 3.14f, 255, 0);
Map* tMap = me->GetMap();
t->SetMap(tMap);
t->AddToWorld();
t->BuildStopMovePacket(tMap);

// transmit creation packet
t->UpdateForMap(tMap);

sMapMgr->m_Transports.insert(t);
sMapMgr->m_TransportsByMap[669].insert(t);*/