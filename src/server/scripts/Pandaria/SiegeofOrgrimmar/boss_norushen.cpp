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

#include "CreatureTextMgr.h"
#include "ScriptedEscortAI.h"
#include "siege_of_orgrimmar.h"

enum eSpells
{
    SPELL_VISUAL_TELEPORT           = 149634,
    SPELL_VISUAL_TELEPORT_AC        = 145188,
    SPELL_EXTRACT_CORRUPTION        = 145143,
    SPELL_EXTRACT_CORRUPTION_S      = 145149,

    //Amalgam_of_Corruption
    SPELL_SPAWN_AMALGAM             = 145118,
    SPELL_CORRUPTION                = 144421, 
    SPELL_ICY_FEAR                  = 145733,
    SPELL_UNLEASHED_ANGER           = 145214,
    SPELL_UNLEASHED_ANGER_DMG       = 145212,
    SPELL_UNCHECKED_CORRUPTION      = 145679,
    SPELL_SELF_DOUBT                = 146124,
    SPELL_QUARANTINE_SAFETY         = 145779,
    SPELL_FRAYED                    = 146179,
    SPELL_UNLEASH_CORRUPTION        = 145769,

    //Blind Hatred
    SPELL_BLIND_HATRED              = 145571,
    SPELL_BLIND_HATRED_V            = 145226,
    SPELL_BLIND_HATRED_D            = 145227,

    // Frayed. Manifestation of corruption.
    SPELL_BURST_OF_ANGER            = 147082,

    //RESIDUAL_CORRUPTION
    SPELL_CORRUPTION_AREA           = 145052,

    //Manifestation of Corruption
    SPELL_TEAR_REALITY              = 144482,
    SPELL_RESIDUAL_CORRUPTION       = 145074,

    //Essence of Corruption
    SPELL_EXPEL_CORRUPTION_AT       = 144548, //Create areatrigger
    SPELL_EXPELLED_CORRUPTION       = 144480,

    // TC
    SPELL_TC_CLEANSE                = 147657,

    //Cleanse
    SPELL_CLEANSE_15                = 144449,//clean 15 corruption
    SPELL_CLEANSE_40                = 144450,//clean 40 corruption
    SPELL_CLEANSE_100               = 147657,//clean 100 corruption

    /*   C H A L L E N GE*/
    SPELL_PURIFIED_CHALLENGE        = 146022,
    SPELL_PURIFIED                  = 144452,

    SPELL_BOTTOMLESS_PIT_AT         = 146793,
    SPELL_BOTTOMLESS_PIT_DMG        = 146703,

    //Test for players
    SPELL_TEST_OF_SERENITY          = 144849, //dd
    SPELL_TEST_OF_RELIANCE          = 144850, //heal
    SPELL_TEST_OF_CONFIDENCE        = 144851, //tank

    //Phase spells
    SPELL_LOOK_WITHIN_DD            = 146837,
    SPELL_LOOK_WITHIN_HEALER        = 144724,
    SPELL_LOOK_WITHIN_TANK          = 144727, //Look Within

    //DD
    SPELL_SPAWN_VICTORY_ORB_DD_BIG  = 144491, //Spawn Victory Orb summon NPC_MANIFESTATION_OF_CORRUPTION in world
    SPELL_SPAWN_VICTORY_ORB_DD_SML  = 145006/*144490*/, //Spawn Victory Orb summon NPC_ESSENCE_OF_CORRUPTION in world

    SPELL_SUM_ESSENCE_OF_CORRUPT_C  = 144733, //Summon NPC_ESSENCE_OF_CORRUPTION_C by Player
    SPELL_SUM_MANIFESTATION_OF_C    = 144739, //Summon NPC_MANIFESTATION_OF_CORRUPTION_C by Player

    //Tank
    SPELL_TITANIC_CORRUPTION        = 144848, //Titanic Corruption

    //Healer
    SPELL_GREATER_CORRUPTION        = 144980, //Greater Corruption
    SPELL_MELEE_COMBTANT            = 144975, //Melee Combatant
    SPELL_CASTER                    = 144977,
    SPELL_SUMMON_GUARDIAN           = 144973, //Summon Guardian
    SPELL_PROTECTORS_EXHAUSTED      = 148424,
    SPELL_PROTECTORS_DD             = 144521,
};

enum sEvents
{
    //Amalgam_of_Corruption
    EVENT_CHECK_VICTIM         = 1,
    EVENT_QUARANTINE_SAFETY    = 2, 
    EVENT_UNLEASHED_ANGER      = 3,
    EVENT_BLIND_HATRED         = 4,

    //Blind Hatred
    EVENT_GET_NEW_POS          = 5,

    //Manifestation of Corruption
    EVENT_TEAR_REALITY         = 6,

    //Titanic Corruption
    EVENT_BURST_OF_CORRUPTION  = 7,
    EVENT_CORRUPTION_TC        = 8,
    EVENT_HURL_CORRUPTION      = 9,
    EVENT_PIERCING_CORRUPTION  = 10,
    EVENT_TITANIC_SMASH        = 11,

    //Essence of Corruption
    EVENT_EXPELLED_CORRUPTION  = 12,
    
    EVENT_RE_ATTACK            = 13,
    EVENT_START_ROTATE         = 14,
    EVENT_FIRST_POINT          = 15,
    EVENT_UPDATE_POINT         = 16,
};

enum sData
{
    //Blind Hatred
    DATA_START_MOVING          = 1,
    DATA_FILL_MOVE_ORDER       = 2,
};

enum sAction
{
    //Blind Hatred
    ACTION_START_EVENT            = 1,
    //Residual corruption
    ACTION_DESPAWN                = 2,

    ACTION_ESCAPE_FROM_VOID_ZONE  = 3,
};

Position eofcpos[4] =
{
    {761.4254f, 982.592f,  356.3398f},
    {766.5504f, 960.6927f, 356.3398f},
    {787.5417f, 987.2986f, 356.3398f},
    {793.2588f, 967.8085f, 356.3400f},
};

Position const plspos[5] =  //purifying light spawn pos
{
    {805.18f, 956.49f, 356.3400f},
    {760.49f, 946.19f, 356.3398f},
    {746.27f, 985.05f, 356.3398f},
    {771.05f, 1006.36f, 356.8000f},
    {805.67f, 991.16f, 356.3400f},
};

//Blind Hatred
Position const BlindHatred[4] =
{
    { 808.897f, 1023.77f, 356.3f},
    { 728.585f, 1006.259f,356.3f},
    { 748.132f, 911.165f, 356.3f},
    { 828.936f, 929.101f, 356.3f},
};

uint32 combatnpc[3] =
{
    NPC_NN_HEAL_EVENT_PROTECTOR_1,
    NPC_NN_HEAL_EVENT_PROTECTOR_2,
    NPC_NN_HEAL_EVENT_PROTECTOR_3,
};

uint32 const challengeauras[3] =
{
    SPELL_TEST_OF_SERENITY,
    SPELL_TEST_OF_RELIANCE,
    SPELL_TEST_OF_CONFIDENCE,
};

typedef std::list<uint8> BlindOrderList;
float const radius = 38.0f;
Position const Norushen  = {767.6754f, 1015.564f, 356.1747f, 4.922687f };
Position const Amalgan  = {777.3924f, 974.2292f, 356.3398f, 1.786108f };

//71967
class boss_norushen : public CreatureScript
{
public:
    boss_norushen() : CreatureScript("boss_norushen") {}
    
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->CLOSE_GOSSIP_MENU();
        if (action)
            creature->AI()->DoAction(true);
        return true;
    }
    
    struct boss_norushenAI : public ScriptedAI
    {
        boss_norushenAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void DoAction(int32 const action)
        {
            if (instance && instance->GetBossState(DATA_NORUSHEN) != DONE)
            {
                me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, 0);
                //continue from EVENT_13
                uint32 t = 0;
                events.RescheduleEvent(EVENT_1, t += 0);          //18:23:14.000
                events.RescheduleEvent(EVENT_2, t += 3000);       //18:23:22.000
                events.RescheduleEvent(EVENT_3, t += 11000);      //18:23:32.000
                events.RescheduleEvent(EVENT_4, t += 2000);       //18:23:34.000
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                    me->CastSpell(me, SPELL_VISUAL_TELEPORT, false);
                    me->CastSpell(me, SPELL_VISUAL_TELEPORT_AC, true);
                    ZoneTalk(eventId + 6, me->GetGUID());
                    instance->SetData(DATA_CLOSE_ZONE_NORUSHEN, 0);
                    break;
                case EVENT_2:
                    me->SetFacingTo(1.791488f);
                    ZoneTalk(eventId + 6, me->GetGUID());
                    DoCast(me, SPELL_EXTRACT_CORRUPTION);
                    break;
                case EVENT_3:
                    DoCast(me, SPELL_EXTRACT_CORRUPTION_S);
                    break;
                case EVENT_4:
                    ZoneTalk(TEXT_GENERIC_9, me->GetGUID());
                    break;
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_norushenAI(creature);
    }
};

//72872
class npc_norushen_lowerwalker : public CreatureScript
{
public:
    npc_norushen_lowerwalker() : CreatureScript("npc_norushen_lowerwalker") { }

    enum phases
    {
        PHASE_EVENT     = 1,
    };

    struct npc_norushen_lowerwalkerAI : public npc_escortAI
    {
        npc_norushen_lowerwalkerAI(Creature* creature) : npc_escortAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid norushGUID;

        void Reset()
        {
            norushGUID.Clear();
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (events.IsInPhase(PHASE_EVENT))
                return;

            Start(false, false);
            events.SetPhase(PHASE_EVENT);
            uint32 t = 0;
            events.RescheduleEvent(EVENT_1, t += 1000);    //18:20:50.000
            events.RescheduleEvent(EVENT_2, t += 6000);    //18:20:56.000
            events.RescheduleEvent(EVENT_3, t += 8000);    //18:21:04.000
            events.RescheduleEvent(EVENT_4, t += 8000);    //18:21:11.000
            events.RescheduleEvent(EVENT_5, t += 2000);    //18:21:13.000
            events.RescheduleEvent(EVENT_6, t += 7000);    //18:21:20.000
            events.RescheduleEvent(EVENT_7, t += 7000);    //18:21:27.000
            events.RescheduleEvent(EVENT_8, t += 8000);    //18:21:35.000
            events.RescheduleEvent(EVENT_9, t += 5000);    //18:21:40.000
            events.RescheduleEvent(EVENT_10, t += 3000);   //18:21:43.000
            events.RescheduleEvent(EVENT_11, t += 14000);  //18:21:56.000
            events.RescheduleEvent(EVENT_12, t += 8000);   //18:22:04.000
            events.RescheduleEvent(EVENT_13, t += 10000);  //18:22:14.000
            events.RescheduleEvent(EVENT_14, t += 1);
        }

        void WaypointReached(uint32 i)
        {
            if (i == 2)
                SetEscortPaused(true);
        }

        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_1:
                case EVENT_2:
                case EVENT_3:
                    ZoneTalk(eventId - 1, me->GetGUID());
                    break;
                case EVENT_4:
                    if (Creature* norush = instance->instance->SummonCreature(NPC_NORUSHEN, Norushen))
                    {
                        norush->AI()->ZoneTalk(TEXT_GENERIC_0, me->GetGUID());
                        norushGUID = norush->GetGUID();
                    }
                    break;
                case EVENT_5:
                    ZoneTalk(TEXT_GENERIC_3, me->GetGUID());
                    break;
                case EVENT_6:
                    if (Creature* norush = instance->instance->GetCreature(norushGUID))
                        norush->AI()->ZoneTalk(TEXT_GENERIC_1, me->GetGUID());
                    break;
                case EVENT_7:
                    ZoneTalk(TEXT_GENERIC_4, me->GetGUID());
                    break;
                case EVENT_8:
                    if (Creature* norush = instance->instance->GetCreature(norushGUID))
                        norush->AI()->ZoneTalk(TEXT_GENERIC_2, me->GetGUID());
                    break;
                case EVENT_9:
                    ZoneTalk(TEXT_GENERIC_5, me->GetGUID());
                    break;
                case EVENT_10:
                case EVENT_11:
                case EVENT_12:
                case EVENT_13:
                    if (Creature* norush = instance->instance->GetCreature(norushGUID))
                        norush->AI()->ZoneTalk(eventId - 7, me->GetGUID());
                    break;
                case EVENT_14:
                    if (Creature* norush = instance->instance->GetCreature(norushGUID))
                        norush->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_norushen_lowerwalkerAI(creature);
    }
};

//72276
class boss_amalgam_of_corruption : public CreatureScript
{
public:
    boss_amalgam_of_corruption() : CreatureScript("boss_amalgam_of_corruption") {}
    
    struct boss_amalgam_of_corruptionAI : public BossAI
    {
        boss_amalgam_of_corruptionAI(Creature* creature) : BossAI(creature, DATA_NORUSHEN)
        {
            instance = creature->GetInstanceScript();
            SetCombatMovement(false);
        }
        InstanceScript* instance;
        uint8 FrayedCounter;

        void Reset()
        {
            _Reset();
            DespawnOwerSummons();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PURIFIED);
            me->RemoveAllAuras();
            me->SetReactState(REACT_DEFENSIVE);
            me->ModifyAuraState(AURA_STATE_UNKNOWN22, true);
            ApplyOrRemoveBar(false);
            FrayedCounter = 0;
            if (Creature* HealChGreater = me->FindNearestCreature(NPC_GREATER_CORRUPTION, 200.0f))
                me->Kill(HealChGreater);
        }

        void DespawnOwerSummons()
        {
            std::list<Creature*>addlist;
            addlist.clear();
            GetCreatureListWithEntryInGrid(addlist, me, NPC_RESIDUAL_CORRUPTION, 150.0f);
            GetCreatureListWithEntryInGrid(addlist, me, NPC_ESSENCE_OF_CORRUPTION, 150.0f);
            if (!addlist.empty())
                for (std::list<Creature*>::const_iterator itr = addlist.begin(); itr != addlist.end(); itr++)
                    (*itr)->DespawnOrUnsummon();
        }

        void IsSummonedBy(Unit* summoner)
        {
            DoZoneInCombat(me, 150.0f);
        }

        void ApplyOrRemoveBar(bool state)
        {
            Map::PlayerList const &players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
            {
                if (Player* pl = i->getSource())
                {
                    if (pl->isAlive() && state)
                        pl->AddAura(SPELL_CORRUPTION, pl);
                    else
                        pl->RemoveAurasDueToSpell(SPELL_CORRUPTION);
                }
            }
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            me->AddAura(SPELL_ICY_FEAR, me);
            ApplyOrRemoveBar(true);
            events.RescheduleEvent(EVENT_CHECK_VICTIM, 2000);
            events.RescheduleEvent(EVENT_UNLEASHED_ANGER, 11000);
            events.RescheduleEvent(EVENT_QUARANTINE_SAFETY, 420000);
            events.RescheduleEvent(EVENT_BLIND_HATRED, 12000);
            me->CastSpell(me, SPELL_SPAWN_AMALGAM, true);
            for (uint8 i = 0; i < 5; ++i)
                me->SummonCreature(NPC_PURIFYING_LIGHT, plspos[i].GetPositionX(), plspos[i].GetPositionY(), plspos[i].GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!attacker->HasAura(SPELL_CORRUPTION))
            {
                damage = 0;
                return;
            }
            damage = CalculatePct(damage, 125 - attacker->GetPower(POWER_ALTERNATE));
            if (HealthBelowPct(50) && damage < me->GetHealth())
            {
                if (HealthBelowPct(50 - 10 * FrayedCounter))
                {
                    if (!me->HasAura(SPELL_FRAYED))
                        me->AddAura(SPELL_FRAYED, me);
                    ++FrayedCounter;
                    DoCast(me, SPELL_UNLEASH_CORRUPTION);
                }
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            ApplyOrRemoveBar(false);
            DespawnOwerSummons();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PURIFIED);
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
                case EVENT_CHECK_VICTIM:
                    if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                        DoCastAOE(SPELL_UNCHECKED_CORRUPTION);
                    events.RescheduleEvent(EVENT_CHECK_VICTIM, 2000);
                    break;
                case EVENT_QUARANTINE_SAFETY:
                    me->CastSpell(me, SPELL_QUARANTINE_SAFETY, true);
                    EnterEvadeMode();
                    break;
                case EVENT_UNLEASHED_ANGER:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_UNLEASHED_ANGER);
                    events.RescheduleEvent(EVENT_UNLEASHED_ANGER, 11000);
                    break;
                case EVENT_BLIND_HATRED:
                    DoCast(me, SPELL_BLIND_HATRED);
                    events.RescheduleEvent(EVENT_BLIND_HATRED, 60000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_amalgam_of_corruptionAI(creature);
    }
};

//72595
class npc_blind_hatred_base : public CreatureScript
{
public:
    npc_blind_hatred_base() : CreatureScript("npc_blind_hatred_base") { }

    struct npc_blind_hatred_baseAI : public ScriptedAI
    {
        npc_blind_hatred_baseAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
            bhGuid.Clear();
            dist = 0;
        }
        EventMap events;
        ObjectGuid bhGuid;
        float dist;

        void Reset()
        {
            events.Reset();
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            bhGuid = guid;
            if (Creature* bh = me->GetCreature(*me, bhGuid))
            {
                dist = me->GetDistance(bh);
                events.RescheduleEvent(EVENT_START_ROTATE, 3000);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_START_ROTATE:
                {
                    uint8 rand = urand(0, 1);
                    me->GetMotionMaster()->MoveRotate(50000, rand ? ROTATE_DIRECTION_RIGHT : ROTATE_DIRECTION_LEFT);
                    events.RescheduleEvent(EVENT_FIRST_POINT, 500);
                    break;
                }
                case EVENT_FIRST_POINT:
                    if (Creature* bh = me->GetCreature(*me, bhGuid))
                    {
                        float x, y;
                        GetPositionWithDistInOrientation(me, dist, me->GetOrientation(), x, y);
                        bh->GetMotionMaster()->MoveJump(x, y, bh->GetPositionZ(), 7.0f, 0.0f, 1);
                    }
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_blind_hatred_baseAI(pCreature);
    }
};

//72565 
class npc_blind_hatred : public CreatureScript
{
public:
    npc_blind_hatred() : CreatureScript("npc_blind_hatred") { }

    struct npc_blind_hatredAI : public ScriptedAI
    {
        npc_blind_hatredAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
            bhbaseGuid.Clear();
            dist = 0;
        }
        InstanceScript* pInstance;
        EventMap events;
        ObjectGuid bhbaseGuid;
        float dist;

        void Reset()
        {
            events.Reset();
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            bhbaseGuid = guid;
            if (Creature* bhbase = me->GetCreature(*me, bhbaseGuid))
            {
                dist = me->GetDistance(bhbase);
                events.RescheduleEvent(EVENT_START_ROTATE, 1000);
            }
        }

        void EnterEvadeMode(){}

        void EnterCombat(Unit* who){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE || type == POINT_MOTION_TYPE)
                if (pointId == 1)
                    events.RescheduleEvent(EVENT_UPDATE_POINT, 100);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_UPDATE_POINT)
                {
                    if (Creature* bhbase = me->GetCreature(*me, bhbaseGuid))
                    {
                        float x, y;
                        GetPositionWithDistInOrientation(bhbase, dist, bhbase->GetOrientation(), x, y);
                        me->GetMotionMaster()->MoveJump(x, y, me->GetPositionZ(), 7.0f, 0.0f, 1);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_blind_hatredAI(pCreature);
    }
};

class npc_norushen_purifying_light : public CreatureScript
{
public:
    npc_norushen_purifying_light() : CreatureScript("npc_norushen_purifying_light") { }

    struct npc_norushen_purifying_lightAI : public CreatureAI
    {
        npc_norushen_purifying_lightAI(Creature* pCreature) : CreatureAI(pCreature)
        {
            pInstance = (InstanceScript*)pCreature->GetInstanceScript();
        }
        InstanceScript* pInstance;

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void OnSpellClick(Unit* clicker)
        {
            if (Player* p = clicker->ToPlayer())
            {
                if (p->GetPower(POWER_ALTERNATE))
                {
                    switch (p->GetSpecializationRole())
                    {
                        case ROLES_TANK:
                            p->CastSpell(p, SPELL_TEST_OF_CONFIDENCE, false);
                            break;
                        case ROLES_DPS:
                            p->CastSpell(p, SPELL_TEST_OF_SERENITY, false);
                            break;
                        case ROLES_HEALER:
                            p->CastSpell(p, SPELL_TEST_OF_RELIANCE, false);
                            break;
                        default:
                            p->CastSpell(p, SPELL_TEST_OF_SERENITY, false);
                            TC_LOG_ERROR(LOG_FILTER_PLAYER, "Script::npc_norushen_purifying_light: Player %s has not localized role specID.", p->ToString().c_str());
                            break;
                    }
                }
            }
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff){}     
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_purifying_lightAI(pCreature);
    }
};

//72550
class npc_norushen_residual_corruption : public CreatureScript
{
public:
    npc_norushen_residual_corruption() : CreatureScript("npc_norushen_residual_corruption") { }

    struct npc_norushen_residual_corruptionAI : public ScriptedAI
    {
        npc_norushen_residual_corruptionAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            SetCombatMovement(false);
        }

        InstanceScript* instance;

        void Reset() 
        { 
            DoCast(me, SPELL_RESIDUAL_CORRUPTION);
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_DESPAWN)
            {
                if (AreaTrigger* at = me->GetAreaObject(SPELL_RESIDUAL_CORRUPTION))
                    at->Despawn();
                me->DespawnOrUnsummon();
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_norushen_residual_corruptionAI(creature);
    }
};

//71976 for dd
class npc_essence_of_corruption_challenge : public CreatureScript
{
public:
    npc_essence_of_corruption_challenge() : CreatureScript("npc_essence_of_corruption_challenge") { }

    struct npc_essence_of_corruptionAI : public ScriptedAI
    {
        npc_essence_of_corruptionAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            SetCombatMovement(false);
            instance = pCreature->GetInstanceScript();
        }
        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;
       
        enum spell
        {
            SPELL_ESSENCE_OF_CORUPTION          = 148452,   //Essence of Corruption
            SPELL_STEALTH_DETECTION             = 8279,     //Stealth Detection
            SPELL_EXPEL_CORRUPTUIN              = 144479,   //Expel Corruption
            SPELL_EXPELED_CORRUPTION            = 144480,
        };

        void IsSummonedBy(Unit* summoner)
        {
            targetGuid = summoner->ToPlayer() ? summoner->GetGUID() : ObjectGuid::Empty;
            me->SetReactState(REACT_PASSIVE);
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->CastSpell(me, SPELL_ESSENCE_OF_CORUPTION, false);
            me->CastSpell(me, SPELL_STEALTH_DETECTION, true);
            AttackStart(summoner);
            events.RescheduleEvent(EVENT_2, urand(3000, 6000));
        }

        void EnterEvadeMode()
        {
            if (Creature* amalgam = me->GetCreature(*me, instance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
                amalgam->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_SPAWN_VICTORY_ORB_DD_SML);
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* killer)
        {
            if (Player* plr = me->GetPlayer(*me, targetGuid))
                if (plr->isAlive())
                    plr->CastSpell(plr, SPELL_CLEANSE_15, true);

            if (Creature* amalgam = me->GetCreature(*me, instance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
                amalgam->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_SPAWN_VICTORY_ORB_DD_SML);
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
                if (eventId == EVENT_2)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 60.0f, true))
                        me->CastSpell(target, SPELL_EXPEL_CORRUPTUIN);
                    events.RescheduleEvent(EVENT_2, 9000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_essence_of_corruptionAI(pCreature);
    }
};

//71977 for dd
class npc_norushen_manifestation_of_corruption_challenge : public CreatureScript
{
public:
    npc_norushen_manifestation_of_corruption_challenge() : CreatureScript("npc_norushen_manifestation_of_corruption_challenge") { }

    struct npc_norushen_manifestation_of_corruption_challengeAI : public ScriptedAI
    {
        npc_norushen_manifestation_of_corruption_challengeAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            attack = 0;
            active = false;
        }
        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;
        uint32 attack;
        bool active;

        enum spells
        {
            SPELL_STEALTH_AND_INVISIBILITY_DETECT = 141048,
        };

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_TEAR_REALITY, 8500);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!active)
                damage = 0;
        }

        void EnterEvadeMode()
        {
            if (Creature* amalgam = me->GetCreature(*me, instance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
                amalgam->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_SPAWN_VICTORY_ORB_DD_BIG);
            me->DespawnOrUnsummon();
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_DEFENSIVE);
            targetGuid = summoner->ToPlayer() ? summoner->GetGUID() : ObjectGuid::Empty;
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->CastSpell(me, SPELL_STEALTH_AND_INVISIBILITY_DETECT, true);
            attack = 3000;
        }

        void JustDied(Unit* killer)
        {
            if (Player* plr = me->GetPlayer(*me, targetGuid))
                if (plr->isAlive())
                    plr->CastSpell(plr, SPELL_CLEANSE_40, true);

            if (Creature* amalgam = me->GetCreature(*me, instance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
                amalgam->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_SPAWN_VICTORY_ORB_DD_BIG);
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (attack)
            {
                if (attack <= diff)
                {
                    attack = 0;
                    active = true;
                    me->SetFullHealth();
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                }
                else
                    attack -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {   
                if (eventId == EVENT_TEAR_REALITY)
                {
                    me->AttackStop();
                    DoCastAOE(SPELL_TEAR_REALITY);
                    events.RescheduleEvent(EVENT_TEAR_REALITY, 8500);
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_manifestation_of_corruption_challengeAI(pCreature);
    }
};

//72264
class npc_norushen_manifestation_of_corruption_released : public CreatureScript
{
public:
    npc_norushen_manifestation_of_corruption_released() : CreatureScript("npc_norushen_manifestation_of_corruption_released") { }

    struct npc_norushen_manifestation_of_corruption_releasedAI : public ScriptedAI
    {
        npc_norushen_manifestation_of_corruption_releasedAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            pInstance = (InstanceScript*)pCreature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;

        enum sp
        {
            SPELL_UNLEASHED                              = 146173,
            SPELL_UNLEASHED_0_EFFECT_PROCK               = 148974,
        };

        void Reset()
        {
            events.Reset();

            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetPower(POWER_ENERGY, 100);
        }
        
        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_1, 5000);
        }

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->CastSpell(me, SPELL_UNLEASHED, false);
            me->SetInCombatWithZone();
            if (Creature* amalgam = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
                amalgam->CastSpell(me, SPELL_UNLEASHED_0_EFFECT_PROCK, false);
        }

        void JustDied(Unit* killer)
        {
            if (Creature* n = me->FindNearestCreature(NPC_NORUSHEN, 150.0f, true))
                if (Creature* rc = n->SummonCreature(NPC_RESIDUAL_CORRUPTION, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                    rc->CastSpell(rc, SPELL_RESIDUAL_CORRUPTION, true);
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
                if (eventId == EVENT_1)
                {
                    DoCastAOE(SPELL_BURST_OF_ANGER);
                    events.RescheduleEvent(EVENT_1, 5000);
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_manifestation_of_corruption_releasedAI(pCreature);
    }
};

//72263
class npc_essence_of_corruption_released : public CreatureScript
{
public:
    npc_essence_of_corruption_released() : CreatureScript("npc_essence_of_corruption_released") { }

    struct npc_essence_of_corruption_releasedAI : public ScriptedAI
    {
        npc_essence_of_corruption_releasedAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = (InstanceScript*)pCreature->GetInstanceScript();
            SetCombatMovement(false);
        }
       
        enum spell
        {
            SPELL_UNLEASHED                     = 146174,
            SPELL_UNLEASHED_0_EFFECT_PROCK      = 148974,
            SPELL_STEALTH_DETECTION             = 8279,     //Stealth Detection
            SPELL_EXPEL_CORRUPTION              = 145064,   //145132 on friend | 145134 on enemy
        };

        InstanceScript* instance;
        EventMap events;

        void Reset(){}

        void IsSummonedBy(Unit* /*summoner*/)
        {
            me->CastSpell(me, SPELL_UNLEASHED, false);
            me->CastSpell(me, SPELL_STEALTH_DETECTION, false);
            if (Creature* amalgam = instance->instance->GetCreature(instance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
            {
                amalgam->CastSpell(me, SPELL_UNLEASHED_0_EFFECT_PROCK, false);
                me->SetFacingToObject(amalgam);
            }
            me->CastSpell(me, SPELL_EXPEL_CORRUPTION, true);
            events.RescheduleEvent(EVENT_1, 6*IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_1)
                {
                    if (Creature* amalgam = instance->instance->GetCreature(instance->GetGuidData(NPC_AMALGAM_OF_CORRUPTION)))
                        me->SetFacingToObject(amalgam);
                    me->CastSpell(me, SPELL_EXPEL_CORRUPTION, false);
                    events.RescheduleEvent(EVENT_1, 6 * IN_MILLISECONDS);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_essence_of_corruption_releasedAI(pCreature);
    }
};

//72051 for tank
class npc_titanic_corruption : public CreatureScript
{
public:
    npc_titanic_corruption() : CreatureScript("npc_titanic_corruption") { }

    struct npc_titanic_corruptionAI : public ScriptedAI
    {
        npc_titanic_corruptionAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            attack = 0;
            active = false;
        }

        enum spells
        {
            SPELL_BURST_OF_CORRUPTION       = 144654,
            SPELL_CORRUPTION_TC             = 144639,
            SPELL_HURL_CORRUPTION           = 144649,
            SPELL_PIERCING_CORRUPTION       = 144657,
            SPELL_TITANIC_SMASH             = 144628,
        };

        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;
        uint32 attack;
        bool active;

        void Reset()
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            targetGuid = summoner->ToPlayer() ? summoner->GetGUID() : ObjectGuid::Empty;
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->DespawnOrUnsummon(60000);
            attack = 3000;
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!active)
                damage = 0;
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_PIERCING_CORRUPTION, 14000);
            events.RescheduleEvent(EVENT_TITANIC_SMASH, 16000);
            events.RescheduleEvent(EVENT_HURL_CORRUPTION, 20000);
            events.RescheduleEvent(EVENT_BURST_OF_CORRUPTION, 35000);
        }

        void JustDied(Unit* killer)
        {
            if (Player* pl = me->GetPlayer(*me, targetGuid))
                if (pl->isAlive())
                    pl->CastSpell(pl, SPELL_CLEANSE_100, true);
            me->DespawnOrUnsummon();
        }
        
        void UpdateAI(uint32 diff)
        {
            if (attack)
            {
                if (attack <= diff)
                {
                    attack = 0;
                    active = true;
                    me->SetFullHealth();
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                }
                else
                    attack -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_PIERCING_CORRUPTION:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_PIERCING_CORRUPTION);
                    events.RescheduleEvent(EVENT_PIERCING_CORRUPTION, 14000);
                    break;
                case EVENT_TITANIC_SMASH:
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    DoCastAOE(SPELL_TITANIC_SMASH);
                    events.RescheduleEvent(EVENT_RE_ATTACK, 1000);
                    events.RescheduleEvent(EVENT_TITANIC_SMASH, 16000);
                    break;
                case EVENT_HURL_CORRUPTION:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_HURL_CORRUPTION);
                    events.RescheduleEvent(EVENT_HURL_CORRUPTION, 20000);
                    break;
                case EVENT_BURST_OF_CORRUPTION:
                    DoCastAOE(SPELL_BURST_OF_CORRUPTION);
                    events.RescheduleEvent(EVENT_BURST_OF_CORRUPTION, 35000);
                    break;
                case EVENT_RE_ATTACK:
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 75.0f);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_titanic_corruptionAI(pCreature);
    }
};

//72001 for healers
class npc_norushen_heal_ch_greater_corruption : public CreatureScript
{
public:
    npc_norushen_heal_ch_greater_corruption() : CreatureScript("npc_norushen_heal_ch_greater_corruption") { }

    struct npc_norushen_heal_ch_greater_corruptionAI : public ScriptedAI
    {
        npc_norushen_heal_ch_greater_corruptionAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetVisible(false);
            attack = 0;
            active = false;
        }

        enum spells
        {
            SPELL_DISHEARTENING_LAUGH        = 146707,
            SPELL_LINGERING_CORRUPTION       = 144514,
        };

        enum events
        {
            EVENT_SPELL_DISHEARTENING_LAUGH  = 1,
            EVENT_SPELL_LINGERING_CORRUPTION = 2,
        };

        InstanceScript* pInstance;
        EventMap events;
        ObjectGuid targetGuid;
        uint32 attack;
        bool active;

        void Reset()
        {
            events.Reset();
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!active)
                damage = 0;
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_PASSIVE);
            targetGuid = summoner->ToPlayer() ? summoner->GetGUID() : ObjectGuid::Empty;
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->SetVisible(true);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            attack = 3000;
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_SPELL_DISHEARTENING_LAUGH, 12000);
            events.RescheduleEvent(EVENT_SPELL_LINGERING_CORRUPTION, 14000);
        }

        void JustDied(Unit* killer)
        {
            if (Player* plr = me->GetPlayer(*me, targetGuid))
                if (plr->isAlive())
                    plr->CastSpell(plr, SPELL_CLEANSE_100, true);
            me->DespawnOrUnsummon();
        }

        void StartCombatWithCreatures()
        {
            std::list<Creature*> npclist;
            npclist.clear();
            for (uint8 n = 0; n < 3; n++)
                if (Creature* battlenpc = me->FindNearestCreature(combatnpc[n], 100.0f, true))
                    npclist.push_back(battlenpc);

            if (!npclist.empty())
            {
                me->SetReactState(REACT_AGGRESSIVE);
                for (std::list<Creature*>::const_iterator itr = npclist.begin(); itr != npclist.end(); itr++)
                {
                    (*itr)->Attack(me, true);
                    if ((*itr)->GetEntry() != NPC_NN_HEAL_EVENT_PROTECTOR_2)
                        (*itr)->GetMotionMaster()->MoveChase(me);
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (attack)
            {
                if (attack <= diff)
                {
                    attack = 0;
                    active = true;
                    me->SetFullHealth();
                    StartCombatWithCreatures();
                }
                else
                    attack -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SPELL_DISHEARTENING_LAUGH:
                    DoCast(me, SPELL_DISHEARTENING_LAUGH);
                    events.RescheduleEvent(EVENT_SPELL_DISHEARTENING_LAUGH, 12000);
                    break;
                case EVENT_SPELL_LINGERING_CORRUPTION:
                {
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (Creature* battlenpc = me->FindNearestCreature(combatnpc[n], 100.0f, true))
                        {
                            if (!battlenpc->HasAura(SPELL_LINGERING_CORRUPTION))
                            {
                                DoCast(battlenpc, SPELL_LINGERING_CORRUPTION, true);
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_SPELL_LINGERING_CORRUPTION, 14000);
                }
                break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_heal_ch_greater_corruptionAI(pCreature);
    }
};

//71996
class npc_norushen_heal_ch_melee_combtant : public CreatureScript
{
public:
    npc_norushen_heal_ch_melee_combtant() : CreatureScript("npc_norushen_heal_ch_melee_combtant") { }

    struct npc_norushen_heal_ch_melee_combtantAI : public ScriptedAI
    {
        npc_norushen_heal_ch_melee_combtantAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetVisible(false);
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_DEFENSIVE);
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->SetVisible(true);
            DoCast(me, SPELL_PROTECTORS_DD);
            me->SetHealth(me->GetMaxHealth() / 2);
            //if (summoner->ToPlayer())
            //    summoner->ToPlayer()->SendEncounterUnitForPlayer(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(30) && !me->HasAura(SPELL_PROTECTORS_EXHAUSTED))
                DoCast(me, SPELL_PROTECTORS_EXHAUSTED, true);

            if (damage >= me->GetHealth())
                damage = 0;
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_ESCAPE_FROM_VOID_ZONE)
            {
                Position pos;
                me->GetNearPosition(pos, 5.0f, 5.3f);
                me->GetMotionMaster()->MoveCharge(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 3.0f);
            }
        }

        void HealReceived(Unit* /*done_by*/, uint32& addhealth)
        {
            float newpct = GetHealthPctWithHeal(addhealth);
            if (newpct > 30.0f && me->HasAura(SPELL_PROTECTORS_EXHAUSTED))
                me->RemoveAurasDueToSpell(SPELL_PROTECTORS_EXHAUSTED);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (IsInDisable())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_heal_ch_melee_combtantAI(pCreature);
    }
};

//72000
class npc_norushen_heal_ch_caster : public CreatureScript
{
public:
    npc_norushen_heal_ch_caster() : CreatureScript("npc_norushen_heal_ch_caster") { }

    struct npc_norushen_heal_ch_casterAI : public ScriptedAI
    {
        npc_norushen_heal_ch_casterAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetVisible(false);
        }

        EventMap events;

        enum spells
        {
            SPELL_FIREBALL = 144522,
        };

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_DEFENSIVE);
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->SetVisible(true);
            DoCast(me, SPELL_PROTECTORS_DD);
            me->SetHealth(me->GetMaxHealth() / 2);
            //if (summoner->ToPlayer())
            //    summoner->ToPlayer()->SendEncounterUnitForPlayer(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_ESCAPE_FROM_VOID_ZONE)
            {
                Position pos;
                me->GetNearPosition(pos, 5.0f, 6.0f);
                me->GetMotionMaster()->MoveCharge(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 3.0f);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(30) && !me->HasAura(SPELL_PROTECTORS_EXHAUSTED))
                DoCast(me, SPELL_PROTECTORS_EXHAUSTED, true);

            if (damage >= me->GetHealth())
                damage = 0;
        }

        void HealReceived(Unit* /*done_by*/, uint32& addhealth)
        {
            float newpct = GetHealthPctWithHeal(addhealth);
            if (newpct > 30.0f && me->HasAura(SPELL_PROTECTORS_EXHAUSTED))
                me->RemoveAurasDueToSpell(SPELL_PROTECTORS_EXHAUSTED);
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_2, 1500);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (IsInDisable())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_2)
                {
                    if (Creature* gc = me->FindNearestCreature(NPC_GREATER_CORRUPTION, 100.0f, true))
                        DoCast(gc, SPELL_FIREBALL);
                    events.RescheduleEvent(EVENT_2, 4000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_heal_ch_casterAI(pCreature);
    }
};

//71995
class npc_norushen_heal_ch_guardian : public CreatureScript
{
public:
    npc_norushen_heal_ch_guardian() : CreatureScript("npc_norushen_heal_ch_guardian") { }

    struct npc_norushen_heal_ch_guardianAI : public ScriptedAI
    {
        npc_norushen_heal_ch_guardianAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetVisible(false);
        }

        EventMap events;

        enum spells
        {
            SPELL_THREATENING_STRIKE = 144527,
        };

        void IsSummonedBy(Unit* summoner)
        {
            me->SetReactState(REACT_DEFENSIVE);
            //me->SetPhaseId(summoner->GetGUID(), true);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            me->SetVisible(true);
            me->SetHealth(me->GetMaxHealth() / 2);
            //if (summoner->ToPlayer())
            //    summoner->ToPlayer()->SendEncounterUnitForPlayer(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_2, 5000);
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_ESCAPE_FROM_VOID_ZONE)
            {
                Position pos;
                me->GetNearPosition(pos, 5.0f, 5.3f);
                me->GetMotionMaster()->MoveCharge(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 3.0f);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(30) && !me->HasAura(SPELL_PROTECTORS_EXHAUSTED))
                DoCast(me, SPELL_PROTECTORS_EXHAUSTED, true);

            if (damage >= me->GetHealth())
                damage = 0;
        }

        void HealReceived(Unit* /*done_by*/, uint32& addhealth)
        {
            float newpct = GetHealthPctWithHeal(addhealth);
            if (newpct > 30.0f && me->HasAura(SPELL_PROTECTORS_EXHAUSTED))
                me->RemoveAurasDueToSpell(SPELL_PROTECTORS_EXHAUSTED);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (IsInDisable())
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_2)
                {
                    if (Creature* gc = me->FindNearestCreature(NPC_GREATER_CORRUPTION, 100.0f, true))
                        DoCast(gc, SPELL_THREATENING_STRIKE);
                    events.RescheduleEvent(EVENT_2, 3000);
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_norushen_heal_ch_guardianAI(pCreature);
    }
};

//145571
class spell_norushen_blind_hatred : public SpellScriptLoader
{
public:
    spell_norushen_blind_hatred() : SpellScriptLoader("spell_norushen_blind_hatred") { }
    
    class spell_norushen_blind_hatred_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_norushen_blind_hatred_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster())
            {
                uint8 mod = urand(0, 3);
                if (Creature* bhbase = GetCaster()->SummonCreature(NPC_BLIND_HATRED_BASE, GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ() + 2.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000))
                {
                    if (Creature* bh = GetCaster()->SummonCreature(NPC_BLIND_HATRED, BlindHatred[mod].GetPositionX(), BlindHatred[mod].GetPositionY(), BlindHatred[mod].GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 32000))
                    {
                        bhbase->SetFacingToObject(bh);
                        bhbase->AI()->SetGUID(bh->GetGUID(), 1);
                        bh->AI()->SetGUID(bhbase->GetGUID(), 1);
                        bhbase->CastSpell(bh, SPELL_BLIND_HATRED_V, true);
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_norushen_blind_hatred_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };
    
    SpellScript* GetSpellScript() const override
    {
        return new spell_norushen_blind_hatred_SpellScript();
    }
};

//145573
class spell_blind_hatred_periodic : public SpellScriptLoader
{
public:
    spell_blind_hatred_periodic() : SpellScriptLoader("spell_blind_hatred_periodic") { }

    class spell_blind_hatred_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_blind_hatred_periodic_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_BLIND_HATRED_D, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_blind_hatred_periodic_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_blind_hatred_periodic_AuraScript();
    }
};

class ChallengeFilterTargets
{
public:
    bool operator()(WorldObject* target)
    {
        if (Unit* unit = target->ToUnit())
            for (uint8 n = 0; n < 3; n++)
                if (unit->HasAura(challengeauras[n]))
                    return true;
        return false;
    }
};

class BlindHatredDmgSelector
{
public:
    BlindHatredDmgSelector(Unit* caster, Creature* blindhatred) : _caster(caster), _blindhatred(blindhatred) {}
    
    bool operator()(WorldObject* target)
    {
        if (Unit* unit = target->ToUnit())
            if (unit->IsInBetween(_caster, _blindhatred))
                return false;
        return true;
    }
private:
    Unit* _caster;
    Creature* _blindhatred;
};

//145227
class spell_norushen_blind_hatred_prock : public SpellScriptLoader
{
public:
    spell_norushen_blind_hatred_prock() : SpellScriptLoader("spell_blind_hatred") { }
    
    class spell_norushen_blind_hatred_prock_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_norushen_blind_hatred_prock_SpellScript);

        void FilterTargets(std::list<WorldObject*>&unitList)
        {
            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                unitList.remove_if(ChallengeFilterTargets());
                if (Creature* bh = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(NPC_BLIND_HATRED)))
                    unitList.remove_if(BlindHatredDmgSelector(GetCaster(), bh));
                else
                    unitList.clear();
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_norushen_blind_hatred_prock_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_norushen_blind_hatred_prock_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_norushen_blind_hatred_prock_SpellScript();
    }
};

//145216
class spell_unleashed_anger : public SpellScriptLoader
{
public:
    spell_unleashed_anger() : SpellScriptLoader("spell_unleashed_anger") { }
    
    class spell_unleashed_anger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_unleashed_anger_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                GetCaster()->CastSpell(GetHitUnit(), SPELL_UNLEASHED_ANGER_DMG, true);
                GetCaster()->AddAura(SPELL_SELF_DOUBT, GetHitUnit());
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_unleashed_anger_SpellScript::DealDamage);
        }
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_unleashed_anger_SpellScript();
    }
};

//145735
class spell_icy_fear_dmg : public SpellScriptLoader
{
public:
    spell_icy_fear_dmg() : SpellScriptLoader("spell_icy_fear_dmg") { }
    
    class spell_icy_fear_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_icy_fear_dmg_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                uint32 mod = 1200;
                uint32 pctmod = 100 - (uint32)floor(GetCaster()->GetHealthPct());
                SetHitDamage(GetHitDamage() + mod*pctmod);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_icy_fear_dmg_SpellScript::DealDamage);
        }
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_icy_fear_dmg_SpellScript();
    }
};

//144849 144850 144851
class spell_norushen_challenge : public SpellScriptLoader
{
public:
    spell_norushen_challenge() : SpellScriptLoader("spell_norushen_challenge") { }

    class spell_norushen_challenge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_norushen_challenge_AuraScript);

        ObjectGuid eventGUID;

        uint32 getPhaseSpell()
        {
            switch (GetId())
            {
            case SPELL_TEST_OF_SERENITY:    //dd
                return SPELL_LOOK_WITHIN_DD;
            case SPELL_TEST_OF_RELIANCE:    //heal
                return SPELL_LOOK_WITHIN_HEALER;
            case SPELL_TEST_OF_CONFIDENCE:  //tank
                return SPELL_LOOK_WITHIN_TANK;
            }
            return 0;
        }

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            PreventDefaultAction();
            if (GetTarget())
            { 
                if (InstanceScript* instance = GetTarget()->GetInstanceScript())
                {
                    if (Creature* norush = GetTarget()->GetCreature(*GetTarget(), instance->GetGuidData(NPC_NORUSHEN)))
                        norush->AI()->ZoneTalk(TEXT_GENERIC_10, GetTarget()->GetGUID());
                    GetTarget()->CastSpell(GetTarget(), getPhaseSpell(), true);
                    if (Player* player = GetTarget()->ToPlayer())
                    {
                        player->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_DUEL, player->GetGUID());
                        player->UpdateObjectVisibility();
                    }
                    switch (GetId())
                    {
                    case SPELL_TEST_OF_SERENITY:    //dd
                    {
                        GetTarget()->SummonCreature(NPC_MANIFESTATION_OF_CORRUPTION_C, 777.5012f, 974.7348f, 356.3398f, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 2000, GetTarget()->GetGUID());
                        int32 plpower = GetTarget()->GetPower(POWER_ALTERNATE);
                        if (plpower > 40 && plpower <= 55)
                            GetTarget()->SummonCreature(NPC_ESSENCE_OF_CORRUPTION_C, eofcpos[0].GetPositionX(), eofcpos[0].GetPositionY(), eofcpos[0].GetPositionZ(), 0.0f, TEMPSUMMON_DEAD_DESPAWN, 2000, GetTarget()->GetGUID());
                        else if (plpower > 55 && plpower <= 70)
                            for (uint8 n = 0; n < 2; n++)
                                GetTarget()->SummonCreature(NPC_ESSENCE_OF_CORRUPTION_C, eofcpos[n].GetPositionX(), eofcpos[n].GetPositionY(), eofcpos[n].GetPositionZ(), 0.0f, TEMPSUMMON_DEAD_DESPAWN, 2000, GetTarget()->GetGUID());
                        else if (plpower > 70 && plpower <= 85)
                            for (uint8 n = 0; n < 3; n++)
                                GetTarget()->SummonCreature(NPC_ESSENCE_OF_CORRUPTION_C, eofcpos[n].GetPositionX(), eofcpos[n].GetPositionY(), eofcpos[n].GetPositionZ(), 0.0f, TEMPSUMMON_DEAD_DESPAWN, 2000, GetTarget()->GetGUID());
                        else if (plpower > 85 && plpower <= 100)
                            for (uint8 n = 0; n < 4; n++)
                                GetTarget()->SummonCreature(NPC_ESSENCE_OF_CORRUPTION_C, eofcpos[n].GetPositionX(), eofcpos[n].GetPositionY(), eofcpos[n].GetPositionZ(), 0.0f, TEMPSUMMON_DEAD_DESPAWN, 2000, GetTarget()->GetGUID());
                        break;
                    }
                    case SPELL_TEST_OF_RELIANCE:    //heal
                        GetTarget()->CastSpell(777.5012f, 974.7348f, 356.3398f, SPELL_GREATER_CORRUPTION);
                        GetTarget()->CastSpell(789.889f, 958.021f, 356.34f, SPELL_MELEE_COMBTANT);
                        GetTarget()->CastSpell(772.854f, 947.467f, 356.34f, SPELL_CASTER);
                        GetTarget()->CastSpell(780.8785f, 974.7535f, 356.34f, SPELL_SUMMON_GUARDIAN);
                        break;
                    case SPELL_TEST_OF_CONFIDENCE:  //tank
                        GetTarget()->SummonCreature(NPC_TITANIC_CORRUPTION, 777.5012f, 974.7348f, 356.3398f, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 2000, GetTarget()->GetGUID());
                        break;
                    }
                }
            }
        }

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (GetTarget())
                if (!GetTarget()->GetPower(POWER_ALTERNATE))
                    GetTarget()->RemoveAurasDueToSpell(GetId(), ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget())
            {
                GetTarget()->RemoveAurasDueToSpell(getPhaseSpell());
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE ||
                    GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEFAULT)
                {
                    if (Player* pl = GetTarget()->ToPlayer())
                    {
                        switch (pl->GetSpecializationRole())
                        {
                            case ROLES_HEALER:
                                pl->RemoveAllMinionsByFilter(NPC_GREATER_CORRUPTION);
                                pl->RemoveAllMinionsByFilter(NPC_NN_HEAL_EVENT_PROTECTOR_1);
                                pl->RemoveAllMinionsByFilter(NPC_NN_HEAL_EVENT_PROTECTOR_2);
                                pl->RemoveAllMinionsByFilter(NPC_NN_HEAL_EVENT_PROTECTOR_3);
                                break;
                            case ROLES_TANK:
                                pl->RemoveAllMinionsByFilter(NPC_TITANIC_CORRUPTION);
                                break;
                            default:
                                break;
                        }
                    }
                }
                else if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_CANCEL)
                {
                    if (Player* pl = GetTarget()->ToPlayer())
                    {
                        pl->CastSpell(pl, SPELL_PURIFIED_CHALLENGE, true);
                        switch (pl->GetSpecializationRole())
                        {
                            case ROLES_HEALER:
                                pl->RemoveAllMinionsByFilter(NPC_GREATER_CORRUPTION);
                                pl->RemoveAllMinionsByFilter(NPC_NN_HEAL_EVENT_PROTECTOR_1);
                                pl->RemoveAllMinionsByFilter(NPC_NN_HEAL_EVENT_PROTECTOR_2);
                                pl->RemoveAllMinionsByFilter(NPC_NN_HEAL_EVENT_PROTECTOR_3);
                                pl->CastCustomSpell(SPELL_PURIFIED, SPELLVALUE_BASE_POINT0, 25.0f, pl, true);
                                break;
                            case ROLES_TANK:
                                pl->RemoveAllMinionsByFilter(NPC_TITANIC_CORRUPTION);
                                pl->CastCustomSpell(SPELL_PURIFIED, SPELLVALUE_BASE_POINT1, -25.0f, pl, true);
                                break;
                            case ROLES_DPS:
                                pl->RemoveAllMinionsByFilter(NPC_MANIFESTATION_OF_CORRUPTION_C);
                                pl->RemoveAllMinionsByFilter(NPC_ESSENCE_OF_CORRUPTION_C);
                                pl->CastCustomSpell(SPELL_PURIFIED, SPELLVALUE_BASE_POINT3, 25.0f, pl, true);
                                break;
                            default:
                                break;
                        }
                    }
                }
                if (Player* player = GetTarget()->ToPlayer())
                {
                    player->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_DUEL, 0);
                    player->UpdateObjectVisibility();
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_norushen_challenge_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_norushen_challenge_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_norushen_challenge_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_norushen_challenge_AuraScript();
    }
};

//144521
class spell_norushen_heal_test_dd : public SpellScriptLoader
{
public:
    spell_norushen_heal_test_dd() : SpellScriptLoader("spell_norushen_heal_test_dd") { }
    
    class spell_norushen_heal_test_dd_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_norushen_heal_test_dd_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                //ToDo: posible it's dinamically damage modificator.
                if (AuraEffect* aurEff = GetCaster()->GetAuraEffect(GetSpellInfo()->Id, EFFECT_2))
                {
                    if (aurEff->GetAmount() == 300)
                        return;

                    if (AuraApplication * aurApp = GetAura()->GetApplicationOfTarget(GetCasterGUID()))
                    {
                        aurEff->HandleEffect(aurApp, AURA_EFFECT_HANDLE_REAL, false);
                        aurEff->SetAmount(300);
                        aurEff->HandleEffect(aurApp, AURA_EFFECT_HANDLE_REAL, true);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_norushen_heal_test_dd_AuraScript::OnTick, EFFECT_3, SPELL_AURA_PERIODIC_DUMMY);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_norushen_heal_test_dd_AuraScript();
    }
};

class spell_essence_expel_corruption : public SpellScriptLoader
{
public:
    spell_essence_expel_corruption() : SpellScriptLoader("spell_essence_expel_corruption") { }
    
    class spell_essence_expel_corruption_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_essence_expel_corruption_SpellScript);

        void HandleAfterCast()
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* ptarget = GetExplTargetUnit())
                {
                    caster->SetFacingToObject(ptarget);
                    caster->AttackStop();
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_essence_expel_corruption_SpellScript::HandleAfterCast);
        }
    };
    
    SpellScript* GetSpellScript() const
    {
        return new spell_essence_expel_corruption_SpellScript();
    }
};

class BurstOfAngerFilter
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (Player* pl = unit->ToPlayer())
            if (!pl->HasAura(SPELL_TEST_OF_SERENITY) && !pl->HasAura(SPELL_TEST_OF_RELIANCE) && !pl->HasAura(SPELL_TEST_OF_CONFIDENCE))
                return false;
        return true;
    }
};

//147082
class spell_burst_of_anger : public SpellScriptLoader
{
public:
    spell_burst_of_anger() : SpellScriptLoader("spell_burst_of_anger") { }

    class spell_burst_of_anger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_burst_of_anger_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (!targets.empty())
                targets.remove_if(BurstOfAngerFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_burst_of_anger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_burst_of_anger_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_burst_of_anger_SpellScript();
    }
};

//145052
class spell_corrupt : public SpellScriptLoader
{
public:
    spell_corrupt() : SpellScriptLoader("spell_corrupt") { }

    class spell_corrupt_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corrupt_SpellScript);

        void _HandleHit()
        {
            if (GetCaster() && GetHitUnit())
            {
                if (GetHitUnit()->GetPower(POWER_ALTERNATE) + 25 <= 100)
                {
                    GetHitUnit()->SetPower(POWER_ALTERNATE, GetHitUnit()->GetPower(POWER_ALTERNATE) + 25);
                    GetHitUnit()->RemoveAurasDueToSpell(SPELL_PURIFIED);
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_DESPAWN);
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_corrupt_SpellScript::_HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_corrupt_SpellScript();
    }
};

//144514
class spell_lingering_corruption : public SpellScriptLoader
{
public:
    spell_lingering_corruption() : SpellScriptLoader("spell_lingering_corruption") { }

    class spell_lingering_corruption_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lingering_corruption_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                GetCaster()->CastSpell(GetTarget(), SPELL_BOTTOMLESS_PIT_AT, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_lingering_corruption_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_lingering_corruption_AuraScript();
    }
};


//146703
class spell_bottomless_pit : public SpellScriptLoader
{
public:
    spell_bottomless_pit() : SpellScriptLoader("spell_bottomless_pit") { }

    class spell_bottomless_pit_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_bottomless_pit_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->ToCreature())
                GetTarget()->ToCreature()->AI()->DoAction(ACTION_ESCAPE_FROM_VOID_ZONE);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_bottomless_pit_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_bottomless_pit_AuraScript();
    }
};

void AddSC_boss_norushen()
{
    new boss_norushen();
    new npc_norushen_lowerwalker();
    new boss_amalgam_of_corruption();
    new npc_blind_hatred_base();
    new npc_blind_hatred();
    new npc_norushen_purifying_light();
    new npc_norushen_residual_corruption();
    new npc_essence_of_corruption_challenge();
    new npc_norushen_manifestation_of_corruption_challenge();
    new npc_norushen_manifestation_of_corruption_released();
    new npc_essence_of_corruption_released();
    new npc_titanic_corruption();
    new npc_norushen_heal_ch_greater_corruption();
    new npc_norushen_heal_ch_melee_combtant();
    new npc_norushen_heal_ch_caster();
    new npc_norushen_heal_ch_guardian();
    new spell_norushen_blind_hatred();
    new spell_blind_hatred_periodic();
    new spell_norushen_blind_hatred_prock();
    new spell_unleashed_anger();
    new spell_icy_fear_dmg();
    new spell_norushen_challenge();
    new spell_norushen_heal_test_dd();
    new spell_essence_expel_corruption();
    new spell_burst_of_anger();
    new spell_corrupt();
    new spell_lingering_corruption();
    new spell_bottomless_pit();
}