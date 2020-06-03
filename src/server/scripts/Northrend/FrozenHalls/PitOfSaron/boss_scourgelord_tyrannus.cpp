#include "pit_of_saron.h"
#include "LFGMgr.h"
#include "Group.h"

enum Yells
{
    SAY_AMBUSH_1                    = -1658050,
    SAY_AMBUSH_2                    = -1658051,
    SAY_GAUNTLET_START              = -1658052,
    SAY_TYRANNUS_INTRO_1            = -1658053,
    SAY_GORKUN_INTRO_2              = -1658054,
    SAY_TYRANNUS_INTRO_3            = -1658055,

    SAY_AGGRO                       = -1658056,
    SAY_SLAY_1                      = -1658057,
    SAY_SLAY_2                      = -1658058,
    SAY_DEATH                       = -1658059,
    SAY_MARK_RIMEFANG_1             = -1658060,
    SAY_MARK_RIMEFANG_2             = -1658061,
    SAY_DARK_MIGHT_1                = -1658062,
    SAY_DARK_MIGHT_2                = -1658063,

    SAY_GORKUN_OUTRO_1              = -1658064,
    SAY_GORKUN_OUTRO_2              = -1658065,
    SAY_JAYNA_OUTRO_3               = -1658066,
    SAY_SYLVANAS_OUTRO_3            = -1658067,
    SAY_JAYNA_OUTRO_4               = -1658068,
    SAY_SYLVANAS_OUTRO_4            = -1658069,
    SAY_JAYNA_OUTRO_5               = -1658070,

    
    SAY_GAUNTLET1                   = -1610046,
    SAY_GAUNTLET2                   = -1610048,
    SAY_TUNNEL                      = -1610083,
    SAY_OUTRO1_SLAVE                = -1658071,
    SAY_OUTRO2_SLAVE                = -1658072,
    SAY_OUTRO3_ALY                  = -1658073,
    SAY_OUTRO3_HORDE                = -1658074,
    
    NPC_WRATHBRINGER                = 36840,
    NPC_FLAMEBEARER                 = 36893,
    NPC_DEATHBRINGER                = 36892,
    // another 2 waves
    NPC_FALLEN_WARRIOR              = 36841,
    NPC_WRATHBONE_COLDWRAITH        = 36842,
    NPC_WRATHBONE_SORCERER          = 37728,    // this is for the end event, not used
    NPC_GLACIAL_REVENANT            = 36874,
    NPC_COLLAPSING_ICICLE           = 36847,
    
    NPC_SLAVE_HORDE_1               = 37578,
    NPC_SLAVE_HORDE_2               = 37577,
    NPC_SLAVE_HORDE_3               = 37579,
    NPC_SLAVE_ALY_1                 = 37572,
    NPC_SLAVE_ALY_2                 = 37575,
    NPC_SLAVE_ALY_3                 = 37576,
    NPC_SINDRAGOSA                  = 37755,
    NPC_KALIRA                      = 37583,
    //NPC_ELANDRA                     = 37774,
    NPC_LORALEN                     = 37779,
    NPC_KORELN                      = 37582,
};

enum Spells
{
    SPELL_OVERLORD_BRAND            = 69172,
    SPELL_OVERLORD_BRAND_HEAL       = 69190,
    SPELL_OVERLORD_BRAND_DAMAGE     = 69189,
    SPELL_FORCEFUL_SMASH            = 69155,
    SPELL_UNHOLY_POWER              = 69167,
    SPELL_MARK_OF_RIMEFANG          = 69275,
    SPELL_HOARFROST                 = 69246,

    SPELL_ICY_BLAST                 = 69232,
    SPELL_ICY_BLAST_AURA            = 69238,

    SPELL_EJECT_ALL_PASSENGERS      = 50630,
    SPELL_FULL_HEAL                 = 43979,
    SPELL_FROST_BOMB                = 70521,
};

enum Events
{
    EVENT_OVERLORD_BRAND    = 1,
    EVENT_FORCEFUL_SMASH    = 2,
    EVENT_UNHOLY_POWER      = 3,
    EVENT_MARK_OF_RIMEFANG  = 4,

    // Rimefang
    EVENT_MOVE_NEXT         = 5,
    EVENT_HOARFROST         = 6,
    EVENT_ICY_BLAST         = 7,

    EVENT_INTRO_1           = 8,
    EVENT_INTRO_2           = 9,
    EVENT_INTRO_3           = 10,
    EVENT_COMBAT_START      = 11,
};

enum Phases
{
    PHASE_NONE      = 0,
    PHASE_INTRO     = 1,
    PHASE_COMBAT    = 2,
    PHASE_OUTRO     = 3,
};

enum Actions
{
    ACTION_START_INTRO      = 1,
    ACTION_START_RIMEFANG   = 2,
    ACTION_START_OUTRO      = 3,
    ACTION_END_COMBAT       = 4,
    ACTION_GAUNTLET_START   = 5,
    EXIT_VEHICLE            = 7,
    ACTION_SPELL_BOMB       = 8,
};

#define GUID_HOARFROST 1

static const Position rimefangPos[10] =
{
    {1017.299f, 168.9740f, 642.9259f, 0.000000f},
    {1047.868f, 126.4931f, 665.0453f, 0.000000f},
    {1069.828f, 138.3837f, 665.0453f, 0.000000f},
    {1063.042f, 164.5174f, 665.0453f, 0.000000f},
    {1031.158f, 195.1441f, 665.0453f, 0.000000f},
    {1019.087f, 197.8038f, 665.0453f, 0.000000f},
    {967.6233f, 168.9670f, 665.0453f, 0.000000f},
    {969.1198f, 140.4722f, 665.0453f, 0.000000f},
    {986.7153f, 141.6424f, 665.0453f, 0.000000f},
    {1012.601f, 142.4965f, 665.0453f, 0.000000f},
};

//Positional defines 
struct LocationsXY
{
    float x, y, z, o;
    uint32 id;
};

static LocationsXY GauntletLoc[]=
{
    // first & second wave
    {939.545f, 79.563f, 564.941f, 3.3f},    // 0
    {936.182f, 68.153f, 565.948f, 3.3f},
    {932.409f, 89.724f, 563.709f, 3.3f},
    {942.788f, 85.200f, 565.518f, 3.3f},
    {939.697f, 67.286f, 566.268f, 3.3f},
    // third & forth wave
    {930.037f, -30.718f, 589.314f, 1.45f},    // 5
    {935.862f, -26.780f, 589.284f, 1.45f},
    {919.687f, -23.890f, 585.659f, 1.45f},
    {924.874f, -30.458f, 588.207f, 1.45f},
    {932.928f, -30.944f, 590.035f, 1.45f},
};

static LocationsXY TunnelLoc[]=
{
    // used for summoning skeletons and for icicles
    {953.161f, -106.254f, 594.972f, 4.45f},
    {968.903f, -119.275f, 598.156f, 4.45f},
    {1014.822f,-132.030f, 622.475f, 4.45f},
    {1044.323f,-111.136f, 629.631f, 4.45f},
    {1053.802f, -93.590f, 632.728f, 4.45f},
    {1061.800f, -68.209f, 633.955f, 4.45f},
    {1069.361f, -33.613f, 633.624f, 4.45f},
    {1073.614f,  -9.413f, 633.548f, 4.45f},    //7 revenant
    {1074.075f,  37.676f, 629.672f, 4.45f},
    {1066.194f,  75.234f, 630.872f, 4.45f},
};

static LocationsXY MoveLoc2[]=
{
    {1019.006f, 129.684f, 628.156f}, 
    {1003.889f, 159.652f, 628.159f},
    {1015.389f, 183.650f, 628.156f},
    {1065.827f, 210.836f, 628.156f},
    {1072.659f, 204.432f, 628.156f},
};

static LocationsXY SummonLoc[]=
{
    {1060.955f, 107.274f, 628.424f},
    {1052.122f, 103.916f, 628.454f},
    {1068.363f, 110.432f, 629.009f},
};

static LocationsXY MoveLoc[]=
{
    {1019.006f, 129.684f, 628.156f}, 
    {1003.889f, 159.652f, 628.159f},
    {1015.389f, 183.650f, 628.156f},
    {1065.827f, 210.836f, 628.156f},
    {1072.659f, 204.432f, 628.156f},
};

const float RimefangSummon[4] = {1013.827f, 169.71f, 628.157f, 5.31f};
static const Position miscPos = {1018.376f, 167.2495f, 628.2811f, 0.000000f};   //tyrannus combat start position

class boss_tyrannus : public CreatureScript
{
    public:
        boss_tyrannus() : CreatureScript("boss_tyrannus") { }

        struct boss_tyrannusAI : public BossAI
        {
            boss_tyrannusAI(Creature* creature) : BossAI(creature, DATA_TYRANNUS)
            {
                instance = me->GetInstanceScript();
            }

            InstanceScript* instance;
            bool m_startGAUNTLET;
            bool m_startPhaseIntroTyr;
            uint8 m_uiGauntletPhase;
            uint8 m_uiAtackPhase;
            uint32 m_uiGauntletTimer;
            uint32 m_uiAtackTimer;
            uint32 m_uiMobsDied;
            uint32 m_uiAddEntry;
            float angle, homeX, homeY;
            ObjectGuid m_uiRimefangGuid;

            void Reset() override
            {
                _Reset();
                events.SetPhase(PHASE_NONE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                m_startGAUNTLET = false;
                m_uiGauntletTimer = 1000;
                m_uiAtackTimer = 1000;
                m_uiGauntletPhase = 0;
                m_startPhaseIntroTyr = false;
                m_uiMobsDied        = 0;
                m_uiAddEntry        = 0;
                m_uiRimefangGuid.Clear();
                m_uiAtackPhase = 0;
                angle = 0; 
                homeX = 0; 
                homeY = 0;

                if (instance->GetBossState(DATA_TYRANNUS) == DONE)
                    me->DespawnOrUnsummon();
            }

            Creature* GetRimefang()
            {
                return ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_RIMEFANG));
            }

            void EnterCombat(Unit* /*who*/) override
            {
                DoScriptText(SAY_AGGRO, me);
            }

            void AttackStart(Unit* victim) override
            {
                if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    return;

                if (!events.IsInPhase(PHASE_INTRO) && victim && me->Attack(victim, true))
                    me->GetMotionMaster()->MoveChase(victim);
            }

            void EnterEvadeMode() override
            {
                BossAI::EnterEvadeMode();

                if (Creature* rimefang = GetRimefang())
                    rimefang->AI()->EnterEvadeMode();

                me->DespawnOrUnsummon();
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                DoScriptText(SAY_DEATH, me);
                if (instance)
                {
                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    if (!players.isEmpty())
                    {
                        Player* pPlayer = players.begin()->getSource();
                        if (pPlayer && pPlayer->GetGroup())
                            if (sLFGMgr->GetQueueId(995))
                                sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 995);
                    }
                }

                // Prevent corpse despawning
                if (TempSummon* summ = me->ToTempSummon())
                    summ->SetTempSummonType(TEMPSUMMON_DEAD_DESPAWN);

                // Stop combat for Rimefang
                if (Creature* rimefang = GetRimefang())
                    rimefang->AI()->DoAction(ACTION_END_COMBAT);
            }

            void DoAction(const int32 actionId) override
            {
                if (actionId == ACTION_START_INTRO)
                {
                    instance->SetBossState(DATA_TYRANNUS, IN_PROGRESS);
                    m_startPhaseIntroTyr = true;
                    m_startGAUNTLET = false;
                }
            }
    
            void UpdateAI(uint32 diff) override
            {
                if(m_startPhaseIntroTyr)
                {
                    if(m_uiAtackTimer < diff)
                    {
                        switch(m_uiAtackPhase)
                        {
                            case 0:
                                DoScriptText(SAY_TYRANNUS_INTRO_1, me);
                                instance->SetData(DATA_GAUNTLET, DONE);
                                ++m_uiAtackPhase;
                                break;
                            case 1:
                                ++m_uiAtackPhase;
                                m_uiAtackTimer = 8000;
                                break;
                            case 2:
                                if(Creature* pMartin = GetClosestCreatureWithEntry(me, 37580, 1525.0f))
                                {
                                    DoScriptText(SAY_GORKUN_INTRO_2, pMartin);
                                }
                                if(Creature* pGorkun = GetClosestCreatureWithEntry(me, 37581, 1525.0f))
                                {
                                    DoScriptText(SAY_GORKUN_INTRO_2, pGorkun);
                                }
                                ++m_uiAtackPhase;
                                m_uiAtackTimer = 8000;
                                break;
                            case 3:    
                                DoScriptText(SAY_TYRANNUS_INTRO_3, me);
                                ++m_uiAtackPhase;
                                m_uiAtackTimer = 7000;
                                break;
                            case 4:
                                if (Creature* rimeflang = GetClosestCreatureWithEntry(me, 36661, 325.0f))
                                {
                                    rimeflang->AI()->DoAction(EXIT_VEHICLE);
                                    rimeflang->SetReactState(REACT_PASSIVE);
                                    rimeflang->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                    rimeflang->SetInCombatWithZone();
                                    rimeflang->AI()->DoAction(ACTION_START_RIMEFANG);    //set rimefang also infight
                                }
                                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                me->SetReactState(REACT_AGGRESSIVE);
                                DoCast(me, SPELL_FULL_HEAL);
                                DoZoneInCombat();
                                m_uiAtackPhase = urand(5, 8);
                                m_uiAtackTimer = 5000;
                                break;
                            case 5:
                                if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                        DoCast(target, SPELL_OVERLORD_BRAND);
                                m_uiAtackPhase = urand(6,8);
                                m_uiAtackTimer = 5000;
                                break;
                            case 6:
                                DoCastVictim(SPELL_FORCEFUL_SMASH);
                                m_uiAtackPhase = urand(7, 8);
                                m_uiAtackTimer = urand(14000,15000);
                                break;
                            case 7:
                                DoScriptText(SAY_DARK_MIGHT_1, me);
                                DoScriptText(SAY_DARK_MIGHT_2, me);
                                DoCast(me, SPELL_UNHOLY_POWER);
                                m_uiAtackPhase = urand(5, 6);
                                m_uiAtackTimer = 20000;
                                break;
                            case 8:
                                DoScriptText(SAY_MARK_RIMEFANG_1, me);
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0))
                                {
                                    DoScriptText(SAY_MARK_RIMEFANG_2, me, target);
                                    DoCast(target, SPELL_MARK_OF_RIMEFANG);
                                }
                                m_uiAtackPhase = urand(5,7);
                                m_uiAtackTimer = 20000;
                                break;
                        }
                    }
                    else m_uiAtackTimer -= diff;
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAIForInstance<boss_tyrannusAI>(creature, PoSScriptName);
        }
};

class boss_rimefang : public CreatureScript
{
    public:
        boss_rimefang() : CreatureScript("boss_rimefang") { }

        struct boss_rimefangAI : public BossAI
        {
            boss_rimefangAI(Creature* creature) : BossAI(creature, DATA_RIMEFLANG)
            {
                m_bIsRegularMode = creature->GetMap()->IsRegularDifficulty();
            }
            
            bool m_bIsRegularMode;
            
            void Reset() override
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                me->InterruptSpell(CURRENT_GENERIC_SPELL);
                me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF); 
                me->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
            }

            void DoAction(const int32 actionId) override
            {
                if (actionId == EXIT_VEHICLE)
                {
                    me->GetVehicleKit()->RemoveAllPassengers();
                }
                
                if (actionId == ACTION_START_RIMEFANG)
                {
                    _events.SetPhase(PHASE_COMBAT);
                    DoZoneInCombat();
                    _events.ScheduleEvent(EVENT_MOVE_NEXT, 500, 0, PHASE_COMBAT);
                    _events.ScheduleEvent(EVENT_ICY_BLAST, 15000, 0, PHASE_COMBAT);
                }
                if (actionId == ACTION_END_COMBAT) 
                {
                    EnterEvadeMode();
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(0,  me->GetPositionX(),me->GetPositionY(), 1645.330f);
                    me->setFaction(35);
                    me->DeleteThreatList();
                    me->RemoveAllAuras();
                    me->SetVisible(false);
                }
            }

            void SetGUID(ObjectGuid const& guid, int32 type) override
            {
                if (type == GUID_HOARFROST)
                {
                    _hoarfrostTargetGUID = guid;
                    _events.ScheduleEvent(EVENT_HOARFROST, 1000);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MOVE_NEXT:
                            if (_currentWaypoint >= 10 || _currentWaypoint == 0)
                                _currentWaypoint = 1;
                            me->GetMotionMaster()->MovePoint(0, rimefangPos[_currentWaypoint]);
                            ++_currentWaypoint;
                            _events.ScheduleEvent(EVENT_MOVE_NEXT, 2000, 0, PHASE_COMBAT);
                            break;
                        case EVENT_ICY_BLAST:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            {
                                DoCast(target, m_bIsRegularMode ? SPELL_ICY_BLAST : SPELL_ICY_BLAST_AURA);
                            }
                            _events.ScheduleEvent(EVENT_ICY_BLAST, 15000, 0, PHASE_COMBAT);
                            break;
                        case EVENT_HOARFROST:
                            if (Unit* target = me->GetUnit(*me, _hoarfrostTargetGUID))
                            {
                                DoCast(target, SPELL_HOARFROST);
                                _hoarfrostTargetGUID.Clear();
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            private:
                EventMap _events;
                ObjectGuid _hoarfrostTargetGUID;
                uint8 _currentWaypoint;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_rimefangAI(creature);
        }
};

class player_overlord_brandAI : public PlayerAI
{
    public:
        player_overlord_brandAI(Player* player) : PlayerAI(player), tyrannusGuid(ObjectGuid::Empty) {}

        ObjectGuid tyrannusGuid;

        void SetGUID(ObjectGuid const& guid, int32 /*Type = 0*/) override
        {
            tyrannusGuid = guid;
            if (!getTyrannus())
                me->IsAIEnabled = false;
        }

        Creature* getTyrannus() const
        {
            return ObjectAccessor::GetCreature(*me, tyrannusGuid);
        }

        void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
        {
            Creature* tyrannus = getTyrannus();
            if (tyrannus && tyrannus->getVictim())
                me->CastCustomSpell(SPELL_OVERLORD_BRAND_DAMAGE, SPELLVALUE_BASE_POINT0, damage, tyrannus->getVictim(), true, NULL, NULL, tyrannus->GetGUID());
        }

        void HealDone(Unit* /*target*/, uint32& addHealth) override
        {
            if (Creature* tyrannus = getTyrannus())
                me->CastCustomSpell(SPELL_OVERLORD_BRAND_HEAL, SPELLVALUE_BASE_POINT0, int32(addHealth*5.5f), tyrannus, true, NULL, NULL, tyrannus->GetGUID());
        }

        void UpdateAI(uint32 /*diff*/) override {}
};

class spell_tyrannus_overlord_brand : public SpellScriptLoader
{
    public:
        spell_tyrannus_overlord_brand() : SpellScriptLoader("spell_tyrannus_overlord_brand") { }

        class spell_tyrannus_overlord_brand_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_tyrannus_overlord_brand_AuraScript);

            bool Load() override
            {
                return GetCaster() && GetCaster()->GetEntry() == NPC_TYRANNUS;
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
                    return;

                oldAI = GetTarget()->GetAI();
                oldAIState = GetTarget()->IsAIEnabled;
                GetTarget()->SetAI(new player_overlord_brandAI(GetTarget()->ToPlayer()));
                GetTarget()->GetAI()->SetGUID(GetCasterGUID());
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
                    return;

                GetTarget()->IsAIEnabled = oldAIState;
                UnitAI* thisAI = GetTarget()->GetAI();
                GetTarget()->SetAI(oldAI);
                delete thisAI;
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_tyrannus_overlord_brand_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_tyrannus_overlord_brand_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }

            UnitAI* oldAI;
            bool oldAIState;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_tyrannus_overlord_brand_AuraScript();
        }
};

class spell_tyrannus_mark_of_rimefang : public SpellScriptLoader
{
    public:
        spell_tyrannus_mark_of_rimefang() : SpellScriptLoader("spell_tyrannus_mark_of_rimefang") { }

        class spell_tyrannus_mark_of_rimefang_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_tyrannus_mark_of_rimefang_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster() || GetCaster()->GetTypeId() != TYPEID_UNIT)
                    return;

                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* rimefang = ObjectAccessor::GetCreature(*GetCaster(), instance->GetGuidData(DATA_RIMEFANG)))
                        rimefang->AI()->SetGUID(GetTarget()->GetGUID(), GUID_HOARFROST);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_tyrannus_mark_of_rimefang_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_tyrannus_mark_of_rimefang_AuraScript();
        }
};

class at_tyrannus_event_starter : public AreaTriggerScript
{
    public:
        at_tyrannus_event_starter() : AreaTriggerScript("at_tyrannus_event_starter") { }

        bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/) override
        {
            InstanceScript* instance = player->GetInstanceScript();
            if (!instance)
                return false;

            if (instance->GetBossState(DATA_TYRANNUS) != IN_PROGRESS && instance->GetBossState(DATA_TYRANNUS) != DONE)
                if (Creature* tyrannus = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_TYRANNUS)))
                {
                    tyrannus->AI()->DoAction(ACTION_START_INTRO);
                    return true;
                }

            return false;
        }
};


class at_tyrannus_gauntlet_starter : public AreaTriggerScript
{
    public:
        at_tyrannus_gauntlet_starter() : AreaTriggerScript("at_tyrannus_gauntlet_starter") { }

        bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/) override
        {
            InstanceScript* instance = player->GetInstanceScript();
            if (!instance)
                return false;

            if (instance->GetData(DATA_GAUNTLET) != DONE && instance->GetData(DATA_GAUNTLET) != IN_PROGRESS && instance->GetBossState(DATA_ICK) != IN_PROGRESS && instance->GetBossState(DATA_ICK) != NOT_STARTED && instance->GetBossState(DATA_GARFROST) != IN_PROGRESS && instance->GetBossState(DATA_GARFROST) != NOT_STARTED && instance->GetBossState(DATA_TYRANNUS) != IN_PROGRESS && instance->GetBossState(DATA_TYRANNUS) != DONE)
                if (Creature* tyrannus = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_TYRANNUS)))
                {
                    instance->SetData(DATA_GAUNTLET, IN_PROGRESS);
                    return true;
                }

            return false;
        }
};

static LocationsXY SummonLoc2[]=
{
    {1060.955f, 107.274f, 628.424f},
    {1052.122f, 103.916f, 628.454f},
    {1068.363f, 110.432f, 629.009f},
};

class npc_martin_gorkun_end : public CreatureScript
{
public:
    npc_martin_gorkun_end() : CreatureScript("npc_martin_gorkun_end") {}

    struct npc_martin_gorkun_endAI: public ScriptedAI
    {
        npc_martin_gorkun_endAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        uint8 m_uiOutro_Phase;
        uint8 m_uiOutroEnd_Phase;
        uint32 creatureEntry;
        uint32 m_uiSpeech_Timer;

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            m_uiOutro_Phase     = 0;
            m_uiOutroEnd_Phase  = 0;
            m_uiSpeech_Timer    = 1000;
            creatureEntry = me->GetEntry();
        }

        void SummonHordeSlavesEnd()
        {
            for (uint8 i = 0; i < 1; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_1, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }

            for (uint8 i = 0; i < 5; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_3, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }

            for (uint8 i = 5; i < 10; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_2, SummonLoc2[1].x + urand(0, 10), SummonLoc2[1].y - urand(0, 10), SummonLoc2[1].z, SummonLoc2[1].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }

            for (uint8 i = 10; i < 15; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_3, SummonLoc2[2].x - urand(0, 20), SummonLoc2[2].y - urand(0, 20), SummonLoc2[2].z, SummonLoc2[2].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }
        }

        void SummonAlySlavesEnd()
        {
            for (uint8 i = 0; i < 1; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_1, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }

            for (uint8 i = 0; i < 5; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_3, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }

            for (uint8 i = 5; i < 10; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_2, SummonLoc2[1].x + urand(0, 10), SummonLoc2[1].y - urand(0, 10), SummonLoc2[1].z, SummonLoc2[1].o, TEMPSUMMON_DEAD_DESPAWN, 0);
        
            }

            for (uint8 i = 10; i < 15; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_3, SummonLoc2[2].x - urand(0, 20), SummonLoc2[2].y - urand(0, 20), SummonLoc2[2].z, SummonLoc2[2].o, TEMPSUMMON_DEAD_DESPAWN, 0);
            }
        }
    
        void SummonAlySlavesEndMove()
        {
            for (uint8 i = 0; i < 1; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_1, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[0].x + urand(0, 20), MoveLoc2[0].y + urand(0, 20), MoveLoc2[0].z);
            }

            for (uint8 i = 0; i < 5; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_3, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[2].x + urand(0, 20), MoveLoc2[2].y - urand(0, 20), MoveLoc2[2].z);
            }

            for (uint8 i = 5; i < 10; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_2, SummonLoc2[1].x + urand(0, 10), SummonLoc2[1].y - urand(0, 10), SummonLoc2[1].z, SummonLoc2[1].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[1].x - urand(0, 20), MoveLoc2[1].y - urand(0, 20), MoveLoc2[1].z);
            }

            for (uint8 i = 10; i < 15; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_ALY_3, SummonLoc2[2].x - urand(0, 20), SummonLoc2[2].y - urand(0, 20), SummonLoc2[2].z, SummonLoc2[2].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[0].x - urand(0, 20), MoveLoc2[0].y - urand(0, 20), MoveLoc2[0].z);
            }
        }

        void SummonHordeSlavesEndMove()
        {
            for (uint8 i = 0; i < 1; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_1, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[0].x + urand(0, 20), MoveLoc2[0].y + urand(0, 20), MoveLoc2[0].z);
            }

            for (uint8 i = 0; i < 5; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_3, SummonLoc2[0].x + urand(0, 20), SummonLoc2[0].y + urand(0, 20), SummonLoc2[0].z, SummonLoc2[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[2].x + urand(0, 20), MoveLoc2[2].y - urand(0, 20), MoveLoc2[2].z);
            }

            for (uint8 i = 5; i < 10; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_2, SummonLoc2[1].x + urand(0, 10), SummonLoc2[1].y - urand(0, 10), SummonLoc2[1].z, SummonLoc2[1].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[1].x - urand(0, 20), MoveLoc2[1].y - urand(0, 20), MoveLoc2[1].z);
            }

            for (uint8 i = 10; i < 15; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_3, SummonLoc2[2].x - urand(0, 20), SummonLoc2[2].y - urand(0, 20), SummonLoc2[2].z, SummonLoc2[2].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc2[0].x - urand(0, 20), MoveLoc2[0].y - urand(0, 20), MoveLoc2[0].z);
            }
        }

        void SummonAlyAssist()
        {
            if (Creature* pElandra = me->SummonCreature(NPC_ELANDRA, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[20].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pElandra->GetMotionMaster()->MovePoint(0, MoveLoc2[3].x, MoveLoc2[3].y, MoveLoc2[3].z);

            if (Creature* pKoreln = me->SummonCreature(NPC_KORELN, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[21].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pKoreln->GetMotionMaster()->MovePoint(0, MoveLoc2[4].x, MoveLoc2[4].y, MoveLoc2[4].z);
        }

        void SummonHordeAssist()
        {
            if (Creature *pLoralen = me->SummonCreature(NPC_LORALEN, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[20].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pLoralen->GetMotionMaster()->MovePoint(0, MoveLoc2[3].x, MoveLoc2[3].y, MoveLoc2[3].z);

            if (Creature *pKelira = me->SummonCreature(NPC_KALIRA, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[21].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pKelira->GetMotionMaster()->MovePoint(0, MoveLoc2[4].x, MoveLoc2[4].y, MoveLoc2[4].z);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (instance->GetBossState(DATA_TYRANNUS) == IN_PROGRESS && GetClosestCreatureWithEntry(me, 36661, 125.0f))
            {
                if (m_uiSpeech_Timer < uiDiff)
                {
                    switch(m_uiOutro_Phase)
                    {
                        case 0:
                            switch (creatureEntry)
                            {
                                case NPC_GORKUN_IRONSKULL_END:
                                    SummonHordeSlavesEnd();
                                    SummonHordeSlavesEnd();
                                    SummonHordeSlavesEnd();
                                    break;
                                case NPC_MARTIN_VICTUS_END:
                                    SummonAlySlavesEnd();
                                    SummonAlySlavesEnd();
                                    SummonAlySlavesEnd();
                                    break;
                            }
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 10000;
                            break;
                        case 1:
                        {
                            for (uint8 i = 0; i < 2; i++)
                            {
                                if (Creature* pGDeathMob = me->SummonCreature(NPC_WRATHBRINGER, 1060.955f, 107.274f, 629.424f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 10000))
                                {
                                    if(Creature* pGDeathMob2 = me->SummonCreature(NPC_WRATHBRINGER, 1073.955f, 58.274f, 630.424f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 10000))
                                        pGDeathMob2->MonsterMoveWithSpeed(1060.955f, 107.274f, 629.424f, 1);
                                
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_1, 1025.0f)) 
                                    {
                                        pGDeathMob->SetInCombatWith(pDefenderNPC);
                                        pGDeathMob->AI()->AttackStart(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_2, 1025.0f)) 
                                    {
                                        pGDeathMob->SetInCombatWith(pDefenderNPC);
                                        pGDeathMob->AI()->AttackStart(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_3, 1025.0f)) 
                                    {
                                        pGDeathMob->SetInCombatWith(pDefenderNPC);
                                        pGDeathMob->AI()->AttackStart(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_1, 1025.0f)) 
                                    {
                                        pGDeathMob->SetInCombatWith(pDefenderNPC);
                                        pGDeathMob->AI()->AttackStart(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_2, 1025.0f)) 
                                    {
                                        pGDeathMob->SetInCombatWith(pDefenderNPC);
                                        pGDeathMob->AI()->AttackStart(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_3, 1025.0f)) 
                                    {
                                        pGDeathMob->SetInCombatWith(pDefenderNPC);
                                        pGDeathMob->AI()->AttackStart(pDefenderNPC);
                                    }
                                }
                            }
                            if(Creature* pGDeathMob = GetClosestCreatureWithEntry(me, NPC_WRATHBRINGER, 200.0f)) 
                            {
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_1, 1025.0f))
                                {
                                    pDefenderNPC->SetInCombatWith(pGDeathMob);
                                    pDefenderNPC->AI()->AttackStart(pGDeathMob);
                                    pDefenderNPC->CastSpell(pDefenderNPC, 35199, true);
                                    if(Creature* pDefendersNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_2, 1025.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefendersNPC, 69569, true);
                                        pDefenderNPC->CastSpell(pDefendersNPC, 70425, true);
                                    }
                                    if(Creature* pDefendersNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_3, 1025.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefendersNPC, 69569, true);
                                        pDefenderNPC->CastSpell(pDefendersNPC, 70425, true);
                                    }
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_2, 1025.0f))
                                {
                                    pDefenderNPC->SetInCombatWith(pGDeathMob);
                                    pDefenderNPC->AI()->AttackStart(pGDeathMob);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69565, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69566, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69570, true);
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_3, 1025.0f))
                                {
                                    pDefenderNPC->SetInCombatWith(pGDeathMob);
                                    pDefenderNPC->AI()->AttackStart(pGDeathMob);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69565, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69566, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69570, true);
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_1, 1025.0f))
                                {
                                    pDefenderNPC->SetInCombatWith(pGDeathMob);
                                    pDefenderNPC->AI()->AttackStart(pGDeathMob);
                                    pDefenderNPC->CastSpell(pDefenderNPC, 35199, true);
                                    if(Creature* pDefendersNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_2, 1025.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefendersNPC, 69569, true);
                                        pDefenderNPC->CastSpell(pDefendersNPC, 70425, true);
                                    }
                                    if(Creature* pDefendersNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_3, 1025.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefendersNPC, 69569, true);
                                        pDefenderNPC->CastSpell(pDefendersNPC, 70425, true);
                                    }
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_2, 1025.0f))
                                {
                                    pDefenderNPC->SetInCombatWith(pGDeathMob);
                                    pDefenderNPC->AI()->AttackStart(pGDeathMob);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69565, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69566, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69570, true);
                                    pDefenderNPC->CastSpell(pDefenderNPC, 35199, true);
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_3, 1025.0f))
                                {
                                    pDefenderNPC->SetInCombatWith(pGDeathMob);
                                    pDefenderNPC->AI()->AttackStart(pGDeathMob);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69565, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69566, true);
                                    pDefenderNPC->CastSpell(pGDeathMob, 69570, true);
                                    pDefenderNPC->CastSpell(pDefenderNPC, 35199, true);
                                }
                            }
                            m_uiOutro_Phase=1;
                            m_uiSpeech_Timer = 10000;
                            break;
                        }
                    }
                } else m_uiSpeech_Timer -= uiDiff;
            }
            
            if (instance->GetBossState(DATA_TYRANNUS) == DONE)
            {
                if(m_uiSpeech_Timer < uiDiff)
                {
                    switch(m_uiOutroEnd_Phase)
                    {
                        case 0:
                            for(uint8 i = 0; i < 31; i++)
                            {
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_1, 1025.0f))
                                {
                                    pDefenderNPC->DespawnOrUnsummon();
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_2, 1025.0f))
                                {
                                    pDefenderNPC->DespawnOrUnsummon();
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_3, 1025.0f))
                                {
                                    pDefenderNPC->DespawnOrUnsummon();
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_1, 1025.0f))
                                {
                                    pDefenderNPC->DespawnOrUnsummon();
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_2, 1025.0f))
                                {
                                    pDefenderNPC->DespawnOrUnsummon();
                                }
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_3, 1025.0f))
                                {
                                    pDefenderNPC->DespawnOrUnsummon();
                                }
                            }
                            ++m_uiOutroEnd_Phase;
                            m_uiSpeech_Timer = 100;
                            break;
                        case 1:
                            switch (creatureEntry)
                            {
                                case NPC_MARTIN_VICTUS_END:
                                    SummonAlySlavesEndMove();
                                    SummonAlySlavesEndMove();
                                    SummonAlySlavesEndMove();
                                    DoScriptText(SAY_OUTRO1_SLAVE, me);
                                    me->GetMotionMaster()->MovePoint(0, 1014.670f, 158.714f, 628.156f);
                                    break;
                                case NPC_GORKUN_IRONSKULL_END:
                                    SummonHordeSlavesEndMove();
                                    SummonHordeSlavesEndMove();
                                    SummonHordeSlavesEndMove();
                                    DoScriptText(SAY_OUTRO1_SLAVE, me);
                                    me->GetMotionMaster()->MovePoint(0, 1014.670f, 158.714f, 628.156f);
                                    break;
                            }
                            ++m_uiOutroEnd_Phase;
                            m_uiSpeech_Timer = 7000;
                            break;
                        case 2:
                            if(Creature* pSindragosa = me->SummonCreature(NPC_SINDRAGOSA, 977.224f, 164.056f, 283.216f,  653.064f, TEMPSUMMON_TIMED_DESPAWN, 20000))
                            {
                                pSindragosa->GetMotionMaster()->MoveIdle();
                                pSindragosa->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
                                pSindragosa->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
                                pSindragosa->GetMotionMaster()->MoveIdle();
                                pSindragosa->MonsterMoveWithSpeed(977.224f, 164.056f, 653.216f, 1);
                            }
                            ++m_uiOutroEnd_Phase;
                            m_uiSpeech_Timer = 7000;
                            break;
                        case 3:
                            switch (creatureEntry)
                            {
                                case NPC_MARTIN_VICTUS_END:
                                    me->GetMotionMaster()->MovePoint(0, 1014.670f, 158.714f, 628.156f);
                                    DoScriptText(SAY_OUTRO2_SLAVE, me);
                                    m_uiSpeech_Timer = 17000;
                                    break;
                                case NPC_GORKUN_IRONSKULL_END:
                                    me->GetMotionMaster()->MovePoint(0, 1014.670f, 158.714f, 628.156f);
                                    DoScriptText(SAY_OUTRO2_SLAVE, me);
                                    m_uiSpeech_Timer = 10000;
                                    break;
                            }
                            ++m_uiOutroEnd_Phase;
                            break;
                        case 4:
                            switch (creatureEntry)
                            {
                                case 37580:
                                me->CastSpell(me, SPELL_FROST_BOMB, false);
                                for(uint8 i = 0; i < 31; i++)
                                {
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_1, 200.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                        me->Kill(pDefenderNPC);
                                        pDefenderNPC->Kill(pDefenderNPC);
                                    }
    
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_2, 200.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                        me->Kill(pDefenderNPC);
                                        pDefenderNPC->Kill(pDefenderNPC);
                                    }
    
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_3, 200.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                        pDefenderNPC->Kill(pDefenderNPC);
                                        me->Kill(pDefenderNPC);
                                    }
                                }
                                    break;
                                case NPC_GORKUN_IRONSKULL_END:
                                    for(uint8 i = 0; i < 31; i++)
                                    {
                                        if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_1, 200.0f))
                                        {
                                            me->Kill(pDefenderNPC);
                                            pDefenderNPC->Kill(pDefenderNPC);
                                        }
                                        if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_2, 200.0f))
                                        {
                                            me->Kill(pDefenderNPC);
                                            pDefenderNPC->Kill(pDefenderNPC);
                                        }
                                        if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_ALY_3, 200.0f))
                                        {
                                            me->Kill(pDefenderNPC);
                                            pDefenderNPC->Kill(pDefenderNPC);
                                        }
                                    }
                                    break;
                            }
                            ++m_uiOutroEnd_Phase;
                            m_uiSpeech_Timer = 1000;
                            break;
                        case 5:    
                            me->Kill(me);
                            ++m_uiOutroEnd_Phase;
                            m_uiSpeech_Timer = 1000;
                            break;
                    }
                } else m_uiSpeech_Timer -= uiDiff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_martin_gorkun_endAI(creature);
    }    
};
    
class boss_sindra : public CreatureScript
{
    public:
        boss_sindra() : CreatureScript("boss_sindra") { }

        struct boss_sindraAI : public BossAI
        {
            boss_sindraAI(Creature* creature) : BossAI(creature, DATA_SINDRA) {}

            uint8 m_uiOutro_Phase;
            uint32 m_uiSpeech_Timerz;

            void Reset() override
            {
                m_uiOutro_Phase      = 0;
                m_uiSpeech_Timerz    = 1000;
            }

            void TeleportPlayers()
            {
                Map* pMap = me->GetMap();
                if (pMap)
                {
                    Map::PlayerList const &lPlayers = pMap->GetPlayers();
                    if (!lPlayers.isEmpty())
                    {
                        for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                        {
                            if (Player* pPlayer = itr->getSource())
                                pPlayer->TeleportTo(me->GetMapId(), 1065.983f, 94.954f, 630.997f, 2.247f);
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (m_uiSpeech_Timerz)
                {
                    if(m_uiSpeech_Timerz <= diff)
                    {
                        switch(m_uiOutro_Phase)
                        {
                            case 0:
                                ++m_uiOutro_Phase;
                                m_uiSpeech_Timerz = 15000;
                                break;
                            case 1:
                                TeleportPlayers();
                                if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_GORKUN_IRONSKULL_END, 200.0f))
                                {
                                    pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                }
                                if(Creature* pDefenderNPCS = GetClosestCreatureWithEntry(me, 37580, 3525.0f))
                                {
                                    pDefenderNPCS->CastSpell(pDefenderNPCS, SPELL_FROST_BOMB, false);
                                }
                                for(uint8 i = 0; i < 31; i++)
                                {
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_1, 200.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                        me->Kill(pDefenderNPC);
                                        pDefenderNPC->Kill(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_2, 200.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                        me->Kill(pDefenderNPC);
                                        pDefenderNPC->Kill(pDefenderNPC);
                                    }
                                    if(Creature* pDefenderNPC = GetClosestCreatureWithEntry(me, NPC_SLAVE_HORDE_3, 200.0f))
                                    {
                                        pDefenderNPC->CastSpell(pDefenderNPC, SPELL_FROST_BOMB, false);
                                        pDefenderNPC->Kill(pDefenderNPC);
                                        me->Kill(pDefenderNPC);
                                    }
                                }
                                ++m_uiOutro_Phase;
                                m_uiSpeech_Timerz = 1000;
                                break;
                            default:
                                m_uiSpeech_Timerz = 0;
                        }
                    } else m_uiSpeech_Timerz -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_sindraAI(creature);
        }
};

class npc_sylvanas_jaina_pos_end : public CreatureScript
{
public:
    npc_sylvanas_jaina_pos_end() : CreatureScript("npc_sylvanas_jaina_pos_end") {}

    struct npc_sylvanas_jaina_pos_endAI: public ScriptedAI
    {
        npc_sylvanas_jaina_pos_endAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        uint8 m_uiOutro_Phase;
        uint8 m_uiOutroEnd_Phase;
        uint32 creatureEntry;
        uint32 m_uiSpeech_Timer;

        void Reset() override
        {
            m_uiOutro_Phase     = 0;
            m_uiOutroEnd_Phase  = 0;
            m_uiSpeech_Timer    = 1000;
            creatureEntry = me->GetEntry();
        }

        void SummonAlyAssist()
        {
            if (Creature* pElandra = me->SummonCreature(NPC_ELANDRA, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[20].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pElandra->GetMotionMaster()->MovePoint(0, MoveLoc2[3].x, MoveLoc2[3].y, MoveLoc2[3].z);

            if (Creature* pKoreln = me->SummonCreature(NPC_KORELN, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[21].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pKoreln->GetMotionMaster()->MovePoint(0, MoveLoc2[4].x, MoveLoc2[4].y, MoveLoc2[4].z);
        }

        void SummonHordeAssist()
        {
            if (Creature* pLoralen = me->SummonCreature(NPC_LORALEN, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[20].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pLoralen->GetMotionMaster()->MovePoint(0, MoveLoc2[3].x, MoveLoc2[3].y, MoveLoc2[3].z);

            if (Creature* pKelira = me->SummonCreature(NPC_KALIRA, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[21].o, TEMPSUMMON_DEAD_DESPAWN, 0))
                pKelira->GetMotionMaster()->MovePoint(0, MoveLoc2[4].x, MoveLoc2[4].y, MoveLoc2[4].z);
        }

        void UpdateAI(uint32 diff) override
        {
            if (instance->GetBossState(DATA_TYRANNUS) == DONE)
            {
                if(m_uiSpeech_Timer < diff)
                {
                    switch(m_uiOutro_Phase)
                    {
                        case 0:
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 28350;
                            break;
                        case 1:
                            if(creatureEntry==NPC_SYLVANAS_PART2)
                            {
                                DoScriptText(SAY_OUTRO3_HORDE, me);
                            }
                            if(creatureEntry==NPC_JAINA_PART2)
                            {
                                DoScriptText(SAY_OUTRO3_ALY, me);
                            }
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 11000;
                            break;
                        case 2:
                            switch (creatureEntry)
                            {
                                case NPC_JAINA_PART2:
                                    DoScriptText(SAY_JAYNA_OUTRO_4, me);
                                    me->GetMotionMaster()->MovePoint(0, 1068.709f, 208.378f, 628.156f);
                                    SummonAlyAssist();
                                    m_uiSpeech_Timer = 7000;
                                    break;
                                case NPC_SYLVANAS_PART2:
                                    DoScriptText(SAY_SYLVANAS_OUTRO_4, me);
                                    me->GetMotionMaster()->MovePoint(0, 1068.709f, 208.378f, 628.156f);
                                    SummonHordeAssist();
                                    m_uiSpeech_Timer = 7000;
                                    break;
                            }
                            ++m_uiOutro_Phase;
                            break;
                        case 3:
                            switch (creatureEntry)
                            {
                                case NPC_JAINA_PART2:
                                    DoScriptText(SAY_JAYNA_OUTRO_5, me);
                                    break;
                                case NPC_SYLVANAS_PART2:
                                    break;
                            }
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 5000;
                            break;
                        }
                } else m_uiSpeech_Timer -= diff;
            }    
        }
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sylvanas_jaina_pos_endAI(creature);
    }    
};
    
class npc_ice_mob_bomb : public CreatureScript
{
public:
    npc_ice_mob_bomb() : CreatureScript("npc_ice_mob_bomb") {}

    struct npc_ice_mob_bombAI: public ScriptedAI
    {
        npc_ice_mob_bombAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 creatureEntry;
        uint32 m_uiSpeech_Timer;
        uint8 m_uiOutro_Phase;

        void Reset() override
        {
            m_uiOutro_Phase     = 0;
            m_uiSpeech_Timer    = 1000;
        }

        void UpdateAI(uint32 diff) override
        {
            if (m_uiSpeech_Timer < diff)
            {
                switch(m_uiOutro_Phase)
                {
                    case 0:
                        DoCast(62234);
                        DoCast(69426);
                        m_uiOutro_Phase=0;
                        m_uiSpeech_Timer = 12000;
                        break;
                }
            } else m_uiSpeech_Timer -= diff;
        }

    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ice_mob_bombAI(creature);
    }    
};
    
enum 
{
    SAY_TYRANNUS_DEATH                          = -1659007,
    SPELL_FLY                                   = 59553,
    SAY_TYRANNUS_OUTRO_7                        = -1658037,
    SAY_TYRANNUS_OUTRO_9                        = -1658039,
    SPELL_NECROMANTIC_POWER                     = 32889,
};
    
static const Position outroPos[1] =
{
    {893.4820f, -57.1602f, 606.3624f, 1.57f},  // Tyrannus fly up (not sniffed)
};

class npc_tyr_event_pit : public CreatureScript
{
public:
    npc_tyr_event_pit() : CreatureScript("npc_tyr_event_pit") {}

    struct npc_tyr_event_pitAI: public ScriptedAI
    {
        npc_tyr_event_pitAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        bool checkEventIck;
        uint32 creatureEntry;
        uint32 m_uiSpeech_Timer;
        uint8 m_uiOutro_Phase;
        uint32 m_uiSpeech_Timers;
        uint8 m_uiOutro_Phases;
        uint32 m_uiSpeech_Timersq;
        uint8 m_uiOutro_Phasesq;
        uint32 m_uiSpeech_Timerss;
        uint8 m_uiOutro_Phasess;
        uint32 m_uiSpeech_Timere;
        uint32 m_uiGauntletTimer;
        uint32 m_uiAddEntry;
        uint8 m_uiOutro_Phasee;
        uint8 m_uiGauntletPhase;
        float angle, homeX, homeY;

        void Reset() override
        {
            checkEventIck = false;
            m_uiOutro_Phase = 0;
            m_uiSpeech_Timer = 1000;
            m_uiOutro_Phases = 0;
            m_uiSpeech_Timers = 1000;
            m_uiOutro_Phasesq = 0;
            m_uiSpeech_Timersq = 1000;
            m_uiOutro_Phasess = 0;
            m_uiSpeech_Timerss = 1000;
            m_uiOutro_Phasee = 0;
            m_uiSpeech_Timere = 1000;
            m_uiGauntletTimer = 1000;
            m_uiGauntletPhase = 0;
            m_uiAddEntry = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (!GetClosestCreatureWithEntry(me, 36661, 157.0f)) 
            {
                if (Creature* Tyr = GetClosestCreatureWithEntry(me, 36658, 155.0f)) 
                    Tyr->DespawnOrUnsummon();
            }

            if (instance->GetBossState(DATA_ICK) == DONE && instance->GetBossState(DATA_GARFROST) == DONE && instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED )
            {
                if(m_uiGauntletTimer < diff)
                {
                    switch(m_uiGauntletPhase)
                    {
                        case 0:
                            switch(urand(0, 1))
                            {
                                case 0: DoScriptText(SAY_GAUNTLET1, me); break;
                                case 1: DoScriptText(SAY_GAUNTLET2, me); break;
                            }
                            // summon first wave and set in combat
                            if(Creature* pTemp = me->SummonCreature(NPC_DEATHBRINGER, 888.0f, 67.0f, 534.5f, GauntletLoc[0].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_WRATHBRINGER, 888.0f, 63.0f, 534.5f, GauntletLoc[1].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_WRATHBRINGER, 888.0f, 60.0f, 534.5f, GauntletLoc[2].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            //if(Creature* pTemp = me->SummonCreature(NPC_FLAMEBEARER, 888.0f, 57.0f, 534.5f, GauntletLoc[3].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                //pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_FLAMEBEARER, 888.0f, 55.0f, 534.5f, GauntletLoc[4].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            ++m_uiGauntletPhase;
                            m_uiGauntletTimer = 5000;
                            break;
                        case 1:
                            if(GetClosestCreatureWithEntry(me, NPC_FLAMEBEARER, 2515.0f) || GetClosestCreatureWithEntry(me, NPC_WRATHBRINGER, 2515.0f) || GetClosestCreatureWithEntry(me, NPC_DEATHBRINGER, 2515.0f)) 
                            {
                                m_uiGauntletPhase = 1;
                                m_uiGauntletTimer = 1000;
                            } 
                            else 
                            {
                                me->SummonCreature(NPC_DEATHBRINGER, GauntletLoc[0].x, GauntletLoc[0].y, GauntletLoc[0].z, GauntletLoc[0].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                                me->SummonCreature(NPC_WRATHBRINGER, GauntletLoc[1].x, GauntletLoc[1].y, GauntletLoc[1].z, GauntletLoc[1].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                                //me->SummonCreature(NPC_WRATHBRINGER, GauntletLoc[2].x, GauntletLoc[2].y, GauntletLoc[2].z, GauntletLoc[2].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                                me->SummonCreature(NPC_FLAMEBEARER, GauntletLoc[3].x, GauntletLoc[3].y, GauntletLoc[3].z, GauntletLoc[3].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                                //me->SummonCreature(NPC_FLAMEBEARER, GauntletLoc[4].x, GauntletLoc[4].y, GauntletLoc[4].z, GauntletLoc[4].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                                ++m_uiGauntletPhase;
                                m_uiGauntletTimer = 1000;
                            }
                            break;
                        case 2:
                            if(GetClosestCreatureWithEntry(me, NPC_FLAMEBEARER, 2515.0f) || GetClosestCreatureWithEntry(me, NPC_WRATHBRINGER, 2515.0f) || GetClosestCreatureWithEntry(me, NPC_DEATHBRINGER, 2515.0f))
                            {
                                m_uiGauntletPhase=2;
                                m_uiGauntletTimer = 1000;
                            } 
                            else 
                            {
                                ++m_uiGauntletPhase;
                                m_uiGauntletTimer = 3000;
                            }
                            break;
                        case 3:
                            me->GetMotionMaster()->MoveIdle();
                            me->SetGuidValue(UNIT_FIELD_TARGET, me->GetGUID());
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            me->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
                            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
                            me->GetMotionMaster()->MoveIdle();
                            DoScriptText(SAY_TUNNEL, me);
                            ++m_uiGauntletPhase;
                            m_uiGauntletTimer = 5000;
                            break;
                        case 4:
                            // summon third wave and set in combat
                            if(Creature* pTemp = me->SummonCreature(NPC_FALLEN_WARRIOR, GauntletLoc[5].x, GauntletLoc[5].y, GauntletLoc[5].z, GauntletLoc[5].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_FALLEN_WARRIOR, GauntletLoc[6].x, GauntletLoc[6].y, GauntletLoc[6].z, GauntletLoc[6].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_FALLEN_WARRIOR, GauntletLoc[7].x, GauntletLoc[7].y, GauntletLoc[7].z, GauntletLoc[7].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_FALLEN_WARRIOR, GauntletLoc[8].x, GauntletLoc[8].y, GauntletLoc[8].z, GauntletLoc[8].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            if(Creature* pTemp = me->SummonCreature(NPC_FALLEN_WARRIOR, GauntletLoc[9].x, GauntletLoc[9].y, GauntletLoc[9].z, GauntletLoc[9].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                                pTemp->SetInCombatWithZone();
                            ++m_uiGauntletPhase;
                            m_uiGauntletTimer = 5000;
                            break;
                        case 5:
                            // summon last wave, static
                            me->SummonCreature(NPC_DEATHBRINGER, GauntletLoc[5].x, GauntletLoc[5].y, GauntletLoc[5].z, GauntletLoc[5].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            me->SummonCreature(NPC_WRATHBRINGER, GauntletLoc[6].x, GauntletLoc[6].y, GauntletLoc[6].z, GauntletLoc[6].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            //me->SummonCreature(NPC_WRATHBRINGER, GauntletLoc[7].x, GauntletLoc[7].y, GauntletLoc[7].z, GauntletLoc[7].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            //me->SummonCreature(NPC_FLAMEBEARER, GauntletLoc[8].x, GauntletLoc[8].y, GauntletLoc[8].z, GauntletLoc[8].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            me->SummonCreature(NPC_FLAMEBEARER, GauntletLoc[9].x, GauntletLoc[9].y, GauntletLoc[9].z, GauntletLoc[9].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            ++m_uiGauntletPhase;
                            m_uiGauntletTimer = 10000;
                            break;
                        case 6:
                            me->SummonCreature(NPC_GLACIAL_REVENANT, TunnelLoc[7].x, TunnelLoc[7].y, TunnelLoc[7].z, TunnelLoc[7].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            for(uint8 i = 0; i < 10; i++)
                            {
                                for(uint8 j = 0; j < 2; j++)
                                {
                                    switch(urand(0, 3))
                                    {
                                        case 0: m_uiAddEntry = NPC_WRATHBONE_COLDWRAITH; break;
                                        case 1: case 2: case 3: m_uiAddEntry = NPC_FALLEN_WARRIOR; break;
                                    }
                                    angle = (float) rand()*360/RAND_MAX + 1;
                                    homeX = TunnelLoc[i].x + urand(0, 7)*cos(angle*(M_PI/180));
                                    homeY = TunnelLoc[i].y + urand(0, 7)*sin(angle*(M_PI/180));
                                    me->SummonCreature(m_uiAddEntry, homeX, homeY, TunnelLoc[i].z, TunnelLoc[i].o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                                }
                            }
                            ++m_uiGauntletPhase;
                            m_uiGauntletTimer = 5000;
                            break;
                        case 7:
                            //me->MonsterMoveWithSpeed(990.014f, 161.778f, 643.168f, 1);
                            me->SetVisible(false);    
                            ++m_uiGauntletPhase;
                            m_uiGauntletTimer = urand(3000, 5000);
                            break;
                        case 8:
                            // do icicles
                            for(uint8 i = 0; i < 10; i++)
                            {
                                angle = (float) rand()*360/RAND_MAX + 1;
                                homeX = TunnelLoc[i].x + urand(0, 7)*cos(angle*(M_PI/180));
                                homeY = TunnelLoc[i].y + urand(0, 7)*sin(angle*(M_PI/180));
                                me->SummonCreature(NPC_COLLAPSING_ICICLE, homeX, homeY, TunnelLoc[i].z + 1, TunnelLoc[i].o, TEMPSUMMON_TIMED_DESPAWN, 10000);
                            }
                            m_uiGauntletTimer = urand(3000, 5000);
                            break;
                    }
                }else m_uiGauntletTimer -= diff;
            }
    
            if(instance->GetBossState(DATA_ICK) == NOT_STARTED &&  instance->GetBossState(DATA_GARFROST) != DONE && instance->GetBossState(DATA_GARFROST) == SPECIAL) 
            { 
                if(m_uiSpeech_Timers < diff)
                {
                    switch(m_uiOutro_Phases)
                    {
                        case 0:
                            me->CastSpell(me, SPELL_FLY, true);
                            me->MonsterMoveWithSpeed(703.30f, -194.517f, 561.727f, 1);
                            ++m_uiOutro_Phases;
                            m_uiSpeech_Timers = 10000;
                            break;
                        case 1:
                            me->GetMap()->CreatureRelocation(me, 703.30f, -194.517f, 561.727f, 1.784f);
                            ++m_uiOutro_Phases;
                            m_uiSpeech_Timers = 1;
                            break;
                    }
                } else m_uiSpeech_Timers -= diff;
            }

            if(instance->GetBossState(DATA_ICK) == SPECIAL)
            { 
                checkEventIck = true;
            }
    
            if(checkEventIck && instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED ) 
            {
                if(m_uiSpeech_Timerss < diff)
                {
                    switch(m_uiOutro_Phasess)
                    {
                        case 0:
                            me->SetSpeed(MOVE_FLIGHT, 3.5f, true);
                            me->MonsterMoveWithSpeed(835.5887f, 139.4345f, 530.9526f, 1);
                            ++m_uiOutro_Phasess;
                            m_uiSpeech_Timerss = 8000;
                            break;
                        case 1:
                            DoScriptText(SAY_TYRANNUS_OUTRO_7, me);
                            ++m_uiOutro_Phasess;
                            m_uiSpeech_Timerss = 3000;
                            break;
                        case 2:
                            if(Creature* pKrickNPC = GetClosestCreatureWithEntry(me, NPC_KRICK, 1725.0f))
                            {
                                DoCast(pKrickNPC, SPELL_NECROMANTIC_POWER);  //not sure if it's the right spell :/
                            } 
                            ++m_uiOutro_Phasess;
                            m_uiSpeech_Timerss = 7000;
                            break;
                        case 3:
                            DoScriptText(SAY_TYRANNUS_OUTRO_9, me);
                            ++m_uiOutro_Phasess;
                            m_uiSpeech_Timerss = 3000;
                            break;
                        case 4:
                            me->SetSpeed(MOVE_FLIGHT, 3.0f, true);
                            me->GetMotionMaster()->MovePoint(0, outroPos[0]);
                            ++m_uiOutro_Phasess;
                            m_uiSpeech_Timerss = 10000;
                            break;
                        case 5:
                            me->GetMap()->CreatureRelocation(me, 893.4820f, -57.1602f, 606.3624f, 1.57f);
                            m_uiSpeech_Timerss = 1;
                        break;
                    }
                } else m_uiSpeech_Timerss -= diff;
            }
            if(instance->GetBossState(DATA_GARFROST) != NOT_STARTED && instance->GetBossState(DATA_GARFROST) == DONE && instance->GetBossState(DATA_ICK) == NOT_STARTED && instance->GetBossState(DATA_ICK) != DONE && instance->GetBossState(DATA_ICK) != IN_PROGRESS) 
            {
                if(m_uiSpeech_Timer < diff)
                {
                    switch(m_uiOutro_Phase)
                    {
                        case 0:
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 7000;
                            break;
                        case 1:
                            DoScriptText(SAY_TYRANNUS_DEATH, me);
                            me->MonsterMoveWithSpeed(841.0100f, 196.2450f, 573.9640f, 1);
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 10000;
                            break;
                        case 2:
                            me->GetMap()->CreatureRelocation(me, 841.0100f, 196.2450f, 573.9640f, 4.57f);
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 1;
                            break;
                    }
                } else m_uiSpeech_Timer -= diff;
            }
    
        /*  if(instance->GetBossState(DATA_GARFROST)==SPECIAL && instance->GetBossState(DATA_ICK)==IN_PROGRESS)
            {
                if(m_uiSpeech_Timere < diff)
                {
                    switch(m_uiOutro_Phasee)
                    {
                        case 0:
                            ++m_uiOutro_Phasee;
                            m_uiSpeech_Timere = 1000;
                            break;
                        case 1:
                            me->MonsterMoveWithSpeed(841.0100f, 196.2450f, 573.9640f, 1);
                            ++m_uiOutro_Phasee;
                            m_uiSpeech_Timere = 5000;
                            break;
                        case 2:
                            me->GetMap()->CreatureRelocation(me, 841.0100f, 196.2450f, 573.9640f, 1.57f);
                            ++m_uiOutro_Phasee;
                            m_uiSpeech_Timere = 1;
                            break;
                    }
                } else m_uiSpeech_Timere -= diff;
            }*/
        }
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tyr_event_pitAI(creature);
    }    
};

void AddSC_boss_tyrannus()
{
    new boss_tyrannus();
    new boss_rimefang();
    new spell_tyrannus_overlord_brand();
    new spell_tyrannus_mark_of_rimefang();
    new at_tyrannus_event_starter();
    new at_tyrannus_gauntlet_starter();
    //new npc_martin_gorkun_end();
    //new boss_sindra();
    //new npc_sylvanas_jaina_pos_end();
    new npc_ice_mob_bomb();
    //new npc_tyr_event_pit();
}