#include "upper_blackrock_spire.h"
#include "MoveSplineInit.h"
#include "ScriptedEscortAI.h"

enum Events
{
    // Leeroy Jenkins
    TALK_0 = 1,
    TALK_1,
    TALK_2,
    TALK_3,
    TALK_4,
    EVENT_UPDATE_CHICKEN,
    EVENT_CHICKEN,
    EVENT_MOVE,
    EVENT_BACK,
    EVENT_SHOULDERS,
    EVENT_SHOULDERS_APPLY,
    EVENT_FINAL,
    EVENT_MOVE_2,
    EVENT_MOVE_3,

    // Son Of The Beast
    EVENT_FIERY_CHARGE = 1,
    EVENT_CHARGE_STOP,
    EVENT_FLAME_ERUPTION,
    EVENT_TERRIFYING_ROAL
};

enum Spells
{
    // Leeroy Jenkins
    SPELL_CLASS_SPECIFIC_RES = 157175,
    SPELL_PERMANENT_FEIGN_DEATH = 29266,
    SPELL_CHIKEN_TIMER = 172584,
    SPELL_DEVOUT_SHOULDERS = 166563,
    SPELL_COSMETIC_HEARTHSTONE = 101188,
    SPELL_FOLLOWER_LEEROY = 174984,

    // Son Of The Beast
    SPELL_FIERY_CHARGE = 157347,
    SPELL_FIERY_TRAIL = 157364,
    SPELL_FLAME_ERUPTION = 157467,
    SPELL_TERRIFYING_ROAR = 157428
};

enum Talks
{
    // Leeroy Jenkins
    TALK_INTRO_1,
    TALK_INTRO_2,
    TALK_INTRO_3,
    TALK_INTRO_4,
    TALK_INTRO_5,
    TALK_START,
    TALK_SHOULDERS,
    TALK_GARRISON,
    TALK_INTRO_6,
    TALK_INTRO_7
};

enum Achievements
{
    ACHIEVEMENT_LEEEEEEEEEEEEEROY = 9058,
};

class npc_leeroy_jenkins : public CreatureScript
{
public:
    npc_leeroy_jenkins() : CreatureScript("npc_leeroy_jenkins") { }

    struct npc_leeroy_jenkinsAI : public ScriptedAI
    {
        npc_leeroy_jenkinsAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            ChickenTimer = 0;
            Check = false;
            intro = false;
        }

        EventMap events;
        InstanceScript* instance;
        SummonList summons;

        bool Check;
        bool intro;
        uint32 ChickenTimer;
        uint32 EventFinalJenkins;
        ObjectGuid SonOfBeastGUID;

        void Reset()
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void JustSummoned(Creature* sum) override
        {
            summons.Summon(sum);

            if (sum->GetEntry() == NPC_SON_OF_THE_BEAST)
            {
                SonOfBeastGUID.Clear();
                SonOfBeastGUID = sum->GetGUID();
            }
        }

        void EnterCombat(Unit* /*attacker*/)
        {
            Movement::MoveSplineInit init(*me);
            init.Stop();
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
            {
                instance->DoUpdateWorldState(static_cast<WorldStates>(9523), 0);
                instance->DoUpdateWorldState(static_cast<WorldStates>(9524), 0);
            }
            me->DespawnOrUnsummon();
            summons.DespawnAll();
        }

        void SummonSonOfBeast()
        {
            me->SummonCreature(NPC_SON_OF_THE_BEAST, 87.6401f, -553.402f, 111.009f, 2.62025f);
        }

        void SpellHit(Unit* caster, SpellInfo const* spellInfo)
        {
            if (IsHeroic() && instance->GetBossState(DATA_KYRAK) == DONE)
            {
                if (!spellInfo->HasEffect(SPELL_EFFECT_RESURRECT) || !me->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
                    return;

                me->SetUInt32Value(UNIT_FIELD_INTERACT_SPELL_ID, 0);
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, ANIM_KNEEL_LOOP);
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH | UNIT_FLAG_NON_ATTACKABLE);
                me->SetFullHealth();
                me->RemoveAura(SPELL_PERMANENT_FEIGN_DEATH);
                me->SetOrientation(1.4885f);
                me->SetFacingTo(1.4885f);
                me->SetWalk(true);
                SummonSonOfBeast();

                events.RescheduleEvent(TALK_0, 1000);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case TALK_0:
                Talk(TALK_INTRO_1);
                events.RescheduleEvent(TALK_1, 8000);
                break;
            case TALK_1:
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, 0);
                Talk(TALK_INTRO_2);
                me->GetMotionMaster()->MovePoint(0, 74.720f, -319.148f, 91.446f);
                events.RescheduleEvent(TALK_2, 13000);
                break;
            case TALK_2:
                Talk(TALK_INTRO_3);
                events.RescheduleEvent(TALK_3, 9000);
                break;
            case TALK_3:
                Talk(TALK_INTRO_4);
                events.RescheduleEvent(TALK_4, 10000);
                break;
            case TALK_4:
            {
                Talk(TALK_INTRO_5);
                events.RescheduleEvent(EVENT_MOVE, 6500);
                break;
            }
            case EVENT_UPDATE_CHICKEN:
            {
                if (ChickenTimer)
                {
                    --ChickenTimer;
                    events.RescheduleEvent(EVENT_UPDATE_CHICKEN, 1000);
                }

                if (instance)
                    instance->DoUpdateWorldState(static_cast<WorldStates>(9524), ChickenTimer);
                break;
            }
            case EVENT_CHICKEN:
            {
                if (instance)
                {
                    instance->DoUpdateWorldState(static_cast<WorldStates>(9523), 0);
                    instance->DoUpdateWorldState(static_cast<WorldStates>(9524), 0);
                }

                me->RemoveAura(SPELL_CHIKEN_TIMER);
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);

                Talk(TALK_START);
                me->SetWalk(false);

                Movement::MoveSplineInit init(*me);
                init.Path().push_back(G3D::Vector3(63.575f, -310.544f, 91.5003f));
                init.Path().push_back(G3D::Vector3(82.85f, -304.639f, 91.4411f));
                init.Path().push_back(G3D::Vector3(78.21f, -277.6954f, 91.4679f));
                init.Path().push_back(G3D::Vector3(61.92f, -267.67f, 93.4947f));
                init.Path().push_back(G3D::Vector3(62.95f, -248.55f, 97.5597f));
                init.Path().push_back(G3D::Vector3(67.622f, -240.374f, 98.381f));
                init.Path().push_back(G3D::Vector3(103.112f, -243.005f, 106.436f));
                init.Path().push_back(G3D::Vector3(106.794f, -259.459f, 106.436f));
                init.Path().push_back(G3D::Vector3(119.23f, -259.81f, 108.7294f));
                init.Path().push_back(G3D::Vector3(150.768f, -262.547f, 110.908f));
                init.Path().push_back(G3D::Vector3(138.4327f, -345.4686f, 110.9761f));
                init.Path().push_back(G3D::Vector3(136.134f, -364.976f, 116.833f));
                init.Path().push_back(G3D::Vector3(95.402f, -366.344f, 116.838f));
                init.Path().push_back(G3D::Vector3(92.7182f, -393.1929f, 113.9506f));
                init.Path().push_back(G3D::Vector3(92.91f, -434.1419f, 110.9227f));
                init.Path().push_back(G3D::Vector3(92.8158f, -448.8309f, 113.9506f));
                init.Path().push_back(G3D::Vector3(93.032f, -473.292f, 116.842f));
                init.Path().push_back(G3D::Vector3(65.0141f, -474.5555f, 113.9505f));
                init.Path().push_back(G3D::Vector3(35.159f, -475.309f, 110.950f));
                init.Path().push_back(G3D::Vector3(23.331f, -525.586f, 110.942f));
                init.Path().push_back(G3D::Vector3(57.042f, -542.330f, 110.933f));
                init.Path().push_back(G3D::Vector3(86.1871f, -555.602f, 110.9234f));
                init.SetWalk(false);
                init.Launch();
                me->SetReactState(REACT_AGGRESSIVE);
                Check = true;
                break;
            }
            case EVENT_MOVE:
                me->GetMotionMaster()->MovePoint(1, 69.690f, -309.859f, 91.427f);
                break;
            case EVENT_BACK:
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, 0);
                me->GetMotionMaster()->MovePoint(3, 69.690f, -309.859f, 91.427f);
                break;
            case EVENT_SHOULDERS:
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, ANIM_EMOTE_CHEER);
                Talk(TALK_SHOULDERS);
                events.RescheduleEvent(EVENT_SHOULDERS_APPLY, 3000);
                break;
            case EVENT_SHOULDERS_APPLY:
                me->CastSpell(me, SPELL_DEVOUT_SHOULDERS, true);
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, ANIM_EMOTE_FLEX);
                events.RescheduleEvent(EVENT_FINAL, 3000);
                break;
            case EVENT_FINAL:
                Talk(TALK_GARRISON);
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, 0);
                instance->DoCompleteAchievement(ACHIEVEMENT_LEEEEEEEEEEEEEROY);
                instance->DoCastSpellOnPlayers(SPELL_FOLLOWER_LEEROY);
                me->CastSpell(me, SPELL_COSMETIC_HEARTHSTONE, false);
                break;
            case EVENT_MOVE_2:
                me->GetMotionMaster()->MovePoint(2, 49.700f, -308.808f, 91.545f);
                break;
            case EVENT_MOVE_3:
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                Talk(TALK_INTRO_7);
                me->SetWalk(false);
                me->GetMotionMaster()->MovePoint(3, 73.64f, -333.143f, 91.4939f);
                break;
            default:
                break;
            }

            if (Check && me->IsSplineFinished())
            {
                if (instance->GetData(DATA_FINAL_EVENT_JENKINS) == DONE)
                {
                    Check = false;
                    Position pos;
                    if (Creature* SonOfBeast = me->GetCreature(*me, SonOfBeastGUID))
                        SonOfBeast->GetPosition(&pos);
                    pos.m_positionX -= 3.0f;
                    pos.m_positionY -= 3.0f;
                    me->GetMotionMaster()->MovePoint(4, pos);
                }
            }

            if (!intro && instance->GetBossState(DATA_KYRAK) == DONE)
            {
                if (IsHeroic())
                {
                    intro = true;
                    me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            switch (id)
            {
            case 1:
                events.RescheduleEvent(EVENT_MOVE_2, 1000);
                break;
            case 2:
                Talk(TALK_INTRO_6);
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_USE_STANDING);
                me->CastSpell(me, SPELL_CHIKEN_TIMER, true);
                ChickenTimer = 900;
                events.RescheduleEvent(EVENT_UPDATE_CHICKEN, 1000);
                events.RescheduleEvent(EVENT_CHICKEN, 1000000);
                events.RescheduleEvent(EVENT_MOVE_3, 334000);

                if (instance)
                {
                    instance->DoUpdateWorldState(static_cast<WorldStates>(9523), 1);
                    instance->DoUpdateWorldState(static_cast<WorldStates>(9524), ChickenTimer);
                }
                break;
            case 3:
                me->SetOrientation(4.42291f);
                me->SetFacingTo(4.42291f);
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_USE_STANDING);
                me->SetWalk(true);
                break;
            case 4:
                me->SetUInt32Value(UNIT_FIELD_STATE_ANIM_ID, ANIM_KNEEL_LOOP);
                events.RescheduleEvent(EVENT_SHOULDERS, 2000);
                break;
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_leeroy_jenkinsAI(creature);
    }
};

class npc_son_of_the_beast : public CreatureScript
{
public:
    npc_son_of_the_beast() : CreatureScript("npc_son_of_the_beast") { }

    struct npc_son_of_the_beastAI : public ScriptedAI
    {
        npc_son_of_the_beastAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        bool charging;
        EventMap events;
        InstanceScript* instance;

        void Reset()
        {
            events.Reset();
            charging = false;
        }

        void EnterCombat(Unit*)
        {
            events.RescheduleEvent(EVENT_FIERY_CHARGE, 8000);
            events.RescheduleEvent(EVENT_FLAME_ERUPTION, 15000);
            events.RescheduleEvent(EVENT_TERRIFYING_ROAL, 20000);
        }

        void JustDied(Unit*)
        {
            if (!me)
                return;

            me->RemoveAllAreaObjects();

            if (instance->GetData(DATA_FINAL_EVENT_JENKINS) != DONE)
                instance->SetData(DATA_FINAL_EVENT_JENKINS, DONE);
        }

        void EnterEvadeMode() override
        {
            if (!me)
                return;

            me->RemoveAllAreaObjects();
            ScriptedAI::EnterEvadeMode();
        }

        void UpdateAI(uint32 diff) override
        {
            if (charging)
                me->CastSpell(me, SPELL_FIERY_TRAIL, true);

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
            case EVENT_FIERY_CHARGE:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SPELL_FIERY_CHARGE, true);
                events.RescheduleEvent(EVENT_FIERY_CHARGE, 8000);
                events.RescheduleEvent(EVENT_CHARGE_STOP, 600);
                charging = true;
                break;
            case EVENT_CHARGE_STOP:
                charging = false;
                break;
            case EVENT_FLAME_ERUPTION:
                me->CastSpell(me, SPELL_FLAME_ERUPTION, true);
                events.RescheduleEvent(EVENT_FLAME_ERUPTION, 15000);
                break;
            case EVENT_TERRIFYING_ROAL:
                me->CastSpell(me, SPELL_TERRIFYING_ROAR, true);
                events.RescheduleEvent(EVENT_TERRIFYING_ROAL, 20000);
                break;
            default:
                break;
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_son_of_the_beastAI(creature);
    }
};

void AddSC_upper_blackrock_spire()
{
    new npc_leeroy_jenkins();
    new npc_son_of_the_beast();
}
