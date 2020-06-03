
//Throne of Thunder

#ifndef THRONEOFTHUNDER
#define THRONEOFTHUNDER

enum eData
{
    DATA_STORM_CALLER           = 1, //Mini Boss
    DATA_JINROKH                = 2,
    DATA_STORMBRINGER           = 3, //Mini Boss
    DATA_HORRIDON               = 4,
    DATA_COUNCIL_OF_ELDERS      = 5,
    DATA_TORTOS                 = 6,
    DATA_MEGAERA                = 7,
    DATA_JI_KUN                 = 8,
    DATA_DURUMU                 = 9,
    DATA_PRIMORDIUS             = 10,
    DATA_DARK_ANIMUS            = 11,
    DATA_IRON_QON               = 12,
    DATA_TWIN_CONSORTS          = 13,
    DATA_LEI_SHEN               = 14,
    DATA_RA_DEN                 = 15,

    DATA_RESET_MOGU_FONTS,
    DATA_CHECK_VALIDATE_THUNDERING_THROW,
    DATA_GET_PHASE,
    DATA_SEND_DEST_POS,
    DATA_CHECK_COUNCIL_PROGRESS,
    DATA_GET_NEXT_HEAD,
    DATA_CHECK_PROGRESS_MEGAERA,
    DATA_GET_COUNT_RANGE_HEADS,
    DATA_SEND_LAST_DIED_HEAD,
    DATA_GET_ELEMENTAL_BLOOD_OF_MEGAERA,
    DATA_ACTIVE_NEXT_NEST,
    DATA_UPDATE_MOD_TIMER,
    DATA_SPAWN_NEW_HEAD,
    DATA_ACTIVE_NEST,
    DATA_RESET_NEST,
    DATA_JIKUN_RESET_ALL_NESTS,
    DATA_TAKEOFF,
    DATA_MORPH,
    DATA_LAUNCH_FEED_NEST,
    DATA_LAUNCH_FEED_PLATFORM,
    DATA_ENTERCOMBAT,
    DATA_IS_CONE_TARGET_CREATURE,
    DATA_DESPAWN_CREATURE_CONE_TARGET,
    DATA_CLEAR_CRIMSON_FOG_LIST,
    DATA_GET_DURUMU_ROTATE_DIRECTION,
    DATA_CREATE_MIST,
    DATA_CLOSE_SAFE_ZONE_IN_MIST,
    DATA_MUTATE,
    DATA_REMOVE_MUTATE,
};

enum esAction
{
    ACTION_MEGAERA_IN_PROGRESS = 50,
    ACTION_MEGAERA_RESET,
    ACTION_MEGAERA_DONE,
    ACTION_UNSUMMON,
    ACTION_MEGAERA_RAMPAGE,
    ACTION_RESET_EVENTS,
    ACTION_RESTART_EVENTS,
    ACTION_PREPARE_TO_UNSUMMON ,
    ACTION_COLORBLIND_PHASE_DONE,
};

Position const megaeraspawnpos[6] =
{
    { 6438.71f, 4533.36f, -209.609f, 2.7672f }, //left
    { 6419.33f, 4504.38f, -209.609f, 2.3032f }, //right
    { 6469.97f, 4483.59f, -209.609f, 2.4602f }, //center r far
    { 6487.91f, 4502.82f, -209.609f, 2.4602f }, //center l far
    { 6437.48f, 4564.84f, -209.609f, 3.6468f }, //left++
    { 6394.25f, 4493.86f, -209.609f, 1.5930f }, //right++
};

Position const megaerarangespawnpos[8] =
{
    { 6469.97f, 4483.59f, -209.609f, 2.4602f },
    { 6487.91f, 4502.82f, -209.609f, 2.4602f },
    { 6446.96f, 4471.15f, -209.609f, 2.3289f },
    { 6427.34f, 4459.08f, -209.609f, 1.9951f },
    { 6403.90f, 4457.46f, -209.609f, 1.5467f },
    { 6491.58f, 4520.66f, -209.609f, 2.9651f },
    { 6492.65f, 4546.71f, -209.609f, 3.1222f },
    { 6487.61f, 4577.31f, -209.609f, 3.4254f },
};

enum eCreatures
{
    //Minibosses
    NPC_STORM_CALLER            = 70236,
    NPC_STORMBRINGER            = 70445,

    //Npc
    NPC_CONDUCTIVE_WATER        = 69469,
    NPC_MOGU_FONT               = 90005, //new trigger, need update in DB
    NPC_STORM_STALKER           = 69676,
    NPC_LIGHTNING_BALL          = 69232,
    NPC_LIGHTNING_FISSURE       = 69609,
    NPC_WHIRL_TURTLE            = 67966,
    NPC_VAMPIRIC_CAVE_BAT       = 69352,
    NPC_CINDERS                 = 70432,
    NPC_LIVING_FLUID            = 69069,
    NPC_AT_CASTER_STALKER       = 69081,
    NPC_ANIMA_GOLEM             = 69701,
    NPC_LARGE_ANIMA_GOLEM       = 69700,
    NPC_MASSIVE_ANIMA_GOLEM     = 69699,
    NPC_BEAST_OF_NIGHTMARES     = 69479,
    NPC_CORRUPTED_ANIMA         = 69957,
    NPC_CORRUPTED_VITA          = 69958,
    NPC_SAND_TRAP               = 69346,
    NPC_LIVING_POISON           = 69313,
    NPC_FROZEN_ORB              = 69268,
    NPC_LIGHTNING_NOVA_TOTEM    = 69215,
    NPC_LIVING_SAND             = 69153,
    NPC_GARAJAL_SOUL            = 69182,
    NPC_ROCKFALL                = 68219,
    NPC_ACID_RAIN               = 70435,
    NPC_TORRENT_OF_ICE          = 70439,
    NPC_ICY_GROUND              = 70446,
    NPC_JUMP_TO_BOSS_PLATFORM   = 69885,

    NPC_BLESSED_LOA_SPIRIT      = 69480,
    NPC_BLESSED_LOA_SPIRIT_2    = 69491,
    NPC_BLESSED_LOA_SPIRIT_3    = 69492,

    NPC_DARK_LOA_SPIRIT         = 69548,
    NPC_DARK_LOA_SPIRIT_2       = 69553,
    NPC_DARK_LOA_SPIRIT_3       = 69556,

    //Horridon adds
    //Farrak Gate
    //Big
    NPC_FARRAKI_WASTEWALKER     = 69175,
    NPC_FARRAKI_SKIRMISHER      = 69173,
    //Small
    NPC_SULLITHUZ_STONEGAZER    = 69172,
    //Gurubashi
    //Big
    NPC_GURUBASHI_VENOM_PRIEST  = 69164,
    NPC_VENOMOUS_EFFUSION       = 69314,
    //Small
    NPC_GURUBASHI_BLOODLORD     = 69167,
    //Drakkari
    //Big
    NPC_DRAKKARI_FROZEN_WARLORD = 69178,
    //Small
    NPC_RISEN_DRAKKARI_CHAMPION = 69185,
    NPC_RISEN_DRAKKARI_WARRIOR  = 69184,
    //Amani
    //Big
    NPC_AMANI_WARBEAR           = 69177,
    NPC_AMANISHI_BEAST_SHAMAN   = 69176,
    //Small
    NPC_AMANISHI_FLAME_CASTER   = 69168,
    NPC_AMANISHI_PROTECTOR      = 69169,
    //
    NPC_ZANDALARI_DINOMANCER    = 69221,
    NPC_H_GATE_CONTROLLER       = 90010,

    //Bosses
    NPC_JINROKH                 = 69465,
    NPC_HORRIDON                = 68476,
    NPC_JALAK                   = 69374,
    //Council of Elders
    NPC_FROST_KING_MALAKK       = 69131,
    NPC_PRINCESS_MARLI          = 69132,
    NPC_KAZRAJIN                = 69134,
    NPC_SUL_SANDCRAWLER         = 69078,
    //
    NPC_TORTOS                  = 67977,
    //Megaera
    NPC_MEGAERA                 = 68065,
    NPC_FLAMING_HEAD_MELEE      = 70212,
    NPC_FLAMING_HEAD_RANGE      = 70229,
    NPC_VENOMOUS_HEAD_MELEE     = 70247,
    NPC_VENOMOUS_HEAD_RANGE     = 70251,
    NPC_FROZEN_HEAD_MELEE       = 70235,
    NPC_FROZEN_HEAD_RANGE       = 70250,
    //
    //Jikun
    NPC_JI_KUN                  = 69712,
    //Eggs
    NPC_YOUNG_EGG_OF_JIKUN      = 68194,
    NPC_MATURE_EGG_OF_JIKUN     = 69628,
    NPC_JIKUN_FLEDGLING_EGG     = 68202,
    //
    NPC_HATCHLING               = 68192,
    NPC_JUVENILE                = 69836,
    NPC_JUVENILE_FROM_F_EGG     = 70095,

    NPC_INCUBATER               = 69626,
    NPC_FEED                    = 68178,
    NPC_FEED_NEST_POOL          = 70216, //nest pool
    NPC_FEED_P_POOL             = 68188, //platform pool
    NPC_FALL_CATCHER            = 69839,
    //
    NPC_DURUMU                  = 68036,
    NPC_APPRAISING_EYE          = 67858,
    NPC_MIND_EYE                = 67875,

    NPC_RED_EYE                 = 67855,
    NPC_RED_EYEBEAM_TARGET      = 67851,
    NPC_CRIMSON_FOG             = 69050,

    NPC_BLUE_EYE                = 67854,
    NPC_BLUE_EYEBEAM_TARGET     = 67829,
    NPC_AZURE_FOG               = 69052,

    NPC_YELLOW_EYE              = 67856,
    NPC_YELLOW_EYEBEAM_TARGET   = 67852,

    NPC_CROSS_EYE               = 67857, //cast 134755
    NPC_HUNGRY_EYE              = 67859, //cast 137727, 133796
    NPC_EYEBEAM_TARGET_DURUMU   = 67882,

    NPC_PRIMORDIUS              = 69017,
    NPC_DARK_ANIMUS             = 69427,
    NPC_IRON_QON                = 68078,
    //Iron Qon Maunts
    NPC_ROSHAK                  = 68079, //Fire
    NPC_QUETZAL                 = 68080, //Storm
    NPC_DAMREN                  = 68081, //Frozen
    //Twin consorts
    NPC_SULIN                   = 68904,
    NPC_LULIN                   = 68905,
    //
    NPC_LEI_SHEN                = 68397,
    NPC_RA_DEN                  = 69473,
};

enum esSpell
{
    SPELL_SHADO_PAN_ONSLAUGHT     = 149070,
    SPELL_HYDRA_FRENZY            = 139942,
    SPELL_TORRENT_OF_ICE_T        = 139857,
    SPELL_JIKUN_FLY               = 140013,
    SPELL_INCUBATE_ZONE           = 137526,
    SPELL_JUMPS_DOWN_TO_HATCHLING = 138904,
    SPELL_JUMP_DOWN_TO_PLATFORM   = 140575,
    SPELL_SLIMED_AT               = 134255, //feed fly AT (catch feed)
    SPELL_FEED_FALL_VISUAL        = 140788, //visual feed, then fly
};

enum eGameObjects
{  
    //Jinrokh
    GO_JINROKH_PRE_DOOR     = 218665,
    GO_JINROKH_ENT_DOOR     = 218664,
    GO_MOGU_SR              = 218675,
    GO_MOGU_NR              = 218676,
    GO_MOGU_NL              = 218677,
    GO_MOGU_SL              = 218678,
    GO_JINROKH_EX_DOOR      = 218663,
    //Horridon
    GO_HORRIDON_PRE_DOOR    = 218669,
    GO_HORRIDON_ENT_DOOR    = 218667,
    //Event door
    GO_MAIN_GATE            = 218674,
    //Add gates
    GO_FARRAK_GATE          = 218672,
    GO_GURUBASHI_GATE       = 218670,
    GO_DRAKKARI_GATE        = 218671,
    GO_AMANI_GATE           = 218673,

    GO_ORB_OF_CONTROL       = 218374,
    //
    GO_HORRIDON_EX_DOOR     = 218666,
    //Council of Elders
    GO_COUNCIL_LENT_DOOR    = 218655,
    GO_COUNCIL_RENT_DOOR    = 218656,
    GO_COUNCIL_EX_DOOR      = 218657,
    GO_COUNCIL_EX2_DOOR     = 218469,
    //Tortos
    GO_TORTOS_EX_DOOR       = 218980,
    GO_TORTOS_EX2_DOOR      = 218987,
    //Megaera
    GO_MEGAERA_EX_DOOR      = 218746,
    //Ji Kun
    GO_JI_KUN_FEATHER       = 218543,
    GO_JI_KUN_EX_DOOR       = 218888,
    GO_JIKUN_EGG            = 218382,
    //Durumu
    GO_DURUMU_EX_DOOR       = 218390,
    GO_THUNDER_KING_SMALL   = 218395,
    GO_THUNDER_KING_LARGE   = 218396,
    //Primordius
    GO_PRIMORDIUS_ENT_DOOR  = 218584,
    GO_PRIMORDIUS_EX_DOOR   = 218585,
    //Secret Ra Den door
    GO_S_RA_DEN_ENT_DOOR    = 218553,
    //Dark Animus
    GO_DARK_ANIMUS_ENT_DOOR = 218392,
    GO_DARK_ANIMUS_EX_DOOR  = 218393,
    //Iron Qon
    GO_IRON_QON_ENT_DOOR    = 218388,
    GO_IRON_QON_EX_DOOR     = 218588,
    //Twin Consorts
    GO_TWIN_ENT_DOOR        = 218781,
    GO_TWIN_FENCE_DOOR      = 218711,
    GO_TWIN_FENCE_DOOR_2    = 218712,
    GO_TWIN_EX_DOOR         = 218394,
    //Lei Shen
    GO_TP_PLATFORM          = 218417,
    GO_TP_TO_RA_DEN         = 218418,
    GO_CHARGING_STATION     = 218397,
    //Ra Den
    GO_RA_DEN_ENT_DOOR      = 218555,
};

#endif THRONEOFTHUNDER
