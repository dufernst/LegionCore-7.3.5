#include "bloodmaul_slag_mines.h"
#include "Garrison.h"

enum Texts
{
    TEXT_0, // 'Сломать вас будет удовольствием.'
    TEXT_1, // 'Сначала сломлю ваш дух, потом продам в рабство.'
    TEXT_2, // 'Горите!'
    TEXT_3, // 'Стихии повинуются мне!
    TEXT_4, // 'Вам… не видать... свободы…'
    TEXT_5, // '|TInterface\\Icons\\spell_fire_elementaldevastation.blp:20|t |cFFF00000|Hspell:150678|h[Огненные Недра]|h|r заставляют |3-3(%s) вызвать |cFFF00000|Hspell:150682|h[Лавовый залп]|h|r!'
    TEXT_6, // 'Вы будете рабами лавы!'
    TEXT_7, // '|TInterface\\Icons\\spell_fire_selfdestruct.blp:20|t %s: |cFFF00000|Hspell:150734|h[Необузданное Пламя]|h|r достигает максимальной силы!'
};

enum Spells
{
    SPELL_MOLTEN_BLAST              = 150677,

    SPELL_MAGMA_ERUPTION_COSMETIC   = 150776,
    SPELL_MAGMA_ERUPTION_SUMMON     = 150764,
    SPELL_MAGMA_ERUPTION_AURA       = 150784, //< casted by magma to players in 8 yards...
    SPELL_MAGMA_ERUPRION_A_TRIGGER  = 150783, //< triggered by SPELL_MAGMA_ERUPTION_SSC
    SPELL_MAGMA_ERUPTION_SSC        = 150777,

    SPELL_SUMMON_UNSTABLE_SLAG      = 150755,
    SPELL_MOLTEN_CORE               = 150678,
    SPELL_MOLTEN_BARRAGE            = 150682,
    SPELL_EMPOWERED_FLAMES          = 152091,
    SPELL_FLAME_BUFFET              = 163802,

    //< Unstable Slag
    SPELL_ENSLAVE_SLAG              = 150756, //< casted on self before start movement
    SPELL_SIPHON_FLAMES             = 150732,
    SPELL_UNLEASHED_FLAMES          = 150734, //< triggered by SPELL_SIPHON_FLAMES

    //Christmas
    SPELL_CHRISTMAS_CAP             = 176925
};

enum NPCs
{
    NPC_UNSTABLE_SLAG               = 74927,
    NPC_SLG_GENERIC_MOP             = 68553,
    NPC_MAGMA_ERUPTION              = 74967
};

struct boss_gugrokk : public BossAI
{
    boss_gugrokk(Creature* creature) : BossAI(creature, DATA_GUGROKK) {}

    void Reset() override
    {
        _Reset();

        if (sGameEventMgr->IsActiveEvent(2))
            DoCast(SPELL_CHRISTMAS_CAP);
        else
        {
            if (me->HasAura(SPELL_CHRISTMAS_CAP))
                me->RemoveAura(SPELL_CHRISTMAS_CAP);
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();

        events.RescheduleEvent(EVENT_1, 10 * IN_MILLISECONDS);
        events.RescheduleEvent(EVENT_3, urand(5, 7) * IN_MILLISECONDS);
        events.RescheduleEvent(EVENT_4, 20 * IN_MILLISECONDS);

        if (IsHeroic())
            events.RescheduleEvent(EVENT_5, 6 * IN_MILLISECONDS);

        Talk(TEXT_0);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        Talk(TEXT_4);

        if (instance->GetData(DATA_CROMAN_PROGRESS) == IN_PROGRESS)
        {
            instance->instance->ApplyOnEveryPlayer([&](Player* player)
            {
                if (auto garrison = player->GetGarrison())
                    garrison->AddFollower(177); //< Croman
            });
        }

        summons.DespawnAll();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void EnterEvadeMode() override
    {
        CreatureAI::EnterEvadeMode();
        summons.DespawnAll();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->GetDistance(me->GetHomePosition()) >= 120.0f)
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
            case EVENT_5:
                events.RescheduleEvent(EVENT_5, urand(10, 19) * IN_MILLISECONDS);

                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_FLAME_BUFFET, false);

                Talk(TEXT_6);
                break;
            case EVENT_1:
                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    me->CastSpell(player->GetPosition(), SPELL_MAGMA_ERUPTION_SUMMON, false);
                });
                Talk(TEXT_1);
                events.RescheduleEvent(EVENT_1, urand(13, 18) * IN_MILLISECONDS);
                DoCast(SPELL_MAGMA_ERUPTION_COSMETIC);
                break;
            case EVENT_3:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_MOLTEN_BLAST, false);

                me->AddAura(SPELL_MOLTEN_CORE, me);

                if (me->GetAura(SPELL_MOLTEN_CORE)->GetStackAmount() == 3)
                    events.RescheduleEvent(EVENT_7, 2 * IN_MILLISECONDS);

                events.RescheduleEvent(EVENT_3, urand(8, 9) * IN_MILLISECONDS);
                break;
            case EVENT_7:
                Talk(TEXT_5);
                // DoCastToAllHostilePlayers(SPELL_MOLTEN_BARRAGE);
                me->RemoveAura(SPELL_MOLTEN_CORE);
                break;
            case EVENT_4:
            {
                events.RescheduleEvent(EVENT_4, 20 * IN_MILLISECONDS);
                events.RescheduleEvent(EVENT_6, 3 * IN_MILLISECONDS);

                float x = 0.0f, y = 0.0f;
                GetRandPosFromCenterInDist(me->GetPositionX(), me->GetPositionY(), 7.0f, x, y);
                me->CastSpell(x, y, me->GetPositionZ(), SPELL_SUMMON_UNSTABLE_SLAG, false);
                Talk(TEXT_2);
                break;
            }
            case EVENT_6:
                if (auto slag = me->FindNearestCreature(NPC_UNSTABLE_SLAG, 150.0f))
                {
                    events.RescheduleEvent(EVENT_10, 10 * IN_MILLISECONDS);
                    slag->AI()->DoAction(ACTION_1);
                    Talk(TEXT_3);
                }
                break;
            case EVENT_10:
                Talk(TEXT_7);
                break;
            default:
                break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_bloodmaul_magma_eruption : public ScriptedAI
{
    npc_bloodmaul_magma_eruption(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;
    uint32 aura_timer;

    void Reset() override
    {
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        me->AddAura(SPELL_MAGMA_ERUPRION_A_TRIGGER, me);
        me->SetVisible(false);

        aura_timer = 1 * IN_MILLISECONDS;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (me->IsWithinDistInMap(who, 5.0f) && !who->GetAuraEffect(SPELL_MAGMA_ERUPTION_AURA, EFFECT_0, me->GetGUID()))
                me->AddAura(SPELL_MAGMA_ERUPTION_AURA, who);

            if (!me->IsWithinDistInMap(who, 5.0f) && who->GetAuraEffect(SPELL_MAGMA_ERUPTION_AURA, EFFECT_0, me->GetGUID()))
                who->RemoveAura(SPELL_MAGMA_ERUPTION_AURA);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (aura_timer <= diff)
        {
            me->AddAura(SPELL_MAGMA_ERUPTION_SSC, me);
            aura_timer = 2 * IN_MILLISECONDS;
        }
        else
            aura_timer -= diff;
    }
};

struct npc_bloodmaul_unstable_slag : public ScriptedAI
{
    npc_bloodmaul_unstable_slag(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    bool reached;
    EventMap events;
    uint32 checkTimer;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        reached = false;

        checkTimer = 1 * IN_MILLISECONDS;
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            DoCast(SPELL_ENSLAVE_SLAG);
            me->SetReactState(REACT_PASSIVE);
            reached = false;
            events.RescheduleEvent(EVENT_8, 2 * IN_MILLISECONDS);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->DespawnOrUnsummon(3 * IN_MILLISECONDS);
    }

    void EnterEvadeMode() override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->DespawnOrUnsummon(3 * IN_MILLISECONDS);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (checkTimer <= diff && !reached)
        {
            checkTimer = 2 * IN_MILLISECONDS;

            if (auto target = me->FindNearestCreature(NPC_SLG_GENERIC_MOP, 100.0f))
            {
                if (me->IsInDist2d(target->GetPositionX(), target->GetPositionY(), 4.0f) && !me->HasAura(SPELL_EMPOWERED_FLAMES))
                {
                    me->AddAura(SPELL_EMPOWERED_FLAMES, me);
                    reached = true;

                    if (reached && (me->GetReactState() != REACT_AGGRESSIVE))
                        me->SetReactState(REACT_AGGRESSIVE);
                }
            }
        }
        else
            checkTimer -= diff;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_8:
                if (auto target = me->FindNearestCreature(NPC_SLG_GENERIC_MOP, 100.0f))
                {
                    me->GetMotionMaster()->MoveChase(target, 3.0f);
                    DoCast(target, SPELL_SIPHON_FLAMES, false);
                    DoCast(target, SPELL_UNLEASHED_FLAMES, false);
                }
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

class TargetFilter
{
public:
    TargetFilter(Unit* caster) : victim(caster->getVictim()) { }

    bool operator()(WorldObject* target)
    {
        if (target == victim && target->GetEntry() == NPC_SLG_GENERIC_MOP)
            return false;

        return true;
    }

private:
    Unit* victim;
};


class spell_unleashed_flames : public SpellScript
{
    PrepareSpellScript(spell_unleashed_flames);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if(TargetFilter(GetCaster()));
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_unleashed_flames::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_unleashed_flames::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_unleashed_flames::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

void AddSC_boss_gugrokk()
{
    RegisterCreatureAI(boss_gugrokk);
    RegisterCreatureAI(npc_bloodmaul_magma_eruption);
    RegisterCreatureAI(npc_bloodmaul_unstable_slag);
    RegisterSpellScript(spell_unleashed_flames);
}
