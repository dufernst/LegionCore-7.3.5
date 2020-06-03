#include "antorus.h"
#include "QuestData.h"

// 127233
struct npc_atbt_flamewear : public ScriptedAI
{
    npc_atbt_flamewear(Creature* cre) : ScriptedAI(cre) {}

    EventMap events;
    std::vector<ObjectGuid> targs{};

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* who) override
    {
        events.RescheduleEvent(EVENT_1, 1500);
        events.RescheduleEvent(EVENT_2, 10000);
    }

    void SetGUID(const ObjectGuid& guid, int32 type) override
    {
        targs.push_back(guid);
        if (targs.size() != 2)
            return;
        for (bool x : {false, true})
            if (Player * player = ObjectAccessor::GetPlayer(*me, targs[x]))
                if (Aura* aura = player->GetAura(252621))
                    aura->SetScriptGuid(0, targs[!x]);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(252614);
                events.RescheduleEvent(EVENT_1, 3500);
                break;
            case EVENT_2:
                targs.clear();
                DoCast(252621);
                events.RescheduleEvent(EVENT_2, urand(1000, 14000));
                break;
            }
        }

        DoMeleeAttackIfReady();
    }

};

// 125819
struct npc_atbt_battleship_controller : public ScriptedAI
{
    npc_atbt_battleship_controller(Creature* cre) : ScriptedAI(cre)
    {
        SetCanSeeEvenInPassiveMode(true);
    }

    EventMap events;
    bool start = false;

    void Reset() override
    {
        events.Reset();
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (start)
            return;

        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 98.0f)
            return;

        start = true;

        events.RescheduleEvent(EVENT_1, 3000);

        if (Creature* cre = me->FindNearestCreature(125480, 90.0, true)) // really need it =C
            cre->AI()->Talk(0);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!start)
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                me->SummonCreature(NPC_BATTLE_SHIP, -3268.71f, 10120.37f, -67.19f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000);
                events.RescheduleEvent(EVENT_1, urand(3000, 5000));
                break;
            }
        }
    }

};

// 125771
struct npc_atbt_battleship : public ScriptedAI
{
    npc_atbt_battleship(Creature* cre) : ScriptedAI(cre) {}

    void IsSummonedBy(Unit* owner) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetCanFly(true);
        me->GetMotionMaster()->MovePoint(0, -3312.39f, 9992.18f, -87.09f); 

        me->AddDelayedEvent(urand(2000, 3000), [this]() -> void
        {
            uint8 i = 0;
            for (float y = 10099.79f; y >= 9980.05f; y -= frand(2, 7))
            {
                me->AddDelayedEvent(100 * ++i, [this, y]() -> void
                {
                    float x = frand(-3305.59f, -3286.11f);
                    me->CastSpell(x, y, -115.73f, 249933, true);
                });
            }
        });
    }
};

// 128289
struct npt_atbt_teleport : public ScriptedAI
{
    npt_atbt_teleport(Creature* cre) : ScriptedAI(cre) {}

    void OnSpellClick(Unit* who) override
    {
        if (!who)
            return;

        if (who->IsMounted())
        {
            who->Dismount();
            return;
        }

        who->CastSpell(who, 253773, true);
        who->GetMotionMaster()->MoveIdle();
        who->GetMotionMaster()->MovePath(me->GetPositionX() < -3100.0f ? 12871196 : 12871197, false, frand(-2, 2), frand(-2, 2));
    }
};


//125720, 128303 128304
class npt_atbt_tele_gates : public CreatureScript
{
public:
    npt_atbt_tele_gates() : CreatureScript("npt_atbt_tele_gates") {}

    bool OnGossipHello(Player* player, Creature* me)  override
    {
        InstanceScript* instance = me->GetInstanceScript();
        if (!instance)
            return false;

        bool isDone = true;
        for (uint8 i = DATA_WORLDBREAKER; i <= DATA_EONAR; ++i)
            if (instance->GetBossState(i) != DONE)
                isDone = false;

        if (isDone && me->GetEntry() != 128303) 
            player->ADD_GOSSIP_ITEM_DB(21377, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // imonar

        if (me->GetEntry() != 125720) 
            player->ADD_GOSSIP_ITEM_DB(21377, 3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2); // start

        isDone = true;
        for (uint8 i = DATA_IMONAR; i <= DATA_KINGAROTH; ++i)
            if (instance->GetBossState(i) != DONE)
                isDone = false;

        if (me->GetEntry() != 128304)
            player->ADD_GOSSIP_ITEM_DB(21377, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3); // Varimatras

        player->SEND_GOSSIP_MENU(32550, me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* me, uint32 sender, uint32 action)  override
    {
        if (me->GetEntry() == 125720)
            player->CastSpell(player, 254498);
        else if (me->GetEntry() == 128303)
            player->CastSpell(player, 254499);

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->NearTeleportTo(-10574.47f, 8772.07f, 1871.46f, 4.68f);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->NearTeleportTo(-3437.5f, 10156.9f, -150.022f, 0.0f);
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->NearTeleportTo(-12633.91f, -3369.47f, 2513.82f, 0.0f);
            break;
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

// 128169
class npc_atbt_muradin : public CreatureScript
{
public:
    npc_atbt_muradin() : CreatureScript("npc_atbt_muradin") {}

    bool OnGossipSelect(Player* player, Creature* me, uint32 sender, uint32 action)  override
    {
        player->CastSpell(player, 254311);
        player->NearTeleportTo(2825.47f, -4567.23f, 291.94f, 0.0f);
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

// 252621
class spell_atbt_bound_by_fel : public AuraScript
{
    PrepareAuraScript(spell_atbt_bound_by_fel);

    ObjectGuid second{};

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (target->GetAuraCount(252621) >= 20)
        {
            target->RemoveAurasDueToSpell(252621);
            return;
        }

        if(second.IsEmpty())
            return;

        Unit* second_targ = ObjectAccessor::GetPlayer(*target, second);
        if (!second_targ)
            return;

        target->CastSpell(second_targ, 252622);
        if (target->GetDistance(second_targ) <= 10)
            target->CastSpell(second_targ, 252623);
        else if (Unit* caster = GetCaster())
            if (caster->isAlive())
                caster->CastSpell(target, 252621);
    }

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        if (!GetCaster() || !GetTarget())
            return;

        Creature* cre = GetCaster()->ToCreature();
        if (!cre)
            return;

        Unit* target = GetTarget();
        cre->AI()->SetGUID(target->GetGUID());
    }

    void SetGuid(uint32 type, ObjectGuid const& guid) override
    {
        second = guid;
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_atbt_bound_by_fel::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectApply += AuraEffectApplyFn(spell_atbt_bound_by_fel::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 253600
class spell_atbt_soulburn : public AuraScript
{
    PrepareAuraScript(spell_atbt_soulburn);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        target->CastSpell(target, 253601);

        std::list<Player*> players;
        target->GetPlayerListInGrid(players, 8.0f);
        players.remove_if([](Player* player)
        {
            if (!player) 
                return true;
        
            return !player->HasAura(253600);
        });
        Trinity::Containers::RandomResizeList(players, 5);
        for (Player* player : players)
            player->CastSpell(player, 253600);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_atbt_soulburn::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 252740
class spell_atbt_anihilation : public SpellScript
{
    PrepareSpellScript(spell_atbt_anihilation);

    bool check = false;

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        
        for (uint8 i = 0; i < 3; ++i)
        {
            Position pos(caster->GetPosition());
            caster->GetFirstCollisionPosition(pos, 15.0f * frand(0.3f, 1.0f), static_cast<float>(rand_norm()) * static_cast<float>(2.0f * M_PI));
            caster->CastSpell(pos, 252742, false);
            caster->CastSpell(pos, 252741, false);
        }
    }

    void Register()
    {
        AfterCast += SpellCastFn(spell_atbt_anihilation::HandleAfterCast);
    }
};

//800-812, 815-816
class eventobject_antorus_into : public EventObjectScript
{
public:
    eventobject_antorus_into() : EventObjectScript("eventobject_antorus_into") {}

    bool eventDone = false;

    bool OnTrigger(Player* player, EventObject* eo, bool enter) override
    {
        if (!enter)
            return true;

        if (eventDone)
            return true;

        if (eo->GetEntry() != 805 && eo->GetEntry() != 815 && eo->GetEntry() != 816)
            eventDone = true;

        InstanceScript* instance = player->GetInstanceScript();
        if (!instance)
            return false;

        switch (eo->GetEntry())
        {
        case 800:
            player->CreateConversation(5545);
            player->AddDelayedEvent(7000, [player]() -> void
            {
                player->CreateConversation(5522);
            });
            break;
        case 801:
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CreateConversation(5524);
            });
            break;
        case 802:
            player->AddDelayedEvent(8000, [player, instance]() -> void
            {
                instance->instance->LoadGrid(-3705.06f, 1357.21f);
                if (Creature* cre = instance->instance->GetCreature(instance->GetGuidData(NPC_HASABEL)))
                    cre->AI()->ZoneTalk(0);
            });
            break;
        case 803:
            if (instance->GetBossState(DATA_HASABEL) != DONE)
            {
                eventDone = false;
                return false;
            }
            eo->SummonCreature(NPC_IMAGE_OF_EONAR, eo->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 21000);
            break;
        case 804:
            if (Creature* eonar = eo->SummonCreature(NPC_EONAR_EVENT, -4299.92f, -11179.68f, 817.63f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40000))
            {
                player->CreateConversation(5703);
                eonar->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);
            }
            break;
        case 805:
            if (!player->HasAura(SPELL_SURGE_OF_LIFE_OVERRIDE))
            {
                std::list<Creature*> mobs;
                player->GetCreatureListWithEntryInGrid(mobs, 127681, 30.0f);
                for (auto& npc : mobs)
                    npc->CastSpell(player, SPELL_BLESSING_LIFEBINDER_VISUAL, true);
            }
            break;
        case 806:
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CastSpell(player, 249441);
            });
            break;
        case 807:
            if (instance->GetBossState(DATA_IMONAR) == DONE)
                break;
            instance->instance->LoadGrid(-10599.29f, 9036.34f);
            player->CreateConversation(5608);

            if (Creature* cre = eo->SummonCreature(NPC_IMONAR_INTRO, -10574.15f, 8606.92f, 1909.52f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000))
            {
                instance->instance->LoadGrid(-10754.15f, 8606.92f);
                cre->GetMotionMaster()->MovePoint(0, -10572.75f, 8715.77f, 1891.68f);
                cre->AddDelayedEvent(9000, [cre]() -> void
                {
                    InstanceScript* script = cre->GetInstanceScript();
                    if (!script)
                        return;
                    
                    if (Creature* boss = script->instance->GetCreature(script->GetGuidData(NPC_IMONAR)))
                    {
                        boss->SetVisible(true);
                        boss->CastSpell(boss, 249569);
                        boss->AI()->Talk(0);
                    }

                    cre->AddDelayedEvent(3000, [cre]() -> void
                    {
                        cre->GetMotionMaster()->MovePoint(0, -10791.75f, 8883.12f, 2056.20f);
                    });
                });
            }
            break;
        case 808:
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CastSpell(player, 249802);
            });
            break;
        case 809:
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CastSpell(player, 250695);
            });
            instance->instance->LoadGrid(-12694.59f, -3597.12f);
            break;      
        case 810:
        {
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CastSpell(player, 250797);
            });
            uint32 const entries[3]{ 125683, 124913, 125682 };
            Position const pos[3]{
                {-12639.84f, -3029.82f, 2498.75f, 1.57f},
                {-12631.39f, -3029.82f, 2498.97f, 1.57f},
                {-12626.20f, -3029.82f, 2498.97f, 1.57f }
            };
            for (uint8 i = 0; i < 3; ++i)
                if (Creature* cre = eo->SummonCreature(entries[i], pos[i]))
                {
                    cre->SetReactState(REACT_ATTACK_OFF);
                    cre->AddDelayedEvent(1000, [cre]() -> void
                    {
                        cre->GetMotionMaster()->MovePoint(1, cre->GetPositionX(), -2947.84f + frand(-3.0f, 3.0f), 2499.12f);
                    });
                }

            break;
        }
        case 811:
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CastSpell(player, 250698);
            });
            break;
        case 812:
            instance->instance->LoadGrid(-12633.20f, -2231.10f);
            player->AddDelayedEvent(1000, [player]() -> void
            {
                player->CastSpell(player, 250700);
                player->AddDelayedEvent(50000, [player]() -> void
                {
                    player->CastSpell(player, 250728);
                });
            });
            break;
        //case 815:
        //    if (instance->GetBossState(DATA_WORLDBREAKER) == DONE && instance->GetBossState(DATA_FELHOUNDS) == DONE)
        //        player->TeleportTo(1712, -2891.32f, 10767.63f, 124.0f, player->GetOrientation());
        //    break;
        //case 816:
        //    if (instance->GetBossState(DATA_WORLDBREAKER) == DONE && instance->GetBossState(DATA_FELHOUNDS) == DONE)
        //        player->TeleportTo(1712, -2891.07f, 10768.03f, -90.78f, player->GetOrientation());
        //    break;
        }
        return true;
    }
};

class spell_vantus_rune_antorus : public SpellScriptLoader
{
public:
    spell_vantus_rune_antorus() : SpellScriptLoader("spell_vantus_rune_antorus") {}

    class spell_vantus_rune_antorus_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vantus_rune_antorus_AuraScript);

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
                case 250153:
                    if (instance->GetBossState(DATA_WORLDBREAKER) == IN_PROGRESS && !player->HasAura(250152))
                        player->CastSpell(player, 250152, false);
                    break;
                case 250156:
                    if (instance->GetBossState(DATA_FELHOUNDS) == IN_PROGRESS && !player->HasAura(250155))
                        player->CastSpell(player, 250155, false);
                    break;
                case 250167:
                    if (instance->GetBossState(DATA_ANTORAN) == IN_PROGRESS && !player->HasAura(250166))
                        player->CastSpell(player, 250166, false);
                    break;
                case 250160:
                    if (instance->GetBossState(DATA_HASABEL) == IN_PROGRESS && !player->HasAura(250159))
                        player->CastSpell(player, 250159, false);
                    break;
                case 250150:
                    if (instance->GetBossState(DATA_EONAR) == IN_PROGRESS && !player->HasAura(250149))
                        player->CastSpell(player, 250149, false);
                    break;
                case 250158:
                    if (instance->GetBossState(DATA_IMONAR) == IN_PROGRESS && !player->HasAura(250157))
                        player->CastSpell(player, 250157, false);
                    break;
                case 250148:
                    if (instance->GetBossState(DATA_KINGAROTH) == IN_PROGRESS && !player->HasAura(250147))
                        player->CastSpell(player, 250147, false);
                    break;
                case 250165:
                    if (instance->GetBossState(DATA_VARIMATHRAS) == IN_PROGRESS && !player->HasAura(250164))
                        player->CastSpell(player, 250164, false);
                    break;
                case 250163:
                    if (instance->GetBossState(DATA_COVEN) == IN_PROGRESS && !player->HasAura(250162))
                        player->CastSpell(player, 250162, false);
                    break;
                case 250144:
                    if (instance->GetBossState(DATA_AGGRAMAR) == IN_PROGRESS && !player->HasAura(250143))
                        player->CastSpell(player, 250143, false);
                    break;
                case 250146:
                    if (instance->GetBossState(DATA_ARGUS) == IN_PROGRESS && !player->HasAura(250145))
                        player->CastSpell(player, 250145, false);
                    break;
                }
            }
            else
                checkOnProc -= diff;

            if (checkOnRemove <= diff)
            {
                if (player->HasAura(250152))
                {
                    if (instance->GetBossState(DATA_WORLDBREAKER) == DONE || instance->GetBossState(DATA_WORLDBREAKER) == NOT_STARTED)
                    {
                        player->RemoveAura(250152);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250155))
                {
                    if (instance->GetBossState(DATA_FELHOUNDS) == DONE || instance->GetBossState(DATA_FELHOUNDS) == NOT_STARTED)
                    {
                        player->RemoveAura(250155);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250166))
                {
                    if (instance->GetBossState(DATA_ANTORAN) == DONE || instance->GetBossState(DATA_ANTORAN) == NOT_STARTED)
                    {
                        player->RemoveAura(250166);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250159))
                {
                    if (instance->GetBossState(DATA_HASABEL) == DONE || instance->GetBossState(DATA_HASABEL) == NOT_STARTED)
                    {
                        player->RemoveAura(250159);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250149))
                {
                    if (instance->GetBossState(DATA_EONAR) == DONE || instance->GetBossState(DATA_EONAR) == NOT_STARTED)
                    {
                        player->RemoveAura(250149);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250157))
                {
                    if (instance->GetBossState(DATA_IMONAR) == DONE || instance->GetBossState(DATA_IMONAR) == NOT_STARTED)
                    {
                        player->RemoveAura(250157);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250147))
                {
                    if (instance->GetBossState(DATA_KINGAROTH) == DONE || instance->GetBossState(DATA_KINGAROTH) == NOT_STARTED)
                    {
                        player->RemoveAura(250147);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250164))
                {
                    if (instance->GetBossState(DATA_VARIMATHRAS) == DONE || instance->GetBossState(DATA_VARIMATHRAS) == NOT_STARTED)
                    {
                        player->RemoveAura(250164);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250162))
                {
                    if (instance->GetBossState(DATA_COVEN) == DONE || instance->GetBossState(DATA_COVEN) == NOT_STARTED)
                    {
                        player->RemoveAura(250162);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250143))
                {
                    if (instance->GetBossState(DATA_AGGRAMAR) == DONE || instance->GetBossState(DATA_AGGRAMAR) == NOT_STARTED)
                    {
                        player->RemoveAura(250143);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(250145))
                {
                    if (instance->GetBossState(DATA_ARGUS) == DONE || instance->GetBossState(DATA_ARGUS) == NOT_STARTED)
                    {
                        player->RemoveAura(250145);
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
            OnEffectUpdate += AuraEffectUpdateFn(spell_vantus_rune_antorus_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_vantus_rune_antorus_AuraScript();
    }

    class spell_vantus_rune_antorus_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_vantus_rune_antorus_SpellScript);

        SpellCastResult CheckCast()
        {
            if (auto player = GetCaster()->ToPlayer())
                if (!player->GetQuestRewardStatus(39695))
                    return SPELL_CAST_OK;

            SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_YOU_ALREADY_USED_VANTUS_RUNE);
            return SPELL_FAILED_CUSTOM_ERROR;
        }

        void HandleOnCast()
        {
            if (auto player = GetCaster()->ToPlayer())
                if (Quest const* quest = sQuestDataStore->GetQuestTemplate(39695))
                    if (player->CanTakeQuest(quest, false))
                        player->CompleteQuest(39695);
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_vantus_rune_antorus_SpellScript::CheckCast);
            OnCast += SpellCastFn(spell_vantus_rune_antorus_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_vantus_rune_antorus_SpellScript();
    }
};

class spell_vantus_rune_antorus_choose : public SpellScript
{
    PrepareSpellScript(spell_vantus_rune_antorus_choose);

    SpellCastResult CheckCast()
    {
        if (auto player = GetCaster()->ToPlayer())
        {
            if (player->GetMapId() != 1712)
                return SPELL_FAILED_NOT_HERE;

            if (player->GetQuestRewardStatus(39695))
            {
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_YOU_ALREADY_USED_VANTUS_RUNE);
                return SPELL_FAILED_CUSTOM_ERROR;
            }
        }

        return SPELL_CAST_OK;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* boss = GetHitUnit();
        if (!boss)
            return;

        switch (boss->GetEntry())
        {
        case 122450:
            if (!caster->HasAura(250153))
                caster->CastSpell(caster, 250153, true);
            break;
        case 122477:
        case 122135:
            if (!caster->HasAura(250156))
                caster->CastSpell(caster, 250156, true);
            break;
        case 122367:
            if (!caster->HasAura(250167))
                caster->CastSpell(caster, 250167, true);
            break;
        case 122104:
            if (!caster->HasAura(250160))
                caster->CastSpell(caster, 250160, true);
            break;
        case 122500:
            if (!caster->HasAura(250150))
                caster->CastSpell(caster, 250150, true);
            break;
        case 124158:
            if (!caster->HasAura(250158))
                caster->CastSpell(caster, 250158, true);
            break;
        case 122578:
            if (!caster->HasAura(250148))
                caster->CastSpell(caster, 250148, true);
            break;
        case 122366:
            if (!caster->HasAura(250165))
                caster->CastSpell(caster, 250165, true);
            break;
        case 122468:
        case 122467:
        case 122469:
            if (!caster->HasAura(250163))
                caster->CastSpell(caster, 250163, true);
            break;
        case 124691:
            if (!caster->HasAura(250144))
                caster->CastSpell(caster, 250144, true);
            break;
        case 124828:
            if (!caster->HasAura(250146))
                caster->CastSpell(caster, 250146, true);
            break;
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_vantus_rune_antorus_choose::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnCheckCast += SpellCheckCastFn(spell_vantus_rune_antorus_choose::CheckCast);
    }
};

void AddSC_antorus()
{
    RegisterCreatureAI(npc_atbt_flamewear);
    RegisterCreatureAI(npc_atbt_battleship_controller);
    RegisterCreatureAI(npc_atbt_battleship);
    RegisterCreatureAI(npt_atbt_teleport);
    new npt_atbt_tele_gates;
    new npc_atbt_muradin();
    RegisterAuraScript(spell_atbt_bound_by_fel);
    RegisterAuraScript(spell_atbt_soulburn);
    RegisterSpellScript(spell_atbt_anihilation);
    RegisterSpellScript(spell_vantus_rune_antorus_choose);
    new eventobject_antorus_into();
    new spell_vantus_rune_antorus;
}
