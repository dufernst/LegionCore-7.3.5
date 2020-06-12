#include "Spell.h"
#include "blackwing_descent.h"


/* 
43404 - maloriak
43396 - nefarian
43407 - atramedes
43402 - flames
43400 - memory fog

69676 - aura like ghost *all
81271 - potion in hand  *maloriak
81184 - fire aura
81178 - memory fog


INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`) VALUES (334803, 43396, 669, 15, 1, 32440, 0, 150.781, -231.196, 75.5367, 2.21657, 7200, 0, 0, 9877580, 947000, 0, 0, 0, 0);
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`) VALUES (334805, 43402, 669, 15, 1, 11686, 0, 166.764, -229.984, 74.9906, 3.12414, 7200, 0, 0, 697410, 62356, 0, 0, 0, 0);
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`) VALUES (334806, 43402, 669, 15, 1, 11686, 0, 149.431, -245.88, 74.9906, 0.593412, 7200, 0, 0, 697410, 62356, 0, 0, 0, 0);
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`) VALUES (334804, 43404, 669, 15, 1, 33186, 0, 149.757, -207.628, 75.5367, 4.04916, 7200, 0, 0, 30062200, 0, 0, 0, 0, 0);
INSERT INTO `creature` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`) VALUES (334802, 43407, 669, 15, 1, 399, 0, 136.076, -207.644, 75.5367, 4.76475, 7200, 0, 0, 42, 0, 0, 0, 0, 0);
*/

enum ScriptTexts
{
    SAY_AGGRO       = 0,
    SAY_KILL        = 1,
    SAY_DEATH       = 2,
    SAY_FLAME       = 5,
    SAY_FLY         = 6,
};

enum Spells
{
    SPELL_MODULATION                = 77612,
    SPELL_SONAR_PULSE_AURA          = 77674,
    SPELL_SONAR_PULSE_DMG           = 77675,
    SPELL_TRACKING                  = 78092,
    SPELL_SONIC_BREATH              = 78098,
    SPELL_SEARING_FLAME_MISSILE     = 77966,
    SPELL_SEARING_FLAME             = 77840,
    SPELL_SEARING_FLAME_1           = 77974,
    SPELL_SEARING_FLAME_DMG         = 77982,
    SPELL_DEVASTATION               = 78875,
    SPELL_DEVASTATION_DMG           = 78868,
    SPELL_RESONATING_CLASH          = 77611,
    SPELL_RESONATING_CLASH_1        = 77709,
    SPELL_RESONATING_CLASH_2        = 78168,
    SPELL_VERTIGO                   = 77717,
    SPELL_SONIC_FLAMES              = 78864,
    SPELL_SONIC_FLAMES_DMG          = 77782,
    SPELL_SONAR_BOMB                = 92557,
    SPELL_SONAR_BOMB_DMG            = 92553,
    SPELL_SONAR_FIREBALL_T          = 78030,
    SPELL_SONAR_FIREBALL            = 78115,
    SPELL_NOISY                     = 78897,
    SPELL_ROARING_FLAME             = 78221,
    SPELL_ROARING_FLAME_DMG         = 78353,
    SPELL_ROARING_FLAME_AURA        = 78018,
    SPELL_ROARING_FLAME_AURA_DMG    = 78023,
};

enum Adds
{
    NPC_SONAR_PULSE                 = 41546,
    NPC_TRACKING_FLAMES             = 41879,
    NPC_ROARING_FLAME               = 41807,
    NPC_ROARING_FLAME_TARGET        = 42121,
    NPC_ABNOXIOUS_FIEND             = 49740,
    NPC_LORD_VICTOR_NEFARIUS_A      = 43396, // �� ������

    NPC_IMP_PORTAL_STALKER          = 49801,
    NPC_BLIND_DRAGON_TAIL           = 42356,
    NPC_REFEBRIATING_FLAME          = 42345,

    NPC_ATRAMEDES_WHELP             = 43407,
    NPC_MALORIAK_1                  = 43404,
    NPC_LORD_VICTOR_NEFARIUS_1      = 49580,
};

enum Actions
{
    ACTION_SHIELD                   = 1,
    ACTION_FLAME                    = 2,
    ACTION_SHIELD_KILL              = 3,
    ACTION_NEFARIUS_SHIELD          = 4,
    ACTION_ABNOXIOUS_FIEND          = 5,
};

enum Events
{
    EVENT_MODULATION                = 1,
    EVENT_GROUND                    = 4,
    EVENT_FLY                       = 5,
    EVENT_NEXT_SPELL                = 6,
    EVENT_SONAR_PULSE_MOVE          = 7,
    EVENT_SONAR_FIREBALL            = 8,
    EVENT_SONAR_BOMB                = 9,
    EVENT_ROARING_FLAME             = 10,
    EVENT_ROARING_FLAME_SPD         = 11,
    EVENT_DEVASTATION               = 12,
    EVENT_TRACKING_FLAMES_MOVE      = 13,
    EVENT_ABNOXIOUS_FIEND           = 14,
    EVENT_NEFARIUS_SUMMON_1         = 15,
    EVENT_NEFARIUS_SUMMON_2         = 16,
    EVENT_NEFARIUS_SHIELD           = 17,
};

const Position flyPos = {127.94f, -225.10f, 110.45f, 0.0f};
const Position groundPos = {127.94f, -225.10f, 75.45f, 0.0f};
const uint32 dwarvenshieldsEntry[8] = 
{
    41445,
    42958,
    42947,
    42960,
    42949,
    42951,
    42954,
    42956,
};
const Position dwarvenshieldsPos[8] = 
{
    {169.57f, -262.49f, 76.72f, 2.42f},
    {154.70f, -273.64f, 76.64f, 2.18f},
    {130.48f, -282.24f, 76.72f, 1.46f},
    {106.28f, -276.95f, 76.72f, 1.01f},
    {169.69f, -186.16f, 76.72f, 3.89f},
    {152.00f, -173.88f, 76.72f, 4.29f},
    {129.58f, -167.68f, 76.72f, 4.83f},
    {108.62f, -171.25f, 76.72f, 5.02f}
};

const Position atramedesnefariusspawnPos = {96.54f, -220.32f, 94.90f, 0.06f};

Unit* atramedesTarget; // ���� ���� ��������
Creature* atramedesShield; // ��������� �������������� ���
Creature* roaringsummon;
Creature* _shields[8];

//
// ������� ���������� ��� �������� ����:
// pulse, breath, pulse, breath, flame, pulse, breath, pulse
// ���� 80���, ������ �������� ������ 13 ������ ���-�� ������ �����������, ����� ������� ������� ������
//
class boss_atramedes : public CreatureScript
{
public:
    boss_atramedes() : CreatureScript("boss_atramedes") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_atramedesAI (pCreature);
    }

    struct boss_atramedesAI : public BossAI
    {
        boss_atramedesAI(Creature *pCreature) : BossAI(pCreature, DATA_ATRAMEDES), summons(me) 
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

        EventMap events;
        uint8 stage;    //0 - ground, 1 - fly
        uint8 nextspell; // from 0 to 7
        SummonList summons;

        void InitializeAI()
        {
            if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(BDScriptName))
                me->IsAIEnabled = false;
            else if (!me->isDead())
                Reset();
        }

        void Reset()
        {
            _Reset();

            summons.DespawnAll();
            for (uint8 i = 0; i < 8; i++)
                _shields[i] = me->SummonCreature(42949, dwarvenshieldsPos[i]);

            me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 15);
            me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 15);

            stage = 0;
            nextspell = 0;
            events.Reset();
            me->SetCanFly(false);
            SetCombatMovement(true);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
            case ACTION_SHIELD:
                if (stage == 0)
                {
                    me->RemoveAurasDueToSpell(SPELL_SEARING_FLAME_1);
                    me->CastStop();
                    DoCast(me, SPELL_VERTIGO);
                }
                break;
            case ACTION_FLAME:
                events.RescheduleEvent(EVENT_ROARING_FLAME, 5000);
                break;
            case ACTION_SHIELD_KILL:
                if (atramedesShield)
                    DoCast(atramedesShield, SPELL_SONIC_FLAMES, true);
                break;
            }
        }


        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void JustDied(Unit* who)
        {
            _JustDied();
            summons.DespawnAll();
            Talk(SAY_DEATH);
        }

        void KilledUnit(Unit* victim)
        {
            Talk(SAY_KILL);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (me->isInCombat())
                summon->SetInCombatWithZone();
            switch (summon->GetEntry())
            {
            case NPC_TRACKING_FLAMES:
                DoCast(summon, SPELL_SONIC_BREATH);
                break;
            case NPC_ROARING_FLAME_TARGET:
                if (atramedesTarget)
                {
                    summon->GetMotionMaster()->MoveFollow(atramedesTarget, 0.01f, 0.0f);
                    roaringsummon = summon;
                    DoCast(summon, SPELL_ROARING_FLAME);
                }
                break;
            }
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_MODULATION, urand(7000, 10000));
            events.RescheduleEvent(EVENT_NEXT_SPELL, 5000);
            events.RescheduleEvent(EVENT_FLY, 80000);
            Talk(SAY_AGGRO);
            DoZoneInCombat();
            instance->SetBossState(DATA_ATRAMEDES, IN_PROGRESS);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case 1:
                    stage = 1;
                    Talk(SAY_FLY);
                    DoCast(me, SPELL_SONAR_FIREBALL_T);
                    events.RescheduleEvent(EVENT_SONAR_BOMB, 3000);
                    events.RescheduleEvent(EVENT_ROARING_FLAME, 2000);
                    events.RescheduleEvent(EVENT_GROUND, 40000);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                    break;
                case 2:
                    me->SetCanFly(false);
                    SetCombatMovement(true);
                    stage = 0;
                    nextspell = 0;
                    events.RescheduleEvent(EVENT_MODULATION, 8000);
                    events.RescheduleEvent(EVENT_NEXT_SPELL, 5000);
                    events.RescheduleEvent(EVENT_FLY, 80000);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                    me->GetMotionMaster()->MoveChase(me->getVictim());
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                if (!me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MODULATION:
                    DoCast(me, SPELL_MODULATION);
                    events.RescheduleEvent(EVENT_MODULATION, urand(18000, 22000));
                    break;
                case EVENT_FLY:
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                    events.Reset();
                    SetCombatMovement(false);
                    me->SetCanFly(true);
                    me->GetMotionMaster()->MovePoint(1, flyPos);
                    break;
                case EVENT_GROUND:
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                    me->CastStop();
                    summons.DespawnEntry(NPC_ROARING_FLAME_TARGET);
                    me->RemoveAurasDueToSpell(SPELL_SONAR_FIREBALL_T);
                    events.Reset();
                    me->GetMotionMaster()->MovePoint(2, groundPos);
                    break;
                case EVENT_NEXT_SPELL:
                    //�� ���� ���� ����� ���������� 7 �������
                    if (nextspell > 7)
                        break;
                    switch (nextspell)
                    {
                    case 0:
                    case 2:
                    case 5:
                    case 7:
                        for (uint8 i = 0; i < 3; i++)
                            me->SummonCreature(NPC_SONAR_PULSE,
                            me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f);
                        break;
                    case 1:
                    case 3:
                    case 6:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        {
                            me->SummonCreature(NPC_TRACKING_FLAMES,
                                target->GetPositionX(), target->GetPositionY(),
                                target->GetPositionZ(), me->GetOrientation(),
                                TEMPSUMMON_TIMED_DESPAWN, 8000);
                        }
                        break;
                    case 4:
                        Talk(SAY_FLAME);
                        DoCast(SPELL_SEARING_FLAME_1);
                        DoCast(SPELL_SEARING_FLAME);
                        break;
                    }
                    nextspell++;
                    events.RescheduleEvent(EVENT_NEXT_SPELL, 10000);
                    break;
                case EVENT_SONAR_FIREBALL:
                    me->CastSpell(me, SPELL_SONAR_FIREBALL, true);
                    events.RescheduleEvent(EVENT_SONAR_FIREBALL, 10000);
                    break;
                case EVENT_SONAR_BOMB:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        me->CastSpell(target, SPELL_SONAR_BOMB, true);
                    events.RescheduleEvent(EVENT_SONAR_BOMB, 2000);
                    break;
                case EVENT_ROARING_FLAME:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    {
                        atramedesTarget = target;
                        me->SummonCreature(NPC_ROARING_FLAME_TARGET,
                            target->GetPositionX() + urand(5, 9),
                            target->GetPositionY() + urand(5, 9),
                            target->GetPositionZ());
                    }
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_tracking_flames : public CreatureScript
{
public:
    npc_tracking_flames() : CreatureScript("npc_tracking_flames") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_tracking_flamesAI (pCreature);
    }

    struct npc_tracking_flamesAI : public ScriptedAI
    {
        npc_tracking_flamesAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
        }

        void UpdateAI(uint32 diff)
        {
        }
    };
};

class npc_roaring_flame : public CreatureScript
{
public:
    npc_roaring_flame() : CreatureScript("npc_roaring_flame") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_roaring_flameAI (pCreature);
    }

    struct npc_roaring_flameAI : public ScriptedAI
    {
        npc_roaring_flameAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            DoCast(me, SPELL_ROARING_FLAME_AURA);
        }
    };
};

class npc_sonar_pulse : public CreatureScript
{
public:
    npc_sonar_pulse() : CreatureScript("npc_sonar_pulse") {}

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_sonar_pulseAI (pCreature);
    }

    struct npc_sonar_pulseAI : public ScriptedAI
    {
        npc_sonar_pulseAI(Creature* creature) : ScriptedAI(creature) 
        {
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        EventMap events;
        ObjectGuid targetGUID;

        void Reset()
        {
            events.RescheduleEvent(EVENT_SONAR_PULSE_MOVE, 1500);
            DoCast(me, SPELL_SONAR_PULSE_AURA, true);
        }

        void IsSummonedBy(Unit* owner)
        {
            if (owner)
            {
                if (Unit* ownerTarget = owner->ToCreature()->GetAI()->SelectTarget(SELECT_TARGET_RANDOM))
                    targetGUID = ownerTarget->GetGUID();
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (id == 1)
                    me->DespawnOrUnsummon();
            }            
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SONAR_PULSE_MOVE:
                        Position pos;
                        if (!targetGUID.IsEmpty())
                            if (Unit* target = Unit::GetUnit(*me, targetGUID))
                                me->GetNearPosition(pos, 50.0f, me->GetAngle(target->GetPositionX(), target->GetPositionY()));
                        me->GetMotionMaster()->MovePoint(1, pos);
                        break;
                }
            }
        }
    };
};

class npc_roaring_flame_target : public CreatureScript
{
public:
    npc_roaring_flame_target() : CreatureScript("npc_roaring_flame_target") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_roaring_flame_targetAI (pCreature);
    }

    struct npc_roaring_flame_targetAI : public ScriptedAI
    {
        npc_roaring_flame_targetAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetSpeed(MOVE_WALK, 0.8f);
            me->SetSpeed(MOVE_RUN, 0.8f);
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset()
        {
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case 1:
                    if (Creature* atramedes = me->FindNearestCreature(NPC_ATRAMEDES, 100.0f))
                        atramedes->AI()->DoAction(ACTION_FLAME);
                    if (atramedesShield)
                        atramedesShield->DespawnOrUnsummon();
                    me->DespawnOrUnsummon();
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            if (!me->FindNearestCreature(NPC_ROARING_FLAME, 4.0f))
                me->SummonCreature(NPC_ROARING_FLAME,
                me->GetPositionX(),
                me->GetPositionY(),
                me->GetPositionZ(),
                0.0f, TEMPSUMMON_TIMED_DESPAWN, 45000);
        }
    };
};

class npc_atramedes_gong : public CreatureScript
{
public:
    npc_atramedes_gong() : CreatureScript("npc_atramedes_gong") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        InstanceScript* instance = pCreature->GetInstanceScript();
        if (!instance)
            return false;
        if (instance->GetBossState(DATA_ATRAMEDES) != IN_PROGRESS)
            return true;
        if (Creature* atramedes = pCreature->FindNearestCreature(NPC_ATRAMEDES, 200.0f))
        {
            pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            atramedesShield = pCreature;
            atramedesTarget = pPlayer;
            atramedes->AI()->DoAction(ACTION_SHIELD);
        }
        return true;
    }
};

class npc_abnoxious_fiend : public CreatureScript
{
public:
    npc_abnoxious_fiend() : CreatureScript("npc_abnoxious_fiend") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_abnoxious_fiendAI(creature);
    }

    struct npc_abnoxious_fiendAI : public ScriptedAI
    {
        npc_abnoxious_fiendAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        void Reset()
        {
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            if (instance->GetData(DATA_ATRAMEDES) != IN_PROGRESS)
                me->DespawnOrUnsummon();

            DoMeleeAttackIfReady();
        }
    };
};

class spell_atramedes_resonating_clash : public SpellScriptLoader
{
public:
    spell_atramedes_resonating_clash() : SpellScriptLoader("spell_atramedes_resonating_clash") { }

    class spell_atramedes_resonating_clash_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_resonating_clash_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_RESONATING_CLASH, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_resonating_clash_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_resonating_clash_SpellScript();
    }
};

class spell_atramedes_resonating_clash_1 : public SpellScriptLoader
{
public:
    spell_atramedes_resonating_clash_1() : SpellScriptLoader("spell_atramedes_resonating_clash_1") { }

    class spell_atramedes_resonating_clash_1_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_resonating_clash_1_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;
            GetHitUnit()->RemoveAurasDueToSpell(SPELL_NOISY);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_resonating_clash_1_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_resonating_clash_1_SpellScript();
    }
};

class spell_atramedes_vertigo : public SpellScriptLoader
{
public:
    spell_atramedes_vertigo() : SpellScriptLoader("spell_atramedes_vertigo") { }

    class spell_atramedes_vertigo_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_atramedes_vertigo_AuraScript);

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (!GetTarget())
                return;

            GetTarget()->ToCreature()->AI()->DoAction(ACTION_SHIELD_KILL);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_atramedes_vertigo_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_atramedes_vertigo_AuraScript();
    }
};

class spell_atramedes_modulation : public SpellScriptLoader
{
public:
    spell_atramedes_modulation() : SpellScriptLoader("spell_atramedes_modulation") { }

    class spell_atramedes_modulation_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_modulation_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            uint32 power = GetHitUnit()->GetPower(POWER_ALTERNATE);
            GetHitUnit()->SetPower(POWER_ALTERNATE, power + 7);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_modulation_SpellScript::RecalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_modulation_SpellScript();
    }
};

class spell_atramedes_sonar_pulse : public SpellScriptLoader
{
public:
    spell_atramedes_sonar_pulse() : SpellScriptLoader("spell_atramedes_sonar_pulse") { }

    class spell_atramedes_sonar_pulse_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_sonar_pulse_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            uint32 power = GetHitUnit()->GetPower(POWER_ALTERNATE);
            GetHitUnit()->SetPower(POWER_ALTERNATE, power + 3);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_sonar_pulse_SpellScript::RecalculateDamage, EFFECT_0, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_sonar_pulse_SpellScript();
    }
};

class spell_atramedes_roaring_flame : public SpellScriptLoader
{
public:
    spell_atramedes_roaring_flame() : SpellScriptLoader("spell_atramedes_roaring_flame") { }

    class spell_atramedes_roaring_flame_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_roaring_flame_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            uint32 power = GetHitUnit()->GetPower(POWER_ALTERNATE);
            GetHitUnit()->SetPower(POWER_ALTERNATE, power + 3);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_roaring_flame_SpellScript::RecalculateDamage, EFFECT_1, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_roaring_flame_SpellScript();
    }
};

class spell_atramedes_roaring_flame_aura : public SpellScriptLoader
{
public:
    spell_atramedes_roaring_flame_aura() : SpellScriptLoader("spell_atramedes_roaring_flame_aura") { }

    class spell_atramedes_roaring_flame_aura_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_roaring_flame_aura_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            uint32 power = GetHitUnit()->GetPower(POWER_ALTERNATE);
            GetHitUnit()->SetPower(POWER_ALTERNATE, power + 5);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_roaring_flame_aura_SpellScript::RecalculateDamage, EFFECT_2, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_roaring_flame_aura_SpellScript();
    }
};

class spell_atramedes_sonar_bomb : public SpellScriptLoader
{
public:
    spell_atramedes_sonar_bomb() : SpellScriptLoader("spell_atramedes_sonar_bomb") { }

    class spell_atramedes_sonar_bomb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_sonar_bomb_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            uint32 power = GetHitUnit()->GetPower(POWER_ALTERNATE);
            GetHitUnit()->SetPower(POWER_ALTERNATE, power + 20);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_sonar_bomb_SpellScript::RecalculateDamage, EFFECT_1, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_sonar_bomb_SpellScript();
    }
};

class spell_atramedes_sonic_breath : public SpellScriptLoader
{
public:
    spell_atramedes_sonic_breath() : SpellScriptLoader("spell_atramedes_sonic_breath") { }

    class spell_atramedes_sonic_breath_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_atramedes_sonic_breath_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            uint32 power = GetHitUnit()->GetPower(POWER_ALTERNATE);
            GetHitUnit()->SetPower(POWER_ALTERNATE, power + 20);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_atramedes_sonic_breath_SpellScript::RecalculateDamage, EFFECT_1, SPELL_EFFECT_ENERGIZE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_atramedes_sonic_breath_SpellScript();
    }
};

void AddSC_boss_atramedes()
{
    new boss_atramedes();
    new npc_sonar_pulse();
    new npc_tracking_flames();
    new npc_roaring_flame_target();
    new npc_roaring_flame();
    new npc_atramedes_gong();
    new npc_abnoxious_fiend();
    new spell_atramedes_resonating_clash();
    new spell_atramedes_resonating_clash_1();
    new spell_atramedes_vertigo();
    //new spell_atramedes_modulation();
    //new spell_atramedes_sonar_pulse();
    //new spell_atramedes_roaring_flame();
    //new spell_atramedes_roaring_flame_aura();
    //new spell_atramedes_sonar_bomb();
    //new spell_atramedes_sonic_breath();
}