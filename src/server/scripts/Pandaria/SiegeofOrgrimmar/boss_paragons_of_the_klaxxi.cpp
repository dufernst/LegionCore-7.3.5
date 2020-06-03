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

#include "siege_of_orgrimmar.h"
#include "LFG.h"

enum eSpells
{
    //Special spells
    //Palladin
    SPELL_SHIELD_OF_THE_RIGHTEOUS      = 132403,
    //Warrior
    SPELL_SHIELD_BLOCK                 = 132404,
    //Druid
    SPELL_SAVAGE_DEFENSE               = 132402,
    //Monk
    SPELL_ELUSIVE_BREW                 = 115308,
    //Death Knight
    SPELL_BLOOD_SHIELD                 = 77535,
    SPELL_READY_TO_FIGHT               = 143542,
    //

    //Hisek
    SPELL_MULTI_SHOT                   = 144839,
    SPELL_AIM_DUMMY                    = 142948,
    SPELL_AIM_STUN                     = 144759,
    SPELL_FIRE                         = 142950,
    SPELL_RAPID_FIRE_DUMMY             = 143243,
    SPELL_RAPID_FIRE_DMG               = 135815,
    SPELL_SONIC_RESONANCE_HISEK        = 144094,

    //Skeer
    SPELL_HEVER_OF_FOES                = 143273,
    SPELL_BLODDLETTING                 = 143280,
    SPELL_BLOODLETTING_SUM             = 143280,
    SPELL_BLODDLETTING_BUFF            = 143320,

    //Rikkal
    SPELL_PREY                         = 144286,
    SPELL_MAD_SCIENTIST_AURA           = 143277,
    SPELL_INJECTION_SUM                = 143340,
    SPELL_MUTATE                       = 143337,
    SPELL_FAULTY_MUTATION              = 148589,
    SPELL_FAULTY_MUTATION_KILL         = 148587,

    //Iyyokyk
    SPELL_DIMINISH                     = 143666,
    SPELL_INSANE_CALC_FIERY_EDGE       = 142416,
    SPELL_FIERY_EDGE_PRE_DUMMY         = 142811,
    SPELL_FIERY_EDGE_DUMMY             = 142808,
    SPELL_FIERY_EDGE_DMG               = 142809,

    //Kaztik
    SPELL_SONIC_PROJECTION_AT          = 143765,

    //Kilruk
    SPELL_DEATH_FROM_ABOVE             = 142264,
    SPELL_DEATH_FROM_ABOVE_SUM         = 142263,
    SPELL_DEATH_FROM_ABOVE_VISUAL      = 144126,
    SPELL_RAZOR_SHARP_BLADES           = 142918,
    SPELL_GOUGE                        = 143939,
    SPELL_MUTILATE                     = 143941,
    SPELL_REAVE_PRE                    = 148677,
    SPELL_REAVE                        = 148681,

    //Xaril
    SPELL_TENDERIZING_STRIKES          = 142927,
    SPELL_CAUSTIC_BLOOD                = 142315,
    SPELL_BLOODY_EXPLOSION             = 142317,
    SPELL_TOXIC_INJECTION              = 142528,
    //Toxins
    SPELL_DELAYED_CATALYST_RED         = 142936,
    SPELL_DELAYED_CATALYST_BLUE        = 142935,
    SPELL_REACTION_BLUE                = 142735,
    SPELL_DELAYED_CATALYST_YELLOW      = 142937,
    SPELL_CATALYST_YELLOW_AT           = 142737,
    //Heroic Toxins
    SPELL_DELAYED_CATALYST_ORANGE      = 142938,
    SPELL_DELAYED_CATALYST_PURPLE      = 142729,
    SPELL_DELAYED_CATALYST_GREEN       = 142730,

    //Korven
    SPELL_SHIELD_BASH                  = 143974,
    SPELL_VICIOUS_ASSAULT_DMG          = 143980,
    SPELL_VICIOUS_ASSAULT_DMG_2        = 143981,
    SPELL_VICIOUS_ASSAULT_DMG_3        = 143982,
    SPELL_VICIOUS_ASSAULT_DMG_4        = 143984,
    SPELL_VICIOUS_ASSAULT_DMG_5        = 143985,
    SPELL_VICIOUS_ASSAULT_DOT          = 143979,
    SPELL_ENCASE_IN_AMBER              = 142564,
    SPELL_AMBER_VISUAL                 = 144120,
    SPELL_AMBER_REGENERATION           = 142576,

    //Karoz
    SPELL_STORE_KINETIC_ENERGY         = 143709,
    SPELL_FLASH                        = 143704,
    SPELL_FLASH_DUMMY                  = 143700,
    SPELL_HURL_AMBER                   = 143759,
    SPELL_HURL_AMBER_DMG               = 143733,

    //Special
    SPELL_AURA_VISUAL_FS               = 143548, 
    SPELL_AURA_ENRAGE                  = 146983,
    SPELL_ENRAGE                       = 146982,
    SPELL_PARAGONS_PURPOSE_HEAL        = 143483,
    SPELL_PARAGONS_PURPOSE_DMG         = 143482,

    //Amber Parasite
    SPELL_FEED                         = 143362,
    SPELL_REGENERATE                   = 143356,
    SPELL_GENETIC_MOD                  = 143355,
    //Hungry Kunchong
    SPELL_THICK_SHELL                  = 142667,
    SPELL_HUNGRY                       = 142630,
    SPELL_H_DEVOUR                     = 142638,
    SPELL_MOLT                         = 142651,
    SPELL_DEVOUR                       = 142649,
    SPELL_MESMERIZE                    = 142671,
    SPELL_SWIPE                        = 142655,

    //Buffs
    //Kaztik
    SPELL_MASTER_PUPPETS               = 127351,
    //Karoz
    SPELL_STRONG_LEGS                  = 141853,
    SPELL_STRONG_LEGS2                 = 143963,
    //Xaril
    SPELL_VOLATILE_POULTICE_HEAL       = 142897,
    //

    //Purple toxin player effect
    SPELL_CANNED_HEAT                  = 143570,
    //Green toxin player effect
    SPELL_EERIE_FOG_DMG                = 142945,
    SPELL_EERIE_FOG_AT                 = 142812,
};

enum sEvents
{
    EVENT_START_KLAXXI                 = 1,
    EVENT_CHECK                        = 2,
    //Skeer
    EVENT_BLODDLETTING                 = 3,
    //Kilruk
    EVENT_GOUGE                        = 4,
    EVENT_REAVE                        = 5,
    EVENT_DEATH_FROM_ABOVE             = 6,
    EVENT_DEATH_FROM_ABOVE_START       = 7,
    //Hisek
    EVENT_MULTI_SHOT                   = 8,
    EVENT_AIM                          = 9,
    EVENT_RAPID_FIRE                   = 10,
    EVENT_FIRE                         = 11,
    //Rikkal
    EVENT_MUTATE                       = 12,
    EVENT_INJECTION                    = 13,
    //Xaril
    EVENT_TOXIC_INJECTION              = 14,
    EVENT_CATALYST                     = 15,
    //Korven
    EVENT_SHIELD_BASH                  = 16,
    //Iyyokyk
    EVENT_DIMINISH                     = 17,
    EVENT_INSANE_CALCULATION           = 18,
    //Kaztik
    EVENT_SONIC_PROJECTION             = 19,
    EVENT_SUM_HUNGRY_KUNCHONG          = 20,
    //Karoz
    EVENT_HURL_AMBER                   = 21,
    EVENT_FLASH                        = 22,
    //Amber Parasite
    EVENT_FEED                         = 23,
    EVENT_REGENERATE                   = 24,
    //Blood
    EVENT_FIND_LOW_HP_KLAXXI           = 25,
    EVENT_CHECK_DIST_TO_KLAXXI         = 26,
    EVENT_CHECK_PLAYER                 = 27,
    EVENT_RE_ATTACK                    = 28,
    EVENT_PARAGONS_PURPOSE             = 29,
    //Hungry Kunchong
    EVENT_SWIPE                        = 30,
    EVENT_DEVOUR                       = 31,
};

enum sActions
{
    ACTION_KLAXXI_START                = 1,
    ACTION_RE_ATTACK                   = 3,
    ACTION_MOVE_TO_CENTER              = 4,
    ACTION_RE_ATTACK_KILRUK            = 5,
    ACTION_MELEE_PHASE                 = 6,
    ACTION_DEVOUR                      = 7,
};

enum CreatureText
{
    SAY_SKEER_PULL                     = 0,
    SAY_SKEER_BLOOD                    = 1,
    SAY_SKEER_DIE                      = 2,
    SAY_RIKKAL_DIE                     = 3,
    SAY_HISEK_AIM                      = 4,
    SAY_HISEK_DIE                      = 5,
    SAY_KAROZ_PULL                     = 6,
    SAY_KAROZ_FLASH                    = 7,
    SAY_KAROZ_DIE                      = 8,
    SAY_KORVEN_PULL                    = 9,
    SAY_KORVEN_SP                      = 10,
    SAY_KORVEN_SP2                     = 11,
    SAY_KORVEN_DIE                     = 12,
    SAY_IYYOKYK_PULL                   = 13,
    SAY_IYYOKYK_SP                     = 14,
    SAY_IYYOKYK_DIE                    = 15,
    SAY_XARIL_PULL                     = 16,
    SAY_XARIL_SP                       = 17,
    SAY_XARIL_SP2                      = 18,
    SAY_XARIL_DIE                      = 19,
    SAY_KAZTIK_SP                      = 20,
    SAY_KAZTIK_DIE                     = 21,
    SAY_KILRUK_SP                      = 22,
    SAY_KILRUK_SP2                     = 23,
    SAY_KILRUK_DIE                     = 24,
};

uint32 removeaurasentry[4] =
{
    SPELL_READY_TO_FIGHT,
    SPELL_AURA_ENRAGE,
    SPELL_ENRAGE,
    SPELL_PARAGONS_PURPOSE_DMG,
};

uint32 EvadeSpells[5] =
{
    SPELL_SHIELD_OF_THE_RIGHTEOUS,
    SPELL_SHIELD_BLOCK,
    SPELL_SAVAGE_DEFENSE,
    SPELL_ELUSIVE_BREW,
    SPELL_BLOOD_SHIELD,
};

Position bloodsumpos[3] =
{
    { 1591.96f, -5650.64f, -314.7395f, 4.4687f },
    { 1619.42f, -5682.49f, -314.7208f, 3.1806f },
    { 1549.81f, -5705.55f, -314.6497f, 0.5652f },
};

uint32 toxinlist[6] =
{
    SPELL_TOXIN_BLUE,
    SPELL_TOXIN_RED,
    SPELL_TOXIN_YELLOW,
    SPELL_TOXIN_ORANGE,
    SPELL_TOXIN_PURPLE,
    SPELL_TOXIN_GREEN,
};

uint32 catalystlist[6] =
{
    SPELL_DELAYED_CATALYST_RED,
    SPELL_DELAYED_CATALYST_BLUE,
    SPELL_DELAYED_CATALYST_YELLOW,
    SPELL_DELAYED_CATALYST_ORANGE,
    SPELL_DELAYED_CATALYST_PURPLE,
    SPELL_DELAYED_CATALYST_GREEN,
};

Position klaxxispawnpos[9] =
{
    {1648.27f, -5736.53f, -291.0f, 2.34133f},//Hisek
    {1515.46f, -5634.43f, -291.0f, 5.65569f},//Rikkal
    {1549.77f, -5635.94f, -291.0f, 5.31405f},//Skeer
    {1615.19f, -5732.10f, -291.0f, 2.22666f},//Karoz
    {1540.31f, -5762.38f, -298.2f, 0.80514f},//Iyyokyk
    {1606.35f, -5766.56f, -291.0f, 2.00833f},//Korven
    {1519.30f, -5665.30f, -294.4f, 5.85205f},//Kaztik
    {1559.07f, -5603.99f, -291.0f, 5.25514f},//Xaril
    {1494.92f, -5694.08f, -298.4f, 0.24357f},//Kilruk
};

uint32 klaxxientry[9] =
{
    NPC_HISEK,
    NPC_RIKKAL,
    NPC_SKEER,
    NPC_KAROZ,
    NPC_IYYOKYK,
    NPC_KORVEN,
    NPC_KAZTIK,
    NPC_XARIL,
    NPC_KILRUK,
};

//71628
class npc_amber_piece : public CreatureScript
{
public:
    npc_amber_piece() : CreatureScript("npc_amber_piece") {}

    struct npc_amber_pieceAI : public ScriptedAI
    {
        npc_amber_pieceAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_AURA_VISUAL_FS, true);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}

        void OnSpellClick(Unit* clicker)
        {
            if (instance->GetBossState(DATA_KLAXXI) != DONE)
            {
                if (instance->GetBossState(DATA_THOK) == DONE && instance->GetBossState(DATA_BLACKFUSE) == DONE)
                {
                    if (me->HasAura(SPELL_AURA_VISUAL_FS))
                    {
                        me->RemoveAurasDueToSpell(SPELL_AURA_VISUAL_FS);
                        if (Creature* ck = me->GetCreature(*me, instance->GetGuidData(NPC_KLAXXI_CONTROLLER)))
                            ck->AI()->DoAction(ACTION_KLAXXI_START);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_amber_pieceAI(creature);
    }
};

//71592
class npc_klaxxi_controller : public CreatureScript
{
public:
    npc_klaxxi_controller() : CreatureScript("npc_klaxxi_controller") {}

    struct npc_klaxxi_controllerAI : public ScriptedAI
    {
        npc_klaxxi_controllerAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;
        SummonList summons;
        EventMap events;

        void Reset()
        {
            events.Reset();
            DespawnSummons();
            summons.DespawnAll();
            SummonKlaxxi();
        }

        void DespawnSummons()
        {
            std::list<Creature*> sumlist;
            sumlist.clear();
            GetCreatureListWithEntryInGrid(sumlist, me, NPC_AMBER_PARASITE, 150.0f);
            GetCreatureListWithEntryInGrid(sumlist, me, NPC_HUNGRY_KUNCHONG, 150.0f);
            GetCreatureListWithEntryInGrid(sumlist, me, NPC_AMBER, 150.0f);
            GetCreatureListWithEntryInGrid(sumlist, me, NPC_BLOOD, 150.0f);
            GetCreatureListWithEntryInGrid(sumlist, me, NPC_FLASH_STALKER, 150.0f);
            if (!sumlist.empty())
                for (std::list<Creature*>::const_iterator itr = sumlist.begin(); itr != sumlist.end(); itr++)
                    (*itr)->DespawnOrUnsummon();

            std::list<AreaTrigger*> atlist;
            atlist.clear();
            me->GetAreaTriggersWithEntryInRange(atlist, 4850, ObjectGuid::Empty, 100.0f);
            if (!atlist.empty())
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                    (*itr)->RemoveFromWorld();

            instance->SetData(DATA_CLEAR_KLAXXI_LIST, 0);
        }

        void SummonKlaxxi()
        {
            for (uint8 n = 0; n < 9; n++)
                if (Creature* klaxxi = me->FindNearestCreature(klaxxientry[n], 150.0f, true))
                    continue;
                else
                    me->SummonCreature(klaxxientry[n], klaxxispawnpos[n]);
            if (Creature* ap = me->FindNearestCreature(NPC_AMBER_PIECE, 30.0f, true))
                ap->AI()->Reset();
        }

        void JustSummoned(Creature* sum)
        {
            summons.Summon(sum);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_KLAXXI_START:
                instance->SetBossState(DATA_KLAXXI, IN_PROGRESS);
                events.RescheduleEvent(EVENT_START_KLAXXI, 5000);
                break;
            case ACTION_KLAXXI_DONE:
                if (me->GetMap()->IsHeroic())
                    DoCast(me, 148512, true);
                DespawnSummons();
                events.Reset();
                if (Creature* ap = me->GetCreature(*me, instance->GetGuidData(NPC_AMBER_PIECE)))
                    me->Kill(ap, true);
                me->Kill(me, true);
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_START_KLAXXI)
                    instance->SetData(DATA_KLAXXI_START, 0);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_klaxxi_controllerAI(creature);
    }
};

class boss_paragons_of_the_klaxxi : public CreatureScript
{
public:
    boss_paragons_of_the_klaxxi() : CreatureScript("boss_paragons_of_the_klaxxi") {}

    struct boss_paragons_of_the_klaxxiAI : public ScriptedAI
    {
        boss_paragons_of_the_klaxxiAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetDisableGravity(true);
            me->SetCanFly(true);
        }

        InstanceScript* instance;
        SummonList summons;
        EventMap events;
        ObjectGuid jtGuid, dfatargetGuid;
        uint32 checkklaxxi, healcooldown;
        uint8 flashcount;
        bool healready;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
            for (uint8 n = 0; n < 4; n++)
                me->RemoveAurasDueToSpell(removeaurasentry[n]);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            jtGuid.Clear();
            dfatargetGuid.Clear();
            checkklaxxi = 0;
            healcooldown = 0;
            flashcount = 0;
            healready = true;
            switch (me->GetEntry())
            {
            case NPC_SKEER:
            case NPC_RIKKAL:
            case NPC_HISEK:
                DoCast(me, SPELL_READY_TO_FIGHT, true);
                break;
            default:
                break;
            }
        }

        void JustSummoned(Creature* sum)
        {
            if (sum->GetEntry() == 71309)
            {
                sum->SetDisplayId(11686);
                sum->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                jtGuid = sum->GetGUID();
                DoCast(sum, SPELL_DEATH_FROM_ABOVE_VISUAL);
                events.RescheduleEvent(EVENT_DEATH_FROM_ABOVE_START, 2000);
            }
            summons.Summon(sum);
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 300.0f);
            switch (me->GetEntry())
            {
            case NPC_KILRUK:
                DoCast(me, SPELL_RAZOR_SHARP_BLADES, true);
                events.RescheduleEvent(EVENT_GOUGE, 20000);
                events.RescheduleEvent(EVENT_DEATH_FROM_ABOVE, 34000);
                if (me->GetMap()->IsHeroic())
                    events.RescheduleEvent(EVENT_REAVE, 16000);
                break;
            case NPC_XARIL:
                Talk(SAY_XARIL_PULL);
                DoCast(me, SPELL_TENDERIZING_STRIKES, true);
                events.RescheduleEvent(EVENT_TOXIC_INJECTION, me->GetMap()->IsHeroic() ? 20000 : 3000);
                break;
            case NPC_SKEER:
                Talk(SAY_SKEER_PULL);
                DoCast(me, SPELL_HEVER_OF_FOES, true);
                events.RescheduleEvent(EVENT_BLODDLETTING, 10000);
                break;
            case NPC_RIKKAL:
                DoCast(me, SPELL_MAD_SCIENTIST_AURA, true);
                events.RescheduleEvent(EVENT_MUTATE, 34000);
                events.RescheduleEvent(EVENT_INJECTION, 14000);
                break;
            case NPC_KAZTIK:
                events.RescheduleEvent(EVENT_SONIC_PROJECTION, 5000);
                events.RescheduleEvent(EVENT_SUM_HUNGRY_KUNCHONG, 2000);
                break;
            case NPC_KORVEN:
                Talk(SAY_KORVEN_PULL);
                events.RescheduleEvent(EVENT_SHIELD_BASH, 18000);
                checkklaxxi = 2000;
                break;
            case NPC_IYYOKYK:
                Talk(SAY_IYYOKYK_PULL);
                events.RescheduleEvent(EVENT_DIMINISH, 5000);
                events.RescheduleEvent(EVENT_INSANE_CALCULATION, 25000);
                break;
            case NPC_KAROZ:
                Talk(SAY_KAROZ_PULL);
                events.RescheduleEvent(EVENT_HURL_AMBER, 43000);
                break;
            case NPC_HISEK:
                events.RescheduleEvent(EVENT_MULTI_SHOT, 2000);
                events.RescheduleEvent(EVENT_AIM, 39500);
                /*if (me->GetMap()->IsHeroic()) Not found spell
                    events.RescheduleEvent(EVENT_RAPID_FIRE, 47000);*/
                break;
            }
            if (me->GetMap()->IsHeroic())
                events.RescheduleEvent(EVENT_PARAGONS_PURPOSE, 50000);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_KLAXXI_IN_PROGRESS:
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                me->GetMotionMaster()->MoveJump(1582.4f, -5684.9f, -313.635f, 15.0f, 15.0f, 1);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                break;
            case ACTION_RE_ATTACK:
                me->SetFullHealth();
                if (me->GetEntry() != NPC_HISEK)
                    me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 150.0f);
                break;
            case ACTION_RE_ATTACK_KILRUK:
                me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->SetReactState(REACT_AGGRESSIVE);
                events.RescheduleEvent(EVENT_DEATH_FROM_ABOVE, 34000);
                break;
            case ACTION_MOVE_TO_CENTER:
                events.RescheduleEvent(EVENT_RE_ATTACK, 2000);
                break;
            }
        }

        void EnterEvadeMode()
        {
            if (instance->GetBossState(DATA_KLAXXI) != NOT_STARTED)
                instance->SetBossState(DATA_KLAXXI, NOT_STARTED);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            switch (type)
            {
            case EFFECT_MOTION_TYPE:
            {
                switch (pointId)
                {
                case 1:
                    if (Player* pl = me->FindNearestPlayer(250.0f, true))
                    {
                        me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                        me->RemoveAurasDueToSpell(SPELL_READY_TO_FIGHT);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        if (me->GetEntry() != NPC_HISEK)
                            me->SetReactState(REACT_AGGRESSIVE);
                        me->SetInCombatWith(pl);
                        pl->SetInCombatWith(me);
                        me->AddThreat(pl, 0.0f);
                        instance->SetData(DATA_BUFF_NEXT_KLAXXI, 0);
                    }
                    else
                    {
                        if (instance->GetBossState(DATA_KLAXXI) != NOT_STARTED)
                            instance->SetBossState(DATA_KLAXXI, NOT_STARTED);
                    }
                    break;
                case 2:
                    if (Creature* kc = me->GetCreature(*me, instance->GetGuidData(NPC_KLAXXI_CONTROLLER)))
                        me->SetFacingToObject(kc);
                    DoCast(me, SPELL_HURL_AMBER);
                    break;
                case 3:
                    me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.RescheduleEvent(EVENT_FLASH, 10000);
                    break;
                case 4:
                    if (Player* pl = me->GetPlayer(*me, dfatargetGuid))
                        DoCast(pl, SPELL_DEATH_FROM_ABOVE_SUM);
                    break;
                }
            }
            break;
            case POINT_MOTION_TYPE:
            {
                if (pointId == 1003)
                {
                    flashcount--;
                    if (flashcount)
                        events.RescheduleEvent(EVENT_FLASH, 3000);
                    else
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.RescheduleEvent(EVENT_HURL_AMBER, 50000);
                    }
                }
            }
            break;
            }
        }

        void JustDied(Unit* killer)
        {
            if (killer != me && instance)
            {
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                if (!instance->GetData(DATA_IS_KLAXXI_DONE))
                {
                    me->SetLootRecipient(NULL);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                    instance->SetData(DATA_INTRO_NEXT_KLAXXI, 0);
                }
                else
                {
                    Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                    if (!PlayerList.isEmpty())
                        for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                            if (Player* player = Itr->getSource())
                                player->ModifyCurrency(CURRENCY_TYPE_VALOR_POINTS, 7000);
                    instance->SetBossState(DATA_KLAXXI, DONE);
                }
                switch (me->GetEntry())
                {
                case NPC_SKEER:
                    Talk(SAY_SKEER_DIE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HEWN);
                    break;
                case NPC_RIKKAL:
                    Talk(SAY_RIKKAL_DIE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INJECTION);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GENETIC_ALTERATION);
                    break;
                case NPC_HISEK:
                    Talk(SAY_HISEK_DIE);
                    break;
                case NPC_KAROZ:
                    Talk(SAY_KAROZ_DIE);
                    break;
                case NPC_KORVEN:
                    Talk(SAY_KORVEN_DIE);
                    break;
                case NPC_IYYOKYK:
                    Talk(SAY_IYYOKYK_DIE);
                    break;
                case NPC_XARIL:
                    Talk(SAY_XARIL_DIE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TENDERIZING_STRIKES_DMG);
                    for (uint8 n = 0; n < 6; n++)
                        instance->DoRemoveAurasDueToSpellOnPlayers(toxinlist[n]);
                    break;
                case NPC_KAZTIK:
                    Talk(SAY_KAZTIK_DIE);
                    break;
                case NPC_KILRUK:
                    Talk(SAY_KILRUK_DIE);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EXPOSED_VEINS);
                    break;
                default:
                    break;
                }
            }
        }

        void OnSpellClick(Unit* clicker)
        {
            if (instance && instance->GetBossState(DATA_KLAXXI) == IN_PROGRESS)
            {
                if (Player* pl = clicker->ToPlayer())
                {
                    switch (me->GetEntry())
                    {
                    case NPC_RIKKAL:
                        //Any
                        pl->CastSpell(pl, SPELL_MAD_SCIENTIST, true);
                        break;
                    case NPC_KORVEN:
                        if (pl->GetSpecializationRole() == ROLES_TANK)
                            pl->CastSpell(pl, SPELL_MASTER_OF_AMBER, true);
                        break;
                    case NPC_KILRUK:
                        if (pl->GetSpecializationRole() == ROLES_DPS)
                            pl->CastSpell(pl, SPELL_ANGEL_OF_DEATH, true);
                        break;
                    case NPC_HISEK:
                        if (pl->GetSpecializationRole() == ROLES_DPS)
                            pl->CastSpell(pl, SPELL_COMPOUND_EYE, true);
                        break;
                    case NPC_XARIL:
                        if (pl->GetSpecializationRole() == ROLES_DPS)
                            pl->CastSpell(pl, SPELL_VAST_APOTHECARIAL_KNOWLEDGE, true);
                        break;
                    default:
                        break;
                    }
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                }
            }
        }

        ObjectGuid GetTargetGuid(uint32 filteraura = 0)
        {
            std::list<Player*> pllist;
            pllist.clear();
            GetPlayerListInGrid(pllist, me, 150.0f);
            if (!pllist.empty())
            {
                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                {
                    if (filteraura)
                    {
                        if (!(*itr)->isInTankSpec() && !(*itr)->HasAura(filteraura))
                            return (*itr)->GetGUID();
                    }
                    else
                    {
                        if (!(*itr)->isInTankSpec())
                            return (*itr)->GetGUID();
                    }
                }
            }
            return ObjectGuid::Empty;
        }

        ObjectGuid GetFlashTargetGuid()
        {
            std::list<Player*> pllist;
            pllist.clear();
            GetPlayerListInGrid(pllist, me, 150.0f);
            if (!pllist.empty())
            {
                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    if (!(*itr)->isInTankSpec() && me->GetDistance(*itr) >= 30.0f)
                        return (*itr)->GetGUID();

                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    if (!(*itr)->isInTankSpec())
                        return (*itr)->GetGUID();
            }
            return ObjectGuid::Empty;
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (healcooldown)
            {
                if (healcooldown <= diff)
                {
                    healcooldown = 0;
                    healready = true;
                }
                else
                    healcooldown -= diff;
            }

            events.Update(diff);

            if (me->HasAura(SPELL_ENCASE_IN_AMBER) || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (checkklaxxi)
            {
                if (checkklaxxi <= diff)
                {
                    if (healready)
                    {
                        for (uint8 n = 0; n < 9; n++)
                        {
                            if (Creature* klaxxi = me->FindNearestCreature(klaxxientry[n], 150.0f, true))
                            {
                                if (klaxxi->isInCombat())
                                {
                                    if (klaxxi->HealthBelowPct(50))
                                    {
                                        healready = false;
                                        klaxxi->CastSpell(klaxxi, SPELL_ENCASE_IN_AMBER, true);
                                        klaxxi->SummonCreature(NPC_AMBER, klaxxi->GetPositionX(), klaxxi->GetPositionY(), klaxxi->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 10000);
                                        healcooldown = 90000;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    checkklaxxi = 2000;
                }
                else
                    checkklaxxi -= diff;
            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    //Kaztik
                case EVENT_SONIC_PROJECTION:
                    if (me->getVictim())
                        DoCastVictim(SPELL_SONIC_PROJECTION_AT);
                    events.RescheduleEvent(EVENT_SONIC_PROJECTION, 4000);
                    break;
                case EVENT_SUM_HUNGRY_KUNCHONG:
                {
                    uint8 maxcount = me->GetMap()->Is25ManRaid() ? 4 : 2;
                    for (uint8 n = 0; n < maxcount; n++)
                    {
                        if (Creature* ap = me->FindNearestCreature(NPC_AMBER_PIECE, 150.0f, true))
                        {
                            float x, y;
                            GetPosInRadiusWithRandomOrientation(ap, 50.0f, x, y);
                            me->SummonCreature(NPC_HUNGRY_KUNCHONG, x, y, ap->GetPositionZ());
                        }
                    }
                    events.RescheduleEvent(EVENT_DEVOUR, 60000);
                    break;
                }
                case EVENT_DEVOUR:
                {
                    std::list<Creature*> hklist;
                    GetCreatureListWithEntryInGrid(hklist, me, NPC_HUNGRY_KUNCHONG, 150.0f);
                    if (!hklist.empty())
                    {
                        for (std::list<Creature*>::const_iterator itr = hklist.begin(); itr != hklist.end(); itr++)
                        {
                            if (!(*itr)->HasAura(SPELL_MOLT))
                            {
                                (*itr)->AI()->DoAction(ACTION_DEVOUR);
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_DEVOUR, 60000);
                    break;
                }
                    //Iyyokyk
                case EVENT_DIMINISH:
                    DoCastVictim(SPELL_DIMINISH);
                    events.RescheduleEvent(EVENT_DIMINISH, 5000);
                    break;
                case EVENT_INSANE_CALCULATION:
                    DoCast(me, SPELL_INSANE_CALC_FIERY_EDGE);
                    events.RescheduleEvent(EVENT_INSANE_CALCULATION, 36000);
                    break;
                    //Hisek
                case EVENT_MULTI_SHOT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                        DoCast(target, SPELL_MULTI_SHOT);
                    events.RescheduleEvent(EVENT_MULTI_SHOT, 4000);
                    break;
                case EVENT_AIM:
                    if (Player* pl = me->GetPlayer(*me, GetTargetGuid()))
                    {
                        Talk(SAY_HISEK_AIM);
                        DoCast(pl, SPELL_AIM_DUMMY);
                    }
                    events.RescheduleEvent(EVENT_AIM, 39500);
                    break;
                case EVENT_RAPID_FIRE:
                    DoCast(me, SPELL_RAPID_FIRE_DUMMY);
                    events.RescheduleEvent(EVENT_RAPID_FIRE, 47000);
                    break;
                    //Skeer
                case EVENT_BLODDLETTING:
                    Talk(SAY_SKEER_BLOOD);
                    DoCastVictim(SPELL_BLODDLETTING);
                    events.RescheduleEvent(EVENT_BLODDLETTING, 35000);
                    break;
                    //Kilruk
                case EVENT_DEATH_FROM_ABOVE:
                    if (Player* pl = me->GetPlayer(*me, GetTargetGuid()))
                    {
                        dfatargetGuid = pl->GetGUID();
                        events.DelayEvents(6000);
                        me->StopAttack();
                        me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                        me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 10.0f, 15.0f, 15.0f, 4);
                    }
                    else
                        events.RescheduleEvent(EVENT_DEATH_FROM_ABOVE, 34000);
                    break;
                case EVENT_DEATH_FROM_ABOVE_START:
                    if (Creature* jt = me->GetCreature(*me, jtGuid))
                        DoCast(jt, SPELL_DEATH_FROM_ABOVE);
                    break;
                case EVENT_GOUGE:
                    if (me->getVictim())
                        DoCastVictim(SPELL_GOUGE);
                    events.RescheduleEvent(EVENT_GOUGE, 20000);
                    break;
                case EVENT_REAVE:
                    if (Player* pl = me->GetPlayer(*me, GetTargetGuid()))
                    {
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        DoCast(pl, SPELL_REAVE);
                    }
                    events.RescheduleEvent(EVENT_REAVE, 33000);
                    break;
                    //Rikkal
                case EVENT_MUTATE:
                {
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 100.0f);
                    uint8 count = 0;
                    uint8 maxcount = me->GetMap()->Is25ManRaid() ? 3 : 1;
                    if (!pllist.empty() && maxcount)
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->isInTankSpec() && !(*itr)->HasAura(SPELL_MUTATE))
                            {
                                count++;
                                DoCast(*itr, SPELL_MUTATE, true);
                            }
                            if (count >= maxcount)
                                break;
                        }
                    }
                    events.RescheduleEvent(EVENT_MUTATE, 34000);
                    break;
                }
                case EVENT_INJECTION:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_INJECTION);
                    events.RescheduleEvent(EVENT_INJECTION, 10000);
                    break;
                    //Xaril
                case EVENT_TOXIC_INJECTION:
                    if (me->GetMap()->IsHeroic())
                        DoCast(me, catalystlist[urand(4, 5)]);
                    else
                        ToxicInjection();
                    events.RescheduleEvent(EVENT_CATALYST, 27000);
                    break;
                case EVENT_CATALYST:
                {
                    if (!instance)
                        return;

                    uint32 mod = me->GetMap()->IsHeroic() ? urand(3, 5) : urand(0, 2);
                    DoCast(me, catalystlist[mod], true);
                    for (uint8 b = 0; b < 6; ++b)
                        if (b != mod)
                            instance->DoRemoveAurasDueToSpellOnPlayers(toxinlist[b]);
                    events.RescheduleEvent(EVENT_TOXIC_INJECTION, 32000);
                    break;
                }
                //Korven
                case EVENT_SHIELD_BASH:
                    if (me->getVictim())
                        DoCastVictim(SPELL_SHIELD_BASH);
                    events.RescheduleEvent(EVENT_SHIELD_BASH, 17000);
                    break;
                    //Karoz
                case EVENT_HURL_AMBER:
                    me->StopAttack();
                    me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                    if (Creature* ab = me->FindNearestCreature(NPC_AMBER_BOMB, 110.0f, true))
                        me->GetMotionMaster()->MoveJump(ab->GetPositionX(), ab->GetPositionY(), ab->GetPositionZ() + 5.0f, 15.0f, 15.0f, 2);
                    break;
                case EVENT_FLASH:
                    me->StopAttack();
                    if (!flashcount)
                        flashcount = urand(2, 3);
                    if (Player* pl = me->GetPlayer(*me, GetFlashTargetGuid()))
                        DoCast(pl, SPELL_STORE_KINETIC_ENERGY);
                    break;
                    //Special
                case EVENT_RE_ATTACK:
                    me->GetMotionMaster()->MoveJump(1582.4f, -5684.9f, -313.635f, 15.0f, 15.0f, 3);
                    break;
                    //HM
                case EVENT_PARAGONS_PURPOSE:
                    DoCast(me, SPELL_PARAGONS_PURPOSE_DMG, true);
                    events.RescheduleEvent(EVENT_PARAGONS_PURPOSE, 50000);
                    break;
                }
            }
            if (me->GetEntry() != NPC_HISEK)
                DoMeleeAttackIfReady();
        }

        void ToxicInjection()
        {
            uint8 firsttoxin = 0; //blue
            uint8 secondtoxin = 0;//red
            uint8 lasttoxin = 0;  //yellow
            if (me->GetMap()->Is25ManRaid())
            {
                uint8 random = urand(0, 2);
                switch (random)
                {
                case 0:
                    firsttoxin = 3;
                    secondtoxin = 11;
                    lasttoxin = 11;
                    break;
                case 1:
                    firsttoxin = 3;
                    secondtoxin = 10;
                    lasttoxin = 12;
                    break;
                case 2:
                    firsttoxin = 3;
                    secondtoxin = 12;
                    lasttoxin = 10;
                    break;
                }
            }
            else
            {
                uint8 random = urand(0, 2);
                switch (random)
                {
                case 0:
                    firsttoxin = 2;
                    secondtoxin = 4;
                    lasttoxin = 4;
                    break;
                case 1:
                    firsttoxin = 2;
                    secondtoxin = 5;
                    lasttoxin = 3;
                    break;
                case 2:
                    firsttoxin = 2;
                    secondtoxin = 3;
                    lasttoxin = 5;
                    break;
                }
            }
            std::list<Player*>pllist;
            pllist.clear();
            me->GetPlayerListInGrid(pllist, 150.0f);
            if (pllist.size())
            {
                std::vector<Player*>_pllist;
                _pllist.clear();
                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    _pllist.push_back(*itr);

                uint8 _size = _pllist.size();
                for (uint8 n = 0; n < _size; ++n)
                {
                    if (firsttoxin)
                    {
                        firsttoxin--;
                        DoCast(_pllist[n], toxinlist[0]);
                    }
                    else if (secondtoxin)
                    {
                        secondtoxin--;
                        DoCast(_pllist[n], toxinlist[1]);
                    }
                    else if (lasttoxin)
                    {
                        lasttoxin--;
                        DoCast(_pllist[n], toxinlist[2]);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_paragons_of_the_klaxxiAI(creature);
    }
};

//71542
class npc_klaxxi_blood : public CreatureScript
{
public:
    npc_klaxxi_blood() : CreatureScript("npc_klaxxi_blood") {}

    struct npc_klaxxi_bloodAI : public ScriptedAI
    {
        npc_klaxxi_bloodAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            klaxxiGuid.Clear();
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid klaxxiGuid;
        uint32 checktarget;

        void Reset()
        {
            events.RescheduleEvent(EVENT_FIND_LOW_HP_KLAXXI, 500);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            me->getThreatManager().addThreat(attacker, 0.0f);
        }

        ObjectGuid GetLowestHpKlaxxi()
        {     
            std::list<Creature*> klaxxilist;
            klaxxilist.clear();
            float lasthppct = 100;
            for (uint8 n = 0; n < 9; n++)
            {   //only active Klaxxi
                if (Creature* klaxxi = me->FindNearestCreature(klaxxientry[n], 150.0f, true))
                    if (klaxxi->isInCombat())
                        klaxxilist.push_back(klaxxi);
            }

            if (!klaxxilist.empty())
            {   //get lowest hp value from Klaxxi
                for (std::list<Creature*>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                {
                    if ((*itr)->GetHealthPct() < lasthppct)
                        lasthppct = (*itr)->GetHealthPct();
                }
                //find Klaxxi with this value
                for (std::list<Creature*>::const_iterator itr = klaxxilist.begin(); itr != klaxxilist.end(); itr++)
                    if ((*itr)->GetHealthPct() <= lasthppct)
                        return ((*itr)->GetGUID());
            }
            return ObjectGuid::Empty;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (pointId == 0)
                {
                    events.Reset();
                    me->StopMoving();
                    me->GetMotionMaster()->Clear(false);
                    if (Creature* klaxxi = me->GetCreature(*me, klaxxiGuid))
                    {
                        if (klaxxi->isAlive())
                        {
                            klaxxi->SetHealth(klaxxi->GetHealth() + me->GetHealth());
                            me->DespawnOrUnsummon();
                        }
                    }
                }
            }
        }
        
        void CheckDistToKlaxxi()
        {
            if (Creature* klaxxi = me->GetCreature(*me, klaxxiGuid))
            {
                if (klaxxi->isAlive())
                {
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveCharge(klaxxi->GetPositionX(), klaxxi->GetPositionY(), me->GetPositionZ(), 4.0f, 0);
                    events.RescheduleEvent(EVENT_CHECK_DIST_TO_KLAXXI, 1000);
                    return;
                }
            }
            me->StopMoving();
            me->GetMotionMaster()->Clear(false);
            events.RescheduleEvent(EVENT_FIND_LOW_HP_KLAXXI, 1000);
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
                case EVENT_FIND_LOW_HP_KLAXXI:
                    if (Creature* klaxxi = me->GetCreature(*me, GetLowestHpKlaxxi()))
                    {
                        klaxxiGuid = klaxxi->GetGUID();
                        me->GetMotionMaster()->MoveCharge(klaxxi->GetPositionX(), klaxxi->GetPositionY(), me->GetPositionZ(), 4.0f, 0);
                        events.RescheduleEvent(EVENT_CHECK_DIST_TO_KLAXXI, 1000);
                    }
                    else
                        events.RescheduleEvent(EVENT_FIND_LOW_HP_KLAXXI, 1000);
                    break;
                case EVENT_CHECK_DIST_TO_KLAXXI:
                    CheckDistToKlaxxi();
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_klaxxi_bloodAI(creature);
    }
};

//71578
class npc_amber_parasite : public CreatureScript
{
public:
    npc_amber_parasite() : CreatureScript("npc_amber_parasite") {}

    struct npc_amber_parasiteAI : public ScriptedAI
    {
        npc_amber_parasiteAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;
        uint32 checktarget;
        bool fullhealth;

        void Reset()
        {
            if (instance && instance->GetBossState(DATA_KLAXXI) != IN_PROGRESS)
            {
                me->DespawnOrUnsummon();
                return;
            }
            DoCast(me, SPELL_GENETIC_MOD, true);
            targetGuid.Clear();
            checktarget = 0;
            fullhealth = true;
            SetHungerTarget();
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_FEED, 5000);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (attacker->GetGUID() != targetGuid)
                me->getThreatManager().addThreat(attacker, 0.0f);
        }

        void SpellHit(Unit* caster, SpellInfo const *spell)
        {
            if (spell->Id == SPELL_PREY && caster->HasAura(SPELL_FAULTY_MUTATION))
                caster->RemoveAurasDueToSpell(SPELL_FAULTY_MUTATION);
        }

        void JustDied(Unit* killer)
        {
            if (Player* pl = me->GetPlayer(*me, targetGuid))
                if (pl->isAlive())
                    pl->RemoveAurasDueToSpell(SPELL_HUNGER);
        }

        void SetHungerTarget()
        {
            std::list<Player*> pllist;
            pllist.clear();
            GetPlayerListInGrid(pllist, me, 150.0f);
            if (!pllist.empty())
            {
                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                {
                    if (!(*itr)->isInTankSpec() && !(*itr)->HasAura(SPELL_HUNGER))
                    {
                        DoCast(*itr, SPELL_HUNGER, true);
                        targetGuid = (*itr)->GetGUID();
                        me->AddThreat((*itr), 50000000.0f);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->TauntApply((*itr));
                        break;
                    }
                }
            }
            checktarget = 1500;
        }

        void UpdateAI(uint32 diff)
        {
            if (checktarget)
            {
                if (checktarget <= diff)
                {
                    Player* pl = me->GetPlayer(*me, targetGuid);
                    if (!pl || !pl->isAlive())
                    {
                        checktarget = 0;
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        SetHungerTarget();
                    }
                }
                else
                    checktarget -= diff;
            }
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_FEED)
                {
                    if (me->getVictim())
                        DoCastVictim(SPELL_FEED);
                    events.RescheduleEvent(EVENT_FEED, 15000);
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_amber_parasiteAI(creature);
    }
};

//71407
class npc_amber : public CreatureScript
{
public:
    npc_amber() : CreatureScript("npc_amber") {}

    struct npc_amberAI : public ScriptedAI
    {
        npc_amberAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            if (me->GetMap()->IsHeroic())
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_AMBER_VISUAL, true);
        }

        InstanceScript* instance;

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (me->GetMap()->IsHeroic()) //for safe
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void JustDied(Unit* killer)
        {
            if (me->ToTempSummon())
            {
                if (Unit* klaxxi = me->ToTempSummon()->GetSummoner())
                {
                    if (GameObject* it = me->FindNearestGameObject(GO_ICE_TOMB, 3.0f))
                        it->Delete();
                    klaxxi->RemoveAurasDueToSpell(SPELL_ENCASE_IN_AMBER, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
                    me->DespawnOrUnsummon();
                }
            }
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_amberAI(creature);
    }
};

//71413
class npc_amber_player : public CreatureScript
{
public:
    npc_amber_player() : CreatureScript("npc_amber_player") {}

    struct npc_amber_playerAI : public ScriptedAI
    {
        npc_amber_playerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_AMBER_VISUAL, true);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_amber_playerAI(creature);
    }
};

//71420
class npc_kunchong : public CreatureScript
{
public:
    npc_kunchong() : CreatureScript("npc_kunchong") {}

    struct npc_kunchongAI : public ScriptedAI
    {
        npc_kunchongAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;
        uint32 donepct;

        void Reset()
        {
            events.Reset();
            targetGuid.Clear();
            donepct = 1;
            me->SetReactState(REACT_PASSIVE);
            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetPower(POWER_ENERGY, 0);
            me->SetSpeed(MOVE_RUN, 0.5f);
            DoCast(me, SPELL_HUNGRY, true);
            DoCast(me, SPELL_THICK_SHELL, true);
            DoZoneInCombat(me, 150.0f);
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            if (id == 1)
            {
                targetGuid = guid;
                donepct = (me->GetHealthPct() - 25) > 0 ? floor(me->GetHealthPct() - 25) : 1;
            }
        }

        void SpellHit(Unit* caster, SpellInfo const *spell)
        {
            if (spell->Id == SPELL_PLAYER_REAVE && me->HasAura(SPELL_THICK_SHELL))
                me->RemoveAurasDueToSpell(SPELL_THICK_SHELL);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(donepct) && !me->HasAura(SPELL_THICK_SHELL))
            {
                if (me->GetPower(POWER_ENERGY) < 100)
                    DoCast(me, SPELL_THICK_SHELL, true);
                if (Player* pl = me->GetPlayer(*me, targetGuid))
                    if (pl->isAlive())
                        pl->RemoveAurasDueToSpell(SPELL_DEVOUR);
                me->RemoveAurasDueToSpell(SPELL_DEVOUR);
                targetGuid.Clear();
                donepct = 1;
            }
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_MELEE_PHASE:
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 150.0f);
                events.RescheduleEvent(EVENT_SWIPE, 5000);
                break;
            case ACTION_DEVOUR:
            {
                std::list<Player*> pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, me, 150.0f);
                if (!pllist.empty())
                {
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    {
                        if (!(*itr)->isInTankSpec() && !(*itr)->HasAura(SPELL_MUTATE))
                        {
                            DoCast(*itr, SPELL_DEVOUR);
                            break;
                        }
                    }
                }
            }
            break;
            }
        }

        void KilledUnit(Unit* victim)
        {
            if (victim->GetGUID() == targetGuid)
            {
                targetGuid.Clear();
                me->RemoveAurasDueToSpell(SPELL_DEVOUR);
                me->SetPower(POWER_ENERGY, 100);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_SWIPE)
                {
                    DoCast(me, SPELL_SWIPE);
                    events.RescheduleEvent(EVENT_SWIPE, 12000);
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_kunchongAI(creature);
    }
};

//90003
class npc_flash_stalker : public CreatureScript
{
public:
    npc_flash_stalker() : CreatureScript("npc_flash_stalker") {}

    struct npc_flash_stalkerAI : public ScriptedAI
    {
        npc_flash_stalkerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_flash_stalkerAI(creature);
    }
};

//71453
class npc_eerie_fog : public CreatureScript
{
public:
    npc_eerie_fog() : CreatureScript("npc_eerie_fog") {}

    struct npc_eerie_fogAI : public ScriptedAI
    {
        npc_eerie_fogAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_EERIE_FOG_AT, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_eerie_fogAI(creature);
    }
};

//143939
class spell_klaxxi_gouge: public SpellScriptLoader
{
public:
    spell_klaxxi_gouge() : SpellScriptLoader("spell_klaxxi_gouge") { }

    class spell_klaxxi_gouge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_klaxxi_gouge_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTarget())
                GetCaster()->CastSpell(GetTarget(), SPELL_MUTILATE, false);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_klaxxi_gouge_AuraScript::OnApply, EFFECT_0, SPELL_AURA_FIXATE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_klaxxi_gouge_AuraScript();
    }
};

//143373
class spell_gene_splice : public SpellScriptLoader
{
public:
    spell_gene_splice() : SpellScriptLoader("spell_gene_splice") { }

    class spell_gene_splice_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gene_splice_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_GENE_SPLICE);
                GetTarget()->RemoveAurasDueToSpell(SPELL_MAD_SCIENTIST);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_gene_splice_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_gene_splice_AuraScript();
    }
};

//143217
class spell_snipe : public SpellScriptLoader
{
public:
    spell_snipe() : SpellScriptLoader("spell_snipe") { }

    class spell_snipe_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_snipe_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                float distance = GetCaster()->GetExactDist2d(GetHitUnit());
                if (distance && distance <= 100)
                    SetHitDamage(GetHitDamage() * (distance / 100));
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_snipe_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_snipe_SpellScript();
    }
};

//148676
class spell_klaxxi_reave : public SpellScriptLoader
{
public:
    spell_klaxxi_reave() : SpellScriptLoader("spell_klaxxi_reave") { }

    class spell_klaxxi_reave_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_klaxxi_reave_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
                {
                    GetCaster()->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    GetCaster()->ToCreature()->AI()->DoZoneInCombat(GetCaster()->ToCreature(), 80.0f);
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_klaxxi_reave_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_klaxxi_reave_AuraScript();
    }
};

//143339 
class spell_klaxxi_injection : public SpellScriptLoader
{
public:
    spell_klaxxi_injection() : SpellScriptLoader("spell_klaxxi_injection") { }

    class spell_klaxxi_injection_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_klaxxi_injection_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                GetTarget()->CastSpell(GetTarget(), SPELL_INJECTION_SUM, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_klaxxi_injection_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_klaxxi_injection_AuraScript();
    }
};

//143337
class spell_klaxxi_mutate : public SpellScriptLoader
{
public:
    spell_klaxxi_mutate() : SpellScriptLoader("spell_klaxxi_mutate") { }

    class spell_klaxxi_mutate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_klaxxi_mutate_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->GetMap()->IsHeroic())
                GetTarget()->CastSpell(GetTarget(), SPELL_FAULTY_MUTATION, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_klaxxi_mutate_AuraScript::OnApply, EFFECT_0, SPELL_AURA_TRANSFORM, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_klaxxi_mutate_AuraScript();
    }
};

//148589
class spell_faulty_mutation : public SpellScriptLoader
{
public:
    spell_faulty_mutation() : SpellScriptLoader("spell_faulty_mutation") { }

    class spell_faulty_mutation_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_faulty_mutation_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    GetTarget()->CastSpell(GetTarget(), SPELL_FAULTY_MUTATION_KILL, true);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_faulty_mutation_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_faulty_mutation_AuraScript();
    }
};

//143280
class spell_klaxxi_bloodletting : public SpellScriptLoader
{
public:
    spell_klaxxi_bloodletting() : SpellScriptLoader("spell_klaxxi_bloodletting") { }

    class spell_klaxxi_bloodletting_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_klaxxi_bloodletting_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                uint8 n = GetCaster()->GetMap()->Is25ManRaid() ? 3 : 2;
                for (uint8 b = 0; b < n; b++)
                    GetCaster()->SummonCreature(NPC_BLOOD, bloodsumpos[b]);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_klaxxi_bloodletting_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_klaxxi_bloodletting_SpellScript();
    }
};

//142929
class spell_tenderizing_strikes_dmg : public SpellScriptLoader
{
public:
    spell_tenderizing_strikes_dmg() : SpellScriptLoader("spell_tenderizing_strikes_dmg") { }

    class spell_tenderizing_strikes_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tenderizing_strikes_dmg_SpellScript);

        void HandleHit()
        {
            if (GetCaster() && GetHitUnit())
            {
                for (uint8 n = 0; n < 5; n++)
                    if (GetHitUnit()->HasAura(EvadeSpells[n]))
                        return;
                GetCaster()->CastSpell(GetHitUnit(), SPELL_CAUSTIC_BLOOD);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_tenderizing_strikes_dmg_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tenderizing_strikes_dmg_SpellScript();
    }
};

//142315
class spell_caustic_blood : public SpellScriptLoader
{
public:
    spell_caustic_blood() : SpellScriptLoader("spell_caustic_blood") { }

    class spell_caustic_blood_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_caustic_blood_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetTarget()->HasAura(SPELL_CAUSTIC_BLOOD))
            {
                if (GetTarget()->GetAura(SPELL_CAUSTIC_BLOOD)->GetStackAmount() >= 10)
                {
                    GetTarget()->RemoveAurasDueToSpell(SPELL_CAUSTIC_BLOOD);
                    GetTarget()->CastSpell(GetTarget(), SPELL_BLOODY_EXPLOSION, true);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_caustic_blood_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_caustic_blood_AuraScript();
    }
};

//142564
class spell_encase_in_amber : public SpellScriptLoader
{
public:
    spell_encase_in_amber() : SpellScriptLoader("spell_encase_in_amber") { }

    class spell_encase_in_amber_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_encase_in_amber_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEFAULT)
                GetTarget()->CastSpell(GetTarget(), SPELL_AMBER_REGENERATION, true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_encase_in_amber_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_encase_in_amber_AuraScript();
    }
};

//143666
class spell_diminish : public SpellScriptLoader
{
public:
    spell_diminish() : SpellScriptLoader("spell_diminish") { }

    class spell_diminish_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_diminish_SpellScript);

        void HandleHit()
        {
            if (GetCaster() && GetHitUnit())
            {
                if (GetHitUnit()->HealthBelowPct(34))
                    GetHitUnit()->Kill(GetHitUnit(), true);
                else
                {
                    int32 dmg = GetHitUnit()->CountPctFromMaxHealth(34);
                    SetHitDamage(dmg);
                }
            }
        }

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            uint8 maxcount = 0;
            if (GetCaster())
            {
                switch (GetCaster()->GetMap()->GetDifficultyID())
                {
                    case DIFFICULTY_10_N:
                        maxcount = 1;
                        break;
                    case DIFFICULTY_25_N:
                        maxcount = 3;
                        break;
                    case DIFFICULTY_10_HC:
                        maxcount = 2;
                        break;
                    case DIFFICULTY_25_HC:
                        maxcount = 5;
                        break;
                    default:
                        break;
                }
            }
            if (targets.size() > maxcount)
                targets.resize(maxcount);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_diminish_SpellScript::HandleHit);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_diminish_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_diminish_SpellScript::_FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_diminish_SpellScript();
    }
};

//143759
class spell_hurl_amber : public SpellScriptLoader
{
public:
    spell_hurl_amber() : SpellScriptLoader("spell_hurl_amber") { }

    class spell_hurl_amber_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hurl_amber_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                std::list<Player*>pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 100.0f);
                if (!pllist.empty())
                {
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    {
                        if (!(*itr)->isInTankSpec())
                        {
                            GetCaster()->CastSpell(*itr, SPELL_HURL_AMBER_DMG, true);
                            break;
                        }
                    }
                }
            }
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    if (GetTarget()->isInCombat() && GetTarget()->ToCreature())
                        GetTarget()->ToCreature()->AI()->DoAction(ACTION_MOVE_TO_CENTER);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hurl_amber_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            OnEffectRemove += AuraEffectRemoveFn(spell_hurl_amber_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hurl_amber_AuraScript();
    }
};

//143765
class spell_sonic_projection : public SpellScriptLoader
{
public:
    spell_sonic_projection() : SpellScriptLoader("spell_sonic_projection") { }

    class spell_sonic_projection_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sonic_projection_SpellScript);

        void HandleBeforeCast()
        {
            if (GetCaster())
            {
                float x, y, ang;
                ang = GetCaster()->GetOrientation();;
                GetPositionWithDistInOrientation(GetCaster(), 200.0f, ang, x, y);
                GetSpell()->destAtTarget.Relocate(x, y, GetCaster()->GetPositionZ());
            }
        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_sonic_projection_SpellScript::HandleBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sonic_projection_SpellScript();
    }
};

//142948
class spell_klaxxi_aim : public SpellScriptLoader
{
public:
    spell_klaxxi_aim() : SpellScriptLoader("spell_klaxxi_aim") { }

    class spell_klaxxi_aim_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_klaxxi_aim_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetTarget())
            {
                GetCaster()->SetFacingTo(GetTarget());
                GetTarget()->AddAura(SPELL_AIM_STUN, GetTarget());
                if (GetCaster()->GetDistance(GetTarget()) < 45.0f)
                {
                    float x, y, ang;
                    ang = GetCaster()->GetAngle(GetTarget());
                    GetPositionWithDistInOrientation(GetCaster(), 45.0f, ang, x, y);
                    GetTarget()->GetMotionMaster()->MoveJump(x, y, GetTarget()->GetPositionZ(), 15.0f, 15.0f);
                }
            }
        }

        void HandlelEffectRemove(AuraEffect const * aurEff, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
                GetCaster()->CastSpell(GetTarget(), SPELL_FIRE, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_klaxxi_aim_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_klaxxi_aim_AuraScript::HandlelEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_klaxxi_aim_AuraScript();
    }
};

//143243
class spell_klaxxi_rapid_fire : public SpellScriptLoader
{
public:
    spell_klaxxi_rapid_fire() : SpellScriptLoader("spell_klaxxi_rapid_fire") { }

    class spell_klaxxi_rapid_fire_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_klaxxi_rapid_fire_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (Unit* target = GetCaster()->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    GetCaster()->CastSpell(target, SPELL_RAPID_FIRE_DMG);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_klaxxi_rapid_fire_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_klaxxi_rapid_fire_AuraScript();
    }
};

//142935
class spell_reaction_blue : public SpellScriptLoader
{
public:
    spell_reaction_blue() : SpellScriptLoader("spell_reaction_blue") { }

    class spell_reaction_blue_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_reaction_blue_SpellScript);

        void HandleHit()
        {
            if (GetHitUnit())
            {
                std::list<Player*> pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetHitUnit(), 8.0f);
                uint8 maxcount = GetHitUnit()->GetMap()->Is25ManRaid() ? 5 : 2;
                uint8 count = 0;
                if (pllist.size() >= maxcount)
                {
                    int32 dmg = GetHitDamage() / maxcount;
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    {
                        count++;
                        if ((*itr)->GetGUID() != GetHitUnit()->GetGUID())
                            (*itr)->CastCustomSpell(SPELL_REACTION_BLUE, SPELLVALUE_BASE_POINT0, dmg, *itr);
                        if (count == maxcount)
                            break;
                    }
                    SetHitDamage(dmg);
                }
                else
                    SetHitDamage(GetHitDamage());
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_reaction_blue_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_reaction_blue_SpellScript();
    }
};

//142416
class spell_insane_calc_fiery_edge : public SpellScriptLoader
{
public:
    spell_insane_calc_fiery_edge() : SpellScriptLoader("spell_insane_calc_fiery_edge") { }

    class spell_insane_calc_fiery_edge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_insane_calc_fiery_edge_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                std::list<Player*> pllist;
                pllist.clear();
                uint8 maxcount = GetCaster()->GetMap()->Is25ManRaid() ? urand(8, 9) : urand(3, 4);
                uint8 count = 0;
                GetPlayerListInGrid(pllist, GetCaster(), 150.0f);
                if (pllist.size() >= 3)
                {
                    std::vector<Player*>targetlist;
                    targetlist.clear();
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    {
                        if (!(*itr)->isInTankSpec() && !(*itr)->HasAura(SPELL_MESMERIZE))
                        {
                            count++;
                            targetlist.push_back(*itr);
                            if (count == maxcount)
                                break;
                        }
                    }
                    if (targetlist.size() >= maxcount)
                    {
                        for (uint8 n = 0; n < (maxcount - 1); ++n)
                            targetlist[n]->CastSpell(targetlist[n + 1], SPELL_FIERY_EDGE_PRE_DUMMY, true);
                        targetlist[0]->CastSpell(targetlist[maxcount - 1], SPELL_FIERY_EDGE_PRE_DUMMY, true);
                    }
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_insane_calc_fiery_edge_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_insane_calc_fiery_edge_SpellScript();
    }
};

//142811
class spell_fiery_edge_pre_dummy : public SpellScriptLoader
{
public:
    spell_fiery_edge_pre_dummy() : SpellScriptLoader("spell_fiery_edge_pre_dummy") { }

    class spell_fiery_edge_pre_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fiery_edge_pre_dummy_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                GetCaster()->CastSpell(GetTarget(), SPELL_FIERY_EDGE_DUMMY, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_fiery_edge_pre_dummy_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fiery_edge_pre_dummy_AuraScript();
    }
};

//142808
class spell_fiery_edge_dummy : public SpellScriptLoader
{
public:
    spell_fiery_edge_dummy() : SpellScriptLoader("spell_fiery_edge_dummy") { }

    class spell_fiery_edge_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fiery_edge_dummy_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetTarget())
            {
                int32 dmg = GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                GetCaster()->CastCustomSpell(SPELL_FIERY_EDGE_DMG, SPELLVALUE_BASE_POINT0, dmg, GetTarget());
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_fiery_edge_dummy_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fiery_edge_dummy_AuraScript();
    }
};

class IsInBetweenCheck
{
public:
    IsInBetweenCheck(Player* caster, Player* target) : _caster(caster), _target(target){}

    bool operator()(WorldObject* unit)
    {
        if (unit->IsInBetween(_caster, _target))
            return false;
        return true;
    }
private:
    Player* _caster;
    Player* _target;
};

//142809
class spell_fiery_edge_dmg : public SpellScriptLoader
{
public:
    spell_fiery_edge_dmg() : SpellScriptLoader("spell_fiery_edge_dmg") { }

    class spell_fiery_edge_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fiery_edge_dmg_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (GetCaster() && GetCaster()->ToPlayer())
            {
                ObjectGuid beamtargetGuid;
                std::list<Player*>targets;
                targets.clear();
                GetPlayerListInGrid(targets, GetCaster(), 100.0f);
                if (!targets.empty())
                {   //find my target
                    for (std::list<Player*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    {
                        if ((*itr)->GetGUID() != GetCaster()->GetGUID())
                        {
                            if (Aura* aura = (*itr)->GetAura(SPELL_FIERY_EDGE_DUMMY))
                            {
                                if (aura->GetCasterGUID() == GetCaster()->GetGUID())
                                {
                                    beamtargetGuid = aura->GetCasterGUID();
                                    break;
                                }
                            }
                        }
                    }
                }
                if (Player* beamtarget = GetCaster()->GetPlayer(*GetCaster(), beamtargetGuid))
                    targets.remove_if(IsInBetweenCheck(GetCaster()->ToPlayer(), beamtarget));
            }
        }
            
        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_fiery_edge_dmg_SpellScript::_FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fiery_edge_dmg_SpellScript();
    }
};

//142272
class spell_reave : public SpellScriptLoader
{
public:
    spell_reave() : SpellScriptLoader("spell_reave") { }

    class spell_reave_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_reave_SpellScript);

        void HandleHit()
        {
            SetHitDamage(300000);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_reave_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_reave_SpellScript();
    }
};

//144839
class spell_klaxxi_multi_shot : public SpellScriptLoader
{
public:
    spell_klaxxi_multi_shot() : SpellScriptLoader("spell_klaxxi_multi_shot") { }

    class spell_klaxxi_multi_shot_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_klaxxi_multi_shot_SpellScript);

        void HandleHit()
        {
            SetHitDamage(urand(100000, 120000));
        }

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            uint8 maxcount = 0;
            if (GetCaster())
            {
                switch (GetCaster()->GetMap()->GetDifficultyID())
                {
                    case DIFFICULTY_10_N:
                        maxcount = 3;
                        break;
                    case DIFFICULTY_25_N:
                        maxcount = 5;
                        break;
                    case DIFFICULTY_10_HC:
                        maxcount = 4;
                        break;
                    case DIFFICULTY_25_HC:
                        maxcount = 7;
                        break;
                    default:
                        break;
                }
            }
            if (targets.size() > maxcount)
                targets.resize(maxcount);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_klaxxi_multi_shot_SpellScript::HandleHit);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_klaxxi_multi_shot_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_klaxxi_multi_shot_SpellScript();
    }
};

//143704
class spell_whirling_target : public SpellScriptLoader
{
public:
    spell_whirling_target() : SpellScriptLoader("spell_whirling_target") { }

    class spell_whirling_target_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_whirling_target_SpellScript);

        SpellCastResult CheckTarget()
        {
            if (GetCaster() && GetExplTargetUnit())
            {
                if (Player* pl = GetExplTargetUnit()->ToPlayer())
                {
                    GetCaster()->SetFacingToObject(pl);
                    pl->CastSpell(pl, SPELL_FLASH_DUMMY, true);
                    Position pos;
                    GetCaster()->GetPosition(&pos);
                    GetCaster()->SummonCreature(NPC_FLASH_STALKER, pos, TEMPSUMMON_TIMED_DESPAWN, 2000);
                }
            }
            return SPELL_CAST_OK;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_whirling_target_SpellScript::CheckTarget);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_whirling_target_SpellScript();
    }
};

class WhirlingTargetFilter
{
public:
    WhirlingTargetFilter(Player* player, Creature* stalker) :  _player(player), _stalker(stalker){}

    bool operator()(WorldObject* unit)
    {
        if (unit->IsInBetween(_stalker, _player, 4.0f))
            return false;
        else if (_player->GetGUID() == unit->GetGUID())
            return false;
        return true;
    }
private:
    Player* _player;
    Creature* _stalker;
};

//143701
class spell_whirling : public SpellScriptLoader
{
public:
    spell_whirling() : SpellScriptLoader("spell_whirling") { }

    class spell_whirling_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_whirling_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
            {
                if (!targets.empty())
                {
                    for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    {
                        if ((*itr)->ToPlayer())
                        {
                            if ((*itr)->ToPlayer()->HasAura(SPELL_FLASH_DUMMY))
                            {
                                flashtargetGuid = (*itr)->GetGUID();
                                break;
                            }
                        }
                    }
                }
                if (Player* maintarget = GetCaster()->GetPlayer(*GetCaster(), flashtargetGuid))
                    if (Creature* stalker = GetCaster()->FindNearestCreature(NPC_FLASH_STALKER, 150.0f, true))
                        targets.remove_if(WhirlingTargetFilter(maintarget, stalker));
            }
        }

        void HandleOnHit()
        {
            if (GetHitUnit())
                GetHitUnit()->RemoveAurasDueToSpell(SPELL_FLASH_DUMMY);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_whirling_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_whirling_SpellScript::_FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_whirling_SpellScript::_FilterTarget, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_whirling_SpellScript::HandleOnHit);
        }
    private:
        ObjectGuid flashtargetGuid;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_whirling_SpellScript();
    }
};

//143355
class spell_genetic_modifications : public SpellScriptLoader
{
public:
    spell_genetic_modifications() : SpellScriptLoader("spell_genetic_modifications") { }

    class spell_genetic_modifications_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_genetic_modifications_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                int32 dmg = GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                GetCaster()->CastCustomSpell(SPELL_REGENERATE, SPELLVALUE_BASE_POINT0, dmg, GetCaster());
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_genetic_modifications_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_genetic_modifications_AuraScript();
    }
};

//143709
class spell_store_kinetic_energy : public SpellScriptLoader
{
public:
    spell_store_kinetic_energy() : SpellScriptLoader("spell_store_kinetic_energy") { }

    class spell_store_kinetic_energy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_store_kinetic_energy_SpellScript);

        void HandleHit()
        {
            if (GetCaster() && GetHitUnit())
                GetCaster()->CastSpell(GetHitUnit(), SPELL_FLASH);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_store_kinetic_energy_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_store_kinetic_energy_SpellScript();
    }
};

//144157
class spell_landing_pose : public SpellScriptLoader
{
public:
    spell_landing_pose() : SpellScriptLoader("spell_landing_pose") { }

    class spell_landing_pose_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_landing_pose_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK_KILRUK);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_landing_pose_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_ROOT, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_landing_pose_AuraScript();
    }
};

//143977
class spell_vicious_assaullt_periodic : public SpellScriptLoader
{
public:
    spell_vicious_assaullt_periodic() : SpellScriptLoader("spell_vicious_assaullt_periodic") { }

    class spell_vicious_assaullt_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_vicious_assaullt_periodic_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                switch (aurEff->GetTickNumber())
                {
                case 2:
                    GetCaster()->CastSpell(GetCaster(), SPELL_VICIOUS_ASSAULT_DMG, true);
                    break;
                case 4:
                    GetCaster()->CastSpell(GetCaster(), SPELL_VICIOUS_ASSAULT_DMG, true);
                    break;
                case 6:
                    GetCaster()->CastSpell(GetCaster(), SPELL_VICIOUS_ASSAULT_DMG_2, true);
                    break;
                case 8:
                    GetCaster()->CastSpell(GetCaster(), SPELL_VICIOUS_ASSAULT_DMG_3, true);
                    break;
                case 10:
                    GetCaster()->CastSpell(GetCaster(), SPELL_VICIOUS_ASSAULT_DMG_4, true);
                    break;
                case 12:
                    GetCaster()->CastSpell(GetCaster(), SPELL_VICIOUS_ASSAULT_DMG_5, true);
                    break;
                default:
                    break;
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_vicious_assaullt_periodic_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_vicious_assaullt_periodic_AuraScript();
    }
};

//143980, 143981, 143982, 143984, 143985
class spell_vicious_assaullt : public SpellScriptLoader
{
public:
    spell_vicious_assaullt() : SpellScriptLoader("spell_vicious_assaullt") { }

    class spell_vicious_assaullt_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_vicious_assaullt_SpellScript);

        void HandleHit()
        {
            if (GetHitUnit() && !GetHitUnit()->HasAura(SPELL_VICIOUS_ASSAULT_DOT))
                GetHitUnit()->AddAura(SPELL_VICIOUS_ASSAULT_DOT, GetHitUnit());
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_vicious_assaullt_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_vicious_assaullt_SpellScript();
    }
};


class HisekAimFilter
{
public:
    HisekAimFilter(Unit* caster, Player* target) : _caster(caster), _target(target){}

    bool operator()(WorldObject* unit)
    {
        if (unit->ToPlayer()->HasAura(SPELL_AIM_STUN))
            return false;
        else if (unit->IsInBetween(_caster, _target, 2.0f))
            return false;
        return true;
    }
private:
    Unit* _caster;
    Player* _target;
};

//142950
class spell_hisek_fire : public SpellScriptLoader
{
public:
    spell_hisek_fire() : SpellScriptLoader("spell_hisek_fire") { }

    class spell_hisek_fire_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hisek_fire_SpellScript);

        void HandleHit()
        {
            if (GetHitUnit() && !GetHitUnit()->HasAura(SPELL_AIM_STUN))
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_SONIC_RESONANCE_HISEK, true);
        }

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
            {
                ObjectGuid aimtargetGuid;

                if (!targets.empty())
                {
                    for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    {
                        if ((*itr)->ToPlayer())
                        {
                            if ((*itr)->ToPlayer()->HasAura(SPELL_AIM_STUN))
                            {
                                aimtargetGuid = (*itr)->GetGUID();
                                break;
                            }
                        }
                    }
                }
                if (Player* aimtarget = GetCaster()->GetPlayer(*GetCaster(), aimtargetGuid))
                    targets.remove_if(HisekAimFilter(GetCaster(), aimtarget));
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_hisek_fire_SpellScript::HandleHit);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hisek_fire_SpellScript::_FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hisek_fire_SpellScript();
    }
};

//142649
class spell_kunchong_devour : public SpellScriptLoader
{
public:
    spell_kunchong_devour() : SpellScriptLoader("spell_kunchong_devour") { }

    class spell_kunchong_devour_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kunchong_devour_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTarget())
            {
                if (GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->SetGUID(GetTarget()->GetGUID(), 1);
                GetTarget()->CastSpell(GetTarget(), SPELL_MESMERIZE, true);
                GetTarget()->SetCharmedBy(GetCaster(), CHARM_TYPE_CONVERT, NULL);
                GetCaster()->RemoveAurasDueToSpell(SPELL_THICK_SHELL);
                GetTarget()->GetMotionMaster()->MoveFollow(GetCaster(), PET_FOLLOW_DIST, GetCaster()->GetFollowAngle());
            }
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTarget())
            {
                GetTarget()->RemoveCharmedBy(GetCaster());
                GetTarget()->RemoveAurasDueToSpell(SPELL_MESMERIZE);
                GetTarget()->StopMoving();
                GetTarget()->GetMotionMaster()->Clear(false);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_kunchong_devour_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_kunchong_devour_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_kunchong_devour_AuraScript();
    }
};

//142630
class spell_hungry_periodic : public SpellScriptLoader
{
public:
    spell_hungry_periodic() : SpellScriptLoader("spell_hungry_periodic") { }

    class spell_hungry_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hungry_periodic_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster())
            {
                if (GetCaster()->GetPower(POWER_ENERGY) == 100)
                {
                    GetCaster()->InterruptNonMeleeSpells(true);
                    GetCaster()->RemoveAurasDueToSpell(SPELL_HUNGRY);
                    GetCaster()->CastSpell(GetCaster(), SPELL_MOLT, true);
                    if (GetCaster()->ToCreature())
                        GetCaster()->ToCreature()->AI()->DoAction(ACTION_MELEE_PHASE);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hungry_periodic_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hungry_periodic_AuraScript();
    }
};

//142945
class spell_eerie_fog : public SpellScriptLoader
{
public:
    spell_eerie_fog() : SpellScriptLoader("spell_eerie_fog") { }

    class spell_eerie_fog_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_eerie_fog_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget())
                if (Aura* aura = GetTarget()->GetAura(SPELL_EERIE_FOG_DMG))
                    if (aura->GetStackAmount() < 99)
                        aura->SetStackAmount(aura->GetStackAmount() + 1);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_eerie_fog_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_eerie_fog_AuraScript();
    }
};

//142729, 142730
class spell_heroic_catalyst : public SpellScriptLoader
{
public:
    spell_heroic_catalyst() : SpellScriptLoader("spell_heroic_catalyst") { }

    class spell_heroic_catalyst_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_heroic_catalyst_SpellScript);

        void FilterTargets(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
            {
                uint8 count = GetCaster()->GetMap()->Is25ManRaid() ? 11 : 5;
                if (targets.size() > count)
                    targets.resize(count);
            }
        }

        void HitHandle()
        {
            if (GetCaster() && GetHitUnit())
            {
                switch (GetSpellInfo()->Id)
                {
                case SPELL_DELAYED_CATALYST_PURPLE:
                    GetHitUnit()->CastSpell(GetHitUnit(), SPELL_CANNED_HEAT_BASE, true);
                    break;
                case SPELL_DELAYED_CATALYST_GREEN:
                    GetCaster()->SummonCreature(NPC_EERIE_FOG, GetHitUnit()->GetPositionX(), GetHitUnit()->GetPositionY(), GetCaster()->GetPositionZ());
                    break;
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_heroic_catalyst_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            OnHit += SpellHitFn(spell_heroic_catalyst_SpellScript::HitHandle);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_heroic_catalyst_SpellScript();
    }
};

void AddSC_boss_paragons_of_the_klaxxi()
{
    new npc_amber_piece();
    new npc_klaxxi_controller();
    new boss_paragons_of_the_klaxxi();
    new npc_klaxxi_blood();
    new npc_amber_parasite();
    new npc_amber();
    new npc_amber_player();
    new npc_kunchong();
    //new npc_flash_stalker();
    new npc_eerie_fog();
    new spell_klaxxi_gouge();
    new spell_gene_splice();
    new spell_snipe();
    new spell_klaxxi_reave();
    new spell_klaxxi_injection();
    new spell_klaxxi_mutate();
    new spell_faulty_mutation();
    new spell_klaxxi_bloodletting();
    new spell_tenderizing_strikes_dmg();
    new spell_caustic_blood();
    new spell_encase_in_amber();
    new spell_diminish();
    new spell_hurl_amber();
    new spell_sonic_projection();
    new spell_klaxxi_aim();
    new spell_klaxxi_rapid_fire();
    new spell_reaction_blue();
    new spell_insane_calc_fiery_edge();
    new spell_fiery_edge_pre_dummy();
    new spell_fiery_edge_dummy();
    new spell_fiery_edge_dmg();
    new spell_reave();
    new spell_klaxxi_multi_shot();
    new spell_whirling_target();
    new spell_whirling();
    new spell_genetic_modifications();
    new spell_store_kinetic_energy();
    new spell_landing_pose();
    new spell_vicious_assaullt_periodic();
    new spell_vicious_assaullt();
    new spell_hisek_fire();
    new spell_kunchong_devour();
    new spell_hungry_periodic();
    new spell_eerie_fog();
    new spell_heroic_catalyst();
}
