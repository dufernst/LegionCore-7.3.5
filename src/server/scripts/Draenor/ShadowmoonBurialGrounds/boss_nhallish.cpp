/*
    Dungeon : Shadowmoon Burial Grounds 100
    Encounter: Nhallish
*/

#include "shadowmoon_burial_grounds.h"

enum Says
{
    SAY_AGGRO            = 0,
    SAY_DEATH            = 1,
    SAY_INTRO_1          = 2,
    SAY_INTRO_2          = 3,
    SAY_INTRO_3          = 4,
    SAY_KILLED_PLAYER    = 5,
    SAY_VOID_VORTEX      = 6,
    SAY_VOID_DEVASTATION = 7,
    SAY_SOUL_STEAL       = 8
};

enum Spells
{
    //intro
    SPELL_SHACKLED_SOUL             = 154025, //summon npc 76269
    SPELL_VOID_CHAINS               = 153624, //hit npc 76269

    SPELL_VOID_BLAST                = 152792,
    SPELL_PLANAR_SHIFT              = 153623,
    SPELL_VOID_VORTEX               = 152801, //поправить притягивалку ат и настройки в дате
    SPELL_SOUL_STEAL                = 152962,
    SPELL_TEMPORAL_DISTORTION       = 158382,
    SPELL_TEMPORAL_DISTORTION_STUN  = 158372,
    SPELL_SOUL_STEAL_CHANNEL        = 156755,
    SPELL_TEMPORAL_DISPELL          = 158379,
    SPELL_VOID_DEVASTATION          = 153067,
    SPELL_TEMPORAL_DISTORTION_CHECK = 158432,

    //Possessed Soul
    SPELL_UNORTHODOX_EXISTENCE      = 152976,
    SPELL_SOUL_SHRED                = 152979,
    SPELL_RETURNED_SOUL             = 153033,
    SPELL_MIRROR_IMAGE              = 153493,
    SPELL_RECLAIMING_SOUL_KILL      = 153486,
    SPELL_RECLAIMING_SOUL           = 154921, //???
    SPELL_RECLAIMING_SOUL_REMOVE    = 154925, //spellclick
    SPELL_SOULLESS_SCREAN           = 154947,
    SPELL_FEIGN_DEATH               = 114371,

    SPELL_PHASE_SHIFT_1             = 155005,
    SPELL_PHASE_SHIFT_2             = 155006,
    SPELL_PHASE_SHIFT_3             = 155007,
    SPELL_PHASE_SHIFT_4             = 155009,
    SPELL_PHASE_SHIFT_5             = 155010,
    
    SPELL_DEFIILED_BURIAL_SITE      = 153238
};

enum eEvents
{
    EVENT_VOID_BLAST        = 1,
    EVENT_PLANAR_SHIFT      = 2,
    EVENT_SOUL_STEAL        = 3,
    EVENT_VOID_DEVASTATION  = 4
};

uint32 phaseSpell[5] =
{
    SPELL_PHASE_SHIFT_1,
    SPELL_PHASE_SHIFT_2,
    SPELL_PHASE_SHIFT_3,
    SPELL_PHASE_SHIFT_4,
    SPELL_PHASE_SHIFT_5
};

struct boss_nhallish : public BossAI
{
    explicit boss_nhallish(Creature* creature) : BossAI(creature, DATA_NHALLISH) {}

    uint16 checkTimer;
    bool soulsEvent;

    void Reset() override
    {
        events.Reset();
        _Reset();
        me->SetReactState(REACT_AGGRESSIVE);

        soulsEvent = false;
        checkTimer = 1000;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        _EnterCombat();

        events.RescheduleEvent(EVENT_VOID_BLAST, 12000);
        events.RescheduleEvent(EVENT_PLANAR_SHIFT, 24000);
        events.RescheduleEvent(EVENT_SOUL_STEAL, 34000);
        events.RescheduleEvent(EVENT_VOID_DEVASTATION, 66000);
    }

    void EnterEvadeMode() override
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(153501);
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(153501);
        Talk(SAY_DEATH);
        _JustDied();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim->IsPlayer())
            return;

        uint8 rand = urand(0, 1);

        if (rand)
            Talk(SAY_KILLED_PLAYER);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (target->IsPlayer())
        {
            switch (spell->Id)
            {
            case SPELL_PLANAR_SHIFT:
                events.RescheduleEvent(EVENT_5, 500);
                break;
            case SPELL_SOUL_STEAL:
                Talk(SAY_SOUL_STEAL);
                DoCast(me, SPELL_TEMPORAL_DISTORTION, true);
                DoCast(me, SPELL_TEMPORAL_DISTORTION_STUN, true);
                soulsEvent = true;
                break;
            }
        }
        else
        {
            if (spell->Id == SPELL_TEMPORAL_DISPELL)
                target->RemoveAurasDueToSpell(SPELL_TEMPORAL_DISTORTION_STUN);
        }
    }

    void CheckPlrSoul()
    {
        const Map::PlayerList &PlayerList = me->GetMap()->GetPlayers();
        if (!PlayerList.isEmpty())
        {
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* plr = i->getSource())
                {
                    if (!plr->isAlive() || plr->isGameMaster())
                        return;

                    if (!plr->HasAura(SPELL_SOULLESS_SCREAN))
                    {
                        soulsEvent = false;
                        DoCast(me, SPELL_TEMPORAL_DISPELL, true);
                        me->RemoveAurasDueToSpell(SPELL_TEMPORAL_DISTORTION);
                        me->SetReactState(REACT_AGGRESSIVE);
                    }
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (soulsEvent)
        {
            if (checkTimer <= diff)
            {
                CheckPlrSoul();

                if (me->GetDistance(me->GetHomePosition()) >= 30.0f)
                {
                    EnterEvadeMode();
                    return;
                }
                checkTimer = 1000;
            }
            else
                checkTimer -= diff;
        }

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_VOID_BLAST:
                DoCast(SPELL_VOID_BLAST);
                events.RescheduleEvent(EVENT_VOID_BLAST, 24000);
                break;
            case EVENT_PLANAR_SHIFT:
                DoCast(SPELL_PLANAR_SHIFT);
                events.RescheduleEvent(EVENT_PLANAR_SHIFT, 70000);
                break;
            case EVENT_SOUL_STEAL:
                me->StopAttack();
                DoCast(SPELL_SOUL_STEAL);
                events.RescheduleEvent(EVENT_SOUL_STEAL, 70000);
                break;
            case EVENT_VOID_DEVASTATION:
                Talk(SAY_VOID_DEVASTATION);
                DoCast(SPELL_VOID_DEVASTATION);
                events.RescheduleEvent(EVENT_VOID_DEVASTATION, 70000);
                break;
            case EVENT_5:
                Talk(SAY_VOID_VORTEX);
                DoCast(SPELL_VOID_VORTEX);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//75899
struct npc_nhallish_possessed_soul : public ScriptedAI
{
    explicit npc_nhallish_possessed_soul(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override {}

    void IsSummonedBy(Unit* summoner) override
    {
        DoZoneInCombat(me, 50.0f);
        DoCast(me, SPELL_UNORTHODOX_EXISTENCE, true);
        DoCast(summoner, SPELL_MIRROR_IMAGE, true);
        DoCast(summoner, SPELL_SOULLESS_SCREAN, true);
        DoCast(summoner, SPELL_SOUL_SHRED);
        me->GetMotionMaster()->MoveRandom(10.0f);
    }

    void DamageTaken(Unit* who, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (damage >= me->GetHealth())
        {
            if (who->GetEntry() != NPC_POSSESSED_SOUL)
                damage = 0;

            me->setFaction(35);
            me->RemoveAurasDueToSpell(111232); //mirror image
            DoCast(me, SPELL_FEIGN_DEATH, true);
        }
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_RECLAIMING_SOUL_REMOVE)
        {
            DoCast(target, SPELL_RETURNED_SOUL, true);
            DoCast(me, SPELL_RECLAIMING_SOUL_KILL, true);
        }
    }
};

//75977
struct npc_nhallish_defiled_burial_site : public ScriptedAI
{
    explicit npc_nhallish_defiled_burial_site(Creature* creature) : ScriptedAI(creature)
    {
        activate = false;
    }

    bool activate;

    void Reset() override {}

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->IsPlayer())
        {
            if (me->IsWithinDistInMap(who, 2.0f) && !activate)
            {
                activate = true;
                DoCast(SPELL_DEFIILED_BURIAL_SITE);
                me->DespawnOrUnsummon(5000);
            }
        }
    }
};

//153493
class spell_nhallish_mirror_image : public SpellScript
{
    PrepareSpellScript(spell_nhallish_mirror_image);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (!GetCaster() || !GetHitUnit())
            return;

        GetHitUnit()->CastSpell(GetCaster(), GetSpellInfo()->Effects[EFFECT_0]->BasePoints, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_nhallish_mirror_image::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

//152962
class spell_nhallish_soul_summon : public SpellScript
{
    PrepareSpellScript(spell_nhallish_soul_summon);

    uint8 soulCount;

    bool Load()
    {
        soulCount = 0;
        return true;
    }

    void HandleScript()
    {
        Player* caster = GetHitUnit()->ToPlayer();
        if (!caster)
            return;

        caster->CastSpell(caster, phaseSpell[soulCount], true);
        soulCount++;
    }

    void Register() override
    {
        BeforeHit += SpellHitFn(spell_nhallish_soul_summon::HandleScript);
    }
};

void AddSC_boss_nhallish()
{
    RegisterCreatureAI(boss_nhallish);
    RegisterCreatureAI(npc_nhallish_possessed_soul);
    RegisterCreatureAI(npc_nhallish_defiled_burial_site);
    RegisterSpellScript(spell_nhallish_mirror_image);
    RegisterSpellScript(spell_nhallish_soul_summon);
}
