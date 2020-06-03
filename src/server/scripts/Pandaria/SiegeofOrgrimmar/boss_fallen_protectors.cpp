/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureTextMgr.h"
#include "siege_of_orgrimmar.h"

enum eSpells
{
    SPELL_BOUND_OF_GOLDEN_LOTUS             = 143497, //Bond of the Golden Lotus
    SPELL_EJECT_ALL_PASSANGERS              = 68576,
    SPELL_DESPAWN_AT                        = 138175,

    //Rook
    SPELL_VENGEFUL_STRIKE                   = 144396, //Vengeful Strikes
    SPELL_CORRUPTED_BREW_BASE               = 143019, //Corrupted Brew

    SPELL_CORRUPTED_BREW                    = 143021,
    SPELL_CORRUPTED_BREW2                   = 145608,
    SPELL_CORRUPTED_BREW3                   = 145609,
    SPELL_CORRUPTED_BREW4                   = 145610,
    SPELL_CORRUPTED_BREW5                   = 145611,
    SPELL_CORRUPTED_BREW6                   = 145612,
    SPELL_CORRUPTED_BREW7                   = 145615,
    SPELL_CORRUPTED_BREW8                   = 145617,

    SPELL_CLASH                             = 143027, //Clash   cast 143028
    SPELL_CORRUPTION_KICK                   = 143007, //Corruption Kick
    SPELL_MISERY_SORROW_GLOOM               = 143955, //Misery, Sorrow, and Gloom

    //He
    SPELL_SHADOWSTEP                        = 143048,
    SPELL_GARROTE                           = 143198, //Garrote
    SPELL_GOUGE                             = 143301, //Gouge
    SPELL_NOXIOUS_POISON                    = 143225, //Noxious Poison
    SPELL_NOXIOUS_POISON_AURA               = 143239,
    SPELL_INSTANT_POISON                    = 143210, //Instant Poison
    SPELL_MARK_OF_ANGUISH_MEDITATION        = 143812, //Mark of Anguish

    //Sun
    SPELL_SHA_SEAR                          = 143423, //Sha Sear
    SPELL_SHADOW_WORD_BANE                  = 143434, //Shadow Word: Bane
    SPELL_CALAMITY                          = 143491, //Calamity
    SPELL_CALAMITY_DMG                      = 143493,
    SPELL_DARK_MEDITATION                   = 143546,
    SPELL_DARK_MEDITATION_JUMP              = 143730, //Prock after jump 143546
    SPELL_DARK_MEDITATION_SHARE_HEALTH_P    = 143745, //
    SPELL_DARK_MEDITATION_SHARE_HEALTH      = 143723,
    //HM
    SPELL_MEDITATION_SPIKE                  = 143599,

    SPELL_CLEAR_ALL_DEBUFS                  = 34098,  //ClearAllDebuffs
    SPELL_SHA_CORRUPTION                    = 143708, // begore 34098 and 17683
    SPELL_FULL_HEALTH                       = 17683,

    //meashures of sun
    SPELL_MANIFEST_DESPERATION              = 144504, //Manifest Desperation of 71482
    SPELL_MANIFEST_DESPAIR                  = 143746, //Manifest Despair of 71474
    SPELL_SHA_CORRUPTION_OF_SUN             = 142891,

    //meashures of he
    SPELL_SHA_CORRUPTION_SUMMONED           = 142885,
    SPELL_MARK_OF_ANGUISH_JUMP              = 143808, //Mark of Anguish
    SPELL_MARK_OF_ANGUISH_SELECT_TARGET     = 143822, //Mark of Anguish by 143840
    SPELL_MARK_OF_ANGUISH_STUN              = 143840,
    SPELL_MARK_OF_ANGUISH_DAMAGE            = 144365,
    SPELL_MARK_OF_ANGUISH_GIVE_A_FRIEND     = 143842,
    SPELL_SHADOW_WEAKNES_PROC               = 144079, //prock spell on hit or something else
    SPELL_DEBILITATION                      = 147383, //Debilitation
    SPELL_SHADOW_WEAKNESS                   = 144176, //charges targetGUID: Full: 0x70000000695B52A Type: Player Low: 110474538 
    SPELL_SHADOW_WEAKNES_MASS               = 144081,

    //measures of rook
    SPELL_SHA_CORRUPTION_MIS_OF_ROOK        = 142892,
    SPELL_SHA_CORRUPTION_GLOOM_OF_ROOK      = 142889,
    SPELL_SHA_CORRUPTION_SOR_OF_ROOK        = 142893,
    SPELL_MISERY_SORROW_GLOOM_SUMON         = 143948,
    SPELL_DEFILED_GROUND                    = 143961, //Defiled Ground apply 143959
    SPELL_DEFILE_GROUND_PROC                = 143959,
    SPELL_INFERNO_STRIKE                    = 143962, //Inferno Strike
    SPELL_CORRUPTION_SHOCK                  = 143958, //Corruption Shock

    SPELL_BERSERK                            = 26662,
    SPELL_BERSERK_TR_EF                      = 64112,
};

enum Phase
{
    PHASE_NULL                      = 0,
    PHASE_BATTLE                    = 1,
    PHASE_BATTLE_TWO                = 2,
    PHASE_BATTLE_THREE              = 3,
    PHASE_DESPERATE_MEASURES        = 4,
    PHASE_DESPERATE_MEASURES2       = 5,
    PHASE_BOND_GOLDEN_LOTUS         = 6,
};

enum PhaseEvents
{
    EVENT_VENGEFUL_STRIKE           = 3,
    EVENT_CORRUPTED_BREW            = 4,
    EVENT_CLASH                     = 5,
    EVENT_GARROTE                   = 6,
    EVENT_GOUGE                     = 7,
    EVENT_POISON_NOXIOUS            = 8,
    EVENT_POISON_INSTANT            = 9,
    EVENT_SHA_SEAR                  = 10,
    EVENT_SHADOW_WORD_BANE          = 11,
    EVENT_CALAMITY                  = 12,
    EVENT_ACTIVE                    = 13,
    EVENT_MEDITATION_SPIKE          = 14,
};

enum Actions
{
    ACTION_DESPERATE_MEASURES      = 1,
    ACTION_BOND_GOLDEN_LOTUS       = 2,
    ACTION_BOND_GOLDEN_LOTUS_END   = 3,
    ACTION_RESET_EVENTS            = 4,
    ACTION_START_EVENTS            = 5,
};

enum data
{
    BATTLE_AREA                    = 6798,
    DATA_SHADOW_WORD_DAMAGE        = 1,
    DATA_SHADOW_WORD_REMOVED       = 2,
    DATA_CALAMITY_HIT              = 3,
};

uint32 corruptedbrew[8] =
{
    SPELL_CORRUPTED_BREW,
    SPELL_CORRUPTED_BREW2,
    SPELL_CORRUPTED_BREW3,
    SPELL_CORRUPTED_BREW4,
    SPELL_CORRUPTED_BREW5,
    SPELL_CORRUPTED_BREW6,
    SPELL_CORRUPTED_BREW7,
    SPELL_CORRUPTED_BREW8,
};


uint32 const protectors[3] =
{
    NPC_ROOK_STONETOE,
    NPC_SUN_TENDERHEART,
    NPC_HE_SOFTFOOT,
};

uint32 const rookmeasure[3] =
{
    NPC_EMBODIED_MISERY_OF_ROOK,
    NPC_EMBODIED_GLOOM_OF_ROOK,
    NPC_EMBODIED_SORROW_OF_ROOK,
};

uint32 const sunmeasure[2] =
{
    NPC_EMBODIED_DESPERATION_OF_SUN,
    NPC_EMBODIED_DESPIRE_OF_SUN ,
};

Position rookmeasurepos[3] =
{
    {1196.35f, 1013.02f, 418.0625f, 0.7143f},
    {1211.83f, 1039.07f, 417.9586f, 4.7827f},
    {1230.79f, 1015.51f, 418.0644f, 2.5954f},
};

Position sunmeasurepos[2] =
{
    {1212.45f, 1057.11f, 417.5685f, 4.7112f},
    {1213.37f, 1005.80f, 418.0643f, 1.6277f},
};

struct boss_fallen_protectors : public ScriptedAI
{
    boss_fallen_protectors(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void CallOtherProtectors()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (me->GetEntry() != prot->GetEntry())
                    if (prot->isAlive() && !prot->isInCombat())
                        DoZoneInCombat(prot, 150.0f);

        if (instance->GetBossState(DATA_F_PROTECTORS) != IN_PROGRESS)
            instance->SetBossState(DATA_F_PROTECTORS, IN_PROGRESS);
    }

    void ResetProtectors()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (me->GetEntry() != prot->GetEntry())
                    if (prot->isAlive() && prot->isInCombat())
                        prot->AI()->EnterEvadeMode();

        if (instance->GetBossState(DATA_F_PROTECTORS) != NOT_STARTED)
            instance->SetBossState(DATA_F_PROTECTORS, NOT_STARTED);
    }

    void SendDone()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (me->GetEntry() != prot->GetEntry())
                    if (prot->isAlive())
                        prot->Kill(prot, true);

        if (instance->GetBossState(DATA_F_PROTECTORS) != DONE)
        {
            instance->SetBossState(DATA_F_PROTECTORS, DONE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MARK_OF_ANGUISH_STUN);
        }
    }

    bool CheckLotus()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (me->GetEntry() != prot->GetEntry())
                    if (prot->GetHealth() != 1)
                        return false;
        return true;
    }

    void ResetEvents()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (me->GetEntry() != prot->GetEntry())
                    if (prot->isAlive())
                        prot->AI()->DoAction(ACTION_RESET_EVENTS);
    }

    void StartEvents()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (me->GetEntry() != prot->GetEntry())
                    if (prot->isAlive() && !prot->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                        prot->AI()->DoAction(ACTION_START_EVENTS);
    }

    void LaunchBerserk()
    {
        for (int32 i = 0; i < 3; i++)
            if (Creature* prot = me->GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (prot->isAlive() && !prot->HasAura(SPELL_BERSERK))
                    prot->CastSpell(prot, SPELL_BERSERK, true);
    }
};

//Rook Stonetoe
class boss_rook_stonetoe : public CreatureScript
{
public:
    boss_rook_stonetoe() : CreatureScript("boss_rook_stonetoe") {}

    struct boss_rook_stonetoeAI : public boss_fallen_protectors
    {
        boss_rook_stonetoeAI(Creature* creature) : boss_fallen_protectors(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        SummonList summon;
        EventMap events;
        Phase phase;
        uint8 corruptedbrewcount;
        uint32 berserk;

        void Reset()
        {
            summon.DespawnAll();
            events.Reset();
            ResetProtectors();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_DEFENSIVE);
            corruptedbrewcount = 0;
            phase = PHASE_NULL;
            berserk = 0;
            DoCast(me, SPELL_DESPAWN_AT, true);
            me->RemoveAurasDueToSpell(SPELL_BERSERK);
            me->RemoveAurasDueToSpell(SPELL_BERSERK_TR_EF);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 150.0f);
            CallOtherProtectors();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, me->GetGUID());
            phase = PHASE_BATTLE;
            events.RescheduleEvent(EVENT_VENGEFUL_STRIKE, 7000);
            events.RescheduleEvent(EVENT_CORRUPTED_BREW, 5000);
            events.RescheduleEvent(EVENT_CLASH, 27000);
            berserk = me->GetMap()->IsHeroic() ? 6000000 : 900000;
        }

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (phase == PHASE_BATTLE && HealthBelowPct(66))
            {
                damage = 0;
                phase = PHASE_BATTLE_TWO;
                DoAction(ACTION_DESPERATE_MEASURES);
            }
            else if (phase == PHASE_BATTLE_TWO && HealthBelowPct(33))
            {
                damage = 0;
                phase = PHASE_BATTLE_THREE;
                DoAction(ACTION_DESPERATE_MEASURES);
            }
            else if (damage >= me->GetHealth() && phase == PHASE_BATTLE_THREE)
            {
                damage = 0;
                phase = PHASE_BOND_GOLDEN_LOTUS;
                me->SetHealth(1);
                DoAction(ACTION_BOND_GOLDEN_LOTUS);
            }
            else if (damage >= me->GetHealth() && phase == PHASE_BOND_GOLDEN_LOTUS && !CheckLotus())
                damage = 0;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_DESPERATE_MEASURES:
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                me->StopAttack();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                corruptedbrewcount = 0;
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4, me->GetGUID());
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5, me->GetGUID());
                DoCast(me, SPELL_MISERY_SORROW_GLOOM);
                float x, y;
                GetPosInRadiusWithRandomOrientation(me, 15.0f, x, y);
                for (uint32 n = 0; n < 3; n++)
                    if (Creature* measure = me->SummonCreature(rookmeasure[n], rookmeasurepos[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                        measure->AI()->DoZoneInCombat(measure, 150.0f);
                if (!me->GetMap()->IsHeroic())
                    ResetEvents();
                break;
            case ACTION_BOND_GOLDEN_LOTUS:
                events.Reset();
                me->StopAttack();
                me->InterruptNonMeleeSpells(true);
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_6, me->GetGUID());
                DoCast(me, SPELL_BOUND_OF_GOLDEN_LOTUS);
                break;
            case ACTION_END_DESPERATE_MEASURES:
                me->RemoveAurasDueToSpell(SPELL_MISERY_SORROW_GLOOM);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveAllAreaObjects();
                me->SetReactState(REACT_AGGRESSIVE);
                StartEvents();
                events.RescheduleEvent(EVENT_VENGEFUL_STRIKE, urand(10000, 20000));
                events.RescheduleEvent(EVENT_CORRUPTED_BREW, urand(2000, 5000));
                events.RescheduleEvent(EVENT_CLASH, urand(20000, 30000));
                break;
            case ACTION_BOND_GOLDEN_LOTUS_END:
                me->RemoveAurasDueToSpell(SPELL_BOUND_OF_GOLDEN_LOTUS);
                me->SetReactState(REACT_AGGRESSIVE);
                phase = PHASE_BATTLE_THREE;
                events.RescheduleEvent(EVENT_VENGEFUL_STRIKE, urand(10000, 20000));
                events.RescheduleEvent(EVENT_CORRUPTED_BREW, urand(2000, 5000));
                events.RescheduleEvent(EVENT_CLASH, urand(20000, 30000));
                break;
            case ACTION_RESET_EVENTS:
                events.CancelEvent(EVENT_CLASH);
                break;
            case ACTION_START_EVENTS:
                events.RescheduleEvent(EVENT_CLASH, urand(20000, 30000));
                break;
            }
        }

        void JustDied(Unit* killer)
        {
            if (killer != me)
                SendDone();
            summon.DespawnAll();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (berserk)
            {
                if (berserk <= diff)
                {
                    berserk = 0;
                    LaunchBerserk();
                }
                else
                    berserk -= diff;
            }

            EnterEvadeIfOutOfCombatArea(diff);
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_VENGEFUL_STRIKE:
                    DoCastVictim(SPELL_VENGEFUL_STRIKE);
                    events.RescheduleEvent(EVENT_VENGEFUL_STRIKE, 33000);
                    break;
                case EVENT_CORRUPTED_BREW:
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0.0f, true))
                        DoCast(target, SPELL_CORRUPTED_BREW_BASE);
                    events.RescheduleEvent(EVENT_CORRUPTED_BREW, 13000);
                    break;
                case EVENT_CLASH:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_CLASH);
                    events.RescheduleEvent(EVENT_CLASH, 40000);
                    break;
                }
            }
            if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) && phase != PHASE_BOND_GOLDEN_LOTUS)
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_rook_stonetoeAI(creature);
    }
};

//He Softfoot
class boss_he_softfoot : public CreatureScript
{
public:
    boss_he_softfoot() : CreatureScript("boss_he_softfoot") {}

    struct boss_he_softfootAI : public boss_fallen_protectors
    {
        boss_he_softfootAI(Creature* creature) : boss_fallen_protectors(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;
        EventMap events;
        Phase phase;

        void Reset()
        {
            summons.DespawnAll();
            events.Reset();
            ResetProtectors();
            RemoveDebuffs();
            DespawnAllAT();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_DEFENSIVE);
            phase = PHASE_NULL;
            me->RemoveAurasDueToSpell(SPELL_BERSERK);
            me->RemoveAurasDueToSpell(SPELL_BERSERK_TR_EF);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GARROTE);
        }

        void RemoveDebuffs()
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GARROTE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NOXIOUS_POISON_AURA);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WEAKNESS);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DEBILITATION);
        }

        void DespawnAllAT()
        {
            std::list<AreaTrigger*> atlist;
            atlist.clear();
            me->GetAreaTriggersWithEntryInRange(atlist, 1013, me->GetGUID(), 100.0f);
            if (!atlist.empty())
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                    (*itr)->RemoveFromWorld();
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 150.0f);
            CallOtherProtectors();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            DespawnAllAT();
            DoCast(who, SPELL_INSTANT_POISON);
            phase = PHASE_BATTLE;
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WEAKNESS);
            events.RescheduleEvent(EVENT_GARROTE, 5000);
            events.RescheduleEvent(EVENT_GOUGE, urand(2000, 5000));
            events.RescheduleEvent(EVENT_POISON_NOXIOUS, 21000);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (phase == PHASE_BATTLE && HealthBelowPct(66))
            {
                damage = 0;
                phase = PHASE_BATTLE_TWO;
                DoAction(ACTION_DESPERATE_MEASURES);
            }
            else if (phase == PHASE_BATTLE_TWO && HealthBelowPct(33))
            {
                damage = 0;
                phase = PHASE_BATTLE_THREE;
                DoAction(ACTION_DESPERATE_MEASURES);
            }
            else if (damage >= me->GetHealth() && phase == PHASE_BATTLE_THREE)
            {
                damage = 0;
                phase = PHASE_BOND_GOLDEN_LOTUS;
                me->SetHealth(1);
                DoAction(ACTION_BOND_GOLDEN_LOTUS);
            }
            else if (damage >= me->GetHealth() && phase == PHASE_BOND_GOLDEN_LOTUS && !CheckLotus())
                damage = 0;
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_DESPERATE_MEASURES:
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GARROTE);
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                me->StopAttack();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, me->GetGUID());
                DoCast(me, SPELL_MARK_OF_ANGUISH_MEDITATION);
                me->SummonCreature(NPC_EMBODIED_ANGUISH_OF_HE, 1200.98f, 1044.95f, 417.9685f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                if (!me->GetMap()->IsHeroic())
                    ResetEvents();
                break;
            case ACTION_BOND_GOLDEN_LOTUS:
                events.Reset();
                me->StopAttack();
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2, me->GetGUID());
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3, me->GetGUID());
                me->InterruptNonMeleeSpells(true);
                DoCast(me, SPELL_BOUND_OF_GOLDEN_LOTUS);
                break;
            case ACTION_END_DESPERATE_MEASURES:
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WEAKNESS);
                me->RemoveAurasDueToSpell(SPELL_MARK_OF_ANGUISH_MEDITATION);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveAllAreaObjects();
                me->SetReactState(REACT_AGGRESSIVE);
                StartEvents();
                events.RescheduleEvent(EVENT_GARROTE, 5000);
                events.RescheduleEvent(EVENT_GOUGE, urand(2000, 5000));
                events.RescheduleEvent(EVENT_POISON_NOXIOUS, urand(20000, 30000));
                break;
            case ACTION_BOND_GOLDEN_LOTUS_END:
                me->RemoveAurasDueToSpell(SPELL_BOUND_OF_GOLDEN_LOTUS);
                me->SetReactState(REACT_AGGRESSIVE);
                phase = PHASE_BATTLE_THREE;
                events.RescheduleEvent(EVENT_GARROTE, 5000);
                events.RescheduleEvent(EVENT_GOUGE, urand(2000, 5000));
                events.RescheduleEvent(EVENT_POISON_NOXIOUS, urand(20000, 30000));
                break;
            case ACTION_RESET_EVENTS:
                events.CancelEvent(EVENT_POISON_NOXIOUS);
                events.CancelEvent(EVENT_POISON_INSTANT);
                break;
            case ACTION_START_EVENTS:
                events.RescheduleEvent(EVENT_POISON_NOXIOUS, 21000);
                break;
            }
        }

        void JustDied(Unit* killer)
        {
            if (killer != me)
                SendDone();
            summons.DespawnAll();
            RemoveDebuffs();
            DespawnAllAT();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            EnterEvadeIfOutOfCombatArea(diff);
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_GARROTE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                        DoCast(target, SPELL_SHADOWSTEP);
                    events.RescheduleEvent(EVENT_GARROTE, 25000);
                    break;
                case EVENT_GOUGE:
                    if (me->getVictim())
                        DoCastVictim(SPELL_GOUGE);
                    events.RescheduleEvent(EVENT_GOUGE, 21000);
                    break;
                case EVENT_POISON_NOXIOUS:
                    if (me->getVictim())
                        DoCastVictim(SPELL_NOXIOUS_POISON);
                    events.RescheduleEvent(EVENT_POISON_INSTANT, urand(20000, 30000));
                    break;
                case EVENT_POISON_INSTANT:
                    if (me->getVictim())
                        DoCastVictim(SPELL_INSTANT_POISON);
                    me->RemoveAllAreaObjects();
                    events.RescheduleEvent(EVENT_POISON_NOXIOUS, urand(20000, 30000));
                    break;
                }
            }
            if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) && phase != PHASE_BOND_GOLDEN_LOTUS)
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_he_softfootAI(creature);
    }
};

//Sun Tenderheart
class boss_sun_tenderheart : public CreatureScript
{
public:
    boss_sun_tenderheart() : CreatureScript("boss_sun_tenderheart") {}

    struct boss_sun_tenderheartAI : public boss_fallen_protectors
    {
        boss_sun_tenderheartAI(Creature* creature) : boss_fallen_protectors(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;
        uint32 checkprogress;
        SummonList summons;
        EventMap events;
        Phase phase;
        uint32 shadow_word_count;
        uint8 calamitycount;

        void Reset()
        {
            events.Reset();
            ResetProtectors();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_DEFENSIVE);
            DespawnMeasure();
            phase = PHASE_NULL;
            shadow_word_count = 0;
            calamitycount = 0;
            checkprogress = 0;
            summons.DespawnAll();
            me->SummonCreature(NPC_GOLD_LOTOS_MAIN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0);
            me->RemoveAurasDueToSpell(SPELL_BERSERK);
            me->RemoveAurasDueToSpell(SPELL_BERSERK_TR_EF);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WORD_BANE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void DespawnMeasure()
        {
            std::list<Creature*> list;
            list.clear();
            GetCreatureListWithEntryInGrid(list, me, NPC_EMBODIED_DESPIRE_OF_SUN, 100.0f);
            GetCreatureListWithEntryInGrid(list, me, NPC_EMBODIED_DESPERATION_OF_SUN, 100.0f);
            GetCreatureListWithEntryInGrid(list, me, NPC_DESPAIR_SPAWN, 100.0f);
            GetCreatureListWithEntryInGrid(list, me, BPC_DESPERATION_SPAWN, 100.0f);
            if (!list.empty())
                for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                    (*itr)->DespawnOrUnsummon();
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 150.0f);
            CallOtherProtectors();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_1, me->GetGUID());
            phase = PHASE_BATTLE;
            calamitycount = 0;
            //checkprogress = 5000;
            events.RescheduleEvent(EVENT_SHA_SEAR, 2000);
            events.RescheduleEvent(EVENT_SHADOW_WORD_BANE, urand(15000, 25000));
            events.RescheduleEvent(EVENT_CALAMITY, 30000);
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_CALAMITY_COUNT)
                return (uint32(calamitycount));
            return 0;
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (phase == PHASE_BATTLE && HealthBelowPct(66))
            {
                damage = 0;
                phase = PHASE_BATTLE_TWO;
                DoAction(ACTION_DESPERATE_MEASURES);
            }
            else if (phase == PHASE_BATTLE_TWO && HealthBelowPct(33))
            {
                damage = 0;
                phase = PHASE_BATTLE_THREE;
                DoAction(ACTION_DESPERATE_MEASURES);
            }
            else if (damage >= me->GetHealth() && phase == PHASE_BATTLE_THREE)
            {
                damage = 0;
                phase = PHASE_BOND_GOLDEN_LOTUS;
                me->SetHealth(1);
                DoAction(ACTION_BOND_GOLDEN_LOTUS);
            }
            else if (damage >= me->GetHealth() && phase == PHASE_BOND_GOLDEN_LOTUS && !CheckLotus())
                damage = 0;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_DESPERATE_MEASURES:
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                me->StopAttack();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                calamitycount = 0;
                if (Creature* lotos = me->GetCreature(*me, instance->GetGuidData(NPC_GOLD_LOTOS_MAIN)))
                    DoCast(lotos, SPELL_DARK_MEDITATION_JUMP, true);
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_4, me->GetGUID());
                for (uint32 n = 0; n < 2; n++)
                    if (Creature* measure = me->SummonCreature(sunmeasure[n], sunmeasurepos[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                        measure->AI()->DoZoneInCombat(measure, 150.0f);
                if (me->GetMap()->IsHeroic())
                    events.RescheduleEvent(EVENT_MEDITATION_SPIKE, 6000, PHASE_DESPERATE_MEASURES);
                else
                    ResetEvents();
                break;
            case ACTION_BOND_GOLDEN_LOTUS:
                events.Reset();
                me->StopAttack();
                sCreatureTextMgr->SendChat(me, TEXT_GENERIC_5, me->GetGUID());
                me->InterruptNonMeleeSpells(true);
                DoCast(me, SPELL_BOUND_OF_GOLDEN_LOTUS);
                break;
            case ACTION_END_DESPERATE_MEASURES:
                events.Reset();
                me->RemoveAurasDueToSpell(SPELL_DARK_MEDITATION);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveAllAreaObjects();
                me->SetReactState(REACT_AGGRESSIVE);
                StartEvents();
                events.RescheduleEvent(EVENT_SHA_SEAR, 2000, 0, PHASE_BATTLE);
                events.RescheduleEvent(EVENT_SHADOW_WORD_BANE, urand(15000, 25000), 0, PHASE_BATTLE);
                events.RescheduleEvent(EVENT_CALAMITY, urand(60000, 70000), 0, PHASE_BATTLE);
                break;
            case ACTION_BOND_GOLDEN_LOTUS_END:
                me->RemoveAurasDueToSpell(SPELL_BOUND_OF_GOLDEN_LOTUS);
                me->SetReactState(REACT_AGGRESSIVE);
                phase = PHASE_BATTLE_THREE;
                events.RescheduleEvent(EVENT_SHA_SEAR, 2000);
                events.RescheduleEvent(EVENT_SHADOW_WORD_BANE, urand(15000, 25000));
                events.RescheduleEvent(EVENT_CALAMITY, urand(60000, 70000));
                break;
            case ACTION_RESET_EVENTS:
                events.CancelEvent(EVENT_CALAMITY);
                break;
            case ACTION_START_EVENTS:
                events.RescheduleEvent(EVENT_CALAMITY, urand(60000, 70000));
                break;
            }
        }

        void JustDied(Unit* killer)
        {
            if (killer != me)
                SendDone();
            summons.DespawnAll();
            DespawnMeasure();
            sCreatureTextMgr->SendChat(me, TEXT_GENERIC_6, me->GetGUID());
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WORD_BANE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void SetData(uint32 type, uint32 value)
        {
            switch (type)
            {
            case DATA_SHADOW_WORD_DAMAGE:
                events.RescheduleEvent(EVENT_SHADOW_WORD_BANE, 1000);
                break;
            case DATA_SHADOW_WORD_REMOVED:
                --shadow_word_count;
                break;
            case DATA_CALAMITY_HIT:
                calamitycount++;
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WORD_BANE);
                events.RescheduleEvent(EVENT_SHADOW_WORD_BANE, urand(20000, 30000));
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (checkprogress)
            {
                if (checkprogress <= diff)
                {
                    if (instance->GetBossState(DATA_IMMERSEUS) != DONE)
                        EnterEvadeMode();
                }
                else
                    checkprogress -= diff;
            }

            EnterEvadeIfOutOfCombatArea(diff);
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SHA_SEAR:
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0.0f, true))
                        DoCast(target, SPELL_SHA_SEAR, true);
                    events.RescheduleEvent(EVENT_SHA_SEAR, urand(5000, 10000));
                    break;
                case EVENT_SHADOW_WORD_BANE:
                    if (shadow_word_count < 3)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, -SPELL_SHADOW_WORD_BANE))
                            DoCast(target, SPELL_SHADOW_WORD_BANE, true);
                        ++shadow_word_count;
                    }
                    events.RescheduleEvent(EVENT_SHADOW_WORD_BANE, urand(20000, 30000));
                    break;
                case EVENT_CALAMITY:
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_2);
                    sCreatureTextMgr->SendChat(me, TEXT_GENERIC_3);
                    DoCast(me, SPELL_CALAMITY);
                    events.RescheduleEvent(EVENT_CALAMITY, 30000);
                    break;
                case EVENT_MEDITATION_SPIKE:
                    if (me->getVictim())
                        DoCastVictim(SPELL_MEDITATION_SPIKE, true);
                    events.RescheduleEvent(EVENT_MEDITATION_SPIKE, 6000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_sun_tenderheartAI(creature);
    }
};

//Golden Lotus
class npc_golden_lotus_control : public CreatureScript
{
public:
    npc_golden_lotus_control() : CreatureScript("npc_golden_lotus_control") { }

    struct npc_golden_lotus_controlAI : public ScriptedAI
    {
        npc_golden_lotus_controlAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();            
        }

        enum data
        {
            SUMMON_MOVER        = 143705,
            SPELL_FACE_CHANNEL  = 116351,
        };

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_1, 1000);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                if (Creature* mover = instance->instance->GetCreature(instance->GetGuidData(NPC_GOLD_LOTOS_MOVER)))
                {
                    who->CastSpell(mover, SPELL_FACE_CHANNEL, true);
                    if (!me->HasAura(SPELL_FACE_CHANNEL))
                        me->CastSpell(mover, SPELL_FACE_CHANNEL, true);
                }
            }
        }

        void UpdateAI(uint32 diff){}
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_golden_lotus_controlAI(creature);
    }
};

Position const LotusJumpPosition[6]   =
{
    {1214.094f, 1006.571f, 418.0658f, 0.0f}, //NPC_EMBODIED_DESPIRE_OF_SUN
    {1212.148f, 1057.528f, 417.1646f, 0.0f}, //NPC_EMBODIED_DESPERATION_OF_SUN
    {1204.724f, 1055.807f, 417.6278f, 0.0f}, //NPC_EMBODIED_ANGUISH_OF_HE
    {1202.516f, 1011.427f, 418.1869f, 0.0f}, //NPC_EMBODIED_MISERY_OF_ROOK
    {1208.924f, 1014.313f, 452.1267f, 0.0f}, //NPC_EMBODIED_GLOOM_OF_ROOK
    {1228.698f, 1038.337f, 418.0633f, 0.0f}, //NPC_EMBODIED_SORROW_OF_ROOK
};

class ExitVexMeasure : public BasicEvent
{
public:
    explicit ExitVexMeasure(Creature *c) : creature(c) { }

    bool Execute(uint64 /*currTime*/, uint32 /*diff*/)
    {
        uint8 _idx = 0;
        switch (creature->GetEntry())
        {
        case NPC_EMBODIED_DESPIRE_OF_SUN:
            _idx = 0;
            break;
        case NPC_EMBODIED_DESPERATION_OF_SUN:
            _idx = 1;
            break;
        case NPC_EMBODIED_ANGUISH_OF_HE:
            _idx = 2;
            break;
        case NPC_EMBODIED_MISERY_OF_ROOK:
            _idx = 3;
            break;
        case NPC_EMBODIED_GLOOM_OF_ROOK:
            _idx = 4;
            break;
        case NPC_EMBODIED_SORROW_OF_ROOK:
            _idx = 5;
            break;
        default:
            return true;
        }
        creature->GetMotionMaster()->MoveJump(LotusJumpPosition[_idx].m_positionX, LotusJumpPosition[_idx].m_positionY, LotusJumpPosition[_idx].m_positionZ, 20.0f, 20.0f);
        creature->AI()->DoAction(EVENT_1);
        return true;
    }
private:
    Creature *creature;
};

class vehicle_golden_lotus_conteiner : public VehicleScript
{
public:
    vehicle_golden_lotus_conteiner() : VehicleScript("vehicle_golden_lotus_conteiner") {}

    void OnRemovePassenger(Vehicle* veh, Unit* passenger)
    {
        /*Unit* own = veh->GetBase();
        if (!own)
            return;

        InstanceScript* instance = own->GetInstanceScript();
        if (!instance)
            return;

        Creature* lotos = instance->instance->GetCreature(instance->GetGuidData(NPC_GOLD_LOTOS_MAIN));
        if (!lotos)
            return;

        passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        passenger->m_Events.AddEvent(new ExitVexMeasure(passenger->ToCreature()), passenger->m_Events.CalculateTime(1000));*/
    }
};

class npc_measure_of_sun : public CreatureScript
{
public:
    npc_measure_of_sun() : CreatureScript("npc_measure_of_sun") { }

    struct npc_measure_of_sunAI : public ScriptedAI
    {
        npc_measure_of_sunAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            _spell = me->GetEntry() == NPC_EMBODIED_DESPERATION_OF_SUN ? SPELL_MANIFEST_DESPERATION : SPELL_MANIFEST_DESPAIR;
        }
        InstanceScript* instance;
        SummonList summons;
        uint32 _spell;

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->ToCreature() && summoner->GetEntry() == NPC_SUN_TENDERHEART)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAura(SPELL_SHA_CORRUPTION);
                me->CastSpell(me, SPELL_SHA_CORRUPTION_OF_SUN, true);
                DoZoneInCombat(me, 150.0f);
                DoCast(me, _spell, true);
                if (Creature* lotos = instance->instance->GetCreature(instance->GetGuidData(NPC_GOLD_LOTOS_MAIN)))
                    me->SetFacingToObject(lotos);
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            DoCast(summon, SPELL_DARK_MEDITATION_SHARE_HEALTH, true);
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                summon->AI()->AttackStart(target);
        }

        void JustDied(Unit* killer)
        {
            summons.DespawnAll();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoCast(me, _spell);
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_measure_of_sunAI(creature);
    }
};

class npc_measure_of_he : public CreatureScript
{
public:
    npc_measure_of_he() : CreatureScript("npc_measure_of_he") { }

    struct npc_measure_of_heAI : public ScriptedAI
    {
        npc_measure_of_heAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            _target.Clear();
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid _target;

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->ToCreature() && summoner->GetEntry() == NPC_HE_SOFTFOOT)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                DoCast(me, SPELL_SHA_CORRUPTION_SUMMONED, true);
                events.RescheduleEvent(EVENT_1, 3000);
            }
        }

        void RemoveShadowWeakness()
        {
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SHADOW_WEAKNESS);
        }

        void SetGUID(ObjectGuid const& guid, int32 type) override
        {
            RemoveShadowWeakness();
            events.Reset();
            me->StopAttack();
            me->SetReactState(REACT_PASSIVE);
            _target = guid;
            if (Unit* target = me->GetUnit(*me, guid))
            {
                if (Creature* he = me->GetCreature(*me, instance->GetGuidData(NPC_HE_SOFTFOOT)))
                {
                    sCreatureTextMgr->SendChat(he, TEXT_GENERIC_0);
                    float x, y;
                    GetPosInRadiusWithRandomOrientation(target, 5.0f, x, y);
                    me->CastSpell(x, y, target->GetPositionZ(), SPELL_MARK_OF_ANGUISH_JUMP, true);
                    target->CastSpell(target, SPELL_DEBILITATION, true);
                    me->CastSpell(target, SPELL_MARK_OF_ANGUISH_STUN, false);
                    me->AddThreat(target, 50000000.0f);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->Attack(target, true);
                    DoCast(target, SPELL_SHADOW_WEAKNESS, true);
                    events.RescheduleEvent(EVENT_ACTIVE, 1000);
                }
            }
        }

        void JustDied(Unit* killer)
        {
            RemoveShadowWeakness();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);


            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE:
                    if (Unit* target = me->GetUnit(*me, _target))
                    {
                        if (!target->isAlive() || !target->HasAura(SPELL_MARK_OF_ANGUISH_STUN))
                        {
                            RemoveShadowWeakness();
                            events.RescheduleEvent(EVENT_1, 1000);
                            return;
                        }
                    }
                    events.RescheduleEvent(EVENT_ACTIVE, 1000);
                    break;
                case EVENT_1:
                    me->InterruptNonMeleeSpells(true);
                    DoCast(me, SPELL_SHADOW_WEAKNES_PROC, true);
                    DoCast(me, SPELL_MARK_OF_ANGUISH_SELECT_TARGET, true);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_measure_of_heAI(creature);
    }
};

struct rook_measureAI : ScriptedAI
{
    rook_measureAI(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }
    InstanceScript* instance;

    void SplitDmg(uint32 damage)
    {
        for (int32 n = 0; n < 3; n++)
            if (me->GetEntry() != rookmeasure[n])
                if (Creature* measure = me->GetCreature(*me, instance->GetGuidData(rookmeasure[n])))
                    if (measure->isAlive())
                        measure->SetHealth(measure->GetHealth() - damage);
    }

    void CallDied()
    {
        for (int32 n = 0; n < 3; n++)
            if (me->GetEntry() != rookmeasure[n])
                if (Creature* measure = me->GetCreature(*me, instance->GetGuidData(rookmeasure[n])))
                    if (measure->isAlive())
                        measure->Kill(measure, true);
    }
};

class npc_measure_of_rook : public CreatureScript
{
public:
    npc_measure_of_rook() : CreatureScript("npc_measure_of_rook") { }

    struct npc_measure_of_rookAI : public rook_measureAI
    {
        npc_measure_of_rookAI(Creature* creature) : rook_measureAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            me->SetReactState(REACT_PASSIVE);
            _spell = 0;
        }

        EventMap events;
        uint32 _spell;

        void Reset()
        {
            switch (me->GetEntry())
            {
            case NPC_EMBODIED_MISERY_OF_ROOK:
                _spell = SPELL_DEFILED_GROUND;
                break;
            case NPC_EMBODIED_GLOOM_OF_ROOK:
                _spell = SPELL_CORRUPTION_SHOCK;
                break;
            case NPC_EMBODIED_SORROW_OF_ROOK:
                _spell = SPELL_INFERNO_STRIKE;
                break;
            default:
                break;
            }
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->ToCreature() && summoner->GetEntry() == NPC_ROOK_STONETOE)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                events.RescheduleEvent(EVENT_ACTIVE, 3000);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!me->GetMap()->IsHeroic())
                return;

            if (damage >= me->GetHealth())
                CallDied();
            else
                SplitDmg(damage);
        }

        void EnterCombat(Unit* who)
        {
            switch (me->GetEntry())
            {
            case NPC_EMBODIED_MISERY_OF_ROOK:
                me->CastSpell(me, SPELL_SHA_CORRUPTION_MIS_OF_ROOK, true);
                break;
            case NPC_EMBODIED_GLOOM_OF_ROOK:
                me->CastSpell(me, SPELL_SHA_CORRUPTION_GLOOM_OF_ROOK, true);
                break;
            case NPC_EMBODIED_SORROW_OF_ROOK:
                me->CastSpell(me, SPELL_SHA_CORRUPTION_SOR_OF_ROOK, true);
                break;
            }
            events.RescheduleEvent(EVENT_1, 5000);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);
            
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE:
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                    break;
                case EVENT_1:
                    DoCastVictim(_spell);
                    events.RescheduleEvent(EVENT_1, urand(10000, 15000));
                    break;
                }
            }         
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_measure_of_rookAI(creature);
    }
};

class spell_clash : public SpellScriptLoader
{
public:
    spell_clash() : SpellScriptLoader("spell_OO_clash") { }

    class spell_clash_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_clash_SpellScript);

        enum proc
        {
            SPELL_PROCK = 143028,
        };

        void HandleOnHit()
        {
            if (GetCaster() && GetHitUnit())
                GetCaster()->CastSpell(GetHitUnit(), SPELL_PROCK, false);
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_clash_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_clash_SpellScript();
    }
};

//143019
class spell_corrupted_brew : public SpellScriptLoader
{
public:
    spell_corrupted_brew() : SpellScriptLoader("spell_OO_corrupted_brew") { }

    class spell_corrupted_brew_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_corrupted_brew_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetHitUnit())
            {
                if (!GetCaster()->GetMap()->IsHeroic())
                    GetCaster()->CastSpell(GetHitUnit(), corruptedbrew[0]);
                else
                {
                    uint8 mod = GetCaster()->ToCreature()->AI()->GetData(DATA_CORRUPTED_BREW_COUNT);
                    mod = mod > 8 ? 7 : mod--;
                    GetCaster()->CastSpell(GetHitUnit(), corruptedbrew[mod]);
                }
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_corrupted_brew_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_corrupted_brew_SpellScript();
    }
};

class spell_gouge : public SpellScriptLoader
{
public:
    spell_gouge() : SpellScriptLoader("spell_OO_gouge") { }

    class spell_gouge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gouge_SpellScript);

        void HandleEffect(SpellEffIndex effIndex)
        {
            if (GetCaster() && GetHitUnit())
            {
                if (GetHitUnit()->HasInArc(static_cast<float>(M_PI), GetCaster()))
                {
                    GetCaster()->getThreatManager().modifyThreatPercent(GetHitUnit(), -100);
                    GetCaster()->DeleteFromThreatList(GetCaster());
                }
                else
                    PreventHitAura();
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_gouge_SpellScript::HandleEffect, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gouge_SpellScript();
    }
};

class spell_dark_meditation : public SpellScriptLoader
{
public:
    spell_dark_meditation() : SpellScriptLoader("spell_OO_dark_meditation") { }

    enum proc
    {
        SPELL_PROCK = 143559,
    };

    class spell_dark_meditation_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dark_meditation_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_PROCK, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dark_meditation_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dark_meditation_AuraScript();
    }
};

//Shadow Word: Bane
class spell_fallen_protectors_shadow_word_bane : public SpellScriptLoader
{
public:

    spell_fallen_protectors_shadow_word_bane() :  SpellScriptLoader("spell_fallen_protectors_shadow_word_bane") { }

    class spell_fallen_protectors_shadow_word_bane_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fallen_protectors_shadow_word_bane_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetCaster()->ToCreature())
                GetCaster()->ToCreature()->AI()->SetData(DATA_SHADOW_WORD_REMOVED, true);
        }

        void OnPeriodic(AuraEffect const* /*aurEff*/)
        {
            if (GetCaster() && GetCaster()->ToCreature())
                GetCaster()->ToCreature()->AI()->SetData(DATA_SHADOW_WORD_DAMAGE, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_fallen_protectors_shadow_word_bane_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_fallen_protectors_shadow_word_bane_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fallen_protectors_shadow_word_bane_AuraScript();
    }
};

//143491
class spell_fallen_protectors_calamity : public SpellScriptLoader
{
public:
    spell_fallen_protectors_calamity() : SpellScriptLoader("spell_fallen_protectors_calamity") { }

    class spell_fallen_protectors_calamity_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fallen_protectors_calamity_SpellScript);

        void HandleAfterCast()
        {
            std::list<Player*> pllist;
            pllist.clear();
            GetPlayerListInGrid(pllist, GetCaster(), 150.0f);
            int32 bs = GetCaster()->ToCreature()->AI()->GetData(DATA_CALAMITY_COUNT);
            float dmg = !GetCaster()->GetMap()->IsHeroic() ? 30 : 30 + bs * 10;
            if (!pllist.empty())
            {
                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                    GetCaster()->CastCustomSpell(SPELL_CALAMITY_DMG, SPELLVALUE_BASE_POINT0, dmg, *itr, true);
            }
            GetCaster()->ToCreature()->AI()->SetData(DATA_CALAMITY_HIT, true);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_fallen_protectors_calamity_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_fallen_protectors_calamity_SpellScript();
    }
};

//Mark of Anguish select target.
class spell_fallen_protectors_mark_of_anguish_select_first_target : public SpellScriptLoader
{
public:
    spell_fallen_protectors_mark_of_anguish_select_first_target() :  SpellScriptLoader("spell_fallen_protectors_mark_of_anguish_select_first_target") { }

    class spell_fallen_protectors_mark_of_anguish_select_first_target_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fallen_protectors_mark_of_anguish_select_first_target_SpellScript);

        void SelectTarget(std::list<WorldObject*>& unitList)
        {
            if (unitList.empty())
                return;

            unitList.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
            if (unitList.size() < 1)
                return;

            unitList.resize(1);
        }

        void HandleOnHit()
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetHitUnit())
                GetCaster()->ToCreature()->AI()->SetGUID(GetHitUnit()->GetGUID(), true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_fallen_protectors_mark_of_anguish_select_first_target_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_fallen_protectors_mark_of_anguish_select_first_target_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fallen_protectors_mark_of_anguish_select_first_target_SpellScript();
    }
};

//Shadow Weakness
class spell_fallen_protectors_shadow_weakness_prock : public SpellScriptLoader
{
public:
    spell_fallen_protectors_shadow_weakness_prock() : SpellScriptLoader("spell_fallen_protectors_shadow_weakness_prock") { }

    class spell_fallen_protectors_shadow_weakness_prock_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fallen_protectors_shadow_weakness_prock_AuraScript);

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();
            if (GetCaster())
                if (Unit* target = eventInfo.GetProcTarget())
                    GetCaster()->CastSpell(target, SPELL_SHADOW_WEAKNESS, false);
        }

        void Register()
        {
            OnEffectProc += AuraEffectProcFn(spell_fallen_protectors_shadow_weakness_prock_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fallen_protectors_shadow_weakness_prock_AuraScript();
    }
};

//Mark of Anguish
class spell_fallen_protectors_mark_of_anguish : public SpellScriptLoader
{
public:
    spell_fallen_protectors_mark_of_anguish() : SpellScriptLoader("spell_fallen_protectors_mark_of_anguish") { }

    class spell_fallen_protectors_mark_of_anguish_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fallen_protectors_mark_of_anguish_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetCaster())
            {
                GetTarget()->CastSpell(GetTarget(), SPELL_MARK_OF_ANGUISH_DAMAGE, true, NULL, NULL, GetCaster()->GetGUID());
                //By normal way should prock from our proc system... but where is caster and target is channel target... so this is custom prock reason.
                if (SpellInfo const* m_spellInfo = sSpellMgr->GetSpellInfo(SPELL_MARK_OF_ANGUISH_DAMAGE))
                {
                    DamageInfo dmgInfoProc = DamageInfo(GetCaster(), GetTarget(), 1, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(GetTarget()->GetMap()->GetDifficultyID())->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, 0);
                    GetCaster()->ProcDamageAndSpell(GetTarget(), PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG, 0, PROC_EX_NORMAL_HIT, &dmgInfoProc, BASE_ATTACK, m_spellInfo, aurEff->GetSpellInfo());
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_fallen_protectors_mark_of_anguish_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fallen_protectors_mark_of_anguish_AuraScript();
    }
};

//SPELL_MARK_OF_ANGUISH_GIVE_A_FRIEND     = 143842,
class spell_fallen_protectors_mark_of_anguish_transfer : public SpellScriptLoader
{
public:
    spell_fallen_protectors_mark_of_anguish_transfer() : SpellScriptLoader("spell_fallen_protectors_mark_of_anguish_transfer") { }

    class spell_fallen_protectors_mark_of_anguish_transfer_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fallen_protectors_mark_of_anguish_transfer_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster() && GetHitUnit())
            {
                if (Creature* mesOfHe = GetCaster()->FindNearestCreature(NPC_EMBODIED_ANGUISH_OF_HE, 50.0f, true))
                {
                    mesOfHe->CastSpell(mesOfHe, SPELL_SHADOW_WEAKNES_MASS, true);
                    GetCaster()->RemoveAurasDueToSpell(SPELL_MARK_OF_ANGUISH_STUN);
                    mesOfHe->AI()->SetGUID(GetHitUnit()->GetGUID(), true);
                }
            }
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_fallen_protectors_mark_of_anguish_transfer_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_fallen_protectors_mark_of_anguish_transfer_SpellScript();
    }
};

//SPELL_INFERNO_STRIKE                    = 143962, //Inferno Strike
class spell_fallen_protectors_inferno_strike : public SpellScriptLoader
{
public:
    spell_fallen_protectors_inferno_strike() :  SpellScriptLoader("spell_fallen_protectors_inferno_strike") { }

    class spell_fallen_protectors_inferno_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fallen_protectors_inferno_strike_SpellScript);

        void SelectTarget(std::list<WorldObject*>& unitList)
        {
            SpellValue const* val = GetSpellValue();
            if (!val || unitList.empty())
                return;

            uint32 count = val->EffectBasePoints[EFFECT_0];
            unitList.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
            if (unitList.size() < count)
                return;

            unitList.resize(count);
        }

        void RecalculateDamage()
        {
            SetHitDamage(GetHitDamage() * GetSpellInfo()->Effects[EFFECT_0]->BasePoints);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_fallen_protectors_inferno_strike_SpellScript::SelectTarget, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            OnHit += SpellHitFn(spell_fallen_protectors_inferno_strike_SpellScript::RecalculateDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fallen_protectors_inferno_strike_SpellScript();
    }
};

class spell_fallen_protectors_defile_ground : public SpellScriptLoader
{
public:
    spell_fallen_protectors_defile_ground() : SpellScriptLoader("spell_fallen_protectors_defile_ground") { }

    class spell_fallen_protectors_defile_ground_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fallen_protectors_defile_ground_SpellScript);

        enum data
        {
            AT_ENTRY = 4906,
        };

        void HandleTriggerEffect(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(EFFECT_1);

            if (GetCaster() && GetExplTargetUnit())
            {
                AreaTrigger * areaTrigger = new AreaTrigger;
                if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), AT_ENTRY, GetCaster(), GetSpellInfo(), *GetExplTargetUnit(), *GetExplTargetUnit(), GetSpell()))
                {
                    delete areaTrigger;
                    return;
                }
                areaTrigger->SetSpellId(GetSpellInfo()->Effects[EFFECT_1]->TriggerSpell);
            }
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_fallen_protectors_defile_ground_SpellScript::HandleTriggerEffect, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fallen_protectors_defile_ground_SpellScript();
    }
};

//143497
class spell_bound_of_golden_lotus : public SpellScriptLoader
{
public:
    spell_bound_of_golden_lotus() : SpellScriptLoader("spell_bound_of_golden_lotus") { }

    class spell_bound_of_golden_lotus_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_bound_of_golden_lotus_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature())
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_BOND_GOLDEN_LOTUS_END);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_bound_of_golden_lotus_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_bound_of_golden_lotus_SpellScript();
    }
};

void AddSC_boss_fallen_protectors()
{
    new boss_rook_stonetoe();
    new boss_he_softfoot();
    new boss_sun_tenderheart();
    new npc_golden_lotus_control();
    new vehicle_golden_lotus_conteiner();
    new npc_measure_of_sun();
    new npc_measure_of_he();
    new npc_measure_of_rook();
    new spell_clash();
    new spell_corrupted_brew();
    new spell_gouge();
    new spell_dark_meditation();
    new spell_fallen_protectors_shadow_word_bane();
    new spell_fallen_protectors_calamity();
    new spell_fallen_protectors_mark_of_anguish_select_first_target();
    new spell_fallen_protectors_shadow_weakness_prock();
    new spell_fallen_protectors_mark_of_anguish();
    new spell_fallen_protectors_mark_of_anguish_transfer();
    new spell_fallen_protectors_inferno_strike();
    new spell_fallen_protectors_defile_ground();
    new spell_bound_of_golden_lotus();
}