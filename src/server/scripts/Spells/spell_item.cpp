/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Scripts for spells with SPELLFAMILY_GENERIC spells used by items.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_item_".
 */

#include "QuestData.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SkillDiscovery.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "PlayerDefines.h"

// Generic script for handling item dummy effects which trigger another spell.
class spell_item_trigger_spell : public SpellScriptLoader
{
    private:
        uint32 _triggeredSpellId;

    public:
        spell_item_trigger_spell(const char* name, uint32 triggeredSpellId) : SpellScriptLoader(name), _triggeredSpellId(triggeredSpellId) { }

        class spell_item_trigger_spell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_trigger_spell_SpellScript);
        private:
            uint32 _triggeredSpellId;

        public:
            spell_item_trigger_spell_SpellScript(uint32 triggeredSpellId) : SpellScript(), _triggeredSpellId(triggeredSpellId) { }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(_triggeredSpellId))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Item* item = GetCastItem())
                    caster->CastSpell(caster, _triggeredSpellId, true, item);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_trigger_spell_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_trigger_spell_SpellScript(_triggeredSpellId);
        }
};

// http://www.wowhead.com/item=6522 Deviate Fish
// 8063 Deviate Fish
enum DeviateFishSpells
{
    SPELL_SLEEPY            = 8064,
    SPELL_INVIGORATE        = 8065,
    SPELL_SHRINK            = 8066,
    SPELL_PARTY_TIME        = 8067,
    SPELL_HEALTHY_SPIRIT    = 8068,
};

class spell_item_deviate_fish : public SpellScriptLoader
{
    public:
        spell_item_deviate_fish() : SpellScriptLoader("spell_item_deviate_fish") { }

        class spell_item_deviate_fish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_deviate_fish_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                for (uint32 spellId = SPELL_SLEEPY; spellId <= SPELL_HEALTHY_SPIRIT; ++spellId)
                    if (!sSpellMgr->GetSpellInfo(spellId))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = urand(SPELL_SLEEPY, SPELL_HEALTHY_SPIRIT);
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_deviate_fish_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_deviate_fish_SpellScript();
        }
};

// http://www.wowhead.com/item=47499 Flask of the North
// 67019 Flask of the North
enum FlaskOfTheNorthSpells
{
    SPELL_FLASK_OF_THE_NORTH_SP = 67016,
    SPELL_FLASK_OF_THE_NORTH_AP = 67017,
    SPELL_FLASK_OF_THE_NORTH_STR = 67018,
};

class spell_item_flask_of_the_north : public SpellScriptLoader
{
    public:
        spell_item_flask_of_the_north() : SpellScriptLoader("spell_item_flask_of_the_north") { }

        class spell_item_flask_of_the_north_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_flask_of_the_north_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_FLASK_OF_THE_NORTH_SP) || !sSpellMgr->GetSpellInfo(SPELL_FLASK_OF_THE_NORTH_AP) || !sSpellMgr->GetSpellInfo(SPELL_FLASK_OF_THE_NORTH_STR))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                std::vector<uint32> possibleSpells;
                switch (caster->getClass())
                {
                    case CLASS_WARLOCK:
                    case CLASS_MAGE:
                    case CLASS_PRIEST:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_SP);
                        break;
                    case CLASS_DEATH_KNIGHT:
                    case CLASS_WARRIOR:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_STR);
                        break;
                    case CLASS_ROGUE:
                    case CLASS_HUNTER:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_AP);
                        break;
                    case CLASS_DRUID:
                    case CLASS_PALADIN:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_SP);
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_STR);
                        break;
                    case CLASS_SHAMAN:
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_SP);
                        possibleSpells.push_back(SPELL_FLASK_OF_THE_NORTH_AP);
                        break;
                }

                caster->CastSpell(caster, possibleSpells[irand(0, (possibleSpells.size() - 1))], true, NULL);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_flask_of_the_north_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_flask_of_the_north_SpellScript();
        }
};

// http://www.wowhead.com/item=10645 Gnomish Death Ray
// 13280 Gnomish Death Ray
enum GnomishDeathRay
{
    SPELL_GNOMISH_DEATH_RAY_SELF = 13493,
    SPELL_GNOMISH_DEATH_RAY_TARGET = 13279,
};

class spell_item_gnomish_death_ray : public SpellScriptLoader
{
    public:
        spell_item_gnomish_death_ray() : SpellScriptLoader("spell_item_gnomish_death_ray") { }

        class spell_item_gnomish_death_ray_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_gnomish_death_ray_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_GNOMISH_DEATH_RAY_SELF) || !sSpellMgr->GetSpellInfo(SPELL_GNOMISH_DEATH_RAY_TARGET))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    if (urand(0, 99) < 15)
                        caster->CastSpell(caster, SPELL_GNOMISH_DEATH_RAY_SELF, true, NULL);    // failure
                    else
                        caster->CastSpell(target, SPELL_GNOMISH_DEATH_RAY_TARGET, true, NULL);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_gnomish_death_ray_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_gnomish_death_ray_SpellScript();
        }
};

// http://www.wowhead.com/item=27388 Mr. Pinchy
// 33060 Make a Wish
enum MakeAWish
{
    SPELL_MR_PINCHYS_BLESSING       = 33053,
    SPELL_SUMMON_MIGHTY_MR_PINCHY   = 33057,
    SPELL_SUMMON_FURIOUS_MR_PINCHY  = 33059,
    SPELL_TINY_MAGICAL_CRAWDAD      = 33062,
    SPELL_MR_PINCHYS_GIFT           = 33064,
};

class spell_item_make_a_wish : public SpellScriptLoader
{
    public:
        spell_item_make_a_wish() : SpellScriptLoader("spell_item_make_a_wish") { }

        class spell_item_make_a_wish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_make_a_wish_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MR_PINCHYS_BLESSING) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_MIGHTY_MR_PINCHY) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_FURIOUS_MR_PINCHY) || !sSpellMgr->GetSpellInfo(SPELL_TINY_MAGICAL_CRAWDAD) || !sSpellMgr->GetSpellInfo(SPELL_MR_PINCHYS_GIFT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = SPELL_MR_PINCHYS_GIFT;
                switch (urand(1, 5))
                {
                    case 1: spellId = SPELL_MR_PINCHYS_BLESSING; break;
                    case 2: spellId = SPELL_SUMMON_MIGHTY_MR_PINCHY; break;
                    case 3: spellId = SPELL_SUMMON_FURIOUS_MR_PINCHY; break;
                    case 4: spellId = SPELL_TINY_MAGICAL_CRAWDAD; break;
                }
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_make_a_wish_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_make_a_wish_SpellScript();
        }
};

// http://www.wowhead.com/item=32686 Mingo's Fortune Giblets
// 40802 Mingo's Fortune Generator
class spell_item_mingos_fortune_generator : public SpellScriptLoader
{
    public:
        spell_item_mingos_fortune_generator() : SpellScriptLoader("spell_item_mingos_fortune_generator") { }

        class spell_item_mingos_fortune_generator_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_mingos_fortune_generator_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                // Selecting one from Bloodstained Fortune item
                uint32 newitemid;
                switch (urand(1, 20))
                {
                    case 1:  newitemid = 32688; break;
                    case 2:  newitemid = 32689; break;
                    case 3:  newitemid = 32690; break;
                    case 4:  newitemid = 32691; break;
                    case 5:  newitemid = 32692; break;
                    case 6:  newitemid = 32693; break;
                    case 7:  newitemid = 32700; break;
                    case 8:  newitemid = 32701; break;
                    case 9:  newitemid = 32702; break;
                    case 10: newitemid = 32703; break;
                    case 11: newitemid = 32704; break;
                    case 12: newitemid = 32705; break;
                    case 13: newitemid = 32706; break;
                    case 14: newitemid = 32707; break;
                    case 15: newitemid = 32708; break;
                    case 16: newitemid = 32709; break;
                    case 17: newitemid = 32710; break;
                    case 18: newitemid = 32711; break;
                    case 19: newitemid = 32712; break;
                    case 20: newitemid = 32713; break;
                    default:
                        return;
                }

                CreateItem(effIndex, newitemid);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_mingos_fortune_generator_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_mingos_fortune_generator_SpellScript();
        }
};

// http://www.wowhead.com/item=10720 Gnomish Net-o-Matic Projector
// 13120 Net-o-Matic
enum NetOMaticSpells
{
    SPELL_NET_O_MATIC_TRIGGERED1 = 16566,
    SPELL_NET_O_MATIC_TRIGGERED2 = 13119,
    SPELL_NET_O_MATIC_TRIGGERED3 = 13099,
};

class spell_item_net_o_matic : public SpellScriptLoader
{
    public:
        spell_item_net_o_matic() : SpellScriptLoader("spell_item_net_o_matic") { }

        class spell_item_net_o_matic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_net_o_matic_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NET_O_MATIC_TRIGGERED1) || !sSpellMgr->GetSpellInfo(SPELL_NET_O_MATIC_TRIGGERED2) || !sSpellMgr->GetSpellInfo(SPELL_NET_O_MATIC_TRIGGERED3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    uint32 spellId = SPELL_NET_O_MATIC_TRIGGERED3;
                    uint32 roll = urand(0, 99);
                    if (roll < 2)                            // 2% for 30 sec self root (off-like chance unknown)
                        spellId = SPELL_NET_O_MATIC_TRIGGERED1;
                    else if (roll < 4)                       // 2% for 20 sec root, charge to target (off-like chance unknown)
                        spellId = SPELL_NET_O_MATIC_TRIGGERED2;

                    GetCaster()->CastSpell(target, spellId, true, NULL);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_net_o_matic_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_net_o_matic_SpellScript();
        }
};

// http://www.wowhead.com/item=8529 Noggenfogger Elixir
// 16589 Noggenfogger Elixir
enum NoggenfoggerElixirSpells
{
    SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED1 = 16595,
    SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED2 = 16593,
    SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED3 = 16591,
};

class spell_item_noggenfogger_elixir : public SpellScriptLoader
{
    public:
        spell_item_noggenfogger_elixir() : SpellScriptLoader("spell_item_noggenfogger_elixir") { }

        class spell_item_noggenfogger_elixir_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_noggenfogger_elixir_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED1) || !sSpellMgr->GetSpellInfo(SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED2) || !sSpellMgr->GetSpellInfo(SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED3;
                switch (urand(1, 3))
                {
                    case 1: spellId = SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED1; break;
                    case 2: spellId = SPELL_NOGGENFOGGER_ELIXIR_TRIGGERED2; break;
                }

                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_noggenfogger_elixir_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_noggenfogger_elixir_SpellScript();
        }
};

// http://www.wowhead.com/item=6657 Savory Deviate Delight
// 8213 Savory Deviate Delight
enum SavoryDeviateDelight
{
    SPELL_FLIP_OUT_MALE     = 8219,
    SPELL_FLIP_OUT_FEMALE   = 8220,
    SPELL_YAAARRRR_MALE     = 8221,
    SPELL_YAAARRRR_FEMALE   = 8222,
};

class spell_item_savory_deviate_delight : public SpellScriptLoader
{
    public:
        spell_item_savory_deviate_delight() : SpellScriptLoader("spell_item_savory_deviate_delight") { }

        class spell_item_savory_deviate_delight_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_savory_deviate_delight_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                for (uint32 spellId = SPELL_FLIP_OUT_MALE; spellId <= SPELL_YAAARRRR_FEMALE; ++spellId)
                    if (!sSpellMgr->GetSpellInfo(spellId))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = 0;
                switch (urand(1, 2))
                {
                    // Flip Out - ninja
                    case 1: spellId = (caster->getGender() == GENDER_MALE ? SPELL_FLIP_OUT_MALE : SPELL_FLIP_OUT_FEMALE); break;
                    // Yaaarrrr - pirate
                    case 2: spellId = (caster->getGender() == GENDER_MALE ? SPELL_YAAARRRR_MALE : SPELL_YAAARRRR_FEMALE); break;
                }
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_savory_deviate_delight_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_savory_deviate_delight_SpellScript();
        }
};

// http://www.wowhead.com/item=7734 Six Demon Bag
// 14537 Six Demon Bag
enum SixDemonBagSpells
{
    SPELL_FROSTBOLT                 = 11538,
    SPELL_POLYMORPH                 = 14621,
    SPELL_SUMMON_FELHOUND_MINION    = 14642,
    SPELL_FIREBALL                  = 15662,
    SPELL_CHAIN_LIGHTNING           = 21179,
    SPELL_ENVELOPING_WINDS          = 25189,
};

class spell_item_six_demon_bag : public SpellScriptLoader
{
    public:
        spell_item_six_demon_bag() : SpellScriptLoader("spell_item_six_demon_bag") { }

        class spell_item_six_demon_bag_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_six_demon_bag_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_FROSTBOLT) || !sSpellMgr->GetSpellInfo(SPELL_POLYMORPH) || !sSpellMgr->GetSpellInfo(SPELL_SUMMON_FELHOUND_MINION) || !sSpellMgr->GetSpellInfo(SPELL_FIREBALL) || !sSpellMgr->GetSpellInfo(SPELL_CHAIN_LIGHTNING) || !sSpellMgr->GetSpellInfo(SPELL_ENVELOPING_WINDS))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    uint32 spellId = 0;
                    uint32 rand = urand(0, 99);
                    if (rand < 25)                      // Fireball (25% chance)
                        spellId = SPELL_FIREBALL;
                    else if (rand < 50)                 // Frostball (25% chance)
                        spellId = SPELL_FROSTBOLT;
                    else if (rand < 70)                 // Chain Lighting (20% chance)
                        spellId = SPELL_CHAIN_LIGHTNING;
                    else if (rand < 80)                 // Polymorph (10% chance)
                    {
                        spellId = SPELL_POLYMORPH;
                        if (urand(0, 100) <= 30)        // 30% chance to self-cast
                            target = caster;
                    }
                    else if (rand < 95)                 // Enveloping Winds (15% chance)
                        spellId = SPELL_ENVELOPING_WINDS;
                    else                                // Summon Felhund minion (5% chance)
                    {
                        spellId = SPELL_SUMMON_FELHOUND_MINION;
                        target = caster;
                    }

                    caster->CastSpell(target, spellId, true, GetCastItem());
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_six_demon_bag_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_six_demon_bag_SpellScript();
        }
};

// http://www.wowhead.com/item=44012 Underbelly Elixir
// 59640 Underbelly Elixir
enum UnderbellyElixirSpells
{
    SPELL_UNDERBELLY_ELIXIR_TRIGGERED1 = 59645,
    SPELL_UNDERBELLY_ELIXIR_TRIGGERED2 = 59831,
    SPELL_UNDERBELLY_ELIXIR_TRIGGERED3 = 59843,
};

class spell_item_underbelly_elixir : public SpellScriptLoader
{
    public:
        spell_item_underbelly_elixir() : SpellScriptLoader("spell_item_underbelly_elixir") { }

        class spell_item_underbelly_elixir_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_underbelly_elixir_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }
            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_UNDERBELLY_ELIXIR_TRIGGERED1) || !sSpellMgr->GetSpellInfo(SPELL_UNDERBELLY_ELIXIR_TRIGGERED2) || !sSpellMgr->GetSpellInfo(SPELL_UNDERBELLY_ELIXIR_TRIGGERED3))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                uint32 spellId = SPELL_UNDERBELLY_ELIXIR_TRIGGERED3;
                switch (urand(1, 3))
                {
                    case 1: spellId = SPELL_UNDERBELLY_ELIXIR_TRIGGERED1; break;
                    case 2: spellId = SPELL_UNDERBELLY_ELIXIR_TRIGGERED2; break;
                }
                caster->CastSpell(caster, spellId, true, NULL);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_underbelly_elixir_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_underbelly_elixir_SpellScript();
        }
};

enum eShadowmourneVisuals
{
    SPELL_SHADOWMOURNE_VISUAL_LOW       = 72521,
    SPELL_SHADOWMOURNE_VISUAL_HIGH      = 72523,
    SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF  = 73422,
};

class spell_item_shadowmourne : public SpellScriptLoader
{
public:
    spell_item_shadowmourne() : SpellScriptLoader("spell_item_shadowmourne") { }

    class spell_item_shadowmourne_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_shadowmourne_AuraScript);

        bool Validate(SpellInfo const* /*SpellInfo*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_VISUAL_LOW) || !sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_VISUAL_HIGH) || !sSpellMgr->GetSpellInfo(SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF))
                return false;
            return true;
        }

        void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            switch (GetStackAmount())
            {
                case 1:
                    target->CastSpell(target, SPELL_SHADOWMOURNE_VISUAL_LOW, true);
                    break;
                case 6:
                    target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_LOW);
                    target->CastSpell(target, SPELL_SHADOWMOURNE_VISUAL_HIGH, true);
                    break;
                case 10:
                    target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_HIGH);
                    target->CastSpell(target, SPELL_SHADOWMOURNE_CHAOS_BANE_BUFF, true);
                    break;
                default:
                    break;
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_LOW);
            target->RemoveAurasDueToSpell(SPELL_SHADOWMOURNE_VISUAL_HIGH);
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_item_shadowmourne_AuraScript::OnStackChange, EFFECT_0, SPELL_AURA_MOD_STAT, AuraEffectHandleModes(AURA_EFFECT_HANDLE_REAL | AURA_EFFECT_HANDLE_REAPPLY));
            AfterEffectRemove += AuraEffectRemoveFn(spell_item_shadowmourne_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_shadowmourne_AuraScript();
    }
};

enum AirRifleSpells
{
    SPELL_AIR_RIFLE_HOLD_VISUAL = 65582,
    SPELL_AIR_RIFLE_SHOOT       = 67532,
    SPELL_AIR_RIFLE_SHOOT_SELF  = 65577,
};

class spell_item_red_rider_air_rifle : public SpellScriptLoader
{
    public:
        spell_item_red_rider_air_rifle() : SpellScriptLoader("spell_item_red_rider_air_rifle") { }

        class spell_item_red_rider_air_rifle_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_red_rider_air_rifle_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_AIR_RIFLE_HOLD_VISUAL) || !sSpellMgr->GetSpellInfo(SPELL_AIR_RIFLE_SHOOT) || !sSpellMgr->GetSpellInfo(SPELL_AIR_RIFLE_SHOOT_SELF))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                if (Unit* target = GetHitUnit())
                {
                    caster->CastSpell(caster, SPELL_AIR_RIFLE_HOLD_VISUAL, true);
                    // needed because this spell shares GCD with its triggered spells (which must not be cast with triggered flag)
                    if (caster->IsPlayer())
                        caster->GetGlobalCooldownMgr().CancelGlobalCooldown(GetSpellInfo());
                    if (urand(0, 4))
                        caster->CastSpell(target, SPELL_AIR_RIFLE_SHOOT, false);
                    else
                        caster->CastSpell(caster, SPELL_AIR_RIFLE_SHOOT_SELF, false);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_red_rider_air_rifle_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_red_rider_air_rifle_SpellScript();
        }
};

enum GenericData
{
    SPELL_ARCANITE_DRAGONLING           = 19804,
    SPELL_BATTLE_CHICKEN                = 13166,
    SPELL_MECHANICAL_DRAGONLING         = 4073,
    SPELL_MITHRIL_MECHANICAL_DRAGONLING = 12749,
};

enum CreateHeartCandy
{
    ITEM_HEART_CANDY_1 = 21818,
    ITEM_HEART_CANDY_2 = 21817,
    ITEM_HEART_CANDY_3 = 21821,
    ITEM_HEART_CANDY_4 = 21819,
    ITEM_HEART_CANDY_5 = 21816,
    ITEM_HEART_CANDY_6 = 21823,
    ITEM_HEART_CANDY_7 = 21822,
    ITEM_HEART_CANDY_8 = 21820,
};

class spell_item_create_heart_candy : public SpellScriptLoader
{
    public:
        spell_item_create_heart_candy() : SpellScriptLoader("spell_item_create_heart_candy") { }

        class spell_item_create_heart_candy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_create_heart_candy_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Player* target = GetHitPlayer())
                {
                    static const uint32 items[] = {ITEM_HEART_CANDY_1, ITEM_HEART_CANDY_2, ITEM_HEART_CANDY_3, ITEM_HEART_CANDY_4, ITEM_HEART_CANDY_5, ITEM_HEART_CANDY_6, ITEM_HEART_CANDY_7, ITEM_HEART_CANDY_8};
                    target->AddItem(items[urand(0, 7)], 1);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_create_heart_candy_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_create_heart_candy_SpellScript();
        }
};

class spell_item_book_of_glyph_mastery : public SpellScriptLoader
{
    public:
        spell_item_book_of_glyph_mastery() : SpellScriptLoader("spell_item_book_of_glyph_mastery") {}

        class spell_item_book_of_glyph_mastery_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_book_of_glyph_mastery_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            SpellCastResult CheckRequirement()
            {
                if (HasDiscoveredAllSpells(GetSpellInfo()->Id, GetCaster()->ToPlayer()))
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_LEARNED_EVERYTHING);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_item_book_of_glyph_mastery_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_book_of_glyph_mastery_SpellScript();
        }
};

enum GiftOfTheHarvester
{
    NPC_GHOUL   = 28845,
    MAX_GHOULS  = 5,
};

class spell_item_gift_of_the_harvester : public SpellScriptLoader
{
    public:
        spell_item_gift_of_the_harvester() : SpellScriptLoader("spell_item_gift_of_the_harvester") {}

        class spell_item_gift_of_the_harvester_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_gift_of_the_harvester_SpellScript);

            SpellCastResult CheckRequirement()
            {
                std::list<Creature*> ghouls;
                GetCaster()->GetAllMinionsByEntry(ghouls, NPC_GHOUL);
                if (ghouls.size() >= MAX_GHOULS)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TOO_MANY_GHOULS);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_item_gift_of_the_harvester_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_gift_of_the_harvester_SpellScript();
        }
};

enum Sinkholes
{
    NPC_SOUTH_SINKHOLE      = 25664,
    NPC_NORTHEAST_SINKHOLE  = 25665,
    NPC_NORTHWEST_SINKHOLE  = 25666,
};

class spell_item_map_of_the_geyser_fields : public SpellScriptLoader
{
    public:
        spell_item_map_of_the_geyser_fields() : SpellScriptLoader("spell_item_map_of_the_geyser_fields") {}

        class spell_item_map_of_the_geyser_fields_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_map_of_the_geyser_fields_SpellScript);

            SpellCastResult CheckSinkholes()
            {
                Unit* caster = GetCaster();
                if (caster->FindNearestCreature(NPC_SOUTH_SINKHOLE, 30.0f, true) ||
                    caster->FindNearestCreature(NPC_NORTHEAST_SINKHOLE, 30.0f, true) ||
                    caster->FindNearestCreature(NPC_NORTHWEST_SINKHOLE, 30.0f, true))
                    return SPELL_CAST_OK;

                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_CLOSE_TO_SINKHOLE);
                return SPELL_FAILED_CUSTOM_ERROR;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_item_map_of_the_geyser_fields_SpellScript::CheckSinkholes);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_map_of_the_geyser_fields_SpellScript();
        }
};

enum VanquishedClutchesSpells
{
    SPELL_CRUSHER       = 64982,
    SPELL_CONSTRICTOR   = 64983,
    SPELL_CORRUPTOR     = 64984,
};

class spell_item_vanquished_clutches : public SpellScriptLoader
{
    public:
        spell_item_vanquished_clutches() : SpellScriptLoader("spell_item_vanquished_clutches") { }

        class spell_item_vanquished_clutches_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_vanquished_clutches_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CRUSHER) || !sSpellMgr->GetSpellInfo(SPELL_CONSTRICTOR) || !sSpellMgr->GetSpellInfo(SPELL_CORRUPTOR))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint32 spellId = RAND(SPELL_CRUSHER, SPELL_CONSTRICTOR, SPELL_CORRUPTOR);
                Unit* caster = GetCaster();
                caster->CastSpell(caster, spellId, true);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_vanquished_clutches_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_vanquished_clutches_SpellScript();
        }
};

enum MagicEater
{
    SPELL_WILD_MAGIC                             = 58891,
    SPELL_WELL_FED_1                             = 57288,
    SPELL_WELL_FED_2                             = 57139,
    SPELL_WELL_FED_3                             = 57111,
    SPELL_WELL_FED_4                             = 57286,
    SPELL_WELL_FED_5                             = 57291,
};

class spell_magic_eater_food : public SpellScriptLoader
{
    public:
        spell_magic_eater_food() : SpellScriptLoader("spell_magic_eater_food") {}

        class spell_magic_eater_food_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_magic_eater_food_AuraScript);

            void HandleTriggerSpell(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                Unit* target = GetTarget();
                switch (urand(0, 5))
                {
                    case 0:
                        target->CastSpell(target, SPELL_WILD_MAGIC, true);
                        break;
                    case 1:
                        target->CastSpell(target, SPELL_WELL_FED_1, true);
                        break;
                    case 2:
                        target->CastSpell(target, SPELL_WELL_FED_2, true);
                        break;
                    case 3:
                        target->CastSpell(target, SPELL_WELL_FED_3, true);
                        break;
                    case 4:
                        target->CastSpell(target, SPELL_WELL_FED_4, true);
                        break;
                    case 5:
                        target->CastSpell(target, SPELL_WELL_FED_5, true);
                        break;
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_magic_eater_food_AuraScript::HandleTriggerSpell, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_magic_eater_food_AuraScript();
        }
};

class spell_item_shimmering_vessel : public SpellScriptLoader
{
    public:
        spell_item_shimmering_vessel() : SpellScriptLoader("spell_item_shimmering_vessel") { }

        class spell_item_shimmering_vessel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_shimmering_vessel_SpellScript);

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Creature* target = GetHitCreature())
                    target->setDeathState(JUST_RESPAWNED);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_shimmering_vessel_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_shimmering_vessel_SpellScript();
        }
};

enum PurifyHelboarMeat
{
    SPELL_SUMMON_PURIFIED_HELBOAR_MEAT      = 29277,
    SPELL_SUMMON_TOXIC_HELBOAR_MEAT         = 29278,
    ITEM_PURIFIED_HELBOAR_MEAT              = 23248,
};

class spell_item_purify_helboar_meat : public SpellScriptLoader
{
    public:
        spell_item_purify_helboar_meat() : SpellScriptLoader("spell_item_purify_helboar_meat") { }

        class spell_item_purify_helboar_meat_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_purify_helboar_meat_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SUMMON_PURIFIED_HELBOAR_MEAT) ||  !sSpellMgr->GetSpellInfo(SPELL_SUMMON_TOXIC_HELBOAR_MEAT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if(roll_chance_i(50))
                    caster->CastSpell(caster, SPELL_SUMMON_TOXIC_HELBOAR_MEAT, true);
                else
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                        caster->ToPlayer()->AddItem(ITEM_PURIFIED_HELBOAR_MEAT, 1);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_purify_helboar_meat_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_purify_helboar_meat_SpellScript();
        }
};

enum CrystalPrison
{
    OBJECT_IMPRISONED_DOOMGUARD     = 179644,
};

class spell_item_crystal_prison_dummy_dnd : public SpellScriptLoader
{
    public:
        spell_item_crystal_prison_dummy_dnd() : SpellScriptLoader("spell_item_crystal_prison_dummy_dnd") { }

        class spell_item_crystal_prison_dummy_dnd_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_crystal_prison_dummy_dnd_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sObjectMgr->GetGameObjectTemplate(OBJECT_IMPRISONED_DOOMGUARD))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Creature* target = GetHitCreature())
                    if (target->isDead() && !target->isPet())
                    {
                        GetCaster()->SummonGameObject(OBJECT_IMPRISONED_DOOMGUARD, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), 0, 0, 0, 0, uint32(target->GetRespawnTime()-time(NULL)));
                        target->DespawnOrUnsummon();
                    }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_crystal_prison_dummy_dnd_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_crystal_prison_dummy_dnd_SpellScript();
        }
};

enum ReindeerTransformation
{
    SPELL_FLYING_REINDEER_310                   = 44827,
    SPELL_FLYING_REINDEER_280                   = 44825,
    SPELL_FLYING_REINDEER_60                    = 44824,
    SPELL_REINDEER_100                          = 25859,
    SPELL_REINDEER_60                           = 25858,
};

//25860
class spell_item_reindeer_transformation : public SpellScriptLoader
{
    public:
        spell_item_reindeer_transformation() : SpellScriptLoader("spell_item_reindeer_transformation") { }

        class spell_item_reindeer_transformation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_reindeer_transformation_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_FLYING_REINDEER_310) || !sSpellMgr->GetSpellInfo(SPELL_FLYING_REINDEER_280)
                    || !sSpellMgr->GetSpellInfo(SPELL_FLYING_REINDEER_60) || !sSpellMgr->GetSpellInfo(SPELL_REINDEER_100)
                    || !sSpellMgr->GetSpellInfo(SPELL_REINDEER_60))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (caster->HasAuraType(SPELL_AURA_MOUNTED))
                {
                    float flyspeed = caster->GetSpeedRate(MOVE_FLIGHT);
                    float speed = caster->GetSpeedRate(MOVE_RUN);

                    caster->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    //5 different spells used depending on mounted speed and if mount can fly or not

                    if (flyspeed >= 4.1f)
                        // Flying Reindeer
                        caster->CastSpell(caster, SPELL_FLYING_REINDEER_310, true); //310% flying Reindeer
                    else if (flyspeed >= 3.8f)
                        // Flying Reindeer
                        caster->CastSpell(caster, SPELL_FLYING_REINDEER_280, true); //280% flying Reindeer
                    else if (flyspeed >= 1.6f)
                        // Flying Reindeer
                        caster->CastSpell(caster, SPELL_FLYING_REINDEER_60, true); //60% flying Reindeer
                    else if (speed >= 2.0f)
                        // Reindeer
                        caster->CastSpell(caster, SPELL_REINDEER_100, true); //100% ground Reindeer
                    else
                        // Reindeer
                        caster->CastSpell(caster, SPELL_REINDEER_60, true); //60% ground Reindeer
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_reindeer_transformation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_item_reindeer_transformation_SpellScript();
    }
};

enum NighInvulnerability
{
    SPELL_NIGH_INVULNERABILITY                  = 30456,
    SPELL_COMPLETE_VULNERABILITY                = 30457,
};

class spell_item_nigh_invulnerability : public SpellScriptLoader
{
    public:
        spell_item_nigh_invulnerability() : SpellScriptLoader("spell_item_nigh_invulnerability") { }

        class spell_item_nigh_invulnerability_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_nigh_invulnerability_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_NIGH_INVULNERABILITY) || !sSpellMgr->GetSpellInfo(SPELL_COMPLETE_VULNERABILITY))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (Item* castItem = GetCastItem())
                {
                    if (roll_chance_i(86))                  // Nigh-Invulnerability   - success
                        caster->CastSpell(caster, SPELL_NIGH_INVULNERABILITY, true, castItem);
                    else                                    // Complete Vulnerability - backfire in 14% casts
                        caster->CastSpell(caster, SPELL_COMPLETE_VULNERABILITY, true, castItem);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_nigh_invulnerability_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_nigh_invulnerability_SpellScript();
        }
};

enum Poultryzer
{
    SPELL_POULTRYIZER_SUCCESS    = 30501,
    SPELL_POULTRYIZER_BACKFIRE   = 30504,
};

class spell_item_poultryizer : public SpellScriptLoader
{
    public:
        spell_item_poultryizer() : SpellScriptLoader("spell_item_poultryizer") { }

        class spell_item_poultryizer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_poultryizer_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_POULTRYIZER_SUCCESS) || !sSpellMgr->GetSpellInfo(SPELL_POULTRYIZER_BACKFIRE))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (GetCastItem() && GetHitUnit())
                    GetCaster()->CastSpell(GetHitUnit(), roll_chance_i(80) ? SPELL_POULTRYIZER_SUCCESS : SPELL_POULTRYIZER_BACKFIRE , true, GetCastItem());
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_poultryizer_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_poultryizer_SpellScript();
        }
};

enum SocretharsStone
{
    SPELL_SOCRETHAR_TO_SEAT     = 35743,
    SPELL_SOCRETHAR_FROM_SEAT   = 35744,
};

class spell_item_socrethars_stone : public SpellScriptLoader
{
    public:
        spell_item_socrethars_stone() : SpellScriptLoader("spell_item_socrethars_stone") { }

        class spell_item_socrethars_stone_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_socrethars_stone_SpellScript);

            bool Load() override
            {
                return (GetCaster()->GetCurrentAreaID() == 3900 || GetCaster()->GetCurrentAreaID() == 3742);
            }
            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_SOCRETHAR_TO_SEAT) || !sSpellMgr->GetSpellInfo(SPELL_SOCRETHAR_FROM_SEAT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                switch (caster->GetCurrentAreaID())
                {
                    case 3900:
                        caster->CastSpell(caster, SPELL_SOCRETHAR_TO_SEAT, true);
                        break;
                    case 3742:
                        caster->CastSpell(caster, SPELL_SOCRETHAR_FROM_SEAT, true);
                        break;
                    default:
                        return;
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_socrethars_stone_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_socrethars_stone_SpellScript();
        }
};

enum DemonBroiledSurprise
{
    QUEST_SUPER_HOT_STEW                    = 11379,
    SPELL_CREATE_DEMON_BROILED_SURPRISE     = 43753,
    NPC_ABYSSAL_FLAMEBRINGER                = 19973,
};

class spell_item_demon_broiled_surprise : public SpellScriptLoader
{
    public:
        spell_item_demon_broiled_surprise() : SpellScriptLoader("spell_item_demon_broiled_surprise") { }

        class spell_item_demon_broiled_surprise_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_demon_broiled_surprise_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CREATE_DEMON_BROILED_SURPRISE) || !sObjectMgr->GetCreatureTemplate(NPC_ABYSSAL_FLAMEBRINGER) || !sQuestDataStore->GetQuestTemplate(QUEST_SUPER_HOT_STEW))
                    return false;
                return true;
            }

            bool Load() override
            {
               return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* player = GetCaster();
                player->CastSpell(player, SPELL_CREATE_DEMON_BROILED_SURPRISE, false);
            }

            SpellCastResult CheckRequirement()
            {
                Player* player = GetCaster()->ToPlayer();
                if (player->GetQuestStatus(QUEST_SUPER_HOT_STEW) != QUEST_STATUS_INCOMPLETE)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                if (Creature* creature = player->FindNearestCreature(NPC_ABYSSAL_FLAMEBRINGER, 10, false))
                    if (creature->isDead())
                        return SPELL_CAST_OK;
                return SPELL_FAILED_NOT_HERE;
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_demon_broiled_surprise_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_item_demon_broiled_surprise_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_demon_broiled_surprise_SpellScript();
        }
};

enum CompleteRaptorCapture
{
    SPELL_RAPTOR_CAPTURE_CREDIT     = 42337,
};

class spell_item_complete_raptor_capture : public SpellScriptLoader
{
    public:
        spell_item_complete_raptor_capture() : SpellScriptLoader("spell_item_complete_raptor_capture") { }

        class spell_item_complete_raptor_capture_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_complete_raptor_capture_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_RAPTOR_CAPTURE_CREDIT))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (GetHitCreature())
                {
                    GetHitCreature()->DespawnOrUnsummon();

                    //cast spell Raptor Capture Credit
                    caster->CastSpell(caster, SPELL_RAPTOR_CAPTURE_CREDIT, true, NULL);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_complete_raptor_capture_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_complete_raptor_capture_SpellScript();
        }
};

enum ImpaleLeviroth
{
    NPC_LEVIROTH                = 26452,
    SPELL_LEVIROTH_SELF_IMPALE  = 49882,
};

class spell_item_impale_leviroth : public SpellScriptLoader
{
    public:
        spell_item_impale_leviroth() : SpellScriptLoader("spell_item_impale_leviroth") { }

        class spell_item_impale_leviroth_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_impale_leviroth_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sObjectMgr->GetCreatureTemplate(NPC_LEVIROTH))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                if (Unit* target = GetHitCreature())
                    if (target->GetEntry() == NPC_LEVIROTH && !target->HealthBelowPct(95))
                        target->CastSpell(target, SPELL_LEVIROTH_SELF_IMPALE, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_impale_leviroth_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_impale_leviroth_SpellScript();
        }
};

enum BrewfestMountTransformation
{
    SPELL_MOUNT_RAM_100                         = 43900,
    SPELL_MOUNT_RAM_60                          = 43899,
    SPELL_MOUNT_KODO_100                        = 49379,
    SPELL_MOUNT_KODO_60                         = 49378,
    SPELL_BREWFEST_MOUNT_TRANSFORM              = 49357,
    SPELL_BREWFEST_MOUNT_TRANSFORM_REVERSE      = 52845,
};

class spell_item_brewfest_mount_transformation : public SpellScriptLoader
{
    public:
        spell_item_brewfest_mount_transformation() : SpellScriptLoader("spell_item_brewfest_mount_transformation") { }

        class spell_item_brewfest_mount_transformation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_brewfest_mount_transformation_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_MOUNT_RAM_100) || !sSpellMgr->GetSpellInfo(SPELL_MOUNT_RAM_60) || !sSpellMgr->GetSpellInfo(SPELL_MOUNT_KODO_100) || !sSpellMgr->GetSpellInfo(SPELL_MOUNT_KODO_60))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (caster->HasAuraType(SPELL_AURA_MOUNTED))
                {
                    caster->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    uint32 spell_id;

                    switch (GetSpellInfo()->Id)
                    {
                        case SPELL_BREWFEST_MOUNT_TRANSFORM:
                            if (caster->GetSpeedRate(MOVE_RUN) >= 2.0f)
                                spell_id = caster->GetTeam() == ALLIANCE ? SPELL_MOUNT_RAM_100 : SPELL_MOUNT_KODO_100;
                            else
                                spell_id = caster->GetTeam() == ALLIANCE ? SPELL_MOUNT_RAM_60 : SPELL_MOUNT_KODO_60;
                            break;
                        case SPELL_BREWFEST_MOUNT_TRANSFORM_REVERSE:
                            if (caster->GetSpeedRate(MOVE_RUN) >= 2.0f)
                                spell_id = caster->GetTeam() == HORDE ? SPELL_MOUNT_RAM_100 : SPELL_MOUNT_KODO_100;
                            else
                                spell_id = caster->GetTeam() == HORDE ? SPELL_MOUNT_RAM_60 : SPELL_MOUNT_KODO_60;
                            break;
                        default:
                            return;
                    }
                    caster->CastSpell(caster, spell_id, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_brewfest_mount_transformation_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_brewfest_mount_transformation_SpellScript();
        }
};

enum NitroBoots
{
    SPELL_NITRO_BOOTS_SUCCESS       = 54861,
    SPELL_NITRO_BOOTS_BACKFIRE      = 46014,
};

class spell_item_nitro_boots : public SpellScript
{
    PrepareSpellScript(spell_item_nitro_boots);

    bool Load() override
    {
        if (!GetCastItem())
            return false;

        return true;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpellDelay(caster, roll_chance_i(95) ? SPELL_NITRO_BOOTS_SUCCESS : SPELL_NITRO_BOOTS_BACKFIRE, true, 100, GetCastItem());
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_item_nitro_boots::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum TeachLanguage
{
    SPELL_LEARN_GNOMISH_BINARY      = 50242,
    SPELL_LEARN_GOBLIN_BINARY       = 50246,
};

class spell_item_teach_language : public SpellScriptLoader
{
    public:
        spell_item_teach_language() : SpellScriptLoader("spell_item_teach_language") { }

        class spell_item_teach_language_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_teach_language_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_LEARN_GNOMISH_BINARY) || !sSpellMgr->GetSpellInfo(SPELL_LEARN_GOBLIN_BINARY))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();

                if (roll_chance_i(34))
                    caster->CastSpell(caster,caster->GetTeam() == ALLIANCE ? SPELL_LEARN_GNOMISH_BINARY : SPELL_LEARN_GOBLIN_BINARY, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_teach_language_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_teach_language_SpellScript();
        }
};

enum RocketBoots
{
    SPELL_ROCKET_BOOTS_PROC      = 30452,
};

class spell_item_rocket_boots : public SpellScriptLoader
{
    public:
        spell_item_rocket_boots() : SpellScriptLoader("spell_item_rocket_boots") { }

        class spell_item_rocket_boots_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_rocket_boots_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_ROCKET_BOOTS_PROC))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Battleground* bg = caster->GetBattleground())
                    bg->EventPlayerDroppedFlag(caster);

                caster->RemoveSpellCooldown(SPELL_ROCKET_BOOTS_PROC);
                caster->CastSpell(caster, SPELL_ROCKET_BOOTS_PROC, true, NULL);
            }

            SpellCastResult CheckCast()
            {
                if (GetCaster()->IsInWater())
                    return SPELL_FAILED_ONLY_ABOVEWATER;
                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_item_rocket_boots_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_item_rocket_boots_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_rocket_boots_SpellScript();
        }
};

enum PygmyOil
{
    SPELL_PYGMY_OIL_PYGMY_AURA      = 53806,
    SPELL_PYGMY_OIL_SMALLER_AURA    = 53805,
};

class spell_item_pygmy_oil : public SpellScriptLoader
{
    public:
        spell_item_pygmy_oil() : SpellScriptLoader("spell_item_pygmy_oil") { }

        class spell_item_pygmy_oil_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_pygmy_oil_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PYGMY_OIL_PYGMY_AURA) || !sSpellMgr->GetSpellInfo(SPELL_PYGMY_OIL_SMALLER_AURA))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Unit* caster = GetCaster();
                if (Aura* aura = caster->GetAura(SPELL_PYGMY_OIL_PYGMY_AURA))
                    aura->RefreshDuration();
                else
                {
                    aura = caster->GetAura(SPELL_PYGMY_OIL_SMALLER_AURA);
                    if (!aura || aura->GetStackAmount() < 5 || !roll_chance_i(50))
                         caster->CastSpell(caster, SPELL_PYGMY_OIL_SMALLER_AURA, true);
                    else
                    {
                        aura->Remove();
                        caster->CastSpell(caster, SPELL_PYGMY_OIL_PYGMY_AURA, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_pygmy_oil_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_pygmy_oil_SpellScript();
        }
};

enum ChickenCover
{
    SPELL_CHICKEN_NET               = 51959,
    SPELL_CAPTURE_CHICKEN_ESCAPE    = 51037,
    QUEST_CHICKEN_PARTY             = 12702,
    QUEST_FLOWN_THE_COOP            = 12532,
};

class spell_item_chicken_cover : public SpellScriptLoader
{
    public:
        spell_item_chicken_cover() : SpellScriptLoader("spell_item_chicken_cover") { }

        class spell_item_chicken_cover_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_chicken_cover_SpellScript);

            bool Load() override
            {
                return GetCaster()->IsPlayer();
            }

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_CHICKEN_NET) || !sSpellMgr->GetSpellInfo(SPELL_CAPTURE_CHICKEN_ESCAPE) || !sQuestDataStore->GetQuestTemplate(QUEST_CHICKEN_PARTY) || !sQuestDataStore->GetQuestTemplate(QUEST_FLOWN_THE_COOP))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                if (Unit* target = GetHitUnit())
                {
                    if (!target->HasAura(SPELL_CHICKEN_NET) && (caster->GetQuestStatus(QUEST_CHICKEN_PARTY) == QUEST_STATUS_INCOMPLETE || caster->GetQuestStatus(QUEST_FLOWN_THE_COOP) == QUEST_STATUS_INCOMPLETE))
                    {
                        caster->CastSpell(caster, SPELL_CAPTURE_CHICKEN_ESCAPE, true);
                        target->Kill(target);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_chicken_cover_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_chicken_cover_SpellScript();
        }
};

enum Refocus
{
    SPELL_AIMED_SHOT    = 19434,
    SPELL_MULTISHOT     = 2643,
    SPELL_VOLLEY        = 42243,
};

class spell_item_refocus : public SpellScriptLoader
{
    public:
        spell_item_refocus() : SpellScriptLoader("spell_item_refocus") { }

        class spell_item_refocus_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_refocus_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Player* caster = GetCaster()->ToPlayer();

                if (!caster || caster->getClass() != CLASS_HUNTER)
                    return;

                if (caster->HasSpellCooldown(SPELL_AIMED_SHOT))
                    caster->RemoveSpellCooldown(SPELL_AIMED_SHOT, true);

                if (caster->HasSpellCooldown(SPELL_MULTISHOT))
                    caster->RemoveSpellCooldown(SPELL_MULTISHOT, true);

                if (caster->HasSpellCooldown(SPELL_VOLLEY))
                    caster->RemoveSpellCooldown(SPELL_VOLLEY, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_refocus_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_refocus_SpellScript();
        }
};

class spell_item_muisek_vessel : public SpellScriptLoader
{
    public:
        spell_item_muisek_vessel() : SpellScriptLoader("spell_item_muisek_vessel") { }

        class spell_item_muisek_vessel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_muisek_vessel_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Creature* target = GetHitCreature())
                    if (target->isDead())
                        target->DespawnOrUnsummon();
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_muisek_vessel_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_muisek_vessel_SpellScript();
        }
};

enum GreatmothersSoulcather
{
    SPELL_FORCE_CAST_SUMMON_GNOME_SOUL = 46486,
};

class spell_item_greatmothers_soulcatcher : public SpellScriptLoader
{
public:
    spell_item_greatmothers_soulcatcher() : SpellScriptLoader("spell_item_greatmothers_soulcatcher") { }

    class spell_item_greatmothers_soulcatcher_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_greatmothers_soulcatcher_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit())
                GetCaster()->CastSpell(GetCaster(),SPELL_FORCE_CAST_SUMMON_GNOME_SOUL);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_item_greatmothers_soulcatcher_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_item_greatmothers_soulcatcher_SpellScript();
    }
};

// Enohar Explosive Arrows - 78838
class spell_item_enohar_explosive_arrows : public SpellScriptLoader
{
    public:
        spell_item_enohar_explosive_arrows() : SpellScriptLoader("spell_item_enohar_explosive_arrows") { }

        class spell_item_enohar_explosive_arrows_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_enohar_explosive_arrows_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (caster && target)
                    caster->DealDamage(target, target->GetHealth());
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_enohar_explosive_arrows_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_enohar_explosive_arrows_SpellScript();
        }
};

enum HolyThurible
{
    NPC_WITHDRAWN_SOUL           = 45166,
};

class spell_item_holy_thurible : public SpellScriptLoader
{
    public:
        spell_item_holy_thurible() : SpellScriptLoader("spell_item_holy_thurible") { }

        class spell_item_holy_thurible_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_holy_thurible_SpellScript);

          
            void HandleDummy(SpellEffIndex /* effIndex */)
            {
                Player* caster = GetCaster()->ToPlayer();
                // GetHitCreature don't work
                Creature* target  = caster->FindNearestCreature(NPC_WITHDRAWN_SOUL, 2.0f, true);
                if(target && caster){

                    if(roll_chance_i(50)){
                        caster->KilledMonsterCredit(NPC_WITHDRAWN_SOUL, target->GetGUID());
                        target->DespawnOrUnsummon(0);
                    }
                    else
                        target->setFaction(14);
                }                
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_holy_thurible_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_holy_thurible_SpellScript();
        }
};

class spell_item_gen_alchemy_mop : public SpellScriptLoader
{
    public:
        spell_item_gen_alchemy_mop() : SpellScriptLoader("spell_item_gen_alchemy_mop") { }

        class spell_item_gen_alchemy_mop_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_gen_alchemy_mop_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleOnHit()
            {
                uint8 chance = urand(1,5); // not official, todo: find the rate
                Player* caster = GetCaster()->ToPlayer();
                if (caster && GetCaster()->GetTypeId() == TYPEID_PLAYER && !HasDiscoveredAllSpells(114751, GetCaster()->ToPlayer()) && chance == 1)
                {
                    if (uint32 discoveredSpellId = GetExplicitDiscoverySpell(114751, caster->ToPlayer()))
                        caster->learnSpell(discoveredSpellId, false);
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_item_gen_alchemy_mop_SpellScript::HandleOnHit);
            }

        };
        SpellScript* GetSpellScript() const override
        {
            return new spell_item_gen_alchemy_mop_SpellScript();
        }                 
};

class spell_alchemist_rejuvenation : public SpellScriptLoader
{
    public:
        spell_alchemist_rejuvenation() : SpellScriptLoader("spell_alchemist_rejuvenation") { }

        class spell_alchemist_rejuvenation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_alchemist_rejuvenation_SpellScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void HandleOnHit()
            {
                if(Unit* caster = GetCaster())
                {
                    if(caster->getPowerType() == POWER_MANA)
                    {
                        switch (caster->getLevel())
                        {
                             case 85:
                                caster->EnergizeBySpell(caster, 105704, urand(8484, 9376), POWER_MANA);                                
                                break;
                            case 86:
                                caster->EnergizeBySpell(caster, 105704, urand(13651, 15087), POWER_MANA);  
                                break;
                            case 87:
                                caster->EnergizeBySpell(caster, 105704, urand(16451, 18181), POWER_MANA);  
                                break;
                            case 88:
                                caster->EnergizeBySpell(caster, 105704, urand(19818, 21902), POWER_MANA);  
                                break;
                            case 89:
                                caster->EnergizeBySpell(caster, 105704, urand(23884, 26398), POWER_MANA);  
                                break;
                            case 90:
                                caster->EnergizeBySpell(caster, 105704, urand(28500, 31500), POWER_MANA);  
                                break;
                            default:
                                break;
                        }
                    }
                    switch (caster->getLevel())
                    {
                        case 85:
                            caster->HealBySpell(caster, sSpellMgr->GetSpellInfo(105704), urand(33935, 37505), false);                                
                            break;
                        case 86:
                            caster->HealBySpell(caster, sSpellMgr->GetSpellInfo(105704), urand(54601, 60347), false);  
                            break;
                        case 87:
                            caster->HealBySpell(caster, sSpellMgr->GetSpellInfo(105704), urand(65801, 72727), false);  
                            break;
                        case 88:
                            caster->HealBySpell(caster, sSpellMgr->GetSpellInfo(105704), urand(79268, 87610), false);  
                            break;
                        case 89:
                            caster->HealBySpell(caster, sSpellMgr->GetSpellInfo(105704), urand(95534, 105590), false);  
                            break;
                        case 90:
                            caster->HealBySpell(caster, sSpellMgr->GetSpellInfo(105704), urand(114001, 126001), false);  
                            break;
                        default:
                            break;
                    }
                }                    
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_alchemist_rejuvenation_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_alchemist_rejuvenation_SpellScript();
        }
};

// Hardened Shell - 129787
class spell_item_hardened_shell : public SpellScriptLoader
{
    public:
        spell_item_hardened_shell() : SpellScriptLoader("spell_item_hardened_shell") { }

        class spell_item_hardened_shell_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_hardened_shell_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetTarget())
                    target->RemoveAurasByType(SPELL_AURA_MOUNTED);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_item_hardened_shell_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_hardened_shell_AuraScript();
        }
};

// item=104111 Elixir of Wandering Spirits
// spell=147412
enum ElixirModelSpells
{
    SPELL_LEVEN_DAWNBLADE   = 147402,
    SPELL_ANJI_AUTUMNLIGHT  = 147403,
    SPELL_KUN_AUTUMNLIGHT   = 147405,
    SPELL_ROOK_STONETOE     = 147406,
    SPELL_REN_FIRETONGUE    = 147407,
    SPELL_HE_SOFTFOOT       = 147409,
    SPELL_SUN_TENDERHEART   = 147410,
    SPELL_CHE_WILDWALKER    = 147411,
};

static const uint32 ElixirModel[] = 
{
SPELL_LEVEN_DAWNBLADE, 
SPELL_ANJI_AUTUMNLIGHT, 
SPELL_KUN_AUTUMNLIGHT, 
SPELL_ROOK_STONETOE, 
SPELL_REN_FIRETONGUE, 
SPELL_HE_SOFTFOOT,
SPELL_SUN_TENDERHEART,
SPELL_CHE_WILDWALKER
};

class spell_item_elixir_of_wandering_spirits : public SpellScriptLoader
{
    public:
        spell_item_elixir_of_wandering_spirits() : SpellScriptLoader("spell_item_elixir_of_wandering_spirits") { }

        class spell_item_elixir_of_wandering_spirits_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_elixir_of_wandering_spirits_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                caster->CastSpell(caster, ElixirModel[urand(0, 7)], true);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_elixir_of_wandering_spirits_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_elixir_of_wandering_spirits_SpellScript();
        }
};

enum SkymirrorCloneSpell
{
    SKYMIRROR_IMAGE_CLONE_VISUAL = 127315,
};

//129803
class spell_item_ai_li_skymirror : public SpellScriptLoader
{
    public:
        spell_item_ai_li_skymirror() : SpellScriptLoader("spell_item_ai_li_skymirror") { }

        class spell_item_ai_li_skymirror_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_ai_li_skymirror_SpellScript);

            SpellCastResult CheckRequirement()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (Unit* target = player->GetSelectedUnit())
                        if (target->HasAuraType(SPELL_AURA_MOD_STEALTH) || target->HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
                            return SPELL_FAILED_VISION_OBSCURED;

                return SPELL_CAST_OK;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                if (auto caster = GetCaster())
                    if (auto target = GetHitUnit())
                    {
                        caster->CastSpell(caster, SKYMIRROR_IMAGE_CLONE_VISUAL, true);
                        caster->InitMirrorImageData(target);
                        caster->SetDisplayId(target->GetDisplayId());
                        caster->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
                    }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_ai_li_skymirror_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnCheckCast += SpellCheckCastFn(spell_item_ai_li_skymirror_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_ai_li_skymirror_SpellScript();
        }
};


class spell_item_eye_of_the_black_prince : public SpellScriptLoader
{
    public:
        spell_item_eye_of_the_black_prince() : SpellScriptLoader("spell_item_eye_of_the_black_prince") {}

        class spell_item_eye_of_the_black_prince_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_eye_of_the_black_prince_SpellScript);

            SpellCastResult CheckRequirement()
            {
                if (Item* itemTarget = GetExplTargetItem())
                {
                    if(ItemTemplate const* itemProto = itemTarget->GetTemplate())
                    {
                        for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
                        {
                            //if (itemProto->Socket[i].Color && itemProto->Socket[i].Color == SOCKET_COLOR_SHA)
                            //    return SPELL_CAST_OK;
                        }
                    }
                }

                return SPELL_FAILED_BAD_TARGETS;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_item_eye_of_the_black_prince_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_eye_of_the_black_prince_SpellScript();
        }
};

class spell_item_book_of_the_ages : public SpellScriptLoader
{
    public:
        spell_item_book_of_the_ages() : SpellScriptLoader("spell_item_book_of_the_ages") { }

        class spell_item_book_of_the_ages_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_book_of_the_ages_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                int32 agila = caster->GetStat(STAT_AGILITY);
                int32 intelect = caster->GetStat(STAT_INTELLECT);
                int32 streight = caster->GetStat(STAT_STRENGTH);

                if(agila > intelect && agila > streight)
                    caster->CastSpell(caster, 147355, true);
                else if(intelect > agila && intelect > streight)
                    caster->CastSpell(caster, 147357, true);
                else if(streight > intelect && streight > agila)
                    caster->CastSpell(caster, 147359, true);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_item_book_of_the_ages_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_book_of_the_ages_SpellScript();
        }
};

// http://www.wowhead.com/spell=87649 Item: Chocolate Cookie
class spell_item_chocolate_cookie : public SpellScriptLoader
{
public:
    spell_item_chocolate_cookie() : SpellScriptLoader("spell_item_chocolate_cookie") { }

    class spell_item_chocolate_cookie_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_chocolate_cookie_AuraScript);

        void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Player* caster = GetCaster()->ToPlayer();
            if (!caster)
                return;

            // Achiev: You'll Feel Right as Rain
            if (GetStackAmount() == 91)
                caster->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 99041);
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_item_chocolate_cookie_AuraScript::OnStackChange, EFFECT_0, SPELL_AURA_DUMMY, AuraEffectHandleModes(AURA_EFFECT_HANDLE_REAL | AURA_EFFECT_HANDLE_REAPPLY));
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_chocolate_cookie_AuraScript();
    }
};

// Essence of Wrathion - 146428
class spell_item_essence_of_wrathion : public SpellScriptLoader
{
public:
    spell_item_essence_of_wrathion() : SpellScriptLoader("spell_item_essence_of_wrathion") { }

    class spell_item_essence_of_wrathion_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_item_essence_of_wrathion_SpellScript);

        void HandleOnCast()
        {
            if (Player* player = GetCaster()->ToPlayer())
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK))
                {
                    switch (item->GetEntry())
                    {
                        case 98149:
                            player->CastSpell(player, 146564);
                            break;
                        case 98147:
                            player->CastSpell(player, 146562);
                            break;
                        case 98146:
                            player->CastSpell(player, 146560);
                            break;
                        case 98335:
                            player->CastSpell(player, 146566);
                            break;
                        case 98148:
                            player->CastSpell(player, 146563);
                            break;
                        case 98150:
                            player->CastSpell(player, 146565);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        void Register() override
        {
            OnCast += SpellCastFn(spell_item_essence_of_wrathion_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_item_essence_of_wrathion_SpellScript();
    }
};

// Warlord's Fortitude - 214624
class spell_item_warlords_fortitude : public SpellScriptLoader
{
    public:
        spell_item_warlords_fortitude() : SpellScriptLoader("spell_item_warlords_fortitude") { }

        class spell_item_warlords_fortitude_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_warlords_fortitude_AuraScript);

            void CalculateAmountHP(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(214648))
                        return;
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(214622, EFFECT_1))
                        amount = aurEff->GetAmount();
                }
            }

            void CalculateAmountRA(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(214648))
                        return;
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(214622, EFFECT_3))
                        amount = aurEff->GetAmount();
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_warlords_fortitude_AuraScript::CalculateAmountHP, EFFECT_0, SPELL_AURA_MOD_INCREASE_HEALTH_OR_PCT);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_warlords_fortitude_AuraScript::CalculateAmountRA, EFFECT_1, SPELL_AURA_MOD_RATING);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_warlords_fortitude_AuraScript();
        }
};

// 207472 - Xavarics Magnum Opus (legendary item 132444 - Prydaz, Xavarics Magnum Opus)
class spell_item_prydaz_abs : public SpellScriptLoader
{
    public:
        spell_item_prydaz_abs() : SpellScriptLoader("spell_item_prydaz_abs") { }

        class spell_item_prydaz_abs_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_prydaz_abs_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                if (auto plr = GetCaster()->ToPlayer())
                {
                    if (plr->isInTankSpec()) // for tank specs
                        amount = plr->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_0]->CalcValue(plr)) * 0.6f;
                    else
                        amount = plr->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_0]->CalcValue(plr));
                }
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_prydaz_abs_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_prydaz_abs_AuraScript();
        }
};

//161775
class spell_item_goren_gas_extractor : public SpellScriptLoader
{
    public:
        spell_item_goren_gas_extractor() : SpellScriptLoader("spell_item_goren_gas_extractor") {}

        class spell_item_goren_gas_extractor_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_goren_gas_extractor_SpellScript);

            SpellCastResult CheckRequirement()
            {
                if (Player* player = GetCaster()->ToPlayer())
                    if (Unit* target = player->GetSelectedUnit())
                        if (!target->isAlive() && (target->GetEntry() == 79190 || target->GetEntry() == 80345))
                            return SPELL_CAST_OK;

                return SPELL_FAILED_BAD_TARGETS;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_item_goren_gas_extractor_SpellScript::CheckRequirement);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_item_goren_gas_extractor_SpellScript();
        }
};

class spell_autographed_hearthstone_card : public SpellScript
{
    PrepareSpellScript(spell_autographed_hearthstone_card);

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        std::string emote;

        if (Player* player = GetCaster()->ToPlayer())
        {
            switch (irand(1, 4))
            {
            case 1:
                if (auto entry = sBroadcastTextStore.LookupEntry(90707))
                {
                    emote = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());
                    player->TextEmote(emote);
                }

                player->PlayDistanceSound(47498, NULL);
                player->SendPlaySpellVisualKit(0, 52852, 0);
                break;
            case 2:
                if (auto entry = sBroadcastTextStore.LookupEntry(90711))
                {
                    emote = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());
                    player->TextEmote(emote);
                }

                player->PlayDistanceSound(47499, NULL);
                player->SendPlaySpellVisualKit(0, 52855, 0);
                break;
            case 3:
                if (auto entry = sBroadcastTextStore.LookupEntry(90712))
                {
                    emote = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());
                    player->TextEmote(emote);
                }

                player->PlayDistanceSound(47500, NULL);
                player->SendPlaySpellVisualKit(0, 52856, 0);
                break;
            case 4:
                if (auto entry = sBroadcastTextStore.LookupEntry(90715))
                {
                    emote = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());
                    player->TextEmote(emote);
                }

                player->PlayDistanceSound(47501, NULL);
                player->SendPlaySpellVisualKit(0, 52857, 0);
                break;
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_autographed_hearthstone_card::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_item_spinning : public SpellScript
{
    PrepareSpellScript(spell_item_spinning);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Player* player = GetCaster()->ToPlayer())
            player->SetFacingTo(frand(0.0f, 62832.0f) / 10000.0f);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_spinning::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_item_slightly_chewed_insult_book : public SpellScript
{
    PrepareSpellScript(spell_item_slightly_chewed_insult_book);

    uint32 RandomTalk[10] = { 114461, 114436, 114433, 114462, 114441, 114463, 114434, 114435, 114437, 114439 };

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        std::string text;
        auto entry = sBroadcastTextStore.LookupEntry(RandomTalk[urand(0, 9)]);
        if (!entry)
            return;

        text = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());
        player->Yell(text, LANG_UNIVERSAL, NULL);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_slightly_chewed_insult_book::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_gamons_heroic_spirit : public SpellScript
{
    PrepareSpellScript(spell_gamons_heroic_spirit);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        std::string text;
        auto entry = sBroadcastTextStore.LookupEntry(66282);
        if (!entry)
            return;

        text = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());
        player->Yell(text, LANG_UNIVERSAL, NULL);
        player->PlayDistanceSound(38282, NULL);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gamons_heroic_spirit::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Living Carapace - 225033, Stance of the Mountain - 214423, Wailing Souls - 242609
class spell_item_living_carapace : public SpellScriptLoader
{
    public:
        spell_item_living_carapace() : SpellScriptLoader("spell_item_living_carapace") { }

        class spell_item_living_carapace_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_living_carapace_AuraScript);

            uint32 absorbPct;

            bool Load() override
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
                return true;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_living_carapace_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_living_carapace_AuraScript();
        }
};

// Infernal Contract - 225140
class spell_item_infernal_contract : public SpellScriptLoader
{
    public:
        spell_item_infernal_contract() : SpellScriptLoader("spell_item_infernal_contract") { }

        class spell_item_infernal_contract_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_infernal_contract_AuraScript);

            uint32 absorbPct;
            bool takeDamage = false;

            bool Load() override
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
                return true;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
                takeDamage = true;
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!takeDamage)
                    return;

                if(Unit* target = GetTarget())
                {
                    if (ObjectGuid itemGUID = aurEff->GetBase()->GetCastItemGUID())
                        if (Player* player = target->ToPlayer())
                            if (Item* castItem = player->GetItemByGuid(itemGUID))
                                target->CastSpell(target, 225778, true, castItem);
                }
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_infernal_contract_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectRemove += AuraEffectRemoveFn(spell_item_infernal_contract_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_infernal_contract_AuraScript();
        }
};

// Sands of Time - 225124
class spell_item_sands_of_time : public SpellScriptLoader
{
    public:
        spell_item_sands_of_time() : SpellScriptLoader("spell_item_sands_of_time") { }

        class spell_item_sands_of_time_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_sands_of_time_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, int32 & amount, bool & /*canBeRecalculated*/)
            {
                if (Aura* aura = aurEff->GetBase())
                    if (AuraEffect* aurEff0 = aura->GetEffect(EFFECT_0))
                        amount = aurEff0->GetAmount();
            }

            void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                Unit* target = GetTarget();

                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                if (target->HasAura(229333))
                    return;

                int32 damage = dmgInfo.GetDamage();
                if (damage > aurEff->GetAmount())
                    damage = aurEff->GetAmount();
                float bp0 = dmgInfo.GetDamage() / 5;

                Item* castItem = NULL;
                if (ObjectGuid itemGUID = aurEff->GetBase()->GetCastItemGUID())
                    if (Player* player = target->ToPlayer())
                        castItem = player->GetItemByGuid(itemGUID);

                target->CastCustomSpell(target, 229457, &bp0, NULL, NULL, true);
                target->CastSpell(target, 225720, true, castItem);
                target->CastSpell(target, 229333, true);
                absorbAmount = dmgInfo.GetDamage();
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_sands_of_time_AuraScript::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_sands_of_time_AuraScript();
        }
};

//203808, 203843, 203844
class spell_item_honorable_pennant : public SpellScriptLoader
{
    public:
        spell_item_honorable_pennant() : SpellScriptLoader("spell_item_honorable_pennant") {}

        class spell_item_honorable_pennant_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_item_honorable_pennant_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if (Player* player = GetCaster()->ToPlayer())
                {
                    bool TeamAli = player->GetTeam() == ALLIANCE;

                    if (GetSpellInfo()->Id == 203808)
                        player->CastSpell(player, TeamAli ? 203812 : 203814, true); // Honorable Pennant
                    else if (GetSpellInfo()->Id == 203843)
                        player->CastSpell(player, TeamAli ? 203846 : 203849, true); // Prestigious Pennant
                    else if (GetSpellInfo()->Id == 203844)
                        player->CastSpell(player, TeamAli ? 203847 : 203850, true); // Elite Pennant
                    else if (GetSpellInfo()->Id == 203845)
                        player->CastSpell(player, TeamAli ? 203848 : 203852, true); // Esteemed Pennant
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_item_honorable_pennant_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_item_honorable_pennant_SpellScript();
        }
};

// 242105 Vigilance Perch
class spell_item_vigilance_perch : public AuraScript
{
    PrepareAuraScript(spell_item_vigilance_perch);
 
    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            if (Aura* aura = caster->GetAura(242066))
                if (aura->GetStackAmount() >= 5 && !caster->isInCombat() && !caster->HasAura(242070) && caster->GetShapeshiftForm() == FORM_NONE)
                    caster->CastSpell(caster, 242070, true);
    }
 
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_vigilance_perch::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};
 
// 242111 The Sentinels Eternal Refuge
class spell_item_sentinels_eternal_refuge : public AuraScript
{
    PrepareAuraScript(spell_item_sentinels_eternal_refuge);
 
    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
            if (Aura* aura = caster->GetAura(241846))
                if (aura->GetStackAmount() >= 5 && !caster->isInCombat() && !caster->HasAura(241849) && caster->GetShapeshiftForm() == FORM_NONE)
                    caster->CastSpell(caster, 241849, true);
    }
 
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_sentinels_eternal_refuge::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Orb of Destruction - 229700
class spell_item_orb_of_destruction : public SpellScript
{
    PrepareSpellScript(spell_item_orb_of_destruction);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Position const* pos = GetExplTargetDest();
        if (Unit* target = GetHitUnit())
        {
            int32 pct = 100 - (75.f / 20.f * target->GetDistance(*pos));
            SetHitDamage(CalculatePct(GetHitDamage(), pct));
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_orb_of_destruction::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 253242 - Prototype Personnel Decimator
class spell_item_proto_person_decimator_proc : public AuraScript
{
    PrepareAuraScript(spell_item_proto_person_decimator_proc);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = eventInfo.GetProcTarget())
            {
                if (caster->GetDistance(target) < aurEff->GetAmount())
                    PreventDefaultAction();
                return;
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_proto_person_decimator_proc::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 255629 - Prototype Personnel Decimator
class spell_item_proto_person_decimator : public SpellScript
{
    PrepareSpellScript(spell_item_proto_person_decimator);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Position const* pos = GetExplTargetDest();
        if (Unit* target = GetHitUnit())
        {
            int32 pct = 100 - target->GetDistance(*pos) * 4.f;
            SetHitDamage(CalculatePct(GetHitDamage(), pct));
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_proto_person_decimator::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Felshield - 253277
class spell_item_felshield : public AuraScript
{
    PrepareAuraScript(spell_item_felshield);

    float absorb = 0.f;
    bool startRemove = false;

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (startRemove)
            return;

        startRemove = true;
        if (Unit* caster = GetCaster())
            if (Unit* target = GetUnitOwner())
                caster->CastCustomSpell(target, 253278, &absorb, nullptr, nullptr, true);
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
    {
        absorbAmount = dmgInfo.GetDamage();
        absorb += CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_1]->BasePoints);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_item_felshield::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_felshield::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 215405 - Rancid Maw
class spell_item_raincid_maw : public SpellScript
{
    PrepareSpellScript(spell_item_raincid_maw);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* target = GetHitUnit();
        if (!target)
            return;

        if (Aura* aur = caster->GetAura(215404))
        {
            int32 pct = caster->GetDistance(target) / 20.f * 100.f;
            if (pct > 100.f)
                pct = 100.f;
            SetHitDamage(CalculatePct(GetHitDamage(), pct));
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_raincid_maw::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 242458 - Rising Tides
class spell_item_rishing_tides : public AuraScript
{
    PrepareAuraScript(spell_item_rishing_tides);

    void OnTick(AuraEffect const* aurEff)
    {
        if (auto caster = GetCaster())
            if (Aura* aur = caster->GetAura(GetSpellInfo()->Id))
                if (!caster->isMoving())
                    aur->ModStackAmount(1);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_rishing_tides::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 251952 - Hammer-Forged, 224347 - Solemnity
class spell_item_hammer_forged : public SpellScript
{
    PrepareSpellScript(spell_item_hammer_forged);

    void ModAuraStack()
    {
        if (Aura* aur = GetHitAura())
            aur->SetStackAmount(GetSpellInfo()->GetAuraOptions()->CumulativeAura);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_item_hammer_forged::ModAuraStack);
    }
};

// 181765 181884 selfie cams aura
class spell_item_camera_selfie : public SpellScriptLoader
{
public:
    spell_item_camera_selfie() : SpellScriptLoader("spell_item_camera_selfie") { }

    class spell_item_camera_selfie_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_item_camera_selfie_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                for (uint32 filters : { 181767, 181779, 181773 })
                {
                    if (caster->HasAura(filters))
                    {
                        caster->RemoveAura(filters);
                        break;
                    }
                }
                caster->CancelSpellVisualKit(54169);
            }
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_item_camera_selfie_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_CAMERA_SELFIE, AURA_EFFECT_HANDLE_REAL);
        }

    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_item_camera_selfie_AuraScript();
    }
};

// 161399
class spell_item_swapblaster : public SpellScript
{
    PrepareSpellScript(spell_item_swapblaster);

    void HandleDummy(SpellEffIndex effIndex)
    {
        Player* player = GetCaster()->ToPlayer();
        Player* target = GetHitUnit()->ToPlayer();
        if (!player || !target)
            return;

        player->TeleportTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation());
        target->TeleportTo(player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_swapblaster::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 193832
class spell_item_spike_toed_booterang : public SpellScript
{
    PrepareSpellScript(spell_item_spike_toed_booterang);

    SpellCastResult CheckRequirement()
    {
        Player* player = GetCaster()->ToPlayer();

        if (!player)
            return SPELL_FAILED_DONT_REPORT;

        if (Unit* target = player->GetSelectedUnit())
        {
            if (target->getLevel() > 100)
                return SPELL_FAILED_BAD_TARGETS;
        }
        return SPELL_CAST_OK;
    }

    void HandleDummy()
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        if (target->IsMounted())
            target->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_spike_toed_booterang::CheckRequirement);
        AfterHit += SpellHitFn(spell_item_spike_toed_booterang::HandleDummy);
    }
};

uint32 RandomMorphs[16] =
{
    30089,
    30096,
    30084,
    29907,
    19840,
    30085,
    30086,
    29909,
    30094,
    30093,
    29908,
    30088,
    7614,
    30092,
    30095,
    11670
};

// 74589
class spell_item_faded_wizard_hat : public AuraScript
{
    PrepareAuraScript(spell_item_faded_wizard_hat);

    void OnApply(AuraEffect const*, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->SetDisplayId(RandomMorphs[urand(0, 15)]);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_item_faded_wizard_hat::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
    }
};

uint32 MaleModels[] = { 20122, 20124, 20130, 20131, 20132, 20133, 20134, 20756, 20757, 20758, 20759 };
class spell_item_demon_hunters_aspect : public AuraScript
{
    PrepareAuraScript(spell_item_demon_hunters_aspect);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* player = GetTarget()->ToPlayer();

        if (!player)
            return;

        if (player->getGender() == GENDER_MALE)
            player->SetDisplayId(MaleModels[urand(0, 10)]);
        else
            player->SetDisplayId(20126);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* player = GetTarget()->ToPlayer();

        if (!player)
            return;

        player->SetDisplayId(player->GetNativeDisplayId());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectRemoveFn(spell_item_demon_hunters_aspect::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_item_demon_hunters_aspect::OnRemove, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
    }
};

// 242459 - Ocean's Embrace
class spell_item_ocean_embrace : public SpellScriptLoader
{
    public:
        spell_item_ocean_embrace() : SpellScriptLoader("spell_item_ocean_embrace") { }

        class spell_item_ocean_embrace_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_item_ocean_embrace_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (auto itemGUID = aurEff->GetBase()->GetCastItemGUID())
                    if (auto plr = GetCaster()->ToPlayer())
                        if (auto castItem = plr->GetItemByGuid(itemGUID))
                        {
                            switch (plr->GetSpecializationId())
                            {
                                case SPEC_DRUID_RESTORATION:
                                    plr->CastSpell(plr, 242460, true, castItem);
                                    break;
                                case SPEC_MONK_MISTWEAVER:
                                    plr->CastSpell(plr, 242461, true, castItem);
                                    break;
                                case SPEC_PRIEST_HOLY:
                                    plr->CastSpell(plr, 242462, true, castItem);
                                    break;
                                case SPEC_PRIEST_DISCIPLINE:
                                    plr->CastSpell(plr, 242463, true, castItem);
                                    break;
                                case SPEC_PALADIN_HOLY:
                                    plr->CastSpell(plr, 242464, true, castItem);
                                    break;
                                case SPEC_SHAMAN_RESTORATION:
                                    plr->CastSpell(plr, 242465, true, castItem);
                                    break;
                            }
                        }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (auto caster = GetCaster())
                {
                    caster->RemoveAurasDueToSpell(242460);
                    caster->RemoveAurasDueToSpell(242461);
                    caster->RemoveAurasDueToSpell(242462);
                    caster->RemoveAurasDueToSpell(242463);
                    caster->RemoveAurasDueToSpell(242464);
                    caster->RemoveAurasDueToSpell(242465);
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_item_ocean_embrace_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_item_ocean_embrace_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_item_ocean_embrace_AuraScript();
        }
};

//129339
class spell_flip_it_trigger : public SpellScript
{
    PrepareSpellScript(spell_flip_it_trigger);

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        uint32 spellId = 0;
        switch (urand(1, 2))
        {
            case 1: spellId = 129338; break;
            case 2: spellId = 128783; break;
        }
        caster->CastSpell(caster, spellId);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_flip_it_trigger::HandleAfterCast);
    }
};

//129338
class spell_flip_it_trigger_table_one : public SpellScript
{
    PrepareSpellScript(spell_flip_it_trigger_table_one);

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        for (uint32 food : { 128857, 128876, 128877, 128878, 128879 })
            caster->CastSpell(caster, food, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_flip_it_trigger_table_one::HandleAfterCast);
    }
};

//128783
class spell_flip_it_trigger_table_two : public SpellScript
{
    PrepareSpellScript(spell_flip_it_trigger_table_two);

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        for (uint32 food : { 128857, 128876, 128877, 128878 })
            caster->CastSpell(caster, food, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_flip_it_trigger_table_two::HandleAfterCast);
    }
};

/*// 257233 - Mark of the Pantheon
class spell_item_mark_of_the_panteon : public AuraScript
{
    PrepareAuraScript(spell_item_mark_of_the_panteon);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        if (Unit* caster = GetCaster())
        {
            Player* plr = caster->ToPlayer();
            if (!plr || plr->isBeingLoaded()) // Prevent crash when load char from DB
                return;

            if (caster->GetMapId() == 1712)
            {
                if (Aura* aur = GetAura())
                {
                    if (AuraEffect const* Eff = aur->GetTriggeredAuraEff())
                    {
                        uint32 spellId = 256832; // Mark of Aman'thul

                        switch (Eff->GetId())
                        {
                            case 256815: spellId = 256831; break; // Mark of Aggramar
                            case 256819: spellId = 256833; break; // Mark of Golganneth
                            case 256825: spellId = 256835; break; // Mark of Khaz'goroth
                            case 256827: spellId = 256836; break; // Mark of Norgannon
                            case 256822:                          // Mark of Eonar
                            {
                                switch (plr->GetSpecializationId())
                                {
                                    case SPEC_DRUID_RESTORATION:  spellId = 257470; break;
                                    case SPEC_MONK_MISTWEAVER:    spellId = 257471; break;
                                    case SPEC_PRIEST_HOLY:        spellId = 257474; break;
                                    case SPEC_PRIEST_DISCIPLINE:  spellId = 257473; break;
                                    case SPEC_PALADIN_HOLY:       spellId = 257472; break;
                                    case SPEC_SHAMAN_RESTORATION: spellId = 257475; break;
                                }
                            }
                            break;
                        }
                        Item* castItem = nullptr;
                        if (ObjectGuid castItemGUID = aur->GetCastItemGUID())
                        {
                            if (Player* player = caster->ToPlayer())
                                castItem = player->GetItemByGuid(castItemGUID);
                        }
                        caster->CastSpell(caster, spellId, true, castItem, aurEff, caster->GetGUID());
                        aur->SetDuration(6000);
                    }
                }
            }
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_item_mark_of_the_panteon::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};*/

// 242629 - Cunning of the Deceiver
class spell_item_cunning_of_the_deceiver : public SpellScript
{
    PrepareSpellScript(spell_item_cunning_of_the_deceiver);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aura = caster->GetAura(242630))
            {
                uint8 stack = aura->GetStackAmount();
                if (Player* plr = caster->ToPlayer())
                {
                    switch (plr->GetSpecializationId())
                    {
                        case SPEC_DRUID_BEAR:
                        {
                            if (Aura* aur1 = caster->GetAura(61336))
                                aur1->SetDuration(aur1->GetDuration() + stack * 500);
                            else
                                plr->ModSpellChargeCooldown(61336, stack * 10 * IN_MILLISECONDS);
                        }
                        break;
                        case SPEC_MONK_BREWMASTER:
                        {
                            if (Aura* aur2 = caster->GetAura(115203))
                                aur2->SetDuration(aur2->GetDuration() + stack * IN_MILLISECONDS);
                            else
                                plr->ModifySpellCooldown(115203, -stack * 10 * IN_MILLISECONDS);
                        }
                        break;
                        case SPEC_WARRIOR_PROTECTION:
                        {
                            if (Aura* aur3 = caster->GetAura(871))
                                aur3->SetDuration(aur3->GetDuration() + stack * 750);
                            else
                                plr->ModifySpellCooldown(871, -stack * 10 * IN_MILLISECONDS);
                        }
                        break;
                        case SPEC_DK_BLOOD:
                        {
                            if (Aura* aur4 = caster->GetAura(48792))
                                aur4->SetDuration(aur4->GetDuration() + stack * 750);
                            else
                                plr->ModifySpellCooldown(48792, -stack * 10 * IN_MILLISECONDS);
                        }
                        break;
                        case SPEC_PALADIN_PROTECTION:
                        {
                            if (Aura* aur5 = caster->GetAura(86659))
                                aur5->SetDuration(aur5->GetDuration() + stack * IN_MILLISECONDS);
                            if (Aura* aura5 = caster->GetAura(212641))
                                aura5->SetDuration(aura5->GetDuration() + stack * IN_MILLISECONDS);

                            else
                            {
                                plr->ModifySpellCooldown(86659, -stack * 10 * IN_MILLISECONDS);
                                plr->ModifySpellCooldown(212641, -stack * 10 * IN_MILLISECONDS);
                            }
                        }
                        break;
                        case SPEC_DEMON_HUNER_VENGEANCE:
                        {
                            if (Aura* aur6 = caster->GetAura(187827))
                                aur6->SetDuration(aur6->GetDuration() + stack * 750);
                            else
                                plr->ModifySpellCooldown(187827, -stack * 10 * IN_MILLISECONDS);
                        }
                        break;
                    }
                    aura->Remove();
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_cunning_of_the_deceiver::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 242609 - Wailing Souls
class spell_item_walling_souls : public AuraScript
{
    PrepareAuraScript(spell_item_walling_souls);

    uint32 absorbPct;

    bool Load() override
    {
        absorbPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
        return true;
    }

    void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
    {
        absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
    }

    void Trigger(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
    {
        if (auto caster = GetCaster())
        {
            if (Player* plr = caster->ToPlayer())
            {
                float bp = dmgInfo.GetDamage() / absorbAmount;
                int32 CapCD = GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
                if (bp > CapCD)
                    bp = CapCD;
                plr->ModifySpellCooldown(242609, -bp * IN_MILLISECONDS);
            }
        }
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_walling_souls::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        AfterEffectAbsorb += AuraEffectAbsorbFn(spell_item_walling_souls::Trigger, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

class spell_mystic_image : public SpellScript
{
    PrepareSpellScript(spell_mystic_image);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* pet = GetHitUnit();
        if (!pet)
            return;

        pet->CastSpell(caster, 187358, false);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mystic_image::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 252921 - Sorrow
class spell_item_sorrow_dummy : public AuraScript
{
    PrepareAuraScript(spell_item_sorrow_dummy);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto caster = GetCaster())
        {
            float bp0 = aurEff->GetAmount();
            caster->CastCustomSpell(GetTarget(), 253022, &bp0, nullptr, nullptr, true);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_item_sorrow_dummy::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 214577 - Nightwell Energy
class spell_item_nightwell_energy : public AuraScript
{
    PrepareAuraScript(spell_item_nightwell_energy);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
    {
        if (auto caster = GetCaster())
        {
            if (Aura* aur = caster->GetAura(214572))
            {
                amount = amount * aur->GetStackAmount();
                aur->Remove();
            }
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_nightwell_energy::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 224437 - Fetid Regurgitation
class spell_item_fetid_regurgitation : public SpellScript
{
    PrepareSpellScript(spell_item_fetid_regurgitation);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
            if (Aura* aur = caster->GetAura(215126))
                SetHitDamage(GetHitDamage() * aur->GetStackAmount());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_fetid_regurgitation::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 170950
class spell_item_haunting_memento : public AuraScript
{
    PrepareAuraScript(spell_item_haunting_memento);

    uint32 random_effect[7] = { 170953, 170954, 170955, 170956, 170957, 170958, 170961 };

    void OnProc(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        target->CastSpell(target, random_effect[urand(0, 6)], true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_item_haunting_memento::OnProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 182993
class spell_item_leystone_hoofplates : public AuraScript
{
    PrepareAuraScript(spell_item_leystone_hoofplates);

    uint16 timer;

    bool Load()
    {
        timer = 2000;
        return true;
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (timer <= diff)
            if (caster->HasAura(182993) && caster->GetMapId() == 1220 && !caster->HasAura(233044))
                caster->CastSpell(caster, 233044, false);
        else
            timer -= diff;
    }

    void CalculateMaxDuration(int32& duration)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        duration = 120 * MINUTE * IN_MILLISECONDS;

        if (caster->ToPlayer()->HasSkill(164))
            duration = duration * 4;
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAura(233044);
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_item_leystone_hoofplates::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_item_leystone_hoofplates::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_item_leystone_hoofplates::CalculateMaxDuration);
    }
};

// 209563
class spell_item_demonsteel_stirrups : public AuraScript
{
    PrepareAuraScript(spell_item_demonsteel_stirrups);

    uint16 timer;

    bool Load() override
    {
        timer = 2000;
        return true;
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (timer <= diff)
            if (caster->HasAura(209563) && caster->GetMapId() == 1220 && !caster->HasAura(233043))
                caster->CastSpell(caster, 233043, false);
            else
                timer -= diff;
    }

    void CalculateMaxDuration(int32& duration)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        duration = 120 * MINUTE * IN_MILLISECONDS;

        if (caster->ToPlayer()->HasSkill(164))
            duration = duration * 4;
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAura(233043);
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_item_demonsteel_stirrups::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
        DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_item_demonsteel_stirrups::CalculateMaxDuration);
        AfterEffectRemove += AuraEffectRemoveFn(spell_item_demonsteel_stirrups::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 134359
class spell_item_golem : public AuraScript
{
    PrepareAuraScript(spell_item_golem);

    uint16 timer;

    bool Load() override
    {
        timer = 500;
        return true;
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        if (!caster)
            return;

        if (caster->GetMap()->IsBattlegroundOrArena() || caster->GetMap()->IsRaid() || caster->GetMap()->IsDungeon())
            return;

        if (timer <= diff)
        {
            if (caster->HasSkill(182))
            {
                timer = 2000;

                if (caster->FindNearestGameObjectOfType(GAMEOBJECT_TYPE_GATHERING_NODE, 3.0f))
                {
                    if (!caster->HasFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED))
                        caster->SetFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED);
                }
                else
                {
                    if (caster->HasFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED))
                        caster->RemoveFlag(PLAYER_FIELD_LOCAL_FLAGS, PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED);
                }
            }
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_item_golem::OnUpdate, EFFECT_0, SPELL_AURA_MOUNTED);
    }
};

// 208890
class spell_item_leystone_buoy : public AuraScript
{
    PrepareAuraScript(spell_item_leystone_buoy);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->IsInWater())
            caster->GetMotionMaster()->MovePoint(1, caster->GetPositionX(), caster->GetPositionY(), caster->GetWaterOrGroundLevel(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ()));
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_item_leystone_buoy::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 148385
class spell_censer_of_eternal_agony : public AuraScript
{
    PrepareAuraScript(spell_censer_of_eternal_agony);

    void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto caster = GetCaster())
            caster->SetPvP(true);
    }

    void HandlePeriodicTick(AuraEffect const* aurEff)
    {
        if (auto caster = GetCaster())
            caster->SetPvP(true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_censer_of_eternal_agony::HandlePeriodicTick, EFFECT_3, SPELL_AURA_PERIODIC_DUMMY);
        AfterEffectApply += AuraEffectApplyFn(spell_censer_of_eternal_agony::HandleEffectApply, EFFECT_0, SPELL_AURA_MOD_FACTION, AURA_EFFECT_HANDLE_REAL);
    }
};

// 215405 - Rancid Maw
class spell_item_rancid_maw : public SpellScript
{
    PrepareSpellScript(spell_item_rancid_maw);

    uint8 perc;

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        if (Unit* target = GetHitUnit())
        {
            if (caster->GetDistance(target) >= 20)
                perc = 100;
            else
                perc = 100 - caster->GetDistance(target);

            SetHitDamage(CalculatePct(GetHitDamage(), perc));
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_rancid_maw::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 214366 - Crystalline Body
class spell_item_crystalline_body : public AuraScript
{
    PrepareAuraScript(spell_item_crystalline_body);

    void Absorb(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, float& absorbAmount)
    {
        if (Aura* aura = aurEff->GetBase())
            if (AuraEffect* aurEff0 = aura->GetEffect(EFFECT_1))
                absorbAmount = aurEff0->GetAmount();
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_crystalline_body::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 190810
class spell_masters_hunter_seeking_crystal : public SpellScript
{
    PrepareSpellScript(spell_masters_hunter_seeking_crystal);

    SpellCastResult CheckElevation()
    {
        auto player = GetCaster();

        if (!player)
            return SPELL_FAILED_NO_VALID_TARGETS;
        
        for (uint32 RareEntries : {91871,92552,92451,90884, 90777, 92411, 92429, 91098, 91695, 93125, 98284, 92941, 92274, 90442,
            93057, 90437, 90887, 98283, 91093, 90888, 93028, 92495, 92977, 91087, 93001, 90122, 93264, 93076, 80398, 90434, 95053,
            91232, 89675, 92517, 93279, 90936, 92694, 92408, 96424, 92647, 95044, 90438, 93002, 91009, 92508, 98408, 90094, 90519,
            95054, 91727, 91374, 90429, 95056, 92627, 90782, 92197, 90885, 90024, 92606, 93168, 92887, 93236, 91243, 92574, 92657,
            92645, 92636, 91227, 92465, 98285})
            if (auto crea = player->FindNearestCreature(RareEntries, 500.0f, true))
                return SPELL_CAST_OK;

        return SPELL_FAILED_NO_VALID_TARGETS;
    }

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, 190811, true);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_masters_hunter_seeking_crystal::CheckElevation);
        AfterCast += SpellCastFn(spell_masters_hunter_seeking_crystal::HandleAfterCast);
    }
};

// 190811
class spell_masters_hunter_seeking_crystal_teleport : public SpellScript
{
    PrepareSpellScript(spell_masters_hunter_seeking_crystal_teleport);

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        for (uint32 RareEntries : {91871, 92552, 92451, 90884, 90777, 92411, 92429, 91098, 91695, 93125, 98284, 92941, 92274, 90442,
            93057, 90437, 90887, 98283, 91093, 90888, 93028, 92495, 92977, 91087, 93001, 90122, 93264, 93076, 80398, 90434, 95053,
            91232, 89675, 92517, 93279, 90936, 92694, 92408, 96424, 92647, 95044, 90438, 93002, 91009, 92508, 98408, 90094, 90519,
            95054, 91727, 91374, 90429, 95056, 92627, 90782, 92197, 90885, 90024, 92606, 93168, 92887, 93236, 91243, 92574, 92657,
            92645, 92636, 91227, 92465, 98285})
            if (auto crea = caster->FindNearestCreature(RareEntries, 500.0f, true))
            {
                Position pos = crea->GetPosition();
                if (crea->GetEntry() == 96424)
                {
                    caster->CastSpell(caster, 190820, true);
                    caster->AddDelayedEvent(500, [caster, pos] {
                        caster->NearTeleportTo(pos);    });
                }
                else
                {
                    caster->AddDelayedEvent(500, [caster, pos] {
                        caster->NearTeleportTo(pos);    });
                }
            }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_masters_hunter_seeking_crystal_teleport::HandleAfterCast);
    }
};

// 243941 - Insidious Corruption
class spell_item_insidious_corruption : public AuraScript
{
    PrepareAuraScript(spell_item_insidious_corruption);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode == AURA_REMOVE_BY_DEATH || removeMode == AURA_REMOVE_BY_EXPIRE)
            {
                if (Aura* corrupt = aurEff->GetBase())
                {
                    if (ObjectGuid itemGUID = corrupt->GetCastItemGUID())
                    {
                        if (Player* player = caster->ToPlayer())
                        {
                            if (Item* castItem = player->GetItemByGuid(itemGUID))
                            {
                                caster->CastSpell(caster, 243942, true, castItem);
                                if (Aura* aur = caster->GetAura(243942))
                                    aur->SetDuration(aur->GetDuration() + corrupt->GetDuration());
                            }
                        }
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_item_insidious_corruption::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_moonfeather_fever : public AuraScript
{
    PrepareAuraScript(spell_moonfeather_fever);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (Aura* aura = target->GetAura(195776))
        {
            if (aura->GetStackAmount() != 20)
                target->AddAura(195776, target);

            if (aura->GetStackAmount() == 8)
                target->CastSpell(target, 195802, false);
            else if (aura->GetStackAmount() == 10)
                target->CastSpell(target, 195805, false);
            else if (aura->GetStackAmount() == 15)
                target->CastSpell(target, 195810, false);
            else if (aura->GetStackAmount() == 20)
                target->CastSpell(target, 195816, false);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_moonfeather_fever::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 230357
class spell_ivory_talon_main : public SpellScript
{
    PrepareSpellScript(spell_ivory_talon_main);

    SpellCastResult CheckElevation()
    {
        auto caster = GetCaster();
        if (!caster)
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (auto player = caster->ToPlayer())
        {
            if (player->GetTeam() == ALLIANCE)
            {
                switch (caster->GetCurrentAreaID())
                {
                case 8304:
                case 8324:
                case 8328:
                case 8320:
                case 8078:
                case 8051:
                    for (uint32 pvpQ : {43183, 43248, 43601, 43599, 41421, 41257})
                        if (player->GetQuestStatus(pvpQ) == QUEST_STATUS_INCOMPLETE)
                            return SPELL_CAST_OK;
                    break;
                }
            }
            else
            {
                switch (caster->GetCurrentAreaID())
                {
                case 8304:
                case 8324:
                case 8328:
                case 8320:
                case 8078:
                case 8051:
                    for (uint32 pvpQ : {41227, 41420, 43598, 43600, 43247, 42070})
                        if (player->GetQuestStatus(pvpQ) == QUEST_STATUS_INCOMPLETE)
                            return SPELL_CAST_OK;
                    break;
                }
            }
        }
        return SPELL_FAILED_NOT_HERE; 
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_ivory_talon_main::CheckElevation);
    }
};

// 253258 - Reverberating Vitality
class spell_item_reverberating_vitality : public AuraScript
{
    PrepareAuraScript(spell_item_reverberating_vitality);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
            if (Unit* target = caster->getVictim())
                amount = CalculatePct(amount, target->GetHealthPct());
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_item_reverberating_vitality::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_STAT);
    }
};

// 253287 - Highfather's Timekeeping
class spell_item_highfathers_timekeeping : public AuraScript
{
    PrepareAuraScript(spell_item_highfathers_timekeeping);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = eventInfo.GetActionTarget())
            {
                if (target->GetHealthPct() > GetSpellInfo()->Effects[EFFECT_0]->BasePoints)
                {
                    PreventDefaultAction();
                    return;
                }
                else
                {
                    if (Player* plr = caster->ToPlayer())
                    {
                        if (Item* castItem = plr->GetItemByGuid(GetAura()->GetCastItemGUID()))
                        {
                            if (Aura* aur = caster->GetAura(255932))
                            {
                                if (AuraEffect* eff = aur->GetEffect(EFFECT_0))
                                {
                                    float heal = eff->GetAmount() * aurEff->GetBase()->GetStackAmount();
                                    caster->CastCustomSpell(target, 253288, &heal, nullptr, nullptr, true, castItem);
                                    aurEff->GetBase()->Remove();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_highfathers_timekeeping::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// March of the Legion - 228445
class spell_item_march_of_the_legion : public AuraScript
{
    PrepareAuraScript(spell_item_march_of_the_legion);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* target = eventInfo.GetProcTarget())
        {
            if (target->GetCreatureType() != CREATURE_TYPE_DEMON)
                PreventDefaultAction();
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_item_march_of_the_legion::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// 230121 - Guardian's Familiar
class spell_item_guardians_familiar : public AuraScript
{
    PrepareAuraScript(spell_item_guardians_familiar);

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
    {
        if (Unit* caster = GetCaster())
        {
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_2]->BasePoints);
            caster->CastCustomSpell(caster, 230166, &absorbAmount, nullptr, nullptr, true);
        }
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_item_guardians_familiar::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

class spell_item_toy_train_set_pulse : public SpellScript
{
    PrepareSpellScript(spell_item_toy_train_set_pulse);

    void HandleDummy(SpellEffIndex /*index*/)
    {
        if (Player* target = GetHitUnit()->ToPlayer())
        {
            target->HandleEmoteCommand(EMOTE_ONESHOT_TRAIN);

            switch (target->getRace())
            {
            case RACE_HUMAN:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7634);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7635);
                    break;
                }
                break;
            case RACE_WORGEN:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(RAND(19357, 23323));
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(RAND(19445, 23318));
                    break;
                }
                break;
            case RACE_ORC:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7638);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7639);
                    break;
                }
                break;
            case RACE_DWARF:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7636);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7637);
                    break;
                }
                break;
            case RACE_NIGHTELF:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7643);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7642);
                    break;
                }
                break;
            case RACE_NIGHTBORNE:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(96400);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(96332);
                    break;
                }
                break;
            case RACE_UNDEAD_PLAYER:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7644);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7645);
                    break;
                }
                break;
            case RACE_TAUREN:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7646);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7647);
                    break;
                }
                break;
            case RACE_HIGHMOUNTAIN_TAUREN:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(95882);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(95560);
                    break;
                }
                break;
            case RACE_GNOME:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7641);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7640);
                    break;
                }
                break;
            case RACE_TROLL:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(7648);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(7649);
                    break;
                }
                break;
            case RACE_BLOODELF:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    if (target->getClass() == CLASS_DEMON_HUNTER)
                        target->PlayDistanceSound(56584);
                    else
                        target->PlayDistanceSound(9672);
                    break;
                case GENDER_FEMALE:
                    if (target->getClass() == CLASS_DEMON_HUNTER)
                        target->PlayDistanceSound(57024);
                    else
                        target->PlayDistanceSound(9644);
                    break;
                }
                break;
            case RACE_VOID_ELF:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(95682);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(95888);
                    break;
                }
                break;
            case RACE_DRAENEI:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(9722);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(9697);
                    break;
                }
                break;
            case RACE_LIGHTFORGED_DRAENEI:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(96264);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(96196);
                    break;
                }
                break;
            case RACE_GOBLIN:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(19147);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(19257);
                    break;
                }
                break;
            case RACE_PANDAREN_HORDE:
            case RACE_PANDAREN_ALLIANCE:
            case RACE_PANDAREN_NEUTRAL:
                switch (target->getGender())
                {
                case GENDER_MALE:
                    target->PlayDistanceSound(28932);
                    break;
                case GENDER_FEMALE:
                    target->PlayDistanceSound(29826);
                    break;
                }
                break;
            }
        }
    }

    void HandleTargets(std::list<WorldObject*>& targetList)
    {
        targetList.remove_if([](WorldObject const* obj) { return !obj->IsPlayer(); });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_toy_train_set_pulse::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_item_toy_train_set_pulse::HandleTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_item_toy_train_set_pulse::HandleTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

class spell_item_last_deck_of_nemelex_nobeh : public SpellScript
{
    PrepareSpellScript(spell_item_last_deck_of_nemelex_nobeh);

    SpellCastResult CheckRequirement()
    {
        if (auto caster = GetCaster()->ToPlayer())
        {
            if (caster->GetMap()->IsBattleground())
                return SPELL_FAILED_NOT_IN_BATTLEGROUND;
            else if (caster->GetMap()->IsDungeon())
                return SPELL_FAILED_NOT_IN_LFG_DUNGEON;
        }

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_item_last_deck_of_nemelex_nobeh::CheckRequirement);
    }
};

enum AcridCatalystInjector
{
	SPELL_FERVOR_OF_THE_LEGION = 253261, // Haste
	SPELL_BURTALITY_OF_THE_LEGION = 255742, // Critical
	SPELL_MALICE_OF_THE_LEGION = 255744, // Mastery
	SPELL_CYCLE_OF_THE_LEGION = 253260 // Haste, Mastery, and Critical
};

// Item - 151955: Acrid Catalyst Injector
// 253259 - Item - Antorus, the Burning Throne (End raid of legion expansion)
template <uint32 CriticalSpellId, uint32 HasteSpellId, uint32 MasterySpellId>
class spell_item_acrid_catalyst_injector : public SpellScriptLoader
{
public:
	spell_item_acrid_catalyst_injector(char const* ScriptName) : SpellScriptLoader(ScriptName) { }

	template <uint32 Critical, uint32 Haste, uint32 Mastery>
	class spell_item_acrid_catalyst_injector_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_item_acrid_catalyst_injector_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/) override
		{
			return ValidateSpellInfo(
				{
					Critical,
					Haste,
					Mastery
				});
		}

		void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
		{
			/*if (eventInfo.GetHitMask() & PROC_HIT_CRITICAL)
			{*/
			static std::vector<uint32> const triggeredSpells[1] =
			{
				{ Critical, Haste, Mastery }
			};

			//PreventDefaultAction();
			if (Unit* caster = eventInfo.GetActor()) 
			{

				if (Aura* aur = GetAura())
				{
					if (AuraEffect const* Eff = aur->GetTriggeredAuraEff())
					{
						Item* castItem = nullptr;
						if (ObjectGuid castItemGUID = aur->GetCastItemGUID())
						{
							if (Player* player = caster->ToPlayer())
								castItem = player->GetItemByGuid(castItemGUID);
						}

						std::vector<uint32> const& randomSpells = triggeredSpells[0];
						if (randomSpells.empty())
							return;

						uint32 spellId = Trinity::Containers::SelectRandomContainerElement(randomSpells);

						//caster->CastSpell(caster, spellId, true, nullptr, aurEff);

						caster->CastSpell(caster, spellId, true, castItem, aurEff, caster->GetGUID());
						//aur->SetDuration(6000);

						for (std::vector<uint32>::const_iterator it = randomSpells.begin(); it != randomSpells.end(); ++it)
						{
							if (caster->HasAura((*it)))
							{
								auto trinketAura = caster->GetAura((*it));
								if (trinketAura->GetStackAmount() >= 5)
								{
									//caster->CastSpell(caster, SPELL_CYCLE_OF_THE_LEGION, true, nullptr, aurEff);
									caster->CastSpell(caster, SPELL_CYCLE_OF_THE_LEGION, true, castItem, aurEff, caster->GetGUID());
									caster->RemoveAura(Critical);
									caster->RemoveAura(Haste);
									caster->RemoveAura(Mastery);
									break;
								}
							}
						}
					}
				}





				//}
			}

		}

		void Register() override
		{
			OnEffectProc += AuraEffectProcFn(spell_item_acrid_catalyst_injector_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
		}
	};

	AuraScript* GetAuraScript() const override
	{
		return new spell_item_acrid_catalyst_injector_AuraScript<CriticalSpellId, HasteSpellId, MasterySpellId>();
	}
};

void AddSC_item_spell_scripts()
{
    // 23074 Arcanite Dragonling
    new spell_item_trigger_spell("spell_item_arcanite_dragonling", SPELL_ARCANITE_DRAGONLING);
    // 23133 Gnomish Battle Chicken
    new spell_item_trigger_spell("spell_item_gnomish_battle_chicken", SPELL_BATTLE_CHICKEN);
    // 23076 Mechanical Dragonling
    new spell_item_trigger_spell("spell_item_mechanical_dragonling", SPELL_MECHANICAL_DRAGONLING);
    // 23075 Mithril Mechanical Dragonling
    new spell_item_trigger_spell("spell_item_mithril_mechanical_dragonling", SPELL_MITHRIL_MECHANICAL_DRAGONLING);
    new spell_item_deviate_fish();
    new spell_item_flask_of_the_north();
    new spell_item_gnomish_death_ray();
    new spell_item_make_a_wish();
    new spell_item_mingos_fortune_generator();
    new spell_item_net_o_matic();
    new spell_item_noggenfogger_elixir();
    new spell_item_savory_deviate_delight();
    new spell_item_six_demon_bag();
    new spell_item_underbelly_elixir();
    new spell_item_shadowmourne();
    new spell_item_red_rider_air_rifle();
    new spell_item_create_heart_candy();
    new spell_item_book_of_glyph_mastery();
    new spell_item_gift_of_the_harvester();
    new spell_item_map_of_the_geyser_fields();
    new spell_item_vanquished_clutches();
    new spell_magic_eater_food();
    new spell_item_refocus();
    new spell_item_shimmering_vessel();
    new spell_item_purify_helboar_meat();
    new spell_item_crystal_prison_dummy_dnd();
    new spell_item_reindeer_transformation();
    new spell_item_nigh_invulnerability();
    new spell_item_poultryizer();
    new spell_item_socrethars_stone();
    new spell_item_demon_broiled_surprise();
    new spell_item_complete_raptor_capture();
    new spell_item_impale_leviroth();
    new spell_item_brewfest_mount_transformation();
    new spell_item_teach_language();
    new spell_item_rocket_boots();
    new spell_item_pygmy_oil();
    new spell_item_chicken_cover();
    new spell_item_muisek_vessel();
    new spell_item_greatmothers_soulcatcher();
    new spell_item_enohar_explosive_arrows();
    new spell_item_holy_thurible();
    new spell_item_gen_alchemy_mop();
    new spell_alchemist_rejuvenation();
    new spell_item_hardened_shell();
    new spell_item_elixir_of_wandering_spirits();
    new spell_item_ai_li_skymirror();
    new spell_item_eye_of_the_black_prince();
    new spell_item_book_of_the_ages();
    new spell_item_chocolate_cookie();
    new spell_item_essence_of_wrathion();
    new spell_item_warlords_fortitude();
    new spell_item_prydaz_abs();
    new spell_item_goren_gas_extractor();
    new spell_item_living_carapace();
    new spell_item_infernal_contract();
    new spell_item_sands_of_time();
    new spell_item_honorable_pennant();
    RegisterAuraScript(spell_item_vigilance_perch);
    RegisterAuraScript(spell_item_sentinels_eternal_refuge);
    RegisterSpellScript(spell_item_orb_of_destruction);
    RegisterAuraScript(spell_item_proto_person_decimator_proc);
    RegisterSpellScript(spell_item_proto_person_decimator);
    RegisterAuraScript(spell_item_felshield);
    RegisterSpellScript(spell_item_raincid_maw);
    RegisterAuraScript(spell_item_rishing_tides);
    RegisterSpellScript(spell_item_hammer_forged);
    new spell_item_camera_selfie();
    RegisterSpellScript(spell_gamons_heroic_spirit);
    RegisterSpellScript(spell_autographed_hearthstone_card);
    RegisterSpellScript(spell_item_spinning);
    RegisterSpellScript(spell_item_slightly_chewed_insult_book);
    RegisterSpellScript(spell_item_swapblaster);
    RegisterSpellScript(spell_item_spike_toed_booterang);
    RegisterSpellScript(spell_flip_it_trigger);
    RegisterSpellScript(spell_flip_it_trigger_table_one);
    RegisterSpellScript(spell_flip_it_trigger_table_two);
    RegisterAuraScript(spell_item_faded_wizard_hat);
    RegisterAuraScript(spell_item_demon_hunters_aspect);
    new spell_item_ocean_embrace();
    //RegisterAuraScript(spell_item_mark_of_the_panteon);
    RegisterSpellScript(spell_item_cunning_of_the_deceiver);
    RegisterAuraScript(spell_item_walling_souls);
    RegisterSpellScript(spell_mystic_image);
    RegisterAuraScript(spell_item_sorrow_dummy);
    RegisterAuraScript(spell_item_nightwell_energy);
    RegisterSpellScript(spell_item_fetid_regurgitation);
    RegisterAuraScript(spell_item_haunting_memento);
    RegisterAuraScript(spell_item_leystone_hoofplates);
    RegisterAuraScript(spell_item_demonsteel_stirrups);
    RegisterAuraScript(spell_item_golem);
    RegisterAuraScript(spell_item_leystone_buoy);
    RegisterAuraScript(spell_censer_of_eternal_agony);
    RegisterSpellScript(spell_item_rancid_maw);
    RegisterAuraScript(spell_item_crystalline_body);
    RegisterSpellScript(spell_masters_hunter_seeking_crystal);
    RegisterSpellScript(spell_masters_hunter_seeking_crystal_teleport);
    RegisterAuraScript(spell_item_insidious_corruption);
    RegisterAuraScript(spell_moonfeather_fever);
    RegisterSpellScript(spell_ivory_talon_main);
    RegisterSpellScript(spell_item_nitro_boots);
    RegisterAuraScript(spell_item_reverberating_vitality);
    RegisterAuraScript(spell_item_highfathers_timekeeping);
    RegisterAuraScript(spell_item_march_of_the_legion);
    RegisterAuraScript(spell_item_guardians_familiar);
    RegisterSpellScript(spell_item_toy_train_set_pulse);
    RegisterSpellScript(spell_item_last_deck_of_nemelex_nobeh);

	new spell_item_acrid_catalyst_injector<SPELL_FERVOR_OF_THE_LEGION, SPELL_BURTALITY_OF_THE_LEGION, SPELL_MALICE_OF_THE_LEGION>("spell_item_acrid_catalyst_injector");
}
