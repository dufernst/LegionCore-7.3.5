/*
    Dungeon : The Everbloom 100
    Encounter: Yalnu
*/

#include "the_everbloom.h"

enum Says
{
    SAY_WARN_GENESIS    = 0,
    SAY_LADY_AGGRO      = 1
};

enum Spells
{
    SPELL_COLOSSAL_BLOW         = 169756,
    SPELL_COLOSSAL_BLOW_DMG     = 169179,

    SPELL_ENTANGLEMENT          = 169251,
    SPELL_ENTANGLEMENT_SELECT   = 169247,
    SPELL_ENTANGLEMENT_SUM      = 169237,
    SPELL_ENTANGLEMENT_VISUAL   = 169192,
    SPELL_ENTANGLEMENT_STUN     = 169240,

    SPELL_ENTANGLEMENT_PLR      = 170124,
    SPELL_ENTANGLEMENT_SELECT_P = 170126,
    SPELL_ENTANGLEMENT_SUM_P    = 170127,
    SPELL_ENTANGLEMENT_STUN_P   = 170132,

    SPELL_DISARM                = 169320,
    SPELL_GERMINATE_ARBORBLADE  = 169265,

    SPELL_FONT_OF_LIFE          = 169120,
    SPELL_FONT_OF_LIFE_VISUAL   = 169123,

    SPELL_GENESIS               = 169125,
    SPELL_GENESIS_CH            = 169613,
    SPELL_GENESIS_SUBMERGED     = 169126,
    SPELL_GENESIS_VISUAL        = 173539,
    SPELL_GENESIS_AT            = 173537,
    SPELL_SUBMERGED             = 175123,

    //Feral Lasher
    SPELL_LASHER_VENOM          = 173563,

    //Mages
    SPELL_FIREBALL              = 168666,
    SPELL_FLAMESTRIKE           = 169094,

    SPELL_FROSTBOLT             = 170028,
    SPELL_ICE_COMET             = 170032,

    SPELL_ARCANE_BLAST          = 170035,
    SPELL_ARCANE_ORB            = 170040
};

enum eEvents
{
    EVENT_C_BLOW_1          = 1,
    EVENT_C_BLOW_2          = 2,
    EVENT_C_BLOW_3          = 3,
    EVENT_ENTANGLEMENT      = 4,
    EVENT_ENTANGLEMENT_PLR  = 5,
    EVENT_FONT_OF_LIFE      = 6,
    EVENT_GENESIS           = 7
};

enum MageClass
{
    MAGE_FIRE    = 1,
    MAGE_FROST   = 2,
    MAGE_ARCANE  = 3,
};

Position const magePos[7] =
{
    {947.38f, -1235.11f, 181.25f, 0.68f},
    {966.46f, -1214.60f, 181.25f, 4.11f},
    {960.97f, -1208.64f, 181.24f, 4.22f},
    {970.09f, -1236.66f, 181.25f, 2.47f},
    {959.63f, -1238.97f, 181.25f, 1.62f},
    {942.67f, -1212.62f, 181.35f, 5.44f},
    {976.43f, -1223.22f, 181.33f, 3.08f}
};

struct boss_yalnu : public BossAI
{
    explicit boss_yalnu(Creature* creature) : BossAI(creature, DATA_YALNU) {}

    uint16 checkTimer;

    void Reset() override
    {
        events.Reset();
        _Reset();

        summons.DespawnAll();
        me->SetReactState(REACT_AGGRESSIVE);
        checkTimer = 2000;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();

        for (uint8 i = 0; i < 6; i++)
            me->SummonCreature(NPC_KIRIN_TOR_MAGE, magePos[i]);

        me->SummonCreature(NPC_LADY_BAIHU, magePos[6]);

        events.RescheduleEvent(EVENT_C_BLOW_1, 5000);
        events.RescheduleEvent(EVENT_ENTANGLEMENT, 10000);

        if (GetDifficultyID() != DIFFICULTY_NORMAL)
            events.RescheduleEvent(EVENT_ENTANGLEMENT_PLR, 50000);

        events.RescheduleEvent(EVENT_FONT_OF_LIFE, 12000);
        events.RescheduleEvent(EVENT_GENESIS, 30000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (attacker->ToCreature() && !attacker->ToCreature()->isPet())
            damage /= 2;
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
        case SPELL_ENTANGLEMENT_SELECT:
            DoCast(me, SPELL_DISARM, true);
            DoCast(SPELL_GERMINATE_ARBORBLADE);
            break;
        case SPELL_ENTANGLEMENT_SELECT_P:
            target->CastSpell(target, SPELL_ENTANGLEMENT_SUM_P, true);
            DoCast(me, SPELL_DISARM, true);
            DoCast(SPELL_GERMINATE_ARBORBLADE);
            break;
        }
    }

    void CheckPlrInBattleZone()
    {
        bool find = false;

        const Map::PlayerList &PlayerList = me->GetMap()->GetPlayers();

        if (!PlayerList.isEmpty())
        {
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* plr = i->getSource())
                {
                    if (!plr->isAlive() || plr->isGameMaster())
                        break;

                    if (plr->GetCurrentAreaID() == 7330)
                        find = true;
                }

                if (find && !me->isInCombat())
                    DoZoneInCombat();
                else if (!find && me->isInCombat())
                    EnterEvadeMode();
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (checkTimer <= diff)
        {
            checkTimer = 2000;
            CheckPlrInBattleZone();
        }
        else
            checkTimer -= diff;

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_C_BLOW_1:
                me->StopAttack();
                DoCast(SPELL_COLOSSAL_BLOW);
                events.RescheduleEvent(EVENT_C_BLOW_1, 20000);
                events.RescheduleEvent(EVENT_C_BLOW_2, 100);
                break;
            case EVENT_C_BLOW_2:
                DoCast(SPELL_COLOSSAL_BLOW_DMG);

                if (auto blow = me->FindNearestCreature(NPC_COLOSSAL_BLOW, 100.0f))
                    me->SetFacingToObject(blow);

                events.RescheduleEvent(EVENT_C_BLOW_3, 100);
                break;
            case EVENT_C_BLOW_3:
                me->SetReactState(REACT_AGGRESSIVE);
                break;
            case EVENT_ENTANGLEMENT:
            {
                std::list<Creature*> mageList;
                me->GetCreatureListWithEntryInGrid(mageList, NPC_KIRIN_TOR_MAGE, 100.0f);
                if (!mageList.empty())
                {
                    Trinity::Containers::RandomResizeList(mageList, 1);

                    if (auto target = mageList.front())
                        DoCast(target, SPELL_ENTANGLEMENT);
                }
                events.RescheduleEvent(EVENT_ENTANGLEMENT, 60000);
                break;
            }
            case EVENT_ENTANGLEMENT_PLR:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_ENTANGLEMENT_PLR, false);

                events.RescheduleEvent(EVENT_ENTANGLEMENT_PLR, 60000);
                break;
            case EVENT_FONT_OF_LIFE:
                DoCast(SPELL_FONT_OF_LIFE);
                events.RescheduleEvent(EVENT_FONT_OF_LIFE, 15000);
                break;
            case EVENT_GENESIS:
                for (uint8 i = 0; i < 22; i++)
                {
                    float x = frand(930, 983);
                    float y = frand(-1267, -1201);
                    me->CastSpell(x, y, me->GetPositionZ() + 2, SPELL_GENESIS, true);
                }
                DoCast(SPELL_GENESIS_CH);
                Talk(SAY_WARN_GENESIS);
                events.RescheduleEvent(EVENT_GENESIS, 60000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//84329, 84358
struct npc_yalnu_kirin_tor_mage : public ScriptedAI
{
    explicit npc_yalnu_kirin_tor_mage(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        SetCombatMovement(false);
    }

    EventMap events;
    SummonList summons;
    uint8 classMage;

    void Reset() override
    {
        summons.DespawnAll();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (me->GetEntry() == NPC_LADY_BAIHU)
            Talk(SAY_LADY_AGGRO);

        switch (classMage)
        {
        case MAGE_FIRE:
            events.RescheduleEvent(EVENT_1, 0); //45:47
            events.RescheduleEvent(EVENT_2, urand(18, 35) * IN_MILLISECONDS);
            break;
        case MAGE_FROST:
            events.RescheduleEvent(EVENT_3, 0); //45:47
            events.RescheduleEvent(EVENT_4, urand(18, 30) * IN_MILLISECONDS);
            break;
        case MAGE_ARCANE:
            events.RescheduleEvent(EVENT_5, 0); //45:47
            events.RescheduleEvent(EVENT_6, urand(18, 35) * IN_MILLISECONDS);
            break;
        }
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCast(142193); //Visual Teleport
        classMage = urand(MAGE_FIRE, MAGE_ARCANE);
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_ENTANGLEMENT_SELECT)
            DoCast(me, SPELL_ENTANGLEMENT_SUM, true);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() == NPC_ENTANGLEMENT)
        {
            summon->SetReactState(REACT_PASSIVE);
            summon->CastSpell(summon, SPELL_ENTANGLEMENT_VISUAL, true);
            summon->CastSpell(me, SPELL_ENTANGLEMENT_STUN, true);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        summons.DespawnAll();
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
            case EVENT_1: //Fireball
                DoCastVictim(SPELL_FIREBALL);
                events.RescheduleEvent(EVENT_1, 3000);
                break;
            case EVENT_2: //Flamestrike
                DoCast(SPELL_FLAMESTRIKE);
                events.RescheduleEvent(EVENT_2, urand(18, 35) * IN_MILLISECONDS);
                break;
            case EVENT_3: //Frostball
                DoCastVictim(SPELL_FROSTBOLT);
                events.RescheduleEvent(EVENT_3, 3000);
                break;
            case EVENT_4: //Ice Comet
                DoCastVictim(SPELL_ICE_COMET);
                events.RescheduleEvent(EVENT_4, urand(18, 30) * IN_MILLISECONDS);
                break;
            case EVENT_5: //Arcane Blast
                DoCastVictim(SPELL_ARCANE_BLAST);
                events.RescheduleEvent(EVENT_5, 3000);
                break;
            case EVENT_6: //Arcane Orb
                DoCast(SPELL_ARCANE_ORB);
                events.RescheduleEvent(EVENT_6, urand(18, 35) * IN_MILLISECONDS);
                break;
            }
        }
    }
};

//85194, 85107
struct npc_yalnu_eventer : public ScriptedAI
{
    explicit npc_yalnu_eventer(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void IsSummonedBy(Unit* summoner) override
    {
        switch (me->GetEntry())
        {
        case NPC_ENTANGLEMENT_PLR:
            DoCast(me, SPELL_ENTANGLEMENT_VISUAL, true);
            DoCast(summoner, SPELL_ENTANGLEMENT_STUN_P, true);
            break;
        case NPC_FONT_LIFE_STALKER:
            DoCast(me, SPELL_FONT_OF_LIFE_VISUAL, true);
            events.RescheduleEvent(EVENT_1 + urand(0, 2), 1000);
            me->DespawnOrUnsummon(3000);
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
                me->SummonCreature(NPC_VICIOUS_MANDRAGORA, me->GetPositionX() + 5, me->GetPositionY() - 5, me->GetPositionZ() + 2);
                me->SummonCreature(NPC_VICIOUS_MANDRAGORA, me->GetPositionX() - 5, me->GetPositionY() + 5, me->GetPositionZ() + 2);
                break;
            case EVENT_2:
                me->SummonCreature(NPC_GNARLED_ANCIENT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                break;
            case EVENT_3:
                for (uint8 i = 0; i < 6; i++)
                    me->SummonCreature(NPC_SWIFT_SPROUTLING, me->GetPositionX() + i, me->GetPositionY() - i, me->GetPositionZ() + 2);
                break;
            }
        }
    }
};

//86684
struct npc_yalnu_feral_lasher : public ScriptedAI
{
    explicit npc_yalnu_feral_lasher(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        activated = false;
    }

    EventMap events;
    bool activated;

    void Reset() override
    {
        events.Reset();
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCast(me, SPELL_SUBMERGED, true);
        DoCast(me, SPELL_GENESIS_AT, true);
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
        case SPELL_GENESIS_SUBMERGED:
            me->RemoveAurasDueToSpell(SPELL_SUBMERGED);
            me->SetReactState(REACT_AGGRESSIVE);
            DoZoneInCombat(me, 100.0f);
            events.RescheduleEvent(EVENT_1, 2000);
            break;
        case SPELL_GENESIS_VISUAL:
            if (!activated)
            {
                activated = true;
                me->DespawnOrUnsummon(500);
            }
            break;
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
                DoCast(SPELL_LASHER_VENOM);
                events.RescheduleEvent(EVENT_1, 3000 * urand(1, 3));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_yalnu()
{
    RegisterCreatureAI(boss_yalnu);
    RegisterCreatureAI(npc_yalnu_kirin_tor_mage);
    RegisterCreatureAI(npc_yalnu_eventer);
    RegisterCreatureAI(npc_yalnu_feral_lasher);
}
