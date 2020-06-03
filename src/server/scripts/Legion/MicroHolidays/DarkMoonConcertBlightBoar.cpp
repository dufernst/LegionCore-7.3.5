#include "MoveSplineInit.h"

enum eSay
{
    SAY_JAY_MAGUIRE_20_MIN_LEFT         = 0,
    SAY_JAY_MAGUIRE_10_MIN_LEFT,
    SAY_JAY_MAGUIRE_1_MIN_LEFT,
    SAY_JAY_MAGUIRE_START_CONCERT,
    SAY_JAY_MAGUIRE_30_MIN_LEFT,

    SAY_DEATH_METAL_KNIGHT_INTRO_1      = 0,
    SAY_DEATH_METAL_KNIGHT_INTRO_2,
    SAY_DEATH_METAL_KNIGHT_SUMMON_BANSHEE,
    SAY_DEATH_METAL_KNIGHT_DEATH_PLAYER,
    SAY_DEATH_METAL_KNIGHT_DEATHKNIGHT_ARMYOFTHEDEAD,
    SAY_DEATH_METAL_KNIGHT_PHASE_2,
    SAY_DEATH_METAL_KNIGHT_DEATH,

    SAY_DEVLYNN_STYX_SCREAM             = 0
};

enum eCreatures
{
    NPC_MANAGER                 = 125089,
    NPC_JAY_MAGUIRE             = 120928,
    NPC_DEATH_METAL_KNIGHT      = 122899,
    NPC_JON_GRAVES              = 121529,
    NPC_DEVLYNN_STYX            = 121527,
    NPC_CAGE_HEAD               = 121528,
    NPC_VFX_BUNNY               = 122323,
    NPC_INVISIBLE_STALKER       = 121295,
    NPC_DRUMMER                 = 121561,
    NPC_METAL_FAN               = 121535,
    NPC_METAL_FAN2              = 121537,
    NPC_METAL_FAN3              = 123691,
    NPC_GHOUL                   = 122943
};

enum eSpells
{
    //DEATH METAL KNIGHT
    SPELL_GRAND_ENTRANCE            = 244950,
    SPELL_ARMY_OF_THE_DEAD          = 245162,
    SPELL_COLD_AS_ICE               = 245025,
    SPELL_SUMMON_GHOUL              = 245067,
    SPELL_SWORD                     = 246038,
    SPELL_WAILING_WALL              = 244955,
    //Jon Graves
    SPELL_MIND_BLOWN                = 245034,
    SPELL_GUITAR                    = 243196,
    //DEVLYNN STYX
    SPELL_MICROPHONE                = 242932,
    SPELL_BASSIST                   = 246584,
    SPELL_GUITARIST                 = 246583,
    SPELL_VOCALS                    = 246586,
    SPELL_DRUMMER                   = 246585,
    SPELL_STAGE_DIVE                = 245105,
    SPELL_IN_THE_CROWD              = 245148,
    SPELL_WHOOOOOOAH                = 245155,
    SPELL_SCORE_UP                  = 248246,
    //CAGE HEAD
    SPELL_BASS                      = 245160,
    //VFX BUNNY
    SPELL_LIGHT                     = 243893,
    //MANAGER
    SPELL_PLAY_MUSIC                = 247666,
    SPELL_END_SONG                  = 247667,
    //INVISIBLE STALKER
    SPELL_COSMETIC_FEL_EXPLOSION    = 240894,
    //DRUMMER
    SPELL_DRUM_STICKS               = 242908,
    //GENERAL
    SPELL_ACHIEVEMENT               = 245363,
    SPELL_ACTIVATE_GAMEOBJECTS      = 252525,
    SPELL_CASUAL_FAN                = 245063,
    SPELL_SERIOUS_FAN               = 245065,
    SPELL_HARDCORE_FUN              = 245066,
    //GHOUL
    SPELL_DEATHLY_PALLOR            = 245255,
    SPELL_DEATHS_ADVANCE            = 245256,
    SPELL_TOUCH_OF_THE_GRAVE        = 245245
};

enum eEvents
{
    //Jay Maguire
    EVENT_PREPARE_30_MIN        = 1,
    EVENT_PREPARE_20_MIN        = 2,
    EVENT_PREPARE_10_MIN        = 3,
    EVENT_PREPARE_1_MIN         = 4,
    EVENT_READY                 = 5,
    EVENT_ENDED                 = 6,
    EVENT_STARTED               = 7,
    //MANAGER
    EVENT_START_CONCERT         = 8,
    EVENT_END_CONCERT           = 9,
    //Death Metal Knight
    EVENT_BANSHEE_AT            = 10,
    EVENT_INTRO_WP              = 11,
    EVENT_ARMY_OF_THE_DEAD      = 12,
    //DEVLYNN STYX
    EVENT_JUMP_SCREAM           = 13,
    EVENT_WHOOOOOOAH            = 14
};

static Position destSummon[10] =
{
    { -4189.54f, 6037.1f, 6.59233f, 5.19811f },
    { -4190.14f, 6014.1f, 5.78008f, 1.09447f },
    { -4180.64f, 6009.92f, 5.60332f, 1.23409f },
    { -4191.35f, 6035.88f, 6.48239f, 5.18861f },
    { -4186.57f, 6012.41f, 5.7644f, 1.16428f },
    { -4165.23f, 6019.99f, 6.80102f, 2.67399f },
    { -4166.17f, 6014.87f, 6.86055f, 3.0499f },
    { -4165.81f, 6002.39f, 6.94393f, 3.11536f },
    { -4183.02f, 6011.02f, 5.75034f, 1.21664f },
    { -4193.4f, 6034.81f, 6.682f, 5.21955f }
};

Position const ATSpawnPosition[20] =
{
    { -4169.229f, 6052.789f, 6.260829f, 4.371895f },
    { -4172.854f, 6054.479f, 6.260829f, 4.43865f},
    { -4167.416f, 6051.943f, 6.260829f, 4.336551f },
    { -4165.604f, 6051.098f, 6.260829f, 4.273721f },
    { -4163.791f, 6050.253f, 6.260829f, 4.297285f},
    { -4175.737f, 5996.241f, 6.260829f, 1.108567f },
    { -4173.925f, 5995.396f, 6.260829f, 1.128202f },
    { -4172.112f, 5994.551f, 6.260829f, 1.104645f },
    { -4181.175f, 5998.777f, 6.260829f, 1.085010f },
    { -4179.362f, 5997.932f, 6.260829f, 1.085010f },
    { -4177.55f, 5997.086f, 6.260829f, 1.085010f },
    { -4170.3f, 5993.706f, 6.260829f, 1.085010f },
    { -4184.8f, 6000.467f, 6.260829f, 1.253874f },
    { -4197.487f, 6006.384f, 6.260829f, 1.324560f },
    { -4199.3f, 6007.229f, 6.260829f, 1.316706f },
    { -4201.112f, 6008.074f, 6.260829f, 1.316706f },
    { -4202.925f, 6008.919f, 6.260829f, 1.316708f },
    { -4204.737f, 6009.765f, 6.260829f, 1.112502f },
    { -4195.675f, 6005.539f, 6.260829f, 1.112502f },
    { -4206.55f, 6010.61f, 6.260829f, 1.112502f }
};

struct npc_jay_maguire : public ScriptedAI
{
    npc_jay_maguire(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool finishwp;

    void Reset() override
    {
        events.Reset();
        events.RescheduleEvent(EVENT_PREPARE_30_MIN, 1000);
        events.RescheduleEvent(EVENT_PREPARE_20_MIN, 1000);
        events.RescheduleEvent(EVENT_PREPARE_10_MIN, 1000);
        events.RescheduleEvent(EVENT_PREPARE_1_MIN, 1000);
        events.RescheduleEvent(EVENT_STARTED, 1000);
        finishwp = false;
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_PREPARE_30_MIN:
                if (sGameEventMgr->IsActiveEvent(74))
                    Talk(SAY_JAY_MAGUIRE_30_MIN_LEFT);
                else
                    events.RescheduleEvent(EVENT_PREPARE_30_MIN, 1000);
                break;
            case EVENT_PREPARE_20_MIN:
                if (sGameEventMgr->IsActiveEvent(71))
                    Talk(SAY_JAY_MAGUIRE_20_MIN_LEFT);
                else
                    events.RescheduleEvent(EVENT_PREPARE_20_MIN, 1000);
                break;
            case EVENT_PREPARE_10_MIN:
                if (sGameEventMgr->IsActiveEvent(72))
                    Talk(SAY_JAY_MAGUIRE_10_MIN_LEFT);
                else
                    events.RescheduleEvent(EVENT_PREPARE_10_MIN, 1000);
                break;
            case EVENT_PREPARE_1_MIN:
                if (sGameEventMgr->IsActiveEvent(73))
                    Talk(SAY_JAY_MAGUIRE_1_MIN_LEFT);
                else
                    events.RescheduleEvent(EVENT_PREPARE_1_MIN, 1000);
                break;
            case EVENT_READY:
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCE);
                events.RescheduleEvent(EVENT_ENDED, 312000);
                break;
            case EVENT_ENDED:
                me->NearTeleportTo(-4155.22f, 6067.37f, 8.65011f, 2.19868f);
                me->DespawnOrUnsummon(1000);
                break;
            case EVENT_STARTED:
                if (sGameEventMgr->IsActiveEvent(79))
                {
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_ONESHOT_NONE);
                    me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    Movement::MoveSplineInit init(*me);
                    init.Path().push_back(G3D::Vector3(-4155.821f, 6066.204f, 8.775227f));
                    init.Path().push_back(G3D::Vector3(-4157.071f, 6065.454f, 9.275227f));
                    init.Path().push_back(G3D::Vector3(-4159.321f, 6063.204f, 9.775227f));
                    init.Path().push_back(G3D::Vector3(-4161.071f, 6061.204f, 9.025227f));
                    init.Path().push_back(G3D::Vector3(-4164.071f, 6056.704f, 8.275227f));
                    init.Path().push_back(G3D::Vector3(-4164.821f, 6049.704f, 7.525226f));
                    init.Path().push_back(G3D::Vector3(-4166.071f, 6047.704f, 7.275226f));
                    init.Path().push_back(G3D::Vector3(-4168.071f, 6038.454f, 8.025227f));
                    init.Path().push_back(G3D::Vector3(-4168.321f, 6033.454f, 7.275226f));
                    init.Path().push_back(G3D::Vector3(-4169.071f, 6031.204f, 6.775226f));
                    init.Path().push_back(G3D::Vector3(-4172.071f, 6019.704f, 6.275226f));
                    init.Path().push_back(G3D::Vector3(-4174.071f, 6008.204f, 6.025226f));
                    init.Path().push_back(G3D::Vector3(-4174.42f, 6003.042f, 5.900339f));
                    init.SetWalk(false);
                    init.Launch();
                    finishwp = true;
                }
                else
                    events.RescheduleEvent(EVENT_STARTED, 1000);
                break;
            }

            if (finishwp && me->IsSplineFinished())
            {
                finishwp = false;
                me->SetOrientation(1.598367f);
                me->SetFacingTo(1.598367f);
                Talk(SAY_JAY_MAGUIRE_START_CONCERT);
                events.RescheduleEvent(EVENT_READY, 4000);
            }
        }
    }
};

struct npc_darkmoon_manager : public ScriptedAI
{
    npc_darkmoon_manager(Creature* creature) : ScriptedAI(creature), summons(me) {}

    EventMap events;
    SummonList summons;
    ObjectGuid InvisibleStalkerGUID;
    bool startEvent;

    void Reset() override
    {
        events.Reset();
        startEvent = false;
    }

    void JustSummoned(Creature* sum) override
    {
        summons.Summon(sum);

        switch (sum->GetEntry())
        {
        case NPC_JON_GRAVES:
            sum->CastSpell(sum, SPELL_GUITAR, true);
            sum->SetAnimKitId(13199);
            break;
        case NPC_DEVLYNN_STYX:
            sum->CastSpell(sum, SPELL_MICROPHONE, true);
            sum->SetAnimKitId(13113);
            break;
        case NPC_CAGE_HEAD:
            sum->CastSpell(sum, SPELL_BASS, true);
            sum->SetAnimKitId(13414);
            break;
        case NPC_DRUMMER:
            sum->CastSpell(sum, SPELL_DRUM_STICKS, true);
            sum->SetAnimKitId(13099);
            break;
        case NPC_INVISIBLE_STALKER:
            InvisibleStalkerGUID.Clear();
            InvisibleStalkerGUID = sum->GetGUID();
            sum->CastSpell(sum, SPELL_COSMETIC_FEL_EXPLOSION, true);
            break;
        case NPC_METAL_FAN:
            sum->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCE);
            break;
        case NPC_METAL_FAN2:
            sum->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCE);
            break;
        case NPC_METAL_FAN3:
            sum->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCE);
            break;
        default:
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_START_CONCERT:
                DoCast(SPELL_PLAY_MUSIC);
                DoCast(SPELL_ACTIVATE_GAMEOBJECTS);
                me->SummonCreatureGroup(0);
                events.RescheduleEvent(EVENT_END_CONCERT, 312300);
                break;
            case EVENT_END_CONCERT:
                if (auto stalker = me->GetCreature(*me, InvisibleStalkerGUID))
                    stalker->CastSpell(stalker, SPELL_COSMETIC_FEL_EXPLOSION, true);

                startEvent = false;
                DoCast(SPELL_ACHIEVEMENT);
                DoCast(SPELL_END_SONG);
                events.RescheduleEvent(EVENT_2, 2000);
                break;
            case EVENT_2:
                me->DespawnOrUnsummon();
                break;
            }
        }

        if ((sGameEventMgr->IsActiveEvent(79)) && !startEvent)
        {
            startEvent = true;
            events.RescheduleEvent(EVENT_START_CONCERT, 18000);
        }
    }
};

struct npc_death_metal_knight : public ScriptedAI
{
    npc_death_metal_knight(Creature* creature) : ScriptedAI(creature)
    {
        introDone = false;
        introDone2 = false;
    }

    EventMap events;
    bool army;
    bool introDone;
    bool introDone2;

    void Reset() override
    {
        events.Reset();
        DespawnAllSummons();
        me->RemoveAllAreaObjects();
        me->SetReactState(REACT_PASSIVE);
        DoCast(SPELL_SWORD);
        army = false;
    }

    void EnterEvadeMode() override
    {
        if (auto styx = !me->FindNearestCreature(NPC_DEVLYNN_STYX, 100.0f, true))
            me->DespawnOrUnsummon();

        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        DoCast(SPELL_COLD_AS_ICE);
        events.RescheduleEvent(EVENT_BANSHEE_AT, 27000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH_METAL_KNIGHT_DEATH);
        me->RemoveAllAreaObjects();
        DespawnAllSummons();
    }

    void DespawnAllSummons()
    {
        std::list<Creature*> list;
        list.clear();
        me->GetCreatureListWithEntryInGrid(list, NPC_GHOUL, 200.0f);
        if (!list.empty())
            for (auto& cre : list)
                cre->DespawnOrUnsummon();
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        if (!introDone)
        {
            introDone = true;
            DoCast(SPELL_GRAND_ENTRANCE);
            Talk(SAY_DEATH_METAL_KNIGHT_INTRO_1);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);
            me->AddDelayedEvent(15000, [=]() -> void
            {
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(1, -4179.04f, 6026.6616f, 6.154604f);
            });
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (!me->HealthAbovePct(28) && !army)
        {
            if (me->HealthBelowPct(28))
            {
                army = true;
                events.RescheduleEvent(EVENT_ARMY_OF_THE_DEAD, 1000);
            }
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        switch (id)
        {
        case 1:
            if (!introDone2)
            {
                introDone2 = true;
                Talk(SAY_DEATH_METAL_KNIGHT_INTRO_2);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetHomePosition(-4179.04f, 6026.6616f, 6.154604f, 4.30094f);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);
            }
            break;
        default:
            break;
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_ARMY_OF_THE_DEAD)
            me->SetReactState(REACT_AGGRESSIVE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BANSHEE_AT:
                Talk(SAY_DEATH_METAL_KNIGHT_SUMMON_BANSHEE);

                for (uint8 i = 0; i < 20; ++i)
                    me->CastSpell(ATSpawnPosition[i], SPELL_WAILING_WALL, true);

                events.RescheduleEvent(EVENT_BANSHEE_AT, 25000);
                break;
            case EVENT_ARMY_OF_THE_DEAD:
                me->CastStop();
                me->SetReactState(REACT_PASSIVE);
                DoCast(SPELL_ARMY_OF_THE_DEAD);
                Talk(SAY_DEATH_METAL_KNIGHT_DEATHKNIGHT_ARMYOFTHEDEAD);
                events.RescheduleEvent(EVENT_ARMY_OF_THE_DEAD, 25000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_darkmoon_ghoul : public ScriptedAI
{
    npc_darkmoon_ghoul(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* owner) override
    {
        DoCast(SPELL_DEATHLY_PALLOR);
        DoCast(SPELL_DEATHS_ADVANCE);

        if (auto styx = me->FindNearestCreature(NPC_DEVLYNN_STYX, 100.0f, true))
            me->GetMotionMaster()->MoveFollow(styx, 0.5, 1.5, MOTION_SLOT_ACTIVE);
        else
            me->GetMotionMaster()->MoveFollow(owner, 0.5, 1.5, MOTION_SLOT_ACTIVE);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == FOLLOW_MOTION_TYPE)
            DoCast(SPELL_TOUCH_OF_THE_GRAVE);
    }
};

struct npc_devlynn_styx : public ScriptedAI
{
    npc_devlynn_styx(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        events.Reset();
        events.RescheduleEvent(EVENT_JUMP_SCREAM, 120000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_JUMP_SCREAM:
            {
                DoCast(SPELL_STAGE_DIVE);
                events.RescheduleEvent(EVENT_WHOOOOOOAH, 13000);
                Talk(SAY_DEVLYNN_STYX_SCREAM);
                me->AddDelayedEvent(1000, [=]() -> void { DoCast(SPELL_IN_THE_CROWD); });
                break;
            }
            case EVENT_WHOOOOOOAH:
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 100.0f);
                Trinity::Containers::RandomResizeList(playerList, 1);

                if (playerList.empty())
                    return;

                for (auto& player : playerList)
                {
                    me->SetFacingTo(player);
                    DoCast(SPELL_WHOOOOOOAH);
                }

                events.RescheduleEvent(EVENT_WHOOOOOOAH, 25000);
                break;
            }
        }
    }
};

//245162
class spell_army_of_the_dead : public AuraScript
{
    PrepareAuraScript(spell_army_of_the_dead);

    uint8 RandSummonCheck{};

    void Tick(AuraEffect const* aurEff)
    {
        if (Creature* deathmetal = GetCaster()->ToCreature())
        {
            deathmetal->CastSpell(destSummon[RandSummonCheck], SPELL_SUMMON_GHOUL, false);
            RandSummonCheck++;
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_army_of_the_dead::Tick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

void AddSC_DarkMoonConcertBlightBoar()
{
    RegisterCreatureAI(npc_death_metal_knight);
    RegisterCreatureAI(npc_darkmoon_manager);
    RegisterCreatureAI(npc_jay_maguire);
    RegisterCreatureAI(npc_darkmoon_ghoul);
    RegisterCreatureAI(npc_devlynn_styx);
    RegisterAuraScript(spell_army_of_the_dead);
}