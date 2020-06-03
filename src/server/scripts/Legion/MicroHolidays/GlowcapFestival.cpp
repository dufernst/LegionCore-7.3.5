/*
*/

enum eSay
{
    SAY_TALK_1,
    SAY_TALK_2,
    SAY_TALK_3,
    SAY_TALK_4
};

struct npc_tzhuffk : ScriptedAI
{
    explicit npc_tzhuffk(Creature* creature) : ScriptedAI(creature), summons(me) {}

    EventMap events;
    ObjectGuid fshoo;
    SummonList summons;
    bool intro;

    void Reset() override
    {
        events.Reset();
        summons.DespawnAll();
        intro = false;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!intro && me->IsWithinDistInMap(who, 50.0f))
        {
            intro = true;
            events.RescheduleEvent(EVENT_1, 10000);
        }
    }

    void JustSummoned(Creature* sum) override
    {
        summons.Summon(sum);

        switch (sum->GetEntry())
        {
        case 118509:
            fshoo.Clear();
            fshoo = sum->GetGUID();
            break;
        default:
            break;
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            Talk(SAY_TALK_4);
            events.RescheduleEvent(EVENT_1, 340000);
            break;
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
                Talk(SAY_TALK_1);
                events.RescheduleEvent(EVENT_2, 33000);
                break;
            case EVENT_2:
                Talk(SAY_TALK_2);
                events.RescheduleEvent(EVENT_3, 6000);
                break;
            case EVENT_3:
                Talk(SAY_TALK_3);
                events.RescheduleEvent(EVENT_4, 3000);
                break;
            case EVENT_4:
                me->SummonCreature(118509, 218.1059f, 8517.672f, 25.15009f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);
                break;
            }
        }
    }
};

struct npc_fshoo : ScriptedAI
{
    explicit npc_fshoo(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;
    bool eventComplete;
    std::set<ObjectGuid> listPlayers;
    uint32 timeCount = 0;

    void Reset() override
    {
        events.Reset();
        eventComplete = false;
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        events.RescheduleEvent(EVENT_1, 4000);
        events.RescheduleEvent(EVENT_2, 500);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth())
            damage = 0;

        if (me->HealthAbovePct(6) || eventComplete)
            return;

        eventComplete = true;
        me->DespawnOrUnsummon(500);

        if (auto owner = me->GetAnyOwner())
            owner->GetAI()->DoAction(ACTION_1);

        if (!listPlayers.empty())
        {
            for (auto& target : listPlayers)
            {
                if (Player* player = ObjectAccessor::GetPlayer(*me, target))
                {
                    if (Aura const* aura = me->GetAura(235202))
                    {
                        if (player->GetCurrentAreaID() == 3649)
                        {
                            uint32 repChange = (aura->GetStackAmount() * 50);
                            player->GetReputationMgr().ModifyReputation(sFactionStore.LookupEntry(970), repChange);
                        }
                    }
                }
            }
        }
    }

    void HealReceived(Unit* who, uint32& /*addhealth*/) override
    {
        if (who && who->IsPlayer())
            listPlayers.insert(who->GetGUID());
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                    if (!me->HasAura(235269))
                        DoCast(235202);
                    events.RescheduleEvent(EVENT_1, 3000);
                    break;
                case EVENT_2:
                    if (Aura* aura = me->GetAura(235202))
                    {
                        if (aura->GetStackAmount() > 100)
                            timeCount = 5;
                        else if (aura->GetStackAmount() >= 50 && aura->GetStackAmount() < 100)
                            timeCount = 4;
                        else if (aura->GetStackAmount() < 50 || !me->HasAura(235202))
                            timeCount = 3;

                        if (aura->GetStackAmount() >= 30 && aura->GetStackAmount() <= 50)
                            me->SetDisplayId(74773);
                        else if (aura->GetStackAmount() >= 100 && aura->GetStackAmount() <= 120)
                            me->SetDisplayId(74771);
                    }

                    if (!me->HasAura(235269))
                    {
                        uint32 damage = (me->GetMaxHealth() / 100) * timeCount;
                        me->DealDamage(me, damage);
                    }
                    events.RescheduleEvent(EVENT_2, 1000);
                    break;
            }
        }
    }
};

void AddSC_GlowcapFestival()
{
    RegisterCreatureAI(npc_tzhuffk);
    RegisterCreatureAI(npc_fshoo);
}