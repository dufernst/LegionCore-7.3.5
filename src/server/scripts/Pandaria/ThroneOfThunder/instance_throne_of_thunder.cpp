
//Throne of Thunder

#include "throne_of_thunder.h"

const DoorData doorData[] =
{
    {GO_JINROKH_PRE_DOOR,    DATA_STORM_CALLER,      DOOR_TYPE_PASSAGE, 0},
    {GO_JINROKH_EX_DOOR,     DATA_JINROKH,           DOOR_TYPE_PASSAGE, 0},
    {GO_HORRIDON_PRE_DOOR,   DATA_STORMBRINGER,      DOOR_TYPE_PASSAGE, 0},
    {GO_HORRIDON_EX_DOOR,    DATA_HORRIDON,          DOOR_TYPE_PASSAGE, 0},
    {GO_COUNCIL_EX_DOOR,     DATA_COUNCIL_OF_ELDERS, DOOR_TYPE_PASSAGE, 0},
    {GO_COUNCIL_EX2_DOOR,    DATA_COUNCIL_OF_ELDERS, DOOR_TYPE_PASSAGE, 0},
    {GO_TORTOS_EX_DOOR,      DATA_TORTOS,            DOOR_TYPE_PASSAGE, 0},
    {GO_TORTOS_EX2_DOOR,     DATA_TORTOS,            DOOR_TYPE_PASSAGE, 0},
    {GO_MEGAERA_EX_DOOR,     DATA_MEGAERA,           DOOR_TYPE_PASSAGE, 0},
    {GO_JI_KUN_EX_DOOR,      DATA_JI_KUN,            DOOR_TYPE_PASSAGE, 0},
    {GO_DURUMU_EX_DOOR,      DATA_DURUMU,            DOOR_TYPE_PASSAGE, 0},
    {GO_PRIMORDIUS_EX_DOOR,  DATA_PRIMORDIUS,        DOOR_TYPE_PASSAGE, 0},
    {GO_DARK_ANIMUS_EX_DOOR, DATA_DARK_ANIMUS,       DOOR_TYPE_PASSAGE, 0},
    {GO_IRON_QON_EX_DOOR,    DATA_IRON_QON,          DOOR_TYPE_PASSAGE, 0},
    {GO_TWIN_EX_DOOR,        DATA_TWIN_CONSORTS,     DOOR_TYPE_PASSAGE, 0},
    {0,                      0,                      DOOR_TYPE_PASSAGE, 0},
};

uint32 HorridonAddGates[4] =
{
    GO_FARRAK_GATE,
    GO_GURUBASHI_GATE,
    GO_DRAKKARI_GATE,
    GO_AMANI_GATE,
};

class instance_throne_of_thunder : public InstanceMapScript
{
public:
    instance_throne_of_thunder() : InstanceMapScript("instance_throne_of_thunder", 1098) { }

    struct instance_throne_of_thunder_InstanceMapScript : public InstanceScript
    {
        instance_throne_of_thunder_InstanceMapScript(Map* map) : InstanceScript(map) {}

        //Special lists for Megaera heads mechanic
        GuidVector megaeralist;
        uint8 ElementalBloodofMegaera[3]; //0 - flame, 1 - frozen, 2 - venomounce;
        uint32 megaeraheadlist[3];
        uint32 othermeleehead = 0;
        uint32 lastdiedhead_convert = 0;
        uint32 othermeleehead_convert = 0;
        uint32 safedespawnmegaeratimer = 0;

        //Special list for Jikun nest order
        ObjectGuid jikunincubatelist[10];
        std::vector<uint8> nestnumlist;
        uint8 nestmod = 0;
        uint8 nestnum = 0;
        uint8 nestmaxcount = 0;

        //GameObjects
        ObjectGuid jinrokhpredoorGuid;
        ObjectGuid jinrokhentdoorGuid;
        ObjectGuid jinrokhexdoorGuid;
        ObjectGuid horridonpredoorGuid;
        ObjectGuid horridonentdoorGuid;
        ObjectGuid horridonexdoorGuid;
        ObjectGuid councilexdoorGuid;
        ObjectGuid councilex2doorGuid;
        ObjectGuid tortosexdoorGuid;
        ObjectGuid tortosex2doorGuid;
        ObjectGuid megaeraexdoorGuid;
        ObjectGuid jikunexdoorGuid;
        ObjectGuid durumuexdoorGuid;
        ObjectGuid durumucombatfenchGuid;
        ObjectGuid durumucombatfench2Guid;
        ObjectGuid primordiusentdoorGuid;
        ObjectGuid secretradendoorGuid;
        ObjectGuid primordiusexdoorGuid;
        ObjectGuid danimusentdoorGuid;
        ObjectGuid danimusexdoorGuid;
        ObjectGuid ironqonentdoorGuid;
        ObjectGuid ironqonexdoorGuid;
        ObjectGuid twinentdoorGuid;
        ObjectGuid twinexdoorGuid;
        ObjectGuid radenentdoorGuid;
        
        //Creature
        ObjectGuid stormcallerGuid;
        ObjectGuid jinrokhGuid;
        ObjectGuid stormbringerGuid;
        ObjectGuid horridonGuid;
        ObjectGuid hgatecontrollerGuid;
        ObjectGuid jalakGuid;
        ObjectGuid mallakGuid;
        ObjectGuid marliGuid;
        ObjectGuid kazrajinGuid;
        ObjectGuid sulGuid;
        ObjectGuid garajalsoulGuid;
        ObjectGuid tortosGuid;
        ObjectGuid megaeraGuid;
        ObjectGuid nextmegaeraheadGuid;
        ObjectGuid jikunGuid;
        ObjectGuid durumuGuid;
        ObjectGuid durumueyetargetGuid;
        ObjectGuid primordiusGuid;
        ObjectGuid atcasterstalkerGuid;
        ObjectGuid darkanimusGuid;
        ObjectGuid ironqonGuid;
        ObjectGuid roshakGuid;
        ObjectGuid quetzalGuid;
        ObjectGuid damrenGuid;
        ObjectGuid sulinGuid;
        ObjectGuid lulinGuid;
        ObjectGuid leishenGuid;
        ObjectGuid radenGuid;
        ObjectGuid canimaGuid;
        ObjectGuid cvitaGuid;

        GuidVector councilGuids;
        GuidVector mogufontsGuids;
        GuidVector councilentdoorGuids;
        GuidVector jikunfeatherGuids;
        GuidVector massiveanimagolemGuids;
        GuidVector twinfencedoorGuids;
        GuidVector horridonaddgateGuids;
        GuidVector cinderlistGuids;
        GuidVector icygroundGuids;
        GuidVector torrentoficeGuids;
        GuidVector acidraindGuids;
        GuidVector crimsonfogGuids;
        
        void Initialize()
        {
            SetBossNumber(16);
            LoadDoorData(doorData);

            nestnum = 0;

            switch (instance->GetDifficultyID())
            {
                case DIFFICULTY_LFR:
                case DIFFICULTY_LFR_RAID:
                    nestmod = 1;
                    nestmaxcount = 5;
                    break;
                case DIFFICULTY_10_N:
                case DIFFICULTY_10_HC:
                    nestmod = 2;
                    nestmaxcount = 16;
                    break;
                case DIFFICULTY_25_N:
                    nestmod = 3;
                    nestmaxcount = 20;
                    break;
                case DIFFICULTY_25_HC:
                    nestmod = 4;
                    nestmaxcount = 19;
                    break;
                default:
                    break;
            }

            //Creature
            othermeleehead          = 0;
            lastdiedhead_convert    = 0;
            othermeleehead_convert  = 0;
            safedespawnmegaeratimer = 0;

            for (uint8 n = 0; n < 3; n++)
                megaeraheadlist[n] = 0;

            GenerateMegaeraHeads();
        }

        void GenerateMegaeraHeads()
        {
            for (uint8 n = 0; n < 3; n++)
                ElementalBloodofMegaera[n] = 0;

            uint8 mod = urand(0, 5);

            switch (mod)
            {
            case 0:
                megaeraheadlist[0] = NPC_FLAMING_HEAD_MELEE;
                megaeraheadlist[1] = NPC_FROZEN_HEAD_MELEE;
                megaeraheadlist[2] = NPC_VENOMOUS_HEAD_RANGE;
                break;
            case 1:
                megaeraheadlist[0] = NPC_FROZEN_HEAD_MELEE;
                megaeraheadlist[1] = NPC_FLAMING_HEAD_MELEE;
                megaeraheadlist[2] = NPC_VENOMOUS_HEAD_RANGE;
                break;
            case 2:
                megaeraheadlist[0] = NPC_FROZEN_HEAD_MELEE;
                megaeraheadlist[1] = NPC_VENOMOUS_HEAD_MELEE;
                megaeraheadlist[2] = NPC_FLAMING_HEAD_RANGE;
                break;
            case 3:
                megaeraheadlist[0] = NPC_VENOMOUS_HEAD_MELEE;
                megaeraheadlist[1] = NPC_FROZEN_HEAD_MELEE;
                megaeraheadlist[2] = NPC_FLAMING_HEAD_RANGE;
                break;
            case 4:
                megaeraheadlist[0] = NPC_VENOMOUS_HEAD_MELEE;
                megaeraheadlist[1] = NPC_FLAMING_HEAD_MELEE;
                megaeraheadlist[2] = NPC_FROZEN_HEAD_RANGE;
                break;
            case 5:
                megaeraheadlist[0] = NPC_FLAMING_HEAD_MELEE;
                megaeraheadlist[1] = NPC_VENOMOUS_HEAD_MELEE;
                megaeraheadlist[2] = NPC_FROZEN_HEAD_RANGE;
                break;
            }
        }

        void PrepareToUnsummonMegaera()
        {
            if (Creature* megaera = instance->GetCreature(megaeraGuid))
                megaera->AI()->DoAction(ACTION_MEGAERA_RESET);

            if (!megaeralist.empty())
                for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                    if (Creature* mh = instance->GetCreature(*itr))
                        mh->AI()->DoAction(ACTION_PREPARE_TO_UNSUMMON);
        }

        void ResetMegaera()
        {
            for (uint8 n = 0; n < 3; n++)
                ElementalBloodofMegaera[n] = 0;

            if (!megaeralist.empty())
                for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                    if (Creature* mh = instance->GetCreature(*itr))
                        mh->DespawnOrUnsummon();

            othermeleehead = 0;
            lastdiedhead_convert = 0;
            othermeleehead_convert = 0;
            megaeralist.clear();

            for (uint8 n = 0; n < 3; n++)
                instance->SummonCreature(megaeraheadlist[n], megaeraspawnpos[n]);
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
            case NPC_STORM_CALLER:
                stormcallerGuid = creature->GetGUID();
                break;
            case NPC_JINROKH:
                jinrokhGuid = creature->GetGUID();
                break;
            case NPC_STORMBRINGER:
                stormbringerGuid = creature->GetGUID();
                break;
            case NPC_HORRIDON: 
                horridonGuid = creature->GetGUID();
                break;
            case NPC_JALAK:
                jalakGuid = creature->GetGUID();
                break;
            case NPC_H_GATE_CONTROLLER:
                hgatecontrollerGuid = creature->GetGUID();
                break;
            //Council of Elders
            case NPC_FROST_KING_MALAKK:
                mallakGuid = creature->GetGUID();
                councilGuids.push_back(creature->GetGUID());
                break;
            case NPC_PRINCESS_MARLI:
                marliGuid = creature->GetGUID();
                councilGuids.push_back(creature->GetGUID());
                break;  
            case NPC_KAZRAJIN:  
                kazrajinGuid = creature->GetGUID();
                councilGuids.push_back(creature->GetGUID());
                break;
            case NPC_SUL_SANDCRAWLER: 
                sulGuid = creature->GetGUID();
                councilGuids.push_back(creature->GetGUID());
                break;
            case NPC_GARAJAL_SOUL:
                garajalsoulGuid = creature->GetGUID();
                break;
            //
            case NPC_TORTOS: 
                tortosGuid = creature->GetGUID();
                break;
            //Megaera
            case NPC_MEGAERA:
                megaeraGuid = creature->GetGUID();
                if (!creature->isAlive()) //Megaera done, unsummon heads
                {
                    if (!megaeralist.empty())
                        for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                            if (Creature* mh = instance->GetCreature(*itr))
                                mh->DespawnOrUnsummon();
                }
                else
                {
                    for (uint8 n = 0; n < 3; n++)
                        instance->SummonCreature(megaeraheadlist[n], megaeraspawnpos[n]);
                }
                break;
            case NPC_FLAMING_HEAD_MELEE:
            case NPC_FLAMING_HEAD_RANGE:
            case NPC_VENOMOUS_HEAD_MELEE:
            case NPC_VENOMOUS_HEAD_RANGE:
            case NPC_FROZEN_HEAD_MELEE:
            case NPC_FROZEN_HEAD_RANGE:
                megaeralist.push_back(creature->GetGUID());
                break;
            case NPC_CINDERS:
                cinderlistGuids.push_back(creature->GetGUID());
                break;
            case NPC_ICY_GROUND:
                icygroundGuids.push_back(creature->GetGUID());
                break;
            case NPC_TORRENT_OF_ICE:
                torrentoficeGuids.push_back(creature->GetGUID());
                break;
            case NPC_ACID_RAIN:
                acidraindGuids.push_back(creature->GetGUID());
                break;
            case NPC_JI_KUN:  
                jikunGuid = creature->GetGUID();
                break;
            case NPC_INCUBATER:
                if (creature->GetPositionZ() == -70.551804f)      //NorthEast low
                    jikunincubatelist[0] = creature->GetGUID();
                else if (creature->GetPositionZ() == 41.017502f)  //NorthEast up
                    jikunincubatelist[1] = creature->GetGUID();
                else if (creature->GetPositionZ() == 37.581402f)  //SouthEast up
                    jikunincubatelist[2] = creature->GetGUID();
                else if (creature->GetPositionZ() == -101.667999f)//SouthEast low
                    jikunincubatelist[3] = creature->GetGUID();
                else if (creature->GetPositionZ() == -93.935402f) //SouthWest low
                    jikunincubatelist[4] = creature->GetGUID();
                else if (creature->GetPositionZ() == 43.440941f)  //SouthWest up
                    jikunincubatelist[5] = creature->GetGUID();
                else if (creature->GetPositionZ() == -70.741302f) //West low
                    jikunincubatelist[6] = creature->GetGUID();
                else if (creature->GetPositionZ() == 66.061798f)  //NorthWest up
                    jikunincubatelist[7] = creature->GetGUID();
                else if (creature->GetPositionZ() == -59.078201f) //NorthWest low
                    jikunincubatelist[8] = creature->GetGUID();
                else if (creature->GetPositionZ() == 69.920601f)  //Middle up
                    jikunincubatelist[9] = creature->GetGUID();
                break;
            case NPC_DURUMU:  
                durumuGuid = creature->GetGUID();
                break;
            case NPC_EYEBEAM_TARGET_DURUMU:
                durumueyetargetGuid = creature->GetGUID();
                break;
            case NPC_CRIMSON_FOG:
                if (creature->ToTempSummon())
                    crimsonfogGuids.push_back(creature->GetGUID());
                break;
            case NPC_PRIMORDIUS: 
                primordiusGuid = creature->GetGUID();
                break;
            case NPC_AT_CASTER_STALKER:
                atcasterstalkerGuid = creature->GetGUID();
                break;
            case NPC_MASSIVE_ANIMA_GOLEM:
                massiveanimagolemGuids.push_back(creature->GetGUID());
                break;
            case NPC_DARK_ANIMUS:  
                darkanimusGuid = creature->GetGUID();
                break;
            case NPC_IRON_QON:
                ironqonGuid = creature->GetGUID();
                break;
            case NPC_ROSHAK:
                roshakGuid = creature->GetGUID();
                break;
            case NPC_QUETZAL:
                quetzalGuid = creature->GetGUID();
                break;
            case NPC_DAMREN:
                damrenGuid = creature->GetGUID();
                break;
            //Twin consorts
            case NPC_SULIN:   
                sulinGuid = creature->GetGUID();
                break;
            case NPC_LULIN: 
                lulinGuid = creature->GetGUID();
                break;
            //
            case NPC_LEI_SHEN:
                leishenGuid = creature->GetGUID();
                break;
            case NPC_RA_DEN:
                radenGuid = creature->GetGUID();
                break;
            case NPC_CORRUPTED_ANIMA:
                canimaGuid = creature->GetGUID();
                break;
            case NPC_CORRUPTED_VITA:
                cvitaGuid = creature->GetGUID();
                break;
            }
            //Patch 5.4
            if (IsRaidBoss(creature->GetEntry()))
                if (creature->isAlive())
                    creature->CastSpell(creature, SPELL_SHADO_PAN_ONSLAUGHT, true);
        }

        void OnGameObjectCreate(GameObject* go)
        {    
            switch (go->GetEntry())
            {
            case GO_JINROKH_PRE_DOOR:
                AddDoor(go, true);
                jinrokhpredoorGuid = go->GetGUID();
                break;
            case GO_JINROKH_ENT_DOOR:
                jinrokhentdoorGuid = go->GetGUID();
                break;
            //Mogu Fonts
            case GO_MOGU_SR:
            case GO_MOGU_NR:
            case GO_MOGU_NL:
            case GO_MOGU_SL:
                mogufontsGuids.push_back(go->GetGUID());
                break;
            //
            case GO_JINROKH_EX_DOOR:
                AddDoor(go, true);
                jinrokhexdoorGuid = go->GetGUID();
                break;
            case GO_HORRIDON_PRE_DOOR:
                AddDoor(go, true);
                horridonpredoorGuid = go->GetGUID();
                break;
            case GO_HORRIDON_ENT_DOOR:
                horridonentdoorGuid = go->GetGUID();
                break;
            case GO_FARRAK_GATE:
            case GO_GURUBASHI_GATE:
            case GO_DRAKKARI_GATE:
            case GO_AMANI_GATE:
                horridonaddgateGuids.push_back(go->GetGUID());
                break;
            case GO_HORRIDON_EX_DOOR:
                AddDoor(go, true);
                horridonexdoorGuid = go->GetGUID();
                break;
            case GO_COUNCIL_LENT_DOOR:
                councilentdoorGuids.push_back(go->GetGUID());
                break;
            case GO_COUNCIL_RENT_DOOR:
                councilentdoorGuids.push_back(go->GetGUID());
                break;
            case GO_COUNCIL_EX_DOOR:
                AddDoor(go, true);
                councilexdoorGuid = go->GetGUID();
                break;
            case GO_COUNCIL_EX2_DOOR:
                AddDoor(go, true);
                councilex2doorGuid = go->GetGUID();
                break;
            case GO_TORTOS_EX_DOOR:
                AddDoor(go, true);
                tortosexdoorGuid = go->GetGUID();
                break;
            case GO_TORTOS_EX2_DOOR:
                AddDoor(go, true);
                tortosex2doorGuid = go->GetGUID();
                break;
            case GO_MEGAERA_EX_DOOR:
                AddDoor(go, true);
                megaeraexdoorGuid = go->GetGUID();
                break;
            case GO_JI_KUN_FEATHER:
                jikunfeatherGuids.push_back(go->GetGUID());
                if (GetBossState(DATA_JI_KUN) == DONE)
                {
                    go->SetRespawnTime(604800);
                    go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                }
                break;
            case GO_JIKUN_EGG:
                go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                break;
            case GO_JI_KUN_EX_DOOR:
                AddDoor(go, true);
                jikunexdoorGuid = go->GetGUID();
                break;
            case GO_DURUMU_EX_DOOR:
                AddDoor(go, true);
                durumuexdoorGuid = go->GetGUID();
                break;
            case GO_THUNDER_KING_SMALL:
                durumucombatfench2Guid = go->GetGUID();
                break;
            case GO_THUNDER_KING_LARGE:
                durumucombatfenchGuid = go->GetGUID();
                break;
            case GO_PRIMORDIUS_ENT_DOOR:
                primordiusentdoorGuid = go->GetGUID();
                break;
            case GO_S_RA_DEN_ENT_DOOR:
                LoadSecretRaDenDoor(go);
                secretradendoorGuid = go->GetGUID();
            case GO_PRIMORDIUS_EX_DOOR:
                AddDoor(go, true);
                primordiusexdoorGuid = go->GetGUID();
                break;
            case GO_DARK_ANIMUS_ENT_DOOR:
                danimusentdoorGuid = go->GetGUID();
                break;
            case GO_DARK_ANIMUS_EX_DOOR:
                AddDoor(go, true);
                danimusexdoorGuid = go->GetGUID();
                break;
            case GO_IRON_QON_ENT_DOOR:
                ironqonentdoorGuid = go->GetGUID();
                break;
            case GO_IRON_QON_EX_DOOR:
                AddDoor(go, true);
                ironqonexdoorGuid = go->GetGUID();
                break;
            case GO_TWIN_ENT_DOOR:
                twinentdoorGuid = go->GetGUID();
                break;
            case GO_TWIN_FENCE_DOOR:
                twinfencedoorGuids.push_back(go->GetGUID());
                break;
            case GO_TWIN_FENCE_DOOR_2:
                twinfencedoorGuids.push_back(go->GetGUID());
                break;
            case GO_TWIN_EX_DOOR:
                AddDoor(go, true);
                twinexdoorGuid = go->GetGUID();
                break;
            case GO_RA_DEN_ENT_DOOR:
                radenentdoorGuid = go->GetGUID();
                break;
            default:
                break;
            }
        }

        void DespawnMegaeraSummons()
        {
            if (!cinderlistGuids.empty())
                for (GuidVector::const_iterator itr = cinderlistGuids.begin(); itr != cinderlistGuids.end(); itr++)
                    if (Creature* cinder = instance->GetCreature(*itr))
                        cinder->DespawnOrUnsummon();

            if (!icygroundGuids.empty())
                for (GuidVector::const_iterator itr = icygroundGuids.begin(); itr != icygroundGuids.end(); itr++)
                    if (Creature* icyground = instance->GetCreature(*itr))
                        icyground->DespawnOrUnsummon();

            if (!torrentoficeGuids.empty())
                for (GuidVector::const_iterator itr = torrentoficeGuids.begin(); itr != torrentoficeGuids.end(); itr++)
                    if (Creature* torrentofice = instance->GetCreature(*itr))
                        torrentofice->DespawnOrUnsummon();

            if (!acidraindGuids.empty())
                for (GuidVector::const_iterator itr = acidraindGuids.begin(); itr != acidraindGuids.end(); itr++)
                    if (Creature* acidrain = instance->GetCreature(*itr))
                        acidrain->DespawnOrUnsummon();

            //Clear all lists
            cinderlistGuids.clear();
            icygroundGuids.clear();
            torrentoficeGuids.clear();
            acidraindGuids.clear();
            DoRemoveAurasDueToSpellOnPlayers(SPELL_TORRENT_OF_ICE_T);
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
            case DATA_STORM_CALLER:
                if (state == DONE)
                    HandleGameObject(jinrokhpredoorGuid, true);
                break;
            case DATA_JINROKH:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        for (GuidVector::const_iterator guid = mogufontsGuids.begin(); guid != mogufontsGuids.end(); guid++)
                            HandleGameObject(*guid, false);
                        HandleGameObject(jinrokhentdoorGuid, true);
                        SetData(DATA_RESET_MOGU_FONTS, 0);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(jinrokhentdoorGuid, false);
                        break;
                    case DONE:
                        HandleGameObject(jinrokhentdoorGuid, true);
                        HandleGameObject(jinrokhexdoorGuid, true); 
                        break;
                    }
                }
                break;
            case DATA_STORMBRINGER:
                if (state == DONE)
                    HandleGameObject(horridonpredoorGuid, true);
                break;
            case DATA_HORRIDON:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        ResetHorridonAddGates();
                        HandleGameObject(horridonentdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        ResetHorridonAddGates();
                        HandleGameObject(horridonentdoorGuid, false);
                        break;
                    case DONE:
                        HandleGameObject(horridonentdoorGuid, true);
                        HandleGameObject(horridonexdoorGuid, true);
                        break;
                    }
                }
                break;
            case DATA_COUNCIL_OF_ELDERS:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        for (GuidVector::const_iterator guids = councilentdoorGuids.begin(); guids != councilentdoorGuids.end(); guids++)
                            HandleGameObject(*guids, true);
                        break;
                    case IN_PROGRESS:
                        for (GuidVector::const_iterator guids = councilentdoorGuids.begin(); guids != councilentdoorGuids.end(); guids++)
                            HandleGameObject(*guids, false);
                        break;
                    case DONE:
                        for (GuidVector::const_iterator guids = councilentdoorGuids.begin(); guids != councilentdoorGuids.end(); guids++)
                            HandleGameObject(*guids, true);
                        if (Creature* gs = instance->GetCreature(garajalsoulGuid))
                            gs->DespawnOrUnsummon();
                        HandleGameObject(councilexdoorGuid, true);
                        HandleGameObject(councilex2doorGuid, true);
                        break;
                    }
                }
                break;
            case DATA_TORTOS:
                if (state == DONE)
                {
                    HandleGameObject(tortosexdoorGuid, true);
                    HandleGameObject(tortosex2doorGuid, true);
                }
                break;
            case DATA_MEGAERA:
                switch (state)
                {
                case NOT_STARTED:
                    if (Creature* megaera = instance->GetCreature(megaeraGuid))
                        megaera->AI()->DoAction(ACTION_MEGAERA_RESET);
                    break;
                case IN_PROGRESS:
                    if (Creature* megaera = instance->GetCreature(megaeraGuid))
                        megaera->AI()->DoAction(ACTION_MEGAERA_IN_PROGRESS);
                    if (!megaeralist.empty())
                        for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                            if (Creature* mh = instance->GetCreature(*itr))
                                if (mh->GetEntry() == NPC_FLAMING_HEAD_MELEE || mh->GetEntry() == NPC_FROZEN_HEAD_MELEE || mh->GetEntry() == NPC_VENOMOUS_HEAD_MELEE)
                                    SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, mh);        
                    break;
                case DONE:
                    DespawnMegaeraSummons();
                    if (Creature* megaera = instance->GetCreature(megaeraGuid))
                    {
                        if (!megaeralist.empty())
                            for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                                if (Creature* mh = instance->GetCreature(*itr))
                                    mh->AI()->DoAction(ACTION_UNSUMMON);

                        othermeleehead = 0;
                        lastdiedhead_convert = 0;
                        othermeleehead_convert = 0;
                        megaeralist.clear();

                        megaera->setFaction(35);
                        if (!instance->IsLfr())
                            if (GameObject* chest = megaera->SummonGameObject(218805, 6415.06f, 4527.67f, -209.1780f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 604800))
                                chest->SetObjectScale(3.0f);
                        megaera->Kill(megaera);
                    }
                    HandleGameObject(megaeraexdoorGuid, true);
                    break;
                case FAIL:
                    if (!safedespawnmegaeratimer)
                    {
                        PrepareToUnsummonMegaera();
                        DespawnMegaeraSummons();
                        safedespawnmegaeratimer = 5000;
                    }
                    break;
                default:
                    break;
                }
                break;
            case DATA_JI_KUN:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        nestnum = 0;
                        HandleGameObject(megaeraexdoorGuid, true);
                        break;
                    case DONE:
                        HandleGameObject(jikunexdoorGuid, true);
                        for (GuidVector::const_iterator guid = jikunfeatherGuids.begin(); guid != jikunfeatherGuids.end(); guid++)
                        {
                            if (GameObject* feather = instance->GetGameObject(*guid))
                            {
                                feather->SetRespawnTime(604800);
                                feather->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                            }
                        }
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(megaeraexdoorGuid, false);
                        for (GuidVector::const_iterator guid = jikunfeatherGuids.begin(); guid != jikunfeatherGuids.end(); guid++)
                            if (GameObject* feather = instance->GetGameObject(*guid))
                                feather->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    }
                }
                break;
            case DATA_DURUMU:
            {
                switch (state)
                {
                case NOT_STARTED:
                    HandleGameObject(durumucombatfench2Guid, false);
                    HandleGameObject(durumucombatfenchGuid, false);
                    break;
                case IN_PROGRESS:
                    HandleGameObject(durumucombatfench2Guid, true);
                    HandleGameObject(durumucombatfenchGuid, true);
                    break;
                case DONE:
                    HandleGameObject(durumuexdoorGuid, true);
                    HandleGameObject(durumucombatfench2Guid, false);
                    HandleGameObject(durumucombatfenchGuid, false);
                    break;
                }
                break;
            }
            case DATA_PRIMORDIUS:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(primordiusentdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(primordiusentdoorGuid, false);
                        break;
                    case DONE:
                        HandleGameObject(primordiusentdoorGuid, true);
                        HandleGameObject(primordiusexdoorGuid, true);
                        break;
                    }
                }
                break;
            case DATA_DARK_ANIMUS:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        for (GuidVector::const_iterator guid = massiveanimagolemGuids.begin(); guid != massiveanimagolemGuids.end(); guid++)
                        {
                            if (Creature* mag = instance->GetCreature(*guid))
                            {
                                if (mag->isAlive() && mag->isInCombat())
                                    mag->AI()->EnterEvadeMode();
                                else if (!mag->isAlive())
                                {
                                    mag->Respawn();
                                    mag->GetMotionMaster()->MoveTargetedHome();
                                }
                            }
                        }
                        HandleGameObject(danimusentdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        if (Creature* animus = instance->GetCreature(darkanimusGuid))
                        {
                            if (animus->isAlive() && !animus->isInCombat())
                                animus->AI()->DoZoneInCombat(animus, 150.0f);
                        }

                        for (GuidVector::const_iterator guid = massiveanimagolemGuids.begin(); guid != massiveanimagolemGuids.end(); guid++)
                        {
                            if (Creature* mag = instance->GetCreature(*guid))
                            {
                                if (mag->isAlive() && !mag->isInCombat())
                                    mag->AI()->DoZoneInCombat(mag, 150.0f);
                            }
                        }
                        HandleGameObject(danimusentdoorGuid, false);
                        break;
                    case DONE:
                        HandleGameObject(danimusentdoorGuid, true);
                        HandleGameObject(danimusexdoorGuid, true);
                        break;
                    }
                }
                break;
            case DATA_IRON_QON:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(ironqonentdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(ironqonentdoorGuid, false);
                        break;
                    case DONE:
                        HandleGameObject(ironqonentdoorGuid, true);
                        HandleGameObject(ironqonexdoorGuid, true);
                        break;
                    }
                }
                break;
            case DATA_TWIN_CONSORTS:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            for (GuidVector::const_iterator guid = twinfencedoorGuids.begin(); guid != twinfencedoorGuids.end(); guid++)
                                HandleGameObject(*guid, true);
                            HandleGameObject(twinentdoorGuid, true);
                            break;
                        case IN_PROGRESS:
                            for (GuidVector::const_iterator guid = twinfencedoorGuids.begin(); guid != twinfencedoorGuids.end(); guid++)
                                HandleGameObject(*guid, false);
                            HandleGameObject(twinentdoorGuid, false);
                            break;
                        case DONE:
                            for (GuidVector::const_iterator guid = twinfencedoorGuids.begin(); guid != twinfencedoorGuids.end(); guid++)
                                HandleGameObject(*guid, true);
                            HandleGameObject(twinentdoorGuid, true);
                            HandleGameObject(twinexdoorGuid, true);
                            break;                         
                    }
                }
                break;
            case DATA_RA_DEN:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                    case DONE:
                        HandleGameObject(radenentdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(radenentdoorGuid, false);
                        break;
                    }
                }
                break;
            default:
                break;
            }
           
            if (state == DONE && id != DATA_RA_DEN)
                if (GameObject* go = instance->GetGameObject(secretradendoorGuid))
                    LoadSecretRaDenDoor(go);
            return true;
        }

        void Update(uint32 diff)
        {
            if (safedespawnmegaeratimer)
            {
                if (safedespawnmegaeratimer <= diff)
                {
                    safedespawnmegaeratimer = 0;
                    ResetMegaera();
                }
                else
                    safedespawnmegaeratimer -= diff;
            }
        }

        void ResetHorridonAddGates()
        {
            if (!horridonaddgateGuids.empty())
                for (GuidVector::const_iterator itr = horridonaddgateGuids.begin(); itr != horridonaddgateGuids.end(); itr++)
                    if (GameObject* gate = instance->GetGameObject(*itr))
                        gate->SetGoState(GO_STATE_READY);
        }

        void LoadSecretRaDenDoor(GameObject* go)
        {
            if (go && (instance->GetDifficultyID() == DIFFICULTY_10_HC || instance->GetDifficultyID() == DIFFICULTY_25_HC))
            {
                if (GetBossState(DATA_LEI_SHEN) == DONE)
                    go->SetGoState(GO_STATE_ACTIVE);
            }

            /* for (uint8 n = DATA_STORM_CALLER; n <= DATA_LEI_SHEN; n++)
            {
                if (GetBossState(n) != DONE)
                    return;
            }
            go->SetGoState(GO_STATE_ACTIVE); */
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_RESET_MOGU_FONTS:
                for (GuidVector::const_iterator itr = mogufontsGuids.begin(); itr != mogufontsGuids.end(); itr++)
                    if (GameObject* mogufont = instance->GetGameObject(*itr))
                        mogufont->SetGoState(GO_STATE_READY);
                break;
            case DATA_SEND_LAST_DIED_HEAD:
                if (!megaeralist.empty())
                {
                    for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                    {
                        if (Creature* mh = instance->GetCreature(*itr))
                        {
                            if (mh->GetEntry() != data && (mh->GetEntry() == NPC_FLAMING_HEAD_MELEE || mh->GetEntry() == NPC_FROZEN_HEAD_MELEE || mh->GetEntry() == NPC_VENOMOUS_HEAD_MELEE))
                            {
                                othermeleehead = mh->GetEntry();
                                mh->SetFullHealth();
                                mh->CastSpell(mh, SPELL_HYDRA_FRENZY, true);
                            }
                        }
                    }
                }
                switch (data)
                {
                case NPC_FLAMING_HEAD_MELEE:
                    ElementalBloodofMegaera[0]++;
                    lastdiedhead_convert = NPC_FLAMING_HEAD_RANGE;
                    break;
                case NPC_FROZEN_HEAD_MELEE:
                    ElementalBloodofMegaera[1]++;
                    lastdiedhead_convert = NPC_FROZEN_HEAD_RANGE;
                    break;
                case NPC_VENOMOUS_HEAD_MELEE:
                    ElementalBloodofMegaera[2]++;
                    lastdiedhead_convert = NPC_VENOMOUS_HEAD_RANGE;
                    break;
                }
                switch (othermeleehead)
                {
                case NPC_FLAMING_HEAD_MELEE:
                    othermeleehead_convert = NPC_FLAMING_HEAD_RANGE;
                    break;
                case NPC_FROZEN_HEAD_MELEE:
                    othermeleehead_convert = NPC_FROZEN_HEAD_RANGE;
                    break;
                case NPC_VENOMOUS_HEAD_MELEE:
                    othermeleehead_convert = NPC_VENOMOUS_HEAD_RANGE;
                    break;
                default:
                    break;
                }
                break;
            case DATA_ACTIVE_NEXT_NEST:
                ActiveNextNest();
                break;
            case DATA_LAUNCH_FEED_NEST:
                if (Creature* jikun = instance->GetCreature(GetGuidData(NPC_JI_KUN)))
                {
                    std::list<Creature*> activeincubatelist;
                    activeincubatelist.clear();
                    //find active nest and write in list
                    for (uint8 n = 0; n < 10; n++)
                        if (Creature* incubate = instance->GetCreature(jikunincubatelist[n]))
                            if (incubate->GetPositionZ() < 36.0f && incubate->HasAura(SPELL_INCUBATE_ZONE))
                                activeincubatelist.push_back(incubate);
                    //launch feed in this nest
                    if (!activeincubatelist.empty())
                    {
                        for (std::list<Creature*>::const_iterator Itr = activeincubatelist.begin(); Itr != activeincubatelist.end(); Itr++)
                        {
                            for (uint8 n = 0; n < 4; n++)
                            {
                                if (Creature* feed = jikun->SummonCreature(NPC_FEED, jikun->GetPositionX(), jikun->GetPositionY(), jikun->GetPositionZ() + 12.0f, 0.0f))
                                {
                                    float x, y;
                                    GetPosInRadiusWithRandomOrientation(*Itr, 4.0f, x, y);
                                    feed->SetDisplayId(11686);
                                    feed->CastSpell(feed, SPELL_FEED_FALL_VISUAL, true);
                                    feed->CastSpell(feed, SPELL_SLIMED_AT, true);
                                    feed->CastSpell(x, y, (*Itr)->GetPositionZ() + 1.5f, SPELL_JUMPS_DOWN_TO_HATCHLING, true);
                                }
                            }
                        }
                    }
                }
                break;
            case DATA_LAUNCH_FEED_PLATFORM:
                if (Creature* jikun = instance->GetCreature(GetGuidData(NPC_JI_KUN)))
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, jikun, 50.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::iterator itr = pllist.begin(); itr != pllist.end();)
                        {
                            if ((*itr)->isInTankSpec())
                                pllist.erase(itr++);
                            else
                                ++itr;
                        }
                        if (!pllist.empty())
                        {
                            uint8 maxsize = urand(2, 3);
                            GuidVector _pllist;
                            _pllist.clear();
                            for (std::list<Player*>::const_iterator Itr = pllist.begin(); Itr != pllist.end(); Itr++)
                                _pllist.push_back((*Itr)->GetGUID());
                            pllist.clear();
                            std::random_shuffle(_pllist.begin(), _pllist.end());
                            if (_pllist.size() > maxsize)
                                _pllist.resize(maxsize);
                            for (GuidVector::const_iterator itr = _pllist.begin(); itr != _pllist.end(); itr++)
                            {
                                if (Creature* feed = jikun->SummonCreature(NPC_FEED, jikun->GetPositionX(), jikun->GetPositionY(), jikun->GetPositionZ() + 12.0f, 0.0f))
                                {
                                    feed->SetDisplayId(48142);
                                    if (Player* plr = jikun->GetPlayer(*jikun, *itr))
                                        feed->CastSpell(plr, SPELL_JUMP_DOWN_TO_PLATFORM, true);
                                }
                            }
                        }
                    }
                }
                break;
            case DATA_JIKUN_RESET_ALL_NESTS:
                for (uint8 n = 0; n < 10; n++)
                    if (Creature* incubater = instance->GetCreature(jikunincubatelist[n]))
                        incubater->AI()->SetData(DATA_RESET_NEST, 0);
                break;
            case DATA_CLEAR_CRIMSON_FOG_LIST:
                crimsonfogGuids.clear();
                break;
            }
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
            case DATA_CHECK_VALIDATE_THUNDERING_THROW:
                for (GuidVector::const_iterator itr = mogufontsGuids.begin(); itr != mogufontsGuids.end(); itr++)
                    if (GameObject* mogufont = instance->GetGameObject(*itr))
                        if (mogufont->GetGoState() == GO_STATE_READY)
                            return 1;
            case DATA_CHECK_COUNCIL_PROGRESS:
                if (!councilGuids.empty())
                {
                    for (GuidVector::const_iterator itr = councilGuids.begin(); itr != councilGuids.end(); itr++)
                        if (Creature* council = instance->GetCreature(*itr))
                            if (council->isAlive())
                                return 1;
                    return 0;
                }
            case DATA_CHECK_PROGRESS_MEGAERA:
                if (Creature* megaera = instance->GetCreature(megaeraGuid))
                {
                    if (megaera->GetHealthPct() <= 14.2)//Done
                    {
                        megaera->Kill(megaera);
                        const_cast<instance_throne_of_thunder_InstanceMapScript*>(this)->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, megaera);
                        const_cast<instance_throne_of_thunder_InstanceMapScript*>(this)->SetBossState(DATA_MEGAERA, DONE);
                        return 0;
                    }
                    else
                        return 1; //Still in progress
                }
            case DATA_GET_COUNT_RANGE_HEADS:
                if (!megaeralist.empty())
                {
                    uint8 count = 0;
                    for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                    {
                        if (Creature* mh = instance->GetCreature(*itr))
                            if (mh->GetEntry() == NPC_FLAMING_HEAD_RANGE || mh->GetEntry() == NPC_FROZEN_HEAD_RANGE || mh->GetEntry() == NPC_VENOMOUS_HEAD_RANGE)
                                count++;
                    }
                    return count;
                }
            case NPC_FLAMING_HEAD_MELEE:
                return ElementalBloodofMegaera[0];
            case NPC_FROZEN_HEAD_MELEE:
                return ElementalBloodofMegaera[1];
            case NPC_VENOMOUS_HEAD_MELEE:
                return ElementalBloodofMegaera[2];
            }
            return 0;
        }

        void CreatureDies(Creature* creature, Unit* /*killer*/)
        {
            switch (creature->GetEntry())
            {
            case NPC_FROST_KING_MALAKK:
            case NPC_PRINCESS_MARLI:
            case NPC_KAZRAJIN:
            case NPC_SUL_SANDCRAWLER:
                if (!councilGuids.empty())
                {
                    for (GuidVector::const_iterator itr = councilGuids.begin(); itr != councilGuids.end(); itr++)
                        if (Creature* council = instance->GetCreature(*itr))
                            if (council->isAlive())
                                return;

                    SetBossState(DATA_COUNCIL_OF_ELDERS, DONE);
                }
                break;
            case NPC_CRIMSON_FOG:
                if (!crimsonfogGuids.empty())
                {
                    uint8 alivecount = 0;
                    for (GuidVector::const_iterator itr = crimsonfogGuids.begin(); itr != crimsonfogGuids.end(); itr++)
                        if (Creature* cfog = instance->GetCreature(*itr))
                            if (cfog->isAlive())
                                alivecount++;

                    if (alivecount)
                    {
                        DoUpdateWorldState(WorldStates::WORLD_STATE_ALIVE_FOG_COUNT, alivecount);
                        return;
                    }

                    DoUpdateWorldState(WorldStates::WORLD_STATE_ALIVE_FOG_COUNT, 0);
                    crimsonfogGuids.clear();
                    if (Creature* durumu = instance->GetCreature(durumuGuid))
                        if (durumu->isAlive() && durumu->isInCombat())
                            durumu->AI()->DoAction(ACTION_COLORBLIND_PHASE_DONE);
                }
                break;
            default:
                break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
            case NPC_JINROKH:
                return jinrokhGuid;
            case NPC_HORRIDON: 
                return horridonGuid;
            case NPC_JALAK:
                return jalakGuid;
            case NPC_H_GATE_CONTROLLER:
                return hgatecontrollerGuid;
            case GO_FARRAK_GATE:
            case GO_GURUBASHI_GATE:
            case GO_DRAKKARI_GATE:
            case GO_AMANI_GATE:
                for (GuidVector::const_iterator itr = horridonaddgateGuids.begin(); itr != horridonaddgateGuids.end(); itr++)
                    if (GameObject* gate = instance->GetGameObject(*itr))
                        if (gate->GetEntry() == type)
                            return gate->GetGUID();
            //Council of Elders
            case NPC_FROST_KING_MALAKK:
                return mallakGuid;
            case NPC_PRINCESS_MARLI:
                return marliGuid;
            case NPC_KAZRAJIN:  
                return kazrajinGuid;
            case NPC_SUL_SANDCRAWLER: 
                return sulGuid;
            case NPC_GARAJAL_SOUL:
                return garajalsoulGuid;
            //Megaera
            case NPC_MEGAERA:
                return megaeraGuid;
            case NPC_FLAMING_HEAD_MELEE:
            case NPC_FLAMING_HEAD_RANGE:
            case NPC_VENOMOUS_HEAD_MELEE:
            case NPC_VENOMOUS_HEAD_RANGE:
            case NPC_FROZEN_HEAD_MELEE:
            case NPC_FROZEN_HEAD_RANGE:
            {
                for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                    if (Creature* mh = instance->GetCreature(*itr))
                        if (mh->GetEntry() == type)
                            return mh->GetGUID();
            }
            case DATA_GET_NEXT_HEAD:
            {
                if (!megaeralist.empty())
                    for (GuidVector::const_iterator itr = megaeralist.begin(); itr != megaeralist.end(); itr++)
                        if (Creature* mh = instance->GetCreature(*itr))
                            if (mh->GetEntry() == NPC_FLAMING_HEAD_RANGE || mh->GetEntry() == NPC_FROZEN_HEAD_RANGE || mh->GetEntry() == NPC_VENOMOUS_HEAD_RANGE)
                                if (mh->GetEntry() != othermeleehead_convert && mh->GetEntry() != lastdiedhead_convert)
                                    return mh->GetGUID();
            }
            case NPC_TORTOS: 
                return tortosGuid;
            case NPC_JI_KUN:  
                return jikunGuid;
            case NPC_DURUMU:  
                return durumuGuid;
            case NPC_EYEBEAM_TARGET_DURUMU:
                return durumueyetargetGuid;
            case NPC_PRIMORDIUS: 
                return primordiusGuid;
            case NPC_AT_CASTER_STALKER:
                return atcasterstalkerGuid;
            case NPC_DARK_ANIMUS:  
                return darkanimusGuid;
            case NPC_IRON_QON:
                return ironqonGuid;
            //Iron Qon Maunts
            case NPC_ROSHAK:
                return roshakGuid;
            case NPC_QUETZAL:
                return quetzalGuid;
            case NPC_DAMREN:
                return damrenGuid;
            //Twin consorts
            case NPC_SULIN:   
                return sulinGuid;
            case NPC_LULIN: 
                return lulinGuid;
            case NPC_LEI_SHEN:
                return leishenGuid;
            case NPC_RA_DEN:
                return radenGuid;
            case NPC_CORRUPTED_ANIMA:
                return canimaGuid;
            case NPC_CORRUPTED_VITA:
                return cvitaGuid;
            }
            return ObjectGuid::Empty;
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

                if (player->isAlive() && !player->isGameMaster() && !player->HasAura(115877)) // Aura 115877 = Totaly Petrified
                    return false;
            }

            return true;
        }

        bool IsRaidBoss(uint32 creature_entry)
        {
            switch (creature_entry)
            {
            case NPC_JINROKH:
            case NPC_HORRIDON:
            case NPC_JALAK:
            case NPC_FROST_KING_MALAKK:
            case NPC_PRINCESS_MARLI:
            case NPC_KAZRAJIN:
            case NPC_SUL_SANDCRAWLER:
            case NPC_TORTOS:
            case NPC_FLAMING_HEAD_MELEE:
            case NPC_FROZEN_HEAD_MELEE:
            case NPC_VENOMOUS_HEAD_MELEE:
            case NPC_JI_KUN:
            case NPC_DURUMU:
            case NPC_PRIMORDIUS:
            case NPC_DARK_ANIMUS:
            case NPC_IRON_QON:
            case NPC_ROSHAK:
            case NPC_QUETZAL:
            case NPC_DAMREN:
            case NPC_SULIN:
            case NPC_LULIN:
            case NPC_LEI_SHEN:
            case NPC_RA_DEN:
                return true;
            }
            return false;
        }

        //JiKun Nest Order
        void ActiveNextNest()
        {
            nestnumlist.clear();
            switch (nestmod)
            {
            case 1: //Lfr
            {
                switch (nestnum)
                {
                case 0:
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                case 1:
                    nestnumlist.push_back(3); //SouthWest low
                    break;
                case 2:
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 3:
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 4:
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 5:
                    nestnumlist.push_back(9); //Middle up
                    break;
                }
                break;
            }
            case 2: //10 normal/heroic
            {
                switch (nestnum)
                {
                case 0:
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                case 1:
                    nestnumlist.push_back(3); //SouthEast low
                    break;
                case 2:
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 3:
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 4:
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 5:
                    nestnumlist.push_back(9); //Middle up
                    break;
                case 6:
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                case 7:
                    nestnumlist.push_back(3); //SouthEast low
                    break;
                case 8:
                    nestnumlist.push_back(4); //SouthWest low
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 9:
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 10:
                    nestnumlist.push_back(9); //Middle up
                    break;
                case 11:
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                case 12:
                    nestnumlist.push_back(3); //SouthEast low
                    break;
                case 13:
                    nestnumlist.push_back(4); //SouthWest low
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 14:
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 15:
                    nestnumlist.push_back(9); //Middle up
                    break;
                }
                break;
            }
            case 3: //25 normal
            {
                switch (nestnum)
                {
                case 0:
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                case 1:
                    nestnumlist.push_back(3); //SouthEast low
                    break;
                case 2:
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 3:
                    nestnumlist.push_back(6); //West low
                    break;
                case 4:
                    nestnumlist.push_back(8); //NorthWest low
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 5:
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 6:
                    nestnumlist.push_back(9); //Middle up
                    break;
                case 7:
                    nestnumlist.push_back(0); //NorthEast low
                    nestnumlist.push_back(5); //SouthWest up
                    break;
                case 8:
                    nestnumlist.push_back(3); //SouthEast low
                    nestnumlist.push_back(7); //NorthWest up
                    break;
                case 9:
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 10:
                    nestnumlist.push_back(6); //West low
                    break;
                case 11:
                    nestnumlist.push_back(8); //NorthWest low
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 12:
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 13:
                    nestnumlist.push_back(0); //NorthEast low
                    nestnumlist.push_back(9); //Middle up
                    break;
                case 14:
                    nestnumlist.push_back(3); //SouthEast low
                    nestnumlist.push_back(5); //SouthWest up
                    break;
                case 15:
                    nestnumlist.push_back(4); //SouthWest low
                    nestnumlist.push_back(7); //NorthWest up
                    break;
                case 16:
                    nestnumlist.push_back(6); //West low
                    break;
                case 17:
                    nestnumlist.push_back(8); //NorthWest low
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 18:
                    nestnumlist.push_back(0); //NorthEast low
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 19:
                    nestnumlist.push_back(3); //SouthEast low
                    nestnumlist.push_back(9); //MIddle up
                    break;
                }
                break;
            }
            case 4: //25 heroic
            {
                switch (nestnum)
                {
                case 0:
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                case 1:
                    nestnumlist.push_back(3); //SouthEast low
                    break;
                case 2:
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 3:
                    nestnumlist.push_back(6); //West low
                    nestnumlist.push_back(1); //NorthEast up
                    break;
                case 4:
                    nestnumlist.push_back(8); //NorthWest low
                    nestnumlist.push_back(2); //SouthEast up
                    break;
                case 5:
                    nestnumlist.push_back(9); //Middle
                    break;
                case 6:
                    nestnumlist.push_back(0); //NorthEast low
                    nestnumlist.push_back(5); //SouthWest up
                    break;
                case 7:
                    nestnumlist.push_back(3); //SouthEast low
                    nestnumlist.push_back(7); //NorthWest up
                    break;
                case 8:
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 9:
                    nestnumlist.push_back(1); //NorthEast up
                    nestnumlist.push_back(6); //West low
                    break;
                case 10:
                    nestnumlist.push_back(2); //SouthEast up 
                    nestnumlist.push_back(8); //NorthWest low
                    break;
                case 11:
                    nestnumlist.push_back(0); //NorthEast low
                    nestnumlist.push_back(9); //Middle up
                    break;
                case 12:
                    nestnumlist.push_back(3); //SouthEast low
                    nestnumlist.push_back(4); //SouthWest low
                    break;
                case 13:
                    nestnumlist.push_back(1); //NorthEast up
                    nestnumlist.push_back(4); //SouthWest low
                    nestnumlist.push_back(7); //NorthWest up
                    break;
                case 14:
                    nestnumlist.push_back(2); //SouthEast up 
                    nestnumlist.push_back(6); //West low
                    break;
                case 15:
                    nestnumlist.push_back(0); //NorthEast low
                    nestnumlist.push_back(9); //Middle up
                    nestnumlist.push_back(8); //NorthWest low
                    break;
                case 16:
                    nestnumlist.push_back(3); //SouthEast low
                    nestnumlist.push_back(5); //SouthWest up
                    break;
                case 17:
                    nestnumlist.push_back(1); //NorthEast up
                    nestnumlist.push_back(4); //SouthWest low
                    nestnumlist.push_back(7); //NorthWest up
                    break;
                case 18:
                    nestnumlist.push_back(8); //NorthWest low
                    nestnumlist.push_back(2); //SouthEast up
                    nestnumlist.push_back(0); //NorthEast low
                    break;
                }
            }
            break;
            }

            if (!nestnumlist.empty())
                for (std::vector < uint8 >::const_iterator itr = nestnumlist.begin(); itr != nestnumlist.end(); itr++)
                    if (Creature* incubate = instance->GetCreature(jikunincubatelist[*itr]))
                        incubate->AI()->SetData(DATA_ACTIVE_NEST, 0);

            nestnumlist.clear();
            nestnum++;
            if (nestnum >= nestmaxcount)
                nestnum = 0;
        }

        std::string GetSaveData()
        {
            std::ostringstream saveStream;
            saveStream << GetBossSaveData() << " ";
            return saveStream.str();
        }

        void Load(const char* data)
        {
            std::istringstream loadStream(LoadBossState(data));
            uint32 buff;
            for (uint32 i = 0; i < 16; ++i)
                loadStream >> buff;
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_throne_of_thunder_InstanceMapScript(map);
    }
};

enum sSpells
{
    SPELL_STORM_WEAPON   = 139319,
    SPELL_STORM_ENERGY   = 139322,
    SPELL_CHAIN_LIGHTNIG = 139903,
    SPELL_STORMCLOUD     = 139900,
};

enum sEvent
{
    EVENT_STORM_ENERGY   = 1,
    EVENT_CHAIN_LIGHTNIG = 2,
};

//Mini boss, guard Jinrokh entrance
class npc_storm_caller : public CreatureScript
{
    public:
        npc_storm_caller() : CreatureScript("npc_storm_caller") { }
        
        struct npc_storm_callerAI : public BossAI
        {
            npc_storm_callerAI(Creature* pCreature) : BossAI(pCreature, DATA_STORM_CALLER)
            {
                pInstance = pCreature->GetInstanceScript();
            }
            
            InstanceScript* pInstance;

            void Reset()
            {
                _Reset();
                me->RemoveAurasDueToSpell(SPELL_STORM_WEAPON);
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                me->AddAura(SPELL_STORM_WEAPON, me);
                events.RescheduleEvent(EVENT_STORM_ENERGY, urand(15000, 20000));
            }

            void JustDied(Unit* killer)
            {
                _JustDied();
            }
            
            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_STORM_ENERGY)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_STORM_ENERGY);
                        events.RescheduleEvent(EVENT_STORM_ENERGY, urand(15000, 20000));
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_storm_callerAI(pCreature);
        }
};

//Mini boss, guard Horridon entrance
class npc_stormbringer : public CreatureScript
{
    public:
        npc_stormbringer() : CreatureScript("npc_stormbringer") { }
        
        struct npc_stormbringerAI : public BossAI
        {
            npc_stormbringerAI(Creature* pCreature) : BossAI(pCreature, DATA_STORMBRINGER)
            {
                pInstance = pCreature->GetInstanceScript();
            }
            
            InstanceScript* pInstance;

            void Reset()
            {
                _Reset();
                me->RemoveAurasDueToSpell(SPELL_STORMCLOUD);
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                DoCast(me, SPELL_STORMCLOUD);
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNIG, urand(15000, 20000));
            }

            void JustDied(Unit* killer)
            {
                _JustDied();
            }
            
            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_CHAIN_LIGHTNIG)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_CHAIN_LIGHTNIG);
                        events.RescheduleEvent(EVENT_STORM_ENERGY, urand(15000, 20000));
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_stormbringerAI(pCreature);
        }
};

Position const onbridge    = {6045.42f, 5163.28f, 148.1146f, 1.548f};
Position const underbridge = {6042.31f, 5088.96f,  -43.152f, 4.654f};

//Te;eport to Tortos, and back
class npc_teleporter : public CreatureScript
{
    public:
        npc_teleporter() : CreatureScript("npc_teleporter") {}

        struct npc_teleporterAI : public CreatureAI
        {
            npc_teleporterAI(Creature* creature) : CreatureAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->AddAura(126493, me); //Visual
            }

            InstanceScript* instance;

            void Reset(){}
            
            void OnSpellClick(Unit* clicker)
            {
                if (instance)
                {
                   if (clicker->GetTypeId() == TYPEID_PLAYER)
                   {
                       if (me->GetPositionZ() > 140.0f)
                           clicker->NearTeleportTo(underbridge.GetPositionX(), underbridge.GetPositionY(), underbridge.GetPositionZ(), underbridge.GetOrientation());
                       else
                           clicker->NearTeleportTo(onbridge.GetPositionX(), onbridge.GetPositionY(), onbridge.GetPositionZ(), onbridge.GetOrientation());
                   }
                }
            }
            
            void EnterEvadeMode(){}

            void EnterCombat(Unit* who){}

            void UpdateAI(uint32 diff){}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_teleporterAI(creature);
        }
};

void AddSC_instance_throne_of_thunder()
{
    new instance_throne_of_thunder();
    new npc_storm_caller();
    new npc_stormbringer();
    //new npc_teleporter();
}
