/*
*/

//122138
struct npc_moonkin_hatchling : ScriptedAI
{
    explicit npc_moonkin_hatchling(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->AddDelayedEvent(1000, [=]() -> void
        {
            if (me->FindNearestCreature(122241, 40.0f, true))
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCE);
            else
            {
                DoCast(243958);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
        });
    }
};

//122139
struct npc_moonkin_hatchling_lunar_strike : ScriptedAI
{
    explicit npc_moonkin_hatchling_lunar_strike(Creature* creature) : ScriptedAI(creature)
    {
        me->AddUnitState(UNIT_STATE_ROOT);
    }

    uint32 SpellVisualCast = 0;

    void Reset() override
    {
        SpellVisualCast = urand(100, 2500);
    }

    void UpdateAI(uint32 diff) override
    {
        if (SpellVisualCast <= diff)
        {
            SpellVisualCast = 4000;

            if (!me->isInCombat())
            {
                if (auto target = me->FindNearestCreature(122252, 30.0f, true))
                    AttackStart(target);
            }
            else
            {
                if (auto victim = me->getVictim())
                    DoCast(victim, 243383, false);
            }
        }
        else
            SpellVisualCast -= diff;
    }
};

//122134
struct npc_makkaw : ScriptedAI
{
    explicit npc_makkaw(Creature* creature) : ScriptedAI(creature) {}

    bool useEvent;

    void Reset() override
    {
        useEvent = false;
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
            useEvent = false;
    }

    void sQuestAccept(Player* /*player*/, Quest const* /*quest*/) override
    {
        if (!useEvent)
        {
            useEvent = true;

            if (auto astralaire = me->FindNearestCreature(122241, 50.0f, true))
                astralaire->GetAI()->DoAction(ACTION_1);
        }
    }
};

//122241
struct npc_astralaire : ScriptedAI
{
    explicit npc_astralaire(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    std::list<Player*> playerList;
    std::list<Creature*> creatureList;

    void Reset() override
    {
        me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_DANCE);

        playerList.clear();
        creatureList.clear();
        events.Reset();
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
            events.RescheduleEvent(EVENT_1, 3000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(0);
                events.RescheduleEvent(EVENT_2, 4000);
                break;
            case EVENT_2:
                Talk(1);
                events.RescheduleEvent(EVENT_3, 6000);
                break;
            case EVENT_3:
                Talk(2);
                events.RescheduleEvent(EVENT_4, 7000);
                break;
            case EVENT_4:
                Talk(3);
                events.RescheduleEvent(EVENT_5, 7000);
                break;
            case EVENT_5:
                Talk(4);
                events.RescheduleEvent(EVENT_6, 1000);
                break;
            case EVENT_6:
                me->GetCreatureListWithEntryInGrid(creatureList, 122138, 50.0f);
                if (!creatureList.empty())
                    for (auto& cre : creatureList)
                        cre->AI()->Talk(0);

                events.RescheduleEvent(EVENT_7, 4000);
                break;
            case EVENT_7:
                playerList.clear();
                GetPlayerListInGrid(playerList, me, 20.0f);
                if (!playerList.empty())
                    for (auto& player : playerList)
                        if (player->GetQuestStatus(47430) == QUEST_STATUS_INCOMPLETE)
                            player->KilledMonsterCredit(me->GetEntry());

                if (auto makkaw = me->FindNearestCreature(122134, 50.0f, true))
                    makkaw->AI()->DoAction(ACTION_1);
                break;
            }
        }
    }
};

//122390
struct npc_dramock: ScriptedAI
{
    explicit npc_dramock(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    std::list<Player*> playerList;
    std::list<ObjectGuid> playerGUIDs;
    bool useEvent;

    void Reset() override
    {
        DoCast(243958);

        events.Reset();
        playerList.clear();
        playerGUIDs.clear();
        useEvent = false;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 15.0f)
            return;

        if (who->ToPlayer()->GetQuestStatus(47430) != QUEST_STATUS_INCOMPLETE)
            return;

        uint32 credit = who->ToPlayer()->GetQuestObjectiveData(47430, me->GetEntry());

        if (credit != 1)
        {
            if (!useEvent)
            {
                useEvent = true;
                credit = 0;

                me->AddDelayedEvent(500, [=]() -> void
                {
                    playerGUIDs.clear();
                    playerList.clear();

                    GetPlayerListInGrid(playerList, me, 15.0f);
                    if (!playerList.empty())
                        for (auto& player : playerList)
                            if (player->GetQuestStatus(47430) == QUEST_STATUS_INCOMPLETE)
                                playerGUIDs.push_back(player->GetGUID());

                    for (auto& guid : playerGUIDs)
                        Talk(0, guid);

                    events.RescheduleEvent(EVENT_1, 4000);
                });
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                for (auto& guid : playerGUIDs)
                    Talk(1, guid);

                events.RescheduleEvent(EVENT_2, 6000);
                break;
            case EVENT_2:
                for (auto& guid : playerGUIDs)
                    Talk(2, guid);

                events.RescheduleEvent(EVENT_3, 7000);
                break;
            case EVENT_3:
                for (auto& guid : playerGUIDs)
                    Talk(3, guid);

                events.RescheduleEvent(EVENT_4, 7000);
                break;
            case EVENT_4:
                Talk(4);
                events.RescheduleEvent(EVENT_5, 5000);
                break;
            case EVENT_5:
                useEvent = false;

                if (!playerList.empty())
                    for (auto& player : playerList)
                        player->KilledMonsterCredit(me->GetEntry());
                break;
            }
        }
    }
};

//122256
struct npc_pewkew : ScriptedAI
{
    explicit npc_pewkew(Creature* creature) : ScriptedAI(creature)
    {
        me->AddUnitState(UNIT_STATE_ROOT);
    }

    EventMap events;
    std::list<Player*> playerList;
    bool useEvent;

    void Reset() override
    {
        events.Reset();
        playerList.clear();
        useEvent = false;

        events.RescheduleEvent(EVENT_6, 1000);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 15.0f)
            return;

        if (who->ToPlayer()->GetQuestStatus(47430) != QUEST_STATUS_INCOMPLETE)
            return;

        uint32 credit = who->ToPlayer()->GetQuestObjectiveData(47430, me->GetEntry());

        if (credit != 1)
        {
            if (!useEvent)
            {
                useEvent = true;
                credit = 0;

                me->AddDelayedEvent(500, [=]() -> void
                {
                    Talk(0);
                    events.RescheduleEvent(EVENT_1, 4000);
                });
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(1);
                events.RescheduleEvent(EVENT_2, 6000);
                break;
            case EVENT_2:
                Talk(2);
                events.RescheduleEvent(EVENT_3, 7000);
                break;
            case EVENT_3:
                Talk(3);
                events.RescheduleEvent(EVENT_4, 7000);
                break;
            case EVENT_4:
                Talk(4);
                events.RescheduleEvent(EVENT_5, 5000);
                break;
            case EVENT_5:
                useEvent = false;

                playerList.clear();
                GetPlayerListInGrid(playerList, me, 20.0f);
                if (!playerList.empty())
                    for (auto& player : playerList)
                        player->KilledMonsterCredit(me->GetEntry());
                break;
            case EVENT_6:
                if (!me->isInCombat())
                {
                    if (auto target = me->FindNearestCreature(122252, 30.0f, true))
                        AttackStart(target);
                }
                else
                {
                    if (auto victim = me->getVictim())
                        DoCast(victim, 243829, false);
                }
                events.RescheduleEvent(EVENT_6, 1700);
                break;
            }
        }
    }
};

//122139
struct npc_clookle : ScriptedAI
{
    explicit npc_clookle(Creature* creature) : ScriptedAI(creature)
    {
        DoCast(244526);
    }

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 0)
        {
            me->RemoveAura(244526);
            me->AddDelayedEvent(3000, [=]() -> void {  DoCast(244526); });

            player->DestroyItemCount(150750, 1, true);
            player->CastSpell(player, 244157, false);
            player->CLOSE_GOSSIP_MENU();
        }
    }
};

//122859,121946
struct npc_moonkin_hatchling_pet : ScriptedAI
{
    explicit npc_moonkin_hatchling_pet(Creature* creature) : ScriptedAI(creature) {}

    uint32 checking;
    uint32 SpellVisualCast;

    void Reset() override
    {
        checking = 1000;
    }

    void EnterEvadeMode() override
    {
        SpellVisualCast = 0;
        me->SetReactState(REACT_PASSIVE);
        ScriptedAI::EnterEvadeMode();
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == 245342)
            me->SendPlaySpellVisualKit(0, 84318, 0);
    }

    void UpdateAI(uint32 diff) override
    {
        if (checking <= diff)
        {
            checking = 1000;

            if (auto owner = me->GetAnyOwner())
            {
                if (owner->isInCombat())
                {
                    me->SetReactState(REACT_AGGRESSIVE);

                    if (me->isInCombat())
                    {
                        SpellVisualCast = 1000;

                        if (auto target = owner->getVictim())
                            AttackStart(target);
                    }
                }
                else
                {
                    if (me->isInCombat())
                        EnterEvadeMode();
                }
                
                if (owner->IsMounted() || owner->HasAura(783) || owner->HasAura(125883))
                {
                    if (!me->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY))
                    {
                        me->SetCanFly(true);
                        me->SetDisableGravity(true);
                        me->SetHover(true);
                        me->SetSpeed(MOVE_FLIGHT, owner->GetSpeed(MOVE_FLIGHT));
                    }
                }
                else
                {
                    if (me->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY))
                    {
                        me->SetCanFly(false);
                        me->SetDisableGravity(false);
                        me->SetHover(false);
                    }
                }
            }
        }
        else
            checking -= diff;

        if (SpellVisualCast <= diff)
        {
            SpellVisualCast = 4000;

            if (me->isInCombat())
                if (auto victim = me->getVictim())
                    DoCast(victim, 243383, false);
        }
        else
            SpellVisualCast -= diff;
    }
};

//247428
class spell_moonkin_hatchling_tracker : public AuraScript
{
    PrepareAuraScript(spell_moonkin_hatchling_tracker);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (!caster->HasAura(243333) && !caster->HasAura(244151) && !caster->HasAura(244155) && !caster->HasAura(244157) && !caster->HasAura(244158))
            caster->CastSpellDelay(caster, 243333, false, 5000);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_moonkin_hatchling_tracker::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

//247429
class spell_item_beanie_boomie : public SpellScript
{
    PrepareSpellScript(spell_item_beanie_boomie);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(244157))
            caster->CastSpell(caster, 244158, false);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_item_beanie_boomie::HandleOnHit, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

//244495
class spell_moonkissed_antidote : public SpellScript
{
    PrepareSpellScript(spell_moonkissed_antidote);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(244151))
            caster->CastSpell(caster, 244155, false);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_moonkissed_antidote::HandleOnHit, EFFECT_0, SPELL_EFFECT_CREATE_ITEM);
    }
};


//244188
class spell_feed_moonkin_hatchling : public AuraScript
{
    PrepareAuraScript(spell_feed_moonkin_hatchling);

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
        {
            if (Aura* aura = caster->GetAura(244188))
            {
                if (aura->GetStackAmount() == 5)
                {
                    if (caster->HasAura(243333))
                    {
                        caster->CastSpell(caster, 244151, false);
                        caster->RemoveAura(244188);
                    }
                }
            }
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_feed_moonkin_hatchling::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

void AddSC_MoonkinFestival()
{
    RegisterCreatureAI(npc_moonkin_hatchling);
    RegisterCreatureAI(npc_moonkin_hatchling_lunar_strike);
    RegisterCreatureAI(npc_makkaw);
    RegisterCreatureAI(npc_astralaire);
    RegisterCreatureAI(npc_dramock);
    RegisterCreatureAI(npc_pewkew);
    RegisterCreatureAI(npc_clookle);
    RegisterCreatureAI(npc_moonkin_hatchling_pet);
    RegisterAuraScript(spell_moonkin_hatchling_tracker);
    RegisterAuraScript(spell_feed_moonkin_hatchling);
    RegisterSpellScript(spell_item_beanie_boomie);
    RegisterSpellScript(spell_moonkissed_antidote);
}