/*==============
==============*/

#include "gate_setting_sun.h"

enum spells
{
    SPELL_MANTID_MUNITION_EXPLOSION     = 107153,
    SPELL_EXPLOSE_GATE                  = 115456,

    SPELL_BOMB_CAST_VISUAL              = 106729,
    SPELL_BOMB_AURA                     = 106875,
    
    SPELL_RESIN_RESIDUE                 = 118795,
    SPELL_ACHIEVEMENT_COMPLETE          = 118797
};

struct npc_krikthik_bombarder : public ScriptedAI
{
    npc_krikthik_bombarder(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    uint32 bombTimer;

    void Reset() override
    {
        me->GetMotionMaster()->MoveRandom(5.0f);
        bombTimer = urand(1000, 7500);
    }

    // Called when spell hits a target
    void SpellHitTarget(Unit* target, SpellInfo const* /*spell*/) override
    {
        if (target->GetEntry() == NPC_BOMB_STALKER)
            me->AddAura(SPELL_BOMB_AURA, target);
    }

    void UpdateAI(uint32 diff) override
    {
        if (bombTimer <= diff)
        {
            if (auto stalker = instance->instance->GetCreature(instance->GetGuidData(DATA_RANDOM_BOMB_STALKER)))
                if (!stalker->HasAura(SPELL_BOMB_AURA))
                    DoCast(stalker, SPELL_BOMB_CAST_VISUAL, true);

            bombTimer = urand(1000, 5000);
        }
        else
            bombTimer -= diff;
    }
};

struct npc_krikthik_conscript : public ScriptedAI
{
    npc_krikthik_conscript(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override {}

    void JustDied(Unit* killer) override
    {
        DoCast(killer, SPELL_RESIN_RESIDUE, true);
    }

    void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
    {
        if (attacker->ToCreature() && me->HealthBelowPct(85))
            damage = 0;
    }
};

class spell_resin_residue : public AuraScript
{
    PrepareAuraScript(spell_resin_residue);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (auto aura = target->GetAura(SPELL_RESIN_RESIDUE))
            if (aura->GetStackAmount() > 2)
                target->CastSpell(target, SPELL_ACHIEVEMENT_COMPLETE, true);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_resin_residue::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAPPLY);
    }
};

//8359
class AreaTrigger_at_first_door : public AreaTriggerScript
{
    public:
        AreaTrigger_at_first_door() : AreaTriggerScript("at_first_door") {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/, bool apply) override
        {
            if (InstanceScript* instance = player->GetInstanceScript())
                player->GetInstanceScript()->SetData(DATA_OPEN_FIRST_DOOR, DONE);

            return false;
        }
};

class go_setting_sun_brasier : public GameObjectScript
{
public:
    go_setting_sun_brasier() : GameObjectScript("go_setting_sun_brasier") {}

    bool OnGossipHello(Player* player, GameObject* /*go*/) override
    {
        if (InstanceScript* instance = player->GetInstanceScript())
            player->GetInstanceScript()->SetData(DATA_BRASIER_CLICKED, DONE);

        return false;
    }
};

void AddSC_gate_setting_sun()
{
    RegisterCreatureAI(npc_krikthik_bombarder);
    RegisterCreatureAI(npc_krikthik_conscript);
    RegisterAuraScript(spell_resin_residue);
    //new AreaTrigger_at_first_door();
    new go_setting_sun_brasier();
}
