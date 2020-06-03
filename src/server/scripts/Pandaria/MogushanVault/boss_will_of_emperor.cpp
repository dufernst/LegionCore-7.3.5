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
    // Jan Xi && Qin Xi
    SPELL_ZERO_REGEN                = 118357,
    SPELL_ENERGIZE_REGEN            = 118365,
    SPELL_MAGNETIC_ARMOR_Q          = 116815,
    SPELL_MAGNETIC_ARMOR_J          = 117193,

    SPELL_ARC_RIGHT                 = 116971, // Удар по дуге справа 117005
    SPELL_DEVASTATING_ARC_RIGHT     = 117005,
    SPELL_ARC_LEFT                  = 116968, // Удар по дуге слева 117003
    SPELL_DEVASTATING_ARC_LEFT      = 117003,
    SPELL_ARC_CENTER                = 116972, // Удар по дуге перед собой
    SPELL_DEVASTATING_ARC_FRONT     = 117006,
    SPELL_STOMP                     = 116969,
    SPELL_GROWING_OPPORTUNITY       = 117854, // Ожидание нужного момента

    // Woi controller
    SPELL_TITAN_GAS                 = 116779,
    // On players if evade death attack from boss
    SPELL_OPPORTUNISTIC_STRIKE      = 116808,
    // Rage
    SPELL_FOCALISED_ASSAULT         = 116525,
    SPELL_WITHOUT_ARMOR             = 116535,
    // Courage
    SPELL_FOCALISED_DEFENSE         = 116778,
    SPELL_PHALANX_WALL              = 116549,
    SPELL_IMPEDING_THRUST           = 117485,
    SPELL_HALF_PLATE                = 116537,
    // Force
    SPELL_ENERGIZING_SMASH          = 116550,
    SPELL_FULL_PLATE                = 116540,
    // Titan Spark
    SPELL_ENERGY_VISUAL             = 116673,
    SPELL_SUMMON_TITAN_SPARK        = 117746,
    SPELL_FOCALISED_ENERGY          = 116829,
    SPELL_ENERGY_OF_CREATION        = 117734
};

enum eEvents
{
    // Imperators
    EVENT_DEATH_ATTACKS_START       = 8,
    EVENT_DEATH_ATTACKS_END         = 9,
    // All adds
    EVENT_CHECK_TARGET              = 10,
    // Courage
    EVENT_IMPEDING_THRUST           = 12,
    // Strenght
    EVENT_ENERGIZING_SMASH          = 13,
    // Woi Controller
    EVENT_CHECK_WIPE                = 14,
    EVENT_SPAWN_IMPERATOR           = 15,
    EVENT_TITAN_GAS                 = 16,
    EVENT_SPAWN_RANDOM_ADD          = 17,
    EVENT_SPAWN_RAGE                = 60396,
    EVENT_SPAWN_FORCE               = 60397,
    EVENT_SPAWN_COURAGE             = 60398,
};

enum esAction
{
    // Jan Xi && Qin Xi
    ACTION_HIT_DEATH_ATTACK     = 1,
    // Woi controller
    ACTION_DONE                 = 2,
};

Position const janxipos  = {3829.48f, 1523.41f, 362.26f, 0.6683f};
Position const qinxipos  = {3828.49f, 1576.28f, 362.26f, 5.9148f};

Position const ragepos[4] =
{
    {3819.99f, 1538.02f, 362.31f, 0.19f},
    {3821.39f, 1550.37f, 362.26f, 0.04f},
    {3814.87f, 1563.53f, 368.72f, 6.07f},
    {3827.08f, 1577.66f, 362.30f, 5.79f},
};

Position const couragepos[2] =
{
    {3894.50f, 1498.40f, 362.26f, 1.6925f},
    {3895.52f, 1602.42f, 362.26f, 4.8458f},
};

Position const forcepos[2] = 
{
    {3841.71f, 1591.97f, 362.27f, 5.5487f},
    {3841.46f, 1508.87f, 362.28f, 0.8599f},
};

uint32 imperators[2] = 
{
    NPC_QIN_XI,
    NPC_JAN_XI,  
};

uint32 spellList[4] = {SPELL_ARC_RIGHT, SPELL_ARC_LEFT, SPELL_ARC_CENTER, SPELL_STOMP};

//200467
class npc_woi_controller : public CreatureScript
{
    public:
        npc_woi_controller() : CreatureScript("npc_woi_controller") {}

        struct npc_woi_controllerAI : public BossAI
        {
            npc_woi_controllerAI(Creature* creature) : BossAI(creature, DATA_WILL_OF_EMPEROR), summons(me)
            {
                pInstance = creature->GetInstanceScript();
                me->SetDisplayId(11686);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                done = false;
            }

            InstanceScript* pInstance;
            SummonList summons;
            bool done;
            uint8 raidMode;
            uint32 addentry[3]; //Array for summons entry

            void Reset() override
            {
                if (pInstance)
                {
                    _Reset();
                    summons.DespawnAll();
                    me->RemoveAurasDueToSpell(SPELL_TITAN_GAS);
                    RemovePursuitAuraOnPlayers();
                    for (uint8 n = 0; n < 3 ; n++)
                        addentry[n] = 0;
                }
            }

            void RemovePursuitAuraOnPlayers()
            {
               pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FOCALISED_ASSAULT);
               pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FOCALISED_DEFENSE);
            }

            void EnterCombat(Unit* who) override
            {
                //_EnterCombat();
                DoZoneInCombat();
                instance->SetBossState(DATA_WILL_OF_EMPEROR, IN_PROGRESS);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_START, me);
                PushAddArray();
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);
            }

            void PushAddArray()
            {
                uint8 pos = urand(0, 3);
                switch (pos)
                {
                case 0:
                    addentry[0] = NPC_RAGE;
                    addentry[1] = NPC_FORCE;                     
                    addentry[2] = NPC_COURAGE;
                    break;
                case 1:
                    addentry[0] = NPC_FORCE;
                    addentry[1] = NPC_RAGE;                    
                    addentry[2] = NPC_COURAGE;
                    break;
                case 2:
                    addentry[0] = NPC_COURAGE;
                    addentry[1] = NPC_FORCE;                    
                    addentry[2] = NPC_RAGE; 
                    break;
                case 3:
                    addentry[0] = NPC_RAGE;
                    addentry[1] = NPC_COURAGE;                      
                    addentry[2] = NPC_FORCE;
                    break;
                }
                StartEvent();
            }

            void StartEvent()
            {
                events.RescheduleEvent(EVENT_CHECK_WIPE,       1500);
                events.RescheduleEvent(addentry[0],            1000);
                events.RescheduleEvent(addentry[1],            30000);
                events.RescheduleEvent(addentry[2],            60000);
                events.RescheduleEvent(EVENT_SPAWN_IMPERATOR,  90000);
                if (IsHeroic())
                    events.RescheduleEvent(EVENT_TITAN_GAS, 1000);
            }

            void JustDied(Unit* /*killer*/) override {}

            void DoAction(const int32 action) override
            {
                switch (action)
                {
                    case ACTION_DONE:
                        if (pInstance && !done)
                        {
                            done = true;
                            RemovePursuitAuraOnPlayers();
                            pInstance->SetBossState(DATA_WILL_OF_EMPEROR, DONE);
                            summons.DespawnEntry(NPC_RAGE);
                            summons.DespawnEntry(NPC_FORCE);
                            summons.DespawnEntry(NPC_COURAGE);
                            me->DespawnOrUnsummon(100);
                        }
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CHECK_WIPE:
                        if (pInstance->IsWipe())
                            EnterEvadeMode();
                        events.RescheduleEvent(EVENT_CHECK_WIPE, 1500);
                        break;
                    case EVENT_SPAWN_RAGE:
                        raidMode = Is25ManRaid() ? 4 : 2;
                        for (uint8 i = 0; i < raidMode; i++)
                            me->SummonCreature(NPC_RAGE, ragepos[i], TEMPSUMMON_CORPSE_DESPAWN);
                        break;
                    case EVENT_SPAWN_FORCE:
                        if (Is25ManRaid())
                            for (uint8 i = 0; i < 2; i++)
                                me->SummonCreature(NPC_FORCE, forcepos[i], TEMPSUMMON_CORPSE_DESPAWN);
                        else
                            me->SummonCreature(NPC_COURAGE, couragepos[rand()%2], TEMPSUMMON_CORPSE_DESPAWN);
                        break;
                    case EVENT_SPAWN_COURAGE:
                        if (Is25ManRaid())
                            for (uint8 i = 0; i < 2; i++)
                                me->SummonCreature(NPC_COURAGE, couragepos[i], TEMPSUMMON_CORPSE_DESPAWN);
                        else
                            me->SummonCreature(NPC_COURAGE, couragepos[rand()%2], TEMPSUMMON_CORPSE_DESPAWN);
                        break;
                    case EVENT_SPAWN_RANDOM_ADD:
                        {
                            uint8 pos = urand(0, 2);
                            switch (pos)
                            {
                            case 0:
                                events.RescheduleEvent(EVENT_SPAWN_RAGE, 500);
                                break;
                            case 1:
                                events.RescheduleEvent(EVENT_SPAWN_FORCE, 500);
                                break;
                            case 2:
                                events.RescheduleEvent(EVENT_SPAWN_COURAGE, 500);
                                break;
                            }
                            events.RescheduleEvent(EVENT_SPAWN_RANDOM_ADD, 30000);
                            break;
                        }
                    case EVENT_SPAWN_IMPERATOR:
                        me->SummonCreature(NPC_QIN_XI, qinxipos);
                        me->SummonCreature(NPC_JAN_XI, janxipos);
                        events.RescheduleEvent(EVENT_SPAWN_RANDOM_ADD, 500);
                        if (!IsHeroic())
                            events.RescheduleEvent(EVENT_TITAN_GAS, 120000);
                        break;
                    case EVENT_TITAN_GAS:
                        DoCast(me, SPELL_TITAN_GAS, true);
                        if (!IsHeroic())
                            events.RescheduleEvent(EVENT_TITAN_GAS, 120000);
                        else
                            events.RescheduleEvent(EVENT_TITAN_GAS, 30000);
                        break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_woi_controllerAI(creature);
        }
};

void SendDied(InstanceScript* pInstance, Creature* caller, uint32 callerentry)
{
    if (caller && pInstance)
    {
        for (uint8 n = 0; n < 2; n++)
        {
            if (Creature* imperator = caller->GetCreature(*caller, pInstance->GetGuidData(imperators[n])))
            {
                if (imperator->isAlive() && imperator->GetEntry() != callerentry)
                {
                    imperator->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    imperator->Kill(imperator, true);
                    imperator->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                }
            }
        }
        if (Creature* controller = caller->GetCreature(*caller, pInstance->GetGuidData(NPC_WOI_CONTROLLER)))
            controller->AI()->DoAction(ACTION_DONE);
    }
}

void SendDamage(InstanceScript* pInstance, Creature* caller, uint32 callerentry, uint32 damage)
{
    if (caller && pInstance)
    {
        for (uint8 n = 0; n < 2; n++)
        {
            if (Creature* imperator = caller->GetCreature(*caller, pInstance->GetGuidData(imperators[n])))
            {
                if (imperator->isAlive() && imperator->GetEntry() != callerentry)
                    imperator->SetHealth(imperator->GetHealth() - damage);
            }
        }
    }
}

//60399, 60400
class boss_generic_imperator : public CreatureScript
{
    public:
        boss_generic_imperator() : CreatureScript("boss_generic_imperator") {}

        struct boss_generic_imperatorAI : public ScriptedAI
        {
            boss_generic_imperatorAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
            bool deathAttack;
            bool checkHitAttack;
            uint16 powerTimer;
            std::vector<int> list;
            std::vector<int> listStrike;

            void Reset() override
            {
                deathAttack = false;
                me->LowerPlayerDamageReq(me->GetMaxHealth());
                me->RemoveAurasDueToSpell(SPELL_ENERGIZE_REGEN);
                DoCast(me, SPELL_ZERO_REGEN, true);
                DoZoneInCombat(me, 150.0f);
                powerTimer = 500;
                list = { 0, 1, 2, 3 };
                listStrike = { 0, 1, 2, 3 };
            }

            void EnterCombat(Unit* /*who*/)
            {
                DoCast(me, SPELL_ENERGIZE_REGEN, true);

                events.RescheduleEvent(EVENT_1, 10000);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (damage >= me->GetHealth())
                    SendDied(pInstance, me, me->GetEntry());
                else
                    SendDamage(pInstance, me, me->GetEntry(), damage);
            }

            void DoAction(const int32 action) override
            {
                if (action == ACTION_HIT_DEATH_ATTACK)
                    checkHitAttack = false;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (powerTimer <= diff)
                {
                    powerTimer = 500;
                    if (me->isInCombat())
                    {
                        if (me->GetPower(POWER_MANA) == 20 && !deathAttack)
                        {
                            deathAttack = true;
                            checkHitAttack = true;
                            me->RemoveAurasDueToSpell(SPELL_ENERGIZE_REGEN);
                            events.RescheduleEvent(EVENT_DEATH_ATTACKS_START, 1000);
                        }
                    }
                }
                else
                    powerTimer -= diff;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                        {
                            if (me->GetEntry() == NPC_QIN_XI)
                                DoCast(me, SPELL_MAGNETIC_ARMOR_Q, true);
                            if (me->GetEntry() == NPC_JAN_XI)
                                DoCast(me, SPELL_MAGNETIC_ARMOR_J, true);
                            events.RescheduleEvent(EVENT_1, 10000);
                            break;
                        }
                        case EVENT_DEATH_ATTACKS_START:
                        {
                            if (me->GetPower(POWER_MANA) > 0)
                            {
                                uint8 now = urand(0, listStrike.size() - 1);
                                //if (Unit* pTarget = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                //{
                                    DoCast(spellList[listStrike[now]]);
                                //    me->SetFacingToObject(pTarget);
                                //}
                                listStrike.erase(listStrike.begin() + now);
 
                                if (listStrike.empty())
                                    listStrike = list;
                            }
                            else
                            {
                                deathAttack = false;
                                DoCast(me, SPELL_ENERGIZE_REGEN, true);
                                if (checkHitAttack)
                                {
                                    checkHitAttack = false;
                                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                        pTarget->CastSpell(pTarget, SPELL_OPPORTUNISTIC_STRIKE, true);
                                }
                            }
 
                            if (deathAttack)
                                events.RescheduleEvent(EVENT_DEATH_ATTACKS_START, 3000);
                            break;
                        }
                    }
                }
                if (!deathAttack)
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_generic_imperatorAI(creature);
        }
};

class mob_woi_add_generic : public CreatureScript
{
    public:
        mob_woi_add_generic() : CreatureScript("mob_woi_add_generic") {}

        struct mob_woi_add_genericAI : public ScriptedAI
        {
            mob_woi_add_genericAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
            uint32 focusspell;
            ObjectGuid targetguid;

            void Reset() override
            {
                targetguid.Clear();
                focusspell = 0;
                DoZoneInCombat(me, 150.0f);
                switch (me->GetEntry())
                {
                    case NPC_RAGE:
                        focusspell = SPELL_FOCALISED_ASSAULT;
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                        {
                            DoCast(target, focusspell, true);
                            targetguid = target->GetGUID();
                            AttackStart(target);
                            me->getThreatManager().addThreat(target, 1000000.0f);
                        }
                        me->AddAura(SPELL_WITHOUT_ARMOR, me);
                        break;
                    case NPC_COURAGE:
                        focusspell = SPELL_FOCALISED_DEFENSE;
                        if (Unit* randomBoss = pInstance->instance->GetCreature(pInstance->GetGuidData(urand(0, 1) ? NPC_QIN_XI: NPC_JAN_XI)))
                        {
                            if (Unit* tank = randomBoss->getVictim())
                            {
                                DoCast(tank, focusspell, true);
                                targetguid = tank->GetGUID();
                                AttackStart(tank);
                                me->getThreatManager().addThreat(tank, 1000000.0f);
                            }
                        }
                        else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                        {
                            target->AddAura(focusspell, target);
                            targetguid = target->GetGUID();
                            AttackStart(target);
                            me->getThreatManager().addThreat(target, 1000000.0f);                           
                        }
                        DoCast(me, SPELL_HALF_PLATE, true);
                        DoCast(me, SPELL_PHALANX_WALL, true);
                        events.RescheduleEvent(EVENT_IMPEDING_THRUST, 5000);
                        break;
                    case NPC_FORCE:
                        DoCast(me, SPELL_FULL_PLATE, true);
                        events.RescheduleEvent(EVENT_ENERGIZING_SMASH, 6000);
                        break;
                }
                events.RescheduleEvent(EVENT_CHECK_TARGET, 1500);
            }
            
            void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
            {
                if (me->GetEntry() == NPC_COURAGE)
                    if (me->isInFront(attacker))
                        damage = 0;

                if (damage >= me->GetHealth())
                {
                   if (me->GetEntry() == NPC_RAGE || me->GetEntry() == NPC_COURAGE)
                   {
                       if (Unit* target = me->GetUnit(*me, targetguid))
                       {
                           if (target->isAlive())
                               target->RemoveAurasDueToSpell(focusspell);
                       }
                   }
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                if (me->GetMap()->IsHeroic())
                    DoCast(me, SPELL_SUMMON_TITAN_SPARK, true);
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
                        case EVENT_CHECK_TARGET: //All adds, check pursuit target
                            if (!me->getVictim() || !me->getVictim()->isAlive())
                            {
                                switch (me->GetEntry())
                                {
                                case NPC_RAGE:
                                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                                    {
                                        target->AddAura(focusspell, target);
                                        targetguid = target->GetGUID();
                                        AttackStart(target);
                                        me->getThreatManager().addThreat(target, 1000000.0f);
                                    }
                                    break;
                                case NPC_FORCE:
                                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                                    {
                                        AttackStart(target);
                                        me->getThreatManager().addThreat(target, 1000000.0f);
                                    }
                                    break;
                                case NPC_COURAGE:
                                    if (Unit* randomBoss = pInstance->instance->GetCreature(pInstance->GetGuidData(urand(0, 1) ? NPC_QIN_XI: NPC_JAN_XI)))
                                    {
                                        if (Unit* tank = randomBoss->getVictim())
                                        {
                                            tank->AddAura(focusspell, tank);
                                            targetguid = tank->GetGUID();
                                            AttackStart(tank);
                                            me->getThreatManager().addThreat(tank, 1000000.0f);
                                        }
                                    }
                                    else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                                    {
                                        target->AddAura(focusspell, target);
                                        targetguid = target->GetGUID();
                                        AttackStart(target);
                                        me->getThreatManager().addThreat(target, 1000000.0f);
                                    }
                                    events.RescheduleEvent(EVENT_IMPEDING_THRUST, 2000);
                                    break;
                                }
                            }
                            events.RescheduleEvent(EVENT_CHECK_TARGET, 2000);
                            break;
                        case EVENT_IMPEDING_THRUST: // Courage
                            if (me->getVictim())
                            {
                                if (me->IsWithinMeleeRange(me->getVictim()))
                                {
                                    DoCast(me->getVictim(), SPELL_IMPEDING_THRUST);
                                    events.RescheduleEvent(EVENT_IMPEDING_THRUST, 10000);
                                }
                                else
                                    events.RescheduleEvent(EVENT_IMPEDING_THRUST, 3000);
                            }
                            break;
                        case EVENT_ENERGIZING_SMASH: // Strenght
                            DoCast(me, SPELL_ENERGIZING_SMASH);
                            events.RescheduleEvent(EVENT_ENERGIZING_SMASH, 6000);
                            break;
                    }
                }
                if (me->GetEntry() != NPC_FORCE)
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_woi_add_genericAI(creature);
        }
};

//67221, 60575
class npc_emperor_terracotta_boss : public CreatureScript
{
    public:
        npc_emperor_terracotta_boss() : CreatureScript("npc_emperor_terracotta_boss") {}

        struct npc_emperor_terracotta_bossAI : public ScriptedAI
        {
            npc_emperor_terracotta_bossAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* pInstance;

            void Reset() override {}

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                damage = 0;
            }

            void SpellHit(Unit* caster, const SpellInfo* spell)
            {
                switch (spell->Id)
                {
                    case SPELL_ARC_RIGHT:
                        me->SetFacingTo(caster->GetOrientation() - 1.5f);
                        me->CastSpell(me, SPELL_DEVASTATING_ARC_RIGHT, TriggerCastFlags(TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE));
                        break;
                    case SPELL_ARC_LEFT:
                        me->SetFacingTo(caster->GetOrientation() + 1.5f);
                        me->CastSpell(me, SPELL_DEVASTATING_ARC_LEFT, TriggerCastFlags(TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE));
                        break;
                    case SPELL_ARC_CENTER:
                        me->CastSpell(me, SPELL_DEVASTATING_ARC_FRONT, TriggerCastFlags(TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE));
                        break;
                }
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                switch (spell->Id)
                {
                    case SPELL_DEVASTATING_ARC_RIGHT:
                    case SPELL_DEVASTATING_ARC_LEFT:
                    case SPELL_DEVASTATING_ARC_FRONT:
                        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                            if (Creature* emperor = summoner->ToCreature())
                                if (Unit* tank = emperor->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO))
                                    if (target == tank)
                                        emperor->AI()->DoAction(ACTION_HIT_DEATH_ATTACK);
                        break;
                }
            }

            void UpdateAI(uint32 diff) override {}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_emperor_terracotta_bossAI(creature);
        }
};

//60480
class npc_emperor_titan_spark : public CreatureScript
{
    public:
        npc_emperor_titan_spark() : CreatureScript("npc_emperor_titan_spark") {}

        struct npc_emperor_titan_sparkAI : public ScriptedAI
        {
            npc_emperor_titan_sparkAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISTRACT, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* pInstance;
            EventMap events;
            bool active;
            bool explosion;

            void Reset() override {}
            
            void IsSummonedBy(Unit* summoner)
            {
                active = false;
                explosion = false;
                DoCast(me, SPELL_ENERGY_VISUAL, true);
                events.RescheduleEvent(EVENT_1, 3000); //Active
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_FOCALISED_ENERGY)
                {
                    DoZoneInCombat(me, 100.0f);
                    me->AddThreat(target, 100000.0f);
                }
                if (spell->Id == 117766)
                    events.RescheduleEvent(EVENT_2, 1000); //Despawn
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;

                    if (!explosion)
                    {
                        explosion = true;
                        me->StopAttack();
                        me->StopMoving();
                        DoCast(117766); //Explosion
                        events.RescheduleEvent(EVENT_2, 1000); //Despawn
                    }
                }
            }

            void UpdateAI(uint32 diff) override 
            {
                if (!UpdateVictim() && active)
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                            active = true;
                            DoCast(me, SPELL_FOCALISED_ENERGY, true);
                            DoCast(me, SPELL_ENERGY_OF_CREATION, true);
                            break;
                        case EVENT_2:
                            me->DespawnOrUnsummon();
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_emperor_titan_sparkAI(creature);
        }
};

class ExactDistanceCheck
{
    public:
        ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) { }

        bool operator()(WorldObject* unit)
        {
            return _source->GetExactDist2d(unit) > _dist;
        }

    private:
        Unit* _source;
        float _dist;
};

//116550
class spell_eperor_energizing_smash : public SpellScriptLoader
{
    public:
        spell_eperor_energizing_smash() : SpellScriptLoader("spell_eperor_energizing_smash") { }

        class spell_eperor_energizing_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eperor_energizing_smash_SpellScript);

            uint8 stacks;

            bool Load()
            {
                stacks = 0;
                return true;
            }

            void ResizeEffectRadiusTargetChecker(std::list<WorldObject*>& targets)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Aura* aura = caster->GetAura(113314);
                if (!aura)
                    stacks = 0;
                else
                    stacks = aura->GetStackAmount();

                targets.remove_if(ExactDistanceCheck(caster, 10.0f + stacks));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eperor_energizing_smash_SpellScript::ResizeEffectRadiusTargetChecker, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eperor_energizing_smash_SpellScript::ResizeEffectRadiusTargetChecker, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_eperor_energizing_smash_SpellScript();
        }
};

void AddSC_boss_will_of_emperor()
{
    new npc_woi_controller();
    new boss_generic_imperator();
    new mob_woi_add_generic();
    new npc_emperor_terracotta_boss();
    new npc_emperor_titan_spark();
    new spell_eperor_energizing_smash();
}
