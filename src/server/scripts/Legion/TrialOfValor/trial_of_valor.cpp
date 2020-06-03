#include "trial_of_valor.h"
#include "QuestData.h"

enum Conversations
{
    CONV_INTRO = 4180
};

enum Spells
{
    SpellOdynTeleport = 232580,
    SpellOdynTeleportTrigger = 232581
};

struct npc_odyn_after_kill_helya : ScriptedAI
{
    explicit npc_odyn_after_kill_helya(Creature* creature) : ScriptedAI(creature)
    {
        DoCast(SpellOdynTeleport);
    }

    void SpellHit(Unit* /*who*/, const SpellInfo* spellInfo)
    {
        if (spellInfo->Id == SpellOdynTeleportTrigger)
            me->CreateConversation(CONV_INTRO);
    }

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        uint32 graveyardId = 0;

        if (action == 1)
            graveyardId = 5779;

        if (graveyardId)
            if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(graveyardId))
                player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);

        player->CLOSE_GOSSIP_MENU();
    }
};

class eventobject_teleport_helheim_target : public EventObjectScript
{
public:
    eventobject_teleport_helheim_target() : EventObjectScript("eventobject_teleport_helheim_target") {}

    bool OnTrigger(Player* player, EventObject* eo, bool enter) override
    {
        if (!enter)
            return true;

        uint32 graveyardId = 0;

        InstanceScript* instance = eo->GetInstanceScript();
        if (!instance)
            return true;
        if (player->isInCombat())
            return true;

        if (graveyardId = 5827)
            if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(graveyardId))
                player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);

        return true;
    }
};

class eventobject_teleport_helheim_exit : public EventObjectScript
{
public:
    eventobject_teleport_helheim_exit() : EventObjectScript("eventobject_teleport_helheim_exit") {}

    bool OnTrigger(Player* player, EventObject* eo, bool enter) override
    {
        if (!enter)
            return true;

        uint32 graveyardId = 0;
        InstanceScript* instance = eo->GetInstanceScript();
        if (!instance)
            return true;
        if (player->isInCombat())
            return true;

        if (graveyardId = 5828)
            if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(graveyardId))
                player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);

        return true;
    }
};

//228891
class spell_frigid_spray : public SpellScript
{
    PrepareSpellScript(spell_frigid_spray);

    uint8 targetsCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targetsCount = targets.size();
    }

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (targetsCount)
            SetHitDamage(GetHitDamage() / targetsCount);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_frigid_spray::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_54);
        OnEffectHitTarget += SpellEffectFn(spell_frigid_spray::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_vantus_rune_trial_of_valor : public SpellScriptLoader
{
public:
    spell_vantus_rune_trial_of_valor() : SpellScriptLoader("spell_vantus_rune_trial_of_valor") {}

    class spell_vantus_rune_trial_of_valor_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vantus_rune_trial_of_valor_AuraScript);

        uint16 checkOnProc;
        uint16 checkOnRemove;

        bool Load()
        {
            checkOnProc = 1000;
            checkOnRemove = 1000;
            return true;
        }

        void OnUpdate(uint32 diff, AuraEffect* aurEff)
        {
            Unit* player = GetCaster();
            if (!player)
                return;

            InstanceScript* instance = player->GetInstanceScript();
            if (!instance)
                return;

            if (checkOnProc <= diff)
            {
                switch (aurEff->GetSpellInfo()->Id)
                {
                case 229175:
                    if (instance->GetBossState(Data::BossIDs::GarmID) == IN_PROGRESS && !player->HasAura(229187))
                        player->CastSpell(player, 229187, false);
                    break;
                case 229174:
                    if (instance->GetBossState(Data::BossIDs::OdynID) == IN_PROGRESS && !player->HasAura(229186))
                        player->CastSpell(player, 229186, false);
                    break;
                case 229176:
                    if (instance->GetBossState(Data::BossIDs::HelyaID) == IN_PROGRESS && !player->HasAura(229188))
                        player->CastSpell(player, 229188, false);
                    break;
                }
            }
            else
                checkOnProc -= diff;

            if (checkOnRemove <= diff)
            {
                if (player->HasAura(229187))
                {
                    if (instance->GetBossState(Data::BossIDs::GarmID) == DONE || instance->GetBossState(Data::BossIDs::GarmID) == NOT_STARTED)
                    {
                        player->RemoveAura(229187);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(229186))
                {
                    if (instance->GetBossState(Data::BossIDs::OdynID) == DONE || instance->GetBossState(Data::BossIDs::OdynID) == NOT_STARTED)
                    {
                        player->RemoveAura(229186);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(229188))
                {
                    if (instance->GetBossState(Data::BossIDs::HelyaID) == DONE || instance->GetBossState(Data::BossIDs::HelyaID) == NOT_STARTED)
                    {
                        player->RemoveAura(229188);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }
            }
            else
                checkOnRemove -= diff;
        }

        void Register() override
        {
            OnEffectUpdate += AuraEffectUpdateFn(spell_vantus_rune_trial_of_valor_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_vantus_rune_trial_of_valor_AuraScript();
    }

    class spell_vantus_rune_trial_of_valor_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_vantus_rune_trial_of_valor_SpellScript);

        SpellCastResult CheckCast()
        {
            if (auto player = GetCaster()->ToPlayer())
                if (!player->GetQuestRewardStatus(39695))
                    return SPELL_CAST_OK;

            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_YOU_ALREADY_USED_VANTUS_RUNE);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        void HandleOnHit()
        {
            if (auto player = GetCaster()->ToPlayer())
                if (Quest const* quest = sQuestDataStore->GetQuestTemplate(39695))
                    if (player->CanTakeQuest(quest, false))
                        player->CompleteQuest(39695);
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_vantus_rune_trial_of_valor_SpellScript::CheckCast);
            OnHit += SpellHitFn(spell_vantus_rune_trial_of_valor_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_vantus_rune_trial_of_valor_SpellScript();
    }
};

void AddSC_trial_of_valor()
{
    RegisterCreatureAI(npc_odyn_after_kill_helya);
    RegisterSpellScript(spell_frigid_spray);
    new spell_vantus_rune_trial_of_valor();
    new eventobject_teleport_helheim_target();
    new eventobject_teleport_helheim_exit();
}
