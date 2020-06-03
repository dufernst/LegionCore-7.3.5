/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Vehicle.h"
#include "ObjectMgr.h"
#include "ScriptedEscortAI.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "GameObjectAI.h"

/*######
##Quest 12848
######*/

#define GCD_CAST    1

enum eDeathKnightSpells
{
    SPELL_SOUL_PRISON_CHAIN_SELF    = 54612,
    SPELL_SOUL_PRISON_CHAIN         = 54613,
    SPELL_DK_INITIATE_VISUAL        = 51519,

    SPELL_ICY_TOUCH                 = 52372,
    SPELL_PLAGUE_STRIKE             = 52373,
    SPELL_BLOOD_STRIKE              = 52374,
    SPELL_DEATH_COIL                = 52375
};

#define EVENT_ICY_TOUCH                 1
#define EVENT_PLAGUE_STRIKE             2
#define EVENT_BLOOD_STRIKE              3
#define EVENT_DEATH_COIL                4

//used by 29519, 29520, 29565, 29566, 29567 but signed for 29519

uint32 acherus_soul_prison[12] =
{
    191577,
    191580,
    191581,
    191582,
    191583,
    191584,
    191585,
    191586,
    191587,
    191588,
    191589,
    191590
};

uint32 acherus_unworthy_initiate[5] =
{
    29519,
    29520,
    29565,
    29566,
    29567
};

enum UnworthyInitiatePhase
{
    PHASE_CHAINED,
    PHASE_TO_EQUIP,
    PHASE_EQUIPING,
    PHASE_TO_ATTACK,
    PHASE_ATTACKING,
};

class npc_unworthy_initiate : public CreatureScript
{
public:
    npc_unworthy_initiate() : CreatureScript("npc_unworthy_initiate") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unworthy_initiateAI(creature);
    }

    struct npc_unworthy_initiateAI : public ScriptedAI
    {
        npc_unworthy_initiateAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            if (!me->GetCurrentEquipmentId())
                me->SetCurrentEquipmentId(me->GetOriginalEquipmentId());
        }

        ObjectGuid playerGUID;
        UnworthyInitiatePhase phase;
        uint32 wait_timer;
        float anchorX, anchorY;
        ObjectGuid anchorGUID;

        EventMap events;

        void Reset()
        {
            anchorGUID.Clear();
            phase = PHASE_CHAINED;
            events.Reset();
            me->setFaction(7);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 8);
            me->LoadEquipment(0, true);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.RescheduleEvent(EVENT_ICY_TOUCH, 1000, GCD_CAST);
            events.RescheduleEvent(EVENT_PLAGUE_STRIKE, 3000, GCD_CAST);
            events.RescheduleEvent(EVENT_BLOOD_STRIKE, 2000, GCD_CAST);
            events.RescheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                wait_timer = 5000;
                me->CastSpell(me, SPELL_DK_INITIATE_VISUAL, true);

                if (Player* starter = Unit::GetPlayer(*me, playerGUID))
                    Talk(1, playerGUID);

                phase = PHASE_TO_ATTACK;
            }
        }

        void EventStart(Creature* anchor, Player* target)
        {
            wait_timer = 5000;
            phase = PHASE_TO_EQUIP;

            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            me->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN_SELF);
            me->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN);

            float z;
            anchor->GetContactPoint(me, anchorX, anchorY, z, 1.0f);

            playerGUID = target->GetGUID();
            Talk(0, playerGUID);
        }

        void UpdateAI(uint32 diff)
        {
            switch (phase)
            {
            case PHASE_CHAINED:
                if (!anchorGUID)
                {
                    if (Creature* anchor = me->FindNearestCreature(29521, 30))
                    {
                        anchor->AI()->SetGUID(me->GetGUID());
                        anchor->CastSpell(me, SPELL_SOUL_PRISON_CHAIN, true);
                        anchorGUID = anchor->GetGUID();
                    }
                    else
                        TC_LOG_ERROR(LOG_FILTER_TSCR, "npc_unworthy_initiateAI: unable to find anchor!");

                    float dist = 99.0f;
                    GameObject* prison = NULL;

                    for (uint8 i = 0; i < 12; ++i)
                    {
                        if (GameObject* temp_prison = me->FindNearestGameObject(acherus_soul_prison[i], 30))
                        {
                            if (me->IsWithinDist(temp_prison, dist, false))
                            {
                                dist = me->GetDistance2d(temp_prison);
                                prison = temp_prison;
                            }
                        }
                    }

                    if (prison)
                        prison->ResetDoorOrButton();
                    else
                        TC_LOG_ERROR(LOG_FILTER_TSCR, "npc_unworthy_initiateAI: unable to find prison!");
                }
                break;
            case PHASE_TO_EQUIP:
                if (wait_timer)
                {
                    if (wait_timer > diff)
                        wait_timer -= diff;
                    else
                    {
// there is bug in this part of the script, unworthy initiate drops through
// the floor instead of going to the acherus soul prison..
/*                      me->GetMotionMaster()->MovePoint(1, anchorX, anchorY, me->GetPositionZ());
                        //TC_LOG_DEBUG(LOG_FILTER_TSCR, "npc_unworthy_initiateAI: move to %f %f %f", anchorX, anchorY, me->GetPositionZ());
                        phase = PHASE_EQUIPING;
                        wait_timer = 0;
*/

/*BUG WORKAROUND*/
                        wait_timer = 5000;
                        me->CastSpell(me, SPELL_DK_INITIATE_VISUAL, true);

                        if (Player* starter = Unit::GetPlayer(*me, playerGUID))
                        Talk(1, playerGUID);

                        phase = PHASE_TO_ATTACK;
/*BUG WORKAROUND END*/
                    }
                }
                break;
            case PHASE_TO_ATTACK:
                if (wait_timer)
                {
                    if (wait_timer > diff)
                        wait_timer -= diff;
                    else
                    {
                        me->setFaction(14);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        phase = PHASE_ATTACKING;

                        if (Player* target = Unit::GetPlayer(*me, playerGUID))
                            me->AI()->AttackStart(target);
                        wait_timer = 0;
                    }
                }
                break;
            case PHASE_ATTACKING:
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ICY_TOUCH:
                        DoCast(me->getVictim(), SPELL_ICY_TOUCH);
                        events.DelayEvents(1000, GCD_CAST);
                        events.RescheduleEvent(EVENT_ICY_TOUCH, 5000, GCD_CAST);
                        break;
                    case EVENT_PLAGUE_STRIKE:
                        DoCast(me->getVictim(), SPELL_PLAGUE_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.RescheduleEvent(SPELL_PLAGUE_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_BLOOD_STRIKE:
                        DoCast(me->getVictim(), SPELL_BLOOD_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.RescheduleEvent(EVENT_BLOOD_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_DEATH_COIL:
                        DoCast(me->getVictim(), SPELL_DEATH_COIL);
                        events.DelayEvents(1000, GCD_CAST);
                        events.RescheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
                        break;
                    }
                }

                DoMeleeAttackIfReady();
                break;
            default:
                break;
            }
        }
    };
};

class npc_unworthy_initiate_anchor : public CreatureScript
{
public:
    npc_unworthy_initiate_anchor() : CreatureScript("npc_unworthy_initiate_anchor") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unworthy_initiate_anchorAI(creature);
    }

    struct npc_unworthy_initiate_anchorAI : public PassiveAI
    {
        npc_unworthy_initiate_anchorAI(Creature* creature) : PassiveAI(creature), prisonerGUID() {}

        ObjectGuid prisonerGUID;

        void SetGUID(ObjectGuid const& guid, int32 /*id*/)
        {
            if (!prisonerGUID)
                prisonerGUID = guid;
        }

        ObjectGuid GetGUID(int32 /*id*/)
        {
            return prisonerGUID;
        }
    };
};

class go_acherus_soul_prison : public GameObjectScript
{
public:
    go_acherus_soul_prison() : GameObjectScript("go_acherus_soul_prison") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (Creature* anchor = go->FindNearestCreature(29521, 15))
            if (ObjectGuid prisonerGUID = anchor->AI()->GetGUID())
                if (Creature* prisoner = Creature::GetCreature(*player, prisonerGUID))
                    CAST_AI(npc_unworthy_initiate::npc_unworthy_initiateAI, prisoner->AI())->EventStart(anchor, player);

        return false;
    }

    struct go_acherus_soul_prisonAI : public GameObjectAI
    {
        go_acherus_soul_prisonAI(GameObject* go) : GameObjectAI(go) { }

        bool GossipHello(Player* player) override
        {
            if (player->GetQuestStatus(12848) != QUEST_STATUS_INCOMPLETE || !player->HasItemCount(40732, 1))
                return false;

            if (Creature* anchor = go->FindNearestCreature(29521, 15))
                if (ObjectGuid prisonerGUID = anchor->AI()->GetGUID())
                    if (Creature* prisoner = Creature::GetCreature(*player, prisonerGUID))
                        CAST_AI(npc_unworthy_initiate::npc_unworthy_initiateAI, prisoner->AI())->EventStart(anchor, player);

            return false;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const
    {
        return new go_acherus_soul_prisonAI(go);
    }
};

/*######
## npc_death_knight_initiate
######*/

#define GOSSIP_ACCEPT_DUEL      "I challenge you, death knight!"

enum eDuelEnums
{
    SAY_DUEL_A                  = -1609080,
    SAY_DUEL_B                  = -1609081,
    SAY_DUEL_C                  = -1609082,
    SAY_DUEL_D                  = -1609083,
    SAY_DUEL_E                  = -1609084,
    SAY_DUEL_F                  = -1609085,
    SAY_DUEL_G                  = -1609086,
    SAY_DUEL_H                  = -1609087,
    SAY_DUEL_I                  = -1609088,

    SPELL_DUEL                  = 52996,
    //SPELL_DUEL_TRIGGERED        = 52990,
    SPELL_DUEL_VICTORY          = 52994,
    SPELL_DUEL_FLAG             = 52991,

    QUEST_DEATH_CHALLENGE       = 12733,
    FACTION_HOSTILE             = 2068
};

int32 m_auiRandomSay[] =
{
    SAY_DUEL_A, SAY_DUEL_B, SAY_DUEL_C, SAY_DUEL_D, SAY_DUEL_E, SAY_DUEL_F, SAY_DUEL_G, SAY_DUEL_H, SAY_DUEL_I
};

class npc_death_knight_initiate : public CreatureScript
{
public:
    npc_death_knight_initiate() : CreatureScript("npc_death_knight_initiate") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            player->CLOSE_GOSSIP_MENU();

            if (player->isInCombat() || creature->isInCombat())
                return true;

            if (npc_death_knight_initiateAI* pInitiateAI = CAST_AI(npc_death_knight_initiate::npc_death_knight_initiateAI, creature->AI()))
            {
                if (pInitiateAI->m_bIsDuelInProgress)
                    return true;
            }

            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);

            int32 uiSayId = rand()% (sizeof(m_auiRandomSay)/sizeof(int32));
            DoScriptText(m_auiRandomSay[uiSayId], creature, player);

            player->CastSpell(creature, SPELL_DUEL, false);
            player->CastSpell(player, SPELL_DUEL_FLAG, true);
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(QUEST_DEATH_CHALLENGE) == QUEST_STATUS_INCOMPLETE && creature->IsFullHealth())
        {
            if (player->HealthBelowPct(10))
                return true;

            if (player->isInCombat() || creature->isInCombat())
                return true;

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ACCEPT_DUEL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_death_knight_initiateAI(creature);
    }

    struct npc_death_knight_initiateAI : public CombatAI
    {
        npc_death_knight_initiateAI(Creature* creature) : CombatAI(creature)
        {
            m_bIsDuelInProgress = false;
        }

        bool lose;
        ObjectGuid m_uiDuelerGUID;
        uint32 m_uiDuelTimer;
        bool m_bIsDuelInProgress;

        void Reset()
        {
            lose = false;
            me->RestoreFaction();
            CombatAI::Reset();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);

            m_uiDuelerGUID.Clear();
            m_uiDuelTimer = 5000;
            m_bIsDuelInProgress = false;
        }

        void SpellHit(Unit* pCaster, const SpellInfo* pSpell)
        {
            if (!m_bIsDuelInProgress && pSpell->Id == SPELL_DUEL)
            {
                m_uiDuelerGUID = pCaster->GetGUID();
                m_bIsDuelInProgress = true;
            }
        }

       void DamageTaken(Unit* pDoneBy, uint32 &uiDamage, DamageEffectType dmgType)
        {
            if (m_bIsDuelInProgress && pDoneBy->IsControlledByPlayer())
            {
                if (pDoneBy->GetGUID() != m_uiDuelerGUID && pDoneBy->GetOwnerGUID() != m_uiDuelerGUID) // other players cannot help
                    uiDamage = 0;
                else if (uiDamage >= me->GetHealth())
                {
                    uiDamage = 0;

                    if (!lose)
                    {
                        pDoneBy->RemoveGameObject(SPELL_DUEL_FLAG, true);
                        pDoneBy->AttackStop();
                        me->CastSpell(pDoneBy, SPELL_DUEL_VICTORY, true);
                        lose = true;
                        me->CastSpell(me, 7267, true);
                        me->RestoreFaction();
                    }
                }
            }
        }

        void UpdateAI(uint32 uiDiff)
        {
            if (!UpdateVictim())
            {
                if (m_bIsDuelInProgress)
                {
                    if (m_uiDuelTimer <= uiDiff)
                    {
                        me->setFaction(FACTION_HOSTILE);

                        if (Unit* unit = Unit::GetUnit(*me, m_uiDuelerGUID))
                            AttackStart(unit);
                    }
                    else
                        m_uiDuelTimer -= uiDiff;
                }
                return;
            }

            if (m_bIsDuelInProgress)
            {
                if (lose)
                {
                    if (!me->HasAura(7267))
                        EnterEvadeMode();
                    return;
                }
                else if (me->getVictim() && me->getVictim()->GetTypeId() == TYPEID_PLAYER && me->getVictim()->HealthBelowPct(10))
                {
                    me->getVictim()->CastSpell(me->getVictim(), 7267, true); // beg
                    me->getVictim()->RemoveGameObject(SPELL_DUEL_FLAG, true);
                    EnterEvadeMode();
                    return;
                }
            }

            // TODO: spells

            CombatAI::UpdateAI(uiDiff);
        }
    };

};

/*######
## npc_dark_rider_of_acherus
######*/

#define DESPAWN_HORSE 52267
#define SAY_DARK_RIDER      "The realm of shadows awaits..."

class npc_dark_rider_of_acherus : public CreatureScript
{
public:
    npc_dark_rider_of_acherus() : CreatureScript("npc_dark_rider_of_acherus") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dark_rider_of_acherusAI(creature);
    }

    struct npc_dark_rider_of_acherusAI : public ScriptedAI
    {
        npc_dark_rider_of_acherusAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 PhaseTimer;
        uint32 Phase;
        bool Intro;
        ObjectGuid TargetGUID;

        void Reset()
        {
            PhaseTimer = 4000;
            Phase = 0;
            Intro = false;
            TargetGUID.Clear();
        }

        void UpdateAI(uint32 diff)
        {
            if (!Intro || !TargetGUID)
                return;

            if (PhaseTimer <= diff)
            {
                switch (Phase)
                {
                   case 0:
                        me->MonsterSay(SAY_DARK_RIDER, LANG_UNIVERSAL, ObjectGuid::Empty);
                        PhaseTimer = 5000;
                        Phase = 1;
                        break;
                    case 1:
                        if (Unit* target = Unit::GetUnit(*me, TargetGUID))
                            DoCast(target, DESPAWN_HORSE, true);
                        PhaseTimer = 3000;
                        Phase = 2;
                        break;
                    case 2:
                        me->SetVisible(false);
                        PhaseTimer = 2000;
                        Phase = 3;
                        break;
                    case 3:
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            } else PhaseTimer -= diff;

        }

        void InitDespawnHorse(Unit* who)
        {
            if (!who)
                return;

            TargetGUID = who->GetGUID();
            me->SetWalk(true);
            me->SetSpeed(MOVE_RUN, 0.4f);
            me->GetMotionMaster()->MoveChase(who);
            me->SetTarget(TargetGUID);
            Intro = true;
        }

    };

};

/*######
## npc_salanar_the_horseman
######*/

enum eSalanar
{
    REALM_OF_SHADOWS            = 52693,
    EFFECT_STOLEN_HORSE         = 52263,
    DELIVER_STOLEN_HORSE        = 52264,
    CALL_DARK_RIDER             = 52266,
    SPELL_EFFECT_OVERTAKE       = 52349
};

class npc_salanar_the_horseman : public CreatureScript
{
public:
    npc_salanar_the_horseman() : CreatureScript("npc_salanar_the_horseman") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_salanar_the_horsemanAI(creature);
    }

    struct npc_salanar_the_horsemanAI : public ScriptedAI
    {
        npc_salanar_the_horsemanAI(Creature* creature) : ScriptedAI(creature) {}

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == DELIVER_STOLEN_HORSE)
            {
                if (caster->GetTypeId() == TYPEID_UNIT && caster->IsVehicle())
                {
                    if (Unit* charmer = caster->GetCharmer())
                    {
                        if (charmer->HasAura(EFFECT_STOLEN_HORSE))
                        {
                            charmer->RemoveAurasDueToSpell(EFFECT_STOLEN_HORSE);
                            caster->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                            caster->setFaction(35);
                            DoCast(caster, CALL_DARK_RIDER, true);
                            if (Creature* Dark_Rider = me->FindNearestCreature(28654, 15))
                                CAST_AI(npc_dark_rider_of_acherus::npc_dark_rider_of_acherusAI, Dark_Rider->AI())->InitDespawnHorse(caster);
                        }
                    }
                }
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (who->GetTypeId() == TYPEID_UNIT && who->IsVehicle() && me->IsWithinDistInMap(who, 5.0f))
            {
                if (Unit* charmer = who->GetCharmer())
                {
                    if (charmer->GetTypeId() == TYPEID_PLAYER)
                    {
                        // for quest Into the Realm of Shadows(12687)
                        if (me->GetEntry() == 28788 && CAST_PLR(charmer)->GetQuestStatus(12687) == QUEST_STATUS_INCOMPLETE)
                        {
                            CAST_PLR(charmer)->GroupEventHappens(12687, me);
                            charmer->RemoveAurasDueToSpell(SPELL_EFFECT_OVERTAKE);
                            CAST_CRE(who)->DespawnOrUnsummon();
                            //CAST_CRE(who)->Respawn(true);
                        }

                        if (CAST_PLR(charmer)->HasAura(REALM_OF_SHADOWS))
                            charmer->RemoveAurasDueToSpell(REALM_OF_SHADOWS);
                    }
                }
            }
        }
    };

};

/*######
## npc_ros_dark_rider
######*/

class npc_ros_dark_rider : public CreatureScript
{
public:
    npc_ros_dark_rider() : CreatureScript("npc_ros_dark_rider") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ros_dark_riderAI(creature);
    }

    struct npc_ros_dark_riderAI : public ScriptedAI
    {
        npc_ros_dark_riderAI(Creature* creature) : ScriptedAI(creature) {}

        void EnterCombat(Unit* /*who*/)
        {
            me->ExitVehicle();
        }

        void Reset()
        {
            Creature* deathcharger = me->FindNearestCreature(28782, 30);
            if (!deathcharger)
                return;

            deathcharger->RestoreFaction();
            deathcharger->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            deathcharger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            //if (!me->GetVehicle() && deathcharger->IsVehicle() && deathcharger->GetVehicleKit()->HasEmptySeat(0))
                //me->EnterVehicle(deathcharger);
        }

        void JustDied(Unit* killer)
        {
            Creature* deathcharger = me->FindNearestCreature(28782, 30);
            if (!deathcharger)
                return;

            if (killer->GetTypeId() == TYPEID_PLAYER && deathcharger->GetTypeId() == TYPEID_UNIT && deathcharger->IsVehicle())
            {
                deathcharger->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                deathcharger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                deathcharger->setFaction(2096);
            }
        }
    };

};

// correct way: 52312 52314 52555 ...
enum SG
{
    GHOULS = 28845,
    GHOSTS = 28846,
};
class npc_dkc1_gothik : public CreatureScript
{
public:
    npc_dkc1_gothik() : CreatureScript("npc_dkc1_gothik") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dkc1_gothikAI(creature);
    }

    struct npc_dkc1_gothikAI : public ScriptedAI
    {
        npc_dkc1_gothikAI(Creature* creature) : ScriptedAI(creature) {}

        void MoveInLineOfSight(Unit* who)
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (who->GetEntry() == GHOULS && me->IsWithinDistInMap(who, 10.0f))
            {
                if (Unit* owner = who->GetOwner())
                {
                    if (owner->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (CAST_PLR(owner)->GetQuestStatus(12698) == QUEST_STATUS_INCOMPLETE)
                            CAST_CRE(who)->CastSpell(owner, 52517, true);

                        //Todo: Creatures must not be removed, but, must instead
                        //      stand next to Gothik and be commanded into the pit
                        //      and dig into the ground.
                        CAST_CRE(who)->DespawnOrUnsummon();

                        if (CAST_PLR(owner)->GetQuestStatus(12698) == QUEST_STATUS_COMPLETE)
                            owner->RemoveAllMinionsByFilter(GHOULS);
                    }
                }
            }
        }
    };

};

class npc_scarlet_ghoul : public CreatureScript
{
public:
    npc_scarlet_ghoul() : CreatureScript("npc_scarlet_ghoul") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_ghoulAI(creature);
    }

    struct npc_scarlet_ghoulAI : public ScriptedAI
    {
        npc_scarlet_ghoulAI(Creature* creature) : ScriptedAI(creature)
        {
            // Ghouls should display their Birth Animation
            // Crawling out of the ground
            DoCast(me, 35177, true);
            me->MonsterSay("Mommy?", LANG_UNIVERSAL, ObjectGuid::Empty);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void FindMinions(Unit* owner)
        {
            std::list<Creature*> MinionList;
            owner->GetAllMinionsByEntry(MinionList, GHOULS);

            if (!MinionList.empty())
            {
                for (std::list<Creature*>::const_iterator itr = MinionList.begin(); itr != MinionList.end(); ++itr)
                {
                    if ((*itr)->GetOwner()->GetGUID() == me->GetOwner()->GetGUID())
                    {
                        if ((*itr)->isInCombat() && (*itr)->getAttackerForHelper())
                        {
                            AttackStart((*itr)->getAttackerForHelper());
                        }
                    }
                }
            }
        }

        void UpdateAI(uint32 /*diff*/)
        {
            if (!me->isInCombat())
            {
                if (Unit* owner = me->GetOwner())
                {
                    Player* plrOwner = owner->ToPlayer();
                    if (plrOwner && plrOwner->isInCombat())
                    {
                        if (plrOwner->getAttackerForHelper() && plrOwner->getAttackerForHelper()->GetEntry() == GHOSTS)
                            AttackStart(plrOwner->getAttackerForHelper());
                        else
                            FindMinions(owner);
                    }
                }
            }

            if (!UpdateVictim())
                return;

            //ScriptedAI::UpdateAI(diff);
            //Check if we have a current target
            if (me->getVictim()->GetEntry() == GHOSTS)
            {
                if (me->isAttackReady())
                {
                    //If we are within range melee the target
                    if (me->IsWithinMeleeRange(me->getVictim()))
                    {
                        me->AttackerStateUpdate(me->getVictim());
                        me->resetAttackTimer();
                    }
                }
            }
        }
    };

};

/*####
## npc_scarlet_miner_cart
####*/

enum ScarletMinerCart
{
    SPELL_CART_CHECK        = 54173,
    SPELL_SUMMON_CART       = 52463,
    SPELL_SUMMON_MINER      = 52464,
    SPELL_CART_DRAG         = 52465,

    NPC_MINER               = 28841
};

class npc_scarlet_miner_cart : public CreatureScript
{
    public:
        npc_scarlet_miner_cart() : CreatureScript("npc_scarlet_miner_cart") { }

        struct npc_scarlet_miner_cartAI : public ScriptedAI
        {
            npc_scarlet_miner_cartAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetDisplayId(me->GetCreatureTemplate()->Modelid[0]); // Modelid2 is a horse.
            }

            void JustSummoned(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_MINER)
                {
                    _minerGUID = summon->GetGUID();
                    summon->AI()->SetGUID(_playerGUID);
                }
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_MINER)
                    _minerGUID.Clear();
            }

            void DoAction(const int32 /*param*/) override
            {
                if (Creature* miner = ObjectAccessor::GetCreature(*me, _minerGUID))
                {
                    me->SetWalk(false);
                    me->SetSpeed(MOVE_RUN, 1.25f);
                    me->GetMotionMaster()->MoveFollow(miner, 1.0f, 0);
                }
            }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
            {
                if (apply)
                {
                    _playerGUID = who->GetGUID();
                    me->CastSpell(me, SPELL_SUMMON_MINER, true);
                }
                else
                {
                    _playerGUID.Clear();
                    if (Creature* miner = ObjectAccessor::GetCreature(*me, _minerGUID))
                        miner->DespawnOrUnsummon();
                }
            }

        private:
            ObjectGuid _minerGUID;
            ObjectGuid _playerGUID;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_scarlet_miner_cartAI(creature);
        }
};

/*####
## npc_scarlet_miner
####*/

#define SAY_SCARLET_MINER1  "Where'd this come from? I better get this down to the ships before the foreman sees it!"
#define SAY_SCARLET_MINER2  "Now I can have a rest!"

class npc_scarlet_miner : public CreatureScript
{
public:
    npc_scarlet_miner() : CreatureScript("npc_scarlet_miner") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_minerAI(creature);
    }

    struct npc_scarlet_minerAI : public npc_escortAI
    {
        npc_scarlet_minerAI(Creature* creature) : npc_escortAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        uint32 IntroTimer;
        uint32 IntroPhase;
        ObjectGuid carGUID;

        void Reset()
        {
            carGUID.Clear();
            IntroTimer = 0;
            IntroPhase = 0;
        }

        void InitWaypoint()
        {
            AddWaypoint(1, 2389.03f,     -5902.74f,     109.014f, 5000);
            AddWaypoint(2, 2341.812012f, -5900.484863f, 102.619743f);
            AddWaypoint(3, 2306.561279f, -5901.738281f, 91.792419f);
            AddWaypoint(4, 2300.098389f, -5912.618652f, 86.014885f);
            AddWaypoint(5, 2294.142090f, -5927.274414f, 75.316849f);
            AddWaypoint(6, 2286.984375f, -5944.955566f, 63.714966f);
            AddWaypoint(7, 2280.001709f, -5961.186035f, 54.228283f);
            AddWaypoint(8, 2259.389648f, -5974.197754f, 42.359348f);
            AddWaypoint(9, 2242.882812f, -5984.642578f, 32.827850f);
            AddWaypoint(10, 2217.265625f, -6028.959473f, 7.675705f);
            AddWaypoint(11, 2202.595947f, -6061.325684f, 5.882018f);
            AddWaypoint(12, 2188.974609f, -6080.866699f, 3.370027f);

            if (urand(0, 1))
            {
                AddWaypoint(13, 2176.483887f, -6110.407227f, 1.855181f);
                AddWaypoint(14, 2172.516602f, -6146.752441f, 1.074235f);
                AddWaypoint(15, 2138.918457f, -6158.920898f, 1.342926f);
                AddWaypoint(16, 2129.866699f, -6174.107910f, 4.380779f);
                AddWaypoint(17, 2117.709473f, -6193.830078f, 13.3542f, 10000);
            }
            else
            {
                AddWaypoint(13, 2184.190186f, -6166.447266f, 0.968877f);
                AddWaypoint(14, 2234.265625f, -6163.741211f, 0.916021f);
                AddWaypoint(15, 2268.071777f, -6158.750977f, 1.822252f);
                AddWaypoint(16, 2270.028320f, -6176.505859f, 6.340538f);
                AddWaypoint(17, 2271.739014f, -6195.401855f, 13.3542f, 10000);
            }
        }

        void IsSummonedBy(Unit* summoner) override
        {
            carGUID = summoner->GetGUID();
        }

        void SetGUID(ObjectGuid const& guid, int32 /*id = 0*/) override
        {
            InitWaypoint();
            Start(false, false, guid);
            SetDespawnAtFar(false);
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 1:
                    if (Unit* car = Unit::GetCreature(*me, carGUID))
                    {
                        me->SetFacingTo(car);
                    }
                    me->MonsterSay(SAY_SCARLET_MINER1, LANG_UNIVERSAL, ObjectGuid::Empty);
                    SetRun(true);
                    IntroTimer = 4000;
                    IntroPhase = 1;
                    break;
                case 17:
                    if (Unit* car = Unit::GetCreature(*me, carGUID))
                    {
                        me->SetFacingTo(car);
                        car->Relocate(car->GetPositionX(), car->GetPositionY(), me->GetPositionZ() + 1);
                        car->StopMoving();
                        car->RemoveAura(SPELL_CART_DRAG);
                        car->GetPlayer(*me, car->GetCreatorGUID())->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        car->GetPlayer(*me, car->GetCreatorGUID())->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                    }
                    me->MonsterSay(SAY_SCARLET_MINER2, LANG_UNIVERSAL, ObjectGuid::Empty);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (IntroPhase)
            {
                if (IntroTimer <= diff)
                {
                    if (IntroPhase == 1)
                    {
                        if (Creature* car = Unit::GetCreature(*me, carGUID))
                            DoCast(car, SPELL_CART_DRAG);
                        IntroTimer = 800;
                        IntroPhase = 2;
                    }
                    else
                    {
                        if (Creature* car = Unit::GetCreature(*me, carGUID))
                            car->AI()->DoAction(0);
                        IntroPhase = 0;
                    }
                } else IntroTimer-=diff;
            }
            npc_escortAI::UpdateAI(diff);
        }
    };

};

/*######
## npc_eye_of_acherus
######*/

class npc_eye_of_acherus : public CreatureScript
{
public:
    npc_eye_of_acherus() : CreatureScript("npc_eye_of_acherus") { }

    CreatureAI* GetAI(Creature* _Creature) const
    {
        return new npc_eye_of_acherusAI(_Creature);
    }

    struct npc_eye_of_acherusAI : public ScriptedAI
    {
        npc_eye_of_acherusAI(Creature* c) : ScriptedAI(c)
        {
            me->setActive(true);
            me->SetLevel(55); //else one hack
            StartTimer = 1000;
            Active = false;
        }

        uint32 StartTimer;
        bool Active;

        void Reset(){}
        void AttackStart(Unit *) {}
        void MoveInLineOfSight(Unit*) {}
        void OnCharmed(bool /*apply*/)
        {
            //NOT DISABLE AI!
        }

        void JustDied(Unit*u)
        {
            if(!me || me->GetTypeId() != TYPEID_UNIT)
                return;

            Unit *target = me->GetCharmer();

            if(!target || target->GetTypeId() != TYPEID_PLAYER)
                return;

            target->RemoveAurasDueToSpell(51852);
            me->RemoveCharmedBy(NULL);

            //me->DespawnOrUnsummon();
        }

        void MovementInform(uint32 uiType, uint32 uiPointId)
        {
            if (uiType != POINT_MOTION_TYPE && uiPointId == 0)
                return;

                char * text = "The Eye of Acherus is in your control";
                me->MonsterTextEmote(text, me->GetGUID(), true);
                me->CastSpell(me, 51890, true);
                //me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                //me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
        }

        void UpdateAI(uint32 uiDiff)
        {
            if(me->isCharmed())
            {
                if (StartTimer < uiDiff && !Active)
                {
                    //me->CastSpell(me, 70889, true);
                    //me->CastSpell(me, 51892, true);
                    //me->SetPhaseMask(3, false);
                    char * text = "The Eye of Acherus launches towards its destination";
                    me->MonsterTextEmote(text, me->GetGUID(), true);
                    me->SetSpeed(MOVE_FLIGHT, 6.4f,true);
                    me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
                    me->GetMotionMaster()->MovePoint(0, 1750.8276f, -5873.788f, 147.2266f);
                    Active = true;
                }
                else StartTimer -= uiDiff;
            }
            /*else
            {
                me->DespawnOrUnsummon();
            }*/
        }
    };
};

void AddSC_the_scarlet_enclave_c1()
{
    new npc_unworthy_initiate();
    new npc_unworthy_initiate_anchor();
    new go_acherus_soul_prison();
    //new npc_death_knight_initiate();
    new npc_salanar_the_horseman();
    new npc_dark_rider_of_acherus();
    new npc_ros_dark_rider();
    new npc_dkc1_gothik();
    //new npc_scarlet_ghoul();
    new npc_scarlet_miner();
    new npc_scarlet_miner_cart();
    new npc_eye_of_acherus();
}
