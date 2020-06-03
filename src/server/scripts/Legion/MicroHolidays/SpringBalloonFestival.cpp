/*
*/

enum eSay
{
    // GENERAL
    SAY_NEED_PLAYERS              = 0,

    // DALARAN
    SAY_LINE_CHENS_1              = 0,
    SAY_LINE_CHENS_2,
    SAY_LINE_CHENS_3,
    SAY_LINE_CHENS_4,
    SAY_LINE_CHENS_5,
    SAY_LINE_CHENS_6,
    SAY_LINE_CHENS_7,
    SAY_LINE_CHENS_8,
    SAY_LINE_CHENS_9,
    SAY_LINE_CHENS_10,
    SAY_LINE_CHENS_11,

    // ULDUM
    SAY_LINE_SCHNOTTZ_1           = 0,
    SAY_LINE_SCHNOTTZ_2,
    SAY_LINE_SCHNOTTZ_3,
    SAY_LINE_SCHNOTTZ_4,
    SAY_LINE_SCHNOTTZ_5,
    SAY_LINE_SCHNOTTZ_6,
    SAY_LINE_SCHNOTTZ_7,
    SAY_LINE_SCHNOTTZ_8,
    SAY_LINE_BALLOON              = 0,

    // PANDARIA
    SAY_LINE_LIN_CLOUDWALKER_1    = 0,
    SAY_LINE_LIN_CLOUDWALKER_2,
    SAY_LINE_LIN_CLOUDWALKER_3,
    SAY_LINE_LIN_CLOUDWALKER_4,
    SAY_LINE_LIN_CLOUDWALKER_5,
    SAY_LINE_LIN_CLOUDWALKER_6,
    SAY_LINE_LIN_CLOUDWALKER_7,
    SAY_LINE_LIN_CLOUDWALKER_8,
    SAY_LINE_LIN_CLOUDWALKER_9,
    SAY_LINE_LIN_CLOUDWALKER_10,
    SAY_LINE_LIN_CLOUDWALKER_11,
    SAY_LINE_LIN_CLOUDWALKER_12,
    SAY_LINE_LIN_CLOUDWALKER_13,
    SAY_LINE_LIN_CLOUDWALKER_14,
    SAY_LINE_LIN_CLOUDWALKER_15,
    SAY_LINE_LIN_CLOUDWALKER_16,

    // AZSUNA
    SAY_LINE_BOSCOE_1             = 0,
    SAY_LINE_BOSCOE_2,
    SAY_LINE_BOSCOE_3,
    SAY_LINE_BOSCOE_4,
    SAY_LINE_BOSCOE_5,
    SAY_LINE_BOSCOE_6,
    SAY_LINE_BOSCOE_7,
    SAY_LINE_BOSCOE_8,
    SAY_LINE_BOSCOE_9,
    SAY_LINE_RUNGLE_1             = 0,
    SAY_LINE_RUNGLE_2,
    SAY_LINE_RUNGLE_3,
    SAY_LINE_RUNGLE_4,
    SAY_LINE_RUNGLE_5,
    SAY_LINE_RUNGLE_6,

    // SURAMAR
    SAY_LINE_ZANG_1               = 0,
    SAY_LINE_ZANG_2,
    SAY_LINE_ZANG_3,
    SAY_LINE_ZANG_4,
    SAY_LINE_ZANG_5,
    SAY_LINE_ZANG_6,
    SAY_LINE_ZANG_7,
    SAY_LINE_ZANG_8,
    SAY_LINE_ZANG_9,
    SAY_LINE_ZANG_10,
    SAY_LINE_ZANG_11,
    SAY_LINE_ZANG_12,
    SAY_LINE_ZANG_13,
    SAY_LINE_ZANG_14,
    SAY_LINE_ZANG_15,
    SAY_LINE_ZANG_16,
    SAY_LINE_ZANG_17,
    SAY_LINE_ZANG_18,

    // HIGHMOUNTAINS
    SAY_LINE_HEMET_NESINGWARY_1    = 0,
    SAY_LINE_HEMET_NESINGWARY_2,
    SAY_LINE_HEMET_NESINGWARY_3,
    SAY_LINE_HEMET_NESINGWARY_4,
    SAY_LINE_HEMET_NESINGWARY_5,
    SAY_LINE_HEMET_NESINGWARY_6,
    SAY_LINE_HEMET_NESINGWARY_7,
    SAY_LINE_HEMET_NESINGWARY_8,
    SAY_LINE_HEMET_NESINGWARY_9,
    SAY_LINE_HEMET_NESINGWARY_10,
    SAY_LINE_HEMET_NESINGWARY_11,
    SAY_LINE_HEMET_NESINGWARY_12,
    SAY_LINE_HEMET_NESINGWARY_13,
    SAY_LINE_HEMET_NESINGWARY_14,

    // STORMHEIM
    SAY_LINE_EMI_LAN_1             = 0,
    SAY_LINE_EMI_LAN_2,
    SAY_LINE_EMI_LAN_3,
    SAY_LINE_EMI_LAN_4,
    SAY_LINE_EMI_LAN_5,
    SAY_LINE_EMI_LAN_6,
    SAY_LINE_EMI_LAN_7,
    SAY_LINE_EMI_LAN_8,
    SAY_LINE_EMI_LAN_9,
    SAY_LINE_EMI_LAN_10,
    SAY_LINE_EMI_LAN_11,
};

enum eCreatures
{
    // DALARAN
    NPC_CHENS                       = 118470,
    NPC_CHENS_VH                    = 118354,

    // ULDUM
    NPC_SCHNOTTZ                    = 118482,
    NPC_SCHNOTTZ_VH                 = 118340,

    // PANDARIA
    NPC_LIN_CLOUDWALKER             = 118474,
    NPC_LIN_CLOUDWALKER_VH          = 118355,

    // AZSUNA
    NPC_BOSCOE                      = 118467,
    NPC_BOSCOE_VH                   = 118351,
    NPC_RUNGLE                      = 118468,
    NPC_RUNGLE_VH                   = 118352,

    // SURAMAR
    NPC_ZANG_CLOUDWALKER            = 118436,
    NPC_ZANG_CLOUDWALKER_VH         = 118357,

    // HIGHMOUNTAINS
    NPC_HEMET_NESINGWARY            = 94409,
    NPC_HEMET_NESINGWARY_VH         = 118353,

    // STORMHEIM
    NPC_EMI_LAN                     = 118443,
    NPC_EMI_LAN_VH                  = 118356
};

enum eSpells
{
    // GENERAL
    SPELL_BALLOON_FIND_PLAYERS              = 234944,
    SPELL_WAINTING_FOR_BALLOON              = 234941,
    SPELL_RIDE_BALLOON                      = 234947,

    // ULDUM
    SPELL_SCHNOTTZ_HAS_TO_GO_OR             = 236349,
    SPELL_SCHNOTTZ_HAS_TO_GO                = 236346,

    // AZSUNA
    SPELL_JUNIOR_BALLONER_PREPAREDNESS_KIT  = 236323
};

struct npc_chens : ScriptedAI
{
    npc_chens(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118365, 5259.373f, -668.9254f, 163.235f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118365, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
                timer = 2000;
            }
            else
                timer -= diff;
        }
    }
};

struct npc_scgnottz : ScriptedAI
{
    npc_scgnottz(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118297, -9516.056f, -1008.158f, 102.724f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118297, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
            }
            timer = 2000;
        }
        else
            timer -= diff;
    }
};

struct npc_lin_cloudwalker : ScriptedAI
{
    npc_lin_cloudwalker(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118366, 547.6684f, -653.8698f, 257.4579f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118366, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
                timer = 2000;
            }
            else
                timer -= diff;
        }
    }
};


struct npc_rungle : ScriptedAI
{
    npc_rungle(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118363, 470.5434f, 6518.068f, 39.03863f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118363, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
                timer = 2000;
            }
            else
                timer -= diff;
        }
    }
};

struct npc_zang_cloudwalker : ScriptedAI
{
    npc_zang_cloudwalker(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118359, 1661.786f, 4788.782f, 138.8493f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118359, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
                timer = 2000;
            }
            else
                timer -= diff;
        }
    }
};

struct npc_hemet_nesingwary : ScriptedAI
{
    npc_hemet_nesingwary(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118364, 4506.38f, 4856.669f, 662.1934f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118364, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
                timer = 2000;
            }
            else
                timer -= diff;
        }
    }
};

struct npc_emi_lan : ScriptedAI
{
    npc_emi_lan(Creature* creature) : ScriptedAI(creature) {}

    uint32 timerAuraCheck = 1000;
    uint32 timer = 2000;
    std::list<ObjectGuid> guidList;
    EventMap events;

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 1)
        {
            if (!player->HasAura(SPELL_WAINTING_FOR_BALLOON))
            {
                Talk(SAY_NEED_PLAYERS, player->GetGUID());
                player->CastSpell(player, SPELL_WAINTING_FOR_BALLOON, false);
            }
            player->CLOSE_GOSSIP_MENU();
        }
    }

    void SpellHit(Unit* owner, SpellInfo const* spell) override
    {
        if (spell->Id == 234941)
            guidList.push_back(owner->GetGUID());
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == 1)
            guidList.remove(guid);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (timerAuraCheck)
        {
            if (timerAuraCheck <= diff)
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 60.0f);
                for (auto player : playerList)
                    if (!player->IsInDist2d(me, 20.0f) && player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                        player->RemoveAurasDueToSpell(SPELL_WAINTING_FOR_BALLOON);

                timerAuraCheck = 1000;
            }
            else
                timerAuraCheck -= diff;
        }

        if (timer)
        {
            if (timer <= diff)
            {
                if (!guidList.empty())
                {
                    if (guidList.size() >= 3)
                    {
                        uint8 i = 0;
                        std::list<ObjectGuid> guidTemp = guidList;
                        guidTemp.resize(3);
                        me->SummonCreature(118367, 3838.949f, 2060.948f, 254.2826f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN, 0);

                        for (auto& guid : guidTemp)
                        {
                            if (++i <= 3)
                            {
                                if (auto g_player = ObjectAccessor::GetPlayer(*me, guid))
                                {
                                    if (g_player->HasAura(SPELL_WAINTING_FOR_BALLOON))
                                    {
                                        g_player->AddDelayedEvent(i * 400 + 1, [g_player]()-> void
                                        {
                                            if (auto balloon = g_player->FindNearestCreature(118367, 40.0f))
                                                g_player->CastSpell(balloon, 234947, true);
                                        });
                                    }
                                }
                                guidList.remove(guid);
                            }
                        }
                    }
                }
                timer = 2000;
            }
            else
                timer -= diff;
        }
    }
};

// Dalaran
struct npc_chens_vh : ScriptedAI
{
    npc_chens_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_CHENS_1);
                events.RescheduleEvent(EVENT_2, 6000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_CHENS_2);
                events.RescheduleEvent(EVENT_3, 7000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_CHENS_3);
                events.RescheduleEvent(EVENT_4, 8000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_CHENS_4);
                events.RescheduleEvent(EVENT_5, 8000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_CHENS_5);
                events.RescheduleEvent(EVENT_6, 8000);
                break;
            case EVENT_6:
                Talk(SAY_LINE_CHENS_6);
                events.RescheduleEvent(EVENT_7, 6000);
                break;
            case EVENT_7:
                Talk(SAY_LINE_CHENS_7);
                events.RescheduleEvent(EVENT_8, 9000);
                break;
            case EVENT_8:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_1);
                events.RescheduleEvent(EVENT_9, 35000);
                break;
            case EVENT_9:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_2);
                events.RescheduleEvent(EVENT_10, 33000);
                break;
            case EVENT_10:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_3);
                events.RescheduleEvent(EVENT_11, 35000);
                break;
            case EVENT_11:
                Talk(SAY_LINE_CHENS_11);
                break;
            }
        }
    }
};

struct npc_chens_balloon : ScriptedAI
{
    npc_chens_balloon(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> playerList;
    ObjectGuid FirstPlayer;
    ObjectGuid SecondPlayer;
    ObjectGuid ThirdPlayer;
    ObjectGuid Chens;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (apply && who->IsPlayer())
            playerList.push_back(who->GetGUID());
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_CHENS_VH)
            Chens = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 7)
        {
            DoCast(65785);
            me->DespawnOrUnsummon(500);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (auto chens = ObjectAccessor::GetUnit(*me, Chens))
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    chens->ToCreature()->AI()->Talk(SAY_LINE_CHENS_8, target->GetGUID());
            break;
        case ACTION_2:
            if (auto chens = ObjectAccessor::GetUnit(*me, Chens))
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    chens->ToCreature()->AI()->Talk(SAY_LINE_CHENS_9, target->GetGUID());
            break;
        case ACTION_3:
            if (auto chens = ObjectAccessor::GetUnit(*me, Chens))
                if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    chens->ToCreature()->AI()->Talk(SAY_LINE_CHENS_10, target->GetGUID());
            break;
        default:
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
                uint8 i = 0;
                if (!playerList.empty())
                {
                    for (auto& player : playerList)
                    {
                        me->AddPlayerInPersonnalVisibilityList(player);
                        if (auto chens = ObjectAccessor::GetUnit(*me, Chens))
                            chens->AddPlayerInPersonnalVisibilityList(player);

                        if (++i <= 3)
                        {
                            switch (i)
                            {
                            case 1:
                                if (playerList.size() >= 1)
                                    FirstPlayer = player;
                                break;
                            case 2:
                                if (playerList.size() >= 2)
                                    SecondPlayer = player;
                                break;
                            case 3:
                                if (playerList.size() >= 3)
                                    ThirdPlayer = player;
                                break;
                            }
                        }
                    }
                }
                me->GetMotionMaster()->MovePath(118365, false);
                break;
            }
        }
    }
};

//Uldum
struct npc_scgnottz_vh : ScriptedAI
{
    npc_scgnottz_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            Talk(SAY_LINE_SCHNOTTZ_8);
            me->DespawnOrUnsummon(1000);
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
                Talk(SAY_LINE_SCHNOTTZ_1);
                events.RescheduleEvent(EVENT_2, 6000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_SCHNOTTZ_2);
                events.RescheduleEvent(EVENT_3, 5000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_SCHNOTTZ_3);
                events.RescheduleEvent(EVENT_4, 6000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_SCHNOTTZ_4);
                events.RescheduleEvent(EVENT_5, 5000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_SCHNOTTZ_5);
                events.RescheduleEvent(EVENT_6, 6000);
                break;
            case EVENT_6:
                Talk(SAY_LINE_SCHNOTTZ_6);
                events.RescheduleEvent(EVENT_7, 8000);
                break;
            case EVENT_7:
                Talk(SAY_LINE_SCHNOTTZ_7);
                break;
            }
        }
    }
};

struct npc_scgnottz_ballon : ScriptedAI
{
    npc_scgnottz_ballon(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> playerList;
    uint32 vote;
    uint32 countPlayer;
    ObjectGuid Schnottz;

    void IsSummonedBy(Unit* /*who*/) override
    {
        countPlayer = 0;
        vote = 0;
        events.RescheduleEvent(EVENT_1, 43000);
        events.RescheduleEvent(EVENT_2, 2000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (who->IsPlayer())
        {
            if (apply)
                playerList.push_back(who->GetGUID());
            else
                playerList.remove(who->GetGUID());
        }
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_SCHNOTTZ_VH)
            Schnottz = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 5)
        {
            DoCast(65785);
            me->DespawnOrUnsummon(500);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            vote++;
            countPlayer = playerList.size();
            if (vote == countPlayer)
            {
                Talk(SAY_LINE_BALLOON);
                if (auto schnottz = me->FindNearestCreature(NPC_SCHNOTTZ_VH, 5.0f))
                    schnottz->GetAI()->DoAction(ACTION_1);
            }
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
                if (!playerList.empty())
                {
                    for (auto& target : playerList)
                    {
                        if (auto player = ObjectAccessor::GetPlayer(*me, target))
                            player->AddAura(SPELL_SCHNOTTZ_HAS_TO_GO_OR, player);
                    }
                }
                break;
            case EVENT_2:
                if (!playerList.empty())
                {
                    for (auto& player : playerList)
                    {
                        me->AddPlayerInPersonnalVisibilityList(player);
                        if (auto schnottz = ObjectAccessor::GetUnit(*me, Schnottz))
                            schnottz->AddPlayerInPersonnalVisibilityList(player);
                    }
                }
                me->GetMotionMaster()->MovePath(118297, false);
                break;
            }
        }
    }
};

//Pandaria
struct npc_lin_cloudwalker_vh : ScriptedAI
{
    npc_lin_cloudwalker_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_LIN_CLOUDWALKER_1);
                events.RescheduleEvent(EVENT_2, 7000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_LIN_CLOUDWALKER_2);
                events.RescheduleEvent(EVENT_3, 8000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_LIN_CLOUDWALKER_3);
                events.RescheduleEvent(EVENT_4, 4000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_LIN_CLOUDWALKER_4);
                events.RescheduleEvent(EVENT_5, 6000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_LIN_CLOUDWALKER_5);
                events.RescheduleEvent(EVENT_6, 4000);
                break;
            case EVENT_6:
                Talk(SAY_LINE_LIN_CLOUDWALKER_6);
                events.RescheduleEvent(EVENT_7, 6000);
                break;
            case EVENT_7:
                Talk(SAY_LINE_LIN_CLOUDWALKER_7);
                events.RescheduleEvent(EVENT_8, 11000);
                break;
            case EVENT_8:
                Talk(SAY_LINE_LIN_CLOUDWALKER_8);
                events.RescheduleEvent(EVENT_9, 8000);
                break;
            case EVENT_9:
                Talk(SAY_LINE_LIN_CLOUDWALKER_9);
                events.RescheduleEvent(EVENT_10, 3000);
                break;
            case EVENT_10:
                Talk(SAY_LINE_LIN_CLOUDWALKER_10);
                events.RescheduleEvent(EVENT_11, 2000);
                break;
            case EVENT_11:
                Talk(SAY_LINE_LIN_CLOUDWALKER_11);
                events.RescheduleEvent(EVENT_12, 6000);
                break;
            case EVENT_12:
                Talk(SAY_LINE_LIN_CLOUDWALKER_12);
                events.RescheduleEvent(EVENT_13, 6000);
                break;
            case EVENT_13:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_1);
                events.RescheduleEvent(EVENT_14, 36000);
                break;
            case EVENT_14:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_2);
                events.RescheduleEvent(EVENT_15, 35000);
                break;
            case EVENT_15:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_3);
                events.RescheduleEvent(EVENT_16, 35000);
                break;
            case EVENT_16:
                Talk(SAY_LINE_LIN_CLOUDWALKER_16);
                break;
            }
        }
    }
};

struct npc_cloudwalker_express_mop : ScriptedAI
{
    npc_cloudwalker_express_mop(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> playerList;
    ObjectGuid FirstPlayer;
    ObjectGuid SecondPlayer;
    ObjectGuid ThirdPlayer;
    ObjectGuid Lin;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (apply && who->IsPlayer())
            playerList.push_back(who->GetGUID());
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_LIN_CLOUDWALKER_VH)
            Lin = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 7)
        {
            DoCast(65785);
            me->DespawnOrUnsummon(500);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (auto lin = ObjectAccessor::GetUnit(*me, Lin))
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    lin->ToCreature()->AI()->Talk(SAY_LINE_LIN_CLOUDWALKER_13, target->GetGUID());
            break;
        case ACTION_2:
            if (auto lin = ObjectAccessor::GetUnit(*me, Lin))
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    lin->ToCreature()->AI()->Talk(SAY_LINE_LIN_CLOUDWALKER_14, target->GetGUID());
            break;
        case ACTION_3:
            if (auto lin = ObjectAccessor::GetUnit(*me, Lin))
                if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    lin->ToCreature()->AI()->Talk(SAY_LINE_LIN_CLOUDWALKER_15, target->GetGUID());
            break;
        default:
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
                uint8 i = 0;
                if (!playerList.empty())
                {
                    for (auto& player : playerList)
                    {
                        me->AddPlayerInPersonnalVisibilityList(player);
                        if (auto lin = ObjectAccessor::GetUnit(*me, Lin))
                            lin->AddPlayerInPersonnalVisibilityList(player);

                        if (++i <= 3)
                        {
                            switch (i)
                            {
                            case 1:
                                if (playerList.size() >= 1)
                                    FirstPlayer = player;
                                break;
                            case 2:
                                if (playerList.size() >= 2)
                                    SecondPlayer = player;
                                break;
                            case 3:
                                if (playerList.size() >= 3)
                                    ThirdPlayer = player;
                                break;
                            }
                        }
                    }
                }
                me->GetMotionMaster()->MovePath(118366, false);
                break;
            }
        }
    }
};

//Azsuna
struct npc_boscoe_vh : ScriptedAI
{
    npc_boscoe_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_BOSCOE_1);
                events.RescheduleEvent(EVENT_2, 7000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_BOSCOE_2);
                events.RescheduleEvent(EVENT_3, 12000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_BOSCOE_3);
                events.RescheduleEvent(EVENT_4, 11000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_BOSCOE_4);
                events.RescheduleEvent(EVENT_5, 12000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_BOSCOE_5);
                events.RescheduleEvent(EVENT_6, 9000);
                break;
            case EVENT_6:
                Talk(SAY_LINE_BOSCOE_6);
                events.RescheduleEvent(EVENT_7, 8000);
                break;
            case EVENT_7:
                Talk(SAY_LINE_BOSCOE_7);
                events.RescheduleEvent(EVENT_8, 8000);
                break;
            case EVENT_8:
                Talk(SAY_LINE_BOSCOE_8);
                events.RescheduleEvent(EVENT_9, 14000);
                break;
            case EVENT_9:
                Talk(SAY_LINE_BOSCOE_9);
                break;
            }
        }
    }
};

struct npc_rungle_vh : ScriptedAI
{
    npc_rungle_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_RUNGLE_1);
                events.RescheduleEvent(EVENT_2, 11000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_RUNGLE_2);
                events.RescheduleEvent(EVENT_3, 13000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_RUNGLE_3);
                events.RescheduleEvent(EVENT_4, 10000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_RUNGLE_4);
                events.RescheduleEvent(EVENT_5, 26000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_RUNGLE_5);
                break;
            }
        }
    }
};

struct npc_sky_chariot : ScriptedAI
{
    npc_sky_chariot(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> listPlayers;
    ObjectGuid Boscoe;
    ObjectGuid Rungle;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 3000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (who->IsPlayer())
        {
            if (apply)
                listPlayers.push_back(who->GetGUID());
        }
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_BOSCOE_VH)
            Boscoe = sum->GetGUID();
        if (sum->GetEntry() == NPC_RUNGLE_VH)
            Rungle = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 5)
        {
            for (auto& guid : listPlayers)
            {
                if (!listPlayers.empty())
                {
                    if (auto player = ObjectAccessor::GetPlayer(*me, guid))
                    {
                        DoCast(65785);
                        DoCast(SPELL_JUNIOR_BALLONER_PREPAREDNESS_KIT);
                        player->CastSpell(player, 126408, true);
                        player->AddAura(SPELL_JUNIOR_BALLONER_PREPAREDNESS_KIT, player);
                    }
                }
            }
            me->DespawnOrUnsummon(3000);
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
                if (!listPlayers.empty())
                {
                    for (auto& players : listPlayers)
                    {
                        me->AddPlayerInPersonnalVisibilityList(players);

                        if (auto boscoe = ObjectAccessor::GetUnit(*me, Boscoe))
                            boscoe->AddPlayerInPersonnalVisibilityList(players);
                        if (auto rungle = ObjectAccessor::GetUnit(*me, Rungle))
                            rungle->AddPlayerInPersonnalVisibilityList(players);
                    }
                }
                me->GetMotionMaster()->MovePath(118363, false);
                break;
            }
        }
    }
};

//Suramar
struct npc_zang_cloudwalker_vh : ScriptedAI
{
    npc_zang_cloudwalker_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 4000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_ZANG_1);
                events.RescheduleEvent(EVENT_2, 11000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_ZANG_2);
                events.RescheduleEvent(EVENT_3, 6000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_ZANG_3);
                events.RescheduleEvent(EVENT_4, 6000);
                break;
            case EVENT_4:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_1);
                events.RescheduleEvent(EVENT_5, 11000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_ZANG_5);
                events.RescheduleEvent(EVENT_6, 6000);
                break;
            case EVENT_6:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_2);
                events.RescheduleEvent(EVENT_7, 13000);
                break;
            case EVENT_7:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_3);
                events.RescheduleEvent(EVENT_8, 12000);
                break;
            case EVENT_8:
                Talk(SAY_LINE_ZANG_8);
                events.RescheduleEvent(EVENT_9, 7000);
                break;
            case EVENT_9:
                Talk(SAY_LINE_ZANG_9);
                events.RescheduleEvent(EVENT_10, 8000);
                break;
            case EVENT_10:
                Talk(SAY_LINE_ZANG_10);
                events.RescheduleEvent(EVENT_11, 7000);
                break;
            case EVENT_11:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_4);
                events.RescheduleEvent(EVENT_12, 16000);
                break;
            case EVENT_12:
                Talk(SAY_LINE_ZANG_12);
                events.RescheduleEvent(EVENT_13, 11000);
                break;
            case EVENT_13:
                Talk(SAY_LINE_ZANG_13);
                events.RescheduleEvent(EVENT_14, 7000);
                break;
            case EVENT_14:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_5);
                events.RescheduleEvent(EVENT_15, 16000);
                break;
            case EVENT_15:
                Talk(SAY_LINE_ZANG_15);
                events.RescheduleEvent(EVENT_16, 11000);
                break;
            case EVENT_16:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_6);
                events.RescheduleEvent(EVENT_17, 16000);
                break;
            case EVENT_17:
                Talk(SAY_LINE_ZANG_17);
                events.RescheduleEvent(EVENT_18, 12000);
                break;
            case EVENT_18:
                Talk(SAY_LINE_ZANG_18);
                break;
            }
        }
    }
};

struct npc_cloudwalker_express : ScriptedAI
{
    npc_cloudwalker_express(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> listPlayers;
    ObjectGuid FirstPlayer;
    ObjectGuid SecondPlayer;
    ObjectGuid ThirdPlayer;
    ObjectGuid Zang;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (apply && who->IsPlayer())
            listPlayers.push_back(who->GetGUID());
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_ZANG_CLOUDWALKER_VH)
            Zang = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 10)
        {
            DoCast(65785);
            me->DespawnOrUnsummon(500);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_4, target->GetGUID());
            break;
        case ACTION_2:
            if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_6, target->GetGUID());
            break;
        case ACTION_3:
            if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
                if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_7, target->GetGUID());
            break;
        case ACTION_4:
            if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
            {
                if (FirstPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_11, target->GetGUID());
                }
                else if (SecondPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_11, target->GetGUID());
                }
                else if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_11, target->GetGUID());
                }
            }
            break;
        case ACTION_5:
            if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
            {
                if (SecondPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_14, target->GetGUID());
                }
                else if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_14, target->GetGUID());
                }
                else if (FirstPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_14, target->GetGUID());
                }
            }
            break;
        case ACTION_6:
            if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
            {
                if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_16, target->GetGUID());
                }
                else if (FirstPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_16, target->GetGUID());
                }
                else if (SecondPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                        zang->ToCreature()->AI()->Talk(SAY_LINE_ZANG_16, target->GetGUID());
                }
            }
            break;
        default:
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
                uint8 i = 0;
                if (!listPlayers.empty())
                {
                    for (auto& player : listPlayers)
                    {
                        me->AddPlayerInPersonnalVisibilityList(player);
                        if (auto zang = ObjectAccessor::GetUnit(*me, Zang))
                            zang->AddPlayerInPersonnalVisibilityList(player);

                        if (++i <= 3)
                        {
                            switch (i)
                            {
                            case 1:
                                if (listPlayers.size() >= 1)
                                    FirstPlayer = player;
                                break;
                            case 2:
                                if (listPlayers.size() >= 2)
                                    SecondPlayer = player;
                                break;
                            case 3:
                                if (listPlayers.size() >= 3)
                                    ThirdPlayer = player;
                                break;
                            }
                        }
                    }
                }
                me->GetMotionMaster()->MovePath(118359, false);
                break;
            }
        }
    }
};

//Highmountains
struct npc_hemet_nesingwary_vh : ScriptedAI
{
    npc_hemet_nesingwary_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_HEMET_NESINGWARY_1);
                events.RescheduleEvent(EVENT_2, 7000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_HEMET_NESINGWARY_2);
                events.RescheduleEvent(EVENT_3, 8000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_HEMET_NESINGWARY_3);
                events.RescheduleEvent(EVENT_4, 9000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_HEMET_NESINGWARY_4);
                events.RescheduleEvent(EVENT_5, 7000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_HEMET_NESINGWARY_5);
                events.RescheduleEvent(EVENT_6, 6000);
                break;
            case EVENT_6:
                Talk(SAY_LINE_HEMET_NESINGWARY_6);
                events.RescheduleEvent(EVENT_7, 9000);
                break;
            case EVENT_7:
                Talk(SAY_LINE_HEMET_NESINGWARY_7);
                events.RescheduleEvent(EVENT_8, 11000);
                break;
            case EVENT_8:
                Talk(SAY_LINE_HEMET_NESINGWARY_8);
                events.RescheduleEvent(EVENT_9, 10000);
                break;
            case EVENT_9:
                Talk(SAY_LINE_HEMET_NESINGWARY_9);
                events.RescheduleEvent(EVENT_10, 8000);
                break;
            case EVENT_10:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_1);
                events.RescheduleEvent(EVENT_11, 4000);
                break;
            case EVENT_11:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_2);
                events.RescheduleEvent(EVENT_12, 3000);
                break;
            case EVENT_12:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_3);
                events.RescheduleEvent(EVENT_13, 60000);
                break;
            case EVENT_13:
                Talk(SAY_LINE_HEMET_NESINGWARY_13);
                events.RescheduleEvent(EVENT_14, 76000);
                break;
            case EVENT_14:
                Talk(SAY_LINE_HEMET_NESINGWARY_14);
                break;
            }
        }
    }
};

struct npc_the_killemjaro : ScriptedAI
{
    npc_the_killemjaro(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> listPlayers;
    ObjectGuid FirstPlayer;
    ObjectGuid SecondPlayer;
    ObjectGuid ThirdPlayer;
    ObjectGuid Hemet;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (apply && who->IsPlayer())
            listPlayers.push_back(who->GetGUID());
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_HEMET_NESINGWARY_VH)
            Hemet = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 9)
        {
            DoCast(65785);
            me->DespawnOrUnsummon(500);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (auto hemet = ObjectAccessor::GetUnit(*me, Hemet))
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    hemet->ToCreature()->AI()->Talk(SAY_LINE_HEMET_NESINGWARY_10, target->GetGUID());
            break;
        case ACTION_2:
            if (auto hemet = ObjectAccessor::GetUnit(*me, Hemet))
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    hemet->ToCreature()->AI()->Talk(SAY_LINE_HEMET_NESINGWARY_11, target->GetGUID());
            break;
        case ACTION_3:
            if (auto hemet = ObjectAccessor::GetUnit(*me, Hemet))
                if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    hemet->ToCreature()->AI()->Talk(SAY_LINE_HEMET_NESINGWARY_12, target->GetGUID());
            break;
        default:
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
                uint8 i = 0;
                if (!listPlayers.empty())
                {
                    for (auto& player : listPlayers)
                    {
                        me->AddPlayerInPersonnalVisibilityList(player);
                        if (auto hemet = ObjectAccessor::GetUnit(*me, Hemet))
                            hemet->AddPlayerInPersonnalVisibilityList(player);

                        if (++i <= 3)
                        {
                            switch (i)
                            {
                            case 1:
                                if (listPlayers.size() >= 1)
                                    FirstPlayer = player;
                                break;
                            case 2:
                                if (listPlayers.size() >= 2)
                                    SecondPlayer = player;
                                break;
                            case 3:
                                if (listPlayers.size() >= 3)
                                    ThirdPlayer = player;
                                break;
                            }
                        }
                    }
                }
                me->GetMotionMaster()->MovePath(118364, false);
                break;
            }
        }
    }
};

//Stormheim
struct npc_emi_lan_vh : ScriptedAI
{
    npc_emi_lan_vh(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Talk(SAY_LINE_EMI_LAN_1);
                events.RescheduleEvent(EVENT_2, 8000);
                break;
            case EVENT_2:
                Talk(SAY_LINE_EMI_LAN_2);
                events.RescheduleEvent(EVENT_3, 8000);
                break;
            case EVENT_3:
                Talk(SAY_LINE_EMI_LAN_3);
                events.RescheduleEvent(EVENT_4, 7000);
                break;
            case EVENT_4:
                Talk(SAY_LINE_EMI_LAN_4);
                events.RescheduleEvent(EVENT_5, 7000);
                break;
            case EVENT_5:
                Talk(SAY_LINE_EMI_LAN_5);
                events.RescheduleEvent(EVENT_6, 7000);
                break;
            case EVENT_6:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_1);
                events.RescheduleEvent(EVENT_7, 9000);
                break;
            case EVENT_7:
                Talk(SAY_LINE_EMI_LAN_7);
                events.RescheduleEvent(EVENT_8, 5000);
                break;
            case EVENT_8:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_2);
                events.RescheduleEvent(EVENT_9, 9000);
                break;
            case EVENT_9:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_3);
                events.RescheduleEvent(EVENT_10, 6000);
                break;
            case EVENT_10:
                if (auto owner = me->GetAnyOwner())
                    owner->GetAI()->DoAction(ACTION_4);
                events.RescheduleEvent(EVENT_11, 6000);
                break;
            case EVENT_11:
                Talk(SAY_LINE_EMI_LAN_11);
                break;
            }
        }
    }
};

struct npc_emi_lans_skylounge : ScriptedAI
{
    npc_emi_lans_skylounge(Creature* creature) : ScriptedAI(creature), summon(me) {}

    EventMap events;
    SummonList summon;
    std::list<ObjectGuid> playerList;
    ObjectGuid FirstPlayer;
    ObjectGuid SecondPlayer;
    ObjectGuid ThirdPlayer;
    ObjectGuid Emi;

    void IsSummonedBy(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
    {
        if (!who)
            return;

        if (apply && who->IsPlayer())
            playerList.push_back(who->GetGUID());
    }

    void JustSummoned(Creature* sum) override
    {
        summon.Summon(sum);

        if (sum->GetEntry() == NPC_EMI_LAN_VH)
            Emi = sum->GetGUID();
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != WAYPOINT_MOTION_TYPE)
            return;

        if (pointId == 7)
        {
            DoCast(65785);
            me->DespawnOrUnsummon(500);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (auto emi = ObjectAccessor::GetUnit(*me, Emi))
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    emi->ToCreature()->AI()->Talk(SAY_LINE_EMI_LAN_6, target->GetGUID());
            break;
        case ACTION_2:
            if (auto emi = ObjectAccessor::GetUnit(*me, Emi))
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    emi->ToCreature()->AI()->Talk(SAY_LINE_EMI_LAN_8, target->GetGUID());
            break;
        case ACTION_3:
            if (auto emi = ObjectAccessor::GetUnit(*me, Emi))
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    emi->ToCreature()->AI()->Talk(SAY_LINE_EMI_LAN_9, target->GetGUID());
            break;
        case ACTION_4:
            if (auto emi = ObjectAccessor::GetUnit(*me, Emi))
            {
                if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                        emi->ToCreature()->AI()->Talk(SAY_LINE_EMI_LAN_10, target->GetGUID());
                }
                if (FirstPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                        emi->ToCreature()->AI()->Talk(SAY_LINE_EMI_LAN_10, target->GetGUID());
                }
                if (SecondPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                        emi->ToCreature()->AI()->Talk(SAY_LINE_EMI_LAN_10, target->GetGUID());
                }
            }
            break;
        default:
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
                uint8 i = 0;
                if (!playerList.empty())
                {
                    for (auto& player : playerList)
                    {
                        me->AddPlayerInPersonnalVisibilityList(player);
                        if (auto emi = ObjectAccessor::GetUnit(*me, Emi))
                            emi->AddPlayerInPersonnalVisibilityList(player);

                        if (++i <= 3)
                        {
                            switch (i)
                            {
                            case 1:
                                if (playerList.size() >= 1)
                                    FirstPlayer = player;
                                break;
                            case 2:
                                if (playerList.size() >= 2)
                                    SecondPlayer = player;
                                break;
                            case 3:
                                if (playerList.size() >= 3)
                                    ThirdPlayer = player;
                                break;
                            }
                        }
                    }
                }
                me->GetMotionMaster()->MovePath(118367, false);
                break;
            }
        }
    }
};

//236346
class spell_schnottz_has_to_go : public SpellScript
{
    PrepareSpellScript(spell_schnottz_has_to_go);

    void HandleOnCast()
    {
        if (Unit* caster = GetCaster())
            if (auto vehicle = caster->GetAnyOwner())
                vehicle->GetAI()->DoAction(ACTION_1);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_schnottz_has_to_go::HandleOnCast);
    }
};

class spell_waiting_for_balloon : public AuraScript
{
    PrepareAuraScript(spell_waiting_for_balloon);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (auto c = caster->FindNearestCreature(118470, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
        else if (auto c = caster->FindNearestCreature(118474, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
        else if (auto c = caster->FindNearestCreature(118468, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
        else if (auto c = caster->FindNearestCreature(118436, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
        else if (auto c = caster->FindNearestCreature(94409, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
        else if (auto c = caster->FindNearestCreature(118443, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
        else if (auto c = caster->FindNearestCreature(118482, 20.0f))
            c->ToCreature()->AI()->SetGUID(caster->GetGUID(), 1);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_waiting_for_balloon::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_SpringBalloonFestival()
{
    RegisterCreatureAI(npc_chens);
    RegisterCreatureAI(npc_scgnottz);
    RegisterCreatureAI(npc_lin_cloudwalker);
    RegisterCreatureAI(npc_rungle);
    RegisterCreatureAI(npc_zang_cloudwalker);
    RegisterCreatureAI(npc_hemet_nesingwary);
    RegisterCreatureAI(npc_emi_lan);
    RegisterCreatureAI(npc_chens_vh);
    RegisterCreatureAI(npc_scgnottz_vh);
    //RegisterCreatureAI(npc_lin_cloudwalker_vh);
    RegisterCreatureAI(npc_boscoe_vh);
    RegisterCreatureAI(npc_rungle_vh);
    RegisterCreatureAI(npc_zang_cloudwalker_vh);
    RegisterCreatureAI(npc_hemet_nesingwary_vh);
    RegisterCreatureAI(npc_emi_lan_vh);
    RegisterCreatureAI(npc_scgnottz_ballon);
    RegisterCreatureAI(npc_sky_chariot);
    RegisterCreatureAI(npc_chens_balloon);
    RegisterCreatureAI(npc_cloudwalker_express);
    //RegisterCreatureAI(npc_cloudwalker_express_mop);
    RegisterCreatureAI(npc_the_killemjaro);
    RegisterCreatureAI(npc_emi_lans_skylounge);
    RegisterSpellScript(spell_schnottz_has_to_go);
    RegisterAuraScript(spell_waiting_for_balloon);
}