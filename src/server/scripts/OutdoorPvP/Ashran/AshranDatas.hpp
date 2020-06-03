////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

enum eAshranDatas
{
    /// Maps, Zones, Areas IDs
    AshranZoneID                = 6941,
    AshranMapID                 = 1191,
    AshranNeutralMapID          = 1116,
    AshranPreAreaHorde          = 7333,
    AshranPreAreaAlliance       = 7332,
    AshranHordeBase             = 7099,
    AshranAllianceBase          = 7100,
    AshranPvPAreaID             = 24,
    EmberfallTowerAreaID        = 7080,
    VolrathsAdvanceAreaID       = 7476,
    ArchmageOverwatchAreaID     = 7479,
    TrembladesVanguardAreaID    = 7478,
    KingsRestAreaID             = 7439,

    /// Timers
    AshranTimeForInvite         = 60,
    AshranTimeForBattle         = 25,
    AshranEventTimer            = 30,   ///< In minutes
    AshranEventWarning          = 3,    ///< In minutes
    AncientArtifactMaxTime      = 30 * MINUTE * IN_MILLISECONDS,

    /// Generic data
    BattlefieldWorldPvP         = 0x20000,
    AshranHallowedGroundH       = 7782,
    AshranHallowedGroundA       = 7781,
    AshranHallowedGroundID      = 42989,
    AncientArtifactCount        = 9,
    AllianceRacingMovesCount    = 39,
    HordeRacingMovesCount       = 36,
    MaxStadiumRacingLaps        = 3,

    /// Misc
    PlayerMinLevel              = 100,
    AshranGenericMobTypeID      = 68553,
    TaxiPathBaseHordeToAlliance = 4665,
    TaxiPathBaseAllianceToHorde = 4666,
    KillCountForPlayer          = 5,
    KillCountForFactionGuard    = 1,
    HealthPCTAddedByHostileRef  = 25,

    /// Ennemies Slain
    EnnemiesSlainCap1           = 50,
    EnnemiesSlainCap2           = 100,
    EnnemiesSlainCap3           = 200,
    EnnemiesSlainCap4           = 400,
    EnnemiesSlainCap5           = 800,
    EnnemiesSlainCap6           = 1000
};

enum eAshranSpells
{
    /// Zone buffs
    SpellLootable                   = 161733,
    SpellHoldYourGround             = 173534,   ///< +30% damage, healing and health
    SpellTowerDefense               = 173541,   ///< +20% damage, healing and health
    SpellStandFast                  = 173549,   ///< +10% damage, healing and health
    SpellHallowedGround             = 171496,

    /// Rewarding spells
    SpellAllianceReward             = 178531,   ///< Trigger horde strongbox (120151)
    SpellHordeReward                = 178533,   ///< Trigger alliance strongbox (118065)
    SpellEventHordeReward           = 175094,   ///< Dented Ashmaul Strongbox (118093)
    SpellEventAllianceReward        = 175093,   ///< Dented Ashmaul Strongbox (118094)

    /// Misc
    SpellSpiritHeal                 = 22011,
    SpellAncientArtifact            = 168506,   ///< Buff granted when interact with Ancient Artifact object

    /// Quest spells
    WelcomeToAshranAlliance         = 169144,   ///< Gives quest 36119
    WelcomeToAshranHorde            = 169146,   ///< Gives quest 36196

    /// Events
    SpellEventCollectEmpoweredOre   = 178019,
    SpellEventRisenSpirits          = 178020,
    SpellEventOgreFires             = 178021,
    SpellEventStadiumRacing         = 178022,   ///< Seems to proc every ~35m38s
    SpellEventKorlokTheOgreKing     = 178380
};

enum eWorldStates
{
    WorldStateDisabled                      = 0,
    WorldStateEnabled                       = 1,
    //////////////////////////////////////////////////
    /// Molten Quarry(southeast) - Event: Collect Empowered Ore
    WorldStateOreCollectedAlliance          = 1581,
    WorldStateOreCollectedHorde             = 1582,
    WorldStateEnableOreCollection           = 9274,
    /// Molten Quarry(southeast) - Event: Collect Empowered Ore
    //////////////////////////////////////////////////
    WorldStateEnnemiesSlainAlliance         = 8933,
    WorldStateEnnemiesSlainHorde            = 8934,
    WorldStateNextBattleTimestamp           = 8945,
    WorldStateNextBattleEnabled             = 8946,
    WorldStateActiveStage                   = 8955,
    //////////////////////////////////////////////////
    /// Risen Spirits (Ashmaul Burial Grounds)
    WorldStateRisenSpiritsCaptureEnabled    = 9199,
    WorldStateRisenSpiritsCapturedAlliance  = 9200,
    WorldStateRisenSpiritsCapturedHorde     = 9201,
    /// Risen Spirits (Ashmaul Burial Grounds)
    //////////////////////////////////////////////////
    /// Amphitheater of Annihilation (northwest) - Event: The Race!
    WorldStateEnableLapsEvent               = 9290,
    WorldStateLapsAlliance                  = 9291,
    WorldStateLapsHorde                     = 9292,
    /// Amphitheater of Annihilation (northwest) - Event: The Race!
    //////////////////////////////////////////////////
    WorldStateTimeRemainingForBoss          = 9316,
    WorldStateSlayVolrath                   = 9317, ///< Enable time remaining for boss
    WorldStateSlayTremblade                 = 9326, ///< Enable time remaining for boss
    //////////////////////////////////////////////////
    /// Brute's Rise (northeast) - Event: Ogre Fires!
    WorldStateFiresScoringEnabled           = 9414,
    WorldStateFiresScoringAlliance          = 9418,
    WorldStateFiresScoringHorde             = 9419,
    WorldStateFiresControlledAlliance       = 9782,
    WorldStateFiresControlledHorde          = 9783,
    WorldStateFiresScoringMax               = 9784,
    /// Brute's Rise (northeast) - Event: Ogre Fires!
    //////////////////////////////////////////////////
    WorldStateHighWarlordVolrath            = 9713,
    WorldStateEmberfallTowerBattle          = 9714,
    WorldStateVolrathsAdvanceBattle         = 9715,
    WorldStateTheCrossroadsBattle           = 9716,
    WorldStateTrembladesVanguardBattle      = 9717,
    WorldStateArchmageOverwatchBattle       = 9718,
    WorldStateGrandMarshalTrembladeBattle   = 9719,
    WorldStateControlTheFlag                = 9785,
    WorldStateEnnemiesSlainAllianceMax      = 9801,
    WorldStateEnnemiesSlainHordeMax         = 9802,
    //////////////////////////////////////////////////
    /// Kings' Rest (west)
    WorldStateEnableGraveyardProgressBar    = 9053,
    WorldStateGraveyardProgressBar          = 9054,
    WorldStateGraveyardProgressBarGreyPct   = 9055,
    WorldStateGraveyardStatusForAlliance    = 9214,
    WorldStateGraveyardStatusForHorde       = 9215,
    /// Kings' Rest (west)
    //////////////////////////////////////////////////
    /// Five Towers line the Road of Glory
    WorldStateEnableTowerProgressBar        = 9110,
    WorldStateTowerProgressBar              = 9111,
    WorldStateTowerProgressBarGreyPct       = 9112,
    /// Five Towers line the Road of Glory
    //////////////////////////////////////////////////
    /// Five Towers line - Control Status
    WorldStateWarspearOutpostStatus         = 9100, ///< 0 - In fight, 1 - Horde, 2 - Destroyed
    WorldStateEmberfallTowerStatus          = 9101, ///< 0 - Neutral, 1 - Horde, 2 - Alliance
    WorldStateVolrathsAdvanceStatus         = 9102, ///< 0 - Neutral, 1 - Horde, 2 - Alliance
    WorldStateTheCrossroadsStatus           = 9103, ///< 0 - Neutral, 1 - Horde, 2 - Alliance
    WorldStateTrembladesVanguardStatus      = 9104, ///< 0 - Neutral, 1 - Horde, 2 - Alliance
    WorldStateArchmageOverwatchStatus       = 9105, ///< 0 - Neutral, 1 - Horde, 2 - Alliance
    WorldStateStormshieldStrongholdStatus   = 9106, ///< 0 - In fight, 1 - Destroyed, 2 - Alliance
    /// Five Towers line - Control Status
    //////////////////////////////////////////////////
    /// Not implemented yet
    WorldStateOgreKingThroneStatus          = 9113, ///< 0 - Neutral, 1 - Horde
    /// Not implemented yet
    //////////////////////////////////////////////////
    /// Artifact Fragments
    ///     - Current counter for Horde
    WorldStateHordeMageArtifactCount        = 9313,
    WorldStateHordeWarlockArtifactCount     = 9250,
    WorldStateHordeWarriorArtifactCount     = 9311,
    WorldStateHordeShamanArtifactCount      = 9261,
    ///     - Current counter for Alliance
    WorldStateAllianceMageArtifactCount     = 9314,
    WorldStateAllianceWarlockArtifactCount  = 9253,
    WorldStateAllianceWarriorArtifactCount  = 9312,
    WorldStateAllianceShamanArtifactCount   = 9262,
    ///     - Max counter
    WorldStateMageArtifactMaxCount          = 9705, ///< 400
    WorldStateWarlockArtifactMaxCount       = 9706, ///< 400
    WorldStateWarriorArtifactMaxCount       = 9707, ///< 600
    WorldStateShamanArtifactMaxCount        = 9708  ///< 3000
    /// Artifact Fragments
    //////////////////////////////////////////////////
};

enum eAshranEvents
{
    EventKorlokTheOgreKing,
    EventCollectEmpoweredOre,
    EventRisenSpirits,
    EventOgreFires,
    EventStadiumRacing,
    MaxEvents
};

enum eControlStatus
{
    ControlNeutral,
    ControlHorde,
    ControlAlliance
};

enum eCreatures
{
    /// Neutrals
    AshranHerald            = 84113,
    BladeTwisterTrigger     = 89320,
    SLGGenericMoPLargeAoI   = 68553,
    KorlokTheOgreKing       = 80858,

    /// Alliance
    StormshieldVanguard     = 83717,
    StormshieldKnight       = 80256,
    StormshieldSentinel     = 79990,
    StormshieldFootman      = 79268,
    StormshieldPriest       = 79947,
    StormshieldGladiator    = 85812,
    GrandMarshalTremblade   = 82876,    ///< Alliance boss
    StormshieldGryphon      = 87689,    ///< Alliance taxi
    TinaKelatara            = 87617,    ///< Alliance <Flight Master>
    RylaiCrestfall          = 88224,    ///< Alliance Guardian
    AllianceSpiritGuide     = 80723,
    GaulDunFirok            = 81726,    ///< Gaul Dun Firok <Alliance Champion>
    MarshalKarshStormforge  = 82880,
    MarshalGabriel          = 82878,
    LifelessAncient         = 81883,
    AllianceRider           = 82870,
    ExAllianceRacer         = 82884,

    /// Horde
    WarspearBloodGuard      = 83699,
    WarspearRaptorRider     = 80297,
    WarspearHeadhunter      = 79993,
    WarspearGrunt           = 79269,
    WarspearPriest          = 79982,
    WarspearGladiator       = 85811,
    HighWarlordVolrath      = 82877,    ///< Horde boss
    JeronEmberfall          = 88178,    ///< Horde Guardian
    WarspearWyvern          = 87687,    ///< Horde taxi
    ShevanManille           = 87672,    ///< Horde <Flight Master>
    HordeSpiritGuide        = 80724,
    MukmarRaz               = 81725,    ///< Muk'Mar Raz <Horde Champion>
    GeneralAevd             = 82882,
    WarlordNoktyn           = 82883,
    UnderpoweredEarthFury   = 82200,
    HordeRider              = 82864,
    SpeedyHordeRacer        = 82903,

    /// Artifact Fragments NPCs
    /// Horde
    Nisstyr                 = 83997,    ///< Horde warlock leader
    Fura                    = 83995,    ///< Horde mage leader
    Kalgan                  = 83830,    ///< Horde warrior leader
    Atomik                  = 82204,    ///< Horde shaman leader
    /// Alliance
    Marketa                 = 82660,    ///< Alliance warlock leader
    Ecilam                  = 82966,    ///< Alliance mage leader
    ValantBrightsworn       = 82893,    ///< Alliance paladin leader
    Anenga                  = 81870,    ///< Alliance druid leader

    /// Artifacts events related
    /// Alliance
    PortalMageA             = 83435,
    VignetteDummyA          = 84471,
    Kauper                  = 84466,
    FalconAtherton          = 84652,
    DeckerWatts             = 84651,
    Frangraal               = 81859,
    /// Horde
    PortalMageH             = 83948,
    VignetteDummyH          = 84683,
    ZaramSunraiser          = 84468,
    GaylePlagueheart        = 84645,
    IlyaPlagueheart         = 84646,
    Kronus                  = 82201
};

enum eAshranCaptains
{
    /// Alliance
    AvengerTurley       = 80499,
    JacksonBajheera     = 80484,
    JohnSwifty          = 79902,
    TosanGalaxyfist     = 80494,
    BrockTheCrazed      = 80498,
    AluneWindmane       = 80488,
    ChaniMalflame       = 85129,
    HildieHackerguard   = 80495,
    TaylorDewland       = 80500,
    MaldaBrewbelly      = 85122,
    ShaniFreezewind     = 80485,
    AnneOtther          = 85140,
    MathiasZunn         = 85137,
    MaxAllianceCaptains = 13,

    /// Horde
    LordMes             = 80497,
    MindbenderTalbadar  = 80490,
    ElliotVanRook       = 80493,
    VanguardSamuelle    = 80492,
    ElementalistNovo    = 80491,
    CaptainHoodrych     = 79900,
    SoulbrewerNadagast  = 80489,
    NecrolordAzael      = 80486,
    RifthunterYoske     = 80496,
    Morriz              = 85133,
    KazEndsky           = 87690,
    RazorGuerra         = 85138,
    JaredVHellstrike    = 85131,
    /// Kimilyn serve as an additional Captain for the Horde side, bringing their total to 14, above the Alliance's 13.
    /// She is not included in the Take Them Out (Alliance) achievement, likely since this would make the achievement harder for the Alliance to earn than for the Horde.
    Kimilyn             = 88109,
    MaxHordeCaptains    = 14
};

enum eArtifactsDatas
{
    /// Artifact Fragments count
    MaxCountForMage             = 400,
    MaxCountForWarlock          = 400,
    MaxCountForWarriorPaladin   = 600,
    MaxCountForDruidShaman      = 3000,

    /// Handling IDs
    CountForMage                = 0,
    CountForWarlock             = 1,
    CountForWarriorPaladin      = 2,
    CountForDruidShaman         = 3,
    MaxArtifactCounts,

    HonorConversionRate         = 3,
    ReputationConversionRate    = 5
};

enum eGameObjects
{
    /// Marketplace Graveyard
    GraveyardBannerHorde    = 233518,
    GraveyardBannerAlliance = 233517,
    GraveyardControlBanner  = 231201,

    /// Road of Glory
    CapturePointBanner      = 230876,
    BonfireWithSmokeLarge1  = 233531,
    Smallfire1              = 233534,
    FXFireMediumLowSlow     = 233535,

    /// Misc
    AncientArtifact         = 233825,

    /// Artifact events
    PortalToStormshield     = 233285,
    PortalToWarspear        = 237624,
    HordeGateway1           = 234082,
    HordeGateway2           = 234083,
    AllianceGateway1        = 234067,
    AllianceGateway2        = 234081,

    /// Stadium Racing
    AllianceRacingFlag      = 239131,
    NeutralRacingFlag       = 239132,
    HordeRacingFlag         = 239133
};

enum eAshranActions
{
    AnnounceMarketplaceGraveyard,
    AnnounceHordeGraveyard,
    AnnounceAllianceGraveyard,
    AnnounceHordeVictory,
    AnnounceAllianceKillBoss,
    AnnounceAllianceVictory,
    AnnounceHordeKillBoss,
    WarspearOutpostInFight,
    StormshieldStrongholdInFight,
    WarspearVictory,
    StormshieldVictory
};

enum eGraveyards
{
    AllianceBase    = 4742, ///< Ashran - Lane - Base GY (A)
    HordeBase       = 4743, ///< Ashran - Lane - Base GY (H)
    AllianceCenter  = 4822, ///< Ashran - Lane - Center GY (A)
    HordeCenter     = 4825, ///< Ashran - Lane - Center GY (H)
    TowerAlliance   = 4821, ///< Ashran - Lane - Tower GY (A)
    TowerHorde      = 4824, ///< Ashran - Lane - Tower GY (H)

    /// Not used yet
    Stage1Alliance  = 4769, ///< Ashran - Lane - Stage 1 (Horde Approach) - Alliance GY
    Stage1Horde     = 4770, ///< Ashran - Lane - Stage 1 (Horde Approach) - Horde GY
    Stage3Alliance  = 4768, ///< Ashran - Lane - Stage 3 (Alliance Approach) - Alliance GY
    Stage3Horde     = 4767, ///< Ashran - Lane - Stage 3 (Alliance Approach) - Horde GY
    QuarryAlliance  = 4717, ///< Ashran - Quarry - Alliance TEMP GY
    QuarryHorde     = 4718, ///< Ashran - Quarry - Horde TEMP GY
    ArenaAlliance   = 4730, ///< Ashran - The Arena - Alliance GY
    ArenaHorde      = 4731, ///< Ashran - The Arena - Horde GY

    MaxGraveyards   = 3,    ///< Only three used yet
    TotalGraveyards = 6     ///< Two for bases, Two for towers and Marketplace Graveyard
};

enum eBanners
{
    GraveyardBanner0,
    GraveyardBanner1,
    GraveyardBanner2,
    GraveyardBanner3,
    GraveyardMaxBanner
};

enum eFlagStates
{
    FlagNeutral    = 21,
    FlagHorde      = 1,
    FlagAlliance   = 2
};

enum eBattleType
{
    EmberfallTower,
    VolrathsAdvance,
    TheCrossroads,
    TrembladesVanguard,
    ArchmageOverwatch,
    MaxBattleType
};

enum eSpawns
{
    EmberfallTowerCreaturesCount        = 15,
    EmberfallTowerObjectsCount          = 12,
    EmberfallTowerSpawnsIDs             = EmberfallTowerCreaturesCount + EmberfallTowerObjectsCount,
    VolrathsAdvanceCreaturesCount       = 18,
    VolrathsAdvanceObjectsCount         = 5,
    VolrathsAdvanceSpawnsIDs            = EmberfallTowerSpawnsIDs + VolrathsAdvanceCreaturesCount + VolrathsAdvanceObjectsCount,
    TheCrossroadsCreaturesCount         = 10,
    TheCrossroadsObjectsCount           = 2,
    TheCrossroadsSpawnsIDs              = VolrathsAdvanceSpawnsIDs + TheCrossroadsCreaturesCount + TheCrossroadsObjectsCount,
    TrembladesVanguardCreaturesCount    = 14,
    TrembladesVanguardObjectsCount      = 5,
    TrembladesVanguardSpawnsIDs         = TheCrossroadsSpawnsIDs + TrembladesVanguardCreaturesCount + TrembladesVanguardObjectsCount,
    ArchmageOverwatchCreaturesCount     = 14,
    ArchmageOverwatchObjectsCount       = 12,
    ArchmageOverwatchSpawnsIDs          = TrembladesVanguardSpawnsIDs + ArchmageOverwatchCreaturesCount + ArchmageOverwatchObjectsCount,
    WarspearGladiatorsCount             = 19,
    WarspearGladiatorsSpawnsIDs         = ArchmageOverwatchSpawnsIDs + WarspearGladiatorsCount,
    StormshieldGladiatorsCount          = 16,
    StormshieldGladiatorsSpawnsIDs      = WarspearGladiatorsSpawnsIDs + StormshieldGladiatorsCount
};

enum eSpecialSpawns
{
    /// Tower guardians (Emberfall Tower & Archmage Overwatch)
    HordeTowerGuardian = StormshieldGladiatorsSpawnsIDs,
    AllianceTowerGuardian,

    /// Faction bosses (High Warlord Volrath & Grand Marshal Tremblade)
    HordeFactionBoss,
    AllianceFactionBoss,

    /// Flight masters (after a faction boss died)
    HordeTaxiToBase1,
    HordeTaxiToBase2,
    HordeFlightMaster,
    AllianceTaxiToBase1,
    AllianceTaxiToBase2,
    AllianceFlightMaster,

    /// Spirit healers
    /// Two are statics
    AllianceBaseSpiritHealer,
    HordeBaseSpiritHealer,
    /// Three are dynamics
    EmberfallTowerSpiritHealer,
    ArchmageOverwatchSpiritHealer,
    MarketplaceGraveyardSpiritHealer,

    /// Throne of the Ogre King
    NeutralKorlokTheOgreKing,
    OgreAllianceChampion,
    OgreHordeChapion,

    /// Boss factions guardians
    HordeWarlordNoktyn,
    HordeGeneralAevd,
    AllianceMarshalGabriel,
    AllianceMarshalKarshStormforge,

    /// Alliance events
    /// Mage portals
    AllianceMagePortal1,
    AllianceMagePortal2,
    AllianceVignetteDummy,
    AllianceKauper,
    AlliancePortalToStormshield,
    /// Warlock Gateways
    AllianceFalconAtherton,
    AllianceDeckerWatts,
    AllianceWarlockGateway1,
    AllianceWarlockGateway2,

    /// Horde events
    /// Mage portals
    HordeMagePortal1,
    HordeMagePortal2,
    HordeVignetteDummy,
    HordeZaramSunraiser,
    HordePortalToWarspear,
    /// Warlock Gateways
    HordeGaylePlagueheart,
    HordeIlyaPlagueheart,
    HordeWarlockGateway1,
    HordeWarlockGateway2,

    /// Horde and Alliance Guardians
    AllianceGuardian,
    HordeGuardian,
    AllianceFangraal,
    HordeKronus,

    /// Reserved IDs for Captains
    CaptainAvengerTurley,
    CaptainJacksonBajheera,
    CaptainJohnSwifty,
    CaptainTosanGalaxyfist,
    CaptainBrockTheCrazed,
    CaptainAluneWindmane,
    CaptainChaniMalflame,
    CaptainHildieHackerguard,
    CaptainTaylorDewland,
    CaptainMaldaBrewbelly,
    CaptainShaniFreezewind,
    CaptainAnneOtther,
    CaptainMathiasZunn,
    CaptainLordMes,
    CaptainMindbenderTalbadar,
    CaptainElliotVanRook,
    CaptainVanguardSamuelle,
    CaptainElementalistNovo,
    CaptainCaptainHoodrych,
    CaptainSoulbrewerNadagast,
    CaptainNecrolordAzael,
    CaptainRifthunterYoske,
    CaptainMorriz,
    CaptainKazEndsky,
    CaptainRazorGuerra,
    CaptainJaredVHellstrike,
    CaptainKimilyn,

    /// Ancient Artifact
    AncientArtifactSpawn,

    /// Stadium Racing
    AllianceRacingFlagSpawn1,
    AllianceRacingFlagSpawn2,
    NeutralRacingFlagSpawn1,
    NeutralRacingFlagSpawn2,
    NeutralRacingFlagSpawn3,
    NeutralRacingFlagSpawn4,
    HordeRacingFlagSpawn1,
    HordeRacingFlagSpawn2,
    NeutralRacingFlagSpawn5,
    NeutralRacingFlagSpawn6,
    NeutralRacingFlagSpawn7,
    NeutralRacingFlagSpawn8,
    SpeedyHordeRacerSpawn,
    HordeRiderSpawn,
    AllianceExRiderSpawn,
    AllianceRiderSpawn,

    /// Max spawn count
    MaxTowerGuardians           = 2,
    MaxFactionBosses            = 2,
    MaxTaxiToBases              = 3,
    MaxBossGuardian             = 2,
    MagePortalsCreatures        = 4,
    WarlockGatewaysCreatures    = 2,
    WarlockGatewaysObjects      = 2,
    MaxAshranCaptains           = MaxAllianceCaptains + MaxHordeCaptains,
    MaxRacingFlags              = 12,
    MaxRacingCreatures          = 4
};

enum eFactions
{
    KorlokForHorde      = 1735,
    KorlokForAlliance   = 2618,
    KorlokNeutral       = 188,
    MukmarFaction       = 1735,
    GaulDunFaction      = 1732,

    VoljinsSpear        = 1681,
    WrynnsVanguard      = 1682
};

enum eAshranVignettes
{
    VignetteKronus              = 367,
    VignetteFangraal            = 368,
    VignetteWarlockGateway1     = 431,
    VignetteStormshieldPortal   = 432,
    VignetteWarlockGateway2     = 435,
    VignetteWarspearPortal      = 436,
    VignetteKorlok              = 643
};

enum eAshranTalks
{
    ArtifactLootedByHorde       = 4,
    ArtifactLootedByAlliance    = 5,
    AllianceVictorious          = 6,
    HordeVictorious             = 7
};

struct AshranGraveyard
{
    uint32 m_ID;
    TeamId m_StartTeam;
};

struct AshranCaptain
{
    AshranCaptain()
    {
        Entry = 0;
        Type = 0;
    }

    AshranCaptain(uint32 p_Entry, uint32 type)
    {
        Entry = p_Entry;
        Type = type;
    }

    uint32 Entry;
    uint32 Type;
};

AshranCaptain const g_AshranCaptains[MaxAshranCaptains] =
{
    { AvengerTurley,         CaptainAvengerTurley      },
    { JacksonBajheera,       CaptainJacksonBajheera    },
    { JohnSwifty,            CaptainJohnSwifty         },
    { TosanGalaxyfist,       CaptainTosanGalaxyfist    },
    { BrockTheCrazed,        CaptainBrockTheCrazed     },
    { AluneWindmane,         CaptainAluneWindmane      },
    { ChaniMalflame,         CaptainChaniMalflame      },
    { HildieHackerguard,     CaptainHildieHackerguard  },
    { TaylorDewland,         CaptainTaylorDewland      },
    { MaldaBrewbelly,        CaptainMaldaBrewbelly     },
    { ShaniFreezewind,       CaptainShaniFreezewind    },
    { AnneOtther,            CaptainAnneOtther         },
    { MathiasZunn,           CaptainMathiasZunn        },
    { LordMes,               CaptainLordMes            },
    { MindbenderTalbadar,    CaptainMindbenderTalbadar },
    { ElliotVanRook,         CaptainElliotVanRook      },
    { VanguardSamuelle,      CaptainVanguardSamuelle   },
    { ElementalistNovo,      CaptainElementalistNovo   },
    { CaptainHoodrych,       CaptainCaptainHoodrych    },
    { SoulbrewerNadagast,    CaptainSoulbrewerNadagast },
    { NecrolordAzael,        CaptainNecrolordAzael     },
    { RifthunterYoske,       CaptainRifthunterYoske    },
    { Morriz,                CaptainMorriz             },
    { KazEndsky,             CaptainKazEndsky          },
    { RazorGuerra,           CaptainRazorGuerra        },
    { JaredVHellstrike,      CaptainJaredVHellstrike   },
    { Kimilyn,               CaptainKimilyn            }
};

creature_type const g_RacingCreaturesPos[MaxRacingCreatures] =
{
    { SpeedyHordeRacer, HORDE,    AshranMapID, 4768.788f, -3832.371f, 4.797024f, 0.671473f },
    { HordeRider,       HORDE,    AshranMapID, 4768.788f, -3832.371f, 4.797024f, 0.671473f },
    { ExAllianceRacer,  ALLIANCE, AshranMapID, 4805.709f, -3872.469f, 7.171597f, 3.974371f },
    { AllianceRider,    ALLIANCE, AshranMapID, 4805.709f, -3872.469f, 7.171597f, 3.974371f }
};

go_type const g_RacingFlagsPos[MaxRacingFlags] =
{
    { AllianceRacingFlag, AshranMapID, 4812.02f, -3879.64f, 7.83354f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AllianceRacingFlag, AshranMapID, 4800.17f, -3866.97f, 6.43225f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4813.62f, -3878.16f, 7.56979f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4798.85f, -3868.50f, 6.66704f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4810.55f, -3881.18f, 8.00249f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4802.15f, -3864.88f, 6.05869f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { HordeRacingFlag,    AshranMapID, 4764.51f, -3827.79f, 3.84885f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { HordeRacingFlag,    AshranMapID, 4773.66f, -3838.16f, 4.02257f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4765.65f, -3827.09f, 3.81885f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4763.41f, -3828.92f, 4.11111f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4774.54f, -3836.07f, 4.27951f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { NeutralRacingFlag,  AshranMapID, 4771.92f, -3838.98f, 4.15799f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
};

Position const g_AllianceRacingMoves[AllianceRacingMovesCount] =
{
    { 4814.682f, -3862.875f, 5.536155f },
    { 4815.376f, -3862.155f, 5.536155f },
    { 4816.498f, -3862.345f, 6.306688f },
    { 4820.679f, -3858.191f, 5.590374f },
    { 4825.521f, -3853.332f, 5.090096f },
    { 4831.548f, -3847.128f, 4.229769f },
    { 4834.912f, -3843.898f, 4.084693f },
    { 4841.262f, -3838.458f, 3.462050f },
    { 4851.496f, -3827.995f, 2.004415f },
    { 4855.956f, -3817.465f, 0.988973f },
    { 4855.874f, -3809.616f, 1.395477f },
    { 4853.279f, -3799.901f, 1.203610f },
    { 4849.236f, -3792.505f, 1.329617f },
    { 4842.870f, -3788.474f, 1.476189f },
    { 4835.221f, -3787.316f, 0.566372f },
    { 4827.774f, -3788.517f, -0.34404f },
    { 4820.616f, -3792.066f, -0.67678f },
    { 4812.039f, -3795.568f, -0.01472f },
    { 4803.723f, -3801.929f, 1.616438f },
    { 4796.220f, -3809.214f, 2.413211f },
    { 4790.671f, -3814.639f, 3.082258f },
    { 4785.616f, -3819.665f, 3.768104f },
    { 4777.779f, -3827.781f, 4.535831f },
    { 4772.362f, -3833.052f, 4.764608f },
    { 4766.514f, -3838.061f, 5.236484f },
    { 4761.383f, -3842.380f, 5.622338f },
    { 4755.631f, -3848.533f, 6.082021f },
    { 4751.801f, -3854.245f, 6.497694f },
    { 4747.529f, -3861.557f, 7.432248f },
    { 4748.080f, -3868.351f, 8.293043f },
    { 4751.052f, -3875.366f, 8.715312f },
    { 4755.855f, -3881.583f, 9.274425f },
    { 4762.792f, -3886.458f, 9.860820f },
    { 4769.792f, -3887.969f, 10.04148f },
    { 4777.491f, -3888.252f, 10.00944f },
    { 4784.366f, -3886.172f, 9.555762f },
    { 4791.596f, -3881.873f, 8.814582f },
    { 4798.754f, -3876.899f, 7.969384f },
    { 4805.709f, -3872.469f, 7.171597f }
};

Position const g_HordeRacingMoves[HordeRacingMovesCount] =
{
    { 4777.989f, -3825.748f, 4.340241f },
    { 4778.601f, -3824.957f, 4.340241f },
    { 4780.496f, -3821.010f, 3.984424f },
    { 4783.910f, -3817.453f, 3.636849f },
    { 4788.273f, -3812.526f, 3.117988f },
    { 4792.983f, -3806.557f, 2.305152f },
    { 4800.424f, -3798.637f, 1.166585f },
    { 4807.171f, -3790.910f, -0.12893f },
    { 4818.670f, -3784.517f, 0.142154f },
    { 4826.697f, -3782.406f, -0.21501f },
    { 4835.237f, -3782.021f, 0.492600f },
    { 4845.827f, -3790.156f, 0.953786f },
    { 4848.400f, -3795.340f, 0.852263f },
    { 4851.490f, -3802.050f, 1.471959f },
    { 4850.046f, -3808.712f, 1.435560f },
    { 4848.367f, -3816.236f, 1.673889f },
    { 4845.000f, -3823.087f, 2.704359f },
    { 4836.265f, -3833.030f, 3.446886f },
    { 4829.943f, -3838.101f, 3.804240f },
    { 4827.483f, -3840.512f, 3.934387f },
    { 4821.239f, -3847.049f, 4.479604f },
    { 4816.172f, -3852.036f, 5.014072f },
    { 4810.801f, -3856.542f, 5.240987f },
    { 4805.717f, -3862.576f, 5.969340f },
    { 4800.070f, -3868.363f, 6.689080f },
    { 4796.327f, -3871.899f, 7.165856f },
    { 4790.208f, -3877.332f, 8.006101f },
    { 4783.330f, -3882.576f, 8.943475f },
    { 4775.374f, -3890.049f, 10.27254f },
    { 4761.454f, -3892.630f, 10.50814f },
    { 4749.913f, -3880.188f, 9.045112f },
    { 4750.520f, -3865.828f, 7.953274f },
    { 4753.360f, -3860.568f, 7.074049f },
    { 4756.783f, -3852.842f, 6.472168f },
    { 4763.476f, -3839.750f, 5.462884f },
    { 4768.788f, -3832.371f, 4.797024f }
};

go_type const g_AncientArtifactPos[AncientArtifactCount] =
{
    { AncientArtifact,    AshranMapID, 4966.47f, -3720.54f, 3.63465f, 6.27924f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4889.49f, -3750.56f, 12.1343f, 3.96231f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4820.18f, -3652.79f, 0.49170f, 4.64168f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4204.46f, -3971.46f, 10.6328f, 5.50052f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4348.12f, -3898.68f, 8.38964f, 1.61672f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4092.32f, -4497.17f, 86.2245f, 1.53424f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4592.43f, -4317.44f, 18.9973f, 3.27390f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 4977.82f, -4643.51f, 91.2159f, 2.13901f, 0.0f, 0.0f, 0.0f, 0.0f },
    { AncientArtifact,    AshranMapID, 5068.70f, -4606.89f, 51.1554f, 1.49499f, 0.0f, 0.0f, 0.0f, 0.0f }
};

creature_type const g_StormshieldGladiators[StormshieldGladiatorsCount] =
{
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4156.29f, -4176.58f, 36.4109f, 5.8621f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4164.74f, -4163.67f, 36.8250f, 5.2542f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4099.48f, -4122.77f, 47.6154f, 4.0449f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4093.27f, -4131.02f, 47.6142f, 0.9267f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4049.91f, -4097.18f, 50.7366f, 5.3448f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4032.91f, -4044.67f, 58.9775f, 3.5312f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4021.08f, -4131.07f, 51.9782f, 1.2920f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 4015.75f, -4079.92f, 59.3681f, 2.5316f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3997.71f, -4031.23f, 57.1706f, 4.6809f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3996.25f, -4126.35f, 57.9055f, 0.8888f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3989.01f, -4030.99f, 57.1706f, 4.6809f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3940.46f, -4080.42f, 66.9511f, 3.8684f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3935.03f, -4070.07f, 76.0506f, 4.5880f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3928.70f, -4098.29f, 66.9150f, 1.4584f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3915.77f, -4073.19f, 67.3723f, 4.8286f },
    { StormshieldGladiator, ALLIANCE, AshranMapID, 3912.61f, -4097.09f, 75.8634f, 6.1543f }
};

creature_type const g_WarspearGladiators[WarspearGladiatorsCount] =
{
    { WarspearGladiator, HORDE, AshranMapID, 4946.54f, -4175.01f, 38.1617f, 3.50328f },
    { WarspearGladiator, HORDE, AshranMapID, 5131.97f, -4126.99f, 59.2029f, 3.46303f },
    { WarspearGladiator, HORDE, AshranMapID, 4975.60f, -4132.03f, 46.0504f, 1.21083f },
    { WarspearGladiator, HORDE, AshranMapID, 5032.04f, -4243.52f, 42.6787f, 3.44195f },
    { WarspearGladiator, HORDE, AshranMapID, 5055.02f, -4209.76f, 41.9912f, 1.74085f },
    { WarspearGladiator, HORDE, AshranMapID, 5102.40f, -4119.96f, 59.1992f, 4.92061f },
    { WarspearGladiator, HORDE, AshranMapID, 5008.90f, -4182.19f, 47.5115f, 2.54578f },
    { WarspearGladiator, HORDE, AshranMapID, 4871.65f, -4205.86f, 34.3379f, 3.81429f },
    { WarspearGladiator, HORDE, AshranMapID, 5112.08f, -4109.43f, 59.2032f, 4.43069f },
    { WarspearGladiator, HORDE, AshranMapID, 5014.87f, -4104.30f, 44.9336f, 5.34918f },
    { WarspearGladiator, HORDE, AshranMapID, 4876.68f, -4213.28f, 34.3620f, 3.35024f },
    { WarspearGladiator, HORDE, AshranMapID, 4991.81f, -4177.18f, 43.4432f, 2.56387f },
    { WarspearGladiator, HORDE, AshranMapID, 5116.34f, -4136.79f, 59.2032f, 3.17963f },
    { WarspearGladiator, HORDE, AshranMapID, 5082.56f, -4088.73f, 51.4390f, 3.16257f },
    { WarspearGladiator, HORDE, AshranMapID, 5012.77f, -4165.94f, 45.9783f, 3.37161f },
    { WarspearGladiator, HORDE, AshranMapID, 5066.76f, -4207.80f, 42.9071f, 1.72416f },
    { WarspearGladiator, HORDE, AshranMapID, 4942.65f, -4166.76f, 38.0596f, 3.64596f },
    { WarspearGladiator, HORDE, AshranMapID, 5075.51f, -4076.22f, 50.8183f, 4.14423f },
    { WarspearGladiator, HORDE, AshranMapID, 4993.67f, -4153.33f, 43.4391f, 3.82256f }
};

creature_type const g_AllianceFangraal  = { Frangraal,  ALLIANCE, AshranMapID, 3993.69f, -4097.46f, 57.5987f, 1.03238f };
creature_type const g_HordeKronus       = { Kronus,     HORDE,    AshranMapID, 5062.01f, -4202.66f, 49.1949f, 1.59762f };

creature_type const g_AllianceGuardian  = { LifelessAncient,        ALLIANCE, AshranMapID, 3983.90f, -4117.12f, 58.0536f, 1.10755f };
creature_type const g_HordeGuardian     = { UnderpoweredEarthFury,  HORDE,    AshranMapID, 5064.02f, -4232.31f, 41.4964f, 2.03435f };

go_type const g_WarlockGatewaysGob[TEAM_NEUTRAL][WarlockGatewaysObjects] =
{
    /// Alliance
    {
        { AllianceGateway1, AshranMapID, 4167.15f, -4541.17f, 78.3000f, 0.31959f, 0.0f, 0.0f, 0.0f, 0.0f },
        { AllianceGateway2, AshranMapID, 4941.93f, -3745.08f, 2.05739f, 5.75514f, 0.0f, 0.0f, 0.0f, 0.0f }
    },
    /// Horde
    {
        { HordeGateway1, AshranMapID, 4768.854f, -3714.03f, 1.33882f, 0.5902176f, 0.0f, 0.0f, 0.0f, 0.0f },
        { HordeGateway2, AshranMapID, 4028.808f, -4498.14f, 85.7033f, 1.9328010f, 0.0f, 0.0f, 0.0f, 0.0f }
    }
};

creature_type const g_WarlockGatewaysSpawns[TEAM_NEUTRAL][WarlockGatewaysCreatures] =
{
    /// Alliance
    {
        { FalconAtherton,   ALLIANCE, AshranMapID, 4172.79f, -4538.52f, 78.1886f, 0.749476f },
        { DeckerWatts,      ALLIANCE, AshranMapID, 4949.16f, -3748.60f, 2.85976f, 5.726880f }
    },
    /// Horde
    {
        { GaylePlagueheart, HORDE, AshranMapID, 4025.942f, -4491.694f, 85.82835f, 1.9133300f },
        { IlyaPlagueheart,  HORDE, AshranMapID, 4773.523f, -3710.321f, 1.422153f, 0.8231511f }
    }
};

go_type const g_MagePortalsGob[TEAM_NEUTRAL] =
{
    { PortalToStormshield,    AshranMapID, 4645.94f, -4097.77f, 22.6435f, 5.14771f, 0.0f, 0.0f, 0.0f, 0.0f },  ///< Alliance
    { PortalToWarspear,       AshranMapID, 4428.61f, -4082.23f, 28.1979f, 1.69192f, 0.0f, 0.0f, 0.0f, 0.0f }   ///< Horde
};

creature_type const g_MagePortalsSpawns[TEAM_NEUTRAL][MagePortalsCreatures] =
{
    /// Alliance
    {
        { PortalMageA,      ALLIANCE, AshranMapID, 4641.67f, -4099.74f, 22.49f, 0.423455f   },
        { PortalMageA,      ALLIANCE, AshranMapID, 4649.82f, -4100.56f, 22.49f, 2.602931f   },
        { VignetteDummyA,   ALLIANCE, AshranMapID, 4646.04f, -4098.80f, 22.70f, 0.0f        },
        { Kauper,           ALLIANCE, AshranMapID, 4646.18f, -4093.56f, 22.63f, 1.534031f   }
    },
    /// Horde
    {
        { PortalMageH,      HORDE, AshranMapID, 4432.37f, -4084.51f, 28.15f, 2.839093f  },
        { PortalMageH,      HORDE, AshranMapID, 4424.04f, -4084.11f, 28.15f, 0.283151f  },
        { VignetteDummyH,   HORDE, AshranMapID, 4428.70f, -4082.89f, 28.28f, 0.0f       },
        { ZaramSunraiser,   HORDE, AshranMapID, 4428.91f, -4078.07f, 28.20f, 1.69192f  }
    }
};

uint32 const g_MaxArtifactsToCollect[MaxArtifactCounts] =
{
    MaxCountForMage,
    MaxCountForWarlock,
    MaxCountForWarriorPaladin,
    MaxCountForDruidShaman
};

uint32 const g_ArtifactsWorldStates[TEAM_NEUTRAL][MaxArtifactCounts] =
{
    /// Alliance
    {
        WorldStateAllianceMageArtifactCount,
        WorldStateAllianceWarlockArtifactCount,
        WorldStateAllianceWarriorArtifactCount,
        WorldStateAllianceShamanArtifactCount
    },
    /// Horde
    {
        WorldStateHordeMageArtifactCount,
        WorldStateHordeWarlockArtifactCount,
        WorldStateHordeWarriorArtifactCount,
        WorldStateHordeShamanArtifactCount
    }
};

AshranGraveyard const g_AshranGraveyards[TotalGraveyards] =
{
    { AllianceBase,    TEAM_ALLIANCE   },  ///< 0 - Alliance base
    { HordeBase,       TEAM_HORDE      },  ///< 1 - Horde base
    { AllianceCenter,  TEAM_NEUTRAL    },  ///< 2 - Marketplace GY (A)
    { HordeCenter,     TEAM_NEUTRAL    },  ///< 3 - Marketplace GY (H)
    { TowerAlliance,   TEAM_ALLIANCE   },  ///< 4 - Archmage Overwatch
    { TowerHorde,      TEAM_HORDE      }   ///< 5 - Emberfall Tower
};

uint32 const g_GraveyardIDs[TEAM_NEUTRAL][MaxGraveyards] =
{
    /// Alliance
    {
        AllianceBase,
        TowerAlliance,
        AllianceCenter
    },
    /// Horde
    {
        HordeBase,
        TowerHorde,
        HordeCenter
    }
};

Position const g_HordeTeleportPos = { 5216.443359f, -3963.191406f, 5.553593f, 6.242684f };
Position const g_AllianceTeleportPos = { 3849.396240f, -4013.051025f, 26.282335f, 3.141932f };

uint32 const g_HallowedGroundEntries[TEAM_NEUTRAL] =
{
    AshranHallowedGroundH,
    AshranHallowedGroundA
};

Position const g_HallowedGroundPos[TEAM_NEUTRAL] =
{
    { 5090.467f, -4076.731f, 49.38393f, 3.379836f },    ///< eAshranDatas::AshranHallowedGroundH
    { 3928.052f, -4032.738f, 57.41695f, 5.473989f }     ///< eAshranDatas::AshranHallowedGroundA
};

go_type const g_GraveyardBanner_H[GraveyardMaxBanner] =
{
    { GraveyardBannerHorde, AshranMapID, 4527.93f, -3999.18f, 5.95707f, 0.588123f, 0.00f, 0.00f, 0.00f, 0.00f },
    { GraveyardBannerHorde, AshranMapID, 4528.02f, -4006.75f, 6.05358f, 2.277280f, 0.00f, 0.00f, 0.00f, 0.00f },
    { GraveyardBannerHorde, AshranMapID, 4537.33f, -3999.50f, 6.13882f, 5.566630f, 0.00f, 0.00f, 0.00f, 0.00f },
    { GraveyardBannerHorde, AshranMapID, 4536.55f, -4006.41f, 6.38824f, 4.122270f, 0.00f, 0.00f, 0.00f, 0.00f }
};

go_type const g_GraveyardBanner_A[GraveyardMaxBanner] =
{
    { GraveyardBannerAlliance, AshranMapID, 4527.93f, -3999.18f, 5.95707f, 0.588123f, 0.00f, 0.00f, 0.00f, 0.00f },
    { GraveyardBannerAlliance, AshranMapID, 4528.02f, -4006.75f, 6.05358f, 2.277280f, 0.00f, 0.00f, 0.00f, 0.00f },
    { GraveyardBannerAlliance, AshranMapID, 4537.33f, -3999.50f, 6.13882f, 5.566630f, 0.00f, 0.00f, 0.00f, 0.00f },
    { GraveyardBannerAlliance, AshranMapID, 4536.55f, -4006.41f, 6.38824f, 4.122270f, 0.00f, 0.00f, 0.00f, 0.00f }
};

go_type const g_GraveyardBanner_N = { GraveyardControlBanner, AshranMapID, 4532.632f, -4003.269f, 6.317888f, 0.0f, 0.0f, 0.0f, 0.008727f, -0.999962f };

uint32 const g_TowerControlStatus[MaxBattleType] =
{
    WorldStateEmberfallTowerStatus,
    WorldStateVolrathsAdvanceStatus,
    WorldStateTheCrossroadsStatus,
    WorldStateTrembladesVanguardStatus,
    WorldStateArchmageOverwatchStatus
};

creature_type const g_MarketplaceGraveyardSpirits[TEAM_NEUTRAL] =
{
    { AllianceSpiritGuide, ALLIANCE, AshranMapID, 4532.90f, -4007.06f, 6.08817f, 4.7095f },
    { HordeSpiritGuide,    HORDE,    AshranMapID, 4532.90f, -4007.06f, 6.08817f, 4.7095f }
};

creature_type const g_EmberfallTowerSpiritHealer[TEAM_NEUTRAL] =
{
    { AllianceSpiritGuide, ALLIANCE, AshranMapID, 4846.03f, -4186.43f, 31.7727f, 2.7156f },
    { HordeSpiritGuide,    HORDE,    AshranMapID, 4846.03f, -4186.43f, 31.7727f, 2.7156f }
};

creature_type const g_ArchmageOverwatchSpiritHealer[TEAM_NEUTRAL] =
{
    { AllianceSpiritGuide, ALLIANCE, AshranMapID, 4192.70f, -4152.65f, 31.7642f, 0.0512f },
    { HordeSpiritGuide,    HORDE,    AshranMapID, 4192.70f, -4152.65f, 31.7642f, 0.0512f }
};

creature_type const g_BasesSpiritHealers[TEAM_NEUTRAL] =
{
    { AllianceSpiritGuide, ALLIANCE, AshranMapID, 3924.49f, -4030.79f, 59.2817f, 5.9936f },
    { HordeSpiritGuide,    HORDE,    AshranMapID, 5089.37f, -4077.54f, 50.9001f, 3.7238f }
};

creature_type const g_FactionGuardians[MaxTowerGuardians] =
{
    { RylaiCrestfall, ALLIANCE, AshranMapID, 4271.71f, -4202.35f, 55.0845f, 6.0219f },
    { JeronEmberfall, HORDE,    AshranMapID, 4763.33f, -4231.81f, 56.6295f, 3.0479f }
};

/// Three spawn positions depending on towers status
creature_type const g_FactionBossesSpawn[MaxFactionBosses * 3] =
{
    { GrandMarshalTremblade, ALLIANCE, AshranMapID, 3914.96f, -4087.11f, 66.53f, 0.1372f }, ///< The Crossroads
    { GrandMarshalTremblade, ALLIANCE, AshranMapID, 3955.19f, -4081.59f, 63.72f, 0.1411f }, ///< Tremblade's Vanguard
    { GrandMarshalTremblade, ALLIANCE, AshranMapID, 3991.94f, -4094.43f, 57.47f, 6.0214f }, ///< Archmage Overwatch
    { HighWarlordVolrath,    HORDE,    AshranMapID, 5125.03f, -4115.48f, 59.13f, 3.8966f }, ///< The Crossroads
    { HighWarlordVolrath,    HORDE,    AshranMapID, 5073.46f, -4160.39f, 47.21f, 3.8534f }, ///< Volrath's Advance
    { HighWarlordVolrath,    HORDE,    AshranMapID, 5035.13f, -4177.21f, 46.15f, 2.9979f }  ///< Emberfall Tower
};

creature_type const g_FactionBossesGuardians[MaxBossGuardian * 2 * 3] =
{
    { MarshalKarshStormforge,   ALLIANCE, AshranMapID, 3922.10f, -4080.21f, 66.53f, 4.8326f },    ///< The Crossroads
    { MarshalGabriel,           ALLIANCE, AshranMapID, 3923.77f, -4091.46f, 66.53f, 1.7185f },    ///< The Crossroads
    { MarshalKarshStormforge,   ALLIANCE, AshranMapID, 3954.91f, -4074.61f, 63.81f, 5.9717f },    ///< Tremblade's Vanguard
    { MarshalGabriel,           ALLIANCE, AshranMapID, 3957.60f, -4089.45f, 63.22f, 0.5868f },    ///< Tremblade's Vanguard
    { MarshalKarshStormforge,   ALLIANCE, AshranMapID, 3993.42f, -4089.08f, 57.23f, 5.8411f },    ///< Archmage Overwatch
    { MarshalGabriel,           ALLIANCE, AshranMapID, 3991.55f, -4100.32f, 57.79f, 0.0764f },    ///< Archmage Overwatch
    { GeneralAevd,              HORDE,    AshranMapID, 5120.18f, -4112.07f, 59.13f, 4.4784f },    ///< The Crossroads
    { WarlordNoktyn,            HORDE,    AshranMapID, 5127.97f, -4120.82f, 59.21f, 3.5152f },    ///< The Crossroads
    { GeneralAevd,              HORDE,    AshranMapID, 5064.04f, -4152.86f, 47.76f, 4.0072f },    ///< Volrath's Advance
    { WarlordNoktyn,            HORDE,    AshranMapID, 5076.36f, -4168.92f, 46.80f, 3.1864f },    ///< Volrath's Advance
    { GeneralAevd,              HORDE,    AshranMapID, 5033.13f, -4165.64f, 46.64f, 3.7904f },    ///< Emberfall Tower
    { WarlordNoktyn,            HORDE,    AshranMapID, 5032.13f, -4185.38f, 47.08f, 2.5503f },    ///< Emberfall Tower
};

creature_type const g_FactionTaxisToBase[TEAM_NEUTRAL][MaxTaxiToBases] =
{
    /// Alliance
    {
        { StormshieldGryphon, ALLIANCE, AshranMapID, 4969.05f, -4188.10f, 40.6841f, 0.87024f },
        { StormshieldGryphon, ALLIANCE, AshranMapID, 4965.20f, -4181.11f, 40.5744f, 0.09716f },
        { TinaKelatara,       ALLIANCE, AshranMapID, 4966.77f, -4184.87f, 40.6023f, 0.48167f }
    },
    /// Horde
    {
        { WarspearWyvern, HORDE, AshranMapID, 4053.90f, -4129.36f, 48.0519f, 2.01738f },
        { WarspearWyvern, HORDE, AshranMapID, 4048.99f, -4130.84f, 48.1592f, 1.86423f },
        { ShevanManille,  HORDE, AshranMapID, 4051.13f, -4131.14f, 48.1242f, 1.87993f }
    }
};

creature_type const g_EmberfallTowerSpawns[TEAM_NEUTRAL][EmberfallTowerCreaturesCount] =
{
    // ALLIANCE
    {
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4791.99f, -4180.81f, 33.22f, 4.95f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4788.07f, -4182.17f, 33.22f, 5.80f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4775.78f, -4217.50f, 32.58f, 0.00f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4776.99f, -4220.96f, 32.33f, 0.91f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4788.38f, -4226.64f, 56.56f, 0.80f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4780.22f, -4219.33f, 56.11f, 1.43f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4795.50f, -4237.04f, 56.35f, 6.15f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4785.99f, -4247.97f, 56.57f, 5.44f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4767.79f, -4228.28f, 56.62f, 2.25f },
        { StormshieldKnight,    ALLIANCE, AshranMapID, 4781.27f, -4265.98f, 27.76f, 3.72f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4783.97f, -4261.41f, 28.89f, 3.72f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4787.13f, -4266.29f, 28.49f, 3.72f },
        { StormshieldKnight,    ALLIANCE, AshranMapID, 4816.92f, -4207.30f, 32.73f, 1.97f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4821.17f, -4209.80f, 32.63f, 1.97f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4813.77f, -4212.76f, 33.13f, 1.97f }
    },
    // HORDE
    {
        { WarspearBloodGuard,  HORDE, AshranMapID, 4791.99f, -4180.81f, 33.22f, 4.95f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4788.07f, -4182.17f, 33.22f, 5.80f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4775.78f, -4217.50f, 32.58f, 0.00f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4776.99f, -4220.96f, 32.33f, 0.91f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4788.38f, -4226.64f, 56.56f, 0.80f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4780.22f, -4219.33f, 56.11f, 1.43f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4795.50f, -4237.04f, 56.35f, 6.15f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4785.99f, -4247.97f, 56.57f, 5.44f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4767.79f, -4228.28f, 56.62f, 2.25f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4781.27f, -4265.98f, 27.76f, 3.72f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4783.97f, -4261.41f, 28.89f, 3.72f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4787.13f, -4266.29f, 28.49f, 3.72f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4816.92f, -4207.30f, 32.73f, 1.97f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4821.17f, -4209.80f, 32.63f, 1.97f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4813.77f, -4212.76f, 33.13f, 1.97f }
    }
};

creature_type const g_EmberfallTowerNeutralSpawns[EmberfallTowerCreaturesCount] =
{
    { WarspearGrunt,      HORDE,    AshranMapID, 4791.99f, -4180.81f, 33.22f, 4.95f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4788.07f, -4182.17f, 33.22f, 5.80f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4775.78f, -4217.50f, 32.58f, 0.00f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4776.99f, -4220.96f, 32.33f, 0.91f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4824.54f, -4183.08f, 31.66f, 3.81f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4827.80f, -4189.65f, 31.68f, 3.41f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4802.03f, -4234.42f, 33.04f, 5.03f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4810.90f, -4247.38f, 33.12f, 2.83f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4789.45f, -4185.29f, 32.91f, 1.28f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4794.69f, -4180.93f, 33.22f, 2.73f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4817.72f, -4186.66f, 31.83f, 0.07f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4822.87f, -4192.89f, 31.68f, 1.17f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4775.27f, -4224.87f, 32.67f, 0.55f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4772.74f, -4218.13f, 32.32f, 0.36f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4768.51f, -4225.33f, 31.27f, 0.63f }
};

go_type const g_EmberfallFiresSpawns[EmberfallTowerObjectsCount] =
{
    { BonfireWithSmokeLarge1, AshranMapID, 4778.00f, -4224.00f, 56.68f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { BonfireWithSmokeLarge1, AshranMapID, 4790.39f, -4241.27f, 56.15f, 0.09f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4812.25f, -4252.67f, 37.19f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4796.12f, -4234.99f, 37.11f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4827.44f, -4195.64f, 37.34f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4816.45f, -4164.16f, 35.71f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4799.46f, -4164.38f, 35.47f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4776.97f, -4185.94f, 41.45f, 2.35f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4769.31f, -4182.60f, 46.10f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4756.42f, -4175.85f, 38.16f, 0.41f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4749.02f, -4183.04f, 38.13f, 0.41f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4768.50f, -4245.03f, 47.74f, 3.60f, 0.00f, 0.00f, 0.00f, 0.00f }
};

creature_type const g_VolrathsAdvanceSpawns[TEAM_NEUTRAL][VolrathsAdvanceCreaturesCount] =
{
    // ALLIANCE
    {
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4671.80f, -4242.89f, 29.6425f, 6.10519f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4653.12f, -4206.77f, 28.3985f, 4.78364f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4679.35f, -4242.53f, 28.9622f, 2.04980f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4654.56f, -4199.31f, 27.2997f, 5.45237f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4652.03f, -4218.00f, 9.92943f, 4.59145f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4652.04f, -4223.96f, 9.92325f, 1.58571f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4654.54f, -4224.20f, 11.4234f, 0.00943f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4654.81f, -4218.55f, 11.4234f, 0.00935f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4683.18f, -4211.89f, 10.3594f, 3.00827f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4683.80f, -4205.95f, 10.1736f, 3.14209f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4688.85f, -4230.05f, 11.1861f, 0.24750f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4693.90f, -4232.60f, 11.1861f, 0.43090f },
        { StormshieldKnight,   ALLIANCE, AshranMapID, 4650.39f, -4220.74f, 9.55521f, 6.17425f },
        { StormshieldKnight,   ALLIANCE, AshranMapID, 4681.88f, -4208.51f, 10.2640f, 3.28998f },
        { StormshieldPriest,   ALLIANCE, AshranMapID, 4725.44f, -4203.77f, 22.3601f, 3.56446f },
        { StormshieldPriest,   ALLIANCE, AshranMapID, 4651.05f, -4211.01f, 9.87623f, 5.53973f },
        { StormshieldPriest,   ALLIANCE, AshranMapID, 4668.79f, -4257.62f, 11.5811f, 1.59311f },
        { StormshieldPriest,   ALLIANCE, AshranMapID, 4682.44f, -4184.54f, 21.8149f, 4.45475f }
    },
    // HORDE
    {
        { WarspearHeadhunter,  HORDE, AshranMapID, 4671.80f, -4242.89f, 29.6425f, 6.10519f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4653.12f, -4206.77f, 28.3985f, 4.78364f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4679.35f, -4242.53f, 28.9622f, 2.04980f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4654.56f, -4199.31f, 27.2997f, 5.45237f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4652.03f, -4218.00f, 9.92943f, 4.59145f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4652.04f, -4223.96f, 9.92325f, 1.58571f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4654.54f, -4224.20f, 11.4234f, 0.00943f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4654.81f, -4218.55f, 11.4234f, 0.00935f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4683.18f, -4211.89f, 10.3594f, 3.00827f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4683.80f, -4205.95f, 10.1736f, 3.14209f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4688.85f, -4230.05f, 11.1861f, 0.24750f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4693.90f, -4232.60f, 11.1861f, 0.43090f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4650.39f, -4220.74f, 9.55521f, 6.17425f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4681.88f, -4208.51f, 10.2640f, 3.28998f },
        { WarspearPriest,      HORDE, AshranMapID, 4725.44f, -4203.77f, 22.3601f, 3.56446f },
        { WarspearPriest,      HORDE, AshranMapID, 4651.05f, -4211.01f, 9.87623f, 5.53973f },
        { WarspearPriest,      HORDE, AshranMapID, 4668.79f, -4257.62f, 11.5811f, 1.59311f },
        { WarspearPriest,      HORDE, AshranMapID, 4682.44f, -4184.54f, 21.8149f, 4.45475f }
    }
};

creature_type const g_VolrathsAdvanceNeutralSpawns[VolrathsAdvanceCreaturesCount] =
{
    { WarspearGrunt,      HORDE,    AshranMapID, 4709.55f, -4221.30f, 10.9748f, 5.71988f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4713.37f, -4218.79f, 11.1445f, 5.13476f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4682.40f, -4265.17f, 12.1166f, 0.39958f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4681.34f, -4259.01f, 11.6778f, 6.22331f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4682.55f, -4185.68f, 21.4261f, 6.22722f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4683.04f, -4180.22f, 23.2955f, 5.99160f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4648.47f, -4232.44f, 10.1874f, 4.64069f },
    { WarspearPriest,     HORDE,    AshranMapID, 4732.59f, -4210.11f, 24.4721f, 3.63575f },
    { WarspearPriest,     HORDE,    AshranMapID, 4663.53f, -4201.14f, 9.47571f, 5.10444f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4718.06f, -4222.33f, 11.1647f, 2.62541f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4713.48f, -4226.52f, 11.1247f, 1.92248f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4694.93f, -4262.08f, 11.8211f, 3.12492f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4692.43f, -4256.23f, 11.8035f, 3.69040f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4697.10f, -4181.13f, 22.7873f, 3.26234f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4695.53f, -4187.58f, 20.5008f, 2.86964f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4646.38f, -4243.53f, 10.9999f, 1.22023f },
    { StormshieldPriest,  ALLIANCE, AshranMapID, 4657.05f, -4256.82f, 13.1483f, 1.58978f },
    { StormshieldPriest,  ALLIANCE, AshranMapID, 4652.04f, -4210.78f, 9.98361f, 5.58745f }
};

go_type const g_VolrathsAdvanceFires[VolrathsAdvanceObjectsCount] =
{
    { Smallfire1, AshranMapID, 4720.46f, -4225.85f, 16.50f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4715.67f, -4239.35f, 13.88f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4703.38f, -4247.46f, 13.71f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4642.08f, -4236.12f, 13.30f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4646.33f, -4207.88f, 12.42f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f }
};

creature_type const g_CrossroadSpawns[TEAM_NEUTRAL][TheCrossroadsCreaturesCount] =
{
    // Alliance
    {
        { StormshieldKnight,    ALLIANCE, AshranMapID, 4478.58f, -4198.21f, 6.18646f, 2.85147f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4481.10f, -4196.31f, 7.18646f, 2.85147f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4480.48f, -4201.73f, 2.92569f, 2.85147f },
        { StormshieldKnight,    ALLIANCE, AshranMapID, 4592.31f, -4218.70f, 7.07966f, 3.05845f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4593.84f, -4221.67f, 7.27999f, 3.05845f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4594.30f, -4216.03f, 7.27999f, 3.05845f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4526.08f, -4183.99f, 7.08054f, 0.02788f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4537.70f, -4183.91f, 7.07746f, 3.18518f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4526.93f, -4238.27f, 7.11127f, 6.25608f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4540.17f, -4238.74f, 7.16359f, 3.11056f }
    },
    // Horde
    {
        { WarspearRaptorRider, HORDE, AshranMapID, 4478.58f, -4198.21f, 6.18646f, 2.85147f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4481.10f, -4196.31f, 7.18646f, 2.85147f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4480.48f, -4201.73f, 2.92569f, 2.85147f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4592.31f, -4218.70f, 7.07966f, 3.05845f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4593.84f, -4221.67f, 7.27999f, 3.05845f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4594.30f, -4216.03f, 7.27999f, 3.05845f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4526.08f, -4183.99f, 7.08054f, 0.02788f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4537.70f, -4183.91f, 7.07746f, 3.18518f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4526.93f, -4238.27f, 7.11127f, 6.25608f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4540.17f, -4238.74f, 7.16359f, 3.11056f }
    }
};

creature_type const g_CrossroadsNeutralSpawns[TheCrossroadsCreaturesCount] =
{
    { WarspearGrunt,      HORDE,    AshranMapID, 4540.60f, -4201.94f, 6.95165f, 2.68286f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4543.13f, -4196.73f, 6.96004f, 2.79282f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4535.51f, -4243.45f, 7.26939f, 1.68147f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4539.97f, -4234.36f, 7.27037f, 4.51675f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4537.95f, -4169.41f, 7.09234f, 3.12659f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4535.55f, -4193.37f, 7.02832f, 5.67916f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4531.69f, -4200.82f, 7.05973f, 5.90299f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4528.11f, -4243.37f, 7.11128f, 1.50082f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4528.08f, -4234.97f, 7.14110f, 4.84268f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4527.00f, -4169.13f, 7.09213f, 6.21319f }
};

go_type const g_CrossroadsBanners[TEAM_NEUTRAL][TheCrossroadsObjectsCount] =
{
    // ALLIANCE
    {
        { GraveyardBannerAlliance, AshranMapID, 4546.98f, -4195.84f, 7.10f, 3.79f, 0.00f, 0.00f, 0.00f, 0.00f },
        { GraveyardBannerAlliance, AshranMapID, 4513.73f, -4220.42f, 7.14f, 0.53f, 0.00f, 0.00f, 0.00f, 0.00f }
    },
    // HORDE
    {
        { GraveyardBannerHorde, AshranMapID, 4546.98f, -4195.84f, 7.10f, 3.79f, 0.00f, 0.00f, 0.00f, 0.00f },
        { GraveyardBannerHorde, AshranMapID, 4513.73f, -4220.42f, 7.14f, 0.53f, 0.00f, 0.00f, 0.00f, 0.00f }
    }
};

creature_type const g_TrembladesVanguardSpawns[TEAM_NEUTRAL][TrembladesVanguardCreaturesCount] =
{
    // Alliance
    {
        { StormshieldKnight,    ALLIANCE, AshranMapID, 4353.65f, -4206.14f, 10.2067f, 6.11085f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4347.50f, -4202.97f, 10.2067f, 6.11085f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4346.38f, -4208.36f, 10.2067f, 6.11085f },
        { StormshieldKnight,    ALLIANCE, AshranMapID, 4406.39f, -4188.80f, 7.57000f, 3.17523f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4410.28f, -4191.92f, 7.46672f, 3.08886f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4410.58f, -4186.27f, 7.57197f, 3.08906f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4368.73f, -4186.40f, 10.3865f, 1.85123f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4384.90f, -4186.78f, 9.54397f, 2.45305f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4380.17f, -4191.64f, 9.98860f, 2.90894f },
        { StormshieldVanguard,  ALLIANCE, AshranMapID, 4358.06f, -4179.99f, 10.2831f, 2.98455f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4364.32f, -4167.70f, 25.7766f, 4.71050f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4345.87f, -4167.31f, 26.7916f, 5.90887f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4358.52f, -4220.68f, 27.9513f, 5.29877f },
        { StormshieldSentinel,  ALLIANCE, AshranMapID, 4394.62f, -4208.00f, 28.0643f, 2.80040f }
    },
    // Horde
    {
        { WarspearRaptorRider, HORDE, AshranMapID, 4353.65f, -4206.14f, 10.2067f, 6.11085f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4347.50f, -4202.97f, 10.2067f, 6.11085f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4346.38f, -4208.36f, 10.2067f, 6.11085f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4406.39f, -4188.80f, 7.57000f, 3.17523f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4410.28f, -4191.92f, 7.46672f, 3.08886f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4410.58f, -4186.27f, 7.57197f, 3.08906f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4368.73f, -4186.40f, 10.3865f, 1.85123f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4384.90f, -4186.78f, 9.54397f, 2.45305f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4380.17f, -4191.64f, 9.98860f, 2.90894f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4358.06f, -4179.99f, 10.2831f, 2.98455f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4364.32f, -4167.70f, 25.7766f, 4.71050f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4345.87f, -4167.31f, 26.7916f, 5.90887f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4358.52f, -4220.68f, 27.9513f, 5.29877f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4394.62f, -4208.00f, 28.0643f, 2.80040f }
    }
};

creature_type const g_TrembladesVanguardNeutralSpawns[TrembladesVanguardCreaturesCount] =
{
    { WarspearGrunt,      HORDE,    AshranMapID, 4375.42f, -4175.97f, 11.1146f, 2.25815f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4320.26f, -4202.23f, 10.8944f, 2.15211f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4342.93f, -4221.81f, 11.8033f, 3.66008f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4389.26f, -4201.89f, 10.8939f, 3.46087f },
    { WarspearGrunt,      HORDE,    AshranMapID, 4384.46f, -4195.61f, 10.1722f, 3.99101f },
    { WarspearPriest,     HORDE,    AshranMapID, 4389.14f, -4177.96f, 9.59706f, 0.40085f },
    { WarspearPriest,     HORDE,    AshranMapID, 4317.07f, -4200.65f, 10.9585f, 0.40085f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4369.57f, -4169.96f, 11.2516f, 5.39975f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4315.65f, -4194.23f, 10.7408f, 5.36439f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4333.98f, -4230.01f, 12.4511f, 0.67163f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4379.19f, -4202.59f, 11.2631f, 0.63343f },
    { StormshieldFootman, ALLIANCE, AshranMapID, 4382.84f, -4207.47f, 11.7597f, 0.92795f },
    { StormshieldPriest,  ALLIANCE, AshranMapID, 4383.32f, -4215.80f, 11.5476f, 0.70961f },
    { StormshieldPriest,  ALLIANCE, AshranMapID, 4382.09f, -4216.85f, 11.4726f, 0.40085f }
};

go_type const g_TrembladesVanguardFires[TrembladesVanguardObjectsCount] =
{
    { Smallfire1, AshranMapID, 4394.97f, -4176.13f, 13.97f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4395.08f, -4201.61f, 14.55f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4332.16f, -4221.05f, 16.76f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4316.36f, -4206.56f, 16.92f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1, AshranMapID, 4307.32f, -4195.97f, 10.80f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f }
};

creature_type const g_ArchmageOverwatchSpawns[TEAM_NEUTRAL][ArchmageOverwatchCreaturesCount] =
{
    // Alliance
    {
        { StormshieldKnight,   ALLIANCE, AshranMapID, 4235.87f, -4219.78f, 31.6387f, 2.41959f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4244.08f, -4223.27f, 30.4281f, 2.41959f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4242.08f, -4229.02f, 29.7179f, 2.45058f },
        { StormshieldKnight,   ALLIANCE, AshranMapID, 4232.65f, -4183.01f, 31.0318f, 0.78809f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4234.03f, -4187.15f, 30.9069f, 0.78809f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4228.85f, -4181.65f, 31.0318f, 0.78809f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4261.52f, -4171.32f, 31.1670f, 2.14280f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4262.90f, -4167.40f, 31.1766f, 3.10849f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4240.65f, -4140.60f, 32.0990f, 4.85066f },
        { StormshieldVanguard, ALLIANCE, AshranMapID, 4244.48f, -4144.43f, 32.4948f, 3.10849f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4249.71f, -4184.24f, 55.1481f, 2.11727f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4240.87f, -4198.12f, 55.1894f, 3.20496f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4253.66f, -4212.38f, 55.0774f, 4.70262f },
        { StormshieldSentinel, ALLIANCE, AshranMapID, 4265.54f, -4211.42f, 55.1122f, 4.83459f }
    },
    // Horde
    {
        { WarspearRaptorRider, HORDE, AshranMapID, 4235.87f, -4219.78f, 31.6387f, 2.41959f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4244.08f, -4223.27f, 30.4281f, 2.41959f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4242.08f, -4229.02f, 29.7179f, 2.45058f },
        { WarspearRaptorRider, HORDE, AshranMapID, 4232.65f, -4183.01f, 31.0318f, 0.78809f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4234.03f, -4187.15f, 30.9069f, 0.78809f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4228.85f, -4181.65f, 31.0318f, 0.78809f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4261.52f, -4171.32f, 31.1670f, 2.14280f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4262.90f, -4167.40f, 31.1766f, 3.10849f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4240.65f, -4140.60f, 32.0990f, 4.85066f },
        { WarspearBloodGuard,  HORDE, AshranMapID, 4244.48f, -4144.43f, 32.4948f, 3.10849f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4249.71f, -4184.24f, 55.1481f, 2.11727f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4240.87f, -4198.12f, 55.1894f, 3.20496f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4253.66f, -4212.38f, 55.0774f, 4.70262f },
        { WarspearHeadhunter,  HORDE, AshranMapID, 4265.54f, -4211.42f, 55.1122f, 4.83459f }
    }
};

go_type const g_ArchmageOverwatchFires[ArchmageOverwatchObjectsCount] =
{
    { BonfireWithSmokeLarge1, AshranMapID, 4267.42f, -4199.62f, 61.32f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { BonfireWithSmokeLarge1, AshranMapID, 4266.43f, -4184.83f, 54.81f, 0.09f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4269.72f, -4191.55f, 47.15f, 0.41f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4257.58f, -4211.56f, 45.87f, 0.41f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4246.40f, -4191.69f, 46.00f, 0.41f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4234.25f, -4122.79f, 36.43f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4212.66f, -4126.20f, 37.60f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4195.31f, -4146.53f, 31.68f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4195.42f, -4159.43f, 31.68f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4239.21f, -4199.39f, 35.55f, 0.41f, 0.00f, 0.00f, 0.00f, 0.00f },
    { Smallfire1,             AshranMapID, 4220.10f, -4223.96f, 37.71f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f },
    { FXFireMediumLowSlow,    AshranMapID, 4252.13f, -4146.81f, 38.35f, 0.37f, 0.00f, 0.00f, 0.00f, 0.00f }
};

creature_type const g_ArchmageOverwatchNeutral[ArchmageOverwatchCreaturesCount] =
{
    { WarspearGrunt,       HORDE,    AshranMapID, 4271.14f, -4166.63f, 31.3710f, 3.36416f },
    { WarspearGrunt,       HORDE,    AshranMapID, 4269.84f, -4161.86f, 31.1973f, 3.50306f },
    { WarspearGrunt,       HORDE,    AshranMapID, 4246.79f, -4137.83f, 32.9107f, 3.57439f },
    { WarspearGrunt,       HORDE,    AshranMapID, 4247.10f, -4139.70f, 32.9390f, 2.83231f },
    { WarspearGrunt,       HORDE,    AshranMapID, 4232.03f, -4191.03f, 31.0079f, 1.10651f },
    { WarspearGrunt,       HORDE,    AshranMapID, 4205.28f, -4147.64f, 31.6484f, 4.64015f },
    { WarspearGrunt,       HORDE,    AshranMapID, 4202.43f, -4145.92f, 31.6566f, 4.15713f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4260.53f, -4170.63f, 31.1360f, 0.23275f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4259.43f, -4164.99f, 31.0425f, 0.13952f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4245.96f, -4140.50f, 32.8186f, 1.27162f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4244.52f, -4138.88f, 32.6640f, 0.43295f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4238.14f, -4180.38f, 30.9901f, 4.00969f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4196.91f, -4154.23f, 31.6799f, 0.82704f },
    { StormshieldFootman,  ALLIANCE, AshranMapID, 4200.85f, -4157.09f, 31.6799f, 1.12942f }
};

// See order below
go_type const g_CapturePoint[MaxBattleType] =
{
    { CapturePointBanner, AshranMapID, 4801.65f, -4211.40f, 32.9733f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
    { CapturePointBanner, AshranMapID, 4677.77f, -4222.42f, 10.1084f, 0.01f, 0.00f, 0.00f, 0.00f, 0.00f },
    { CapturePointBanner, AshranMapID, 4533.45f, -4211.45f, 7.11222f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
    { CapturePointBanner, AshranMapID, 4353.77f, -4190.81f, 10.0985f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
    { CapturePointBanner, AshranMapID, 4226.71f, -4171.36f, 31.2031f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f }
};

uint32 const g_MiddleBattlesEntries[MaxBattleType] =
{
    WorldStateEmberfallTowerBattle,
    WorldStateVolrathsAdvanceBattle,
    WorldStateTheCrossroadsBattle,
    WorldStateTrembladesVanguardBattle,
    WorldStateArchmageOverwatchBattle
};

uint32 const g_EventWarnTexts[MaxEvents] =
{
    //TrinityStrings::LangSendKorlokTheOgreKingEvent,
    //TrinityStrings::LangAshranReserved1,
    //TrinityStrings::LangAshranReserved2,
    //TrinityStrings::LangAshranReserved3,
    //TrinityStrings::LangSendStadiumRacingEvent
    0,0,0,0,0
};

creature_type const g_Korlok            = { KorlokTheOgreKing, TEAM_OTHER, AshranMapID, 4533.17f, -4446.13f, 28.3867f, 1.56182f };
creature_type const g_AllianceChapion   = { GaulDunFirok, ALLIANCE, AshranMapID, 4510.53f, -4384.28f, 20.6805f, 5.79889f };
creature_type const g_HordeChampion     = { MukmarRaz, HORDE, AshranMapID, 4553.26f, -4382.69f, 20.6805f, 3.38924f };
