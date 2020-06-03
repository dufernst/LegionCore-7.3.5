/*
    Dungeon : Shadowmoon Burial Grounds 100
    Encounter: Ner'zhul
*/

#include "CreatureGroups.h"
#include "shadowmoon_burial_grounds.h"

#define MAX_DIST    60

enum Says
{
    SAY_AGGRO           = 0,
    SAY_BONE            = 1,
    SAY_DEATH           = 2
};

enum Spells
{
    //Intro scene
    SPELL_FLOATING_DEAD            = 160467,
    SPELL_CLIENT_SCENE_TELEPORT    = 178093,
    //Nerzhul
    SPELL_MALEVOLENCE_SUM          = 154439,
    SPELL_MALEVOLENCE_DMG          = 154442,
    SPELL_OMEN_OF_DEATH            = 177691,
    //Omen
    SPELL_OMEN_OF_DEATH_VISUAL     = 154351,
    SPELL_OMEN_OF_DEATH_DMG        = 154352,
    //Bones
    SPELL_RITUAL_OF_BONES          = 154559,
    SPELL_RITUAL_OF_BONES_VISUAL   = 156312,
    SPELL_RITUAL_OF_BONES_DMG_FIN  = 160537,

    //Christmas
    SPELL_CHRISTMAS_CAP             = 176923
};

enum eEvents
{
    EVENT_MALEVOLENCE      = 1,
    EVENT_OMEN_OF_DEATH    = 2,
    EVENT_RITUAL_OF_BONES  = 3
};

Position const bonePos[6] =
{
    {1679.81f, -829.47f, 73.85f, 0.1f},
    {1679.28f, -815.14f, 73.85f, 0.1f},
    {1681.24f, -843.43f, 73.85f, 0.1f},
    {1679.23f, -802.71f, 73.85f, 0.1f},
    {1693.66f, -854.61f, 73.85f, 0.1f},
    {1689.09f, -790.68f, 73.85f, 0.1f}
};

struct boss_nerzhul : public BossAI
{
    boss_nerzhul(Creature* creature) : BossAI(creature, DATA_NERZHUL) {}

    uint8 bonesCount;

    void Reset() override
    {
        events.Reset();
        _Reset();

        summons.DespawnAll();

        if (sGameEventMgr->IsActiveEvent(2))
            DoCast(SPELL_CHRISTMAS_CAP);
        else
        {
            if (me->HasAura(SPELL_CHRISTMAS_CAP))
                me->RemoveAura(SPELL_CHRISTMAS_CAP);
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        _EnterCombat();

        events.RescheduleEvent(EVENT_MALEVOLENCE, 6000);
        events.RescheduleEvent(EVENT_OMEN_OF_DEATH, 10000);
        events.RescheduleEvent(EVENT_RITUAL_OF_BONES, 20000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();

        me->SummonCreature(NPC_PORTAL, 1727.25f, -810.65f, 73.80f, 3.56f);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
        summons.SetReactState(REACT_PASSIVE);

        switch (summon->GetEntry())
        {
        case NPC_OMEN_OF_DEATH:
            summon->CastSpell(summon, SPELL_OMEN_OF_DEATH_VISUAL, true);
            summon->CastSpell(summon, SPELL_OMEN_OF_DEATH_DMG, true);
            break;
        case NPC_RITUAL_OF_BONES:
            if (bonesCount < 4)
                summon->AI()->DoAction(1);
            else
                summon->AI()->DoAction(2);

            bonesCount++;
            break;
        }
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
            case EVENT_MALEVOLENCE:
                DoCast(me, SPELL_MALEVOLENCE_SUM, true);
                DoCast(SPELL_MALEVOLENCE_DMG);
                events.RescheduleEvent(EVENT_MALEVOLENCE, 10000);
                break;
            case EVENT_OMEN_OF_DEATH:
                DoCast(SPELL_OMEN_OF_DEATH);
                events.RescheduleEvent(EVENT_OMEN_OF_DEATH, 12000);
                break;
            case EVENT_RITUAL_OF_BONES:
                bonesCount = 0;
                Talk(SAY_BONE);

                for (uint8 i = 0; i < 6; ++i)
                    me->SummonCreature(NPC_RITUAL_OF_BONES, bonePos[i]);

                events.RescheduleEvent(EVENT_RITUAL_OF_BONES, 50000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//79497
struct npc_shadowmoon_nerzhul_intro : public ScriptedAI
{
    npc_shadowmoon_nerzhul_intro(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void Reset() override {}

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->IsPlayer())
        {
            if (me->IsWithinDistInMap(who, 70.0f))
            {
                who->CastSpell(who, SPELL_FLOATING_DEAD, true);
                who->CastSpell(who, SPELL_CLIENT_SCENE_TELEPORT, true);
            }
        }
    }
};

//76518
struct npc_nerzhul_ritual_bones : public ScriptedAI
{
    npc_nerzhul_ritual_bones(Creature* creature) : ScriptedAI(creature)
    {
        me->SetWalk(true);
    }

    EventMap events;

    void Reset() override {}

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCast(SPELL_RITUAL_OF_BONES_VISUAL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAllAuras();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 1)
        {
            DoCast(SPELL_RITUAL_OF_BONES_DMG_FIN);
            events.RescheduleEvent(EVENT_2, 5000);
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == 1)
            events.RescheduleEvent(EVENT_1, 1000);
        if (action == 2)
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
                DoCast(SPELL_RITUAL_OF_BONES);
                float x, y, z;
                me->GetClosePoint(x, y, z, me->GetObjectSize(), 68.0f);
                me->GetMotionMaster()->MovePoint(1, x, y, z);
                break;
            case EVENT_2:
                me->RemoveAreaObject(154462);
                me->DespawnOrUnsummon();
                break;
            }
        }
    }
};

//154353
class spell_omen_of_death: public SpellScript
{
    PrepareSpellScript(spell_omen_of_death);

    void DealDamage()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        if (!caster || !target)
            return;

        float distance = caster->GetExactDist2d(target);

        if (distance >= 0 && distance <= 60)
            SetHitDamage(GetHitDamage() * ((MAX_DIST - distance) / 100));
    }

    void Register()
    {
        OnHit += SpellHitFn(spell_omen_of_death::DealDamage);
    }
};

void AddSC_boss_nerzhul()
{
    RegisterCreatureAI(boss_nerzhul);
    RegisterCreatureAI(npc_shadowmoon_nerzhul_intro);
    RegisterCreatureAI(npc_nerzhul_ritual_bones);
    RegisterSpellScript(spell_omen_of_death);
}
