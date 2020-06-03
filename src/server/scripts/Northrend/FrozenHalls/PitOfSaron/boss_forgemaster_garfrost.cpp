/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "pit_of_saron.h"

enum Texts
{
    SAY_AGGRO                   = 0,
    SAY_PHASE2                  = 1,
    SAY_PHASE3                  = 2,
    SAY_DEATH                   = 3,
    SAY_SLAY                    = 4,
    SAY_THROW_SARONITE          = 5,
    SAY_CAST_DEEP_FREEZE        = 6,

    SAY_TYRANNUS_DEATH          = 0
};

enum Npc
{
    NPC_SLAVE_HORDE_1           = 37578,
    NPC_SLAVE_HORDE_2           = 37577,
    NPC_SLAVE_HORDE_3           = 37579,
    NPC_SLAVE_ALY_1             = 37572,
    NPC_SLAVE_ALY_2             = 37575,
    NPC_SLAVE_ALY_3             = 37576,
    NPC_TYRANNUS_INTRO          = 36794,
};

enum Spells
{
    SPELL_PERMAFROST            = 70326,
    SPELL_THROW_SARONITE        = 68788,
    SPELL_THUNDERING_STOMP      = 68771,
    SPELL_CHILLING_WAVE         = 68778,
    SPELL_DEEP_FREEZE           = 70381,
    SPELL_FORGE_MACE            = 68785,
    SPELL_FORGE_BLADE           = 68774,
    SPELL_PERMAFROST_HELPER     = 68786,
};

enum Events
{
    EVENT_THROW_SARONITE        = 1,
    EVENT_CHILLING_WAVE         = 2,
    EVENT_DEEP_FREEZE           = 3,
    //EVENT_JUMP                = 4,
    EVENT_FORGING               = 5,
    EVENT_RESUME_ATTACK         = 6,
};

enum Phases
{
    PHASE_ONE                   = 1,
    PHASE_TWO                   = 2,
    PHASE_THREE                 = 3,
};

enum MiscData
{
    EQUIP_ID_SWORD              = 49345,
    EQUIP_ID_MACE               = 49344,
    ACHIEV_DOESNT_GO_TO_ELEVEN  = 0,
    POINT_FORGE                 = 0,
};
//Positional defines 
struct LocationsXY
{
    float x, y, z, o;
    uint32 id;
};

static LocationsXY MoveLoc[]=
{
    {677.445f, -186.521f, 526.702f},
    {708.190f, -194.619f, 526.805f},
    {687.257f, -193.644f, 526.717f}, 
};

static LocationsXY SummonLoc[]=
{
    {719.812f, -167.183f, 526.721f,},
    {698.703f, -165.497f, 527.464f,},
    {671.455f, -167.968f, 526.741f,},
};

Position const northForgePos = {722.5643f, -234.1615f, 527.182f, 2.16421f};
Position const southForgePos = {639.257f, -210.1198f, 529.015f, 0.523599f};

class boss_garfrost : public CreatureScript
{
    public:
        boss_garfrost() : CreatureScript("boss_garfrost") {}

        struct boss_garfrostAI : public BossAI
        {
            boss_garfrostAI(Creature *creature) : BossAI(creature, DATA_GARFROST) {}

            void Reset() override
            {
                _Reset();
                events.SetPhase(PHASE_ONE);
                SetEquipmentSlots(true);
                _permafrostStack = 0;
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                DoCast(me, SPELL_PERMAFROST);
                events.ScheduleEvent(EVENT_THROW_SARONITE, 7000);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim && victim->IsPlayer())
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
                _JustDied();
                if (Creature* tyrannus = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_TYRANNUS)))
                    tyrannus->AI()->Talk(SAY_TYRANNUS_DEATH);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*uiDamage*/, DamageEffectType dmgType) override
            {
                if (events.IsInPhase(PHASE_ONE) && !HealthAbovePct(66))
                {
                    events.SetPhase(PHASE_TWO);
                    events.DelayEvents(8000);
                    DoCast(me, SPELL_THUNDERING_STOMP);
                    events.ScheduleEvent(EVENT_JUMP, 1500);
                    return;
                }

                if (events.IsInPhase(PHASE_TWO) && !HealthAbovePct(33))
                {
                    events.SetPhase(PHASE_THREE);
                    events.DelayEvents(8000);
                    DoCast(me, SPELL_THUNDERING_STOMP);
                    events.ScheduleEvent(EVENT_JUMP, 1500);
                    return;
                }
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if ((type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE) || id != POINT_FORGE)
                    return;

                if (events.IsInPhase(PHASE_TWO))
                    DoCast(me, SPELL_FORGE_BLADE);
                else if (events.IsInPhase(PHASE_THREE))
                {
                    me->RemoveAurasDueToSpell(SPELL_FORGE_BLADE);
                    DoCast(me, SPELL_FORGE_MACE);
                }
                events.ScheduleEvent(EVENT_RESUME_ATTACK, 5000);
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell) override
            {
                if (spell->Id == SPELL_PERMAFROST_HELPER)
                {
                    if (Aura *aura = target->GetAura(SPELL_PERMAFROST_HELPER))
                        _permafrostStack = std::max<uint32>(_permafrostStack, aura->GetStackAmount());
                }
                else if (spell->Id == SPELL_FORGE_BLADE)
                    SetEquipmentSlots(false, EQUIP_ID_SWORD);
                else if (spell->Id == SPELL_FORGE_MACE)
                    SetEquipmentSlots(false, EQUIP_ID_MACE);
            }

            uint32 GetData(uint32 /*type*/) const override
            {
                return _permafrostStack;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_THROW_SARONITE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_THROW_SARONITE);
                            Talk(SAY_THROW_SARONITE);
                            events.ScheduleEvent(EVENT_THROW_SARONITE, urand(12500, 20000));
                            break;
                        case EVENT_CHILLING_WAVE:
                            DoCast(me, SPELL_CHILLING_WAVE);
                            events.ScheduleEvent(EVENT_CHILLING_WAVE, 40000, 0, PHASE_TWO);
                            break;
                        case EVENT_DEEP_FREEZE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_DEEP_FREEZE);
                            Talk(SAY_CAST_DEEP_FREEZE);
                            events.ScheduleEvent(EVENT_DEEP_FREEZE, 35000, 0, PHASE_THREE);
                            break;
                        case EVENT_JUMP:
                            me->StopAttack();
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                            if (events.IsInPhase(PHASE_TWO))
                                me->GetMotionMaster()->MoveJump(northForgePos.GetPositionX(), northForgePos.GetPositionY(), northForgePos.GetPositionZ(), 25.0f, 15.0f, POINT_FORGE);
                            else if (events.IsInPhase(PHASE_THREE))
                                me->GetMotionMaster()->MoveJump(southForgePos.GetPositionX(), southForgePos.GetPositionY(), southForgePos.GetPositionZ(), 25.0f, 15.0f, POINT_FORGE);
                            break;
                        case EVENT_RESUME_ATTACK:
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
                            if (events.IsInPhase(PHASE_TWO))
                                events.ScheduleEvent(EVENT_CHILLING_WAVE, 5000, 0, PHASE_TWO);
                            else if (events.IsInPhase(PHASE_THREE))
                                events.ScheduleEvent(EVENT_DEEP_FREEZE, 10000, 0, PHASE_THREE);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

        private:
            uint32 _permafrostStack;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetAIForInstance<boss_garfrostAI>(creature, PoSScriptName);
        }
};

class spell_garfrost_permafrost : public SpellScriptLoader
{
    public:
        spell_garfrost_permafrost() : SpellScriptLoader("spell_garfrost_permafrost") {}

        class spell_garfrost_permafrost_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_garfrost_permafrost_SpellScript);

            bool prevented = false;

            void PreventHitByLoS()
            {
                if (Unit* target = GetHitUnit())
                {
                    Unit* caster = GetCaster();
                    //Temporary Line of Sight Check
                    std::list<GameObject*> blockList;
                    caster->GetGameObjectListWithEntryInGrid(blockList, GO_SARONITE_ROCK, 100.0f);
                    if (!blockList.empty())
                    {
                        for (std::list<GameObject*>::const_iterator itr = blockList.begin(); itr != blockList.end(); ++itr)
                        {
                            if (!(*itr)->IsInvisibleDueToDespawn())
                            {
                                if ((*itr)->IsInBetween(caster, target, 4.0f))
                                {
                                    prevented = true;
                                    target->ApplySpellImmune(GetSpellInfo()->Id, IMMUNITY_ID, GetSpellInfo()->Id, true);
                                    PreventHitDefaultEffect(EFFECT_0);
                                    PreventHitDefaultEffect(EFFECT_1);
                                    PreventHitDefaultEffect(EFFECT_2);
                                    PreventHitDamage();
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            void RestoreImmunity()
            {
                if (Unit* target = GetHitUnit())
                {
                    target->ApplySpellImmune(GetSpellInfo()->Id, IMMUNITY_ID, GetSpellInfo()->Id, false);
                    if (prevented)
                        PreventHitAura();
                }
            }

            void Register() override
            {
                BeforeHit += SpellHitFn(spell_garfrost_permafrost_SpellScript::PreventHitByLoS);
                AfterHit += SpellHitFn(spell_garfrost_permafrost_SpellScript::RestoreImmunity);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_garfrost_permafrost_SpellScript();
        }
};

class achievement_doesnt_go_to_eleven : public AchievementCriteriaScript
{
public:
    achievement_doesnt_go_to_eleven() : AchievementCriteriaScript("achievement_doesnt_go_to_eleven") {}

    bool OnCheck(Player* /*source*/, Unit* target) override
    {
        if (target)
            if (Creature* garfrost = target->ToCreature())
                if (garfrost->AI()->GetData(ACHIEV_DOESNT_GO_TO_ELEVEN) <= 10)
                    return true;

        return false;
    }
};


class npc_martin_gorkun : public CreatureScript
{
public:
    npc_martin_gorkun() : CreatureScript("npc_martin_gorkun") {}

    struct npc_martin_gorkunAI: public ScriptedAI
    {
        npc_martin_gorkunAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        bool m_bIsOutro;
        uint8 m_uiOutro_Phase;
        uint8 m_uiTyr_Phase;
        uint32 m_uiSpeech_Timer;
        uint32 creatureEntry;

        void Reset() override
        {
            m_uiOutro_Phase     = 0;
            m_uiTyr_Phase       = 0;
            m_uiSpeech_Timer    = 1000;
            m_bIsOutro          = true;
            creatureEntry = me->GetEntry();
        }

        void SummonHordeSlaves()
        {
            for (uint8 i = 0; i < 5; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_1, SummonLoc[0].x + urand(0, 20), SummonLoc[0].y + urand(0, 20), SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc[0].x + urand(0, 20), MoveLoc[0].y + urand(0, 20), MoveLoc[0].z);
            }

            for (uint8 i = 5; i < 10; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_2, SummonLoc[1].x + urand(0, 10), SummonLoc[1].y - urand(0, 10), SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + urand(0, 20), MoveLoc[2].y - urand(0, 20), MoveLoc[2].z);
            }

            for (uint8 i = 10; i < 15; i++)
            {
                Creature* pTemp = me->SummonCreature(NPC_SLAVE_HORDE_3, SummonLoc[2].x - urand(0, 20), SummonLoc[2].y - urand(0, 20), SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc[1].x - urand(0, 20), MoveLoc[1].y - urand(0, 20), MoveLoc[1].z);
            }
        }

        void SummonAlySlaves()
        {
            for (uint8 i = 0; i < 5; i++)
            {
                Creature *pTemp = me->SummonCreature(NPC_SLAVE_ALY_1, SummonLoc[0].x + urand(0, 20), SummonLoc[0].y + urand(0, 20), SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc[0].x + urand(0, 20), MoveLoc[0].y + urand(0, 20), MoveLoc[0].z);
            }
    
            for (uint8 i = 5; i < 10; i++)
            {
                Creature *pTemp = me->SummonCreature(NPC_SLAVE_ALY_2, SummonLoc[1].x + urand(0, 10), SummonLoc[1].y - urand(0, 10), SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + urand(0, 20), MoveLoc[2].y - urand(0, 20), MoveLoc[2].z);
            }
    
            for (uint8 i = 10; i < 15; i++)
            {
                Creature *pTemp = me->SummonCreature(NPC_SLAVE_ALY_3, SummonLoc[2].x - urand(0, 20), SummonLoc[2].y - urand(0, 20), SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pTemp)
                    pTemp->GetMotionMaster()->MovePoint(0, MoveLoc[1].x - urand(0, 20), MoveLoc[1].y - urand(0, 20), MoveLoc[1].z);
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (m_bIsOutro && instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED)
            {
                if (m_uiSpeech_Timer < uiDiff)
                {
                    switch(m_uiOutro_Phase)
                    {
                        case 0:
                            switch (creatureEntry)
                            {
                                case NPC_MARTIN_VICTUS_1:
                                    SummonAlySlaves();
                                    break;
                                case NPC_GORKUN_IRONSKULL_1:
                                    SummonHordeSlaves();
                                    break;
                                default:
                                    break;
                            }
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 2000;
                            break;
                        case 1:
                            switch (creatureEntry)
                            {
                                case NPC_MARTIN_VICTUS_1:
                                    Talk(0);
                                    break;
                                case NPC_GORKUN_IRONSKULL_1:
                                    Talk(0);
                                    break;
                                default:
                                    break;
                            }
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 3000;
                            break;
                        case 2:
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 3000;
                            break;
                        case 3:
                            m_bIsOutro = false;
                            ++m_uiOutro_Phase;
                            m_uiSpeech_Timer = 10000;
                            break;
                        default:
                            m_uiSpeech_Timer = 100000;
                    }
                }else m_uiSpeech_Timer -= uiDiff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_martin_gorkunAI(creature);
    }    
};

void AddSC_boss_garfrost()
{
    new boss_garfrost();
    new spell_garfrost_permafrost();
    new achievement_doesnt_go_to_eleven();
    //new npc_martin_gorkun();
}
