#include "blackrock_caverns.h"

enum ScriptTexts
{
    SAY_AGGRO       = 0,
    SAY_KILL        = 1,
    SAY_TRANSFORM   = 2,
    SAY_DEATH       = 3
};

enum Spells
{
    SPELL_EVOLUTION                 = 75697,
    SPELL_TWILIGHT_EVOLUTION        = 75732,
    SPELL_AURA_OF_ACCELERATION      = 75817,
    SPELL_DARK_COMMAND              = 75823,
    SPELL_KNEELING_IN_SUPPLICATION  = 75608,
    SPELL_DRAIN_ESSENCE_CHANNELING  = 75645,
    SPELL_NETHERESSENCE_AURA        = 75649,
    SPELL_NETHERESSENCE_VISUAL      = 75650,
    SPELL_NETHERBEAM                = 75677,
    SPELL_FORCE_BLAST               = 76522,
    SPELL_GRAVITY_STRIKE            = 76561,
    SPELL_GRIEVOUS_WHIRL            = 93658,
    SPELL_SHADOW_STRIKE             = 66134
};

enum Events
{
    EVENT_DARK_COMMAND      = 1,
    EVENT_FORCE_BLAST       = 2,
    EVENT_GRAVITY_STRIKE    = 3,
    EVENT_GRIEVOUS_WHIRL    = 4,
    EVENT_SHADOW_STRIKE     = 5
};

enum Actions
{
    ACTION_TRIGGER_START_CHANNELING = 1,
    ACTION_TRIGGER_STOP_CHANNELING  = 2
};

Position const summonPositions[4] =
{
    { 573.676f, 980.619f, 155.354f, 1.58448f },
    { 580.919f, 982.981f, 155.354f, 2.05572f },
    { 565.629f, 983.0f, 155.354f, 0.689123f  },
    { 573.272f, 907.707f, 178.506f, 1.55050f }
};

class boss_corla_herald_of_twilight : public CreatureScript
{
public:
    boss_corla_herald_of_twilight() : CreatureScript("boss_corla_herald_of_twilight") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_corla_herald_of_twilightAI(creature);
    }

    struct boss_corla_herald_of_twilightAI : public ScriptedAI
    {
        boss_corla_herald_of_twilightAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();

            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
                NetherEssenceTrigger[i] = NULL;

            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
                TwilightZealotsList[i] = NULL;
        }

        InstanceScript* instance;
        EventMap events;
        Creature* TwilightZealotsList[3];
        Creature* NetherEssenceTrigger[3];

        void Reset() override
        {
            events.Reset();
            me->GetMotionMaster()->MoveTargetedHome();
            me->RemoveAllAuras();

            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
            {
                if (TwilightZealotsList[i] == NULL)
                    TwilightZealotsList[i] = me->SummonCreature(NPC_TWILIGHT_ZEALOT_CORLA, summonPositions[i], TEMPSUMMON_MANUAL_DESPAWN);
                if (NetherEssenceTrigger[i] == NULL)
                    NetherEssenceTrigger[i] = TwilightZealotsList[i]->SummonCreature(NPC_NETHER_ESSENCE_TRIGGER, summonPositions[3], TEMPSUMMON_MANUAL_DESPAWN);
                if (TwilightZealotsList[i]->isDead())
                    TwilightZealotsList[i]->Respawn();

                TwilightZealotsList[i]->RemoveAura(SPELL_AURA_OF_ACCELERATION);
                TwilightZealotsList[i]->RemoveAura(SPELL_TWILIGHT_EVOLUTION);
                TwilightZealotsList[i]->RemoveAura(SPELL_EVOLUTION);

                TwilightZealotsList[i]->NearTeleportTo(summonPositions[i].GetPositionX(), summonPositions[i].GetPositionY(), summonPositions[i].GetPositionZ(), summonPositions[i].GetOrientation());

                if (!TwilightZealotsList[i]->HasAura(SPELL_KNEELING_IN_SUPPLICATION))
                    TwilightZealotsList[i]->CastSpell(TwilightZealotsList[i], SPELL_KNEELING_IN_SUPPLICATION, true);
            }
            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
                NetherEssenceTrigger[i]->GetAI()->DoAction(ACTION_TRIGGER_STOP_CHANNELING);
            me->CastSpell(me, SPELL_DRAIN_ESSENCE_CHANNELING, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            me->CastStop();
            me->GetMotionMaster()->MoveChase(me->getVictim());

            events.RescheduleEvent(EVENT_DARK_COMMAND, 20000);

            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
                NetherEssenceTrigger[i]->GetAI()->DoAction(ACTION_TRIGGER_START_CHANNELING);

            NetherEssenceTrigger[0]->CastSpell(NetherEssenceTrigger[0], SPELL_NETHERESSENCE_AURA, true);
            DoCast(SPELL_AURA_OF_ACCELERATION);
            Talk(SAY_AGGRO);
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
                case EVENT_DARK_COMMAND:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_DARK_COMMAND);

                    events.RescheduleEvent(EVENT_DARK_COMMAND, 20000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
                NetherEssenceTrigger[i]->GetAI()->DoAction(ACTION_TRIGGER_STOP_CHANNELING);

            for (uint8 i = 0; i <= RAID_MODE(1, 2); i++)
            {
                TwilightZealotsList[i]->DespawnOrUnsummon();
                TwilightZealotsList[i] = NULL;
            }
            Talk(SAY_DEATH);
        }

        void KilledUnit(Unit* who) override
        {
            if (who && who->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_KILL);
        }

        void JustReachedHome() override
        {
            me->CastSpell(me, SPELL_DRAIN_ESSENCE_CHANNELING, true);
        }
    };
};

class mob_twilight_zealot : public CreatureScript
{
public:
    mob_twilight_zealot() : CreatureScript("mob_twilight_zealot") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_twilight_zealotAI(creature);
    }

    struct mob_twilight_zealotAI : public ScriptedAI
    {
        mob_twilight_zealotAI(Creature* creature) : ScriptedAI(creature), Intialized(false) {}

        bool Intialized;
        EventMap events;

        void Reset() override
        {
            events.Reset();
            Intialized = false;
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 Diff) override
        {
            if (!Intialized && !me->HasAura(SPELL_KNEELING_IN_SUPPLICATION))
            {
                events.RescheduleEvent(EVENT_FORCE_BLAST, 10000);
                events.RescheduleEvent(EVENT_GRAVITY_STRIKE, 22000);
                events.RescheduleEvent(EVENT_GRIEVOUS_WHIRL, 7000);
                events.RescheduleEvent(EVENT_SHADOW_STRIKE, 14000);

                Intialized = true;

                me->SetReactState(REACT_AGGRESSIVE);

                me->GetMotionMaster()->MoveChase(GetPlayerAtMinimumRange(1.0f));
                me->Attack(GetPlayerAtMinimumRange(1.0f), false);
            }
            if (!me->isInCombat() || me->HasAura(SPELL_TWILIGHT_EVOLUTION))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FORCE_BLAST:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                        DoCast(target, SPELL_FORCE_BLAST);

                    events.RescheduleEvent(EVENT_FORCE_BLAST, 10000);
                    break;
                case EVENT_GRAVITY_STRIKE:
                    DoCastVictim(SPELL_GRAVITY_STRIKE);
                    events.RescheduleEvent(EVENT_GRAVITY_STRIKE, 22000);
                    break;
                case EVENT_GRIEVOUS_WHIRL:
                    DoCastAOE(SPELL_GRIEVOUS_WHIRL);
                    events.RescheduleEvent(EVENT_GRIEVOUS_WHIRL, 7000);
                    break;
                case EVENT_SHADOW_STRIKE:
                    DoCastVictim(SPELL_SHADOW_STRIKE);
                    events.RescheduleEvent(EVENT_SHADOW_STRIKE, 14000);
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class mob_corla_netheressence_trigger : public CreatureScript
{
public:
    mob_corla_netheressence_trigger() : CreatureScript("mob_corla_netheressence_trigger") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_corla_netheressence_triggerAI(creature);
    }

    struct mob_corla_netheressence_triggerAI : public ScriptedAI
    {
        mob_corla_netheressence_triggerAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        ObjectGuid zealotGuid;
        ObjectGuid channelTargetGuid;
        ObjectGuid lastTargetGuid;

        Map::PlayerList CharmedPlayerList;

        uint32 uiCheckPlayerIsBetween;
        uint32 uiNetherEssenceVisual;

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!channelTargetGuid || !zealotGuid)
                return;

            Unit* channelTarget = ObjectAccessor::GetUnit(*me, channelTargetGuid);
            Unit* zealot = ObjectAccessor::GetUnit(*me, zealotGuid);
            if (!channelTarget || !zealot)
                return;

            if (zealot->HasAura(SPELL_TWILIGHT_EVOLUTION) || zealot->isDead())
                return;

            if (uiCheckPlayerIsBetween <= diff)
            {
                channelTarget = zealot;
                Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();

                if (!PlayerList.isEmpty())
                {
                    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        if (i->getSource()->IsInBetween(me, zealot, 1.0f))
                            channelTarget = i->getSource();
                }

                me->CastSpell(channelTarget, SPELL_NETHERBEAM, true);

                zealot->SetAuraStack(SPELL_EVOLUTION, channelTarget, channelTarget->GetAuraCount(SPELL_EVOLUTION) + 1);
                if (Aura* aura = channelTarget->GetAura(SPELL_EVOLUTION))
                    aura->RefreshDuration();

                if (channelTarget->GetAuraCount(SPELL_EVOLUTION) == 100)
                {
                    if (channelTarget == zealot)
                        channelTarget->RemoveAllAuras();

                    zealot->CastSpell(channelTarget, SPELL_TWILIGHT_EVOLUTION, true);
                }

                uiCheckPlayerIsBetween = 175;
            }
            else uiCheckPlayerIsBetween -= diff;
        }

        void IsSummonedBy(Unit* summoner) override
        {
            zealotGuid = summoner->GetGUID();
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
            case ACTION_TRIGGER_START_CHANNELING:
                CharmedPlayerList.clearReferences();

                channelTargetGuid = zealotGuid;

                uiCheckPlayerIsBetween = 100;
                lastTargetGuid = me->GetGUID();
                break;

            case ACTION_TRIGGER_STOP_CHANNELING:
                me->RemoveAllAuras();
                lastTargetGuid.Clear();
                channelTargetGuid.Clear();
                break;
            }
        }
    };
};

void AddSC_boss_corla_herald_of_twilight()
{
    new boss_corla_herald_of_twilight();
    new mob_twilight_zealot();
    new mob_corla_netheressence_trigger();
}
