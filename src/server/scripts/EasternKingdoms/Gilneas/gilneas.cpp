/*
 * Copyright (C) 2011-2013 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
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

#include "Unit.h"
#include "gilneas.h"
#include "ScriptedEscortAI.h"
#include "Vehicle.h"
#include "CharmInfo.h"

//Phase 1
/*######
## npc_prince_liam_greymane_phase1
######*/

class npc_prince_liam_greymane_phase1 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase1() : CreatureScript("npc_prince_liam_greymane_phase1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_phase1AI (creature);
    }

    struct npc_prince_liam_greymane_phase1AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSay; //Time left for say
        uint32 cSay; //Current Say

        //Evade or Respawn
        void Reset() override
        {
            tSay = DELAY_SAY_PRINCE_LIAM_GREYMANE; //Reset timer
            cSay = 1; //Start from 1
        }

        //Timed events
        void UpdateAI(uint32 diff) override
        {
            //Out of combat
            if (!me->getVictim())
            {
                //Timed say
                if (tSay <= diff)
                {
                    switch (cSay)
                    {
                        default:
                        case 1:
                            Talk(SAY_PRINCE_LIAM_GREYMANE_1);
                            cSay++;
                            break;
                        case 2:
                            Talk(SAY_PRINCE_LIAM_GREYMANE_2);
                            cSay++;
                            break;
                        case 3:
                            Talk(SAY_PRINCE_LIAM_GREYMANE_3);
                            cSay = 1; //Reset to 1
                            break;
                    }

                    tSay = DELAY_SAY_PRINCE_LIAM_GREYMANE; //Reset the timer
                }
                else
                {
                    tSay -= diff;
                }
                return;
            }
        }
    };
};

/*######
## npc_gilneas_city_guard_phase1
######*/

class npc_gilneas_city_guard_phase1 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase1() : CreatureScript("npc_gilneas_city_guard_phase1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_phase1AI (creature);
    }

    struct npc_gilneas_city_guard_phase1AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSay; //Time left for say

        //Evade or Respawn
        void Reset() override
        {
            if (me->GetGUID().GetCounter() == 3486400)
                tSay = DELAY_SAY_GILNEAS_CITY_GUARD_GATE; //Reset timer
        }

        void UpdateAI(uint32 diff) override
        {
            //Out of combat and
            if (me->GetGUID().GetCounter() == 3486400)
            {
                //Timed say
                if (tSay <= diff)
                {
                    Talk(0);
                    tSay = DELAY_SAY_GILNEAS_CITY_GUARD_GATE; //Reset timer
                }
                else
                    tSay -= diff;
            }
        }
    };
};

//Phase 2
/*######
## npc_gilneas_city_guard_phase2
######*/

class npc_gilneas_city_guard_phase2 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase2() : CreatureScript("npc_gilneas_city_guard_phase2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_phase2AI (creature);
    }

    struct npc_gilneas_city_guard_phase2AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset() override
        {
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (who->IsPlayer())
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->GetEntry() == NPC_RAMPAGING_WORGEN_1 && me->HealthBelowPct(AI_MIN_HP))
                damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); //optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else
                tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_prince_liam_greymane_phase2
######*/

class npc_prince_liam_greymane_phase2 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase2() : CreatureScript("npc_prince_liam_greymane_phase2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_SOMETHINGS_AMISS || quest->GetQuestId() == QUEST_ALL_HELL_BREAKS_LOOSE || quest->GetQuestId() == QUEST_EVAC_MERC_SQUA)
        {
            if (creature->isQuestGiver())
            {
                player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
                player->PrepareGossipMenu(creature, 0 ,true);
                player->SendPreparedGossip(creature);
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_phase2AI (creature);
    }

    struct npc_prince_liam_greymane_phase2AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tYell, tSeek;
        bool doYell;

        void Reset() override
        {
            tSeek     = urand(1000, 2000);
            doYell    = true;
            tYell     = DELAY_YELL_PRINCE_LIAM_GREYMANE;
        }

        //There is NO phase shift here!!!!
        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->GetEntry() == NPC_RAMPAGING_WORGEN_1 && me->HealthBelowPct(AI_MIN_HP))
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            //If creature has no target
            if (!UpdateVictim())
            {
                if (tSeek <= diff)
                {
                    //Find worgen nearby
                    if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                        if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                            me->AI()->AttackStart(enemy);

                    tSeek = urand(1000, 2000);//optimize cpu load
                }
                else
                    tSeek -= diff;

                //Yell only once after Reset()
                if (doYell)
                {
                    //Yell Timer
                    if (tYell <= diff)
                    {
                        Talk(0);
                        doYell = false;
                    }
                    else
                        tYell -= diff;
                }
            }
            else
            {
                DoMeleeAttackIfReady();//Stop yell timer on combat
                doYell = false;
            }
        }
    };
};

/*######
## npc_gwen_armstead_p2
######*/

class npc_gwen_armstead_p2 : public CreatureScript
{
public:
    npc_gwen_armstead_p2() : CreatureScript("npc_gwen_armstead_p2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_ROYAL_ORDERS)
        {
            if (creature->isQuestGiver())
            {
                player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
                player->PrepareGossipMenu(creature, 0 ,true);
                player->SendPreparedGossip(creature);
            }
        }
        return true;
    }
};

/*######
## npc_rampaging_worgen
######*/

class npc_rampaging_worgen : public CreatureScript
{
public:
    npc_rampaging_worgen() : CreatureScript("npc_rampaging_worgen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_rampaging_worgenAI (pCreature);
    }

    struct npc_rampaging_worgenAI : public ScriptedAI
    {
        npc_rampaging_worgenAI(Creature *c) : ScriptedAI(c) {}

        uint32 tEnrage;
        uint32 dmgCount;
        uint32 tAnimate;
        uint32 tSound;
        bool playSound;

        void Reset() override
        {
            tEnrage = 0;
            dmgCount = 0;
            tAnimate = DELAY_ANIMATE;
            tSound = DELAY_SOUND;
            playSound = false;
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType /*damageType*/) override
        {
            if (target->IsUnit())
                dmgCount++;
        }

        void DamageTaken(Unit * pWho, uint32 &uiDamage, DamageEffectType /*dmgType*/) override
        {
            if (pWho->IsPlayer())
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 100000.0f);
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else
                uiDamage = 0;
        }

        void SpellHit(Unit* source, SpellInfo const* spell) override
        {
            if (source)
            {
                Player* player = source->ToPlayer();
                if(!player)
                    return;

                switch (spell->Id)
                {
                    case 589: 
                    case 172: 
                    case 100:
                    case 8921:
                    {
                        player->KilledMonsterCredit(44175, ObjectGuid::Empty);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (tEnrage <= diff)
            {
                if (me->GetHealthPct() <= 30)
                {
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else
                tEnrage -= diff;

            //play attack sound
            if (playSound == true) tSound -= diff;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_PLATE);
                tSound = DELAY_SOUND;
                playSound = false;
            }
            
            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
                dmgCount = 0;
            else if (me->getVictim()->isPet())
                dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                    tAnimate = DELAY_ANIMATE;
                    playSound = true;
                }
                else
                    tAnimate -= diff;
            }
        }
    };
};

/*######
## npc_rampaging_worgen2
######*/

class npc_rampaging_worgen2 : public CreatureScript
{
public:
    npc_rampaging_worgen2() : CreatureScript("npc_rampaging_worgen2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen2AI (creature);
    }

    struct npc_rampaging_worgen2AI : public ScriptedAI
    {
        npc_rampaging_worgen2AI(Creature* creature) : ScriptedAI(creature) {}

        uint16 tRun, tEnrage;
        bool onceRun, willCastEnrage;
        float x, y, z;

        void JustRespawned() override
        {
            tEnrage = 0;
            tRun = 500;
            onceRun = true;
            x = me->m_positionX + cos(me->GetOrientation())*8;
            y = me->m_positionY + sin(me->GetOrientation())*8;
            z = me->m_positionZ;
            willCastEnrage = urand(0, 1);
        }

        void UpdateAI(uint32 diff) override
        {
            if (tRun <= diff && onceRun)
            {
                me->GetMotionMaster()->MoveCharge(x, y, z, 8);
                onceRun = false;
            }
            else
                tRun -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff)
            {
                if (me->GetHealthPct() <= 30 && willCastEnrage)
                {
                    me->MonsterTextEmote(-106, ObjectGuid::Empty);
                    DoCast(me, SPELL_ENRAGE, false);
                    tEnrage = CD_ENRAGE;
                }
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## go_merchant_square_door
######*/

class go_merchant_square_door : public GameObjectScript
{
public:
    go_merchant_square_door() : GameObjectScript("go_merchant_square_door"), aPlayer(NULL) {}

    float x, y, z, wx, wy, angle, tQuestCredit;
    bool opened;
    uint8 spawnKind;
    Player* aPlayer;
    GameObject* go;
    uint32 DoorTimer;

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (player->GetQuestStatus(QUEST_EVAC_MERC_SQUA) == QUEST_STATUS_INCOMPLETE && go->GetGoState() == GO_STATE_READY)
        {
            aPlayer          = player;
            opened           = 1;
            tQuestCredit     = 2500;
            go->SetGoState(GO_STATE_ACTIVE);
            DoorTimer = DOOR_TIMER;
            spawnKind = urand(1, 3); //1, 2=citizen, 3=citizen&worgen (66%, 33%)
            angle = go->GetOrientation();
            x = go->GetPositionX()-cos(angle)*2;
            y = go->GetPositionY()-sin(angle)*2;
            z = go->GetPositionZ();
            wx = x-cos(angle)*2;
            wy = y-sin(angle)*2;

            if (spawnKind < 3)
            {
                if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_1, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                {
                    spawnedCreature->SetPhaseMask(6, 1);
                    spawnedCreature->Respawn(1);
                }
            }
            else
            {
                if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_2, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                {
                    spawnedCreature->SetPhaseMask(6, 1);
                    spawnedCreature->Respawn(1);
                }
            }
            return true;
        }
        return false;
    }

    void OnUpdate(GameObject* go, uint32 diff) override
    {
        if (opened == 1)
        {
            if (tQuestCredit <= ((float)diff/8))
            {
                opened = 0;
                
                if(aPlayer)
                    aPlayer->KilledMonsterCredit(35830, ObjectGuid::Empty);
                
                if (spawnKind == 3)
                {
                    if (Creature* spawnedCreature = go->SummonCreature(NPC_RAMPAGING_WORGEN_2, wx, wy, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                    {
                        spawnedCreature->SetPhaseMask(6, 1);
                        spawnedCreature->Respawn(1);
                        spawnedCreature->getThreatManager().resetAllAggro();
                        if(aPlayer)
                            aPlayer->AddThreat(spawnedCreature, 1.0f);
                        spawnedCreature->AddThreat(aPlayer, 1.0f);
                    }
                }
            }
            else tQuestCredit -= ((float)diff/8);
        }
        if (DoorTimer <= diff)
        {
            if (go->GetGoState() == GO_STATE_ACTIVE)
                go->SetGoState(GO_STATE_READY);

            DoorTimer = DOOR_TIMER;
        }
        else
            DoorTimer -= diff;
    }
};

/*######
## npc_frightened_citizen
######*/

struct Point
{
    float x, y;
};

struct WayPointID
{
    int pathID, pointID;
};

struct Paths
{
    uint8 pointsCount[8]; //pathID, pointsCount
    Point paths[8][10];   //pathID, pointID, Point
};

class npc_frightened_citizen : public CreatureScript
{
public:
    npc_frightened_citizen() : CreatureScript("npc_frightened_citizen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frightened_citizenAI (creature);
    }

    struct npc_frightened_citizenAI : public ScriptedAI
    {
        npc_frightened_citizenAI(Creature* creature) : ScriptedAI(creature) {}

        uint16 tRun, tRun2, tSay;
        bool onceRun, onceRun2, onceGet, onceSay;
        float x, y, z;
        WayPointID nearestPointID;
        Paths paths;

        Paths LoadPaths()
        {
            paths.pointsCount[0] = 7;
            paths.paths[0][0].x = -1544.83f; paths.paths[0][0].y = 1429.68f;
            paths.paths[0][1].x = -1554.44f; paths.paths[0][1].y = 1409.34f;
            paths.paths[0][2].x = -1554.34f; paths.paths[0][2].y = 1388.02f;
            paths.paths[0][3].x = -1557.97f; paths.paths[0][3].y = 1361.57f;
            paths.paths[0][4].x = -1560.59f; paths.paths[0][4].y = 1333.97f;
            paths.paths[0][5].x = -1568.32f; paths.paths[0][5].y = 1327.29f;
            paths.paths[0][6].x = -1577.35f; paths.paths[0][6].y = 1317.59f;

            paths.pointsCount[1] = 10;
            paths.paths[1][0].x = -1463.96f; paths.paths[1][0].y = 1429.41f;
            paths.paths[1][1].x = -1429.19f; paths.paths[1][1].y = 1422.41f;
            paths.paths[1][2].x = -1419.33f; paths.paths[1][2].y = 1419.23f;
            paths.paths[1][3].x = -1406.90f; paths.paths[1][3].y = 1416.03f;
            paths.paths[1][4].x = -1403.28f; paths.paths[1][4].y = 1401.21f;
            paths.paths[1][5].x = -1407.48f; paths.paths[1][5].y = 1375.36f;
            paths.paths[1][6].x = -1502.08f; paths.paths[1][6].y = 1342.68f;
            paths.paths[1][7].x = -1537.70f; paths.paths[1][7].y = 1330.3f;
            paths.paths[1][8].x = -1562.15f; paths.paths[1][8].y = 1319.15f;
            paths.paths[1][9].x = -1577.96f; paths.paths[1][9].y = 1316.79f;

            return paths;
        }

        void MultiDistanceMeter(Point *p, uint8 pointsCount, float *dist)
        {
            for (uint8 i = 0; i <= (pointsCount-1); i++)
                dist[i] = me->GetDistance2d(p[i].x, p[i].y);
        }

        WayPointID GetNearestPoint(Paths paths)
        {
            WayPointID nearestPointID;
            float dist[PATHS_COUNT][10], lowestDists[PATHS_COUNT];
            uint8 nearestPointsID[PATHS_COUNT], lowestDist;
            for (uint8 i = 0; i <= PATHS_COUNT-1; i++)
            {
                MultiDistanceMeter(paths.paths[i], paths.pointsCount[i], dist[i]);
                for (uint8 j = 0; j <= paths.pointsCount[i]-1; j++)
                {
                    if (j == 0)
                    {
                        lowestDists[i] = dist[i][j];
                        nearestPointsID[i] = j;
                    }
                    else if (lowestDists[i] > dist[i][j])
                    {
                        lowestDists[i] = dist[i][j];
                        nearestPointsID[i] = j;
                    }
                }
            }
            for (uint8 i = 0; i < PATHS_COUNT; i++)
            {
                if (i == 0)
                {
                    lowestDist = lowestDists[i];
                    nearestPointID.pointID = nearestPointsID[i];
                    nearestPointID.pathID = i;
                }
                else if (lowestDist > lowestDists[i])
                {
                    lowestDist = lowestDists[i];
                    nearestPointID.pointID = nearestPointsID[i];
                    nearestPointID.pathID = i;
                }
            }
            return nearestPointID;
        }

        void JustRespawned() override
        {
            paths          = LoadPaths();
            tRun           = 500;
            tRun2          = 2500;
            tSay           = 1000;
            onceRun = onceRun2 = onceSay = onceGet = true;
            x = me->m_positionX + cos(me->GetOrientation())*5;
            y = me->m_positionY + sin(me->GetOrientation())*5;
            z = me->m_positionZ;
        }

        void UpdateAI(uint32 diff) override
        {
            if (tRun <= diff && onceRun)
            {
                me->GetMotionMaster()->MoveCharge(x, y, z, 8);
                onceRun = false;
            }
            else
                tRun -= diff;

            if (tSay <= diff && onceSay)
            {
                Talk(0);
                onceSay = false;
            }
            else
                tSay -= diff;

            if (tRun2 <= diff)
            {
                if (onceGet)
                {
                    nearestPointID = GetNearestPoint(paths);
                    onceGet = false;
                }
                else
                {
                    if (me->GetDistance2d(paths.paths[nearestPointID.pathID][nearestPointID.pointID].x, paths.paths[nearestPointID.pathID][nearestPointID.pointID].y) > 1)
                        me->GetMotionMaster()->MoveCharge(paths.paths[nearestPointID.pathID][nearestPointID.pointID].x, paths.paths[nearestPointID.pathID][nearestPointID.pointID].y, z, 8);
                    else
                        nearestPointID.pointID ++;

                    if (nearestPointID.pointID >= paths.pointsCount[nearestPointID.pathID])
                        me->DespawnOrUnsummon();
                }
            }
            else
                tRun2 -= diff;
        }
    };
};

/*######
## npc_sergeant_cleese
######*/

class npc_sergeant_cleese : public CreatureScript
{
public:
    npc_sergeant_cleese() : CreatureScript("npc_sergeant_cleese") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sergeant_cleeseAI (creature);
    }

    struct npc_sergeant_cleeseAI : public ScriptedAI
    {
        npc_sergeant_cleeseAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset() override
        {
            tSeek   = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (who->IsPlayer())
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 10.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); //optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else
                tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_gilnean_royal_guard
######*/

class npc_gilnean_royal_guard : public CreatureScript
{
public:
    npc_gilnean_royal_guard() : CreatureScript("npc_gilnean_royal_guard") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_royal_guardAI (creature);
    }

    struct npc_gilnean_royal_guardAI : public ScriptedAI
    {
        npc_gilnean_royal_guardAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset() override
        {
            tSeek = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); //optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else
                tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_mariam_spellwalker
######*/

class npc_mariam_spellwalker : public CreatureScript
{
public:
    npc_mariam_spellwalker() : CreatureScript("npc_mariam_spellwalker") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mariam_spellwalkerAI (creature);
    }

    struct npc_mariam_spellwalkerAI : public ScriptedAI
    {
        npc_mariam_spellwalkerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset() override
        {
            tSeek = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 5.0f, true))
                        me->AI()->AttackStart(enemy); //She should really only grab agro when npc Cleese is not there, so we will keep this range small
                tSeek = urand(1000, 2000); //optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (me->getVictim()->GetEntry() == NPC_BLOODFANG_WORGEN)
                DoSpellAttackIfReady(SPELL_FROSTBOLT_VISUAL_ONLY); //Dummy spell, visual only to prevent getting agro (Blizz-like)
            else
                DoMeleeAttackIfReady();
        }
    };
};

/* QUEST - 14154 - By The Skin of His Teeth - START */

/*######
## npc_sean_dempsey
######*/

class npc_sean_dempsey : public CreatureScript
{
public:
    npc_sean_dempsey() : CreatureScript("npc_sean_dempsey") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sean_dempseyAI (creature);
    }

    struct npc_sean_dempseyAI : public ScriptedAI
    {
        npc_sean_dempseyAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSummon, tEvent_Timer, tWave_Time;
        bool EventActive, RunOnce;
        Player* player;

        void Reset() override
        {
            EventActive      = false;
            RunOnce          = true;
            tSummon          = 0;
            tEvent_Timer     = 0;
            tWave_Time       = urand(9000, 15000); // How often we spawn
        }

        void SummonNextWave()
        {
            if (!EventActive)
                return;
            else
            {
                if (RunOnce) // Our inital spawn should always be the same
                {
                    me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                    me->SummonCreature(NPC_WORGEN_ALPHA_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                    RunOnce = false;
                }
                else
                {
                    switch (urand (1,5)) // After intial wave, wave spawns should be random
                    {
                        case 1: // One Alpha on SW Roof and One Alpha on NW Roof
                            me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            me->SummonCreature(NPC_WORGEN_ALPHA_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;

                        case 2: // 8 Runts on NW Roof
                            for (int i = 0; i < 5; i++)
                                me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                                me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;

                        case 3: // 8 Runts on SW Roof
                            for (int i = 0; i < 5; i++)
                                me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                                me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;

                        case 4: // One Alpha on SW Roof and One Alpha on N Roof
                            me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            me->SummonCreature(NPC_WORGEN_ALPHA_C1, N_ROOF_SPAWN_LOC, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;
                        case 5: // 8 Runts - Half NW and Half SW
                            for (int i = 0; i < 5; i++)
                                me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                                me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;
                    }
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!EventActive)
                return;
            else
            {
                if (tEvent_Timer <= diff)
                {
                    EventActive = false;
                    tEvent_Timer = false;
                    return;
                }
                else // Event is still active
                {
                    tEvent_Timer -= diff;
                    if (tSummon <= diff) // Time for next spawn wave
                    {
                        SummonNextWave(); // Activate next spawn wave
                        tSummon = tWave_Time; // Reset our spawn timer
                    }
                    else
                        tSummon -= diff;
                }
            }
        }
    };
};

/*######
## npc_lord_darius_crowley_c1
######*/

class npc_lord_darius_crowley_c1 : public CreatureScript
{
public:
    npc_lord_darius_crowley_c1() : CreatureScript("npc_lord_darius_crowley_c1") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_BY_THE_SKIN_ON_HIS_TEETH)
        {
            creature->CastSpell(player, SPELL_BY_THE_SKIN_ON_HIS_TEETH, true);
            if (Creature* dempsey = GetClosestCreatureWithEntry(creature, NPC_SEAN_DEMPSEY, 100.0f))
            {
                CAST_AI(npc_sean_dempsey::npc_sean_dempseyAI, dempsey->AI())->EventActive = true; // Start Event
                CAST_AI(npc_sean_dempsey::npc_sean_dempseyAI, dempsey->AI())->tEvent_Timer = Event_Time; // Event lasts for 2 minutes - We'll stop spawning a few seconds short (Blizz-like)
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_darius_crowley_c1AI (creature);
    }

    struct npc_lord_darius_crowley_c1AI : public ScriptedAI
    {
        npc_lord_darius_crowley_c1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tAttack;

        void Reset() override
        {
            tAttack = urand(1700, 2400);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
            {
                // Reset home if no target
                me->GetMotionMaster()->MoveCharge(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), 8.0f);
                me->SetOrientation(me->GetHomePosition().GetOrientation()); // Reset to my original orientation
                return;
            }

            if (tAttack <= diff) // If we have a target, and it is time for our attack
            {
                if (me->IsWithinMeleeRange(me->getVictim()))
                {
                    switch (urand(0, 2)) // Perform one of 3 random attacks
                    {
                        case 0: // Do Left Hook
                            if (me->GetOrientation() > 2.0f && me->GetOrientation() < 3.0f || me->GetOrientation() > 5.0f && me->GetOrientation() < 6.0f) 
                            {
                                if (auto victim = me->getVictim())
                                    DoCast(victim, SPELL_LEFT_HOOK, true);
                            }
                                tAttack = urand(1700, 2400);
                            break;

                        case 1: // Do Demoralizing Shout
                            if (auto victim = me->getVictim())
                                DoCast(victim, SPELL_DEMORALIZING_SHOUT, true);
                            tAttack = urand(1700, 2400);
                            break;

                        case 2: // Do Snap Kick
                            if (auto victim = me->getVictim())
                                DoCast(victim, SPELL_SNAP_KICK, true);

                            tAttack = urand(1700, 2400);
                            break;
                    }
                }
                else
                {
                    if (auto victim = me->getVictim())
                        me->GetMotionMaster()->MoveChase(victim);
                }
            }
            else // If we have a target but our attack timer is still not ready, do regular attack
            {
                tAttack -= diff;
                DoMeleeAttackIfReady();
            }
        }
    };
};

/*######
## npc_worgen_runt_c1
######*/

class npc_worgen_runt_c1 : public CreatureScript
{
public:
    npc_worgen_runt_c1() : CreatureScript("npc_worgen_runt_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_c1AI (creature);
    }

    struct npc_worgen_runt_c1AI : public ScriptedAI
    {
        npc_worgen_runt_c1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Loc1, Loc2, Jump, Combat;

        void Reset() override
        {
            Run = Loc1 = Loc2 = Combat = Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->GetPositionX() == -1611.40f && me->GetPositionY() == 1498.49f) // I was spawned in location 1
            {
                Run = true; // Start running across roof
                Loc1 = true;
            }
            else if (me->GetPositionX() == -1618.86f && me->GetPositionY() == 1505.68f) // I was spawned in location 2
            {
                Run = true; // Start running across roof
                Loc2 = true;
            }

            if (Run && !Jump && !Combat)
            {
                if (Loc1) // If I was spawned in Location 1
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,NW_WAYPOINT_LOC1[WaypointId].X, NW_WAYPOINT_LOC1[WaypointId].Y, NW_WAYPOINT_LOC1[WaypointId].Z);
                }
                else if (Loc2)// If I was spawned in Location 2
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,NW_WAYPOINT_LOC2[WaypointId].X, NW_WAYPOINT_LOC2[WaypointId].Y, NW_WAYPOINT_LOC2[WaypointId].Z);
                }
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, ObjectGuid::Empty);
                        DoCast(me, SPELL_ENRAGE, false);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId) override
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            if (Loc1)
            {
                CommonWPCount = sizeof(NW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints
            }
            else if (Loc2)
            {
                CommonWPCount = sizeof(NW_WAYPOINT_LOC2)/sizeof(Waypoint); // Count our waypoints
            }

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                if (Loc1)
                {
                    me->GetMotionMaster()->MoveJump(-1668.52f + irand(-3, 3), 1439.69f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc1 = false;
                }
                else if (Loc2)
                {
                    me->GetMotionMaster()->MoveJump(-1678.04f + irand(-3, 3), 1450.88f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc2 = false;
                }

                Run = false; // Stop running - Regardless of spawn location
                Jump = true; // Time to Jump - Regardless of spawn location
            }
        }
    };
};

/*######
## npc_worgen_runt_c2
######*/

class npc_worgen_runt_c2 : public CreatureScript
{
public:
    npc_worgen_runt_c2() : CreatureScript("npc_worgen_runt_c2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_c2AI (creature);
    }

    struct npc_worgen_runt_c2AI : public ScriptedAI
    {
        npc_worgen_runt_c2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Loc1, Loc2, Jump, Combat;

        void Reset() override
        {
            Run = Loc1 = Loc2 = Combat= Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->GetPositionX() == -1732.81f && me->GetPositionY() == 1526.34f) // I was spawned in location 1
            {
                Run = true; // Start running across roof
                Loc1 = true;
            }
            else if (me->GetPositionX() == -1737.49f && me->GetPositionY() == 1526.11f) // I was spawned in location 2
            {
                Run = true; // Start running across roof
                Loc2 = true;
            }

            if (Run && !Jump && !Combat)
            {
                if (Loc1) // If I was spawned in Location 1
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,SW_WAYPOINT_LOC1[WaypointId].X, SW_WAYPOINT_LOC1[WaypointId].Y, SW_WAYPOINT_LOC1[WaypointId].Z);
                }
                else if (Loc2)// If I was spawned in Location 2
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,SW_WAYPOINT_LOC2[WaypointId].X, SW_WAYPOINT_LOC2[WaypointId].Y, SW_WAYPOINT_LOC2[WaypointId].Z);
                }
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(50.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 50.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, ObjectGuid::Empty);
                        DoCast(me, SPELL_ENRAGE);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId) override
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            if (Loc1)
            {
                CommonWPCount = sizeof(SW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints
            }
            else if (Loc2)
            {
                CommonWPCount = sizeof(SW_WAYPOINT_LOC2)/sizeof(Waypoint); // Count our waypoints
            }

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                if (Loc1)
                {
                    me->GetMotionMaster()->MoveJump(-1685.521f + irand(-3, 3), 1458.48f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc1 = false;
                }
                else if (Loc2)
                {
                    me->GetMotionMaster()->MoveJump(-1681.81f + irand(-3, 3), 1445.54f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc2 = false;
                }

                Run = false; // Stop running - Regardless of spawn location
                Jump = true; // Time to Jump - Regardless of spawn location
            }
        }
    };
};

/*######
## npc_worgen_alpha_c1
######*/

class npc_worgen_alpha_c1 : public CreatureScript
{
public:
    npc_worgen_alpha_c1() : CreatureScript("npc_worgen_alpha_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_c1AI (creature);
    }

    struct npc_worgen_alpha_c1AI : public ScriptedAI
    {
        npc_worgen_alpha_c1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Loc1, Loc2, Jump, Combat;

        void Reset()
        {
            Run = Loc1 = Loc2 = Combat = Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(uint32 diff)
        {
            if (me->GetPositionX() == -1618.86f && me->GetPositionY() == 1505.68f) // I was spawned in location 1 on NW Rooftop
            {
                Run = true; // Start running across roof
                Loc1 = true;
            }
            else if (me->GetPositionX() == -1562.59f && me->GetPositionY() == 1409.35f) // I was spawned on the North Rooftop
            {
                Run = true; // Start running across roof
                Loc2 = true;
            }

            if (Run && !Jump && !Combat)
            {
                if (Loc1) // If I was spawned in Location 1
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,NW_WAYPOINT_LOC1[WaypointId].X, NW_WAYPOINT_LOC1[WaypointId].Y, NW_WAYPOINT_LOC1[WaypointId].Z);
                }
                else if (Loc2)// If I was spawned in Location 2
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,N_WAYPOINT_LOC[WaypointId].X, N_WAYPOINT_LOC[WaypointId].Y, N_WAYPOINT_LOC[WaypointId].Z);
                }
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, ObjectGuid::Empty);
                        DoCast(me, SPELL_ENRAGE, false);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId) override
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            if (Loc1)
            {
                CommonWPCount = sizeof(NW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints
            }
            else if (Loc2)
            {
                CommonWPCount = sizeof(N_WAYPOINT_LOC)/sizeof(Waypoint); // Count our waypoints
            }

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                if (Loc1)
                {
                    me->GetMotionMaster()->MoveJump(-1668.52f + irand(-3, 3), 1439.69f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc1 = false;
                }
                else if (Loc2)
                {
                    me->GetMotionMaster()->MoveJump(-1660.17f + irand(-3, 3), 1429.55f + irand(-3, 3), PLATFORM_Z, 22.0f, 20.0f);
                    Loc2 = false;
                }

                Run = false; // Stop running - Regardless of spawn location
                Jump = true; // Time to Jump - Regardless of spawn location
            }
        }
    };
};

/*######
## npc_worgen_alpha_c2
######*/

class npc_worgen_alpha_c2 : public CreatureScript
{
public:
    npc_worgen_alpha_c2() : CreatureScript("npc_worgen_alpha_c2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_c2AI (creature);
    }

    struct npc_worgen_alpha_c2AI : public ScriptedAI
    {
        npc_worgen_alpha_c2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Jump, Combat;

        void Reset() override
        {
            Run = Combat= Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->GetPositionX() == -1732.81f && me->GetPositionY() == 1526.34f) // I was just spawned
            {
                Run = true; // Start running across roof
            }

            if (Run && !Jump && !Combat)
            {
                if (WaypointId < 2)
                    me->GetMotionMaster()->MovePoint(WaypointId,SW_WAYPOINT_LOC1[WaypointId].X, SW_WAYPOINT_LOC1[WaypointId].Y, SW_WAYPOINT_LOC1[WaypointId].Z);
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, ObjectGuid::Empty);
                        DoCast(me, SPELL_ENRAGE, false);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId) override
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            CommonWPCount = sizeof(SW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints

            WaypointId = PointId + 1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                me->GetMotionMaster()->MoveJump(-1685.52f + irand(-3, 3), 1458.48f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                Run = false; // Stop running
                Jump = true; // Time to Jump
            }
        }
    };
};

/* QUEST - 14154 - By The Skin of His Teeth - END */

/* QUEST - 14159 - The Rebel Lord's Arsenal - START */
// Also Phase shift from Phase 2 to Phase 4

/*######
## npc_josiah_avery_trigger
######*/

class npc_josiah_avery_trigger : public CreatureScript
{
public:
    npc_josiah_avery_trigger() : CreatureScript("npc_josiah_avery_trigger") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_triggerAI (creature);
    }

    struct npc_josiah_avery_triggerAI : public ScriptedAI
    {
        npc_josiah_avery_triggerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 Phase, tEvent;
        ObjectGuid PlayerGUID;

        void Reset() override
        {
            Phase       = 0;
            PlayerGUID.Clear();
            tEvent      = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (Creature* Lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY_P4, 60.0f, true))
            if (Creature* BadAvery = me->FindNearestCreature(NPC_JOSIAH_AVERY_P4, 80.0f, true))
            if (Player* player = me->SelectNearestPlayer(50.0f))
            {
                if (!player->HasAura(SPELL_WORGEN_BITE))
                    return;
                else
                {
                    PlayerGUID = player->GetGUID();
                    if (tEvent <= diff)
                    {
                        switch (Phase)
                        {
                        case (0):
                        {
                            me->AI()->Talk(SAY_JOSAIH_AVERY_TRIGGER, PlayerGUID); // Tell Player they have been bitten
                            tEvent = 200;
                            Phase++;
                            break;
                        }

                        case (1):
                        {
                            BadAvery->SetOrientation(BadAvery->GetAngle(player)); // Face Player
                            BadAvery->CastSpell(player, 69873, true); // Do Cosmetic Attack
                            player->GetMotionMaster()->MoveJump(-1791.94f, 1427.29f, 12.4584f, 22.0f, 8.0f);
                            BadAvery->getThreatManager().resetAllAggro();
                            tEvent = 1200;
                            Phase++;
                            break;
                        }

                        case (2):
                        {
                            BadAvery->GetMotionMaster()->MoveJump(-1791.94f, 1427.29f, 12.4584f, 18.0f, 7.0f);
                            tEvent = 600;
                            Phase++;
                            break;
                        }

                        case (3):
                        {
                            Lorna->CastSpell(BadAvery, SPELL_SHOOT, true);
                            tEvent = 200;
                            Phase++;
                            break;
                        }

                        case (4):
                        {
                            BadAvery->CastSpell(BadAvery, SPELL_GET_SHOT, true);
                            BadAvery->setDeathState(JUST_DIED);
                            player->SaveToDB();
                            BadAvery->DespawnOrUnsummon(1000);
                            me->DespawnOrUnsummon(1000);
                            tEvent = 5000;
                            Phase++;
                            break;
                        }
                        }
                    }
                    else
                        tEvent -= diff;
                }
            }
        }
    };
};

/*######
## npc_josiah_avery_p2
######*/

class npc_josiah_avery_p2 : public CreatureScript
{
public:
    npc_josiah_avery_p2() : CreatureScript("npc_josiah_avery_p2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) override
    {
        if (quest->GetQuestId() == QUEST_THE_REBEL_LORDS_ARSENAL)
        {
            creature->AddAura(SPELL_WORGEN_BITE, player);
            player->RemoveAura(SPELL_PHASE_QUEST_2);
            creature->SetPhaseMask(4, 1);
            creature->CastSpell(creature, SPELL_SUMMON_JOSIAH_AVERY);
            creature->SetPhaseMask(2, 1);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_p2AI (creature);
    }

    struct npc_josiah_avery_p2AI : public ScriptedAI
    {
        npc_josiah_avery_p2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSay, tPlayerCheck;
        ObjectGuid PlayerGUID;
        bool Talk;

        void Reset() override
        {
            tSay        = urand(2000, 4000);
            PlayerGUID.Clear();
            tPlayerCheck= 500;
            Talk        = false;
        }

        void UpdateAI(uint32 diff) override
        {
            if (tPlayerCheck <= diff)
            {
                if (Player* player = me->SelectNearestPlayer(10.0f)) // We should only talk when player is close
                    Talk = true;
                else
                    Talk = false;
            }
            else
                tPlayerCheck -= diff;

            if (Talk && tSay <= diff)
            {
                me->AI()->Talk(SAY_JOSIAH_AVERY_P2);
                tSay = urand(2000, 4000);
            }
            else
                tSay -= diff;
        }
    };
};

/* QUEST - 14159 - The Rebel Lord's Arsenal - END */

/* QUEST - 14204 - FROM THE SHADOWS - START */

/*######
## npc_lorna_crowley_p4
######*/

class npc_lorna_crowley_p4 : public CreatureScript
{
public:
    npc_lorna_crowley_p4() : CreatureScript("npc_lorna_crowley_p4") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
        {
            player->CastSpell(player, SPELL_SUMMON_GILNEAN_MASTIFF);
            creature->AI()->Talk(SAY_LORNA_CROWLEY_P4);
        }
        return true;
    }
};


/*######
## npc_lorna_crowley
######*/

// The Hungry Ettin Q14416
class npc_lorna_crowley : public CreatureScript
{
public:
    npc_lorna_crowley() : CreatureScript("npc_lorna_crowley") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lorna_crowleyAI(creature);
    }

    struct npc_lorna_crowleyAI : public ScriptedAI
    {
        npc_lorna_crowleyAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override {}

        void MoveInLineOfSight(Unit* who) override
        {
            if (!who)
                return;

            Vehicle *pVehicle = who->GetVehicleKit();
            if (!pVehicle)
                return;

            if (!me->IsWithinDistInMap(who, 5.0f))
                return;

            Unit* rider = pVehicle->GetPassenger(0);
            if (!rider)
                return;

            if (Player* player = rider->ToPlayer())
            {
                player->KilledMonsterCredit(NPC_MOUNTAICE_HOURCE_CREDIT, ObjectGuid::Empty);
                player->ExitVehicle();
                who->ToCreature()->ForcedDespawn(100);
            }
        }
    };
};

/*######
# npc_gilnean_mastiff
######*/

class npc_gilnean_mastiff : public CreatureScript
{
public:
    npc_gilnean_mastiff() : CreatureScript("npc_gilnean_mastiff") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_mastiffAI(creature);
    }

    struct npc_gilnean_mastiffAI : public ScriptedAI
    {
        npc_gilnean_mastiffAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            me->InitCharmInfo();
            me->GetCharmInfo()->InitEmptyActionBar(false);
            me->GetCharmInfo()->SetActionBar(0, SPELL_ATTACK_LURKER, ACT_PASSIVE);
            me->SetReactState(REACT_DEFENSIVE);
            me->GetCharmInfo()->SetIsFollowing(true);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (me->GetOwner())
            {
                Player* player = me->GetOwner()->ToPlayer();
                if (!player)
                    return;

                if (player->GetQuestStatus(QUEST_FROM_THE_SHADOWS) == QUEST_STATUS_REWARDED)
                {
                    me->DespawnOrUnsummon(1);
                }

                if (!UpdateVictim())
                {
                    me->GetCharmInfo()->SetIsFollowing(true);
                    me->SetReactState(REACT_DEFENSIVE);
                    return;
                }

                DoMeleeAttackIfReady();
            }
            else
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        }

        void SpellHitTarget(Unit* Mastiff, SpellInfo const* cSpell) override
        {
            if (cSpell->Id == SPELL_ATTACK_LURKER)
            {
                Mastiff->RemoveAura(SPELL_SHADOWSTALKER_STEALTH);
                Mastiff->AddThreat(me, 1.0f);
                me->AddThreat(Mastiff, 1.0f);
                me->AI()->AttackStart(Mastiff);
            }
        }

        void JustDied(Unit* /*killer*/) override // Otherwise, player is stuck with pet corpse they cannot remove from world
        {
            me->DespawnOrUnsummon(1);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Reset();
        }
    };
};

/*######
## npc_bloodfang_lurker
######*/

class npc_bloodfang_lurker : public CreatureScript
{
public:
    npc_bloodfang_lurker() : CreatureScript("npc_bloodfang_lurker") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_lurkerAI (creature);
    }

    struct npc_bloodfang_lurkerAI : public ScriptedAI
    {
        npc_bloodfang_lurkerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset() override
        {
            tEnrage           = 0;
            willCastEnrage    = urand(0, 1);
            tSeek             = urand(5000, 10000);
            DoCast(me, SPELL_SHADOWSTALKER_STEALTH);
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 2.0f)))
                {
                    if (Player* player = me->SelectNearestPlayer(2.0f))
                    {
                        if (!player->isInCombat())
                        {
                            me->AI()->AttackStart(player);
                            tSeek = urand(5000, 10000);
                        }
                    }
                }
            }
            else
                tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, ObjectGuid::Empty);
                DoCast(me, SPELL_ENRAGE, false);
                tEnrage = CD_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/* QUEST - 14204 - FROM THE SHADOWS - END */

/*######
## npc_king_genn_greymane_p4
######*/

class npc_king_genn_greymane_p4 : public CreatureScript
{
public:
    npc_king_genn_greymane_p4() : CreatureScript("npc_king_genn_greymane_p4") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            player->CastSpell(player, 68221);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_genn_greymane_p4AI (creature);
    }

    struct npc_king_genn_greymane_p4AI : public ScriptedAI
    {
        npc_king_genn_greymane_p4AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSummon, tSay;
        bool EventActive, RunOnce;

        void Reset() override
        {
            tSay    = urand(10000, 20000);
            tSummon = urand(3000, 5000); // How often we spawn
        }

        void SummonNextWave()
        {
            switch (urand (1,4))
            {
                case (1):
                    for (uint8 i = 0; i < 5; ++i)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (2):
                    for (uint8 i = 0; i < 5; ++i)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1756.30f + irand(-15, 15), 1380.61f + irand(-15, 15), 19.7652f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (3):
                    for (uint8 i = 0; i < 5; ++i)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1739.84f + irand(-15, 15), 1384.87f + irand(-15, 15), 19.841f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (4):
                    for (uint8 i = 0; i < 5; ++i)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSay <= diff) // Time for next spawn wave
            {
                Talk(SAY_KING_GENN_GREYMANE_P4);
                tSay = urand(10000, 20000);
            }
            else
                tSay -= diff;

            if (tSummon <= diff) // Time for next spawn wave
            {
                SummonNextWave(); // Activate next spawn wave
                tSummon = urand(3000, 5000); // Reset our spawn timer
            }
            else
                tSummon -= diff;
        }

        void JustSummoned(Creature* summoned) override
        {
            summoned->GetDefaultMovementType();
            summoned->SetReactState(REACT_AGGRESSIVE);
        }
    };
};

/*######
## npc_gilneas_city_guard_p8
######*/

class npc_gilneas_city_guard_p8 : public CreatureScript
{
public:
    npc_gilneas_city_guard_p8() : CreatureScript("npc_gilneas_city_guard_p8") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_p8AI (creature);
    }

    struct npc_gilneas_city_guard_p8AI : public ScriptedAI
    {
        npc_gilneas_city_guard_p8AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tYell, SayChance, WillSay;

        void Reset() override
        {
            WillSay     = urand(0,100);
            SayChance   = 10;
            tYell       = urand(10000, 20000);
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (who->GetEntry() == NPC_AFFLICTED_GILNEAN_P8 && me->GetHealthPct() <= AI_MIN_HP)
            {
                damage = 0;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if(tYell <= diff)
            {
                if (WillSay <= SayChance)
                {
                    Talk(SAY_GILNEAS_CITY_GUARD_P8);
                    tYell = urand(10000, 20000);
                }
            }
            else
                tYell -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_afflicted_gilnean_p8
######*/

class npc_afflicted_gilnean_p8 : public CreatureScript
{
public:
    npc_afflicted_gilnean_p8() : CreatureScript("npc_afflicted_gilnean_p8") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_afflicted_gilnean_p8AI (creature);
    }

    struct npc_afflicted_gilnean_p8AI : public ScriptedAI
    {
        npc_afflicted_gilnean_p8AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset() override
        {
            tEnrage           = 0;
            willCastEnrage    = urand(0, 1);
            tSeek             = 100; // On initial loading, we should find our target rather quickly
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (who->IsPlayer())
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->GetEntry() == NPC_GILNEAS_CITY_GUARD_P8)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetHealth(me->GetMaxHealth() * .85);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_GILNEAS_CITY_GUARD_P8, 5.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else
                tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, ObjectGuid::Empty);
                DoCast(me, SPELL_ENRAGE, false);
                tEnrage = CD_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_lord_darius_crowley_c2
######*/
class npc_lord_darius_crowley_c2: public CreatureScript
{
public:
    npc_lord_darius_crowley_c2() : CreatureScript("npc_lord_darius_crowley_c2") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        //for (int i = 0; i < 30; i++)
        //    player->KilledMonsterCredit(35582, 0);

        //player->TeleportTo(638, -1533.76f, 1582.44f, 26.54f, 4.05f);
        player->CastSpell(player, 67766, true);
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PrepareGossipMenu(creature, 0 ,true);
        if (player->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(0, "Start event!", GOSSIP_SENDER_MAIN, 100);
        player->SendPreparedGossip(creature);

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        if (action == 100)
        {
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player, 67766, true);
        }

        return true;
    }
};

/*######
## npc_lord_darius_crowley_c3
######*/

class npc_lord_darius_crowley_c3 : public CreatureScript
{
public:
    npc_lord_darius_crowley_c3() : CreatureScript("npc_lord_darius_crowley_c3") {}

    bool OnQuestComplete(Player* player, Creature* /*creature*/, Quest const* quest) override
    {
        if (quest->GetQuestId() == 14222)
        {
            player->SendMovieStart(21);
            player->CastSpell(player, 93477, true);
            player->RemoveAurasDueToSpell(72870);
            player->CastSpell(player, 76642, true);
            player->CastSpell(player, 68630, true);
            player->CastSpell(player, 72788, true);
            player->CastSpell(player, 68996, true);

            player->CastSpell(player, 69123, true);
            player->CastSpell(player, 68632, true);
            player->CastSpell(player, 68634, true);

            WorldLocation loc;
            loc.m_mapId       = 654;
            loc.m_positionX   = -1818.4f;
            loc.m_positionY   = 2294.25f;
            loc.m_positionZ   = 42.2135f;
            loc.m_orientation = 3.14f;

            player->SetHomebind(loc, 4786);
        }
        return true;
    }
};

/*######
## npc_king_genn_greymane_c2
######*/

class npc_king_genn_greymane_c2 : public CreatureScript
{
public:
    npc_king_genn_greymane_c2() : CreatureScript("npc_king_genn_greymane_c2") {}

    bool OnQuestComplete(Player* player, Creature* /*creature*/, Quest const* /*quest*/) override
    {
        player->RemoveAurasDueToSpell(68630);
        player->RemoveAurasDueToSpell(76642);
        player->CastSpell(player, 68481, true);
        return true;
    }
};

/*######
## npc_greymane_horse
######*/

class npc_greymane_horse : public CreatureScript
{
public:
    npc_greymane_horse() : CreatureScript("npc_greymane_horse") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_greymane_horseAI (creature);
    }

    struct npc_greymane_horseAI : public npc_escortAI
    {
        npc_greymane_horseAI(Creature* creature) : npc_escortAI(creature) {}

        uint32 krennansay;
        bool PlayerOn, KrennanOn;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()override
        {
             krennansay     = 500;//Check every 500ms initially
             PlayerOn       = false;
             KrennanOn      = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (who->IsPlayer())
            {
                PlayerOn = true;
                if (apply)
                    Start(false, true, who->GetGUID());
            }
            else if (who->IsUnit())
            {
                KrennanOn = true;
                SetEscortPaused(false);
            }
        }

        void WaypointReached(uint32 i) override
        {
            Player* player = GetPlayerForEscort();

            switch(i)
            {
                case 5:
                    Talk(SAY_GREYMANE_HORSE, player->GetGUID());
                    me->GetMotionMaster()->MoveJump(-1679.089f, 1348.42f, 15.31f, 25.0f, 15.0f);
                    if (me->GetVehicleKit()->HasEmptySeat(1))
                    {
                        SetEscortPaused(true);
                        break;
                    }
                    break;
                case 12:
                    player->ExitVehicle();
                    player->SetClientControl(me, 1);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SAVE_KRENNAN_ARANAS);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
            Player* player = GetPlayerForEscort();

            if (PlayerOn)
            {
                player->SetClientControl(me, 0);
                PlayerOn = false;
            }

            if (KrennanOn) // Do Not yell for help after krennan is on
                return;

            if (krennansay <=diff)
            {
                if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 70.0f, true))
                {
                    krennan->AI()->Talk(SAY_NPC_KRENNAN_ARANAS_TREE, player->GetGUID());
                    krennansay = urand(4000,7000);//Repeat every 4 to 7 seconds
                }
            }
            else
                krennansay -= diff;
        }
    };
};

/*######
## npc_krennan_aranas_c2
######*/

class npc_krennan_aranas_c2 : public CreatureScript
{
public:
    npc_krennan_aranas_c2() : CreatureScript("npc_krennan_aranas_c2") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_krennan_aranas_c2AI(creature);
    }

    struct npc_krennan_aranas_c2AI : public ScriptedAI
    {
        npc_krennan_aranas_c2AI(Creature* creature) : ScriptedAI(creature) {}

        bool Say, Move, Cast, KrennanDead;
        uint32 SayTimer;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset() override
        {
            Say             = false;
            Move            = true;
            Cast            = true;
            KrennanDead     = false;
            SayTimer        = 500;
        }

        void UpdateAI(uint32 diff) override
        {
            if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 50.0f))
            {
                if (!KrennanDead)
                {
                    krennan->ForcedDespawn(0);
                    KrennanDead = true;
                }
            }

            if (Creature* horse = me->FindNearestCreature(NPC_GREYMANE_HORSE_P4, 20.0f))//Jump onto horse in seat 2
            {
                if (Cast)
                    DoCast(horse, 84275, true);

                if (me->HasAura(84275))
                    Cast = false;
            }

            if (!me->HasAura(84275) && Move)
            {
                me->NearTeleportTo(KRENNAN_END_X, KRENNAN_END_Y, KRENNAN_END_Z, KRENNAN_END_O);
                Say = true;
                Move = false;
                SayTimer = 500;
            }

            if (Say && SayTimer <= diff)
            {
                Talk(SAY_KRENNAN_C2);
                me->ForcedDespawn(6000);
                Say = false;
            }
            else
                SayTimer -= diff;
        }
    };
};

/*######
## npc_commandeered_cannon
######*/

class npc_commandeered_cannon : public CreatureScript
{
public:
    npc_commandeered_cannon() : CreatureScript("npc_commandeered_cannon") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_commandeered_cannonAI (creature);
    }

    struct npc_commandeered_cannonAI : public ScriptedAI
    {
        npc_commandeered_cannonAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEvent;
        uint8 Count, Phase;
        bool EventStart;

        void Reset() override
        {
            tEvent          = 1400;
            Phase           = 0;
            Count           = 0;
            EventStart      = false;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!EventStart)
                return;

            if (Count > 2)
            {
                Reset();
                return;
            }

            if (tEvent <= diff)
            {
                switch (Phase)
                {
                case (0):
                    for (uint8 i = 0; i < 12; ++i)
                        me->SummonCreature(NPC_BLOODFANG_WORGEN, -1757.65f + irand(-6, 6), 1384.01f + irand(-6, 6), 19.872f, urand(0, 6), TEMPSUMMON_TIMED_DESPAWN, 5000);

                    tEvent = 400;
                    Phase++;
                    break;

                case (1):
                    if (Creature* Worgen = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 50.0f, true))
                    {
                        me->CastSpell(Worgen, SPELL_CANNON_FIRE, true);
                        tEvent = 1700;
                        Phase = 0;
                        Count++;
                    }
                    break;
                }
            } else
                tEvent -= diff;
        }

        void JustSummoned(Creature* summon) override
        {
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
    };
};

/*######
## npc_lord_godfrey_p4_8
######*/

class npc_lord_godfrey_p4_8 : public CreatureScript
{
public:
    npc_lord_godfrey_p4_8() : CreatureScript("npc_lord_godfrey_p4_8") { }

    bool OnQuestReward(Player* player, Creature* godfrey, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            godfrey->AI()->Talk(SAY_LORD_GODFREY_P4);
            player->RemoveAurasDueToSpell(SPELL_WORGEN_BITE);
            godfrey->AddAura(SPELL_INFECTED_BITE, player);
            player->CastSpell(player, SPELL_GILNEAS_CANNON_CAMERA);
            player->SaveToDB();
            if (Creature* cannon = GetClosestCreatureWithEntry(godfrey, NPC_COMMANDEERED_CANNON, 50.0f))
            {
                CAST_AI(npc_commandeered_cannon::npc_commandeered_cannonAI, cannon->AI())->EventStart = true; // Start Event
            }
        }
        return true;
    }
};

// Keg Placed - 68555
class spell_keg_placed : public SpellScriptLoader
{
    public:
        spell_keg_placed() : SpellScriptLoader("spell_keg_placed") {}

        class spell_keg_placed_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_keg_placed_AuraScript);

            uint32 tick, tickcount;

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                tick = urand(1, 4);
                tickcount = 0;
            }

            void HandlePeriodic(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                {
                    if (tickcount > tick)
                    {
                        if (caster->GetTypeId() != TYPEID_PLAYER)
                            return;

                        caster->ToPlayer()->KilledMonsterCredit(36233, ObjectGuid::Empty);
                        if (Unit* target = GetTarget())
                            target->Kill(target);
                    }
                    tickcount++;
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_keg_placed_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_keg_placed_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_keg_placed_AuraScript();
        }
};

/*######
## npc_crowley_horse
######*/

class npc_crowley_horse : public CreatureScript
{
public:
    npc_crowley_horse() : CreatureScript("npc_crowley_horse") {}

    struct npc_crowley_horseAI : public npc_escortAI
    {
        npc_crowley_horseAI(Creature* creature) : npc_escortAI(creature) {}

        bool CrowleyOn;
        bool CrowleySpawn;
        bool Run;

        void AttackStart(Unit* /*who*/) override {}
        void EnterCombat(Unit* /*who*/) override {}
        void EnterEvadeMode() override {}

        void Reset() override
        {
            CrowleyOn = false;
            CrowleySpawn = false;
            Run = false;
        }

        void DamageTaken(Unit* pWho, uint32 &uiDamage, DamageEffectType /*dmgType*/) override
        {
            if (me->GetHealthPct() <= 30)
                uiDamage = 0;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (who->IsPlayer())
                if (apply)
                    Start(false, true, who->GetGUID());
        }

        void WaypointReached(uint32 i)
        {
            Vehicle *pVehicle = me->GetVehicleKit();
            Player* player = GetPlayerForEscort();
            Creature* crowley = (Creature*)pVehicle->GetPassenger(1);
            if (!pVehicle || !player || !crowley)
                return;

            switch (i)
            {
                case 1:
                    player->SetClientControl(me, 0);
                    crowley->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->GetMotionMaster()->MoveJump(-1714.02f, 1666.37f, 20.57f, 25.0f, 15.0f);
                    break;
                case 4:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_1);
                    break;
                case 10:
                    me->GetMotionMaster()->MoveJump(-1571.32f, 1710.58f, 20.49f, 25.0f, 15.0f);
                    break;
                case 11:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                    break;
                case 16:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                    break;
                case 20:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->getThreatManager().resetAllAggro();
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->getThreatManager().resetAllAggro();
                    break;
                case 21:
                    player->SetClientControl(me, 1);
                    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->ExitVehicle();
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SACRIFICES);
        }

        void OnCharmed(bool /*apply*/) override {}

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);

            if (!CrowleySpawn)
            {
                DoCast(SPELL_SUMMON_CROWLEY);

                if (Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true))
                    CrowleySpawn = true;
            }

            if (CrowleySpawn && !CrowleyOn)
            {
                Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);
                crowley->CastSpell(me, SPELL_RIDE_HORSE, true); //Mount Crowley in seat 1
                CrowleyOn = true;
            }

            if (!Run)
            {
                me->SetSpeed(MOVE_RUN, CROWLEY_SPEED);
                Run = true;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crowley_horseAI (creature);
    }
};

/*######
## npc_bloodfang_stalker_c1
######*/

class npc_bloodfang_stalker_c1 : public CreatureScript
{
public:
    npc_bloodfang_stalker_c1() : CreatureScript("npc_bloodfang_stalker_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_stalker_c1AI (creature);
    }

    struct npc_bloodfang_stalker_c1AI : public ScriptedAI
    {
        npc_bloodfang_stalker_c1AI(Creature* creature) : ScriptedAI(creature) {}

        Player* player;
        uint32 tEnrage;
        uint32 tAnimate;
        uint32 BurningReset;
        bool Miss, willCastEnrage, Burning;

        void Reset() override
        {
            tEnrage    = 0;
            tAnimate   = DELAY_ANIMATE;
            Miss  = false;
            willCastEnrage = urand(0, 1);
            BurningReset = 3000;
            Burning = false;
        }

        void UpdateAI(uint32 diff) override
        {
            if(me->HasAura(SPELL_THROW_TORCH))
            {
                Burning = true;
            }
            else
                Burning = false;

            if (Burning && BurningReset <=diff)//Extra fail-safe in case for some reason the aura fails to automatically remove itself (happened a few times during testing - cause is still unknown at this time)
            {
                me->RemoveAllAuras();
                BurningReset = 5000;
                Burning = false;
            }
            else
                BurningReset -= diff;

            if (!UpdateVictim())
            {
                return;
            }

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
                    me->MonsterTextEmote(-106, ObjectGuid::Empty);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else
                tEnrage -= diff;

            if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
            {
                Miss = false;
            }
            else if (me->getVictim()->isPet())
            {
                Miss = false;
            }
            else if (me->getVictim()->GetEntry() == NPC_NORTHGATE_REBEL_1)
            {
                if (me->getVictim()->GetHealthPct() < 90)
                    Miss = true;
            }

            if (Miss && tAnimate <= diff)
            {
                //me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKUNARMED);
                me->PlayDistanceSound(SOUND_WORGEN_ATTACK);
                tAnimate = DELAY_ANIMATE;
            }
            else
                tAnimate -= diff;

            if (!Miss)
            {
                DoMeleeAttackIfReady();
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            Creature* horse = me->FindNearestCreature(NPC_CROWLEY_HORSE, 100, true);
            if (spell->Id == SPELL_THROW_TORCH && horse)
            {
                Burning = true;

                if (auto victim = me->getVictim())
                {
                    if (victim && victim->IsPlayer()) //We should ONLY switch our victim if we currently have the player targeted
                    {
                        me->getThreatManager().resetAllAggro();//We need to aggro on crowley's horse, not the player
                        horse->AddThreat(me, 1.0f);
                        me->AddThreat(horse, 1.0f);
                        me->AI()->AttackStart(horse);
                    }
                }

                if (caster->IsPlayer() && caster->ToPlayer()->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                    caster->ToPlayer()->KilledMonsterCredit(NPC_BLOODFANG_STALKER_CREDIT, ObjectGuid::Empty);
            }
        }
    };
};

/*######
## npc_gilnean_crow
######*/

class npc_gilnean_crow : public CreatureScript
{
public:
    npc_gilnean_crow() : CreatureScript("npc_gilnean_crow") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_crowAI (creature);
    }

    struct npc_gilnean_crowAI : public ScriptedAI
    {
        npc_gilnean_crowAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSpawn;
        bool Move;

        void Reset() override
        {
            Move            = false;
            tSpawn          = 0;
            me->SetPosition(me->GetCreatureData()->posX,me->GetCreatureData()->posY, me->GetCreatureData()->posZ, me->GetCreatureData()->orientation);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_PING_GILNEAN_CROW)
            {
                if (!Move)
                {
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE , EMOTE_ONESHOT_NONE); // Change our emote state to allow flight
                    me->SetDisableGravity(true);
                    Move = true;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!Move)
                return;

            if (tSpawn <= diff)
            {
                me->GetMotionMaster()->MovePoint(0, (me->GetPositionX() + irand(-15, 15)), (me->GetPositionY() + irand(-15, 15)), (me->GetPositionZ() + irand(5, 15)));
                tSpawn = urand (500, 1000);
            }
            else
                tSpawn -= diff;

            if ((me->GetPositionZ() - me->GetCreatureData()->posZ) >= 20.0f)
            {
                me->DisappearAndDie();
                me->RemoveCorpse(true);
                Move = false;
            }
        }
    };
};

/*
38615 -> 38540 Captured Riding Bat
*/
class npc_captured_riding_bat : public CreatureScript
{
public:
    npc_captured_riding_bat() : CreatureScript("npc_captured_riding_bat") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_captured_riding_batAI (creature);
    }

    struct npc_captured_riding_batAI : public npc_escortAI
    {
        npc_captured_riding_batAI(Creature* creature) : npc_escortAI(creature) {}

        bool PlayerOn;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset() override
        {
             PlayerOn       = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (who->IsPlayer())
            {
                PlayerOn = true;

                if (apply)
                    Start(false, true, who->GetGUID(), NULL, NULL, true);
            }
        }

        void WaypointReached(uint32 i) override
        {
            Player* player = GetPlayerForEscort();
            switch (i)
            {
                case 36:
                    player->ExitVehicle();
                    player->SetClientControl(me, 1);
                    break;
                default:
                    break;
            }
        }

        void JustDied(Unit* /*killer*/) override {}

        void OnCharmed(bool /*apply*/) override {}

        void UpdateAI(uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            Player* player = GetPlayerForEscort();
            if (player)
                return;

            if (PlayerOn)
            {
                player->SetClientControl(me, 0);
                PlayerOn = false;
            }
        }
    };
};

void AddSC_gilneas()
{
    new npc_gilneas_city_guard_phase1();
    new npc_prince_liam_greymane_phase1();
    new npc_gilneas_city_guard_phase2();
    new npc_prince_liam_greymane_phase2();
    new npc_gwen_armstead_p2();
    new npc_rampaging_worgen();
    new npc_rampaging_worgen2();
    new go_merchant_square_door();
    new npc_sergeant_cleese();
    new npc_frightened_citizen();
    new npc_gilnean_royal_guard();
    new npc_mariam_spellwalker();
    new npc_sean_dempsey();
    new npc_lord_darius_crowley_c1();
    new npc_worgen_runt_c1();
    new npc_worgen_alpha_c1();
    new npc_worgen_runt_c2();
    new npc_worgen_alpha_c2();
    new npc_josiah_avery_p2();
    new npc_josiah_avery_trigger();
    new npc_lorna_crowley();
    new npc_lorna_crowley_p4();
    new npc_gilnean_mastiff();
    new npc_bloodfang_lurker();
    new npc_king_genn_greymane_p4();
    new npc_gilneas_city_guard_p8();
    new npc_afflicted_gilnean_p8();
    new npc_lord_darius_crowley_c2();
    new npc_lord_darius_crowley_c3();
    new npc_king_genn_greymane_c2();
    new npc_crowley_horse();
    new spell_keg_placed();
    new npc_greymane_horse();
    new npc_krennan_aranas_c2();
    new npc_lord_godfrey_p4_8();
    new npc_commandeered_cannon();
    new npc_bloodfang_stalker_c1();
    new npc_gilnean_crow();
    new npc_captured_riding_bat();
}
