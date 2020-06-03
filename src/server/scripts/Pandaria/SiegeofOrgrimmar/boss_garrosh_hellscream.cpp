/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "siege_of_orgrimmar.h"

enum eSpells
{
    //Garrosh
    SPELL_HELLSCREAM_WARSONG         = 144821,
    SPELL_DESECRETE                  = 144748,
    SPELL_EM_DESECRETE               = 144749,
    SPELL_DESECRATED                 = 144762,
    SPELL_EM_DESECRATED              = 144817,
    SPELL_DESECRATED_WEAPON_AT       = 144760,
    SPELL_EM_DESECRATED_WEAPON_AT    = 144818,
    SPELL_DESECRATED_WEAPON_AXE      = 145880,
    SPELL_WHIRLING_CORRUPTION        = 144985,
    SPELL_EM_WHIRLING_CORRUPTION     = 145037,
    SPELL_EM_WHIRLING_CORRUPTION_S   = 145023,
    SPELL_GRIPPING_DESPAIR           = 145183,
    SPELL_EM_GRIPPING_DESPAIR        = 145195,
    SPELL_PL_TOUCH_OF_YSHAARJ        = 145599,
    SPELL_EXPLOSIVE_DESPAIR_EXPLOSE  = 145199,
    SPELL_EXPLOSIVE_DESPAIR_DOT      = 145213,
    //HM
    SPELL_GARROSH_ENERGY_4           = 146999,
    SPELL_CALL_BOMBARTMENT           = 147120,
    SPELL_BOMBARTMENT                = 147133,
    SPELL_BOMBARTMENT_TR_M           = 147140,
    SPELL_BOMBARTMENT_AURA           = 147122,
    SPELL_BOMBARTMENT_AT             = 147135,
    SPELL_COSMETIC_MISSILE           = 148364,
    SPELL_MANIFEST_RAGE              = 147011,
    SPELL_MALICE                     = 147209,
    SPELL_MALICIOUS_BLAST            = 147235,
    SPELL_MALICIOUS_ENERGY           = 147236,
    SPELL_MALICIOUS_ENERGY_EXPLOSION = 147733,
    //Garrosh Special
    SPELL_TRANSITION_VISUAL_PHASE_2  = 144852,
    SPELL_TRANSITION_VISUAL_BASE     = 146756,
    SPELL_TRANSITION_VISUAL_ADVANCE  = 146845,
    SPELL_TRANSITION_VISUAL_PHASE_3  = 145222,
    SPELL_PHASE_TWO_TRANSFORM        = 144842,
    SPELL_PHASE_THREE_TRANSFORM      = 145246,
    //Embodied despair
    SPELL_CONSUMED_HOPE              = 149032,
    SPELL_HOPE_AT                    = 149003,
    SPELL_HOPE_BUFF                  = 149004,
    SPELL_EMBODIED_DESPAIR           = 145276,
    SPELL_COURAGE                    = 148983,

    //Iron Star
    SPELL_IRON_STAR_IMPACT_AT        = 144645,
    SPELL_IRON_STAR_IMPACT_DMG       = 144650,
    SPELL_EXPLODING_IRON_STAR        = 144798,
    //HM
    SPELL_FIRE_UNSTABLE_IRON_STAR    = 147047, //spawn
    SPELL_IRON_STAR_IMPACT_AT_HM     = 149468,
    SPELL_FIXATE_IRON_STAR           = 147665,
    SPELL_UNSTABLE_IRON_STAR_DMG     = 147173,
    SPELL_UNSTABLE_IRON_STAR_STUN    = 147177,
    SPELL_UNSTABLE_IRON_STAR_DUMMY   = 148299,

    //Engeneer
    SPELL_POWER_IRON_STAR            = 144616,

    //Warbringer
    SPELL_HAMSTRING                  = 144582,
    //HM
    SPELL_BLOOD_FRENZIED             = 147300,

    //Wolf Rider
    SPELL_ANCESTRAL_FURY             = 144585,
    SPELL_FURY                       = 144588,
    SPELL_ANCESTRAL_CHAIN_HEAL       = 144583,
    SPELL_CHAIN_LIGHTNING            = 144584,
    //Embodied doubt
    SPELL_EMBODIED_DOUBT             = 145275,
    SPELL_EMBODIED_DOUBT_HM          = 149347,
    //Minion of Yshaarj
    SPELL_EMPOWERED                  = 145050,
    SPELL_EMPOWERED_HM               = 148714,

    //Realm of Yshaarj
    SPELL_GARROSH_ENERGY             = 145801,
    SPELL_REMOVE_REALM_OF_YSHAARJ    = 145647,
    SPELL_ANNIHILLATE                = 144969,
    SPELL_COSMETIC_CHANNEL           = 145431,
    //HM
    SPELL_CRUSHING_FEAR_T_M          = 147320,

    //Special
    SPELL_SUMMON_ADDS                = 144489,
    SPELL_HEARTBEAT_SOUND            = 148591,
    SPELL_ENTER_REALM_OF_YSHAARJ     = 144867,
    SPELL_WEAKENED_BLOWS             = 115798,

    //Remove this absorb aura when player on mind control
    SPELL_SPIRIT_SHELL               = 114908,
    SPELL_DIVINE_AEGIS               = 47753,
    SPELL_POWER_WORD_SHIELD          = 17,
    SPELL_SACRED_SHIELD              = 65148,
};

uint32 auraarray[4] = 
{
    SPELL_SPIRIT_SHELL,
    SPELL_DIVINE_AEGIS,
    SPELL_POWER_WORD_SHIELD,
    SPELL_SACRED_SHIELD,
};

enum sEvents
{
    //Garrosh
    EVENT_DESECRATED_WEAPON          = 1,
    EVENT_HELLSCREAM_WARSONG         = 2,
    EVENT_SUMMON_WARBRINGERS         = 3,
    EVENT_SUMMON_WOLF_RIDER          = 4,
    EVENT_SUMMON_ENGINEER            = 5,
    EVENT_PHASE_TWO                  = 6,
    EVENT_WHIRLING_CORRUPTION        = 7,
    EVENT_GRIPPING_DESPAIR           = 8,
    EVENT_ENTER_REALM_OF_YSHAARJ     = 9,
    EVENT_RETURN_TO_REAL             = 10,
    EVENT_TOUCH_OF_YSHAARJ           = 11,
    //HM
    EVENT_INTRO_PHASE_FOUR           = 12,
    EVENT_PHASE_FOUR                 = 13,
    EVENT_MALICE                     = 14,
    EVENT_BOMBARTMENT                = 15,
    EVENT_MANIFEST_RAGE              = 16,
    //In Realm
    EVENT_ANNIHILLATE                = 17,
    //Desecrated weapon
    EVENT_REGENERATE                 = 18,
    //Summons
    EVENT_LAUNCH_STAR                = 19,
    EVENT_HAMSTRING                  = 20,
    EVENT_CHAIN_HEAL                 = 21,
    EVENT_CHAIN_LIGHTNING            = 22,
    //Iron Star
    EVENT_ACTIVE                     = 23,
    //Adds in realm
    EVENT_EMBODIED_DESPAIR           = 24,
    EVENT_EMBODIED_DOUBT             = 25,
    //Special events
    EVENT_CHECK_PROGRESS             = 26,
    EVENT_CAST                       = 27,
    EVENT_RE_ATTACK                  = 28,
    EVENT_CHANGE_TARGET              = 29,
};

enum Phase
{
    PHASE_NULL,
    PHASE_ONE,
    PHASE_PREPARE,
    PHASE_REALM_OF_YSHAARJ,
    PHASE_TWO,
    PHASE_LAST_PREPARE,
    PHASE_THREE,
    PHASE_FOUR_READY,
    PHASE_FOUR_PREPARE,
    PHASE_FOUR,
};

enum sActions
{
    ACTION_LAUNCH                    = 1,
    ACTION_PHASE_PREPARE             = 2,
    ACTION_INTRO_REALM_OF_YSHAARJ    = 3,
    ACTION_INTRO_PHASE_THREE         = 4,
    ACTION_PHASE_THREE               = 5,
    ACTION_INTRO_PHASE_FOUR          = 6,
    ACTION_PHASE_FOUR                = 7,
    ACTION_CHANGE_TARGET             = 8,
    ACTION_BOMBARTMENT               = 9,
    ACTION_GARROSH_HM_DONE           = 10,
    ACTION_MANIFEST_RAGE             = 11,
};

Position ironstarspawnpos[2] =
{
    {1087.05f, -5758.29f, -317.689f, 1.45992f},//R
    {1059.92f, -5520.2f, -317.689f, 4.64799f}, //L
};

Position engeneerspawnpos[2] =
{
    {1062.53f, -5739.85f, -303.7210f, 1.4820f},//R
    {1083.65f, -5538.95f, -303.7225f, 4.5215f},//L
};

Position rsspos[3] =
{
    {1015.77f, -5696.90f, -317.6967f, 6.1730f}, 
    {1014.27f, -5703.18f, -317.6998f, 6.1730f},
    {1012.70f, -5711.02f, -317.7060f, 6.1730f},
};

Position rdestpos[3] =
{
    {1060.23f, -5656.00f, -317.6279f, 0.0f},
    {1064.52f, -5658.68f, -317.6323f, 0.0f},
    {1069.67f, -5659.76f, -317.5973f, 0.0f},
};

Position lsspos[3] = 
{
    {1028.11f, -5572.19f, -317.6992f, 6.1730f},
    {1028.52f, -5563.87f, -317.7022f, 6.1730f},
    {1029.31f, -5556.24f, -317.7059f, 6.1730f},
};

Position ldestpos[3] =
{
    {1064.12f, -5620.54f, -317.6365f, 0.0f},
    {1068.99f, -5619.15f, -317.6217f, 0.0f},
    {1073.68f, -5619.27f, -317.5790f, 0.0f},
};

Position wspos[2] =
{
    {1020.22f, -5703.03f, -317.6922f, 6.1730f}, //r
    {1034.52f, -5565.62f, -317.6930f, 6.1730f}, //l
};

Position tppos[3] =
{   //1.1 scale sha vortex                                                      //HM
    {1092.66f, -5453.67f, -354.902802f, 1.4436f},//Temple of the Jade Serpent   //1
    {1084.72f, -5631.07f, -423.453369f, 3.0819f},//Terrace of Endless Spring    //2
    {1055.22f, -5844.58f, -318.864105f, 4.6069f},//Temple of the Red Crane      //3
};

Position gspos[3] =
{
    {1105.01f, -5344.21f, -349.7873f, 4.5881f},  //Temple of the Jade Serpent
    {820.47f,  -5601.41f, -397.7068f, 6.1840f},  //Terrace of Endless Spring
    {1056.55f, -5829.83f, -368.6667f, 4.6313f},  //Temple of the Red Crane
};

Position lastpltppos = {-8485.50f, 1132.68f, 18.2643f, 4.4671f};
Position lastgtppos  = {-8498.81f, 1079.69f, 17.9525f, 1.4708f};
Position spspawnpos  = {-8501.24f, 1112.63f, 17.9674f, 4.6652f};

Position crushingfeardest[60] =
{
    { 945.09f, -5616.38f, -416.4596f },
    { 943.80f, -5606.74f, -416.6546f },
    { 941.56f, -5626.87f, -416.6546f },
    { 941.50f, -5633.27f, -416.6546f },
    { 929.23f, -5624.66f, -416.6546f },
    { 927.09f, -5608.24f, -416.6545f },
    { 920.46f, -5615.16f, -416.6439f },
    { 909.92f, -5619.93f, -412.5998f },
    { 908.04f, -5607.89f, -410.5522f },
    { 903.76f, -5613.95f, -408.2687f },
    { 898.88f, -5618.77f, -405.5613f },
    { 900.51f, -5610.78f, -405.9869f },
    { 895.42f, -5616.37f, -406.1470f },
    { 887.50f, -5616.90f, -398.1998f },
    { 890.99f, -5608.75f, -399.7753f },
    { 884.20f, -5601.87f, -398.1624f },
    { 877.93f, -5608.24f, -398.2571f },
    { 875.95f, -5617.98f, -398.1881f },
    { 868.91f, -5609.46f, -398.2189f },
    { 857.37f, -5615.89f, -398.0228f },
    { 866.14f, -5601.85f, -398.1347f },
    { 852.62f, -5601.21f, -398.0230f },
    { 846.85f, -5618.18f, -398.0201f },
    { 843.52f, -5628.98f, -398.0222f },
    { 834.25f, -5639.14f, -398.0222f },
    { 820.12f, -5635.49f, -398.0016f },
    { 810.24f, -5644.04f, -398.0219f },
    { 807.55f, -5634.74f, -397.9928f },
    { 796.34f, -5642.77f, -398.0228f },
    { 801.46f, -5631.31f, -397.9842f },
    { 779.44f, -5637.16f, -398.0232f },
    { 784.98f, -5626.74f, -397.9983f },
    { 789.00f, -5617.14f, -397.9674f },
    { 769.92f, -5611.99f, -398.0152f },
    { 777.22f, -5603.79f, -397.9869f },
    { 788.07f, -5601.16f, -397.7069f },
    { 777.39f, -5598.40f, -397.9869f },
    { 781.84f, -5587.61f, -397.9838f },
    { 790.38f, -5589.49f, -397.7062f },
    { 772.27f, -5580.16f, -398.0230f },
    { 781.06f, -5573.41f, -398.0181f },
    { 789.79f, -5579.16f, -397.9833f },
    { 794.53f, -5582.82f, -397.9600f },
    { 790.67f, -5558.37f, -398.0228f },
    { 795.20f, -5567.22f, -398.0073f },
    { 801.34f, -5575.37f, -397.9735f },
    { 813.43f, -5569.04f, -397.9918f },
    { 811.53f, -5561.69f, -398.0173f },
    { 815.39f, -5553.86f, -398.0230f },
    { 827.26f, -5556.51f, -398.0230f },
    { 825.22f, -5564.75f, -398.0197f },
    { 819.54f, -5570.49f, -397.9948f },
    { 833.32f, -5559.99f, -398.0224f },
    { 842.52f, -5566.11f, -398.0224f },
    { 843.62f, -5575.47f, -398.0224f },
    { 852.16f, -5579.41f, -398.0224f },
    { 844.53f, -5582.54f, -398.0202f },
    { 854.53f, -5587.33f, -398.0223f },
    { 850.03f, -5595.03f, -398.0222f },
    { 858.35f, -5594.51f, -398.0222f },
};

Position centerpos = {1073.09f, -5639.70f, -317.3894f};
Position realmtppos = {1073.14f, -5639.47f, -317.3893f, 3.0128f};

uint32 transformvisual[4] =
{
    SPELL_TRANSITION_VISUAL_PHASE_2,
    SPELL_TRANSITION_VISUAL_PHASE_3,
    SPELL_PHASE_TWO_TRANSFORM,
    SPELL_PHASE_THREE_TRANSFORM,
};

enum CreatureText
{
    //Real
    SAY_ENTERCOMBAT                 = 1,//Я, Гаррош, сын Грома, покажу вам, что значит быть Адским Криком! 38064
    SAY_HELLSCREAM_WARSONG          = 2,//Умрите с честью! 38075
    SAY_SUMMON_WOLF_RIDER           = 3,//Исцелите наши раны! 38072
    SAY_START_LAUNCH_IRON_STAR      = 4,//Узрите силу оружия Истинной Орды! 38068
    SAY_START_LAUNCH_IRON_STAR2     = 5,//Мы очистим этот мир сталью и пламенем! 38070
    SAY_PHASE_PREPARE               = 6,//Злоба. Ненависть. Страх! Вот орудия войны, вот слуги Вождя! 38048
    SAY_PHASE_REALM_OF_YSHAARJ      = 7,//Да... я вижу... вижу, какое будущее ждет этот мир... им будет править Орда... Моя Орда! 38051
    SAY_WHIRLING_CORRUPTION         = 8,//Я полон силы! 38076
    SAY_EM_WHIRLING_CORRUPTION      = 9,//Сила во мне уничтожит вас и весь ваш мир. 38077
    SAY_LAST_PHASE                  = 10,//Начнется правление Истинной Орды. Я видел это. Он показал мне горы черепов и реки крови. Мир... будет... моим! 38056
    SAY_KILL_PLAYER                 = 11,//Ничтожество! 38065
    SAY_DIE                         = 12,//Это не может... кончиться... так... Нет... я же видел... 38046
    SAY_ENTER_REALM_OF_YSHAARJ      = 13,//Вы окажетесь в ловушке навеки! 38055
    SAY_HM_LAST_PHASE               = 14,//(долго злобно смеется) Думаете вы победили? Слепцы. Я раскрою вам глаза! 38057
    SAY_MANIFEST_RAGE               = 15,//Я уничтожу все что вам было дорого! 38050
};

//71865
class boss_garrosh_hellscream : public CreatureScript
{
    public:
        boss_garrosh_hellscream() : CreatureScript("boss_garrosh_hellscream") {}

        struct boss_garrosh_hellscreamAI : public BossAI
        {
            boss_garrosh_hellscreamAI(Creature* creature) : BossAI(creature, DATA_GARROSH)
            {
                instance = creature->GetInstanceScript();
                me->setPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_HELLSCREAM_WARSONG, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
            }
            InstanceScript* instance;
            uint32 realmofyshaarjtimer;
            uint32 updatepower;
            uint32 checkevade;
            uint32 lastphaseready;
            uint32 lastbombartmenttimer;
            uint8 bombartmentnum;
            uint8 realmnum;
            bool phasetwo;
            bool diedHealth{};
            Phase phase;

            void Reset()
            {
                phasetwo = false;
                checkevade = 0;
                updatepower = 0;
                bombartmentnum = 0;
                lastphaseready = 0;
                realmofyshaarjtimer = 0;
                lastbombartmenttimer = 55000;
                me->RemoveAurasDueToSpell(SPELL_GARROSH_ENERGY_4);
                me->SetPower(POWER_ENERGY, 0);
                phase = PHASE_NULL;
                if (me->ToTempSummon())
                {       //StormWind (Last phase Heroic) spawn   
                    if (me->GetMap()->GetAreaId(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()) == 6816)
                    {
                        me->AddAura(SPELL_SUMMON_ADDS, me);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_PASSIVE);
                        me->AddAura(SPELL_PHASE_TWO_TRANSFORM, me);
                        me->AddAura(SPELL_PHASE_THREE_TRANSFORM, me);
                    }
                    else
                    {   //Realm of Yshaarj spawn
                        updatepower = 1500;
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        me->SetReactState(REACT_PASSIVE);
                        DoCast(me, SPELL_YSHAARJ_PROTECTION);
                    }
                }
                else
                {       //Main Garrosh
                    if (me->GetMap()->IsHeroic() && instance->GetBossState(DATA_GARROSH) == DONE)
                    {
                        me->SetVisible(false);
                        return;
                    }

                    _Reset();
                    for (uint8 n = 0; n < 4; n++)
                        me->RemoveAurasDueToSpell(transformvisual[n]);
                    me->SetReactState(REACT_DEFENSIVE);
                    instance->SetData(DATA_RESET_REALM_OF_YSHAARJ, 0);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GARROSH_ENERGY);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DESECRATED);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GRIPPING_DESPAIR);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EM_GRIPPING_DESPAIR);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EXPLOSIVE_DESPAIR_DOT);
                }
            }

            void SpawnIronStar()
            {
                for (uint8 n = 0; n < 2; n++)
                    me->SummonCreature(NPC_KORKRON_IRON_STAR, ironstarspawnpos[n]);
            }

            void EnterCombat(Unit* who)
            {
                if (!me->ToTempSummon())
                {
                    _EnterCombat();
                    Talk(SAY_ENTERCOMBAT);
                    checkevade = 1000;
                    SpawnIronStar();
                    phase = PHASE_ONE;
                    realmnum = 0;
                    events.RescheduleEvent(EVENT_SUMMON_WARBRINGERS, 4000);
                    //events.RescheduleEvent(EVENT_CHECK_PROGRESS, 5000);
                    events.RescheduleEvent(EVENT_DESECRATED_WEAPON, 12000);
                    events.RescheduleEvent(EVENT_HELLSCREAM_WARSONG, 18000);
                    events.RescheduleEvent(EVENT_SUMMON_WOLF_RIDER, 30000);
                    events.RescheduleEvent(EVENT_SUMMON_ENGINEER, 20000);

                    //For safe
                    if (me->GetMap()->IsHeroic())
                        if (Creature* kgs = me->GetCreature(*me, instance->GetGuidData(NPC_KORKRON_GUNSHIP)))
                            kgs->AI()->DoAction(ACTION_RESET);
                }
            }

            void EnterEvadeMode()
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                ScriptedAI::EnterEvadeMode();
            }

            void JustReachedHome()
            {
                if (!me->ToTempSummon())
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    me->SetReactState(REACT_DEFENSIVE);
                }
            }

            void KilledUnit(Unit* unit)
            {
                if (unit->ToPlayer())
                    Talk(SAY_KILL_PLAYER);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                    //Copy of Garrosh(Realm of Yshaarj or StormWind)
                if (me->ToTempSummon())
                {
                    if (damage >= me->GetHealth() && phase != PHASE_FOUR)
                        damage = 0;
                }
                else
                {
                    diedHealth = false;
                    //Real Garrosh
                    if (damage >= me->GetHealth())
                    {
                        if (!me->GetMap()->IsHeroic())
                        {
                            if (phase != PHASE_THREE)
                            {
                                diedHealth = true;
                                damage = 0;
                            }
                        }
                        else
                        {
                            diedHealth = true;
                            damage = 0;
                        }
                    }
                    if (diedHealth)
                        me->SetHealth(100);

                    if (HealthBelowPct(10) && phase == PHASE_ONE)
                    {   //phase two
                        me->InterruptNonMeleeSpells(true);
                        phase = PHASE_PREPARE;
                        DoAction(ACTION_PHASE_PREPARE);
                    }
                    else if (HealthBelowPct(10) && phase == PHASE_TWO)
                    {   //phase three
                        me->InterruptNonMeleeSpells(true);
                        phase = PHASE_LAST_PREPARE;
                        realmofyshaarjtimer = 0;
                        DoAction(ACTION_INTRO_PHASE_THREE);
                    }
                    else if (HealthBelowPct(1) && phase == PHASE_FOUR_READY && me->GetMap()->IsHeroic())
                    {   //phase four, only heroic
                        me->InterruptNonMeleeSpells(true);
                        phase = PHASE_FOUR_PREPARE;
                        DoAction(ACTION_INTRO_PHASE_FOUR);
                    }
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                case ACTION_PHASE_PREPARE:
                    Talk(SAY_PHASE_PREPARE);
                    events.Reset();
                    if (!summons.empty())
                        summons.DespawnEntry(NPC_SIEGE_ENGINEER);
                    me->StopAttack();
                    me->GetMotionMaster()->MovePoint(1, centerpos);
                    break;
                case ACTION_INTRO_REALM_OF_YSHAARJ:
                    if (!phasetwo)
                    {
                        phasetwo = true;
                        me->SetFullHealth();
                    }
                    Talk(SAY_PHASE_REALM_OF_YSHAARJ);
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.RescheduleEvent(EVENT_ENTER_REALM_OF_YSHAARJ, 12000);
                    break;
                case ACTION_LAUNCH_ANNIHILLATE:
                    if (instance)
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                    updatepower = 0;
                    events.RescheduleEvent(EVENT_ANNIHILLATE, 4000);
                    break;
                case ACTION_CANCEL_ANNIHILLATE:
                    events.CancelEvent(EVENT_ANNIHILLATE);
                    me->InterruptNonMeleeSpells(true);
                    break;
                case ACTION_INTRO_PHASE_THREE:
                    events.Reset();
                    me->StopAttack();
                    me->GetMotionMaster()->MovePoint(2, centerpos);
                    break;
                case ACTION_PHASE_THREE:
                    phase = PHASE_THREE;
                    me->SetPower(POWER_ENERGY, 100);
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.RescheduleEvent(EVENT_GRIPPING_DESPAIR, 2000);
                    events.RescheduleEvent(EVENT_DESECRATED_WEAPON, 12000);
                    events.RescheduleEvent(EVENT_TOUCH_OF_YSHAARJ, 16000);
                    events.RescheduleEvent(EVENT_WHIRLING_CORRUPTION, 30000);
                    if (me->GetMap()->IsHeroic())
                        lastphaseready = 5000; //need delay before change phase
                    break;
                case ACTION_INTRO_PHASE_FOUR:
                    Talk(SAY_HM_LAST_PHASE);
                    events.Reset();
                    me->StopAttack();
                    events.RescheduleEvent(EVENT_INTRO_PHASE_FOUR, 20000);
                    break;
                case ACTION_PHASE_FOUR:
                    events.RescheduleEvent(EVENT_PHASE_FOUR, 10000);
                    break;
                case ACTION_MANIFEST_RAGE:
                    Talk(SAY_MANIFEST_RAGE);
                    DoCast(me, SPELL_MANIFEST_RAGE);
                    break;
                case ACTION_GARROSH_HM_DONE:
                    for (uint8 n = 0; n < 4; n++)
                        me->RemoveAurasDueToSpell(transformvisual[n]);
                    if (!summons.empty())
                    {
                        summons.DespawnEntry(NPC_MANIFESTATION_OF_RAGE);
                        summons.DespawnEntry(NPC_KORKRON_IRON_STAR_HM);
                    }
                    me->SetDisplayId(11686);
                    me->SetVisible(false);
                    me->SetLootRecipient(NULL);
                    me->Kill(me);
                    break;
                }
            }

            void MovementInform(uint32 type, uint32 pointId)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                    case 1:
                        DoCast(me, SPELL_TRANSITION_VISUAL_BASE, true);
                        DoCast(me, SPELL_TRANSITION_VISUAL_PHASE_2, true);
                        break;
                    case 2:
                        Talk(SAY_LAST_PHASE);
                        DoCast(me, SPELL_TRANSITION_VISUAL_BASE, true);
                        DoCast(me, SPELL_TRANSITION_VISUAL_ADVANCE, true);
                        DoCast(me, SPELL_TRANSITION_VISUAL_PHASE_3, true);
                        break;
                    default:
                        break;
                    }
                }
            }

            void JustDied(Unit* killer)
            {
                if (killer != me)
                {
                    Talk(SAY_DIE);
                    _JustDied();
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOUCH_OF_YSHAARJ);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EM_TOUCH_OF_YSHAARJ);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DESECRATED);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EM_GRIPPING_DESPAIR);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EXPLOSIVE_DESPAIR_DOT);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_IRON_STAR);
                    instance->SetData(DATA_PLAY_FINAL_MOVIE, 0);
                }

                //StormWind copy of Garrosh died, need destroy and obscure real Garrosh...
                if (me->ToTempSummon() && me->GetMap()->GetAreaId(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()) == 6816)
                {
                    if (Creature* realgarrosh = me->GetCreature(*me, instance->GetGuidData(DATA_GARROSH)))
                    {
                        realgarrosh->AI()->DoAction(ACTION_GARROSH_HM_DONE);
                        me->SummonCreature(NPC_PORTAL_TO_REALITY, spspawnpos);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MALICE);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MALICIOUS_BLAST);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_IRON_STAR);
                    }
                }
            }

            bool IfTargetHavePlayersInRange(Player* target, uint8 count, float radius)
            {
                count++;
                std::list<Player*>pllist;
                GetPlayerListInGrid(pllist, target, radius);
                if (pllist.size() >= count)
                    return true;
                return false;
            }

            bool CheckEvade()
            {
                if (Creature* stalker = me->FindNearestCreature(NPC_HEART_OF_YSHAARJ, 80.0f, true))
                    return true;
                return false;
            }

            void UpdateAI(uint32 diff)
            {
                //Garrosh from realm yshaarj
                if (updatepower)
                {
                    if (updatepower <= diff)
                    {
                        if (me->GetPower(POWER_ENERGY) <= 99)
                        {
                            uint8 power = me->GetPower(POWER_ENERGY) + 1;
                            me->SetPower(POWER_ENERGY, power);
                            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                            if (!PlayerList.isEmpty())
                                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                                    if (Player* player = Itr->getSource())
                                        if (player->isAlive())
                                            if (player->HasAura(SPELL_GARROSH_ENERGY))
                                                player->SetPower(POWER_ALTERNATE, power);
                            updatepower = 1500;
                        }
                    }
                    else
                        updatepower -= diff;
                }
                //

                if (lastphaseready)
                {
                    if (lastphaseready <= diff)
                    {
                        lastphaseready = 0;
                        phase = PHASE_FOUR_READY;
                    }
                    else
                        lastphaseready -= diff;
                }

                if (realmofyshaarjtimer)
                {
                    if (realmofyshaarjtimer <= diff)
                    {
                        realmofyshaarjtimer = 0;
                        me->InterruptNonMeleeSpells(true);
                        phase = PHASE_PREPARE;
                        DoAction(ACTION_PHASE_PREPARE);
                    }
                    else
                        realmofyshaarjtimer -= diff;
                }

                if (checkevade)
                {
                    if (checkevade <= diff)
                    {
                        if (!CheckEvade())
                        {
                            me->SetFullHealth();
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            EnterEvadeMode();
                            return;
                        }
                        else
                            checkevade = 1000;
                    }
                    else
                        checkevade -= diff;
                }
                //

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    //In Realm
                    //Phase one
                    case EVENT_CHECK_PROGRESS:
                        if (!instance->GetData(DATA_CHECK_INSTANCE_PROGRESS))
                        {
                            me->MonsterTextEmote("Not All Bosses Done, EnterEvadeMode", ObjectGuid::Empty, true);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            EnterEvadeMode();
                        }
                        break;
                    case EVENT_ANNIHILLATE:
                    {
                        float mod = urand(0, 6);
                        float orientation = mod <= 5 ? mod + float(urand(1, 9)) / 10 : mod;
                        me->SetFacingTo(orientation);
                        DoCast(me, SPELL_ANNIHILLATE);
                        events.RescheduleEvent(EVENT_ANNIHILLATE, 4000);
                        break;
                    }
                    //
                    case EVENT_SUMMON_WARBRINGERS:
                        instance->SetData(DATA_OPEN_SOLDIER_FENCH, 0);
                        for (uint8 n = 0; n < 3; n++)
                            if (Creature* warbringer = me->SummonCreature(NPC_WARBRINGER, rsspos[n]))
                                warbringer->GetMotionMaster()->MoveCharge(rdestpos[n].GetPositionX(), rdestpos[n].GetPositionY(), rdestpos[n].GetPositionZ(), 10.0f + n*2, 5);
                        for (uint8 n = 0; n < 3; n++)
                            if (Creature* warbringer = me->SummonCreature(NPC_WARBRINGER, lsspos[n]))
                                warbringer->GetMotionMaster()->MoveCharge(ldestpos[n].GetPositionX(), ldestpos[n].GetPositionY(), ldestpos[n].GetPositionZ(), 10.0f + n*2, 5);
                        events.RescheduleEvent(EVENT_SUMMON_WARBRINGERS, 45000);
                        break;
                    case EVENT_HELLSCREAM_WARSONG:
                        Talk(SAY_HELLSCREAM_WARSONG);
                        DoCast(me, SPELL_HELLSCREAM_WARSONG);
                        events.RescheduleEvent(EVENT_HELLSCREAM_WARSONG, 42000);
                        break;
                    case EVENT_SUMMON_WOLF_RIDER:
                    {
                        Talk(SAY_SUMMON_WOLF_RIDER);
                        uint8 pos = urand(0, 1);
                        if (Creature* wrider = me->SummonCreature(NPC_WOLF_RIDER, wspos[pos]))
                            wrider->GetMotionMaster()->MoveCharge(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 10.0f, 5);
                        events.RescheduleEvent(EVENT_SUMMON_WOLF_RIDER, 50000);
                        break;
                    }
                    case EVENT_SUMMON_ENGINEER:
                        if (!summons.empty())
                        {
                            summons.DespawnEntry(NPC_KORKRON_IRON_STAR);
                            summons.DespawnEntry(NPC_SIEGE_ENGINEER);
                        }
                        SpawnIronStar();
                        for (uint8 n = 0; n < 2; n++)
                            me->SummonCreature(NPC_SIEGE_ENGINEER, engeneerspawnpos[n]);
                        Talk(urand(SAY_START_LAUNCH_IRON_STAR, SAY_START_LAUNCH_IRON_STAR2));
                        events.RescheduleEvent(EVENT_SUMMON_ENGINEER, 40000);
                        break;
                    case EVENT_DESECRATED_WEAPON:
                    {
                        uint8 count = Is25ManRaid() ? 7 : 3;
                        bool havetarget = false;
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, me, 150.0f);
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->isInTankSpec() && me->GetExactDist(*itr) >= 15.0f)
                                {
                                    if (IfTargetHavePlayersInRange(*itr, count, 15.0f))
                                    {
                                        havetarget = true;
                                        DoCast(*itr, me->GetPower(POWER_ENERGY) >= 75 ? SPELL_EM_DESECRETE : SPELL_DESECRETE);
                                        break;
                                    }
                                }
                            }
                            //If no target in range, take melee
                            if (!havetarget)
                            {
                                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                                {
                                    if (!(*itr)->isInTankSpec())
                                    {
                                        if (IfTargetHavePlayersInRange(*itr, count, 15.0f))
                                        {
                                            havetarget = true;
                                            DoCast(*itr, me->GetPower(POWER_ENERGY) >= 75 ? SPELL_EM_DESECRETE : SPELL_DESECRETE);
                                            break;
                                        }
                                    }
                                }
                            }
                            //If still no target, take random include tank
                            if (!havetarget)
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                    DoCast(target, me->GetPower(POWER_ENERGY) >= 75 ? SPELL_EM_DESECRETE : SPELL_DESECRETE);
                        }
                        events.RescheduleEvent(EVENT_DESECRATED_WEAPON, 40000);
                        break;
                    }
                    case EVENT_ENTER_REALM_OF_YSHAARJ:
                    {
                        instance->SetData(DATA_ACTION_SOLDIER, 1);
                        me->StopAttack();
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                        instance->SetData(DATA_RESET_REALM_OF_YSHAARJ, 0); //for safe
                        uint8 mod;
                        if (me->GetMap()->IsHeroic())
                        {
                            mod = realmnum;
                            instance->SetData(DATA_PREPARE_REALM_OF_YSHAARJ, mod);
                        }
                        else
                            mod = uint8(instance->GetData(DATA_GET_REALM_OF_YSHAARJ));
                        if (Creature* garroshrealm = me->SummonCreature(NPC_GARROSH, gspos[mod]))
                        {
                            garroshrealm->AddAura(SPELL_PHASE_TWO_TRANSFORM, garroshrealm);
                            uint32 hp = me->GetHealth();
                            uint32 power = me->GetPower(POWER_ENERGY);
                            garroshrealm->SetHealth(hp);
                            garroshrealm->SetPower(POWER_ENERGY, power);
                            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HAMSTRING);
                            std::list<Player*>pllist;
                            GetPlayerListInGrid(pllist, me, 150.0f);
                            if (!pllist.empty())
                            {
                                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                                {
                                    (*itr)->NearTeleportTo(tppos[mod].GetPositionX(), tppos[mod].GetPositionY(), tppos[mod].GetPositionZ(), tppos[mod].GetOrientation());
                                    (*itr)->AddAura(SPELL_REALM_OF_YSHAARJ, *itr);
                                    (*itr)->CastSpell(*itr, SPELL_GARROSH_ENERGY, true);
                                }
                            }
                            if (me->GetMap()->IsHeroic() && mod == 1)
                                garroshrealm->CastSpell(garroshrealm, SPELL_CRUSHING_FEAR, true);
                            realmnum++;
                            garroshrealm->AI()->Talk(SAY_ENTER_REALM_OF_YSHAARJ);
                            phase = PHASE_REALM_OF_YSHAARJ;
                            events.RescheduleEvent(EVENT_RETURN_TO_REAL, 60000);
                        }
                        break;
                    }
                    case EVENT_RETURN_TO_REAL:
                    {
                        if (Creature* garroshrealm = me->GetCreature(*me, instance->GetGuidData(DATA_GARROSH_REALM)))
                            garroshrealm->AI()->DoAction(ACTION_CANCEL_ANNIHILLATE);
                        instance->SetData(DATA_CHECK_DIED_PLAYER_IN_REALM_OF_YSHARRJ, 0);
                        Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                        if (!PlayerList.isEmpty())
                            for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                                if (Player* player = Itr->getSource())
                                    if (player->isAlive())
                                        if (player->HasAura(SPELL_REALM_OF_YSHAARJ))
                                            player->RemoveAurasDueToSpell(SPELL_REALM_OF_YSHAARJ);
                        events.RescheduleEvent(EVENT_PHASE_TWO, 3000);
                        break;
                    }
                    //Phase Two
                    case EVENT_PHASE_TWO:
                        if (Creature* garroshrealm = me->GetCreature(*me, instance->GetGuidData(DATA_GARROSH_REALM)))
                        {
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, garroshrealm);
                            int32 power = garroshrealm->GetPower(POWER_ENERGY);
                            uint32 hp = garroshrealm->GetHealth();
                            garroshrealm->DespawnOrUnsummon();
                            instance->SetData(DATA_UPDATE_GARROSH_REALM, 0);
                            me->SetPower(POWER_ENERGY, power);
                            me->SetHealth(hp);
                            phase = PHASE_TWO;
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                            me->SetReactState(REACT_AGGRESSIVE);
                            instance->SetData(DATA_ACTION_SOLDIER, 0);
                            events.RescheduleEvent(EVENT_GRIPPING_DESPAIR, 2000);
                            events.RescheduleEvent(EVENT_DESECRATED_WEAPON, 12000);
                            events.RescheduleEvent(EVENT_TOUCH_OF_YSHAARJ, 16000);
                            events.RescheduleEvent(EVENT_WHIRLING_CORRUPTION, 30000); 
                            if (realmnum != 3)
                                realmofyshaarjtimer = 150000;
                        }
                        break;
                    case EVENT_TOUCH_OF_YSHAARJ:
                    {
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, me, 150.0f);
                        uint8 count = 0;
                        uint8 maxcount = me->GetMap()->Is25ManRaid() ? 4 : 2;
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->isInTankSpec())
                                {
                                    if (!(*itr)->HasAura(SPELL_TOUCH_OF_YSHAARJ) && !(*itr)->HasAura(SPELL_EM_TOUCH_OF_YSHAARJ))
                                    {
                                        count++;
                                        DoCast(*itr, me->GetPower(POWER_ENERGY) >= 50 ? SPELL_EM_TOUCH_OF_YSHAARJ : SPELL_TOUCH_OF_YSHAARJ, true);
                                    }
                                    if (count >= maxcount)
                                        break;
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_TOUCH_OF_YSHAARJ, 45000);
                        break;
                    }
                    case EVENT_GRIPPING_DESPAIR:
                        if (me->getVictim())
                            DoCastVictim(me->GetPower(POWER_ENERGY) == 100 ? SPELL_EM_GRIPPING_DESPAIR : SPELL_GRIPPING_DESPAIR);
                        events.RescheduleEvent(EVENT_GRIPPING_DESPAIR, 4000);
                        break;
                    case EVENT_WHIRLING_CORRUPTION:
                        events.CancelEvent(EVENT_GRIPPING_DESPAIR);
                        Talk(me->GetPower(POWER_ENERGY) >= 25 ? SAY_WHIRLING_CORRUPTION : SAY_EM_WHIRLING_CORRUPTION);
                        DoCast(me, me->GetPower(POWER_ENERGY) >= 25 ? SPELL_EM_WHIRLING_CORRUPTION : SPELL_WHIRLING_CORRUPTION);
                        events.RescheduleEvent(EVENT_GRIPPING_DESPAIR, 10000);
                        events.RescheduleEvent(EVENT_WHIRLING_CORRUPTION, 49500);
                        break;
                    //HM
                    case EVENT_INTRO_PHASE_FOUR:
                    {
                        checkevade = 0;
                        me->ModifyHealth(int32(me->CountPctFromMaxHealth(60)));
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        me->SetPower(POWER_ENERGY, 0);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                        instance->SetData(DATA_KILL_PLAYERS_IN_MIND_CONTROL, 0);
                        if (!summons.empty())
                        {
                            summons.DespawnEntry(NPC_MINION_OF_YSHAARJ);
                            summons.DespawnEntry(NPC_DESECRATED_WEAPON);
                            summons.DespawnEntry(NPC_EMPOWERED_DESECRATED_WEAPON);
                        }
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GRIPPING_DESPAIR);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EM_GRIPPING_DESPAIR);
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EXPLOSIVE_DESPAIR_DOT);
                        uint32 hp = me->GetHealth();
                        if (Creature* stormwindgarrosh = me->SummonCreature(NPC_GARROSH, lastgtppos.GetPositionX(), lastgtppos.GetPositionY(), lastgtppos.GetPositionZ(), lastgtppos.GetOrientation()))
                        {
                            stormwindgarrosh->SetHealth(hp);
                            std::list<Player*>pllist;
                            GetPlayerListInGrid(pllist, me, 150.0f);
                            if (!pllist.empty())
                                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                                    (*itr)->NearTeleportTo(lastpltppos.GetPositionX(), lastpltppos.GetPositionY(), lastpltppos.GetPositionZ(), lastpltppos.GetOrientation());
                            stormwindgarrosh->AI()->DoAction(ACTION_PHASE_FOUR);
                        }
                        break;
                    }
                    case EVENT_PHASE_FOUR:
                    {
                        phase = PHASE_FOUR;
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                        DoCast(me, SPELL_GARROSH_ENERGY_4, true);
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.RescheduleEvent(EVENT_MALICE, 14000);
                        events.RescheduleEvent(EVENT_BOMBARTMENT, lastbombartmenttimer);
                        break;
                    }
                    case EVENT_MALICE:
                    {
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, me, 150.0f);
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->isInTankSpec() && !(*itr)->HasAura(SPELL_FIXATE_IRON_STAR))
                                {
                                    (*itr)->AddAura(SPELL_MALICE, *itr);
                                    break;
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_MALICE, 28000);
                        break;
                    }
                    case EVENT_BOMBARTMENT:
                        me->MonsterTextEmote("Warning: Bombartment", ObjectGuid::Empty, true);
                        if (Creature* kg = me->GetCreature(*me, instance->GetGuidData(NPC_KORKRON_GUNSHIP)))
                            kg->AI()->DoAction(ACTION_BOMBARTMENT);
                        if (++bombartmentnum == 2)
                        {
                            bombartmentnum = 0;
                            lastbombartmenttimer = lastbombartmenttimer > 15000 ? lastbombartmenttimer - 15000 : 15000;
                        }
                        events.RescheduleEvent(EVENT_BOMBARTMENT, lastbombartmenttimer);
                        break;
                    }
                }
                if (!me->ToTempSummon())
                    DoMeleeAttackIfReady();
                else if (me->HasAura(SPELL_GARROSH_ENERGY_4))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_garrosh_hellscreamAI(creature);
        }
};

//All summons, include realm of yshaarj
class npc_garrosh_soldier : public CreatureScript
{
public:
    npc_garrosh_soldier() : CreatureScript("npc_garrosh_soldier") {}

    struct npc_garrosh_soldierAI : public ScriptedAI
    {
        npc_garrosh_soldierAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        EventMap events;
        bool firstengeneerdied;

        void Reset()
        {
            firstengeneerdied = false;
            switch (me->GetEntry())
            {
            case NPC_SIEGE_ENGINEER:
                me->SetReactState(REACT_PASSIVE);
                events.RescheduleEvent(EVENT_LAUNCH_STAR, 500);
                break;
            case NPC_EMBODIED_DESPAIR:
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->SetReactState(REACT_AGGRESSIVE);
                DoCast(me, SPELL_CONSUMED_HOPE, true);
                break;
            case NPC_MINION_OF_YSHAARJ:
                me->setPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetPower(POWER_ENERGY, 0);
                if (me->ToTempSummon())
                {
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    {
                        if (!summoner->isAlive() || !summoner->isInCombat())
                        {
                            me->DespawnOrUnsummon();
                            return;
                        }
                    }
                }
                me->SetReactState(REACT_AGGRESSIVE);
                DoCast(me, SPELL_EMPOWERED);
                if (me->GetMap()->IsHeroic())
                    DoCast(me, SPELL_EMPOWERED_HM, true);
                break;
            case NPC_EMBODIED_DOUBT:
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->SetReactState(REACT_AGGRESSIVE);
                break;
            case NPC_WARBRINGER:
            case NPC_WOLF_RIDER:
                me->StopAttack();
                me->AddAura(SPELL_SUMMON_ADDS, me);
                break;
            default:
                me->AddAura(SPELL_SUMMON_ADDS, me);
                break;
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (pointId == 5 && (me->GetEntry() == NPC_WARBRINGER || me->GetEntry() == NPC_WOLF_RIDER))
                events.RescheduleEvent(EVENT_ACTIVE, 100);
        }

        void OnInterruptCast(Unit* /*caster*/, uint32 /*spellId*/, uint32 curSpellID, uint32 /*schoolMask*/)
        {
            if (me->HasAura(SPELL_ANCESTRAL_FURY))
                DoCast(me, SPELL_FURY, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (me->GetEntry() == NPC_SIEGE_ENGINEER)
            {
                if (damage >= me->GetHealth())
                {
                    me->RemoveAurasDueToSpell(SPELL_POWER_IRON_STAR, ObjectGuid::Empty, 0, AURA_REMOVE_BY_DEATH);
                    if (firstengeneerdied)
                        damage = 0;
                }
            }
        }

        void DoAction(int32 const action)
        {
            if (me->GetEntry() == NPC_SIEGE_ENGINEER)
                if (action == ACTION_FIRST_ENGENEER_DIED)
                    firstengeneerdied = true;
        }

        void JustDied(Unit* killer)
        {
            if (me->GetEntry() == NPC_WARBRINGER || me->GetEntry() == NPC_WOLF_RIDER)
                me->DespawnOrUnsummon();
            else if (me->GetEntry() == NPC_SIEGE_ENGINEER)
            {
                if (instance)
                    instance->SetData(DATA_FIRST_ENGENEER_DIED, 1);
                me->DespawnOrUnsummon();
            }
            else if (me->GetEntry() == NPC_MINION_OF_YSHAARJ)
                me->DespawnOrUnsummon(2000);
        }

        void EnterCombat(Unit* who)
        {
            switch (me->GetEntry())
            {
            case NPC_WARBRINGER:
                events.RescheduleEvent(EVENT_HAMSTRING, 5000);
                break;
            case NPC_WOLF_RIDER:
                DoCast(me, SPELL_ANCESTRAL_FURY);
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 15000);
                events.RescheduleEvent(EVENT_CHAIN_HEAL, 21500);
                break;
            case NPC_EMBODIED_DESPAIR:
                if (me->GetMap()->IsHeroic())
                    DoCast(me, SPELL_ULTIMATE_DESPAIR);
                else
                    events.RescheduleEvent(EVENT_EMBODIED_DESPAIR, 10000);
                break;
            case NPC_EMBODIED_DOUBT:
                if (me->GetMap()->IsHeroic())
                    events.RescheduleEvent(EVENT_EMBODIED_DOUBT, 2000);
                break;
            default:
                break;
            }
        }

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (me->GetEntry() == NPC_WOLF_RIDER && IsInControl())
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    //Engineer
                case EVENT_LAUNCH_STAR:
                    if (Creature* ironstar = me->FindNearestCreature(NPC_KORKRON_IRON_STAR, 60.0f, true))
                    {
                        me->SetFacingToObject(ironstar);
                        DoCast(me, SPELL_POWER_IRON_STAR);
                    }
                    break;
                    //Warbringer
                case EVENT_ACTIVE:
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                    /*if (me->GetEntry() == NPC_WARBRINGER)
                    {
                        if (!me->GetMap()->IsHeroic())
                        {
                            me->SetReactState(REACT_AGGRESSIVE);
                            DoZoneInCombat(me, 150.0f);
                        }
                        else
                        {
                            if (Player* pl = me->FindNearestPlayer(150.0f, true))
                            {
                                me->AddThreat(pl, 50000000.0f);
                                me->SetReactState(REACT_AGGRESSIVE);
                                me->Attack(pl, true);
                                me->GetMotionMaster()->MoveChase(pl);
                            }
                            events.RescheduleEvent(EVENT_CHANGE_TARGET, urand(4000, 6000));
                        }
                    }
                    else if (me->GetEntry() == NPC_WOLF_RIDER)
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat(me, 150.0f);
                    }*/
                    break;
                case EVENT_CHANGE_TARGET:
                {
                    ObjectGuid LastTargetGuid = me->getVictim() ? me->getVictim()->GetGUID() : ObjectGuid::Empty;
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    if (pllist.size() > 1) //if have any targets
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if ((*itr)->GetGUID() != LastTargetGuid)
                            {
                                me->StopAttack();
                                me->AddThreat(*itr, 50000000.0f);
                                me->SetReactState(REACT_AGGRESSIVE);
                                me->Attack(*itr, true);
                                me->GetMotionMaster()->MoveChase(*itr);
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_CHANGE_TARGET, urand(4000, 6000));
                    break;
                }
                case EVENT_HAMSTRING:
                    if (me->getVictim() && me->ToTempSummon())
                        if (Unit* garrosh = me->ToTempSummon()->GetSummoner())
                            me->CastSpell(me->getVictim(), SPELL_HAMSTRING, false, 0, 0, garrosh->GetGUID());
                    events.RescheduleEvent(EVENT_HAMSTRING, 15000);
                    break;
                    //Wolf Rider
                case EVENT_CHAIN_LIGHTNING:
                {
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 100.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->isInTankSpec())
                            {
                                DoCast(*itr, SPELL_CHAIN_LIGHTNING);
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 15000);
                    break;
                }
                case EVENT_CHAIN_HEAL:
                    if (Unit* ftarget = DoSelectLowestHpFriendly(60.0f))
                        if (ftarget->HealthBelowPct(90))
                            DoCast(ftarget, SPELL_ANCESTRAL_CHAIN_HEAL);
                    events.RescheduleEvent(EVENT_CHAIN_HEAL, 21500);
                    break;
                //Realm of Yshaarj
                //Embodied Despair
                case EVENT_EMBODIED_DESPAIR:
                    DoCast(me, SPELL_EMBODIED_DESPAIR);
                    events.RescheduleEvent(EVENT_EMBODIED_DESPAIR, 20000);
                    break;
                //Embodied doubt
                case EVENT_EMBODIED_DOUBT:
                    DoCast(me, SPELL_EMBODIED_DOUBT_HM);
                    events.RescheduleEvent(EVENT_EMBODIED_DOUBT, 4000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_garrosh_soldierAI(creature);
    }
};

//71985
class npc_iron_star : public CreatureScript
{
public:
    npc_iron_star() : CreatureScript("npc_iron_star") {}

    struct npc_iron_starAI : public ScriptedAI
    {
        npc_iron_starAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset(){}

        void DoAction(int32 const action)
        {
            if (action == ACTION_LAUNCH)
            {
                float x, y;
                DoCast(me, SPELL_IRON_STAR_IMPACT_AT, true);
                GetPositionWithDistInOrientation(me, 200.0f, me->GetOrientation(), x, y);
                me->GetMotionMaster()->MoveJump(x, y, -317.4815f, 25.0f, 0.0f, 1);
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE && pointId == 1)
            {
                DoCast(me, SPELL_EXPLODING_IRON_STAR, true);
                me->Kill(me, true);
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE)
                {
                    float x, y;
                    DoCast(me, SPELL_IRON_STAR_IMPACT_AT, true);
                    GetPositionWithDistInOrientation(me, 200.0f, me->GetOrientation(), x, y);
                    me->GetMotionMaster()->MoveJump(x, y, -317.4815f, 25.0f, 0.0f, 1);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_iron_starAI(creature);
    }
};

//73059
class npc_unstable_iron_star : public CreatureScript
{
public:
    npc_unstable_iron_star() : CreatureScript("npc_unstable_iron_star") {}

    struct npc_unstable_iron_starAI : public ScriptedAI
    {
        npc_unstable_iron_starAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_ACTIVE, 3000);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DoAction(int32 const action)
        {
            if (action == ACTION_CHANGE_TARGET)
                events.RescheduleEvent(EVENT_CHANGE_TARGET, 100);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        ObjectGuid GetFixateTarget()
        {
            float range = 0;
            std::list<Player*>pllist;
            for (uint8 n = 1; n < 10; n++)
            {
                float range = n * 10;
                pllist.clear();
                GetPlayerListInGrid(pllist, me, range);
                if (!pllist.empty())
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        if (!(*itr)->HasAura(SPELL_MALICE))
                            return (*itr)->GetGUID();
            }
            return ObjectGuid::Empty;
        }

        void FindAndFixateTarget()
        {
            if (Player* pl = me->GetPlayer(*me, GetFixateTarget()))
            {
                DoCast(me, SPELL_IRON_STAR_IMPACT_AT_HM, true);
                DoCast(pl, SPELL_FIXATE_IRON_STAR, true);
                me->AddThreat(pl, 50000000.0f);
                me->SetReactState(REACT_AGGRESSIVE);
                me->Attack(pl, true);
                me->GetMotionMaster()->MoveChase(pl);
            }
            else
                me->DespawnOrUnsummon();
        }

        void JustDied(Unit* killer)
        {
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_IRON_STAR);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE:
                    me->getThreatManager().resetAllAggro();
                    FindAndFixateTarget();
                    break;
                case EVENT_CHANGE_TARGET:
                    me->StopAttack();
                    if (instance)
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_IRON_STAR);
                    events.RescheduleEvent(EVENT_ACTIVE, 2000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unstable_iron_starAI(creature);
    }
};

//72154
class npc_desecrated_weapon : public CreatureScript
{
public:
    npc_desecrated_weapon() : CreatureScript("npc_desecrated_weapon") {}

    struct npc_desecrated_weaponAI : public ScriptedAI
    {
        npc_desecrated_weaponAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            lastpct = 90;
        }

        InstanceScript* instance;
        uint8 lastpct;

        void Reset()
        {
            if (me->ToTempSummon())
            {
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                {
                    if (!summoner->isInCombat())
                        me->DespawnOrUnsummon();
                    else
                    {
                        DoCast(me, SPELL_DESECRATED_WEAPON_AXE);
                        DoCast(me, SPELL_DESECRATED_WEAPON_AT);
                    }
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                me->DespawnOrUnsummon();
            else
            {
                if (lastpct && HealthBelowPct(lastpct))
                {
                    float scale = float(lastpct) / 100;
                    //if (AreaTrigger* at = me->GetAreaObject(SPELL_DESECRATED_WEAPON_AT))
                    //    at->SetFloatValue(AREATRIGGER_EXPLICIT_SCALE, scale);
                    lastpct = lastpct - 10;
                }
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_desecrated_weaponAI(creature);
    }
};

//72198
class npc_empowered_desecrated_weapon : public CreatureScript
{
public:
    npc_empowered_desecrated_weapon() : CreatureScript("npc_empowered_desecrated_weapon") {}

    struct npc_empowered_desecrated_weaponAI : public ScriptedAI
    {
        npc_empowered_desecrated_weaponAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            lastpct = 90;
        }

        InstanceScript* instance;
        EventMap events;
        uint32 hppctmod;
        uint8 lastpct;

        void Reset()
        {
            events.Reset();
            hppctmod = me->CountPctFromMaxHealth(10);
            if (me->ToTempSummon())
            {
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                {
                    if (!summoner->isInCombat())
                        me->DespawnOrUnsummon();
                    else
                    {
                        DoCast(me, SPELL_DESECRATED_WEAPON_AXE);
                        DoCast(me, SPELL_EM_DESECRATED_WEAPON_AT);
                        events.RescheduleEvent(EVENT_REGENERATE, 10000);
                    }
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;

            if (lastpct && HealthBelowPct(lastpct))
            {
                float scale = float(lastpct) / 100;
                //if (AreaTrigger* at = me->GetAreaObject(SPELL_EM_DESECRATED_WEAPON_AT))
                //    at->SetFloatValue(AREATRIGGER_EXPLICIT_SCALE, scale);
                lastpct = lastpct - 10;
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_REGENERATE)
                {
                    if (me->GetHealthPct() < 100)
                    {
                        float scale = 0;
                        if (me->GetHealth() + hppctmod >= me->GetMaxHealth())
                        {
                            me->SetFullHealth();
                            lastpct = 100;
                            scale = 1;
                        }
                        else
                        {
                            me->SetHealth(me->GetHealth() + hppctmod);
                            lastpct = uint8(floor(me->GetHealthPct()));
                            scale = float(lastpct) / 100;
                        }
                        //if (AreaTrigger* at = me->GetAreaObject(SPELL_EM_DESECRATED_WEAPON_AT))
                        //    at->SetFloatValue(AREATRIGGER_EXPLICIT_SCALE, scale);
                    }
                    events.RescheduleEvent(EVENT_REGENERATE, 10000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_empowered_desecrated_weaponAI(creature);
    }
};

//72215 
class npc_heart_of_yshaarj : public CreatureScript
{
public:
    npc_heart_of_yshaarj() : CreatureScript("npc_heart_of_yshaarj") {}

    struct npc_heart_of_yshaarjAI : public ScriptedAI
    {
        npc_heart_of_yshaarjAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid charmedplGuid;

        void Reset()
        {
            if (!me->ToTempSummon())
            {
                charmedplGuid.Clear();
                DoCast(me, SPELL_HEARTBEAT_SOUND, true);
            }
        }

        void SetGUID(ObjectGuid const& guid, int32 type) override
        {
            charmedplGuid = guid;
            events.RescheduleEvent(EVENT_CAST, 6000);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        bool IsPlayerInMindControl(Player* pl)
        {
            if (pl->HasAura(SPELL_TOUCH_OF_YSHAARJ) || pl->HasAura(SPELL_EM_TOUCH_OF_YSHAARJ))
                return true;
            return false;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CAST:
                    if (Player* pl = me->GetPlayer(*me, charmedplGuid))
                    {
                        if (pl->isAlive() && IsPlayerInMindControl(pl))
                        {
                            pl->GetMotionMaster()->MoveIdle();
                            pl->CastSpell(pl, SPELL_PL_TOUCH_OF_YSHAARJ);
                            events.RescheduleEvent(EVENT_RE_ATTACK, 3000);
                            return;
                        }
                    }
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_RE_ATTACK:
                    if (Player* pl = me->GetPlayer(*me, charmedplGuid))
                    {
                        if (pl->isAlive() && IsPlayerInMindControl(pl))
                        {
                            if (pl->getVictim())
                                pl->GetMotionMaster()->MoveChase(pl->getVictim());
                            events.RescheduleEvent(EVENT_CAST, 6000);
                            return;
                        }
                    }
                    me->DespawnOrUnsummon();
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_heart_of_yshaarjAI(creature);
    }
};

//72228
class npc_heart_of_yshaarj_realm : public CreatureScript
{
public:
    npc_heart_of_yshaarj_realm() : CreatureScript("npc_heart_of_yshaarj_realm") {}

    struct npc_heart_of_yshaarj_realmAI : public ScriptedAI
    {
        npc_heart_of_yshaarj_realmAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }

        InstanceScript* instance;

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_heart_of_yshaarj_realmAI(creature);
    }
};

//72239
class npc_sha_vortex : public CreatureScript
{
public:
    npc_sha_vortex() : CreatureScript("npc_sha_vortex") {}

    struct npc_sha_vortexAI : public ScriptedAI
    {
        npc_sha_vortexAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }

        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_YSHAARJ_PROTECTION_AT, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sha_vortexAI(creature);
    }
};

//73065
class npc_korkron_gunship : public CreatureScript
{
public:
    npc_korkron_gunship() : CreatureScript("npc_korkron_gunship") {}

    struct npc_korkron_gunshipAI : public ScriptedAI
    {
        npc_korkron_gunshipAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            if (!me->GetMap()->IsHeroic())
                me->SetVisible(false);
        }
        InstanceScript* instance;

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_BOMBARTMENT:
                DoCast(me, SPELL_CALL_BOMBARTMENT);
                break;
            case ACTION_RESET:
                me->RemoveAurasDueToSpell(SPELL_CALL_BOMBARTMENT);
                DespawnAllATAndSummons();
                break;
            }
        }

        void DespawnAllATAndSummons()
        {
            std::list<AreaTrigger*> atlist;
            me->GetAreaTriggersWithEntryInRange(atlist, 5236, ObjectGuid::Empty, 400.0f);
            if (!atlist.empty())
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                    (*itr)->RemoveFromWorld();

            std::list<Creature*> iistarlist;
            iistarlist.clear();
            GetCreatureListWithEntryInGrid(iistarlist, me, NPC_KORKRON_IRON_STAR_HM, 400.0f);
            if (!iistarlist.empty())
                for (std::list<Creature*>::const_iterator Itr = iistarlist.begin(); Itr != iistarlist.end(); Itr++)
                    (*Itr)->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_gunshipAI(creature);
    }
};

//73665
class npc_horde_cannon : public CreatureScript
{
public:
    npc_horde_cannon() : CreatureScript("npc_horde_cannon") {}

    struct npc_horde_cannonAI : public ScriptedAI
    {
        npc_horde_cannonAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            if (!me->GetMap()->IsHeroic())
                me->SetVisible(false);
        }
        InstanceScript* instance;

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_horde_cannonAI(creature);
    }
};

//74007
class npc_portal_to_reality : public CreatureScript
{
public:
    npc_portal_to_reality() : CreatureScript("npc_portal_to_reality") {}

    struct npc_portal_to_realityAI : public ScriptedAI
    {
        npc_portal_to_realityAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void OnSpellClick(Unit* clicker)
        {
            clicker->NearTeleportTo(centerpos.GetPositionX(), centerpos.GetPositionY(), centerpos.GetPositionZ(), 0.0f);
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_portal_to_realityAI(creature);
    }
};

//144798
class spell_exploding_iron_star : public SpellScriptLoader
{
public:
    spell_exploding_iron_star() : SpellScriptLoader("spell_exploding_iron_star") { }

    class spell_exploding_iron_star_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_exploding_iron_star_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                float distance = GetCaster()->GetExactDist2d(GetHitUnit());
                if (distance)
                {
                    //Better Scale Damage
                    if (distance <= 50)
                        SetHitDamage((GetHitDamage()) * (1 - (distance / 300)));
                    else if (distance > 50 && distance <= 80)
                        SetHitDamage((GetHitDamage() / 2) * (1 - (distance / 300)));
                    else if (distance > 80 && distance <= 300)
                        SetHitDamage((GetHitDamage() / 3) * (1 - (distance / 300)));
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_exploding_iron_star_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_exploding_iron_star_SpellScript();
    }
};

//144989
class spell_whirling_corruption : public SpellScriptLoader
{
public:
    spell_whirling_corruption() : SpellScriptLoader("spell_whirling_corruption") { }

    class spell_whirling_corruption_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_whirling_corruption_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                float distance = GetCaster()->GetExactDist2d(GetHitUnit());
                if (distance >= 0 && distance <= 100)
                    SetHitDamage(GetHitDamage() * (1 - (distance / 100)));
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_whirling_corruption_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_whirling_corruption_SpellScript();
    }
};

//145037
class spell_empovered_whirling_corruption : public SpellScriptLoader
{
public:
    spell_empovered_whirling_corruption() : SpellScriptLoader("spell_empovered_whirling_corruption") { }

    class spell_empovered_whirling_corruption_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_empovered_whirling_corruption_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                switch (aurEff->GetTickNumber())
                {
                case 2:
                case 4:
                case 6:
                case 8:
                case 10:
                case 12:
                    if (Unit* target = GetCaster()->ToCreature()->AI()->SelectTarget(SELECT_TARGET_FARTHEST, 0, 100.0f, true))
                        GetCaster()->CastSpell(target, SPELL_EM_WHIRLING_CORRUPTION_S, true);
                    break;
                default:
                    break;
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_empovered_whirling_corruption_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_empovered_whirling_corruption_AuraScript();
    }
};

//144616
class spell_power_iron_star : public SpellScriptLoader
{
public:
    spell_power_iron_star() : SpellScriptLoader("spell_power_iron_star") { }

    class spell_power_iron_star_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_power_iron_star_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            {
                if (Creature* ironstar = GetCaster()->FindNearestCreature(NPC_KORKRON_IRON_STAR, 30.0f, true))
                {
                    if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                        instance->SetData(DATA_FIRST_ENGENEER_DIED, 0);
                    if (GetCaster()->ToCreature())
                        GetCaster()->ToCreature()->DespawnOrUnsummon();
                    ironstar->AI()->DoAction(ACTION_LAUNCH);
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_power_iron_star_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_power_iron_star_AuraScript();
    }
};

//148299
class spell_unstable_iron_star_dummy : public SpellScriptLoader
{
public:
    spell_unstable_iron_star_dummy() : SpellScriptLoader("spell_unstable_iron_star_dummy") { }

    class spell_unstable_iron_star_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_unstable_iron_star_dummy_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetCaster())
            {
                if (GetTarget()->ToCreature()) //hit Garrosh
                    GetTarget()->AddAura(SPELL_UNSTABLE_IRON_STAR_STUN, GetTarget());
                GetCaster()->CastSpell(GetCaster(), SPELL_UNSTABLE_IRON_STAR_DMG, true);
                GetCaster()->Kill(GetCaster(), true);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_unstable_iron_star_dummy_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_unstable_iron_star_dummy_AuraScript();
    }
};

//144867
class spell_enter_realm_of_yshaarj : public SpellScriptLoader
{
public:
    spell_enter_realm_of_yshaarj() : SpellScriptLoader("spell_enter_realm_of_yshaarj") { }

    class spell_enter_realm_of_yshaarj_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_enter_realm_of_yshaarj_AuraScript);

        void HandleEffectApply(AuraEffect const * aurEff, AuraEffectHandleModes mode)
        {
            if (InstanceScript* instance = GetTarget()->GetInstanceScript())
            {
                uint8 mod = instance->GetData(DATA_GET_REALM_OF_YSHAARJ);
                GetTarget()->NearTeleportTo(tppos[mod].GetPositionX(), tppos[mod].GetPositionY(), tppos[mod].GetPositionZ(), tppos[mod].GetOrientation());
                GetTarget()->AddAura(SPELL_REALM_OF_YSHAARJ, GetTarget());
                GetTarget()->CastSpell(GetTarget(), SPELL_GARROSH_ENERGY, true);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectRemoveFn(spell_enter_realm_of_yshaarj_AuraScript::HandleEffectApply, EFFECT_2, SPELL_AURA_SCREEN_EFFECT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_enter_realm_of_yshaarj_AuraScript();
    }
};

//144954
class spell_realm_of_yshaarj : public SpellScriptLoader
{
public:
    spell_realm_of_yshaarj() : SpellScriptLoader("spell_realm_of_yshaarj") { }
    
    class spell_realm_of_yshaarj_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_realm_of_yshaarj_AuraScript);

        void HandleEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (GetTarget())
            {
                GetTarget()->NearTeleportTo(realmtppos);
                GetTarget()->RemoveAurasDueToSpell(SPELL_HOPE_BUFF);
                GetTarget()->RemoveAurasDueToSpell(SPELL_COURAGE);
                GetTarget()->RemoveAurasDueToSpell(SPELL_GARROSH_ENERGY);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_realm_of_yshaarj_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_SCREEN_EFFECT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_realm_of_yshaarj_AuraScript();
    }
};

//144852, 145222
class spell_transition_visual : public SpellScriptLoader
{
public:
    spell_transition_visual() : SpellScriptLoader("spell_transition_visual") { }

    class spell_transition_visual_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_transition_visual_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                switch (GetSpellInfo()->Id)
                {
                case SPELL_TRANSITION_VISUAL_PHASE_2:
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_INTRO_REALM_OF_YSHAARJ);
                    break;
                case SPELL_TRANSITION_VISUAL_PHASE_3:
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_PHASE_THREE);
                    break;
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_transition_visual_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_transition_visual_AuraScript();
    }
};

class EmpoweringCorruptionFilter
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (unit->ToCreature() && unit->GetEntry() == NPC_MINION_OF_YSHAARJ)
            return false;
        return true;
    }
};

//145043,  149536
class spell_empowering_corruption : public SpellScriptLoader
{
public:
    spell_empowering_corruption() : SpellScriptLoader("spell_empowering_corruption") { }

    class spell_empowering_corruption_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_empowering_corruption_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!targets.empty())
                targets.remove_if(EmpoweringCorruptionFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_empowering_corruption_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_empowering_corruption_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_empowering_corruption_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_empowering_corruption_SpellScript::FilterTargets, EFFECT_3, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_empowering_corruption_SpellScript::FilterTargets, EFFECT_4, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_empowering_corruption_SpellScript();
    }
};

class HellscreamWarsongFilter
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (unit->ToCreature())
            if (unit->GetEntry() == NPC_WARBRINGER || unit->GetEntry() == NPC_WOLF_RIDER)
                return false;
        return true;
    }
};

//144821
class spell_hellscream_warsong : public SpellScriptLoader
{
public:
    spell_hellscream_warsong() : SpellScriptLoader("spell_hellscream_warsong") { }

    class spell_hellscream_warsong_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hellscream_warsong_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!targets.empty())
                targets.remove_if(HellscreamWarsongFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hellscream_warsong_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hellscream_warsong_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hellscream_warsong_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hellscream_warsong_SpellScript();
    }
};

//145065, 145171
class spell_touch_of_yshaarj : public SpellScriptLoader
{
public:
    spell_touch_of_yshaarj() : SpellScriptLoader("spell_touch_of_yshaarj") { }

    class spell_touch_of_yshaarj_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_touch_of_yshaarj_AuraScript);

        void HandleOnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (GetTarget())
            {
                for (uint8 n = 0; n < 4; n++)
                    GetTarget()->RemoveAurasDueToSpell(auraarray[n]);

                uint32 entry = GetSpellInfo()->Id;
                if (AuraEffect* aurEffb = GetTarget()->GetAura(entry)->GetEffect(0))
                {
                    GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, SPELL_DESECRATED, true);
                    int32 newamount = int32(GetTarget()->CountPctFromMaxHealth(80));
                    aurEffb->SetAmount(newamount); 
                    if (InstanceScript* instance = GetTarget()->GetInstanceScript())
                        if (Creature* garrosh = GetTarget()->GetCreature(*GetTarget(), instance->GetGuidData(DATA_GARROSH)))
                            if (Creature* heart = garrosh->SummonCreature(NPC_HEART_OF_YSHAARJ, centerpos, TEMPSUMMON_TIMED_DESPAWN, 60000))
                                heart->AI()->SetGUID(GetTarget()->GetGUID(), 1);
                }
            }
        }

        void AfterAbsorb(AuraEffect* aurEff, DamageInfo& dmgInfo, float& absorbAmount)
        {
            if (GetTarget() && aurEff->GetAmount())
            {
                uint32 hpmod = GetTarget()->CountPctFromMaxHealth(20);
                GetTarget()->SetHealth(uint32(aurEff->GetAmount()) + hpmod);
            }
        }

        void HandleEffectRemove(AuraEffect const * aurEff, AuraEffectHandleModes mode)
        {
            if (GetTarget())
                GetTarget()->ApplySpellImmune(0, IMMUNITY_ID, SPELL_DESECRATED, false);

            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_WEAKENED_BLOWS);
                GetTarget()->SetFullHealth();
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_touch_of_yshaarj_AuraScript::HandleOnApply, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_touch_of_yshaarj_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            AfterEffectAbsorb += AuraEffectAbsorbFn(spell_touch_of_yshaarj_AuraScript::AfterAbsorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_touch_of_yshaarj_AuraScript();
    }
};

//145599
class spell_player_touch_of_yshaarj : public SpellScriptLoader
{
public:
    spell_player_touch_of_yshaarj() : SpellScriptLoader("spell_player_touch_of_yshaarj") { }

    class spell_player_touch_of_yshaarj_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_player_touch_of_yshaarj_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                if (Creature* garrosh = GetCaster()->FindNearestCreature(NPC_GARROSH, 100.0f, true))
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, garrosh, 100.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        {
                            if (!(*itr)->HasAura(SPELL_TOUCH_OF_YSHAARJ) && !(*itr)->HasAura(SPELL_EM_TOUCH_OF_YSHAARJ))
                            {
                                GetCaster()->CastSpell((*itr), garrosh->GetPower(POWER_ENERGY) >= 50 ? SPELL_EM_TOUCH_OF_YSHAARJ : SPELL_TOUCH_OF_YSHAARJ, true, 0, 0, garrosh->GetGUID());
                                break;
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_player_touch_of_yshaarj_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_player_touch_of_yshaarj_SpellScript();
    }
};

//145195
class spell_empovered_gripping_despair : public SpellScriptLoader
{
public:
    spell_empovered_gripping_despair() : SpellScriptLoader("spell_empovered_gripping_despair") { }

    class spell_empovered_gripping_despair_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_empovered_gripping_despair_AuraScript);

        void HandleEffectRemove(AuraEffect const * aurEff, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTarget())
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                {
                    float dmg = GetSpellInfo()->GetEffect(0)->BasePoints;
                    uint8 stack = aurEff->GetBase()->GetStackAmount();
                    GetTarget()->CastCustomSpell(SPELL_EXPLOSIVE_DESPAIR_DOT, SPELLVALUE_AURA_STACK, stack, GetTarget(), true, 0, 0, GetCaster()->GetGUID());
                    GetTarget()->CastCustomSpell(SPELL_EXPLOSIVE_DESPAIR_EXPLOSE, SPELLVALUE_BASE_POINT0, dmg, GetTarget(), true);
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_empovered_gripping_despair_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_empovered_gripping_despair_AuraScript();
    }
};

//147319
class spell_crushing_fear : public SpellScriptLoader
{
public:
    spell_crushing_fear() : SpellScriptLoader("spell_crushing_fear") { }

    class spell_crushing_fear_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_crushing_fear_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                uint8 mod = urand(0, 59);
                GetCaster()->CastSpell(crushingfeardest[mod].GetPositionX(), crushingfeardest[mod].GetPositionY(), crushingfeardest[mod].GetPositionZ(), SPELL_CRUSHING_FEAR_T_M, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_crushing_fear_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_crushing_fear_AuraScript();
    }
};

//147011
class spell_manifest_rage : public SpellScriptLoader
{
public:
    spell_manifest_rage() : SpellScriptLoader("spell_manifest_rage") { }

    class spell_manifest_rage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_manifest_rage_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster())
            {
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                {
                    if (Creature* realgarrosh = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(DATA_GARROSH)))
                    {
                        for (uint8 n = 0; n < 4; n++)
                        {
                            float x, y;
                            GetPosInRadiusWithRandomOrientation(GetCaster(), float(urand(20, 40)), x, y);
                            if (Creature* mof = realgarrosh->SummonCreature(NPC_MANIFESTATION_OF_RAGE, x, y, GetCaster()->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN))
                                mof->AI()->DoZoneInCombat(mof, 300.0f);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_manifest_rage_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_manifest_rage_AuraScript();
    }
};

//147229
class spell_malice_dummy : public SpellScriptLoader
{
public:
    spell_malice_dummy() : SpellScriptLoader("spell_malice_dummy") { }

    class spell_malice_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malice_dummy_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (GetCaster())
            {
                uint8 countmod = GetCaster()->GetMap()->Is25ManRaid() ? 5 : 2;
                if (targets.size() < countmod)
                {
                    float mod = 5 * (countmod - targets.size());
                    if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                        if (Creature* garrosh = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(DATA_GARROSH_STORMWIND)))
                            GetCaster()->CastCustomSpell(SPELL_MALICIOUS_ENERGY, SPELLVALUE_BASE_POINT0, mod, garrosh, true);
                }
                else if (targets.size() > countmod)
                    targets.resize(countmod);
            }
        }

        void DealDamage()
        {
            if (GetHitUnit())
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_MALICIOUS_BLAST, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_malice_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            OnHit += SpellHitFn(spell_malice_dummy_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_malice_dummy_SpellScript();
    }
};

//147665
class spell_fixate_iron_star : public SpellScriptLoader
{
public:
    spell_fixate_iron_star() : SpellScriptLoader("spell_fixate_iron_star") { }

    class spell_fixate_iron_star_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fixate_iron_star_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE) //end time pursuit or target died
                if (GetCaster() && GetCaster()->isAlive() && GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_CHANGE_TARGET);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_fixate_iron_star_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_FIXATE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fixate_iron_star_AuraScript();
    }
};

//147120
class spell_call_bombartment : public SpellScriptLoader
{
public:
    spell_call_bombartment() : SpellScriptLoader("spell_call_bombartment") { }

    class spell_call_bombartment_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_call_bombartment_AuraScript);

        bool IfTargetHavePlayersInRange(Player* target, uint8 count, float radius)
        {
            count++;
            std::list<Player*>pllist;
            GetPlayerListInGrid(pllist, target, radius);
            if (pllist.size() >= count)
                return true;
            return false;
        }

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                {
                    if (Creature* hcannon = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(NPC_HORDE_CANNON)))
                    {
                        bool havetarget = false;
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, GetCaster(), 400.0f);
                        uint8 count = GetCaster()->GetMap()->Is25ManRaid() ? 7 : 4;
                        if (!pllist.empty())
                        {
                            if (aurEff->GetTickNumber() == 1)
                            {
                                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                                {
                                    if (IfTargetHavePlayersInRange(*itr, count, 8.0f))
                                    {
                                        havetarget = true;
                                        hcannon->SetFacingToObject(*itr);
                                        hcannon->CastSpell(*itr, SPELL_BOMBARTMENT_TR_M, true);
                                        break;
                                    }
                                }
                                if (!havetarget)
                                {
                                    std::list<Player*>::iterator itr = pllist.begin();
                                    std::advance(itr, urand(0, pllist.size() - 1));
                                    hcannon->SetFacingToObject(*itr);
                                    hcannon->CastSpell(*itr, SPELL_BOMBARTMENT_TR_M, true);
                                }
                            }
                            else
                            {
                                std::list<Player*>::iterator itr = pllist.begin();
                                std::advance(itr, urand(0, pllist.size() - 1));
                                hcannon->SetFacingToObject(*itr);
                                hcannon->CastSpell(*itr, SPELL_BOMBARTMENT_TR_M, true);
                            }
                        }
                    }
                }
            }
        }

        void _OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                uint32 tick = aurEff->GetTickNumber();
                switch (tick)
                {
                case 1:
                case 6:
                case 12:
                    if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    {
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, GetCaster(), 400.0f);
                        uint8 count = GetCaster()->GetMap()->Is25ManRaid() ? 7 : 4;
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (IfTargetHavePlayersInRange(*itr, count, 8.0f))
                                {
                                    if (Creature* garroshstormwind = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(DATA_GARROSH_STORMWIND)))
                                    {
                                        garroshstormwind->CastSpell(*itr, SPELL_FIRE_UNSTABLE_IRON_STAR);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_call_bombartment_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL); //search target to shoot
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_call_bombartment_AuraScript::_OnTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL); //search target to spawn Iron Star
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_call_bombartment_AuraScript();
    }
};

//146999
class spell_growing_power : public SpellScriptLoader
{
public:
    spell_growing_power() : SpellScriptLoader("spell_growing_power") { }

    class spell_growing_power_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_growing_power_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (GetCaster()->GetPower(POWER_ENERGY) <= 99)
                    GetCaster()->SetPower(POWER_ENERGY, GetCaster()->GetPower(POWER_ENERGY) + 1);

                if (GetCaster()->GetPower(POWER_ENERGY) == 100 && !GetCaster()->HasAura(SPELL_UNSTABLE_IRON_STAR_STUN))
                {
                    GetCaster()->SetPower(POWER_ENERGY, 0);
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_MANIFEST_RAGE);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_growing_power_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_growing_power_AuraScript();
    }
};

void AddSC_boss_garrosh_hellscream()
{
    new boss_garrosh_hellscream();
    new npc_garrosh_soldier();
    new npc_iron_star();
    new npc_unstable_iron_star();
    new npc_desecrated_weapon();
    new npc_empowered_desecrated_weapon();
    new npc_sha_vortex();
    new npc_heart_of_yshaarj();
    new npc_heart_of_yshaarj_realm();
    new npc_korkron_gunship();
    new npc_horde_cannon();
    new npc_portal_to_reality();
    new spell_exploding_iron_star();
    new spell_whirling_corruption();
    new spell_empovered_whirling_corruption();
    new spell_power_iron_star();
    new spell_unstable_iron_star_dummy();
    new spell_enter_realm_of_yshaarj();
    new spell_realm_of_yshaarj();
    new spell_transition_visual();
    new spell_empowering_corruption();
    new spell_hellscream_warsong();
    new spell_touch_of_yshaarj();
    new spell_player_touch_of_yshaarj();
    new spell_empovered_gripping_despair();
    new spell_crushing_fear();
    new spell_manifest_rage();
    new spell_malice_dummy();
    new spell_fixate_iron_star();
    new spell_call_bombartment();
    new spell_growing_power();
}
