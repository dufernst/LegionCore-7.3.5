/*==============
==============*/

enum eBosses
{
    BOSS_SHA_OF_DOUBT
};

enum eSpells
{
    SPELL_WITHER_WILL           = 106736,
    SPELL_TOUCH_OF_NOTHINGNESS  = 106113,
    SPELL_FIGMENT_OF_DOUBT      = 106937,
    SPELL_BOUNDS_OF_REALITY     = 131498,
    SPELL_BOUNDS_OF_REALITY_2   = 117665,
    SPELL_CHI_WAVE              = 132464,
    SPELL_CHI_WAVE_2            = 132466,
    
    SPELL_FIGMENT_OF_DOUBT_2    = 106935,
    SPELL_FIGMENT_OF_DOUBT_3    = 106936,
    SPELL_COPY_WEAPON           = 41054,
    SPELL_COPY_WEAPON_2         = 41055,
    SPELL_GROW                  = 104921,
    SPELL_DROWNED_STATE         = 117509,
    SPELL_DRAW_DOUBT            = 106290,
    SPELL_KNOCK_BACK_SELF       = 117525,
    SPELL_GATHERING_DOUBT       = 117570,
    SPELL_GATHERING_DOUBT_2     = 117571,
    SPELL_INVISIBILITY_DETECTION= 126839,
    SPELL_WEAKENED_BLOWS        = 115798,
    SPELL_RELEASE_DOUBT         = 106112
};

enum eEvents
{
    EVENT_WITHER_WILL           = 1,
    EVENT_TOUCH_OF_NOTHINGNESS  = 2,
    EVENT_BOUNDS_OF_REALITY     = 3,

    EVENT_GATHERING_DOUBT       = 4,

    EVENT_SPELL_PHANTOM_STRIKE  = 5,
    EVENT_SPELL_ARMOR_BUFF      = 6,
    EVENT_SPELL_FURY            = 7,
    EVENT_SANCTUARY             = 8,
    EVENT_SIPHON_ESSENCE        = 9,
    EVENT_STUN                  = 10,
    EVENT_BLADE_SONG            = 11,
    EVENT_UNTAMED_FURY          = 12,
    EVENT_GLIMPSE_OF_MADNESS    = 13
};

enum eCreatures
{
    CREATURE_SHA_OF_DOUBT       = 56439
};

enum eTalks
{
    TALK_AGGRO,
    TALK_DEATH,
    TALK_FIGMENT_01,
    TALK_FIGMENT_02,
    TALK_RESET,
    TALK_SLAY_01,
    TALK_SLAY_02
};

struct boss_sha_of_doubt : public BossAI
{
    explicit boss_sha_of_doubt(Creature* creature) : BossAI(creature, BOSS_SHA_OF_DOUBT)
    {
        DoCast(SPELL_INVISIBILITY_DETECTION);
    }

    bool isAtBoundsOfReality;

    void Reset() override
    {
        _Reset();
        Talk(TALK_RESET);
        events.Reset();
    }

    void KilledUnit(Unit* u) override
    {
        if (!u && !u->IsPlayer())
            return;

        Talk(TALK_SLAY_01 + urand(0, 1));
    }

    void EnterEvadeMode() override
    {
        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, 56792, 100.0f);
        if (!creList.empty())
            for (auto& cre : creList)
                cre->DespawnOrUnsummon();

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*u*/) override
    {
        Talk(TALK_DEATH);
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        Talk(TALK_AGGRO);
        events.RescheduleEvent(EVENT_WITHER_WILL, 5000);
        events.RescheduleEvent(EVENT_TOUCH_OF_NOTHINGNESS, 500);
        events.RescheduleEvent(EVENT_BOUNDS_OF_REALITY, 3000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_WITHER_WILL:
                if (!me->HasAura(SPELL_BOUNDS_OF_REALITY_2))
                {
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        DoCast(target, SPELL_WITHER_WILL, false);
                        DoCast(target, SPELL_WITHER_WILL, false);
                    }
                }
                events.RescheduleEvent(EVENT_WITHER_WILL, 5000);
                break;
            case EVENT_TOUCH_OF_NOTHINGNESS:
                if (!me->HasAura(SPELL_BOUNDS_OF_REALITY_2))

                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_TOUCH_OF_NOTHINGNESS, false);

                events.RescheduleEvent(EVENT_TOUCH_OF_NOTHINGNESS, 30000);
                break;
            case EVENT_BOUNDS_OF_REALITY:
                Talk(TALK_FIGMENT_01 + urand(0, 1));
                instance->SetData(8, 0);
                DoCast(SPELL_BOUNDS_OF_REALITY_2);
                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (player->isAlive())
                        player->CastSpell(player, SPELL_FIGMENT_OF_DOUBT_3, true);
                });

                events.RescheduleEvent(EVENT_BOUNDS_OF_REALITY, 60000);
            break;
            }
        }

        if (!me->HasAura(SPELL_BOUNDS_OF_REALITY_2))
            DoMeleeAttackIfReady();
    }
};

class mob_figment_of_doubt : public CreatureScript
{
    public:
        mob_figment_of_doubt() : CreatureScript("mob_figment_of_doubt") {}

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_figment_of_doubt_AI(creature);
        }

        enum Classes
        {
            CLASS_DPS,
            CLASS_HEAL,
            CLASS_TANK,
        };

        struct mob_figment_of_doubt_AI : public ScriptedAI
        {
            explicit mob_figment_of_doubt_AI(Creature* creature) : ScriptedAI(creature)
            {
                me->CastSpell(me, SPELL_GROW, false);
                Classes cl = Classes(me->GetInstanceScript()->GetData(8));
                if (cl <= CLASS_TANK)
                    _class = cl;
                else
                    _class = CLASS_DPS;
            }

            EventMap events;
            Classes _class;

            void JustDied(Unit* /*killer*/) override
            {
                DoCast(SPELL_DROWNED_STATE);
                me->RemoveAura(SPELL_GATHERING_DOUBT);
                me->ForcedDespawn(5000);
                me->GetInstanceScript()->SetData(9, _class);
            }

            void IsSummonedBy(Unit* owner) override
            {
                DoCast(owner, SPELL_DRAW_DOUBT, false);
                AttackStart(owner);
                me->SetReactState(REACT_PASSIVE);
            }

            void EnterCombat(Unit* u) override
            {
                DoCast(SPELL_GATHERING_DOUBT);
                events.RescheduleEvent(EVENT_GATHERING_DOUBT, 1000);
                
                events.RescheduleEvent(EVENT_SIPHON_ESSENCE, 8000);
                switch (_class)
                {
                case CLASS_DPS:
                    events.RescheduleEvent(EVENT_SPELL_FURY, 5000);
                    events.RescheduleEvent(EVENT_BLADE_SONG, 13000);
                    events.RescheduleEvent(EVENT_GLIMPSE_OF_MADNESS, 8000);
                case CLASS_HEAL:
                    events.RescheduleEvent(EVENT_SPELL_PHANTOM_STRIKE, 20000);
                    events.RescheduleEvent(EVENT_STUN, 7000);
                case CLASS_TANK:
                    events.RescheduleEvent(EVENT_SPELL_ARMOR_BUFF, 10000);
                    events.RescheduleEvent(EVENT_SANCTUARY, 10000);
                    events.RescheduleEvent(EVENT_STUN, 7000);
                    break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_GATHERING_DOUBT:
                        if (me->GetAuraCount(SPELL_GATHERING_DOUBT_2) == 30)
                        {
                            DoCast(SPELL_RELEASE_DOUBT);
                            me->RemoveAura(SPELL_GATHERING_DOUBT);
                            me->DealDamage(me, me->GetMaxHealth());
                            me->ForcedDespawn(5000);

                            if (ObjectGuid guid_sha_of_doubt = me->GetInstanceScript()->GetGuidData(CREATURE_SHA_OF_DOUBT))
                            {
                                if (auto creature = me->GetMap()->GetCreature(guid_sha_of_doubt))
                                {
                                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_RELEASE_DOUBT))
                                    {
                                        DoCast(creature, SPELL_CHI_WAVE, true);
                                        me->HealBySpell(creature, spellInfo, uint32(float(creature->GetMaxHealth()) * 0.1f));
                                    }
                                }
                            }
                        }
                        else
                            events.RescheduleEvent(EVENT_GATHERING_DOUBT, 1000);
                        break;
                    case EVENT_SPELL_PHANTOM_STRIKE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, 9806, false);

                        events.RescheduleEvent(EVENT_SPELL_PHANTOM_STRIKE, 20000);
                        break;
                    case EVENT_SPELL_ARMOR_BUFF:
                        DoCast(34199);
                        events.RescheduleEvent(EVENT_SPELL_ARMOR_BUFF, 10000);
                        break;
                    case EVENT_SPELL_FURY:
                        DoCast(15494);
                        events.RescheduleEvent(EVENT_SPELL_FURY, 5000);
                        break;
                    case EVENT_SANCTUARY:
                        DoCast(69207);
                        events.RescheduleEvent(EVENT_SANCTUARY, 10000);
                        break;
                    case EVENT_SIPHON_ESSENCE:
                        DoCast(40291);
                        events.RescheduleEvent(EVENT_SIPHON_ESSENCE, 8000);
                        break;
                    case EVENT_STUN:
                        if (auto victim = me->getVictim())
                            DoCast(victim, 23454, false);

                        events.RescheduleEvent(EVENT_STUN, 7000);
                        break;
                    case EVENT_BLADE_SONG:
                        DoCast(38282);
                        events.RescheduleEvent(EVENT_BLADE_SONG, 13000);
                        break;
                    case EVENT_UNTAMED_FURY:
                        DoCast(23719);
                        events.RescheduleEvent(EVENT_UNTAMED_FURY, 9000);
                        break;
                    case EVENT_GLIMPSE_OF_MADNESS:
                        if (auto victim = me->getVictim())
                            me->CastSpell(victim, 26108, false);

                        events.RescheduleEvent(EVENT_GLIMPSE_OF_MADNESS, 8000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

void AddSC_boss_sha_of_doubt()
{
    RegisterCreatureAI(boss_sha_of_doubt);
    new mob_figment_of_doubt();
}