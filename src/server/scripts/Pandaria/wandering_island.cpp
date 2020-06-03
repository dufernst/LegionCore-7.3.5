#include "ScriptedEscortAI.h"
#include "CreatureTextMgr.h"
#include "MapManager.h"
#include "ScriptMgr.h"
#include "Packets/WorldStatePackets.h"

enum panda_npc
{
    NPC_ANNOUNCER_1                                = 60183,
    NPC_ANNOUNCER_2                                = 60244,
    NPC_ANNOUNCER_3                                = 54943,
    NPC_ANNOUNCER_4                                = 54568,
    NPC_ANNOUNCER_5_TRAVEL                         = 57712,
    NPC_ANNOUNCER_6                                = 55694,
    NPC_ANNOUNCER_7                                = 55672,
    NPC_ANNOUNCER_8                                = 60889,
    NPC_AMBERLEAF_SCAMP                            = 54130, //Amberleaf Scamp
    NPC_MIN_DIMWIND_OUTRO                          = 56503, //Min Dimwind
    NPC_MASTER_LI_FAI                              = 54856, 
    NPC_EAST_CHILDREN_CAI                          = 60250,
    NPC_EAST_CHILDREN_DEN                          = 60249,
    NPC_AYSA_WATTER_OUTRO_EVENT                    = 54975,
    NPC_XO_TREVELER                                = 54958,
};

enum panda_quests
{
    QUEST_THE_DISCIPLE_CHALLENGE                   = 29409, //29409 The Disciple's Challenge
    QUEST_MISSING_DRIVER                           = 29419,
    QUEST_AYSA_OF_TUSHUI                           = 29410, // Aysa of the Tushui
    QUEST_PARCHEMIN_VOLANT                         = 29421,
    QUEST_PASSION_OF_SHEN                          = 29423, //The Passion of Shen-zin Su
    QUEST_NEW_FRIEND                               = 29679,
    QUEST_SINGING_POOLS                            = 29521, // The Singing Pools
    QUEST_SOURCE_OF_OUR_LIVELIHOOD                 = 29680, // The Source of Our Livelihood
    QUEST_NOT_IN_FACE                              = 29774,
    QUEST_SPIRIT_AND_BODY                          = 29775, //The Spirit and Body of Shen-zin Su
    QUEST_MORNING_BREEZE_BILLAGE                   = 29776, // Morning Breeze Village
    QUEST_BALANCED_PERSPECTIVE                     = 29784, // Balanced Perspective
    QUST_DAFENG_SPIRIT_OF_AIR                      = 29785, // Dafeng, the Spirit of Air
    QIEST_BATTLE_FOR_SKIES                         = 29786, // Battle for the Skies
    QUEST_PASSING_WISDOM                           = 29790,
    QUEST_SUF_SHUN_ZI                              = 29791,
    QUEST_BIDDEN_TO_GREATNESS                      = 29792, // Bidden to Greatness
    QUEST_NONE_LEFT_BEHINED                        = 29794, //None Left Behind
    QUEST_ACIENT_EVIL                              = 29798,
    QUEST_RISKING_IT_ALL                           = 30767, //Risking It All
    QUEST_HEALING_SHEN                             = 29799,
    QUSRT_NEW_FATE                                 = 31450, //A New Fate
};

enum spell_panda
{
    SPELL_SUMMON_CHILDREN                          = 116190,
    SPELL_CSA_AT_TIMER                             = 116219, //CSA Area Trigger Dummy Timer Aura A
    SPELL_SUMMON_SPIRIT_OF_WATTER                  = 103538,
    SPELL_CREDIT_NOT_IN_FACE                       = 104017, // Quest credit Not In the Face!
    SPELL_SUMMON_WIND_TELEPORTER                   = 104396,
    SUMMON_MANDORI_DOOR                            = 115426, // Summon Mandori Door
    SUMMON_PEI_WU_DOOR                             = 115435, // Summon Pei-Wu Door
    SUMMON_GO_TRIGER_CHECKER                       = 115343,
};

class npc_panda_announcer : public CreatureScript
{
    public:
        npc_panda_announcer() : CreatureScript("npc_panda_announcer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_panda_announcerAI(creature);
    }
    
    struct npc_panda_announcerAI : public ScriptedAI
    {
        npc_panda_announcerAI(Creature* creature) : ScriptedAI(creature)
        {

        }

        void Reset()
        {
            text = TEXT_GENERIC_0;
            targetGUID.Clear();
        }

        enum events
        {
            EVENT_1            = 1,
            EVENT_2_ANNOUNCER6 = 2,
            EVENT_CLEAR        = 3,
        };

        uint32 text;
        ObjectGuid targetGUID;
        GuidSet m_player_for_event;
        EventMap events;
        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle())
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (!me->IsWithinDistInMap(who, 60.0f))
                return;

            uint32 eTimer = 4000;

            switch(me->GetEntry())
            {
                case NPC_ANNOUNCER_1:
                    if (who->ToPlayer()->GetQuestStatus(QUEST_THE_DISCIPLE_CHALLENGE) != QUEST_STATUS_INCOMPLETE)
                        return;
                    break;
                case NPC_ANNOUNCER_2:
                case NPC_ANNOUNCER_3:
                    if (who->ToPlayer()->GetQuestStatus(QUEST_AYSA_OF_TUSHUI) == QUEST_STATUS_REWARDED)
                        return;
                    break;
                case NPC_ANNOUNCER_5_TRAVEL:
                    if (me->GetAreaId() == 5826 && who->ToPlayer()->GetQuestStatus(QUEST_NEW_FRIEND) != QUEST_STATUS_REWARDED) // Bassins chantants
                        return;
                    if (me->GetAreaId() == 5881) // Ferme Dai-Lo
                    {
                        if (who->ToPlayer()->GetQuestStatus(QUEST_NOT_IN_FACE) != QUEST_STATUS_REWARDED)
                            return;
                        text = TEXT_GENERIC_1;
                    }
                    if (me->GetAreaId() == 5833) // Epave du Chercheciel
                    {
                        if (who->ToPlayer()->GetQuestStatus(QUEST_NOT_IN_FACE) != QUEST_STATUS_REWARDED)
                            return;
                        text = TEXT_GENERIC_1;
                    }
                    break;
                case NPC_ANNOUNCER_6:
                    if (!me->IsWithinDistInMap(who, 35.0f))
                        return;
                    break;
                case NPC_ANNOUNCER_8:
                    eTimer = 13000;
                    break;
                default:
                    break;
            }

            m_player_for_event.insert(who->GetGUID());
            events.RescheduleEvent(EVENT_1, eTimer);
            events.RescheduleEvent(EVENT_CLEAR, 300000);
            targetGUID = who->GetGUID();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        sCreatureTextMgr->SendChat(me, text, targetGUID);
                        if (me->GetEntry() == NPC_ANNOUNCER_6)
                            events.RescheduleEvent(EVENT_2_ANNOUNCER6, 5000);
                        break;
                    case EVENT_2_ANNOUNCER6:
                        me->DespawnOrUnsummon(5000);
//                        me->GetMotionMaster()->MovePoint(1, 919.6441f, 3631.506f, 251.9946f);
                        me->GetMotionMaster()->MovePoint(1, 902.8281f, 3667.672f, 268.9162f);
                        break;
                    case EVENT_CLEAR:
                        m_player_for_event.clear();
                        break;
                }
            }
        }
    };
};

class mob_tushui_trainee : public CreatureScript
{
    public:
        mob_tushui_trainee() : CreatureScript("mob_tushui_trainee") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_tushui_trainee_AI(creature);
        }

        struct mob_tushui_trainee_AI : public ScriptedAI
        {
            mob_tushui_trainee_AI(Creature* creature) : ScriptedAI(creature) {}

            enum data
            {
                EVENT_1     = 1,
                EVENT_2     = 2,
                SPELL       = 109080,
            };

            EventMap events;

            void Reset()
            {
                events.Reset();
                me->SetReactState(REACT_DEFENSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                events.RescheduleEvent(EVENT_2, 5000);
            }

            void EnterCombat(Unit* unit)
            {
                events.RescheduleEvent(EVENT_1, 5000);
                events.CancelEvent(EVENT_2);
            }

            void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType)
            {
                if (attacker && me->HealthBelowPctDamaged(5, damage))
                {
                    if(attacker->GetTypeId() == TYPEID_PLAYER)
                        attacker->ToPlayer()->KilledMonsterCredit(54586, ObjectGuid::Empty);
                    //me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, attacker->GetGUID());
                    me->CombatStop();
                    me->SetFullHealth();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    me->DespawnOrUnsummon(4000);
                    damage = 0;
                }
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_1:
                            if (me->getVictim())
                                me->CastSpell(me->getVictim(), SPELL, true);
                            events.RescheduleEvent(EVENT_1, 5000);
                            break;
                        case EVENT_2:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_MONKOFFENSE_ATTACKUNARMEDOFF);
                            events.RescheduleEvent(EVENT_2, 5000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

// Should be done by summon npc 59591
class mob_master_shang_xi : public CreatureScript
{
    public:
        mob_master_shang_xi() : CreatureScript("mob_master_shang_xi") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
        {
            if (quest->GetQuestId() == 29408) // La lecon du parchemin brulant
            {
                creature->AddAura(114610, creature);
                creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                creature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            }

            return true;
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_master_shang_xi_AI(creature);
        }

        struct mob_master_shang_xi_AI : public ScriptedAI
        {
            mob_master_shang_xi_AI(Creature* creature) : ScriptedAI(creature)
            {
                checkPlayersTime = 2000;
            }

            uint32 checkPlayersTime;

            void SpellHit(Unit* caster, const SpellInfo* pSpell)
            {
                if (pSpell->Id == 114746) // Attraper la flamme
                {
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (caster->ToPlayer()->GetQuestStatus(29408) == QUEST_STATUS_INCOMPLETE)
                        {
                            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                            me->CastSpell(caster, 114611, true);
                            me->RemoveAurasDueToSpell(114610);
                            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (checkPlayersTime <= diff)
                {
                    std::list<Player*> playerList;
                    GetPlayerListInGrid(playerList, me, 5.0f);

                    bool playerWithQuestNear = false;

                    for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                    {
                        if ((*itr)->GetQuestStatus(29408) == QUEST_STATUS_INCOMPLETE && !(*itr)->HasItemCount(80212))
                                playerWithQuestNear = true;
                    }

                    if (playerWithQuestNear && !me->HasAura(114610))
                    {
                        me->AddAura(114610, me);
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    }
                    else if (!playerWithQuestNear && me->HasAura(114610))
                    {
                        me->RemoveAurasDueToSpell(114610);
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    }

                    checkPlayersTime = 2000;
                }
                else
                    checkPlayersTime -= diff;
            }
        };
};

// cast 88811 for check something
class boss_jaomin_ro : public CreatureScript
{
public:
    boss_jaomin_ro() : CreatureScript("boss_jaomin_ro") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jaomin_roAI(creature);
    }
    
    struct boss_jaomin_roAI : public ScriptedAI
    {
        boss_jaomin_roAI(Creature* creature) : ScriptedAI(creature) {}

        enum eEvents
        {
            EVENT_JAOMIN_JUMP   = 1,
            EVENT_HIT_CIRCLE    = 2,
            EVENT_FALCON        = 3,
            EVENT_RESET         = 4,
        };

        EventMap events;
        bool isInFalcon;
        bool fightEnd;

        void EnterCombat(Unit* unit)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
            events.RescheduleEvent(EVENT_JAOMIN_JUMP, 1000);
            events.RescheduleEvent(EVENT_HIT_CIRCLE, 2000);
        }

        void Reset()
        {
            events.Reset();
            me->SetReactState(REACT_DEFENSIVE);
            me->SetDisplayId(39755);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);
            isInFalcon = false;
            fightEnd = false;
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPctDamaged(30, damage) && !isInFalcon)
            {
                isInFalcon = true;
                me->SetDisplayId(39796); //faucon
                events.RescheduleEvent(EVENT_FALCON, 1000);
                events.CancelEvent(EVENT_JAOMIN_JUMP);
                events.CancelEvent(EVENT_HIT_CIRCLE);
            }

            if (me->HealthBelowPctDamaged(5, damage) && !fightEnd)
            {
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);

                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 10.0f);
                for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                    (*itr)->KilledMonsterCredit(me->GetEntry(), ObjectGuid::Empty);

                fightEnd = true;
                me->StopAttack();
                me->SetFullHealth();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);
                attacker->AttackStop();
                me->SetDisplayId(39755);
                me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                events.Reset();
                events.RescheduleEvent(EVENT_RESET, 9000);
                DoCast(me, 108959, true);
                damage = 0;
            }

            if (damage >= me->GetHealth())
                damage = 0;
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (CheckHomeDistToEvade(diff, 30.0f))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_JAOMIN_JUMP:
                        DoCastVictim(108938);
                        events.RescheduleEvent(EVENT_JAOMIN_JUMP, 30000);
                        break;
                    case EVENT_HIT_CIRCLE:
                        DoCastVictim(119301);
                        events.RescheduleEvent(EVENT_HIT_CIRCLE, 8000);
                        break;
                    case EVENT_FALCON:
                        DoCast(108935);
                        events.RescheduleEvent(EVENT_FALCON, 6000);
                        break;
                    case EVENT_RESET:
                        me->NearTeleportTo(me->GetHomePosition());
                        EnterEvadeMode();
                    	break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class mob_attacker_dimwind : public CreatureScript
{
public:
    mob_attacker_dimwind() : CreatureScript("mob_attacker_dimwind") { }
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_attacker_dimwindAI(creature);
    }
    
    struct mob_attacker_dimwindAI : public ScriptedAI
    {
        mob_attacker_dimwindAI(Creature* creature) : ScriptedAI(creature) {}
        
        void DamageTaken(Unit* pDoneBy, uint32 &uiDamage, DamageEffectType dmgType)
        {
            if(me->GetHealthPct() < 90 && pDoneBy && pDoneBy->ToCreature() && pDoneBy->ToCreature()->GetEntry() == 54785)
                uiDamage = 0;
        }

        void JustDied(Unit* killer)
        {
            if (killer->GetTypeId() != TYPEID_PLAYER || !me->ToTempSummon())
                return;

            if (Creature* owner = me->GetMap()->GetCreature(me->ToTempSummon()->GetSummonerGUID()))
                owner->AI()->SetGUID(killer->GetGUID(), 0);
        }
    };
};

class mob_min_dimwind : public CreatureScript
{
public:
    mob_min_dimwind() : CreatureScript("mob_min_dimwind") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_min_dimwindAI(creature);
    }
    
    struct mob_min_dimwindAI : public ScriptedAI
    {
        EventMap events;
        GuidSet guidMob;
        ObjectGuid plrGUID;
        GuidSet m_player_for_event;
        bool mt;

        mob_min_dimwindAI(Creature* creature) : ScriptedAI(creature)
        {
            mt = false;
        }

        void Reset()
        {
            me->setActive(true);
            me->HandleEmoteCommand(EMOTE_STATE_READY2H);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() != TYPEID_PLAYER || who->ToPlayer()->GetQuestStatus(QUEST_MISSING_DRIVER) != QUEST_STATUS_INCOMPLETE)
                return;
            
            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            m_player_for_event.insert(who->GetGUID());
            if (!mt)
            {
                mt = true;
                InitMobs(who);
            }
        }

        void DamageTaken(Unit* pDoneBy, uint32 &uiDamage, DamageEffectType dmgType)
        {
            if(me->GetHealthPct() < 25 && pDoneBy && pDoneBy->ToCreature() && pDoneBy->ToCreature()->GetEntry() == NPC_AMBERLEAF_SCAMP)
                uiDamage = 0;
        }
        
        void SetGUID(ObjectGuid const& guid, int32 /*id*/ = 0)
        {
            plrGUID = guid;
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            guidMob.erase(summon->GetGUID());
            if (guidMob.empty())
            {
                mt = false;
                me->HandleEmoteCommand(EMOTE_STATE_STAND);
                if (Player* target = sObjectAccessor->FindPlayer(plrGUID))
                {
                    target->KilledMonsterCredit(54855, ObjectGuid::Empty);
                    // by spell 106205
                    if(TempSummon* mind = target->SummonCreature(NPC_MIN_DIMWIND_OUTRO, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
                    {
                        mind->AI()->SetGUID(plrGUID, 0);
                        mind->setFaction(35);
                    }
                }
                me->DespawnOrUnsummon(1000);
            }
        }
        
        void InitMobs(Unit* who)
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY2H);
            for(GuidSet::iterator itr = guidMob.begin(); itr != guidMob.end(); ++itr)
                if (Creature* c = me->GetMap()->GetCreature(*itr))
                    c->DespawnOrUnsummon(1000);
            guidMob.clear();

            for(int i = 0; i < 4; i++)
            {
                if(TempSummon* temp = me->SummonCreature(NPC_AMBERLEAF_SCAMP, me->GetPositionX()-3+rand()%6, me->GetPositionY() + 4 + rand()%4, me->GetPositionZ()+2, 3.3f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
                {
                    guidMob.insert(temp->GetGUID());
                    
                    if (i == 0)
                        sCreatureTextMgr->SendChat(temp->ToCreature(), TEXT_GENERIC_0, who->GetGUID());

                    temp->SetFacingToObject(me);
                    temp->HandleEmoteCommand(EMOTE_STATE_READY2H);
                    temp->Attack(me, true);
                    //temp->getThreatManager().addThreat(me, 250.0f);
                }
            }
        }
    };
};

class npc_min_dimwind_outro : public CreatureScript
{
public:
    npc_min_dimwind_outro() : CreatureScript("npc_min_dimwind_outro") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_min_dimwind_outroAI (creature);
    }

    struct npc_min_dimwind_outroAI : public npc_escortAI
    {
        npc_min_dimwind_outroAI(Creature* creature) : npc_escortAI(creature) {}

        EventMap events;
        ObjectGuid playerGUID;

        enum eEvents
        {
            EVENT_1    = 1,
            EVENT_2    = 2,
        };

        void Reset()
        {
            playerGUID.Clear();
        }

        void SetGUID(ObjectGuid const& guid, int32 id)
        {
            playerGUID = guid;
            events.RescheduleEvent(EVENT_1, 1000);
        }

        void WaypointReached(uint32 pointId)
        {            
            switch(pointId)
            {
                case 3:
                case 4:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGUID);
                    break;
                case 12:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGUID);
                    me->DespawnOrUnsummon(30000);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGUID);
                        events.RescheduleEvent(EVENT_2, 1000);
                        break;
                    case EVENT_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGUID);
                        Start(true, true);
                        break;
                    default:
                        break;
                }
            }
        }
    };
};

class mob_aysa_lake_escort : public CreatureScript
{
public:
    mob_aysa_lake_escort() : CreatureScript("mob_aysa_lake_escort") { }

    struct mob_aysa_lake_escortAI : public npc_escortAI
    {        
        mob_aysa_lake_escortAI(Creature* creature) : npc_escortAI(creature)
        {}

        uint32 IntroTimer;

        void Reset()
        {
            IntroTimer = 2500;
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            npc_escortAI::MovementInform(uiType, uiId);

            if (uiType != POINT_MOTION_TYPE && uiType != EFFECT_MOTION_TYPE)
                return;

            switch (uiId)
            {
                case 10:
                    me->GetMotionMaster()->MoveJump(1227.11f, 3489.73f, 100.37f, 10, 20, 11);
                    break;
                case 11:
                    me->GetMotionMaster()->MoveJump(1236.68f, 3456.68f, 102.58f, 10, 20, 12);
                    break;
                case 12:
                    Start(false, true);
                    break;
                default:
                    break;
            }
        }

        void IsSummonedBy(Unit* summoner)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
        }

        void WaypointReached(uint32 waypointId)
        {
            if (waypointId == 4)
                me->DespawnOrUnsummon(500);
        }

        void UpdateAI(uint32 diff)
        {
            if (IntroTimer)
            {
                if (IntroTimer <= diff)
                {
                    IntroTimer = 0;
                    me->GetMotionMaster()->MoveJump(1216.78f, 3499.44f, 91.15f, 10, 20, 10);
                }
                else
                    IntroTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysa_lake_escortAI(creature);
    }
    
};

class mob_aysa : public CreatureScript
{
public:
    mob_aysa() : CreatureScript("mob_aysa") { }
   
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysaAI(creature);
    }
    
    struct mob_aysaAI : public ScriptedAI
    {
        EventMap events;
        std::list<Player*> playersInvolved;
        ObjectGuid lifeiGUID;
        bool inCombat;
        uint32 timer;
        
        mob_aysaAI(Creature* creature) : ScriptedAI(creature)
        {
            events.RescheduleEvent(1, 600); //Begin script
            inCombat = false;
            timer = 0;
            lifeiGUID.Clear();
            me->SetReactState(REACT_DEFENSIVE);
            me->setFaction(2263);
        }

        enum eEvents
        {
            EVENT_START         = 1,
            EVENT_SPAWN_MOBS    = 2,
            EVENT_PROGRESS      = 3,
            EVENT_END           = 4,
        };

        enum eText
        {
            TEXT_TUSHI_0          = 0,
            TEXT_TUSHI_1          = 1,
            TEXT_TUSHI_2          = 2,
            TEXT_TUSHI_3          = 3,
            TEXT_TUSHI_4          = 4,
            TEXT_TUSHI_5          = 5,
            TEXT_TUSHI_6          = 6,
        };
        
        void DamageTaken(Unit* pDoneBy, uint32 &uiDamage, DamageEffectType dmgType)
        {
            if(me->HealthBelowPctDamaged(5, uiDamage))
            {
                if (Creature* lifei = me->GetMap()->GetCreature(lifeiGUID))
                {
                    lifei->DespawnOrUnsummon(0);
                    lifeiGUID.Clear();
                    timer = 0;
                }
                
                uiDamage = 0;
                me->SetFullHealth();
                me->SetReactState(REACT_DEFENSIVE);
                
                std::list<Creature*> unitlist;
                GetCreatureListWithEntryInGrid(unitlist, me, 59637, 50.0f);
                for (std::list<Creature*>::const_iterator itr = unitlist.begin(); itr != unitlist.end(); ++itr)
                    me->Kill(*itr);
                    
                events.RescheduleEvent(EVENT_START, 20000);
                events.CancelEvent(EVENT_SPAWN_MOBS);
                events.CancelEvent(EVENT_PROGRESS);
                events.CancelEvent(EVENT_END);
            }
        }
        
        void updatePlayerList()
        {
            playersInvolved.clear();
            
            std::list<Player*> PlayerList;
            GetPlayerListInGrid(PlayerList, me, 20.0f);

            for (std::list<Player*>::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
            {
                Player* player = *itr;
                if(player->GetQuestStatus(29414) == QUEST_STATUS_INCOMPLETE)
                    playersInvolved.push_back(player);
            }
        }
        
        uint32 getLang(uint32 timer) const
        {
            switch(timer)
            {
                case 30: return TEXT_TUSHI_2;
                case 42: return TEXT_TUSHI_0;
                case 54: return TEXT_TUSHI_1;
                case 66: return TEXT_TUSHI_3;
                case 78: return TEXT_TUSHI_5;
                case 85: return TEXT_TUSHI_6;
                default: return 0;
            }
            return 0;
        }
        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_START: //Begin script if playersInvolved is not empty
                    {
                        updatePlayerList();
                        if(playersInvolved.empty())
                            events.RescheduleEvent(1, 600);
                        else
                        {
                            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                            me->SetReactState(REACT_PASSIVE);
                            timer = 0;
                            events.RescheduleEvent(EVENT_SPAWN_MOBS, 5000); //spawn mobs
                            events.RescheduleEvent(EVENT_PROGRESS, 1000); //update time
                            events.RescheduleEvent(EVENT_END, 90000); //end quest
                        }
                        break;
                    }
                    case EVENT_SPAWN_MOBS: //Spawn 3 mobs
                    {
                        updatePlayerList();
                        for(int i = 0; i < std::max((int)playersInvolved.size()*3,3); i++)
                        {
                            if(TempSummon* temp = me->SummonCreature(59637, 1144.55f, 3435.65f, 104.97f, 3.3f,TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                            {
                                if (temp->AI())
                                    temp->AI()->AttackStart(me);

                                temp->AddThreat(me, 250.0f);
                                temp->GetMotionMaster()->Clear();
                                temp->GetMotionMaster()->MoveChase(me);
                            }
                        }
                        events.RescheduleEvent(EVENT_SPAWN_MOBS, 20000); //spawn mobs
                        break;
                    }
                    case EVENT_PROGRESS: //update energy
                    {
                        timer++;
                        if(timer == 25 && !lifeiGUID)
                        {
                            if (Creature *lifei = me->SummonCreature(NPC_MASTER_LI_FAI, 1130.162231f, 3435.905518f, 105.496597f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                sCreatureTextMgr->SendChat(lifei, TEXT_TUSHI_4);
                                lifeiGUID = lifei->GetGUID();
                            }
                        }
                        
                        if (uint32 lang = getLang(timer))
                        {
                            if (Creature* lifei = me->GetMap()->GetCreature(lifeiGUID))
                            {
                                sCreatureTextMgr->SendChat(lifei, lang);
                                if(timer == 85)
                                {
                                    lifei->DespawnOrUnsummon(0);
                                    lifeiGUID.Clear();
                                }
                            }
                        }
                        
                        updatePlayerList();
                        for (std::list<Player*>::const_iterator itr = playersInvolved.begin(); itr != playersInvolved.end(); ++itr)
                        {
                            Player* player = *itr;
                            if(!player->HasAura(116421))
                                player->CastSpell(player, 116421);

                            player->ModifyPower(POWER_ALTERNATE, timer/25);
                            player->SetMaxPower(POWER_ALTERNATE, 90);
                        }

                        events.RescheduleEvent(EVENT_PROGRESS, 1000);
                        break;
                    }
                    case EVENT_END: //script end
                    {
                        if (Creature* lifei = me->GetMap()->GetCreature(lifeiGUID))
                        {
                            lifei->DespawnOrUnsummon(0);
                            lifeiGUID.Clear();
                            timer = 0;
                        }
                        events.RescheduleEvent(EVENT_START, 10000);
                        events.CancelEvent(EVENT_SPAWN_MOBS);
                        events.CancelEvent(EVENT_PROGRESS);
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        updatePlayerList();
                        me->SetReactState(REACT_DEFENSIVE);
                        for (std::list<Player*>::const_iterator itr = playersInvolved.begin(); itr != playersInvolved.end(); ++itr)
                        {
                            Player* player = *itr;
                            player->KilledMonsterCredit(NPC_MASTER_LI_FAI, ObjectGuid::Empty);
                            player->RemoveAura(116421);
                        }
                        break;
                    }
                }
            }
        }
    };
};

class boss_living_air : public CreatureScript
{
public:
    boss_living_air() : CreatureScript("boss_living_air") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_living_airAI(creature);
    }
    
    struct boss_living_airAI : public ScriptedAI
    {
        boss_living_airAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }
        
        EventMap events;
        
        void EnterCombat(Unit* unit)
        {
            events.RescheduleEvent(1, 3000);
            events.RescheduleEvent(2, 5000);
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;
            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case 1:
                        me->CastSpell(me->getVictim(), 108693);
                        break;
                    case 2:
                        me->CastSpell(me->getVictim(), 73212);
                        events.RescheduleEvent(2, 5000);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };
};

class boss_li_fei : public CreatureScript
{
public:
    boss_li_fei() : CreatureScript("boss_li_fei") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_PARCHEMIN_VOLANT) // La lecon du parchemin brulant
        {
            // used by spell 102445
            if (Creature* tempSummon = creature->SummonCreature(54734, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000, player->GetGUID()))
            {
                //player->CastSpell(player, 108149);  //visib
                //player->CastSpell(player, 108150);  //invis
                
                tempSummon->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                tempSummon->AI()->AttackStart(player);
                tempSummon->AI()->SetGUID(player->GetGUID());
            }
        }

        return true;
    }
};

class boss_li_fei_fight : public CreatureScript
{
public:
    boss_li_fei_fight() : CreatureScript("boss_li_fei_fight") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_li_fei_fightAI(creature);
    }
    
    struct boss_li_fei_fightAI : public ScriptedAI
    {
        EventMap events;
        std::list<Player*> playersInvolved;
        ObjectGuid playerGuid;

        boss_li_fei_fightAI(Creature* creature) : ScriptedAI(creature)
        {}

        enum eEvents
        {
            EVENT_CHECK_PLAYER      = 1,
            EVENT_FEET_OF_FURY      = 2,
            EVENT_SHADOW_KICK       = 3,
            EVENT_SHADOW_KICK_STUN  = 4,
        };

        void Reset()
        {
            // This particular entry is also spawned on an other event
            if (me->GetAreaId() != 5849) // Cavern areaid
                return;

            playerGuid.Clear();
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
            me->setFaction(16);
            events.RescheduleEvent(EVENT_CHECK_PLAYER, 2500);
            events.RescheduleEvent(EVENT_FEET_OF_FURY, 5000);
            events.RescheduleEvent(EVENT_SHADOW_KICK,  1000);
        }

        void SetGUID(ObjectGuid const& guid, int32 /*type*/) override
        {
            playerGuid = guid;
        }
        
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPctDamaged(10, damage))
            {
                damage = 0;
                me->setFaction(35);
                me->CombatStop();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);

                if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
                {
                    player->KilledMonsterCredit(54734, ObjectGuid::Empty);
                    player->RemoveAurasDueToSpell(108150);
                }
                if (Creature* owner = me->GetMap()->GetCreature(me->ToTempSummon()->GetSummonerGUID()))
                    sCreatureTextMgr->SendChat(owner, TEXT_GENERIC_0, attacker->GetGUID());
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            {
                victim->ToPlayer()->SetQuestStatus(QUEST_PARCHEMIN_VOLANT, QUEST_STATUS_FAILED);

                if (victim->GetGUID() == playerGuid)
                    me->DespawnOrUnsummon(3000);
            }
        }
        
        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_CHECK_PLAYER:
                    {
                        bool checkPassed = true;
                        Player* player = ObjectAccessor::FindPlayer(playerGuid);

                        if (!player || !player->isAlive())
                        {
                            me->DespawnOrUnsummon(1000);
                            playerGuid.Clear();
                            break;
                        }

                        if (player->GetQuestStatus(QUEST_PARCHEMIN_VOLANT) != QUEST_STATUS_INCOMPLETE)
                        {
                            me->DespawnOrUnsummon(1000);
                            playerGuid.Clear();
                            break;
                        }

                        events.RescheduleEvent(EVENT_CHECK_PLAYER, 2500);
                        break;
                    }
                    case EVENT_FEET_OF_FURY:
                        if(me->getVictim())
                            me->CastSpell(me->getVictim(), 108958);

                        events.RescheduleEvent(EVENT_FEET_OF_FURY, 5000);
                        break;
                    case EVENT_SHADOW_KICK:
                        if(me->getVictim())
                            me->CastSpell(me->getVictim(), 108936);

                        events.RescheduleEvent(EVENT_SHADOW_KICK_STUN, 2500);
                        events.RescheduleEvent(EVENT_SHADOW_KICK, 30000);
                        break;
                    case 4:
                        if(me->getVictim())
                            me->CastSpell(me->getVictim(), 108944);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };
};

class AreaTrigger_at_temple_entrance : public AreaTriggerScript
{
    public:
        AreaTrigger_at_temple_entrance() : AreaTriggerScript("AreaTrigger_at_temple_entrance")
        {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger, bool enter)
        {
            if (player->GetQuestStatus(QUEST_PASSION_OF_SHEN) == QUEST_STATUS_INCOMPLETE)
            {
                player->KilledMonsterCredit(61128, ObjectGuid::Empty);

                std::list<Creature*> huoList;
                GetCreatureListWithEntryInGrid(huoList, player, 54958, 20.0f);

                for (std::list<Creature*>::const_iterator itr = huoList.begin(); itr != huoList.end(); ++itr)
                {
                    Creature* huo = *itr;
                    if (huo->ToTempSummon())
                    {
                        if (huo->ToTempSummon()->GetOwnerGUID() == player->GetGUID())
                        {
                            huo->GetMotionMaster()->Clear();
                            huo->GetMotionMaster()->MovePoint(1, 950.0f, 3601.0f, 203.0f);
                            huo->DespawnOrUnsummon(5000);
                        }
                    }
                }
            }

            return true;
        }
};

/*
========================================
========= E A S T  P A R T =============
========================================
*/

class at_going_to_east : public AreaTriggerScript
{
    public:
        at_going_to_east() : AreaTriggerScript("at_going_to_east")
        {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger, bool enter)
        {
            if (player->HasAura(SPELL_CSA_AT_TIMER) || player->ToPlayer()->GetQuestStatus(QUEST_SINGING_POOLS) != QUEST_STATUS_COMPLETE) 
                return true;

            player->CastSpell(player, SPELL_CSA_AT_TIMER, true);
            //player->CastSpell(player, SPELL_SUMMON_CHILDREN, true);
            
            if (Creature *cai = player->SummonCreature(NPC_EAST_CHILDREN_CAI, 934.0156f, 3513.154f, 188.1347f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
            {
                cai->AI()->SetGUID(player->GetGUID(), 0);
                cai->GetMotionMaster()->MoveFollow(player, 2.0f, M_PI / 4);
            }
            if (Creature *cai = player->SummonCreature(NPC_EAST_CHILDREN_DEN, 949.37f, 3510.0f, 187.7983f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
            {
                cai->AI()->SetGUID(player->GetGUID(), 0);
                cai->GetMotionMaster()->MoveFollow(player, 2.0f, M_PI / 2);
            }
            return true;
        }
};

class npc_childrens_going_to_east : public CreatureScript
{
public:
    npc_childrens_going_to_east() : CreatureScript("npc_childrens_going_to_east") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_childrens_going_to_eastAI(creature);
    }
    
    struct npc_childrens_going_to_eastAI : public ScriptedAI
    {
        npc_childrens_going_to_eastAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        enum eEvents
        {
            EVENT_1   = 1,
            EVENT_2   = 2,
            EVENT_3   = 3,
            EVENT_DESPOWN   = 4,
        };
        
        EventMap events;
        ObjectGuid plrGUID;

        void SetGUID(ObjectGuid const& guid, int32 /*id*/ = 0)
        {
            plrGUID = guid;
        }

        void Reset()
        {
            plrGUID.Clear();
            events.RescheduleEvent(EVENT_DESPOWN, 60000);

            if (me->GetEntry() == NPC_EAST_CHILDREN_CAI)
            {
                events.RescheduleEvent(EVENT_1, 1000);
                events.RescheduleEvent(EVENT_2, 25000);
                events.RescheduleEvent(EVENT_3, 50000);
            }else
            {
                events.RescheduleEvent(EVENT_1, 15000);
                events.RescheduleEvent(EVENT_2, 40000);
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
                    case EVENT_2:
                    case EVENT_3:
                        sCreatureTextMgr->SendChat(me, eventId - 1, plrGUID);
                        break;
                    case EVENT_DESPOWN:
                        sCreatureTextMgr->SendChat(me, me->GetEntry() == NPC_EAST_CHILDREN_CAI ? TEXT_GENERIC_3 : TEXT_GENERIC_2, plrGUID);
                        me->DespawnOrUnsummon(5000);
                        break;
                    default:
                        break;
                }
            }
        }
    };
};

class checkArea : public BasicEvent
{
    Player* player;
    uint32 spellId;
public:
    explicit checkArea(Player * me, uint32 sp) : player(me), spellId(sp) {}

    virtual bool Execute(uint64 , uint32)
    {
        LiquidData liquidStatus;
        ZLiquidStatus status = player->GetMap()->getLiquidStatus(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), MAP_ALL_LIQUIDS, &liquidStatus);
        if (!player->IsOnVehicle() && status == LIQUID_MAP_IN_WATER)
            player->AddAura(spellId, player);
        return true;
    }
};

class AreaTrigger_at_bassin_curse : public AreaTriggerScript
{
    public:
        AreaTrigger_at_bassin_curse() : AreaTriggerScript("AreaTrigger_at_bassin_curse")
        {}

        enum eTriggers
        {
            AREA_CRANE              = 6991,
            AREA_SKUNK              = 6988,
            AREA_FROG2              = 6986,
            AREA_FROG               = 6987,
            AREA_FROG_EXIT          = 6986,
            AREA_TURTLE             = 7012,
            AREA_CROCODILE          = 6990
        };

        enum eSpells
        {
            SPELL_FROG              = 102938,
            SPELL_SKUNK             = 102939,
            SPELL_TURTLE            = 102940,
            SPELL_CRANE             = 102941,
            SPELL_CROCODILE         = 102942,
        };

        void AddOrRemoveSpell(Player* player, uint32 spellId, bool enter)
        {
            RemoveAllSpellsExcept(player, spellId);

            if (!player->HasAura(spellId))
            {
                player->m_Events.AddEvent(new checkArea(player, spellId), player->m_Events.CalculateTime(1000));
            }
            else if (!enter)
                player->RemoveAurasDueToSpell(spellId);
        }

        void RemoveAllSpellsExcept(Player* player, uint32 spellId)
        {
            uint32 spellTable[5] = {SPELL_FROG, SPELL_SKUNK, SPELL_TURTLE, SPELL_CRANE, SPELL_CROCODILE};

            for (uint8 i = 0; i < 5; ++i)
                if (spellId != spellTable[i])
                    player->RemoveAurasDueToSpell(spellTable[i]);
        }

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger, bool enter)
        {
            //if (player->IsOnVehicle())
            //    return true;

            switch(trigger->ID)
            {
                case AREA_CRANE:     AddOrRemoveSpell(player, SPELL_CRANE, enter);     break;
                case AREA_SKUNK:     AddOrRemoveSpell(player, SPELL_SKUNK, enter);     break;
                case AREA_FROG: case AREA_FROG_EXIT: AddOrRemoveSpell(player, SPELL_FROG, enter);      break;
                //case AREA_FROG_EXIT: RemoveAllSpellsExcept(player, 0);          break;
                case AREA_TURTLE:    AddOrRemoveSpell(player, SPELL_TURTLE, enter);    break;
                case AREA_CROCODILE: AddOrRemoveSpell(player, SPELL_CROCODILE, enter); break;
            }

            return true;
        }
};

// Npc's : 54993 - 55083 - 57431
class vehicle_balance_pole : public VehicleScript
{
    public:
        vehicle_balance_pole() : VehicleScript("vehicle_balance_pole") {}

        void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 /*seatId*/)
        {
            if (passenger->HasAura(102938))
                passenger->ExitVehicle();
        }

        void OnRemovePassenger(Vehicle* veh, Unit* passenger)
        {
            Player* p = passenger->ToPlayer();
            if (!p || !p->GetLastAreaTrigger())
                return;

            sScriptMgr->OnAreaTrigger(p, p->GetLastAreaTrigger(), true);
            //passenger->AddAura(102938, passenger);
        }
};

/*
blizz create random pole... one model set invisible. this enable all visible.
UPDATE `creature_template` SET `modelid2` = '0' WHERE `entry` in (54993, 55083, 57431);
UPDATE `creature_template` SET `modelid1` = '38347', `modelid2` = '0' WHERE `entry` = 57626;
*/
class mob_tushui_monk : public CreatureScript
{
public:
    mob_tushui_monk() : CreatureScript("mob_tushui_monk") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_tushui_monkAI(creature);
    }

    struct mob_tushui_monkAI : public ScriptedAI
    {
        mob_tushui_monkAI(Creature* creature) : ScriptedAI(creature)
        {
            me->setFaction(2357);
        }

        enum data
        {
            NPC_VEH         =   54993,
        };

        ObjectGuid vehGUID;
        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(1, 1000);
            vehGUID.Clear();
        }

        void JustDied(Unit* /*killer*/)
        {
            me->ExitVehicle();
            me->DespawnOrUnsummon(1000);
            if (Creature *v = me->GetMap()->GetCreature(vehGUID))
                v->DespawnOrUnsummon(100);
        }

        void UpdateAI(uint32 diff)
        {
            //UpdateVictim();

            events.Update(diff);

            while (events.ExecuteEvent())
            {
                if (me->GetVehicle())
                    return;

                me->StopMoving();

                if (Creature * c = me->GetMap()->SummonCreature(NPC_VEH, *me, NULL, 0, NULL, ObjectGuid::Empty, 0, 1749))
                {
                    c->SetDisplayId(39004);
                    me->EnterVehicle(c);
                    vehGUID = c->GetGUID();
                }else
                    events.RescheduleEvent(1, 1000);
            }
            DoMeleeAttackIfReady();
        }
    };
};

class mob_jojo_ironbrow_1 : public CreatureScript
{
public:
    mob_jojo_ironbrow_1() : CreatureScript("mob_jojo_ironbrow_1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_jojo_ironbrow_1_AI (creature);
    }

    struct mob_jojo_ironbrow_1_AI : public ScriptedAI
    {
        mob_jojo_ironbrow_1_AI(Creature* creature) : ScriptedAI(creature) {}

        enum eEvents
        {
            EVENT_1    = 1,
            EVENT_2    = 2,
            EVENT_3    = 3,
            EVENT_4    = 4,
        };

        enum eSpell
        {
            NPC_TARGET          = 57636,
            SUPER_DUPER_KULAK   = 129272,
        };

        EventMap events;

        void Reset()
        {
            me->SetWalk(true);
            events.RescheduleEvent(EVENT_1, 1000);
        }

        void MovementInform(uint32 moveType, uint32 pointId)
        {
            if (pointId == EVENT_4)
                me->DespawnOrUnsummon(1000);
            else
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_1:
                        me->GetMotionMaster()->MovePoint(EVENT_1, 1039.0f, 3284.0f, 126.6f);
                        events.RescheduleEvent(EVENT_2, 10000);
                        break;
                    case EVENT_2:
                        if (Creature* target = me->FindNearestCreature(NPC_TARGET, 50.0f, true))
                            me->CastSpell(target, SUPER_DUPER_KULAK, true);
                        events.RescheduleEvent(EVENT_3, 3000);
                        break;
                    case EVENT_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        events.RescheduleEvent(EVENT_4, 10000);
                        break;
                    case EVENT_4:
                        me->GetMotionMaster()->MovePoint(EVENT_4, 1027.379f, 3287.417f, 126.2935f);
                        break;
                    default:
                        break;
                }
            }
        }
    };
};

// Rock Jump - 103069 / 103070 / 103077
class spell_rock_jump: public SpellScriptLoader
{
    public:
        spell_rock_jump() : SpellScriptLoader("spell_rock_jump") { }

        class spell_rock_jump_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rock_jump_SpellScript);

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetPositionZ() < 90.0f)
                        caster->GetMotionMaster()->MoveJump(1045.36f, 2848.47f, 91.38f, 10.0f, 10.0f);
                    else if (caster->GetPositionZ() < 92.0f)
                        caster->GetMotionMaster()->MoveJump(1054.42f, 2842.65f, 92.96f, 10.0f, 10.0f);
                    else if (caster->GetPositionZ() < 94.0f)
                        caster->GetMotionMaster()->MoveJump(1063.66f, 2843.49f, 95.50f, 10.0f, 10.0f);
                    else
                    {
                        caster->GetMotionMaster()->MoveJump(1078.42f, 2845.07f, 95.16f, 10.0f, 10.0f);

                        if (caster->ToPlayer())
                            caster->ToPlayer()->KilledMonsterCredit(57476);
                    }
                }
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_rock_jump_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rock_jump_SpellScript();
        }
};

Position rocksPos[4] =
{
    {1102.05f, 2882.11f, 94.32f, 0.11f},
    {1120.01f, 2883.20f, 96.44f, 4.17f},
    {1128.09f, 2859.44f, 97.64f, 2.51f},
    {1111.52f, 2849.84f, 94.84f, 1.94f}
};

class mob_shu_water_spirit : public CreatureScript
{
public:
    mob_shu_water_spirit() : CreatureScript("mob_shu_water_spirit") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_shu_water_spiritAI(creature);
    }

    struct mob_shu_water_spiritAI : public ScriptedAI
    {
        mob_shu_water_spiritAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap _events;
        uint8 actualPlace;

        ObjectGuid waterSpoutGUID;

        enum eShuSpells
        {
            SPELL_WATER_SPOUT_SUMMON    = 116810,
            SPELL_WATER_SPOUT_WARNING   = 116695,
            SPELL_WATER_SPOUT_EJECT     = 116696,
            SPELL_WATER_SPOUT_VISUAL    = 117057,
        };

        enum eEvents
        {
            EVENT_CHANGE_PLACE          = 1,
            EVENT_SUMMON_WATER_SPOUT    = 2,
            EVENT_WATER_SPOUT_VISUAL    = 3,
            EVENT_WATER_SPOUT_EJECT     = 4,
            EVENT_WATER_SPOUT_DESPAWN   = 5,
        };

        void Reset()
        {
            _events.Reset();
            actualPlace = 0;
            waterSpoutGUID.Clear();

            _events.RescheduleEvent(EVENT_CHANGE_PLACE, 5000);
        }

        void MovementInform(uint32 typeId, uint32 pointId) override
        {
            if (typeId != EFFECT_MOTION_TYPE)
                return;

            if (pointId == 1)
            {
                me->RemoveAurasDueToSpell(SPELL_WATER_SPOUT_WARNING);
                if (Player* player = me->SelectNearestPlayerNotGM(50.0f))
                {
                    me->SetOrientation(me->GetAngle(player));
                    me->SetFacingToObject(player);
                    _events.RescheduleEvent(EVENT_SUMMON_WATER_SPOUT, 2000);
                }
                else
                    _events.RescheduleEvent(EVENT_CHANGE_PLACE, 5000);
            }
        }

        Creature* getWaterSpout()
        {
            return me->GetMap()->GetCreature(waterSpoutGUID);
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
                case EVENT_CHANGE_PLACE:
                {
                    uint8 newPlace = 0;

                    do { newPlace = urand(0, 3); } while (newPlace == actualPlace);

                    me->GetMotionMaster()->MoveJump(rocksPos[newPlace].GetPositionX(), rocksPos[newPlace].GetPositionY(), rocksPos[newPlace].GetPositionZ(), 10.0f, 10.0f, 1);
                    me->AddAura(SPELL_WATER_SPOUT_WARNING, me); // Just visual
                    actualPlace = newPlace;
                    break;
                }
                case EVENT_SUMMON_WATER_SPOUT:
                {
                    float x = 0.0f, y = 0.0f;
                    GetPositionWithDistInOrientation(me, 5.0f, me->GetOrientation() + frand(-M_PI, M_PI), x, y);
                    waterSpoutGUID.Clear();

                    if (Creature* waterSpout = me->SummonCreature(60488, x, y, 92.189629f))
                        waterSpoutGUID = waterSpout->GetGUID();

                    _events.RescheduleEvent(EVENT_WATER_SPOUT_VISUAL, 500);
                    _events.RescheduleEvent(EVENT_WATER_SPOUT_EJECT, 7500);
                    break;
                }
                case EVENT_WATER_SPOUT_VISUAL:
                {
                    if (Creature* waterSpout = getWaterSpout())
                        waterSpout->CastSpell(waterSpout, SPELL_WATER_SPOUT_WARNING, true);
                    break;
                }
                case EVENT_WATER_SPOUT_EJECT:
                {
                    if (Creature* waterSpout = getWaterSpout())
                    {
                        std::list<Player*> playerList;
                        GetPlayerListInGrid(playerList, waterSpout, 1.0f);

                        for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                            (*itr)->CastSpell((*itr), SPELL_WATER_SPOUT_EJECT, true);

                        waterSpout->CastSpell(waterSpout, SPELL_WATER_SPOUT_VISUAL, true);
                    }
                    _events.RescheduleEvent(EVENT_WATER_SPOUT_DESPAWN, 3000);
                    break;
                }
                case EVENT_WATER_SPOUT_DESPAWN:
                {
                    if (Creature* waterSpout = getWaterSpout())
                        waterSpout->DespawnOrUnsummon();

                    waterSpoutGUID.Clear();

                    _events.RescheduleEvent(EVENT_CHANGE_PLACE, 2000);
                    break;
                }
            }
        }
    };
};

// Summon Spirit of Water - 103538
class spell_summon_spirit_of_watter: public SpellScriptLoader
{
    public:
        spell_summon_spirit_of_watter() : SpellScriptLoader("spell_summon_spirit_of_watter") { }

        class spell_summon_spirit_of_watter_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_summon_spirit_of_watter_AuraScript);
            
            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();

                if (!target)
                    return;

                std::list<Creature*> shuList;
                GetCreatureListWithEntryInGrid(shuList, target, NPC_AYSA_WATTER_OUTRO_EVENT, 20.0f);
                for (std::list<Creature*>::const_iterator itr = shuList.begin(); itr != shuList.end(); ++itr)
                {
                    (*itr)->AI()->SetGUID(target->GetGUID(), 0);
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();

                if (!target)
                    return;

            }

            void Register()
            {
                OnEffectApply  += AuraEffectApplyFn (spell_summon_spirit_of_watter_AuraScript::OnApply,  EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectRemove += AuraEffectRemoveFn(spell_summon_spirit_of_watter_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_summon_spirit_of_watter_AuraScript();
        }
};

class mob_aysa_cloudsinger_watter_outro : public CreatureScript
{
public:
    mob_aysa_cloudsinger_watter_outro() : CreatureScript("mob_aysa_cloudsinger_watter_outro") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysa_cloudsinger_watter_outroAI(creature);
    }

    struct mob_aysa_cloudsinger_watter_outroAI : public ScriptedAI
    {
        mob_aysa_cloudsinger_watter_outroAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap _events;
        ObjectGuid plrGUID;
        enum eEvents
        {
            EVENT_1          = 1,
            EVENT_2          = 2,

        };

        void Reset()
        {
            plrGUID.Clear();
            _events.Reset();
        }

        void SetGUID(ObjectGuid const& guid, int32 /*id*/ = 0)
        {
            plrGUID = guid;
            _events.RescheduleEvent(EVENT_1, 1000);
            _events.RescheduleEvent(EVENT_2, 10000);
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
                sCreatureTextMgr->SendChat(me, eventId - 1, plrGUID);
        }
    };
};

// Grab Carriage - 115904
class spell_grab_carriage: public SpellScriptLoader
{
    public:
        spell_grab_carriage() : SpellScriptLoader("spell_grab_carriage") { }

        enum misc
        {
            _credit = 57710,    //Q: 29680
            _credit2 = 59497,   //Q: 59497
            _credit3 = 57741,   //Q: 29800
        };

        class spell_grab_carriage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_grab_carriage_SpellScript);

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();

                if (!caster)
                    return;

                Creature* carriage = NULL;
                Creature* yak      = NULL;
                
                if (caster->GetAreaId() == 5826) // Bassins chantants
                {
                    carriage = caster->SummonCreature(57208, 979.06f, 2863.87f, 87.88f, 4.7822f, TEMPSUMMON_MANUAL_DESPAWN, 0, caster->GetGUID());
                    yak      = caster->SummonCreature(57207, 979.37f, 2860.29f, 88.22f, 4.4759f, TEMPSUMMON_MANUAL_DESPAWN, 0, caster->GetGUID());
                    if (Player* p = caster->ToPlayer())
                        p->TalkedToCreature(_credit, ObjectGuid::Empty);
                }
                else if (caster->GetAreaId() == 5881) // Ferme Dai-Lo
                {
                    carriage = caster->SummonCreature(57208, 588.70f, 3165.63f, 88.86f, 4.4156f, TEMPSUMMON_MANUAL_DESPAWN, 0, caster->GetGUID());
                    yak      = caster->SummonCreature(59499, 587.61f, 3161.91f, 89.31f, 4.3633f, TEMPSUMMON_MANUAL_DESPAWN, 0, caster->GetGUID());
                    if (Player* p = caster->ToPlayer())
                        p->TalkedToCreature(_credit2, ObjectGuid::Empty);
                }
                else if (caster->GetAreaId() == 5833) // Epave du Chercheciel
                {
                    carriage = caster->SummonCreature(57208, 264.37f, 3867.60f, 73.56f, 0.9948f, TEMPSUMMON_MANUAL_DESPAWN, 0, caster->GetGUID());
                    // spell 108932
                    yak      = caster->SummonCreature(57742, 268.38f, 3872.36f, 74.50f, 0.8245f, TEMPSUMMON_MANUAL_DESPAWN, 0, caster->GetGUID());
                    if (Player* p = caster->ToPlayer())
                        p->TalkedToCreature(_credit3, ObjectGuid::Empty);
                }

                if (!carriage || !yak)
                    return;

                yak->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                carriage->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                yak->SetReactState(REACT_PASSIVE);
                carriage->SetReactState(REACT_PASSIVE);


                yak->AI()->SetGUID(carriage->GetGUID(), 0); // enable following
                caster->EnterVehicle(carriage, 0);
                caster->RemoveAllMinionsByFilter(55213);
            }

            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_grab_carriage_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_grab_carriage_SpellScript();
        }
};

// Npc's : 57208
class vehicle_carriage : public VehicleScript
{
    public:
        vehicle_carriage() : VehicleScript("vehicle_carriage") {}
        void OnRemovePassenger(Vehicle* veh, Unit* passenger)
        {
            if(Unit* u = veh->GetBase())
                if (Creature * c = u->ToCreature())
                    c->DespawnOrUnsummon(1000);
        }
};

class npc_nourished_yak : public CreatureScript
{
public:
    npc_nourished_yak() : CreatureScript("npc_nourished_yak") { }

    struct npc_nourished_yakAI : public npc_escortAI
    {        
        npc_nourished_yakAI(Creature* creature) : npc_escortAI(creature)
        {}

        uint32 IntroTimer;

        void Reset()
        {
            if (me->isSummon())
            {
                IntroTimer = 2500;
            }
            else
                IntroTimer = 0;
        }

        void SetGUID(ObjectGuid const& guid, int32 /*id*/ = 0)
        {
            SetFollowerGUID(guid);
        }

        void WaypointReached(uint32 waypointId)
        {

        }

        void UpdateAI(uint32 diff)
        {
            if (IntroTimer)
            {
                if (IntroTimer <= diff)
                {
                    Start(false, true);
                    IntroTimer = 0;
                }
                else
                    IntroTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_nourished_yakAI(creature);
    }
    
};

class mob_jojo_ironbrow_2 : public CreatureScript
{
public:
    mob_jojo_ironbrow_2() : CreatureScript("mob_jojo_ironbrow_2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_jojo_ironbrow_2_AI (creature);
    }

    struct mob_jojo_ironbrow_2_AI : public ScriptedAI
    {
        mob_jojo_ironbrow_2_AI(Creature* creature) : ScriptedAI(creature) {}

        enum eEvents
        {
            EVENT_1    = 1,
            EVENT_2    = 2,
            EVENT_3    = 3,
            EVENT_4    = 4,
        };

        enum eSpell
        {
            NPC_TARGET          = 57667,
            SUPER_DUPER_KULAK   = 129293,
        };

        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_1, 1000);
            me->SetWalk(true);
        }

        void MovementInform(uint32 moveType, uint32 pointId)
        {
            if (pointId == EVENT_4)
                me->DespawnOrUnsummon(1000);
            else
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_1:
                        me->GetMotionMaster()->MovePoint(EVENT_1, 599.215f, 3132.27f, 89.06574f);
                        events.RescheduleEvent(EVENT_2, 10000);
                        break;
                    case EVENT_2:
                        if (Creature* target = me->FindNearestCreature(NPC_TARGET, 50.0f, true))
                            me->CastSpell(target, SUPER_DUPER_KULAK, true);
                        events.RescheduleEvent(EVENT_3, 3000);
                        break;
                    case EVENT_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        events.RescheduleEvent(EVENT_4, 10000);
                        break;
                    case EVENT_4:
                        me->GetMotionMaster()->MovePoint(EVENT_4, 568.2413f, 3155.377f, 84.04771f);
                        break;
                    default:
                        break;
                }
            }
        }
    };
};

class npc_water_spirit_dailo : public CreatureScript
{
public:
    npc_water_spirit_dailo() : CreatureScript("npc_water_spirit_dailo") { }

    enum Credit
    {
        CREDIT_1    = 55548,
        CREDIT_2    = 55547,
    };
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == 1)
        {
            player->CLOSE_GOSSIP_MENU();
            if (player->GetQuestStatus(QUEST_NOT_IN_FACE) == QUEST_STATUS_INCOMPLETE)
            {
                player->KilledMonsterCredit(CREDIT_1);
                creature->AI()->SetGUID(player->GetGUID());
            }
        }

        return true;
    }

    struct npc_water_spirit_dailoAI : public ScriptedAI
    {
        npc_water_spirit_dailoAI(Creature* creature) : ScriptedAI(creature)
        {}

        ObjectGuid playerGuid;
        uint16 eventTimer;
        uint8  eventProgress;

        void Reset()
        {
            eventTimer = 0;
            eventProgress = 0;
            playerGuid.Clear();
        }

        void SetGUID(ObjectGuid const& guid, int32 /*type*/)
        {
            playerGuid = guid;
            eventTimer = 2500;
        }

        void MovementInform(uint32 typeId, uint32 pointId)
        {
            if (typeId != POINT_MOTION_TYPE)
                return;

            switch (pointId)
            {
                case 1:
                    eventTimer = 250;
                    ++eventProgress;
                    break;
                case 2:
                    eventTimer = 250;
                    ++eventProgress;
                    break;
                case 3:
                    if (Creature* wugou = GetClosestCreatureWithEntry(me, 60916, 20.0f))
                        me->SetFacingToObject(wugou);
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY_UNARMED);
                    eventTimer = 2000;
                    ++eventProgress;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (eventTimer)
            {
                if (eventTimer <= diff)
                {
                    switch (eventProgress)
                    {
                        case 0:
                            me->GetMotionMaster()->MovePoint(1, 650.30f, 3127.16f, 89.62f);
                            eventTimer = 0;
                            break;
                        case 1:
                            me->GetMotionMaster()->MovePoint(2, 625.25f, 3127.88f, 87.95f);
                            eventTimer = 0;
                            break;
                        case 2:
                            me->GetMotionMaster()->MovePoint(3, 624.44f, 3142.94f, 87.75f);
                            eventTimer = 0;
                            break;
                        case 3:
                            if (Creature* wugou = GetClosestCreatureWithEntry(me, 60916, 20.0f))
                                wugou->CastSpell(wugou, 118027, false);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_ONESHOT_NONE);
                            eventTimer = 3000;
                            ++eventProgress;
                            break;
                        case 4:
                            eventTimer = 0;
                            if (Player* owner = ObjectAccessor::FindPlayer(playerGuid))
                            {
                                owner->KilledMonsterCredit(CREDIT_2);  //hack it already done in credit spell, but for cust it need already finished quest.
                                owner->CastSpell(owner, SPELL_CREDIT_NOT_IN_FACE, false);
                                me->GetMotionMaster()->MoveTargetedHome();
                                //me->DespawnOrUnsummon(100);
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                    eventTimer -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_water_spirit_dailoAI(creature);
    }
};

class AreaTrigger_at_middle_temple_from_east : public AreaTriggerScript
{
    public:
        AreaTrigger_at_middle_temple_from_east() : AreaTriggerScript("AreaTrigger_at_middle_temple_from_east")
        {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger, bool enter)
        {
            player->RemoveAllMinionsByFilter(60916);
            player->RemoveAllMinionsByFilter(55558);
            return true;
        }
};

/*
========================================
========= W E S T  P A R T =============
========================================
*/

class mob_master_shang_xi_temple : public CreatureScript
{
    public:
        mob_master_shang_xi_temple() : CreatureScript("mob_master_shang_xi_temple") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_master_shang_xi_templeAI(creature);
    }

    struct mob_master_shang_xi_templeAI : public ScriptedAI
    {
        mob_master_shang_xi_templeAI(Creature* creature) : ScriptedAI(creature)
        {}

        ObjectGuid playerGuid;
        GuidSet m_player_for_event;
        EventMap events;

        enum events
        {
            EVENT_XO1    = 1,
            EVENT_CLEAN  = 2,
        };

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() != TYPEID_PLAYER)
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (who->ToPlayer()->GetQuestStatus(QUEST_PASSION_OF_SHEN) == QUEST_STATUS_COMPLETE)
                events.RescheduleEvent(EVENT_XO1, 1000);

            events.RescheduleEvent(EVENT_CLEAN, 60000);
            m_player_for_event.insert(who->GetGUID());
        }

        void Reset()
        {
            playerGuid.Clear();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CLEAN:
                        events.Reset();
                        break;
                    case EVENT_XO1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                        break;
                }
            }
        }
    };

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*opt*/)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_SPIRIT_AND_BODY:
                sCreatureTextMgr->SendChat(creature, TEXT_GENERIC_7);
                player->RemoveAurasDueToSpell(SPELL_CREDIT_NOT_IN_FACE);
                player->RemoveAllMinionsByFilter(118036, 1);
                player->RemoveAllMinionsByFilter(SPELL_CREDIT_NOT_IN_FACE, 1);
                break;
        }
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_SINGING_POOLS:
                sCreatureTextMgr->SendChat(creature, TEXT_GENERIC_1);
                break;
            case QUEST_MORNING_BREEZE_BILLAGE:
            {
                if (quest->GetQuestId() == QUEST_MORNING_BREEZE_BILLAGE) // Brise du matin
                {
                    player->CastSpell(player, SPELL_SUMMON_WIND_TELEPORTER, true);
                }
                break;
            }
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        if (action == 1)
        {
            player->CastSpell(player, SPELL_SUMMON_WIND_TELEPORTER, true);
            //player->NearTeleportTo(926.58f, 3605.33f, 251.63f, 3.114f);
        }

        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }
};

class npc_wind_vehicle : public CreatureScript
{
public:
    npc_wind_vehicle() : CreatureScript("npc_wind_vehicle") { }

    struct npc_wind_vehicleAI : public npc_escortAI
    {        
        npc_wind_vehicleAI(Creature* creature) : npc_escortAI(creature)
        {}

        void Reset()
        {
        }
        
        void OnCharmed(bool /*apply*/)
        {
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
                Start(false, true);

            //if (!apply || who->GetTypeId() != TYPEID_PLAYER)
            //    return;

            // PlayerOn = true;
            // Start(false, true, who->GetGUID());
        }

        void WaypointReached(uint32 waypointId)
        {
            if (waypointId == 16)
            {
                if (me->GetVehicleKit())
                    me->GetVehicleKit()->RemoveAllPassengers();

                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wind_vehicleAI(creature);
    }
};

class npc_panda_history_leason : public CreatureScript
{
    public:
        npc_panda_history_leason() : CreatureScript("npc_panda_history_leason") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_panda_history_leasonAI(creature);
    }
    
    struct npc_panda_history_leasonAI : public ScriptedAI
    {
        npc_panda_history_leasonAI(Creature* creature) : ScriptedAI(creature)
        {

        }

        void Reset()
        {
            text = TEXT_GENERIC_0;
            events.RescheduleEvent(EVENT_1, 10000);
        }

        enum datalocal
        {
            EVENT_1            = 1,
            MAX_TEXT           = 15,

        };

        uint32 text;
        EventMap events;

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                events.RescheduleEvent(EVENT_1, 10000);
                sCreatureTextMgr->SendChat(me, text);
                ++text;
                if (text >= MAX_TEXT)
                    text = TEXT_GENERIC_0;
            }
        }
    };
};

class mob_jojo_ironbrow_3 : public CreatureScript
{
public:
    mob_jojo_ironbrow_3() : CreatureScript("mob_jojo_ironbrow_3") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_jojo_ironbrow_3_AI (creature);
    }

    struct mob_jojo_ironbrow_3_AI : public ScriptedAI
    {
        mob_jojo_ironbrow_3_AI(Creature* creature) : ScriptedAI(creature) {}

        enum eEvents
        {
            EVENT_1    = 1,
            EVENT_2    = 2,
            EVENT_3    = 3,
            EVENT_4    = 4,
        };

        enum eSpell
        {
            NPC_TARGET          = 57668,
            SUPER_DUPER_KULAK   = 129294,
        };

        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_1, 1000);
            me->SetWalk(true);
        }

        void MovementInform(uint32 moveType, uint32 pointId)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_1:
                        me->GetMotionMaster()->MovePoint(EVENT_1, 1077.31f, 4179.94f, 205.7737f);
                        events.RescheduleEvent(EVENT_2, 10000);
                        break;
                    case EVENT_2:
                        if (Creature* target = me->FindNearestCreature(NPC_TARGET, 50.0f, true))
                            me->CastSpell(target, SUPER_DUPER_KULAK, true);
                        events.RescheduleEvent(EVENT_3, 3000);
                        break;
                    case EVENT_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        events.RescheduleEvent(EVENT_4, 10000);
                        break;
                    case EVENT_4:
                        me->DespawnOrUnsummon(10000);
                        break;
                    default:
                        break;
                }
            }
        }
    };
};

class mob_huojin_monk : public CreatureScript
{
public:
    mob_huojin_monk() : CreatureScript("mob_huojin_monk") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_huojin_monk_AI (creature);
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_BALANCED_PERSPECTIVE)
        {
            sCreatureTextMgr->SendChat(creature, TEXT_GENERIC_2);
            creature->GetMotionMaster()->MovePoint(1, 1149.78f, 4550.93f, 223.17f);
            creature->DespawnOrUnsummon(10000);
        }
        return true;
    }

    struct mob_huojin_monk_AI : public ScriptedAI
    {
        mob_huojin_monk_AI(Creature* creature) : ScriptedAI(creature) {}
        
        enum eEvents
        {
            EVENT_KICK              = 1,
            EVENT_FIST_FURY         = 2,
            EVENT_JAP               = 3,
            EVENT_SPINING_KICK      = 4,
            EVENT_CHECK_ZONE        = 5,
        };

        enum eSpell
        {
            SPELL_ROLL          = 117312,
            SPELL_KICK          = 128631,
            SPELL_FIST_FURY     = 128635,
            SPELL_JAP           = 128630,
            SPELL_SPINING_KICK  = 128632,
            SPELL_SUM_HILING_SP = 128643, // use 128641
            AREA                = 5831,
        };

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_KICK, 5000);
            events.RescheduleEvent(EVENT_FIST_FURY, 7000);
            events.RescheduleEvent(EVENT_JAP, 12000);
            events.RescheduleEvent(EVENT_SPINING_KICK, 15000);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void KilledUnit(Unit* /*victim*/)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
        }

        void IsSummonedBy(Unit* summoner)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType)
        {
            damage = 1;
        }

        void EnterCombat(Unit* victim)
        {
            if (me->GetDistance(victim) > 10)
                me->CastSpell(victim, SPELL_ROLL, true);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_KICK:
                        me->CastSpell(me->getVictim(), SPELL_KICK, true);
                        events.RescheduleEvent(EVENT_KICK, 5000);
                        break;
                    case EVENT_FIST_FURY:
                        me->CastSpell(me->getVictim(), SPELL_FIST_FURY, true);
                        events.RescheduleEvent(EVENT_FIST_FURY, 5000);
                        break;
                    case EVENT_JAP:
                        me->CastSpell(me->getVictim(), SPELL_JAP, true);
                        events.RescheduleEvent(EVENT_JAP, 5000);
                        break;
                    case EVENT_SPINING_KICK:
                        me->CastSpell(me->getVictim(), SPELL_SPINING_KICK, true);
                        events.RescheduleEvent(EVENT_SPINING_KICK, 16000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};
// Summon Ji Yuan 105306
class spell_summon_ji_yung: public SpellScriptLoader
{
    public:
        spell_summon_ji_yung() : SpellScriptLoader("spell_summon_ji_yung") { }

        class spell_summon_ji_yung_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_summon_ji_yung_AuraScript);
            
            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();

                if (!target)
                    return;

                target->RemoveAllMinionsByFilter(65558);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_summon_ji_yung_AuraScript::OnRemove, EFFECT_3, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_summon_ji_yung_AuraScript();
        }
};

class mob_jojo_ironbrow_4 : public CreatureScript
{
public:
    mob_jojo_ironbrow_4() : CreatureScript("mob_jojo_ironbrow_4") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_jojo_ironbrow_4_AI (creature);
    }

    struct mob_jojo_ironbrow_4_AI : public ScriptedAI
    {
        mob_jojo_ironbrow_4_AI(Creature* creature) : ScriptedAI(creature) {}

        enum eEvents
        {
            EVENT_1    = 1,
            EVENT_2    = 2,
            EVENT_3    = 3,
            EVENT_4    = 4,
            EVENT_5    = 5,
        };

        enum eSpell
        {
            SUPER_DUPER_KULAK   = 129297,
        };

        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_1, 1000);
            me->SetWalk(true);
        }

        void MovementInform(uint32 moveType, uint32 pointId)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_1:
                        me->GetMotionMaster()->MovePoint(EVENT_1, 1077.31f, 4179.94f, 205.7737f);
                        events.RescheduleEvent(EVENT_2, 5000);
                        break;
                    case EVENT_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        events.RescheduleEvent(EVENT_3, 10000);
                        break;
                    case EVENT_3:
                        me->CastSpell(me, SUPER_DUPER_KULAK, true);
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                        me->PlayOneShotAnimKit(1078);
                        events.RescheduleEvent(EVENT_4, 6000);
                        break;
                    case EVENT_4:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                        me->SetByteValue(UNIT_FIELD_BYTES_1, 0, UNIT_STAND_STATE_SLEEP);
                        me->PlayOneShotAnimKit(0);
                        events.RescheduleEvent(EVENT_5, 15000);
                        break;
                    case EVENT_5:
                        me->DespawnOrUnsummon(30000);
                        break;
                    default:
                        break;
                }
            }
        }
    };
};

class AreaTrigger_at_wind_temple_entrance : public AreaTriggerScript
{
    public:
        AreaTrigger_at_wind_temple_entrance() : AreaTriggerScript("AreaTrigger_at_wind_temple_entrance")
        {}

        enum spell
        {
            SUMMON_SPELL    = 104571,
        };

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger, bool enter)
        {
            if (player->GetQuestStatus(QUST_DAFENG_SPIRIT_OF_AIR) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SUMMON_SPELL))
            {
                player->CastSpell(player, SUMMON_SPELL, true);
                //if (Creature* aysa = player->SummonCreature(55744, 665.60f, 4220.66f, 201.93f, 1.93f, TEMPSUMMON_MANUAL_DESPAWN, 0, player->GetGUID()))
                //    aysa->AI()->SetGUID(player->GetGUID());
            }

            return true;
        }
};

class mob_aysa_wind_temple_escort : public CreatureScript
{
    public:
        mob_aysa_wind_temple_escort() : CreatureScript("mob_aysa_wind_temple_escort") { }

    struct mob_aysa_wind_temple_escortAI : public npc_escortAI
    {        
        mob_aysa_wind_temple_escortAI(Creature* creature) : npc_escortAI(creature)
        {}
        
        ObjectGuid playerGuid;
        EventMap events;

        enum events
        {
            EVENT_1                 = 1,
            EVENT_2                 = 2,
            EVENT_JECK_DEAD         = 3,
        };

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void SetGUID(ObjectGuid const& guid, int32)
        {
            playerGuid = guid;
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            playerGuid = summoner->GetGUID();
            events.RescheduleEvent(EVENT_1, 1000);
        }

        void DoAction(int32 const /*param*/)
        {
            SetEscortPaused(false);
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 2:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGuid);
                    SetEscortPaused(true);
                    me->SetFacingTo(2.38f);
                    break;
                case 7:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGuid);
                    SetEscortPaused(true);
                    // cast 104612 on owner
                    break;
                case 11:
                    //if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
                    //    player->KilledMonsterCredit(55666);
                    me->DespawnOrUnsummon(5000);
                    break;
                default:
                    break;
            }
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
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGuid);
                        events.RescheduleEvent(EVENT_2, 5000);
                        break;
                    case EVENT_2:
                        Start(false, true);
                        break;
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysa_wind_temple_escortAI(creature);
    }
};

class mob_frightened_wind : public CreatureScript
{
public:
    mob_frightened_wind() : CreatureScript("mob_frightened_wind") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_frightened_windAI(creature);
    }

    struct mob_frightened_windAI : public ScriptedAI
    {
        mob_frightened_windAI(Creature* creature) : ScriptedAI(creature)
        {}

        uint32 tornadeTimer;
        ObjectGuid goWinder;
        enum Spells
        {
            SPELL_TORNADE    = 104333,
            SPELL_OFFTIME    = 105678,
            GO_WIND          = 209685,
        };

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            tornadeTimer = 8 * IN_MILLISECONDS;
            if (GameObject* wind =  me->FindNearestGameObject(GO_WIND, 15.0f))
                goWinder = wind->GetGUID();
        }

        void UpdateAI(uint32 diff)
        {
            if (tornadeTimer <= diff)
            {
                me->ToggleAura(SPELL_TORNADE, me);

                bool enable = false;
                if (!me->HasAura(SPELL_TORNADE))
                {
                    me->RemoveAurasDueToSpell(SPELL_OFFTIME);
                    enable = true;
                }else
                {
                    me->CastSpell(me, SPELL_OFFTIME, true);
                    if (Creature* aysa = me->FindNearestCreature(55744, 200.0f, true))
                        aysa->AI()->DoAction(1);
                }

                if (GameObject* g = me->GetMap()->GetGameObject(goWinder))
                {
                    //g->SetGoState(enable ? GO_STATE_ACTIVE : GO_STATE_READY);
                    g->EnableOrDisableGo(!enable);
                }
                tornadeTimer = 8 * IN_MILLISECONDS;
            }
            else
                tornadeTimer -= diff;
        }
    };
};

enum Enums
{
    NPC_ROCKET_LAUNCHER = 64507,
    NPC_AISA_SHAO       = 64506,
    SPELL_ROCKET_LAUNCH = 104855,
            
    EVENT_NEXT_MOVEMENT = 1,
    EVENT_STUNNED       = 2,
    EVENT_LIGHTNING     = 3,

    SPELL_SERPENT_SWEEP = 125990,
    SPELL_STUNNED       = 125992,
    SPELL_LIGHTNING     = 126006,
};

Position ZhaoPos[] = 
{
    {719.36f, 4164.60f, 216.06f}, // Center
    {745.91f, 4154.35f, 223.48f},
    {717.04f, 4141.16f, 219.83f},
    {689.62f, 4153.16f, 217.63f},
    {684.53f, 4173.24f, 216.98f},
    {704.77f, 4190.16f, 218.24f},
    {736.90f, 4183.85f, 221.41f}
};

class boss_zhao_ren : public CreatureScript
{
public:
    boss_zhao_ren() : CreatureScript("boss_zhao_ren") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_zhao_renAI(creature);
    }

    struct boss_zhao_renAI : public ScriptedAI
    {
        boss_zhao_renAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap _events;
        bool eventStarted;
        bool falling;
        uint8 hitCount;
        uint8 currentPos;

        void Reset()
        {
            _events.Reset();
            me->SetReactState(REACT_PASSIVE);

            falling = false;
            eventStarted = false;
            hitCount = 0;
            currentPos = 0;

            me->SetFullHealth();
            me->RemoveAurasDueToSpell(SPELL_STUNNED);

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(0, ZhaoPos[0].GetPositionX(), ZhaoPos[0].GetPositionY(), ZhaoPos[0].GetPositionZ());
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_ROCKET_LAUNCH)
            {
                if (++hitCount >= 5)
                {
                    falling = true;
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveFall();
                    _events.RescheduleEvent(EVENT_STUNNED, 12000);
                    hitCount = 0;
                }else if (hitCount == 1)
                {
                    if (Creature* aysa = me->FindNearestCreature(NPC_AISA_SHAO, 100.0f, true))
                        sCreatureTextMgr->SendChat(aysa, TEXT_GENERIC_0);
                }
            }
        }
        
        bool checkPlayers()
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 80.0f);

            for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                Player* player = *itr;
                if (player->GetQuestStatus(QIEST_BATTLE_FOR_SKIES) == QUEST_STATUS_INCOMPLETE)
                    if (player->isAlive())
                        return true;
            }

            return false;
        }

        void GoToNextPos()
        {
            if (++currentPos > 6)
                currentPos = 1;

            me->GetMotionMaster()->MovePoint(currentPos, ZhaoPos[currentPos].GetPositionX(), ZhaoPos[currentPos].GetPositionY(), ZhaoPos[currentPos].GetPositionZ());
        }

        Player* GetRandomPlayer()
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 50.0f);

            if (playerList.empty())
                return NULL;

            Trinity::Containers::RandomResizeList(playerList, 1);

            return *playerList.begin();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (falling)
            {
                me->CastSpell(me, SPELL_STUNNED, true);
                falling = false;
                if (Creature* aysa = me->FindNearestCreature(NPC_AISA_SHAO, 100.0f, true))
                {
                    sCreatureTextMgr->SendChat(aysa, TEXT_GENERIC_1);
                    aysa->AI()->AttackStart(me);
                }
            }

            if (type != POINT_MOTION_TYPE)
                return;

            if (!id)
                return;

            _events.RescheduleEvent(EVENT_NEXT_MOVEMENT, 200);
        }

        void JustDied(Unit* attacker)
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 50.0f);

            for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                Player* player = *itr;
                if (player->GetQuestStatus(QIEST_BATTLE_FOR_SKIES) == QUEST_STATUS_INCOMPLETE)
                    if (player->isAlive())
                        player->KilledMonsterCredit(me->GetEntry());
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (checkPlayers())
            {
                if (!eventStarted)  // Event not started, player found
                {
                    _events.RescheduleEvent(EVENT_NEXT_MOVEMENT, 1000);
                    _events.RescheduleEvent(EVENT_LIGHTNING, 5000);
                    eventStarted = true;
                }
            }
            else
            {
                if (eventStarted)  // Event started, no player found
                    Reset();

                return;
            }

            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
                case EVENT_NEXT_MOVEMENT:
                {
                    if (me->HasAura(SPELL_STUNNED))
                        _events.RescheduleEvent(EVENT_NEXT_MOVEMENT, 2000);

                    GoToNextPos();
                    break;
                }
                case EVENT_STUNNED:
                {
                    me->RemoveAurasDueToSpell(SPELL_STUNNED);
                    me->CastSpell(me, SPELL_SERPENT_SWEEP, false);
                    _events.RescheduleEvent(EVENT_NEXT_MOVEMENT, 3000);
                    break;
                }
                case EVENT_LIGHTNING:
                {
                    if (Player* player = GetRandomPlayer())
                        me->CastSpell(player, SPELL_LIGHTNING, false);

                    _events.RescheduleEvent(EVENT_LIGHTNING, 5000);
                    break;
                }
            }
        }
    };
};

class npc_rocket_launcher : public CreatureScript
{
public:
    npc_rocket_launcher() : CreatureScript("npc_rocket_launcher") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rocket_launcherAI (creature);
    }

    struct npc_rocket_launcherAI : public ScriptedAI
    {
        npc_rocket_launcherAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 cooldown;

        void Reset()
        {
            cooldown = 0;
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void OnSpellClick(Unit* Clicker)
        {
            if (cooldown)
                return;
            if (Creature* zhao = GetClosestCreatureWithEntry(me, 55786, 50.0f))
                Clicker->CastSpell(zhao, SPELL_ROCKET_LAUNCH, false);

            cooldown = 5000;
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void EnterCombat(Unit* /*who*/)
        {
            return;
        }

        void UpdateAI(uint32 diff)
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

class mob_master_shang_xi_after_zhao_escort : public CreatureScript
{
    public:
        mob_master_shang_xi_after_zhao_escort() : CreatureScript("mob_master_shang_xi_after_zhao_escort") { }

    struct mob_master_shang_xi_after_zhao_escortAI : public npc_escortAI
    {        
        mob_master_shang_xi_after_zhao_escortAI(Creature* creature) : npc_escortAI(creature)
        {}
        
        uint32 IntroTimer;

        ObjectGuid playerGuid;

        void Reset()
        {
            IntroTimer = 250;
            me->SetReactState(REACT_PASSIVE);
            me->SetWalk(false);
            //Start(false, true);
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            playerGuid = summoner->GetGUID();
            Start(false, true);
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 7:
                    me->SetWalk(true);
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                    break;
                case 8:
                    if (GameObject* gate =  me->FindNearestGameObject(209922, 200.0f))
                        gate->EnableOrDisableGo(false);
                    me->SummonCreature(56274, 845.89f, 4372.62f, 223.98f, 4.78f, TEMPSUMMON_CORPSE_DESPAWN, 0, playerGuid);
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                    break;
                case 9:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                    break;
                case 11:
                    me->SetFacingTo(0.0f);
                    SetEscortPaused(true);
                    break;
                case 12:
                    me->SetFacingTo(4.537860f);
                    me->DespawnOrUnsummon(1000);
                    break;
                default:
                    break;
            }
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            if (summon->GetEntry() == 56274)
            {
                if (auto owner = me->GetAnyOwner())
                {
                    if (auto plr = owner->ToPlayer())
                        plr->KilledMonsterCredit(56274);
                }
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4);
                if (GameObject* gate =  me->FindNearestGameObject(209922, 200.0f))
                    gate->EnableOrDisableGo(true);
                me->SetWalk(false);
                SetEscortPaused(false);
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_master_shang_xi_after_zhao_escortAI(creature);
    }
};

class mob_master_shang_xi_thousand_staff : public CreatureScript
{
    public:
        mob_master_shang_xi_thousand_staff() : CreatureScript("mob_master_shang_xi_thousand_staff") { }

    struct mob_master_shang_xi_thousand_staffAI : public ScriptedAI
    {        
        mob_master_shang_xi_thousand_staffAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap _events;
        ObjectGuid playerGuid;
        enum events
        {
            EVENT_DESPAWN   = 1,
            EVENT_1         = 2,
            EVENT_2         = 3,
            EVENT_3         = 4,
            EVENT_4         = 5,
            EVENT_5         = 6,
        };

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetWalk(true);
            playerGuid.Clear();
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            playerGuid = summoner->GetGUID();
            
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
            _events.RescheduleEvent(EVENT_1, 12000);
        }

         void MovementInform(uint32 moveType, uint32 waypointId)
        {
            switch (waypointId)
            {
                case 2:
                    _events.RescheduleEvent(EVENT_3, 12000);
                    break;
                default:
                    break;
            }
            if (Player* p = ObjectAccessor::GetPlayer(*me, playerGuid))
                me->SetFacingToObject(p);
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        me->GetMotionMaster()->MovePoint(1, 871.0573f, 4460.548f, 241.4504f);
                        _events.RescheduleEvent(EVENT_2, 14000);
                        break;
                    case EVENT_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                        me->GetMotionMaster()->MovePoint(2, 868.007f, 4464.838f, 241.6647f);
                        break;
                    case EVENT_3:
                        me->GetMotionMaster()->MovePoint(3, 874.205f, 4464.75f, 241.3819f);
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                        _events.RescheduleEvent(EVENT_4, 15000);
                        break;
                    case EVENT_4:
                         me->RemoveAurasDueToSpell(126160);
                         me->CastSpell(me, 128850, true);       //summon stuff
                         sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4);
                         _events.RescheduleEvent(EVENT_5, 15000);
                        break;
                    case EVENT_5:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5);
                        me->SetByteValue(UNIT_FIELD_BYTES_1, 0, UNIT_STAND_STATE_KNEEL);
                        me->CastSpell(me, 128851, true);
                        me->CastSpell(me, 109336, true);
                        me->CastSpell(me, 106625, true);
                        _events.RescheduleEvent(EVENT_DESPAWN, 10000);
                        break;
                    case EVENT_DESPAWN:
                    {
                        me->DespawnOrUnsummon();
                        if (Player* owner = ObjectAccessor::GetPlayer(*me, playerGuid))
                        {
                            owner->CastSpell(owner, 106625, true);
                            //owner->KilledMonsterCredit(56688);
                        }
                        break;
                    }
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_master_shang_xi_thousand_staffAI(creature);
    }
};

class mob_aisa_pre_balon_event : public CreatureScript
{
    public:
        mob_aisa_pre_balon_event() : CreatureScript("mob_aisa_pre_balon_event") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_SUF_SHUN_ZI)
            sCreatureTextMgr->SendChat(creature, TEXT_GENERIC_1, player->GetGUID());

        return true;
    }

    struct mob_aisa_pre_balon_eventAI : public ScriptedAI
    {        
        mob_aisa_pre_balon_eventAI(Creature* creature) : ScriptedAI(creature)
        {}

        bool justSpeaking;
        EventMap _events;
        GuidSet m_player_for_event;

        enum events
        {
            EVENT_1    = 1,
            EVENT_2    = 2,
            EVENT_3    = 3,

            NPC_FRIEND  = 56663,
        };

        void Reset()
        {
            justSpeaking = false;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (justSpeaking || who->GetTypeId() != TYPEID_PLAYER || who->IsOnVehicle())
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (who->ToPlayer()->GetQuestStatus(QUEST_PASSING_WISDOM) != QUEST_STATUS_COMPLETE)
                return;

            m_player_for_event.insert(who->GetGUID());
            justSpeaking = true;
            _events.RescheduleEvent(EVENT_1, 10000);
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, who->GetGUID());
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_1:
                    {
                        _events.RescheduleEvent(EVENT_2, 8000);
                        if (Creature* f = me->FindNearestCreature(NPC_FRIEND, 100.0f, true))
                        {
                            sCreatureTextMgr->SendChat(f, TEXT_GENERIC_0);
                            f->SetFacingToObject(me);
                        }
                        break;
                    }
                    case EVENT_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                        justSpeaking = false;
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aisa_pre_balon_eventAI(creature);
    }
};

class mop_air_balloon : public VehicleScript
{
    public:
        mop_air_balloon() : VehicleScript("mop_air_balloon") { }

    struct mop_air_balloonAI : public npc_escortAI
    {        
        mop_air_balloonAI(Creature* creature) : npc_escortAI(creature)
        {}
        
        ObjectGuid playerGuid;
        ObjectGuid aisaGUID;
        ObjectGuid firepawGUID;
        ObjectGuid shenZiGUID;
        ObjectGuid headGUID;
        EventMap events;

        void Reset()
        {
            me->SetWalk(false);
            //me->SetSpeed(MOVE_FLIGHT, 8.0f, false);

            playerGuid.Clear();
            aisaGUID.Clear();
            firepawGUID.Clear();
            shenZiGUID.Clear();
            headGUID.Clear();
            me->setActive(true);
            me->SetReactState(REACT_PASSIVE);
            me->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK5);
            me->m_invisibilityDetect.AddValue(INVISIBILITY_UNK5, 999);
        }

        enum localdata
        {
            NPC_AISA                          = 56661,
            NPC_FIREPAW                       = 56660,
            NPC_SHEN_ZI_SU                    = 56676,
            NPC_TURTLE_HEAD                   = 57769,

            SPELL_HEAD_ANIM_RISE              = 114888,
            SPELL_HEAD_ANIM_1                 = 114898,
            SPELL_HEAD_ANIM_2                 = 118571,
            SPELL_HEAD_ANIM_3                 = 118572,
            SPELL_VOICE_ANIM                  = 106759,

            SPELL_AISA_ENTER_SEAT_2           = 63313, //106617

            SPELL_CREDIT_1                    = 105895,
            SPELL_CREDIT_2                    = 105010,
            SPELL_EJECT_PASSANGER             = 60603,
            SPELL_PARASHUT                    = 45472,

            EVENT_1                           = 1, // 17:24:47.000

            EVENT_AISA_TALK_0                 = 2,  //17:24:51.000
            EVENT_AISA_TALK_1                 = 3,  //17:25:07.000
            EVENT_AISA_TALK_2                 = 4,  //17:25:18.000
            EVENT_AISA_TALK_3                 = 5,  //17:25:31.000
            EVENT_AISA_TALK_4                 = 6,  //17:25:38.000
            EVENT_AISA_TALK_5                 = 7,  //17:26:40.000
            EVENT_AISA_TALK_6                 = 8,  //17:27:02.000
            EVENT_AISA_TALK_7                 = 9,  //17:27:29.000
            EVENT_AISA_TALK_8                 = 10, //17:27:50.000
            EVENT_AISA_TALK_9                 = 11, //17:28:04.000
            EVENT_AISA_TALK_10                = 12, //17:28:10.000

            EVENT_FIREPAW_TALK_0              = 13, //17:24:47.000
            EVENT_FIREPAW_TALK_1              = 14, //17:24:57.000
            EVENT_FIREPAW_TALK_2              = 15, //17:25:13.000
            EVENT_FIREPAW_TALK_3              = 16, //17:27:16.000
            EVENT_FIREPAW_TALK_4              = 17, //17:27:22.000
            EVENT_FIREPAW_TALK_5              = 18, //17:27:43.000
            EVENT_FIREPAW_TALK_6              = 19, //17:27:57.000

            EVENT_SHEN_ZI_SU_TALK_0           = 20, //17:25:44.000
            EVENT_SHEN_ZI_SU_TALK_1           = 21, //17:25:58.000
            EVENT_SHEN_ZI_SU_TALK_2           = 22, //17:26:12.000
            EVENT_SHEN_ZI_SU_TALK_3           = 23, //17:26:25.000
            EVENT_SHEN_ZI_SU_TALK_4           = 24, //17:26:47.000 
            EVENT_SHEN_ZI_SU_TALK_5           = 25, //17:27:09.000
        };

        void InitTalking(Player* player)
        {
            me->GetMap()->LoadGrid(865.222f, 4986.84f); //voice
            me->GetMap()->LoadGrid(868.356f, 4631.19f); //head
            if (WorldObject* head = me->GetMap()->GetActiveObjectWithEntry(NPC_TURTLE_HEAD))
            {
                head->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK5);
                head->m_invisibilityDetect.AddValue(INVISIBILITY_UNK5, 999);
                player->AddToExtraLook(head->GetGUID());
                headGUID = head->GetGUID();
            }
            else
            {
                me->MonsterSay("SCRIPT::mop_air_balloon not find turtle heat entry 57769", LANG_UNIVERSAL, playerGuid);
                player->ExitVehicle();
                return;
            }

            if (WorldObject* shen = me->GetMap()->GetActiveObjectWithEntry(NPC_SHEN_ZI_SU))
                shenZiGUID = shen->GetGUID();
            else
            {
                me->MonsterSay("SCRIPT::mop_air_balloon not find shen zi su entry 56676", LANG_UNIVERSAL, playerGuid);
                player->ExitVehicle();
                return;
            }

            uint32 t = 3000;
            events.RescheduleEvent(EVENT_FIREPAW_TALK_0, t += 1000);       //17:24:47.000
            events.RescheduleEvent(EVENT_AISA_TALK_0, t += 4000);          //17:24:51.000
            events.RescheduleEvent(EVENT_FIREPAW_TALK_1, t += 6000);       //17:24:57.000
            events.RescheduleEvent(EVENT_AISA_TALK_1, t += 10000);         //17:25:07.000
            events.RescheduleEvent(EVENT_FIREPAW_TALK_2, t += 6000);       //17:25:13.000
            events.RescheduleEvent(EVENT_AISA_TALK_2, t += 5000);          //17:25:18.000
            events.RescheduleEvent(EVENT_AISA_TALK_3, t += 14000);         //17:25:31.000
            events.RescheduleEvent(EVENT_AISA_TALK_4, t += 6000);          //17:25:38.000
            events.RescheduleEvent(EVENT_SHEN_ZI_SU_TALK_0, t += 6000);    //17:25:44.000
            events.RescheduleEvent(EVENT_SHEN_ZI_SU_TALK_1, t += 14000);   //17:25:58.000
            events.RescheduleEvent(EVENT_SHEN_ZI_SU_TALK_2, t += 14000);   //17:26:12.000
            events.RescheduleEvent(EVENT_SHEN_ZI_SU_TALK_3, t += 13000);   //17:26:25.000
            events.RescheduleEvent(EVENT_AISA_TALK_5, t += 15000);         //17:26:40.000
            events.RescheduleEvent(EVENT_SHEN_ZI_SU_TALK_4, t +=7000);     //17:26:47.000 
            events.RescheduleEvent(EVENT_AISA_TALK_6, t += 15000);         //17:27:02.000
            events.RescheduleEvent(EVENT_SHEN_ZI_SU_TALK_5, t +=7000);     //17:27:09.000
            events.RescheduleEvent(EVENT_FIREPAW_TALK_3, t += 7000);       //17:27:16.000
            events.RescheduleEvent(EVENT_FIREPAW_TALK_4, t += 6000);       //17:27:22.000
            events.RescheduleEvent(EVENT_AISA_TALK_7, t += 7000);          //17:27:29.000
            events.RescheduleEvent(EVENT_FIREPAW_TALK_5, t += 14000);      //17:27:43.000
            events.RescheduleEvent(EVENT_AISA_TALK_8, t += 7000);          //17:27:50.000
            events.RescheduleEvent(EVENT_FIREPAW_TALK_6, t += 7000);       //17:27:57.000
            events.RescheduleEvent(EVENT_AISA_TALK_9, t += 7000);          //17:28:04.000
            events.RescheduleEvent(EVENT_AISA_TALK_10, t += 7000);         //17:28:10.000
        };

        void TalkShenZiSU(uint32 text)
        {
            Creature *shen = me->GetMap()->GetCreature(shenZiGUID);

            if (!shen)
                return;

            if (Player* plr = sObjectAccessor->FindPlayer(playerGuid))
            {
                Creature *head = me->GetMap()->GetCreature(headGUID);
                if (!head)
                    return;

                switch(text)
                {
                    //cast 114888                                   //17:25:31.000
                    case TEXT_GENERIC_0:                            //17:25:44.000
                        plr->CastSpell(shen, SPELL_HEAD_ANIM_1, false);     
                        break;
                    case TEXT_GENERIC_1:                            //17:25:58.000
                        plr->CastSpell(shen, SPELL_VOICE_ANIM, false);
                        break;
                    case TEXT_GENERIC_2:                            //17:26:11.000
                    case TEXT_GENERIC_3:                            //17:26:25.000
                    case TEXT_GENERIC_5:                            //17:27:08.000
                        plr->CastSpell(shen, SPELL_HEAD_ANIM_2, false);
                        break;
                    case TEXT_GENERIC_4:                            //17:26:47.000
                        plr->CastSpell(shen, SPELL_HEAD_ANIM_3, false);
                        break;
                }
                if (text == TEXT_GENERIC_5) // restore emote
                {
                    head->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, ANIM_FLY_LAND);   //hack
                    plr->RemoveFromExtraLook(head->GetGUID());
                }
            }
            sCreatureTextMgr->SendChat(shen, text, playerGuid, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_AREA);
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool apply)
        {
            if (!apply)
            {
                if (passenger->GetTypeId() == TYPEID_PLAYER)
                {
                    me->DespawnOrUnsummon(1000);
                    me->CastSpell(passenger, SPELL_PARASHUT, true);
                }
                else
                    passenger->ToCreature()->DespawnOrUnsummon(1000);
                return;
            }

            if (seatId == 0)
            {
                if (Player* player = passenger->ToPlayer())
                {
                    playerGuid = player->GetGUID();
                    me->CastSpell(player, SPELL_CREDIT_1, true);
                    InitTalking(player);
                }
            }

            if (passenger->GetTypeId() != TYPEID_PLAYER)
            {
                passenger->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK5);
                passenger->m_invisibilityDetect.AddValue(INVISIBILITY_UNK5, 999);
                switch(passenger->GetEntry())
                {
                    case NPC_AISA: aisaGUID = passenger->GetGUID(); break;
                    case NPC_FIREPAW: firepawGUID = passenger->GetGUID(); break;
                    default:
                        break;
                }
            }
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            playerGuid = summoner->GetGUID();
            summoner->EnterVehicle(me, 0);
            events.RescheduleEvent(EVENT_1, 1000);
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 8:
                    me->SetSpeed(MOVE_FLIGHT, 3.0f, false);
                    break;
                case 15:
                {
                    if (Player* plr = sObjectAccessor->FindPlayer(playerGuid))
                        me->CastSpell(plr, SPELL_CREDIT_2, true);
                    break;
                }
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            npc_escortAI::UpdateAI(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_1:
                    {
                        if (Creature* f = me->FindNearestCreature(NPC_AISA, 100.0f, true))
                            f->CastSpell(me, SPELL_AISA_ENTER_SEAT_2, true);
                        Start(false, true);
                        break;
                    }
                    case EVENT_AISA_TALK_3:
                        if (Creature *head = me->GetMap()->GetCreature(headGUID))
                            if (Player* plr = sObjectAccessor->FindPlayer(playerGuid))
                            {
                                plr->CastSpell(plr, SPELL_HEAD_ANIM_RISE, false);    //17:25:31.000
                                head->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);   //hack
                            }
                    case EVENT_AISA_TALK_0:
                    case EVENT_AISA_TALK_1:
                    case EVENT_AISA_TALK_2:
                    case EVENT_AISA_TALK_4:
                    case EVENT_AISA_TALK_5:
                    case EVENT_AISA_TALK_6:
                    case EVENT_AISA_TALK_7:
                    case EVENT_AISA_TALK_8:
                    case EVENT_AISA_TALK_9:
                    case EVENT_AISA_TALK_10:
                    {
                        if (Creature *aisa = me->GetMap()->GetCreature(aisaGUID))
                            sCreatureTextMgr->SendChat(aisa, eventId - 2, playerGuid);
                        break;
                    }
                    
                    case EVENT_FIREPAW_TALK_0:
                    case EVENT_FIREPAW_TALK_1:
                    case EVENT_FIREPAW_TALK_2:
                    case EVENT_FIREPAW_TALK_3:
                    case EVENT_FIREPAW_TALK_4:
                    case EVENT_FIREPAW_TALK_5:
                    case EVENT_FIREPAW_TALK_6:
                    {
                        if (Creature *paw = me->GetMap()->GetCreature(firepawGUID))
                            sCreatureTextMgr->SendChat(paw, eventId - 13, playerGuid);
                        break;
                    }
                    case EVENT_SHEN_ZI_SU_TALK_0:   // 114898
                    case EVENT_SHEN_ZI_SU_TALK_1:   //cast 106759
                    case EVENT_SHEN_ZI_SU_TALK_2:   //cast 118571
                    case EVENT_SHEN_ZI_SU_TALK_3:   //118571
                    case EVENT_SHEN_ZI_SU_TALK_4:   //118572
                    case EVENT_SHEN_ZI_SU_TALK_5:   //118571
                        TalkShenZiSU(eventId - 20);
                        break;
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mop_air_balloonAI(creature);
    }
};

/*
========================================
========= S O U T H  P A R T ===========
========================================
*/

class mob_mandori_triger : public CreatureScript
{
public:
    mob_mandori_triger() : CreatureScript("mob_mandori_triger") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mandori_trigerAI(creature);
    }
    
    struct mob_mandori_trigerAI : public ScriptedAI
    {

        enum spells
        {
            SUMMON_AYSA                 = 115332, // Summon Aysa
            SUMMON_JI                   = 115335, // Summon Ji
            SUMMON_JOJO                 = 115337, // Summon Jojo
        };

        mob_mandori_trigerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
        }

        void MoveInLineOfSight(Unit* who)
        {
            Player *player = who->ToPlayer();
            if (!player)
                return;

           if (player->GetPositionX() < 710.0f || !me->IsWithinDistInMap(who, 50.0f))
               return;

           if (player->GetQuestStatus(QUEST_BIDDEN_TO_GREATNESS) != QUEST_STATUS_INCOMPLETE)
               return;

           if (player->HasAura(SUMMON_MANDORI_DOOR) || player->HasAura(SUMMON_PEI_WU_DOOR))
               return;

            //summon doors
            player->CastSpell(player, SUMMON_MANDORI_DOOR, false);
            player->CastSpell(player, SUMMON_PEI_WU_DOOR, false);

            //summon escort
            player->CastSpell(player, SUMMON_AYSA, true);
            player->CastSpell(player, SUMMON_JI, true);
            player->CastSpell(player, SUMMON_JOJO, true);
            return;
        }
    };
};

class mob_mandori_escort : public CreatureScript
{
    public:
        mob_mandori_escort() : CreatureScript("mob_mandori_escort") { }

    struct mob_mandori_escortAI : public npc_escortAI
    {        
        mob_mandori_escortAI(Creature* creature) : npc_escortAI(creature)
        {}

        enum escortEntry
        {
            NPC_AYSA                = 59986,
            NPC_JI                  = 59988,
            NPC_JOJO                = 59989,
            NPC_KORGA_STRONGMANE    = 60042, //Korga Strongmane
            NPC_WEI_PALERAGE        = 55943, //Wei Palerage <Hermit of the Forbidden Forest>

            SPELL_CREDIT_1          = 115442,
            SPELL_CREDIT_2          = 115443,

            SPELL_VIS_28            = 115449,

            EVENT_AISA_0            = 1,    //17:29:44.000 talk + credit
            EVENT_AISA_1            = 2,    //17:29:48.000 start move
            EVENT_AISA_2            = 3,    //17:30:12.000 stop move + emote stop
            EVENT_AISA_3            = 4,    //17:30:15.000 talk
            EVENT_AISA_4            = 5, 
            EVENT_AISA_5            = 6,
            EVENT_AISA_6            = 7,
            EVENT_AISA_7            = 8,

            EVENT_JI_0              = 9,
            EVENT_JI_1              = 10,
            EVENT_JI_2              = 11,
            EVENT_JI_3              = 12,
            EVENT_JI_4              = 13,
            EVENT_JI_5              = 14,
            EVENT_JI_6              = 15,
            EVENT_JI_7              = 16,
            EVENT_JI_8              = 17,

            EVENT_JOJO_0            = 18,
            EVENT_JOJO_1            = 19,
            EVENT_JOJO_2            = 20,
            EVENT_JOJO_3            = 21,
            EVENT_JOJO_4            = 22,
            EVENT_JOJO_5            = 23,

            EVENT_KORGA_0           = 24,
            EVENT_KORGA_1           = 25,
            EVENT_KORGA_2           = 26,
            EVENT_WEI_PALERAGE      = 27,
        };
        
        EventMap events;
        ObjectGuid playerGuid;
        
        ObjectGuid mandoriDoorGuid;
        ObjectGuid peiwuDoorGuid;

        void Reset()
        {
            playerGuid.Clear();
            mandoriDoorGuid.Clear();
            peiwuDoorGuid.Clear();

            me->SetReactState(REACT_PASSIVE);
        }

        bool Is(uint32 npc_entry) const
        {
            return me->GetEntry() == npc_entry;
        }

        void IsSummonedBy(Unit* summoner)
        {
            Player *player = summoner->ToPlayer();
            if (!player)
            {
                me->MonsterSay("SCRIPT::mob_mandori_escort summoner is not player", LANG_UNIVERSAL, ObjectGuid::Empty);
                return;
            }

            playerGuid = summoner->GetGUID();
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());

            if (GameObject* mandoriDoor = summoner->GetGameObject(SUMMON_MANDORI_DOOR))
                mandoriDoorGuid = mandoriDoor->GetGUID();
            else
            {
                me->MonsterSay("SCRIPT::mob_mandori_escort no go summoned by SUMMON_MANDORI_DOOR 115426", LANG_UNIVERSAL, playerGuid);
                return;
            }

            if (GameObject* mandoriDoor = summoner->GetGameObject(SUMMON_PEI_WU_DOOR))
                peiwuDoorGuid = mandoriDoor->GetGUID();
            else
            {
                me->MonsterSay("SCRIPT::mob_mandori_escort no go summoned by SUMMON_PEI_WU_DOOR 115435", LANG_UNIVERSAL, playerGuid);
                return;
            }

            uint32 t = 4000;                                         //17:29:43.000
            if (Is(NPC_AYSA))
            {
                events.RescheduleEvent(EVENT_AISA_0, t += 1000);       //17:29:44.000 talk + credit
                events.RescheduleEvent(EVENT_AISA_1, t += 1000);       //17:29:48.000
            }else// if (Is(NPC_JI) || Is(NPC_JOJO))
            {
                if (Is(NPC_JI))
                    events.RescheduleEvent(EVENT_JI_0, t += 2000);     //17:29:48.000
                if (Is(NPC_JOJO))
                    events.RescheduleEvent(EVENT_JOJO_0, t += 2000);   //17:29:48.000
            }
        }

        void WaypointReached(uint32 waypointId)
        {
            uint32 t = 0;
            switch (waypointId)
            {
                //cast 9  - 17:30:05.000 cast 115346 trigger dummy. 
                case 12:
                    SetEscortPaused(true);

                    if (Is(NPC_AYSA))
                    {
                        t = 1000;                                            //
                        events.RescheduleEvent(EVENT_AISA_2, t += 2000);       //17:30:12.000 stop move + emote stop
                        events.RescheduleEvent(EVENT_AISA_3, t += 3000);       //17:30:15.000 talk
                        events.RescheduleEvent(EVENT_AISA_4, t += 2000);       //17:30:17.000 talk
                        events.RescheduleEvent(EVENT_AISA_5, t += 16000);      //17:30:33.000 move continue
                    }else if (Is(NPC_JI))
                    {
                        t = 1000;                                           //17:30:10.000
                        events.RescheduleEvent(EVENT_JI_1, t += 9000);        //17:30:19.000 talk
                        events.RescheduleEvent(EVENT_JI_2, t += 3000);        //17:30:22.000 move + emote
                        events.RescheduleEvent(EVENT_JI_3, t += 2000);        //17:30:24.000
                        events.RescheduleEvent(EVENT_JI_4, t += 4000);        //17:30:28.000
                        events.RescheduleEvent(EVENT_JI_5, t += 5000);        //17:30:33.000
                        events.RescheduleEvent(EVENT_JI_6, t += 5000);        //17:30:38.000
                    }else if (Is(NPC_JOJO))
                    {
                        t = 0;                                              //17:30:10.000
                        events.RescheduleEvent(EVENT_JOJO_1, t += 1000);      //17:30:10.000
                        events.RescheduleEvent(EVENT_JOJO_2, t += 22000);     //17:30:32.000
                        events.RescheduleEvent(EVENT_JOJO_3, t += 2000);      //17:30:34.000
                        events.RescheduleEvent(EVENT_JOJO_4, t += 1000);      //17:30:35.000
                    }
                    break;
                case 20:
                    if (Is(NPC_AYSA))
                    {
                        SetEscortPaused(true);
                        t = 1000;                                            //17:30:59.000
                        events.RescheduleEvent(EVENT_AISA_6, t += 27000);      //17:31:27.000
                        events.RescheduleEvent(EVENT_AISA_7, t += 7000);       //17:31:35.000
                    }else if (Is(NPC_JOJO))
                    {
                        SetEscortPaused(true);
                        events.RescheduleEvent(EVENT_JOJO_5, 37000);           //17:31:37.000 
                    }
                    break;
                case 23:
                    if (Is(NPC_JI))
                    {
                        SetEscortPaused(true);
                        t = 1000;                                           //17:30:59.000
                        events.RescheduleEvent(EVENT_KORGA_0, t +=2000);      //17:31:02.000
                        events.RescheduleEvent(EVENT_WEI_PALERAGE, t +=4000); //17:31:06.000
                        events.RescheduleEvent(EVENT_KORGA_1, t +=9000);      //17:31:15.000
                        events.RescheduleEvent(EVENT_JI_7, t += 22000);       //17:31:37.000
                        events.RescheduleEvent(EVENT_KORGA_2, t +=7000);      //17:31:44.000
                        events.RescheduleEvent(EVENT_JI_8, t += 9000);        //17:31:53.000 + 17:33:58.000  despawn
                    }
                    break;
                default:
                    break;
            }
        }

        void LastWaypointReached()
        {           
            if (Is(NPC_AYSA))
            {
                if (GameObject* mandoriDoor = me->GetMap()->GetGameObject(mandoriDoorGuid))
                    mandoriDoor->Delete();
                if (GameObject* peiwuDoor = me->GetMap()->GetGameObject(peiwuDoorGuid))
                    peiwuDoor->Delete();
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            npc_escortAI::UpdateAI(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_AISA_0:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_AISA_1:
                        if (GameObject* mandoriDoor = me->GetMap()->GetGameObject(mandoriDoorGuid))
                            mandoriDoor->SetGoState(GO_STATE_ACTIVE);

                        if (Player* player = ObjectAccessor::FindPlayer(playerGuid))
                        {
                            me->CastSpell(player, SPELL_CREDIT_1, true);
                            player->RemoveAurasDueToSpell(SPELL_VIS_28);
                        }
                        Start(false, true);
                        break;
                    case EVENT_AISA_2:
                        me->HandleEmoteCommand(432);
                        break;
                    case EVENT_AISA_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_AISA_4:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGuid);
                        break;
                    case EVENT_AISA_7:
                    case EVENT_AISA_5:
                    case EVENT_JOJO_5:
                        SetEscortPaused(false);
                        break;
                    case EVENT_AISA_6:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGuid);
                        break;
                    case EVENT_JI_0:
                    case EVENT_JOJO_0:
                        Start(false, true);
                        break;
                    case EVENT_JI_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_JI_2:
                        me->GetMotionMaster()->MovePoint(100, 569.153f, 3582.24f, 94.95621f);
                        me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STEALTH_STAND);
                        break;
                    case EVENT_JI_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_JI_4:
                        me->SendPlaySpellVisualKit(15801, 0);
                        break;
                    case EVENT_JI_5:
                        me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                        SetEscortPaused(false);
                        break;
                    case EVENT_JI_6:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGuid);
                        break;
                    case EVENT_JI_7:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGuid);
                        break;
                    case EVENT_JI_8:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4, playerGuid);
                        me->DespawnOrUnsummon(120000);
                        break;
                    case EVENT_JOJO_1:
                        me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY_BOW);
                        break;
                    case EVENT_JOJO_2:
                        me->HandleEmoteCommand(15);
                        me->PlayDistanceSound(32037);
                        break;
                    case EVENT_JOJO_3:
                        if (GameObject* peiwuDoor = me->GetMap()->GetGameObject(peiwuDoorGuid))
                            peiwuDoor->SetGoState(GO_STATE_ACTIVE);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
                            me->CastSpell(player, SPELL_CREDIT_2, true);
                        me->GetMotionMaster()->MoveCharge(567.99f, 3583.41f, 94.74f);
                        break;
                    case EVENT_JOJO_4:
                        SetEscortPaused(false);
                        me->SendPlaySpellVisualKit(28180, 0);
                        break;
                    case EVENT_WEI_PALERAGE:
                        if (Creature* wei = me->FindNearestCreature(NPC_WEI_PALERAGE, 50.0f, true))
                            sCreatureTextMgr->SendChat(wei, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_KORGA_0:
                        if (Creature* korga = me->FindNearestCreature(NPC_KORGA_STRONGMANE, 50.0f, true))
                            sCreatureTextMgr->SendChat(korga, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_KORGA_1:
                        if (Creature* korga = me->FindNearestCreature(NPC_KORGA_STRONGMANE, 50.0f, true))
                            sCreatureTextMgr->SendChat(korga, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_KORGA_2:
                        if (Creature* korga = me->FindNearestCreature(NPC_KORGA_STRONGMANE, 50.0f, true))
                            sCreatureTextMgr->SendChat(korga, TEXT_GENERIC_2, playerGuid);
                        break;
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mandori_escortAI(creature);
    }
};

// Ji Yuan
class npc_ji_yuan : public CreatureScript
{
    public:
        npc_ji_yuan() : CreatureScript("npc_ji_yuan") { }

    struct npc_ji_yuanAI : public ScriptedAI
    {        
        npc_ji_yuanAI(Creature* creature) : ScriptedAI(creature)
        {}

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* summoner)
        {
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, summoner->GetGUID());
            me->GetMotionMaster()->MovePoint(1, 495.061f, 4007.92f, 78.3662f);
        }

        void MovementInform(uint32 moveType, uint32 waypointId)
        {
            me->DespawnOrUnsummon(5000);
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ji_yuanAI(creature);
    }
};

//56476 Injured Sailor Rescue Controller
class npc_injured_sailor_rescue_controller : public CreatureScript
{
public:
    npc_injured_sailor_rescue_controller() : CreatureScript("npc_injured_sailor_rescue_controller") { }
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_injured_sailor_rescue_controllerAI(creature);
    }
    
    struct npc_injured_sailor_rescue_controllerAI : public ScriptedAI
    {

        enum spells
        {
            SPELL_CANCEL_INJURED_SAILOR             = 117987,   //Cancel Injured Sailor Rescue Aura
            NPC_SAILOR                              = 56236,
            NPC_QUEST_CREDIT                        = 55999,
        };

        npc_injured_sailor_rescue_controllerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
        }

        void MoveInLineOfSight(Unit* who)
        {
           if (!me->IsWithinDistInMap(who, 10.0f))
               return;

           Vehicle* veh = who->GetVehicle();
           if (who->GetEntry() != NPC_SAILOR || !veh)
               return;

           Player* player = veh->GetBase()->ToPlayer();
           if (!player || player->GetQuestStatus(QUEST_NONE_LEFT_BEHINED) != QUEST_STATUS_INCOMPLETE)
               return;

            player->KilledMonsterCredit(NPC_QUEST_CREDIT);

            player->CastSpell(player, SPELL_CANCEL_INJURED_SAILOR, true);
            sCreatureTextMgr->SendChat(who->ToCreature(), TEXT_GENERIC_0, player->GetGUID());
            who->ToCreature()->DespawnOrUnsummon(5000);
            return;
        }
    };

};

class npc_hurted_soldier : public CreatureScript
{
public:
    npc_hurted_soldier() : CreatureScript("npc_hurted_soldier") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hurted_soldierAI (creature);
    }

    struct npc_hurted_soldierAI : public ScriptedAI
    {
        npc_hurted_soldierAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 checkSavedTimer;
        bool HasBeenSaved;

        void Reset()
        {
            checkSavedTimer = 2500;
            HasBeenSaved = false;
        }

        void OnSpellClick(Unit* Clicker)
        {
            HasBeenSaved = true;
        }

        void UpdateAI(uint32 diff)
        {
            if (checkSavedTimer)
            {
                if (checkSavedTimer <= diff)
                {
                    if (HasBeenSaved && !me->GetVehicle())
                    {
                        me->DespawnOrUnsummon(5000);
                        checkSavedTimer = 0;
                    }
                    else
                        checkSavedTimer = 2500;
                }
                else
                    checkSavedTimer -= diff;
            }
        }
    };
};

class boss_vordraka : public CreatureScript
{
public:
    boss_vordraka() : CreatureScript("boss_vordraka") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_vordrakaAI(creature);
    }

    struct boss_vordrakaAI : public ScriptedAI
    {
        boss_vordrakaAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap _events;

        enum eEnums
        {
            QUEST_ANCIEN_MAL        = 29798,

            EVENT_DEEP_ATTACK       = 1,
            EVENT_DEEP_SEA_RUPTURE  = 2,

            SPELL_DEEP_ATTACK       = 117287,
            SPELL_DEEP_SEA_RUPTURE  = 117456,
        };

        void Reset()
        {
            _events.RescheduleEvent(EVENT_DEEP_ATTACK, 10000);
            _events.RescheduleEvent(EVENT_DEEP_SEA_RUPTURE, 15000);
        }

        void JustDied(Unit* attacker)
        {
            std::list<Player*> playerList;
            GetPlayerListInGrid(playerList, me, 50.0f);

            for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                Player* player = *itr;
                if (player->GetQuestStatus(QUEST_ANCIEN_MAL) == QUEST_STATUS_INCOMPLETE)
                    if (player->isAlive())
                        player->KilledMonsterCredit(me->GetEntry());
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (_events.ExecuteEvent())
            {
                case EVENT_DEEP_ATTACK:
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 25.0f, true))
                        me->CastSpell(target, SPELL_DEEP_ATTACK, false);
                    _events.RescheduleEvent(EVENT_DEEP_ATTACK, 15000);
                    break;
                }
                case EVENT_DEEP_SEA_RUPTURE:
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 25.0f, true))
                        me->CastSpell(target, SPELL_DEEP_SEA_RUPTURE, false);
                    _events.RescheduleEvent(EVENT_DEEP_SEA_RUPTURE, 15000);
                    break;
                }
            }
        }
    };
};

//Aysa Cloudsinger <Master of Tushui>
class npc_aysa_cloudsinger : public CreatureScript
{
public:
    npc_aysa_cloudsinger() : CreatureScript("npc_aysa_cloudsinger") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_aysa_cloudsinger_AI (creature);
    }

    enum eSpell
    {
        SPELL_ROLL          = 117312,
        SPELL_FIST_FURY     = 117275,
        SPELL_SUMMON_AISA   = 117497,
        SPELL_SUMMON_JI     = 117597,
        NPC_NIGHMIRE        = 56009,
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_RISKING_IT_ALL)
            player->CastSpell(player, SPELL_SUMMON_AISA, true);
        return true;
    }

    struct npc_aysa_cloudsinger_AI : public ScriptedAI
    {
        npc_aysa_cloudsinger_AI(Creature* creature) : ScriptedAI(creature)
        {
            bossGUID.Clear();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        }
        
        enum eEvents
        {
            EVENT_FIST_FURY         = 1,
            EVENT_1                 = 2,
            EVENT_2                 = 3,
            EVENT_3                 = 4,
        };

        EventMap events;
        ObjectGuid bossGUID;
        GuidSet m_player_for_event;
        uint16 checkPOSTimer;

        void Reset()
        {
            events.Reset();
            events.RescheduleEvent(EVENT_FIST_FURY, 7000);
            checkPOSTimer = 2000;
        }

        void MoveInLineOfSight(Unit* who)
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (who->GetTypeId() != TYPEID_PLAYER)
                return;

            if (!me->IsWithinDistInMap(who, 60.0f))
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            if (who->ToPlayer()->GetQuestStatus(QUEST_ACIENT_EVIL) != QUEST_STATUS_INCOMPLETE)
                return;

            m_player_for_event.insert(who->GetGUID());
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, who->GetGUID());
            if (Creature* boss = me->FindNearestCreature(NPC_NIGHMIRE, 100.0f, true))
            {
                bossGUID = boss->GetGUID();
                if (!boss->isAlive())
                    boss->Respawn(true);
                AttackStart(boss);

                events.RescheduleEvent(EVENT_1, 10000);
                events.RescheduleEvent(EVENT_2, 15000);
                events.RescheduleEvent(EVENT_3, 35000);
            }
        }

        void EnterEvadeMode()
        {
            ScriptedAI::EnterEvadeMode();

            if (bossGUID)
            {
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4);
                bossGUID.Clear();
            }

            DoTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ());
        }

        void KilledUnit(Unit* /*victim*/)
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
            else
                damage = 1;
        }

        void EnterCombat(Unit* victim)
        {
            if (me->GetDistance(victim) > 10)
                me->CastSpell(victim, SPELL_ROLL, true);
        }

        void UpdateAI(uint32 diff)
        {
            UpdateVictim();

            if (checkPOSTimer)
            {
                if (checkPOSTimer <= diff)
                {
                    checkPOSTimer = 2000;
                    if ((me->GetDistance(me->GetHomePosition()) > 20.0f) || me->GetPositionZ() < 70.0f || me->GetPositionZ() > 80.0f)
                        DoTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ());
                }
                else
                    checkPOSTimer -= diff;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        break;
                    case EVENT_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                        break;
                    case EVENT_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                        break;
                    case EVENT_FIST_FURY:
                        DoCastVictim(SPELL_FIST_FURY, true);
                        events.RescheduleEvent(EVENT_FIST_FURY, 5000);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class mob_aysa_gunship_crash_escort : public CreatureScript
{
public:
    mob_aysa_gunship_crash_escort() : CreatureScript("mob_aysa_gunship_crash_escort") { }

    struct mob_aysa_gunship_crash_escortAI : public npc_escortAI
    {        
        mob_aysa_gunship_crash_escortAI(Creature* creature) : npc_escortAI(creature)
        {}

        EventMap events;
        ObjectGuid playerGuid;
        ObjectGuid jiGuid;

        enum escortEntry
        {
            EVENT_AISA_0        = 1,
            EVENT_AISA_1        = 2,
            EVENT_AISA_2        = 3,
            EVENT_AISA_3        = 4,
            EVENT_AISA_4        = 5,
            EVENT_AISA_5        = 6,
            EVENT_AISA_6        = 7,
            EVENT_AISA_7        = 8,
            EVENT_AISA_8        = 9,

            EVENT_JI_0          = 10,
            EVENT_JI_1          = 11,
            EVENT_JI_2          = 12,
            EVENT_JI_3          = 13,
            EVENT_JI_4          = 14,

            SPELL_FINISH        = 106037,
        };

        void Reset()
        {
            playerGuid.Clear();
            jiGuid.Clear();
        }

        void IsSummonedBy(Unit* summoner)
        {
            Player *player = summoner->ToPlayer();
            if (!player)
            {
                me->MonsterSay("SCRIPT::mob_aysa_gunship_crash_escort summoner is not player", LANG_UNIVERSAL, ObjectGuid::Empty);
                return;
            }

            playerGuid = summoner->GetGUID();
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());

            // summoned by spell 117600
            if (Creature* ji = me->SummonCreature(60741, 230.31f, 4006.67f, 87.27f, 3.38f, TEMPSUMMON_MANUAL_DESPAWN, 0, playerGuid))
                jiGuid = ji->GetGUID();

            uint32 t = 0;                                        //17:54:19.000
            events.RescheduleEvent(EVENT_AISA_0, t += 2000);       //17:54:21.000
            events.RescheduleEvent(EVENT_AISA_1, t += 3000);       //17:54:24.000
        }

        void WaypointReached(uint32 waypointId)
        {
            if (waypointId == 8)
            {
                SetEscortPaused(true);
                uint32 t = 0;                                        //
                events.RescheduleEvent(EVENT_AISA_2, t += 2000);       //17:54:41.000
                events.RescheduleEvent(EVENT_JI_0, t += 4000);         //17:54:45.000
                events.RescheduleEvent(EVENT_AISA_3, t += 9000);       //17:54:54.000
                events.RescheduleEvent(EVENT_JI_1, t += 4000);         //17:54:58.000
                events.RescheduleEvent(EVENT_AISA_4, t += 10000);      //17:55:08.000
                events.RescheduleEvent(EVENT_JI_2, t += 4000);         //17:55:12.000
                events.RescheduleEvent(EVENT_AISA_5, t += 6000);       //17:55:18.000
                events.RescheduleEvent(EVENT_AISA_6, t += 6000);       //17:55:24.000
                events.RescheduleEvent(EVENT_JI_3, t += 5000);         //17:55:29.000
                events.RescheduleEvent(EVENT_AISA_7, t += 4000);       //17:55:33.000
                events.RescheduleEvent(EVENT_AISA_8, t += 500);        //17:55:33.000
            }
        }

        void LastWaypointReached()
        {   
            if (Creature* ji = getJi())
                ji->DespawnOrUnsummon();
        }

        Creature* getJi()
        {
            return me->GetMap()->GetCreature(jiGuid);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            npc_escortAI::UpdateAI(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_AISA_0:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_AISA_1:
                        Start(false, true);
                        break;
                    case EVENT_AISA_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_AISA_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGuid);
                        break;
                    case EVENT_AISA_4:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGuid);
                        break;
                    case EVENT_AISA_5:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4, playerGuid);
                        break;
                    case EVENT_AISA_6:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5, playerGuid);
                        break;
                    case EVENT_AISA_7:
                        SetEscortPaused(false);
                        if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
                        {
                            player->KilledMonsterCredit(60727);
                            player->SendMovieStart(117);
                        }
                        break;
                    case EVENT_AISA_8:
                        if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
                            player->NearTeleportTo(249.38f, 3939.55f, 65.61f, 1.501471f);
                        break;
                    case EVENT_JI_0:
                        if (Creature* ji = getJi())
                        {
                            ji->SetFacingToObject(me);
                            sCreatureTextMgr->SendChat(ji, TEXT_GENERIC_0, playerGuid);
                        }
                        break;
                    case EVENT_JI_1:
                        if (Creature* ji = getJi())
                            sCreatureTextMgr->SendChat(ji, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_JI_2:
                        if (Creature* ji = getJi())
                            sCreatureTextMgr->SendChat(ji, TEXT_GENERIC_2, playerGuid);
                        break;
                    case EVENT_JI_3:
                        if (Creature* ji = getJi())
                            ji->GetMotionMaster()->MovePoint(0, 230.4045f, 3975.614f, 87.7406f);
                        break;
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_aysa_gunship_crash_escortAI(creature);
    }
    
};

#define MAX_ENNEMIES_POS   2
#define MAX_HEALER_COUNT   12
#define UPDATE_POWER_TIMER 3000

Position ennemiesPositions[MAX_ENNEMIES_POS] =
{
    {215.0f, 3951.0f, 71.4f},
    {290.0f, 3939.0f, 86.7f}
};


enum eEnums
{           
    NPC_HEALER_A            = 60878,
    NPC_HEALER_H            = 60896,
    NPC_ENNEMY              = 60858,
    NPC_ENEMY_2             = 60780,

    NPC_SHEN_HEAL_CREDIT    = 56011,

    EVENT_CHECK_PLAYERS     = 1,
    EVENT_UPDATE_POWER      = 2,
    EVENT_SUMMON_ENNEMY     = 3,
    EVENT_SUMMON_HEALER     = 4,
    EVENT_SEND_PHASE        = 5,

    SPELL_SHEN_HEALING      = 117783,
    SPELL_HEALER_A          = 117784,
    SPELL_HEALER_H          = 117932,

    SPELL_SCEAN_HEAL        = 117790,
};

class npc_ji_end_event : public CreatureScript
{
public:
    npc_ji_end_event() : CreatureScript("npc_ji_end_event") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ji_end_eventAI(creature);
    }

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        switch(quest->GetQuestId())
        {
            case QUEST_HEALING_SHEN:
                player->CastSpell(player, SPELL_SCEAN_HEAL, true);
                break;
        }
        return true;
    }

    struct npc_ji_end_eventAI : public ScriptedAI
    {
        npc_ji_end_eventAI(Creature* creature) : ScriptedAI(creature), _summons(creature)
        {}

        EventMap   _events;
        SummonList _summons;

        bool       inProgress;
        uint8      healerCount;
        uint8      ennemiesCount;
        uint16     actualPower;

        GuidSet m_player_for_event;

        void Reset()
        {
            _summons.DespawnAll();

            healerCount   = 0;
            ennemiesCount = 0;
            actualPower   = 0;

            inProgress = false;

            _events.Reset();
            _events.RescheduleEvent(EVENT_CHECK_PLAYERS, 5000);
        }


        void MoveInLineOfSight(Unit* who)
        {
            Player* player = who->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(QUEST_HEALING_SHEN) != QUEST_STATUS_INCOMPLETE)
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;

            m_player_for_event.insert(who->GetGUID());
            SendState(player, 1);
        }

        void SendState(Player* player, int32 value)
        {
            WorldPackets::WorldState::InitWorldStates packet;
            packet.AreaID = 5833;
            packet.MapID = 860;
            packet.SubareaID = 5736;
            packet.Worldstates.emplace_back(WorldStates::WS_ENABLE, value);
            packet.Worldstates.emplace_back(WorldStates::WS_HEALER_COUNT, healerCount);
            player->SendDirectMessage(packet.Write());            
        }

        void UpdateState()
        {
            for (GuidSet::iterator itr = m_player_for_event.begin(); itr != m_player_for_event.end(); ++itr)
                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                    player->SendUpdateWorldState(WorldStates::WS_HEALER_COUNT, healerCount);
        }

        bool CheckPlayers()
        {
            for (GuidSet::iterator itr = m_player_for_event.begin(); itr != m_player_for_event.end(); ++itr)
                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                    if (player->isAlive())
                        return true;

            return false;
        }

        void UpdatePower()
        {
            actualPower = (actualPower + healerCount <= 700) ? actualPower + healerCount: 700;
            for (GuidSet::iterator itr = m_player_for_event.begin(); itr != m_player_for_event.end(); ++itr)
                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                {
                    if (player->isAlive())
                    {
                        if (actualPower < 700) // IN_PROGRESS
                        {
                            if (!player->HasAura(SPELL_SHEN_HEALING))
                                player->CastSpell(player, SPELL_SHEN_HEALING, true);

                            player->SetPower(POWER_ALTERNATE, actualPower);
                        }
                        else
                        {
                            if (player->HasAura(SPELL_SHEN_HEALING))
                                player->RemoveAurasDueToSpell(SPELL_SHEN_HEALING);

                            player->KilledMonsterCredit(NPC_SHEN_HEAL_CREDIT);
                            SendState(player, 0);
                            continue;
                        }
                    }
                    
                }

            if (actualPower >= 700)
            {
                m_player_for_event.clear();
                Reset();
            }
        }

        void SummonEnnemy()
        {
            for(uint32 i = 0; i < 1; ++i)
                me->SummonCreature(RAND(NPC_ENEMY_2, NPC_ENNEMY), ennemiesPositions[i], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
        }

        void SummonHealer()
        {
            Map* map = sMapMgr->CreateMap(975, NULL);
            if (!map)
                return;

            float posX = frand(228.0f, 270.0f);
            float posY = frand(3949.0f, 3962.0f);

            me->SummonCreature(RAND(NPC_HEALER_A, NPC_HEALER_H), posX, posY, map->GetHeight(me->GetPhases(), posX, posY, 100.0f), 1.37f, TEMPSUMMON_CORPSE_DESPAWN);
        }

        void JustSummoned(Creature* summon)
        {
            _summons.Summon(summon);

            switch (summon->GetEntry())
            {
                case NPC_HEALER_A:
                case NPC_HEALER_H:
                     _events.RescheduleEvent(EVENT_SEND_PHASE, 1000);
                    ++healerCount;
                    break;
                case NPC_ENNEMY:
                case NPC_ENEMY_2:
                    AttackHealers(summon);
                    ++ennemiesCount;
                    break;
            }
        }

        void AttackHealers(Creature* summon)
        {
            uint32 hAttack = urand(0, healerCount - 1);
            uint32 i = 0;
            for(SummonList::iterator itr = _summons.begin(); itr != _summons.end(); ++itr)
            {
                if ((*itr).GetEntry() == NPC_HEALER_A || (*itr).GetEntry() == NPC_HEALER_H)
                {
                    if (Creature* healer = me->GetMap()->GetCreature(*itr))
                    {
                        if (i == hAttack)
                        {
                            summon->JumpTo(healer, 20.0f);
                            summon->AI()->AttackStart(healer);
                        }

                        summon->AddThreat(healer, 1000.0f);
                        ++i;
                    }
                }
            }
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            _summons.Despawn(summon);

            switch (summon->GetEntry())
            {
                case NPC_HEALER_A:
                case NPC_HEALER_H:
                    _events.RescheduleEvent(EVENT_SEND_PHASE, 1000);
                    --healerCount;
                    break;
                case NPC_ENNEMY:
                case NPC_ENEMY_2:
                    --ennemiesCount;
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
                case EVENT_CHECK_PLAYERS:
                {
                    bool playerNearWithQuest = CheckPlayers();

                    if (inProgress && !playerNearWithQuest)
                    {
                        inProgress = false;
                        Reset();
                    }
                    else if (!inProgress && playerNearWithQuest)
                    {
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                        inProgress = true;
                        _events.RescheduleEvent(EVENT_UPDATE_POWER,  UPDATE_POWER_TIMER);
                        _events.RescheduleEvent(EVENT_SUMMON_ENNEMY, 6000);
                        _events.RescheduleEvent(EVENT_SUMMON_HEALER, 5000);
                    }
                    _events.RescheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                    break;
                }
                case EVENT_UPDATE_POWER:
                    UpdatePower();
                    _events.RescheduleEvent(EVENT_UPDATE_POWER, UPDATE_POWER_TIMER);
                    break;
                case EVENT_SUMMON_ENNEMY:
                    if (healerCount > 0 && ennemiesCount < 5 && ennemiesCount < healerCount*2)
                        SummonEnnemy();
                    _events.RescheduleEvent(EVENT_SUMMON_ENNEMY, 7000);
                    break;
                case EVENT_SUMMON_HEALER:
                    if (healerCount < MAX_HEALER_COUNT)
                        SummonHealer();

                    _events.RescheduleEvent(EVENT_SUMMON_HEALER, 12500);
                    break;
                case EVENT_SEND_PHASE:
                    UpdateState();
                    break;
            }
        }
    };
};

class npc_shen_healer : public CreatureScript
{
    public:
        npc_shen_healer() : CreatureScript("npc_shen_healer") { }

        struct npc_shen_healerAI : public ScriptedAI
        {        
            npc_shen_healerAI(Creature* creature) : ScriptedAI(creature)
            {}

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                me->CastSpell(me, me->GetEntry() == NPC_HEALER_A ? SPELL_HEALER_A: SPELL_HEALER_H, true);
            }

            void EnterCombat(Unit*)
            {
                return;
            }
        };
    
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_shen_healerAI(creature);
        }
};

class npc_shang_xi_choose_faction : public CreatureScript
{
    public:
        npc_shang_xi_choose_faction() : CreatureScript("npc_shang_xi_choose_faction") { }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        if (action == 1)
        {
            if (player->GetQuestStatus(QUSRT_NEW_FATE) == QUEST_STATUS_REWARDED || 
                player->GetQuestStatus(QUSRT_NEW_FATE) == QUEST_STATUS_INCOMPLETE)
                player->ShowNeutralPlayerFactionSelectUI();
        }
        else if (action == 2)
            player->TeleportTo(0, -8866.55f, 671.93f, 97.90f, 5.31f);
        else if (action == 3)
            player->TeleportTo(1, 1577.30f, -4453.64f, 15.68f, 1.84f);

        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUSRT_NEW_FATE)
            creature->AI()->SetGUID(player->GetGUID(), 0);

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shang_xi_choose_factionAI(creature);
    }

    struct npc_shang_xi_choose_factionAI : public ScriptedAI
    {        
        npc_shang_xi_choose_factionAI(Creature* creature) : ScriptedAI(creature)
        {}

        EventMap events;
        ObjectGuid playerGuid;
        enum ev
        {
            EVENT_0        = 1, 
            EVENT_1        = 2,
            EVENT_2        = 3,
            EVENT_3        = 4,
            EVENT_4        = 5,
            EVENT_5        = 6,
        };

        void SetGUID(ObjectGuid const& guid, int32 id)
        {
            playerGuid = guid;
            uint32 t = 0;
            events.RescheduleEvent(EVENT_0, t += 1000);     //18:08:37.000
            events.RescheduleEvent(EVENT_1, t += 9000);     //18:08:46.000
            events.RescheduleEvent(EVENT_2, t += 11000);    //18:08:57.000
            events.RescheduleEvent(EVENT_3, t += 7000);     //18:09:04.000 
            events.RescheduleEvent(EVENT_4, t += 8000);     //18:09:12.000
            events.RescheduleEvent(EVENT_5, t += 10000);    //18:09:22.000 
        }

        void Reset()
        {
            playerGuid.Clear();
            events.Reset();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_0:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_2:
                        if (Creature* aysa = me->FindNearestCreature(57721, 100.0f, true))
                            sCreatureTextMgr->SendChat(aysa, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGuid);
                        break;
                    case EVENT_4:
                        if (Creature* czi = me->FindNearestCreature(57720, 100.0f, true))
                            sCreatureTextMgr->SendChat(czi, TEXT_GENERIC_0, playerGuid);
                        break;
                    case EVENT_5:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGuid);
                        break;
                }
            }
        }
    };
};

// by spell 120753
class mob_garosh_hord_way : public CreatureScript
{
public:
    mob_garosh_hord_way() : CreatureScript("mob_garosh_hord_way") { }

    struct mob_garosh_hord_wayAI : public npc_escortAI
    {        
        mob_garosh_hord_wayAI(Creature* creature) : npc_escortAI(creature)
        {}

        EventMap events;
        ObjectGuid playerGuid;
        ObjectGuid CziGUID;

        enum dataType
        {
            NPC_CZI        = 62081,
            NPC_CREDIT     = 62089,

            EVENT_0        = 1,
            EVENT_1        = 2,
            EVENT_2        = 3,
            EVENT_3        = 4,
            EVENT_4        = 5,
            EVENT_5        = 6,
            EVENT_6        = 7,
            EVENT_7        = 8,
            EVENT_8        = 9,
            EVENT_9,
            EVENT_10,
            EVENT_11,
            EVENT_12,
            EVENT_13,
            EVENT_14,
            EVENT_15,
            EVENT_16,
            EVENT_17,
            EVENT_18,
            EVENT_19,
            EVENT_20,

            EVENT_CZI_0_1,
            EVENT_CZI_0,
            EVENT_CZI_1,

        };

        void Reset()
        {
            me->SetWalk(true);
            playerGuid.Clear();
            CziGUID.Clear();
        }

        void IsSummonedBy(Unit* summoner)
        {
            Player *player = summoner->ToPlayer();
            if (!player)
            {
                me->MonsterSay("SCRIPT::mob_garosh_hord_way summoner is not player", LANG_UNIVERSAL, ObjectGuid::Empty);
                return;
            }

            playerGuid = summoner->GetGUID();
            me->AddPlayerInPersonnalVisibilityList(summoner->GetGUID());

            uint32 t = 0;                                        //
            events.RescheduleEvent(EVENT_0, t += 2000);            //18:12:24.000 start + talk
            events.RescheduleEvent(EVENT_1, t += 5000);            //18:12:29.000
            events.RescheduleEvent(EVENT_2, t += 8000);            //18:12:37.000
            events.RescheduleEvent(EVENT_3, t += 6000);            //18:12:43.000
            events.RescheduleEvent(EVENT_4, t += 5000);            //18:12:48.000
            events.RescheduleEvent(EVENT_5, t += 5000);            //18:12:54.000
            events.RescheduleEvent(EVENT_6, t += 1000);            //18:12:55.000
            events.RescheduleEvent(EVENT_7, t += 2000);            //18:12:57.000
            events.RescheduleEvent(EVENT_8, t += 9000);            //18:13:06.000
            events.RescheduleEvent(EVENT_CZI_0, t += 3000);        //18:13:09.000 Message: Да, вождь.
            events.RescheduleEvent(EVENT_9, t += 3000);            //18:13:12.000
            events.RescheduleEvent(EVENT_10, t += 3000);           //18:13:15.000
            events.RescheduleEvent(EVENT_11, t += 8000);           //18:13:23.000
            events.RescheduleEvent(EVENT_12, t += 6000);           //18:13:29.000
            events.RescheduleEvent(EVENT_13, t += 5000);           //18:13:34.000
            events.RescheduleEvent(EVENT_14, t += 13000);          //18:13:47.000
            events.RescheduleEvent(EVENT_15, t += 5000);           //18:13:52.000 
            events.RescheduleEvent(EVENT_16, t += 11000);          //18:14:03.000
            events.RescheduleEvent(EVENT_CZI_1, t += 3000);        //18:14:06.000 Message: Да... Да, конечно...
            events.RescheduleEvent(EVENT_17, t += 4000);           //18:14:10.000
            events.RescheduleEvent(EVENT_18, t += 1000);           //18:14:11.000
            events.RescheduleEvent(EVENT_19, t += 2000);           //18:14:13.000
            //events.RescheduleEvent(EVENT_20, t += 20000);           //18:14:33.000
        }

        void WaypointReached(uint32 waypointId)
        {
            switch(waypointId)
            {
                case 3:
                case 6:
                case 7:
                case 12: 
                case 13:
                case 16:
                if (Player* target = sObjectAccessor->FindPlayer(playerGuid))
                    me->SetFacingToObject(target);
                    SetEscortPaused(true);
                    break;
                case 26:
                    events.RescheduleEvent(EVENT_20, 1000);           //18:14:33.000
                    SetEscortPaused(true);
                    break;
            }
        }

        void initSummon()
        {
            if (Player* target = sObjectAccessor->FindPlayer(playerGuid))
                if (Creature* czi = me->FindNearestCreature(NPC_CZI, 200.0f, true))
                {
                    czi->GetMotionMaster()->MoveFollow(target, 2.0f, M_PI / 4);
                    CziGUID = czi->GetGUID();
                }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            npc_escortAI::UpdateAI(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_0:
                        Start(false, false);
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, playerGuid);
                        initSummon();
                        break;
                    case EVENT_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, playerGuid);
                        break;
                    case EVENT_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, playerGuid);
                        break;
                    case EVENT_3:
                        SetEscortPaused(false);
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGuid);
                        break;
                    case EVENT_4:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4, playerGuid);
                        break;
                    case EVENT_5:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5, playerGuid);
                        break;
                    case EVENT_6:
                        SetEscortPaused(false);
                        break;
                    case EVENT_7:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_6, playerGuid);
                        break;
                    case EVENT_8:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_7, playerGuid);
                        if (Creature* zu = me->GetMap()->GetCreature(CziGUID))
                            me->SetFacingToObject(zu);
                        break;
                    case EVENT_9:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_8, playerGuid);
                        break;
                    case EVENT_10:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_9, playerGuid);
                        SetEscortPaused(false);
                        break;
                    case EVENT_11:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_10, playerGuid);
                        break;
                    case EVENT_12:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_11, playerGuid);
                        break;
                    case EVENT_13:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_12, playerGuid);
                        SetEscortPaused(false);
                        break;
                    case EVENT_14:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_13, playerGuid);
                        SetEscortPaused(false);
                        break;
                    case EVENT_15:
                        me->HandleEmoteCommand(397);
                        break;
                    case EVENT_16:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_14, playerGuid);
                        if (Creature* zu = me->GetMap()->GetCreature(CziGUID))
                            me->SetFacingToObject(zu);
                        break;
                    case EVENT_17:
                        break;
                    case EVENT_18:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_15, playerGuid);
                        break;
                    case EVENT_19:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_16, playerGuid);
                        SetEscortPaused(false);
                        if (Player* plr = sObjectAccessor->FindPlayer(playerGuid))
                            plr->KilledMonsterCredit(NPC_CREDIT, ObjectGuid::Empty);
                        break;
                    case EVENT_20:
                        me->SetUInt32Value(UNIT_FIELD_MOUNT_DISPLAY_ID, 17719);
                        me->SetSpeed(MOVE_RUN, 8.0f, false);
                        me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0x3000000);
                        me->DespawnOrUnsummon(10000);
                        if (Creature* zu = me->GetMap()->GetCreature(CziGUID))
                            zu->DespawnOrUnsummon(10000);
                        break;
                    case EVENT_CZI_0:
                        if (Creature* zu = me->GetMap()->GetCreature(CziGUID))
                            sCreatureTextMgr->SendChat(zu, TEXT_GENERIC_3, playerGuid);
                        break;
                    case EVENT_CZI_1:
                        if (Creature* zu = me->GetMap()->GetCreature(CziGUID))
                            sCreatureTextMgr->SendChat(zu, TEXT_GENERIC_4, playerGuid);
                        break;
                }
            }
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_garosh_hord_wayAI(creature);
    }
    
};
void AddSC_WanderingIsland()
{
    //new mob_tushui_trainee();
    //new mob_master_shang_xi();
    new boss_jaomin_ro();
    new npc_panda_announcer();
    new mob_attacker_dimwind();
    new mob_min_dimwind();
    new npc_min_dimwind_outro();
    new mob_aysa_lake_escort();
    new mob_aysa();
    new boss_living_air();
    new boss_li_fei();
    new boss_li_fei_fight();
    new AreaTrigger_at_temple_entrance();
    // east
    new at_going_to_east();
    new npc_childrens_going_to_east();
    new AreaTrigger_at_bassin_curse();
    new vehicle_balance_pole();
    new mob_tushui_monk();
    new mob_jojo_ironbrow_1();
    new spell_rock_jump();
    new mob_shu_water_spirit();
    new spell_summon_spirit_of_watter();
    new mob_aysa_cloudsinger_watter_outro();
    new spell_grab_carriage();
    new vehicle_carriage();
    new npc_nourished_yak();
    new mob_jojo_ironbrow_2();
    new npc_water_spirit_dailo();
    new AreaTrigger_at_middle_temple_from_east();
    // west
    new mob_master_shang_xi_temple();
    new npc_wind_vehicle();
    new npc_panda_history_leason();
    new mob_jojo_ironbrow_3();
    new mob_huojin_monk();
    new spell_summon_ji_yung();
    new mob_jojo_ironbrow_4();
    new AreaTrigger_at_wind_temple_entrance();
    new mob_aysa_wind_temple_escort();
    new mob_frightened_wind();
    new boss_zhao_ren();
    new npc_rocket_launcher();
    new mob_master_shang_xi_after_zhao_escort();
    new mob_master_shang_xi_thousand_staff();
    new mob_aisa_pre_balon_event();
    new mop_air_balloon();
    // south
    new mob_mandori_triger();
    new mob_mandori_escort();
    new npc_ji_yuan();
    new npc_injured_sailor_rescue_controller();
    new npc_hurted_soldier();
    new boss_vordraka();
    new npc_aysa_cloudsinger();
    new mob_aysa_gunship_crash_escort();
    new npc_ji_end_event();
    new npc_shen_healer();
    new npc_shang_xi_choose_faction();
    new mob_garosh_hord_way();
}