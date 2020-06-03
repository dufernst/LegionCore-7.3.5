
////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///  Coded by Davethebrave
////////////////////////////////////////////////////////////////////////////////

#include "GridNotifiers.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "auchindoun.hpp"

enum eKaatharSpells
{
    SpellHallowedGround                = 154526,
    SpellHallowedGroundsTriggerMissile = 155646,
    SpellConsecratedLight              = 153006,
    SpellConsecratedLightDamage        = 156746,
    SpellHolyShieldThrow               = 153002,
    SpellHolyShieldLos                 = 153028,
    SpellHolyShieldLosSpells           = 153452,
    SpellHolyShieldAreatrigger         = 153478,
    SpellHolyShieldOffHandDisarm       = 174205,
    SpellHolyShieldTwoHandDisarm       = 174206,
    SpellHolyShieldKnockBack           = 153481,
    SpellHolyShieldDamageFromKnockBack = 153480,
    SpellHolyShieldAreaTrigger         = 153478,
    SpellSanctifiedStrikeDummy         = 152954,
    SpellSanctifiedGroundTickDamage    = 161457,
    SpellSanctifiedGroundAura          = 153430,
    SpellSanctifiedGroundDamageTick    = 161457,
    SpellSanctifiedStrikeAreaTrigger   = 165064,
    SpellSanctifiedStrikeAreaTrigger2  = 163559,
    SpellSancitfiedStrikeAreaTrigger3  = 165055,
    SpellSanctifiedStrikeAreaTrigger4  = 165065,
    SpellFate                          = 157465
};

enum eKaatharEvents
{
    EventHallowedGround = 1,
    EventHolyShield,
    EventConsecratedLight,
    EventSanctifiedStrike,
    EventHolyShieldReturn,
    EventFate,
    EventCheckPlayer,
    EventDecreaseSize
};

enum eKaatharActions
{
    ActionActivateBoss = 1,
    ActionFateHallowedGround,
    ActionDespawnCreatures
};

enum eKaatharTalks
{
    VigilantKaatherIntro  = 18,   ///< None Live who Assault The Holy Auchenai. (46436)
    VigilantKaatherAgro   = 19,   ///< I Will Strike You Down. (46434)
    VigilantKaatherSpell1 = 20,   ///< Bathe In The Glory Of The Light! (46438)
    VigilantKaatherSpell2 = 21,   ///< The Light Guide My Hand! (46439)
    VigilantKaatherKill   = 22,   ///< Light Guide You. (46434)
    VigilantKaatherDeath  = 23    ///< Auchindoun...Is...Doomed...(46435)
};

enum eKaatharCreatures
{
    TriggerHallowedGround  = 537324,
    TriggerHolyShield      = 76071,
    TriggerFissureSummoner = 543536
};

Position const g_KaatharNewHomePosition = {1911.47f, 3152.13f, 30.972f, 1.166194f};


/// 1st Starting Event
class EventKaatharDespawnCreatures : public BasicEvent
{
public:

    explicit EventKaatharDespawnCreatures(Unit* unit, int value) : BasicEvent(), m_InstanceScript(nullptr), m_Obj(unit), m_Modifier(value), m_Event(0)
    {
    }

    bool Execute(uint64 /*p_CurrTime*/, uint32 /*p_Diff*/) override
    {
        if (m_Obj)
        {
            if (auto instance = m_Obj->GetInstanceScript())
            {
                if (auto l_Kaathar = instance->instance->GetCreature(instance->GetGuidData(DataBossKathaar)))
                {
                    if (l_Kaathar->IsAIEnabled)
                        l_Kaathar->GetAI()->DoAction(ActionDespawnCreatures);
                }
            }
        }

        return true;
    }

private:
    InstanceScript* m_InstanceScript;
    Unit* m_Obj;
    int m_Modifier;
    int m_Event;
};

class EventNyamiEscape : public BasicEvent
{
public:

    explicit EventNyamiEscape(Unit* unit, int value) : BasicEvent(), m_Obj(unit), m_Modifier(value), m_Event(0)
    {
    }

    bool Execute(uint64 /*p_CurrTime*/, uint32 /*p_Diff*/) override
    {
        if (m_Obj)
        {
            if (InstanceScript* instance = m_Obj->GetInstanceScript())
            {
                if (Creature* l_Tuulani = instance->instance->GetCreature(instance->GetGuidData(DataTuulani)))
                {
                    if (Creature* l_Nyami = instance->instance->GetCreature(instance->GetGuidData(DataNyami)))
                    {
                        if (Creature* l_Kaathar = instance->instance->GetCreature(instance->GetGuidData(DataBossKathaar)))
                        {
                            if (l_Tuulani->IsAIEnabled && l_Nyami->IsAIEnabled && l_Kaathar->IsAIEnabled)
                            {
                                switch (m_Modifier)
                                {
                                    case 0:
                                    {
                                        l_Nyami->AI()->Talk(NYAMITALK6);
                                        l_Nyami->CastSpell(l_Nyami, SpellNyamiExplodeCrystal);
                                        if (GameObject* l_Crystal = instance->instance->GetGameObject(instance->GetGuidData(DataCrystal)))
                                            l_Crystal->Delete();

                                        if (Unit* l_Caster = l_Nyami->FindNearestCreature(CreatureLeftCrystalTrigger, 1000.0f))
                                            l_Nyami->m_Events.AddEvent(new EventNyamiEscape(l_Nyami, 1), l_Caster->m_Events.CalculateTime(3 * IN_MILLISECONDS));
                                        break;
                                    }
                                    case 1:
                                    {
                                        l_Nyami->AI()->Talk(NYAMITALK7);

                                        /// Cosmetic crystal projectiles flies toward the middle
                                        if (Creature* l_Teronoger = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
                                        {
                                            if (Unit* l_Caster = l_Nyami->FindNearestCreature(CreatureLeftCrystalTrigger, 1000.0f))
                                            {
                                                for (uint8 l_I = 0; l_I < 20; l_I++)
                                                {
                                                    Position l_Source(1911.741f, 3183.639f, 56.50413f);
                                                    Position l_Dest(1911.731f, 3183.639f, 56.50413f);
                                                    Position l_Orientation(0.0f, 0.0f, 0.0f);

                                                    l_Caster->PlayOrphanSpellVisual(l_Source, l_Orientation, l_Dest, SpellVisualKitBlackOrbFallingDownInSpiral, 8.0f);
                                                }

                                                l_Caster->m_Events.AddEvent(new EventNyamiEscape(l_Caster, 2), l_Caster->m_Events.CalculateTime(6 * IN_MILLISECONDS));
                                            }
                                        }

                                        break;
                                    }
                                    case 2:
                                    {
                                        /// Cosmetic crystal projectiles flies toward the middle
                                        if (Creature* l_Teronoger = instance->instance->GetCreature(instance->GetGuidData(DataBossTeronogor)))
                                        {
                                            if (Unit* l_Caster = l_Nyami->FindNearestCreature(CreatureLeftCrystalTrigger, 1000.0f, true))
                                            {
                                                for (uint8 l_I = 0; l_I < 20; l_I++)
                                                {
                                                    Position l_Source(l_Caster->GetPositionX(), l_Caster->GetPositionY(), l_Caster->GetPositionZ());
                                                    Position l_Dest(l_Teronoger->m_positionX, l_Teronoger->m_positionY, 59.996f);
                                                    Position l_Orientation(0.0f, 0.0f, 0.0f);

                                                    l_Caster->PlayOrphanSpellVisual(l_Source, l_Orientation, l_Dest, SpellVisualKitBlackOrbFallingDownInSpiral, 8.0f);
                                                }

                                                l_Caster->m_Events.AddEvent(new EventNyamiEscape(l_Caster, 50), l_Tuulani->m_Events.CalculateTime(4 * IN_MILLISECONDS));
                                            }
                                        }

                                        break;
                                    }
                                    case 50:
                                        l_Nyami->AI()->Talk(NYAMITALK8);
                                        l_Nyami->m_Events.AddEvent(new EventNyamiEscape(l_Nyami, 3), l_Tuulani->m_Events.CalculateTime(5 * IN_MILLISECONDS));
                                        break;
                                    case 3:
                                    {
                                        l_Nyami->GetMotionMaster()->MovePoint(MovementInformNyamiEscape06, g_PositionNyamiEscapeMovement[1]);
                                        l_Tuulani->m_Events.AddEvent(new EventNyamiEscape(l_Tuulani, 4), l_Tuulani->m_Events.CalculateTime(5 * IN_MILLISECONDS));
                                        break;
                                    }
                                    case 4:
                                    {
                                        l_Kaathar->setFaction(HostileFaction);
                                        l_Nyami->SetSpeed(UnitMoveType::MOVE_RUN, 10.0f, true);
                                        l_Nyami->RemoveAura(SpellTuulaniCapturedVoidPrison);
                                        l_Nyami->AddAura(SpellDispersionVisualNyami, l_Nyami);
                                        l_Nyami->CastSpell(l_Nyami, SpellCrystalEarthquake);
                                        l_Nyami->GetMotionMaster()->MovePoint(MovementInformNyamiEscape07, 1765.926f, 3178.208f, 57.904f, true);
                                        l_Kaathar->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                                        l_Kaathar->GetMotionMaster()->MoveJump(g_PositionKaatharCombatJump.GetPositionX(), g_PositionKaatharCombatJump.GetPositionY(), g_PositionKaatharCombatJump.GetPositionZ(), 10.0f, 10.0f, 10.0f, 0);
                                        l_Tuulani->m_Events.AddEvent(new EventNyamiEscape(l_Tuulani, 7), l_Tuulani->m_Events.CalculateTime(4 * IN_MILLISECONDS));
                                        break;
                                    }
                                    case 7:
                                    {
                                        l_Nyami->DespawnOrUnsummon(5 * IN_MILLISECONDS);
                                        l_Tuulani->AI()->Talk(TUULANITALK10);
                                        l_Tuulani->m_Events.AddEvent(new EventNyamiEscape(l_Tuulani, 8), l_Tuulani->m_Events.CalculateTime(5 * IN_MILLISECONDS));
                                        break;
                                    }
                                    case 8:
                                    {
                                        l_Tuulani->AI()->Talk(TUULANITALK11);
                                        l_Tuulani->m_Events.AddEvent(new EventNyamiEscape(l_Tuulani, 9), l_Tuulani->m_Events.CalculateTime(5 * IN_MILLISECONDS));
                                        break;
                                    }
                                    case 9:
                                    {
                                        l_Tuulani->AI()->Talk(TUULANITALK9);
                                        l_Tuulani->AddAura(SpellTuulaniCapturedVoidPrison, l_Tuulani);
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

        return true;
    }

private:
    Unit* m_Obj;
    int m_Modifier;
    int m_Event;
};


class EventPostKaathar : public BasicEvent
{
public:
    explicit EventPostKaathar(Unit* unit, int value) : BasicEvent(), m_Obj(unit), m_Modifier(value), m_Event(0)
    {
    }

    bool Execute(uint64 /*p_CurrTime*/, uint32 /*p_Diff*/) override
    {
        if (m_Obj)
        {
            if (auto instance = m_Obj->GetInstanceScript())
            {
                if (auto l_Tuulani = instance->instance->GetCreature(instance->GetGuidData(DataTuulani)))
                {
                    if (l_Tuulani->IsAIEnabled)
                    {
                        switch (m_Modifier)
                        {
                            case 0:
                            {
                                l_Tuulani->AI()->Talk(TUULANITALK14);
                                l_Tuulani->RemoveAura(SpellTuulaniCapturedVoidPrison);

                                for (int8 l_I = 0; l_I < 4; l_I++)
                                {
                                    if (Creature* l_Defender = m_Obj->SummonCreature(CreatureAucheniDefender, g_PositionFourMagesThatSpawnAfterKaatharIsKaaput[l_I], TEMPSUMMON_DEAD_DESPAWN))
                                    {
                                        l_Defender->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 505);
                                        l_Defender->GetMotionMaster()->MovePoint(0, g_PositionFourMagesThatSpawnAfterKaatharIsKaaput[l_I]);
                                    }
                                }

                                if (Creature* l_Nyami = instance->instance->GetCreature(instance->GetGuidData(DataBossNyami)))
                                    l_Nyami->SetPhaseMask(1, true);

                                if (Creature* l_Magus = m_Obj->SummonCreature(CreatureAucheniMagus, g_PositionMageSpawning, TEMPSUMMON_DEAD_DESPAWN))
                                {
                                    l_Magus->GetMotionMaster()->MovePoint(0, g_PositionMageMoveTo);
                                    l_Magus->m_Events.AddEvent(new EventPostKaathar(l_Magus, 1), l_Magus->m_Events.CalculateTime(7 * IN_MILLISECONDS));
                                    l_Magus->m_Events.AddEvent(new EventPostKaathar(l_Magus, 2), l_Magus->m_Events.CalculateTime(55 * IN_MILLISECONDS));
                                }
                                break;
                            }
                            case 1:
                            {
                                l_Tuulani->AI()->Talk(TUULANITALK15);

                                /// Holy Wall, Object In MIddle
                                l_Tuulani->SummonGameObject(GameobjectHolyWall, g_PositionWallInMiddleFromNyami.GetPositionX(), g_PositionWallInMiddleFromNyami.GetPositionY(), g_PositionWallInMiddleFromNyami.GetPositionZ(), g_PositionWallInMiddleFromNyami.GetOrientation(), 0, 0, 0, 0, 0);
                                /// Holy Wall, Object Behind
                                l_Tuulani->SummonGameObject(GameobjectHolyWall, g_PositionWallInBackFromNyami.GetPositionX(), g_PositionWallInBackFromNyami.GetPositionY(), g_PositionWallInBackFromNyami.GetPositionZ(), g_PositionWallInBackFromNyami.GetOrientation(), 0, 0, 0, 0, 0);

                                /// Three prisonners
                                for (int8 l_I = 0; l_I < 3; l_I++)
                                {
                                    if (Creature* l_Prisoners = l_Tuulani->SummonCreature(CreatureAucheniSoulPriest, g_PositionThreePrisoners[l_I], TEMPSUMMON_DEAD_DESPAWN))
                                    {
                                        l_Prisoners->SetCanFly(true);
                                        l_Prisoners->SetDisableGravity(true);
                                        l_Prisoners->SetReactState(REACT_PASSIVE);
                                        l_Prisoners->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                                        l_Prisoners->CastSpell(l_Prisoners, SpellPrisonAura, true);
                                        l_Prisoners->CastSpell(l_Prisoners, SpellStrangulate, true);
                                        l_Prisoners->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                                        l_Prisoners->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                                    }
                                }

                                /// Twelve prisoners (cosmetic)
                                for (int8 l_I = 0; l_I < 11; l_I++)
                                {
                                    if (Creature* l_Prisoners = l_Tuulani->SummonCreature(CreatureAucheniDefender, g_PositionCorpsesNearNyomi[l_I], TEMPSUMMON_DEAD_DESPAWN))
                                    {
                                        l_Prisoners->SetReactState(REACT_PASSIVE);
                                        l_Prisoners->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                                        l_Prisoners->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                                        l_Prisoners->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN | UNIT_FLAG2_FEIGN_DEATH);
                                    }
                                }

                                /// Defenders
                                for (int8 l_I = 0; l_I < 4; l_I++)
                                {
                                    if (Creature* l_Defenders = l_Tuulani->SummonCreature(CreatureAucheniWarden, g_PositionDefenderBehindMiddleWallOfNyami[l_I], TEMPSUMMON_DEAD_DESPAWN))
                                    {
                                        l_Defenders->SetReactState(REACT_PASSIVE);
                                        l_Defenders->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                                        if (Creature* l_Stalker = l_Defenders->FindNearestCreature(CreatureLightWallTargets, 8.0f))
                                            l_Defenders->CastSpell(l_Stalker, SpellHolyBeam);
                                    }
                                }

                                /// Cosmetic Wardens
                                for (int8 l_I = 0; l_I < 2; l_I++)
                                {
                                    if (Creature* l_Defenders = l_Tuulani->SummonCreature(CreatureAucheniWarden, g_PositionDefenderBehindBackWallOfNyami[l_I], TEMPSUMMON_DEAD_DESPAWN))
                                    {
                                        l_Defenders->SetReactState(REACT_PASSIVE);
                                        l_Defenders->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                                        if (Creature* l_Stalker = l_Defenders->FindNearestCreature(CreatureLightWallTargets, 15.0f))
                                            l_Defenders->CastSpell(l_Stalker, SpellHolyBeam);
                                    }
                                }

                                l_Tuulani->SummonCreature(BossNyami, g_PositionNyamiSpawn, TEMPSUMMON_MANUAL_DESPAWN);

                                // Corpses		                          
                                for (int8 l_I = 0; l_I < 2; l_I++)                              /// Holy Wall, Object In MIddle
                                    l_Tuulani->SummonGameObject(GameobjectHolyWall, g_PositionWallInMiddleFromNyami.GetPositionX(), g_PositionWallInMiddleFromNyami.GetPositionY(), g_PositionWallInMiddleFromNyami.GetPositionZ(), g_PositionWallInMiddleFromNyami.GetOrientation(), 0, 0, 0, 0, 0);

                                l_Tuulani->SummonCreature(CreatureSargereiDefender, g_PositionMagusAndDefenderHostile[1], TEMPSUMMON_DEAD_DESPAWN);

                                // Hostile near Two corpses          		
                                l_Tuulani->SummonCreature(CreatureSargereiMagus, g_PositionMagusAndDefenderHostile[0], TEMPSUMMON_DEAD_DESPAWN);

                                // Two defender		
                                for (int8 l_I = 0; l_I < 2; l_I++)
                                {
                                    l_Tuulani->SummonCreature(CreatureSargereiDefender, g_PositionSargereiDefenders[l_I], TEMPSUMMON_DEAD_DESPAWN);
                                }

                                // Magus who control footmans                                  		
                                l_Tuulani->SummonCreature(CreatureSargereiMagus, g_PositionMagusp_WhoControlFootmans, TEMPSUMMON_DEAD_DESPAWN);

                                // Twelve prisoners (cosmetic)		
                                for (int8 l_I = 0; l_I < 11; l_I++)
                                {
                                    if (Creature* l_Prisoner = l_Tuulani->SummonCreature(CreatureAucheniDefender, g_PositionCorpsesNearNyomi[l_I], TEMPSUMMON_DEAD_DESPAWN))
                                    {
                                        l_Prisoner->SetReactState(REACT_PASSIVE);
                                        l_Prisoner->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
                                        l_Prisoner->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN | UNIT_FLAG2_FEIGN_DEATH);
                                    }
                                }

                                l_Tuulani->SummonCreature(CreatureSpitefulArbitrerTrash, g_PositionThreeHostileArbitrerMagusSoulPriest[0], TEMPSUMMON_DEAD_DESPAWN);
                                l_Tuulani->SummonCreature(CreatureSargereiMagus, g_PositionThreeHostileArbitrerMagusSoulPriest[1], TEMPSUMMON_DEAD_DESPAWN);
                                l_Tuulani->SummonCreature(CreatureSargeriSoulPriest, g_PositionThreeHostileArbitrerMagusSoulPriest[2], TEMPSUMMON_DEAD_DESPAWN);
                                l_Tuulani->SummonCreature(CreatureSargeriWarden, g_PositionWardenAndGuards, TEMPSUMMON_DEAD_DESPAWN);

                                for (int8 l_I = 0; l_I < 2; l_I++)
                                {
                                    l_Tuulani->SummonCreature(CreatureSargereiDefender, g_PositionGuardsAndWardens[l_I], TEMPSUMMON_DEAD_DESPAWN);
                                }

                                m_Obj->CastSpell(m_Obj, SpellArcaneChanneling);
                                m_Obj->SummonGameObject(GameobjectTaladorPortal, g_PositionTuulaniGobjectPortalSpawn.GetPositionX(), g_PositionTuulaniGobjectPortalSpawn.GetPositionY(), g_PositionTuulaniGobjectPortalSpawn.GetPositionZ(), g_PositionTuulaniGobjectPortalSpawn.GetOrientation(), 0, 0, 0, 0, 0);
                                break;
                            }
                            case 2:
                            {
                                uint32 l_CreaturesTeronogorPhaseIn[7] = {CreatureZipteq, CreatureZashoo, CreatureShaadum, CreatureGromtashTheDestructor, CreatureGulkosh, CreatureDurem, BossTeronogor};

                                std::list<Creature*> l_CreaturesTeronogorPhaseInList;

                                for (uint8 l_I = 0; l_I < 7; l_I++)
                                    l_Tuulani->GetCreatureListWithEntryInGrid(l_CreaturesTeronogorPhaseInList, l_CreaturesTeronogorPhaseIn[l_I], 700.0f);

                                if (!l_CreaturesTeronogorPhaseInList.empty())
                                {
                                    for (Creature* itr : l_CreaturesTeronogorPhaseInList)
                                    {
                                        if (!itr)
                                            continue;

                                        itr->SetPhaseMask(1, true);
                                    }
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

/// Nyami Mob - 77810
class auchindoun_kaathar_mob_nyami : public CreatureScript
{
public:

    auchindoun_kaathar_mob_nyami() : CreatureScript("auchindoun_kaathar_mob_nyami") {}

    struct auchindoun_kaathar_mob_nyamiAI : public ScriptedAI
    {
        auchindoun_kaathar_mob_nyamiAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = creature->GetInstanceScript();
        }

        InstanceScript* m_Instance;

        void Reset() override
        {
            me->setFaction(FriendlyFaction);
            me->SetReactState(REACT_PASSIVE);
            me->CastSpell(me, SpellLevitateNyami);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_kaathar_mob_nyamiAI(creature);
    }
};

/// Vigilant Kaathar - 75839
class boss_kaathar : public CreatureScript
{
public:

    boss_kaathar() : CreatureScript("boss_kaathar") { }

    struct boss_kaatharAI : public BossAI
    {
        boss_kaatharAI(Creature* creature) : BossAI(creature, DataBossKathaar), m_Counting(0), m_IntroDone(false), m_Intro(false)
        {
            m_Instance = me->GetInstanceScript();
            m_False = true;
        }

        InstanceScript* m_Instance;
        uint32 m_Counting;
        bool m_IntroDone;
        bool m_Intro;
        bool m_False;

        void Reset() override
        {
            _Reset();
            events.Reset();
            KillAllDelayedEvents();
            me->SetCurrentEquipmentId(1);

            ActivateDoors();

            if (m_False)
            {
                m_Counting = 0;
                m_Intro = false;
                m_False = false;
                m_IntroDone = false;
                me->setFaction(FriendlyFaction);
                me->CastSpell(me, SpellGuard);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }

            if (m_Instance)
                m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellSanctifiedGroundAura);
        }

        void ActivateDoors()
        {
            if (m_Instance)
            {
                if (GameObject* l_HolyBarrier = m_Instance->instance->GetGameObject(m_Instance->GetGuidData(DataHolyBarrier)))
                {
                    l_HolyBarrier->SetLootState(GO_READY);
                    l_HolyBarrier->UseDoorOrButton();
                }
            }
        }

        void JustReachedHome() override
        {
            _JustReachedHome();
            summons.DespawnAll();
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who && who->IsInWorld() && who->IsPlayer() && !m_Intro && me->IsWithinDistInMap(who, 10.0f) && m_IntroDone)
            {
                m_Intro = true;
                Talk(VigilantKaatherIntro);
                me->SetHomePosition(g_KaatharNewHomePosition);
            }
        }

        void KilledUnit(Unit* who) override
        {
            if (who && who->IsPlayer())
                Talk(VigilantKaatherKill);
        }

        void EnterCombat(Unit* who) override
        {
            _EnterCombat();
            ActivateDoors();

            if (me->GetMap() && me->GetMap()->IsHeroic())
                events.RescheduleEvent(EventFate, 45 * IN_MILLISECONDS);

            events.RescheduleEvent(EventHallowedGround, urand(12 * IN_MILLISECONDS, 17 * IN_MILLISECONDS));
            events.RescheduleEvent(EventSanctifiedStrike, 8 * IN_MILLISECONDS);
            events.RescheduleEvent(EventHolyShield, 30 * IN_MILLISECONDS);

            if (m_Instance)
                m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            Talk(VigilantKaatherAgro);
        }

        void DoAction(int32 const actionID) override
        {
            switch (actionID)
            {
                case ActionCountPre1StBossKill:
                {
                    m_Counting = m_Counting + 1;

                    if (m_Counting >= 12)
                    {
                        me->RemoveAura(SpellGuard);

                        if (m_Instance)
                        {
                            if (Creature* l_Tuulani = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataTuulani)))
                            {
                                if (Creature* l_Nyami = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataNyami)))
                                {
                                    l_Tuulani->m_Events.KillAllEvents(true);
                                    l_Nyami->m_Events.KillAllEvents(true);
                                }
                            }
                        }

                        m_IntroDone = true;
                        me->m_Events.AddEvent(new EventNyamiEscape(me, 0), me->m_Events.CalculateTime(1 * IN_MILLISECONDS));
                    }
                    break;
                }
                case ActionDespawnCreatures:
                    DespawnAllAucheniDraeneis();
                    break;
                default:
                    break;
            }
        }

        /// Responsible for the phase change after Kaathar fight - despawning
        void DespawnAllAucheniDraeneis()
        {
            if (m_Instance)
            {
                if (Creature* l_AssinatingGuard = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataAssinatingGuard)))
                {
                    if (Creature* l_AssinatedGuard = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataAssinatedGuard)))
                    {
                        l_AssinatingGuard->GetMotionMaster()->MovePoint(0, *l_AssinatedGuard);
                        l_AssinatingGuard->Attack(l_AssinatedGuard, true);
                        l_AssinatedGuard->Kill(l_AssinatedGuard);
                    }
                }
            }

            uint32 l_Entries[13] = {77693, 76595, CreatureAuchenaiDefenderUnique, CreatureAucheniMagus2, CreatureAucheniHoplite, CreatureAucheniZealot, CreatureAucheniWarden, CreatureAucheniRitualist, CreatureAucheniMagus, CreatureAucheniSoulPriest, CreatureAucheniArbiter, CreatureAucheniCleric, CreatureAucheniDefender};

            for (uint8 l_I = 0; l_I < 13; l_I++)
                DespawnCreaturesInArea(l_Entries[l_I], me);
        }

        void JustDied(Unit* /*p_Killer*/) override
        {
            _JustDied();

            std::list<Player*> l_ListPlayers;
            me->GetPlayerListInGrid(l_ListPlayers, 600.0f/*, true*/);
            if (!l_ListPlayers.empty())
            {
                for (Player* itr : l_ListPlayers)
                {
                    if (!itr)
                        continue;

                    //itr->PlayScene(SpellAuchindounSceneTeronogorSpawn, itr);
                }
            }

            DespawnAllAucheniDraeneis();

            /// Remove the auchenai shield npc
            if (Creature* l_Nearest = me->FindNearestCreature(CreatureAuchenaiShield, 300.0f))
                l_Nearest->DespawnOrUnsummon();

            if (m_Instance)
                m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellSanctifiedGroundAura);

            Talk(VigilantKaatherDeath);
            me->m_Events.AddEvent(new EventPostKaathar(me, 0), me->m_Events.CalculateTime(2 * IN_MILLISECONDS));
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EventFate:
                {
                    me->CastSpell(me, SpellFate);
                    me->MonsterTextEmote("|cffff0000[Fate]|cfffaeb00!", me->GetGUID(), true);
                    events.RescheduleEvent(EventFate, 30 * IN_MILLISECONDS);
                    break;
                }
                case EventHallowedGround:
                {
                    Position l_Position;
                    me->GetRandomNearPosition(l_Position, 20.0f);
                    me->SummonCreature(TriggerHallowedGround, l_Position, TEMPSUMMON_TIMED_DESPAWN, 30 * IN_MILLISECONDS);
                    events.RescheduleEvent(EventHallowedGround, 8 * IN_MILLISECONDS);
                    break;
                }
                case EventHolyShield:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                    {
                        Talk(VigilantKaatherSpell2);
                        me->CastSpell(l_Target, SpellHolyShieldThrow);
                        if (me->HasAura(SpellHolyShieldOffHandDisarm))
                            me->AddAura(SpellHolyShieldTwoHandDisarm, me);
                        else
                            me->AddAura(SpellHolyShieldOffHandDisarm, me);

                        std::string l_Str;
                        l_Str += "Vigilant kaathar hurls his |cffff0000[Holy Shield]|cfffaeb00! at ";
                        l_Str += l_Target->GetName();
                        me->MonsterTextEmote(l_Str.c_str(), me->GetGUID(), true);
                        events.RescheduleEvent(EventConsecratedLight, 4 * IN_MILLISECONDS);
                        events.RescheduleEvent(EventHolyShieldReturn, 14 * IN_MILLISECONDS);
                    }
                    events.RescheduleEvent(EventHolyShield, 30 * IN_MILLISECONDS);
                    break;
                }
                case EventHolyShieldReturn:
                {
                    if (Creature* l_Shield = me->FindNearestCreature(TriggerHolyShield, 50.0f, true))
                    {
                        me->SetCurrentEquipmentId(1); /// Equipment Id
                        l_Shield->DespawnOrUnsummon(3 * IN_MILLISECONDS);
                        l_Shield->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 20.0f, 10.0f, 10.0f);
                        DespawnCreaturesInArea(TriggerHolyShield, me);

                        if (me->HasAura(SpellHolyShieldOffHandDisarm))
                            me->RemoveAura(SpellHolyShieldOffHandDisarm);
                        else if (me->HasAura(SpellHolyShieldTwoHandDisarm))
                            me->RemoveAura(SpellHolyShieldTwoHandDisarm);
                    }
                    break;
                }
                case EventConsecratedLight:
                {
                    Talk(VigilantKaatherSpell1);
                    DoCast(SpellConsecratedLight);
                    me->MonsterTextEmote("Vigilant Kaathar readies |cffff0000[Consecrated Light]|cfffaeb00!. Hide!", me->GetGUID(), true);
                    break;
                }
                case EventSanctifiedStrike:
                {
                    DoCastVictim(SpellSanctifiedStrikeDummy);

                    for (uint8 l_I = 0; l_I <= 10; l_I++)
                    {
                        float l_PosX = me->GetPositionX() + (l_I + 1) * cos(me->m_orientation);;
                        float l_PosY = me->GetPositionY() + l_I * sin(me->m_orientation);

                        l_PosX += frand(0.5f, 1.8f);
                        l_PosY += frand(0.7f, 1.9f);

                        me->SummonCreature(TriggerFissureSummoner, l_PosX, l_PosY, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 15 * IN_MILLISECONDS);
                    }
                    events.RescheduleEvent(EventSanctifiedStrike, 8 * IN_MILLISECONDS);
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
        return new boss_kaatharAI(creature);
    }
};

/// Hallowed Ground - 537324
class auchindoun_kaathar_mob_hallowed_ground : public CreatureScript
{
public:

    auchindoun_kaathar_mob_hallowed_ground() : CreatureScript("auchindoun_kaathar_mob_hallowed_ground") { }

    struct auchindoun_kaathar_mob_hallowed_groundAI : public Scripted_NoMovementAI
    {
        auchindoun_kaathar_mob_hallowed_groundAI(Creature* creature) : Scripted_NoMovementAI(creature), m_VisualDiff(0), m_HasExploded(false)
        {
            m_First = false;
        }

        uint32 m_VisualDiff;
        bool m_HasExploded;
        bool m_First;
        EventMap events;

        void Reset() override
        {
            events.Reset();
            m_HasExploded = false;
            me->setFaction(HostileFaction);
            m_VisualDiff = 1 * IN_MILLISECONDS;
            events.RescheduleEvent(EventCheckPlayer, 4 * IN_MILLISECONDS); // Takes 4 seconds to charge

            if (!m_First)
            {
                me->SetObjectScale(1.0f);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
            }
        }

        void DoAction(int32 const actionID) override
        {
            switch (actionID)
            {
                case ActionFateHallowedGround:
                    DoCast(me, SpellHallowedGround);
                    me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 p_Diff) override
        {
            events.Update(p_Diff);

            // Visual
            if (m_VisualDiff <= p_Diff)
            {
                me->CastSpell(me, SpellHallowedGroundsTriggerMissile);
                m_VisualDiff = 1 * IN_MILLISECONDS;
            }
            else
                m_VisualDiff -= p_Diff;

            switch (events.ExecuteEvent())
            {
                case EventCheckPlayer:
                    if (m_HasExploded)
                        return;

                    if (Player* player = me->FindNearestPlayer(4.0f, true))
                    {
                        if (player->IsWithinDistInMap(me, 4.0f))
                        {
                            m_HasExploded = true;

                            DoCast(me, SpellHallowedGround);
                            me->DespawnOrUnsummon(1 * IN_MILLISECONDS);
                        }
                    }

                    events.RescheduleEvent(EventCheckPlayer, 1 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_kaathar_mob_hallowed_groundAI(creature);
    }
};

/// Fissure Spawner Trigger - 543536
class auchindoun_kaathar_mob_spawn_fissures : public CreatureScript
{
public:
    auchindoun_kaathar_mob_spawn_fissures() : CreatureScript("auchindoun_kaathar_mob_spawn_fissures") { }

    struct auchindoun_kaathar_mob_spawn_fissuresAI : public Scripted_NoMovementAI
    {
        auchindoun_kaathar_mob_spawn_fissuresAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            m_Instance = creature->GetInstanceScript();
        }

        InstanceScript* m_Instance;

        void Reset() override
        {
            me->setFaction(HostileFaction);
            me->SetDisplayId(InvisibleDisplay);
            me->CastSpell(me, SpellSanctifiedStrikeAreaTrigger4);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_kaathar_mob_spawn_fissuresAI(creature);
    }
};

/// Post Fight Summoner to Center - 324235
class auchindoun_kaathar_mob_teleport_players : public CreatureScript
{
public:

    auchindoun_kaathar_mob_teleport_players() : CreatureScript("auchindoun_kaathar_mob_teleport_players") { }

    struct auchindoun_kaathar_mob_teleport_playersAI : public ScriptedAI
    {
        auchindoun_kaathar_mob_teleport_playersAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Instance = creature->GetInstanceScript();
        }

        InstanceScript* m_Instance;

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (m_Instance)
            {
                if (Creature* l_Kaathar = m_Instance->instance->GetCreature(m_Instance->GetGuidData(DataBossKathaar)))
                {
                    if (l_Kaathar->isDead())
                    {
                        if (Player * player = me->FindNearestPlayer(10.0f, true))
                            player->TeleportTo(1182, 1904.29f, 3185.111f, 30.799f, 3.34086f);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_kaathar_mob_teleport_playersAI(creature);
    }
};

/// Holy Shield - 76071
class auchindoun_kaathar_mob_holy_shield : public CreatureScript
{
public:
    auchindoun_kaathar_mob_holy_shield() : CreatureScript("auchindoun_kaathar_mob_holy_shield") { }

    struct auchindoun_kaathar_mob_holy_shieldAI : public Scripted_NoMovementAI
    {
        auchindoun_kaathar_mob_holy_shieldAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            m_Instance = creature->GetInstanceScript();
        }

        InstanceScript* m_Instance;

        void Reset() override
        {
            me->setFaction(HostileFaction);
            me->SetDisplayId(InvisibleDisplay);
            me->CastSpell(me, SpellHolyShieldLos);
            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            /// Knockback
            std::list<Player*> l_ListPlayers;
            me->GetPlayerListInGrid(l_ListPlayers, 5.0f);
            if (!l_ListPlayers.empty())
            {
                for (Player* l_itr : l_ListPlayers)
                {
                    if (!l_itr)
                        continue;

                    l_itr->CastSpell(l_itr, SpellHolyShieldKnockBack);
                    me->CastSpell(l_itr, SpellHolyShieldDamageFromKnockBack);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new auchindoun_kaathar_mob_holy_shieldAI(creature);
    }
};

/// Consecrated Light - 153006
class auchindoun_kaathar_spell_consecrated_light : public SpellScriptLoader
{
public:

    auchindoun_kaathar_spell_consecrated_light() : SpellScriptLoader("auchindoun_kaathar_spell_consecrated_light") { }

    class auchindoun_kaathar_spell_consecrated_light_SpellScript : public AuraScript
    {
        PrepareAuraScript(auchindoun_kaathar_spell_consecrated_light_SpellScript);

        void HandlePeriodic(AuraEffect const* /*auraEffect*/)
        {
            if (GetCaster())
            {
                if (InstanceScript* m_Instance = GetCaster()->GetInstanceScript())
                {
                    std::list<Player*> l_ListPlayers;
                    GetCaster()->GetPlayerListInGrid(l_ListPlayers, 200.0f);
                    if (!l_ListPlayers.empty())
                    {
                        for (Player* itr : l_ListPlayers)
                        {
                            if (!itr)
                                continue;

                            itr->CastSpell(itr, SpellConsecratedLightDamage, true);
                        }
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(auchindoun_kaathar_spell_consecrated_light_SpellScript::HandlePeriodic, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new auchindoun_kaathar_spell_consecrated_light_SpellScript();
    }
};

/// Consecrated Light - 156746
class auchindoun_kaathar_spell_conscreated_damage : public SpellScriptLoader
{
public:

    auchindoun_kaathar_spell_conscreated_damage() : SpellScriptLoader("auchindoun_kaathar_spell_conscreated_damage") { }

    class auchindoun_kaathar_spell_conscreated_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_kaathar_spell_conscreated_damage_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effectIndex*/)
        {
            if (!GetCaster() && !GetHitUnit())
                return;

            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                if (!GetHitDamage())
                    return;

                if (Creature* l_Kaathar = instance->instance->GetCreature(instance->GetGuidData(DataBossKathaar)))
                    if (Creature* l_Shield = GetCaster()->FindNearestCreature(TriggerHolyShield, 200.0f))
                        if (l_Shield->IsInBetween(GetHitUnit(), l_Kaathar))
                            SetHitDamage(0);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(auchindoun_kaathar_spell_conscreated_damage_SpellScript::RecalculateDamage, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_kaathar_spell_conscreated_damage_SpellScript();
    }
};

/// Sanctified Ground Periodic Dummy - 153430
class auchindoun_kaathar_spell_sanctified_ground : public SpellScriptLoader
{
public:

    auchindoun_kaathar_spell_sanctified_ground() : SpellScriptLoader("auchindoun_kaathar_spell_sanctified_ground") { }

    class auchindoun_kaathar_spell_sanctified_ground_AuraScript : public AuraScript
    {
        PrepareAuraScript(auchindoun_kaathar_spell_sanctified_ground_AuraScript);

        void HandlePeriodic(AuraEffect const* auraEffect)
        {
            PreventDefaultAction();

            if (GetCaster() && GetTarget())
            {
                GetTarget()->CastSpell(GetTarget(), SpellSanctifiedGroundTickDamage);
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(auchindoun_kaathar_spell_sanctified_ground_AuraScript::HandlePeriodic, SpellEffIndex::EFFECT_0, AuraType::SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new auchindoun_kaathar_spell_sanctified_ground_AuraScript();
    }
};

/// Fate - 157465
class auchindoun_kaathar_spell_fate : public SpellScriptLoader
{
public:

    auchindoun_kaathar_spell_fate() : SpellScriptLoader("auchindoun_kaathar_spell_fate") { }

    class auchindoun_kaathar_spell_fate_SpellScript : public SpellScript
    {
        PrepareSpellScript(auchindoun_kaathar_spell_fate_SpellScript);

        void HandleDummy(SpellEffIndex effectIndex)
        {
            if (Unit* l_Caster = GetCaster())
            {
                std::list<Creature*> l_HallowedGroundCreatures;
                GetCaster()->GetCreatureListWithEntryInGrid(l_HallowedGroundCreatures, TriggerHallowedGround, 150.0f);
                if (!l_HallowedGroundCreatures.empty())
                {
                    for (Creature* itr : l_HallowedGroundCreatures)
                    {
                        if (itr->IsAIEnabled)
                            itr->GetAI()->DoAction(ActionFateHallowedGround);
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(auchindoun_kaathar_spell_fate_SpellScript::HandleDummy, SpellEffIndex::EFFECT_0, SpellEffects::SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new auchindoun_kaathar_spell_fate_SpellScript();
    }
};

void AddSC_boss_kaathar()
{
    new boss_kaathar();                                                 ///< 75839
    //new auchindoun_kaathar_mob_spawn_fissures();                        ///< 543536
    //new auchindoun_kaathar_mob_hallowed_ground();                       ///< 537324
    new auchindoun_kaathar_mob_holy_shield();                           ///< 76071
    //new auchindoun_kaathar_mob_nyami();                                 ///< 77810
    //new auchindoun_kaathar_mob_teleport_players();                      ///< 3242352
    new auchindoun_kaathar_spell_consecrated_light();                   ///< 153006
    new auchindoun_kaathar_spell_fate();                                ///< 157465
    new auchindoun_kaathar_spell_sanctified_ground();                   ///< 153430
    new auchindoun_kaathar_spell_conscreated_damage();                  ///< 156746
}
