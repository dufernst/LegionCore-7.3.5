#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Vehicle.h"

// 49337
struct npc_darnel_q26800 : public ScriptedAI
{
    npc_darnel_q26800(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        Talk(0);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            for (uint8 i = 1; i < 4; ++i)
            {
                if (auto pass = me->GetVehicleKit()->GetPassenger(i))
                    pass->ExitVehicle();
            }
        }
    }

    void PassengerBoarded(Unit* who, int8 seatId, bool apply) override
    {
        if (!me->GetVehicle())
            return;

        if (!apply && who->ToCreature())
        {
            who->ToCreature()->DespawnOrUnsummon(2000);
        }
    }
};

// 49340
struct npc_scarlet_corpse_q26800 : public ScriptedAI
{
    npc_scarlet_corpse_q26800(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool used = false;

    void Reset() override
    {
        used = false;
    }

    void IsSummonedBy(Unit* summoner) override
    {
        used = true;
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        me->RemoveAura(92230);
    }

    void OnSpellClick(Unit* who) override
    {
        if (!used)
        {
            used = true;
            auto cList = who->GetSummonList(49337);
            for (GuidList::const_iterator iter = cList->begin(); iter != cList->end(); ++iter)
            {
                if (auto darnel = ObjectAccessor::GetCreature(*who, (*iter)))
                {
                    if (auto sum = me->SummonCreature(49340, me->GetPosition()))
                    {
                        if (auto plr = who->ToPlayer())
                        {
                            plr->KilledMonsterCredit(49340);
                            sum->CastSpell(darnel, 46598, true);
                            me->DespawnOrUnsummon(100);
                        }
                    }
                }
            }
        }
    }
};

// 1740
struct npc_deathguard_saltain : public ScriptedAI
{
    npc_deathguard_saltain(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void OnQuestReward(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == 26800)
        {
            auto cList = player->GetSummonList(49337);
            for (GuidList::const_iterator iter = cList->begin(); iter != cList->end(); ++iter)
            {
                if (auto darnel = ObjectAccessor::GetCreature(*player, (*iter)))
                {
                    if (darnel->IsAIEnabled)
                        darnel->AI()->DoAction(ACTION_1);
                }
            }
        }
    }

};



// 91938
class spell_summon_darnel_q26800 : public AuraScript
{
    PrepareAuraScript(spell_summon_darnel_q26800);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto caster = GetCaster())
        {
            auto cList = caster->GetSummonList(49337);
            for (GuidList::const_iterator iter = cList->begin(); iter != cList->end(); ++iter)
            {
                if (auto darnel = ObjectAccessor::GetCreature(*caster, (*iter)))
                    darnel->DespawnOrUnsummon(100);
            }
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_summon_darnel_q26800::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_tirisfal_glades()
{
    RegisterCreatureAI(npc_darnel_q26800);
    RegisterCreatureAI(npc_scarlet_corpse_q26800);
    RegisterCreatureAI(npc_deathguard_saltain);
    RegisterAuraScript(spell_summon_darnel_q26800);
}
