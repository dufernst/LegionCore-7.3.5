/*====================
======================*/

#include "stormstout_brewery.h"

enum eEvents
{
    EVENT_WP_1      = 1,
    EVENT_WP_2      = 2,
    EVENT_WP_3      = 3,
    EVENT_WP_4      = 4
};

const Position ePos[6] =
{
    {-717.66f, 1308.82f, 163.0f},
    {-748.75f, 1321.84f, 163.0f},
    {-770.68f, 1281.18f, 163.0f},
    {-761.24f, 1247.89f, 163.0f},
    {-758.56f, 1237.43f, 163.0f},
    {-775.34f, 1215.79f, 169.0f}
};

struct npc_hopling : public ScriptedAI
{
    npc_hopling(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        move = false;
    }

    InstanceScript* instance;
    EventMap events;
    bool move;

    void IsSummonedBy(Unit* owner) override
    {
        if (owner->GetEntry() == NPC_TRIGGER_SUMMONER)
        {
            move = true;
            me->setFaction(14);
            me->GetMotionMaster()->MovePoint(1, ePos[0]);
        }
    }

    void EnterEvadeMode() override
    {
        me->DespawnOrUnsummon();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (id)
            {
            case 1:
                events.RescheduleEvent(EVENT_WP_1, 0);
                break;
            case 2:
                events.RescheduleEvent(EVENT_WP_2, 0);
                break;
            case 3:
                events.RescheduleEvent(EVENT_WP_3, 0);
                break;
            case 4:
                events.RescheduleEvent(EVENT_WP_4, 0);
                break;
            case 5:
                me->GetMotionMaster()->MoveJump(ePos[5].GetPositionX() + irand(-10, 10), ePos[5].GetPositionY() + irand(-10, 10), ePos[5].GetPositionZ(), 15, 15);
                me->DespawnOrUnsummon(1000);
                break;
            }
        }
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (instance)
        {
            if (spell->Id == SPELL_SMASH_DMG)
            {
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 20, 10, 18);

                uint32 HoplingCount = instance->GetData(DATA_HOPLING) + 1;
                instance->SetData(DATA_HOPLING, HoplingCount);

                if (HoplingCount == 100)
                    DoCast(SPELL_SMASH_ACHIEV);

                me->DespawnOrUnsummon(1000);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && !move)
            return;

        events.Update(diff);
        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_WP_1:
                me->GetMotionMaster()->MovePoint(2, ePos[1]);
                break;
            case EVENT_WP_2:
                me->GetMotionMaster()->MovePoint(3, ePos[2]);
                break;
            case EVENT_WP_3:
                me->GetMotionMaster()->MovePoint(4, ePos[3]);
                break;
            case EVENT_WP_4:
                me->GetMotionMaster()->MovePoint(5, ePos[4]);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_golden_hopling : public ScriptedAI
{
    npc_golden_hopling(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        OneClick = false;
    }

    InstanceScript* instance;
    bool OneClick;

    void OnSpellClick(Unit* /*clicker*/) override
    {
        if (instance && !OneClick)
        {
            OneClick = true;
            uint32 GoldenHoplingCount = instance->GetData(DATA_GOLDEN_HOPLING) + 1;
            instance->SetData(DATA_GOLDEN_HOPLING, GoldenHoplingCount);

            if (GoldenHoplingCount >= 30)
                DoCast(SPELL_GOLDEN_VERMING_ACHIEV);

            me->DespawnOrUnsummon();
        }
    }
};

struct npc_big_ol_hammer : public ScriptedAI
{
    npc_big_ol_hammer(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        OneClick = false;
    }

    InstanceScript* instance;
    bool OneClick;

    void OnSpellClick(Unit* clicker) override
    {
        if (instance && !OneClick)
        {
            OneClick = true;

            for (uint8 i = 0; i < 2; ++i)
                clicker->CastSpell(clicker, SPELL_SMASH_OVERRIDE);

            me->DespawnOrUnsummon();
        }
    }
};

class spell_stormstout_brewery_habanero_beer : public SpellScript
{
    PrepareSpellScript(spell_stormstout_brewery_habanero_beer);

    void HandleInstaKill(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Creature*> creatureList;
        GetCreatureListWithEntryInGrid(creatureList, caster, NPC_BARREL, 10.0f);
        caster->RemoveAurasDueToSpell(SPELL_PROC_EXPLOSION);

        for (auto& cre : creatureList)
        {
            if (cre->HasAura(SPELL_PROC_EXPLOSION))
            {
                cre->RemoveAurasDueToSpell(SPELL_PROC_EXPLOSION);
                cre->CastSpell(cre, GetSpellInfo()->Id, true);
            }
        }
    }

    void HandleAfterCast()
    {
        if (auto caster = GetCaster()->ToCreature())
            caster->ForcedDespawn(1000);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_stormstout_brewery_habanero_beer::HandleInstaKill, EFFECT_1, SPELL_EFFECT_INSTAKILL);
        AfterCast += SpellCastFn(spell_stormstout_brewery_habanero_beer::HandleAfterCast);
    }
};

class spell_hopling_summoner : public AuraScript
{
    PrepareAuraScript(spell_hopling_summoner)

    void OnPeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_HOPLING_SUMM_3, false);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hopling_summoner::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

class spell_sb_smash : public SpellScript
{
    PrepareSpellScript(spell_sb_smash);

    void HandleScript(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        target->RemoveAuraFromStack(SPELL_SMASH_OVERRIDE);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_sb_smash::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_stormstout_brewery()
{
    RegisterCreatureAI(npc_hopling);
    RegisterCreatureAI(npc_golden_hopling);
    RegisterCreatureAI(npc_big_ol_hammer);
    RegisterSpellScript(spell_stormstout_brewery_habanero_beer);
    RegisterSpellScript(spell_sb_smash);
    RegisterAuraScript(spell_hopling_summoner);
}