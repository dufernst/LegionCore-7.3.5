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

#include "AccountMgr.h"
#include "ulduar.h"

const DoorData doorData[] =
{
    {194416,    BOSS_LEVIATHAN, DOOR_TYPE_ROOM,     0},
    {194905,    BOSS_LEVIATHAN, DOOR_TYPE_PASSAGE,  0},
    {194631,    BOSS_XT002,     DOOR_TYPE_ROOM,     0},
    {194554,    BOSS_ASSEMBLY,  DOOR_TYPE_ROOM,     0},
    {194556,    BOSS_ASSEMBLY,  DOOR_TYPE_PASSAGE,  0},
    {194553,    BOSS_KOLOGARN,  DOOR_TYPE_ROOM,     0},
    {194441,    BOSS_HODIR,     DOOR_TYPE_PASSAGE,  0},
    {194634,    BOSS_HODIR,     DOOR_TYPE_PASSAGE,  0},
    {194442,    BOSS_HODIR,     DOOR_TYPE_ROOM,     0},
    {194559,    BOSS_THORIM,    DOOR_TYPE_ROOM,     0},
    {194774,    BOSS_MIMIRON,   DOOR_TYPE_ROOM,     0},
    {194775,    BOSS_MIMIRON,   DOOR_TYPE_ROOM,     0},
    {194776,    BOSS_MIMIRON,   DOOR_TYPE_ROOM,     0},
    {194750,    BOSS_VEZAX,     DOOR_TYPE_PASSAGE,  0},
    {194773,    BOSS_YOGGSARON, DOOR_TYPE_ROOM,     0},
    {0,         0,              DOOR_TYPE_ROOM,     0}
};

class instance_ulduar : public InstanceMapScript
{
public:
    instance_ulduar() : InstanceMapScript("instance_ulduar", 603) { }

    InstanceScript* GetInstanceScript(InstanceMap* pMap) const override
    {
        return new instance_ulduar_InstanceMapScript(pMap);
    }

    struct instance_ulduar_InstanceMapScript : public InstanceScript
    {
        instance_ulduar_InstanceMapScript(Map* pMap) : InstanceScript(pMap)
        {
            SetBossNumber(MAX_BOSS_NUMBER);
            LoadDoorData(doorData);
            ShieldCheck = 0;
            ApplyAchiv = false;
            _unbroken = true;
            threeknock = true;
            Immortal = true;
            ironDefTimer = 0;
            count = 0;
            _maxArmorItemLevel = 0;
            _maxWeaponItemLevel = 0;
        }   

        ObjectGuid uiLeviathan;
        ObjectGuid uiNorgannon;
        ObjectGuid uiIgnis;
        ObjectGuid uiRazorscale;
        ObjectGuid uiExpCommander;
        ObjectGuid uiXT002;
        ObjectGuid uiSteelbreaker;
        ObjectGuid uiMolgeim;
        ObjectGuid uiBrundir;
        ObjectGuid uiKologarn;
        ObjectGuid uiKologarnleftarm;
        ObjectGuid uiKologarnrightarm;
        ObjectGuid uiKologarnBridge;
        ObjectGuid uiAuriaya;
        ObjectGuid uiBrightleaf;
        ObjectGuid uiIronbranch;
        ObjectGuid uiStonebark;
        ObjectGuid uiFreya;
        ObjectGuid uiThorim;
        ObjectGuid uiRunicColossus;
        ObjectGuid uiRuneGiant;
        ObjectGuid uiMimiron;
        ObjectGuid uiLeviathanMKII;
        ObjectGuid uiVX001;
        ObjectGuid uiAerialUnit;
        ObjectGuid uiMagneticCore;
        ObjectGuid KeepersGateGUID;
        ObjectGuid uiVezax;
        ObjectGuid uiFreyaImage;
        ObjectGuid uiThorimImage;
        ObjectGuid uiMimironImage;
        ObjectGuid uiHodirImage;
        ObjectGuid uiFreyaYS;
        ObjectGuid uiThorimYS;
        ObjectGuid uiMimironYS;
        ObjectGuid uiHodirYS;
        ObjectGuid uiYoggSaronBrain;
        ObjectGuid uiYoggSaron;
        ObjectGuid LeviathanDoorGUID;
        ObjectGuid HodirRareChestGUID;
        ObjectGuid RunicDoorGUID;
        ObjectGuid StoneDoorGUID;
        ObjectGuid ThorimLeverGUID;
        ObjectGuid ThorimDoorGUID;
        ObjectGuid MimironTramGUID;
        ObjectGuid MimironElevatorGUID;
        ObjectGuid Sara;
        ObjectGuid AlgalonSigilDoorGUID[3];

        uint8 count;
        uint32 ironDefTimer;
        uint32 ShieldCheck;
        uint32 _maxArmorItemLevel;
        uint32 _maxWeaponItemLevel;

        bool Immortal;
        bool _unbroken;
        bool threeknock;
        bool ApplyAchiv;
        
        void OnGameObjectCreate(GameObject* pGo) override
        {
            AddDoor(pGo, true);
            switch(pGo->GetEntry())
            {
                case GO_LEVIATHAN_DOOR:
                    LeviathanDoorGUID = pGo->GetGUID();
                    break;
                case GO_KOLOGARN_BRIDGE:
                    uiKologarnBridge = pGo->GetGUID();
                    HandleGameObject(uiKologarnBridge, true);
                    break;
                case GO_HODIR_RARE_CHEST_10:
                    HodirRareChestGUID = pGo->GetGUID();
                    break;
                case GO_HODIR_RARE_CHEST_25:
                    HodirRareChestGUID = pGo->GetGUID();
                    break;
                case GO_RUNIC_DOOR:
                    RunicDoorGUID = pGo->GetGUID();
                    break;
                case GO_STONE_DOOR:
                    StoneDoorGUID = pGo->GetGUID();
                    break;
                case GO_THORIM_LEVER:
                    ThorimLeverGUID = pGo->GetGUID();
                    break;
                case GO_THORIM_DOOR:
                    ThorimDoorGUID = pGo->GetGUID();
                    break;
                case GO_MIMIRON_TRAM:
                    MimironTramGUID = pGo->GetGUID();
                    break;
                case GO_MIMIRON_ELEVATOR:
                    MimironElevatorGUID = pGo->GetGUID();
                    break;
                case GO_KEEPERS_DOOR:
                    KeepersGateGUID = pGo->GetGUID();
                    {
                        if (InstanceScript* instance = pGo->GetInstanceScript())
                        {
                            if (instance->GetBossState(BOSS_MIMIRON) == DONE && 
                                instance->GetBossState(BOSS_FREYA) == DONE &&
                                instance->GetBossState(BOSS_HODIR) == DONE &&
                                instance->GetBossState(BOSS_THORIM) == DONE)
                                pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
                            else
                                pGo->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
                        }
                    }
                    break;
                case GO_DOODAD_UL_SIGILDOOR_01:
                    AlgalonSigilDoorGUID[0] = pGo->GetGUID();
                    break;
                case GO_DOODAD_UL_SIGILDOOR_02:
                    AlgalonSigilDoorGUID[1] = pGo->GetGUID();
                    break;
                default:
                    break;
            }
        }
        
        void OnGameObjectRemove(GameObject* go) override
        {
            AddDoor(go, false);
        }

        void OnUnitDeath(Unit* unit) override
        {
            if (IsEncounterInProgress())
            {
                if (unit->GetTypeId() == TYPEID_PLAYER)
                    if (Immortal)
                        Immortal = false;
            }

            Creature* creature = unit->ToCreature();
            
            if (!creature)
                return;

            //Three Knock on Wood
            if (creature->GetEntry() == 32913 ||
                creature->GetEntry() == 32914 ||
                creature->GetEntry() == 32915)
                threeknock = false;

            if (creature->GetEntry() == 33354 ||
                creature->GetEntry() == 33355 || 
                creature->GetEntry() == 33430 ||
                creature->GetEntry() == 33431 ||
                creature->GetEntry() == 33525 ||
                creature->GetEntry() == 33526 ||
                creature->GetEntry() == 33527 ||
                creature->GetEntry() == 33528)
                DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, 21597);
            

            if (ApplyAchiv)
                return;
            
            if (creature->GetEntry() == NPC_STEELFORGED_DEFFENDER)
            {
                if (!ironDefTimer)
                {
                    DoStartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET2, CRITERIA_DWARFGEDDON);
                    ironDefTimer = 10*IN_MILLISECONDS;
                }

                count++;
                DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, CRITERIA_DWARFGEDDON);

                if (count >= 100)
                {
                    ApplyAchiv = true;
                    ironDefTimer = 0;
                    count = 0;
                    DoStopTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET2, CRITERIA_DWARFGEDDON);
                }
            }
        }
        
        void Update(uint32 diff) override
        {
            if (ironDefTimer)
            {
                if (ironDefTimer <= diff)
                {
                    ironDefTimer = 0;
                    count = 0;
                    DoStopTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET2, CRITERIA_DWARFGEDDON);
                } else ironDefTimer -= diff;
            }

            if (ShieldCheck)
            {
                if (ShieldCheck <= diff)
                {
                    Shield();
                    ShieldCheck = 1500;
                }
                else
                    ShieldCheck -= diff;
            }
        }
        
        void Shield()
        {
            Map::PlayerList const &players = instance->GetPlayers();
            for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
            {
                if (Player* pPlayer = i->getSource())
                {
                    if (pPlayer->isAlive() && pPlayer->GetVehicle())
                    {
                        if (Creature* vehicle = pPlayer->GetVehicleBase()->ToCreature())
                        {
                            if (vehicle->GetEntry() != 32934 && vehicle->GetEntry() != 33118) //Right Arm Kologarn && Ignis Grab
                                vehicle->DespawnOrUnsummon();
                        }
                    }
                }
            }
        }
        
        void OnCreatureCreate(Creature* pCreature) override
        {
            Map::PlayerList const &players = instance->GetPlayers();
            uint32 TeamInInstance = 0;

            if (!players.isEmpty())
                if (Player* pPlayer = players.begin()->getSource())
                    TeamInInstance = pPlayer->GetTeam();
        
            switch(pCreature->GetEntry())
            {
                case 33113: uiLeviathan = pCreature->GetGUID(); return;
                case 33686: uiNorgannon = pCreature->GetGUID(); return;
                case 33118: uiIgnis = pCreature->GetGUID(); return;
                case 33186: uiRazorscale = pCreature->GetGUID(); return;
                case 33210: uiExpCommander = pCreature->GetGUID(); return;
                case 33293: uiXT002 = pCreature->GetGUID(); return;
                case 32867: uiSteelbreaker = pCreature->GetGUID(); return;
                case 32927: uiMolgeim = pCreature->GetGUID(); return;
                case 32857: uiBrundir = pCreature->GetGUID(); return;
                case 32930: uiKologarn = pCreature->GetGUID(); return;
                case 32933: uiKologarnleftarm = pCreature->GetGUID();return;
                case 32934: uiKologarnrightarm = pCreature->GetGUID();return;
                case 33515: uiAuriaya = pCreature->GetGUID(); return;
                case 32915: uiBrightleaf = pCreature->GetGUID(); return;
                case 32913: uiIronbranch = pCreature->GetGUID(); return;
                case 32914: uiStonebark = pCreature->GetGUID(); return;
                case 32906: uiFreya = pCreature->GetGUID(); return;
                case 32865: uiThorim = pCreature->GetGUID(); return;
                case 32872: uiRunicColossus = pCreature->GetGUID(); return;
                case 32873: uiRuneGiant = pCreature->GetGUID(); return;
                case 33350: uiMimiron = pCreature->GetGUID(); return;
                case 33432: uiLeviathanMKII = pCreature->GetGUID(); return;
                case 33651: uiVX001 = pCreature->GetGUID(); return;
                case 33670: uiAerialUnit = pCreature->GetGUID(); return;
                case 34068: uiMagneticCore = pCreature->GetGUID(); return;
                case 33271: uiVezax = pCreature->GetGUID(); return;
                case 33410: uiFreyaYS = pCreature->GetGUID(); return;
                case 33413: uiThorimYS = pCreature->GetGUID(); return;
                case 33412: uiMimironYS = pCreature->GetGUID(); return;
                case 33411: uiHodirYS = pCreature->GetGUID(); return;
                case 33890: uiYoggSaronBrain = pCreature->GetGUID(); return;
                case 33288: uiYoggSaron = pCreature->GetGUID(); return;
                case 33134: Sara = pCreature->GetGUID(); return;

            
                // Keeper's Images
                case 33241: uiFreyaImage = pCreature->GetGUID();
                {
                    InstanceScript* instance = pCreature->GetInstanceScript();
                    pCreature->SetVisible(false);
                    if (instance && instance->GetBossState(BOSS_VEZAX) == DONE)
                        pCreature->SetVisible(true);
                }
                return;
                case 33242: uiThorimImage = pCreature->GetGUID();
                {
                    InstanceScript* instance = pCreature->GetInstanceScript();
                    pCreature->SetVisible(false);
                    if (instance && instance->GetBossState(BOSS_VEZAX) == DONE)
                        pCreature->SetVisible(true);
                }
                return;
                case 33244: uiMimironImage = pCreature->GetGUID();
                {
                    InstanceScript* instance = pCreature->GetInstanceScript();
                    pCreature->SetVisible(false);
                    if (instance && instance->GetBossState(BOSS_VEZAX) == DONE)
                        pCreature->SetVisible(true);
                }            
                return;
                case 33213: uiHodirImage = pCreature->GetGUID();
                {
                    InstanceScript* instance = pCreature->GetInstanceScript();
                    pCreature->SetVisible(false);
                    if (instance && instance->GetBossState(BOSS_VEZAX) == DONE)
                        pCreature->SetVisible(true);
                }
                return;
            }

            // Some npcs are faction dependent
            if (TeamInInstance == HORDE)
            {
                switch(pCreature->GetEntry())
                {
                    case 33062: pCreature->SetDisplayId(25871); return;
                    case 33325: pCreature->UpdateEntry(32941, HORDE); return;
                    case 32901: pCreature->UpdateEntry(33333, HORDE); return;
                    case 33328: pCreature->UpdateEntry(33332, HORDE); return;
                    case 32900: pCreature->UpdateEntry(32950, HORDE); return;
                    case 32893: pCreature->UpdateEntry(33331, HORDE); return;
                    case 33327: pCreature->UpdateEntry(32946, HORDE); return;
                    case 32897: pCreature->UpdateEntry(32948, HORDE); return;
                    case 33326: pCreature->UpdateEntry(33330, HORDE); return;
                    case 32907: pCreature->UpdateEntry(32908, HORDE); return;
                    case 32885: pCreature->UpdateEntry(32883, HORDE); return;
                }
            }
        }

        uint32 GetData(uint32 id) const override
        {
            switch (id)
            {
                case DATA_UNBROKEN:
                    return _unbroken ? 1 : 0;
                case DATA_THREE_KNOCK:
                    return threeknock ? 1 : 0;
            }

            return 0;
        }

        ObjectGuid GetGuidData(uint32 id) const override
        {
            switch(id)
            {
                case DATA_LEVIATHAN:
                    return uiLeviathan;
                case DATA_NORGANNON:
                    return uiNorgannon;
                case DATA_IGNIS:
                    return uiIgnis;
                case DATA_RAZORSCALE:
                    return uiRazorscale;
                case DATA_EXP_COMMANDER:
                    return uiExpCommander;
                case DATA_XT002:
                    return uiXT002;
                case DATA_STEELBREAKER:
                    return uiSteelbreaker;
                case DATA_MOLGEIM:
                    return uiMolgeim;
                case DATA_BRUNDIR:
                    return uiBrundir;
                case DATA_KOLOGARN:
                    return uiKologarn;
                case DATA_LEFT_ARM:
                    return uiKologarnleftarm;
                case DATA_RIGHT_ARM:
                    return uiKologarnrightarm;
                case DATA_AURIAYA:
                    return uiAuriaya;
                case DATA_BRIGHTLEAF:
                    return uiBrightleaf;
                case DATA_IRONBRANCH:
                    return uiIronbranch;
                case DATA_STONEBARK:
                    return uiStonebark;
                case DATA_FREYA:
                    return uiFreya;
                case DATA_THORIM:
                    return uiThorim;
                case DATA_RUNIC_COLOSSUS:
                    return uiRunicColossus;
                case DATA_RUNE_GIANT:
                    return uiRuneGiant;
                case DATA_MIMIRON:
                    return uiMimiron;
                case DATA_LEVIATHAN_MK_II:
                    return uiLeviathanMKII;
                case DATA_VX_001:
                    return uiVX001;
                case DATA_AERIAL_UNIT:
                    return uiAerialUnit;
                case DATA_MAGNETIC_CORE:
                    return uiMagneticCore;
                case DATA_VEZAX:
                    return uiVezax;
                case DATA_YS_FREYA:
                    return uiFreyaYS;
                case DATA_YS_THORIM:
                    return uiThorimYS;
                case DATA_YS_MIMIRON:
                    return uiMimironYS;
                case DATA_YS_HODIR:
                    return uiHodirYS;
                case DATA_YOGGSARON_BRAIN:
                    return uiYoggSaronBrain;
                case DATA_YOGGSARON:
                    return uiYoggSaron;
                case DATA_SARA:
                    return Sara;
                case GO_DOODAD_UL_SIGILDOOR_01:
                    return AlgalonSigilDoorGUID[0];
                case GO_DOODAD_UL_SIGILDOOR_02:
                    return AlgalonSigilDoorGUID[1];
            }
            return ObjectGuid::Empty;
        }
    
        void SetData(uint32 id, uint32 value) override
        {
            switch(id)
            {
                case DATA_LEVIATHAN_DOOR:
                    if (GameObject* pLeviathanDoor = instance->GetGameObject(LeviathanDoorGUID))
                        pLeviathanDoor->SetGoState(GOState(value));
                    break;
                case DATA_RUNIC_DOOR:
                    if (GameObject* pRunicDoor = instance->GetGameObject(RunicDoorGUID))
                        pRunicDoor->SetGoState(GOState(value));
                    break;
                case DATA_STONE_DOOR:
                    if (GameObject* pStoneDoor = instance->GetGameObject(StoneDoorGUID))
                        pStoneDoor->SetGoState(GOState(value));
                    break;
                case DATA_CALL_TRAM:
                    if (GameObject* MimironTram = instance->GetGameObject(MimironTramGUID))
                    {
                        // Load Mimiron Tram (unfortunally only server side)
                        instance->LoadGrid(2307, 284.632f);
                
                        if (value == 0)
                            MimironTram->SetGoState(GO_STATE_READY);
                        if (value == 1)
                            MimironTram->SetGoState(GO_STATE_ACTIVE);
                    
                        // Send movement update to players
                        if (Map* pMap = MimironTram->GetMap())
                        {
                            if (pMap->IsDungeon())
                            {
                                Map::PlayerList const &PlayerList = pMap->GetPlayers();

                                if (!PlayerList.isEmpty())
                                {
                                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                    {
                                        if (Player* pPlayer = i->getSource())
                                        {
                                            UpdateData data(pPlayer->GetMapId());
                                            WorldPacket pkt;
                                            MimironTram->BuildValuesUpdateBlockForPlayer(&data, pPlayer);
                                            data.BuildPacket(&pkt);
                                            pPlayer->GetSession()->SendPacket(&pkt);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;
                case DATA_MIMIRON_ELEVATOR:
                    if (GameObject* MimironElevator = instance->GetGameObject(MimironElevatorGUID))
                        MimironElevator->SetGoState(GOState(value));
                    break;
                case DATA_HODIR_RARE_CHEST:
                    if (GameObject* HodirRareChest = instance->GetGameObject(HodirRareChestGUID))
                    {
                        if (value == GO_STATE_READY)
                            HodirRareChest->RemoveFlag(GAMEOBJECT_FIELD_FLAGS,GO_FLAG_NOT_SELECTABLE);
                    }
                    break;
                case DATA_UNBROKEN:
                    _unbroken = (value == 1) ? false : true;
                    break;
                case DATA_OPEN_FIRS_DOOR:
                    HandleGameObject(ThorimDoorGUID, true);
                    break;
                case DATA_THREE_KNOCK:
                    threeknock = (value == 0) ? true : false;
                    break;
                case DATA_ALGALON:
                    {
                        if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_10_N)
                        {
                            // get item level (armor cannot be swapped in combat)
                            Map::PlayerList const& players = instance->GetPlayers();
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            {
                                if (Player* player = itr->getSource())
                                {
                                    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                                    {
                                        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
                                            continue;
        
                                        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                                        {
                                            if (slot >= EQUIPMENT_SLOT_MAINHAND && slot <= EQUIPMENT_SLOT_RANGED)
                                            {
                                                if (item->GetTemplate()->ItemLevel > _maxWeaponItemLevel)
                                                    _maxWeaponItemLevel = item->GetTemplate()->ItemLevel;
                                            }
                                            else if (item->GetTemplate()->ItemLevel > _maxArmorItemLevel)
                                                    _maxArmorItemLevel = item->GetTemplate()->ItemLevel;
                                        }
                                    }
                                }
                            }
                        }
                    }
                break;
            }
        }

            void ProcessEvent(WorldObject* /*gameObject*/, uint32 eventId) override
            {
                // Flame Leviathan's Tower Event triggers
                Creature* FlameLeviathan = instance->GetCreature(uiLeviathan);
                if (FlameLeviathan && FlameLeviathan->isAlive()) // No leviathan, no event triggering ;)
                    switch (eventId)
                    {
                        case EVENT_TOWER_OF_STORM_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_STORM_DESTROYED);
                            break;
                        case EVENT_TOWER_OF_FROST_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_FROST_DESTROYED);
                            break;
                        case EVENT_TOWER_OF_FLAMES_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_FLAMES_DESTROYED);
                            break;
                        case EVENT_TOWER_OF_LIFE_DESTROYED:
                            FlameLeviathan->AI()->DoAction(ACTION_TOWER_OF_LIFE_DESTROYED);
                            break;
                    }
            }

        bool SetBossState(uint32 id, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;
            
            switch (id)
            {
                case BOSS_KOLOGARN:
                    if (state == DONE)
                        HandleGameObject(uiKologarnBridge, false);
                    break;
                case BOSS_HODIR:
                    CheckKeepersState();
                    break;
                case BOSS_THORIM:
                    if (GameObject* pThorimLever = instance->GetGameObject(ThorimLeverGUID))
                    {
                        if (state == IN_PROGRESS)
                            pThorimLever->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        else
                            pThorimLever->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    if (state == NOT_STARTED)
                        HandleGameObject(ThorimDoorGUID, false);
                    CheckKeepersState();
                    break;
                case BOSS_MIMIRON:
                    CheckKeepersState();
                    break;
                case BOSS_FREYA:
                    CheckKeepersState();
                    break;
                case BOSS_VEZAX:
                    if (state == DONE)
                    {
                        // Keeper's Images
                        if (Creature* pFreya = instance->GetCreature(uiFreyaImage))
                            pFreya->SetVisible(true);
                        if (Creature* pThorim = instance->GetCreature(uiThorimImage))
                            pThorim->SetVisible(true);
                        if (Creature* pMimiron = instance->GetCreature(uiMimironImage))
                            pMimiron->SetVisible(true);
                        if (Creature* pHodir = instance->GetCreature(uiHodirImage))
                            pHodir->SetVisible(true);
                    }
                    break;
                case BOSS_ALGALON:
                {
                    if (state == DONE)
                    {
                        //Achievement Herald of the Titans
                       if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_10_N)
                       {
                            // get item level (recheck weapons)
                            Map::PlayerList const& players = instance->GetPlayers();
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                if (Player* player = itr->getSource())
                                {
                                    for (uint8 slot = EQUIPMENT_SLOT_MAINHAND; slot <= EQUIPMENT_SLOT_RANGED; ++slot)
                                        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                                            if (item->GetTemplate()->ItemLevel > _maxWeaponItemLevel)
                                                _maxWeaponItemLevel = item->GetTemplate()->ItemLevel;
                                }
                       }
                    }
                    else if (state == IN_PROGRESS)
                    {
                        if (Difficulty(instance->GetSpawnMode()) == DIFFICULTY_10_N)
                        {
                            // get item level (armor cannot be swapped in combat)
                            Map::PlayerList const& players = instance->GetPlayers();
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            {
                                if (Player* player = itr->getSource())
                                {
                                    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                                    {
                                        if (slot == EQUIPMENT_SLOT_TABARD || slot == EQUIPMENT_SLOT_BODY)
                                            continue;
        
                                        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                                        {
                                            if (slot >= EQUIPMENT_SLOT_MAINHAND && slot <= EQUIPMENT_SLOT_RANGED)
                                            {
                                                if (item->GetTemplate()->ItemLevel > _maxWeaponItemLevel)
                                                    _maxWeaponItemLevel = item->GetTemplate()->ItemLevel;
                                            }
                                            else if (item->GetTemplate()->ItemLevel > _maxArmorItemLevel)
                                                    _maxArmorItemLevel = item->GetTemplate()->ItemLevel;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            
            if (state == IN_PROGRESS && id != BOSS_LEVIATHAN)
                ShieldCheck = 1500;
            else if (state == DONE || state == NOT_STARTED)
                if (ShieldCheck)
                    ShieldCheck = 0;
            
            if (state == DONE)
            {
                if (InstanceMap * im = instance->ToInstanceMap())
                {
                    InstanceScript* instance = im->GetInstanceScript();
                    int8 bossval = 0;
                    
                    for (uint32 state = BOSS_LEVIATHAN; state < BOSS_ALGALON; state++)
                    {
                        if (instance->GetBossState(state) == DONE)
                            bossval++;
                        else
                            break;
                    }
                    if (bossval == 13 && Immortal)
                    {
                        if (Difficulty(instance->instance->GetSpawnMode()) == DIFFICULTY_10_N)
                            DoCompleteAchievement(2903);
                        else if (Difficulty(instance->instance->GetSpawnMode()) == DIFFICULTY_25_N)
                            DoCompleteAchievement(2904);
                    }
                }
            }
            return true;
        }

        bool CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* player, Unit const* /* = NULL */, uint32 /* = 0 */) override
        {
            switch (criteriaId)
            {
                case CRITERIA_HERALD_OF_TITANS:
                    return _maxArmorItemLevel <= MAX_HERALD_ARMOR_ITEMLEVEL && _maxWeaponItemLevel <= MAX_HERALD_WEAPON_ITEMLEVEL && player->getLevel() == 80;
            }

            return false;
        }

        bool IsWipe() const override
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();
            
            if (PlayerList.isEmpty())
                return true;
            
            for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
            {
                Player* player = Itr->getSource();
                
                if (!player)
                    continue;
                
                if (player->isAlive() && !player->isGameMaster())
                    return false;
            }
            return true;
        }
        
        void CheckKeepersState()
        {
            if (GameObject* pGo = instance->GetGameObject(KeepersGateGUID))
            {
                InstanceScript* instance = pGo->GetInstanceScript();
                if (instance)
                {
                    if (instance->GetBossState(BOSS_MIMIRON) == DONE &&
                        instance->GetBossState(BOSS_FREYA) == DONE &&
                        instance->GetBossState(BOSS_HODIR) == DONE &&
                        instance->GetBossState(BOSS_THORIM) == DONE)
                        pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
                    else
                        pGo->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED);
                }
            }
        }
        
        std::string GetSaveData() override
        {
            std::ostringstream saveStream;
            saveStream << GetBossSaveData() << " " << Immortal;
            return saveStream.str();
        }

        void Load(const char * data) override
        {
            std::istringstream loadStream(LoadBossState(data));
            uint32 buff;
            for (uint32 i=0; i<MAX_BOSS_NUMBER; ++i)
                loadStream >> buff;
            loadStream >> Immortal;
        }
        
        bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = NULL) const override
        {
            if (player && player->GetSession() && AccountMgr::IsGMAccount(player->GetSession()->GetSecurity()))
                return true;

            switch (bossId)
            {
                case BOSS_KOLOGARN:
                case BOSS_AURIAYA:
                case BOSS_YOGGSARON:
                    if (GetBossState(BOSS_LEVIATHAN) != DONE)
                        return false;
                default:
                    break;
            }

            return true;
        }
    };
};

class go_call_tram : public GameObjectScript
{
public:
    go_call_tram() : GameObjectScript("go_call_tram") { }

    bool OnGossipHello(Player* /*pPlayer*/, GameObject* pGo) override
    {
        InstanceScript* instance = pGo->GetInstanceScript();

        if (!instance)
            return false;

        switch(pGo->GetEntry())
        {
            case 194914:
            case 194438:
                instance->SetData(DATA_CALL_TRAM, 0);
                break;
            case 194912:
            case 194437:
                instance->SetData(DATA_CALL_TRAM, 1);
                break;
        }
        return true;
    }
};

void AddSC_instance_ulduar()
{
    new instance_ulduar();
    new go_call_tram();
}
