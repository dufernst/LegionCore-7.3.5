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

#include "mogu_shan_vault.h"

enum eSpells
{
    // Shared
    SPELL_SPIRIT_BOLT                   = 118530,
    SPELL_STRENGHT_OF_SPIRIT            = 116363,
    SPELL_DRAW_ESSENCE                  = 121631,

    // Visuals
    SPELL_SPIRIT_FIST                   = 115743,
    SPELL_SPIRIT_SPEAR                  = 115740,
    SPELL_SPIRIT_STAFF                  = 115742,
    SPELL_SPIRIT_SHIELD                 = 115741,

    // Spirit of the Fist
    SPELL_LIGHTNING_LASH                = 131788,
    SPELL_LIGHTNING_FISTS               = 116157,
    SPELL_EPICENTER                     = 116018,

    // Spirit of the Spear
    SPELL_FLAMING_SPEAR                 = 116942,
    SPELL_WILDFIRE_SPARK                = 116784,
    SPELL_DRAW_FLAME                    = 116711,
    SPELL_WILDFIRE_INFUSION             = 116817,
    SPELL_WILDFIRE_INFUSION_VISUAL      = 116821,

    // Spirit of the Staff 
    SPELL_ARCANE_SHOCK                  = 131790,
    SPELL_ARCANE_VELOCITY               = 116364,
    SPELL_ARCANE_RESONANCE              = 116417,

    // Spirit of the Shield ( Heroic )
    SPELL_SHADOWBURN                    = 131792,
    SPELL_CHAINS_OF_SHADOW              = 118783,
    SPELL_SIPHONING_SHIELD              = 117203,
    SPELL_S_SHIELD_HEAL                 = 118071,
    SPELL_S_SHIELD_VISUAL_SHIELD        = 117763,
    SPELL_S_SHIELD_VISUAL_ZONE          = 117240,
    SPELL_S_SHIELD_SUM_FRAGMENT         = 117716, //> 117717
    SPELL_S_SHIELD_FRAGMENT_LINE        = 117736,

    // Stolen Essences of Stone
    SPELL_NULLIFICATION_BARRIER         = 115817,
    SPELL_SHROUD_OF_REVERSAL            = 115911,

    // Controler Visual
    SPELL_VISUAL_FIST                   = 105032,
    SPELL_VISUAL_SPEAR                  = 118485,
    SPELL_VISUAL_STAFF                  = 118486,
    SPELL_VISUAL_SHIELD                 = 117303,

    // Inversions Spell
    SPELL_INVERSION                     = 115972,

    SPELL_EPICENTER_INVERSION           = 118300,
    SPELL_LIGHTNING_FISTS_INVERSION     = 118302,
    SPELL_ARCANE_RESONANCE_INVERSION    = 118304,
    SPELL_ARCANE_VELOCITY_INVERSION     = 118305,
    SPELL_WILDFIRE_SPARK_INVERSION      = 118307,
    SPELL_FLAMING_SPEAR_INVERSION       = 118308,
    // Inversion bouclier siphon        = 118471,
    SPELL_SHADOWBURN_INVERSION          = 132296,
    SPELL_LIGHTNING_LASH_INVERSION      = 132297,
    SPELL_ARCANE_SHOCK_INVERSION        = 132298,

    SPELL_UNARMED                       = 124252,
    SPELL_CLONE_ME                      = 126240,
};

enum eSummon
{
    NPC_LIGHTNING_FISTS         = 60241,
    NPC_WILDFIRE_SPARK          = 60438,
    NPC_SIPHONING_SHIELD        = 60627,
    NPC_SOUL_FRAGMENT           = 60781,
};

enum eEvents
{
    EVENT_DOT_ATTACK            = 1,
    EVENT_RE_ATTACK             = 2,

    EVENT_LIGHTNING_FISTS       = 3,
    EVENT_EPICENTER             = 4,

    EVENT_WILDFIRE_SPARK        = 5,
    EVENT_DRAW_FLAME            = 6,

    EVENT_ARCANE_VELOCITY       = 7,
    EVENT_ARCANE_VELOCITY_END   = 8,
    EVENT_ARCANE_RESONANCE      = 9,

    EVENT_SIPHONING_SHIELD      = 10,
    EVENT_CHAINS_OF_SHADOW      = 11,

    EVENT_SPIRIT_BOLT           = 12,
};

enum eFengPhases
{
    PHASE_NONE                  = 0,
    PHASE_FIST                  = 1,
    PHASE_SPEAR                 = 2,
    PHASE_STAFF                 = 3,
    PHASE_SHIELD                = 4
};

Position modPhasePositions[4] =
{
    {4063.26f, 1320.50f, 466.30f, 5.5014f}, // Phase Fist
    {4021.17f, 1320.50f, 466.30f, 3.9306f}, // Phase Spear
    {4021.17f, 1362.80f, 466.30f, 2.0378f}, // Phase Staff
    {4063.26f, 1362.80f, 466.30f, 0.7772f}, // Phase Shield
};

uint32 statueEntryInOrder[4] = {GOB_FIST_STATUE,   GOB_SPEAR_STATUE,   GOB_STAFF_STATUE,   GOB_SHIELD_STATUE};
uint32 controlerVisualId[4]  = {SPELL_VISUAL_FIST, SPELL_VISUAL_SPEAR, SPELL_VISUAL_STAFF, SPELL_VISUAL_SHIELD};
uint32 fengVisualId[4]       = {SPELL_SPIRIT_FIST, SPELL_SPIRIT_SPEAR, SPELL_SPIRIT_STAFF, SPELL_SPIRIT_SHIELD};

#define MAX_INVERSION_SPELLS    9
uint32 inversionMatching[MAX_INVERSION_SPELLS][2] =
{
    {SPELL_EPICENTER,        SPELL_EPICENTER_INVERSION},
    {SPELL_LIGHTNING_FISTS,  SPELL_LIGHTNING_FISTS_INVERSION},
    {SPELL_ARCANE_RESONANCE, SPELL_ARCANE_RESONANCE_INVERSION},
    {SPELL_ARCANE_VELOCITY,  SPELL_ARCANE_VELOCITY_INVERSION},
    {SPELL_WILDFIRE_SPARK,   SPELL_WILDFIRE_SPARK_INVERSION},
    {SPELL_FLAMING_SPEAR,    SPELL_FLAMING_SPEAR_INVERSION},
    {SPELL_SHADOWBURN,       SPELL_SHADOWBURN_INVERSION},
    {SPELL_LIGHTNING_LASH,   SPELL_LIGHTNING_LASH_INVERSION},
    {SPELL_ARCANE_SHOCK,     SPELL_ARCANE_SHOCK_INVERSION}
};

#define MAX_DIST    60

float const minpullpos = 4007.9379f;
float const maxpullpos = 4076.9135f;

class boss_feng : public CreatureScript
{
    public:
        boss_feng() : CreatureScript("boss_feng") {}

        struct boss_fengAI : public BossAI
        {
            boss_fengAI(Creature* creature) : BossAI(creature, DATA_FENG), summons(me)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            SummonList summons;
            bool phaseone, phasetwo, phasethree, phasefour;
            uint8 newphase;
            uint8 actualPhase;
            uint8 fragmentCount;
            uint32 dotSpellId, checkvictim;
            ObjectGuid targetGuid;

            void Reset() override
            {
                _Reset();
                events.Reset();
                summons.DespawnAll();
                TrashDespawn();
                phaseone = false;
                phasetwo = false;
                phasethree = false;
                phasefour = false;
                checkvictim = 0; 
                newphase = 0;
                actualPhase = PHASE_NONE;
                dotSpellId = 0;
                targetGuid.Clear();
                fragmentCount = 0;
                me->RemoveAllAuras();
                pInstance->DoRemoveAurasDueToSpellOnPlayers(115811);
                pInstance->DoRemoveAurasDueToSpellOnPlayers(115972);

                for (uint8 i = 0; i < 4; ++i)
                {
                    me->RemoveAurasDueToSpell(fengVisualId[i]);

                    if (GameObject* oldStatue = pInstance->instance->GetGameObject(pInstance->GetGuidData(statueEntryInOrder[i])))
                    {
                        oldStatue->SetLootState(GO_READY);
                        oldStatue->SetGoState(GO_STATE_READY);
                    }
                }

                if (GameObject* inversionGob = pInstance->instance->GetGameObject(pInstance->GetGuidData(GOB_INVERSION)))
                    inversionGob->Delete();

                if (GameObject* cancelGob = pInstance->instance->GetGameObject(pInstance->GetGuidData(GOB_CANCEL)))
                    cancelGob->Delete();
            }

            void JustReachedHome() override
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
            }

            bool CheckPullPlayerPos(Unit* who)
            {
                if (!who->ToPlayer() || who->GetPositionX() < minpullpos || who->GetPositionX() > maxpullpos)
                    return false;

                return true;
            }
            
            void EnterCombat(Unit* who) override
            {
                if (instance)
                {
                    if (!CheckPullPlayerPos(who))
                    {
                        EnterEvadeMode();
                        return;
                    }
                }
                _EnterCombat();
                checkvictim = 1500;

                me->SummonGameObject(GOB_INVERSION, 4027.3f, 1331.39f, 468.80f, 0, 0, 0, 0, 0, 604800);
                me->SummonGameObject(GOB_CANCEL, 4028.32f, 1352.85f, 466.30f, 0, 0, 0, 0, 0, 604800);
            }

            void JustDied(Unit* attacker) override
            {
                _JustDied();
                summons.DespawnAll();
                TrashDespawn();
                pInstance->DoRemoveAurasDueToSpellOnPlayers(115811);
                pInstance->DoRemoveAurasDueToSpellOnPlayers(115972);

                if (GameObject* inversionGob = pInstance->instance->GetGameObject(pInstance->GetGuidData(GOB_INVERSION)))
                    inversionGob->Delete();

                if (GameObject* cancelGob = pInstance->instance->GetGameObject(pInstance->GetGuidData(GOB_CANCEL)))
                    cancelGob->Delete();
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id >= 1 && id <= 4)
                    PrepareNewPhase(id);
            }

            /* void DoAction(const int32 action) override
            {
                if (action == ACTION_SPARK)
                    if (Aura* aura = me->GetAura(SPELL_WILDFIRE_INFUSION))
                        aura->ModCharges(1);
                    else
                        me->AddAura(SPELL_WILDFIRE_INFUSION, me);
            } */

            void PrepareNewPhase(uint8 newPhase)
            {
                events.Reset();
                events.RescheduleEvent(EVENT_DOT_ATTACK, 15000);
                events.RescheduleEvent(EVENT_RE_ATTACK,  1000);
                events.RescheduleEvent(EVENT_SPIRIT_BOLT, 8000);

                me->GetMotionMaster()->Clear();

                if (Creature* controler = me->FindNearestCreature(NPC_PHASE_CONTROLER, 60.0f, true))
                {
                    controler->RemoveAllAuras();
                    controler->DespawnOrUnsummon();
                }

                // Desactivate old statue and enable the new one
                if (GameObject* oldStatue = pInstance->instance->GetGameObject(pInstance->GetGuidData(statueEntryInOrder[actualPhase - 1])))
                    oldStatue->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);

                if (GameObject* newStatue = pInstance->instance->GetGameObject(pInstance->GetGuidData(statueEntryInOrder[newPhase - 1])))
                    newStatue->UseDoorOrButton();

                me->CastSpell(me, fengVisualId[newPhase - 1], true);
                me->CastSpell(me, SPELL_DRAW_ESSENCE, true);

                if (IsHeroic())
                    me->CastSpell(me, SPELL_STRENGHT_OF_SPIRIT, true);

                switch (newPhase)
                {
                    case PHASE_FIST:
                    {
                        dotSpellId = SPELL_LIGHTNING_LASH;
                        events.RescheduleEvent(EVENT_LIGHTNING_FISTS,  20000, PHASE_FIST);
                        events.RescheduleEvent(EVENT_EPICENTER,        35000, PHASE_FIST);
                        break;
                    }
                    case PHASE_SPEAR:
                    {
                        dotSpellId = SPELL_FLAMING_SPEAR;
                        events.RescheduleEvent(EVENT_WILDFIRE_SPARK,   12000, PHASE_SPEAR);
                        events.RescheduleEvent(EVENT_DRAW_FLAME,       36000, PHASE_SPEAR);
                        break;
                    }
                    case PHASE_STAFF:
                    {
                        dotSpellId = SPELL_ARCANE_SHOCK;
                        events.RescheduleEvent(EVENT_ARCANE_VELOCITY,  25000, PHASE_STAFF);
                        events.RescheduleEvent(EVENT_ARCANE_RESONANCE, 10000, PHASE_STAFF);
                        break;
                    }
                    case PHASE_SHIELD:
                    {
                        dotSpellId = SPELL_SHADOWBURN;
                        events.RescheduleEvent(EVENT_SIPHONING_SHIELD, 6000, PHASE_SHIELD);
                        events.RescheduleEvent(EVENT_CHAINS_OF_SHADOW, 8000, PHASE_SHIELD);
                        break;
                    }
                    default:
                        break;
                }

                actualPhase = newPhase;
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (!pInstance || !me->isInCombat())
                    return;

                if (HealthBelowPct(IsHeroic() ? 100 : 95) && !phaseone)
                {
                    phaseone = true;
                    newphase = 1;
                    me->StopAttack();
                    if (Creature* controler = me->SummonCreature(NPC_PHASE_CONTROLER, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ()))
                        controler->AddAura(controlerVisualId[newphase - 1], controler);
                    me->GetMotionMaster()->MovePoint(newphase, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ());
                }
                else if (HealthBelowPct(IsHeroic() ? 75 : 66) && !phasetwo)
                {
                    phasetwo = true;
                    newphase = 2;
                    me->StopAttack();
                    if (Creature* controler = me->SummonCreature(NPC_PHASE_CONTROLER, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ()))
                        controler->AddAura(controlerVisualId[newphase - 1], controler);
                    me->GetMotionMaster()->MovePoint(newphase, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ());
                }
                else if (HealthBelowPct(IsHeroic() ? 50 : 33) && !phasethree)
                {
                    phasethree = true;
                    newphase = 3;
                    me->StopAttack();
                    if (Creature* controler = me->SummonCreature(NPC_PHASE_CONTROLER, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ()))
                        controler->AddAura(controlerVisualId[newphase - 1], controler);
                    me->GetMotionMaster()->MovePoint(newphase, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ());
                }
                else if (HealthBelowPct(25) && !phasefour && IsHeroic())
                {
                    phasefour = true;
                    newphase = 4;
                    me->StopAttack();
                    if (Creature* controler = me->SummonCreature(NPC_PHASE_CONTROLER, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ()))
                        controler->AddAura(controlerVisualId[newphase - 1], controler);
                    me->GetMotionMaster()->MovePoint(newphase, modPhasePositions[newphase - 1].GetPositionX(), modPhasePositions[newphase - 1].GetPositionY(), modPhasePositions[newphase - 1].GetPositionZ());
                }
            }

            void TrashDespawn()
            {
                std::list<Creature*> trashList;
                me->GetCreatureListWithEntryInGrid(trashList, NPC_WILDFIRE_SPARK, 100.0f);
                me->GetCreatureListWithEntryInGrid(trashList, NPC_SOUL_FRAGMENT, 100.0f);
                for (std::list<Creature*>::const_iterator itr = trashList.begin(); itr != trashList.end(); ++itr)
                    (*itr)->DespawnOrUnsummon();
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_S_SHIELD_SUM_FRAGMENT)
                    target->CastSpell(target, 117717, true, 0, NULL, me->GetGUID()); //Sum Fragment
            }

            void JustSummoned(Creature* sum) override
            {
                summons.Summon(sum);

                switch (sum->GetEntry())
                {
                    case NPC_LIGHTNING_FISTS:
                        if (Unit* pl = me->GetPlayer(*me, targetGuid))
                        {
                            sum->AI()->SetGUID(targetGuid); 
                            targetGuid.Clear();
                            break;
                        }
                    case NPC_SIPHONING_SHIELD:
                        sum->SetReactState(REACT_PASSIVE);
                        sum->CastSpell(sum, SPELL_S_SHIELD_VISUAL_SHIELD, true);
                        sum->CastSpell(sum, SPELL_S_SHIELD_VISUAL_ZONE, true);
                        me->CastSpell(me, SPELL_S_SHIELD_SUM_FRAGMENT, true);
                        break;
                    case NPC_SOUL_FRAGMENT:
                        fragmentCount++;
                        break;
                }
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
            {
                if (summon->GetEntry() == NPC_SOUL_FRAGMENT)
                {
                    fragmentCount--;
                    if (fragmentCount <= 0)
                        summons.DespawnEntry(NPC_SIPHONING_SHIELD);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (checkvictim && instance)
                {
                    if (checkvictim <= diff)
                    {
                        if (me->getVictim())
                        {
                            if (!CheckPullPlayerPos(me->getVictim()))
                            {
                                me->AttackStop();
                                me->SetReactState(REACT_PASSIVE);
                                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                EnterEvadeMode();
                                checkvictim = 0;
                            }
                            else
                                checkvictim = 1500;
                        }
                    }
                    else
                        checkvictim -= diff;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                switch(events.ExecuteEvent())
                {
                    // All Phases
                    case EVENT_DOT_ATTACK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            DoCast(target, dotSpellId);
                        events.RescheduleEvent(EVENT_DOT_ATTACK, 12500);
                        break;
                    case EVENT_RE_ATTACK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            me->GetMotionMaster()->MoveChase(target);
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case EVENT_SPIRIT_BOLT:
                        DoCast(SPELL_SPIRIT_BOLT);
                        events.RescheduleEvent(EVENT_SPIRIT_BOLT, 8000);
                        break;
                     // Fist Phase
                    case EVENT_LIGHTNING_FISTS:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            if (targetGuid)
                                targetGuid.Clear();
                            targetGuid = target->GetGUID();
                            DoCast(target, SPELL_LIGHTNING_FISTS);
                        }
                        events.RescheduleEvent(EVENT_LIGHTNING_FISTS, 20000);
                        break;
                    case EVENT_EPICENTER:
                        DoCast(me, SPELL_EPICENTER);
                        events.RescheduleEvent(EVENT_EPICENTER, 30000);
                        break;
                    // Spear Phase
                    case EVENT_WILDFIRE_SPARK:
                        DoCast(SPELL_WILDFIRE_SPARK);
                        events.RescheduleEvent(EVENT_WILDFIRE_SPARK, 14000);
                        break;
                    case EVENT_DRAW_FLAME: 
                        DoCast(me, SPELL_WILDFIRE_INFUSION, true);
                        DoCast(me, SPELL_DRAW_FLAME);
                        events.RescheduleEvent(EVENT_DRAW_FLAME, 36000);
                        break;
                    // Staff Phase
                    case EVENT_ARCANE_VELOCITY:
                        DoCast(me, SPELL_ARCANE_VELOCITY);
                        events.RescheduleEvent(EVENT_ARCANE_VELOCITY, 15000);
                        break;
                    case EVENT_ARCANE_RESONANCE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                            target->AddAura(SPELL_ARCANE_RESONANCE, target);
                        events.RescheduleEvent(EVENT_ARCANE_RESONANCE, 20000);
                        break;
                    // Shield Phase
                    case EVENT_SIPHONING_SHIELD:
                        DoCast(SPELL_SIPHONING_SHIELD);
                        events.RescheduleEvent(EVENT_SIPHONING_SHIELD, 34000);
                        break;
                    case EVENT_CHAINS_OF_SHADOW:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            DoCast(target, SPELL_CHAINS_OF_SHADOW);
                        events.RescheduleEvent(EVENT_CHAINS_OF_SHADOW, 6000, PHASE_SHIELD);
                        break;
                    default:
                        break;
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_fengAI(creature);
        }
};

enum eLightningFistSpell
{
    SPELL_FIST_BARRIER      = 115856,
    SPELL_FIST_CHARGE       = 116374,
    SPELL_FIST_VISUAL       = 116225,
    SPELL_AURA_SEARCHER     = 129426,
    SPELL_SEARCHER          = 129428
};

class mob_lightning_fist : public CreatureScript
{
    public:
        mob_lightning_fist() : CreatureScript("mob_lightning_fist") {}

        struct mob_lightning_fistAI : public ScriptedAI
        {
            mob_lightning_fistAI(Creature* creature) : ScriptedAI(creature)
            {
                InstanceScript* pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* pInstance;

            uint32 unsummon;
            
            void Reset() override
            {
                unsummon = 6000;
            }

            void SetGUID(ObjectGuid const& guid, int32 type/* = 0 */) override
            {
                if (guid)
                {
                    if (Unit* target = me->GetPlayer(*me, guid))
                    {
                        if (target->isAlive())
                        {
                            me->SetFacingToObject(target);
                            me->AddAura(SPELL_AURA_SEARCHER, me);
                            me->AddAura(SPELL_FIST_VISUAL, me);
                            float x = 0, y = 0;
                            GetPositionWithDistInOrientation(me, 100.0f, me->GetOrientation(), x, y);
                            me->GetMotionMaster()->MovePoint(1, x, y, me->GetPositionZ());
                            return;
                        }
                    }
                }
                me->DespawnOrUnsummon();
            }

            void EnterCombat(Unit* who) override {}

            void EnterEvadeMode() override {}

            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                if (target->GetTypeId() == TYPEID_PLAYER && spell->Id == SPELL_SEARCHER)
                    DoCast(target, SPELL_FIST_CHARGE);
            }

            void UpdateAI(uint32 diff) override
            {
                if (unsummon)
                {
                    if (unsummon <= diff)
                        me->DespawnOrUnsummon();
                    else
                        unsummon -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_lightning_fistAI(creature);
        }
};

//60438
class mob_wild_spark : public CreatureScript
{
    public:
        mob_wild_spark() : CreatureScript("mob_wild_spark") {}

        struct mob_wild_sparkAI : public ScriptedAI
        {
            mob_wild_sparkAI(Creature* creature) : ScriptedAI(creature)
            {}

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                me->CastSpell(me, 116717, true); // Fire aura
                me->CastSpell(me, 116787, true); //Fire Visual
                me->GetMotionMaster()->MoveRandom(5.0f);
            }
    
            void SpellHit(Unit* caster, SpellInfo const* spell) override
            {
                if (spell->Id == SPELL_DRAW_FLAME)
                    me->GetMotionMaster()->MovePoint(1, caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == 1)
                    if (InstanceScript* pInstance = me->GetInstanceScript())
                        if (Creature* feng = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_FENG)))
                        {
                            feng->CastSpell(feng, SPELL_WILDFIRE_INFUSION_VISUAL, true);
                            me->DespawnOrUnsummon();
                        }
            }

            void UpdateAI(uint32 diff) override
            {
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_wild_sparkAI(creature);
        }
};

//60781
class npc_feng_soul_fragment : public CreatureScript
{
    public:
        npc_feng_soul_fragment() : CreatureScript("npc_feng_soul_fragment") {}

        struct npc_feng_soul_fragmentAI : public ScriptedAI
        {
            npc_feng_soul_fragmentAI(Creature* creature) : ScriptedAI(creature)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
            }
    
            void IsSummonedBy(Unit* summoner)
            {
                if (Player* plr = me->FindNearestPlayer(5.0f))
                    plr->CastSpell(me, SPELL_CLONE_ME, true);

                if (Creature* shield = me->FindNearestCreature(NPC_SIPHONING_SHIELD, 100.0f))
                {
                    me->CastSpell(shield, SPELL_S_SHIELD_FRAGMENT_LINE, true);
                    me->GetMotionMaster()->MovePoint(1, shield->GetPositionX(), shield->GetPositionY(), shield->GetPositionZ());
                }
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == 1)
                    if (InstanceScript* pInstance = me->GetInstanceScript())
                    {
                        float bp = pInstance->instance->GetDifficultyID() == DIFFICULTY_10_HC ? 10.f : 5.f;
                        me->CastCustomSpell(me, SPELL_S_SHIELD_HEAL, &bp, NULL, NULL, true);
                        me->Kill(me);
                    }
            }

            void UpdateAI(uint32 diff) override {}
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_feng_soul_fragmentAI(creature);
        }
};

//211626, 211628
class go_stolen_essences_of_stone : public GameObjectScript
{
public:
    go_stolen_essences_of_stone() : GameObjectScript("go_stolen_essences_of_stone") {}

    bool OnGossipHello(Player* pPlayer, GameObject* pGo)
    {
        if (pPlayer->isInTankSpec())
            return false;

        return true;
    }
};

// Mogu Epicenter - 116040
class spell_mogu_epicenter : public SpellScriptLoader
{
    public:
        spell_mogu_epicenter() : SpellScriptLoader("spell_mogu_epicenter") { }

        class spell_mogu_epicenter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mogu_epicenter_SpellScript);

            void DealDamage()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!caster || !target)
                    return;
                
                float distance = caster->GetExactDist2d(target);

                if (distance >= 0 && distance <= 60)
                    SetHitDamage(GetHitDamage() * (1 - (distance / MAX_DIST)));
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_mogu_epicenter_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mogu_epicenter_SpellScript();
        }
};

// Wildfire Spark - 116583
class spell_mogu_wildfire_spark : public SpellScriptLoader
{
    public:
        spell_mogu_wildfire_spark() : SpellScriptLoader("spell_mogu_wildfire_spark") { }

        class spell_mogu_wildfire_spark_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mogu_wildfire_spark_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                uint8 maxSpark = 3;

                Unit* caster = GetCaster();
                if (!caster)
                    return;

                for (uint8 i = 0; i < maxSpark; ++i)
                {
                    float position_x = caster->GetPositionX() + frand(-3.0f, 3.0f);
                    float position_y = caster->GetPositionY() + frand(-3.0f, 3.0f);
                    if (InstanceScript* pInstance = caster->GetInstanceScript())
                        if (Creature* feng = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_FENG)))
                            feng->CastSpell(position_x, position_y, caster->GetPositionZ(), 116586, true);
                }
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_mogu_wildfire_spark_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mogu_wildfire_spark_SpellScript();
        }
};

// Wildfire Infusion - 116816
class spell_mogu_wildfire_infusion : public SpellScriptLoader
{
    public:
        spell_mogu_wildfire_infusion() : SpellScriptLoader("spell_mogu_wildfire_infusion") { }

        class spell_mogu_wildfire_infusion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mogu_wildfire_infusion_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                    if (Aura* aura = caster->GetAura(SPELL_WILDFIRE_INFUSION_VISUAL))
                    {
                        int8 stack = aura->GetStackAmount();
                        aura->SetStackAmount(stack - 1);

                        if (stack < 1)
                        {
                            caster->RemoveAura(SPELL_WILDFIRE_INFUSION);
                            caster->RemoveAura(SPELL_WILDFIRE_INFUSION_VISUAL);
                        }
                    }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_mogu_wildfire_infusion_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mogu_wildfire_infusion_SpellScript();
        }
};

// Arcane Velocity - 116365
class spell_mogu_arcane_velocity : public SpellScriptLoader
{
    public:
        spell_mogu_arcane_velocity() : SpellScriptLoader("spell_mogu_arcane_velocity") { }

        class spell_mogu_arcane_velocity_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mogu_arcane_velocity_SpellScript);

            void DealDamage()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!caster || !target)
                    return;
                
                float distance = caster->GetExactDist2d(target);

                if (distance >= 0 && distance <= 60)
                    SetHitDamage(GetHitDamage() * (distance / MAX_DIST));
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_mogu_arcane_velocity_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mogu_arcane_velocity_SpellScript();
        }
};

// Arcane Resonance - 116434
class spell_mogu_arcane_resonance : public SpellScriptLoader
{
    public:
        spell_mogu_arcane_resonance() : SpellScriptLoader("spell_mogu_arcane_resonance") { }

        class spell_mogu_arcane_resonance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mogu_arcane_resonance_SpellScript);

            uint8 targetCount;

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targetCount = targets.size();
            }

            void DealDamage()
            {
                if (targetCount > 25)
                    targetCount = 1;

                SetHitDamage(GetHitDamage() * targetCount);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mogu_arcane_resonance_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnHit                    += SpellHitFn(spell_mogu_arcane_resonance_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mogu_arcane_resonance_SpellScript();
        }
};

// Mogu Inversion - 118300 / 118302 / 118304 / 118305 / 118307 / 118308 / 132296 / 132297 / 132298
class spell_mogu_inversion : public SpellScriptLoader
{
    public:
        spell_mogu_inversion() : SpellScriptLoader("spell_mogu_inversion") { }

        class spell_mogu_inversion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mogu_inversion_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    GetTarget()->RemoveAurasDueToSpell(SPELL_INVERSION);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    GetTarget()->CastSpell(GetTarget(), SPELL_INVERSION, true);
            }

            void Register() override
            {
                OnEffectApply     += AuraEffectApplyFn(spell_mogu_inversion_AuraScript::OnApply,   EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_mogu_inversion_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mogu_inversion_AuraScript();
        }
};

void AddSC_boss_feng()
{
    new boss_feng();
    new mob_lightning_fist();
    new mob_wild_spark();
    new npc_feng_soul_fragment();
    new go_stolen_essences_of_stone();
    new spell_mogu_epicenter();
    new spell_mogu_wildfire_spark();
    new spell_mogu_wildfire_infusion();
    new spell_mogu_arcane_velocity();
    new spell_mogu_arcane_resonance();
    new spell_mogu_inversion();
}
