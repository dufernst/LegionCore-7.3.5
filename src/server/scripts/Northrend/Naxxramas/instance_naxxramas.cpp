/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "naxxramas.h"

const DoorData doorData[] =
{
    {181126,    BOSS_ANUBREKHAN, DOOR_TYPE_ROOM,     BOUNDARY_S},
    {181195,    BOSS_ANUBREKHAN, DOOR_TYPE_PASSAGE,  0},
    {194022,    BOSS_FAERLINA,  DOOR_TYPE_PASSAGE,  0},
    {181209,    BOSS_FAERLINA,  DOOR_TYPE_PASSAGE,  0},
    {181209,    BOSS_MAEXXNA,   DOOR_TYPE_ROOM,     BOUNDARY_SW},
    {181200,    BOSS_NOTH,      DOOR_TYPE_ROOM,     BOUNDARY_N},
    {181201,    BOSS_NOTH,      DOOR_TYPE_PASSAGE,  BOUNDARY_E},
    {181202,    BOSS_NOTH,      DOOR_TYPE_PASSAGE,  0},
    {181202,    BOSS_HEIGAN,    DOOR_TYPE_ROOM,     BOUNDARY_N},
    {181203,    BOSS_HEIGAN,    DOOR_TYPE_PASSAGE,  BOUNDARY_E},
    {181241,    BOSS_HEIGAN,    DOOR_TYPE_PASSAGE,  0},
    {181241,    BOSS_LOATHEB,   DOOR_TYPE_ROOM,     BOUNDARY_W},
    {181123,    BOSS_PATCHWERK, DOOR_TYPE_PASSAGE,  0},
    {181123,    BOSS_GROBBULUS, DOOR_TYPE_ROOM,     0},
    {181120,    BOSS_GLUTH,     DOOR_TYPE_PASSAGE,  BOUNDARY_NW},
    {181121,    BOSS_GLUTH,     DOOR_TYPE_PASSAGE,  0},
    {181121,    BOSS_THADDIUS,  DOOR_TYPE_ROOM,     0},
    {181124,    BOSS_RAZUVIOUS, DOOR_TYPE_PASSAGE,  0},
    {181124,    BOSS_GOTHIK,    DOOR_TYPE_ROOM,     BOUNDARY_N},
    {181125,    BOSS_GOTHIK,    DOOR_TYPE_PASSAGE,  BOUNDARY_S},
    {181119,    BOSS_GOTHIK,    DOOR_TYPE_PASSAGE,  0},
    {181119,    BOSS_HORSEMEN,  DOOR_TYPE_ROOM,     BOUNDARY_NE},
    {181225,    BOSS_SAPPHIRON, DOOR_TYPE_PASSAGE,  BOUNDARY_W},
    {181228,    BOSS_KELTHUZAD, DOOR_TYPE_ROOM,     BOUNDARY_S},
    {0,         0,              DOOR_TYPE_ROOM,     0}, // EOF
};

const MinionData minionData[] =
{
    //{16573,     BOSS_ANUBREKHAN},     there is no spawn point in db, so we do not add them here
    {16506,     BOSS_FAERLINA},
    {16803,     BOSS_RAZUVIOUS},
    {16063,     BOSS_HORSEMEN},
    {16064,     BOSS_HORSEMEN},
    {16065,     BOSS_HORSEMEN},
    {30549,     BOSS_HORSEMEN},
    {0,         0, }
};

enum eEnums
{
    GO_HORSEMEN_CHEST_HERO  = 193426,
    GO_HORSEMEN_CHEST       = 181366,                   //four horsemen event, DoRespawnGameObject() when event == DONE
    GO_GOTHIK_GATE          = 181170,
    GO_KELTHUZAD_PORTAL01   = 181402,
    GO_KELTHUZAD_PORTAL02   = 181403,
    GO_KELTHUZAD_PORTAL03   = 181404,
    GO_KELTHUZAD_PORTAL04   = 181405,
    GO_KELTHUZAD_TRIGGER    = 181444,

    // Eyes
    GO_ARAC_EYE_RAMP        = 181212,
    GO_PLAG_EYE_RAMP        = 181211,
    GO_MILI_EYE_RAMP        = 181210,
    GO_CONS_EYE_RAMP        = 181213,

    GO_ARAC_EYE_BOSS        = 181233,
    GO_PLAG_EYE_BOSS        = 181231,
    GO_MILI_EYE_BOSS        = 181230,
    GO_CONS_EYE_BOSS        = 181232,

    // Portals
    GO_ARAC_PORTAL          = 181575,
    GO_PLAG_PORTAL          = 181577,
    GO_MILI_PORTAL          = 181578,
    GO_CONS_PORTAL          = 181576,

    GO_THADDIUS_TESLA05     = 181477,
    GO_THADDIUS_TESLA06     = 181478,

    SPELL_ERUPTION          = 29371,

    TRIGGER_NPC_KELTHUZAD   = 2300000,

    SAY_TAUNT1              = -1533090, //not used
    SAY_TAUNT2              = -1533091, //not used
    SAY_TAUNT3              = -1533092, //not used
    SAY_TAUNT4              = -1533093, //not used

    ACHIEVEMENT_UNDYING_10  = 2187,
    ACHIEVEMENT_IMMORTAL_25 = 2186,

    ACHIEVEMENT_SUBTRACTION_10 = 2180,
    ACHIEVEMENT_SUBTRACTION_25 = 2181,
};

const float HeiganPos[2] = {2796, -3707};
const float HeiganEruptionSlope[3] =
{
    (-3685 - HeiganPos[1]) /(2724 - HeiganPos[0]),
    (-3647 - HeiganPos[1]) /(2749 - HeiganPos[0]),
    (-3637 - HeiganPos[1]) /(2771 - HeiganPos[0]),
};

// 0  H      x
//  1        ^
//   2       |
//    3  y<--o
inline uint32 GetEruptionSection(float x, float y)
{
    y -= HeiganPos[1];
    if (y < 1.0f)
        return 0;

    x -= HeiganPos[0];
    if (x > -1.0f)
        return 3;

    float slope = y/x;
    for (uint32 i = 0; i < 3; ++i)
        if (slope > HeiganEruptionSlope[i])
            return i;
    return 3;
}

class instance_naxxramas : public InstanceMapScript
{
public:
    instance_naxxramas() : InstanceMapScript("instance_naxxramas", 533) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_naxxramas_InstanceMapScript(map);
    }

    struct instance_naxxramas_InstanceMapScript : public InstanceScript
    {
        instance_naxxramas_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_BOSS_NUMBER);
            LoadDoorData(doorData);
            LoadMinionData(minionData);
        }

    GuidSet HeiganEruptionGUID[4];
    ObjectGuid GothikGateGUID;
    ObjectGuid HorsemenChestGUID;
    ObjectGuid SapphironGUID;
    ObjectGuid uiFaerlina;
    ObjectGuid uiThane;
    ObjectGuid uiLady;
    ObjectGuid uiBaron;
    ObjectGuid uiSir;

    ObjectGuid uiMaexxna;
    ObjectGuid uiLoatheb;

    ObjectGuid uiThaddius;
    ObjectGuid uiFeugen;
    ObjectGuid uiStalagg;

    ObjectGuid uiKelthuzad;
    ObjectGuid uiKelthuzadTrigger;
    ObjectGuid uiPortals[4];

    ObjectGuid m_uiAracEyeRampGUID;
    ObjectGuid m_uiPlagEyeRampGUID;
    ObjectGuid m_uiMiliEyeRampGUID;
    ObjectGuid m_uiConsEyeRampGUID;

    ObjectGuid m_uiAracPortalGUID;
    ObjectGuid m_uiPlagPortalGUID;
    ObjectGuid m_uiMiliPortalGUID;
    ObjectGuid m_uiConsPortalGUID;

    ObjectGuid m_uiAracEyeBossGUID;
    ObjectGuid m_uiPlagEyeBossGUID;
    ObjectGuid m_uiMiliEyeBossGUID;
    ObjectGuid m_uiConsEyeBossGUID;

    ObjectGuid tesla05Guid;
    ObjectGuid tesla06Guid;

    GOState gothikDoorState;

    time_t minHorsemenDiedTime;
    time_t maxHorsemenDiedTime;

    uint32 uiCheckAchievTimer;
    bool bImmortal;
    bool bAllBossesDone;
    bool bfewer;

    void Initialize() override
    {
        GothikGateGUID.Clear();
        HorsemenChestGUID.Clear();
        SapphironGUID.Clear();
        uiFaerlina.Clear();
        uiThane.Clear();
        uiLady.Clear();
        uiBaron.Clear();
        uiSir.Clear();
        uiMaexxna.Clear();
        uiLoatheb.Clear();
        uiThaddius.Clear();
        uiFeugen.Clear();
        uiStalagg.Clear();
        uiKelthuzad.Clear();
        uiKelthuzadTrigger.Clear();
        uiPortals[0].Clear();
        uiPortals[1].Clear();
        uiPortals[2].Clear();
        uiPortals[3].Clear();
        m_uiAracEyeRampGUID.Clear();
        m_uiPlagEyeRampGUID.Clear();
        m_uiMiliEyeRampGUID.Clear();
        m_uiConsEyeRampGUID.Clear();
        m_uiAracEyeBossGUID.Clear();
        m_uiPlagEyeBossGUID.Clear();
        m_uiMiliEyeBossGUID.Clear();
        m_uiConsEyeBossGUID.Clear();
        m_uiAracPortalGUID.Clear();
        m_uiPlagPortalGUID.Clear();
        m_uiMiliPortalGUID.Clear();
        m_uiConsPortalGUID.Clear();
        tesla05Guid.Clear();
        tesla06Guid.Clear();
        gothikDoorState = GO_STATE_ACTIVE;
        minHorsemenDiedTime = 0;
        maxHorsemenDiedTime = 0;
        uiCheckAchievTimer = 1*IN_MILLISECONDS;
        bImmortal = true;
        bAllBossesDone = false;
        bfewer = true;
    }

    void OnPlayerEnter(Player *pPlayer) override
    {
        if (instance->GetSpawnMode() == DIFFICULTY_10_N)
            if (instance->GetPlayersCountExceptGMs() > 8)
                bfewer = false;

        else if (instance->GetSpawnMode() == DIFFICULTY_25_N)
                if (instance->GetPlayersCountExceptGMs() > 20)
                    bfewer = false;
    }

    void OnCreatureCreate(Creature* pCreature) override
    {
        switch(pCreature->GetEntry())
        {
            case 15989: SapphironGUID = pCreature->GetGUID(); return;
            case 15953: uiFaerlina = pCreature->GetGUID(); return;
            case 15952: uiMaexxna = pCreature->GetGUID(); return;
            case 16064: uiThane = pCreature->GetGUID(); return;
            case 16065: uiLady = pCreature->GetGUID(); return;
            case 30549: uiBaron = pCreature->GetGUID(); return;
            case 16063: uiSir = pCreature->GetGUID(); return;
            case 16011: uiLoatheb = pCreature->GetGUID(); return;;
            case 15928: uiThaddius = pCreature->GetGUID(); return;
            case 15930: uiFeugen = pCreature->GetGUID(); return;
            case 15929: uiStalagg = pCreature->GetGUID(); return;
            case 15990: uiKelthuzad = pCreature->GetGUID(); return;
            case 16218: pCreature->SetDisableGravity(true); return;
        }

        AddMinion(pCreature, true);
    }

    void OnCreatureRemove(Creature* creature) override
    {
        AddMinion(creature, false);
    }

    void OnGameObjectCreate(GameObject* pGo) override
    {
        if (pGo->GetGOInfo()->displayId == 6785 || pGo->GetGOInfo()->displayId == 1287)
        {
            uint32 section = GetEruptionSection(pGo->GetPositionX(), pGo->GetPositionY());
            HeiganEruptionGUID[section].insert(pGo->GetGUID());
            return;
        }

        switch(pGo->GetEntry())
        {
            case GO_GOTHIK_GATE:
                GothikGateGUID = pGo->GetGUID();
                pGo->SetGoState(gothikDoorState);
                break;
            case GO_HORSEMEN_CHEST:
                HorsemenChestGUID = pGo->GetGUID();
                break;
            case GO_HORSEMEN_CHEST_HERO:
                HorsemenChestGUID = pGo->GetGUID();
                break;
            case GO_KELTHUZAD_PORTAL01:
                uiPortals[0] = pGo->GetGUID();
                break;
            case GO_KELTHUZAD_PORTAL02:
                uiPortals[1] = pGo->GetGUID();
                break;
            case GO_KELTHUZAD_PORTAL03:
                uiPortals[2] = pGo->GetGUID();
                break;
            case GO_KELTHUZAD_PORTAL04:
                uiPortals[3] = pGo->GetGUID();
                break;
            case GO_KELTHUZAD_TRIGGER:
                uiKelthuzadTrigger = pGo->GetGUID();
                break;
            case GO_ARAC_EYE_RAMP:
                m_uiAracEyeRampGUID = pGo->GetGUID();
                if (GetBossState(BOSS_MAEXXNA)== DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_ARAC_EYE_BOSS:
                m_uiAracEyeBossGUID = pGo->GetGUID();
                if (GetBossState(BOSS_MAEXXNA)== DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_PLAG_EYE_RAMP:
                m_uiPlagEyeRampGUID = pGo->GetGUID();
                if (GetBossState(BOSS_LOATHEB) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_PLAG_EYE_BOSS:
                m_uiPlagEyeBossGUID = pGo->GetGUID();
                if (GetBossState(BOSS_LOATHEB) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_MILI_EYE_RAMP:
                m_uiMiliEyeRampGUID = pGo->GetGUID();
                if (GetBossState(BOSS_HORSEMEN) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_MILI_EYE_BOSS:
                m_uiMiliEyeBossGUID = pGo->GetGUID();
                if (GetBossState(BOSS_HORSEMEN) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_CONS_EYE_RAMP:
                m_uiConsEyeRampGUID = pGo->GetGUID();
                if (GetBossState(BOSS_THADDIUS) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_CONS_EYE_BOSS:
                m_uiConsEyeBossGUID = pGo->GetGUID();
                if (GetBossState(BOSS_THADDIUS) == DONE)
                    pGo->SetGoState(GO_STATE_ACTIVE);
                break;
            case GO_ARAC_PORTAL:
                m_uiAracPortalGUID = pGo->GetGUID();
                if (GetBossState(BOSS_MAEXXNA) == DONE)
                    pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS,GO_FLAG_NOT_SELECTABLE);
                break;
            case GO_PLAG_PORTAL:
                m_uiPlagPortalGUID = pGo->GetGUID();
                if (GetBossState(BOSS_LOATHEB) == DONE)
                    pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS,GO_FLAG_NOT_SELECTABLE);
                break;
            case GO_MILI_PORTAL:
                m_uiMiliPortalGUID = pGo->GetGUID();
                if (GetBossState(BOSS_HORSEMEN) == DONE)
                    pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS,GO_FLAG_NOT_SELECTABLE);
                break;
            case GO_CONS_PORTAL:
                m_uiConsPortalGUID = pGo->GetGUID();
                if (GetBossState(BOSS_THADDIUS) == DONE)
                    pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS,GO_FLAG_NOT_SELECTABLE);
                break;
            case GO_THADDIUS_TESLA05:
                tesla05Guid = pGo->GetGUID();
                pGo->ResetDoorOrButton();
                break;
            case GO_THADDIUS_TESLA06:
                tesla06Guid = pGo->GetGUID();
                pGo->ResetDoorOrButton();
                break;
        }

        AddDoor(pGo, true);
    }

    void OnGameObjectRemove(GameObject* go) override
    {
        if (go->GetGOInfo()->displayId == 6785 || go->GetGOInfo()->displayId == 1287)
        {
            uint32 section = GetEruptionSection(go->GetPositionX(), go->GetPositionY());

            HeiganEruptionGUID[section].erase(go->GetGUID());
            return;
        }

        switch (go->GetEntry())
        {
        case GO_BIRTH:
            if (SapphironGUID)
            {
                if (Creature* pSapphiron = instance->GetCreature(SapphironGUID))
                    pSapphiron->AI()->DoAction(DATA_SAPPHIRON_BIRTH);
                return;
            }
            break;
        default:
            break;
        }

        AddDoor(go, false);
    }

    void SetData(uint32 id, uint32 value) override
    {
        switch(id)
        {
            case DATA_HEIGAN_ERUPT:
                HeiganErupt(value);
                break;
            case DATA_GOTHIK_GATE:
                if (GameObject *pGothikGate = instance->GetGameObject(GothikGateGUID))
                    pGothikGate->SetGoState(GOState(value));
                gothikDoorState = GOState(value);
                break;

            case DATA_HORSEMEN0:
            case DATA_HORSEMEN1:
            case DATA_HORSEMEN2:
            case DATA_HORSEMEN3:
                if (value == NOT_STARTED)
                {
                    minHorsemenDiedTime = 0;
                    maxHorsemenDiedTime = 0;
                }
                else if (value == DONE)
                {
                    time_t now = time(NULL);

                    if (minHorsemenDiedTime == 0)
                        minHorsemenDiedTime = now;

                    maxHorsemenDiedTime = now;
                }
                break;
        }
    }

    ObjectGuid GetGuidData(uint32 id) const override
    {
        switch(id)
        {
        case DATA_FAERLINA:
            return uiFaerlina;
        case DATA_THANE:
            return uiThane;
        case DATA_LADY:
            return uiLady;
        case DATA_BARON:
            return uiBaron;
        case DATA_SIR:
            return uiSir;
        case DATA_LOATHEB:
            return uiLoatheb;
        case DATA_THADDIUS:
            return uiThaddius;
        case DATA_FEUGEN:
            return uiFeugen;
        case DATA_STALAGG:
            return uiStalagg;
        case DATA_KELTHUZAD:
            return uiKelthuzad;
        case DATA_KELTHUZAD_PORTAL01:
            return uiPortals[0];
        case DATA_KELTHUZAD_PORTAL02:
            return uiPortals[1];
        case DATA_KELTHUZAD_PORTAL03:
            return uiPortals[2];
        case DATA_KELTHUZAD_PORTAL04:
            return uiPortals[3];
        case DATA_KELTHUZAD_TRIGGER:
            return uiKelthuzadTrigger;
        case DATA_THADDIUS_TESLA05:
            return tesla05Guid;
        case DATA_THADDIUS_TESLA06:
            return tesla06Guid;
        }
        return ObjectGuid::Empty;
    }

    bool SetBossState(uint32 id, EncounterState state) override
    {
        if (!InstanceScript::SetBossState(id, state))
            return false;

        if (state == DONE)
        {
            switch (id)
            {
                case BOSS_MAEXXNA:
                    if (Creature* pMaexxna = instance->GetCreature(uiMaexxna))
                    {
                        Position pos;
                        pMaexxna->GetPosition(&pos);
                        if (Creature *pTrigger = pMaexxna->SummonCreature(TRIGGER_NPC_KELTHUZAD, pos, TEMPSUMMON_TIMED_DESPAWN, 5*IN_MILLISECONDS))
                        {
                            pTrigger->SetName("Kel'Thuzad");
                            DoScriptText(RAND(SAY_TAUNT1,SAY_TAUNT2,SAY_TAUNT3,SAY_TAUNT4), pTrigger);
                        }
                        if (GameObject* go = instance->GetGameObject(m_uiAracEyeBossGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                        if (GameObject* go = instance->GetGameObject(m_uiAracEyeRampGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                        if (GameObject* pPortal = instance->GetGameObject(m_uiAracPortalGUID))
                            pPortal->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    break;
                case BOSS_HORSEMEN:
                    if (Creature* pSir = instance->GetCreature(uiSir))
                    {
                        Position pos;
                        pSir->GetPosition(&pos);
                        if (Creature *pTrigger = pSir->SummonCreature(TRIGGER_NPC_KELTHUZAD, pos, TEMPSUMMON_TIMED_DESPAWN, 5*IN_MILLISECONDS))
                        {
                            pTrigger->SetName("Kel'Thuzad");
                            DoScriptText(RAND(SAY_TAUNT1,SAY_TAUNT2,SAY_TAUNT3,SAY_TAUNT4), pTrigger);
                        }
                    }
                    if (GameObject *pHorsemenChest = instance->GetGameObject(HorsemenChestGUID))
                    {
                        pHorsemenChest->SetRespawnTime(pHorsemenChest->GetRespawnDelay());
                        pHorsemenChest->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    if (GameObject* go = instance->GetGameObject(m_uiMiliEyeBossGUID))
                        go->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* go = instance->GetGameObject(m_uiMiliEyeRampGUID))
                        go->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* pPortal = instance->GetGameObject(m_uiMiliPortalGUID))
                        pPortal->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                case BOSS_THADDIUS:
                  /*  if (bfewer)
                    {
                        if (instance->GetSpawnMode() == DIFFICULTY_10_N)
                            DoCompleteAchievement(ACHIEVEMENT_SUBTRACTION_10);
                        else if (instance->GetSpawnMode() == DIFFICULTY_25_N)
                            DoCompleteAchievement(ACHIEVEMENT_SUBTRACTION_25);
                    }*/
                    if (Creature* pThaddius = instance->GetCreature(uiThaddius))
                    {
                        Position pos;
                        pThaddius->GetPosition(&pos);
                        if (Creature *pTrigger = pThaddius->SummonCreature(TRIGGER_NPC_KELTHUZAD, pos, TEMPSUMMON_TIMED_DESPAWN, 5*IN_MILLISECONDS))
                        {
                            pTrigger->SetName("Kel'Thuzad");
                            DoScriptText(RAND(SAY_TAUNT1,SAY_TAUNT2,SAY_TAUNT3,SAY_TAUNT4), pTrigger);
                        }
                    }
                    if (GameObject* go = instance->GetGameObject(m_uiConsEyeBossGUID))
                        go->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* go = instance->GetGameObject(m_uiConsEyeRampGUID))
                        go->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* pPortal = instance->GetGameObject(m_uiConsPortalGUID))
                        pPortal->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                case BOSS_LOATHEB:
                    if (Creature* pLoatheb = instance->GetCreature(uiLoatheb))
                    {
                        Position pos;
                        pLoatheb->GetPosition(&pos);
                        if (Creature *pTrigger = pLoatheb->SummonCreature(TRIGGER_NPC_KELTHUZAD, pos, TEMPSUMMON_TIMED_DESPAWN, 5*IN_MILLISECONDS))
                        {
                            pTrigger->SetName("Kel'Thuzad");
                            DoScriptText(RAND(SAY_TAUNT1,SAY_TAUNT2,SAY_TAUNT3,SAY_TAUNT4), pTrigger);
                        }
                    }
                    if (GameObject* go = instance->GetGameObject(m_uiPlagEyeBossGUID))
                        go->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* go = instance->GetGameObject(m_uiPlagEyeRampGUID))
                        go->SetGoState(GO_STATE_ACTIVE);
                    if (GameObject* pPortal = instance->GetGameObject(m_uiPlagPortalGUID))
                        pPortal->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                default:
                    break;
            }
            for (uint32 i=0; i<MAX_BOSS_NUMBER; ++i)
            {
                bAllBossesDone = true;
                if (GetBossState(i) != DONE)
                {
                    bAllBossesDone = false;
                    break;
                }
            }
            if (bAllBossesDone && bImmortal)
                if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_10_N)
                    DoCompleteAchievement(ACHIEVEMENT_UNDYING_10);
                else if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_25_N)
                    DoCompleteAchievement(ACHIEVEMENT_IMMORTAL_25);
        }
        if (state == IN_PROGRESS)
        {
            if (id != BOSS_HORSEMEN)
            {
                if (instance->GetSpawnMode() == DIFFICULTY_10_N)
                {
                    if (instance->GetPlayersCountExceptGMs() > 8)
                        bfewer = false;
                    else
                        bfewer = true;
                }
                else if (instance->GetSpawnMode() == DIFFICULTY_25_N)
                {
                    if (instance->GetPlayersCountExceptGMs() > 20)
                        bfewer = false;
                    else
                        bfewer = true;
                }
            }
        }
        return true;
    }

    void HeiganErupt(uint32 section)
    {
        for (uint32 i = 0; i < 4; ++i)
        {
            if (i == section)
                continue;

            for (GuidSet::const_iterator itr = HeiganEruptionGUID[i].begin(); itr != HeiganEruptionGUID[i].end(); ++itr)
            {
                if (GameObject *pHeiganEruption = instance->GetGameObject(*itr))
                {
                    if (pHeiganEruption->GetGOInfo()->displayId == 1287)
                        pHeiganEruption->CastSpell(NULL, SPELL_ERUPTION);
                    else
                        pHeiganEruption->SendCustomAnim(pHeiganEruption->GetGoAnimProgress());
                }
            }
        }
    }

    bool CheckAchievementCriteriaMeet(uint32 criteria_id, Player const* /*source*/, Unit const* /*target = NULL*/, uint32 /*miscvalue1 = 0*/) override
    {
        switch(criteria_id)
        {
            case 7600:  // Criteria for achievement 2176: And They Would All Go Down Together 15sec of each other 10-man
                if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_10_N && (maxHorsemenDiedTime - minHorsemenDiedTime) < 15)
                    return true;
                return false;
            case 7601:  // Criteria for achievement 2177: And They Would All Go Down Together 15sec of each other 25-man
                if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_25_N && (maxHorsemenDiedTime - minHorsemenDiedTime) < 15)
                    return true;
                return false;
            case 13233: // Criteria for achievement 2186: The Immortal (25-man)
                // TODO.
                break;
            case 13237: // Criteria for achievement 2187: The Undying (10-man)
                // TODO.
                break;
            case 7608:  // Criteria for achievement 2180: Subtraction (10 player)
            case 7609:  // Criteria for achievement 2181: Subtraction (25 player)
            // Criteria for achievement 578: The Dedicated Few (10 player)
            case 6802:    // Kel'Thuzad
            case 7146:    // Anub'Rekhan
            case 7147:    // Gro?witwe Faerlina
            case 7148:    // Maexxna
            case 7149:    // Flickwerk
            case 7150:    // Grobbulus
            case 7151:    // Gluth
            case 7152:    // Thaddius
            case 7153:    // Noth der Seuchenfurst
            case 7154:    // Heigan der Unreine
            case 7155:    // Loatheb
            case 7156:    // Instrukteur Razuvious
            case 7157:    // Gothik der Ernter
            case 7158:    // Saphiron
            // Criteria for achievement 579: The Dedicated Few (25 player)
            case 7159:    // Anub'Rekhan
            case 7160:    // Gro?witwe Faerlina
            case 7161:    // Maexxna
            case 7162:    // Flickwerk
            case 7163:    // Grobbulus
            case 7164:    // Gluth
            case 7165:    // Thaddius
            case 7166:    // Noth der Seuchenfurst
            case 7167:    // Heigan der Unreine
            case 7168:    // Loatheb
            case 7169:    // Instrukteur Razuvious
            case 7170:    // Gothik der Ernter
            case 7171:    // Saphiron
            case 7172:    // Kel'Thuzad
                return bfewer;
        }
        return false;
    }

    std::string GetSaveData() override
    {
        std::ostringstream saveStream;
        saveStream << GetBossSaveData() << " " << gothikDoorState << " " << bImmortal;
        return saveStream.str();
    }

    void Load(const char * data) override
    {
        std::istringstream loadStream(LoadBossState(data));
        uint32 buff;
        for (uint32 i=0; i<MAX_BOSS_NUMBER; ++i)
            loadStream >> buff;
        loadStream >> buff;
        gothikDoorState = GOState(buff);
        loadStream >> bImmortal;
    }

    void Update(uint32 diff) override
    {
        if (!IsEncounterInProgress())
            return;

        if (bImmortal)
            if (uiCheckAchievTimer <= diff)
            {
                Map::PlayerList const &PlayerList = instance->GetPlayers();
                if (!PlayerList.isEmpty())
                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        if (i->getSource()->isDead())
                            bImmortal = false;

                uiCheckAchievTimer = 1*IN_MILLISECONDS;
            } else
                uiCheckAchievTimer -= diff;
    }
};
};

//class AreaTrigger_at_naxxramas_frostwyrm_wing : public AreaTriggerScript
//{
//public:
//    AreaTrigger_at_naxxramas_frostwyrm_wing() : AreaTriggerScript("AreaTrigger_at_naxxramas_frostwyrm_wing") { }
//
//bool OnTrigger(Player* pPlayer, const AreaTriggerEntry * /*at*/, bool /*enter*/)
//{
//    if (pPlayer->isGameMaster())
//        return false;
//
//    InstanceScript *data = pPlayer->GetInstanceScript();
//    if (data)
//        for (uint32 i = BOSS_ANUBREKHAN; i < BOSS_SAPPHIRON; ++i)
//            if (data->GetBossState(i) != DONE)
//                return true;
//
//    return false;
//}
//
//};


void AddSC_instance_naxxramas()
{
    new instance_naxxramas();
}