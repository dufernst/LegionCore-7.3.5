////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "Cell.h"
#include "CellImpl.h"
#include "CreatureTextMgr.h"
#include "GameObjectAI.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "InstanceScript.h"
#include "MoveSplineInit.h"
#include "ScriptedCosmeticAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "ScriptUtils.h"
#include "Vehicle.h"

#ifndef HIGHMAUL_HPP
#define HIGHMAUL_HPP

Position const g_PlayScenePos = { 3448.069f, 7573.542f, 55.30419f, 0.3921495f };

static void CastSpellToPlayers(Map* map, Unit* caster, uint32 spellID, bool triggered)
{
    if (!map)
        return;
    
    map->ApplyOnEveryPlayer([&](Player* player)
    {
        if (!player)
            return;

        if (caster)
            caster->CastSpell(player, spellID, triggered);
        else
            player->CastSpell(player, spellID, triggered);
    });
}

enum eHighmaulDatas
{
    /// Bosses
    BossKargathBladefist,
    BossTheButcher,
    BossBrackenspore,
    BossTectus,
    BossTwinOgron,
    BossKoragh,
    BossImperatorMargok,
    MaxHighmaulBosses,

    /// Instance datas
    ElevatorActivated = 0,
    TestsActivated,
    KargathAchievement,
    ButcherAchievement,
    TectusAchievement,
    BrackensporeAchievement,
    TwinOgronAchievement,
    KoraghOverflowingEnergy,
    KoraghNullificationBarrier,
    KoraghAchievement,
    ImperatorAchievement,

    /// Misc values
    /// Kargath
    RaidGrate001 = 0,
    RaidGrate002,
    RaidGrate003,
    RaidGrate004,
    MaxRaidGrates,
    HighmaulSweeperCount = 2,
    HighmaulSweeperMovesCount = 9,
    /// The Butcher
    MaxMaggotToKill = 6,
    /// Brackenspore
    MaxCreepingMoss = 16,
    MaxFleshEaterPos = 2,
    BurningInfusionNeeded = 15,
    /// The Market
    MaxTectusGuardians = 3,
    /// Twin Ogron
    DispositionPCT = 30,
    /// Ko'ragh
    MaxOverflowingEnergy = 14,
    /// Imperator Mar'gok
    MaxIntervalles = 2
};

enum eHighmaulCreatures
{
    /// Walled City
    GhargArenaMaster        = 84971,
    GorianEnforcer          = 88724,
    /// The Coliseum
    KargathBladefist        = 78714,
    JhornTheMad             = 83377,
    ThoktarIronskull        = 83378,
    Vulgor                  = 80048,
    BladespireSorcerer      = 80071,
    CrowdAreatrigger        = 79260,
    MargokCosmetic          = 83268,
    IronBomberSpawner       = 79712,
    IronBomber              = 78926,
    DrunkenBileslinger      = 78954,
    HighmaulSweeper         = 88874,
    /// The Underbelly
    TheButcher              = 77404,
    /// Gorian Strands
    IronGrunt               = 88118,
    BlackrockGrunt          = 86610,
    LowBatchDeadPale        = 86283,
    NightTwistedPaleVis     = 82694,
    CosmeticGorianWarr      = 82690,
    GorianCivilian          = 85371,
    Brackenspore            = 78491,
    BFC9000                 = 81403,
    /// The Market
    Tectus                  = 78948,
    Rokka                   = 86071,
    Oro                     = 86072,
    Lokk                    = 86073,
    /// The Gorthenon
    Phemos                  = 78237,
    Pol                     = 78238,
    /// Chamber of Nullification
    Koragh                  = 79015,
    RuneOfNullification     = 79559,
    VolatileAnomaly         = 79956,
    /// Throne of the Imperator
    ImperatorMargok         = 77428,
    HighCouncilorMalgris    = 81811,
    KingPrison              = 89185
};

enum eHighmaulGameobjects
{
    ArenaElevator           = 233098,
    CollisionWall           = 234299,
    InstancePortal2         = 231770,

    /// Kargath's Doors
    GateArenaExit           = 231781,
    GateArenaInner          = 231780,

    /// The Butcher's Doors
    EarthenPillar           = 239110,

    /// Brackenspore's Doors
    FungalGiantDoor         = 239124,
    WindDoor                = 236703,

    /// Kargath Bladefist
    RaidGrate1              = 232368,
    RaidGrate2              = 232369,
    RaidGrate3              = 232370,
    RaidGrate4              = 232371,

    /// Tectus's Doors
    Earthwall1              = 237777,
    Earthwall2              = 237778,
    Earthwall3              = 237779,
    Earthwall4              = 237780,
    HighmaulLFRDoor         = 239126,
    HighmaulLFRDoorColiseum = 239125,

    /// Twin Ogron's Doors
    TwinOgronEntrance       = 236211,
    TwinOgronExit           = 236212,

    /// Ko'ragh's Doors
    FelBreakerEntrance      = 236213,
    FelBreakerExitDoor      = 236214,

    /// Imperator Margokk Door
    ThroneRoomDoor          = 231938,
    StairBlockingDoor       = 236210,

    /// Misc
    Teleporter              = 231776,
    SLGGenericMoPLargeAoI   = 68553
};

enum eHighmaulWorldStates
{
    UnknownHighmaulWorldState   = 8902,
    DisableCrowdSound           = 8903,
    UnknownHighmaulWorldState2  = 9118,
    IronBomberEnable            = 9722,
    IronBomberRemaining         = 9723,
    DrunkenBileslingerEnable    = 9724,
    DrunkenBileslingerRemaining = 9725
};

enum eHighmaulSpells
{
    PlayChogallScene    = 178333,
    ChogallNight        = 163661,
    Berserker           = 26662
};

enum eHighmaulLocs
{
    BeachEntrance       = 4780,
    ExitTarget          = 4782,
    ArenaCenter         = 4783,
    KargathDefeated     = 4784,
    PalaceFrontGate     = 4785,
    FelBreakerRoom      = 4786,
    ImperatorsRise      = 4787,
    CityBaseTeleporter  = 4788
};

enum eHighmaulDungeons
{
    WalledCity      = 849,
    ArcaneSanctum   = 850,
    ImperatorsFall  = 851
};

enum eHighmaulAchievements
{
    FlameOn             = 8948,
    HurryUpMaggot       = 8947,
    MoreLikeWreckedUs   = 8974,
    AFungusAmongUs      = 8975,
    BrothersInArms      = 8958,
    PairAnnihilation    = 8976,
    LineageOfPower      = 8977,
    AheadOfTheCurve     = 9441,
    CuttingEdge         = 9442
};

#define hScriptName "instance_highmaul"

template<typename AI>
inline AI* GetHighmaulAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, hScriptName);
}

#define RegisterHighmaulCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetHighmaulAI)

#endif
