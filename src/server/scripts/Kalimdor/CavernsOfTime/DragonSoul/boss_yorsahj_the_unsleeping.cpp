#include "CreatureTextMgr.h"
#include "Containers.h"
#include "dragon_soul.h"
#include "ObjectVisitors.hpp"

enum ScriptedTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_INTRO   = 2,
    SAY_KILL    = 3,
    SAY_SPELL   = 4,
    SAY_AGGRO_1 = 5,
    SAY_DEATH_1 = 6,
    SAY_INTRO_1 = 7,
    SAY_KILL_1  = 8,
    SAY_SPELL_1 = 9,
    ANN_COMB_1N = 10,
    ANN_COMB_2N = 11,
    ANN_COMB_3N = 12,
    ANN_COMB_4N = 13,
    ANN_COMB_5N = 14,
    ANN_COMB_6N = 15,
    ANN_COMB_1H = 16,
    ANN_COMB_2H = 17,
    ANN_COMB_3H = 18,
    ANN_COMB_4H = 19,
    ANN_COMB_5H = 20,
    ANN_COMB_6H = 21,
};

enum Spells
{
    SPELL_BERSERK                   = 26662,
    SPELL_VOID_BOLT                 = 104849,
    SPELL_VOID_BOLT_AOE             = 105416,
    SPELL_SEARING_BLOOD             = 105033,
    SPELL_DEEP_CORRUPTION           = 105171,
    SPELL_DEEP_CORRUPTION_AURA      = 103628,
    SPELL_DEEP_CORRUPTION_DMG       = 105173,
    SPELL_DIGESTIVE_ACID            = 105031,
    SPELL_DIGESTIVE_ACID_DUMMY      = 105562,
    SPELL_DIGESTIVE_ACID_AOE        = 105571,
    SPELL_DIGESTIVE_ACID_DMG        = 105573,
    SPELL_MANA_VOID_SUMMON          = 105034,
    SPELL_MANA_VOID_DUMMY_1         = 105505,
    SPELL_MANA_VOID                 = 105530,
    SPELL_MANA_VOID_DUMMY_2         = 105534,
    SPELL_MANA_VOID_DUMMY_3         = 105536,
    SPELL_MANA_DIFFUSION            = 108228, //105539,
    SPELL_CORRUPTED_MINIONS_AURA    = 105636,
    SPELL_CORRUPTED_MINIONS_SUMMON  = 105637,
    SPELL_PSYCHIC_SLICE             = 105671,

    SPELL_CRIMSON_BLOOD_OF_SHUMA    = 104897,
    SPELL_ACIDIC_BLOOD_OF_SHUMA     = 104898,
    SPELL_GLOWING_BLOOD_OF_SHUMA    = 104901,
    SPELL_BLACK_BLOOD_OF_SHUMA      = 104894,
    SPELL_SHADOWED_BLOOD_OF_SHUMA   = 104896,
    SPELL_COBALT_BLOOD_OF_SHUMA     = 105027, // 104900 ?

    SPELL_FUSING_VAPORS             = 103968,
    SPELL_FUSING_VAPORS_HEAL        = 103635,
    SPELL_FUSING_VAPORS_IMMUNE      = 105904,

    SPELL_COLOR_COMBINATION_1       = 105420, // Purple, Green, Blue : Black
    SPELL_COLOR_COMBINATION_2       = 105435, // Green, Red, Black : Blue
    SPELL_COLOR_COMBINATION_3       = 105436, // Green, Yellow, Red : Black
    SPELL_COLOR_COMBINATION_4       = 105437, // Blue, Purple, Yellow : Green
    SPELL_COLOR_COMBINATION_5       = 105439, // Blue, Black, Yellow : Purple
    SPELL_COLOR_COMBINATION_6       = 105440, // Purple, Red, Black : Yellow

    SPELL_AURA_TALL_BLUE            = 105473,
    SPELL_AURA_TALL_RED             = 105474,
    SPELL_AURA_TALL_GREEN           = 105475,
    SPELL_AURA_TALL_YELLOW          = 105476,
    SPELL_AURA_TALL_PURPLE          = 105477,
    SPELL_AURA_TALL_BLACK           = 105478,

    SPELL_SPAWNING_POOL_1           = 105600,
    SPELL_SPAWNING_POOL_2           = 105601,
    SPELL_SPAWNING_POOL_3           = 105603,

    SPELL_YORSAHJ_WHISPER_INTRO     = 109894,
    SPELL_YORSAHJ_WHISPER_AGGRO     = 109895,
    SPELL_YORSAHJ_WHISPER_DEATH     = 109896,
    SPELL_YORSAHJ_WHISPER_KILL      = 109897,
    SPELL_YORSAHJ_WHISPER_SPELL     = 109898,
};

enum Events
{
    EVENT_BERSERK           = 1,
    EVENT_VOID_BOLT         = 2,
    EVENT_CALL_BLOOD_1      = 3,
    EVENT_CALL_BLOOD_2      = 4, // after teleport
    EVENT_CONTINUE          = 5,
    EVENT_CALL_BLOOD_3      = 6,
    EVENT_SEARING_BLOOD     = 7,
    EVENT_DIGESTIVE_ACID    = 8,
    EVENT_CORRUPTED_MINIONS = 9,
    EVENT_DEEP_CORRUPTION   = 10,
    EVENT_MANA_VOID_1       = 11,
    EVENT_PSYCHIC_SLICE     = 12,
    EVENT_MANA_VOID_2       = 13,
};

enum Adds
{
    NPC_MAW_OF_SHUMA        = 55544,
    NPC_MANA_VOID           = 56231,
    NPC_FORGOTTEN_ONE       = 56265,
    NPC_CRIMSON_GLOBULE     = 55865,
    NPC_ACIDIC_GLOBULE      = 55862,
    NPC_GLOWING_GLOBULE     = 55864,
    NPC_DARK_GLOBULE        = 55867,
    NPC_SHADOWED_GLOBULE    = 55863,
    NPC_COBALT_GLOBULE      = 55866,
};

enum Actions
{
    ACTION_CONTINUE = 1,
    ACTION_RED      = 2,
    ACTION_GREEN    = 3,
    ACTION_YELLOW   = 4,
    ACTION_DARK     = 5,
    ACTION_PURPLE   = 6,
    ACTION_BLUE     = 7,
};

const Position globulesPos[6] = 
{
    {-1662.959961f, -2992.280029f, -173.52f, 3.63f}, // Crimson
    {-1723.760010f, -2935.330078f, -174.03f, 4.32f}, // Acidic
    {-1863.989990f, -2993.090088f, -174.12f, 5.86f}, // Glowing
    {-1808.229980f, -3136.739990f, -173.48f, 1.09f}, // Dark
    {-1663.890015f, -3077.129883f, -174.48f, 2.72f}, // Shadowed
    {-1722.599976f, -3137.159912f, -173.39f, 1.93f}  // Cobalt
};

const Position mawofshumaPos = {-1762.56f, -3036.65f, -116.44f, 0.0f};
const Position centerPos = {-1765.66f, -3034.35f, -182.38f, 3.52f};

class boss_yorsahj_the_unsleeping: public CreatureScript
{
    public:
        boss_yorsahj_the_unsleeping() : CreatureScript("boss_yorsahj_the_unsleeping") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<boss_yorsahj_the_unsleepingAI>(pCreature);
        }

        struct boss_yorsahj_the_unsleepingAI : public BossAI
        {
            boss_yorsahj_the_unsleepingAI(Creature* pCreature) : BossAI(pCreature, DATA_YORSAHJ)
            {             
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->setActive(true);
                bIntro = false;
                bShuma = false;
                memset(bAchieve, false, sizeof(bAchieve));
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (bIntro)
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                return;

                if (!me->IsWithinDistInMap(who,100.0f, false))
                    return;

                Talk(SAY_INTRO);
                DoCastAOE(SPELL_YORSAHJ_WHISPER_INTRO, true);
                bIntro = true;
            }

            void Reset()
            {
                _Reset();

                bShuma = false;
                bContinue = false;
                _spellId = 0;
                memset(bAchieve, false, sizeof(bAchieve));

                me->SetReactState(REACT_AGGRESSIVE);
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
            }

            void EnterCombat(Unit* who)
            {
                if (instance->GetBossState(DATA_MORCHOK) != DONE)
                {
                    EnterEvadeMode();
                    instance->DoNearTeleportPlayers(portalsPos[0]);
                    return;
                }

                Talk(SAY_AGGRO);
                DoCastAOE(SPELL_YORSAHJ_WHISPER_AGGRO, true);

                events.ScheduleEvent(EVENT_BERSERK, 10 * MINUTE * IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_VOID_BOLT, 6000);
                events.ScheduleEvent(EVENT_CALL_BLOOD_1, 22000);

                instance->SetBossState(DATA_YORSAHJ, IN_PROGRESS);

                std::list<Creature*> trashmobs;
                GetCreatureListWithEntryInGrid(trashmobs, me, NPC_CRIMSON_GLOBULE_TRASH, 150);
                GetCreatureListWithEntryInGrid(trashmobs, me, NPC_ACIDIC_GLOBULE_TRASH, 150);
                GetCreatureListWithEntryInGrid(trashmobs, me, NPC_GLOWING_GLOBULE_TRASH, 150);
                GetCreatureListWithEntryInGrid(trashmobs, me, NPC_DARK_GLOBULE_TRASH, 150);
                GetCreatureListWithEntryInGrid(trashmobs, me, NPC_SHADOWED_GLOBULE_TRASH, 150);
                GetCreatureListWithEntryInGrid(trashmobs, me, NPC_COBALT_GLOBULE_TRASH, 150);
                for (std::list<Creature*>::const_iterator itr = trashmobs.begin(); itr != trashmobs.end(); ++itr)
                    if (Creature* trash = *itr)
                        if (trash->isAlive())
                            trash->SetInCombatWithZone();
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();

                Talk(SAY_DEATH);
                DoCastAOE(SPELL_YORSAHJ_WHISPER_DEATH, true);
            }

            bool AllowAchieve(uint32 Id)
            {
                return bAchieve[Id];
            }

            void KilledUnit(Unit* victim)
            {
                if (victim && victim->GetTypeId() == TYPEID_PLAYER)
                {
                    Talk(SAY_KILL);
                    DoCastAOE(SPELL_YORSAHJ_WHISPER_KILL, true);
                }
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_CONTINUE && !bContinue)
                {
                    bContinue = true;
                    events.ScheduleEvent(EVENT_CONTINUE, 1000);
                }
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);
                if (summon->GetEntry() == NPC_MANA_VOID)
                    summon->GetMotionMaster()->MoveRandom(25.0f);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->GetDistance(me->GetHomePosition()) > 150.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK);
                            break;
                        case EVENT_VOID_BOLT:
                            if (bShuma)
                                DoCastAOE(SPELL_VOID_BOLT_AOE);
                            else
                                DoCastVictim(SPELL_VOID_BOLT);

                            events.ScheduleEvent(EVENT_VOID_BOLT, (bShuma ? 5000 : 9000));
                            break;
                        case EVENT_CALL_BLOOD_1:
                            events.CancelEvent(EVENT_VOID_BOLT);
                            events.CancelEvent(EVENT_SEARING_BLOOD);
                            events.CancelEvent(EVENT_DIGESTIVE_ACID);
                            events.CancelEvent(EVENT_CORRUPTED_MINIONS);
                            events.CancelEvent(EVENT_DEEP_CORRUPTION);
                            events.CancelEvent(EVENT_MANA_VOID_1);
                            events.CancelEvent(EVENT_MANA_VOID_2);
                            me->RemoveAura(SPELL_CRIMSON_BLOOD_OF_SHUMA);
                            me->RemoveAura(SPELL_ACIDIC_BLOOD_OF_SHUMA);
                            me->RemoveAura(SPELL_GLOWING_BLOOD_OF_SHUMA);
                            me->RemoveAura(SPELL_BLACK_BLOOD_OF_SHUMA);
                            me->RemoveAura(SPELL_SHADOWED_BLOOD_OF_SHUMA);
                            me->RemoveAura(SPELL_COBALT_BLOOD_OF_SHUMA);

                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();

                            bContinue = false;
                            bShuma = false;

                            events.ScheduleEvent(EVENT_CALL_BLOOD_2, 1000);

                            me->NearTeleportTo(centerPos.GetPositionX(), centerPos.GetPositionY(), centerPos.GetPositionZ(), centerPos.GetOrientation());
                            break;
                        case EVENT_CALL_BLOOD_2:
                        {
                            Talk(SAY_SPELL);
                            DoCastAOE(SPELL_YORSAHJ_WHISPER_SPELL, true);

                            std::list<uint32> globList;
                            _spellId = 0;
                            uint8 i = 0;
                            SelectRandomGlobules(_spellId, globList);
                            if (globList.empty() || !_spellId)
                                break;

                            for (std::list<uint32>::const_iterator itr = globList.begin(); itr != globList.end(); ++itr)
                            {
                                switch ((*itr))
                                {
                                    case NPC_CRIMSON_GLOBULE: i = 0; break;  // red
                                    case NPC_ACIDIC_GLOBULE: i = 1; break;   // green
                                    case NPC_GLOWING_GLOBULE: i = 2;break;   // yellow
                                    case NPC_DARK_GLOBULE: i = 3; break;     // black
                                    case NPC_SHADOWED_GLOBULE: i = 4; break; // purple
                                    case NPC_COBALT_GLOBULE: i = 5; break;   // blue
                                    default: return;
                                }

                                if (Creature* pGlobule = me->SummonCreature((*itr), globulesPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                {
                                    pGlobule->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                    pGlobule->SetDisplayId(11686);
                                }
                            }
                            if (!IsHeroic())
                            {
                                if (HasGlobule(globList, NPC_COBALT_GLOBULE)   && HasGlobule(globList, NPC_DARK_GLOBULE)    && HasGlobule(globList, NPC_GLOWING_GLOBULE))
                                    Talk(ANN_COMB_1N);
                                if (HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_ACIDIC_GLOBULE)  && HasGlobule(globList, NPC_COBALT_GLOBULE) )
                                    Talk(ANN_COMB_2N);
                                if (HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_CRIMSON_GLOBULE) && HasGlobule(globList, NPC_DARK_GLOBULE)   )
                                    Talk(ANN_COMB_3N);
                                if (HasGlobule(globList, NPC_ACIDIC_GLOBULE)   && HasGlobule(globList, NPC_GLOWING_GLOBULE) && HasGlobule(globList, NPC_CRIMSON_GLOBULE))
                                    Talk(ANN_COMB_4N);
                                if (HasGlobule(globList, NPC_ACIDIC_GLOBULE)   && HasGlobule(globList, NPC_CRIMSON_GLOBULE) && HasGlobule(globList, NPC_DARK_GLOBULE)   )
                                    Talk(ANN_COMB_5N);
                                if (HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_COBALT_GLOBULE)  && HasGlobule(globList, NPC_GLOWING_GLOBULE))
                                    Talk(ANN_COMB_6N);
                            }
                            else
                            {
                                if (HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_CRIMSON_GLOBULE)  && HasGlobule(globList, NPC_GLOWING_GLOBULE)  && HasGlobule(globList, NPC_DARK_GLOBULE)   )
                                    Talk(ANN_COMB_1H);
                                if (HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_ACIDIC_GLOBULE)   && HasGlobule(globList, NPC_DARK_GLOBULE)     && HasGlobule(globList, NPC_COBALT_GLOBULE) )
                                    Talk(ANN_COMB_2H);
                                if (HasGlobule(globList, NPC_COBALT_GLOBULE)   && HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_ACIDIC_GLOBULE)   && HasGlobule(globList, NPC_GLOWING_GLOBULE))
                                    Talk(ANN_COMB_3H);
                                if (HasGlobule(globList, NPC_COBALT_GLOBULE)   && HasGlobule(globList, NPC_DARK_GLOBULE)     && HasGlobule(globList, NPC_SHADOWED_GLOBULE) && HasGlobule(globList, NPC_GLOWING_GLOBULE))
                                    Talk(ANN_COMB_4H);
                                if (HasGlobule(globList, NPC_ACIDIC_GLOBULE)   && HasGlobule(globList, NPC_CRIMSON_GLOBULE)  && HasGlobule(globList, NPC_COBALT_GLOBULE)   && HasGlobule(globList, NPC_DARK_GLOBULE)   )
                                    Talk(ANN_COMB_5H);
                                if (HasGlobule(globList, NPC_ACIDIC_GLOBULE)   && HasGlobule(globList, NPC_GLOWING_GLOBULE)  && HasGlobule(globList, NPC_DARK_GLOBULE)     && HasGlobule(globList, NPC_CRIMSON_GLOBULE))
                                    Talk(ANN_COMB_6H);
                            }
                            events.ScheduleEvent(EVENT_CALL_BLOOD_3, 1000);
                            break;
                        }
                        case EVENT_CALL_BLOOD_3:
                            DoCast(me, _spellId);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_SPELL_CHANNEL_OMNI);
                            break;
                        case EVENT_CONTINUE:
                        {
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                            std::list<Creature*> creatures;
                            GlobulesCheck checker;
                            Trinity::CreatureListSearcher<GlobulesCheck> searcher(me, creatures, checker);
                            Trinity::VisitNearbyObject(me, 100.0f, searcher);

                            if (!creatures.empty())
                            {
                                for (std::list<Creature*>::const_iterator itr = creatures.begin(); itr != creatures.end(); ++itr)
                                {
                                    switch ((*itr)->GetEntry())
                                    {
                                        case NPC_CRIMSON_GLOBULE: 
                                            DoCast(me, SPELL_CRIMSON_BLOOD_OF_SHUMA, true); 
                                            events.ScheduleEvent(EVENT_SEARING_BLOOD, urand(5000, 7000));
                                            break;
                                        case NPC_ACIDIC_GLOBULE: 
                                            me->SummonCreature(NPC_MAW_OF_SHUMA, mawofshumaPos, TEMPSUMMON_TIMED_DESPAWN, 60000);
                                            DoCast(me, SPELL_ACIDIC_BLOOD_OF_SHUMA, true);
                                            DoCast(me, SPELL_DIGESTIVE_ACID_DUMMY, true);
                                            events.ScheduleEvent(EVENT_DIGESTIVE_ACID, urand(7000, 9000));
                                            break;
                                        case NPC_GLOWING_GLOBULE: 
                                            bShuma = true;
                                            DoCast(me, SPELL_GLOWING_BLOOD_OF_SHUMA, true); 
                                            break;
                                        case NPC_DARK_GLOBULE: 
                                            DoCast(me, SPELL_BLACK_BLOOD_OF_SHUMA, true); 
                                            events.ScheduleEvent(EVENT_CORRUPTED_MINIONS, 1000);
                                            break;
                                        case NPC_SHADOWED_GLOBULE: 
                                            DoCast(me, SPELL_SHADOWED_BLOOD_OF_SHUMA, true);
                                            events.ScheduleEvent(EVENT_DEEP_CORRUPTION, urand(3000, 4000));
                                            break;
                                        case NPC_COBALT_GLOBULE: 
                                            DoCast(me, SPELL_COBALT_BLOOD_OF_SHUMA, true);
                                            events.ScheduleEvent(EVENT_MANA_VOID_1, urand(2000, 3000));
                                            break;
                                        default: break;
                                    }
                                    (*itr)->DespawnOrUnsummon();
                                }
                            }

                            if (me->HasAura(SPELL_BLACK_BLOOD_OF_SHUMA) && me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA))
                                bAchieve[0] = true;
                            else if (me->HasAura(SPELL_CRIMSON_BLOOD_OF_SHUMA) && me->HasAura(SPELL_ACIDIC_BLOOD_OF_SHUMA))
                                bAchieve[1] = true;
                            else if (me->HasAura(SPELL_BLACK_BLOOD_OF_SHUMA) && me->HasAura(SPELL_COBALT_BLOOD_OF_SHUMA))
                                bAchieve[2] = true;
                            else if (me->HasAura(SPELL_SHADOWED_BLOOD_OF_SHUMA) && me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA))
                                bAchieve[3] = true;

                            me->SetReactState(REACT_AGGRESSIVE);
                            AttackStart(me->getVictim());
                            events.ScheduleEvent(EVENT_VOID_BOLT, urand(6000, 7000));
                            events.ScheduleEvent(EVENT_CALL_BLOOD_1, 62000);
                            break;
                        }
                        case EVENT_SEARING_BLOOD:
                            me->CastCustomSpell(SPELL_SEARING_BLOOD, SPELLVALUE_MAX_TARGETS, RAID_MODE(3, 8), me);
                            events.ScheduleEvent(EVENT_SEARING_BLOOD, (bShuma ? 3500 : 6000));
                            break;
                        case EVENT_DIGESTIVE_ACID:
                            DoCast(me, SPELL_DIGESTIVE_ACID);
                            events.ScheduleEvent(EVENT_DIGESTIVE_ACID, (bShuma ? 3500 : 8300));
                            break;
                        case EVENT_CORRUPTED_MINIONS:
                            DoCast(me, SPELL_SPAWNING_POOL_1, true);
                            DoCast(me, SPELL_SPAWNING_POOL_2, true);
                            DoCast(me, SPELL_SPAWNING_POOL_3, true);
                            DoCast(me, SPELL_CORRUPTED_MINIONS_AURA);
                            if (bShuma)
                                events.ScheduleEvent(EVENT_CORRUPTED_MINIONS, 40000);
                            break;
                        case EVENT_DEEP_CORRUPTION:
                            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DEEP_CORRUPTION_AURA);
                            DoCastAOE(SPELL_DEEP_CORRUPTION);
                            events.ScheduleEvent(EVENT_DEEP_CORRUPTION, 25000);
                            break;
                        case EVENT_MANA_VOID_1:
                            DoCastAOE(SPELL_MANA_VOID);
                            events.ScheduleEvent(EVENT_MANA_VOID_2, 5000);
                            break;
                        case EVENT_MANA_VOID_2:
                            DoCast(me, SPELL_MANA_VOID_SUMMON);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            bool bIntro;
            uint32 _spellId;
            bool bContinue;
            bool bShuma;
            bool bAchieve[4];

            // Returns spell for animation
            void SelectRandomGlobules(uint32 &spellId, std::list<uint32> &entryList)
            {
                switch (urand(0, 5))
                {
                    case 0:
                        entryList.push_back(NPC_SHADOWED_GLOBULE);
                        entryList.push_back(NPC_ACIDIC_GLOBULE);
                        entryList.push_back(NPC_COBALT_GLOBULE);
                        if (IsHeroic())
                            entryList.push_back(NPC_DARK_GLOBULE);
                        spellId = SPELL_COLOR_COMBINATION_1;
                        break;
                    case 1:
                        entryList.push_back(NPC_ACIDIC_GLOBULE);
                        entryList.push_back(NPC_CRIMSON_GLOBULE);
                        entryList.push_back(NPC_DARK_GLOBULE);
                        if (IsHeroic())
                            entryList.push_back(NPC_COBALT_GLOBULE);
                        spellId = SPELL_COLOR_COMBINATION_2;
                        break;
                    case 2:
                        entryList.push_back(NPC_ACIDIC_GLOBULE);
                        entryList.push_back(NPC_GLOWING_GLOBULE);
                        entryList.push_back(NPC_CRIMSON_GLOBULE);
                        if (IsHeroic())
                            entryList.push_back(NPC_DARK_GLOBULE);
                        spellId = SPELL_COLOR_COMBINATION_3;
                        break;
                    case 3:
                        entryList.push_back(NPC_COBALT_GLOBULE);
                        entryList.push_back(NPC_SHADOWED_GLOBULE);
                        entryList.push_back(NPC_GLOWING_GLOBULE);
                        if (IsHeroic())
                            entryList.push_back(NPC_ACIDIC_GLOBULE);
                        spellId = SPELL_COLOR_COMBINATION_4;
                        break;
                    case 4:
                        entryList.push_back(NPC_COBALT_GLOBULE);
                        entryList.push_back(NPC_DARK_GLOBULE);
                        entryList.push_back(NPC_GLOWING_GLOBULE);
                        if (IsHeroic())
                            entryList.push_back(NPC_SHADOWED_GLOBULE);
                        spellId = SPELL_COLOR_COMBINATION_5;
                        break;
                    case 5:
                        entryList.push_back(NPC_SHADOWED_GLOBULE);
                        entryList.push_back(NPC_CRIMSON_GLOBULE);
                        entryList.push_back(NPC_DARK_GLOBULE);
                        if (IsHeroic())
                            entryList.push_back(NPC_GLOWING_GLOBULE);
                        spellId = SPELL_COLOR_COMBINATION_6;
                        break;
                    default:
                        break;
                }
            }
            bool HasGlobule(std::list<uint32>& globList, uint32 entry)
            {
                for (std::list<uint32>::const_iterator itr = globList.begin(); itr != globList.end(); ++itr)
                    if (*itr == entry)
                        return true;
                return false;
            }

            class GlobulesCheck
            {
                public:
                    GlobulesCheck() {}

                    bool operator()(Creature* u)
                    {
                        if ((u->GetEntry() == NPC_CRIMSON_GLOBULE || 
                            u->GetEntry() == NPC_ACIDIC_GLOBULE ||
                            u->GetEntry() == NPC_GLOWING_GLOBULE ||
                            u->GetEntry() == NPC_DARK_GLOBULE || 
                            u->GetEntry() == NPC_SHADOWED_GLOBULE ||
                            u->GetEntry() == NPC_COBALT_GLOBULE) && 
                            u->isAlive())
                            return true;
                        return false;
                    }
            };
        };
};

class npc_yorsahj_the_unsleeping_globule: public CreatureScript
{
    public:
        npc_yorsahj_the_unsleeping_globule() : CreatureScript("npc_yorsahj_the_unsleeping_globule") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_yorsahj_the_unsleeping_globuleAI>(pCreature);
        }

        struct npc_yorsahj_the_unsleeping_globuleAI : public Scripted_NoMovementAI
        {
            npc_yorsahj_the_unsleeping_globuleAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {             
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->setActive(true);
                bDespawn = false;
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* /*owner*/)
            {
                switch(me->GetEntry())
                { 
                    case NPC_CRIMSON_GLOBULE: DoCast(me, SPELL_AURA_TALL_RED, true); break;
                    case NPC_ACIDIC_GLOBULE: DoCast(me, SPELL_AURA_TALL_GREEN, true); break;
                    case NPC_GLOWING_GLOBULE: DoCast(me, SPELL_AURA_TALL_YELLOW, true); break;
                    case NPC_DARK_GLOBULE: DoCast(me, SPELL_AURA_TALL_BLACK, true); break;
                    case NPC_SHADOWED_GLOBULE: DoCast(me, SPELL_AURA_TALL_PURPLE, true); break;
                    case NPC_COBALT_GLOBULE: DoCast(me, SPELL_AURA_TALL_BLUE, true); break;
                    default: break;
                }
            }

            void SpellHit(Unit* /*who*/, const SpellInfo* spellInfo)
            {
                if (spellInfo->Id == SPELL_COLOR_COMBINATION_1 ||
                    spellInfo->Id == SPELL_COLOR_COMBINATION_2 ||
                    spellInfo->Id == SPELL_COLOR_COMBINATION_3 ||
                    spellInfo->Id == SPELL_COLOR_COMBINATION_4 ||
                    spellInfo->Id == SPELL_COLOR_COMBINATION_5 ||
                    spellInfo->Id == SPELL_COLOR_COMBINATION_6)
                {
                    events.ScheduleEvent(EVENT_CONTINUE, 2000);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCastAOE(SPELL_FUSING_VAPORS_IMMUNE, true);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (!bDespawn)
                {
                    if (me->isMoving())
                    {
                        float z = me->GetPositionZ();
                        me->UpdateAllowedPositionZ(me->GetPositionX(), me->GetPositionY(), z);
                        me->UpdatePosition(me->GetPositionX(), me->GetPositionY(), z, me->GetOrientation(), false);
                    }
                    if (Creature* pYorsahj = me->FindNearestCreature(NPC_YORSAHJ, 1.0f))
                    {
                        bDespawn = true;
                        pYorsahj->AI()->DoAction(ACTION_CONTINUE);
                        //me->DespawnOrUnsummon(100);
                    }
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CONTINUE:
                            me->SetSpeed(MOVE_RUN, 0.47142876f, true);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            me->SetDisplayId(me->GetNativeDisplayId());
                            DoCast(me, SPELL_FUSING_VAPORS, true);
                            if (Creature* pYorsahj = me->FindNearestCreature(NPC_YORSAHJ, 200.0f))
                                me->GetMotionMaster()->MoveFollow(pYorsahj, 0.0f, 0.0f);
                            break;
                        default:
                            break;
                    }
                }
            }
        private:
            EventMap events;
            bool bDespawn;
        };
};

class npc_yorsahj_the_unsleeping_forgotten_one: public CreatureScript
{
    public:
        npc_yorsahj_the_unsleeping_forgotten_one() : CreatureScript("npc_yorsahj_the_unsleeping_forgotten_one") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_yorsahj_the_unsleeping_forgotten_oneAI>(pCreature);
        }

        struct npc_yorsahj_the_unsleeping_forgotten_oneAI : public ScriptedAI
        {
            npc_yorsahj_the_unsleeping_forgotten_oneAI(Creature* pCreature) : ScriptedAI(pCreature)
            {             
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                me->setActive(true);
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* /*owner*/)
            {
                events.ScheduleEvent(EVENT_CONTINUE, 1000);
            }

            void JustDied(Unit* /*killer*/)
            {
                me->DespawnOrUnsummon(3000);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CONTINUE:
                            me->SetReactState(REACT_AGGRESSIVE);
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                me->AddThreat(pTarget, 1000000.0f);
                                AttackStart(pTarget);
                                events.ScheduleEvent(EVENT_PSYCHIC_SLICE, urand(6000, 20000));
                            }
                            break;
                        case EVENT_PSYCHIC_SLICE:
                            DoCastVictim(SPELL_PSYCHIC_SLICE);
                            events.ScheduleEvent(EVENT_PSYCHIC_SLICE, urand(15000, 20000));
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            EventMap events;
        };
};

class npc_yorsahj_the_unsleeping_mana_void: public CreatureScript
{
    public:
        npc_yorsahj_the_unsleeping_mana_void() : CreatureScript("npc_yorsahj_the_unsleeping_mana_void") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_yorsahj_the_unsleeping_mana_voidAI>(pCreature);
        }

        struct npc_yorsahj_the_unsleeping_mana_voidAI : public Scripted_NoMovementAI
        {
            npc_yorsahj_the_unsleeping_mana_voidAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {             
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->setActive(true);
                me->SetReactState(REACT_PASSIVE);
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCastAOE(SPELL_MANA_DIFFUSION, true);
                me->DespawnOrUnsummon(1000);
            }
        };
};

class spell_yorsahj_the_unsleeping_whisper : public SpellScriptLoader
{
    public:
        spell_yorsahj_the_unsleeping_whisper() : SpellScriptLoader("spell_yorsahj_the_unsleeping_whisper") { }

        class spell_yorsahj_the_unsleeping_whisper_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yorsahj_the_unsleeping_whisper_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (!GetCaster()->ToCreature() || !GetHitUnit()->ToPlayer())
                    return;

                uint32 textId = 0;

                switch (GetSpellInfo()->Id)
                {
                    case SPELL_YORSAHJ_WHISPER_AGGRO: textId = SAY_AGGRO_1;  break;
                    case SPELL_YORSAHJ_WHISPER_DEATH: textId = SAY_DEATH_1; break;
                    case SPELL_YORSAHJ_WHISPER_INTRO: textId = SAY_INTRO_1; break;
                    case SPELL_YORSAHJ_WHISPER_KILL: textId = SAY_KILL_1; break;
                    case SPELL_YORSAHJ_WHISPER_SPELL: textId = SAY_SPELL_1; break;
                    default: return;
                }

                sCreatureTextMgr->SendChat(GetCaster()->ToCreature(), textId, GetHitUnit()->GetGUID(), CHAT_MSG_MONSTER_WHISPER, LANG_ADDON, TEXT_RANGE_AREA);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_yorsahj_the_unsleeping_whisper_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yorsahj_the_unsleeping_whisper_SpellScript();
        }
};

class spell_yorsahj_the_unsleeping_deep_corruption : public SpellScriptLoader
{
    public:
        spell_yorsahj_the_unsleeping_deep_corruption() : SpellScriptLoader("spell_yorsahj_the_unsleeping_deep_corruption") { }

        class spell_yorsahj_the_unsleeping_deep_corruption_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_yorsahj_the_unsleeping_deep_corruption_AuraScript);

            void HandlePeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster() || !GetUnitOwner())
                    return;

                if (Aura* aur = GetAura())
                {
                    if (aur->GetStackAmount() >= 5)
                    {
                        uint32 spellId = SPELL_DEEP_CORRUPTION_DMG;

                        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                        {
                            if (ObjectGuid guid = instance->GetGuidData(DATA_YORSAHJ))
                                GetUnitOwner()->CastSpell(GetUnitOwner(), spellId, true, 0, NULL, guid);
                        }
                        else
                            GetUnitOwner()->CastSpell(GetUnitOwner(), spellId, true);

                        aur->Remove();
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_yorsahj_the_unsleeping_deep_corruption_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_yorsahj_the_unsleeping_deep_corruption_AuraScript();
        }
};

class spell_yorsahj_the_unsleeping_deep_corruption_dmg : public SpellScriptLoader
{
    public:
        spell_yorsahj_the_unsleeping_deep_corruption_dmg() : SpellScriptLoader("spell_yorsahj_the_unsleeping_deep_corruption_dmg") { }

        class spell_yorsahj_the_unsleeping_deep_corruption_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yorsahj_the_unsleeping_deep_corruption_dmg_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (GetCaster())
                    targets.push_back(GetCaster());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yorsahj_the_unsleeping_deep_corruption_dmg_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yorsahj_the_unsleeping_deep_corruption_dmg_SpellScript();
        }
};

class spell_yorsahj_the_unsleeping_digestive_acid_aoe : public SpellScriptLoader
{
    public:
        spell_yorsahj_the_unsleeping_digestive_acid_aoe() : SpellScriptLoader("spell_yorsahj_the_unsleeping_digestive_acid_aoe") { }

        class spell_yorsahj_the_unsleeping_digestive_acid_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yorsahj_the_unsleeping_digestive_acid_aoe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;

                if (Creature* pYorsahj = GetCaster()->FindNearestCreature(NPC_YORSAHJ, 200.0f))
                    if (Unit* pTank = pYorsahj->getVictim())
                        targets.remove(pTank);

                uint32 max_targets = (GetCaster()->GetMap()->Is25ManRaid() ? 8 : 4);
                Trinity::Containers::RandomResizeList(targets, max_targets);
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_DIGESTIVE_ACID_DMG, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yorsahj_the_unsleeping_digestive_acid_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_yorsahj_the_unsleeping_digestive_acid_aoe_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yorsahj_the_unsleeping_digestive_acid_aoe_SpellScript();
        }
};

class spell_yorsahj_the_unsleeping_mana_void : public SpellScriptLoader
{
    public:
        spell_yorsahj_the_unsleeping_mana_void() : SpellScriptLoader("spell_yorsahj_the_unsleeping_mana_void") { }

        class spell_yorsahj_the_unsleeping_mana_void_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_yorsahj_the_unsleeping_mana_void_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;

                targets.remove_if(ManaCheck());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yorsahj_the_unsleeping_mana_void_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_yorsahj_the_unsleeping_mana_void_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }

        private:
            class ManaCheck
            {
                public:
                    ManaCheck() {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (unit->GetTypeId() != TYPEID_PLAYER)
                            return true;
                        return (unit->ToPlayer()->getPowerType() != POWER_MANA);
                    }
            };

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_yorsahj_the_unsleeping_mana_void_SpellScript();
        }
};

typedef boss_yorsahj_the_unsleeping::boss_yorsahj_the_unsleepingAI YorsahjAI;

class achievement_taste_the_rainbow : public AchievementCriteriaScript
{
    public:
        achievement_taste_the_rainbow(char const* scriptName, uint32 Id) : AchievementCriteriaScript(scriptName), _Id(Id) { }

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (YorsahjAI* jorsahjAI = CAST_AI(YorsahjAI, target->GetAI()))
                return jorsahjAI->AllowAchieve(_Id);

            return false;
        }

    private:
        uint32 _Id;
};

void AddSC_boss_yorsahj_the_unsleeping()
{
    new boss_yorsahj_the_unsleeping();
    new npc_yorsahj_the_unsleeping_globule();
    new npc_yorsahj_the_unsleeping_forgotten_one();
    new npc_yorsahj_the_unsleeping_mana_void();
    new spell_yorsahj_the_unsleeping_whisper();
    new spell_yorsahj_the_unsleeping_deep_corruption();
    new spell_yorsahj_the_unsleeping_deep_corruption_dmg();
    new spell_yorsahj_the_unsleeping_digestive_acid_aoe();
    new spell_yorsahj_the_unsleeping_mana_void();
    new achievement_taste_the_rainbow("achievement_taste_the_rainbow_BY", 0);
    new achievement_taste_the_rainbow("achievement_taste_the_rainbow_RG", 1);
    new achievement_taste_the_rainbow("achievement_taste_the_rainbow_BB", 2);
    new achievement_taste_the_rainbow("achievement_taste_the_rainbow_PY", 3);
}