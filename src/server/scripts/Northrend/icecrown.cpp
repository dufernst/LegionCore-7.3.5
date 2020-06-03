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

/* ScriptData
SDName: Icecrown
SD%Complete: 100
SDComment: Quest support: 12807
SDCategory: Icecrown
EndScriptData */

/* ContentData
npc_arete
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "Player.h"
#include "TemporarySummon.h"
#include "CombatAI.h"
#include "CreatureGroups.h"

/*######
## npc_arete
######*/

#define GOSSIP_ARETE_ITEM1 "Lord-Commander, I would hear your tale."
#define GOSSIP_ARETE_ITEM2 "<You nod slightly but do not complete the motion as the Lord-Commander narrows his eyes before he continues.>"
#define GOSSIP_ARETE_ITEM3 "I thought that they now called themselves the Scarlet Onslaught?"
#define GOSSIP_ARETE_ITEM4 "Where did the grand admiral go?"
#define GOSSIP_ARETE_ITEM5 "That's fine. When do I start?"
#define GOSSIP_ARETE_ITEM6 "Let's finish this!"
#define GOSSIP_ARETE_ITEM7 "That's quite a tale, Lord-Commander."

enum eArete
{
    GOSSIP_TEXTID_ARETE1        = 13525,
    GOSSIP_TEXTID_ARETE2        = 13526,
    GOSSIP_TEXTID_ARETE3        = 13527,
    GOSSIP_TEXTID_ARETE4        = 13528,
    GOSSIP_TEXTID_ARETE5        = 13529,
    GOSSIP_TEXTID_ARETE6        = 13530,
    GOSSIP_TEXTID_ARETE7        = 13531,

    QUEST_THE_STORY_THUS_FAR    = 12807
};

class npc_arete : public CreatureScript
{
public:
    npc_arete() : CreatureScript("npc_arete") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_THE_STORY_THUS_FAR) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE1, creature->GetGUID());
            return true;
        }

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE2, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE3, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE4, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE5, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+5:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE6, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+6:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE7, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+7:
                player->CLOSE_GOSSIP_MENU();
                player->AreaExploredOrEventHappens(QUEST_THE_STORY_THUS_FAR);
                break;
        }

        return true;
    }
};

/*######
## npc_squire_david
######*/

enum eSquireDavid
{
    QUEST_THE_ASPIRANT_S_CHALLENGE_H                    = 13680,
    QUEST_THE_ASPIRANT_S_CHALLENGE_A                    = 13679,

    NPC_ARGENT_VALIANT                                  = 33448,

    GOSSIP_TEXTID_SQUIRE                                = 14407
};

#define GOSSIP_SQUIRE_ITEM_1 "I am ready to fight!"
#define GOSSIP_SQUIRE_ITEM_2 "How do the Argent Crusader raiders fight?"

class npc_squire_david : public CreatureScript
{
public:
    npc_squire_david() : CreatureScript("npc_squire_david") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(QUEST_THE_ASPIRANT_S_CHALLENGE_H) == QUEST_STATUS_INCOMPLETE ||
            player->GetQuestStatus(QUEST_THE_ASPIRANT_S_CHALLENGE_A) == QUEST_STATUS_INCOMPLETE)//We need more info about it.
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        }

        player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SQUIRE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            player->CLOSE_GOSSIP_MENU();
            creature->SummonCreature(NPC_ARGENT_VALIANT, 8575.451f, 952.472f, 547.554f, 0.38f);
        }
        return true;
    }
};

/*######
## npc_argent_valiant
######*/

enum eArgentValiant
{
    SPELL_CHARGE                = 63010,
    SPELL_SHIELD_BREAKER        = 65147,
    SPELL_DEFEND                = 62719,
    SPELL_THRUST                = 62544,

    NPC_ARGENT_VALIANT_CREDIT   = 38595
};

enum eValiantText
{
    NPC_FACTION_VAILIANT_TEXT_SAY_START_1     = -1850004,//    Tenez-vous pret !
    NPC_FACTION_VAILIANT_TEXT_SAY_START_2     = -1850005,//    Que le combat commence !
    NPC_FACTION_VAILIANT_TEXT_SAY_START_3     = -1850006,//    Preparez-vous !
    NPC_ARGENT_VAILIANT_TEXT_SAY_START         = -1850007,//    Vous pensez avoir la vaillance en vous ? Nous verrons.
    NPC_ARGENT_VAILIANT_TEXT_SAY_WIN         = -1850008,//    Impressionnante demonstration. Je pense que vous etes tout a fait en mesure de rejoindre les rangs des vaillants.
    NPC_ARGENT_VAILIANT_TEXT_SAY_LOOSE         = -1850009,//    J'ai gagne. Vous aurez sans doute plus de chance la prochaine fois.
    NPC_FACTION_VAILIANT_TEXT_SAY_WIN_1     = -1850010,//    Je suis vaincue. Joli combat !
    NPC_FACTION_VAILIANT_TEXT_SAY_WIN_2     = -1850011,//    On dirait que j'ai sous-estime vos competences. Bien joue.
    NPC_FACTION_VAILIANT_TEXT_SAY_LOOSE     = -1850012,//    J'ai gagne. Vous aurez sans doute plus de chance la prochaine fois.
};

class npc_argent_valiant : public CreatureScript
{
public:
    npc_argent_valiant() : CreatureScript("npc_argent_valiant") { }

    struct npc_argent_valiantAI : public ScriptedAI
    {
        npc_argent_valiantAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->CastSpell(me, SPELL_DEFEND, true);
            pCreature->GetMotionMaster()->MovePoint(0, 8599.258f, 963.951f, 547.553f);
            pCreature->setFaction(35); //wrong faction in db?
        }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        uint32 uiDefendTimer;

        void Reset() override
        {
            uiChargeTimer = 7000;
            uiShieldBreakerTimer = 10000;
            uiDefendTimer = 10000;
        }

        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            me->setFaction(14);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage, DamageEffectType dmgType) override
        {
            if(pDoneBy)
            {
                if (uiDamage > me->GetHealth() && (pDoneBy->GetTypeId() == TYPEID_PLAYER || pDoneBy->GetOwner()))
                {
                    DoScriptText(NPC_ARGENT_VAILIANT_TEXT_SAY_WIN, me);
                    uiDamage = 0;

                    if(pDoneBy->GetOwner())
                        (pDoneBy->GetOwner())->ToPlayer()->KilledMonsterCredit(NPC_ARGENT_VALIANT_CREDIT, ObjectGuid::Empty);
                    if(pDoneBy->GetTypeId() == TYPEID_PLAYER)
                        pDoneBy->ToPlayer()->KilledMonsterCredit(NPC_ARGENT_VALIANT_CREDIT, ObjectGuid::Empty);

                    me->setFaction(35);
                    me->DespawnOrUnsummon(5000);
                    me->SetHomePosition(me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation());
                    EnterEvadeMode();
                }
            }
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            me->setFaction(35);
            me->DespawnOrUnsummon(5000);
            DoScriptText(NPC_ARGENT_VAILIANT_TEXT_SAY_LOOSE, me);
            me->CombatStop(true);
        }

        void DoMeleeAttackIfReady()
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            //Make sure our attack is ready and we aren't currently casting before checking distance
            if (me->isAttackReady())
            {
                //If we are within range melee the target
                if (me->IsWithinMeleeRange(me->getVictim()))
                {
                    DoCastVictim(SPELL_THRUST);
                    me->resetAttackTimer();
                }
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            DoScriptText(NPC_ARGENT_VAILIANT_TEXT_SAY_START, me);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiChargeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CHARGE);
                uiChargeTimer = 7000;
            } else uiChargeTimer -= uiDiff;

            if (uiShieldBreakerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHIELD_BREAKER);
                uiShieldBreakerTimer = 10000;
            } else uiShieldBreakerTimer -= uiDiff;

            if (uiDefendTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_DEFEND, true);
                uiDefendTimer = 10000;
            } else uiDefendTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new npc_argent_valiantAI(creature);
    }
};

/*######
## npc_guardian_pavilion
######*/

enum eGuardianPavilion
{
    SPELL_TRESPASSER_H                            = 63987,
    AREA_SUNREAVER_PAVILION                       = 4676,

    AREA_SILVER_COVENANT_PAVILION                 = 4677,
    SPELL_TRESPASSER_A                            = 63986,
};

class npc_guardian_pavilion : public CreatureScript
{
public:
    npc_guardian_pavilion() : CreatureScript("npc_guardian_pavilion") { }

    struct npc_guardian_pavilionAI : public Scripted_NoMovementAI
    {
        npc_guardian_pavilionAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

        void MoveInLineOfSight(Unit* who) override
        {
            if (me->GetAreaId() != AREA_SUNREAVER_PAVILION && me->GetAreaId() != AREA_SILVER_COVENANT_PAVILION)
                return;

            if (!who || who->GetTypeId() != TYPEID_PLAYER || !me->IsHostileTo(who) || !me->isInBackInMap(who, 5.0f))
                return;

            if (who->HasAura(SPELL_TRESPASSER_H) || who->HasAura(SPELL_TRESPASSER_A))
                return;

            if (who->ToPlayer()->GetTeamId() == TEAM_ALLIANCE)
                who->CastSpell(who, SPELL_TRESPASSER_H, true);
            else
                who->CastSpell(who, SPELL_TRESPASSER_A, true);

        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_guardian_pavilionAI(creature);
    }
};

/*######
## npc_vereth_the_cunning
######*/

enum eVerethTheCunning
{
    NPC_GEIST_RETURN_BUNNY_KC   = 31049,
    NPC_LITHE_STALKER           = 30894,
    SPELL_SUBDUED_LITHE_STALKER = 58151,
};

class npc_vereth_the_cunning : public CreatureScript
{
public:
    npc_vereth_the_cunning() : CreatureScript("npc_vereth_the_cunning") { }

    struct npc_vereth_the_cunningAI : public ScriptedAI
    {
        npc_vereth_the_cunningAI(Creature* creature) : ScriptedAI(creature) {}

        void MoveInLineOfSight(Unit* who) override
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (who->GetEntry() == NPC_LITHE_STALKER && me->IsWithinDistInMap(who, 10.0f))
            {
                if (Unit* owner = who->GetCharmer())
                {
                    if (who->HasAura(SPELL_SUBDUED_LITHE_STALKER))
                        {
                        owner->ToPlayer()->KilledMonsterCredit(NPC_GEIST_RETURN_BUNNY_KC, ObjectGuid::Empty);
                            who->ToCreature()->DisappearAndDie();

                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_vereth_the_cunningAI(creature);
    }
};

/*######
* npc_tournament_training_dummy
######*/
enum TournamentDummy
{
    NPC_CHARGE_TARGET         = 33272,
    NPC_MELEE_TARGET          = 33229,
    NPC_RANGED_TARGET         = 33243,

    SPELL_CHARGE_CREDIT       = 62658,
    SPELL_MELEE_CREDIT        = 62672,
    SPELL_RANGED_CREDIT       = 62673,

    SPELL_PLAYER_THRUST       = 62544,
    SPELL_PLAYER_BREAK_SHIELD = 62626,
    SPELL_PLAYER_CHARGE       = 62874,

    SPELL_RANGED_DEFEND       = 62719,
    SPELL_CHARGE_DEFEND       = 64100,
    SPELL_VULNERABLE          = 62665,

    SPELL_COUNTERATTACK       = 62709,

    EVENT_DUMMY_RECAST_DEFEND = 1,
    EVENT_DUMMY_RESET         = 2,
};

class npc_tournament_training_dummy : public CreatureScript
{
    public:
        npc_tournament_training_dummy(): CreatureScript("npc_tournament_training_dummy"){}

        struct npc_tournament_training_dummyAI : Scripted_NoMovementAI
        {
            npc_tournament_training_dummyAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            EventMap events;
            bool isVulnerable;

            void Reset() override
            {
                me->SetControlled(true, UNIT_STATE_STUNNED);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                isVulnerable = false;

                // Cast Defend spells to max stack size
                switch (me->GetEntry())
                {
                    case NPC_CHARGE_TARGET:
                        DoCast(SPELL_CHARGE_DEFEND);
                        break;
                    case NPC_RANGED_TARGET:
                        me->CastCustomSpell(SPELL_RANGED_DEFEND, SPELLVALUE_AURA_STACK, 3, me);
                        break;
                }

                events.Reset();
                events.ScheduleEvent(EVENT_DUMMY_RECAST_DEFEND, 5000);
            }

            void EnterEvadeMode() override
            {
                if (!_EnterEvadeMode())
                    return;

                Reset();
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
            {
                damage = 0;
                events.RescheduleEvent(EVENT_DUMMY_RESET, 10000);
            }

            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                switch (me->GetEntry())
                {
                    case NPC_CHARGE_TARGET:
                        if (spell->Id == SPELL_PLAYER_CHARGE)
                            if (isVulnerable)
                                DoCast(caster, SPELL_CHARGE_CREDIT, true);
                        break;
                    case NPC_MELEE_TARGET:
                        if (spell->Id == SPELL_PLAYER_THRUST)
                        {
                            DoCast(caster, SPELL_MELEE_CREDIT, true);

                            if (Unit* target = caster->GetVehicleBase())
                                DoCast(target, SPELL_COUNTERATTACK, true);
                        }
                        break;
                    case NPC_RANGED_TARGET:
                        if (spell->Id == SPELL_PLAYER_BREAK_SHIELD)
                            if (isVulnerable)
                                DoCast(caster, SPELL_RANGED_CREDIT, true);
                        break;
                }

                if (spell->Id == SPELL_PLAYER_BREAK_SHIELD)
                    if (!me->HasAura(SPELL_CHARGE_DEFEND) && !me->HasAura(SPELL_RANGED_DEFEND))
                        isVulnerable = true;
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_DUMMY_RECAST_DEFEND:
                        switch (me->GetEntry())
                        {
                            case NPC_CHARGE_TARGET:
                            {
                                if (!me->HasAura(SPELL_CHARGE_DEFEND))
                                    DoCast(SPELL_CHARGE_DEFEND);
                                break;
                            }
                            case NPC_RANGED_TARGET:
                            {
                                Aura* defend = me->GetAura(SPELL_RANGED_DEFEND);
                                if (!defend || defend->GetStackAmount() < 3 || defend->GetDuration() <= 8000)
                                    DoCast(SPELL_RANGED_DEFEND);
                                break;
                            }
                        }
                        isVulnerable = false;
                        events.ScheduleEvent(EVENT_DUMMY_RECAST_DEFEND, 5000);
                        break;
                    case EVENT_DUMMY_RESET:
                        if (UpdateVictim())
                        {
                            EnterEvadeMode();
                            events.ScheduleEvent(EVENT_DUMMY_RESET, 10000);
                        }
                        break;
                }

                if (!UpdateVictim())
                    return;

                if (!me->HasUnitState(UNIT_STATE_STUNNED))
                    me->SetControlled(true, UNIT_STATE_STUNNED);
            }

            void MoveInLineOfSight(Unit* /*who*/) override {}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_tournament_training_dummyAI(creature);
        }

};

class npc_valiant : public CreatureScript
{
public:
    npc_valiant() : CreatureScript("npc_valiant") { }

    struct npc_valiantAI : public ScriptedAI
    {
        npc_valiantAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->CastSpell(me, SPELL_DEFEND, true);
            me->setFaction(35); //wrong faction in db?
        }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        uint32 uiDefendTimer;

        void Reset() override
        {
            uiChargeTimer = 7000;
            uiShieldBreakerTimer = 10000;
            uiDefendTimer = 10000;
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage, DamageEffectType dmgType) override
        {
            if(pDoneBy)
            {
                if (uiDamage > me->GetHealth() && (pDoneBy->GetTypeId() == TYPEID_PLAYER || pDoneBy->GetOwner()))
                {
                    uiDamage = 0;

                    if(pDoneBy->GetOwner())
                        pDoneBy->GetOwner()->CastSpell(pDoneBy->GetOwner(), 62724, true);
                    if(pDoneBy->GetTypeId() == TYPEID_PLAYER)
                        pDoneBy->CastSpell(pDoneBy, 62724, true);

                    me->setFaction(35);
                    me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    EnterEvadeMode();
                }
            }
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            me->setFaction(35);
            me->CombatStop(true);
        }

        void DoAction(int32 const /*param*/) override
        {
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->setFaction(14);
            me->setActive(true);
            me->SetReactState(REACT_AGGRESSIVE);
            DoZoneInCombat();
        }

        void DoMeleeAttackIfReady()
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            //Make sure our attack is ready and we aren't currently casting before checking distance
            if (me->isAttackReady())
            {
                //If we are within range melee the target
                if (me->IsWithinMeleeRange(me->getVictim()))
                {
                    DoCastVictim(SPELL_THRUST);
                    me->resetAttackTimer();
                }
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiChargeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CHARGE);
                uiChargeTimer = 7000;
            } else uiChargeTimer -= uiDiff;

            if (uiShieldBreakerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHIELD_BREAKER);
                uiShieldBreakerTimer = 10000;
            } else uiShieldBreakerTimer -= uiDiff;

            if (uiDefendTimer <= uiDiff)
            {
                me->CastSpell(me, SPELL_DEFEND, true);
                uiDefendTimer = 10000;
            } else uiDefendTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
        player->PrepareGossipMenu(creature, 0);
        player->ADD_GOSSIP_ITEM(0, "I am ready to fight!", GOSSIP_SENDER_MAIN, 90909090);
        player->SendPreparedGossip(creature);
        return true;
    }
    
    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if(sender != GOSSIP_SENDER_MAIN) return true;
        if(!player->getAttackers()->empty()) return true;

        switch(action)
        {
            case 90909090:
                player->CLOSE_GOSSIP_MENU();
                ((npc_valiantAI*)creature->AI())->DoAction(action);
                break;
            default:
                return false;                                   // nothing defined      -> trinity core handling
        }
        return true;
    }

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new npc_valiantAI(creature);
    }
};

/*
* Npc Jeran Lockwood (33973)
*/
#define JERAN_DEFAULT_TEXTID 14453
#define JERAN_QUEST_TEXTID 14431
#define JERAN_RP_TEXTID 14434
#define GOSSIP_HELLO_JERAN_1 "Montrez-moi comment m'entraA®ner sur une cible de mA?lA©e."
#define GOSSIP_HELLO_JERAN_2 "Parlez-moi de la dA©fense et du coup de lance."
#define SPELL_CREDIT_JERAN 64113

class npc_jeran_lockwood : public CreatureScript
{
public:
    npc_jeran_lockwood(): CreatureScript("npc_jeran_lockwood"){}
        
    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if((pPlayer->GetQuestStatus(13828) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(13829) == QUEST_STATUS_INCOMPLETE))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_JERAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_JERAN_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        pPlayer->SEND_GOSSIP_MENU(JERAN_QUEST_TEXTID, pCreature->GetGUID());
        }
        else
        {
            pPlayer->SEND_GOSSIP_MENU(JERAN_DEFAULT_TEXTID, pCreature->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        switch(uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->CastSpell(pPlayer,SPELL_CREDIT_JERAN,true);
                pPlayer->CLOSE_GOSSIP_MENU();
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_JERAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
                pPlayer->SEND_GOSSIP_MENU(JERAN_RP_TEXTID, pCreature->GetGUID());
                break;
        }
        return true;
    }

};

/*
* Npc Rugan Steelbelly (33972)
*/
#define RUGAN_DEFAULT_TEXTID 14453
#define RUGAN_QUEST_TEXTID 14436
#define RUGAN_RP_TEXTID 14437
#define GOSSIP_HELLO_RUGAN_1 "Montrez-moi comment m'entraA®ner sur une cible de charge."
#define GOSSIP_HELLO_RUGAN_2 "Parlez-moi de la charge"
#define SPELL_CREDIT_RUGAN 64114

class npc_rugan_steelbelly : public CreatureScript
{
public:
    npc_rugan_steelbelly(): CreatureScript("npc_rugan_steelbelly"){}
        
    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if((pPlayer->GetQuestStatus(13837) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(13839) == QUEST_STATUS_INCOMPLETE))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_RUGAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_RUGAN_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        pPlayer->SEND_GOSSIP_MENU(RUGAN_QUEST_TEXTID, pCreature->GetGUID());
        }
        else
        {
            pPlayer->SEND_GOSSIP_MENU(RUGAN_DEFAULT_TEXTID, pCreature->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        switch(uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->CastSpell(pPlayer,SPELL_CREDIT_RUGAN,true);
                pPlayer->CLOSE_GOSSIP_MENU();
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_RUGAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
                pPlayer->SEND_GOSSIP_MENU(RUGAN_RP_TEXTID, pCreature->GetGUID());
                break;
        }
        return true;
    }

};

/*
* Npc Valis Windchaser
*/
#define VALIS_DEFAULT_TEXTID 14453
#define VALIS_QUEST_TEXTID 14438
#define VALIS_RP_TEXTID 14439
#define GOSSIP_HELLO_VALIS_1 "Montrez-moi comment m'entraA®ner sur une cible A distance."
#define GOSSIP_HELLO_VALIS_2 "Expliquez-moi comment utiliser le brise-bouclier."
#define SPELL_CREDIT_VALIS 64115
class npc_valis_windchaser : public CreatureScript
{
public:
    npc_valis_windchaser(): CreatureScript("npc_valis_windchaser"){}

    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        //Si il a la quete
        if((pPlayer->GetQuestStatus(13835) == QUEST_STATUS_INCOMPLETE) || 
            (pPlayer->GetQuestStatus(13838) == QUEST_STATUS_INCOMPLETE))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_VALIS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_VALIS_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        pPlayer->SEND_GOSSIP_MENU(VALIS_QUEST_TEXTID, pCreature->GetGUID());
        }
        //Sinon Texte par defaut
        else
            pPlayer->SEND_GOSSIP_MENU(VALIS_DEFAULT_TEXTID, pCreature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->CastSpell(pPlayer,SPELL_CREDIT_VALIS,true);//Cast du sort de credit quest (valide l'objectif)
                pPlayer->CLOSE_GOSSIP_MENU();//Ferme la fenetre du gossip cote client
            break;
            case GOSSIP_ACTION_INFO_DEF+2:
                //Raconte un blabla
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_VALIS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->SEND_GOSSIP_MENU(VALIS_RP_TEXTID, pCreature->GetGUID());
            break;
        }
        return true;
    }

};

/*######
## npc_squire_danny
######*/

enum eSquireDanny
{
    QUEST_THE_VALIANT_S_CHALLENGE_0 = 13699,
    QUEST_THE_VALIANT_S_CHALLENGE_1 = 13713,
    QUEST_THE_VALIANT_S_CHALLENGE_2 = 13723,
    QUEST_THE_VALIANT_S_CHALLENGE_3 = 13724,
    QUEST_THE_VALIANT_S_CHALLENGE_4 = 13725,
    QUEST_THE_VALIANT_S_CHALLENGE_5 = 13726,
    QUEST_THE_VALIANT_S_CHALLENGE_6 = 13727,
    QUEST_THE_VALIANT_S_CHALLENGE_7 = 13728,
    QUEST_THE_VALIANT_S_CHALLENGE_8 = 13729,
    QUEST_THE_VALIANT_S_CHALLENGE_9 = 13731,

    NPC_ARGENT_CHAMPION = 33707,

    GOSSIP_TEXTID_SQUIRE_DANNY = 14407
};

#define GOSSIP_SQUIRE_ITEM_1 "I am ready to fight!"
#define GOSSIP_SQUIRE_ITEM_2 "How do the Argent Crusader raiders fight?"

class npc_squire_danny : public CreatureScript
{
public:
    npc_squire_danny(): CreatureScript("npc_squire_danny"){}

    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        if (pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_0) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_1) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_2) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_3) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_4) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_5) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_6) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_7) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_8) == QUEST_STATUS_INCOMPLETE ||
        pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_9) == QUEST_STATUS_INCOMPLETE)
        {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        }

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SQUIRE_DANNY, pCreature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
        {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->SummonCreature(NPC_ARGENT_CHAMPION,8562.836914f,1099.153931f,556.787598f,5.026550f); // TODO (Recuperer les coordonnees reelles)
        }
        //else
        //pPlayer->SEND_GOSSIP_MENU(???, pCreature->GetGUID()); Missing text
        return true;
    }

};

/*######
## npc_argent_champion
######*/

enum eArgentChampion
{
    SPELL_CHARGE_CHAMPION                = 63010,
    SPELL_SHIELD_BREAKER_CHAMPION        = 65147,
    SPELL_DEFEND_CHAMPION        = 62719,
    SPELL_THRUST_CHAMPION        = 62544,

    NPC_ARGENT_CHAMPION_CREDIT   = 33708
};

enum eChampionText
{
    NPC_FACTION_CHAMPION_TEXT_SAY_START_1     = -1850004,//    Tenez-vous pret !
    NPC_FACTION_CHAMPION_TEXT_SAY_START_2     = -1850005,//    Que le combat commence !
    NPC_FACTION_CHAMPION_TEXT_SAY_START_3     = -1850006,//    Preparez-vous !
    NPC_ARGENT_CHAMPION_TEXT_SAY_START         = -1850007,//    Vous pensez avoir la vaillance en vous ? Nous verrons.
    NPC_ARGENT_CHAMPION_TEXT_SAY_WIN         = -1850008,//    Impressionnante demonstration. Je pense que vous etes tout a fait en mesure de rejoindre les rangs des vaillants.
    NPC_ARGENT_CHAMPION_TEXT_SAY_LOOSE         = -1850009,//    J'ai gagne. Vous aurez sans doute plus de chance la prochaine fois.
    NPC_FACTION_CHAMPION_TEXT_SAY_WIN_1     = -1850010,//    Je suis vaincue. Joli combat !
    NPC_FACTION_CHAMPION_TEXT_SAY_WIN_2     = -1850011,//    On dirait que j'ai sous-estime vos competences. Bien joue.
    NPC_FACTION_CHAMPION_TEXT_SAY_LOOSE     = -1850012,//    J'ai gagne. Vous aurez sans doute plus de chance la prochaine fois.
};

class npc_argent_champion : public CreatureScript
{
public:
    npc_argent_champion(): CreatureScript("npc_argent_champion"){}

    struct npc_argent_championAI : public ScriptedAI
    {
        npc_argent_championAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        me->CastSpell(me, SPELL_DEFEND_CHAMPION, true);
        me->CastSpell(me, SPELL_DEFEND_CHAMPION, true);
        pCreature->GetMotionMaster()->MovePoint(0,8552.469727f,1124.128784f,556.787598f); // TODO (Trouver les coordonnees exactes)
        pCreature->setFaction(35); //wrong faction in db?
        }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        uint32 uiDefendTimer;

        void Reset() override
        {
        uiChargeTimer = 7000;
        uiShieldBreakerTimer = 10000;
        }

        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
        if (uiType != POINT_MOTION_TYPE)
            return;

        me->setFaction(14);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage, DamageEffectType dmgType) override
        {
            if(pDoneBy)
            {
                if (uiDamage > me->GetHealth() && (pDoneBy->GetTypeId() == TYPEID_PLAYER || pDoneBy->GetOwner()))
                {
                    DoScriptText(NPC_ARGENT_CHAMPION_TEXT_SAY_WIN, me);
                    uiDamage = 0;

                    if(pDoneBy->GetOwner())
                        (pDoneBy->GetOwner())->ToPlayer()->KilledMonsterCredit(NPC_ARGENT_CHAMPION_CREDIT, ObjectGuid::Empty);
                    if(pDoneBy->GetTypeId() == TYPEID_PLAYER)
                        pDoneBy->ToPlayer()->KilledMonsterCredit(NPC_ARGENT_CHAMPION_CREDIT, ObjectGuid::Empty);

                    me->setFaction(35);
                    me->DespawnOrUnsummon(5000);
                    me->SetHomePosition(me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation());
                    EnterEvadeMode();
                }
            }
        }

        void KilledUnit(Unit* /*victim*/) override
        {
        me->setFaction(35);
        me->DespawnOrUnsummon(5000);
        DoScriptText(NPC_ARGENT_CHAMPION_TEXT_SAY_LOOSE, me);
        me->CombatStop(true);
        }

        void DoMeleeAttackIfReady()
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            //Make sure our attack is ready and we aren't currently casting before checking distance
            if (me->isAttackReady())
            {
                //If we are within range melee the target
                if (me->IsWithinMeleeRange(me->getVictim()))
                {
                DoCastVictim(SPELL_THRUST_CHAMPION);
                me->resetAttackTimer();
                }
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
        DoScriptText(NPC_ARGENT_CHAMPION_TEXT_SAY_START, me);
        }

        void UpdateAI(uint32 uiDiff) override
        {
        if (!UpdateVictim())
            return;

        if (uiChargeTimer <= uiDiff)
        {
            DoCastVictim(SPELL_CHARGE_CHAMPION);
            uiChargeTimer = 7000;
        } else uiChargeTimer -= uiDiff;

        if (uiShieldBreakerTimer <= uiDiff)
        {
            DoCastVictim(SPELL_SHIELD_BREAKER_CHAMPION);
            uiShieldBreakerTimer = 10000;
        } else uiShieldBreakerTimer -= uiDiff;

        if (uiDefendTimer <= uiDiff)
        {
            me->CastSpell(me, SPELL_DEFEND_CHAMPION, true);
            uiDefendTimer = 10000;
        } else uiDefendTimer -= uiDiff;

        DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_argent_championAI (pCreature);
    }

};

/*######
* npc_training_dummy_argent
######*/
// UPDATE `creature_template` SET `ScriptName`='npc_training_dummy_argent' WHERE `entry`=33229;
// UPDATE `creature_template` SET `ScriptName`='npc_training_dummy_argent' WHERE `entry`=33272;
// UPDATE `creature_template` SET `ScriptName`='npc_training_dummy_argent' WHERE `entry`=33243;
enum eTrainingdummy
{
    CREDIT_RANGE               = 33339,
    CREDIT_CHARGE              = 33340,
    CREDIT_MELEE               = 33341,
    NPC_MELEE                  = 33229,
    NPC_CHARGE                 = 33272,
    NPC_RANGE                  = 33243,
    SPELL_ARGENT_MELEE         = 62544,
    SPELL_ARGENT_CHARGE        = 62874,
    SPELL_ARGENT_BREAK_SHIELD  = 62626,  // spell goes't work
    SPELL_DEFEND_AURA          = 62719,  // it's spell spam in console
    SPELL_DEFEND_AURA_1        = 64100,  // it's spell spam in console
    NPC_ADVANCED_TARGET_DUMMY  = 2674,
    NPC_TARGET_DUMMY           = 2673
};

class npc_training_dummy_argent : public CreatureScript
{
public:
    npc_training_dummy_argent(): CreatureScript("npc_training_dummy_argent"){}
        
    struct npc_training_dummy_argentAI : Scripted_NoMovementAI
    {
        npc_training_dummy_argentAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            Npc_Entry = pCreature->GetEntry();
        }

        uint32 Npc_Entry;
        uint32 ResetTimer;
        uint32 DespawnTimer;
        uint32 ShielTimer;

        void Reset() override
        {
            me->SetControlled(true,UNIT_STATE_STUNNED);//disable rotate
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);//imune to knock aways like blast wave
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            ResetTimer = 5000;
            DespawnTimer = 15000;
            ShielTimer=0;
        }

        void EnterEvadeMode() override
        {
            if (!_EnterEvadeMode())
                return;
            Reset();
        }

        void DamageTaken(Unit * /*done_by*/, uint32 &damage, DamageEffectType dmgType) override
        {
            ResetTimer = 5000;
            damage = 0;
        }

        void EnterCombat(Unit * /*who*/) override
        {
            if (Npc_Entry != NPC_ADVANCED_TARGET_DUMMY && Npc_Entry != NPC_TARGET_DUMMY)
                return;
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            switch (Npc_Entry)
            {
                case NPC_MELEE: // dummy melee
                    if (spell->Id == SPELL_ARGENT_MELEE)
                        me->CastSpell(caster,62672,true);
                    return;
                case NPC_CHARGE: // dummy charge
                    if (spell->Id == SPELL_ARGENT_CHARGE)
                        me->CastSpell(caster,62658,true);
                    return;
                case NPC_RANGE: // dummy range
                    if (spell->Id == SPELL_ARGENT_BREAK_SHIELD)
                        me->CastSpell(caster,62673,true);
                    return;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (ShielTimer <= diff)
            {
                if(Npc_Entry == NPC_RANGE)
                    me->CastSpell(me,SPELL_DEFEND_AURA,true);

                if(Npc_Entry == NPC_CHARGE && !me->GetAura(SPELL_DEFEND_AURA_1))
                    me->CastSpell(me,SPELL_DEFEND_AURA_1,true);
                ShielTimer = 8000;
            }
            else
                ShielTimer -= diff;

            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STATE_STUNNED))
                me->SetControlled(true,UNIT_STATE_STUNNED);//disable rotate
            
            if (Npc_Entry != NPC_ADVANCED_TARGET_DUMMY && Npc_Entry != NPC_TARGET_DUMMY)
            {
                if (ResetTimer <= diff)
                {
                    EnterEvadeMode();
                    ResetTimer = 5000;
                }
                else
                    ResetTimer -= diff;
                return;
            }
            else
            {
                if (DespawnTimer <= diff)
                    me->DespawnOrUnsummon();
                else
                    DespawnTimer -= diff;
            }
        }
        void MoveInLineOfSight(Unit * /*who*/) override {return;}
    };

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_training_dummy_argentAI(pCreature);
    }

};

/*######
## npc_vendor_argent_tournament
######*/
const uint32 ArgentTournamentVendor[10][4] =
{
    {33553,13726,2,14460}, // Orc
    {33554,13726,8,14464}, // Troll
    {33556,13728,6,14458}, // Tauren
    {33555,13729,5,14459}, // Undead
    {33557,13731,10,14465}, // Blood Elf
    {33307,13699,1,14456}, // Human
    {33310,13713,3,14457}, // Dwarf
    {33653,13725,4,14463}, // Night Elf
    {33650,13723,7,14462}, // Gnome
    {33657,13724,11,14461} // Draenei
};

class npc_vendor_argent_tournament : public CreatureScript
{
public:
    npc_vendor_argent_tournament(): CreatureScript("npc_vendor_argent_tournament"){}

    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        bool npcCheck = false;
        bool questCheck = false;
        bool raceCheck = false;
        uint32 textId = 0;
        
        for(int i = 0; (i < 10) && !npcCheck; i++)
        {
            if(pCreature->GetEntry() == ArgentTournamentVendor[i][0])
            {
                npcCheck = true;
                questCheck = pPlayer->GetQuestStatus(ArgentTournamentVendor[i][1]) == QUEST_STATUS_COMPLETE;
                raceCheck = pPlayer->getRace() == ArgentTournamentVendor[i][2];
                textId = ArgentTournamentVendor[i][3];
            }
        }
        
        if(questCheck || raceCheck)
            pPlayer->GetSession()->SendListInventory(pCreature->GetGUID()); 
        else
            pPlayer->SEND_GOSSIP_MENU(textId, pCreature->GetGUID());
        return true;
    }

};

/*######
Tirion's Gambit
######*/
enum TalkList
{
    SAY_TIRION_01                  = 0,
    SAY_TIRION_02                  = 1,
    SAY_TIRION_03                  = 2,
    SAY_TIRION_04                  = 3,
    SAY_TIRION_05                  = 4,
    SAY_TIRION_06                  = 5,
    SAY_TIRION_07                  = 6,
    SAY_TIRION_08                  = 7,
    SAY_TIRION_09                  = 8,

    SAY_ZEALOT_01                  = 0,

    SAY_LICH_KING_01               = 0,
    SAY_LICH_KING_02               = 1,
    SAY_LICH_KING_03               = 2,
    SAY_LICH_KING_04               = 3,
    SAY_LICH_KING_05               = 4,
    SAY_LICH_KING_06               = 5,
    SAY_LICH_KING_07               = 6,

    SAY_CRUASDER_01                = 0,

    SAY_THASSARIAN_01              = 0,
    SAY_THASSARIAN_02              = 1,

    SAY_KOLITRA_01                 = 0,
    SAY_KOLITRA_02                 = 1,

    SAY_DARION_01                  = 0,
    SAY_DARION_02                  = 1,
};

enum NPC
{
    NPC_TIRION_FORDRING            = 32239,
    NPC_THE_LICH_KING              = 32184,
    NPC_HIGH_INVOKER_BASALEP       = 32272,
    NPC_DARION_MOGRAINE            = 32312,
    NPC_KOLITRA_DEATHWEAVER        = 32311,
    NPC_THASSARIAN                 = 32310,
    NPC_DISGUISED_CRUSADER         = 32241,
    NPC_CHOSEN_ZEALOT              = 32175,
    NPC_EBON_KNIGHT                = 32309,
    NPC_QUEST_CREDIT               = 32648,
    NPC_JUMP_TRIGGER               = 24042,
};

enum Spells
{
    SPELL_LICH_KING_S_FURY         = 60536,
    SPELL_TIRION_S_ATTACK          = 42904,
    SPELL_HEART_EXPLOISON          = 60484,
    SPELL_CULTIST_HOOD             = 61131,
    SPELL_QUEST_CREDIT             = 61487,
    SPELL_TIRION_JUMP              = 60456,
    
    // Helpers:
    SPELL_BLOOD_STRIKES        = 52374,
    SPELL_FROST                = 52375,
    SPELL_DEATH                = 57602,
    SPELL_GRIP                 = 52372,
    SPELL_WRATH                = 60545,
};

enum ActionList
{
    ACTION_START_ESCORT            = 1,
    ACTION_PLAYER_FACTION          = 2,
};

enum Events
{
    EVENT_NONE,
    EVENT_START_ESCORT,
    EVENT_MOVE_1,
    EVENT_MOVE_2,
    EVENT_MOVE_3,
    EVENT_TALK_1,
    EVENT_MOVE_4,
    EVENT_MOVE_5,
    EVENT_MOVE_6,
    EVENT_MOVE_7,
    EVENT_MOVE_8,
    EVENT_STAY_1,
    EVENT_KNEEL,
    EVENT_SPAWN_HEART,
    EVENT_STAY_2,
    EVENT_ACYLTE_SAY,
    EVENT_STAND,
    EVENT_BOW,
    EVENT_ACYLTE_LEAVE,
    EVENT_SUMMON_LICH_KING,
    EVENT_ACYLTE_MOVE,
    EVENT_UNCOVERED,
    EVENT_MOVE_08,
    EVENT_DIALOGUE,
    EVENT_DESTROY_HEART,
    EVENT_TIRION_ATTACK,
    EVENT_HEART_BLOW,
    EVENT_EXPLOISON,
    EVENT_LK_KNEEL,
    EVENT_ESCORT_DEFEND_1,
    EVENT_ESCORT_DEFEND_2,
    EVENT_ROOM_ATTACK,
    EVENT_SPAWN_KNIGHTS,
    EVENT_SUMMON_PORTAL,
    EVENT_DARION_TALK,
    EVENT_DARION_TALK_2,
    EVENT_LICH_KING_S_FURY,
    EVENT_RESET,

    //Talk events
    EVENT_TALK_2,
    EVENT_TALK_3,
    EVENT_TALK_4,
    EVENT_TALK_5,
    EVENT_TALK_6,
    EVENT_TALK_7,
    EVENT_TALK_8,  
    EVENT_TALK_9,
    EVENT_TALK_10,
};

enum Misc
{
    EQUIP_ASHBRINGER               = 13262,
    EQUIP_ESCORT                   = 13160,

    GO_FROZEN_HEART                = 193794,
    GO_ESCAPE_PORTAL               = 193941,

    QUEST_TIRION_GAMBIT_A          = 13403,
    QUEST_TIRION_GAMBIT_H          = 13364,
};

struct Location
{
    float x, y, z, x2, y2, z2;
    uint32 id;
};

static Location Summon[]=
{
    {6162.911f, 2688.551f, 573.914f, 6133.930f, 2758.642f, 573.914f}, 
    {6165.741f, 2689.889f, 573.914f, 6133.930f, 2758.642f, 573.914f}, 
    {6169.457f, 2691.660f, 573.914f, 6133.930f, 2758.642f, 573.914f}, 
    {6168.437f, 2687.293f, 573.914f, 6133.930f, 2758.642f, 573.914f}
};

class npc_tg_tirion_fordring : public CreatureScript
{
    public:
        npc_tg_tirion_fordring(): CreatureScript("npc_tg_tirion_fordring"){}

        struct npc_tg_tirion_fordringAI : ScriptedAI
        {
            npc_tg_tirion_fordringAI(Creature* creature) : ScriptedAI(creature) 
            {
                Reset();
            }

            void Reset() override
            {
                SetEquipmentSlots(false, 0, 0, 0);
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                LichKingGUID.Clear();
                MograineGUID.Clear();
                EscortGUID[0].Clear();
                EscortGUID[1].Clear();
                EscortGUID[2].Clear();
                AcylteGUID[0].Clear();
                AcylteGUID[1].Clear();
                AcylteGUID[2].Clear();
                isHorde = false;
                gofhGuid.Clear();
                goepGuid.Clear();
                Events.Reset();
            }

            void DoAction(const int32 actionId) override
            {
                switch (actionId)
                {
                    case ACTION_START_ESCORT:
                        {
                            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                            Talk(SAY_TIRION_01);

                            uint32 index = 0;
                            std::list<Creature*> escortList;
                            me->GetCreatureListWithEntryInGrid(escortList, NPC_DISGUISED_CRUSADER, 40.0f);
                            for (std::list<Creature*>::iterator itr = escortList.begin(); itr != escortList.end() && index < 3; ++itr)
                            {
                                EscortGUID[index++] = (*itr)->GetGUID();
                                (*itr)->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                (*itr)->GetMotionMaster()->MoveFollow(me, 5.0f, 0.0f);
                            }

                            std::list<Creature*> acylteList;
                            me->GetCreatureListWithEntryInGrid(acylteList, NPC_CHOSEN_ZEALOT, 500.0f);
                            for (std::list<Creature*>::iterator itr = acylteList.begin(); itr != acylteList.end(); ++itr)
                            {
                                (*itr)->Respawn();
                                (*itr)->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                            }
                            Events.ScheduleEvent(EVENT_START_ESCORT, 5000);
                        }
                        break;
                    case ACTION_PLAYER_FACTION:
                        isHorde = true;
                        break;
                    default:
                        break;
                }

            }

            void UpdateAI(uint32 diff) override
            {
                Events.Update(diff);
                while (uint32 EventId = Events.ExecuteEvent())
                {
                    switch (EventId)
                    {
                        case EVENT_START_ESCORT:
                            me->GetMotionMaster()->MovePoint(0, 6244.514f, 2653.304f, 570.250f);
                            Events.ScheduleEvent(EVENT_MOVE_1, 3500);
                            break;
                        case EVENT_MOVE_1:
                            me->GetMotionMaster()->MovePoint(0, 6238.080f, 2637.720f, 570.250f);
                            Events.ScheduleEvent(EVENT_MOVE_2, 5000);
                            break;
                        case EVENT_MOVE_2:
                            me->GetMotionMaster()->MovePoint(0, 6205.637f, 2607.220f, 570.250f);
                            Events.ScheduleEvent(EVENT_MOVE_3, 17000);
                            break;
                        case EVENT_MOVE_3:
                            me->GetMotionMaster()->MovePoint(0, 6190.103f, 2637.606f, 570.250f);
                            Events.ScheduleEvent(EVENT_TALK_1, 17000);
                            break;
                        case EVENT_TALK_1:
                            Talk(SAY_TIRION_02);
                            Events.ScheduleEvent(EVENT_MOVE_4, 2500);
                            break;
                        case EVENT_MOVE_4:
                            me->GetMotionMaster()->MovePoint(0, 6180.998f, 2657.858f, 573.766f);
                            Events.ScheduleEvent(EVENT_MOVE_5, 10000);
                            break;
                        case EVENT_MOVE_5:
                            me->GetMotionMaster()->MovePoint(0, 6144.981f, 2736.049f, 573.914f);
                            Events.ScheduleEvent(EVENT_MOVE_6, 34000);
                            break;
                        case EVENT_MOVE_6:
                            //me->GetFormation()->IgnoreFormationMotion = false;
                            me->GetMotionMaster()->MovePoint(0, 6163.32f, 2746.957f, 573.914f);
                            if (Creature* Escort = me->GetCreature(*me, EscortGUID[0]))
                            {
                                Escort->GetMotionMaster()->Clear(false);
                                Escort->GetMotionMaster()->MovePoint(0, 6168.990f, 2761.715f, 573.915f);
                            }
                            if (Creature* Escort = me->GetCreature(*me, EscortGUID[1]))
                            {
                                Escort->GetMotionMaster()->Clear(false);
                                Escort->GetMotionMaster()->MovePoint(0, 6172.088f, 2763.098f, 573.915f);
                            }
                            if (Creature* Escort = me->GetCreature(*me, EscortGUID[2]))
                            {
                                Escort->GetMotionMaster()->Clear(false);
                                Escort->GetMotionMaster()->MovePoint(0, 6175.234f, 2764.600f, 573.915f);
                            }
                            Events.ScheduleEvent(EVENT_MOVE_7, 9000);
                            break;
                        case EVENT_MOVE_7:
                            me->GetMotionMaster()->MovePoint(0, 6165.613f, 2760.049f, 573.914f);
                            Events.ScheduleEvent(EVENT_STAY_1, 9000);
                            break;
                        case EVENT_STAY_1:
                            if (Creature* Invoker = me->FindNearestCreature(NPC_HIGH_INVOKER_BASALEP, 150.0f))
                            {
                                me->SetFacingToObject(Invoker);
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[0]))
                                    Escort->SetFacingToObject(Invoker);
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[1]))
                                    Escort->SetFacingToObject(Invoker);
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[2]))
                                    Escort->SetFacingToObject(Invoker);
                            }
                            Talk(SAY_TIRION_03);
                            if (Creature* Acylte = me->SummonCreature(NPC_CHOSEN_ZEALOT, 6166.140f ,2688.851f ,573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 75000))
                            {
                                AcylteGUID[0] = Acylte->GetGUID();
                                Acylte->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Acylte->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                Acylte->GetMotionMaster()->MovePoint(0, 6133.930f, 2758.642f, 573.914f);
                            } 
                            if (Creature* Acylte = me->SummonCreature(NPC_CHOSEN_ZEALOT, 6165.166f, 2684.646f, 573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 75000))
                            {
                                AcylteGUID[1] = Acylte->GetGUID();
                                Acylte->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Acylte->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                Acylte->GetMotionMaster()->MovePoint(0, 6132.520f, 2753.841f, 573.914f);
                            } 
                            if (Creature* Acylte = me->SummonCreature(NPC_CHOSEN_ZEALOT, 6170.443f, 2687.208f, 573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 75000))
                            {
                                AcylteGUID[2] = Acylte->GetGUID();
                                Acylte->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Acylte->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                Acylte->GetMotionMaster()->MovePoint(0, 6138.576f, 2757.051f, 573.914f);
                            } 
                            Events.ScheduleEvent(EVENT_KNEEL, 32000);
                            break;
                        case EVENT_KNEEL:
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[0]))
                                Acylte->SetStandState(UNIT_STAND_STATE_KNEEL);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[1]))
                                Acylte->SetStandState(UNIT_STAND_STATE_KNEEL);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[2]))
                                Acylte->SetStandState(UNIT_STAND_STATE_KNEEL);
                            Events.ScheduleEvent(EVENT_SPAWN_HEART, 2000);
                            break;
                        case EVENT_SPAWN_HEART:
                            if (GameObject* gofh = me->SummonGameObject(GO_FROZEN_HEART, 6132.78f, 2760.67f, 573.914f, 1.9979f, 0.0f, 0.0f, 0.0f, 1, 120000))
                                gofhGuid = gofh->GetGUID();
                            Events.ScheduleEvent(EVENT_ACYLTE_SAY, 3000);
                            break;
                        case EVENT_ACYLTE_SAY:
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[0]))
                                Acylte->AI()->Talk(SAY_ZEALOT_01);
                            Events.ScheduleEvent(EVENT_STAND, 4000);
                            break;
                        case EVENT_STAND:
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[0]))
                                Acylte->SetStandState(UNIT_STAND_STATE_STAND);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[1]))
                                Acylte->SetStandState(UNIT_STAND_STATE_STAND);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[2]))
                                Acylte->SetStandState(UNIT_STAND_STATE_STAND);
                            Events.ScheduleEvent(EVENT_BOW, 2000);
                            break;
                        case EVENT_BOW:
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[0]))
                                Acylte->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[1]))
                                Acylte->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[2]))
                                Acylte->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                            Events.ScheduleEvent(EVENT_ACYLTE_LEAVE, 2000);
                            break;
                        case EVENT_ACYLTE_LEAVE:
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[0]))
                                Acylte->GetMotionMaster()->MovePoint(0, 6166.140f, 2688.851f, 573.914f);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[1]))
                                Acylte->GetMotionMaster()->MovePoint(0, 6165.166f, 2684.646f, 573.914f);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[2]))
                                Acylte->GetMotionMaster()->MovePoint(0, 6170.443f, 2687.208f, 573.914f);
                            Events.ScheduleEvent(EVENT_TALK_10,          1500);
                            Events.ScheduleEvent(EVENT_SUMMON_LICH_KING, 5000);
                            break;
                        case EVENT_SUMMON_LICH_KING:
                            Talk(SAY_TIRION_05);
                            if (Creature* LichKing = me->SummonCreature(NPC_THE_LICH_KING, 6166.140f, 2688.851f, 573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 240000))
                            {
                                LichKingGUID = LichKing->GetGUID();
                                LichKing->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                LichKing->GetMotionMaster()->MovePoint(0, 6133.930f, 2758.642f, 573.914f);
                                me->SetFacingToObject(LichKing);
                            }
                            Events.ScheduleEvent(EVENT_ACYLTE_MOVE, 9000);
                            break;
                        case EVENT_ACYLTE_MOVE:
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[0]))
                                Acylte->GetMotionMaster()->MovePoint(0, 6133.294f, 2691.485f, 573.914f);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[1]))
                                Acylte->GetMotionMaster()->MovePoint(0, 6133.294f, 2691.485f, 573.914f);
                            if (Creature* Acylte = me->GetCreature(*me, AcylteGUID[2]))
                                Acylte->GetMotionMaster()->MovePoint(0, 6133.294f ,2691.485f, 573.914f);
                            Events.ScheduleEvent(EVENT_UNCOVERED, 25000);
                            break;
                        case EVENT_UNCOVERED:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                            {
                                LichKing->SetFacingToObject(me);
                                LichKing->AI()->Talk(SAY_LICH_KING_01);
                                me->SetVirtualItem(0, uint32(EQUIP_ASHBRINGER));
                                me->GetMotionMaster()->MovePoint(0, 6167.601f, 2757.219f, 573.914f);
                            }
                            Events.ScheduleEvent(EVENT_MOVE_08, 2000);
                            break;
                        case EVENT_MOVE_08:
                            me->GetMotionMaster()->MovePoint(0, 6157.914f, 2755.602f, 573.914f);
                            Events.ScheduleEvent(EVENT_DIALOGUE, 3000);
                            break;
                        case EVENT_DIALOGUE:
                            me->GetMotionMaster()->MovePoint(0, 6143.597f, 2757.256f, 573.914f);
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                LichKing->AI()->Talk(SAY_LICH_KING_02);
                            Events.ScheduleEvent(EVENT_TALK_2,        19000);
                            Events.ScheduleEvent(EVENT_TALK_3,        36000);
                            Events.ScheduleEvent(EVENT_TALK_4,        44000);
                            Events.ScheduleEvent(EVENT_TALK_5,        14000);
                            Events.ScheduleEvent(EVENT_TALK_6,        30000);
                            Events.ScheduleEvent(EVENT_TALK_7,        47000);
                            Events.ScheduleEvent(EVENT_TALK_8,        63000);
                            Events.ScheduleEvent(EVENT_DESTROY_HEART, 67000);
                            break;
                        case EVENT_TALK_2:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                LichKing->AI()->Talk(SAY_LICH_KING_03);
                            break;
                        case EVENT_TALK_3:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                LichKing->AI()->Talk(SAY_LICH_KING_04);
                            break;
                        case EVENT_TALK_4:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                LichKing->AI()->Talk(SAY_LICH_KING_05);
                            break;
                        case EVENT_TALK_5:
                            Talk(SAY_TIRION_06);
                            break;
                        case EVENT_TALK_6:
                            Talk(SAY_TIRION_07);
                            break;
                        case EVENT_TALK_7:
                            Talk(SAY_TIRION_08);
                            break;
                        case EVENT_TALK_8:
                            Talk(SAY_TIRION_09);
                            break;
                        case EVENT_TALK_9:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                LichKing->AI()->Talk(SAY_LICH_KING_07);
                            break;
                        case EVENT_TALK_10:
                            Talk(SAY_TIRION_04);
                            break;
                        case EVENT_DESTROY_HEART:
                            // me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                            me->GetMotionMaster()->Clear(false);
                            if (Creature* Trigger = me->FindNearestCreature(NPC_JUMP_TRIGGER, 50.0f))
                                DoCast(Trigger, SPELL_TIRION_JUMP, true);
                            Events.ScheduleEvent(EVENT_TIRION_ATTACK, 500);
                            break;
                        case EVENT_TIRION_ATTACK:
                            DoCast(me, SPELL_TIRION_S_ATTACK);
                            Events.ScheduleEvent(EVENT_HEART_BLOW, 800);
                            break;
                        case EVENT_HEART_BLOW:
                            DoCast(me, SPELL_HEART_EXPLOISON);
                            Events.ScheduleEvent(EVENT_EXPLOISON, 500);
                            break;
                        case EVENT_EXPLOISON:
                            me->SetHealth(1);
                            if (Creature* Invoker = me->FindNearestCreature(NPC_HIGH_INVOKER_BASALEP, 150.0f))
                                Invoker->Kill(Invoker);
                            me->SetStandState(UNIT_STAND_STATE_DEAD);
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                            {
                                LichKing->DealDamage(LichKing, 19500000, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                                LichKing->GetMap()->CreatureRelocation(((Creature*)LichKing), 6130.926f, 2743.355f, 573.914f, 0.537f);
                                //LichKing->SendMonsterMove(6130.926f, 2743.355f, 573.914f, SPLINETYPE_NORMAL , SPLINEFLAG_NONE, 500);
                                LichKing->SetOrientation(0.537f);
                            }
                            Events.ScheduleEvent(EVENT_LK_KNEEL, 500);
                            break;
                        case EVENT_LK_KNEEL:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                            {
                                LichKing->SetStandState(UNIT_STAND_STATE_KNEEL);
                                LichKing->AI()->Talk(SAY_LICH_KING_06);
                            }
                            Events.ScheduleEvent(EVENT_ESCORT_DEFEND_1, 500);
                            break;
                        case EVENT_ESCORT_DEFEND_1:
                            if (Creature* Escort = me->GetCreature(*me, EscortGUID[0]))
                            {
                                Escort->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Escort->SetVirtualItem(0, uint32(EQUIP_ESCORT));
                                Escort->GetMotionMaster()->MovePoint(0, 6137.778f, 2759.621f, 573.914f);
                            }
                            if (Creature* Escort = me->GetCreature(*me, EscortGUID[1]))
                            {
                                Escort->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Escort->SetVirtualItem(0, uint32(EQUIP_ESCORT));
                                Escort->GetMotionMaster()->MovePoint(0, 6128.400f, 2757.948f, 573.914f);
                            }
                            if (Creature* Escort = me->GetCreature(*me, EscortGUID[2]))
                            {
                                Escort->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Escort->SetVirtualItem(0, uint32(EQUIP_ESCORT));
                                Escort->GetMotionMaster()->MovePoint(0, 6132.821f, 2765.189f, 573.914f);
                            }
                            Events.ScheduleEvent(EVENT_ESCORT_DEFEND_2, 5000);
                            break;
                        case EVENT_ESCORT_DEFEND_2:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                            {
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[0]))
                                {
                                    Escort->SetFacingToObject(LichKing);
                                    Escort->AI()->Talk(SAY_CRUASDER_01);
                                    Escort->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY2H);
                                }
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[1]))
                                {
                                    Escort->SetFacingToObject(LichKing);
                                    Escort->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY2H);
                                }
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[2]))
                                {
                                    Escort->SetFacingToObject(LichKing);
                                    Escort->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY2H);
                                }
                            }
                            Events.ScheduleEvent(EVENT_TALK_9, 3000);
                            Events.ScheduleEvent(EVENT_ROOM_ATTACK, 6000);
                            break;
                        case EVENT_ROOM_ATTACK:
                            {
                            std::list<Creature*> Zealots;
                            GetCreatureListWithEntryInGrid(Zealots, me, NPC_CHOSEN_ZEALOT, 100.0f);

                            if (Zealots.empty())
                                return;
 
                            for (std::list<Creature*>::iterator itr = Zealots.begin(); itr != Zealots.end(); ++itr)
                            {
                                float Ang = me->GetAngle((*itr));
                                float x = me->GetPositionX() + frand(14.f, 17.f)*cos(Ang);
                                float y = me->GetPositionY() + frand(14.f, 17.f)*sin(Ang);
                                (*itr)->SetStandState(UNIT_STAND_STATE_STAND);
                                (*itr)->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                (*itr)->GetMotionMaster()->MovePoint(0, x, y, me->GetPositionZ());
                                (*itr)->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_ONESHOT_READY_UNARMED);
                            }
                            Events.ScheduleEvent(EVENT_SPAWN_KNIGHTS, 6000);
                            }
                            break;
                        case EVENT_SPAWN_KNIGHTS:
                            if (Creature* Mograine = me->SummonCreature(NPC_DARION_MOGRAINE, 6164.402f, 2694.072f, 573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            {
                                MograineGUID = Mograine->GetGUID();
                                Mograine->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Mograine->GetMotionMaster()->MovePoint(0, 6133.930f, 2758.642f, 573.914f);
                            }
                            if (Creature* Koltira = me->SummonCreature(NPC_KOLITRA_DEATHWEAVER, 6161.058f, 2692.482f, 573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            { 
                                Koltira->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Koltira->GetMotionMaster()->MovePoint(0, 6133.930f, 2758.642f, 573.914f);
                                if (isHorde)
                                    Koltira->AI()->Talk(SAY_KOLITRA_01);
                            }
                            if (Creature* Thassarian = me->SummonCreature(NPC_THASSARIAN, 6162.667f, 2688.819f, 573.914f, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            { 
                                Thassarian->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Thassarian->GetMotionMaster()->MovePoint(0, 6134.484f, 2757.386f, 573.911f);
                                if (!isHorde)
                                    Thassarian->AI()->Talk(SAY_THASSARIAN_01);
                            }
                            for (uint8 i = 0; i < 4; i++)
                            {
                                if (Creature* Knight = me->SummonCreature(NPC_EBON_KNIGHT, Summon[i].x, Summon[i].y, Summon[i].z, 1.987f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                                {
                                    Knight->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                    Knight->GetMotionMaster()->MovePoint(0, Summon[i].x2, Summon[i].y2, Summon[i].z2);
                                }
                            }
                            Events.ScheduleEvent(EVENT_SUMMON_PORTAL, 50000);
                            break;
                        case EVENT_SUMMON_PORTAL:
                            if (Creature* Mograine = me->GetCreature(*me, MograineGUID))
                            {
                                Mograine->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                Mograine->GetMotionMaster()->MovePoint(0, 6133.930f, 2758.642f, 573.914f);
                                Mograine->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY1H);
                            }
                            Events.ScheduleEvent(EVENT_DARION_TALK, 4000);
                            break;
                        case EVENT_DARION_TALK:
                            if (Creature* Mograine = me->GetCreature(*me, MograineGUID))
                            {
                                if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                {
                                    LichKing->SetStandState(UNIT_STAND_STATE_STAND);
                                    Mograine->SetFacingToObject(LichKing);
                                }
                                Mograine->AI()->Talk(SAY_DARION_01);
                            }
                            if (GameObject* goep = me->SummonGameObject(GO_ESCAPE_PORTAL, 6135.29f, 2755.13f, 573.914f, 1.15124f, 0.0f, 0.0f, 0.0f, 1, 32000))
                                goepGuid = goep->GetGUID();
                            Events.ScheduleEvent(EVENT_DARION_TALK_2, 9000);
                            break;
                        case EVENT_DARION_TALK_2:
                            if (isHorde)
                                if (Creature* Koltira = me->FindNearestCreature(NPC_KOLITRA_DEATHWEAVER, 50.0f, true))
                                    Koltira->AI()->Talk(SAY_KOLITRA_02);
                            if (!isHorde)
                                if (Creature* Thassarian = me->FindNearestCreature(NPC_THASSARIAN, 50.0f, true))
                                    Thassarian->AI()->Talk(SAY_THASSARIAN_02);
                            if (Creature* Mograine = me->GetCreature(*me, MograineGUID))
                                Mograine->AI()->Talk(SAY_DARION_02);
                            
                            DoCast(SPELL_QUEST_CREDIT);
                            Events.ScheduleEvent(EVENT_LICH_KING_S_FURY, 16000);
                            break;
                        case EVENT_LICH_KING_S_FURY:
                            if (Creature* LichKing = me->GetCreature(*me, LichKingGUID))
                                LichKing->CastSpell(LichKing, SPELL_LICH_KING_S_FURY);
                            Events.ScheduleEvent(EVENT_RESET, 5000);
                            break;
                        case EVENT_RESET:
                            if (GameObject* gofh = me->GetMap()->GetGameObject(gofhGuid))
                                gofh->Delete();
                            if (GameObject* goep = me->GetMap()->GetGameObject(goepGuid))
                                goep->Delete();
                            for (uint8 i = 0; i < 3; i++)
                            {
                                if (Creature* Escort = me->GetCreature(*me, EscortGUID[i]))
                                {
                                    Escort->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                    Escort->SetVirtualItem(0, uint32(0));
                                    Escort->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STAND);
                                    Escort->DespawnOrUnsummon();
                                }
                            }
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            Reset();
                            me->DespawnOrUnsummon();
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap Events;
            ObjectGuid gofhGuid;
            ObjectGuid goepGuid;
            ObjectGuid EscortGUID[3];
            ObjectGuid AcylteGUID[3];
            ObjectGuid LichKingGUID;
            ObjectGuid MograineGUID;
            bool isHorde;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_tg_tirion_fordringAI(creature);
        }

       bool OnGossipHello(Player* player, Creature* creature) override
       {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if ((player->GetQuestStatus(QUEST_TIRION_GAMBIT_A) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_TIRION_GAMBIT_H) == QUEST_STATUS_INCOMPLETE) && player->HasAura(SPELL_CULTIST_HOOD))
                    player->ADD_GOSSIP_ITEM(0, "Tirion, I'am ready!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                if (creature->AI())
                {
                    creature->AI()->DoAction(ACTION_START_ESCORT);

                    if (player->GetTeam() == HORDE)
                        creature->AI()->DoAction(ACTION_PLAYER_FACTION);
                }
                player->CLOSE_GOSSIP_MENU();
            }
            return true;
        }

};

class npc_tg_helper : public CreatureScript
{
    public:
        npc_tg_helper(): CreatureScript("npc_tg_helper"){}

        struct npc_tg_helperAI : ScriptedAI
        {
            npc_tg_helperAI(Creature* creature) : ScriptedAI(creature) { }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
            {
                if (damage > me->GetHealth())
                {
                    damage = me->GetHealth() - 1;
                    me->SetHealth(me->GetMaxHealth());
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_tg_helperAI(creature);
        }
};

void AddSC_icecrown()
{
    new npc_arete;
    new npc_squire_david;
    new npc_argent_valiant;
    new npc_guardian_pavilion;
    new npc_vereth_the_cunning;
    new npc_tournament_training_dummy;
    new npc_valiant;
    new npc_valis_windchaser;
    new npc_rugan_steelbelly;
    new npc_jeran_lockwood;
    new npc_squire_danny;
    new npc_argent_champion;
    new npc_training_dummy_argent;
    new npc_vendor_argent_tournament;
    new npc_tg_tirion_fordring();
    new npc_tg_helper();
}
