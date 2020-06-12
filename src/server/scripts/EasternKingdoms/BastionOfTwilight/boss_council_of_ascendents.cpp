#include "bastion_of_twilight.h"


enum FeludiusScriptText
{
    SAY_FELUDIUS_AGGRO          = 0,
    SAY_FELUDIUS_KILL           = 1,
    SAY_FELUDIUS_SPELL          = 2,
    SAY_FELUDIUS_SPECIAL        = 3,
};

enum IgcaciousScriptText
{
    SAY_IGNACIOUS_AGGRO         = 0,
    SAY_IGNACIOUS_KILL          = 1,
    SAY_IGNACIOUS_SPELL         = 2,
    SAY_IGNACIOUS_SPECIAL       = 3,
};

enum ArionScriptText
{
    SAY_ARION_AGGRO             = 0,
    SAY_ARION_KILL              = 1,
    SAY_ARION_SPELL             = 2,
    SAY_ARION_SPECIAL           = 3,
};

enum TerrastraScriptText
{
    SAY_TERRASTRA_AGGRO         = 1,
    SAY_TERRASTRA_KILL          = 0,
    SAY_TERRASTRA_SPELL         = 2,
    SAY_TERRASTRA_SPECIAL       = 3,
};

enum MonstrosityScriptText
{
    SAY_MONSTROSITY_AGGRO       = 0,
    SAY_MONSTROSITY_KILL        = 1,
    SAY_MONSTROSITY_DEATH       = 3,
    SAY_MONSTROSITY_SPELL       = 4,
};

enum Spells
{
    //shared
    SPELL_ELEMENTAL_STATIS          = 82285,

    //feludius
    SPELL_HEART_OF_ICE              = 82665,
    SPELL_ICY_REPRISAL              = 82817,
    SPELL_HYDROLANCE                = 82752,
    SPELL_GLACIATE                  = 82746, 
    SPELL_FROZEN                    = 82772,
    SPELL_WATER_BOMB_AURA           = 77349,
    SPELL_WATER_BOMB_SUMMON         = 77347,
    SPELL_WATER_BOMB                = 82699,
    SPELL_WATER_BOMB_1              = 82700,
    SPELL_WATERLOGGED               = 82762,
    SPELL_FROST_IMBUED              = 82666,
    SPELL_FROZEN_ORB_AURA           = 92302,
    SPELL_FROZEN_ORB_SUMMON         = 92269,
    SPELL_FROST_BEACON              = 92307,
    SPELL_GLACIATE_2                = 92548,

    //ignacious
    SPELL_FLAME_TORRENT             = 82777,
    SPELL_INFERNO_JUMP              = 82856,
    SPELL_INFERNO_JUMP_DMG          = 82857,
    SPELL_INFERNO_RUSH              = 82859,
    SPELL_INFERNO_RUSH_AURA         = 88579,
    SPELL_BURNING_BLOOD             = 82660,
    SPELL_BURNING_SPITE             = 82813,
    SPELL_FLAME_IMBUED              = 82663,
    SPELL_AEGIS_OF_FLAMES           = 82631,
    SPELL_RISING_FLAMES             = 82636,
    SPELL_RISING_FLAMES_BUFF        = 82639,
    SPELL_FLAME_STRIKE              = 92211,
    SPELL_FLAME_STRIKE_DUMMY        = 92212,
    SPELL_FLAME_STRIKE_AURA         = 92215,
    SPELL_FLAME_STRIKE_AURA_DMG     = 92216,

    //arion
    SPELL_CALL_WINDS                = 83491,
    SPELL_DISPERSE                  = 83087,
    SPELL_DISPERSE_1                = 83078,
    SPELL_LIGHTNING_BLAST           = 83070,
    SPELL_LIGHTNING_ROD             = 83099,
    SPELL_THUNDERSHOCK              = 83067,
    SPELL_CHAIN_LIGHTNING_DUMMY     = 83300,
    SPELL_CHAIN_LIGHTNING           = 83282,
    SPELL_SWIRLING_WINDS            = 83500,
    SPELL_LASHING_WINDS             = 83472,
    SPELL_LASHING_WINDS_DMG         = 83498,
    SPELL_STATIC_OVERLOAD           = 92067,

    //terrastra
    SPELL_HARDEN_SKIN               = 83718,
    SPELL_SHATTER                   = 83760,
    SPELL_GRAVITY_WELL              = 83572,
    SPELL_GRAVITY_WELL_DUMMY        = 95760,
    SPELL_MAGNETIC_PULL             = 83579,
    SPELL_QUAKE                     = 83565,
    SPELL_ERUPTION_DUMMY            = 83675,
    SPELL_ERUPTION                  = 83692,
    SPELL_ERUPTION_AURA             = 83662,
    SPELL_ERUPTION_SUMMON           = 83661,
    SPELL_GROUNDED                  = 83581,
    SPELL_GRAVITY_CORE              = 92075,
    SPELL_GRAVITY_CORE_DMG          = 92076,

    //monstrosity
    SPELL_LAVA_SEED                     = 84913,
    SPELL_LAVA_SEED_DUMMY               = 84911,
    SPELL_LAVA_PLUME                    = 84912,
    SPELL_ELECTRIC_INSTABILITY_DUMMY    = 84526,
    SPELL_ELECTRIC_INSTABILITY          = 84529,
    SPELL_ELECTRIC_INSTABILITY_DMG      = 84529,
    SPELL_GRAVITY_CRUSH                 = 84948,
    SPELL_GRAVITY_CRUSH_VEHICLE         = 84952,
    SPELL_LIQUID_ICE                    = 84914,
    SPELL_LIQUID_ICE_DMG                = 84915,
    SPELL_LIQUID_ICE_SIZE               = 84917,
    SPELL_CRYOGENIC_AURA                = 84918,
};

enum Events
{
    EVENT_HEART_OF_ICE              = 1,
    EVENT_HYDROLANCE                = 2,
    EVENT_GLACIATE                  = 3,
    EVENT_WATER_BOMB                = 4,
    EVENT_FLAME_TORRENT             = 5,
    EVENT_INFERNO_JUMP              = 6,
    EVENT_BURNING_BLOOD             = 7,
    EVENT_AEGIS_OF_FLAMES           = 8,
    EVENT_RISING_FLAMES             = 9,
    EVENT_TEST                      = 111,
    EVENT_HARDEN_SKIN               = 10,
    EVENT_ERUPTION                  = 11,
    EVENT_GRAVITY_WELL              = 12,
    EVENT_QUAKE                     = 13,
    EVENT_DISPERSE                  = 14,
    EVENT_CALL_WINDS                = 15,
    EVENT_LIGHTNING_BLAST           = 16,
    EVENT_LIGHTNING_ROD             = 17,
    EVENT_CHAIN_LIGHTNING           = 18,
    EVENT_THUNDERSHOCK              = 19,
    EVENT_PHASE_3_1                 = 20,
    EVENT_PHASE_3_2                 = 21,
    EVENT_PHASE_3_3                 = 22,
    EVENT_PHASE_3_4                 = 23,
    EVENT_CRYOGENIC_AURA            = 24,
    EVENT_LAVA_SEED                 = 25,
    EVENT_GRAVITY_CRUSH             = 26,
    EVENT_ELECTRIC_INSTABILITY      = 27,
    EVENT_LAVA_PLUME                = 28,
    EVENT_FELUDIUS_INTRO            = 29,
    EVENT_IGNACIOUS_INTRO           = 30,
    EVENT_ARION_INTRO               = 31,
    EVENT_TERRASTRA_INTRO           = 32,
    EVENT_ERUPTION_DMG              = 33,
    EVENT_GRAVITY_WELL_DMG          = 34,
    EVENT_INFERNO_RUSH              = 35,
    EVENT_FLAME_STRIKE              = 36,
    EVENT_FROST_ORB                 = 37,
    EVENT_GRAVITY_CORE              = 38,
    EVENT_STATIC_OVERLOAD           = 39,
};

enum Adds
{
    NPC_WATER_BOMB                          = 44201,
    NPC_WATER_BOMB2                         = 41264,
    NPC_INFERNO_RUSH                        = 47501,
    NPC_INFERNO_LEAP                        = 47040,
    NPC_ERUPTION_TARGET                     = 44845,
    NPC_GRAVITY_WELL                        = 44824,
    NPC_VIOLENT_CYCLONE                     = 44747,
    NPC_LIQUID_ICE                          = 45452,
    NPC_ASCENDANT_COUNCIL_PLUME_STALKER     = 45420,
    NPC_ASCENDANT_COUNCIL_TARGET_STALKER    = 44553,
    NPC_ASCENDANT_COUNCIL_CONTROLLER        = 43691,
    NPC_GRAVITY_CRUSH                       = 45476,
    NPC_FLAME_STRIKE                        = 49432,
    NPC_FROZEN_ORB                          = 49518,
    NPC_FROZEN_ORB_SPAWNER                  = 49517,
};

enum Actions
{
    ACTION_PHASE_2          = 1,
    ACTION_THUNDERSHOCK     = 2,
    ACTION_QUAKE            = 3,
    ACTION_PHASE_3          = 4,
    ACTION_FLAME_STRIKE     = 5,
};

const Position councilPos[4] = 
{
    {-1053.54f, -564.38f, 835.02f, 5.81f},
    {-1054.04f, -600.60f, 835.03f, 0.42f},
    {-1057.89f, -653.51f, 877.70f, 0.79f},
    {-1057.93f, -533.38f, 877.68f, 5.47f},
};

const Position groundPos[7] = 
{
    {-1038.45f, -590.60f, 831.98f, 6.04f},
    {-1034.61f, -571.70f, 831.90f, 6.04f},
    {-1008.42f, -571.32f, 831.90f, 4.71f},
    {-995.92f, -582.68f, 831.90f, 3.18f},
    {-1008.75f, -594.82f, 831.90f, 1.52f},
    {-1020.88f, -582.67f, 831.90f, 0.0f},
    {-1008.58f, -582.97f, 831.90f, 6.23f},
};

const Position randomPos[34] = 
{
    {-976.85f, -596.94f, 831.90f, 4.42f},
    {-985.56f, -607.92f, 831.90f, 3.61f},
    {-995.49f, -609.65f, 831.90f, 3.16f},
    {-1010.19f, -616.54f, 831.90f, 3.66f},
    {-1019.31f, -611.42f, 831.90f, 2.38f},
    {-1033.72f, -610.70f, 834.02f, 3.02f},
    {-1042.03f, -615.29f, 835.17f, 2.72f},
    {-1047.88f, -605.81f, 835.20f, 1.91f},
    {-1050.91f, -595.78f, 835.23f, 1.77f},
    {-1052.30f, -578.92f, 835.02f, 1.53f},
    {-1050.25f, -566.79f, 835.23f, 1.27f},
    {-1043.81f, -554.45f, 835.20f, 0.87f},
    {-1035.13f, -557.34f, 833.72f, 4.13f},
    {-1041.10f, -571.14f, 833.11f, 4.33f},
    {-1045.98f, -583.89f, 833.87f, 4.54f},
    {-1036.06f, -589.51f, 831.90f, 0.65f},
    {-1030.99f, -581.64f, 831.90f, 0.84f},
    {-1021.70f, -571.16f, 831.90f, 0.84f},
    {-1008.63f, -563.33f, 831.90f, 0.13f},
    {-998.83f, -565.96f, 831.90f, 5.81f},
    {-989.52f, -578.90f, 831.90f, 5.08f},
    {-991.82f, -591.03f, 831.90f, 4.10f},
    {-1000.78f, -590.82f, 831.90f, 2.38f},
    {-1003.93f, -576.96f, 831.90f, 1.54f},
    {-999.24f, -560.80f, 831.90f, 1.24f},
    {-996.02f, -550.22f, 831.90f, 1.57f},
    {-1003.03f, -538.78f, 831.90f, 2.67f},
    {-999.54f, -530.48f, 831.89f, 0.09f},
    {-986.53f, -540.11f, 831.90f, 5.88f},
    {-975.28f, -547.22f, 831.90f, 5.59f},
    {-970.45f, -558.16f, 831.90f, 5.06f},
    {-964.18f, -570.52f, 831.90f, 5.95f},
    {-963.99f, -583.44f, 831.90f, 4.39f},
    {-976.46f, -585.91f, 831.90f, 2.92f}
};

class boss_feludius : public CreatureScript
{
    public:
        boss_feludius() : CreatureScript("boss_feludius") { }

        CreatureAI * GetAI(Creature* pCreature) const
        {
            return new boss_feludiusAI(pCreature);
        }

        struct boss_feludiusAI : BossAI
        {
            boss_feludiusAI(Creature * pCreature) : BossAI(pCreature, DATA_COUNCIL), summons(me)
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
            SummonList summons;
            bool bPhaseTwo;
            bool bPhaseThree;

            void Reset()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _Reset();

                bPhaseTwo = false;
                bPhaseThree = false;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                me->NearTeleportTo(me->GetHomePosition());
                summons.DespawnAll();
                events.Reset();
            }

            void JustReachedHome()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
                if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                    if (!_ignacious->isInCombat())
                        _ignacious->SetInCombatWithZone();
                if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                    if (!_arion->isInCombat())
                        _arion->SetInCombatWithZone();
                if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                    if (!_terrastra->isInCombat())
                        _terrastra->SetInCombatWithZone();

                DoCast(me, SPELL_WATER_BOMB_AURA, true);
                events.RescheduleEvent(EVENT_HEART_OF_ICE, urand(10000, 22000));
                events.RescheduleEvent(EVENT_HYDROLANCE, urand(12000, 15000));
                events.RescheduleEvent(EVENT_WATER_BOMB, urand(19000, 33000));
                //Talk(SAY_FELUDIUS_AGGRO);
                events.RescheduleEvent(EVENT_FELUDIUS_INTRO, 1000);
                instance->SetBossState(DATA_COUNCIL, IN_PROGRESS);
                DoZoneInCombat();
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

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_FELUDIUS_KILL);
            }

            void SpellHit(Unit* attacker, const SpellInfo* spell)
            {
                if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_HYDROLANCE)
                        for (uint8 i = 0; i < 3; ++i)
                            if (spell->Effects[i]->Effect == SPELL_EFFECT_INTERRUPT_CAST)
                                me->InterruptSpell(CURRENT_GENERIC_SPELL);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetHealth(me->CountPctFromMaxHealth(24));
                }

                if (me->HealthBelowPct(25) && !bPhaseTwo)
                {
                    DoAction(ACTION_PHASE_2);
                    if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                        _ignacious->AI()->DoAction(ACTION_PHASE_2);    
                }
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_PHASE_2:
                        if (!bPhaseTwo)
                        {
                            bPhaseTwo = true;
                            events.Reset();
                            me->RemoveAllAuras();
                            me->StopAttack();
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->NearTeleportTo(
                                councilPos[2].GetPositionX(),
                                councilPos[2].GetPositionY(),
                                councilPos[2].GetPositionZ(),
                                0.0f);
                            if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                            {
                                _arion->AI()->DoAction(ACTION_PHASE_2);
                                _arion->SetReactState(REACT_AGGRESSIVE);
                                _arion->NearTeleportTo(
                                groundPos[0].GetPositionX(),
                                groundPos[0].GetPositionY(),
                                groundPos[0].GetPositionZ(),
                                0.0f);
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                    _arion->AI()->AttackStart(target);
                            }
                        }
                        break;
                    case ACTION_PHASE_3:
                        if (!bPhaseThree)
                        {
                            bPhaseThree = true;
                            me->NearTeleportTo(
                                groundPos[2].GetPositionX(),
                                groundPos[2].GetPositionY(),
                                groundPos[2].GetPositionZ(),
                                0.0f);
                            events.RescheduleEvent(EVENT_PHASE_3_3, 9000);
                        }
                        break;
                }
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
                    case EVENT_FELUDIUS_INTRO:
                        Talk(SAY_FELUDIUS_AGGRO);
                        break;
                    case EVENT_PHASE_3_3:
                        Talk(SAY_FELUDIUS_SPECIAL);
                        me->GetMotionMaster()->MovePoint(1, groundPos[6]);
                        break;
                    case EVENT_WATER_BOMB:
                    {
                        std::set<uint8> rPos;
                        for (uint8 i = 0; i < 4;)
                        {
                            uint8 t = urand(0, 33);
                            if (rPos.find(t) == rPos.end())
                            {
                                rPos.insert(t);
                                ++i;
                            }
                        }
                        for (std::set<uint8>::const_iterator itr = rPos.begin(); itr != rPos.end(); ++itr)
                            me->SummonCreature(NPC_WATER_BOMB2, randomPos[(*itr)], TEMPSUMMON_TIMED_DESPAWN, 20000);
                        DoCast(me, SPELL_WATER_BOMB);
                        events.RescheduleEvent(EVENT_WATER_BOMB, urand(32000, 35000));
                        events.RescheduleEvent(EVENT_GLACIATE, 15000);
                        break;
                    }
                    case EVENT_HYDROLANCE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                            DoCast(target, SPELL_HYDROLANCE);
                        events.RescheduleEvent(EVENT_HYDROLANCE, urand(15000, 18000));
                        break;
                    case EVENT_GLACIATE:
                        Talk(SAY_FELUDIUS_SPELL);
                        DoCast(me, SPELL_GLACIATE);
                        break;
                    case EVENT_HEART_OF_ICE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                            DoCast(target, SPELL_HEART_OF_ICE);
                        events.RescheduleEvent(EVENT_HEART_OF_ICE, urand(21000, 24000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }            
        };
};

class boss_ignacious : public CreatureScript
{
    public:
        boss_ignacious() : CreatureScript("boss_ignacious") { }

        CreatureAI * GetAI(Creature * pCreature) const
        {
            return new boss_ignaciousAI(pCreature);
        }

        struct boss_ignaciousAI : public BossAI
        {
            boss_ignaciousAI(Creature * pCreature) : BossAI(pCreature, DATA_COUNCIL), summons(me)
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
            SummonList summons;
            bool bPhaseTwo;
            bool bPhaseThree;
            Position jumppos;

            void Reset()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _Reset();

                bPhaseTwo = false;
                bPhaseThree = false;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                me->NearTeleportTo(me->GetHomePosition());
                //summons.DespawnAll();
                events.Reset();
            }

            void JustReachedHome()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
                if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                    if (!_feludius->isInCombat())
                        _feludius->SetInCombatWithZone();
                if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                    if (!_arion->isInCombat())
                        _arion->SetInCombatWithZone();
                if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                    if (!_terrastra->isInCombat())
                        _terrastra->SetInCombatWithZone();

                events.RescheduleEvent(EVENT_BURNING_BLOOD, urand(10000, 22000));
                events.RescheduleEvent(EVENT_FLAME_TORRENT, urand(15000, 30000));
                events.RescheduleEvent(EVENT_AEGIS_OF_FLAMES, urand(30000, 40000));
                events.RescheduleEvent(EVENT_INFERNO_JUMP, urand(20000, 30000));
                //Talk(SAY_IGNACIOUS_AGGRO);
                events.RescheduleEvent(EVENT_IGNACIOUS_INTRO, 4000);
                //instance->SetBossState(DATA_COUNCIL, IN_PROGRESS);
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

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_IGNACIOUS_KILL);
            }

            void SpellHit(Unit* attacker, const SpellInfo* spell)
            {
                if (me->HasAura(SPELL_AEGIS_OF_FLAMES))
                    return;

                if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    if (me->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id == SPELL_RISING_FLAMES)
                        for (uint8 i = 0; i < 3; ++i)
                            if (spell->Effects[i]->Effect == SPELL_EFFECT_INTERRUPT_CAST)
                                me->InterruptSpell(CURRENT_CHANNELED_SPELL);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetHealth(me->CountPctFromMaxHealth(24));
                }

                if (me->HealthBelowPct(25) && !bPhaseTwo)
                {
                    DoAction(ACTION_PHASE_2);
                    if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                        _feludius->AI()->DoAction(ACTION_PHASE_2);
                }
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_PHASE_2:
                        if (!bPhaseTwo)
                        {
                            bPhaseTwo = true;
                            events.Reset();
                            me->StopAttack();
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->RemoveAllAuras();
                            me->NearTeleportTo(
                                councilPos[3].GetPositionX(),
                                councilPos[3].GetPositionY(),
                                councilPos[3].GetPositionZ(),
                                0.0f);
                            if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                            {
                                _terrastra->AI()->DoAction(ACTION_PHASE_2);
                                _terrastra->SetReactState(REACT_AGGRESSIVE);
                                _terrastra->NearTeleportTo(
                                groundPos[1].GetPositionX(),
                                groundPos[1].GetPositionY(),
                                groundPos[1].GetPositionZ(),
                                0.0f);
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                    _terrastra->AI()->AttackStart(target);
                            }
                        }
                        break;
                    case ACTION_PHASE_3:
                        if (!bPhaseThree)
                        {
                            bPhaseThree = true;
                            me->NearTeleportTo(
                                groundPos[3].GetPositionX(),
                                groundPos[3].GetPositionY(),
                                groundPos[3].GetPositionZ(),
                                0.0f);
                            events.RescheduleEvent(EVENT_PHASE_3_4, 14000);
                        }
                        break;
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (id)
                    {
                        case 1:
                        {
                            uint32 _health;
                            _health = 0;
                            if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                            {
                                _feludius->SetVisible(false);
                                _health += _feludius->GetHealth();
                            }
                            if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                            {
                                _arion->SetVisible(false);
                                _health += _arion->GetHealth();
                            }
                            if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                            {
                                _terrastra->SetVisible(false);
                                _health += _terrastra->GetHealth();
                            }
                            if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                            {
                                _ignacious->SetVisible(false);
                                _health += _ignacious->GetHealth();
                            }
                            if (Creature* _monstrocity = me->SummonCreature(NPC_MONSTROSITY, groundPos[6]))
                            {
                                _monstrocity->SetHealth(_health);
                                _monstrocity->LowerPlayerDamageReq(me->GetMaxHealth());
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))    
                                    _monstrocity->AI()->AttackStart(target);
                            }
                            break;
                        }
                        case EVENT_JUMP:
                        {
                            DoCastAOE(SPELL_INFERNO_JUMP_DMG, true);
                            events.RescheduleEvent(EVENT_INFERNO_RUSH, 1000);
                            break;
                        }
                    }
                }
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
                    case EVENT_IGNACIOUS_INTRO:
                        Talk(SAY_IGNACIOUS_AGGRO);
                        break;
                    case EVENT_PHASE_3_4:
                        Talk(SAY_IGNACIOUS_SPECIAL);
                        me->GetMotionMaster()->MovePoint(1, groundPos[6]);
                        break;
                    case EVENT_INFERNO_JUMP:
                        Unit* pTarget;
                        pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, -20.0f, true);
                        if (!pTarget)
                            pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true);
                        if (pTarget)
                        {
                            me->SetReactState(REACT_PASSIVE);
                            pTarget->GetPosition(&jumppos);
                            DoCast(pTarget, SPELL_INFERNO_JUMP);
                        }
                        events.RescheduleEvent(EVENT_INFERNO_JUMP, urand(20000, 30000));
                        break;
                    case EVENT_INFERNO_RUSH:
                    {
                        me->SetReactState(REACT_AGGRESSIVE);

                        if (!me->getVictim())
                            break;

                        if (Creature* pInfernoLeap = me->SummonCreature(NPC_INFERNO_LEAP, jumppos))
                            pInfernoLeap->GetMotionMaster()->MovePoint(1001, me->getVictim()->GetPositionX(), me->getVictim()->GetPositionY(), me->getVictim()->GetPositionZ());
                        
                        DoCast(me->getVictim(), SPELL_INFERNO_RUSH);
                        break;
                    }
                    case EVENT_FLAME_TORRENT:
                        DoCast(me, SPELL_FLAME_TORRENT);
                        events.RescheduleEvent(EVENT_FLAME_TORRENT, urand(15000, 23000));
                        break;
                    case EVENT_BURNING_BLOOD:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f , true))
                            DoCast(target, SPELL_BURNING_BLOOD);
                        events.RescheduleEvent(EVENT_BURNING_BLOOD, urand(21000, 24000));
                        break;
                    case EVENT_AEGIS_OF_FLAMES:
                        Talk(SAY_IGNACIOUS_SPELL);
                        DoCast(me, SPELL_AEGIS_OF_FLAMES);
                        events.RescheduleEvent(EVENT_AEGIS_OF_FLAMES, 60000);
                        events.RescheduleEvent(EVENT_RISING_FLAMES, 1000);
                        break;
                    case EVENT_RISING_FLAMES:
                        DoCast(me, SPELL_RISING_FLAMES);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class boss_arion : public CreatureScript
{
    public:
        boss_arion() : CreatureScript("boss_arion") { }

        CreatureAI * GetAI(Creature * pCreature) const
        {
            return new boss_arionAI(pCreature);
        }

        struct boss_arionAI : public BossAI
        {
            boss_arionAI(Creature * pCreature) : BossAI(pCreature, DATA_COUNCIL), summons(me)
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
            SummonList summons;
            bool bPhaseTwo;
            bool bPhaseThree;

            void Reset()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _Reset();

                bPhaseTwo = false;
                bPhaseThree = false;
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->NearTeleportTo(me->GetHomePosition());
                summons.DespawnAll();
                events.Reset();
            }

            void JustReachedHome()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {                
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

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_ARION_KILL);
            }

            void SpellHit(Unit* attacker, const SpellInfo* spell)
            {
                if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_LIGHTNING_BLAST)
                        for (uint8 i = 0; i < 3; ++i)
                            if (spell->Effects[i]->Effect == SPELL_EFFECT_INTERRUPT_CAST)
                                me->InterruptSpell(CURRENT_GENERIC_SPELL);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetHealth(me->CountPctFromMaxHealth(24));
                }

                if (me->HealthBelowPct(25) && !bPhaseThree)
                {
                    DoAction(ACTION_PHASE_3);
                    DoCast(me, SPELL_ELEMENTAL_STATIS, true);
                    if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                        _terrastra->AI()->DoAction(ACTION_PHASE_3);
                    if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                        _feludius->AI()->DoAction(ACTION_PHASE_3);
                    if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                        _ignacious->AI()->DoAction(ACTION_PHASE_3);
                }
            }

            void DoAction(const int32 action)
            {
                switch(action)
                {
                    case ACTION_PHASE_2:
                        if (!bPhaseTwo)
                        {
                            bPhaseTwo = true;
                            //Talk(SAY_ARION_AGGRO);
                            events.RescheduleEvent(EVENT_ARION_INTRO, 1000);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            events.RescheduleEvent(EVENT_CALL_WINDS, urand(10000, 11000));
                            events.RescheduleEvent(EVENT_DISPERSE, urand(15000, 20000));
                            events.RescheduleEvent(EVENT_LIGHTNING_ROD, urand(20000, 30000));
                        }
                        break;
                    case ACTION_THUNDERSHOCK:
                        events.RescheduleEvent(EVENT_THUNDERSHOCK, urand(30000, 34000));
                        break;
                    case ACTION_PHASE_3:
                        if (!bPhaseThree)
                        {
                            bPhaseThree = true;
                            summons.DespawnAll();
                            events.Reset();
                            me->StopAttack();
                            me->RemoveAllAuras();
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->NearTeleportTo(
                                groundPos[4].GetPositionX(),
                                groundPos[4].GetPositionY(),
                                groundPos[4].GetPositionZ(),
                                0.0f);
                            events.RescheduleEvent(EVENT_PHASE_3_1, 2000);
                        }
                        break;
                }
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
                    case EVENT_ARION_INTRO:
                        Talk(SAY_ARION_AGGRO);
                        break;
                    case EVENT_PHASE_3_1:
                        Talk(SAY_ARION_SPECIAL);
                        me->GetMotionMaster()->MovePoint(1, groundPos[6]);
                        break;
                    case EVENT_CALL_WINDS:
                        Talk(SAY_ARION_SPELL);
                        DoCast(me, SPELL_CALL_WINDS);
                        events.RescheduleEvent(EVENT_CALL_WINDS, urand(20000, 30000));
                        break;
                    case EVENT_DISPERSE:
                        //DoCast(me, SPELL_DISPERSE);
                        //events.RescheduleEvent(EVENT_LIGHTNING_BLAST, 2000);
                        DoCast(me->getVictim(), SPELL_LIGHTNING_BLAST);
                        events.RescheduleEvent(EVENT_DISPERSE, urand(30000, 40000));
                        break;
                    case EVENT_LIGHTNING_BLAST:
                        DoCast(me->getVictim(), SPELL_LIGHTNING_BLAST);
                        break;
                    case EVENT_LIGHTNING_ROD:
                        me->CastCustomSpell(SPELL_LIGHTNING_ROD, SPELLVALUE_MAX_TARGETS, RAID_MODE(1, 3, 1, 3), 0, false);
                        events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 5000);
                        events.RescheduleEvent(EVENT_LIGHTNING_ROD, urand(25000, 30000));
                        break;
                    case EVENT_CHAIN_LIGHTNING:
                        DoCast(me, SPELL_CHAIN_LIGHTNING_DUMMY);
                        break;
                    case EVENT_THUNDERSHOCK:
                        DoCast(me, SPELL_THUNDERSHOCK);
                        if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                            _terrastra->AI()->DoAction(ACTION_QUAKE);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class boss_terrastra: public CreatureScript
{
    public:
        boss_terrastra() : CreatureScript("boss_terrastra") { }

        CreatureAI * GetAI(Creature * pCreature) const
        {
            return new boss_terrastraAI(pCreature);
        }

        struct boss_terrastraAI : public BossAI
        {
            boss_terrastraAI(Creature * pCreature) : BossAI(pCreature, DATA_COUNCIL), summons(me)
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
            SummonList summons;
            bool bPhaseTwo;
            bool bPhaseThree;

            void Reset()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _Reset();

                bPhaseTwo = false;
                bPhaseThree = false;
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->NearTeleportTo(me->GetHomePosition());
                summons.DespawnAll();
                events.Reset();
            }

            void JustReachedHome()
            {
                if (instance->GetBossState(DATA_COUNCIL) == DONE)
                    me->DespawnOrUnsummon();

                _JustReachedHome();
            }

            void EnterCombat(Unit* who)
            {
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

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_TERRASTRA_KILL);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetHealth(me->CountPctFromMaxHealth(24));
                }

                if (me->HealthBelowPct(25) && !bPhaseThree)
                {
                    DoCast(me, SPELL_ELEMENTAL_STATIS, true);
                    DoAction(ACTION_PHASE_3);
                    if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                        _arion->AI()->DoAction(ACTION_PHASE_3);
                    if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                        _feludius->AI()->DoAction(ACTION_PHASE_3);
                    if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                        _ignacious->AI()->DoAction(ACTION_PHASE_3);
                }
            }

            void DoAction(const int32 action)
            {
                switch(action)
                {
                    case ACTION_PHASE_2:
                        if (!bPhaseTwo)
                        {
                            bPhaseTwo = true;
                            //Talk(SAY_TERRASTRA_AGGRO);
                            events.RescheduleEvent(EVENT_TERRASTRA_INTRO, 4000);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            events.RescheduleEvent(EVENT_HARDEN_SKIN, urand(20000, 30000));
                            events.RescheduleEvent(EVENT_ERUPTION, urand(15000, 20000));
                            events.RescheduleEvent(EVENT_GRAVITY_WELL, urand(20000, 30000));
                            events.RescheduleEvent(EVENT_QUAKE, urand(30000, 35000));
                        }
                        break;
                    case ACTION_QUAKE:
                        events.RescheduleEvent(EVENT_QUAKE, urand(30000, 34000));
                        break;
                    case ACTION_PHASE_3:
                        if (!bPhaseThree)
                        {
                            bPhaseThree = true;
                            events.Reset();
                            summons.DespawnAll();
                            me->RemoveAllAuras();
                            me->StopAttack();
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->NearTeleportTo(
                                groundPos[5].GetPositionX(),
                                groundPos[5].GetPositionY(),
                                groundPos[5].GetPositionZ(),
                                0.0f);
                            events.RescheduleEvent(EVENT_PHASE_3_2, 6000);
                        }
                        break;
                }
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
                    case EVENT_TERRASTRA_INTRO:
                        Talk(SAY_TERRASTRA_AGGRO);
                        break;
                    case EVENT_PHASE_3_2:
                        Talk(SAY_TERRASTRA_SPECIAL);
                        me->GetMotionMaster()->MovePoint(1, groundPos[6]);
                        break;
                    case EVENT_HARDEN_SKIN:
                        DoCast(me, SPELL_HARDEN_SKIN);
                        events.RescheduleEvent(EVENT_HARDEN_SKIN, urand(40000, 50000));
                        break;
                    case EVENT_ERUPTION:
                        DoCast(me, SPELL_ERUPTION_DUMMY);
                        events.RescheduleEvent(EVENT_ERUPTION, urand(13000, 16000));
                        break;
                    case EVENT_GRAVITY_WELL:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                            DoCast(target, SPELL_GRAVITY_WELL);
                        events.RescheduleEvent(EVENT_GRAVITY_WELL, urand(20000, 30000));
                        break;
                    case EVENT_QUAKE:
                        Talk(SAY_TERRASTRA_SPELL);
                        DoCast(me, SPELL_QUAKE);
                        if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                            _arion->AI()->DoAction(ACTION_THUNDERSHOCK);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class boss_elementium_monstrosity : public CreatureScript
{
    public:
        boss_elementium_monstrosity() : CreatureScript("boss_elementium_monstrosity") { }

        CreatureAI * GetAI(Creature * pCreature) const
        {
            return new boss_elementium_monstrosityAI(pCreature);
        }

        struct boss_elementium_monstrosityAI : public BossAI
        {
            boss_elementium_monstrosityAI(Creature * pCreature) : BossAI(pCreature, DATA_COUNCIL), summons(me)
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
            SummonList summons;

            void Reset()
            {
                summons.DespawnAll();
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                Talk(SAY_MONSTROSITY_AGGRO);
                DoCast(me, SPELL_CRYOGENIC_AURA);
                events.RescheduleEvent(EVENT_ELECTRIC_INSTABILITY, 5000);
                events.RescheduleEvent(EVENT_GRAVITY_CRUSH, urand(7000, 20000));
                events.RescheduleEvent(EVENT_LAVA_SEED, urand(8000, 15000));
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

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_MONSTROSITY_KILL);
            }

            void JustDied(Unit* killer)
            {
                _JustDied();
                summons.DespawnAll();
                Talk(SAY_MONSTROSITY_DEATH);
                if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                {
                    _feludius->SetVisible(true);
                    _feludius->AI()->EnterEvadeMode();
                }
                if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                {
                    _arion->SetVisible(true);
                    _arion->AI()->EnterEvadeMode();
                }
                if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                {
                    _terrastra->SetVisible(true);
                    _terrastra->AI()->EnterEvadeMode();
                }
                if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                {
                    _ignacious->SetVisible(true);
                    _ignacious->AI()->EnterEvadeMode();
                }
            }

            void EnterEvadeMode()
            {
                summons.DespawnAll();
                if (Creature* _feludius = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_FELUDIUS)))
                {
                    _feludius->SetVisible(true);
                    _feludius->AI()->EnterEvadeMode();
                }
                if (Creature* _arion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARION)))
                {
                    _arion->SetVisible(true);
                    _arion->AI()->EnterEvadeMode();
                }
                if (Creature* _terrastra = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TERRASTRA)))
                {
                    _terrastra->SetVisible(true);
                    _terrastra->AI()->EnterEvadeMode();
                }
                if (Creature* _ignacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                {
                    _ignacious->SetVisible(true);
                    _ignacious->AI()->EnterEvadeMode();
                }
                BossAI::EnterEvadeMode();
                me->DespawnOrUnsummon();
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
                    case EVENT_LAVA_SEED:
                        Talk(SAY_MONSTROSITY_SPELL);
                        DoCast(me, SPELL_LAVA_SEED);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        {
                            me->SummonCreature(NPC_ASCENDANT_COUNCIL_PLUME_STALKER,
                                target->GetPositionX(),
                                target->GetPositionY(),
                                target->GetPositionZ(),
                                0.0f, TEMPSUMMON_TIMED_DESPAWN, 6000);
                        }
                        events.RescheduleEvent(EVENT_LAVA_SEED, urand(20000, 24000));
                        break;
                    case EVENT_GRAVITY_CRUSH:
                        me->CastCustomSpell(SPELL_GRAVITY_CRUSH, SPELLVALUE_MAX_TARGETS, RAID_MODE(1, 3, 1, 3), 0, false);
                        events.RescheduleEvent(EVENT_GRAVITY_CRUSH, urand(22000, 24000));
                        break;
                    case EVENT_ELECTRIC_INSTABILITY:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            DoCast(target, SPELL_ELECTRIC_INSTABILITY);
                        events.RescheduleEvent(EVENT_ELECTRIC_INSTABILITY, 1000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};


class npc_violent_cyclone : public CreatureScript
{
public:
    npc_violent_cyclone() : CreatureScript("npc_violent_cyclone") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_violent_cycloneAI(pCreature);
    }

    struct npc_violent_cycloneAI : public Scripted_NoMovementAI
    {
        npc_violent_cycloneAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset()
        {
            if (!instance)
                return;

            DoCast(me, SPELL_LASHING_WINDS);
        }

        void UpdateAI(uint32 diff)
        {
        };

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }
    };
};

class npc_bt_gravity_well : public CreatureScript
{
public:
    npc_bt_gravity_well() : CreatureScript("npc_bt_gravity_well") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_bt_gravity_wellAI(pCreature);
    }

    struct npc_bt_gravity_wellAI : public Scripted_NoMovementAI
    {
        npc_bt_gravity_wellAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_GRAVITY_WELL_DMG, 3000);
            DoCast(me, SPELL_GRAVITY_WELL_DUMMY);
            
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_GRAVITY_WELL_DMG:
                    me->RemoveAurasDueToSpell(SPELL_GRAVITY_WELL_DUMMY);
                    DoCast(me, SPELL_MAGNETIC_PULL);
                    break;
                }
            }
        };

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }
    };
};

class npc_ignacious_inferno_leap : public CreatureScript
{
    public:
        npc_ignacious_inferno_leap() : CreatureScript("npc_ignacious_inferno_leap") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ignacious_inferno_leapAI(pCreature);
        }

        struct npc_ignacious_inferno_leapAI : public ScriptedAI
        {
            npc_ignacious_inferno_leapAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetSpeed(MOVE_RUN, 3.0f);
                me->SetSpeed(MOVE_WALK, 3.0f);
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript* instance;

            void UpdateAI(uint32 diff)
            {
                if (!instance)
                    me->DespawnOrUnsummon();

                if (!me->FindNearestCreature(NPC_INFERNO_RUSH, 5.0f))
                    if (Creature* pIgnacious = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_IGNACIOUS)))
                        pIgnacious->SummonCreature(NPC_INFERNO_RUSH, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 20000);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                    if (id == 1001)
                        me->DespawnOrUnsummon();
            }

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class npc_ignacious_inferno_rush : public CreatureScript
{
    public:
        npc_ignacious_inferno_rush() : CreatureScript("npc_ignacious_inferno_rush") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ignacious_inferno_rushAI(pCreature);
        }

        struct npc_ignacious_inferno_rushAI : public Scripted_NoMovementAI
        {
            npc_ignacious_inferno_rushAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                DoCast(me, SPELL_INFERNO_RUSH_AURA);
            }

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class npc_feludius_water_bomb : public CreatureScript
{
    public:
        npc_feludius_water_bomb() : CreatureScript("npc_feludius_water_bomb") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_feludius_water_bombAI(pCreature);
        }

        struct npc_feludius_water_bombAI : public Scripted_NoMovementAI
        {
            npc_feludius_water_bombAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void SpellHit(Unit* attacker, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_WATER_BOMB)
                    DoCastAOE(SPELL_WATER_BOMB_1);
            }

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class npc_ignacious_flame_strike : public CreatureScript
{
    public:
        npc_ignacious_flame_strike() : CreatureScript("npc_ignacious_flame_strike") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ignacious_flame_strikeAI(pCreature);
        }

        struct npc_ignacious_flame_strikeAI : public Scripted_NoMovementAI
        {
            npc_ignacious_flame_strikeAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                DoCast(SPELL_FLAME_STRIKE);
            }

            void SpellHit(Unit* attacker, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_FLAME_STRIKE_DUMMY)
                {
                    me->RemoveAurasDueToSpell(SPELL_FLAME_STRIKE);
                    DoCastAOE(SPELL_FLAME_STRIKE_AURA);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (me->HasAura(SPELL_FLAME_STRIKE_AURA))
                {
                    if (Creature* pOrb = me->FindNearestCreature(NPC_FROZEN_ORB, 5.0f))
                    {
                        pOrb->DespawnOrUnsummon();
                        me->DespawnOrUnsummon();
                    }
                }
            }

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class npc_liquid_ice : public CreatureScript
{
    public:
        npc_liquid_ice() : CreatureScript("npc_liquid_ice") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_liquid_iceAI(pCreature);
        }

        struct npc_liquid_iceAI : public Scripted_NoMovementAI
        {
            npc_liquid_iceAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript* instance;

            void Reset()
            {
                if (!instance)
                    return;

                DoCast(me, SPELL_LIQUID_ICE);
            }

            void UpdateAI(uint32 diff)
            {
            };

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class npc_ascendant_council_plume_stalker : public CreatureScript
{
    public:
        npc_ascendant_council_plume_stalker() : CreatureScript("npc_ascendant_council_plume_stalker") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ascendant_council_plume_stalkerAI(pCreature);
        }

        struct npc_ascendant_council_plume_stalkerAI : public Scripted_NoMovementAI
        {
            npc_ascendant_council_plume_stalkerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                if (!instance)
                    return;

                events.Reset();
                DoCast(me, SPELL_LAVA_SEED_DUMMY);
                events.RescheduleEvent(EVENT_LAVA_PLUME, 3000);
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_LAVA_PLUME:
                        me->RemoveAurasDueToSpell(SPELL_LAVA_SEED_DUMMY);
                        DoCast(me, SPELL_LAVA_PLUME);
                        break;
                    }
                }
            };

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class npc_eruption_target : public CreatureScript
{
    public:
        npc_eruption_target() : CreatureScript("npc_eruption_target") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_eruption_targetAI(pCreature);
        }

        struct npc_eruption_targetAI : public Scripted_NoMovementAI
        {
            npc_eruption_targetAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                if (!instance)
                    return;

                DoCast(me, SPELL_ERUPTION_AURA);
                events.RescheduleEvent(EVENT_ERUPTION_DMG, 3000);
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ERUPTION_DMG:
                        me->RemoveAurasDueToSpell(SPELL_ERUPTION_AURA);
                        DoCast(me, SPELL_ERUPTION);
                        break;
                    }
                }
            };

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class spell_feludius_glaciate : public SpellScriptLoader
{
    public:
        spell_feludius_glaciate() : SpellScriptLoader("spell_feludius_glaciate") { }


        class spell_feludius_glaciate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_feludius_glaciate_SpellScript);


            void HandleScript()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                float distance = GetHitUnit()->GetExactDist2d(GetCaster());
                if (distance < 5.0f)
                    return;

                float distVar = distance >= 40.0f ? 4 : (10.0f/3.0f);
                SetHitDamage(int32(GetHitDamage() * distVar / distance));

                if (GetHitUnit()->HasAura(SPELL_WATERLOGGED))
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_FROZEN, true);
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_feludius_glaciate_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_feludius_glaciate_SpellScript();
        }
};

class spell_ignis_rising_flames : public SpellScriptLoader
{
    public:
        spell_ignis_rising_flames() : SpellScriptLoader("spell_ignis_rising_flames") { }
 
        class spell_ignis_rising_flames_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ignis_rising_flames_AuraScript)

            void OnPeriodic(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_RISING_FLAMES_BUFF, true);
            }
 
            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_ignis_rising_flames_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };
 
        AuraScript* GetAuraScript() const
        {
            return new spell_ignis_rising_flames_AuraScript();
        }
};

class spell_ignacious_inferno_rush: public SpellScriptLoader
{
    public:
        spell_ignacious_inferno_rush() : SpellScriptLoader("spell_ignacious_inferno_rush") { }


        class spell_ignacious_inferno_rush_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ignacious_inferno_rush_SpellScript);


            void HandleScript()
            {
                if (!GetHitUnit())
                    return; 

                GetHitUnit()->RemoveAurasDueToSpell(SPELL_WATERLOGGED);

            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_ignacious_inferno_rush_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ignacious_inferno_rush_SpellScript();
        }
};

class spell_arion_lashing_winds : public SpellScriptLoader
{
    public:
        spell_arion_lashing_winds() : SpellScriptLoader("spell_arion_lashing_winds") { }

        class spell_arion_lashing_winds_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_arion_lashing_winds_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if(!GetCaster() || !GetHitUnit())
                    return;
                
                GetCaster()->CastSpell(GetHitUnit(), SPELL_SWIRLING_WINDS, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_arion_lashing_winds_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_arion_lashing_winds_SpellScript();
        }
};

class spell_arion_chain_lightning : public SpellScriptLoader
{
    public:
        spell_arion_chain_lightning() : SpellScriptLoader("spell_arion_chain_lightning") { }

        class spell_arion_chain_lightning_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_arion_chain_lightning_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if(!GetCaster() || !GetHitUnit())
                    return;

                if (GetHitUnit()->HasAura(SPELL_LIGHTNING_ROD))
                {
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_CHAIN_LIGHTNING, true);
                    GetHitUnit()->RemoveAurasDueToSpell(SPELL_LIGHTNING_ROD);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_arion_chain_lightning_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_arion_chain_lightning_SpellScript();
        }
};

class spell_arion_disperse : public SpellScriptLoader
{
    public:
        spell_arion_disperse() : SpellScriptLoader("spell_arion_disperse") { }

        class spell_arion_disperse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_arion_disperse_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if(!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_DISPERSE_1, true);
                uint32 i = urand(0, 33);
                GetCaster()->NearTeleportTo(randomPos[i].GetPositionX(), randomPos[i].GetPositionY(), randomPos[i].GetPositionZ(), randomPos[i].GetOrientation(), true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_arion_disperse_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_arion_disperse_SpellScript();
        }
};

class spell_terrastra_eruption : public SpellScriptLoader
{
    public:
        spell_terrastra_eruption() : SpellScriptLoader("spell_terrastra_eruption") { }

        class spell_terrastra_eruption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_terrastra_eruption_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if(!GetCaster())
                    return;
                if (Unit* target = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                    GetCaster()->CastSpell(target, SPELL_ERUPTION_SUMMON, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_terrastra_eruption_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_terrastra_eruption_SpellScript();
        }
};

class spell_arion_thundershock : public SpellScriptLoader
{
    public:
        spell_arion_thundershock() : SpellScriptLoader("spell_arion_thundershock") { }


        class spell_arion_thundershock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_arion_thundershock_SpellScript);


            void HandleScript()
            {

                if (!GetHitUnit())
                    return;

                if (GetHitUnit()->HasAura(SPELL_GROUNDED))
                    SetHitDamage(0);
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_arion_thundershock_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_arion_thundershock_SpellScript();
        }
};

class spell_terrastra_quake : public SpellScriptLoader
{
    public:
        spell_terrastra_quake() : SpellScriptLoader("spell_terrastra_quake") { }


        class spell_terrastra_quake_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_terrastra_quake_SpellScript);


            void HandleScript()
            {
                if (!GetHitUnit())
                    return;

                if (GetHitUnit()->HasAura(SPELL_SWIRLING_WINDS))
                    SetHitDamage(0);
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_terrastra_quake_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_terrastra_quake_SpellScript();
        }
};

class spell_terrastra_harden_skin : public SpellScriptLoader
{
    public:
        spell_terrastra_harden_skin() : SpellScriptLoader("spell_terrastra_harden_skin") { }

        class spell_terrastra_harden_skin_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_terrastra_harden_skin_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (!GetTarget())
                    return;

                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                {
                    float bp = aurEff->GetBaseAmount();
                    GetTarget()->CastCustomSpell(SPELL_SHATTER, SPELLVALUE_BASE_POINT0, bp, GetTarget(), true);
                }
            }

            void OnAbsorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                dmgInfo.ModifyDamage(-(int32)(dmgInfo.GetDamage()/2));
            }
            
            void Register()
            {
                OnEffectRemove += AuraEffectApplyFn(spell_terrastra_harden_skin_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_terrastra_harden_skin_AuraScript::OnAbsorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_terrastra_harden_skin_AuraScript();
        }
};

class spell_monstrosity_cryogenic_aura : public SpellScriptLoader
{
    public:
        spell_monstrosity_cryogenic_aura() : SpellScriptLoader("spell_monstrosity_cryogenic_aura") { }
 
        class spell_monstrosity_cryogenic_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monstrosity_cryogenic_aura_AuraScript)

            void OnPeriodic(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (GetCaster()->FindNearestCreature(NPC_LIQUID_ICE, 4.0f))
                    return;

                GetCaster()->SummonCreature(NPC_LIQUID_ICE,
                    GetCaster()->GetPositionX(),
                    GetCaster()->GetPositionY(),
                    GetCaster()->GetPositionZ(),
                    0.0f);
            }
 
            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monstrosity_cryogenic_aura_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };
 
        AuraScript* GetAuraScript() const
        {
            return new spell_monstrosity_cryogenic_aura_AuraScript();
        }
};

class spell_elemental_statis : public SpellScriptLoader
{
    public:
        spell_elemental_statis() : SpellScriptLoader("spell_elemental_statis") { }

        class spell_elemental_statis_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_elemental_statis_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (!GetCaster() || !GetTarget())
                    return;

                GetTarget()->RemoveAurasDueToSpell(SPELL_SWIRLING_WINDS);
                GetTarget()->RemoveAurasDueToSpell(SPELL_GROUNDED);
            }
            
            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_elemental_statis_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_elemental_statis_AuraScript();
        }
};

void AddSC_boss_ascendant_council()
{
    new boss_feludius();
    new boss_ignacious();
    new boss_arion();
    new boss_terrastra();
    new boss_elementium_monstrosity();
    new npc_feludius_water_bomb();
    new npc_ignacious_inferno_rush();
    new npc_ignacious_inferno_leap();
    new npc_ignacious_flame_strike();
    new npc_violent_cyclone();
    new npc_bt_gravity_well();
    new npc_liquid_ice();
    new npc_ascendant_council_plume_stalker();
    new npc_eruption_target();
    new spell_feludius_glaciate();
    new spell_ignis_rising_flames();
    new spell_ignacious_inferno_rush();
    new spell_arion_chain_lightning();
    new spell_arion_disperse();
    new spell_arion_thundershock();
    new spell_terrastra_eruption();
    new spell_terrastra_harden_skin();
    new spell_terrastra_quake();
    new spell_arion_lashing_winds();
    new spell_monstrosity_cryogenic_aura();
    new spell_elemental_statis();
}
