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
#include "ScriptedEscortAI.h"

//How to win friends and influence enemies
// texts signed for creature 28939 but used for 28939, 28940, 28610
enum win_friends
{
    SAY_PERSUADE1                     = 1,
    SAY_PERSUADE2                     = 2,
    SAY_PERSUADE3                     = 3,
    SAY_PERSUADE4                     = 4,
    SAY_PERSUADE5                     = 5,
    SAY_PERSUADE6                     = 6,
    SAY_PERSUADE7                     = 7,
    SAY_CRUSADER1                     = 8,
    SAY_CRUSADER2                     = 9,
    SAY_CRUSADER3                     = 10,
    SAY_CRUSADER4                     = 11,
    SAY_CRUSADER5                     = 12,
    SAY_CRUSADER6                     = 13,
    SAY_PERSUADED1                    = 14,
    SAY_PERSUADED2                    = 15,
    SAY_PERSUADED3                    = 16,
    SAY_PERSUADED4                    = 17,
    SAY_PERSUADED5                    = 18,
    SAY_PERSUADED6                    = 19,
    SPELL_PERSUASIVE_STRIKE           = 52781
};

class npc_crusade_persuaded : public CreatureScript
{
public:
    npc_crusade_persuaded() : CreatureScript("npc_crusade_persuaded") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crusade_persuadedAI (creature);
    }

    struct npc_crusade_persuadedAI : public ScriptedAI
    {
        npc_crusade_persuadedAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiSpeech_timer;
        uint32 uiSpeech_counter;
        ObjectGuid uiPlayerGUID;

        void Reset()
        {
            uiSpeech_timer = 0;
            uiSpeech_counter = 0;
            uiPlayerGUID.Clear();
            me->SetReactState(REACT_AGGRESSIVE);
            me->RestoreFaction();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_PERSUASIVE_STRIKE && caster->IsPlayer() && me->isAlive() && !uiSpeech_counter)
            {
                if (CAST_PLR(caster)->GetQuestStatus(12720) == QUEST_STATUS_INCOMPLETE)
                {
                    if (roll_chance_i(5))
                    {
                        uiPlayerGUID = caster->GetGUID();
                        uiSpeech_timer = 1000;
                        uiSpeech_counter = 1;
                        me->setFaction(caster->getFaction());
                        me->CombatStop(true);
                        me->GetMotionMaster()->MoveIdle();
                        me->SetReactState(REACT_PASSIVE);
                        DoCastAOE(58111, true);
                    }
                    else if (roll_chance_i(20))
                    {
                        Talk(RAND(SAY_PERSUADE1, SAY_PERSUADE2, SAY_PERSUADE3,
                                          SAY_PERSUADE4, SAY_PERSUADE5, SAY_PERSUADE6,
                                          SAY_PERSUADE7));
                    }
                    else if (roll_chance_i(40)) 
                    {
                        Talk(RAND(SAY_CRUSADER1, SAY_CRUSADER2, SAY_CRUSADER3,
                                          SAY_CRUSADER4, SAY_CRUSADER5, SAY_CRUSADER6));
                    }
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (uiSpeech_counter)
            {
                if (uiSpeech_timer <= diff)
                {
                    Player* player = Unit::GetPlayer(*me, uiPlayerGUID);
                    if (!player)
                    {
                        EnterEvadeMode();
                        return;
                    }

                    switch (uiSpeech_counter)
                    {
                        case 1:
                            Talk(SAY_PERSUADED1);
                            uiSpeech_timer = 8000;
                            break;

                        case 2:
                            Talk(SAY_PERSUADED2);
                            uiSpeech_timer = 8000;
                            break;

                        case 3:
                            Talk(SAY_PERSUADED3);
                            uiSpeech_timer = 8000;
                            break;

                        case 4:
                            Talk(SAY_PERSUADED4);
                            uiSpeech_timer = 8000;
                            break;

                        case 5:
                            Talk(SAY_PERSUADED5);
                            uiSpeech_timer = 8000;
                            break;

                        case 6:
                            Talk(SAY_PERSUADED6);
                            player->Kill(me);
                            //me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            //me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            uiSpeech_counter = 0;
                            player->GroupEventHappens(12720, me);
                            return;
                    }

                    ++uiSpeech_counter;
                    DoCast(me, 58111, true);
                }
                else
                    uiSpeech_timer -= diff;

                return;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

};

/*######
## npc_koltira_deathweaver
######*/

enum Koltira
{
    SAY_BREAKOUT1                   = 1,
    SAY_BREAKOUT2                   = 2,
    SAY_BREAKOUT3                   = 3,
    SAY_BREAKOUT4                   = 4,
    SAY_BREAKOUT5                   = 5,
    SAY_BREAKOUT6                   = 6,
    SAY_BREAKOUT7                   = 7,
    SAY_BREAKOUT8                   = 8,
    SAY_BREAKOUT9                   = 9,
    SAY_BREAKOUT10                  = 10,

    SPELL_KOLTIRA_TRANSFORM         = 52899,
    SPELL_ANTI_MAGIC_ZONE           = 52894,

    QUEST_BREAKOUT                  = 12727,

    NPC_CRIMSON_ACOLYTE             = 29007,
    NPC_HIGH_INQUISITOR_VALROTH     = 29001,


    //not sure about this id
    //NPC_DEATH_KNIGHT_MOUNT        = 29201,
    MODEL_DEATH_KNIGHT_MOUNT        = 25278
};

class npc_koltira_deathweaver : public CreatureScript
{
public:
    npc_koltira_deathweaver() : CreatureScript("npc_koltira_deathweaver") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest* quest)
    {
        if (quest->GetQuestId() == QUEST_BREAKOUT)
        {
            creature->SetStandState(UNIT_STAND_STATE_STAND);

            if (npc_escortAI* escortAI = CAST_AI(npc_koltira_deathweaver::npc_koltira_deathweaverAI, creature->AI()))
                escortAI->Start(false, false, player->GetGUID());
        }
        return true;
    }

    struct npc_koltira_deathweaverAI : public npc_escortAI
    {
        npc_koltira_deathweaverAI(Creature* creature) : npc_escortAI(creature)
        {
            me->SetReactState(REACT_DEFENSIVE);
        }

        void Reset()
        {
            if (!HasEscortState(STATE_ESCORT_ESCORTING))
            {
                wave = 0;
                waveTimer = 3000;
                valrothGUID.Clear();
                me->LoadEquipment(0, true);
                me->RemoveAurasDueToSpell(SPELL_ANTI_MAGIC_ZONE);
                me->RemoveAurasDueToSpell(SPELL_KOLTIRA_TRANSFORM);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }

        void WaypointReached(uint32 waypointId)
        {
            switch (waypointId)
            {
                case 0:
                    Talk(SAY_BREAKOUT1);
                    break;
                case 1:
                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    break;
                case 2:
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    DoCast(me, SPELL_KOLTIRA_TRANSFORM);
                    me->LoadEquipment();
                    break;
                case 3:
                    SetEscortPaused(true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    Talk(SAY_BREAKOUT2);
                    DoCast(me, SPELL_ANTI_MAGIC_ZONE);
                    break;
                case 4:
                    SetRun(true);
                    break;
                case 9:
                    me->Mount(MODEL_DEATH_KNIGHT_MOUNT);
                    break;
                case 10:
                    me->Dismount();
                    break;
            }
        }

        void JustSummoned(Creature* summoned)
        {
            if (Player* player = GetPlayerForEscort())
                summoned->AI()->AttackStart(player);

            if (summoned->GetEntry() == NPC_HIGH_INQUISITOR_VALROTH)
                valrothGUID = summoned->GetGUID();

            summoned->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        void SummonAcolyte(uint32 uiAmount)
        {
            for (uint32 i = 0; i < uiAmount; ++i)
                me->SummonCreature(NPC_CRIMSON_ACOLYTE, 1642.329f, -6045.818f, 127.583f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
        }

        void UpdateAI(uint32 uiDiff)
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (waveTimer <= uiDiff)
                {
                    switch (wave)
                    {
                        case 0:
                            Talk(SAY_BREAKOUT3);
                            SummonAcolyte(3);
                            waveTimer = 20000;
                            break;
                        case 1:
                            Talk(SAY_BREAKOUT4);
                            SummonAcolyte(3);
                            waveTimer = 20000;
                            break;
                        case 2:
                            Talk(SAY_BREAKOUT5);
                            SummonAcolyte(4);
                            waveTimer = 20000;
                            break;
                        case 3:
                            Talk(SAY_BREAKOUT6);
                            me->SummonCreature(NPC_HIGH_INQUISITOR_VALROTH, 1642.329f, -6045.818f, 127.583f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);
                            waveTimer = 1000;
                            break;
                        case 4:
                        {
                            Creature* temp = Unit::GetCreature(*me, valrothGUID);

                            if (!temp || !temp->isAlive())
                            {
                                Talk(SAY_BREAKOUT8);
                                waveTimer = 5000;
                            }
                            else
                            {
                                waveTimer = 2500;
                                return;
                            }
                            break;
                        }
                        case 5:
                            Talk(SAY_BREAKOUT9);
                            me->RemoveAurasDueToSpell(SPELL_ANTI_MAGIC_ZONE);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            waveTimer = 2500;
                            break;
                        case 6:
                            Talk(SAY_BREAKOUT10);
                            SetEscortPaused(false);
                            break;
                    }

                    ++wave;
                }
                else
                    waveTimer -= uiDiff;
            }
        }
        
    private:
        uint8 wave;
        uint32 waveTimer;
        ObjectGuid valrothGUID;

    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_koltira_deathweaverAI(creature);
    }
};
//Scarlet courier
enum ScarletCourierEnum
{
    SAY_TREE1                          = 0,
    SAY_TREE2                          = 1,
    SPELL_SHOOT                        = 52818,
    GO_INCONSPICUOUS_TREE              = 191144,
    NPC_SCARLET_COURIER                = 29076
};

class mob_scarlet_courier : public CreatureScript
{
public:
    mob_scarlet_courier() : CreatureScript("mob_scarlet_courier") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_scarlet_courierAI (creature);
    }

    struct mob_scarlet_courierAI : public ScriptedAI
    {
        mob_scarlet_courierAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiStage;
        uint32 uiStage_timer;

        void Reset()
        {
            me->Mount(14338); // not sure about this id
            uiStage = 1;
            uiStage_timer = 3000;
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_TREE2);
            me->Dismount();
            uiStage = 0;
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
                uiStage = 2;
        }

        void UpdateAI(uint32 diff)
        {
            if (uiStage && !me->isInCombat())
            {
                if (uiStage_timer <= diff)
                {
                    switch (uiStage)
                    {
                    case 1:
                        me->SetWalk(true);
                        if (GameObject* tree = me->FindNearestGameObject(GO_INCONSPICUOUS_TREE, 40.0f))
                        {
                            Talk(SAY_TREE1);
                            float x, y, z;
                            tree->GetContactPoint(me, x, y, z);
                            me->GetMotionMaster()->MovePoint(1, x, y, z);
                        }
                        break;
                    case 2:
                        if (GameObject* tree = me->FindNearestGameObject(GO_INCONSPICUOUS_TREE, 40.0f))
                            if (Unit* unit = tree->GetOwner())
                                AttackStart(unit);
                        break;
                    }
                    uiStage_timer = 3000;
                    uiStage = 0;
                } else uiStage_timer -= diff;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

};

//Koltira & Valroth- Breakout

enum valroth
{
    SAY_VALROTH1                      = 4,
    SAY_VALROTH2                      = 5,
    SAY_VALROTH3                      = 3,
    SAY_VALROTH4                      = 7,
    SAY_VALROTH5                      = 2,
    SAY_VALROTH6                      = 6,
    SPELL_RENEW                       = 38210,
    SPELL_INQUISITOR_PENANCE          = 52922,
    SPELL_VALROTH_SMITE               = 52926,
    SPELL_SUMMON_VALROTH_REMAINS      = 52929
};

class mob_high_inquisitor_valroth : public CreatureScript
{
public:
    mob_high_inquisitor_valroth() : CreatureScript("mob_high_inquisitor_valroth") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_high_inquisitor_valrothAI (creature);
    }

    struct mob_high_inquisitor_valrothAI : public ScriptedAI
    {
        mob_high_inquisitor_valrothAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiRenew_timer;
        uint32 uiInquisitor_Penance_timer;
        uint32 uiValroth_Smite_timer;

        void Reset() override
        {
            uiRenew_timer = 1000;
            uiInquisitor_Penance_timer = 2000;
            uiValroth_Smite_timer = 1000;
        }

        void EnterCombat(Unit* who) override
        {
            Talk(SAY_VALROTH2);
            DoCast(who, SPELL_VALROTH_SMITE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (uiRenew_timer <= diff)
            {
                Shout();
                DoCast(me, SPELL_RENEW, false);
                uiRenew_timer = urand(1000, 6000);
            }
            else
                uiRenew_timer -= diff;

            if (uiInquisitor_Penance_timer <= diff)
            {
                Shout();

                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_INQUISITOR_PENANCE, false);

                uiInquisitor_Penance_timer = urand(2000, 7000);
            }
            else
                uiInquisitor_Penance_timer -= diff;

            if (uiValroth_Smite_timer <= diff)
            {
                Shout();

                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_VALROTH_SMITE);

                uiValroth_Smite_timer = urand(1000, 6000);
            }
            else
                uiValroth_Smite_timer -= diff;

            DoMeleeAttackIfReady();
        }

        void Shout()
        {
            if (rand()%100 < 15)
                Talk(RAND(SAY_VALROTH3, SAY_VALROTH4, SAY_VALROTH5));
        }

        void JustDied(Unit* killer) override
        {
            Talk(SAY_VALROTH6);
            killer->CastSpell(me, SPELL_SUMMON_VALROTH_REMAINS, true);
        }
    };

};

/*######
## npc_a_special_surprise
######*/
//used by 29032, 29061, 29065, 29067, 29068, 29070, 29074, 29072, 29073, 29071 but signed for 29032
enum SpecialSurprise
{
    SAY_EXEC_START_1            = 0,                 // speech for all
    SAY_EXEC_START_2            = 1,
    SAY_EXEC_START_3            = 2,
    SAY_EXEC_PROG_1             = 3,
    SAY_EXEC_PROG_2             = 4,
    SAY_EXEC_PROG_3             = 5,
    SAY_EXEC_PROG_4             = 6,
    SAY_EXEC_PROG_5             = 7,
    SAY_EXEC_PROG_6             = 8,
    SAY_EXEC_PROG_7             = 9,
    SAY_EXEC_NAME_1             = 10,
    SAY_EXEC_NAME_2             = 11,
    SAY_EXEC_RECOG_1            = 12,
    SAY_EXEC_RECOG_2            = 13,
    SAY_EXEC_RECOG_3            = 14,
    SAY_EXEC_RECOG_4            = 15,
    SAY_EXEC_RECOG_5            = 16,
    SAY_EXEC_RECOG_6            = 17,
    SAY_EXEC_NOREM_1            = 18,
    SAY_EXEC_NOREM_2            = 19,
    SAY_EXEC_NOREM_3            = 20,
    SAY_EXEC_NOREM_4            = 21,
    SAY_EXEC_NOREM_5            = 22,
    SAY_EXEC_NOREM_6            = 23,
    SAY_EXEC_NOREM_7            = 24,
    SAY_EXEC_NOREM_8            = 25,
    SAY_EXEC_NOREM_9            = 26,
    SAY_EXEC_THINK_1            = 27,
    SAY_EXEC_THINK_2            = 28,
    SAY_EXEC_THINK_3            = 29,
    SAY_EXEC_THINK_4            = 30,
    SAY_EXEC_THINK_5            = 31,
    SAY_EXEC_THINK_6            = 32,
    SAY_EXEC_THINK_7            = 33,
    SAY_EXEC_THINK_8            = 34,
    SAY_EXEC_THINK_9            = 35,
    SAY_EXEC_THINK_10           = 36,
    SAY_EXEC_LISTEN_1           = 37,
    SAY_EXEC_LISTEN_2           = 38,
    SAY_EXEC_LISTEN_3           = 39,
    SAY_EXEC_LISTEN_4           = 40,
    SAY_PLAGUEFIST              = 41,
    SAY_EXEC_TIME_1             = 42,
    SAY_EXEC_TIME_2             = 43,
    SAY_EXEC_TIME_3             = 44,
    SAY_EXEC_TIME_4             = 45,
    SAY_EXEC_TIME_5             = 46,
    SAY_EXEC_TIME_6             = 47,
    SAY_EXEC_TIME_7             = 48,
    SAY_EXEC_TIME_8             = 49,
    SAY_EXEC_TIME_9             = 50,
    SAY_EXEC_TIME_10            = 51,
    SAY_EXEC_WAITING            = 52,
    EMOTE_DIES                  = 53,

    NPC_PLAGUEFIST              = 29053
};

class npc_a_special_surprise : public CreatureScript
{
public:
    npc_a_special_surprise() : CreatureScript("npc_a_special_surprise") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_a_special_surpriseAI(creature);
    }

    struct npc_a_special_surpriseAI : public ScriptedAI
    {
        npc_a_special_surpriseAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 ExecuteSpeech_Timer;
        uint32 ExecuteSpeech_Counter;
        ObjectGuid PlayerGUID;

        void Reset() override
        {
            ExecuteSpeech_Timer = 0;
            ExecuteSpeech_Counter = 0;
            PlayerGUID.Clear();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        bool MeetQuestCondition(Player* player)
        {
            switch (me->GetEntry())
            {
                case 29061:                                     // Ellen Stanbridge
                    if (player->GetQuestStatus(12742) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29072:                                     // Kug Ironjaw
                    if (player->GetQuestStatus(12748) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29067:                                     // Donovan Pulfrost
                    if (player->GetQuestStatus(12744) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29065:                                     // Yazmina Oakenthorn
                    if (player->GetQuestStatus(12743) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29071:                                     // Antoine Brack
                    if (player->GetQuestStatus(12750) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29032:                                     // Malar Bravehorn
                    if (player->GetQuestStatus(12739) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29068:                                     // Goby Blastenheimer
                    if (player->GetQuestStatus(12745) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29073:                                     // Iggy Darktusk
                    if (player->GetQuestStatus(12749) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29074:                                     // Lady Eonys
                    if (player->GetQuestStatus(12747) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 29070:                                     // Valok the Righteous
                    if (player->GetQuestStatus(12746) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 49355:                                     // Seigneur Haford
                    if (player->GetQuestStatus(28649) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                case 49356:                                     // Gally Grossetache
                    if (player->GetQuestStatus(28650) == QUEST_STATUS_INCOMPLETE)
                        return true;
                    break;
                    
            }

            return false;
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (PlayerGUID || !who->IsPlayer() || !who->IsWithinDist(me, INTERACTION_DISTANCE))
                return;

            if (MeetQuestCondition(who->ToPlayer()))
                PlayerGUID = who->GetGUID();
        }

        void UpdateAI(uint32 diff) override
        {
            if (PlayerGUID && !me->getVictim() && me->isAlive())
            {
                if (ExecuteSpeech_Timer <= diff)
                {
                    Player* player = Unit::GetPlayer(*me, PlayerGUID);

                    if (!player)
                    {
                        Reset();
                        return;
                    }

                    //TODO: simplify text's selection

                    switch (player->getRace())
                    {
                        // TODO : exactly text for goblin & worgen
                        case RACE_GOBLIN:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_5, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_5, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_7, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_6, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_WORGEN:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_5, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_5, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_7, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_6, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_HUMAN:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_5, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_5, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_7, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_6, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_ORC:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_6, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_7, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_8, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_8, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_DWARF:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_2, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_2, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_3, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_2, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_5, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_2, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_3, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_NIGHTELF:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_5, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_6, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_2, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_7, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_UNDEAD_PLAYER:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_3, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_4, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_3, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_1, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_3, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_4, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_TAUREN:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_1, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_5, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_8, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_9, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_9, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_GNOME:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_4, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_4, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_6, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_5, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_TROLL:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_3, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_7, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_2, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_6, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_9, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_10, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_4, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_10, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_BLOODELF:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_1, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_1, player->GetGUID()); break;
                                //case 5: //unknown
                                case 6: Talk(SAY_EXEC_THINK_3, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_1, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                        case RACE_DRAENEI:
                            switch (ExecuteSpeech_Counter)
                            {
                                case 0: Talk(SAY_EXEC_START_1, player->GetGUID()); break;
                                case 1: me->SetStandState(UNIT_STAND_STATE_STAND); break;
                                case 2: Talk(SAY_EXEC_PROG_1, player->GetGUID()); break;
                                case 3: Talk(SAY_EXEC_NAME_1, player->GetGUID()); break;
                                case 4: Talk(SAY_EXEC_RECOG_2, player->GetGUID()); break;
                                case 5: Talk(SAY_EXEC_NOREM_1, player->GetGUID()); break;
                                case 6: Talk(SAY_EXEC_THINK_4, player->GetGUID()); break;
                                case 7: Talk(SAY_EXEC_LISTEN_1, player->GetGUID()); break;
                                case 8:
                                    if (Creature* Plaguefist = GetClosestCreatureWithEntry(me, NPC_PLAGUEFIST, 85.0f))
                                        Plaguefist->AI()->Talk(SAY_PLAGUEFIST, player->GetGUID());
                                    break;
                                case 9:
                                    Talk(SAY_EXEC_TIME_2, player->GetGUID());
                                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    break;
                                case 10:
                                    Talk(SAY_EXEC_WAITING, player->GetGUID());
                                    break;
                                case 11:
                                    Talk(EMOTE_DIES);
                                    me->setDeathState(JUST_DIED);
                                    me->SetHealth(0);
                                    return;
                            }
                            break;
                    }

                    if (ExecuteSpeech_Counter >= 9)
                        ExecuteSpeech_Timer = 15000;
                    else
                        ExecuteSpeech_Timer = 7000;

                    ++ExecuteSpeech_Counter;
                }
                else
                    ExecuteSpeech_Timer -= diff;
            }
        }
    };
};

void AddSC_the_scarlet_enclave_c2()
{
    new npc_crusade_persuaded();
    new mob_scarlet_courier();
    new npc_koltira_deathweaver();
    new mob_high_inquisitor_valroth();
    new npc_a_special_surprise();
}
