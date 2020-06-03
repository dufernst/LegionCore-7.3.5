/*
Cosmetic toDo:
-- spell 74076 by 43359
-- spell 74070 by 43359 
-- spell 74072 by 43359
-- spell 74085 by 43359
*/

#include "PrecompiledHeaders/ScriptPCH.h"
#include "CreatureTextMgr.h"
#include "ScriptedEscortAI.h"
#include "QuestData.h"

enum isle_quests
{
    QUEST_DONT_GO_INTO_LIGHT                     = 14239,
    QUEST_GOBLIN_ESCAPE_PODS                     = 14474, // Goblin Escape Pods or 14001
    QUEST_MINER_TROUBLES                         = 14021, // Miner Troubles
    QUEST_CAPTURE_UNKNOWN                        = 14031, // Capturing the Unknown
    QUEST_WEED_WHACKER                           = 14236, // Weed Whacker
    QUEST_WARCHIEF_REVENGE                       = 14243, // Warchief's Revenge
    QUEST_CLUSTER_CLUCK                          = 24671, // Cluster Cluck
    QUEST_BIGGEST_EGG                            = 24744, // The Biggest Egg Ever
    QUEST_INVASION_IMMINENT                      = 24856, // Invasion Imminent!
    QUEST_IRRESTIBLE_POOL_PONY                   = 24864, // Irresistible Pool Pony
    QUEST_SURRENDER_OR_ELSE                      = 24868, // Surrender or Else!
    QUEST_CHILDREN_OF_TURTLE                     = 24954, // Children of a Turtle God
    QUEST_VOLCANOTH                              = 24958, // Volcanoth!
    QUEST_OLD_FRIENDS                            = 25023, // Old Friends
    QUEST_COLA_GIVE_YOU_IDEAS                    = 25110, // Kaja'Cola Gives You IDEAS! (TM)
    QUEST_MORALE_BOOST                           = 25122, // Morale Boost
    QUEST_ESCAPE_VELOCITY                        = 25214, // Escape Velocity
    QUEST_FINAL_CONFRONTATION                    = 25251, // Final Confrontation
};

enum isle_spells
{
    //Intro
    SPELL_SUMMON_DOC_ZAPNOZZLE                   = 69018, // Don't Go Into The Light!: Summon Doc Zapnozzle
    SPELL_NEAR_DEATH                             = 69010, // Near Death!
    SPELL_DOC_TO_CHAR                            = 69085, // Don't Go Into The Light!: Force Cast from Doc to Character + proc 69086
    SPELL_INTRO_VISUAL                           = 69085,
    SPELL_INTRO_RES                              = 69022,
    SPELL_INVISIBLE_INRO_DUMMY                   = 76354,
    SPELL_SUMMON_FRIGTNED_MINER                  = 68059,
    SPELL_SUMMON_ORE_CART                        = 68064,
    SPELL_VISUAL_ORE_CART_CHAIN                  = 68122,
    SPELL_VISUAL_CART_TRANSFORM                  = 68065,
    SPELL_PHOTO_VISUAL_SCREEN_EFFECT             = 70649, // Capturing The Unknown: Player's Screen Effect
    SPELL_PHOTO_VISUAL_BIND_SIGHT                = 70641, // Capturing The Unknown: Player's Bind Sight
    SPELL_PHOTO_SNAPSHOT                         = 68281, // KTC Snapflash
    SPELL_SHOOT                                  = 15620,
    SPELL_VISUAL_VILE_CAPTURE                    = 68295,
    SPELL_SUMMON_WW_CHANNEL_BUNNY                = 68216, // Weed Whacker: Summon Weed Whacker Channel Bunny
    SPELL_WEED_WHACKER                           = 68212, // Weed Whacker DMG aura
    SPELL_WEED_WHACKER_BUF                       = 68824, // Weed Whacker
    SPELL_PHASE_4                                = 67852,
    SPELL_PHASE_8                                = 67853,
    SPELL_TRALL_CHAIN_LIGHTING_RIGHT             = 68441,
    SPELL_TRALL_CHAIN_LIGHTING_LEFT              = 68440,
    SPELL_VISUAL_FLAME_AFTER_LIGHTING            = 42345,
    SPELL_VISUAL_UP_UP_ROCKET                    = 68813, // Up, Up & Away!: Force Cast from Sling Rocket
    SPELL_UP_UP_AWAY_KILL_CREDIT                 = 66127, // Up, Up & Away!: Kill Credit + Explosion
    SPELL_VISUAL_ROCKET_BLAST                    = 66110, // Rocket Scouting: Rocket Blast
    SPELL_REMOTE_CONTROL_FIREWORKS               = 71170, // Remote Control Fireworks
    SPELL_CC_FIREWORKS_VISUAL                    = 74177, // Cluster Cluck: Remote Control Fireworks Visual
    SPELL_PERMAMENT_DEATH                        = 29266,
    SPELL_FROST_NOWA                             = 11831,
    SPELL_FROSTBALL                              = 9672,
    SPELL_SUMMON_HATCHLONG1                      = 71919,
    SPELL_SUMMON_HATCHLONG2                      = 71918,
    SPELL_SUMMON_HATCHLONG3                      = 83115,
    SPELL_SUMMON_HATCHLONG4                      = 83116,
    SPELL_PONNY_AURA                             = 71914, // Surrender Or Else!: Faceless of the Deep - Beam Effect
    SPELL_SOE_FREEZE_ANIM                        = 72126, // Surrender Or Else!: Faceless of the Deep - Freeze Anim
    SPELL_SOE_ABSORPTION_SHIELD                  = 72055, // Absorption Shield
    SPELL_SOE_BEAM_EFFECT                        = 72076, 
    SPELL_SOE_STRANGE_TENTACLE                   = 71910, // Strange Tentacle: Base Effect
    SPELL_SHADOW_CRASH                           = 75903, // Shadow Crash
    SPELL_ENVELOPING_WINDS                       = 72518, // Enveloping Winds
    SPELL_ENVELOPING_WINDS_CAPTURED              = 72522, 
    SPELL_ZOMBIES_ROCKET_BOOTS_AICAST_ENTER      = 72897, // Zombies vs. Super Booster Rocket Boots: AICast on Enter Seat
    SPELL_ZVSBRB_NEW_BOOTS                       = 72948, // Zombies vs. Super Booster Rocket Boots: New Boots from Gossip. ToDo: add gossip and item check
    SPELL_ZVSBRB_DAMAGE_AURA                     = 72885, // Zombies vs. Super Booster Rocket Boots: Damage - Trigger
    SPELL_OF_BIND                                = 73135, // Old Friends: Quest Accept & Bind
    SPELL_OF_SUMMON_BOMBER                       = 73105, // Old Friends: Summon Flying Bomber
    SPELL_KCGYI_SUMMON_GREELY                    = 73603, // Kaja'Cola Gives You IDEAS! (TM): Summon Assistant Greely
    SPELL_FM_IZZY_FREED                          = 73613, // Free Their Minds: Izzy Freed
    SPELL_FM_GOOBER_FREED                        = 73614, // Free Their Minds: Gobber Freed
    SPELL_FM_ACE_FREED                           = 73602, 
    SPELL_EC_ON_INTERACT                         = 73947, // Escape Velocity: On Interact
    SPELL_EC_ROCKETS                             = 73948, // Escape Velocity: Rockets
};

enum isle_items
{
    ITEM_REMOTE_CONTROL_FIREWORKS                = 52712,
};

enum isle_npc
{
    NPC_DOC_ZAPNNOZZLE                           = 36608,
    NPC_GIZMO                                    = 36600, // Geargrinder Gizmo
    NPC_FRIGHTENED_MINER                         = 35813, // Frightened Miner
    NPC_FOREMAN_DAMPWICK                         = 35769, // Foreman Dampwick
    NPC_ORE_CART                                 = 35814, // Miner Troubles Ore Cart 35814
    NPC_QUEST_MINE_TROUBLES_CREDIT               = 35816,
    NPC_CLUSTER_CLUCK_KILL_CREDIT                = 38117, // Cluster Cluck Kill Credit
    NPC_RAPTOR                                   = 38187,
    NPC_BAMM_MEGABOMB                            = 38122, // Bamm Megabomb <Hunter Trainer>
    NPC_ASSISTANT_GREELY                         = 38124, // Assistant Greely
    NPC_ELM_PURPOSE_BUNNY                        = 24021, // ELM General Purpose Bunny (scale x0.01)
    NPC_NAGA_HATCHLING                           = 38412, // Naga Hatchling
    NPC_NAGA_HATCHLING2                          = 44578, // Naga Hatchling
    NPC_NAGA_HATCHLING3                          = 44579, // Naga Hatchling
    NPC_NAGA_HATCHLING4                          = 44580, // Naga Hatchling
    NPC_NAGA_KILL_CREDIT                         = 38413, // Naga Hatchling Kill Credit
    NPC_NAGA_PROTECTOR_HATCHLING                 = 38360,
    NPC_OOMLOT_SHAMAN                            = 38644, // Oomlot Shaman
    NPC_GOBLIN_ZOMBIE                            = 38753, // Goblin Zombie
    NPC_BASTIA_2                                 = 39152,
    NPC_KCGYI_GREELY                             = 39199,
    NPC_MB_KEZAN_CITIZEN                         = 38745, //Kezan Citizen
    NPC_MB_KEZAN_CITIZEN2                        = 38409,
    NPC_MB_IZI                                   = 38647,
    NPC_MB_GOOBER                                = 38746,
    NPC_MB_ACE                                   = 38441,
};

enum isle_sound
{
    SOUND_HATCHLING                             = 3437,
    MUSIC_VOLCANO                               = 23722,
};

enum isle_go
{
    GO_KAJAMITE_ORE                             = 195622, // Kaja'mite Ore
    GO_RAPTOR_TRAP                              = 201972, // Raptor Trap
    GO_RAPTOR_EGG                               = 201974,
};

enum isle_events
{
    EVENT_THE_VERY_BEGINING_1                      = 1,
    EVENT_THE_VERY_BEGINING_2                      = 2,
    EVENT_THE_VERY_BEGINING_3                      = 3,
    EVENT_THE_VERY_BEGINING_4                      = 4,
    EVENT_THE_VERY_BEGINING_5                      = 5,
    EVENT_THE_VERY_BEGINING_6                      = 6,
    EVENT_THE_VERY_BEGINING_7                      = 7,
    EVENT_THE_VERY_BEGINING_8                      = 8,
    EVENT_THE_VERY_BEGINING_9                      = 9,
    EVENT_THE_VERY_BEGINING_10                     = 10,
    EVENT_THE_VERY_BEGINING_11                     = 11,
    EVENT_THE_VERY_BEGINING_12                     = 12,
    EVENT_THE_VERY_BEGINING_13                     = 13,

    EVENT_GENERIC_1                                = 1,
    EVENT_GENERIC_2                                = 2,
    EVENT_GENERIC_3                                = 3,
    EVENT_GENERIC_4                                = 4,
    EVENT_GENERIC_5                                = 5,
    EVENT_GENERIC_6                                = 6,

    EVENT_POINT_MINE                               = 1000,
};

enum isle_emote
{
    EMOTE_FIND_MINE                                = 396,
    EMOTE_FIND_MINING                              = 233,
    EMOTE_SAFE_ORC                                 = 66,
    EMOTE_HATCHLING                                = 35,
};

enum gizmo_text
{
    TEXT_GIZMO_QUEST                             = 0,
};


class npc_gizmo : public CreatureScript
{
    public:
        npc_gizmo() : CreatureScript("npc_gizmo") { }

    struct npc_gizmoAI : public ScriptedAI
    {
        npc_gizmoAI(Creature* creature) : ScriptedAI(creature)
        {
            me->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK7);
            me->m_invisibilityDetect.AddValue(INVISIBILITY_UNK7, 100000);
        }

        EventMap events;
        GuidSet m_player_for_event;
        void Reset() override
        {
            m_player_for_event.clear();
            events.ScheduleEvent(EVENT_GENERIC_1, 1000);
        }

        void OnStartQuest(Player* player, Quest const* quest) override
        {
            if (!quest || quest->GetQuestId() != QUEST_GOBLIN_ESCAPE_PODS)
                return;

            sCreatureTextMgr->SendChat(me, TEXT_GIZMO_QUEST, player ? player->GetGUID() : ObjectGuid::Empty);
        }

        // Remove from conteiner for posibility repeat it.
        // If plr disconect or not finish quest.
        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            m_player_for_event.erase(guid);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->HasAura(SPELL_NEAR_DEATH))
            {
                // always player. don't warry
                // waiting then plr finish movie.
                if (who->ToPlayer()->isWatchingMovie())
                    return;

                GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
                if (itr == m_player_for_event.end())
                {
                    m_player_for_event.insert(who->GetGUID());
                    //me->CastSpell(537.135f, 3272.25f, 0.18f, SPELL_SUMMON_DOC_ZAPNOZZLE, true);
                    Position pos;
                    pos.Relocate(537.135f, 3272.25f, 0.18f, 2.46f);
                    if (TempSummon* summon = me->GetMap()->SummonCreature(NPC_DOC_ZAPNNOZZLE, pos))
                    {
                        summon->AddPlayerInPersonnalVisibilityList(who->GetGUID());
                        summon->AI()->SetGUID(who->GetGUID(), 0);
                        summon->AI()->SetGUID(me->GetGUID(), 1);
                    }
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                events.ScheduleEvent(EVENT_GENERIC_1, 1000);
                if (Player* p = me->FindNearestPlayer(50.0f, true))
                    MoveInLineOfSight(p);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_gizmoAI(creature);
    }
};



enum intro_text
{
    TEXT_INTRO_1                                   = 0,
    TEXT_INTRO_2                                   = 1,
    TEXT_INTRO_3                                   = 2,
    TEXT_INTRO_4                                   = 3,
    TEXT_INTRO_5                                   = 4,
};

class npc_doc_zapnnozzle : public CreatureScript
{
    public:
        npc_doc_zapnnozzle() : CreatureScript("npc_doc_zapnnozzle") { }

    struct npc_doc_zapnnozzleAI : public ScriptedAI
    {
        npc_doc_zapnnozzleAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->m_invisibilityDetect.AddFlag(INVISIBILITY_UNK7);
            creature->m_invisibilityDetect.AddValue(INVISIBILITY_UNK7, 100000);
        }

        ObjectGuid plrGUID;
        ObjectGuid gizmoGUID;
        EventMap events;

        void Reset() override
        {
            plrGUID.Clear();
            gizmoGUID.Clear();
            events.Reset();
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            switch(id)
            {
                case 0: plrGUID = guid; break;
                case 1:
                    gizmoGUID = guid;
                    events.ScheduleEvent(EVENT_THE_VERY_BEGINING_1, 1000);
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
                    case EVENT_THE_VERY_BEGINING_1:
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_1, plrGUID);
                        events.ScheduleEvent(++eventId, 2000);
                        break;
                    case EVENT_THE_VERY_BEGINING_2:
                        events.ScheduleEvent(++eventId, 3000);
                        if (Creature* gizmo = Unit::GetCreature(*me, gizmoGUID))
                            sCreatureTextMgr->SendChat(gizmo, TEXT_INTRO_1, plrGUID);
                        break;
                    case EVENT_THE_VERY_BEGINING_3:
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_2, plrGUID);
                        events.ScheduleEvent(++eventId, 3000);
                        break;
                    case EVENT_THE_VERY_BEGINING_4:
                        if (Player* target = sObjectAccessor->GetPlayer(*me, plrGUID))
                            DoCast(target, SPELL_INTRO_VISUAL);
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_3, plrGUID);
                        events.ScheduleEvent(++eventId, 1000);
                        break;
                    case EVENT_THE_VERY_BEGINING_5:
                    case EVENT_THE_VERY_BEGINING_6:
                    case EVENT_THE_VERY_BEGINING_8:
                    case EVENT_THE_VERY_BEGINING_9:
                        if (Player* target = sObjectAccessor->GetPlayer(*me, plrGUID))
                            DoCast(target, SPELL_INTRO_RES);
                        events.ScheduleEvent(++eventId, 2500);
                        break;
                    case EVENT_THE_VERY_BEGINING_7:
                        if (Player* target = sObjectAccessor->GetPlayer(*me, plrGUID))
                            DoCast(target, SPELL_INTRO_RES);
                        events.ScheduleEvent(++eventId, 2000);
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_4, plrGUID);
                        break;
                    case EVENT_THE_VERY_BEGINING_10:
                        sCreatureTextMgr->SendChat(me, TEXT_INTRO_5, plrGUID);
                        events.ScheduleEvent(++eventId, 2000);
                        break;
                    case EVENT_THE_VERY_BEGINING_11:
                        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        if (Player* target = sObjectAccessor->GetPlayer(*me, plrGUID))
                            target->RemoveAura(SPELL_NEAR_DEATH);
                        me->RemoveAura(SPELL_INVISIBLE_INRO_DUMMY);   
                        me->DespawnOrUnsummon(30000);
                        if (Creature* gizmo = Unit::GetCreature(*me, gizmoGUID))
                            gizmo->AI()->SetGUID(plrGUID, 0);
                        break;
                    default:
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_doc_zapnnozzleAI(creature);
    }
};

// NPC_FOREMAN_DAMPWICK                         = 35769, //Foreman Dampwick

enum foreman_text
{
    TEXT_FOREMAN_0          = 0,
    TEXT_FOREMAN_1          = 1,
};

class npc_foreman_dampwick : public CreatureScript
{
    public:
        npc_foreman_dampwick() : CreatureScript("npc_foreman_dampwick") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    { 
        player->CLOSE_GOSSIP_MENU();
        if (action == 1)
        {
            if (player->GetQuestStatus(QUEST_MINER_TROUBLES) != QUEST_STATUS_INCOMPLETE)
                return true;

            player->IncompleteQuest(QUEST_MINER_TROUBLES);
            creature->AI()->OnStartQuest(player, sQuestDataStore->GetQuestTemplate(QUEST_MINER_TROUBLES));
        }
        return true;
    }

    struct npc_foreman_dampwickAI : public ScriptedAI
    {
        npc_foreman_dampwickAI(Creature* creature) : ScriptedAI(creature)
        {

        }

        ObjectGuid guidMiner;

        void Reset() override
        {
            guidMiner.Clear();
        }

        void OnStartQuest(Player* player, Quest const* quest) override
        {
            if (!quest || quest->GetQuestId() != QUEST_MINER_TROUBLES)
                return;

            // last summoned creature in world.
            if (Unit::GetCreature(*me, guidMiner))
                return;

            Position pos;
            pos.Relocate(492.4184f, 2976.321f, 8.040207f);
            if (TempSummon* summon = player->GetMap()->SummonCreature(NPC_FRIGHTENED_MINER, pos, NULL, 0, player))
            {
                summon->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                summon->AI()->SetGUID(player->GetGUID(), 0);
                sCreatureTextMgr->SendChat(me, TEXT_FOREMAN_0, player->GetGUID());
                guidMiner = summon->GetGUID();
            }
        }

        void OnQuestReward(Player* player, Quest const* quest) override
        {
            if (!quest || quest->GetQuestId() != QUEST_MINER_TROUBLES)
                return;

            sCreatureTextMgr->SendChat(me, TEXT_FOREMAN_1, player->GetGUID());
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_foreman_dampwickAI(creature);
    }
};

enum miner_text
{
    TEXT_MINER_0      = 0,
    TEXT_MINER_1      = 1,
    TEXT_MINER_2      = 2,
    TEXT_MINER_3      = 3,
    TEXT_MINER_4      = 4,
    TEXT_MINER_5      = 5,
};

//! If something wrong loot ak SPELL_ATTR3_NO_INITIAL_AGGRO on spell.cpp
class npc_frightened_miner : public CreatureScript
{
public:
    npc_frightened_miner() : CreatureScript("npc_frightened_miner") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_frightened_minerAI (creature);
    }

    struct npc_frightened_minerAI : public npc_escortAI
    {
        npc_frightened_minerAI(Creature* creature) : npc_escortAI(creature) {}

        ObjectGuid plrGUID;
        ObjectGuid cartGUID;
        ObjectGuid mineGUID;
        EventMap events;
        uint32 wpMine;

        void Reset() override
        {
            plrGUID.Clear();
            cartGUID.Clear();
            mineGUID.Clear();
            wpMine = 0;
            events.Reset();
            
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            plrGUID = guid;
            Start(true, false, guid);
            DoCast(me, SPELL_SUMMON_ORE_CART);
        }

        void EnterEvadeMode() override
        {
            npc_escortAI::EnterEvadeMode();
            if (Creature* cart = Unit::GetCreature(*me, cartGUID))
                me->CastSpell(cart, SPELL_VISUAL_ORE_CART_CHAIN, true);
        }

        void JustSummoned(Creature* summon) override
        {
            summon->AddPlayerInPersonnalVisibilityList(plrGUID);
            summon->SetWalk(false);
            summon->SetSpeed(MOVE_RUN, 1.25f);
            summon->setFaction(35);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            summon->SetReactState(REACT_PASSIVE);
            SetFollowerGUID(summon->GetGUID());
            //summon->GetMotionMaster()->MoveFollow(me, 1.0f, 0);
            cartGUID = summon->GetGUID();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_MINER_TROUBLES);
        }

        void seachMine()
        {
            if (GameObject* mine = me->FindNearestGameObject(GO_KAJAMITE_ORE, 20.0f))
            {
                mineGUID = mine->GetGUID();
                SetEscortPaused(true);

                events.Reset();

                //Position pos;
                //mine->GetNearPosition(pos, 1.0f, 0.0f);
                me->GetMotionMaster()->MovePoint(EVENT_POINT_MINE, mine->m_positionX, mine->m_positionY, mine->m_positionZ);
                me->HandleEmoteCommand(EMOTE_FIND_MINE);
            }else if (HasEscortState(STATE_ESCORT_PAUSED))
                SetEscortPaused(false);
        }
        
        void MovementInform(uint32 moveType, uint32 pointId) override
        {
            if (pointId == EVENT_POINT_MINE)
            {
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_FIND_MINING);
                events.ScheduleEvent(EVENT_GENERIC_1, 10000);
                return;
            }
            if (HasEscortState(STATE_ESCORT_PAUSED))
                seachMine();

            npc_escortAI::MovementInform(moveType, pointId);
        }

        void WaypointReached(uint32 pointId) override
        {            
            switch(pointId)
            {
                case 1:
                    sCreatureTextMgr->SendChat(me, TEXT_MINER_0, plrGUID);
                    if (Creature* cart = Unit::GetCreature(*me, cartGUID))
                        me->CastSpell(cart, SPELL_VISUAL_ORE_CART_CHAIN, true);
                    break;
                case 7:
                    sCreatureTextMgr->SendChat(me, TEXT_MINER_1, plrGUID);
                    break;
                case 10:
                case 14:
                case 17:
                case 22:
                    wpMine = pointId;
                    seachMine();
                    break;
                case 23:
                    sCreatureTextMgr->SendChat(me, TEXT_MINER_5, plrGUID);
                    if (Player* player = GetPlayerForEscort())
                        player->KilledMonsterCredit(NPC_QUEST_MINE_TROUBLES_CREDIT);
                    break;
                case 24:
                    me->SetWalk(false);
                    break;
                default:
                    break;
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
                    // emotes only when in vehicle.
                    case EVENT_GENERIC_1:
                    {
                        uint32 text = 0;
                        switch(wpMine)
                        {
                            case 10: text = TEXT_MINER_2; break;
                            case 14: text = TEXT_MINER_3; break;
                            case 17: text = TEXT_MINER_4; break;
                        }
                        me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                        SetEscortPaused(false);
                        if (text)
                            sCreatureTextMgr->SendChat(me, text, plrGUID);
                        if (GameObject* go = ObjectAccessor::GetGameObject(*me, mineGUID))
                        {
                            go->SendCustomAnim(0);
                            go->SetLootState(GO_JUST_DEACTIVATED);
                        }
                        break;
                    }
                }
            }
        }
    };
};


/*
DELETE FROM `spell_scripts` WHERE id = 68279;
INSERT INTO `spell_scripts` (`id`, `effIndex`, `delay`, `command`, `datalong`, `datalong2`, `dataint`, `x`, `y`, `z`, `o`) VALUES 
('68279', '0', '0', '15', '70649', '2', '0', '0', '0', '0', '0'),
('68279', '0', '1', '15', '70641', '3', '0', '0', '0', '0', '0'),
('68279', '0', '2', '15', '68281', '3', '0', '0', '0', '0', '0');
*/


/*-- on hit 68279 add aura  70649 on player
-- and cast 70641 on creature target witch add this aura - bind sight
-- and start channel 68281 on target witch add this aura
*/
class spell_photo_capturing : public SpellScriptLoader
{
public:
    spell_photo_capturing() : SpellScriptLoader("spell_photo_capturing") { }

    class spell_photo_capturing_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_photo_capturing_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit* caster =  GetCaster();
            if (!caster)
                return;

            Unit* target = GetHitUnit();
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return;

            if (target->GetTypeId() == TYPEID_PLAYER &&
                target->ToPlayer()->GetQuestStatus(QUEST_CAPTURE_UNKNOWN) != QUEST_STATUS_INCOMPLETE)
                return;

            target->CastSpell(target, SPELL_PHOTO_VISUAL_SCREEN_EFFECT, true);
            target->CastSpell(caster, SPELL_PHOTO_VISUAL_BIND_SIGHT, true);
            target->CastSpell(caster, SPELL_PHOTO_SNAPSHOT, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_photo_capturing_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_photo_capturing_SpellScript();
    }
};

//Capturing The Unknown: KTC Snapflash Effect
class spell_ctu_snap_effect : public SpellScriptLoader
{
public:
    spell_ctu_snap_effect() : SpellScriptLoader("spell_ctu_snap_effect") { }

    class spell_ctu_snap_effect_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ctu_snap_effect_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit* caster =  GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (caster->GetTypeId() == TYPEID_PLAYER &&
                caster->ToPlayer()->GetQuestStatus(QUEST_CAPTURE_UNKNOWN) != QUEST_STATUS_INCOMPLETE)
                return;

            Unit* target = GetHitUnit();
            if (!target)
                return;

            for (int32 i = INVISIBILITY_UNK5; i < INVISIBILITY_UNK10; ++i)
            {
                if (target->m_invisibility.HasFlag((InvisibilityType)i))
                {
                    caster->m_invisibilityDetect.DelFlag((InvisibilityType)i);
                    break;
                }
            }
            caster->UpdateObjectVisibility();
            caster->RemoveAurasDueToSpell(SPELL_PHOTO_VISUAL_SCREEN_EFFECT);
            caster->ToPlayer()->KilledMonsterCredit(target->GetEntry());
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_ctu_snap_effect_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_ctu_snap_effect_SpellScript();
    }
};

//Orc Scout
class npc_orc_scout : public CreatureScript
{
    public:
        npc_orc_scout() : CreatureScript("npc_orc_scout") { }

    struct npc_orc_scoutAI : public Scripted_NoMovementAI
    {
        npc_orc_scoutAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        uint32 check;
        void Reset() override
        {
            check = 5000;
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            // God mode.
            damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            UpdateVictim();

            if (check <= diff)
            {
                Unit * target = me->getVictim();
                if (!target)
                {
                    target = me->SelectNearestTargetInAttackDistance(50.0f);
                    AttackStart(target);
                }
                if (target)
                    DoCast(target, SPELL_SHOOT);
                check = 5000;
            }else
                check -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_orc_scoutAI(creature);
    }
};

//Strangle Vine
class npc_strangle_vine : public CreatureScript
{
    public:
        npc_strangle_vine() : CreatureScript("npc_strangle_vine") { }

    struct npc_strangle_vineAI : public Scripted_NoMovementAI
    {
        npc_strangle_vineAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        void Reset() override
        {

        }

        void OnCharmed(bool /*apply*/) override
        {
        }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            if (passenger->GetTypeId() != TYPEID_UNIT)
                return;
            if (apply)
                passenger->CastSpell(passenger, SPELL_VISUAL_VILE_CAPTURE, true);
            else
            {
                passenger->RemoveAura(SPELL_VISUAL_VILE_CAPTURE);
                sCreatureTextMgr->SendChat(passenger->ToCreature(), TEXT_GENERIC_0);
                passenger->HandleEmoteCommand(EMOTE_SAFE_ORC);
                passenger->GetMotionMaster()->MovePoint(0, 606.6343f, 2785.689f, 88.11332f);
                passenger->ToCreature()->DespawnOrUnsummon(15000);
            }
                
        }

        void UpdateAI(uint32 diff) override
        {
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_strangle_vineAI(creature);
    }
};

//Weed Whacker 
class spell_weed_whacker : public SpellScriptLoader
{
public:
    spell_weed_whacker() : SpellScriptLoader("spell_weed_whacker") { }

    class spell_weed_whacker_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_weed_whacker_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit* caster =  GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (caster->GetTypeId() == TYPEID_PLAYER &&
                caster->ToPlayer()->GetQuestStatus(QUEST_WEED_WHACKER) != QUEST_STATUS_INCOMPLETE)
                return;

            if (caster->HasAura(SPELL_WEED_WHACKER))
            {
                caster->RemoveAura(SPELL_WEED_WHACKER);
                caster->RemoveAura(SPELL_WEED_WHACKER_BUF);
            }else
            {
                //caster->CastSpell(caster, 68216, true); // summon bunny
                //bunny cast channel on plr 68214 
                //if (TempSummon* summon = caster->GetMap()->SummonCreature(35903, *caster))
                //{
                //    summon->CastSpell(caster, 68214, true);
                //    summon->CastSpell(caster, 68217, true);
                //}

                caster->CastSpell(caster, SPELL_WEED_WHACKER, true);
                caster->CastSpell(caster, SPELL_WEED_WHACKER_BUF, true);
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_weed_whacker_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_weed_whacker_SpellScript();
    }
};

/*
ServerToClient: SMSG_ON_MONSTER_MOVE (0x6E17) Length: 182 ConnectionIndex: 2 Time: 08/07/2012 10:46:08.603 Type: Unknown Opcode Type Number: 69708
GUID: Full: 0xF1508EE900066DF4 Type: Vehicle Entry: 36585 Low: 421364
Toggle AnimTierInTrans: False
Position: X: 867.7984 Y: 2821.093 Z: 108.3891
Move Ticks: 33381247
Spline Type: Normal (0)
Spline Flags: Walkmode, Parabolic (35651584)
Move Time: 47733
Vertical Speed: 7.431631
Async-time in ms: 45413
Waypoints: 32
Waypoint Endpoint: X: 1079.168 Y: 3239.45 Z: 81.53538
[0] Waypoint: X: 1080.983 Y: 3235.022 Z: 82.71225
[1] Waypoint: X: 1080.983 Y: 3222.522 Z: 86.21225
[2] Waypoint: X: 1071.483 Y: 3201.772 Z: 88.21225
[3] Waypoint: X: 1064.483 Y: 3190.772 Z: 88.96225
[4] Waypoint: X: 1047.733 Y: 3181.772 Z: 89.71225
[5] Waypoint: X: 1032.233 Y: 3166.522 Z: 89.71225
[6] Waypoint: X: 1020.983 Y: 3152.772 Z: 87.71225
[7] Waypoint: X: 1009.983 Y: 3138.272 Z: 83.96225
[8] Waypoint: X: 1000.733 Y: 3123.272 Z: 81.21225
[9] Waypoint: X: 991.7334 Y: 3115.522 Z: 80.21225
[10] Waypoint: X: 977.4834 Y: 3112.022 Z: 79.71225
[11] Waypoint: X: 954.7334 Y: 3111.522 Z: 80.71225
[12] Waypoint: X: 936.7334 Y: 3109.522 Z: 81.46225
[13] Waypoint: X: 921.7334 Y: 3100.772 Z: 79.46225
[14] Waypoint: X: 910.2334 Y: 3091.522 Z: 77.71225
[15] Waypoint: X: 891.2334 Y: 3076.522 Z: 75.71225
[16] Waypoint: X: 875.9834 Y: 3058.272 Z: 71.96225
[17] Waypoint: X: 867.2334 Y: 3040.272 Z: 68.21225
[18] Waypoint: X: 859.2334 Y: 3017.522 Z: 66.46225
[19] Waypoint: X: 861.9834 Y: 2996.522 Z: 65.96225
[20] Waypoint: X: 868.4834 Y: 2978.772 Z: 64.96225
[21] Waypoint: X: 879.2334 Y: 2964.772 Z: 63.96225
[22] Waypoint: X: 898.9834 Y: 2944.772 Z: 64.46225
[23] Waypoint: X: 916.2334 Y: 2930.272 Z: 64.96225
[24] Waypoint: X: 925.7334 Y: 2916.772 Z: 65.96225
[25] Waypoint: X: 926.2334 Y: 2893.772 Z: 69.46225
[26] Waypoint: X: 911.9834 Y: 2882.772 Z: 71.21225
[27] Waypoint: X: 897.9834 Y: 2867.772 Z: 74.46225
[28] Waypoint: X: 894.7334 Y: 2858.522 Z: 77.21225
[29] Waypoint: X: 892.2334 Y: 2848.022 Z: 83.96225
[30] Waypoint: X: 886.2334 Y: 2836.772 Z: 92.21225
*/
class npc_bastia : public CreatureScript
{
public:
    npc_bastia() : CreatureScript("npc_bastia") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bastiaAI (creature);
    }

    struct npc_bastiaAI : public npc_escortAI
    {
        npc_bastiaAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;
        void Reset() override
        {
             PlayerOn       = false;
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
            switch(i)
            {
                case 20:
                    if (me->GetEntry() == NPC_BASTIA_2)
                        return;
                    //no break
                case 48:    // for NPC_BASTIA_2
                    if (Player* player = GetPlayerForEscort())
                    {
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
                    player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};

class npc_gyrochoppa : public CreatureScript
{
public:
    npc_gyrochoppa() : CreatureScript("npc_gyrochoppa") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_gyrochoppaAI (creature);
    }

    struct npc_gyrochoppaAI : public npc_escortAI
    {
        npc_gyrochoppaAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;
        void Reset() override
        {
             PlayerOn       = false;
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
             who->ToPlayer()->PlayDistanceSound(16422, who->ToPlayer());
        }

        void WaypointReached(uint32 i) override
        {
            switch(i)
            {
                case 6:
                    if (Player* player = GetPlayerForEscort())
                    {
                        SetEscortPaused(true);
                        player->ExitVehicle();
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, player->GetGUID());
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
                    player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};

// Meet Me Up Top: Quest Phase Controller Aura
// castom phaser. Blizz use it for remove phase 8 | 4 by i did it by removing them at spell_area... so just set phase logic
class spell_mmut_phase_controller : public SpellScriptLoader
{
    public:
        spell_mmut_phase_controller() : SpellScriptLoader("spell_mmut_phase_controller") { }

        class spell_mmut_phase_controller_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mmut_phase_controller_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* owner = GetUnitOwner();
                if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                    return;

                owner->ToPlayer()->GetPhaseMgr().RegisterPhasingAuraEffect(aurEff);    
                owner->UpdateObjectVisibility();
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* owner = GetUnitOwner();
                if (!owner  || owner->GetTypeId() != TYPEID_PLAYER)
                    return;

                owner->ToPlayer()->GetPhaseMgr().UnRegisterPhasingAuraEffect(aurEff);
                owner->UpdateObjectVisibility();
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectRemoveFn(spell_mmut_phase_controller_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_mmut_phase_controller_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mmut_phase_controller_AuraScript();
        }
};


//Chain Lightning trall
class spell_trall_chain_lightning : public SpellScriptLoader
{
public:
    spell_trall_chain_lightning() : SpellScriptLoader("spell_trall_chain_lightning") { }

    class spell_trall_chain_lightning_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_trall_chain_lightning_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit* caster =  GetCaster();
            if (!caster)
                return;

            Unit* target = GetHitUnit();
            if (!target || target->GetTypeId() == TYPEID_PLAYER)
                return;

            if (944.0f > target->GetPositionX() || 1030.0f < target->GetPositionX())
                target->CastSpell(target, SPELL_VISUAL_FLAME_AFTER_LIGHTING, true);
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Unit* caster =  GetCaster();
            if (!caster)
                return;

            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
            {
                if (983.0f < (*itr)->GetPositionX())    //left
                {
                    if (m_scriptSpellId == SPELL_TRALL_CHAIN_LIGHTING_LEFT)
                        ++itr;
                    else
                        targets.erase(itr++);
                }else
                {
                    if (m_scriptSpellId == SPELL_TRALL_CHAIN_LIGHTING_RIGHT)
                        ++itr;
                    else
                        targets.erase(itr++);
                }
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_trall_chain_lightning_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_trall_chain_lightning_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_trall_chain_lightning_SpellScript();
    }
};

class npc_cyclone_of_the_elements : public CreatureScript
{
public:
    npc_cyclone_of_the_elements() : CreatureScript("npc_cyclone_of_the_elements") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_cyclone_of_the_elementsAI (creature);
    }

    struct npc_cyclone_of_the_elementsAI : public npc_escortAI
    {
        npc_cyclone_of_the_elementsAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;
        bool onFinish;
        void Reset() override
        {
             PlayerOn       = false;
             onFinish       = false;
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
             who->ToPlayer()->PlayDistanceSound(16424, who->ToPlayer());
        }

        void WaypointReached(uint32 i) override
        {
            switch(i)
            {
                // last point of cycle. set 4 point, skipp start.
                case 25:
                    SetNextWaypoint(4, false, false);
                    break;
                case 34:
                    if (Player* player = GetPlayerForEscort())
                        player->ExitVehicle();
                    break;
                default:
                {
                    if (onFinish)
                        break;
                    if (Player* player = GetPlayerForEscort())
                    {
                        if (player->GetQuestStatus(QUEST_WARCHIEF_REVENGE) == QUEST_STATUS_COMPLETE)
                        {
                            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, player->GetGUID());
                            SetNextWaypoint(26, false, false);
                            onFinish = true;
                        }
                    }
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            
            if (PlayerOn)
            {
                if (Player* player = GetPlayerForEscort())
                    player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};

class npc_sling_rocket : public CreatureScript
{
public:
    npc_sling_rocket() : CreatureScript("npc_sling_rocket") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sling_rocketAI (creature);
    }

    struct npc_sling_rocketAI : public npc_escortAI
    {
        npc_sling_rocketAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;
        void Reset() override
        {
             PlayerOn       = false;
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
             who->ToPlayer()->PlayDistanceSound(2304, who->ToPlayer());
             me->CastSpell(me, SPELL_VISUAL_ROCKET_BLAST, true);
        }

        void WaypointReached(uint32 i) override
        {
            switch(i)
            {
                case 6:
                case 7:
                    me->CastSpell(me, SPELL_VISUAL_UP_UP_ROCKET, true);
                    break;
                case 8:
                    if (Player* player = GetPlayerForEscort())
                    {
                        player->ExitVehicle();
                        player->CastSpell(player, SPELL_UP_UP_AWAY_KILL_CREDIT, true);                        
                    }
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            
            if (PlayerOn)
            {
                if (Player* player = GetPlayerForEscort())
                    player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};

class npc_wild_clucker : public CreatureScript
{
public:
    npc_wild_clucker() : CreatureScript("npc_wild_clucker") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wild_cluckerAI (creature);
    }

    struct npc_wild_cluckerAI : public npc_escortAI
    {
        npc_wild_cluckerAI(Creature* creature) : npc_escortAI(creature) {}

        void Reset() override
        {

        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (caster->GetTypeId() != TYPEID_PLAYER || spell->Id != SPELL_REMOTE_CONTROL_FIREWORKS)
                return;

            Player *player = caster->ToPlayer();

            if (player->GetQuestStatus(QUEST_CLUSTER_CLUCK) != QUEST_STATUS_INCOMPLETE)
                return;

            Start(false, true);
            me->SendPlaySound(6820, false);
            me->CastSpell(me, SPELL_CC_FIREWORKS_VISUAL, true);
            player->KilledMonsterCredit(NPC_CLUSTER_CLUCK_KILL_CREDIT);
        }

        void WaypointReached(uint32 i) override
        {
            switch(i)
            {
                case 12:
                    me->DespawnOrUnsummon(1000);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
        }
    };
};

class npc_wild_clucker_egg : public CreatureScript
{
    public:
        npc_wild_clucker_egg() : CreatureScript("npc_wild_clucker_egg") { }

    struct npc_wild_clucker_eggAI : public Scripted_NoMovementAI
    {
        npc_wild_clucker_eggAI(Creature* creature) : Scripted_NoMovementAI(creature)
        { 
            me->SetLevel(6);
        }

        void Reset() override
        {
            if (GameObject* mine = me->FindNearestGameObject(GO_RAPTOR_TRAP, 20.0f))
                me->GetMotionMaster()->MovePoint(EVENT_POINT_MINE, mine->m_positionX, mine->m_positionY, mine->m_positionZ);

            
            if (Unit* target = me->FindNearestCreature(NPC_RAPTOR, 100.0f, true))
            {
                if (target->GetTypeId() == TYPEID_UNIT)
                {
                    me->CombatStart(target, true);
                    target->AddThreat(me, 10000);
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker->GetTypeId() != TYPEID_UNIT)
                return;

            if (GameObject* trap = me->FindNearestGameObject(GO_RAPTOR_TRAP, 20.0f))
            {
                trap->SendCustomAnim(0);                
                trap->SetLootState(GO_JUST_DEACTIVATED);
                trap->SummonGameObject(GO_RAPTOR_EGG, trap->m_positionX, trap->m_positionY, trap->m_positionZ, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 30000);
                attacker->CastSpell(attacker, SPELL_PERMAMENT_DEATH, true);
                attacker->ToCreature()->DespawnOrUnsummon(10000);
                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(uint32 diff) override
        {

        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wild_clucker_eggAI(creature);
    }
};

enum eventCooking
{
    EVENT_COOOKING_1        = 1,
    EVENT_COOOKING_2        = 2,
    EVENT_COOOKING_3        = 3,
    EVENT_COOOKING_4        = 4,
    EVENT_COOOKING_5        = 5,
    EVENT_COOOKING_6        = 6,
};

class npc_hobart_grapplehammer : public CreatureScript
{
    public:
        npc_hobart_grapplehammer() : CreatureScript("npc_hobart_grapplehammer") { }

    struct npc_hobart_grapplehammerAI : public ScriptedAI
    {
        npc_hobart_grapplehammerAI(Creature* creature) : ScriptedAI(creature)
        {

        }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void OnStartQuest(Player* player, Quest const* quest) override
        {
            if (!quest)
                return;
            
            switch(quest->GetQuestId())
            {
                case QUEST_CLUSTER_CLUCK:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, player->GetGUID());
                    break;
                case QUEST_BIGGEST_EGG:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, player->GetGUID());
                    break;
                case QUEST_INVASION_IMMINENT:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5, player->GetGUID());
                    break;
                case QUEST_CHILDREN_OF_TURTLE:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_6, player->GetGUID());
                    break;
                case QUEST_VOLCANOTH:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_7, player->GetGUID());
                    break;
                default:
                    break;
            }
        }

        void OnQuestReward(Player* player, Quest const* quest) override
        {
            if (!quest)
                return;

            switch(quest->GetQuestId())
            {
                case QUEST_CLUSTER_CLUCK:
                    if (Creature* target = me->FindNearestCreature(NPC_BAMM_MEGABOMB, 100.0f, true))
                        sCreatureTextMgr->SendChat(target, TEXT_GENERIC_0, player->GetGUID());
                    break;
                case QUEST_BIGGEST_EGG:
                    events.ScheduleEvent(EVENT_COOOKING_1, 1000);
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
                    case EVENT_COOOKING_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                        events.ScheduleEvent(EVENT_COOOKING_2, 3000);
                        break;
                    case EVENT_COOOKING_2:
                        events.ScheduleEvent(EVENT_COOOKING_3, 7000);
                        if (Creature* target = me->FindNearestCreature(NPC_ASSISTANT_GREELY, 100.0f, true))
                            sCreatureTextMgr->SendChat(target, TEXT_GENERIC_0);
                        break;
                    case EVENT_COOOKING_3:
                        events.ScheduleEvent(EVENT_COOOKING_4, 6000);
                        if (Creature* target = me->FindNearestCreature(NPC_ASSISTANT_GREELY, 100.0f, true))
                            sCreatureTextMgr->SendChat(target, TEXT_GENERIC_1);
                        break;
                    case EVENT_COOOKING_4:
                    {
                        // 
                        events.ScheduleEvent(EVENT_COOOKING_5, 5000);
                        if (Creature* target = me->FindNearestCreature(NPC_ELM_PURPOSE_BUNNY, 100.0f, true))
                            target->AI()->SetData(EVENT_COOOKING_1, true);
                        if (Creature* target = me->FindNearestCreature(NPC_ASSISTANT_GREELY, 100.0f, true))
                            sCreatureTextMgr->SendChat(target, TEXT_GENERIC_2);
                        break;
                    }
                    case EVENT_COOOKING_5:
                        events.ScheduleEvent(EVENT_COOOKING_6, 5000);
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4);
                        break;
                    case EVENT_COOOKING_6:
                        if (Creature* target = me->FindNearestCreature(NPC_ELM_PURPOSE_BUNNY, 100.0f, true))
                            target->AI()->SetData(EVENT_COOOKING_1, false);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hobart_grapplehammerAI(creature);
    }
};

class npc_vashjelan_siren : public CreatureScript
{
    public:
        npc_vashjelan_siren() : CreatureScript("npc_vashjelan_siren") { }

    struct npc_vashjelan_sirenAI : public ScriptedAI
    {
        npc_vashjelan_sirenAI(Creature* creature) : ScriptedAI(creature){}
        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void Hatchiling(std::list<Creature*>& creatureList, Unit* killer)
        {
            for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
            {
                Creature* c = *itr;
                c->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                c->GetMotionMaster()->MovePoint(EVENT_GENERIC_1, killer->m_positionX, killer->m_positionY, killer->m_positionZ);
                c->AI()->SetGUID(killer->GetGUID(), EVENT_GENERIC_1);
            }
        }

        void JustDied(Unit* killer) override
        {
            if (!killer->HasAura(SPELL_PONNY_AURA))
                return;

            Player *player = killer->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(QUEST_IRRESTIBLE_POOL_PONY) != QUEST_STATUS_INCOMPLETE)
                return;

            std::list<Creature*> creatureList;
            me->GetCreatureListWithEntryInGrid(creatureList, NPC_NAGA_HATCHLING, 25.0f);
            Hatchiling(creatureList, killer);
            me->GetCreatureListWithEntryInGrid(creatureList, NPC_NAGA_HATCHLING2, 25.0f);
            Hatchiling(creatureList, killer);
            me->GetCreatureListWithEntryInGrid(creatureList, NPC_NAGA_HATCHLING3, 25.0f);
            Hatchiling(creatureList, killer);
            me->GetCreatureListWithEntryInGrid(creatureList, NPC_NAGA_HATCHLING4, 25.0f);
            Hatchiling(creatureList, killer);
        }

        void EnterCombat(Unit* /*victim*/) override
        {
            events.ScheduleEvent(EVENT_GENERIC_1, 3000);
            events.ScheduleEvent(EVENT_GENERIC_2, 4000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GENERIC_1:
                        me->CastSpell(me, SPELL_FROST_NOWA, true);
                        break;
                    case EVENT_GENERIC_2:
                        if (Unit *target = me->getVictim())
                            me->CastSpell(target, SPELL_FROSTBALL, true);
                        events.ScheduleEvent(EVENT_GENERIC_2, 3000);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_vashjelan_sirenAI(creature);
    }
};

const uint32 sum_hat[4] = {SPELL_SUMMON_HATCHLONG1, SPELL_SUMMON_HATCHLONG2, SPELL_SUMMON_HATCHLONG3, SPELL_SUMMON_HATCHLONG4};

class npc_naga_hatchling : public CreatureScript
{
    public:
        npc_naga_hatchling() : CreatureScript("npc_naga_hatchling") { }

    struct npc_naga_hatchlingAI : public ScriptedAI
    {
        npc_naga_hatchlingAI(Creature* creature) : ScriptedAI(creature){}
        EventMap events;
        ObjectGuid plrGUID;
        void Reset() override
        {
            events.Reset();
            plrGUID.Clear();
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            events.ScheduleEvent(EVENT_GENERIC_1, 20000);
            plrGUID = guid;
        }

        void OnSpellClick(Unit* clicker) override
        {
            if (!clicker || clicker->GetTypeId() != TYPEID_PLAYER)
                return;

            clicker->CastSpell(clicker, sum_hat[urand(0, 3)], true);
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->PlayDistanceSound(SOUND_HATCHLING);
            me->HandleEmoteCommand(EMOTE_HATCHLING);
            clicker->ToPlayer()->KilledMonsterCredit(NPC_NAGA_KILL_CREDIT);
            me->DespawnOrUnsummon(100);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE)
                return;
            me->PlayDistanceSound(SOUND_HATCHLING);
            me->HandleEmoteCommand(EMOTE_HATCHLING);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!who->HasAura(SPELL_PONNY_AURA) || me->HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
                return;

            Player *player = who->ToPlayer();
            if (!player)
                return;

            if (player->GetQuestStatus(QUEST_IRRESTIBLE_POOL_PONY) != QUEST_STATUS_INCOMPLETE)
                return;

            if (me->FindNearestCreature(NPC_NAGA_PROTECTOR_HATCHLING, 25.0f, true))
                return;

            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->GetMotionMaster()->MovePoint(EVENT_GENERIC_1, who->m_positionX, who->m_positionY, who->m_positionZ);
            SetGUID(who->GetGUID(), EVENT_GENERIC_1);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GENERIC_1:
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        break;
                    default:
                        break;
                }
            }

        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_naga_hatchlingAI(creature);
    }
};

enum Phases
{
    PHASE_INTRO = 1,
    PHASE_COMBAT,
};


class npc_faceless_of_the_deep : public CreatureScript
{
    public:
        npc_faceless_of_the_deep() : CreatureScript("npc_faceless_of_the_deep") { }

    struct npc_faceless_of_the_deepAI : public ScriptedAI
    {
        npc_faceless_of_the_deepAI(Creature* creature) : ScriptedAI(creature){}
        EventMap events;
        ObjectGuid playerGUID;
        void Reset() override
        {
            events.Reset();
            playerGUID.Clear();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }


        void EnterCombat(Unit* /*victim*/) override
        {
            me->SetCanFly(false);
            me->SetDisableGravity(false);
        }

        void JustDied(Unit* /*killer*/) override
        {
            events.Reset();
        }

        void MoveInLineOfSight(Unit* who) override
        {
            Player *player = who->ToPlayer();
            if (!player || events.IsInPhase(PHASE_INTRO) || events.IsInPhase(PHASE_COMBAT))
                return;

            me->CastSpell(me, SPELL_SOE_ABSORPTION_SHIELD, true);
            
            if (player->GetQuestStatus(QUEST_SURRENDER_OR_ELSE) != QUEST_STATUS_INCOMPLETE)
                return;

            playerGUID = who->GetGUID();
            events.SetPhase(PHASE_INTRO);
            events.ScheduleEvent(EVENT_GENERIC_1, 300, 0, PHASE_INTRO);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->GetMotionMaster()->MovePoint(EVENT_GENERIC_1, 132.3455f,  1938.528f,  17.56664f);
            //events.ScheduleEvent(EVENT_GENERIC_2, 1000, 0, PHASE_INTRO);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            
            if (type != POINT_MOTION_TYPE || pointId != EVENT_GENERIC_1)
                return;
            
            if (Player* target = sObjectAccessor->GetPlayer(*me, playerGUID))
                me->SetFacingToObject(target);
            events.ScheduleEvent(EVENT_GENERIC_2, 1000, 0, PHASE_INTRO);
            me->CastSpell(me, SPELL_SOE_STRANGE_TENTACLE, true);
        }

        void UpdateAI(uint32 diff) override
        {
            if (events.IsInPhase(PHASE_COMBAT) && !UpdateVictim())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GENERIC_1:
                        me->CastSpell(me, SPELL_SOE_BEAM_EFFECT, true);
                        me->CastSpell(me, SPELL_SOE_BEAM_EFFECT, true);
                        me->CastSpell(me, SPELL_SOE_BEAM_EFFECT, true);
                        events.ScheduleEvent(EVENT_GENERIC_1, 300, 0, PHASE_INTRO);
                        break;
                    case EVENT_GENERIC_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
                        events.ScheduleEvent(EVENT_GENERIC_3, 3000, 0, PHASE_INTRO);
                        break;
                    case EVENT_GENERIC_3:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1);
                        events.ScheduleEvent(EVENT_GENERIC_4, 6000, 0, PHASE_INTRO);
                        break;
                    case EVENT_GENERIC_4:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                        events.ScheduleEvent(EVENT_GENERIC_5, 6000);
                        break;
                    case EVENT_GENERIC_5:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, playerGUID);
                        events.SetPhase(PHASE_COMBAT);
                        if (Player* target = sObjectAccessor->GetPlayer(*me, playerGUID))
                        {
                            events.ScheduleEvent(EVENT_GENERIC_6, 6000, 0, PHASE_COMBAT);
                            me->RemoveAura(SPELL_SOE_ABSORPTION_SHIELD);
                            me->RemoveAura(SPELL_SOE_STRANGE_TENTACLE);
                            me->RemoveAura(SPELL_SOE_FREEZE_ANIM);
                            me->GetMotionMaster()->MoveJump(167.6672f, 1944.108f, 5.213703f, 10, 15);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        }else
                            EnterEvadeMode();
                        break;
                    case EVENT_GENERIC_6:
                        events.ScheduleEvent(EVENT_GENERIC_6, 6000, 0, PHASE_COMBAT);
                        if (Player* target = sObjectAccessor->GetPlayer(*me, playerGUID))
                            me->CastSpell(target, SPELL_SHADOW_CRASH, false);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_faceless_of_the_deepAI(creature);
    }
};

#define MAX_OMLOT_WP_TICK    15
class npc_omlot_warrior : public CreatureScript
{
public:
    npc_omlot_warrior() : CreatureScript("npc_omlot_warrior") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_omlot_warriorAI (creature);
    }

    struct npc_omlot_warriorAI : public npc_escortAI
    {
        npc_omlot_warriorAI(Creature* creature) : npc_escortAI(creature) {}
        EventMap events;

        void Reset() override
        {
            events.Reset();
            me->setActive(true);
            Start(false, false);
            if (me->GetPositionY() < 2285.0f)
            {
                switch(me->GetGUID().GetCounter()%3)
                {
                    case 0:
                        SetNextWaypoint(urand(0, 15), true, false);
                        break;
                    case 1:
                        SetNextWaypoint(urand(26, 41), true, false);
                        break;
                    default:
                        SetNextWaypoint(urand(51, 66), true, false);
                        break;
                }
                
            }
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            events.ScheduleEvent(EVENT_GENERIC_1, urand(20000, 40000));
        }    

        void WaypointReached(uint32 pointId) override
        {
            switch(pointId)
            {
                case 25:
                case 50:
                case 78:
                    SetEscortPaused(true);
                    break;
                default:
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker->GetTypeId() != TYPEID_PLAYER)
                damage *= 100;
        }

        void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
        { 
            damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                events.ScheduleEvent(EVENT_GENERIC_1, 25000);
                
                if (Unit* victim = me->getVictim())
                {
                    if (me->GetPositionY() < 2340.0f && victim->GetTypeId() != TYPEID_PLAYER)
                    {
                        victim->DealDamage(me, me->GetHealth(), NULL, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        return;
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};


class npc_goblin_captive : public CreatureScript
{
public:
    npc_goblin_captive() : CreatureScript("npc_goblin_captive") {}

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_goblin_captiveAI (creature);
    }

    struct npc_goblin_captiveAI : public npc_escortAI
    {
        npc_goblin_captiveAI(Creature* creature) : npc_escortAI(creature) {}
        EventMap events;
        ObjectGuid sharMan;
        void Reset() override
        {
            sharMan.Clear();

            events.Reset();

            Position pos;
            me->GetNearPosition(pos, 4.0f, 0.0f);

            if (TempSummon* summon = me->SummonCreature(NPC_OOMLOT_SHAMAN, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
            {
                sharMan = summon->GetGUID();
                summon->CastSpell(me, SPELL_ENVELOPING_WINDS, true);
            }
        }    

        void SummonedCreatureDespawn(Creature* /*summon*/) override
        {
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
            Start(false, true);
            me->RemoveAura(SPELL_ENVELOPING_WINDS_CAPTURED); 
        }

        void WaypointReached(uint32 pointId) override
        {
        }
    };
};

class npc_super_booster_rocket_boots : public CreatureScript
{
    public:
        npc_super_booster_rocket_boots() : CreatureScript("npc_super_booster_rocket_boots") { }

    struct npc_super_booster_rocket_bootsAI : public ScriptedAI
    {
        npc_super_booster_rocket_bootsAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {

        }

        void OnCharmed(bool /*apply*/) override
        {
        }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            if (passenger->GetTypeId() != TYPEID_PLAYER)
                return;

            if (apply)
                passenger->CastSpell(passenger, SPELL_ZVSBRB_DAMAGE_AURA, true);
            else
                passenger->RemoveAura(SPELL_ZVSBRB_DAMAGE_AURA); 
        }

        void UpdateAI(uint32 diff) override
        {
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_super_booster_rocket_bootsAI(creature);
    }
};

class npc_sassy_hardwrench : public CreatureScript
{
    public:
        npc_sassy_hardwrench() : CreatureScript("npc_sassy_hardwrench") { }

    struct npc_sassy_hardwrenchAI : public ScriptedAI
    {
        npc_sassy_hardwrenchAI(Creature* creature) : ScriptedAI(creature)
        {

        }

        GuidSet m_player_for_event;
        void Reset() override
        {
            m_player_for_event.clear();
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->GetTypeId() != TYPEID_PLAYER)
                return;

            GuidSet::iterator itr = m_player_for_event.find(who->GetGUID());
            if (itr != m_player_for_event.end())
                return;
            m_player_for_event.insert(who->GetGUID());
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, who->GetGUID());
        }

        void OnStartQuest(Player* player, Quest const* quest) override
        {
            if (!quest || quest->GetQuestId() != QUEST_OLD_FRIENDS)
                return;

            //WorldLocation loc(648, 1580.93f, 2720.48f, 83.2326f);
            //player->SetHomebind(loc, 4912);
            me->CastSpell(player, SPELL_OF_BIND, true);
            player->PlayDistanceSound(MUSIC_VOLCANO, player);
            player->CastSpell(player, SPELL_OF_SUMMON_BOMBER, true);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sassy_hardwrenchAI(creature);
    }
};

class npc_flying_bomber : public CreatureScript
{
public:
    npc_flying_bomber() : CreatureScript("npc_flying_bomber") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_flying_bomberAI (creature);
    }

    struct npc_flying_bomberAI : public npc_escortAI
    {
        npc_flying_bomberAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;
        void Reset() override
        {
             PlayerOn       = false;
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
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            Vehicle*  v = me->GetVehicleKit();
            if (!v)
                return;

            Unit* sassy = v->GetPassenger(0);
            if (!sassy)
                return;

            switch(i)
            {
                case 1:
                    sCreatureTextMgr->SendChat(sassy->ToCreature(), TEXT_GENERIC_0, player->GetGUID());
                    break;
                case 3:
                    sCreatureTextMgr->SendChat(sassy->ToCreature(), TEXT_GENERIC_1, player->GetGUID());
                    break;
                case 25 :
                    SetEscortPaused(true);
                    player->ExitVehicle();
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            
            if (PlayerOn)
            {
                if (Player* player = GetPlayerForEscort())
                    player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};
// Kaja'Cola Gives You IDEAS! (TM): Dummy To Assistant Greely
class spell_kcgyi_assistant_greely : public SpellScriptLoader
{
public:
    spell_kcgyi_assistant_greely() : SpellScriptLoader("spell_kcgyi_assistant_greely") { }

    class spell_kcgyi_assistant_greely_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kcgyi_assistant_greely_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit* caster =  GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (caster->GetTypeId() == TYPEID_PLAYER &&
                caster->ToPlayer()->GetQuestStatus(QUEST_COLA_GIVE_YOU_IDEAS) != QUEST_STATUS_REWARDED)
                return;

            Unit* target = GetHitUnit();
            if (!target)
                return;

            std::list<Creature*> Minions;
            caster->GetAllMinionsByEntry(Minions, NPC_KCGYI_GREELY);
            if (Minions.empty())
                caster->CastSpell(caster, SPELL_KCGYI_SUMMON_GREELY, true);            
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_kcgyi_assistant_greely_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_kcgyi_assistant_greely_SpellScript();
    }
};

//Kaja'Cola Zero-One
class spell_cola_zero_one : public SpellScriptLoader
{
public:
    spell_cola_zero_one() : SpellScriptLoader("spell_cola_zero_one") { }

    class spell_cola_zero_one_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_cola_zero_one_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit* caster =  GetCaster();
            if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                return;

            //if (caster->GetTypeId() == TYPEID_PLAYER &&
            //    caster->ToPlayer()->GetQuestStatus(QUEST_MORALE_BOOST) != QUEST_STATUS_REWARDED)
            //    return;

            Unit* target = GetHitUnit();
            if (!target || !target->ToCreature())
                return;

            switch(target->GetEntry())
            {
                case NPC_MB_KEZAN_CITIZEN:
                case NPC_MB_KEZAN_CITIZEN2:
                    sCreatureTextMgr->SendChat(target->ToCreature(), TEXT_GENERIC_0, caster->GetGUID());
                    caster->ToPlayer()->KilledMonsterCredit(NPC_MB_KEZAN_CITIZEN2);
                    target->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                    target->GetMotionMaster()->MoveFollow(caster, 1.0f, 0);
                    target->ToCreature()->DespawnOrUnsummon(5000);
                    break;
                case NPC_MB_IZI:
                    caster->CastSpell(caster, SPELL_FM_IZZY_FREED, true); 
                    caster->ToPlayer()->KilledMonsterCredit(NPC_MB_IZI);
                    target->ToCreature()->DespawnOrUnsummon(0);
                    break;
                case NPC_MB_GOOBER:
                    caster->CastSpell(caster, SPELL_FM_GOOBER_FREED, true); 
                    caster->ToPlayer()->KilledMonsterCredit(NPC_MB_GOOBER);
                    target->ToCreature()->DespawnOrUnsummon(0);
                    break;
                case NPC_MB_ACE:
                    caster->CastSpell(caster, SPELL_FM_ACE_FREED, true); 
                    caster->ToPlayer()->KilledMonsterCredit(NPC_MB_ACE);
                    target->ToCreature()->DespawnOrUnsummon(0);
                    break;
            }         
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_cola_zero_one_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_cola_zero_one_SpellScript();
    }
};

class npc_footbomb_uniform : public CreatureScript
{
public:
    npc_footbomb_uniform() : CreatureScript("npc_footbomb_uniform") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_footbomb_uniformAI (creature);
    }

    struct npc_footbomb_uniformAI : public npc_escortAI
    {
        npc_footbomb_uniformAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;
        void Reset() override
        {
             PlayerOn       = false;
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
            if (i == 12)
            {
                if (Player* player = GetPlayerForEscort())
                {
                    SetEscortPaused(true);
                    player->ExitVehicle();
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            
            if (PlayerOn)
            {
                if (Player* player = GetPlayerForEscort())
                    player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};

class npc_captured_goblin : public CreatureScript
{
    public:
        npc_captured_goblin() : CreatureScript("npc_captured_goblin") { }

    struct npc_captured_goblinAI : public Scripted_NoMovementAI
    {
        npc_captured_goblinAI(Creature* creature) : Scripted_NoMovementAI(creature) { }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }
        
        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (caster->GetTypeId() != TYPEID_PLAYER || spell->Id != SPELL_EC_ON_INTERACT)
                return;

            Player *player = caster->ToPlayer();

            if (player->GetQuestStatus(QUEST_ESCAPE_VELOCITY) != QUEST_STATUS_INCOMPLETE)
                return;

            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0, caster->GetGUID());
            me->CastSpell(me, SPELL_EC_ROCKETS, true);
            player->KilledMonsterCredit(me->GetEntry());
            events.ScheduleEvent(EVENT_GENERIC_1, 3000);
        }

        void MovementInform(uint32 moveType, uint32 pointId) override
        {
            if (pointId == EVENT_POINT_MINE)
                me->DespawnOrUnsummon(0);
        }

        void UpdateAI(uint32 diff) override
        {

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
                me->GetMotionMaster()->MovePoint(EVENT_POINT_MINE, me->m_positionX, me->m_positionY, me->m_positionZ+200.0f);
            // SMSG_SET_PLAY_HOVER_ANIM
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_captured_goblinAI(creature);
    }
};

enum princePhases
{
    PHASE_PRINCE_COMBAT = 1,
    PHASE_PRINCE_FINAL,
};

const uint32 prince_spells[5] = {81000, 74003, 74005, 74000, 74004};
class npc_trade_prince_gallywix_final : public CreatureScript
{
    public:
        npc_trade_prince_gallywix_final() : CreatureScript("npc_trade_prince_gallywix_final") { }

    struct npc_trade_prince_gallywix_finalAI : public ScriptedAI
    {
        npc_trade_prince_gallywix_finalAI(Creature* creature) : ScriptedAI(creature){}
        EventMap events;
        ObjectGuid playerGUID;
        void Reset() override
        {
            playerGUID.Clear();
            events.Reset();
            me->SetCreateHealth(98260);
            me->SetMaxHealth(98260);
            me->setFaction(14);
        }

        void EnterCombat(Unit* victim) override
        {
            if (events.GetPhaseMask())
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
            else
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_0);
            events.ScheduleEvent(EVENT_GENERIC_5, 3000, 0, PHASE_PRINCE_COMBAT);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            Player *player = who->ToPlayer();
            if (!player || events.IsInPhase(PHASE_PRINCE_COMBAT))
                return;
            
            if (player->GetQuestStatus(QUEST_FINAL_CONFRONTATION) != QUEST_STATUS_INCOMPLETE)
                return;

            events.SetPhase(PHASE_PRINCE_COMBAT);
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
            playerGUID = who->GetGUID();
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            // God mode.
            if (!events.IsInPhase(PHASE_PRINCE_COMBAT))
            {
                damage = 0;
                return;
            }

            if (damage >= me->GetHealth())
            {
                damage = 0;
                events.SetPhase(PHASE_PRINCE_FINAL);
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4);
                events.ScheduleEvent(EVENT_GENERIC_1, 3000, 0, PHASE_PRINCE_FINAL);
                if (Player* target = sObjectAccessor->GetPlayer(*me, playerGUID))
                    target->KilledMonsterCredit(me->GetEntry());
                me->setFaction(2239);
            }                
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType /*damageType*/) override
        {
            damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (events.IsInPhase(PHASE_PRINCE_COMBAT) && !UpdateVictim())
                return;

            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GENERIC_1:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5);
                        events.ScheduleEvent(EVENT_GENERIC_2, 3000, 0, PHASE_PRINCE_FINAL);
                        break;
                    case EVENT_GENERIC_2:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_6);
                        events.ScheduleEvent(EVENT_GENERIC_3, 3000, 0, PHASE_PRINCE_FINAL);
                        break;
                    case EVENT_GENERIC_3:
                        // trall say miss text
                        //sCreatureTextMgr->SendChat(trall, TEXT_GENERIC_1);
                        events.ScheduleEvent(EVENT_GENERIC_4, 6000, 0, PHASE_PRINCE_FINAL);
                        me->DespawnOrUnsummon(1000);
                        break;
                    case EVENT_GENERIC_4:
                        // trall say miss text
                        //sCreatureTextMgr->SendChat(trall, TEXT_GENERIC_2);
                        break;
                    case EVENT_GENERIC_5:
                        sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                        if (Unit * target = me->getVictim())
                            me->CastSpell(target, prince_spells[urand(0, 4)], false);
                        events.ScheduleEvent(EVENT_GENERIC_5, 4000, 0, PHASE_PRINCE_COMBAT);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_trade_prince_gallywix_finalAI(creature);
    }
};

void AddSC_lost_isle()
{
    new npc_gizmo();
    new npc_doc_zapnnozzle();
    new npc_foreman_dampwick();
    new npc_frightened_miner();
    new spell_photo_capturing();
    new spell_ctu_snap_effect();
    new npc_orc_scout();
    new npc_strangle_vine();
    new spell_weed_whacker();
    new npc_bastia();
    new npc_gyrochoppa();
    new spell_mmut_phase_controller();
    new spell_trall_chain_lightning();
    new npc_cyclone_of_the_elements();
    new npc_sling_rocket();
    new npc_wild_clucker();
    new npc_wild_clucker_egg();
    new npc_hobart_grapplehammer();
    new npc_vashjelan_siren();
    new npc_naga_hatchling();
    new npc_faceless_of_the_deep();
    new npc_omlot_warrior();
    new npc_goblin_captive();
    new npc_super_booster_rocket_boots();
    new npc_sassy_hardwrench();
    new npc_flying_bomber();
    new spell_kcgyi_assistant_greely();
    new spell_cola_zero_one();
    new npc_footbomb_uniform();
    new npc_captured_goblin();
    //new npc_trade_prince_gallywix_final();
}