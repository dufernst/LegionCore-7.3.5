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

    // Quiang
    SPELL_FLANKING_ORDERS       = 117910, // Also when vanquished
    SPELL_MASSIVE_ATTACKS       = 117920,
    SPELL_ANNIHILATE            = 117948,
    SPELL_IMPERVIOUS_SHIELD     = 117961, // Heroic

    // Subetai
    SPELL_PILLAGE               = 118049,
    SPELL_VOLLEY_SUM_TARGET     = 118088,
    SPELL_VOLLEY_VISUAL         = 118100,
    SPELL_VOLLEY_1              = 118094,
    SPELL_VOLLEY_2              = 118105,
    SPELL_VOLLEY_3              = 118106,
    SPELL_RAIN_OF_ARROWS        = 118121,
    SPELL_SLEIGHT_OF_HAND       = 118162, // Heroic

    // Zian
    SPELL_UNDYING_SHADOWS       = 117504, // Also when vanquished
    SPELL_FIXATE                = 118303,
    SPELL_UNDYING_SHADOW_DOT    = 117514,
    SPELL_COALESCING_SHADOW_DOT = 117539,

    SPELL_SHADOW_BLAST          = 117628,
    SPELL_CHARGED_SHADOWS       = 117684,
    SPELL_SHIELD_OF_DARKNESS    = 117697, // Heroic

    // Meng
    SPELL_ENERGY_DRAIN          = 117707, // Disable regenerate energy
    SPELL_MADDENING_SHOUT       = 117708, // Also when vanquished
    SPELL_CRAZED                = 117737,
    SPELL_COWARDICE             = 117756,
    SPELL_COWARDICE_DMG         = 117829,
    SPELL_CRAZY_TOUGHT          = 117833,
    SPELL_DELIRIOUS             = 117837, // Heroic

    // Shared
    SPELL_INACTIVE              = 118205,
    SPELL_INACTIVE_STUN         = 118319,
    SPELL_BERSERK               = 120207,
    SPELL_NEXT_SPIRIT_VISUAL    = 118861,

    // Flanking Mogu
    SPELL_GHOST_VISUAL          = 117904,
    SPELL_TRIGGER_ATTACK        = 117917,
};

enum eEvents
{
    // Controler
    EVENT_CHECK_WIPE            = 1,

    // Quiang
    EVENT_FLANKING_MOGU         = 2,
    EVENT_MASSIVE_ATTACK        = 3,
    EVENT_ANNIHILATE            = 4,
    EVENT_IMPERVIOUS_SHIELD     = 5,

    // Subetai
    EVENT_PILLAGE               = 6,
    EVENT_VOLLEY                = 7,
    EVENT_SLEIGHT_OF_HAND       = 8,
    EVENT_RAIN_OF_ARROWS        = 9,

    // Zian
    EVENT_UNDYING_SHADOWS       = 10,
    EVENT_SHADOW_BLAST          = 11,
    EVENT_CHARGED_SHADOWS       = 12,
    EVENT_SHIELD_OF_DARKNESS    = 13,

    // Meng
    EVENT_MADDENING_SHOUT       = 14,
    EVENT_CRAZY_TOUGHT          = 15,
    EVENT_DELIRIOUS             = 16
};

// This array need for remove some auras
uint32 spiritKingsEntry[4] =
{
    NPC_QIANG,
    NPC_SUBETAI,
    NPC_ZIAN,
    NPC_MENG
};

uint32 vSpells[3] = 
{
    SPELL_VOLLEY_1,
    SPELL_VOLLEY_2,
    SPELL_VOLLEY_3,
};

class boss_spirit_kings_controler : public CreatureScript
{
    public:
        boss_spirit_kings_controler() : CreatureScript("boss_spirit_kings_controler") {}

        struct boss_spirit_kings_controlerAI : public BossAI
        {
            boss_spirit_kings_controlerAI(Creature* creature) : BossAI(creature, DATA_SPIRIT_KINGS), summons(me)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                me->SetDisplayId(11686);
            }

            InstanceScript* pInstance;
            SummonList summons;
            bool fightInProgress;
            uint32 spiritkings[3]; //Need for Event
            uint32 spiritkingsvirtual[3]; //Need for finish Event

            void Reset() override
            {
                if (pInstance)
                {
                    summons.DespawnAll();
                    pInstance->SetBossState(DATA_SPIRIT_KINGS, NOT_STARTED);
                    fightInProgress = false;
                    
                    for (uint8 n = 0; n < 3; n++)
                        spiritkings[n] = 0;
                    
                    for (uint8 n = 0; n < 3; n++)
                        spiritkingsvirtual[n] = 0;
                }
            }
            
            void EnterCombat(Unit* who) override
            {
                if (pInstance)
                {
                    PushArrayBoss();
                    pInstance->SetBossState(DATA_SPIRIT_KINGS, IN_PROGRESS);
                    fightInProgress = true;
                    events.RescheduleEvent(EVENT_CHECK_WIPE, 1500);
                }
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void PushArrayBoss()
            {
                uint8 pos = urand(0, 5);
                switch (pos)
                {
                case 0:
                    spiritkings[0] = 60710;
                    spiritkings[1] = 60701;
                    spiritkings[2] = 60708;
                    break;
                case 1:
                    spiritkings[0] = 60710;
                    spiritkings[1] = 60708;
                    spiritkings[2] = 60701;
                    break;
                case 2:
                    spiritkings[0] = 60708;
                    spiritkings[1] = 60701;
                    spiritkings[2] = 60710;
                    break;
                case 3:
                    spiritkings[0] = 60708;
                    spiritkings[1] = 60710;
                    spiritkings[2] = 60701;
                    break;
                case 4:
                    spiritkings[0] = 60701;
                    spiritkings[1] = 60708;
                    spiritkings[2] = 60710;
                    break;
                case 5:
                    spiritkings[0] = 60701;
                    spiritkings[1] = 60710;
                    spiritkings[2] = 60708;
                    break;
                }
                spiritkingsvirtual[0] = spiritkings[0];;
                spiritkingsvirtual[1] = spiritkings[1];
                spiritkingsvirtual[2] = NPC_QIANG;
                RestartEvent();
            }

            void RestartEvent()
            {   // Qiang always first
                if (Creature* qiang = me->GetCreature(*me, pInstance->GetGuidData(NPC_QIANG)))
                {
                    if (!qiang->isAlive())
                        qiang->Respawn();
                    
                    qiang->AI()->DoAction(ACTION_START_FIGHT);                   
                }

                for (uint8 n = 0; n < 3; n++)
                {
                    if (Creature* kings = me->GetCreature(*me, pInstance->GetGuidData(spiritkings[n])))
                    {
                        if (!kings->isAlive())
                            kings->Respawn();
                    }
                }

                if (Creature* nspirit = me->GetCreature(*me, pInstance->GetGuidData(spiritkings[0])))
                        nspirit->AddAura(SPELL_NEXT_SPIRIT_VISUAL, nspirit);
            }

            void DoAction(const int32 action) override
            {
                if (!pInstance)
                    return;

                switch (action)
                {
                case ACTION_SPIRIT_KILLED:
                {
                    uint32 nextspirit = 0;
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (spiritkings[n] != 0)
                        {
                            nextspirit = spiritkings[n];
                            if (nextspirit == spiritkings[2])
                            {
                                if (Creature* sp = me->GetCreature(*me, pInstance->GetGuidData(nextspirit)))
                                    sp->AI()->DoAction(ACTION_SPIRIT_LOW_HEALTH);
                            }
                            else
                            {
                                if (Creature* nspirit = me->GetCreature(*me, pInstance->GetGuidData(spiritkings[n + 1])))
                                    nspirit->AddAura(SPELL_NEXT_SPIRIT_VISUAL, nspirit);
                            }
                            spiritkings[n] = 0;
                            break;
                        }
                    }
                    if (nextspirit)
                    {
                        if (Creature* king = me->GetCreature(*me, pInstance->GetGuidData(nextspirit)))
                            king->AI()->DoAction(ACTION_START_FIGHT);
                    }
                }
                break;
                case ACTION_SPIRIT_DONE:
                    for (uint8 i = 0; i < 3; i++)
                    {
                        if (Creature* king = me->GetCreature(*me, pInstance->GetGuidData(spiritkingsvirtual[i])))
                        {
                            if (king->isAlive())
                            {
                                me->Kill(king, true);
                                king->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                            }
                        }
                    }
                    pInstance->SetBossState(DATA_SPIRIT_KINGS, DONE);
                    me->Kill(me, true);
                    break;
                case ACTION_FLANKING_MOGU:
                {
                    float angle = frand(0.0f, 6.0f);
                    float angleMinus = angle;
                    float angleAlt = angle + 3.14f;
                    float angleAltMinus = angle + 3.14f;

                    Position posRand, posRandMinus, posRandAlt, posRandAltMinus;
                    me->GetNearPosition(posRand, 30.0f, angle);
                    me->GetNearPosition(posRandAlt, 30.0f, angleAlt);
                    float orient = posRand.GetAngle(me);
                    float orientAlt = posRandAlt.GetAngle(me);

                    me->SummonCreature(NPC_FLANKING_MOGU, posRand.GetPositionX(), posRand.GetPositionY(), posRand.GetPositionZ(), orient);
                    if (IsHeroic())
                        me->SummonCreature(NPC_FLANKING_MOGU, posRandAlt.GetPositionX(), posRandAlt.GetPositionY(), posRandAlt.GetPositionZ(), orientAlt);
                    for (int8 i = 0; i < 2; i++)
                    {
                        angle += 0.15f;
                        angleMinus -= 0.15f;
                        me->GetNearPosition(posRand, 30.0f, angle);
                        me->GetNearPosition(posRandMinus, 30.0f, angleMinus);
                        me->SummonCreature(NPC_FLANKING_MOGU, posRand.GetPositionX(), posRand.GetPositionY(), posRand.GetPositionZ(), orient);
                        me->SummonCreature(NPC_FLANKING_MOGU, posRandMinus.GetPositionX(), posRandMinus.GetPositionY(), posRandMinus.GetPositionZ(), orient);
                        if (IsHeroic())
                        {
                            angleAlt += 0.15f;
                            angleAltMinus -= 0.15f;
                            me->GetNearPosition(posRandAlt, 30.0f, angleAlt);
                            me->GetNearPosition(posRandAltMinus, 30.0f, angleAltMinus);
                            me->SummonCreature(NPC_FLANKING_MOGU, posRandAlt.GetPositionX(), posRandAlt.GetPositionY(), posRandAlt.GetPositionZ(), orientAlt);
                            me->SummonCreature(NPC_FLANKING_MOGU, posRandAltMinus.GetPositionX(), posRandAltMinus.GetPositionY(), posRandAltMinus.GetPositionZ(), orientAlt);
                        }
                    }
                }
                break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!fightInProgress)
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId ==  EVENT_CHECK_WIPE)
                    {
                        if (pInstance->IsWipe())
                        {
                            for (uint8 n = 0; n < 4; n++)
                                if (Creature* king = me->GetCreature(*me, pInstance->GetGuidData(spiritKingsEntry[n])))
                                    if (king->isAlive() && king->HasAura(SPELL_NEXT_SPIRIT_VISUAL))
                                        king->RemoveAurasDueToSpell(SPELL_NEXT_SPIRIT_VISUAL);
                            EnterEvadeMode();
                        }
                        else
                            events.RescheduleEvent(EVENT_CHECK_WIPE, 1500);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_spirit_kings_controlerAI(creature);
        }
};

class boss_spirit_kings : public CreatureScript
{
    public:
        boss_spirit_kings() : CreatureScript("boss_spirit_kings") {}

        struct boss_spirit_kingsAI : public ScriptedAI
        {
            boss_spirit_kingsAI(Creature* creature) : ScriptedAI(creature), summons(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap   events;
            SummonList summons;
            bool vanquished, lastboss;
            uint8 shadowCount;
            uint8 maxShadowCount;
            uint8 volleyCount;

            void Reset() override
            {
                summons.DespawnAll();
                events.Reset();
                if (me->HasAura(SPELL_NEXT_SPIRIT_VISUAL))
                    me->RemoveAurasDueToSpell(SPELL_NEXT_SPIRIT_VISUAL);
                me->SetReactState(REACT_PASSIVE);
                shadowCount = 0;
                maxShadowCount = 3;
                volleyCount = 0;
                vanquished = false;
                lastboss = false;
                DoCast(me, SPELL_INACTIVE, true);
                if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveAurasDueToSpell(SPELL_CRAZED);
                me->RemoveAurasDueToSpell(SPELL_COWARDICE);
                if (me->GetEntry() == NPC_MENG)
                {
                    DoCast(me, SPELL_ENERGY_DRAIN, true);
                    me->SetPower(POWER_ENERGY, 0);
                }
            }

            Creature* GetControler()
            {
                if (pInstance)
                    return pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_SPIRIT_GUID_CONTROLER)); else return NULL;
            }

            void EnterCombat(Unit* attacker) override
            {
                switch (me->GetEntry())
                {
                    case NPC_QIANG:
                        DoCast(SPELL_MASSIVE_ATTACKS);
                        events.RescheduleEvent(EVENT_FLANKING_MOGU, 30000);
                        events.RescheduleEvent(EVENT_ANNIHILATE, urand(15000, 20000));
                        if (IsHeroic())
                            events.RescheduleEvent(EVENT_IMPERVIOUS_SHIELD, 40000);
                        break;
                    case NPC_SUBETAI:
                        events.RescheduleEvent(EVENT_PILLAGE,             30000);
                        events.RescheduleEvent(EVENT_VOLLEY,              5000);
                        events.RescheduleEvent(EVENT_RAIN_OF_ARROWS,      45000);
                        if (IsHeroic())
                            events.RescheduleEvent(EVENT_SLEIGHT_OF_HAND, 16000);
                        break;
                    case NPC_ZIAN:
                        events.RescheduleEvent(EVENT_UNDYING_SHADOWS,     30000);
                        events.RescheduleEvent(EVENT_SHADOW_BLAST,        15000);
                        events.RescheduleEvent(EVENT_CHARGED_SHADOWS,     10000);
                        if (IsHeroic())
                            events.RescheduleEvent(EVENT_SHIELD_OF_DARKNESS, 40000);
                        break;
                    case NPC_MENG:
                        DoCast(me, SPELL_CRAZED, true);
                        events.RescheduleEvent(EVENT_MADDENING_SHOUT,     30000);
                        events.RescheduleEvent(EVENT_CRAZY_TOUGHT,        10000);
                        if (IsHeroic())
                            events.RescheduleEvent(EVENT_DELIRIOUS,       20000);
                        break;
                }
            }

            void DoAction(const int32 action) override
            {
                switch (action)
                {
                    case ACTION_START_FIGHT:
                        if (me->HasAura(SPELL_NEXT_SPIRIT_VISUAL))
                            me->RemoveAurasDueToSpell(SPELL_NEXT_SPIRIT_VISUAL);
                        me->RemoveAurasDueToSpell(SPELL_INACTIVE);
                        me->setFaction(16);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat(me, 100.0f);
                        break;
                    case ACTION_SPIRIT_LOW_HEALTH: //Last king in event - need JustDied
                        lastboss = true;
                        break;
                }
            }

            void JustDied(Unit* killer) override
            {
                summons.DespawnAll();

                if (Creature* con = GetControler())
                    con->AI()->DoAction(ACTION_SPIRIT_DONE);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (damage >= me->GetHealth() && !lastboss)
                {
                    damage = 0;
                    me->SetHealth(1);
                }

                if (me->HealthBelowPctDamaged(5, damage) && !vanquished && !lastboss)
                {
                    vanquished = true;

                    if (Creature* controler = GetControler())
                        controler->AI()->DoAction(ACTION_SPIRIT_KILLED);

                    me->StopAttack();
                    me->AddAura(SPELL_INACTIVE, me);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    // We reschedule only the vanquished spell
                    events.Reset();
                    switch (me->GetEntry())
                    {
                        case NPC_QIANG:
                            events.RescheduleEvent(EVENT_FLANKING_MOGU, 30000);
                            break;
                        case NPC_SUBETAI:
                            events.RescheduleEvent(EVENT_PILLAGE, 30000);
                            break;
                        case NPC_ZIAN:
                            events.RescheduleEvent(EVENT_UNDYING_SHADOWS, 30000);
                            break;
                        case NPC_MENG:
                            events.RescheduleEvent(EVENT_MADDENING_SHOUT, 30000);
                            break;
                        default:
                            break;
                    }
                }

                if (me->HasAura(SPELL_COWARDICE))
                {
                    if (AuraEffect* effect = me->GetAuraEffect(SPELL_COWARDICE, EFFECT_1))
                    {
                        float amount = effect->GetAmount();
                        float bp1 = damage * amount / 100.f;
                        me->CastCustomSpell(attacker, SPELL_COWARDICE_DMG, &bp1, NULL, NULL, true);
                    }
                }
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_COWARDICE)
                    me->StopAttack();

                if (spell->Id == SPELL_CRAZED && !vanquished)
                    me->SetReactState(REACT_AGGRESSIVE);
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
                    switch(eventId)
                    {
                        // Qiang
                        case EVENT_FLANKING_MOGU:
                            if (Creature* controler = GetControler())
                            {
                                DoCast(me, SPELL_FLANKING_ORDERS);
                                controler->AI()->DoAction(ACTION_FLANKING_MOGU);
                            }
                            events.RescheduleEvent(EVENT_FLANKING_MOGU, 30000);
                            break;
                        case EVENT_ANNIHILATE:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_ANNIHILATE);
                            events.RescheduleEvent(EVENT_ANNIHILATE, urand(15000, 20000));
                            break;
                        case EVENT_IMPERVIOUS_SHIELD:
                            DoCast(SPELL_IMPERVIOUS_SHIELD);
                            events.RescheduleEvent(EVENT_IMPERVIOUS_SHIELD, 60000);
                            break;
                        // Subetai
                        case EVENT_PILLAGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_PILLAGE);
                            events.RescheduleEvent(EVENT_PILLAGE, 30000);
                            break;
                        case EVENT_VOLLEY:
                        {
                            if (!volleyCount)
                            {
                                me->StopAttack();
                                DoCast(SPELL_VOLLEY_SUM_TARGET);
                            }
                            if (volleyCount < 3)
                            {
                                volleyCount++;
                                if (Creature* target = me->FindNearestCreature(NPC_VOLLEY, 100.0f, true))
                                {
                                    me->SetFacingToObject(target);
                                    DoCast(target, SPELL_VOLLEY_VISUAL, true);
                                    DoCast(target, vSpells[volleyCount - 1]);
                                }
                                events.RescheduleEvent(EVENT_VOLLEY, 1000);
                            }
                            else
                            {
                                volleyCount = 0;
                                me->SetReactState(REACT_AGGRESSIVE);
                                events.RescheduleEvent(EVENT_VOLLEY, 38000);
                            }
                            break;
                        }
                        case EVENT_SLEIGHT_OF_HAND:
                            DoCast(SPELL_SLEIGHT_OF_HAND);
                            events.RescheduleEvent(EVENT_SLEIGHT_OF_HAND, 42000);
                            break;
                        case EVENT_RAIN_OF_ARROWS:
                            DoCast(SPELL_RAIN_OF_ARROWS);
                            events.RescheduleEvent(EVENT_RAIN_OF_ARROWS, 45000);
                            break;
                        // Zian
                        case EVENT_UNDYING_SHADOWS:
                            if (shadowCount < maxShadowCount) // Max 3 undying shadow during the fight
                                DoCast(SPELL_UNDYING_SHADOWS);
                            events.RescheduleEvent(EVENT_UNDYING_SHADOWS, 45000);
                            break;
                        case EVENT_SHADOW_BLAST:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                DoCast(target, SPELL_SHADOW_BLAST);
                            events.RescheduleEvent(EVENT_SHADOW_BLAST, 15000);
                            break;
                        case EVENT_CHARGED_SHADOWS:
                            DoCast(SPELL_CHARGED_SHADOWS);
                            events.RescheduleEvent(EVENT_CHARGED_SHADOWS, 15000);
                            break;
                        case EVENT_SHIELD_OF_DARKNESS:
                            DoCast(SPELL_SHIELD_OF_DARKNESS);
                            events.RescheduleEvent(EVENT_SHIELD_OF_DARKNESS, 40000);
                            break;
                        // Meng
                        case EVENT_MADDENING_SHOUT:
                            DoCast(me, SPELL_MADDENING_SHOUT);
                            events.RescheduleEvent(EVENT_MADDENING_SHOUT, 30000);
                            break;
                        case EVENT_CRAZY_TOUGHT:
                            DoCast(me, SPELL_CRAZY_TOUGHT);
                            events.RescheduleEvent(EVENT_CRAZY_TOUGHT, 10000);
                            break;
                        case EVENT_DELIRIOUS:
                            DoCast(SPELL_DELIRIOUS);
                            events.RescheduleEvent(EVENT_DELIRIOUS, 20000);
                            break;
                        default:
                            break;
                    }
                }
                if (!vanquished)
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_spirit_kingsAI(creature);
        }
};

class mob_pinning_arrow : public CreatureScript
{
    public:
        mob_pinning_arrow() : CreatureScript("mob_pinning_arrow") {}

        struct mob_pinning_arrowAI : public ScriptedAI
        {
            mob_pinning_arrowAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            ObjectGuid playerGuid;

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                playerGuid.Clear();
            }

            void SetGUID(ObjectGuid const& guid, int32 /*id*/ = 0)
            {
                playerGuid = guid;

                if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
                    me->AddAura(118141, me); // Pinnig arrow visual
            }

            void JustDied(Unit* attacker) override
            {
                if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
                {
                    player->RemoveAurasDueToSpell(118135); //Aura(stun)
                    me->DespawnOrUnsummon();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_pinning_arrowAI(creature);
        }
};

#define PHASE_UNDYING_SHADOW    true
#define PHASE_COALESCING_SHADOW false

class mob_undying_shadow : public CreatureScript
{
    public:
        mob_undying_shadow() : CreatureScript("mob_undying_shadow") {}

        struct mob_undying_shadowAI : public ScriptedAI
        {
            mob_undying_shadowAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            bool phase;
            uint32 switchPhaseTimer;

            void Reset() override
            {
                me->CastSpell(me, SPELL_UNDYING_SHADOW_DOT, true);
                DoZoneInCombat();

                if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST))
                {
                    me->CastSpell(target, SPELL_FIXATE, true);
                    me->GetMotionMaster()->MoveChase(target);
                }
                switchPhaseTimer = 0;

                phase = PHASE_UNDYING_SHADOW;
            }

            void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
            {
                if (phase == PHASE_UNDYING_SHADOW)
                {
                    if (damage >= me->GetHealth())
                    {
                        me->InterruptNonMeleeSpells(true, SPELL_FIXATE);
                        me->RemoveAurasDueToSpell(SPELL_UNDYING_SHADOW_DOT);
                        DoCast(me, SPELL_COALESCING_SHADOW_DOT, true);
                        me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                        phase = PHASE_COALESCING_SHADOW;
                        switchPhaseTimer = 30000;
                        damage = 0;
                    }
                }
                else
                    damage = 0;
            }

            void UpdateAI(uint32 diff) override
            {
                if (switchPhaseTimer)
                {
                    if (switchPhaseTimer <= diff)
                    {
                        me->RemoveAurasDueToSpell(SPELL_COALESCING_SHADOW_DOT);
                        DoCast(me, SPELL_UNDYING_SHADOW_DOT, true);
                        me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                        me->SetHealth(me->GetMaxHealth());
                        phase = PHASE_UNDYING_SHADOW;
                        switchPhaseTimer = 0;
                        DoZoneInCombat();

                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            me->CastSpell(target, SPELL_FIXATE, true);
                            me->GetMotionMaster()->MoveChase(target);
                        }
                    }
                    else switchPhaseTimer -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_undying_shadowAI(creature);
        }
};

class npc_flanking_mogu : public CreatureScript
{
    public:
        npc_flanking_mogu() : CreatureScript("npc_flanking_mogu") {}

        struct npc_flanking_moguAI : public ScriptedAI
        {
            npc_flanking_moguAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->RemoveAurasDueToSpell(SPELL_GHOST_VISUAL);
                DoCast(SPELL_TRIGGER_ATTACK);
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_TRIGGER_ATTACK)
                {
                    Position pos;
                    me->GetNearPosition(pos, 60.0f, me->GetAngle(me));
                    me->GetMotionMaster()->MovePoint(1, pos);
                }
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (id == 1)
                    me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) override {}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_flanking_moguAI(creature);
        }
};

class spell_pinned_down : public SpellScriptLoader
{
    public:
        spell_pinned_down() : SpellScriptLoader("spell_pinned_down") { }

        class spell_pinned_down_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pinned_down_SpellScript);

            void HandleAfterHit()
            {
                if (GetHitUnit())
                {
                    if (Creature* pinningArrow = GetHitUnit()->SummonCreature(NPC_PINNING_ARROW, GetHitUnit()->GetPositionX(), GetHitUnit()->GetPositionY(), GetHitUnit()->GetPositionZ()))
                        pinningArrow->AI()->SetGUID(GetHitUnit()->GetGUID());
                }
            }

            void Register() override
            {
                AfterHit += SpellHitFn(spell_pinned_down_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pinned_down_SpellScript();
        }
};

//117737, 117756
class spell_meng_crazed : public SpellScriptLoader
{
    public:
        spell_meng_crazed() : SpellScriptLoader("spell_meng_crazed") { }
 
        class spell_meng_crazed_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_meng_crazed_AuraScript)

            void OnPeriodic(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                uint32 power = caster->GetPower(POWER_ENERGY);
                if (power < 100)
                {
                    if (caster->HasAura(SPELL_DELIRIOUS))
                        caster->SetPower(POWER_ENERGY, power + 2);
                    else
                        caster->SetPower(POWER_ENERGY, power + 1);

                    switch (GetId())
                    {
                        case SPELL_CRAZED:
                            if (Aura* aura = caster->GetAura(SPELL_CRAZED))
                                aura->SetStackAmount(aura->GetStackAmount() + 1);
                            break;
                        case SPELL_COWARDICE:
                            if (AuraEffect* effect = GetCaster()->GetAuraEffect(SPELL_COWARDICE, EFFECT_1))
                                effect->SetAmount(power / 2);
                            break;
                    }
                }
                else
                {
                    if (caster->HasAura(SPELL_CRAZED))
                    {
                        caster->RemoveAura(SPELL_CRAZED);
                        caster->CastSpell(caster, SPELL_COWARDICE, true);
                    }
                    else if (caster->HasAura(SPELL_COWARDICE))
                    {
                        caster->RemoveAura(SPELL_COWARDICE);
                        caster->CastSpell(caster, SPELL_CRAZED, true);
                    }

                    caster->SetPower(POWER_ENERGY, 0);
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_meng_crazed_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_meng_crazed_AuraScript();
        }
};

// 117708
class spell_boss_maddening_shout : public SpellScriptLoader
{
    public:
        spell_boss_maddening_shout() : SpellScriptLoader("spell_boss_maddening_shout") { }

        class spell_boss_maddening_shout_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_boss_maddening_shout_AuraScript);

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                if (Unit* target = dmgInfo.GetAttacker())
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        absorbAmount = 0;
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_boss_maddening_shout_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_boss_maddening_shout_AuraScript();
        }
};

void AddSC_boss_spirit_kings()
{
    new boss_spirit_kings_controler();
    new boss_spirit_kings();
    new mob_pinning_arrow();
    new mob_undying_shadow();
    new npc_flanking_mogu();
    new spell_pinned_down();
    new spell_meng_crazed();
    new spell_boss_maddening_shout();
}
