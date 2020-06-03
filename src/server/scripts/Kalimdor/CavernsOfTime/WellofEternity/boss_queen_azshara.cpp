#include "well_of_eternity.h"
#include "ScriptedEscortAI.h"

enum eEnums
{
    SPELL_COLDFLAME_AURA            = 102465,
    SPELL_COLDFLAME_VISUAL_EFFECT   = 102466,
};

class spell_boss_queen_azshara_drain_sssence : public SpellScriptLoader
{
    public:
        spell_boss_queen_azshara_drain_sssence() : SpellScriptLoader("spell_boss_queen_azshara_drain_sssence") { }

        class spell_boss_queen_azshara_drain_sssence_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_boss_queen_azshara_drain_sssence_AuraScript);

            uint32 numberTick;
            float x, y, z, _angle;

            bool Load()
            {
                numberTick = 1;
                x = y = z = _angle = 0.0f;
                return true;
            }

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if(Unit* target = GetTarget())
                if (Unit* victim = target->getVictim())
                {
                    float dist;
                    Position _pos;
                    if(!x)
                    {
                        victim->GetPosition(x, y, z);
                        _angle = target->GetAngle(x, y);
                    }
                    dist = (target->GetDistance(x, y, z) / 12.0f) * numberTick;
                    target->GetNearPosition(_pos, dist, _angle);
                    numberTick++;
                    target->CastSpell(_pos.GetPositionX(), _pos.GetPositionY(), _pos.GetPositionZ(), SPELL_COLDFLAME_VISUAL_EFFECT, true);
                }
            }

            // function registering
            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_boss_queen_azshara_drain_sssence_AuraScript::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_boss_queen_azshara_drain_sssence_AuraScript();
        }
};

void AddSC_boss_queen_azshara()
{
    new spell_boss_queen_azshara_drain_sssence();
}
