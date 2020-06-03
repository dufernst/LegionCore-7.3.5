#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "SpellScript.h"

/*######
## npc_vekjik
######*/

#define GOSSIP_VEKJIK_ITEM1 "Shaman Vekjik, I have spoken with the big-tongues and they desire peace. I have brought this offering on their behalf."
#define GOSSIP_VEKJIK_ITEM2 "No no... I had no intentions of betraying your people. I was only defending myself. it was all a misunderstanding."

enum eVekjik
{
    GOSSIP_TEXTID_VEKJIK1       = 13137,
    GOSSIP_TEXTID_VEKJIK2       = 13138,

    SAY_TEXTID_VEKJIK1          = -1000208,

    SPELL_FREANZYHEARTS_FURY    = 51469,

    QUEST_MAKING_PEACE          = 12573
};

class npc_vekjik : public CreatureScript
{
public:
    npc_vekjik() : CreatureScript("npc_vekjik") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_MAKING_PEACE) == QUEST_STATUS_INCOMPLETE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_VEKJIK_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VEKJIK1, creature->GetGUID());
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
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_VEKJIK_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VEKJIK2, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->CLOSE_GOSSIP_MENU();
                DoScriptText(SAY_TEXTID_VEKJIK1, creature, player);
                player->AreaExploredOrEventHappens(QUEST_MAKING_PEACE);
                creature->CastSpell(player, SPELL_FREANZYHEARTS_FURY, false);
                break;
        }

        return true;
    }
};

/*######
## avatar_of_freya
######*/

#define GOSSIP_ITEM_AOF1 "I want to stop the Scourge as much as you do. How can I help?"
#define GOSSIP_ITEM_AOF2 "You can trust me. I am no friend of the Lich King."
#define GOSSIP_ITEM_AOF3 "I will not fail."

enum eFreya
{
    QUEST_FREYA_PACT         = 12621,

    SPELL_FREYA_CONVERSATION = 52045,

    GOSSIP_TEXTID_AVATAR1    = 13303,
    GOSSIP_TEXTID_AVATAR2    = 13304,
    GOSSIP_TEXTID_AVATAR3    = 13305
};

class npc_avatar_of_freya : public CreatureScript
{
public:
    npc_avatar_of_freya() : CreatureScript("npc_avatar_of_freya") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_FREYA_PACT) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_AOF1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXTID_AVATAR1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_AOF2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXTID_AVATAR2, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_AOF3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXTID_AVATAR3, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->CastSpell(player, SPELL_FREYA_CONVERSATION, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        return true;
    }
};

/*######
## npc_bushwhacker
######*/

class npc_bushwhacker : public CreatureScript
{
public:
    npc_bushwhacker() : CreatureScript("npc_bushwhacker") { }

    struct npc_bushwhackerAI : public ScriptedAI
    {
        npc_bushwhackerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void InitializeAI() override
        {
            if (me->isDead())
                return;

            if (TempSummon* summ = me->ToTempSummon())
                if (Unit* summoner = summ->GetSummoner())
                    me->GetMotionMaster()->MovePoint(0, summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ());

            Reset();
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
        return new npc_bushwhackerAI(creature);
    }
};

/*######
## npc_engineer_helice
######*/

enum eEnums
{
    SPELL_EXPLODE_CRYSTAL       = 62487,
    SPELL_FLAMES                = 64561,

    SAY_WP_7                    = -1800047,
    SAY_WP_6                    = -1800048,
    SAY_WP_5                    = -1800049,
    SAY_WP_4                    = -1800050,
    SAY_WP_3                    = -1800051,
    SAY_WP_2                    = -1800052,
    SAY_WP_1                    = -1800053,

    QUEST_DISASTER              = 12688
};

class npc_engineer_helice : public CreatureScript
{
public:
    npc_engineer_helice() : CreatureScript("npc_engineer_helice") { }

    struct npc_engineer_heliceAI : public npc_escortAI
    {
        npc_engineer_heliceAI(Creature* creature) : npc_escortAI(creature) { }

        uint32 m_uiChatTimer;

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();

            switch (waypointId)
            {
                case 0:
                    DoScriptText(SAY_WP_2, me);
                    break;
                case 1:
                    DoScriptText(SAY_WP_3, me);
                    me->CastSpell(5918.33f, 5372.91f, -98.770f, SPELL_EXPLODE_CRYSTAL, true);
                    me->SummonGameObject(184743, 5918.33f, 5372.91f, -98.770f, 0, 0, 0, 0, 0, TEMPSUMMON_MANUAL_DESPAWN);     //approx 3 to 4 seconds
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    break;
                case 2:
                    DoScriptText(SAY_WP_4, me);
                    break;
                case 7:
                    DoScriptText(SAY_WP_5, me);
                    break;
                case 8:
                    me->CastSpell(5887.37f, 5379.39f, -91.289f, SPELL_EXPLODE_CRYSTAL, true);
                    me->SummonGameObject(184743, 5887.37f, 5379.39f, -91.289f, 0, 0, 0, 0, 0, TEMPSUMMON_MANUAL_DESPAWN);      //approx 3 to 4 seconds
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    break;
                case 9:
                    DoScriptText(SAY_WP_6, me);
                    break;
                case 13:
                    if (player)
                    {
                        player->GroupEventHappens(QUEST_DISASTER, me);
                        DoScriptText(SAY_WP_7, me);
                    }
                    break;
            }
        }

        void Reset() override
        {
            m_uiChatTimer = 4000;
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (Player* player = GetPlayerForEscort())
                    player->FailQuest(QUEST_DISASTER);
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (m_uiChatTimer <= uiDiff)
                {
                    m_uiChatTimer = 12000;
                }
                else
                    m_uiChatTimer -= uiDiff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_engineer_heliceAI(creature);
    }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest) override
    {
        if (quest->GetQuestId() == QUEST_DISASTER)
        {
            if (npc_engineer_heliceAI* pEscortAI = CAST_AI(npc_engineer_helice::npc_engineer_heliceAI, creature->AI()))
            {
                creature->GetMotionMaster()->MoveJumpTo(0, 0.4f, 0.4f);
                creature->setFaction(113);

                pEscortAI->Start(false, false, player->GetGUID());
                DoScriptText(SAY_WP_1, creature);
            }
        }
        return true;
    }
};

/*#####
## npc_jungle_punch_target
#####*/

#define SAY_OFFER     "Care to try Grimbooze Thunderbrew's new jungle punch?"
#define SAY_HEMET_1   "Aye, I'll try it."
#define SAY_HEMET_2   "That's exactly what I needed!"
#define SAY_HEMET_3   "It's got my vote! That'll put hair on your chest like nothing else will."
#define SAY_HADRIUS_1 "I'm always up for something of Grimbooze's."
#define SAY_HADRIUS_2 "Well, so far, it tastes like something my wife would drink..."
#define SAY_HADRIUS_3 "Now, there's the kick I've come to expect from Grimbooze's drinks! I like it!"
#define SAY_TAMARA_1  "Sure!"
#define SAY_TAMARA_2  "Oh my..."
#define SAY_TAMARA_3  "Tastes like I'm drinking... engine degreaser!"

enum utils
{
    NPC_HEMET   = 27986,
    NPC_HADRIUS = 28047,
    NPC_TAMARA  = 28568,
    SPELL_OFFER = 51962,
    QUEST_ENTRY = 12645,
};

class npc_jungle_punch_target : public CreatureScript
{
public:
    npc_jungle_punch_target() : CreatureScript("npc_jungle_punch_target") { }

    struct npc_jungle_punch_targetAI : public ScriptedAI
    {
        npc_jungle_punch_targetAI(Creature* creature) : ScriptedAI(creature) {}

        uint16 sayTimer;
        uint8 sayStep;

        void Reset() override
        {
            sayTimer = 3500;
            sayStep = 0;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!sayStep)
                return;

            if (sayTimer < uiDiff)
            {
                switch (sayStep)
                {
                    case 0:
                    {
                        switch (me->GetEntry())
                        {
                        case NPC_HEMET:   me->MonsterSay(SAY_HEMET_1, LANG_UNIVERSAL, ObjectGuid::Empty);   break;
                        case NPC_HADRIUS: me->MonsterSay(SAY_HADRIUS_1, LANG_UNIVERSAL, ObjectGuid::Empty); break;
                        case NPC_TAMARA:  me->MonsterSay(SAY_TAMARA_1, LANG_UNIVERSAL, ObjectGuid::Empty);  break;
                        }
                        sayTimer = 3000;
                        sayStep++;
                        break;
                    }
                    case 1:
                    {
                        switch (me->GetEntry())
                        {
                        case NPC_HEMET:   me->MonsterSay(SAY_HEMET_2, LANG_UNIVERSAL, ObjectGuid::Empty);   break;
                        case NPC_HADRIUS: me->MonsterSay(SAY_HADRIUS_2, LANG_UNIVERSAL, ObjectGuid::Empty); break;
                        case NPC_TAMARA:  me->MonsterSay(SAY_TAMARA_2, LANG_UNIVERSAL, ObjectGuid::Empty);  break;
                        }
                        sayTimer = 3000;
                        sayStep++;
                        break;
                    }
                    case 2:
                    {
                        switch (me->GetEntry())
                        {
                        case NPC_HEMET:   me->MonsterSay(SAY_HEMET_3, LANG_UNIVERSAL, ObjectGuid::Empty);   break;
                        case NPC_HADRIUS: me->MonsterSay(SAY_HADRIUS_3, LANG_UNIVERSAL, ObjectGuid::Empty); break;
                        case NPC_TAMARA:  me->MonsterSay(SAY_TAMARA_3, LANG_UNIVERSAL, ObjectGuid::Empty);  break;
                        }
                        sayTimer = 3000;
                        sayStep = 0;
                        break;
                    }
                }
            }
            else
                sayTimer -= uiDiff;
        }

        void SpellHit(Unit* caster, const SpellInfo* proto) override
        {
            if (!proto || proto->Id != SPELL_OFFER)
                return;

            if (!caster->ToPlayer())
                return;

            QuestStatusData* status = caster->ToPlayer()->getQuestStatus(QUEST_ENTRY);
            if (!status || status->Status != QUEST_STATUS_INCOMPLETE)
                return;

            for (uint8 i=0; i<3; i++)
            {
                switch (i)
                {
                   case 0:
                       if (NPC_HEMET != me->GetEntry())
                           continue;
                       else
                           break;
                   case 1:
                       if (NPC_HADRIUS != me->GetEntry())
                           continue;
                       else
                           break;
                   case 2:
                       if (NPC_TAMARA != me->GetEntry())
                           continue;
                       else
                           break;
                }

                caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), ObjectGuid::Empty);
                caster->ToPlayer()->Say(SAY_OFFER, LANG_UNIVERSAL);
                sayStep = 0;
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jungle_punch_targetAI(creature);
    }
};

/*######
## npc_adventurous_dwarf
######*/

#define GOSSIP_OPTION_ORANGE    "Can you spare an orange?"
#define GOSSIP_OPTION_BANANAS   "Have a spare bunch of bananas?"
#define GOSSIP_OPTION_PAPAYA    "I could really use a papaya."

enum eAdventurousDwarf
{
    QUEST_12634         = 12634,

    ITEM_BANANAS        = 38653,
    ITEM_PAPAYA         = 38655,
    ITEM_ORANGE         = 38656,

    SPELL_ADD_ORANGE    = 52073,
    SPELL_ADD_BANANAS   = 52074,
    SPELL_ADD_PAPAYA    = 52076,

    GOSSIP_MENU_DWARF   = 13307,

    SAY_DWARF_OUCH      = -1571042,
    SAY_DWARF_HELP      = -1571043
};

class npc_adventurous_dwarf : public CreatureScript
{
public:
    npc_adventurous_dwarf() : CreatureScript("npc_adventurous_dwarf") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        DoScriptText(SAY_DWARF_OUCH, creature);
        return NULL;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(QUEST_12634) != QUEST_STATUS_INCOMPLETE)
            return false;

        if (player->GetItemCount(ITEM_ORANGE) < 1)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_OPTION_ORANGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if (player->GetItemCount(ITEM_BANANAS) < 2)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_OPTION_BANANAS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        if (player->GetItemCount(ITEM_PAPAYA) < 1)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_OPTION_PAPAYA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        player->PlayerTalkClass->SendGossipMenu(GOSSIP_MENU_DWARF, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 spellId = 0;
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1: spellId = SPELL_ADD_ORANGE;     break;
            case GOSSIP_ACTION_INFO_DEF + 2: spellId = SPELL_ADD_BANANAS;    break;
            case GOSSIP_ACTION_INFO_DEF + 3: spellId = SPELL_ADD_PAPAYA;     break;
        }
        if (spellId)
            player->CastSpell(player, spellId, true);
        DoScriptText(SAY_DWARF_HELP, creature);
        creature->DespawnOrUnsummon();
        return true;
    }
};

enum Q12581_Data
{
    SPELL_ARTRUIS_FB        = 15530,
    SPELL_ARTRUIS_LANCE     = 54261,
    SPELL_ARTRUIS_NOVA      = 11831,
    SPELL_ARTRUIS_VEINS     = 54792,
    SPELL_ARTRUIS_IMMUN     = 52185,
    SPELL_ARTRUIS_URN       = 52518,
    SPELL_VISUAL_TOMB       = 52182,

    SAY_AGGRO               = 0,
    SAY_75                  = 1,
    SAY_50                  = 2,
    SAY_30_1                = 3,
    SAY_30_2                = 4,
    SAY_DEATH               = 5,

    NPC_JALOOT              = 28667,
    NPC_ZEPHIK              = 28668,
    NPC_ARTRUIS             = 28659,
};

// 28659
struct npc_artruis_Q12581 : public ScriptedAI
{
    npc_artruis_Q12581(Creature* creature) : ScriptedAI(creature)
    {
        me->SetControlled(1, UNIT_STATE_ROOT);
    }

    EventMap events;
    bool hp75 = false;
    bool hp50 = false;
    bool hp30 = false;
    bool cankill = false;

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        ResAdds();
        events.ScheduleEvent(EVENT_1, urand(3500, 4500));
        events.ScheduleEvent(EVENT_2, urand(7000, 10000));
        events.ScheduleEvent(EVENT_3, 15000);
        events.ScheduleEvent(EVENT_4, urand(2000, 3500));
    }

    void ResAdds()
    {
        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, NPC_JALOOT, 40.0f);
        GetCreatureListWithEntryInGrid(creList, me, NPC_ZEPHIK, 40.0f);
        for (auto const& adds : creList)
        {
            if (!creList.empty())
            {
                if (!adds->isAlive())
                {
                    adds->Respawn(true);
                }
            }
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            events.Reset();
            me->RemoveAurasDueToSpell(SPELL_ARTRUIS_IMMUN);
            cankill = true;
        }
    }

    void DamageTaken(Unit* damager, uint32 &damage, DamageEffectType dmgType)
    {
        if (me->HealthBelowPct(75) && !hp75)
        {
            hp75 = true;
            Talk(SAY_75);
        }
        else if (me->HealthBelowPct(50) && !hp50)
        {
            hp50 = true;
            Talk(SAY_50);
        }
        if (damage >= me->GetHealth() || me->HealthBelowPct(30))
        {
            if (!cankill)
                damage = 0;

            if (!hp30)
            {
                hp30 = true;
                Talk(SAY_30_1);
                Talk(SAY_30_2);
                me->InterruptNonMeleeSpells(false);
                DoCast(SPELL_ARTRUIS_IMMUN);
                if (auto jaloot = me->FindNearestCreature(NPC_JALOOT, 40.f))
                {
                    if (auto zephik = me->FindNearestCreature(NPC_ZEPHIK, 40.f))
                    {
                        if (jaloot->IsAIEnabled && zephik->IsAIEnabled)
                        {
                            jaloot->AI()->DoAction(ACTION_1);
                            zephik->AI()->DoAction(ACTION_1);
                        }
                    }
                }
            }
        }
    }

    void Reset() override
    {
        events.Reset();
        hp75 = false;
        hp50 = false;
        hp30 = false;
        cankill = false;
        me->RemoveAurasDueToSpell(SPELL_ARTRUIS_IMMUN);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoCast(me, SPELL_ARTRUIS_URN, true);
        Talk(SAY_DEATH);

        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, NPC_JALOOT, 40.0f);
        GetCreatureListWithEntryInGrid(creList, me, NPC_ZEPHIK, 40.0f);
        for (auto const& adds : creList)
        {
            if (!creList.empty())
            {
                if (adds->isAlive())
                {
                    adds->CombatStop();
                    adds->NearTeleportTo(adds->GetHomePosition());
                    adds->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    adds->AddDelayedEvent(25000, [adds]() -> void
                    {
                        adds->DespawnOrUnsummon(500);
                    });
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasAura(SPELL_ARTRUIS_IMMUN))
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
            {
                DoCast(SPELL_ARTRUIS_FB);
                events.ScheduleEvent(EVENT_1, urand(3500, 4500));
                break;
            }
            case EVENT_2:
            {
                DoCast(SPELL_ARTRUIS_LANCE);
                events.ScheduleEvent(EVENT_2, urand(7000, 10000));
                break;
            }
            case EVENT_3:
            {
                if (me->SelectNearestPlayer(10.f))
                    DoCast(SPELL_ARTRUIS_NOVA);
                events.ScheduleEvent(EVENT_3, 15000);
                break;
            }
            case EVENT_4:
            {
                DoCast(SPELL_ARTRUIS_VEINS);
                events.ScheduleEvent(EVENT_4, urand(25000, 35000));
                break;
            }
            }
        }
        DoMeleeAttackIfReady();
    }
};

enum Q12581_jaloot
{
    SPELL_L_WHIRL       = 52943,
    SPELL_L_STRIKE      = 52944,
    SPELL_FRENZY        = 52964,
    SPELL_SYPHON        = 52969,

    SAY_ZEPH_DIE        = 0,
};

// 28659
struct npc_jaloot_Q12581 : public ScriptedAI
{
    npc_jaloot_Q12581(Creature* creature) : ScriptedAI(creature) 
    {
        Res();
    }

    EventMap events;

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_1, urand(1000, 1500));
        events.ScheduleEvent(EVENT_2, urand(2500, 7500));
        events.ScheduleEvent(EVENT_3, urand(5000, 6500));
        events.ScheduleEvent(EVENT_4, 3000);
    }

    void OnQuestReward(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == 12581)
        {
            player->RemoveRewardedQuest(12581);
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->DespawnOrUnsummon(4000);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
        {
            me->setFaction(14);
            me->RemoveAurasDueToSpell(SPELL_VISUAL_TOMB);
            if (auto plr = me->SelectNearestPlayer(40.f))
                if (me->IsAIEnabled)
                    me->AI()->AttackStart(plr);
            break;
        }
        case ACTION_2:
        {
            if (me->isAlive())
            {
                Talk(SAY_ZEPH_DIE);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->setFaction(250);
                me->RemoveAurasDueToSpell(SPELL_ARTRUIS_IMMUN);
                if (auto artr = me->FindNearestCreature(NPC_ARTRUIS, 40.f, true))
                {
                    if (me->IsAIEnabled)
                        me->AI()->AttackStart(artr);
                }
            }
            break;
        }
        }
    }

    void Res()
    {
        DoCast(me, SPELL_VISUAL_TOMB, true);
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->setFaction(250);
        me->NearTeleportTo(me->GetHomePosition());
    }

    void Reset() override
    {
        events.Reset();
        Res();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (auto zephik = me->FindNearestCreature(NPC_ZEPHIK, 45.f))
        {
            if (auto artruis = me->FindNearestCreature(NPC_ARTRUIS, 45.f))
            {
                if (zephik->IsAIEnabled && artruis->IsAIEnabled)
                {
                    zephik->AI()->DoAction(ACTION_2);
                    artruis->AI()->DoAction(ACTION_1);
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
            {
                DoCast(SPELL_L_WHIRL);
                events.ScheduleEvent(EVENT_1, urand(15000, 20000));
                break;
            }
            case EVENT_2:
            {
                DoCast(SPELL_L_STRIKE);
                events.ScheduleEvent(EVENT_2, urand(15000, 18000));
                break;
            }
            case EVENT_3:
            {
                DoCast(SPELL_FRENZY);
                events.ScheduleEvent(EVENT_3, urand(20000, 25000));
                break;
            }
            case EVENT_4:
            {
                if (me->HealthBelowPct(30))
                    DoCast(SPELL_SYPHON);
                events.ScheduleEvent(EVENT_4, 15000);
                break;
            }
            }
        }
        DoMeleeAttackIfReady();
    }
};


enum Q12581_zephik
{
    SPELL_BARB_NET          = 52761,
    SPELL_ENVENOMED         = 52889,
    SPELL_OPEN_WOUND        = 52873,
    SPELL_PERC_ARROW        = 52758,
    SPELL_SPIKE_TRAP        = 52886,
    SPELL_ZEP_BANDAGE       = 52895,

    SAY_JAL_DIE             = 0,
};

// 28659
struct npc_zephik_Q12581 : public ScriptedAI
{
    npc_zephik_Q12581(Creature* creature) : ScriptedAI(creature)
    {
        Res();
    }

    EventMap events;

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_1, urand(2000, 4500));
        events.ScheduleEvent(EVENT_2, urand(7500, 9500));
        events.ScheduleEvent(EVENT_3, urand(14000, 16000));
        events.ScheduleEvent(EVENT_4, urand(19000, 22000));
        events.ScheduleEvent(EVENT_5, urand(25000, 29000));
        events.ScheduleEvent(EVENT_6, 3000);
    }

    void OnQuestReward(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == 12581)
        {
            player->RemoveRewardedQuest(12581);
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->DespawnOrUnsummon(5000);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
        {
            me->setFaction(14);
            me->RemoveAurasDueToSpell(SPELL_VISUAL_TOMB);
            if (auto plr = me->SelectNearestPlayer(40.f))
                if (me->IsAIEnabled)
                    me->AI()->AttackStart(plr);
            break;
        }
        case ACTION_2:
        {
            if (me->isAlive())
            {
                Talk(SAY_JAL_DIE);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->setFaction(250);
                me->RemoveAurasDueToSpell(SPELL_ARTRUIS_IMMUN);
                if (auto artr = me->FindNearestCreature(NPC_ARTRUIS, 40.f, true))
                {
                    if (me->IsAIEnabled)
                        me->AI()->AttackStart(artr);
                }
            }
            break;
        }
        }
    }

    void Res()
    {
        DoCast(me, SPELL_VISUAL_TOMB, true);
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->setFaction(250);
        me->NearTeleportTo(me->GetHomePosition());
    }

    void Reset() override
    {
        events.Reset();
        Res();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (auto jaloot = me->FindNearestCreature(NPC_JALOOT, 45.f))
        {
            if (auto artruis = me->FindNearestCreature(NPC_ARTRUIS, 45.f))
            {
                if (jaloot->IsAIEnabled && artruis->IsAIEnabled)
                {
                    jaloot->AI()->DoAction(ACTION_2);
                    artruis->AI()->DoAction(ACTION_1);
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
            {
                DoCast(SPELL_BARB_NET);
                events.ScheduleEvent(EVENT_1, urand(5000, 6000));
                break;
            }
            case EVENT_2:
            {
                DoCast(SPELL_ENVENOMED);
                events.ScheduleEvent(EVENT_2, urand(8000, 9000));
                break;
            }
            case EVENT_3:
            {
                DoCast(SPELL_OPEN_WOUND);
                events.ScheduleEvent(EVENT_3, urand(12000, 13000));
                break;
            }
            case EVENT_4:
            {
                DoCast(SPELL_PERC_ARROW);
                events.ScheduleEvent(EVENT_4, 15000);
                break;
            }
            case EVENT_5:
            {
                DoCast(SPELL_SPIKE_TRAP);
                events.ScheduleEvent(EVENT_5, 18000);
                break;
            }
            case EVENT_6:
            {
                if (me->HealthBelowPct(30))
                    DoCast(SPELL_ZEP_BANDAGE);
                events.ScheduleEvent(EVENT_6, 16000);
                break;
            }
            }
        }
        DoMeleeAttackIfReady();
    }
};


/*######
## Quest The Lifewarden's Wrath
######*/

enum MiscLifewarden
{
    NPC_PRESENCE = 28563, // Freya's Presence
    NPC_SABOTEUR = 28538, // Cultist Saboteur
    NPC_SERVANT = 28320, // Servant of Freya

    WHISPER_ACTIVATE = 0,

    SPELL_FREYA_DUMMY = 51318,
    SPELL_LIFEFORCE = 51395,
    SPELL_FREYA_DUMMY_TRIGGER = 51335,
    SPELL_LASHER_EMERGE = 48195,
    SPELL_WILD_GROWTH = 52948,
};

class spell_q12620_the_lifewarden_wrath : public SpellScriptLoader
{
public:
    spell_q12620_the_lifewarden_wrath() : SpellScriptLoader("spell_q12620_the_lifewarden_wrath") { }

    class spell_q12620_the_lifewarden_wrath_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_q12620_the_lifewarden_wrath_SpellScript);

        void HandleSendEvent(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (Unit* caster = GetCaster())
            {
                if (Creature* presence = caster->FindNearestCreature(NPC_PRESENCE, 50.0f))
                {
                    presence->AI()->Talk(WHISPER_ACTIVATE, caster->GetGUID());
                    presence->CastSpell(presence, SPELL_FREYA_DUMMY, true); // will target plants
                    // Freya Dummy could be scripted with the following code

                    // Revive plants
                    std::list<Creature*> servants;
                    GetCaster()->GetCreatureListWithEntryInGrid(servants, NPC_SERVANT, 200.0f);
                    for (std::list<Creature*>::iterator itr = servants.begin(); itr != servants.end(); ++itr)
                    {
                        // Couldn't find a spell that does this
                        if ((*itr)->isDead())
                            (*itr)->Respawn(true);

                        (*itr)->CastSpell(*itr, SPELL_FREYA_DUMMY_TRIGGER, true);
                        (*itr)->CastSpell(*itr, SPELL_LASHER_EMERGE, false);
                        (*itr)->CastSpell(*itr, SPELL_WILD_GROWTH, false);

                        if (Unit* target = (*itr)->SelectNearestTarget(150.0f))
                            (*itr)->AI()->AttackStart(target);
                    }

                    // Kill nearby enemies
                    std::list<Creature*> saboteurs;
                    caster->GetCreatureListWithEntryInGrid(saboteurs, NPC_SABOTEUR, 200.0f);
                    for (std::list<Creature*>::iterator itr = saboteurs.begin(); itr != saboteurs.end(); ++itr)
                        if ((*itr)->isAlive())
                            // Lifeforce has a cast duration, it should be cast at all saboteurs one by one
                            presence->CastSpell((*itr), SPELL_LIFEFORCE, false);
                }
            }
        }

        void Register() override
        {
            OnEffectHit += SpellEffectFn(spell_q12620_the_lifewarden_wrath_SpellScript::HandleSendEvent, EFFECT_0, SPELL_EFFECT_SEND_EVENT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_q12620_the_lifewarden_wrath_SpellScript();
    }
};

/*######
## Quest Kick, What Kick? (12589)
######*/

enum KickWhatKick
{
    NPC_LUCKY_WILHELM = 28054,
    NPC_APPLE = 28053,
    NPC_DROSTAN = 28328,
    NPC_CRUNCHY = 28346,
    NPC_THICKBIRD = 28093,

    SPELL_HIT_APPLE = 51331,
    SPELL_MISS_APPLE = 51332,
    SPELL_MISS_BIRD_APPLE = 51366,
    SPELL_APPLE_FALL = 51371,
    SPELL_BIRD_FALL = 51369,

    EVENT_MISS = 0,
    EVENT_HIT = 1,
    EVENT_MISS_BIRD = 2,

    SAY_WILHELM_MISS = 0,
    SAY_WILHELM_HIT = 1,
    SAY_DROSTAN_REPLY_MISS = 0,
};

class spell_q12589_shoot_rjr : public SpellScriptLoader
{
public:
    spell_q12589_shoot_rjr() : SpellScriptLoader("spell_q12589_shoot_rjr") { }

    class spell_q12589_shoot_rjr_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_q12589_shoot_rjr_SpellScript);

        SpellCastResult CheckCast()
        {
            if (Unit* target = GetExplTargetUnit())
                if (target->GetEntry() == NPC_LUCKY_WILHELM)
                    return SPELL_CAST_OK;

            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_TARGET_WILHELM);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            uint32 roll = urand(1, 100);

            uint8 ev;
            if (roll <= 50)
                ev = EVENT_MISS;
            else if (roll <= 83)
                ev = EVENT_HIT;
            else
                ev = EVENT_MISS_BIRD;

            Unit* shooter = GetCaster();
            Creature* wilhelm = GetHitUnit()->ToCreature();
            Creature* apple = shooter->FindNearestCreature(NPC_APPLE, 30);
            Creature* drostan = shooter->FindNearestCreature(NPC_DROSTAN, 30);

            if (!wilhelm || !apple || !drostan)
                return;

            switch (ev)
            {
                case EVENT_MISS_BIRD:
                {
                    Creature* crunchy = shooter->FindNearestCreature(NPC_CRUNCHY, 30);
                    Creature* bird = shooter->FindNearestCreature(NPC_THICKBIRD, 30);

                    if (!bird || !crunchy)
                        ; // fall to EVENT_MISS
                    else
                    {
                        shooter->CastSpell(bird, SPELL_MISS_BIRD_APPLE);
                        bird->CastSpell(bird, SPELL_BIRD_FALL);
                        wilhelm->AI()->Talk(SAY_WILHELM_MISS);
                        drostan->AI()->Talk(SAY_DROSTAN_REPLY_MISS);

                        bird->Kill(bird);
                        crunchy->GetMotionMaster()->MovePoint(0, bird->GetPositionX(), bird->GetPositionY(),
                            bird->GetMap()->GetWaterOrGroundLevel(bird->GetPhases(), bird->GetPositionX(), bird->GetPositionY(), bird->GetPositionZ()));
                        // TODO: Make crunchy perform emote eat when he reaches the bird

                        break;
                    }
                }
                case EVENT_MISS:
                {
                    shooter->CastSpell(wilhelm, SPELL_MISS_APPLE);
                    wilhelm->AI()->Talk(SAY_WILHELM_MISS);
                    drostan->AI()->Talk(SAY_DROSTAN_REPLY_MISS);
                    break;
                }
                case EVENT_HIT:
                {
                    shooter->CastSpell(apple, SPELL_HIT_APPLE);
                    apple->CastSpell(apple, SPELL_APPLE_FALL);
                    wilhelm->AI()->Talk(SAY_WILHELM_HIT);
                    if (Player* player = shooter->ToPlayer())
                        player->KilledMonsterCredit(NPC_APPLE, ObjectGuid::Empty);
                    apple->DespawnOrUnsummon();

                    break;
                }
            }
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_q12589_shoot_rjr_SpellScript::CheckCast);
            OnEffectHitTarget += SpellEffectFn(spell_q12589_shoot_rjr_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_q12589_shoot_rjr_SpellScript();
    }
};

void AddSC_sholazar_basin()
{
    new npc_vekjik();
    new npc_avatar_of_freya();
    new npc_bushwhacker();
    new npc_engineer_helice();
    new npc_adventurous_dwarf();
    new npc_jungle_punch_target();
    new spell_q12620_the_lifewarden_wrath();
    new spell_q12589_shoot_rjr();
    RegisterCreatureAI(npc_artruis_Q12581);
    //RegisterCreatureAI(npc_jaloot_Q12581);
    //RegisterCreatureAI(npc_zephik_Q12581);
}
