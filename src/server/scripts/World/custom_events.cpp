#include "AreaTriggerAI.h"
#include "Chat.h"
#include "CombatAI.h"
#include "custom_events.h"
#include "GameEventMgr.h"
#include "OutdoorPvP.h"
#include "QuestData.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Vehicle.h"


/*
    Summon Point: Abandoned Temple
*/



enum bossTempleVonjin
{
    SPELL_WEAPON_THROW          = 305108,
    SPELL_WEAPON_THROW_STUN     = 305109,
    SPELL_BLEED                 = 305110,
    SPELL_BLADESTORM            = 305111,
    SPELL_BLADESTORM_DMG        = 305112,
    SPELL_DROP_WEAPON           = 305113,
    SPELL_DROP_WEAPON_DMG       = 305115,
    SPELL_CLEAVE                = 305116,
    SPELL_EMPOWER               = 305117, 
    SPELL_SCREAM                = 305118,
};

// 150010
struct boss_temple_vonjin : public ScriptedAI
{
    boss_temple_vonjin(Creature* creature) : ScriptedAI(creature), summons(me) 
    {
        Skull();
    }

    EventMap events;
    SummonList summons;
    uint16 stack = 0;
    uint16 timer = 0;

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(0);
        events.ScheduleEvent(EVENT_1, 5000);
        events.ScheduleEvent(EVENT_3, urand(11000, 12000));
        events.ScheduleEvent(EVENT_6, 8000);
    }

    void KilledUnit(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        Talk(1);
        me->RemoveAurasDueToSpell(SPELL_EMPOWER);
    }

    void Reset() override
    {
        events.Reset();
        summons.DespawnAll();
        stack = 0;
        timer = 0;
        me->RemoveAurasDueToSpell(SPELL_EMPOWER);
        me->SetControlled(0, UNIT_STATE_ROOT);
        
        Skull();
    }

    void Skull()
    {
        if (auto skull = me->FindNearestGameObject(go_hazorn_skull_1, 50.f))
            if (me->isAlive())
                skull->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
            else
                skull->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void JustDied(Unit* who) override
    {
        Talk(2);
    }

    void SpellFinishCast(SpellInfo const* spellInfo) override
    {
        switch (spellInfo->Id)
        {
        case SPELL_SCREAM:
            if (auto aura = me->GetAura(SPELL_EMPOWER))
                if (auto count = aura->GetStackAmount())
                    aura->SetStackAmount(count - 15);
            break;
        }  
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            ++stack;
            DoCast(me, SPELL_EMPOWER, true);
            if (!timer && stack >= 15)
                timer = 3000;
            break;
        case ACTION_2:
            DoCast(SPELL_SCREAM);
            stack = stack - 15;
            timer = 5000;
            break;
        }
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_BLADESTORM_DMG)
            DoAction(ACTION_1);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (CheckHomeDistToEvade(diff, 60.f, -664.49f, -42.28f, -90.83f))
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_BLADESTORM))
            return;

        if (timer && stack >= 15)
        {
            if (timer <= diff)
            {
                DoAction(ACTION_2);
            }
            else
                timer -= diff;
        }

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto target = me->getVictim())
                    DoCast(target, SPELL_BLEED);

                events.ScheduleEvent(EVENT_2, 25000);
                break;
            case EVENT_2:
                DoCast(SPELL_CLEAVE);
                events.ScheduleEvent(EVENT_4, 10000);
                break;
            case EVENT_3:
                if (!me->HasAura(SPELL_BLADESTORM))
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.f, true))
                        me->CastSpell(target, SPELL_WEAPON_THROW, true);

                events.ScheduleEvent(EVENT_3, urand(11000, 12000));
                break;
            case EVENT_4:
                me->SetControlled(1, UNIT_STATE_ROOT);
                DoCast(SPELL_BLADESTORM);
                events.ScheduleEvent(EVENT_5, 22000);
                Talk(3);
                break;
            case EVENT_5:
                DoCast(SPELL_DROP_WEAPON);
                events.ScheduleEvent(EVENT_6, 10000);
                break;
            case EVENT_6:
                events.ScheduleEvent(EVENT_2, 5000);
                break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

// 150012
struct npc_temple_vonjin_weapon : public ScriptedAI
{
    npc_temple_vonjin_weapon(Creature* creature) : ScriptedAI(creature)
    {
        //me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        DoCast(me, 305114);
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_DROP_WEAPON_DMG)
        {
            if (auto owner = me->GetAnyOwner())
                if (owner->IsAIEnabled)
                    if (auto v = owner->ToCreature())
                        v->AI()->DoAction(ACTION_1);
        }
    }
};


enum bossTempleLessar
{
    SPELL_MELEE_STOMP       = 305119,
    SPELL_POISON_SPIT       = 305120,
    SPELL_POISON_ARROWS     = 305121,
    SPELL_ENVENOM           = 305122,

};

// 150011
struct boss_temple_lessar : public ScriptedAI
{
    boss_temple_lessar(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        Blood();
    }

    EventMap events;
    SummonList summons;

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(0);
        events.ScheduleEvent(EVENT_1, 5000);
        events.ScheduleEvent(EVENT_2, 12000);
        events.ScheduleEvent(EVENT_3, 18000);
        events.ScheduleEvent(EVENT_4, 25000);
        events.ScheduleEvent(EVENT_5, 15000);

    }

    void Blood()
    {
        if (auto blood = me->FindNearestGameObject(go_ritual_blood_1, 50.f))
            if (me->isAlive())
                blood->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
            else
                blood->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
    }

    void KilledUnit(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        Talk(1);
    }

    void Reset() override
    {
        events.Reset();
        summons.DespawnAll();
        Blood();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void JustDied(Unit* who) override
    {
        Talk(2);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (CheckHomeDistToEvade(diff, 60.f, -443.88f, -85.82f, -90.82f))
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;


        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto target = me->getVictim())
                    DoCast(target, SPELL_ENVENOM);
                events.ScheduleEvent(EVENT_1, 10000);
                break;
            case EVENT_2:
                DoCast(SPELL_POISON_ARROWS);
                events.ScheduleEvent(EVENT_2, 12000);
                break;
            case EVENT_3:
                DoCast(SPELL_MELEE_STOMP);
                events.ScheduleEvent(EVENT_3, 25000);
                Talk(3);
                break;
            case EVENT_4:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.f, true))
                    DoCast(target, 305123);

                events.ScheduleEvent(EVENT_4, 30000);
                break;
            case EVENT_5:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.f, true))
                    DoCast(target, SPELL_POISON_SPIT);

                events.ScheduleEvent(EVENT_5, urand(8000,13000));
                break;
            }
        }

        DoMeleeAttackIfReady();
    }
};


enum bossTempleHexLord
{
    //boss
    SPELL_POISON_CLOUD          = 305123, //
    SPELL_POISON_CLOUD_TS       = 305124, //
    SPELL_POISON_CLOUD_DUM      = 305125, 
    SPELL_TEMPLE_FURY           = 305126, //
    SPELL_TEMPLE_FURY_DMG       = 305127,
    SPELL_TEMPLE_FURY_TS        = 305145, //
    SPELL_BLOWING_DEBUFF        = 305128,
    SPELL_BLOWING_DEBUFF_DMG    = 305129,
    SPELL_CHAINS_TS             = 305130, //
    SPELL_CHAINS_STUN           = 305131,
    SPELL_EXPLODE_HL            = 305132, //
    SPELL_EMPOWER_SPHERE_VIS    = 305133,
    SPELL_STACKS_HL             = 305142, //
    SPELL_TRANSFER              = 305134, //
    SPELL_TRANSFER_TS           = 305135,
    SPELL_TRANSFER_TS_OUT       = 305149,
    SPELL_POISON_TOTEM          = 305136, //
    SPELL_CRY_OF_THE_DAMNED     = 305146, //
    SPELL_EXPOSE_ARMOR          = 305147, //
    SPELL_SPIRITS_WORLD         = 305148, //

    SPELL_POISON_ARROWS_HL      = 305138,
    SPELL_POISON_VISUAL         = 146201,

    //adds
    SPELL_HEAL_1                = 305139,
    SPELL_RENEW_1               = 305140,
    SPELL_RENEW_PEREODIC        = 305141,

    SPELL_SCREAM_W              = 228278,
    SPELL_AOE_INTERRUPT         = 160845,
    SPELL_THUNDERSTORM          = 183543,

    SPELL_KNOCKBACK_1           = 177145,
    SPELL_HEX                   = 51514,
    SPELL_BLOODLUST             = 23951,

    //misc
    SPELL_TRANSFER_CENTER_VIS   = 305143,
    SPELL_FTB                   = 89092,
    SPELL_ENERG                 = 37290,
    SPELL_CLEANER               = 305150,
    SPELL_SKULLT                = 305151,
    SPELL_BLOODT                = 305152,
    SPELL_RITUAL_START          = 305107,
    SPELL_RITUAL_VIS_1          = 177893,
    SPELL_RITUAL_VIS_2          = 181186,

    SPELL_BOSS_SPAWN_VIS_1      = 187836,

};

// 150004
struct boss_hex_lord_hadorn : public ScriptedAI
{
    boss_hex_lord_hadorn(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        //me->SetVisible(false); activate after finish scripting
        RitualReset();   
    }

    EventMap events;
    SummonList summons;

    uint8 phase = 0;
    uint8 randomRings;
    uint8 ringProcessed;
    uint8 totemsProcessed;
    uint32 powertimer = 1000;
    uint32 checktimer = 1000;
    bool paused = false;
    bool reseted = false;
    bool ritual;

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(0);
        phase = 1;
        events.ScheduleEvent(EVENT_1, 10000);
        events.ScheduleEvent(EVENT_2, 16000);
        events.ScheduleEvent(EVENT_3, 30000);
        events.ScheduleEvent(EVENT_5, 25000);
        events.ScheduleEvent(EVENT_6, 20000);
        events.ScheduleEvent(EVENT_7, 60000);
        events.ScheduleEvent(EVENT_10, 80000);
        DoCast(SPELL_ENERG);
    }

    void RitualReset()
    {
        if (!ritual && me->isAlive())
        {
            me->SummonCreature(npc_circle_sum_eff, summonCircle);
            me->SummonCreature(npc_skull_place, summonSkull);
            me->SummonCreature(npc_blood_place, summonBlood);
            for (uint8 i = 0; i < 4; ++i)
                if (auto er = me->SummonCreature(npc_enter_ritual, ritualPoints[i]))
                    er->SetReactState(REACT_PASSIVE);
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void KilledUnit(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        Talk(1);
    }

    void Reset() override
    {
        phase = 0;
        paused = false;
        reseted = false;
        events.Reset();
        summons.DespawnAll();
        ringProcessed = 0;
        totemsProcessed = 0;
        me->SetCreateMana(100);
        me->SetMaxPower(POWER_MANA, 100);
        me->SetPower(POWER_MANA, 0);
        CleanUp();
    }

    void CleanUp()
    {
        me->RemoveAura(SPELL_STACKS_HL);

        std::list<Player*> plrList;
        me->GetPlayerListInGrid(plrList, 180.f);
        if (!plrList.empty())
        {
            for (Player* itr : plrList)
            {
                if (!itr)
                    continue;

                if (itr && itr->IsInWorld())
                {
                    itr->RemoveAura(SPELL_SPIRITS_WORLD);
                    itr->RemoveAura(SPELL_TRANSFER);
                    itr->RemoveAura(SPELL_CHAINS_STUN);
                    if (itr->GetPositionZ() <= -105.f)
                    {
                        itr->NearTeleportTo(addsPos[urand(0, 4)]);
                    }
                }
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (me->HealthBelowPct(50) && phase != 2)
        {
            phase = 2;
            events.ScheduleEvent(EVENT_3, 8000);
        }
        if (me->HealthBelowPct(40) && !reseted)
        {
            reseted = true;
            events.Reset();

            events.ScheduleEvent(EVENT_1, urand(15000, 20000));
            events.ScheduleEvent(EVENT_2, 15000);
            events.ScheduleEvent(EVENT_3, 15000);
            events.ScheduleEvent(EVENT_5, 25000);
            events.ScheduleEvent(EVENT_6, 20000);
            events.ScheduleEvent(EVENT_7, 30000);
            events.ScheduleEvent(EVENT_10, 40000);
        }
    }

    void PoisonRing(uint32 ring1, uint32 ring2)
    {
        auto npc1 = ring1;
        auto npc2 = ring2;

        std::list<Creature*> creaList;
        GetCreatureListWithEntryInGrid(creaList, me, npc1, 110.0f);
        GetCreatureListWithEntryInGrid(creaList, me, npc2, 110.0f);
        for (auto const& rings : creaList)
            if (rings->IsAIEnabled)
                rings->AI()->DoAction(ACTION_1);
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
        //case 6343:
         //   ForDebugOnly();
         //   break;
        case SPELL_RENEW_PEREODIC:
        case SPELL_HEAL_1:
            if (auto aura = me->GetAura(SPELL_STACKS_HL))
            {
                if (auto aCount = aura->GetStackAmount())
                    me->AddAura(SPELL_STACKS_HL, me, nullptr, aCount + 1);
            }
            else if (!me->HasAura(SPELL_STACKS_HL))
                me->AddAura(SPELL_STACKS_HL, me);

            break;
        }
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
        case SPELL_CHAINS_TS:
            target->CastSpell(target, SPELL_CHAINS_STUN, true);
            break;
        case SPELL_TRANSFER_TS:
            target->CastSpell(target, SPELL_TRANSFER, true);
            break;
        case SPELL_TRANSFER_TS_OUT:
            if (target->GetPositionZ() <= -105.f)
            {
                target->CastSpell(target, SPELL_FTB, true);
                target->AddDelayedEvent(1500, [target]() -> void
                {
                    if (target)
                    {
                        target->RemoveAura(SPELL_SPIRITS_WORLD);
                        target->NearTeleportTo(addsPos[urand(0, 4)]);
                    }  
                });
            }
            break;
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case npc_zandalari_priest:
        case npc_venom_slayer:
        case npc_zandalari_bers:
            if (auto aura = me->GetAura(SPELL_STACKS_HL))
            {
                if (auto aCount = aura->GetStackAmount())
                    me->AddAura(SPELL_STACKS_HL, me, nullptr, aCount - 1);
            }
            break;
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            ringProcessed = 0;
            if (phase == 1)
                events.CancelEvent(EVENT_3);
            events.ScheduleEvent(EVENT_4, 10000);
            break;
        case ACTION_2:
            Adds(true);
            break;
        case ACTION_3:
            me->SetVisible(true);
            DoCast(me, SPELL_BOSS_SPAWN_VIS_1, true);
            ritual = true;
            Talk(6);
            break;
        }
    }

    void Spheres()
    {
        for (uint8 i = 0; i < max_spheres; ++i)
            if (auto sphere = me->SummonCreature(npc_empowering_sph, spherePos[i]))
            {
                sphere->SetReactState(REACT_PASSIVE);
                me->SetPower(POWER_MANA, 0);
            }

        DoCast(me, SPELL_ENERG, true);
    }

    bool StacksCount()
    {
        if (auto aura = me->GetAura(SPELL_STACKS_HL))
            if (aura->GetStackAmount() >= 10)
                return true;

        return false;
    }

    void ForDebugOnly()
    {
        me->SummonCreature(npc_zandalari_priest, addsPos[0]);
        me->SummonCreature(npc_zandalari_priest, addsPos[3]);
        me->SummonCreature(npc_venom_slayer, addsPos[2]);
        me->SummonCreature(npc_zandalari_bers, addsPos[1]);
        me->SummonCreature(npc_zandalari_bers, addsPos[4]);
    }

    void Adds(bool withoutevents)
    {
        if (!withoutevents)
        {
            events.CancelEvent(EVENT_5);
            if (phase == 1 || phase == 3)
                events.ScheduleEvent(EVENT_9, 25000);
            if (phase == 2 || phase == 3)
                events.ScheduleEvent(EVENT_9, 20000);
        }
        me->SummonCreature(npc_zandalari_priest, addsPos[0]);
        me->SummonCreature(npc_zandalari_priest, addsPos[3]);
        me->SummonCreature(npc_venom_slayer, addsPos[2]);
        me->SummonCreature(npc_zandalari_bers, addsPos[1]);
        me->SummonCreature(npc_zandalari_bers, addsPos[4]);
    }

    void Spirits()
    {
        std::list<Creature*> creaList;
        GetCreatureListWithEntryInGrid(creaList, me, npc_spirit_4kill, 160.0f);
        if (!creaList.empty())
        {
            if (auto count = creaList.size())
            {
                if (auto aura = me->GetAura(SPELL_STACKS_HL))
                {
                    if (auto aCount = aura->GetStackAmount())
                        me->AddAura(SPELL_STACKS_HL, me, nullptr, aCount + count);
                }
                else
                    me->AddAura(SPELL_STACKS_HL, me, nullptr, count);
            }
            for (auto spirits : creaList)
                spirits->DespawnOrUnsummon(100);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (powertimer <= diff)
        {
            powertimer = 1000;
            if (auto pow = me->GetPower(POWER_MANA))
            {
                me->SetPower(POWER_MANA, pow + 1);
                if (pow >= 100)
                    Spheres();
            }
        }
        else
            powertimer -= diff;

        if (checktimer <= diff)
        {
            if (StacksCount())
            {
                DoCast(SPELL_CRY_OF_THE_DAMNED);
                checktimer = 6000;
            }
            checktimer = 1000;
        }
        else
            checktimer -= diff;

        events.Update(diff);

        //if (CheckHomeDistToEvade(diff, 62.0f, -478.07f, 95.33f, -94.66f))
            //return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(me, SPELL_EXPLODE_HL, true);
                events.ScheduleEvent(EVENT_1, urand(15000,20000));
                break;
            case EVENT_2:
                if (auto tfury = me->FindNearestCreature(npc_temple_fury_t, 100.f, true))
                    if (tfury->IsAIEnabled)
                        tfury->AI()->DoAction(ACTION_1);

                events.ScheduleEvent(EVENT_2, reseted ? urand(15000, 20000) : 13000);
                break;
            case EVENT_3:
                switch (phase)
                {
                case 1:
                    if (!paused)
                    {
                        randomRings = urand(0, 3);
                        if (randomRings == 0)
                            PoisonRing(npc_poison_ring_a, npc_poison_ring_c);

                        if (randomRings == 1)
                            PoisonRing(npc_poison_ring_b, npc_poison_ring_c);

                        if (randomRings == 2)
                            PoisonRing(npc_poison_ring_a, npc_poison_ring_b);

                        if (randomRings == 3)
                            PoisonRing(npc_poison_ring_a, npc_poison_ring_c);

                        events.ScheduleEvent(EVENT_3, 35000);
                        ++ringProcessed;
                        if (ringProcessed == 2)
                        {
                            ringProcessed = 0;
                            events.CancelEvent(EVENT_3);
                            events.ScheduleEvent(EVENT_4, 10000);
                        }
                    }
                    break;
                case 2:
                case 3:
                    DoCast(me, SPELL_POISON_CLOUD_TS, true);
                    events.ScheduleEvent(EVENT_3, 3000);
                    ++ringProcessed;
                    if (ringProcessed == 4)
                    {
                        ringProcessed = 0;
                        events.CancelEvent(EVENT_3);
                        events.ScheduleEvent(EVENT_4, 10000);
                    }
                    break;
                }
                break;
            case EVENT_4:
                DoCast(me, SPELL_CHAINS_TS, true);
                if (phase == 1)
                    events.ScheduleEvent(EVENT_3, 8000);
                if (phase == 2 || phase == 3)
                    events.ScheduleEvent(EVENT_3, 3500);
                break;
            case EVENT_5:
                if (!paused)
                {
                    me->SummonCreature(npc_poison_totem, me->GetPositionX() + frand(3, 5), me->GetPositionY() + frand(2, 4), me->GetPositionZ() + 1.5f, me->GetOrientation());
                    events.ScheduleEvent(EVENT_5, 25000);
                    ++totemsProcessed;
                    if (phase == 1 && totemsProcessed == 4)
                    {
                        paused = true;
                        Adds(false);
                    }
                    if ((phase == 2 || phase == 3) && totemsProcessed == 2)
                    {
                        paused = false;
                        Adds(false);
                    }
                }
                break;
            case EVENT_6:
                if (auto target = me->getVictim())
                    DoCast(target, SPELL_EXPOSE_ARMOR, true);

                events.ScheduleEvent(EVENT_6, 25000);
                break;
            case EVENT_7:
                if (auto tfury_b = me->FindNearestCreature(npc_temple_fury_b, 120.f, true))
                    tfury_b->CastSpell(tfury_b, SPELL_TRANSFER_CENTER_VIS);

                DoCast(me, SPELL_TRANSFER_TS, true);
                for (uint8 i = 0; i < spirits_max; ++i)
                    if (auto spirit = me->SummonCreature(npc_spirit_4kill, spiritPos[i]))
                        spirit->SetReactState(REACT_PASSIVE);

                events.ScheduleEvent(EVENT_8, 42000);
                events.ScheduleEvent(EVENT_7, reseted ? 110000 : 95000);
                break;
            case EVENT_8:
                Spirits();
                DoCast(me, SPELL_TRANSFER_TS_OUT, true);
                break;
            case EVENT_9:
                paused = false;
                totemsProcessed = 0;
                events.ScheduleEvent(EVENT_3, 1000);
                events.ScheduleEvent(EVENT_5, 8000);
                break;
            case EVENT_10:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true, -SPELL_SPIRITS_WORLD))
                    DoCast(target, SPELL_BLOWING_DEBUFF, true);

                events.ScheduleEvent(EVENT_10, 35000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};


// 150017 150018 150019
struct npc_hexlord_hadorn_poison_ring : public ScriptedAI
{
    npc_hexlord_hadorn_poison_ring(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;
    uint8 phase = 0;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            DoCast(me, SPELL_POISON_VISUAL, true);
            if (phase != 2)
                phase = 1;

            if (phase == 1)
                events.ScheduleEvent(EVENT_1, 4500);
            if (phase == 2)
                events.ScheduleEvent(EVENT_1, 2000);
                
            break;
        }
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
        case SPELL_POISON_CLOUD_TS:
            phase = 2;
            DoAction(ACTION_1);
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(SPELL_POISON_CLOUD);
                events.ScheduleEvent(EVENT_2, 15000);
                break;
            case EVENT_2:
                me->RemoveAura(SPELL_POISON_VISUAL);
                break;
            }
        }
    }
};

// 150020 top 150026 bottom
struct npc_hexlord_hadorn_fury_of_temple : public ScriptedAI
{
    npc_hexlord_hadorn_fury_of_temple(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_TEMPLE_FURY_TS)
            DoCast(target, SPELL_TEMPLE_FURY, true);
    }
    void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
    {
        if (spellId == SPELL_TRANSFER_CENTER_VIS && !apply)
        {
            if (auto boss = me->FindNearestCreature(npc_hadorn, 100.f, true))
                if (boss->IsAIEnabled)
                    boss->AI()->DoAction(ACTION_2);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            DoCast(SPELL_TEMPLE_FURY_TS);
            break;
        }
    }
};

// 150021
struct npc_event_spirit_chains : public ScriptedAI
{
    npc_event_spirit_chains(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (auto summoner = me->GetAnyOwner())
            summoner->RemoveAura(SPELL_CHAINS_STUN);
    }


    void Reset() override {}

    void UpdateAI(uint32 diff) override
    {

    }
};


// 150003
struct npc_event_summon_points_portal_initiator : public ScriptedAI
{
    npc_event_summon_points_portal_initiator(Creature* creature) : ScriptedAI(creature)
    {

    }

    void EnterCombat(Unit* /*who*/) override
    {

    }

    void Reset() override {}

    void UpdateAI(uint32 diff) override
    {

    }
};

// 150025
struct npc_hexlord_hadorn_totem : public ScriptedAI
{
    npc_hexlord_hadorn_totem(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void IsSummonedBy(Unit* summoner) override
    {
        if (auto target = me->FindNearestPlayer(40.f, true))
            me->CombatStart(target, true);

        events.ScheduleEvent(EVENT_1, 5000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!me->isInCombat())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(SPELL_POISON_TOTEM);
                events.ScheduleEvent(EVENT_1, 5000);
                break;
            }
        }
    }
};

// 150022
struct npc_hexlord_hadorn_sphere : public ScriptedAI
{
    npc_hexlord_hadorn_sphere(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 timer = 500;
    EventMap events;

    void IsSummonedBy(Unit* summoner) override
    {
        DoCast(SPELL_EMPOWER_SPHERE_VIS);
        events.ScheduleEvent(EVENT_1, 500);
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (auto boss = me->FindNearestCreature(npc_hadorn, 1.f, true))
            {
                if (auto aura = boss->GetAura(SPELL_STACKS_HL))
                {
                    if (auto aCount = aura->GetStackAmount())
                    {
                        boss->AddAura(SPELL_STACKS_HL, boss, nullptr, aCount + 1);
                        me->DespawnOrUnsummon(100);
                    }
                }
                else if (!boss->HasAura(SPELL_STACKS_HL))
                {
                    boss->AddAura(SPELL_STACKS_HL, boss);
                    me->DespawnOrUnsummon(100);
                }
                
            }
            else if (me->SelectNearestPlayer(1.f))
                me->DespawnOrUnsummon(100);

            timer = 500;
        }
        else
            timer -= diff;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto owner = me->GetAnyOwner())
                    me->GetMotionMaster()->MovePoint(0, owner->GetPosition());

                events.ScheduleEvent(EVENT_1, 500);
                break;
            }
        }
    }
};

// 150014 priest 150016 slayer 150015 bers
struct npc_hexlord_hadorn_add : public ScriptedAI
{
    npc_hexlord_hadorn_add(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint8 add = 0;
    uint32 timer = 5000;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        switch (me->GetEntry())
        {
        case npc_zandalari_priest:
            events.ScheduleEvent(EVENT_1, urand(8000, 10000));
            events.ScheduleEvent(EVENT_2, urand(12000, 15000));
            break;
        case npc_venom_slayer:
            events.ScheduleEvent(EVENT_1, urand(8000, 12000));
            events.ScheduleEvent(EVENT_2, urand(20000, 23000));
            break;
        case npc_zandalari_bers:
            events.ScheduleEvent(EVENT_1, urand(8000, 10000));
            events.ScheduleEvent(EVENT_2, urand(12000, 15000));
            break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(100);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        if (me->GetPositionX() == addsPos[0].GetPositionX() && me->GetPositionY() == addsPos[0].GetPositionY())
            add = 1;
        if (me->GetPositionX() == addsPos[1].GetPositionX() && me->GetPositionY() == addsPos[1].GetPositionY())
            add = 2;
        if (me->GetPositionX() == addsPos[2].GetPositionX() && me->GetPositionY() == addsPos[2].GetPositionY())
            add = 3;
        if (me->GetPositionX() == addsPos[3].GetPositionX() && me->GetPositionY() == addsPos[3].GetPositionY())
            add = 4;
        if (me->GetPositionX() == addsPos[4].GetPositionX() && me->GetPositionY() == addsPos[4].GetPositionY())
            add = 5;

        float x, y, z;
        me->GetClosePoint(x, y, z, me->GetObjectSize() / 3, static_cast<float>(4.3f));
        me->GetMotionMaster()->MovePoint(1, x, y, z);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 1)
            {
                if (add == 1)
                    me->GetMotionMaster()->MoveJump(addsMove[1], 15.f, 15.f);
                if (add == 2)
                    me->GetMotionMaster()->MoveJump(addsMove[3], 15.f, 15.f);
                if (add == 3)
                    me->GetMotionMaster()->MoveJump(addsMove[5], 15.f, 15.f);
                if (add == 4)
                    me->GetMotionMaster()->MoveJump(addsMove[7], 15.f, 15.f);
                if (add == 5)
                    me->GetMotionMaster()->MoveJump(addsMove[9], 15.f, 15.f);

                me->AddDelayedEvent(5000, [this]() -> void {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->GetMotionMaster()->MoveRandom(8.f);
                });
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            me->SetHomePosition(me->GetPosition());
            if (auto target = me->SelectNearestPlayer(35.f))
                me->CombatStart(target, true);

            timer = 5000;
        }
        else
            timer -= diff;

        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                switch (me->GetEntry())
                {
                case npc_zandalari_priest:
                    DoCast(SPELL_HEAL_1);
                    events.ScheduleEvent(EVENT_1, urand(18000, 25000));
                    break;
                case npc_venom_slayer:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(target, SPELL_HEX);
                    events.ScheduleEvent(EVENT_1, urand(15000, 23000));
                    break;
                case npc_zandalari_bers:
                    DoCast(SPELL_SCREAM_W);
                    events.ScheduleEvent(EVENT_1, urand(20000, 23000));
                    break;
                }

                break;
            case EVENT_2:
                switch (me->GetEntry())
                {
                case npc_zandalari_priest:
                    if (auto target = me->FindNearestCreature(npc_hadorn, 100.f, true))
                        me->CastSpell(target, SPELL_RENEW_1);

                    events.ScheduleEvent(EVENT_2, urand(18000, 20000));
                    break;
                case npc_venom_slayer:
                    DoCast(SPELL_BLOODLUST);
                    events.ScheduleEvent(EVENT_2, urand(33000, 37000));
                    break;
                case npc_zandalari_bers:
                    DoCast(SPELL_AOE_INTERRUPT);
                    events.ScheduleEvent(EVENT_2, urand(25000, 28000));
                    break;
                }

                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 150008 sk 150009
class npc_events_temple_summon_ritual_f : public CreatureScript
{
public:
    npc_events_temple_summon_ritual_f() : CreatureScript("npc_events_temple_summon_ritual_f") 
    {
        summoned = false;
    }

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        if (!summoned)
        {
            if (player->HasAura(SPELL_SKULLT))
            {
                creature->SummonGameObject(go_hazorn_skull_2, creature->GetPosition(), 0.f, 0.f, 0.f, 0.f, 0.f, 0);
                std::list<Creature*> creaList;
                GetCreatureListWithEntryInGrid(creaList, creature, npc_enter_ritual, 25.0f);
                for (auto const& rit : creaList)
                    if (rit->IsAIEnabled)
                        rit->AI()->DoAction(ACTION_1);

                summoned = true;
                return true;
            } 
        }
        return false;
    }

private:
    bool summoned = false;
};

class npc_events_temple_summon_ritual_d : public CreatureScript
{
public:
    npc_events_temple_summon_ritual_d() : CreatureScript("npc_events_temple_summon_ritual_d") 
    {
        summoned = false;
    }

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        if (!summoned)
        {
            if (player->HasAura(SPELL_BLOODT))
            {
                creature->SummonGameObject(go_ritual_blood_2, creature->GetPosition(), 0.f, 0.f, 0.f, 0.f, 0.f, 0);
                std::list<Creature*> creaList;
                GetCreatureListWithEntryInGrid(creaList, creature, npc_enter_ritual, 25.0f);
                for (auto const& rit : creaList)
                    if (rit->IsAIEnabled)
                        rit->AI()->DoAction(ACTION_2);

                summoned = true;
                return true;
            }
        }
        return false;
    }

private:
    bool summoned = false;
};

// 542137
class npc_events_portal_master_portal : public CreatureScript
{
public:
    npc_events_portal_master_portal() : CreatureScript("npc_events_portal_master_portal") {}

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        if (player->HasAura(305181) || player->HasAura(305180) || player->HasAura(event_deserter) || !sGameEventMgr->IsActiveEvent(812))
            return false;
        
        uint8 p = urand(0, 12);
        player->RemoveAura(783);
        player->RemoveAura(125883);
        player->RemoveAurasByType(SPELL_AURA_MOUNTED);
        player->CastSpell(player, 305180, true);
        player->CastSpell(player, 305181, true);
        player->TeleportTo(mapStromgard, stromgardPlayersPos[p].m_positionX, stromgardPlayersPos[p].m_positionY, stromgardPlayersPos[p].m_positionZ, stromgardPlayersPos[p].m_orientation);
        return true;
    }
};

// 542139
class npc_events_portal_master_portal_exit : public CreatureScript
{
public:
    npc_events_portal_master_portal_exit() : CreatureScript("npc_events_portal_master_portal_exit") {}

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        player->RemoveAura(305180);
        player->TeleportTo(1220, DalaranPos.m_positionX, DalaranPos.m_positionY, DalaranPos.m_positionZ, DalaranPos.m_orientation);
        return true;
    }
};

// 542173 542174
class npc_events_warsong_vehicle_take : public CreatureScript
{
public:
    npc_events_warsong_vehicle_take() : CreatureScript("npc_events_warsong_vehicle_take") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)  override
    {
        if (player->IsOnVehicle() || player->IsMounted())
        {
            ChatHandler(player).PSendSysMessage("ERROR! You need dismount.");
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        player->PlayerTalkClass->ClearMenus();

        if (auto sum = player->SummonCreature(creature->GetEntry() == 542173 ? 542171 : 542172, player->GetPosition()))
            player->CastSpell(sum, 46598, true);

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

// 150006
struct npc_events_temple_summon_ritual_s : public ScriptedAI
{
    npc_events_temple_summon_ritual_s(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    bool skull = false;
    bool blood = false;

    void Reset() override
    {
        skull = false;
        blood = false;
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            skull = true;
            break;
        case ACTION_2:
            blood = true;
            break;
        }
        if (skull && blood)
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }
};

// 150005
struct npc_events_temple_summon_ritual_t : public ScriptedAI
{
    npc_events_temple_summon_ritual_t(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint8 count = 0;
    bool started = false;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            if (!started)
            {
                started = true;
                DoCast(SPELL_RITUAL_VIS_1);
                me->AddDelayedEvent(5000, [this] {
                    me->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    DoCast(SPELL_RITUAL_VIS_2);
                });
                me->AddDelayedEvent(15000, [this] {
                    me->RemoveAura(SPELL_RITUAL_VIS_2);
                    if (auto owner = me->GetAnyOwner())
                        if (owner->IsAIEnabled)
                            owner->ToCreature()->AI()->DoAction(ACTION_3);
                });
            }
            break;
        }
    }

    void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
    {
        if (spellId == SPELL_RITUAL_START)
        {
            if (apply)
            {
                ++count;
                if (count == 1) //change to 4 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    DoAction(ACTION_1);
            }
            else
            {
                count = count - 1;
            }
        }
    }

    void UpdateAI(uint32 diff) override {}
};

// 305134
class spell_events_transfer_temple_bottom : public AuraScript
{
    PrepareAuraScript(spell_events_transfer_temple_bottom);

    uint16 timer = 1000;

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
        {
            if (!target)
                return;

            target->SetCanFly(true);
            target->SetSpeed(MOVE_FLIGHT, 1.8f);
            target->GetMotionMaster()->MoveIdle();
            target->GetMotionMaster()->MovePath(3051340, false);
            target->CastSpell(target, SPELL_SPIRITS_WORLD, true);
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
        {
            if (!target)
                return;

            target->SetSpeed(MOVE_FLIGHT, 1.0f);
            target->SetCanFly(false);
        }
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (timer <= diff)
        {
            if (auto target = GetCaster())
                if (target->GetPositionZ() <= -174.f)
                    target->RemoveAurasDueToSpell(305134);
            
            timer = 1000;
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_events_transfer_temple_bottom::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_events_transfer_temple_bottom::OnRemove, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
        OnEffectUpdate += AuraEffectUpdateFn(spell_events_transfer_temple_bottom::OnUpdate, EFFECT_0, SPELL_AURA_TRANSFORM);
    }
};

// 305111
class spell_events_vonjin_bladestorm : public AuraScript
{
    PrepareAuraScript(spell_events_vonjin_bladestorm);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
        {
            if (!target)
                return;

            target->SetSpeed(MOVE_WALK, 0.3f);
            target->SetSpeed(MOVE_RUN, 0.3f);
            target->SetControlled(0, UNIT_STATE_ROOT);
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
        {
            if (!target)
                return;

            target->SetSpeed(MOVE_WALK, 1.6f);
            target->SetSpeed(MOVE_RUN, 1.6f);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_events_vonjin_bladestorm::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_events_vonjin_bladestorm::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 305131
class spell_events_spirit_chains : public AuraScript
{
    PrepareAuraScript(spell_events_spirit_chains);

    GuidList ChainsGuid;
    uint32 timer = 1000;
    uint64 health;

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
        {
            if (!target)
                return;

            if (auto chains = target->SummonCreature(npc_spirit_chains, target->GetPosition()))
                ChainsGuid.push_back(chains->GetGUID());
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
        {
            if (!target)
                return;

            if (ChainsGuid.empty())
                return;

            for (GuidList::const_iterator itr = ChainsGuid.begin(); itr != ChainsGuid.end(); ++itr)
                if (auto roots = ObjectAccessor::GetCreature(*target, *itr))
                    if (roots)
                        roots->DespawnOrUnsummon(100);
        }
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (timer <= diff)
        {
            if (auto target = GetCaster())
                if (health = target->GetHealth())
                    target->SetHealth(health - (health * 5 / 100));

            timer = 1000;
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_events_spirit_chains::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_events_spirit_chains::OnRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        OnEffectUpdate += AuraEffectUpdateFn(spell_events_spirit_chains::OnUpdate, EFFECT_0, SPELL_AURA_MOD_STUN);
    }
};

// 305130
class spell_events_temple_aoe_bottom_top_filter : public SpellScript
{
    PrepareSpellScript(spell_events_temple_aoe_bottom_top_filter);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([this](WorldObject* object) -> bool
        {
            if (object->GetPositionZ() >= -75.0f || object->GetPositionZ() <= -96.0f)
                return true;

            if (object == nullptr || !object->IsPlayer() || object->ToPlayer()->HasAura(SPELL_SPIRITS_WORLD))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_events_temple_aoe_bottom_top_filter::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 305177
class spell_events_aoe_trap_stromgard : public SpellScript
{
    PrepareSpellScript(spell_events_aoe_trap_stromgard);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (auto caster = GetCaster())
        {
            targets.remove_if([caster](WorldObject* object) -> bool
            {
                float delta_z = fabs(caster->GetPositionZ()) - fabs(object->GetPositionZ());
                if (delta_z > 0.8f || delta_z < -3.5f)
                    return true;

                return false;
            });
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_events_aoe_trap_stromgard::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
    }
};

// 305146
class spell_events_hadorn_cry_of_the_damned : public SpellScript
{
    PrepareSpellScript(spell_events_hadorn_cry_of_the_damned);

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        if (auto aura = caster->GetAura(SPELL_STACKS_HL))
        {
            if (auto aCount = aura->GetStackAmount())
            {
                aura->ModStackAmount(-10);
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_events_hadorn_cry_of_the_damned::HandleAfterCast);
    }
};

// 305224
class spell_events_warsong_remove_passenger : public SpellScript
{
    PrepareSpellScript(spell_events_warsong_remove_passenger);

    SpellCastResult CheckElevation()
    {
        auto caster = GetCaster();
        if (!caster)
            return SPELL_FAILED_BAD_TARGETS;

        if (caster->GetEntry() == 542171)
        {
            if (caster->FindNearestCreature(542176, 8.f))
                return SPELL_CAST_OK;
        }
        else
            if (caster->FindNearestCreature(542175, 8.f))
                return SPELL_CAST_OK;

        return SPELL_FAILED_BAD_TARGETS;
    }

    bool EventCheck()
    {
        for (uint32 events : {813, 814, 815, 816, 817, 818, 819, 820, 821})
            if (sGameEventMgr->IsActiveEvent(events))
                return true;

        return false;
    }

    void HandleAfterCast()
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        if (auto veh = caster->GetVehicleKit())
        {
            if (auto pass = veh->GetPassenger(1))
            {
                Position pos = caster->GetPosition();
                pass->ExitVehicle(&pos);
                if (auto cre = pass->ToCreature())
                    cre->DespawnOrUnsummon(1000);

                if (auto rider = veh->GetPassenger(0))
                {
                    if (auto plr = rider->ToPlayer())
                        plr->KilledMonsterCredit(542140);

                    if (EventCheck())
                        rider->CastSpell(rider, 305225, true);
                }
            }
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_events_warsong_remove_passenger::CheckElevation);
        AfterCast += SpellCastFn(spell_events_warsong_remove_passenger::HandleAfterCast);
    }
};

// 305223
class spell_events_warsong_take_passenger_check : public SpellScript
{
    PrepareSpellScript(spell_events_warsong_take_passenger_check);

    SpellCastResult CheckElevation()
    {
        auto caster = GetCaster();
        auto target = GetExplTargetUnit();
        if (!caster && !target)
            return SPELL_FAILED_BAD_TARGETS;

        if (auto veh = caster->GetVehicleKit())
            if (veh->GetPassenger(1))
                return SPELL_FAILED_NOT_READY;

        if (caster->GetEntry() == 542172)
        {
            if (target->GetEntry() != 542169)
                return SPELL_FAILED_BAD_TARGETS;
        }
        else
            if (target->GetEntry() != 542167)
                return SPELL_FAILED_BAD_TARGETS; 

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_events_warsong_take_passenger_check::CheckElevation);
    }
};

// 305229
class spell_events_scroll_of_summon : public SpellScript
{
    PrepareSpellScript(spell_events_scroll_of_summon);

    SpellCastResult CheckElevation()
    {
        auto caster = GetCaster();
        if (!caster)
            return SPELL_FAILED_BAD_TARGETS;

        if (!caster->GetMap()->IsContinent())
            return SPELL_FAILED_NOT_HERE;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_events_scroll_of_summon::CheckElevation);
    }
};

// 305128
class spell_events_hadorn_explode_debuff : public AuraScript
{
    PrepareAuraScript(spell_events_hadorn_explode_debuff);

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        auto target = GetTarget();
        if (!target)
            return;

        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        target->CastSpell(target, SPELL_BLOWING_DEBUFF_DMG, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_events_hadorn_explode_debuff::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 305249
class spell_new_year_atray_explode_frost_debuff : public AuraScript
{
    PrepareAuraScript(spell_new_year_atray_explode_frost_debuff);

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        auto target = GetTarget();
        if (!target)
            return;

        target->CastSpell(target, 305250, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_new_year_atray_explode_frost_debuff::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 305226
class spell_events_warsong_speedbonus : public AuraScript
{
    PrepareAuraScript(spell_events_warsong_speedbonus);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
            if (auto sp = target->GetSpeedRate(MOVE_FLIGHT))
                target->SetSpeed(MOVE_FLIGHT, sp * 2);
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto target = GetTarget())
            if (auto sp = target->GetSpeedRate(MOVE_FLIGHT))
                target->SetSpeed(MOVE_FLIGHT, sp / 2);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_events_warsong_speedbonus::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_events_warsong_speedbonus::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 305232
class spell_events_tabard_arcane_magic : public AuraScript
{
    PrepareAuraScript(spell_events_tabard_arcane_magic);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (auto caster = GetTarget())
            caster->AddAura(305233, caster);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_events_tabard_arcane_magic::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};


// 305239
class spell_new_year_atray_cold : public AuraScript
{
    PrepareAuraScript(spell_new_year_atray_cold);

    bool CombatCheck(Unit* owner)
    {
        HostileRefManager& refManager = owner->getHostileRefManager();
        HostileReference* ref = refManager.getFirst();

        if (!ref)
            return false;

        while (ref)
        {
            if (auto unit = ref->getSource()->getOwner())
            {
                if (auto cre = unit->ToCreature())
                    if (cre->GetEntry() == 542185)
                        return true;
            }
            ref = ref->next();
        }
        return false;
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (auto owner = GetTarget())
        {
            if (owner->isInCombat() && CombatCheck(owner))
            {
                if (!owner->HasAura(305240))
                    owner->CastSpell(owner, 305240, true);
            }
            else if (!CombatCheck(owner))
            {
                owner->RemoveAura(305240);
            }

            owner->CastSpell(owner, 216155, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_new_year_atray_cold::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 305240
class spell_new_year_atray_cold_stacking : public AuraScript
{
    PrepareAuraScript(spell_new_year_atray_cold_stacking);

    void OnTick(AuraEffect const* aurEff)
    {
        if (auto owner = GetTarget())
        {
            if (auto aura = aurEff->GetBase())
            {
                if (auto count = aura->GetStackAmount())
                {
                    if (owner->isMoving())
                    {
                        if (count > 1)
                            aura->ModStackAmount(-1);
                    }
                    else
                    {
                        if (!owner->HasAura(305254) && !owner->HasAura(27827))
                            owner->AddAura(305240, owner, nullptr, count + 1);
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_new_year_atray_cold_stacking::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 305252
class spell_new_year_atray_khadgar_ts : public SpellScript
{
    PrepareSpellScript(spell_new_year_atray_khadgar_ts);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (auto caster = GetCaster())
        {
            targets.remove_if([caster](WorldObject* object) -> bool
            {
                float z = object->GetPositionZ();
                if (z > 59.68f && z < 59.67f)
                    return true;

                return false;
            });
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_new_year_atray_khadgar_ts::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// 305278
class spell_new_year_evala_tomb_ts : public SpellScript
{
    PrepareSpellScript(spell_new_year_evala_tomb_ts);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (auto caster = GetCaster())
        {
            targets.remove_if([caster](WorldObject* object) -> bool
            {
                if (auto plr = object->ToPlayer())
                    if (plr->isInFlight())
                        return true;

                return false;
            });
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_new_year_evala_tomb_ts::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 305248
class spell_new_year_atray_fury : public AuraScript
{
    PrepareAuraScript(spell_new_year_atray_fury);

    void OnTick(AuraEffect const* aurEff)
    {
        if (auto owner = GetTarget())
            owner->CastSpell(owner, 305255, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_new_year_atray_fury::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//  305279
class spell_new_year_evala_auras_305279 : public AuraScript
{
    PrepareAuraScript(spell_new_year_evala_auras_305279);

    uint16 timer = 3000;

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (timer <= diff)
        {
            if (auto target = GetCaster())
            {
                if (target->isInCombat())
                    return;

                if (auto aura = GetAura())
                {
                    if (auto b = target->FindNearestCreature(npc_evala, 80.f))
                    {
                        if (!b->isInCombat())
                            target->RemoveAura(aura->GetId());
                    }
                    else
                        target->RemoveAura(aura->GetId());
                }
            }
            timer = 3000;
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_new_year_evala_auras_305279::OnUpdate, EFFECT_0, SPELL_AURA_MOD_STUN);
    }
};

// 305266
class spell_new_year_evala_auras_305266 : public AuraScript
{
    PrepareAuraScript(spell_new_year_evala_auras_305266);

    uint16 timer = 3000;

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (timer <= diff)
        {
            if (auto target = GetCaster())
            {
                if (target->isInCombat())
                    return;

                if (auto aura = GetAura())
                {
                    if (auto b = target->FindNearestCreature(npc_evala, 80.f))
                    {
                        if (!b->isInCombat())
                            target->RemoveAura(aura->GetId());
                    }
                    else
                        target->RemoveAura(aura->GetId());
                }
            }
            timer = 3000;
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_new_year_evala_auras_305266::OnUpdate, EFFECT_0, SPELL_AURA_PHASE);
    }
};

// 305260 305261
class spell_new_year_evala_auras_305260 : public AuraScript
{
    PrepareAuraScript(spell_new_year_evala_auras_305260);

    uint16 timer = 3000;

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (timer <= diff)
        {
            if (auto target = GetCaster())
            {
                if (target->isInCombat())
                    return;

                if (auto aura = GetAura())
                {
                    if (auto b = target->FindNearestCreature(npc_evala, 80.f))
                    {
                        if (!b->isInCombat())
                            target->RemoveAura(aura->GetId());
                    }
                    else
                        target->RemoveAura(aura->GetId());
                }
            }
            timer = 3000;
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_new_year_evala_auras_305260::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// 305262 305264
class spell_new_year_evala_auras_305262 : public AuraScript
{
    PrepareAuraScript(spell_new_year_evala_auras_305262);

    uint16 timer = 3000;

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (timer <= diff)
        {
            if (auto target = GetCaster())
            {
                if (target->isInCombat())
                    return;

                if (auto aura = GetAura())
                {
                    if (auto b = target->FindNearestCreature(npc_evala, 80.f))
                    {
                        if (!b->isInCombat())
                            target->RemoveAura(aura->GetId());
                    }
                    else
                        target->RemoveAura(aura->GetId());
                }
            }
            timer = 3000;
        }
        else
            timer -= diff;
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_new_year_evala_auras_305262::OnUpdate, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 305247
class spell_new_year_atray_barrage_hit : public SpellScript
{
    PrepareSpellScript(spell_new_year_atray_barrage_hit);

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (!GetHitUnit())
            return;

        GetHitUnit()->CastSpell(GetHitUnit(), 305256, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_new_year_atray_barrage_hit::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 305263
class spell_new_year_evala_neg_check : public SpellScript
{
    PrepareSpellScript(spell_new_year_evala_neg_check);

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        auto caster = GetCaster();
        auto target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->GetDistance(target) <= 1.f)
        {
            caster->RemoveAurasDueToSpell(305260);
            caster->RemoveAurasDueToSpell(305262);
            target->RemoveAurasDueToSpell(305260);
            target->RemoveAurasDueToSpell(305262);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_new_year_evala_neg_check::HandleDamage, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 305265
class spell_new_year_evala_pos_check : public SpellScript
{
    PrepareSpellScript(spell_new_year_evala_pos_check);

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        auto caster = GetCaster();
        auto target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->GetDistance(target) >= 30.f)
        {
            caster->RemoveAurasDueToSpell(305261);
            caster->RemoveAurasDueToSpell(305264);
            target->RemoveAurasDueToSpell(305261);
            target->RemoveAurasDueToSpell(305264);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_new_year_evala_pos_check::HandleDamage, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};






/*

    New Year 2019 world event

*/


//atray
class OutdoorPvPAB_winter_event : public OutdoorPvP
{
public:
    OutdoorPvPAB_winter_event()
    {
        m_TypeId = OUTDOOR_PVP_AB_WINTER_EVENT;
    }

    ~OutdoorPvPAB_winter_event() = default;

    bool SetupOutdoorPvP() override
    {
        RegisterZone(65);
        return true;
    }

    // disabled when event inactive
    /*bool Update(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (!sGameEventMgr->IsActiveEvent(823))
            {
                if (!m_boss.empty())
                {
                    for (auto& guid : m_boss)
                    {
                        if (auto boss = m_map->GetCreature(guid))
                        {
                            if (boss->IsAIEnabled)
                                boss->AI()->DoAction(ACTION_1);
                            boss->DespawnOrUnsummon(1000);
                            m_boss.clear();
                        }
                    }
                }
            }
            else
            {
                if (m_boss.empty())
                {
                    if (auto atray = m_map->SummonCreature(npc_atray, bossPositions[0]))
                        m_boss.push_back(atray->GetGUID());
                }
                if (!m_boss.empty())
                {
                    for (auto& guid : m_boss)
                    {
                        if (auto boss = m_map->GetCreature(guid))
                        {
                            if (boss->isInCombat())
                            {
                                if (boss->GetHealthPct() >= 15)
                                {
                                    ApplyOnEveryPlayerInZone([this](Player* player)
                                    {
                                        if (player->getLevel() >= 100 && !player->isGameMaster())
                                        {
                                            if (!CombatCheck(player))
                                                player->CastSpell(player, 305301, true);
                                            else
                                                player->RemoveAurasDueToSpell(305301);
                                        }
                                    });
                                }
                                else
                                {
                                    ApplyOnEveryPlayerInZone([this](Player* player)
                                    {
                                        if (CombatCheck(player))
                                            player->RemoveAurasDueToSpell(305301);
                                    });
                                }
                            }
                            else
                            {
                                ApplyOnEveryPlayerInZone([this](Player* player)
                                {
                                    player->RemoveAurasDueToSpell(305301);
                                });
                            }
                        }
                    }
                }
            }
            timer = 2000;
        }
        else
            timer -= diff;

        return true;
    }*/

    void HandleGameEventStart(uint32 eventId) override
    {
        if (eventId == 823)
        {
            if (auto atray = m_map->SummonCreature(npc_atray, bossPositions[0]))
                m_boss.push_back(atray->GetGUID());
        }
    }

private:
    uint32 timer = 2000;
    std::vector<ObjectGuid> m_boss{};

    bool CombatCheck(Player* player)
    {
        HostileRefManager& refManager = player->getHostileRefManager();
        HostileReference* ref = refManager.getFirst();

        if (!ref)
            return false;

        while (ref)
        {
            if (auto unit = ref->getSource()->getOwner())
            {
                if (auto cre = unit->ToCreature())
                    if (cre->GetEntry() == 542185)
                        return true;
            }
            ref = ref->next();
        }
        return false;
    }
};

class OutdoorPvP_AB_winter_event : public OutdoorPvPScript
{
public:
    OutdoorPvP_AB_winter_event() : OutdoorPvPScript("outdoorpvp_ab_winter_event") {}

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPvPAB_winter_event();
    }
};


// evala
class OutdoorPvPAB_winter_event_ev : public OutdoorPvP
{
public:
    OutdoorPvPAB_winter_event_ev()
    {
        m_TypeId = OUTDOOR_PVP_AB_WNTR_EVENT_EV;
    }

    ~OutdoorPvPAB_winter_event_ev() = default;

    bool SetupOutdoorPvP() override
    {
        RegisterZone(618);
        return true;
    }

    // disabled when event inactive
    /*bool Update(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (!sGameEventMgr->IsActiveEvent(824))
            {
                if (!m_boss.empty())
                {
                    for (auto& guid : m_boss)
                    {
                        if (auto boss = m_map->GetCreature(guid))
                        {
                            if (boss->IsAIEnabled)
                                boss->AI()->DoAction(ACTION_2);
                            boss->DespawnOrUnsummon(1000);
                            m_boss.clear();
                        }
                    }
                }
            }
            else
            {
                if (m_boss.empty())
                {
                    if (auto evala = m_map->SummonCreature(npc_evala, bossPositions[1]))
                        m_boss.push_back(evala->GetGUID());
                }
                if (!m_boss.empty())
                {
                    for (auto& guid : m_boss)
                    {
                        if (auto boss = m_map->GetCreature(guid))
                        {
                            if (boss->isInCombat())
                            {
                                if (boss->GetHealthPct() >= 15)
                                {
                                    ApplyOnEveryPlayerInZone([this](Player* player)
                                    {
                                        if (player->getLevel() >= 100 && !player->isGameMaster())
                                        {
                                            if (!CombatCheck(player))
                                                player->CastSpell(player, 305301, true);
                                            else
                                                player->RemoveAurasDueToSpell(305301);
                                        }
                                    });
                                }
                                else
                                {
                                    ApplyOnEveryPlayerInZone([this](Player* player)
                                    {
                                        if (CombatCheck(player))
                                            player->RemoveAurasDueToSpell(305301);
                                    });
                                }
                            }
                            else
                            {
                                ApplyOnEveryPlayerInZone([this](Player* player)
                                {
                                    player->RemoveAurasDueToSpell(305301);
                                });
                            }
                        }
                    }
                }
            }
            timer = 2000;
        }
        else
            timer -= diff;

        return true;
    }*/

    void HandleGameEventStart(uint32 eventId) override
    {
        if (eventId == 824)
        {
            if (auto evala = m_map->SummonCreature(npc_evala, bossPositions[1]))
            {
                m_boss.push_back(evala->GetGUID());
                std::list<Creature*> creatures;
                evala->GetCreatureListWithEntryInGrid(creatures, npc_evala_shard, 20.f);
                for (auto cre : creatures)
                {
                    if (!cre->isAlive())
                        cre->Respawn(true);
                }
            }
        }
    }

private:
    uint32 timer = 1000;
    std::vector<ObjectGuid> m_boss{};

    bool CombatCheck(Player* player)
    {
        HostileRefManager& refManager = player->getHostileRefManager();
        HostileReference* ref = refManager.getFirst();

        if (!ref)
            return false;

        while (ref)
        {
            if (auto unit = ref->getSource()->getOwner())
            {
                if (auto cre = unit->ToCreature())
                    if (cre->GetEntry() == 542191)
                        return true;
            }
            ref = ref->next();
        }
        return false;
    }
};

class OutdoorPvP_AB_winter_event_ev : public OutdoorPvPScript
{
public:
    OutdoorPvP_AB_winter_event_ev() : OutdoorPvPScript("outdoorpvp_ab_winter_event_ev") {}

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPvPAB_winter_event_ev();
    }
};




enum WBData_atray
{
    SPELL_ATRAY_BRR_COLD        = 305239,
    SPELL_ATRAY_BRR_COLD_DMG    = 305240,
    SPELL_ATRAY_BLIZZARD        = 305241,
    SPELL_ATRAY_TAIL            = 305243,
    SPELL_ATRAY_KHADGAR_BARIER  = 305245,
    SPELL_ATRAY_BARIER_EFFECT   = 305251,
    SPELL_ATRAY_BARIER_EFFECT_A = 305253,
    SPELL_ATRAY_EXPLOSION       = 305246,
    SPELL_ATRAY_EXPLOSION_F     = 305255,
    SPELL_ATRAY_FROST_BARRAGE   = 305247,
    SPELL_ATRAY_FRST_BARRAGE_AOE= 305256,
    SPELL_ATRAY_FURY            = 305248,
    SPELL_ATRAY_KHADGAR_TS      = 305252,
    SPELL_ATRAY_DEATH_FROST     = 305249,
    SPELL_ATRAY_BREATH          = 305257,
    SPELL_ATRAY_DOT             = 305292,
    SPELL_ATRAY_ADD_EXPL        = 305293,

    SPELL_RAGE_FROSTBOLT        = 305295,
    SPELL_B_ACTIVITY            = 305301,
};

enum AtrayEvents
{
    ATRAY_BLIZZARD              = 1,
    ATRAY_TAIL,
    ATRAY_EXPLOSION,
    ATRAY_FURY,
    ATRAY_FLYPHASE,
    ATRAY_RESET,
    ATRAY_DEATHFROST,
    ATRAY_BREATH,
    ATRAY_DOT,
    ATRAY_ADD,
    ATRAY_RAGE,
};

// 542185
struct boss_new_year_2019_atray : public ScriptedAI
{
    boss_new_year_2019_atray(Creature* creature) : ScriptedAI(creature), summons(me) {}

    EventMap events;
    SummonList summons;
    uint8 explodeCasted = 0;
    uint32 timer = 3000;
    uint32 attackersCheck = 5000;
    bool phase;
    bool ragephase;

    void EnterCombat(Unit* /*who*/) override
    {
        StartDefaultEvents(true, true);
    }

    void StartDefaultEvents(bool first, bool explodeReset)
    {
        phase = false;
        events.RescheduleEvent(ATRAY_ADD, urand(25000, 30000));
        events.RescheduleEvent(ATRAY_DOT, first ? 8000 : 6000);
        events.RescheduleEvent(ATRAY_BLIZZARD, first ? 20000 : 10000);
        events.RescheduleEvent(ATRAY_TAIL, first ? urand(12000, 17000) : urand(10000, 12000));
        events.RescheduleEvent(ATRAY_EXPLOSION, first ? urand(9000, 11000) : 5000);
        events.RescheduleEvent(ATRAY_DEATHFROST, urand(25000, 30000));
        events.RescheduleEvent(ATRAY_RAGE, 2000);
        if (explodeReset)
            explodeCasted = 0;
    }

    void StartFlyPhase()
    {
        events.Reset();
        events.RescheduleEvent(ATRAY_BLIZZARD, 8000);
        events.RescheduleEvent(ATRAY_DEATHFROST, urand(25000, 30000));
        me->StopAttack(true);
        SetFlyMode(true);
        me->SetAnimTier(3);
        me->AddDelayedEvent(2000, [this]() -> void
        {
            me->GetMotionMaster()->MovePoint(1, AtrayFlyPhasePos[urand(0, 7)], false);
        });
    }

    void FuryPhase()
    {
        events.Reset();
        events.RescheduleEvent(ATRAY_RESET, 35000);
        DoCast(me, SPELL_ATRAY_KHADGAR_TS, true);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
            summons.DespawnAll();
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_ATRAY_KHADGAR_TS)
        {
            Position pos = target->GetPosition();
            pos.Relocate(pos.m_positionX - frand(2, 3), pos.m_positionY + frand(-2, 2), pos.m_positionZ, pos.m_orientation);
            if (auto khadgar = me->SummonCreature(npc_atray_khadgar, pos, TEMPSUMMON_TIMED_DESPAWN, 34000))
            {
                khadgar->SetReactState(REACT_PASSIVE);
                if (khadgar->IsAIEnabled)
                    khadgar->AI()->Talk(1);
                khadgar->AddDelayedEvent(3000, [khadgar]() -> void
                {
                    if (auto tr = khadgar->SummonCreature(npc_atray_khadgar_tr, khadgar->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 31000))
                    {
                        tr->SetReactState(REACT_PASSIVE);
                        tr->CastSpell(tr, SPELL_ATRAY_BARIER_EFFECT_A);
                    }
                });
                khadgar->AddDelayedEvent(2000, [khadgar]() -> void
                {
                    khadgar->CastSpell(khadgar, SPELL_ATRAY_KHADGAR_BARIER);
                });
                me->AddDelayedCombat(2000, [this]() -> void
                {
                    Talk(1);
                    DoCast(me, SPELL_ATRAY_FURY);
                });
            }
        }
    }

    void Adds(uint8 count)
    {
        for (uint8 i = 0; i < count; ++i)
        {
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                if (auto add = me->SummonCreature(npc_atray_son, target->GetPositionX() + frand(1, 3), target->GetPositionY() - frand(1, 3), target->GetPositionZ() + 1.5f, 1.f))
                    add->Attack(target, true);
        }
    }

    void FrostMinions(uint8 count)
    {
        std::list<Position> randPos;
        me->GenerateNonDuplicatePoints(randPos, me->GetPosition(), count, 10.f, 31.f, 15.f);
        for (auto pos : randPos)
        {
            if (auto portal = me->SummonCreature(npc_atray_add, pos))
                portal->SetReactState(REACT_PASSIVE);
        }
    }

    void MoveAction(uint32 point, bool adds, uint8 addcount)
    {
        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
            me->SetFacingTo(target);
        me->AddDelayedCombat(1000, [this]() -> void
        {
            me->CastSpell(me, SPELL_ATRAY_FROST_BARRAGE);
        });
        me->AddDelayedCombat(3000, [this, adds, addcount]() -> void
        {
            me->CastSpell(me, SPELL_ATRAY_FROST_BARRAGE);
            if (adds)
                Adds(addcount);
        });
        me->AddDelayedCombat(6000, [this, point]() -> void
        {
            me->GetMotionMaster()->MovePoint(point, AtrayFlyPhasePos[urand(0, 7)], false);
        });
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
        case 1:
        case 2:
            MoveAction(id + 1, false, 0);
            break;
        case 3:
            MoveAction(id + 1, true, 10);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            MoveAction(id + 1, true, 10);
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            MoveAction(id + 1, false, 0);
            break;
        case 12:
            MoveAction(id + 1, true, 10);
            break;
        case 13:
        case 14:
        case 15:
            MoveAction(id + 1, true, 10);
            break;
        case 16:
        case 17:
        case 18:
            MoveAction(id + 1, false, 0);
            break;
        case 19:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                me->SetFacingTo(target);
            me->AddDelayedCombat(1000, [this]() -> void
            {
                me->CastSpell(me, SPELL_ATRAY_FROST_BARRAGE);
            });
            me->AddDelayedCombat(3000, [this]() -> void
            {
                me->CastSpell(me, SPELL_ATRAY_FROST_BARRAGE);
            });
            me->AddDelayedCombat(6000, [this]() -> void
            {
                me->GetMotionMaster()->MovePoint(20, AtrayFlyEndPos, false);
            });
            break;
        case 20:
            me->SetFacingTo(0.8f);
            SetFlyMode(false);
            me->SetAnimTier(0);
            me->GetMotionMaster()->MoveFall();
            me->SetReactState(REACT_AGGRESSIVE, 2000);
            StartDefaultEvents(false, true);
            break;
        }
    }

    void Reset() override
    {
        events.Reset();
        summons.DespawnAll();
        explodeCasted = 0;
        me->SetReactState(REACT_AGGRESSIVE);
        SetFlyMode(false);
        me->SetAnimTier(0);
        phase = false;
        me->RemoveAura(SPELL_ATRAY_FURY);
        ragephase = false;
        me->UpdateMaxHealth();
    }

    void EnterEvadeMode()
    {
        CreatureAI::EnterEvadeMode();
        me->SetHomePosition(AtrayHomePos);
        me->NearTeleportTo(AtrayHomePos);
        me->SetVisible(false);
        me->AddDelayedEvent(8000, [this]() -> void
        {
            me->SetVisible(true);
        });
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case npc_atray_son:
        case npc_atray_add:
            summon->DespawnOrUnsummon(1000);
            break;
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void JustDied(Unit* /*killer*/) override
    {
        summons.DespawnAll();
    }

    bool AttackersCheck()
    {
        if (me->GetSizeSaveThreat() <= 4)
            return false;
        else
            return true;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (timer <= diff)
        {
            timer = 3000;
            if (me->GetDistance(3606.88f, -158.52f, 60.67f) >= 70.f)
                EnterEvadeMode();

            me->SetHomePosition(me->GetPosition());
        }
        else
            timer -= diff;

        if (attackersCheck <= diff)
        {
            attackersCheck = 5000;
            if (!AttackersCheck())
                ragephase = true;
            else
                ragephase = false;
        }
        else
            attackersCheck -= diff;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case ATRAY_BLIZZARD:
            {
                for (uint8 i = 0; i < 4; ++i)
                {
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 75.0f, true))
                    {
                        if (auto bTr = me->SummonCreature(npc_atray_blizzard, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 30000))
                        {
                            bTr->GetMotionMaster()->MoveRandom(8.f);
                        }
                    }
                }
                events.RescheduleEvent(ATRAY_BLIZZARD, 23000);
                break;
            }
            case ATRAY_TAIL:
            {
                DoCast(me, SPELL_ATRAY_TAIL, true);
                events.RescheduleEvent(ATRAY_TAIL, urand(12000, 17000));
                break;
            }
            case ATRAY_EXPLOSION:
            {
                DoCast(me, SPELL_ATRAY_EXPLOSION);
                ++explodeCasted;
                if (explodeCasted == 6)
                {
                    phase = true;
                    events.ScheduleEvent(ATRAY_FURY, 3000);
                }
                else if (explodeCasted == 8)
                {
                    phase = true;
                    events.ScheduleEvent(ATRAY_FLYPHASE, 3000);
                }
                events.RescheduleEvent(ATRAY_EXPLOSION, urand(14000, 18000));
                if (!phase)
                    events.RescheduleEvent(ATRAY_BREATH, urand(3000, 4000));
                break;
            }
            case ATRAY_FURY:
            {
                FuryPhase();
                break;
            }
            case ATRAY_FLYPHASE:
            {
                StartFlyPhase();
                break;
            }
            case ATRAY_RESET:
            {
                StartDefaultEvents(false, false);
                break;
            }
            case ATRAY_DEATHFROST:
            {
                for (uint8 i = 0; i < 2; ++i)
                {
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true, -ATRAY_DEATHFROST))
                        DoCast(target, SPELL_ATRAY_DEATH_FROST, true);
                }
                events.RescheduleEvent(ATRAY_DEATHFROST, urand(28000, 32000));
                break;
            }
            case ATRAY_BREATH:
            {
                DoCast(me, SPELL_ATRAY_BREATH);
                break;
            }
            case ATRAY_DOT:
            {
                if (auto target = me->getVictim())
                    DoCast(target, SPELL_ATRAY_DOT, true);
                events.RescheduleEvent(ATRAY_DOT, 12000);
                break;
            }
            case ATRAY_ADD:
            {
                FrostMinions(urand(2, 3));
                events.RescheduleEvent(ATRAY_ADD, urand(25000, 30000));
                break;
            }
            case ATRAY_RAGE:
            {
                if (ragephase)
                {
                    if (auto target = me->getVictim())
                        DoCast(target, SPELL_RAGE_FROSTBOLT, true);
                }
                events.RescheduleEvent(ATRAY_RAGE, 2000);
                break;
            }
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 542188
struct npc_new_year_2019_atray_blizzard : public ScriptedAI
{
    npc_new_year_2019_atray_blizzard(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 timer = 2500;
    uint8 count = 0;

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (count < 4)
            {
                DoCast(me, SPELL_ATRAY_BLIZZARD, true);
                ++count;
            }
            timer = 2500;
        }
        else
            timer -= diff;
    }
};

// 542200
struct npc_new_year_2019_atray_frost_minion : public ScriptedAI
{
    npc_new_year_2019_atray_frost_minion(Creature* creature) : ScriptedAI(creature) {}

    uint32 timer = urand(2000, 4500);
    uint8 count = 0;

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            DoCast(me, SPELL_ATRAY_ADD_EXPL);
            timer = urand(4500, 6000);
        }
        else
            timer -= diff;
    }
};



enum WBData_evala
{
    SPELL_EVALA_TRANSFORM           = 305259,
    SPELL_EVALA_VOID_ZONE           = 305281,
    SPELL_EVALA_VOID_ZONE_DMG       = 305282,

    SPELL_EVALA_NEG_CHARGE          = 305260,
    SPELL_EVALA_NEG_CHARGE_TS       = 305280,
    SPELL_EVALA_POS_CHARGE          = 305261,
    SPELL_EVALA_POS_CHARGE_TS       = 305294,
    SPELL_EVALA_NEG_CH_CHECK        = 305262,
    SPELL_EVALA_NEG_CH_CHECK_EFF    = 305263,
    SPELL_EVALA_POS_CH_CHECK        = 305264,
    SPELL_EVALA_POS_CH_CHECK_EFF    = 305265,
    SPELL_EVALA_PHASE               = 305266, //phase 5436
    SPELL_EVALA_FROST_EXPL_VIS      = 305267,
    SPELL_EVALA_FROST_EXPL_LOW      = 305268,
    SPELL_EVALA_FROST_EXPL_HIGH     = 305269,
    SPELL_EVALA_FROST_ARMOR         = 305270,
    SPELL_EVALA_FROST_ARMOR_DMG     = 305271,
    SPELL_EVALA_DEFENCE             = 305272,
    SPELL_EVALA_INSTABILITY         = 305273,
    SPELL_EVALA_FROSTBOLT           = 305274,
    SPELL_EVALA_FROSTFEVER          = 305276,
    SPELL_EVALA_FROSTFIREBOLT       = 305277,
    SPELL_EVALA_ICY_TOMB_TS         = 305278,
    SPELL_EVALA_ICY_TOMB_STUN       = 305279,
    SPELL_EVALA_SILIENCE            = 37031,

    SPELL_ICE_COMET                 = 217928, //sample

    VOID_ZONE_P_COUNT               = 2,
    MAX_PORTAL_ADDS                 = 2,
    MAX_VALKYR                      = 4,
};

enum EvalaEvents
{
    EVALA_CHARGES                   = 1,
    EVALA_VOID_ZONE_PERIODIC,
    EVALA_PHASE,
    EVALA_VOID_ZONE_CLOSE,
    EVALA_FROST_ARMOR,
    EVALA_SPHERES,
    EVALA_TOMBS,
    EVALA_VALKYR,
    EVALA_RAGE,
};

// 542191
struct boss_new_year_2019_evala : public ScriptedAI
{
    boss_new_year_2019_evala(Creature* creature) : ScriptedAI(creature), summons(me) 
    {
        PreAdds();
        DoCast(me, SPELL_EVALA_TRANSFORM, true);
        EvalaFlags(true);
    }

    EventMap events;
    SummonList summons;
    uint8 addDied = 0;
    uint32 attack = 5000;
    bool ragephase;

    void PreAdds()
    {
        for (uint8 i = 0; i < 7; ++i)
        {
            if (auto valkyr = me->SummonCreature(npc_evala_guard, valkyrGuardPos[i]))
                valkyr->GetMotionMaster()->MoveRandom(3.f);
        }
    }

    void EvalaFlags(bool enable)
    {
        me->SetVirtualItem(0, enable ? 0 : 40491);
        me->SetVirtualItem(1, enable ? 0 : 40491);
        if (enable)
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
        else
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case npc_evala_guard:
            ++addDied;
            summon->DespawnOrUnsummon(500);
            if (addDied == 7)
            {
                EvalaFlags(false);
                me->RemoveAura(SPELL_EVALA_TRANSFORM);
            }
            break;
        case npc_evala_tomb:
        case npc_evala_sphere:
        case npc_evala_minion:
            summon->DespawnOrUnsummon(300);
            break;
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVALA_CHARGES, urand(28000, 33000));
        events.RescheduleEvent(EVALA_VOID_ZONE_PERIODIC, urand(18000, 22000));
        events.RescheduleEvent(EVALA_PHASE, 90000);
        events.ScheduleEvent(EVALA_VOID_ZONE_CLOSE, 35000);
        events.RescheduleEvent(EVALA_FROST_ARMOR, 15000);
        events.RescheduleEvent(EVALA_SPHERES, urand(25000, 28000));
        events.RescheduleEvent(EVALA_TOMBS, 55000);
        events.RescheduleEvent(EVALA_VALKYR, 75000);
        events.RescheduleEvent(EVALA_RAGE, 2500);
    }

    void JustDied(Unit* /*killer*/) override
    {
        summons.DespawnAll();
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
        case SPELL_EVALA_ICY_TOMB_TS:
            target->CastSpell(target, SPELL_EVALA_ICY_TOMB_STUN, true);
            me->AddDelayedCombat(500, [this, target]() -> void
            {
                if (auto tomb = target->SummonCreature(npc_evala_tomb, target->GetPosition()))
                    tomb->SetReactState(REACT_PASSIVE);
            });
            break;
        case SPELL_EVALA_NEG_CHARGE_TS:
            target->CastSpell(target, SPELL_EVALA_NEG_CHARGE, true);
            break;
        case SPELL_EVALA_POS_CHARGE_TS:
            target->CastSpell(target, SPELL_EVALA_POS_CHARGE, true);
            break;
        }

    }

    void Reset() override
    {
        events.Reset();
        summons.DespawnAll();
        me->RemoveAurasDueToSpell(SPELL_EVALA_DEFENCE);
        me->RemoveAurasDueToSpell(SPELL_EVALA_FROST_ARMOR);
        ragephase = false;
        me->UpdateMaxHealth();

        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, npc_evala_frostmage, 125.0f);
        GetCreatureListWithEntryInGrid(creList, me, npc_evala_frostwarr, 125.0f);
        GetCreatureListWithEntryInGrid(creList, me, npc_evala_tomb, 125.0f);
        for (auto const& portadd : creList)
        {
            portadd->DespawnOrUnsummon(100);
            if (portadd->GetEntry() == npc_evala_tomb)
                portadd->Kill(portadd);
        }

        std::list<Player*> plrList;
        me->GetPlayerListInGrid(plrList, 125.f);
        if (!plrList.empty())
        {
            for (Player* itr : plrList)
            {
                if (!itr)
                    continue;

                if (itr && itr->IsInWorld())
                {
                    itr->RemoveAurasDueToSpell(SPELL_EVALA_PHASE);
                    itr->RemoveAurasDueToSpell(SPELL_EVALA_NEG_CHARGE);
                    itr->RemoveAurasDueToSpell(SPELL_EVALA_POS_CHARGE);
                    itr->RemoveAurasDueToSpell(SPELL_EVALA_NEG_CH_CHECK);
                    itr->RemoveAurasDueToSpell(SPELL_EVALA_POS_CH_CHECK);
                }
            }
        }
    }

    void EnterEvadeMode()
    {
        CreatureAI::EnterEvadeMode();
        me->SetVisible(false);
        me->AddDelayedEvent(8000, [this]() -> void
        {
            me->SetVisible(true);
        });
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            me->RemoveAurasDueToSpell(SPELL_EVALA_DEFENCE);
            break;
        case ACTION_2:
            summons.DespawnAll();
            break;
        }
    }

    void PhaseActions(uint8 portalsCount)
    {
        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, npc_evala_shard, 60.0f);
        if (!creList.empty())
        {
            for (auto const& sh : creList)
            {
                if (!sh->isAlive())
                    sh->Respawn(true);
            }
        }

        std::list<Position> randPhase;
        me->GenerateNonDuplicatePoints(randPhase, me->GetPosition(), portalsCount, 10.f, 35.f, 15.f);
        for (auto pos : randPhase)
        {
            if (auto portal = me->SummonCreature(npc_evala_portal, pos))
                portal->SetReactState(REACT_PASSIVE);
        }
        DoCast(me, SPELL_EVALA_DEFENCE);
    }

    void VoidZoneActions(uint8 count)
    {
        std::list<Position> randVzones;
        me->GenerateNonDuplicatePoints(randVzones, me->GetPosition(), count, 10.f, 34.f, 12.f);
        for (auto pos : randVzones)
        {
            me->AddDelayedEvent(10, [this, pos]() -> void
            {
                if (auto vz = me->SummonCreature(npc_evala_void_zone, pos))
                {
                    vz->SetReactState(REACT_PASSIVE);
                }
            });
        }
    }

    void SphereActions(uint8 count)
    {
        std::list<Position> randSpheresPos;
        me->GenerateNonDuplicatePoints(randSpheresPos, me->GetPosition(), count, 10.f, 33.f, 15.f);
        for (auto pos : randSpheresPos)
        {
            if (auto s = me->SummonCreature(npc_evala_sphere, pos))
            {
                s->SetReactState(REACT_PASSIVE);
            }
        }
    }

    bool CheckAuras(Unit* target)
    {
        if (target->HasAura(SPELL_EVALA_ICY_TOMB_STUN) || target->HasAura(SPELL_EVALA_NEG_CHARGE) || target->HasAura(SPELL_EVALA_POS_CHARGE) ||
            target->IsOnVehicle())
            return false;
        
        return true;
    }

    void ValkyrActions()
    {
        for (uint8 i = 0; i < MAX_VALKYR; ++i)
        {
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
            {
                if (auto v = me->SummonCreature(npc_evala_minion, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ() + 4.5f, 0.f))
                    v->SetReactState(REACT_PASSIVE);
            }
        }
    }

    bool AttackersCheck()
    {
        if (me->GetSizeSaveThreat() <= 4)
            return false;
        else
            return true;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (attack <= diff)
        {
            if (AttackersCheck())
                ragephase = false;
            else
                ragephase = true;

            attack = 5000;
        }
        else 
            attack -= diff;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVALA_CHARGES:
            {
                DoCast(me, SPELL_EVALA_NEG_CHARGE_TS, true);
                me->AddDelayedCombat(500, [this]() -> void
                {
                    DoCast(me, SPELL_EVALA_POS_CHARGE_TS, true);
                });
                events.RescheduleEvent(EVALA_CHARGES, urand(28000, 33000));
                break;
            }
            case EVALA_VOID_ZONE_PERIODIC:
            {
                for (uint8 i = 0; i < VOID_ZONE_P_COUNT; ++i)
                {
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true))
                        DoCast(target, SPELL_EVALA_VOID_ZONE, true);
                }
                events.RescheduleEvent(EVALA_VOID_ZONE_PERIODIC, urand(22000, 25000));
                break;
            }
            case EVALA_PHASE:
            {
                PhaseActions(2);
                events.RescheduleEvent(EVALA_PHASE, 100000);
                break;
            }
            case EVALA_VOID_ZONE_CLOSE:
            {
                VoidZoneActions(5);
                events.ScheduleEvent(EVALA_VOID_ZONE_CLOSE, 35000);
                break;
            }
            case EVALA_FROST_ARMOR:
            {
                DoCast(me, SPELL_EVALA_FROST_ARMOR);
                events.RescheduleEvent(EVALA_FROST_ARMOR, 15000);
                break;
            }
            case EVALA_SPHERES:
            {
                SphereActions(4);
                events.RescheduleEvent(EVALA_SPHERES, urand(25000, 28000));
                break;
            }
            case EVALA_TOMBS:
            {
                DoCast(me, SPELL_EVALA_ICY_TOMB_TS);
                events.RescheduleEvent(EVALA_TOMBS, 60000);
                break;
            }
            case EVALA_VALKYR:
            {
                ValkyrActions();
                events.RescheduleEvent(EVALA_VALKYR, 85000);
                break;
            }
            case EVALA_RAGE:
            {
                if (ragephase)
                {
                    if (auto target = me->getVictim())
                        DoCast(target, SPELL_RAGE_FROSTBOLT, true);

                    events.RescheduleEvent(EVALA_RAGE, 2000);
                }
                break;
            }
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 542199
struct npc_new_year_2019_evala_portal : public ScriptedAI
{
    npc_new_year_2019_evala_portal(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;
    uint8 summ = 0;

    void IsSummonedBy(Unit* summoner) override
    {
        events.RescheduleEvent(EVENT_1, 1000);
    }

    void OnSpellClick(Unit* clicker) override
    {
        if (!clicker->HasAura(SPELL_EVALA_PHASE))
            clicker->CastSpell(clicker, SPELL_EVALA_PHASE, true);
        else
            clicker->RemoveAurasDueToSpell(SPELL_EVALA_PHASE);
    }

    void UpdateAI(uint32 diff)
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
            {
                Position pos = me->GetPosition();
                uint32 add[2] = { npc_evala_frostmage, npc_evala_frostwarr };
                if (auto adds = me->SummonCreature(add[urand(0, 1)], pos.m_positionX + frand(1, 2), pos.m_positionY + frand(0, 2), pos.m_positionZ, pos.m_orientation))
                    if (auto target = adds->SelectNearestPlayerNotGM(37.f))
                        adds->GetMotionMaster()->MovePoint(1, target->GetPosition());

                ++summ;
                if (summ == 1)
                    events.RescheduleEvent(EVENT_1, 5000);
                else if (summ == 2)
                    events.RescheduleEvent(EVENT_1, 25000);
                else if (summ == 3)
                    me->DespawnOrUnsummon(5000);
            }
        }
    }
};

// 542196
struct npc_new_year_2019_void_zone : public ScriptedAI
{
    npc_new_year_2019_void_zone(Creature* creature) : ScriptedAI(creature) 
    {
        Actions();
    }

    EventMap events;

    void Actions()
    {
        DoCast(me, SPELL_EVALA_FROST_EXPL_VIS, true);
        events.ScheduleEvent(EVENT_1, 8000);
    }

    void UpdateAI(uint32 diff)
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
            {
                if (me->SelectNearestPlayer(4.5f))
                    DoCast(me, SPELL_EVALA_FROST_EXPL_LOW, true);
                else
                    DoCast(me, SPELL_EVALA_FROST_EXPL_HIGH, true);

                me->DespawnOrUnsummon(500);
            }
        }
    }
};

// 542193
struct npc_new_year_2019_evala_arctic_cloud : public ScriptedAI
{
    npc_new_year_2019_evala_arctic_cloud(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void IsSummonedBy(Unit* summoner) override
    {
        events.ScheduleEvent(EVENT_1, urand(1000, 2000));
    }

    void UpdateAI(uint32 diff)
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
            {
                DoCast(SPELL_EVALA_INSTABILITY);
                events.ScheduleEvent(EVENT_1, urand(2500, 4000));
            }
        }
    }
};

// 542198
struct npc_new_year_2019_evala_shard : public ScriptedAI
{
    npc_new_year_2019_evala_shard(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 3000);
    }

    void Reset() override
    {
        events.Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        std::list<Player*> plrList;
        me->GetPlayerListInGrid(plrList, 80.f);
        if (!plrList.empty())
        {
            for (Player* itr : plrList)
            {
                if (!itr)
                    continue;

                if (itr && itr->IsInWorld())
                {
                    itr->RemoveAurasDueToSpell(SPELL_EVALA_PHASE);
                }
            }
        }

        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, npc_evala, 80.0f);
        if (!creList.empty())
        {
            for (auto const& ev : creList)
            {
                if (ev->IsAIEnabled)
                    ev->AI()->DoAction(ACTION_1);
            }
        }
    }

    void UpdateAI(uint32 diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
            {
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                    DoCast(target, SPELL_EVALA_FROSTBOLT);

                events.ScheduleEvent(EVENT_1, urand(2000, 3000));
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 542194
struct npc_new_year_2019_evala_frostmage : public ScriptedAI
{
    npc_new_year_2019_evala_frostmage(Creature* creature) : ScriptedAI(creature) 
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    EventMap events;

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 4000);
    }

    void Reset() override
    {
        events.Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(200);
    }

    void UpdateAI(uint32 diff)
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
            {
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                    DoCast(target, SPELL_EVALA_FROSTFIREBOLT);

                events.ScheduleEvent(EVENT_1, urand(4000, 7000));
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 542195
struct npc_new_year_2019_evala_frostwarr : public ScriptedAI
{
    npc_new_year_2019_evala_frostwarr(Creature* creature) : ScriptedAI(creature) 
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    EventMap events;

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 3000);
    }

    void Reset() override
    {
        events.Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(200);
    }

    void UpdateAI(uint32 diff)
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
            {
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true, -SPELL_EVALA_FROSTFEVER))
                    DoCast(target, SPELL_EVALA_FROSTFEVER);

                events.ScheduleEvent(EVENT_1, urand(7000, 11000));
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 542190
struct npc_new_year_2019_evala_minion : public VehicleAI
{
    npc_new_year_2019_evala_minion(Creature* creature) : VehicleAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        SetFlyMode(true);
    }

    uint32 timer = 7000;
    EventMap events;
    bool insert = false;

    void Reset() override
    {
        VehicleAI::Reset();
    }

    void IsSummonedBy(Unit* summoner) override
    {
        events.RescheduleEvent(EVENT_1, urand(7000, 8000));
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
        case 1:
            if (auto veh = me->GetVehicleKit())
            {
                if (auto pass = veh->GetPassenger(0))
                {
                    Position pos = me->GetPosition();
                    pass->ExitVehicle(&pos);
                    pass->CastSpell(pass, SPELL_EVALA_SILIENCE, true);
                    pass->RemoveAurasByType(SPELL_AURA_FEATHER_FALL);
                    me->DespawnOrUnsummon(5000);
                }
            }
            break;
        }
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatID*/, bool apply) override
    {
        me->StopAttack(true);
        SetFlyMode(true);
        me->SetAnimTier(3);
        me->SetSpeed(MOVE_FLIGHT, 0.2f);
        me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 70.f);
        me->AddDelayedEvent(14000, [this]() -> void
        {
            me->SetSpeed(MOVE_FLIGHT, 1.8f);
        });
    }

    bool CheckAuras(Player* target)
    {
        if (target->HasAura(SPELL_EVALA_ICY_TOMB_STUN) || target->HasAura(SPELL_EVALA_NEG_CHARGE) || target->HasAura(SPELL_EVALA_POS_CHARGE) ||
            target->IsOnVehicle())
            return false;

        return true;
    }

    void ValkyrAct()
    {
        if (auto target = me->SelectNearestPlayerNotGM(30.f))
        {
            if (CheckAuras(target) && !insert)
            {
                insert = true;
                target->CastSpell(me, 46598, true);
            }
        }
    }

    void UpdateAI(uint32 diff)
    {
        if (insert)
            return;

        if (timer <= diff)
        {
            if (auto veh = me->GetVehicleKit())
                if (!veh->GetPassenger(0))
                    events.RescheduleEvent(EVENT_1, 500);

            timer = 7000;
        }
        else
            timer -= diff;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_1)
                ValkyrAct();
        }
    }
};

// 542197
struct npc_new_year_2019_evala_tomb : public ScriptedAI
{
    npc_new_year_2019_evala_tomb(Creature* creature) : ScriptedAI(creature) {}

    uint32 timer = 1500;

    void JustDied(Unit* /*killer*/) override
    {
        if (auto owner = me->GetAnyOwner())
            owner->RemoveAurasDueToSpell(SPELL_EVALA_ICY_TOMB_STUN);
    }

    void UpdateAI(uint32 diff)
    {
        if (timer <= diff)
        {
            if (auto owner = me->GetAnyOwner())
                if (owner->isDead())
                    me->DespawnOrUnsummon(500);

            timer = 1500;
        }
        else
            timer -= diff;
    }
};

// 542192
struct npc_new_year_2019_evala_guard : public ScriptedAI
{
    npc_new_year_2019_evala_guard(Creature* creature) : ScriptedAI(creature) {}

    uint32 timer = 2000;
    uint32 attack = 2000;

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(100);
    }

    void UpdateAI(uint32 diff)
    {
        if (timer <= diff)
        {
            if (!sGameEventMgr->IsActiveEvent(824))
                me->DespawnOrUnsummon(100);

            timer = 1500;
        }
        else
            timer -= diff;

        if (attack <= diff)
        {
            if (auto target = me->getVictim())
                DoCast(target, SPELL_EVALA_FROSTFIREBOLT);

            attack = 1500;
        }
        else
            attack -= diff;
    }
};




/*
        
        Ingame Bonuses

*/


// 542000
enum Data
{
    DB_MENU_ID = 542000,
    DB_GOSSIP = 300001,
};

class npc_server_bonuses : public CreatureScript
{
public:
    npc_server_bonuses() : CreatureScript("npc_server_bonuses") {}

    std::unordered_map<ObjectGuid, uint32> RewardsPreview{};

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        LocaleConstant localeConstant = player->GetSession()->GetSessionDbLocaleIndex();

        player->ADD_GOSSIP_ITEM_DB(DB_MENU_ID, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM_DB(DB_MENU_ID, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM_DB(DB_MENU_ID, 3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        //player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, sObjectMgr->GetTrinityString(600023, localeConstant), GOSSIP_SENDER_MAIN, 3, "", 0, true);
        //player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, sObjectMgr->GetTrinityString(600024, localeConstant), GOSSIP_SENDER_MAIN, 4, "", 1, true);

        player->SEND_GOSSIP_MENU(DB_GOSSIP, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)  override
    {
        player->PlayerTalkClass->ClearMenus();
        CurrencyTypesEntry const* currencyType = sCurrencyTypesStore.LookupEntry(1160);

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            if (currencyType) //if currency not found = vendor not open for avoid buy items for free
                player->GetSession()->SendListInventory(creature->GetGUID());
        break;
        case GOSSIP_ACTION_INFO_DEF + 2:
        {
            auto itr = RewardsPreview.find(player->GetGUID());
            if ((itr != RewardsPreview.end() && time(NULL) - (*itr).second >= 90) || itr == RewardsPreview.end())
            {
                if (auto prev = player->SummonCreature(542002, creature->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 90000, 0, player->GetGUID()))
                {
                    RewardsPreview[player->GetGUID()] = time(NULL);
                    prev->AddPlayerInPersonnalVisibilityList(player->GetGUID());
                    prev->GetMotionMaster()->MoveFollow(player, 1.f, 2.f);
                }
            }
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            ChatHandler(player).PSendSysMessage(542144);
            break;
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 /*sender*/, uint32 action, const char* code) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1:
            temp1 = atol(code);
            break;
        case 2:
            temp2 = atol(code);
            break;
        }
        return OnGossipHello(player, creature);
    }

private:
    uint32 temp1 = 0;
    uint32 temp2 = 0;
};


// 542140
class npc_server_bonuses_table : public CreatureScript
{
public:
    npc_server_bonuses_table() : CreatureScript("npc_server_bonuses_table") {}

    bool EventCheck()
    {
        for (uint32 events : {813, 814, 815, 816, 817, 818, 819, 820, 821})
            if (sGameEventMgr->IsActiveEvent(events))
                return true;

        return false;
    }

    void BattleShipTele(Player* player)
    {
        Position pos = player->GetTeam() == ALLIANCE ? alliShipStart_APos : alliShipStart_HPos;
        player->TeleportTo(1, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
    }

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        if (EventCheck())
            player->ADD_GOSSIP_ITEM_DB(542140, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        
        if (player->HasAura(305225))
            player->ADD_GOSSIP_ITEM_DB(542140, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        if (player->GetQuestStatus(60023) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(60024) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM_DB(542140, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        //player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, sObjectMgr->GetTrinityString(600023, localeConstant), GOSSIP_SENDER_MAIN, 3, "", 0, true);
        player->SEND_GOSSIP_MENU(300001, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)  override
    {
        player->PlayerTalkClass->ClearMenus();
        CurrencyTypesEntry const* currencyType = sCurrencyTypesStore.LookupEntry(1160);

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
            if (EventCheck() && !player->HasAura(event_deserter))
            {
                if (!player->HasAura(305228))
                    player->CastSpell(player, 305228, true);
                
                LocaleConstant locale = sWorld->GetDefaultDbcLocale();
                std::string text = sObjectMgr->GetTrinityString(player->GetTeam() == ALLIANCE ? 542146 : 542147, locale);

                ChatHandler(player).PSendSysMessage(542145, text.c_str());
                Position pos = player->GetTeam() == ALLIANCE ? aliWSGResPos : hordeWSGResPos;
                player->TeleportTo(2100, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
            }
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
        {
            if (auto count = player->GetAuraCount(305225))
            {
                if (currencyType)
                {
                    player->ModifyCurrency(1160, count, true, true);
                    player->RemoveAura(305225);
                }
            }
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 3:
        {
            player->CastSpell(player, 53208, true);
            BattleShipTele(player);
        }
        break;
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 /*sender*/, uint32 action, const char* code) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1:
            temp1 = atol(code);
            break;
        case 2:
            temp2 = atol(code);
            break;
        }
        return OnGossipHello(player, creature);
    }

private:
    uint32 temp1 = 0;
    uint32 temp2 = 0;
};

// 542002
class npc_server_bonuses_preview : public CreatureScript
{
public:
    npc_server_bonuses_preview() : CreatureScript("npc_server_bonuses_preview") {}

    bool CanFly(Creature* creature)
    {
        for (uint32 auras : {305191, 305192, 305195})
            if (creature->HasAura(auras))
                return true;

        return false;
    }

    bool OnGossipHello(Player* player, Creature* creature)  override
    {
        player->ADD_GOSSIP_ITEM_DB(542002, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM_DB(542002, 1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM_DB(542002, 2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM_DB(542002, 3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        player->ADD_GOSSIP_ITEM_DB(542002, 4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        player->ADD_GOSSIP_ITEM_DB(542002, 5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
        player->ADD_GOSSIP_ITEM_DB(542002, 6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
        player->ADD_GOSSIP_ITEM_DB(542002, 7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
        player->ADD_GOSSIP_ITEM_DB(542002, 8, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
        player->ADD_GOSSIP_ITEM_DB(542002, 9, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
        player->ADD_GOSSIP_ITEM_DB(542002, 10, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
        player->ADD_GOSSIP_ITEM_DB(542002, 11, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
        if (CanFly(creature))
            player->ADD_GOSSIP_ITEM_DB(542002, 12, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13); // show fly menu

        player->ADD_GOSSIP_ITEM_DB(542002, 13, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
        player->ADD_GOSSIP_ITEM_DB(542002, 14, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
        player->ADD_GOSSIP_ITEM_DB(542002, 15, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);

        player->SEND_GOSSIP_MENU(300001, creature->GetGUID());
        return true;
    }

    void Cast(Creature* creature, uint32 spell)
    {
        creature->RemoveAurasByType(SPELL_AURA_MOUNTED);
        creature->RemoveAurasByType(SPELL_AURA_DUMMY);
        creature->CastSpell(creature, spell, true);
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)  override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            Cast(creature, 305184);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 2:
        {
            Cast(creature, 305185);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 3:
        {
            Cast(creature, 305186);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 4:
        {
            Cast(creature, 305187);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 5:
        {
            Cast(creature, 305188);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 6:
        {
            Cast(creature, 305189);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 7:
        {
            Cast(creature, 305190);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 8:
        {
            Cast(creature, 305191);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 9:
        {
            Cast(creature, 305192);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 10:
        {
            Cast(creature, 305193);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 11:
        {
            Cast(creature, 305194);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 12:
        {
            Cast(creature, 305195);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 13:
        {
            creature->SetCanFly(true);

            float x, y, z, o;
            creature->GetPosition(x, y, z, o);
            creature->GetMotionMaster()->MovePoint(0, x + 8.f, y + 8.f, z + 8.f);
            creature->AddDelayedEvent(7000, [creature, player]() -> void {
                if (creature && player)
                    creature->GetMotionMaster()->MoveFollow(player, 1.f, 2.f); 
            });
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 14:
        {
            Cast(creature, 305230);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 15:
        {
            Cast(creature, 305232);
        }
        break;
        case GOSSIP_ACTION_INFO_DEF + 16:
        {
            Cast(creature, 305285);
        }
        break;
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

// 60968
class npc_monk_art_xuen_1 : public CreatureScript
{
public:
    npc_monk_art_xuen_1() : CreatureScript("npc_monk_art_xuen_1") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)  override
    {
        player->PlayerTalkClass->ClearMenus();
        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 47723);
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};



// 542007
struct Location
{
    uint32 mapId;
    float x;
    float y;
    float z;
    float o;
};

struct TelepData
{
    uint32 action;
    uint32 subAction;
    uint32 nameStringId;
    Location loc;
    uint32 team;
    bool isEmpty;
    bool isPrev;
};

#define LOCATIONS_COUNT 25

TelepData teleData[] =
{
    { 1251, 0, karazhan,            {0, -11125.72f, -2022.30f, 47.17f, 1.06f },         0,  false, false },
    { 1252, 0, vaultofthewardens,   { 1220, -1818.19f, 6667.63f, 146.70f, 5.92f },      0,  false, false },
    { 1253, 0, eyeofazhara,         { 1220, -0.55f, 5780.14f, 3.82f, 1.44f },           0,  false, false },
    { 1254, 0, suramarcatacombs,    { 1220, 1167.06f, 4296.69f, 12.62f, 0.95f },        0,  false, false },
    { 1255, 0, courtofstars,        { 1220, 1013.71f, 3863.18f, 7.82f, 5.15f },         0,  false, false },
    { 1256, 0, mawofsouls,          { 1220, 3377.03f, 1978.07f, 4.78f, 6.f },           0,  false, false },
    { 1257, 0, trialofvalor,        { 1220, 2376.55f, 848.29f, 252.9f, 5.3f },          0,  false, false },
    { 1258, 0, neltarionlair,       { 1220, 3739.05f, 4160.01f, 887.14f, 2.14f },       0,  false, false },
    { 1259, 0, blackrookhold,       { 1220, 3087.41f, 7477.08f, 51.55f, 2.19f },        0,  false, false },
    { 1260, 0, darkheartthicket,    { 1220, 3771.66f, 6350.10f, 183.93f, 0.90f },       0,  false, false },
    { 1261, 0, emeraldnightmare,    { 1220, 3578.87f, 6488.74f, 178.01f, 0.97f },       0,  false, false },
    { 1262, 0, tombofsargeras,      { 1220, -474.44f, 2487.98f, 106.22f, 2.56f },       0,  false, false },
    { 1263, 0, triumvirate,         { 1669, 5376.9f, 10848.14f, 11.84f, 5.58f },        0,  false, false },
    { 1264, 0, antorus,             { 1669, -3144.44f, 9395.15f, -194.17f, 2.95f },     0,  false, false },

    { 5550, 0, nextpage,            { 0, 0, 0, 0, 0 },                                  0,  true,  false },
    { 5551, 0, temp,                { 0, 0, 0, 0, 0 },                                  0,  false,  true },
    { 1265, 5550, twomoons,         { 870, 1663.69f, 929.29f, 471.24f, 0.10f },         HORDE,  false, false },
    { 1266, 5550, sevenstars,       { 870, 833.09f, 261.77f, 503.67f, 3.74f },          ALLIANCE,   false, false },
    { 1267, 5550, isleofthunder,    { 1064, 6824.91f, 5454.14f, 29.57f, 5.48f },        0,  false, false },
    { 1268, 5550, isleofqueldanas,  { 530, 12753.49f, -6921.82f, 12.23f, 3.7f },        0,  false, false },
    { 1269, 5550, shattrath,        { 530, -1853.27f, 5408.83f, -12.42f, 2.09f },       0,  false, false },
    { 1270, 5550, ashranA,          { 1116, 3678.72f, -3837.66f, 44.97f, 4.15f },       ALLIANCE,   false, false },
    { 1271, 5550, ashranH,          { 1116, 5354.12f, -3944.59f, 32.73f, 3.85f },       HORDE,  false, false },
    { 1272, 5550, dalaranN,         { 571, 5798.33f, 631.85f, 647.41f, 0.87f },         0,  false, false },
    { 1273, 5550, temp,             { 0, 0, 0, 0, 0 },                                  0,  false, true },
};

enum CheckType
{
    QUEST = 1,
    ACHIEVEMENT,
    LEVEL,
};

class npc_events_teleporter_sp : public CreatureScript
{
public:
    npc_events_teleporter_sp() : CreatureScript("npc_events_teleporter_sp") {}

    bool Check(Player *player, uint8 type, uint32 criteria)
    {
        if (type == QUEST)
        {
            if (player->GetQuestStatus(criteria) == QUEST_STATUS_REWARDED)
                return true;
            else
                return false;
        }
        if (type == LEVEL)
        {
            if (player->getLevel() < criteria)
                return false;
        }

        return true;
    }

    bool CanAddMenu(Player *player, uint16 index, uint16 submenu)
    {
        if (submenu == 0)
        {
            switch (index)
            {
            case 12:
            case 13:
            {
                if (!Check(player, QUEST, 47220))
                    return false;
                break;
            }
            }
            if (index >= 0 && index <= 13)
                if (!Check(player, LEVEL, 98))
                    return false;
        }
        if (submenu == 5550)
        {
            switch (index)
            {
            case 16:
            case 17:
            case 18:
                if (!Check(player, LEVEL, 88))
                    return false;
                break;
            case 19:
            case 20:
                if (!Check(player, LEVEL, 60))
                    return false;
                break;
            case 21:
            case 22:
                if (!Check(player, LEVEL, 98))
                    return false;
                break;
            case 23:
                if (!Check(player, LEVEL, 77))
                    return false;
                break;
            }
        }

        return true;
    }

    void AddAction(Player *player, uint16 index)
    {
        LocaleConstant locale = player->GetSession()->GetSessionDbLocaleIndex();

        std::string text = sObjectMgr->GetTrinityString(teleData[index].nameStringId, locale);

        player->ADD_GOSSIP_ITEM(5, text.c_str(), GOSSIP_SENDER_MAIN, teleData[index].action);
    }

    void ActionLoad(Player* player, uint8 subAction)
    {
        for (uint16 i = 0; i < LOCATIONS_COUNT; ++i)
        {
            TelepData _data = teleData[i];
            if (_data.subAction == subAction && (_data.team == player->GetTeam() || !_data.team))
                if (CanAddMenu(player, i, 0))
                    AddAction(player, i);
        }
    }

    bool OnGossipHello(Player* player, Creature* _Creature) override
    {
        ActionLoad(player, 0);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, _Creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* _Creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        int16 actionIndex = -1;
        for (uint16 i = 0; i < LOCATIONS_COUNT; ++i)
        {
            TelepData _data = teleData[i];
            if (_data.action == action)
                actionIndex = i;
        }

        if (actionIndex < 0)
            return false;

        if (teleData[actionIndex].isEmpty)
        {
            SendSubMenu(player, _Creature, action);
            return true;
        }
        if (teleData[actionIndex].isPrev)
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        if (!player->getAttackers()->empty())
        {
            player->CLOSE_GOSSIP_MENU();
            _Creature->Whisper(542007, player->GetGUID());
            return false;
        }

        player->CLOSE_GOSSIP_MENU();

        TelePlayerByAction(player, actionIndex);

        return true;
    }

    void TelePlayerByAction(Player *player, uint16 actionIndex)
    {
        Location loc = teleData[actionIndex].loc;
        player->TeleportTo(loc.mapId, loc.x, loc.y, loc.z, loc.z);
    }

    void SendSubMenu(Player *player, Creature *creature, uint16 submenu)
    {
        for (uint16 i = 0; i < LOCATIONS_COUNT; ++i)
        {
            TelepData _data = teleData[i];
            if (_data.subAction == submenu && (_data.team == player->GetTeam() || !_data.team))
                if (CanAddMenu(player, i, submenu))
                    AddAction(player, i);
        }
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    struct npc_events_teleporter_spAI : public ScriptedAI
    {
        npc_events_teleporter_spAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            Init();
        }

        void Init()
        {
            if (auto owner = me->GetAnyOwner())
                if (auto plr = owner->ToPlayer())
                    me->setFaction(plr->GetTeam() == HORDE ? 85 : 11);
            
            me->AddDelayedEvent(100, [this]() -> void
            {
                DoCast(me, 142156);
            });
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_events_teleporter_spAI(creature);
    }
};

















/*
        Legion Invasion: Barrens & Westfall Daily
*/

enum legInv
{
    SPELL_SPAWN_VIS     = 216240,
    SPELL_SHIP_ATTACK   = 305162,
    MAX_ADDS_A          = 4,
    MAX_ADDS_H          = 3,
};

struct npc_legion_invasion_daily_quest_initiator : public ScriptedAI
{
    npc_legion_invasion_daily_quest_initiator(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool started = false;
    uint32 timer = 5000;
    uint32 entry = 0;
    uint32 addsDied = 0;
    EventMap events;

    void AddsWave(bool justOne)
    {
        switch (me->GetEntry())
        {
        case npc_inv_init_a:
            if (!justOne)
            {
                for (uint8 i = 0; i < MAX_ADDS_A; ++i)
                {
                    if (auto adds = me->SummonCreature(RandEntry(), westfallLegionAddPos[urand(0, 42)]))
                    {
                        adds->CastSpell(adds, SPELL_SPAWN_VIS, true);
                        adds->AddDelayedEvent(2500, [adds]() -> void
                        {
                            if (!adds->isInCombat())
                                adds->GetMotionMaster()->MoveRandom(7.f);
                        });
                    }
                }
            }
            else
                if (auto adds = me->SummonCreature(RandEntry(), westfallLegionAddPos[urand(0, 42)]))
                {
                    adds->CastSpell(adds, SPELL_SPAWN_VIS, true);
                    adds->AddDelayedEvent(2500, [adds]() -> void
                    {
                        if (!adds->isInCombat())
                            adds->GetMotionMaster()->MoveRandom(7.f);
                    });
                }
            break;
        case npc_inv_init_h:
            if (!justOne)
            {
                for (uint8 i = 0; i < MAX_ADDS_H; ++i)
                {
                    if (auto adds = me->SummonCreature(RandEntry(), barrensLegionAddPos[urand(0, 33)]))
                    {
                        adds->CastSpell(adds, SPELL_SPAWN_VIS, true);
                        adds->AddDelayedEvent(2500, [adds]() -> void
                        {
                            if (!adds->isInCombat())
                                adds->GetMotionMaster()->MoveRandom(7.f);
                        });
                    }
                }
            }
            else
                if (auto adds = me->SummonCreature(RandEntry(), barrensLegionAddPos[urand(0, 33)]))
                {
                    adds->CastSpell(adds, SPELL_SPAWN_VIS, true);
                    adds->AddDelayedEvent(2500, [adds]() -> void
                    {
                        if (!adds->isInCombat())
                            adds->GetMotionMaster()->MoveRandom(7.f);
                    });
                }
            break;
        }
    }

    uint32 RandEntry() const
    {
        uint32 rand = urand(1, 100);
        if (rand < 40)
            return sAdds[urand(0, sAdds.size() - 1)];
        else if (rand > 40 && rand < 70)
            return lAdds[urand(0, lAdds.size() - 1)];
        else if (rand > 70 && rand < 100)
            return xAdds[urand(0, xAdds.size() - 1)];
        else
            return sAdds[urand(0, sAdds.size() - 1)];
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case npc_inv_add_s1:
        case npc_inv_add_s2:
        case npc_inv_add_s3:
        case npc_inv_add_l1:
        case npc_inv_add_l2:
        case npc_inv_add_l3:
        case npc_inv_add_l4:
        case npc_inv_add_x1:
        case npc_inv_add_x2:
            ++addsDied;
            DoAction(ACTION_2);
            if (addsDied >= 15)
                DoAction(ACTION_3);
            break;
        }
    }

    void ShipData()
    {
        std::list<Creature*> creaList;
        GetCreatureListWithEntryInGrid(creaList, me, npc_inv_ship_a, 250.0f);
        GetCreatureListWithEntryInGrid(creaList, me, npc_inv_ship_h, 250.0f);
        if (!creaList.empty())
        {
            for (auto const& ships : creaList)
                if (ships->IsAIEnabled)
                    ships->AI()->DoAction(ACTION_1);
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            AddsWave(false);
            ShipData();
            events.ScheduleEvent(EVENT_1, urand(3000, 5000));
            events.ScheduleEvent(EVENT_2, urand(7000, 8000));
            events.ScheduleEvent(EVENT_3, urand(10000, 12000));
            events.ScheduleEvent(EVENT_4, urand(14000, 17000));
            events.ScheduleEvent(EVENT_5, urand(20000, 23000));
            events.ScheduleEvent(EVENT_6, urand(26000, 29000));
            break;
        case ACTION_2:
            AddsWave(true);
            break;
        case ACTION_3:
            addsDied = 0;
            uint32 rand[2] = { npc_inv_commander1, npc_inv_commander2 };
            if (me->GetEntry() == npc_inv_init_a)
            {
                if (auto commander = me->SummonCreature(rand[urand(0, 1)], commandersPosWB[0]))
                    commander->CastSpell(commander, SPELL_SPAWN_VIS, true);
            }
            else
                if (auto commander = me->SummonCreature(rand[urand(0, 1)], commandersPosWB[1]))
                    commander->CastSpell(commander, SPELL_SPAWN_VIS, true);
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!sGameEventMgr->IsActiveEvent(811))
            return;

        if (timer && !started)
        {
            if (timer <= diff)
            {
                started = true;
                DoAction(ACTION_1);
                timer = 5000;
            }
            else
                timer -= diff;
        }

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
            case EVENT_2:
            case EVENT_3:
            case EVENT_4:
            case EVENT_5:
            case EVENT_6:
                AddsWave(false);
                break;
            }
        }
    }
};

// 542141 
struct npc_events_alliance_ship_attack_daily_a : public ScriptedAI
{
    npc_events_alliance_ship_attack_daily_a(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool started = false;
    uint32 timer = 5000;
    uint32 addsDied = 0;
    uint32 adds[4] = { 542146, 542147, 542148, 542149 };

    void AddsWave(bool first)
    {
        if (first)
        {
            for (uint8 i = 0; i < 6; ++i)
                me->SummonCreature(urand(adds[0], adds[3]), aliShipAirPos_A[i]);
        }
        else
        {
            uint8 rand = urand(1, 2);
            me->SummonCreature(urand(adds[0], adds[3]), rand == 1 ? aliShipAirPos_A[urand(0, 5)] : aliShipInsidePos_A[urand(0, 10)]);
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case 542146:
        case 542147:
        case 542148:
        case 542149:
            ++addsDied;
            AddsWave(false);
            summon->DespawnOrUnsummon(1000);
            if (addsDied >= 15)
            {
                DoAction(ACTION_3);
                addsDied = 0;
            }
            break;
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            AddsWave(true);
            if (auto anpc = me->FindNearestCreature(542162, 200.f, true))
                if (anpc->IsAIEnabled)
                    anpc->AI()->DoAction(ACTION_1);
            break;
        case ACTION_2:
            for (uint8 i = 0; i < 11; ++i)
                SummonCreatureDelay(urand(1000, 3000), urand(adds[0], adds[3]), aliShipInsidePos_A[i]);
            break;
        case ACTION_3:
            if (auto capt = me->SummonCreature(542159, hordeofficerPos))
                if (capt->IsAIEnabled)
                    capt->AI()->Talk(1);
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer && !started)
        {
            if (timer <= diff)
            {
                started = true;
                DoAction(ACTION_1);
                DoAction(ACTION_2);
                timer = 5000;
            }
            else
                timer -= diff;
        }
    }
};

// 542158 
struct npc_events_alliance_ship_attack_daily_h : public ScriptedAI
{
    npc_events_alliance_ship_attack_daily_h(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    bool started = false;
    uint32 timer = 5000;
    uint32 addsDied = 0;
    uint32 adds[4] = { 542154, 542155, 542156, 542157 };

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case 542154:
        case 542155:
        case 542156:
        case 542157:
            ++addsDied;
            summon->DespawnOrUnsummon(1000);
            me->SummonCreature(urand(adds[0], adds[3]), aliShipInsidePos_H[urand(0,21)]);
            if (addsDied >= 15)
            {
                DoAction(ACTION_2);
                addsDied = 0;
            }
            break;
        }
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            for (uint8 i = 0; i < 15; ++i)
                SummonCreatureDelay(urand(1000, 3500), urand(adds[0], adds[3]), aliShipInsidePos_H[i]);

            if (auto anpc = me->FindNearestCreature(542161, 200.f, true))
                if (anpc->IsAIEnabled)
                    anpc->AI()->DoAction(ACTION_1);
            break;
        case ACTION_2:
            if (auto capt = me->SummonCreature(542160, aliofficerPos))
                if (capt->IsAIEnabled)
                    capt->AI()->Talk(1);
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer && !started)
        {
            if (timer <= diff)
            {
                started = true;
                DoAction(ACTION_1);
                timer = 5000;
            }
            else
                timer -= diff;
        }
    }
};

// 542142 542143 542144 542145 542150 542151 542152 542153
struct npc_events_alliance_ship_attack_npcgeneric : public ScriptedAI
{
    npc_events_alliance_ship_attack_npcgeneric(Creature* creature) : ScriptedAI(creature) {}

    uint32 timer = 5000;

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth() && !attacker->IsPlayer())
            damage = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (!me->isInCombat())
                if (auto target = me->SelectNearestTarget(10.f))
                    if (target->IsWithinLOSInMap(me))
                        me->Attack(target, false);

            timer = 5000;
        }
        else
            timer -= diff;

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

//

enum wsg_misc
{
    hordeflag_visual    = 305214,
    alliflag_visual     = 305213,
};

struct npc_events_warsong_generic : public ScriptedAI
{
    npc_events_warsong_generic(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    uint32 timer = 2000;
    uint32 stimer = urand(5000, 30000);
    uint32 randomDisplay_a[16] = { 58467, 61371, 58690, 58502, 58457, 25807, 60075, 60070, 60487, 61502, 60543, 60409, 60041, 60519, 58682, 58786 };
    uint32 randomDisplay_h[18] = { 4515, 17878, 58488, 60449, 58677, 59972, 58259, 61503, 61508, 60036, 61519, 60558, 58687, 56527, 58456, 61353, 60486, 61346 };
    uint32 weapons[13] = { 33669, 33734, 33756, 33763, 31958, 31965, 32927, 32044, 32053, 32963, 35014, 35058, 35082 };

    void Reset() override
    {
        me->SetDisplayId(me->getFaction() == fact_alliance ? randomDisplay_a[urand(0, 15)] : randomDisplay_h[urand(0, 17)]);
        me->SetVirtualItem(0, weapons[urand(0, 12)]);
        me->SetVirtualItem(1, weapons[urand(0, 12)]);
        switch (me->GetEntry())
        {
        case 542165:
        case 542166:
            me->AddAura(me->getFaction() == fact_alliance ? hordeflag_visual : alliflag_visual, me);
            break;
        }
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth())
            damage = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (!me->isInCombat())
                if (auto target = me->SelectNearestTarget(10.f))
                    if (target->IsWithinLOSInMap(me) && !target->IsPlayer())
                        me->Attack(target, false);

            timer = 5000;
        }
        else
            timer -= diff;

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();

        if (stimer <= diff)
        {
            uint32 roll = urand(1, 100);
            if (roll <= 80)
                Talk(1);

            stimer = urand(15000, 66000);
        }
        else
            stimer -= diff;
    }
};

// 542107 542110
struct npc_legion_invasion_daily_quest_ship : public ScriptedAI
{
    npc_legion_invasion_daily_quest_ship(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            events.ScheduleEvent(EVENT_1, urand(3000, 5000));
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto target = me->SelectNearestPlayer(120.f))
                    DoCast(target, SPELL_SHIP_ATTACK);

                events.ScheduleEvent(EVENT_1, urand(5000, 7000));
                break;
            }
        }
    }
};

// 542161 542162
struct npc_events_aliship_attack_airforce : public ScriptedAI
{
    npc_events_aliship_attack_airforce(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            events.ScheduleEvent(EVENT_1, urand(3000, 5000));
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto target = me->SelectNearestPlayer(120.f))
                    DoCast(target, 305208);

                events.ScheduleEvent(EVENT_1, urand(9000, 12000));
                break;
            }
        }
    }
};

// 542171 542172
struct npc_events_warsong_vehicle : public VehicleAI
{
    npc_events_warsong_vehicle(Creature* creature) : VehicleAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        VehicleAI::Reset();
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (attacker->IsPlayer())
            damage = 0;
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatID*/, bool apply) override
    {
        if (apply)
        {
            me->SetCanFly(true);
        }
        else
        {
            if (auto plr = passenger->ToPlayer())
            {
                if (auto veh = me->GetVehicleKit())
                    if (auto pass = veh->GetPassenger(1))
                        if (auto cre = pass->ToCreature())
                            cre->DespawnOrUnsummon(500);

                me->DespawnOrUnsummon(500);
                plr->NearTeleportTo(plr->GetTeam() == ALLIANCE ? aliWSGResPos : hordeWSGResPos);
            }
        }
    }
};

// 542167 542169
struct npc_events_warsong_injured_fighter : public ScriptedAI
{
    npc_events_warsong_injured_fighter(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 timer = urand(5000, 20000);
    uint32 randomDisplay_a[16] = { 58467, 61371, 58690, 58502, 58457, 25807, 60075, 60070, 60487, 61502, 60543, 60409, 60041, 60519, 58682, 58786 };
    uint32 randomDisplay_h[18] = { 4515, 17878, 58488, 60449, 58677, 59972, 58259, 61503, 61508, 60036, 61519, 60558, 58687, 56527, 58456, 61353, 60486, 61346 };
    uint32 weapons[13] = { 33669, 33734, 33756, 33763, 31958, 31965, 32927, 32044, 32053, 32963, 35014, 35058, 35082 };
    bool hited;

    void Reset() override
    {
        if (!me->ToTempSummon())
        {
            hited = false;
            me->SetVisible(true);
            me->SetDisplayId(me->getFaction() == fact_alliance ? randomDisplay_a[urand(0, 15)] : randomDisplay_h[urand(0, 17)]);
            me->SetVirtualItem(0, weapons[urand(0, 12)]);
            me->SetVirtualItem(1, weapons[urand(0, 12)]);
            me->CastSpellDelay(me, 305215, true, 500);
        }
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
        case 305223:
            if (!hited && !me->ToTempSummon())
            {
                hited = true;
                if (auto sum = target->SummonCreature(me->GetEntry(), me->GetPosition()))
                {
                    sum->CastSpell(target, 46598, true);
                    sum->SetDisplayId(me->GetDisplayId());
                    me->SetVisible(false);
                    me->DespawnOrUnsummon(1500);
                }
            }
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->ToTempSummon())
            return;

        if (timer <= diff)
        {
            Talk(1);
            timer = urand(15000, 30000);
        }
        else
            timer -= diff;
    }
};

// 542179
struct npc_events_warsong_bonuses : public ScriptedAI
{
    npc_events_warsong_bonuses(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        events.ScheduleEvent(EVENT_1, urand(3000, 5000));
        rolled = false;
    }

    EventMap events;
    bool rolled = false;
    uint8 collect = 0;

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1)
        {
            ++collect;
            if (collect == 2)
                rolled = false;
        }
    }

    bool EventCheck()
    {
        for (uint32 events : {813, 814, 815, 816, 817, 818, 819, 820, 821})
            if (sGameEventMgr->IsActiveEvent(events))
                return true;

        return false;
    }

    void Bonus(bool justOne)
    {
        if (EventCheck())
        {
            uint32 roll = urand(1, 100);
            if (roll <= 50)
            {
                rolled = true;
                for (uint8 i = 0; i < 2; ++i)
                {
                    Position pos = WSG_bonusPos[urand(0, 9)];
                    me->CreateAreaTrigger(0, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), 75075);
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (rolled)
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                Bonus(false);
                events.RescheduleEvent(EVENT_1, urand(55000, 78000));
                break;
            }
        }
    }
};

// 542003
struct npc_events_banker_rift : public ScriptedAI
{
    npc_events_banker_rift(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        me->AddDelayedEvent(2000, [this]() -> void
        {
            if (auto sum = me->SummonCreature(me->GetEntry() == 542003 ? 542004 : 542005, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 660000))
                if (auto owner = me->GetAnyOwner())
                    if (auto player = owner->ToPlayer())
                        sum->setFaction(player->GetTeam() == ALLIANCE ? 12 : 29);
        });

        me->DespawnOrUnsummon(1500);
    }
};

// 75075
struct at_events_warsong_bonuses : AreaTriggerAI
{
    explicit at_events_warsong_bonuses(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    void OnUnitEnter(Unit* unit) override
    {
        switch (unit->GetEntry())
        {
        case 542171:
        case 542172:
            if (auto i = unit->FindNearestCreature(542179, 500.f))
                if (i->IsAIEnabled)
                    i->AI()->DoAction(ACTION_1);

            unit->CastSpell(unit, 305226, true);
            at->Despawn();
            break;
        }
    }
};

// 
class go_events_sv_chests_search : public GameObjectScript
{
public:
    go_events_sv_chests_search() : GameObjectScript("go_events_sv_chests_search") {}

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if ((used < 3) && sGameEventMgr->IsActiveEvent(822) && player->GetQuestStatus(60027) == QUEST_STATUS_INCOMPLETE)
        {
            ++used;
            player->AddItem(166531, 1);
            if (used == 1)
                sWorld->SendWorldText(542140, player->GetName());
            else if (used == 2)
                sWorld->SendWorldText(542141, player->GetName());
            else if (used == 3)
                sWorld->SendWorldText(542142, player->GetName());
        }

        return true;
    }

private:
    uint8 used = 0;
};

class player_kick_from_events : public PlayerScript
{
public:
    player_kick_from_events() : PlayerScript("player_kick_from_events") {}

    uint32 timer = 3500;
    uint32 afktimer = 0;

    void ResAndTeleDalaran(Player* player, uint32 castAtFirst)
    {
        if (!player->isAlive())
            player->ResurrectPlayer(0.5f);

        if (castAtFirst)
            player->CastSpell(player, castAtFirst, true);

        player->TeleportTo(1220, DalaranPos.m_positionX, DalaranPos.m_positionY, DalaranPos.m_positionZ, DalaranPos.m_orientation);
    }

    void OnMapChanged(Player* player)
    {
        if (auto sec = player->GetSession()->GetSecurity())
        {
            if (sec == 1 || sec == 2)
            {
                if (auto map = player->GetMap())
                    if (map->IsDungeon() || map->IsRaid() || map->isChallenge() || map->IsBattlegroundOrArena())
                        player->TeleportTo(1, 16222.12f, 16258.40f, 13.19f, 1.48f);
            }
        }
    }

    void OnUpdate(Player* player, uint32 diff) override
    {
        if (player->isGameMaster())
            return;

        if (player->IsBeingTeleported())
            return;

        if (timer <= diff)
        {
            switch (player->GetMapId())
            {
            case mapStromgard:
            {
                if (!player->HasAura(305180))
                    ResAndTeleDalaran(player, 0);

                if (player->HasAura(783))
                    player->RemoveAura(783);

                if (player->HasAura(125883))
                    player->RemoveAura(125883);

                break;
            }
            case mapWarsong:
            {
                if (!player->IsOnVehicle())
                    if (!player->FindNearestCreature(player->GetTeam() == ALLIANCE ? 542175 : 542176, 12.f))
                        player->NearTeleportTo(player->GetTeam() == ALLIANCE ? aliWSGResPos : hordeWSGResPos);

                if (!player->HasAura(305228))
                    ResAndTeleDalaran(player, 0);
                break;
            }
            }
            timer = 1000;
        }
        else
            timer -= diff;
        

        if (player->isAFK())
        {
            bool ttimer = afktimer;
            switch (player->GetMapId())
            {
            case mapStromgard:
            case mapWarsong:
                if (!ttimer)
                {
                    ChatHandler(player).PSendSysMessage("WARNING! You are in AFK mode and you has been removed from event after 2 minutes if don't remove AFK mode.");
                    afktimer = 120000;
                }
                break;
            }
            if (afktimer)
            {
                if (afktimer <= diff)
                {
                    ResAndTeleDalaran(player, event_deserter);
                }
                else
                    afktimer -= diff;
            }
        }
        else
            afktimer = 0;
    }
};

class player_events_quest_give_in_zone : public PlayerScript
{
public:
    player_events_quest_give_in_zone() : PlayerScript("player_events_quest_give_in_zone") {}

    bool HasQuest(Player* player, uint32 questID)
    {
        if (player->GetQuestStatus(questID) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(questID) == QUEST_STATUS_COMPLETE ||
            player->GetQuestStatus(questID) == QUEST_STATUS_REWARDED)
            return true;

        return false;
    }

    void QuestAdd(Player* player, uint32 questID, uint32 activeEventCheck)
    {
        if (!HasQuest(player, questID))
        {
            if (activeEventCheck)
                if (!sGameEventMgr->IsActiveEvent(activeEventCheck))
                    return;

            Quest const* quest = sQuestDataStore->GetQuestTemplate(questID);
            if (quest && player->CanAddQuest(quest, true))
                player->AddQuest(quest, NULL);
        }
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        switch (newZone)
        {
        case 5339:
        case 33:
            QuestAdd(player, 60027, 822);
            break;
        case 40:
            if (player->GetTeam() == ALLIANCE)
                QuestAdd(player, 60022, 811);
            break;
        case 17:
            if (player->GetTeam() == HORDE)
                QuestAdd(player, 60021, 811);
            break;
        case 65:
            QuestAdd(player, 60028, 823);
            break;
        case 618:
            QuestAdd(player, 60029, 824);
            break;
        }
    }

    void OnEnterCombat(Player* player, Unit* /*target*/)
    {
        if (player->GetCurrentZoneID() == 65 && sGameEventMgr->IsActiveEvent(823))
        {
            if (!player->HasAura(SPELL_ATRAY_BRR_COLD))
                player->CastSpell(player, SPELL_ATRAY_BRR_COLD, true);
        }
    }

    void OnQuestReward(Player* player, Quest const* quest)
    {
        switch (quest->Id)
        {
        case 25266:
            if (player->getRace() == RACE_GOBLIN)
                if (auto t = player->FindNearestCreature(39609, 20.f))
                    if (auto sess = player->GetSession())
                        sess->SendBindPoint(t);
            break;
        case 14434:
            if (player->getRace() == RACE_WORGEN)
                player->AddDelayedEvent(15000, [player]() -> void
                {
                    if (auto t = player->FindNearestCreature(42968, 65.f))
                        if (auto sess = player->GetSession())
                            sess->SendBindPoint(t);
                });
            break;
        }
    }

    void OnSpellLearned(Player* player, uint32 spellID)
    {
        switch (spellID)
        {
        case 305189:
            sLog->outWarden("Player %s (GUID: %u) learned Xuen Mount spellID 305189", player->GetName(), player->GetGUIDLow());
            break;
        }
    }
};


//
class player_monk_quest_43062 : public PlayerScript
{
public:
    player_monk_quest_43062() : PlayerScript("player_monk_quest_43062") {}

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/)
    {
        switch (spell->GetSpellInfo()->Id)
        {
        case 115008:
        case 109132:
        {
            if (player->GetQuestStatus(43062) == QUEST_STATUS_INCOMPLETE)
            {
                player->KilledMonsterCredit(109597);
            }
            break;
        }
        case 117952:
        {
            if (player->GetQuestStatus(43062) == QUEST_STATUS_INCOMPLETE)
            {
                player->KilledMonsterCredit(109599);
            }
            break;
        }
        case 100784:
        case 205523:
        {
            if (player->GetQuestStatus(43062) == QUEST_STATUS_INCOMPLETE)
            {
                player->KilledMonsterCredit(109598);
            }
            break;
        }
        }
    }
};

class player_remove_flight_if_not_mounted : public PlayerScript
{
public:
    player_remove_flight_if_not_mounted() : PlayerScript("player_remove_flight_if_not_mounted") {}

    uint32 timer = 2000;

    void OnLogin(Player* player) // fix some bugs with flying without mount
    {
        if (!player->IsMounted() && player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED))
            player->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
    }
};


void AddSC_custom_events()
{
    //RegisterCreatureAI(boss_temple_vonjin);
    //RegisterCreatureAI(npc_temple_vonjin_weapon);

    //RegisterCreatureAI(boss_temple_lessar);

    RegisterCreatureAI(npc_legion_invasion_daily_quest_initiator);
    RegisterCreatureAI(npc_legion_invasion_daily_quest_ship);
    RegisterCreatureAI(npc_events_aliship_attack_airforce);
    RegisterCreatureAI(npc_events_alliance_ship_attack_daily_a);
    RegisterCreatureAI(npc_events_alliance_ship_attack_daily_h);
    RegisterCreatureAI(npc_events_alliance_ship_attack_npcgeneric);
    RegisterCreatureAI(npc_events_warsong_generic);
    RegisterCreatureAI(npc_events_warsong_vehicle);
    RegisterCreatureAI(npc_events_warsong_injured_fighter);
    RegisterCreatureAI(npc_events_warsong_bonuses);
    RegisterCreatureAI(npc_events_banker_rift);


    RegisterCreatureAI(boss_new_year_2019_atray);
    RegisterCreatureAI(boss_new_year_2019_evala);
    RegisterCreatureAI(npc_new_year_2019_atray_blizzard);
    RegisterCreatureAI(npc_new_year_2019_atray_frost_minion);
    RegisterCreatureAI(npc_new_year_2019_evala_minion);
    RegisterCreatureAI(npc_new_year_2019_evala_portal);
    RegisterCreatureAI(npc_new_year_2019_void_zone);
    RegisterCreatureAI(npc_new_year_2019_evala_arctic_cloud);
    RegisterCreatureAI(npc_new_year_2019_evala_shard);
    RegisterCreatureAI(npc_new_year_2019_evala_frostmage);
    RegisterCreatureAI(npc_new_year_2019_evala_frostwarr);
    RegisterCreatureAI(npc_new_year_2019_evala_tomb);
    RegisterCreatureAI(npc_new_year_2019_evala_guard);


    //RegisterCreatureAI(boss_hex_lord_hadorn);
    //RegisterCreatureAI(npc_hexlord_hadorn_poison_ring);
    //RegisterCreatureAI(npc_hexlord_hadorn_fury_of_temple);
    //RegisterCreatureAI(npc_event_summon_points_portal_initiator);
    //RegisterCreatureAI(npc_event_spirit_chains);
    //RegisterCreatureAI(npc_hexlord_hadorn_totem);
    //RegisterCreatureAI(npc_hexlord_hadorn_sphere);
    //RegisterCreatureAI(npc_hexlord_hadorn_add);
    //RegisterCreatureAI(npc_events_temple_summon_ritual_s);
    //RegisterCreatureAI(npc_events_temple_summon_ritual_t);

    //RegisterAuraScript(spell_events_transfer_temple_bottom);
    //RegisterAuraScript(spell_events_vonjin_bladestorm);
    //RegisterAuraScript(spell_events_spirit_chains);
    //RegisterAuraScript(spell_events_hadorn_explode_debuff);
    RegisterAuraScript(spell_events_warsong_speedbonus);
    RegisterAuraScript(spell_events_tabard_arcane_magic);
    RegisterAuraScript(spell_new_year_atray_explode_frost_debuff);
    RegisterAuraScript(spell_new_year_atray_cold);
    RegisterAuraScript(spell_new_year_atray_cold_stacking);
    RegisterAuraScript(spell_new_year_atray_fury);
    RegisterAuraScript(spell_new_year_evala_auras_305266);
    RegisterAuraScript(spell_new_year_evala_auras_305279);
    RegisterAuraScript(spell_new_year_evala_auras_305260);
    RegisterAuraScript(spell_new_year_evala_auras_305262);
    //RegisterSpellScript(spell_events_temple_aoe_bottom_top_filter);
    //RegisterSpellScript(spell_events_hadorn_cry_of_the_damned);
    RegisterSpellScript(spell_events_aoe_trap_stromgard);
    RegisterSpellScript(spell_events_warsong_remove_passenger);
    RegisterSpellScript(spell_events_warsong_take_passenger_check);
    RegisterSpellScript(spell_events_scroll_of_summon);
    RegisterSpellScript(spell_new_year_atray_barrage_hit);
    RegisterSpellScript(spell_new_year_evala_neg_check);
    RegisterSpellScript(spell_new_year_evala_pos_check);
    RegisterSpellScript(spell_new_year_evala_tomb_ts);

    RegisterAreaTriggerAI(at_events_warsong_bonuses);

    //new npc_events_temple_summon_ritual_f();
    //new npc_events_temple_summon_ritual_d();
    new npc_events_portal_master_portal();
    new npc_events_portal_master_portal_exit();
    new npc_events_warsong_vehicle_take();

    new npc_server_bonuses();
    new npc_server_bonuses_table();
    new npc_server_bonuses_preview();
    new npc_events_teleporter_sp();
    new npc_monk_art_xuen_1();

    new go_events_sv_chests_search();

    new player_kick_from_events();
    new player_events_quest_give_in_zone();
    new player_monk_quest_43062();
    new player_remove_flight_if_not_mounted();

    new OutdoorPvP_AB_winter_event();
    new OutdoorPvP_AB_winter_event_ev();
};
