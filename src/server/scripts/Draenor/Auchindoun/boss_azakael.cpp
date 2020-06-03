
////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///  Coded by Davethebrave
////////////////////////////////////////////////////////////////////////////////

#include "ScriptedCreature.h"
#include "auchindoun.hpp"

enum eAzzakelSpells
{
    SpellClawsOfArgusBuff            = 153762,
    SpellClawsOfArgusVisual          = 153764,
    SpellClawsOfArgusDmg             = 153772,
    SpellCurtainOfFlameAura          = 153392,
    SpellCurtainOfFlameForceCast     = 153396,
    SpellCurtainOfFlameVisual        = 153400,
    SpellFelLashVisual               = 153234,
    SpellFelLashDummy                = 174872,
    SpellFelLashDebuff               = 177120,
    SpellFelLashDebuffTwo            = 177121,
    SpellMalevilentCrush             = 153499,
    SpellFelPoolAreatriger           = 153500, 
    SpellFelPoolDebuffDmg            = 153616,
    SpellVisualFelBurst              = 169682,
    SpellFelSparkAreaTrigger         = 153725,
    SpellFelSparkDamage              = 153726,
    SpellFelSparkPerioidicCreation   = 153727,
    SpellSummonImp                   = 153775,
    SpellFelGuard                    = 164080,
    SpellSummonPyromaniac            = 164127,
    SpellFly                         = 161778
};

enum eAzzakelEvents
{
    EventClawsOfArgus = 1,
    EventCurtainOfFlame,
    EventFelLash,
    EventFelPool,
    EventFelSpark = 78,
    EventMalevolentCrush01,
    EventMalevolentCrush02,
    EventSummonAzzakel01,
    EventSummonAzzakel02
};

enum eAzzakelTalks
{
    AzzakelIntro   = 37,  ///< Who Dares Meddlie In The Works Of The Legion?! (46776) 
    AzzakelAggro   = 38,  ///< This World...All World...Shell Burn!(46774)
    AzzakelSpell03 = 39,  ///< Die, Insect!(46781)
    AzzakelSpell02 = 40,  ///< Burn In The Master'S Fire!(46780)
    AzzakelSpell01 = 41,  ///< Come Forth, Servants!(46779)
    AzzakelKill01  = 42,  ///< The Masters Blase Your Soul! (46777)
    AzzakelKill02  = 43,  ///< Burn! (46778)
    AzzakelDeath   = 44   ///< (46775)
};

enum eAzzakelCreatures
{
    TriggerFelPool             = 326526,
    TriggerFelSpark            = 326527,
    TriggerDemonSummoning      = 432636,
    CreatureFelguard           = 76259,
    CreatureCacklingPyromaniac = 76260,
    CreatureBlazingTrickster   = 79511,
    CreatureBlazingTrickster02 = 76220
};

enum eAzzakelActions
{
    ActionFelSpark  = 1,
    ActionSummonDemons,
    ActionRenewEvents,
    ActionBoolActivate,
    ActionBoolDeactivate,
    ActionMalevolentCrash
};

enum eAzzakelMovements
{
    MovementAzzakelMalevolentCrash = 2
};

Position const g_PositionAzzakel_Blackgate = {1929.65f, 2699.27f, 30.799f, 4.428220f};

Position const g_PositionSpawningFlyCoords[2] =
{
    {1912.13f, 2720.44f, 49.818f, 1.600908f},
    {1911.65f, 2757.73f, 30.799f, 4.748000f}
};

Position const g_PositionAzzakelBlackgateLittle[4] =
{
    {1911.90f, 2680.62f, 31.418f, 1.450705f},
    {1911.79f, 2764.35f, 31.418f, 4.721891f},
    {1953.55f, 2722.47f, 31.418f, 3.139304f},
    {1869.70f, 2722.45f, 31.418f, 0.001632f}
};

static void HandleDoors(Unit* p_Me)
{
    std::list<GameObject*> l_ListGameObjects;
    p_Me->GetGameObjectListWithEntryInGrid(l_ListGameObjects, GameobjectFelBarrier, 100.0f);
    if (l_ListGameObjects.empty())
        return;

    for (GameObject* itr : l_ListGameObjects)
        itr->Delete();
}

/// Azzakael Controller - 76216
class auchindoun_azzakel_mob_controller : public CreatureScript
{
public:

    auchindoun_azzakel_mob_controller() : CreatureScript("auchindoun_azzakel_mob_controller") { }

    struct auchindoun_azzakel_mob_controllerAI : Scripted_NoMovementAI
    {
        auchindoun_azzakel_mob_controllerAI(Creature* creature) : Scripted_NoMovementAI(creature), m_Counting(0), m_Summoned(false)
        {
            m_Instance = me->GetInstanceScript();
            m_First = true;
        }

        InstanceScript* m_Instance;
        ObjectGuid m_Azzakel;
        int32 m_Counting;
        bool m_Summoned;
        bool m_First;
        EventMap events;

        void Reset() override
        {
            if (m_First)
            {
                HandleDoors(me);
                m_Counting = 0;
                m_First = false;
                m_Summoned = false;
                me->setFaction(FriendlyFaction);
                me->SetReactState(REACT_PASSIVE);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
            }
        }

        void JustSummoned(Creature* summon) override
        {
            if (summon)
            {
                switch (summon->GetEntry())
                {
                    case CreatureBlazingTrickster:
                    case CreatureCacklingPyromaniac:
                    case CreatureFelguard:
                        summon->SetReactState(REACT_AGGRESSIVE);
                        summon->SetInCombatWithZone();
                        break;
                    default:
                        break;
                }
            }
        }

        void DoAction(int32 const actionID) override
        {
            switch (actionID)
            {
                case ActionCountPre3StBossKill:
                {
                    m_Counting = m_Counting + 1;

                    if (m_Counting > 6 && !m_Summoned)
                    {
                        m_Summoned = true;
                        me->CastSpell(me, SpellVisualFelBurst);
                        me->NearTeleportTo(1911.50f, 2722.52f, 30.799f, g_PositionAzzakel_Blackgate.GetOrientation());
                        events.RescheduleEvent(EventSummonAzzakel01, 2 * IN_MILLISECONDS);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
                case EventSummonAzzakel01:
                {
                    if (Creature* TempAzzakael = me->SummonCreature(BossAzaakel, *me, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        if (GameObject* l_Portal = me->SummonGameObject(GameobjectDemonicPortal, *me, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0))
                        {
                            m_Azzakel = TempAzzakael->GetGUID();
                            l_Portal->SetFlag(11, GO_FLAG_NOT_SELECTABLE | GO_FLAG_NODESPAWN | GO_FLAG_INTERACT_COND);
                            TempAzzakael->GetMotionMaster()->MoveJump(g_PositionSpawningFlyCoords[1].GetPositionX(), g_PositionSpawningFlyCoords[1].GetPositionY(), g_PositionSpawningFlyCoords[1].GetPositionZ(), 30.0f, 25.0f);
                        }
                    }
                    events.RescheduleEvent(EventSummonAzzakel02, 1 * IN_MILLISECONDS);
                    break;
                }
                case EventSummonAzzakel02:
                {
                    if (m_Azzakel)
                    {
                        if (Creature* l_Azzakael = Creature::GetCreature(*me, m_Azzakel))
                        {
                            l_Azzakael->setFaction(HostileFaction);
                            l_Azzakael->GetMotionMaster()->MoveCharge(1911.93f, 2754.40f, 30.973f, 42.0f);
                        }
                        break;
                    }
                }
                default:
                    break;
            }
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_azzakel_mob_controllerAI(creature);
    }
};

/// Fel Spark Trigger - 326527
class auchindoun_azzakel_mob_fel_spark_trigger : public CreatureScript
{
public:

    auchindoun_azzakel_mob_fel_spark_trigger() : CreatureScript("auchindoun_azzakel_mob_fel_spark_trigger") {}

    struct auchindoun_azzakel_mob_fel_spark_triggerAI : ScriptedAI
    {
        auchindoun_azzakel_mob_fel_spark_triggerAI(Creature* creature) : ScriptedAI(creature)
        {
            m_First = true;
        }

        enum eFelSparkSpells
        {
            SpellFelSparkAreaTrigger = 153725
        };

        enum eFelSparkCreatures
        {
            CreatureFelSparkNullAI = 326527,
            CreatureFelSparkNullAITrigger = 326528
        };

        bool m_First;

        void Reset() override
        {
            m_First = false;
            me->setFaction(HostileFaction);
            me->SetDisplayId(InvisibleDisplay);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);

            for (uint8 l_I = 0; l_I < 20; ++l_I)
            {
                float l_X = me->m_positionX + (l_I + 1) * cos(me->m_orientation);
                float l_Y = me->m_positionY + (l_I + 1) * sin(me->m_orientation);

                /// 326528
                if (Creature* l_FelSparkNullAITrigger = me->SummonCreature(CreatureFelSparkNullAITrigger, l_X, l_Y, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 7 * IN_MILLISECONDS))
                {
                    l_FelSparkNullAITrigger->setFaction(HostileFaction);
                    l_FelSparkNullAITrigger->SetDisplayId(InvisibleDisplay);
                    l_FelSparkNullAITrigger->SetReactState(REACT_PASSIVE);
                    l_FelSparkNullAITrigger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_PC);

                    l_FelSparkNullAITrigger->CastSpell(l_FelSparkNullAITrigger, SpellFelSparkAreaTrigger, true);
                }
            }

            me->DespawnOrUnsummon(7 * IN_MILLISECONDS);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_azzakel_mob_fel_spark_triggerAI(creature);
    }
};

/// Azzakael - 75927
class boss_azzakel : public CreatureScript
{
public:

    boss_azzakel() : CreatureScript("boss_azzakel") { }

    struct boss_azzakelAI : BossAI
    {
        boss_azzakelAI(Creature* creature) : BossAI(creature, DataBossAzzakael), m_Interval(0), m_Argus(false), m_Achievement(false)
        {
            m_Instance = me->GetInstanceScript();
            m_Intro = false;
            m_First = true;
        }

        InstanceScript* m_Instance;
        uint32 m_Interval;
        bool m_Intro;
        bool m_Argus;
        bool m_First;
        bool m_Achievement;

        void Reset() override
        {
            _Reset();
            events.Reset();
            m_Argus = false;
            m_Interval = 3 * IN_MILLISECONDS;
            if (m_First)
            {
                m_First = false;
                me->setFaction(FriendlyFaction);
            }

            m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellFelPoolDebuffDmg);
            m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellFelSparkPerioidicCreation);
            me->SetHomePosition(g_PositionSpawningFlyCoords[1].GetPositionX(), g_PositionSpawningFlyCoords[1].GetPositionY(), g_PositionSpawningFlyCoords[1].GetPositionZ(), me->GetOrientation());
            me->GetMotionMaster()->MoveTargetedHome();

            uint32 l_Entries[5] = {TriggerDemonSummoning, CreatureCacklingPyromaniac, CreatureBlazingTrickster, CreatureFelguard, CreatureBlazingTrickster02};
            for (uint32 entry : l_Entries)
                DespawnCreaturesInArea(entry, me);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who && who->IsInWorld() && who->IsPlayer() && me->IsWithinDistInMap(who, 18.0f) && !m_Intro)
            {
                m_Intro = true;
                Talk(AzzakelIntro);
            }
        }

        void DoAction(int32 const actionID) override
        {
            switch (actionID)
            {
                case ActionDemonSoulsAchievement:
                    m_Achievement = false;
                    break;
                case ActionBoolDeactivate:
                    m_Argus = false;
                    me->UpdatePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), true);
                    break;
                case ActionBoolActivate:
                    m_Argus = true;
                    break;
                case ActionFelSpark:
                    events.RescheduleEvent(EventFelSpark, 1 * IN_MILLISECONDS);
                    break;
                case ActionRenewEvents:
                {
                    events.Reset();
                    me->GetMotionMaster()->Clear();
                    me->UpdatePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), false);
                    events.RescheduleEvent(EventFelLash, 8 * IN_MILLISECONDS);
                    events.RescheduleEvent(EventClawsOfArgus, 45 * IN_MILLISECONDS);
                    events.RescheduleEvent(EventCurtainOfFlame, 14 * IN_MILLISECONDS);
                    events.RescheduleEvent(EventMalevolentCrush01, 20 * IN_MILLISECONDS);
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                        AttackStart(l_Target);
                    break;
                }
                case ActionMalevolentCrash:
                    events.RescheduleEvent(EventMalevolentCrush01, 1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void JustReachedHome() override
        {
            _JustReachedHome();
            HandleDoors(me);
            summons.DespawnAll();

            for (uint32 entry : {CreatureCacklingPyromaniac, CreatureBlazingTrickster, CreatureFelguard, CreatureBlazingTrickster02})
                DespawnCreaturesInArea(entry, me);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            m_Achievement = true;
            Talk(AzzakelAggro);

            me->SummonGameObject(GameobjectFelBarrier, 1911.01f, 2722.89f, 30.799f, g_PositionAzzakel_Blackgate.GetOrientation(), 0, 0, 0, 0, 0);
            events.RescheduleEvent(EventFelLash, 8 * IN_MILLISECONDS);
            events.RescheduleEvent(EventClawsOfArgus, 45 * IN_MILLISECONDS);
            events.RescheduleEvent(EventCurtainOfFlame, 14 * IN_MILLISECONDS);
            events.RescheduleEvent(EventMalevolentCrush01, 20 * IN_MILLISECONDS);

            if (m_Instance)
            {
                m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                DoZoneInCombat();
            }
        }

        void KilledUnit(Unit* who) override
        {
            if (who && who->IsPlayer())
            {
                if (roll_chance_i(50))
                    Talk(AzzakelKill01);
                else
                    Talk(AzzakelKill02);
            }
        }

        void MovementInform(uint32 moveType, uint32 id) override
        {
            if (id == MovementAzzakelMalevolentCrash)
            {
                me->SummonCreature(TriggerFelPool, *me, TEMPSUMMON_MANUAL_DESPAWN);
                me->CastSpell(me, SpellFelPoolAreatriger);

                me->SetCanFly(false);
                me->SetDisableGravity(false);
                Talk(AzzakelSpell02);
                me->SetReactState(REACT_AGGRESSIVE);

                if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                {
                    me->GetMotionMaster()->MoveChase(l_Target, 0.0f, 0.0f);
                    me->Attack(l_Target, true);
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            Talk(AzzakelDeath);
            for (uint32 entry : {CreatureCacklingPyromaniac, CreatureBlazingTrickster, CreatureFelguard, CreatureBlazingTrickster02})
                DespawnCreaturesInArea(entry, me);

            if (m_Instance)
            {
                m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellFelPoolDebuffDmg);
                m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellFelSparkPerioidicCreation);
                m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (GameObject* l_Transport = m_Instance->instance->GetGameObject(m_Instance->GetGuidData(DataSoulTransport1)))
                {
                    l_Transport->SetLootState(GO_READY);
                    l_Transport->UseDoorOrButton(10 * IN_MILLISECONDS, false, me);
                }

                if (m_Achievement)
                {
                    if (me->GetMap() && me->GetMap()->IsHeroic())
                        m_Instance->DoCompleteAchievement(AchievementDemonSouls);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (m_Argus)
            {
                if (m_Interval <= diff)
                {
                    if (Creature* l_AzzakelController = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataTriggerAzzakelController)))
                    {
                        switch (urand(0, 3))
                        {
                            case 0:
                                l_AzzakelController->CastSpell(l_AzzakelController, SpellSummonImp);
                                break;
                            case 1:
                                l_AzzakelController->CastSpell(l_AzzakelController, SpellFelGuard);
                                break;
                            case 2:
                                l_AzzakelController->CastSpell(l_AzzakelController, SpellSummonPyromaniac);
                                break;
                            default:
                                break;
                        }
                    }

                    m_Interval = 3 * IN_MILLISECONDS;
                }
                else
                    m_Interval -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventFelLash:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellFelLashVisual);
                    events.RescheduleEvent(EventFelLash, urand(8 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
                    break;
                case EventClawsOfArgus:
                {
                    events.Reset();
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetSpeed(MOVE_FLIGHT, 2.5f, true);
                    Talk(AzzakelSpell01);
                    me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
                    me->CastSpell(me, SpellClawsOfArgusVisual);
                    me->MonsterTextEmote("Azzakel casts |cffff0000[Azzakael casts [Claws of Agrus]|cfffaeb00!", me->GetGUID(), true);
                    events.RescheduleEvent(EventClawsOfArgus, 45 * IN_MILLISECONDS);
                    break;
                }
                case EventCurtainOfFlame:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                    {
                        Talk(AzzakelSpell03);
                        me->AddAura(SpellCurtainOfFlameAura, l_Target);
                        std::string l_Str;
                        l_Str += "Azzakel casts |cffff0000[Curtain of Flame]|cfffaeb00! on ";
                        l_Str += l_Target->GetName();
                        me->MonsterTextEmote(l_Str.c_str(), me->GetGUID(), true);
                    }

                    events.RescheduleEvent(EventCurtainOfFlame, urand(8 * IN_MILLISECONDS, 15 * IN_MILLISECONDS));
                    break;
                }
                case EventMalevolentCrush01:
                {
                    me->StopMoving();
                    me->AttackStop();
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    Talk(AzzakelSpell02);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetSpeed(MOVE_FLIGHT, 2.5f, true);

                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->GetMotionMaster()->MoveJump(l_Target->GetPositionX(), l_Target->GetPositionY(), l_Target->GetPositionZ(), 15.0f, 20.0f, 10.0f, MovementAzzakelMalevolentCrash);

                    events.RescheduleEvent(EventMalevolentCrush01, 20 * IN_MILLISECONDS);
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
        return new boss_azzakelAI(creature);
    }
};

/// Fel Pool - 326526
class auchindoun_azzakel_mob_fel_pool : public CreatureScript
{
public:

    auchindoun_azzakel_mob_fel_pool() : CreatureScript("auchindoun_azzakel_mob_fel_pool") {}

    struct auchindoun_azzakel_mob_fel_poolAI : Scripted_NoMovementAI
    {
        auchindoun_azzakel_mob_fel_poolAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            m_First = true;
            m_Instance = me->GetInstanceScript();
        }

        enum eFelPoolSpells
        {
            SpellFelPoolDebuffDmg = 153616
        };

        bool m_First;
        InstanceScript* m_Instance;

        void Reset() override
        {
            if (m_First)
            {
                m_First = false;
                me->setFaction(HostileFaction);
                me->SetDisplayId(InvisibleDisplay);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }

            if (m_Instance)
            {
                if (m_Instance->instance->IsHeroic())
                {
                    me->SummonCreature(TriggerFelSpark, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 4.756f, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(TriggerFelSpark, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.028f, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(TriggerFelSpark, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 1.583f, TEMPSUMMON_MANUAL_DESPAWN);
                    me->SummonCreature(TriggerFelSpark, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.111f, TEMPSUMMON_MANUAL_DESPAWN);
                }
            }
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            //std::list<Player*> playerList;
            //Trinity::AnyPlayerInObjectRangeCheck check(me, 15.0f);
            //Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, playerList, check);
            //me->VisitNearbyObject(15.0f, searcher);
            //if (!playerList.empty())
            //{
            //    for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            //    {
            //        if (!*itr)
            //            continue;

            //        if ((*itr)->IsWithinDistInMap(me, 5.0f))
            //        {
            //            if (!(*itr)->HasAura(SpellFelPoolDebuffDmg))
            //                me->AddAura(SpellFelPoolDebuffDmg, *itr);
            //        }
            //        else
            //        {
            //            if ((*itr)->HasAura(SpellFelPoolDebuffDmg, me->GetGUID()))
            //                (*itr)->RemoveAura(SpellFelPoolDebuffDmg);
            //        }
            //    }
            //}
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_azzakel_mob_fel_poolAI(creature);
    }
};

/// Curtain of Flames - 153392
class auchindoun_azzakel_spell_curtain_flames : public SpellScriptLoader
{
public:
    auchindoun_azzakel_spell_curtain_flames() : SpellScriptLoader("auchindoun_azzakel_spell_curtain_flames") { }

    class auchindoun_azzakel_spell_curtain_flames_AuraScript : public AuraScript
    {
        PrepareAuraScript(auchindoun_azzakel_spell_curtain_flames_AuraScript);

            enum eCurtainFlamesSpells
        {
            SpellCurtainOfFlameAura = 153392,
            SpellCurtainOfFlameVisual = 153400
        };

        void OnPeriodic(AuraEffect const* /*auraEffect*/)
        {
            if (Unit* l_Target = GetTarget())
            {
                if (!l_Target->IsPlayer())
                    return;

                std::list<Player*> playerList;
                l_Target->GetPlayerListInGrid(playerList, 3.0f);
                if (!playerList.empty())
                {
                    for (Player* itr : playerList)
                    {
                        if (!itr)
                            continue;

                        if (GetTarget()->GetGUID() != itr->GetGUID())
                        {
                            l_Target->AddAura(SpellCurtainOfFlameAura, itr);
                            l_Target->CastSpell(l_Target, SpellCurtainOfFlameVisual);
                        }
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(auchindoun_azzakel_spell_curtain_flames_AuraScript::OnPeriodic, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new auchindoun_azzakel_spell_curtain_flames_AuraScript();
    }
};

/// Claw of Flames Loader - 153764
class auchindoun_azzakel_spell_claws_of_argus : public SpellScriptLoader
{
public:

    auchindoun_azzakel_spell_claws_of_argus() : SpellScriptLoader("auchindoun_azzakel_spell_claws_of_argus") { }

    class auchindoun_azzakel_spell_claws_of_argus_AuraScript : public AuraScript
    {
        PrepareAuraScript(auchindoun_azzakel_spell_claws_of_argus_AuraScript);

        enum eAzzakelEvents
        {
            EventMalevolentCrush01 = 78
        };

        void OnApply(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetCaster())
                return;

            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                if (Creature* l_Azzakel = instance->instance->GetCreature(instance->GetGuidData(DataBossAzzakael)))
                {
                    if (l_Azzakel->IsAIEnabled)
                    {
                        l_Azzakel->SetCanFly(true);
                        l_Azzakel->SetDisableGravity(true);
                        l_Azzakel->GetAI()->DoAction(ActionBoolActivate);
                        l_Azzakel->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        l_Azzakel->GetMotionMaster()->MoveTakeoff(1000, l_Azzakel->GetPositionX(), l_Azzakel->GetPositionY(), 45.0f);
                    }
                }
            }
        }

        void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetCaster())
                return;

            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                if (Creature* l_Azzakel = instance->instance->GetCreature(instance->GetGuidData(DataBossAzzakael)))
                {
                    if (l_Azzakel->IsAIEnabled)
                    {
                        l_Azzakel->RemoveAllAuras();
                        l_Azzakel->SetReactState(REACT_DEFENSIVE);
                        l_Azzakel->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                        l_Azzakel->GetAI()->DoAction(ActionBoolDeactivate);
                        l_Azzakel->GetAI()->DoAction(ActionRenewEvents);

                        l_Azzakel->SetCanFly(false);
                        l_Azzakel->SetDisableGravity(false);
                        l_Azzakel->RemoveAura(SpellFly);
                        l_Azzakel->SetReactState(REACT_AGGRESSIVE);
                        l_Azzakel->SetSpeed(MOVE_FLIGHT, 0.3f, true);

                        if (l_Azzakel->GetMap() && l_Azzakel->GetMap()->IsHeroic())
                            l_Azzakel->GetAI()->DoAction(ActionMalevolentCrash);
                        else
                        {
                            if (Unit* l_Target = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 100.0f, true))
                            {
                                GetCaster()->GetMotionMaster()->MoveChase(l_Target, 0.0f, 0.0f);
                                GetCaster()->Attack(l_Target, true);
                            }
                        }
                    }
                }
            }
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(auchindoun_azzakel_spell_claws_of_argus_AuraScript::OnApply, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DUMMY, AuraEffectHandleModes::AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(auchindoun_azzakel_spell_claws_of_argus_AuraScript::OnRemove, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DUMMY, AuraEffectHandleModes::AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new auchindoun_azzakel_spell_claws_of_argus_AuraScript();
    }
};

void AddSC_boss_azzakel()
{
    new boss_azzakel();                             ///< 75927
    new auchindoun_azzakel_mob_controller();        ///< 76216
    //new auchindoun_azzakel_mob_fel_spark_trigger(); ///< 326527
    //new auchindoun_azzakel_mob_fel_pool();          ///< 326526
    new auchindoun_azzakel_spell_curtain_flames();  ///< 153392
    new auchindoun_azzakel_spell_claws_of_argus();  ///< 153764
}