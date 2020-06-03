/*
    Dungeon : Eye of Azshara 100-110
    Encounter: Lady Hatecoil
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "eye_of_azshara.h"

enum Says
{
    SAY_AGGRO           = 0,
    SAY_NOVA            = 1,
    SAY_NOVA_EMOTE      = 2,
    SAY_STORM_EMOTE     = 3,
    SAY_STORM           = 4,
    SAY_FOCUSED         = 5,
    SAY_DEATH           = 6,
};

enum Spells
{
    SPELL_ARCANE_SHIELDING      = 197868,
    SPELL_CRACKLING_THUNDER     = 197324,
    SPELL_CRACKLING_THUNDER_DMG = 197326,
    SPELL_STATIC_NOVA           = 193597,
    SPELL_CURSE_WITCH_1         = 193712, //MaxTargets 1
    SPELL_CURSE_WITCH_2         = 193716, //MaxTargets 3
    SPELL_CURSE_WITCH_3         = 193717, //MaxTargets 5
    SPELL_CURSE_WITCH_AURA      = 193698,
    SPELL_CURSE_WITCH_KNOCK     = 193700,
    SPELL_CURSE_WITCH_KILL_G    = 193720, //Hit npc: 98293
    SPELL_CURSE_WITCH_ROOT      = 194197,
    SPELL_BECKON_STORM          = 193682,
    SPELL_FOCUSED_LIGHTNING     = 193611,
    SPELL_MONSOON_FILTER_1      = 196629, //MaxTargets 1
    SPELL_MONSOON_FILTER_2      = 196634, //MaxTargets 3
    SPELL_MONSOON_FILTER_3      = 196635, //MaxTargets 5
    SPELL_MONSOON_SUM           = 196630,
    SPELL_MONSOON_VISUAL        = 196609,
    SPELL_MONSOON_SEARCH_PLR    = 196624,
    SPELL_MONSOON_FIXATE        = 196622,

    SPELL_SAND_DUNE_GO          = 193060,
    SPELL_SAND_DUNE_AT          = 193064,
};

enum eEvents
{
    EVENT_CRACKLING_THUNDER     = 1,
    EVENT_STATIC_NOVA           = 2,
    EVENT_CURSE_WITCH           = 3,
    EVENT_BECKON_STORM          = 4,
    EVENT_FOCUSED_LIGHTNING     = 5,
    EVENT_MONSOON               = 6, //Heroic
};

uint32 curseSpells[3] =
{
    SPELL_CURSE_WITCH_1,
    SPELL_CURSE_WITCH_2,
    SPELL_CURSE_WITCH_3
};

Position const homePos = {-3443.38f, 4590.50f, 0.0f};

Position const nagPos[4] =
{
    {-3358.20f, 4634.52f, 0.45f, 2.76f},
    {-3457.37f, 4760.65f, 4.28f, 3.50f},
    {-3555.38f, 4741.02f, 4.76f, 5.77f},
    {-3652.25f, 4623.6f, 15.37f, 3.55f}
};

//91789
class boss_lady_hatecoil : public CreatureScript
{
public:
    boss_lady_hatecoil() : CreatureScript("boss_lady_hatecoil") {}

    struct boss_lady_hatecoilAI : public BossAI
    {
        boss_lady_hatecoilAI(Creature* creature) : BossAI(creature, DATA_HATECOIL) 
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            SummonNagas();
        }

        uint8 nagaDiedCount = 0;
        uint8 monsoonCount = 0;

        void Reset() override
        {
            DelSandDune();
            _Reset();

            monsoonCount = 0;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_AGGRO);
            _EnterCombat();

            events.RescheduleEvent(EVENT_CRACKLING_THUNDER, 4000);
            events.RescheduleEvent(EVENT_STATIC_NOVA, 11000);
            events.RescheduleEvent(EVENT_CURSE_WITCH, 17000);
            events.RescheduleEvent(EVENT_BECKON_STORM, 19000);
            events.RescheduleEvent(EVENT_FOCUSED_LIGHTNING, 25000);

            if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
                events.RescheduleEvent(EVENT_MONSOON, 32000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_CURSE_WITCH_1:
                case SPELL_CURSE_WITCH_2:
                case SPELL_CURSE_WITCH_3:
                    DoCast(target, SPELL_CURSE_WITCH_AURA, true);
                    break;
                case SPELL_MONSOON_FILTER_1:
                case SPELL_MONSOON_FILTER_2:
                case SPELL_MONSOON_FILTER_3:
                    DoCast(target, SPELL_MONSOON_SUM, true);
                    break;
            }   
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == NPC_HATECOIL_ARCANIST)
            {
                ++nagaDiedCount;

                if (nagaDiedCount == 1)
                {
                    summon->AI()->Talk(0);

                    if (auto aura = me->GetAura(SPELL_ARCANE_SHIELDING))
                        aura->ModStackAmount(-1);
                }
                else if (nagaDiedCount == 2)
                {
                    summon->AI()->Talk(1);
                    me->GetMotionMaster()->MovePoint(1, me->GetHomePosition(), false);
                }
            }
            
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            me->SetDisableGravity(false);
            me->RemoveAurasDueToSpell(SPELL_ARCANE_SHIELDING);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void DelSandDune()
        {
            std::list<GameObject*> duneList;
            me->GetGameObjectListWithEntryInGrid(duneList, GO_SAND_DUNE, 100.0f);
            for (std::list<GameObject*>::iterator itr = duneList.begin(); itr != duneList.end(); ++itr)
                (*itr)->Delete();
        }

        void SummonNagas()
        {
            me->SetDisableGravity(true);
            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 14.0f, me->GetOrientation());

            for (uint8 i = 0; i < 4; ++i)
            {
                auto naga = me->SummonCreature(NPC_HATECOIL_ARCANIST, nagPos[i]);
                if (!naga)
                    return;

                if (i < 2)
                    DoCast(me, SPELL_ARCANE_SHIELDING, true);
                if (i == 3)
                    naga->GetMotionMaster()->MovePath(9717100, true);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (me->GetDistance(me->GetHomePosition()) >= 50.0f)
            {
                EnterEvadeMode();
                return;
            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CRACKLING_THUNDER:
                        DoCast(me, SPELL_CRACKLING_THUNDER, true);
                        events.RescheduleEvent(EVENT_CRACKLING_THUNDER, 4000);
                        break;
                    case EVENT_STATIC_NOVA:
                        DoCast(SPELL_STATIC_NOVA);
                        Talk(SAY_NOVA);
                        Talk(SAY_NOVA_EMOTE);
                        events.RescheduleEvent(EVENT_STATIC_NOVA, 35000);
                        break;
                    case EVENT_CURSE_WITCH:
                        DoCast(curseSpells[urand(0, 2)]);
                        events.RescheduleEvent(EVENT_CURSE_WITCH, 24000);
                        break;
                    case EVENT_BECKON_STORM:
                        DoCast(SPELL_BECKON_STORM);
                        Talk(SAY_STORM_EMOTE);
                        Talk(SAY_STORM);
                        events.RescheduleEvent(EVENT_BECKON_STORM, 47000);
                        break;
                    case EVENT_FOCUSED_LIGHTNING:
                        DoCast(SPELL_FOCUSED_LIGHTNING);
                        Talk(SAY_FOCUSED);
                        events.RescheduleEvent(EVENT_FOCUSED_LIGHTNING, 36000);
                        break;
                    case EVENT_MONSOON:
                        if (monsoonCount < 2)
                            DoCast(SPELL_MONSOON_FILTER_1);
                        else if (monsoonCount >= 2 && monsoonCount <= 5)
                            DoCast(SPELL_MONSOON_FILTER_2);
                        else if (monsoonCount > 5)
                            DoCast(SPELL_MONSOON_FILTER_3);
                        monsoonCount++;
                        events.RescheduleEvent(EVENT_MONSOON, 21000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_lady_hatecoilAI (creature);
    }
};

//97853
class npc_hatecoil_sand_dune : public CreatureScript
{
public:
    npc_hatecoil_sand_dune() : CreatureScript("npc_hatecoil_sand_dune") {}

    struct npc_hatecoil_sand_duneAI : public ScriptedAI
    {
        npc_hatecoil_sand_duneAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(me, SPELL_SAND_DUNE_GO, true);
            DoCast(me, SPELL_SAND_DUNE_AT, true);
        }

        void SpellHit(Unit* target, const SpellInfo* spell) override
        {
            if (spell->Id == 193625 || spell->Id == 196614)
            {
                me->RemoveAreaObject(SPELL_SAND_DUNE_AT);
                me->RemoveAurasDueToSpell(SPELL_SAND_DUNE_AT);
                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hatecoil_sand_duneAI(creature);
    }
};

//99852
class npc_hatecoil_monsoon : public CreatureScript
{
public:
    npc_hatecoil_monsoon() : CreatureScript("npc_hatecoil_monsoon") {}

    struct npc_hatecoil_monsoonAI : public ScriptedAI
    {
        npc_hatecoil_monsoonAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        bool spawn;
        uint16 searchPlrTimer;
        Unit* playerTarget = nullptr;

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            DoZoneInCombat(me, 100.0f);
            searchPlrTimer = 2000;
            spawn = false;
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_MONSOON_FIXATE:
                    me->AddThreat(target, 10000.0f);
                    AttackStart(target);
                    playerTarget = target;
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (searchPlrTimer <= diff)
            {
                if (!playerTarget || !playerTarget->isAlive())
                {
                    if (!spawn)
                    {
                        spawn = true;
                        DoCast(me, SPELL_MONSOON_VISUAL, true);
                    }
                    playerTarget = nullptr;
                    DoCast(me, SPELL_MONSOON_SEARCH_PLR, true);
                }
                searchPlrTimer = 2000;
            }
            else
                searchPlrTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hatecoil_monsoonAI(creature);
    }
};

//197324
class spell_crackling_thunder_filter : public SpellScriptLoader
{
public:
    spell_crackling_thunder_filter() : SpellScriptLoader("spell_crackling_thunder_filter") { }

    class spell_crackling_thunder_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_crackling_thunder_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            std::list<WorldObject*> tempList;
            for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                if ((*itr)->GetDistance(homePos) > 45.0f)
                    tempList.push_back((*itr));

            targets.clear();
            targets = tempList;
        }

        void HandleOnHit()
        {
            if (!GetCaster() || !GetHitUnit())
                return;

            GetCaster()->CastSpell(GetHitUnit(), SPELL_CRACKLING_THUNDER_DMG, true);
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_crackling_thunder_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_crackling_thunder_filter_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_crackling_thunder_filter_SpellScript();
    }
};

//193698
class spell_hatecoil_curse_of_the_witch : public SpellScriptLoader
{
    public:
        spell_hatecoil_curse_of_the_witch() : SpellScriptLoader("spell_hatecoil_curse_of_the_witch") { }

        class spell_hatecoil_curse_of_the_witch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hatecoil_curse_of_the_witch_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster() || !GetTarget())
                    return;

                GetTarget()->CastSpell(GetTarget(), SPELL_CURSE_WITCH_KNOCK, true, nullptr, nullptr, GetCasterGUID());
                GetTarget()->CastSpell(GetTarget(), SPELL_CURSE_WITCH_KILL_G, true);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_hatecoil_curse_of_the_witch_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hatecoil_curse_of_the_witch_AuraScript();
        }
};

void AddSC_boss_lady_hatecoil()
{
    new boss_lady_hatecoil();
    new npc_hatecoil_sand_dune();
    new npc_hatecoil_monsoon();
    new spell_crackling_thunder_filter();
    new spell_hatecoil_curse_of_the_witch();
}