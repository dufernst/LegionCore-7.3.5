#include "ScriptMgr.h"

enum Says
{
    //Annoncer
    SAY_CHIJI_START         = 0,
    SAY_CHIJI_FINISH        = 1,
    SAY_XUEN_START          = 2,
    SAY_XUEN_FINISH         = 3,
    SAY_YULON_START         = 4,
    SAY_YULON_FINISH        = 5,
    SAY_NIUZAO_START        = 6,
    SAY_NIUZAO_FINISH       = 7,

    //Misc
    SAY_ENTER_POS           = 0,
    SAY_AGGRO               = 1,

    //ChiJi
    SAY_CHIJI_BEACON        = 2,
    SAY_CHIJI_RUSH_1        = 4,
    SAY_CHIJI_BLAZ_SONG     = 3,
    SAY_CHIJI_RUSH_2        = 5,
    SAY_CHIJI_END           = 6,

    //Xuen
    SAY_XUEN_BARRAGE        = 2,
    SAY_XUEN_LIGHTNING      = 3,
    SAY_XUEN_PLAYER_DEATH   = 4,
    SAY_XUEN_AGGILITY       = 5,
    SAY_XUEN_END            = 6,

    //Yulon
    SAY_YULON_PLAYER_DEATH   = 2,
    SAY_YULON_WALL_1         = 3,
    SAY_YULON_WALL_2         = 4,
    SAY_YULON_END            = 5,

    //Niuzao
    SAY_NIUZAO_CHARGE_1      = 2,
    SAY_NIUZAO_PLAYER_DEATH  = 3,
    SAY_NIUZAO_QUAKE         = 4,
    SAY_NIUZAO_CHARGE_2      = 5,
    SAY_NIUZAO_END           = 6
};

enum Npc
{
    NPC_SHAOHAO         = 73303,
    NPC_CHIJI           = 71952,
    NPC_XUEN            = 71953,
    NPC_YULON           = 71955,
    NPC_NIUZAO          = 71954,
    //Summons
    NPC_FIRESTORM       = 71971,
    NPC_BEACON_OF_HOPE  = 71978,
    NPC_CHILD_OF_CHIJI  = 71990,
    NPC_JADEFIRE_BLAZE  = 72016,
    NPC_JADEFIRE_WALL   = 72020
};

enum Spells
{
    SPELL_CELESTIAL_SPAWN       = 149307,
    //ChiJi
    SPELL_FIRESTORM             = 144461,
    SPELL_FIRESTORM_AURA        = 144463,
    SPELL_INSPIRING_SONG        = 144468,
    SPELL_BEACON_OF_HOPE        = 144473,
    SPELL_BEACON_OF_HOPE_AURA   = 144474,
    SPELL_BLAZING_SONG          = 144471,
    SPELL_BLAZING_NOVA          = 144493,
    SPELL_CRANE_RUSH            = 144470,
    //Xuen
    SPELL_CHI_BARRAGE           = 144642,
    SPELL_CRACKLING_LIGHTNING   = 144635,
    SPELL_LEAP                  = 144640,
    SPELL_SPECTRAL_SWIPE        = 144638,
    SPELL_AGILITY               = 144631,
    //Yulon
    SPELL_JADEFLAME_BUFFET      = 144630,
    SPELL_JADEFIRE_BREATH       = 144530,
    SPELL_JADEFIRE_BOLT         = 144545,
    SPELL_JADEFIRE_BLAZE        = 144537,
    SPELL_JADEFIRE_WALL         = 144533,
    //Niuzao
    SPELL_MASSIVE_QUAKE         = 144611,
    SPELL_OXEN_FORTITUDE        = 144606,
    SPELL_HEADBUTT              = 144610,
    SPELL_CHARGE                = 144608,
    
    //Ordos
    SPELL_BANISHMENT            = 148705,
    
    //Other
    SPELL_GHOSTLY_VOID          = 147495,
    SPELL_DESATURATE            = 129290,
    SPELL_SPIRIT_STRANGLE       = 144059
};

enum Events
{
    //Annoncer
    EVENT_CHIJI_START       = 1,
    EVENT_XUEN_START        = 2,
    EVENT_YULON_START       = 3,
    EVENT_NIUZAO_START      = 4,
    EVENT_CHIJI_END         = 5,
    EVENT_XUEN_END          = 6,
    EVENT_YULON_END         = 7,
    EVENT_NIUZAO_END        = 8,
    //ChiJi
    EVENT_FIRESTORM         = 3,
    EVENT_INSPIRING_SONG    = 4,
    EVENT_BEACON            = 5,
    EVENT_BLAZING_SONG      = 6,
    //Xuen
    EVENT_CHI_BARRAGE       = 3,
    EVENT_LIGHTNING         = 4,
    EVENT_LEAP              = 5,
    EVENT_SPECTRAL_SWIPE    = 6,
    EVENT_AGILITY           = 7,
    //Yulon
    EVENT_JADEFLAME_BUFFET  = 3,
    EVENT_JADEFIRE_BREATH   = 4,
    EVENT_JADEFIRE_BOLT     = 5,
    EVENT_FIRE_WALL         = 6,
    //Niuzao
    EVENT_MASSIVE_QUAKE     = 3,
    EVENT_OXEN_FORTITUDE    = 4,
    EVENT_HEADBUTT          = 5,
    EVENT_MOVE_PATH_1       = 6,
    EVENT_MOVE_PATH_2       = 7
};

enum Actions
{
    ACTION_MOVE_CENTR_POSS  = 1,
    ACTION_CHIJI_END        = 2,
    ACTION_CHIJI_FAIL       = 3,
    ACTION_XUEN_END         = 4,
    ACTION_XUEN_FAIL        = 5,
    ACTION_YULON_END        = 6,
    ACTION_YULON_FAIL       = 7,
    ACTION_NIUZAO_END       = 8,
    ACTION_NIUZAO_FAIL      = 9
};

const Position summonPos[4] = 
{
    {-553.17f, -4949.02f, -6.27f, -2.30f},
    {-740.80f, -4955.08f, -6.27f, 5.63f},
    {-746.32f, -5078.48f, -6.27f, 0.69f},
    {-551.16f, -5080.01f, -6.27f, 2.28f}
};

const Position CentrPos[1] =
{
    {-650.14f, -5024.67f, -6.27f}
};

const Position WallPos[7] =
{
    {-680.63f, -5091.60f, -6.19f, 1.58f},
    {-713.89f, -5091.50f, -6.19f, 1.57f},
    {-580.55f, -5091.66f, -6.19f, 1.58f},
    {-613.91f, -5091.65f, -6.19f, 1.54f},
    {-547.26f, -5091.75f, -6.19f, 1.56f},
    {-647.20f, -5091.63f, -6.19f, 1.57f},
    {-747.19f, -5091.65f, -6.19f, 1.59f}
};

Position const niuzaoWP[8] =
{
    { -573.247f, -5024.76f, -6.27667f, 0.0f },
    { -574.627f, -5074.96f, -6.27667f, 0.0f },
    { -674.698f, -5074.79f, -6.27722f, 0.0f },
    { -741.14f, -5074.97f, -6.27722f, 0.0f },
    { -741.446f, -5017.58f, -6.27723f, 0.0f },
    { -741.128f, -4958.48f, -6.27723f, 0.0f },
    { -652.9f, -4958.67f, -6.27723f, 0.0f },
    { -572.782f, -4960.81f, -6.27723f, 0.0f }
};

class npc_emperor_shaohao : public CreatureScript
{
public:
    npc_emperor_shaohao() : CreatureScript("npc_emperor_shaohao") {}

    struct npc_emperor_shaohaoAI : public ScriptedAI
    {
        npc_emperor_shaohaoAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

        EventMap events;
        SummonList summons;

        void Reset() 
        {
            me->SummonCreature(NPC_CHIJI, summonPos[0]);
            me->SummonCreature(NPC_XUEN, summonPos[1]);
            me->SummonCreature(NPC_YULON, summonPos[2]);
            me->SummonCreature(NPC_NIUZAO, summonPos[3]);

            events.RescheduleEvent(urand(EVENT_CHIJI_START, EVENT_NIUZAO_START), 60000);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_CHIJI_END:
                    events.RescheduleEvent(EVENT_CHIJI_END, 10000);
                    break;
                case ACTION_CHIJI_FAIL:
                    events.RescheduleEvent(EVENT_CHIJI_START, 30000);
                    break;
                case ACTION_XUEN_END:
                    events.RescheduleEvent(EVENT_XUEN_END, 10000);
                    break;
                case ACTION_XUEN_FAIL:
                    events.RescheduleEvent(EVENT_XUEN_START, 30000);
                    break;
                case ACTION_YULON_END:
                    events.RescheduleEvent(EVENT_YULON_END, 10000);
                    break;
                case ACTION_YULON_FAIL:
                    events.RescheduleEvent(EVENT_YULON_START, 30000);
                    break;
                case ACTION_NIUZAO_END:
                    events.RescheduleEvent(EVENT_NIUZAO_END, 10000);
                    break; 
                case ACTION_NIUZAO_FAIL:
                    events.RescheduleEvent(EVENT_NIUZAO_START, 30000);
                    break;
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHIJI_START:
                    {
                        Talk(SAY_CHIJI_START);
                        EntryCheckPredicate pred1(NPC_CHIJI);
                        summons.DoAction(ACTION_MOVE_CENTR_POSS, pred1);
                        break;
                    }
                    case EVENT_CHIJI_END:
                        Talk(SAY_CHIJI_FINISH);
                        events.RescheduleEvent(EVENT_XUEN_START, 60000);
                        break;
                    case EVENT_XUEN_START:
                    {
                        Talk(SAY_XUEN_START);
                        EntryCheckPredicate pred1(NPC_XUEN);
                        summons.DoAction(ACTION_MOVE_CENTR_POSS, pred1);
                        break;
                    }
                    case EVENT_XUEN_END:
                        Talk(SAY_XUEN_FINISH);
                        events.RescheduleEvent(EVENT_YULON_START, 60000);
                        break;
                    case EVENT_YULON_START:
                    {
                        Talk(SAY_YULON_START);
                        EntryCheckPredicate pred1(NPC_YULON);
                        summons.DoAction(ACTION_MOVE_CENTR_POSS, pred1);
                        break;
                    }
                    case EVENT_YULON_END:
                    {
                        Talk(SAY_YULON_FINISH);
                        events.RescheduleEvent(EVENT_NIUZAO_START, 60000);
                        break;
                    }
                    case EVENT_NIUZAO_START:
                    {
                        Talk(SAY_NIUZAO_START);
                        EntryCheckPredicate pred1(NPC_NIUZAO);
                        summons.DoAction(ACTION_MOVE_CENTR_POSS, pred1);
                        break;
                    }
                    case EVENT_NIUZAO_END:
                    {
                        Talk(SAY_NIUZAO_FINISH);
                        events.RescheduleEvent(EVENT_CHIJI_START, 60000);
                        break;
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_emperor_shaohaoAI (creature);
    }
};

class boss_chiji : public CreatureScript
{
public:
    boss_chiji() : CreatureScript("boss_chiji") {}

    struct boss_chijiAI : public ScriptedAI
    {
        boss_chijiAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

        EventMap events;
        SummonList summons;
        uint32 orient;

        bool EventProgress;
        bool bEvent_1;
        bool bEvent_2;

        void Reset() 
        {
            bEvent_1 = false;
            bEvent_2 = false;
            EventProgress = false;
            me->setFaction(35);
            me->RemoveAllAuras();
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            events.RescheduleEvent(EVENT_FIRESTORM, 10000);
            events.RescheduleEvent(EVENT_INSPIRING_SONG, 20000);
            events.RescheduleEvent(EVENT_BEACON, 40000);
            events.RescheduleEvent(EVENT_BLAZING_SONG, 44000);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterEvadeMode()
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (EventProgress)
            {
                me->NearTeleportTo(summonPos[0].GetPositionX(), summonPos[0].GetPositionY(), summonPos[0].GetPositionZ(), summonPos[0].GetOrientation());
                me->SetHomePosition(summonPos[0]);
                if (auto Shao = me->GetAnyOwner())
                    Shao->GetAI()->DoAction(ACTION_CHIJI_FAIL);
            }
            else
                events.RescheduleEvent(EVENT_2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_MOVE_CENTR_POSS:
                    EventProgress = true;
                    me->SetHomePosition(CentrPos[0]);
                    me->GetMotionMaster()->MovePoint(1, CentrPos[0]);
                    break;
            }
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType dmgType)
        {
            if (!who || !who->IsPlayer())
                return;

            if (me->HealthBelowPct(66) && !bEvent_1)
            {
                bEvent_1 = true;
                Talk(SAY_CHIJI_RUSH_1);
                DoCast(SPELL_CRANE_RUSH);
            }

            if (me->HealthBelowPct(33) && !bEvent_2)
            {
                bEvent_2 = true;
                Talk(SAY_CHIJI_RUSH_2);
                DoCast(SPELL_CRANE_RUSH);
            }

            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    summons.DespawnAll();
                    me->GeneratePersonalLoot(me, (Player*)who);
                    Talk(SAY_CHIJI_END);
                    if (auto Shao = me->GetAnyOwner())
                        Shao->GetAI()->DoAction(ACTION_CHIJI_END);
                }
                EnterEvadeMode();
            }
        }

        void QuestCredit()
        {
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Player *pTarget = Player::GetPlayer(*me, (*itr)->getUnitGuid());
                if (!pTarget)
                    return;

                pTarget->KilledMonsterCredit(me->GetEntry());
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                Talk(SAY_ENTER_POS);
                me->SetFacingTo(1.5f);
                events.RescheduleEvent(EVENT_1, 5000);
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            summon->SetReactState(REACT_PASSIVE);

            if (summon->GetEntry() == NPC_FIRESTORM)
                summon->AI()->DoCast(SPELL_FIRESTORM_AURA);

            if (summon->GetEntry() == NPC_BEACON_OF_HOPE)
                summon->AI()->DoCast(SPELL_BEACON_OF_HOPE_AURA);

            if (summon->GetEntry() == NPC_CHILD_OF_CHIJI)
            {
                summon->AI()->DoCast(SPELL_BLAZING_NOVA);
                orient = summon->GetOrientation();
                summon->SetOrientation(urand(0, 6));
                float x, y, z;
                summon->GetClosePoint(x, y, z, me->GetObjectSize(), 70.0f);
                summon->GetMotionMaster()->MovePoint(1, x, y, z);
                summon->DespawnOrUnsummon(10000);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            EnterEvadeIfOutOfCombatArea(diff);
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_CRANE_RUSH))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, 768);
                        me->setFaction(190);
                        break;
                    case EVENT_FIRESTORM:
                        DoCast(SPELL_FIRESTORM);
                        events.RescheduleEvent(EVENT_FIRESTORM, 20000);
                        break;
                    case EVENT_INSPIRING_SONG:
                        DoCast(SPELL_INSPIRING_SONG);
                        events.RescheduleEvent(EVENT_INSPIRING_SONG, 28000);
                        break;
                    case EVENT_BEACON:
                        Talk(SAY_CHIJI_BEACON);
                        if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            DoCast(pTarget, SPELL_BEACON_OF_HOPE);
                        events.RescheduleEvent(EVENT_BEACON, 76000);
                        break;
                    case EVENT_BLAZING_SONG:
                        Talk(SAY_CHIJI_BLAZ_SONG);
                        DoCast(SPELL_BLAZING_SONG);
                        events.RescheduleEvent(EVENT_BLAZING_SONG, 76000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[0].GetPositionX(), summonPos[0].GetPositionY(), summonPos[0].GetPositionZ(), summonPos[0].GetOrientation());
                        me->SetHomePosition(summonPos[0]);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_chijiAI (creature);
    }
};

class boss_xuen : public CreatureScript
{
public:
    boss_xuen() : CreatureScript("boss_xuen") {}

    struct boss_xuenAI : public ScriptedAI
    {
        boss_xuenAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool EventProgress;

        void Reset() 
        {
            EventProgress = false;
            me->setFaction(35);
            me->RemoveAllAuras();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            events.RescheduleEvent(EVENT_CHI_BARRAGE, 18000);
            events.RescheduleEvent(EVENT_LIGHTNING, 36000);
            events.RescheduleEvent(EVENT_LEAP, 12000);
            events.RescheduleEvent(EVENT_SPECTRAL_SWIPE, 8000);
            events.RescheduleEvent(EVENT_AGILITY, 28000);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterEvadeMode()
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (EventProgress)
            {
                me->NearTeleportTo(summonPos[1].GetPositionX(), summonPos[1].GetPositionY(), summonPos[1].GetPositionZ(), summonPos[1].GetOrientation());
                me->SetHomePosition(summonPos[1]);
                if (auto Shao = me->GetAnyOwner())
                    Shao->GetAI()->DoAction(ACTION_XUEN_FAIL);
            }
            else
                events.RescheduleEvent(EVENT_2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim && victim->ToPlayer())
                Talk(SAY_XUEN_PLAYER_DEATH);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_MOVE_CENTR_POSS:
                    EventProgress = true;
                    me->SetHomePosition(CentrPos[0]);
                    me->GetMotionMaster()->MovePoint(1, CentrPos[0]);
                    break;
            }
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType dmgType)
        {
            if (!who || !who->IsPlayer())
                return;

            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    me->GeneratePersonalLoot(me, (Player*)who);
                    Talk(SAY_XUEN_END);
                    if (auto Shao = me->GetAnyOwner())
                        Shao->GetAI()->DoAction(ACTION_XUEN_END);
                }
                EnterEvadeMode();
            }
        }

        void QuestCredit()
        {
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Player *pTarget = Player::GetPlayer(*me, (*itr)->getUnitGuid());
                if (!pTarget)
                    return;

                pTarget->KilledMonsterCredit(me->GetEntry());
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                Talk(SAY_ENTER_POS);
                me->SetFacingTo(1.5f);
                events.RescheduleEvent(EVENT_1, 5000);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);
            EnterEvadeIfOutOfCombatArea(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, 768);
                        me->setFaction(190);
                        break;
                    case EVENT_CHI_BARRAGE:
                        Talk(SAY_XUEN_BARRAGE);
                        DoCast(SPELL_CHI_BARRAGE);
                        events.RescheduleEvent(EVENT_CHI_BARRAGE, 34000);
                        break;
                    case EVENT_LIGHTNING:
                        Talk(SAY_XUEN_LIGHTNING);
                        DoCast(SPELL_CRACKLING_LIGHTNING);
                        events.RescheduleEvent(EVENT_LIGHTNING, 46000);
                        break;
                    case EVENT_LEAP:
                        if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                            DoCast(pTarget, SPELL_LEAP);
                        events.RescheduleEvent(EVENT_LEAP, 48000);
                        break;
                    case EVENT_SPECTRAL_SWIPE:
                        if (auto target = me->getVictim())
                            DoCast(target, SPELL_SPECTRAL_SWIPE);
                        events.RescheduleEvent(EVENT_SPECTRAL_SWIPE, 14000);
                        break;
                    case EVENT_AGILITY:
                        Talk(SAY_XUEN_AGGILITY);
                        DoCast(SPELL_AGILITY);
                        events.RescheduleEvent(EVENT_AGILITY, 60000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[1].GetPositionX(), summonPos[1].GetPositionY(), summonPos[1].GetPositionZ(), summonPos[1].GetOrientation());
                        me->SetHomePosition(summonPos[1]);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_xuenAI (creature);
    }
};

class boss_yulon : public CreatureScript
{
public:
    boss_yulon() : CreatureScript("boss_yulon") {}

    struct boss_yulonAI : public ScriptedAI
    {
        boss_yulonAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

        EventMap events;
        SummonList summons;
        bool EventProgress;
        bool wallrand;

        void Reset() 
        {
            EventProgress = false;
            me->setFaction(35);
            me->RemoveAllAuras();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            events.RescheduleEvent(EVENT_JADEFLAME_BUFFET, 20000);
            events.RescheduleEvent(EVENT_JADEFIRE_BREATH, 4000);
            events.RescheduleEvent(EVENT_JADEFIRE_BOLT, 21000);
            events.RescheduleEvent(EVENT_FIRE_WALL, 46000);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterEvadeMode()
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
            summons.DespawnAll();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (EventProgress)
            {
                me->NearTeleportTo(summonPos[2].GetPositionX(), summonPos[2].GetPositionY(), summonPos[2].GetPositionZ(), summonPos[2].GetOrientation());
                me->SetHomePosition(summonPos[2]);
                if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                    if (auto Shao = me->ToTempSummon()->GetSummoner()->ToCreature())
                        Shao->AI()->DoAction(ACTION_YULON_FAIL);
            }
            else
                events.RescheduleEvent(EVENT_2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim && victim->ToPlayer())
                Talk(SAY_YULON_PLAYER_DEATH);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_MOVE_CENTR_POSS:
                    EventProgress = true;
                    me->SetHomePosition(CentrPos[0]);
                    me->GetMotionMaster()->MovePoint(1, CentrPos[0]);
                    break;
            }
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType dmgType)
        {
            if (!who || !who->IsPlayer())
                return;

            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    me->GeneratePersonalLoot(me, (Player*)who);
                    Talk(SAY_YULON_END);
                    if (auto Shao = me->GetAnyOwner())
                        Shao->GetAI()->DoAction(ACTION_YULON_END);
                }
                EnterEvadeMode();
            }
        }

        void QuestCredit()
        {
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Player *pTarget = Player::GetPlayer(*me, (*itr)->getUnitGuid());
                if (!pTarget)
                    return;

                pTarget->KilledMonsterCredit(me->GetEntry());
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);

            summon->SetReactState(REACT_PASSIVE);
            summon->SetDisplayId(11686);

            if (summon->GetEntry() == NPC_JADEFIRE_WALL)
            {
                summon->AI()->DoCast(SPELL_JADEFIRE_WALL);
                float x, y, z;
                summon->GetClosePoint(x, y, z, me->GetObjectSize(), 150.0f);
                summon->GetMotionMaster()->MovePoint(1, x, y, z);
                summon->DespawnOrUnsummon(15000);
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                Talk(SAY_ENTER_POS);
                me->SetFacingTo(1.5f);
                events.RescheduleEvent(EVENT_1, 5000);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);
            EnterEvadeIfOutOfCombatArea(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, 768);
                        me->setFaction(190);
                        break;
                    case EVENT_JADEFLAME_BUFFET:
                        DoCast(SPELL_JADEFLAME_BUFFET);
                        events.RescheduleEvent(EVENT_JADEFLAME_BUFFET, 20000);
                        break;
                    case EVENT_JADEFIRE_BREATH:
                        if (auto target = me->getVictim())
                            DoCast(target, SPELL_JADEFIRE_BREATH);
                        events.RescheduleEvent(EVENT_JADEFIRE_BREATH, 18000);
                        break;
                    case EVENT_JADEFIRE_BOLT:
                        DoCast(SPELL_JADEFIRE_BOLT);
                        events.RescheduleEvent(EVENT_JADEFIRE_BOLT, 18000);
                        break;
                    case EVENT_FIRE_WALL:
                    {
                        Talk(urand(SAY_YULON_WALL_1, SAY_YULON_WALL_2));
                        bool wall = true;
                        for(uint8 i = 0; i < 7; i++)
                        {
                            if (wall && i != 7 && roll_chance_i(15))
                                wall = false;
                            else
                                me->SummonCreature(72020, WallPos[i]);
                        }
                        events.RescheduleEvent(EVENT_FIRE_WALL, 60000);
                        break;
                    }
                    case EVENT_2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[2].GetPositionX(), summonPos[2].GetPositionY(), summonPos[2].GetPositionZ(), summonPos[2].GetOrientation());
                        me->SetHomePosition(summonPos[2]);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_yulonAI (creature);
    }
};

class boss_niuzao : public CreatureScript
{
public:
    boss_niuzao() : CreatureScript("boss_niuzao") {}

    struct boss_niuzaoAI : public ScriptedAI
    {
        boss_niuzaoAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool EventProgress;
        bool bEvent_1;
        bool bEvent_2;
        bool ChargeEvent;

        void Reset() 
        {
            EventProgress = false;
            bEvent_1 = false;
            bEvent_2 = false;
            ChargeEvent = false;
            me->SetReactState(REACT_AGGRESSIVE);
            me->setFaction(35);
            me->RemoveAllAuras();
        }

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO);
            events.RescheduleEvent(EVENT_MASSIVE_QUAKE, 45000);
            events.RescheduleEvent(EVENT_OXEN_FORTITUDE, 30000);
            events.RescheduleEvent(EVENT_HEADBUTT, 30000);
        }

        void EnterEvadeMode()
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (EventProgress)
            {
                me->NearTeleportTo(summonPos[3].GetPositionX(), summonPos[3].GetPositionY(), summonPos[3].GetPositionZ(), summonPos[3].GetOrientation());
                me->SetHomePosition(summonPos[3]);
                if (auto Shao = me->GetAnyOwner())
                    Shao->GetAI()->DoAction(ACTION_NIUZAO_FAIL);
            }
            else
                events.RescheduleEvent(EVENT_2, 5000);

            ScriptedAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (victim && victim->ToPlayer())
                Talk(SAY_NIUZAO_PLAYER_DEATH);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_MOVE_CENTR_POSS:
                    me->SetSpeed(MAX_MOVE_TYPE, 2.0f);
                    EventProgress = true;
                    me->SetHomePosition(CentrPos[0]);
                    me->GetMotionMaster()->MovePoint(1, CentrPos[0]);
                    break;
            }
        }

        void DamageTaken(Unit* who, uint32& damage, DamageEffectType dmgType)
        {
            if (!who || !who->IsPlayer())
                return;

            if (me->HealthBelowPct(66) && !bEvent_1 && !ChargeEvent)
            {
                bEvent_1 = true;
                Talk(SAY_NIUZAO_CHARGE_1);
                events.RescheduleEvent(EVENT_MOVE_PATH_1, 100);
            }

            if (me->HealthBelowPct(33) && !bEvent_2 && !ChargeEvent)
            {
                bEvent_2 = true;
                Talk(SAY_NIUZAO_CHARGE_2);
                events.RescheduleEvent(EVENT_MOVE_PATH_1, 100);
            }

            if (damage >= me->GetHealth())
            {
                damage = 0;

                if (EventProgress)
                {
                    EventProgress = false;
                    ChargeEvent = false;
                    QuestCredit();
                    me->setFaction(35);
                    me->RemoveAllAuras();
                    me->GeneratePersonalLoot(me, (Player*)who);
                    Talk(SAY_NIUZAO_END);
                    if (auto Shao = me->GetAnyOwner())
                        Shao->GetAI()->DoAction(ACTION_NIUZAO_END);
                }
                EnterEvadeMode();
            }
        }

        void QuestCredit()
        {
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Player *pTarget = Player::GetPlayer(*me, (*itr)->getUnitGuid());
                if (!pTarget)
                    return;

                pTarget->KilledMonsterCredit(me->GetEntry());
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (id == 1)
                {
                    Talk(SAY_ENTER_POS);
                    me->SetFacingTo(1.5f);
                    events.RescheduleEvent(EVENT_1, 5000);
                }
                if (id == 2)
                {
                    DoCast(SPELL_CHARGE);
                    events.RescheduleEvent(EVENT_MOVE_PATH_2, 3000);
                }
                if (id == 3)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(4, niuzaoWP[1]);
                    });
                if (id == 4)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(5, niuzaoWP[2]);
                    });
                if (id == 5)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(6, niuzaoWP[3]);
                    });
                if (id == 6)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(7, niuzaoWP[4]);
                    });
                if (id == 7)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(8, niuzaoWP[5]);
                    });
                if (id == 8)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(9, niuzaoWP[6]);
                    });
                if (id == 9)
                    me->AddDelayedCombat(100, [=]() -> void
                    {
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(10, niuzaoWP[7]);
                    });
                if (id == 10)
                {
                    me->RemoveAura(SPELL_CHARGE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    ChargeEvent = false;
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            EnterEvadeIfOutOfCombatArea(diff);
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || ChargeEvent)
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, 768);
                        me->setFaction(190);
                        break;
                    case EVENT_MASSIVE_QUAKE:
                        Talk(SAY_NIUZAO_QUAKE);
                        DoCast(SPELL_MASSIVE_QUAKE);
                        events.RescheduleEvent(EVENT_MASSIVE_QUAKE, 48000);
                        break;
                    case EVENT_OXEN_FORTITUDE:
                        DoCast(SPELL_OXEN_FORTITUDE);
                        events.RescheduleEvent(EVENT_OXEN_FORTITUDE, 46000);
                        break;
                    case EVENT_HEADBUTT:
                        if (auto target = me->getVictim())
                        {
                            DoCast(target, SPELL_HEADBUTT);
                            DoModifyThreatPercent(target, -100);
                        }
                        events.RescheduleEvent(EVENT_HEADBUTT, 30000);
                        break;
                    case EVENT_MOVE_PATH_1:
                        me->StopAttack(true);
                        me->GetMotionMaster()->MovePoint(2, CentrPos[0]);
                        break;
                    case EVENT_MOVE_PATH_2:
                        me->GetMotionMaster()->MovePoint(3, niuzaoWP[0]);
                        ChargeEvent = true;
                        break;
                    case EVENT_2:
                        DoCast(SPELL_CELESTIAL_SPAWN);
                        me->NearTeleportTo(summonPos[3].GetPositionX(), summonPos[3].GetPositionY(), summonPos[3].GetPositionZ(), summonPos[3].GetOrientation());
                        me->SetHomePosition(summonPos[3]);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_niuzaoAI (creature);
    }
};
class npc_timeless_spirit : public CreatureScript
{
public:
    npc_timeless_spirit() : CreatureScript("npc_timeless_spirit") {}

    struct npc_timeless_spiritAI : public ScriptedAI
    {
        npc_timeless_spiritAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 SpiritStrangle_timer;

        void Reset() 
        {
            SpiritStrangle_timer = 0;
            //me->GetMotionMaster()->MoveRandom(10.0f);
            DoCast(SPELL_GHOSTLY_VOID);
            DoCast(SPELL_DESATURATE);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (Unit* pTarget = me->getVictim())
            {
                if (me->GetDistance(pTarget) >= 15.0f)
                    EnterEvadeMode();

                if (SpiritStrangle_timer <= diff)
                {
                    DoCast(pTarget, SPELL_SPIRIT_STRANGLE);
    
                    SpiritStrangle_timer = 7000;
                }
                else SpiritStrangle_timer -= diff;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_timeless_spiritAI (creature);
    }
};

class at_ordos_entrance : public AreaTriggerScript
{
public:
    at_ordos_entrance() : AreaTriggerScript("at_ordos_entrance") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (pPlayer->isGameMaster())
            return false;

        // A Pandaren Legend
        if (pPlayer->GetQuestStatus(33104) != QUEST_STATUS_REWARDED && !pPlayer->HasAchieved(8325))
            pPlayer->CastSpell(pPlayer, SPELL_BANISHMENT, true);

        return false;
    }
};

enum eZarimEvents
{
    QUEST_BONE_APART_INTRO      = 33348,
    NPC_ZARIM                   = 71876,
    SPELL_BONE_APART_INTRO      = 149122,
};
// AT - 9211
class at_tom_bone_apart : public AreaTriggerScript
{
    public:
        at_tom_bone_apart() : AreaTriggerScript("at_tom_bone_apart") {}

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/, bool /*enter*/)
        {
            if (Creature* Zarim = GetClosestCreatureWithEntry(player, NPC_ZARIM, 30.0f))
            {
                if (player->GetDailyQuestStatus(QUEST_BONE_APART_INTRO) != QUEST_STATUS_REWARDED)
                {
                    player->CastSpell(player, SPELL_BONE_APART_INTRO, true);
                    Zarim->AI()->Talk(0);
                }
            }
            return false;
        }
};

class spell_chi_barrage : public SpellScriptLoader
{
public:
    spell_chi_barrage() : SpellScriptLoader("spell_chi_barrage") { }

    class spell_chi_barrage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_chi_barrage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Trinity::Containers::RandomResizeList(targets, 6);
        }

        void HandleOnHit()
        {
            if (Unit* target = GetHitUnit())
                GetCaster()->CastSpell(target, GetSpellInfo()->Effects[EFFECT_0]->BasePoints);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_chi_barrage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_chi_barrage_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_chi_barrage_SpellScript();
    }
};

class spell_crackling_lightning : public SpellScriptLoader
{
public:
    spell_crackling_lightning() : SpellScriptLoader("spell_crackling_lightning") { }

    class spell_crackling_lightning_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_crackling_lightning_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Trinity::Containers::RandomResizeList(targets, 1);
        }

        void HandleOnHit()
        {
            if (Unit* target = GetHitUnit())
                GetCaster()->CastSpell(target, GetSpellInfo()->Effects[EFFECT_0]->BasePoints);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_crackling_lightning_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_crackling_lightning_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_crackling_lightning_SpellScript();
    }
};

class spell_jadefire_bolt : public SpellScriptLoader
{
public:
    spell_jadefire_bolt() : SpellScriptLoader("spell_jadefire_bolt") { }

    class spell_jadefire_bolt_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_jadefire_bolt_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            Trinity::Containers::RandomResizeList(targets, 4);
        }

        void HandleOnHit()
        {
            if (Unit* target = GetHitUnit())
                GetCaster()->CastSpell(target, GetSpellInfo()->Effects[EFFECT_0]->BasePoints);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_jadefire_bolt_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_jadefire_bolt_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_jadefire_bolt_SpellScript();
    }
};

void AddSC_timeless_isle()
{
    new npc_emperor_shaohao();
    new boss_chiji();
    new boss_xuen();
    new boss_yulon();
    new boss_niuzao();
    new npc_timeless_spirit();
    //new at_ordos_entrance();
    //new at_tom_bone_apart();
    new spell_chi_barrage();
    new spell_crackling_lightning();
    new spell_jadefire_bolt();
}