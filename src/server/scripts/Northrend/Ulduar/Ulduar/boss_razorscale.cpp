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

//TODO: Harpoon event is automated needs to be checked

/* ScriptData
SDName: Razorscale
SDAuthor: PrinceCreed
SD%Complete: 100
EndScriptData */

#include "ulduar.h"

enum Says
{
    SAY_GREET                       = -1603260,
    SAY_GROUND_PHASE                = -1603261,
    SAY_AGGRO_1                     = -1603262,
    SAY_AGGRO_2                     = -1603263,
    SAY_AGGRO_3                     = -1603264,
    SAY_TURRETS                     = -1603265,
    EMOTE_HARPOON                   = -1603266,
    EMOTE_BREATH                    = -1603267,
    EMOTE_PERMA                     = -1603268,
};

#define GOSSIP_ITEM_1               "We are ready to fight!"

enum Spells
{
    SPELL_FLAMEBUFFET               = 64016,
    SPELL_FIREBALL                  = 62796,
    SPELL_FLAME_GROUND              = 64709,
    SPELL_WINGBUFFET                = 62666,
    SPELL_FLAMEBREATH_10            = 63317,
    SPELL_FLAMEBREATH_25            = 64021,
    SPELL_FUSEARMOR                 = 64771,
    SPELL_DEVOURING_FLAME           = 63236,
    SPELL_HARPOON                   = 54933,
    SPELL_FLAMED                    = 62696,
    SPELL_STUN                      = 9032,
    SPELL_BERSERK                   = 47008
};

const Position PosHarpoon[4] =
{
{594.317f, -136.953f, 391.517f, 4.544f},
{577.449f, -136.953f, 391.517f, 4.877f},
{607.726f, -146.857f, 391.517f, 4.041f},
{561.449f, -146.857f, 391.517f, 5.426f}
};

const Position PosEngSpawn = {591.951f, -95.968f, 391.517f, 0};

const Position PosEngRepair[4] =
{
{590.442f, -130.550f, 391.517f, 4.789f},
{574.850f, -133.687f, 391.517f, 4.252f},
{606.567f, -143.369f, 391.517f, 4.434f},
{560.609f, -142.967f, 391.517f, 5.074f}
};

const Position PosDefSpawn[4] =
{
{600.75f, -104.850f, 391.517f, 0},
{596.38f, -110.262f, 391.517f, 0},
{566.47f, -103.633f, 391.517f, 0},
{570.41f, -108.791f, 391.517f, 0}
};

const Position PosDefCombat[4] =
{
{614.975f, -155.138f, 391.517f, 4.154f},
{609.814f, -204.968f, 391.517f, 5.385f},
{563.531f, -201.557f, 391.517f, 4.108f},
{560.231f, -153.677f, 391.517f, 5.403f}
};

const Position RazorFlight = {588.050f, -251.191f, 470.536f, 1.605f};
const Position RazorGround = {586.966f, -175.534f, 391.517f, 1.692f};

enum Mobs
{
    NPC_DARK_RUNE_GUARDIAN          = 33388,
    NPC_DARK_RUNE_SENTINEL          = 33846,
    NPC_DARK_RUNE_WATCHER           = 33453,
    MOLE_MACHINE_TRIGGER            = 33245,
    NPC_COMMANDER                   = 33210,
    NPC_ENGINEER                    = 33287,
    NPC_DEFENDER                    = 33816,
    NPC_HARPOON                     = 33184,
    GOB_MOLE_MACHINE                = 194316
};

enum DarkRuneSpells
{
    // Dark Rune Watcher
    SPELL_CHAIN_LIGHTNING           = 64758,
    SPELL_LIGHTNING_BOLT            = 63809,

    // Dark Rune Guardian
    SPELL_STORMSTRIKE               = 64757,

    // Dark Rune Sentinel
    SPELL_BATTLE_SHOUT              = 46763,
    SPELL_HEROIC_STRIKE             = 45026,
    SPELL_WHIRLWIND                 = 63807,
};

#define ACHIEVEMENT_QUICK_SHAVE     12321

#define ACTION_EVENT_START          1
#define ACTION_GROUND_PHASE         2

enum Phases
{
    PHASE_NULL,
    PHASE_PERMAGROUND,
    PHASE_GROUND,
    PHASE_FLIGHT
};

enum Events
{
    EVENT_NONE,
    EVENT_BERSERK,
    EVENT_BREATH,
    EVENT_BUFFET,
    EVENT_HARPOON,
    EVENT_FIREBALL,
    EVENT_FLIGHT,
    EVENT_DEVOURING,
    EVENT_FLAME,
    EVENT_LAND,
    EVENT_GROUND,
    EVENT_FUSE,
    EVENT_SUMMON
};

//33186
class boss_razorscale : public CreatureScript
{
public:
    boss_razorscale() : CreatureScript("boss_razorscale") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_razorscaleAI (pCreature);
    }

    struct boss_razorscaleAI : public BossAI
    {
        boss_razorscaleAI(Creature *pCreature) : BossAI(pCreature, BOSS_RAZORSCALE), phase(PHASE_NULL)
        {
            // Do not let Razorscale be affected by Battle Shout buff
            me->ApplySpellImmune(0, IMMUNITY_ID, (SPELL_BATTLE_SHOUT), true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        Phases phase;

        uint32 EnrageTimer;
        uint32 FlyCount;

        Creature* Harpoon[4];
        bool PermaGround;
        bool Enraged;
        bool ApplyShave;
        bool FailAchiv;

        void Reset() override
        {
            _Reset();
            me->SetCanFly(true);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            PermaGround = false;
            ApplyShave = true;
            FailAchiv = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            //for (uint8 n = 0; n < 2; ++n)
                //Harpoon[n] = me->SummonCreature(NPC_HARPOON, PosHarpoon[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 0);
            me->SetSpeed(MOVE_FLIGHT, 3.0f, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_PASSIVE);
            phase = PHASE_GROUND;
            events.SetPhase(PHASE_GROUND);
            FlyCount = 0;
            EnrageTimer = 15*60*1000;   // Enrage in 15 min
            Enraged = false;
            events.ScheduleEvent(EVENT_FLIGHT, 0, 0, PHASE_GROUND);
        }

        bool isApplyShave()
        {
            return ApplyShave;
        }

        void JustDied(Unit* /*Killer*/) override
        {
            _JustDied();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (!FailAchiv && FlyCount > 2)
            {
                ApplyShave = false;
                FailAchiv = true;
            }

            if (me->getVictim() && !me->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
                me->Kill(me->getVictim());

            events.Update(diff);

            if (HealthBelowPct(50) && !PermaGround)
                EnterPermaGround();

            if (EnrageTimer <= diff && !Enraged)
            {
                DoCast(me, SPELL_BERSERK);
                Enraged = true;
            }
            else EnrageTimer -= diff;

            if (phase == PHASE_GROUND)
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_FLIGHT:
                            phase = PHASE_FLIGHT;
                            events.SetPhase(PHASE_FLIGHT);
                            me->SetCanFly(true);
                            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->RemoveAllAuras();
                            me->GetMotionMaster()->MovePoint(0,RazorFlight);
                            events.ScheduleEvent(EVENT_FIREBALL, 7000, 0, PHASE_FLIGHT);
                            events.ScheduleEvent(EVENT_DEVOURING, 10000, 0, PHASE_FLIGHT);
                            events.ScheduleEvent(EVENT_SUMMON, 5000, 0, PHASE_FLIGHT);
                            events.ScheduleEvent(EVENT_GROUND, 75000, 0, PHASE_FLIGHT);
                            ++FlyCount;
                            return;
                        case EVENT_LAND:
                            me->SetCanFly(false);
                            me->NearTeleportTo(586.966f, -175.534f, 391.517f, 1.692f);
                            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            DoCast(me, SPELL_STUN, true);
                            if (Creature *pCommander = me->GetCreature(*me, instance->GetGuidData(DATA_EXP_COMMANDER)))
                                pCommander->AI()->DoAction(ACTION_GROUND_PHASE);
                            //events.ScheduleEvent(EVENT_HARPOON, 0, 0, PHASE_GROUND);
                            events.ScheduleEvent(EVENT_BREATH, 32000, 0, PHASE_GROUND);
                            events.ScheduleEvent(EVENT_BUFFET, 37000, 0, PHASE_GROUND);
                            events.ScheduleEvent(EVENT_FLIGHT, 41000, 0, PHASE_GROUND);
                            return;
                        case EVENT_HARPOON:
                            for (uint8 n = 0; n < 2; ++n)
                                if (Harpoon[n])
                                    Harpoon[n]->CastSpell(me, SPELL_HARPOON, true);
                            events.ScheduleEvent(EVENT_HARPOON, 1500, 0, PHASE_GROUND);
                            return;
                        case EVENT_BREATH:
                            me->MonsterTextEmote(EMOTE_BREATH, ObjectGuid::Empty, true);
                            DoCast(SPELL_FLAMEBREATH_10);
                            //events.CancelEvent(EVENT_HARPOON);
                            events.CancelEvent(EVENT_BREATH);
                            return;
                        case EVENT_BUFFET:
                            DoCast(SPELL_WINGBUFFET);
                           // for (uint8 n = 0; n < 2; ++n)
                                //if (Harpoon[n])
                                    //Harpoon[n]->CastSpell(Harpoon[n], SPELL_FLAMED, true);
                            //events.CancelEvent(EVENT_HARPOON);
                            events.CancelEvent(EVENT_BUFFET);
                            return;
                    }
                }
            }
            if (phase == PHASE_PERMAGROUND)
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_FLAME:
                            DoCast(SPELL_FLAMEBUFFET);
                            events.ScheduleEvent(EVENT_FLAME, 10000, 0, PHASE_PERMAGROUND);
                            return;
                        case EVENT_BREATH:
                            me->MonsterTextEmote(EMOTE_BREATH, ObjectGuid::Empty, true);
                            DoCast(SPELL_FLAMEBREATH_10);
                            events.ScheduleEvent(EVENT_BREATH, 20000, 0, PHASE_PERMAGROUND);
                            return;
                        case EVENT_FIREBALL:
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                                DoCast(pTarget, SPELL_FIREBALL);
                            events.ScheduleEvent(EVENT_FIREBALL, 3000, 0, PHASE_PERMAGROUND);
                            return;
                        case EVENT_DEVOURING:
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                                DoCast(pTarget, SPELL_DEVOURING_FLAME);
                            events.ScheduleEvent(EVENT_DEVOURING, 10000, 0, PHASE_PERMAGROUND);
                            return;
                        case EVENT_BUFFET:
                            DoCast(SPELL_WINGBUFFET);
                            events.CancelEvent(EVENT_BUFFET);
                            return;
                        case EVENT_FUSE:
                            DoCastVictim(SPELL_FUSEARMOR);
                            events.ScheduleEvent(EVENT_FUSE, 10000, 0, PHASE_PERMAGROUND);
                            return;
                    }
                }

                DoMeleeAttackIfReady();
            }
            else
            {
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_GROUND:
                            phase = PHASE_GROUND;
                            events.SetPhase(PHASE_GROUND);
                            //if (Harpoon[0])
                              //  Harpoon[0]->MonsterTextEmote(EMOTE_HARPOON, 0, true);
                            me->GetMotionMaster()->MovePoint(0,RazorGround);
                            events.ScheduleEvent(EVENT_LAND, 5500, 0, PHASE_GROUND);
                            return;
                        case EVENT_FIREBALL:
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                                DoCast(pTarget, SPELL_FIREBALL);
                            events.ScheduleEvent(EVENT_FIREBALL, 3000, 0, PHASE_FLIGHT);
                            return;
                        case EVENT_DEVOURING:
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                                DoCast(pTarget, SPELL_DEVOURING_FLAME);
                            events.ScheduleEvent(EVENT_DEVOURING, 10000, 0, PHASE_FLIGHT);
                            return;
                        case EVENT_SUMMON:
                            SummonAdds();
                            events.ScheduleEvent(EVENT_SUMMON, 45000, 0, PHASE_FLIGHT);
                            return;
                    }
                }
            }
        }

        void EnterPermaGround()
        {
            me->MonsterTextEmote(EMOTE_PERMA, ObjectGuid::Empty, true);
            phase = PHASE_PERMAGROUND;
            events.SetPhase(PHASE_PERMAGROUND);
            me->NearTeleportTo(586.966f, -175.534f, 391.517f, 1.692f);
            me->SetCanFly(false);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveAurasDueToSpell(SPELL_STUN);
            me->SetSpeed(MOVE_FLIGHT, 1.0f, true);
            PermaGround = true;
            DoCast(SPELL_FLAMEBREATH_10);
            events.ScheduleEvent(EVENT_FLAME, 15000, 0, PHASE_PERMAGROUND);
            events.RescheduleEvent(EVENT_DEVOURING, 15000, 0, PHASE_PERMAGROUND);
            events.RescheduleEvent(EVENT_BREATH, 20000, 0, PHASE_PERMAGROUND);
            events.RescheduleEvent(EVENT_FIREBALL, 3000, 0, PHASE_PERMAGROUND);
            events.RescheduleEvent(EVENT_DEVOURING, 6000, 0, PHASE_PERMAGROUND);
            events.RescheduleEvent(EVENT_BUFFET, 2500, 0, PHASE_PERMAGROUND);
            events.RescheduleEvent(EVENT_FUSE, 5000, 0, PHASE_PERMAGROUND);
        }

        void SummonAdds()
        {
            // Adds will come in waves from mole machines. One mole can spawn a Dark Rune Watcher
            // with 1-2 Guardians, or a lone Sentinel. Up to 4 mole machines can spawn adds at any given time.
            uint8 random = urand(1,4);
            for (uint8 i = 0; i < random; ++i)
            {
                float x = float(irand(540, 640));       // Safe range is between 500 and 650
                float y = float(irand(-230, -195));     // Safe range is between -235 and -145
                float z = 391.5f;                       // Ground level
                me->SummonCreature(MOLE_MACHINE_TRIGGER, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 10000);
            }
        }

        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case ACTION_EVENT_START:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat();
                    break;
            }
        }
    };

};

/*====================================================================================
====================================================================================*/

class npc_expedition_commander : public CreatureScript
{
public:
    npc_expedition_commander() : CreatureScript("npc_expedition_commander") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch(uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF:
                if (pPlayer)
                    pPlayer->CLOSE_GOSSIP_MENU();
                CAST_AI(npc_expedition_commanderAI, (pCreature->AI()))->uiPhase = 1;
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature) override
    {
        InstanceScript* instance = pCreature->GetInstanceScript();
        if (instance && instance->GetBossState(BOSS_RAZORSCALE) == NOT_STARTED && pPlayer)
        {
            pPlayer->PrepareGossipMenu(pCreature);

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT,GOSSIP_ITEM_1,GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(13853, pCreature->GetGUID());
        }
        else pPlayer->SEND_GOSSIP_MENU(13910, pCreature->GetGUID());

        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_expedition_commanderAI (pCreature);
    }

    struct npc_expedition_commanderAI : public ScriptedAI
    {
        npc_expedition_commanderAI(Creature* pCreature) : ScriptedAI(pCreature), summons(me)
        {
            instance = pCreature->GetInstanceScript();
            greet = false;
        }

        InstanceScript* instance;
        SummonList summons;

        bool greet;
        uint32 uiTimer;
        uint8  uiPhase;
        ObjectGuid engineerGuid[4];
        ObjectGuid defenderGuid[4];

        void Reset() override
        {
            uiTimer = 0;
            uiPhase = 0;
            greet = false;
        }

        void MoveInLineOfSight(Unit *who) override
        {
            if (!greet && me->IsWithinDistInMap(who, 10.0f) && who->GetTypeId() == TYPEID_PLAYER)
            {
                DoScriptText(SAY_GREET, me);
                greet = true;
            }
        }

        void JustSummoned(Creature *summon) override
        {
            summons.Summon(summon);
        }

        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case ACTION_GROUND_PHASE:
                    DoScriptText(SAY_GROUND_PHASE, me);
                    break;
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            ScriptedAI::UpdateAI(uiDiff);
            if (uiTimer <= uiDiff)
            {
                switch(uiPhase)
                {
                    case 1:
                        instance->SetBossState(BOSS_RAZORSCALE, IN_PROGRESS);
                        summons.DespawnAll();
                        uiTimer = 1000;
                        uiPhase = 2;
                        break;
                    case 2:
                        for (uint8 n = 0; n < 2; ++n)
                        {
                            if (Creature* engineer = me->SummonCreature(NPC_ENGINEER, PosEngSpawn, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                            {
                                if (n == 0)
                                    engineer->MonsterYell(SAY_AGGRO_3, LANG_UNIVERSAL, ObjectGuid::Empty);

                                engineerGuid[n] = engineer->GetGUID();
                                engineer->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                engineer->SetSpeed(MOVE_RUN, 0.5f);
                                engineer->SetHomePosition(PosEngRepair[n]);
                                engineer->GetMotionMaster()->MoveTargetedHome();
                            }
                        }
                        uiPhase = 3;
                        uiTimer = 14000;
                        break;
                    case 3:
                        for (uint8 n = 0; n < 4; ++n)
                        {
                            if (Creature* defender = me->SummonCreature(NPC_DEFENDER, PosDefSpawn[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
                            {
                                defenderGuid[n] = defender->GetGUID();
                                defender->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                defender->SetHomePosition(PosDefCombat[n]);
                                defender->GetMotionMaster()->MoveTargetedHome();
                            }
                        }
                        uiPhase = 4;
                        break;
                    case 4:
                        // Disable, crashed http://pastebin.com/RFR5bsAt
                        for (uint8 n = 0; n < 2; ++n)
                            if (Creature* engineer = ObjectAccessor::GetCreature(*me, engineerGuid[n]))
                                engineer->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_USE_STANDING);
                        for (uint8 n = 0; n < 4; ++n)
                            if (Creature* defender = ObjectAccessor::GetCreature(*me, defenderGuid[n]))
                                defender->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_READY2H);
                        me->MonsterYell(SAY_AGGRO_2, LANG_UNIVERSAL, ObjectGuid::Empty);
                        uiTimer = 16000;
                        uiPhase = 5;
                        break;
                    case 5:
                        if (Creature *pRazorscale = me->GetCreature(*me, instance->GetGuidData(DATA_RAZORSCALE)))
                            pRazorscale->AI()->DoAction(ACTION_EVENT_START);
                        if (Creature* engineer = ObjectAccessor::GetCreature(*me, engineerGuid[0]))
                            engineer->MonsterYell(SAY_AGGRO_1, LANG_UNIVERSAL, ObjectGuid::Empty);
                        uiPhase = 0;
                        break;
                }
            }
            else uiTimer -= uiDiff;
        }
    };

};


class npc_mole_machine_trigger : public CreatureScript
{
public:
    npc_mole_machine_trigger() : CreatureScript("npc_mole_machine_trigger") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_mole_machine_triggerAI (pCreature);
    }

    struct npc_mole_machine_triggerAI : public Scripted_NoMovementAI
    {
        npc_mole_machine_triggerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetVisible(false);
        }

        GameObject* MoleMachine;
        uint32 SummonTimer;

        void Reset() override
        {
            MoleMachine = me->SummonGameObject(GOB_MOLE_MACHINE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(),
                float(urand(0, 6)), 0, 0, 0, 0, 300);
            if (MoleMachine)
                MoleMachine->SetGoState(GO_STATE_ACTIVE);
            SummonTimer = 6000;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (SummonTimer <= uiDiff)
            {
                float x = me->GetPositionX();
                float y = me->GetPositionY();
                float z = me->GetPositionZ();

                // One mole can spawn a Dark Rune Watcher with 1-2 Guardians, or a lone Sentinel
                if (!(rand()%2))
                {
                    me->SummonCreature(NPC_DARK_RUNE_WATCHER, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);
                    uint8 random = urand(1,2);
                    for (uint8 i = 0; i < random; ++i)
                        me->SummonCreature(NPC_DARK_RUNE_GUARDIAN, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);
                }
                else me->SummonCreature(NPC_DARK_RUNE_SENTINEL, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);

                SummonTimer = 15000;
            }
            else SummonTimer -= uiDiff;
        }

        void JustSummoned(Creature *summon) override
        {
            summon->AI()->DoZoneInCombat();
        }
    };

};


class npc_devouring_flame : public CreatureScript
{
public:
    npc_devouring_flame() : CreatureScript("npc_devouring_flame") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_devouring_flameAI (pCreature);
    }

    struct npc_devouring_flameAI : public Scripted_NoMovementAI
    {
        npc_devouring_flameAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetDisplayId(11686);
        }

        void Reset() override
        {
            DoCast(me, SPELL_FLAME_GROUND);
        }
    };

};

//33453
class npc_darkrune_watcher : public CreatureScript
{
public:
    npc_darkrune_watcher() : CreatureScript("npc_darkrune_watcher") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_darkrune_watcherAI (pCreature);
    }

    struct npc_darkrune_watcherAI : public ScriptedAI
    {
        npc_darkrune_watcherAI(Creature *pCreature) : ScriptedAI(pCreature){}

        bool forceCombat = false;
        uint32 ChainTimer;
        uint32 LightTimer;

        void Reset() override
        {
            if (!forceCombat)
            {
                forceCombat = true;
                DoZoneInCombat(me, 100.0f);
            }

            ChainTimer = urand(10000, 15000);
            LightTimer = urand(1000, 3000);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (ChainTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CHAIN_LIGHTNING);
                ChainTimer = urand(10000, 15000);
            }
            else ChainTimer -= uiDiff;

            if (LightTimer <= uiDiff)
            {
                DoCastVictim(SPELL_LIGHTNING_BOLT);
                LightTimer = urand(5000, 7000);
            }
            else LightTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

};

//33388
class npc_darkrune_guardian : public CreatureScript
{
public:
    npc_darkrune_guardian() : CreatureScript("npc_darkrune_guardian") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_darkrune_guardianAI (pCreature);
    }

    struct npc_darkrune_guardianAI : public ScriptedAI
    {
        npc_darkrune_guardianAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        bool forceCombat = false;
        uint32 StormTimer = 0;
        InstanceScript* instance;

        void Reset() override
        {
            if (!forceCombat)
            {
                forceCombat = true;
                DoZoneInCombat(me, 100.0f);
            }
            StormTimer = urand(3000, 6000);
        }
        
        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_FLAMEBREATH_10 || spell->Id == SPELL_FLAMEBREATH_25)
            {
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 226732);
                me->Kill(me);
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (StormTimer)
            {
                if (StormTimer <= uiDiff)
                {
                    DoCastVictim(SPELL_STORMSTRIKE);
                    StormTimer = urand(4000, 8000);
                }
                else StormTimer -= uiDiff;
            }

            DoMeleeAttackIfReady();
        }
    };

};

//33846
class npc_darkrune_sentinel : public CreatureScript
{
public:
    npc_darkrune_sentinel() : CreatureScript("npc_darkrune_sentinel") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_darkrune_sentinelAI (pCreature);
    }

    struct npc_darkrune_sentinelAI : public ScriptedAI
    {
        npc_darkrune_sentinelAI(Creature *pCreature) : ScriptedAI(pCreature){}

        bool forceCombat = false;
        uint32 HeroicTimer;
        uint32 WhirlTimer;
        uint32 ShoutTimer;

        void Reset() override
        {
            if (!forceCombat)
            {
                forceCombat = true;
                DoZoneInCombat(me, 100.0f);
            }

            HeroicTimer = urand(4000, 8000);
            WhirlTimer = urand(20000, 25000);
            ShoutTimer = urand(15000, 30000);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (HeroicTimer <= uiDiff)
            {
                DoCastVictim(SPELL_HEROIC_STRIKE);
                HeroicTimer = urand(4000, 6000);
            }
            else HeroicTimer -= uiDiff;

            if (WhirlTimer <= uiDiff)
            {
                DoCastVictim(SPELL_WHIRLWIND);
                WhirlTimer = urand(20000, 25000);
            }
            else WhirlTimer -= uiDiff;

            if (ShoutTimer <= uiDiff)
            {
                DoCast(me, SPELL_BATTLE_SHOUT);
                ShoutTimer = urand(30000, 40000);
            }
            else ShoutTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

};

//10062, 10063
class achievement_quick_shave_deprecated_disable : public AchievementCriteriaScript
{
    public:
        achievement_quick_shave_deprecated_disable() : AchievementCriteriaScript("achievement_quick_shave_deprecated_disable") {}

        bool OnCheck(Player* player, Unit* target) override
        {
            return false;
        }
};

class achievement_quick_shave : public AchievementCriteriaScript
{
    public:
        achievement_quick_shave() : AchievementCriteriaScript("achievement_quick_shave") {}

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* razorscale = target->ToCreature())
                if (boss_razorscale::boss_razorscaleAI* razorscaleAI = CAST_AI(boss_razorscale::boss_razorscaleAI, razorscale->AI()))
                    if (razorscaleAI->isApplyShave())
                        return true;

            return false;
        }
};

//33104, 33105
class achievement_iron_dwarf_medium_rare : public AchievementCriteriaScript
{
    public:
        achievement_iron_dwarf_medium_rare() : AchievementCriteriaScript("achievement_iron_dwarf_medium_rare") {}

        bool OnCheck(Player* player, Unit* target) override
        {
            //! Deprecated!

            /* if (!target)
                return false;

            if (target->GetEntry() != NPC_DARK_RUNE_GUARDIAN)
                return false;

            if (Creature* dw = target->ToCreature())
                if (npc_darkrune_guardian::npc_darkrune_guardianAI* dwAI = CAST_AI(npc_darkrune_guardian::npc_darkrune_guardianAI, dw->AI()))
                    if (dwAI->IsKilledByRazorscale())
                        return true; */

            return false;
        }
};

void AddSC_boss_razorscale()
{
    new boss_razorscale();
    new npc_expedition_commander();
    new npc_mole_machine_trigger();
    new npc_devouring_flame();
    new npc_darkrune_watcher();
    new npc_darkrune_guardian();
    new npc_darkrune_sentinel();
    new achievement_quick_shave_deprecated_disable();
    new achievement_quick_shave();
    new achievement_iron_dwarf_medium_rare();
}