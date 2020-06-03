
////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///  Coded by Davethebrave
////////////////////////////////////////////////////////////////////////////////

#include "ScriptedCreature.h"
#include "auchindoun.hpp"

enum eTerongorSpells
{
    SpellAgony                              = 156925,
    SpellChaosBolt                          = 156975,
    SpellChaosWaveDummy                     = 157001,
    SpellChaosWaveDmg                       = 157002,
    SpellConflagrate                        = 154083,
    SpellCorruptionDmg                      = 156842,
    SpellCurseOfExhaustionDebuff            = 164841,
    SpellDemonicLeapDummy                   = 157039,
    SpellDemonicLeapAreatriger              = 157040,
    SpellDoomBuff                           = 156965,
    SpellDrainLife                          = 156854,
    SpellImmolate                           = 156964,
    SpellIncinrate                          = 156963,
    SpellRainOfFire                         = 156974,
    SpellSeedOfMalevolenceApplyAura         = 156921,
    SpellSeedOfMalevolenceBuff              = 166451,
    SpellSeedOfMalevolenceDmg               = 156924,
    SpellSeedOfMalevolenceVisualTriger      = 166462,
    SpellShadowBolt                         = 156829,
    SpellTouchOfChaosDummy                  = 169028,
    SpellTouchOfChaosDmg                    = 156968,
    SpellUnstableAffliction                 = 156954,
    SpellWrathCleave                        = 159035,
    SpellWrathStorm                         = 159033,
    SpellTranscend                          = 164009,
    SpellAfflictionTransform                = 156863,
    SpellDestructionTransform               = 156866,
    SpellDemonologyTransform                = 156919,
    SpellSummonAbyssalMeteor                = 164508,
    SpellSummonAbyssalGroundEffect          = 159681,
    SpellSummonAbyssalDummy                 = 157214,
    SpellSummonAbyssalSummonSpell           = 157216,
    SpellDemonicCircleVisual                = 149133,
    SpellTeronogorShield                    = 157017,
    SpellBrokenSouls                        = 72398,
    SpellSoulBarrage                        = 72305,
    SpellGuldenSoulVisual                   = 166453 
};

enum eTerongorEvents
{
    EventAgony = 1,
    EventChaosBolt,
    EventChaosWave,
    EventConflagrate,
    EventCorruption,
    EventCurseOfExhaustion,
    EventDemonicLeap,
    EventDoom,
    EventDrainLife,
    EventImmolate,
    EventIncinrate,
    EventRainOfFire,
    EventSeedOfMalevolence,
    EventTouchOfChaos,
    EventUnstableAffliction,
    EventShadowBolt,
    EventWrathcleave,
    EventWrathstorm,
    EventBloomOfMalevolence,
    EventDepassive,
    EventTransform,
    EventTransformRemovePassive
};

enum eTerongorTalks
{
    TERONGOR_INTRO_01 = 42, ///< who know the draenei held such...delicious treasure in their temple?  (44423)
    TERONGOR_INTRO_02 = 43, ///< Do you dare challenge me,defenders of Auchindoun?!(44424)
    TERONGOR_INTRO_03 = 44, ///< Such decadence...it will all burn. (44425)
    TERONGOR_INTRO_05 = 45, ///< Gul'dan, such foolshness. This gift...so much more you could ever know...(44427)
    TERONGOR_INTRO_06 = 46, ///< Long have i waited...(44428)
    TERONGOR_INTRO_07 = 47, ///< ...hungered... to be more... (44429)
    TERONGOR_INTRO_08 = 48, ///<  And now, all shall bow before me!(44430)
    TERONGOR_KILL_01  = 49, ///< All will fall before me!(44431)
    TERONGOR_AGGRO_01 = 50, ///< This power. you will be the first to know it.(44418)
    TERONGOR_SPELL_01 = 52, ///< Destruction!(44433)
    TERONGOR_SPELL_02 = 53, ///< Wither.. away! (44434)
    TERONGOR_SPELL_03 = 53, ///< I become something greater!(44435)
    TERONGOR_SPELL_04 = 54, ///< Die! (44436)
    TERONGOR_SPELL_05 = 55, ///< Your demise awaits! (44437)
    TERONGOR_SPELL_06 = 56, ///< Quickly now. (44438)
    TERONGOR_SPELL_07 = 57, ///< More! I...need...more!(44439)
    TERONGOR_EVENT_01 = 60, ///< Jorrun.. who battle demons in life, in death.. your power will feed them! (44420)
    TERONGOR_EVENT_02 = 61, ///< Joraa, paragon of order - i shall twist your soul to power Destruction!(44421)
    TERONGOR_EVENT_03 = 61, ///< Elum, life time of healing... now, you shall fuel Deziz and Decay!(44422)
    TERONGOR_DEATH    = 100 ///< (44419)
};

enum eTeronogorActions
{
    ActionTransport = 1,
    ActionChoosePower
};

enum eTeronogorCreatures
{
    TriggerSummonAbyssal = 213593
};

enum eTeronogorTransformations
{
    TransformationAffliction  = 0,
    TransformationDestruction = 1,
    TransformationDemonology  = 2,
    TransformationPreChannel  = 3,
    TransformationOccur       = 4,
    TransformationOccured     = 5
};

enum eTeronogorMovements
{
    MovementBossDeathSceneStart = 1,
    MovementBossDeathSceneStage01,
    MovementBossDeathSceneEnd
};

Position const g_EndBossCinematicTeleport = {1904.59f, 2982.96f, 16.844f};
Position const g_EndBossCinematicTeleportPreDespawn = {1930.304f, 3056.913f, 33.249f};
Position const g_EndBossCinematicTeleportDespawn = {1925.326f, 3043.384f, -53.435f};

class EventTeronogorTransform : public BasicEvent
{
public:

    explicit EventTeronogorTransform(Unit* unit, int32 value, int32 p_TransformationType) : BasicEvent(), m_Obj(unit), m_Modifier(value), m_tType(p_TransformationType), m_Event(0)
    {
    }

    bool Execute(uint64 /*p_CurrTime*/, uint32 /*diff*/) override
    {
        enum eTeronogorTransforkmSpells
        {
            SpellDrainSoulVisual = 156862,
            SpellAfflictionTransform = 156863,
            SpellDestructionTransform = 156866,
            SpellDemonologyTransform = 156919
        };

        if (m_Obj)
        {
            if (InstanceScript* instance = m_Obj->GetInstanceScript())
            {
                if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
                {
                    if (Creature* l_Iruun = instance->instance->GetCreature(instance->GetGuidData(DataIruun)))
                    {
                        if (Creature* l_Jorra = instance->instance->GetCreature(instance->GetGuidData(DataJorra)))
                        {
                            if (Creature* l_Elum = instance->instance->GetCreature(instance->GetGuidData(DataElum)))
                            {
                                if (l_Teronogor->IsAIEnabled)
                                {
                                    switch (m_Modifier)
                                    {
                                        case TransformationPreChannel:
                                        {
                                            switch (m_tType)
                                            {
                                                case TransformationAffliction:
                                                    l_Teronogor->CastSpell(l_Elum, SpellDrainSoulVisual);
                                                    break;
                                                case TransformationDestruction:
                                                    l_Teronogor->CastSpell(l_Jorra, SpellDrainSoulVisual);
                                                    break;
                                                case TransformationDemonology:
                                                    l_Teronogor->CastSpell(l_Iruun, SpellDrainSoulVisual);
                                                    break;
                                                default:
                                                    break;
                                            }

                                            l_Teronogor->m_Events.AddEvent(new EventTeronogorTransform(l_Teronogor, TransformationOccur, m_tType), l_Teronogor->m_Events.CalculateTime(5 * IN_MILLISECONDS));
                                            break;
                                        }
                                        case TransformationOccur:
                                        {
                                            l_Teronogor->RemoveAllAuras();
                                            l_Teronogor->CastStop();

                                            switch (m_tType)
                                            {
                                                case TransformationAffliction:
                                                    l_Teronogor->CastSpell(l_Teronogor, SpellAfflictionTransform);
                                                    break;
                                                case TransformationDestruction:
                                                    l_Teronogor->CastSpell(l_Teronogor, SpellDestructionTransform);
                                                    break;
                                                case TransformationDemonology:
                                                    l_Teronogor->CastSpell(l_Teronogor, SpellDemonologyTransform);
                                                    break;
                                                default:
                                                    break;
                                            }

                                            if (l_Teronogor->IsAIEnabled)
                                            {
                                                if (Unit* l_Victim = l_Teronogor->GetAI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                                                    l_Teronogor->Attack(l_Victim, true);
                                            }

                                            l_Teronogor->UpdatePosition(*l_Teronogor);
                                            l_Teronogor->SetReactState(REACT_DEFENSIVE);
                                            l_Teronogor->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
                                            break;
                                        }
                                        default:
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

private:
    Unit* m_Obj;
    int32 m_Modifier;
    int32 m_tType;
    int32 m_Event;
};

/// Teron'gor <Shadow Council> - 77734
class boss_teronogor : public CreatureScript
{
public:

    boss_teronogor() : CreatureScript("boss_teronogor") { }

    struct boss_teronogorAI : BossAI
    {
        boss_teronogorAI(Creature* creature) : BossAI(creature, DataBossTeronogor), m_SecondPhase(false), m_SoulTransport01(false), m_SoulTransport02(false), m_SoulTransport03(false), m_SoulTransport04(false), m_SoulTransport05(false)
        {
            m_Instance = me->GetInstanceScript();
            m_Intro = false;
            m_First = false;
            m_Death = false;
        }

        InstanceScript* m_Instance;
        bool m_Intro;
        bool m_First;
        bool m_SecondPhase;
        bool m_SoulTransport01;
        bool m_SoulTransport02;
        bool m_SoulTransport03;
        bool m_SoulTransport04;
        bool m_SoulTransport05;
        bool m_Death;

        void Reset() override
        {
            _Reset();

            events.Reset();
            m_SecondPhase = false;
            me->SetReactState(REACT_DEFENSIVE);

            if (!m_Death)
            {
                if (m_Instance)
                {
                    if (Creature* l_Jorra = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataIruun)))
                        me->CastSpell(l_Jorra, SpellDrainSoulVisual);
                }

                if (m_First)
                {
                    m_First = false;

                    m_SoulTransport01 = false;
                    m_SoulTransport02 = false;
                    m_SoulTransport03 = false;
                    m_SoulTransport04 = false;
                    m_SoulTransport05 = false;

                    me->AddAura(SpellTeronogorShield, me);
                    me->CastSpell(me, SpellDemonicCircleVisual);

                    //me->GetMap()->SetObjectVisibility(1000.0f);
                }
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who && who->IsInWorld() && who->IsPlayer() && me->IsWithinDistInMap(who, 18.0f) && !m_Intro)
            {
                m_Intro = true;
                Talk(TERONGOR_INTRO_01);
            }
        }

        void JustReachedHome() override
        {
            summons.DespawnAll();
            DespawnCreaturesInArea(CreatureFelborneAbyssal, me);

            if (m_Instance)
                instance->SetBossState(DataBossTeronogor, FAIL);
        }

        void DamageTaken(Unit* atacker, uint32 &p_Damage, DamageEffectType dmgType) override
        {
            if (me->GetHealthPct() <= 75 && !m_SecondPhase)
            {
                events.Reset();
                me->AttackStop();
                m_SecondPhase = true;
                me->SetReactState(REACT_PASSIVE);
                DoAction(ActionChoosePower);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }

        void DoAction(int32 const actionID) override
        {
            switch (actionID)
            {
                case ActionSoulMove1:
                    m_SoulTransport01 = true;
                    break;
                case ActionSoulMove2:
                    m_SoulTransport02 = true;
                    break;
                case ActionSoulMove3:
                    m_SoulTransport03 = true;
                    break;
                case ActionSoulMove4:
                    m_SoulTransport04 = true;
                    break;
                case ActionChoosePower:
                {
                    events.Reset();

                    switch (urand(TransformationAffliction, TransformationDemonology))
                    {
                        case TransformationAffliction: // Mender Elum - Affliction
                            events.Reset();
                            Talk(TERONGOR_EVENT_03);
                            events.RescheduleEvent(EventShadowBolt, urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventCurseOfExhaustion, 13 * IN_MILLISECONDS);
                            events.RescheduleEvent(EventSeedOfMalevolence, urand(22 * IN_MILLISECONDS, 25 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventAgony, 16 * IN_MILLISECONDS);
                            events.RescheduleEvent(EventDrainLife, urand(13 * IN_MILLISECONDS, 16 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventUnstableAffliction, 20 * IN_MILLISECONDS);
                            me->m_Events.AddEvent(new EventTeronogorTransform(me, TransformationPreChannel, 0), me->m_Events.CalculateTime(2 * IN_MILLISECONDS));
                            break;
                        case TransformationDestruction: // Arcanist Jorra's - destruction
                            events.Reset();
                            Talk(TERONGOR_EVENT_01);
                            events.RescheduleEvent(EventChaosBolt, 20 * IN_MILLISECONDS);
                            events.RescheduleEvent(EventImmolate, urand(10 * IN_MILLISECONDS, 14 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventConflagrate, urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventRainOfFire, 24 * IN_MILLISECONDS);
                            events.RescheduleEvent(EventIncinrate, 16 * IN_MILLISECONDS);
                            me->m_Events.AddEvent(new EventTeronogorTransform(me, TransformationPreChannel, 1), me->m_Events.CalculateTime(2 * IN_MILLISECONDS));
                            break;
                        case TransformationDemonology: // Vindication Iruun's - demonology
                            events.Reset();
                            Talk(TERONGOR_EVENT_02);
                            events.RescheduleEvent(EventDoom, urand(8 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventDemonicLeap, urand(10 * IN_MILLISECONDS, 14 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventCurseOfExhaustion, 18 * IN_MILLISECONDS);
                            events.RescheduleEvent(EventCorruption, urand(10 * IN_MILLISECONDS, 14 * IN_MILLISECONDS));
                            events.RescheduleEvent(EventChaosBolt, 25 * IN_MILLISECONDS);
                            events.RescheduleEvent(EventTouchOfChaos, 16 * IN_MILLISECONDS);
                            me->m_Events.AddEvent(new EventTeronogorTransform(me, TransformationPreChannel, 2), me->m_Events.CalculateTime(2 * IN_MILLISECONDS));
                            break;
                        default:
                            break;
                    }
                }
                default:
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            me->CastStop();
            me->RemoveAllAuras();
            Talk(TERONGOR_AGGRO_01);
            me->RemoveAura(SpellTeronogorShield);
            events.RescheduleEvent(EventShadowBolt, urand(8 * IN_MILLISECONDS, 16 * IN_MILLISECONDS));
            events.RescheduleEvent(EventCorruption, urand(10 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
            events.RescheduleEvent(EventRainOfFire, 21 * IN_MILLISECONDS);
            events.RescheduleEvent(EventDrainLife, 16 * IN_MILLISECONDS);

            if (m_Instance)
            {
                m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                if (m_Instance->instance->IsHeroic())
                {
                    Position l_Position;
                    me->GetRandomNearPosition(l_Position, 10.0f);
                    me->SummonCreature(TriggerSummonAbyssal, l_Position, TEMPSUMMON_MANUAL_DESPAWN);
                }
            }
        }

        void KilledUnit(Unit* who) override
        {
            if (who && who->IsPlayer())
                Talk(TERONGOR_KILL_01);
        }

        void MovementInform(uint32 /*moveType*/, uint32 id) override
        {
            switch (id)
            {
                case MovementBossDeathSceneStart:
                    if (Player* l_Nearest = me->FindNearestPlayer(100.0f, true))
                        me->SetFacingToObject(l_Nearest);

                    Talk(TERONGOR_DEATH);

                    me->SetSpeed(MOVE_FLIGHT, 0.7f, true);
                    me->CastSpell(me, SpellGuldenSoulVisual);
                    me->GetMotionMaster()->MoveTakeoff(MovementBossDeathSceneStage01, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 4.0f);
                    break;
                case MovementBossDeathSceneStage01:
                    me->CastSpell(me, SpellGuldenSoulVisual);

                    me->SetSpeed(MOVE_FLIGHT_BACK, 15.0f, true);
                    me->RemoveAura(SpellGuldenSoulVisual);
                    me->GetMotionMaster()->MoveBackward(MovementBossDeathSceneEnd, g_EndBossCinematicTeleportPreDespawn.GetPositionX(), g_EndBossCinematicTeleportPreDespawn.GetPositionY(), g_EndBossCinematicTeleportPreDespawn.GetPositionZ(), 15.0F);
                    break;
                case MovementBossDeathSceneEnd:
                    me->SetSpeed(MOVE_FLIGHT, 15.0f, true);
                    me->DespawnOrUnsummon(10 * IN_MILLISECONDS);
                    me->GetMotionMaster()->MoveBackward(1000, g_EndBossCinematicTeleportDespawn.GetPositionX(), g_EndBossCinematicTeleportDespawn.GetPositionY(), g_EndBossCinematicTeleportDespawn.GetPositionZ(), 15.0F);
                    break;
                default:
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();

            m_Death = true;

            DespawnCreaturesInArea(CreatureFelborneAbyssal, me);
            if (m_Instance)
                m_Instance->SetBossState(DataBossTeronogor, DONE);

            me->setFaction(FriendlyFaction);
            me->SetReactState(REACT_PASSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);

            if (Player* l_Nearest = me->FindNearestPlayer(100.0f, true))
                me->SetFacingToObject(l_Nearest);

            Talk(TERONGOR_DEATH);

            me->SetSpeed(MOVE_FLIGHT, 0.7f, true);
            me->GetMotionMaster()->MoveTakeoff(MovementBossDeathSceneStage01, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 4.0f);

            me->SummonGameObject(GameobjectChestAucheni, 1891.84f, 2973.80f, 16.844f, 5.664811f, 0, 0, 0, 0, 0);
            me->SummonCreature(CreatureSoulBinderTuulani01, 1911.65f, 2757.72f, 30.799f, 1.566535f, TEMPSUMMON_MANUAL_DESPAWN);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventShadowBolt:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellShadowBolt);

                    events.RescheduleEvent(EventShadowBolt, 6 * IN_MILLISECONDS);
                    break;
                }
                case EventCorruption:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true, -SpellCorruptionDmg))
                        me->CastSpell(l_Random, SpellCorruptionDmg);

                    events.RescheduleEvent(EventCorruption, 9 * IN_MILLISECONDS);
                    break;
                }
                case EventDrainLife:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellDrainLife);

                    events.RescheduleEvent(EventDrainLife, 16 * IN_MILLISECONDS);
                    break;
                }
                case EventCurseOfExhaustion:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true, -SpellCurseOfExhaustionDebuff))
                        me->CastSpell(l_Random, SpellCurseOfExhaustionDebuff);

                    events.RescheduleEvent(EventCurseOfExhaustion, 13 * IN_MILLISECONDS);
                    break;
                }
                case EventAgony:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellAgony);

                    events.RescheduleEvent(EventAgony, 16 * IN_MILLISECONDS);
                    break;
                }
                case EventUnstableAffliction:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true, -SpellUnstableAffliction))
                        me->CastSpell(l_Random, SpellUnstableAffliction);

                    events.RescheduleEvent(EventUnstableAffliction, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventSeedOfMalevolence:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellSeedOfMalevolenceApplyAura);

                    events.RescheduleEvent(EventSeedOfMalevolence, urand(22 * IN_MILLISECONDS, 25 * IN_MILLISECONDS));
                    break;
                }
                case EventChaosBolt:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(me->getVictim(), SpellChaosBolt);

                    events.RescheduleEvent(EventChaosBolt, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventImmolate:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellImmolate);

                    events.RescheduleEvent(EventImmolate, 12 * IN_MILLISECONDS);
                    break;
                }
                case EventConflagrate:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellConflagrate);

                    events.RescheduleEvent(EventConflagrate, urand(15 * IN_MILLISECONDS, 20 * IN_MILLISECONDS));
                    break;
                }
                case EventIncinrate:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellIncinrate);

                    events.RescheduleEvent(EventIncinrate, 6 * IN_MILLISECONDS);
                    break;
                }
                case EventRainOfFire:
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellRainOfFire, true);

                    events.RescheduleEvent(EventRainOfFire, 10 * IN_MILLISECONDS);
                    break;
                case EventDoom:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellDoomBuff);

                    events.RescheduleEvent(EventDoom, urand(8 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
                    break;
                }
                case EventDemonicLeap:
                {
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellDemonicLeapDummy);

                    events.RescheduleEvent(EventDemonicLeap, urand(18 * IN_MILLISECONDS, 30 * IN_MILLISECONDS));
                    break;
                }
                case EventChaosWave:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellChaosWaveDummy);

                    events.RescheduleEvent(EventChaosWave, urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS));
                    break;
                }
                case EventTouchOfChaos:
                {
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellTouchOfChaosDummy);

                    events.RescheduleEvent(EventTouchOfChaos, 16 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_teronogorAI(creature);
    }
};

/// Durag The Dominator <Shadow Council> - 77890
class auchindoun_teronogor_mob_durag : public CreatureScript
{
public:

    auchindoun_teronogor_mob_durag() : CreatureScript("auchindoun_teronogor_mob_durag") { }

    struct auchindoun_teronogor_mob_duragAI : ScriptedAI
    {
        auchindoun_teronogor_mob_duragAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = me->GetInstanceScript();
            m_First = true;
        }

        EventMap events;

        enum eTeronogorDuragSpells
        {
            GrimoireofServitude = 159021,
            SpellShadowBolt = 156829,
            SpellCorruptionDmg = 156842,
            SpellChaosWaveDummy = 157001
        };

        enum eTeronogorDuragEvents
        {
            EventShadowBolt = 1,
            EventCorruption,
            EventChaosWave
        };

        InstanceScript* m_Instance;
        bool m_First;

        void Reset() override
        {
            events.Reset();
            //if (me->GetMap())
            //    me->GetMap()->SetObjectVisibility(1000.0f);

            /// Cosmetic channel - 
            if (m_Instance)
            {
                if (Creature* l_Teronogor = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossTeronogor)))
                {
                    me->CastStop();
                    me->CastSpell(l_Teronogor, SpellDrainSoulVisual);
                }
            }

            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            me->CastStop();
            me->RemoveAllAuras();
            me->CastSpell(me, GrimoireofServitude);
            events.RescheduleEvent(EventShadowBolt, 6 * IN_MILLISECONDS);
            events.RescheduleEvent(EventCorruption, 9 * IN_MILLISECONDS);
            events.RescheduleEvent(EventChaosWave, 12 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventShadowBolt:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellShadowBolt);
                    events.RescheduleEvent(EventShadowBolt, 6 * IN_MILLISECONDS);
                    break;
                case EventCorruption:
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true, -SpellCorruptionDmg))
                        me->CastSpell(l_Random, SpellCorruptionDmg);
                    events.RescheduleEvent(EventCorruption, 9 * IN_MILLISECONDS);
                    break;
                case EventChaosWave:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellChaosWaveDummy);
                    events.RescheduleEvent(EventChaosWave, 20 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_teronogor_mob_duragAI(creature);
    }
};

/// Gulkosh <Shadow Council> - 78437
class auchindoun_teronogor_mob_gulkosh : public CreatureScript
{
public:

    auchindoun_teronogor_mob_gulkosh() : CreatureScript("auchindoun_teronogor_mob_gulkosh") { }

    struct auchindoun_teronogor_mob_gulkoshAI : ScriptedAI
    {
        auchindoun_teronogor_mob_gulkoshAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = me->GetInstanceScript();
            m_First = true;
        }

        EventMap events;

        enum eTeronogorGulkoshSpells
        {
            SpellShadowBolt = 156829,
            SpellUnstableAffliction = 156954,
            SpellDrainLife = 156854
        };

        enum eTeronogorGulkoshEvents
        {
            EventShadowBolt = 1,
            EventUnstableAffliction,
            EventDrainLife
        };

        InstanceScript* m_Instance;
        bool m_First;

        void Reset() override
        {
            events.Reset();

            //if (me->GetMap())
            //    me->GetMap()->SetObjectVisibility(1000.0f);

            if (m_Instance)
            {
                if (Creature* l_Teronogor = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossTeronogor)))
                {
                    me->CastStop();
                    me->CastSpell(l_Teronogor, SpellDrainSoulVisual);
                }
            }

            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            me->CastStop();
            me->RemoveAllAuras();
            events.RescheduleEvent(EventShadowBolt, 6 * IN_MILLISECONDS);
            events.RescheduleEvent(EventUnstableAffliction, 8 * IN_MILLISECONDS);
            events.RescheduleEvent(EventDrainLife, 10 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventShadowBolt:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellShadowBolt);
                    events.RescheduleEvent(EventShadowBolt, 6 * IN_MILLISECONDS);
                    break;
                case EventUnstableAffliction:
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellUnstableAffliction);
                    events.RescheduleEvent(EventUnstableAffliction, 8 * IN_MILLISECONDS);
                    break;
                case EventDrainLife:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Target, SpellDrainLife);
                    events.RescheduleEvent(EventDrainLife, 10 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_teronogor_mob_gulkoshAI(creature);
    }
};

/// Shaduum - 78728
class auchindoun_teronogor_mob_shaadum : public CreatureScript
{
public:
    auchindoun_teronogor_mob_shaadum() : CreatureScript("auchindoun_teronogor_mob_shaadum") { }

    struct auchindoun_teronogor_mob_shaadumAI : ScriptedAI
    {
        auchindoun_teronogor_mob_shaadumAI(Creature* creature) : ScriptedAI(creature)
        {
            m_First = true;
        }

        EventMap events;

        enum eShaddumSpells
        {
            SpellWrathStorm = 159033,
            SpellWrathCleave = 159035
        };

        enum eShaddumEvents
        {
            EventWrathcleave = 1,
            EventWrathstorm
        };

        bool m_First;

        void Reset() override
        {
            events.Reset();
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            events.RescheduleEvent(EventWrathcleave, 10 * IN_MILLISECONDS);
            events.RescheduleEvent(EventWrathstorm, urand(14 * IN_MILLISECONDS, 16 * IN_MILLISECONDS));
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventWrathstorm:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellWrathStorm);
                    events.RescheduleEvent(EventWrathstorm, urand(8 * IN_MILLISECONDS, 10 * IN_MILLISECONDS));
                    break;
                case EventWrathcleave:
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellWrathCleave);
                    events.RescheduleEvent(EventWrathcleave, urand(10 * IN_MILLISECONDS, 14 * IN_MILLISECONDS));
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_teronogor_mob_shaadumAI(creature);
    }
};

/// Grom'tash The Destructor <Shadow Council> - 77889
class auchindoun_teronogor_mob_gromkash : public CreatureScript
{
public:

    auchindoun_teronogor_mob_gromkash() : CreatureScript("auchindoun_teronogor_mob_gromkash") { }

    struct auchindoun_teronogor_mob_gromkashAI : ScriptedAI
    {
        auchindoun_teronogor_mob_gromkashAI(Creature* creature) : ScriptedAI(creature)
        {
            m_First = true;
            m_Instance = me->GetInstanceScript();
        }

        EventMap events;

        enum eGromkashSpells
        {
            SpellGrimoireOfSacrifice = 159024,
            SpellImmolate = 156964,
            SpellIncinrate = 146963,
            SpellRainOfFire = 65757
        };

        enum eGromkashEvents
        {
            EventImmolate = 1,
            EventIncinrate,
            EventRainOfFire
        };

        InstanceScript* m_Instance;
        bool m_First;

        void Reset() override
        {
            events.Reset();

            //if (me->GetMap())
            //    me->GetMap()->SetObjectVisibility(1000.0f);

            if (m_Instance)
            {
                if (Creature* l_Teronogor = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossTeronogor)))
                    me->CastSpell(l_Teronogor, SpellDrainSoulVisual);
            }
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            me->CastStop();
            me->RemoveAllAuras();
            events.RescheduleEvent(EventImmolate, 8 * IN_MILLISECONDS);
            events.RescheduleEvent(EventIncinrate, urand(10 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
            events.RescheduleEvent(EventRainOfFire, 18 * IN_MILLISECONDS);
            if (Creature* l_Zashoo = me->FindNearestCreature(CreatureZashoo, 20.0f, true))
                me->CastSpell(l_Zashoo, SpellGrimoireOfSacrifice);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (m_Instance)
            {
                if (Creature* Teronogor = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossTeronogor)))
                    Teronogor->RemoveAura(SpellTeronogorShield);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventImmolate:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellImmolate);
                    events.RescheduleEvent(EventImmolate, 10 * IN_MILLISECONDS);
                    break;
                case EventIncinrate:
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellIncinrate);
                    events.RescheduleEvent(EventIncinrate, 6 * IN_MILLISECONDS);
                    break;
                case EventRainOfFire:
                    if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Random, SpellRainOfFire);
                    events.RescheduleEvent(EventRainOfFire, 15 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_teronogor_mob_gromkashAI(creature);
    }
};

/// Abyssal - 77905
class auchindoun_teronogor_mob_abyssal : public CreatureScript
{
public:

    auchindoun_teronogor_mob_abyssal() : CreatureScript("auchindoun_teronogor_mob_abyssal") { }

    struct auchindoun_teronogor_mob_abyssalAI : ScriptedAI
    {
        auchindoun_teronogor_mob_abyssalAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = me->GetInstanceScript();
            m_First = true;
        }

        EventMap events;

        enum eTeronogorAbyssalSpells
        {
            SpellFixate = 173080,
        };

        enum eTeronogorAbyssalEvents
        {
            EventFixate = 1,
        };

        InstanceScript* m_Instance;
        ObjectGuid m_Target;
        bool m_First;

        void Reset() override
        {
            m_Target = ObjectGuid::Empty;
            events.Reset();
            KillAllDelayedEvents();

            if (m_First)
            {
                AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
                {
                    me->SetInCombatWithZone();
                    me->SetDisplayId(InvisibleDisplay);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                });

                m_First = false;
                me->SetDisplayId(InvisibleDisplay);
                me->SetReactState(REACT_PASSIVE);
                me->CastSpell(me, SpellSummonAbyssalMeteor);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            }
        }

        void BeginFixation()
        {
            if (!m_Target)
            {
                if (Unit* l_Random = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    m_Target = l_Random->GetGUID();
                    me->GetMotionMaster()->MoveChase(l_Random);
                    me->AddAura(SpellFixate, l_Random);
                }
            }
        }

        //void OnAddThreat(Unit* victim, float& threat, SpellSchoolMask /*p_SchoolMask*/, SpellInfo const* /*p_ThreatSpell*/) override
        //{
        //    if (m_Target)
        //    {
        //        if (victim->GetGUID() != m_Target)
        //            threat = 0;
        //        return;
        //    }
        //}

        void UpdateAI(uint32 diff) override
        {
            me->SetSpeed(MOVE_RUN, 0.3f, true);

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasUnitState(UNIT_STATE_STUNNED))
                return;

            if (m_Target)
            {
                ///< Burst hardcoded
                if (Player* player = sObjectAccessor->GetPlayer(*me, m_Target))
                {
                    if (!player->HasAura(SpellFixate)) /// Fixated aura
                        me->AddAura(SpellFixate, player);

                    if (!me->isMoving())
                        me->GetMotionMaster()->MoveFollow(player, 0, 0, MOTION_SLOT_ACTIVE);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_teronogor_mob_abyssalAI(creature);
    }
};

class auchindoun_teronogor_mob_spirit : public CreatureScript
{
public:

    auchindoun_teronogor_mob_spirit() : CreatureScript("auchindoun_teronogor_mob_spirit") { }

    struct auchindoun_teronogor_mob_spiritAI : ScriptedAI
    {
        auchindoun_teronogor_mob_spiritAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = me->GetInstanceScript();
        }

        InstanceScript* m_Instance;

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_teronogor_mob_spiritAI(creature);
    }
};

/// Chaos Wave - 157001
class auchindoun_teronogor_spell_chaos_wave : public SpellScriptLoader
{
public:

    auchindoun_teronogor_spell_chaos_wave() : SpellScriptLoader("auchindoun_teronogor_spell_chaos_wave") { }

    class auchindoun_teronogor_spell_chaos_wave_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_teronogor_spell_chaos_wave_SpellScript);

        enum eChaosWaveSpells
        {
            SpellChaosWaveDmg = 157002
        };

        void HandleDummy(SpellEffIndex effectIndex)
        {
            if (!GetCaster() && !GetExplTargetUnit())
                return;

            GetCaster()->CastSpell(GetExplTargetUnit(), eChaosWaveSpells::SpellChaosWaveDmg);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(auchindoun_teronogor_spell_chaos_wave_SpellScript::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_teronogor_spell_chaos_wave_SpellScript();
    }
};

/// Seed of Malevolence - 156921 
class auchindoun_teronogor_spell_seed_of_malevolence : public SpellScriptLoader
{
public:

    auchindoun_teronogor_spell_seed_of_malevolence() : SpellScriptLoader("auchindoun_teronogor_spell_seed_of_malevolence") { }

    class auchindoun_teronogor_spell_seed_of_malevolence_AuraScript : public AuraScript
    {
        PrepareAuraScript(auchindoun_teronogor_spell_seed_of_malevolence_AuraScript);

        enum eSeedOfMalevolanceSpells
        {
            SpellSeedOfMalevolenceDmg = 156924
        };

        void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetTarget())
                return;

            GetTarget()->CastSpell(GetTarget(), eSeedOfMalevolanceSpells::SpellSeedOfMalevolenceDmg);
        }

        void Register() override
        {
            AfterEffectRemove += AuraEffectRemoveFn(auchindoun_teronogor_spell_seed_of_malevolence_AuraScript::OnRemove, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DAMAGE, AuraEffectHandleModes::AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new auchindoun_teronogor_spell_seed_of_malevolence_AuraScript();
    }
};

/// Spell Demon Jump - 157039
class auchindoun_teronogor_spell_demonic_leap_jump : public SpellScriptLoader
{
public:

    auchindoun_teronogor_spell_demonic_leap_jump() : SpellScriptLoader("auchindoun_teronogor_spell_demonic_leap_jump") { }

    class auchindoun_teronogor_spell_demonic_leap_jump_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_teronogor_spell_demonic_leap_jump_SpellScript);

        void Land(SpellEffIndex /*effectIndex*/)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(caster, SpellDemonicLeapAreatriger);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(auchindoun_teronogor_spell_demonic_leap_jump_SpellScript::Land, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_teronogor_spell_demonic_leap_jump_SpellScript();
    }
};

/// Demonic Leap - 148969 
class auchindoun_teronogor_spell_demonic_leap : public SpellScriptLoader
{
public:

    auchindoun_teronogor_spell_demonic_leap() : SpellScriptLoader("auchindoun_teronogor_spell_demonic_leap") { }

    class auchindoun_teronogor_spell_demonic_leap_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_teronogor_spell_demonic_leap_SpellScript);

        enum eDemonicLeapSpells
        {
            SpellDemonicLeapJump = 157039
        };

        void HandleDummy(SpellEffIndex effectIndex)
        {
            if (!GetCaster() && !GetExplTargetUnit())
                return;

            GetCaster()->CastSpell(GetExplTargetUnit(), eDemonicLeapSpells::SpellDemonicLeapJump);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(auchindoun_teronogor_spell_demonic_leap_SpellScript::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_teronogor_spell_demonic_leap_SpellScript();
    }
};

/// Teleportation Event
class auchindoun_soul_transportation_event : public BasicEvent
{
public:

    explicit auchindoun_soul_transportation_event(Unit* unit, int value) : BasicEvent(), m_Obj(unit), m_Modifier(value), m_Event(0)
    {
    }

    bool Execute(uint64 /*p_CurrTime*/, uint32 /*diff*/) override
    {
        if (m_Obj)
        {
            if (InstanceScript* m_Instance = m_Obj->GetInstanceScript())
            {
                switch (m_Modifier)
                {
                    /// First soul transport
                    case 0:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionFirstPlatformFirstMove.GetPositionX(), g_PositionFirstPlatformFirstMove.GetPositionY(), g_PositionFirstPlatformFirstMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 1), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 1:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionFirstPlatormSecondMove.GetPositionX(), g_PositionFirstPlatormSecondMove.GetPositionY(), g_PositionFirstPlatormSecondMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 2), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 2:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionFirstPlatformThirdMove.GetPositionX(), g_PositionFirstPlatformThirdMove.GetPositionY(), g_PositionFirstPlatformThirdMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 100), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                        /// Second soul transport
                    case 4:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionSecondPlatformFirstMove.GetPositionX(), g_PositionSecondPlatformFirstMove.GetPositionY(), g_PositionSecondPlatformFirstMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 5), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 5:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionSecondPlatformSecondMove.GetPositionX(), g_PositionSecondPlatformSecondMove.GetPositionY(), g_PositionSecondPlatformSecondMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 6), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 6:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionSecondPlatformThirdMove.GetPositionX(), g_PositionSecondPlatformThirdMove.GetPositionY(), g_PositionSecondPlatformThirdMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 100), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                        /// Third soul transport
                    case 7:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionThirdPlatformFirstMove.GetPositionX(), g_PositionThirdPlatformFirstMove.GetPositionY(), g_PositionThirdPlatformFirstMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 8), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 8:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionThirdPlatformsSecondMove.GetPositionX(), g_PositionThirdPlatformsSecondMove.GetPositionY(), g_PositionThirdPlatformsSecondMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 9), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 9:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionThirdPlatformThirdMove.GetPositionX(), g_PositionThirdPlatformThirdMove.GetPositionY(), g_PositionThirdPlatformThirdMove.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 100), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                        /// Fourth soul transport and last
                    case 10:
                        m_Obj->GetMotionMaster()->MoveCharge(g_PositionFourthMovement.GetPositionX(), g_PositionFourthMovement.GetPositionY(), g_PositionFourthMovement.GetPositionZ(), 60.0f);
                        m_Obj->m_Events.AddEvent(new auchindoun_soul_transportation_event(m_Obj, 100), m_Obj->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                        break;
                    case 100:
                        m_Obj->RemoveAura(SpellTranscend);
                        m_Obj->RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
                        m_Obj->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        m_Obj->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                        break;
                    default:
                        break;
                }
            }
        }

        return true;
    }

private:
    Unit* m_Obj;
    int m_Modifier;
    int m_Event;
};

/// Soul Transport Object 01 - 231736
class auchindoun_teronogor_gameobject_soul_transporter_01 : public GameObjectScript
{
public:

    auchindoun_teronogor_gameobject_soul_transporter_01() : GameObjectScript("auchindoun_teronogor_gameobject_soul_transporter_01") { }

    bool OnGossipHello(Player* p_Player, GameObject* p_Gobject) override
    {
        if (InstanceScript* m_Instance = p_Gobject->GetInstanceScript())
        {
            if (Creature* m_Teronogor = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossTeronogor)))
            {
                if (Creature* m_Azzakel = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossAzzakael)))
                {
                    if (m_Azzakel->isAlive())
                        return false;

                    if (boss_teronogor::boss_teronogorAI* l_LinkAI = CAST_AI(boss_teronogor::boss_teronogorAI, m_Teronogor->GetAI()))
                    {
                        if (l_LinkAI->m_SoulTransport01)
                        {
                            p_Player->AddAura(SpellTranscend, p_Player);
                            p_Player->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                            p_Player->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                            p_Player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_NPC);
                            p_Player->m_Events.AddEvent(new auchindoun_soul_transportation_event(p_Player, 0), p_Player->m_Events.CalculateTime(1 * IN_MILLISECONDS));
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
};

/// Soul Transport Object 02 - 345366
class auchindoun_teronogor_gameobject_soul_transporter_02 : public GameObjectScript
{
public:

    auchindoun_teronogor_gameobject_soul_transporter_02() : GameObjectScript("auchindoun_teronogor_gameobject_soul_transporter_02") { }

    bool OnGossipHello(Player* p_Player, GameObject* p_Gobject) override
    {
        if (InstanceScript* instance = p_Gobject->GetInstanceScript())
        {
            if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
            {
                if (Creature* l_Durag = instance->instance->GetCreature(instance->GetGuidData(DataDurag)))
                {
                    if (l_Durag->isDead())
                    {
                        if (l_Teronogor->IsAIEnabled)
                        {
                            if (boss_teronogor::boss_teronogorAI* l_LinkAI = CAST_AI(boss_teronogor::boss_teronogorAI, l_Teronogor->GetAI()))
                            {
                                if (l_LinkAI->m_SoulTransport02)
                                {
                                    p_Player->AddAura(SpellTranscend, p_Player);
                                    p_Player->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                                    p_Player->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                                    p_Player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_NPC);
                                    p_Player->m_Events.AddEvent(new auchindoun_soul_transportation_event(p_Player, 4), p_Player->m_Events.CalculateTime(1 * IN_MILLISECONDS));
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }
    /*
    struct auchindoun_teronogor_gameobject_soul_transporter_02AI : public GameObjectAI
    {
        auchindoun_teronogor_gameobject_soul_transporter_02AI(GameObject* p_GameObject) : GameObjectAI(p_GameObject) { }

        void Reset() override
        {
            if (InstanceScript* instance = go->GetInstanceScript())
            {
                if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
                {
                    go->SetLootState(GO_READY);
                    go->UseDoorOrButton(10 * IN_MILLISECONDS, false, l_Teronogor);
                }
            }
        }
    };


    GameObjectAI* GetAI(GameObject* p_GameObject) const override
    {
        return new auchindoun_teronogor_gameobject_soul_transporter_02AI(p_GameObject);
    }
    */
};

/// Soul Transport Object 03 - 345367
class auchindoun_teronogor_gameobject_soul_transporter_03 : public GameObjectScript
{
public:

    auchindoun_teronogor_gameobject_soul_transporter_03() : GameObjectScript("auchindoun_teronogor_gameobject_soul_transporter_03") { }

    bool OnGossipHello(Player* p_Player, GameObject* p_Gobject) override
    {
        if (InstanceScript* instance = p_Gobject->GetInstanceScript())
        {
            if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
            {
                if (l_Teronogor->IsAIEnabled)
                {
                    if (boss_teronogor::boss_teronogorAI* l_LinkAI = CAST_AI(boss_teronogor::boss_teronogorAI, l_Teronogor->GetAI()))
                    {
                        if (l_LinkAI->m_SoulTransport03)
                        {
                            p_Player->AddAura(SpellTranscend, p_Player);
                            p_Player->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                            p_Player->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                            p_Player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_NPC);
                            p_Player->m_Events.AddEvent(new auchindoun_soul_transportation_event(p_Player, 7), p_Player->m_Events.CalculateTime(1 * IN_MILLISECONDS));
                            return true;
                        }
                    }
                }
            }

        }

        return false;
    }
    /*
    struct auchindoun_teronogor_gameobject_soul_transporter_02AI : public GameObjectAI
    {
    auchindoun_teronogor_gameobject_soul_transporter_02AI(GameObject* p_GameObject) : GameObjectAI(p_GameObject) { }

    void Reset() override
    {
    if (InstanceScript* instance = go->GetInstanceScript())
    {
    if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
    {
    go->SetLootState(GO_READY);
    go->UseDoorOrButton(10 * IN_MILLISECONDS, false, l_Teronogor);
    }
    }
    }
    };


    GameObjectAI* GetAI(GameObject* p_GameObject) const override
    {
    return new auchindoun_teronogor_gameobject_soul_transporter_02AI(p_GameObject);
    }
    */
};

/// Soul Transport Object 04 - 345368
class auchindoun_teronogor_gameobject_soul_transporter_04 : public GameObjectScript
{
public:

    auchindoun_teronogor_gameobject_soul_transporter_04() : GameObjectScript("auchindoun_teronogor_gameobject_soul_transporter_04") { }

    bool OnGossipHello(Player* p_Player, GameObject* p_Gobject) override
    {
        if (InstanceScript* instance = p_Gobject->GetInstanceScript())
        {
            if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
            {
                if (l_Teronogor->IsAIEnabled)
                {
                    if (boss_teronogor::boss_teronogorAI* l_LinkAI = CAST_AI(boss_teronogor::boss_teronogorAI, l_Teronogor->GetAI()))
                    {
                        if (l_LinkAI->m_SoulTransport04)
                        {
                            p_Player->AddAura(SpellTranscend, p_Player);
                            p_Player->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                            p_Player->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                            p_Player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_NPC);
                            p_Player->m_Events.AddEvent(new auchindoun_soul_transportation_event(p_Player, 10), p_Player->m_Events.CalculateTime(1 * IN_MILLISECONDS));

                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
    /*
    struct auchindoun_teronogor_gameobject_soul_transporter_02AI : public GameObjectAI
    {
    auchindoun_teronogor_gameobject_soul_transporter_02AI(GameObject* p_GameObject) : GameObjectAI(p_GameObject) { }

    void Reset() override
    {
    if (InstanceScript* instance = go->GetInstanceScript())
    {
    if (Creature* l_Teronogor = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
    {
    go->SetLootState(instance);
    go->UseDoorOrButton(10 * IN_MILLISECONDS, false, l_Teronogor);
    }
    }
    }
    };


    GameObjectAI* GetAI(GameObject* p_GameObject) const override
    {
    return new auchindoun_teronogor_gameobject_soul_transporter_02AI(p_GameObject);
    }
    */
};

void AddSC_boss_teronogor()
{
    new boss_teronogor();                                           ///< 77734
    new auchindoun_teronogor_mob_gromkash();                        ///< 77889
    //new auchindoun_teronogor_mob_abyssal();                         ///< 77905
    new auchindoun_teronogor_mob_durag();                           ///< 77890
    new auchindoun_teronogor_mob_gulkosh();                         ///< 78437
    new auchindoun_teronogor_mob_shaadum();                         ///< 78728
    //new auchindoun_teronogor_mob_spirit();
    new auchindoun_teronogor_spell_chaos_wave();                    ///< 157001
    //new auchindoun_teronogor_spell_demonic_leap();                  ///< 148969
    new auchindoun_teronogor_spell_seed_of_malevolence();           ///< 156921
    new auchindoun_teronogor_spell_demonic_leap_jump();             ///< 157039
    new auchindoun_teronogor_gameobject_soul_transporter_01();      ///< 231736
    //new auchindoun_teronogor_gameobject_soul_transporter_02();      ///< 345366
    //new auchindoun_teronogor_gameobject_soul_transporter_03();      ///< 345367
    //new auchindoun_teronogor_gameobject_soul_transporter_04();      ///< 345368
}