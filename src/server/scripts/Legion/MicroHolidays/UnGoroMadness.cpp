/*
*/

//118271
struct npc_clutchmother_zavas : public ScriptedAI
{
    explicit npc_clutchmother_zavas(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        DespawnAllSummons();
        events.Reset();
        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 9000);
        events.RescheduleEvent(EVENT_2, 12000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DespawnAllSummons();
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void DespawnAllSummons()
    {
        std::list<Creature*> list;
        list.clear();
        me->GetCreatureListWithEntryInGrid(list, 114523, 200.0f);
        if (!list.empty())
            for (auto& cre : list)
                cre->DespawnOrUnsummon();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, 235990, false);

                events.RescheduleEvent(EVENT_1, 6000);
                break;
            case EVENT_2:
                DoCast(235993);
                events.RescheduleEvent(EVENT_2, 14000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//118268
struct npc_giantfin : public ScriptedAI
{
    explicit npc_giantfin(Creature* creature) : ScriptedAI(creature){}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        DespawnAllSummons();
        events.Reset();
        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 4000);
        events.RescheduleEvent(EVENT_2, 9000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DespawnAllSummons();
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void DespawnAllSummons()
    {
        std::list<Creature*> list;
        list.clear();
        me->GetCreatureListWithEntryInGrid(list, 118810, 200.0f);
        me->GetCreatureListWithEntryInGrid(list, 118812, 200.0f);
        if (!list.empty())
            for (auto& cre : list)
                cre->DespawnOrUnsummon();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }

        if (spell->Id == 235769)
        {
            auto& threatlist = me->getThreatManager().getThreatList();
            if (threatlist.empty())
                return;

            uint32 threatSize = threatlist.size();

            for (uint8 i = 0; i < threatSize; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235774, true);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(235762);
                events.RescheduleEvent(EVENT_1, 8000);
                break;
            case EVENT_2:
                Talk(0);
                DoCast(235769);
                events.RescheduleEvent(EVENT_2, 17000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

Position const posAT[48] =
{
    { -8161.94f, -1083.115f, -201.6921f },
    { -8161.94f, -1092.115f, -207.7341f },
    { -8161.94f, -1110.115f, -221.4117f },
    { -8161.94f, -1119.115f, -224.4112f },
    { -8161.94f, -1128.115f, -227.7803f },
    { -8161.94f, -1137.115f, -225.0319f },
    { -8170.94f, -1083.115f, -207.8955f },
    { -8170.94f, -1092.115f, -215.226f },
    { -8170.94f, -1101.115f, -218.4752f },
    { -8170.94f, -1110.115f, -221.0602f },
    { -8170.94f, -1119.115f, -223.2702f },
    { -8170.94f, -1128.115f, -225.2435f },
    { -8170.94f, -1137.115f, -221.1996f },
    { -8177.241f, -1083.115f, -211.9348f },
    { -8177.241f, -1092.115f, -215.7346f },
    { -8177.241f, -1101.115f, -218.2252f },
    { -8177.241f, -1110.115f, -220.3854f },
    { -8177.241f, -1119.115f, -222.3392f },
    { -8177.241f, -1128.115f, -223.7538f },
    { -8177.241f, -1137.115f, -219.8124f },
    { -8183.541f, -1083.115f, -212.5307f },
    { -8183.541f, -1092.115f, -215.7431f },
    { -8183.541f, -1101.115f, -217.8878f },
    { -8183.541f, -1110.115f, -219.798f },
    { -8183.541f, -1119.115f, -221.8141f },
    { -8183.541f, -1128.115f, -222.5851f },
    { -8183.541f, -1137.115f, -218.7789f },
    { -8192.541f, -1083.115f, -211.9094f },
    { -8192.541f, -1092.115f, -214.9507f },
    { -8192.541f, -1101.115f, -217.2789f },
    { -8192.541f, -1110.115f, -219.2179f },
    { -8192.541f, -1119.115f, -221.1705f },
    { -8192.541f, -1128.115f, -220.8751f },
    { -8192.541f, -1137.115f, -218.2152f },
    { -8201.541f, -1092.115f, -213.5187f },
    { -8207.841f, -1137.115f, -217.3117f },
    { -8207.841f, -1128.115f, -217.5539f },
    { -8207.841f, -1119.115f, -218.7365f },
    { -8207.841f, -1110.115f, -218.6452f },
    { -8207.841f, -1101.115f, -215.8363f },
    { -8207.841f, -1092.115f, -212.3851f },
    { -8207.841f, -1083.115f, -208.5526f },
    { -8201.541f, -1137.115f, -217.7068f },
    { -8201.541f, -1128.115f, -218.7548f },
    { -8201.541f, -1119.115f, -220.1249f },
    { -8201.541f, -1110.115f, -218.9252f },
    { -8201.541f, -1101.115f, -216.2675f },
    { -8161.94f, -1101.115f, -218.4751f }
};

//118270
struct npc_sunkeeper_croesus : public ScriptedAI
{
    explicit npc_sunkeeper_croesus(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 7000);
        events.RescheduleEvent(EVENT_2, 16000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }

        if (spell->Id == 235955)
            for (uint8 i = 0; i < 48; ++i)
                me->CastSpell(posAT[i], 235954, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(235944);
                events.RescheduleEvent(EVENT_1, 12000);
                break;
            case EVENT_2:
                DoCast(235955);
                events.RescheduleEvent(EVENT_2, 40686);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

Position const PointMove[4] =
{
    { -6849.83f, -696.16f, -271.40918f, 0.0f },
    { -6871.765f, -693.045f, -271.93203f, 0.0f },
    { -6858.377f, -770.06402f, -271.08038f, 0.0f },
    { -6990.795f, -681.0243f, -271.11471f, 0.0f }
};

//118277
struct npc_skrox : public ScriptedAI
{
    explicit npc_skrox(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
        me->SetReactState(REACT_AGGRESSIVE);
        SetCombatMovement(false);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 10000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void MovementInform(uint32 type, uint32 data) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (data)
            {
            case 1:
            case 2:
            case 3:
            case 4:
                me->SetReactState(REACT_AGGRESSIVE);
                SetCombatMovement(false);
                me->SetDisableGravity(true);
                me->SetCanFly(false);
                DoCast(236594);
                events.RescheduleEvent(EVENT_1, 20000);
                events.RescheduleEvent(EVENT_2, 1500);
                break;
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                SetCombatMovement(true);
                me->SetDisableGravity(true);
                me->SetCanFly(true);
                me->StopAttack(true);
                me->GetMotionMaster()->MovePoint(1, PointMove[urand(0, 3)], false);
                break;
            case EVENT_2:
                DoCast(236594);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//118273
struct npc_akaridal : public ScriptedAI
{
    explicit npc_akaridal(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();

        DoCast(236170);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        me->RemoveAura(236170);
        me->SetAnimKitId(0);
        events.RescheduleEvent(EVENT_1, 10000);
        events.RescheduleEvent(EVENT_2, 6000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(236230);
                events.RescheduleEvent(EVENT_1, 15000);
                break;
            case EVENT_2:
                if (auto victim = me->getVictim())
                    DoCast(victim, 236226, false);

                events.RescheduleEvent(EVENT_2, 12000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//118267
struct npc_tyrantus : public ScriptedAI
{
    explicit npc_tyrantus(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 6000);
        events.RescheduleEvent(EVENT_2, 15000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(235748);
                events.RescheduleEvent(EVENT_1, 32000);
                break;
            case EVENT_2:
                DoCast(235749);
                events.RescheduleEvent(EVENT_2, 15000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//118279
struct npc_dadanga : public ScriptedAI
{
    explicit npc_dadanga(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 7000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(236363);
                events.RescheduleEvent(EVENT_1, 27000);
                events.RescheduleEvent(EVENT_2, 13000);
                break;
            case EVENT_2:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, 236217, false);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//118272
struct npc_sherazin : public ScriptedAI
{
    explicit npc_sherazin(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;
    uint32 timer = 0;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
        timer = 0;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        timer = 10000;
        events.RescheduleEvent(EVENT_1, 22000);
        events.RescheduleEvent(EVENT_2, 5000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (timer)
        {
            if (timer <= diff)
            {
                timer = 41000;

                for (uint8 i = 0; i < 3; ++i)
                {
                    Position pos;
                    me->GetRandomNearPosition(pos, 20.0f);
                    me->CastSpell(pos, 236127, true);
                }
            }
            else
                timer -= diff;
        }

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(236133);
                events.RescheduleEvent(EVENT_1, 60000);
                break;
            case EVENT_2:
                if (!me->HasAura(236129))
                    DoCast(236070);

                events.RescheduleEvent(EVENT_2, 11000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

Position const ATPos[28] =
{
    { -7387.195f, -2023.212f, -271.5974f, 1.83626f },
    { -7389.165f, -2023.559f, -271.6965f, 1.83626f },
    { -7391.134f, -2023.906f, -271.6965f, 1.83626f },
    { -7393.104f, -2024.254f, -271.8064f, 1.83626f },
    { -7415.912f, -1906.424f, -272.3759f, 4.43985f },
    { -7417.881f, -1906.771f, -272.2091f, 4.43985f },
    { -7419.851f, -1907.118f, -272.0976f, 4.43985f },
    { -7421.821f, -1907.466f, -272.0009f, 4.43985f },
    { -7402.953f, -2025.99f, -272.0715f, 1.776965f },
    { -7404.922f, -2026.337f, -272.0715f, 1.776965f },
    { -7406.892f, -2026.685f, -271.9465f, 1.776965f },
    { -7431.669f, -1909.202f, -271.5544f, 4.823139f },
    { -7433.639f, -1909.549f, -271.6331f, 4.823139f },
    { -7435.609f, -1909.897f, -271.5913f, 4.823139f },
    { -7463.185f, -1914.759f, -271.9663f, 5.34544f },
    { -7440.377f, -2032.589f, -271.2672f, 1.145496f },
    { -7438.408f, -2032.241f, -271.1889f, 1.145496f },
    { -7436.438f, -2031.894f, -271.0773f, 1.145496f },
    { -7434.468f, -2031.547f, -271.0172f, 1.145496f },
    { -7432.499f, -2031.199f, -270.8779f, 1.145496f },
    { -7430.529f, -2030.852f, -270.8215f, 1.145496f },
    { -7428.559f, -2030.505f, -270.8215f, 1.145496f },
    { -7447.427f, -1911.98f, -271.7161f, 5.054824f },
    { -7445.458f, -1911.633f, -271.8413f, 5.054824f },
    { -7443.488f, -1911.286f, -271.7058f, 5.054824f },
    { -7441.518f, -1910.939f, -271.7607f, 5.054824f },
    { -7418.71f, -2028.768f, -271.2637f, 1.737139f },
    { -7416.741f, -2028.421f, -271.5002f, 1.737139f }
};

//118269
struct npc_queen_zavra : public ScriptedAI
{
    explicit npc_queen_zavra(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 7000);
        events.RescheduleEvent(EVENT_2, 3000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void SpellHit(Unit* /*owner*/, SpellInfo const* spell) override
    {
        if (spell->Id == 235886)
        {
            for (uint8 i = 0; i < 28; ++i)
                me->CastSpell(ATPos[i], 235888, true);
        }
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 235884, false);

                events.RescheduleEvent(EVENT_1, 7000);
                break;
            case EVENT_2:
                Talk(0);
                DoCast(235886);
                events.RescheduleEvent(EVENT_2, 43000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_tar_tyrant : public ScriptedAI
{
    explicit npc_tar_tyrant(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;
    bool useBonus;

    void Reset() override
    {
        useBonus = false;
        events.Reset();
        me->RemoveAllAreaObjects();
        DespawnAllSummons();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 9000);
        events.RescheduleEvent(EVENT_2, 12000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAreaObjects();
        DespawnAllSummons();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        if (spell->Id == 235675)
        {
            for (uint8 i = 0; i < 24; ++i)
            {
                Position pos;
                me->GetRandomNearPosition(pos, 60.0f);
                me->CastSpell(pos, 235676, true);
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(80) && !useBonus)
        {
            useBonus = true;
            DoCast(235757);
        }
    }

    void DespawnAllSummons()
    {
        std::list<Creature*> list;
        list.clear();
        me->GetCreatureListWithEntryInGrid(list, 119042, 200.0f);
        me->GetCreatureListWithEntryInGrid(list, 119041, 200.0f);
        if (!list.empty())
            for (auto& cre : list)
                cre->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 236358, false);

                events.RescheduleEvent(EVENT_1, 9000);
                break;
            case EVENT_2:
                for (uint8 i = 0; i < 3; ++i)
                {
                    Position pos;
                    me->GetFirstCollisionPosition(pos, frand(40.0f, 50.0f), frand(0.0f, 6.28f));
                    me->CastSpell(pos, 236315, false);
                }
                DoCast(236314);
                events.RescheduleEvent(EVENT_2, 46000);
                events.RescheduleEvent(EVENT_3, 3500);
                break;
            case EVENT_3:
                Talk(0);
                DoCast(236333);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//118929
struct npc_ravenous_larva : ScriptedAI
{
    explicit npc_ravenous_larva(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        useTalk = false;
    }

    EventMap events;
    ObjectGuid nextPlayer;
    bool useTalk;

    void IsSummonedBy(Unit* /*owner*/) override
    {
        events.RescheduleEvent(EVENT_1, 1000);
        events.RescheduleEvent(EVENT_3, 1000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto next = ObjectAccessor::GetPlayer(*me, nextPlayer))
                {
                    me->GetMotionMaster()->MovePoint(1, next->GetPosition());
                    events.RescheduleEvent(EVENT_2, 1000);
                }
                else
                {
                    nextPlayer.Clear();
                    useTalk = false;

                    if (auto owner = me->GetAnyOwner())
                    {
                        if (!owner->isDead())
                            nextPlayer = owner->GetGUID();
                    }
                    else if (auto newPlayer = me->FindNearestPlayer(50.0f, true))
                    {
                        if (newPlayer->isInCombat())
                            nextPlayer = newPlayer->GetGUID();
                    }

                    if (auto next = ObjectAccessor::GetPlayer(*me, nextPlayer))
                    {
                        me->GetMotionMaster()->MovePoint(1, next->GetPosition());

                        if (!useTalk)
                        {
                            if (!next->HasAura(173079))
                                DoCast(next, 173079, true);

                            Talk(0, next->GetGUID());
                            useTalk = true;
                        }
                    }

                    events.RescheduleEvent(EVENT_2, 1000);
                }
                break;
            case EVENT_2:
                if (auto target = ObjectAccessor::GetPlayer(*me, nextPlayer))
                {
                    if (target->isDead())
                        nextPlayer.Clear();
                    else
                        me->GetMotionMaster()->MovePoint(1, target->GetPosition());
                }
                else
                    events.RescheduleEvent(EVENT_1, 1000);

                events.RescheduleEvent(EVENT_2, 1000);
                break;
            case EVENT_3:
                if (auto target = ObjectAccessor::GetPlayer(*me, nextPlayer))
                    if (target->IsInDist2d(me, 3.0f))
                        target->CastSpell(target, 236015, false);

                events.RescheduleEvent(EVENT_3, 1000);
                break;
            }
        }
    }
};

//119041
struct npc_reanimated_sludge : ScriptedAI
{
    explicit npc_reanimated_sludge(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        if (auto boss = me->FindNearestCreature(118274, 100.0f, true))
            me->GetMotionMaster()->MoveFollow(boss, 0.5, 1.5, MOTION_SLOT_ACTIVE);
    }

    void MovementInform(uint32 type, uint32 /*id*/) override
    {
        if (type == FOLLOW_MOTION_TYPE)
            if (auto boss = me->FindNearestCreature(118274, 10.0f, true))
                DoCast(boss, 236335, true);
    }
};

//119042
struct npc_tar_pit : ScriptedAI
{
    explicit npc_tar_pit(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint16 timer;

    void IsSummonedBy(Unit* /*owner*/) override
    {
        DoCast(236297);
        timer = 2900;
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->HasAura(236333))
        {
            if (timer <= diff)
            {
                timer = 2900;

                if (!me->FindNearestPlayer(2.0f, true))
                    DoCast(me, 236334, true);
            }
            else
                timer -= diff;
        }
    }
};

//118812
struct npc_water_bubble_murloc : ScriptedAI
{
    explicit npc_water_bubble_murloc(Creature* creature) : ScriptedAI(creature) {}

    uint32 timer;

    void Reset() override
    {
        timer = 0;
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        DoCast(me, 235770, true);

        if (auto target = me->FindNearestPlayer(100.0f, true))
            AttackStart(target);
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        timer = 100;
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer)
        {
            if (timer <= diff)
            {
                timer = 2000;

                if (auto victim = me->getVictim())
                    DoCast(victim, 235857, false);
            }
            else
                timer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

//118810
struct npc_water_bubble : ScriptedAI
{
    explicit npc_water_bubble(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 timer = 0;

    void IsSummonedBy(Unit* owner) override
    {
        owner->CastSpell(me, 46598, true);
        timer = 3000;
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == 235874)
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
                vehicle->RemoveAllPassengers();

            me->DespawnOrUnsummon(1000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer)
        {
            if (timer <= diff)
            {
                timer = 0;
                me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 4.0f, 1.0f, 1.0f);
            }
            else
                timer -= diff;
        }
    }
};

//118956
struct npc_crystal_vine : ScriptedAI
{
    explicit npc_crystal_vine(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        DoCast(236108);
    }

    void SpellHit(Unit* /*target*/, SpellInfo const* spell) override
    {
        if (spell->Id == 236120)
            me->DespawnOrUnsummon();
    }
};

//235990
class spell_fertile_toxin : public AuraScript
{
    PrepareAuraScript(spell_fertile_toxin);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        target->CastSpell(target, 236055, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_fertile_toxin::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

//234781
class spell_dino_mojo : public SpellScript
{
    PrepareSpellScript(spell_dino_mojo);

    SpellCastResult Check()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(197912))
                return SPELL_FAILED_NOT_IN_RATED_BATTLEGROUND;
            else if (caster->GetMap()->IsDungeon() || caster->GetMap()->IsRaid() || caster->GetMap()->isChallenge() || caster->GetMap()->IsEventScenario() || caster->GetMap()->IsScenario())
                return SPELL_FAILED_NOT_HERE;
        }

        return SPELL_CAST_OK;
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Player* caster = GetCaster()->ToPlayer();
        Player* target = GetHitUnit()->ToPlayer();
        if (!target || !caster)
            return;

        if (auto entry = sBroadcastTextStore.LookupEntry(126599))
        {
            std::string text = DB2Manager::GetBroadcastTextValue(entry, caster->GetSession()->GetSessionDbLocaleIndex());
            caster->BossWhisper(text, LANG_UNIVERSAL, target->GetGUID());
        }

        uint32 rand_bonus[4] = { 234761, 234763, 234776, 234758 };
        target->CastSpell(target, rand_bonus[urand(0, 3)]);
        target->CastSpell(target, 234756, false);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint32 rand_bonus[4] = { 234761, 234763, 234776, 234758 };
        caster->CastSpell(caster, rand_bonus[urand(0, 3)]);
        caster->CastSpell(caster, 234756, false);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_dino_mojo::Check);
        OnEffectHitTarget += SpellEffectFn(spell_dino_mojo::HandleScript, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectHit += SpellEffectFn(spell_dino_mojo::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

//234756
class spell_dino_mojo_bonus : public AuraScript
{
    PrepareAuraScript(spell_dino_mojo_bonus);

    uint16 timer;

    bool Load() override
    {
        timer = 1000;
        return true;
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (timer)
        {
            if (timer <= diff)
            {
                timer = 1000;

                if (Aura* aura1 = caster->GetAura(234761))
                    if (AuraEffect* aurEff1 = caster->GetAuraEffect(234756, EFFECT_0))
                        aurEff1->SetAmount(aura1->GetStackAmount());
                if (Aura* aura2 = caster->GetAura(234763))
                    if (AuraEffect* aurEff2 = caster->GetAuraEffect(234756, EFFECT_1))
                        aurEff2->SetAmount(aura2->GetStackAmount());
                if (Aura* aura3 = caster->GetAura(234776))
                    if (AuraEffect* aurEff3 = caster->GetAuraEffect(234756, EFFECT_2))
                        aurEff3->SetAmount(aura3->GetStackAmount());
                if (Aura* aura4 = caster->GetAura(234758))
                    if (AuraEffect* aurEff4 = caster->GetAuraEffect(234756, EFFECT_3))
                        aurEff4->SetAmount(aura4->GetStackAmount());

                if (caster->HasAura(234761) && caster->HasAura(234763) && caster->HasAura(234776) && caster->HasAura(234758))
                {
                    if (Aura* aura1 = caster->GetAura(234761))
                    {
                        if (Aura* aura2 = caster->GetAura(234763))
                        {
                            if (Aura* aura3 = caster->GetAura(234776))
                            {
                                if (Aura* aura4 = caster->GetAura(234758))
                                {
                                    if (AuraEffect* aurEff5 = caster->GetAuraEffect(234756, EFFECT_4))
                                    {
                                        if (aura1->GetStackAmount() >= 1 && aura2->GetStackAmount() >= 1 && aura3->GetStackAmount() >= 1 && aura4->GetStackAmount() == 1)
                                            aurEff5->SetAmount(1);
                                        if (aura1->GetStackAmount() >= 2 && aura2->GetStackAmount() >= 2 && aura3->GetStackAmount() >= 2 && aura4->GetStackAmount() >= 2)
                                            aurEff5->SetAmount(2);
                                        if (aura1->GetStackAmount() >= 3 && aura2->GetStackAmount() >= 3 && aura3->GetStackAmount() >= 3 && aura4->GetStackAmount() >= 3)
                                            aurEff5->SetAmount(3);
                                        if (aura1->GetStackAmount() >= 4 && aura2->GetStackAmount() >= 4 && aura3->GetStackAmount() >= 4 && aura4->GetStackAmount() >= 4)
                                            aurEff5->SetAmount(4);
                                        if (aura1->GetStackAmount() >= 5 && aura2->GetStackAmount() >= 5 && aura3->GetStackAmount() >= 5 && aura4->GetStackAmount() >= 5)
                                            aurEff5->SetAmount(5);
                                    }
                                }
                            }
                        }
                    }
                }

                if (caster->HasAura(197912) || caster->GetMap()->IsDungeon() || caster->GetMap()->IsRaid() || caster->GetMap()->isChallenge() || caster->GetMap()->IsEventScenario() || caster->GetMap()->IsScenario())
                {
                    caster->RemoveAura(234761);
                    caster->RemoveAura(234763);
                    caster->RemoveAura(234776);
                    caster->RemoveAura(234758);
                }
            }
            else
                timer -= diff;
        }
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_dino_mojo_bonus::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_adaptation_caustic_blood : public AuraScript
{
    PrepareAuraScript(spell_adaptation_caustic_blood);

    void Tick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Player*> playerList;
        playerList.clear();
        GetPlayerListInGrid(playerList, caster, 100.0f);
        Trinity::Containers::RandomResizeList(playerList, 1);

        if (!playerList.empty())
            for (auto& target : playerList)
                caster->CastSpell(target, 235679, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_adaptation_caustic_blood::Tick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//235757
class spell_adaptation : public SpellScript
{
    PrepareSpellScript(spell_adaptation);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint32 bonus_spell[5] = { 235654, 235655, 235656, 235658, 235659/*, 235660*/ };
        caster->CastSpell(caster, bonus_spell[urand(0, 4)], true);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_adaptation::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

//236358
class spell_tar_strike : public AuraScript
{
    PrepareAuraScript(spell_tar_strike);

    void Tick(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (auto aura = target->GetAura(236358))
        {
            if (aura->GetStackAmount() == 5)
            {
                target->RemoveAura(236358);
                target->CastSpell(target, 236360, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_tar_strike::Tick, EFFECT_3, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//235656
class spell_adaptation_heavy_footed : public AuraScript
{
    PrepareAuraScript(spell_adaptation_heavy_footed);

    void OnPeriodic(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->isMoving())
            caster->CastSpell(caster, GetSpellInfo()->Effects[EFFECT_0]->TriggerSpell, false);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_adaptation_heavy_footed::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

//236133
class spell_bloom : public SpellScript
{
    PrepareSpellScript(spell_bloom);

    void SelectTargets(std::list<WorldObject*>&targets)
    {
        targets.remove_if([](WorldObject* object) -> bool
        {
            Unit* unit = object->ToUnit();
            if (!unit)
                return true;

            if (unit->HasAura(236128))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_bloom::SelectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

void AddSC_UnGoroMadness()
{
    //RegisterCreatureAI(npc_clutchmother_zavas);
    //RegisterCreatureAI(npc_ravenous_larva);
    //RegisterCreatureAI(npc_giantfin);
    //RegisterCreatureAI(npc_sunkeeper_croesus);
    //RegisterCreatureAI(npc_skrox);
    //RegisterCreatureAI(npc_akaridal);
    //RegisterCreatureAI(npc_tyrantus);
    //RegisterCreatureAI(npc_queen_zavra);
    //RegisterCreatureAI(npc_tar_tyrant);
    //RegisterCreatureAI(npc_dadanga);
    //RegisterCreatureAI(npc_tar_pit);
    //RegisterCreatureAI(npc_reanimated_sludge);
    //RegisterCreatureAI(npc_water_bubble);
    //RegisterCreatureAI(npc_water_bubble_murloc);
    //RegisterCreatureAI(npc_crystal_vine);
    //RegisterSpellScript(spell_dino_mojo);
    //RegisterSpellScript(spell_adaptation);
    //RegisterSpellScript(spell_bloom);
    //RegisterAuraScript(spell_dino_mojo_bonus);
    //RegisterAuraScript(spell_fertile_toxin);
    //RegisterAuraScript(spell_adaptation_caustic_blood);
    //RegisterAuraScript(spell_tar_strike);
    //RegisterAuraScript(spell_adaptation_heavy_footed);
}