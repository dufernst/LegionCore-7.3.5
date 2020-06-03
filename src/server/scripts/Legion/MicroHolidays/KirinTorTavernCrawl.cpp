/*
    The event is implemented through the SmartAI!
*/

#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "Spell.h"
#include "ScriptedCreature.h"

struct Location
{
    float x;
    float y;
    float z;
    float o;
    uint32 mapId;
};

Location const tpPos[10] =
{
    { -191.659f, 5518.12f, 27.9408f, 0.3023731f, 530 },
    { -696.62f, 1413.67f, 135.5644f, 0.272403f, 870 },
    { -7060.45f, -3852.34f, 9.9125f, 3.00167f, 1 },
    { -1760.06f, 5165.45f, -37.2043f, 5.02799f, 530 },
    { -6840.18f, 743.94f, 42.6232f, 3.59464f, 1 },
    { 7888.75f, -2509.41f, 489.35101f, 1.62316f, 1 },
    { 6689.64f, -4669.78f, 721.6306f, 3.139107f, 1 },
    { -14469.07f, 493.83f, 15.1133f, 5.55027f, 0 },
    { 816.89f, -167.37f, 415.2347f, 5.76423f, 870 },
    { -1031.57f, -3669.53f, 23.0296f, 2.92152f, 1 }
};

uint32 Random_Tavern_Finery[24]
{
    235773,
    236399,
    236401,
    236402,
    236398,
    236404,
    236405,
    236406,
    236407,
    236408,
    236397,
    236409,
    236396,
    236410,
    236395,
    236411,
    236394,
    236412,
    235925,
    236413,
    235776,
    236414,
    236403,
    236400
};

class spell_random_tp_on_hs : public SpellScript
{
    PrepareSpellScript(spell_random_tp_on_hs);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* caster = GetCaster();
        if (!caster)
            return;
        if (!caster->IsPlayer())
            return;

        uint8 rand = urand(0, 9);
        caster->ToPlayer()->TeleportTo(tpPos[rand].mapId, tpPos[rand].x, tpPos[rand].y, tpPos[rand].z, tpPos[rand].o);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_random_tp_on_hs::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_TELEPORT_L);
    }
};

class spell_lost_and_fount_hats : public SpellScript
{
    PrepareSpellScript(spell_lost_and_fount_hats);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint8 rand = urand(0, 23);
        caster->CastSpell(caster, Random_Tavern_Finery[rand], true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_lost_and_fount_hats::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_KirinTorTavernCrawl()
{
    RegisterSpellScript(spell_random_tp_on_hs);
    RegisterSpellScript(spell_lost_and_fount_hats);
}