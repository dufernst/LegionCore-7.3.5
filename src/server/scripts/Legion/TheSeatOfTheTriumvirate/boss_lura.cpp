/*
    The Seat of the Triumvirate: L'ura [heroic & mythic]
    TODO hp mods to boss and trash
*/

#include "the_seat_of_the_triumvirate.h"

enum Spells
{
    //Lura
    SPELL_DARK_BLAST            = 244750,
    SPELL_CALL                  = 247795,
    SPELL_CHANNEL               = 245393,
    SPELL_BLAST                 = 245289,
    SPELL_SELF_STUN             = 247816,
    SPELL_STACKS                = 247915,
    SPELL_CADENCE               = 247930,
    SPELL_IMMUNITY              = 254020,
    SPELL_SHIFT                 = 249009,
    SPELL_REPLACE               = 249057,
    SPELL_OUTRO_MOVIE           = 250035,
    //void portal
    SPELL_PORT_VISUAL           = 247840,
    SPELL_CHANNEL_ADD           = 247835,
    SPELL_SMALL_ADD             = 245235,
    SPELL_ADD_PEREODIC          = 247909,
    //small add
    SPELL_SUM_TRIG              = 245244,
    SPELL_LASHING               = 254727,
    SPELL_REMNANT_SUM           = 245244,
    SPELL_REMNANT_AT            = 245241,
    SPELL_REMNANT_AT_D          = 245242,
    //great warden
    SPELL_FRAGMMENT             = 245164,
    SPELL_FRAGMMENT_D           = 245177,
    SPELL_ERUPTING              = 245178,
    SPELL_MISILE_VISUAL         = 245187,
    SPELL_SHOCK                 = 247948,
    //intro
    SPELL_INTRO_CONVERS         = 250011,
    SPELL_WARDEN_VISUAL         = 241896,
    SPELL_WARDEN_FEAR           = 248133,
    SPELL_WARDEN_REMNANT        = 248128,
    //alleria
    SPELL_CLOSE_PORTAL          = 247878,
    SPELL_BREAK_SHIELD          = 250318,
    SPELL_TRANSFORM_VE          = 250027,
};

enum eEvents
{
    EVENT_BLAST                 = 1,
    EVENT_SHIFT                 = 2,
    EVENT_CADENCE               = 3,
};

Position const riftIntro[3] =
{
    { 5993.89f, 10259.29f, 15.95f, 4.75f },
    { 6017.91f, 10236.09f, 15.51f, 4.75f },
    { 6012.26f, 10254.50f, 15.34f, 3.82f },
};

Position const riftA[2] =
{
    { 5993.89f, 10259.29f, 15.95f, 4.75f },
    { 6017.91f, 10236.09f, 15.51f, 4.75f },
};

Position const aw[2] =
{
    { 6008.14f, 10249.90f, 14.87f, 3.74f },
    { 6007.62f, 10252.79f, 15.07f, 3.74f },
};

Position const voidP[2] =
{
    { 6018.16f, 10221.20f, 13.22f, 2.48f },
    { 5978.56f, 10258.90f, 14.13f, 5.58f },
};

//124729
class boss_lura : public CreatureScript
{
public:
    boss_lura() : CreatureScript("boss_lura") {}

    struct boss_luraAI : public BossAI
    {
        boss_luraAI(Creature* creature) : BossAI(creature, DATA_LURA)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            if (instance->GetData(LURA_INTRO_ADDS) != DONE)
                IntroAdds();
            if (instance->GetData(LURA_INTRO_ADDS) == DONE)
            {
                DoAction(1);
                ResetAdds();
            }
            AddsDiedCount = 0;
            me->SetControlled(1, UNIT_STATE_ROOT);
        }

        uint8 AddsDiedCount = 0;
        uint8 shieldsbreaked = 0;

        void Reset() override 
        {
            _Reset();
            CleanUp();
            shieldsbreaked = 0;
        }

        void EnterCombat(Unit* /*who*/) override 
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_BLAST, urand(10000, 18000));
            me->AddDelayedEvent(1500, [this] {
                DoCast(SPELL_CALL);
            });
            me->AddDelayedEvent(3500, [this] {
                me->SummonCreature(NPC_VOID_PORTAL, voidP[0]);
            });
        }

        void CleanUp()
        {
            me->AddDelayedEvent(6000, [this] {
                me->RemoveAurasDueToSpell(SPELL_CHANNEL);
                me->RemoveAurasDueToSpell(SPELL_STACKS);
                me->RemoveAurasDueToSpell(SPELL_IMMUNITY);
                std::list<Creature*> creaList;
                GetCreatureListWithEntryInGrid(creaList, me, NPC_VOID_PORTAL, 100.0f);
                GetCreatureListWithEntryInGrid(creaList, me, NPC_VOID_PORTAL_ADD, 100.0f);
                GetCreatureListWithEntryInGrid(creaList, me, NPC_LURA_VOID_ZONE, 100.0f);
                for (auto const& spawns : creaList)
                    if (!creaList.empty())
                        spawns->DespawnOrUnsummon(500);
            });
        }

        void JustReachedHome() override
        {
            me->SetControlled(1, UNIT_STATE_ROOT);
            ResetAdds();
        }

        void JustDied(Unit* /*killer*/) override 
        {
            _JustDied();
            CleanUp();
            DoCast(SPELL_OUTRO_MOVIE);
            me->SummonCreature(NPC_LURA_ALLERIA_OUTRO, aw[0], TEMPSUMMON_TIMED_DESPAWN, 45000);
            me->SummonCreature(NPC_LURA_WALKER_OUTRO, aw[1], TEMPSUMMON_TIMED_DESPAWN, 45000);
        }

        void IntroAdds()
        {
            me->SummonCreature(NPC_LURA_INTROPORTAL, riftIntro[0]);
            me->SummonCreature(NPC_LURA_INTROPORTAL, riftIntro[1]);
        }

        void ResetAdds()
        {
            me->SummonCreature(NPC_LURA_INTROPORTAL, riftIntro[2], TEMPSUMMON_TIMED_DESPAWN, 2500);
            me->AddDelayedEvent(1000, [this] {
                me->SummonCreature(NPC_LURA_ALLERIA, aw[0]);
                me->SummonCreature(NPC_LURA_LOCUSWALKER, aw[1]);
            });
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_CALL)
            {
                me->AddDelayedEvent(500, [this] {
                    DoCast(me, SPELL_IMMUNITY);
                    DoCast(me, SPELL_CHANNEL);
                });
            }
            if (spellInfo->Id == SPELL_SHIFT)
            {
                events.RescheduleEvent(EVENT_SHIFT, 15000);
                std::list<Creature*> creaList;
                GetCreatureListWithEntryInGrid(creaList, me, NPC_LURA_VOID_ZONE, 100.0f);
                for (std::list<Creature*>::iterator itr = creaList.begin(); itr != creaList.end(); ++itr)
                    if (!creaList.empty())
                        if (Creature* vz = *itr)
                            vz->AI()->DoAction(1);
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            switch (summon->GetEntry())
            {
            case NPC_LURA_INTROPORTAL:
                AddsDiedCount++;
                break;
            }
            if (AddsDiedCount == 2)
            {
                ResetAdds();
                DoAction(1);
                instance->SetData(LURA_INTRO_ADDS, DONE);
            }
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                me->AddDelayedEvent(2000, [this] {
                    if (instance)
                    {
                        Conversation* conversation = new Conversation;
                        if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 5647, me, NULL, *me))
                            delete conversation;
                    }
                });
                me->AddDelayedEvent(24000, [this] {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                });
                break;
            case 2:
                if (me->isInCombat())
                {
                    me->AddDelayedEvent(1500, [this] {
                        ++shieldsbreaked;
                        me->InterruptSpell(CURRENT_CHANNELED_SPELL);
                        me->RemoveAurasDueToSpell(SPELL_STACKS);
                        me->RemoveAurasDueToSpell(SPELL_CHANNEL);
                        me->RemoveAurasDueToSpell(SPELL_IMMUNITY);
                        DoCast(SPELL_SELF_STUN);
                    });
                }
                break;
            case 3:
                if (me->isInCombat())
                {
                    me->AddDelayedEvent(1500, [this] {
                        DoCast(SPELL_CALL);
                    });
                    me->AddDelayedEvent(2000, [this] {
                        DoCast(me, SPELL_IMMUNITY);
                        DoCast(me, SPELL_CHANNEL);
                    });
                    me->AddDelayedEvent(3500, [this] {
                        me->SummonCreature(NPC_VOID_PORTAL, voidP[0]);
                        me->SummonCreature(NPC_VOID_PORTAL, voidP[1]);
                    });
                }
                break;
            case 4:
                if (me->isInCombat())
                {
                    me->AddDelayedEvent(2000, [this]
                    {
                        me->SetControlled(0, UNIT_STATE_ROOT);
                        DoCast(me, SPELL_CHANNEL);
                    });
                    if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                        events.RescheduleEvent(EVENT_SHIFT, 21000);

                    events.RescheduleEvent(EVENT_CADENCE, 25000);
                }
                break;
            }
        }

        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
        {
            if (spellId == SPELL_SELF_STUN && !apply)
                if (me->isInCombat())
                {
                    if (shieldsbreaked == 1)
                        DoAction(3);
                    if (shieldsbreaked == 2)
                        DoAction(4);
                }
        }

        void UpdateAI(uint32 diff) override 
        {
            if (!UpdateVictim())
                return;

            if (CheckHomeDistToEvade(diff, 62.0f))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BLAST:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 39.0f, true))
                        if (!me->HasAura(SPELL_SELF_STUN))
                            DoCast(target, SPELL_BLAST, true);
                    events.RescheduleEvent(EVENT_BLAST, urand(10000, 18000));
                    break;
                case EVENT_SHIFT:
                    DoCast(SPELL_SHIFT);
                    break;
                case EVENT_CADENCE:
                    DoCast(SPELL_CADENCE);
                    events.RescheduleEvent(EVENT_CADENCE, urand(13000, 15000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_luraAI (creature);
    }
};

//123187
class npc_lura_alleria : public CreatureScript
{
public:
    npc_lura_alleria() : CreatureScript("npc_lura_alleria") {}

    struct npc_lura_alleriaAI : public ScriptedAI
    {
        npc_lura_alleriaAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetPower(POWER_ENERGY, 0);
        }

        InstanceScript* instance;
        uint8 casted = 0;

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                if (instance)
                    if (instance->GetBossState(DATA_LURA) == IN_PROGRESS)
                    {
                        me->AddDelayedEvent(5000, [this] {
                            me->SummonCreature(NPC_LURA_ALLERIA, aw[0]);
                            me->SummonCreature(NPC_LURA_LOCUSWALKER, aw[1]);
                        });
                    }
                break;
            }
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_BREAK_SHIELD)
            {
                if (Creature* lura = me->FindNearestCreature(NPC_LURA, 100.0f, true))
                    lura->AI()->DoAction(2);
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_CLOSE_PORTAL)
            {
                ++casted;

                if (casted == 1)
                {
                    me->AddDelayedEvent(5000, [this] {
                        me->SetPower(POWER_ENERGY, 100);
                    });

                    me->AddDelayedEvent(5500, [this] {
                        if (Creature* lura = me->FindNearestCreature(NPC_LURA, 100.0f, true))
                            DoCast(lura, SPELL_BREAK_SHIELD);
                    });
                }
                if (casted == 2)
                {
                    me->AddDelayedEvent(5000, [this] {
                        me->SetPower(POWER_ENERGY, 50);
                    });
                }
                if (casted == 3)
                {
                    me->AddDelayedEvent(5000, [this] {
                        me->SetPower(POWER_ENERGY, 100);
                    });

                    me->AddDelayedEvent(5500, [this] {
                        if (Creature* lura = me->FindNearestCreature(NPC_LURA, 100.0f, true))
                            DoCast(lura, SPELL_BREAK_SHIELD);
                    });
                }
            }
            if (spellInfo->Id == SPELL_BREAK_SHIELD)
            {
                me->SetPower(POWER_ENERGY, 0);
                Talk(0);
            }
        }

        void Reset() override 
        {
            casted = 0;
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_alleriaAI(creature);
    }
};

//125860
class npc_lura_intro_rift_warden : public CreatureScript
{
public:
    npc_lura_intro_rift_warden() : CreatureScript("npc_lura_intro_rift_warden") {}

    struct npc_lura_intro_rift_wardenAI : public ScriptedAI
    {
        npc_lura_intro_rift_wardenAI(Creature* creature) : ScriptedAI(creature)
        {
            DoCast(me, SPELL_WARDEN_VISUAL);
        }

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, urand(5000,8000));
            events.RescheduleEvent(EVENT_2, urand(8000,10000));
            events.RescheduleEvent(EVENT_3, urand(10000,12000));
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
                    DoCast(SPELL_WARDEN_FEAR);
                    events.RescheduleEvent(EVENT_1, urand(8000, 12000));
                    break;
                case EVENT_2:
                    DoCast(SPELL_WARDEN_REMNANT);
                    events.RescheduleEvent(EVENT_2, urand(15000, 17000));
                    break;
                case EVENT_3:
                    me->SummonCreature(NPC_LURA_RIFT_ADD, riftA[0]);
                    me->SummonCreature(NPC_LURA_RIFT_ADD, riftA[1]);
                    events.RescheduleEvent(EVENT_3, urand(10000, 12000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_intro_rift_wardenAI(creature);
    }
};

//124734
class npc_lura_void_portal : public CreatureScript
{
public:
    npc_lura_void_portal() : CreatureScript("npc_lura_void_portal") {}

    struct npc_lura_void_portalAI : public ScriptedAI
    {
        npc_lura_void_portalAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(me, SPELL_PORT_VISUAL, true);
            DoCast(me, SPELL_ADD_PEREODIC);
            me->AddDelayedEvent(1000, [this] {
                if (Creature* grward = me->SummonCreature(NPC_VOID_PORTAL_ADD, me->GetPositionX() + frand(2, 4), me->GetPositionY() + frand(2, 4), me->GetPositionZ(), me->GetOrientation()))
                    grward->AI()->DoZoneInCombat();
            });
        }

        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
        {
            if (spellId == SPELL_CLOSE_PORTAL && !apply)
                me->DespawnOrUnsummon();
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            if (summon->GetEntry() == NPC_VOID_PORTAL_ADD)
            {
                me->RemoveAurasDueToSpell(SPELL_ADD_PEREODIC);
                if (Creature* walker = me->FindNearestCreature(NPC_LURA_LOCUSWALKER, 100.0f, true))
                    walker->AI()->Talk(0);

                if (Creature* alleria = me->FindNearestCreature(NPC_LURA_ALLERIA, 100.0f, true))
                    if (!alleria->HasUnitState(UNIT_STATE_CASTING))
                        alleria->CastSpell(me, SPELL_CLOSE_PORTAL);
                    else
                    {
                        me->AddDelayedEvent(7000, [this] {
                            if (Creature* alleria = me->FindNearestCreature(NPC_LURA_ALLERIA, 100.0f, true))
                                alleria->CastSpell(me, SPELL_CLOSE_PORTAL);
                        });
                    }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_void_portalAI(creature);
    }
};

//124745
class npc_lura_great_warden : public CreatureScript
{
public:
    npc_lura_great_warden() : CreatureScript("npc_lura_great_warden") {}

    struct npc_lura_great_wardenAI : public ScriptedAI
    {
        npc_lura_great_wardenAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner) override
        {
            summoner->CastSpell(me, SPELL_CHANNEL_ADD);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, urand(8000, 10000));
            events.RescheduleEvent(EVENT_2, urand(5000, 8000));
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
                    Talk(0);
                    DoCast(SPELL_FRAGMMENT);
                    events.RescheduleEvent(EVENT_1, urand(18000, 22000));
                    break;
                case EVENT_2:
                    DoCast(SPELL_SHOCK);
                    events.RescheduleEvent(EVENT_2, urand(10000, 12000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_great_wardenAI(creature);
    }
};

//123008
class npc_lura_spells_target : public CreatureScript
{
public:
    npc_lura_spells_target() : CreatureScript("npc_lura_spells_target") {}

    struct npc_lura_spells_targetAI : public ScriptedAI
    {
        npc_lura_spells_targetAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_FRAGMMENT)
                DoCast(me, SPELL_MISILE_VISUAL);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_spells_targetAI(creature);
    }
};

//123050
class npc_lura_waning_void : public CreatureScript
{
public:
    npc_lura_waning_void() : CreatureScript("npc_lura_waning_void") {}

    struct npc_lura_waning_voidAI : public ScriptedAI
    {
        npc_lura_waning_voidAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset() override
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner) override
        {
            DoZoneInCombat();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_2, urand(5000, 8000));
        }

        void JustDied(Unit* /*killer*/) override
        {
            DoCast(me, SPELL_REMNANT_SUM, true);
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
                    DoCast(SPELL_LASHING);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_waning_voidAI(creature);
    }
};

//123054
class npc_lura_remnant_of_anguish : public CreatureScript
{
public:
    npc_lura_remnant_of_anguish() : CreatureScript("npc_lura_remnant_of_anguish") {}

    struct npc_lura_remnant_of_anguishAI : public ScriptedAI
    {
        npc_lura_remnant_of_anguishAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(me, SPELL_REMNANT_AT);
        }

        Player* PlayerSelector()
        {
            std::vector<Player*> PlayerSelector;
            PlayerSelector.clear();

            Map::PlayerList const &players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                if (Player* player = itr->getSource()->ToPlayer())
                    if (!player->isGameMaster() && player->isAlive())
                            PlayerSelector.push_back(player);
                   
            if (PlayerSelector.empty())
                return NULL;
            return Trinity::Containers::SelectRandomContainerElement(PlayerSelector);
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                if (Player* player = PlayerSelector())
                    me->CastSpell(player, SPELL_REPLACE);
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_remnant_of_anguishAI(creature);
    }
};

//125871
class npc_lura_alleria_outro : public CreatureScript
{
public:
    npc_lura_alleria_outro() : CreatureScript("npc_lura_alleria_outro") {}

    struct npc_lura_alleria_outroAI : public ScriptedAI
    {
        npc_lura_alleria_outroAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            DoCast(SPELL_TRANSFORM_VE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lura_alleria_outroAI(creature);
    }
};

//245177
class spell_lura_fragment_of_despire : public SpellScript
{
    PrepareSpellScript(spell_lura_fragment_of_despire);

    uint8 targetCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (!GetCaster() || !GetExplTargetDest())
            return;

        if (targets.empty())
            GetCaster()->CastSpell(GetExplTargetDest(), SPELL_ERUPTING, true);
        else
            targetCount = targets.size();
    }

    void Register()
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lura_fragment_of_despire::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
    }
};

//250318
class spell_lura_dark_torrent : public SpellScript
{
    PrepareSpellScript(spell_lura_dark_torrent);

    void HandleDummy(SpellEffIndex /* effIndex */)
    {
        auto target = GetHitUnit();
        if (!target)
            return;

        if (target->GetEntry() == NPC_LURA)
            if (target->IsAIEnabled)
                target->GetAI()->DoAction(2);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_lura_dark_torrent::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

//249058
class spell_lura_remnant_of_anguish : public SpellScript
{
    PrepareSpellScript(spell_lura_remnant_of_anguish);

    void HandleDummy(SpellEffIndex /* effIndex */)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        if (caster->GetEntry() == NPC_LURA_VOID_ZONE)
            if (Creature* cstr = caster->ToCreature())
                cstr->DespawnOrUnsummon(500);  
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_lura_remnant_of_anguish::HandleDummy, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};

void AddSC_boss_lura()
{
    new boss_lura();
    new npc_lura_alleria();
    new npc_lura_intro_rift_warden();
    new npc_lura_void_portal();
    new npc_lura_great_warden();
    new npc_lura_spells_target();
    new npc_lura_waning_void();
    new npc_lura_remnant_of_anguish();
    new npc_lura_alleria_outro();
    RegisterSpellScript(spell_lura_fragment_of_despire);
    RegisterSpellScript(spell_lura_dark_torrent);
    RegisterSpellScript(spell_lura_remnant_of_anguish);
}