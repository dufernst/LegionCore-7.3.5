#ifndef DEF_DRAGONSOUL_H
#define DEF_DRAGONSOUL_H
// 109247 - ds nerf real
// 109251 - ds nerf dummy
// 109255 - ds nerf dummy
// 108202 - temple teleport
// 108263 - ship teleport
// 106094 - eye teleport +
// 109835 - summit teleport +
// 106093 - summit teleport aoe
// 106092 - temple teleport aoe

enum Datas
{
    DATA_MORCHOK            = 0,
    DATA_YORSAHJ            = 1,
    DATA_ZONOZZ             = 2,
    DATA_HAGARA             = 3,
    DATA_ULTRAXION          = 4,
    DATA_BLACKHORN          = 5,
    DATA_SPINE              = 6,
    DATA_MADNESS            = 7,
    DATA_DEATHWING          = 8,
    DATA_KOHCROM            = 10,
    DATA_HAGARA_EVENT       = 11,
    DATA_LESSER_CACHE_10N   = 12,
    DATA_LESSER_CACHE_25N   = 13,
    DATA_LESSER_CACHE_10H   = 14,
    DATA_LESSER_CACHE_25H   = 15,
    DATA_SWAYZE             = 16,
    DATA_HORDE_SHIP         = 17,
    DATA_ALLIANCE_SHIP      = 18,
    DATA_BACK_PLATE_1       = 19,
    DATA_BACK_PLATE_2       = 20,
    DATA_BACK_PLATE_3       = 21,
    DATA_GREATER_CACHE_10N  = 22,
    DATA_GREATER_CACHE_25N  = 23,
    DATA_GREATER_CACHE_10H  = 24,
    DATA_GREATER_CACHE_25H  = 25,
    DATA_ALEXSTRASZA_DRAGON = 26,
    DATA_NOZDORMU_DRAGON    = 27,
    DATA_YSERA_DRAGON       = 28,
    DATA_KALECGOS_DRAGON    = 29,
    DATA_ELEM_FRAGMENT_10N  = 30,
    DATA_ELEM_FRAGMENT_25N  = 31,
    DATA_ELEM_FRAGMENT_10H  = 32,
    DATA_ELEM_FRAGMENT_25H  = 33,
    DATA_THRALL_MADNESS     = 34,

    DATA_ALLIANCE_SHIP_FIRST  = 93,
    DATA_OPEN_PORTAL_TO_EYE   = 94,
    DATA_SPAWN_GREATER_CHEST  = 95,
    DATA_NEXT_ASSAULTER       = 96,
    DATA_DRAGONS_COUNT        = 97,
    DATA_DRAGON_SOUL_EVENT    = 98,
    DATA_ULTRAXION_TRASH      = 99,    
    DATA_IS_LFR               = 100,
    DATA_IS_FALL_OF_DEATHWING = 101,
};

enum ActionsDS
{
    ACTION_ULTRAXION_WIN            = 1,
    ACTION_AFTER_HAGARA             = 2,
    ACTION_SPAWN_DRAGONS            = 3,
    ACTION_DEATHWING_INTRO          = 4,
    ACTION_DEATHWING_RESET          = 5,
    ACTION_ALEXSTRASZA              = 6,
    ACTION_START_ULTRAXION          = 7,
    ACTION_LOAD_EVENT               = 8,
    ACTION_LOAD_TRASH               = 9,
    ACTION_STOP_ASSAULTERS_SPAWN    = 10,
    ACTION_START_ASSAULT            = 11,
    ACTION_STOP_ASSAULT             = 12,
};

enum SayDS
{
    SAY_YSERA_PORTAL        = 0,
    SAY_YSERA_START         = 1,
    SAY_YSERA_EVENT         = 2,
    SAY_YSERA_HELP          = 3,
    SAY_YSERA_WIPE          = 4,

    SAY_NOZDORMU_EVENT      = 0,
    SAY_NOZDORMU_HELP       = 1,

    SAY_KAKECGOS_PORTAL_1   = 0,
    SAY_KAKECGOS_PORTAL_2   = 1,
    SAY_KAKECGOS_PORTAL_3   = 2,
    SAY_KALECGOS_EVENT      = 3,
    SAY_KALECGOS_HELP       = 4,

    SAY_ALEXSTRASZA_PORTAL  = 0,
    SAY_ALEXSTRASZA_EVENT_1 = 1,
    SAY_ALEXSTRASZA_EVENT_2 = 2,
    SAY_ALEXSTRASZA_HELP    = 3,
    SAY_ALEXSTRASZA_WIPE    = 4,
    SAY_ALEXSTRASZA_WIN     = 5,

    SAY_THRALL_PORTAL_1     = 0,
    SAY_THRALL_PORTAL_2     = 1,
    SAY_THRALL_PORTAL_3     = 2,
    SAY_THRALL_EVENT_1      = 3,
    SAY_THRALL_EVENT_2      = 4,
    SAY_THRALL_EVENT_3      = 5,
    SAY_THRALL_START        = 6,
    SAY_THRALL_WIN          = 7,

    SAY_DEATHWING_INTRO_1   = 0,
    SAY_DEATHWING_INTRO_2   = 1,
    SAY_DEATHWING_INTRO_3   = 2,
    SAY_DEATHWING_INTRO_4   = 3,
    SAY_DEATHWING_INTRO_5   = 4,
};

enum CreatureIds
{
    NPC_MORCHOK                     = 55265,
    NPC_KOHCROM                     = 57773,
    NPC_YORSAHJ                     = 55312,
    NPC_ZONOZZ                      = 55308,
    NPC_HAGARA                      = 55689,
    NPC_ULTRAXION                   = 55294,
    NPC_BLACKHORN                   = 56427,
    NPC_GORIONA                     = 56781,
    NPC_SKY_CAPTAIN_SWAYZE          = 55870,
    NPC_KAANU_REEVS                 = 55891,
    NPC_SPINE_OF_DEATHWING          = 53879, // 109983 105003 109035 95278 105036
    NPC_DEATHWING                   = 56173, // at the eye

    NPC_NETHESTRASZ                 = 57287, // teleport upstairs
    NPC_EIENDORMI                   = 57288, // teleport to Yor'sahj
    NPC_VALEERA                     = 57289, // teleport to Zon'ozz
    NPC_TRAVEL_TO_WYRMREST_TEMPLE   = 57328, //
    NPC_TRAVEL_TO_WYRMREST_BASE     = 57882, //
    NPC_TRAVEL_TO_WYRMREST_SUMMIT   = 57379, //
    NPC_TRAVEL_TO_EYE_OF_ETERNITY   = 57377, // teleport to Hagara
    NPC_TRAVEL_TO_MAELSTORM         = 57443, //
    NPC_TRAVEL_TO_DECK              = 57378, //
    NPC_DASNURIMI                   = 58153, // trader
    NPC_YSERA_THE_AWAKENED          = 56665,
    NPC_ALEXSTRASZA_THE_LIFE_BINDER = 56630,
    NPC_KALECGOS                    = 56664,
    NPC_THRALL_1                    = 56667, // near summit
    NPC_NOZDORMU_THE_TIMELESS_ONE   = 56666,
    NPC_THE_DRAGON_SOUL             = 56668, // near summit
    NPC_THRALL_ON_SHIP              = 57266,

    NPC_THRALL_2                    = 56103, // after spine
    NPC_THE_DRAGON_SOUL_2           = 56694, // after spine

    NPC_ALEXSTRASZA_DRAGON          = 56099, // 1
    NPC_NOZDORMU_DRAGON             = 56102, // 2
    NPC_YSERA_DRAGON                = 56100, // 3
    NPC_KALECGOS_DRAGON             = 56101, // 4

    NPC_AGGRA                       = 58211, // after madness
    NPC_THRALL_3                    = 58232, // after madness
    NPC_KALECGOS_2                  = 58210, // after madness
    NPC_NOZDORMU_2                  = 58208, // after madness
    NPC_ALEXSTRASZA_2               = 58207, // after madness
    NPC_YSERA_2                     = 58209, // after madness

    // ultraxion event
    NPC_DEATHWING_EVENT             = 55971,
    NPC_TWILIGHT_ASSAULTER_1        = 56249,
    NPC_TWILIGHT_ASSAULTER_2        = 56250,
    NPC_TWILIGHT_ASSAULTER_3        = 56251,
    NPC_TWILIGHT_ASSAULTER_4        = 56252,
    NPC_TWILIGHT_ASSAULTER_STALKER  = 57281,

    // zonozz trash
    NPC_FLAIL_OF_GORATH_TRASH       = 57877,
    NPC_CLAW_OF_GORATH_TRASH        = 57890,
    NPC_EYE_OF_GORATH_TRASH         = 57875,
    NPC_TENTACLE_TOSS_STALKER       = 57836,

    // yorsahj trash
    NPC_CRIMSON_GLOBULE_TRASH       = 57386,
    NPC_ACIDIC_GLOBULE_TRASH        = 57333,
    NPC_GLOWING_GLOBULE_TRASH       = 57387,
    NPC_DARK_GLOBULE_TRASH          = 57382,
    NPC_SHADOWED_GLOBULE_TRASH      = 57388,
    NPC_COBALT_GLOBULE_TRASH        = 57384,
};

enum GameObjects
{
    GO_INNER_WALL                       = 209596,
    GO_THE_FOCUSING_IRIS                = 210132,
    GO_LESSER_CACHE_OF_THE_ASPECTS_LFR  = 210221, // ultraxion
    GO_LESSER_CACHE_OF_THE_ASPECTS_10N  = 210160,
    GO_LESSER_CACHE_OF_THE_ASPECTS_25N  = 210161,
    GO_LESSER_CACHE_OF_THE_ASPECTS_10H  = 210162,
    GO_LESSER_CACHE_OF_THE_ASPECTS_25H  = 210163,
    GO_ALLIANCE_SHIP                    = 210210,
    GO_ALLIANCE_SHIP_FIRST              = 210211,
    GO_HORDE_SHIP                       = 210061,
    GO_DEATHWING_BACK_PLATE_1           = 209623,
    GO_DEATHWING_BACK_PLATE_2           = 209631,
    GO_DEATHWING_BACK_PLATE_3           = 209632,
    GO_GREATER_CACHE_OF_THE_ASPECTS_LFR = 210222, // spine of deathwing
    GO_GREATER_CACHE_OF_THE_ASPECTS_10N = 209894,
    GO_GREATER_CACHE_OF_THE_ASPECTS_25N = 209895,
    GO_GREATER_CACHE_OF_THE_ASPECTS_10H = 209896,
    GO_GREATER_CACHE_OF_THE_ASPECTS_25H = 209897,
    GO_ELEMENTIUM_FRAGMENT_LFR          = 210079, // madness of deathwing
    GO_ELEMENTIUM_FRAGMENT_10N          = 210217,
    GO_ELEMENTIUM_FRAGMENT_25N          = 210218,
    GO_ELEMENTIUM_FRAGMENT_10H          = 210219,
    GO_ELEMENTIUM_FRAGMENT_25H          = 210220,
};

enum SharedSpells
{
    SPELL_TELEPORT_VISUAL_ACTIVE                = 108203,
    SPELL_TELEPORT_VISUAL_DISABLED              = 108227,
    
    SPELL_CHARGING_UP_LIFE                      = 108490,
    SPELL_CHARGING_UP_MAGIC                     = 108491,
    SPELL_CHARGING_UP_EARTH                     = 108492,
    SPELL_CHARGING_UP_TIME                      = 108493,
    SPELL_CHARGING_UP_DREAMS                    = 108494,
    SPELL_WARD_OF_TIME                          = 108160,
    SPELL_WARD_OF_EARTH                         = 108161,
    SPELL_WARD_OF_MAGIC                         = 108162,
    SPELL_WARD_OF_LIFE                          = 108163,
    SPELL_WARD_OF_DREAMS                        = 108164,

    SPELL_TELEPORT_SINGLE_TO_DEATHWINGS_BACK    = 106054,
    SPELL_DRAGON_SOUL_PARATROOPER_KIT_1         = 104953, // Swayze has it while jumping to spine of deathwing
    SPELL_DRAGON_SOUL_PARATROOPER_KIT_2         = 105008, // Reevs has it while jumping to spine of deathwing

    SPELL_PARACHUTE                             = 110660, // used by players

    SPELL_PLAY_MOVIE_DEATHWING_2                = 106085, // movie before jumping at spine of deathwing

    SPELL_CALM_MAELSTROM_SKYBOX                 = 109480,

    // Ultraxion Trash
    SPELL_TEMPERAMENT                           = 98958,
    SPELL_TWILIGHT_SPAWN                        = 109684,
    SPELL_TWILIGHT_ESCAPE                       = 109904,
    SPELL_DRAGON_SOUL_COSMETIC                  = 108544,
    SPELL_DRAGON_SOUL_COSMETIC_CHARGED          = 110489,
};

enum AchievementCriteriaIds
{
    // Morchok
    CRITERIA_STATS_MORCHOK_KILLS_LFR    = 18687,
    CRITARIA_DONT_STAND_SO_CLOSE_TO_ME  = 18607,

    // Warlord Zon'ozz
    CRITERIA_STATS_ZONOZZ_KILLS_LFR     = 18688,
    CRITERIA_PING_PONG_CHAMPION         = 18494,

    // Yor'sahj the Unsleeping
    CRITERIA_STATS_YORSAHJ_KILLS_LFR    = 18689,
    CRITERIA_TASTE_THE_RAINBOW_BY       = 18495,
    CRITERIA_TASTE_THE_RAINBOW_RG       = 18496,
    CRITERIA_TASTE_THE_RAINBOW_BB       = 18497,
    CRITERIA_TASTE_THE_RAINBOW_PY       = 18498,

    // Hagara the Stormbinder
    CRITERIA_STATS_HAGARA_KILLS_LFR     = 18690,
    CRITERIA_HOLDING_HANDS              = 18608,

    // Ultraxion
    CRITERIA_STATS_ULTRAXION_KILLS_LFR  = 18691,
    CRITERIA_MINUTES_TO_MIDNIGHT        = 18391,

    // Warmaster Blackhorn
    CRITERIA_STATS_WARMASTER_KILLS_LFR  = 18692,
    CRITERIA_DECK_DEFENDER              = 18444,

    // Spine of Deathwing
    CRITERIA_STATS_SPINE_KILLS_LFR      = 18693,
    CRITERIA_MAYBE_HELL_GET_DIZZY       = 18502,

    // Madness of Deathwing
    CRITERIA_DESTROYERS_END             = 18652,
    CRITERIA_STATS_DEATHWING_KILLS_LFR  = 18694,
    CRITERIA_ALEXSTRASZA_FIRST          = 18658,
    CRITERIA_KALECGOS_FIRST             = 18659,
    CRITERIA_NOZDORMU_FIRST             = 18660,
    CRITERIA_YSERA_FIRST                = 18661,
};

const Position portalsPos[11] =
{
    {-1810.5516f, -2396.0664f,  45.6659f, 0.08f},  // Wyrmrest Base
    {-1788.1086f, -2374.2214f,  45.6464f, 4.78f},  // Wyrmrest Temple from Zon'ozz
    {-1784.1575f, -2413.6674f,  45.6492f, 1.65f},  // Wyrmrest Temple from Yor'sahj
    {-1743.6478f, -1835.1325f, -220.509f, 4.53f},  // To Warlord Zon'ozz
    {-1854.2331f, -3068.6586f, -178.339f, 0.46f},  // To Yor'sahj The Unsleeping
    {-1781.1884f, -2375.1225f,  341.350f, 4.43f},  // To Wyrmrest Summit
    { 13629.356f,  13612.099f,  123.485f, 3.14f},  // To Hagara
    {-1811.4489f, -2406.3552f,  340.790f, 0.42f},  // From Hagara
    { 13444.900f, -12133.299f,  151.136f, 6.23f},  // To Skyfire
    {-12109.200f,  12165.965f, -2.73490f, 6.03f},  // To Maelstorm
    {-1768.1762f, -2391.7094f,  45.6463f, 3.21f},  // From Maelstorm to Wyrmrest Temple
};

const Position ultraxionPos[2] = 
{
    {-1564.0000f, -2369.0000f, 250.0830f, 3.28f},  // spawn
    {-1699.4699f, -2388.0300f, 355.1929f, 3.21f},  // move to
};

const Position customPos[6] =
{
    { 13444.900f, -12133.299f, 151.136f, 6.23f},  // skyfirePos
    {-1695.6000f, -2353.7200f, 339.810f, 4.69f},  // swayzePos
    {-1692.0000f, -2353.5100f, 339.810f, 4.66f},  // reevsPos
    {-13854.804f, -13668.660f, 297.378f, 1.52f},  // spinedeathwingPos
    {-12109.200f,  12165.965f, 30.6000f, 6.03f},  // madnessdeathwingPos
    {-1786.7100f, -2393.2299f, 343.602f},         // summitCenterPos
};

const Position aspectsMadness[4] =
{
    {-11927.923f, 12222.394f, 65.3500f, 4.94f},  // Alextrasza
    {-12035.129f, 12222.428f, 65.3500f, 5.32f},  // Nozdormu
    {-12097.647f, 12158.226f, 65.3500f, 5.81f},  // Ysera
    {-12083.188f, 12057.625f, 65.3500f, 5.83f},  // Kakecgos
};

class instance_dragon_soul_trash_accessor
{
public:
    virtual Creature* GetNextTwilightAssaulterStalker(Creature const* current) = 0;
    virtual Position const* GetRandomTwilightAssaulterAssaultPosition(bool horizonal, bool fromEnd, uint8& lane, ObjectGuid& targetGUID) = 0;
    virtual void FreeTwilightAssaulterAssaultLane(bool horizontal, uint8 lane) = 0;
    virtual void CleanTwilightAssaulterAssaultLane(bool horizontal, uint8 lane) = 0;
};

template<class AI>
CreatureAI* GetInstanceAI(Creature* creature)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId("instance_dragon_soul"))
                return new AI(creature);
    return NULL;
}

#endif
