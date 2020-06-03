#include "the_nighthold.h"
#include "GameObjectAI.h"
#include "Group.h"
#include "QuestData.h"

enum eePath
{
    PATH_FIRST_POINT = 9100409,
    PATH_FIRST_BOSS  = 9100410,
    PATH_AFTER_FIRST = 9100411,
    PATH_OPEN_DOOR   = 9100412,
    
    PATH_AFTER_SECOND = 9100414,
    PATH_AFTER_THIRD  = 9100415,
    PATH_TO_FOURTH    = 9100416
};

uint32 ePath[]
{
    PATH_FIRST_POINT,
    PATH_FIRST_BOSS,
    PATH_AFTER_FIRST,
    PATH_OPEN_DOOR,
    PATH_AFTER_SECOND,
    PATH_AFTER_THIRD,
    PATH_TO_FOURTH
};

// 110791
struct npc_nighthold_talysra_helper : ScriptedAI
{
    npc_nighthold_talysra_helper(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NON_ATTACKABLE);
        dataPath = 0;
    }

    uint32 dataPath;
    SummonList summons;

    void Reset() override
    {
        dataPath = 0;
        CallFollowToMe(true);
    }

    void LastWPReached() override
    {
        switch (dataPath)
        {
        case PATH_FIRST_BOSS:
            ZoneTalk(0);
            break;
        case PATH_OPEN_DOOR:
            ZoneTalk(3);
            me->AddDelayedEvent(13000, [this]() -> void
            {
                ZoneTalk(4);
                me->AddDelayedEvent(13000, [this]() -> void
                {
                    ZoneTalk(5);
                });
            });
            break;
        case PATH_AFTER_THIRD:
            summons.DespawnAll();
            me->DespawnOrUnsummon();
            break;
            // case PATH_TO_FOURTH:
                // me->CreateConversation();
                // break;
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_OPEN_DOOR_SECOND:
            me->AddDelayedEvent(3000, [this]() -> void
            {
                ZoneTalk(2);
            });

            me->AddDelayedEvent(6500, [this]() -> void
            {
                me->CastSpell(me, 230980);
                if (InstanceScript* instance = me->GetInstanceScript())
                    if (GameObject* go = me->GetMap()->GetGameObject(instance->GetGuidData(GO_ANOMALY_PRE)))
                        go->SetByteValue(GAMEOBJECT_FIELD_BYTES_1, 0, 0);
            });
            break;
        case ACTION_MOVE_AFTER_THIRD:
            ZoneTalk(6);
            break;
        }

        dataPath = ePath[action];
        CallFollowToMe();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePath(dataPath, false);
    }

    void MovementInform(uint32 /*type*/, uint32 /*point*/) override
    {
        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

        CallFollowToMe(true);
    }

    void CallFollowToMe(bool needsave = false)
    {
        if (summons.empty())
        {
            std::list<Creature*> guards;
            GetCreatureListWithEntryInGrid(guards, me, NPC_TALYSRA_ADDS, 60.0f);
            GetCreatureListWithEntryInGrid(guards, me, NPC_TALYSRA_ADDS_1, 60.0f);
            GetCreatureListWithEntryInGrid(guards, me, NPC_KHADGAR, 60.0f);
            if (!guards.empty())
                for (auto & guard : guards)
                {
                    if (needsave)
                        guard->SetHomePosition(guard->GetPositionX(), guard->GetPositionY(), guard->GetPositionZ(), guard->GetOrientation());

                    guard->GetMotionMaster()->Clear();
                    if (dataPath)
                        guard->GetMotionMaster()->MovePath(dataPath, false, frand(-3, 3), frand(-3, 3));
                    guard->SetReactState(REACT_PASSIVE);
                    guard->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);

                    summons.Summon(guard);
                }
        }
        else
        {
            for (auto & it : summons)
            {
                if (Creature* summon = Unit::GetCreature(*me, it))
                {
                    if (needsave)
                        summon->SetHomePosition(summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ(), summon->GetOrientation());
                    else
                    {
                        summon->GetMotionMaster()->Clear();
                        if (dataPath)
                            summon->GetMotionMaster()->MovePath(dataPath, false, frand(-3, 3), frand(-3, 3));
                        summon->SetReactState(REACT_PASSIVE);
                        summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
                    }
                }
            }
        }
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

//225665, 225673 - down
//225667, 225675 - up
class spell_suramar_portal_plr_path : public AuraScript
{
    PrepareAuraScript(spell_suramar_portal_plr_path);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
        {
            //player->GetMotionMaster()->MoveIdle();

            switch (GetId())
            {
            case 225665:
                player->NearTeleportTo(1135.44f, 4188.93f, 23.64f, 2.59f); //hack
                //player->GetMotionMaster()->MovePath(22566500, false);
                break;
            case 225667:
                //player->GetMotionMaster()->MovePath(22566700, false);
                player->NearTeleportTo(1138.89f, 4186.87f, 95.60f, 2.60f); //hack
                break;
            case 225673:
                //player->GetMotionMaster()->MovePath(22567300, false);
                player->NearTeleportTo(1082.38f, 4219.53f, 23.63f, 5.72f); //hack
                break;
            case 225675:
                //player->GetMotionMaster()->MovePath(22567500, false);
                player->NearTeleportTo(1084.13f, 4218.51f, 95.60f, 5.80f); //hack
                break;
            }
            player->RemoveAurasDueToSpell(GetId()); //hack
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_suramar_portal_plr_path::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
    }
};

//112655
struct npc_night_hold_celestial_acolyte : ScriptedAI
{
    explicit npc_night_hold_celestial_acolyte(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        DoMeleeAttackIfReady();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        scheduler
            .Schedule(Seconds(3), [this](TaskContext context)
        {
            context.Repeat();
            DoCast(224378);
        })
            .Schedule(Seconds(20), [this](TaskContext context)
        {
            context.Repeat();
            DoCast(224229);
        });
    }

private:
    TaskScheduler scheduler;
};

//112803
struct npc_night_hold_astrologer_jarin : ScriptedAI
{
    explicit npc_night_hold_astrologer_jarin(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        DoMeleeAttackIfReady();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        scheduler
            .Schedule(Seconds(3), [this](TaskContext context)
        {
            context.Repeat(Seconds(5));
            DoCast(224560);
        })
            .Schedule(Seconds(10), [this](TaskContext context)
        {
            context.Repeat();
            DoCast(224632);
        });
    }

private:
    TaskScheduler scheduler;
};

// 111225 Chaos Mage Beleron
struct npc_night_hold_chaos_mage_beleron : ScriptedAI
{
    explicit npc_night_hold_chaos_mage_beleron(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
        scheduler.SetValidator([this]
        {
            return !me->HasUnitState(UNIT_STATE_CASTING);
        });

        if(me->isAlive() && me->IsVisible())
        {
            me->AddDelayedEvent(1500, [this]() -> void
            {
                Creature* nearest = nullptr;

                std::list<Creature*> creatureList;
                GetCreatureListWithEntryInGrid(creatureList, me, 111581, 70.0f);
                for (auto creature : creatureList)
                    if (!nearest || creature->GetPositionZ() >= 5.0f)
                        nearest = creature;

                me->CastSpell(nearest, SpellEmpowering);
            });
        }
    }

    enum Spells
    {
        SpellChaoticEnergies = 221464,
        SpellEmpowering = 222192,
        SpellEmpowering_2 = 222200,
    };

    void EnterCombat(Unit* attacker) override
    {
        me->CastStop();
        me->RemoveAura(SpellEmpowering);
        
        std::list<Creature*> creatureList;
        GetCreatureListWithEntryInGrid(creatureList, me, 111581, 90.0f);
        for (auto creature : creatureList)
        {
            creature->RemoveAura(SpellEmpowering);
            creature->RemoveAura(SpellEmpowering_2);
        }

        scheduler.Schedule(Seconds(10), [this](TaskContext context)
        {
            context.Repeat();
            DoCast(SpellChaoticEnergies);
        });

        if (auto summonerXiv = instance->GetCreature(DATA_SUMMONER_XIV))
            summonerXiv->AI()->AttackStart(attacker);
        if (auto felweaverPharamere = instance->GetCreature(DATA_FELWEAVER_PHARAMERE))
            felweaverPharamere->AI()->AttackStart(attacker);

    }

    void JustDied(Unit* killer) override
    {
        scheduler.CancelAll();
        ScriptedAI::JustDied(killer);

        if (!instance)
            return;

        auto summonerXiv = instance->GetCreature(DATA_SUMMONER_XIV);
        auto felweaverPharamere = instance->GetCreature(DATA_FELWEAVER_PHARAMERE);

        if (summonerXiv && summonerXiv->isDead() && felweaverPharamere && felweaverPharamere->isDead())
            instance->SetData(DATA_KROSUS_INTRO_TRASH, DONE);
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
        scheduler.CancelAll();

        if (!instance)
            return;

        auto summonerXiv = instance->GetCreature(DATA_SUMMONER_XIV);
        auto felweaverPharamere = instance->GetCreature(DATA_FELWEAVER_PHARAMERE);

        if (summonerXiv && felweaverPharamere)
        {
            instance->SetData(DATA_KROSUS_INTRO_TRASH, FAIL);
            // summonerXiv->AI()->EnterEvadeMode();
            // felweaverPharamere->AI()->EnterEvadeMode();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        scheduler.Update(diff, std::bind(&ScriptedAI::DoMeleeAttackIfReady, this, SPELL_SCHOOL_MASK_NORMAL));
    }

private:
    InstanceScript * instance;
    TaskScheduler scheduler;
};

// 111226 Summoner Xiv
struct npc_night_hold_summoner_xiv : ScriptedAI
{
    npc_night_hold_summoner_xiv(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
        scheduler.SetValidator([this]
        {
            return !me->HasUnitState(UNIT_STATE_CASTING);
        });

        if(me->isAlive() && me->IsVisible())
        {
            me->AddDelayedEvent(1500, [this]() -> void
            {

                Creature* nearest = nullptr;

                std::list<Creature*> creatureList;
                GetCreatureListWithEntryInGrid(creatureList, me, 111581, 70.0f);
                for (auto creature : creatureList)
                    if (!nearest || creature->GetPositionZ() >= 5.0f)
                        nearest = creature;

                me->CastSpell(nearest, SpellEmpowering);
            });
        }
    }

    enum Spells
    {
        SpellFelBlast = 222156,
        SpellEmpowering = 222192,
        SpellEmpowering_2 = 222200,
    };

    void EnterCombat(Unit* attacker) override
    {
        me->CastStop();
        me->RemoveAura(SpellEmpowering);
        
        std::list<Creature*> creatureList;
        GetCreatureListWithEntryInGrid(creatureList, me, 111581, 90.0f);
        for (auto creature : creatureList)
        {
            creature->RemoveAura(SpellEmpowering);
            creature->RemoveAura(SpellEmpowering_2);
        }
        scheduler.Schedule(Seconds(3), [this](TaskContext context)
        {
            context.Repeat();
            DoCast(SpellFelBlast);
        });

        if (auto chaosMageBelorn = instance->GetCreature(DATA_CHAOS_MAGE_BELORN))
            chaosMageBelorn->AI()->AttackStart(attacker);
        if (auto felweaverPharamere = instance->GetCreature(DATA_FELWEAVER_PHARAMERE))
            felweaverPharamere->AI()->AttackStart(attacker);
    }

    void JustDied(Unit* killer) override
    {
        scheduler.CancelAll();
        ScriptedAI::JustDied(killer);

        if (!instance)
            return;

        auto chaosMageBelorn = instance->GetCreature(DATA_CHAOS_MAGE_BELORN);
        auto felweaverPharamere = instance->GetCreature(DATA_FELWEAVER_PHARAMERE);

        if (chaosMageBelorn && chaosMageBelorn->isDead() && felweaverPharamere && felweaverPharamere->isDead())
            instance->SetData(DATA_KROSUS_INTRO_TRASH, DONE);
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
        scheduler.CancelAll();

        if (!instance)
            return;

        auto chaosMageBelorn = instance->GetCreature(DATA_CHAOS_MAGE_BELORN);
        auto felweaverPharamere = instance->GetCreature(DATA_FELWEAVER_PHARAMERE);

        if (chaosMageBelorn && felweaverPharamere)
        {
            instance->SetData(DATA_KROSUS_INTRO_TRASH, FAIL);
            // chaosMageBelorn->AI()->EnterEvadeMode();
            // felweaverPharamere->AI()->EnterEvadeMode();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        scheduler.Update(diff, std::bind(&ScriptedAI::DoMeleeAttackIfReady, this, SPELL_SCHOOL_MASK_NORMAL));
    }

private:
    InstanceScript * instance;
    TaskScheduler scheduler;
};

// 111227 Felweaver Pharamere
struct npc_night_hold_felweaver_pharamere : ScriptedAI
{
    npc_night_hold_felweaver_pharamere(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
        scheduler.SetValidator([this]
        {
            return !me->HasUnitState(UNIT_STATE_CASTING);
        });

        if(me->isAlive() && me->IsVisible())
        {
            me->AddDelayedEvent(1500, [this]() -> void
            {
                Creature* nearest = nullptr;

                std::list<Creature*> creatureList;
                GetCreatureListWithEntryInGrid(creatureList, me, 111581, 70.0f);
                for (auto creature : creatureList)
                    if (!nearest || creature->GetPositionZ() >= 5.0f)
                        nearest = creature;

                me->CastSpell(nearest, SpellEmpowering);
            });
        }
    }

    enum Spells
    {
        SpellInfernalBlade = 222078,
        SpellEmpowering = 222192,
        SpellEmpowering_2 = 222200,
    };

    void EnterCombat(Unit* attacker) override
    {
        me->CastStop();
        me->RemoveAura(SpellEmpowering);
        std::list<Creature*> creatureList;
        GetCreatureListWithEntryInGrid(creatureList, me, 111581, 90.0f);
        for (auto creature : creatureList)
        {
            creature->RemoveAura(SpellEmpowering);
            creature->RemoveAura(SpellEmpowering_2);
        }

        scheduler.Schedule(Milliseconds(10), [this](TaskContext context)
        {
            context.Repeat();
            me->AddAura(SpellInfernalBlade, me);
        });

        if (auto chaosMageBelorn = instance->GetCreature(DATA_CHAOS_MAGE_BELORN))
            chaosMageBelorn->AI()->AttackStart(attacker);
        if (auto summonerXiv = instance->GetCreature(DATA_SUMMONER_XIV))
            summonerXiv->AI()->AttackStart(attacker);
    }

    void JustDied(Unit* killer) override
    {
        scheduler.CancelAll();
        ScriptedAI::JustDied(killer);

        if (!instance)
            return;

        auto chaosMageBelorn = instance->GetCreature(DATA_CHAOS_MAGE_BELORN);
        auto summonerXiv = instance->GetCreature(DATA_SUMMONER_XIV);

        if (chaosMageBelorn && chaosMageBelorn->isDead() && summonerXiv && summonerXiv->isDead())
            instance->SetData(DATA_KROSUS_INTRO_TRASH, DONE);
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
        scheduler.CancelAll();

        if (!instance)
            return;

        auto chaosMageBelorn = instance->GetCreature(DATA_CHAOS_MAGE_BELORN);
        auto summonerXiv = instance->GetCreature(DATA_SUMMONER_XIV);

        if (chaosMageBelorn && summonerXiv)
        {
            instance->SetData(DATA_KROSUS_INTRO_TRASH, FAIL);
            chaosMageBelorn->AI()->EnterEvadeMode();
            summonerXiv->AI()->EnterEvadeMode();
        }
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SpellEmpowering && me->isAlive() && me->IsVisible())
        {
            Creature* nearest = nullptr;

            std::list<Creature*> creatureList;
            GetCreatureListWithEntryInGrid(creatureList, me, 111581, 90.0f);
            for (auto creature : creatureList)
                if (!nearest || creature->GetPositionZ() <= 5.0f)
                    nearest = creature;

            if (auto flameCore = target->FindNearestCreature(111581, 60.0f))
                flameCore->CastSpell(nearest, SpellEmpowering_2);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        scheduler.Update(diff, std::bind(&ScriptedAI::DoMeleeAttackIfReady, this, SPELL_SCHOOL_MASK_NORMAL));
    }

private:
    InstanceScript * instance;
    TaskScheduler scheduler;
};

// 111581
struct npc_night_hold_flame_core : ScriptedAI
{
    enum Spells
    {
        SpellEmpowering = 222192,
        SpellEmpowering_2 = 222200,
    };

    explicit npc_night_hold_flame_core(Creature* creature) : ScriptedAI(creature)
    {
        me->AddDelayedEvent(1500, [this]() -> void
        {
            Creature* nearest = nullptr;

            std::list<Creature*> creatureList;
            GetCreatureListWithEntryInGrid(creatureList, me, 111581, 70.0f);
            for (auto creature : creatureList)
                if (!nearest || creature->GetPositionZ() < 1.0f)
                    nearest = creature;

            me->CastSpell(nearest, SpellEmpowering_2);
            me->CastSpell(nearest, SpellEmpowering);
        });
    }
};

// 110677
struct npc_night_hold_image_of_khadgar : ScriptedAI
{
    explicit npc_night_hold_image_of_khadgar(Creature* creature) : ScriptedAI(creature) {}

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 /*action*/) override
    {
        me->CastSpell(player, 220070);
        player->GetMotionMaster()->MoveIdle();
        player->GetMotionMaster()->MovePath(370434, false);
        player->AddDelayedEvent(13000, [player]() -> void
        {
            player->RemoveAurasDueToSpell(220071);
        });
    }
};


// 116670, 116819, 116662, 116667, 116820
class npc_nighthold_portal : public CreatureScript
{
public:
    npc_nighthold_portal() : CreatureScript("npc_nighthold_portal") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        InstanceScript* instance = creature->GetInstanceScript();
        
        if (!instance)
            return false;
        
        player->ADD_GOSSIP_ITEM_DB(20697, 3, GOSSIP_SENDER_MAIN, 2); // start
        
        if (instance->GetBossState(DATA_ALURIEL) == DONE)
        {
            player->ADD_GOSSIP_ITEM_DB(20697, 0, GOSSIP_SENDER_MAIN, 1);
            player->ADD_GOSSIP_ITEM_DB(20697, 4, GOSSIP_SENDER_MAIN, 3);
        }
        
        bool check = true;
        for (uint8 i =0; i < DATA_ELISANDE; ++i)
            if (instance->GetBossState(i) != DONE)
                check = false;

        if (!check)
        {
            const auto& players = creature->GetMap()->GetPlayers();
            if (Player* player = players.getFirst()->getSource())
            {
                Player* leader = nullptr;
                if (Group* group = player->GetGroup())
                {
                    if (Player* _leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
                        leader = _leader;
                }
                
                if(!leader)
                    leader = player;

                if ((creature->GetMap()->IsMythicRaid() && leader->IsQuestRewarded(45383)) ||
                    (creature->GetMap()->IsHeroicRaid() && leader->IsQuestRewarded(45382)) ||
                    (creature->GetMap()->IsNormalRaid() && leader->IsQuestRewarded(45381)))
                    check = true;
            }
        }
            
        if (check)
        {
            player->ADD_GOSSIP_ITEM_DB(20697, 5, GOSSIP_SENDER_MAIN, 4);
            player->ADD_GOSSIP_ITEM_DB(20697, 1, GOSSIP_SENDER_MAIN, 5);
        }
        
        if (instance->GetBossState(DATA_ELISANDE) == DONE)
            player->ADD_GOSSIP_ITEM_DB(20697, 2, GOSSIP_SENDER_MAIN, 6);
        
        
        player->SEND_GOSSIP_MENU(31058, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        uint32 graveyardId = 0;
        Position pos{};
        switch(action)
        {
            case 1:
                graveyardId = 5809;
                break;
            case 2:
                graveyardId = 5338;
                break;
            case 3:
                graveyardId = 5536;
                break;
            case 4:
                pos = {332.77f, 3084.27f, 289.41f, 2.23f};
                break;
            case 5:
                pos = {362.21f, 3056.37f, 216.16f, 2.40f};
                break;
            case 6:
                graveyardId = 5537;
                break;
            default:
                break;;
        }
        if (graveyardId)
        {
            if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(graveyardId))
                player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, entry->Loc.O * M_PI / 180.0f);
        }
        else
            player->TeleportTo(1530, &pos);
        
        
        player->CLOSE_GOSSIP_MENU();

        return true;
    }
};


// 254241
struct go_nighthold_gate_event : public GameObjectAI
{
    go_nighthold_gate_event(GameObject* go) : GameObjectAI(go) {}
    
    bool eventDone = false;
    
    void OnStateChanged(uint32 state, Unit* unit) override
    {
        if (eventDone)
            return;
        
        if (!unit)
            return;
        
        InstanceScript* instance = go->GetInstanceScript();
        if (!instance)
            return;
        
        if (Creature* talysra = go->GetMap()->GetCreature(instance->GetGuidData(NPC_TALYSRA)))
            talysra->AI()->DoAction(ACTION_MOVE_TO_FOURTH);
        
        go->AddDelayedEvent(1000, [this] () -> void
        {
            std::list<Creature*> adds;
            GetCreatureListWithEntryInGrid(adds, go, 112660, 20.0f);
            uint8 text = 0;
            for (auto & guard : adds)
            {
                guard->SetFacingToObject(go);
                if (text < 2)
                    guard->AI()->Talk(text++);

                guard->AddDelayedEvent(2000, [guard] () -> void
                {
                    guard->GetMotionMaster()->MovePoint(0, 568.30f, 3286.02f, 109.36f);
                    guard->DespawnOrUnsummon(7000);
                });
            }
        });
            

        go->AddDelayedEvent(2000, [this] () -> void
        {
            std::list<Creature*> adds;
            
            GetCreatureListWithEntryInGrid(adds, go, 113307, 110.0f);
            GetCreatureListWithEntryInGrid(adds, go, 112675, 110.0f);
            GetCreatureListWithEntryInGrid(adds, go, 112671, 110.0f);
            GetCreatureListWithEntryInGrid(adds, go, 112665, 110.0f);
            
            for (auto & guard : adds)
            {
                if (guard->GetPositionZ() <= 113.39f)
                    guard->GetMotionMaster()->MovePoint(0, 561.89f, 3207.92f, 109.36f);
            }
        });
        
        go->AddDelayedEvent(8000, [this] () -> void
        {
            if (Creature* add = go->FindNearestCreature(112660, 60.0f, true))
                add->AI()->ZoneTalk(2);
        });
    }

};

// 221356
class spell_nighthold_smash_dest_SpellScript : public SpellScript
{
    PrepareSpellScript(spell_nighthold_smash_dest_SpellScript);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        WorldLocation* dest = GetHitDest();
            
        if (caster && dest)
            caster->CastSpell(dest, 221354, true);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_nighthold_smash_dest_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SMASH_DEST);
    }
};

class spell_vantus_rune_the_nighthold : public SpellScriptLoader
{
public:
    spell_vantus_rune_the_nighthold() : SpellScriptLoader("spell_vantus_rune_the_nighthold") {}

    class spell_vantus_rune_the_nighthold_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vantus_rune_the_nighthold_AuraScript);

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
                case 192767:
                    if (instance->GetBossState(DATA_SKORPYRON) == IN_PROGRESS && !player->HasAura(208850))
                        player->CastSpell(player, 208850, false);
                    break;
                case 192768:
                    if (instance->GetBossState(DATA_ANOMALY) == IN_PROGRESS && !player->HasAura(208851))
                        player->CastSpell(player, 208851, false);
                    break;
                case 192769:
                    if (instance->GetBossState(DATA_TRILLIAX) == IN_PROGRESS && !player->HasAura(208852))
                        player->CastSpell(player, 208852, false);
                    break;
                case 192770:
                    if (instance->GetBossState(DATA_ALURIEL) == IN_PROGRESS && !player->HasAura(208853))
                        player->CastSpell(player, 208853, false);
                    break;
                case 192774:
                    if (instance->GetBossState(DATA_ETRAEUS) == IN_PROGRESS && !player->HasAura(208857))
                        player->CastSpell(player, 208857, false);
                    break;
                case 192772:
                    if (instance->GetBossState(DATA_TELARN) == IN_PROGRESS && !player->HasAura(208855))
                        player->CastSpell(player, 208855, false);
                    break;
                case 192773:
                    if (instance->GetBossState(DATA_KROSUS) == IN_PROGRESS && !player->HasAura(208856))
                        player->CastSpell(player, 208856, false);
                    break;
                case 192771:
                    if (instance->GetBossState(DATA_TICHONDRIUS) == IN_PROGRESS && !player->HasAura(208854))
                        player->CastSpell(player, 208854, false);
                    break;
                case 192775:
                    if (instance->GetBossState(DATA_ELISANDE) == IN_PROGRESS && !player->HasAura(208858))
                        player->CastSpell(player, 208858, false);
                    break;
                case 192776:
                    if (instance->GetBossState(DATA_GULDAN) == IN_PROGRESS && !player->HasAura(208859))
                        player->CastSpell(player, 208859, false);
                    break;
                }
            }
            else
                checkOnProc -= diff;

            if (checkOnRemove <= diff)
            {
                if (player->HasAura(208850))
                {
                    if (instance->GetBossState(DATA_SKORPYRON) == DONE || instance->GetBossState(DATA_SKORPYRON) == NOT_STARTED)
                    {
                        player->RemoveAura(208850);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208851))
                {
                    if (instance->GetBossState(DATA_ANOMALY) == DONE || instance->GetBossState(DATA_ANOMALY) == NOT_STARTED)
                    {
                        player->RemoveAura(208851);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208852))
                {
                    if (instance->GetBossState(DATA_TRILLIAX) == DONE || instance->GetBossState(DATA_TRILLIAX) == NOT_STARTED)
                    {
                        player->RemoveAura(208852);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208853))
                {
                    if (instance->GetBossState(DATA_ALURIEL) == DONE || instance->GetBossState(DATA_ALURIEL) == NOT_STARTED)
                    {
                        player->RemoveAura(208853);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208856))
                {
                    if (instance->GetBossState(DATA_ETRAEUS) == DONE || instance->GetBossState(DATA_ETRAEUS) == NOT_STARTED)
                    {
                        player->RemoveAura(208856);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208855))
                {
                    if (instance->GetBossState(DATA_TELARN) == DONE || instance->GetBossState(DATA_TELARN) == NOT_STARTED)
                    {
                        player->RemoveAura(208855);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208856))
                {
                    if (instance->GetBossState(DATA_KROSUS) == DONE || instance->GetBossState(DATA_KROSUS) == NOT_STARTED)
                    {
                        player->RemoveAura(208856);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208854))
                {
                    if (instance->GetBossState(DATA_TICHONDRIUS) == DONE || instance->GetBossState(DATA_TICHONDRIUS) == NOT_STARTED)
                    {
                        player->RemoveAura(208854);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208858))
                {
                    if (instance->GetBossState(DATA_ELISANDE) == DONE || instance->GetBossState(DATA_ELISANDE) == NOT_STARTED)
                    {
                        player->RemoveAura(208858);
                        checkOnProc = 1000;
                        checkOnRemove = 1000;
                    }
                }

                if (player->HasAura(208858))
                {
                    if (instance->GetBossState(DATA_GULDAN) == DONE || instance->GetBossState(DATA_GULDAN) == NOT_STARTED)
                    {
                        player->RemoveAura(208859);
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
            OnEffectUpdate += AuraEffectUpdateFn(spell_vantus_rune_the_nighthold_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_vantus_rune_the_nighthold_AuraScript();
    }

    class spell_vantus_rune_the_nighthold_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_vantus_rune_the_nighthold_SpellScript);

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
            OnCheckCast += SpellCheckCastFn(spell_vantus_rune_the_nighthold_SpellScript::CheckCast);
            OnHit += SpellHitFn(spell_vantus_rune_the_nighthold_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_vantus_rune_the_nighthold_SpellScript();
    }
};

void AddSC_the_nighthold()
{
    RegisterCreatureAI(npc_nighthold_talysra_helper);
    RegisterAuraScript(spell_suramar_portal_plr_path);
    RegisterCreatureAI(npc_night_hold_celestial_acolyte);
    RegisterCreatureAI(npc_night_hold_astrologer_jarin);
    RegisterCreatureAI(npc_night_hold_chaos_mage_beleron);
    RegisterCreatureAI(npc_night_hold_summoner_xiv);
    RegisterCreatureAI(npc_night_hold_felweaver_pharamere);
    RegisterCreatureAI(npc_night_hold_image_of_khadgar);
    new npc_nighthold_portal();
    new spell_vantus_rune_the_nighthold();
    RegisterGameObjectAI(go_nighthold_gate_event);
}
