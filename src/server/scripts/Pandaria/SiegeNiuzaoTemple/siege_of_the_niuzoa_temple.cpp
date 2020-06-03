/*==============
==============*/

#include "siege_of_the_niuzoa_temple.h"

enum spells
{
    MALLEABLE_RESIN = 121421,
    RESIN_WEAVING = 121114,
    ENCASED_IN_RESIN = 121116,
    RESIN_SHELL = 120946,
    RESIDUE = 120938
};

struct mob_sikthik_guardian : public ScriptedAI
{
    mob_sikthik_guardian(Creature* creature) : ScriptedAI(creature) {}

    uint32 malleableResinTimer;

    void Reset() override
    {
        malleableResinTimer = urand(5000, 8000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (malleableResinTimer <= diff)
        {
            if (auto target = me->SelectNearestTarget(5.0f))
                if (!target->IsFriendlyTo(me))
                    DoCast(target, MALLEABLE_RESIN, true);

            malleableResinTimer = urand(8000, 12000);
        }
        else
            malleableResinTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct mob_resin_flake : public ScriptedAI
{
    mob_resin_flake(Creature* creature) : ScriptedAI(creature) {}

    uint32 residueTimer;

    void Reset() override
    {
        residueTimer = urand(5000, 7000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (residueTimer <= diff)
        {
            if (auto target = me->SelectNearestTarget(5.0f))
                if (!target->IsFriendlyTo(me))
                    DoCast(target, RESIDUE, true);

            residueTimer = urand(5000, 7000);
        }
        else
            residueTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct mob_sikthik_amber_weaver : public ScriptedAI
{
    mob_sikthik_amber_weaver(Creature* creature) : ScriptedAI(creature) {}

    uint32 resinWeavingTimer;
    uint32 resinShellTimer;
    bool resinShellOnlyOne;

    void Reset() override
    {
        resinWeavingTimer = urand(10000, 12000);
        resinShellTimer = urand(8000, 15000);
        resinShellOnlyOne = true;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (resinWeavingTimer <= diff)
        {
            if (auto target = me->SelectNearestTarget(5.0f))
                if (!target->IsFriendlyTo(me))
                    DoCast(target, RESIN_WEAVING, true);

            resinWeavingTimer = urand(10000, 12000);
        }
        else
            resinWeavingTimer -= diff;

        if (resinShellTimer <= diff && resinShellOnlyOne == true)
        {
            if (auto target = me->SelectNearestTarget(5.0f))
            {
                DoCast(me, RESIN_SHELL, true);
                resinShellOnlyOne = false;
            }
        }
        else
            resinShellTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

class spell_resin_weaving : public AuraScript
{
    PrepareAuraScript(spell_resin_weaving);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        Unit* target = GetTarget();
        if (!target)
            return;

        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_EXPIRE)
            caster->CastSpell(target, ENCASED_IN_RESIN, true);
    }
    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_resin_weaving::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_siege_of_the_niuzoa_temple()
{
    RegisterCreatureAI(mob_sikthik_guardian);
    RegisterCreatureAI(mob_resin_flake);
    RegisterCreatureAI(mob_sikthik_amber_weaver);
    RegisterAuraScript(spell_resin_weaving);
}