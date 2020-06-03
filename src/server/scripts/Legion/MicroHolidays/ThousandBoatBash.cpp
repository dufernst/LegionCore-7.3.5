/*
*/

#include "OutdoorPvP.h"
#include "QuestData.h"

class OutdoorPvPThousandNeedles : public OutdoorPvP
{
public:
    OutdoorPvPThousandNeedles()
    {
        m_TypeId = OUTDOOR_PVP_THOUSAND_NEEDLES;
    }

    ~OutdoorPvPThousandNeedles() = default;

    bool SetupOutdoorPvP() override
    {
        RegisterZone(400);
        return true;
    }

    bool Update(uint32 diff) override
    {
        if (ch_water)
        {
            if (ch_water <= diff)
            {
                ApplyOnEveryPlayerInZone([this](Player* player) -> void
                {
                    ch_water = 2000;
                    Zliquid_status = player->GetMap()->getLiquidStatus(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), MAP_ALL_LIQUIDS, &liquid_status);

                    if (Zliquid_status & (LIQUID_MAP_WATER_WALK | LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER))
                    {
                        if (!player->HasAura(234458) && !player->HasAura(234449))
                        {
                            if (auto entry = sBroadcastTextStore.LookupEntry(126573))
                            {
                                std::string text = DB2Manager::GetBroadcastTextValue(entry, player->GetSession()->GetSessionDbLocaleIndex());

                                player->BossWhisper(text, LANG_UNIVERSAL, player->GetGUID());
                                player->CastSpell(player, 234458, true);
                            }
                        }
                    }             
                });
            }
            else
                ch_water -= diff;
        }

        return true;
    }
    void HandleGameEventStart(uint32 eventId) override
    {
        if (eventId != 307)
            return;

        if (!m_map)
            return;

        ch_water = 2000;
    }

private:
    uint8 m_stage{};
    uint32 ch_water{};
    LiquidData liquid_status;
    ZLiquidStatus Zliquid_status;
};

class OutdoorPvP_ThousandNeedles : public OutdoorPvPScript
{
public:
    OutdoorPvP_ThousandNeedles() : OutdoorPvPScript("outdoorpvp_thousandneedles") {}

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPvPThousandNeedles();
    }
};

class sceneTrigger_beach_ball : public SceneTriggerScript
{
public:
    sceneTrigger_beach_ball() : SceneTriggerScript("sceneTrigger_beach_ball") {}

    bool OnTrigger(Player* player, SpellScene const* trigger, std::string type) override
    {
        if (type == "credit")
            player->CastSpell(player, 234524, false);

        return true;
    }
};

//234449
class spell_bating : public AuraScript
{
    PrepareAuraScript(spell_bating);

    void OnProc(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        LiquidData liquid_status;
        ZLiquidStatus Zliquid_status = caster->GetMap()->getLiquidStatus(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), MAP_ALL_LIQUIDS, &liquid_status);

        if (Zliquid_status & (LIQUID_MAP_WATER_WALK | LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER))
            caster->CastSpell(caster, 234450, true);
        else
            caster->RemoveAura(234449);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAura(234450);
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAura(234458);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_bating::OnProc, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_bating::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectApply += AuraEffectApplyFn(spell_bating::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

//234458
class spell_boat_day : public AuraScript
{
    PrepareAuraScript(spell_boat_day);

    void OnProc(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        LiquidData liquid_status;
        ZLiquidStatus Zliquid_status = caster->GetMap()->getLiquidStatus(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), MAP_ALL_LIQUIDS, &liquid_status);

        if (Zliquid_status & (LIQUID_MAP_WATER_WALK | LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER))
            caster->CastSpell(caster, 234460, true);
        else
            caster->RemoveAura(234458);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAura(234460);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_boat_day::OnProc, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_boat_day::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

//114996
struct npc_fizzle_brassbolts : ScriptedAI
{
    explicit npc_fizzle_brassbolts(Creature* creature) : ScriptedAI(creature) {}

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 15.0f)
            return;

        if (!sGameEventMgr->IsActiveEvent(307))
            return;

        if (Player* player = who->ToPlayer())
        {
            if (WorldQuest const* wq = sQuestDataStore->GetWorldQuest(sQuestDataStore->GetQuestTemplate(45806)))
            {
                if (!player->WorldQuestCompleted(45806))
                {
                    uint32 credit = player->GetQuestObjectiveData(45806, 118140);

                    if (credit != 1)
                    {
                        player->KilledMonsterCredit(118140);
                        player->CreateConversation(4349);
                    }
                }
            }
        }
    }
};

//118157
struct npc_tu_luak : ScriptedAI
{
    explicit npc_tu_luak(Creature* creature) : ScriptedAI(creature) {}

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 15.0f)
            return;

        if (!sGameEventMgr->IsActiveEvent(307))
            return;

        if (Player* player = who->ToPlayer())
        {
            if (WorldQuest const* wq = sQuestDataStore->GetWorldQuest(sQuestDataStore->GetQuestTemplate(45806)))
            {
                if (!player->WorldQuestCompleted(45806))
                {
                    uint32 credit = player->GetQuestObjectiveData(45806, 118141);

                    if (credit != 1)
                    {
                        player->KilledMonsterCredit(118141);
                        player->CreateConversation(4352);
                    }
                }
            }
        }
    }
};

//118146
struct npc_theldurin_the_lost : ScriptedAI
{
    explicit npc_theldurin_the_lost(Creature* creature) : ScriptedAI(creature) {}

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 15.0f)
            return;

        if (!sGameEventMgr->IsActiveEvent(307))
            return;

        if (Player* player = who->ToPlayer())
        {
            if (WorldQuest const* wq = sQuestDataStore->GetWorldQuest(sQuestDataStore->GetQuestTemplate(45806)))
            {
                if (!player->WorldQuestCompleted(45806))
                {
                    uint32 credit = player->GetQuestObjectiveData(45806, 118143);

                    if (credit != 1)
                    {
                        player->KilledMonsterCredit(118143);
                        player->CreateConversation(4351);
                    }
                }
            }
        }
    }
};

//118145
struct npc_ilthine_sunsong : ScriptedAI
{
    explicit npc_ilthine_sunsong(Creature* creature) : ScriptedAI(creature) {}

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 30.0f)
            return;

        if (!sGameEventMgr->IsActiveEvent(307))
            return;

        if (Player* player = who->ToPlayer())
        {
            if (WorldQuest const* wq = sQuestDataStore->GetWorldQuest(sQuestDataStore->GetQuestTemplate(45806)))
            {
                if (!player->WorldQuestCompleted(45806))
                {
                    uint32 credit = player->GetQuestObjectiveData(45806, 118144);

                    if (credit != 1)
                    {
                        player->KilledMonsterCredit(118144);
                        player->CreateConversation(4350);
                    }
                }
            }
        }
    }
};

void AddSC_ThousandBoatBash()
{
    new sceneTrigger_beach_ball();
    //new OutdoorPvP_ThousandNeedles();
    RegisterAuraScript(spell_bating);
    RegisterAuraScript(spell_boat_day);
    RegisterCreatureAI(npc_fizzle_brassbolts);
    RegisterCreatureAI(npc_tu_luak);
    RegisterCreatureAI(npc_theldurin_the_lost);
    RegisterCreatureAI(npc_ilthine_sunsong);
}