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

#include "throne_of_thunder.h"

enum eSpells
{
    //Mallak
    SPELL_FRIGIT_ASSAULT     = 136904,
    SPELL_FRIGIT_ASSAULT_S   = 136910,
    SPELL_FRIGIT_ASSAULT_D   = 136903,
    SPELL_BITING_COLD        = 136992,
    SPELL_FROSTBITE          = 136922,
    SPELL_FROSTBITE_DMG      = 136937,
    //Sul
    SPELL_SAND_BOLT          = 136189,
    SPELL_SAND_TRAP_VISUAL   = 136851,
    SPELL_SAND_TRAP_AT       = 136861,
    SPELL_SAND_TRAP_DMG      = 136860,
    SPELL_ENSNARED           = 136878,
    SPELL_ENTRAPPED          = 136857,
    SPELL_QUICK_SAND_VISUAL  = 136851,
    SPELL_BLESSED_TRANSFORM  = 137181,
    SPELL_SHADOW_TRANSFORM   = 137271,
    SPELL_B_LOA_SPIRIT_P_SUM = 137203,
    SPELL_B_LOA_SPIRIT_SUM   = 137200,
    SPELL_BLESSED_GIFT       = 137303,
    SPELL_D_LOA_SPIRIT_P_SUM = 137350,
    SPELL_D_LOA_SPIRIT_SUM   = 137351,
    SPELL_SANDSTORM          = 136894,
    SPELL_SANDSTORM_VISUAL   = 136895,
    SPELL_FORTIFIED          = 136864,
    //Marli
    SPELL_WRATH_OF_THE_LOA   = 137344,
    SPELL_D_WRATH_OF_THE_LOA = 137347,
    //Kazrajin
    SPELL_R_CHARGE_DMG       = 137133,
    SPELL_R_CHARGE_POINT_T   = 138026,
    SPELL_R_CHARGE_VISUAL    = 137117,
    SPELL_R_CHARGE_POINT_DMG = 137122,
    SPELL_K_OVERLOAD_AURA    = 137149,

    SPELL_LINGERING_PRESENCE = 136467,
    SPELL_DARK_POWER         = 136507,
    SPELL_POSSESSED          = 136442,
    SPELL_GARAJAL_SOUL_V     = 136423,

    //Dark loa
    SPELL_MARKED_SOUL        = 137359,
    SPELL_DARK_EXPLOSE       = 137390,
};

enum SsAction
{
    ACTION_RESET,
    ACTION_ACTIVE,
    ACTION_CHANGE_COUNCIL,
    ACTION_GARAJAL_SOUL_ACTIVE,
};

enum eEvents
{
    //Mallak
    EVENT_BITTING_COLD       = 1,
    EVENT_FRIGIT_ASSAULT     = 2,
    //Sul
    EVENT_SAND_BOLT          = 3,
    EVENT_SAND_TRAP          = 4,
    EVENT_SANDSTORM          = 5,
    //Kazrajin
    EVENT_R_CHARGE           = 6,
    //Marli
    EVENT_LOA_SPIRIT         = 7,
    //
    EVENT_MOVING             = 8,
    EVENT_FIND_LOWHP_COUNCIL = 9,
    EVENT_CHECK_COUNCIL      = 10,
    EVENT_FIND_PLAYER        = 11,
    EVENT_CHECK_DISTANCE     = 12,
};

//Spells summon loa spirit
uint32 blessed_loa_spirit[3] =
{
    137200,
    137201,
    137202,
};

uint32 dark_loa_spirit[3] =
{
    137351,
    137352,
    137353,
};

uint32 councilentry[4] =
{
    NPC_FROST_KING_MALAKK,
    NPC_PRINCESS_MARLI,
    NPC_KAZRAJIN,
    NPC_SUL_SANDCRAWLER,
};

uint32 const loaspiritentry[6] =
{
    NPC_BLESSED_LOA_SPIRIT,
    NPC_BLESSED_LOA_SPIRIT_2,
    NPC_BLESSED_LOA_SPIRIT_3,
    NPC_DARK_LOA_SPIRIT,
    NPC_DARK_LOA_SPIRIT_2,
    NPC_DARK_LOA_SPIRIT_3,
};

struct council_of_eldersAI : public ScriptedAI
{
    council_of_eldersAI(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }
    InstanceScript* instance;

    void CouncilsReset()
    {
        for (int32 i = 0; i < 4; i++)
        {
            if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[i])))
            {
                if (me->GetEntry() != council->GetEntry())
                {
                    if (council->isAlive() && council->isInCombat())
                        council->AI()->EnterEvadeMode();
                    else
                    {
                        council->Respawn();
                        council->GetMotionMaster()->MoveTargetedHome();
                    }
                }
            }
        }
        if (instance->GetBossState(DATA_COUNCIL_OF_ELDERS != NOT_STARTED))
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MARKED_SOUL);
            instance->SetBossState(DATA_COUNCIL_OF_ELDERS, NOT_STARTED);
        }
    }

    void CouncilsEnterCombat()
    {
        for (int32 i = 0; i < 4; i++)
            if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[i])))
                if (me->GetEntry() != council->GetEntry())
                    if (council->isAlive() && !council->isInCombat())
                        DoZoneInCombat(council, 150.0f);
        if (instance->GetBossState(DATA_COUNCIL_OF_ELDERS) != IN_PROGRESS)
            instance->SetBossState(DATA_COUNCIL_OF_ELDERS, IN_PROGRESS);
    }
};

class boss_council_of_elders : public CreatureScript
{
public:
    boss_council_of_elders() : CreatureScript("boss_council_of_elders") {}

    struct boss_council_of_eldersAI : public council_of_eldersAI
    {
        boss_council_of_eldersAI(Creature* creature) : council_of_eldersAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        SummonList summon;
        Position chargepos;
        EventMap events;
        uint32 donehppct;
        uint32 timerpower;
        uint32 actualtimer;
        uint32 loaspirittimer;
        bool needresetevents;

        void Reset()
        {
            CouncilsReset();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            RemovePassenger(true);
            events.Reset();
            summon.DespawnAll();
            me->RemoveAurasDueToSpell(SPELL_R_CHARGE_VISUAL);
            me->RemoveAurasDueToSpell(SPELL_K_OVERLOAD_AURA);
            if (me->GetEntry() == NPC_KAZRAJIN)
                me->SetReactState(REACT_PASSIVE);
            else
                me->SetReactState(REACT_DEFENSIVE);
            if (me->GetEntry() == NPC_FROST_KING_MALAKK)
                ResetGarajalSoul();
            me->setPowerType(POWER_ENERGY);
            me->SetPower(POWER_ENERGY, 0);
            donehppct = 100; //default value
            timerpower = 0;
            actualtimer = 0;
            needresetevents = false;
            loaspirittimer = 0;
        }

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
        }

        void RemovePassenger(bool action)
        {
            if (Vehicle* ij = me->GetVehicleKit())
            {
                if (Unit* p = ij->GetPassenger(0))
                {
                    if (p->ToCreature() && p->ToCreature()->AI())
                    {
                        if (action)
                            p->ToCreature()->AI()->DoAction(ACTION_RESET);
                        else
                            p->ToCreature()->AI()->DoAction(ACTION_CHANGE_COUNCIL);
                    }
                }
            }
        }

        void ResetGarajalSoul()
        {
            if (Creature* gs = me->GetCreature(*me, instance->GetGuidData(NPC_GARAJAL_SOUL)))
                gs->AI()->DoAction(ACTION_RESET);
        }

        void ActiveGarajalSoul()
        {
            if (Creature* gs = me->GetCreature(*me, instance->GetGuidData(NPC_GARAJAL_SOUL)))
                gs->AI()->DoAction(ACTION_GARAJAL_SOUL_ACTIVE);
        }

        void ResetEvents()
        {
            switch (me->GetEntry())
            {
            case NPC_FROST_KING_MALAKK:
                events.RescheduleEvent(EVENT_FRIGIT_ASSAULT, 15000);
                events.RescheduleEvent(EVENT_BITTING_COLD, 30000);
                break;
            case NPC_SUL_SANDCRAWLER:
                events.RescheduleEvent(EVENT_SAND_BOLT, 30000);
                events.RescheduleEvent(EVENT_SAND_TRAP, 40000);
                break;
            case NPC_PRINCESS_MARLI:
                loaspirittimer = 38000;
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (donehppct < 100)
            {
                if (HealthBelowPct(donehppct))
                {
                    donehppct = 100;
                    RemovePassenger(false);
                }
            }
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                donehppct = uint32(me->GetHealthPct()) - 25 < 0 ? 1 : uint32(me->GetHealthPct() - 25);
                DoCast(me, SPELL_POSSESSED, true);
                if (me->HasAura(SPELL_LINGERING_PRESENCE))
                {
                    uint8 stack = me->GetAura(SPELL_LINGERING_PRESENCE)->GetStackAmount();
                    timerpower = 670 - (67 * stack);
                    if (me->GetEntry() == NPC_SUL_SANDCRAWLER)
                    {
                        events.Reset();
                        events.RescheduleEvent(EVENT_SANDSTORM, 1000);
                    }
                }
                else
                    timerpower = 670;
                actualtimer = timerpower;
            }
            else
            {
                me->RemoveAurasDueToSpell(SPELL_POSSESSED);
                me->SetPower(POWER_ENERGY, 0);
                DoCast(me, SPELL_LINGERING_PRESENCE, true);
                timerpower = 0;
                actualtimer = 0;
                if (me->GetEntry() != NPC_KAZRAJIN)
                {
                    if (needresetevents)
                    {
                        needresetevents = false;
                        ResetEvents();
                    }
                }
            }
        }

        void EnterCombat(Unit* who)
        {
            CouncilsEnterCombat();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            switch (me->GetEntry())
            {
            case NPC_FROST_KING_MALAKK:
                ActiveGarajalSoul();
                events.RescheduleEvent(EVENT_FRIGIT_ASSAULT, 15000);
                events.RescheduleEvent(EVENT_BITTING_COLD, 30000);
                break;
            case NPC_SUL_SANDCRAWLER:
                events.RescheduleEvent(EVENT_SAND_BOLT, 35000);
                events.RescheduleEvent(EVENT_SAND_TRAP, 40000);
                break;
            case NPC_KAZRAJIN:
                events.RescheduleEvent(EVENT_R_CHARGE, 6000);
                break;
            case NPC_PRINCESS_MARLI:
                loaspirittimer = 38000;
                break;
            default:
                break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            if (instance->GetData(DATA_CHECK_COUNCIL_PROGRESS))
                me->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            else
            {
                std::list<Creature*>list;
                list.clear();
                for (uint8 n = 0; n < 6; n++)
                    GetCreatureListWithEntryInGrid(list, me, loaspiritentry[n], 150.0f);
                if (!list.empty())
                    for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                        (*itr)->DespawnOrUnsummon();
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_R_CHARGE_VISUAL)
                me->GetMotionMaster()->MoveCharge(chargepos.GetPositionX(), chargepos.GetPositionY(), chargepos.GetPositionZ(), 35.0f, 1);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (pointId == 1)
                {
                    me->RemoveAurasDueToSpell(SPELL_R_CHARGE_VISUAL);
                    DoCastAOE(SPELL_R_CHARGE_POINT_DMG);
                    if (me->GetPower(POWER_ENERGY) != 100)
                    {
                        if (me->HasAura(SPELL_POSSESSED))
                        {
                            me->AddAura(SPELL_K_OVERLOAD_AURA, me);
                            events.RescheduleEvent(EVENT_R_CHARGE, 23000);
                        }
                        else
                            events.RescheduleEvent(EVENT_R_CHARGE, 3000);
                    }
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (timerpower)
            {
                if (timerpower <= diff)
                {
                    if (me->GetPower(POWER_ENERGY) < 100)
                    {
                        me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) + 1);
                        if (me->GetPower(POWER_ENERGY) == 100)
                        {
                            needresetevents = true;
                            events.Reset();
                            if (me->GetEntry() == NPC_PRINCESS_MARLI)
                                loaspirittimer = 0;
                            timerpower = 0;
                        }
                        else
                            timerpower = actualtimer;
                    }
                }
                else
                    timerpower -= diff;
            }

            if (loaspirittimer)
            {
                if (loaspirittimer <= diff)
                {
                    DoCast(me, me->HasAura(SPELL_POSSESSED) ? SPELL_B_LOA_SPIRIT_P_SUM : SPELL_D_LOA_SPIRIT_P_SUM);
                    loaspirittimer = 38000;
                }
                else
                    loaspirittimer -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                //Mallak
                case EVENT_BITTING_COLD:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                        DoCast(target, SPELL_BITING_COLD);
                    events.RescheduleEvent(EVENT_BITTING_COLD, 45000);
                    break;
                case EVENT_FRIGIT_ASSAULT:
                    DoCast(me, SPELL_FRIGIT_ASSAULT);
                    events.RescheduleEvent(EVENT_FRIGIT_ASSAULT, 30000);
                    break;
                //Sul
                case EVENT_SAND_BOLT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                        DoCast(target, SPELL_SAND_BOLT);
                    events.RescheduleEvent(EVENT_SAND_BOLT, 30000);
                    break;
                case EVENT_SAND_TRAP:
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 80.0f, true))
                    {
                        Position pos;
                        target->GetPosition(&pos);
                        me->SummonCreature(NPC_LIVING_SAND, pos.GetPositionX(), pos.GetPositionY(), me->GetPositionZ());
                    }
                    events.RescheduleEvent(EVENT_SAND_TRAP, 40000);
                    break;
                case EVENT_SANDSTORM:
                    DoCast(me, SPELL_SANDSTORM);
                    events.RescheduleEvent(EVENT_SANDSTORM, 40000);
                    break;
                //Kazrajin
                case EVENT_R_CHARGE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                    {
                        me->SetFacingToObject(target);
                        target->GetPosition(&chargepos);
                        target->CastSpell(target, SPELL_R_CHARGE_POINT_T);
                        DoCast(me, SPELL_R_CHARGE_VISUAL);
                    }
                    break;
                }
            }
            if (me->GetEntry() != NPC_PRINCESS_MARLI)
                DoMeleeAttackIfReady();
            else
            {
                if (me->HasAura(SPELL_POSSESSED))
                    DoSpellAttackIfReady(SPELL_D_WRATH_OF_THE_LOA);
                else
                    DoSpellAttackIfReady(SPELL_WRATH_OF_THE_LOA);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_council_of_eldersAI(creature);
    }
};

//69182
class npc_garajal_soul : public CreatureScript
{
public:
    npc_garajal_soul() : CreatureScript("npc_garajal_soul") {}

    struct npc_garajal_soulAI : public ScriptedAI
    {
        npc_garajal_soulAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* instance;
        EventMap events;
        uint32 lastcouncil;
        bool donecouncil;
        uint8 councilcount;

        void Reset()
        {
            events.Reset();
            councilcount = 0;
            lastcouncil = 0;
            donecouncil = false;
            DoCast(me, SPELL_GARAJAL_SOUL_V, true);
        }

        uint8 GetCouncilCount(uint32 entry)
        {
            switch (entry)
            {
            case NPC_FROST_KING_MALAKK:
                return 0;
            case NPC_PRINCESS_MARLI:
                return 1;
            case NPC_KAZRAJIN:
                return 2;
            case NPC_SUL_SANDCRAWLER:
                return 3;
            }
            return 0;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_RESET:
                events.Reset();
                councilcount = 0;
                donecouncil = false;
                lastcouncil = 0;
                me->ExitVehicle();
                me->GetMotionMaster()->Clear(false);
                me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), me->GetHomePosition().GetOrientation());
                break;
            case ACTION_CHANGE_COUNCIL:
                me->ExitVehicle();
                councilcount++;
                if ((councilcount > 3 && !donecouncil) || donecouncil)
                {
                    if (!donecouncil)
                        donecouncil = true;
                    GuidVector councillistGuids;
                    councillistGuids.clear();
                    for (uint8 n = 0; n < 4; n++)
                        if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[n])))
                            if (council->GetEntry() != lastcouncil)
                                if (council->isAlive() && council->isInCombat())
                                    councillistGuids.push_back(council->GetGUID());

                    if (!councillistGuids.empty())
                    {
                        GuidVector::const_iterator Itr = councillistGuids.begin();
                        std::advance(Itr, urand(0, councillistGuids.size() - 1));
                        if (Creature* council = me->GetCreature(*me, *Itr))
                            councilcount = GetCouncilCount(council->GetEntry());

                        if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[councilcount])))
                        {
                            lastcouncil = council->GetEntry();
                            me->GetMotionMaster()->MoveCharge(council->GetPositionX(), council->GetPositionY(), council->GetPositionZ(), 15.0f, 1);
                        }
                    }
                }
                else
                {
                    for (uint8 b = councilcount; b < 4; b++)
                    {
                        if (Creature* council = me->FindNearestCreature(councilentry[b], 200.0f, true))
                        {
                            councilcount = b;
                            lastcouncil = council->GetEntry();
                            me->GetMotionMaster()->MoveCharge(council->GetPositionX(), council->GetPositionY(), council->GetPositionZ(), 15.0f, 1);
                            break;
                        }
                    }
                }
                events.RescheduleEvent(EVENT_MOVING, 1000);
                break;
            case ACTION_GARAJAL_SOUL_ACTIVE:
                if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[councilcount])))
                {
                    lastcouncil = council->GetEntry();
                    me->GetMotionMaster()->MoveCharge(council->GetPositionX(), council->GetPositionY(), council->GetPositionZ(), 15.0f, 1);
                    events.RescheduleEvent(EVENT_MOVING, 1000);
                }
                break;
            default:
                break;
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE && pointId)
            {
                events.Reset();
                if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[councilcount])))
                    me->EnterVehicle(council, 0);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_MOVING)
                {
                    if (Creature* council = me->GetCreature(*me, instance->GetGuidData(councilentry[councilcount])))
                    {
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MoveCharge(council->GetPositionX(), council->GetPositionY(), council->GetPositionZ(), 15.0f, 1);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_garajal_soulAI(creature);
    }
};

//69480, 69491, 69492; 69548, 69553, 69556
class npc_loa_spirit : public CreatureScript
{
public:
    npc_loa_spirit() : CreatureScript("npc_loa_spirit") {}

    struct npc_loa_spiritAI : public ScriptedAI
    {
        npc_loa_spiritAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }
        InstanceScript* instance;
        EventMap events;
        ObjectGuid councilGuid;
        ObjectGuid targetGuid;
        uint32 checktarget;
        bool done;

        void Reset()
        {
            councilGuid.Clear();
            targetGuid.Clear();
            done = false;
            switch (me->GetEntry())
            {
            //go to low hp boss
            case NPC_BLESSED_LOA_SPIRIT:
            case NPC_BLESSED_LOA_SPIRIT_2:
            case NPC_BLESSED_LOA_SPIRIT_3:
                DoCast(me, SPELL_BLESSED_TRANSFORM, true);
                events.RescheduleEvent(EVENT_FIND_LOWHP_COUNCIL, 500);
                break;
            //go to player
            case NPC_DARK_LOA_SPIRIT:
            case NPC_DARK_LOA_SPIRIT_2:
            case NPC_DARK_LOA_SPIRIT_3:
                DoCast(me, SPELL_SHADOW_TRANSFORM, true);
                events.RescheduleEvent(EVENT_FIND_PLAYER, 500);
                break;
            }
        }

        void FindAndStartPursuitPlayer()
        {
            if (me->ToTempSummon())
            {
                if (Unit* council = me->ToTempSummon()->GetSummoner())
                {
                    if (council->ToCreature() && council->ToCreature()->AI())
                    {
                        if (Unit* target = council->ToCreature()->AI()->SelectTarget(SELECT_TARGET_FARTHEST, 0.0f, 100.0f, true))
                        {
                            targetGuid = target->GetGUID();
                            DoCast(target, SPELL_MARKED_SOUL, true);
                            me->AddThreat(target, 50000000.0f);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->TauntApply(target);
                        }
                    }
                }
            }
            events.RescheduleEvent(EVENT_CHECK_DISTANCE, 1000);
        }

        ObjectGuid GetLowestHpCouncil()
        {
            std::list<Creature*> councillist;
            councillist.clear();
            float lasthppct = 100;
            for (uint8 n = 0; n < 4; n++)
                if (Creature* council = me->FindNearestCreature(councilentry[n], 150.0f, true))
                    councillist.push_back(council);

            if (!councillist.empty())
            {
                for (std::list<Creature*>::const_iterator itr = councillist.begin(); itr != councillist.end(); itr++)
                    if ((*itr)->GetHealthPct() < lasthppct)
                        lasthppct = (*itr)->GetHealthPct();

                for (std::list<Creature*>::const_iterator itr = councillist.begin(); itr != councillist.end(); itr++)
                    if ((*itr)->GetHealthPct() <= lasthppct)
                        return ((*itr)->GetGUID());
            }
            return ObjectGuid::Empty;
        }

        void CheckCouncil()
        {
            if (Creature* council = me->GetCreature(*me, councilGuid))
            {
                if (council->isAlive())
                {
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveCharge(council->GetPositionX(), council->GetPositionY(), council->GetPositionZ(), 4.0f, 0);
                    events.RescheduleEvent(EVENT_CHECK_COUNCIL, 1000);
                    return;
                }
            }
            me->StopMoving();
            me->GetMotionMaster()->Clear(false);
            events.RescheduleEvent(EVENT_FIND_LOWHP_COUNCIL, 1000);
        }

        void JustDied(Unit* killer)
        {
            switch (me->GetEntry())
            {
            case NPC_DARK_LOA_SPIRIT:
            case NPC_DARK_LOA_SPIRIT_2:
            case NPC_DARK_LOA_SPIRIT_3:
                if (Player* pl = me->GetPlayer(*me, targetGuid))
                    pl->RemoveAurasDueToSpell(SPELL_MARKED_SOUL);
                break;
            default:
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            me->getThreatManager().addThreat(attacker, 0.0f);
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
                    if (Creature* council = me->GetCreature(*me, councilGuid))
                    {
                        if (council->isAlive())
                        {
                            DoCast(council, SPELL_BLESSED_GIFT, true);
                            me->DespawnOrUnsummon();
                        }
                    }
                }
            }
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
                case EVENT_FIND_LOWHP_COUNCIL:
                    if (Creature* council = me->GetCreature(*me, GetLowestHpCouncil()))
                    {
                        councilGuid = council->GetGUID();
                        me->GetMotionMaster()->MoveCharge(council->GetPositionX(), council->GetPositionY(), council->GetPositionZ(), 4.0f, 0);
                        events.RescheduleEvent(EVENT_CHECK_COUNCIL, 1000);
                    }
                    else
                        events.RescheduleEvent(EVENT_FIND_LOWHP_COUNCIL, 1000);
                    break;
                case EVENT_CHECK_COUNCIL:
                    CheckCouncil();
                    break;
                case EVENT_FIND_PLAYER:
                    FindAndStartPursuitPlayer();
                    break;
                case EVENT_CHECK_DISTANCE:
                {
                    Player* pl = me->GetPlayer(*me, targetGuid);
                    if (pl && pl->isAlive())
                    {
                        if (IsInControl())
                        {
                            events.RescheduleEvent(EVENT_CHECK_DISTANCE, 1000);
                            return;
                        }
                        if (me->GetDistance(pl) <= 6.0f && !done)
                        {
                            done = true;
                            pl->RemoveAurasDueToSpell(SPELL_MARKED_SOUL);
                            me->GetMotionMaster()->Clear(false);
                            DoCast(pl, SPELL_DARK_EXPLOSE, true);
                            me->DespawnOrUnsummon(1000);
                            return;
                        }
                    }
                    else
                    {
                        me->StopAttack();
                        events.RescheduleEvent(EVENT_FIND_PLAYER, 1000);
                        return;
                    }
                    events.RescheduleEvent(EVENT_CHECK_DISTANCE, 1000);
                }
                break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_loa_spiritAI(creature);
    }
};

//69153
class npc_living_sand : public CreatureScript
{
public:
    npc_living_sand() : CreatureScript("npc_living_sand") {}

    struct npc_living_sandAI : public ScriptedAI
    {
        npc_living_sandAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;
        bool done;

        void Reset()
        {
            done = false;
            DoCast(me, SPELL_SAND_TRAP_VISUAL, true);
            DoCast(me, SPELL_SAND_TRAP_AT, true);
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_ACTIVE)
            {
                me->RemoveAurasDueToSpell(SPELL_SAND_TRAP_VISUAL);
                me->RemoveAurasDueToSpell(SPELL_SAND_TRAP_AT);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->Attack(target, true);
                    me->GetMotionMaster()->MoveChase(target);
                    done = false;
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth() && !done)
            {
                done = true;
                me->StopAttack();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                DoCast(me, SPELL_SAND_TRAP_VISUAL, true);
                DoCast(me, SPELL_SAND_TRAP_AT, true);
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_living_sandAI(creature);
    }
};


class spell_frigit_assault : public SpellScriptLoader
{
public:
    spell_frigit_assault() : SpellScriptLoader("spell_frigit_assault") { }

    class spell_frigit_assault_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_frigit_assault_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget())
            {
                if (GetTarget()->HasAura(SPELL_FRIGIT_ASSAULT_D) && !GetTarget()->HasAura(SPELL_FRIGIT_ASSAULT_S))
                    if (GetTarget()->GetAura(SPELL_FRIGIT_ASSAULT_D)->GetStackAmount() == 15)
                        GetTarget()->AddAura(SPELL_FRIGIT_ASSAULT_S, GetTarget());
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_frigit_assault_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_frigit_assault_AuraScript();
    }
};

//136442
class spell_possessed : public SpellScriptLoader
{
public:
    spell_possessed() : SpellScriptLoader("spell_possessed") { }

    class spell_possessed_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_possessed_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetTarget()->ToCreature())
                if (GetTarget()->GetPower(POWER_ENERGY) == 100)
                    GetTarget()->CastSpell(GetTarget(), SPELL_DARK_POWER, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_possessed_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_possessed_AuraScript();
    }
};

//137203, 137350
class spell_loa_spirit : public SpellScriptLoader
{
public:
    spell_loa_spirit() : SpellScriptLoader("spell_loa_spirit") { }

    class spell_loa_spirit_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_loa_spirit_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                switch (GetSpellInfo()->Id)
                {
                case SPELL_B_LOA_SPIRIT_P_SUM:
                    GetCaster()->CastSpell(GetCaster(), blessed_loa_spirit[urand(0, 2)], true);
                    break;
                case SPELL_D_LOA_SPIRIT_P_SUM:
                    GetCaster()->CastSpell(GetCaster(), dark_loa_spirit[urand(0, 2)], true);
                    break;
                default:
                    break;
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_loa_spirit_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_loa_spirit_SpellScript();
    }
};

//136894
class spell_sandstorm : public SpellScriptLoader
{
public:
    spell_sandstorm() : SpellScriptLoader("spell_sandstorm") { }

    class spell_sandstorm_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_sandstorm_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if (GetCaster())
            {
                GetCaster()->AddAura(SPELL_SANDSTORM_VISUAL, GetCaster());
                std::list<Creature*>list;
                list.clear();
                GetCreatureListWithEntryInGrid(list, GetCaster(), NPC_LIVING_SAND, 150.0f);
                if (!list.empty())
                {
                    for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                    {
                        if ((*itr)->HasAura(SPELL_SAND_TRAP_VISUAL))
                            (*itr)->AI()->DoAction(ACTION_ACTIVE);
                        else
                            (*itr)->CastSpell(*itr, SPELL_FORTIFIED, true);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_sandstorm_AuraScript::OnApply, EFFECT_0, SPELL_EFFECT_APPLY_AURA, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_sandstorm_AuraScript();
    }
};

//136860
class spell_coe_sand_trap : public SpellScriptLoader
{
public:
    spell_coe_sand_trap() : SpellScriptLoader("spell_coe_sand_trap") { }

    class spell_coe_sand_trap_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_coe_sand_trap_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget())
                GetTarget()->CastSpell(GetTarget(), SPELL_ENSNARED, true);

            if (Aura* aura = GetTarget()->GetAura(SPELL_ENSNARED))
                if (aura->GetStackAmount() == 5 && !GetTarget()->HasAura(SPELL_ENTRAPPED))
                    GetTarget()->CastSpell(GetTarget(), SPELL_ENTRAPPED, true);
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget())
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_ENSNARED);
                GetTarget()->RemoveAurasDueToSpell(SPELL_ENTRAPPED);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_coe_sand_trap_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            OnEffectRemove += AuraEffectRemoveFn(spell_coe_sand_trap_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_coe_sand_trap_AuraScript();
    }
};

void AddSC_boss_council_of_elders()
{
    new boss_council_of_elders();
    new npc_garajal_soul();
    new npc_loa_spirit();
    new npc_living_sand();
    new spell_frigit_assault();
    new spell_possessed();
    new spell_loa_spirit();
    new spell_sandstorm();
    new spell_coe_sand_trap();
}
