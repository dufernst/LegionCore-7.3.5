
//Siege of Orgrimmar

#include "AccountMgr.h"
#include "PlayerDefines.h"
#include "siege_of_orgrimmar.h"
#include "Packets/WorldStatePackets.h"

Position const LorewalkerChoSpawn[5]  = {
    {1448.236f, 312.6528f, 289.2837f, 4.652967f},
    {1441.406f, 988.1795f, 340.1876f, 1.985304f},   //fallen
    {806.067f,  841.3726f, 371.2589f, 1.791488f},   //norushen
    {805.7786f, 879.8768f, 371.0946f, 1.911932f},   //sha
    {761.5104f, 1048.512f, 357.2339f, 1.767873f},   //sha finish
};

uint32 bonusklaxxientry[6] =
{
    NPC_KAROZ,
    NPC_KORVEN,
    NPC_IYYOKYK,
    NPC_XARIL,
    NPC_KAZTIK,
    NPC_KILRUK,
};

uint32 removelist[30] =
{
    SPELL_HEWN,
    SPELL_ANGEL_OF_DEATH,
    SPELL_REAVE_PL,
    SPELL_PLAYER_REAVE,
    SPELL_COMPOUND_EYE,
    SPELL_SNIPE,
    SPELL_MAD_SCIENTIST,
    SPELL_GENETIC_ALTERATION,
    SPELL_GENE_SPLICE,
    SPELL_GENE_SPLICE_PLAYER,
    SPELL_TOXIN_RED,
    SPELL_TOXIN_BLUE,
    SPELL_TOXIN_YELLOW,
    SPELL_TOXIN_ORANGE,
    SPELL_TOXIN_PURPLE,
    SPELL_TOXIN_GREEN,
    SPELL_APOTHECARIAL_KNOWLEDGE,
    SPELL_VAST_APOTHECARIAL_KNOWLEDGE,
    SPELL_APOTHECARY_VOLATILE_POULTICE,
    SPELL_VOLATILE_POULTICE,
    SPELL_EXPOSED_VEINS,
    SPELL_TENDERIZING_STRIKES_DMG,
    SPELL_GENETIC_ALTERATION,
    SPELL_HUNGER,
    SPELL_INJECTION,
    SPELL_CAUSTIC_AMBER_AURA_DMG,
    SPELL_AIM_PLAYER,
    SPELL_MASTER_OF_AMBER,
    SPELL_MASTER_OF_AMBER2,
    SPELL_CANNED_HEAT_BASE,
};

Position const Sha_of_pride_Norushe  = {797.357f, 880.5637f, 371.1606f, 1.786108f };
Position Garroshroomcenterpos = { 1073.09f, -5639.70f, -317.3894f, 3.0128f };

//Active Weapon Point Positions(Blackfuse)
Position spawnaweaponpos[3] =
{
    {1915.95f, -5711.31f, -302.8854f},
    {1924.21f, -5723.62f, -302.8854f},
    {1933.46f, -5736.51f, -302.8854f},
};

Position destapos[3] =
{
    {1866.87f, -5637.11f, -302.8854f},
    {1874.14f, -5649.20f, -302.8854f},
    {1882.69f, -5661.84f, -302.8854f},
};

DoorData const doorData[] =
{
    {GO_IMMERSEUS_EX_DOOR,                   DATA_IMMERSEUS,              DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_SHA_FIELD,                           DATA_F_PROTECTORS,           DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_NORUSHEN_EX_DOOR,                    DATA_SHA_OF_PRIDE,           DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_ORGRIMMAR_GATE,                      DATA_IRON_JUGGERNAUT,        DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_RUSTY_BARS,                          DATA_KORKRON_D_SHAMAN,       DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_NAZGRIM_EX_DOOR,                     DATA_GENERAL_NAZGRIM,        DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_BLACKFUSE_ENT_DOOR,                  DATA_MALKOROK,               DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_ENT_GATE,                            DATA_MALKOROK,               DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_SP_EX_DOOR,                          DATA_SPOILS_OF_PANDARIA,     DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_KLAXXI_EX_DOOR,                      DATA_KLAXXI,                 DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {0,                                      0,                           DOOR_TYPE_ROOM,       BOUNDARY_NONE},
};

static uint32 _wavearray[6][4] =
{
    { 0, NPC_ACTIVATED_MISSILE_TURRET, NPC_BLACKFUSE_CRAWLER_MINE, NPC_ACTIVATED_LASER_TURRET },
    { 1, NPC_BLACKFUSE_CRAWLER_MINE, NPC_ACTIVATED_LASER_TURRET, NPC_ACTIVATED_MISSILE_TURRET },
    { 2, NPC_ACTIVATED_LASER_TURRET, NPC_ACTIVATED_ELECTROMAGNET, NPC_ACTIVATED_MISSILE_TURRET },
    { 3, NPC_ACTIVATED_LASER_TURRET, NPC_ACTIVATED_MISSILE_TURRET, NPC_BLACKFUSE_CRAWLER_MINE },
    { 4, NPC_ACTIVATED_MISSILE_TURRET, NPC_ACTIVATED_ELECTROMAGNET, NPC_BLACKFUSE_CRAWLER_MINE },
    { 5, NPC_BLACKFUSE_CRAWLER_MINE, NPC_ACTIVATED_LASER_TURRET, NPC_BLACKFUSE_CRAWLER_MINE },
};

uint32 _toxinlist[6] =
{
    SPELL_TOXIN_BLUE,
    SPELL_TOXIN_RED,
    SPELL_TOXIN_YELLOW,
    SPELL_TOXIN_ORANGE,
    SPELL_TOXIN_PURPLE,
    SPELL_TOXIN_GREEN,
};

uint32 weaponpriority[4] =
{
    NPC_ACTIVATED_ELECTROMAGNET,
    NPC_BLACKFUSE_CRAWLER_MINE,
    NPC_ACTIVATED_MISSILE_TURRET,
    NPC_ACTIVATED_LASER_TURRET,
};

class instance_siege_of_orgrimmar : public InstanceMapScript
{
public:
    instance_siege_of_orgrimmar() : InstanceMapScript("instance_siege_of_orgrimmar", 1136) { }

    struct instance_siege_of_orgrimmar_InstanceMapScript : public InstanceScript
    {
        instance_siege_of_orgrimmar_InstanceMapScript(Map* map) : InstanceScript(map) {}

        std::map<uint32, ObjectGuid> easyGUIDconteiner;
        //count killed klaxxi
        uint8 klaxxidiecount;
        //count weapons finish move
        uint8 weaponsdone;
        uint8 crawlerminenum;
        uint8 rycount;
        uint32 lastsuperheatweapon;
        uint32 newsuperheatweapon;
        //Misc
        uint32 TeamInInstance;
        uint32 EventfieldOfSha;
        uint32 lingering_corruption_count;

        //Galakras worldstate
        uint16 ShowCannon;
        uint16 CannonCount;
        uint32 ShowSouthTower;
        uint32 ShowNorthTower;
        uint32 SouthTowerCount;
        uint32 NorthTowerCount;

        //GameObjects
        ObjectGuid immerseusexdoorGUID;
        ObjectGuid fprotectorexdoorGUID;
        ObjectGuid chestShaVaultOfForbiddenTreasures;
        GuidVector lightqGUIDs;
        ObjectGuid northropeGuid;
        ObjectGuid northropeskeinGuid;
        ObjectGuid southropeGuid;
        ObjectGuid southropeskeinGuid;
        ObjectGuid winddoorGuid;
        ObjectGuid orgrimmargateGuid;
        ObjectGuid orgrimmargate2Guid;
        ObjectGuid rustybarsGuid;
        ObjectGuid nazgrimdoorGuid;
        ObjectGuid nazgrimexdoorGuid;
        GuidVector malkorokfenchGuids;
        GuidVector roomgateGuids;
        GuidVector roomdoorGuids;
        GuidVector irondoorGuids;
        GuidVector leverGuids;
        GuidVector sopboxGuids;
        GuidVector spoilsGuids;  //for send frames
        GuidVector spoils2Guids; //find players and send aura bar in radius
        ObjectGuid gossopsGuid;
        ObjectGuid gonsopsGuid;
        ObjectGuid spentdoorGuid;
        ObjectGuid klaxxientdoorGuid;
        ObjectGuid spexdoorGuid;
        ObjectGuid thokentdoorGuid;
        GuidVector jaillistGuids;
        GuidVector klaxxiarenagateGuid;
        ObjectGuid blackfuseentdoorGuid;
        GuidVector garroshfenchGuids;
        GuidVector soldierfenchGuids;
        ObjectGuid klaxxiexdoorGuid;
        ObjectGuid garroshentdoorGuid;
        GuidVector goshavortexGuids;
        
        //Creature
        GuidSet shaSlgGUID;
        GuidVector PortalOrgrimmarGUID;
        GuidVector MeasureGUID;
        ObjectGuid LorewalkerChoGUIDtmp;
        ObjectGuid fpGUID[3];
        GuidVector rookmeasureGuids;
        GuidVector sunmeasureGuids;
        ObjectGuid hemeasureGuid;
        ObjectGuid WrynOrLorthemarGUID;
        ObjectGuid JainaOrSylvanaGUID;
        ObjectGuid VereesaOrAethasGUID;
        ObjectGuid sExpertGUID;
        ObjectGuid nExpertGUID;
        ObjectGuid ironjuggGuid;
        ObjectGuid kardrisGuid;
        ObjectGuid harommGuid;
        ObjectGuid bloodclawGuid;
        ObjectGuid darkfangGuid;
        ObjectGuid gnazgrimGuid;
        ObjectGuid amGuid;
        ObjectGuid npcssopsGuid;
        GuidVector npcleverlistGuids;
        GuidVector klaxxilist; //all klaxxi
        ObjectGuid amberpieceGuid;
        ObjectGuid klaxxicontrollerGuid;
        ObjectGuid thokGuid;
        GuidVector prisonerGuids;
        GuidVector dweaponGuids;
        std::vector<uint32> aweaponentry;
        ObjectGuid bsGuid;
        ObjectGuid blackfuseGuid;
        GuidVector crawlermineGuids;
        ObjectGuid swmstalkerGuid;
        ObjectGuid electromagnetGuid;
        ObjectGuid laserturretGuid;
        ObjectGuid heartofyshaarjGuid;
        GuidVector shavortexGuids;
        GuidVector edespairGuids;
        GuidVector edoubtGuids;
        GuidVector efearGuids;
        ObjectGuid garroshGuid;
        ObjectGuid garroshrealmGuid;
        ObjectGuid garroshstormwindGuid;
        GuidVector engeneerGuids;
        GuidVector garroshsoldiersGuids;
        ObjectGuid korkrongunshipGuid;
        GuidVector hordecannonlistGuids;

        EventMap Events;

        bool onInitEnterState;
        
        bool STowerFull;
        bool NTowerFull;
        bool STowerNull;
        bool NTowerNull;

        Transport* transport;

        ~instance_siege_of_orgrimmar_InstanceMapScript()
        {
            delete transport;
        }

        void Initialize()
        {
            SetBossNumber(DATA_MAX);
            LoadDoorData(doorData);

            klaxxidiecount = 0;
            weaponsdone = 0;
            TeamInInstance = 0;
            lingering_corruption_count = 0;
            crawlerminenum = 0;
            rycount = 0;
            lastsuperheatweapon = 0;

            //GameObject
            immerseusexdoorGUID.Clear();
            chestShaVaultOfForbiddenTreasures.Clear();
            lightqGUIDs.clear();
            northropeGuid.Clear();
            northropeskeinGuid.Clear();
            southropeGuid.Clear();
            southropeskeinGuid.Clear();
            winddoorGuid.Clear();
            orgrimmargateGuid.Clear();
            orgrimmargate2Guid.Clear();
            rustybarsGuid.Clear();
            nazgrimdoorGuid.Clear();
            nazgrimexdoorGuid.Clear();
            malkorokfenchGuids.clear();
            roomgateGuids.clear();
            roomdoorGuids.clear();
            irondoorGuids.clear();
            leverGuids.clear();
            sopboxGuids.clear();
            spoilsGuids.clear();
            spoils2Guids.clear();
            gossopsGuid.Clear();
            gonsopsGuid.Clear();
            klaxxientdoorGuid.Clear();
            spentdoorGuid.Clear();
            spexdoorGuid.Clear();
            thokentdoorGuid.Clear();
            jaillistGuids.clear();
            klaxxiarenagateGuid.clear();
            blackfuseentdoorGuid.Clear();
            garroshfenchGuids.clear();
            soldierfenchGuids.clear();
            garroshentdoorGuid.Clear();
            goshavortexGuids.clear();
           
            //Creature
            LorewalkerChoGUIDtmp.Clear();
            rookmeasureGuids.clear();
            sunmeasureGuids.clear();
            hemeasureGuid.Clear();
            memset(fpGUID, 0, 3 * sizeof(ObjectGuid));
            EventfieldOfSha     = 0;
            WrynOrLorthemarGUID.Clear();
            JainaOrSylvanaGUID.Clear();
            VereesaOrAethasGUID.Clear();
            sExpertGUID.Clear();
            nExpertGUID.Clear();
            ironjuggGuid.Clear();
            kardrisGuid.Clear();
            harommGuid.Clear();
            bloodclawGuid.Clear();
            darkfangGuid.Clear();
            gnazgrimGuid.Clear();
            amGuid.Clear();
            npcssopsGuid.Clear();
            npcleverlistGuids.clear();
            klaxxilist.clear();
            amberpieceGuid.Clear();
            klaxxicontrollerGuid.Clear();
            thokGuid.Clear();
            prisonerGuids.clear();
            bsGuid.Clear();
            dweaponGuids.clear();
            aweaponentry.clear();
            blackfuseGuid.Clear();
            crawlermineGuids.clear();
            swmstalkerGuid.Clear();
            electromagnetGuid.Clear();
            laserturretGuid.Clear();
            heartofyshaarjGuid.Clear();
            shavortexGuids.clear();
            edespairGuids.clear();
            edoubtGuids.clear();
            efearGuids.clear();
            garroshGuid.Clear();
            garroshrealmGuid.Clear();
            engeneerGuids.clear();
            garroshsoldiersGuids.clear();
            korkrongunshipGuid.Clear();
            hordecannonlistGuids.clear();

            onInitEnterState = false;
            STowerFull = false;
            NTowerFull = false;
            STowerNull = false;
            NTowerNull = false;
            transport = NULL;
            
            //Galakras WorldState
            ShowCannon            = NOT_STARTED;
            ShowSouthTower        = NOT_STARTED;
            ShowNorthTower        = NOT_STARTED;

            CannonCount     = 0;
            SouthTowerCount = 0;
            NorthTowerCount = 0;
        }

        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override
        {   
            packet.Worldstates.emplace_back(WorldStates::WS_SHOW_SOUTH_TOWER, ShowSouthTower == IN_PROGRESS);
            packet.Worldstates.emplace_back(WorldStates::WS_SHOW_NORTH_TOWER, ShowNorthTower == IN_PROGRESS);
            packet.Worldstates.emplace_back(WorldStates::WS_SHOW_CAPTURE_SOUTH_TOWER, ShowSouthTower == SPECIAL);
            packet.Worldstates.emplace_back(WorldStates::WS_SHOW_CAPTURE_NORTH_TOWER, ShowNorthTower == SPECIAL);
            packet.Worldstates.emplace_back(WorldStates::WS_SOUTH_TOWER, SouthTowerCount);
            packet.Worldstates.emplace_back(WorldStates::WS_NORTH_TOWER, NorthTowerCount);
            packet.Worldstates.emplace_back(WorldStates::WS_CAPTURE_SOUTH_TOWER, SouthTowerCount);
            packet.Worldstates.emplace_back(WorldStates::WS_CAPTURE_NORTH_TOWER, NorthTowerCount);
            packet.Worldstates.emplace_back(WorldStates::WS_SHOW_KORKRON_CANNON, ShowCannon == IN_PROGRESS);
            packet.Worldstates.emplace_back(WorldStates::WS_KORKRON_CANNON_COUNT, CannonCount);
        }

        void OnPlayerEnter(Player* player)
        {
            if (!TeamInInstance)
                TeamInInstance = player->GetTeam();
            
            if (GetBossState(DATA_SHA_OF_PRIDE) == IN_PROGRESS)
                player->CastSpell(player, 144343, true);
            
            if (GetBossState(DATA_NORUSHEN) == IN_PROGRESS)
                player->CastSpell(player, 144421, true);

            //Custom check.
            CustomSpellCheck(player);

            //not handle lorewalker summon if already done.
            if (onInitEnterState)
                return;
            onInitEnterState = true;

            DoSummoneEventCreatures();

            if (!transport)
                transport = sTransportMgr->CreateTransport(TeamInInstance == HORDE ? GO_SHIP_HORDE : GO_SHIP_ALLIANCE, 0, player->GetMap());

            // transport->EnableMovement(false);

            //Crash or disconnect when Klaxxi in progress
            for (uint8 n = 0; n < 6; n++)
                player->RemoveAurasDueToSpell(_toxinlist[n]);
        }

        void OnPlayerLeave(Player* player)
        {
            for (uint8 n = 0; n < 30; ++n)
                DoRemoveAurasDueToSpellOnPlayers(removelist[n]);
        }

        void RemoveDebuffFromPlayers()
        {
            for (uint8 n = 0; n < 30; ++n)
                DoRemoveAurasDueToSpellOnPlayers(removelist[n]);
        }

        //Some auras should not stay after relog. If player out of dung whey remove automatically
        //but if player on dungeon he could use it.
        void CustomSpellCheck(Player* player)
        {
            if (GetBossState(DATA_SHA_OF_PRIDE) != IN_PROGRESS)
            {
                //Sha of pride: SPELL_OVERCOME
                if (player->HasAura(144843))
                    player->RemoveAura(144843);

                //Sha of pride: SPELL_PRIDE
                if (player->HasAura(144343))
                    player->RemoveAura(144343);
            }
            if (GetBossState(DATA_NORUSHEN) != IN_PROGRESS)
            {
                //Norushen: Coruption
                if (player->HasAura(144421))
                    player->RemoveAura(144421);

                //Norushen: PURIFIED
                if (player->HasAura(144452))
                    player->RemoveAura(144452);
            }
        }

        void DoSummoneEventCreatures()
        {
            if (GetBossState(DATA_IMMERSEUS) != DONE)
            {
                if (Creature* cho = instance->SummonCreature(NPC_LOREWALKER_CHO, LorewalkerChoSpawn[0]))
                {
                    cho->setActive(true);
                    cho->AI()->SetData(DATA_IMMERSEUS, NOT_STARTED);
                }
            }
            else if (GetBossState(DATA_F_PROTECTORS) != DONE)
            {
                if (Creature* cho = instance->SummonCreature(NPC_LOREWALKER_CHO, LorewalkerChoSpawn[1]))
                {
                    cho->setActive(true);
                    cho->AI()->SetData(DATA_F_PROTECTORS, NOT_STARTED);
                }
            }
            else if (GetBossState(DATA_NORUSHEN) != DONE)
            {
                if (Creature* cho = instance->SummonCreature(NPC_LOREWALKER_CHO2, LorewalkerChoSpawn[2]))
                    cho->setActive(true);
            }
            else if (GetBossState(DATA_SHA_OF_PRIDE) != DONE)
            {
                if (Creature * c = instance->SummonCreature(NPC_SHA_NORUSHEN, Sha_of_pride_Norushe))
                    c->setActive(true);
                if (Creature * c = instance->SummonCreature(NPC_LOREWALKER_CHO3, LorewalkerChoSpawn[3]))
                    c->setActive(true);
            }
            else if (GetBossState(DATA_GALAKRAS) != DONE)
            {
                if (Creature * c = instance->SummonCreature(NPC_LOREWALKER_CHO3, LorewalkerChoSpawn[4]))
                {
                    c->setActive(true);
                    c->AI()->DoAction(EVENT_2);
                }
            }
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_LOREWALKER_CHO:
                case NPC_LOREWALKER_CHO2:
                case NPC_LOREWALKER_CHO3:
                    LorewalkerChoGUIDtmp = creature->GetGUID();
                    break;
                case NPC_IMMERSEUS:
                case NPC_PUDDLE_POINT:
                case NPC_GOLD_LOTOS_MOVER:
                case NPC_GOLD_LOTOS_MAIN:
                case NPC_GOLD_LOTOS_HE:
                case NPC_GOLD_LOTOS_SUN:
                case NPC_SHA_NORUSHEN:
                case NPC_SHA_TARAN_ZHU:
                case NPC_SHA_OF_PRIDE_END_LADY_JAINA:
                case NPC_SHA_OF_PRIDE_END_THERON:
                case NPC_NORUSHEN:
                case NPC_AMALGAM_OF_CORRUPTION:
                case NPC_B_H_CONTROLLER:
                case NPC_BLIND_HATRED:
                case NPC_GALAKRAS:
                case NPC_WARLORD_ZAELA:
                case NPC_TOWER_SOUTH:
                case NPC_TOWER_NORTH:
                case NPC_ANTIAIR_TURRET:
                case NPC_BLACKFUSE:
                    easyGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                break;
           
                //Fallen Protectors
                case NPC_ROOK_STONETOE: 
                    fpGUID[0] = creature->GetGUID();
                    break;
                case NPC_SUN_TENDERHEART:
                    fpGUID[1] = creature->GetGUID();
                    break;
                case NPC_HE_SOFTFOOT:
                    fpGUID[2] = creature->GetGUID();
                    break;
                case NPC_EMBODIED_MISERY_OF_ROOK:
                case NPC_EMBODIED_GLOOM_OF_ROOK:
                case NPC_EMBODIED_SORROW_OF_ROOK:
                    if (creature->ToTempSummon())
                        if (creature->ToTempSummon()->GetSummoner()->GetGUID() == fpGUID[0])
                            rookmeasureGuids.push_back(creature->GetGUID());
                    break;
                case NPC_EMBODIED_DESPERATION_OF_SUN:
                case NPC_EMBODIED_DESPIRE_OF_SUN:
                    if (creature->ToTempSummon())
                        if (creature->ToTempSummon()->GetSummoner()->GetGUID() == fpGUID[1])
                            sunmeasureGuids.push_back(creature->GetGUID());
                    break;
                case NPC_EMBODIED_ANGUISH_OF_HE:
                    if (creature->ToTempSummon())
                        if (creature->ToTempSummon()->GetSummoner()->GetGUID() == fpGUID[2])
                            hemeasureGuid = creature->GetGUID();
                    break;
                //Sha
                case NPC_SHA_OF_PRIDE:
                    easyGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                    creature->SetVisible(false);
                    break;
                case NPC_LINGERING_CORRUPTION:
                    ++lingering_corruption_count;
                    if (!creature->isAlive())
                        creature->Respawn(true);
                    break;
                case NPC_SLG_GENERIC_MOP:
                    shaSlgGUID.insert(creature->GetGUID());
                    break;
                case NPC_PORTAL_TO_ORGRIMMAR:
                    PortalOrgrimmarGUID.push_back(creature->GetGUID());
                    creature->SetDisplayId(51795);
                    creature->SetVisible((GetBossState(DATA_SHA_OF_PRIDE)==DONE) ? true : false);
                    break;

                //Galakras:
                case NPC_KING_VARIAN_WRYNN_A:
                case NPC_LORTHEMAR_THERON_H:
                    WrynOrLorthemarGUID = creature->GetGUID();
                    easyGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                    break;
                case NPC_LADY_JAINA_PROUDMOORE_A:
                case NPC_LADY_SYLVANAS_WINDRUNNER_H:
                    JainaOrSylvanaGUID = creature->GetGUID();
                    easyGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                    break;
                case NPC_VEREESA_WINDRUNNER_A:
                case NPC_ARCHMAGE_AETHAS_SUNREAVER_H:
                    VereesaOrAethasGUID = creature->GetGUID();
                    easyGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                    break;
                case NPC_DEMOLITIONS_EXPERT_S_A:
                case NPC_DEMOLITIONS_EXPERT_S_H:
                    sExpertGUID = creature->GetGUID();
                    break;
                case NPC_DEMOLITIONS_EXPERT_N_A:
                case NPC_DEMOLITIONS_EXPERT_N_H:
                    nExpertGUID = creature->GetGUID();
                    break;
                case NPC_IRON_JUGGERNAUT:
                    ironjuggGuid = creature->GetGUID();
                    break;
                //Korkron Dark Shamans
                case NPC_WAVEBINDER_KARDRIS:
                    kardrisGuid = creature->GetGUID();
                    break;
                case NPC_EARTHBREAKER_HAROMM:
                    harommGuid = creature->GetGUID();
                    break;
                case NPC_BLOODCLAW:
                    bloodclawGuid = creature->GetGUID();
                    break;
                case NPC_DARKFANG:
                    darkfangGuid = creature->GetGUID();
                    break;
                //
                //General Nazgrim
                case NPC_GENERAL_NAZGRIM:
                    gnazgrimGuid = creature->GetGUID();
                    break;
                //Malkorok
                case NPC_ANCIENT_MIASMA:
                    amGuid = creature->GetGUID();
                    break;
                //Spoils of Pandaria
                case NPC_SSOP_SPOILS:
                    npcssopsGuid = creature->GetGUID();
                    break;
                case NPC_MOGU_SPOILS:
                case NPC_MOGU_SPOILS2:
                case NPC_MANTIS_SPOILS:
                case NPC_MANTIS_SPOILS2:
                    if (uint32(creature->GetPositionZ()) == -271)
                        spoilsGuids.push_back(creature->GetGUID());
                    else
                        spoils2Guids.push_back(creature->GetGUID());
                    break;
                case NPC_LEVER:
                    npcleverlistGuids.push_back(creature->GetGUID());
                    break;
                //Paragons of the Klaxxi
                case NPC_KILRUK:
                case NPC_XARIL:
                case NPC_KAZTIK:
                case NPC_KORVEN:
                case NPC_IYYOKYK:
                case NPC_KAROZ:
                case NPC_SKEER:
                case NPC_RIKKAL:
                case NPC_HISEK:
                    klaxxilist.push_back(creature->GetGUID());
                    break;
                case NPC_AMBER_PIECE:
                    amberpieceGuid = creature->GetGUID();
                    break;
                case NPC_KLAXXI_CONTROLLER:
                    klaxxicontrollerGuid = creature->GetGUID();
                    break;
                //Thok
                case NPC_THOK:
                    thokGuid = creature->GetGUID();
                    break;
                case NPC_BODY_STALKER:
                    bsGuid = creature->GetGUID();
                    break;
                //Prisoners
                case NPC_AKOLIK:
                case NPC_MONTAK:
                case NPC_WATERSPEAKER_GORAI:
                    prisonerGuids.push_back(creature->GetGUID());
                    break;
                //BlackFuse
                case NPC_BLACKFUSE_MAUNT:
                    blackfuseGuid = creature->GetGUID();
                    break;
                case NPC_DISASSEMBLED_CRAWLER_MINE:
                case NPC_DEACTIVATED_LASER_TURRET:
                case NPC_DEACTIVATED_ELECTROMAGNET:
                case NPC_DEACTIVATED_MISSILE_TURRET:
                    dweaponGuids.push_back(creature->GetGUID());
                    break;
                case NPC_BLACKFUSE_CRAWLER_MINE:
                    crawlermineGuids.push_back(creature->GetGUID());
                    break;
                case NPC_SHOCKWAVE_MISSILE_STALKER:
                    if (!creature->ToTempSummon())
                        swmstalkerGuid = creature->GetGUID();
                    break;
                case NPC_ACTIVATED_ELECTROMAGNET:
                    electromagnetGuid = creature->GetGUID();
                    break;
                //Garrosh
                case NPC_GARROSH:
                    if (Unit* garrosh = creature->ToUnit())
                    {
                        if (garrosh->ToTempSummon())
                        {
                            if (garrosh->GetMap()->GetAreaId(garrosh->GetPositionX(), garrosh->GetPositionY(), garrosh->GetPositionZ()) == 6816)
                                garroshstormwindGuid = creature->GetGUID();
                            else
                                garroshrealmGuid = creature->GetGUID();
                        }
                        else
                            garroshGuid = creature->GetGUID();
                    }
                    break;
                case NPC_HEART_OF_YSHAARJ:
                    heartofyshaarjGuid = creature->GetGUID();
                    break;
                case NPC_SHA_VORTEX:
                    shavortexGuids.push_back(creature->GetGUID());
                    break;
                case NPC_EMBODIED_DESPAIR:
                    edespairGuids.push_back(creature->GetGUID());
                    creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
                    break;
                case NPC_EMBODIED_DOUBT:
                    edoubtGuids.push_back(creature->GetGUID());
                    creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
                    break;
                case NPC_EMBODIED_FEAR:
                    efearGuids.push_back(creature->GetGUID());
                    creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                    creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                    creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
                    break;
                case NPC_SIEGE_ENGINEER:
                    engeneerGuids.push_back(creature->GetGUID());
                    break;
                case NPC_WARBRINGER:
                case NPC_WOLF_RIDER:
                    garroshsoldiersGuids.push_back(creature->GetGUID());
                    break;
                case NPC_KORKRON_GUNSHIP:
                    korkrongunshipGuid = creature->GetGUID();
                    break;
                case NPC_HORDE_CANNON:
                    hordecannonlistGuids.push_back(creature->GetGUID());
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_NORUSHEN_EX_DOOR:
                case GO_CORRUPTED_PRISON_WEST:
                case GO_CORRUPTED_BUTTON_WEST_1:
                case GO_CORRUPTED_BUTTON_WEST_2:
                case GO_CORRUPTED_BUTTON_WEST_3:
                case GO_CORRUPTED_PRISON_EAST:
                case GO_CORRUPTED_BUTTON_EAST_1:
                case GO_CORRUPTED_BUTTON_EAST_2:
                case GO_CORRUPTED_BUTTON_EAST_3:
                case GO_CORRUPTED_PRISON_NORTH:
                case GO_CORRUPTED_BUTTON_NORTH_1:
                case GO_CORRUPTED_BUTTON_NORTH_2:
                case GO_CORRUPTED_BUTTON_NORTH_3:
                case GO_CORRUPTED_PRISON_SOUTH:
                case GO_CORRUPTED_BUTTON_SOUTH_1:
                case GO_CORRUPTED_BUTTON_SOUTH_2:
                case GO_CORRUPTED_BUTTON_SOUTH_3:
                case GO_SOUTH_DOOR:
                case GO_NORTH_DOOR:
                    easyGUIDconteiner[go->GetEntry()] = go->GetGUID();
                    break;
                case GO_VAULT_OF_FORBIDDEN_TREASURES_1:
                    chestShaVaultOfForbiddenTreasures = go->GetGUID();
                    break;
                case GO_IMMERSEUS_EX_DOOR:
                    AddDoor(go, true);
                    break;
                case GO_SHA_FIELD:
                    AddDoor(go, true);
                    fprotectorexdoorGUID = go->GetGUID();
                    break;
                case GO_LIGTH_QUARANTINE:
                case GO_LIGTH_QUARANTINE_2:
                case GO_LIGTH_QUARANTINE_3:
                case GO_LIGTH_QUARANTINE_4:
                case GO_LIGTH_QUARANTINE_5:
                case GO_LIGTH_QUARANTINE_6:
                case GO_LIGHT_RAY_01:
                case GO_LIGHT_RAY_02:
                case GO_LIGHT_RAY_03:
                case GO_LIGHT_RAY_04:
                case GO_LIGHT_RAY_05:
                case GO_LIGHT_RAY_06:
                case GO_LIGHT_RAY_07:
                case GO_LIGHT_RAY_08:
                case GO_LIGHT_RAY_09:
                case GO_LIGHT_RAY_10:
                case GO_LIGHT_RAY_11:
                case GO_LIGHT_RAY_12:
                case GO_LIGHT_RAY_13:
                case GO_LIGHT_RAY_14:
                case GO_LIGHT_RAY_15:
                case GO_LIGHT_RAY_16:
                    go->setIgnorePhaseIdCheck(true);
                    lightqGUIDs.push_back(go->GetGUID());
                    break;
                case GO_SHA_ENERGY_WALL:
                    easyGUIDconteiner[go->GetEntry()] = go->GetGUID();
                    if (EventfieldOfSha >= 3)
                        HandleGameObject(go->GetGUID(), true, go);
                    break;
                case GO_ROPE:
                    if (go->GetPositionZ() > 35.0f && go->GetPositionZ() < 40.0f)
                        southropeGuid = go->GetGUID();
                    else if (go->GetPositionZ() < 35.0f)
                        northropeGuid = go->GetGUID();
                    break;
                case GO_NORTH_ROPE_SKEIN:
                    northropeskeinGuid = go->GetGUID();
                    break;
                case GO_SOUTH_ROPE_SKEIN:
                    southropeskeinGuid = go->GetGUID();
                    break;
                case GO_WIND_DOOR:
                    winddoorGuid = go->GetGUID();
                    break;
                case GO_ORGRIMMAR_GATE:
                    AddDoor(go, true);
                    orgrimmargateGuid = go->GetGUID();
                    break;
                case GO_ORGRIMMAR_GATE2:
                    orgrimmargate2Guid = go->GetGUID();
                    break;
                case GO_RUSTY_BARS:
                    rustybarsGuid = go->GetGUID();
                    break;
                case GO_NAZGRIM_DOOR:
                    nazgrimdoorGuid = go->GetGUID();
                    break;
                case GO_NAZGRIM_EX_DOOR:
                    AddDoor(go, true);
                    nazgrimexdoorGuid = go->GetGUID();
                    break;
                case GO_MALKOROK_FENCH:
                case GO_MALKOROK_FENCH_2:
                    malkorokfenchGuids.push_back(go->GetGUID());
                    break;
                case GO_BLACKFUSE_ENT_DOOR:
                    AddDoor(go, true);
                    blackfuseentdoorGuid = go->GetGUID();
                    break;
                case GO_ENT_GATE:
                    AddDoor(go, true);
                    spentdoorGuid = go->GetGUID();
                    break;
                case GO_SP_EX_DOOR:
                    AddDoor(go, true);
                    spexdoorGuid = go->GetGUID();
                    break;
                //Thok
                case GO_THOK_ENT_DOOR:
                    thokentdoorGuid = go->GetGUID();
                    break;
                case GO_JINUI_JAIL:
                case GO_JINUI_JAIL2:
                case GO_SAUROK_JAIL:
                case GO_SAUROK_JAIL2:
                case GO_YAUNGOLIAN_JAIL:
                case GO_YAUNGOLIAN_JAIL2:
                    jaillistGuids.push_back(go->GetGUID());
                    break;
                //Spoils of pandaria
                case GO_SSOP_SPOILS:
                    if (GetBossState(DATA_SPOILS_OF_PANDARIA) != DONE)
                    {
                        SetBossState(DATA_SPOILS_OF_PANDARIA, NOT_STARTED);
                        gossopsGuid = go->GetGUID();
                    }
                    else if ((GetBossState(DATA_SPOILS_OF_PANDARIA) == DONE))
                        go->Delete();
                    break;
                case GO_NSOP_SPOILS:
                    gonsopsGuid = go->GetGUID();
                    break;
                case GO_SMALL_MOGU_BOX:
                case GO_MEDIUM_MOGU_BOX:
                case GO_BIG_MOGU_BOX:
                case GO_SMALL_MANTIS_BOX:
                case GO_MEDIUM_MANTIS_BOX:
                case GO_BIG_MANTIS_BOX:
                case GO_PANDAREN_RELIC_BOX:
                    sopboxGuids.push_back(go->GetGUID());
                    break;
                case GO_ENT_DOOR_LEFT:
                case GO_ENT_DOOR_RIGHT:
                case GO_EX_DOOR_RIGHT:
                case GO_EX_DOOR_LEFT:
                    roomdoorGuids.push_back(go->GetGUID());
                    break;
                case GO_ROOM_GATE:
                case GO_ROOM_GATE2:
                case GO_ROOM_GATE3:
                case GO_ROOM_GATE4:
                    roomgateGuids.push_back(go->GetGUID());
                    break;
                case GO_IRON_DOOR_R:
                case GO_IRON_DOOR_L:
                    irondoorGuids.push_back(go->GetGUID());
                    break;
                case GO_LEVER_R:
                case GO_LEVER_L:
                    leverGuids.push_back(go->GetGUID());
                    break;
                //Paragons of Klaxxi
                case GO_PRE_ENT_KLAXXI_DOOR:
                    klaxxientdoorGuid = go->GetGUID();
                    CheckProgressForKlaxxi();
                    break;
                case GO_ARENA_WALL:
                    klaxxiarenagateGuid.push_back(go->GetGUID());
                    break;
                case GO_KLAXXI_EX_DOOR:
                    AddDoor(go, true);
                    klaxxiexdoorGuid = go->GetGUID();
                    break;
                case GO_GARROSH_FENCH:
                case GO_GARROSH_FENCH2:
                    garroshfenchGuids.push_back(go->GetGUID());
                    break;
                case GO_SOLDIER_RIGHT_DOOR:
                case GO_SOLDIER_LEFT_DOOR:
                    soldierfenchGuids.push_back(go->GetGUID());
                    break;
                case GO_GARROSH_ENT_DOOR:
                    garroshentdoorGuid = go->GetGUID();
                    break;
                case GO_SHA_VORTEX:
                    goshavortexGuids.push_back(go->GetGUID());
                    break;
            }
        }

        void CheckProgressForKlaxxi()
        {
            if (GetBossState(DATA_SPOILS_OF_PANDARIA) == DONE && GetBossState(DATA_THOK) == DONE && GetBossState(DATA_BLACKFUSE) == DONE)
                HandleGameObject(klaxxientdoorGuid, true);
        }

        bool CheckProgressForGarrosh()
        {
            for (uint32 n = 0; n < DATA_GARROSH; n++)
                if (GetBossState(n) != DONE)
                    return false;
            return true;
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            //Privent overwrite state.
            if (GetBossState(id) == DONE)
                return false;

            if (!InstanceScript::SetBossState(id, state))
                return false;
            
            switch (id)
            {
            case DATA_IMMERSEUS:
                if (state == DONE)
                    if (Creature* bq = instance->GetCreature(LorewalkerChoGUIDtmp))
                        bq->AI()->SetData(DATA_IMMERSEUS, DONE);
                break;
            case DATA_F_PROTECTORS:
                if (state == DONE)
                {
                    HandleGameObject(fprotectorexdoorGUID, true);
                    if (Creature* bq = instance->GetCreature(LorewalkerChoGUIDtmp))
                        bq->AI()->SetData(DATA_F_PROTECTORS, DONE);
                }
                break;
            case DATA_NORUSHEN:
            {
                switch (state)
                {
                case NOT_STARTED:
                    for (std::vector<ObjectGuid>::const_iterator guid = lightqGUIDs.begin(); guid != lightqGUIDs.end(); guid++)
                        HandleGameObject(*guid, true);
                    break;
                case IN_PROGRESS:
                    for (std::vector<ObjectGuid>::const_iterator guid = lightqGUIDs.begin(); guid != lightqGUIDs.end(); guid++)
                        HandleGameObject(*guid, false);
                    break;
                case DONE:
                    for (std::vector<ObjectGuid>::const_iterator guid = lightqGUIDs.begin(); guid != lightqGUIDs.end(); guid++)
                        HandleGameObject(*guid, true);
                    if (Creature* norush = instance->GetCreature(GetGuidData(NPC_NORUSHEN)))
                        norush->DespawnOrUnsummon();
                    if (Creature* bq = instance->GetCreature(LorewalkerChoGUIDtmp))
                        bq->DespawnOrUnsummon();
                    break;
                }
                break;
            }
            case DATA_SHA_OF_PRIDE:
                if (state == DONE)
                {
                    if (!instance->IsLfr())
                        if (GameObject* pChest = instance->GetGameObject(chestShaVaultOfForbiddenTreasures))
                            pChest->SetRespawnTime(pChest->GetRespawnDelay());
                    if (GetData(DATA_GALAKRAS_PRE_EVENT) != IN_PROGRESS)
                    {
                        if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                            Galakras->AI()->DoAction(ACTION_PRE_EVENT);
                        SetData(DATA_GALAKRAS_PRE_EVENT, IN_PROGRESS);
                    }
                }
                break;
            case DATA_GALAKRAS:
            {
                switch (state)
                {
                case NOT_STARTED:
                    SetData(DATA_SOUTH_TOWER, NOT_STARTED);
                    SetData(DATA_NORTH_TOWER, NOT_STARTED);
                    SetData(DATA_DISABLE_ROPES, 0);
                    STowerFull = false;
                    STowerNull = false;
                    NTowerFull = false;
                    NTowerNull = false;
                    if (GameObject* SouthDoor = instance->GetGameObject(GetGuidData(GO_SOUTH_DOOR)))
                        SouthDoor->SetGoState(GO_STATE_READY);
                    if (GameObject* NorthDoor = instance->GetGameObject(GetGuidData(GO_NORTH_DOOR)))
                        NorthDoor->SetGoState(GO_STATE_READY);
                    if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                    {
                        Galakras->AI()->Reset();
                        Galakras->AI()->EnterEvadeMode();
                    }
                    break;
                case IN_PROGRESS:
                    if (Creature* JainaOrSylvana = instance->GetCreature(JainaOrSylvanaGUID))
                        JainaOrSylvana->AI()->DoAction(ACTION_FRIENDLY_BOSS);
                    if (Creature* VereesOrAethas = instance->GetCreature(VereesaOrAethasGUID))
                        VereesOrAethas->AI()->DoAction(ACTION_FRIENDLY_BOSS);
                    break;
                default:
                    break;
                }
                break;
            }
            case DATA_IRON_JUGGERNAUT:
            {
                switch (state)
                {
                case NOT_STARTED:
                    if (Creature* ij = instance->GetCreature(ironjuggGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, ij);
                    HandleGameObject(winddoorGuid, true);
                    break;
                case DONE:
                    if (Creature* ij = instance->GetCreature(ironjuggGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, ij);
                    HandleGameObject(winddoorGuid, true);
                    HandleGameObject(orgrimmargateGuid, true);
                    break;
                case IN_PROGRESS:
                    if (Creature* ij = instance->GetCreature(ironjuggGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, ij);
                    HandleGameObject(winddoorGuid, false);
                    break;
                }
                break;
            }
            case DATA_KORKRON_D_SHAMAN:
            {
                switch (state)
                {
                case NOT_STARTED:
                    for (uint32 n = NPC_WAVEBINDER_KARDRIS; n <= NPC_EARTHBREAKER_HAROMM; n++)
                        if (Creature* shaman = instance->GetCreature(GetGuidData(n)))
                            SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, shaman);
                    HandleGameObject(orgrimmargate2Guid, true);
                    break;
                case DONE:
                    for (uint32 n = NPC_WAVEBINDER_KARDRIS; n <= NPC_EARTHBREAKER_HAROMM; n++)
                        if (Creature* shaman = instance->GetCreature(GetGuidData(n)))
                            SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, shaman);
                    HandleGameObject(orgrimmargate2Guid, true);
                    HandleGameObject(rustybarsGuid, true);
                    break;
                case IN_PROGRESS:
                    for (uint32 n = NPC_WAVEBINDER_KARDRIS; n <= NPC_EARTHBREAKER_HAROMM; n++)
                        if (Creature* shaman = instance->GetCreature(GetGuidData(n)))
                            SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, shaman);
                    HandleGameObject(orgrimmargate2Guid, false);
                    break;
                }
                break;
            }
            case DATA_GENERAL_NAZGRIM:
            {
                switch (state)
                {
                case NOT_STARTED:
                    if (Creature* nazgrim = instance->GetCreature(gnazgrimGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, nazgrim);
                    HandleGameObject(nazgrimdoorGuid, true);
                    break;
                case IN_PROGRESS:
                    if (Creature* nazgrim = instance->GetCreature(gnazgrimGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, nazgrim);
                    HandleGameObject(nazgrimdoorGuid, false);
                    break;
                case DONE:
                    if (Creature* nazgrim = instance->GetCreature(gnazgrimGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, nazgrim);
                    HandleGameObject(nazgrimdoorGuid, true);
                    HandleGameObject(nazgrimexdoorGuid, true);
                    break;
                }
                break;
            }
            case DATA_MALKOROK:
            {
                switch (state)
                {
                case NOT_STARTED:
                    for (std::vector<ObjectGuid>::const_iterator itr = malkorokfenchGuids.begin(); itr != malkorokfenchGuids.end(); itr++)
                        HandleGameObject(*itr, true);
                    break;
                case IN_PROGRESS:
                    for (std::vector<ObjectGuid>::const_iterator itr = malkorokfenchGuids.begin(); itr != malkorokfenchGuids.end(); itr++)
                        HandleGameObject(*itr, false);
                    break;
                case DONE:
                    for (std::vector<ObjectGuid>::const_iterator itr = malkorokfenchGuids.begin(); itr != malkorokfenchGuids.end(); itr++)
                        HandleGameObject(*itr, true);
                    break;
                }
                break;
            }
            case DATA_SPOILS_OF_PANDARIA:
            {
                switch (state)
                {
                case NOT_STARTED:
                    //Remove Combat
                    for (std::vector<ObjectGuid>::const_iterator itr = npcleverlistGuids.begin(); itr != npcleverlistGuids.end(); itr++)
                        if (Creature* npclever = instance->GetCreature(*itr))
                            npclever->AI()->EnterEvadeMode();
                    //Clear Frames
                    for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            spoil->AI()->DoAction(ACTION_RESET);
                    //Reset Spoils
                    for (std::vector<ObjectGuid>::const_iterator itr = spoils2Guids.begin(); itr != spoils2Guids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            spoil->AI()->DoAction(ACTION_RESET);
                    //Reset All levers
                    for (std::vector<ObjectGuid>::const_iterator itr = leverGuids.begin(); itr != leverGuids.end(); itr++)
                    {
                        if (GameObject* lever = instance->GetGameObject(*itr))
                        {
                            lever->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                            lever->SetGoState(GO_STATE_READY);
                            lever->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
                        }
                    }
                    //Close All Room's Gates
                    for (std::vector<ObjectGuid>::const_iterator itr = roomgateGuids.begin(); itr != roomgateGuids.end(); itr++)
                        HandleGameObject(*itr, false);
                    //Close All Room's Doors
                    for (std::vector<ObjectGuid>::const_iterator itr = irondoorGuids.begin(); itr != irondoorGuids.end(); itr++)
                        HandleGameObject(*itr, false);
                    //Reset All Boxes
                    for (std::vector<ObjectGuid>::const_iterator itr = sopboxGuids.begin(); itr != sopboxGuids.end(); itr++)
                    {
                        if (GameObject* box = instance->GetGameObject(*itr))
                        {
                            box->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                            box->SetGoState(GO_STATE_READY);
                            box->SetLootState(GO_READY);
                            box->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
                        }
                    }
                    HandleGameObject(spentdoorGuid, true);
                    //Remove all buffs
                    DoRemoveAurasDueToSpellOnPlayers(146068);
                    DoRemoveAurasDueToSpellOnPlayers(146099);
                    DoRemoveAurasDueToSpellOnPlayers(146141);
                    break;
                case IN_PROGRESS:
                    if (Creature* ssops = instance->GetCreature(npcssopsGuid))
                        ssops->AI()->DoAction(ACTION_SSOPS_IN_PROGRESS);
                    for (std::vector<ObjectGuid>::const_iterator itr = npcleverlistGuids.begin(); itr != npcleverlistGuids.end(); itr++)
                        if (Creature* npclever = instance->GetCreature(*itr))
                            npclever->AI()->DoZoneInCombat(npclever, 100.0f);
                    HandleGameObject(spentdoorGuid, false);
                    break;
                case DONE:
                    //Clear Frames
                    for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            spoil->AI()->DoAction(ACTION_RESET);
                    //Reset Spoils
                    for (std::vector<ObjectGuid>::const_iterator itr = spoils2Guids.begin(); itr != spoils2Guids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            spoil->AI()->DoAction(ACTION_RESET);
                    //Remove Combat
                    for (std::vector<ObjectGuid>::const_iterator itr = npcleverlistGuids.begin(); itr != npcleverlistGuids.end(); itr++)
                        if (Creature* npclever = instance->GetCreature(*itr))
                            npclever->AI()->EnterEvadeMode();
                    //Open Room's Doors
                    for (std::vector<ObjectGuid>::const_iterator itr = roomdoorGuids.begin(); itr != roomdoorGuids.end(); itr++)
                        HandleGameObject(*itr, true);
                    
                    if (Creature* ssops = instance->GetCreature(npcssopsGuid))
                        ssops->AI()->DoAction(ACTION_SSOPS_DONE);

                    if (GameObject* _ssops = instance->GetGameObject(gossopsGuid))
                        _ssops->Delete();

                    //Open All Gates (for safe)
                    for (std::vector<ObjectGuid>::const_iterator itr = roomgateGuids.begin(); itr != roomgateGuids.end(); itr++)
                        HandleGameObject(*itr, true);

                    //Block interact with all boxes
                    for (std::vector<ObjectGuid>::const_iterator itr = sopboxGuids.begin(); itr != sopboxGuids.end(); itr++)
                        if (GameObject* box = instance->GetGameObject(*itr))
                            box->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);

                    DoRemoveAurasDueToSpellOnPlayers(SPELL_MOGU_RUNE_OF_POWER_AURA);
                    HandleGameObject(spentdoorGuid, true);
                    HandleGameObject(spexdoorGuid, true);
                    //Remove all buffs
                    DoRemoveAurasDueToSpellOnPlayers(146068);
                    DoRemoveAurasDueToSpellOnPlayers(146099);
                    DoRemoveAurasDueToSpellOnPlayers(146141);
                    break;
                case SPECIAL: //first room done, start second
                    if (Creature* ssops = instance->GetCreature(npcssopsGuid))
                        ssops->AI()->DoAction(ACTION_SSOPS_SECOND_ROOM);
                    //Open Next Gates In Room
                    for (std::vector<ObjectGuid>::const_iterator itr = roomgateGuids.begin(); itr != roomgateGuids.end(); itr++)
                        if (GameObject* gate = instance->GetGameObject(*itr))
                            if (gate->GetEntry() == GO_ROOM_GATE || gate->GetEntry() == GO_ROOM_GATE3)
                                gate->SetGoState(GO_STATE_ACTIVE);
                    //Clear Frames
                    for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            spoil->AI()->DoAction(ACTION_RESET);
                    //Reset Spoils
                    for (std::vector<ObjectGuid>::const_iterator itr = spoils2Guids.begin(); itr != spoils2Guids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            spoil->AI()->DoAction(ACTION_RESET);
                    //Send Next Frames
                    for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                        if (Creature* spoil = instance->GetCreature(*itr))
                            if (spoil->GetEntry() == NPC_MOGU_SPOILS || spoil->GetEntry() == NPC_MANTIS_SPOILS)
                                spoil->AI()->DoAction(ACTION_IN_PROGRESS);
                    break;
                }
                break;
            }
            case DATA_KLAXXI:
            {
                switch (state)
                {
                case NOT_STARTED:
                    RemoveDebuffFromPlayers();
                    klaxxidiecount = 0;
                    for (std::vector<ObjectGuid>::const_iterator itr = klaxxiarenagateGuid.begin(); itr != klaxxiarenagateGuid.end(); itr++)
                        HandleGameObject(*itr, true);
                    if (Creature* kc = instance->GetCreature(klaxxicontrollerGuid))
                        kc->AI()->Reset();
                    break;
                case IN_PROGRESS:
                    for (std::vector<ObjectGuid>::const_iterator itr = klaxxiarenagateGuid.begin(); itr != klaxxiarenagateGuid.end(); itr++)
                        HandleGameObject(*itr, false);
                    break;
                case DONE:
                    RemoveDebuffFromPlayers();
                    if (Creature* kc = instance->GetCreature(klaxxicontrollerGuid))
                        kc->AI()->DoAction(ACTION_KLAXXI_DONE);

                    for (std::vector<ObjectGuid>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                    {
                        if (Creature* klaxxi = instance->GetCreature(*itr))
                        {
                            SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, klaxxi);
                            klaxxi->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        }
                    }

                    for (std::vector<ObjectGuid>::const_iterator itr = klaxxiarenagateGuid.begin(); itr != klaxxiarenagateGuid.end(); itr++)
                        HandleGameObject(*itr, true);
                    break;
                }
                break;
            }
            case DATA_THOK:
            {
                switch (state)
                {
                case NOT_STARTED:
                    for (std::vector<ObjectGuid>::const_iterator Itr = prisonerGuids.begin(); Itr != prisonerGuids.end(); Itr++)
                    {
                        if (Creature* p = instance->GetCreature(*Itr))
                        {
                            if (!p->isAlive())
                                p->Respawn();
                            p->AI()->DoAction(ACTION_RESET);
                            p->NearTeleportTo(p->GetHomePosition().GetPositionX(), p->GetHomePosition().GetPositionY(), p->GetHomePosition().GetPositionZ(), p->GetHomePosition().GetOrientation());
                        }
                    }
                    for (std::vector<ObjectGuid>::const_iterator itr = jaillistGuids.begin(); itr != jaillistGuids.end(); itr++)
                    {
                        if (GameObject* jail = instance->GetGameObject(*itr))
                        {
                            jail->SetGoState(GO_STATE_READY);
                            jail->SetLootState(GO_READY);
                            jail->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
                        }
                    }
                    if (Creature* thok = instance->GetCreature(thokGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, thok);
                    HandleGameObject(thokentdoorGuid, true);
                    break;
                case IN_PROGRESS:
                    if (Creature* thok = instance->GetCreature(thokGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, thok);
                    HandleGameObject(thokentdoorGuid, false);
                    break;
                case DONE:
                    if (Creature* thok = instance->GetCreature(thokGuid))
                        SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, thok);
                    HandleGameObject(thokentdoorGuid, true);
                    break;
                }
                break;
            }
            case DATA_BLACKFUSE:
            {
                switch (state)
                {
                case NOT_STARTED:
                    weaponsdone = 0;
                    lastsuperheatweapon = 0;
                    HandleGameObject(blackfuseentdoorGuid, true);
                    break;
                case IN_PROGRESS:
                    crawlerminenum = instance->Is25ManRaid() ? 7 : 3;
                    HandleGameObject(blackfuseentdoorGuid, false);
                    break;
                case DONE:
                    HandleGameObject(blackfuseentdoorGuid, true);
                    break;
                }
            }
            break;
            case DATA_GARROSH:
            {
                switch (state)
                {
                case NOT_STARTED:
                    garroshsoldiersGuids.clear();
                    for (std::vector<ObjectGuid>::const_iterator itr = garroshfenchGuids.begin(); itr != garroshfenchGuids.end(); ++itr)
                        HandleGameObject(*itr, true);
                    HandleGameObject(garroshentdoorGuid, true);
                    SomeActionsAfterGarroshEvade();
                    if (instance->IsHeroic())
                        if (Creature* kgs = instance->GetCreature(korkrongunshipGuid))
                            kgs->AI()->DoAction(ACTION_RESET);
                    break;
                case IN_PROGRESS:
                    rycount = urand(0, 2);
                    for (std::vector<ObjectGuid>::const_iterator itr = garroshfenchGuids.begin(); itr != garroshfenchGuids.end(); ++itr)
                        HandleGameObject(*itr, false);
                    HandleGameObject(garroshentdoorGuid, false);
                    break;
                case DONE:
                    garroshsoldiersGuids.clear();
                    for (std::vector<ObjectGuid>::const_iterator itr = garroshfenchGuids.begin(); itr != garroshfenchGuids.end(); ++itr)
                        HandleGameObject(*itr, true);
                    HandleGameObject(garroshentdoorGuid, true);
                    if (instance->IsHeroic())
                        if (Creature* kgs = instance->GetCreature(korkrongunshipGuid))
                            kgs->AI()->DoAction(ACTION_RESET);
                    break;
                }
            }
            break;
            }

            if (state == DONE)
            {
                DoSummoneEventCreatures();
                if (id == DATA_BLACKFUSE || id == DATA_SPOILS_OF_PANDARIA || id == DATA_THOK)
                    CheckProgressForKlaxxi();
                if (id < DATA_GARROSH && CheckProgressForGarrosh())
                    HandleGameObject(klaxxiexdoorGuid, true);
            }
            else if (state == IN_PROGRESS)
                DoRemoveAurasDueToSpellOnPlayers(SPELL_MOGU_RUNE_OF_POWER_AURA);

            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_FIELD_OF_SHA:
                ++EventfieldOfSha;
                if (EventfieldOfSha >= 3)
                {
                    HandleGameObject(GetGuidData(GO_SHA_ENERGY_WALL), true);
                    SaveToDB();
                }
                break;
            case DATA_SHA_PRE_EVENT:
                for (std::set<ObjectGuid>::iterator itr = shaSlgGUID.begin(); itr != shaSlgGUID.end(); ++itr)
                    if (Creature* slg = instance->GetCreature(*itr))
                        if (data == IN_PROGRESS)
                            slg->AddAura(SPELL_SHA_VORTEX, slg);
                        else
                            slg->RemoveAura(SPELL_SHA_VORTEX);
                break;
            case DATA_SHA_OF_PRIDE:
                if (data == DONE)
                    for (std::vector<ObjectGuid>::iterator itr = PortalOrgrimmarGUID.begin(); itr != PortalOrgrimmarGUID.end(); ++itr)
                        if (Creature* c = instance->GetCreature(*itr))
                            c->SetVisible(true);
                break;
            case DATA_GALAKRAS_PRE_EVENT:
            {
                switch (data)
                {
                case IN_PROGRESS:
                    ShowCannon = data;
                    DoUpdateWorldState(WS_SHOW_KORKRON_CANNON, ShowCannon);
                    break;
                case DONE:
                    ShowCannon = data;
                    DoUpdateWorldState(WS_SHOW_KORKRON_CANNON, 0);
                    DoUpdateWorldState(WS_KORKRON_CANNON_COUNT, 0);
                    if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_PRE_EVENT_FINISH);
                    break;
                }
                break;
            }
            case DATA_GALAKRAS_PRE_EVENT_COUNT:
            {
                CannonCount = data;
                DoUpdateWorldState(WS_KORKRON_CANNON_COUNT, CannonCount);

                if (CannonCount > 7)
                    CannonCount = 7;

                if (CannonCount == 0)
                    SetData(DATA_GALAKRAS_PRE_EVENT, DONE);
                break;
            }
            case DATA_GALAKRAS:
            {
                if (data == DONE)
                {
                    if (TeamInInstance == HORDE)
                        Events.RescheduleEvent(EVENT_FINISH_1_H, 3000);
                    else
                        Events.RescheduleEvent(EVENT_FINISH_1_A, 3000);
                }
                break;
            }
            case DATA_SOUTH_TOWER:
            {
                switch (data)
                {
                case IN_PROGRESS:
                    ShowSouthTower = data;
                    DoUpdateWorldState(WS_SHOW_SOUTH_TOWER, 1);
                    if (instance->IsHeroic())
                        if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                            Galakras->AI()->DoAction(ACTION_GRUNT_SOUTH);
                    break;
                case NOT_STARTED:
                    ShowSouthTower = data;
                    DoUpdateWorldState(WS_SHOW_SOUTH_TOWER, 0);
                    DoUpdateWorldState(WS_SHOW_CAPTURE_SOUTH_TOWER, 0);
                    DoUpdateWorldState(WS_SOUTH_TOWER, SouthTowerCount = 0);
                    break;
                case SPECIAL:
                    ShowSouthTower = data;
                    DoUpdateWorldState(WS_SHOW_SOUTH_TOWER, 0);
                    DoUpdateWorldState(WS_SHOW_CAPTURE_SOUTH_TOWER, 1);
                    if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_GRUNT_SOUTH_FINISH);
                    break;
                }
                break;
            }
            case DATA_NORTH_TOWER:
            {
                switch (data)
                {
                case IN_PROGRESS:
                    ShowNorthTower = data;
                    DoUpdateWorldState(WS_SHOW_NORTH_TOWER, 1);
                    if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_GRUNT_NORTH);
                    break;
                case NOT_STARTED:
                    ShowNorthTower = data;
                    DoUpdateWorldState(WS_SHOW_NORTH_TOWER, 0);
                    DoUpdateWorldState(WS_SHOW_CAPTURE_NORTH_TOWER, 0);
                    DoUpdateWorldState(WS_NORTH_TOWER, NorthTowerCount = 0);
                    break;
                case SPECIAL:
                    ShowNorthTower = data;
                    DoUpdateWorldState(WS_SHOW_NORTH_TOWER, 0);
                    DoUpdateWorldState(WS_SHOW_CAPTURE_NORTH_TOWER, 1);
                    if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_GRUNT_NORTH_FINISH);
                    break;
                }
                break;
            }
            case DATA_SOUTH_COUNT:
            {
                SouthTowerCount = data;
                if (SouthTowerCount < 0)
                    SouthTowerCount = 0;
                DoUpdateWorldState(WS_SOUTH_TOWER, SouthTowerCount);
                DoUpdateWorldState(WS_CAPTURE_SOUTH_TOWER, SouthTowerCount);

                if (SouthTowerCount >= 100 && !STowerFull)
                {
                    STowerFull = true;
                    if (GameObject* SouthDoor = instance->GetGameObject(GetGuidData(GO_SOUTH_DOOR)))
                        SouthDoor->SetGoState(GO_STATE_ACTIVE);
                    if (Creature* Galakras = instance->GetCreature(GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_DEMOLITIONS_NORTH);
                    if (Creature* STower = instance->GetCreature(GetGuidData(NPC_TOWER_SOUTH)))
                        STower->AI()->DoAction(ACTION_TOWER_GUARDS);
                    if (Creature* sDemo = instance->GetCreature(sExpertGUID))
                        sDemo->AI()->DoAction(ACTION_DEMOLITIONS_COMPLETE);
                    SetData(DATA_SOUTH_TOWER, SPECIAL);
                    SetData(DATA_NORTH_TOWER, IN_PROGRESS);
                }
                if (SouthTowerCount == 0 && !STowerNull)
                {
                    STowerNull = true;
                    SetData(DATA_SOUTH_TOWER, NOT_STARTED);
                    if (Creature* STower = instance->GetCreature(GetGuidData(NPC_TOWER_SOUTH)))
                        STower->AI()->DoAction(ACTION_TOWER_TURRET);
                }
                break;
            }
            case DATA_NORTH_COUNT:
            {
                NorthTowerCount = data;
                if (NorthTowerCount < 0)
                    NorthTowerCount = 0;
                DoUpdateWorldState(WS_NORTH_TOWER, NorthTowerCount);
                DoUpdateWorldState(WS_CAPTURE_NORTH_TOWER, NorthTowerCount);

                if (NorthTowerCount >= 100 && !NTowerFull)
                {
                    NTowerFull = true;
                    if (GameObject* NorthDoor = instance->GetGameObject(GetGuidData(GO_NORTH_DOOR)))
                        NorthDoor->SetGoState(GO_STATE_ACTIVE);
                    if (Creature* NTower = instance->GetCreature(GetGuidData(NPC_TOWER_NORTH)))
                        NTower->AI()->DoAction(ACTION_TOWER_GUARDS);
                    if (Creature* nDemo = instance->GetCreature(nExpertGUID))
                        nDemo->AI()->DoAction(ACTION_DEMOLITIONS_COMPLETE);
                    SetData(DATA_NORTH_TOWER, SPECIAL);
                }
                if (NorthTowerCount == 0 && !NTowerNull)
                {
                    NTowerNull = true;
                    SetData(DATA_NORTH_TOWER, NOT_STARTED);
                    if (Creature* NTower = instance->GetCreature(GetGuidData(NPC_TOWER_NORTH)))
                        NTower->AI()->DoAction(ACTION_TOWER_TURRET);
                }
                break;
            }
            case DATA_ACTIVE_NORTH_ROPE:
                if (GameObject* nrs = instance->GetGameObject(northropeskeinGuid))
                    nrs->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* nr = instance->GetGameObject(northropeGuid))
                    nr->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                break;
            case DATA_ACTIVE_SOUTH_ROPE:
                if (GameObject* srs = instance->GetGameObject(southropeskeinGuid))
                    srs->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* sr = instance->GetGameObject(southropeGuid))
                    sr->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                break;
            case DATA_DISABLE_ROPES:
                if (GameObject* nrs = instance->GetGameObject(northropeskeinGuid))
                    nrs->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* nr = instance->GetGameObject(northropeGuid))
                    nr->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* srs = instance->GetGameObject(southropeskeinGuid))
                    srs->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                if (GameObject* sr = instance->GetGameObject(southropeGuid))
                    sr->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                break;
            case DATA_SOP_START:
                //Open Gates In Room
                for (std::vector<ObjectGuid>::const_iterator itr = roomgateGuids.begin(); itr != roomgateGuids.end(); itr++)
                    if (GameObject* gate = instance->GetGameObject(*itr))
                        if (gate->GetEntry() == GO_ROOM_GATE2 || gate->GetEntry() == GO_ROOM_GATE4)
                            gate->SetGoState(GO_STATE_ACTIVE);
                //Send Frames
                for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                    if (Creature* spoil = instance->GetCreature(*itr))
                        if (spoil->GetEntry() == NPC_MOGU_SPOILS2 || spoil->GetEntry() == NPC_MANTIS_SPOILS2)
                            spoil->AI()->DoAction(ACTION_IN_PROGRESS);
                break;
            case DATA_KLAXXI_START:
                for (std::vector<ObjectGuid>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                {
                    if (Creature* klaxxi = instance->GetCreature(*itr))
                    {
                        klaxxi->CastSpell(klaxxi, 146983, true); //Aura Enrage
                        if (klaxxi->HasAura(143542))
                            klaxxi->AI()->DoAction(ACTION_KLAXXI_IN_PROGRESS);
                    }
                }
                break;
            case DATA_BUFF_NEXT_KLAXXI:
                if (klaxxidiecount < 6)
                    if (Creature* klaxxi = instance->GetCreature(GetGuidData(bonusklaxxientry[klaxxidiecount])))
                        klaxxi->CastSpell(klaxxi, 143542, true); //Ready to Fight 
                break;
            case DATA_INTRO_NEXT_KLAXXI:
                if (klaxxidiecount < 6)
                    if (Creature* klaxxi = instance->GetCreature(GetGuidData(bonusklaxxientry[klaxxidiecount])))
                        klaxxi->AI()->DoAction(ACTION_KLAXXI_IN_PROGRESS);
                klaxxidiecount++;
                for (std::vector<ObjectGuid>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                    if (Creature* klaxxi = instance->GetCreature(*itr))
                        if (klaxxi->isAlive() && klaxxi->isInCombat())
                            klaxxi->CastSpell(klaxxi, 143483, true); //Paragons Purpose Heal
                break;
            case DATA_CLEAR_KLAXXI_LIST:
                klaxxilist.clear();
                break;
            case DATA_SAFE_WEAPONS:
                if (!dweaponGuids.empty())
                {
                    uint32 entry = 0;
                    for (std::vector<ObjectGuid>::const_iterator itr = dweaponGuids.begin(); itr != dweaponGuids.end(); itr++)
                    {
                        if (Creature* dw = instance->GetCreature(*itr))
                        {
                            if (dw->isAlive())
                            {
                                dw->AddAura(SPELL_ELECTROMAGNETIC_BARRIER, dw);
                                dw->AddAura(SPELL_ELECTROMAGNETIC_BARRIER_V, dw);
                                switch (dw->GetEntry())
                                {
                                case NPC_DISASSEMBLED_CRAWLER_MINE:
                                    entry = NPC_BLACKFUSE_CRAWLER_MINE;
                                    break;
                                case NPC_DEACTIVATED_LASER_TURRET:
                                    entry = NPC_ACTIVATED_LASER_TURRET;
                                    break;
                                case NPC_DEACTIVATED_ELECTROMAGNET:
                                    entry = NPC_ACTIVATED_ELECTROMAGNET;
                                    break;
                                case NPC_DEACTIVATED_MISSILE_TURRET:
                                    entry = NPC_ACTIVATED_MISSILE_TURRET;
                                    break;
                                }
                                aweaponentry.push_back(entry);
                                entry = 0;
                            }
                        }
                    }

                    if (instance->IsHeroic()) //superheat mechanic
                    {
                        if (Creature* blackfuse = instance->GetCreature(blackfuseGuid))
                        {
                            uint8 numwave = blackfuse->AI()->GetData(DATA_GET_WEAPON_WAVE_INDEX);
                            if (!numwave && lastsuperheatweapon == NPC_BLACKFUSE_CRAWLER_MINE)
                            {
                                bool find = false;
                                for (std::vector<uint32>::const_iterator itr = aweaponentry.begin(); itr != aweaponentry.end(); itr++)
                                {
                                    if (*itr == NPC_ACTIVATED_LASER_TURRET)
                                    {
                                        find = true;
                                        newsuperheatweapon = NPC_ACTIVATED_LASER_TURRET;
                                        break;
                                    }
                                }
                                if (!find)
                                    newsuperheatweapon = NPC_BLACKFUSE_CRAWLER_MINE;
                            }
                            else
                            {
                                uint8 num = 4;
                                for (uint8 n = 0; n < 4; n++)
                                    for (std::vector<uint32>::const_iterator itr = aweaponentry.begin(); itr != aweaponentry.end(); itr++)
                                        if (weaponpriority[n] == (*itr) && (*itr) != lastsuperheatweapon)
                                            if (n < num)
                                                num = n;
                                newsuperheatweapon = weaponpriority[num];
                            }
                        }
                    }

                    if (Creature* blackfuse = instance->GetCreature(blackfuseGuid))
                        blackfuse->CastSpell(blackfuse, SPELL_PROTECTIVE_FRENZY, true);
                }
                break;
            case DATA_D_WEAPON_IN_DEST_POINT:
                weaponsdone++;
                if (Creature* blackfuse = instance->GetCreature(blackfuseGuid))
                {
                    if (weaponsdone == 2 && !aweaponentry.empty())
                    {
                        weaponsdone = 0;
                        bool superheatmine = false;
                        for (uint8 n = 0; n < 2; n++)
                        {
                            if (aweaponentry[n] == NPC_BLACKFUSE_CRAWLER_MINE)
                            {
                                for (uint8 b = crawlerminenum; b > 0; b--)
                                {
                                    if (Creature* aw = blackfuse->SummonCreature(aweaponentry[n], spawnaweaponpos[n].GetPositionX() + float(b + 2), spawnaweaponpos[n].GetPositionY() + float(b + 2), spawnaweaponpos[n].GetPositionZ(), 0.0f))
                                    {
                                        aw->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                        aw->GetMotionMaster()->MoveCharge(destapos[n].GetPositionX() + float(b + 2), destapos[n].GetPositionY() + float(b + 2), destapos[n].GetPositionZ(), 10.0f, 1, false);
                                    }
                                }
                                if (newsuperheatweapon == NPC_BLACKFUSE_CRAWLER_MINE && !superheatmine)
                                {
                                    superheatmine = true;
                                    for (uint8 m = 0; m < 2; m++)
                                    {
                                        if (Creature* aw = blackfuse->SummonCreature(aweaponentry[n], spawnaweaponpos[n].GetPositionX() + float(m + 8), spawnaweaponpos[n].GetPositionY() + float(m + 8), spawnaweaponpos[n].GetPositionZ(), 0.0f))
                                        {
                                            aw->CastSpell(aw, SPELL_SUPERHEATED_CRAWLER_MINE, true);
                                            aw->SetFloatValue(OBJECT_FIELD_SCALE, 2.0f);
                                            aw->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                            aw->GetMotionMaster()->MoveCharge(destapos[m].GetPositionX() + float(m + 8), destapos[m].GetPositionY() + float(m + 8), destapos[m].GetPositionZ(), 10.0f, 1, false);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (Creature* aw = blackfuse->SummonCreature(aweaponentry[n], spawnaweaponpos[n]))
                                {
                                    if (aw->GetEntry() == newsuperheatweapon)
                                        aw->AI()->SetData(DATA_ACTIVE_SUPERHEAT, 0);
                                    aw->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                    aw->GetMotionMaster()->MoveCharge(destapos[n].GetPositionX(), destapos[n].GetPositionY(), destapos[n].GetPositionZ(), 10.0f, 1, false);
                                    if (aw->GetEntry() == NPC_ACTIVATED_LASER_TURRET)
                                        laserturretGuid = aw->GetGUID();
                                }
                            }
                        }
                        lastsuperheatweapon = newsuperheatweapon;
                        aweaponentry.clear();
                        EjectPlayersFromConveyor();
                    }
                    else if (weaponsdone == 3 && aweaponentry.empty())
                    {
                        weaponsdone = 0;
                        bool superheatmine = false;
                        blackfuse->CastSpell(blackfuse, SPELL_ENERGIZED_DEFENSIVE_MATRIX, true);
                        uint8 num = blackfuse->AI()->GetData(DATA_GET_WEAPON_WAVE_INDEX);
                        num = !num ? 5 : --num;

                        if (instance->IsHeroic()) //superheat mechanic
                        {
                            uint8 _num = 4;
                            for (uint8 n = 0; n < 4; n++)
                                for (uint8 b = 1; b < 6; b++)
                                    if (weaponpriority[n] == _wavearray[num][b] && _wavearray[num][b] != lastsuperheatweapon)
                                        if (n < _num)
                                            _num = n;
                            newsuperheatweapon = weaponpriority[_num];
                        }

                        for (uint8 n = 1; n < 4; n++)
                        {
                            if (_wavearray[num][n] == NPC_BLACKFUSE_CRAWLER_MINE)
                            {
                                for (uint8 b = crawlerminenum; b > 0; b--)
                                {
                                    if (Creature* weapon = blackfuse->SummonCreature(_wavearray[num][n], spawnaweaponpos[n - 1].GetPositionX() + float(b + 2), spawnaweaponpos[n - 1].GetPositionY() + float(b + 2), spawnaweaponpos[n - 1].GetPositionZ(), 0.0f))
                                    {
                                        weapon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                        weapon->GetMotionMaster()->MoveCharge(destapos[n - 1].GetPositionX() + float(b + 2), destapos[n - 1].GetPositionY() + float(b + 2), destapos[n - 1].GetPositionZ(), 10.0f, 1, false);
                                    }
                                }
                                if (newsuperheatweapon == NPC_BLACKFUSE_CRAWLER_MINE && !superheatmine)
                                {
                                    superheatmine = true;
                                    for (uint8 m = 0; m < 2; m++)
                                    {
                                        if (Creature* aw = blackfuse->SummonCreature(_wavearray[num][n], spawnaweaponpos[n].GetPositionX() + float(m + 8), spawnaweaponpos[n].GetPositionY() + float(m + 8), spawnaweaponpos[n].GetPositionZ(), 0.0f))
                                        {
                                            aw->CastSpell(aw, SPELL_SUPERHEATED_CRAWLER_MINE, true);
                                            aw->SetFloatValue(OBJECT_FIELD_SCALE, 2.0f);
                                            aw->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                            aw->GetMotionMaster()->MoveCharge(destapos[m].GetPositionX() + float(m + 8), destapos[m].GetPositionY() + float(m + 8), destapos[m].GetPositionZ(), 10.0f, 1, false);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (Creature* weapon = blackfuse->SummonCreature(_wavearray[num][n], spawnaweaponpos[n - 1]))
                                {
                                    if (weapon->GetEntry() == newsuperheatweapon)
                                        weapon->AI()->SetData(DATA_ACTIVE_SUPERHEAT, 0);
                                    weapon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                    weapon->GetMotionMaster()->MoveCharge(destapos[n - 1].GetPositionX(), destapos[n - 1].GetPositionY(), destapos[n - 1].GetPositionZ(), 10.0f, 1, false);
                                    if (weapon->GetEntry() == NPC_ACTIVATED_LASER_TURRET)
                                        laserturretGuid = weapon->GetGUID();
                                }
                            }
                        }
                        lastsuperheatweapon = newsuperheatweapon;
                        EjectPlayersFromConveyor();
                    }
                }
                break;
            case DATA_CRAWLER_MINE_READY:
                crawlerminenum--;
                if (!crawlerminenum)
                {
                    for (uint8 m = 0; m < crawlermineGuids.size(); m++)
                        if (Creature* cm = instance->GetCreature(crawlermineGuids[m]))
                            cm->AI()->SetData(DATA_CRAWLER_MINE_ENTERCOMBAT, uint32(m));
                    crawlermineGuids.clear();
                    crawlerminenum = instance->Is25ManRaid() ? 7 : 3;
                }
                break;
            case DATA_OPEN_SOLDIER_FENCH:
                for (std::vector<ObjectGuid>::const_iterator itr = soldierfenchGuids.begin(); itr != soldierfenchGuids.end(); itr++)
                    DoUseDoorOrButton(*itr);
                break;
            case DATA_UPDATE_GARROSH_REALM:
                rycount = rycount == 2 ? 0 : ++rycount;
                break;
            case DATA_FIRST_ENGENEER_DIED:
                if (data && !engeneerGuids.empty())
                {
                    for (std::vector<ObjectGuid>::const_iterator itr = engeneerGuids.begin(); itr != engeneerGuids.end(); itr++)
                        if (Creature* eng = instance->GetCreature(*itr))
                            if (eng->isAlive())
                                eng->AI()->DoAction(ACTION_FIRST_ENGENEER_DIED);
                }
                engeneerGuids.clear();
                break;
            case DATA_ACTION_SOLDIER:
                if (!garroshsoldiersGuids.empty())
                {
                    switch (data)
                    {
                    case 0:
                        for (std::vector<ObjectGuid>::const_iterator itr = garroshsoldiersGuids.begin(); itr != garroshsoldiersGuids.end(); itr++)
                            if (Creature* soldier = instance->GetCreature(*itr))
                                if (soldier->isAlive())
                                    soldier->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case 1:
                        for (std::vector<ObjectGuid>::const_iterator itr = garroshsoldiersGuids.begin(); itr != garroshsoldiersGuids.end(); itr++)
                            if (Creature* soldier = instance->GetCreature(*itr))
                                if (soldier->isAlive())
                                    soldier->StopAttack();
                        break;
                    }
                }
                break;
            case DATA_RESET_REALM_OF_YSHAARJ:
                ResetRealmOfYshaarj(true);
                break;
            case DATA_PLAY_FINAL_MOVIE:
            {
                uint32 spell = GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? SPELL_HORDE : SPELL_ALLIANCE;
                uint32 _achievemententry = GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? 8680 : 8679; //Liberator of Orgrimmar : Conqueror of Orgrimmar
                Map::PlayerList const& PlayerList = instance->GetPlayers();
                if (!PlayerList.isEmpty())
                {
                    for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                    {
                        if (Player* player = Itr->getSource())
                        {
                            if (AchievementEntry const* achievementEntry = sAchievementStore.LookupEntry(_achievemententry))
                                if (!player->HasAchieved(_achievemententry))
                                    player->CompletedAchievement(achievementEntry);
                            player->CastSpell(player, spell, true);
                        }
                    }
                }
            }
            break;
            case DATA_CHECK_DIED_PLAYER_IN_REALM_OF_YSHARRJ:
                CheckPlayersDiedInRealOfYshaarj();
                break;
            case DATA_CLOSE_ZONE_NORUSHEN:
                for (std::vector<ObjectGuid>::const_iterator guid = lightqGUIDs.begin(); guid != lightqGUIDs.end(); guid++)
                    HandleGameObject(*guid, false);
                break;
            case DATA_CHECK_KDS_RESET_IS_DONE:
                for (uint8 n = 0; n < 2; n++)
                {
                    Creature* kdsmaunt = instance->GetCreature(!n ? bloodclawGuid : darkfangGuid);
                    if (!kdsmaunt)
                        return;
                    if (!kdsmaunt->isAlive() || kdsmaunt->isInCombat())
                        return;
                }

                for (uint8 n = 0; n < 2; n++)
                    if (Creature* kdsmaunt = instance->GetCreature(!n ? bloodclawGuid : darkfangGuid))
                        kdsmaunt->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                break;
            case DATA_PREPARE_REALM_OF_YSHAARJ:
                switch (data)
                {
                case 0:
                    ResetBuffOnEmbodiedDoubts();
                    break;
                case 1:
                    ResetBuffOnEmbodiedFears();
                    break;
                default:
                    break;
                }
                break;
            case DATA_KILL_PLAYERS_IN_MIND_CONTROL:
                KillPlayersInMindControl();
                break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_TEAM_IN_INSTANCE:
                    /* if (!TeamInInstance)
                    {
                        Map::PlayerList const &players = instance->GetPlayers();
                        if (!players.isEmpty())
                            if (Player* player = players.begin()->getSource())
                                TeamInInstance = player->GetTeam();
                    } */
                    return TeamInInstance;
                case DATA_GALAKRAS_PRE_EVENT:
                    return ShowCannon;
                case DATA_GALAKRAS_PRE_EVENT_COUNT:
                    return CannonCount;
                case DATA_SOUTH_TOWER:
                    return ShowSouthTower;
                case DATA_SOUTH_COUNT:
                    return SouthTowerCount;
                case DATA_NORTH_TOWER:
                    return ShowNorthTower;
                case DATA_NORTH_COUNT:
                    return NorthTowerCount;
                case DATA_IS_KLAXXI_DONE:
                    return uint32(const_cast<instance_siege_of_orgrimmar_InstanceMapScript*>(this)->IsKlaxxiDone());
                case DATA_CHECK_INSTANCE_PROGRESS:
                    return uint32(const_cast<instance_siege_of_orgrimmar_InstanceMapScript*>(this)->CheckProgressForGarrosh());
                case DATA_GET_REALM_OF_YSHAARJ:
                    switch (rycount)
                    {
                        case 0:
                            const_cast<instance_siege_of_orgrimmar_InstanceMapScript*>(this)->ResetBuffOnEmbodiedDoubts();
                            break;
                        case 1:
                            const_cast<instance_siege_of_orgrimmar_InstanceMapScript*>(this)->ResetBuffOnEmbodiedFears();
                            break;
                        default:
                            break;
                    }  
                    return rycount;
            }
            return 0;
        }

        bool IsKlaxxiDone()
        {
            for (std::vector<ObjectGuid>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                if (Creature* klaxxi = instance->GetCreature(*itr))
                    if (klaxxi->isAlive())
                        return false;
            return true;
        }

        void KillPlayersInMindControl()
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                    if (Player* player = Itr->getSource())
                        if (player->isAlive() && (player->HasAura(SPELL_TOUCH_OF_YSHAARJ) || player->HasAura(SPELL_EM_TOUCH_OF_YSHAARJ)))
                            player->Kill(player, true);
        }

        void SomeActionsAfterGarroshEvade()
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                {
                    if (Player* player = Itr->getSource())
                    {
                        if (player->isAlive())
                        {       //mind control
                            if (player->HasAura(SPELL_TOUCH_OF_YSHAARJ) || player->HasAura(SPELL_EM_TOUCH_OF_YSHAARJ))
                                player->Kill(player, true);
                        }
                        else
                        {       //player die in yshaarj realm
                            if (player->HasAura(SPELL_REALM_OF_YSHAARJ))
                            {
                                player->NearTeleportTo(Garroshroomcenterpos.GetPositionX(), Garroshroomcenterpos.GetPositionY(), Garroshroomcenterpos.GetPositionZ(), Garroshroomcenterpos.GetOrientation());
                                player->RemoveAurasDueToSpell(SPELL_REALM_OF_YSHAARJ);
                            }  //player die in StormWind (Last phase Heroic)
                            else if (player->GetMap()->GetAreaId(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ()) == 6816) //last phase Garrosh HM area
                                player->NearTeleportTo(Garroshroomcenterpos.GetPositionX(), Garroshroomcenterpos.GetPositionY(), Garroshroomcenterpos.GetPositionZ(), Garroshroomcenterpos.GetOrientation());
                        }
                    }
                }
            }
        }

        void CheckPlayersDiedInRealOfYshaarj()
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                {
                    if (Player* player = Itr->getSource())
                    {
                        if (!player->isAlive())
                        {
                            if (player->HasAura(SPELL_REALM_OF_YSHAARJ))
                            {
                                player->NearTeleportTo(Garroshroomcenterpos.GetPositionX(), Garroshroomcenterpos.GetPositionY(), Garroshroomcenterpos.GetPositionZ(), Garroshroomcenterpos.GetOrientation());
                                player->RemoveAurasDueToSpell(SPELL_REALM_OF_YSHAARJ);
                            }
                        }
                    }
                }
            }
        }

        void ResetBuffOnEmbodiedDoubts()
        {
            for (std::vector<ObjectGuid>::const_iterator itr = edoubtGuids.begin(); itr != edoubtGuids.end(); itr++)
                if (Creature* add = instance->GetCreature(*itr))
                    add->RemoveAurasDueToSpell(SPELL_CONSUMED_FAITH);
            std::random_shuffle(edoubtGuids.begin(), edoubtGuids.end());
            uint8 count = instance->Is25ManRaid() ? 8 : 3;
            for (uint8 n = 0; n < count; n++)
                if (Creature* add = instance->GetCreature(edoubtGuids[n]))
                    add->AddAura(SPELL_CONSUMED_FAITH, add);
        }

        void ResetBuffOnEmbodiedFears()
        {
            for (std::vector<ObjectGuid>::const_iterator itr = efearGuids.begin(); itr != efearGuids.end(); itr++)
                if (Creature* add = instance->GetCreature(*itr))
                    add->RemoveAurasDueToSpell(SPELL_CONSUMED_COURAGE);
            std::random_shuffle(efearGuids.begin(), efearGuids.end());
            uint8 count = instance->Is25ManRaid() ? 25 : 10;
            for (uint8 n = 0; n < count; n++)
                if (Creature* add = instance->GetCreature(efearGuids[n]))
                    add->AddAura(SPELL_CONSUMED_COURAGE, add);
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                //Fallen Protectors
                case NPC_ROOK_STONETOE: 
                    return fpGUID[0];
                case NPC_SUN_TENDERHEART:
                    return fpGUID[1];
                case NPC_HE_SOFTFOOT:
                    return fpGUID[2];
                case NPC_EMBODIED_MISERY_OF_ROOK:
                case NPC_EMBODIED_GLOOM_OF_ROOK:
                case NPC_EMBODIED_SORROW_OF_ROOK:
                    if (!rookmeasureGuids.empty())
                        for (std::vector<ObjectGuid>::const_iterator itr = rookmeasureGuids.begin(); itr != rookmeasureGuids.end(); itr++)
                            if (Creature* measure = instance->GetCreature(*itr))
                                if (measure->GetEntry() == type)
                                    return measure->GetGUID();
                //Galakras
                case DATA_JAINA_OR_SYLVANA:
                    return JainaOrSylvanaGUID;
                case DATA_VEREESA_OR_AETHAS:
                    return VereesaOrAethasGUID;
                case DATA_DEMOLITIONS_EXPERT_S:
                    return sExpertGUID;
                case DATA_DEMOLITIONS_EXPERT_N:
                    return nExpertGUID;
                case NPC_LOREWALKER_CHO:
                case NPC_LOREWALKER_CHO3:
                    return LorewalkerChoGUIDtmp;
                //Korkron Dark Shaman
                case NPC_WAVEBINDER_KARDRIS:
                    return kardrisGuid;
                case NPC_EARTHBREAKER_HAROMM:
                    return harommGuid;
                case NPC_BLOODCLAW:
                    return bloodclawGuid;
                case NPC_DARKFANG:
                    return darkfangGuid;
                //Malkorok
                case NPC_ANCIENT_MIASMA:
                    return amGuid;
                //Spoils of Pandaria
                case GO_LEVER_R:
                case GO_LEVER_L:
                    for (std::vector<ObjectGuid>::const_iterator itr = leverGuids.begin(); itr != leverGuids.end(); itr++)
                        if (GameObject* lever = instance->GetGameObject(*itr))
                            if (lever->GetEntry() == type)
                                return lever->GetGUID();
                case GO_IRON_DOOR_R:
                case GO_IRON_DOOR_L:
                    for (std::vector<ObjectGuid>::const_iterator itr = irondoorGuids.begin(); itr != irondoorGuids.end(); itr++)
                        if (GameObject* door = instance->GetGameObject(*itr))
                            if (door->GetEntry() == type)
                                return door->GetGUID();
                case DATA_SPOIL_MANTIS: 
                    for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                    {
                        if (Creature* spoil = instance->GetCreature(*itr))
                        {
                            if (GetBossState(DATA_SPOILS_OF_PANDARIA) == IN_PROGRESS)
                            {
                                if (spoil->GetEntry() == NPC_MANTIS_SPOILS2)
                                    return spoil->GetGUID();
                            }
                            else if (GetBossState(DATA_SPOILS_OF_PANDARIA) == SPECIAL)
                                if (spoil->GetEntry() == NPC_MANTIS_SPOILS)
                                    return spoil->GetGUID();
                        }
                    }
                case DATA_SPOIL_MOGU: 
                    for (std::vector<ObjectGuid>::const_iterator itr = spoilsGuids.begin(); itr != spoilsGuids.end(); itr++)
                    {
                        if (Creature* spoil = instance->GetCreature(*itr))
                        {
                            if (GetBossState(DATA_SPOILS_OF_PANDARIA) == IN_PROGRESS)
                            {
                                if (spoil->GetEntry() == NPC_MOGU_SPOILS2)
                                    return spoil->GetGUID();
                            }
                            else if (GetBossState(DATA_SPOILS_OF_PANDARIA) == SPECIAL)
                                if (spoil->GetEntry() == NPC_MOGU_SPOILS)
                                    return spoil->GetGUID();
                        }
                    }
                case NPC_MOGU_SPOILS:    
                case NPC_MOGU_SPOILS2:   
                case NPC_MANTIS_SPOILS:
                case NPC_MANTIS_SPOILS2: 
                    for (std::vector<ObjectGuid>::const_iterator itr = spoils2Guids.begin(); itr != spoils2Guids.end(); itr++)
                        if (Creature* spoil2 = instance->GetCreature(*itr))
                            if (spoil2->GetEntry() == type)
                                return spoil2->GetGUID();
                //Mogu
                case GO_SMALL_MOGU_BOX:
                case GO_MEDIUM_MOGU_BOX:
                case GO_BIG_MOGU_BOX:
                    for (std::vector<ObjectGuid>::const_iterator itr = spoils2Guids.begin(); itr != spoils2Guids.end(); itr++)
                    {
                        if (Creature* spoil = instance->GetCreature(*itr))
                        {
                            if (GetBossState(DATA_SPOILS_OF_PANDARIA) == IN_PROGRESS)
                            {
                                if (spoil->GetEntry() == NPC_MOGU_SPOILS2)
                                    return spoil->GetGUID();
                            }
                            else if (GetBossState(DATA_SPOILS_OF_PANDARIA) == SPECIAL)
                                if (spoil->GetEntry() == NPC_MOGU_SPOILS)
                                    return spoil->GetGUID();
                        }
                    }
                //Mantis
                case GO_SMALL_MANTIS_BOX:
                case GO_MEDIUM_MANTIS_BOX:
                case GO_BIG_MANTIS_BOX:
                    for (std::vector<ObjectGuid>::const_iterator itr = spoils2Guids.begin(); itr != spoils2Guids.end(); itr++)
                    {
                        if (Creature* spoil = instance->GetCreature(*itr))
                        {
                            if (GetBossState(DATA_SPOILS_OF_PANDARIA) == IN_PROGRESS)
                            {
                                if (spoil->GetEntry() == NPC_MANTIS_SPOILS2)
                                    return spoil->GetGUID();
                            }
                            else if (GetBossState(DATA_SPOILS_OF_PANDARIA) == SPECIAL)
                                if (spoil->GetEntry() == NPC_MANTIS_SPOILS)
                                    return spoil->GetGUID();
                        }
                    }
                case NPC_SSOP_SPOILS:
                case GO_PANDAREN_RELIC_BOX:
                    return npcssopsGuid;
                //Paragons of the Klaxxi
                case NPC_KILRUK:
                case NPC_XARIL:
                case NPC_KAZTIK:
                case NPC_KORVEN:
                case NPC_IYYOKYK:
                case NPC_KAROZ:
                case NPC_SKEER:
                case NPC_RIKKAL:
                case NPC_HISEK:
                    for (std::vector<ObjectGuid>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                        if (Creature* klaxxi = instance->GetCreature(*itr))
                            if (klaxxi->GetEntry() == type)
                                return klaxxi->GetGUID();
                case NPC_AMBER_PIECE:
                    return amberpieceGuid;
                case NPC_KLAXXI_CONTROLLER:
                    return klaxxicontrollerGuid;
                //Thok
                case NPC_THOK:
                    return thokGuid;
                case NPC_BODY_STALKER:
                    return bsGuid;
                //Blackfuse
                case NPC_BLACKFUSE_MAUNT:
                    return blackfuseGuid;
                case NPC_SHOCKWAVE_MISSILE_STALKER:
                    return swmstalkerGuid;
                case NPC_ACTIVATED_ELECTROMAGNET:
                    return electromagnetGuid;
                case NPC_ACTIVATED_LASER_TURRET:
                    return laserturretGuid;
                case NPC_HEART_OF_YSHAARJ:
                    return heartofyshaarjGuid;
                case DATA_GARROSH:
                    return garroshGuid;
                case DATA_GARROSH_REALM:
                    return garroshrealmGuid;
                case NPC_HORDE_CANNON:
                    if (!hordecannonlistGuids.empty())
                    {
                        std::vector<ObjectGuid>::const_iterator itr = hordecannonlistGuids.begin();
                        std::advance(itr, urand(0, hordecannonlistGuids.size() - 1));
                        return *itr;
                    }
                case NPC_KORKRON_GUNSHIP:
                    return korkrongunshipGuid;
                case DATA_GARROSH_STORMWIND:
                    return garroshstormwindGuid;
            }
            std::map<uint32, ObjectGuid>::const_iterator itr = easyGUIDconteiner.find(type);
            if (itr != easyGUIDconteiner.end())
                return itr->second;

            return ObjectGuid::Empty;
        }

        void CreatureDies(Creature* creature, Unit* /*killer*/)
        {
            switch (creature->GetEntry())
            {
            case NPC_EMBODIED_MISERY_OF_ROOK:
            case NPC_EMBODIED_GLOOM_OF_ROOK:
            case NPC_EMBODIED_SORROW_OF_ROOK:
                if (!rookmeasureGuids.empty())
                {
                    for (std::vector<ObjectGuid>::const_iterator itr = rookmeasureGuids.begin(); itr != rookmeasureGuids.end(); itr++)
                        if (Creature* measure = instance->GetCreature(*itr))
                            if (measure->isAlive())
                                return;

                    if (Creature* rook = instance->GetCreature(fpGUID[0]))
                        if (rook->isAlive() && rook->isInCombat())
                            rook->AI()->DoAction(ACTION_END_DESPERATE_MEASURES);
                }
                break;
            case NPC_EMBODIED_DESPERATION_OF_SUN:
            case NPC_EMBODIED_DESPIRE_OF_SUN:
                if (!sunmeasureGuids.empty())
                {
                    for (std::vector<ObjectGuid>::const_iterator itr = sunmeasureGuids.begin(); itr != sunmeasureGuids.end(); itr++)
                        if (Creature* measure = instance->GetCreature(*itr))
                            if (measure->isAlive())
                                return;

                    if (Creature* sun = instance->GetCreature(fpGUID[1]))
                        if (sun->isAlive() && sun->isInCombat())
                            sun->AI()->DoAction(ACTION_END_DESPERATE_MEASURES);
                }
                break;
            case NPC_EMBODIED_ANGUISH_OF_HE:
                if (creature->GetGUID() == hemeasureGuid)
                    if (Creature* he = instance->GetCreature(fpGUID[2]))
                        if (he->isAlive() && he->isInCombat())
                            he->AI()->DoAction(ACTION_END_DESPERATE_MEASURES);
                break;
            case NPC_ZEAL:
            case NPC_ARROGANCE:
            case NPC_VANITY:
                SetData(DATA_FIELD_OF_SHA, true);
                break;
            case NPC_LINGERING_CORRUPTION:
                --lingering_corruption_count;
                if (!lingering_corruption_count)
                {
                    if (Creature* Norushen = instance->GetCreature(GetGuidData(NPC_SHA_NORUSHEN)))
                        Norushen->AI()->SetData(NPC_LINGERING_CORRUPTION, DONE);
                }
                break;
            case NPC_EMBODIED_DESPAIR:
                for (std::vector<ObjectGuid>::const_iterator itr = edespairGuids.begin(); itr != edespairGuids.end(); itr++)
                    if (Creature* add = instance->GetCreature(*itr))
                        if (add->isAlive())
                            return;
                RemoveProtectFromGarrosh();
                break;
            case NPC_EMBODIED_DOUBT:
                for (std::vector<ObjectGuid>::const_iterator itr = edoubtGuids.begin(); itr != edoubtGuids.end(); itr++)
                    if (Creature* add = instance->GetCreature(*itr))
                        if (add->isAlive())
                            return;
                RemoveProtectFromGarrosh();
                break;
            case NPC_EMBODIED_FEAR:
                for (std::vector<ObjectGuid>::const_iterator itr = efearGuids.begin(); itr != efearGuids.end(); itr++)
                    if (Creature* add = instance->GetCreature(*itr))
                        if (add->isAlive())
                            return;
                RemoveProtectFromGarrosh();
                break;
            case NPC_HUNGRY_KUNCHONG:
                if (Creature* kaztik = instance->GetCreature(GetGuidData(NPC_KAZTIK)))
                {
                    if (kaztik->isAlive() && kaztik->isInCombat())
                    {
                        if (Creature* ap = kaztik->FindNearestCreature(NPC_AMBER_PIECE, 150.0f, true))
                        {
                            float x, y;
                            GetPosInRadiusWithRandomOrientation(ap, 50.0f, x, y);
                            kaztik->SummonCreature(NPC_HUNGRY_KUNCHONG, x, y, ap->GetPositionZ());
                        }
                    }
                }
                break;
            case NPC_MOGU_SHADOW_RITUALIST:
                RemoveTormentFromPlayers(creature->GetGUID());
                break;
            }
        }

        void RemoveTormentFromPlayers(ObjectGuid CasterGuid)
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                    if (Player* player = Itr->getSource())
                        if (player->isAlive() && player->HasAura(SPELL_TORMENT_MAIN))
                            if (Aura* tormentaura = player->GetAura(SPELL_TORMENT_MAIN))
                                if (CasterGuid == tormentaura->GetCasterGUID())
                                    player->RemoveAurasDueToSpell(SPELL_TORMENT_MAIN);
            }
        }

        void RemoveProtectFromGarrosh()
        {
            for (std::vector<ObjectGuid>::const_iterator itr = shavortexGuids.begin(); itr != shavortexGuids.end(); itr++)
                if (Creature* sv = instance->GetCreature(*itr))
                    sv->RemoveAurasDueToSpell(SPELL_YSHAARJ_PROTECTION_AT);

            for (std::vector<ObjectGuid>::const_iterator Itr = goshavortexGuids.begin(); Itr != goshavortexGuids.end(); Itr++)
                HandleGameObject(*Itr, true);

            if (Creature* garroshrealm = instance->GetCreature(garroshrealmGuid))
            {
                garroshrealm->RemoveAurasDueToSpell(SPELL_CRUSHING_FEAR);
                garroshrealm->RemoveAurasDueToSpell(SPELL_YSHAARJ_PROTECTION);
                garroshrealm->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                garroshrealm->AI()->DoAction(ACTION_LAUNCH_ANNIHILLATE);
            }
        }

        void ResetRealmOfYshaarj(bool full)
        {
            for (std::vector<ObjectGuid>::const_iterator itr = shavortexGuids.begin(); itr != shavortexGuids.end(); itr++)
                if (Creature* sv = instance->GetCreature(*itr))
                    if (!sv->HasAura(SPELL_YSHAARJ_PROTECTION_AT))
                        sv->CastSpell(sv, SPELL_YSHAARJ_PROTECTION_AT);

            for (std::vector<ObjectGuid>::const_iterator Itr = goshavortexGuids.begin(); Itr != goshavortexGuids.end(); Itr++)
                HandleGameObject(*Itr, false);

            if (full)
            {
                for (std::vector<ObjectGuid>::const_iterator itr = edespairGuids.begin(); itr != edespairGuids.end(); itr++)
                {
                    if (Creature* add = instance->GetCreature(*itr))
                    {
                        if (!add->isAlive())
                        {
                            add->Respawn();
                            add->GetMotionMaster()->MoveTargetedHome();
                        }
                        else
                            add->SetFullHealth();
                    }
                }

                for (std::vector<ObjectGuid>::const_iterator itr = edoubtGuids.begin(); itr != edoubtGuids.end(); itr++)
                {
                    if (Creature* add = instance->GetCreature(*itr))
                    {
                        if (!add->isAlive())
                        {
                            add->Respawn();
                            add->GetMotionMaster()->MoveTargetedHome();
                        }
                        else
                            add->SetFullHealth();
                    }
                }

                for (std::vector<ObjectGuid>::const_iterator itr = efearGuids.begin(); itr != efearGuids.end(); itr++)
                {
                    if (Creature* add = instance->GetCreature(*itr))
                    {
                        if (!add->isAlive())
                        {
                            add->Respawn();
                            add->GetMotionMaster()->MoveTargetedHome();
                        }
                        else
                            add->SetFullHealth();
                    }
                }
            }
        }

        void EjectPlayersFromConveyor()
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                {
                    if (Player* player = Itr->getSource())
                    {
                        if (player->isAlive() && player->HasAura(SPELL_ON_CONVEYOR))
                        {
                            player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY); //for safe(block all ability)
                            player->RemoveAurasDueToSpell(SPELL_ON_CONVEYOR);
                            player->GetMotionMaster()->MoveJump(1983.22f, -5559.18f, -309.3264f, 30.0f, 30.0f, 145351);
                        }
                    }
                }
            }
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

        std::string GetSaveData()
        {
            std::ostringstream saveStream;
            saveStream << "S O " << GetBossSaveData() << " " << EventfieldOfSha;
            return saveStream.str();
        }

        void Load(const char* data)
        {
            if (!data)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(data);

            char dataHead1, dataHead2;

            std::istringstream loadStream(data);
            loadStream >> dataHead1 >> dataHead2;

            if (dataHead1 == 'S' && dataHead2 == 'O')
            {
                for (uint32 i = 0; i < DATA_MAX; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }
                loadStream >> EventfieldOfSha;
            }
            else
                OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
        
        bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = NULL) const
        {
            if (player && AccountMgr::IsGMAccount(player->GetSession()->GetSecurity()))
                return true;

            switch (bossId)
            {
                case DATA_IMMERSEUS:
                    return true;
                case DATA_F_PROTECTORS:
                    return GetBossState(DATA_IMMERSEUS) == DONE;
                case DATA_NORUSHEN:
                    return GetBossState(DATA_F_PROTECTORS) == DONE;
            }
            return true;
        }

        bool IsRaidBoss(uint32 creature_entry)
        {
            switch (creature_entry)
            {
            case NPC_IMMERSEUS:
            case NPC_ROOK_STONETOE:
            case NPC_SUN_TENDERHEART:
            case NPC_HE_SOFTFOOT:
            case NPC_SHA_OF_PRIDE:
            case NPC_AMALGAM_OF_CORRUPTION:
            case NPC_GALAKRAS:
            case NPC_IRON_JUGGERNAUT:
            case NPC_WAVEBINDER_KARDRIS:
            case NPC_EARTHBREAKER_HAROMM:
            case NPC_GENERAL_NAZGRIM:
            case NPC_MALKOROK:
            case NPC_THOK:
            case NPC_BLACKFUSE_MAUNT:
            case NPC_KILRUK:
            case NPC_XARIL:
            case NPC_KAZTIK:
            case NPC_KORVEN:
            case NPC_IYYOKYK:
            case NPC_KAROZ:
            case NPC_SKEER:
            case NPC_RIKKAL:
            case NPC_HISEK:
            case NPC_GARROSH:
            case NPC_KORKRON_IRON_STAR_HM: //safe for some exploits
                return true;
            }
            return false;
        }

        void Update(uint32 diff)
        {
            Events.Update(diff);

            if (uint32 eventId = Events.ExecuteEvent())
            {
                switch (eventId)
                {
                // Galakras finish event. Horde
                case EVENT_FINISH_1_H:
                    if (Creature* Lorthemar = instance->GetCreature(GetGuidData(NPC_LORTHEMAR_THERON_H)))
                        Lorthemar->AI()->Talk(7);
                    Events.RescheduleEvent(EVENT_FINISH_2_H, 2000);
                    break;
                case EVENT_FINISH_2_H:
                    if (Creature* Sylvana = instance->GetCreature(GetGuidData(NPC_LADY_SYLVANAS_WINDRUNNER_H)))
                        Sylvana->AI()->Talk(5);
                    Events.RescheduleEvent(EVENT_FINISH_3_H, 4000);
                    break;
                case EVENT_FINISH_3_H:
                    if (Creature* Lorthemar = instance->GetCreature(GetGuidData(NPC_LORTHEMAR_THERON_H)))
                        Lorthemar->AI()->Talk(8);
                    break;
                    // Galakras finish event. Alliance
                case EVENT_FINISH_1_A:
                    if (Creature* Jaina = instance->GetCreature(GetGuidData(NPC_LADY_JAINA_PROUDMOORE_A)))
                        Jaina->AI()->Talk(5);
                    Events.RescheduleEvent(EVENT_FINISH_2_A, 2000);
                    break;
                case EVENT_FINISH_2_A:
                    if (Creature* Varian = instance->GetCreature(GetGuidData(NPC_KING_VARIAN_WRYNN_A)))
                        Varian->AI()->Talk(7);
                    Events.RescheduleEvent(EVENT_FINISH_3_A, 4000);
                    break;
                case EVENT_FINISH_3_A:
                    if (Creature* Jaina = instance->GetCreature(GetGuidData(NPC_LADY_JAINA_PROUDMOORE_A)))
                        Jaina->AI()->Talk(6);
                    break;
                }
            }
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_siege_of_orgrimmar_InstanceMapScript(map);
    }
};

void AddSC_instance_siege_of_orgrimmar()
{
    new instance_siege_of_orgrimmar();
}
