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
SDName: Borean_Tundra
SD%Complete: 100
SDComment: Quest support: 11708. Taxi vendors.
SDCategory: Borean Tundra
EndScriptData */

/* ContentData
npc_iruk
npc_corastrasza
npc_jenny
npc_sinkhole_kill_credit
npc_khunok_the_behemoth
npc_scourge_prisoner
mob_nerubar_victim
npc_keristrasza
npc_nesingwary_trapper
npc_lurgglbr
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"

/*######
## npc_sinkhole_kill_credit
######*/

enum eSinkhole
{
    SPELL_SET_CART                = 46797,
    SPELL_EXPLODE_CART            = 46799,
    SPELL_SUMMON_CART             = 46798,
    SPELL_SUMMON_WORM             = 46800,
};

class npc_sinkhole_kill_credit : public CreatureScript
{
public:
    npc_sinkhole_kill_credit() : CreatureScript("npc_sinkhole_kill_credit") {}

    struct npc_sinkhole_kill_creditAI : public ScriptedAI
    {
        npc_sinkhole_kill_creditAI(Creature* creature) : ScriptedAI(creature){}

        uint32 uiPhaseTimer;
        uint8  Phase;
        ObjectGuid casterGuid;

        void Reset() override
        {
            uiPhaseTimer = 500;
            Phase = 0;
            casterGuid.Clear();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (Phase)
                return;

            if (spell->Id == SPELL_SET_CART && caster->IsPlayer() && CAST_PLR(caster)->GetQuestStatus(11897) == QUEST_STATUS_INCOMPLETE)
            {
                Phase = 1;
                casterGuid = caster->GetGUID();
            }
        }

        void EnterCombat(Unit* /*who*/) override {}

        void UpdateAI(uint32 diff) override
        {
            if (!Phase)
                return;

            if (uiPhaseTimer <= diff)
            {
                switch (Phase)
                {
                    case 1:
                        DoCast(me, SPELL_EXPLODE_CART, true);
                        DoCast(me, SPELL_SUMMON_CART, true);

                        if (GameObject* cart = me->FindNearestGameObject(188160, 3))
                            cart->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, 14);

                        uiPhaseTimer = 3000;
                        Phase = 2;
                        break;
                    case 2:
                        if (GameObject* cart = me->FindNearestGameObject(188160, 3))
                            cart->UseDoorOrButton();

                        DoCast(me, SPELL_EXPLODE_CART, true);
                        uiPhaseTimer = 3000;
                        Phase = 3;
                        break;
                    case 3:
                        DoCast(me, SPELL_EXPLODE_CART, true);
                        uiPhaseTimer = 2000;
                        Phase = 4;
                    case 5:
                        DoCast(me, SPELL_SUMMON_WORM, true);
                        if (Unit* worm = me->FindNearestCreature(26250, 3))
                        {
                            worm->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            worm->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                        }
                        uiPhaseTimer = 1000;
                        Phase = 6;
                        break;
                    case 6:
                        DoCast(me, SPELL_EXPLODE_CART, true);
                        if (Unit* worm = me->FindNearestCreature(26250, 3))
                        {
                            me->Kill(worm);
                            worm->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                        }
                        uiPhaseTimer = 2000;
                        Phase = 7;
                        break;
                    case 7:
                        DoCast(me, SPELL_EXPLODE_CART, true);

                        if (Player* caster = Unit::GetPlayer(*me, casterGuid))
                            caster->KilledMonster(me->GetCreatureTemplate(), me->GetGUID());

                        uiPhaseTimer = 5000;
                        Phase = 8;
                        break;
                    case 8:
                        EnterEvadeMode();
                        break;
                }
            }
            else
                uiPhaseTimer -= diff;

        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sinkhole_kill_creditAI(creature);
    }
};

/*######
## npc_khunok_the_behemoth
######*/

class npc_khunok_the_behemoth : public CreatureScript
{
public:
    npc_khunok_the_behemoth() : CreatureScript("npc_khunok_the_behemoth") {}

    struct npc_khunok_the_behemothAI : public ScriptedAI
    {
        npc_khunok_the_behemothAI(Creature* creature) : ScriptedAI(creature) {}

        void MoveInLineOfSight(Unit* who) override
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (!who->IsUnit())
                return;

            if (who->GetEntry() == 25861 && me->IsWithinDistInMap(who, 10.0f))
            {
                if (Unit* owner = who->GetOwner())
                {
                    if (owner->IsPlayer())
                    {
                        owner->CastSpell(owner, 46231, true);
                        CAST_CRE(who)->DespawnOrUnsummon();
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_khunok_the_behemothAI(creature);
    }
};

/*######
## npc_keristrasza
######*/

enum eKeristrasza
{
    SPELL_TELEPORT_TO_SARAGOSA = 46772
};

#define GOSSIP_HELLO_KERI   "I am prepared to face Saragosa!"

class npc_keristrasza : public CreatureScript
{
public:
    npc_keristrasza() : CreatureScript("npc_keristrasza") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(11957) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_KERI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player, SPELL_TELEPORT_TO_SARAGOSA, true);
        }
        return true;
    }
};

/*######
## npc_corastrasza
######*/

#define GOSSIP_ITEM_C_1 "I... I think so..."

enum eCorastrasza
{
    SPELL_SUMMON_WYRMREST_SKYTALON               = 61240,
    SPELL_WYRMREST_SKYTALON_RIDE_PERIODIC        = 61244,

    QUEST_ACES_HIGH_DAILY                        = 13414,
    QUEST_ACES_HIGH                              = 13413
};

class npc_corastrasza : public CreatureScript
{
public:
    npc_corastrasza() : CreatureScript("npc_corastrasza") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_ACES_HIGH) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_ACES_HIGH_DAILY) == QUEST_STATUS_INCOMPLETE) //It's the same dragon for both quests.
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_C_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            player->CLOSE_GOSSIP_MENU();

            player->CastSpell(player, SPELL_SUMMON_WYRMREST_SKYTALON, true);
            player->CastSpell(player, SPELL_WYRMREST_SKYTALON_RIDE_PERIODIC, true);

        }

        return true;
    }
};

/*######
## npc_iruk
######*/

#define GOSSIP_ITEM_I  "<Search corpse for Issliruk's Totem.>"

enum eIruk
{
    QUEST_SPIRITS_WATCH_OVER_US             = 11961,
    SPELL_CREATURE_TOTEM_OF_ISSLIRUK        = 46816,
    GOSSIP_TEXT_I                           = 12585
};

class npc_iruk : public CreatureScript
{
public:
    npc_iruk() : CreatureScript("npc_iruk") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(QUEST_SPIRITS_WATCH_OVER_US) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_I, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_I, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->CastSpell(player, SPELL_CREATURE_TOTEM_OF_ISSLIRUK, true);
                player->CLOSE_GOSSIP_MENU();
                break;
        }
        return true;
    }
};

/*######
## mob_nerubar_victim
######*/

#define WARSONG_PEON        25270

const uint32 nerubarVictims[3] =
{
    45526, 45527, 45514
};

class mob_nerubar_victim : public CreatureScript
{
public:
    mob_nerubar_victim() : CreatureScript("mob_nerubar_victim") {}

    struct mob_nerubar_victimAI : public ScriptedAI
    {
        mob_nerubar_victimAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override {}
        void EnterCombat(Unit* /*who*/) override {}
        void MoveInLineOfSight(Unit* /*who*/) override {}

        void JustDied(Unit* killer) override
        {
            Player* player = killer->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(11611) == QUEST_STATUS_INCOMPLETE)
            {
                uint8 uiRand = urand(0, 99);
                if (uiRand < 25)
                {
                    player->CastSpell(me, 45532, true);
                    player->KilledMonsterCredit(WARSONG_PEON, ObjectGuid::Empty);
                }
                else if (uiRand < 75)
                    player->CastSpell(me, nerubarVictims[urand(0, 2)], true);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_nerubar_victimAI(creature);
    }
};

/*######
## npc_scourge_prisoner
######*/

enum eScourgePrisoner
{
    GO_SCOURGE_CAGE = 187867
};

class npc_scourge_prisoner : public CreatureScript
{
public:
    npc_scourge_prisoner() : CreatureScript("npc_scourge_prisoner") {}

    struct npc_scourge_prisonerAI : public ScriptedAI
    {
        npc_scourge_prisonerAI(Creature* creature) : ScriptedAI (creature){}

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);

            if (GameObject* go = me->FindNearestGameObject(GO_SCOURGE_CAGE, 5.0f))
                if (go->GetGoState() == GO_STATE_ACTIVE)
                    go->SetGoState(GO_STATE_READY);
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scourge_prisonerAI(creature);
    }
};

/*######
## npc_jenny
######*/

enum eJenny
{
    QUEST_LOADER_UP             = 11881,

    NPC_FEZZIX_GEARTWIST        = 25849,
    NPC_JENNY                   = 25969,

    SPELL_GIVE_JENNY_CREDIT     = 46358,
    SPELL_CRATES_CARRIED        = 46340,
    SPELL_DROP_CRATE            = 46342
};

class npc_jenny : public CreatureScript
{
public:
    npc_jenny() : CreatureScript("npc_jenny") {}

    struct npc_jennyAI : public ScriptedAI
    {
        npc_jennyAI(Creature* creature) : ScriptedAI(creature) {}

        bool setCrateNumber;

        void Reset() override
        {
            if (!setCrateNumber)
                setCrateNumber = true;

            me->SetReactState(REACT_PASSIVE);

            switch (CAST_PLR(me->GetOwner())->GetTeamId())
            {
                case TEAM_ALLIANCE:
                    me->setFaction(FACTION_ESCORT_A_NEUTRAL_ACTIVE);
                    break;
                default:
                case TEAM_HORDE:
                    me->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);
                    break;
            }
        }

        void DamageTaken(Unit* /*pDone_by*/, uint32& /*uiDamage*/, DamageEffectType /*dmgType*/) override
        {
            DoCast(me, SPELL_DROP_CRATE, true);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (setCrateNumber)
            {
                me->AddAura(SPELL_CRATES_CARRIED, me);
                setCrateNumber = false;
            }

            if (!setCrateNumber && !me->HasAura(SPELL_CRATES_CARRIED))
                me->DisappearAndDie();

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jennyAI (creature);
    }
};

/*######
## npc_fezzix_geartwist
######*/

class npc_fezzix_geartwist : public CreatureScript
{
public:
    npc_fezzix_geartwist() : CreatureScript("npc_fezzix_geartwist") {}

    struct npc_fezzix_geartwistAI : public ScriptedAI
    {
        npc_fezzix_geartwistAI(Creature* creature) : ScriptedAI(creature) {}

        void MoveInLineOfSight(Unit* who) override
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (!who->IsUnit())
                return;

            if (who->GetEntry() == NPC_JENNY && me->IsWithinDistInMap(who, 10.0f))
            {
                if (Unit* owner = who->GetOwner())
                {
                    if (owner->IsPlayer())
                    {
                        if (who->HasAura(SPELL_CRATES_CARRIED))
                        {
                            owner->CastSpell(owner, SPELL_GIVE_JENNY_CREDIT, true); // Maybe is not working.
                            CAST_PLR(owner)->CompleteQuest(QUEST_LOADER_UP);
                            CAST_CRE(who)->DisappearAndDie();
                        }
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fezzix_geartwistAI(creature);
    }
};

/*######
## npc_nesingwary_trapper
######*/

enum eNesingwaryTrapper
{
    GO_HIGH_QUALITY_FUR = 187983,

    GO_CARIBOU_TRAP_1   = 187982,
    GO_CARIBOU_TRAP_2   = 187995,
    GO_CARIBOU_TRAP_3   = 187996,
    GO_CARIBOU_TRAP_4   = 187997,
    GO_CARIBOU_TRAP_5   = 187998,
    GO_CARIBOU_TRAP_6   = 187999,
    GO_CARIBOU_TRAP_7   = 188000,
    GO_CARIBOU_TRAP_8   = 188001,
    GO_CARIBOU_TRAP_9   = 188002,
    GO_CARIBOU_TRAP_10  = 188003,
    GO_CARIBOU_TRAP_11  = 188004,
    GO_CARIBOU_TRAP_12  = 188005,
    GO_CARIBOU_TRAP_13  = 188006,
    GO_CARIBOU_TRAP_14  = 188007,
    GO_CARIBOU_TRAP_15  = 188008,

    SPELL_TRAPPED       = 46104,
};

#define CaribouTrapsNum 15
const uint32 CaribouTraps[CaribouTrapsNum] =
{
    GO_CARIBOU_TRAP_1, GO_CARIBOU_TRAP_2, GO_CARIBOU_TRAP_3, GO_CARIBOU_TRAP_4, GO_CARIBOU_TRAP_5,
    GO_CARIBOU_TRAP_6, GO_CARIBOU_TRAP_7, GO_CARIBOU_TRAP_8, GO_CARIBOU_TRAP_9, GO_CARIBOU_TRAP_10,
    GO_CARIBOU_TRAP_11, GO_CARIBOU_TRAP_12, GO_CARIBOU_TRAP_13, GO_CARIBOU_TRAP_14, GO_CARIBOU_TRAP_15,
};

class npc_nesingwary_trapper : public CreatureScript
{
public:
    npc_nesingwary_trapper() : CreatureScript("npc_nesingwary_trapper") {}

    struct npc_nesingwary_trapperAI : public ScriptedAI
    {
        npc_nesingwary_trapperAI(Creature* creature) : ScriptedAI(creature) { creature->SetVisible(false); }

        ObjectGuid go_caribouGUID;
        uint8  Phase;
        uint32 uiPhaseTimer;

        void Reset() override
        {
            me->SetVisible(false);
            uiPhaseTimer = 2500;
            Phase = 1;
            go_caribouGUID.Clear();
        }

        void EnterCombat(Unit* /*who*/) override {}
        void MoveInLineOfSight(Unit* /*who*/) override {}

        void JustDied(Unit* /*killer*/) override
        {
            if (GameObject* go_caribou = me->GetMap()->GetGameObject(go_caribouGUID))
                go_caribou->SetLootState(GO_JUST_DEACTIVATED);

            if (TempSummon* summon = me->ToTempSummon())
                if (summon->isSummon())
                    if (Unit* temp = summon->GetSummoner())
                        if (temp->GetTypeId() == TYPEID_PLAYER)
                            CAST_PLR(temp)->KilledMonsterCredit(me->GetEntry(), ObjectGuid::Empty);

            if (GameObject* go_caribou = me->GetMap()->GetGameObject(go_caribouGUID))
                go_caribou->SetGoState(GO_STATE_READY);
        }

        void UpdateAI(uint32 diff) override
        {
            if (uiPhaseTimer <= diff)
            {
                switch (Phase)
                {
                    case 1:
                        me->SetVisible(true);
                        uiPhaseTimer = 2000;
                        Phase = 2;
                        break;
                    case 2:
                        if (GameObject* go_fur = me->FindNearestGameObject(GO_HIGH_QUALITY_FUR, 11.0f))
                            me->GetMotionMaster()->MovePoint(0, go_fur->GetPositionX(), go_fur->GetPositionY(), go_fur->GetPositionZ());
                        uiPhaseTimer = 1500;
                        Phase = 3;
                        break;
                    case 3:
                        uiPhaseTimer = 2000;
                        Phase = 4;
                        break;
                    case 4:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LOOT);
                        uiPhaseTimer = 1000;
                        Phase = 5;
                        break;
                    case 5:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
                        uiPhaseTimer = 500;
                        Phase = 6;
                        break;
                    case 6:
                        if (GameObject* go_fur = me->FindNearestGameObject(GO_HIGH_QUALITY_FUR, 11.0f))
                            go_fur->Delete();
                        uiPhaseTimer = 500;
                        Phase = 7;
                        break;
                    case 7:
                    {
                        GameObject* go_caribou = NULL;
                        for (uint8 i = 0; i < CaribouTrapsNum; ++i)
                        {
                            go_caribou = me->FindNearestGameObject(CaribouTraps[i], 5.0f);
                            if (go_caribou)
                            {
                                go_caribou->SetGoState(GO_STATE_ACTIVE);
                                go_caribouGUID = go_caribou->GetGUID();
                                break;
                            }
                        }
                        Phase = 8;
                        uiPhaseTimer = 1000;
                    }
                    break;
                    case 8:
                        DoCast(me, SPELL_TRAPPED, true);
                        Phase = 0;
                        break;
                }
            }
            else
                uiPhaseTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_nesingwary_trapperAI(creature);
    }
};

/*######
## npc_lurgglbr
######*/

enum eLurgglbr
{
    QUEST_ESCAPE_WINTERFIN_CAVERNS      = 11570,
    GO_CAGE                             = 187369,
    FACTION_ESCORTEE_A                  = 774,
    FACTION_ESCORTEE_H                  = 775
};

class npc_lurgglbr : public CreatureScript
{
public:
    npc_lurgglbr() : CreatureScript("npc_lurgglbr") {}

    struct npc_lurgglbrAI : public npc_escortAI
    {
        npc_lurgglbrAI(Creature* creature) : npc_escortAI(creature){}

        uint32 IntroTimer;
        uint32 IntroPhase;

        void Reset() override
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
            {
                IntroTimer = 0;
                IntroPhase = 0;
            }
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
                case 0:
                    IntroPhase = 1;
                    IntroTimer = 2000;
                    break;
                case 41:
                    IntroPhase = 4;
                    IntroTimer = 2000;
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (IntroPhase)
            {
                if (IntroTimer <= diff)
                {
                    switch (IntroPhase)
                    {
                        case 1:
                            IntroPhase = 2;
                            IntroTimer = 7500;
                            break;
                        case 2:
                            IntroPhase = 3;
                            IntroTimer = 7500;
                            break;
                        case 3:
                            me->SetReactState(REACT_AGGRESSIVE);
                            IntroPhase = 0;
                            IntroTimer = 0;
                            break;
                        case 4:
                            IntroPhase = 5;
                            IntroTimer = 8000;
                            break;
                        case 5:
                            IntroPhase = 6;
                            IntroTimer = 2500;
                            break;

                        case 6:
                            if (Player* player = GetPlayerForEscort())
                                player->AreaExploredOrEventHappens(QUEST_ESCAPE_WINTERFIN_CAVERNS);

                            IntroPhase = 7;
                            IntroTimer = 2500;
                            break;

                        case 7:
                            me->DespawnOrUnsummon();
                            IntroPhase = 0;
                            IntroTimer = 0;
                            break;
                    }
                } else IntroTimer -= diff;
            }
            npc_escortAI::UpdateAI(diff);

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lurgglbrAI(creature);
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_ESCAPE_WINTERFIN_CAVERNS)
        {
            if (GameObject* go = creature->FindNearestGameObject(GO_CAGE, 5.0f))
            {
                go->SetRespawnTime(0);
                go->SetGoType(GAMEOBJECT_TYPE_BUTTON);
                go->UseDoorOrButton(20);
            }

            if (npc_escortAI* pEscortAI = CAST_AI(npc_lurgglbr::npc_lurgglbrAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());

            switch (player->GetTeam())
            {
                case ALLIANCE:
                    creature->setFaction(FACTION_ESCORTEE_A);
                    break;
                default:
                case HORDE:
                    creature->setFaction(FACTION_ESCORTEE_H);
                    break;
            }

            return true;
        }
        return false;
    }
};

/*######
## npc_thassarian
######*/

enum eThassarian
{
    QUEST_LAST_RITES        = 12019,

    SPELL_TRANSFORM_VALANAR = 46753,
    SPELL_STUN              = 46957,
    SPELL_SHADOW_BOLT       = 15537,

    NPC_IMAGE_LICH_KING     = 26203,
    NPC_COUNSELOR_TALBOT    = 25301,
    NPC_PRINCE_VALANAR      = 28189,
    NPC_GENERAL_ARLOS       = 25250,
    NPC_LERYSSA             = 25251,

    SAY_TALBOT_1            = 0,
    SAY_LICH_1              = 2,
    SAY_TALBOT_2            = 3,
    SAY_THASSARIAN_1        = 6,
    SAY_THASSARIAN_2        = 0,
    SAY_LICH_2              = 0,
    SAY_THASSARIAN_3        = 1,
    SAY_TALBOT_3            = 2,
    SAY_LICH_3              = 1,
    SAY_TALBOT_4            = 3,
    SAY_ARLOS_1             = 0,
    SAY_ARLOS_2             = 1,
    SAY_LERYSSA_1           = 0,
    SAY_THASSARIAN_4        = 2,
    SAY_LERYSSA_2           = 1,
    SAY_THASSARIAN_5        = 3,
    SAY_LERYSSA_3           = 2,
    SAY_THASSARIAN_6        = 4,
    SAY_LERYSSA_4           = 3,
    SAY_THASSARIAN_7        = 5
};

#define GOSSIP_ITEM_T   "Let's do this, Thassarian. It's now or never."

class npc_thassarian : public CreatureScript
{
public:
    npc_thassarian() : CreatureScript("npc_thassarian") {}

    struct npc_thassarianAI : public npc_escortAI
    {
        npc_thassarianAI(Creature* creature) : npc_escortAI(creature) {}

        ObjectGuid uiArthas;
        ObjectGuid uiTalbot;
        ObjectGuid uiLeryssa;
        ObjectGuid uiArlos;

        bool bArthasInPosition;
        bool bArlosInPosition;
        bool bLeryssaInPosition;
        bool bTalbotInPosition;

        uint32 uiPhase;
        uint32 uiPhaseTimer;

        void Reset() override
        {
            me->RestoreFaction();
            me->RemoveStandStateFlags(UNIT_STAND_STATE_SIT);

            uiArthas.Clear();
            uiTalbot.Clear();
            uiLeryssa.Clear();
            uiArlos.Clear();

            bArthasInPosition           = false;
            bArlosInPosition            = false;
            bLeryssaInPosition          = false;
            bTalbotInPosition           = false;

            uiPhase = 0;
            uiPhaseTimer = 0;
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 3:
                    SetEscortPaused(true);
                    if (Creature* pArthas = me->SummonCreature(NPC_IMAGE_LICH_KING, 3730.313f, 3518.689f, 473.324f, 1.562f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000))
                    {
                        uiArthas = pArthas->GetGUID();
                        pArthas->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        pArthas->SetReactState(REACT_PASSIVE);
                        pArthas->SetWalk(true);
                        pArthas->GetMotionMaster()->MovePoint(0, 3737.374756f, 3564.841309f, 477.433014f);
                    }
                    if (Creature* pTalbot = me->SummonCreature(NPC_COUNSELOR_TALBOT, 3747.23f, 3614.936f, 473.321f, 4.462012f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000))
                    {
                        uiTalbot = pTalbot->GetGUID();
                        pTalbot->SetWalk(true);
                        pTalbot->GetMotionMaster()->MovePoint(0, 3738.000977f, 3568.882080f, 477.433014f);
                    }
                    me->SetWalk(false);
                    break;
                case 4:
                    SetEscortPaused(true);
                    uiPhase = 7;
                    break;
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (bArthasInPosition && bTalbotInPosition)
            {
                uiPhase = 1;
                bArthasInPosition = false;
                bTalbotInPosition = false;
            }

            if (bArlosInPosition && bLeryssaInPosition)
            {
                bArlosInPosition   = false;
                bLeryssaInPosition = false;
                Talk(SAY_THASSARIAN_1);
                SetEscortPaused(false);
            }

            if (uiPhaseTimer <= uiDiff)
            {
                Creature* pTalbot = me->GetCreature(*me, uiTalbot);
                Creature* pArthas = me->GetCreature(*me, uiArthas);
                switch (uiPhase)
                {
                    case 1:
                        if (pTalbot)
                            pTalbot->SetStandState(UNIT_STAND_STATE_KNEEL);
                        uiPhaseTimer = 3000;
                        ++uiPhase;
                        break;

                    case 2:
                        if (pTalbot)
                        {
                            pTalbot->UpdateEntry(NPC_PRINCE_VALANAR, ALLIANCE);
                            pTalbot->setFaction(14);
                            pTalbot->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            pTalbot->SetReactState(REACT_PASSIVE);
                        }
                        uiPhaseTimer = 5000;
                        ++uiPhase;
                        break;

                    case 3:
                        if (pTalbot)
                            pTalbot->AI()->Talk(SAY_TALBOT_1);
                        uiPhaseTimer = 5000;
                        ++uiPhase;
                        break;

                    case 4:
                        if (pArthas)
                            pTalbot->AI()->Talk(SAY_LICH_1);
                        uiPhaseTimer = 5000;
                        ++uiPhase;
                        break;

                    case 5:
                        if (pTalbot)
                            pTalbot->AI()->Talk(SAY_TALBOT_2);
                        uiPhaseTimer = 5000;
                        ++uiPhase;
                        break;

                    case 6:
                        if (Creature* pArlos = me->SummonCreature(NPC_GENERAL_ARLOS, 3745.527100f, 3615.655029f, 473.321533f, 4.447805f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000))
                        {
                            uiArlos = pArlos->GetGUID();
                            pArlos->SetWalk(true);
                            pArlos->GetMotionMaster()->MovePoint(0, 3735.570068f, 3572.419922f, 477.441010f);
                        }
                        if (Creature* pLeryssa = me->SummonCreature(NPC_LERYSSA, 3749.654541f, 3614.959717f, 473.323486f, 4.524959f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000))
                        {
                            uiLeryssa = pLeryssa->GetGUID();
                            pLeryssa->SetWalk(false);
                            pLeryssa->SetReactState(REACT_PASSIVE);
                            pLeryssa->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            pLeryssa->GetMotionMaster()->MovePoint(0, 3741.969971f, 3571.439941f, 477.441010f);
                        }
                        uiPhaseTimer = 2000;
                        uiPhase = 0;
                        break;

                    case 7:
                        Talk(SAY_THASSARIAN_2);
                        uiPhaseTimer = 5000;
                        ++uiPhase;
                        break;

                    case 8:
                        if (pArthas && pTalbot)
                        {
                            pArthas->SetInFront(me); //The client doesen't update with the new orientation :l
                            pTalbot->SetStandState(UNIT_STAND_STATE_STAND);
                            pArthas->AI()->Talk(SAY_LICH_2);
                        }
                        uiPhaseTimer = 5000;
                        uiPhase = 9;
                        break;

                   case 9:
                        Talk(SAY_THASSARIAN_3);
                        uiPhaseTimer = 5000;
                        uiPhase = 10;
                        break;

                   case 10:
                        if (pTalbot)
                            pTalbot->AI()->Talk(SAY_TALBOT_3);
                        uiPhaseTimer = 5000;
                        uiPhase = 11;
                        break;

                   case 11:
                        if (pArthas)
                            pArthas->AI()->Talk(SAY_LICH_3);
                        uiPhaseTimer = 5000;
                        uiPhase = 12;
                        break;

                    case 12:
                        if (pTalbot)
                            pTalbot->AI()->Talk(SAY_TALBOT_4);
                        uiPhaseTimer = 2000;
                        uiPhase = 13;
                        break;

                    case 13:
                        if (pArthas)
                            pArthas->RemoveFromWorld();
                        ++uiPhase;
                        break;

                    case 14:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        if (pTalbot)
                        {
                            pTalbot->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            pTalbot->SetReactState(REACT_AGGRESSIVE);
                            pTalbot->CastSpell(me, SPELL_SHADOW_BOLT, false);
                        }
                        uiPhaseTimer = 1500;
                        ++uiPhase;
                        break;

                    case 15:
                        me->SetReactState(REACT_AGGRESSIVE);
                        AttackStart(pTalbot);
                        uiPhase = 0;
                        break;

                    case 16:
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        uiPhaseTimer = 20000;
                        ++uiPhase;
                        break;

                   case 17:
                        if (Creature* pLeryssa = me->GetCreature(*me, uiLeryssa))
                            pLeryssa->RemoveFromWorld();
                        if (Creature* pArlos= me->GetCreature(*me, uiArlos))
                            pArlos->RemoveFromWorld();
                        if (pTalbot)
                            pTalbot->RemoveFromWorld();
                        me->RemoveStandStateFlags(UNIT_STAND_STATE_SIT);
                        SetEscortPaused(false);
                        uiPhaseTimer = 0;
                        uiPhase = 0;
                }
            }
            else
                uiPhaseTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Creature* pTalbot = me->GetCreature(*me, uiTalbot))
                pTalbot->RemoveFromWorld();

            if (Creature* pLeryssa = me->GetCreature(*me, uiLeryssa))
                pLeryssa->RemoveFromWorld();

            if (Creature* pArlos = me->GetCreature(*me, uiArlos))
                pArlos->RemoveFromWorld();

            if (Creature* pArthas = me->GetCreature(*me, uiArthas))
                pArthas->RemoveFromWorld();
        }
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_LAST_RITES) == QUEST_STATUS_INCOMPLETE && creature->GetAreaId() == 4128)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_T, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                CAST_AI(npc_escortAI, (creature->AI()))->Start(true, false, player->GetGUID());
                CAST_AI(npc_escortAI, (creature->AI()))->SetMaxPlayerDistance(200.0f);
                break;
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_thassarianAI(creature);
    }
};

/*######
## npc_image_lich_king
######*/

class npc_image_lich_king : public CreatureScript
{
public:
    npc_image_lich_king() : CreatureScript("npc_image_lich_king") { }

    struct npc_image_lich_kingAI : public ScriptedAI
    {
        npc_image_lich_kingAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            me->RestoreFaction();
        }

        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (me->isSummon())
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    CAST_AI(npc_thassarian::npc_thassarianAI, CAST_CRE(summoner)->AI())->bArthasInPosition = true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_image_lich_kingAI(creature);
    }
};

/*######
## npc_general_arlos
######*/

class npc_general_arlos : public CreatureScript
{
public:
    npc_general_arlos() : CreatureScript("npc_general_arlos") { }

    struct npc_general_arlosAI : public ScriptedAI
    {
        npc_general_arlosAI(Creature* creature) : ScriptedAI(creature) {}

        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            me->AddUnitState(UNIT_STATE_STUNNED);
            me->CastSpell(me, SPELL_STUN, true);
            if (me->isSummon())
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    CAST_AI(npc_thassarian::npc_thassarianAI, CAST_CRE(summoner)->AI())->bArlosInPosition = true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_general_arlosAI(creature);
    }
};

/*######
## npc_counselor_talbot
######*/

enum eCounselorTalbot
{
    SPELL_DEFLECTION    = 51009,
    SPELL_SOUL_BLAST    = 50992,
};

class npc_counselor_talbot : public CreatureScript
{
public:
    npc_counselor_talbot() : CreatureScript("npc_counselor_talbot") { }

    struct npc_counselor_talbotAI : public ScriptedAI
    {
        npc_counselor_talbotAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->RestoreFaction();
        }

        ObjectGuid LeryssaGUID;
        ObjectGuid ArlosGUID;

        bool bCheck;

        uint32 uiShadowBoltTimer;
        uint32 uiDeflectionTimer;
        uint32 uiSoulBlastTimer;

        void Reset() override
        {
            LeryssaGUID.Clear();
            ArlosGUID.Clear();
            bCheck              = false;
            uiShadowBoltTimer   = urand(5000, 12000);
            uiDeflectionTimer   = urand(20000, 25000);
            uiSoulBlastTimer    = urand (12000, 18000);
        }
        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (me->isSummon())
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    CAST_AI(npc_thassarian::npc_thassarianAI, CAST_CRE(summoner)->AI())->bTalbotInPosition = true;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bCheck)
            {
                if (Creature* pLeryssa = me->FindNearestCreature(NPC_LERYSSA, 50.0f, true))
                    LeryssaGUID = pLeryssa->GetGUID();
                if (Creature* pArlos = me->FindNearestCreature(NPC_GENERAL_ARLOS, 50.0f, true))
                    ArlosGUID = pArlos->GetGUID();
                bCheck = false;
            }

            if (!UpdateVictim())
                return;

            if (me->GetAreaId() == 4125)
            {
                if (uiShadowBoltTimer <= uiDiff)
                {
                    DoCast(me->getVictim(), SPELL_SHADOW_BOLT);
                    uiShadowBoltTimer = urand(5000, 12000);
                } else uiShadowBoltTimer -= uiDiff;

                if (uiDeflectionTimer <= uiDiff)
                {
                    DoCast(me->getVictim(), SPELL_DEFLECTION);
                    uiDeflectionTimer = urand(20000, 25000);
                } else uiDeflectionTimer -= uiDiff;

                if (uiSoulBlastTimer <= uiDiff)
                {
                    DoCast(me->getVictim(), SPELL_SOUL_BLAST);
                    uiSoulBlastTimer  = urand (12000, 18000);
                } else uiSoulBlastTimer -= uiDiff;
            }

            DoMeleeAttackIfReady();
       }

       void JustDied(Unit* killer) override
       {
            if (!LeryssaGUID || !ArlosGUID)
                return;

            Creature* pLeryssa = Unit::GetCreature(*me, LeryssaGUID);
            Creature* pArlos = Unit::GetCreature(*me, ArlosGUID);
            if (!pLeryssa || !pArlos)
                return;

            pArlos->AI()->Talk(SAY_ARLOS_1);
            pArlos->AI()->Talk(SAY_ARLOS_2);
            pLeryssa->AI()->Talk(SAY_LERYSSA_1);
            pArlos->Kill(pArlos, false);
            pLeryssa->RemoveAura(SPELL_STUN);
            pLeryssa->ClearUnitState(UNIT_STATE_STUNNED);
            pLeryssa->SetWalk(false);
            pLeryssa->GetMotionMaster()->MovePoint(0, 3722.114502f, 3564.201660f, 477.441437f);

            if (Player* player = killer->ToPlayer())
                player->RewardPlayerAndGroupAtEvent(NPC_PRINCE_VALANAR, 0);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_counselor_talbotAI(creature);
    }
};

/*######
## npc_leryssa
######*/

class npc_leryssa : public CreatureScript
{
public:
    npc_leryssa() : CreatureScript("npc_leryssa") { }

    struct npc_leryssaAI : public ScriptedAI
    {
        npc_leryssaAI(Creature* creature) : ScriptedAI(creature)
        {
            bDone = false;
            Phase = 0;
            uiPhaseTimer = 0;

            creature->RemoveStandStateFlags(UNIT_STAND_STATE_SIT);
        }

        bool bDone;

        uint32 Phase;
        uint32 uiPhaseTimer;

        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            if (!bDone)
            {
                if (Creature* pTalbot = me->FindNearestCreature(NPC_PRINCE_VALANAR, 50.0f, true))
                    CAST_AI(npc_counselor_talbot::npc_counselor_talbotAI, pTalbot->GetAI())->bCheck = true;

                me->AddUnitState(UNIT_STATE_STUNNED);
                me->CastSpell(me, SPELL_STUN, true);

                if (me->isSummon())
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                        CAST_AI(npc_thassarian::npc_thassarianAI, summoner->GetAI())->bLeryssaInPosition = true;
                bDone = true;
            }
            else
            {
                me->SetStandState(UNIT_STAND_STATE_SIT);
                if (me->isSummon())
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    summoner->SetStandState(UNIT_STAND_STATE_SIT);
                uiPhaseTimer = 1500;
                Phase = 1;
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            ScriptedAI::UpdateAI(uiDiff);

            if (uiPhaseTimer <= uiDiff)
            {
                switch (Phase)
                {
                    case 1:
                        if (me->isSummon())
                            if (Unit* pThassarian = me->ToTempSummon()->GetSummoner())
                                pThassarian->ToCreature()->AI()->Talk(SAY_THASSARIAN_4);
                        uiPhaseTimer = 5000;
                        ++Phase;
                        break;
                    case 2:
                        Talk(SAY_LERYSSA_2);
                        uiPhaseTimer = 5000;
                        ++Phase;
                        break;
                    case 3:
                        if (me->isSummon())
                            if (Unit* pThassarian = me->ToTempSummon()->GetSummoner())
                                pThassarian->ToCreature()->AI()->Talk(SAY_THASSARIAN_5);
                        uiPhaseTimer = 5000;
                        ++Phase;
                        break;
                    case 4:
                        Talk(SAY_LERYSSA_3);
                        uiPhaseTimer = 5000;
                        ++Phase;
                        break;
                    case 5:
                        if (me->isSummon())
                            if (Unit* pThassarian = me->ToTempSummon()->GetSummoner())
                                pThassarian->ToCreature()->AI()->Talk(SAY_THASSARIAN_6);
                        uiPhaseTimer = 5000;
                        ++Phase;
                        break;

                    case 6:
                        Talk(SAY_LERYSSA_4);
                        uiPhaseTimer = 5000;
                        ++Phase;
                        break;
                    case 7:
                        if (me->isSummon())
                            if (Unit* pThassarian = me->ToTempSummon()->GetSummoner())
                            {
                                pThassarian->ToCreature()->AI()->Talk(SAY_THASSARIAN_7);
                                CAST_AI(npc_thassarian::npc_thassarianAI, pThassarian->GetAI())->uiPhase = 16;
                            }
                        uiPhaseTimer = 5000;
                        Phase = 0;
                        break;
                }
            }
            else
                uiPhaseTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_leryssaAI(creature);
    }
};

/*######
## npc_beryl_sorcerer
######*/

enum eBerylSorcerer
{
    NPC_CAPTURED_BERLY_SORCERER         = 25474,
    NPC_LIBRARIAN_DONATHAN              = 25262,

    SPELL_ARCANE_CHAINS                 = 45611,
    SPELL_COSMETIC_CHAINS               = 54324,
    SPELL_COSMETIC_ENSLAVE_CHAINS_SELF  = 45631
};

class npc_beryl_sorcerer : public CreatureScript
{
public:
    npc_beryl_sorcerer() : CreatureScript("npc_beryl_sorcerer") { }

    struct npc_beryl_sorcererAI : public FollowerAI
    {
        npc_beryl_sorcererAI(Creature* creature) : FollowerAI(creature) {}

        bool bEnslaved;

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
            bEnslaved = false;
        }

        void EnterCombat(Unit* who) override
        {
            if (me->IsValidAttackTarget(who))
                AttackStart(who);
        }

        void SpellHit(Unit* pCaster, const SpellInfo* pSpell) override
        {
            if (pSpell->Id == SPELL_ARCANE_CHAINS && pCaster->GetTypeId() == TYPEID_PLAYER && !HealthAbovePct(50) && !bEnslaved)
            {
                EnterEvadeMode(); //We make sure that the npc is not attacking the player!
                me->SetReactState(REACT_PASSIVE);
                StartFollow(pCaster->ToPlayer(), 0, NULL);
                me->UpdateEntry(NPC_CAPTURED_BERLY_SORCERER, TEAM_NEUTRAL);
                DoCast(me, SPELL_COSMETIC_ENSLAVE_CHAINS_SELF, true);

                if (Player* player = pCaster->ToPlayer())
                    player->KilledMonsterCredit(NPC_CAPTURED_BERLY_SORCERER, ObjectGuid::Empty);

                bEnslaved = true;
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            FollowerAI::MoveInLineOfSight(who);

            if (who->GetEntry() == NPC_LIBRARIAN_DONATHAN && me->IsWithinDistInMap(who, INTERACTION_DISTANCE))
            {
                SetFollowComplete();
                me->DisappearAndDie();
            }
        }

        void UpdateAI(uint32 /*uiDiff*/) override
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_beryl_sorcererAI(creature);
    }
};

/*######
## npc_imprisoned_beryl_sorcerer
######*/

enum eImprisionedBerylSorcerer
{
    SPELL_NEURAL_NEEDLE             = 45634,
    NPC_IMPRISONED_BERYL_SORCERER   = 25478,
    SAY_IMPRISIONED_BERYL_1         = 0,
    SAY_IMPRISIONED_BERYL_2         = 1,
    SAY_IMPRISIONED_BERYL_3         = 2,
    SAY_IMPRISIONED_BERYL_4         = 3,
    SAY_IMPRISIONED_BERYL_5         = 4,
    SAY_IMPRISIONED_BERYL_6         = 5,
    SAY_IMPRISIONED_BERYL_7         = 6
};

class npc_imprisoned_beryl_sorcerer : public CreatureScript
{
public:
    npc_imprisoned_beryl_sorcerer() : CreatureScript("npc_imprisoned_beryl_sorcerer") {}

    struct npc_imprisoned_beryl_sorcererAI : public ScriptedAI
    {
        npc_imprisoned_beryl_sorcererAI(Creature* creature) : ScriptedAI(creature) {}

        ObjectGuid CasterGUID;
        uint32 rebuff;
    
    void Reset() override
    {
        if (me->GetReactState() != REACT_PASSIVE)
            me->SetReactState(REACT_PASSIVE);

        rebuff = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateVictim();

        if (rebuff <= diff)
        {
            if (!me->HasAura(SPELL_COSMETIC_ENSLAVE_CHAINS_SELF))
                DoCast(me, SPELL_COSMETIC_ENSLAVE_CHAINS_SELF, false);

            rebuff = 180000;
        }
        else
            rebuff -= diff;

        DoMeleeAttackIfReady();
    }
        void EnterCombat(Unit* /*who*/) override {}

        void SpellHit(Unit* unit, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_NEURAL_NEEDLE && unit->IsPlayer())
                if (Player* player = unit->ToPlayer())
                    GotStinged(player->GetGUID());
        }

        void GotStinged(ObjectGuid const& casterGUID)
        {
            if(Player* caster = Player::GetPlayer(*me, casterGUID))
            {
                uint32 step = caster->GetAuraCount(SPELL_NEURAL_NEEDLE) + 1;
                switch (step)
                {
                case 1:
                    Talk(SAY_IMPRISIONED_BERYL_1);
                    break;
                case 2:
                    Talk(SAY_IMPRISIONED_BERYL_2, casterGUID);
                    break;
                case 3:
                    Talk(SAY_IMPRISIONED_BERYL_3);
                    break;
                case 4:
                    Talk(SAY_IMPRISIONED_BERYL_4);
                    break;
                case 5:
                    Talk(SAY_IMPRISIONED_BERYL_5);
                    break;
                case 6:
                    Talk(SAY_IMPRISIONED_BERYL_6, casterGUID);
                    break;
                case 7:
                    Talk(SAY_IMPRISIONED_BERYL_7);
                    caster->KilledMonsterCredit(25478, ObjectGuid::Empty);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_imprisoned_beryl_sorcererAI(creature);
    }
};

/*######
## npc_mootoo_the_younger
######*/
enum Script_Texts_Mootoo_the_Younger
{
    SAY_1   = 0,
    SAY_2   = 1,
    SAY_3   = 2,
    SAY_4   = 3,
    SAY_5   = 4
};
enum Mootoo_the_Younger_Entries
{
    NPC_MOOTOO_THE_YOUNGER          =25504,
    QUEST_ESCAPING_THE_MIST         =11664
};

class npc_mootoo_the_younger : public CreatureScript
{
public:
    npc_mootoo_the_younger() : CreatureScript("npc_mootoo_the_younger") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_ESCAPING_THE_MIST)
        {
            switch (player->GetTeam())
            {
            case ALLIANCE:
                creature->setFaction(FACTION_ESCORTEE_A);
                break;
            case HORDE:
                creature->setFaction(FACTION_ESCORTEE_H);
                break;
            }

            creature->SetStandState(UNIT_STAND_STATE_STAND);
            creature->AI()->Talk(SAY_1);

            CAST_AI(npc_escortAI, (creature->AI()))->Start(true, false, player->GetGUID());
        }
        return true;
    }

    struct npc_mootoo_the_youngerAI : public npc_escortAI
    {
        npc_mootoo_the_youngerAI(Creature* creature) : npc_escortAI(creature) {}

        void Reset() override
        {
            SetDespawnAtFar(false);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player=GetPlayerForEscort())
                player->FailQuest(QUEST_ESCAPING_THE_MIST);
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 10:
                    me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                    Talk(SAY_2);
                    break;
                case 12:
                    Talk(SAY_3);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LOOT);
                    break;
                case 16:
                    Talk(SAY_4);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                    break;
                case 20:
                    me->SetPhaseMask(1, true);
                    Talk(SAY_5);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                    player->GroupEventHappens(QUEST_ESCAPING_THE_MIST, me);
                    SetRun(true);
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_mootoo_the_youngerAI(creature);
    }
};

/*######
## npc_bonker_togglevolt
######*/

enum Bonker_Togglevolt_Entries
{
    NPC_BONKER_TOGGLEVOLT   = 25589,
    QUEST_GET_ME_OUTA_HERE  = 11673
};
enum Script_Texts_Bonker_Togglevolt
{
    SAY_bonker_1 = 3,
    SAY_bonker_2 = 0
};

class npc_bonker_togglevolt : public CreatureScript
{
public:
    npc_bonker_togglevolt() : CreatureScript("npc_bonker_togglevolt") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_GET_ME_OUTA_HERE)
        {
            creature->SetStandState(UNIT_STAND_STATE_STAND);
            creature->AI()->Talk(SAY_bonker_2, player->GetGUID());
            CAST_AI(npc_escortAI, (creature->AI()))->Start(true, true, player->GetGUID());
        }
        return true;
    }

    struct npc_bonker_togglevoltAI : public npc_escortAI
    {
        npc_bonker_togglevoltAI(Creature* creature) : npc_escortAI(creature) {}
        uint32 Bonker_agro;

        void Reset() override
        {
            Bonker_agro=0;
            SetDespawnAtFar(false);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_GET_ME_OUTA_HERE);
        }

        void UpdateEscortAI(const uint32 /*diff*/) override
        {
            if (GetAttack() && UpdateVictim())
            {
                if (Bonker_agro == 0)
                {
                    Talk(SAY_bonker_1);
                    Bonker_agro++;
                }

                DoMeleeAttackIfReady();
            }
            else
                Bonker_agro=0;
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            if (waypointId == 29)
                player->GroupEventHappens(QUEST_GET_ME_OUTA_HERE, me);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bonker_togglevoltAI(creature);
    }
};

/*######
## Help Those That Cannot Help Themselves, Quest 11876
######*/

enum eHelpThemselves
{
    QUEST_CANNOT_HELP_THEMSELVES                  =  11876,
    GO_MAMMOTH_TRAP_1                             = 188022,
    GO_MAMMOTH_TRAP_2                             = 188024,
    GO_MAMMOTH_TRAP_3                             = 188025,
    GO_MAMMOTH_TRAP_4                             = 188026,
    GO_MAMMOTH_TRAP_5                             = 188027,
    GO_MAMMOTH_TRAP_6                             = 188028,
    GO_MAMMOTH_TRAP_7                             = 188029,
    GO_MAMMOTH_TRAP_8                             = 188030,
    GO_MAMMOTH_TRAP_9                             = 188031,
    GO_MAMMOTH_TRAP_10                            = 188032,
    GO_MAMMOTH_TRAP_11                            = 188033,
    GO_MAMMOTH_TRAP_12                            = 188034,
    GO_MAMMOTH_TRAP_13                            = 188035,
    GO_MAMMOTH_TRAP_14                            = 188036,
    GO_MAMMOTH_TRAP_15                            = 188037,
    GO_MAMMOTH_TRAP_16                            = 188038,
    GO_MAMMOTH_TRAP_17                            = 188039,
    GO_MAMMOTH_TRAP_18                            = 188040,
    GO_MAMMOTH_TRAP_19                            = 188041,
    GO_MAMMOTH_TRAP_20                            = 188042,
    GO_MAMMOTH_TRAP_21                            = 188043,
    GO_MAMMOTH_TRAP_22                            = 188044,
};

#define MammothTrapsNum 22
const uint32 MammothTraps[MammothTrapsNum] =
{
    GO_MAMMOTH_TRAP_1, GO_MAMMOTH_TRAP_2, GO_MAMMOTH_TRAP_3, GO_MAMMOTH_TRAP_4, GO_MAMMOTH_TRAP_5,
    GO_MAMMOTH_TRAP_6, GO_MAMMOTH_TRAP_7, GO_MAMMOTH_TRAP_8, GO_MAMMOTH_TRAP_9, GO_MAMMOTH_TRAP_10,
    GO_MAMMOTH_TRAP_11, GO_MAMMOTH_TRAP_12, GO_MAMMOTH_TRAP_13, GO_MAMMOTH_TRAP_14, GO_MAMMOTH_TRAP_15,
    GO_MAMMOTH_TRAP_16, GO_MAMMOTH_TRAP_17, GO_MAMMOTH_TRAP_18, GO_MAMMOTH_TRAP_19, GO_MAMMOTH_TRAP_20,
    GO_MAMMOTH_TRAP_21, GO_MAMMOTH_TRAP_22
};

class npc_trapped_mammoth_calf : public CreatureScript
{
public:
    npc_trapped_mammoth_calf() : CreatureScript("npc_trapped_mammoth_calf") {}

    struct npc_trapped_mammoth_calfAI : public ScriptedAI
    {
        npc_trapped_mammoth_calfAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiTimer;
        bool bStarted;

        void Reset() override
        {
            uiTimer = 1500;
            bStarted = false;

            GameObject* pTrap = NULL;
            for (uint8 i = 0; i < MammothTrapsNum; ++i)
            {
                pTrap = me->FindNearestGameObject(MammothTraps[i], 11.0f);
                if (pTrap)
                {
                    pTrap->SetGoState(GO_STATE_ACTIVE);
                    return;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (bStarted)
            {
                if (uiTimer <= diff)
                {
                    Position pos;
                    me->GetRandomNearPosition(pos, 10.0f);
                    me->GetMotionMaster()->MovePoint(0, pos);
                    bStarted = false;
                }
                else
                    uiTimer -= diff;
            }
        }

        void DoAction(int32 const param) override
        {
            if (param == 1)
                bStarted = true;
        }

        void MovementInform(uint32 uiType, uint32 /*uiId*/) override
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            me->DisappearAndDie();

            GameObject* pTrap = NULL;
            for (uint8 i = 0; i < MammothTrapsNum; ++i)
            {
                pTrap = me->FindNearestGameObject(MammothTraps[i], 11.0f);

                if (pTrap)
                {
                    pTrap->SetLootState(GO_JUST_DEACTIVATED);
                    return;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_trapped_mammoth_calfAI(creature);
    }
};

/*######
## Quest 11608: Bury Those Cockroaches!
######*/

#define QUEST_BURY_THOSE_COCKROACHES            11608
#define SPELL_SEAFORIUM_DEPTH_CHARGE_EXPLOSION  45502

class npc_seaforium_depth_charge : public CreatureScript
{
public:
    npc_seaforium_depth_charge() : CreatureScript("npc_seaforium_depth_charge") {}

    struct npc_seaforium_depth_chargeAI : public ScriptedAI
    {
        npc_seaforium_depth_chargeAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 uiExplosionTimer;

        void Reset() override
        {
            uiExplosionTimer = urand(5000, 10000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (uiExplosionTimer < diff)
            {
                DoCast(SPELL_SEAFORIUM_DEPTH_CHARGE_EXPLOSION);
                for (uint8 i = 0; i < 4; ++i)
                {
                    if (Creature* cCredit = me->FindNearestCreature(25402 + i, 10.0f))//25402-25405 credit markers
                    {
                        if (Unit* uOwner = me->GetOwner())
                        {
                            Player* owner = uOwner->ToPlayer();
                            if (owner && owner->GetQuestStatus(QUEST_BURY_THOSE_COCKROACHES) == QUEST_STATUS_INCOMPLETE)
                                owner->KilledMonsterCredit(cCredit->GetEntry(), cCredit->GetGUID());
                        }
                    }
                }
                me->Kill(me);
                return;
            }
            else
                uiExplosionTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_seaforium_depth_chargeAI(creature);
    }
};

/*######
## Help Those That Cannot Help Themselves, Quest 11876
######*/

enum eValiancekeepcannons
{
    GO_VALIANCE_KEEP_CANNON_1                     = 187560,
    GO_VALIANCE_KEEP_CANNON_2                     = 188692
};

class npc_valiance_keep_cannoneer : public CreatureScript
{
public:
    npc_valiance_keep_cannoneer() : CreatureScript("npc_valiance_keep_cannoneer") {}

    struct npc_valiance_keep_cannoneerAI : public ScriptedAI
    {
        npc_valiance_keep_cannoneerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiTimer;

        void Reset() override
        {
            uiTimer = urand(13000, 18000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (uiTimer <= diff)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
                GameObject* pCannon = me->FindNearestGameObject(GO_VALIANCE_KEEP_CANNON_1, 10);
                if (!pCannon)
                    pCannon = me->FindNearestGameObject(GO_VALIANCE_KEEP_CANNON_2, 10);
                if (pCannon)
                    pCannon->Use(me);
                uiTimer = urand(13000, 18000);
            }
            else
                uiTimer -= diff;

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_valiance_keep_cannoneerAI(creature);
    }
};

/*******************************************************
 * npc_warmage_coldarra
 *******************************************************/

enum Spells
{
    SPELL_TRANSITUS_SHIELD_BEAM = 48310
};

enum NPCs
{
    NPC_TRANSITUS_SHIELD_DUMMY   = 27306,
    NPC_WARMAGE_HOLLISTER        = 27906,
    NPC_WARMAGE_CALANDRA         = 27173,
    NPC_WARMAGE_WATKINS          = 27904
};

class npc_warmage_coldarra : public CreatureScript
{
public:
    npc_warmage_coldarra() : CreatureScript("npc_warmage_coldarra") {}

    struct npc_warmage_coldarraAI : public Scripted_NoMovementAI
    {
        npc_warmage_coldarraAI(Creature* creature) : Scripted_NoMovementAI(creature){}

        uint32 m_uiTimer;                 //Timer until recast

        void Reset() override
        {
            m_uiTimer = 0;
        }

        void EnterCombat(Unit* /*who*/) override {}

        void AttackStart(Unit* /*who*/) override {}

        void UpdateAI(uint32 uiDiff) override
        {
            if (m_uiTimer <= uiDiff)
            {
                std::list<Creature*> orbList;
                GetCreatureListWithEntryInGrid(orbList, me, NPC_TRANSITUS_SHIELD_DUMMY, 32.0f);
                if (orbList.empty())
                    return;

                switch (me->GetEntry())
                {
                    case NPC_WARMAGE_HOLLISTER:
                    {
                        if (!orbList.empty())
                        {
                            for (std::list<Creature*>::const_iterator itr = orbList.begin(); itr != orbList.end(); ++itr)
                            {
                                if (Creature* pOrb = *itr)
                                    if (pOrb->GetPositionY() > 6680)
                                        DoCast(pOrb, SPELL_TRANSITUS_SHIELD_BEAM);
                            }
                        }
                        m_uiTimer = urand(90000, 120000);
                        break;
                    }
                    case NPC_WARMAGE_CALANDRA:
                    {
                        if (!orbList.empty())
                        {
                            for (std::list<Creature*>::const_iterator itr = orbList.begin(); itr != orbList.end(); ++itr)
                            {
                                if (Creature* pOrb = *itr)
                                    if ((pOrb->GetPositionY() < 6680) && (pOrb->GetPositionY() > 6630))
                                        DoCast(pOrb, SPELL_TRANSITUS_SHIELD_BEAM);
                            }
                        }
                        m_uiTimer = urand(90000, 120000);
                        break;
                    }
                    case NPC_WARMAGE_WATKINS:
                    {
                        if (!orbList.empty())
                        {
                            for (std::list<Creature*>::const_iterator itr = orbList.begin(); itr != orbList.end(); ++itr)
                            {
                                if (Creature* pOrb = *itr)
                                    if (pOrb->GetPositionY() < 6630)
                                        DoCast(pOrb, SPELL_TRANSITUS_SHIELD_BEAM);
                            }
                        }
                        m_uiTimer = urand(90000, 120000);
                        break;
                    }
                }
            }
            else
                m_uiTimer -= uiDiff;

            ScriptedAI::UpdateAI(uiDiff);

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_warmage_coldarraAI(creature);
    }
};

/*######
## npc_hidden_cultist
######*/

enum eHiddenCultist
{
    SPELL_SHROUD_OF_THE_DEATH_CULTIST           = 46077, //not working
    SPELL_RIGHTEOUS_VISION                      = 46078, //player aura

    QUEST_THE_HUNT_IS_ON                        = 11794,

    GOSSIP_TEXT_SALTY_JOHN_THORPE               = 12529,
    GOSSIP_TEXT_GUARD_MITCHELSS                 = 12530,
    GOSSIP_TEXT_TOM_HEGGER                      = 12528,

    NPC_TOM_HEGGER                              = 25827,
    NPC_SALTY_JOHN_THORPE                       = 25248,
    NPC_GUARD_MITCHELLS                         = 25828,

    SAY_HIDDEN_CULTIST_1                        = 0,
    SAY_HIDDEN_CULTIST_2                        = 1,
    SAY_HIDDEN_CULTIST_3                        = 0,
    SAY_HIDDEN_CULTIST_4                        = 1
};

const char* GOSSIP_ITEM_TOM_HEGGER = "What do you know about the Cult of the Damned?";
const char* GOSSIP_ITEM_GUARD_MITCHELLS = "How long have you worked for the Cult of the Damned?";
const char* GOSSIP_ITEM_SALTY_JOHN_THORPE = "I have a reason to believe you're involved in the cultist activity";

class npc_hidden_cultist : public CreatureScript
{
public:
    npc_hidden_cultist() : CreatureScript("npc_hidden_cultist") {}

    struct npc_hidden_cultistAI : public ScriptedAI
    {
        npc_hidden_cultistAI(Creature* creature) : ScriptedAI(creature)
        {
           uiEmoteState = creature->GetUInt32Value(UNIT_FIELD_EMOTE_STATE);
           uiNpcFlags = creature->GetUInt32Value(UNIT_FIELD_NPC_FLAGS);
        }

        uint32 uiEmoteState;
        uint32 uiNpcFlags;
        uint32 uiEventTimer;
        uint8 uiEventPhase;
        ObjectGuid uiPlayerGUID;

        void Reset() override
        {
            if (uiEmoteState)
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, uiEmoteState);

            if (uiNpcFlags)
                me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, uiNpcFlags);

            uiEventTimer = 0;
            uiEventPhase = 0;

            uiPlayerGUID.Clear();

            DoCast(SPELL_SHROUD_OF_THE_DEATH_CULTIST);

            me->RestoreFaction();
        }

        void DoAction(int32 const /*iParam*/) override
        {
            me->StopMoving();
            me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, 0);

            if (Player* player = me->GetPlayer(*me, uiPlayerGUID))
                me->SetFacingTo(player);

            uiEventTimer = 3000;
            uiEventPhase = 1;
        }

        void SetGUID(ObjectGuid const& uiGuid, int32 /*iId*/) override
        {
            uiPlayerGUID = uiGuid;
        }

        void AttackPlayer()
        {
            me->setFaction(14);

            if (Player* player = me->GetPlayer(*me, uiPlayerGUID))
                me->AI()->AttackStart(player);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (uiEventTimer && uiEventTimer <= uiDiff)
            {
                switch (uiEventPhase)
                {
                    case 1:
                        switch (me->GetEntry())
                        {
                            case NPC_SALTY_JOHN_THORPE:
                                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                                Talk(SAY_HIDDEN_CULTIST_1);
                                uiEventTimer = 5000;
                                uiEventPhase = 2;
                                break;
                            case NPC_GUARD_MITCHELLS:
                                Talk(SAY_HIDDEN_CULTIST_2);
                                uiEventTimer = 5000;
                                uiEventPhase = 2;
                                break;
                            case NPC_TOM_HEGGER:
                                Talk(SAY_HIDDEN_CULTIST_3);
                                uiEventTimer = 5000;
                                uiEventPhase = 2;
                                break;
                        }
                        break;
                    case 2:
                        switch (me->GetEntry())
                        {
                            case NPC_SALTY_JOHN_THORPE:
                                Talk(SAY_HIDDEN_CULTIST_4);

                                if (Player* player = me->GetPlayer(*me, uiPlayerGUID))
                                    me->SetFacingTo(player);

                                uiEventTimer = 3000;
                                uiEventPhase = 3;
                                break;
                            case NPC_GUARD_MITCHELLS:
                            case NPC_TOM_HEGGER:
                                AttackPlayer();
                                uiEventPhase = 0;
                                break;
                        }
                        break;
                    case 3:
                        if (me->GetEntry() == NPC_SALTY_JOHN_THORPE)
                        {
                            AttackPlayer();
                            uiEventPhase = 0;
                        }
                        break;
                }
            }
            else
                uiEventTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hidden_cultistAI(creature);
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        uint32 uiGossipText = 0;
        const char* charGossipItem;

        switch (creature->GetEntry())
        {
            case NPC_TOM_HEGGER:
                uiGossipText = GOSSIP_TEXT_TOM_HEGGER;
                charGossipItem = GOSSIP_ITEM_TOM_HEGGER;
                break;
            case NPC_SALTY_JOHN_THORPE:
                uiGossipText = GOSSIP_TEXT_SALTY_JOHN_THORPE;
                charGossipItem = GOSSIP_ITEM_SALTY_JOHN_THORPE;
                break;
            case NPC_GUARD_MITCHELLS:
                uiGossipText = GOSSIP_TEXT_GUARD_MITCHELSS;
                charGossipItem = GOSSIP_ITEM_GUARD_MITCHELLS;
                break;
            default:
                charGossipItem = "";
                return false;
        }

        if (player->HasAura(SPELL_RIGHTEOUS_VISION) && player->GetQuestStatus(QUEST_THE_HUNT_IS_ON) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, charGossipItem, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        if (creature->isVendor())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        player->SEND_GOSSIP_MENU(uiGossipText, creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (action == GOSSIP_ACTION_INFO_DEF+1)
        {
            player->CLOSE_GOSSIP_MENU();
            creature->AI()->SetGUID(player->GetGUID());
            creature->AI()->DoAction(1);
        }

        if (action == GOSSIP_ACTION_TRADE)
            player->GetSession()->SendListInventory(creature->GetGUID());

        return true;
    }

};

void AddSC_borean_tundra()
{
    new npc_sinkhole_kill_credit();
    new npc_khunok_the_behemoth();
    new npc_keristrasza();
    new npc_corastrasza();
    new npc_iruk();
    new mob_nerubar_victim();
    new npc_scourge_prisoner();
    new npc_jenny();
    new npc_fezzix_geartwist();
    new npc_nesingwary_trapper();
    new npc_lurgglbr();
    new npc_thassarian();
    new npc_image_lich_king();
    new npc_counselor_talbot();
    new npc_leryssa();
    new npc_general_arlos();
    new npc_beryl_sorcerer();
    new npc_imprisoned_beryl_sorcerer();
    new npc_mootoo_the_younger();
    new npc_bonker_togglevolt();
    new npc_trapped_mammoth_calf();
    new npc_seaforium_depth_charge();
    new npc_valiance_keep_cannoneer();
    new npc_warmage_coldarra();
    new npc_hidden_cultist();
}
