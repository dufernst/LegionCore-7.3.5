#include "end_time.h"

enum Yells
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_INTRO   = 2,
    SAY_KILL    = 3,
    SAY_SPELL   = 4
};

enum Spells
{
    SPELL_BAINE_VISUALS     = 101624,

    SPELL_THROW_TOTEM       = 101615,
    SPELL_PULVERIZE         = 101626, 
    SPELL_PULVERIZE_DMG     = 101627, 
    SPELL_PULVERIZE_AOE     = 101625,
    SPELL_MOLTEN_AXE        = 101836,
    SPELL_MOLTEN_FIST       = 101866,
    SPELL_THROW_TOTEM_BACK  = 101602,
    SPELL_THROW_TOTEM_AURA  = 107837
};

enum Events
{
    EVENT_PULVERIZE     = 1,
    EVENT_THROW_TOTEM   = 2,
    EVENT_CHECK_SELF    = 3
};

enum Adds
{
    NPC_ROCK_ISLAND     = 54496,
    NPC_BAINES_TOTEM    = 54434
};

class boss_echo_of_baine : public CreatureScript
{
    public:
        boss_echo_of_baine() : CreatureScript("boss_echo_of_baine") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetAIForInstance< boss_echo_of_baineAI>(pCreature, ETScriptName);
        }

        struct boss_echo_of_baineAI : public BossAI
        {
            explicit boss_echo_of_baineAI(Creature* pCreature) : BossAI(pCreature, DATA_ECHO_OF_BAINE)
            {
                me->setActive(true);
                bIntroDone = false;
            }

            bool bIntroDone;

            void Reset() override
            {
                _Reset();
                me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 5.0f);
                me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 5.0f);
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _EnterCombat();
                Talk(SAY_AGGRO);

                events.ScheduleEvent(EVENT_PULVERIZE, 60000);
                events.ScheduleEvent(EVENT_THROW_TOTEM, 25000);
                events.ScheduleEvent(EVENT_THROW_TOTEM, 25000);
                events.ScheduleEvent(EVENT_CHECK_SELF, 1000);

                instance->SetBossState(DATA_ECHO_OF_BAINE, IN_PROGRESS);
                DoZoneInCombat();
            }

            void EnterEvadeMode() override
            {
                instance->SetData(DATA_PLATFORMS, NOT_STARTED);
                BossAI::EnterEvadeMode();
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (bIntroDone)
                    return;

                if (!who->IsPlayer())
                    return;

                if (!me->IsWithinDistInMap(who, 100.0f, false))
                    return;

                Talk(SAY_INTRO);
                bIntroDone = true;
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);

                // Quest
                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (me->GetDistance2d(player) <= 50.0f && player->GetQuestStatus(30097) == QUEST_STATUS_INCOMPLETE)
                        DoCast(player, SPELL_ARCHIVED_BAINE, true);
                });
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void MovementInform(uint32 type, uint32 data) override
            {
                if (type == EFFECT_MOTION_TYPE)
                    if (data == EVENT_JUMP)
                        DoCast(SPELL_PULVERIZE_DMG);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_SELF:
                            if (me->GetPositionY() < 1398.0f)
                            {
                                EnterEvadeMode();
                                return;
                            }
                            events.ScheduleEvent(EVENT_CHECK_SELF, 1000);
                            break;
                        case EVENT_PULVERIZE:
                            Talk(SAY_SPELL);
                            DoCast(SPELL_PULVERIZE_AOE);
                            events.ScheduleEvent(EVENT_PULVERIZE, 60000);
                            events.ScheduleEvent(EVENT_THROW_TOTEM, 25000);
                            events.ScheduleEvent(EVENT_THROW_TOTEM, 25000);
                            break;
                        case EVENT_THROW_TOTEM:
                            if (auto target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f))
                                DoCast(target, SPELL_THROW_TOTEM, false);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        private:
            struct PositionSelector : public std::unary_function<Unit*, bool>
            {
                public:
                    bool operator()(Unit const* target) const
                    {
                        if (!target->IsPlayer())
                            return false;

                        if (target->GetAreaId() != AREA_OBSIDIAN)
                            return false;

                        if (target->IsInWater())
                            return false;

                        return true;
                    }
                private:
                    bool _b;
            };
        };      
};

struct npc_echo_of_baine_baines_totem : public Scripted_NoMovementAI
{
    explicit npc_echo_of_baine_baines_totem(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        me->SetReactState(REACT_PASSIVE);
        bDespawn = false;
    }

    bool bDespawn;

    void OnSpellClick(Unit* /*who*/) override
    {
        if (!bDespawn)
        {
            bDespawn = true;
            me->DespawnOrUnsummon();
        }
    }
};

class spell_echo_of_baine_pulverize_aoe : public SpellScript
{
    PrepareSpellScript(spell_echo_of_baine_pulverize_aoe);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<WorldObject*> tempList;
        for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
            if ((*itr)->ToUnit() && !(*itr)->ToUnit()->IsInWater())
                tempList.push_back((*itr));

        if (tempList.size() > 1)
        {
            if (auto victim = caster->getVictim())
                tempList.remove(victim);
        }

        targets.clear();
        if (!tempList.empty())
            targets.push_back(Trinity::Containers::SelectRandomContainerElement(tempList));
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster)
            return;
        if (!target)
            return;

        caster->CastSpell(target, SPELL_PULVERIZE, true);
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_echo_of_baine_pulverize_aoe::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_echo_of_baine_pulverize_aoe::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_boss_echo_of_baine()
{
    RegisterCreatureAI(npc_echo_of_baine_baines_totem);
    RegisterSpellScript(spell_echo_of_baine_pulverize_aoe);
    new boss_echo_of_baine();
}