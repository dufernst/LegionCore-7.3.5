#include "ScriptedEscortAI.h"
#include "CreatureTextMgr.h"
#include "MapManager.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"

//245728
const uint32 q38690[8] = {97614, 97622, 97618, 97616, 97615, 97620, 97617, 97619};

class go_q38690 : public GameObjectScript
{
public:
    go_q38690() : GameObjectScript("go_q38690") { }

    struct go_q38690AI : public GameObjectAI
    {
        go_q38690AI(GameObject* go) : GameObjectAI(go)
        {

        }

        enum data
        {
            QUEST = 38690,
        };

        bool GossipHello(Player* player) override
        {
            for (int32 i = 0; i < sizeof(q38690); ++i)
            {
                if (!player->GetReqKillOrCastCurrentCount(QUEST, q38690[i]))
                {
                    player->KilledMonsterCredit(q38690[i]);
                    return true;
                }
            }

            return false;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_q38690AI(go);
    }
};

//! 92782 92776
class npc_q38689 : public CreatureScript
{
public:
    npc_q38689() : CreatureScript("npc_q38689") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q38689AI(creature);
    }

    struct npc_q38689AI : public ScriptedAI
    {

        npc_q38689AI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetCorpseDelay(30);
            creature->SetRespawnDelay(15);
        }
        
        EventMap events;

        enum data
        {
            QUEST = 38689,
            SPELL_CREDIT = 133511,
            SPELL_CREDIT2 = 133509,
        };

        void Reset() override
        {
            events.Reset();
            if (me->getVictim() || me->IsInEvadeMode())
                return;

            std::list<Unit*> list;
            me->GetAttackableUnitListInRange(list, 70.0f);
            for (auto enemy : list)
            {
                if (enemy->ToPlayer())
                    continue;
                me->AI()->AttackStart(enemy);
                break;
            }
        }
        
        void EnterCombat(Unit* who) override
        {
            if (me->GetEntry() == 92782)
                events.RescheduleEvent(EVENT_1, 15000); // 200992
            if (me->GetEntry() == 92776)
            {
                if (urand(0, 1) == 1)
                    Talk(urand(0, 10));
                events.RescheduleEvent(EVENT_2, 14000); // 200930
                events.RescheduleEvent(EVENT_3, 20000); // 200963
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            std::list<Player*> lList;
            me->GetPlayerListInGrid(lList, 5.0f);
            for (auto player : lList)
            {
                if (player->GetQuestStatus(QUEST) == QUEST_STATUS_INCOMPLETE)
                {
                    uint32 ra = urand(2, 10);
                    for (uint8 i = 0; i < ra; i++)
                    {
                        player->CastSpell(player, SPELL_CREDIT, true);
                        player->CastSpell(player, SPELL_CREDIT2, true);
                    }
                }
            }
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(200992);
                        events.RescheduleEvent(EVENT_1, 15000); // 200992
                        break;
                    case EVENT_2:
                        DoCast(200930);
                        events.RescheduleEvent(EVENT_2, 14000); // 200930
                        break;
                    case EVENT_3:
                        DoCast(200963);
                        events.RescheduleEvent(EVENT_3, 20000); // 200963
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

    };
};

//! 187864
class spell_q38723 : public SpellScriptLoader
{
public:
    spell_q38723() : SpellScriptLoader("spell_q38723") { }

    class spell_q38723_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_q38723_AuraScript);


        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();

            if (!target || !target->ToPlayer())
                return;
            
            target->ToPlayer()->TeleportTo(1468, 4084.27f, -298.11f, -282.07f, 3.118031f);
            target->ToPlayer()->KilledMonsterCredit(99303);
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_q38723_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ACTIVATE_SCENE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_q38723_AuraScript();
    }
};

//! 92990, 97632
class npc_q38723 : public CreatureScript
{
public:
    npc_q38723() : CreatureScript("npc_q38723") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q38723AI(creature);
    }

    struct npc_q38723AI : public ScriptedAI
    {

        npc_q38723AI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetCorpseDelay(30);
            creature->SetRespawnDelay(15);
        }

        EventMap events;
        enum data
        {
            QUEST = 38723,
            SPELL_CREDIT = 210461,
            SPELL_CREDIT2 = 195327,
        };

        void Reset() override
        {
            events.Reset();
            
            if (me->getVictim() || me->IsInEvadeMode())
                return;

            std::list<Unit*> list;
            me->GetAttackableUnitListInRange(list, 70.0f);
            for (auto enemy : list)
            {
                if (enemy->ToPlayer())
                    continue;
                me->AI()->AttackStart(enemy);
                break;
            }
        }
        
        void EnterCombat(Unit* who) override
        {
            if (me->GetEntry() == 92990)
            {
                events.RescheduleEvent(EVENT_1, 20000); // 199602
                events.RescheduleEvent(EVENT_4, 5000);
            }
            if (me->GetEntry() == 97632)
                events.RescheduleEvent(EVENT_2, 16000); // 199645
            events.RescheduleEvent(EVENT_3, 42000); // 199556
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
        {
            if (Creature* oAdd = GetOtherAdd())
            {
                if (damage >= me->GetHealth())
                    oAdd->Kill(oAdd, true);
                else
                    oAdd->SetHealth(oAdd->GetHealth() - damage);
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            std::list<Player*> lList;
            me->GetPlayerListInGrid(lList, 200.0f);
            for (auto player : lList)
            {
                if (player->GetQuestStatus(QUEST) == QUEST_STATUS_INCOMPLETE)
                {
                    me->CastSpell(player, SPELL_CREDIT, true);
                    player->KilledMonsterCredit(106241);

                    
                    player->CastSpell(player, SPELL_CREDIT2, true);
                }
            }
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(199602);
                        events.RescheduleEvent(EVENT_1, 20000); // 199602
                        break;
                    case EVENT_2:
                        DoCast(199645);
                        events.RescheduleEvent(EVENT_2, 16000); // 199645
                        break;
                    case EVENT_3:
                        DoCast(199556);
                        events.RescheduleEvent(EVENT_3, 42000); // 199556
                        break;
                    case EVENT_4:
                    {
                        std::list<Player*> playerList;
                        me->GetPlayerListInGrid(playerList, 30.0f);
                        for (auto plr : playerList)
                        {
                            Conversation* conversation = new Conversation;
                            if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 528, plr, NULL, *plr))
                                delete conversation;
                        }
                        events.RescheduleEvent(EVENT_5, 5000);
                    }
                    break;
                    case EVENT_5:
                    {
                        std::list<Player*> playerList;
                        me->GetPlayerListInGrid(playerList, 30.0f);
                        for (auto plr : playerList)
                        {
                            Conversation* conversation = new Conversation;
                            if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 529, plr, NULL, *plr))
                                delete conversation;
                        }
                        events.RescheduleEvent(EVENT_6, 5000);
                    }
                    break;
                    case EVENT_6:
                    {
                        std::list<Player*> playerList;
                        me->GetPlayerListInGrid(playerList, 30.0f);
                        for (auto plr : playerList)
                        {
                            Conversation* conversation = new Conversation;
                            if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 530, plr, NULL, *plr))
                                delete conversation;
                        }
                        events.RescheduleEvent(EVENT_6, 5000);
                    }
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
        
        Creature* GetOtherAdd()
        {

            if (Creature* oAdd = me->FindNearestCreature((me->GetEntry() == 92990 ? 92990 : 97632), 100.0f, true))
            {
                if (oAdd->isAlive())
                    return oAdd;
            }
            
            return NULL;
        }

    };
};

/*
[1] byte70: False
[1] [0] Waypoint: X: 4436.342 Y: -285.1404 Z: -245.9517
[1] [1] Waypoint: X: 4436.776 Y: -284.2395 Z: -245.9517
[1] [2] Waypoint: X: 4446.447 Y: -295.8958 Z: -235.3393
[1] [3] Waypoint: X: 4456.686 Y: -311.0035 Z: -207.6675
[1] [4] Waypoint: X: 4460.651 Y: -321.5399 Z: -187.7354
[1] [5] Waypoint: X: 4454.374 Y: -328.5417 Z: -165.5679
[1] [6] Waypoint: X: 4447.924 Y: -327.4219 Z: -153.9652
[1] [7] Waypoint: X: 4448.483 Y: -319.5451 Z: -144.8872
[1] [8] Waypoint: X: 4454.757 Y: -318.3906 Z: -127.1212
[1] [9] Waypoint: X: 4455.591 Y: -325.2778 Z: -107.7209
[1] [10] Waypoint: X: 4449.187 Y: -328.9375 Z: -85.20784
[1] [11] Waypoint: X: 4444.792 Y: -325.2083 Z: -66.88081
[1] [12] Waypoint: X: 4446.588 Y: -317.9913 Z: -27.77394
[1] [13] Waypoint: X: 4451.853 Y: -316.2778 Z: -6.795706
[1] [14] Waypoint: X: 4454.854 Y: -320.9028 Z: 38.56396
[1] [15] Waypoint: X: 4454.283 Y: -324.1094 Z: 68.72004
[1] [16] Waypoint: X: 4451.352 Y: -326.1875 Z: 91.97843
[1] [17] Waypoint: X: 4451.143 Y: -327.8229 Z: 136.2271
[1] [18] Waypoint: X: 4451.238 Y: -351.0729 Z: 129.3609
[1] [19] Waypoint: X: 4451.238 Y: -351.0729 Z: 129.3609
[1] RecID: 4372
[1] InitialRawFacing: 1.121713
*/
class npc_q39682 : public CreatureScript
{
public:
    npc_q39682() : CreatureScript("npc_q39682") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q39682AI(creature);
    }

    struct npc_q39682AI : public npc_escortAI
    {
        npc_q39682AI(Creature* creature) : npc_escortAI(creature), PlayerOn(false)
        {
        }

        bool PlayerOn;
        void Reset() override
        {
            PlayerOn = false;
        }

        void OnCharmed(bool /*apply*/) override
        {
        }


        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (!apply || who->GetTypeId() != TYPEID_PLAYER)
                return;

            PlayerOn = true;
            Start(false, true, who->GetGUID());
        }

        void WaypointReached(uint32 i) override
        {
            switch (i)
            {
                case 20:    // for NPC_BASTIA_2
                    if (Player* player = GetPlayerForEscort())
                    {
                        WorldLocation loc;
                        loc.m_mapId = 1468;
                        loc.m_positionX = 4450.89f;
                        loc.m_positionY = -431.51f;
                        loc.m_positionZ = 119.26f;
                        loc.SetOrientation(player->GetOrientation());
                        player->SetHomebind(loc, 7866);
                        player->SendBindPointUpdate();

                        player->KilledMonsterCredit(96659);
                        SetEscortPaused(true);
                        player->ExitVehicle();
                    }
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (PlayerOn)
            {
                if (Player* player = GetPlayerForEscort())
                    player->SetClientControl(me, false);
                PlayerOn = false;
            }
        }
    };
};

//Q: 39685
class npc_q39685 : public CreatureScript
{
public:
    npc_q39685() : CreatureScript("npc_q39685") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q39685AI(creature);
    }

    enum data
    {
        SPELL_ID = 0,
        SPELL_AURA = 195189,
    };
    struct npc_q39685AI : public ScriptedAI
    {
        npc_q39685AI(Creature* creature) : ScriptedAI(creature), cooldown(0)
        {
        }

        uint32 cooldown;

        void Reset() override
        {
            cooldown = 0;
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, 51861);
            me->RemoveAurasDueToSpell(SPELL_AURA);
            me->CastSpell(me, SPELL_AURA, true);
            //me->AddAura(SPELL_AURA, me);
            
        }

        void OnSpellClick(Unit* Clicker) override
        {
            if (cooldown)
                return;

            Clicker->ToPlayer()->KilledMonsterCredit(me->GetEntry());
            //Clicker->CastSpell(me, SPELL_ID, false);
                

            cooldown = 5000;
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            return;
        }

        void UpdateAI(uint32 diff) override
        {
            if (cooldown)
            {
                if (cooldown <= diff)
                {
                    me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    cooldown = 0;
                }
                else
                    cooldown -= diff;
            }
        }
    };
};

//! 96682
class npc_q39683 : public CreatureScript
{
public:
    npc_q39683() : CreatureScript("npc_q39683") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q39683AI(creature);
    }

    //! Scripted_NoMovementAI HACK!
    struct npc_q39683AI : public ScriptedAI
    {
        EventMap events;
        ObjectGuid playerGuid;
        int8 lowStep;
        bool phase2;
        
        npc_q39683AI(Creature* creature) : ScriptedAI(creature), phase2(false)
        {
            lowStep = 1;
            creature->SetCorpseDelay(30);
            creature->SetRespawnDelay(15);
            me->SetHealth(me->CountPctFromMaxHealth(21));
            me->setRegeneratingHealth(false);
        }

        enum data
        {
            REWARD_SPELL = 210486,
            REWARD_SPELL2 = 195440,
            CREDIT = 106254,                   
        };

        void EnterCombat(Unit* victim) override
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, victim->GetGUID());

            Player *player = victim->ToPlayer();
            if (!player)
                return;

            lowStep = 1;

            float health_pct = 100.0f;
            if (player->GetQuestStatus(39684) == QUEST_STATUS_COMPLETE || player->GetQuestStatus(39684) ==  QUEST_STATUS_REWARDED)
            {
                ++lowStep;
                health_pct -= 40.0f;
            }
            if (player->GetQuestStatus(39685) == QUEST_STATUS_COMPLETE || player->GetQuestStatus(39685) == QUEST_STATUS_COMPLETE)
            {
                ++lowStep;
                health_pct -= 40.0f;
            }

         //   me->SetHealth(float(me->GetMaxHealth() / 100.0f)*health_pct);
            me->SetHealth(me->CountPctFromMaxHealth(21));
            events.RescheduleEvent(EVENT_1, 20000); // 199758
            events.RescheduleEvent(EVENT_2, 15000); // 199836
        }
        
        void Reset() override
        {
            phase2 = false;
            events.Reset();
            me->SetHealth(me->CountPctFromMaxHealth(21));
            me->setRegeneratingHealth(false);
        }

        //! HACK!!! ANTIL FINISH EVENT
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            damage *= lowStep;
            if (me->HealthBelowPct(5) && !phase2)
            {
                phase2 = true;
                DoCast(199828);
                Talk(2);
            }
        }

        void JustDied(Unit* killer) override
        {
            Player *player = killer->ToPlayer();
            if (!player)
                return;

            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, player->GetGUID());
            player->CastSpell(player, REWARD_SPELL, true);
            player->CastSpell(player, REWARD_SPELL2, true);
            player->KilledMonsterCredit(CREDIT);
        }
        void UpdateAI(uint32 diff) override
        {
            UpdateVictim();

            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(199758);
                        events.RescheduleEvent(EVENT_1, 40000); // 199758
                        break;
                    case EVENT_2:
                        DoCast(199836);
                        events.RescheduleEvent(EVENT_2, 31000); // 199836
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

//! 96675
class npc_dh_questgiver_96675 : public CreatureScript
{
public:
    npc_dh_questgiver_96675() : CreatureScript("npc_dh_questgiver_96675") {}
    enum data
    {
        QUEST_03 = 39686,
        GO_ELEVATOR = 244644,
        CREDIT = 96814,
    };
    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_03) // La lecon du parchemin brulant
        {
            WorldLocation loc;
            loc.m_mapId = 1468;
            loc.m_positionX = 4317.87f;
            loc.m_positionY = -451.61f;
            loc.m_positionZ = 259.36f;
            loc.SetOrientation(player->GetOrientation());
            player->SetHomebind(loc, 7866);
            player->SendBindPointUpdate();

            creature->GetMap()->LoadGrid(4450.857f, -451.7679f); //voice

            std::list<GameObject*> lList;
            creature->GetGameObjectListWithEntryInGrid(lList, GO_ELEVATOR, 500.0f);
            if (lList.empty())
            {
                creature->MonsterSay("SCRIPT::npc_dh_questgiver_96675 no elevator", LANG_UNIVERSAL, player->GetGUID());
                return true;
            }
            //creature->MonsterSay("SCRIPT::npc_dh_questgiver_96675 OK", LANG_UNIVERSAL, player->GetGUID());

            GameObject* elevator = *lList.begin();
            player->AddToExtraLook(elevator->GetGUID());

            UpdateData transData(player->GetMapId());
            elevator->BuildCreateUpdateBlockForPlayer(&transData, player);
            player->KilledMonsterCredit(CREDIT);
            sCreatureTextMgr->SendChat(creature, TEXT_GENERIC_0, player->GetGUID());
        }

        return true;
    }
};

//! 244455
class go_q39687 : public GameObjectScript
{
public:
    go_q39687() : GameObjectScript("go_q39687") { }

    struct go_q39687AI : public GameObjectAI
    {
        go_q39687AI(GameObject* go) : GameObjectAI(go)
        {

        }

        enum data
        {
            QUEST = 39687,
            CREDIT = 100166,
        };

        bool GossipHello(Player* player) override
        {
            if (player->GetQuestStatus(QUEST) == QUEST_STATUS_INCOMPLETE)
            {
                player->CastSpell(player, 226867);
                player->KilledMonsterCredit(CREDIT);
                return false;
            }
            else
                return true;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_q39687AI(go);
    }
};

//! 96675
class npc_dh_questgiver_97644 : public CreatureScript
{
public:
    npc_dh_questgiver_97644() : CreatureScript("npc_dh_questgiver_97644") {}
    
    enum data
    {
        QUEST_02 = 40373,
        CAST_SPELL = 196650,
    };
    
    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_02) // La lecon du parchemin brulant
        {
            player->CastSpell(player, CAST_SPELL, true);
        }

        return true;
    }
    
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_dh_questgiver_97644AI(creature);
    }

    //! Scripted_NoMovementAI HACK!
    struct npc_dh_questgiver_97644AI : public Scripted_NoMovementAI
    {
        npc_dh_questgiver_97644AI(Creature* creature) : Scripted_NoMovementAI(creature) {}

        GuidSet m_player_for_event;

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle())
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (!me->IsWithinDistInMap(who, 10.0f))
                return;
            
            Conversation* conversation = new Conversation;
            if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 536, who, NULL, *who))
                delete conversation;
            m_player_for_event.insert(who->GetGUID());
        }

    };
};

//! 96783
class npc_q39694 : public CreatureScript
{
public:
    npc_q39694() : CreatureScript("npc_q39694") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q39694AI(creature);
    }

    struct npc_q39694AI : public ScriptedAI
    {
        SummonList summons;
        EventMap events;
        ObjectGuid playerGuid;
        npc_q39694AI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            creature->SetCorpseDelay(30);
            creature->SetRespawnDelay(15);
        }

        enum data
        {
            REWARD_SPELL = 210500,
            REWARD_SPELL2 = 195450,
            CREDIT = 106255,
            CREDIT2 = 113812,
            
            SPELL_1 = 200027,  // 45
            SPELL_2 = 200007,  // 5 + 35
            SPELL_3 = 200353,  // 16 + 50  say + summon
            SPELL_4 = 200002,  // 23  say
            
        };
        
        void Reset() override
        {
            summons.DespawnAll();
            events.Reset();
        }

        void EnterCombat(Unit* victim) override
        {
            Talk(0);
            events.RescheduleEvent(EVENT_1, 45000);
            events.RescheduleEvent(EVENT_2, 5000);
            events.RescheduleEvent(EVENT_3, 15000);
            events.RescheduleEvent(EVENT_4, 23000);
        }
        
        void JustSummoned(Creature* summon) override
        { 
            summons.Summon(summon);
        }

        void JustDied(Unit* killer) override
        {
            Player *player = killer->ToPlayer();
            if (!player)
                return;

            Talk(2);

            std::list<Player*> playerList;
            me->GetPlayerListInGrid(playerList, 60.0f);
            for (auto plr : playerList)
            {
                plr->CastSpell(plr, REWARD_SPELL, true);
                plr->CastSpell(plr, REWARD_SPELL2, true);
                plr->KilledMonsterCredit(CREDIT);
                plr->KilledMonsterCredit(CREDIT2);
            }
        }
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;    

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_1);
                        events.RescheduleEvent(EVENT_1, 45000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_2);
                        events.RescheduleEvent(EVENT_2, 35000);
                        break;
                    case EVENT_3:
                    {
                        DoCast(SPELL_3);
                        Talk(1);
                        for (uint8 i = 0; i < 4; i++)
                        {
                            if (Creature* add = me->SummonCreature(101505, 4221.51f + irand(-3, 3), -627.92f + irand(-3, 3), 255.12f, 3.10f))
                            {
                                add->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                                DoZoneInCombat(add, 50.0f);
                            }
                        }
                        for (uint8 i = 0; i < 4; i++)
                        {
                            if (Creature* add = me->SummonCreature(101505, 4146.97f + irand(-3, 3), -626.29f + irand(-3, 3), 255.12f, 6.27f))
                            {
                                add->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                                DoZoneInCombat(add, 50.0f);
                            }
                        }
                        events.RescheduleEvent(EVENT_3, 50000);
                    }
                        break;
                    case EVENT_4:
                        DoCast(SPELL_4);
                        events.RescheduleEvent(EVENT_4, 23000);
                        break;
                }

            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_92718 : public CreatureScript
{
public:
    npc_92718() : CreatureScript("npc_92718") { }
    
    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == 38672)
        {
            if (Creature* tar = player->SummonCreature(creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation()))
            {
                sCreatureTextMgr->SendChat(tar, TEXT_GENERIC_1, player->GetGUID());
                tar->GetMotionMaster()->MovePoint(0, 4325.57f, -562.81f, -281.74f);
                tar->DespawnOrUnsummon(7000);
                tar->AddPlayerInPersonnalVisibilityList(player->GetGUID());
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_92718AI(creature);
    }

    struct npc_92718AI : public ScriptedAI
    {
        npc_92718AI(Creature* creature) : ScriptedAI(creature) {}

        GuidSet m_player_for_event;

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle())
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (!me->IsWithinDistInMap(who, 10.0f))
                return;
            
            who->CastSpell(who, 192605, true);
            m_player_for_event.insert(who->GetGUID());
        }

    };
};

class spell_199760 : public SpellScriptLoader
{
    public:
        spell_199760() : SpellScriptLoader("spell_199760") { }

        class spell_199760_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_199760_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                PreventHitDefaultEffect(EFFECT_0);

                if (!GetCaster())
                    return;

                Position pos;
                GetCaster()->GetRandomNearPosition(pos, 20.0f);
                WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());
                dest->Relocate(pos);
            }

            void Register() override
            {
                OnEffectLaunch += SpellEffectFn(spell_199760_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_199760_SpellScript();
        }
};

class npc_102391 : public CreatureScript
{
public:
    npc_102391() : CreatureScript("npc_102391") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_102391AI(creature);
    }

    struct npc_102391AI : public npc_escortAI
    {
        npc_102391AI(Creature* creature) : npc_escortAI(creature) {}

        GuidSet m_player_for_event;
        ObjectGuid leftAdd;
        ObjectGuid rightAdd;
        ObjectGuid bossGUID = ObjectGuid::Create<HighGuid::Creature>(me->GetMapId(), 96680, 365929);

        void IsSummonedBy(Unit* summoner) override
        {
            me->RemoveAura(202212);
            if (summoner->GetTypeId() != TYPEID_PLAYER)
                return;

            Start(false, true, summoner->GetGUID());
            if (TempSummon* add = me->SummonCreature(96656, 4445.42, -679.495, 117.316, 5.73285, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 28500))
            {
                add->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
                leftAdd = add->GetGUID();
            }
            if (TempSummon* add = me->SummonCreature(96656, 4453.46, -680.844, 117.284, 4.0283, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 28500))
            {
                add->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
                rightAdd = add->GetGUID();
            }
        }

        void WaypointReached(uint32 waypointId) override
        {
            switch (waypointId)
            {
            case 3:
                Talk(TEXT_GENERIC_1);
                break;
            }
        }

        void WaypointStart(uint32 waypointId) override
        {
            switch (waypointId)
            {
            case 1:
                if (Creature* boss = me->GetCreature(*me, bossGUID))
                    boss->CastSpell(boss, 202220, true);
                break;
            case 4:
                if (Creature* left = me->GetCreature(*me, leftAdd))
                    left->GetMotionMaster()->MoveFollow(me, 0.5f, 90.0f);
                if (Creature* right = me->GetCreature(*me, rightAdd))
                    right->GetMotionMaster()->MoveFollow(me, 0.5f, 270.0f);
                break;
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            // only the static spawn should do this, the summons are for the escape sequence
            if (me->isSummon())
                return;

            if (who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle())
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (!me->IsWithinDistInMap(who, 50.0f))
                return;

            if (who->ToPlayer()->GetQuestStatus(39684) == QUEST_STATUS_INCOMPLETE)
                Talk(TEXT_GENERIC_0, who->GetGUID());
            
            m_player_for_event.insert(who->GetGUID());
        }

    };
};

class spell_196460 : public SpellScriptLoader
{
public:
    spell_196460() : SpellScriptLoader("spell_196460") { }

    class spell_196460_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_196460_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster())
                return;

            uint32 missiles = urand(1, 3);
            float directionX = frand(0.0f, 6.0f);
            float directionY = 6.0f - directionX;
            if (urand(0, 1))
                directionX *= -1.0f;
            if (urand(0, 1))
                directionY *= -1.0f;

            Position pos;
            GetCaster()->GetRandomNearPosition(pos, 25.0f);
            for (int i = 0; i < missiles; i++)
            {
                GetCaster()->CastSpellDelay(pos, 196504, true, i * 1000);
                pos.m_positionX += directionX;
                pos.m_positionY += directionY;
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_196460_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_196460_SpellScript();
    }
};

class spell_196462 : public SpellScriptLoader
{
public:
    spell_196462() : SpellScriptLoader("spell_196462") { }

    class spell_196462_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_196462_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster())
                return;

            Position pos = GetCaster()->GetPosition();
            pos.SetOrientation(frand(0.0f, 2.0f) * static_cast<float>(M_PI));
            GetCaster()->CastSpell(pos, 194853, true);
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_196462_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_196462_SpellScript();
    }
};

// 103658 103655
class npc_q38672 : public CreatureScript
{
public:
    npc_q38672() : CreatureScript("npc_q38672") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_q38672AI(creature);
    }

    //! Scripted_NoMovementAI HACK!
    struct npc_q38672AI : public Scripted_NoMovementAI
    {
        npc_q38672AI(Creature* creature) : Scripted_NoMovementAI(creature) {}

        GuidSet m_player_for_event;

        void OnSpellClick(Unit* Clicker) override
        {
            if (Clicker->GetTypeId() == TYPEID_PLAYER)
            {                
                Player* player = Clicker->ToPlayer();
                if (player->GetQuestStatus(38672) != QUEST_STATUS_INCOMPLETE)
                    return;
                
              //  me->AddToHideList(player->GetGUID());
              //  me->DestroyForPlayer(player);
                
                if (me->GetEntry() == 103655)
                {
                    if (player->GetReqKillOrCastCurrentCount(38672, 99632))
                        return;
                    
                    if (Creature* targ = Clicker->SummonCreature(99632, 4309.16f, -589.73f, -281.40f, 6.09f))
                    {
                        targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                        targ->GetMotionMaster()->MovePath(10267122, false);  // 1 7
                        targ->LoadEquipment(0, true);
                        targ->DespawnOrUnsummon(60000);
                    }
                    
                    if (!player->GetReqKillOrCastCurrentCount(38672, 92848))
                    {
                        new delayed_complete(player);
                    }
                }
                if (me->GetEntry() == 103658)
                {
                    if (player->GetReqKillOrCastCurrentCount(38672, 99631))
                        return;
                    
                    if (Creature* targ = Clicker->SummonCreature(99631, 4341.70f, -589.69f, -281.40f, 3.26f))
                    {
                        targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                        targ->GetMotionMaster()->MovePath(10267123, false);  // 1 7
                        targ->LoadEquipment(0, true );
                        targ->DespawnOrUnsummon(60000);
                    }
                    
                    if (!player->GetReqKillOrCastCurrentCount(38672, 92849))
                    {
                        new delayed_complete(player);
                    }
                }
                
                if(Creature* targ = Clicker->SummonCreature(me->GetEntry(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                {
                    targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                    targ->PlayOneShotAnimKit(3761);
                    targ->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    targ->DespawnOrUnsummon(2000);
                }
            }
        }

    };
    
    class delayed_complete : public BasicEvent
    {
        public:
        delayed_complete(Player* player) : player(player)
        {
            player->m_Events.AddEvent(this, player->m_Events.CalculateTime(7000));
        }

        bool Execute(uint64 /*time*/, uint32 /*diff*/) override
        {
            player->KilledMonsterCredit(99326);
            return true;
        }
        Player* player;
    };
};

class go_244923 : public GameObjectScript
{
public:
    go_244923() : GameObjectScript("go_244923") { }

    bool OnQuestReward(Player* player, GameObject* go, Quest const* quest, uint32) override
    {
        if (quest->GetQuestId() == 38672)
        {
            if (Creature* targ = player->SummonCreature(92980, 4328.41f, -583.50f, -281.92f, 3.29f))
            {
                targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                sCreatureTextMgr->SendChat(targ, TEXT_GENERIC_0, player->GetGUID());
                targ->GetMotionMaster()->MovePoint(0, 4330.08f, -548.28f, -281.75f);
                targ->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                targ->DespawnOrUnsummon(11000);
            }
            
            if (Creature* targ = player->SummonCreature(92986, 4322.64f, -583.44f, -281.92f, 0.02f))
            {
                targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                targ->GetMotionMaster()->MovePoint(0, 4321.75f, -548.86f, -281.75f);
                targ->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                targ->DespawnOrUnsummon(11000);
            }
        }
        return true;
    }
};

class npc_96665 : public CreatureScript
{
public:
    npc_96665() : CreatureScript("npc_96665") {}
        
    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        switch(quest->GetQuestId()) 
        {
            case 39682:
                if(Creature* targ = player->SummonCreature(creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation()))
                {
                    targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                    sCreatureTextMgr->SendChat(targ, TEXT_GENERIC_0, player->GetGUID());
                    targ->DespawnOrUnsummon(15000);
                    targ->GetMotionMaster()->MovePoint(0, 4243.23f, -298.26f, -281.19f);
                }  
                if(Creature* targ = player->SummonCreature(92986, 4078.70f, -288.80f, -281.45f, 3.16f))
                {
                    targ->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                    targ->GetMotionMaster()->MovePoint(0, 4243.23f, -298.26f, -281.19f);
                    targ->DespawnOrUnsummon(15000);
                }  
                break;
            
            default:
                break;
        }

        return true;
    }
    
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_96665AI(creature);
    }
    struct npc_96665AI : public ScriptedAI
    {
        npc_96665AI(Creature* creature) : ScriptedAI(creature) 
        {
            check1 = false;
            check2 = false;
            timer = 0;
        }
        
        bool check1;
        bool check2;
        uint32 timer;
        
        void IsSummonedBy(Unit* summoner) override
        {
            if (summoner->GetTypeId() == TYPEID_PLAYER)
            {
                check1 = true;
                timer = 4000;
            }
            else 
                me->DespawnOrUnsummon();
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (check1)
                if (timer <= diff)
                {
                    if (Creature* t = me->FindNearestCreature(92986, 50.0f, true))
                        t->AI()->Talk(2);
                    check1 = false;
                    timer = 5000;
                    check2 = true;
                } else timer -= diff;
                
            if (check2)
                if (timer <= diff)
                {
                    Talk(2);
                    check2 = false;
                } else timer -= diff;
        }

    };
};

class npc_96666 : public CreatureScript
{
public:
    npc_96666() : CreatureScript("npc_96666") {}
            
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_96666AI(creature);
    }
    struct npc_96666AI : public ScriptedAI
    {
        npc_96666AI(Creature* creature) : ScriptedAI(creature) {}
        GuidSet m_player_for_event;
        
        void MoveInLineOfSight(Unit* who) override
        {
            if (who->GetTypeId() != TYPEID_PLAYER || !me->IsWithinDistInMap(who, 10.0f))
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;           

            if (Creature* targ = who->SummonCreature(97978, 4149.20f, -877.07f, 291.37f, 1.01f, TEMPSUMMON_MANUAL_DESPAWN, 0, who->GetGUID(), NULL))
            {
                targ->SetDisplayId(68476);
                targ->GetMotionMaster()->MovePoint(0, 4154.08f, -868.86f, 291.69f, 1.00f);
            }
            
            m_player_for_event.insert(who->GetGUID());
        }
    };
    
};



void AddSC_warden_prison()
{
    new go_q38690();
    new npc_q38689();
    new spell_q38723();
    new npc_q38723();
    new npc_q39682();
    new npc_q39685();
    new npc_q39683();
    new npc_dh_questgiver_96675();
    new go_q39687();
    new npc_dh_questgiver_97644();
    new npc_q39694();
    new npc_92718();
    new spell_199760();
    new npc_102391();
    new spell_196460();
    new spell_196462();
    new npc_q38672();
    new go_244923();
    new npc_96665();
    new npc_96666();
}
