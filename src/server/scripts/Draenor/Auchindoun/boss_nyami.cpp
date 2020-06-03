
////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "auchindoun.hpp"

enum eNyamiSpells
{
    SpellShadowWordPain             = 154477,
    SpellSoulVesselDummy            = 155327,
    SpellSoulBubbleVisual           = 177549,
    SpellSoulBubbleBuff             = 177550,
    SpellSoulVesselDmg              = 154187,
    SpellSoulVesselAreatrigger      = 153888,
    SpellTornSpritsDummy            = 153994,
    SpellTornSpritsDummyTwo         = 153991,
    SpellTournSpiritsJump           = 153992,
    SpellArbitrerHammer             = 154218,
    SpellRadiantFuryVisualStar      = 157787,
    SpellRadiantFuryDummy           = 154261,
    SpellRadiantFullyVisual         = 154264,
    SpellRadiantDamage              = 154301,
    SpellRadiantFuryJump            = 154262,
    SpellCrusaderStrike             = 176931,
    SpellArcaneBolt                 = 154235,
    SpellVoidChanneling             = 160677,
    SpellStrangulateState           = 78037,
	SpellSpiritVisual				= 145945,
};

enum eNyamiEvents
{
    EventShadowWordPain = 1,
    EventMindSpikeNyami,
    EventSoulVessel,
    EventTornSpirit,
    EventTornSpiritNyamiEffect,
    EventTornSpiritsDummy,
    EventArbitrerHammer,
    EventRadiantFury,
    EventRadiantFurySummonTrigger,
    EventRadiantFuryStop,
    EventArcaneBolt,
    EventCrusaderStrike,
    EventArcaneBombNyami
};

enum eNyamiTalks
{
    NyamiSpell1     = 1,   ///< Your Oath Is Unfinished! (43647)
    NyamiSpell3     = 2,   ///< The Spirits Are Mine To Command! (43649)
    NyamiSpell2     = 3,   ///< Return To This World! (43648)
    NyamiSpell4     = 4,   ///< I Will Drown This World In Shadows!(43650)
    NyamiAggro      = 5,   ///< I Will Coil Your Souls Into Darkness!(43636)
    NyamiSlay       = 6,   ///< Shadows Envelop You! (43646)
    NyamiDeath      = 7,   ///< Too Late...My Master...Comes...(43637) 
    Auchenaiwarden1 = 32,  ///< Champions! Nyami'S Agents - They Went This Way.
    Auchenaiwarden2 = 33,  ///< No...Oh...No...
    Auchenaiwarden3 = 34,  ///< The Barrier Protecting Auchindoun Is Asunder.
    Auchenaiwarden4 = 35,  ///< A Foul Force Has Penetrated These Sacerd Chambers.
    Auchenaiwarden5 = 36   ///< Scount Ahead While We Establish A Foothold.
};

enum eNyamiActions
{
    ActionSummonSpirits = 1,
    ActionBreakLoose,
    ActionReleaseAnimationPreSoulVessel
};

enum eNyamiCreatures
{
    CreatureSpitefulArbitrer            = 76284,
    CreatureTwistedMagus                = 76296,
    CreatureMaleficDefender             = 76283,
    CreatureRadiantFury                 = 432626,
    CreatureSoulVesselHackBubbleEffect  = 342652
};

enum eNyamiMovementInformed
{
    MovementInformedRadiantFury = 1,
    MovementWardenMoveOuttaGate
};

/// Nyami after death event
class EventPostNyamiFight : public BasicEvent
{
public:

    explicit EventPostNyamiFight(Unit* unit, int value) : BasicEvent(), m_Obj(unit), m_Modifier(value), m_Event(0)
    {
    }

    bool Execute(uint64 /*p_CurrTime*/, uint32 /*diff*/) override
    {
        if (m_Obj)
        {
            if (InstanceScript* instance = m_Obj->GetInstanceScript())
            {
                if (Creature* l_Warden = instance->instance->GetCreature(instance->GetGuidData(DataWarden)))
                {
                    if (l_Warden->IsAIEnabled)
                    {
                        switch (m_Modifier)
                        {
                            case 1:
                            {
                                l_Warden->SetCanFly(false);
                                l_Warden->RemoveAllAuras();
                                l_Warden->SetDisableGravity(false);
                                l_Warden->AddAura(SpellKneel, l_Warden);
                                l_Warden->m_Events.AddEvent(new EventPostNyamiFight(l_Warden, 2), l_Warden->m_Events.CalculateTime(6 * IN_MILLISECONDS));
                                break;
                            }
                            case 2:
                            {
                                l_Warden->RemoveAllAuras();
                                l_Warden->AI()->Talk(Auchenaiwarden1);
                                l_Warden->RemoveAura(SpellKneel);
                                l_Warden->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                                if (GameObject* l_Door = l_Warden->FindNearestGameObject(GameobjectHolyWall, 30.0f))
                                    l_Door->Delete();

                                l_Warden->GetMotionMaster()->MovePoint(60, g_PositionWardenPosition1st);
                                l_Warden->m_Events.AddEvent(new EventPostNyamiFight(l_Warden, 3), l_Warden->m_Events.CalculateTime(10 * IN_MILLISECONDS));
                                break;
                            }
                            case 3:
                            {
                                if (instance != nullptr)
                                {
                                    if (GameObject* l_Window = instance->instance->GetGameObject(instance->GetGuidData(DataAuchindounWindow)))
                                    {
                                        l_Window->SetLootState(GO_READY);
                                        l_Window->UseDoorOrButton(10 * IN_MILLISECONDS);
                                    }

                                    l_Warden->SetReactState(REACT_PASSIVE);
                                    l_Warden->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                                    l_Warden->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                                    l_Warden->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN | UNIT_FLAG2_FEIGN_DEATH);
                                    l_Warden->SummonCreature(CreatureFelborneAbyssal, l_Warden->GetPositionX(), l_Warden->GetPositionY(), l_Warden->GetPositionZ(), TEMPSUMMON_MANUAL_DESPAWN);

                                    if (l_Warden->IsAIEnabled)
                                        l_Warden->AI()->Talk(Auchenaiwarden2);
                                }
                                break;
                            }
                            default:
                                break;
                        }
                    }
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

/// Nyami - 76177
class boss_nyami : public CreatureScript
{
public:

    boss_nyami() : CreatureScript("boss_nyami") { }

    struct boss_nyamiAI : public BossAI
    {
        boss_nyamiAI(Creature* creature) : BossAI(creature, DataBossNyami), m_DiffVisual(0), m_DiffChannel(0), m_CanChain(false)
        {
            m_Instance = me->GetInstanceScript();
            m_First = false;
        }

        InstanceScript* m_Instance;
        uint32 m_DiffVisual;
        uint32 m_DiffChannel;
        bool m_CanChain;
        bool m_First;

        void Reset() override
        {
            _Reset();
            events.Reset();
            m_CanChain = false;
            m_DiffVisual = 8 * IN_MILLISECONDS;
            m_DiffChannel = 2 * IN_MILLISECONDS;

            if (m_First)
            {
                m_First = false;
                if (Creature* l_Teronogor = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossTeronogor)))
                    l_Teronogor->SummonCreature(CreatureWardenAzzakael, g_PositionWardenSpawnPoint);
            }
        }

        void JustReachedHome() override
        {
            _JustReachedHome();
            summons.DespawnAll();
            uint32 l_Entries[3] = {CreatureTwistedMagus, CreatureMaleficDefender, CreatureSpitefulArbitrer};
            for (uint32 entry : l_Entries)
                DespawnCreaturesInArea(entry, me);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();

            if (m_Instance)
                m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

            Talk(NyamiAggro);
            events.RescheduleEvent(EventMindSpikeNyami, urand(8 * IN_MILLISECONDS, 15 * IN_MILLISECONDS));
            events.RescheduleEvent(EventShadowWordPain, urand(12 * IN_MILLISECONDS, 18 * IN_MILLISECONDS));
            events.RescheduleEvent(EventSoulVessel, 20 * IN_MILLISECONDS);
            events.RescheduleEvent(EventTornSpirit, 35 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* who) override
        {
            if (who && who->IsPlayer())
                Talk(NyamiSlay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            Talk(NyamiDeath);

            if (m_Instance)
            {
                m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                if (Creature* l_Warden = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataWarden)))
                {
                    l_Warden->GetMotionMaster()->MoveTakeoff(2, l_Warden->GetPositionX(), l_Warden->GetPositionY(), 34.764f);
                    l_Warden->RemoveAura(SpellPrisonAura);
                }
                me->m_Events.AddEvent(new EventPostNyamiFight(me, 1), me->m_Events.CalculateTime(5 * IN_MILLISECONDS));
            }
        }

        void HandleNonCombatVisuals(uint32 const diff)
        {
            /// Non Combat
            if (!UpdateVictim())
            {
                if (m_DiffVisual <= diff)
                {
                    std::list<Creature*> listCorpsesTriggers;
                    me->GetCreatureListWithEntryInGrid(listCorpsesTriggers, CreatureCorpsesNyamiFight, 40.0f);
                    if (!listCorpsesTriggers.empty())
                    {
                        std::list<Creature*>::const_iterator itr = listCorpsesTriggers.begin();
                        std::advance(itr, urand(0, listCorpsesTriggers.size() - 1));

                        if (*itr)
                        {
                            m_CanChain = true;

                            me->RemoveAura(SpellVoidChanneling);
                            me->CastSpell(*itr, SpellTournSpiritsJump);
                            me->GetMotionMaster()->MoveJump((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), 15.0f, 5.0f);

                            Position l_Source(me->m_positionX, me->m_positionY, me->m_positionZ);
                            Position l_Dest(me->m_positionX, me->m_positionY, me->m_positionZ + 5.0f);
                            Position l_Orientation(0.0f, 0.0f, 0.0f);

                            me->PlayOrphanSpellVisual(l_Source, l_Orientation, l_Dest, SpellVisualKitNyamiSpiralUponTornSpirit, 1.0f);
                        }
                    }

                    m_DiffChannel = 2 * IN_MILLISECONDS;
                    m_DiffVisual = 15 * IN_MILLISECONDS;
                }
                else
                    m_DiffVisual -= diff;

                if (m_DiffChannel <= diff && m_CanChain)
                {
                    me->CastSpell(me, SpellVoidChanneling);
                    m_CanChain = false;
                }
                else
                    m_DiffChannel -= diff;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            HandleNonCombatVisuals(diff);

            // Combat
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventMindSpikeNyami:
                    if (Unit * l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellMindSpike);
                    events.RescheduleEvent(EventMindSpikeNyami, urand(6 * IN_MILLISECONDS, 8 * IN_MILLISECONDS));
                    break;
                case EventShadowWordPain:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                        me->CastSpell(l_Target, SpellShadowWordPain);
                    events.RescheduleEvent(EventShadowWordPain, urand(12 * IN_MILLISECONDS, 18 * IN_MILLISECONDS));
                    break;
                case EventSoulVessel:
                    if (m_Instance)
                    {
                        me->CastSpell(me, SpellSoulVesselDummy);
                        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellSoulBubbleBuff);
                        me->MonsterTextEmote("Soulbinder Nyami begins to cast|cffff0000[Soul Vessel]|cfffaeb00!", me->GetGUID(), true);
                        if (Creature* l_Bubble = me->SummonCreature(CreatureSoulVesselHackBubbleEffect, g_PositionBubble, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            if (l_Bubble->IsAIEnabled)
                                l_Bubble->AI()->DoAction(ActionReleaseAnimationPreSoulVessel);
                        }
                    }
                    events.RescheduleEvent(EventSoulVessel, 25 * IN_MILLISECONDS);
                    break;
                case EventTornSpirit:
                {
                    std::list<Creature*> listCorpsesTriggers;
                    me->GetCreatureListWithEntryInGrid(listCorpsesTriggers, CreatureCorpsesNyamiFight, 40.0f);
                    if (!listCorpsesTriggers.empty())
                    {
                        std::list<Creature*>::const_iterator itr = listCorpsesTriggers.begin();
                        std::advance(itr, urand(0, listCorpsesTriggers.size() - 1));
                        if (*itr)
                        {
                            me->SetReactState(REACT_PASSIVE);
                            me->SetSpeed(MOVE_RUN, 20.0f, true);
                            me->CastSpell(*itr, SpellDispersionVisualNyami);
                            me->GetMotionMaster()->MovePoint(0, (*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ());
                        }
                    }
                    events.RescheduleEvent(EventTornSpiritsDummy, 3 * IN_MILLISECONDS);
                    events.RescheduleEvent(EventTornSpirit, 35 * IN_MILLISECONDS);
                    break;
                }
                case EventTornSpiritsDummy:
                    me->SetSpeed(MOVE_RUN, 1.5f, true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->CastSpell(me, SpellTornSpritsDummy);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_nyamiAI(creature);
    }
};

/// Warden - 76572
class auchindoun_nyami_mob_warden_cosmetic : public CreatureScript
{
public:

    auchindoun_nyami_mob_warden_cosmetic() : CreatureScript("auchindoun_nyami_mob_warden_cosmetic") {}

    struct auchindoun_nyami_mob_warden_cosmeticAI : public ScriptedAI
    {
        auchindoun_nyami_mob_warden_cosmeticAI(Creature* creature) : ScriptedAI(creature)
        {
            m_First = false;
        }

        bool m_First;

        void Reset() override
        {
            if (!m_First)
            {
                m_First = true;
                me->AddAura(SpellPrisonAura, me);
            }

            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetReactState(REACT_PASSIVE);
            me->CastSpell(me, SpellStrangulateState);
            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_nyami_mob_warden_cosmeticAI(creature);
    }
};

/// Bubble Creature - 342652
class auchindoun_nyami_mob_bubble : public CreatureScript
{
public:

    auchindoun_nyami_mob_bubble() : CreatureScript("auchindoun_nyami_mob_bubble") {}

    struct auchindoun_nyami_mob_bubbleAI : public Scripted_NoMovementAI
    {
        auchindoun_nyami_mob_bubbleAI(Creature* creature) : Scripted_NoMovementAI(creature), m_SpellDiff(0), m_Visual(false)
        {
        }

        enum eBubbleSpells
        {
            SpellSoulBubbleVisual = 177549,
            SpellSoulBubbleBuff = 177550
        };

        EventMap events;
        uint32 m_SpellDiff;
        bool m_Visual;

        void Reset() override
        {
            events.Reset();
            m_Visual = false;
            me->SetDisplayId(InvisibleDisplay);
            m_SpellDiff = 1 * IN_MILLISECONDS;
            me->CastSpell(me, SpellSoulBubbleVisual);
            me->CastSpell(me, SpellSoulBubbleBuff);
        }

        void UpdateAI(uint32 diff) override
        {
            if (m_Visual)
            {
                if (m_SpellDiff <= diff)
                {
                    //std::list<Player*> playerList;
                    //Trinity::AnyPlayerInObjectRangeCheck check(me, 20.0f);
                    //Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, playerList, check);
                    //me->VisitNearbyObject(12.0f, searcher);
                    //if (!playerList.empty())
                    //{
                    //    for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                    //    {
                    //        if (!*itr)
                    //            continue;

                    //        if ((*itr)->IsWithinDistInMap(me, 4.0f))
                    //        {
                    //            if (!(*itr)->HasAura(SpellSoulBubbleBuff))
                    //                me->AddAura(SpellSoulBubbleBuff, *itr);
                    //        }
                    //        else
                    //        {
                    //            if ((*itr)->HasAura(SpellSoulBubbleBuff))
                    //                (*itr)->RemoveAura(SpellSoulBubbleBuff);
                    //        }
                    //    }
                    //}

                    Position l_Source(me->m_positionX, me->m_positionY, me->m_positionZ);
                    Position l_Dest(1652.273f, 3008.761f, 36.79123f);
                    Position l_Orientation(0.0f, 0.0f, 0.0f);

                    me->PlayOrphanSpellVisual(l_Source, l_Orientation, l_Dest, SpellVisualKitNyamiSoulVesselCircle, 1.0f);

                    m_SpellDiff = 1 * IN_MILLISECONDS;
                }
                else
                    m_SpellDiff -= diff;
            }
        }

        void DoAction(int32 const actionID) override
        {
            switch (actionID)
            {
                case ActionReleaseAnimationPreSoulVessel:
                {
                    me->CancelOrphanSpellVisual(SpellVisualKitNyamiSoulVesselCircle);
                    me->CancelOrphanSpellVisual(SpellVisualKitNyamiSoulVesselSpiralCircle);

                    if (m_Visual)
                        m_Visual = false;
                    else
                        m_Visual = true;
                    break;
                }
                default:
                    break;
            }
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_nyami_mob_bubbleAI(creature);
    }
};

/// Malefic Defender - 76283
class auchindoun_nyami_mob_malefic_defender : public CreatureScript
{
public:

    auchindoun_nyami_mob_malefic_defender() : CreatureScript("auchindoun_nyami_mob_malefic_defender") { }

    struct auchindoun_nyami_mob_malefic_defenderAI : public ScriptedAI
    {
        auchindoun_nyami_mob_malefic_defenderAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        enum eMaleficDefenderSpells
        {
            SpellCrusaderStirke = 176931
        };

        enum eMaleficDefenderEvents
        {
            EventCrusaderStirke = 1
        };

        void Reset() override
        {
            events.Reset();
            me->AddAura(SpellSpiritVisual, me);
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            events.RescheduleEvent(EventCrusaderStirke, 5 * IN_MILLISECONDS);
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
                case EventCrusaderStirke:
                    if (Unit* l_Target = me->getVictim())
                        me->CastSpell(l_Target, SpellCrusaderStirke);
                    events.RescheduleEvent(EventCrusaderStirke, urand(7 * IN_MILLISECONDS, 12 * IN_MILLISECONDS));
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_nyami_mob_malefic_defenderAI(creature);
    }
};

/// Spiteful Arbitrer - 76284
class auchindoun_nyami_mob_spiteful_arbitrer : public CreatureScript
{
public:

    auchindoun_nyami_mob_spiteful_arbitrer() : CreatureScript("auchindoun_nyami_mob_spiteful_arbitrer") { }

    struct auchindoun_nyami_mob_spiteful_arbitrerAI : public ScriptedAI
    {
        auchindoun_nyami_mob_spiteful_arbitrerAI(Creature* creature) : ScriptedAI(creature), m_RadiantDiff(0), m_Radiant(false)
        {
            m_Instance = me->GetInstanceScript();
        }

        enum eArbitrerSpells
        {
            SpellRadiantFuryAreatrigger = 157787
        };

        EventMap events;
        InstanceScript* m_Instance;
        uint32 m_RadiantDiff;
        bool m_Radiant;

        void Reset() override
        {
            events.Reset();
            m_Radiant = false;
            me->AddAura(SpellSpiritVisual, me);
            m_RadiantDiff = 1 * IN_MILLISECONDS;
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            events.RescheduleEvent(EventRadiantFury, 8 * IN_MILLISECONDS);
            events.RescheduleEvent(EventArbitrerHammer, 14 * IN_MILLISECONDS);
        }

        void MovementInform(uint32 /*moveType*/, uint32 id) override
        {
            switch (id)
            {
                case MovementInformedRadiantFury:
                    m_Radiant = false;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (m_Radiant)
            {
                if (m_RadiantDiff <= diff)
                {
                    me->CastSpell(me, SpellRadiantFuryAreatrigger, true);

                    m_RadiantDiff = 200;
                }
                else
                    m_RadiantDiff -= diff;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventArbitrerHammer:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Target, SpellArbitrerHammer);
                    events.RescheduleEvent(EventArbitrerHammer, 14 * IN_MILLISECONDS);
                    break;
                case EventRadiantFury:
                {
                    m_Radiant = true;
                    m_RadiantDiff = 200;

                    float l_X = me->GetPositionX() + 30 * cos(me->m_orientation);
                    float l_Y = me->GetPositionY() + 30 * sin(me->m_orientation);
                    me->GetMotionMaster()->MoveJump(l_X, l_Y, me->GetPositionZ(), 20.0f, 7.0f, 10.0f);

                    events.RescheduleEvent(EventRadiantFuryStop, 6 * IN_MILLISECONDS);
                    break;
                }
                case EventRadiantFuryStop:
                {
                    m_Radiant = false;
                    events.CancelEvent(EventRadiantFurySummonTrigger);
                    events.RescheduleEvent(EventRadiantFury, urand(15 * IN_MILLISECONDS, 20 * IN_MILLISECONDS));
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
        return new auchindoun_nyami_mob_spiteful_arbitrerAI(creature);
    }
};

/// Twisted Magus - 76296
class auchindoun_nyami_mob_twisted_magus : public CreatureScript
{
public:

    auchindoun_nyami_mob_twisted_magus() : CreatureScript("auchindoun_nyami_mob_twisted_magus") { }

    struct auchindoun_nyami_mob_twisted_magusAI : public ScriptedAI
    {
        auchindoun_nyami_mob_twisted_magusAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = me->GetInstanceScript();
        }

        enum eTwistedMagusSpells
        {
            SpellArcaneBolt = 153235,
            SpellArcaneBombDummy = 157652,
        };

        enum eTwistedMagusEvents
        {
            EventArcaneBolt = 1,
            EventArcaneBomb
        };

        EventMap events;
        InstanceScript* m_Instance;

        void Reset() override
        {
            events.Reset();
            me->AddAura(SpellSpiritVisual, me);
        }

        void EnterCombat(Unit* /*atacker*/) override
        {
            events.RescheduleEvent(EventArcaneBolt, 4 * IN_MILLISECONDS);
            events.RescheduleEvent(EventArcaneBomb, 12 * IN_MILLISECONDS);
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
                case EventArcaneBolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(l_Target, SpellArcaneBolt);
                    events.RescheduleEvent(EventArcaneBolt, 15 * IN_MILLISECONDS);
                    break;
                case EventArcaneBomb:
                    me->CastSpell(me, SpellArcaneBombDummy);
                    events.RescheduleEvent(EventArcaneBomb, 25 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_nyami_mob_twisted_magusAI(creature);
    }
};

/// Torn Spirits - 153994
class auchindoun_nyami_spell_torn_spirits : public SpellScriptLoader
{
public:

    auchindoun_nyami_spell_torn_spirits() : SpellScriptLoader("auchindoun_nyami_spell_torn_spirits") { }

    class auchindoun_nyami_spell_torn_spirits_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_nyami_spell_torn_spirits_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            uint32 l_Entries[3] = {CreatureMaleficDefender, CreatureTwistedMagus, CreatureSpitefulArbitrer};

            if (Unit* caster = GetCaster())
            {
                if (Creature* l_Trigger = GetCaster()->FindNearestCreature(CreatureCorpsesNyamiFight, 100.0f, true))
                {
                    Position position;
                    l_Trigger->GetRandomNearPosition(position, 4.0f);

                    for (uint32 entry : l_Entries)
                        caster->SummonCreature(entry, position, TEMPSUMMON_MANUAL_DESPAWN);
                }
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(auchindoun_nyami_spell_torn_spirits_SpellScript::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_nyami_spell_torn_spirits_SpellScript();
    }
};

/// Soul Vessel - 154187
class auchindoun_nyami_spell_soul_vessel : public SpellScriptLoader
{
public:

    auchindoun_nyami_spell_soul_vessel() : SpellScriptLoader("auchindoun_nyami_spell_soul_vessel") { }

    class auchindoun_nyami_spell_soul_vessel_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_nyami_spell_soul_vessel_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effectIndex*/)
        {
            if (GetHitUnit() && GetHitUnit()->HasAura(SpellSoulBubbleBuff))
                SetHitDamage(0);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(auchindoun_nyami_spell_soul_vessel_SpellScript::RecalculateDamage, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_nyami_spell_soul_vessel_SpellScript();
    }
};

/// Soul Vessel - 155327
class auchindoun_nyami_spell_soul_vessel_dummy : public SpellScriptLoader
{
public:

    auchindoun_nyami_spell_soul_vessel_dummy() : SpellScriptLoader("auchindoun_nyami_spell_soul_vessel_dummy") { }

    class auchindoun_nyami_spell_soul_vessel_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(auchindoun_nyami_spell_soul_vessel_dummy_AuraScript);

        void HandlePeriodic(AuraEffect const* /*auraEffect*/)
        {
            if (Unit* caster = GetCaster())
            {
                std::list<Player*> playerList;
                caster->GetPlayerListInGrid(playerList, 200.0f);
                if (!playerList.empty())
                {
                    for (Player* itr : playerList)
                    {
                        if (itr && itr->IsInWorld())
                            itr->CastSpell(itr, SpellSoulVesselDmg);
                    }
                }

                /// Cosmetic
                if (Creature* l_Bubble = caster->FindNearestCreature(CreatureSoulVesselHackBubbleEffect, 150.0f))
                {
                    Position l_Source(l_Bubble->m_positionX, l_Bubble->m_positionY, l_Bubble->m_positionZ);
                    Position l_Dest(1759.477f, 2947.181f, 36.79123f);
                    Position l_Orientation(0.0f, 0.0f, 3.717551f);

                    caster->PlayOrphanSpellVisual(l_Source, l_Orientation, l_Dest, SpellVisualKitNyamiSoulVesselSpiralCircle, 1.0f);
                }
            }
        }

        void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                /// Cosmetic
                if (Creature* l_Bubble = caster->FindNearestCreature(CreatureSoulVesselHackBubbleEffect, 150.0f))
                {
                    if (l_Bubble->IsAIEnabled)
                        l_Bubble->AI()->DoAction(ActionReleaseAnimationPreSoulVessel);
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(auchindoun_nyami_spell_soul_vessel_dummy_AuraScript::HandlePeriodic, SpellEffIndex::EFFECT_1, AuraType::SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            AfterEffectRemove += AuraEffectRemoveFn(auchindoun_nyami_spell_soul_vessel_dummy_AuraScript::OnRemove, SpellEffIndex::EFFECT_1, AuraType::SPELL_AURA_PERIODIC_TRIGGER_SPELL, AuraEffectHandleModes::AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new auchindoun_nyami_spell_soul_vessel_dummy_AuraScript();
    }
};

void AddSC_boss_nyami()
{
    new boss_nyami();                                   ///< 76177
    new auchindoun_nyami_mob_malefic_defender();        ///< 76283
    new auchindoun_nyami_mob_spiteful_arbitrer();       ///< 76284
    new auchindoun_nyami_mob_twisted_magus();           ///< 76296
    new auchindoun_nyami_mob_warden_cosmetic();         ///< 76572
    //new auchindoun_nyami_mob_bubble();                  ///< 342652
    new auchindoun_nyami_spell_soul_vessel();           ///< 153994
    new auchindoun_nyami_spell_torn_spirits();          ///< 154187
    new auchindoun_nyami_spell_soul_vessel_dummy();     ///< 155327
}
