/*

Event Boss Ahune Base Script

*/

#include "the_slave_pens.h"
#include "LFGMgr.h"
#include "Group.h"
#include "CreatureTextMgr.h"
#include "ScriptedGossip.h"
#include "GameObject.h"
#include "Player.h"

enum ahuneevents
{
    EVENT_BIGADD = 1,
    EVENT_SMALLADD,
    EVENT_CRYSTAL,
    EVENT_RESET,
    EVENT_ICESPEAR,
};

enum actions
{
    ACTION_AHUNE,
};

enum Says
{
    SAY_PLAYER_TEXT_1 = 0,
    SAY_PLAYER_TEXT_2 = 1,
    SAY_PLAYER_TEXT_3 = 2
};

// 25740
class boss_ahune_frost_lord : public CreatureScript
{
public:
    boss_ahune_frost_lord() : CreatureScript("boss_ahune_frost_lord") { }

    struct boss_ahune_frost_lordAI : BossAI
    {
        boss_ahune_frost_lordAI(Creature* creature) : BossAI(creature, DATA_AHUNE) 
        {
            IntroEvent = 8000;
            IntroDone = false;
            me->SetVisible(false);
            Submerge();
        }

        uint32 Phase;
        uint32 IntroEvent;
        uint32 IntroDone;

        EventMap events;

        void Reset()
        {
            Phase = 1;
            me->AddAura(45954, me);
            summons.DespawnAll();
            events.Reset();
            me->RemoveAura(46146);
            me->SetVisible(true);
            Emerge();
            IntroEvent = 8000;
        }

        void JustDied(Unit* /*killer*/) override
        {
            summons.DespawnAll();
            DoCast(45939);
            instance->DoCastSpellOnPlayers(62043);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Phase = 1;
            events.RescheduleEvent(EVENT_BIGADD, 5000);
            events.RescheduleEvent(EVENT_SMALLADD, 8000);
            events.RescheduleEvent(EVENT_CRYSTAL, 90000);
            events.RescheduleEvent(EVENT_ICESPEAR, 2000);
            DoCast(46146);
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_AHUNE)
            {
                me->SetVisible(true);
                DoCast(37752);
            }
        }

        void Emerge()
        {
            me->RemoveAurasDueToSpell(46416); //46416
            me->RemoveAurasDueToSpell(46981); // 46981
            DoCast(me, 37752); // 37752
            DoCast(me, 46402, true); // 46402
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void Submerge()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_31);
            DoCast(me, 37751, true); // 37751
            DoCast(me, 46416, true);
            DoCast(me, 46981, true); // 46981
            me->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!IntroDone)
            {
                if (IntroEvent <= diff)
                {
                    Emerge();
                    IntroDone = true;
                    me->SetVisible(true);
                }
                else IntroEvent -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BIGADD:
                    if (Phase == 1)
                        DoCast(46176);

                    events.RescheduleEvent(EVENT_BIGADD, 20000);
                    break;
                case EVENT_SMALLADD:
                    if (Phase == 1)
                    {
                        DoCast(46143);
                        DoCast(46143);
                    }
                    events.RescheduleEvent(EVENT_SMALLADD, 8000);
                    break;
                case EVENT_CRYSTAL:
                    me->SummonCreature(25865, -99.56f, -229.85f, 0.88f, 0, TEMPSUMMON_DEAD_DESPAWN);
                    events.RescheduleEvent(EVENT_RESET, 30000);
                    events.RescheduleEvent(EVENT_CRYSTAL, 120000);
                    Submerge();
                    Phase = 2;
                    break;
                case EVENT_RESET:
                    events.Reset();
                    events.RescheduleEvent(EVENT_BIGADD, 5000);
                    events.RescheduleEvent(EVENT_SMALLADD, 8000);
                    events.RescheduleEvent(EVENT_CRYSTAL, 90000);
                    summons.DespawnEntry(25865);
                    Emerge();
                    Phase = 1;
                    break;
                case EVENT_ICESPEAR:
                    if (Phase == 1)
                        if (!me->HasAura(46371))
                            DoCast(46371);

                    events.RescheduleEvent(EVENT_ICESPEAR, 10000);
                    break;

                }
            }
            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_ahune_frost_lordAI (creature);
    }
};
// 25865
class npc_ahune_frozen_core : public CreatureScript
{
public:
    npc_ahune_frozen_core() : CreatureScript("npc_ahune_frozen_core") { }

    struct npc_ahune_frozen_coreAI : public ScriptedAI
    {
        npc_ahune_frozen_coreAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void JustDied(Unit* /*killer*/) override
        {
            Map::PlayerList const& players = me->GetMap()->GetPlayers();
            if (!players.isEmpty())
            {
                Player* pPlayer = players.begin()->getSource();
                if (pPlayer && pPlayer->GetGroup())
                    sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 286);
            }

            if (Creature* ahune = instance->instance->GetCreature(instance->GetGuidData(NPC_AHUNE)))
                me->Kill(ahune);
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker == me)
                return;
            
            if (Creature* ahune = instance->instance->GetCreature(instance->GetGuidData(NPC_AHUNE)))
            {
                if (damage >= me->GetHealth())
                    ahune->Kill(ahune, true);
                else
                    ahune->DealDamage(ahune, damage);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ahune_frozen_coreAI(creature);
    }
};

// 187882
class go_ahune_ice_stone : public GameObjectScript
{
public:
    go_ahune_ice_stone() : GameObjectScript("go_ahune_ice_stone") { }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 /*action*/)
    {
        InstanceScript* instance = go->GetInstanceScript();
        if (!instance)
            return false;

        go->SetGoState(GO_STATE_ACTIVE);
        go->SummonCreature(NPC_AHUNE, -99.1021f, -233.7526f, -1.22307f, 1.588250f, TEMPSUMMON_DEAD_DESPAWN);
        go->SummonCreature(NPC_SNOWBUNNY, -99.1021f, -233.7526f, -1.22307f, 1.588250f, TEMPSUMMON_DEAD_DESPAWN);
        go->SetVisible(false);

        if (Creature* ahune = instance->instance->GetCreature(instance->GetGuidData(NPC_AHUNE)))
        {
            ahune->AI()->DoAction(ACTION_AHUNE);
        }
        if (Creature* luma = instance->instance->GetCreature(instance->GetGuidData(NPC_LUMA)))
            luma->CastSpell(player, 45926, true);

        //go->Delete();

        return true;
    }
};

// 46371 - Ice Spear Control Aura
class spell_ice_spear_control_aura : public SpellScriptLoader
{
public:
    spell_ice_spear_control_aura() : SpellScriptLoader("spell_ice_spear_control_aura") { }

    class spell_ice_spear_control_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ice_spear_control_aura_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ 46372 });
        }

        void PeriodicTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
                caster->CastSpell(caster, 46372);
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_ice_spear_control_aura_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_ice_spear_control_aura_AuraScript();
    }
};

// 46372 - Ice Spear Target Picker
class spell_ice_spear_target_picker : public SpellScriptLoader
{
public:
    spell_ice_spear_target_picker() : SpellScriptLoader("spell_ice_spear_target_picker") { }

    class spell_ice_spear_target_picker_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ice_spear_target_picker_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ 46359 });
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
            targets.clear();
            targets.push_back(target);
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            GetCaster()->CastSpell(GetHitUnit(), 46359, true);
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ice_spear_target_picker_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_ice_spear_target_picker_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_ice_spear_target_picker_SpellScript();
    }
};

// 45926 - Summoning Rhyme Aura
class spell_summoning_rhyme_aura : public SpellScriptLoader
{
public:
    spell_summoning_rhyme_aura() : SpellScriptLoader("spell_summoning_rhyme_aura") { }

    class spell_summoning_rhyme_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_summoning_rhyme_aura_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ 45930 });
        }

        ObjectGuid LumaGUID;

        void PeriodicTick(AuraEffect const* aurEff)
        {
            Creature* caster = GetCaster()->ToCreature();
            Player* player = GetTarget()->ToPlayer();

            if (!caster || !player)
                return;

            switch (aurEff->GetTickNumber())
            {
            case 1:
                sCreatureTextMgr->SendChat(caster, SAY_PLAYER_TEXT_1, LumaGUID, CHAT_MSG_SAY, LANG_UNIVERSAL, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                player->CastSpell(player, 45930, true);
                break;
            case 2:
                sCreatureTextMgr->SendChat(caster, SAY_PLAYER_TEXT_2, LumaGUID, CHAT_MSG_SAY, LANG_UNIVERSAL, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                break;
            case 3:
                sCreatureTextMgr->SendChat(caster, SAY_PLAYER_TEXT_3, LumaGUID, CHAT_MSG_SAY, LANG_UNIVERSAL, TEXT_RANGE_NORMAL, 0, TEAM_OTHER, false, player);
                Remove();
                break;
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_summoning_rhyme_aura_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_summoning_rhyme_aura_AuraScript();
    }
};

void AddSC_boss_ahune_frost_lord()
{
    new boss_ahune_frost_lord();
    new npc_ahune_frozen_core();
    new go_ahune_ice_stone();
    new spell_ice_spear_target_picker();
    new spell_ice_spear_control_aura();
    new spell_summoning_rhyme_aura();
}
