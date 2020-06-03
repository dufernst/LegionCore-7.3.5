/*
*/

#include "WorldStatePackets.h"
#include "Scenario.h"
#include "ScenarioMgr.h"
#include "ScenePackets.h"

enum eEnums
{
    DATA_TRIAL_OF_STYLE = 1,
    MAX_ENCOUNTER
};

enum eSay
{
    // Nastasia
    SAY_HELLO_LEFT = 17,
    SAY_THIRD_PLACE,
    SAY_FIRST_PLACE,
    SAY_END_1,
    SAY_END_2,

    // Atheris
    SAY_30_SEC = 17,
    SAY_10_SEC,
    SAY_HELLO_RIGHT,
    SAY_SECOND_PLACE
};

enum eSpells
{
    SPELL_MORPH             = 241834,
    SPELL_FIRST_PLACE       = 242146,
    SPELL_SECOND_PLACE      = 242147,
    SPELL_THIRD_PLACE       = 242148,
    SPELL_VOTE              = 241866,
    SPELL_RIGHT_SIDE_STAGE  = 241865,
    SPELL_LEFT_SIDE_STAGE   = 241848,
    SPELL_CENTER_STAGE      = 242091,
    SPELL_CONSOLATION_PRIZE = 242149,
    SPELL_TITLE_CARD        = 242127,
    SPELL_ON_STAGE          = 241844,
    SPELL_STAGE_FIRE        = 261918,
    SPELL_STAGE_SPOTLIGHT   = 261923
};

enum eEvents
{
    EVENT_PREPARE           = 1,
    EVENT_30_SEC,
    EVENT_10_SEC,
    EVENT_INTRO,
    EVENT_THEME,

    EVENT_FIRST_PLACE,
    EVENT_SECOND_PLACE,
    EVENT_CONSOLATION_PLACE,

    EVENT_END_1,
    EVENT_END_2,

    EVENT_END_WP,
    EVENT_MOVE_BACK,

    EVENT_RAUND_2,
    EVENT_RAUND_3,
    EVENT_RAUND_4,
    EVENT_RAUND_5,
    EVENT_RAUND_6,
    EVENT_RAUND_7,
    EVENT_RAUND_8,
    EVENT_RAUND_9,

    EVENT_GENERATE_RESULTS,

    EVENT_COMPLETE_TIMER = 27
};

struct npc_atheris_voguesong : ScriptedAI
{
    npc_atheris_voguesong(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    struct PlayerVoteT
    {
        ObjectGuid guid;
        uint8 count;
    };

    bool intro;
    uint32 generateTheme;
    uint32 numberTheme;
    uint32 roundNumber;
    EventMap events;
    InstanceScript* instance;

    std::map<ObjectGuid, PlayerVoteT> PlayerVote;
    std::vector<PlayerVoteT*> PlayerVoteV;
    std::list<ObjectGuid> PlayerListGUID;
    std::list<ObjectGuid> PlayersListForVote;

    ObjectGuid FirstPlayer;
    ObjectGuid SecondPlayer;
    ObjectGuid ThirdPlayer;
    ObjectGuid FourthPlayer;
    ObjectGuid FifthPlayer;
    ObjectGuid SixthPlayer;

    // Rewards
    std::list<ObjectGuid> ConsolationList;
    ObjectGuid FirstPlace;
    ObjectGuid SecondPlace;
    ObjectGuid ThirdPlace;

    void Reset() override
    {
        events.Reset();

        generateTheme       = 0;
        numberTheme         = 0;
        roundNumber         = 0;

        intro = false;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 30.0f)
            return;

        if (!intro)
        {
            intro = true;

            if (instance->GetBossState(DATA_TRIAL_OF_STYLE) != DONE)
                events.RescheduleEvent(EVENT_PREPARE, 5000);
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_3)
        {
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(1, 246.8698f, -1.246528f, 1.997569f, 0.0f);
        }

        if (action == ACTION_2)
        {
            me->AddDelayedEvent(1000, [=]() -> void
            {
                if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlace))
                {
                    if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                        nastasia->AI()->Talk(SAY_THIRD_PLACE, target->GetGUID());

                    target->RemoveAura(SPELL_MORPH);
                    target->CastSpell(target, 242148, false);

                    if (target->HasAura(261939))
                        target->CastSpell(target, SPELL_STAGE_FIRE, true);
                    else
                        if (target->HasAura(261938))
                            target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                }
                events.RescheduleEvent(EVENT_SECOND_PLACE, 7000);
            });
        }

        if (action == ACTION_1)
        {
            if (PlayerListGUID.size() < 6)
                return;

            if (instance)
            {
                instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 9);
                instance->DoUpdateWorldState(static_cast<WorldStates>(13448), 2);
                instance->DoUpdateWorldState(static_cast<WorldStates>(13691), 1);
                instance->SetBossState(DATA_TRIAL_OF_STYLE, IN_PROGRESS);
            }

            if (!PlayerListGUID.empty())
            {
                FirstPlayer = PlayerListGUID.front();
                PlayerListGUID.pop_front();
                PlayerVote[FirstPlayer].count = 0;
            }

            if (!PlayerListGUID.empty())
            {
                SecondPlayer = PlayerListGUID.front();
                PlayerListGUID.pop_front();
                PlayerVote[SecondPlayer].count = 0;
            }

            if (!PlayerListGUID.empty())
            {
                ThirdPlayer = PlayerListGUID.front();
                PlayerListGUID.pop_front();
                PlayerVote[ThirdPlayer].count = 0;
            }

            if (!PlayerListGUID.empty())
            {
                FourthPlayer = PlayerListGUID.front();
                PlayerListGUID.pop_front();
                PlayerVote[FourthPlayer].count = 0;
            }

            if (!PlayerListGUID.empty())
            {
                FifthPlayer = PlayerListGUID.front();
                PlayerListGUID.pop_front();
                PlayerVote[FifthPlayer].count = 0;
            }

            if (!PlayerListGUID.empty())
            {
                SixthPlayer = PlayerListGUID.front();
                PlayerListGUID.pop_front();
                PlayerVote[SixthPlayer].count = 0;
            }

            if (PlayersListForVote.empty())
            {
                if (ThirdPlayer)
                    PlayersListForVote.push_back(ThirdPlayer);
                if (FourthPlayer)
                    PlayersListForVote.push_back(FourthPlayer);
                if (FifthPlayer)
                    PlayersListForVote.push_back(FifthPlayer);
                if (SixthPlayer)
                    PlayersListForVote.push_back(SixthPlayer);
            }

            if (FirstPlayer)
            {
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                {
                    target->RemoveAura(SPELL_VOTE);
                    target->RemoveAura(SPELL_MORPH);
                    target->CastSpell(target, 241865, false);
                    me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                    target->AddDelayedEvent(500, [target]() -> void
                    {
                        if (target->HasAura(261939))
                            target->CastSpell(target, SPELL_STAGE_FIRE, true);
                        else
                            if (target->HasAura(261938))
                                target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                    });
                }
            }

            if (SecondPlayer)
            {
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                {
                    target->RemoveAura(SPELL_VOTE);
                    target->RemoveAura(SPELL_MORPH);
                    target->CastSpell(target, 241848, false);
                    target->AddDelayedEvent(500, [target]() -> void
                    {
                        if (target->HasAura(261939))
                            target->CastSpell(target, SPELL_STAGE_FIRE, true);
                        else
                            if (target->HasAura(261938))
                                target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                    });

                    if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                        nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                }
            }

            if (!PlayersListForVote.empty())
                for (auto& guids : PlayersListForVote)
                    if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                        target->CastSpell(target, 241866, false);

            events.RescheduleEvent(EVENT_RAUND_2, 20000);
        }

        if (action == ACTION_4)
        {
            if (roundNumber == 0)
                PlayerVote[FirstPlayer].count++;
            else if (roundNumber == 1)
                PlayerVote[SixthPlayer].count++;
            else if (roundNumber == 2)
                PlayerVote[FourthPlayer].count++;
            else if (roundNumber == 3)
                PlayerVote[SixthPlayer].count++;
            else if (roundNumber == 4)
                PlayerVote[FourthPlayer].count++;
            else if (roundNumber == 5)
                PlayerVote[FifthPlayer].count++;
            else if (roundNumber == 6)
                PlayerVote[FifthPlayer].count++;
            else if (roundNumber == 7)
                PlayerVote[FirstPlayer].count++;
            else if (roundNumber == 8)
                PlayerVote[SixthPlayer].count++;
        }

        if (action == ACTION_5)
        {
            if (roundNumber == 0)
                PlayerVote[SecondPlayer].count++;
            else if (roundNumber == 1)
                PlayerVote[FifthPlayer].count++;
            else if (roundNumber == 2)
                PlayerVote[ThirdPlayer].count++;
            else if (roundNumber == 3)
                PlayerVote[ThirdPlayer].count++;
            else if (roundNumber == 4)
                PlayerVote[FirstPlayer].count++;
            else if (roundNumber == 5)
                PlayerVote[SecondPlayer].count++;
            else if (roundNumber == 6)
                PlayerVote[FourthPlayer].count++;
            else if (roundNumber == 7)
                PlayerVote[ThirdPlayer].count++;
            else if (roundNumber == 8)
                PlayerVote[FirstPlayer].count++;
        }
    }

    void MovementInform(uint32 type, uint32 data)
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (data)
            {
            case 1:
                events.RescheduleEvent(EVENT_22, 100);
                break;
            case 2:
                events.RescheduleEvent(EVENT_23, 100);
                break;
            case 3:
                events.RescheduleEvent(EVENT_24, 100);
                break;
            case 4:
                events.RescheduleEvent(EVENT_25, 100);
                break;
            case 5:
                events.RescheduleEvent(EVENT_26, 100);
                break;
            case 6:
                me->DespawnOrUnsummon();
                break;
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
            case EVENT_PREPARE:
            {
                generateTheme = urand(1, 16);
                numberTheme = generateTheme;

                if (instance)
                {
                    instance->SetBossState(DATA_TRIAL_OF_STYLE, NOT_STARTED);
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13449), numberTheme);
                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 58590);
                }

                events.RescheduleEvent(EVENT_THEME, 14000);
                break;
            }
            case EVENT_THEME:
                if (instance)
                {
                    if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                        nastasia->AI()->Talk(numberTheme);

                    instance->DoCastSpellOnPlayers(SPELL_TITLE_CARD);
                }
                events.RescheduleEvent(EVENT_INTRO, 4000);
                break;
            case EVENT_INTRO:
                Talk(numberTheme);
                events.RescheduleEvent(EVENT_30_SEC, 111500);
                break;
            case EVENT_30_SEC:
                ZoneTalk(SAY_30_SEC);
                events.RescheduleEvent(EVENT_10_SEC, 20000);
                break;
            case EVENT_10_SEC:
                instance->instance->ApplyOnEveryPlayer([&](Player* player) { PlayerListGUID.push_back(player->GetGUID()); });
                ZoneTalk(SAY_10_SEC);
                events.RescheduleEvent(EVENT_COMPLETE_TIMER, 9000);
                break;
            case EVENT_COMPLETE_TIMER:
                if (instance)
                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 57797);
                break;
            case EVENT_RAUND_2:
                events.RescheduleEvent(EVENT_RAUND_3, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 8);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (FirstPlayer)
                        PlayersListForVote.push_back(FirstPlayer);
                    if (SecondPlayer)
                        PlayersListForVote.push_back(SecondPlayer);
                    if (ThirdPlayer)
                        PlayersListForVote.push_back(ThirdPlayer);
                    if (FourthPlayer)
                        PlayersListForVote.push_back(FourthPlayer);
                }

                if (FifthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FifthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (SixthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SixthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_3:
                events.RescheduleEvent(EVENT_RAUND_4, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 7);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (FirstPlayer)
                        PlayersListForVote.push_back(FirstPlayer);
                    if (SecondPlayer)
                        PlayersListForVote.push_back(SecondPlayer);
                    if (FifthPlayer)
                        PlayersListForVote.push_back(FifthPlayer);
                    if (SixthPlayer)
                        PlayersListForVote.push_back(SixthPlayer);
                }

                if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (FourthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FourthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_4:
                events.RescheduleEvent(EVENT_RAUND_5, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 6);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (FirstPlayer)
                        PlayersListForVote.push_back(FirstPlayer);
                    if (SecondPlayer)
                        PlayersListForVote.push_back(SecondPlayer);
                    if (FifthPlayer)
                        PlayersListForVote.push_back(FifthPlayer);
                    if (FourthPlayer)
                        PlayersListForVote.push_back(FourthPlayer);
                }

                if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (SixthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SixthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_5:
                events.RescheduleEvent(EVENT_RAUND_6, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 5);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (ThirdPlayer)
                        PlayersListForVote.push_back(ThirdPlayer);
                    if (SecondPlayer)
                        PlayersListForVote.push_back(SecondPlayer);
                    if (FifthPlayer)
                        PlayersListForVote.push_back(FifthPlayer);
                    if (SixthPlayer)
                        PlayersListForVote.push_back(SixthPlayer);
                }

                if (FirstPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (FourthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FourthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_6:
                events.RescheduleEvent(EVENT_RAUND_7, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 4);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (ThirdPlayer)
                        PlayersListForVote.push_back(ThirdPlayer);
                    if (FirstPlayer)
                        PlayersListForVote.push_back(FirstPlayer);
                    if (SixthPlayer)
                        PlayersListForVote.push_back(SixthPlayer);
                    if (FourthPlayer)
                        PlayersListForVote.push_back(FourthPlayer);
                }

                if (SecondPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (FifthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FifthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_7:
                events.RescheduleEvent(EVENT_RAUND_8, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 3);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (ThirdPlayer)
                        PlayersListForVote.push_back(ThirdPlayer);
                    if (FirstPlayer)
                        PlayersListForVote.push_back(FirstPlayer);
                    if (SixthPlayer)
                        PlayersListForVote.push_back(SixthPlayer);
                    if (SecondPlayer)
                        PlayersListForVote.push_back(SecondPlayer);
                }

                if (FourthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FourthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (FifthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FifthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_8:
                events.RescheduleEvent(EVENT_RAUND_9, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 2);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (FourthPlayer)
                        PlayersListForVote.push_back(FourthPlayer);
                    if (FirstPlayer)
                        PlayersListForVote.push_back(FirstPlayer);
                    if (SixthPlayer)
                        PlayersListForVote.push_back(SixthPlayer);
                    if (FifthPlayer)
                        PlayersListForVote.push_back(FifthPlayer);
                }

                if (ThirdPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, ThirdPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (SecondPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                if (!PlayersListForVote.empty())
                    for (auto& guids : PlayersListForVote)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                            target->CastSpell(target, 241866, false);
                break;
            case EVENT_RAUND_9:
                events.RescheduleEvent(EVENT_GENERATE_RESULTS, 20000);
                roundNumber++;

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 1);

                if (!PlayersListForVote.empty())
                    PlayersListForVote.clear();

                if (PlayersListForVote.empty())
                {
                    if (FourthPlayer)
                        PlayersListForVote.push_back(FourthPlayer);
                    if (SecondPlayer)
                        PlayersListForVote.push_back(SecondPlayer);
                    if (ThirdPlayer)
                        PlayersListForVote.push_back(ThirdPlayer);
                    if (FifthPlayer)
                        PlayersListForVote.push_back(FifthPlayer);
                }

                if (FirstPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241865, false);
                        me->AddDelayedEvent(2000, [=]() -> void { Talk(SAY_HELLO_RIGHT, target->GetGUID()); });
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });
                    }
                }

                if (SixthPlayer)
                {
                    if (auto target = ObjectAccessor::GetPlayer(*me, SixthPlayer))
                    {
                        target->RemoveAura(SPELL_MORPH);
                        target->RemoveAura(SPELL_VOTE);
                        target->CastSpell(target, 241848, false);
                        target->AddDelayedEvent(500, [target]() -> void
                        {
                            if (target->HasAura(261939))
                                target->CastSpell(target, SPELL_STAGE_FIRE, true);
                            else
                                if (target->HasAura(261938))
                                    target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                        });

                        if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                            nastasia->AI()->Talk(SAY_HELLO_LEFT, target->GetGUID());
                    }
                }

                for (auto& guids : PlayersListForVote)
                    if (auto target = ObjectAccessor::GetPlayer(*me, guids))
                        target->CastSpell(target, 241866, false);
                break;
            case EVENT_GENERATE_RESULTS:
            {
                uint8 i = 0;
                if (!PlayerVote.empty())
                {
                    for (auto& temp : PlayerVote)
                    {
                        temp.second.guid = temp.first;
                        PlayerVoteV.push_back(&temp.second);
                    }

                    std::sort(PlayerVoteV.begin(), PlayerVoteV.end(), [](PlayerVoteT const* a, PlayerVoteT const* b) { return a->count > b->count; });

                    for (auto& itr : PlayerVoteV)
                    {
                        if (i++ <= 3)
                        {
                            switch (i)
                            {
                            case 1:
                                FirstPlace = itr->guid;
                                break;
                            case 2:
                                SecondPlace = itr->guid;
                                break;
                            case 3:
                                ThirdPlace = itr->guid;
                                break;
                            }
                        }
                        else
                            ConsolationList.push_back(itr->guid);
                    }
                }

                if (instance)
                {
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13692), 0);
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13448), 0);
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13691), 0);
                    instance->DoUpdateWorldState(static_cast<WorldStates>(13449), 0);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TITLE_CARD);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MORPH);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOTE);
                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 57798);
                }
                break;
            }
            case EVENT_SECOND_PLACE:
                if (auto target = ObjectAccessor::GetPlayer(*me, SecondPlace))
                {
                    Talk(SAY_SECOND_PLACE, target->GetGUID());
                    target->RemoveAura(SPELL_MORPH);
                    target->CastSpell(target, 242147, false);
                    target->AddDelayedEvent(500, [target]() -> void
                    {
                        if (target->HasAura(261939))
                            target->CastSpell(target, SPELL_STAGE_FIRE, true);
                        else
                            if (target->HasAura(261938))
                                target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                    });
                }
                events.RescheduleEvent(EVENT_MOVE_BACK, 4000);
                break;
            case EVENT_MOVE_BACK:
                if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                    nastasia->AI()->DoAction(ACTION_2);

                me->GetMotionMaster()->MoveBackward(1, 242.3472f, -1.911458f, 1.913132f);
                events.RescheduleEvent(EVENT_FIRST_PLACE, 2000);
                break;
            case EVENT_FIRST_PLACE:
                if (auto target = ObjectAccessor::GetPlayer(*me, FirstPlace))
                {
                    if (auto nastasia = instance->instance->GetCreature(instance->GetGuidData(121325)))
                    {
                        nastasia->AI()->Talk(SAY_FIRST_PLACE, target->GetGUID());
                        nastasia->AI()->DoAction(ACTION_1);
                    }
                    target->RemoveAura(SPELL_MORPH);
                    target->CastSpell(target, 242146, false);
                    target->AddDelayedEvent(500, [target]() -> void
                    {
                        if (target->HasAura(261939))
                            target->CastSpell(target, SPELL_STAGE_FIRE, true);
                        else
                            if (target->HasAura(261938))
                                target->CastSpell(target, SPELL_STAGE_SPOTLIGHT, true);
                    });

                    instance->SetBossState(DATA_TRIAL_OF_STYLE, DONE);
                    events.RescheduleEvent(EVENT_CONSOLATION_PLACE, 1000);
                }
                break;
            case EVENT_CONSOLATION_PLACE:
                if (!ConsolationList.empty())
                    for (auto& guid : ConsolationList)
                        if (auto target = ObjectAccessor::GetPlayer(*me, guid))
                            target->CastSpell(target, 242149, false);
                break;
            case EVENT_22:
                me->GetMotionMaster()->MovePoint(2, 251.9184f, -1.289931f, 1.777769f, 0.0f);
                break;
            case EVENT_23:
                me->GetMotionMaster()->MovePoint(3, 253.0122f, -4.375f, 1.832731f, 0.0f);
                break;
            case EVENT_24:
                me->GetMotionMaster()->MovePoint(4, 253.2396f, -11.15625f, 1.929671f, 0.0f);
                break;
            case EVENT_25:
                me->GetMotionMaster()->MovePoint(5, 253.3142f, -17.10764f, 1.868134f, 0.0f);
                break;
            case EVENT_26:
                me->GetMotionMaster()->MovePoint(6, 253.5764f, -24.18924f, 1.868134f, 0.0f);
                break;
            }
        }
    }
};

struct npc_nastasia_flairwatcher : ScriptedAI
{
    npc_nastasia_flairwatcher(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;

    void Reset() override
    {
        events.Reset();
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            events.RescheduleEvent(EVENT_END_1, 5000);
        }

        if (action == ACTION_2)
        {
            me->GetMotionMaster()->MoveBackward(1, 242.343f, 2.159722f, 1.861324f);
        }
    }

    void MovementInform(uint32 type, uint32 data)
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (data)
            {
            case 1:
                events.RescheduleEvent(EVENT_13, 100);
                break;
            case 2:
                events.RescheduleEvent(EVENT_14, 100);
                break;
            case 3:
                events.RescheduleEvent(EVENT_15, 100);
                break;
            case 4:
                events.RescheduleEvent(EVENT_16, 100);
                break;
            case 5:
                events.RescheduleEvent(EVENT_17, 100);
                break;
            case 6:
                events.RescheduleEvent(EVENT_18, 100);
                break;
            case 7:
                events.RescheduleEvent(EVENT_19, 100);
                break;
            case 8:
                events.RescheduleEvent(EVENT_20, 100);
                break;
            case 9:
                me->DespawnOrUnsummon();
                break;
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
            case EVENT_END_1:
                Talk(SAY_END_1);
                events.RescheduleEvent(EVENT_END_2, 4000);
                break;
            case EVENT_END_2:
                Talk(SAY_END_2);
                events.RescheduleEvent(EVENT_END_WP, 5000);
                break;
            case EVENT_END_WP:
                if (auto atheris = instance->instance->GetCreature(instance->GetGuidData(121326)))
                    atheris->AI()->DoAction(ACTION_3);

                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(1, 249.724f, 2.81684f, 2.19091f, 0.0f);
                break;
            case EVENT_13:
                me->GetMotionMaster()->MovePoint(2, 251.6042f, 2.973958f, 2.020497f, 0.0f);
                break;
            case EVENT_14:
                me->GetMotionMaster()->MovePoint(3, 252.9241f, 2.91665f, 1.694532f, 0.0f);
                break;
            case EVENT_15:
                me->GetMotionMaster()->MovePoint(4, 254.9826f, 3.144097f, 1.428368f, 0.0f);
                break;
            case EVENT_16:
                me->GetMotionMaster()->MovePoint(5, 256.8021f, 0.08854166f, 1.428368f, 0.0f);
                break;
            case EVENT_17:
                me->GetMotionMaster()->MovePoint(6, 256.8767f, -7.097222f, 1.268127f, 0.0f);
                break;
            case EVENT_18:
                me->GetMotionMaster()->MovePoint(7, 256.7049f, -10.11632f, 2.009011f, 0.0f);
                break;
            case EVENT_19:
                me->GetMotionMaster()->MovePoint(8, 256.1406f, -17.11111f, 1.868134f, 0.0f);
                break;
            case EVENT_20:
                me->GetMotionMaster()->MovePoint(9, 255.5451f, -24.24653f, 1.868134f, 0.0f);
                break;
            }
        }
    }
};

Position const randomTeleport[] =
{
    { 233.375f, -5.94793f, -2.96241f },
    { 230.149f, -6.24524f, -2.9328f },
    { 232.436f, -1.56904f, -2.95623f },
    { 233.798f, 2.25448f, -2.98525f },
    { 231.339f, 0.42372f, -2.95058f },
    { 232.463f, 4.62785f, -2.95272f },
    { 226.787f, 2.74287f, -2.91368f },
    { 230.207f, 0.486381f, -2.93776f },
    { 237.905f, 0.292878f, -3.02129f }
};

//241844
class spell_trial_of_style_on_stage : public AuraScript
{
    PrepareAuraScript(spell_trial_of_style_on_stage);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        target->RemoveAurasByType(SPELL_AURA_TRANSFORM);
        target->RemoveAura(130157);
        target->RemoveAura(130158);
        target->RemoveAura(127323);
    }

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        InstanceScript* instance = target->GetInstanceScript();

        if (instance)
        {
            target->RemoveAura(SPELL_STAGE_FIRE);
            target->RemoveAura(SPELL_STAGE_SPOTLIGHT);

            if (instance->GetBossState(DATA_TRIAL_OF_STYLE) != DONE)
            {
                target->CastSpell(target, SPELL_MORPH, false);
                target->NearTeleportTo(randomTeleport[urand(0, 8)]);
            }
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_trial_of_style_on_stage::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL);
        OnEffectApply += AuraEffectApplyFn(spell_trial_of_style_on_stage::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL);
    }
};

//241866
class spell_trial_of_style_vote : public AuraScript
{
    PrepareAuraScript(spell_trial_of_style_vote);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->ToPlayer()->SendDirectMessage(WorldPackets::Scene::CancelScene(1694).Write());
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_trial_of_style_vote::OnRemove, EFFECT_0, SPELL_AURA_ACTIVATE_SCENE, AURA_EFFECT_HANDLE_REAL);
    }
};

class sceneTrigger_trial_of_style : public SceneTriggerScript
{
public:
    sceneTrigger_trial_of_style() : SceneTriggerScript("sceneTrigger_trial_of_style") {}

    bool OnTrigger(Player* player, SpellScene const* trigger, std::string type) override
    {
        if (type == "voteA")
        {
            if (auto instance = player->GetInstanceScript())
                if (auto atheris = instance->instance->GetCreature(instance->GetGuidData(121326)))
                    atheris->GetAI()->DoAction(ACTION_4);
        }

        if (type == "voteB")
        {
            if (auto instance = player->GetInstanceScript())
                if (auto atheris = instance->instance->GetCreature(instance->GetGuidData(121326)))
                    atheris->GetAI()->DoAction(ACTION_5);
        }
        return true;
    }
};

class instance_trial_of_style : public InstanceMapScript
{
public:
    instance_trial_of_style() : InstanceMapScript("instance_trial_of_style", 1744) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_trial_of_style_InstanceMapScript(map);
    }

    struct instance_trial_of_style_InstanceMapScript : public InstanceScript
    {
        instance_trial_of_style_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        ObjectGuid atherisGUID;
        ObjectGuid nastasiaGUID;
        ObjectGuid torellGUID;
        ObjectGuid sondariGUID;
        ObjectGuid nellieGUID;
        std::vector<ObjectGuid> GoChairGUID{};
        uint32 checkTimer = 1000;

        void Initialize() override {}

        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
        {
            packet.Worldstates.emplace_back(static_cast<WorldStates>(13692), 0);
            packet.Worldstates.emplace_back(static_cast<WorldStates>(13448), 0);
            packet.Worldstates.emplace_back(static_cast<WorldStates>(13691), 0);
            packet.Worldstates.emplace_back(static_cast<WorldStates>(13449), 0);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            switch (creature->GetEntry())
            {
            case 121326:
                atherisGUID = creature->GetGUID();
                break;
            case 121325:
                nastasiaGUID = creature->GetGUID();
                break;
            case 121307:
                sondariGUID = creature->GetGUID();
                break;
            case 121139:
                torellGUID = creature->GetGUID();
                break;
            case 133164:
                nellieGUID = creature->GetGUID();
                break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            InstanceScript::OnGameObjectCreate(go);

            switch (go->GetEntry())
            {
            case 269229:
                GoChairGUID.push_back(go->GetGUID());
                break;
            default:
                break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
            case 121326:
                return atherisGUID;
            case 121325:
                return nastasiaGUID;
            case 121307:
                return sondariGUID;
            case 121139:
                return torellGUID;
            case 133164:
                return nellieGUID;
            }
            return ObjectGuid::Empty;
        }

        void OnPlayerEnter(Player* player) override
        {
            player->CastSpell(player, SPELL_MORPH, true);
        }

        void onScenarionNextStep(uint32 newStep) override
        {
            switch (newStep)
            {
            case 1:
                if (auto atheris = instance->GetCreature(atherisGUID))
                    atheris->CreateConversation(4978);

                instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    player->AddDelayedEvent(500, [=]() -> void
                    {
                        player->GetAchievementMgr()->StartTimedAchievement(static_cast<CriteriaTimedTypes>(CRITERIA_TIMED_TYPE_EVENT2), 57838);

                        if (auto scenario = sScenarioMgr->GetScenario(instance->GetInstanceId()))
                            scenario->GetAchievementMgr().StartTimedAchievement(static_cast<CriteriaTimedTypes>(CRITERIA_TIMED_TYPE_EVENT2), 57838);
                    });
                });
                break;
            case 2:
                if (auto atheris = instance->GetCreature(atherisGUID))
                    atheris->GetAI()->DoAction(ACTION_1);
                if (auto sontari = instance->GetCreature(sondariGUID))
                    sontari->SetVisible(false);
                if (auto torell = instance->GetCreature(torellGUID))
                    torell->SetVisible(false);
                if (auto nellie = instance->GetCreature(nellieGUID))
                    nellie->SetVisible(false);
                for (auto& guid : GoChairGUID)
                    if (auto chair = instance->GetGameObject(guid))
                        chair->SetPhaseMask(0, true);
                break;
            case 3:
                if (auto atheris = instance->GetCreature(atherisGUID))
                    atheris->GetAI()->DoAction(ACTION_2);
                break;
            }
        }

        void Update(uint32 diff) override
        {
            if (checkTimer <= diff)
            {
                checkTimer = 1000;

                instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (GetBossState(DATA_TRIAL_OF_STYLE) != DONE && (player->GetPositionZ() >= 0.5f))
                    {
                        player->AddDelayedEvent(500, [=]() -> void
                        {
                            if (!player->HasAura(SPELL_ON_STAGE))
                                player->NearTeleportTo(randomTeleport[urand(0, 8)]);
                        });
                    }

                    if (GetBossState(DATA_TRIAL_OF_STYLE) == NOT_STARTED)
                        if (player->IsStandState())
                            player->CastSpell(player, SPELL_MORPH, false);
                });
            }
            else
                checkTimer -= diff;
        }
    };
};

void AddSC_TrialofStyle()
{
    RegisterCreatureAI(npc_atheris_voguesong);
    RegisterCreatureAI(npc_nastasia_flairwatcher);
    RegisterAuraScript(spell_trial_of_style_on_stage);
    RegisterAuraScript(spell_trial_of_style_vote);
    new instance_trial_of_style();
    new sceneTrigger_trial_of_style();
}