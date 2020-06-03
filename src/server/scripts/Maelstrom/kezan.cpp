/*
Phases:
start q14113/14153 spell phase(59073) phase 2.
start q14115 spell phase(59074) phase 4.
start q14116 spell - 59087 phase 8
 */

#include "PrecompiledHeaders/ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Creature.h"
#include "CreatureTextMgr.h"
#include "Vehicle.h"
#include "QuestData.h"

enum Kezan_quests
{
    QUEST_ROLLING_WITH_MY_HOMIES       = 14071,  //Rolling with my Homies
    QUEST_GOOD_HELP_IS_HARD_TO_FIND    = 14069,
    QUEST_NECESSARY_ROUGHNESS          = 24502,
    QUEST_GOAL                         = 24503,
    QUEST_GOAL_2                       = 28414,
    QUEST_LIBERATE_KAJAMITE            = 14124,
    QUEST_GREAT_BANK_HEIST             = 14122,
    QUEST_ROBBING_HOODS                = 14121,
};

// npc_deffiant_troll
enum NPC_DeffiantTroll
{
    DEFFIANT_KILL_CREDIT               = 34830,
    SPELL_LIGHTNING_VISUAL             = 66306,
    GO_DEPOSIT                         = 195489,
};

enum TrollSpell
{
    SPELL_NOT_WORK = 45111,
    SPELL_SLEEP    = 62248,
};

enum DefiantSay
{
    SAY_WORK_1 = 0,
    SAY_WORK_2,
    SAY_WORK_3,
    SAY_WORK_4,
    SAY_WORK_5,
    SAY_WORK_6,
    SAY_WORK_7,
    SAY_WORK_8
};

#define WORK_TIMER 20000

class npc_defiant_troll : public CreatureScript
{
    public:
    npc_defiant_troll() : CreatureScript("npc_deffiant_troll") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_defiant_trollAI(creature);
    }

    struct npc_defiant_trollAI : public ScriptedAI
    {
        npc_defiant_trollAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 workTimer;
        bool work;

        void Reset () override
        {
            workTimer = 0;
            SetNotWork();
        }

        void SetNotWork()
        {
            work = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15 | UNIT_FLAG_NOT_SELECTABLE);
            me->CastSpell(me, SPELL_NOT_WORK, true);
            SetEquipmentSlots(false, 0);
            switch(urand(0, 2))
            {
                case 0:
                    me->CastSpell(me, SPELL_SLEEP, true);
                    break;
                case 1:
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 412);
                    break;
                default:
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 10);
                    break;
            }
        }

        void JustReachedHome() override
        {
            SetNotWork();
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_LIGHTNING_VISUAL && caster->GetTypeId() == TYPEID_PLAYER
                && caster->ToPlayer()->GetQuestStatus(QUEST_GOOD_HELP_IS_HARD_TO_FIND) == QUEST_STATUS_INCOMPLETE && work == false)
            {
                caster->ToPlayer()->KilledMonsterCredit(DEFFIANT_KILL_CREDIT, me->GetGUID());
                sCreatureTextMgr->SendChat(me, urand(0, 7), caster->GetGUID());
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15 | UNIT_FLAG_NOT_SELECTABLE);
                if (GameObject* Deposit = me->FindNearestGameObject(GO_DEPOSIT, 20))
                    me->GetMotionMaster()->MovePoint(1, Deposit->GetPositionX()-1, Deposit->GetPositionY(), Deposit->GetPositionZ());

                work = true;
                workTimer = WORK_TIMER;

                // set working animation
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 467);
                SetEquipmentSlots(false, urand(0, 1) == 0 ? 2202 : 2704);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            //If work send working animation
            if (work)
            {
                if(workTimer > diff)
                    workTimer -= diff;
                else
                {
                    me->GetMotionMaster()->MoveTargetedHome();
                }
                return;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};


enum Rod_Enum
{
    RADIO_MUSIC                     = 23406,
    GOBLIN_ZONE_MUSIC               = 15962,

    NPC_HIRED_LOOTER                = 35234,
    ITEM_STOLEN_LOOT                = 47530,
    SPELL_ADD_STOLEN_ITEM           = 67041,

    SPELL_RADIO                     = 66299,
    SPELL_STOP_RADIO                = 90247,
    SPELL_KNOCKBACK                 = 66301,

    SPELL_KEY_HOT_ROD               = 91551,  // area spell
    SPELL_EXIT_HOT_ROD              = 66611,

    SPELL_RESUMMON_IZZY             = 66646,
    SPELL_RESUMMON_ACE              = 66644,
    SPELL_RESUMMON_GOBER            = 66645,

    EVENT_KNOCK_BACK                = 1,
};

int QuestTemplateData [4][4] = {
    //entry     visibility   resumon seatID
    {0, 0, 0},
    {34959, INVISIBILITY_UNK9, 66646, 3},   // IZZY
    {34957, INVISIBILITY_UNK7, 66644, 1},   // ACE
    {34958, INVISIBILITY_UNK8, 66645, 2}    // GOBER
};

// Hot Rod
class npc_hot_rod : public CreatureScript
{
    public:
        npc_hot_rod() : CreatureScript("npc_hot_rod") { }

    struct npc_hot_rodAI : public ScriptedAI
    {
        npc_hot_rodAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        { 
            events.Reset();
            events.ScheduleEvent(EVENT_KNOCK_BACK, 500);
        }

        EventMap events;

        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id != SPELL_KNOCKBACK)
                return;

            if(target->GetEntry() == NPC_HIRED_LOOTER)
            {
                if(!me->GetVehicleKit())
                    return;

                Unit* passanger = me->GetVehicleKit()->GetPassenger(0);
                if (!passanger)
                    return;

                Player* player = passanger->ToPlayer();
                if (!player)
                    return;

                if (player->GetQuestStatus(QUEST_ROBBING_HOODS) != QUEST_STATUS_INCOMPLETE)
                    return;

                player->AddItem(ITEM_STOLEN_LOOT, 1);
                //target->CastSpell(player, SPELL_ADD_STOLEN_ITEM, true);
                target->CastSpell(target, 3240, false);     //kill. Blizz use spell 3617, but it not fined on dbc.
                
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_KNOCK_BACK:
                        DoCast(me, SPELL_KNOCKBACK);
                        events.ScheduleEvent(EVENT_KNOCK_BACK, 500);
                        break;
                }
            }
        }
        void OnCharmed(bool apply) override
        {
            //AI should work all time. in original mode id't disabled
        }

        void SetVisibilityDetect(Unit* target, InvisibilityType type, bool apply)
        {
            if (apply)
                target->m_invisibilityDetect.AddFlag(type);
            else
                target->m_invisibilityDetect.DelFlag(type);
            target->m_invisibilityDetect.AddValue(type, apply ? 1 : -1);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (Player* player = who->ToPlayer())
            {
                Quest const* qInfo = sQuestDataStore->GetQuestTemplate(QUEST_ROLLING_WITH_MY_HOMIES);
                ASSERT(qInfo);

                // Enable radio. As our veh not yet has passanger we do it a little hacky.
                if (apply)
                {
                    // enable radio
                    player->PlayDistanceSound(RADIO_MUSIC, player);

                    // update visibility for QUEST_ROLLING_WITH_MY_HOMIES
                    QuestStatusData* status = player->getQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES);
                    if (status)
                    //if (itr->second.Status == QUEST_STATUS_INCOMPLETE)
                    {
                        std::list<Creature*> Minions;
                        for(int j = 1; j < 4; ++j)
                        {
                            //if we not complite we should add visibility state for finding them
                            if (!player->GetQuestObjectiveData(qInfo, j))
                            {
                                SetVisibilityDetect(player, (InvisibilityType)QuestTemplateData[j][1], true);
                                SetVisibilityDetect(me, (InvisibilityType)QuestTemplateData[j][1], true);
                            }else
                            {
                                //TMP.
                                //Minions.clear();
                                //player->GetAllMinionsByEntry(Minions, QuestTemplateData[j][0]);
                                //if(Minions.empty())
                                //    player->CastSpell(player, QuestTemplateData[j][2], true);
                                //else
                                //{
                                //    if(Creature* guard = *Minions.begin())
                                //    {
                                //        if(me->GetVehicleKit()->HasEmptySeat(QuestTemplateData[j][3]))
                                //            guard->EnterVehicle(me, QuestTemplateData[j][3]);
                                //    }
                                //}
                            }
                        }
                    }
                }else
                {
                    //remove radio music
                    DoCast(who, SPELL_STOP_RADIO);

                    //remove visibility for QUEST_ROLLING_WITH_MY_HOMIES
                    for(int32 i = INVISIBILITY_UNK7; i < INVISIBILITY_UNK10; ++i)
                        SetVisibilityDetect(player, InvisibilityType(i), false);

                    //check for resumon guards
                    QuestStatusData* status = player->getQuestStatus(QUEST_ROLLING_WITH_MY_HOMIES);
                    if (status)
                    //if (itr->second.Status == QUEST_STATUS_INCOMPLETE)
                    {
                        for(int j = 1; j < 4; ++j)
                        {
                            //if we not complite we should add visibility state for finding them
                            if (player->GetQuestObjectiveData(qInfo, j))
                            {
                                //player->RemoveAllMinionsByFilter(QuestTemplateData[j][0]);
                                //TMP. In some cases call crash http://pastebin.com/vcnGS701
                                //player->CastSpell(player, QuestTemplateData[j][2], true);
                            }
                        }
                    }
                }

            }else if (!apply)
            {
                //remove all passangers
                who->ToCreature()->DespawnOrUnsummon();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hot_rodAI(creature);
    }
};

enum Friends
{
    NPC_IZZY        = 34890,
    NPC_ACE         = 34892,
    NPC_GOBER       = 34954,

    SPELL_SUMMON_IZZY               = 66600, // q.14071 sum.34959 orig.34890
    SPELL_SUMMON_ACE                = 66597, // q.14071 sum.34957 orig.34892
    SPELL_SUMMON_GOBER              = 66645, // q.14071 sum.34958 orig.34954

    //by 46598 they sit to veh
};

class npc_roling_friends : public CreatureScript
{
    public:
        npc_roling_friends() : CreatureScript("npc_roling_friends") { }

    struct npc_roling_friendsAI : public ScriptedAI
    {
        npc_roling_friendsAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override {}

        void MoveInLineOfSight(Unit* who) override
        {
            if (!me->IsWithinDistInMap(who, 10.0f))
                return;

            if (who->GetTypeId() == TYPEID_UNIT && who->ToCreature()->IsVehicle())
            {
                InvisibilityType type = INVISIBILITY_GENERAL;
                uint32 spellID = 0;
                switch(me->GetEntry())
                {
                    case NPC_IZZY:  type = INVISIBILITY_UNK9; spellID = SPELL_SUMMON_IZZY;    break;
                    case NPC_ACE:   type = INVISIBILITY_UNK7; spellID = SPELL_SUMMON_ACE;     break;
                    case NPC_GOBER: type = INVISIBILITY_UNK8; spellID = SPELL_RESUMMON_GOBER; break;
                    default:
                        return;
                }

                if(who->GetVehicleKit())
                if (Unit* passanger = who->GetVehicleKit()->GetPassenger(0))
                {
                    if (Player* player = passanger->ToPlayer())
                    {
                        //remove visibility for plr.
                        player->m_invisibilityDetect.DelFlag(type);
                        player->m_invisibilityDetect.AddValue(type, -1);
                        //summon
                        player->CastSpell(player, spellID, true);
                    }
                }
                //remove visibility for vehicle
                who->m_invisibilityDetect.DelFlag(type);
                who->m_invisibilityDetect.AddValue(type, -1);
            }
        }

        void UpdateAI(uint32 diff) override
        {

        }
 
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_roling_friendsAI(creature);
    }
};

enum FriendsGuard
{
    NPC_IZZY_GUARD        = 34959,  //Seat 3
    NPC_ACE_GUARD         = 34957,  //Seat 1
    NPC_GOBER_GUARD       = 34958,  //Seat 2

    EVENT_EMOTE           = 1,

    TEXT_KILL             = 0,
    TEXT_AGGRO            = 1,

};

int EmoteData [6] = { 273, 274, 11, 5, 25, 1 };
int GoberGuardEmote [6] = { 463, 20, 15, 1, 11, 440 };

class npc_roling_friends_guard : public CreatureScript
{
    public:
        npc_roling_friends_guard() : CreatureScript("npc_roling_friends_guard") { }

    struct npc_roling_friends_guardAI : public ScriptedAI
    {
        npc_roling_friends_guardAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset() override { events.Reset(); }

        void OnCharmed(bool apply) override
        {
            //AI should work all time. in original mode id't disabled
        }

        void EnterCombat(Unit* /*victim*/) override
        {
            sCreatureTextMgr->SendChat(me, TEXT_AGGRO);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            // God mode.
            damage = 0;
        }

        void EnterEvadeMode() override
        {
            //should send when our group kill someone. so do it on evade.
            if(me->isInCombat() && me->isAlive())
                sCreatureTextMgr->SendChat(me, TEXT_KILL);
            ScriptedAI::EnterEvadeMode();
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->SetReactState(REACT_AGGRESSIVE);

            Player* player = summoner->ToPlayer();
            if (!player || !player->m_movementInfo.transport.Guid)
                return;

            player->KilledMonsterCredit(me->GetEntry(), ObjectGuid::Empty);
            if (Unit* veh = player->GetVehicleBase())
            {
                uint8 seatID = 0;
                switch(me->GetEntry())
                {
                    case NPC_IZZY_GUARD:  seatID = 3; break;
                    case NPC_ACE_GUARD:   seatID = 1; break;
                    case NPC_GOBER_GUARD: seatID = 2; break;
                    default:
                        return;
                }

                
                if(veh->GetVehicleKit() && veh->GetVehicleKit()->HasEmptySeat(seatID))
                {
                    events.ScheduleEvent(EVENT_EMOTE, urand(1000, 5000));
                    me->EnterVehicle(veh, seatID);
                }
                else
                    me->ToCreature()->DespawnOrUnsummon();
            }
        }

        uint32 GenerateEmote()
        {
            switch(me->GetEntry())
            {
                case NPC_IZZY_GUARD:
                case NPC_ACE_GUARD:
                    return EmoteData[urand(0,5)];
                case NPC_GOBER_GUARD:
                    return GoberGuardEmote[urand(0,5)];
                default:
                    return 0;
            }
            return 0;
        }

        void UpdateAI(uint32 diff) override
        {
             events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_EMOTE:
                        me->HandleEmoteCommand(GenerateEmote());
                        events.ScheduleEvent(EVENT_EMOTE, urand(4000, 8000));
                        break;
                }
            }

            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }
 
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_roling_friends_guardAI(creature);
    }
};

/*
    Quest Necessary Roughness id=24502
    Vehicle Bilgewater Buccaneer id=37179 q.24502
*/

enum NecessaryRoughness
{
    NPC_VEH_PHASE_1                 = 37179,    //q24502
    NPC_VEH_PHASE_2                 = 37213,    //q37203
    NPC_PHASE_2_TARGET              = 37203,    //q37203

    TEXT_VEH_ANNOUNCE               = 0,

    EVENT_START_SHARK               = 1,
    EVENT_PLAY_SOUND                = 2,
    EVENT_FAIL                      = 3,
    EVENT_COMPLETE                  = 4,

    SOUND_START                     = 17466,
    SOUND_START_2                   = 17467,

    SPELL_SUMMON_VEH_PHASE_2        = 70075,
    SPELL_REVEHICLE                 = 70065,    // 70075+70065 

    SPELL_SUMMON                    = 69971,    //summon sharks 1. -8288.62 Y: 1479.97 Z: 43.8908
    SPELL_SUMMON_2                  = 69976,    //summon sharks 2. -8273.75 Y: 1484.46 Z: 42.9395
    SPELL_SUMMON_3                  = 69977,    //summon sharks 3. -8288.08 Y: 1487.72 Z: 43.8463
    SPELL_SUMMON_4                  = 69978,    //summon sharks 4. -8281.04 Y: 1477.49 Z: 43.3046
    SPELL_SUMMON_5                  = 69979,    //summon sharks 5. -8281.33 Y: 1490.41 Z: 43.4756
    SPELL_SUMMON_6                  = 69980,    //summon sharks 6. -8295.1 Y: 1484.91 Z: 44.3231
    SPELL_SUMMON_7                  = 69981,    //-8294.66 Y: 1474.68 Z: 44.2946
    SPELL_SUMMON_8                  = 69982,    //-8294.61 Y: 1493.67 Z: 44.6239
    SPELL_DESPAWN_SHARKS            = 69987,

    WAIPOINTS_SUMMON_1              = 371140,
    WAIPOINTS_SUMMON_2              = 371141,
    WAIPOINTS_SUMMON_3              = 371142,
    WAIPOINTS_SUMMON_4              = 371143,
    WAIPOINTS_SUMMON_5              = 371144,
    WAIPOINTS_SUMMON_6              = 371145,
    WAIPOINTS_SUMMON_7              = 371146,
    WAIPOINTS_SUMMON_8              = 371147,

    MAX_SPELL_ENUM                  = 8,
};

int citizen_emotes[6] = {1, 274, 92, 15, 94, 25};
int spell_summon[8] = {SPELL_SUMMON, SPELL_SUMMON_2, SPELL_SUMMON_3, SPELL_SUMMON_4, SPELL_SUMMON_5, SPELL_SUMMON_6, SPELL_SUMMON_7, SPELL_SUMMON_8};
float spawn_coods[8][3] =
{
    {-8288.62f, 1479.97f, 43.8908f},
    {-8273.75f, 1484.46f, 42.9395f},
    {-8288.08f, 1487.72f, 43.8463f},
    {-8281.04f, 1477.49f, 43.3046f},
    {-8281.33f, 1490.41f, 43.4756f},
    {-8295.1f,  1484.91f, 44.3231f},
    {-8294.66f, 1474.68f, 44.2946f},
    {-8294.61f, 1493.67f, 44.6239f},
};

class VehicleTriger : public BasicEvent
{
    public:
        explicit VehicleTriger(Unit *owner, uint32 spell) : _owner(owner), _spell(spell) { }

        bool Execute(uint64 /*currTime*/, uint32 /*diff*/) override
        {
            _owner->CastSpell(_owner, _spell, true);
            return true;
        }

    private:
        Unit *_owner;
        uint32 _spell;
};

//48526
class npc_bilgewater_buccaneer_click : public CreatureScript
{
    public:
        npc_bilgewater_buccaneer_click() : CreatureScript("npc_bilgewater_buccaneer_click") {}

        struct npc_bilgewater_buccaneer_clickAI : public CreatureAI
        {
            npc_bilgewater_buccaneer_clickAI(Creature* creature) : CreatureAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override {}
            
            void OnSpellClick(Unit* clicker) override
            {
                if (clicker->ToPlayer()) //HACK!!!!!!!!
                {
                    if (clicker->ToPlayer()->GetQuestStatus(QUEST_NECESSARY_ROUGHNESS) == QUEST_STATUS_INCOMPLETE)
                    {
                        clicker->ToPlayer()->CompleteQuest(QUEST_NECESSARY_ROUGHNESS);
                        //clicker->CastSpell(clicker, 70015, true); // VEH_PHASE_1
                    }
                    else if (clicker->ToPlayer()->GetQuestStatus(QUEST_GOAL) == QUEST_STATUS_INCOMPLETE)
                    {
                        clicker->ToPlayer()->KilledMonsterCredit(NPC_PHASE_2_TARGET);
                        //clicker->CastSpell(clicker, SPELL_SUMMON_VEH_PHASE_2, true);
                    }
                    else if (clicker->ToPlayer()->GetQuestStatus(QUEST_GOAL_2) == QUEST_STATUS_INCOMPLETE)
                    {
                        clicker->ToPlayer()->KilledMonsterCredit(NPC_PHASE_2_TARGET);
                        //clicker->CastSpell(clicker, SPELL_SUMMON_VEH_PHASE_2, true);
                    }
                }
            }
            
            void EnterCombat(Unit* who) override {}

            void UpdateAI(uint32 diff) override {}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_bilgewater_buccaneer_clickAI(creature);
        }
};

class npc_bilgewater_buccaneer : public CreatureScript
{
    public:
        npc_bilgewater_buccaneer() : CreatureScript("npc_bilgewater_buccaneer") {}

    struct npc_bilgewater_buccaneerAI : public ScriptedAI
    {
        npc_bilgewater_buccaneerAI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            me->SetControlled(true, UNIT_STATE_ROOT);
        }

        EventMap events;
        SummonList summons;
        uint32 soundID;

        void Reset() override
        { 
            events.Reset();
            soundID = 0;
        }

        Player * GetPassenger()
        {
            if(me->GetVehicleKit())
                if(Unit* passanger = me->GetVehicleKit()->GetPassenger(0))
                    return passanger->ToPlayer();
            return NULL;
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon) override
        {
            summons.Despawn(summon);
        }

        void DoAction(int32 const param) override
        {
            switch(param)
            {
                // at complete just despawn and exit
                case EVENT_COMPLETE:
                case EVENT_FAIL:
                    summons.DespawnAll();
                    if (me->GetVehicleKit())
                        me->GetVehicleKit()->RemoveAllPassengers();
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PLAY_SOUND:
                        if (Player* plr = GetPassenger())
                        {
                            plr->PlayDirectSound(SOUND_START, plr);
                            events.ScheduleEvent(EVENT_PLAY_SOUND, 1000);
                        }
                        break;
                    case EVENT_START_SHARK:
                    {
                        for(int i = 0; i < MAX_SPELL_ENUM; ++i)
                            me->CastSpell(spawn_coods[i][0], spawn_coods[i][1], spawn_coods[i][2], spell_summon[i], true);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        void OnCharmed(bool apply) override
        {
            //AI should work all time. in original mode id't disabled
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (Player* player = who->ToPlayer())
            {
                if (apply)
                {
                    if (player->GetQuestStatus(QUEST_NECESSARY_ROUGHNESS) == QUEST_STATUS_INCOMPLETE)
                    {
                        sCreatureTextMgr->SendChat(me, TEXT_VEH_ANNOUNCE, player->GetGUID());
                        events.ScheduleEvent(EVENT_START_SHARK, 5000);
                        events.ScheduleEvent(EVENT_PLAY_SOUND, 1000);
                    }
                }else
                {
                    //hard event stop
                    summons.DespawnAll();
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bilgewater_buccaneerAI(creature);
    }
};

class npc_bilgewater_buccaneer_2 : public CreatureScript
{
    public:
        npc_bilgewater_buccaneer_2() : CreatureScript("npc_bilgewater_buccaneer_2") {}

    struct npc_bilgewater_buccaneer_2_AI : public ScriptedAI
    {
        npc_bilgewater_buccaneer_2_AI(Creature* creature) : ScriptedAI(creature), summons(creature)
        {
            me->SetControlled(true, UNIT_STATE_ROOT);
        }

        EventMap events;
        SummonList summons;

        void Reset() override
        { 
            events.Reset();
        }

        void SetVisibilityDetect(Unit* target, InvisibilityType type, bool apply)
        {
            if (apply)
                target->m_invisibilityDetect.AddFlag(type);
            else
                target->m_invisibilityDetect.DelFlag(type);

            target->m_invisibilityDetect.AddValue(type, apply ? 1 : -1);
        }

        Player * GetPassenger()
        {
            if(me->GetVehicleKit())
                if(Unit* passanger = me->GetVehicleKit()->GetPassenger(0))
                    return passanger->ToPlayer();
            return NULL;
        }

        // Used for 2-nd phase vehicle summon
        void IsSummonedBy(Unit* summoner) override
        {
            summoner->AddDelayedEvent(100, [summoner]() -> void
            {
                summoner->CastSpell(summoner, SPELL_REVEHICLE, true);
            });
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PLAY_SOUND:
                        if (Player* plr = GetPassenger())
                        {
                            plr->PlayDirectSound(SOUND_START_2, plr);
                            events.ScheduleEvent(EVENT_PLAY_SOUND, 1000);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        void OnCharmed(bool apply) override
        {
            //AI should work all time. in original mode id't disabled
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (Player* player = who->ToPlayer())
            {
                if (apply)
                {
                    if (player->GetQuestStatus(QUEST_GOAL) == QUEST_STATUS_INCOMPLETE)
                    {
                        sCreatureTextMgr->SendChat(me, TEXT_VEH_ANNOUNCE, player->GetGUID());
                        player->CastSpell(player, 76651, true);
                        //SetVisibilityDetect(player, INVISIBILITY_UNK15, true);
                        if (Creature* target = me->FindNearestCreature(NPC_PHASE_2_TARGET, 500.0f, true))
                            me->SetFacingToObject(target);
                    }
                }else
                {
                    //SetVisibilityDetect(player, INVISIBILITY_UNK15, false);
                    me->DespawnOrUnsummon();
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bilgewater_buccaneer_2_AI(creature);
    }
};

int wp_steamwheedle_shark[8] = { WAIPOINTS_SUMMON_1, WAIPOINTS_SUMMON_2, WAIPOINTS_SUMMON_3, WAIPOINTS_SUMMON_4, WAIPOINTS_SUMMON_5, WAIPOINTS_SUMMON_6, WAIPOINTS_SUMMON_7, WAIPOINTS_SUMMON_8 };
int final_point[8] = { 4, 3, 4, 4, 4, 5, 5, 5 };

enum
{
    SPELL_BALL_HIT      = 69993,
    SPELL_DEATH         = 29266,
};

// Steamwheedle Shark
class npc_steamwheedle_shark : public CreatureScript
{
    public:
        npc_steamwheedle_shark() : CreatureScript("npc_steamwheedle_shark") { }

        struct npc_steamwheedle_sharkAI : public npc_escortAI
    {
        npc_steamwheedle_sharkAI(Creature* creature) : npc_escortAI(creature) {}

        uint8 _ID;
        EventMap events;

        void Reset() override { _ID = 0;}

        void IsSummonedBy(Unit* summoner) override
        {
            // as at creation we didn't get spellId creation use event for check it.
            events.ScheduleEvent(EVENT_START_SHARK, 1000);
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_BALL_HIT && !me->HasAura(SPELL_DEATH))
            {
                if (caster->GetTypeId() == TYPEID_UNIT && caster->ToCreature()->IsVehicle())
                {
                    if(caster->GetVehicleKit())
                    if (Unit* passanger = caster->GetVehicleKit()->GetPassenger(0))
                        if(Player* plr = passanger->ToPlayer())
                        {
                            plr->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
                            //check if we are on complete state
                            if(plr->GetQuestStatus(QUEST_NECESSARY_ROUGHNESS) == QUEST_STATUS_COMPLETE)
                            {
                                if (caster->ToCreature() && caster->IsAIEnabled)
                                    caster->ToCreature()->AI()->DoAction(EVENT_COMPLETE);
                            }
                        }
                }
                me->CastSpell(me, SPELL_DEATH, true);
                //hack for dummy effectof that spell.
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_EMOTES);
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveIdle();
                me->DespawnOrUnsummon(1000);
            }
        }

        void WaypointReached(uint32 id) override
        {
            if(final_point[_ID]==id)
            {
                //quest event failed
                if (TempSummon* summon = me->ToTempSummon())
                {
                    if (Unit* summoner = summon->GetSummoner())
                        if (summoner->ToCreature() && summoner->IsAIEnabled)
                            summoner->ToCreature()->AI()->DoAction(EVENT_FAIL);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_SHARK:
                    {
                        switch(me->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL))
                        {
                            case SPELL_SUMMON:
                                _ID = 0;
                                AddWaypoint(0, -8289.612f, 1479.857f, 43.79533f, 0);
                                AddWaypoint(1, -8288.619f, 1479.970f, 43.79533f, 5000);
                                AddWaypoint(2, -8278.678f, 1481.103f, 43.17033f, 5000);
                                AddWaypoint(3, -8268.736f, 1482.235f, 42.54533f, 5000);
                                AddWaypoint(4, -8260.878f, 1483.131f, 42.17003f, 5000);
                                break;
                            case SPELL_SUMMON_2:
                                _ID = 1;
                                AddWaypoint(0, -8274.75f, 1484.469f, 42.92033f, 0);
                                AddWaypoint(1, -8273.75f, 1484.46f, 42.92033f, 5000);
                                AddWaypoint(2, -8264.75f, 1484.381f, 42.3981f, 5000);
                                AddWaypoint(3, -8260.942f, 1484.347f, 42.1601f, 10000);
                                break;
                            case SPELL_SUMMON_3:
                                _ID = 2;
                                AddWaypoint(0, -8289.076f, 1487.813f, 43.92033f, 0);
                                AddWaypoint(1, -8288.08f, 1487.72f, 43.92033f, 5000);
                                AddWaypoint(2, -8281.107f, 1487.07f, 43.29533f, 5000);
                                AddWaypoint(3, -8269.154f, 1485.957f, 42.67033f, 5000);
                                AddWaypoint(4, -8261.186f, 1485.215f, 42.1481f, 5000);
                                break;
                            case SPELL_SUMMON_4:
                                _ID = 3;
                                AddWaypoint(0, -8282.017f, 1477.271f, 43.29533f, 0);
                                AddWaypoint(1, -8281.041f, 1477.49f, 43.29533f, 5000);
                                AddWaypoint(2, -8270.32f, 1479.907f, 42.67033f, 5000);
                                AddWaypoint(3, -8261.549f, 1481.885f, 42.1481f, 5000);
                                AddWaypoint(4, -8260.705f, 1482.075f, 42.1481f, 5000);
                                break;
                            case SPELL_SUMMON_5:
                                _ID = 4;
                                AddWaypoint(0, -8282.311f, 1490.609f, 43.54533f, 0);
                                AddWaypoint(1, -8281.33f, 1490.41f, 43.54533f, 5000);
                                AddWaypoint(2, -8273.486f, 1488.824f, 42.92033f, 5000);
                                AddWaypoint(3, -8264.662f, 1487.04f, 42.3981f, 5000);
                                AddWaypoint(4, -8260.743f, 1486.248f, 42.14829f, 5000);
                                break;
                            case SPELL_SUMMON_6:
                                _ID = 5;
                                AddWaypoint(0, -8296.1f, 1484.925f, 44.29533f, 0);
                                AddWaypoint(1, -8295.1f, 1484.91f, 44.29533f, 5000);
                                AddWaypoint(2, -8284.1f, 1484.749f, 43.54533f, 5000);
                                AddWaypoint(3, -8274.1f, 1484.603f, 42.92033f, 5000);
                                AddWaypoint(4, -8265.1f, 1484.471f, 42.3981f, 5000);
                                AddWaypoint(5, -8260.942f, 1484.41f, 42.1481f, 5000);
                                break;
                            case SPELL_SUMMON_7:
                                _ID = 6;
                                AddWaypoint(0, -8295.637f, 1474.465f, 44.17033f, 0);
                                AddWaypoint(1, -8294.66f, 1474.68f, 44.17033f, 5000);
                                AddWaypoint(2, -8284.895f, 1476.818f, 43.54533f, 5000);
                                AddWaypoint(3, -8274.152f, 1479.171f, 42.92033f, 5000);
                                AddWaypoint(4, -8266.34f, 1480.882f, 42.3981f, 5000);
                                AddWaypoint(5, -8260.713f, 1482.114f, 42.1481f, 5000);
                                break;
                            case SPELL_SUMMON_8:
                                _ID = 7;
                                AddWaypoint(0, -8295.589f, 1493.881f, 44.29533f, 0);
                                AddWaypoint(1, -8294.611f, 1493.67f, 44.29533f, 5000);
                                AddWaypoint(2, -8284.846f, 1491.561f, 43.67033f, 5000);
                                AddWaypoint(3, -8275.08f, 1489.451f, 43.04533f, 5000);
                                AddWaypoint(4, -8266.291f, 1487.553f, 42.5231f, 5000);
                                AddWaypoint(5, -8260.721f, 1486.349f, 42.16659f, 5000);
                                break;
                            default:
                                return;
                        }
                        Start(false, false);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
 
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_steamwheedle_sharkAI(creature);
    }
};

class spell_gen_stop_playing_current_music : public SpellScriptLoader
{
public:
    spell_gen_stop_playing_current_music() : SpellScriptLoader("spell_gen_stop_playing_current_music") { }

    class spell_gen_stop_playing_current_music_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gen_stop_playing_current_music_SpellScript);
              
        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* owner =  GetCaster()->GetOwner())
            {
                if (Player* player = owner->ToPlayer())
                    player->PlayDistanceSound(GOBLIN_ZONE_MUSIC, player);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_gen_stop_playing_current_music_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gen_stop_playing_current_music_SpellScript();
    }
};

class spell_gen_radio : public SpellScriptLoader
{
public:
    spell_gen_radio() : SpellScriptLoader("spell_gen_radio") { }

    class spell_gen_radio_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gen_radio_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* owner =  GetCaster()->GetOwner())
            {
                if (Player* player = owner->ToPlayer())
                    player->PlayDistanceSound(RADIO_MUSIC, player);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_gen_radio_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gen_radio_SpellScript();
    }
};

enum event_hacking
{
    EVENT_BANK_INTRO    = 1,
    EVENT_BANK_INTRO_2  = 2,
    EVENT_BANK_INTRO_3  = 3,
    EVENT_BANK_GENERATE = 4,
    EVENT_TIMEOUT       = 5,

};

enum bank_text
{
    TEXT_INTRO_1        = 0,
    TEXT_INTRO_2        = 1,
    TEXT_INTRO_3        = 2,

    TEXT_TIMEOUT        = 4,
    TEXT_RIGHT          = 7,
    TEXT_WRONG          = 10,
    TEXT_WIN            = 11,

    TEXT_REQ_SPELL_1    = 3,
    TEXT_REQ_SPELL_2    = 5,
    TEXT_REQ_SPELL_3    = 6,
    TEXT_REQ_SPELL_4    = 9,
    TEXT_REQ_SPELL_5    = 8,

    TEXT_EMPTY_STRING   = 12,   //Yes it's off-like, for cleaning string.
};

enum bank_data
{
    SPELL_TIMER             = 67502,
    SPELL_POWER_CORRECT     = 67493, //The Great Bank Heist: Power Correct
    SPELL_POWER_INCORECT    = 67494,//The Great Bank Heist: Power Incorrect
};

// The Great Bank Heist: Vault Interact
class spell_great_bank_heist : public SpellScriptLoader
{
public:
    spell_great_bank_heist() : SpellScriptLoader("spell_great_bank_heist") { }

    class spell_great_bank_heist_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_great_bank_heist_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster =  GetCaster())
            {
                if (caster->GetTypeId() == TYPEID_PLAYER &&
                    caster->ToPlayer()->GetQuestStatus(QUEST_GREAT_BANK_HEIST) != QUEST_STATUS_INCOMPLETE)
                    return;

                Position pos;
                caster->GetPosition(&pos);
                TempSummon* summon = caster->GetMap()->SummonCreature(35486, pos, NULL, 0, caster);

                //
                caster->CastSpell(summon, 67476, true);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_great_bank_heist_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_great_bank_heist_SpellScript();
    }
};

//hacking bank spells
class spell_gen_bank_hacking_spell : public SpellScriptLoader
{
public:
    spell_gen_bank_hacking_spell() : SpellScriptLoader("spell_bank_hacking_spells") { }

    class spell_gen_bank_hacking_spell_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gen_bank_hacking_spell_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            Creature *vehicle = caster->GetVehicleCreatureBase();
            if (!vehicle)
                return;

            if (!vehicle->IsAIEnabled || vehicle->GetTypeId() != TYPEID_UNIT)
                return;

            vehicle->AI()->SetData(m_scriptSpellId, 0);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_gen_bank_hacking_spell_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_gen_bank_hacking_spell_SpellScript();
    }
};

uint32 const hacking_spell[5] = { 67526, 67508, 67524, 67525, 67522 };
uint32 const hacking_text[5] =  {TEXT_REQ_SPELL_1, TEXT_REQ_SPELL_2, TEXT_REQ_SPELL_3, TEXT_REQ_SPELL_4, TEXT_REQ_SPELL_5};

class npc_hack_bank_controller : public CreatureScript
{
    public:
        npc_hack_bank_controller() : CreatureScript("npc_hack_bank_controller") { }

    struct npc_hack_bank_controllerAI : public ScriptedAI
    {
        npc_hack_bank_controllerAI(Creature* creature) : ScriptedAI(creature)
        {
            me->setPowerType(POWER_MANA);
            me->SetMaxPower(POWER_MANA, 100);
            me->SetPower(POWER_MANA, 0);
        }

        EventMap events;
        uint32 _select;
        ObjectGuid _playerGUID;

        void Reset() override
        { 
            events.Reset();
            _select = 0;
            _playerGUID.Clear();
        }

        void generate()
        {
            if (Player *pPlayer = Unit::GetPlayer(*me, _playerGUID))
            {
                _select = urand(0, 4);
                sCreatureTextMgr->SendChat(me, hacking_text[_select], _playerGUID); //
                sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                events.ScheduleEvent(EVENT_TIMEOUT, 5000);
                pPlayer->CastSpell(pPlayer, SPELL_TIMER, true);
            }else
                me->DespawnOrUnsummon();            
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (Player *pPlayer = Unit::GetPlayer(*me, _playerGUID))
                pPlayer->RemoveAura(SPELL_TIMER);

            events.CancelEvent(EVENT_TIMEOUT);
            
            if (hacking_spell[_select] == type)
            {
                // Right
                me->CastSpell(me, SPELL_POWER_CORRECT, true);
                if (me->GetPower(me->getPowerType()) < 100)
                {
                    events.ScheduleEvent(EVENT_BANK_GENERATE, urand(1000, 5000));
                    sCreatureTextMgr->SendChat(me, TEXT_RIGHT, _playerGUID);
                    sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                }
                else
                {
                    //Win
                    me->DespawnOrUnsummon();
                    sCreatureTextMgr->SendChat(me, TEXT_WIN, _playerGUID);
                    sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                    if (Player *pPlayer = Unit::GetPlayer(*me, _playerGUID))
                        pPlayer->AddItem(46858, 1);
                }
            }else
            {
                // Wrong
                events.ScheduleEvent(EVENT_BANK_GENERATE, 5000);
                sCreatureTextMgr->SendChat(me, TEXT_WRONG, _playerGUID);
                sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                me->CastSpell(me, SPELL_POWER_INCORECT, true);
            }

            // privent lagging casts.
            _select = 100;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    // emotes only when in vehicle.
                    case EVENT_BANK_INTRO:
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_1, _playerGUID);
                        sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                        events.ScheduleEvent(EVENT_BANK_INTRO_2, 10000);
                        break;
                    case EVENT_BANK_INTRO_2:
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_2, _playerGUID);
                        sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                        events.ScheduleEvent(EVENT_BANK_INTRO_3, 10000);
                        break;
                    case EVENT_BANK_INTRO_3:
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_3, _playerGUID);
                        sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                        events.ScheduleEvent(EVENT_BANK_GENERATE, 10000);
                        break;
                    case EVENT_BANK_GENERATE:
                        generate();
                        break;
                    case EVENT_TIMEOUT:
                        sCreatureTextMgr->SendChat(me, TEXT_TIMEOUT, _playerGUID);
                        sCreatureTextMgr->SendChat(me, TEXT_EMPTY_STRING, _playerGUID);
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            }
        }

        void OnCharmed(bool apply) override
        {
            //AI should work all time. in original mode id't disabled
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (!who->ToPlayer() || !apply)
            {
                me->DespawnOrUnsummon(1000);
                return;
            }

            _playerGUID = who->GetGUID();
            events.ScheduleEvent(EVENT_BANK_INTRO, 1000);
            who->ToPlayer()->KilledMonsterCredit(35486);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hack_bank_controllerAI(creature);
    }
};

//67502 - Используйте |cFFFF2222невероятное гамма-излучение!|r$B|TInterface\Icons\INV_Misc_EngGizmos_20.blp:64|t
//67020 - sound - 847 - 16381 - spell 67494
//67502 -67496 Используйте |cFFFF2222взрывхлопушки!|r$B|TInterface\Icons\INV_Misc_Bomb_07.blp:64|t

//Kaja'mite Deposit
enum misc_data
{
    GO_KAJAMITE_CHUNK       = 195492, //Kaja'mite Chunk
};

class go_kajamite_deposit : public GameObjectScript
{
public:
    go_kajamite_deposit() : GameObjectScript("go_kajamite_deposit") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (player->GetQuestStatus(QUEST_LIBERATE_KAJAMITE) == QUEST_STATUS_INCOMPLETE)
        {
            go->SendCustomAnim(0);
            go->SetLootState(GO_JUST_DEACTIVATED);

            uint8 count = urand(1, 4);
            Position pos;
            while(count > 0)
            {
                go->GetRandomPoint(*go, 5.0f, pos);
                go->SummonGameObject(GO_KAJAMITE_CHUNK, pos.m_positionX, pos.m_positionY, pos.m_positionZ, 0, 0, 0, 0, 0, 30000);
                --count;
            }
        }
        return true;
    }
};

void AddSC_kezan()
{
    new npc_defiant_troll;
    //new npc_hot_rod();
    new npc_roling_friends();
    new npc_roling_friends_guard();
    new npc_bilgewater_buccaneer_click();
    new npc_bilgewater_buccaneer();
    new npc_bilgewater_buccaneer_2();
    //new npc_steamwheedle_shark();
    new spell_gen_stop_playing_current_music();
    new spell_gen_radio();
    new spell_great_bank_heist();
    new spell_gen_bank_hacking_spell();
    new npc_hack_bank_controller();
    new go_kajamite_deposit();
}