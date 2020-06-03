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
 * Spells used in holidays/game events that do not fit any other category.
 * Scriptnames in this file should be prefixed with "spell_#holidayname_".
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "GameObjectAI.h"
#include "ObjectVisitors.hpp"

// 45102 Romantic Picnic
enum SpellsPicnic
{
    SPELL_BASKET_CHECK              = 45119, // Holiday - Valentine - Romantic Picnic Near Basket Check
    SPELL_MEAL_PERIODIC             = 45103, // Holiday - Valentine - Romantic Picnic Meal Periodic - effect dummy
    SPELL_MEAL_EAT_VISUAL           = 45120, // Holiday - Valentine - Romantic Picnic Meal Eat Visual
    //SPELL_MEAL_PARTICLE             = 45114, // Holiday - Valentine - Romantic Picnic Meal Particle - unused
    SPELL_DRINK_VISUAL              = 45121, // Holiday - Valentine - Romantic Picnic Drink Visual
    SPELL_ROMANTIC_PICNIC_ACHIEV    = 45123, // Romantic Picnic periodic = 5000
};

class spell_love_is_in_the_air_romantic_picnic : public SpellScriptLoader
{
    public:
        spell_love_is_in_the_air_romantic_picnic() : SpellScriptLoader("spell_love_is_in_the_air_romantic_picnic") { }

        class spell_love_is_in_the_air_romantic_picnic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_love_is_in_the_air_romantic_picnic_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                target->SetStandState(UNIT_STAND_STATE_SIT);
                target->CastSpell(target, SPELL_MEAL_PERIODIC, false);
            }

            void OnPeriodic(AuraEffect const* /*aurEff*/)
            {
                // Every 5 seconds
                Unit* target = GetTarget();
                Unit* caster = GetCaster();

                // If our player is no longer sit, remove all auras
                if (target->getStandState() != UNIT_STAND_STATE_SIT)
                {
                    target->RemoveAura(SPELL_ROMANTIC_PICNIC_ACHIEV);
                    target->RemoveAura(GetAura());
                    return;
                }

                target->CastSpell(target, SPELL_BASKET_CHECK, false); // unknown use, it targets Romantic Basket
                target->CastSpell(target, RAND(SPELL_MEAL_EAT_VISUAL, SPELL_DRINK_VISUAL), false);

                bool foundSomeone = false;
                // For nearby players, check if they have the same aura. If so, cast Romantic Picnic (45123)
                // required by achievement and "hearts" visual
                std::list<Player*> playerList;
                Trinity::AnyPlayerInObjectRangeCheck checker(target, INTERACTION_DISTANCE*2);
                Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(target, playerList, checker);
                Trinity::VisitNearbyWorldObject(target, INTERACTION_DISTANCE*2, searcher);
                for (std::list<Player*>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
                    if ((*itr) != target && (*itr)->HasAura(GetId())) // && (*itr)->getStandState() == UNIT_STAND_STATE_SIT)
                    {
                        if (caster)
                        {
                            caster->CastSpell(*itr, SPELL_ROMANTIC_PICNIC_ACHIEV, true);
                            caster->CastSpell(target, SPELL_ROMANTIC_PICNIC_ACHIEV, true);
                        }
                        foundSomeone = true;
                        // break;
                    }
                }

                if (!foundSomeone && target->HasAura(SPELL_ROMANTIC_PICNIC_ACHIEV))
                    target->RemoveAura(SPELL_ROMANTIC_PICNIC_ACHIEV);
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_love_is_in_the_air_romantic_picnic_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_love_is_in_the_air_romantic_picnic_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_love_is_in_the_air_romantic_picnic_AuraScript();
        }
};

// Hallowen: Q29075 Q29376
enum hallow
{
    GO_THE_WICKERMAN        = 180433, //The Wickerman
    GO_WICKERMAN_EMBER      = 180437, //Wickerman Ember
    GO_TORCH                = 208186,
    Q29075                  = 29075,
    Q29376                  = 29376,
};

enum evens
{
    ENABLE_WICKERMAN            = 1,
    END_BURN_WICKERMAN          = 2,
};

class spell_hallowen_torch_wickerman : public SpellScriptLoader
{
    public:
        spell_hallowen_torch_wickerman() : SpellScriptLoader("spell_hallowen_torch_wickerman") { }

        class sspell_hallowen_torch_wickerman_SpellScript : public SpellScript
        {
            PrepareSpellScript(sspell_hallowen_torch_wickerman_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (GameObject* go = _player->FindNearestGameObject(GO_THE_WICKERMAN, 100.0f))
                    {
                        go->AI()->DoAction(ENABLE_WICKERMAN);
                        go->AI()->SetGUID(_player->GetGUID(), 0);
                    }
                }
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(sspell_hallowen_torch_wickerman_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new sspell_hallowen_torch_wickerman_SpellScript();
        }
};

class go_hallow_wickerman : public GameObjectScript
{
public:
    go_hallow_wickerman() : GameObjectScript("go_hallow_wickerman") { }

    struct go_hallow_wickermanAI : public GameObjectAI
    {
        go_hallow_wickermanAI(GameObject* go) : GameObjectAI(go)
        {
            emberGUID.Clear();
            playerGUID.Clear();
        }

        EventMap events;
        ObjectGuid emberGUID;
        ObjectGuid playerGUID;

        void SetGUID(ObjectGuid const& guid, int32 /*id = 0 */) override
        {
            if (playerGUID)
                return;

            playerGUID = guid;
        }

        void DoAction(const int32 param) override
        {
            if (playerGUID)
                return;

            events.RescheduleEvent(END_BURN_WICKERMAN, 10000);
            go->EnableOrDisableGo(true, true);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case END_BURN_WICKERMAN:
                    {
                        go->EnableOrDisableGo(false, false);

                        if (Player* player = ObjectAccessor::GetPlayer(*go, playerGUID))
                            if (GameObject * e = player->SummonGameObject(GO_WICKERMAN_EMBER, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), go->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, DAY))
                                emberGUID = e->GetGUID();                            
                        events.RescheduleEvent(ENABLE_WICKERMAN, 20000);
                        go->UpdateObjectVisibility();
                        go->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_GAMEMASTER);
                        break;
                    }
                    case ENABLE_WICKERMAN:
                    {
                        playerGUID.Clear();
                        go->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);
                        go->UpdateObjectVisibility();
                        if (GameObject* e = ObjectAccessor::GetGameObject(*go, emberGUID))
                        {
                            e->Delete();
                            emberGUID.Clear();
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    };
    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_hallow_wickermanAI(go);
    }
};

void AddSC_holiday_spell_scripts()
{
    new spell_love_is_in_the_air_romantic_picnic();
    new spell_hallowen_torch_wickerman();
    new go_hallow_wickerman();
}
