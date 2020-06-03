/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mogu_shan_vault.h"

enum eSpells
{   
    //Elegon
    SPELL_TOUCH_OF_THE_TITANS       = 117870,
    SPELL_OVERCHARGED               = 117877,
    SPELL_CELESTIAL_BREATH          = 117960,
    SPELL_ENERGY_TENDROLS           = 127362,
    SPELL_MATERIALIZE_PROTECTOR     = 117954,
    SPELL_VORTEX_VISIBILITY         = 127005,
    SPELL_CATASTROPHIC_ANOMALY      = 127341,
    SPELL_CATASTROPHIC_ANOMALY_BERS = 132256,
    //Phase 2
    SPELL_DRAW_POWER                = 124967,
    SPELL_FOCUS_POWER               = 119358,
    SPELL_DAMAGE_IMMUNE             = 118921,
    SPELL_UNSTABLE_ENERGY           = 116994,
    SPELL_ENERGY_CASCADE            = 122199,
    //Phase 3
    SPELL_RADIATING_ENERGIES        = 118310,
    SPELL_RADIATING_ENERGIES_VISUAL = 118992,

    //Charger
    SPELL_HIGH_ENERGY               = 118118,
    SPELL_CORE_BEAM                 = 118430,
    SPELL_DISCHARGE                 = 118299,
    SPELL_DISCHARGE_VISUAL          = 118023,

    //Tower
    SPELL_FOCUS_STATE_INACTIVE      = 127303,
    SPELL_FOCUS_STATE_ACTIVATE      = 127305,
    SPELL_LINKED_FOCUS_ACTIVATE     = 132257,
    SPELL_LINKED_FOCUS_INACTIVE     = 132258,
    SPELL_ENERGY_CONDUIT            = 116598,
    SPELL_ENERGY_CONDUIT_VISUAL     = 116604,
    SPELL_ENERGY_CONDUIT_AT         = 116546,
    SPELL_OVERLOADED                = 116989,
    SPELL_DESPAWN_AREA_TRIGGERS     = 115905,
    SPELL_PURGE_ALL_DEBUFFS         = 130286,

    //Protector
    SPELL_TOTAL_ANNIHILATION        = 129711,
    SPELL_TOTAL_ANNIHILATION_DMG    = 117914,
    SPELL_DESTABILIZED              = 132226, //Heroic
    SPELL_ARCING_ENERGY             = 117945,
    SPELL_STABILITY_FLUX            = 117911,
    SPELL_ECLIPSE                   = 117885,
};

enum eEvents
{
    //Elegon
    EVENT_VORTEX_VISIBILITY      = 1,
    EVENT_CHECK_DISTANCE         = 2,
    EVENT_BREATH                 = 3,
    EVENT_PROTECTOR              = 4,
    EVENT_ENERGY_CASCADE         = 5,
    EVENT_DISABLE_PLATFORM       = 6,
    EVENT_NEXT_PHASE             = 7,
    EVENT_CATASTROPHIC           = 8,

    //Buff controller
    EVENT_CHECK_DIST             = 9,

    //Protector
    EVENT_ARCING_ENERGY          = 10,
};

enum ePhase
{
    PHASE_ONE       = 1,
    PHASE_TWO       = 2,
    PHASE_THREE     = 3,
    PHASE_FOUR      = 4
};

Position const midpos = {4023.13f, 1907.75f, 358.083f, 0.0f};

Position const focusPos[6] =
{
    {3992.02f, 1876.64f, 358.872f, 3.92f},
    {4054.24f, 1938.86f, 358.872f, 0.78f},
    {4067.13f, 1907.75f, 358.872f, 0.0f},
    {4054.24f, 1876.64f, 358.872f, 5.49f},
    {3992.02f, 1938.86f, 358.872f, 2.35f},
    {3979.13f, 1907.75f, 358.872f, 3.14f}
};

class boss_elegon : public CreatureScript
{
    public:
        boss_elegon() : CreatureScript("boss_elegon") {}

        struct boss_elegonAI : public BossAI
        {
            boss_elegonAI(Creature* creature) : BossAI(creature, DATA_ELEGON), summons(me)
            {
                pInstance = creature->GetInstanceScript();
                me->SetCanFly(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            }

            InstanceScript* pInstance;
            SummonList summons;
            uint8 HealthPct;
            uint8 TowerCount;
            uint8 ChargeWave;
            uint32 BerserkTimer;
            bool secondPhaseStarted, secondPhaseActive, secondPhaseComplete;

            void Reset() override
            {
                _Reset();
                events.Reset();
                summons.DespawnAll();
                RemoveBuff();
                HealthPct = 85;
                TowerCount = 0;
                ChargeWave = 0;
                BerserkTimer = 9 * MINUTE * IN_MILLISECONDS + 30 * IN_MILLISECONDS;
                secondPhaseStarted = false;
                secondPhaseActive = false;
                secondPhaseComplete = false;
                me->SetReactState(REACT_DEFENSIVE);
                for (int8 i = 0; i < 7; i++)
                    me->SummonCreature(NPC_EMPYREAL_FOCUS, focusPos[i]);

                instance->HandleGameObject(instance->GetGuidData(GOB_ENERGY_PLATFORM), false);
            }

            void RemoveBuff()
            {
                if (pInstance)
                {
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_OVERCHARGED);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOUCH_OF_THE_TITANS);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DESTABILIZED);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(117878); //Overcharged (trigger aura)
                }
            }

            void EnterCombat(Unit* who) override
            {
                _EnterCombat();
                me->SummonCreature(NPC_INVISIBLE_STALKER, me->GetPositionX(), me->GetPositionY(), 360.0f); //Buff Controller
                me->SummonCreature(NPC_ENERGY_VORTEX_STALKER, me->GetPositionX(), me->GetPositionY(), 360.0f);

                ChangePhase(PHASE_ONE);
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);

                if (summoned->GetEntry() == NPC_ENERGY_CHARGE)
                {
                    for (uint8 i = 0; i < ChargeWave; i++)
                        summoned->CastSpell(summoned, SPELL_HIGH_ENERGY, true);
                }

                if (summoned->GetEntry() == NPC_ENERGY_VORTEX_STALKER)
                    summoned->SetReactState(REACT_PASSIVE);
            }
            void JustDied(Unit* attacker) override
            {
                _JustDied();
                summons.DespawnAll();
                instance->HandleGameObject(instance->GetGuidData(GOB_ENERGY_PLATFORM), false);
                RemoveBuff();
                me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ() - 10, me->GetHomePosition().GetOrientation());
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_FOCUS_POWER)
                    ChargeWave++;
            }

            void DoAction(const int32 action) override
            {
                switch (action)
                {
                    case ACTION_1:
                        if (!secondPhaseStarted)
                        {
                            secondPhaseStarted = true;
                            ChargeWave = 0;
                            ChangePhase(PHASE_THREE);
                        }
                        break;
                    case ACTION_2:
                        TowerCount--;
                        if (!TowerCount)
                        {
                            secondPhaseActive = false;
                            events.RescheduleEvent(EVENT_NEXT_PHASE, 6000);
                        }
                        break;
                    case ACTION_3:
                        events.RescheduleEvent(EVENT_CATASTROPHIC, 1000);
                        break;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (HealthBelowPct(HealthPct) && !secondPhaseActive && !secondPhaseComplete)
                {
                    if (HealthPct <= 50)
                        secondPhaseComplete = true;

                    HealthPct -= 35;
                    secondPhaseStarted = false;
                    secondPhaseActive = true;
                    me->StopAttack();
                    ChangePhase(PHASE_TWO);
                }
            }

            void ChangePhase(uint8 newPhase)
            {
                events.Reset();
                me->InterruptNonMeleeSpells(false);
                me->RemoveAurasDueToSpell(SPELL_DAMAGE_IMMUNE);
                me->RemoveAurasDueToSpell(SPELL_UNSTABLE_ENERGY);

                switch (newPhase)
                {
                    case PHASE_ONE:
                        events.RescheduleEvent(EVENT_CHECK_DISTANCE, 5000);
                        events.RescheduleEvent(EVENT_BREATH, 10000);
                        events.RescheduleEvent(EVENT_PROTECTOR, 20000);
                        if (IsHeroic())
                            events.RescheduleEvent(EVENT_VORTEX_VISIBILITY, 500);
                        break;
                    case PHASE_TWO:
                        DoCast(SPELL_DRAW_POWER);
                        break;
                    case PHASE_THREE:
                    {
                        TowerCount = 6;
                        EntryCheckPredicate pred(NPC_EMPYREAL_FOCUS);
                        summons.DoAction(ACTION_1, pred);
                        DoCast(me, SPELL_DAMAGE_IMMUNE, true);
                        DoCast(me, SPELL_UNSTABLE_ENERGY, true);
                        events.RescheduleEvent(EVENT_ENERGY_CASCADE, 5000);
                        events.RescheduleEvent(EVENT_DISABLE_PLATFORM, 10000);
                        break;
                    }
                    case PHASE_FOUR:
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoCast(me, SPELL_RADIATING_ENERGIES_VISUAL, true);
                        DoCast(SPELL_RADIATING_ENERGIES);
                        events.RescheduleEvent(EVENT_BREATH, 10000);
                        if (IsHeroic())
                            events.RescheduleEvent(EVENT_VORTEX_VISIBILITY, 500);
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (BerserkTimer <= diff)
                {
                    me->InterruptNonMeleeSpells(false);
                    DoCast(me, SPELL_CATASTROPHIC_ANOMALY_BERS);
                    BerserkTimer = 9 * MINUTE * IN_MILLISECONDS + 30 * IN_MILLISECONDS;
                }
                else BerserkTimer -= diff;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                switch (events.ExecuteEvent())
                {
                    case EVENT_VORTEX_VISIBILITY:
                        if (Creature* vortex = me->FindNearestCreature(NPC_ENERGY_VORTEX_STALKER, 15.0f))
                            vortex->CastSpell(vortex, SPELL_VORTEX_VISIBILITY);
                        events.RescheduleEvent(EVENT_VORTEX_VISIBILITY, 500);
                        break;
                    case EVENT_CHECK_DISTANCE:
                        if (me->getVictim())
                            if (!me->IsWithinMeleeRange(me->getVictim()))
                                DoCast(SPELL_ENERGY_TENDROLS);
                        events.RescheduleEvent(EVENT_CHECK_DISTANCE, 5000);
                        break;
                    case EVENT_BREATH:
                        DoCast(me, SPELL_CELESTIAL_BREATH);
                        events.RescheduleEvent(EVENT_BREATH, 15000);
                        break;
                    case EVENT_PROTECTOR:
                        DoCast(SPELL_MATERIALIZE_PROTECTOR);
                        events.RescheduleEvent(EVENT_PROTECTOR, 25000);
                        break;
                    case EVENT_ENERGY_CASCADE:
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f , true))
                            DoCast(pTarget, SPELL_ENERGY_CASCADE);
                        events.RescheduleEvent(EVENT_ENERGY_CASCADE, 4000);
                        break;
                    case EVENT_DISABLE_PLATFORM:
                        instance->HandleGameObject(instance->GetGuidData(GOB_ENERGY_PLATFORM), true);
                        break;
                    case EVENT_NEXT_PHASE:
                    {
                        if (secondPhaseComplete)
                            ChangePhase(PHASE_FOUR);
                        else
                            ChangePhase(PHASE_ONE);

                        instance->HandleGameObject(instance->GetGuidData(GOB_ENERGY_PLATFORM), false);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveAurasDueToSpell(117204);
                        break;
                    }
                    case EVENT_CATASTROPHIC:
                        DoCast(SPELL_CATASTROPHIC_ANOMALY);
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_elegonAI(creature);
        }
};

class npc_buff_controller : public CreatureScript
{
    public:
        npc_buff_controller() : CreatureScript("npc_buff_controller") {}

        struct npc_buff_controllerAI : public CreatureAI
        {
            npc_buff_controllerAI(Creature* creature) : CreatureAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(11686);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.RescheduleEvent(EVENT_CHECK_DIST, 1000);
            }

            void CheckDistPlayersToMe()
            {
                if (Map* map = me->GetMap())
                    if (map->IsDungeon())
                    {
                        Map::PlayerList const &players = map->GetPlayers();
                        for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                        {
                            if (Player* pl = i->getSource())
                            {
                                if (pl->isAlive() && pl->GetDistance(me) <= 38.9f)
                                {
                                    if (!pl->HasAura(SPELL_TOUCH_OF_THE_TITANS))
                                        pl->AddAura(SPELL_TOUCH_OF_THE_TITANS, pl);

                                    if (!pl->HasAura(SPELL_OVERCHARGED))
                                        pl->AddAura(SPELL_OVERCHARGED, pl);
                                }
                                else if (pl->isAlive() && pl->GetDistance(me) >= 38.9f)
                                {
                                    pl->RemoveAurasDueToSpell(SPELL_TOUCH_OF_THE_TITANS);
                                    pl->RemoveAurasDueToSpell(SPELL_OVERCHARGED);
                                    pl->RemoveAurasDueToSpell(117878); //Overcharged (trigger aura)
                                }
                            }
                        }
                        std::list<Creature*> creatures;
                        GetCreatureListWithEntryInGrid(creatures, me, NPC_COSMIC_SPARK, 100.0f);
                        for (std::list<Creature*>::iterator itr = creatures.begin(); itr != creatures.end(); ++itr)
                            if (me->GetDistance((*itr)) <= 38.9f)
                            {
                                if (!(*itr)->HasAura(SPELL_TOUCH_OF_THE_TITANS))
                                    (*itr)->CastSpell((*itr), SPELL_TOUCH_OF_THE_TITANS, true);

                                if (!(*itr)->HasAura(SPELL_OVERCHARGED))
                                    (*itr)->CastSpell((*itr), SPELL_OVERCHARGED, true);
                            }
                            else if (me->GetDistance((*itr)) >= 38.9f)
                            {
                                (*itr)->RemoveAurasDueToSpell(SPELL_TOUCH_OF_THE_TITANS);
                                (*itr)->RemoveAurasDueToSpell(SPELL_OVERCHARGED);
                                (*itr)->RemoveAurasDueToSpell(117878); //Overcharged (trigger aura)
                            }
                        events.RescheduleEvent(EVENT_CHECK_DIST, 1000);
                    }
            }
            
            void EnterEvadeMode() override {}

            void EnterCombat(Unit* who) override {}

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                case EVENT_CHECK_DIST:
                    CheckDistPlayersToMe();
                    break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_buff_controllerAI(creature);
        }
};

//60793
class npc_celestial_protector : public CreatureScript
{
    public:
        npc_celestial_protector() : CreatureScript("npc_celestial_protector") { }

        struct npc_celestial_protectorAI : public ScriptedAI
        {
            npc_celestial_protectorAI(Creature* creature) : ScriptedAI(creature){}

            EventMap events;
            bool flux;
            bool annihilation;
            bool eclipse;

            void Reset() override
            {
                DoZoneInCombat(me, 100.0f);
                events.Reset();
                flux = false;
                eclipse = false;
                annihilation = false;
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.RescheduleEvent(EVENT_ARCING_ENERGY, 12000);
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_TOTAL_ANNIHILATION_DMG)
                    if (target->HasAura(SPELL_DESTABILIZED))
                        me->CastSpell(target, 132232, true); // Kill Player
            }

            void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
            {
                if (me->HealthBelowPctDamaged(25, damage) && !flux)
                {
                    flux = true;
                    DoCast(me, SPELL_STABILITY_FLUX);
                }

                if (me->GetHealthPct() > 25.0f && me->GetDistance(midpos) > 40 && !eclipse)
                {
                    eclipse = true;
                    DoCast(me, SPELL_ECLIPSE);
                }

                if (me->GetHealth() <= damage)
                {
                    damage = 0;

                    if (!annihilation)
                    {
                        annihilation = true;
                        events.Reset();
                        me->StopAttack();
                        me->DespawnOrUnsummon(7000);
                        DoCast(me, SPELL_TOTAL_ANNIHILATION);
                    }
                }
            }

            void UpdateAI(uint32 diff) override
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
                        case EVENT_ARCING_ENERGY:
                            DoCast(SPELL_ARCING_ENERGY);
                            events.RescheduleEvent(EVENT_ARCING_ENERGY, 12000);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_celestial_protectorAI(creature);
        }
};

//60913
class npc_energy_charge : public CreatureScript
{
    public:
        npc_energy_charge() : CreatureScript("npc_energy_charge") {}

        struct npc_energy_chargeAI : public ScriptedAI
        {
            npc_energy_chargeAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
            bool active;

            void Reset() override {}

            void IsSummonedBy(Unit* summoner)
            {
                active = true;

                if (Creature* empyrealFocus = me->FindNearestCreature(NPC_EMPYREAL_FOCUS, 40.0f))
                {
                    DoCast(empyrealFocus, SPELL_CORE_BEAM, true);
                    me->GetMotionMaster()->MovePoint(1, empyrealFocus->GetPositionX(), empyrealFocus->GetPositionY(), empyrealFocus->GetPositionZ());
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    if (active)
                    {
                        active = false;
                        DoCast(SPELL_DISCHARGE);
                        DoCast(me, SPELL_DISCHARGE_VISUAL, true);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    }
                }
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == 1)
                {
                    if (active)
                        if (Creature* elegon = instance->instance->GetCreature(instance->GetGuidData(NPC_ELEGON)))
                            elegon->AI()->DoAction(ACTION_1);
                    me->DespawnOrUnsummon();
                }
            }

            void UpdateAI(uint32 diff) override {}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_energy_chargeAI(creature);
        }
};

//60776
class npc_empyreal_focus : public CreatureScript
{
    public:
        npc_empyreal_focus() : CreatureScript("npc_empyreal_focus") {}

        struct npc_empyreal_focusAI : public Scripted_NoMovementAI
        {
            npc_empyreal_focusAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
            }

            InstanceScript* instance;
            bool active;
            uint16 CheckTimer;
            ObjectGuid targetfocusGUID;

            void Reset() override 
            {
                CheckTimer = 1000;
            }

            void DoAction(const int32 action) override
            {
                if (ACTION_1)
                {
                    active = true;
                    targetfocusGUID.Clear();
                    me->SetHealth(me->GetMaxHealth());
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    me->SetOrientation(me->GetHomePosition().GetOrientation());
                    DoCast(me, SPELL_ENERGY_CONDUIT_AT, true);
                    DoCast(me, SPELL_FOCUS_STATE_ACTIVATE, true);
                    DoCast(me, SPELL_LINKED_FOCUS_ACTIVATE, true);

                    std::list<Creature*> creatures;
                    GetCreatureListWithEntryInGrid(creatures, me, NPC_EMPYREAL_FOCUS, 90.0f);
                    for (std::list<Creature*>::iterator itr = creatures.begin(); itr != creatures.end(); ++itr)
                        if (me->GetDistance((*itr)) > 70.0f)
                            DoCast((*itr), SPELL_ENERGY_CONDUIT, true);
                }
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_ENERGY_CONDUIT)
                    targetfocusGUID = caster->GetGUID();
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;

                    if (active)
                    {
                        active = false;
                        me->RemoveAurasDueToSpell(SPELL_LINKED_FOCUS_ACTIVATE);
                        DoCast(me, SPELL_LINKED_FOCUS_INACTIVE, true);
                    }
                }
            }

            void DeactivateFocus()
            {
                Creature* focus = me->GetCreature(*me, targetfocusGUID);
                if (!focus)
                    return;

                if (me->HasAura(SPELL_LINKED_FOCUS_INACTIVE) && focus->HasAura(SPELL_LINKED_FOCUS_INACTIVE))
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    me->RemoveAurasDueToSpell(SPELL_FOCUS_STATE_ACTIVATE);
                    me->RemoveAurasDueToSpell(SPELL_ENERGY_CONDUIT);
                    me->RemoveAurasDueToSpell(SPELL_ENERGY_CONDUIT_VISUAL);

                    DoCast(SPELL_OVERLOADED);
                    DoCast(me, SPELL_FOCUS_STATE_INACTIVE, true);
                    DoCast(me, SPELL_DESPAWN_AREA_TRIGGERS, true);
                    DoCast(me, SPELL_PURGE_ALL_DEBUFFS, true);
                    active = true;
    
                    if (Creature* elegon = instance->instance->GetCreature(instance->GetGuidData(NPC_ELEGON)))
                        elegon->AI()->DoAction(ACTION_2);
                }
            }

            void UpdateAI(uint32 diff) override 
            {
                if (!active)
                {
                    if (CheckTimer <= diff)
                    {
                        DeactivateFocus();
                        CheckTimer = 1000;
                    }
                    else
                        CheckTimer -=diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_empyreal_focusAI(creature);
        }
};

//118010, 118011, 118012, 118014, 118015, 118016
class spell_elegon_draw_power : public SpellScriptLoader
{
    public:
        spell_elegon_draw_power() : SpellScriptLoader("spell_elegon_draw_power") { }

        class spell_elegon_draw_power_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_elegon_draw_power_SpellScript);

            void ModDestHeight(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(EFFECT_0);

                Unit* caster = GetCaster();
                if (!caster)
                    return;

                float x, y;
                WorldLocation loc;

                switch (GetSpellInfo()->Id)
                {
                    case 118010:
                        caster->GetNearPoint2D(x, y, 5.0f, 3.9f);
                        break;
                    case 118011:
                        caster->GetNearPoint2D(x, y, 5.0f, 3.1f);
                        break;
                    case 118012:
                        caster->GetNearPoint2D(x, y, 5.0f, 2.3f);
                        break;
                    case 118014:
                        caster->GetNearPoint2D(x, y, 5.0f, 5.5f);
                        break;
                    case 118015:
                        caster->GetNearPoint2D(x, y, 5.0f, 6.2f);
                        break;
                    case 118016:
                        caster->GetNearPoint2D(x, y, 5.0f, 0.8f);
                        break;
                }
                loc.Relocate(x, y, caster->GetPositionZ() - 8.0f);
                GetHitDest()->Relocate(loc);
            }

            void Register() override
            {
                OnEffectLaunch += SpellEffectFn(spell_elegon_draw_power_SpellScript::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_elegon_draw_power_SpellScript();
        }
};

//132222
class spell_elegon_destabilizing_energies : public SpellScriptLoader
{
public:
    spell_elegon_destabilizing_energies() : SpellScriptLoader("spell_elegon_destabilizing_energies") { }

    class spell_elegon_destabilizing_energies_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_elegon_destabilizing_energies_AuraScript)

            void OnPeriodic(AuraEffect const*aurEff)
            {
                if (!GetTarget())
                    return;

                if (GetTarget()->HealthAbovePct(80))
                {
                    GetTarget()->CastSpell(GetTarget(), SPELL_DESTABILIZED, true);
                    GetTarget()->RemoveAurasDueToSpell(aurEff->GetId());
                }
            }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_elegon_destabilizing_energies_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_elegon_destabilizing_energies_AuraScript();
    }
};

class TargetAurasCheck
{
public:
    TargetAurasCheck(Unit* caster) : _caster(caster) {}
    
    bool operator()(WorldObject* target)
    {
        Player* plr = target->ToPlayer();
        
        if (!plr)
            return false;

        if (plr->HasAura(SPELL_VORTEX_VISIBILITY) && !_caster->HasAura(SPELL_VORTEX_VISIBILITY)
            || !plr->HasAura(SPELL_VORTEX_VISIBILITY) && _caster->HasAura(SPELL_VORTEX_VISIBILITY))
            return true;
        
        return false;
    }
private:
    Unit* _caster;
};

//117912, 117914, 132222
class spell_elegon_vortex_filter : public SpellScriptLoader
{
public:
    spell_elegon_vortex_filter() : SpellScriptLoader("spell_elegon_vortex_filter") { }

    class spell_elegon_vortex_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_elegon_vortex_filter_SpellScript);

        void FilterTarget(std::list<WorldObject*>& targets)
        {
            targets.remove_if(TargetAurasCheck(GetCaster()));

            if (GetSpellInfo()->Id == SPELL_TOTAL_ANNIHILATION_DMG)
            {
                uint8 raidMod = GetCaster()->GetMap()->Is25ManRaid() ? 3 : 1;
                if (targets.size() < raidMod)
                    if (InstanceScript* pInstance = GetCaster()->GetInstanceScript())
                        if (Creature* elegon = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_ELEGON)))
                            elegon->AI()->DoAction(ACTION_3);
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_elegon_vortex_filter_SpellScript::FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_elegon_vortex_filter_SpellScript::FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_elegon_vortex_filter_SpellScript();
    }
};

void AddSC_boss_elegon()
{
    new boss_elegon();
    new npc_buff_controller();
    new npc_celestial_protector();
    new npc_energy_charge();
    new npc_empyreal_focus();
    new spell_elegon_draw_power();
    new spell_elegon_destabilizing_energies();
    new spell_elegon_vortex_filter();
}
